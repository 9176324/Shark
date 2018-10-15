/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmchek2.c

Abstract:

    This module implements consistency checking for the registry.

--*/

#include    "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpValidateHiveSecurityDescriptors)
#endif

extern ULONG   CmpUsedStorage;

BOOLEAN
CmpValidateHiveSecurityDescriptors(
    IN PHHIVE       Hive,
    OUT PBOOLEAN    ResetSD
    )
/*++

Routine Description:

    Walks the list of security descriptors present in the hive and passes
    each security descriptor to RtlValidSecurityDescriptor.

    Only applies to descriptors in Stable store.  Those in Volatile store
    cannot have come from disk and therefore do not need this treatment
    anyway.

Arguments:

    Hive - Supplies pointer to the hive control structure

Return Value:

    TRUE  - All security descriptors are valid
    FALSE - At least one security descriptor is invalid

--*/

{
    PCM_KEY_NODE        RootNode;
    PCM_KEY_SECURITY    SecurityCell;
    HCELL_INDEX         ListAnchor;
    HCELL_INDEX         NextCell;
    HCELL_INDEX         LastCell;
    BOOLEAN             BuildSecurityCache;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpValidateHiveSecurityDescriptor: Hive = %p\n",(ULONG_PTR)Hive));

    ASSERT( Hive->ReleaseCellRoutine == NULL );

    *ResetSD = FALSE;

    if( ((PCMHIVE)Hive)->SecurityCount == 0 ) {
        BuildSecurityCache = TRUE;
    } else {
        BuildSecurityCache = FALSE;
    }
    if (!HvIsCellAllocated(Hive,Hive->BaseBlock->RootCell)) {
        //
        // root cell HCELL_INDEX is bogus
        //
        return(FALSE);
    }
    RootNode = (PCM_KEY_NODE) HvGetCell(Hive, Hive->BaseBlock->RootCell);
    if( RootNode == NULL ) {
        //
        // we couldn't map a view for the bin containing this cell
        //
        return FALSE;
    }
    
    if( FALSE ) {
YankSD:
        if( CmDoSelfHeal() ) {
            //
            // reset all security for the entire hive to the root security. There is no reliable way to 
            // patch the security list
            //
            if(!HvIsCellAllocated(Hive, RootNode->Security)) {
                return FALSE;
            }
            SecurityCell = (PCM_KEY_SECURITY) HvGetCell(Hive, RootNode->Security);
            if( SecurityCell == NULL ) {
                return FALSE;
            }

            if( HvMarkCellDirty(Hive, RootNode->Security,FALSE) ) {
                SecurityCell->Flink = SecurityCell->Blink = RootNode->Security;
            } else {
                return FALSE;
            }
            //
            // destroy existing cache and set up an empty one
            //
            CmpDestroySecurityCache((PCMHIVE)Hive);
            CmpInitSecurityCache((PCMHIVE)Hive);
            CmMarkSelfHeal(Hive);
            *ResetSD = TRUE;
        } else {
            return FALSE;
        }

    }

    LastCell = 0;
    ListAnchor = NextCell = RootNode->Security;

    do {
        if (!HvIsCellAllocated(Hive, NextCell)) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CM: CmpValidateHiveSecurityDescriptors\n"));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"    NextCell: %08lx is invalid HCELL_INDEX\n",NextCell));
            goto YankSD;
        }
        SecurityCell = (PCM_KEY_SECURITY) HvGetCell(Hive, NextCell);
        if( SecurityCell == NULL ) {
            //
            // we couldn't map a view for the bin containing this cell
            //
            return FALSE;
        }

        if (NextCell != ListAnchor) {
            //
            // Check to make sure that our Blink points to where we just
            // came from.
            //
            if (SecurityCell->Blink != LastCell) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"  Invalid Blink (%08lx) on security cell %08lx\n",SecurityCell->Blink, NextCell));
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"  should point to %08lx\n", LastCell));
                return(FALSE);
            }
        }

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpValidSD:  SD shared by %d nodes\n",SecurityCell->ReferenceCount));
        if (!SeValidSecurityDescriptor(SecurityCell->DescriptorLength, &SecurityCell->Descriptor)) {
#if DBG
            CmpDumpSecurityDescriptor(&SecurityCell->Descriptor,"INVALID DESCRIPTOR");
#endif
            goto YankSD;
        }
        //
        // cache this security cell; now that we know it is valid
        //
        if( BuildSecurityCache == TRUE ) {
            if( !NT_SUCCESS(CmpAddSecurityCellToCache ( (PCMHIVE)Hive,NextCell,TRUE,NULL) ) ) {
                return FALSE;
            }
        } else {
            //
            // just check this cell is there
            //
            ULONG Index;
            if( CmpFindSecurityCellCacheIndex ((PCMHIVE)Hive,NextCell,&Index) == FALSE ) {
                //
                // bad things happened; maybe an error in our caching code?
                //
                return FALSE;
            }

        }

        LastCell = NextCell;
        NextCell = SecurityCell->Flink;
    } while ( NextCell != ListAnchor );

    if( BuildSecurityCache == TRUE ) {
        //
        // adjust the size of the cache in case we allocated too much
        //
        CmpAdjustSecurityCacheSize ( (PCMHIVE)Hive );
    }

    return(TRUE);
}
