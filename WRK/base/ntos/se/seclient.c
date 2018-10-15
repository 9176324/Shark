/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    seclient.c

Abstract:

   This module implements routines providing client impersonation to
   communication session layers (such as LPC Ports).

        WARNING: The following notes apply to the use of these services:

        (1)  No synchronization of operations to a security context block are
             performed by these services.  The caller of these services must
             ensure that use of an individual security context block is
             serialized to prevent simultaneous, incompatible updates.

        (2)  Any or all of these services may create, open, or operate on a
             token object.  This may result in a mutex being acquired at
             MUTEXT_LEVEL_SE_TOKEN level.  The caller must ensure that no
             mutexes are held at levels that conflict with this action.

--*/

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
NTSTATUS
SepCreateClientSecurity(
    IN PACCESS_TOKEN Token,
    IN PSECURITY_QUALITY_OF_SERVICE ClientSecurityQos,
    IN BOOLEAN ServerIsRemote,
    TOKEN_TYPE TokenType,
    BOOLEAN ThreadEffectiveOnly,
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    OUT PSECURITY_CLIENT_CONTEXT ClientContext
    );
#pragma alloc_text(PAGE,SepCreateClientSecurity)
#pragma alloc_text(PAGE,SeCreateClientSecurity)
#pragma alloc_text(PAGE,SeUpdateClientSecurity)
#pragma alloc_text(PAGE,SeImpersonateClient)
#pragma alloc_text(PAGE,SeImpersonateClientEx)
#pragma alloc_text(PAGE,SeCreateClientSecurityFromSubjectContext)
#endif


////////////////////////////////////////////////////////////////////////
//                                                                    //
//           Routines                                                 //
//                                                                    //
////////////////////////////////////////////////////////////////////////


NTSTATUS
SepCreateClientSecurity(
    IN PACCESS_TOKEN Token,
    IN PSECURITY_QUALITY_OF_SERVICE ClientSecurityQos,
    IN BOOLEAN ServerIsRemote,
    TOKEN_TYPE TokenType,
    BOOLEAN ThreadEffectiveOnly,
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    OUT PSECURITY_CLIENT_CONTEXT ClientContext
    )

/*++

Routine Description

    This routine creates a context block to represent the passed token.  The token
    is expected to be properly referenced when passed to the function.  If the call 
    is unsuccessful, the caller is responsible to dereference the token.  Also, if the 
    call succeeds but the caller requested SECURITY_STATIC_TRACKING, then we have duplicated
    the token and the caller is again responsible to dereference the passed token.
    
Arguments

    Token - The effective token for which the context is constructed.
    
    ClientSecurityQos - Points to the security quality of service
        parameters specified by the client for this communication
        session.

    ServerIsRemote - Provides an indication as to whether the session
        this context block is being used for is an inter-system
        session or intra-system session.  This is reconciled with the
        impersonation level of the client thread's token (in case the
        client has a client of his own that didn't specify delegation).
    
   TokenType - specifies the type of the passed token.
   
   ThreadEffectiveOnly - if the token is an impersonation token, then this 
        is the thread's ImpersonationInfo->EffectiveOnly field value.
    
    ImpersonationLevel - the impersonation level of the token.
    
    ClientContext - Points to the client security context block to be
        initialized.
        
Return Value

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    PACCESS_TOKEN DuplicateToken;

    PAGED_CODE();

    if ( !VALID_IMPERSONATION_LEVEL(ClientSecurityQos->ImpersonationLevel) ) {

        return STATUS_INVALID_PARAMETER;
    }

    //
    // Make sure the client is not trying to abuse use of a
    // client of its own by attempting an invalid impersonation.
    // Also set the ClientContext->DirectAccessEffectiveOnly flag
    // appropriately if the impersonation is legitimate.  The
    // DirectAccessEffectiveOnly flag value will end up being ignored
    // if STATIC mode is requested, but this is the most efficient
    // place to calculate it, and we are optimizing for DYNAMIC mode.
    //

    if (TokenType == TokenImpersonation) {

        if ( ClientSecurityQos->ImpersonationLevel > ImpersonationLevel) {

            return STATUS_BAD_IMPERSONATION_LEVEL;
        }

        if ( SepBadImpersonationLevel(ImpersonationLevel,ServerIsRemote)) {

            return STATUS_BAD_IMPERSONATION_LEVEL;

        } else {

            //
            // TokenType is TokenImpersonation and the impersonation is legit.
            // Set the DirectAccessEffectiveOnly flag to be the minimum of
            // the current thread value and the caller specified value.
            //

            ClientContext->DirectAccessEffectiveOnly =
                ( (ThreadEffectiveOnly || (ClientSecurityQos->EffectiveOnly)) ?
                  TRUE : FALSE );
        }

    } else {

        //
        // TokenType is TokenPrimary.  In this case, the client specified
        // EffectiveOnly value is always used.
        //

        ClientContext->DirectAccessEffectiveOnly =
            ClientSecurityQos->EffectiveOnly;
    }

    //
    // Copy the token if necessary (i.e., static tracking requested)
    //

    if (ClientSecurityQos->ContextTrackingMode == SECURITY_STATIC_TRACKING) {

        ClientContext->DirectlyAccessClientToken = FALSE;

        Status = SeCopyClientToken(
                     Token,
                     ClientSecurityQos->ImpersonationLevel,
                     KernelMode,
                     &DuplicateToken
                     );

        Token = DuplicateToken;

        //
        // If there was an error, we're done.
        //
        if (!NT_SUCCESS(Status)) {
            return Status;
        }

    } else {

        ClientContext->DirectlyAccessClientToken = TRUE;

        if (ServerIsRemote) {
            //
            // Get a copy of the client token's control information
            // so that we can tell if it changes in the future.
            //

            SeGetTokenControlInformation( Token,
                                          &ClientContext->ClientTokenControl
                                          );

        }

    }

    ClientContext->SecurityQos.Length =
        (ULONG)sizeof(SECURITY_QUALITY_OF_SERVICE);

    ClientContext->SecurityQos.ImpersonationLevel =
        ClientSecurityQos->ImpersonationLevel;

    ClientContext->SecurityQos.ContextTrackingMode =
        ClientSecurityQos->ContextTrackingMode;

    ClientContext->SecurityQos.EffectiveOnly =
        ClientSecurityQos->EffectiveOnly;

    ClientContext->ServerIsRemote = ServerIsRemote;

    ClientContext->ClientToken = Token;

    return STATUS_SUCCESS;
}

NTSTATUS
SeCreateClientSecurity (
    __in PETHREAD ClientThread,
    __in PSECURITY_QUALITY_OF_SERVICE ClientSecurityQos,
    __in BOOLEAN ServerIsRemote,
    __out PSECURITY_CLIENT_CONTEXT ClientContext
    )

/*++

Routine Description:

    This service initializes a context block to represent a client's
    security context.  This may simply result in a reference to the
    client's token, or may cause the client's token to be duplicated,
    depending upon the security quality of service information specified.

                               NOTE

        The code in this routine is optimized for DYNAMIC context
        tracking.  This is only mode in which direct access to a
        caller's token is allowed, and the mode expected to be used
        most often.  STATIC context tracking always requires the
        caller's token to be copied.


Arguments:

    ClientThread - Points to the client's thread.  This is used to
        locate the client's security context (token).

    ClientSecurityQos - Points to the security quality of service
        parameters specified by the client for this communication
        session.

    ServerIsRemote - Provides an indication as to whether the session
        this context block is being used for is an inter-system
        session or intra-system session.  This is reconciled with the
        impersonation level of the client thread's token (in case the
        client has a client of his own that didn't specify delegation).

    ClientContext - Points to the client security context block to be
        initialized.


Return Value:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_BAD_IMPERSONATION_LEVEL - The client is currently
        impersonating either an Anonymous or Identification level
        token, which can not be passed on for use by another server.
        This status may also be returned if the security context
        block is for an inter-system communication session and the
        client thread is impersonating a client of its own using
        other than delegation impersonation level.


--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    PACCESS_TOKEN Token;
    TOKEN_TYPE TokenType;
    BOOLEAN ThreadEffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    PACCESS_TOKEN DuplicateToken;

    PAGED_CODE();

    //
    // Gain access to the client thread's effective token
    //

    Token = PsReferenceEffectiveToken(
                ClientThread,
                &TokenType,
                &ThreadEffectiveOnly,
                &ImpersonationLevel
                );


    Status = SepCreateClientSecurity(
                Token,
                ClientSecurityQos,
                ServerIsRemote,
                TokenType,
                ThreadEffectiveOnly,
                ImpersonationLevel,
                ClientContext );

    //
    // If failed, or if a token was copied internally, then deref our token.
    //

    if ((!NT_SUCCESS( Status )) || (ClientSecurityQos->ContextTrackingMode == SECURITY_STATIC_TRACKING)) {

        ObDereferenceObject( Token );
    }

    return Status;
}




VOID
SeImpersonateClient(
    __in PSECURITY_CLIENT_CONTEXT ClientContext,
    __in_opt PETHREAD ServerThread
    )
/*++

Routine Description:

    This service is used to cause the calling thread to impersonate a
    client.  The client security context in ClientContext is assumed to
    be up to date.


Arguments:

    ClientContext - Points to client security context block.

    ServerThread - (Optional) Specifies the thread which is to be made to
        impersonate the client.  If not specified, the calling thread is
        used.


Return Value:

    None.


--*/


{

    PAGED_CODE();

#if DBG
    DbgPrint("SE:  Obsolete call:  SeImpersonateClient\n");
#endif

    (VOID) SeImpersonateClientEx( ClientContext, ServerThread );
}


NTSTATUS
SeImpersonateClientEx(
    __in PSECURITY_CLIENT_CONTEXT ClientContext,
    __in_opt PETHREAD ServerThread
    )
/*++

Routine Description:

    This service is used to cause the calling thread to impersonate a
    client.  The client security context in ClientContext is assumed to
    be up to date.


Arguments:

    ClientContext - Points to client security context block.

    ServerThread - (Optional) Specifies the thread which is to be made to
        impersonate the client.  If not specified, the calling thread is
        used.


Return Value:

    None.


--*/


{

    BOOLEAN EffectiveValueToUse;
    PETHREAD Thread;
    NTSTATUS Status ;

    PAGED_CODE();

    if (ClientContext->DirectlyAccessClientToken) {
        EffectiveValueToUse = ClientContext->DirectAccessEffectiveOnly;
    } else {
        EffectiveValueToUse = ClientContext->SecurityQos.EffectiveOnly;
    }



    //
    // if a ServerThread wasn't specified, then default to the current
    // thread.
    //

    if (!ARGUMENT_PRESENT(ServerThread)) {
        Thread = PsGetCurrentThread();
    } else {
        Thread = ServerThread;
    }



    //
    // Assign the context to the calling thread
    //

    Status = PsImpersonateClient( Thread,
                         ClientContext->ClientToken,
                         TRUE,
                         EffectiveValueToUse,
                         ClientContext->SecurityQos.ImpersonationLevel
                         );

    return Status ;

}


NTSTATUS
SeCreateClientSecurityFromSubjectContext (
    __in PSECURITY_SUBJECT_CONTEXT SubjectContext,
    __in PSECURITY_QUALITY_OF_SERVICE ClientSecurityQos,
    __in BOOLEAN ServerIsRemote,
    __out PSECURITY_CLIENT_CONTEXT ClientContext
    )                              
/*++

Routine Description:

    This service initializes a context block to represent a client's
    security context.  This may simply result in a reference to the
    client's token, or may cause the client's token to be duplicated,
    depending upon the security quality of service information specified.

                               NOTE

        The code in this routine is optimized for DYNAMIC context
        tracking.  This is only mode in which direct access to a
        caller's token is allowed, and the mode expected to be used
        most often.  STATIC context tracking always requires the
        caller's token to be copied.


Arguments:

    SubjectContext - Points to the SubjectContext that should serve
        as the basis for this client context.

    ClientSecurityQos - Points to the security quality of service
        parameters specified by the client for this communication
        session.

    ServerIsRemote - Provides an indication as to whether the session
        this context block is being used for is an inter-system
        session or intra-system session.  This is reconciled with the
        impersonation level of the client thread's token (in case the
        client has a client of his own that didn't specify delegation).

    ClientContext - Points to the client security context block to be
        initialized.


Return Value:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_BAD_IMPERSONATION_LEVEL - The client is currently
        impersonating either an Anonymous or Identification level
        token, which can not be passed on for use by another server.
        This status may also be returned if the security context
        block is for an inter-system communication session and the
        client thread is impersonating a client of its own using
        other than delegation impersonation level.


--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    PACCESS_TOKEN Token;
    TOKEN_TYPE Type;
    BOOLEAN ThreadEffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    PACCESS_TOKEN DuplicateToken;

    PAGED_CODE();

    Token = SeQuerySubjectContextToken(
                SubjectContext
                );

    ObReferenceObject( Token );

    if ( SubjectContext->ClientToken )
    {
        Type = TokenImpersonation ;
    }
    else 
    {
        Type = TokenPrimary ;
    }

    Status = SepCreateClientSecurity(
                Token,
                ClientSecurityQos,
                ServerIsRemote,
                Type,
                FALSE,
                SubjectContext->ImpersonationLevel,
                ClientContext
                );

    //
    // If failed, or if a token was copied internally, then deref our token.
    //

    if ((!NT_SUCCESS( Status )) || (ClientSecurityQos->ContextTrackingMode == SECURITY_STATIC_TRACKING)) {
        ObDereferenceObject( Token );
    }
    
    return Status ;
}

