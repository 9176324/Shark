/******************************Module*Header**********************************\
*
*                           *******************
*                           * D3D SAMPLE CODE *
*                           *******************
*
* Module Name: d3dstrct.h
*
* Content: Internal D3D structure management headers and macros
*
* Copyright (c) 1994-1999 3Dlabs Inc. Ltd. All rights reserved.
* Copyright (c) 1995-2002 Microsoft Corporation.  All rights reserved.
\*****************************************************************************/
#ifndef __D3DSTRCT_H
#define __D3DSTRCT_H

//-----------------------------------------------------------------------------
//
// Array functions and structures
//
//-----------------------------------------------------------------------------

typedef void (*PA_DestroyCB)(struct tagPointerArray* pTable, 
                             void* pData, 
                             void* pExtra);

typedef struct tagPointerArray
{
    ULONG_PTR* pPointers;
    DWORD dwNumPointers;
    PA_DestroyCB pfnDestroyCallback;
} PointerArray;

PointerArray* PA_CreateArray();
BOOL PA_DestroyArray(PointerArray* pArray, VOID* pExtra);
void* PA_GetEntry(PointerArray* pArray, DWORD dwNum);
BOOL PA_SetEntry(PointerArray* pArray, DWORD dwNum, void* pData);
void PA_SetDataDestroyCallback(PointerArray* pArray,
                               PA_DestroyCB DestroyCallback);

//-----------------------------------------------------------------------------
//
// Hashing functions and structures
//
//-----------------------------------------------------------------------------

#define HASH_SIZE 4096      // this many entries in the hash table

#define HT_HASH_OF(i)    ((i) & 0xFFF)

typedef struct tagHashSlot
{
    void* pData;
    ULONG_PTR dwHandle;

    struct tagHashSlot* pNext;
    struct tagHashSlot* pPrev;

} HashSlot;

typedef void (*DataDestroyCB)(struct tagHashTable* pTable, 
                              void* pData, 
                              void*pExtra);

typedef struct tagHashTable
{
    HashSlot* Slots[HASH_SIZE];
    DataDestroyCB pfnDestroyCallback;
} HashTable;

// Helper functions
static __inline HashSlot* HT_GetSlotFromHandle(HashTable* pTable, 
                                               ULONG_PTR dwHandle)
{
    HashSlot* pBase = NULL; 
    
    ASSERTDD(pTable != NULL,"ERROR: HashTable passed in is not valid!");

    pBase = pTable->Slots[HT_HASH_OF(dwHandle)];

    while (pBase != NULL)
    {
        if (pBase->dwHandle == dwHandle)
            return pBase;
        pBase = pBase->pNext;
    }

    return NULL;
} // HT_GetSlotFromHandle

static __inline void* HT_GetEntry(HashTable* pTable, ULONG_PTR dwHandle)
{
    HashSlot* pEntry = HT_GetSlotFromHandle(pTable, dwHandle);

    if (pEntry)
    {
        return pEntry->pData;
    }
    return NULL;
} /// HT_GetEntry


// Public interfaces
HashTable* HT_CreateHashTable();
void HT_ClearEntriesHashTable(HashTable* pHashTable, VOID* pExtra);
void HT_DestroyHashTable(HashTable* pHashTable, VOID* pExtra);
void HT_SetDataDestroyCallback(HashTable* pTable, 
                               DataDestroyCB DestroyCallback);
BOOL HT_SwapEntries(HashTable* pTable, DWORD dwHandle1, DWORD dwHandle2);
BOOL HT_AddEntry(HashTable* pTable, ULONG_PTR dwHandle, void* pData);
BOOL HT_RemoveEntry(HashTable* pTable, ULONG_PTR dwHandle, VOID* pExtra);

// Generic code to handle double-linked list from DDK
#ifndef FORCEINLINE
#if (_MSC_VER >= 1200)
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __inline
#endif
#endif

// LIST_ENTRY is defined in winnt.h

VOID
FORCEINLINE
InitializeListHead(
    IN PLIST_ENTRY ListHead
    )
{
    ListHead->Flink = ListHead->Blink = ListHead;
}

//
//  BOOLEAN
//  IsListEmpty(
//      PLIST_ENTRY ListHead
//      );
//

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

VOID
FORCEINLINE
RemoveEntryList(
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
}

PLIST_ENTRY
FORCEINLINE
RemoveHeadList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;
    return Entry;
}

PLIST_ENTRY
FORCEINLINE
RemoveTailList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Blink;
    Blink = Entry->Blink;
    ListHead->Blink = Blink;
    Blink->Flink = ListHead;
    return Entry;
}

VOID
FORCEINLINE
InsertTailList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;

    Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
}

VOID
FORCEINLINE
InsertHeadList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}

//
// Calculate the address of the base of the structure given its type, and an
// address of a field within the structure.
//

#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (PCHAR)(address) - \
                                                  (ULONG_PTR)(&((type *)0)->field)))

#endif // __D3DSTRCT_H



