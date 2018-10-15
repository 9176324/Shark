/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   CcPerf.c

Abstract:

    This module contains the perf trace routines in Cc Component

--*/

#include "cc.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEWMI, CcPerfFileRunDown)
#endif //ALLOC_PRAGMA

VOID
CcPerfFileRunDown(
    PPERFINFO_ENTRY_TABLE HashTable
    )
/*++

Routine Description:

    This routine walks the following lists:

    1. CcDirtySharedCacheMapList
    2. CcCleanSharedCacheMapList
    
    and returns a pointer to a pool allocation
    containing the referenced file object pointers.

Arguments:
 
    None.
 
Return Value:
 
    Returns a pointer to a NULL terminated pool allocation 
    containing the file object pointers from the two lists, 
    NULL if the memory could not be allocated.
     
    It is also the responsibility of the caller to dereference each
    file object in the list and then free the returned pool.

Environment:

    PASSIVE_LEVEL, arbitrary thread context.
--*/
{
    KIRQL OldIrql;
    PSHARED_CACHE_MAP SharedCacheMap;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    CcAcquireMasterLock( &OldIrql );

    //
    // Walk through CcDirtySharedCacheMapList
    //
    
    SharedCacheMap = CONTAINING_RECORD( CcDirtySharedCacheMapList.SharedCacheMapLinks.Flink,
                                        SHARED_CACHE_MAP,
                                        SharedCacheMapLinks );
    
    while (&SharedCacheMap->SharedCacheMapLinks != &CcDirtySharedCacheMapList.SharedCacheMapLinks) {
        //
        //  Skip over cursors
        //
        if (!FlagOn(SharedCacheMap->Flags, IS_CURSOR)) {
            PerfInfoAddToFileHash(HashTable, SharedCacheMap->FileObject);
        }
        SharedCacheMap = CONTAINING_RECORD( SharedCacheMap->SharedCacheMapLinks.Flink,
                                            SHARED_CACHE_MAP,
                                            SharedCacheMapLinks );
    }                   

    //
    // CcCleanSharedCacheMapList
    //
    SharedCacheMap = CONTAINING_RECORD( CcCleanSharedCacheMapList.Flink,
                                        SHARED_CACHE_MAP,
                                        SharedCacheMapLinks );

    while (&SharedCacheMap->SharedCacheMapLinks != &CcCleanSharedCacheMapList) {
        PerfInfoAddToFileHash(HashTable, SharedCacheMap->FileObject);

        SharedCacheMap = CONTAINING_RECORD( SharedCacheMap->SharedCacheMapLinks.Flink,
                                            SHARED_CACHE_MAP,
                                            SharedCacheMapLinks );

    }

    CcReleaseMasterLock( OldIrql );
    return;
}
