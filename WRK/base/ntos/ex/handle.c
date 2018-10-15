/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    handle.c

Abstract:

    This module implements a set of functions for supporting handles.

Revision History:

    Support dynamic changes to the number of levels we use. The code
    performs the best for typical handle table sizes and scales better.

    Make the handle allocate, free and duplicate paths mostly lock free except
    for the lock entry locks, table expansion and locks to solve the A-B-A problem.

--*/

#include "exp.h"
#pragma hdrstop

//
//  Local constants and support routines
//

//
//  Define global structures that link all handle tables together except the
//  ones where the user has called RemoveHandleTable
//

#if DBG

BOOLEAN ExTraceAllTables = FALSE;

#define TRACE_ALL_TABLES ExTraceAllTables

#else

#define TRACE_ALL_TABLES FALSE

#endif

EX_PUSH_LOCK HandleTableListLock;

ULONG TotalTraceBuffers = 0;

#ifdef ALLOC_PRAGMA
#pragma data_seg("PAGED")
#endif

LIST_ENTRY HandleTableListHead;

#ifdef ALLOC_PRAGMA
#pragma data_seg()
#endif

#if DBG
#define EXHANDLE_EXTRA_CHECKS 0
#else
#define EXHANDLE_EXTRA_CHECKS 0
#endif

#if EXHANDLE_EXTRA_CHECKS

#define EXASSERT( exp ) \
    ((!(exp)) ? \
        (RtlAssert( #exp, __FILE__, __LINE__, NULL ),FALSE) : \
        TRUE)

#else

#define EXASSERT ASSERT

#endif

//
//  This is the sign low bit used to lock handle table entries
//

#define EXHANDLE_TABLE_ENTRY_LOCK_BIT    1

#define EX_ADDITIONAL_INFO_SIGNATURE (-2)

#define ExpIsValidObjectEntry(Entry) \
    ( (Entry != NULL) && (Entry->Object != NULL) && (Entry->NextFreeTableEntry != EX_ADDITIONAL_INFO_SIGNATURE) )


#define TABLE_PAGE_SIZE PAGE_SIZE

//
// Absolute maximum number of handles allowed
//
#define MAX_HANDLES (1<<24)

#if EXHANDLE_EXTRA_CHECKS

//
// Mask for next free value from the free lists.
//
#define FREE_HANDLE_MASK ((MAX_HANDLES<<2) - 1)

#else

//
// When no checks compiled in this gets optimized away
//
#define FREE_HANDLE_MASK 0xFFFFFFFF

#endif

//
// Mask for the free list sequence number
//
#define FREE_SEQ_MASK (0xFFFFFFFF & ~FREE_HANDLE_MASK)


#if (FREE_HANDLE_MASK == 0xFFFFFFFF)
#define FREE_SEQ_INC 0
#define GetNextSeq() 0
#else
//
// Increment value to progress the sequence number
//
#define FREE_SEQ_INC  (FREE_HANDLE_MASK + 1)
ULONG CurrentSeq = 0;
#define GetNextSeq() (CurrentSeq += FREE_SEQ_INC)
#endif



#define LOWLEVEL_COUNT (TABLE_PAGE_SIZE / sizeof(HANDLE_TABLE_ENTRY))
#define MIDLEVEL_COUNT (PAGE_SIZE / sizeof(PHANDLE_TABLE_ENTRY))
#define HIGHLEVEL_COUNT  MAX_HANDLES / (LOWLEVEL_COUNT * MIDLEVEL_COUNT)

#define LOWLEVEL_THRESHOLD LOWLEVEL_COUNT
#define MIDLEVEL_THRESHOLD (MIDLEVEL_COUNT * LOWLEVEL_COUNT)
#define HIGHLEVEL_THRESHOLD (MIDLEVEL_COUNT * MIDLEVEL_COUNT * LOWLEVEL_COUNT)

#define HIGHLEVEL_SIZE (HIGHLEVEL_COUNT * sizeof (PHANDLE_TABLE_ENTRY))

#define LEVEL_CODE_MASK 3

//
//  Local support routines
//

PHANDLE_TABLE
ExpAllocateHandleTable (
    IN PEPROCESS Process OPTIONAL,
    IN BOOLEAN DoInit
    );

VOID
ExpFreeHandleTable (
    IN PHANDLE_TABLE HandleTable
    );

BOOLEAN
ExpAllocateHandleTableEntrySlow (
    IN PHANDLE_TABLE HandleTable,
    IN BOOLEAN DoInit
    );

PHANDLE_TABLE_ENTRY
ExpAllocateHandleTableEntry (
    IN PHANDLE_TABLE HandleTable,
    OUT PEXHANDLE Handle
    );

VOID
ExpFreeHandleTableEntry (
    IN PHANDLE_TABLE HandleTable,
    IN EXHANDLE Handle,
    IN PHANDLE_TABLE_ENTRY HandleTableEntry
    );

PHANDLE_TABLE_ENTRY
ExpLookupHandleTableEntry (
    IN PHANDLE_TABLE HandleTable,
    IN EXHANDLE Handle
    );

PHANDLE_TABLE_ENTRY *
ExpAllocateMidLevelTable (
    IN PHANDLE_TABLE HandleTable,
    IN BOOLEAN DoInit,
    OUT PHANDLE_TABLE_ENTRY *pNewLowLevel
    );

PVOID
ExpAllocateTablePagedPool (
    IN PEPROCESS QuotaProcess OPTIONAL,
    IN SIZE_T NumberOfBytes
    );

VOID
ExpFreeTablePagedPool (
    IN PEPROCESS QuotaProcess OPTIONAL,
    IN PVOID PoolMemory,
    IN SIZE_T NumberOfBytes
    );

PHANDLE_TABLE_ENTRY
ExpAllocateLowLevelTable (
    IN PHANDLE_TABLE HandleTable,
    IN BOOLEAN DoInit
    );

VOID
ExpFreeLowLevelTable (
    IN PEPROCESS QuotaProcess,
    IN PHANDLE_TABLE_ENTRY TableLevel1
    );

VOID
ExpBlockOnLockedHandleEntry (
    PHANDLE_TABLE HandleTable,
    PHANDLE_TABLE_ENTRY HandleTableEntry
    );

ULONG
ExpMoveFreeHandles (
    IN PHANDLE_TABLE HandleTable
    );

VOID
ExpUpdateDebugInfo(
    PHANDLE_TABLE HandleTable,
    PETHREAD CurrentThread,
    HANDLE Handle,
    ULONG Type
    );

PVOID
ExpAllocateTablePagedPoolNoZero (
    IN PEPROCESS QuotaProcess OPTIONAL,
    IN SIZE_T NumberOfBytes
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, ExInitializeHandleTablePackage)
#pragma alloc_text(INIT, ExSetHandleTableStrictFIFO)
#pragma alloc_text(PAGE, ExUnlockHandleTableEntry)
#pragma alloc_text(PAGE, ExCreateHandleTable)
#pragma alloc_text(PAGE, ExRemoveHandleTable)
#pragma alloc_text(PAGE, ExDestroyHandleTable)
#pragma alloc_text(PAGE, ExEnumHandleTable)
#pragma alloc_text(PAGE, ExDupHandleTable)
#pragma alloc_text(PAGE, ExSnapShotHandleTables)
#pragma alloc_text(PAGE, ExCreateHandle)
#pragma alloc_text(PAGE, ExDestroyHandle)
#pragma alloc_text(PAGE, ExChangeHandle)
#pragma alloc_text(PAGE, ExMapHandleToPointer)
#pragma alloc_text(PAGE, ExMapHandleToPointerEx)
#pragma alloc_text(PAGE, ExpAllocateHandleTable)
#pragma alloc_text(PAGE, ExpFreeHandleTable)
#pragma alloc_text(PAGE, ExpAllocateHandleTableEntry)
#pragma alloc_text(PAGE, ExpAllocateHandleTableEntrySlow)
#pragma alloc_text(PAGE, ExpFreeHandleTableEntry)
#pragma alloc_text(PAGE, ExpLookupHandleTableEntry)
#pragma alloc_text(PAGE, ExSweepHandleTable)
#pragma alloc_text(PAGE, ExpAllocateMidLevelTable)
#pragma alloc_text(PAGE, ExpAllocateTablePagedPool)
#pragma alloc_text(PAGE, ExpAllocateTablePagedPoolNoZero)
#pragma alloc_text(PAGE, ExpFreeTablePagedPool)
#pragma alloc_text(PAGE, ExpAllocateLowLevelTable)
#pragma alloc_text(PAGE, ExSetHandleInfo)
#pragma alloc_text(PAGE, ExpGetHandleInfo)
#pragma alloc_text(PAGE, ExSnapShotHandleTablesEx)
#pragma alloc_text(PAGE, ExpFreeLowLevelTable)
#pragma alloc_text(PAGE, ExpBlockOnLockedHandleEntry)
#pragma alloc_text(PAGE, ExpMoveFreeHandles)
#pragma alloc_text(PAGE, ExEnableHandleTracing)
#pragma alloc_text(PAGE, ExDereferenceHandleDebugInfo)
#pragma alloc_text(PAGE, ExReferenceHandleDebugInfo)
#pragma alloc_text(PAGE, ExpUpdateDebugInfo)
#endif

//
// Define macros to lock and unlock the handle table.
// We use this lock only for handle table expansion.
//
#define ExpLockHandleTableExclusive(xxHandleTable,xxCurrentThread) { \
    KeEnterCriticalRegionThread (xxCurrentThread);                   \
    ExAcquirePushLockExclusive (&xxHandleTable->HandleTableLock[0]); \
}


#define ExpUnlockHandleTableExclusive(xxHandleTable,xxCurrentThread) { \
    ExReleasePushLockExclusive (&xxHandleTable->HandleTableLock[0]);   \
    KeLeaveCriticalRegionThread (xxCurrentThread);                     \
}
    
#define ExpLockHandleTableShared(xxHandleTable,xxCurrentThread,xxIdx) { \
    KeEnterCriticalRegionThread (xxCurrentThread);                      \
    ExAcquirePushLockShared (&xxHandleTable->HandleTableLock[xxIdx]);   \
}


#define ExpUnlockHandleTableShared(xxHandleTable,xxCurrentThread,xxIdx) { \
    ExReleasePushLockShared (&xxHandleTable->HandleTableLock[xxIdx]);     \
    KeLeaveCriticalRegionThread (xxCurrentThread);                        \
}



FORCEINLINE
ULONG
ExpInterlockedExchange (
    IN OUT PULONG Index,
    IN ULONG FirstIndex,
    IN PHANDLE_TABLE_ENTRY Entry
    )
/*++

Routine Description:

    This performs the following steps:
    1. Set Entry->NextFreeTableEntry = *Index
    2. Loops until *Index == (the value of *Index when we entered the function)
       When they're equal, we set *Index = FirstIndex


Arguments:

    Index - Points to the ULONG we want to set.
    
    FirstIndex - New value to set Index to.

    Entry - TableEntry that will get the initial value of *Index before it's
            updated.

Return Value:

    New value of *Index (i.e. FirstIndex).

--*/
{
    ULONG OldIndex, NewIndex;

    EXASSERT (Entry->Object == NULL);

    //
    // Load new value and generate the sequence number on pushes
    //

    NewIndex = FirstIndex + GetNextSeq();

    while (1) {

        //
        // remember original value and
        // archive it in NextFreeTableEntry.
        //

        OldIndex = ReadForWriteAccess (Index);
        Entry->NextFreeTableEntry = OldIndex;

        
        //
        // Swap in the new value, and if the swap occurs
        // successfully, we're done.
        //
        if (OldIndex == (ULONG) InterlockedCompareExchange ((PLONG)Index,
                                                            NewIndex,
                                                            OldIndex)) {
            return OldIndex;
        }
    }
}

ULONG
ExpMoveFreeHandles (
    IN PHANDLE_TABLE HandleTable
    )
{
    ULONG OldValue, NewValue;
    ULONG Index, OldIndex, NewIndex, FreeSize;
    PHANDLE_TABLE_ENTRY Entry, FirstEntry;
    EXHANDLE Handle;
    ULONG Idx;
    BOOLEAN StrictFIFO;

    //
    // First remove all the handles from the free list so we can add them to the
    // list we use for allocates.
    //

    OldValue = InterlockedExchange ((PLONG)&HandleTable->LastFree,
                                    0);
    Index = OldValue;
    if (Index == 0) {
        //
        // There are no free handles.  Nothing to do.
        //
        return OldValue;
    }

       
    //
    // We are pushing old entries onto the free list.
    // We have the A-B-A problem here as these items may have been moved here because
    // another thread was using them in the pop code.
    //
    for (Idx = 1; Idx < HANDLE_TABLE_LOCKS; Idx++) {
        ExAcquireReleasePushLockExclusive (&HandleTable->HandleTableLock[Idx]);
    }
    StrictFIFO = HandleTable->StrictFIFO;
 
    //
    // If we are strict FIFO then reverse the list to make handle reuse rare.
    //
    if (!StrictFIFO) {
        //
        // We have a complete chain here. If there is no existing chain we
        // can just push this one without any hassles. If we can't then
        // we can just fall into the reversing code anyway as we need
        // to find the end of the chain to continue it.
        //

        //
        // This is a push so create a new sequence number
        //

        if (InterlockedCompareExchange ((PLONG)&HandleTable->FirstFree,
                                        OldValue + GetNextSeq(),
                                        0) == 0) {
            return OldValue;
        }
    }

    //
    // Loop over all the entries and reverse the chain.
    //
    FreeSize = OldIndex = 0;
    FirstEntry = NULL;
    while (1) {
        FreeSize++;
        Handle.Value = Index;
        Entry = ExpLookupHandleTableEntry (HandleTable, Handle);

        EXASSERT (Entry->Object == NULL);

        NewIndex = Entry->NextFreeTableEntry;
        Entry->NextFreeTableEntry = OldIndex;
        if (OldIndex == 0) {
            FirstEntry = Entry;
        }
        OldIndex = Index;
        if (NewIndex == 0) {
            break;
        }
        Index = NewIndex;
    }

    NewValue = ExpInterlockedExchange (&HandleTable->FirstFree,
                                       OldIndex,
                                       FirstEntry);

    //
    // If we haven't got a pool of a few handles then force
    // table expansion to keep the free handle size high
    //
    if (FreeSize < 100 && StrictFIFO) {
        OldValue = 0;
    }
    return OldValue;
}

PHANDLE_TABLE_ENTRY
ExpAllocateHandleTableEntry (
    IN PHANDLE_TABLE HandleTable,
    OUT PEXHANDLE pHandle
    )
/*++

Routine Description:

    This routine does a fast allocate of a free handle. It's lock free if
    possible.

    Only the rare case of handle table expansion is covered by the handle
    table lock.

Arguments:

    HandleTable - Supplies the handle table being allocated from.

    pHandle - Handle returned

Return Value:

    PHANDLE_TABLE_ENTRY - The allocated handle table entry pointer or NULL
                          on failure.

--*/
{
    PKTHREAD CurrentThread;
    ULONG OldValue, NewValue, NewValue1;
    PHANDLE_TABLE_ENTRY Entry;
    EXHANDLE Handle;
    BOOLEAN RetVal;
    ULONG Idx;


    CurrentThread = KeGetCurrentThread ();
    while (1) {

        OldValue = HandleTable->FirstFree;


        while (OldValue == 0) {
            //
            //  Lock the handle table for exclusive access as we will be
            //  allocating a new table level.
            //
            ExpLockHandleTableExclusive (HandleTable, CurrentThread);

            //
            // If we have multiple threads trying to expand the table at
            // the same time then by just acquiring the table lock we
            // force those threads to complete their allocations and
            // populate the free list. We must check the free list here
            // so we don't expand the list twice without needing to.
            //

            OldValue = HandleTable->FirstFree;
            if (OldValue != 0) {
                ExpUnlockHandleTableExclusive (HandleTable, CurrentThread);
                break;
            }

            //
            // See if we have any handles on the alternate free list
            // These handles need some locking to move them over.
            //
            OldValue = ExpMoveFreeHandles (HandleTable);
            if (OldValue != 0) {
                ExpUnlockHandleTableExclusive (HandleTable, CurrentThread);
                break;
            }

            //
            // This must be the first thread attempting expansion or all the
            // free handles allocated by another thread got used up in the gap.
            //

            RetVal = ExpAllocateHandleTableEntrySlow (HandleTable, TRUE);

            ExpUnlockHandleTableExclusive (HandleTable, CurrentThread);


            OldValue = HandleTable->FirstFree;

            //
            // If ExpAllocateHandleTableEntrySlow had a failed allocation
            // then we want to fail the call.  We check for free entries
            // before we exit just in case they got allocated or freed by
            // somebody else in the gap.
            //

            if (!RetVal) {
                if (OldValue == 0) {
                    pHandle->GenericHandleOverlay = NULL;
                    return NULL;
                }            
            }
        }


        Handle.Value = (OldValue & FREE_HANDLE_MASK);

        Entry = ExpLookupHandleTableEntry (HandleTable, Handle);

        Idx = ((OldValue & FREE_HANDLE_MASK)>>2) % HANDLE_TABLE_LOCKS;
        ExpLockHandleTableShared (HandleTable, CurrentThread, Idx);

        if (OldValue != *(volatile ULONG *) &HandleTable->FirstFree) {
            ExpUnlockHandleTableShared (HandleTable, CurrentThread, Idx);
            continue;
        }

        KeMemoryBarrier ();

        NewValue = *(volatile ULONG *) &Entry->NextFreeTableEntry;

        NewValue1 = InterlockedCompareExchange ((PLONG)&HandleTable->FirstFree,
                                                NewValue,
                                                OldValue);

        ExpUnlockHandleTableShared (HandleTable, CurrentThread, Idx);

        if (NewValue1 == OldValue) {
            EXASSERT ((NewValue & FREE_HANDLE_MASK) < HandleTable->NextHandleNeedingPool);
            break;
        } else {
            //
            // We should have eliminated the A-B-A problem so if only the sequence number has
            // changed we are broken.
            //
            EXASSERT ((NewValue1 & FREE_HANDLE_MASK) != (OldValue & FREE_HANDLE_MASK));
        }
    }
    InterlockedIncrement (&HandleTable->HandleCount);

    *pHandle = Handle;
    
    return Entry;
}


VOID
ExpBlockOnLockedHandleEntry (
    PHANDLE_TABLE HandleTable,
    PHANDLE_TABLE_ENTRY HandleTableEntry
    )
{
    EX_PUSH_LOCK_WAIT_BLOCK WaitBlock;
    LONG_PTR CurrentValue;

    //
    // Queue our wait block to be signaled by a releasing thread.
    //

    ExBlockPushLock (&HandleTable->HandleContentionEvent, &WaitBlock);

    CurrentValue = HandleTableEntry->Value;
    if (CurrentValue == 0 || (CurrentValue&EXHANDLE_TABLE_ENTRY_LOCK_BIT) != 0) {
        ExUnblockPushLock (&HandleTable->HandleContentionEvent, &WaitBlock);
    } else {
        ExWaitForUnblockPushLock (&HandleTable->HandleContentionEvent, &WaitBlock);
   }
}


BOOLEAN
FORCEINLINE
ExpLockHandleTableEntry (
    PHANDLE_TABLE HandleTable,
    PHANDLE_TABLE_ENTRY HandleTableEntry
    )

/*++

Routine Description:

    This routine locks the specified handle table entry.  After the entry is
    locked the sign bit will be set.

Arguments:

    HandleTable - Supplies the handle table containing the entry being locked.

    HandleTableEntry - Supplies the handle table entry being locked.

Return Value:

    TRUE if the entry is valid and locked, and FALSE if the entry is
    marked free.

--*/

{
    LONG_PTR NewValue;
    LONG_PTR CurrentValue;

    //
    // We are about to take a lock. Make sure we are protected.
    //
    ASSERT ((KeGetCurrentThread()->CombinedApcDisable != 0) || (KeGetCurrentIrql() == APC_LEVEL));

    //
    //  We'll keep on looping reading in the value, making sure it is not null,
    //  and if it is not currently locked we'll try for the lock and return
    //  true if we get it.  Otherwise we'll pause a bit and then try again.
    //


    while (TRUE) {

        CurrentValue = ReadForWriteAccess(((volatile LONG_PTR *)&HandleTableEntry->Object));

        //
        //  If the handle value is greater than zero then it is not currently
        //  locked and we should try for the lock, by setting the lock bit and
        //  doing an interlocked exchange.
        //

        if (CurrentValue & EXHANDLE_TABLE_ENTRY_LOCK_BIT) {

            //
            // Remove the
            //
            NewValue = CurrentValue - EXHANDLE_TABLE_ENTRY_LOCK_BIT;

            if ((LONG_PTR)(InterlockedCompareExchangePointer (&HandleTableEntry->Object,
                                                              (PVOID)NewValue,
                                                              (PVOID)CurrentValue)) == CurrentValue) {

                return TRUE;
            }
        } else {
            //
            //  Make sure the handle table entry is not freed
            //

            if (CurrentValue == 0) {

                return FALSE;
            }
        }
        ExpBlockOnLockedHandleEntry (HandleTable, HandleTableEntry);
    }
}


NTKERNELAPI
VOID
FORCEINLINE
ExUnlockHandleTableEntry (
    __inout PHANDLE_TABLE HandleTable,
    __inout PHANDLE_TABLE_ENTRY HandleTableEntry
    )

/*++

Routine Description:

    This routine unlocks the specified handle table entry.  After the entry is
    unlocked the sign bit will be clear.

Arguments:

    HandleTable - Supplies the handle table containing the entry being unlocked.

    HandleTableEntry - Supplies the handle table entry being unlocked.

Return Value:

    None.

--*/

{
    LONG_PTR OldValue;

    PAGED_CODE();

    //
    // We are about to release a lock. Make sure we are protected from suspension.
    //
    ASSERT ((KeGetCurrentThread()->CombinedApcDisable != 0) || (KeGetCurrentIrql() == APC_LEVEL));

    //
    //  This routine does not need to loop and attempt the unlock operation more
    //  than once because by definition the caller has the entry already locked
    //  and no one can be changing the value without the lock.
    //


#if defined (_WIN64)

    OldValue = InterlockedExchangeAdd64 ((PLONGLONG) &HandleTableEntry->Value, EXHANDLE_TABLE_ENTRY_LOCK_BIT);


#else

    OldValue = InterlockedOr ((LONG *) &HandleTableEntry->Value, EXHANDLE_TABLE_ENTRY_LOCK_BIT);


#endif

    EXASSERT ((OldValue&EXHANDLE_TABLE_ENTRY_LOCK_BIT) == 0);

    //
    // Unblock any waiters waiting for this table entry.
    //
    ExUnblockPushLock (&HandleTable->HandleContentionEvent, NULL);

    return;
}


NTKERNELAPI
VOID
ExInitializeHandleTablePackage (
    VOID
    )

/*++

Routine Description:

    This routine is called once at system initialization to setup the ex handle
    table package

Arguments:

    None.

Return Value:

    None.

--*/

{
    //
    //  Initialize the handle table synchronization resource and list head
    //

    InitializeListHead( &HandleTableListHead );
    ExInitializePushLock( &HandleTableListLock );

    return;
}


NTKERNELAPI
PHANDLE_TABLE
ExCreateHandleTable (
    __in_opt struct _EPROCESS *Process
    )

/*++

Routine Description:

    This function allocate and initialize a new new handle table

Arguments:

    Process - Supplies an optional pointer to the process against which quota
        will be charged.

Return Value:

    If a handle table is successfully created, then the address of the
    handle table is returned as the function value. Otherwise, a value
    NULL is returned.

--*/

{
    PKTHREAD CurrentThread;
    PHANDLE_TABLE HandleTable;

    PAGED_CODE();

    CurrentThread = KeGetCurrentThread ();

    //
    //  Allocate and initialize a handle table descriptor
    //

    HandleTable = ExpAllocateHandleTable( Process, TRUE );

    if (HandleTable == NULL) {
        return NULL;
    }
    //
    //  Insert the handle table in the handle table list.
    //

    KeEnterCriticalRegionThread (CurrentThread);
    ExAcquirePushLockExclusive( &HandleTableListLock );

    InsertTailList( &HandleTableListHead, &HandleTable->HandleTableList );

    ExReleasePushLockExclusive( &HandleTableListLock );
    KeLeaveCriticalRegionThread (CurrentThread);


    //
    //  And return to our caller
    //

    return HandleTable;
}


NTKERNELAPI
VOID
ExRemoveHandleTable (
    __inout PHANDLE_TABLE HandleTable
    )

/*++

Routine Description:

    This function removes the specified exhandle table from the list of
    exhandle tables.  Used by PS and ATOM packages to make sure their handle
    tables are not in the list enumerated by the ExSnapShotHandleTables
    routine and the !handle debugger extension.

Arguments:

    HandleTable - Supplies a pointer to a handle table

Return Value:

    None.

--*/

{
    PKTHREAD CurrentThread;

    PAGED_CODE();

    CurrentThread = KeGetCurrentThread ();

    //
    //  First, acquire the global handle table lock
    //

    KeEnterCriticalRegionThread (CurrentThread);
    ExAcquirePushLockExclusive( &HandleTableListLock );

    //
    //  Remove the handle table from the handle table list.  This routine is
    //  written so that multiple calls to remove a handle table will not
    //  corrupt the system.
    //

    RemoveEntryList( &HandleTable->HandleTableList );
    InitializeListHead( &HandleTable->HandleTableList );

    //
    //  Now release the global lock and return to our caller
    //

    ExReleasePushLockExclusive( &HandleTableListLock );
    KeLeaveCriticalRegionThread (CurrentThread);

    return;
}


NTKERNELAPI
VOID
ExDestroyHandleTable (
    __inout PHANDLE_TABLE HandleTable,
    __in EX_DESTROY_HANDLE_ROUTINE DestroyHandleProcedure OPTIONAL
    )

/*++

Routine Description:

    This function destroys the specified handle table.

Arguments:

    HandleTable - Supplies a pointer to a handle table

    DestroyHandleProcedure - Supplies a pointer to a function to call for each
        valid handle entry in the handle table.

Return Value:

    None.

--*/

{
    EXHANDLE Handle;
    PHANDLE_TABLE_ENTRY HandleTableEntry;

    PAGED_CODE();

    //
    //  Remove the handle table from the handle table list
    //

    ExRemoveHandleTable( HandleTable );

    //
    //  Iterate through the handle table and for each handle that is allocated
    //  we'll invoke the call back.  Note that this loop exits when we get a
    //  null handle table entry.  We know there will be no more possible
    //  entries after the first null one is encountered because we allocate
    //  memory of the handles in a dense fashion.  But first test that we have
    //  call back to use
    //

    if (ARGUMENT_PRESENT(DestroyHandleProcedure)) {

        for (Handle.Value = 0;
             (HandleTableEntry = ExpLookupHandleTableEntry( HandleTable, Handle )) != NULL;
             Handle.Value += HANDLE_VALUE_INC) {

            //
            //  Only do the callback if the entry is not free
            //

            if ( ExpIsValidObjectEntry(HandleTableEntry) ) {

                (*DestroyHandleProcedure)( Handle.GenericHandleOverlay );
            }
        }
    }

    //
    //  Now free up the handle table memory and return to our caller
    //

    ExpFreeHandleTable( HandleTable );

    return;
}


NTKERNELAPI
VOID
ExSweepHandleTable (
    __in PHANDLE_TABLE HandleTable,
    __in EX_ENUMERATE_HANDLE_ROUTINE EnumHandleProcedure,
    __in PVOID EnumParameter
    )

/*++

Routine Description:

    This function sweeps a handle table in a unsynchronized manner.

Arguments:

    HandleTable - Supplies a pointer to a handle table

    EnumHandleProcedure - Supplies a pointer to a function to call for
        each valid handle in the enumerated handle table.

    EnumParameter - Supplies an uninterpreted 32-bit value that is passed
        to the EnumHandleProcedure each time it is called.

Return Value:

    None.

--*/

{
    EXHANDLE Handle;
    PHANDLE_TABLE_ENTRY HandleTableEntry;

    PAGED_CODE();

    //
    //  Iterate through the handle table and for each handle that is allocated
    //  we'll invoke the call back.  Note that this loop exits when we get a
    //  null handle table entry.  We know there will be no more possible
    //  entries after the first null one is encountered because we allocate
    //  memory of the handles in a dense fashion.
    //
    Handle.Value = HANDLE_VALUE_INC;

    while ((HandleTableEntry = ExpLookupHandleTableEntry( HandleTable, Handle )) != NULL) {

        do {

            //
            //  Only do the callback if the entry is not free
            //
            //

            if (ExpLockHandleTableEntry( HandleTable, HandleTableEntry )) {

                (*EnumHandleProcedure)( HandleTableEntry,
                                        Handle.GenericHandleOverlay,
                                        EnumParameter );
            }
            Handle.Value += HANDLE_VALUE_INC;
            HandleTableEntry++;
        } while ((Handle.Value % (LOWLEVEL_COUNT * HANDLE_VALUE_INC)) != 0);
        // Skip past the first entry that's not a real entry
        Handle.Value += HANDLE_VALUE_INC;
    }

    return;
}



NTKERNELAPI
BOOLEAN
ExEnumHandleTable (
    __in PHANDLE_TABLE HandleTable,
    __in EX_ENUMERATE_HANDLE_ROUTINE EnumHandleProcedure,
    __in PVOID EnumParameter,
    __out_opt PHANDLE Handle
    )

/*++

Routine Description:

    This function enumerates all the valid handles in a handle table.
    For each valid handle in the handle table, the specified eumeration
    function is called. If the enumeration function returns TRUE, then
    the enumeration is stopped, the current handle is returned to the
    caller via the optional Handle parameter, and this function returns
    TRUE to indicated that the enumeration stopped at a specific handle.

Arguments:

    HandleTable - Supplies a pointer to a handle table.

    EnumHandleProcedure - Supplies a pointer to a function to call for
        each valid handle in the enumerated handle table.

    EnumParameter - Supplies an uninterpreted 32-bit value that is passed
        to the EnumHandleProcedure each time it is called.

    Handle - Supplies an optional pointer a variable that receives the
        Handle value that the enumeration stopped at. Contents of the
        variable only valid if this function returns TRUE.

Return Value:

    If the enumeration stopped at a specific handle, then a value of TRUE
    is returned. Otherwise, a value of FALSE is returned.

--*/

{
    PKTHREAD CurrentThread;
    BOOLEAN ResultValue;
    EXHANDLE LocalHandle;
    PHANDLE_TABLE_ENTRY HandleTableEntry;

    PAGED_CODE();

    CurrentThread = KeGetCurrentThread ();

    //
    //  Our initial return value is false until the enumeration callback
    //  function tells us otherwise
    //

    ResultValue = FALSE;

    //
    //  Iterate through the handle table and for each handle that is
    //  allocated we'll invoke the call back.  Note that this loop exits
    //  when we get a null handle table entry.  We know there will be no
    //  more possible entries after the first null one is encountered
    //  because we allocate memory for the handles in a dense fashion
    //

    KeEnterCriticalRegionThread (CurrentThread);

    for (LocalHandle.Value = 0; // does essentially the following "LocalHandle.Index = 0, LocalHandle.TagBits = 0;"
         (HandleTableEntry = ExpLookupHandleTableEntry( HandleTable, LocalHandle )) != NULL;
         LocalHandle.Value += HANDLE_VALUE_INC) {

        //
        //  Only do the callback if the entry is not free
        //

        if ( ExpIsValidObjectEntry( HandleTableEntry ) ) {

            //
            //  Lock the handle table entry because we're about to give
            //  it to the callback function, then release the entry
            //  right after the call back.
            //

            if (ExpLockHandleTableEntry( HandleTable, HandleTableEntry )) {

                //
                //  Invoke the callback, and if it returns true then set
                //  the proper output values and break out of the loop.
                //

                ResultValue = (*EnumHandleProcedure)( HandleTableEntry,
                                                      LocalHandle.GenericHandleOverlay,
                                                      EnumParameter );

                ExUnlockHandleTableEntry( HandleTable, HandleTableEntry );

                if (ResultValue) {
                    if (ARGUMENT_PRESENT( Handle )) {

                        *Handle = LocalHandle.GenericHandleOverlay;
                    }
                    break;
                }
            }
        }
    }
    KeLeaveCriticalRegionThread (CurrentThread);


    return ResultValue;
}


NTKERNELAPI
PHANDLE_TABLE
ExDupHandleTable (
    __inout_opt struct _EPROCESS *Process,
    __in PHANDLE_TABLE OldHandleTable,
    __in EX_DUPLICATE_HANDLE_ROUTINE DupHandleProcedure,
    __in ULONG_PTR Mask
    )

/*++

Routine Description:

    This function creates a duplicate copy of the specified handle table.

Arguments:

    Process - Supplies an optional to the process to charge quota to.

    OldHandleTable - Supplies a pointer to a handle table.

    DupHandleProcedure - Supplies an optional pointer to a function to call
        for each valid handle in the duplicated handle table.

    Mask - Mask applied to the object pointer to work outif we need to duplicate

Return Value:

    If the specified handle table is successfully duplicated, then the
    address of the new handle table is returned as the function value.
    Otherwise, a value NULL is returned.

--*/

{
    PKTHREAD CurrentThread;
    PHANDLE_TABLE NewHandleTable;
    EXHANDLE Handle;
    PHANDLE_TABLE_ENTRY OldHandleTableEntry;
    PHANDLE_TABLE_ENTRY NewHandleTableEntry;
    BOOLEAN FreeEntry;
    NTSTATUS Status;

    PAGED_CODE();

    CurrentThread = KeGetCurrentThread ();

    //
    //  First allocate a new handle table.  If this fails then
    //  return immediately to our caller
    //

    NewHandleTable = ExpAllocateHandleTable( Process, FALSE );

    if (NewHandleTable == NULL) {

        return NULL;
    }


    //
    //  Now we'll build up the new handle table. We do this by calling
    //  allocating new handle table entries, and "fooling" the worker
    //  routine to allocate keep on allocating until the next free
    //  index needing pool are equal
    //
    while (NewHandleTable->NextHandleNeedingPool < OldHandleTable->NextHandleNeedingPool) {

        //
        //  Call the worker routine to grow the new handle table.  If
        //  not successful then free the new table as far as we got,
        //  set our output variable and exit out here
        //
        if (!ExpAllocateHandleTableEntrySlow (NewHandleTable, FALSE)) {

            ExpFreeHandleTable (NewHandleTable);
            return NULL;
        }
    }

    //
    // Make sure any table reads occur after the value we fetched from NextHandleNeedingPool
    //

    KeMemoryBarrier ();

    //
    //  Now modify the new handle table to think it has zero handles
    //  and set its free list to start on the same index as the old
    //  free list
    //

    NewHandleTable->HandleCount = 0;
    NewHandleTable->ExtraInfoPages = 0;
    NewHandleTable->FirstFree = 0;

    //
    //  Now for every valid index value we'll copy over the old entry into
    //  the new entry
    //


    Handle.Value = HANDLE_VALUE_INC;

    KeEnterCriticalRegionThread (CurrentThread);
    while ((NewHandleTableEntry = ExpLookupHandleTableEntry( NewHandleTable, Handle )) != NULL) {

        //
        // Lookup the old entry.
        //

        OldHandleTableEntry = ExpLookupHandleTableEntry( OldHandleTable, Handle );

        do {

            //
            //  If the old entry is free then simply copy over the entire
            //  old entry to the new entry.  The lock command will tell us
            //  if the entry is free.
            //
            if ((OldHandleTableEntry->Value&Mask) == 0 ||
                !ExpLockHandleTableEntry( OldHandleTable, OldHandleTableEntry )) {
                FreeEntry = TRUE;
            } else {

                PHANDLE_TABLE_ENTRY_INFO EntryInfo;
                
                //
                //  Otherwise we have a non empty entry.  So now copy it
                //  over, and unlock the old entry.  In both cases we bump
                //  the handle count because either the entry is going into
                //  the new table or we're going to remove it with Exp Free
                //  Handle Table Entry which will decrement the handle count
                //

                *NewHandleTableEntry = *OldHandleTableEntry;

                //
                //  Copy the entry info data, if any
                //

                Status = STATUS_SUCCESS;
                EntryInfo = ExGetHandleInfo(OldHandleTable, Handle.GenericHandleOverlay, TRUE);

                if (EntryInfo) {

                    Status = ExSetHandleInfo(NewHandleTable, Handle.GenericHandleOverlay, EntryInfo, TRUE);
                }


                //
                //  Invoke the callback and if it returns true then we
                //  unlock the new entry
                //

                if (NT_SUCCESS (Status)) {
                    if  ((*DupHandleProcedure) (Process,
                                                OldHandleTable,
                                                OldHandleTableEntry,
                                                NewHandleTableEntry)) {

                        if (NewHandleTable->DebugInfo != NULL) {
                            ExpUpdateDebugInfo(
                                NewHandleTable,
                                PsGetCurrentThread (),
                                Handle.GenericHandleOverlay,
                                HANDLE_TRACE_DB_OPEN);
                        }
                        //
                        // Since there is no route to the new table yet we can just
                        // clear the lock bit
                        //
                        NewHandleTableEntry->Value |= EXHANDLE_TABLE_ENTRY_LOCK_BIT;
                        NewHandleTable->HandleCount += 1;
                        FreeEntry = FALSE;
                    } else {
                        if (EntryInfo) {
                            EntryInfo->AuditMask = 0;
                        }

                        FreeEntry = TRUE;
                    }
                } else {
                    //
                    // Duplicate routine doesn't want this handle duplicated so free it
                    //
                    ExUnlockHandleTableEntry( OldHandleTable, OldHandleTableEntry );
                    FreeEntry = TRUE;
                }

            }
            if (FreeEntry) {
                NewHandleTableEntry->Object = NULL;
                NewHandleTableEntry->NextFreeTableEntry =
                    NewHandleTable->FirstFree;
                NewHandleTable->FirstFree = (ULONG) Handle.Value;
            }
            Handle.Value += HANDLE_VALUE_INC;
            NewHandleTableEntry++;
            OldHandleTableEntry++;

        } while ((Handle.Value % (LOWLEVEL_COUNT * HANDLE_VALUE_INC)) != 0);

        Handle.Value += HANDLE_VALUE_INC; // Skip past the first entry thats not a real entry
    }

    //
    //  Insert the handle table in the handle table list.
    //

    ExAcquirePushLockExclusive( &HandleTableListLock );

    InsertTailList( &HandleTableListHead, &NewHandleTable->HandleTableList );

    ExReleasePushLockExclusive( &HandleTableListLock );
    KeLeaveCriticalRegionThread (CurrentThread);

    //
    //  lastly return the new handle table to our caller
    //

    return NewHandleTable;
}


NTKERNELAPI
NTSTATUS
ExSnapShotHandleTables (
    __in PEX_SNAPSHOT_HANDLE_ENTRY SnapShotHandleEntry,
    __inout PSYSTEM_HANDLE_INFORMATION HandleInformation,
    __in ULONG Length,
    __inout PULONG RequiredLength
    )

/*++

Routine Description:

    This function visits and invokes the specified callback for every valid
    handle that it can find off of the handle table.

Arguments:

    SnapShotHandleEntry - Supplies a pointer to a function to call for
        each valid handle we encounter.

    HandleInformation - Supplies a handle information structure to
        be filled in for each handle table we encounter.  This routine
        fills in the handle count, but relies on a callback to fill in
        entry info fields.

    Length - Supplies a parameter for the callback.  In reality this is
        the total size, in bytes, of the Handle Information buffer.

    RequiredLength - Supplies a parameter for the callback.  In reality
        this is a final size in bytes used to store the requested
        information.

Return Value:

    The last return status of the callback

--*/

{
    NTSTATUS Status;
    PKTHREAD CurrentThread;
    PSYSTEM_HANDLE_TABLE_ENTRY_INFO HandleEntryInfo;
    PLIST_ENTRY NextEntry;
    PHANDLE_TABLE HandleTable;
    EXHANDLE Handle;
    PHANDLE_TABLE_ENTRY HandleTableEntry;

    PAGED_CODE();

    CurrentThread = KeGetCurrentThread ();


    Status = STATUS_SUCCESS;

    //
    //  Setup the output buffer pointer that the callback will maintain
    //

    HandleEntryInfo = &HandleInformation->Handles[0];

    //
    //  Zero out the handle count
    //

    HandleInformation->NumberOfHandles = 0;

    //
    //  Lock the handle table list exclusive and traverse the list of handle
    //  tables.
    //

    KeEnterCriticalRegionThread (CurrentThread);
    ExAcquirePushLockShared( &HandleTableListLock );

    //
    //  Iterate through all the handle tables in the system.
    //

    for (NextEntry = HandleTableListHead.Flink;
         NextEntry != &HandleTableListHead;
         NextEntry = NextEntry->Flink) {

        //
        //  Get the address of the next handle table, lock the handle
        //  table exclusive, and scan the list of handle entries.
        //

        HandleTable = CONTAINING_RECORD( NextEntry,
                                         HANDLE_TABLE,
                                         HandleTableList );


        //  Iterate through the handle table and for each handle that
        //  is allocated we'll invoke the call back.  Note that this
        //  loop exits when we get a null handle table entry.  We know
        //  there will be no more possible entries after the first null
        //  one is encountered because we allocate memory of the
        //  handles in a dense fashion
        //

        for (Handle.Value = 0;
             (HandleTableEntry = ExpLookupHandleTableEntry( HandleTable, Handle )) != NULL;
             Handle.Value += HANDLE_VALUE_INC) {

            //
            //  Only do the callback if the entry is not free
            //

            if ( ExpIsValidObjectEntry(HandleTableEntry) ) {

                //
                //  Increment the handle count information in the
                //  information buffer
                //

                HandleInformation->NumberOfHandles += 1;

                //
                //  Lock the handle table entry because we're about to
                //  give it to the callback function, then release the
                //  entry right after the call back.
                //

                if (ExpLockHandleTableEntry( HandleTable, HandleTableEntry )) {

                    Status = (*SnapShotHandleEntry)( &HandleEntryInfo,
                                                     HandleTable->UniqueProcessId,
                                                     HandleTableEntry,
                                                     Handle.GenericHandleOverlay,
                                                     Length,
                                                     RequiredLength );

                    ExUnlockHandleTableEntry( HandleTable, HandleTableEntry );
                }
            }
        }
    }

    ExReleasePushLockShared( &HandleTableListLock );
    KeLeaveCriticalRegionThread (CurrentThread);

    return Status;
}


NTKERNELAPI
NTSTATUS
ExSnapShotHandleTablesEx (
    __in PEX_SNAPSHOT_HANDLE_ENTRY_EX SnapShotHandleEntry,
    __inout PSYSTEM_HANDLE_INFORMATION_EX HandleInformation,
    __in ULONG Length,
    __inout PULONG RequiredLength
    )

/*++

Routine Description:

    This function visits and invokes the specified callback for every valid
    handle that it can find off of the handle table.

Arguments:

    SnapShotHandleEntry - Supplies a pointer to a function to call for
        each valid handle we encounter.

    HandleInformation - Supplies a handle information structure to
        be filled in for each handle table we encounter.  This routine
        fills in the handle count, but relies on a callback to fill in
        entry info fields.

    Length - Supplies a parameter for the callback.  In reality this is
        the total size, in bytes, of the Handle Information buffer.

    RequiredLength - Supplies a parameter for the callback.  In reality
        this is a final size in bytes used to store the requested
        information.

Return Value:

    The last return status of the callback

--*/

{
    NTSTATUS Status;
    PKTHREAD CurrentThread;
    PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX HandleEntryInfo;
    PLIST_ENTRY NextEntry;
    PHANDLE_TABLE HandleTable;
    EXHANDLE Handle;
    PHANDLE_TABLE_ENTRY HandleTableEntry;

    PAGED_CODE();

    CurrentThread = KeGetCurrentThread ();

    Status = STATUS_SUCCESS;


    //
    //  Setup the output buffer pointer that the callback will maintain
    //

    HandleEntryInfo = &HandleInformation->Handles[0];

    //
    //  Zero out the handle count
    //

    HandleInformation->NumberOfHandles = 0;

    //
    //  Lock the handle table list exclusive and traverse the list of handle
    //  tables.
    //

    KeEnterCriticalRegionThread (CurrentThread);
    ExAcquirePushLockShared( &HandleTableListLock );

    //
    //  Iterate through all the handle tables in the system.
    //

    for (NextEntry = HandleTableListHead.Flink;
         NextEntry != &HandleTableListHead;
         NextEntry = NextEntry->Flink) {

        //
        //  Get the address of the next handle table, lock the handle
        //  table exclusive, and scan the list of handle entries.
        //

        HandleTable = CONTAINING_RECORD( NextEntry,
                                         HANDLE_TABLE,
                                         HandleTableList );


        //  Iterate through the handle table and for each handle that
        //  is allocated we'll invoke the call back.  Note that this
        //  loop exits when we get a null handle table entry.  We know
        //  there will be no more possible entries after the first null
        //  one is encountered because we allocate memory of the
        //  handles in a dense fashion
        //

        for (Handle.Value = 0;
             (HandleTableEntry = ExpLookupHandleTableEntry( HandleTable, Handle )) != NULL;
             Handle.Value += HANDLE_VALUE_INC) {

            //
            //  Only do the callback if the entry is not free
            //

            if ( ExpIsValidObjectEntry(HandleTableEntry) ) {

                //
                //  Increment the handle count information in the
                //  information buffer
                //

                HandleInformation->NumberOfHandles += 1;

                //
                //  Lock the handle table entry because we're about to
                //  give it to the callback function, then release the
                //  entry right after the call back.
                //

                if (ExpLockHandleTableEntry( HandleTable, HandleTableEntry )) {

                    Status = (*SnapShotHandleEntry)( &HandleEntryInfo,
                                                     HandleTable->UniqueProcessId,
                                                     HandleTableEntry,
                                                     Handle.GenericHandleOverlay,
                                                     Length,
                                                     RequiredLength );

                    ExUnlockHandleTableEntry( HandleTable, HandleTableEntry );
                }
            }
        }
    }

    ExReleasePushLockShared( &HandleTableListLock );
    KeLeaveCriticalRegionThread (CurrentThread);

    return Status;
}


NTKERNELAPI
HANDLE
ExCreateHandle (
    __inout PHANDLE_TABLE HandleTable,
    __in PHANDLE_TABLE_ENTRY HandleTableEntry
    )

/*++

Routine Description:

    This function creates a handle entry in the specified handle table and
    returns a handle for the entry.

Arguments:

    HandleTable - Supplies a pointer to a handle table

    HandleEntry - Supplies a pointer to the handle entry for which a
        handle entry is created.

Return Value:

    If the handle entry is successfully created, then value of the created
    handle is returned as the function value.  Otherwise, a value of zero is
    returned.

--*/

{
    EXHANDLE Handle;
    PETHREAD CurrentThread;
    PHANDLE_TABLE_ENTRY NewHandleTableEntry;

    PAGED_CODE();

    //
    //  Set out output variable to zero (i.e., null) before going on
    //

    //
    // Clears Handle.Index and Handle.TagBits
    //

    Handle.GenericHandleOverlay = NULL;


    //
    //  Allocate a new handle table entry, and get the handle value
    //

    NewHandleTableEntry = ExpAllocateHandleTableEntry( HandleTable,
                                                       &Handle );

    //
    //  If we really got a handle then copy over the template and unlock
    //  the entry
    //

    if (NewHandleTableEntry != NULL) {

        CurrentThread = PsGetCurrentThread ();

        //
        // We are about to create a locked entry so protect against suspension
        //
        KeEnterCriticalRegionThread (&CurrentThread->Tcb);

        *NewHandleTableEntry = *HandleTableEntry;

        //
        // If we are debugging handle operations then save away the details
        //
        if (HandleTable->DebugInfo != NULL) {
            ExpUpdateDebugInfo(HandleTable, CurrentThread, Handle.GenericHandleOverlay, HANDLE_TRACE_DB_OPEN);
        }

        ExUnlockHandleTableEntry( HandleTable, NewHandleTableEntry );

        KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
    }

    return Handle.GenericHandleOverlay;
}


NTKERNELAPI
BOOLEAN
ExDestroyHandle (
    __inout PHANDLE_TABLE HandleTable,
    __in HANDLE Handle,
    __inout_opt PHANDLE_TABLE_ENTRY HandleTableEntry
    )

/*++

Routine Description:

    This function removes a handle from a handle table.

Arguments:

    HandleTable - Supplies a pointer to a handle table

    Handle - Supplies the handle value of the entry to remove.

    HandleTableEntry - Optionally supplies a pointer to the handle
        table entry being destroyed.  If supplied the entry is
        assume to be locked.

Return Value:

    If the specified handle is successfully removed, then a value of
    TRUE is returned. Otherwise, a value of FALSE is returned.

--*/

{
    EXHANDLE LocalHandle;
    PETHREAD CurrentThread;
    PVOID Object;

    PAGED_CODE();

    LocalHandle.GenericHandleOverlay = Handle;

    CurrentThread = PsGetCurrentThread ();

    //
    //  If the caller did not supply the optional handle table entry then
    //  locate the entry via the supplied handle, make sure it is real, and
    //  then lock the entry.
    //

    KeEnterCriticalRegionThread (&CurrentThread->Tcb);

    if (HandleTableEntry == NULL) {

        HandleTableEntry = ExpLookupHandleTableEntry( HandleTable,
                                                      LocalHandle );

        if (!ExpIsValidObjectEntry(HandleTableEntry)) {

            KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
            return FALSE;
        }


        if (!ExpLockHandleTableEntry( HandleTable, HandleTableEntry )) {

            KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
            return FALSE;
        }
    } else {
        EXASSERT ((HandleTableEntry->Value&EXHANDLE_TABLE_ENTRY_LOCK_BIT) == 0);
    }


    //
    // If we are debugging handle operations then save away the details
    //

    if (HandleTable->DebugInfo != NULL) {
        ExpUpdateDebugInfo(HandleTable, CurrentThread, Handle, HANDLE_TRACE_DB_CLOSE);
    }

    //
    //  At this point we have a locked handle table entry.  Now mark it free
    //  which does the implicit unlock.  The system will not allocate it
    //  again until we add it to the free list which we will do right after
    //  we take out the lock
    //

    Object = InterlockedExchangePointer (&HandleTableEntry->Object, NULL);

    EXASSERT (Object != NULL);
    EXASSERT ((((ULONG_PTR)Object)&EXHANDLE_TABLE_ENTRY_LOCK_BIT) == 0);

    //
    // Unblock any waiters waiting for this table entry.
    //
    ExUnblockPushLock (&HandleTable->HandleContentionEvent, NULL);


    ExpFreeHandleTableEntry( HandleTable,
                             LocalHandle,
                             HandleTableEntry );

    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

    return TRUE;
}


NTKERNELAPI
BOOLEAN
ExChangeHandle (
    __in PHANDLE_TABLE HandleTable,
    __in HANDLE Handle,
    __in PEX_CHANGE_HANDLE_ROUTINE ChangeRoutine,
    __in ULONG_PTR Parameter
    )

/*++

Routine Description:

    This function provides the capability to change the contents of the
    handle entry corrsponding to the specified handle.

Arguments:

    HandleTable - Supplies a pointer to a handle table.

    Handle - Supplies the handle for the handle entry that is changed.

    ChangeRoutine - Supplies a pointer to a function that is called to
        perform the change.

    Parameter - Supplies an uninterpreted parameter that is passed to
        the change routine.

Return Value:

    If the operation was successfully performed, then a value of TRUE
    is returned. Otherwise, a value of FALSE is returned.

--*/

{
    EXHANDLE LocalHandle;
    PKTHREAD CurrentThread;

    PHANDLE_TABLE_ENTRY HandleTableEntry;
    BOOLEAN ReturnValue;

    PAGED_CODE();

    LocalHandle.GenericHandleOverlay = Handle;

    CurrentThread = KeGetCurrentThread ();

    //
    //  Translate the input handle to a handle table entry and make
    //  sure it is a valid handle.
    //

    HandleTableEntry = ExpLookupHandleTableEntry( HandleTable,
                                                  LocalHandle );

    if ((HandleTableEntry == NULL) ||
        !ExpIsValidObjectEntry(HandleTableEntry)) {

        return FALSE;
    }



    //
    //  Try and lock the handle table entry,  If this fails then that's
    //  because someone freed the handle
    //

    //
    //  Make sure we can't get suspended and then invoke the callback
    //

    KeEnterCriticalRegionThread (CurrentThread);

    if (ExpLockHandleTableEntry( HandleTable, HandleTableEntry )) {


        ReturnValue = (*ChangeRoutine)( HandleTableEntry, Parameter );
        
        ExUnlockHandleTableEntry( HandleTable, HandleTableEntry );

    } else {
        ReturnValue = FALSE;
    }

    KeLeaveCriticalRegionThread (CurrentThread);

    return ReturnValue;
}


NTKERNELAPI
PHANDLE_TABLE_ENTRY
ExMapHandleToPointer (
    __in PHANDLE_TABLE HandleTable,
    __in HANDLE Handle
    )

/*++

Routine Description:

    This function maps a handle to a pointer to a handle table entry. If the
    map operation is successful then the handle table entry is locked when
    we return.

Arguments:

    HandleTable - Supplies a pointer to a handle table.

    Handle - Supplies the handle to be mapped to a handle entry.

Return Value:

    If the handle was successfully mapped to a pointer to a handle entry,
    then the address of the handle table entry is returned as the function
    value with the entry locked. Otherwise, a value of NULL is returned.

--*/

{
    EXHANDLE LocalHandle;
    PHANDLE_TABLE_ENTRY HandleTableEntry;

    PAGED_CODE();

    LocalHandle.GenericHandleOverlay = Handle;

    if ((LocalHandle.Index & (LOWLEVEL_COUNT - 1)) == 0) {
        return NULL;
    }

    //
    //  Translate the input handle to a handle table entry and make
    //  sure it is a valid handle.
    //

    HandleTableEntry = ExpLookupHandleTableEntry( HandleTable,
                                                  LocalHandle );

    if ((HandleTableEntry == NULL) ||
        !ExpLockHandleTableEntry( HandleTable, HandleTableEntry)) {
        //
        // If we are debugging handle operations then save away the details
        //

        if (HandleTable->DebugInfo != NULL) {
            ExpUpdateDebugInfo(HandleTable, PsGetCurrentThread (), Handle, HANDLE_TRACE_DB_BADREF);
        }
        return NULL;
    }


    //
    //  Return the locked valid handle table entry
    //

    return HandleTableEntry;
}

NTKERNELAPI
PHANDLE_TABLE_ENTRY
ExMapHandleToPointerEx (
    __in PHANDLE_TABLE HandleTable,
    __in HANDLE Handle,
    __in KPROCESSOR_MODE PreviousMode
    )

/*++

Routine Description:

    This function maps a handle to a pointer to a handle table entry. If the
    map operation is successful then the handle table entry is locked when
    we return.

Arguments:

    HandleTable - Supplies a pointer to a handle table.

    Handle - Supplies the handle to be mapped to a handle entry.

    PreviousMode - Previous mode of caller

Return Value:

    If the handle was successfully mapped to a pointer to a handle entry,
    then the address of the handle table entry is returned as the function
    value with the entry locked. Otherwise, a value of NULL is returned.

--*/

{
    EXHANDLE LocalHandle;
    PHANDLE_TABLE_ENTRY HandleTableEntry = NULL;
    PETHREAD CurrentThread;

    PAGED_CODE();

    LocalHandle.GenericHandleOverlay = Handle;

    //
    //  Translate the input handle to a handle table entry and make
    //  sure it is a valid handle.
    //    

    if (((LocalHandle.Index & (LOWLEVEL_COUNT - 1)) == 0) ||
        ((HandleTableEntry = ExpLookupHandleTableEntry(HandleTable, LocalHandle)) == NULL) ||
        !ExpLockHandleTableEntry( HandleTable, HandleTableEntry)) {

        //
        // If we are debugging handle operations then save away the details
        //

        if (HandleTable->DebugInfo != NULL) {
            CurrentThread = PsGetCurrentThread ();
            ExpUpdateDebugInfo(HandleTable, CurrentThread, Handle, HANDLE_TRACE_DB_BADREF);

            //
            // Since we have a non-null DebugInfo for the handle table of this
            // process it means application verifier was enabled for this process.
            //

            if (PreviousMode == UserMode) {

                if (!KeIsAttachedProcess()) {

                    //
                    // If the current process is marked for verification
                    // then we will raise an exception in user mode. In case
                    // application verifier is enabled system wide we will 
                    // break first.
                    //
                                                 
                    if ((NtGlobalFlag & FLG_APPLICATION_VERIFIER)) {
                        
                        DbgPrint ("AVRF: Invalid handle %p in process %p \n", 
                                  Handle,
                                  PsGetCurrentProcess());

//                        DbgBreakPoint ();
                    }

                    KeRaiseUserException (STATUS_INVALID_HANDLE);
                }
            } else {

                //
                // We bugcheck for kernel handles only if we have the handle
                // exceptions flag set system-wide. This way a user enabling
                // application verifier for a process will not get bugchecks
                // only user mode errors.
                //

                if ((NtGlobalFlag & FLG_ENABLE_HANDLE_EXCEPTIONS)) {

                    KeBugCheckEx(INVALID_KERNEL_HANDLE,
                                 (ULONG_PTR)Handle,
                                 (ULONG_PTR)HandleTable,
                                 (ULONG_PTR)HandleTableEntry,
                                 0x1);
                }
            }
        }
        
        return NULL;
    }


    //
    //  Return the locked valid handle table entry
    //

    return HandleTableEntry;
}

//
//  Local Support Routine
//

PVOID
ExpAllocateTablePagedPool (
    IN PEPROCESS QuotaProcess OPTIONAL,
    IN SIZE_T NumberOfBytes
    )
{
    PVOID PoolMemory;

    PoolMemory = ExAllocatePoolWithTag( PagedPool,
                                        NumberOfBytes,
                                        'btbO' );
    if (PoolMemory != NULL) {

        RtlZeroMemory( PoolMemory,
                       NumberOfBytes );

        if (ARGUMENT_PRESENT(QuotaProcess)) {

            if (!NT_SUCCESS (PsChargeProcessPagedPoolQuota ( QuotaProcess,
                                                             NumberOfBytes ))) {
                ExFreePool( PoolMemory );
                PoolMemory = NULL;
            }

        }
    }

    return PoolMemory;
}

PVOID
ExpAllocateTablePagedPoolNoZero (
    IN PEPROCESS QuotaProcess OPTIONAL,
    IN SIZE_T NumberOfBytes
    )
{
    PVOID PoolMemory;

    PoolMemory = ExAllocatePoolWithTag( PagedPool,
                                        NumberOfBytes,
                                        'btbO' );
    if (PoolMemory != NULL) {

        if (ARGUMENT_PRESENT(QuotaProcess)) {

            if (!NT_SUCCESS (PsChargeProcessPagedPoolQuota ( QuotaProcess,
                                                             NumberOfBytes ))) {
                ExFreePool( PoolMemory );
                PoolMemory = NULL;
            }

        }
    }

    return PoolMemory;
}


//
//  Local Support Routine
//

VOID
ExpFreeTablePagedPool (
    IN PEPROCESS QuotaProcess OPTIONAL,
    IN PVOID PoolMemory,
    IN SIZE_T NumberOfBytes
    )
{

    ExFreePool( PoolMemory );

    if ( QuotaProcess ) {

        PsReturnProcessPagedPoolQuota( QuotaProcess,
                                       NumberOfBytes
                                     );
    }
}



PHANDLE_TRACE_DEBUG_INFO
ExReferenceHandleDebugInfo (
    IN PHANDLE_TABLE HandleTable
    )
{
    LONG RetVal;
    PHANDLE_TRACE_DEBUG_INFO DebugInfo;
    PKTHREAD CurrentThread;

    CurrentThread = KeGetCurrentThread ();

    ExpLockHandleTableShared (HandleTable, CurrentThread, 0);

    DebugInfo = HandleTable->DebugInfo;

    if (DebugInfo != NULL) {
        RetVal = InterlockedIncrement (&DebugInfo->RefCount);
        ASSERT (RetVal > 0);
    }

    ExpUnlockHandleTableShared (HandleTable, CurrentThread, 0);

    return DebugInfo;
}


VOID
ExDereferenceHandleDebugInfo (
    IN PHANDLE_TABLE HandleTable,
    IN PHANDLE_TRACE_DEBUG_INFO DebugInfo
    )
{
    ULONG TraceSize;
    ULONG TableSize;
    LONG RetVal;

    RetVal = InterlockedDecrement (&DebugInfo->RefCount);

    ASSERT (RetVal >= 0);

    if (RetVal == 0) {

        TableSize = DebugInfo->TableSize;
        TraceSize = sizeof (*DebugInfo) + TableSize * sizeof (DebugInfo->TraceDb[0]) - sizeof (DebugInfo->TraceDb);

        ExFreePool (DebugInfo);

        if (HandleTable->QuotaProcess != NULL) {
            PsReturnProcessNonPagedPoolQuota (HandleTable->QuotaProcess,
                                              TraceSize);
        }

        InterlockedExchangeAdd ((PLONG) &TotalTraceBuffers, -(LONG)TableSize);
    }
}

NTKERNELAPI
NTSTATUS
ExDisableHandleTracing (
    __inout PHANDLE_TABLE HandleTable
    )
/*++

Routine Description:

    This routine turns off handle tracing for the specified table

Arguments:

    HandleTable - Table to disable tracing in


Return Value:

    NTSTATUS - Status of operation

--*/
{
    PHANDLE_TRACE_DEBUG_INFO DebugInfo;
    PKTHREAD CurrentThread;

    CurrentThread = KeGetCurrentThread ();

    ExpLockHandleTableExclusive (HandleTable, CurrentThread);

    DebugInfo = HandleTable->DebugInfo;

    HandleTable->DebugInfo = NULL;

    ExpUnlockHandleTableExclusive (HandleTable, CurrentThread);

    if (DebugInfo != NULL) {
        ExDereferenceHandleDebugInfo (HandleTable, DebugInfo);
    }

    return STATUS_SUCCESS;
}

C_ASSERT (RTL_FIELD_SIZE (HANDLE_TRACE_DEBUG_INFO, TraceDb[0]) <= MAXULONG);

NTKERNELAPI
NTSTATUS
ExEnableHandleTracing (
    __inout PHANDLE_TABLE HandleTable,
    __in ULONG Slots
    )
/*++

Routine Description:

    This routine turns on handle tracing for the specified table

Arguments:

    HandleTable - Table to enable tracing in


Return Value:

    NTSTATUS - Status of operation

--*/

{
    PHANDLE_TRACE_DEBUG_INFO DebugInfo, OldDebugInfo;
    PEPROCESS Process;
    PKTHREAD CurrentThread;
    NTSTATUS Status;
    SIZE_T TotalNow;
    extern SIZE_T MmMaximumNonPagedPoolInBytes;
    SIZE_T TraceSize;
    LONG TotalSlots;

    if (Slots == 0) {
        TotalSlots = HANDLE_TRACE_DB_DEFAULT_STACKS;
    } else {
        if (Slots < HANDLE_TRACE_DB_MIN_STACKS) {
            TotalSlots = HANDLE_TRACE_DB_MIN_STACKS;
        } else {
            TotalSlots = Slots;
        }

        if (TotalSlots < 0 || TotalSlots > HANDLE_TRACE_DB_MAX_STACKS) {
            TotalSlots = HANDLE_TRACE_DB_MAX_STACKS;
        }

        //
        // Round the value up to the next power of 2
        //

        while ((TotalSlots & (TotalSlots - 1)) != 0) {
            TotalSlots |= (TotalSlots - 1);
            TotalSlots += 1;
        }
    }

    //
    // Total slots needs to be a power of two
    //
    ASSERT ((TotalSlots & (TotalSlots - 1)) == 0);
    ASSERT (TotalSlots > 0 && TotalSlots <= HANDLE_TRACE_DB_MAX_STACKS);

    TraceSize = sizeof (*DebugInfo) + TotalSlots * sizeof (DebugInfo->TraceDb[0]) - sizeof (DebugInfo->TraceDb);

    TotalNow = InterlockedExchangeAdd ((PLONG) &TotalTraceBuffers, TotalSlots);

    //
    // See if more than 5/16 (31.25%) of non paged pool has been used.
    // Note that performing math in this manner offers improved efficiency 
    // and avoids overflow.
    //

    if (((ULONGLONG) TotalNow) * sizeof (DebugInfo->TraceDb[0]) > 
        ((MmMaximumNonPagedPoolInBytes >> 2) + (MmMaximumNonPagedPoolInBytes >> 4))) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto return_and_exit;
    }

    Process = HandleTable->QuotaProcess;

    if (Process) {
        Status = PsChargeProcessNonPagedPoolQuota (Process,
                                                   TraceSize);
        if (!NT_SUCCESS (Status)) {
            goto return_and_exit;
        }
    }

    //
    // Allocate the handle debug database
    //
    DebugInfo = ExAllocatePoolWithTag (NonPagedPool,
                                       TraceSize,
                                       'dtbO');
    if (DebugInfo == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto quota_return_and_exit;
    }
    RtlZeroMemory (DebugInfo, TraceSize);

    DebugInfo->RefCount = 1;
    DebugInfo->TableSize = TotalSlots;

    ExInitializeFastMutex(&DebugInfo->CloseCompactionLock);

    //
    // Since we are tracing then we should enforce strict FIFO
    // Only do this for tables with processes so we leave atom tables alone.
    //
    if (Process != NULL) {
        HandleTable->StrictFIFO = TRUE;
    }

    //
    // Put the new table in place releasing any existing table
    //

    CurrentThread = KeGetCurrentThread ();

    ExpLockHandleTableExclusive (HandleTable, CurrentThread);

    OldDebugInfo = HandleTable->DebugInfo;

    HandleTable->DebugInfo = DebugInfo;

    ExpUnlockHandleTableExclusive (HandleTable, CurrentThread);

    if (OldDebugInfo != NULL) {
        ExDereferenceHandleDebugInfo (HandleTable, OldDebugInfo);
    }

    return STATUS_SUCCESS;

quota_return_and_exit:

    if (Process) {
        PsReturnProcessNonPagedPoolQuota (Process,
                                          TraceSize);
    }


return_and_exit:

    InterlockedExchangeAdd ((PLONG) &TotalTraceBuffers, -TotalSlots);
    return Status;
}

//
//  Local Support Routine
//

PHANDLE_TABLE
ExpAllocateHandleTable (
    IN PEPROCESS Process OPTIONAL,
    IN BOOLEAN DoInit
    )

/*++

Routine Description:

    This worker routine will allocate and initialize a new handle table
    structure.  The new structure consists of the basic handle table
    struct plus the first allocation needed to store handles.  This is
    really one page divided up into the top level node, the first mid
    level node, and one bottom level node.

Arguments:

    Process - Optionally supplies the process to charge quota for the
        handle table

    DoInit - If FALSE then we are being called by duplicate and we don't need
             the free list built for the caller

Return Value:

    A pointer to the new handle table or NULL if unsuccessful at getting
    pool.

--*/

{
    PHANDLE_TABLE HandleTable;
    PHANDLE_TABLE_ENTRY HandleTableTable, HandleEntry;
    ULONG i, Idx;

    PAGED_CODE();

    //
    //  If any allocation or quota failures happen we will catch it in the
    //  following try-except clause and cleanup after outselves before
    //  we return null
    //

    //
    //  First allocate the handle table, make sure we got one, charge quota
    //  for it and then zero it out
    //

    HandleTable = (PHANDLE_TABLE)ExAllocatePoolWithTag (PagedPool,
                                                        sizeof(HANDLE_TABLE),
                                                        'btbO');
    if (HandleTable == NULL) {
        return NULL;
    }

    if (ARGUMENT_PRESENT(Process)) {

        if (!NT_SUCCESS (PsChargeProcessPagedPoolQuota( Process,
                                                        sizeof(HANDLE_TABLE)))) {
            ExFreePool( HandleTable );
            return NULL;
        }
    }


    RtlZeroMemory( HandleTable, sizeof(HANDLE_TABLE) );


    //
    //  Now allocate space of the top level, one mid level and one bottom
    //  level table structure.  This will all fit on a page, maybe two.
    //

    HandleTableTable = ExpAllocateTablePagedPoolNoZero ( Process,
                                                         TABLE_PAGE_SIZE
                                                        );

    if ( HandleTableTable == NULL ) {

        ExFreePool( HandleTable );

        if (ARGUMENT_PRESENT(Process)) {

            PsReturnProcessPagedPoolQuota (Process,
                                           sizeof(HANDLE_TABLE));
        }
            
        return NULL;
    }
        
    HandleTable->TableCode = (ULONG_PTR)HandleTableTable;


    //
    //  We stamp with EX_ADDITIONAL_INFO_SIGNATURE to recognize in the future this
    //  is a special information entry
    //

    HandleEntry = &HandleTableTable[0];

    HandleEntry->NextFreeTableEntry = EX_ADDITIONAL_INFO_SIGNATURE;
    HandleEntry->Value = 0;

    //
    // For duplicate calls we skip building the free list as we rebuild it manually as
    // we traverse the old table we are duplicating
    //
    if (DoInit) {
        HandleEntry++;
        //
        //  Now setup the free list.  We do this by chaining together the free
        //  entries such that each free entry give the next free index (i.e.,
        //  like a fat chain).  The chain is terminated with a 0.  Note that
        //  we'll skip handle zero because our callers will get that value
        //  confused with null.
        //


        for (i = 1; i < LOWLEVEL_COUNT - 1; i += 1) {

            HandleEntry->Value = 0;
            HandleEntry->NextFreeTableEntry = (i+1)*HANDLE_VALUE_INC;
            HandleEntry++;
        }
        HandleEntry->Value = 0;
        HandleEntry->NextFreeTableEntry = 0;

        HandleTable->FirstFree = HANDLE_VALUE_INC;

    }
    

    HandleTable->NextHandleNeedingPool = LOWLEVEL_COUNT * HANDLE_VALUE_INC;

    //
    //  Setup the necessary process information
    //

    HandleTable->QuotaProcess = Process;
    HandleTable->UniqueProcessId = PsGetCurrentProcess()->UniqueProcessId;
    HandleTable->Flags = 0;

#if DBG && !EXHANDLE_EXTRA_CHECKS
    if (Process != NULL) {
        HandleTable->StrictFIFO = TRUE;
    }
#endif

    //
    //  Initialize the handle table lock. This is only used by table expansion.
    //

    for (Idx = 0; Idx < HANDLE_TABLE_LOCKS; Idx++) {
        ExInitializePushLock (&HandleTable->HandleTableLock[Idx]);
    }

    //
    //  Initialize the blocker for handle entry lock contention.
    //

    ExInitializePushLock (&HandleTable->HandleContentionEvent);

    if (TRACE_ALL_TABLES) {
        ExEnableHandleTracing (HandleTable, 0);    
    }

    //
    //  And return to our caller
    //

    return HandleTable;
}

//
//  Local Support Routine
//

VOID
ExpFreeLowLevelTable (
    IN PEPROCESS QuotaProcess,
    IN PHANDLE_TABLE_ENTRY TableLevel1
    )

/*++

Routine Description:

    This worker routine frees a low-level handle table
    and the additional info memory, if any.

Arguments:

    HandleTable - Supplies the handle table being freed

Return Value:

    None.

--*/

{
    //
    //  Check whether we have a pool allocated for the additional info
    //

    if (TableLevel1[0].Object) {

        ExpFreeTablePagedPool( QuotaProcess,
                               TableLevel1[0].Object,
                               LOWLEVEL_COUNT * sizeof(HANDLE_TABLE_ENTRY_INFO)
                             );
    }

    //
    //  Now free the low level table and return the quota for the process
    //

    ExpFreeTablePagedPool( QuotaProcess,
                           TableLevel1,
                           TABLE_PAGE_SIZE
                         );
    
    //
    //  And return to our caller
    //

    return;
}


//
//  Local Support Routine
//

VOID
ExpFreeHandleTable (
    IN PHANDLE_TABLE HandleTable
    )

/*++

Routine Description:

    This worker routine tears down and frees the specified handle table.

Arguments:

    HandleTable - Supplies the handle table being freed

Return Value:

    None.

--*/

{
    PEPROCESS Process;
    ULONG i,j;
    ULONG_PTR CapturedTable = HandleTable->TableCode;
    ULONG TableLevel = (ULONG)(CapturedTable & LEVEL_CODE_MASK);

    PAGED_CODE();

    //
    //  Unmask the level bits
    //

    CapturedTable = CapturedTable & ~LEVEL_CODE_MASK;
    Process = HandleTable->QuotaProcess;

    //
    //  We need to free all pages. We have 3 cases, depending on the number
    //  of levels
    //


    if (TableLevel == 0) {

        //
        //  There is a single level handle table. We'll simply free the buffer
        //

        PHANDLE_TABLE_ENTRY TableLevel1 = (PHANDLE_TABLE_ENTRY)CapturedTable;
        
        ExpFreeLowLevelTable( Process, TableLevel1 );

    } else if (TableLevel == 1) {

        //
        //  We have 2 levels in the handle table
        //
        
        PHANDLE_TABLE_ENTRY *TableLevel2 = (PHANDLE_TABLE_ENTRY *)CapturedTable;

        for (i = 0; i < MIDLEVEL_COUNT; i++) {

            //
            //  loop through the pointers to the low-level tables, and free each one
            //

            if (TableLevel2[i] == NULL) {

                break;
            }
            
            ExpFreeLowLevelTable( Process, TableLevel2[i] );
        }
        
        //
        //  Free the top level table
        //

        ExpFreeTablePagedPool( Process,
                               TableLevel2,
                               PAGE_SIZE
                             );

    } else {

        //
        //  Here we handle the case where we have a 3 level handle table
        //

        PHANDLE_TABLE_ENTRY **TableLevel3 = (PHANDLE_TABLE_ENTRY **)CapturedTable;

        //
        //  Iterates through the high-level pointers to mid-table
        //

        for (i = 0; i < HIGHLEVEL_COUNT; i++) {

            if (TableLevel3[i] == NULL) {

                break;
            }
            
            //
            //  Iterate through the mid-level table
            //  and free every low-level page
            //

            for (j = 0; j < MIDLEVEL_COUNT; j++) {

                if (TableLevel3[i][j] == NULL) {

                    break;
                }
                
                ExpFreeLowLevelTable( Process, TableLevel3[i][j] );
            }

            ExpFreeTablePagedPool( Process,
                                   TableLevel3[i],
                                   PAGE_SIZE
                                 );
        }
        
        //
        //  Free the top-level array
        //

        ExpFreeTablePagedPool( Process,
                               TableLevel3,
                               HIGHLEVEL_SIZE
                             );
    }

    //
    // Free any debug info if we have any.
    //

    if (HandleTable->DebugInfo != NULL) {
        ExDereferenceHandleDebugInfo (HandleTable, HandleTable->DebugInfo);
    }

    //
    //  Finally deallocate the handle table itself
    //

    ExFreePool( HandleTable );

    if (Process != NULL) {

        PsReturnProcessPagedPoolQuota (Process,
                                       sizeof(HANDLE_TABLE));
    }

    //
    //  And return to our caller
    //

    return;
}


//
//  Local Support Routine
//

PHANDLE_TABLE_ENTRY
ExpAllocateLowLevelTable (
    IN PHANDLE_TABLE HandleTable,
    IN BOOLEAN DoInit
    )

/*++

Routine Description:

    This worker routine allocates a new low level table
    
    Note: The caller must have already locked the handle table

Arguments:

    HandleTable - Supplies the handle table being used

    DoInit - If FALSE the caller (duplicate) doesn't need the free list maintained

Return Value:

    Returns - a pointer to a low-level table if allocation is
        successful otherwise the return value is null.

--*/

{
    ULONG k;
    PHANDLE_TABLE_ENTRY NewLowLevel = NULL, HandleEntry;
    ULONG BaseHandle;
    
    //
    //  Allocate the pool for lower level
    //

    NewLowLevel = ExpAllocateTablePagedPoolNoZero( HandleTable->QuotaProcess,
                                                   TABLE_PAGE_SIZE
                                                 );

    if (NewLowLevel == NULL) {

        return NULL;
    }

    //
    //  We stamp with EX_ADDITIONAL_INFO_SIGNATURE to recognize in the future this
    //  is a special information entry
    //

    HandleEntry = &NewLowLevel[0];

    HandleEntry->NextFreeTableEntry = EX_ADDITIONAL_INFO_SIGNATURE;
    HandleEntry->Value = 0;

    //
    // Initialize the free list within this page if the caller wants this
    //
    if (DoInit) {

        HandleEntry++;

        //
        //  Now add the new entries to the free list.  To do this we
        //  chain the new free entries together.  We are guaranteed to
        //  have at least one new buffer.  The second buffer we need
        //  to check for.
        //
        //  We reserve the first entry in the table to the structure with
        //  additional info
        //
        //
        //  Do the guaranteed first buffer
        //

        BaseHandle = HandleTable->NextHandleNeedingPool + 2 * HANDLE_VALUE_INC;
        for (k = BaseHandle; k < BaseHandle + (LOWLEVEL_COUNT - 2) * HANDLE_VALUE_INC; k += HANDLE_VALUE_INC) {

            HandleEntry->NextFreeTableEntry = k;
            HandleEntry->Value = 0;
            HandleEntry++;
        }
        HandleEntry->NextFreeTableEntry = 0;
        HandleEntry->Value = 0;
    }


    return NewLowLevel;    
}

PHANDLE_TABLE_ENTRY *
ExpAllocateMidLevelTable (
    IN PHANDLE_TABLE HandleTable,
    IN BOOLEAN DoInit,
    OUT PHANDLE_TABLE_ENTRY *pNewLowLevel
    )

/*++

Routine Description:

    This worker routine allocates a mid-level table. This is an array with
    pointers to low-level tables.
    It will allocate also a low-level table and will save it in the first index
    
    Note: The caller must have already locked the handle table

Arguments:

    HandleTable - Supplies the handle table being used

    DoInit - If FALSE the caller (duplicate) does not want the free list build

    pNewLowLevel - Returns the new low level table for later free list chaining

Return Value:

    Returns a pointer to the new mid-level table allocated
    
--*/

{
    PHANDLE_TABLE_ENTRY *NewMidLevel;
    PHANDLE_TABLE_ENTRY NewLowLevel;
    
    NewMidLevel = ExpAllocateTablePagedPool( HandleTable->QuotaProcess,
                                             PAGE_SIZE
                                           );

    if (NewMidLevel == NULL) {

        return NULL;
    }

    //
    //  If we need a new mid-level, we'll need a low-level too.
    //  We'll create one and if success we'll save it at the first position
    //

    NewLowLevel = ExpAllocateLowLevelTable( HandleTable, DoInit );

    if (NewLowLevel == NULL) {

        ExpFreeTablePagedPool( HandleTable->QuotaProcess,
                               NewMidLevel,
                               PAGE_SIZE
                             );

        return NULL;
    }
    
    //
    //  Set the low-level table at the first index
    //

    NewMidLevel[0] = NewLowLevel;
    *pNewLowLevel = NewLowLevel;

    return NewMidLevel;
}



BOOLEAN
ExpAllocateHandleTableEntrySlow (
    IN PHANDLE_TABLE HandleTable,
    IN BOOLEAN DoInit
    )

/*++

Routine Description:

    This worker routine allocates a new handle table entry for the specified
    handle table.

    Note: The caller must have already locked the handle table

Arguments:

    HandleTable - Supplies the handle table being used

    DoInit - If FALSE then the caller (duplicate) doesn't need the free list built

Return Value:

    BOOLEAN - TRUE, Retry the fast allocation path, FALSE, We failed to allocate memory

--*/

{
    ULONG i,j;

    PHANDLE_TABLE_ENTRY NewLowLevel;
    PHANDLE_TABLE_ENTRY *NewMidLevel;
    PHANDLE_TABLE_ENTRY **NewHighLevel;
    ULONG NewFree, OldFree;
    ULONG OldIndex;
    PVOID OldValue;
    
    ULONG_PTR CapturedTable = HandleTable->TableCode;
    ULONG TableLevel = (ULONG)(CapturedTable & LEVEL_CODE_MASK);
    
    PAGED_CODE();

    //
    // Initializing NewLowLevel is not needed for
    // correctness but without it the compiler cannot compile this code
    // W4 to check for use of uninitialized variables.
    //

    NewLowLevel = NULL;

    CapturedTable = CapturedTable & ~LEVEL_CODE_MASK;


    if ( TableLevel == 0 ) {

        //
        //  We have a single level. We need to ad a mid-layer
        //  to the process handle table
        //

        NewMidLevel = ExpAllocateMidLevelTable( HandleTable, DoInit, &NewLowLevel );

        if (NewMidLevel == NULL) {
            return FALSE;
        }

        //
        //  Since ExpAllocateMidLevelTable initialize the 
        //  first position with a new table, we need to move it in 
        //  the second position, and store in the first position the current one
        //

        NewMidLevel[1] = NewMidLevel[0];
        NewMidLevel[0] = (PHANDLE_TABLE_ENTRY)CapturedTable;
            
        //
        //  Encode the current level and set it to the handle table process
        //

        CapturedTable = ((ULONG_PTR)NewMidLevel) | 1;
            
        OldValue = InterlockedExchangePointer( (PVOID *)&HandleTable->TableCode, (PVOID)CapturedTable );


    } else if (TableLevel == 1) {

        //
        //  We have a 2 levels handle table
        //

        PHANDLE_TABLE_ENTRY *TableLevel2 = (PHANDLE_TABLE_ENTRY *)CapturedTable;

        //
        //  Test whether the index we need to create is still in the 
        //  range for a 2 layers table
        //

        i = HandleTable->NextHandleNeedingPool / (LOWLEVEL_COUNT * HANDLE_VALUE_INC);

        if (i < MIDLEVEL_COUNT) {

            //
            //  We just need to allocate a new low-level
            //  table
            //
                
            NewLowLevel = ExpAllocateLowLevelTable( HandleTable, DoInit );

            if (NewLowLevel == NULL) {
                return FALSE;
            }

            //
            //  Set the new one to the table, at appropriate position
            //

            OldValue = InterlockedExchangePointer( (PVOID *) (&TableLevel2[i]), NewLowLevel );
            EXASSERT (OldValue == NULL);

        } else {

            //
            //  We exhausted the 2 level domain. We need to insert a new one
            //

            NewHighLevel = ExpAllocateTablePagedPool( HandleTable->QuotaProcess,
                                                      HIGHLEVEL_SIZE
                                                    );

            if (NewHighLevel == NULL) {

                return FALSE;
            }
                
            NewMidLevel = ExpAllocateMidLevelTable( HandleTable, DoInit, &NewLowLevel );

            if (NewMidLevel == NULL) {
                    
                ExpFreeTablePagedPool( HandleTable->QuotaProcess,
                                       NewHighLevel,
                                       HIGHLEVEL_SIZE
                                     );

                return FALSE;
            }

            //
            //  Initialize the first index with the previous mid-level layer
            //

            NewHighLevel[0] = (PHANDLE_TABLE_ENTRY*)CapturedTable;
            NewHighLevel[1] = NewMidLevel;

            //
            //  Encode the level into the table pointer
            //

            CapturedTable = ((ULONG_PTR)NewHighLevel) | 2;

            //
            //  Change the handle table pointer with this one
            //

            OldValue = InterlockedExchangePointer( (PVOID *)&HandleTable->TableCode, (PVOID)CapturedTable );

        }

    } else if (TableLevel == 2) {

        //
        //  we have already a table with 3 levels
        //

        ULONG RemainingIndex;
        PHANDLE_TABLE_ENTRY **TableLevel3 = (PHANDLE_TABLE_ENTRY **)CapturedTable;

        i = HandleTable->NextHandleNeedingPool / (MIDLEVEL_THRESHOLD * HANDLE_VALUE_INC);

        //
        //  Check whether we exhausted all possible indexes.
        //

        if (i >= HIGHLEVEL_COUNT) {

            return FALSE;
        }

        if (TableLevel3[i] == NULL) {

            //
            //  The new available handle points to a free mid-level entry
            //  We need then to allocate a new one and save it in that position
            //

            NewMidLevel = ExpAllocateMidLevelTable( HandleTable, DoInit, &NewLowLevel );
                
            if (NewMidLevel == NULL) {
                    
                return FALSE;
            }             

            OldValue = InterlockedExchangePointer( (PVOID *) &(TableLevel3[i]), NewMidLevel );
            EXASSERT (OldValue == NULL);

        } else {

            //
            //  We have already a mid-level table. We just need to add a new low-level one
            //  at the end
            //
                
            RemainingIndex = (HandleTable->NextHandleNeedingPool / HANDLE_VALUE_INC) -
                              i * MIDLEVEL_THRESHOLD;
            j = RemainingIndex / LOWLEVEL_COUNT;

            NewLowLevel = ExpAllocateLowLevelTable( HandleTable, DoInit );

            if (NewLowLevel == NULL) {

                return FALSE;
            }

            OldValue = InterlockedExchangePointer( (PVOID *)(&TableLevel3[i][j]) , NewLowLevel );
            EXASSERT (OldValue == NULL);
        }
    }

    //
    // This must be done after the table pointers so that new created handles
    // are valid before being freed.
    //
    OldIndex = InterlockedExchangeAdd ((PLONG) &HandleTable->NextHandleNeedingPool,
                                       LOWLEVEL_COUNT * HANDLE_VALUE_INC);


    if (DoInit) {
        //
        // Generate a new sequence number since this is a push
        //
        OldIndex += HANDLE_VALUE_INC + GetNextSeq();

        //
        // Now free the handles. These are all ready to be accepted by the lookup logic now.
        //
        while (1) {
            OldFree = ReadForWriteAccess (&HandleTable->FirstFree);
            NewLowLevel[LOWLEVEL_COUNT - 1].NextFreeTableEntry = OldFree;

            //
            // These are new entries that have never existed before. We can't have an A-B-A problem
            // with these so we don't need to take any locks
            //


            NewFree = InterlockedCompareExchange ((PLONG)&HandleTable->FirstFree,
                                                  OldIndex,
                                                  OldFree);
            if (NewFree == OldFree) {
                break;
            }
        }
    }
    return TRUE;
}


VOID
ExSetHandleTableStrictFIFO (
    IN PHANDLE_TABLE HandleTable
    )

/*++

Routine Description:

    This routine marks a handle table so that handle allocation is done in
    a strict FIFO order.


Arguments:

    HandleTable - Supplies the handle table being changed to FIFO

Return Value:

    None.

--*/

{
    HandleTable->StrictFIFO = TRUE;
}


//
//  Local Support Routine
//

//
//  The following is a global variable only present in the checked builds
//  to help catch apps that reuse handle values after they're closed.
//

#if DBG
BOOLEAN ExReuseHandles = 1;
#endif //DBG

VOID
ExpFreeHandleTableEntry (
    IN PHANDLE_TABLE HandleTable,
    IN EXHANDLE Handle,
    IN PHANDLE_TABLE_ENTRY HandleTableEntry
    )

/*++

Routine Description:

    This worker routine returns the specified handle table entry to the free
    list for the handle table.

    Note: The caller must have already locked the handle table

Arguments:

    HandleTable - Supplies the parent handle table being modified

    Handle - Supplies the handle of the entry being freed

    HandleTableEntry - Supplies the table entry being freed

Return Value:

    None.

--*/

{
    PHANDLE_TABLE_ENTRY_INFO EntryInfo;
    ULONG OldFree, NewFree, *Free;
    PKTHREAD CurrentThread;
    ULONG Idx;
    ULONG SeqInc;

    PAGED_CODE();

    EXASSERT (HandleTableEntry->Object == NULL);
    EXASSERT (HandleTableEntry == ExpLookupHandleTableEntry (HandleTable, Handle));

    //
    //  Clear the AuditMask flags if these are present into the table
    //

    EntryInfo = ExGetHandleInfo(HandleTable, Handle.GenericHandleOverlay, TRUE);

    if (EntryInfo) {

        EntryInfo->AuditMask = 0;
    }

    //
    //  A free is simply a push onto the free table entry stack, or in the
    //  debug case we'll sometimes just float the entry to catch apps who
    //  reuse a recycled handle value.
    //

    InterlockedDecrement (&HandleTable->HandleCount);
    CurrentThread = KeGetCurrentThread ();

    NewFree = (ULONG) Handle.Value & ~(HANDLE_VALUE_INC - 1);

#if DBG
    if (ExReuseHandles) {
#endif //DBG

        if (!HandleTable->StrictFIFO) {


            //
            // We are pushing potentially old entries onto the free list.
            // Prevent the A-B-A problem by shifting to an alternate list
            // read this element has the list head out of the loop.
            //
            Idx = (NewFree>>2) % HANDLE_TABLE_LOCKS;
            if (ExTryAcquireReleasePushLockExclusive (&HandleTable->HandleTableLock[Idx])) {
                SeqInc = GetNextSeq();
                Free = &HandleTable->FirstFree;
            } else {
                SeqInc = 0;
                Free = &HandleTable->LastFree;
            }
        } else {
            SeqInc = 0;
            Free = &HandleTable->LastFree;
        }

        while (1) {


            OldFree = ReadForWriteAccess (Free);
            HandleTableEntry->NextFreeTableEntry = OldFree;


            if ((ULONG)InterlockedCompareExchange ((PLONG)Free,
                                                   NewFree + SeqInc,
                                                   OldFree) == OldFree) {

                EXASSERT ((OldFree & FREE_HANDLE_MASK) < HandleTable->NextHandleNeedingPool);

                break;
            }
        }

#if DBG
    } else {

        HandleTableEntry->NextFreeTableEntry = 0;
    }
#endif //DBG


    return;
}


//
//  Local Support Routine
//

PHANDLE_TABLE_ENTRY
ExpLookupHandleTableEntry (
    IN PHANDLE_TABLE HandleTable,
    IN EXHANDLE tHandle
    )

/*++

Routine Description:

    This routine looks up and returns the table entry for the
    specified handle value.

Arguments:

    HandleTable - Supplies the handle table being queried

    tHandle - Supplies the handle value being queried

Return Value:

    Returns a pointer to the corresponding table entry for the input
        handle.  Or NULL if the handle value is invalid (i.e., too large
        for the tables current allocation.

--*/

{
    ULONG_PTR i,j,k;
    ULONG_PTR CapturedTable;
    ULONG TableLevel;
    PHANDLE_TABLE_ENTRY Entry = NULL;
    EXHANDLE Handle;

    PUCHAR TableLevel1;
    PUCHAR TableLevel2;
    PUCHAR TableLevel3;

    ULONG_PTR MaxHandle;

    PAGED_CODE();


    //
    // Extract the handle index
    //
    Handle = tHandle;

    Handle.TagBits = 0;

    MaxHandle = *(volatile ULONG *) &HandleTable->NextHandleNeedingPool;

    //
    // See if this can be a valid handle given the table levels.
    //
    if (Handle.Value >= MaxHandle) {
        return NULL;        
    }

    //
    // Now fetch the table address and level bits. We must preserve the
    // ordering here.
    //
    CapturedTable = *(volatile ULONG_PTR *) &HandleTable->TableCode;

    //
    //  we need to capture the current table. This routine is lock free
    //  so another thread may change the table at HandleTable->TableCode
    //

    TableLevel = (ULONG)(CapturedTable & LEVEL_CODE_MASK);
    CapturedTable = CapturedTable - TableLevel;

    //
    //  The lookup code depends on number of levels we have
    //

    switch (TableLevel) {
        
        case 0:
            
            //
            //  We have a simple index into the array, for a single level
            //  handle table
            //


            TableLevel1 = (PUCHAR) CapturedTable;

            //
            // The index for this level is already scaled by a factor of 4. Take advantage of this
            //

            Entry = (PHANDLE_TABLE_ENTRY) &TableLevel1[Handle.Value *
                                                       (sizeof (HANDLE_TABLE_ENTRY) / HANDLE_VALUE_INC)];

            break;
        
        case 1:
            
            //
            //  we have a 2 level handle table. We need to get the upper index
            //  and lower index into the array
            //


            TableLevel2 = (PUCHAR) CapturedTable;

            i = Handle.Value % (LOWLEVEL_COUNT * HANDLE_VALUE_INC);

            Handle.Value -= i;
            j = Handle.Value / ((LOWLEVEL_COUNT * HANDLE_VALUE_INC) / sizeof (PHANDLE_TABLE_ENTRY));

            TableLevel1 =  (PUCHAR) *(PHANDLE_TABLE_ENTRY *) &TableLevel2[j];
            Entry = (PHANDLE_TABLE_ENTRY) &TableLevel1[i * (sizeof (HANDLE_TABLE_ENTRY) / HANDLE_VALUE_INC)];

            break;
        
        case 2:
            
            //
            //  We have here a three level handle table.
            //


            TableLevel3 = (PUCHAR) CapturedTable;

            i = Handle.Value  % (LOWLEVEL_COUNT * HANDLE_VALUE_INC);

            Handle.Value -= i;

            k = Handle.Value / ((LOWLEVEL_COUNT * HANDLE_VALUE_INC) / sizeof (PHANDLE_TABLE_ENTRY));

            j = k % (MIDLEVEL_COUNT * sizeof (PHANDLE_TABLE_ENTRY));

            k -= j;

            k /= MIDLEVEL_COUNT;


            TableLevel2 = (PUCHAR) *(PHANDLE_TABLE_ENTRY *) &TableLevel3[k];
            TableLevel1 = (PUCHAR) *(PHANDLE_TABLE_ENTRY *) &TableLevel2[j];
            Entry = (PHANDLE_TABLE_ENTRY) &TableLevel1[i * (sizeof (HANDLE_TABLE_ENTRY) / HANDLE_VALUE_INC)];

            break;

        default :
            _assume (0);
    }

    return Entry;
}

NTKERNELAPI
NTSTATUS
ExSetHandleInfo (
    __inout PHANDLE_TABLE HandleTable,
    __in HANDLE Handle,
    __in PHANDLE_TABLE_ENTRY_INFO EntryInfo,
    __in BOOLEAN EntryLocked
    )

/*++

Routine Description:
    
    The routine set the entry info for the specified handle table
    
    Note: the handle entry must be locked when this function is called

Arguments:

    HandleTable - Supplies the handle table being queried

    Handle - Supplies the handle value being queried

Return Value:

--*/

{
    PKTHREAD CurrentThread;
    PHANDLE_TABLE_ENTRY InfoStructure;
    EXHANDLE ExHandle;
    NTSTATUS Status;
    PHANDLE_TABLE_ENTRY TableEntry;
    PHANDLE_TABLE_ENTRY_INFO InfoTable;

    Status = STATUS_UNSUCCESSFUL;
    TableEntry = NULL;
    CurrentThread = NULL;

    ExHandle.GenericHandleOverlay = Handle;
    ExHandle.Index &= ~(LOWLEVEL_COUNT - 1);

    if (!EntryLocked) {
        CurrentThread = KeGetCurrentThread ();
        KeEnterCriticalRegionThread (CurrentThread);
        TableEntry = ExMapHandleToPointer(HandleTable, Handle);

        if (TableEntry == NULL) {
            KeLeaveCriticalRegionThread (CurrentThread);
            
            return STATUS_UNSUCCESSFUL;
        }
    }
    
    //
    //  The info structure is at the first position in each low-level table
    //

    InfoStructure = ExpLookupHandleTableEntry( HandleTable,
                                               ExHandle
                                             );

    if (InfoStructure == NULL || InfoStructure->NextFreeTableEntry != EX_ADDITIONAL_INFO_SIGNATURE) {

        if ( TableEntry ) {
            ExUnlockHandleTableEntry( HandleTable, TableEntry );
            KeLeaveCriticalRegionThread (CurrentThread);
        }

        return STATUS_UNSUCCESSFUL;
    }

    //
    //  Check whether we need to allocate a new table
    //
    InfoTable = InfoStructure->InfoTable;
    if (InfoTable == NULL) {
        //
        //  Nobody allocated the Infotable so far.
        //  We'll do it right now
        //

        InfoTable = ExpAllocateTablePagedPool (HandleTable->QuotaProcess,
                                               LOWLEVEL_COUNT * sizeof(HANDLE_TABLE_ENTRY_INFO));
            
        if (InfoTable) {

            //
            // Update the number of pages for extra info. If somebody beat us to it then free the
            // new table
            //
            if (InterlockedCompareExchangePointer (&InfoStructure->InfoTable,
                                                   InfoTable,
                                                   NULL) == NULL) {

                InterlockedIncrement(&HandleTable->ExtraInfoPages);

            } else {
                ExpFreeTablePagedPool (HandleTable->QuotaProcess,
                                       InfoTable,
                                       LOWLEVEL_COUNT * sizeof(HANDLE_TABLE_ENTRY_INFO));
                InfoTable = InfoStructure->InfoTable;
            }
        }
    }

    if (InfoTable != NULL) {
        
        //
        //  Calculate the index and copy the structure
        //

        ExHandle.GenericHandleOverlay = Handle;

        InfoTable[ExHandle.Index % LOWLEVEL_COUNT] = *EntryInfo;

        Status = STATUS_SUCCESS;
    }

    if ( TableEntry ) {

        ExUnlockHandleTableEntry( HandleTable, TableEntry );
        KeLeaveCriticalRegionThread (CurrentThread);
    }
    
    return Status;
}

NTKERNELAPI
PHANDLE_TABLE_ENTRY_INFO
ExpGetHandleInfo (
    __in PHANDLE_TABLE HandleTable,
    __in HANDLE Handle,
    __in BOOLEAN EntryLocked
    )

/*++

Routine Description:
    
    The routine reads the entry info for the specified handle table
    
    Note: the handle entry must be locked when this function is called

Arguments:

    HandleTable - Supplies the handle table being queried

    Handle - Supplies the handle value being queried

Return Value:

--*/

{
    PHANDLE_TABLE_ENTRY InfoStructure;
    EXHANDLE ExHandle;
    PHANDLE_TABLE_ENTRY TableEntry = NULL;
    
    ExHandle.GenericHandleOverlay = Handle;
    ExHandle.Index &= ~(LOWLEVEL_COUNT - 1);

    if (!EntryLocked) {

        TableEntry = ExMapHandleToPointer(HandleTable, Handle);

        if (TableEntry == NULL) {
            
            return NULL;
        }
    }
    
    //
    //  The info structure is at the first position in each low-level table
    //

    InfoStructure = ExpLookupHandleTableEntry( HandleTable,
                                               ExHandle 
                                             );

    if (InfoStructure == NULL || InfoStructure->NextFreeTableEntry != EX_ADDITIONAL_INFO_SIGNATURE ||
        InfoStructure->InfoTable == NULL) {

        if ( TableEntry ) {
            
            ExUnlockHandleTableEntry( HandleTable, TableEntry );
        }

        return NULL;
    }


    //
    //  Return a pointer to the info structure
    //

    ExHandle.GenericHandleOverlay = Handle;

    return &(InfoStructure->InfoTable[ExHandle.Index % LOWLEVEL_COUNT]);
}

#if DBG
ULONG g_ulExpUpdateDebugInfoDebugLevel = 0;
#endif
void ExpUpdateDebugInfo(
    PHANDLE_TABLE HandleTable,
    PETHREAD CurrentThread,
    HANDLE Handle,
    ULONG Type) 
{
    BOOLEAN LockAcquired = FALSE;
    PHANDLE_TRACE_DEBUG_INFO DebugInfo;

    DebugInfo = ExReferenceHandleDebugInfo (HandleTable);

    if (DebugInfo == NULL) {
        return;
    }

#if DBG
    if (g_ulExpUpdateDebugInfoDebugLevel > 10)
    {
        DbgPrint ("ExpUpdateDebugInfo() BitMaskFlags=0x%x, CurrentStackIndex=%d, Handle=0x%p, Type=%d \n", 
                  DebugInfo->BitMaskFlags,
                  DebugInfo->CurrentStackIndex,
                  Handle,
                  Type);
    }
#endif
    if (DebugInfo->BitMaskFlags & (HANDLE_TRACE_DEBUG_INFO_CLEAN_DEBUG_INFO | HANDLE_TRACE_DEBUG_INFO_COMPACT_CLOSE_HANDLE)) {
        //
        // we wish to preserve lock-free behavior in the non-compaction path
        // so we lock only in this path
        //
        ExAcquireFastMutex(&DebugInfo->CloseCompactionLock);
        LockAcquired = TRUE;
    }

    if (DebugInfo->BitMaskFlags & HANDLE_TRACE_DEBUG_INFO_CLEAN_DEBUG_INFO) {
        //
        // clean debug info, but not the fast mutex!
        //

        ASSERT(LockAcquired);

        DebugInfo->BitMaskFlags &= ~HANDLE_TRACE_DEBUG_INFO_CLEAN_DEBUG_INFO;
        DebugInfo->BitMaskFlags &= ~HANDLE_TRACE_DEBUG_INFO_WAS_WRAPPED_AROUND;
        DebugInfo->BitMaskFlags |= HANDLE_TRACE_DEBUG_INFO_WAS_SOMETIME_CLEANED;
        DebugInfo->CurrentStackIndex = 0;
        RtlZeroMemory (DebugInfo->TraceDb,
                       sizeof (*DebugInfo) +
                       DebugInfo->TableSize * sizeof (DebugInfo->TraceDb[0]) -
                       sizeof (DebugInfo->TraceDb));
    }

    if (
        (DebugInfo->BitMaskFlags & HANDLE_TRACE_DEBUG_INFO_COMPACT_CLOSE_HANDLE) &&
        (Type == HANDLE_TRACE_DB_CLOSE)
       ){
        //
        // This is assuming that either:
        // 1) this flag was set from the beginning, so there are no close items.
        // 2) this flag was set via KD, in which case HANDLE_TRACE_DEBUG_INFO_CLEAN_DEBUG_INFO
        //    must have been set also, so again there are no close items
        //
        ULONG uiMaxNumOfItemsInTraceDb;
        ULONG uiNextItem;

        ASSERT(LockAcquired);

        //
        // look for the matching open item, remove them from the list, and compact the list
        //
        uiMaxNumOfItemsInTraceDb = (DebugInfo->BitMaskFlags & HANDLE_TRACE_DEBUG_INFO_WAS_WRAPPED_AROUND) ? DebugInfo->TableSize : DebugInfo->CurrentStackIndex ;
        for (uiNextItem = 1; uiNextItem <= uiMaxNumOfItemsInTraceDb; uiNextItem++) {
            //
            // if HANDLE_TRACE_DEBUG_INFO_COMPACT_CLOSE_HANDLE is on
            // there could be no HANDLE_TRACE_DB_CLOSE items
            // This ASSERT can fire if the HANDLE_TRACE_DEBUG_INFO_COMPACT_CLOSE_HANDLE flag
            // is set dynamically, so another thread was not using the locks to add
            // items to the list, and could have added a HANDLE_TRACE_DB_CLOSE item.
            //
            ASSERT(DebugInfo->TraceDb[uiNextItem%DebugInfo->TableSize].Type != HANDLE_TRACE_DB_CLOSE);

            if (
                (DebugInfo->TraceDb[uiNextItem%DebugInfo->TableSize].Type == HANDLE_TRACE_DB_OPEN) &&
                (DebugInfo->TraceDb[uiNextItem%DebugInfo->TableSize].Handle == Handle) 
                ) {
                //
                // found the matching open, compact the list
                //
                ULONG IndexToMoveBack;
                DebugInfo->CurrentStackIndex--;
                IndexToMoveBack = DebugInfo->CurrentStackIndex % DebugInfo->TableSize;
                if (0 != IndexToMoveBack)
                {
                    DebugInfo->TraceDb[uiNextItem%DebugInfo->TableSize] = DebugInfo->TraceDb[IndexToMoveBack];
                }
                else
                {
                    //
                    // the list is already empty
                    //
                }
                break;
            }
        }
        if (!(uiNextItem <= uiMaxNumOfItemsInTraceDb)) {
            //
            // a matching open was not found.
            // this must mean that we wrapped around, or cleaned the list sometime after
            // it was created
            // or that a duplicated handle was created before handle tracking was initiated
            // so we cannot ASSERT for that
            //
            /*
            ASSERT(
                (DebugInfo->BitMaskFlags & HANDLE_TRACE_DEBUG_INFO_WAS_SOMETIME_CLEANED) ||
                (DebugInfo->BitMaskFlags & HANDLE_TRACE_DEBUG_INFO_WAS_WRAPPED_AROUND)
                );
                */
            //
            // just ignore this HANDLE_TRACE_DB_CLOSE
            //
        }
    }
    else
    {
        PHANDLE_TRACE_DB_ENTRY DebugEntry;
        ULONG Index = ((ULONG) InterlockedIncrement ((PLONG)&DebugInfo->CurrentStackIndex))
                   % DebugInfo->TableSize;
        ASSERT((Type != HANDLE_TRACE_DB_CLOSE) || (!(DebugInfo->BitMaskFlags & HANDLE_TRACE_DEBUG_INFO_COMPACT_CLOSE_HANDLE)));
        if (0 == Index) {
            //
            // this is a wraparound of the db, mark it as such, and if there's 
            // a debugger attached break into it
            //
            DebugInfo->BitMaskFlags |= HANDLE_TRACE_DEBUG_INFO_WAS_WRAPPED_AROUND;
            if(DebugInfo->BitMaskFlags & HANDLE_TRACE_DEBUG_INFO_BREAK_ON_WRAP_AROUND)
            {
                __try {
                    DbgBreakPoint();
                }
                __except(1) {
                    NOTHING;
                }
            }
        }
        DebugEntry = &DebugInfo->TraceDb[Index];
        DebugEntry->ClientId = CurrentThread->Cid;
        DebugEntry->Handle   = Handle;
        DebugEntry->Type     = Type;
        Index = RtlWalkFrameChain (DebugEntry->StackTrace, HANDLE_TRACE_DB_STACK_SIZE, 0);
        RtlWalkFrameChain (&DebugEntry->StackTrace[Index], HANDLE_TRACE_DB_STACK_SIZE - Index, 1);
    }

    if (LockAcquired)
    {
        ExReleaseFastMutex(&DebugInfo->CloseCompactionLock);
    }

    ExDereferenceHandleDebugInfo (HandleTable, DebugInfo);
}

VOID
ExHandleTest (
    )
{
    PHANDLE_TABLE HandleTable;
    ULONG i, j, k;
#define MAX_ALLOCS 20
    PHANDLE_TABLE_ENTRY HandleEntryArray[MAX_ALLOCS];
    EXHANDLE Handle[MAX_ALLOCS];
    LARGE_INTEGER CurrentTime;

    HandleTable = PsGetCurrentProcess ()->ObjectTable;
    HandleTable->StrictFIFO = 0;

    k = 0;
    for (i = 0; i < 100000; i++) {
        KeQuerySystemTime (&CurrentTime);
        for (j = 0; j < MAX_ALLOCS; j++) {
            //
            // Clears Handle.Index and Handle.TagBits
            //

            Handle[j].GenericHandleOverlay = NULL;


            //
            //  Allocate a new handle table entry, and get the handle value
            //

            HandleEntryArray[j] = ExpAllocateHandleTableEntry (HandleTable,
                                                               &Handle[j]);
            if (HandleEntryArray[j] != NULL) {
                HandleEntryArray[j]->NextFreeTableEntry = 0x88888888;
            }
        }

        for (j = MAX_ALLOCS; j > 0; j--) {
            k = k + CurrentTime.LowPart;
            k = k % j;
            CurrentTime.QuadPart >>= 3;
            if (HandleEntryArray[k] != NULL) {
                //
                // Free the entry
                //
                ExpFreeHandleTableEntry (HandleTable,
                                         Handle[k],
                                         HandleEntryArray[k]);
                HandleEntryArray[k] = HandleEntryArray[j-1];
                HandleEntryArray[j-1] = NULL;
                Handle[k] = Handle[j-1];
                Handle[j-1].GenericHandleOverlay = NULL;
            }
        }
    }
}

