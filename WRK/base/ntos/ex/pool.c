/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    pool.c

Abstract:

    This module implements the NT executive pool allocator.

--*/

#include "exp.h"

#pragma hdrstop

#undef ExAllocatePoolWithTag
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#undef ExAllocatePoolWithQuotaTag
#undef ExFreePool
#undef ExFreePoolWithTag

//
// These bitfield definitions are based on EX_POOL_PRIORITY in inc\ex.h.
//

#define POOL_SPECIAL_POOL_BIT               0x8
#define POOL_SPECIAL_POOL_UNDERRUN_BIT      0x1

//
// Define forward referenced function prototypes.
//

#ifdef ALLOC_PRAGMA
PVOID
ExpAllocateStringRoutine (
    IN SIZE_T NumberOfBytes
    );

VOID
ExDeferredFreePool (
     IN PPOOL_DESCRIPTOR PoolDesc
     );

VOID
ExpSeedHotTags (
    VOID
    );

NTSTATUS
ExGetSessionPoolTagInfo (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN OUT PULONG ReturnedEntries,
    IN OUT PULONG ActualEntries
    );

NTSTATUS
ExGetPoolTagInfo (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN OUT PULONG ReturnLength OPTIONAL
    );

#pragma alloc_text(PAGE, ExpAllocateStringRoutine)
#pragma alloc_text(INIT, InitializePool)
#pragma alloc_text(INIT, ExpSeedHotTags)
#pragma alloc_text(PAGE, ExInitializePoolDescriptor)
#pragma alloc_text(PAGE, ExDrainPoolLookasideList)
#pragma alloc_text(PAGE, ExCreatePoolTagTable)
#pragma alloc_text(PAGE, ExGetSessionPoolTagInfo)
#pragma alloc_text(PAGE, ExGetPoolTagInfo)
#pragma alloc_text(PAGEVRFY, ExAllocatePoolSanityChecks)
#pragma alloc_text(PAGEVRFY, ExFreePoolSanityChecks)
#pragma alloc_text(POOLCODE, ExAllocatePoolWithTag)
#pragma alloc_text(POOLCODE, ExFreePool)
#pragma alloc_text(POOLCODE, ExFreePoolWithTag)
#pragma alloc_text(POOLCODE, ExDeferredFreePool)
#endif

#if defined (NT_UP)
#define USING_HOT_COLD_METRICS (ExpPoolFlags & EX_SEPARATE_HOT_PAGES_DURING_BOOT)
#else
#define USING_HOT_COLD_METRICS 0
#endif

#define EXP_MAXIMUM_POOL_FREES_PENDING 32

PPOOL_DESCRIPTOR ExpSessionPoolDescriptor;
PGENERAL_LOOKASIDE ExpSessionPoolLookaside;
PPOOL_TRACKER_TABLE ExpSessionPoolTrackTable;
SIZE_T ExpSessionPoolTrackTableSize;
SIZE_T ExpSessionPoolTrackTableMask;
PPOOL_TRACKER_BIG_PAGES ExpSessionPoolBigPageTable;
SIZE_T ExpSessionPoolBigPageTableSize;
SIZE_T ExpSessionPoolBigPageTableHash;
ULONG ExpSessionPoolSmallLists;

#if DBG
ULONG ExpLargeSessionPoolUnTracked;
#endif
ULONG ExpBigTableExpansionFailed;
LONG ExpPoolBigEntriesInUse;

extern SIZE_T MmSizeOfNonPagedPoolInBytes;

#if defined (NT_UP)
KDPC ExpBootFinishedTimerDpc;
KTIMER ExpBootFinishedTimer;

VOID
ExpBootFinishedDispatch (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

#else

#if defined (_WIN64)
#define MAXIMUM_PROCESSOR_TAG_TABLES    64      // Must be a power of 2.
#else
#define MAXIMUM_PROCESSOR_TAG_TABLES    32      // Must be a power of 2.
#endif

PPOOL_TRACKER_TABLE ExPoolTagTables[MAXIMUM_PROCESSOR_TAG_TABLES];

#endif

#define DEFAULT_TRACKER_TABLE 2048

PPOOL_TRACKER_TABLE PoolTrackTable;

//
// Registry-overridable, but must be a power of 2.
//

SIZE_T PoolTrackTableSize;

SIZE_T PoolTrackTableMask;

PPOOL_TRACKER_TABLE PoolTrackTableExpansion;
SIZE_T PoolTrackTableExpansionSize;
SIZE_T PoolTrackTableExpansionPages;

#define DEFAULT_BIGPAGE_TABLE 4096

PPOOL_TRACKER_BIG_PAGES PoolBigPageTable;

//
// Registry-overridable, but must be a power of 2.
//

SIZE_T PoolBigPageTableSize;   // Must be a power of 2.

SIZE_T PoolBigPageTableHash;

#define POOL_BIG_TABLE_ENTRY_FREE   0x1     // Must be the low bit since InterlockedAdd is used also to set/clear this.

ULONG PoolHitTag = 0xffffff0f;

FORCEINLINE
ULONG
POOLTAG_HASH (
    IN ULONG Key,
    IN SIZE_T Mask
    )

/*++

Routine Description:

    This function builds a hash table index based on the supplied pool
    tag.

Arguments:

    Key - Supplies the key value to hash.

    Mask - Supplies the mask value to apply to the resultant key to generate
           a valid hash table index.
           PoolDescriptor - Supplies a pointer to the pool descriptor.

Return Value:

    None.

--*/
{
    ULONG64 hash;

    hash = (ULONG64)Key * 40543;
    hash ^= hash >> 32;
    return (ULONG)(hash & Mask);
}

VOID
ExpInsertPoolTracker (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    );

VOID
ExpInsertPoolTrackerExpansion (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    );

VOID
ExpRemovePoolTracker (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    );

VOID
ExpRemovePoolTrackerExpansion (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    );

LOGICAL
ExpAddTagForBigPages (
    IN PVOID Va,
    IN ULONG Key,
    IN ULONG NumberOfPages,
    IN POOL_TYPE PoolType
    );

ULONG
ExpFindAndRemoveTagBigPages (
    IN PVOID Va,
    OUT PULONG BigPages,
    IN POOL_TYPE PoolType
    );

PVOID
ExpAllocateStringRoutine (
    IN SIZE_T NumberOfBytes
    )
{
    return ExAllocatePoolWithTag (PagedPool,NumberOfBytes,'grtS');
}

BOOLEAN
ExOkayToLockRoutine (
    IN PVOID Lock
    )
{
    UNREFERENCED_PARAMETER (Lock);

    if (KeIsExecutingDpc()) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif
const PRTL_ALLOCATE_STRING_ROUTINE RtlAllocateStringRoutine = ExpAllocateStringRoutine;
const PRTL_FREE_STRING_ROUTINE RtlFreeStringRoutine = (PRTL_FREE_STRING_ROUTINE)ExFreePool;
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

ULONG ExPoolFailures;

//
// Define macros to pack and unpack a pool index.
//

#define ENCODE_POOL_INDEX(POOLHEADER,INDEX) {(POOLHEADER)->PoolIndex = ((UCHAR)(INDEX));}
#define DECODE_POOL_INDEX(POOLHEADER)       ((ULONG)((POOLHEADER)->PoolIndex))

//
// The allocated bit carefully overlays the unused cachealign bit in the type.
//

#define POOL_IN_USE_MASK                            0x4

#define MARK_POOL_HEADER_FREED(POOLHEADER)          {(POOLHEADER)->PoolType &= ~POOL_IN_USE_MASK;}
#define IS_POOL_HEADER_MARKED_ALLOCATED(POOLHEADER) ((POOLHEADER)->PoolType & POOL_IN_USE_MASK)

//
// The hotpage bit carefully overlays the raise bit in the type.
//

#define POOL_HOTPAGE_MASK   POOL_RAISE_IF_ALLOCATION_FAILURE

//
// Define the number of paged pools. This value may be overridden at boot
// time.
//

ULONG ExpNumberOfPagedPools = NUMBER_OF_PAGED_POOLS;

ULONG ExpNumberOfNonPagedPools = 1;

//
// The pool descriptor for nonpaged pool is static.
// The pool descriptors for paged pool are dynamically allocated
// since there can be more than one paged pool. There is always one more
// paged pool descriptor than there are paged pools. This descriptor is
// used when a page allocation is done for a paged pool and is the first
// descriptor in the paged pool descriptor array.
//

POOL_DESCRIPTOR NonPagedPoolDescriptor;

#define EXP_MAXIMUM_POOL_NODES 16

PPOOL_DESCRIPTOR ExpNonPagedPoolDescriptor[EXP_MAXIMUM_POOL_NODES];

//
// The pool vector contains an array of pointers to pool descriptors. For
// nonpaged pool this is just a pointer to the nonpaged pool descriptor.
// For paged pool, this is a pointer to an array of pool descriptors.
// The pointer to the paged pool descriptor is duplicated so
// it can be found easily by the kernel debugger.
//

PPOOL_DESCRIPTOR PoolVector[NUMBER_OF_POOLS];
PPOOL_DESCRIPTOR ExpPagedPoolDescriptor[EXP_MAXIMUM_POOL_NODES + 1];
PKGUARDED_MUTEX ExpPagedPoolMutex;

volatile ULONG ExpPoolIndex = 1;
KSPIN_LOCK ExpTaggedPoolLock;

EX_SPIN_LOCK ExpLargePoolTableLock;

#if DBG

LONG ExConcurrentQuotaPool;
LONG ExConcurrentQuotaPoolMax;

PSZ PoolTypeNames[MaxPoolType] = {
    "NonPaged",
    "Paged",
    "NonPagedMustSucceed",
    "NotUsed",
    "NonPagedCacheAligned",
    "PagedCacheAligned",
    "NonPagedCacheAlignedMustS"
    };

#endif //DBG


//
// Define paged and nonpaged pool lookaside descriptors.
//

GENERAL_LOOKASIDE ExpSmallNPagedPoolLookasideLists[POOL_SMALL_LISTS];

GENERAL_LOOKASIDE ExpSmallPagedPoolLookasideLists[POOL_SMALL_LISTS];



#define LOCK_POOL(PoolDesc, LockHandle) {                                   \
    if ((PoolDesc->PoolType & BASE_POOL_TYPE_MASK) == NonPagedPool) {       \
        if (PoolDesc == &NonPagedPoolDescriptor) {                          \
            LockHandle.OldIrql = KeAcquireQueuedSpinLock(LockQueueNonPagedPoolLock); \
        }                                                                   \
        else {                                                              \
            ASSERT (ExpNumberOfNonPagedPools > 1);                          \
            KeAcquireInStackQueuedSpinLock (PoolDesc->LockAddress, &LockHandle); \
        }                                                                   \
    }                                                                       \
    else {                                                                  \
        KeAcquireGuardedMutex ((PKGUARDED_MUTEX)PoolDesc->LockAddress);     \
    }                                                                       \
}



#define UNLOCK_POOL(PoolDesc, LockHandle) {                                 \
    if ((PoolDesc->PoolType & BASE_POOL_TYPE_MASK) == NonPagedPool) {       \
        if (PoolDesc == &NonPagedPoolDescriptor) {                          \
            KeReleaseQueuedSpinLock(LockQueueNonPagedPoolLock, LockHandle.OldIrql); \
        }                                                                   \
        else {                                                              \
            ASSERT (ExpNumberOfNonPagedPools > 1);                          \
            KeReleaseInStackQueuedSpinLock (&LockHandle);                   \
        }                                                                   \
    }                                                                       \
    else {                                                                  \
        KeReleaseGuardedMutex ((PKGUARDED_MUTEX)PoolDesc->LockAddress);     \
    }                                                                       \
}

#ifndef NO_POOL_CHECKS


//
// We redefine the LIST_ENTRY macros to have each pointer biased
// by one so any rogue code using these pointers will access
// violate.  See \nt\public\sdk\inc\ntrtl.h for the original
// definition of these macros.
//
// This is turned off in the shipping product.
//

#define DecodeLink(Link) ((PLIST_ENTRY)((ULONG_PTR)(Link) & ~1))
#define EncodeLink(Link) ((PLIST_ENTRY)((ULONG_PTR)(Link) |  1))

#define PrivateInitializeListHead(ListHead) (                     \
    (ListHead)->Flink = (ListHead)->Blink = EncodeLink(ListHead))

#define PrivateIsListEmpty(ListHead)              \
    (DecodeLink((ListHead)->Flink) == (ListHead))

#define PrivateRemoveHeadList(ListHead)                     \
    DecodeLink((ListHead)->Flink);                          \
    {PrivateRemoveEntryList(DecodeLink((ListHead)->Flink))}

#define PrivateRemoveTailList(ListHead)                     \
    DecodeLink((ListHead)->Blink);                          \
    {PrivateRemoveEntryList(DecodeLink((ListHead)->Blink))}

#define PrivateRemoveEntryList(Entry) {       \
    PLIST_ENTRY _EX_Blink;                    \
    PLIST_ENTRY _EX_Flink;                    \
    _EX_Flink = DecodeLink((Entry)->Flink);   \
    _EX_Blink = DecodeLink((Entry)->Blink);   \
    _EX_Blink->Flink = EncodeLink(_EX_Flink); \
    _EX_Flink->Blink = EncodeLink(_EX_Blink); \
    }

#define CHECK_LIST(LIST)                                                    \
    if ((DecodeLink(DecodeLink((LIST)->Flink)->Blink) != (LIST)) ||         \
        (DecodeLink(DecodeLink((LIST)->Blink)->Flink) != (LIST))) {         \
            KeBugCheckEx (BAD_POOL_HEADER,                                  \
                          3,                                                \
                          (ULONG_PTR)LIST,                                  \
                          (ULONG_PTR)DecodeLink(DecodeLink((LIST)->Flink)->Blink),     \
                          (ULONG_PTR)DecodeLink(DecodeLink((LIST)->Blink)->Flink));    \
    }

#define PrivateInsertTailList(ListHead,Entry) {  \
    PLIST_ENTRY _EX_Blink;                       \
    PLIST_ENTRY _EX_ListHead;                    \
    _EX_ListHead = (ListHead);                   \
    CHECK_LIST(_EX_ListHead);                    \
    _EX_Blink = DecodeLink(_EX_ListHead->Blink); \
    (Entry)->Flink = EncodeLink(_EX_ListHead);   \
    (Entry)->Blink = EncodeLink(_EX_Blink);      \
    _EX_Blink->Flink = EncodeLink(Entry);        \
    _EX_ListHead->Blink = EncodeLink(Entry);     \
    CHECK_LIST(_EX_ListHead);                    \
    }

#define PrivateInsertHeadList(ListHead,Entry) {  \
    PLIST_ENTRY _EX_Flink;                       \
    PLIST_ENTRY _EX_ListHead;                    \
    _EX_ListHead = (ListHead);                   \
    CHECK_LIST(_EX_ListHead);                    \
    _EX_Flink = DecodeLink(_EX_ListHead->Flink); \
    (Entry)->Flink = EncodeLink(_EX_Flink);      \
    (Entry)->Blink = EncodeLink(_EX_ListHead);   \
    _EX_Flink->Blink = EncodeLink(Entry);        \
    _EX_ListHead->Flink = EncodeLink(Entry);     \
    CHECK_LIST(_EX_ListHead);                    \
    }

VOID
FORCEINLINE
ExCheckPoolHeader (
    IN PPOOL_HEADER Entry
    )
{
    PPOOL_HEADER PreviousEntry;
    PPOOL_HEADER NextEntry;

    if (Entry->PreviousSize != 0) {

        PreviousEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry - Entry->PreviousSize);
        if (PAGE_ALIGN (Entry) != PAGE_ALIGN (PreviousEntry)) {
            KeBugCheckEx (BAD_POOL_HEADER,
                          6,
                          (ULONG_PTR) PreviousEntry,
                          __LINE__,
                          (ULONG_PTR)Entry);
        }

        if ((PreviousEntry->BlockSize != Entry->PreviousSize) ||
            (DECODE_POOL_INDEX(PreviousEntry) != DECODE_POOL_INDEX(Entry))) {

            KeBugCheckEx (BAD_POOL_HEADER,
                          5,
                          (ULONG_PTR) PreviousEntry,
                          __LINE__,
                          (ULONG_PTR)Entry);
        }
    }
    else if (!PAGE_ALIGNED (Entry)) {
        KeBugCheckEx (BAD_POOL_HEADER,
                      7,
                      0,
                      __LINE__,
                      (ULONG_PTR)Entry);
    }

    if (Entry->BlockSize == 0) {
        KeBugCheckEx (BAD_POOL_HEADER,
                      8,
                      0,
                      __LINE__,
                      (ULONG_PTR)Entry);
    }

    NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + Entry->BlockSize);

    if (!PAGE_END(NextEntry)) {

        if (PAGE_ALIGN (Entry) != PAGE_ALIGN (NextEntry)) {
            KeBugCheckEx (BAD_POOL_HEADER,
                          9,
                          (ULONG_PTR) NextEntry,
                          __LINE__,
                          (ULONG_PTR)Entry);
        }

        if ((NextEntry->PreviousSize != (Entry)->BlockSize) ||
            (DECODE_POOL_INDEX(NextEntry) != DECODE_POOL_INDEX(Entry))) {

            KeBugCheckEx (BAD_POOL_HEADER,
                          5,
                          (ULONG_PTR) NextEntry,
                          __LINE__,
                          (ULONG_PTR)Entry);
        }
    }
}

#define CHECK_POOL_HEADER(ENTRY) ExCheckPoolHeader(ENTRY)

#define ASSERT_ALLOCATE_IRQL(_PoolType, _NumberOfBytes)                 \
    if ((_PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {               \
        if (KeGetCurrentIrql() > APC_LEVEL) {                           \
            KeBugCheckEx (BAD_POOL_CALLER, 8, KeGetCurrentIrql(), _PoolType, _NumberOfBytes);                                                           \
        }                                                               \
    }                                                                   \
    else {                                                              \
        if (KeGetCurrentIrql() > DISPATCH_LEVEL) {                      \
            KeBugCheckEx (BAD_POOL_CALLER, 8, KeGetCurrentIrql(), _PoolType, _NumberOfBytes);                                                           \
        }                                                               \
    }

#define ASSERT_FREE_IRQL(_PoolType, _P)                                 \
    if ((_PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {               \
        if (KeGetCurrentIrql() > APC_LEVEL) {                           \
            KeBugCheckEx (BAD_POOL_CALLER, 9, KeGetCurrentIrql(), _PoolType, (ULONG_PTR)_P);                                                            \
        }                                                               \
    }                                                                   \
    else {                                                              \
        if (KeGetCurrentIrql() > DISPATCH_LEVEL) {                      \
            KeBugCheckEx (BAD_POOL_CALLER, 9, KeGetCurrentIrql(), _PoolType, (ULONG_PTR)P);                                                             \
        }                                                               \
    }

#define ASSERT_POOL_NOT_FREE(_Entry)                                    \
    if ((_Entry->PoolType & POOL_TYPE_MASK) == 0) {                     \
        KeBugCheckEx (BAD_POOL_CALLER, 6, __LINE__, (ULONG_PTR)_Entry, _Entry->Ulong1);                                                                 \
    }

#define ASSERT_POOL_TYPE_NOT_ZERO(_Entry)                               \
    if (_Entry->PoolType == 0) {                                        \
        KeBugCheckEx(BAD_POOL_CALLER, 1, (ULONG_PTR)_Entry, (ULONG_PTR)(*(PULONG)_Entry), 0);                                                           \
    }

#define CHECK_POOL_PAGE(PAGE) \
    {                                                                         \
        PPOOL_HEADER P = (PPOOL_HEADER)PAGE_ALIGN(PAGE);                      \
        ULONG SIZE = 0;                                                       \
        LOGICAL FOUND=FALSE;                                                  \
        ASSERT (P->PreviousSize == 0);                                        \
        do {                                                                  \
            if (P == (PPOOL_HEADER)PAGE) {                                    \
                FOUND = TRUE;                                                 \
            }                                                                 \
            CHECK_POOL_HEADER(P);                                             \
            SIZE += P->BlockSize;                                             \
            P = (PPOOL_HEADER)((PPOOL_BLOCK)P + P->BlockSize);                \
        } while ((SIZE < (PAGE_SIZE / POOL_SMALLEST_BLOCK)) &&                \
                 (PAGE_END(P) == FALSE));                                     \
        if ((PAGE_END(P) == FALSE) || (FOUND == FALSE)) {                     \
            KeBugCheckEx (BAD_POOL_HEADER, 0xA, (ULONG_PTR) PAGE, __LINE__, (ULONG_PTR) P); \
        }                                                                     \
    }

#else

#define DecodeLink(Link) ((PLIST_ENTRY)((ULONG_PTR)(Link)))
#define EncodeLink(Link) ((PLIST_ENTRY)((ULONG_PTR)(Link)))
#define PrivateInitializeListHead InitializeListHead
#define PrivateIsListEmpty        IsListEmpty
#define PrivateRemoveHeadList     RemoveHeadList
#define PrivateRemoveTailList     RemoveTailList
#define PrivateRemoveEntryList    RemoveEntryList
#define PrivateInsertTailList     InsertTailList
#define PrivateInsertHeadList     InsertHeadList

#define ASSERT_ALLOCATE_IRQL(_PoolType, _P) {NOTHING;}
#define ASSERT_FREE_IRQL(_PoolType, _P)     {NOTHING;}
#define ASSERT_POOL_NOT_FREE(_Entry)        {NOTHING;}
#define ASSERT_POOL_TYPE_NOT_ZERO(_Entry)   {NOTHING;}

//
// The check list macros come in two flavors - there is one in the checked
// and free build that will bugcheck the system if a list is ill-formed, and
// there is one for the final shipping version that has all the checked
// disabled.
//
// The check lookaside list macros also comes in two flavors and is used to
// verify that the look aside lists are well formed.
//
// The check pool header macro (two flavors) verifies that the specified
// pool header matches the preceding and succeeding pool headers.
//

#define CHECK_LIST(LIST)                        {NOTHING;}
#define CHECK_POOL_HEADER(ENTRY)                {NOTHING;}

#define CHECK_POOL_PAGE(PAGE)                   {NOTHING;}

#endif

#define EX_FREE_POOL_BACKTRACE_LENGTH 8

typedef struct _EX_FREE_POOL_TRACES {

    PETHREAD Thread;
    PVOID PoolAddress;
    POOL_HEADER PoolHeader;
    PVOID StackTrace [EX_FREE_POOL_BACKTRACE_LENGTH];

} EX_FREE_POOL_TRACES, *PEX_FREE_POOL_TRACES;

LONG ExFreePoolIndex;
LONG ExFreePoolMask = 0x4000 - 1;

PEX_FREE_POOL_TRACES ExFreePoolTraces;


VOID
ExInitializePoolDescriptor (
    IN PPOOL_DESCRIPTOR PoolDescriptor,
    IN POOL_TYPE PoolType,
    IN ULONG PoolIndex,
    IN ULONG Threshold,
    IN PVOID PoolLock
    )

/*++

Routine Description:

    This function initializes a pool descriptor.

    Note that this routine is called directly by the memory manager.

Arguments:

    PoolDescriptor - Supplies a pointer to the pool descriptor.

    PoolType - Supplies the type of the pool.

    PoolIndex - Supplies the pool descriptor index.

    Threshold - Supplies the threshold value for the specified pool.

    PoolLock - Supplies a pointer to the lock for the specified pool.

Return Value:

    None.

--*/

{
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY LastListEntry;
    PPOOL_TRACKER_BIG_PAGES p;
    PPOOL_TRACKER_BIG_PAGES pend;

    //
    // Initialize statistics fields, the pool type, the threshold value,
    // and the lock address.
    //

    PoolDescriptor->PoolType = PoolType;
    PoolDescriptor->PoolIndex = PoolIndex;
    PoolDescriptor->RunningAllocs = 0;
    PoolDescriptor->RunningDeAllocs = 0;
    PoolDescriptor->TotalPages = 0;
    PoolDescriptor->TotalBytes = 0;
    PoolDescriptor->TotalBigPages = 0;
    PoolDescriptor->Threshold = Threshold;
    PoolDescriptor->LockAddress = PoolLock;

    PoolDescriptor->PendingFrees = NULL;
    PoolDescriptor->PendingFreeDepth = 0;

    //
    // Initialize the allocation listheads.
    //

    ListEntry = PoolDescriptor->ListHeads;
    LastListEntry = ListEntry + POOL_LIST_HEADS;

    while (ListEntry < LastListEntry) {
        PrivateInitializeListHead (ListEntry);
        ListEntry += 1;
    }

    if (PoolType == PagedPoolSession) {
            
        if (ExpSessionPoolDescriptor == NULL) {
            ExpSessionPoolDescriptor = (PPOOL_DESCRIPTOR) MiSessionPoolVector ();
            ExpSessionPoolLookaside = MiSessionPoolLookaside ();

            ExpSessionPoolTrackTable = (PPOOL_TRACKER_TABLE) MiSessionPoolTrackTable ();
            ExpSessionPoolTrackTableSize = MiSessionPoolTrackTableSize ();
            ExpSessionPoolTrackTableMask = ExpSessionPoolTrackTableSize - 1;

            ExpSessionPoolBigPageTable = (PPOOL_TRACKER_BIG_PAGES) MiSessionPoolBigPageTable ();
            ExpSessionPoolBigPageTableSize = MiSessionPoolBigPageTableSize ();
            ExpSessionPoolBigPageTableHash = ExpSessionPoolBigPageTableSize - 1;
            ExpSessionPoolSmallLists = MiSessionPoolSmallLists ();
        }

        p = &ExpSessionPoolBigPageTable[0];
        pend = p + ExpSessionPoolBigPageTableSize;

        while (p < pend) {
            p->Va = (PVOID) POOL_BIG_TABLE_ENTRY_FREE;
            p += 1;
        }
    }

    return;
}

PVOID
ExpDummyAllocate (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

VOID
ExDrainPoolLookasideList (
    IN PPAGED_LOOKASIDE_LIST Lookaside
    )

/*++

Routine Description:

    This function drains the entries from the specified lookaside list.

    This is needed before deleting a pool lookaside list because the
    entries on the lookaside are already marked as free (by ExFreePoolWithTag)
    and so the normal lookaside deletion macros would hit false double free
    bugchecks if the list is not empty when the macros are called.

Arguments:

    Lookaside - Supplies a pointer to a lookaside list structure.

Return Value:

    None.

--*/

{
    PVOID Entry;
    PPOOL_HEADER PoolHeader;

    //
    // Remove all pool entries from the specified lookaside structure,
    // mark them as active, then free them.
    //

    Lookaside->L.Allocate = ExpDummyAllocate;

    while ((Entry = ExAllocateFromPagedLookasideList(Lookaside)) != NULL) {

        PoolHeader = (PPOOL_HEADER)Entry - 1;

        PoolHeader->PoolType = (USHORT)(Lookaside->L.Type + 1);
        PoolHeader->PoolType |= POOL_IN_USE_MASK;

        ExpInsertPoolTracker (PoolHeader->PoolTag,
                              PoolHeader->BlockSize << POOL_BLOCK_SHIFT,
                              Lookaside->L.Type);

        //
        // Set the depth to zero every time as a periodic scan may set it
        // nonzero.  This isn't worth interlocking as the list will absolutely
        // deplete regardless in this fashion anyway.
        //

        Lookaside->L.Depth = 0;

        (Lookaside->L.Free)(Entry);
    }

    return;
}

//
// FREE_CHECK_ERESOURCE - If enabled causes each free pool to verify
// no active ERESOURCEs are in the pool block being freed.
//
// FREE_CHECK_KTIMER - If enabled causes each free pool to verify no
// active KTIMERs are in the pool block being freed.
//

//
// Checking for resources in pool being freed is expensive as there can
// easily be thousands of resources, so don't do it by default but do
// leave the capability for individual systems to enable it.
//

//
// Runtime modifications to these flags must use interlocked sequences.
//

#if DBG
ULONG ExpPoolFlags = EX_CHECK_POOL_FREES_FOR_ACTIVE_TIMERS | \
                     EX_CHECK_POOL_FREES_FOR_ACTIVE_WORKERS;
#else
ULONG ExpPoolFlags = 0;
#endif

#define FREE_CHECK_ERESOURCE(Va, NumberOfBytes)                             \
            if (ExpPoolFlags & EX_CHECK_POOL_FREES_FOR_ACTIVE_RESOURCES) {  \
                ExpCheckForResource(Va, NumberOfBytes);                     \
            }

#define FREE_CHECK_KTIMER(Va, NumberOfBytes)                                \
            if (ExpPoolFlags & EX_CHECK_POOL_FREES_FOR_ACTIVE_TIMERS) {     \
                KeCheckForTimer(Va, NumberOfBytes);                         \
            }

#define FREE_CHECK_WORKER(Va, NumberOfBytes)                                \
            if (ExpPoolFlags & EX_CHECK_POOL_FREES_FOR_ACTIVE_WORKERS) {    \
                ExpCheckForWorker(Va, NumberOfBytes);                       \
            }


VOID
ExSetPoolFlags (
    IN ULONG PoolFlag
    )

/*++

Routine Description:

    This procedure enables the specified pool flag(s).

Arguments:

    PoolFlag - Supplies the pool flag(s) to enable.

Return Value:

    None.

--*/
{
    RtlInterlockedSetBits (&ExpPoolFlags, PoolFlag);
}


VOID
InitializePool (
    IN POOL_TYPE PoolType,
    IN ULONG Threshold
    )

/*++

Routine Description:

    This procedure initializes a pool descriptor for the specified pool
    type.  Once initialized, the pool may be used for allocation and
    deallocation.

    This function should be called once for each base pool type during
    system initialization.

    Each pool descriptor contains an array of list heads for free
    blocks.  Each list head holds blocks which are a multiple of
    the POOL_BLOCK_SIZE.  The first element on the list [0] links
    together free entries of size POOL_BLOCK_SIZE, the second element
    [1] links together entries of POOL_BLOCK_SIZE * 2, the third
    POOL_BLOCK_SIZE * 3, etc, up to the number of blocks which fit
    into a page.

Arguments:

    PoolType - Supplies the type of pool being initialized (e.g.
               nonpaged pool, paged pool...).

    Threshold - Supplies the threshold value for the specified pool.

Return Value:

    None.

--*/

{
    ULONG i;
    PKSPIN_LOCK SpinLock;
    PPOOL_TRACKER_BIG_PAGES p;
    PPOOL_DESCRIPTOR Descriptor;
    ULONG Index;
    PKGUARDED_MUTEX GuardedMutex;
    SIZE_T NumberOfBytes;

    ASSERT((PoolType & MUST_SUCCEED_POOL_TYPE_MASK) == 0);

    if (PoolType == NonPagedPool) {

        //
        // Initialize nonpaged pools.
        //
        // Ensure PoolTrackTableSize is a power of 2, then add 1 to it.
        //
        // Ensure PoolBigPageTableSize is a power of 2.
        //

        NumberOfBytes = PoolTrackTableSize;
        if (NumberOfBytes > MmSizeOfNonPagedPoolInBytes >> 8) {
            NumberOfBytes = MmSizeOfNonPagedPoolInBytes >> 8;
        }

        for (i = 0; i < 32; i += 1) {
            if (NumberOfBytes & 0x1) {
                ASSERT ((NumberOfBytes & ~0x1) == 0);
                if ((NumberOfBytes & ~0x1) == 0) {
                    break;
                }
            }
            NumberOfBytes >>= 1;
        }

        if (i == 32) {
            PoolTrackTableSize = DEFAULT_TRACKER_TABLE;
        }
        else {
            PoolTrackTableSize = ((SIZE_T)1) << i;
            if (PoolTrackTableSize < 64) {
                PoolTrackTableSize = 64;
            }
        }

        do {
            if (PoolTrackTableSize + 1 > (MAXULONG_PTR / sizeof(POOL_TRACKER_TABLE))) {
                PoolTrackTableSize >>= 1;
                continue;
            }

            PoolTrackTable = MiAllocatePoolPages (NonPagedPool,
                                                  (PoolTrackTableSize + 1) *
                                                    sizeof(POOL_TRACKER_TABLE));

            if (PoolTrackTable != NULL) {
                break;
            }

            if (PoolTrackTableSize == 1) {
                KeBugCheckEx (MUST_SUCCEED_POOL_EMPTY,
                              NumberOfBytes,
                              (ULONG_PTR)-1,
                              (ULONG_PTR)-1,
                              (ULONG_PTR)-1);
            }

            PoolTrackTableSize >>= 1;

        } while (TRUE);

        PoolTrackTableSize += 1;
        PoolTrackTableMask = PoolTrackTableSize - 2;

#if !defined (NT_UP)
        ExPoolTagTables[0] = PoolTrackTable;
#endif

        RtlZeroMemory (PoolTrackTable,
                       PoolTrackTableSize * sizeof(POOL_TRACKER_TABLE));

        ExpSeedHotTags ();

        //
        // Initialize the large allocation tag table.
        //

        NumberOfBytes = PoolBigPageTableSize;
        if (NumberOfBytes > MmSizeOfNonPagedPoolInBytes >> 8) {
            NumberOfBytes = MmSizeOfNonPagedPoolInBytes >> 8;
        }

        for (i = 0; i < 32; i += 1) {
            if (NumberOfBytes & 0x1) {
                ASSERT ((NumberOfBytes & ~0x1) == 0);
                if ((NumberOfBytes & ~0x1) == 0) {
                    break;
                }
            }
            NumberOfBytes >>= 1;
        }

        if (i == 32) {
            PoolBigPageTableSize = DEFAULT_BIGPAGE_TABLE;
        }
        else {
            PoolBigPageTableSize = ((SIZE_T)1) << i;
            if (PoolBigPageTableSize < 64) {
                PoolBigPageTableSize = 64;
            }
        }

        do {
            if (PoolBigPageTableSize > (MAXULONG_PTR / sizeof(POOL_TRACKER_BIG_PAGES))) {
                PoolBigPageTableSize >>= 1;
                continue;
            }

            PoolBigPageTable = MiAllocatePoolPages (NonPagedPool,
                                                PoolBigPageTableSize *
                                                sizeof(POOL_TRACKER_BIG_PAGES));

            if (PoolBigPageTable != NULL) {
                break;
            }

            if (PoolBigPageTableSize == 1) {
                KeBugCheckEx (MUST_SUCCEED_POOL_EMPTY,
                              NumberOfBytes,
                              (ULONG_PTR)-1,
                              (ULONG_PTR)-1,
                              (ULONG_PTR)-1);
            }

            PoolBigPageTableSize >>= 1;

        } while (TRUE);

        PoolBigPageTableHash = PoolBigPageTableSize - 1;

        RtlZeroMemory (PoolBigPageTable,
                       PoolBigPageTableSize * sizeof(POOL_TRACKER_BIG_PAGES));

        p = &PoolBigPageTable[0];
        for (i = 0; i < PoolBigPageTableSize; i += 1, p += 1) {
            p->Va = (PVOID) POOL_BIG_TABLE_ENTRY_FREE;
        }

        ExpInsertPoolTracker ('looP',
                              ROUND_TO_PAGES(PoolBigPageTableSize * sizeof(POOL_TRACKER_BIG_PAGES)),
                              NonPagedPool);

        if (KeNumberNodes > 1) {

            ExpNumberOfNonPagedPools = KeNumberNodes;

            //
            // Limit the number of pools to the number of bits in the PoolIndex.
            //

            if (ExpNumberOfNonPagedPools > 127) {
                ExpNumberOfNonPagedPools = 127;
            }

            //
            // Further limit the number of pools by our array of pointers.
            //

            if (ExpNumberOfNonPagedPools > EXP_MAXIMUM_POOL_NODES) {
                ExpNumberOfNonPagedPools = EXP_MAXIMUM_POOL_NODES;
            }

            NumberOfBytes = sizeof(POOL_DESCRIPTOR) + sizeof(KLOCK_QUEUE_HANDLE);

            for (Index = 0; Index < ExpNumberOfNonPagedPools; Index += 1) {

                //
                // Here's a thorny problem.  We'd like to use
                // MmAllocateIndependentPages but can't because we'd need
                // system PTEs to map the pages with and PTEs are not
                // available until nonpaged pool exists.  So just use
                // regular pool pages to hold the descriptors and spinlocks
                // and hope they either a) happen to fall onto the right node 
                // or b) that these lines live in the local processor cache
                // all the time anyway due to frequent usage.
                //

                Descriptor = (PPOOL_DESCRIPTOR) MiAllocatePoolPages (
                                                         NonPagedPool,
                                                         NumberOfBytes);

                if (Descriptor == NULL) {
                    KeBugCheckEx (MUST_SUCCEED_POOL_EMPTY,
                                  NumberOfBytes,
                                  (ULONG_PTR)-1,
                                  (ULONG_PTR)-1,
                                  (ULONG_PTR)-1);
                }

                ExpNonPagedPoolDescriptor[Index] = Descriptor;

                SpinLock = (PKSPIN_LOCK)(Descriptor + 1);

                KeInitializeSpinLock (SpinLock);

                ExInitializePoolDescriptor (Descriptor,
                                            NonPagedPool,
                                            Index,
                                            Threshold,
                                            (PVOID)SpinLock);
            }
        }

        //
        // Initialize the spinlocks for nonpaged pool.
        //

        KeInitializeSpinLock (&ExpTaggedPoolLock);

        //
        // Initialize the nonpaged pool descriptor.
        //

        PoolVector[NonPagedPool] = &NonPagedPoolDescriptor;
        ExInitializePoolDescriptor (&NonPagedPoolDescriptor,
                                    NonPagedPool,
                                    0,
                                    Threshold,
                                    NULL);
    }
    else {

        //
        // Allocate memory for the paged pool descriptors and fast mutexes.
        //

        if (KeNumberNodes > 1) {

            ExpNumberOfPagedPools = KeNumberNodes;

            //
            // Limit the number of pools to the number of bits in the PoolIndex.
            //

            if (ExpNumberOfPagedPools > 127) {
                ExpNumberOfPagedPools = 127;
            }
        }

        //
        // Further limit the number of pools by our array of pointers.
        //

        if (ExpNumberOfPagedPools > EXP_MAXIMUM_POOL_NODES) {
            ExpNumberOfPagedPools = EXP_MAXIMUM_POOL_NODES;
        }

        //
        // For NUMA systems, allocate both the pool descriptor and the
        // associated lock from the local node for performance (even though
        // it costs a little more memory).
        //
        // For non-NUMA systems, allocate everything together in one chunk
        // to reduce memory consumption as there is no performance cost
        // for doing it this way.
        //

        if (KeNumberNodes > 1) {

            NumberOfBytes = sizeof(KGUARDED_MUTEX) + sizeof(POOL_DESCRIPTOR);

            for (Index = 0; Index < ExpNumberOfPagedPools + 1; Index += 1) {

                ULONG Node;

                if (Index == 0) {
                    Node = 0;
                }
                else {
                    Node = Index - 1;
                }

                Descriptor = (PPOOL_DESCRIPTOR) MmAllocateIndependentPages (
                                                                      NumberOfBytes,
                                                                      Node);
                if (Descriptor == NULL) {
                    KeBugCheckEx (MUST_SUCCEED_POOL_EMPTY,
                                  NumberOfBytes,
                                  (ULONG_PTR)-1,
                                  (ULONG_PTR)-1,
                                  (ULONG_PTR)-1);
                }
                ExpPagedPoolDescriptor[Index] = Descriptor;

                GuardedMutex = (PKGUARDED_MUTEX)(Descriptor + 1);

                if (Index == 0) {
                    PoolVector[PagedPool] = Descriptor;
                    ExpPagedPoolMutex = GuardedMutex;
                }

                KeInitializeGuardedMutex (GuardedMutex);

                ExInitializePoolDescriptor (Descriptor,
                                            PagedPool,
                                            Index,
                                            Threshold,
                                            (PVOID) GuardedMutex);
            }
        }
        else {

            NumberOfBytes = (ExpNumberOfPagedPools + 1) * (sizeof(KGUARDED_MUTEX) + sizeof(POOL_DESCRIPTOR));

            Descriptor = (PPOOL_DESCRIPTOR)ExAllocatePoolWithTag (NonPagedPool,
                                                                  NumberOfBytes,
                                                                  'looP');
            if (Descriptor == NULL) {
                KeBugCheckEx (MUST_SUCCEED_POOL_EMPTY,
                              NumberOfBytes,
                              (ULONG_PTR)-1,
                              (ULONG_PTR)-1,
                              (ULONG_PTR)-1);
            }

            GuardedMutex = (PKGUARDED_MUTEX)(Descriptor + ExpNumberOfPagedPools + 1);

            PoolVector[PagedPool] = Descriptor;
            ExpPagedPoolMutex = GuardedMutex;

            for (Index = 0; Index < ExpNumberOfPagedPools + 1; Index += 1) {
                KeInitializeGuardedMutex (GuardedMutex);
                ExpPagedPoolDescriptor[Index] = Descriptor;
                ExInitializePoolDescriptor (Descriptor,
                                            PagedPool,
                                            Index,
                                            Threshold,
                                            (PVOID) GuardedMutex);

                Descriptor += 1;
                GuardedMutex += 1;
            }
        }

        ExpInsertPoolTracker('looP',
                              ROUND_TO_PAGES(PoolTrackTableSize * sizeof(POOL_TRACKER_TABLE)),
                             NonPagedPool);

#if defined (NT_UP)
        if (MmNumberOfPhysicalPages < 32 * 1024) {

            LARGE_INTEGER TwoMinutes;

            //
            // Set the flag to disable lookasides and use hot/cold page
            // separation during bootup.
            //

            ExSetPoolFlags (EX_SEPARATE_HOT_PAGES_DURING_BOOT);

            //
            // Start a timer so the above behavior is disabled once bootup
            // has finished.
            //

            KeInitializeTimer (&ExpBootFinishedTimer);

            KeInitializeDpc (&ExpBootFinishedTimerDpc,
                             (PKDEFERRED_ROUTINE) ExpBootFinishedDispatch,
                             NULL);

            TwoMinutes.QuadPart = Int32x32To64 (120, -10000000);

            KeSetTimer (&ExpBootFinishedTimer,
                        TwoMinutes,
                        &ExpBootFinishedTimerDpc);
        }
#endif
        if ((MmNumberOfPhysicalPages >= 127 * 1024) &&
            ((ExpPoolFlags & EX_SPECIAL_POOL_ENABLED) == 0) &&
            (!NT_SUCCESS (MmIsVerifierEnabled (&i)))) {

            ExSetPoolFlags (EX_DELAY_POOL_FREES);
        }

        if ((ExpPoolFlags & EX_SPECIAL_POOL_ENABLED) ||
            (NT_SUCCESS (MmIsVerifierEnabled (&i)))) {

#if DBG

            //
            // Ensure ExFreePoolMask is a power of 2 minus 1 (or zero).
            //

            if (ExFreePoolMask != 0) {

                NumberOfBytes = ExFreePoolMask + 1;
                ASSERT (NumberOfBytes != 0);

                for (i = 0; i < 32; i += 1) {
                    if (NumberOfBytes & 0x1) {
                        ASSERT ((NumberOfBytes & ~0x1) == 0);
                        break;
                    }
                    NumberOfBytes >>= 1;
                }
            }

#endif

            ExFreePoolTraces = MiAllocatePoolPages (NonPagedPool,
                                                (ExFreePoolMask + 1) *
                                                sizeof (EX_FREE_POOL_TRACES));

            if (ExFreePoolTraces != NULL) {
                RtlZeroMemory (ExFreePoolTraces,
                           (ExFreePoolMask + 1) * sizeof (EX_FREE_POOL_TRACES));
            }
        }
    }
}

#if DBG
ULONG ExStopBadTags;
#endif


FORCEINLINE
VOID
ExpInsertPoolTrackerInline (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    )

/*++

Routine Description:

    This function inserts a pool tag in the tag table, increments the
    number of allocates and updates the total allocation size.

Arguments:

    Key - Supplies the key value used to locate a matching entry in the
          tag table.

    NumberOfBytes - Supplies the allocation size.

    PoolType - Supplies the pool type.

Return Value:

    None.

Environment:

    No pool locks held except during the rare case of expansion table growth.
    so pool may be freely allocated here as needed.  In expansion table growth,
    the tagged spinlock is held on entry, but we are guaranteed to find an
    entry in the builtin table so a recursive acquire cannot occur.

--*/

{
    ULONG Hash;
    ULONG Index;
    LONG OriginalKey;
    KIRQL OldIrql;
    PPOOL_TRACKER_TABLE TrackTable;
    PPOOL_TRACKER_TABLE TrackTableEntry;
    SIZE_T TrackTableMask;
    SIZE_T TrackTableSize;
#if !defined (NT_UP)
    ULONG Processor;
#endif

    //
    // Strip the protected pool bit.
    //

    Key &= ~PROTECTED_POOL;

    if (Key == PoolHitTag) {
        DbgBreakPoint();
    }

#if DBG
    if (ExStopBadTags) {
        ASSERT (Key & 0xFFFFFF00);
    }
#endif

    //
    // Compute the hash index and search (lock-free) for the pool tag
    // in the builtin table.
    //

    if (PoolType & SESSION_POOL_MASK) {
        TrackTable = ExpSessionPoolTrackTable;
        TrackTableMask = ExpSessionPoolTrackTableMask;
        TrackTableSize = ExpSessionPoolTrackTableSize;
    }
    else {

#if !defined (NT_UP)

        //
        // Use the current processor to pick a pool tag table to use.  Note that
        // in rare cases, this thread may context switch to another processor
        // but the algorithms below will still be correct.
        //

        Processor = KeGetCurrentProcessorNumber ();

        ASSERT (Processor < MAXIMUM_PROCESSOR_TAG_TABLES);

        TrackTable = ExPoolTagTables[Processor];

#else

        TrackTable = PoolTrackTable;

#endif

        TrackTableMask = PoolTrackTableMask;
        TrackTableSize = PoolTrackTableSize;
    }

    Hash = POOLTAG_HASH (Key, TrackTableMask);

    Index = Hash;

    do {

        TrackTableEntry = &TrackTable[Hash];

        if (TrackTableEntry->Key == Key) {

            //
            // Update the fields with interlocked operations as other
            // threads may also have begun doing so by this point.
            //

            if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
                InterlockedIncrement ((PLONG) &TrackTableEntry->PagedAllocs);
                InterlockedExchangeAddSizeT (&TrackTableEntry->PagedBytes,
                                             NumberOfBytes);
            }
            else {
                InterlockedIncrement ((PLONG) &TrackTableEntry->NonPagedAllocs);
                InterlockedExchangeAddSizeT (&TrackTableEntry->NonPagedBytes,
                                             NumberOfBytes);

            }

            return;
        }

        if (TrackTableEntry->Key == 0) {

            if (PoolType & SESSION_POOL_MASK) {

                if (Hash == TrackTableSize - 1) {
                    Hash = 0;
                    if (Hash == Index) {
                        break;
                    }
                }
                else {

                    OriginalKey = InterlockedCompareExchange ((PLONG)&TrackTable[Hash].Key,
                                                              (LONG)Key,
                                                              0);
                }

                //
                // Either this thread has won the race and the requested tag
                // is now in or some other thread won the race and took this
                // slot (using this tag or a different one).
                //
                // Just fall through to common checks starting at this slot
                // for both cases.
                //

                continue;
            }

#if !defined (NT_UP)

            if (PoolTrackTable[Hash].Key != 0) {
                TrackTableEntry->Key = PoolTrackTable[Hash].Key;
                continue;
            }

#endif

            if (Hash != PoolTrackTableSize - 1) {

                //
                // New entries cannot be created with an interlocked compare
                // exchange because any new entry must reside at the same index
                // in each processor's private PoolTrackTable.  This is to make
                // ExGetPoolTagInfo statistics gathering much simpler (faster).
                //

                ExAcquireSpinLock (&ExpTaggedPoolLock, &OldIrql);

                if (PoolTrackTable[Hash].Key == 0) {

                    ASSERT (TrackTable[Hash].Key == 0);

                    PoolTrackTable[Hash].Key = Key;
                    TrackTableEntry->Key = Key;
                }

                ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);

                //
                // Either this thread has won the race and the requested tag
                // is now in or some other thread won the race and took this
                // slot (using this tag or a different one).
                //
                // Just fall through to common checks starting at this slot
                // for both cases.
                //

                continue;
            }
        }

        Hash = (Hash + 1) & (ULONG)TrackTableMask;

        if (Hash == Index) {
            break;
        }

    } while (TRUE);

    //
    // No matching entry and no free entry was found.
    //
    // Use the expansion table instead.
    //

    ExpInsertPoolTrackerExpansion (Key, NumberOfBytes, PoolType);
}

FORCEINLINE
VOID
ExpRemovePoolTrackerInline (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    )

/*++

Routine Description:

    This function increments the number of frees and updates the total
    allocation size.

Arguments:

    Key - Supplies the key value used to locate a matching entry in the
          tag table.

    NumberOfBytes - Supplies the allocation size.

    PoolType - Supplies the pool type.

Return Value:

    None.

--*/

{
    ULONG Hash;
    ULONG Index;
    PPOOL_TRACKER_TABLE TrackTable;
    PPOOL_TRACKER_TABLE TrackTableEntry;
    SIZE_T TrackTableMask;
    SIZE_T TrackTableSize;
#if !defined (NT_UP)
    ULONG Processor;
#endif

    //
    // Strip protected pool bit.
    //

    Key &= ~PROTECTED_POOL;
    if (Key == PoolHitTag) {
        DbgBreakPoint ();
    }

    //
    // Compute the hash index and search (lock-free) for the pool tag
    // in the builtin table.
    //

    if (PoolType & SESSION_POOL_MASK) {
        TrackTable = ExpSessionPoolTrackTable;
        TrackTableMask = ExpSessionPoolTrackTableMask;
        TrackTableSize = ExpSessionPoolTrackTableSize;
    }
    else {

#if !defined (NT_UP)

        //
        // Use the current processor to pick a pool tag table to use.  Note that
        // in rare cases, this thread may context switch to another processor
        // but the algorithms below will still be correct.
        //

        Processor = KeGetCurrentProcessorNumber ();

        ASSERT (Processor < MAXIMUM_PROCESSOR_TAG_TABLES);

        TrackTable = ExPoolTagTables[Processor];

#else

        TrackTable = PoolTrackTable;

#endif

        TrackTableMask = PoolTrackTableMask;
        TrackTableSize = PoolTrackTableSize;
    }

    Hash = POOLTAG_HASH (Key, TrackTableMask);

    Index = Hash;

    do {
        TrackTableEntry = &TrackTable[Hash];

        if (TrackTableEntry->Key == Key) {

            if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
                InterlockedIncrement ((PLONG) &TrackTableEntry->PagedFrees);
                InterlockedExchangeAddSizeT (&TrackTableEntry->PagedBytes,
                                             0 - NumberOfBytes);
            }
            else {
                InterlockedIncrement ((PLONG) &TrackTableEntry->NonPagedFrees);
                InterlockedExchangeAddSizeT (&TrackTableEntry->NonPagedBytes,
                                             0 - NumberOfBytes);
            }
            return;
        }

        //
        // Since each processor's table is lazy updated, handle the case
        // here where this processor's table still has no entry for the tag
        // being freed because the allocation happened on a different
        // processor.
        //

        if (TrackTableEntry->Key == 0) {

#if !defined (NT_UP)

            if (((PoolType & SESSION_POOL_MASK) == 0) &&
                (PoolTrackTable[Hash].Key != 0)) {

                TrackTableEntry->Key = PoolTrackTable[Hash].Key;
                continue;
            }

#endif

            ASSERT (Hash == TrackTableMask);
        }

        Hash = (Hash + 1) & (ULONG)TrackTableMask;

        if (Hash == Index) {
            break;
        }

    } while (TRUE);

    //
    // No matching entry and no free entry was found.
    //
    // Linear search through the expansion table.  This is ok because
    // the existence of an expansion table at all is extremely rare.
    //

    ExpRemovePoolTrackerExpansion (Key, NumberOfBytes, PoolType);
}
PVOID
VeAllocatePoolWithTagPriority (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN EX_POOL_PRIORITY Priority,
    IN PVOID CallingAddress
    );


__bcount(NumberOfBytes) 
PVOID
ExAllocatePoolWithTag (
    __in POOL_TYPE PoolType,
    __in SIZE_T NumberOfBytes,
    __in ULONG Tag
    )

/*++

Routine Description:

    This function allocates a block of pool of the specified type and
    returns a pointer to the allocated block. This function is used to
    access both the page-aligned pools and the list head entries (less
    than a page) pools.

    If the number of bytes specifies a size that is too large to be
    satisfied by the appropriate list, then the page-aligned pool
    allocator is used. The allocated block will be page-aligned and a
    page-sized multiple.

    Otherwise, the appropriate pool list entry is used. The allocated
    block will be 64-bit aligned, but will not be page aligned. The
    pool allocator calculates the smallest number of POOL_BLOCK_SIZE
    that can be used to satisfy the request. If there are no blocks
    available of this size, then a block of the next larger block size
    is allocated and split. One piece is placed back into the pool, and
    the other piece is used to satisfy the request. If the allocator
    reaches the paged-sized block list, and nothing is there, the
    page-aligned pool allocator is called. The page is split and added
    to the pool.

Arguments:

    PoolType - Supplies the type of pool to allocate. If the pool type
        is one of the "MustSucceed" pool types, then this call will
        succeed and return a pointer to allocated pool or bugcheck on failure.
        For all other cases, if the system cannot allocate the requested amount
        of memory, NULL is returned.

        Valid pool types:

        NonPagedPool
        PagedPool
        NonPagedPoolMustSucceed,
        NonPagedPoolCacheAligned
        PagedPoolCacheAligned
        NonPagedPoolCacheAlignedMustSucceed

    Tag - Supplies the caller's identifying tag.

    NumberOfBytes - Supplies the number of bytes to allocate.

Return Value:

    NULL - The PoolType is not one of the "MustSucceed" pool types, and
           not enough pool exists to satisfy the request.

    NON-NULL - Returns a pointer to the allocated pool.

--*/

{
    PKGUARDED_MUTEX Lock;
    PVOID Block;
    PPOOL_HEADER Entry;
    PGENERAL_LOOKASIDE LookasideList;
    PPOOL_HEADER NextEntry;
    PPOOL_HEADER SplitEntry;
    KLOCK_QUEUE_HANDLE LockHandle;
    PPOOL_DESCRIPTOR PoolDesc;
    ULONG Index;
    ULONG ListNumber;
    ULONG NeededSize;
    ULONG PoolIndex;
    POOL_TYPE CheckType;
    POOL_TYPE RequestType;
    PLIST_ENTRY ListHead;
    POOL_TYPE NewPoolType;
    PKPRCB Prcb;
    ULONG NumberOfPages;
    ULONG RetryCount;
    PVOID CallingAddress;
#if defined (_X86_)
    PVOID CallersCaller;
#endif

#define CacheOverhead POOL_OVERHEAD

    PERFINFO_EXALLOCATEPOOLWITHTAG_DECL();

    ASSERT (Tag != 0);
    ASSERT (Tag != ' GIB');
    ASSERT (NumberOfBytes != 0);
    ASSERT_ALLOCATE_IRQL (PoolType, NumberOfBytes);

    if (ExpPoolFlags & (EX_KERNEL_VERIFIER_ENABLED | EX_SPECIAL_POOL_ENABLED)) {

        if (ExpPoolFlags & EX_KERNEL_VERIFIER_ENABLED) {

            if ((PoolType & POOL_DRIVER_MASK) == 0) {

                //
                // Use the Driver Verifier pool framework.  Note this will
                // result in a recursive callback to this routine.
                //

#if defined (_X86_)
                RtlGetCallersAddress (&CallingAddress, &CallersCaller);
#else
                CallingAddress = (PVOID)_ReturnAddress();
#endif

                return VeAllocatePoolWithTagPriority (PoolType | POOL_DRIVER_MASK,
                                                  NumberOfBytes,
                                                  Tag,
                                                  HighPoolPriority,
                                                  CallingAddress);
            }
            PoolType &= ~POOL_DRIVER_MASK;
        }

        //
        // Use special pool if there is a tag or size match.
        //

        if ((ExpPoolFlags & EX_SPECIAL_POOL_ENABLED) &&
            (MmUseSpecialPool (NumberOfBytes, Tag))) {

            Entry = MmAllocateSpecialPool (NumberOfBytes,
                                           Tag,
                                           PoolType,
                                           2);
            if (Entry != NULL) {
                return (PVOID)Entry;
            }
        }
    }

    //
    // Isolate the base pool type and select a pool from which to allocate
    // the specified block size.
    //

    CheckType = PoolType & BASE_POOL_TYPE_MASK;

    if ((PoolType & SESSION_POOL_MASK) == 0) {
        PoolDesc = PoolVector[CheckType];
    }
    else {
        PoolDesc = ExpSessionPoolDescriptor;
    }

    ASSERT (PoolDesc != NULL);

    //
    // Initializing LockHandle is not needed for correctness but without
    // it the compiler cannot compile this code W4 to check for use of
    // uninitialized variables.
    //

    LockHandle.OldIrql = 0;

    //
    // Check to determine if the requested block can be allocated from one
    // of the pool lists or must be directly allocated from virtual memory.
    //

    if (NumberOfBytes > POOL_BUDDY_MAX) {

        //
        // The requested size is greater than the largest block maintained
        // by allocation lists.
        //

        RequestType = (PoolType & (BASE_POOL_TYPE_MASK | SESSION_POOL_MASK | POOL_VERIFIER_MASK));

        Entry = (PPOOL_HEADER) MiAllocatePoolPages (RequestType,
                                                    NumberOfBytes);

        if (Entry == NULL) {

            //
            // If there are deferred free blocks, free them now and retry.
            //

            if (ExpPoolFlags & EX_DELAY_POOL_FREES) {

                ExDeferredFreePool (PoolDesc);

                Entry = (PPOOL_HEADER) MiAllocatePoolPages (RequestType,
                                                            NumberOfBytes);
            }
        }

        if (Entry == NULL) {

            if (PoolType & MUST_SUCCEED_POOL_TYPE_MASK) {
                KeBugCheckEx (MUST_SUCCEED_POOL_EMPTY,
                              NumberOfBytes,
                              NonPagedPoolDescriptor.TotalPages,
                              NonPagedPoolDescriptor.TotalBigPages,
                              0);
            }

            ExPoolFailures += 1;

            if (ExpPoolFlags & EX_PRINT_POOL_FAILURES) {
                KdPrint(("EX: ExAllocatePool (%p, 0x%x) returning NULL\n",
                    NumberOfBytes,
                    PoolType));
                if (ExpPoolFlags & EX_STOP_ON_POOL_FAILURES) {
                    DbgBreakPoint ();
                }
            }

            if ((PoolType & POOL_RAISE_IF_ALLOCATION_FAILURE) != 0) {
                ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
            }

            return NULL;
        }

        NumberOfPages = (ULONG) BYTES_TO_PAGES (NumberOfBytes);

        InterlockedExchangeAdd ((PLONG)&PoolDesc->TotalBigPages,
                                (LONG)NumberOfPages);

        InterlockedExchangeAddSizeT (&PoolDesc->TotalBytes,
                                     (SIZE_T)NumberOfPages << PAGE_SHIFT);

        InterlockedIncrement ((PLONG)&PoolDesc->RunningAllocs);

        //
        // Mark the allocation as session-based so that when it is freed
        // we can detect that the session pool descriptor is the one to
        // be credited (not the global nonpaged descriptor).
        //

        if ((PoolType & SESSION_POOL_MASK) && (CheckType == NonPagedPool)) {
            MiMarkPoolLargeSession (Entry);
        }

        //
        // Note nonpaged session allocations get turned into global
        // session allocations internally, so they must be added to the
        // global tag tables.  Paged session allocations go into their
        // own tables.
        //

        if (ExpAddTagForBigPages ((PVOID)Entry,
                                  Tag,
                                  NumberOfPages,
                                  PoolType) == FALSE) {

            //
            // Note that not being able to add the tag entry above
            // implies 2 things: The allocation must now be tagged
            // as BIG because the subsequent free also won't find it
            // in the big page tag table and so it must use BIG when
            // removing it from the PoolTrackTable.  Also that the free
            // must get the size from MiFreePoolPages since the
            // big page tag table won't have the size in this case.
            //

            Tag = ' GIB';
        }

        ExpInsertPoolTracker (Tag,
                              ROUND_TO_PAGES(NumberOfBytes),
                              PoolType);

        PERFINFO_BIGPOOLALLOC (PoolType, Tag, NumberOfBytes, Entry);

        return Entry;
    }

    if (NumberOfBytes == 0) {

        //
        // Besides fragmenting pool, zero byte requests would not be handled
        // in cases where the minimum pool block size is the same as the 
        // pool header size (no room for flink/blinks, etc).
        //

#if DBG
        KeBugCheckEx (BAD_POOL_CALLER, 0, 0, PoolType, Tag);
#else
        NumberOfBytes = 1;
#endif
    }

    //
    // The requested size is less than or equal to the size of the
    // maximum block maintained by the allocation lists.
    //

    PERFINFO_POOLALLOC (PoolType, Tag, NumberOfBytes);

    //
    // Compute the index of the listhead for blocks of the requested size.
    //

    ListNumber = (ULONG)((NumberOfBytes + POOL_OVERHEAD + (POOL_SMALLEST_BLOCK - 1)) >> POOL_BLOCK_SHIFT);

    NeededSize = ListNumber;

    if (CheckType == PagedPool) {

        //
        // If the requested pool block is a small block, then attempt to
        // allocate the requested pool from the per processor lookaside
        // list. If the attempt fails, then attempt to allocate from the
        // system lookaside list. If the attempt fails, then select a
        // pool to allocate from and allocate the block normally.
        //
        // Also note that if hot/cold separation is enabled, allocations are
        // not satisfied from lookaside lists as these are either :
        //
        // 1. cold references
        //
        // or
        //
        // 2. we are still booting on a small machine, thus keeping pool
        //    locality dense (to reduce the working set footprint thereby
        //    reducing page stealing) is a bigger win in terms of overall
        //    speed than trying to satisfy individual requests more quickly.
        //

        if ((PoolType & SESSION_POOL_MASK) == 0) {

            //
            // Check for prototype pool - always allocate it from its own
            // pages as the sharecounts applied on these allocations by
            // memory management make it more difficult to trim these pages.
            // This is an optimization so that other pageable allocation pages
            // (which are much easier to trim because their sharecount is
            // almost always only 1) don't end up being mostly resident because
            // of a single prototype pool allocation within in.  Note this
            // also makes it easier to remove specific pages for hot remove
            // or callers that need contiguous physical memory.
            //

            if (PoolType & POOL_MM_ALLOCATION) {
                PoolIndex = 0;
                ASSERT (PoolDesc->PoolIndex == 0);
                goto restart1;
            }

            if ((NeededSize <= POOL_SMALL_LISTS) &&
                (USING_HOT_COLD_METRICS == 0)) {

                Prcb = KeGetCurrentPrcb ();
                LookasideList = Prcb->PPPagedLookasideList[NeededSize - 1].P;
                LookasideList->TotalAllocates += 1;

                Entry = (PPOOL_HEADER)
                    InterlockedPopEntrySList (&LookasideList->ListHead);

                if (Entry == NULL) {
                    LookasideList = Prcb->PPPagedLookasideList[NeededSize - 1].L;
                    LookasideList->TotalAllocates += 1;

                    Entry = (PPOOL_HEADER)
                        InterlockedPopEntrySList (&LookasideList->ListHead);
                }

                if (Entry != NULL) {

                    Entry -= 1;
                    LookasideList->AllocateHits += 1;
                    NewPoolType = (PoolType & (BASE_POOL_TYPE_MASK | POOL_QUOTA_MASK | SESSION_POOL_MASK | POOL_VERIFIER_MASK)) + 1;
                    NewPoolType |= POOL_IN_USE_MASK;

                    Entry->PoolType = (UCHAR)NewPoolType;

                    Entry->PoolTag = Tag;

                    ExpInsertPoolTrackerInline (Tag,
                                                Entry->BlockSize << POOL_BLOCK_SHIFT,
                                                PoolType);

                    //
                    // Zero out any back pointer to our internal structures
                    // to stop someone from corrupting us via an
                    // uninitialized pointer.
                    //

                    ((PULONG_PTR)((PCHAR)Entry + CacheOverhead))[0] = 0;

                    PERFINFO_POOLALLOC_ADDR((PUCHAR)Entry + CacheOverhead);

                    return (PUCHAR)Entry + CacheOverhead;
                }
            }

            //
            // If there is more than one paged pool, then attempt to find
            // one that can be immediately locked.
            //
            //
            // N.B. The paged pool is selected in a round robin fashion using a
            //      simple counter.  Note that the counter is incremented using
            //      a a noninterlocked sequence, but the pool index is never
            //      allowed to get out of range.
            //

            if (USING_HOT_COLD_METRICS)  {

                if ((PoolType & POOL_COLD_ALLOCATION) == 0) {

                    //
                    // Hot allocations come from the first paged pool.
                    //

                    PoolIndex = 1;
                }
                else {

                    //
                    // Force cold allocations to come from
                    // the last paged pool.
                    //

                    PoolIndex = ExpNumberOfPagedPools;
                }
            }
            else {

                if (KeNumberNodes > 1) {

                    //
                    // Use the pool descriptor which contains memory
                    // local to the current processor even if we have to
                    // wait for it.  While it is possible that the
                    // paged pool addresses in the local descriptor
                    // have been paged out, on large memory
                    // NUMA machines this should be less common.
                    //

                    Prcb = KeGetCurrentPrcb ();

                    PoolIndex = Prcb->ParentNode->Color;

                    if (PoolIndex < ExpNumberOfPagedPools) {
                        PoolIndex += 1;
                        PoolDesc = ExpPagedPoolDescriptor[PoolIndex];
                        RequestType = PoolType & (BASE_POOL_TYPE_MASK | SESSION_POOL_MASK);
                        RetryCount = 0;
                        goto restart2;
                    }
                }

                PoolIndex = 1;
                if (ExpNumberOfPagedPools != PoolIndex) {
                    ExpPoolIndex += 1;
                    PoolIndex = ExpPoolIndex;
                    if (PoolIndex > ExpNumberOfPagedPools) {
                        PoolIndex = 1;
                        ExpPoolIndex = 1;
                    }

                    Index = PoolIndex;
                    do {
                        Lock = (PKGUARDED_MUTEX) ExpPagedPoolDescriptor[PoolIndex]->LockAddress;

                        if (KeGetOwnerGuardedMutex (Lock) == NULL) {
                            break;
                        }

                        PoolIndex += 1;
                        if (PoolIndex > ExpNumberOfPagedPools) {
                            PoolIndex = 1;
                        }

                    } while (PoolIndex != Index);
                }
            }

            PoolDesc = ExpPagedPoolDescriptor[PoolIndex];
        }
        else {

            if (NeededSize <= ExpSessionPoolSmallLists) {

                LookasideList = (PGENERAL_LOOKASIDE)(ULONG_PTR)(ExpSessionPoolLookaside + NeededSize - 1);
                LookasideList->TotalAllocates += 1;

                Entry = (PPOOL_HEADER)
                    InterlockedPopEntrySList (&LookasideList->ListHead);

                if (Entry != NULL) {

                    Entry -= 1;
                    LookasideList->AllocateHits += 1;
                    NewPoolType = (PoolType & (BASE_POOL_TYPE_MASK | POOL_QUOTA_MASK | SESSION_POOL_MASK | POOL_VERIFIER_MASK)) + 1;
                    NewPoolType |= POOL_IN_USE_MASK;

                    Entry->PoolType = (UCHAR)NewPoolType;

                    Entry->PoolTag = Tag;

                    ExpInsertPoolTrackerInline (Tag,
                                                Entry->BlockSize << POOL_BLOCK_SHIFT,
                                                PoolType);

                    //
                    // Zero out any back pointer to our internal structures
                    // to stop someone from corrupting us via an
                    // uninitialized pointer.
                    //

                    ((PULONG_PTR)((PCHAR)Entry + CacheOverhead))[0] = 0;

                    PERFINFO_POOLALLOC_ADDR((PUCHAR)Entry + CacheOverhead);

                    return (PUCHAR)Entry + CacheOverhead;
                }
            }

            //
            // Only one paged pool is available per session.
            //

            PoolIndex = 0;
            ASSERT (PoolDesc == ExpSessionPoolDescriptor);
            ASSERT (PoolDesc->PoolIndex == 0);
        }
    }
    else {

        //
        // If the requested pool block is a small block, then attempt to
        // allocate the requested pool from the per processor lookaside
        // list. If the attempt fails, then attempt to allocate from the
        // system lookaside list. If the attempt fails, then select a
        // pool to allocate from and allocate the block normally.
        //
        // Only session paged pool allocations come from the per session pools.
        // Nonpaged session pool allocations still come from global pool.
        //

        if (NeededSize <= POOL_SMALL_LISTS) {

            Prcb = KeGetCurrentPrcb ();
            LookasideList = Prcb->PPNPagedLookasideList[NeededSize - 1].P;
            LookasideList->TotalAllocates += 1;

            Entry = (PPOOL_HEADER)
                        InterlockedPopEntrySList (&LookasideList->ListHead);

            if (Entry == NULL) {
                LookasideList = Prcb->PPNPagedLookasideList[NeededSize - 1].L;
                LookasideList->TotalAllocates += 1;

                Entry = (PPOOL_HEADER)
                        InterlockedPopEntrySList (&LookasideList->ListHead);
            }

            if (Entry != NULL) {

                Entry -= 1;
                LookasideList->AllocateHits += 1;
                NewPoolType = (PoolType & (BASE_POOL_TYPE_MASK | POOL_QUOTA_MASK | SESSION_POOL_MASK | POOL_VERIFIER_MASK)) + 1;
                NewPoolType |= POOL_IN_USE_MASK;

                Entry->PoolType = (UCHAR)NewPoolType;

                Entry->PoolTag = Tag;

                ExpInsertPoolTrackerInline (Tag,
                                            Entry->BlockSize << POOL_BLOCK_SHIFT,
                                            PoolType);

                //
                // Zero out any back pointer to our internal structures
                // to stop someone from corrupting us via an
                // uninitialized pointer.
                //

                ((PULONG_PTR)((PCHAR)Entry + CacheOverhead))[0] = 0;

                PERFINFO_POOLALLOC_ADDR((PUCHAR)Entry + CacheOverhead);

                return (PUCHAR)Entry + CacheOverhead;
            }
        }

        if (PoolType & SESSION_POOL_MASK) {
            PoolDesc = PoolVector[CheckType];
        }

        if (ExpNumberOfNonPagedPools <= 1) {
            PoolIndex = 0;
        }
        else {

            //
            // Use the pool descriptor which contains memory local to
            // the current processor even if we have to contend for its lock.
            //

            Prcb = KeGetCurrentPrcb ();

            PoolIndex = Prcb->ParentNode->Color;

            if (PoolIndex >= ExpNumberOfNonPagedPools) {
                PoolIndex = ExpNumberOfNonPagedPools - 1;
            }

            PoolDesc = ExpNonPagedPoolDescriptor[PoolIndex];
        }

        ASSERT(PoolIndex == PoolDesc->PoolIndex);
    }

restart1:

    RequestType = PoolType & (BASE_POOL_TYPE_MASK | SESSION_POOL_MASK);
    RetryCount = 0;

restart2:

    ListHead = &PoolDesc->ListHeads[ListNumber];

    //
    // Walk the listheads looking for a free block.
    //

    do {

        //
        // If the list is not empty, then allocate a block from the
        // selected list.
        //

        if (PrivateIsListEmpty (ListHead) == FALSE) {

            LOCK_POOL (PoolDesc, LockHandle);

            if (PrivateIsListEmpty (ListHead)) {

                //
                // The block is no longer available, march on.
                //

                UNLOCK_POOL (PoolDesc, LockHandle);
                ListHead += 1;
                continue;
            }

            CHECK_LIST (ListHead);
            Block = PrivateRemoveHeadList (ListHead);
            CHECK_LIST (ListHead);
            Entry = (PPOOL_HEADER)((PCHAR)Block - POOL_OVERHEAD);

            CHECK_POOL_PAGE (Entry);

            ASSERT(Entry->BlockSize >= NeededSize);

            ASSERT(DECODE_POOL_INDEX(Entry) == PoolIndex);

            ASSERT(Entry->PoolType == 0);

            if (Entry->BlockSize != NeededSize) {

                //
                // The selected block is larger than the allocation
                // request. Split the block and insert the remaining
                // fragment in the appropriate list.
                //
                // If the entry is at the start of a page, then take
                // the allocation from the front of the block so as
                // to minimize fragmentation. Otherwise, take the
                // allocation from the end of the block which may
                // also reduce fragmentation if the block is at the
                // end of a page.
                //

                if (Entry->PreviousSize == 0) {

                    //
                    // The entry is at the start of a page.
                    //

                    SplitEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + NeededSize);
                    SplitEntry->BlockSize = (USHORT)(Entry->BlockSize - NeededSize);
                    SplitEntry->PreviousSize = (USHORT) NeededSize;

                    //
                    // If the allocated block is not at the end of a
                    // page, then adjust the size of the next block.
                    //

                    NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)SplitEntry + SplitEntry->BlockSize);
                    if (PAGE_END(NextEntry) == FALSE) {
                        NextEntry->PreviousSize = SplitEntry->BlockSize;
                    }

                }
                else {

                    //
                    // The entry is not at the start of a page.
                    //

                    SplitEntry = Entry;
                    Entry->BlockSize = (USHORT)(Entry->BlockSize - NeededSize);
                    Entry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + Entry->BlockSize);
                    Entry->PreviousSize = SplitEntry->BlockSize;

                    //
                    // If the allocated block is not at the end of a
                    // page, then adjust the size of the next block.
                    //

                    NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + NeededSize);
                    if (PAGE_END(NextEntry) == FALSE) {
                        NextEntry->PreviousSize = (USHORT) NeededSize;
                    }
                }

                //
                // Set the size of the allocated entry, clear the pool
                // type of the split entry, set the index of the split
                // entry, and insert the split entry in the appropriate
                // free list.
                //

                Entry->BlockSize = (USHORT) NeededSize;
                ENCODE_POOL_INDEX(Entry, PoolIndex);
                SplitEntry->PoolType = 0;
                ENCODE_POOL_INDEX(SplitEntry, PoolIndex);
                Index = SplitEntry->BlockSize;

                CHECK_LIST(&PoolDesc->ListHeads[Index - 1]);

                //
                // Only insert split pool blocks which contain more than just
                // a header as only those have room for a flink/blink !
                // Note if the minimum pool block size is bigger than the
                // header then there can be no blocks like this.
                //

                if ((POOL_OVERHEAD != POOL_SMALLEST_BLOCK) ||
                    (SplitEntry->BlockSize != 1)) {

                    PrivateInsertTailList(&PoolDesc->ListHeads[Index - 1], ((PLIST_ENTRY)((PCHAR)SplitEntry + POOL_OVERHEAD)));

                    CHECK_LIST(((PLIST_ENTRY)((PCHAR)SplitEntry + POOL_OVERHEAD)));
                }
            }

            Entry->PoolType = (UCHAR)(((PoolType & (BASE_POOL_TYPE_MASK | POOL_QUOTA_MASK | SESSION_POOL_MASK | POOL_VERIFIER_MASK)) + 1) | POOL_IN_USE_MASK);

            CHECK_POOL_PAGE (Entry);

            UNLOCK_POOL(PoolDesc, LockHandle);

            InterlockedIncrement ((PLONG)&PoolDesc->RunningAllocs);

            InterlockedExchangeAddSizeT (&PoolDesc->TotalBytes,
                                         Entry->BlockSize << POOL_BLOCK_SHIFT);

            Entry->PoolTag = Tag;

            ExpInsertPoolTrackerInline (Tag,
                                        Entry->BlockSize << POOL_BLOCK_SHIFT,
                                        PoolType);

            //
            // Zero out any back pointer to our internal structures
            // to stop someone from corrupting us via an
            // uninitialized pointer.
            //

            ((PULONGLONG)((PCHAR)Entry + CacheOverhead))[0] = 0;

            PERFINFO_POOLALLOC_ADDR((PUCHAR)Entry + CacheOverhead);
            return (PCHAR)Entry + CacheOverhead;
        }

        ListHead += 1;

    } while (ListHead != &PoolDesc->ListHeads[POOL_LIST_HEADS]);

    //
    // A block of the desired size does not exist and there are
    // no large blocks that can be split to satisfy the allocation.
    // Attempt to expand the pool by allocating another page and
    // adding it to the pool.
    //

    Entry = (PPOOL_HEADER) MiAllocatePoolPages (RequestType, PAGE_SIZE);

    if (Entry == NULL) {

        //
        // If there are deferred free blocks, free them now and retry.
        //

        RetryCount += 1;

        if ((RetryCount == 1) && (ExpPoolFlags & EX_DELAY_POOL_FREES)) {
            ExDeferredFreePool (PoolDesc);
            goto restart2;
        }

        if ((PoolType & MUST_SUCCEED_POOL_TYPE_MASK) != 0) {

            //
            // Must succeed pool was requested so bugcheck.
            //

            KeBugCheckEx (MUST_SUCCEED_POOL_EMPTY,
                          PAGE_SIZE,
                          NonPagedPoolDescriptor.TotalPages,
                          NonPagedPoolDescriptor.TotalBigPages,
                          0);
        }

        //
        // No more pool of the specified type is available.
        //

        ExPoolFailures += 1;

        if (ExpPoolFlags & EX_PRINT_POOL_FAILURES) {
            KdPrint(("EX: ExAllocatePool (%p, 0x%x) returning NULL\n",
                NumberOfBytes,
                PoolType));
            if (ExpPoolFlags & EX_STOP_ON_POOL_FAILURES) {
                DbgBreakPoint ();
            }
        }

        if ((PoolType & POOL_RAISE_IF_ALLOCATION_FAILURE) != 0) {
            ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
        }

        PERFINFO_POOLALLOC_ADDR (NULL);
        return NULL;
    }

    //
    // Initialize the pool header for the new allocation.
    //

    Entry->Ulong1 = 0;
    Entry->PoolIndex = (UCHAR) PoolIndex;
    Entry->BlockSize = (USHORT) NeededSize;

    Entry->PoolType = (UCHAR)(((PoolType & (BASE_POOL_TYPE_MASK | POOL_QUOTA_MASK | SESSION_POOL_MASK | POOL_VERIFIER_MASK)) + 1) | POOL_IN_USE_MASK);


    SplitEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + NeededSize);

    SplitEntry->Ulong1 = 0;

    Index = (PAGE_SIZE / sizeof(POOL_BLOCK)) - NeededSize;

    SplitEntry->BlockSize = (USHORT) Index;
    SplitEntry->PreviousSize = (USHORT) NeededSize;
    SplitEntry->PoolIndex = (UCHAR) PoolIndex;

    //
    // Split the allocated page and insert the remaining
    // fragment in the appropriate listhead.
    //
    // Set the size of the allocated entry, clear the pool
    // type of the split entry, set the index of the split
    // entry, and insert the split entry in the appropriate
    // free list.
    //

    //
    // Note that if the request was for nonpaged session pool, we are
    // not updating the session pool descriptor for this.  Instead we
    // are deliberately updating the global nonpaged pool descriptor
    // because the rest of the fragment goes into global nonpaged pool.
    // This is ok because the session pool descriptor TotalPages count
    // is not relied upon.
    //
    // The individual pool tracking by tag, however, is critical and
    // is properly maintained below (ie: session allocations are charged
    // to the session tracking table and regular nonpaged allocations are
    // charged to the global nonpaged tracking table).
    //

    InterlockedIncrement ((PLONG)&PoolDesc->TotalPages);

    NeededSize <<= POOL_BLOCK_SHIFT;

    InterlockedExchangeAddSizeT (&PoolDesc->TotalBytes, NeededSize);

    PERFINFO_ADDPOOLPAGE(CheckType, PoolIndex, Entry, PoolDesc);

    //
    // Only insert split pool blocks which contain more than just
    // a header as only those have room for a flink/blink !
    // Note if the minimum pool block size is bigger than the
    // header then there can be no blocks like this.
    //

    if ((POOL_OVERHEAD != POOL_SMALLEST_BLOCK) ||
        (SplitEntry->BlockSize != 1)) {

        //
        // Now lock the pool and insert the fragment.
        //

        LOCK_POOL (PoolDesc, LockHandle);

        CHECK_LIST(&PoolDesc->ListHeads[Index - 1]);

        PrivateInsertTailList(&PoolDesc->ListHeads[Index - 1], ((PLIST_ENTRY)((PCHAR)SplitEntry + POOL_OVERHEAD)));

        CHECK_LIST(((PLIST_ENTRY)((PCHAR)SplitEntry + POOL_OVERHEAD)));

        CHECK_POOL_PAGE (Entry);

        UNLOCK_POOL (PoolDesc, LockHandle);
    }
    else {
        CHECK_POOL_PAGE (Entry);
    }

    InterlockedIncrement ((PLONG)&PoolDesc->RunningAllocs);

    Block = (PVOID) ((PCHAR)Entry + CacheOverhead);

    Entry->PoolTag = Tag;

    ExpInsertPoolTrackerInline (Tag, NeededSize, PoolType);

    PERFINFO_POOLALLOC_ADDR (Block);

    return Block;
}


__bcount(NumberOfBytes) 
PVOID
ExAllocatePool (
    __in POOL_TYPE PoolType,
    __in SIZE_T NumberOfBytes
    )

/*++

Routine Description:

    This function allocates a block of pool of the specified type and
    returns a pointer to the allocated block.  This function is used to
    access both the page-aligned pools, and the list head entries (less than
    a page) pools.

    If the number of bytes specifies a size that is too large to be
    satisfied by the appropriate list, then the page-aligned
    pool allocator is used.  The allocated block will be page-aligned
    and a page-sized multiple.

    Otherwise, the appropriate pool list entry is used.  The allocated
    block will be 64-bit aligned, but will not be page aligned.  The
    pool allocator calculates the smallest number of POOL_BLOCK_SIZE
    that can be used to satisfy the request.  If there are no blocks
    available of this size, then a block of the next larger block size
    is allocated and split.  One piece is placed back into the pool, and
    the other piece is used to satisfy the request.  If the allocator
    reaches the paged-sized block list, and nothing is there, the
    page-aligned pool allocator is called.  The page is split and added
    to the pool...

Arguments:

    PoolType - Supplies the type of pool to allocate.  If the pool type
        is one of the "MustSucceed" pool types, then this call will
        succeed and return a pointer to allocated pool or bugcheck on failure.
        For all other cases, if the system cannot allocate the requested amount
        of memory, NULL is returned.

        Valid pool types:

        NonPagedPool
        PagedPool
        NonPagedPoolMustSucceed,
        NonPagedPoolCacheAligned
        PagedPoolCacheAligned
        NonPagedPoolCacheAlignedMustS

    NumberOfBytes - Supplies the number of bytes to allocate.

Return Value:

    NULL - The PoolType is not one of the "MustSucceed" pool types, and
           not enough pool exists to satisfy the request.

    NON-NULL - Returns a pointer to the allocated pool.

--*/

{
    return ExAllocatePoolWithTag (PoolType,
                                  NumberOfBytes,
                                  'enoN');
}


__bcount(NumberOfBytes) 
PVOID
ExAllocatePoolWithTagPriority (
    __in POOL_TYPE PoolType,
    __in SIZE_T NumberOfBytes,
    __in ULONG Tag,
    __in EX_POOL_PRIORITY Priority
    )

/*++

Routine Description:

    This function allocates a block of pool of the specified type and
    returns a pointer to the allocated block.  This function is used to
    access both the page-aligned pools, and the list head entries (less than
    a page) pools.

    If the number of bytes specifies a size that is too large to be
    satisfied by the appropriate list, then the page-aligned
    pool allocator is used.  The allocated block will be page-aligned
    and a page-sized multiple.

    Otherwise, the appropriate pool list entry is used.  The allocated
    block will be 64-bit aligned, but will not be page aligned.  The
    pool allocator calculates the smallest number of POOL_BLOCK_SIZE
    that can be used to satisfy the request.  If there are no blocks
    available of this size, then a block of the next larger block size
    is allocated and split.  One piece is placed back into the pool, and
    the other piece is used to satisfy the request.  If the allocator
    reaches the paged-sized block list, and nothing is there, the
    page-aligned pool allocator is called.  The page is split and added
    to the pool...

Arguments:

    PoolType - Supplies the type of pool to allocate.  If the pool type
        is one of the "MustSucceed" pool types, then this call will
        succeed and return a pointer to allocated pool or bugcheck on failure.
        For all other cases, if the system cannot allocate the requested amount
        of memory, NULL is returned.

        Valid pool types:

        NonPagedPool
        PagedPool
        NonPagedPoolMustSucceed,
        NonPagedPoolCacheAligned
        PagedPoolCacheAligned
        NonPagedPoolCacheAlignedMustS

    NumberOfBytes - Supplies the number of bytes to allocate.

    Tag - Supplies the caller's identifying tag.

    Priority - Supplies an indication as to how important it is that this
               request succeed under low available pool conditions.  This
               can also be used to specify special pool.

Return Value:

    NULL - The PoolType is not one of the "MustSucceed" pool types, and
           not enough pool exists to satisfy the request.

    NON-NULL - Returns a pointer to the allocated pool.

--*/

{
    ULONG i;
    ULONG Ratio;
    PVOID Entry;
    SIZE_T TotalBytes;
    SIZE_T TotalFullPages;
    POOL_TYPE CheckType;
    PPOOL_DESCRIPTOR PoolDesc;

    if ((Priority & POOL_SPECIAL_POOL_BIT) && (NumberOfBytes <= POOL_BUDDY_MAX)) {
        Entry = MmAllocateSpecialPool (NumberOfBytes,
                                       Tag,
                                       PoolType,
                                       (Priority & POOL_SPECIAL_POOL_UNDERRUN_BIT) ? 1 : 0);

        if (Entry != NULL) {
            return Entry;
        }
        Priority &= ~(POOL_SPECIAL_POOL_BIT | POOL_SPECIAL_POOL_UNDERRUN_BIT);
    }

    //
    // Pool and other resources can be allocated directly through the Mm
    // without the pool code knowing - so always call the Mm for the
    // up-to-date counters.
    //

    if ((Priority != HighPoolPriority) &&
        ((PoolType & MUST_SUCCEED_POOL_TYPE_MASK) == 0)) {

        if (MmResourcesAvailable (PoolType, NumberOfBytes, Priority) == FALSE) {

            //
            // The Mm does not have very many full pages left.  Leave those
            // for true high priority callers.  But first see if this request
            // is small, and if so, if there is a lot of fragmentation, then
            // it is likely the request can be satisfied from pre-existing
            // fragments.
            //

            if (NumberOfBytes > POOL_BUDDY_MAX) {
                return NULL;
            }

            //
            // Sum the pool descriptors.
            //

            CheckType = PoolType & BASE_POOL_TYPE_MASK;

            if ((CheckType == NonPagedPool) ||
                ((PoolType & SESSION_POOL_MASK) == 0)) {

                PoolDesc = PoolVector[CheckType];

                TotalBytes = 0;
                TotalFullPages = 0;

                if (CheckType == PagedPool) {

                    if (KeNumberNodes > 1) {
                        for (i = 0; i <= ExpNumberOfPagedPools; i += 1) {
                            PoolDesc = ExpPagedPoolDescriptor[i];
                            TotalFullPages += PoolDesc->TotalPages;
                            TotalFullPages += PoolDesc->TotalBigPages;
                            TotalBytes += PoolDesc->TotalBytes;
                        }
                    }
                    else {
                        for (i = 0; i <= ExpNumberOfPagedPools; i += 1) {
                            TotalFullPages += PoolDesc->TotalPages;
                            TotalFullPages += PoolDesc->TotalBigPages;
                            TotalBytes += PoolDesc->TotalBytes;
                            PoolDesc += 1;
                        }
                    }
                }
                else {
                    if (ExpNumberOfNonPagedPools == 1) {
                        TotalFullPages += PoolDesc->TotalPages;
                        TotalFullPages += PoolDesc->TotalBigPages;
                        TotalBytes += PoolDesc->TotalBytes;
                    }
                    else {
                        for (i = 0; i < ExpNumberOfNonPagedPools; i += 1) {
                            PoolDesc = ExpNonPagedPoolDescriptor[i];
                            TotalFullPages += PoolDesc->TotalPages;
                            TotalFullPages += PoolDesc->TotalBigPages;
                            TotalBytes += PoolDesc->TotalBytes;
                        }
                    }
                }
            }
            else {
                PoolDesc = ExpSessionPoolDescriptor;
                TotalFullPages = PoolDesc->TotalPages;
                TotalFullPages += PoolDesc->TotalBigPages;
                TotalBytes = PoolDesc->TotalBytes;
            }

            //
            // If the pages are more than 80% populated then don't assume
            // we're going to be able to satisfy ths request via a fragment.
            //

            TotalFullPages |= 1;        // Ensure we never divide by zero.
            TotalBytes >>= PAGE_SHIFT;

            //
            // The additions above were performed lock free so we must handle
            // slicing which can cause nonexact sums.
            //

            if (TotalBytes > TotalFullPages) {
                TotalBytes = TotalFullPages;
            }

            Ratio = (ULONG)((TotalBytes * 100) / TotalFullPages);

            if (Ratio >= 80) {
                return NULL;
            }
        }
    }

    //
    // There is a window between determining whether to proceed and actually
    // doing the allocation.  In this window the pool may deplete.  This is not
    // worth closing at this time.
    //

    return ExAllocatePoolWithTag (PoolType, NumberOfBytes, Tag);
}


__bcount(NumberOfBytes) 
PVOID
ExAllocatePoolWithQuota (
    __in POOL_TYPE PoolType,
    __in SIZE_T NumberOfBytes
    )

/*++

Routine Description:

    This function allocates a block of pool of the specified type,
    returns a pointer to the allocated block, and if the binary buddy
    allocator was used to satisfy the request, charges pool quota to the
    current process.  This function is used to access both the
    page-aligned pools, and the binary buddy.

    If the number of bytes specifies a size that is too large to be
    satisfied by the appropriate binary buddy pool, then the
    page-aligned pool allocator is used.  The allocated block will be
    page-aligned and a page-sized multiple.  No quota is charged to the
    current process if this is the case.

    Otherwise, the appropriate binary buddy pool is used.  The allocated
    block will be 64-bit aligned, but will not be page aligned.  After
    the allocation completes, an attempt will be made to charge pool
    quota (of the appropriate type) to the current process object.  If
    the quota charge succeeds, then the pool block's header is adjusted
    to point to the current process.  The process object is not
    dereferenced until the pool is deallocated and the appropriate
    amount of quota is returned to the process.  Otherwise, the pool is
    deallocated, a "quota exceeded" condition is raised.

Arguments:

    PoolType - Supplies the type of pool to allocate.  If the pool type
        is one of the "MustSucceed" pool types and sufficient quota
        exists, then this call will always succeed and return a pointer
        to allocated pool.  Otherwise, if the system cannot allocate
        the requested amount of memory a STATUS_INSUFFICIENT_RESOURCES
        status is raised.

    NumberOfBytes - Supplies the number of bytes to allocate.

Return Value:

    NON-NULL - Returns a pointer to the allocated pool.

    Unspecified - If insufficient quota exists to complete the pool
        allocation, the return value is unspecified.

--*/

{
    return ExAllocatePoolWithQuotaTag (PoolType, NumberOfBytes, 'enoN');
}

FORCEINLINE
PEPROCESS
ExpGetBilledProcess (
    IN PPOOL_HEADER Entry
    )
{
    PEPROCESS ProcessBilled;

    if ((Entry->PoolType & POOL_QUOTA_MASK) == 0) {
        return NULL;
    }

#if defined(_WIN64)
    ProcessBilled = Entry->ProcessBilled;
#else
    ProcessBilled =  * (PVOID *)((PCHAR)Entry + (Entry->BlockSize << POOL_BLOCK_SHIFT) - sizeof (PVOID));
#endif

    if (ProcessBilled != NULL) {
        if (((PKPROCESS)(ProcessBilled))->Header.Type != ProcessObject) {
            KeBugCheckEx (BAD_POOL_CALLER,
                          0xD,
                          (ULONG_PTR)(Entry + 1),
                          Entry->PoolTag,
                          (ULONG_PTR)ProcessBilled);
        }
    }

    return ProcessBilled;
}

__bcount(NumberOfBytes) 
PVOID
ExAllocatePoolWithQuotaTag (
    __in POOL_TYPE PoolType,
    __in SIZE_T NumberOfBytes,
    __in ULONG Tag
    )

/*++

Routine Description:

    This function allocates a block of pool of the specified type,
    returns a pointer to the allocated block, and if the binary buddy
    allocator was used to satisfy the request, charges pool quota to the
    current process.  This function is used to access both the
    page-aligned pools, and the binary buddy.

    If the number of bytes specifies a size that is too large to be
    satisfied by the appropriate binary buddy pool, then the
    page-aligned pool allocator is used.  The allocated block will be
    page-aligned and a page-sized multiple.  No quota is charged to the
    current process if this is the case.

    Otherwise, the appropriate binary buddy pool is used.  The allocated
    block will be 64-bit aligned, but will not be page aligned.  After
    the allocation completes, an attempt will be made to charge pool
    quota (of the appropriate type) to the current process object.  If
    the quota charge succeeds, then the pool block's header is adjusted
    to point to the current process.  The process object is not
    dereferenced until the pool is deallocated and the appropriate
    amount of quota is returned to the process.  Otherwise, the pool is
    deallocated, a "quota exceeded" condition is raised.

Arguments:

    PoolType - Supplies the type of pool to allocate.  If the pool type
        is one of the "MustSucceed" pool types and sufficient quota
        exists, then this call will always succeed and return a pointer
        to allocated pool.  Otherwise, if the system cannot allocate
        the requested amount of memory a STATUS_INSUFFICIENT_RESOURCES
        status is raised.

    NumberOfBytes - Supplies the number of bytes to allocate.

Return Value:

    NON-NULL - Returns a pointer to the allocated pool.

    Unspecified - If insufficient quota exists to complete the pool
        allocation, the return value is unspecified.

--*/

{
    PVOID p;
    PEPROCESS Process;
    PPOOL_HEADER Entry;
    LOGICAL RaiseOnQuotaFailure;
    NTSTATUS Status;
#if DBG
    LONG ConcurrentQuotaPool;
#endif

    RaiseOnQuotaFailure = TRUE;

    if (PoolType & POOL_QUOTA_FAIL_INSTEAD_OF_RAISE) {
        RaiseOnQuotaFailure = FALSE;
        PoolType &= ~POOL_QUOTA_FAIL_INSTEAD_OF_RAISE;
    }

    PoolType = (POOL_TYPE)((UCHAR)PoolType + POOL_QUOTA_MASK);

    Process = PsGetCurrentProcess ();

#if !defined(_WIN64)

    //
    // Add in room for the quota pointer at the end of the caller's allocation.
    // Note for NT64, there is room in the pool header for both the tag and
    // the quota pointer so no extra space is needed at the end.
    //
    // Only add in the quota pointer if doing so won't cause us to spill the
    // allocation into a full page.
    //

    ASSERT (NumberOfBytes != 0);

    if (NumberOfBytes <= PAGE_SIZE - POOL_OVERHEAD - sizeof (PVOID)) {
        if (Process != PsInitialSystemProcess) {
            NumberOfBytes += sizeof (PVOID);
        }
        else {
            PoolType = (POOL_TYPE)((UCHAR)PoolType - POOL_QUOTA_MASK);
        }
    }
    else {

        //
        // Turn off the quota bit prior to allocating if we're not charging
        // as there's no room to put (or subsequently query for) a quota
        // pointer.
        //

        PoolType = (POOL_TYPE)((UCHAR)PoolType - POOL_QUOTA_MASK);
    }

#endif

    p = ExAllocatePoolWithTag (PoolType, NumberOfBytes, Tag);

    //
    // Note - NULL is page aligned.
    //

    if (!PAGE_ALIGNED(p)) {

        if ((ExpPoolFlags & EX_SPECIAL_POOL_ENABLED) &&
            (MmIsSpecialPoolAddress (p))) {
            return p;
        }

        Entry = (PPOOL_HEADER)((PCH)p - POOL_OVERHEAD);

#if defined(_WIN64)
        Entry->ProcessBilled = NULL;
#else
        if ((PoolType & POOL_QUOTA_MASK) == 0) {
            return p;
        }
#endif

        if (Process != PsInitialSystemProcess) {

            Status = PsChargeProcessPoolQuota (Process,
                                 PoolType & BASE_POOL_TYPE_MASK,
                                 (ULONG)(Entry->BlockSize << POOL_BLOCK_SHIFT));


            if (!NT_SUCCESS(Status)) {

                //
                // Back out the allocation.
                //

#if !defined(_WIN64)
                //
                // The quota flag cannot be blindly cleared in NT32 because
                // it's used to denote the allocation is larger (and the
                // verifier finds its own header based on this).
                //
                // Instead of clearing the flag above, instead zero the quota
                // pointer.
                //

                * (PVOID *)((PCHAR)Entry + (Entry->BlockSize << POOL_BLOCK_SHIFT) - sizeof (PVOID)) = NULL;
#endif

                ExFreePoolWithTag (p, Tag);

                if (RaiseOnQuotaFailure) {
                    ExRaiseStatus (Status);
                }
                return NULL;
            }

#if DBG
            ConcurrentQuotaPool = InterlockedIncrement (&ExConcurrentQuotaPool);
            if (ConcurrentQuotaPool > ExConcurrentQuotaPoolMax) {
                ExConcurrentQuotaPoolMax = ConcurrentQuotaPool;
            }
#endif

#if defined(_WIN64)
            Entry->ProcessBilled = Process;
#else

            * (PVOID *)((PCHAR)Entry + (Entry->BlockSize << POOL_BLOCK_SHIFT) - sizeof (PVOID)) = Process;

#endif

            ObReferenceObject (Process);
        }
    }
    else {
        if ((p == NULL) && (RaiseOnQuotaFailure)) {
            ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
        }
    }

    return p;
}

VOID
ExInsertPoolTag (
    ULONG Tag,
    PVOID Va,
    SIZE_T NumberOfBytes,
    POOL_TYPE PoolType
    )

/*++

Routine Description:

    This function inserts a pool tag in the tag table and increments the
    number of allocates and updates the total allocation size.

    This function also inserts the pool tag in the big page tag table.

    N.B. This function is for use by memory management ONLY.

Arguments:

    Tag - Supplies the tag used to insert an entry in the tag table.

    Va - Supplies the allocated virtual address.

    NumberOfBytes - Supplies the allocation size in bytes.

    PoolType - Supplies the pool type.

Return Value:

    None.

Environment:

    No pool locks held so pool may be freely allocated here as needed.

--*/

{
    ULONG NumberOfPages;

#if !DBG
    UNREFERENCED_PARAMETER (PoolType);
#endif

    ASSERT ((PoolType & SESSION_POOL_MASK) == 0);

    if (NumberOfBytes >= PAGE_SIZE) {

        NumberOfPages = (ULONG) BYTES_TO_PAGES (NumberOfBytes);

        if (ExpAddTagForBigPages((PVOID)Va, Tag, NumberOfPages, PoolType) == FALSE) {
            Tag = ' GIB';
        }
    }

    ExpInsertPoolTracker (Tag, NumberOfBytes, NonPagedPool);
}

VOID
ExpSeedHotTags (
    VOID
    )

/*++

Routine Description:

    This function seeds well-known hot tags into the pool tag tracking table
    when the table is first created.  The goal is to increase the likelihood
    that the hash generated for these tags always gets a direct hit.

Arguments:

    None.

Return Value:

    None.

Environment:

    INIT time, no locks held.

--*/

{
    ULONG i;
    ULONG Key;
    ULONG Hash;
    ULONG Index;
    PPOOL_TRACKER_TABLE TrackTable;

    ULONG KeyList[] = {
            '  oI',
            ' laH',
            'PldM',
            'LooP',
            'tSbO',
            ' prI',
            'bdDN',
            'LprI',
            'pOoI',
            ' ldM',
            'eliF',
            'aVMC',
            'dSeS',
            'CFtN',
            'looP',
            'rPCT',
            'bNMC',
            'dTeS',
            'sFtN',
            'TPCT',
            'CPCT',
            ' yeK',
            'qSbO',
            'mNoI',
            'aEoI',
            'cPCT',
            'aFtN',
            '0ftN',
            'tceS',
            'SprI',
            'ekoT',
            '  eS',
            'lCbO',
            'cScC',
            'lFtN',
            'cAeS',
            'mfSF',
            'kWcC',
            'miSF',
            'CdfA',
            'EdfA',
            'orSF',
            'nftN',
            'PRIU',

            'rFpN',
            'RFpN',
            'aPeS',
            'sUeS',
            'FpcA',
            'MpcA',
            'cSeS',
            'mNbO',
            'sFpN',
            'uLeS',
            'DPcS',
            'nevE',
            'vrqR',
            'ldaV',
            '  pP',
            'SdaV',
            ' daV',
            'LdaV',
            'FdaV',

            //
            // BIG is preseeded not because it is hot, but because allocations
            // with this tag must be inserted successfully (ie: cannot be
            // retagged into the Ovfl bucket) because we need a tag to account
            // for them in the PoolTrackTable counting when freeing the pool.
            //

            ' GIB',
    };

    TrackTable = PoolTrackTable;

    for (i = 0; i < sizeof (KeyList) / sizeof (ULONG); i += 1) {

        Key = KeyList[i];

        Hash = POOLTAG_HASH(Key,PoolTrackTableMask);

        Index = Hash;

        do {

            ASSERT (TrackTable[Hash].Key != Key);

            if ((TrackTable[Hash].Key == 0) &&
                (Hash != PoolTrackTableSize - 1)) {

                TrackTable[Hash].Key = Key;
                break;
            }

            ASSERT (TrackTable[Hash].Key != Key);

            Hash = (Hash + 1) & (ULONG)PoolTrackTableMask;

            if (Hash == Index) {
                break;
            }

        } while (TRUE);
    }
}


VOID
ExpInsertPoolTrackerExpansion (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    )

/*++

Routine Description:

    This function inserts a pool tag in the expansion tag table (taking a
    spinlock to do so), increments the number of allocates and updates
    the total allocation size.

Arguments:

    Key - Supplies the key value used to locate a matching entry in the
          tag table.

    NumberOfBytes - Supplies the allocation size.

    PoolType - Supplies the pool type.

Return Value:

    None.

Environment:

    No pool locks held so pool may be freely allocated here as needed.

    This routine is only called if ExpInsertPoolTracker encounters a full
    builtin list.

--*/

{
    ULONG Hash;
    KIRQL OldIrql;
    ULONG BigPages;
    SIZE_T NewSize;
    SIZE_T SizeInBytes;
    SIZE_T NewSizeInBytes;
    PPOOL_TRACKER_TABLE OldTable;
    PPOOL_TRACKER_TABLE NewTable;

    //
    // The protected pool bit has already been stripped.
    //

    ASSERT ((Key & PROTECTED_POOL) == 0);

    if (PoolType & SESSION_POOL_MASK) {

        //
        // Use the very last entry as a bit bucket for overflows.
        //

        NewTable = ExpSessionPoolTrackTable + ExpSessionPoolTrackTableSize - 1;

        ASSERT ((NewTable->Key == 0) || (NewTable->Key == 'lfvO'));

        NewTable->Key = 'lfvO';

        //
        // Update the fields with interlocked operations as other
        // threads may also have begun doing so by this point.
        //

        if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
            InterlockedIncrement ((PLONG) &NewTable->PagedAllocs);
            InterlockedExchangeAddSizeT (&NewTable->PagedBytes,
                                         NumberOfBytes);
        }
        else {
            InterlockedIncrement ((PLONG) &NewTable->NonPagedAllocs);
            InterlockedExchangeAddSizeT (&NewTable->NonPagedBytes,
                                         NumberOfBytes);
        }
        return;
    }

    //
    // Linear search through the expansion table.  This is ok because
    // the case of no free entries in the built-in table is extremely rare.
    //

    ExAcquireSpinLock (&ExpTaggedPoolLock, &OldIrql);

    for (Hash = 0; Hash < PoolTrackTableExpansionSize; Hash += 1) {

        if (PoolTrackTableExpansion[Hash].Key == Key) {
            break;
        }

        if (PoolTrackTableExpansion[Hash].Key == 0) {
            ASSERT (PoolTrackTable[PoolTrackTableSize - 1].Key == 0);
            PoolTrackTableExpansion[Hash].Key = Key;
            break;
        }
    }

    if (Hash != PoolTrackTableExpansionSize) {

        //
        // The entry was found (or created).  Update the other fields now.
        //

        if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
            PoolTrackTableExpansion[Hash].PagedAllocs += 1;
            PoolTrackTableExpansion[Hash].PagedBytes += NumberOfBytes;
        }
        else {
            PoolTrackTableExpansion[Hash].NonPagedAllocs += 1;
            PoolTrackTableExpansion[Hash].NonPagedBytes += NumberOfBytes;
        }

        ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);
        return;
    }

    //
    // The entry was not found and the expansion table is full (or nonexistent).
    // Try to allocate a larger expansion table now.
    //

    if (PoolTrackTable[PoolTrackTableSize - 1].Key != 0) {

        //
        // The overflow bucket has been used so expansion of the tracker table
        // is not allowed because a subsequent free of a tag can go negative
        // as the original allocation is in overflow and a newer allocation
        // may be distinct.
        //

        //
        // Use the very last entry as a bit bucket for overflows.
        //

        ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);

        Hash = (ULONG)PoolTrackTableSize - 1;

        //
        // Update the fields with interlocked operations as other
        // threads may also have begun doing so by this point.
        //

        if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
            InterlockedIncrement ((PLONG) &PoolTrackTable[Hash].PagedAllocs);
            InterlockedExchangeAddSizeT (&PoolTrackTable[Hash].PagedBytes,
                                         NumberOfBytes);
        }
        else {
            InterlockedIncrement ((PLONG) &PoolTrackTable[Hash].NonPagedAllocs);
            InterlockedExchangeAddSizeT (&PoolTrackTable[Hash].NonPagedBytes,
                                         NumberOfBytes);

        }

        return;
    }

    SizeInBytes = PoolTrackTableExpansionSize * sizeof(POOL_TRACKER_TABLE);

    //
    // Use as much of the slush in the final page as possible.
    //

    NewSizeInBytes = (PoolTrackTableExpansionPages + 1) << PAGE_SHIFT;
    NewSize = NewSizeInBytes / sizeof (POOL_TRACKER_TABLE);
    NewSizeInBytes = NewSize * sizeof(POOL_TRACKER_TABLE);

    NewTable = MiAllocatePoolPages (NonPagedPool, NewSizeInBytes);

    if (NewTable != NULL) {

        if (PoolTrackTableExpansion != NULL) {

            //
            // Copy all the existing entries into the new table.
            //

            RtlCopyMemory (NewTable,
                           PoolTrackTableExpansion,
                           SizeInBytes);
        }

        RtlZeroMemory ((PVOID)(NewTable + PoolTrackTableExpansionSize),
                       NewSizeInBytes - SizeInBytes);

        OldTable = PoolTrackTableExpansion;

        PoolTrackTableExpansion = NewTable;
        PoolTrackTableExpansionSize = NewSize;
        PoolTrackTableExpansionPages += 1;

        //
        // Recursively call ourself to insert the new table entry.  This entry
        // must be inserted before releasing the tagged spinlock because
        // another thread may be further growing the table and as soon as we
        // release the spinlock, that thread may grow and try to free our
        // new table !
        //

        ExpInsertPoolTracker ('looP',
                              PoolTrackTableExpansionPages << PAGE_SHIFT,
                              NonPagedPool);

        ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);

        //
        // Free the old table if there was one.
        //

        if (OldTable != NULL) {

            BigPages = MiFreePoolPages (OldTable);

            ExpRemovePoolTracker ('looP',
                                  (SIZE_T) BigPages * PAGE_SIZE,
                                  NonPagedPool);
        }

        //
        // Finally insert the caller's original allocation.
        //

        ExpInsertPoolTrackerExpansion (Key, NumberOfBytes, PoolType);
    }
    else {

        //
        // Use the very last entry as a bit bucket for overflows.
        //

        Hash = (ULONG)PoolTrackTableSize - 1;

        ASSERT (PoolTrackTable[Hash].Key == 0);

        PoolTrackTable[Hash].Key = 'lfvO';

        ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);

        //
        // Update the fields with interlocked operations as other
        // threads may also have begun doing so by this point.
        //

        if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
            InterlockedIncrement ((PLONG) &PoolTrackTable[Hash].PagedAllocs);
            InterlockedExchangeAddSizeT (&PoolTrackTable[Hash].PagedBytes,
                                         NumberOfBytes);
        }
        else {
            InterlockedIncrement ((PLONG) &PoolTrackTable[Hash].NonPagedAllocs);
            InterlockedExchangeAddSizeT (&PoolTrackTable[Hash].NonPagedBytes,
                                         NumberOfBytes);
        }
    }

    return;
}
VOID
ExpInsertPoolTracker (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    )
{
    ExpInsertPoolTrackerInline (Key, NumberOfBytes, PoolType);
}


VOID
ExpRemovePoolTrackerExpansion (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    )

/*++

Routine Description:

    This function increments the number of frees and updates the total
    allocation size in the expansion table.

Arguments:

    Key - Supplies the key value used to locate a matching entry in the
          tag table.

    NumberOfBytes - Supplies the allocation size.

    PoolType - Supplies the pool type.

Return Value:

    None.

--*/

{
    ULONG Hash;
    KIRQL OldIrql;
    PPOOL_TRACKER_TABLE TrackTable;
#if !defined (NT_UP)
    ULONG Processor;
#endif

    //
    // The protected pool bit has already been stripped.
    //

    ASSERT ((Key & PROTECTED_POOL) == 0);

    if (PoolType & SESSION_POOL_MASK) {

        //
        // This entry must have been charged to the overflow bucket.
        // Update the pool tracker table entry for it.
        //

        Hash = (ULONG)ExpSessionPoolTrackTableSize - 1;
        TrackTable = ExpSessionPoolTrackTable;
        goto OverflowEntry;
    }

    //
    // Linear search through the expansion table.  This is ok because
    // the existence of an expansion table at all is extremely rare.
    //

    ExAcquireSpinLock (&ExpTaggedPoolLock, &OldIrql);

    for (Hash = 0; Hash < PoolTrackTableExpansionSize; Hash += 1) {

        if (PoolTrackTableExpansion[Hash].Key == Key) {

            if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
                ASSERT (PoolTrackTableExpansion[Hash].PagedAllocs != 0);
                ASSERT (PoolTrackTableExpansion[Hash].PagedAllocs >=
                        PoolTrackTableExpansion[Hash].PagedFrees);
                ASSERT (PoolTrackTableExpansion[Hash].PagedBytes >= NumberOfBytes);
                PoolTrackTableExpansion[Hash].PagedFrees += 1;
                PoolTrackTableExpansion[Hash].PagedBytes -= NumberOfBytes;
            }
            else {
                ASSERT (PoolTrackTableExpansion[Hash].NonPagedAllocs != 0);
                ASSERT (PoolTrackTableExpansion[Hash].NonPagedAllocs >=
                        PoolTrackTableExpansion[Hash].NonPagedFrees);
                ASSERT (PoolTrackTableExpansion[Hash].NonPagedBytes >= NumberOfBytes);
                PoolTrackTableExpansion[Hash].NonPagedFrees += 1;
                PoolTrackTableExpansion[Hash].NonPagedBytes -= NumberOfBytes;
            }

            ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);
            return;
        }
        if (PoolTrackTableExpansion[Hash].Key == 0) {
            break;
        }
    }

    ExReleaseSpinLock (&ExpTaggedPoolLock, OldIrql);

    //
    // This entry must have been charged to the overflow bucket.
    // Update the pool tracker table entry for it.
    //

    Hash = (ULONG)PoolTrackTableSize - 1;

#if !defined (NT_UP)

    //
    // Use the current processor to pick a pool tag table to use.  Note that
    // in rare cases, this thread may context switch to another processor but
    // the algorithms below will still be correct.
    //

    Processor = KeGetCurrentProcessorNumber ();

    ASSERT (Processor < MAXIMUM_PROCESSOR_TAG_TABLES);

    TrackTable = ExPoolTagTables[Processor];

#else

    TrackTable = PoolTrackTable;

#endif

OverflowEntry:

    if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
#if defined (NT_UP)
        ASSERT (TrackTable[Hash].PagedAllocs != 0);
        ASSERT (TrackTable[Hash].PagedBytes >= NumberOfBytes);
#endif
        InterlockedIncrement ((PLONG) &TrackTable[Hash].PagedFrees);
        InterlockedExchangeAddSizeT (&TrackTable[Hash].PagedBytes,
                                     0 - NumberOfBytes);
    }
    else {
#if defined (NT_UP)
        ASSERT (TrackTable[Hash].NonPagedAllocs != 0);
        ASSERT (TrackTable[Hash].NonPagedBytes >= NumberOfBytes);
#endif
        InterlockedIncrement ((PLONG) &TrackTable[Hash].NonPagedFrees);
        InterlockedExchangeAddSizeT (&TrackTable[Hash].NonPagedBytes,
                                     0 - NumberOfBytes);
    }
    return;
}

VOID
ExpRemovePoolTracker (
    IN ULONG Key,
    IN SIZE_T NumberOfBytes,
    IN POOL_TYPE PoolType
    )
{
    ExpRemovePoolTrackerInline (Key, NumberOfBytes, PoolType);
}


LOGICAL
ExpExpandBigPageTable (
    IN KIRQL OldIrql
    )

/*++

Routine Description:

    This function expands the big page tracking table and rehashes all the
    old entries into the new table at the same time.

Arguments:

    OldIrql - Supplies the IRQL the pool track table spinlock was acquired at.

Return Value:

    TRUE if the table was expanded, FALSE if not.

Environment:

    Pool track table executive spinlock held exclusive at DISPATCH_LEVEL.

    The spinlock is released here (and IRQL lowered) prior to returning !!!

--*/
{
    ULONG Hash;
    ULONG NewHash;
    ULONG BigPages;
    SIZE_T SizeInBytes;
    SIZE_T NewSizeInBytes;
    SIZE_T TableSize;
    PPOOL_TRACKER_BIG_PAGES Table;
    PPOOL_TRACKER_BIG_PAGES TableEnd;
    PPOOL_TRACKER_BIG_PAGES NewTable;

    //
    // Try to expand the tracker table.
    //

    TableSize = PoolBigPageTableSize;

    SizeInBytes = TableSize * sizeof(POOL_TRACKER_BIG_PAGES);

    NewSizeInBytes = (SizeInBytes << 1);

    if (NewSizeInBytes <= SizeInBytes) {
        ExReleaseSpinLockExclusive (&ExpLargePoolTableLock, OldIrql);
        return FALSE;
    }

    NewTable = MiAllocatePoolPages (NonPagedPool, NewSizeInBytes);

    if (NewTable == NULL) {
        ExReleaseSpinLockExclusive (&ExpLargePoolTableLock, OldIrql);
        return FALSE;
    }

    //
    // Initialize the new table, marking all the new entries as free.
    // Note this loop uses the fact that the table size always doubles.
    //

    RtlZeroMemory (NewTable, NewSizeInBytes);

    Table = NewTable;
    TableEnd = NewTable + (TableSize << 1);

    do {
        Table->Va = (PVOID) POOL_BIG_TABLE_ENTRY_FREE;
        Table += 1;
    } while (Table != TableEnd);

    //
    // Rehash the valid tables in the old table
    // into their new locations in the new table.
    //

    Table = PoolBigPageTable;
    TableEnd = Table + TableSize;

    NewHash = (ULONG) ((TableSize << 1) - 1);

    do {

        if (((ULONG_PTR)Table->Va & POOL_BIG_TABLE_ENTRY_FREE) == 0) {

            Hash = (ULONG)(((ULONG_PTR)Table->Va) >> PAGE_SHIFT);
            Hash = ((Hash >> 24) ^ (Hash >> 16) ^ (Hash >> 8) ^ Hash);

            Hash &= NewHash;

            while (((ULONG_PTR)NewTable[Hash].Va & POOL_BIG_TABLE_ENTRY_FREE) == 0) {
                Hash += 1;
                if (Hash > NewHash) {
                    Hash = 0;
                }
            }

            * (&NewTable[Hash]) = *Table;
        }

        Table += 1;

    } while (Table != TableEnd);

    Table = PoolBigPageTable;
    PoolBigPageTable = NewTable;
    PoolBigPageTableSize = (TableSize << 1);
    PoolBigPageTableHash = NewHash;

    ExReleaseSpinLockExclusive (&ExpLargePoolTableLock, OldIrql);

    //
    // The table growth has completed, release the spinlock.
    //

    BigPages = MiFreePoolPages (Table);

    ExpRemovePoolTracker ('looP',
                          (SIZE_T) BigPages * PAGE_SIZE,
                          NonPagedPool);

    ExpInsertPoolTracker ('looP',
                          ROUND_TO_PAGES (NewSizeInBytes),
                          NonPagedPool);

    return TRUE;
}


LOGICAL
ExpAddTagForBigPages (
    IN PVOID Va,
    IN ULONG Key,
    IN ULONG NumberOfPages,
    IN POOL_TYPE PoolType
    )

/*++

Routine Description:

    This function inserts a pool tag in the big page tag table.

Arguments:

    Va - Supplies the allocated virtual address.

    Key - Supplies the key value used to locate a matching entry in the
          tag table.

    NumberOfPages - Supplies the number of pages that were allocated.

    PoolType - Supplies the type of the pool.

Return Value:

    TRUE if an entry was allocated, FALSE if not.

Environment:

    No pool locks held so the table may be freely expanded here as needed.

--*/
{
    ULONG Hash;
    ULONG IterationCount;
    PVOID OldVa;
    KIRQL OldIrql;
    SIZE_T TableSize;
    PPOOL_TRACKER_BIG_PAGES Entry;
    PPOOL_TRACKER_BIG_PAGES EntryEnd;
    PPOOL_TRACKER_BIG_PAGES EntryStart;

    //
    // The low bit of the address is set to indicate a free entry.  The high
    // bit cannot be used because in some configurations the high bit is not
    // set for all kernelmode addresses.
    //

    ASSERT (((ULONG_PTR)Va & POOL_BIG_TABLE_ENTRY_FREE) == 0);

    if (PoolType & SESSION_POOL_MASK) {
        Hash = (ULONG)(((ULONG_PTR)Va) >> PAGE_SHIFT);
        Hash = ((Hash >> 24) ^ (Hash >> 16) ^ (Hash >> 8) ^ Hash);
        Hash &= ExpSessionPoolBigPageTableHash;

        TableSize = ExpSessionPoolBigPageTableSize;

        Entry = &ExpSessionPoolBigPageTable[Hash];
        EntryStart = Entry;
        EntryEnd = &ExpSessionPoolBigPageTable[TableSize];

        do {

            OldVa = Entry->Va;

            if (((ULONG_PTR)OldVa & POOL_BIG_TABLE_ENTRY_FREE) &&
                (InterlockedCompareExchangePointer (&Entry->Va,
                                                    Va,
                                                    OldVa) == OldVa)) {

                Entry->Key = Key;
                Entry->NumberOfPages = NumberOfPages;

                return TRUE;
            }

            Entry += 1;
            if (Entry >= EntryEnd) {
                Entry = &ExpSessionPoolBigPageTable[0];
            }
        } while (Entry != EntryStart);

#if DBG
        ExpLargeSessionPoolUnTracked += 1;
#endif
        return FALSE;
    }

    IterationCount = 0;

    do {

        Hash = (ULONG)(((ULONG_PTR)Va) >> PAGE_SHIFT);
        Hash = ((Hash >> 24) ^ (Hash >> 16) ^ (Hash >> 8) ^ Hash);

        OldIrql = ExAcquireSpinLockShared (&ExpLargePoolTableLock);
    
        Hash &= PoolBigPageTableHash;

        TableSize = PoolBigPageTableSize;
    
        Entry = &PoolBigPageTable[Hash];
        EntryStart = Entry;
        EntryEnd = &PoolBigPageTable[TableSize];
    
        do {
    
            OldVa = Entry->Va;
    
            if (((ULONG_PTR)OldVa & POOL_BIG_TABLE_ENTRY_FREE) &&
                (InterlockedCompareExchangePointer (&Entry->Va,
                                                    Va,
                                                    OldVa) == OldVa)) {
    
                Entry->Key = Key;
                Entry->NumberOfPages = NumberOfPages;

                InterlockedIncrement (&ExpPoolBigEntriesInUse);

                if ((IterationCount >= 16) &&
                    ((ULONG) ExpPoolBigEntriesInUse > (ULONG) (TableSize / 4))) {

                    if (ExTryAcquireSpinLockExclusive (&ExpLargePoolTableLock) == TRUE) {
                        ASSERT (TableSize == PoolBigPageTableSize);
                
                        //
                        // The expansion function always releases the lock.
                        //

                        ExpExpandBigPageTable (OldIrql);
                    }
                    else {
                        ExReleaseSpinLockShared (&ExpLargePoolTableLock, OldIrql);
                    }
                }
                else {
                    ExReleaseSpinLockShared (&ExpLargePoolTableLock, OldIrql);
                }

                return TRUE;
            }
    
            IterationCount += 1;
            Entry += 1;
            if (Entry >= EntryEnd) {
                Entry = &PoolBigPageTable[0];
            }
        } while (Entry != EntryStart);
    
        //
        // Try to expand the tracker table.
        //
        // Since this involves copying the existing entries over and deleting
        // the old table, first acquire the lock exclusive.
        //
    
        if (ExTryAcquireSpinLockExclusive (&ExpLargePoolTableLock) == FALSE) {
            ExReleaseSpinLockShared (&ExpLargePoolTableLock, OldIrql);
            continue;
        }
    
        //
        // No thread could have grown this during the contention above, so
        // ASSERT that this is the case.
        //
    
        ASSERT (TableSize == PoolBigPageTableSize);
    
        //
        // The expansion function always releases the lock.
        //

        if (ExpExpandBigPageTable (OldIrql) == FALSE) {
            ExpBigTableExpansionFailed += 1;
            return FALSE;
        }

    } while (TRUE);
}


ULONG
ExpFindAndRemoveTagBigPages (
    IN PVOID Va,
    OUT PULONG BigPages,
    IN POOL_TYPE PoolType
    )

/*++

Routine Description:

    This function removes a pool tag from the big page tag table.

Arguments:

    Va - Supplies the allocated virtual address.

    BigPages - Returns the number of pages that were allocated.

    PoolType - Supplies the type of the pool.

Return Value:

    TRUE if an entry was found and removed, FALSE if not.

Environment:

    No pool locks held so the table may be freely expanded here as needed.

--*/

{   ULONG Hash;
    LOGICAL Inserted;
    SIZE_T TableSize;
    KIRQL OldIrql;
    ULONG ReturnKey;
    PPOOL_TRACKER_BIG_PAGES Entry;

    ASSERT (((ULONG_PTR)Va & POOL_BIG_TABLE_ENTRY_FREE) == 0);

    if (PoolType & SESSION_POOL_MASK) {
        Hash = (ULONG)(((ULONG_PTR)Va) >> PAGE_SHIFT);
        Hash = ((Hash >> 24) ^ (Hash >> 16) ^ (Hash >> 8) ^ Hash);
        Hash &= ExpSessionPoolBigPageTableHash;

        ReturnKey = Hash;

        do {
            if (ExpSessionPoolBigPageTable[Hash].Va == Va) {

                *BigPages = ExpSessionPoolBigPageTable[Hash].NumberOfPages;
                ReturnKey = ExpSessionPoolBigPageTable[Hash].Key;

                InterlockedOr ((PLONG) &ExpSessionPoolBigPageTable[Hash].Va,
                               POOL_BIG_TABLE_ENTRY_FREE);

                return ReturnKey;
            }

            Hash += 1;
            if (Hash >= ExpSessionPoolBigPageTableSize) {
                Hash = 0;
            }
        } while (Hash != ReturnKey);

        *BigPages = 0;
        return ' GIB';
    }

    Inserted = TRUE;

    Hash = (ULONG)(((ULONG_PTR)Va) >> PAGE_SHIFT);
    Hash = ((Hash >> 24) ^ (Hash >> 16) ^ (Hash >> 8) ^ Hash);

    OldIrql = ExAcquireSpinLockShared (&ExpLargePoolTableLock);

    Hash &= PoolBigPageTableHash;

    TableSize = PoolBigPageTableSize;

    while (PoolBigPageTable[Hash].Va != Va) {
        Hash += 1;
        if (Hash >= TableSize) {

            if (!Inserted) {

                ExReleaseSpinLockShared (&ExpLargePoolTableLock, OldIrql);

                *BigPages = 0;
                return ' GIB';
            }

            Hash = 0;
            Inserted = FALSE;
        }
    }

    Entry = &PoolBigPageTable[Hash];

    *BigPages = Entry->NumberOfPages;
    ReturnKey = Entry->Key;

    InterlockedDecrement (&ExpPoolBigEntriesInUse);

#if defined(_WIN64)
    InterlockedIncrement64 ((PLONGLONG) &Entry->Va);
#else
    InterlockedIncrement ((PLONG) &Entry->Va);
#endif

    ExReleaseSpinLockShared (&ExpLargePoolTableLock, OldIrql);

    return ReturnKey;
}


VOID
ExFreePoolWithTag (
    __in PVOID P,
    __in ULONG TagToFree
    )

/*++

Routine Description:

    This function deallocates a block of pool. This function is used to
    deallocate to both the page aligned pools and the buddy (less than
    a page) pools.

    If the address of the block being deallocated is page-aligned, then
    the page-aligned pool deallocator is used.

    Otherwise, the binary buddy pool deallocator is used.  Deallocation
    looks at the allocated block's pool header to determine the pool
    type and block size being deallocated.  If the pool was allocated
    using ExAllocatePoolWithQuota, then after the deallocation is
    complete, the appropriate process's pool quota is adjusted to reflect
    the deallocation, and the process object is dereferenced.

Arguments:

    P - Supplies the address of the block of pool being deallocated.

    TagToFree - Supplies the tag of the block being freed.

Return Value:

    None.

--*/

{
    PVOID OldValue;
    POOL_TYPE CheckType;
    PPOOL_HEADER Entry;
    ULONG BlockSize;
    KLOCK_QUEUE_HANDLE LockHandle;
    PPOOL_HEADER NextEntry;
    POOL_TYPE PoolType;
    POOL_TYPE EntryPoolType;
    PPOOL_DESCRIPTOR PoolDesc;
    PEPROCESS ProcessBilled;
    LOGICAL Combined;
    ULONG BigPages;
    ULONG BigPages2;
    SIZE_T NumberOfBytes;
    ULONG Tag;
    PKPRCB Prcb;
    PGENERAL_LOOKASIDE LookasideList;

    PERFINFO_FREEPOOL(P);

    UNREFERENCED_PARAMETER (TagToFree);

    //
    // Initializing LockHandle is not needed for correctness but without
    // it the compiler cannot compile this code W4 to check for use of
    // uninitialized variables.
    //

    LockHandle.OldIrql = 0;

    if (ExpPoolFlags & (EX_CHECK_POOL_FREES_FOR_ACTIVE_TIMERS |
                        EX_CHECK_POOL_FREES_FOR_ACTIVE_WORKERS |
                        EX_CHECK_POOL_FREES_FOR_ACTIVE_RESOURCES |
                        EX_KERNEL_VERIFIER_ENABLED |
                        EX_VERIFIER_DEADLOCK_DETECTION_ENABLED |
                        EX_SPECIAL_POOL_ENABLED)) {

        if (ExpPoolFlags & EX_SPECIAL_POOL_ENABLED) {

            //
            // Log all pool frees in this mode.
            //

            ULONG Hash;
            ULONG Index;
            LOGICAL SpecialPool;
            PEX_FREE_POOL_TRACES Information;

            SpecialPool = MmIsSpecialPoolAddress (P);

            if (ExFreePoolTraces != NULL) {

                Index = InterlockedIncrement (&ExFreePoolIndex);
                Index &= ExFreePoolMask;
                Information = &ExFreePoolTraces[Index];

                Information->Thread = PsGetCurrentThread ();
                Information->PoolAddress = P;
                if (SpecialPool == TRUE) {
                    Information->PoolHeader = *(PPOOL_HEADER) PAGE_ALIGN (P);
                }
                else if (!PAGE_ALIGNED(P)) {
                    Entry = (PPOOL_HEADER)((PCHAR)P - POOL_OVERHEAD);
                    Information->PoolHeader = *Entry;
                }
                else {
                    RtlZeroMemory (&Information->PoolHeader,
                                   sizeof (POOL_HEADER));
                    Information->PoolHeader.Ulong1 = MmGetSizeOfBigPoolAllocation (P);
                }

                RtlZeroMemory (&Information->StackTrace[0],
                               EX_FREE_POOL_BACKTRACE_LENGTH * sizeof(PVOID)); 

                RtlCaptureStackBackTrace (1,
                                          EX_FREE_POOL_BACKTRACE_LENGTH,
                                          Information->StackTrace,
                                          &Hash);
            }

            if (SpecialPool == TRUE) {

                if (ExpPoolFlags & EX_VERIFIER_DEADLOCK_DETECTION_ENABLED) {
                    VerifierDeadlockFreePool (P, PAGE_SIZE);
                }

                MmFreeSpecialPool (P);

                return;
            }
        }

        if (!PAGE_ALIGNED(P)) {

            Entry = (PPOOL_HEADER)((PCHAR)P - POOL_OVERHEAD);

            ASSERT_POOL_NOT_FREE(Entry);

            PoolType = (Entry->PoolType & POOL_TYPE_MASK) - 1;

            CheckType = PoolType & BASE_POOL_TYPE_MASK;

            ASSERT_FREE_IRQL(PoolType, P);

            ASSERT_POOL_TYPE_NOT_ZERO(Entry);

            if (!IS_POOL_HEADER_MARKED_ALLOCATED(Entry)) {
                KeBugCheckEx (BAD_POOL_CALLER,
                              7,
                              __LINE__,
                              (ULONG_PTR)Entry->Ulong1,
                              (ULONG_PTR)P);
            }

            //
            // Make sure the next pool header is consistent with respect to
            // the one being freed to ensure we at least catch pool corruptors
            // at the time of the free.
            //
            // It would be nice to also check the previous pool header as
            // if this is not consistent, it's likely the previous
            // allocation's owner is the corruptor and it's also likely he
            // has not yet freed the block either.  However, this cannot
            // be done safely with mutex or lock synchronization
            // and this would be too high a performance penalty to pay.
            //

            NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + Entry->BlockSize);

            if ((PAGE_END(NextEntry) == FALSE) &&
                (Entry->BlockSize != NextEntry->PreviousSize)) {

                KeBugCheckEx (BAD_POOL_HEADER,
                              0x20,
                              (ULONG_PTR) Entry,
                              (ULONG_PTR) NextEntry,
                              Entry->Ulong1);
            }

            NumberOfBytes = (SIZE_T)Entry->BlockSize << POOL_BLOCK_SHIFT;

            if (ExpPoolFlags & EX_VERIFIER_DEADLOCK_DETECTION_ENABLED) {
                VerifierDeadlockFreePool (P, NumberOfBytes - POOL_OVERHEAD);
            }

            if (Entry->PoolType & POOL_VERIFIER_MASK) {
                VerifierFreeTrackedPool (P,
                                         NumberOfBytes,
                                         CheckType,
                                         FALSE);
            }

            //
            // Check if an ERESOURCE is currently active in this memory block.
            //

            FREE_CHECK_ERESOURCE (Entry, NumberOfBytes);

            //
            // Check if a KTIMER is currently active in this memory block.
            //

            FREE_CHECK_KTIMER (Entry, NumberOfBytes);

            //
            // Look for work items still queued.
            //

            FREE_CHECK_WORKER (Entry, NumberOfBytes);
        }
    }

    //
    // If the entry is page aligned, then free the block to the page aligned
    // pool.  Otherwise, free the block to the allocation lists.
    //

    if (PAGE_ALIGNED(P)) {

        PoolType = MmDeterminePoolType (P);

        ASSERT_FREE_IRQL(PoolType, P);

        CheckType = PoolType & BASE_POOL_TYPE_MASK;

        if (PoolType == PagedPoolSession) {
            PoolDesc = ExpSessionPoolDescriptor;
        }
        else {

            PoolDesc = PoolVector[PoolType];

            if (CheckType == NonPagedPool) {
                if (MiIsPoolLargeSession (P) == TRUE) {
                    PoolDesc = ExpSessionPoolDescriptor;
                    PoolType = NonPagedPoolSession;
                }
            }
        }

        Tag = ExpFindAndRemoveTagBigPages (P, &BigPages, PoolType);

        if (BigPages == 0) {

            //
            // This means the allocator wasn't able to insert this
            // entry into the big page tag table.  This allocation must
            // have been re-tagged as BIG at the time, our problem here
            // is that we don't know the size (or the real original tag).
            //
            // Ask Mm directly for the size.
            //

            BigPages = MmGetSizeOfBigPoolAllocation (P);

            ASSERT (BigPages != 0);

            ASSERT (Tag == ' GIB');
        }
        else if (Tag & PROTECTED_POOL) {

            Tag &= ~PROTECTED_POOL;
        }

        NumberOfBytes = (SIZE_T)BigPages << PAGE_SHIFT;

        ExpRemovePoolTracker (Tag, NumberOfBytes, PoolType);

        if (ExpPoolFlags & (EX_CHECK_POOL_FREES_FOR_ACTIVE_TIMERS |
                            EX_CHECK_POOL_FREES_FOR_ACTIVE_WORKERS |
                            EX_CHECK_POOL_FREES_FOR_ACTIVE_RESOURCES |
                            EX_VERIFIER_DEADLOCK_DETECTION_ENABLED)) {

            if (ExpPoolFlags & EX_VERIFIER_DEADLOCK_DETECTION_ENABLED) {
                VerifierDeadlockFreePool (P, NumberOfBytes);
            }

            //
            // Check if an ERESOURCE is currently active in this memory block.
            //

            FREE_CHECK_ERESOURCE (P, NumberOfBytes);

            //
            // Check if a KTIMER is currently active in this memory block.
            //

            FREE_CHECK_KTIMER (P, NumberOfBytes);

            //
            // Search worker queues for work items still queued.
            //

            FREE_CHECK_WORKER (P, NumberOfBytes);
        }

        InterlockedIncrement ((PLONG)&PoolDesc->RunningDeAllocs);

        InterlockedExchangeAddSizeT (&PoolDesc->TotalBytes, 0 - NumberOfBytes);

        BigPages2 = MiFreePoolPages (P);

        ASSERT (BigPages == BigPages2);

        InterlockedExchangeAdd ((PLONG)&PoolDesc->TotalBigPages, (LONG)(0 - BigPages2));

        return;
    }

    //
    // Align the entry address to a pool allocation boundary.
    //

    Entry = (PPOOL_HEADER)((PCHAR)P - POOL_OVERHEAD);

    BlockSize = Entry->BlockSize;

    EntryPoolType = Entry->PoolType;

    PoolType = (Entry->PoolType & POOL_TYPE_MASK) - 1;

    CheckType = PoolType & BASE_POOL_TYPE_MASK;

    ASSERT_POOL_NOT_FREE (Entry);

    ASSERT_FREE_IRQL (PoolType, P);

    ASSERT_POOL_TYPE_NOT_ZERO (Entry);

    if (!IS_POOL_HEADER_MARKED_ALLOCATED(Entry)) {
        KeBugCheckEx (BAD_POOL_CALLER,
                      7,
                      __LINE__,
                      (ULONG_PTR)Entry->Ulong1,
                      (ULONG_PTR)P);
    }

    Tag = Entry->PoolTag;
    if (Tag & PROTECTED_POOL) {
        Tag &= ~PROTECTED_POOL;
    }

    //
    // Make sure the next pool header is consistent with respect to the
    // one being freed to ensure we at least catch pool corruptors at the
    // time of the free.
    //
    // It would be nice to also check the previous pool header as if this
    // is not consistent, it's likely the previous allocation's owner is the
    // corruptor and it's also likely he has not yet freed the block either.
    // However, this cannot be done safely with mutex or lock synchronization
    // and this would be too high a performance penalty to pay.
    //

    NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + BlockSize);

    if ((PAGE_END(NextEntry) == FALSE) &&
        (BlockSize != NextEntry->PreviousSize)) {

        KeBugCheckEx (BAD_POOL_HEADER,
                      0x20,
                      (ULONG_PTR) Entry,
                      (ULONG_PTR) NextEntry,
                      Entry->Ulong1);
    }

    PoolDesc = PoolVector[CheckType];

    MARK_POOL_HEADER_FREED (Entry);

    if (EntryPoolType & SESSION_POOL_MASK) {

        if (CheckType == PagedPool) {
            PoolDesc = ExpSessionPoolDescriptor;
        }
        else if (ExpNumberOfNonPagedPools > 1) {
            PoolDesc = ExpNonPagedPoolDescriptor[DECODE_POOL_INDEX(Entry)];
        }

        //
        // All session space allocations have an index of 0 unless there
        // are multiple nonpaged (session) pools.
        //

        ASSERT ((DECODE_POOL_INDEX(Entry) == 0) || (ExpNumberOfNonPagedPools > 1));
    }
    else {

        if (CheckType == PagedPool) {
            ASSERT (DECODE_POOL_INDEX(Entry) <= ExpNumberOfPagedPools);
            PoolDesc = ExpPagedPoolDescriptor[DECODE_POOL_INDEX(Entry)];
        }
        else {
            ASSERT ((DECODE_POOL_INDEX(Entry) == 0) || (ExpNumberOfNonPagedPools > 1));
            if (ExpNumberOfNonPagedPools > 1) {
                PoolDesc = ExpNonPagedPoolDescriptor[DECODE_POOL_INDEX(Entry)];
            }
        }
    }

    //
    // Update the pool tracking database.
    //

    ExpRemovePoolTrackerInline (Tag,
                                BlockSize << POOL_BLOCK_SHIFT,
                                EntryPoolType - 1);

    //
    // If quota was charged when the pool was allocated, release it now.
    //

    if (EntryPoolType & POOL_QUOTA_MASK) {
        ProcessBilled = ExpGetBilledProcess (Entry);
        if (ProcessBilled != NULL) {
            PsReturnPoolQuota (ProcessBilled,
                               PoolType & BASE_POOL_TYPE_MASK,
                               BlockSize << POOL_BLOCK_SHIFT);

            ObDereferenceObject (ProcessBilled);
#if DBG
            InterlockedDecrement (&ExConcurrentQuotaPool);
#endif
        }
    }

    //
    // If the pool block is a small block, then attempt to free the block
    // to the single entry lookaside list. If the free attempt fails, then
    // free the block by merging it back into the pool data structures.
    //

    if (((EntryPoolType & SESSION_POOL_MASK) == 0) ||
        (CheckType == NonPagedPool)) {

        if ((BlockSize <= POOL_SMALL_LISTS) && (USING_HOT_COLD_METRICS == 0)) {

            //
            // Try to free the small block to a per processor lookaside list.
            //

            Prcb = KeGetCurrentPrcb ();

            if (CheckType == PagedPool) {

                //
                // Prototype pool is never put on general lookaside lists
                // due to the sharecounts applied on these allocations when
                // they are in use (ie: the rest of this page is comprised of
                // prototype allocations even though this allocation is being
                // freed).  Pages containing prototype allocations are much
                // more difficult for memory management to trim (unlike the
                // rest of paged pool) due to the sharecounts generally applied.
                //

                if (PoolDesc->PoolIndex == 0) {
                    goto NoLookaside;
                }

                //
                // Only free the small block to the current processor's
                // lookaside list if the block is local to this node.
                //

                if (KeNumberNodes > 1) {
                    if (Prcb->ParentNode->Color != PoolDesc->PoolIndex - 1) {
                        goto NoLookaside;
                    }
                }

                LookasideList = Prcb->PPPagedLookasideList[BlockSize - 1].P;
                LookasideList->TotalFrees += 1;

                if (ExQueryDepthSList(&LookasideList->ListHead) < LookasideList->Depth) {
                    LookasideList->FreeHits += 1;
                    InterlockedPushEntrySList (&LookasideList->ListHead,
                                               (PSLIST_ENTRY)P);

                    return;
                }

                LookasideList = Prcb->PPPagedLookasideList[BlockSize - 1].L;
                LookasideList->TotalFrees += 1;

                if (ExQueryDepthSList(&LookasideList->ListHead) < LookasideList->Depth) {
                    LookasideList->FreeHits += 1;
                    InterlockedPushEntrySList (&LookasideList->ListHead,
                                               (PSLIST_ENTRY)P);

                    return;
                }

            }
            else {

                //
                // Only free the small block to the current processor's
                // lookaside list if the block is local to this node.
                //

                if (KeNumberNodes > 1) {
                    if (Prcb->ParentNode->Color != PoolDesc->PoolIndex) {
                        goto NoLookaside;
                    }
                }

                LookasideList = Prcb->PPNPagedLookasideList[BlockSize - 1].P;
                LookasideList->TotalFrees += 1;

                if (ExQueryDepthSList(&LookasideList->ListHead) < LookasideList->Depth) {
                    LookasideList->FreeHits += 1;
                    InterlockedPushEntrySList (&LookasideList->ListHead,
                                               (PSLIST_ENTRY)P);

                    return;

                }

                LookasideList = Prcb->PPNPagedLookasideList[BlockSize - 1].L;
                LookasideList->TotalFrees += 1;

                if (ExQueryDepthSList(&LookasideList->ListHead) < LookasideList->Depth) {
                    LookasideList->FreeHits += 1;
                    InterlockedPushEntrySList (&LookasideList->ListHead,
                                               (PSLIST_ENTRY)P);

                    return;
                }
            }
        }
    }
    else {

        if (BlockSize <= ExpSessionPoolSmallLists) {

            //
            // Attempt to free the small block to the session lookaside list.
            //

            LookasideList = (PGENERAL_LOOKASIDE)(ULONG_PTR)(ExpSessionPoolLookaside + BlockSize - 1);

            LookasideList->TotalFrees += 1;

            if (ExQueryDepthSList(&LookasideList->ListHead) < LookasideList->Depth) {
                LookasideList->FreeHits += 1;
                InterlockedPushEntrySList (&LookasideList->ListHead,
                                           (PSLIST_ENTRY)P);

                return;
            }
        }
    }

NoLookaside:

    //
    // If the pool block release can be queued so the pool mutex/spinlock
    // acquisition/release can be amortized then do so.  Note "hot" blocks
    // are generally in the lookasides above to provide fast reuse to take
    // advantage of hardware caching.
    //

    if (ExpPoolFlags & EX_DELAY_POOL_FREES) {

        if (PoolDesc->PendingFreeDepth >= EXP_MAXIMUM_POOL_FREES_PENDING) {
            ExDeferredFreePool (PoolDesc);
        }

        //
        // Push this entry on the deferred list.
        //

        do {

            OldValue = ReadForWriteAccess (&PoolDesc->PendingFrees);
            ((PSINGLE_LIST_ENTRY)P)->Next = OldValue;

        } while (InterlockedCompareExchangePointer (
                        &PoolDesc->PendingFrees,
                        P,
                        OldValue) != OldValue);

        InterlockedIncrement (&PoolDesc->PendingFreeDepth);

        return;
    }

    Combined = FALSE;

    ASSERT (BlockSize == Entry->BlockSize);

    NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + BlockSize);

    InterlockedIncrement ((PLONG)&PoolDesc->RunningDeAllocs);

    InterlockedExchangeAddSizeT (&PoolDesc->TotalBytes, 0 - ((SIZE_T)BlockSize << POOL_BLOCK_SHIFT));

    LOCK_POOL (PoolDesc, LockHandle);

    CHECK_POOL_PAGE (Entry);

    //
    // Free the specified pool block.
    //
    // Check to see if the next entry is free.
    //

    if (PAGE_END(NextEntry) == FALSE) {

        if (NextEntry->PoolType == 0) {

            //
            // This block is free, combine with the released block.
            //

            Combined = TRUE;

            //
            // If the split pool block contains only a header, then
            // it was not inserted and therefore cannot be removed.
            //
            // Note if the minimum pool block size is bigger than the
            // header then there can be no blocks like this.
            //

            if ((POOL_OVERHEAD != POOL_SMALLEST_BLOCK) ||
                (NextEntry->BlockSize != 1)) {

                CHECK_LIST(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD)));
                PrivateRemoveEntryList(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD)));
                CHECK_LIST(DecodeLink(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD))->Flink));
                CHECK_LIST(DecodeLink(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD))->Blink));
            }

            Entry->BlockSize = Entry->BlockSize + NextEntry->BlockSize;
        }
    }

    //
    // Check to see if the previous entry is free.
    //

    if (Entry->PreviousSize != 0) {
        NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry - Entry->PreviousSize);
        if (NextEntry->PoolType == 0) {

            //
            // This block is free, combine with the released block.
            //

            Combined = TRUE;

            //
            // If the split pool block contains only a header, then
            // it was not inserted and therefore cannot be removed.
            //
            // Note if the minimum pool block size is bigger than the
            // header then there can be no blocks like this.
            //

            if ((POOL_OVERHEAD != POOL_SMALLEST_BLOCK) ||
                (NextEntry->BlockSize != 1)) {

                CHECK_LIST(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD)));
                PrivateRemoveEntryList(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD)));
                CHECK_LIST(DecodeLink(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD))->Flink));
                CHECK_LIST(DecodeLink(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD))->Blink));
            }

            NextEntry->BlockSize = NextEntry->BlockSize + Entry->BlockSize;
            Entry = NextEntry;
        }
    }

    //
    // If the block being freed has been combined into a full page,
    // then return the free page to memory management.
    //

    if (PAGE_ALIGNED(Entry) &&
        (PAGE_END((PPOOL_BLOCK)Entry + Entry->BlockSize) != FALSE)) {

        UNLOCK_POOL (PoolDesc, LockHandle);

        InterlockedExchangeAdd ((PLONG)&PoolDesc->TotalPages, (LONG)-1);

        PERFINFO_FREEPOOLPAGE(CheckType, Entry->PoolIndex, Entry, PoolDesc);

        MiFreePoolPages (Entry);
    }
    else {

        //
        // Insert this element into the list.
        //

        Entry->PoolType = 0;
        BlockSize = Entry->BlockSize;

        ASSERT (BlockSize != 1);

        //
        // If the freed block was combined with any other block, then
        // adjust the size of the next block if necessary.
        //

        if (Combined != FALSE) {

            //
            // The size of this entry has changed, if this entry is
            // not the last one in the page, update the pool block
            // after this block to have a new previous allocation size.
            //

            NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + BlockSize);
            if (PAGE_END(NextEntry) == FALSE) {
                NextEntry->PreviousSize = (USHORT) BlockSize;
            }
        }

        //
        // Always insert at the head in hopes of reusing cache lines.
        //

        PrivateInsertHeadList (&PoolDesc->ListHeads[BlockSize - 1],
                               ((PLIST_ENTRY)((PCHAR)Entry + POOL_OVERHEAD)));

        CHECK_LIST(((PLIST_ENTRY)((PCHAR)Entry + POOL_OVERHEAD)));

        UNLOCK_POOL(PoolDesc, LockHandle);
    }
}

VOID
ExFreePool (
    __in PVOID P
    )
{
    ExFreePoolWithTag (P, 0);
    return;
}


VOID
ExDeferredFreePool (
     IN PPOOL_DESCRIPTOR PoolDesc
     )

/*++

Routine Description:

    This routine frees a number of pool allocations at once to amortize the
    synchronization overhead cost.

Arguments:

    PoolDesc - Supplies the relevant pool descriptor.

Return Value:

    None.

Environment:

    Kernel mode.  May be as high as APC_LEVEL for paged pool or DISPATCH_LEVEL
    for nonpaged pool.

--*/

{
    LONG ListCount;
    KLOCK_QUEUE_HANDLE LockHandle;
    POOL_TYPE CheckType;
    PPOOL_HEADER Entry;
    ULONG Index;
    ULONG WholePageCount;
    PPOOL_HEADER NextEntry;
    ULONG PoolIndex;
    LOGICAL Combined;
    PSINGLE_LIST_ENTRY SingleListEntry;
    PSINGLE_LIST_ENTRY NextSingleListEntry;
    PSINGLE_LIST_ENTRY FirstEntry;
    PSINGLE_LIST_ENTRY LastEntry;
    PSINGLE_LIST_ENTRY WholePages;

    CheckType = PoolDesc->PoolType & BASE_POOL_TYPE_MASK;

    //
    // Initializing LockHandle is not needed for correctness but without
    // it the compiler cannot compile this code W4 to check for use of
    // uninitialized variables.
    //

    LockHandle.OldIrql = 0;

    ListCount = 0;
    WholePages = NULL;
    WholePageCount = 0;
    LastEntry = NULL;

    LOCK_POOL (PoolDesc, LockHandle);

    if (PoolDesc->PendingFrees == NULL) {
        UNLOCK_POOL (PoolDesc, LockHandle);
        return;
    }

    //
    // Free each deferred pool entry until they're all done.
    //

    do {

        SingleListEntry = PoolDesc->PendingFrees;

        FirstEntry = SingleListEntry;

        do {

            NextSingleListEntry = SingleListEntry->Next;

            //
            // Process the deferred entry.
            //

            ListCount += 1;

            Entry = (PPOOL_HEADER)((PCHAR)SingleListEntry - POOL_OVERHEAD);

            PoolIndex = DECODE_POOL_INDEX(Entry);

            //
            // Process the block.
            //

            Combined = FALSE;

            CHECK_POOL_PAGE (Entry);

            InterlockedIncrement ((PLONG)&PoolDesc->RunningDeAllocs);

            InterlockedExchangeAddSizeT (&PoolDesc->TotalBytes,
                            0 - ((SIZE_T)Entry->BlockSize << POOL_BLOCK_SHIFT));

            //
            // Free the specified pool block.
            //
            // Check to see if the next entry is free.
            //

            NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + Entry->BlockSize);
            if (PAGE_END(NextEntry) == FALSE) {

                if (NextEntry->PoolType == 0) {

                    //
                    // This block is free, combine with the released block.
                    //

                    Combined = TRUE;

                    //
                    // If the split pool block contains only a header, then
                    // it was not inserted and therefore cannot be removed.
                    //
                    // Note if the minimum pool block size is bigger than the
                    // header then there can be no blocks like this.
                    //

                    if ((POOL_OVERHEAD != POOL_SMALLEST_BLOCK) ||
                        (NextEntry->BlockSize != 1)) {

                        CHECK_LIST(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD)));
                        PrivateRemoveEntryList(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD)));
                        CHECK_LIST(DecodeLink(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD))->Flink));
                        CHECK_LIST(DecodeLink(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD))->Blink));
                    }

                    Entry->BlockSize = Entry->BlockSize + NextEntry->BlockSize;
                }
            }

            //
            // Check to see if the previous entry is free.
            //

            if (Entry->PreviousSize != 0) {
                NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry - Entry->PreviousSize);
                if (NextEntry->PoolType == 0) {

                    //
                    // This block is free, combine with the released block.
                    //

                    Combined = TRUE;

                    //
                    // If the split pool block contains only a header, then
                    // it was not inserted and therefore cannot be removed.
                    //
                    // Note if the minimum pool block size is bigger than the
                    // header then there can be no blocks like this.
                    //

                    if ((POOL_OVERHEAD != POOL_SMALLEST_BLOCK) ||
                        (NextEntry->BlockSize != 1)) {

                        CHECK_LIST(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD)));
                        PrivateRemoveEntryList(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD)));
                        CHECK_LIST(DecodeLink(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD))->Flink));
                        CHECK_LIST(DecodeLink(((PLIST_ENTRY)((PCHAR)NextEntry + POOL_OVERHEAD))->Blink));
                    }

                    NextEntry->BlockSize = NextEntry->BlockSize + Entry->BlockSize;
                    Entry = NextEntry;
                }
            }

            //
            // If the block being freed has been combined into a full page,
            // then return the free page to memory management.
            //

            if (PAGE_ALIGNED(Entry) &&
                (PAGE_END((PPOOL_BLOCK)Entry + Entry->BlockSize) != FALSE)) {

                ((PSINGLE_LIST_ENTRY)Entry)->Next = WholePages;
                WholePages = (PSINGLE_LIST_ENTRY) Entry;
                WholePageCount += 1;
            }
            else {

                //
                // Insert this element into the list.
                //

                Entry->PoolType = 0;
                ENCODE_POOL_INDEX(Entry, PoolIndex);
                Index = Entry->BlockSize;

                ASSERT (Index != 1);

                //
                // If the freed block was combined with any other block, then
                // adjust the size of the next block if necessary.
                //

                if (Combined != FALSE) {

                    //
                    // The size of this entry has changed, if this entry is
                    // not the last one in the page, update the pool block
                    // after this block to have a new previous allocation size.
                    //

                    NextEntry = (PPOOL_HEADER)((PPOOL_BLOCK)Entry + Index);
                    if (PAGE_END(NextEntry) == FALSE) {
                        NextEntry->PreviousSize = (USHORT) Index;
                    }
                }

                //
                // Always insert at the head in hopes of reusing cache lines.
                //

                PrivateInsertHeadList(&PoolDesc->ListHeads[Index - 1], ((PLIST_ENTRY)((PCHAR)Entry + POOL_OVERHEAD)));

                CHECK_LIST(((PLIST_ENTRY)((PCHAR)Entry + POOL_OVERHEAD)));
            }

            //
            // March on to the next entry if there is one.
            //

            if (NextSingleListEntry == LastEntry) {
                break;
            }

            SingleListEntry = NextSingleListEntry;

        } while (TRUE);

        if (InterlockedCompareExchangePointer (&PoolDesc->PendingFrees,
                                               NULL,
                                               FirstEntry) == FirstEntry) {
            break;
        }
        LastEntry = FirstEntry;

    } while (TRUE);

    UNLOCK_POOL (PoolDesc, LockHandle);

    if (WholePages != NULL) {

        //
        // If the pool type is paged pool, then the global paged pool mutex
        // must be held during the free of the pool pages.  Hence any
        // full pages were batched up and are now dealt with in one go.
        //

        Entry = (PPOOL_HEADER) WholePages;

        InterlockedExchangeAdd ((PLONG)&PoolDesc->TotalPages, 0 - WholePageCount);
        do {

            NextEntry = (PPOOL_HEADER) (((PSINGLE_LIST_ENTRY)Entry)->Next);

            PERFINFO_FREEPOOLPAGE(CheckType, PoolIndex, Entry, PoolDesc);

            MiFreePoolPages (Entry);

            Entry = NextEntry;

        } while (Entry != NULL);
    }

    InterlockedExchangeAdd (&PoolDesc->PendingFreeDepth, (0 - ListCount));

    return;
}

SIZE_T
ExQueryPoolBlockSize (
    __in PVOID PoolBlock,
    __out PBOOLEAN QuotaCharged
    )

/*++

Routine Description:

    This function returns the size of the pool block.

Arguments:

    PoolBlock - Supplies the address of the block of pool.

    QuotaCharged - Supplies a BOOLEAN variable to receive whether or not the
        pool block had quota charged.

    NOTE: If the entry is bigger than a page, the value PAGE_SIZE is returned
          rather than the correct number of bytes.

Return Value:

    Size of pool block.

--*/

{
    PPOOL_HEADER Entry;
    SIZE_T size;

    if ((ExpPoolFlags & EX_SPECIAL_POOL_ENABLED) &&
        (MmIsSpecialPoolAddress (PoolBlock))) {
        *QuotaCharged = FALSE;
        return MmQuerySpecialPoolBlockSize (PoolBlock);
    }

    if (PAGE_ALIGNED(PoolBlock)) {
        *QuotaCharged = FALSE;
        return PAGE_SIZE;
    }

    Entry = (PPOOL_HEADER)((PCHAR)PoolBlock - POOL_OVERHEAD);
    size = (ULONG)((Entry->BlockSize << POOL_BLOCK_SHIFT) - POOL_OVERHEAD);

    if (ExpGetBilledProcess (Entry)) {
        *QuotaCharged = TRUE;
    }
    else {
        *QuotaCharged = FALSE;
    }

    return size;
}

VOID
ExQueryPoolUsage (
    __out PULONG PagedPoolPages,
    __out PULONG NonPagedPoolPages,
    __out PULONG PagedPoolAllocs,
    __out PULONG PagedPoolFrees,
    __out PULONG PagedPoolLookasideHits,
    __out PULONG NonPagedPoolAllocs,
    __out PULONG NonPagedPoolFrees,
    __out PULONG NonPagedPoolLookasideHits
    )

{
    ULONG Index;
    PGENERAL_LOOKASIDE Lookaside;
    PLIST_ENTRY NextEntry;
    PPOOL_DESCRIPTOR pd;

    //
    // Sum all the paged pool usage.
    //

    *PagedPoolPages = 0;
    *PagedPoolAllocs = 0;
    *PagedPoolFrees = 0;

    for (Index = 0; Index < ExpNumberOfPagedPools + 1; Index += 1) {
        pd = ExpPagedPoolDescriptor[Index];
        *PagedPoolPages += pd->TotalPages + pd->TotalBigPages;
        *PagedPoolAllocs += pd->RunningAllocs;
        *PagedPoolFrees += pd->RunningDeAllocs;
    }

    //
    // Sum all the nonpaged pool usage.
    //

    pd = &NonPagedPoolDescriptor;
    *NonPagedPoolPages = pd->TotalPages + pd->TotalBigPages;
    *NonPagedPoolAllocs = pd->RunningAllocs;
    *NonPagedPoolFrees = pd->RunningDeAllocs;

    if (ExpNumberOfNonPagedPools > 1) {
        for (Index = 0; Index < ExpNumberOfNonPagedPools; Index += 1) {
            pd = ExpNonPagedPoolDescriptor[Index];
            *NonPagedPoolPages += pd->TotalPages + pd->TotalBigPages;
            *NonPagedPoolAllocs += pd->RunningAllocs;
            *NonPagedPoolFrees += pd->RunningDeAllocs;
        }
    }

    //
    // Sum all the lookaside hits for paged and nonpaged pool.
    //

    NextEntry = ExPoolLookasideListHead.Flink;
    while (NextEntry != &ExPoolLookasideListHead) {
        Lookaside = CONTAINING_RECORD(NextEntry,
                                      GENERAL_LOOKASIDE,
                                      ListEntry);

        if (Lookaside->Type == NonPagedPool) {
            *NonPagedPoolLookasideHits += Lookaside->AllocateHits;

        }
        else {
            *PagedPoolLookasideHits += Lookaside->AllocateHits;
        }

        NextEntry = NextEntry->Flink;
    }

    return;
}


VOID
ExReturnPoolQuota (
    IN PVOID P
    )

/*++

Routine Description:

    This function returns quota charged to a subject process when the
    specified pool block was allocated.

Arguments:

    P - Supplies the address of the block of pool being deallocated.

Return Value:

    None.

--*/

{

    PPOOL_HEADER Entry;
    POOL_TYPE PoolType;
    PEPROCESS ProcessBilled;

    //
    // Do nothing for special pool. No quota was charged.
    //

    if ((ExpPoolFlags & EX_SPECIAL_POOL_ENABLED) &&
        (MmIsSpecialPoolAddress (P))) {
        return;
    }

    //
    // Align the entry address to a pool allocation boundary.
    //

    Entry = (PPOOL_HEADER)((PCHAR)P - POOL_OVERHEAD);

    //
    // If quota was charged, then return the appropriate quota to the
    // subject process.
    //

    if (Entry->PoolType & POOL_QUOTA_MASK) {

        PoolType = (Entry->PoolType & POOL_TYPE_MASK) - 1;

        ProcessBilled = ExpGetBilledProcess (Entry);

        if (ProcessBilled == NULL) {
            return;
        }

#if defined (_WIN64)

        //
        // This flag cannot be cleared in NT32 because it's used to denote the
        // allocation is larger (and the verifier finds its own header
        // based on this).
        //

        Entry->PoolType &= ~POOL_QUOTA_MASK;

#else

        //
        // Instead of clearing the flag above, zero the quota pointer.
        //

        * (PVOID *)((PCHAR)Entry + (Entry->BlockSize << POOL_BLOCK_SHIFT) - sizeof (PVOID)) = NULL;

#endif

        PsReturnPoolQuota (ProcessBilled,
                           PoolType & BASE_POOL_TYPE_MASK,
                           (ULONG)Entry->BlockSize << POOL_BLOCK_SHIFT);

        ObDereferenceObject (ProcessBilled);
#if DBG
        InterlockedDecrement (&ExConcurrentQuotaPool);
#endif
    }

    return;
}

#if !defined (NT_UP)

PVOID
ExCreatePoolTagTable (
    IN ULONG NewProcessorNumber,
    IN UCHAR NodeNumber
    )
{
    SIZE_T NumberOfBytes;
    PPOOL_TRACKER_TABLE NewTagTable;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);
    ASSERT (NewProcessorNumber < MAXIMUM_PROCESSOR_TAG_TABLES);
    ASSERT (ExPoolTagTables[NewProcessorNumber] == NULL);

    NumberOfBytes = (PoolTrackTableSize + 1) * sizeof(POOL_TRACKER_TABLE);

    NewTagTable = MmAllocateIndependentPages (NumberOfBytes, NodeNumber);

    if (NewTagTable != NULL) {

        //
        // Just zero the table here, the tags are lazy filled as various pool
        // allocations and frees occur.  Note no memory barrier is needed
        // because only this processor will read it except when an
        // ExGetPoolTagInfo call occurs, and in that case, explicit memory
        // barriers are used as needed.
        //

        RtlZeroMemory (NewTagTable,
                       PoolTrackTableSize * sizeof(POOL_TRACKER_TABLE));

        ExPoolTagTables[NewProcessorNumber] = NewTagTable;
    }

    return (PVOID) NewTagTable;
}

VOID
ExDeletePoolTagTable (
    IN ULONG NewProcessorNumber
    )

/*++

Routine Description:

    This function deletes the tag table for the specified processor
    number because the processor did not boot.

Arguments:

    NewProcessorNumber - Supplies the processor number that did not boot.

Return Value:

    None.

--*/

{
    KIRQL OldIrql;
    PVOID VirtualAddress;
    SIZE_T NumberOfBytes;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);
    ASSERT (NewProcessorNumber < MAXIMUM_PROCESSOR_TAG_TABLES);
    ASSERT (ExPoolTagTables[NewProcessorNumber] != NULL);

    NumberOfBytes = (PoolTrackTableSize + 1) * sizeof(POOL_TRACKER_TABLE);

    VirtualAddress = ExPoolTagTables[NewProcessorNumber];

    //
    // Raise to DISPATCH to prevent a race when attempting to hot-add a
    // processor while a pool-usage query is active.
    //

    KeRaiseIrql (DISPATCH_LEVEL, &OldIrql);

    ExPoolTagTables[NewProcessorNumber] = NULL;

    KeLowerIrql (OldIrql);

    MmFreeIndependentPages (VirtualAddress, NumberOfBytes);

    return;
}
#endif

typedef struct _POOL_DPC_CONTEXT {

    PPOOL_TRACKER_TABLE PoolTrackTable;
    SIZE_T PoolTrackTableSize;

    PPOOL_TRACKER_TABLE PoolTrackTableExpansion;
    SIZE_T PoolTrackTableSizeExpansion;

} POOL_DPC_CONTEXT, *PPOOL_DPC_CONTEXT;

VOID
ExpGetPoolTagInfoTarget (
    IN PKDPC    Dpc,
    IN PVOID    DeferredContext,
    IN PVOID    SystemArgument1,
    IN PVOID    SystemArgument2
    )
/*++

Routine Description:

    Called by all processors during a pool tag table query.

Arguments:

    Dpc - Supplies a pointer to a control object of type DPC.

    DeferredContext - Deferred context.

    SystemArgument1 - Used to signal completion of this call.

    SystemArgument2 - Used for internal lockstepping during this call.

Return Value:

    None.

Environment:

    DISPATCH_LEVEL since this is called from a DPC.

--*/
{
    PPOOL_DPC_CONTEXT Context;
#if !defined (NT_UP)
    ULONG i;
    PPOOL_TRACKER_TABLE TrackerEntry;
    PPOOL_TRACKER_TABLE LastTrackerEntry;
    PPOOL_TRACKER_TABLE TargetTrackerEntry;
#endif

    UNREFERENCED_PARAMETER (Dpc);

    ASSERT (KeGetCurrentIrql () == DISPATCH_LEVEL);

    Context = DeferredContext;

    //
    // Make sure all DPCs are running (ie: spinning at DISPATCH_LEVEL)
    // to prevent any pool allocations or frees from happening until
    // all the counters are snapped.  Otherwise the counters could
    // be misleading (ie: more frees than allocs, etc).
    //

    if (KeSignalCallDpcSynchronize (SystemArgument2)) {

        //
        // This processor (could be the caller or a target) is the final
        // processor to enter the DPC spinloop.  Snap the data now.
        //

#if defined (NT_UP)

        RtlCopyMemory ((PVOID)Context->PoolTrackTable,
                       (PVOID)PoolTrackTable,
                       Context->PoolTrackTableSize * sizeof (POOL_TRACKER_TABLE));

#else

        RtlCopyMemory ((PVOID)Context->PoolTrackTable,
                       (PVOID)ExPoolTagTables[0],
                       Context->PoolTrackTableSize * sizeof (POOL_TRACKER_TABLE));

        LastTrackerEntry = Context->PoolTrackTable + Context->PoolTrackTableSize;

        for (i = 1; i < MAXIMUM_PROCESSOR_TAG_TABLES; i += 1) {

            TargetTrackerEntry = ExPoolTagTables[i];

            if (TargetTrackerEntry == NULL) {
                continue;
            }

            TrackerEntry = Context->PoolTrackTable;

            while (TrackerEntry != LastTrackerEntry) {

                if (TargetTrackerEntry->Key != 0) {

                    ASSERT (TargetTrackerEntry->Key == TrackerEntry->Key);

                    TrackerEntry->NonPagedAllocs += TargetTrackerEntry->NonPagedAllocs;
                    TrackerEntry->NonPagedFrees += TargetTrackerEntry->NonPagedFrees;
                    TrackerEntry->NonPagedBytes += TargetTrackerEntry->NonPagedBytes;
                    TrackerEntry->PagedAllocs += TargetTrackerEntry->PagedAllocs;
                    TrackerEntry->PagedFrees += TargetTrackerEntry->PagedFrees;
                    TrackerEntry->PagedBytes += TargetTrackerEntry->PagedBytes;
                }
                TrackerEntry += 1;
                TargetTrackerEntry += 1;
            }
        }

#endif

        if (Context->PoolTrackTableSizeExpansion != 0) {
            RtlCopyMemory ((PVOID)(Context->PoolTrackTableExpansion),
                           (PVOID)PoolTrackTableExpansion,
                           Context->PoolTrackTableSizeExpansion * sizeof (POOL_TRACKER_TABLE));
        }
    }

    //
    // Wait until everyone has got to this point before continuing.
    //

    KeSignalCallDpcSynchronize (SystemArgument2);

    //
    // Signal that all processing has been done.
    //

    KeSignalCallDpcDone (SystemArgument1);

    return;
}

NTSTATUS
ExGetPoolTagInfo (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN OUT PULONG ReturnLength OPTIONAL
    )

/*++

Routine Description:

    This function copies the system pool tag information to the supplied
    USER space buffer.  Note that the caller has already probed the USER
    address and wrapped this routine inside a try-except.

Arguments:

    SystemInformation - Supplies a user space buffer to copy the data to.

    SystemInformationLength - Supplies the length of the user buffer.

    ReturnLength - Receives the actual length of the data returned.

Return Value:

    Various NTSTATUS codes.

--*/

{
    SIZE_T NumberOfBytes;
    SIZE_T NumberOfExpansionTableBytes;
    ULONG totalBytes;
    NTSTATUS status;
    PSYSTEM_POOLTAG_INFORMATION taginfo;
    PSYSTEM_POOLTAG poolTag;
    PPOOL_TRACKER_TABLE PoolTrackInfo;
    PPOOL_TRACKER_TABLE TrackerEntry;
    PPOOL_TRACKER_TABLE LastTrackerEntry;
    POOL_DPC_CONTEXT Context;
    SIZE_T LocalTrackTableSize;
    SIZE_T LocalTrackTableSizeExpansion;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    totalBytes = 0;
    status = STATUS_SUCCESS;

    taginfo = (PSYSTEM_POOLTAG_INFORMATION)SystemInformation;
    poolTag = &taginfo->TagInfo[0];
    totalBytes = FIELD_OFFSET(SYSTEM_POOLTAG_INFORMATION, TagInfo);
    taginfo->Count = 0;

    LocalTrackTableSize = PoolTrackTableSize;
    LocalTrackTableSizeExpansion = PoolTrackTableExpansionSize;

    NumberOfBytes = LocalTrackTableSize * sizeof(POOL_TRACKER_TABLE);
    NumberOfExpansionTableBytes = LocalTrackTableSizeExpansion * sizeof (POOL_TRACKER_TABLE);

    PoolTrackInfo = (PPOOL_TRACKER_TABLE) ExAllocatePoolWithTag (
                                    NonPagedPool,
                                    NumberOfBytes + NumberOfExpansionTableBytes,
                                    'ofnI');

    if (PoolTrackInfo == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Context.PoolTrackTable = PoolTrackInfo;
    Context.PoolTrackTableSize = PoolTrackTableSize;

    Context.PoolTrackTableExpansion = (PoolTrackInfo + PoolTrackTableSize);
    Context.PoolTrackTableSizeExpansion = PoolTrackTableExpansionSize;

    KeGenericCallDpc (ExpGetPoolTagInfoTarget, &Context);

    TrackerEntry = PoolTrackInfo;
    LastTrackerEntry = PoolTrackInfo + (LocalTrackTableSize + LocalTrackTableSizeExpansion);

    //
    // Wrap the user space accesses with an exception handler so we can free the
    // pool track info allocation if the user address was bogus.
    //

    try {
        while (TrackerEntry < LastTrackerEntry) {
            if (TrackerEntry->Key != 0) {
                taginfo->Count += 1;
                totalBytes += sizeof (SYSTEM_POOLTAG);
                if (SystemInformationLength < totalBytes) {
                    status = STATUS_INFO_LENGTH_MISMATCH;
                }
                else {
                    ASSERT (TrackerEntry->PagedAllocs >= TrackerEntry->PagedFrees);
                    ASSERT (TrackerEntry->NonPagedAllocs >= TrackerEntry->NonPagedFrees);

                    poolTag->TagUlong = TrackerEntry->Key;
                    poolTag->PagedAllocs = TrackerEntry->PagedAllocs;
                    poolTag->PagedFrees = TrackerEntry->PagedFrees;
                    poolTag->PagedUsed = TrackerEntry->PagedBytes;
                    poolTag->NonPagedAllocs = TrackerEntry->NonPagedAllocs;
                    poolTag->NonPagedFrees = TrackerEntry->NonPagedFrees;
                    poolTag->NonPagedUsed = TrackerEntry->NonPagedBytes;
                    poolTag += 1;
                }
            }
            TrackerEntry += 1;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode ();
    }

    ExFreePool (PoolTrackInfo);

    if (ARGUMENT_PRESENT(ReturnLength)) {
        *ReturnLength = totalBytes;
    }

    return status;
}

NTSTATUS
ExGetSessionPoolTagInfo (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN OUT PULONG ReturnedEntries,
    IN OUT PULONG ActualEntries
    )

/*++

Routine Description:

    This function copies the current session's pool tag information to the
    supplied system-mapped buffer.

Arguments:

    SystemInformation - Supplies a system mapped buffer to copy the data to.

    SystemInformationLength - Supplies the length of the buffer.

    ReturnedEntries - Receives the actual number of entries returned.

    ActualEntries - Receives the total number of entries.
                    This can be more than ReturnedEntries if the caller's
                    buffer is not large enough to hold all the data.

Return Value:

    Various NTSTATUS codes.

--*/

{
    ULONG totalBytes;
    ULONG ActualCount;
    ULONG ReturnedCount;
    NTSTATUS status;
    PSYSTEM_POOLTAG poolTag;
    PPOOL_TRACKER_TABLE TrackerEntry;
    PPOOL_TRACKER_TABLE LastTrackerEntry;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    totalBytes = 0;
    ActualCount = 0;
    ReturnedCount = 0;
    status = STATUS_SUCCESS;

    poolTag = (PSYSTEM_POOLTAG) SystemInformation;

    //
    // Capture the current session's pool information.
    //

    TrackerEntry = ExpSessionPoolTrackTable;
    LastTrackerEntry = TrackerEntry + ExpSessionPoolTrackTableSize;

    while (TrackerEntry < LastTrackerEntry) {
        if (TrackerEntry->Key != 0) {
            ActualCount += 1;
            totalBytes += sizeof (SYSTEM_POOLTAG);

            if (totalBytes > SystemInformationLength) {
                status = STATUS_INFO_LENGTH_MISMATCH;
            }
            else {
                ReturnedCount += 1;

                poolTag->TagUlong = TrackerEntry->Key;
                poolTag->PagedAllocs = TrackerEntry->PagedAllocs;
                poolTag->PagedFrees = TrackerEntry->PagedFrees;
                poolTag->PagedUsed = TrackerEntry->PagedBytes;
                poolTag->NonPagedAllocs = TrackerEntry->NonPagedAllocs;
                poolTag->NonPagedFrees = TrackerEntry->NonPagedFrees;
                poolTag->NonPagedUsed = TrackerEntry->NonPagedBytes;

                //
                // Session pool tag entries are updated with interlocked
                // sequences so it is possible here that we can read one
                // that is in the middle of being updated.  Sanitize the
                // data here so callers don't have to.
                //

                ASSERT ((SSIZE_T)poolTag->PagedUsed >= 0);
                ASSERT ((SSIZE_T)poolTag->NonPagedUsed >= 0);

                if (poolTag->PagedAllocs < poolTag->PagedFrees) {
                    poolTag->PagedAllocs = poolTag->PagedFrees;
                }

                if (poolTag->NonPagedAllocs < poolTag->NonPagedFrees) {
                    poolTag->NonPagedAllocs = poolTag->NonPagedFrees;
                }

                poolTag += 1;
            }
        }
        TrackerEntry += 1;
    }

    *ReturnedEntries = ReturnedCount;
    *ActualEntries = ActualCount;

    return status;
}

NTSTATUS
ExGetBigPoolInfo (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN OUT PULONG ReturnLength OPTIONAL
    )

/*++

Routine Description:

    This function copies the system big pool entry information to the supplied
    USER space buffer.  Note that the caller has already probed the USER
    address and wrapped this routine inside a try-except.

    PAGELK was not used for this function so that calling it causes minimal
    disruption to actual memory usage.

Arguments:

    SystemInformation - Supplies a user space buffer to copy the data to.

    SystemInformationLength - Supplies the length of the user buffer.

    ReturnLength - Supplies the actual length of the data returned.

Return Value:

    Various NTSTATUS codes.

--*/

{
    ULONG TotalBytes;
    KIRQL OldIrql;
    NTSTATUS Status;
    PVOID NewTable;
    PPOOL_TRACKER_BIG_PAGES SystemPoolEntry;
    PPOOL_TRACKER_BIG_PAGES SystemPoolEntryEnd;
    SIZE_T SnappedBigTableSize;
    SIZE_T SnappedBigTableSizeInBytes;

    PSYSTEM_BIGPOOL_ENTRY UserPoolEntry;
    PSYSTEM_BIGPOOL_INFORMATION UserPoolInfo;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    NewTable = NULL;
    Status = STATUS_SUCCESS;

    UserPoolInfo = (PSYSTEM_BIGPOOL_INFORMATION)SystemInformation;
    UserPoolEntry = &UserPoolInfo->AllocatedInfo[0];
    TotalBytes = FIELD_OFFSET(SYSTEM_BIGPOOL_INFORMATION, AllocatedInfo);
    UserPoolInfo->Count = 0;

    do {

        SnappedBigTableSize = PoolBigPageTableSize;
        SnappedBigTableSizeInBytes =
                    SnappedBigTableSize * sizeof (POOL_TRACKER_BIG_PAGES);

        if (NewTable != NULL) {
            MiFreePoolPages (NewTable);
        }

        //
        // Use MiAllocatePoolPages for the temporary buffer so we won't have
        // to filter it out of the results before handing them back.
        //

        NewTable = MiAllocatePoolPages (NonPagedPool,
                                        SnappedBigTableSizeInBytes);

        if (NewTable == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        OldIrql = ExAcquireSpinLockExclusive (&ExpLargePoolTableLock);

        if (SnappedBigTableSize >= PoolBigPageTableSize) {

            //
            // Success - our table is big enough to hold everything.
            //

            break;
        }

        ExReleaseSpinLockExclusive (&ExpLargePoolTableLock, OldIrql);

    } while (TRUE);

    RtlCopyMemory (NewTable,
                   PoolBigPageTable,
                   PoolBigPageTableSize * sizeof (POOL_TRACKER_BIG_PAGES));

    SnappedBigTableSize = PoolBigPageTableSize;

    ExReleaseSpinLockExclusive (&ExpLargePoolTableLock, OldIrql);

    SystemPoolEntry = NewTable;
    SystemPoolEntryEnd = SystemPoolEntry + SnappedBigTableSize;

    //
    // Wrap the user space accesses with an exception handler so we can
    // free the temp buffer if the user address was bogus.
    //

    try {
        while (SystemPoolEntry < SystemPoolEntryEnd) {

            if (((ULONG_PTR)SystemPoolEntry->Va & POOL_BIG_TABLE_ENTRY_FREE) == 0) {

                //
                // This entry is in use so capture it.
                //

                UserPoolInfo->Count += 1;
                TotalBytes += sizeof (SYSTEM_BIGPOOL_ENTRY);

                if (SystemInformationLength < TotalBytes) {
                    Status = STATUS_INFO_LENGTH_MISMATCH;
                }
                else {
                    UserPoolEntry->VirtualAddress = SystemPoolEntry->Va;
                    
                    if (MmDeterminePoolType (SystemPoolEntry->Va) == NonPagedPool) {
                        UserPoolEntry->NonPaged = 1;
                    }

                    UserPoolEntry->TagUlong = SystemPoolEntry->Key & ~PROTECTED_POOL;
                    UserPoolEntry->SizeInBytes = SystemPoolEntry->NumberOfPages << PAGE_SHIFT;
                    UserPoolEntry += 1;
                }
            }
            SystemPoolEntry += 1;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode ();
    }

    MiFreePoolPages (NewTable);

    if (ARGUMENT_PRESENT(ReturnLength)) {
        *ReturnLength = TotalBytes;
    }

    return Status;
}

VOID
ExAllocatePoolSanityChecks (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes
    )

/*++

Routine Description:

    This function performs sanity checks on the caller.

Return Value:

    None.

Environment:

    Only enabled as part of the driver verification package.

--*/

{
    if (NumberOfBytes == 0) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x0,
                      KeGetCurrentIrql(),
                      PoolType,
                      NumberOfBytes);
    }

    if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {

        if (KeGetCurrentIrql() > APC_LEVEL) {

            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x1,
                          KeGetCurrentIrql(),
                          PoolType,
                          NumberOfBytes);
        }
    }
    else {
        if (KeGetCurrentIrql() > DISPATCH_LEVEL) {

            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x2,
                          KeGetCurrentIrql(),
                          PoolType,
                          NumberOfBytes);
        }
    }
}

VOID
ExFreePoolSanityChecks (
    IN PVOID P
    )

/*++

Routine Description:

    This function performs sanity checks on the caller.

Return Value:

    None.

Environment:

    Only enabled as part of the driver verification package.

--*/

{
    PPOOL_HEADER Entry;
    POOL_TYPE PoolType;
    PVOID StillQueued;

    if (P <= (PVOID)(MM_HIGHEST_USER_ADDRESS)) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x10,
                      (ULONG_PTR)P,
                      0,
                      0);
    }

    if ((ExpPoolFlags & EX_SPECIAL_POOL_ENABLED) &&
        (MmIsSpecialPoolAddress (P))) {

        KeCheckForTimer (P, PAGE_SIZE - BYTE_OFFSET (P));

        //
        // Check if an ERESOURCE is currently active in this memory block.
        //

        StillQueued = ExpCheckForResource(P, PAGE_SIZE - BYTE_OFFSET (P));
        if (StillQueued != NULL) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x17,
                          (ULONG_PTR)StillQueued,
                          (ULONG_PTR)-1,
                          (ULONG_PTR)P);
        }

        ExpCheckForWorker (P, PAGE_SIZE - BYTE_OFFSET (P)); // bugchecks inside
        return;
    }

    if (PAGE_ALIGNED(P)) {
        PoolType = MmDeterminePoolType(P);

        if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
            if (KeGetCurrentIrql() > APC_LEVEL) {
                KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                              0x11,
                              KeGetCurrentIrql(),
                              PoolType,
                              (ULONG_PTR)P);
            }
        }
        else {
            if (KeGetCurrentIrql() > DISPATCH_LEVEL) {
                KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                              0x12,
                              KeGetCurrentIrql(),
                              PoolType,
                              (ULONG_PTR)P);
            }
        }

        //
        // Just check the first page.
        //

        KeCheckForTimer(P, PAGE_SIZE);

        //
        // Check if an ERESOURCE is currently active in this memory block.
        //

        StillQueued = ExpCheckForResource(P, PAGE_SIZE);

        if (StillQueued != NULL) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x17,
                          (ULONG_PTR)StillQueued,
                          PoolType,
                          (ULONG_PTR)P);
        }
    }
    else {

        if (((ULONG_PTR)P & (POOL_OVERHEAD - 1)) != 0) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x16,
                          __LINE__,
                          (ULONG_PTR)P,
                          0);
        }

        Entry = (PPOOL_HEADER)((PCHAR)P - POOL_OVERHEAD);

        if ((Entry->PoolType & POOL_TYPE_MASK) == 0) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x13,
                          __LINE__,
                          (ULONG_PTR)Entry,
                          Entry->Ulong1);
        }

        PoolType = (Entry->PoolType & POOL_TYPE_MASK) - 1;

        if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
            if (KeGetCurrentIrql() > APC_LEVEL) {
                KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                              0x11,
                              KeGetCurrentIrql(),
                              PoolType,
                              (ULONG_PTR)P);
            }
        }
        else {
            if (KeGetCurrentIrql() > DISPATCH_LEVEL) {
                KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                              0x12,
                              KeGetCurrentIrql(),
                              PoolType,
                              (ULONG_PTR)P);
            }
        }

        if (!IS_POOL_HEADER_MARKED_ALLOCATED(Entry)) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x14,
                          __LINE__,
                          (ULONG_PTR)Entry,
                          0);
        }

        KeCheckForTimer(Entry, (ULONG)(Entry->BlockSize << POOL_BLOCK_SHIFT));

        //
        // Check if an ERESOURCE is currently active in this memory block.
        //

        StillQueued = ExpCheckForResource(Entry, (ULONG)(Entry->BlockSize << POOL_BLOCK_SHIFT));

        if (StillQueued != NULL) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x17,
                          (ULONG_PTR)StillQueued,
                          PoolType,
                          (ULONG_PTR)P);
        }
    }
}

#if defined (NT_UP)
VOID
ExpBootFinishedDispatch (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

/*++

Routine Description:

    This function is called when the system has booted into a shell.

    It's job is to disable various pool optimizations that are enabled to
    speed up booting and reduce the memory footprint on small machines.

Arguments:

    Dpc - Supplies a pointer to a control object of type DPC.

    DeferredContext - Optional deferred context;  not used.

    SystemArgument1 - Optional argument 1;  not used.

    SystemArgument2 - Optional argument 2;  not used.

Return Value:

    None.

Environment:

    DISPATCH_LEVEL since this is called from a timer expiration.

--*/

{
    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

    //
    // Pretty much all pages are "hot" after bootup.  Since bootup has finished,
    // use lookaside lists and stop trying to separate regular allocations
    // as well.
    //

    RtlInterlockedAndBitsDiscardReturn (&ExpPoolFlags, (ULONG)~EX_SEPARATE_HOT_PAGES_DURING_BOOT);
}
#endif

