/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    timer.c

Abstract:

   This module implements the executive timer object. Functions are provided
   to create, open, cancel, set, and query timer objects.

--*/

#include "exp.h"

//
// Executive timer object structure definition.
//

typedef struct _ETIMER {
    KTIMER KeTimer;
    KAPC TimerApc;
    KDPC TimerDpc;
    LIST_ENTRY ActiveTimerListEntry;
    KSPIN_LOCK Lock;
    LONG Period;
    BOOLEAN ApcAssociated;
    BOOLEAN WakeTimer;
    LIST_ENTRY WakeTimerListEntry;
} ETIMER, *PETIMER;

//
// List of all timers which are set to wake.
//

KSPIN_LOCK ExpWakeTimerListLock;
LIST_ENTRY ExpWakeTimerList;

//
// Address of timer object type descriptor.
//

POBJECT_TYPE ExTimerObjectType;

//
// Structure that describes the mapping of generic access rights to object
// specific access rights for timer objects.
//

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif

const GENERIC_MAPPING ExpTimerMapping = {
    STANDARD_RIGHTS_READ | TIMER_QUERY_STATE,
    STANDARD_RIGHTS_WRITE | TIMER_MODIFY_STATE,
    STANDARD_RIGHTS_EXECUTE | SYNCHRONIZE,
    TIMER_ALL_ACCESS
};

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

#pragma alloc_text(INIT, ExpTimerInitialization)
#pragma alloc_text(PAGE, NtCreateTimer)
#pragma alloc_text(PAGE, NtOpenTimer)
#pragma alloc_text(PAGE, NtQueryTimer)
#pragma alloc_text(PAGELK, ExGetNextWakeTime)

VOID
ExpTimerApcRoutine (
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    )

/*++

Routine Description:

    This function is the special APC routine that is called to remove
    a timer from the current thread's active timer list.

Arguments:

    Apc - Supplies a pointer to the APC object used to invoke this routine.

    NormalRoutine - Supplies a pointer to a pointer to the normal routine
        function that was specified when the APC was initialized.

    NormalContext - Supplies a pointer to a pointer to an arbitrary data
        structure that was specified when the APC was initialized.

    SystemArgument1, SystemArgument2 - Supplies a set of two pointers to
        two arguments that contain untyped data.

Return Value:

    None.

--*/

{

    ULONG DereferenceCount;
    PETHREAD ExThread;
    PETIMER ExTimer;
    KIRQL OldIrql1;

    UNREFERENCED_PARAMETER(NormalContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    //
    // Get address of executive timer object and the current thread object.
    //

    ExThread = PsGetCurrentThread();
    ExTimer = CONTAINING_RECORD(Apc, ETIMER, TimerApc);

    //
    // If the timer is still in the current thread's active timer list, then
    // remove it if it is not a periodic timer and set APC associated FALSE.
    // It is possible for the timer not to be in the current thread's active
    // timer list since the APC could have been delivered, and then another
    // thread could have set the timer again with another APC. This would
    // have caused the timer to be removed from the current thread's active
    // timer list.
    //
    // N. B. The spin locks for the timer and the active timer list must be
    //  acquired in the order: 1) timer lock, 2) thread list lock.
    //

    DereferenceCount = 1;
    ExAcquireSpinLock(&ExTimer->Lock, &OldIrql1);
    ExAcquireSpinLockAtDpcLevel(&ExThread->ActiveTimerListLock);
    if ((ExTimer->ApcAssociated) && (&ExThread->Tcb == ExTimer->TimerApc.Thread)) {
        if (ExTimer->Period == 0) {
            RemoveEntryList(&ExTimer->ActiveTimerListEntry);
            ExTimer->ApcAssociated = FALSE;
            DereferenceCount += 1;
        }

    } else {
        *NormalRoutine = (PKNORMAL_ROUTINE)NULL;
    }

    ExReleaseSpinLockFromDpcLevel(&ExThread->ActiveTimerListLock);
    ExReleaseSpinLock(&ExTimer->Lock, OldIrql1);
    ObDereferenceObjectEx(ExTimer, DereferenceCount);
    return;
}

VOID
ExpTimerDpcRoutine (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

/*++

Routine Description:

    This function is the DPC routine that is called when a timer expires that
    has an associated APC routine. Its function is to insert the associated
    APC into the target thread's APC queue.

Arguments:

    Dpc - Supplies a pointer to a control object of type DPC.

    DeferredContext - Supplies a pointer to the executive timer that contains
        the DPC that caused this routine to be executed.

    SystemArgument1, SystemArgument2 - Supplies values that are not used by
        this routine.

Return Value:

    None.

--*/

{

    PETIMER ExTimer;
    BOOLEAN Inserted;
    PKTIMER KeTimer;

    //
    // Reference the timer so the APC is free to manipulate it.
    //
    // If the object is being deleted the delete routine will flush all
    // pending DPC's so the object won't be completely deleted until after
    // the following code completes.
    //

    ExTimer = (PETIMER)DeferredContext;

#if defined(_AMD64_)

    try {

#else

        UNREFERENCED_PARAMETER(Dpc);

#endif

        if (ObReferenceObjectSafe(ExTimer) == FALSE) {
            return;
        }

#if defined(_AMD64_)

        } except(KiKernelDpcFilter(Dpc, GetExceptionInformation())) {
            return;
        }

#endif

    //
    // If there is still an APC associated with the timer, then insert the APC
    // in target thread's APC queue. It is possible that the timer does not
    // have an associated APC. This can happen when the timer is set to expire
    // by a thread running on another processor just after the DPC has been
    // removed from the DPC queue, but before it has acquired the timer related
    // spin lock.
    //

    KeTimer = &ExTimer->KeTimer;
    Inserted = FALSE;
    ExAcquireSpinLockAtDpcLevel(&ExTimer->Lock);
    if (ExTimer->ApcAssociated) {
        Inserted = KeInsertQueueApc(&ExTimer->TimerApc,
                                    SystemArgument1,
                                    SystemArgument2,
                                    TIMER_APC_INCREMENT);
    }

    ExReleaseSpinLockFromDpcLevel(&ExTimer->Lock);

    //
    // If the timer APC wasn't inserted in the APC list, then release the
    // reference associated with it.
    //

    if (Inserted == FALSE) {
        ObDereferenceObject(ExTimer);
    }

    return;
}

VOID
ExpDeleteTimer (
    IN PVOID Object
    )

/*++

Routine Description:

    This function is the delete routine for timer objects. Its function is
    to cancel the timer and free the spin lock associated with a timer.

Arguments:

    Object - Supplies a pointer to an executive timer object.

Return Value:

    None.

--*/

{

    PETIMER ExTimer;
    KIRQL OldIrql;

    //
    // Remove from wake list.
    //

    ExTimer = (PETIMER)Object;
    if (ExTimer->WakeTimerListEntry.Flink) {
        ExAcquireSpinLock(&ExpWakeTimerListLock, &OldIrql);
        if (ExTimer->WakeTimerListEntry.Flink) {
            RemoveEntryList(&ExTimer->WakeTimerListEntry);
            ExTimer->WakeTimerListEntry.Flink = NULL;
        }

        ExReleaseSpinLock(&ExpWakeTimerListLock, OldIrql);
    }

    //
    // Cancel the timer and free the spin lock associated with the timer.
    //

    KeCancelTimer(&ExTimer->KeTimer);

    //
    // Make sure there are no running DPC's associated with this timer
    // before it can get deleted completely.
    //

    KeFlushQueuedDpcs();
    return;
}

BOOLEAN
ExpTimerInitialization (
    VOID
    )

/*++

Routine Description:

    This function creates the timer object type descriptor at system
    initialization and stores the address of the object type descriptor
    in local static storage.

Arguments:

    None.

Return Value:

    A value of TRUE is returned if the timer object type descriptor is
    successfully initialized. Otherwise a value of FALSE is returned.

--*/

{

    OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
    NTSTATUS Status;
    UNICODE_STRING TypeName;

    //
    // Initialize the wake timer listhead and spinlock.
    //

    KeInitializeSpinLock(&ExpWakeTimerListLock);
    InitializeListHead(&ExpWakeTimerList);

    //
    // Initialize string descriptor.
    //

    RtlInitUnicodeString(&TypeName, L"Timer");

    //
    // Create timer object type descriptor.
    //

    RtlZeroMemory(&ObjectTypeInitializer, sizeof(ObjectTypeInitializer));
    ObjectTypeInitializer.Length = sizeof(ObjectTypeInitializer);
    ObjectTypeInitializer.InvalidAttributes = OBJ_OPENLINK;
    ObjectTypeInitializer.GenericMapping = ExpTimerMapping;
    ObjectTypeInitializer.PoolType = NonPagedPool;
    ObjectTypeInitializer.DefaultNonPagedPoolCharge = sizeof(ETIMER);
    ObjectTypeInitializer.ValidAccessMask = TIMER_ALL_ACCESS;
    ObjectTypeInitializer.DeleteProcedure = ExpDeleteTimer;
    Status = ObCreateObjectType(&TypeName,
                                &ObjectTypeInitializer,
                                (PSECURITY_DESCRIPTOR)NULL,
                                &ExTimerObjectType);

    //
    // If the time object type descriptor was successfully created, then
    // return a value of TRUE. Otherwise return a value of FALSE.
    //

    return (BOOLEAN)(NT_SUCCESS(Status));
}

VOID
ExTimerRundown (
    VOID
    )

/*++

Routine Description:

    This function is called when a thread is about to be terminated to
    process the active timer list. It is assumed that APC's have been
    disabled for the subject thread, thus this code cannot be interrupted
    to execute an APC for the current thread.

Arguments:

    None.

Return Value:

    None.

--*/

{

    LONG DereferenceCount;
    PETHREAD ExThread;
    PETIMER ExTimer;
    PLIST_ENTRY NextEntry;
    KIRQL OldIrql1;

    //
    // Process each entry in the active timer list.
    //

    ExThread = PsGetCurrentThread();
    ExAcquireSpinLock(&ExThread->ActiveTimerListLock, &OldIrql1);
    NextEntry = ExThread->ActiveTimerListHead.Flink;
    while (NextEntry != &ExThread->ActiveTimerListHead) {
        ExTimer = CONTAINING_RECORD(NextEntry, ETIMER, ActiveTimerListEntry);

        //
        // Increment the reference count on the object so that it cannot be
        // deleted, and then drop the active timer list lock.
        //
        // N. B. The object reference cannot fail and will acquire no mutexes.
        //

        ObReferenceObject(ExTimer);
        ExReleaseSpinLock(&ExThread->ActiveTimerListLock, OldIrql1);
        DereferenceCount = 1;

        //
        // Acquire the timer spin lock and reacquire the active time list spin
        // lock. If the timer is still in the current thread's active timer
        // list, then cancel the timer, remove the timer's DPC from the DPC
        // queue, remove the timer's APC from the APC queue, remove the timer
        // from the thread's active timer list, and set the associate APC
        // flag FALSE.
        //
        // N. B. The spin locks for the timer and the active timer list must be
        //       acquired in the order: 1) timer lock, 2) thread list lock.
        //

        ExAcquireSpinLock(&ExTimer->Lock, &OldIrql1);
        ExAcquireSpinLockAtDpcLevel(&ExThread->ActiveTimerListLock);
        if ((ExTimer->ApcAssociated) && (&ExThread->Tcb == ExTimer->TimerApc.Thread)) {
            RemoveEntryList(&ExTimer->ActiveTimerListEntry);
            ExTimer->ApcAssociated = FALSE;
            KeCancelTimer(&ExTimer->KeTimer);
            KeRemoveQueueDpc(&ExTimer->TimerDpc);
            if (KeRemoveQueueApc(&ExTimer->TimerApc)) {
                DereferenceCount += 1;
            }

            DereferenceCount += 1;
        }

        ExReleaseSpinLockFromDpcLevel(&ExThread->ActiveTimerListLock);
        ExReleaseSpinLock(&ExTimer->Lock, OldIrql1);
        ObDereferenceObjectEx(ExTimer, DereferenceCount);

        //
        // Raise IRQL to DISPATCH_LEVEL and reacquire active timer list
        // spin lock.
        //

        ExAcquireSpinLock(&ExThread->ActiveTimerListLock, &OldIrql1);
        NextEntry = ExThread->ActiveTimerListHead.Flink;
    }

    ExReleaseSpinLock(&ExThread->ActiveTimerListLock, OldIrql1);
    return;
}

NTSTATUS
NtCreateTimer (
    __out PHANDLE TimerHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in TIMER_TYPE TimerType
    )

/*++

Routine Description:

    This function creates an timer object and opens a handle to the object with
    the specified desired access.

Arguments:

    TimerHandle - Supplies a pointer to a variable that will receive the
        timer object handle.

    DesiredAccess - Supplies the desired types of access for the timer object.

    ObjectAttributes - Supplies a pointer to an object attributes structure.

    TimerType - Supplies the type of the timer which can be synchronization
        or notification. 

Return Value:

    NTSTATUS.

--*/

{

    PETIMER ExTimer;
    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    //
    // Check argument validity.
    //

    if ((TimerType != NotificationTimer) &&
        (TimerType != SynchronizationTimer)) {

        return STATUS_INVALID_PARAMETER_4;
    }

    //
    // Get previous processor mode and probe output handle address if
    // necessary.
    //

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {
            ProbeForWriteHandle(TimerHandle);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

    //
    // Allocate timer object.
    //

    Status = ObCreateObject(PreviousMode,
                            ExTimerObjectType,
                            ObjectAttributes,
                            PreviousMode,
                            NULL,
                            sizeof(ETIMER),
                            0,
                            0,
                            &ExTimer);

    //
    // If the timer object was successfully allocated, then initialize the
    // timer object and attempt to insert the time object in the current
    // process' handle table.
    //

    if (NT_SUCCESS(Status)) {
        KeInitializeDpc(&ExTimer->TimerDpc,
                        ExpTimerDpcRoutine,
                        (PVOID)ExTimer);

        KeInitializeTimerEx(&ExTimer->KeTimer, TimerType);
        KeInitializeSpinLock(&ExTimer->Lock);
        ExTimer->ApcAssociated = FALSE;
        ExTimer->WakeTimer = FALSE;
        ExTimer->WakeTimerListEntry.Flink = NULL;
        Status = ObInsertObject((PVOID)ExTimer,
                                NULL,
                                DesiredAccess,
                                0,
                                NULL,
                                &Handle);

        //
        // If the timer object was successfully inserted in the current
        // process' handle table, then attempt to write the timer object
        // handle value. If the write attempt fails, then do not report
        // an error. When the caller attempts to access the handle value,
        // an access violation will occur.
        //

        if (NT_SUCCESS(Status)) {
            if (PreviousMode != KernelMode) {
                try {
                    *TimerHandle = Handle;

                } except(EXCEPTION_EXECUTE_HANDLER) {
                    NOTHING;
                }

            } else {
                *TimerHandle = Handle;
            }
        }
    }

    //
    // Return service status.
    //

    return Status;
}

NTSTATUS
NtOpenTimer (
    __out PHANDLE TimerHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    )

/*++

Routine Description:

    This function opens a handle to an timer object with the specified
    desired access.

Arguments:

    TimerHandle - Supplies a pointer to a variable that will receive the
        timer object handle.

    DesiredAccess - Supplies the desired types of access for the timer object.

    ObjectAttributes - Supplies a pointer to an object attributes structure.

Return Value:

    NTSTATUS.

--*/

{

    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    //
    // Get previous processor mode and probe output handle address if
    // necessary.
    //

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {
            ProbeForWriteHandle(TimerHandle);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

    //
    // Open handle to the timer object with the specified desired access.
    //

    Status = ObOpenObjectByName(ObjectAttributes,
                                ExTimerObjectType,
                                PreviousMode,
                                NULL,
                                DesiredAccess,
                                NULL,
                                &Handle);

    //
    // If the open was successful, then attempt to write the timer object
    // handle value. If the write attempt fails, then do not report an
    // error. When the caller attempts to access the handle value, an
    // access violation will occur.
    //

    if (NT_SUCCESS(Status)) {
        if (PreviousMode != KernelMode) {
            try {
                *TimerHandle = Handle;

            } except(EXCEPTION_EXECUTE_HANDLER) {
                NOTHING;
            }

        } else {
            *TimerHandle = Handle;
        }
    }

    //
    // Return service status.
    //

    return Status;
}

NTSTATUS
NtCancelTimer (
    __in HANDLE TimerHandle,
    __out_opt PBOOLEAN CurrentState
    )

/*++

Routine Description:

    This function cancels a timer object.

Arguments:

    TimerHandle - Supplies a handle to an timer object.

    CurrentState - Supplies an optional pointer to a variable that will
        receive the current state of the timer object.

Return Value:

    NTSTATUS.

--*/

{

    ULONG DereferenceCount;
    PETHREAD ExThread;
    PETIMER ExTimer;
    KIRQL OldIrql1;
    KPROCESSOR_MODE PreviousMode;
    BOOLEAN State;
    NTSTATUS Status;

    //
    // Get previous processor mode and probe current state address if
    // necessary.
    //

    PreviousMode = KeGetPreviousMode();
    if (ARGUMENT_PRESENT(CurrentState) && (PreviousMode != KernelMode)) {
        try {
            ProbeForWriteBoolean(CurrentState);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

    //
    // Reference timer object by handle.
    //

    Status = ObReferenceObjectByHandle(TimerHandle,
                                       TIMER_MODIFY_STATE,
                                       ExTimerObjectType,
                                       PreviousMode,
                                       &ExTimer,
                                       NULL);

    //
    // If the reference was successful, then cancel the timer object,
    // dereference the timer object, and write the current state value
    // if specified. If the write attempt fails, then do not report an
    // error. When the caller attempts to access the current state value,
    // an access violation will occur.
    //

    if (NT_SUCCESS(Status)) {
        DereferenceCount = 1;
        ExAcquireSpinLock(&ExTimer->Lock, &OldIrql1);
        if (ExTimer->ApcAssociated) {
            ExThread = CONTAINING_RECORD(ExTimer->TimerApc.Thread, ETHREAD, Tcb);
            ExAcquireSpinLockAtDpcLevel(&ExThread->ActiveTimerListLock);
            RemoveEntryList(&ExTimer->ActiveTimerListEntry);
            ExTimer->ApcAssociated = FALSE;
            ExReleaseSpinLockFromDpcLevel(&ExThread->ActiveTimerListLock);
            KeCancelTimer(&ExTimer->KeTimer);
            KeRemoveQueueDpc(&ExTimer->TimerDpc);
            if (KeRemoveQueueApc(&ExTimer->TimerApc)) {
                DereferenceCount += 1;
            }

            DereferenceCount += 1;

        } else {
            KeCancelTimer(&ExTimer->KeTimer);
        }

        if (ExTimer->WakeTimerListEntry.Flink) {
            ExAcquireSpinLockAtDpcLevel(&ExpWakeTimerListLock);

            //
            // Check again since wait next time might have removed the timer.
            //

            if (ExTimer->WakeTimerListEntry.Flink) {
                RemoveEntryList(&ExTimer->WakeTimerListEntry);
                ExTimer->WakeTimerListEntry.Flink = NULL;
            }

            ExReleaseSpinLockFromDpcLevel(&ExpWakeTimerListLock);
        }

        ExReleaseSpinLock(&ExTimer->Lock, OldIrql1);

        //
        // Read current state of timer, dereference timer object, and set
        // current state.
        //

        State = KeReadStateTimer(&ExTimer->KeTimer);
        ObDereferenceObjectEx(ExTimer, DereferenceCount);
        if (ARGUMENT_PRESENT(CurrentState)) {
            if (PreviousMode != KernelMode) {
                try {
                    *CurrentState = State;

                } except(EXCEPTION_EXECUTE_HANDLER) {
                    NOTHING;
                }

            } else {
                *CurrentState = State;
            }
        }
    }

    //
    // Return service status.
    //

    return Status;
}

NTSTATUS
NtQueryTimer (
    __in HANDLE TimerHandle,
    __in TIMER_INFORMATION_CLASS TimerInformationClass,
    __out_bcount(TimerInformationLength) PVOID TimerInformation,
    __in ULONG TimerInformationLength,
    __out_opt PULONG ReturnLength
    )

/*++

Routine Description:

    This function queries the state of an timer object and returns the
    requested information in the specified record structure.

Arguments:

    TimerHandle - Supplies a handle to an timer object.

    TimerInformationClass - Supplies the class of information being requested.

    TimerInformation - Supplies a pointer to a record that is to receive the
        requested information.

    TimerInformationLength - Supplies the length of the record that is to
        receive the requested information.

    ReturnLength - Supplies an optional pointer to a variable that is to
        receive the actual length of information that is returned.

Return Value:

    NTSTATUS.

--*/

{

    PETIMER ExTimer;
    PTIMER_BASIC_INFORMATION Information;
    PKTIMER KeTimer;
    KPROCESSOR_MODE PreviousMode;
    BOOLEAN State;
    NTSTATUS Status;
    LARGE_INTEGER TimeToGo;

    //
    // Check argument validity.
    //

    if (TimerInformationClass != TimerBasicInformation) {
        return STATUS_INVALID_INFO_CLASS;
    }

    if (TimerInformationLength != sizeof(TIMER_BASIC_INFORMATION)) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    //
    // Get previous processor mode and probe output arguments if necessary.
    //

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {
            ProbeForWriteSmallStructure(TimerInformation,
                                        sizeof(TIMER_BASIC_INFORMATION),
                                        sizeof(ULONG));

            if (ARGUMENT_PRESENT(ReturnLength)) {
                ProbeForWriteUlong(ReturnLength);
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

    //
    // Reference timer object by handle.
    //

    Status = ObReferenceObjectByHandle(TimerHandle,
                                       TIMER_QUERY_STATE,
                                       ExTimerObjectType,
                                       PreviousMode,
                                       &ExTimer,
                                       NULL);

    //
    // If the reference was successful, then read the current state,
    // compute the time remaining, dereference the timer object, fill in
    // the information structure, and return the length of the information
    // structure if specified. If the write of the time information or the
    // return length fails, then do not report an error. When the caller
    // accesses the information structure or the length, an violation will
    // occur.
    //

    if (NT_SUCCESS(Status)) {
        Information = TimerInformation;
        KeTimer = &ExTimer->KeTimer;
        State = KeReadStateTimer(KeTimer);
        KiQueryInterruptTime(&TimeToGo);
        TimeToGo.QuadPart = KeTimer->DueTime.QuadPart - TimeToGo.QuadPart;
        ObDereferenceObject(ExTimer);
        if (PreviousMode != KernelMode) {
            try {
                Information->TimerState = State;
                Information->RemainingTime = TimeToGo;
                if (ARGUMENT_PRESENT(ReturnLength)) {
                    *ReturnLength = sizeof(TIMER_BASIC_INFORMATION);
                }

            } except(EXCEPTION_EXECUTE_HANDLER) {
                NOTHING;
            }

        } else {
            Information->TimerState = State;
            Information->RemainingTime = TimeToGo;
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = sizeof(TIMER_BASIC_INFORMATION);
            }
        }
    }

    //
    // Return service status.
    //

    return Status;
}

NTSTATUS
NtSetTimer (
    __in HANDLE TimerHandle,
    __in PLARGE_INTEGER DueTime,
    __in_opt PTIMER_APC_ROUTINE TimerApcRoutine,
    __in_opt PVOID TimerContext,
    __in BOOLEAN WakeTimer,
    __in_opt LONG Period,
    __out_opt PBOOLEAN PreviousState
    )

/*++

Routine Description:

    This function sets an timer object to a Not-Signaled state and sets the timer
    to expire at the specified time.

Arguments:

    TimerHandle - Supplies a handle to an timer object.

    DueTime - Supplies a pointer to absolute of relative time at which the
        timer is to expire.

    TimerApcRoutine - Supplies an optional pointer to a function which is to
        be executed when the timer expires. If this parameter is not specified,
        then the TimerContext parameter is ignored.

    TimerContext - Supplies an optional pointer to an arbitrary data structure
        that will be passed to the function specified by the TimerApcRoutine
        parameter. This parameter is ignored if the TimerApcRoutine parameter
        is not specified.

    WakeTimer - Supplies a boolean value that specifies whether the timer
        wakes computer operation if sleeping.

    Period - Supplies an optional repetitive period for the timer.

    PreviousState - Supplies an optional pointer to a variable that will
        receive the previous state of the timer object.

Return Value:

    NTSTATUS.

--*/

{

    ULONG DereferenceCount;
    PETHREAD ExThread;
    PETIMER ExTimer;
    LARGE_INTEGER ExpirationTime;
    KIRQL OldIrql1;
    KPROCESSOR_MODE PreviousMode;
    BOOLEAN State;
    NTSTATUS Status;

    //
    // Check argument validity.
    //

    if (Period < 0) {
        return STATUS_INVALID_PARAMETER_6;
    }

    //
    // Get previous processor mode and probe previous state address if
    // necessary.
    //

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {
            if (ARGUMENT_PRESENT(PreviousState)) {
                ProbeForWriteBoolean(PreviousState);
            }

            ProbeForReadSmallStructure(DueTime, sizeof(LARGE_INTEGER), sizeof(ULONG));
            ExpirationTime = *DueTime;

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }

    } else {
        ExpirationTime = *DueTime;
    }

    //
    // Reference timer object by handle.
    //

    Status = ObReferenceObjectByHandle(TimerHandle,
                                       TIMER_MODIFY_STATE,
                                       ExTimerObjectType,
                                       PreviousMode,
                                       &ExTimer,
                                       NULL);

    //
    // If the wake timer flag is set, return the appropriate informational
    // success status code.
    //

    if (NT_SUCCESS(Status) && WakeTimer && !PoWakeTimerSupported()) {
        Status = STATUS_TIMER_RESUME_IGNORED;
    }

    //
    // If the reference was successful, then cancel the timer object, set
    // the timer object, dereference time object, and write the previous
    // state value if specified. If the write of the previous state value
    // fails, then do not report an error. When the caller attempts to
    // access the previous state value, an access violation will occur.
    //

    if (NT_SUCCESS(Status)) {
        DereferenceCount = 1;
        ExAcquireSpinLock(&ExTimer->Lock, &OldIrql1);
        if (ExTimer->ApcAssociated) {
            ExThread = CONTAINING_RECORD(ExTimer->TimerApc.Thread, ETHREAD, Tcb);
            ExAcquireSpinLockAtDpcLevel(&ExThread->ActiveTimerListLock);
            RemoveEntryList(&ExTimer->ActiveTimerListEntry);
            ExTimer->ApcAssociated = FALSE;
            ExReleaseSpinLockFromDpcLevel(&ExThread->ActiveTimerListLock);
            KeCancelTimer(&ExTimer->KeTimer);
            KeRemoveQueueDpc(&ExTimer->TimerDpc);
            if (KeRemoveQueueApc(&ExTimer->TimerApc)) {
                DereferenceCount += 1;
            }

            DereferenceCount += 1;

        } else {
            KeCancelTimer(&ExTimer->KeTimer);
        }

        //
        // Read the current state of the timer.
        //

        State = KeReadStateTimer(&ExTimer->KeTimer);

        //
        // If this is a wake timer ensure it's on the wake timer list.
        //

        ExTimer->WakeTimer = WakeTimer;
        ExAcquireSpinLockAtDpcLevel(&ExpWakeTimerListLock);
        if (WakeTimer) {
            if (!ExTimer->WakeTimerListEntry.Flink) {
                InsertTailList(&ExpWakeTimerList, &ExTimer->WakeTimerListEntry);
            }

        } else {
            if (ExTimer->WakeTimerListEntry.Flink) {
                RemoveEntryList(&ExTimer->WakeTimerListEntry);
                ExTimer->WakeTimerListEntry.Flink = NULL;
            }
        }

        ExReleaseSpinLockFromDpcLevel(&ExpWakeTimerListLock);

        //
        // If an APC routine is specified, then initialize the APC, acquire the
        // thread's active time list lock, insert the timer in the thread's
        // active timer list, set the timer with an associated DPC, and set the
        // associated APC flag TRUE. Otherwise set the timer without an
        // associated DPC, and set the associated APC flag FALSE.
        //

        ExTimer->Period = Period;
        if (ARGUMENT_PRESENT(TimerApcRoutine)) {
            ExThread = PsGetCurrentThread();
            KeInitializeApc(&ExTimer->TimerApc,
                            &ExThread->Tcb,
                            CurrentApcEnvironment,
                            ExpTimerApcRoutine,
                            (PKRUNDOWN_ROUTINE)NULL,
                            (PKNORMAL_ROUTINE)TimerApcRoutine,
                            PreviousMode,
                            TimerContext);

            ExAcquireSpinLockAtDpcLevel(&ExThread->ActiveTimerListLock);
            InsertTailList(&ExThread->ActiveTimerListHead,
                           &ExTimer->ActiveTimerListEntry);

            ExTimer->ApcAssociated = TRUE;
            ExReleaseSpinLockFromDpcLevel(&ExThread->ActiveTimerListLock);
            KeSetTimerEx(&ExTimer->KeTimer,
                         ExpirationTime,
                         Period,
                         &ExTimer->TimerDpc);

            DereferenceCount -= 1;

        } else {
            KeSetTimerEx(&ExTimer->KeTimer,
                         ExpirationTime,
                         Period,
                         NULL);
        }

        ExReleaseSpinLock(&ExTimer->Lock, OldIrql1);

        //
        // Dereference the object as appropriate.
        //

        if (DereferenceCount > 0) {
            ObDereferenceObjectEx(ExTimer, DereferenceCount);
        }

        if (ARGUMENT_PRESENT(PreviousState)) {
            if (PreviousMode != KernelMode) {
                try {
                    *PreviousState = State;

                } except(EXCEPTION_EXECUTE_HANDLER) {
                    NOTHING;
                }

            } else {
                *PreviousState = State;
            }
        }
    }

    //
    // Return service status.
    //

    return Status;
}

VOID
ExGetNextWakeTime (
    OUT PULONGLONG DueTime,
    OUT PTIME_FIELDS TimeFields,
    OUT PVOID *TimerObject
    )

{

    ULONGLONG BestDueTime;
    LARGE_INTEGER CmosTime;
    PETIMER BestTimer;
    PETIMER ExTimer;
    ULONGLONG InterruptTime;
    PLIST_ENTRY Link;
    KIRQL OldIrql;
    LARGE_INTEGER SystemTime;
    ULONGLONG TimerDueTime;

    ExAcquireSpinLock(&ExpWakeTimerListLock, &OldIrql);
    BestDueTime = 0;
    BestTimer = NULL;
    Link = ExpWakeTimerList.Flink;
    while (Link != &ExpWakeTimerList) {
        ExTimer = CONTAINING_RECORD(Link, ETIMER, WakeTimerListEntry);
        Link = Link->Flink;
        if (ExTimer->WakeTimer) {
            TimerDueTime = KeQueryTimerDueTime(&ExTimer->KeTimer);
            TimerDueTime = 0 - TimerDueTime;

            //
            // Is this timers due time closer?
            //

            if (TimerDueTime > BestDueTime) {
                BestDueTime = TimerDueTime;
                BestTimer = ExTimer;
            }

        } else {

            //
            // Timer is not an active wake timer, remove it
            //

            RemoveEntryList(&ExTimer->WakeTimerListEntry);
            ExTimer->WakeTimerListEntry.Flink = NULL;
        }
    }

    ExReleaseSpinLock(&ExpWakeTimerListLock, OldIrql);
    if (BestDueTime) {

        //
        // Convert time to time fields.
        //

        KeQuerySystemTime (&SystemTime);
        InterruptTime = KeQueryInterruptTime ();
        BestDueTime = 0 - BestDueTime;
        SystemTime.QuadPart += BestDueTime - InterruptTime;

        //
        // Many system alarms are only good to 1 second resolution.
        // Add one sceond to the target time so that the timer is really
        // elasped if this is the wake event.
        //

        SystemTime.QuadPart += 10000000;
        ExSystemTimeToLocalTime(&SystemTime,&CmosTime);
        RtlTimeToTimeFields(&CmosTime, TimeFields);
    }

    *DueTime = BestDueTime;
    *TimerObject = BestTimer;
    return;
}

