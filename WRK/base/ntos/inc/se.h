/*++ BUILD Version: 0011    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    se.h

Abstract:

    This module contains the Security routines that are only callable
    from kernel mode.

    This file is included by including "ntos.h".

--*/

#ifndef _SE_
#define _SE_
#include <ntlsa.h>


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  Kernel mode only data structures                                        //
//  Opaque security data structures are defined in seopaque.h               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp
//
//  Security operation codes
//

typedef enum _SECURITY_OPERATION_CODE {
    SetSecurityDescriptor,
    QuerySecurityDescriptor,
    DeleteSecurityDescriptor,
    AssignSecurityDescriptor
    } SECURITY_OPERATION_CODE, *PSECURITY_OPERATION_CODE;

// end_ntddk end_wdm end_nthal end_ntifs end_ntosp



//
//  Default security quota
//
//  This is the minimum amount of quota (in bytes) that will be
//  charged for security information for an object that has
//  security.
//

#define SE_DEFAULT_SECURITY_QUOTA   2048

// begin_ntifs
//
// Token Flags
//
// Flags that may be defined in the TokenFlags field of the token object,
// or in an ACCESS_STATE structure
//

#define TOKEN_HAS_TRAVERSE_PRIVILEGE    0x01
#define TOKEN_HAS_BACKUP_PRIVILEGE      0x02
#define TOKEN_HAS_RESTORE_PRIVILEGE     0x04
#define TOKEN_HAS_ADMIN_GROUP           0x08
#define TOKEN_IS_RESTRICTED             0x10
#define TOKEN_SESSION_NOT_REFERENCED    0x20
#define TOKEN_SANDBOX_INERT             0x40
#define TOKEN_HAS_IMPERSONATE_PRIVILEGE 0x80

// end_ntifs


//
// General flag
//

#define SE_BACKUP_PRIVILEGES_CHECKED    0x00000010




// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp
//
//  Data structure used to capture subject security context
//  for access validations and auditing.
//
//  THE FIELDS OF THIS DATA STRUCTURE SHOULD BE CONSIDERED OPAQUE
//  BY ALL EXCEPT THE SECURITY ROUTINES.
//

typedef struct _SECURITY_SUBJECT_CONTEXT {
    PACCESS_TOKEN ClientToken;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    PACCESS_TOKEN PrimaryToken;
    PVOID ProcessAuditId;
    } SECURITY_SUBJECT_CONTEXT, *PSECURITY_SUBJECT_CONTEXT;

// end_ntddk end_wdm end_nthal end_ntifs end_ntosp
//
// where
//
//    ClientToken - optionally points to a token object being used by the
//        subject's thread to impersonate a client.  If the subject's
//        thread is not impersonating a client, this field is set to null.
//        The token's reference count is incremented to count this field
//        as an outstanding reference.
//
//    ImpersonationLevel - Contains the impersonation level of the subject's
//        thread.  This field is only meaningful if the ClientToken field
//        is not null.  This field over-rides any higher impersonation
//        level value that might be in the client's token.
//
//    PrimaryToken - points the the subject's primary token.  The token's
//        reference count is incremented to count this field value as an
//        outstanding reference.
//
//    ProcessAuditId - Is an ID assigned to represent the subject's process.
//        As an implementation detail, this is the process object's address.
//        However, this field should not be treated as a pointer, and the
//        reference count of the process object is not incremented to
//        count it as an outstanding reference.
//


// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                  ACCESS_STATE and related structures                      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
//  Initial Privilege Set - Room for three privileges, which should
//  be enough for most applications.  This structure exists so that
//  it can be embedded in an ACCESS_STATE structure.  Use PRIVILEGE_SET
//  for all other references to Privilege sets.
//

#define INITIAL_PRIVILEGE_COUNT         3

typedef struct _INITIAL_PRIVILEGE_SET {
    ULONG PrivilegeCount;
    ULONG Control;
    LUID_AND_ATTRIBUTES Privilege[INITIAL_PRIVILEGE_COUNT];
    } INITIAL_PRIVILEGE_SET, * PINITIAL_PRIVILEGE_SET;



//
// Combine the information that describes the state
// of an access-in-progress into a single structure
//


typedef struct _ACCESS_STATE {
   LUID OperationID;
   BOOLEAN SecurityEvaluated;
   BOOLEAN GenerateAudit;
   BOOLEAN GenerateOnClose;
   BOOLEAN PrivilegesAllocated;
   ULONG Flags;
   ACCESS_MASK RemainingDesiredAccess;
   ACCESS_MASK PreviouslyGrantedAccess;
   ACCESS_MASK OriginalDesiredAccess;
   SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
   PSECURITY_DESCRIPTOR SecurityDescriptor;
   PVOID AuxData;
   union {
      INITIAL_PRIVILEGE_SET InitialPrivilegeSet;
      PRIVILEGE_SET PrivilegeSet;
      } Privileges;

   BOOLEAN AuditPrivileges;
   UNICODE_STRING ObjectName;
   UNICODE_STRING ObjectTypeName;

   } ACCESS_STATE, *PACCESS_STATE;

// end_ntddk end_wdm end_nthal end_ntifs end_ntosp

/*
where:

    OperationID - an LUID to identify the operation being performed.  This
        ID will be put in the audit log to allow non-contiguous operations
        on the same object to be associated with one another.

    SecurityEvaluated - a marker to be set by Parse Methods to indicate
        that security access checking and audit logging has been performed.

    Flags - Holds misc flags for reference during the access attempt.

    AuditHandleCreation - a flag set by SeOpenObjectAuditAlarm to indicate
        that auditing is to take place when the handle for the object
        is allocated.

    RemainingDesiredAccess - Access mask containing the access types that
        have not yet been granted.

    PreviouslyGrantedAccess - Access mask containing the access types that
        have been granted, one way or another (for example, a given access
        may be granted as a result of owning a privilege rather than being
        in an ACL.  A routine can check the privilege and mark the access
        as granted without doing a formal access check).

    SubjectSecurityContext - The subject's captured security context

    PrivilegesAllocated - Flag to indicate whether we have allocated
        space for the privilege set from pool memory, so it can be
        freed.

    SecurityDescriptor - Temporarily contains the security descriptor
       for the object being created between the time the user's
       security descriptor is captured and the time the security
       descriptor is passed to SeAssignSecurity.  NO ONE BUT
       SEASSIGNSECURITY SHOULD EVER LOOK IN THIS FIELD FOR AN
       OBJECT'S SECURITY DESCRIPTOR.

    AuxData - points to an auxiliary data structure to be used for future
        expansion of the access state in an upwardly compatible way.  This
        field replaces the PrivilegesUsed pointer, which was for internal
        use only.

    Privileges - A set of privileges, some of which may have the
        UsedForAccess bit set.  If the pre-allocated number of privileges
        is not enough, we will allocate space from pool memory to allow
        for growth.

*/



//*******************************************************************************
//                                                                              *
//  Since the AccessState structure is publically exposed to driver             *
//  writers, this structure contains additional data added after NT 3.51.       *
//                                                                              *
//  Its contents must be accessed only through Se level interfaces,             *
//  never directly by name.                                                     *
//                                                                              *
//  This structure is pointed to by the AuxData field of the AccessState.       *
//  It is allocated by SeCreateAccessState and freed by SeDeleteAccessState.    *
//                                                                              *
//  DO NOT EXPOSE THIS STRUCTURE TO THE PUBLIC.                                 *
//                                                                              *
//*******************************************************************************

// begin_ntosp
typedef struct _AUX_ACCESS_DATA {
    PPRIVILEGE_SET PrivilegesUsed;
    GENERIC_MAPPING GenericMapping;
    ACCESS_MASK AccessesToAudit;
    ACCESS_MASK MaximumAuditMask;
} AUX_ACCESS_DATA, *PAUX_ACCESS_DATA;
// end_ntosp

/*
where:

    PrivilegesUsed - Points to the set of privileges used during the access
        validation.

    GenericMapping - Points to the generic mapping for the object being accessed.
        Normally this would be filled in with the generic mapping passed to
        SeCreateAccessState, but in the case of the IO system (which does not
        know the type of object being accessed until it parses the name),
        it must be filled in later.  See the discussion of the GenericMapping
        parameter in SeCreateAccessState for more details.

    AccessToAudit - Used as a temporary holding area for the access mask
        to put into the audit record.  This field is necessary because the
        access being put into the newly created handle may not be the ones
        we want to audit.  This occurs when a file is opened for read-only
        transacted mode, where a read only file is opened for write access.
        We don't want to audit the fact that we granted write access, since
        we really didn't, and customers would be confused to see the extra
        bit in the audit record.

   MaximumAuditMask - Stores the audit mask that will be stored into the
        new handle structure to support operation based audits.

*/



//
//  Structure describing whether or not a particular type of event
//  is being audited
//

typedef struct _SE_AUDITING_STATE {
    BOOLEAN AuditOnSuccess;
    BOOLEAN AuditOnFailure;
} SE_AUDITING_STATE, *PSE_AUDITING_STATE;




typedef struct _SE_PROCESS_AUDIT_INFO {
    PEPROCESS Process;
    PEPROCESS Parent;
} SE_PROCESS_AUDIT_INFO, *PSE_PROCESS_AUDIT_INFO;




/************************************************************

                 WARNING WARNING WARNING


    Only add new fields to the end of this structure.


*************************************************************/

// begin_ntifs begin_ntosp

typedef struct _SE_EXPORTS {

    //
    // Privilege values
    //

    LUID    SeCreateTokenPrivilege;
    LUID    SeAssignPrimaryTokenPrivilege;
    LUID    SeLockMemoryPrivilege;
    LUID    SeIncreaseQuotaPrivilege;
    LUID    SeUnsolicitedInputPrivilege;
    LUID    SeTcbPrivilege;
    LUID    SeSecurityPrivilege;
    LUID    SeTakeOwnershipPrivilege;
    LUID    SeLoadDriverPrivilege;
    LUID    SeCreatePagefilePrivilege;
    LUID    SeIncreaseBasePriorityPrivilege;
    LUID    SeSystemProfilePrivilege;
    LUID    SeSystemtimePrivilege;
    LUID    SeProfileSingleProcessPrivilege;
    LUID    SeCreatePermanentPrivilege;
    LUID    SeBackupPrivilege;
    LUID    SeRestorePrivilege;
    LUID    SeShutdownPrivilege;
    LUID    SeDebugPrivilege;
    LUID    SeAuditPrivilege;
    LUID    SeSystemEnvironmentPrivilege;
    LUID    SeChangeNotifyPrivilege;
    LUID    SeRemoteShutdownPrivilege;


    //
    // Universally defined Sids
    //


    PSID  SeNullSid;
    PSID  SeWorldSid;
    PSID  SeLocalSid;
    PSID  SeCreatorOwnerSid;
    PSID  SeCreatorGroupSid;


    //
    // Nt defined Sids
    //


    PSID  SeNtAuthoritySid;
    PSID  SeDialupSid;
    PSID  SeNetworkSid;
    PSID  SeBatchSid;
    PSID  SeInteractiveSid;
    PSID  SeLocalSystemSid;
    PSID  SeAliasAdminsSid;
    PSID  SeAliasUsersSid;
    PSID  SeAliasGuestsSid;
    PSID  SeAliasPowerUsersSid;
    PSID  SeAliasAccountOpsSid;
    PSID  SeAliasSystemOpsSid;
    PSID  SeAliasPrintOpsSid;
    PSID  SeAliasBackupOpsSid;

    //
    // New Sids defined for NT5
    //

    PSID  SeAuthenticatedUsersSid;

    PSID  SeRestrictedSid;
    PSID  SeAnonymousLogonSid;

    //
    // New Privileges defined for NT5
    //

    LUID  SeUndockPrivilege;
    LUID  SeSyncAgentPrivilege;
    LUID  SeEnableDelegationPrivilege;

    //
    // New Sids defined for post-Windows 2000

    PSID  SeLocalServiceSid;
    PSID  SeNetworkServiceSid;

    //
    // New Privileges defined for post-Windows 2000
    //

    LUID  SeManageVolumePrivilege;
    LUID  SeImpersonatePrivilege;
    LUID  SeCreateGlobalPrivilege;

} SE_EXPORTS, *PSE_EXPORTS;

// end_ntifs end_ntosp

/************************************************************


                 WARNING WARNING WARNING


    Only add new fields to the end of this structure.


*************************************************************/



// begin_ntifs
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//              Logon session notification callback routines                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
//  These callback routines are used to notify file systems that have
//  registered of logon sessions being terminated, so they can cleanup state
//  associated with this logon session
//

typedef NTSTATUS
(*PSE_LOGON_SESSION_TERMINATED_ROUTINE)(
    IN PLUID LogonId);

// end_ntifs





///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                  Exported Security Macro Definitions                      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//++
//
//  ACCESS_MASK
//  SeComputeDeniedAccesses(
//      IN ACCESS_MASK GrantedAccess,
//      IN ACCESS_MASK DesiredAccess
//      );
//
//  Routine Description:
//
//      This routine generates an access mask containing those accesses
//      requested by DesiredAccess that aren't granted by GrantedAccess.
//      The result of this routine may be compared to 0 to determine
//      if a DesiredAccess mask contains any accesses that have not
//      been granted.
//
//      If the result IS ZERO, then all desired accesses have been granted.
//
//  Arguments:
//
//      GrantedAccess - Specifies the granted access mask.
//
//      DesiredAccess - Specifies the desired access mask.
//
//  Return Value:
//
//      An ACCESS_MASK containing the desired accesses that have
//      not been granted.
//
//--

#define SeComputeDeniedAccesses( GrantedAccess, DesiredAccess ) \
    ((~(GrantedAccess)) & (DesiredAccess) )


//++
//
//  BOOLEAN
//  SeComputeGrantedAccesses(
//      IN ACCESS_MASK GrantedAccess,
//      IN ACCESS_MASK DesiredAccess
//      );
//
//  Routine Description:
//
//      This routine generates an access mask containing accesses
//      requested by DesiredAccess that are granted by GrantedAccess.
//      The result of this routine may be compared to 0 to determine
//      if any desired accesses have been granted.
//
//      If the result IS NON-ZERO, then at least one desired accesses
//      has been granted.
//
//  Arguments:
//
//      GrantedAccess - Specifies the granted access mask.
//
//      DesiredAccess - Specifies the desired access mask.
//
//  Return Value:
//
//      This routine returns TRUE if the DesiredAccess mask does specifies
//      any bits that are set in the GrantedAccess mask.
//
//--

#define SeComputeGrantedAccesses( GrantedAccess, DesiredAccess ) \
    ((GrantedAccess) & (DesiredAccess) )


// begin_ntifs
//++
//
//  ULONG
//  SeLengthSid(
//      IN PSID Sid
//      );
//
//  Routine Description:
//
//      This routine computes the length of a SID.
//
//  Arguments:
//
//      Sid - Points to the SID whose length is to be returned.
//
//  Return Value:
//
//      The length, in bytes of the SID.
//
//--

#define SeLengthSid( Sid ) \
    (8 + (4 * ((SID *)Sid)->SubAuthorityCount))

// end_ntifs


//++
//  BOOLEAN
//  SeSameToken (
//      IN PTOKEN_CONTROL TokenControl1,
//      IN PTOKEN_CONTROL TokenControl2
//      )
//
//
//  Routine Description:
//
//      This routine returns a boolean value indicating whether the two
//      token control values represent the same token.  The token may
//      have changed over time, but must have the same authentication ID
//      and token ID.  A value of TRUE indicates they
//      are equal.  A value of FALSE indicates they are not equal.
//
//
//
//  Arguments:
//
//      TokenControl1 - Points to a token control to compare.
//
//      TokenControl2 - Points to the other token control to compare.
//
//  Return Value:
//
//      TRUE => The token control values represent the same token.
//
//      FALSE => The token control values do not represent the same token.
//
//
//--

#define SeSameToken(TC1,TC2)  (                                               \
        ((TC1)->TokenId.HighPart == (TC2)->TokenId.HighPart)               && \
        ((TC1)->TokenId.LowPart  == (TC2)->TokenId.LowPart)                && \
        (RtlEqualLuid(&(TC1)->AuthenticationId,&(TC2)->AuthenticationId))     \
        )


// begin_ntifs
//
// VOID
// SeDeleteClientSecurity(
//    IN PSECURITY_CLIENT_CONTEXT ClientContext
//    )
//
///*++
//
// Routine Description:
//
//    This service deletes a client security context block,
//    performing whatever cleanup might be necessary to do so.  In
//    particular, reference to any client token is removed.
//
// Arguments:
//
//    ClientContext - Points to the client security context block to be
//        deleted.
//
//
// Return Value:
//
//
//
//--*/
//--

// begin_ntosp
#define SeDeleteClientSecurity(C)  {                                           \
            if (SeTokenType((C)->ClientToken) == TokenPrimary) {               \
                PsDereferencePrimaryToken( (C)->ClientToken );                 \
            } else {                                                           \
                PsDereferenceImpersonationToken( (C)->ClientToken );           \
            }                                                                  \
        }


//++
// VOID
// SeStopImpersonatingClient()
//
///*++
//
// Routine Description:
//
//    This service is used to stop impersonating a client using an
//    impersonation token.  This service must be called in the context
//    of the server thread which wishes to stop impersonating its
//    client.
//
//
// Arguments:
//
//    None.
//
// Return Value:
//
//    None.
//
//--*/
//--

#define SeStopImpersonatingClient() PsRevertToSelf()

// end_ntosp end_ntifs

#define SeAssertMappedCanonicalAccess( AccessMask )                  \
    ASSERT(!( ( AccessMask ) &                                       \
            ( GENERIC_READ        |                                  \
              GENERIC_WRITE       |                                  \
              GENERIC_EXECUTE     |                                  \
              GENERIC_ALL ))                                         \
          )
/*++

Routine Description:

    This routine asserts that the given AccessMask does not contain
    any generic access types.

Arguments:

    AccessMask - The access mask to be checked.

Return Value:

    None, or doesn't return.

--*/



#define SeComputeSecurityQuota( Size )                                 \
    (                                                                  \
       ((( Size ) * 2 )  > SE_DEFAULT_SECURITY_QUOTA) ?                \
                    (( Size ) * 2 ) : SE_DEFAULT_SECURITY_QUOTA        \
    )

/*++

Routine Description:

    This macro computes the amount of quota to charge for
    security information.

    The current algorithm is to use the larger of twice the size
    of the Group + Dacl information being applied and the default as
    specified by SE_DEFAULT_SECURITY_QUOTA.

Arguments:

    Size - The size in bytes of the Group + Dacl information being applied
        to the object.

Return Value:

    The size in bytes to charge for security information on this object.

--*/

// begin_ntifs begin_ntosp

//++
//
//  PACCESS_TOKEN
//  SeQuerySubjectContextToken(
//      IN PSECURITY_SUBJECT_CONTEXT SubjectContext
//      );
//
//  Routine Description:
//
//      This routine returns the effective token from the subject context,
//      either the client token, if present, or the process token.
//
//  Arguments:
//
//      SubjectContext - Context to query
//
//  Return Value:
//
//      This routine returns the PACCESS_TOKEN for the effective token.
//      The pointer may be passed to SeQueryInformationToken.  This routine
//      does not affect the lock status of the token, i.e. the token is not
//      locked.  If the SubjectContext has been locked, the token remains locked,
//      if not, the token remains unlocked.
//
//--

#define SeQuerySubjectContextToken( SubjectContext ) \
        ( ARGUMENT_PRESENT( ((PSECURITY_SUBJECT_CONTEXT) SubjectContext)->ClientToken) ? \
            ((PSECURITY_SUBJECT_CONTEXT) SubjectContext)->ClientToken : \
            ((PSECURITY_SUBJECT_CONTEXT) SubjectContext)->PrimaryToken )

// end_ntifs end_ntosp





///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Define the exported procedures that are callable only from kernel mode   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

BOOLEAN
SeInitSystem( VOID );

VOID
SeSetSecurityAccessMask(
    __in SECURITY_INFORMATION SecurityInformation,
    __out PACCESS_MASK DesiredAccess
    );

VOID
SeQuerySecurityAccessMask(
    __in SECURITY_INFORMATION SecurityInformation,
    __out PACCESS_MASK DesiredAccess
    );


NTSTATUS
SeDefaultObjectMethod (
    __in PVOID Object,
    __in SECURITY_OPERATION_CODE OperationCode,
    __in PSECURITY_INFORMATION SecurityInformation,
    __inout PSECURITY_DESCRIPTOR SecurityDescriptor,
    __inout_opt PULONG CapturedLength,
    __deref_inout PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    __in POOL_TYPE PoolType,
    __in PGENERIC_MAPPING GenericMapping
    );

// begin_ntosp
NTKERNELAPI
NTSTATUS
SeCaptureSecurityDescriptor (
    __in PSECURITY_DESCRIPTOR InputSecurityDescriptor,
    __in KPROCESSOR_MODE RequestorMode,
    __in POOL_TYPE PoolType,
    __in BOOLEAN ForceCapture,
    __deref_out PSECURITY_DESCRIPTOR *OutputSecurityDescriptor
    );

NTKERNELAPI
VOID
SeReleaseSecurityDescriptor (
    __in PSECURITY_DESCRIPTOR CapturedSecurityDescriptor,
    __in KPROCESSOR_MODE RequestorMode,
    __in BOOLEAN ForceCapture
    );

// begin_ntifs

NTKERNELAPI
VOID
SeCaptureSubjectContext (
    __out PSECURITY_SUBJECT_CONTEXT SubjectContext
    );


NTKERNELAPI
VOID
SeLockSubjectContext(
    __in PSECURITY_SUBJECT_CONTEXT SubjectContext
    );

NTKERNELAPI
VOID
SeUnlockSubjectContext(
    __in PSECURITY_SUBJECT_CONTEXT SubjectContext
    );

NTKERNELAPI
VOID
SeReleaseSubjectContext (
    __inout PSECURITY_SUBJECT_CONTEXT SubjectContext
    );

NTSTATUS
SeCaptureAuditPolicy(
    __in PTOKEN_AUDIT_POLICY Policy,
    __in KPROCESSOR_MODE RequestorMode,
    __in_bcount_opt(CaptureBufferLength) PVOID CaptureBuffer,
    __in ULONG CaptureBufferLength,
    __in POOL_TYPE PoolType,
    __in BOOLEAN ForceCapture,
    __deref_out PTOKEN_AUDIT_POLICY *CapturedPolicy
    );

VOID
SeReleaseAuditPolicy (
    __in PTOKEN_AUDIT_POLICY CapturedPolicy,
    __in KPROCESSOR_MODE RequestorMode,
    __in BOOLEAN ForceCapture
    );

// end_ntifs end_ntosp

VOID
SeCaptureSubjectContextEx (
    __in PETHREAD Thread,
    __in PEPROCESS Process,
    __out PSECURITY_SUBJECT_CONTEXT SubjectContext
    );

NTSTATUS
SeCaptureSecurityQos (
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in KPROCESSOR_MODE RequestorMode,
    __out PBOOLEAN SecurityQosPresent,
    __out PSECURITY_ADVANCED_QUALITY_OF_SERVICE CapturedSecurityQos
    );

VOID
SeFreeCapturedSecurityQos(
    __in PVOID SecurityQos
    );

NTSTATUS
SeCaptureSid (
    __in PSID InputSid,
    __in KPROCESSOR_MODE RequestorMode,
    __inout_bcount_opt(CaptureBufferLength) PVOID CaptureBuffer,
    __in ULONG CaptureBufferLength,
    __in POOL_TYPE PoolType,
    __in BOOLEAN ForceCapture,
    __deref_out PSID *CapturedSid
    );


VOID
SeReleaseSid (
    __in PSID CapturedSid,
    __in KPROCESSOR_MODE RequestorMode,
    __in BOOLEAN ForceCapture
    );


NTSTATUS
SeCaptureAcl (
    __in PACL InputAcl,
    __in KPROCESSOR_MODE RequestorMode,
    __inout_bcount_opt(CaptureBufferLength) PVOID CaptureBuffer,
    __in ULONG CaptureBufferLength,
    __in POOL_TYPE PoolType,
    __in BOOLEAN ForceCapture,
    __deref_out_bcount_full(*AlignedAclSize) PACL *CapturedAcl,
    __out PULONG AlignedAclSize
    );


VOID
SeReleaseAcl (
    __in PACL CapturedAcl,
    __in KPROCESSOR_MODE RequestorMode,
    __in BOOLEAN ForceCapture
    );


NTSTATUS
SeCaptureLuidAndAttributesArray (
    __in_ecount(ArrayCount) PLUID_AND_ATTRIBUTES InputArray,
    __in ULONG ArrayCount,
    __in KPROCESSOR_MODE RequestorMode,
    __in_bcount_opt(CaptureBufferLength) PVOID CaptureBuffer,
    __in ULONG CaptureBufferLength,
    __in POOL_TYPE PoolType,
    __in BOOLEAN ForceCapture,
    __deref_out_bcount_full(*AlignedArraySize) PLUID_AND_ATTRIBUTES *CapturedArray,
    __out PULONG AlignedArraySize
    );



VOID
SeReleaseLuidAndAttributesArray (
    __in PLUID_AND_ATTRIBUTES CapturedArray,
    __in KPROCESSOR_MODE RequestorMode,
    __in BOOLEAN ForceCapture
    );



NTSTATUS
SeCaptureSidAndAttributesArray (
    __in_ecount(ArrayCount) PSID_AND_ATTRIBUTES InputArray,
    __in ULONG ArrayCount,
    __in KPROCESSOR_MODE RequestorMode,
    __in_bcount_opt(CaptureBufferLength) PVOID CaptureBuffer,
    __in ULONG CaptureBufferLength,
    __in POOL_TYPE PoolType,
    __in BOOLEAN ForceCapture,
    __deref_out_bcount_full(AlignedArraySize) PSID_AND_ATTRIBUTES *CapturedArray,
    __out PULONG AlignedArraySize
    );


VOID
SeReleaseSidAndAttributesArray (
    __in PSID_AND_ATTRIBUTES CapturedArray,
    __in KPROCESSOR_MODE RequestorMode,
    __in BOOLEAN ForceCapture
    );

// begin_ntddk begin_wdm begin_ntifs begin_ntosp

NTKERNELAPI
NTSTATUS
SeAssignSecurity (
    __in_opt PSECURITY_DESCRIPTOR ParentDescriptor,
    __in_opt PSECURITY_DESCRIPTOR ExplicitDescriptor,
    __out PSECURITY_DESCRIPTOR *NewDescriptor,
    __in BOOLEAN IsDirectoryObject,
    __in PSECURITY_SUBJECT_CONTEXT SubjectContext,
    __in PGENERIC_MAPPING GenericMapping,
    __in POOL_TYPE PoolType
    );

NTKERNELAPI
NTSTATUS
SeAssignSecurityEx (
    __in_opt PSECURITY_DESCRIPTOR ParentDescriptor,
    __in_opt PSECURITY_DESCRIPTOR ExplicitDescriptor,
    __out PSECURITY_DESCRIPTOR *NewDescriptor,
    __in_opt GUID *ObjectType,
    __in BOOLEAN IsDirectoryObject,
    __in ULONG AutoInheritFlags,
    __in PSECURITY_SUBJECT_CONTEXT SubjectContext,
    __in PGENERIC_MAPPING GenericMapping,
    __in POOL_TYPE PoolType
    );

NTKERNELAPI
NTSTATUS
SeDeassignSecurity (
    __deref_inout PSECURITY_DESCRIPTOR *SecurityDescriptor
    );

NTKERNELAPI
BOOLEAN
SeAccessCheck (
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    __in BOOLEAN SubjectContextLocked,
    __in ACCESS_MASK DesiredAccess,
    __in ACCESS_MASK PreviouslyGrantedAccess,
    __deref_out_opt PPRIVILEGE_SET *Privileges,
    __in PGENERIC_MAPPING GenericMapping,
    __in KPROCESSOR_MODE AccessMode,
    __out PACCESS_MASK GrantedAccess,
    __out PNTSTATUS AccessStatus
    );


#ifdef SE_NTFS_WORLD_CACHE

VOID
SeGetWorldRights (
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in PGENERIC_MAPPING GenericMapping,
    __out PACCESS_MASK GrantedAccess
    );

#endif

NTSTATUS
SeSetAuditParameter(
    __inout PSE_ADT_PARAMETER_ARRAY AuditParameters,
    __in SE_ADT_PARAMETER_TYPE Type,
    __in ULONG Index,
    __in PVOID Data
    );

NTSTATUS
SeReportSecurityEvent(
    __in ULONG Flags,
    __in PUNICODE_STRING SourceName,
    __in_opt PSID UserSid,
    __in PSE_ADT_PARAMETER_ARRAY AuditParameters
    );

// end_ntddk end_wdm end_ntifs end_ntosp


// begin_ntifs begin_ntosp

NTKERNELAPI
BOOLEAN
SePrivilegeCheck(
    __inout PPRIVILEGE_SET RequiredPrivileges,
    __in PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    __in KPROCESSOR_MODE AccessMode
    );

NTKERNELAPI
VOID
SeFreePrivileges(
    __in PPRIVILEGE_SET Privileges
    );

// end_ntifs end_ntosp

NTSTATUS
SePrivilegePolicyCheck(
    __inout PACCESS_MASK RemainingDesiredAccess,
    __inout PACCESS_MASK PreviouslyGrantedAccess,
    __in_opt PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    __in_opt PACCESS_TOKEN ExplicitToken,
    __deref_out PPRIVILEGE_SET *PrivilegeSet,
    __in KPROCESSOR_MODE PreviousMode
    );

VOID
SeGenerateMessage (
    IN PSTRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PACCESS_TOKEN Token,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN AccessGranted,
    IN HANDLE AuditPort,
    IN HANDLE AlarmPort,
    IN KPROCESSOR_MODE AccessMode
    );

// begin_ntifs

NTKERNELAPI
VOID
SeOpenObjectAuditAlarm (
    __in PUNICODE_STRING ObjectTypeName,
    __in_opt PVOID Object,
    __in_opt PUNICODE_STRING AbsoluteObjectName,
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in PACCESS_STATE AccessState,
    __in BOOLEAN ObjectCreated,
    __in BOOLEAN AccessGranted,
    __in KPROCESSOR_MODE AccessMode,
    __out PBOOLEAN GenerateOnClose
    );

NTKERNELAPI
VOID
SeOpenObjectForDeleteAuditAlarm (
    __in PUNICODE_STRING ObjectTypeName,
    __in_opt PVOID Object,
    __in_opt PUNICODE_STRING AbsoluteObjectName,
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in PACCESS_STATE AccessState,
    __in BOOLEAN ObjectCreated,
    __in BOOLEAN AccessGranted,
    __in KPROCESSOR_MODE AccessMode,
    __out PBOOLEAN GenerateOnClose
    );

NTKERNELAPI
VOID
SeDeleteObjectAuditAlarm(
    __in PVOID Object,
    __in HANDLE Handle
    );


// end_ntifs

VOID
SeCloseObjectAuditAlarm(
    __in PVOID Object,
    __in HANDLE Handle,
    __in BOOLEAN GenerateOnClose
    );

VOID
SeCreateInstanceAuditAlarm(
    IN PLUID OperationID OPTIONAL,
    IN PVOID Object,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN ACCESS_MASK DesiredAccess,
    IN PPRIVILEGE_SET Privileges OPTIONAL,
    IN BOOLEAN AccessGranted,
    IN KPROCESSOR_MODE AccessMode
    );

VOID
SeCreateObjectAuditAlarm(
    IN PLUID OperationID OPTIONAL,
    IN PVOID Object,
    IN PUNICODE_STRING ComponentName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN ACCESS_MASK DesiredAccess,
    IN PPRIVILEGE_SET Privileges OPTIONAL,
    IN BOOLEAN AccessGranted,
    OUT PBOOLEAN AuditPerformed,
    IN KPROCESSOR_MODE AccessMode
    );

VOID
SeObjectReferenceAuditAlarm(
    __in_opt PLUID OperationID,
    __in PVOID Object,
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    __in ACCESS_MASK DesiredAccess,
    __in_opt PPRIVILEGE_SET Privileges,
    __in BOOLEAN AccessGranted,
    __in KPROCESSOR_MODE AccessMode
    );

// begin_ntosp
NTKERNELAPI
VOID
SePrivilegeObjectAuditAlarm(
    __in HANDLE Handle,
    __in PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    __in ACCESS_MASK DesiredAccess,
    __in PPRIVILEGE_SET Privileges,
    __in BOOLEAN AccessGranted,
    __in KPROCESSOR_MODE AccessMode
    );
// end_ntosp

BOOLEAN
SeCheckPrivilegedObject(
    __in LUID PrivilegeValue,
    __in HANDLE ObjectHandle,
    __in ACCESS_MASK DesiredAccess,
    __in KPROCESSOR_MODE PreviousMode
    );

// begin_ntddk begin_wdm begin_ntifs

NTKERNELAPI
BOOLEAN
SeValidSecurityDescriptor(
    __in ULONG Length,
    __in_bcount(Length) PSECURITY_DESCRIPTOR SecurityDescriptor
    );

// end_ntddk end_wdm end_ntifs



// VOID
// SeImplicitObjectAuditAlarm(
//    IN PLUID OperationID OPTIONAL,
//    IN PVOID Object,
//    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
//    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
//    IN ACCESS_MASK DesiredAccess,
//    IN PPRIVILEGE_SET Privileges OPTIONAL,
//    IN BOOLEAN AccessGranted,
//    IN KPROCESSOR_MODE AccessMode
//    );
//

VOID
SeAuditHandleCreation(
    __in PACCESS_STATE AccessState,
    __in HANDLE Handle
    );



PACCESS_TOKEN
SeMakeSystemToken (
    VOID
    );

PACCESS_TOKEN
SeMakeAnonymousLogonToken (
    VOID
    );

PACCESS_TOKEN
SeMakeAnonymousLogonTokenNoEveryone (
    VOID
    );

VOID
SeGetTokenControlInformation (
    __in PACCESS_TOKEN Token,
    __out PTOKEN_CONTROL TokenControl
    );

// begin_ntosp
extern struct _OBJECT_TYPE *SeTokenObjectType;

NTKERNELAPI                                     // ntifs
TOKEN_TYPE                                      // ntifs
SeTokenType(                                    // ntifs
    __in PACCESS_TOKEN Token                    // ntifs
    );                                          // ntifs

SECURITY_IMPERSONATION_LEVEL
SeTokenImpersonationLevel(
    __in PACCESS_TOKEN Token
    );

NTKERNELAPI                                     // ntifs
BOOLEAN                                         // ntifs
SeTokenIsAdmin(                                 // ntifs
    __in PACCESS_TOKEN Token                    // ntifs
    );                                          // ntifs


NTKERNELAPI                                     // ntifs
BOOLEAN                                         // ntifs
SeTokenIsRestricted(                            // ntifs
    __in PACCESS_TOKEN Token                    // ntifs
    );                                          // ntifs

NTKERNELAPI
NTSTATUS
SeTokenCanImpersonate(
    __in PACCESS_TOKEN ProcessToken,
    __in PACCESS_TOKEN Token,
    __in SECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );
// end_ntosp

NTSTATUS
SeSubProcessToken (
    __in  PACCESS_TOKEN ParentToken,
    __deref_out PACCESS_TOKEN *ChildToken,
    __in  BOOLEAN MarkAsActive,
    __in  ULONG SessionId
    );

VOID
SeAssignPrimaryToken(
    __in PEPROCESS Process,
    __in PACCESS_TOKEN Token
    );

VOID
SeDeassignPrimaryToken(
    __in PEPROCESS Process
    );

NTSTATUS
SeExchangePrimaryToken(
    __in PEPROCESS Process,
    __in PACCESS_TOKEN NewAccessToken,
    __deref_out PACCESS_TOKEN *OldAccessToken
    );

NTSTATUS
SeCopyClientToken(
    __in PACCESS_TOKEN ClientToken,
    __in SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    __in KPROCESSOR_MODE RequestorMode,
    __deref_out PACCESS_TOKEN *DuplicateToken
    );

// begin_ntifs

NTKERNELAPI
NTSTATUS
SeFilterToken (
    __in PACCESS_TOKEN ExistingToken,
    __in ULONG Flags,
    __in_opt PTOKEN_GROUPS SidsToDisable,
    __in_opt PTOKEN_PRIVILEGES PrivilegesToDelete,
    __in_opt PTOKEN_GROUPS RestrictedSids,
    __deref_out PACCESS_TOKEN * FilteredToken
    );

// begin_ntosp
NTKERNELAPI
NTSTATUS
SeQueryAuthenticationIdToken(
    __in PACCESS_TOKEN Token,
    __out PLUID AuthenticationId
    );

// end_ntosp
NTKERNELAPI
NTSTATUS
SeQuerySessionIdToken(
    __in PACCESS_TOKEN Token,
    __out PULONG SessionId
    );

NTKERNELAPI
NTSTATUS
SeSetSessionIdToken(
    __in PACCESS_TOKEN Token,
    __in ULONG SessionId
    );

// begin_ntosp
NTKERNELAPI
NTSTATUS
SeCreateClientSecurity (
    __in PETHREAD ClientThread,
    __in PSECURITY_QUALITY_OF_SERVICE ClientSecurityQos,
    __in BOOLEAN RemoteSession,
    __out PSECURITY_CLIENT_CONTEXT ClientContext
    );
// end_ntosp

NTKERNELAPI
VOID
SeImpersonateClient(
    __in PSECURITY_CLIENT_CONTEXT ClientContext,
    __in_opt PETHREAD ServerThread
    );

// begin_ntosp
NTKERNELAPI
NTSTATUS
SeImpersonateClientEx(
    __in PSECURITY_CLIENT_CONTEXT ClientContext,
    __in_opt PETHREAD ServerThread
    );
// end_ntosp

NTKERNELAPI
NTSTATUS
SeCreateClientSecurityFromSubjectContext (
    __in PSECURITY_SUBJECT_CONTEXT SubjectContext,
    __in PSECURITY_QUALITY_OF_SERVICE ClientSecurityQos,
    __in BOOLEAN ServerIsRemote,
    __out PSECURITY_CLIENT_CONTEXT ClientContext
    );

// end_ntifs

//
// Do not export the following routines to drivers.
// If you need to do so, create a new routine that
// does not take the AuxData parameter and export
// that.
//

// begin_ntosp
NTKERNELAPI
NTSTATUS
SeCreateAccessState(
   __out PACCESS_STATE AccessState,
   __out PAUX_ACCESS_DATA AuxData,
   __in ACCESS_MASK DesiredAccess,
   __in PGENERIC_MAPPING GenericMapping
   );

NTKERNELAPI
VOID
SeDeleteAccessState(
    __in PACCESS_STATE AccessState
    );
// end_ntosp

NTSTATUS
SeCreateAccessStateEx(
   __in_opt PETHREAD Thread,
   __in PEPROCESS Process,
   __out PACCESS_STATE AccessState,
   __out PAUX_ACCESS_DATA AuxData,
   __in ACCESS_MASK DesiredAccess,
   __in_opt PGENERIC_MAPPING GenericMapping
   );

NTSTATUS
SeUpdateClientSecurity(
    IN PETHREAD ClientThread,
    IN OUT PSECURITY_CLIENT_CONTEXT ClientContext,
    OUT PBOOLEAN ChangesMade,
    OUT PBOOLEAN NewToken
    );

BOOLEAN
SeRmInitPhase1(
    VOID
    );

NTSTATUS
SeInitializeProcessAuditName (
    __in __typefix(PFILE_OBJECT) PVOID FileObject,
    __in BOOLEAN bIgnoreAuditPolicy,
    __deref_out POBJECT_NAME_INFORMATION *pAuditName
    );

NTSTATUS
SeLocateProcessImageName(
    __in PEPROCESS Process,
    __deref_out PUNICODE_STRING *pImageFileName
    );

VOID
SeAuditSystemTimeChange(
    __in LARGE_INTEGER OldTime,
    __in LARGE_INTEGER NewTime
    );


// begin_ntifs begin_ntosp

NTKERNELAPI
NTSTATUS
SeQuerySecurityDescriptorInfo (
    __in PSECURITY_INFORMATION SecurityInformation,
    __out_bcount(*Length) PSECURITY_DESCRIPTOR SecurityDescriptor,
    __inout PULONG Length,
    __deref_inout PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor
    );

NTKERNELAPI
NTSTATUS
SeSetSecurityDescriptorInfo (
    __in_opt PVOID Object,
    __in PSECURITY_INFORMATION SecurityInformation,
    __in PSECURITY_DESCRIPTOR ModificationDescriptor,
    __inout PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    __in POOL_TYPE PoolType,
    __in PGENERIC_MAPPING GenericMapping
    );

NTKERNELAPI
NTSTATUS
SeSetSecurityDescriptorInfoEx (
    __in_opt PVOID Object,
    __in PSECURITY_INFORMATION SecurityInformation,
    __in PSECURITY_DESCRIPTOR ModificationDescriptor,
    __inout PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    __in ULONG AutoInheritFlags,
    __in POOL_TYPE PoolType,
    __in PGENERIC_MAPPING GenericMapping
    );

NTKERNELAPI
NTSTATUS
SeAppendPrivileges(
    __inout PACCESS_STATE AccessState,
    __in PPRIVILEGE_SET Privileges
    );

// end_ntifs end_ntosp

NTSTATUS
SeComputeQuotaInformationSize(
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __out PULONG Size
    );

VOID
SePrivilegedServiceAuditAlarm (
    __in_opt PUNICODE_STRING ServiceName,
    __in PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    __in PPRIVILEGE_SET Privileges,
    __in BOOLEAN AccessGranted
    );

NTKERNELAPI                                                     // ntddk ntifs ntosp
BOOLEAN                                                         // ntddk ntifs ntosp
SeSinglePrivilegeCheck(                                         // ntddk ntifs ntosp
    __in LUID PrivilegeValue,                                        // ntddk ntifs ntosp
    __in KPROCESSOR_MODE PreviousMode                                // ntddk ntifs ntosp
    );                                                          // ntddk ntifs ntosp

BOOLEAN
SeCheckAuditPrivilege (
   __in PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
   __in KPROCESSOR_MODE PreviousMode
   );

NTSTATUS
SeAssignWorldSecurityDescriptor(
    __inout_bcount_part( *Length, *Length) PSECURITY_DESCRIPTOR SecurityDescriptor,
    __inout PULONG Length,
    __in PSECURITY_INFORMATION SecurityInformation
    );

BOOLEAN
SeFastTraverseCheck(
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in_opt PACCESS_STATE AccessState,
    __in ACCESS_MASK TraverseAccess,
    __in KPROCESSOR_MODE AccessMode
    );

// begin_ntifs

NTKERNELAPI
BOOLEAN
SeAuditingFileEvents(
    __in BOOLEAN AccessGranted,
    __in PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTKERNELAPI
BOOLEAN
SeAuditingFileEventsWithContext(
    __in BOOLEAN AccessGranted,
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in_opt PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext
    );

NTKERNELAPI
BOOLEAN
SeAuditingHardLinkEvents(
    __in BOOLEAN AccessGranted,
    __in PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTKERNELAPI
BOOLEAN
SeAuditingHardLinkEventsWithContext(
    __in BOOLEAN AccessGranted,
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in_opt PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext
    );

NTKERNELAPI
BOOLEAN
SeAuditingFileOrGlobalEvents(
    __in BOOLEAN AccessGranted,
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext
    );

NTKERNELAPI
BOOLEAN
FASTCALL
SeDetailedAuditingWithToken(
    __in_opt PACCESS_TOKEN AccessToken
    );

// end_ntifs

VOID
SeAuditProcessCreation(
    __in PEPROCESS Process
    );

VOID
SeAuditProcessExit(
    __in PEPROCESS Process
    );

NTKERNELAPI                                                     // ntifs
VOID                                                            // ntifs
SeAuditHardLinkCreation(                                        // ntifs
    __in PUNICODE_STRING FileName,                                // ntifs
    __in PUNICODE_STRING LinkName,                                // ntifs
    __in BOOLEAN bSuccess                                         // ntifs
    );                                                          // ntifs

VOID
SeAuditLPCInvalidUse(
    __in PUNICODE_STRING LpcCallName,
    __in PUNICODE_STRING LpcServerPort
    );

VOID
SeAuditHandleDuplication(
    __in PVOID SourceHandle,
    __in PVOID NewHandle,
    __in PEPROCESS SourceProcess,
    __in PEPROCESS TargetProcess
    );

VOID
SeMaximumAuditMask(
    IN PACL Sacl,
    IN ACCESS_MASK GrantedAccess,
    IN PACCESS_TOKEN Token,
    OUT PACCESS_MASK pAuditMask
    );

VOID
SeOperationAuditAlarm (
    __in_opt PUNICODE_STRING CapturedSubsystemName,
    __in PVOID HandleId,
    __in PUNICODE_STRING ObjectTypeName,
    __in ACCESS_MASK AuditMask,
    __in_opt PSID UserSid
    );

VOID
SeAddSaclToProcess(
    IN PEPROCESS Process,
    IN PACCESS_TOKEN Token,
    IN PVOID Reserved
    );

// begin_ntifs

VOID
SeSetAccessStateGenericMapping (
    __inout PACCESS_STATE AccessState,
    __in PGENERIC_MAPPING GenericMapping
    );

// end_ntifs

// begin_ntifs

NTKERNELAPI
NTSTATUS
SeRegisterLogonSessionTerminatedRoutine(
    IN PSE_LOGON_SESSION_TERMINATED_ROUTINE CallbackRoutine
    );

NTKERNELAPI
NTSTATUS
SeUnregisterLogonSessionTerminatedRoutine(
    IN PSE_LOGON_SESSION_TERMINATED_ROUTINE CallbackRoutine
    );

NTKERNELAPI
NTSTATUS
SeMarkLogonSessionForTerminationNotification(
    IN PLUID LogonId
    );

// begin_ntosp

NTKERNELAPI
NTSTATUS
SeQueryInformationToken (
    __in PACCESS_TOKEN Token,
    __in TOKEN_INFORMATION_CLASS TokenInformationClass,
    __deref_out PVOID *TokenInformation
    );

// end_ntifs end_ntosp

NTSTATUS
SeIsChildToken(
    __in HANDLE Token,
    __out PBOOLEAN IsChild
    );

NTSTATUS
SeIsChildTokenByPointer(
    __in PACCESS_TOKEN Token,
    __out PBOOLEAN IsChild
    );

NTSTATUS
SeIsSiblingToken(
    __in HANDLE Token,
    __out PBOOLEAN IsSibling
    );

NTSTATUS
SeIsSiblingTokenByPointer(
    __in PACCESS_TOKEN Token,
    __out PBOOLEAN IsSibling
    );

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
    );

////////////////////////////////////////////////////////////////////////
//                                                                    //
//           Global, READ ONLY, Security variables                    //
//                                                                    //
////////////////////////////////////////////////////////////////////////


// **************************************************************
//
//              C A V E A T      P R O G R A M M E R
//
//
//  If you wish to include this file in an NT driver and use SeExports structure
//  defined above, you need to call:
//
//
//      SeEnableAccessToExports()
//
//  exactly once during initialization.
//
//              C A V E A T      P R O G R A M M E R
//
// **************************************************************

// begin_ntifs begin_ntosp
//
//  Grants access to SeExports structure
//

extern NTKERNELAPI PSE_EXPORTS SeExports;

// end_ntifs end_ntosp

//
// Value used to represent the authentication ID of system processes
//

extern const LUID SeSystemAuthenticationId;
extern const LUID SeAnonymousAuthenticationId;

extern const TOKEN_SOURCE SeSystemTokenSource;

//
// Universal well known SIDs
//

extern PSID  SeNullSid;
extern PSID  SeWorldSid;
extern PSID  SeLocalSid;
extern PSID  SeCreatorOwnerSid;
extern PSID  SeCreatorGroupSid;
extern PSID  SeCreatorOwnerServerSid;
extern PSID  SeCreatorGroupServerSid;
extern PSID  SePrincipalSelfSid;


//
// Sids defined by NT
//

extern PSID SeNtAuthoritySid;

extern PSID SeDialupSid;
extern PSID SeNetworkSid;
extern PSID SeBatchSid;
extern PSID SeInteractiveSid;
extern PSID SeLocalSystemSid;
extern PSID SeAuthenticatedUsersSid;
extern PSID SeAliasAdminsSid;
extern PSID SeRestrictedSid;
extern PSID SeAnonymousLogonSid;
extern PSID SeAliasUsersSid;
extern PSID SeAliasGuestsSid;
extern PSID SeAliasPowerUsersSid;
extern PSID SeAliasAccountOpsSid;
extern PSID SeAliasSystemOpsSid;
extern PSID SeAliasPrintOpsSid;
extern PSID SeAliasBackupOpsSid;

//
// Well known tokens
//

extern PACCESS_TOKEN SeAnonymousLogonToken;
extern PACCESS_TOKEN SeAnonymousLogonTokenNoEveryone;

//
// System default DACLs & Security Descriptors
//

extern PSECURITY_DESCRIPTOR SePublicDefaultSd;
extern PSECURITY_DESCRIPTOR SePublicDefaultUnrestrictedSd;
extern PSECURITY_DESCRIPTOR SePublicOpenSd;
extern PSECURITY_DESCRIPTOR SePublicOpenUnrestrictedSd;
extern PSECURITY_DESCRIPTOR SeSystemDefaultSd;
extern PSECURITY_DESCRIPTOR SeLocalServicePublicSd;

extern PACL SePublicDefaultDacl;
extern PACL SePublicDefaultUnrestrictedDacl;
extern PACL SePublicOpenDacl;
extern PACL SePublicOpenUnrestrictedDacl;
extern PACL SeSystemDefaultDacl;
extern PACL SeUnrestrictedDacl;
extern PACL SeLocalServicePublicDacl;

//
//  Well known privilege values
//


extern LUID SeCreateTokenPrivilege;
extern LUID SeAssignPrimaryTokenPrivilege;
extern LUID SeLockMemoryPrivilege;
extern LUID SeIncreaseQuotaPrivilege;
extern LUID SeUnsolicitedInputPrivilege;
extern LUID SeTcbPrivilege;
extern LUID SeSecurityPrivilege;
extern LUID SeTakeOwnershipPrivilege;
extern LUID SeLoadDriverPrivilege;
extern LUID SeCreatePagefilePrivilege;
extern LUID SeIncreaseBasePriorityPrivilege;
extern LUID SeSystemProfilePrivilege;
extern LUID SeSystemtimePrivilege;
extern LUID SeProfileSingleProcessPrivilege;
extern LUID SeCreatePermanentPrivilege;
extern LUID SeBackupPrivilege;
extern LUID SeRestorePrivilege;
extern LUID SeShutdownPrivilege;
extern LUID SeDebugPrivilege;
extern LUID SeAuditPrivilege;
extern LUID SeSystemEnvironmentPrivilege;
extern LUID SeChangeNotifyPrivilege;
extern LUID SeRemoteShutdownPrivilege;
extern LUID SeUndockPrivilege;
extern LUID SeSyncAgentPrivilege;
extern LUID SeEnableDelegationPrivilege;
extern LUID SeManageVolumePrivilege;
extern LUID SeImpersonatePrivilege;
extern LUID SeCreateGlobalPrivilege;

//
// Auditing information array
//

extern SE_AUDITING_STATE SeAuditingState[];

extern const UNICODE_STRING SeSubsystemName;


#endif // _SE_
