/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmparse.c

Abstract:

    This module contains parse routines for the configuration manager, particularly
    the registry.

--*/

#include    "cmp.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif
const ULONG CmpCacheOnFlag = CM_CACHE_FAKE_KEY;

extern  BOOLEAN CmpNoMasterCreates;
extern  PCM_KEY_CONTROL_BLOCK CmpKeyControlBlockRoot;
extern  UNICODE_STRING CmSymbolicLinkValueName;

#define CM_HASH_STACK_SIZE  32
#define MAX_LOCAL_KCB_ARRAY (CM_HASH_STACK_SIZE + 1) // need to account for RootObject as well

typedef struct _CM_HASH_ENTRY {
    ULONG ConvKey;
    UNICODE_STRING KeyName;
} CM_HASH_ENTRY, *PCM_HASH_ENTRY;

BOOLEAN
CmpGetHiveName(
    PCMHIVE         CmHive,
    PUNICODE_STRING HiveName
    );

ULONG
CmpComputeHashValue(
    IN PCM_HASH_ENTRY  HashStack,
    IN OUT ULONG  *TotalSubkeys,
    IN ULONG BaseConvKey,
    IN PUNICODE_STRING RemainingName
    );

NTSTATUS
CmpCacheLookup(
    IN PCM_HASH_ENTRY HashStack,
    IN ULONG TotalRemainingSubkeys,
    OUT ULONG *MatchRemainSubkeyLevel,
    IN OUT PCM_KEY_CONTROL_BLOCK *Kcb,
    OUT PUNICODE_STRING RemainingName,
    OUT PHHIVE *Hive,
    OUT HCELL_INDEX *Cell,
    IN PULONG   OuterStackArray
    );

VOID
CmpCacheAdd(
    IN PCM_HASH_ENTRY LastHashEntry,
    IN ULONG Count
    );

PCM_KEY_CONTROL_BLOCK
CmpAddInfoAfterParseFailure(
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    PCM_KEY_NODE    Node,
    PCM_KEY_CONTROL_BLOCK kcb,
    PUNICODE_STRING NodeName
    );

//
// Prototypes for procedures private to this file
//

BOOLEAN
CmpGetSymbolicLink(
    IN PHHIVE Hive,
    IN OUT PUNICODE_STRING ObjectName,
    IN OUT PCM_KEY_CONTROL_BLOCK SymbolicKcb,
    IN PUNICODE_STRING RemainingName
    );

NTSTATUS
CmpDoOpen(
    IN PHHIVE                       Hive,
    IN HCELL_INDEX                  Cell,
    IN PCM_KEY_NODE                 Node,
    IN PACCESS_STATE                AccessState,
    IN KPROCESSOR_MODE              AccessMode,
    IN ULONG                        Attributes,
    IN PCM_PARSE_CONTEXT            Context,
    IN ULONG                        ControlFlags,
    IN OUT PCM_KEY_CONTROL_BLOCK    *CachedKcb,
    IN PUNICODE_STRING              KeyName,
    IN PCMHIVE                      OriginatingHive OPTIONAL,
    IN PULONG                       KcbsLocked,
    OUT PVOID                       *Object,
    OUT PBOOLEAN                    NeedDeref OPTIONAL
    );


NTSTATUS
CmpCreateLinkNode(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell,
    IN PACCESS_STATE AccessState,
    IN UNICODE_STRING Name,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN PCM_PARSE_CONTEXT Context,
    IN PCM_KEY_CONTROL_BLOCK ParentKcb,
    IN PULONG LockedKcbs,
    OUT PVOID *Object
    );

BOOLEAN
CmpOKToFollowLink(  IN PCMHIVE  OrigHive,
                    IN PCMHIVE  DestHive
                    );


NTSTATUS
CmpBuildHashStackAndLookupCache( PCM_KEY_BODY           ParseObject,
                                 PCM_KEY_CONTROL_BLOCK  *kcb,
                                 PUNICODE_STRING        Current,
                                 PHHIVE                 *Hive,
                                 HCELL_INDEX            *Cell,
                                 PULONG                 TotalRemainingSubkeys,
                                 PULONG                 MatchRemainSubkeyLevel,
                                 PULONG                 TotalSubkeys,
                                 PULONG                 OuterStackArray,
                                 PULONG                 *LockedKcbs
                                );

PULONG 
CmpBuildAndLockKcbArray(   
    IN PCM_HASH_ENTRY HashStack,
    IN ULONG TotalLevels,
    IN ULONG RemainingLevel,
    IN PCM_KEY_CONTROL_BLOCK    kcb,
    IN PULONG                   OuterStackArray,
    IN BOOLEAN LockExclusive);

ULONG
CmpUnLockKcbArray(IN PULONG LockedKcbs,
                  IN ULONG  Exempt);

VOID
CmpReLockKcbArray(IN PULONG LockedKcbs,
                  IN BOOLEAN LockExclusive);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpBuildHashStackAndLookupCache)
#pragma alloc_text(PAGE,CmpParseKey)
#pragma alloc_text(PAGE,CmpGetNextName)
#pragma alloc_text(PAGE,CmpDoOpen)
#pragma alloc_text(PAGE,CmpCreateLinkNode)
#pragma alloc_text(PAGE,CmpGetSymbolicLink)
#pragma alloc_text(PAGE,CmpComputeHashValue)
#pragma alloc_text(PAGE,CmpCacheLookup)
#pragma alloc_text(PAGE,CmpAddInfoAfterParseFailure)
#pragma alloc_text(PAGE,CmpOKToFollowLink)
#pragma alloc_text(PAGE,CmpBuildAndLockKcbArray)
#pragma alloc_text(PAGE,CmpUnLockKcbArray)
#pragma alloc_text(PAGE,CmpReLockKcbArray)
#endif
    
/* macro
VOID
CmpStepThroughExit(
    IN OUT PHHIVE       *Hive,
    IN OUT HCELL_INDEX  *Cell,
    IN OUT PCM_KEY_NODE *pNode
    )
*/
#define CmpStepThroughExit(h,c,n,ReleaseHive,ReleaseCell)           \
if ((n)->Flags & KEY_HIVE_EXIT) {                                   \
    if( ReleaseCell != HCELL_NIL ) {                                \
        ASSERT( ReleaseHive != NULL );                              \
        HvReleaseCell( ReleaseHive,ReleaseCell);                    \
    }                                                               \
    (h)=(n)->ChildHiveReference.KeyHive;                            \
    (c)=(n)->ChildHiveReference.KeyCell;                            \
    (n)=(PCM_KEY_NODE)HvGetCell((h),(c));                           \
    if( (n) == NULL ) {                                             \
        ReleaseHive = NULL;                                         \
        ReleaseCell = HCELL_NIL;                                    \
    } else {                                                        \
        ReleaseHive = (h);                                          \
        ReleaseCell = (c);                                          \
    }                                                               \
}

#define CmpReleasePreviousAndHookNew(NewHive,NewCell,ReleaseHive,ReleaseCell)   \
    if( ReleaseCell != HCELL_NIL ) {                                            \
        ASSERT( ReleaseHive != NULL );                                          \
        HvReleaseCell( ReleaseHive,ReleaseCell);                                \
    }                                                                           \
    ReleaseHive = (NewHive);                                                    \
    ReleaseCell = (NewCell)                                                    

#define CMP_PARSE_GOTO_NONE     0
#define CMP_PARSE_GOTO_CREATE   1
#define CMP_PARSE_GOTO_RETURN   2
#define CMP_PARSE_GOTO_RETURN2  3

NTSTATUS
CmpBuildHashStackAndLookupCache( PCM_KEY_BODY           ParseObject,
                                 PCM_KEY_CONTROL_BLOCK  *kcb,
                                 PUNICODE_STRING        Current,
                                 PHHIVE                 *Hive,
                                 HCELL_INDEX            *Cell,
                                 PULONG                 TotalRemainingSubkeys,
                                 PULONG                 MatchRemainSubkeyLevel,
                                 PULONG                 TotalSubkeys,
                                 PULONG                 OuterStackArray,
                                 PULONG                 *LockedKcbs
                                )
{
    CM_HASH_ENTRY   HashStack[CM_HASH_STACK_SIZE];
    ULONG           HashKeyCopy;
    BOOLEAN         RegLocked = FALSE;
    NTSTATUS        status;

    *LockedKcbs = NULL;
RetryHash:
    HashKeyCopy = (*kcb)->ConvKey;
    //
    // Compute the hash values of each subkeys
    //
    *TotalRemainingSubkeys = CmpComputeHashValue(HashStack,
                                                TotalSubkeys,
                                                HashKeyCopy,
                                                Current);
    //
    // we now lock it shared as 85% of the create calls are in fact opens
    // the lock will be acquired exclusively in CmpDoCreate/CmpCreateLinkNode
    //
    // We only lock the registry here, in the parse routine to reduce contention 
    // on the registry lock (NO reason to wait on OB)
    //

    if( !RegLocked ) {
        CmpLockRegistry();
        RegLocked = TRUE;
    }

    //
    // we can't go deeper than what our stack buffer allows us to at one iteration.
    //
    if( *TotalSubkeys > CM_HASH_STACK_SIZE ) {
        return STATUS_NAME_TOO_LONG;
    }

    if( (*kcb)->ConvKey != HashKeyCopy ) {
        goto RetryHash;
    }
    //
    // Check to make sure the passed in root key is not marked for deletion.
    //
    if (((PCM_KEY_BODY)ParseObject)->KeyControlBlock->Delete == TRUE) {
        ASSERT( RegLocked );
        return STATUS_KEY_DELETED;
    }

    //
    // Fetch the starting Hive.Cell.  Because of the way the parse
    // paths work, this will always be defined.  (ObOpenObjectByName
    // had to bounce off of a KeyObject or KeyRootObject to get here)
    //
    *Hive = (*kcb)->KeyHive;
    *Cell = (*kcb)->KeyCell;

    // Look up from the cache.  kcb will be changed if we find a partial or exact match
    // PCmpCacheEntry, the entry found, will be moved to the front of
    // the Cache.
    status = CmpCacheLookup(HashStack,
                          *TotalRemainingSubkeys,
                          MatchRemainSubkeyLevel,
                          kcb,
                          Current,
                          Hive,
                          Cell,
                          OuterStackArray);
    //
    // The RefCount of kcb was increased in the CmpCacheLookup process,
    // It is to protect it from being kicked out of cache.
    // Make sure we dereference it after we are done.
    //
    //
    // lock in advance all kcbs we might be touching so key_nodes don't get away from under us.
    //
    if (NT_SUCCESS (status)) {
        if( ((*TotalRemainingSubkeys) == (*MatchRemainSubkeyLevel)) && (OuterStackArray[0] == 1) ) {
            //
            // fast path; we're lucky and have a hit to an already opened kcb
            // we can (try) to do a lock free open.
            //
            ASSERT( OuterStackArray );
            ASSERT( OuterStackArray[1] == GET_HASH_INDEX((*kcb)->ConvKey) );
            ASSERT_KCB_LOCKED(*kcb);
            *LockedKcbs = OuterStackArray;
        } else {
            *LockedKcbs = CmpBuildAndLockKcbArray(   HashStack,
                                                    *TotalRemainingSubkeys,
                                                    *MatchRemainSubkeyLevel,
                                                    *kcb,
                                                    OuterStackArray,
                                                    TRUE
                                                );
            if( *LockedKcbs == NULL ) {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }
    
    return status;
}

C_ASSERT( sizeof(REG_CREATE_KEY_INFORMATION) >= sizeof(REG_OPEN_KEY_INFORMATION) );
C_ASSERT( FIELD_OFFSET(REG_CREATE_KEY_INFORMATION, CompleteName) == FIELD_OFFSET(REG_OPEN_KEY_INFORMATION, CompleteName) );
C_ASSERT( FIELD_OFFSET(REG_CREATE_KEY_INFORMATION, RootObject) == FIELD_OFFSET(REG_OPEN_KEY_INFORMATION, RootObject) );

NTSTATUS
CmpParseKey(
    IN PVOID ParseObject,
    IN PVOID ObjectType,
    IN OUT PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN OUT PUNICODE_STRING CompleteName,
    IN OUT PUNICODE_STRING RemainingName,
    IN OUT PVOID Context OPTIONAL,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
    OUT PVOID *Object
    )
/*++

Routine Description:

    This routine interfaces to the NT Object Manager.  It is invoked when
    the object system is given the name of an entity to create or open and
    a Key or KeyRoot is encountered in the path.  In practice this means
    that this routine is called for all objects whose names are of the
    form \REGISTRY\...

    This routine will create a Key object, which is effectively an open
    instance to a registry key node, and return its address
    (for the success case.)

Arguments:

    ParseObject - Pointer to a KeyRoot or Key, thus -> KEY_BODY.

    ObjectType - Type of the object being opened.

    AccessState - Running security access state information for operation.

    AccessMode - Access mode of the original caller.

    Attributes - Attributes to be applied to the object.

    CompleteName - Supplies complete name of the object.

    RemainingName - Remaining name of the object.

    Context - if create or hive root open, points to a CM_PARSE_CONTEXT
              structure,
              if open, is NULL.

    SecurityQos - Optional security quality of service indicator.

    Object - The address of a variable to receive the created key object, if
        any.

Return Value:

    The function return value is one of the following:

        a)  Success - This indicates that the function succeeded and the object
            parameter contains the address of the created key object.

        b)  STATUS_REPARSE - This indicates that a symbolic link key was
            found, and the path should be reparsed.

        c)  Error - This indicates that the file was not found or created and
            no file object was created.

--*/
{
    NTSTATUS                status = STATUS_UNSUCCESSFUL;
    BOOLEAN                 rc;
    PHHIVE                  Hive = NULL;
    PCM_KEY_NODE            Node = NULL;
    HCELL_INDEX             Cell = HCELL_NIL;
    HCELL_INDEX             NextCell;
    PCM_PARSE_CONTEXT       lcontext;
    UNICODE_STRING          Current;
    UNICODE_STRING          NextName = {0}; // Component last returned by CmpGetNextName,
                                        // will always be behind Current.
    
    BOOLEAN                 Last;       // TRUE if component NextName points to
                                        // is the last one in the path.

    ULONG           TotalRemainingSubkeys;
    ULONG           MatchRemainSubkeyLevel = 0;
    ULONG           TotalSubkeys=0;
    PCM_KEY_CONTROL_BLOCK   kcb;
    PCM_KEY_CONTROL_BLOCK   ParentKcb = NULL;
    UNICODE_STRING          TmpNodeName;
    ULONG                   GoToValue = CMP_PARSE_GOTO_NONE;
    BOOLEAN                 CompleteKeyCached = FALSE;

    PHHIVE                  HiveToRelease = NULL;
    HCELL_INDEX             CellToRelease = HCELL_NIL;
    PULONG                  LockedKcbs = NULL;
    ULONG                   LocalFastKcbArray[MAX_LOCAL_KCB_ARRAY + 1]; // perf, avoid ExAllocatePool

    CM_PAGED_CODE();

    UNREFERENCED_PARAMETER (SecurityQos);

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpParseKey:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CompleteName = '%wZ'\n\t", CompleteName));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"RemainingName = '%wZ'\n", RemainingName));

    //
    // Strip off any trailing path separators
    //
    while ((RemainingName->Length > 0) &&
           (RemainingName->Buffer[(RemainingName->Length/sizeof(WCHAR)) - 1] == OBJ_NAME_PATH_SEPARATOR)) {
        RemainingName->Length -= sizeof(WCHAR);
    }

    Current = *RemainingName;
    if (ObjectType != CmpKeyObjectType) {
        return STATUS_OBJECT_TYPE_MISMATCH;
    }

    if( ARGUMENT_PRESENT(Context) && (((PCM_PARSE_CONTEXT)Context)->CreateOperation == TRUE) ) {
        lcontext = (PCM_PARSE_CONTEXT)Context;
    } else {
        //
        // keep the old behavior (open == parse without context)
        //
        lcontext = NULL;
    }

    //
    // PreCreate callback
    //
    if ( CmAreCallbacksRegistered() ) {
        REG_CREATE_KEY_INFORMATION  PreCreateInfo;
        PreCreateInfo.CompleteName = CompleteName;
        PreCreateInfo.RootObject = ParseObject;
        if( ARGUMENT_PRESENT(lcontext) ) {
            //
            // NtCreateKey
            //
            status = CmpCallCallBacks(RegNtPreCreateKeyEx,&PreCreateInfo,TRUE,RegNtPostCreateKeyEx,ParseObject);
        } else {
            //
            // NtOpenKey
            //
            status = CmpCallCallBacks(RegNtPreOpenKeyEx,(PREG_OPEN_KEY_INFORMATION)(&PreCreateInfo),TRUE,RegNtPostOpenKeyEx,ParseObject);
        }

        if( !NT_SUCCESS(status) ) {
            return status;
        }
    }
    
    BEGIN_LOCK_CHECKPOINT;

    kcb = ((PCM_KEY_BODY)ParseObject)->KeyControlBlock;

    status = CmpBuildHashStackAndLookupCache(ParseObject,
                                             &kcb,
                                             &Current,
                                             &Hive,
                                             &Cell,
                                             &TotalRemainingSubkeys,
                                             &MatchRemainSubkeyLevel,
                                             &TotalSubkeys,
                                             LocalFastKcbArray,
                                             &LockedKcbs);
    ASSERT_CM_LOCK_OWNED();
    //
    // First make sure it is OK to proceed.
    //
    if (!NT_SUCCESS (status)) {
        goto JustReturn;
    } 

    ParentKcb = kcb;

    if(TotalRemainingSubkeys == 0) {
        //
        // We REALLY don't want to mess with the cache code below
        // in this case (this could only happen if we called with 
        // the lpSubkey = NULL )
        //
        CompleteKeyCached = TRUE;
        goto Found;
    }


    //
    // First check if there are further information in the cached kcb.
    // 
    // The additional information can be
    // 1. This cached key is a fake key (CM_KCB_KEY_NON_EXIST), then either let it be created
    //    or return STATUS_OBJECT_NAME_NOT_FOUND.
    // 2. The cached key is not the destination and it has no subkey (CM_KCB_NO_SUBKEY).
    // 3. The cached key is not the destination and it has 
    //    the first four characters of its subkeys.  If the flag is CM_KCB_SUBKEY_ONE, there is only one subkey
    //    and the four char is embedded in the KCB.  If the flag is CM_KCB_SUBKEY_INFO, then there is
    //    an allocation for these info. 
    //
    // We do need to lock KCB tree to protect the KCB being modified.  Currently there is not lock contention problem
    // on KCBs, We can change KCB lock to a read-write lock if this becomes a problem.
    // We already have the lock on the kcb tree and we need it until we finish work on the cache table.  
    //
    if( kcb->Delete ) {
        //
        // kcb has been deleted while playing with the lock
        //
        status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto JustReturn;

    }

    if (kcb->ExtFlags & CM_KCB_CACHE_MASK) {
        if (MatchRemainSubkeyLevel == TotalRemainingSubkeys) {
            //
            // We have found a cache for the complete path,
            //
            if (kcb->ExtFlags & CM_KCB_KEY_NON_EXIST) {
                //
                // This key does not exist.
                //
                if (ARGUMENT_PRESENT(lcontext)) {
                    ULONG LevelToSkip = TotalRemainingSubkeys-1;
                    ULONG i=0;
                    
                    ParentKcb = kcb->ParentKcb;

                    ASSERT_KCB_LOCKED_EXCLUSIVE(kcb);
                    ASSERT_KCB_LOCKED_EXCLUSIVE(ParentKcb);
                    //
                    // The non-existing key is the destination key and lcontext is present.
                    // delete this fake kcb and let the real one be created.
                    //
                    // Temporarily increase the RefCount of the ParentKcb so it's 
                    // not removed while removing the fake and creating the real KCB.
                    //
                    
                    if (CmpReferenceKeyControlBlock(ParentKcb)) {
                    
                        kcb->Delete = TRUE;
                        CmpRemoveKeyControlBlock(kcb);
                        CmpDereferenceKeyControlBlockWithLock(kcb,FALSE);

                        //
                        // Update Hive, Cell and Node
                        //
                        Hive = ParentKcb->KeyHive;
                        Cell = ParentKcb->KeyCell;
                        Node = (PCM_KEY_NODE)HvGetCell(Hive,Cell);
                        if( Node == NULL ) {
                            //
                            // we couldn't map the bin containing this cell
                            //
                            status = STATUS_INSUFFICIENT_RESOURCES;
                            goto FreeAndReturn;
                        }
                    
                        CmpReleasePreviousAndHookNew(Hive,Cell,HiveToRelease,CellToRelease);

                        //
                        // Now get the child name to be created.
                        //
   
                        NextName = *RemainingName;
                        if ((NextName.Buffer == NULL) || (NextName.Length == 0)) {
                            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Something wrong in finding the child name\n"));
                            DbgBreakPoint();
                        }
                        //
                        // Skip over leading path separators
                        //
                        while (*(NextName.Buffer) == OBJ_NAME_PATH_SEPARATOR) {
                            NextName.Buffer++;
                            NextName.Length -= sizeof(WCHAR);
                            NextName.MaximumLength -= sizeof(WCHAR);
                        }
   
                        while (i < LevelToSkip) {
                            if (*(NextName.Buffer) == OBJ_NAME_PATH_SEPARATOR) {
                                i++;
                                while (*(NextName.Buffer) == OBJ_NAME_PATH_SEPARATOR) {
                                    NextName.Buffer++;
                                    NextName.Length -= sizeof(WCHAR);
                                    NextName.MaximumLength -= sizeof(WCHAR);
                                }
                            } else {
                                NextName.Buffer++;
                                NextName.Length -= sizeof(WCHAR);
                                NextName.MaximumLength -= sizeof(WCHAR);
                            }
                        } 
                        GoToValue = CMP_PARSE_GOTO_CREATE;
                    } else {
                        //
                        // We have maxed the RefCount of ParentKcb; treate it as key cannot be created.
                        // The ParentKcb will not be dereferenced at the end.
                        //
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        GoToValue = CMP_PARSE_GOTO_RETURN2;
                    }
                } else {
                    status = STATUS_OBJECT_NAME_NOT_FOUND;
                    GoToValue = CMP_PARSE_GOTO_RETURN;
                }
            }
        } else if (kcb->ExtFlags & CM_KCB_KEY_NON_EXIST) {
            //
            // one subkey (not destination) in the path does not exist. no point to continue.
            //
            status = STATUS_OBJECT_NAME_NOT_FOUND;
            GoToValue = CMP_PARSE_GOTO_RETURN;
        } else if (kcb->ExtFlags & CM_KCB_NO_SUBKEY) {
            //
            // one parent in the path has no subkey. see if it is a create.
            //
            if (((TotalRemainingSubkeys - MatchRemainSubkeyLevel) == 1) && (ARGUMENT_PRESENT(lcontext))) {
                //
                // Now we are going to create this subkey. 
                // The kcb cache will be updated in CmpDoCreate routine.
                //
            } else {
                status = STATUS_OBJECT_NAME_NOT_FOUND;
                GoToValue = CMP_PARSE_GOTO_RETURN;
            }
        } else {
            //
            // We have a partial match.  Current is the remaining name to be parsed.
            // The Key has either one or a few subkeys and has index hint. check if it is the candidate.
            //
           
            BOOLEAN NoMatch = TRUE;
            ULONG   NextHashKey;
            PULONG  TempHashKey;
            ULONG   HintCounts;
            ULONG   CmpCount;
            //
            // When NoMatch is TRUE, we know for sure there is no subkey that can match.
            // When NoMatch is FALSE, it can we either we found a match or
            // there is not enough information.  Either case, we need to continue
            // the parse.
            //

            TmpNodeName = Current;

            rc = CmpGetNextName(&TmpNodeName, &NextName, &Last);
        
            NextHashKey = CmpComputeHashKey(0,&NextName
#if DBG
                                            , FALSE
#endif
                );

            if (kcb->ExtFlags & CM_KCB_SUBKEY_ONE) {
                HintCounts = 1;
                TempHashKey = &(kcb->HashKey);
            } else {
                //
                // More than one child, the hint info in not inside the kcb but pointed by kcb.
                //
                HintCounts = kcb->IndexHint->Count;
                TempHashKey = &(kcb->IndexHint->HashKey[0]);
            }

            for (CmpCount=0; CmpCount<HintCounts; CmpCount++) {
                if( TempHashKey[CmpCount] == 0) {
                    //
                    // No hint available; assume the subkey exist and go on with the parse
                    //
                    NoMatch = FALSE;
                    break;
                } 
                
                if( NextHashKey == TempHashKey[CmpCount] ) {
                    //
                    // There is a match.
                    //
                    NoMatch = FALSE;
                    break;
                }
            }

            if (NoMatch) {
                if (((TotalRemainingSubkeys - MatchRemainSubkeyLevel) == 1) && (ARGUMENT_PRESENT(lcontext))) {
                    //
                    // No we are going to create this subkey. 
                    // The kcb cache will be updated in CmpDoCreate.
                    //
                } else {
                    status = STATUS_OBJECT_NAME_NOT_FOUND;
                    GoToValue = CMP_PARSE_GOTO_RETURN;
                }
            }
        }
    }

    if (GoToValue == CMP_PARSE_GOTO_CREATE) {
        goto CreateChild;
    } else if (GoToValue == CMP_PARSE_GOTO_RETURN) {
        goto FreeAndReturn;
    } else if (GoToValue == CMP_PARSE_GOTO_RETURN2) {
        goto JustReturn;
    }

    if (MatchRemainSubkeyLevel) {
        // Found something, update the information to start the search
        // from the new BaseName

        if (MatchRemainSubkeyLevel == TotalSubkeys) {
            // The complete key has been found in the cache,
            // go directly to the CmpDoOpen.
            
            //
            // Found the whole thing cached.
            // 
            //
            CompleteKeyCached = TRUE;
            goto Found;
        }
        ASSERT( (Cell == kcb->KeyCell) && (Hive == kcb->KeyHive) );
    }  

    //
    //  Check if we hit a symbolic link case
    //
    if (kcb->Flags & KEY_SYM_LINK) {
        //
        // The given key was a symbolic link.  Find the name of
        // its link, and return STATUS_REPARSE to the Object Manager.
        //
        rc = CmpGetNextName(&Current, &NextName, &Last);
        Current.Buffer = NextName.Buffer;
        if (Current.Length + NextName.Length > MAXUSHORT) {
            status = STATUS_NAME_TOO_LONG;
            goto FreeAndReturn;
        }
        Current.Length = (USHORT)(Current.Length + NextName.Length);

        if (Current.MaximumLength + NextName.MaximumLength > MAXUSHORT) {
            status = STATUS_NAME_TOO_LONG;
            goto FreeAndReturn;
        }
        Current.MaximumLength = (USHORT)(Current.MaximumLength + NextName.MaximumLength);
        //
        // need not to interfere with CmpGetSymbolicLink
        //
        CmpUnLockKcbArray(LockedKcbs,CmpHashTableSize);
        if( LockedKcbs != LocalFastKcbArray ) {
            ExFreePool(LockedKcbs);
        }
        LockedKcbs = NULL;
        if (CmpGetSymbolicLink(Hive,
                               CompleteName,
                               kcb,
                               &Current)) {

            status = STATUS_REPARSE;
            CmpParseRecordOriginatingPoint(Context,(PCMHIVE)kcb->KeyHive);
        } else {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpParseKey: couldn't find symbolic link name\n"));
            status = STATUS_OBJECT_NAME_NOT_FOUND;
        }
        goto FreeAndReturn;
    }

    Node = (PCM_KEY_NODE)HvGetCell(Hive,Cell);
    if( Node == NULL ) {
        //
        // we couldn't map the bin containing this cell
        //
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto FreeAndReturn;
    }
    CmpReleasePreviousAndHookNew(Hive,Cell,HiveToRelease,CellToRelease);

    //
    // Parse the path.
    //

    status = STATUS_SUCCESS;
    while (TRUE) {

        //
        // Parse out next component of name
        //
        rc = CmpGetNextName(&Current, &NextName, &Last);
        if ((NextName.Length > 0) && (rc == TRUE)) {

            //
            // As we iterate through, we will create a kcb for each subkey parsed.
            // 
            // Always use the information in kcb to avoid
            // touching registry data.
            //
            if (!(kcb->Flags & KEY_SYM_LINK)) {
                //
                // Got a legal name component, see if we can find a sub key
                // that actually has such a name.
                //
                NextCell = CmpFindSubKeyByName(Hive,
                                               Node,
                                               &NextName);

                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpParseKey:\n\t"));
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"NextName = '%wZ'\n\t", &NextName));
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"NextCell = %08lx  Last = %01lx\n", NextCell, Last));

                if (NextCell != HCELL_NIL) {
                    Cell = NextCell;
                    Node = (PCM_KEY_NODE)HvGetCell(Hive,Cell);
                    if( Node == NULL ) {
                        //
                        // we couldn't map the bin containing this cell
                        //
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        break;
                    }
                    
                    CmpReleasePreviousAndHookNew(Hive,Cell,HiveToRelease,CellToRelease);

                    if (Last == TRUE) {
                        BOOLEAN     NeedDeref;

Found:
                        //
                        // We will open the key regardless of whether the
                        // call was open or create, so step through exit
                        // portholes here.
                        //

                        if (CompleteKeyCached == TRUE) {
                            //
                            // If the key found is already cached, 
                            // do not need to StepThroughExit
                            // (no kcb is created using exit node).
                            // This prevents us from touching the key node just for the Flags.
                            //
                        } else {
                            CmpStepThroughExit(Hive, Cell, Node,HiveToRelease,CellToRelease);
                            if( Node == NULL ) {
                                //
                                // we couldn't map view for this cell
                                //
                                status = STATUS_INSUFFICIENT_RESOURCES;
                                break;
                            }
                        }
                        //
                        // We have found the entire path, so we want to open
                        // it (for both Open and Create calls).
                        // Hive,Cell -> the key we are supposed to open.
                        //

                        status = CmpDoOpen(Hive,
                                           Cell,
                                           Node,
                                           AccessState,
                                           AccessMode,
                                           Attributes,
                                           lcontext,
                                           CMP_CREATE_KCB_KCB_LOCKED| (CompleteKeyCached?CMP_DO_OPEN_COMPLETE_KEY_CACHED:0),
                                           &kcb,
                                           &NextName,
                                           CmpParseGetOriginatingPoint(Context),
                                           LockedKcbs,
                                           Object,
                                           &NeedDeref);

                        if (status == STATUS_REPARSE) {
                            //
                            // The given key was a symbolic link.  Find the name of
                            // its link, and return STATUS_REPARSE to the Object Manager.
                            //

                            //
                            // need not to interfere with CmpGetSymbolicLink
                            //
                            CmpUnLockKcbArray(LockedKcbs,CmpHashTableSize);
                            if( LockedKcbs != LocalFastKcbArray ) {
                                ExFreePool(LockedKcbs);
                            }
                            LockedKcbs = NULL;

                            if (!CmpGetSymbolicLink(Hive,
                                                    CompleteName,
                                                    kcb,
                                                    NULL)) {
                                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpParseKey: couldn't find symbolic link name\n"));
                                status = STATUS_OBJECT_NAME_NOT_FOUND;
                            }
                            CmpParseRecordOriginatingPoint(Context,(PCMHIVE)kcb->KeyHive);
                            if( TRUE == NeedDeref  ) {
                                CmpDereferenceKeyControlBlock(kcb);
                            }
                        } else {
                            ASSERT( !NeedDeref );
                        }

                        break;
                    }
                    // else
                    //   Not at end, so we'll simply iterate and consume
                    //   the next component.
                    //
                    //
                    // Step through exit portholes here.
                    // This ensures that no KCB is created using
                    // the Exit node.
                    //

                    CmpStepThroughExit(Hive, Cell, Node,HiveToRelease,CellToRelease);
                    if( Node == NULL ) {
                        //
                        // we couldn't map view for this cell
                        //
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        break;
                    }

                    //
                    // Create a kcb for each subkey parsed.
                    //

                    kcb = CmpCreateKeyControlBlock(Hive,
                                                   Cell,
                                                   Node,
                                                   ParentKcb,
                                                   CMP_CREATE_KCB_KCB_LOCKED,
                                                   &NextName);
            
                    if (kcb  == NULL) {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        goto FreeAndReturn;
                        //
                        // Currently, the kcb has one extra reference count to be decremented.
                        // Remember it so we can dereference it properly.
                        //
                    }
                    //
                    // Now we have created a kcb for the next level,
                    // the kcb in the previous level is no longer needed.
                    // Dereference the parent kcb.
                    //
                    CmpDereferenceKeyControlBlockWithLock(ParentKcb,FALSE);

                    ParentKcb = kcb;


                } else {
                    //
                    // We did not find a key matching the name, but no
                    // unexpected error occured
                    //

                    if ((Last == TRUE) && (ARGUMENT_PRESENT(lcontext))) {

CreateChild:
                        //
                        // Only unfound component is last one, and operation
                        // is a create, so perform the create.
                        //

                        //
                        // There are two possibilities here.  The normal one
                        // is that we are simply creating a new node.
                        //
                        // The abnormal one is that we are creating a root
                        // node that is linked to the main hive.  In this
                        // case, we must create the link.  Once the link is
                        // created, we can check to see if the root node
                        // exists, then either create it or open it as
                        // necessary.
                        //
                        // CmpCreateLinkNode creates the link, and calls
                        // back to CmpDoCreate or CmpDoOpen to create or open
                        // the root node as appropriate.
                        //

                        //
                        // either one of this will drop the reglock and reaquire it 
                        // exclusive; we need not to hurt ourselves, so release
                        // all cells here
                        //
                        CmpReleasePreviousAndHookNew(NULL,HCELL_NIL,HiveToRelease,CellToRelease);

                        if (lcontext->CreateLink) {
                            status = CmpCreateLinkNode(Hive,
                                                       Cell,
                                                       AccessState,
                                                       NextName,
                                                       AccessMode,
                                                       Attributes,
                                                       lcontext,
                                                       ParentKcb,
                                                       LockedKcbs,
                                                       Object);

                        } else {

                            if ( (Hive == &(CmpMasterHive->Hive)) &&
                                 (CmpNoMasterCreates == TRUE) ) {
                                //
                                // attempting to create a cell in the master
                                // hive, and not a link, so blow out of here,
                                // since it wouldn't work anyway.
                                //
                                status = STATUS_INVALID_PARAMETER;
                                break;
                            }

                            status = CmpDoCreate(Hive,
                                                 Cell,
                                                 AccessState,
                                                 &NextName,
                                                 AccessMode,
                                                 lcontext,
                                                 ParentKcb,
                                                 CmpParseGetOriginatingPoint(Context),
                                                 Object);
                        }

                        if( status == STATUS_REPARSE ) {
                            //
                            // somebody else created the key in between; 
                            // let the Object Manager work for us !!!
                            // now we have the lock exclusive, so nothing can happen in between 
                            // next iteration will find the key very quick
                            //
                            break;
                        }
                        lcontext->Disposition = REG_CREATED_NEW_KEY;
                        break;

                    } else {

                        //
                        // Did not find a key to match the component, and
                        // are not at the end of the path.  Thus, open must
                        // fail because the whole path dosn't exist, create must
                        // fail because more than 1 component doesn't exist.
                        //
                        //
                        // We have a lookup failure here, so having additional information
                        // about this kcb may help us not to go through all the code just to fail again.
                        // 
                        ParentKcb = CmpAddInfoAfterParseFailure(Hive,
                                                                Cell,
                                                                Node,
                                                                kcb,
                                                                &NextName
                                                                );
                        
                        status = STATUS_OBJECT_NAME_NOT_FOUND;
                        break;
                    }

                }

            } else {
                //
                // The given key was a symbolic link.  Find the name of
                // its link, and return STATUS_REPARSE to the Object Manager.
                //
                Current.Buffer = NextName.Buffer;
                if (Current.Length + NextName.Length > MAXUSHORT) {
                    status = STATUS_NAME_TOO_LONG;
                    break;
                }
                Current.Length = (USHORT)(Current.Length + NextName.Length);

                if (Current.MaximumLength + NextName.MaximumLength > MAXUSHORT) {
                    status = STATUS_NAME_TOO_LONG;
                    break;
                }
                Current.MaximumLength = (USHORT)(Current.MaximumLength + NextName.MaximumLength);
                //
                // need not to interfere with CmpGetSymbolicLink
                //
                CmpUnLockKcbArray(LockedKcbs,CmpHashTableSize);
                if( LockedKcbs != LocalFastKcbArray ) {
                    ExFreePool(LockedKcbs);
                }
                LockedKcbs = NULL;
                if (CmpGetSymbolicLink(Hive,
                                       CompleteName,
                                       kcb,
                                       &Current)) {

                    status = STATUS_REPARSE;
                    CmpParseRecordOriginatingPoint(Context,(PCMHIVE)kcb->KeyHive);
                    break;

                } else {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpParseKey: couldn't find symbolic link name\n"));
                    status = STATUS_OBJECT_NAME_NOT_FOUND;
                    break;
                }
            }

        } else if (rc == TRUE && Last == TRUE) {
            //
            // We will open the \Registry root.
            // Or some strange remaining name that
            // messes up the lookup.
            //
            CmpStepThroughExit(Hive, Cell, Node,HiveToRelease,CellToRelease);
            if( Node == NULL ) {
                //
                // we couldn't map view for this cell
                //
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            //
            // We have found the entire path, so we want to open
            // it (for both Open and Create calls).
            // Hive,Cell -> the key we are supposed to open.
            //
            status = CmpDoOpen(Hive,
                               Cell,
                               Node,
                               AccessState,
                               AccessMode,
                               Attributes,
                               lcontext,
                               CMP_CREATE_KCB_KCB_LOCKED|CMP_DO_OPEN_COMPLETE_KEY_CACHED,
                               &kcb,
                               &NextName,
                               CmpParseGetOriginatingPoint(Context),
                               LockedKcbs,
                               Object,
                               NULL);
            if(status == STATUS_REPARSE ) {
                CmpParseRecordOriginatingPoint(Context,(PCMHIVE)kcb->KeyHive);
            }
            break;

        } else {

            //
            // bogus path -> fail
            //
            status = STATUS_INVALID_PARAMETER;
            break;
        }

    } // while

FreeAndReturn:
    //
    // Now we have to free the last kcb that still has one extra reference count to
    // protect it from being freed.
    //
    if( LockedKcbs != NULL ) {
        CmpUnLockKcbArray(LockedKcbs,CmpHashTableSize);
        if( LockedKcbs != LocalFastKcbArray ) {
            ExFreePool(LockedKcbs);
        }
        LockedKcbs = NULL;
    }

    if( ParentKcb != NULL ) {
        CmpDereferenceKeyControlBlock(ParentKcb);
    }
JustReturn:
    CmpReleasePreviousAndHookNew(NULL,HCELL_NIL,HiveToRelease,CellToRelease);
    if( LockedKcbs != NULL ) {
        CmpUnLockKcbArray(LockedKcbs,CmpHashTableSize);
        if( LockedKcbs != LocalFastKcbArray ) {
            ExFreePool(LockedKcbs);
        }
    }

    CmpUnlockRegistry();
    END_LOCK_CHECKPOINT;

    //
    // PostCreate callback. just a notification; disregard the return status
    //
    CmPostCallbackNotification((ARGUMENT_PRESENT(lcontext)?RegNtPostCreateKeyEx:RegNtPostOpenKeyEx),(*Object),status);

    return status;
}


BOOLEAN
CmpGetNextName(
    IN OUT PUNICODE_STRING  RemainingName,
    OUT    PUNICODE_STRING  NextName,
    OUT    PBOOLEAN  Last
    )
/*++

Routine Description:

    This routine parses off the next component of a registry path, returning
    all of the interesting state about it, including whether it's legal.

Arguments:

    Current - supplies pointer to variable which points to path to parse.
              on input - parsing starts from here
              on output - updated to reflect starting position for next call.

    NextName - supplies pointer to a unicode_string, which will be set up
               to point into the parse string.

    Last - supplies a pointer to a boolean - set to TRUE if this is the
           last component of the name being parse, FALSE otherwise.

Return Value:

    TRUE if all is well.

    FALSE if illegal name (too long component, bad character, etc.)
        (if false, all out parameter values are bogus.)

--*/
{
    BOOLEAN rc = TRUE;

    //
    // Deal with NULL paths, and pointers to NULL paths
    //
    if ((RemainingName->Buffer == NULL) || (RemainingName->Length == 0)) {
        *Last = TRUE;
        NextName->Buffer = NULL;
        NextName->Length = 0;
        return TRUE;
    }

    if (*(RemainingName->Buffer) == UNICODE_NULL) {
        *Last = TRUE;
        NextName->Buffer = NULL;
        NextName->Length = 0;
        return TRUE;
    }

    //
    // Skip over leading path separators
    //
    while (*(RemainingName->Buffer) == OBJ_NAME_PATH_SEPARATOR) {
        RemainingName->Buffer++;
        RemainingName->Length -= sizeof(WCHAR);
        RemainingName->MaximumLength -= sizeof(WCHAR);
    }

    //
    // Remember where the component starts, and scan to the end
    //
    NextName->Buffer = RemainingName->Buffer;
    while (TRUE) {
        if (RemainingName->Length == 0) {
            break;
        }
        if (*RemainingName->Buffer == OBJ_NAME_PATH_SEPARATOR) {
            break;
        }

        //
        // NOT at end
        // NOT another path sep
        //

        RemainingName->Buffer++;
        RemainingName->Length -= sizeof(WCHAR);
        RemainingName->MaximumLength -= sizeof(WCHAR);
    }

    //
    // Compute component length, return error if it's illegal
    //
    NextName->Length = (USHORT)
        ((PUCHAR)RemainingName->Buffer - (PUCHAR)(NextName->Buffer));
    if (NextName->Length > REG_MAX_KEY_NAME_LENGTH)
    {
        rc = FALSE;
    }
    NextName->MaximumLength = NextName->Length;

	//
    // Set last, return success
    //
    *Last = (RemainingName->Length == 0) ? TRUE : FALSE;
    return rc;
}

NTSTATUS
CmpDoOpen(
    IN PHHIVE                       Hive,
    IN HCELL_INDEX                  Cell,
    IN PCM_KEY_NODE                 Node,
    IN PACCESS_STATE                AccessState,
    IN KPROCESSOR_MODE              AccessMode,
    IN ULONG                        Attributes,
    IN PCM_PARSE_CONTEXT            Context OPTIONAL,
    IN ULONG                        ControlFlags,
    IN OUT PCM_KEY_CONTROL_BLOCK    *CachedKcb,
    IN PUNICODE_STRING              KeyName,
    IN PCMHIVE                      OriginatingHive OPTIONAL,
    IN PULONG                       KcbsLocked,
    OUT PVOID                       *Object,
    OUT PBOOLEAN                    NeedDeref OPTIONAL
    )
/*++

Routine Description:

    Open a registry key, create a keycontrol block.

Arguments:

    Hive - supplies a pointer to the hive control structure for the hive

    Cell - supplies index of node to delete

    AccessState - Running security access state information for operation.

    AccessMode - Access mode of the original caller.

    Attributes - Attributes to be applied to the object.

    Context - if create or hive root open, points to a CM_PARSE_CONTEXT
              structure,
              if open, is NULL.

    CompleteKeyCached - BOOLEAN to indicate it the completekey is cached.

    CachedKcb - If the completekey is cached, this is the kcb for the destination.
                If not, this is the parent kcb.

    KeyName - Relative name (to BaseName)

    Object - The address of a variable to receive the created key object, if
             any.

    NeedDeref - if specified, keep reference in the fake create kcb (link case). Caller
                is responsible to release the fake kcb after it finishes with it.

Return Value:

    NTSTATUS


--*/
{
    NTSTATUS status;
    PCM_KEY_BODY            pbody = NULL;
    PCM_KEY_CONTROL_BLOCK kcb = NULL;
    KPROCESSOR_MODE   mode;
    BOOLEAN BackupRestore;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpDoOpen:\n"));
    
    if( ARGUMENT_PRESENT(NeedDeref) ) {
        *NeedDeref = FALSE;
    }

    //
    // don't allow others to use this until it is up and running
    //
    if( (Hive->HiveFlags & HIVE_IS_UNLOADING) && (((PCMHIVE)Hive)->CreatorOwner != KeGetCurrentThread()) ) {
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    if (ARGUMENT_PRESENT(Context)) {

        //
        // It's a create of some sort
        //
        if (Context->CreateLink) {
            //
            // The node already exists as a regular key, so it cannot be
            // turned into a link node.
            //
            return STATUS_ACCESS_DENIED;

        } else if (Context->CreateOptions & REG_OPTION_CREATE_LINK) {
            //
            // Attempt to create a symbolic link has hit an existing key
            // so return an error
            //
            return STATUS_OBJECT_NAME_COLLISION;

        } else {
            //
            // Operation is an open, so set Disposition
            //
            Context->Disposition = REG_OPENED_EXISTING_KEY;
        }
    }

    ASSERT( ControlFlags&CMP_CREATE_KCB_KCB_LOCKED );
    //
    // Check for symbolic link and caller does not want to open a link
    //
    if(ControlFlags&CMP_DO_OPEN_COMPLETE_KEY_CACHED) {
        ASSERT_KCB_LOCKED(*CachedKcb);
    
        //
        // The complete key is cached.
        //
        if ((*CachedKcb)->Flags & KEY_SYM_LINK && !(Attributes & OBJ_OPENLINK)) {
            //
            // If the key is a symbolic link, check if the link has been resolved.
            // If the link is resolved, change the kcb to the real KCB.
            // Otherwise, return for reparse.
            //
            if ((*CachedKcb)->ExtFlags & CM_KCB_SYM_LINK_FOUND) {
                kcb = (*CachedKcb)->ValueCache.RealKcb;
                
                ASSERT( KcbsLocked );
                //
                // deadlock avoidance; unlock all locked kcbs and lock the symbolic one
                // store the index in the array; caller will take care of unlock
                //
                CmpUnLockKcbArray(KcbsLocked,CmpHashTableSize);
                KcbsLocked[0] = 1; //just this
                KcbsLocked[1] = GET_HASH_INDEX(kcb->ConvKey);
                CmpLockHashEntryByIndexExclusive(KcbsLocked[1]);

                if (kcb->Delete == TRUE) {
                    //
                    // The real key it points to had been deleted.
                    // We have no way of knowing if the key has been recreated.
                    // Just clean up the cache and do a reparse.
                    //
                    CmpUnlockHashEntryByIndex(KcbsLocked[1]);
                    KcbsLocked[1] = GET_HASH_INDEX((*CachedKcb)->ConvKey);
                    CmpLockHashEntryByIndexExclusive(KcbsLocked[1]);
                    CmpCleanUpKcbValueCache(*CachedKcb);
                    return(STATUS_REPARSE);
                }
            } else {
                return(STATUS_REPARSE);
            }
        } else {
            //
            // Not a symbolic link, increase the reference Count of Kcb.
            //
            kcb = *CachedKcb;
        }
        // common path instead of repeating code
        if (!CmpReferenceKeyControlBlock(kcb)) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
   } else {
            //
            // The key is not in cache, the CachedKcb is the parentkcb of this
            // key to be opened.
            //
        ASSERT_KCB_LOCKED_EXCLUSIVE(*CachedKcb);

        if (Node->Flags & KEY_SYM_LINK && !(Attributes & OBJ_OPENLINK)) {
            //
            // Create a KCB for this symbolic key and put it in delay close.
            //
            kcb = CmpCreateKeyControlBlock(Hive, Cell, Node, *CachedKcb,CMP_CREATE_KCB_KCB_LOCKED, KeyName);
            if (kcb  == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            ASSERT_KCB_LOCKED_EXCLUSIVE(kcb);
            if( ARGUMENT_PRESENT(NeedDeref) ) {
                //
                // caller will perform deref.
                //
                *NeedDeref = TRUE;
            } else {
                CmpDereferenceKeyControlBlockWithLock(kcb,FALSE);
            }
            *CachedKcb = kcb;
            return(STATUS_REPARSE);
        }
    
        //
        // If key control block does not exist, and cannot be created, fail,
        // else just increment the ref count (done for us by CreateKeyControlBlock)
        //
        kcb = CmpCreateKeyControlBlock(Hive, Cell, Node, *CachedKcb,CMP_CREATE_KCB_KCB_LOCKED, KeyName);
        if (kcb  == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
   
        ASSERT_KCB_LOCKED_EXCLUSIVE(kcb);
        *CachedKcb = kcb;
    }

#if DBG
   if( kcb->ExtFlags & CM_KCB_KEY_NON_EXIST ) {
        //
        // we shouldn't fall into this
        //
        DbgBreakPoint();
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }
#endif // DBG
   
   if(!CmpOKToFollowLink(OriginatingHive,(PCMHIVE)Hive) ) {
       //
       // about to cross class of trust boundary
       //
       status = STATUS_ACCESS_DENIED;
   } else {
        //
        // Allocate the object.
        //
        status = ObCreateObject(AccessMode,
                                CmpKeyObjectType,
                                NULL,
                                AccessMode,
                                NULL,
                                sizeof(CM_KEY_BODY),
                                0,
                                0,
                                Object);
   }
    if (NT_SUCCESS(status)) {

        pbody = (PCM_KEY_BODY)(*Object);

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_POOL,"CmpDoOpen: object allocated at:%p\n", pbody));

        //
        // Check for predefined handle
        //

        pbody = (PCM_KEY_BODY)(*Object);

        if (kcb->Flags & KEY_PREDEF_HANDLE) {
            pbody->Type = kcb->ValueCache.Count;
            pbody->KeyControlBlock = kcb;
            return(STATUS_PREDEFINED_HANDLE);
        } else {
            //
            // Fill in CM specific fields in the object
            //
            pbody->Type = KEY_BODY_TYPE;
            pbody->KeyControlBlock = kcb;
            pbody->NotifyBlock = NULL;
            pbody->ProcessID = PsGetCurrentProcessId();
            EnlistKeyBodyWithKCB(pbody,(ControlFlags&CMP_DO_OPEN_COMPLETE_KEY_CACHED)? CMP_ENLIST_KCB_LOCKED_SHARED : CMP_ENLIST_KCB_LOCKED_EXCLUSIVE);
        }

    } else {

        //
        // Failed to create object, so undo key control block work
        //
#if DBG
        if(ControlFlags&CMP_DO_OPEN_COMPLETE_KEY_CACHED) {
            ASSERT_KCB_LOCKED(kcb);
        } else {
            ASSERT_KCB_LOCKED_EXCLUSIVE(kcb);
        }
#endif
        CmpDereferenceKeyControlBlockWithLock(kcb,FALSE);
        return status;
    }

    //
    // Check to make sure the caller can access the key.
    //
    BackupRestore = FALSE;
    if (ARGUMENT_PRESENT(Context)) {
        if (Context->CreateOptions & REG_OPTION_BACKUP_RESTORE) {
            BackupRestore = TRUE;
        }
    }

    status = STATUS_SUCCESS;

    if (BackupRestore == TRUE) {

        //
        // this is an open to support a backup or restore
        // operation, do the special case work
        //
        AccessState->RemainingDesiredAccess = 0;
        AccessState->PreviouslyGrantedAccess = 0;

        mode = KeGetPreviousMode();

        if (SeSinglePrivilegeCheck(SeBackupPrivilege, mode)) {
            AccessState->PreviouslyGrantedAccess |=
                KEY_READ | ACCESS_SYSTEM_SECURITY;
        }

        if (SeSinglePrivilegeCheck(SeRestorePrivilege, mode)) {
            AccessState->PreviouslyGrantedAccess |=
                KEY_WRITE | ACCESS_SYSTEM_SECURITY | WRITE_DAC | WRITE_OWNER;
        }

        if (AccessState->PreviouslyGrantedAccess == 0) {
            //
            // relevant privileges not asserted/possessed, so
            // deref (which will cause CmpDeleteKeyObject to clean up)
            // and return an error.
            //
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpDoOpen for backup restore: access denied\n"));
            ObDereferenceObjectDeferDelete(*Object);
            return STATUS_ACCESS_DENIED;
        }

    } else {
        BOOLEAN AllowAccess;
        //
        // trick; last bit set means kcb locked exclusive
        //
        ASSERT( pbody );
        pbody->KeyControlBlock = (PCM_KEY_CONTROL_BLOCK)((ULONG_PTR)pbody->KeyControlBlock + 1);

        AllowAccess = ObCheckObjectAccess(*Object,
                                  AccessState,
                                  TRUE,         // Type mutex already locked
                                  AccessMode,
                                  &status);
        pbody->KeyControlBlock = (PCM_KEY_CONTROL_BLOCK)((ULONG_PTR)pbody->KeyControlBlock ^ 1);
        if (!AllowAccess) {
            //
            // Access denied, so deref object, will cause CmpDeleteKeyObject
            // to be called, it will clean up.
            //
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpDoOpen: access denied\n"));
            ObDereferenceObjectDeferDelete(*Object);
        }
    }

    return status;
}

NTSTATUS
CmpCreateLinkNode(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell,
    IN PACCESS_STATE AccessState,
    IN UNICODE_STRING Name,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN PCM_PARSE_CONTEXT Context,
    IN PCM_KEY_CONTROL_BLOCK ParentKcb,
    IN PULONG                   LockedKcbs,
    OUT PVOID *Object
    )
/*++

Routine Description:

    Perform the creation of a link node.  Allocate all components,
    and attach to parent key.  Calls CmpDoCreate or CmpDoOpen to
    create or open the root node of the hive as appropriate.

    Note that you can only create link nodes in the master hive.

Arguments:

    Hive - supplies a pointer to the hive control structure for the hive

    Cell - supplies index of node to create child under

    Name - supplies pointer to a UNICODE string which is the name of
            the child to be created.

    AccessMode - Access mode of the original caller.

    Attributes - Attributes to be applied to the object.

    Context - pointer to CM_PARSE_CONTEXT structure passed through
                the object manager

    BaseName - Name of object create is relative to

    KeyName - Relative name (to BaseName)

    Object - The address of a variable to receive the created key object, if
             any.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS                Status;
    PCELL_DATA              Parent;
    PCELL_DATA              Link;
    PCELL_DATA              CellData;
    HCELL_INDEX             LinkCell;
    HCELL_INDEX             KeyCell;
    HCELL_INDEX             ChildCell;
    PCM_KEY_CONTROL_BLOCK   kcb = ParentKcb;  
    PCM_KEY_BODY            KeyBody;
    LARGE_INTEGER           systemtime;
    PCM_KEY_NODE            TempNode;
#if DBG
    ULONG                   ChildConvKey;
#endif

    ASSERT_CM_LOCK_OWNED();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpCreateLinkNode:\n"));

    if (Hive != &CmpMasterHive->Hive) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpCreateLinkNode: attempt to create link node in\n"));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"    non-master hive %p\n", Hive));
        return(STATUS_ACCESS_DENIED);
    }

#if DBG
    //
    // debug only code
    //
    *Object = NULL;
#endif
#if DBG
    ChildConvKey = ParentKcb->ConvKey;

    if (Name.Length) {
        ULONG                   Cnt;
        WCHAR                   *Cp;
        Cp = Name.Buffer;
        for (Cnt=0; Cnt<Name.Length; Cnt += sizeof(WCHAR)) {
            //
            // UNICODE_NULL is a valid char !!!
            //
            if (*Cp != OBJ_NAME_PATH_SEPARATOR) {
                //(*Cp != UNICODE_NULL)) {
                ChildConvKey = 37 * ChildConvKey + (ULONG)CmUpcaseUnicodeChar(*Cp);
            }
            ++Cp;
        }
    }
    
    ASSERT_HASH_ENTRY_LOCKED_EXCLUSIVE(ChildConvKey);
    ASSERT_KCB_LOCKED_EXCLUSIVE(ParentKcb);
#endif
    // no flush while we are doing this
    CmpLockHiveFlusherShared((PCMHIVE)Hive);
    CmpLockHiveFlusherShared((PCMHIVE)Context->ChildHive.KeyHive);
    //
    // this is a create, so we need exclusive access on the registry
    // first get the time stamp to see if somebody messed with this key
    // this might be more easier if we decide to cache the LastWriteTime
    // in the KCB ; now it IS !!!
    //
    if( CmIsKcbReadOnly(ParentKcb) ) {
        //
        // key is protected
        //
        Status = STATUS_ACCESS_DENIED;
        goto Exit;
    } 

    //
    // make sure nothing changed in between:
    //  1. ParentKcb is still valid
    //  2. Child was not already added by somebody else 
    //
    if( ParentKcb->Delete ) {
        //
        // key was deleted in between
        //
        Status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto Exit;
    }

    //
    // Allocate link node
    //
    // Link nodes are always in the master hive, so their storage type is
    // mostly irrelevant.
    //
    LinkCell = HvAllocateCell(Hive,  CmpHKeyNodeSize(Hive, &Name), Stable,HCELL_NIL);
    if (LinkCell == HCELL_NIL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    KeyCell = Context->ChildHive.KeyCell;

    if (KeyCell != HCELL_NIL) {

        //
        // This hive already exists, so we just need to open the root node.
        //
        ChildCell=KeyCell;

        //
        // The root cell in the hive does not has the Name buffer 
        // space reserved.  This is why we need to pass in the Name for creating KCB
        // instead of using the name in the keynode.
        //
        CellData = HvGetCell(Context->ChildHive.KeyHive, ChildCell);
        if( CellData == NULL ) {
            //
            // we couldn't map the bin containing this cell
            //
            HvFreeCell(Hive, LinkCell);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit;
        }
        
        CellData->u.KeyNode.Parent = LinkCell;
        CellData->u.KeyNode.Flags |= KEY_HIVE_ENTRY | KEY_NO_DELETE;
        HvReleaseCell(Context->ChildHive.KeyHive, ChildCell);
        
        TempNode = (PCM_KEY_NODE)HvGetCell(Context->ChildHive.KeyHive,KeyCell);
        if( TempNode == NULL ) {
            //
            // we couldn't map the bin containing this cell
            //
            HvFreeCell(Hive, LinkCell);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit;
        }

        Status = CmpDoOpen( Context->ChildHive.KeyHive,
                            KeyCell,
                            TempNode,
                            AccessState,
                            AccessMode,
                            Attributes,
                            NULL,
                            CMP_CREATE_KCB_KCB_LOCKED,
                            &kcb,
                            &Name,
                            CmpParseGetOriginatingPoint(Context),
                            LockedKcbs,
                            Object,
                            NULL);
        HvReleaseCell(Context->ChildHive.KeyHive,KeyCell);
    } else {

        //
        // This is a newly created hive, so we must allocate and initialize
        // the root node.
        //

        Status = CmpDoCreateChild( Context->ChildHive.KeyHive,
                                   Cell,
                                   NULL,
                                   AccessState,
                                   &Name,
                                   AccessMode,
                                   Context,
                                   ParentKcb,
                                   KEY_HIVE_ENTRY | KEY_NO_DELETE,
                                   &ChildCell,
                                   Object );

        if (NT_SUCCESS(Status)) {

            //
            // Initialize hive root cell pointer.
            //

            Context->ChildHive.KeyHive->BaseBlock->RootCell = ChildCell;
        }

    }
    if (NT_SUCCESS(Status)) {

        //
        // Initialize parent and flags.  Note that we do this whether the
        // root has been created or opened, because we are not guaranteed
        // that the link node is always the same cell in the master hive.
        //
        if (!HvMarkCellDirty(Context->ChildHive.KeyHive, ChildCell, FALSE)) {
            Status = STATUS_NO_LOG_SPACE;
            goto Cleanup;
        }
        CellData = HvGetCell(Context->ChildHive.KeyHive, ChildCell);
        if( CellData == NULL ) {
            //
            // we couldn't map the bin containing this cell
            //
            HvFreeCell(Hive, LinkCell);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }
        
        // release the cell right here as the view is already pinned
        HvReleaseCell(Context->ChildHive.KeyHive, ChildCell);

        CellData->u.KeyNode.Parent = LinkCell;
        CellData->u.KeyNode.Flags |= KEY_HIVE_ENTRY | KEY_NO_DELETE;

        //
        // Initialize special link node flags and data
        //
        Link = HvGetCell(Hive, LinkCell);
        if( Link == NULL ) {
            //
            // we couldn't map the bin containing this cell
            // this shouldn't happen as we just allocated this cell
            // (i.e. it should be PINNED in memory at this point)
            //
            ASSERT( FALSE );
            HvFreeCell(Hive, LinkCell);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        Link->u.KeyNode.Signature = CM_LINK_NODE_SIGNATURE;
        Link->u.KeyNode.Flags = KEY_HIVE_EXIT | KEY_NO_DELETE;
        Link->u.KeyNode.Parent = Cell;
        Link->u.KeyNode.NameLength = CmpCopyName(Hive, Link->u.KeyNode.Name, &Name);
        if (Link->u.KeyNode.NameLength < Name.Length) {
            Link->u.KeyNode.Flags |= KEY_COMP_NAME;
        }

        KeQuerySystemTime(&systemtime);
        Link->u.KeyNode.LastWriteTime = systemtime;

        //
        // Zero out unused fields.
        //
        Link->u.KeyNode.SubKeyCounts[Stable] = 0;
        Link->u.KeyNode.SubKeyCounts[Volatile] = 0;
        Link->u.KeyNode.SubKeyLists[Stable] = HCELL_NIL;
        Link->u.KeyNode.SubKeyLists[Volatile] = HCELL_NIL;
        Link->u.KeyNode.ValueList.Count = 0;
        Link->u.KeyNode.ValueList.List = HCELL_NIL;
        Link->u.KeyNode.ClassLength = 0;


        //
        // Fill in the link node's pointer to the root node
        //
        Link->u.KeyNode.ChildHiveReference.KeyHive = Context->ChildHive.KeyHive;
        Link->u.KeyNode.ChildHiveReference.KeyCell = ChildCell;

        HvReleaseCell(Hive,LinkCell);
        //
        // get the parent first, we don't need to do unnecessary cleanup
        //
        Parent = HvGetCell(Hive, Cell);
        if( Parent == NULL ) {
            //
            // we couldn't map the bin containing this cell
            //
            HvFreeCell(Hive, LinkCell);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        //
        // Fill in the parent cell's child list
        //
        if (! CmpAddSubKey(Hive, Cell, LinkCell)) {
            HvReleaseCell(Hive,Cell);
            HvFreeCell(Hive, LinkCell);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        //
        // If the parent has the subkey info or hint cached, free it.
        //
        KeyBody = (PCM_KEY_BODY)(*Object);
        CmpCleanUpSubKeyInfo (KeyBody->KeyControlBlock->ParentKcb);

        //
        // Update max keyname and class name length fields
        //

        //
        // It seems to me that the original code is wrong.
        // Isn't the definition of MaxNameLen just the length of the subkey?
        //
        
        // some sanity asserts
        ASSERT( KeyBody->KeyControlBlock->ParentKcb->KeyCell == Cell );
        ASSERT( KeyBody->KeyControlBlock->ParentKcb->KeyHive == Hive );
        ASSERT( KeyBody->KeyControlBlock->ParentKcb->KcbMaxNameLen == Parent->u.KeyNode.MaxNameLen );
        
        //
        // update the LastWriteTime on both keynode and kcb;
        //
        KeQuerySystemTime(&systemtime);
        Parent->u.KeyNode.LastWriteTime = systemtime;
        KeyBody->KeyControlBlock->ParentKcb->KcbLastWriteTime = systemtime;

        if (Parent->u.KeyNode.MaxNameLen < Name.Length) {
            Parent->u.KeyNode.MaxNameLen = Name.Length;
            KeyBody->KeyControlBlock->ParentKcb->KcbMaxNameLen = Name.Length;
        }

        if (Parent->u.KeyNode.MaxClassLen < Context->Class.Length) {
            Parent->u.KeyNode.MaxClassLen = Context->Class.Length;
        }
        HvReleaseCell(Hive,Cell);
Cleanup:
        if( !NT_SUCCESS(Status) ) {
            ASSERT( (*Object) != NULL );
            //
            // mark the kcb as "no-delay-close" so it gets kicked out of cache when 
            // refcount goes down to 0
            //
            KeyBody = (PCM_KEY_BODY)(*Object);

            ASSERT_KCB_LOCKED_EXCLUSIVE(KeyBody->KeyControlBlock);
            ASSERT( KeyBody->KeyControlBlock );
            ASSERT_KCB( KeyBody->KeyControlBlock );
            KeyBody->KeyControlBlock->ExtFlags |= CM_KCB_NO_DELAY_CLOSE;
            KeyBody->KeyControlBlock->Delete = TRUE;
            CmpRemoveKeyControlBlock(KeyBody->KeyControlBlock);
            KeyBody->KeyControlBlock->KeyCell = HCELL_NIL;
            ObDereferenceObjectDeferDelete(*Object);
        }

    } else {
        HvFreeCell(Hive, LinkCell);
    }

Exit:
    CmpUnlockHiveFlusher((PCMHIVE)Context->ChildHive.KeyHive);
    CmpUnlockHiveFlusher((PCMHIVE)Hive);
    return Status;
}

BOOLEAN
CmpGetSymbolicLink(
    IN PHHIVE Hive,
    IN OUT PUNICODE_STRING ObjectName,
    IN OUT PCM_KEY_CONTROL_BLOCK SymbolicKcb,
    IN PUNICODE_STRING RemainingName OPTIONAL
    )

/*++

Routine Description:

    This routine extracts the symbolic link name from a key, if it is
    marked as a symbolic link.

Arguments:

    Hive - Supplies the hive of the key.

    ObjectName - Supplies the current ObjectName.
                 Returns the new ObjectName.  If the new name is longer
                 than the maximum length of the current ObjectName, the
                 old buffer will be freed and a new buffer allocated.

    RemainingName - Supplies the remaining path.  If present, this will be
                concatenated with the symbolic link to form the new objectname.

Return Value:

    TRUE - symbolic link succesfully found

    FALSE - Key is not a symbolic link, or an error occurred

--*/

{
    NTSTATUS                Status;
    HCELL_INDEX             LinkCell = HCELL_NIL;
    PCM_KEY_VALUE           LinkValue = NULL;
    PWSTR                   LinkName = NULL;
    BOOLEAN                 LinkNameAllocated = FALSE;
    PWSTR                   NewBuffer;
    ULONG                   Length = 0;
    ULONG                   ValueLength = 0;
    PUNICODE_STRING         ConstructedName = NULL;
    ULONG                   ConvKey=0;
    PCM_KEY_HASH            KeyHash;
    PCM_KEY_CONTROL_BLOCK   RealKcb;
    BOOLEAN                 KcbFound = FALSE;
    ULONG                   Cnt;
    WCHAR                   *Cp;
    WCHAR                   *Cp2;
    ULONG                   TotalLevels;
    BOOLEAN                 FreeConstructedName = FALSE;
    BOOLEAN                 Result = TRUE;
    HCELL_INDEX             CellToRelease = HCELL_NIL;
    ULONG                   ConvKey1 = 0;
    ULONG                   ConvKey2 = 0;
    BOOLEAN                 BothHashesLocked = FALSE;
    BOOLEAN                 UnlockConvKey1 = FALSE;

    
    //
    ConvKey1 = SymbolicKcb->ConvKey;
    CmpLockKCBExclusive(SymbolicKcb);
Again:
    if( SymbolicKcb->Delete ) {
        if( !BothHashesLocked ) {
            CmpUnlockKCB(SymbolicKcb);
        } else {
            CmpUnlockTwoHashEntries(ConvKey1,ConvKey2);
        }
        return FALSE;
    }
    if (SymbolicKcb->ExtFlags & CM_KCB_SYM_LINK_FOUND) {
        if( !BothHashesLocked ) {
            ConvKey2 = SymbolicKcb->ValueCache.RealKcb->ConvKey;
            CmpUnlockKCB(SymbolicKcb);
            CmpLockTwoHashEntriesExclusive(ConvKey1,ConvKey2);
            BothHashesLocked = TRUE;
            goto Again;
        }
        //
        // First see of the real kcb for this symbolic name has been found
        // 
        ConstructedName = CmpConstructName(SymbolicKcb->ValueCache.RealKcb);
        if (ConstructedName) {
            FreeConstructedName = TRUE;
            LinkName = ConstructedName->Buffer;
            ValueLength = ConstructedName->Length;
            Length = (USHORT)ValueLength + sizeof(WCHAR);
        }
    } 
    // we still need symbolicLink to reach to the name
    if( BothHashesLocked && (GET_HASH_INDEX(ConvKey1) != GET_HASH_INDEX(ConvKey2)) ) {
        CmpUnlockHashEntry(ConvKey2);
    }
    UnlockConvKey1 = TRUE;

    if (FreeConstructedName == FALSE) {
        PCM_KEY_NODE Node;
        ASSERT_KCB_LOCKED(SymbolicKcb);
        //
        // Find the SymbolicLinkValue value.  This is the name of the symbolic link.
        //
        Node = (PCM_KEY_NODE)HvGetCell(SymbolicKcb->KeyHive,SymbolicKcb->KeyCell);
        if( Node == NULL ) {
            //
            // we couldn't map the bin containing this cell
            //
            Result = FALSE;
            goto Exit;
        }

        LinkCell = CmpFindValueByName(Hive,
                                      Node,
                                      &CmSymbolicLinkValueName);
        // release the node here as we don't need it anymore
        HvReleaseCell(SymbolicKcb->KeyHive,SymbolicKcb->KeyCell);
        if (LinkCell == HCELL_NIL) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpGetSymbolicLink: couldn't open symbolic link\n"));
            Result = FALSE;
            goto Exit;
        }
    
        LinkValue = (PCM_KEY_VALUE)HvGetCell(Hive, LinkCell);
        if( LinkValue == NULL ) {
            //
            // we couldn't map the bin containing this cell
            //
            Result = FALSE;
            goto Exit;
        }
    
            if (LinkValue->Type != REG_LINK) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpGetSymbolicLink: link value is wrong type: %08lx", LinkValue->Type));
            Result = FALSE;
            goto Exit;
        }
    

        if( CmpGetValueData(Hive,LinkValue,&ValueLength,&LinkName,&LinkNameAllocated,&CellToRelease) == FALSE ) {
            //
            // insufficient resources; return NULL
            //
            ASSERT( LinkNameAllocated == FALSE );
            ASSERT( LinkName == NULL );
            Result = FALSE;
            goto Exit;
        }
    
        Length = (USHORT)ValueLength + sizeof(WCHAR);

        //
        // Now see if we have this kcb cached.
        //
        Cp = LinkName;
        //
        // first char SHOULD be OBJ_NAME_PATH_SEPARATOR, otherwise we could get into real trouble!!!
        //
        if( *Cp != OBJ_NAME_PATH_SEPARATOR ) {
            Result = FALSE;
            goto Exit;
        }

        TotalLevels = 0;
        for (Cnt=0; Cnt<ValueLength; Cnt += sizeof(WCHAR)) {
            if (*Cp != OBJ_NAME_PATH_SEPARATOR) {
                ConvKey = 37 * ConvKey + (ULONG) CmUpcaseUnicodeChar(*Cp);
            } else {
                TotalLevels++;
            }
            ++Cp;
        }

        CmpUnlockHashEntry(ConvKey1);
        UnlockConvKey1 = FALSE;
        //
        // lock symbolickcb in advance
        //
        CmpLockTwoHashEntriesExclusive(ConvKey,SymbolicKcb->ConvKey);
        if( SymbolicKcb->Delete ) {
            CmpUnlockTwoHashEntries(ConvKey,SymbolicKcb->ConvKey);
            Result = FALSE;
            goto Exit;
        }
        KeyHash = GET_KCB_HASH_ENTRY(CmpCacheTable, ConvKey); 

        while (KeyHash) {
            RealKcb =  CONTAINING_RECORD(KeyHash, CM_KEY_CONTROL_BLOCK, KeyHash);
            if ((ConvKey == KeyHash->ConvKey) && (TotalLevels == RealKcb->TotalLevels) && (!(RealKcb->ExtFlags & CM_KCB_KEY_NON_EXIST)) ) {
                ConstructedName = CmpConstructName(RealKcb);
                if (ConstructedName) {
                    FreeConstructedName = TRUE;
                    if (ConstructedName->Length == ValueLength) {
                        KcbFound = TRUE;
                        Cp = LinkName;
                        Cp2 = ConstructedName->Buffer;
                        for (Cnt=0; Cnt<ConstructedName->Length; Cnt += sizeof(WCHAR)) {
                            if (CmUpcaseUnicodeChar(*Cp) != CmUpcaseUnicodeChar(*Cp2)) {
                                KcbFound = FALSE;
                                break;
                            }
                            ++Cp;
                            ++Cp2;
                        }
                        if (KcbFound) {
                            //
                            // Now the RealKcb is also pointed to by its symbolic link Kcb,
                            // Increase the reference count.
                            // Need to dereference the realkcb when the symbolic kcb is removed.
                            // Do this in CmpCleanUpKcbCacheWithLock();
                            //
                            if (CmpReferenceKeyControlBlock(RealKcb)) {
                                if( CmpOKToFollowLink( (((PCMHIVE)(SymbolicKcb->KeyHive))->Flags&CM_CMHIVE_FLAG_UNTRUSTED)?(PCMHIVE)(SymbolicKcb->KeyHive):NULL,
                                                    (PCMHIVE)(RealKcb->KeyHive))) {
                                    //
                                    // This symbolic kcb may have value lookup for the path
                                    // Cleanup the value cache.
                                    //
                                    CmpCleanUpKcbValueCache(SymbolicKcb);

                                    SymbolicKcb->ExtFlags |= CM_KCB_SYM_LINK_FOUND;
                                    SymbolicKcb->ValueCache.RealKcb = RealKcb;
                                } else {
                                    //
                                    // let go of the extra ref and break
                                    //
                                    CmpDereferenceKeyControlBlockWithLock(RealKcb,FALSE);
                                    break;
                                }
                            } else {
                                //
                                // We have maxed out the ref count on the real kcb.
                                // do not cache the symbolic link.
                                //
                            }
                            break;
                        }
                    }
                } else {
                    break;
                }
            }
            if (FreeConstructedName) {
                ExFreePoolWithTag(ConstructedName, CM_NAME_TAG | PROTECTED_POOL);
                FreeConstructedName = FALSE;
            }
            KeyHash = KeyHash->NextHash;
        }
        CmpUnlockTwoHashEntries(ConvKey,SymbolicKcb->ConvKey);
    }
    
    if (ARGUMENT_PRESENT(RemainingName)) {
        Length += RemainingName->Length + sizeof(WCHAR);
    }

    //
    // Overflow test: If Length overflows the USHRT_MAX value
    //                cleanup and return FALSE  
    //
    if( Length>0xFFFF ) {
        Result = FALSE;
        goto Exit;
    }

	if (Length > ObjectName->MaximumLength) {
        UNICODE_STRING NewObjectName;

        //
        // The new name is too long to fit in the existing ObjectName buffer,
        // so allocate a new buffer.
        //
        NewBuffer = ExAllocatePool(PagedPool, Length);
        if (NewBuffer == NULL) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpGetSymbolicLink: couldn't allocate new name buffer\n"));
            Result = FALSE;
            goto Exit;
        }

        NewObjectName.Buffer = NewBuffer;
#pragma prefast(suppress:12005, "overflow test already done above")
        NewObjectName.MaximumLength = (USHORT)Length;
        NewObjectName.Length = (USHORT)ValueLength;
        RtlCopyMemory(NewBuffer, LinkName, ValueLength);
#if DBG
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpGetSymbolicLink: LinkName is %wZ\n", ObjectName));
        if (ARGUMENT_PRESENT(RemainingName)) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"               RemainingName is %wZ\n", RemainingName));
        } else {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"               RemainingName is NULL\n"));
        }
#endif
        if (ARGUMENT_PRESENT(RemainingName)) {
            NewBuffer[ ValueLength / sizeof(WCHAR) ] = OBJ_NAME_PATH_SEPARATOR;
            NewObjectName.Length += sizeof(WCHAR);
            Status = RtlAppendUnicodeStringToString(&NewObjectName, RemainingName);
            ASSERT(NT_SUCCESS(Status));
        }

        ExFreePool(ObjectName->Buffer);
        *ObjectName = NewObjectName;
    } else {
        //
        // The new name will fit within the maximum length of the existing
        // ObjectName, so do the expansion in-place. Note that the remaining
        // name must be moved into its new position first since the symbolic
        // link may or may not overlap it.
        //
        ObjectName->Length = (USHORT)ValueLength;
        if (ARGUMENT_PRESENT(RemainingName)) {
            RtlMoveMemory(&ObjectName->Buffer[(ValueLength / sizeof(WCHAR)) + 1],
                          RemainingName->Buffer,
                          RemainingName->Length);
            ObjectName->Buffer[ValueLength / sizeof(WCHAR)] = OBJ_NAME_PATH_SEPARATOR;
            ObjectName->Length += RemainingName->Length + sizeof(WCHAR);
        }
        RtlCopyMemory(ObjectName->Buffer, LinkName, ValueLength);
    }
    ObjectName->Buffer[ObjectName->Length / sizeof(WCHAR)] = UNICODE_NULL;

Exit:
    if( UnlockConvKey1 ) {
        CmpUnlockHashEntry(ConvKey1);
    }
    if( LinkNameAllocated ) {
        ExFreePool(LinkName);
    }
    if (FreeConstructedName) {
        ExFreePoolWithTag(ConstructedName, CM_NAME_TAG | PROTECTED_POOL);
    }

    if( LinkValue != NULL ) {
        ASSERT( LinkCell != HCELL_NIL );
        HvReleaseCell(Hive,LinkCell);
    }
    if( CellToRelease != HCELL_NIL ) {
        HvReleaseCell(Hive,CellToRelease);
    }
    return Result;
}


ULONG
CmpComputeHashValue(
    IN PCM_HASH_ENTRY HashStack,
    IN OUT ULONG  *TotalSubkeys,
    IN ULONG BaseConvKey,
    IN PUNICODE_STRING RemainingName
    )

/*++

Routine Description:

    This routine parses the complete path of a request registry key and calculate
    the hash value at each level.

Arguments:

    HashStack - Array for filling the hash value of each level.

    TotalSubkeys - a pointer to fill the total number of subkeys

    BaseConvKey - Supplies the convkey for the base key.

    RemainingName - supplies pointer to a unicode_string for RemainingName.

Return Value:

    Number of Levels in RemainingName

--*/

{
    ULONG  TotalRemainingSubkeys=0;
    ULONG  TotalKeys=0;
    ULONG  ConvKey=BaseConvKey;
    USHORT  Cnt;
    WCHAR *Cp;
    WCHAR *Begin;
    USHORT Length;

    if (RemainingName->Length) {
        Cp = RemainingName->Buffer;
        Cnt = RemainingName->Length;

        // Skip the leading OBJ_NAME_PATH_SEPARATOR

        while (*Cp == OBJ_NAME_PATH_SEPARATOR) {
            Cp++;
            Cnt -= sizeof(WCHAR);
        }
        Begin = Cp;
        Length = 0;

        HashStack[TotalRemainingSubkeys].KeyName.Buffer = Cp;

        while (Cnt) {
            if (*Cp == OBJ_NAME_PATH_SEPARATOR) {
                if (TotalRemainingSubkeys < CM_HASH_STACK_SIZE) {
                    HashStack[TotalRemainingSubkeys].ConvKey = ConvKey;
                    //
                    // Due to the changes in KCB structure, we now only have the subkey name
                    // in the kcb (not the full path).  Change the name in the stack to store
                    // the parse element (each subkey) only.
                    //
                    HashStack[TotalRemainingSubkeys].KeyName.Length = Length;
                    Length = 0;
                    TotalRemainingSubkeys++;
                }

                TotalKeys++;

                //
                // Now skip over leading path separators
                // Just in case someone has a RemainingName '..A\\\\B..'
                //
                //
                // We are stripping all OBJ_NAME_PATH_SEPARATOR (The original code keep the first one).
                // so the KeyName.Buffer is set properly.
                //
                while(*Cp == OBJ_NAME_PATH_SEPARATOR) {
                    Cp++;
                    Cnt -= sizeof(WCHAR);
                }
                if (TotalRemainingSubkeys < CM_HASH_STACK_SIZE) {
                    HashStack[TotalRemainingSubkeys].KeyName.Buffer = Cp;
                }

            } else {
                ConvKey = 37 * ConvKey + (ULONG) CmUpcaseUnicodeChar(*Cp);
                //
                // We are stripping all OBJ_NAME_PATH_SEPARATOR in the above code,
                // we should only move to the next char in the else case.
                //
                Cp++;
                Cnt -= sizeof(WCHAR);
                Length += sizeof(WCHAR);
            
            }


        }

        //
        // Since we have stripped off all trailing path separators in CmpParseKey routine,
        // the last char will not be OBJ_NAME_PATH_SEPARATOR.
        //
        if (TotalRemainingSubkeys < CM_HASH_STACK_SIZE) {
            HashStack[TotalRemainingSubkeys].ConvKey = ConvKey;
            HashStack[TotalRemainingSubkeys].KeyName.Length = Length;
            TotalRemainingSubkeys++;
        }
        TotalKeys++;

        (*TotalSubkeys) = TotalKeys;
    }

    return(TotalRemainingSubkeys);
}
NTSTATUS
CmpCacheLookup(
    IN PCM_HASH_ENTRY HashStack,
    IN ULONG TotalRemainingSubkeys,
    OUT ULONG *MatchRemainSubkeyLevel,
    IN OUT PCM_KEY_CONTROL_BLOCK *Kcb,
    OUT PUNICODE_STRING RemainingName,
    OUT PHHIVE *Hive,
    OUT HCELL_INDEX *Cell,
    IN PULONG   OuterStackArray
    )
/*++

Routine Description:

    This routine Search the cache to find the matching path in the Cache.

Arguments:

    HashStack - Array that has the hash value of each level.

    TotalRemainingSubkeys - Total Subkey counts from base.

    MatchRemainSubkeyLevel - Number of Levels in RemaingName 
                             that matches. (0 if not found)

    kcb - Pointer to the kcb of the basename.
          Will be changed to the kcb for the new basename.

    RemainingName - Returns remaining name

    Hive - Returns the hive of the cache entry found (if any)

    Cell - Returns the cell of the cache entry found (if any)

Return Value:

    Status

--*/

{
    LONG i;
    LONG j;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG CurrentLevel;
    PCM_KEY_HASH Current;
    PCM_KEY_CONTROL_BLOCK BaseKcb;
    PCM_KEY_CONTROL_BLOCK CurrentKcb;
    PCM_KEY_CONTROL_BLOCK ParentKcb;
    BOOLEAN Found;
    BOOLEAN LockedExclusive = FALSE;
    PULONG LockedKcbs = NULL;

    BaseKcb = *Kcb;
    //
    // try shared first
    //
    LockedKcbs = CmpBuildAndLockKcbArray(   HashStack,
                                            TotalRemainingSubkeys,
                                            0,
                                            BaseKcb,
                                            OuterStackArray,
                                            FALSE);
    if( LockedKcbs == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ASSERT(BaseKcb->RefCount != 0);
    CmpReferenceKeyControlBlock(BaseKcb);

RetryExclusive:
    Found = FALSE;

    CurrentLevel = TotalRemainingSubkeys + BaseKcb->TotalLevels + 1;

    for(i = TotalRemainingSubkeys-1; i>=0; i--) {
        //
        // Try to find the longest path in the cache.
        //
        // First, find the kcb that match the hash value.
        //

        CurrentLevel--; 

        Current = GET_KCB_HASH_ENTRY(CmpCacheTable, HashStack[i].ConvKey);
        Found = FALSE;

        while (Current) {
            ASSERT_KEY_HASH(Current);

            //
            // Check against both the ConvKey and total levels;
            //
            CurrentKcb = (CONTAINING_RECORD(Current, CM_KEY_CONTROL_BLOCK, KeyHash));

            if (CurrentKcb->TotalLevels == CurrentLevel) {
                //
                // The total subkey levels match.
                // Iterate through the kcb path and compare each subkey.
                //
                Found = TRUE;
                ParentKcb = CurrentKcb;
                for (j=i; j>=0; j--) {
                    if (HashStack[j].ConvKey == ParentKcb->ConvKey) {
                        //
                        // Convkey matches, compare the string
                        //
                        LONG Result;
                        UNICODE_STRING  TmpNodeName;

                        if (ParentKcb->NameBlock->Compressed) {
                               Result = CmpCompareCompressedName(&(HashStack[j].KeyName),
                                                                 ParentKcb->NameBlock->Name, 
                                                                 ParentKcb->NameBlock->NameLength,
                                                                 CMP_DEST_UP // name block is always UPPERCASE!!!
                                                                 ); 
                        } else {
                               TmpNodeName.Buffer = ParentKcb->NameBlock->Name;
                               TmpNodeName.Length = ParentKcb->NameBlock->NameLength;
                               TmpNodeName.MaximumLength = ParentKcb->NameBlock->NameLength;

                               //
                               // use the cmp compare variant as we know the destination is already uppercased.
                               //
                               Result = CmpCompareUnicodeString(&(HashStack[j].KeyName),
                                                                &TmpNodeName, 
                                                                CMP_DEST_UP);
                        }

                        if (Result) {
                            Found = FALSE;
                            break;
                        } 
                        ParentKcb = ParentKcb->ParentKcb;
                    } else {
                        Found = FALSE;
                        break;
                    }
                }
                if (Found) {
                    //
                    // All remaining key matches.  Now compare the BaseKcb.
                    //
                    if (BaseKcb == ParentKcb) {
                        
                        // if neither of these, don't need to ugrade KCB lock
                        if (CurrentKcb->ParentKcb->Delete || CurrentKcb->Delete) {
                            if( !LockedExclusive ) {
                                CmpUnLockKcbArray(LockedKcbs,CmpHashTableSize);
                                if( LockedKcbs != OuterStackArray ) {
                                    ExFreePool(LockedKcbs);
                                }
                                //
                                // try again, this time with EX lock
                                //
                                LockedKcbs = CmpBuildAndLockKcbArray(   HashStack,
                                                                        TotalRemainingSubkeys,
                                                                        0,
                                                                        BaseKcb,
                                                                        OuterStackArray,
                                                                        TRUE);
                                if( LockedKcbs == NULL ) {
                                    CmpDereferenceKeyControlBlock(BaseKcb);
                                    return STATUS_INSUFFICIENT_RESOURCES;
                                }
                                LockedExclusive = TRUE;
                                goto RetryExclusive;
                            }

                            if (CurrentKcb->ParentKcb->Delete) {
                                //
                                // The parentkcb is marked deleted.  
                                // So this must be a fake key created when the parent still existed.
                                // Otherwise it cannot be in the cache
                                //
                                ASSERT (CurrentKcb->ExtFlags & CM_KCB_KEY_NON_EXIST);

                                //
                                // It is possible that the parent key was deleted but now recreated.
                                // In that case this fake key is not longer valid for the ParentKcb is bad.
                                // We must now remove this fake key out of cache so, if this is a
                                // create operation, we do get hit this kcb in CmpCreateKeyControlBlock. 
                                //
                                ASSERT_KCB_LOCKED_EXCLUSIVE(CurrentKcb);
                                if (CurrentKcb->RefCount == 0) {
                                    //
                                    // No one is holding this fake kcb, just delete it.
                                    //
                                    CmpRemoveFromDelayedClose(CurrentKcb);
                                    CmpCleanUpKcbCacheWithLock(CurrentKcb,FALSE);
                                } else {
                                    //
                                    // Someone is still holding this fake kcb, 
                                    // Mark it as delete and remove it out of cache.
                                    //
                                    CurrentKcb->Delete = TRUE;
                                    CmpRemoveKeyControlBlock(CurrentKcb);
                                }
                                Found = FALSE;
                                break;
                            } else if(CurrentKcb->Delete) {
                                //
                                // the key has been deleted, but still kept in the cache for 
                                // this kcb does not belong here
                                //
                                CmpRemoveKeyControlBlock(CurrentKcb);
                                CmpUnLockKcbArray(LockedKcbs,CmpHashTableSize);
                                if( LockedKcbs != OuterStackArray ) {
                                    ExFreePool(LockedKcbs);
                                }
                                CmpDereferenceKeyControlBlock(BaseKcb);
                                return STATUS_OBJECT_NAME_NOT_FOUND;
                            }
                        }
                        
                        //
                        // We have a match, update the RemainingName.
                        //

                        //
                        // Skip the leading OBJ_NAME_PATH_SEPARATOR
                        //
                        while ((RemainingName->Length > 0) &&
                               (RemainingName->Buffer[0] == OBJ_NAME_PATH_SEPARATOR)) {
                            RemainingName->Buffer++;
                            RemainingName->Length -= sizeof(WCHAR);
                        }

                        //
                        // Skip all subkeys plus OBJ_NAME_PATH_SEPARATOR
                        //
                        for(j=0; j<=i; j++) {
                            RemainingName->Buffer += HashStack[j].KeyName.Length/sizeof(WCHAR);
                            RemainingName->Length = RemainingName->Length - (USHORT)(HashStack[j].KeyName.Length);
                            //
                            // Skip the leading OBJ_NAME_PATH_SEPARATOR 
                            // loop if some dumb caller decided to double quote
                            //
                            while ((RemainingName->Length > 0) &&
                                   (RemainingName->Buffer[0] == OBJ_NAME_PATH_SEPARATOR)) {
                                RemainingName->Buffer++;
                                RemainingName->Length -= sizeof(WCHAR);
                            }
                        }
                        //
                        // unlock all BUT this kcb; then reference it with the lock shared (safe even if it's in the delay close)
                        //
                        CmpUnLockKcbArray(LockedKcbs,GET_HASH_INDEX(CurrentKcb->ConvKey));
                        if( LockedKcbs != OuterStackArray ) {
                            ExFreePool(LockedKcbs);
                        }
                        LockedKcbs = NULL;
                        CmpReferenceKeyControlBlock(CurrentKcb);
                        ASSERT_KCB_LOCKED(CurrentKcb);
                        CmpUnlockKCB(CurrentKcb);
                        CmpDereferenceKeyControlBlock(BaseKcb);
                        //
                        // Update the KCB, Hive and Cell.
                        //
                        *Kcb = CurrentKcb;
                        *Hive = CurrentKcb->KeyHive;
                        *Cell = CurrentKcb->KeyCell;
                        break;
                    } else {
                        Found = FALSE;
                    }
                }
            }
            Current = Current->NextHash;
        }

        if (Found) {
            break;
        }
    }
    if( LockedKcbs != NULL ) {
        CmpUnLockKcbArray(LockedKcbs,CmpHashTableSize);
        if( LockedKcbs != OuterStackArray ) {
            ExFreePool(LockedKcbs);
        }
    }
    CmpLockKCBShared(*Kcb);

    if((*Kcb)->Delete) {
        //
        // the key has been deleted, but still kept in the cache for 
        // this kcb does not belong here
        //
        CmpUnlockKCB(*Kcb);
        CmpDereferenceKeyControlBlock( *Kcb );
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    if( (LONG)TotalRemainingSubkeys != (i+1) ) {
        CmpUnlockKCB(*Kcb);
    } else {
        ASSERT_KCB_LOCKED(*Kcb);
        //
        // check if not a false hit (non-existent key).
        //
        if( (*Kcb)->ExtFlags & CM_KCB_KEY_NON_EXIST ) {
            CmpUnlockKCB(*Kcb);
            OuterStackArray[0] = 0;
        } else {
            //
            // this is the fast path. the caller will know we have already locked the kcb by looking in the 
            // OuterKcbArray and determining Count == 1
            //
            ASSERT( NT_SUCCESS(status) );
            ASSERT( OuterStackArray );
            OuterStackArray[0] = 1;
            OuterStackArray[1] = GET_HASH_INDEX((*Kcb)->ConvKey);
        }
    }

    //
    // Now the kcb will be used in the parse routine.
    // Increase its reference count.
    // Make sure we remember to dereference it at the parse routine.
    //
    //
    // Don't need to do that since we already have a refcount added on this puppy 
    //
    //if (!CmpReferenceKeyControlBlock(*Kcb)) {
    //    status = STATUS_INSUFFICIENT_RESOURCES;
    //}
    *MatchRemainSubkeyLevel = i+1;
    return status;
}


PCM_KEY_CONTROL_BLOCK
CmpAddInfoAfterParseFailure(
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    PCM_KEY_NODE    Node,
    PCM_KEY_CONTROL_BLOCK kcb,
    PUNICODE_STRING NodeName
    )
/*++

Routine Description:

    This routine builds up further information in the cache when parse
    fails.  The additional information can be
    1. The key is has no subkey (CM_KCB_NO_SUBKEY).
    2. The key has a few subkeys, then build the index hint in the cache.
    3. If lookup failed even we have index hint cached, then create a fake key so
       we do not fail again.   This is very useful for lookup failure under keys like
       \registry\machine\software\classes\clsid, which have 1500+ subkeys and lots of
       them have the smae first four chars.
       
       NOTE. Currently we are not seeing too many fake keys being created.
       We need to monitor this periodly and work out a way to work around if
       we do create too many fake keys. 
       One solution is to use hash value for index hint (We can do it in the cache only
       if we need to be backward comparable).
    
Arguments:

    Hive - Supplies Hive that holds the key we are creating a KCB for.

    Cell - Supplies Cell that contains the key we are creating a KCB for.

    Node - Supplies pointer to key node.

    KeyName - The KeyName.

Return Value:

    The KCB that CmpParse need to dereference at the end.

    If resources problem, it returns NULL, and the caller is responsible for cleanup
--*/
{

    ULONG                   TotalSubKeyCounts;
    BOOLEAN                 CreateFakeKcb = FALSE;
    BOOLEAN                 HintCached;
    PCM_KEY_CONTROL_BLOCK   ParentKcb;
    USHORT                  i,j;
    HCELL_INDEX             CellToRelease;
    ULONG                   HashKey;

    if (!UseFastIndex(Hive)) {
        //
        // Older version of hive, do not bother to cache hint.
        //
        return (kcb);
    }

    ASSERT_KCB_LOCKED_EXCLUSIVE(kcb);
    TotalSubKeyCounts = Node->SubKeyCounts[Stable] + Node->SubKeyCounts[Volatile];

    if (TotalSubKeyCounts == 0) {
        kcb->ExtFlags |= CM_KCB_NO_SUBKEY;
        // clean up the invalid flag (if any)
        kcb->ExtFlags &= ~CM_KCB_INVALID_CACHED_INFO;
    } else if (TotalSubKeyCounts == 1) {
        if (!(kcb->ExtFlags & CM_KCB_SUBKEY_ONE)) {
            //
            // Build the subkey hint to avoid unnecessary lookups in the index leaf
            //
            PCM_KEY_INDEX   Index;
            HCELL_INDEX     SubKeyCell = 0;
            PCM_KEY_NODE    SubKeyNode;
            UNICODE_STRING  TmpStr;

            if (Node->SubKeyCounts[Stable] == 1) {
                CellToRelease = Node->SubKeyLists[Stable];
                Index = (PCM_KEY_INDEX)HvGetCell(Hive, CellToRelease);
            } else {
                CellToRelease = Node->SubKeyLists[Volatile];
                Index = (PCM_KEY_INDEX)HvGetCell(Hive, CellToRelease);
            } 
            
            if( Index == NULL ) {
                //
                // we couldn't map the bin containing this cell
                // return NULL; The caller must handle this gracefully!
                //
                return NULL;
            }

            if( Index->Signature == CM_KEY_INDEX_ROOT ) {
                //
                // don't cache root indexes; they are too big
                //
                HvReleaseCell(Hive,CellToRelease);
                return NULL;
            }

            HashKey = 0;
            if ( Index->Signature == CM_KEY_HASH_LEAF ) {
                PCM_KEY_FAST_INDEX FastIndex;
                FastIndex = (PCM_KEY_FAST_INDEX)Index;
                //
                // we already have the hash key handy; preserve it for the kcb hint
                //
                HashKey = FastIndex->List[0].HashKey;
                SubKeyCell = FastIndex->List[0].Cell;
            } else if(Index->Signature == CM_KEY_FAST_LEAF) {
                PCM_KEY_FAST_INDEX FastIndex;
                FastIndex = (PCM_KEY_FAST_INDEX)Index;
                SubKeyCell = FastIndex->List[0].Cell;

            } else {
                SubKeyCell = Index->List[0];
            }
            
            if( HashKey != 0 ) {
                kcb->HashKey = HashKey;
                kcb->ExtFlags |= CM_KCB_SUBKEY_ONE;
                // clean up the invalid flag (if any)
                kcb->ExtFlags &= ~CM_KCB_INVALID_CACHED_INFO;
            } else {
                SubKeyNode = (PCM_KEY_NODE)HvGetCell(Hive,SubKeyCell);
                if( SubKeyNode != NULL ) {
                    if (SubKeyNode->Flags & KEY_COMP_NAME) {
                        kcb->HashKey = CmpComputeHashKeyForCompressedName(0,SubKeyNode->Name,SubKeyNode->NameLength);
                    } else {
                        TmpStr.Buffer = SubKeyNode->Name;
                        TmpStr.Length = SubKeyNode->NameLength;
                        kcb->HashKey = CmpComputeHashKey(0,&TmpStr
#if DBG
                                                        , FALSE
#endif
                            );
                    }
                
                    
                    HvReleaseCell(Hive,SubKeyCell);
                    kcb->ExtFlags |= CM_KCB_SUBKEY_ONE;
                    // clean up the invalid flag (if any)
                    kcb->ExtFlags &= ~CM_KCB_INVALID_CACHED_INFO;
                } else {
                    //
                    // we couldn't map the bin containing this cell
                    // return NULL; The caller must handle this gracefully!
                    //
                    HvReleaseCell(Hive,CellToRelease);
                    return NULL;
                }
            }
            HvReleaseCell(Hive,CellToRelease);
        } else {
            //
            // The name hint does not prevent from this look up
            // Create the fake Kcb.
            //
            CreateFakeKcb = TRUE;
        }
    } else if (TotalSubKeyCounts < CM_MAX_CACHE_HINT_SIZE) {
        if (!(kcb->ExtFlags & CM_KCB_SUBKEY_HINT)) {
            //
            // Build the index leaf info in the parent KCB
            // How to sync the cache with the registry data is a problem to be resolved.
            //
            ULONG               Size;
            PCM_KEY_INDEX       Index;
            PCM_KEY_FAST_INDEX  FastIndex;
            HCELL_INDEX         SubKeyCell = 0;
            PCM_KEY_NODE        SubKeyNode;
            ULONG               HintCrt;
            UNICODE_STRING      TmpStr;

            Size = sizeof(ULONG) * (Node->SubKeyCounts[Stable] + Node->SubKeyCounts[Volatile] + 1);

            kcb->IndexHint = ExAllocatePoolWithTag(PagedPool,
                                                   Size,
                                                   CM_CACHE_INDEX_TAG | PROTECTED_POOL);

            HintCached = TRUE;
            if (kcb->IndexHint) {
                kcb->IndexHint->Count = Node->SubKeyCounts[Stable] + Node->SubKeyCounts[Volatile]; 

                HintCrt = 0;

                for (i = 0; i < Hive->StorageTypeCount; i++) {
                    if(Node->SubKeyCounts[i]) {
                        CellToRelease = Node->SubKeyLists[i];
                        Index = (PCM_KEY_INDEX)HvGetCell(Hive, CellToRelease);
                        if( Index == NULL ) {
                            //
                            // we couldn't map the bin containing this cell
                            // return NULL; The caller must handle this gracefully!
                            //
                            return NULL;
                        }
                        if( Index->Signature == CM_KEY_INDEX_ROOT ) {
                            HvReleaseCell(Hive,CellToRelease);
                            HintCached = FALSE;
                            break;
                        } else {
                          
                            for (j=0; j<Node->SubKeyCounts[i]; j++) {
                                HashKey = 0;

                                if ( Index->Signature == CM_KEY_HASH_LEAF ) {
                                    FastIndex = (PCM_KEY_FAST_INDEX)Index;
                                    //
                                    // preserve the hash key for the kcb hint
                                    //
                                    HashKey = FastIndex->List[j].HashKey;
                                    SubKeyCell = FastIndex->List[j].Cell;
                                } else if( Index->Signature == CM_KEY_FAST_LEAF ) {
                                    FastIndex = (PCM_KEY_FAST_INDEX)Index;
                                    SubKeyCell = FastIndex->List[j].Cell;
                                } else {
                                    SubKeyCell = Index->List[j];
                                }
                            
                                if( HashKey != 0 ) {
                                    kcb->IndexHint->HashKey[HintCrt] = HashKey;
                                } else {
                                    SubKeyNode = (PCM_KEY_NODE)HvGetCell(Hive,SubKeyCell);
                                    if( SubKeyNode == NULL ) {
                                        //
                                        // couldn't map view; bad luck; don't cache hint for this kcb
                                        //
                                        HintCached = FALSE;
                                        break;
                                    }

                                    if (SubKeyNode->Flags & KEY_COMP_NAME) {
                                        kcb->IndexHint->HashKey[HintCrt] = CmpComputeHashKeyForCompressedName(0,SubKeyNode->Name,SubKeyNode->NameLength);
                                    } else {
                                        TmpStr.Buffer = SubKeyNode->Name;
                                        TmpStr.Length = SubKeyNode->NameLength;
                                        kcb->IndexHint->HashKey[HintCrt] = CmpComputeHashKey(0,&TmpStr
#if DBG
                                                                                            , FALSE
#endif
                                            );
                                    }

                                    HvReleaseCell(Hive,SubKeyCell);
                                }
                                //
                                // advance to the new hint
                                //
                                HintCrt++;
                            
                            }
                        }

                        HvReleaseCell(Hive,CellToRelease);
                    }
                }

                if (HintCached) {
                    kcb->ExtFlags |= CM_KCB_SUBKEY_HINT;
                    // clean up the invalid flag (if any)
                    kcb->ExtFlags &= ~CM_KCB_INVALID_CACHED_INFO;
                } else {
                    //
                    // Do not have a FAST_LEAF, free the allocation.
                    //
                    ExFreePoolWithTag(kcb->IndexHint, CM_CACHE_INDEX_TAG | PROTECTED_POOL);
                }
            }
        } else {
            //
            // The name hint does not prevent from this look up
            // Create the fake Kcb.
            //
            CreateFakeKcb = TRUE;
        }
    } else {
        CreateFakeKcb = TRUE;
    }

    ParentKcb = kcb;

    if (CreateFakeKcb && (CmpCacheOnFlag & CM_CACHE_FAKE_KEY)) {
        //
        // It has more than a few children but not the one we are interested.
        // Create a kcb for this non-existing key so we do not try to find it
        // again. Use the cell and node from the parent.
        //
        // Before we create a new one. Dereference the current kcb.
        //
        // CmpCacheOnFlag is for us to turn it on/off easily.
        //

        kcb = CmpCreateKeyControlBlock(Hive,
                                       Cell,
                                       Node,
                                       ParentKcb,
                                       CMP_CREATE_KCB_FAKE|CMP_CREATE_KCB_KCB_LOCKED,
                                       NodeName);

        if (kcb) {
            ASSERT_KCB_LOCKED_EXCLUSIVE(ParentKcb);
            CmpDereferenceKeyControlBlockWithLock(ParentKcb,FALSE);
            ParentKcb = kcb;
        }
    }

    return (ParentKcb);
}

BOOLEAN
CmpOKToFollowLink(  IN PCMHIVE  OrigHive,
                    IN PCMHIVE  DestHive
                    )
/*++

Routine Description:

    1.      You can follow links from TRUSTED to anywhere. 
    2.      You cannot follow links from UNTRUSTED to TRUSTED. 
    3.      Inside the UNTRUSTED name space, you can only follow links inside the same class of trust. 

    OBS: OrigHive is just an address. It should not be dereferenced, as it may not be valid anymore (ie.
            hive could have been unloaded in between). We don't really care as if it was, it will not be 
            in the trust list for the SourceKcb's hive
         If OrigHive == NULL it means we have originated in a trusted hive.
  
Arguments:


Return Value:

    TRUE or FALSE
--*/
{
    PCMHIVE     TmpHive;
    PLIST_ENTRY AnchorAddr;
    
    PAGED_CODE();

    if( OrigHive == NULL ) {
        //
        // OK to follow links from trusted to anywhere
        //
        return TRUE;
    }
    if( OrigHive == DestHive ) {
        //
        // OK to follow links inside the same hive 
        //
        return TRUE;
    }
    
    if( !(DestHive->Flags & CM_CMHIVE_FLAG_UNTRUSTED) ) {
        //
        // fail to follow from untrusted to trusted
        //
        //return FALSE;
        goto Fail;
    }
    //
    // both untrusted; see if they are in the same class of trust
    //
    ASSERT( DestHive->Flags & CM_CMHIVE_FLAG_UNTRUSTED );

    CmpLockHiveListShared();
    //
	// walk the TrustClassEntry list of SrcHive, to see if we can find DstSrc
	//
	AnchorAddr = &(DestHive->TrustClassEntry);
	TmpHive = (PCMHIVE)(DestHive->TrustClassEntry.Flink);

	while ( TmpHive != (PCMHIVE)AnchorAddr ) {
		TmpHive = CONTAINING_RECORD(
						TmpHive,
						CMHIVE,
						TrustClassEntry
						);
		if( TmpHive == OrigHive ) {
			//
			// found it ==> same class of trust
			//
            CmpUnlockHiveList();
            return TRUE;
		}
        //
        // skip to the next element
        //
        TmpHive = (PCMHIVE)(TmpHive->TrustClassEntry.Flink);
	}

    CmpUnlockHiveList();

Fail:
    //
    // returning TRUE here will disable the 'don't follow links outside class of trust' behavior
    //
    return FALSE;
}

PULONG 
CmpBuildAndLockKcbArray(   
    IN PCM_HASH_ENTRY           HashStack,
    IN ULONG                    TotalLevels,
    IN ULONG                    RemainingLevel,
    IN PCM_KEY_CONTROL_BLOCK    kcb,
    IN PULONG                   OuterStackArray,
    IN BOOLEAN                  LockExclusive)
/*++

Routine Description:

    Builds up an array with indexes associated to the hash keys in hash stack starting at RemainingLevel
    then locks all hash entries exclusive
    The array is sorted.
    If RemainingLevel == TotalLevels, kcb->Parent is also locked
    kcb is always locked
  
Arguments:

    HashStack - 
    TotalLevels -
    RemainingLevel -
    Kcb -


Return Value:

    The array, first ULONG is the number of valid elements  in the success case

  - NULL on failure.
--*/
{
    PULONG  IndexArray;
    ULONG   Count;
    ULONG   Current;
    ULONG   HashIndex;
    ULONG   i,k;
    ULONG   InputArray[MAX_LOCAL_KCB_ARRAY];
    PULONG  OutputArray;
    char    OrderArray[MAX_LOCAL_KCB_ARRAY] = {0};

    CM_PAGED_CODE();

    Count = 1; // this kcb
    if( (TotalLevels == RemainingLevel) && (kcb->ParentKcb != NULL) ) {
        //
        // need to lock the parent also
        //
        Count++;
    }
    //
    // now add up the remaining levels.
    //
    Current = 0;
    Count += (TotalLevels - RemainingLevel);
    
    ASSERT( Count <= MAX_LOCAL_KCB_ARRAY);

    //
    // take the fast path and use the array we have handy on the stack.
    //
    ASSERT( OuterStackArray != NULL );
    IndexArray = OuterStackArray;
    OutputArray = &(IndexArray[1]);
    //
    // quick sort them on the fly with an auxiliary array.
    //

    //
    // add parent if needed
    //
    if( (TotalLevels == RemainingLevel) && (kcb->ParentKcb != NULL) ) {
        InputArray[Current++] = GET_HASH_INDEX(kcb->ParentKcb->ConvKey);
    }
    //
    // add this kcb
    //
    HashIndex = GET_HASH_INDEX(kcb->ConvKey);
    if( Current == 1) {
        if( HashIndex != InputArray[0] ) {
            //
            // not same
            //
            InputArray[Current++] = HashIndex;
            if( HashIndex < InputArray[0] ) {
                OrderArray[0]++;
            } else {
                ASSERT( HashIndex > InputArray[0] );
                OrderArray[1]++;
            }
        }
    } else {
        ASSERT( Current == 0 );
        //
        // simply store it
        //
        InputArray[Current++] = HashIndex;
    }

    //
    // now the fun part; parse the input array and sort the output as we go
    //
    for(;RemainingLevel<TotalLevels;RemainingLevel++) {
        InputArray[Current] = GET_HASH_INDEX(HashStack[RemainingLevel].ConvKey);
        for(i=0;i<Current;i++) {
            if(InputArray[Current] == InputArray[i]) {
                //
                // need to go undo what we have touched already
                //
                for( k=0;k<i;k++) {
                    if( InputArray[Current] < InputArray[k] ) {
                        OrderArray[k]--;
                    }
                }
                OrderArray[Current] = 0;
                break;
            }

            if( InputArray[Current] < InputArray[i] ) {
                OrderArray[i]++;
            } else {
                OrderArray[Current]++;
            }
        }

        if( i == Current ) {
            Current++;
        }
    }
    //
    // now sort it up nicely
    //
    for(i=0;i<Current;i++) {
        OutputArray[OrderArray[i]] = InputArray[i];
    }

    //
    // remember how many they are;
    //
    IndexArray[0] = Current;
#if DBG
    //
    // did we sort them right ?
    //
    for( i=0;i<(Current-1);i++) {
        ASSERT( IndexArray[i+1] < IndexArray[i+2] );
    }
#endif
    //
    // now lock'em up;
    //
    for(i=0;i<Current;i++) {
        if( LockExclusive ) {
            CmpLockHashEntryByIndexExclusive(IndexArray[i+1]);
        } else {
            CmpLockHashEntryByIndexShared(IndexArray[i+1]);
        }
    }
    
    return IndexArray;
}

ULONG
CmpUnLockKcbArray(IN PULONG LockedKcbs,
                  IN ULONG  Exempt)
{
    ULONG   i;
    ULONG   Count;

    CM_PAGED_CODE();

    Count = LockedKcbs[0];
    for( i=Count;i>0;i--) {
        if(LockedKcbs[i] != Exempt) {
            CmpUnlockHashEntryByIndex(LockedKcbs[i]);
        }
    }
    LockedKcbs[0] = 0;
    return Count;
}

VOID
CmpReLockKcbArray(IN PULONG LockedKcbs,
                  IN BOOLEAN LockExclusive)
{
    ULONG   i;
    ULONG   Count;

    CM_PAGED_CODE();

    Count = LockedKcbs[0];

    for(i=0;i<Count;i++) {
        if( LockExclusive ) {
            CmpLockHashEntryByIndexExclusive(LockedKcbs[i+1]);
        } else {
            CmpLockHashEntryByIndexShared(LockedKcbs[i+1]);
        }
    }
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

