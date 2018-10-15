/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    rmaudit.c

Abstract:

   This module contains the Reference Monitor Auditing Command Workers.
   These workers call functions in the Auditing sub-component to do the real
   work.

--*/

#include "pch.h"

#pragma hdrstop

VOID
SepRmSetAuditLogWrkr(
    IN PRM_COMMAND_MESSAGE CommandMessage,
    OUT PRM_REPLY_MESSAGE ReplyMessage
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SepRmSetAuditEventWrkr)
#endif



VOID
SepRmSetAuditEventWrkr(
    IN PRM_COMMAND_MESSAGE CommandMessage,
    OUT PRM_REPLY_MESSAGE ReplyMessage
    )

/*++

Routine Description:

    This function carries out the Reference Monitor Set Audit Event
    Command.  This command enables or disables auditing and optionally
    sets the auditing events.


Arguments:

    CommandMessage - Pointer to structure containing RM command message
        information consisting of an LPC PORT_MESSAGE structure followed
        by the command number (RmSetAuditStateCommand) and a single command
        parameter in structure form.

    ReplyMessage - Pointer to structure containing RM reply message
        information consisting of an LPC PORT_MESSAGE structure followed
        by the command ReturnedStatus field in which a status code from the
        command will be returned.

Return Value:

    VOID

--*/

{

    PPOLICY_AUDIT_EVENT_OPTIONS EventAuditingOptions;
    POLICY_AUDIT_EVENT_TYPE EventType;

    PAGED_CODE();

    SepAdtInitializeBounds();

    ReplyMessage->ReturnedStatus = STATUS_SUCCESS;

    //
    // Strict check that command is correct one for this worker.
    //

    ASSERT( CommandMessage->CommandNumber == RmAuditSetCommand );

    //
    // Extract the AuditingMode flag and put it in the right place.
    //

    SepAdtAuditingEnabled = (((PLSARM_POLICY_AUDIT_EVENTS_INFO) CommandMessage->CommandParams)->
                                AuditingMode);

    //
    // For each element in the passed array, process changes to audit
    // nothing, and then success or failure flags.
    //

    EventAuditingOptions = ((PLSARM_POLICY_AUDIT_EVENTS_INFO) CommandMessage->CommandParams)->
                           EventAuditingOptions;


    for ( EventType=AuditEventMinType;
          EventType <= AuditEventMaxType;
          EventType++ ) {

        SeAuditingState[EventType].AuditOnSuccess = FALSE;
        SeAuditingState[EventType].AuditOnFailure = FALSE;

        if ( EventAuditingOptions[EventType] & POLICY_AUDIT_EVENT_SUCCESS ) {

            SeAuditingState[EventType].AuditOnSuccess = TRUE;
        }

        if ( EventAuditingOptions[EventType] & POLICY_AUDIT_EVENT_FAILURE ) {

            SeAuditingState[EventType].AuditOnFailure = TRUE;
        }
    }

    return;
}

