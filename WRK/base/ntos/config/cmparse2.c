/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmparse2.c

Abstract:

    This module contains parse routines for the configuration manager, particularly
    the registry.

--*/

#include    "cmp.h"

BOOLEAN
CmpOKToFollowLink(  IN PCMHIVE  OrigHive,
                    IN PCMHIVE  DestHive
                    );

ULONG
CmpUnLockKcbArray(IN PULONG LockedKcbs,
                  IN ULONG  Exempt);

VOID
CmpReLockKcbArray(IN PULONG LockedKcbs,
                  IN BOOLEAN LockExclusive);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpDoCreate)
#pragma alloc_text(PAGE,CmpDoCreateChild)
#endif

extern  PCM_KEY_CONTROL_BLOCK CmpKeyControlBlockRoot;


NTSTATUS
CmpDoCreate(
    IN PHHIVE                   Hive,
    IN HCELL_INDEX              Cell,
    IN PACCESS_STATE            AccessState,
    IN PUNICODE_STRING          Name,
    IN KPROCESSOR_MODE          AccessMode,
    IN PCM_PARSE_CONTEXT        Context,
    IN PCM_KEY_CONTROL_BLOCK    ParentKcb,
    IN PCMHIVE                  OriginatingHive OPTIONAL,
    OUT PVOID                   *Object
    )
/*++

Routine Description:

    Performs the first step in the creation of a registry key.  This
    routine checks to make sure the caller has the proper access to
    create a key here, and allocates space for the child in the parent
    cell.  It then calls CmpDoCreateChild to initialize the key and
    create the key object.

    This two phase creation allows us to share the child creation code
    with the creation of link nodes.

Arguments:

    Hive - supplies a pointer to the hive control structure for the hive

    Cell - supplies index of node to create child under.

    AccessState - Running security access state information for operation.

    Name - supplies pointer to a UNICODE string which is the name of
            the child to be created.

    AccessMode - Access mode of the original caller.

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
    NTSTATUS                status;
    PCELL_DATA              pdata;
    HCELL_INDEX             KeyCell;
    ULONG                   ParentType;
    ACCESS_MASK             AdditionalAccess;
    BOOLEAN                 CreateAccess;
    PCM_KEY_BODY            KeyBody;
    PSECURITY_DESCRIPTOR    SecurityDescriptor;
    LARGE_INTEGER           TimeStamp;
    BOOLEAN                 BackupRestore;
    KPROCESSOR_MODE         mode;
    PCM_KEY_NODE            ParentNode;
#if DBG
    ULONG                   ChildConvKey;
#endif
    HV_TRACK_CELL_REF       CellRef = {0};

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpDoCreate:\n"));

    BackupRestore = FALSE;
    if (ARGUMENT_PRESENT(Context)) {

        if (Context->CreateOptions & REG_OPTION_BACKUP_RESTORE) {

            //
            // allow backup operators to create new keys
            //
            BackupRestore = TRUE;
        }

        //
        // Operation is a create, so set Disposition
        //
        Context->Disposition = REG_CREATED_NEW_KEY;
    }

#if DBG
    ChildConvKey = ParentKcb->ConvKey;

    if (Name->Length) {
        ULONG                   Cnt;
        WCHAR                   *Cp;
        Cp = Name->Buffer;
        for (Cnt=0; Cnt<Name->Length; Cnt += sizeof(WCHAR)) {
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
    CmpLockHiveFlusherShared((PCMHIVE)Hive);

    if( CmIsKcbReadOnly(ParentKcb) ) {
        //
        // key is protected
        //
        status = STATUS_ACCESS_DENIED;
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
        status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto Exit;
    }

    //
    // KeQuerySystemTime doesn't give us a fine resolution
    // so we have to search if the child has not been created already
    //
    ParentNode = (PCM_KEY_NODE)HvGetCell(Hive, Cell);
    if( ParentNode == NULL ) {
        //
        // we couldn't map the bin containing this cell
        //
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    if( !HvTrackCellRef(&CellRef,Hive,Cell) ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    if( CmpFindSubKeyByName(Hive,ParentNode,Name) != HCELL_NIL ) {
        //
        // key was changed in between; possibly this key was already created ==> reparse
        //
        status = STATUS_REPARSE;
        goto Exit;
    }
    
    if(!CmpOKToFollowLink(OriginatingHive,(PCMHIVE)Hive) ) {
        //
        // about to cross class of trust boundary
        //
        status = STATUS_ACCESS_DENIED;
        goto Exit;
    }

    ASSERT( Cell == ParentKcb->KeyCell );

    ASSERT( ParentKcb->CachedSecurity != NULL );
    SecurityDescriptor = &(ParentKcb->CachedSecurity->Descriptor);

    ParentType = HvGetCellType(Cell);

    if ( (ParentType == Volatile) &&
         ((Context->CreateOptions & REG_OPTION_VOLATILE) == 0) )
    {
        //
        // Trying to create stable child under volatile parent, report error
        //
        status = STATUS_CHILD_MUST_BE_VOLATILE;
        goto Exit;
    }

    if (ParentKcb->Flags &   KEY_SYM_LINK) {
        //
        // Disallow attempts to create anything under a symbolic link
        //
        status = STATUS_ACCESS_DENIED;
        goto Exit;
    }

    AdditionalAccess = (Context->CreateOptions & REG_OPTION_CREATE_LINK) ? KEY_CREATE_LINK : 0;

    if( BackupRestore == TRUE ) {
        //
        // this is a create to support a backup or restore
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
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpDoCreate for backup restore: access denied\n"));
            status = STATUS_ACCESS_DENIED;
            //
            // this is not a backup-restore operator; deny the create
            //
            CreateAccess = FALSE;
        } else {
            //
            // allow backup operators to create new keys
            //
            status = STATUS_SUCCESS;
            CreateAccess = TRUE;
        }

    } else {
        //
        // The FullName is not used in the routine CmpCheckCreateAccess,
        //
        CreateAccess = CmpCheckCreateAccess(NULL,
                                            SecurityDescriptor,
                                            AccessState,
                                            AccessMode,
                                            AdditionalAccess,
                                            &status);
    }

    if (CreateAccess) {

        //
        // Security check passed, so we can go ahead and create
        // the sub-key.
        //
        if ( !HvMarkCellDirty(Hive, Cell,FALSE) ) {
            status = STATUS_NO_LOG_SPACE;
            goto Exit;
        }
        //
        // Create and initialize the new sub-key
        //
        status = CmpDoCreateChild( Hive,
                                   Cell,
                                   SecurityDescriptor,
                                   AccessState,
                                   Name,
                                   AccessMode,
                                   Context,
                                   ParentKcb,
                                   0,
                                   &KeyCell,
                                   Object );

        if (NT_SUCCESS(status)) {
            PCM_KEY_NODE KeyNode;

            KeyBody = (PCM_KEY_BODY)(*Object);

            //
            // Child successfully created, add to parent's list.
            //
            if (! CmpAddSubKey(Hive, Cell, KeyCell)) {
                //
                // Unable to add child, so free it
                //
                CmpFreeKeyByCell(Hive, KeyCell, FALSE);
                //
                // release the object created inside CmpDoCreateChild make sure kcb gets kicked out of cache
                //
                KeyBody->KeyControlBlock->Delete = TRUE;
                CmpRemoveKeyControlBlock(KeyBody->KeyControlBlock);
                ObDereferenceObjectDeferDelete(*Object);
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Exit;
            }

            KeyNode =  (PCM_KEY_NODE)HvGetCell(Hive, Cell);
            if( KeyNode == NULL ) {
                //
                // we couldn't map the bin containing this cell
                // this shouldn't happen as we successfully marked the cell as dirty
                //
                ASSERT( FALSE );
                CmpFreeKeyByCell(Hive, KeyCell, TRUE);
                KeyBody->KeyControlBlock->Delete = TRUE;
                CmpRemoveKeyControlBlock(KeyBody->KeyControlBlock);
                ObDereferenceObjectDeferDelete(*Object);
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Exit;
            }

            if( !HvTrackCellRef(&CellRef,Hive,Cell) ) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Exit;
            }

            CmpCleanUpSubKeyInfo (KeyBody->KeyControlBlock->ParentKcb);

            //
            // Update max keyname and class name length fields
            //

            //some sanity asserts first
            ASSERT( KeyBody->KeyControlBlock->ParentKcb->KeyCell == Cell );
            ASSERT( KeyBody->KeyControlBlock->ParentKcb->KeyHive == Hive );
            ASSERT( KeyBody->KeyControlBlock->ParentKcb == ParentKcb );
            ASSERT( KeyBody->KeyControlBlock->ParentKcb->KcbMaxNameLen == KeyNode->MaxNameLen );

            //
            // update the LastWriteTime on both keynode and kcb;
            //
            KeQuerySystemTime(&TimeStamp);
            KeyNode->LastWriteTime = TimeStamp;
            KeyBody->KeyControlBlock->ParentKcb->KcbLastWriteTime = TimeStamp;

            if (KeyNode->MaxNameLen < Name->Length) {
                KeyNode->MaxNameLen = Name->Length;
                // update the kcb cache too
                KeyBody->KeyControlBlock->ParentKcb->KcbMaxNameLen = Name->Length;
            }

            if (KeyNode->MaxClassLen < Context->Class.Length) {
                KeyNode->MaxClassLen = Context->Class.Length;
            }


            if (Context->CreateOptions & REG_OPTION_CREATE_LINK) {
                pdata = HvGetCell(Hive, KeyCell);
                if( pdata == NULL ) {
                    //
                    // we couldn't map the bin containing this cell
                    // this shouldn't happen as we just allocated the cell
                    // (i.e. it must be PINNED into memory at this point)
                    //
                    ASSERT( FALSE );
                    CmpFreeKeyByCell(Hive, KeyCell, TRUE);
                    KeyBody->KeyControlBlock->Delete = TRUE;
                    CmpRemoveKeyControlBlock(KeyBody->KeyControlBlock);
                    ObDereferenceObjectDeferDelete(*Object);
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto Exit;
                }

                pdata->u.KeyNode.Flags |= KEY_SYM_LINK;
                KeyBody->KeyControlBlock->Flags = pdata->u.KeyNode.Flags;
                HvReleaseCell(Hive,KeyCell);

            }
		}
    }
Exit:
    HvReleaseFreeCellRefArray(&CellRef);
    CmpUnlockHiveFlusher((PCMHIVE)Hive);
    return status;
}


NTSTATUS
CmpDoCreateChild(
    IN PHHIVE Hive,
    IN HCELL_INDEX ParentCell,
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PACCESS_STATE AccessState,
    IN PUNICODE_STRING Name,
    IN KPROCESSOR_MODE AccessMode,
    IN PCM_PARSE_CONTEXT Context,
    IN PCM_KEY_CONTROL_BLOCK ParentKcb,
    IN USHORT Flags,
    OUT PHCELL_INDEX KeyCell,
    OUT PVOID *Object
    )

/*++

Routine Description:

    Creates a new sub-key.  This is called by CmpDoCreate to create child
    sub-keys and CmpCreateLinkNode to create root sub-keys.

Arguments:

    Hive - supplies a pointer to the hive control structure for the hive

    ParentCell - supplies cell index of parent cell

    ParentDescriptor - Supplies security descriptor of parent key, for use
           in inheriting ACLs.

    AccessState - Running security access state information for operation.

    Name - Supplies pointer to a UNICODE string which is the name of the
           child to be created.

    AccessMode - Access mode of the original caller.

    Context - Supplies pointer to CM_PARSE_CONTEXT structure passed through
           the object manager.

    BaseName - Name of object create is relative to

    KeyName - Relative name (to BaseName)

    Flags - Supplies any flags to be set in the newly created node

    KeyCell - Receives the cell index of the newly created sub-key, if any.

    Object - Receives a pointer to the created key object, if any.

Return Value:

    STATUS_SUCCESS - sub-key successfully created.  New object is returned in
            Object, and the new cell's cell index is returned in KeyCell.

    !STATUS_SUCCESS - appropriate error message.

--*/

{
    ULONG alloc=0;
    NTSTATUS Status = STATUS_SUCCESS;
    PCM_KEY_BODY KeyBody;
    HCELL_INDEX ClassCell=HCELL_NIL;
    PCM_KEY_NODE KeyNode;
    PCELL_DATA CellData;
    PCM_KEY_CONTROL_BLOCK kcb = NULL;
    ULONG StorageType;
    PSECURITY_DESCRIPTOR NewDescriptor = NULL;
    LARGE_INTEGER systemtime;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpDoCreateChild:\n"));

    //
    // Get allocation type
    //
    StorageType = Stable;

    try {

        if (Context->CreateOptions & REG_OPTION_VOLATILE) {
            StorageType = Volatile;
        }

        //
        // Allocate child cell
        //
        *KeyCell = HvAllocateCell(
                        Hive,
                        CmpHKeyNodeSize(Hive, Name),
                        StorageType,
                        HCELL_NIL
                        );
        if (*KeyCell == HCELL_NIL) {
			Status = STATUS_INSUFFICIENT_RESOURCES;
			leave;
        }
        alloc = 1;
        KeyNode = (PCM_KEY_NODE)HvGetCell(Hive, *KeyCell);
        if( KeyNode == NULL ) {
            //
            // we couldn't map the bin containing this cell
            // this shouldn't happen as we just allocated the cell
            // (i.e. it must be PINNED into memory at this point)
            //
            ASSERT( FALSE );
			Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }
        // release the cell right here as the view is pinned
        HvReleaseCell(Hive,*KeyCell);

        //
        // Allocate cell for class name
        //
        if (Context->Class.Length > 0) {
            ClassCell = HvAllocateCell(Hive, Context->Class.Length, StorageType,*KeyCell);
            if (ClassCell == HCELL_NIL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
				leave;
            }
        }
        alloc = 2;
        //
        // Allocate the object manager object
        //
        Status = ObCreateObject(AccessMode,
                                CmpKeyObjectType,
                                NULL,
                                AccessMode,
                                NULL,
                                sizeof(CM_KEY_BODY),
                                0,
                                0,
                                Object);

        if (NT_SUCCESS(Status)) {

            KeyBody = (PCM_KEY_BODY)(*Object);

            //
            // We have managed to allocate all of the objects we need to,
            // so initialize them
            //

            //
            // Mark the object as uninitialized (in case we get an error too soon)
            //
            KeyBody->Type = KEY_BODY_TYPE;
            KeyBody->KeyControlBlock = NULL;

            //
            // Fill in the class name
            //
            if (Context->Class.Length > 0) {

                CellData = HvGetCell(Hive, ClassCell);
                if( CellData == NULL ) {
                    //
                    // we couldn't map the bin containing this cell
                    // this shouldn't happen as we just allocated the cell
                    // (i.e. it must be PINNED into memory at this point)
                    //
                    ASSERT( FALSE );
			        Status = STATUS_INSUFFICIENT_RESOURCES;
                    ObDereferenceObject(*Object);
                    leave;
                }

                // release the cell right here as the view is pinned (cell is dirty).
                HvReleaseCell(Hive,ClassCell);

                try {

                    RtlCopyMemory(
                        &(CellData->u.KeyString[0]),
                        Context->Class.Buffer,
                        Context->Class.Length
                        );

                } except(EXCEPTION_EXECUTE_HANDLER) {
                    ObDereferenceObjectDeferDelete(*Object);
                    Status = GetExceptionCode();
                    leave;
                }
            }

            //
            // Fill in the new key itself
            //
            KeyNode->Signature = CM_KEY_NODE_SIGNATURE;
            KeyNode->Flags = Flags;

            KeQuerySystemTime(&systemtime);
            KeyNode->LastWriteTime = systemtime;

            KeyNode->Spare = 0;
            KeyNode->Parent = ParentCell;
            KeyNode->SubKeyCounts[Stable] = 0;
            KeyNode->SubKeyCounts[Volatile] = 0;
            KeyNode->SubKeyLists[Stable] = HCELL_NIL;
            KeyNode->SubKeyLists[Volatile] = HCELL_NIL;
            KeyNode->ValueList.Count = 0;
            KeyNode->ValueList.List = HCELL_NIL;
            KeyNode->Security = HCELL_NIL;
            KeyNode->Class = ClassCell;
            KeyNode->ClassLength = Context->Class.Length;

            KeyNode->MaxValueDataLen = 0;
            KeyNode->MaxNameLen = 0;
            KeyNode->MaxValueNameLen = 0;
            KeyNode->MaxClassLen = 0;

            KeyNode->NameLength = CmpCopyName(Hive,
                                              KeyNode->Name,
                                              Name);
            if (KeyNode->NameLength < Name->Length) {
                KeyNode->Flags |= KEY_COMP_NAME;
            }

            if (Context->CreateOptions & REG_OPTION_PREDEF_HANDLE) {
                KeyNode->ValueList.Count = (ULONG)((ULONG_PTR)Context->PredefinedHandle);
                KeyNode->Flags |= KEY_PREDEF_HANDLE;
            }

            //
            // Create kcb here so all data are filled in.
            //
            // Allocate a key control block
            //
            kcb = CmpCreateKeyControlBlock(Hive, *KeyCell, KeyNode, ParentKcb, CMP_CREATE_KCB_KCB_LOCKED, Name);
            if (kcb == NULL) {
                ObDereferenceObjectDeferDelete(*Object);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                leave;
            }
            ASSERT(kcb->RefCount == 1);
            alloc = 3;

#if DBG
            if( kcb->ExtFlags & CM_KCB_KEY_NON_EXIST ) {
                //
                // we shouldn't fall into this
                //
                ObDereferenceObjectDeferDelete(*Object);
                DbgBreakPoint();
                Status = STATUS_OBJECT_NAME_NOT_FOUND;
                leave;
            }
#endif //DBG
            //
            // Fill in CM specific fields in the object
            //
            KeyBody->Type = KEY_BODY_TYPE;
            KeyBody->KeyControlBlock = kcb;
            KeyBody->NotifyBlock = NULL;
            KeyBody->ProcessID = PsGetCurrentProcessId();
            EnlistKeyBodyWithKCB(KeyBody,CMP_ENLIST_KCB_LOCKED_EXCLUSIVE);
            //
            // Assign a security descriptor to the object.  Note that since
            // registry keys are container objects, and ObAssignSecurity
            // assumes that the only container object in the world is
            // the ObpDirectoryObjectType, we have to call SeAssignSecurity
            // directly in order to get the right inheritance.
            //

            Status = SeAssignSecurity(ParentDescriptor,
                                      AccessState->SecurityDescriptor,
                                      &NewDescriptor,
                                      TRUE,             // container object
                                      &AccessState->SubjectSecurityContext,
                                      &CmpKeyObjectType->TypeInfo.GenericMapping,
                                      CmpKeyObjectType->TypeInfo.PoolType);
            if (NT_SUCCESS(Status)) {
                CmLockHiveSecurityExclusive((PCMHIVE)kcb->KeyHive);
                //
                // force assign to the new kcb, by passing NULL as the trans
                //
                Status = CmpAssignSecurityDescriptorWrapper(*Object,NewDescriptor); 
                CmUnlockHiveSecurity((PCMHIVE)kcb->KeyHive);
            }

            //
            // Since the security descriptor now lives in the hive,
            // free the in-memory copy
            //
            SeDeassignSecurity( &NewDescriptor );

            if (!NT_SUCCESS(Status)) {

                //
                // Note that the dereference will clean up the kcb, so
                // make sure and decrement the allocation count here.
                //
                // Also mark the kcb as deleted so it does not get
                // inappropriately cached.
                //
                kcb->Delete = TRUE;
                CmpRemoveKeyControlBlock(kcb);
                ObDereferenceObjectDeferDelete(*Object);
                alloc = 2;

            } else {
                CmpReportNotify(
                        kcb,
                        kcb->KeyHive,
                        kcb->KeyCell,
                        REG_NOTIFY_CHANGE_NAME
                        );
            }
        }

    } finally {

        if (!NT_SUCCESS(Status)) {

            //
            // Clean up allocations
            //
            switch (alloc) {
            case 3:
                //
                // Mark KCB as deleted so it does not get inadvertently added to
                // the delayed close list. That would have fairly disastrous effects
                // as the KCB points to storage we are about to free.
                //
                kcb->Delete = TRUE;
                CmpRemoveKeyControlBlock(kcb);
                CmpDereferenceKeyControlBlockWithLock(kcb,FALSE);
                // DELIBERATE FALL

            case 2:
                if (Context->Class.Length > 0) {
                    HvFreeCell(Hive, ClassCell);
                }
                // DELIBERATE FALL

            case 1:
                HvFreeCell(Hive, *KeyCell);
                // DELIBERATE FALL
            }
        }
    }

    return(Status);
}
