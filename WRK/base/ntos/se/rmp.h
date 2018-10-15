/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    rmp.h

Abstract:

    Security Reference Monitor Private Data Types, Functions and Defines

--*/

#ifndef _RMP_H_
#define _RMP_H_

#include <nt.h>
#include <ntlsa.h>
#include "sep.h"


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Reference Monitor Private defines                                        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


//
// Used to define the bounds of the array used to track logon session
// reference counts.
//

#define SEP_LOGON_TRACK_INDEX_MASK (0x0000000FL)
#define SEP_LOGON_TRACK_ARRAY_SIZE (0x00000010L)

//
// Used to define the bounds of the locks array used to reference the logon
// sessions. We use a hardcoded index of ZERO for when LogonId is not 
// available.
// The number of locks was chosen to be 4 based on performance tests on
// uniproc and 8-proc machines.
//

#define SEP_LOGON_TRACK_LOCK_INDEX_MASK (0x00000003L)
#define SEP_LOGON_TRACK_LOCK_ARRAY_SIZE (0x00000004L)



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Reference Monitor Private Macros                                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
//  acquire exclusive access to a token
//

#define SepRmAcquireDbReadLock(i)  KeEnterCriticalRegion();         \
                                   ExAcquireResourceSharedLite(&(SepRmDbLock[(i) & SEP_LOGON_TRACK_LOCK_INDEX_MASK]), TRUE)

#define SepRmAcquireDbWriteLock(i) KeEnterCriticalRegion();         \
                                   ExAcquireResourceExclusiveLite(&(SepRmDbLock[(i) & SEP_LOGON_TRACK_LOCK_INDEX_MASK]), TRUE)

#define SepRmReleaseDbReadLock(i)  ExReleaseResourceLite(&(SepRmDbLock[(i) & SEP_LOGON_TRACK_LOCK_INDEX_MASK])); \
                                   KeLeaveCriticalRegion()

#define SepRmReleaseDbWriteLock(i) ExReleaseResourceLite(&(SepRmDbLock[(i) & SEP_LOGON_TRACK_LOCK_INDEX_MASK])); \
                                   KeLeaveCriticalRegion()

#define SepRmAcquireNotifyLock()  KeEnterCriticalRegion();         \
                                  ExAcquireFastMutexUnsafe(&SepRmNotifyMutex)

#define SepRmReleaseNotifyLock()  ExReleaseFastMutexUnsafe(&SepRmNotifyMutex); \
                                  KeLeaveCriticalRegion()


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Reference Monitor Private Data Types                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define SEP_RM_LSA_SHARED_MEMORY_SIZE ((ULONG) PAGE_SIZE)

//
// Reference Monitor Private Global State Data Structure
//

typedef struct _SEP_RM_STATE {

    HANDLE LsaInitEventHandle;
    HANDLE LsaCommandPortHandle;
    HANDLE SepRmThreadHandle;
    HANDLE RmCommandPortHandle;
    HANDLE RmCommandServerPortHandle;
    ULONG AuditingEnabled;
    LSA_OPERATIONAL_MODE OperationalMode;
    HANDLE LsaCommandPortSectionHandle;
    LARGE_INTEGER LsaCommandPortSectionSize;
    PVOID LsaViewPortMemory;
    PVOID RmViewPortMemory;
    LONG LsaCommandPortMemoryDelta;
//    BOOLEAN LsaCommandPortResourceInitialized;
    BOOLEAN LsaCommandPortActive;
//    ERESOURCE LsaCommandPortResource;

} SEP_RM_STATE, *PSEP_RM_STATE;

//
// Reference Monitor Command Port Connection Info
//

typedef struct _SEP_RM_CONNECT_INFO {
    ULONG ConnectInfo;
} SEP_RM_CONNECT_INFO;

typedef struct SEP_RM_CONNECT_INFO *PSEP_RM_CONNECT_INFO;


//
// Reference Monitor Command Table Entry Format
//

#define SEP_RM_COMMAND_MAX 4

typedef VOID (*SEP_RM_COMMAND_WORKER)( PRM_COMMAND_MESSAGE, PRM_REPLY_MESSAGE );

typedef struct _SEP_LOGON_SESSION_TOKEN {
    LIST_ENTRY ListEntry;
    PTOKEN Token;
} SEP_LOGON_SESSION_TOKEN, *PSEP_LOGON_SESSION_TOKEN;


#define SEP_TERMINATION_NOTIFY  0x1

//
// File systems interested in being notified when a logon session is being
// terminated register a callback routine. The following data structure
// describes the callback routines.
//
// The global list of callback routines is pointed to by SeFileSystemNotifyRoutines.
// This list is protected by the RM database lock.
//

typedef struct _SEP_LOGON_SESSION_TERMINATED_NOTIFICATION {
    struct _SEP_LOGON_SESSION_TERMINATED_NOTIFICATION *Next;
    PSE_LOGON_SESSION_TERMINATED_ROUTINE CallbackRoutine;
} SEP_LOGON_SESSION_TERMINATED_NOTIFICATION, *PSEP_LOGON_SESSION_TERMINATED_NOTIFICATION;

extern SEP_LOGON_SESSION_TERMINATED_NOTIFICATION
SeFileSystemNotifyRoutinesHead;


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Reference Monitor Private Function Prototypes                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

BOOLEAN
SepRmDbInitialization(
    VOID
    );

VOID
SepRmCommandServerThread(
    IN PVOID StartContext
    );

BOOLEAN SepRmCommandServerThreadInit(
    );

VOID
SepRmComponentTestCommandWrkr(
    IN PRM_COMMAND_MESSAGE CommandMessage,
    OUT PRM_REPLY_MESSAGE ReplyMessage
    );

VOID
SepRmSetAuditEventWrkr(
    IN PRM_COMMAND_MESSAGE CommandMessage,
    OUT PRM_REPLY_MESSAGE ReplyMessage
    );

VOID
SepRmSendCommandToLsaWrkr(
    IN PRM_COMMAND_MESSAGE CommandMessage,
    OUT PRM_REPLY_MESSAGE ReplyMessage
    );

VOID
SepRmCreateLogonSessionWrkr(
    IN PRM_COMMAND_MESSAGE CommandMessage,
    OUT PRM_REPLY_MESSAGE ReplyMessage
    );

VOID
SepRmDeleteLogonSessionWrkr(
    IN PRM_COMMAND_MESSAGE CommandMessage,
    OUT PRM_REPLY_MESSAGE ReplyMessage
    ) ;


NTSTATUS
SepCreateLogonSessionTrack(
    IN PLUID LogonId
    );

NTSTATUS
SepDeleteLogonSessionTrack(
    IN PLUID LogonId
    );



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Reference Monitor Private Variables Declarations                          //
// These variables are defined in rmvars.c                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

extern PEPROCESS SepRmLsaCallProcess;
extern SEP_RM_STATE SepRmState;
extern ERESOURCE SepRmDbLock[];
extern FAST_MUTEX SepRmNotifyMutex;
extern PSEP_LOGON_SESSION_REFERENCES *SepLogonSessions;

#endif // _RMP_H_

