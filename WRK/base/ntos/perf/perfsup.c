/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    perfsup.c

Abstract:

    This module contains support routines for performance traces.

--*/

#include "perfp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PerfInfoProfileInit)
#pragma alloc_text(PAGE, PerfInfoProfileUninit)
#pragma alloc_text(PAGE, PerfInfoStartLog)
#pragma alloc_text(PAGE, PerfInfoStopLog)
#endif //ALLOC_PRAGMA

extern NTSTATUS IoPerfInit();
extern NTSTATUS IoPerfReset();


VOID
PerfInfoProfileInit(
    )
/*++

Routine description:

    Starts the sampled profile and initializes the cache

Arguments:
    None

Return Value:
    None

--*/
{
#if !defined(NT_UP)
    PerfInfoSampledProfileCaching = FALSE;
#else
    PerfInfoSampledProfileCaching = TRUE;
#endif // !defined(NT_UP)
    PerfInfoSampledProfileFlushInProgress = 0;
    PerfProfileCache.Entries = 0;

    PerfInfoProfileSourceActive = PerfInfoProfileSourceRequested;

    KeSetIntervalProfile(PerfInfoProfileInterval, PerfInfoProfileSourceActive);
    KeInitializeProfile(&PerfInfoProfileObject,
                        NULL,
                        NULL,
                        0,
                        0,
                        0,
                        PerfInfoProfileSourceActive,
                        0
                        );
    KeStartProfile(&PerfInfoProfileObject, NULL);
}


VOID
PerfInfoProfileUninit(
    )
/*++

Routine description:

    Stops the sampled profile

Arguments:
    None

Return Value:
    None

--*/
{
    PerfInfoProfileSourceActive = ProfileMaximum;   // Invalid value stops us from
                                                    // collecting more samples
    KeStopProfile(&PerfInfoProfileObject);
    PerfInfoFlushProfileCache();
}


NTSTATUS
PerfInfoStartLog (
    PERFINFO_GROUPMASK *PGroupMask,
    PERFINFO_START_LOG_LOCATION StartLogLocation
    )

/*++

Routine Description:

    This routine is called by WMI as part of kernel logger initialization.

Arguments:

    GroupMask - Masks for what to log.  This pointer point to an allocated
                area in WMI LoggerContext.
    StartLogLocation - Indication of whether we're starting logging
                at boot time or while the system is running.  If
                we're starting at boot time, we don't need to snapshot
                the open files because there aren't any and we would
                crash if we tried to find the list.

Return Value:

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN ProfileInitialized = FALSE;
    BOOLEAN ContextSwapStarted = FALSE;
    BOOLEAN IoPerfInitialized = FALSE;

    PERFINFO_CLEAR_GROUPMASK(&PerfGlobalGroupMask);

    //
    // Enable logging.
    //

    PPerfGlobalGroupMask = &PerfGlobalGroupMask;
    PerfSetLogging(PPerfGlobalGroupMask);

    if (PerfIsGroupOnInGroupMask(PERF_MEMORY, PGroupMask) ||
        PerfIsGroupOnInGroupMask(PERF_FILENAME, PGroupMask) ||
        PerfIsGroupOnInGroupMask(PERF_DRIVERS, PGroupMask)) {
            PERFINFO_OR_GROUP_WITH_GROUPMASK(PERF_FILENAME_ALL, PGroupMask);
    }


    if (StartLogLocation == PERFINFO_START_LOG_FROM_GLOBAL_LOGGER) {
        //
        // From the wmi global logger, need to do Rundown in kernel mode
        //
        if (PerfIsGroupOnInGroupMask(PERF_PROC_THREAD, PGroupMask)) {
            Status = PerfInfoProcessRunDown();
            if (!NT_SUCCESS(Status)) {
                goto Finish;
            }
        }

        if (PerfIsGroupOnInGroupMask(PERF_PROC_THREAD, PGroupMask)) {
            Status = PerfInfoSysModuleRunDown();
            if (!NT_SUCCESS(Status)) {
                goto Finish;
            }
        }
    }

    //
    // File Name Rundown code
    //
    if ((StartLogLocation != PERFINFO_START_LOG_AT_BOOT) && 
        PerfIsGroupOnInGroupMask(PERF_FILENAME_ALL, PGroupMask)) {
        PERFINFO_OR_GROUP_WITH_GROUPMASK(PERF_FILENAME_ALL, PPerfGlobalGroupMask);
        Status = PerfInfoFileNameRunDown();
        if (!NT_SUCCESS(Status)) {
            goto Finish;
        }
    }

    //
    // Initialize Perf Driver hooks
    //
    if (PerfIsGroupOnInGroupMask(PERF_DRIVERS, PGroupMask)) {
        Status = IoPerfInit();
        if (NT_SUCCESS(Status)) {
            IoPerfInitialized = TRUE;
        } else {
            goto Finish;
        }
    }

    //
    // Enable context swap tracing
    //
    if ( PerfIsGroupOnInGroupMask(PERF_CONTEXT_SWITCH, PGroupMask) ) {
        WmiStartContextSwapTrace();
        ContextSwapStarted = TRUE;
    }

    //
    // Sampled Profile
    //
    if (PerfIsGroupOnInGroupMask(PERF_PROFILE, PGroupMask)) {
        if ((KeGetPreviousMode() == KernelMode)  ||
            (SeSinglePrivilegeCheck(SeSystemProfilePrivilege, UserMode))) {
            PerfInfoProfileInit();
            ProfileInitialized = TRUE;
        } else {
            Status = STATUS_NO_SUCH_PRIVILEGE;
            goto Finish;
        }
    }

    //
    // See if we need to empty the working set to start
    //
    if (PerfIsGroupOnInGroupMask(PERF_FOOTPRINT, PGroupMask) ||
        PerfIsGroupOnInGroupMask(PERF_BIGFOOT, PGroupMask)) {
        MmEmptyAllWorkingSets ();
    }

Finish:

    if (!NT_SUCCESS(Status)) {
        //
        // Failed to turn on trace, clean up now.
        //

        if (ContextSwapStarted) {
            WmiStopContextSwapTrace();
        }

        if (ProfileInitialized) {
            PerfInfoProfileUninit();
        }

        if (IoPerfInitialized) {
            IoPerfReset();
        }

        //
        // Disable logging.
        //

        PPerfGlobalGroupMask = NULL;
        PerfSetLogging(NULL);

        PERFINFO_CLEAR_GROUPMASK(&PerfGlobalGroupMask);
    } else {
        *PPerfGlobalGroupMask = *PGroupMask;
    }

    return Status;
}


NTSTATUS
PerfInfoStopLog (
    )

/*++

Routine Description: 

    This routine turn off the PerfInfo trace hooks.

Arguments:

    None.

Return Value:

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN DisableContextSwaps=FALSE;

    if (PPerfGlobalGroupMask == NULL) {
        return Status;
    }

    if (PERFINFO_IS_GROUP_ON(PERF_MEMORY)) {
        MmIdentifyPhysicalMemory();
    }

    if (PERFINFO_IS_GROUP_ON(PERF_PROFILE)) {
        PerfInfoProfileUninit();
    }

    if (PERFINFO_IS_GROUP_ON(PERF_DRIVERS)) {
        IoPerfReset();
    }

    if ( PERFINFO_IS_GROUP_ON(PERF_CONTEXT_SWITCH) ) {
        DisableContextSwaps = TRUE;
    }

    //
    // Reset the PPerfGlobalGroupMask.
    //

    PERFINFO_CLEAR_GROUPMASK(PPerfGlobalGroupMask);

    //
    // Disable logging.
    //

    PPerfGlobalGroupMask = NULL;
    PerfSetLogging(NULL);

    //
    // Disable context swap tracing.
    // IMPORTANT: This must be done AFTER the global flag is set to NULL!!!
    //
    if( DisableContextSwaps ) {

        WmiStopContextSwapTrace();
    }

    return (Status);

}

