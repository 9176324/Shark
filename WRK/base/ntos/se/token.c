/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    token.c

Abstract:

   This module implements the initialization, open, duplicate and other
   services of the executive token object.

--*/

#include "pch.h"

#pragma hdrstop


BOOLEAN
SepComparePrivilegeAndAttributeArrays(
    IN PLUID_AND_ATTRIBUTES PrivilegeArray1,
    IN ULONG Count1,
    IN PLUID_AND_ATTRIBUTES PrivilegeArray2,
    IN ULONG Count2
    );

BOOLEAN
SepCompareSidAndAttributeArrays(
    IN PSID_AND_ATTRIBUTES SidArray1,
    IN ULONG Count1,
    IN PSID_AND_ATTRIBUTES SidArray2,
    IN ULONG Count2
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SeTokenType)
#pragma alloc_text(PAGE,SeTokenIsAdmin)
#pragma alloc_text(PAGE,SeTokenIsRestricted)
#pragma alloc_text(PAGE,SeTokenImpersonationLevel)
#pragma alloc_text(PAGE,SeAssignPrimaryToken)
#pragma alloc_text(PAGE,SeDeassignPrimaryToken)
#pragma alloc_text(PAGE,SeExchangePrimaryToken)
#pragma alloc_text(PAGE,SeGetTokenControlInformation)
#pragma alloc_text(INIT,SeMakeSystemToken)
#pragma alloc_text(INIT,SeMakeAnonymousLogonToken)
#pragma alloc_text(INIT,SeMakeAnonymousLogonTokenNoEveryone)
#pragma alloc_text(PAGE,SeSubProcessToken)
#pragma alloc_text(INIT,SepTokenInitialization)
#pragma alloc_text(PAGE,NtCreateToken)
#pragma alloc_text(PAGE,SepTokenDeleteMethod)
#pragma alloc_text(PAGE,SepCreateToken)
#pragma alloc_text(PAGE,SepIdAssignableAsOwner)
#pragma alloc_text(PAGE,SeIsChildToken)
#pragma alloc_text(PAGE,SeIsChildTokenByPointer)
#pragma alloc_text(PAGE,SeIsSiblingToken)
#pragma alloc_text(PAGE,SeIsSiblingTokenByPointer)
#pragma alloc_text(PAGE,NtImpersonateAnonymousToken)
#pragma alloc_text(PAGE,NtCompareTokens)
#pragma alloc_text(PAGE,SepComparePrivilegeAndAttributeArrays)
#pragma alloc_text(PAGE,SepCompareSidAndAttributeArrays)
#pragma alloc_text(PAGE,SeAddSaclToProcess)
#endif


////////////////////////////////////////////////////////////////////////
//                                                                    //
//           Global Variables                                         //
//                                                                    //
////////////////////////////////////////////////////////////////////////


//
// Generic mapping of access types
//

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#pragma const_seg("INITCONST")
#endif

const GENERIC_MAPPING SepTokenMapping = { TOKEN_READ,
                                    TOKEN_WRITE,
                                    TOKEN_EXECUTE,
                                    TOKEN_ALL_ACCESS
                                  };

//
// Address of token object type descriptor.
//

POBJECT_TYPE SeTokenObjectType = NULL;


//
// Used to track whether or not a system token has been created or not.
//

#if DBG
BOOLEAN SystemTokenCreated = FALSE;
#endif //DBG


//
// Used to control the active token diagnostic support provided
//

#ifdef    TOKEN_DIAGNOSTICS_ENABLED
ULONG TokenGlobalFlag = 0;
#endif // TOKEN_DIAGNOSTICS_ENABLED




////////////////////////////////////////////////////////////////////////
//                                                                    //
//           Token Object Routines & Methods                          //
//                                                                    //
////////////////////////////////////////////////////////////////////////




TOKEN_TYPE
SeTokenType(
    __in PACCESS_TOKEN Token
    )

/*++

Routine Description:

    This function returns the type of an instance of a token (TokenPrimary,
    or TokenImpersonation).


Arguments:

    Token - Points to the token whose type is to be returned.

Return Value:

    The token's type.

--*/

{
    PAGED_CODE();

    return (((PTOKEN)Token)->TokenType);
}



NTKERNELAPI
BOOLEAN
SeTokenIsAdmin(
    __in PACCESS_TOKEN Token
    )

/*++

Routine Description:

    Returns if the token is a member of the local admin group.

Arguments:

    Token - Points to the token.

Return Value:

    TRUE - Token contains the local admin group
    FALSE - no admin.

--*/

{
    PAGED_CODE();

    return ((((PTOKEN)Token)->TokenFlags & TOKEN_HAS_ADMIN_GROUP) != 0 );
}


NTKERNELAPI
NTSTATUS
SeTokenCanImpersonate(
    __in PACCESS_TOKEN ProcessToken,
    __in PACCESS_TOKEN Token,
    __in SECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    )

/*++

Routine Description:

    Determines if the process token is allowed to impersonate the 
    second token, assuming that the access rights check has already passed.
    
Arguments:

    Token - Points to the token.

Return Value:

    TRUE - Token contains the local admin group
    FALSE - no admin.

--*/

{
    PTOKEN PrimaryToken = (PTOKEN) ProcessToken ;
    PTOKEN ImpToken = (PTOKEN) Token ;
    PSID PrimaryUserSid ;
    PSID ImpUserSid ;
    NTSTATUS Status ;

    PAGED_CODE();

    if ( ImpersonationLevel < SecurityImpersonation )
    {
        return STATUS_SUCCESS ;
    }

    //
    // allow impersonating anonymous tokens
    //

    if (RtlEqualLuid(&ImpToken->AuthenticationId, &SeAnonymousAuthenticationId)) 
    {
        return STATUS_SUCCESS ;
    }

    SepAcquireTokenReadLock( PrimaryToken );

    if ((PrimaryToken->TokenFlags & TOKEN_HAS_IMPERSONATE_PRIVILEGE) != 0 )
    {
        SepReleaseTokenReadLock( PrimaryToken );

        return STATUS_SUCCESS ;
    }

    SepAcquireTokenReadLock( ImpToken );

    Status = STATUS_PRIVILEGE_NOT_HELD ;

    if ( RtlEqualLuid( &PrimaryToken->AuthenticationId, &ImpToken->OriginatingLogonSession ) )
    {
        Status = STATUS_SUCCESS ;

    }
    else
    {
        PrimaryUserSid = PrimaryToken->UserAndGroups[0].Sid ;
        ImpUserSid = ImpToken->UserAndGroups[0].Sid ;

        if ( RtlEqualSid( PrimaryUserSid, ImpUserSid ) )
        {
            //
            // The tokens are representing the same user.  If the primary token
            // is restricted and the impersonation token is not, then the process
            // is attempting to elevate to the nonrestricted version of itself.  Do
            // not allow this.
            //
            if (SeTokenIsRestricted(PrimaryToken) && !SeTokenIsRestricted(ImpToken) ) {
                Status = STATUS_PRIVILEGE_NOT_HELD;
            } else {
                Status = STATUS_SUCCESS ;
            }

        }

    }

    SepReleaseTokenReadLock( ImpToken );
    SepReleaseTokenReadLock( PrimaryToken );

#if DBG
    if ( !NT_SUCCESS( Status ) )
    {
        DbgPrint( "Process %x.%x not allowed to impersonate!  Returning %x\n", PsGetCurrentThread()->Cid.UniqueProcess,
            PsGetCurrentThread()->Cid.UniqueThread, Status );
        
    }
#endif 

    return Status ;
}



NTKERNELAPI
BOOLEAN
SeTokenIsRestricted(
    __in PACCESS_TOKEN Token
    )

/*++

Routine Description:

    Returns if the token is a restricted token.

Arguments:

    Token - Points to the token.

Return Value:

    TRUE - Token contains restricted sids
    FALSE - no admin.

--*/

{
    PAGED_CODE();

    return ((((PTOKEN)Token)->TokenFlags & TOKEN_IS_RESTRICTED) != 0 );
}



SECURITY_IMPERSONATION_LEVEL
SeTokenImpersonationLevel(
    __in PACCESS_TOKEN Token
    )

/*++

Routine Description:

    This function returns the impersonation level of a token.  The token
    is assumed to be a TokenImpersonation type token.


Arguments:

    Token - Points to the token whose impersonation level is to be returned.

Return Value:

    The token's impersonation level.

--*/

{
    PAGED_CODE();

    return ((PTOKEN)Token)->ImpersonationLevel;
}


BOOLEAN
SepCheckTokenForCoreSystemSids(
    __in PACCESS_TOKEN Token
    )
/*++

Routine Description:

    Perform an access-check against SepImportantProcessSd to
    determine if the passed token has at least one of the sids present
    in the ACEs of SepImportantProcessSd.

Arguments:

    Token - a token

Return Value:

    TRUE if Token has at least one of the required SIDs,
    FALSE otherwise

Notes:

--*/
{
    ACCESS_MASK GrantedAccess = 0;
    NTSTATUS AccessStatus = STATUS_ACCESS_DENIED;
    
    PAGED_CODE();
    
    (void) SepAccessCheck(
               SepImportantProcessSd,
               NULL,
               Token,
               NULL,
               SEP_QUERY_MEMBERSHIP,
               NULL,
               0,
               &GenericMappingForMembershipCheck,
               0,
               KernelMode,
               &GrantedAccess,
               NULL,
               &AccessStatus,
               0,
               NULL,
               NULL
               );

    return AccessStatus == STATUS_SUCCESS;
}


VOID
SeAddSaclToProcess(
    __in PEPROCESS Process,
    __in PACCESS_TOKEN Token,
    __in PVOID Reserved
    )
/*++

Routine Description:

    If 'Token' has at least one of the sids present in the ACEs
    of SepImportantProcessSd, add a SACL to the security descriptor
    of 'Process' as defined by SepProcessAuditSd.

Arguments:

    Process - process to add SACL to

    Token - token to examine

Return Value:

    None

Notes:

--*/
{
    NTSTATUS Status;
    SECURITY_INFORMATION SecurityInformationSacl = SACL_SECURITY_INFORMATION;
    POBJECT_HEADER ObjectHeader;

    PAGED_CODE();


    // quickly return if this feature is disabled
    // (indicated by SeProcessAuditSd == NULL)
    //

    if ( SepProcessAuditSd == NULL ) {
        return;
    }

    //
    // if the token does not have core system sids then return
    // without adding SACL.
    // (see comment on SepImportantProcessSd in seglobal.c for more info)
    //

    if (!SepCheckTokenForCoreSystemSids( Token )) {
        return;
    }
    
    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Process );

    //
    // add SACL to existing security descriptor on 'Process'
    //

    Status = ObSetSecurityDescriptorInfo(
                 Process,
                 &SecurityInformationSacl,
                 SepProcessAuditSd,
                 &ObjectHeader->SecurityDescriptor,
                 NonPagedPool,
                 &ObjectHeader->Type->TypeInfo.GenericMapping
                 );

    if (!NT_SUCCESS( Status )) {

        //
        // STATUS_NO_SECURITY_ON_OBJECT should be returned only once during
        // boot when the initial system process is created.
        //

        if ( Status != STATUS_NO_SECURITY_ON_OBJECT ) {
            
            ASSERT( L"SeAddSaclToProcess: ObSetSecurityDescriptorInfo failed" &&
                    FALSE );

            //
            // this will bugcheck if SepCrashOnAuditFail is TRUE
            //

            SepAuditFailed( Status );
        }
    }
}


VOID
SeAssignPrimaryToken(
    __in PEPROCESS Process,
    __in PACCESS_TOKEN Token
    )


/*++

Routine Description:

    This function establishes a primary token for a process.

Arguments:

    Token - Points to the new primary token.

Return Value:

    None.

--*/

{
    NTSTATUS
        Status;

    PTOKEN
        NewToken = (PTOKEN)Token;

    PAGED_CODE();

    ASSERT(NewToken->TokenType == TokenPrimary);
    ASSERT( !NewToken->TokenInUse );


    //
    // audit the assignment of a primary token, if requested
    //

    if (SeDetailedAuditingWithToken(NULL)) {
        SepAuditAssignPrimaryToken( Process, Token );
    }

    //
    // If the token being assigned to the child process has
    // any one of the following SIDs, then the process
    // is considered to be a system process:
    // -- SeLocalSystemSid
    // -- SeLocalServiceSid
    // -- SeNetworkServiceSid
    //
    // For such a process, add SACL to its security descriptor
    // if that option is enabled. If the option is disabled,
    // this function returns very quickly.
    //

    //
    // Dereference the old token if there is one.
    //
    // Processes typically already have a token that must be
    // dereferenced.  There are two cases where this may not
    // be the situation.  First, during phase 0 system initialization,
    // the initial system process starts out without a token.  Second,
    // if an error occurs during process creation, we may be cleaning
    // up a process that hasn't yet had a primary token assigned.
    //

    if (!ExFastRefObjectNull (Process->Token)) {
        SeDeassignPrimaryToken( Process );
    }

    ObReferenceObject(NewToken);
    NewToken->TokenInUse = TRUE;

    ObInitializeFastReference (&Process->Token, Token);
    return;
}



VOID
SeDeassignPrimaryToken(
    __in PEPROCESS Process
    )


/*++

Routine Description:

    This function causes a process reference to a token to be
    dropped.

Arguments:

    Process - Points to the process whose primary token is no longer needed.
        This is probably only the case at process deletion or when
        a primary token is being replaced.

Return Value:

    None.

--*/

{

    PTOKEN
        OldToken = (PTOKEN) ObFastReplaceObject (&Process->Token, NULL);

    PAGED_CODE();

    ASSERT(OldToken->TokenType == TokenPrimary);
    ASSERT(OldToken->TokenInUse);

    OldToken->TokenInUse = FALSE;
    ObDereferenceObject( OldToken );


    return;
}



NTSTATUS
SeExchangePrimaryToken(
    __in PEPROCESS Process,
    __in PACCESS_TOKEN NewAccessToken,
    __deref_out PACCESS_TOKEN *OldAccessToken
    )


/*++

Routine Description:

    This function is used to perform the portions of changing a primary
    token that reference the internals of token structures.

    The new token is checked to make sure it is not already in use.


Arguments:

    Process - Points to the process whose primary token is being exchanged.

    NewAccessToken - Points to the process's new primary token.

    OldAccessToken - Receives a pointer to the process's current token.
        The caller is responsible for dereferencing this token when
        it is no longer needed.  This can't be done while the process
        security locks are held.


Return Value:

    STATUS_SUCCESS - Everything has been updated.

    STATUS_TOKEN_ALREADY_IN_USE - A primary token can only be used by a
        single process.  That is, each process must have its own primary
        token.  The token passed to  be assigned as the primary token is
        already in use as a primary token.

    STATUS_BAD_TOKEN_TYPE - The new token is not a primary token.

    STATUS_NO_TOKEN - The process did not have any existing token. This should never happen.

--*/

{
    NTSTATUS
        Status;

    PTOKEN
        OldToken;

    PTOKEN
        NewToken = (PTOKEN)NewAccessToken;

    ULONG SessionId;

    PAGED_CODE();


    //
    // Make sure the new token is a primary token...
    //

    if (NewToken->TokenType != TokenPrimary) {
        return (STATUS_BAD_TOKEN_TYPE);
    }

    SessionId = MmGetSessionId (Process);

    //
    // Lock the new token so we can atomically test and set the InUse flag
    //

    SepAcquireTokenWriteLock (NewToken);

    //
    // and that it is not already in use...
    //

    if (NewToken->TokenInUse) {
        SepReleaseTokenWriteLock (NewToken, FALSE);
        return (STATUS_TOKEN_ALREADY_IN_USE);
    }

    NewToken->TokenInUse = TRUE;

    //
    // Ensure SessionId consistent for hydra
    //

    NewToken->SessionId = SessionId;

    SepReleaseTokenWriteLock (NewToken, FALSE);

    //
    // audit the assignment of a primary token, if requested
    //

    if (SeDetailedAuditingWithToken (NULL)) {
        SepAuditAssignPrimaryToken (Process, NewToken);
    }

    //
    // If the token being assigned to this process has
    // any one of the following SIDs, then the process
    // is considered to be a system process:
    // -- SeLocalSystemSid
    // -- SeLocalServiceSid
    // -- SeNetworkServiceSid
    //

    //
    // Switch the tokens
    //

    ObReferenceObject (NewToken);

    OldToken = ObFastReplaceObject (&Process->Token, NewToken);

    if (NULL == OldToken){
        return (STATUS_NO_TOKEN);
    }

    ASSERT (OldToken->TokenType == TokenPrimary);

    //
    // Lock the old token to clkear the InUse flag
    //

    SepAcquireTokenWriteLock (OldToken);

    ASSERT (OldToken->TokenInUse);

    //
    // Mark the token as "NOT USED"
    //

    OldToken->TokenInUse = FALSE;

    SepReleaseTokenWriteLock (OldToken, FALSE);

    //
    // Return the pointer to the old token.  The caller
    // is responsible for dereferencing it if they don't need it.
    //

    (*OldAccessToken) = OldToken;

    return (STATUS_SUCCESS);
}





VOID
SeGetTokenControlInformation (
    __in PACCESS_TOKEN Token,
    __out PTOKEN_CONTROL TokenControl
    )

/*++

Routine Description:

    This routine is provided for communication session layers, or
    any other executive component that needs to keep track of
    whether a caller's security context has changed between calls.
    Communication session layers will need to check this, for some
    security quality of service modes, to determine whether or not
    a server's security context needs to be updated to reflect
    changes in the client's security context.

    This routine will also be useful to communications subsystems
    that need to retrieve client' authentication information from
    the local security authority in order to perform a remote
    authentication.


Parameters:

    Token - Points to the token whose information is to be retrieved.

    TokenControl - Points to the buffer to receive the token control
        information.

Return Value:

    None.

--*/

{
    PAGED_CODE();

    //
    // Fetch readonly fields outside of the lock.
    //

    TokenControl->AuthenticationId = ((TOKEN *)Token)->AuthenticationId;
    TokenControl->TokenId = ((TOKEN *)Token)->TokenId;
    TokenControl->TokenSource = ((TOKEN *)Token)->TokenSource;

    //
    //  Acquire shared access to the token
    //

    SepAcquireTokenReadLock( (PTOKEN)Token );

    //
    //  Fetch data that may change
    //

    TokenControl->ModifiedId = ((TOKEN *)Token)->ModifiedId;

    SepReleaseTokenReadLock( (PTOKEN)Token );

    return;

}

PACCESS_TOKEN
SeMakeSystemToken ()

/*++

Routine Description:

    This routine is provided for use by executive components
    DURING SYSTEM INITIALIZATION ONLY.  It creates a token for
    use by system components.

    A system token has the following characteristics:

         - It has LOCAL_SYSTEM as its user ID

         - It has the following groups with the corresponding
           attributes:

               ADMINS_ALIAS      EnabledByDefault |
                                 Enabled          |
                                 Owner

               WORLD             EnabledByDefault |
                                 Enabled          |
                                 Mandatory

               ADMINISTRATORS (alias)  Owner   (disabled)

               AUTHENTICATED_USER
                                EnabledByDefault  |
                                Enabled           |
                                Mandatory


         - It has LOCAL_SYSTEM as its primary group.

         - It has the privileges shown in comments below.


         - It has protection that provides TOKEN_ALL_ACCESS to
           the LOCAL_SYSTEM ID.


         - It has a default ACL that grants GENERIC_ALL access
           to LOCAL_SYSTEM and GENERIC_EXECUTE to WORLD.


Parameters:

    None.

Return Value:

    Pointer to a system token.

--*/

{
    NTSTATUS Status;

    PVOID Token;

    SID_AND_ATTRIBUTES UserId;
    TOKEN_PRIMARY_GROUP PrimaryGroup;
    PSID_AND_ATTRIBUTES GroupIds;
    ULONG GroupIdsLength;
    LUID_AND_ATTRIBUTES Privileges[30];
    PACL TokenAcl;
    PSID Owner;
    ULONG NormalGroupAttributes;
    ULONG OwnerGroupAttributes;
    ULONG Length;
    OBJECT_ATTRIBUTES TokenObjectAttributes;
    PSECURITY_DESCRIPTOR TokenSecurityDescriptor;
    ULONG BufferLength;
    PVOID Buffer;

    ULONG_PTR GroupIdsBuffer[128 * sizeof(ULONG) / sizeof(ULONG_PTR)];

    TIME_FIELDS TimeFields;
    LARGE_INTEGER NoExpiration;

    PAGED_CODE();


    //
    // Make sure only one system token gets created.
    //

#if DBG
    ASSERT( !SystemTokenCreated );
    SystemTokenCreated = TRUE;
#endif //DBG


    //
    // Set up expiration times
    //

    TimeFields.Year = 3000;
    TimeFields.Month = 1;
    TimeFields.Day = 1;
    TimeFields.Hour = 1;
    TimeFields.Minute = 1;
    TimeFields.Second = 1;
    TimeFields.Milliseconds = 1;
    TimeFields.Weekday = 1;

    RtlTimeFieldsToTime( &TimeFields, &NoExpiration );


//    //
//    //  The amount of memory used in the following is gross overkill, but
//    //  it is freed up immediately after creating the token.
//    //
//
//    GroupIds = (PSID_AND_ATTRIBUTES)ExAllocatePool( NonPagedPool, 512 );

    GroupIds = (PSID_AND_ATTRIBUTES)GroupIdsBuffer;


    //
    // Set up the attributes to be assigned to groups
    //

    NormalGroupAttributes =    (SE_GROUP_MANDATORY          |
                                SE_GROUP_ENABLED_BY_DEFAULT |
                                SE_GROUP_ENABLED
                                );

    OwnerGroupAttributes  =    (SE_GROUP_ENABLED_BY_DEFAULT |
                                SE_GROUP_ENABLED            |
                                SE_GROUP_OWNER
                                );

    //
    // Set up the user ID
    //

    UserId.Sid = SeLocalSystemSid;
    UserId.Attributes = 0;

    //
    // Set up the groups
    //


    GroupIds->Sid  = SeAliasAdminsSid;
    (GroupIds+1)->Sid  = SeWorldSid;
    (GroupIds+2)->Sid  = SeAuthenticatedUsersSid;

    GroupIds->Attributes  = OwnerGroupAttributes;
    (GroupIds+1)->Attributes  = NormalGroupAttributes;
    (GroupIds+2)->Attributes  = NormalGroupAttributes;

    GroupIdsLength = (ULONG)LongAlignSize(SeLengthSid(GroupIds->Sid)) +
                     (ULONG)LongAlignSize(SeLengthSid((GroupIds+1)->Sid)) +
                     (ULONG)LongAlignSize(SeLengthSid((GroupIds+2)->Sid)) +
                     sizeof(SID_AND_ATTRIBUTES);

    ASSERT( GroupIdsLength <= 128 * sizeof(ULONG) );


    //
    // Privileges
    //

    //
    // The privileges in the system token are as follows:
    //
    //    Privilege Name                           Attributes
    //    --------------                           ----------
    //
    // SeTcbPrivilege                        enabled/enabled by default
    // SeCreateTokenPrivilege                DISabled/NOT enabled by default
    // SeTakeOwnershipPrivilege              DISabled/NOT enabled by default
    // SeCreatePagefilePrivilege             enabled/enabled by default
    // SeLockMemoryPrivilege                 enabled/enabled by default
    // SeAssignPrimaryTokenPrivilege         DISabled/NOT enabled by default
    // SeIncreaseQuotaPrivilege              DISabled/NOT enabled by default
    // SeIncreaseBasePriorityPrivilege       enabled/enabled by default
    // SeCreatePermanentPrivilege            enabled/enabled by default
    // SeDebugPrivilege                      enabled/enabled by default
    // SeAuditPrivilege                      enabled/enabled by default
    // SeSecurityPrivilege                   DISabled/NOT enabled by default
    // SeSystemEnvironmentPrivilege          DISabled/NOT enabled by default
    // SeChangeNotifyPrivilege               enabled/enabled by default
    // SeBackupPrivilege                     DISabled/NOT enabled by default
    // SeRestorePrivilege                    DISabled/NOT enabled by default
    // SeShutdownPrivilege                   DISabled/NOT enabled by default
    // SeLoadDriverPrivilege                 DISabled/NOT enabled by default
    // SeProfileSingleProcessPrivilege       enabled/enabled by default
    // SeSystemtimePrivilege                 DISabled/NOT enabled by default
    // SeUndockPrivilege                     DISabled/NOT enabled by default
    //
    // The following privileges are not present, and should never be present in
    // the local system token:
    //
    // SeRemoteShutdownPrivilege             no one can come in as local system
    // SeSyncAgentPrivilege                  only users specified by the admin can
    //                                       be sync agents
    // SeEnableDelegationPrivilege           only users specified by the admin can
    //                                       enable delegation on accounts.
    //

    Privileges[0].Luid = SeTcbPrivilege;
    Privileges[0].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |    // Enabled by default
         SE_PRIVILEGE_ENABLED);               // Enabled

    Privileges[1].Luid = SeCreateTokenPrivilege;
    Privileges[1].Attributes = 0;     // Only the LSA should enable this.

    Privileges[2].Luid = SeTakeOwnershipPrivilege;
    Privileges[2].Attributes = 0;

    Privileges[3].Luid = SeCreatePagefilePrivilege;
    Privileges[3].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |    // Enabled by default
         SE_PRIVILEGE_ENABLED);               // Enabled

    Privileges[4].Luid = SeLockMemoryPrivilege;
    Privileges[4].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |    // Enabled by default
         SE_PRIVILEGE_ENABLED);               // Enabled

    Privileges[5].Luid = SeAssignPrimaryTokenPrivilege;
    Privileges[5].Attributes = 0;    // disabled, not enabled by default

    Privileges[6].Luid = SeIncreaseQuotaPrivilege;
    Privileges[6].Attributes = 0;    // disabled, not enabled by default

    Privileges[7].Luid = SeIncreaseBasePriorityPrivilege;
    Privileges[7].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |    // Enabled by default
         SE_PRIVILEGE_ENABLED);               // Enabled

    Privileges[8].Luid = SeCreatePermanentPrivilege;
    Privileges[8].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |    // Enabled by default
         SE_PRIVILEGE_ENABLED);               // Enabled

    Privileges[9].Luid = SeDebugPrivilege;
    Privileges[9].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |   // Enabled by default
         SE_PRIVILEGE_ENABLED);               // Enabled

    Privileges[10].Luid = SeAuditPrivilege;
    Privileges[10].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |    // Enabled by default
         SE_PRIVILEGE_ENABLED);               // Enabled

    Privileges[11].Luid = SeSecurityPrivilege;
    Privileges[11].Attributes = 0;    // disabled, not enabled by default

    Privileges[12].Luid = SeSystemEnvironmentPrivilege;
    Privileges[12].Attributes = 0;    // disabled, not enabled by default

    Privileges[13].Luid = SeChangeNotifyPrivilege;
    Privileges[13].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |    // Enabled by default
         SE_PRIVILEGE_ENABLED);               // Enabled


    Privileges[14].Luid = SeBackupPrivilege;
    Privileges[14].Attributes = 0;    // disabled, not enabled by default

    Privileges[15].Luid = SeRestorePrivilege;
    Privileges[15].Attributes = 0;    // disabled, not enabled by default

    Privileges[16].Luid = SeShutdownPrivilege;
    Privileges[16].Attributes = 0;    // disabled, not enabled by default

    Privileges[17].Luid = SeLoadDriverPrivilege;
    Privileges[17].Attributes = 0;    // disabled, not enabled by default

    Privileges[18].Luid = SeProfileSingleProcessPrivilege;
    Privileges[18].Attributes =
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |    // Enabled by default
         SE_PRIVILEGE_ENABLED);               // Enabled

    Privileges[19].Luid = SeSystemtimePrivilege;
    Privileges[19].Attributes = 0;    // disabled, not enabled by default

    Privileges[20].Luid = SeUndockPrivilege ;
    Privileges[20].Attributes = 0 ;   // disabled, not enabled by default

    Privileges[21].Luid = SeManageVolumePrivilege ;
    Privileges[21].Attributes = 0 ;   // disabled, not enabled by default

    Privileges[22].Luid = SeImpersonatePrivilege ;
    Privileges[22].Attributes = 
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |
         SE_PRIVILEGE_ENABLED);

    Privileges[23].Luid = SeCreateGlobalPrivilege ;
    Privileges[23].Attributes = 
        (SE_PRIVILEGE_ENABLED_BY_DEFAULT |
         SE_PRIVILEGE_ENABLED );

    //BEFORE ADDING ANOTHER PRIVILEGE ^^ HERE ^^ CHECK THE ARRAY BOUND
    //ALSO INCREMENT THE PRIVILEGE COUNT IN THE SepCreateToken() call


    //
    // Establish the primary group and default owner
    //

    PrimaryGroup.PrimaryGroup = SeLocalSystemSid;  // Primary group
    Owner = SeAliasAdminsSid;                      // Default owner





    //
    // Set up an ACL to protect token as well ...
    // give system full reign of terror.  This includes user-mode components
    // running as part of the system.
    //

    Length = (ULONG)sizeof(ACL) +
             ((ULONG)sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG)) +
             SeLengthSid( SeLocalSystemSid ) ;

    TokenAcl = (PACL)ExAllocatePoolWithTag(PagedPool, Length, 'cAeS');

    if ( TokenAcl == NULL ) {

        return NULL ;
    }

    Status = RtlCreateAcl( TokenAcl, Length, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 TokenAcl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 SeLocalSystemSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    TokenSecurityDescriptor =
    (PSECURITY_DESCRIPTOR)ExAllocatePoolWithTag(
                              PagedPool,
                              sizeof(SECURITY_DESCRIPTOR),
                              'dSeS'
                              );

    if ( TokenSecurityDescriptor == NULL ) {

        ExFreePool( TokenAcl );

        return NULL ;
    }

    Status = RtlCreateSecurityDescriptor(
                 TokenSecurityDescriptor,
                 SECURITY_DESCRIPTOR_REVISION
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlSetDaclSecurityDescriptor (
                 TokenSecurityDescriptor,
                 TRUE,
                 TokenAcl,
                 FALSE
                 );
    ASSERT( NT_SUCCESS(Status) );


    Status = RtlSetOwnerSecurityDescriptor (
                 TokenSecurityDescriptor,
                 SeAliasAdminsSid,
                 FALSE // Owner defaulted
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlSetGroupSecurityDescriptor (
                 TokenSecurityDescriptor,
                 SeAliasAdminsSid,
                 FALSE // Group defaulted
                 );
    ASSERT( NT_SUCCESS(Status) );


    //
    // Create the system token
    //

    InitializeObjectAttributes(
        &TokenObjectAttributes,
        NULL,
        0,
        NULL,
        TokenSecurityDescriptor
        );



    ASSERT(SeSystemDefaultDacl != NULL);
    Status = SepCreateToken(
                 (PHANDLE)&Token,
                 KernelMode,
                 0,               // No handle created for system token
                 &TokenObjectAttributes,
                 TokenPrimary,
                 (SECURITY_IMPERSONATION_LEVEL)0,
                 (PLUID)&SeSystemAuthenticationId,
                 &NoExpiration,
                 &UserId,
                 3,                         // GroupCount
                 GroupIds,
                 GroupIdsLength,
                 24,                        // privileges
                 Privileges,
                 Owner,
                 PrimaryGroup.PrimaryGroup,
                 SeSystemDefaultDacl,
                 (PTOKEN_SOURCE)&SeSystemTokenSource,
                 TRUE,                        // System token
                 NULL,
                 NULL
                 );

     ASSERT(NT_SUCCESS(Status));

    //
    // We can free the old one now.
    //

    ExFreePool( TokenAcl );
    ExFreePool( TokenSecurityDescriptor );

    return  (PACCESS_TOKEN)Token;

}


PACCESS_TOKEN
SeMakeAnonymousLogonTokenNoEveryone (
    VOID
    )

/*++

Routine Description:

    This routine is provided for use by executive components
    DURING SYSTEM INITIALIZATION ONLY.  It creates a token for
    use by system components.

    A system token has the following characteristics:

         - It has ANONYMOUS_LOGON as its user ID

         - It has no privileges

         - It has protection that provides TOKEN_ALL_ACCESS to
           the WORLD ID.

         - It has a default ACL that grants GENERIC_ALL access
           to WORLD.


Parameters:

    None.

Return Value:

    Pointer to a system token.

--*/

{
    NTSTATUS Status;

    PVOID Token;

    SID_AND_ATTRIBUTES UserId;
    TOKEN_PRIMARY_GROUP PrimaryGroup;
    PACL TokenAcl;
    PSID Owner;
    ULONG Length;
    OBJECT_ATTRIBUTES TokenObjectAttributes;
    PSECURITY_DESCRIPTOR TokenSecurityDescriptor;

    TIME_FIELDS TimeFields;
    LARGE_INTEGER NoExpiration;

    PAGED_CODE();

    //
    // Set up expiration times
    //

    TimeFields.Year = 3000;
    TimeFields.Month = 1;
    TimeFields.Day = 1;
    TimeFields.Hour = 1;
    TimeFields.Minute = 1;
    TimeFields.Second = 1;
    TimeFields.Milliseconds = 1;
    TimeFields.Weekday = 1;

    RtlTimeFieldsToTime( &TimeFields, &NoExpiration );

    //
    // Set up the user ID
    //

    UserId.Sid = SeAnonymousLogonSid;
    UserId.Attributes = 0;

    //
    // Establish the primary group and default owner
    //

    PrimaryGroup.PrimaryGroup = SeAnonymousLogonSid;  // Primary group

    //
    // Set up an ACL to protect token as well ...
    // Let everyone read/write.  However, the token is dup'ed before we given
    // anyone a handle to it.
    //

    Length = (ULONG)sizeof(ACL) +
             (ULONG)sizeof(ACCESS_ALLOWED_ACE) +
             SeLengthSid( SeWorldSid ) +
             (ULONG)sizeof(ACCESS_ALLOWED_ACE) +
             SeLengthSid( SeAnonymousLogonSid );
    ASSERT( Length < 200 );

    TokenAcl = (PACL)ExAllocatePoolWithTag(PagedPool, 200, 'cAeS');

    if ( !TokenAcl ) {

        return NULL ;
    }

    Status = RtlCreateAcl( TokenAcl, Length, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 TokenAcl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 SeWorldSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 TokenAcl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 SeAnonymousLogonSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    TokenSecurityDescriptor =
    (PSECURITY_DESCRIPTOR)ExAllocatePoolWithTag(
                              PagedPool,
                              SECURITY_DESCRIPTOR_MIN_LENGTH,
                              'dSeS'
                              );

    if ( !TokenSecurityDescriptor ) {

        ExFreePool( TokenAcl );

        return NULL ;
    }

    Status = RtlCreateSecurityDescriptor(
                 TokenSecurityDescriptor,
                 SECURITY_DESCRIPTOR_REVISION
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlSetDaclSecurityDescriptor (
                 TokenSecurityDescriptor,
                 TRUE,
                 TokenAcl,
                 FALSE
                 );
    ASSERT( NT_SUCCESS(Status) );


    Status = RtlSetOwnerSecurityDescriptor (
                 TokenSecurityDescriptor,
                 SeWorldSid,
                 FALSE // Owner defaulted
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlSetGroupSecurityDescriptor (
                 TokenSecurityDescriptor,
                 SeWorldSid,
                 FALSE // Group defaulted
                 );
    ASSERT( NT_SUCCESS(Status) );

    //
    // Create the system token
    //

    InitializeObjectAttributes(
        &TokenObjectAttributes,
        NULL,
        0,
        NULL,
        TokenSecurityDescriptor
        );

    Status = SepCreateToken(
                 (PHANDLE)&Token,
                 KernelMode,
                 0,               // No handle created for system token
                 &TokenObjectAttributes,
                 TokenPrimary,
                 (SECURITY_IMPERSONATION_LEVEL)0,
                 (PLUID)&SeAnonymousAuthenticationId,
                 &NoExpiration,
                 &UserId,
                 0,                         // GroupCount
                 NULL,                      // Group IDs
                 0,                         // Group byte count
                 0,                         // no privileges
                 NULL,                      // no Privileges,
                 NULL,
                 PrimaryGroup.PrimaryGroup,
                 TokenAcl,
                 (PTOKEN_SOURCE)&SeSystemTokenSource,
                 TRUE,                        // System token
                 NULL,
                 NULL
                 );

     ASSERT(NT_SUCCESS(Status));

    //
    // We can free the old one now.
    //

    ExFreePool( TokenAcl );
    ExFreePool( TokenSecurityDescriptor );

    return  (PACCESS_TOKEN)Token;

}


PACCESS_TOKEN
SeMakeAnonymousLogonToken (
    VOID
    )

/*++

Routine Description:

    This routine is provided for use by executive components
    DURING SYSTEM INITIALIZATION ONLY.  It creates a token for
    use by system components.

    A system token has the following characteristics:

         - It has ANONYMOUS_LOGON as its user ID

         - It has the following groups with the corresponding
           attributes:


               WORLD             EnabledByDefault |
                                 Enabled          |
                                 Mandatory

         - It has WORLD as its primary group.

         - It has no privileges

         - It has protection that provides TOKEN_ALL_ACCESS to
           the WORLD ID.

         - It has a default ACL that grants GENERIC_ALL access
           to WORLD.


Parameters:

    None.

Return Value:

    Pointer to a system token.

--*/

{
    NTSTATUS Status;

    PVOID Token;

    SID_AND_ATTRIBUTES UserId;
    PSID_AND_ATTRIBUTES GroupIds;
    TOKEN_PRIMARY_GROUP PrimaryGroup;
    ULONG GroupIdsLength;
    PACL TokenAcl;
    PSID Owner;
    ULONG NormalGroupAttributes;
    ULONG Length;
    OBJECT_ATTRIBUTES TokenObjectAttributes;
    PSECURITY_DESCRIPTOR TokenSecurityDescriptor;

    ULONG_PTR GroupIdsBuffer[128 * sizeof(ULONG) / sizeof(ULONG_PTR)];

    TIME_FIELDS TimeFields;
    LARGE_INTEGER NoExpiration;

    PAGED_CODE();

    //
    // Set up expiration times
    //

    TimeFields.Year = 3000;
    TimeFields.Month = 1;
    TimeFields.Day = 1;
    TimeFields.Hour = 1;
    TimeFields.Minute = 1;
    TimeFields.Second = 1;
    TimeFields.Milliseconds = 1;
    TimeFields.Weekday = 1;

    RtlTimeFieldsToTime( &TimeFields, &NoExpiration );

    GroupIds = (PSID_AND_ATTRIBUTES)GroupIdsBuffer;

    //
    // Set up the attributes to be assigned to groups
    //

    NormalGroupAttributes =    (SE_GROUP_MANDATORY          |
                                SE_GROUP_ENABLED_BY_DEFAULT |
                                SE_GROUP_ENABLED
                                );

    //
    // Set up the user ID
    //

    UserId.Sid = SeAnonymousLogonSid;
    UserId.Attributes = 0;

    //
    // Set up the groups
    //

    GroupIds->Sid  = SeWorldSid;
    GroupIds->Attributes = NormalGroupAttributes;


    GroupIdsLength = (ULONG)LongAlignSize(SeLengthSid(GroupIds->Sid)) +
                                                    sizeof(SID_AND_ATTRIBUTES);

    ASSERT( GroupIdsLength <= 128 * sizeof(ULONG) );

    //
    // Establish the primary group and default owner
    //

    PrimaryGroup.PrimaryGroup = SeAnonymousLogonSid;  // Primary group

    //
    // Set up an ACL to protect token as well ...
    // give system full reign of terror.  This includes user-mode components
    // running as part of the system.
    // Let everyone read/write.  However, the token is dup'ed before we given
    // anyone a handle to it.
    //

    Length = (ULONG)sizeof(ACL) +
             (ULONG)sizeof(ACCESS_ALLOWED_ACE) +
             SeLengthSid( SeWorldSid ) +
             (ULONG)sizeof(ACCESS_ALLOWED_ACE) +
             SeLengthSid( SeAnonymousLogonSid );
    ASSERT( Length < 200 );

    TokenAcl = (PACL)ExAllocatePoolWithTag(PagedPool, 200, 'cAeS');

    if ( !TokenAcl ) {

        return NULL ;
    }

    Status = RtlCreateAcl( TokenAcl, Length, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 TokenAcl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 SeWorldSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAccessAllowedAce (
                 TokenAcl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 SeAnonymousLogonSid
                 );
    ASSERT( NT_SUCCESS(Status) );

    TokenSecurityDescriptor =
    (PSECURITY_DESCRIPTOR)ExAllocatePoolWithTag(
                              PagedPool,
                              SECURITY_DESCRIPTOR_MIN_LENGTH,
                              'dSeS'
                              );

    if ( !TokenSecurityDescriptor ) {

        ExFreePool( TokenAcl );

        return NULL ;
    }


    Status = RtlCreateSecurityDescriptor(
                 TokenSecurityDescriptor,
                 SECURITY_DESCRIPTOR_REVISION
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlSetDaclSecurityDescriptor (
                 TokenSecurityDescriptor,
                 TRUE,
                 TokenAcl,
                 FALSE
                 );
    ASSERT( NT_SUCCESS(Status) );


    Status = RtlSetOwnerSecurityDescriptor (
                 TokenSecurityDescriptor,
                 SeWorldSid,
                 FALSE // Owner defaulted
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlSetGroupSecurityDescriptor (
                 TokenSecurityDescriptor,
                 SeWorldSid,
                 FALSE // Group defaulted
                 );
    ASSERT( NT_SUCCESS(Status) );


    //
    // Create the system token
    //

    InitializeObjectAttributes(
        &TokenObjectAttributes,
        NULL,
        0,
        NULL,
        TokenSecurityDescriptor
        );



    Status = SepCreateToken(
                 (PHANDLE)&Token,
                 KernelMode,
                 0,               // No handle created for system token
                 &TokenObjectAttributes,
                 TokenPrimary,
                 (SECURITY_IMPERSONATION_LEVEL)0,
                 (PLUID)&SeAnonymousAuthenticationId,
                 &NoExpiration,
                 &UserId,
                 1,                         // GroupCount
                 GroupIds,
                 GroupIdsLength,
                 0,                         // no privileges
                 NULL,                      // no Privileges,
                 0,                         // no privileges
                 PrimaryGroup.PrimaryGroup,
                 TokenAcl,
                 (PTOKEN_SOURCE)&SeSystemTokenSource,
                 TRUE,                        // System token
                 NULL,
                 NULL
                 );

     ASSERT(NT_SUCCESS(Status));

    //
    // We can free the old one now.
    //

    ExFreePool( TokenAcl );
    ExFreePool( TokenSecurityDescriptor );

    return  (PACCESS_TOKEN)Token;

}


NTSTATUS
SeSubProcessToken (
    __in PACCESS_TOKEN ParentToken,
    __deref_out PACCESS_TOKEN *ChildToken,
    __in BOOLEAN MarkAsActive,
    __in ULONG SessionId
    )

/*++

Routine Description:

    This routine makes a token for a sub-process that is a duplicate
    of the parent process's token.



Parameters:

    ParentToken - Pointer to the parent token

    ChildToken - Receives a pointer to the child process's token.

    MarkAsActive - Mark the token as active

    SessionId - Create the token with this session ID

Return Value:

    STATUS_SUCCESS - Indicates the sub-process's token has been created
        successfully.

    Other status values may be returned from memory allocation or object
    creation services used and typically indicate insufficient resources
    or quota on the requestor's part.



--*/

{
    PTOKEN NewToken;
    OBJECT_ATTRIBUTES PrimaryTokenAttributes;

    NTSTATUS Status;
    NTSTATUS IgnoreStatus;

    PAGED_CODE();

    InitializeObjectAttributes(
        &PrimaryTokenAttributes,
        NULL,
        0,
        NULL,
        NULL
        );

    Status = SepDuplicateToken(
                ParentToken,                         // ExistingToken
                &PrimaryTokenAttributes,             // ObjectAttributes
                FALSE,                               // EffectiveOnly
                TokenPrimary,                        // TokenType
                (SECURITY_IMPERSONATION_LEVEL)0,     // ImpersonationLevel
                KernelMode,                          // RequestorMode
                &NewToken                            // NewToken
                );

    if (NT_SUCCESS(Status)) {

        NewToken->SessionId = SessionId;

        //
        // Insert the new token object, up its ref count but don't create a handle.
        //

        Status = ObInsertObject(
                     NewToken,
                     NULL,
                     0,
                     0,
                     NULL,
                     NULL);

        if (NT_SUCCESS(Status)) {

            NewToken->TokenInUse = MarkAsActive;
            *ChildToken = NewToken;

        } else {

            //
            // ObInsertObject dereferences the passed object if it
            // fails, so we don't have to do any cleanup on NewToken
            // here.
            //
        }
    }

    return Status;
}


BOOLEAN
SepTokenInitialization ( VOID )

/*++

Routine Description:

    This function creates the token object type descriptor at system
    initialization and stores the address of the object type descriptor
    in global storage.  It also created token related global variables.

    Furthermore, some number of pseudo tokens are created during system
    initialization.  These tokens are tracked down and replaced with
    real tokens.

Arguments:

    None.

Return Value:

    A value of TRUE is returned if the object type descriptor is
    successfully initialized. Otherwise a value of FALSE is returned.

--*/

{

    OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
    NTSTATUS Status;
    UNICODE_STRING TypeName;

    PAGED_CODE();

    //
    // Initialize string descriptor.
    //

    RtlInitUnicodeString(&TypeName, L"Token");


    //
    // Create object type descriptor.
    //

    RtlZeroMemory(&ObjectTypeInitializer,sizeof(ObjectTypeInitializer));
    ObjectTypeInitializer.Length = sizeof(ObjectTypeInitializer);
    ObjectTypeInitializer.InvalidAttributes = OBJ_OPENLINK;
    ObjectTypeInitializer.GenericMapping = SepTokenMapping;
    ObjectTypeInitializer.SecurityRequired = TRUE;
    ObjectTypeInitializer.UseDefaultObject = TRUE;
    ObjectTypeInitializer.PoolType = PagedPool;
    ObjectTypeInitializer.ValidAccessMask = TOKEN_ALL_ACCESS;
    ObjectTypeInitializer.DeleteProcedure = SepTokenDeleteMethod;

    Status = ObCreateObjectType(&TypeName,
                                &ObjectTypeInitializer,
                                (PSECURITY_DESCRIPTOR)NULL,
                                &SeTokenObjectType
                                );

    //
    // If the object type descriptor was successfully created, then
    // return a value of TRUE. Otherwise return a value of FALSE.
    //

    return (BOOLEAN)NT_SUCCESS(Status);
}


NTSTATUS
NtCreateToken(
    __out PHANDLE TokenHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in TOKEN_TYPE TokenType,
    __in PLUID AuthenticationId,
    __in PLARGE_INTEGER ExpirationTime,
    __in PTOKEN_USER User,
    __in PTOKEN_GROUPS Groups,
    __in PTOKEN_PRIVILEGES Privileges,
    __in_opt PTOKEN_OWNER Owner,
    __in PTOKEN_PRIMARY_GROUP PrimaryGroup,
    __in_opt PTOKEN_DEFAULT_DACL DefaultDacl,
    __in PTOKEN_SOURCE TokenSource
    )

/*++

Routine Description:

    Create a token object and return a handle opened for access to
    that token.  This API requires SeCreateTokenPrivilege privilege.

Arguments:

    TokenHandle - Receives the handle of the newly created token.

    DesiredAccess - Is an access mask indicating which access types
        the handle is to provide to the new object.

    ObjectAttributes - Points to the standard object attributes data
        structure.  Refer to the NT Object Management
        Specification for a description of this data structure.

        If the token type is TokenImpersonation, then this parameter
        must specify the impersonation level of the token.

    TokenType - Type of token to be created.  Privilege is required
        to create any type of token.

    AuthenticationId - Points to a LUID (or LUID) providing a unique
        identifier associated with the authentication.  This is used
        within security only, for audit purposes.

    ExpirationTime - Time at which the token becomes invalid.  If this
        value is specified as zero, then the token has no expiration
        time.

    User - Is the user SID to place in the token.

    Groups - Are the group SIDs to place in the token. The API assumes that 
        the caller has not supplied duplicate group sids.

    Privileges - Are the privileges to place in the token. The API assumes that
        the caller has not supplied duplicate privileges.

    Owner - (Optionally) identifies an identifier that is to be used
        as the default owner for the token.  If not provided, the
        user ID is made the default owner.

    PrimaryGroup - Identifies which of the group IDs is to be the
        primary group of the token.

    DefaultDacl - (optionally) establishes an ACL to be used as the
        default discretionary access protection for the token.

    TokenSource - Identifies the token source name string and
        identifier to be assigned to the token.

Return Value:

    STATUS_SUCCESS - Indicates the operation was successful.

    STATUS_INVALID_OWNER - Indicates the ID provided to be assigned
        as the default owner of the token does not have an attribute
        indicating it may be assigned as an owner.

    STATUS_INVALID_PRIMARY_GROUP - Indicates the group ID provided
        via the PrimaryGroup parameter was not among those assigned
        to the token in the Groups parameter.

    STATUS_BAD_IMPERSONATION_LEVEL - Indicates no impersonation level
        was provided when attempting to create a token of type
        TokenImpersonation.

--*/

{

    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    ULONG Ignore;


    HANDLE LocalHandle = NULL;

    BOOLEAN SecurityQosPresent = FALSE;
    SECURITY_ADVANCED_QUALITY_OF_SERVICE CapturedSecurityQos;

    LUID CapturedAuthenticationId;
    LARGE_INTEGER CapturedExpirationTime;

    PSID_AND_ATTRIBUTES CapturedUser = NULL;
    ULONG CapturedUserLength = 0;

    ULONG CapturedGroupCount = 0;
    PSID_AND_ATTRIBUTES CapturedGroups = NULL;
    ULONG CapturedGroupsLength = 0;

    ULONG CapturedPrivilegeCount = 0;
    PLUID_AND_ATTRIBUTES CapturedPrivileges = NULL;
    ULONG CapturedPrivilegesLength = 0;

    PSID CapturedOwner = NULL;

    PSID CapturedPrimaryGroup = NULL;

    PACL CapturedDefaultDacl = NULL;

    TOKEN_SOURCE CapturedTokenSource;

    PVOID CapturedAddress;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        //
        // Probe everything necessary for input to the capture subroutines.
        //

        try {

            ProbeForWriteHandle(TokenHandle);


            ProbeForReadSmallStructure( ExpirationTime, sizeof(LARGE_INTEGER),    sizeof(ULONG) );
            ProbeForReadSmallStructure( Groups,         sizeof(TOKEN_GROUPS),     sizeof(ULONG) );
            ProbeForReadSmallStructure( Privileges,     sizeof(TOKEN_PRIVILEGES), sizeof(ULONG) );
            ProbeForReadSmallStructure( TokenSource,    sizeof(TOKEN_SOURCE),     sizeof(ULONG) );


            if ( ARGUMENT_PRESENT(Owner) ) {
                ProbeForReadSmallStructure( Owner, sizeof(TOKEN_OWNER), sizeof(ULONG) );
            }


            ProbeForReadSmallStructure(
                PrimaryGroup,
                sizeof(TOKEN_PRIMARY_GROUP),
                sizeof(ULONG)
                );


            if ( ARGUMENT_PRESENT(DefaultDacl) ) {
                ProbeForReadSmallStructure(
                    DefaultDacl,
                    sizeof(TOKEN_DEFAULT_DACL),
                    sizeof(ULONG)
                    );
             }

            ProbeForReadSmallStructure(
                AuthenticationId,
                sizeof(LUID),
                sizeof(ULONG)
                );

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }  // end_try

    } //end_if

    //
    // Make sure the TokenType is valid
    //

    if ( (TokenType < TokenPrimary) || (TokenType > TokenImpersonation) ) {
        return(STATUS_BAD_TOKEN_TYPE);
    }


    //
    //  Capture the security quality of service.
    //  This capture routine necessarily does some probing of its own.
    //

    Status = SeCaptureSecurityQos(
                 ObjectAttributes,
                 PreviousMode,
                 &SecurityQosPresent,
                 &CapturedSecurityQos
                 );

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (TokenType == TokenImpersonation) {

        if (!SecurityQosPresent) {
            return STATUS_BAD_IMPERSONATION_LEVEL;
        } // endif

        //
        // Allow only valid impersonation levels.
        //

        switch (CapturedSecurityQos.ImpersonationLevel) {
        case SecurityAnonymous:
        case SecurityIdentification:
        case SecurityImpersonation:
        case SecurityDelegation:
            break;
        default:
            SeFreeCapturedSecurityQos( &CapturedSecurityQos );
            return STATUS_BAD_IMPERSONATION_LEVEL;
        }
    }

    //
    //  Capture the rest of the arguments.
    //  These arguments have already been probed.
    //

    try {

        Status = STATUS_SUCCESS;

        //
        //  Capture and validate AuthenticationID
        //

        RtlCopyLuid( &CapturedAuthenticationId, AuthenticationId );

        //
        //  Capture ExpirationTime
        //

        CapturedExpirationTime = (*ExpirationTime);

        //
        //  Capture User
        //

        if (NT_SUCCESS(Status)) {
            Status = SeCaptureSidAndAttributesArray(
                         &(User->User),
                         1,
                         PreviousMode,
                         NULL, 0,
                         PagedPool,
                         TRUE,
                         &CapturedUser,
                         &CapturedUserLength
                         );
        }


        //
        //  Capture Groups
        //

        if (NT_SUCCESS(Status)) {
            CapturedGroupCount = Groups->GroupCount;
            Status = SeCaptureSidAndAttributesArray(
                         (Groups->Groups),
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
        //  Capture Privileges
        //

        if (NT_SUCCESS(Status)) {
            CapturedPrivilegeCount = Privileges->PrivilegeCount;
            Status = SeCaptureLuidAndAttributesArray(
                         (Privileges->Privileges),
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
        //  Capture Owner
        //

        if ( ARGUMENT_PRESENT(Owner) && NT_SUCCESS(Status)) {
            CapturedAddress = Owner->Owner;
            Status = SeCaptureSid(
                         (PSID)CapturedAddress,
                         PreviousMode,
                         NULL, 0,
                         PagedPool,
                         TRUE,
                         &CapturedOwner
                         );
        }


        //
        //  Capture PrimaryGroup
        //
        if (NT_SUCCESS(Status)) {
            CapturedAddress = PrimaryGroup->PrimaryGroup;
            Status = SeCaptureSid(
                         (PSID)CapturedAddress,
                         PreviousMode,
                         NULL, 0,
                         PagedPool,
                         TRUE,
                         &CapturedPrimaryGroup
                         );
        }


        //
        //  Capture DefaultDacl
        //

        if ( ARGUMENT_PRESENT(DefaultDacl) && NT_SUCCESS(Status) ) {
            CapturedAddress = DefaultDacl->DefaultDacl;
            if (CapturedAddress != NULL) {
                Status = SeCaptureAcl(
                             (PACL)CapturedAddress,
                             PreviousMode,
                             NULL, 0,
                             NonPagedPool,
                             TRUE,
                             &CapturedDefaultDacl,
                             &Ignore
                             );
            }
        }

        //
        //  Capture TokenSource
        //

        CapturedTokenSource = (*TokenSource);


    } except(EXCEPTION_EXECUTE_HANDLER) {

        if (CapturedUser != NULL) {
            SeReleaseSidAndAttributesArray(
                CapturedUser,
                PreviousMode,
                TRUE
                );
        }

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

        if (CapturedOwner != NULL) {
            SeReleaseSid( CapturedOwner, PreviousMode, TRUE);
        }

        if (CapturedPrimaryGroup != NULL) {
            SeReleaseSid( CapturedPrimaryGroup, PreviousMode, TRUE);
        }

        if (CapturedDefaultDacl != NULL) {
            SeReleaseAcl( CapturedDefaultDacl, PreviousMode, TRUE);
        }

        if (SecurityQosPresent == TRUE) {
            SeFreeCapturedSecurityQos( &CapturedSecurityQos );
        }

        return GetExceptionCode();

    }  // end_try{}

    //
    //  Create the token
    //

    if (NT_SUCCESS(Status)) {
        Status = SepCreateToken(
                                &LocalHandle,
                                PreviousMode,
                                DesiredAccess,
                                ObjectAttributes,
                                TokenType,
                                CapturedSecurityQos.ImpersonationLevel,
                                &CapturedAuthenticationId,
                                &CapturedExpirationTime,
                                CapturedUser,
                                CapturedGroupCount,
                                CapturedGroups,
                                CapturedGroupsLength,
                                CapturedPrivilegeCount,
                                CapturedPrivileges,
                                CapturedOwner,
                                CapturedPrimaryGroup,
                                CapturedDefaultDacl,
                                &CapturedTokenSource,
                                FALSE,                       // Not a system token
                                SecurityQosPresent ? CapturedSecurityQos.ProxyData : NULL,
                                SecurityQosPresent ? CapturedSecurityQos.AuditData : NULL
                                );
    }

    //
    //  Clean up the temporary capture buffers
    //

    if (CapturedUser != NULL) {
        SeReleaseSidAndAttributesArray( CapturedUser, PreviousMode, TRUE);
    }
    if (CapturedGroups != NULL) {
        SeReleaseSidAndAttributesArray( CapturedGroups, PreviousMode, TRUE);
    }

    if (CapturedPrivileges != NULL) {
        SeReleaseLuidAndAttributesArray( CapturedPrivileges, PreviousMode, TRUE);
    }

    if (CapturedOwner != NULL) {
        SeReleaseSid( CapturedOwner, PreviousMode, TRUE);
    }

    if (CapturedPrimaryGroup != NULL) {
        SeReleaseSid( CapturedPrimaryGroup, PreviousMode, TRUE);
    }

    if (CapturedDefaultDacl != NULL) {
        SeReleaseAcl( CapturedDefaultDacl, PreviousMode, TRUE);
    }

    if (SecurityQosPresent == TRUE) {
        SeFreeCapturedSecurityQos( &CapturedSecurityQos );
    }

    //
    //  Return the handle to this new token
    //

    if (NT_SUCCESS(Status)) {
        try { *TokenHandle = LocalHandle; }
            except(EXCEPTION_EXECUTE_HANDLER) { return GetExceptionCode(); }
    }

    return Status;

}



////////////////////////////////////////////////////////////////////////
//                                                                    //
//           Token Private Routines                                   //
//                                                                    //
////////////////////////////////////////////////////////////////////////


VOID
SepTokenDeleteMethod (
    IN  PVOID   Token
    )

/*++

Routine Description:

    This function is the token object type-specific delete method.
    It is needed to ensure that all memory allocated for the token
    gets deallocated.

Arguments:

    Token - Points to the token object being deleted.

Return Value:

    None.

--*/

{
    PAGED_CODE();

#if DBG || TOKEN_LEAK_MONITOR
    SepRemoveTokenLogonSession( Token );
#endif

    //
    // De-reference the logon session referenced by this token object
    //

    if ((((TOKEN *)Token)->TokenFlags & TOKEN_SESSION_NOT_REFERENCED ) == 0 ) {
        SepDeReferenceLogonSessionDirect( (((TOKEN *)Token)->LogonSession) );
    } 

    //
    // If this token had an active SEP_AUDIT_POLICY then decrement the Token audit counter
    // because this token is going byebye.
    //

    if ( ((PTOKEN)Token)->AuditPolicy.Overlay ) {
        
        SepModifyTokenPolicyCounter(&((PTOKEN)Token)->AuditPolicy, FALSE);
        ASSERT(SepTokenPolicyCounter >= 0);
    }

    //
    // If the token has an associated Dynamic part, deallocate it.
    //

    if (ARGUMENT_PRESENT( ((TOKEN *)Token)->DynamicPart)) {
        ExFreePool( ((TOKEN *)Token)->DynamicPart );
    }

    //
    // Free the Proxy and Global audit structures if present.
    //

    if (ARGUMENT_PRESENT(((TOKEN *) Token)->ProxyData)) {
        SepFreeProxyData( ((TOKEN *)Token)->ProxyData );
    }

    if (ARGUMENT_PRESENT(((TOKEN *)Token)->AuditData )) {
        ExFreePool( (((TOKEN *)Token)->AuditData) );
    }

    if (ARGUMENT_PRESENT(((TOKEN *)Token)->TokenLock )) {
        ExDeleteResourceLite(((TOKEN *)Token)->TokenLock );
        ExFreePool(((TOKEN *)Token)->TokenLock );
    }

    return;
}

NTSTATUS
SepCreateToken(
    OUT PHANDLE TokenHandle,
    IN KPROCESSOR_MODE RequestorMode,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN TOKEN_TYPE TokenType,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel OPTIONAL,
    IN PLUID AuthenticationId,
    IN PLARGE_INTEGER ExpirationTime,
    IN PSID_AND_ATTRIBUTES User,
    IN ULONG GroupCount,
    IN PSID_AND_ATTRIBUTES Groups,
    IN ULONG GroupsLength,
    IN ULONG PrivilegeCount,
    IN PLUID_AND_ATTRIBUTES Privileges,
    IN PSID Owner OPTIONAL,
    IN PSID PrimaryGroup,
    IN PACL DefaultDacl OPTIONAL,
    IN PTOKEN_SOURCE TokenSource,
    IN BOOLEAN SystemToken,
    IN PSECURITY_TOKEN_PROXY_DATA ProxyData OPTIONAL,
    IN PSECURITY_TOKEN_AUDIT_DATA AuditData OPTIONAL
    )

/*++

Routine Description:

    Create a token object and return a handle opened for access to
    that token.  This API implements the bulk of the work needed
    for NtCreateToken.

    All parameters except DesiredAccess and ObjectAttributes are assumed
    to have been probed and captured.

    The output parameter (TokenHandle) is expected to be returned to a
    safe address, rather than to a user mode address that may cause an
    exception.

    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    NOTE: This routine is also used to create the initial system token.
          In that case, the SystemToken parameter is TRUE and no handle
          is established to the token.  Instead, a pointer to the token
          is returned via the TokenHandle parameter.
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


Arguments:

    TokenHandle - Receives the handle of the newly created token.  If the
        SystemToken parameter is specified is true, then this parameter
        receives a pointer to the token instead of a handle to the token.

    RequestorMode - The mode of the caller on whose behalf the token
        is being created.

    DesiredAccess - Is an access mask indicating which access types
        the handle is to provide to the new object.

    ObjectAttributes - Points to the standard object attributes data
        structure.  Refer to the NT Object Management
        Specification for a description of this data structure.

    TokenType - Type of token to be created.  Privilege is required
        to create any type of token.

    ImpersonationLevel - If the token type is TokenImpersonation, then
        this parameter is used to specify the impersonation level of
        the token.

    AuthenticationId - Points to a LUID (or LUID) providing a unique
        identifier associated with the authentication.  This is used
        within security only, for audit purposes.

    ExpirationTime - Time at which the token becomes invalid.  If this
        value is specified as zero, then the token has no expiration
        time.

    User - Is the user SID to place in the token.

    GroupCount - Indicates the number of groups in the 'Groups' parameter.
        This value may be zero, in which case the 'Groups' parameter is
        ignored.

    Groups - Are the group SIDs, and their corresponding attributes,
        to place in the token.

    GroupsLength - Indicates the length, in bytes, of the array of groups
        to place in the token.

    PrivilegeCount - Indicates the number of privileges in the 'Privileges'
        parameter.  This value may be zero, in which case the 'Privileges'
        parameter is ignored.

    Privileges - Are the privilege LUIDs, and their corresponding attributes,
        to place in the token.

    PrivilegesLength - Indicates the length, in bytes, of the array of
        privileges to place in the token.

    Owner - (Optionally) identifies an identifier that is to be used
        as the default owner for the token.  If not provided, the
        user ID is made the default owner.

    PrimaryGroup - Identifies which of the group IDs is to be the
        primary group of the token.

    DefaultDacl - (optionally) establishes an ACL to be used as the
        default discretionary access protection for the token.

    TokenSource - Identifies the token source name string and
        identifier to be assigned to the token.

Return Value:

    STATUS_SUCCESS - Indicates the operation was successful.

    STATUS_INVALID_OWNER - Indicates the ID provided to be assigned
        as the default owner of the token does not have an attribute
        indicating it may be assigned as an owner.

    STATUS_INVALID_PRIMARY_GROUP - Indicates the group ID provided
        via the PrimaryGroup parameter was not among those assigned
        to the token in the Groups parameter.

    STATUS_INVALID_PARAMETER - Indicates that a required parameter,
        such as User or PrimaryGroup, was not provided with a legitimate
        value.

--*/

{

    PTOKEN Token;
    NTSTATUS Status;

    ULONG PagedPoolSize;

    ULONG PrimaryGroupLength;

    ULONG TokenBodyLength;
    ULONG VariableLength;

    ULONG DefaultOwnerIndex = 0;
    PUCHAR Where;
    ULONG ComputedPrivLength;

    PSID NextSidFree;

    ULONG DynamicLength;
    ULONG DynamicLengthUsed;

    ULONG SubAuthorityCount;
    ULONG GroupIndex;
    ULONG PrivilegeIndex;
    BOOLEAN OwnerFound;

    UCHAR TokenFlags = 0;

    ACCESS_STATE AccessState;
    AUX_ACCESS_DATA AuxData;
    LUID NewModifiedId;

    PERESOURCE TokenLock;

#if DBG || TOKEN_LEAK_MONITOR
    ULONG Frames;
#endif
    
    PAGED_CODE();

    ASSERT( sizeof(SECURITY_IMPERSONATION_LEVEL) <= sizeof(ULONG) );

    //
    // Make sure the Enabled and Enabled-by-default bits are set on every
    // mandatory group.
    //
    // Also, check to see if the local administrators alias is present.
    // if so, turn on the flag so that we can do restrictions later
    //

    for (GroupIndex=0; GroupIndex < GroupCount; GroupIndex++) {
        if (Groups[GroupIndex].Attributes & SE_GROUP_MANDATORY) {
            Groups[GroupIndex].Attributes |= (SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT);
        }
    }
    
    for (GroupIndex=0; GroupIndex < GroupCount; GroupIndex++) {
        if (RtlEqualSid( SeAliasAdminsSid, Groups[GroupIndex].Sid )) {
            TokenFlags |= TOKEN_HAS_ADMIN_GROUP;
            break;
        }
    }

    //
    // Check to see if the token being created is going to be granted
    // SeChangeNotifyPrivilege.  If so, set a flag in the TokenFlags field
    // so we can find this out quickly.
    //

    for (PrivilegeIndex = 0; PrivilegeIndex < PrivilegeCount; PrivilegeIndex++) {

        if (((RtlEqualLuid(&Privileges[PrivilegeIndex].Luid,&SeChangeNotifyPrivilege))
                &&
            (Privileges[PrivilegeIndex].Attributes & SE_PRIVILEGE_ENABLED))) {

            TokenFlags |= TOKEN_HAS_TRAVERSE_PRIVILEGE;
        }
        if (((RtlEqualLuid(&Privileges[PrivilegeIndex].Luid, &SeImpersonatePrivilege)) &&
                ( Privileges[PrivilegeIndex].Attributes & SE_PRIVILEGE_ENABLED))) {

            TokenFlags |= TOKEN_HAS_IMPERSONATE_PRIVILEGE ;
            
        }
    }


    //
    // Get a ModifiedId to use
    //

    ExAllocateLocallyUniqueId( &NewModifiedId );

    //
    // Validate the owner ID, if provided and establish the default
    // owner index.
    //

    if (!ARGUMENT_PRESENT(Owner)) {

        DefaultOwnerIndex = 0;

    } else {


        if ( RtlEqualSid( Owner, User->Sid )  ) {

            DefaultOwnerIndex = 0;

        } else {

            GroupIndex = 0;
            OwnerFound = FALSE;

            while ((GroupIndex < GroupCount) && (!OwnerFound)) {

                if ( RtlEqualSid( Owner, (Groups[GroupIndex].Sid) )  ) {

                    //
                    // Found a match - make sure it is assignable as owner.
                    //

                    if ( SepArrayGroupAttributes( Groups, GroupIndex ) &
                         SE_GROUP_OWNER ) {

                        DefaultOwnerIndex = GroupIndex + 1;
                        OwnerFound = TRUE;

                    } else {

                        return STATUS_INVALID_OWNER;

                    } // endif Owner attribute set

                } // endif owner = group

                GroupIndex += 1;

            }  // endwhile

            if (!OwnerFound) {

                return STATUS_INVALID_OWNER;

            } // endif !OwnerFound
        } // endif owner = user
    } // endif owner specified




    TokenLock = (PERESOURCE)ExAllocatePoolWithTag( NonPagedPool, sizeof( ERESOURCE ), 'dTeS' );

    if (TokenLock == NULL) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    //
    //  Calculate the length needed for the variable portion of the token
    //  This includes the User ID, Group IDs, and Privileges
    //
    //
    // Align the privilege chunk by pointer alignment so that the SIDs will
    // be correctly aligned.  Align the Groups Length so that the SID_AND_ATTR
    // array (which is
    //

    ComputedPrivLength = PrivilegeCount * sizeof( LUID_AND_ATTRIBUTES ) ;

    ComputedPrivLength = ALIGN_UP( ComputedPrivLength, PVOID );

    GroupsLength = ALIGN_UP( GroupsLength, PVOID );


    VariableLength  = GroupsLength + ComputedPrivLength +
                       ALIGN_UP( (GroupCount * sizeof( SID_AND_ATTRIBUTES )), PVOID ) ;

    SubAuthorityCount = ((SID *)(User->Sid))->SubAuthorityCount;
    VariableLength += sizeof(SID_AND_ATTRIBUTES) +
                      (ULONG)LongAlignSize(RtlLengthRequiredSid( SubAuthorityCount ));



    //
    //  Calculate the length needed for the dynamic portion of the token
    //  This includes the default Dacl and the primary group.
    //

    SubAuthorityCount = ((SID *)PrimaryGroup)->SubAuthorityCount;
    DynamicLengthUsed = (ULONG)LongAlignSize(RtlLengthRequiredSid( SubAuthorityCount ));

    if (ARGUMENT_PRESENT(DefaultDacl)) {
        DynamicLengthUsed += (ULONG)LongAlignSize(DefaultDacl->AclSize);
    }

    DynamicLength = DynamicLengthUsed;

    //
    // Now create the token body
    //

    TokenBodyLength = FIELD_OFFSET(TOKEN, VariablePart) + VariableLength;

    if (DynamicLength < TOKEN_DEFAULT_DYNAMIC_CHARGE) {
        PagedPoolSize = TokenBodyLength + TOKEN_DEFAULT_DYNAMIC_CHARGE;
    } else {
        PagedPoolSize = TokenBodyLength + DynamicLength;
    }


    Status = ObCreateObject(
                 RequestorMode,      // ProbeMode
                 SeTokenObjectType, // ObjectType
                 ObjectAttributes,   // ObjectAttributes
                 UserMode,           // OwnershipMode
                 NULL,               // ParseContext
                 TokenBodyLength,    // ObjectBodySize
                 PagedPoolSize,      // PagedPoolCharge
                 0,                  // NonPagedPoolCharge
                 (PVOID *)&Token     // Return pointer to object
                 );

    if (!NT_SUCCESS(Status)) {
        ExFreePool( TokenLock );
        return Status;
    }


    //
    // Main Body initialization
    //

    Token->TokenLock = TokenLock;
    ExInitializeResourceLite( Token->TokenLock );

    ExAllocateLocallyUniqueId( &(Token->TokenId) );
    Token->ParentTokenId = RtlConvertLongToLuid(0);
    Token->OriginatingLogonSession = RtlConvertLongToLuid(0);
    Token->AuthenticationId = (*AuthenticationId);
    Token->TokenInUse = FALSE;
    Token->ModifiedId = NewModifiedId;
    Token->ExpirationTime = (*ExpirationTime);
    Token->TokenType = TokenType;
    Token->ImpersonationLevel = ImpersonationLevel;
    Token->TokenSource = (*TokenSource);

    Token->TokenFlags = TokenFlags;
    Token->SessionId = 0;

    Token->DynamicCharged  = PagedPoolSize - TokenBodyLength;
    Token->DynamicAvailable = 0;

    Token->DefaultOwnerIndex = DefaultOwnerIndex;
    Token->DefaultDacl = NULL;

    Token->VariableLength = VariableLength;
    Token->AuditPolicy.Overlay = 0;

    // Ensure SepTokenDeleteMethod knows the buffers aren't allocated yet.
    
    Token->ProxyData = NULL;
    Token->AuditData = NULL;
    Token->DynamicPart = NULL;

    //
    // Increment the reference count for this logon session
    // (fail if there is no corresponding logon session.)
    //

    Status = SepReferenceLogonSession (AuthenticationId,
                                       &Token->LogonSession);
    if (!NT_SUCCESS (Status)) {
        Token->TokenFlags |= TOKEN_SESSION_NOT_REFERENCED;
        Token->LogonSession = NULL;
        ObDereferenceObject (Token);
        return Status;
    }

#if DBG || TOKEN_LEAK_MONITOR

    Token->ProcessCid      = PsGetCurrentThread()->Cid.UniqueProcess;
    Token->ThreadCid       = PsGetCurrentThread()->Cid.UniqueThread;
    Token->CreateMethod    = 0xC; // Create
    Token->Count           = 0;
    Token->CaptureCount    = 0;

    RtlCopyMemory(
        Token->ImageFileName,
        PsGetCurrentProcess()->ImageFileName, 
        min(sizeof(Token->ImageFileName), sizeof(PsGetCurrentProcess()->ImageFileName))
        );

    Frames = RtlWalkFrameChain(
                 (PVOID)Token->CreateTrace,
                 TRACE_SIZE,
                 0
                 );

    if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
        
        RtlWalkFrameChain(
            (PVOID)&Token->CreateTrace[Frames],
            TRACE_SIZE - Frames,
            1
            );
    }

    SepAddTokenLogonSession(Token);

#endif
    
    if (ARGUMENT_PRESENT( ProxyData )) {

        Status = SepCopyProxyData(
                    &Token->ProxyData,
                    ProxyData
                    );

        if (!NT_SUCCESS(Status)) {
            ObDereferenceObject( Token );
            return( STATUS_NO_MEMORY );
        }

    } else {

        Token->ProxyData = NULL;
    }

    if (ARGUMENT_PRESENT( AuditData )) {

        Token->AuditData = ExAllocatePool( PagedPool, sizeof( SECURITY_TOKEN_AUDIT_DATA ));

        if (Token->AuditData == NULL) {
            ObDereferenceObject( Token );
            return( STATUS_NO_MEMORY );
        }

        *(Token->AuditData) = *AuditData;

    } else {

        Token->AuditData = NULL;
    }


    //
    //  Variable part initialization
    //  Data is in the following order:
    //
    //               Privileges array
    //               User (SID_AND_ATTRIBUTES)
    //               Groups (SID_AND_ATTRIBUTES)
    //               Restricted Sids (SID_AND_ATTRIBUTES)
    //               SIDs
    //

    Where = (PUCHAR) & Token->VariablePart ;

    Token->Privileges = (PLUID_AND_ATTRIBUTES) Where ;
    Token->PrivilegeCount = PrivilegeCount ;

    RtlCopyMemory(
        Where,
        Privileges,
        PrivilegeCount * sizeof(LUID_AND_ATTRIBUTES) );

    ASSERT( ComputedPrivLength >= PrivilegeCount * sizeof( LUID_AND_ATTRIBUTES ) );

    Where += ComputedPrivLength ;
    VariableLength -= ComputedPrivLength ;

    ASSERT( (((ULONG_PTR) Where ) & (sizeof(PVOID) - 1)) == 0 );

    //
    // Now, copy the sid and attributes arrays.
    //

    NextSidFree = (PSID) (Where + (sizeof( SID_AND_ATTRIBUTES ) *
                                   (GroupCount + 1) ) );

    Token->UserAndGroups = (PSID_AND_ATTRIBUTES) Where ;
    Token->UserAndGroupCount = GroupCount + 1 ;


    ASSERT(VariableLength >= ((GroupCount + 1) * (ULONG)sizeof(SID_AND_ATTRIBUTES)));

    VariableLength -= ((GroupCount + 1) * (ULONG)sizeof(SID_AND_ATTRIBUTES));
    Status = RtlCopySidAndAttributesArray(
                 1,
                 User,
                 VariableLength,
                 (PSID_AND_ATTRIBUTES)Where,
                 NextSidFree,
                 &NextSidFree,
                 &VariableLength
                 );

    Where += sizeof( SID_AND_ATTRIBUTES );

    ASSERT( (((ULONG_PTR) Where ) & (sizeof(PVOID) - 1)) == 0 );

    Status = RtlCopySidAndAttributesArray(
                 GroupCount,
                 Groups,
                 VariableLength,
                 (PSID_AND_ATTRIBUTES)Where,
                 NextSidFree,
                 &NextSidFree,
                 &VariableLength
                 );


    ASSERT(NT_SUCCESS(Status));


    Token->RestrictedSids = NULL;
    Token->RestrictedSidCount = 0;


    //
    //  Dynamic part initialization
    //  Data is in the following order:
    //
    //                  PrimaryGroup (SID)
    //                  Default Discreationary Acl (ACL)
    //

    Token->DynamicPart = (PULONG)ExAllocatePoolWithTag( PagedPool, DynamicLength, 'dTeS' );

    //
    // The attempt to allocate the DynamicPart of the token may have
    // failed.  Dereference the created object and exit with an error.
    //

    if (Token->DynamicPart == NULL) {
        ObDereferenceObject( Token );
        return( STATUS_NO_MEMORY );
    }


    Where = (PUCHAR) Token->DynamicPart;

    Token->PrimaryGroup = (PSID) Where;
    PrimaryGroupLength = RtlLengthRequiredSid( ((SID *)PrimaryGroup)->SubAuthorityCount );
    RtlCopySid( PrimaryGroupLength, (PSID)Where, PrimaryGroup );
    Where += (ULONG)LongAlignSize(PrimaryGroupLength);

    if (ARGUMENT_PRESENT(DefaultDacl)) {
        Token->DefaultDacl = (PACL)Where;

        RtlCopyMemory( (PVOID)Where,
                      (PVOID)DefaultDacl,
                      DefaultDacl->AclSize
                      );
    }

    //
    //  Insert the token unless it is a system token.
    //

    if (!SystemToken) {

        Status = SeCreateAccessState(
                     &AccessState,
                     &AuxData,
                     DesiredAccess,
                     &SeTokenObjectType->TypeInfo.GenericMapping
                     );

        if ( NT_SUCCESS(Status) ) {
            BOOLEAN PrivilegeHeld;

            PrivilegeHeld = SeSinglePrivilegeCheck(
                            SeCreateTokenPrivilege,
                            KeGetPreviousMode()
                            );

            if (PrivilegeHeld) {

                Status = ObInsertObject( Token,
                                         &AccessState,
                                         0,
                                         0,
                                         (PVOID *)NULL,
                                         TokenHandle
                                         );

            } else {

                Status = STATUS_PRIVILEGE_NOT_HELD;
                ObDereferenceObject( Token );
            }

            SeDeleteAccessState( &AccessState );

        } else {

            ObDereferenceObject( Token );
        }
    } else {

        ASSERT( NT_SUCCESS( Status ) );

        //
        // Insert the token unless this is phase0 initialization. The system token is inserted later.
        //
        if (!ExFastRefObjectNull (PsGetCurrentProcess()->Token)) {
            Status = ObInsertObject( Token,
                                     NULL,
                                     0,
                                     0,
                                     NULL,
                                     NULL
                                     );
        }
        if (NT_SUCCESS (Status)) {
            //
            // Return pointer instead of handle.
            //

            (*TokenHandle) = (HANDLE)Token;
        } else {
            (*TokenHandle) = NULL;
        }
    }

#if DBG || TOKEN_LEAK_MONITOR
    if (SepTokenLeakTracking && SepTokenLeakMethodWatch == 0xC && PsGetCurrentProcess()->UniqueProcessId == SepTokenLeakProcessCid) {
        
        Token->Count = InterlockedIncrement(&SepTokenLeakMethodCount);
        if (Token->Count >= SepTokenLeakBreakCount) {

            DbgPrint("\nToken number 0x%x = 0x%x\n", Token->Count, Token);
            DbgBreakPoint();
        }
    }
#endif

    return Status;

}

BOOLEAN
SepIdAssignableAsOwner(
    IN PTOKEN Token,
    IN ULONG Index
    )

/*++


Routine Description:

    This routine returns a boolean value indicating whether the user
    or group ID in the specified token with the specified index is
    assignable as the owner of an object.

    If the index is 0, which is always the USER ID, then the ID is
    assignable as owner.  Otherwise, the ID is that of a group, and
    it must have the "Owner" attribute set to be assignable.



Arguments:

    Token - Pointer to a locked Token to use.

    Index - Index into the Token's UserAndGroupsArray.  This value
        is assumed to be valid.

Return Value:

    TRUE  - Indicates the index corresponds to an ID that may be assigned
            as the owner of objects.

    FALSE - Indicates the index does not correspond to an ID that may be
            assigned as the owner of objects.

--*/
{
    PAGED_CODE();

    if (Index == 0) {

        return TRUE;

    } else {

        return (BOOLEAN)
                   ( (SepTokenGroupAttributes(Token,Index) & SE_GROUP_OWNER)
                     != 0
                   );
    }
}


NTSTATUS
SeIsChildToken(
    __in HANDLE Token,
    __out PBOOLEAN IsChild
    )
/*++

Routine Description:

    This routine returns TRUE if the supplied token is a child of the caller's
    process token. This is done by comparing the ParentTokenId field of the
    supplied token to the TokenId field of the token from the current subject
    context.

Arguments:

    Token - Token to check for childhood

    IsChild - Contains results of comparison.

        TRUE - The supplied token is a child of the caller's token
        FALSE- The supplied token is not a child of the caller's token

Returns:

    Status codes from any NT services called.

--*/
{
    PTOKEN CallerToken;
    PTOKEN SuppliedToken;
    LUID CallerTokenId;
    LUID SuppliedParentTokenId;
    NTSTATUS Status = STATUS_SUCCESS;
    PEPROCESS Process;

    *IsChild = FALSE;

    //
    // Capture the caller's token and get the token id
    //

    Process = PsGetCurrentProcess();
    CallerToken = (PTOKEN) PsReferencePrimaryToken(Process);

    if (CallerToken == NULL) {
        return STATUS_UNSUCCESSFUL;
    }

    CallerTokenId = CallerToken->TokenId;

    PsDereferencePrimaryTokenEx(Process, CallerToken);

    //
    // Reference the supplied token and get the parent token id.
    //

    Status = ObReferenceObjectByHandle(
                Token,                   // Handle
                0,                       // DesiredAccess
                SeTokenObjectType,      // ObjectType
                KeGetPreviousMode(),     // AccessMode
                (PVOID *)&SuppliedToken, // Object
                NULL                     // GrantedAccess
                );

    if (NT_SUCCESS(Status))
    {
        SuppliedParentTokenId = SuppliedToken->ParentTokenId;

        ObDereferenceObject(SuppliedToken);

        //
        // Check to see if the supplied token's parent ID is the ID
        // of the caller.
        //

        if (RtlEqualLuid(
                &SuppliedParentTokenId,
                &CallerTokenId
                )) {

            *IsChild = TRUE;
        }

    }
    return(Status);
}


NTSTATUS
SeIsChildTokenByPointer(
    __in PACCESS_TOKEN Token,
    __out PBOOLEAN IsChild
    )
/*++

Routine Description:

    This routine returns TRUE if the supplied token is a child of the caller's
    token. This is done by comparing the ParentTokenId field of the supplied
    token to the TokenId field of the token from the current subject context.

Arguments:

    Token - Token to check for childhood

    IsChild - Contains results of comparison.

        TRUE - The supplied token is a child of the caller's token
        FALSE- The supplied token is not a child of the caller's token

Returns:

    Status codes from any NT services called.

--*/
{
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    PTOKEN CallerToken;
    PTOKEN SuppliedToken;
    LUID CallerTokenId;
    LUID SuppliedParentTokenId;
    NTSTATUS Status = STATUS_SUCCESS;
    PEPROCESS Process;

    *IsChild = FALSE;

    //
    // Capture the caller's token and get the token id
    //

    Process = PsGetCurrentProcess();
    CallerToken = (PTOKEN) PsReferencePrimaryToken(Process);

    if (CallerToken == NULL) {
        return STATUS_UNSUCCESSFUL;
    }

    CallerTokenId = CallerToken->TokenId;

    PsDereferencePrimaryTokenEx(Process, CallerToken);

    SuppliedToken = (PTOKEN) Token;

    SuppliedParentTokenId = SuppliedToken->ParentTokenId;

    //
    // Check to see if the supplied token's parent ID is the ID
    // of the caller.
    //

    if (RtlEqualLuid(
            &SuppliedParentTokenId,
            &CallerTokenId
            )) {

        *IsChild = TRUE;
    }



    return(Status);
}

NTSTATUS
SeIsSiblingToken(
    __in HANDLE Token,
    __out PBOOLEAN IsSibling
    )

/*++

Routine Description:

    This routine returns TRUE if the supplied token is a sibling of the caller's
    process token. This is done by comparing the ParentTokenId field of the
    supplied token to the ParentTokenId field of the token from the current subject
    context. The authentication IDs of the tokens are also compared for equality.

Arguments:

    Token - Token to check for sibling relationship.

    IsSibling - Contains results of comparison.

        TRUE - The supplied token is a sibling of the caller's token
        FALSE- The supplied token is not a sibling of the caller's token

Returns:

    Status codes from any NT services called.

--*/
{
    PTOKEN CallerToken;
    PTOKEN SuppliedToken;
    LUID CallerParentTokenId;
    LUID SuppliedParentTokenId;
    LUID CallerAuthId;
    LUID SuppliedAuthId;
    NTSTATUS Status;
    PEPROCESS Process;

    Status     = STATUS_SUCCESS;
    *IsSibling = FALSE;

    //
    // Capture the caller's token and get the ParentTokenId.
    //

    Process             = PsGetCurrentProcess();
    CallerToken         = (PTOKEN)PsReferencePrimaryToken(Process);

    if (CallerToken == NULL) {
        return STATUS_UNSUCCESSFUL;
    }

    CallerParentTokenId = CallerToken->ParentTokenId;
    CallerAuthId        = CallerToken->AuthenticationId;
    
    PsDereferencePrimaryTokenEx(
        Process, 
        CallerToken
        );

    //
    // Reference the supplied token and get the parent token id.
    //

    Status = ObReferenceObjectByHandle(
                Token,                   // Handle
                0,                       // DesiredAccess
                SeTokenObjectType,      // ObjectType
                KeGetPreviousMode(),     // AccessMode
                (PVOID *)&SuppliedToken, // Object
                NULL                     // GrantedAccess
                );

    if (NT_SUCCESS(Status))
    {
        SuppliedParentTokenId = SuppliedToken->ParentTokenId;
        SuppliedAuthId = SuppliedToken->AuthenticationId;
        ObDereferenceObject(SuppliedToken);

        //
        // If the tokens have identical parent token Id fields and
        // identical authentication ids then one of the tokens is a 
        // duplicate of the other.
        //

        if (RtlEqualLuid(
                &SuppliedParentTokenId,
                &CallerParentTokenId
                )) {

            if (RtlEqualLuid(
                    &SuppliedAuthId,
                    &CallerAuthId
                    )) {
                
                *IsSibling = TRUE;
            }
        }
    }

    return Status;
}


NTSTATUS
SeIsSiblingTokenByPointer(
    __in PACCESS_TOKEN Token,
    __out PBOOLEAN IsSibling
    )

/*++

Routine Description:

    This routine returns TRUE if the supplied token is a sibling of the caller's
    process token. This is done by comparing the ParentTokenId field of the
    supplied token to the ParentTokenId field of the token from the current subject
    context.  The authentication IDs are also compared for equality.  

Arguments:

    Token - Token to check for sibling relationship.

    IsSibling - Contains results of comparison.

        TRUE - The supplied token is a sibling of the caller's token
        FALSE- The supplied token is not a sibling of the caller's token

Returns:

    Status codes from any NT services called.

--*/

{
    PTOKEN CallerToken;
    PTOKEN SuppliedToken;
    LUID CallerParentTokenId;
    LUID SuppliedParentTokenId;
    LUID CallerAuthId;
    LUID SuppliedAuthId;
    NTSTATUS Status = STATUS_SUCCESS;
    PEPROCESS Process;

    *IsSibling = FALSE;

    //
    // Capture the caller's token and get the token id
    //

    Process = PsGetCurrentProcess();
    CallerToken = (PTOKEN) PsReferencePrimaryToken(Process);

    if (CallerToken == NULL) {
        return STATUS_UNSUCCESSFUL;
    }

    CallerParentTokenId = CallerToken->ParentTokenId;
    CallerAuthId        = CallerToken->AuthenticationId;

    PsDereferencePrimaryTokenEx(
        Process, 
        CallerToken
        );

    SuppliedToken         = (PTOKEN)Token;
    SuppliedParentTokenId = SuppliedToken->ParentTokenId;
    SuppliedAuthId        = SuppliedToken->AuthenticationId;

    //
    // If the tokens have identical parent token Id fields then 
    // one of the tokens is a duplicate of the other.
    //

    if (RtlEqualLuid(
            &SuppliedParentTokenId,
            &CallerParentTokenId
            )) {

        if (RtlEqualLuid(
                &SuppliedAuthId,
                &CallerAuthId
                )) {

            *IsSibling = TRUE;
        }
    }

    return Status;
}

NTSTATUS
NtImpersonateAnonymousToken(
    __in HANDLE ThreadHandle
    )

/*++

Routine Description:

    Impersonates the system's anonymous logon token on this thread.

Arguments:

    ThreadHandle - Handle to the thread to do the impersonation.

Return Value:

    STATUS_SUCCESS - Indicates the operation was successful.

    STATUS_INVALID_HANDLE - the thread handle is invalid.

    STATUS_ACCESS_DENIED - The thread handle is not open for impersonation
        access.
--*/
{
    PETHREAD CallerThread = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    PACCESS_TOKEN Token = NULL;
    PEPROCESS Process;
    HANDLE hAnonymousToken = NULL;
    ULONG RegValue;

#define EVERYONE_INCLUDES_ANONYMOUS 1

    //
    // Reference the caller's thread to make sure we can impersonate
    //

    Status = ObReferenceObjectByHandle(
                 ThreadHandle,
                 THREAD_IMPERSONATE,
                 PsThreadType,
                 KeGetPreviousMode(),
                 (PVOID *)&CallerThread,
                 NULL
                 );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    //
    // Check the AnonymousIncludesEveryone reg key setting.
    //

    Status = SepRegQueryDwordValue(
                 L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa",
                 L"EveryoneIncludesAnonymous",
                 &RegValue
                 );
    
    if ( NT_SUCCESS( Status ) && ( RegValue == EVERYONE_INCLUDES_ANONYMOUS )) {

        hAnonymousToken = SeAnonymousLogonToken;
        
    } else {
        
        hAnonymousToken = SeAnonymousLogonTokenNoEveryone;

    };

    //
    // Reference the impersonation token to make sure we are allowed to
    // impersonate it.
    //

    Status = ObReferenceObjectByPointer(
                hAnonymousToken,
                TOKEN_IMPERSONATE,
                SeTokenObjectType,
                KeGetPreviousMode()
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    ObDereferenceObject(hAnonymousToken);

    Process = PsGetCurrentProcess();
    Token = PsReferencePrimaryToken(Process);

    if (Token == NULL) {
        Status = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }

    //
    // Do not allow anonymous impersonation if the primary token is restricted.
    //

    if (SeTokenIsRestricted(Token)) {
        PsDereferencePrimaryToken(Token);
        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }

    PsDereferencePrimaryTokenEx(Process, Token);

    //
    // Do the impersonation. We want copy on open so the caller can't
    // actually modify this system's copy of this token.
    //

    Status = PsImpersonateClient(
                CallerThread,
                hAnonymousToken,
                TRUE,                   // copy on open
                FALSE,                  // no effective only
                SecurityImpersonation
                );
Cleanup:

    if (CallerThread != NULL) {
        ObDereferenceObject(CallerThread);
    }
    return(Status);
}


#define SepEqualSidAndAttribute(a, b)                                          \
        ((RtlEqualSid((a).Sid, (b).Sid)) &&                                    \
         ((((a).Attributes ^ (b).Attributes) &                                 \
           (SE_GROUP_ENABLED | SE_GROUP_USE_FOR_DENY_ONLY)) == 0)              \
        )

#define SepEqualLuidAndAttribute(a, b)                                         \
        ((RtlEqualLuid(&(a).Luid, &(b).Luid)) &&                               \
         ((((a).Attributes ^ (b).Attributes) &  SE_PRIVILEGE_ENABLED) == 0)    \
        )

BOOLEAN
SepComparePrivilegeAndAttributeArrays(
    IN PLUID_AND_ATTRIBUTES PrivilegeArray1,
    IN ULONG Count1,
    IN PLUID_AND_ATTRIBUTES PrivilegeArray2,
    IN ULONG Count2
    )

/*++

Routine Description:

    This routine decides whether the given two privilege arrays are equivalent
    from AccessCheck perspective.

Arguments:

    PrivilegeArray1 - Privilege and attribute array from the first token.

    Count1 - Number of elements from the first array.

    PrivilegeArray2 - Privilege and attribute array from the second token.

    Count2 - Number of elements from the second array.


Return Value:

    TRUE - if the two arrays are equivalent
    FALSE - otherwise

--*/

{
    ULONG i = 0;
    ULONG j = 0;
    ULONG k = 0;

    //
    // If the number of privileges are not equal return FALSE.
    //

    if ( Count1 != Count2 ) {
        return FALSE;
    }

    //
    // In most cases when the privilege arrays are the same, the elements will
    // be ordered in the same manner. Walk the two arrays till we get a mismatch
    // or exhaust the number of entries in the array.
    //

    for ( k = 0; k < Count1; k++ ) {
        if ( !SepEqualLuidAndAttribute(PrivilegeArray1[k], PrivilegeArray2[k]) ) {
            break;
        }
    }

    //
    // If the arrays are identical return TRUE.
    //

    if ( k == Count1 ) {
        return TRUE;
    }

    //
    // Check if all the elements in the first array are present in the second.
    //

    for ( i = k; i < Count1; i++ ) {
        for ( j = k; j < Count2; j++ ) {
            if ( SepEqualLuidAndAttribute(PrivilegeArray1[i], PrivilegeArray2[j]) ) {
                break;
            }
        }

        //
        // The second array does not contain ith element from the first.
        //

        if ( j == Count2 ) {
            return FALSE;
        }
    }

    //
    // Check if all the elements in the second array are present in the first.
    //

    for ( i = k; i < Count2; i++ ) {
        for ( j = k; j < Count1; j++ ) {
            if ( SepEqualLuidAndAttribute(PrivilegeArray2[i], PrivilegeArray1[j]) ) {
                break;
            }
        }

        //
        // The first array does not contain ith element from the second.
        //

        if ( j == Count1 ) {
            return FALSE;
        }
    }

    //
    // If we are here, one array is a permutation of the other. Return TRUE.
    //

    return TRUE;
}

BOOLEAN
SepCompareSidAndAttributeArrays(
    IN PSID_AND_ATTRIBUTES SidArray1,
    IN ULONG Count1,
    IN PSID_AND_ATTRIBUTES SidArray2,
    IN ULONG Count2
    )
/*++

Routine Description:

    This routine decides whether the given two sid and attribute arrays are
    equivalentfrom AccessCheck perspective.

Arguments:

    SidArray1 - Sid and attribute array from the first token.

    Count1 - Number of elements from the first array.

    SidArray2 - Sid and attribute array from the second token.

    Count2 - Number of elements from the second array.


Return Value:

    TRUE - if the two arrays are equivalent
    FALSE - otherwise

--*/

{

    ULONG i = 0;
    ULONG j = 0;
    ULONG k = 0;

    //
    // If the number of groups sids are not equal return FALSE.
    //

    if ( Count1 != Count2 ) {
        return FALSE;
    }

    //
    // In most cases when the sid arrays are the same, the elements will
    // be ordered in the same manner. Walk the two arrays till we get a mismatch
    // or exhaust the number of entries in the array.
    //

    for ( k = 0; k < Count1; k++ ) {
        if ( !SepEqualSidAndAttribute(SidArray1[k], SidArray2[k]) ) {
            break;
        }
    }

    //
    // If the arrays are identical return TRUE.
    //

    if ( k == Count1 ) {
        return TRUE;
    }

    //
    // Check if all the elements in the first array are present in the second.
    //

    for ( i = k; i < Count1; i++ ) {
        for ( j = k; j < Count2; j++ ) {
            if ( SepEqualSidAndAttribute(SidArray1[i], SidArray2[j]) ) {
                break;
            }
        }

        //
        // The second array does not contain ith element from the first.
        //

        if ( j == Count2 ) {
            return FALSE;
        }
    }

    //
    // Check if all the elements in the second array are present in the first.
    //

    for ( i = k; i < Count2; i++ ) {
        for ( j = k; j < Count1; j++ ) {
            if ( SepEqualSidAndAttribute(SidArray2[i], SidArray1[j]) ) {
                break;
            }
        }

        //
        // The first array does not contain ith element from the second.
        //

        if ( j == Count1 ) {
            return FALSE;
        }
    }

    //
    // If we are here, one array is a permutation of the other. Return TRUE.
    //

    return TRUE;
}


NTSTATUS
NtCompareTokens(
    __in HANDLE FirstTokenHandle,
    __in HANDLE SecondTokenHandle,
    __out PBOOLEAN Equal
    )

/*++

Routine Description:

    This routine decides whether the given two tokens are equivalent from
    AccessCheck perspective.

    Two tokens are considered equal if all of the below are true.
      1. Every sid present in one token is the present in the other and vice-versa.
         The access check attributes (SE_GROUP_ENABLED and SE_GROUP_USE_FOR_DENY_ONLY)
         for these sids should match too.
      2. Either none or both the tokens are restricted.
      3. If both tokens are restricted then 1 should hold true for RestrictedSids.
      4. Every privilege present in the one token should be present in the other
         and vice-versa.


Arguments:

    FirstTokenHandle - Handle to the first token. The caller must have TOKEN_QUERY
        access to the token.

    SecondTokenHandle - Handle to the second token. The caller must have TOKEN_QUERY
        access to the token.

    Equal - To return whether the two tokens are equivalent from AccessCheck
        viewpoint.

Return Value:

    STATUS_SUCCESS - Indicates the operation was successful.

--*/

{

    PTOKEN TokenOne = NULL;
    PTOKEN TokenTwo = NULL;
    BOOLEAN RetVal = FALSE;
    NTSTATUS Status = STATUS_SUCCESS;
    KPROCESSOR_MODE PreviousMode;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

            ProbeForWriteBoolean(Equal);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }  // end_try

    } //end_if

    //
    // If its the same handle, return TRUE.
    //

    if ( FirstTokenHandle == SecondTokenHandle ) {
        RetVal = TRUE;
        goto Cleanup;
    }

    //
    // Reference the first token handle with TOKEN_QUERY access so that it does
    // not go away.
    //

    Status = ObReferenceObjectByHandle(
                 FirstTokenHandle,         // Handle
                 TOKEN_QUERY,              // DesiredAccess
                 SeTokenObjectType,       // ObjectType
                 PreviousMode,             // AccessMode
                 (PVOID *)&TokenOne,       // Object
                 NULL                      // GrantedAccess
                 );

    if (!NT_SUCCESS(Status)) {
        TokenOne = NULL;
        goto Cleanup;
    }


    //
    // Reference the second token handle with TOKEN_QUERY access so that it does
    // not go away.
    //

    Status = ObReferenceObjectByHandle(
                 SecondTokenHandle,        // Handle
                 TOKEN_QUERY,              // DesiredAccess
                 SeTokenObjectType,       // ObjectType
                 PreviousMode,             // AccessMode
                 (PVOID *)&TokenTwo,       // Object
                 NULL                      // GrantedAccess
                 );

    if (!NT_SUCCESS(Status)) {
        TokenTwo = NULL;
        goto Cleanup;
    }

    //
    // Acquire read lock on the first token.
    //

    SepAcquireTokenReadLock( TokenOne );

    //
    // Acquire read lock on the second token.
    //

    SepAcquireTokenReadLock( TokenTwo );


    //
    // Compare the user sid as well as its relevant attributes.
    //

    if ( !SepEqualSidAndAttribute(TokenOne->UserAndGroups[0], TokenTwo->UserAndGroups[0]) ) {
        goto Cleanup1;
    }

    //
    // Continue if both tokens are unrestricted OR
    //          if both tokens are restricted and Restricted arrays are equal.
    // Else return UNEQUAL.
    //

    if ( SeTokenIsRestricted( (PACCESS_TOKEN) TokenOne )) {
        if ( !SeTokenIsRestricted( (PACCESS_TOKEN) TokenTwo )) {
            goto Cleanup1;
        }

        RetVal = SepCompareSidAndAttributeArrays(
                     TokenOne->RestrictedSids,
                     TokenOne->RestrictedSidCount,
                     TokenTwo->RestrictedSids,
                     TokenTwo->RestrictedSidCount
                     );

        if (!RetVal) {
            goto Cleanup1;
        }

    } else {
        if ( SeTokenIsRestricted( (PACCESS_TOKEN) TokenTwo )) {
            goto Cleanup1;
        }
    }

    //
    // Compare the sid arrays.
    //

    RetVal = SepCompareSidAndAttributeArrays(
                 TokenOne->UserAndGroups+1,
                 TokenOne->UserAndGroupCount-1,
                 TokenTwo->UserAndGroups+1,
                 TokenTwo->UserAndGroupCount-1
                 );

    if (!RetVal) {
        goto Cleanup1;
    }

    //
    // Compare the privilege arrays.
    //

    RetVal = SepComparePrivilegeAndAttributeArrays(
                 TokenOne->Privileges,
                 TokenOne->PrivilegeCount,
                 TokenTwo->Privileges,
                 TokenTwo->PrivilegeCount
                 );
Cleanup1:

    SepReleaseTokenReadLock( TokenOne );
    SepReleaseTokenReadLock( TokenTwo );

Cleanup:

    if ( TokenOne != NULL) {
        ObDereferenceObject( TokenOne );
    }

    if ( TokenTwo != NULL) {
        ObDereferenceObject( TokenTwo );
    }

    try {
        *Equal = RetVal;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    return Status;
}

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg()
#endif

