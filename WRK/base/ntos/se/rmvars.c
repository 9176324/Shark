/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    rmvars.c

Abstract:

   This module contains the variables used to implement the run-time
   reference monitor database.

--*/

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,SepRmDbInitialization)
#endif


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Read/Write Reference Monitor Variables                                    //
//                                                                            //
//  Access to these variables is protected by the SepRmDbLock.                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


//
//  Resource Locks  - These locks protect access to the modifiable fields of
//                    the reference monitor database. There is one lock for
//                    a set of hash buckets.
//

ERESOURCE SepRmDbLock[SEP_LOGON_TRACK_LOCK_ARRAY_SIZE] = {0};
                     
//
//  Resource Lock  - This mutex protects access to the file systems notify
//                   routines list
//

FAST_MUTEX SepRmNotifyMutex = {0};


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Read Only Reference Monitor Variables                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


//
// The process within which the RM --> LSA command LPC port was established.
// All calls from the reference monitor to the LSA must be made in this
// process in order for the handle to be valid.

PEPROCESS SepRmLsaCallProcess = NULL;

//
// State of the reference monitor
//

SEP_RM_STATE SepRmState = {0};


//
// The following array is used as a hash bucket for tracking logon sessions.
// The sequence number of logon LUIDs is ANDed with 0x0F and then used as an
// index into this array.  This entry in the array serves as a listhead of
// logon session reference count records.
//

PSEP_LOGON_SESSION_REFERENCES *SepLogonSessions = NULL;



////////////////////////////////////////////////////////////////////////
//                                                                    //
//           Variable Initialization Routines                         //
//                                                                    //
////////////////////////////////////////////////////////////////////////

BOOLEAN
SepRmDbInitialization(
    VOID
    )
/*++

Routine Description:

    This function initializes the reference monitor in-memory database.

Arguments:

    None.

Return Value:

    TRUE if database successfully initialized.
    FALSE if not successfully initialized.

--*/
{
    NTSTATUS Status;
    ULONG i;

    //
    // Create the reference monitor database lock
    //
    // Use SepRmAcquireDbReadLock()
    //     SepRmAcquireDbWriteLock()
    //     SepRmReleaseDbReadLock()
    //     SepRmReleaseDbWriteLock()
    //
    // to gain access to the reference monitor database.
    //

    for (i=0;i<SEP_LOGON_TRACK_LOCK_ARRAY_SIZE;i++) {
        ExInitializeResourceLite(&(SepRmDbLock[ i ]));
    }
    
    //
    // Create the file systems notify list mutex
    //
    //
    // Use SepRmAcquireNotifyLock()
    //     SepRmReleaseNotifyLock()
    //
    //
    // to safely access the file systems notify list.
    //

    ExInitializeFastMutex(&SepRmNotifyMutex);

    //
    // Initialize the Logon Session tracking array.
    //

    SepLogonSessions = ExAllocatePoolWithTag( PagedPool,
                                              sizeof( PSEP_LOGON_SESSION_REFERENCES ) * SEP_LOGON_TRACK_ARRAY_SIZE,
                                              'SLeS'
                                              );

    if (SepLogonSessions == NULL) {
        return( FALSE );
    }

    for (i=0;i<SEP_LOGON_TRACK_ARRAY_SIZE;i++) {

        SepLogonSessions[ i ] = NULL;
    }

    //
    // Now add in a record representing the system logon session.
    //

    Status = SepCreateLogonSessionTrack( (PLUID)&SeSystemAuthenticationId );
    ASSERT( NT_SUCCESS(Status) );
    if ( !NT_SUCCESS(Status)) {
        return FALSE;
    }

    //
    // Add one for the null session logon session
    //

    Status = SepCreateLogonSessionTrack( (PLUID)&SeAnonymousAuthenticationId );
    ASSERT( NT_SUCCESS(Status) );
    if ( !NT_SUCCESS(Status)) {
        return FALSE;
    }

    //
    // The correct RM state will be set when the local security policy
    // information is retrieved (by the LSA) and subsequently passed to
    // the reference monitor later on in initialization.  For now, initialize
    // the state to something that will work for the remainder of
    // system initialization.
    //

    SepRmState.AuditingEnabled = 0;    // auditing state disabled.
    SepRmState.OperationalMode = LSA_MODE_PASSWORD_PROTECTED;

    return TRUE;
}

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

