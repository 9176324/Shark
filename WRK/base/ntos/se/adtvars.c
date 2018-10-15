/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    adtvars.c

Abstract:

    Auditing - Private Variables

--*/

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

//
// Auditing State.  This contains the Auditing Mode and the array of
// Event Auditing Options
//

//POLICY_AUDIT_EVENTS_INFO SepAdtState;

//
// Audit Log Information
//

POLICY_AUDIT_LOG_INFO SepAdtLogInformation = {0};

//
// High and low water marks to control the length of the audit queue
// These are initialized to their default values in case we can't get
// them out of the registry.
//

ULONG SepAdtMaxListLength = 0x3000;
ULONG SepAdtMinListLength = 0x2000;

ULONG SepAdtCurrentListLength = 0;

//
// Number of events discarded
//

ULONG SepAdtCountEventsDiscarded = 0;

//
// Flag used to indicate that we are currently discarding audit events
//

BOOLEAN SepAdtDiscardingAudits = FALSE;

//
// see note in adtp.h regarding SEP_AUDIT_OPTIONS
//

SEP_AUDIT_OPTIONS SepAuditOptions = { 0 };

//
// Used to indicate that LSA process has died
// (see adtlog.c and rmmain.c)
//

PKEVENT SepAdtLsaDeadEvent = NULL;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

