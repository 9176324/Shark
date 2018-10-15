/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    perfdata.c

Abstract:

    This module contains the global read/write data for the perf subsystem

--*/

#include "perfp.h"

PERFINFO_GROUPMASK PerfGlobalGroupMask;
PERFINFO_GROUPMASK *PPerfGlobalGroupMask;
const PERFINFO_HOOK_HANDLE PerfNullHookHandle = { NULL, NULL };

//
// Profiling
//

KPROFILE PerfInfoProfileObject;
KPROFILE_SOURCE PerfInfoProfileSourceActive = ProfileMaximum;   // Set to invalid source
KPROFILE_SOURCE PerfInfoProfileSourceRequested = ProfileTime;
KPROFILE_SOURCE PerfInfoProfileInterval = 10000;    // 1ms in 100ns ticks
BOOLEAN PerfInfoSampledProfileCaching;
LONG PerfInfoSampledProfileFlushInProgress;
PERFINFO_SAMPLED_PROFILE_CACHE PerfProfileCache;

