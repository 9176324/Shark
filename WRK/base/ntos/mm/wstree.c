/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   wstree.c

Abstract:

    This module contains the routines which manipulate the working
    set list tree.

--*/

#include "mi.h"

extern ULONG MmSystemCodePage;
extern ULONG MmSystemCachePage;
extern ULONG MmPagedPoolPage;
extern ULONG MmSystemDriverPage;

#if DBG
ULONG MmNumberOfInserts;
#endif

#if defined (_WIN64)
ULONG MiWslePteLoops = 16;
#endif

VOID
MiRepointWsleHashIndex (
    IN MMWSLE WsleEntry,
    IN PMMWSL WorkingSetList,
    IN WSLE_NUMBER NewWsIndex
    );

VOID
MiCheckWsleHash (
    IN PMMWSL WorkingSetList
    );


VOID
FASTCALL
MiInsertWsleHash (
    IN WSLE_NUMBER Entry,
    IN PMMSUPPORT WsInfo
    )

/*++

Routine Description:

    This routine inserts a Working Set List Entry (WSLE) into the
    hash list for the specified working set.

Arguments:

    Entry - The index number of the WSLE to insert.

    WorkingSetInfo - Supplies the working set structure to insert into.

Return Value:

    None.

Environment:

    Kernel mode, APCs disabled, Working Set Mutex held.

--*/

{
    ULONG Tries;
    PMMWSLE Wsle;
    WSLE_NUMBER Hash;
    PMMWSLE_HASH Table;
    WSLE_NUMBER j;
    WSLE_NUMBER Index;
    ULONG HashTableSize;
    PMMWSL WorkingSetList;
    PMMPTE PointerPte;
    WSLE_NUMBER EntriesExamined;
    WSLE_NUMBER CurrentVictimHashIndex;
    MMWSLE CurrentVictimWsleContents;
    ULONG CurrentVictimAccessed;

    WorkingSetList = WsInfo->VmWorkingSetList;

    Wsle = WorkingSetList->Wsle;

    ASSERT (Wsle[Entry].u1.e1.Valid == 1);
    ASSERT (Wsle[Entry].u1.e1.Direct != 1);
    ASSERT (Wsle[Entry].u1.e1.Hashed == 0);

    Table = WorkingSetList->HashTable;

    if (Table == NULL) {
        return;
    }

#if DBG
    MmNumberOfInserts += 1;
#endif

    Hash = MI_WSLE_HASH (Wsle[Entry].u1.Long, WorkingSetList);

    HashTableSize = WorkingSetList->HashTableSize;

    //
    // Check hash table size and see if there is enough room to
    // hash or if the table should be grown.
    //

    if ((WorkingSetList->NonDirectCount + 10 + (HashTableSize >> 4)) >
                HashTableSize) {

        if ((Table + HashTableSize + ((2*PAGE_SIZE) / sizeof (MMWSLE_HASH)) <= (PMMWSLE_HASH)WorkingSetList->HighestPermittedHashAddress)) {

            WsInfo->Flags.GrowWsleHash = 1;
        }

        if ((WorkingSetList->NonDirectCount + (HashTableSize >> 4)) >
                HashTableSize) {

            //
            // Look for a free entry (or repurpose one if necessary) in the
            // hash table, space is tight.
            //

            j = Hash;
            Tries = 0;
            EntriesExamined = 16;
            if (EntriesExamined > HashTableSize) {
                EntriesExamined = HashTableSize;
            }
            CurrentVictimHashIndex = WSLE_NULL_INDEX;
            CurrentVictimWsleContents.u1.Long = 0;
            CurrentVictimAccessed = 0;

            do {

                if (Table[j].Key == 0) {

                    //
                    // This one's free so just use it.
                    //

                    Hash = j;
                    goto GotHash;
                }

                //
                // Examine this in-use one.  We're looking for the least
                // recently accessed one to use as a victim.  Use the WSLE
                // age bits and the PTE access bit to compute this.
                //

                Index = Table[j].Index;
                ASSERT (Wsle[Index].u1.e1.Direct == 0);
                ASSERT (Wsle[Index].u1.e1.Valid == 1);

                ASSERT (Table[j].Key == MI_GENERATE_VALID_WSLE (&Wsle[Index]));

                if (CurrentVictimHashIndex == WSLE_NULL_INDEX) {
                    CurrentVictimHashIndex = j;
                    CurrentVictimWsleContents = Wsle[Index];
                    CurrentVictimAccessed = (ULONG) MI_GET_ACCESSED_IN_PTE (
                                                MiGetPteAddress (Table[j].Key));
                }
                else {
                    if (Wsle[Index].u1.e1.Age > CurrentVictimWsleContents.u1.e1.Age) {
                        CurrentVictimHashIndex = j;
                        CurrentVictimWsleContents = Wsle[Index];
                        CurrentVictimAccessed = (ULONG) MI_GET_ACCESSED_IN_PTE (
                                                MiGetPteAddress (Table[j].Key));
                    }
                    else if (Wsle[Index].u1.e1.Age == CurrentVictimWsleContents.u1.e1.Age) {
                        if (CurrentVictimAccessed) {
                            PointerPte = MiGetPteAddress (Table[j].Key);
                            if (MI_GET_ACCESSED_IN_PTE (PointerPte) == 0) {
                                CurrentVictimHashIndex = j;
                                CurrentVictimWsleContents = Wsle[Index];
                                CurrentVictimAccessed = 0;
                            }
                        }
                    }
                }

                EntriesExamined -= 1;
                if (EntriesExamined == 0) {

                    //
                    // We've searched enough, take what we've got.
                    //

                    Index = Table[CurrentVictimHashIndex].Index;

                    if (CurrentVictimWsleContents.u1.e1.Age >= MI_PASS0_TRIM_AGE) {
                        //
                        // This address hasn't been accessed in a while, so
                        // take the WSLE and the hash entry.
                        //

                        if (MiFreeWsle (Index,
                                        WsInfo,
                                        MiGetPteAddress (Table[CurrentVictimHashIndex].Key))) {

                            //
                            // The previous entry (WSLE & hash) has been
                            // eliminated, so take it.
                            //

                            ASSERT (Table[CurrentVictimHashIndex].Key == 0);
                            Hash = CurrentVictimHashIndex;
                            goto GotHash;
                        }

                        //
                        // Fall through to take just the hash entry.
                        //
                    }

                    //
                    // Purge the previous entry's hash index (but not WSLE) so
                    // it can be reused for the new entry.  This is nice
                    // because it preserves both entries in the working set
                    // (although it is a bit more costly to remove the original
                    // entry later since it won't have a hash entry).
                    //

                    ASSERT (Wsle[Index].u1.e1.Hashed == 1);
                    Wsle[Index].u1.e1.Hashed = 0;

                    Table[CurrentVictimHashIndex].Key = 0;
                    Hash = CurrentVictimHashIndex;
                    goto GotHash;
                }

                j += 1;

                if (j >= HashTableSize) {
                    j = 0;
                    ASSERT (Tries == 0);
                    Tries = 1;
                }

            } while (TRUE);
        }
    }

    //
    // Add to the hash table if there is space.
    //

    Tries = 0;
    j = Hash;

    while (Table[Hash].Key != 0) {
        Hash += 1;
        if (Hash >= HashTableSize) {
            ASSERT (Tries == 0);
            Hash = 0;
            Tries = 1;
        }
        if (j == Hash) {
            ASSERT (Wsle[Entry].u1.e1.Hashed == 0);
            return;
        }
    }

GotHash:

    ASSERT (Table[Hash].Key == 0);
    ASSERT (Hash < HashTableSize);
    Wsle[Entry].u1.e1.Hashed = 1;

    //
    // Or in the valid bit so subsequent searches below can compare in
    // one instruction for both the virtual address and the valid bit.
    //

    Table[Hash].Key = MI_GENERATE_VALID_WSLE (&Wsle[Entry]);
    Table[Hash].Index = Entry;

#if DBG
    if ((MmNumberOfInserts % 1000) == 0) {
        MiCheckWsleHash (WorkingSetList);
    }
#endif
    return;
}

VOID
MiFillWsleHash (
    IN PMMWSL WorkingSetList
    )

/*++

Routine Description:

    This function fills the hash table with all the indirect WSLEs.
    This is typically called after the hash table has been freshly grown.

Arguments:

    WorkingSetList - Supplies a pointer to the working set list structure.

Return Value:

    None.

Environment:

    Kernel mode, APCs disabled, working set lock held.

--*/

{
    LONG Size;
    PMMWSLE Wsle;
    PMMPTE PointerPte;
    PMMWSLE_HASH Table;
    PMMWSLE_HASH OriginalTable;
    PMMWSLE_HASH StartTable;
    PMMWSLE_HASH EndTable;
    PFN_NUMBER PageFrameIndex;
    WSLE_NUMBER Count;
    WSLE_NUMBER Index;
    PMMPFN Pfn1;

    Table = WorkingSetList->HashTable;
    Count = WorkingSetList->NonDirectCount;
    Size = WorkingSetList->HashTableSize;

    StartTable = Table;
    EndTable = Table + Size;

    Index = 0;
    for (Wsle = WorkingSetList->Wsle; TRUE; Wsle += 1, Index += 1) {

        ASSERT (Wsle <= WorkingSetList->Wsle + WorkingSetList->LastEntry);

        if (Wsle->u1.e1.Valid == 0) {
            continue;
        }

        if (Wsle->u1.e1.Direct == 0) {

            Wsle->u1.e1.Hashed = 0;

            PointerPte = MiGetPteAddress (Wsle->u1.VirtualAddress);
            ASSERT (PointerPte->u.Hard.Valid);
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            if (Pfn1->u1.WsIndex == Index) {

                //
                // Even though this working set is not the direct owner, it
                // was able to share the direct owner's index.  So don't
                // bother hashing it for now.  This index will remain
                // valid for us unless the direct owner swaps this WSL
                // entry or our working set gets compressed.
                //

                goto DidOne;
            }

            //
            // Hash this.
            //

            Table = &StartTable[MI_WSLE_HASH (Wsle->u1.Long, WorkingSetList)];
            OriginalTable = Table;

            while (Table->Key != 0) {
                Table += 1;
                ASSERT (Table <= EndTable);
                if (Table == EndTable) {
                    Table = StartTable;
                }
                if (Table == OriginalTable) {

                    //
                    // Not enough space to hash everything but that's ok.
                    // Just bail out, we'll do linear walks to lookup this
                    // entry until the hash can be further expanded later.
                    //
                    // Mark any remaining entries as having no hash entry.
                    //

                    do {
                        ASSERT (Wsle <= WorkingSetList->Wsle + WorkingSetList->LastEntry);
                        if ((Wsle->u1.e1.Valid == 1) &&
                            (Wsle->u1.e1.Direct == 0)) {

                            Wsle->u1.e1.Hashed = 0;
                            Count -= 1;
                        }
                        Wsle += 1;
                    } while (Count != 0);

                    return;
                }
            }

            Table->Key = MI_GENERATE_VALID_WSLE (Wsle);
            Table->Index = Index;

            Wsle->u1.e1.Hashed = 1;

DidOne:
            Count -= 1;
            if (Count == 0) {
                break;
            }
        }
        else {
            ASSERT ((Wsle->u1.e1.Hashed == 0) ||
                    (Wsle->u1.e1.LockedInWs == 1) ||
                    (Wsle->u1.e1.LockedInMemory == 1));
        }
    }

#if DBG
    MiCheckWsleHash (WorkingSetList);
#endif
    return;
}

#if DBG
VOID
MiCheckWsleHash (
    IN PMMWSL WorkingSetList
    )
{
    ULONG i;
    ULONG found;
    PMMWSLE Wsle;

    found = 0;
    Wsle = WorkingSetList->Wsle;

    for (i = 0; i < WorkingSetList->HashTableSize; i += 1) {
        if (WorkingSetList->HashTable[i].Key != 0) {
            found += 1;
            ASSERT (WorkingSetList->HashTable[i].Key ==
                MI_GENERATE_VALID_WSLE (&Wsle[WorkingSetList->HashTable[i].Index]));
        }
    }
    if (found > WorkingSetList->NonDirectCount) {
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
            "MMWSLE: Found %lx, nondirect %lx\n",
                    found, WorkingSetList->NonDirectCount);
        DbgBreakPoint ();
    }
}
#endif



WSLE_NUMBER
FASTCALL
MiLocateWsle (
    IN PVOID VirtualAddress,
    IN PMMWSL WorkingSetList,
    IN WSLE_NUMBER WsPfnIndex,
    IN LOGICAL Deletion
    )

/*++

Routine Description:

    This function locates the specified virtual address within the
    working set list.

Arguments:

    VirtualAddress - Supplies the virtual address to locate within the working
                     set list.

    WorkingSetList - Supplies the working set list to search.

    WsPfnIndex - Supplies a hint to try before hashing or walking linearly.

    Deletion - Supplies TRUE if the WSLE is going to be deleted by the caller.
               This is used as a hint to remove the WSLE hash so that it does
               not have to be searched for twice.

Return Value:

    Returns the index into the working set list which contains the entry.

Environment:

    Kernel mode, APCs disabled, Working Set Mutex held.

--*/

{
    PMMWSLE Wsle;
    PMMWSLE LastWsle;
    WSLE_NUMBER Hash;
    WSLE_NUMBER StartHash;
    PMMWSLE_HASH Table;
    ULONG Tries;
#if defined (_WIN64)
    ULONG LoopCount;
    WSLE_NUMBER WsPteIndex;
    PMMPTE PointerPte;
#endif

    Wsle = WorkingSetList->Wsle;
    VirtualAddress = PAGE_ALIGN (VirtualAddress);

    //
    // Or in the valid bit so the compares in the loops below can be done in
    // one instruction for both the virtual address and the valid bit.
    //

    VirtualAddress = (PVOID)((ULONG_PTR)VirtualAddress | 0x1);

    if ((WsPfnIndex <= WorkingSetList->LastInitializedWsle) &&
        (MI_GENERATE_VALID_WSLE (&Wsle[WsPfnIndex]) == VirtualAddress)) {

        return WsPfnIndex;
    }

#if defined (_WIN64)
    PointerPte = MiGetPteAddress (VirtualAddress);
    WsPteIndex = MI_GET_WORKING_SET_FROM_PTE (PointerPte);

    if (WsPteIndex != 0) {

        LoopCount = MiWslePteLoops;

        while (WsPteIndex <= WorkingSetList->LastInitializedWsle) {

            if (MI_GENERATE_VALID_WSLE (&Wsle[WsPteIndex]) == VirtualAddress) {
                return WsPteIndex;
            }

            LoopCount -= 1;

            //
            // No working set index so far for this PTE.  Since the working
            // set may be very large (8TB would mean up to half a million loops)
            // just fall back to the hash instead.
            //

            if (LoopCount == 0) {
                break;
            }

            WsPteIndex += MI_MAXIMUM_PTE_WORKING_SET_INDEX;
        }
    }
#endif

    if (WorkingSetList->HashTable != NULL) {
        Tries = 0;
        Table = WorkingSetList->HashTable;

        Hash = MI_WSLE_HASH (VirtualAddress, WorkingSetList);
        StartHash = Hash;

        while (Table[Hash].Key != VirtualAddress) {
            Hash += 1;
            if (Hash >= WorkingSetList->HashTableSize) {
                ASSERT (Tries == 0);
                Hash = 0;
                Tries = 1;
            }
            if (Hash == StartHash) {
                Tries = 2;
                break;
            }
        }
        if (Tries < 2) {
            WsPfnIndex = Table[Hash].Index;
            ASSERT (Wsle[WsPfnIndex].u1.e1.Hashed == 1);
            ASSERT (Wsle[WsPfnIndex].u1.e1.Direct == 0);
            ASSERT (MI_GENERATE_VALID_WSLE (&Wsle[WsPfnIndex]) == Table[Hash].Key);
            if (Deletion) {
                Wsle[WsPfnIndex].u1.e1.Hashed = 0;
                Table[Hash].Key = 0;
            }
            return WsPfnIndex;
        }
    }

    LastWsle = Wsle + WorkingSetList->LastInitializedWsle;

    do {
        if (MI_GENERATE_VALID_WSLE (Wsle) == VirtualAddress) {

            ASSERT ((Wsle->u1.e1.Hashed == 0) ||
                    (WorkingSetList->HashTable == NULL));

            ASSERT (Wsle->u1.e1.Direct == 0);
            return (WSLE_NUMBER)(Wsle - WorkingSetList->Wsle);
        }
        Wsle += 1;

    } while (Wsle <= LastWsle);

    KeBugCheckEx (MEMORY_MANAGEMENT,
                  0x41284,
                  (ULONG_PTR) VirtualAddress,
                  WsPfnIndex,
                  (ULONG_PTR) WorkingSetList);
}


VOID
FASTCALL
MiRemoveWsle (
    IN WSLE_NUMBER Entry,
    IN PMMWSL WorkingSetList
    )

/*++

Routine Description:

    This routine removes a Working Set List Entry (WSLE) from the
    working set.

Arguments:

    Entry - The index number of the WSLE to remove.


Return Value:

    None.

Environment:

    Kernel mode, APCs disabled, Working Set Mutex held.

--*/
{
    PMMWSLE Wsle;
    PVOID VirtualAddress;
    PMMWSLE_HASH Table;
    MMWSLE WsleContents;
    WSLE_NUMBER Hash;
    WSLE_NUMBER StartHash;
    ULONG Tries;

    Wsle = WorkingSetList->Wsle;

    //
    // Locate the entry in the tree.
    //

    if (Entry > WorkingSetList->LastInitializedWsle) {
        KeBugCheckEx (MEMORY_MANAGEMENT,
                      0x41785,
                      (ULONG_PTR)WorkingSetList,
                      Entry,
                      0);
    }

    WsleContents = Wsle[Entry];

    ASSERT (WsleContents.u1.e1.Valid == 1);

    VirtualAddress = PAGE_ALIGN (WsleContents.u1.VirtualAddress);

    if (WorkingSetList == MmSystemCacheWorkingSetList) {

        //
        // Count system space inserts and removals.
        //

        if ((VirtualAddress >= MmPagedPoolStart) && (VirtualAddress <= MmPagedPoolEnd)) {
            MmPagedPoolPage -= 1;
        }
        else if (MI_IS_SYSTEM_CACHE_ADDRESS (VirtualAddress)) {
            MmSystemCachePage -= 1;
        }
        else if ((VirtualAddress <= MmSpecialPoolEnd) && (VirtualAddress >= MmSpecialPoolStart)) {
            MmPagedPoolPage -= 1;
        }
        else if (VirtualAddress >= MiLowestSystemPteVirtualAddress) {
            MmSystemDriverPage -= 1;
        }
        else {
            MmSystemCodePage -= 1;
        }
    }

    WsleContents.u1.e1.Valid = 0;

    MI_LOG_WSLE_CHANGE (WorkingSetList, Entry, WsleContents);

    Wsle[Entry].u1.e1.Valid = 0;

    //
    // Skip entries that cannot possibly be hashed ...
    //

    if (WsleContents.u1.e1.Direct == 0) {

        WorkingSetList->NonDirectCount -= 1;

        if (WsleContents.u1.e1.Hashed == 0) {
#if DBG
            if (WorkingSetList->HashTable != NULL) {
    
                //
                // Or in the valid bit so virtual address 0 is handled
                // properly (instead of matching a free hash entry).
                //

                VirtualAddress = (PVOID)((ULONG_PTR)VirtualAddress | 0x1);

                Table = WorkingSetList->HashTable;
                for (Hash = 0; Hash < WorkingSetList->HashTableSize; Hash += 1) {
                    ASSERT (Table[Hash].Key != VirtualAddress);
                }
            }
#endif
            return;
        }

        if (WorkingSetList->HashTable != NULL) {

            Hash = MI_WSLE_HASH (WsleContents.u1.Long, WorkingSetList);
            Table = WorkingSetList->HashTable;
            Tries = 0;
            StartHash = Hash;

            //
            // Or in the valid bit so virtual address 0 is handled
            // properly (instead of matching a free hash entry).
            //

            VirtualAddress = (PVOID)((ULONG_PTR)VirtualAddress | 0x1);

            while (Table[Hash].Key != VirtualAddress) {
                Hash += 1;
                if (Hash >= WorkingSetList->HashTableSize) {
                    ASSERT (Tries == 0);
                    Hash = 0;
                    Tries = 1;
                }
                if (Hash == StartHash) {

                    //
                    // The entry could not be found in the hash, it must
                    // never have been inserted.  This is ok, we don't
                    // need to do anything more in this case.
                    //

                    ASSERT (WsleContents.u1.e1.Hashed == 0);
                    return;
                }
            }

            ASSERT (WsleContents.u1.e1.Hashed == 1);
            Table[Hash].Key = 0;
        }
    }

    return;
}


VOID
MiSwapWslEntries (
    IN WSLE_NUMBER SwapEntry,
    IN WSLE_NUMBER Entry,
    IN PMMSUPPORT WsInfo,
    IN LOGICAL EntryNotInHash
    )

/*++

Routine Description:

    This routine swaps the working set list entries Entry and SwapEntry
    in the specified working set list.

Arguments:

    SwapEntry - Supplies the first entry to swap.  This entry must be
                valid, i.e. in the working set at the current time.

    Entry - Supplies the other entry to swap.  This entry may be valid
            or invalid.

    WsInfo - Supplies the working set list.

    EntryNotInHash - Supplies TRUE if the Entry cannot possibly be in the hash
                     table (ie, it is newly allocated), so the hash table
                     search can be skipped.

Return Value:

    None.

Environment:

    Kernel mode, Working set lock and PFN lock held (if system cache),
                 APCs disabled.

--*/

{
    MMWSLE WsleEntry;
    MMWSLE WsleSwap;
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    PMMWSLE Wsle;
    PMMWSL WorkingSetList;
    PMMWSLE_HASH Table;

    WorkingSetList = WsInfo->VmWorkingSetList;
    Wsle = WorkingSetList->Wsle;

    WsleSwap = Wsle[SwapEntry];

    ASSERT (WsleSwap.u1.e1.Valid != 0);

    WsleEntry = Wsle[Entry];

    Table = WorkingSetList->HashTable;

    if (WsleEntry.u1.e1.Valid == 0) {

        //
        // Entry is not on any list. Remove it from the free list.
        //

        MiRemoveWsleFromFreeList (Entry, Wsle, WorkingSetList);

        //
        // Copy the entry to this free one.
        //

        MI_LOG_WSLE_CHANGE (WorkingSetList, Entry, WsleSwap);

        Wsle[Entry] = WsleSwap;

        PointerPte = MiGetPteAddress (WsleSwap.u1.VirtualAddress);

        if (PointerPte->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
            if (!NT_SUCCESS (MiCheckPdeForPagedPool (WsleSwap.u1.VirtualAddress))) {
#endif

                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x41289,
                              (ULONG_PTR) WsleSwap.u1.VirtualAddress,
                              (ULONG_PTR) PointerPte->u.Long,
                              (ULONG_PTR) WorkingSetList);
#if (_MI_PAGING_LEVELS < 3)
            }
#endif
        }

        ASSERT (PointerPte->u.Hard.Valid == 1);

        if (WsleSwap.u1.e1.Direct) {
            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
            ASSERT (Pfn1->u1.WsIndex == SwapEntry);
            Pfn1->u1.WsIndex = Entry;
        }
        else {

            //
            // Update hash table.
            //

            if (Table != NULL) {
                MiRepointWsleHashIndex (WsleSwap, WorkingSetList, Entry);
            }
        }

        MI_SET_PTE_IN_WORKING_SET (PointerPte, Entry);

        //
        // Put entry on free list.
        //

        ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
                (WorkingSetList->FirstFree == WSLE_NULL_INDEX));

        Wsle[SwapEntry].u1.Long = WorkingSetList->FirstFree << MM_FREE_WSLE_SHIFT;
        WorkingSetList->FirstFree = SwapEntry;
        ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
            (WorkingSetList->FirstFree == WSLE_NULL_INDEX));

    }
    else {

        //
        // Both entries are valid.
        //

        MI_LOG_WSLE_CHANGE (WorkingSetList, SwapEntry, WsleEntry);
        Wsle[SwapEntry] = WsleEntry;

        PointerPte = MiGetPteAddress (WsleEntry.u1.VirtualAddress);

        if (PointerPte->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
            if (!NT_SUCCESS (MiCheckPdeForPagedPool (WsleEntry.u1.VirtualAddress))) {
#endif
                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x4128A,
                              (ULONG_PTR) WsleEntry.u1.VirtualAddress,
                              (ULONG_PTR) PointerPte->u.Long,
                              (ULONG_PTR) WorkingSetList);
#if (_MI_PAGING_LEVELS < 3)
              }
#endif
        }

        ASSERT (PointerPte->u.Hard.Valid == 1);

        if (WsleEntry.u1.e1.Direct) {

            //
            // Swap the PFN WsIndex element to point to the new slot.
            //

            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
            ASSERT (Pfn1->u1.WsIndex == Entry);
            Pfn1->u1.WsIndex = SwapEntry;
        }
        else if (Table != NULL) {

            //
            // Update hash table.
            //

            if (EntryNotInHash == TRUE) {
#if DBG
                WSLE_NUMBER Hash;
                PVOID VirtualAddress;
                    
                VirtualAddress = MI_GENERATE_VALID_WSLE (&WsleEntry);

                for (Hash = 0; Hash < WorkingSetList->HashTableSize; Hash += 1) {
                    ASSERT (Table[Hash].Key != VirtualAddress);
                }
#endif
            }
            else {
                MiRepointWsleHashIndex (WsleEntry, WorkingSetList, SwapEntry);
            }
        }

        MI_SET_PTE_IN_WORKING_SET (PointerPte, SwapEntry);

        MI_LOG_WSLE_CHANGE (WorkingSetList, Entry, WsleSwap);
        Wsle[Entry] = WsleSwap;

        PointerPte = MiGetPteAddress (WsleSwap.u1.VirtualAddress);

        if (PointerPte->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
            if (!NT_SUCCESS (MiCheckPdeForPagedPool (WsleSwap.u1.VirtualAddress))) {
#endif
                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x4128B,
                              (ULONG_PTR) WsleSwap.u1.VirtualAddress,
                              (ULONG_PTR) PointerPte->u.Long,
                              (ULONG_PTR) WorkingSetList);
#if (_MI_PAGING_LEVELS < 3)
              }
#endif
        }

        ASSERT (PointerPte->u.Hard.Valid == 1);

        if (WsleSwap.u1.e1.Direct) {

            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
            ASSERT (Pfn1->u1.WsIndex == SwapEntry);
            Pfn1->u1.WsIndex = Entry;
        }
        else {
            if (Table != NULL) {
                MiRepointWsleHashIndex (WsleSwap, WorkingSetList, Entry);
            }
        }
        MI_SET_PTE_IN_WORKING_SET (PointerPte, Entry);
    }

    return;
}


VOID
FASTCALL
MiTerminateWsle (
    IN PVOID VirtualAddress,
    IN PMMSUPPORT WsInfo,
    IN WSLE_NUMBER WsPfnIndex
    )

/*++

Routine Description:

    This function locates the specified virtual address within the
    working set list and then removes it, performing any necessary structure
    updates.

Arguments:

    VirtualAddress - Supplies the virtual address to locate within the working
                     set list.

    WsInfo - Supplies the working set structure to search.

    WsPfnIndex - Supplies a hint to try before hashing or walking linearly.

Return Value:

    None.

Environment:

    Kernel mode, APCs disabled, Working Set Mutex held.

--*/

{
    MMWSLE WsleContents;
    PMMWSL WorkingSetList;
    WSLE_NUMBER WorkingSetIndex;

    WorkingSetList = WsInfo->VmWorkingSetList;

    WorkingSetIndex = MiLocateWsle (VirtualAddress,
                                    WorkingSetList,
                                    WsPfnIndex,
                                    TRUE);

    ASSERT (WorkingSetIndex != WSLE_NULL_INDEX);

    WsleContents.u1.Long = WorkingSetList->Wsle[WorkingSetIndex].u1.Long;

    ASSERT (PAGE_ALIGN (VirtualAddress) == PAGE_ALIGN (WsleContents.u1.VirtualAddress));
    ASSERT (WsleContents.u1.e1.Valid == 1);

    MiRemoveWsle (WorkingSetIndex, WorkingSetList);

    //
    // Add this entry to the list of free working set entries
    // and adjust the working set count.
    //

    MiReleaseWsle (WorkingSetIndex, WsInfo);

    //
    // If the entry was locked in the working set, then adjust the locked
    // list so it is contiguous by reusing this entry.
    //

    if ((WsleContents.u1.e1.LockedInWs == 1) ||
        (WsleContents.u1.e1.LockedInMemory == 1)) {

        //
        // This entry is locked.
        //

        ASSERT (WorkingSetIndex < WorkingSetList->FirstDynamic);
        WorkingSetList->FirstDynamic -= 1;

        if (WorkingSetIndex != WorkingSetList->FirstDynamic) {

            ASSERT (WorkingSetList->Wsle[WorkingSetList->FirstDynamic].u1.e1.Valid);

            MiSwapWslEntries (WorkingSetList->FirstDynamic,
                              WorkingSetIndex,
                              WsInfo,
                              FALSE);
        }
    }
    else {
        ASSERT (WorkingSetIndex >= WorkingSetList->FirstDynamic);
    }

    return;
}

VOID
MiRepointWsleHashIndex (
    IN MMWSLE WsleEntry,
    IN PMMWSL WorkingSetList,
    IN WSLE_NUMBER NewWsIndex
    )

/*++

Routine Description:

    This routine re-points the working set list hash entry for the supplied
    address so it points at the new working set index.

Arguments:

    WsleEntry - Supplies the virtual address to look up.

    WorkingSetList - Supplies the working set list to operate on.

    NewWsIndex - Supplies the new working set list index to use.

Return Value:

    None.

Environment:

    Kernel mode, Working set mutex held.

--*/

{
    WSLE_NUMBER Hash;
    WSLE_NUMBER StartHash;
    PVOID VirtualAddress;
    PMMWSLE_HASH Table;
    ULONG Tries;
    
    if (WsleEntry.u1.e1.Hashed == 0) {
#if DBG
        if (WorkingSetList->HashTable != NULL) {

            VirtualAddress = MI_GENERATE_VALID_WSLE (&WsleEntry);

            Table = WorkingSetList->HashTable;
            for (Hash = 0; Hash < WorkingSetList->HashTableSize; Hash += 1) {
                ASSERT (Table[Hash].Key != VirtualAddress);
            }
        }
#endif
        return;
    }

    Tries = 0;
    Table = WorkingSetList->HashTable;

    VirtualAddress = MI_GENERATE_VALID_WSLE (&WsleEntry);

    Hash = MI_WSLE_HASH (WsleEntry.u1.VirtualAddress, WorkingSetList);
    StartHash = Hash;

    while (Table[Hash].Key != VirtualAddress) {

        Hash += 1;

        if (Hash >= WorkingSetList->HashTableSize) {
            ASSERT (Tries == 0);
            Hash = 0;
            Tries = 1;
        }

        if (StartHash == Hash) {

            //
            // Didn't find the hash entry, so this virtual address must
            // not have one.  That's ok, just return as nothing needs to
            // be done in this case.
            //

            return;
        }
    }

    Table[Hash].Index = NewWsIndex;

    return;
}

VOID
MiRemoveWsleFromFreeList (
    IN WSLE_NUMBER Entry,
    IN PMMWSLE Wsle,
    IN PMMWSL WorkingSetList
    )

/*++

Routine Description:

    This routine removes a working set list entry from the free list.
    It is used when the entry required is not the first element
    in the free list.

Arguments:

    Entry - Supplies the index of the entry to remove.

    Wsle - Supplies a pointer to the array of WSLEs.

    WorkingSetList - Supplies a pointer to the working set list.

Return Value:

    None.

Environment:

    Kernel mode, Working set lock and PFN lock held, APCs disabled.

--*/

{
    WSLE_NUMBER Free;
    WSLE_NUMBER ParentFree;

    Free = WorkingSetList->FirstFree;

    if (Entry == Free) {

        ASSERT (((Wsle[Entry].u1.Long >> MM_FREE_WSLE_SHIFT) <= WorkingSetList->LastInitializedWsle) ||
                ((Wsle[Entry].u1.Long >> MM_FREE_WSLE_SHIFT) == WSLE_NULL_INDEX));

        WorkingSetList->FirstFree = (WSLE_NUMBER)(Wsle[Entry].u1.Long >> MM_FREE_WSLE_SHIFT);

    }
    else {

        //
        // See if the entry is conveniently pointed to by the previous or
        // next entry to avoid walking the singly linked list when possible.
        //

        ParentFree = (WSLE_NUMBER)-1;

        if ((Entry != 0) && (Wsle[Entry - 1].u1.e1.Valid == 0)) {
            if ((Wsle[Entry - 1].u1.Long >> MM_FREE_WSLE_SHIFT) == Entry) {
                ParentFree = Entry - 1;
            }
        }
        else if ((Entry != WorkingSetList->LastInitializedWsle) && (Wsle[Entry + 1].u1.e1.Valid == 0)) {
            if ((Wsle[Entry + 1].u1.Long >> MM_FREE_WSLE_SHIFT) == Entry) {
                ParentFree = Entry + 1;
            }
        }

        if (ParentFree == (WSLE_NUMBER)-1) {
            do {
                ParentFree = Free;
                ASSERT (Wsle[Free].u1.e1.Valid == 0);
                Free = (WSLE_NUMBER)(Wsle[Free].u1.Long >> MM_FREE_WSLE_SHIFT);
            } while (Free != Entry);
        }

        Wsle[ParentFree].u1.Long = Wsle[Entry].u1.Long;
    }
    ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
            (WorkingSetList->FirstFree == WSLE_NULL_INDEX));
    return;
}

