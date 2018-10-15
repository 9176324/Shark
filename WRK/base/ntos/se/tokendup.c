/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    tokendup.c

Abstract:

   This module implements the token duplication service.

--*/

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtDuplicateToken)
#pragma alloc_text(PAGE,SepDuplicateToken)
#pragma alloc_text(PAGE,SepMakeTokenEffectiveOnly)
#pragma alloc_text(PAGE,SepSidInSidAndAttributes)
#pragma alloc_text(PAGE,SepRemoveDisabledGroupsAndPrivileges)
#pragma alloc_text(PAGE,SeCopyClientToken)
#pragma alloc_text(PAGE,NtFilterToken)
#pragma alloc_text(PAGE,SeFilterToken)
#pragma alloc_text(PAGE,SeFastFilterToken)
#pragma alloc_text(PAGE,SepFilterToken)
#endif


NTSTATUS
NtDuplicateToken(
    __in HANDLE ExistingTokenHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in BOOLEAN EffectiveOnly,
    __in TOKEN_TYPE TokenType,
    __out PHANDLE NewTokenHandle
    )

/*++


Routine Description:

    Create a new token that is a duplicate of an existing token.

Arguments:

    ExistingTokenHandle - Is a handle to a token already open for
        TOKEN_DUPLICATE access.

    DesiredAccess - Is an access mask indicating which access types
        are desired to the newly created token.  If specified as zero,
        the granted access mask of the existing token handle
        is used as the desired access mask for the new token.

    ObjectAttributes - Points to the standard object attributes data
        structure.  Refer to the NT Object Management
        Specification for a description of this data structure.

        If the new token type is TokenImpersonation, then this
        parameter may be used to specify the impersonation level
        of the new token.  If no value is provided, and the source
        token is an impersonation token, then the impersonation level
        of the source will become that of the target as well.  If the
        source token is a primary token, then an impersonation level
        must be explicitly provided.

        If the token being duplicated is an impersonation token, and
        an impersonation level is explicitly provided for the target,
        then the value provided must not be greater than that of the
        source token. For example, an Identification level token can
        not be duplicated to produce a Delegation level token.

    EffectiveOnly - Is a boolean flag indicating whether the entire
        source token should be duplicated into the target token or
        just the effective (currently enabled) part of the token.
        This provides a means for a caller of a protected subsystem
        to limit which privileges and optional groups are made
        available to the protected subsystem.  A value of TRUE
        indicates only the currently enabled parts of the source
        token are to be duplicated.  Otherwise, the entire source
        token is duplicated.

    TokenType - Indicates which type of object the new object is to
        be created as (primary or impersonation).  If you are duplicating
        an Impersonation token to produce a Primary token, then
        the Impersonation token must have an impersonation level of
        either DELEGATE or IMPERSONATE.


    NewTokenHandle - Receives the handle of the newly created token.

Return Value:

    STATUS_SUCCESS - Indicates the operation was successful.

    STATUS_INVALID_PARAMETER - Indicates one or more of the parameter values
        was invalid.  This value is returned if the target token is not
        an impersonation token.

    STATUS_BAD_IMPERSONATION_LEVEL - Indicates the impersonation level
        requested for the duplicate token is not compatible with the
        level of the source token.  The duplicate token may not be assigned
        a level greater than that of the source token.

--*/
{

    PTOKEN Token;
    PTOKEN NewToken;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    SECURITY_ADVANCED_QUALITY_OF_SERVICE SecurityQos;
    BOOLEAN SecurityQosPresent = FALSE;
    HANDLE LocalHandle = NULL;

    OBJECT_HANDLE_INFORMATION HandleInformation;
    ACCESS_MASK EffectiveDesiredAccess;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    //
    //  Probe parameters
    //

    if (PreviousMode != KernelMode) {

        try {

            //
            // Make sure the TokenType is valid
            //

            if ( (TokenType < TokenPrimary) || (TokenType > TokenImpersonation) ) {
                return(STATUS_INVALID_PARAMETER);
            }

            //
            //  Make sure we can write the handle
            //

            ProbeForWriteHandle(NewTokenHandle);


        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }  // end_try

    } //end_if



    Status = SeCaptureSecurityQos(
                 ObjectAttributes,
                 PreviousMode,
                 &SecurityQosPresent,
                 &SecurityQos
                 );

    if (!NT_SUCCESS(Status)) {
        return Status;
    }


    //
    //  Check the handle's access to the existing token and get
    //  a pointer to that token.  Pick up the default desired
    //  access mask from the handle while we're at it.
    //

    Status = ObReferenceObjectByHandle(
                 ExistingTokenHandle,    // Handle
                 TOKEN_DUPLICATE,        // DesiredAccess
                 SeTokenObjectType,     // ObjectType
                 PreviousMode,           // AccessMode
                 (PVOID *)&Token,        // Object
                 &HandleInformation      // GrantedAccess
                 );

    if ( !NT_SUCCESS(Status) ) {

        if (SecurityQosPresent) {
            SeFreeCapturedSecurityQos( &SecurityQos );
        }
        return Status;
    }

    //
    // Check to see if an alternate desired access mask was provided.
    //

    if (ARGUMENT_PRESENT(DesiredAccess)) {

        EffectiveDesiredAccess = DesiredAccess;

    } else {

        EffectiveDesiredAccess = HandleInformation.GrantedAccess;
    }


    //
    //  If no impersonation level was specified, pick one up from
    //  the source token.
    //

    if ( !SecurityQosPresent ) {

        SecurityQos.ImpersonationLevel = Token->ImpersonationLevel;

    }

    //
    // If an impersonation token is duplicated into an impersonation token, we 
    // need to do checks on the impersonation level.
    // The impersonation level requested for the new token is ignored if the 
    // new token type is TokenPrimary.
    //

    if ( (Token->TokenType == TokenImpersonation) &&
         (TokenType == TokenImpersonation) 
       ) {

        //
        // Make sure a legitimate transformation is being requested:
        //
        //    (1) The impersonation level of a target duplicate must not
        //        exceed that of the source token.
        //
        //

        ASSERT( SecurityDelegation     > SecurityImpersonation );
        ASSERT( SecurityImpersonation  > SecurityIdentification );
        ASSERT( SecurityIdentification > SecurityAnonymous );

        if ( (SecurityQos.ImpersonationLevel > Token->ImpersonationLevel) ) {

            ObDereferenceObject( (PVOID)Token );
            if (SecurityQosPresent) {
                SeFreeCapturedSecurityQos( &SecurityQos );
            }
            return STATUS_BAD_IMPERSONATION_LEVEL;
        }

    }

    //
    // If we are producing a Primary token from an impersonation
    // token, then specify an impersonation level of at least
    // Impersonate.
    //

    if ( (Token->TokenType == TokenImpersonation) &&
         (TokenType == TokenPrimary)              &&
         (Token->ImpersonationLevel <  SecurityImpersonation)
       ) {
        ObDereferenceObject( (PVOID)Token );
        if (SecurityQosPresent) {
            SeFreeCapturedSecurityQos( &SecurityQos );
        }
        return STATUS_BAD_IMPERSONATION_LEVEL;
    }

    //
    //  Duplicate the existing token
    //

    NewToken = NULL;
    Status = SepDuplicateToken(
                 Token,
                 ObjectAttributes,
                 EffectiveOnly,
                 TokenType,
                 SecurityQos.ImpersonationLevel,
                 PreviousMode,
                 &NewToken
                 );


    if (NT_SUCCESS(Status)) {

        //
        //  Insert the new token
        //

        Status = ObInsertObject( NewToken,
                                 NULL,
                                 EffectiveDesiredAccess,
                                 0,
                                 (PVOID *)NULL,
                                 &LocalHandle
                                 );
    }

    //
    //  We no longer need our reference to the source token
    //

    ObDereferenceObject( (PVOID)Token );

    if (SecurityQosPresent) {
        SeFreeCapturedSecurityQos( &SecurityQos );
    }

    //
    //  Return the new handle
    //

    if (NT_SUCCESS(Status)) {
        try {
            *NewTokenHandle = LocalHandle;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

   return Status;
}


NTSTATUS
SepDuplicateToken(
    __in PTOKEN ExistingToken,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in BOOLEAN EffectiveOnly,
    __in TOKEN_TYPE TokenType,
    __in SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    __in KPROCESSOR_MODE RequestorMode,
    __deref_out PTOKEN *DuplicateToken
    )


/*++


Routine Description:

    This routine does the bulk of the work to actually duplicate
    a token.  This routine assumes all access validation and argument
    probing (except the ObjectAttributes) has been performed.

    THE CALLER IS RESPONSIBLE FOR CHECKING SUBJECT RIGHTS TO CREATE THE
    TYPE OF TOKEN BEING CREATED.

    This routine acquires a read lock on the token being duplicated.

Arguments:

    ExistingToken - Points to the token to be duplicated.

    ObjectAttributes - Points to the standard object attributes data
        structure.  Refer to the NT Object Management
        Specification for a description of this data structure.

        The security Quality Of Service of the object attributes are ignored.
        This information must be specified using parameters to this
        routine.

    EffectiveOnly - Is a boolean flag indicating whether the entire
        source token should be duplicated into the target token or
        just the effective (currently enabled) part of the token.
        This provides a means for a caller of a protected subsystem
        to limit which privileges and optional groups are made
        available to the protected subsystem.  A value of TRUE
        indicates only the currently enabled parts of the source
        token are to be duplicated.  Otherwise, the entire source
        token is duplicated.

    TokenType - Indicates the type of token to make the duplicate token.

    ImpersonationLevel - This value specifies the impersonation level
        to assign to the duplicate token.  If the TokenType of the
        duplicate is not TokenImpersonation then this parameter is
        ignored.  Otherwise, it is must be provided.

    RequestorMode - Mode of client requesting the token be duplicated.

    DuplicateToken - Receives a pointer to the duplicate token.
        The token has not yet been inserted into any object table.
        No exceptions are expected when trying to set this OUT value.

Return Value:

    STATUS_SUCCESS - The service successfully completed the requested
        operation.


--*/
{
    NTSTATUS Status;

    PTOKEN NewToken;
    PULONG DynamicPart;
    ULONG PagedPoolSize;
    ULONG NonPagedPoolSize;
    ULONG TokenBodyLength;
    ULONG_PTR FieldOffset;
    ULONG DynamicSize;
    ULONG Index;

    PSECURITY_TOKEN_PROXY_DATA NewProxyData = NULL;
    PSECURITY_TOKEN_AUDIT_DATA NewAuditData = NULL;
    PSID_AND_ATTRIBUTES UserAndGroups;
    PSID_AND_ATTRIBUTES RestrictedSids;

    PERESOURCE TokenLock;

#if DBG || TOKEN_LEAK_MONITOR
    ULONG Frames;
#endif

    PAGED_CODE();

    ASSERT( sizeof(SECURITY_IMPERSONATION_LEVEL) <= sizeof(ULONG) );


    if ( TokenType == TokenImpersonation ) {

        ASSERT( SecurityDelegation     > SecurityImpersonation );
        ASSERT( SecurityImpersonation  > SecurityIdentification );
        ASSERT( SecurityIdentification > SecurityAnonymous );

        if ( (ImpersonationLevel > SecurityDelegation)  ||
             (ImpersonationLevel < SecurityAnonymous) ) {

            return STATUS_BAD_IMPERSONATION_LEVEL;
        }
    }

    if (ARGUMENT_PRESENT(ExistingToken->ProxyData)) {

        Status = SepCopyProxyData(
                    &NewProxyData,
                    ExistingToken->ProxyData
                    );

        if (!NT_SUCCESS(Status)) {
            return( Status );
        }

    } else {

        NewProxyData = NULL;
    }

    if (ARGUMENT_PRESENT( ExistingToken->AuditData )) {

        NewAuditData = ExAllocatePool( PagedPool, sizeof( SECURITY_TOKEN_AUDIT_DATA ));

        if (NewAuditData == NULL) {

            SepFreeProxyData( NewProxyData );

            return( STATUS_INSUFFICIENT_RESOURCES );

        } else {

            *NewAuditData = *(ExistingToken->AuditData);
        }

    } else {

        NewAuditData = NULL;

    }

    TokenLock = (PERESOURCE)ExAllocatePoolWithTag( NonPagedPool, sizeof( ERESOURCE ), 'dTeS' );

    if (TokenLock == NULL) {

        if (NewAuditData != NULL) {
            ExFreePool( NewAuditData );
        }

        SepFreeProxyData( NewProxyData );

        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    //
    //  Create a new object
    //

    TokenBodyLength = FIELD_OFFSET(TOKEN, VariablePart) +
                      ExistingToken->VariableLength;

    NonPagedPoolSize = TokenBodyLength;
    PagedPoolSize    = ExistingToken->DynamicCharged;

    Status = ObCreateObject(
                 RequestorMode,      // ProbeMode
                 SeTokenObjectType, // ObjectType
                 ObjectAttributes,   // ObjectAttributes
                 RequestorMode,      // OwnershipMode
                 NULL,               // ParseContext
                 TokenBodyLength,    // ObjectBodySize
                 PagedPoolSize,      // PagedPoolCharge
                 NonPagedPoolSize,   // NonPagedPoolCharge
                 (PVOID *)&NewToken  // Return pointer to object
                 );

    if (!NT_SUCCESS(Status)) {
        SepFreeProxyData( NewProxyData );
        ExFreePool( TokenLock );

        if (NewAuditData != NULL) {
            ExFreePool( NewAuditData );
        }

        return Status;
    }

    //
    // The following fields differ in the new token and can be filled out without the lock.
    //

    ExAllocateLocallyUniqueId( &(NewToken->TokenId) );
    NewToken->TokenInUse = FALSE;
    NewToken->TokenType = TokenType;
    NewToken->ImpersonationLevel = ImpersonationLevel;
    NewToken->TokenLock = TokenLock;

    ExInitializeResourceLite( NewToken->TokenLock );

    NewToken->AuthenticationId = ExistingToken->AuthenticationId;
    NewToken->TokenSource = ExistingToken->TokenSource;
    NewToken->DynamicAvailable = 0;
    NewToken->ProxyData = NewProxyData;
    NewToken->AuditData = NewAuditData;
    NewToken->ParentTokenId = ExistingToken->ParentTokenId;
    NewToken->ExpirationTime = ExistingToken->ExpirationTime;
    NewToken->OriginatingLogonSession  = ExistingToken->OriginatingLogonSession ;

    //
    //  acquire exclusive access to the source token
    //

    SepAcquireTokenReadLock( ExistingToken );


    //
    // Main Body initialization
    //

    //
    // The following fields are unchanged from the source token.
    // Although some may change if EffectiveOnly has been specified.
    //

    NewToken->ModifiedId = ExistingToken->ModifiedId;
    NewToken->DynamicCharged = ExistingToken->DynamicCharged;
    NewToken->DefaultOwnerIndex = ExistingToken->DefaultOwnerIndex;
    NewToken->UserAndGroupCount = ExistingToken->UserAndGroupCount;
    NewToken->RestrictedSidCount = ExistingToken->RestrictedSidCount;
    NewToken->PrivilegeCount = ExistingToken->PrivilegeCount;
    NewToken->VariableLength = ExistingToken->VariableLength;
    NewToken->TokenFlags = ExistingToken->TokenFlags & ~TOKEN_SESSION_NOT_REFERENCED;
    NewToken->SessionId = ExistingToken->SessionId;
    NewToken->AuditPolicy = ExistingToken->AuditPolicy;


    //
    // Increment the reference count for this logon session
    // This can not fail, since there is already a token in this logon
    // session.
    //

    Status = SepDuplicateLogonSessionReference (NewToken, ExistingToken);

    ASSERT( NT_SUCCESS(Status) );

    if (!NT_SUCCESS (Status)) {
        SepReleaseTokenReadLock( ExistingToken );

        NewToken->DynamicPart = NULL;
        ObDereferenceObject (NewToken);
        return Status;
    }


#if DBG || TOKEN_LEAK_MONITOR

    NewToken->ProcessCid          = PsGetCurrentThread()->Cid.UniqueProcess;
    NewToken->ThreadCid           = PsGetCurrentThread()->Cid.UniqueThread;
    NewToken->CreateMethod        = 0xD; // Duplicate
    NewToken->Count               = 0;
    NewToken->CaptureCount        = 0;

    RtlCopyMemory(
        NewToken->ImageFileName,
        PsGetCurrentProcess()->ImageFileName, 
        min(sizeof(NewToken->ImageFileName), sizeof(PsGetCurrentProcess()->ImageFileName))
        );

    Frames = RtlWalkFrameChain(
                 (PVOID)NewToken->CreateTrace,
                 TRACE_SIZE,
                 0
                 );

    if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
        
        RtlWalkFrameChain(
            (PVOID)&NewToken->CreateTrace[Frames],
            TRACE_SIZE - Frames,
            1
            );
    }

    SepAddTokenLogonSession(NewToken);

#endif

    //
    //  Copy and initialize the variable part.
    //  The variable part is assumed to be position independent.
    //

    RtlCopyMemory( (PVOID)&(NewToken->VariablePart),
                  (PVOID)&(ExistingToken->VariablePart),
                  ExistingToken->VariableLength
                  );

    //
    //  Set the address of the UserAndGroups array.
    //

    ASSERT( ARGUMENT_PRESENT(ExistingToken->UserAndGroups ) );
    ASSERT( (ULONG_PTR)(ExistingToken->UserAndGroups) >=
            (ULONG_PTR)(&(ExistingToken->VariablePart)) );

    //
    // Calculate the relative offset between the old and new block. We
    // will use this value to fixup embedded pointers in this block.
    //
    FieldOffset = (ULONG_PTR)&NewToken->VariablePart - (ULONG_PTR)&ExistingToken->VariablePart;

    UserAndGroups = (PSID_AND_ATTRIBUTES) ((ULONG_PTR) ExistingToken->UserAndGroups + FieldOffset);

    NewToken->UserAndGroups = UserAndGroups;

    //
    //  Now go through and change the address of each SID pointer
    //  for the user and groups
    //

    Index = NewToken->UserAndGroupCount;

    while (Index > 0) {

        (ULONG_PTR) UserAndGroups->Sid += FieldOffset;

        Index -= 1;
        UserAndGroups++;

    }

    //
    //  Set the address of the RestrictedSids array.
    //
    
    RestrictedSids = ExistingToken->RestrictedSids;
    NewToken->RestrictedSids = RestrictedSids;

    if (ARGUMENT_PRESENT(RestrictedSids) ) {
        ASSERT( (ULONG_PTR)(ExistingToken->RestrictedSids) >=
                (ULONG_PTR)(&(ExistingToken->VariablePart)) );


        (ULONG_PTR) RestrictedSids += FieldOffset;
        NewToken->RestrictedSids = RestrictedSids;

        //
        //  Now go through and change the address of each SID pointer
        //  for the user and groups
        //

        Index = NewToken->RestrictedSidCount;

        while (Index > 0) {

            (ULONG_PTR) RestrictedSids->Sid += FieldOffset;
            RestrictedSids++;

            Index -= 1;

        }
    }

    //
    // If present, set the address of the privileges
    //

    if (ExistingToken->PrivilegeCount > 0) {
        ASSERT( ARGUMENT_PRESENT(ExistingToken->Privileges ) );
        ASSERT( (ULONG_PTR)(ExistingToken->Privileges) >=
                (ULONG_PTR)(&(ExistingToken->VariablePart)) );

        (ULONG_PTR) NewToken->Privileges = (ULONG_PTR) ExistingToken->Privileges + FieldOffset;

    } else {

        NewToken->Privileges = NULL;

    }

    //
    //  Allocate the dynamic portion
    //
    DynamicSize = SeLengthSid( ExistingToken->PrimaryGroup );
    if (ExistingToken->DefaultDacl) {
        DynamicSize += ExistingToken->DefaultDacl->AclSize;
    }

    DynamicPart = (PULONG)ExAllocatePoolWithTag(
                              PagedPool,
                              DynamicSize,
                              'dTeS'
                              );

    NewToken->DynamicPart = DynamicPart;

    if (DynamicPart == NULL) {
        SepReleaseTokenReadLock( ExistingToken );
        ObDereferenceObject (NewToken);
        return( STATUS_INSUFFICIENT_RESOURCES );
    }
    //
    //  Copy and initialize the dynamic part.
    //  The dynamic part is assumed to be position independent.
    //

    RtlCopyMemory( (PVOID)DynamicPart,
                  (PVOID)(ExistingToken->DynamicPart),
                  DynamicSize
                  );

    FieldOffset = (ULONG_PTR) DynamicPart - (ULONG_PTR) ExistingToken->DynamicPart;
    //
    // If present, set the address of the default Dacl
    //

    NewToken->DefaultDacl = ExistingToken->DefaultDacl;
    if (ARGUMENT_PRESENT(NewToken->DefaultDacl)) {

        ASSERT( (ULONG_PTR)(ExistingToken->DefaultDacl) >=
                (ULONG_PTR)(ExistingToken->DynamicPart) );


        (ULONG_PTR) NewToken->DefaultDacl += FieldOffset;

    }

    //
    // Set the address of the primary group
    //

    ASSERT(ARGUMENT_PRESENT(ExistingToken->PrimaryGroup));

    ASSERT( (ULONG_PTR)(ExistingToken->PrimaryGroup) >=
            (ULONG_PTR)(ExistingToken->DynamicPart) );


    (ULONG_PTR) NewToken->PrimaryGroup = (ULONG_PTR) ExistingToken->PrimaryGroup + FieldOffset;

    //
    // Release the source token.
    //

    SepReleaseTokenReadLock( ExistingToken );




    //
    // For the time being, take the easy way to generating an "EffectiveOnly"
    // duplicate.  That is, use the same space required of the original, just
    // eliminate any IDs or privileges not active.
    //
    // Ultimately, if duplication becomes a common operation, then it will be
    // worthwhile to recalculate the actual space needed and copy only the
    // effective IDs/privileges into the new token.
    //

    if (EffectiveOnly) {
        SepMakeTokenEffectiveOnly( NewToken );
    }


    //
    // If the NewToken inherited an active SEP_AUDIT_POLICY from ExistingToken,
    // then increment the counter of tokens with policies.
    //

    if ( NewToken->AuditPolicy.Overlay ) {
        SepModifyTokenPolicyCounter(&NewToken->AuditPolicy, TRUE);
    }
    
#if DBG || TOKEN_LEAK_MONITOR
    if (SepTokenLeakTracking && SepTokenLeakMethodWatch == 0xD && PsGetCurrentProcess()->UniqueProcessId == SepTokenLeakProcessCid) {
        
        NewToken->Count = InterlockedIncrement(&SepTokenLeakMethodCount);
        if (NewToken->Count >= SepTokenLeakBreakCount) {

            DbgPrint("\nToken number 0x%x = 0x%x\n", NewToken->Count, NewToken);
            DbgBreakPoint();
        }
    }
#endif

    (*DuplicateToken) = NewToken;
    return Status;
}


VOID
SepMakeTokenEffectiveOnly(
    __in PTOKEN Token
    )


/*++


Routine Description:

    This routine eliminates all but the effective groups and privileges from
    a token.  It does this by moving elements of the SID and privileges arrays
    to overwrite lapsed IDs/privileges, and then reducing the array element
    counts.  This results in wasted memory within the token object.

    One side effect of this routine is that a token that initially had a
    default owner ID corresponding to a lapsed group will be changed so
    that the default owner ID is the user ID.

    THIS ROUTINE MUST BE CALLED ONLY AS PART OF TOKEN CREATION (FOR TOKENS
    WHICH HAVE NOT YET BEEN INSERTED INTO AN OBJECT TABLE.)  THIS ROUTINE
    MODIFIES READ ONLY TOKEN FIELDS.

    Note that since we are operating on a token that is not yet visible
    to the user, we do not bother acquiring a read lock on the token
    being modified.

Arguments:

    Token - Points to the token to be made effective only.

Return Value:

    None.

--*/
{

    ULONG Index;
    ULONG ElementCount;

    PAGED_CODE();

    //
    // Walk the privilege array, discarding any lapsed privileges
    //

    ElementCount = Token->PrivilegeCount;
    Index = 0;

    while (Index < ElementCount) {

        //
        // If this privilege is not enabled, replace it with the one at
        // the end of the array and reduce the size of the array by one.
        // Otherwise, move on to the next entry in the array.
        //

        if ( !(SepTokenPrivilegeAttributes(Token,Index) & SE_PRIVILEGE_ENABLED)
            ) {

            (Token->Privileges)[Index] =
                (Token->Privileges)[ElementCount - 1];
            ElementCount -= 1;

        } else {

            Index += 1;

        }

    } // endwhile

    Token->PrivilegeCount = ElementCount;

    //
    // Walk the UserAndGroups array (except for the first entry, which is
    // the user - and can't be disabled) discarding any lapsed groups.
    //

    ElementCount = Token->UserAndGroupCount;
    ASSERT( ElementCount >= 1 );        // Must be at least a user ID
    Index = 1;   // Start at the first group, not the user ID.

    while (Index < ElementCount) {

        //
        // If this group is not enabled, replace it with the one at
        // the end of the array and reduce the size of the array by one.
        //

        if ( !(SepTokenGroupAttributes(Token, Index) & SE_GROUP_ENABLED) &&
             !(SepTokenGroupAttributes(Token, Index) & SE_GROUP_USE_FOR_DENY_ONLY) ) {

            //
            // Reset the TOKEN_HAS_ADMIN_GROUP flag
            //

            if (RtlEqualSid(
                    Token->UserAndGroups[Index].Sid,
                    SeAliasAdminsSid
                    )) {
                Token->TokenFlags &= ~TOKEN_HAS_ADMIN_GROUP;
            }


            (Token->UserAndGroups)[Index] =
                (Token->UserAndGroups)[ElementCount - 1];
            ElementCount -= 1;



        } else {

            Index += 1;

        }

    } // endwhile

    Token->UserAndGroupCount = ElementCount;

    return;
}


BOOLEAN
SepSidInSidAndAttributes (
    __in_ecount(SidCount) PSID_AND_ATTRIBUTES SidAndAttributes,
    __in ULONG SidCount,
    __in_opt PSID PrincipalSelfSid,
    __in PSID Sid
    )

/*++

Routine Description:

    Checks to see if a given SID is in the given token.

    N.B. The code to compute the length of a SID and test for equality
         is duplicated from the security runtime since this is such a
         frequently used routine.

Arguments:

    SidAndAttributes - Pointer to the sid and attributes to be examined

    PrincipalSelfSid - If the object being access checked is an object which
        represents a principal (e.g., a user object), this parameter should
        be the SID of the object.  Any ACE containing the constant
        PRINCIPAL_SELF_SID is replaced by this SID.

        The parameter should be NULL if the object does not represent a principal.


    Sid - Pointer to the SID of interest

Return Value:

    A value of TRUE indicates that the SID is in the token, FALSE
    otherwise.

--*/

{

    ULONG i;
    PISID MatchSid;
    ULONG SidLength;
    PTOKEN Token;
    PSID_AND_ATTRIBUTES TokenSid;
    ULONG UserAndGroupCount;

    PAGED_CODE();


    if (!ARGUMENT_PRESENT( SidAndAttributes ) ) {
        return(FALSE);
    }

    //
    // If Sid is the constant PrincipalSelfSid,
    //  replace it with the passed in PrincipalSelfSid.
    //

    if ( PrincipalSelfSid != NULL &&
         RtlEqualSid( SePrincipalSelfSid, Sid ) ) {
        Sid = PrincipalSelfSid;
    }

    //
    // Get the length of the source SID since this only needs to be computed
    // once.
    //

    SidLength = 8 + (4 * ((PISID)Sid)->SubAuthorityCount);

    //
    // Get address of user/group array and number of user/groups.
    //

    TokenSid = SidAndAttributes;
    UserAndGroupCount = SidCount;

    //
    // Scan through the user/groups and attempt to find a match with the
    // specified SID.
    //

    for (i = 0 ; i < UserAndGroupCount ; i += 1) {
        MatchSid = (PISID)TokenSid->Sid;

        //
        // If the SID revision and length matches, then compare the SIDs
        // for equality.
        //

        if ((((PISID)Sid)->Revision == MatchSid->Revision) &&
            (SidLength == (8 + (4 * (ULONG)MatchSid->SubAuthorityCount)))) {
            if (RtlEqualMemory(Sid, MatchSid, SidLength)) {

                return TRUE;

            }
        }

        TokenSid += 1;
    }

    return FALSE;
}


VOID
SepRemoveDisabledGroupsAndPrivileges(
    __in PTOKEN Token,
    __in ULONG Flags,
    __in ULONG GroupCount,
    __in_ecount(GroupCount) PSID_AND_ATTRIBUTES GroupsToDisable,
    __in ULONG PrivilegeCount,
    __in_ecount( PrivilegeCount ) PLUID_AND_ATTRIBUTES PrivilegesToDelete
    )
/*++


Routine Description:

    This routine eliminates all groups and privileges that are marked
    to be deleted/disabled. It does this by looping through the groups in
    the token and checking each one against the groups to disable. Similarly
    the privileges are compared.  It does this by moving elements of the SID and privileges arrays
    to overwrite lapsed IDs/privileges, and then reducing the array element
    counts.  This results in wasted memory within the token object.


    THIS ROUTINE MUST BE CALLED ONLY AS PART OF TOKEN CREATION (FOR TOKENS
    WHICH HAVE NOT YET BEEN INSERTED INTO AN OBJECT TABLE.)  THIS ROUTINE
    MODIFIES READ ONLY TOKEN FIELDS.

    Note that since we are operating on a token that is not yet visible
    to the user, we do not bother acquiring a read lock on the token
    being modified.

Arguments:

    Token - Points to the token to be made effective only.

    Flags - Flags indicating additional filtering. The flags may be:

                DISABLE_MAX_PRIVILEGE - Disable all privileges

    GroupCount - Count of groups to be removed

    GroupsToDisable - Groups to disable and mark with SE_GROUP_USE_FOR_DENY_ONLY

    PrivilegeCount - Count of privileges to remove

    PrivilegesToDelete - List of privileges to remove

Return Value:

    None.

--*/
{

    ULONG Index;
    ULONG Index2;
    ULONG ElementCount;
    BOOLEAN Found;

    PAGED_CODE();

    //
    // Walk the privilege array, discarding any lapsed privileges
    //

    ElementCount = Token->PrivilegeCount;
    Index = 0;

    while (Index < ElementCount) {

        //
        // If the caller asked us to disable all privileges except change
        // notify, do so now.
        //

        if (((Flags & DISABLE_MAX_PRIVILEGE) != 0) &&
              !RtlEqualLuid(
                &Token->Privileges[Index].Luid,
                &SeChangeNotifyPrivilege
                )) {

            (Token->Privileges)[Index] =
                (Token->Privileges)[ElementCount - 1];
            ElementCount -= 1;

        } else {

            //
            // If this privilege is in the list of those to be removed, replace it
            // with the one at the end of the array and reduce the size of the
            // array by one.  Otherwise, move on to the next entry in the array.
            //

            Found = FALSE;
            for (Index2 = 0; Index2 < PrivilegeCount; Index2++ ) {
                
                if (RtlEqualLuid(
                        &Token->Privileges[Index].Luid,
                        &PrivilegesToDelete[Index2].Luid
                        )) {

                    //
                    // If this was a privilege that is noted in the TokenFlags
                    // then the flag must be turned off as well.
                    //

                    if (RtlEqualLuid(
                            &Token->Privileges[Index].Luid,
                            &SeChangeNotifyPrivilege
                            )) {
                        
                        Token->TokenFlags &= ~TOKEN_HAS_TRAVERSE_PRIVILEGE;
                    
                    } else if (RtlEqualLuid(
                                   &Token->Privileges[Index].Luid,
                                   &SeImpersonatePrivilege
                                   )) {
                        
                        Token->TokenFlags &= ~TOKEN_HAS_IMPERSONATE_PRIVILEGE;
                    }

                    (Token->Privileges)[Index] = (Token->Privileges)[ElementCount - 1];
                    ElementCount--;

                    Found = TRUE;
                    break;
                }
            }

            if (!Found) {
                Index += 1;
            }
        }
    } // endwhile

    //
    // Make sure the impersonate privilege is completely
    // turned off for the "blanket" privilege disablings.
    //

    if (Flags & DISABLE_MAX_PRIVILEGE) {

        Token->TokenFlags &= ~TOKEN_HAS_IMPERSONATE_PRIVILEGE;
    }

    Token->PrivilegeCount = ElementCount;

    //
    // Walk the UserAndGroups array marking any disabled groups.
    //

    ElementCount = Token->UserAndGroupCount;
    ASSERT( ElementCount >= 1 );        // Must be at least a user ID
    Index = 0;   // Start at the first group, not the user ID.

    while (Index < ElementCount) {

        //
        // If this group is not enabled, replace it with the one at
        // the end of the array and reduce the size of the array by one.
        //

        if ( SepSidInSidAndAttributes(
                GroupsToDisable,
                GroupCount,
                NULL,           // no principal self sid
                Token->UserAndGroups[Index].Sid
                )){

            (Token->UserAndGroups)[Index].Attributes &= ~(SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT);
            (Token->UserAndGroups)[Index].Attributes |= SE_GROUP_USE_FOR_DENY_ONLY;

            //
            // If this was the owner, reset the owner to be the user
            //

            if (Index == Token->DefaultOwnerIndex) {
                Token->DefaultOwnerIndex = 0;
            }

            //
            // If this is the admins sid, turn off the admin group flag
            //

            if (RtlEqualSid(
                    Token->UserAndGroups[Index].Sid,
                    SeAliasAdminsSid
                    )) {

                Token->TokenFlags &= ~TOKEN_HAS_ADMIN_GROUP;
            }
        }

        Index += 1;


    } // endwhile


    return;
}


NTSTATUS
SeCopyClientToken(
    __in PACCESS_TOKEN ClientToken,
    __in SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    __in KPROCESSOR_MODE RequestorMode,
    __deref_out PACCESS_TOKEN *DuplicateToken
    )

/*++


Routine Description:

    This routine copies a client's token as part of establishing a client
    context for impersonation.

    The result will be an impersonation token.

    No handles to the new token are established.

    The token will be an exact duplicate of the source token.  It is the
    caller's responsibility to ensure an effective only copy of the token
    is produced when the token is opened, if necessary.


Arguments:

    ClientToken - Points to the token to be duplicated.  This may be either
        a primary or impersonation token.

    ImpersonationLevel - The impersonation level to be assigned to the new
        token.

    RequestorMode - Mode to be assigned as the owner mode of the new token.

    DuplicateToken - Receives a pointer to the duplicate token.
        The token has not yet been inserted into any object table.
        No exceptions are expected when trying to set this OUT value.

Return Value:

    STATUS_SUCCESS - The service successfully completed the requested
        operation.


--*/
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PTOKEN NewToken;

    PAGED_CODE();

    InitializeObjectAttributes(
        &ObjectAttributes,
        NULL,
        0,
        NULL,
        NULL
        );

    Status = SepDuplicateToken(
                 (PTOKEN)ClientToken,              // ExistingToken
                 &ObjectAttributes,                // ObjectAttributes
                 FALSE,                            // EffectiveOnly
                 TokenImpersonation,               // TokenType  (target)
                 ImpersonationLevel,               // ImpersonationLevel
                 RequestorMode,                    // RequestorMode
                 &NewToken                         // DuplicateToken
                 );

    if (NT_SUCCESS (Status)) {
        //
        //  Insert the new token
        //

        Status = ObInsertObject( NewToken,
                                 NULL,
                                 0,
                                 0,
                                 NULL,
                                 NULL
                                 );


    }

    if (NT_SUCCESS (Status)) {
        *DuplicateToken = (PACCESS_TOKEN)NewToken;
    } else {
        *DuplicateToken = NULL;
    }

    return Status;

}



NTSTATUS
NtFilterToken (
    __in HANDLE ExistingTokenHandle,
    __in ULONG Flags,
    __in_opt PTOKEN_GROUPS SidsToDisable,
    __in_opt PTOKEN_PRIVILEGES PrivilegesToDelete,
    __in_opt PTOKEN_GROUPS RestrictedSids,
    __out PHANDLE NewTokenHandle
    )
/*++


Routine Description:

    Create a new token that is a subset of an existing token.

Arguments:

    ExistingTokenHandle - Is a handle to a token already open for
        TOKEN_DUPLICATE access.

    Flags - Flags indicating additional filtering. The flags may be:

                DISABLE_MAX_PRIVILEGE - Disable all privileges
                SANDBOX_INERT - Record this SAIFER flag in the token.


    SidsToDisable - Contains a list of sids and attributes. All sids with
        the USE_FOR_DENY_ONLY attribute that also exist in the token will
        cause the new token to have that sid set with the USE_FOR_DENY_ONLY
        attribute.

    PrivilegesToDelete - Privileges in this list that are present in the
        existing token will not exist in the final token. This is similar
        to duplicating a token effective only with these privileges set to
        disabled.

    RestrictedSids - Contains a list of SIDs and attributes that will be
        stored in the RestrictedSids field of the new token. These SIDs
        are used after a normal access check to further restrict access.
        The attributes of these groups are always SE_GROUP_MANDATORY |
        SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT. If there already
        exist RestrictedSids in the original token, these sids will be
        appended.

    NewTokenHandle - Receives the handle of the newly created token.

Return Value:

    STATUS_SUCCESS - Indicates the operation was successful.

    STATUS_INVALID_PARAMETER - Indicates one or more of the parameter values
        was invalid.  This value is returned if the target token is not
        an impersonation token.


--*/
{

    PTOKEN Token;
    PTOKEN NewToken;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status = STATUS_SUCCESS;

    ULONG CapturedSidCount = 0;
    PSID_AND_ATTRIBUTES CapturedSids = NULL;
    ULONG CapturedSidsLength = 0;

    ULONG CapturedGroupCount = 0;
    PSID_AND_ATTRIBUTES CapturedGroups = NULL;
    ULONG CapturedGroupsLength = 0;

    ULONG CapturedPrivilegeCount = 0;
    PLUID_AND_ATTRIBUTES CapturedPrivileges = NULL;
    ULONG CapturedPrivilegesLength = 0;
    ULONG Index;

    HANDLE LocalHandle = NULL;

    OBJECT_HANDLE_INFORMATION HandleInformation;
    ACCESS_MASK EffectiveDesiredAccess;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    //
    //  Probe parameters
    //


    try {


        //
        //  Make sure we can write the handle
        //

        ProbeForWriteHandle(NewTokenHandle);

        //
        //  Capture Sids to remove
        //

        if (ARGUMENT_PRESENT(SidsToDisable)) {
            ProbeForReadSmallStructure( SidsToDisable, sizeof(TOKEN_GROUPS), sizeof(ULONG) );

            CapturedGroupCount = SidsToDisable->GroupCount;
            Status = SeCaptureSidAndAttributesArray(
                        SidsToDisable->Groups,
                        CapturedGroupCount,
                        PreviousMode,
                        NULL, 0,
                        PagedPool,
                        TRUE,
                        &CapturedGroups,
                        &CapturedGroupsLength
                        );

        }

        //
        //  Capture PrivilegesToDelete
        //

        if (NT_SUCCESS(Status) && ARGUMENT_PRESENT(PrivilegesToDelete)) {
            ProbeForReadSmallStructure( PrivilegesToDelete, sizeof(TOKEN_PRIVILEGES), sizeof(ULONG) );

            CapturedPrivilegeCount = PrivilegesToDelete->PrivilegeCount;
            Status = SeCaptureLuidAndAttributesArray(
                         PrivilegesToDelete->Privileges,
                         CapturedPrivilegeCount,
                         PreviousMode,
                         NULL, 0,
                         PagedPool,
                         TRUE,
                         &CapturedPrivileges,
                         &CapturedPrivilegesLength
                         );

        }

        //
        //  Capture Restricted Sids
        //

        if (NT_SUCCESS(Status) && ARGUMENT_PRESENT(RestrictedSids)) {
            ProbeForReadSmallStructure( RestrictedSids, sizeof(TOKEN_GROUPS), sizeof(ULONG) );

            CapturedSidCount = RestrictedSids->GroupCount;
            Status = SeCaptureSidAndAttributesArray(
                        RestrictedSids->Groups,
                        CapturedSidCount,
                        PreviousMode,
                        NULL, 0,
                        PagedPool,
                        TRUE,
                        &CapturedSids,
                        &CapturedSidsLength
                        );

        }



    } except(EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();
    }  // end_try

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    //
    // Check that the attributes are all zero for the restricted sids
    //

    for (Index = 0; Index < CapturedSidCount ; Index++ )
    {
        if (CapturedSids[Index].Attributes != 0) {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }
    }
    //
    //  Check the handle's access to the existing token and get
    //  a pointer to that token.  Pick up the default desired
    //  access mask from the handle while we're at it.
    //

    Status = ObReferenceObjectByHandle(
                 ExistingTokenHandle,    // Handle
                 TOKEN_DUPLICATE,        // DesiredAccess
                 SeTokenObjectType,     // ObjectType
                 PreviousMode,           // AccessMode
                 (PVOID *)&Token,        // Object
                 &HandleInformation      // GrantedAccess
                 );

    if ( !NT_SUCCESS(Status) ) {

        goto Cleanup;
    }

    //
    // Check to see if an alternate desired access mask was provided.
    //


    EffectiveDesiredAccess = HandleInformation.GrantedAccess;



    //
    //  Filter the existing token
    //

    NewToken = NULL;
    Status = SepFilterToken(
                 Token,
                 PreviousMode,
                 Flags,
                 CapturedGroupCount,
                 CapturedGroups,
                 CapturedPrivilegeCount,
                 CapturedPrivileges,
                 CapturedSidCount,
                 CapturedSids,
                 CapturedSidsLength,
                 &NewToken
                 );


    if (NT_SUCCESS(Status)) {

        //
        //  Insert the new token
        //

        Status = ObInsertObject( NewToken,
                                 NULL,
                                 EffectiveDesiredAccess,
                                 0,
                                 (PVOID *)NULL,
                                 &LocalHandle
                                 );

    }

    //
    //  We no longer need our reference to the source token
    //

    ObDereferenceObject( (PVOID)Token );


    //
    //  Return the new handle
    //

    if (NT_SUCCESS(Status)) {
        try { *NewTokenHandle = LocalHandle; }
            except(EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
            }
    }

Cleanup:

    if (CapturedGroups != NULL) {
        SeReleaseSidAndAttributesArray(
            CapturedGroups,
            PreviousMode,
            TRUE
            );
    }

    if (CapturedPrivileges != NULL) {
        SeReleaseLuidAndAttributesArray(
            CapturedPrivileges,
            PreviousMode,
            TRUE
            );
    }

    if (CapturedSids != NULL) {
        SeReleaseSidAndAttributesArray(
            CapturedSids,
            PreviousMode,
            TRUE
            );
    }

   return Status;
}


NTSTATUS
SeFilterToken (
    __in PACCESS_TOKEN ExistingToken,
    __in ULONG Flags,
    __in_opt PTOKEN_GROUPS SidsToDisable,
    __in_opt PTOKEN_PRIVILEGES PrivilegesToDelete,
    __in_opt PTOKEN_GROUPS RestrictedSids,
    __deref_out PACCESS_TOKEN * NewToken
    )
/*++


Routine Description:

    Create a new token that is a subset of an existing token.

Arguments:

    ExistingToken - Is a  token already open for
        TOKEN_DUPLICATE access.

    Flags - Flags indicating additional filtering. The flags may be:

                DISABLE_MAX_PRIVILEGE - Disable all privileges
                SANDBOX_INERT - Record this SAIFER flag in the token.


    SidsToDisable - Contains a list of sids and attributes. All sids with
        the USE_FOR_DENY_ONLY attribute that also exist in the token will
        cause the new token to have that sid set with the USE_FOR_DENY_ONLY
        attribute.

    PrivilegesToDelete - Privileges in this list that are present in the
        existing token will not exist in the final token. This is similar
        to duplicating a token effective only with these privileges set to
        disabled.

    RestrictedSids - Contains a list of SIDs and attributes that will be
        stored in the RestrictedSids field of the new token. These SIDs
        are used after a normal access check to further restrict access.
        The attributes of these groups are always SE_GROUP_MANDATORY |
        SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT. If there already
        exist RestrictedSids in the original token, these sids will be
        appended.

    NewToken - Receives a pointer to the newly created token.

Return Value:

    STATUS_SUCCESS - Indicates the operation was successful.

    STATUS_INVALID_PARAMETER - Indicates one or more of the parameter values
        was invalid.  This value is returned if the target token is not
        an impersonation token.


--*/
{

    PTOKEN Token;
    PTOKEN FilteredToken = NULL;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Index;

    ULONG CapturedSidCount = 0;
    PSID_AND_ATTRIBUTES CapturedSids = NULL;
    ULONG CapturedSidsLength = 0;

    ULONG CapturedGroupCount = 0;
    PSID_AND_ATTRIBUTES CapturedGroups = NULL;
    ULONG CapturedGroupsLength = 0;

    ULONG CapturedPrivilegeCount = 0;
    PLUID_AND_ATTRIBUTES CapturedPrivileges = NULL;
    ULONG CapturedPrivilegesLength = 0;

    HANDLE LocalHandle;

    OBJECT_HANDLE_INFORMATION HandleInformation;
    ACCESS_MASK EffectiveDesiredAccess;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    //
    //  Probe parameters
    //

    *NewToken = NULL;


    //
    //  Capture Sids to remove
    //

    if (ARGUMENT_PRESENT(SidsToDisable)) {

        CapturedGroupCount = SidsToDisable->GroupCount;
        CapturedGroups = SidsToDisable->Groups;

    }

    //
    //  Capture PrivilegesToDelete
    //

    if (ARGUMENT_PRESENT(PrivilegesToDelete)) {

        CapturedPrivilegeCount = PrivilegesToDelete->PrivilegeCount;
        CapturedPrivileges = PrivilegesToDelete->Privileges;

    }

    //
    //  Capture Restricted Sids
    //

    if (ARGUMENT_PRESENT(RestrictedSids)) {

        CapturedSidCount = RestrictedSids->GroupCount;
        CapturedSids = RestrictedSids->Groups;

        //
        // Check that the attributes are all zero for the restricted sids
        //

        for (Index = 0; Index < CapturedSidCount ; Index++ ) {
            if (CapturedSids[Index].Attributes != 0) {
                return(STATUS_INVALID_PARAMETER);
            }
        }

    }



    //
    //  Check the handle's access to the existing token and get
    //  a pointer to that token.  Pick up the default desired
    //  access mask from the handle while we're at it.
    //

    Token = (PTOKEN) ExistingToken;

    //
    //  Filter the existing token
    //

    Status = SepFilterToken(
                 Token,
                 KernelMode,
                 Flags,
                 CapturedGroupCount,
                 CapturedGroups,
                 CapturedPrivilegeCount,
                 CapturedPrivileges,
                 CapturedSidCount,
                 CapturedSids,
                 CapturedSidsLength,
                 &FilteredToken
                 );


    if (NT_SUCCESS(Status)) {

        //
        //  Insert the new token
        //

        Status = ObInsertObject( FilteredToken,
                                 NULL,
                                 0,
                                 0,
                                 NULL,
                                 NULL
                                 );

        if (NT_SUCCESS( Status )) {

            *NewToken = FilteredToken;

        } else {

            //
            //  ObInsertObject dereferences the passed object on failure
            //  so we don't have to clean up here.
            //

        }
    }

   return Status;
}

NTSTATUS
SeFastFilterToken(
    __in PACCESS_TOKEN ExistingToken,
    __in KPROCESSOR_MODE RequestorMode,
    __in ULONG Flags,
    __in ULONG GroupCount,
    __in_ecount_opt(GroupCount) PSID_AND_ATTRIBUTES GroupsToDisable,
    __in ULONG PrivilegeCount,
    __in_ecount_opt(PrivilegeCount) PLUID_AND_ATTRIBUTES PrivilegesToDelete,
    __in ULONG SidCount,
    __in_ecount_opt( SidCount ) PSID_AND_ATTRIBUTES RestrictedSids,
    __in ULONG SidLength,
    __deref_out PACCESS_TOKEN * FilteredToken
    )
/*++

Routine Description:

    This is a fast wrapper for the Ps code to filter a token
    inline of an impersonate.

    This routine acquires a read lock on the token being filtered.

Arguments:

    ExistingToken - Points to the token to be duplicated.

    RequestorMode - Mode of client requesting the token be duplicated.

    Flags - Flags indicating additional filtering. The flags may be:

                DISABLE_MAX_PRIVILEGE - Disable all privileges
                SANDBOX_INERT - Record this SAIFER flag in the token.

    GroupCount - Count of groups to disable

    GroupsToDisable - Contains a list of sids and attributes. All sids with
        the USE_FOR_DENY_ONLY attribute that also exist in the token will
        cause the new token to have that sid set with the USE_FOR_DENY_ONLY
        attribute.

    PrivilegeCount - Count of privileges to delete

    PrivilegesToDelete - Privileges in this list that are present in the
        existing token will not exist in the final token. This is similar
        to duplicating a token effective only with these privileges set to
        disabled.

    SidCount - Count of restricted sids to add.

    RestrictedSids - Contains a list of SIDs and attributes that will be
        stored in the RestrictedSids field of the new token. These SIDs
        are used after a normal access check to further restrict access.
        The attributes of these groups are always SE_GROUP_MANDATORY |
        SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT. If there already
        exist RestrictedSids in the original token, these sids will be
        appended.

    SidLength - Length of added restricted sids.

    FilteredToken - Receives a pointer to the duplicate token.
        The token has not yet been inserted into any object table.
        No exceptions are expected when trying to set this OUT value.

Return Value:

    STATUS_SUCCESS - The service successfully completed the requested
        operation.


--*/
{
    NTSTATUS Status;
    PTOKEN NewToken;

    Status = SepFilterToken( (PTOKEN) ExistingToken,
                             RequestorMode,
                             Flags,
                             GroupCount,
                             GroupsToDisable,
                             PrivilegeCount,
                             PrivilegesToDelete,
                             SidCount,
                             RestrictedSids,
                             SidLength,
                             &NewToken );

    if (NT_SUCCESS (Status)) {
        //
        //  Insert the new token
        //

        Status = ObInsertObject( NewToken,
                                 NULL,
                                 0,
                                 0,
                                 NULL,
                                 NULL
                                 );

    }

    if (NT_SUCCESS( Status )) {
        *FilteredToken = (PACCESS_TOKEN) NewToken;
    } else {
        *FilteredToken = NULL;
    }
    return Status;
}



NTSTATUS
SepFilterToken(
    __in PTOKEN ExistingToken,
    __in KPROCESSOR_MODE RequestorMode,
    __in ULONG Flags,
    __in ULONG GroupCount,
    __in_ecount_opt(GroupCount) PSID_AND_ATTRIBUTES GroupsToDisable,
    __in ULONG PrivilegeCount,
    __in_ecount_opt(PrivilegeCount) PLUID_AND_ATTRIBUTES PrivilegesToDelete,
    __in ULONG SidCount,
    __in_ecount_opt(SidCount) PSID_AND_ATTRIBUTES RestrictedSids,
    __in ULONG SidLength,
    __deref_out PTOKEN * FilteredToken
    )
/*++


Routine Description:

    This routine does the bulk of the work to actually filter
    a token.  This routine assumes all access validation and argument
    probing has been performed.

    THE CALLER IS RESPONSIBLE FOR CHECKING SUBJECT RIGHTS TO CREATE THE
    TYPE OF TOKEN BEING CREATED.

    This routine acquires a read lock on the token being filtered.

Arguments:

    ExistingToken - Points to the token to be duplicated.

    RequestorMode - Mode of client requesting the token be duplicated.

    Flags - Flags indicating additional filtering. The flags may be:

                DISABLE_MAX_PRIVILEGE - Disable all privileges
                SANDBOX_INERT - Record this SAIFER flag in the token.

    GroupCount - Count of groups to disable

    GroupsToDisable - Contains a list of sids and attributes. All sids with
        the USE_FOR_DENY_ONLY attribute that also exist in the token will
        cause the new token to have that sid set with the USE_FOR_DENY_ONLY
        attribute.

    PrivilegeCount - Count of privileges to delete

    PrivilegesToDelete - Privileges in this list that are present in the
        existing token will not exist in the final token. This is similar
        to duplicating a token effective only with these privileges set to
        disabled.

    SidCount - Count of restricted sids to add.

    RestrictedSids - Contains a list of SIDs and attributes that will be
        stored in the RestrictedSids field of the new token. These SIDs
        are used after a normal access check to further restrict access.
        The attributes of these groups are always SE_GROUP_MANDATORY |
        SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT. If there already
        exist RestrictedSids in the original token, the intersection of the
        two sets will be in the final tokense sids will be.

    SidLength - Length of added restricted sids.

    FilteredToken - Receives a pointer to the duplicate token.
        The token has not yet been inserted into any object table.
        No exceptions are expected when trying to set this OUT value.

Return Value:

    STATUS_SUCCESS - The service successfully completed the requested
        operation.


--*/
{
    NTSTATUS Status;

    PTOKEN NewToken;
    PULONG DynamicPart;
    ULONG PagedPoolSize;
    ULONG NonPagedPoolSize;
    ULONG TokenBodyLength;
    ULONG FieldOffset;
    ULONG_PTR NextFree;
    PSID NextSidFree;
    ULONG VariableLength;
    ULONG Pad;
    ULONG DynamicSize;

    ULONG Index;

    PSECURITY_TOKEN_PROXY_DATA NewProxyData;
    PSECURITY_TOKEN_AUDIT_DATA NewAuditData;
    OBJECT_ATTRIBUTES ObjA ;

    PERESOURCE TokenLock;

#if DBG || TOKEN_LEAK_MONITOR
    ULONG Frames;
#endif

    PAGED_CODE();

    ASSERT( sizeof(SECURITY_IMPERSONATION_LEVEL) <= sizeof(ULONG) );



    if (ARGUMENT_PRESENT(ExistingToken->ProxyData)) {

        Status = SepCopyProxyData(
                    &NewProxyData,
                    ExistingToken->ProxyData
                    );

        if (!NT_SUCCESS(Status)) {
            return( Status );
        }

    } else {

        NewProxyData = NULL;
    }

    if (ARGUMENT_PRESENT( ExistingToken->AuditData )) {

        NewAuditData = ExAllocatePool( PagedPool, sizeof( SECURITY_TOKEN_AUDIT_DATA ));

        if (NewAuditData == NULL) {

            SepFreeProxyData( NewProxyData );
            return( STATUS_INSUFFICIENT_RESOURCES );

        } else {

            *NewAuditData = *(ExistingToken->AuditData);
        }

    } else {

        NewAuditData = NULL;

    }

    TokenLock = (PERESOURCE)ExAllocatePoolWithTag( NonPagedPool, sizeof( ERESOURCE ), 'dTeS' );

    if (TokenLock == NULL) {

        if (NewAuditData != NULL) {
            ExFreePool( NewAuditData );
        }

        SepFreeProxyData( NewProxyData );

        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    //
    //  Create a new object
    //

    VariableLength = ExistingToken->VariableLength + SidLength;

#if defined(_WIN64)

    //
    // Account for some additional alignment requirements later on.
    //

    VariableLength += sizeof(ULONG);

#endif

    TokenBodyLength = FIELD_OFFSET(TOKEN, VariablePart) +
                      VariableLength;

    NonPagedPoolSize = TokenBodyLength;
    PagedPoolSize    = ExistingToken->DynamicCharged;

    InitializeObjectAttributes( &ObjA, NULL, 0, NULL, NULL );

    Status = ObCreateObject(
                 RequestorMode,      // ProbeMode
                 SeTokenObjectType, // ObjectType
                 NULL,               // ObjectAttributes
                 RequestorMode,      // OwnershipMode
                 NULL,               // ParseContext
                 TokenBodyLength,    // ObjectBodySize
                 PagedPoolSize,      // PagedPoolCharge
                 NonPagedPoolSize,   // NonPagedPoolCharge
                 (PVOID *)&NewToken  // Return pointer to object
                 );

    if (!NT_SUCCESS(Status)) {
        SepFreeProxyData( NewProxyData );
        ExFreePool( TokenLock );

        if (NewAuditData != NULL) {
            ExFreePool( NewAuditData );
        }

        return Status;
    }

    //
    // The following fields differ in the new token.
    //

    NewToken->TokenLock = TokenLock;
    ExInitializeResourceLite( NewToken->TokenLock );

    //
    // Allocate a new modified Id to distinguish this token from the original
    // token.
    //

    ExAllocateLocallyUniqueId( &(NewToken->ModifiedId) );
    ExAllocateLocallyUniqueId( &(NewToken->TokenId) );
    NewToken->TokenInUse = FALSE;
    NewToken->AuthenticationId = ExistingToken->AuthenticationId;
    NewToken->TokenSource = ExistingToken->TokenSource;
    NewToken->DynamicAvailable = 0;
    NewToken->RestrictedSidCount = 0;
    NewToken->VariableLength = VariableLength;
    NewToken->ProxyData = NewProxyData;
    NewToken->AuditData = NewAuditData;
    NewToken->ParentTokenId = ExistingToken->TokenId;
    NewToken->TokenType = ExistingToken->TokenType;
    NewToken->ImpersonationLevel = ExistingToken->ImpersonationLevel;
    NewToken->ExpirationTime = ExistingToken->ExpirationTime;
    NewToken->OriginatingLogonSession  = ExistingToken->OriginatingLogonSession ;

    //
    //  acquire exclusive access to the source token
    //

    SepAcquireTokenReadLock( ExistingToken );


    //
    // Main Body initialization
    //

    //
    // The following fields are unchanged from the source token.
    // Although some may change if EffectiveOnly has been specified.
    //

    NewToken->DynamicCharged = ExistingToken->DynamicCharged;
    NewToken->DefaultOwnerIndex = ExistingToken->DefaultOwnerIndex;
    NewToken->UserAndGroupCount = ExistingToken->UserAndGroupCount;
    NewToken->SessionId = ExistingToken->SessionId;
    NewToken->PrivilegeCount = ExistingToken->PrivilegeCount;
    NewToken->TokenFlags = ExistingToken->TokenFlags & ~TOKEN_SESSION_NOT_REFERENCED;
    NewToken->AuditPolicy = ExistingToken->AuditPolicy;

    //
    // Increment the reference count for this logon session
    // This can not fail, since there is already a token in this logon
    // session.
    //

    Status = SepDuplicateLogonSessionReference (NewToken, ExistingToken);

    ASSERT( NT_SUCCESS(Status) );

    if (!NT_SUCCESS (Status)) {
        SepReleaseTokenReadLock( ExistingToken );

        NewToken->DynamicPart = NULL;
        ObDereferenceObject (NewToken);
        return Status;
    }


#if DBG || TOKEN_LEAK_MONITOR

    NewToken->ProcessCid          = PsGetCurrentThread()->Cid.UniqueProcess;
    NewToken->ThreadCid           = PsGetCurrentThread()->Cid.UniqueThread;
    NewToken->CreateMethod        = 0xF; // Filter
    NewToken->Count               = 0;
    NewToken->CaptureCount        = 0;

    RtlCopyMemory(
        NewToken->ImageFileName,
        PsGetCurrentProcess()->ImageFileName, 
        min(sizeof(NewToken->ImageFileName), sizeof(PsGetCurrentProcess()->ImageFileName))
        );

    Frames = RtlWalkFrameChain(
                 (PVOID)NewToken->CreateTrace,
                 TRACE_SIZE,
                 0
                 );


    if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
        
        RtlWalkFrameChain(
            (PVOID)&NewToken->CreateTrace[Frames],
            TRACE_SIZE - Frames,
            1
            );
    }

    SepAddTokenLogonSession(NewToken);

#endif

    //
    // If the caller passed in the sandbox inert flag then record it.
    //

    if ((Flags & SANDBOX_INERT) != 0) {
        NewToken->TokenFlags |= TOKEN_SANDBOX_INERT;
    }


    //
    // Compute the beginning portion of the variable part, which contains the
    // sid & attributes arrays and the privilege set.
    //

    //
    // First copy the privileges. We will later remove the ones that are
    // to be deleted.
    //

    NextFree = (ULONG_PTR)(&NewToken->VariablePart);
    NewToken->Privileges = (PLUID_AND_ATTRIBUTES)NextFree;
    RtlCopyLuidAndAttributesArray( ExistingToken->PrivilegeCount,
                                   ExistingToken->Privileges,
                                   (PLUID_AND_ATTRIBUTES)NextFree
                                   );

    NextFree += (ExistingToken->PrivilegeCount * (ULONG)sizeof(LUID_AND_ATTRIBUTES));
    VariableLength -= ( (ExistingToken->PrivilegeCount * (ULONG)sizeof(LUID_AND_ATTRIBUTES)) );

#if defined(_WIN64)

    //
    // At this point NextFree is 4-byte aligned, so no alignment work
    // is necessary for the 32-bit product.  For Win64, ensure that NextFree
    // is 8-byte aligned.
    //

    Pad = (ULONG)(NextFree & (sizeof(PVOID)-1));
    if (Pad != 0) {
        Pad = sizeof(PVOID) - Pad;
        NextFree += Pad;
        VariableLength -= Pad;
    }

#endif

    //
    // Figure out the count of SIDs. This is the count of users&groups +
    // the number of existing restricted SIDs plus the number of new
    // restricted Sids
    //

#define MAX(_x_,_y_) ((_x_) > (_y_) ? (_x_) : (_y_))

    NextSidFree = (PSID) (NextFree + (ExistingToken->UserAndGroupCount +
                                      MAX(ExistingToken->RestrictedSidCount,SidCount)) * sizeof(SID_AND_ATTRIBUTES));

    NewToken->UserAndGroups = (PSID_AND_ATTRIBUTES) NextFree;

    //
    // Copy in the existing users & groups. We will later flag the ones
    // to be disabled.
    //

    Status = RtlCopySidAndAttributesArray(
                 ExistingToken->UserAndGroupCount,
                 ExistingToken->UserAndGroups,
                 VariableLength,
                 (PSID_AND_ATTRIBUTES)NextFree,
                 NextSidFree,
                 &NextSidFree,
                 &VariableLength
                 );


    ASSERT(NT_SUCCESS(Status));
    NextFree += (ExistingToken->UserAndGroupCount * (ULONG)sizeof(SID_AND_ATTRIBUTES));

    //
    // Now add all the existing restricted sids. We need to take the
    // intersection of the two sets.
    //

    NewToken->RestrictedSids = (PSID_AND_ATTRIBUTES) NextFree;


    for (Index = 0; Index < SidCount ; Index++ ) {
        if ( ( ExistingToken->RestrictedSidCount == 0 ) ||
            SepSidInSidAndAttributes(
                ExistingToken->RestrictedSids,
                ExistingToken->RestrictedSidCount,
                NULL,                           // no self sid
                RestrictedSids[Index].Sid
                )) {

            Status = RtlCopySidAndAttributesArray(
                        1,
                        &RestrictedSids[Index],
                        VariableLength,
                        (PSID_AND_ATTRIBUTES)NextFree,
                        NextSidFree,
                        &NextSidFree,
                        &VariableLength
                        );
            ASSERT(NT_SUCCESS(Status));
            NextFree += sizeof(SID_AND_ATTRIBUTES);
            NewToken->RestrictedSids[NewToken->RestrictedSidCount].Attributes =
                SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_MANDATORY;
            NewToken->RestrictedSidCount++;

        }
    }

    //
    //  Allocate the dynamic portion
    //
    DynamicSize = SeLengthSid( ExistingToken->PrimaryGroup );
    if (ExistingToken->DefaultDacl) {
        DynamicSize += ExistingToken->DefaultDacl->AclSize;
    }

    DynamicPart = (PULONG)ExAllocatePoolWithTag(
                              PagedPool,
                              DynamicSize,
                              'dTeS'
                              );

    NewToken->DynamicPart = DynamicPart;

    if (DynamicPart == NULL) {
        SepReleaseTokenReadLock( ExistingToken );
        ObDereferenceObject( NewToken );
        return( STATUS_INSUFFICIENT_RESOURCES );
    }
    //
    // Make sure the new token has some restrictions.
    // If it doesn't, then we've ended up with a token
    // that gives us more access than the original,
    // which we don't want.
    //

    if ((ExistingToken->RestrictedSidCount != 0) &&
        (NewToken->RestrictedSidCount == 0)) {

        SepReleaseTokenReadLock( ExistingToken );

        Status = STATUS_INVALID_PARAMETER;

        //
        // Cleanup.  ObDereferenceObject will cause the logon
        // session to be dereferenced, and will free the proxy data
        // as well as the audit data.
        //
        // See SepTokenDeleteMethod(), which is called by
        // the object manager when the token object is
        // being freed.
        //

        ObDereferenceObject( NewToken );

        return(Status);
    }

    //
    // If there are any restricted sids in the token, turn on the restricted
    // flag
    //

    if (NewToken->RestrictedSidCount > 0) {
        NewToken->TokenFlags |= TOKEN_IS_RESTRICTED;
    }

    //
    //  Copy and initialize the dynamic part.
    //  The dynamic part is assumed to be position independent.
    //

    RtlCopyMemory( (PVOID)DynamicPart,
                  (PVOID)(ExistingToken->DynamicPart),
                  DynamicSize
                  );


    //
    // If present, set the address of the default Dacl
    //

    if (ARGUMENT_PRESENT(ExistingToken->DefaultDacl)) {

        ASSERT( (ULONG_PTR)(ExistingToken->DefaultDacl) >=
                (ULONG_PTR)(ExistingToken->DynamicPart) );

        FieldOffset = (ULONG)((ULONG_PTR)(ExistingToken->DefaultDacl) -
                              (ULONG_PTR)(ExistingToken->DynamicPart));

        NewToken->DefaultDacl = (PACL)(FieldOffset + (ULONG_PTR)DynamicPart);

    } else {

        NewToken->DefaultDacl = NULL;
    }


    //
    // Set the address of the primary group
    //

    ASSERT(ARGUMENT_PRESENT(ExistingToken->PrimaryGroup));

    ASSERT( (ULONG_PTR)(ExistingToken->PrimaryGroup) >=
            (ULONG_PTR)(ExistingToken->DynamicPart) );

    FieldOffset = (ULONG)((ULONG_PTR)(ExistingToken->PrimaryGroup) -
                          (ULONG_PTR)(ExistingToken->DynamicPart));

    //
    // Release the source token.
    //

    SepReleaseTokenReadLock( ExistingToken );

    NewToken->PrimaryGroup = (PACL)(FieldOffset + (ULONG_PTR)(DynamicPart));

    //
    // For the time being, take the easy way to generating an "EffectiveOnly"
    // duplicate.  That is, use the same space required of the original, just
    // eliminate any IDs or privileges not active.
    //
    // Ultimately, if duplication becomes a common operation, then it will be
    // worthwhile to recalculate the actual space needed and copy only the
    // effective IDs/privileges into the new token.
    //

    SepRemoveDisabledGroupsAndPrivileges(
        NewToken,
        Flags,
        GroupCount,
        GroupsToDisable,
        PrivilegeCount,
        PrivilegesToDelete
        );

    //
    // If the NewToken inherited an active SEP_AUDIT_POLICY from ExistingToken,
    // then increment the counter of tokens with policies.
    //

    if ( NewToken->AuditPolicy.Overlay ) {
        SepModifyTokenPolicyCounter(&NewToken->AuditPolicy, TRUE);
    }

#if DBG || TOKEN_LEAK_MONITOR
    if (SepTokenLeakTracking && SepTokenLeakMethodWatch == 0xF && PsGetCurrentProcess()->UniqueProcessId == SepTokenLeakProcessCid) {
        
        NewToken->Count = InterlockedIncrement(&SepTokenLeakMethodCount);
        if (NewToken->Count >= SepTokenLeakBreakCount) {

            DbgPrint("\nToken number 0x%x = 0x%x\n", NewToken->Count, NewToken);
            DbgBreakPoint();
        }
    }
#endif

    (*FilteredToken) = NewToken;
    return Status;
}

