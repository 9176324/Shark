/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    event.c

Abstract:

   This module implements the executive event object. Functions are provided
   to create, open, set, reset, pulse, and query event objects.

--*/

#include "exp.h"

//
// Temporary so boost is patchable
//

ULONG ExpEventBoost = EVENT_INCREMENT;

//
// Address of event object type descriptor.
//

POBJECT_TYPE ExEventObjectType;

//
// Structure that describes the mapping of generic access rights to object
// specific access rights for event objects.
//

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif

const GENERIC_MAPPING ExpEventMapping = {
    STANDARD_RIGHTS_READ | EVENT_QUERY_STATE,
    STANDARD_RIGHTS_WRITE | EVENT_MODIFY_STATE,
    STANDARD_RIGHTS_EXECUTE | SYNCHRONIZE,
    EVENT_ALL_ACCESS
};

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

#pragma alloc_text(INIT, ExpEventInitialization)
#pragma alloc_text(PAGE, NtClearEvent)
#pragma alloc_text(PAGE, NtCreateEvent)
#pragma alloc_text(PAGE, NtOpenEvent)
#pragma alloc_text(PAGE, NtPulseEvent)
#pragma alloc_text(PAGE, NtQueryEvent)
#pragma alloc_text(PAGE, NtResetEvent)
#pragma alloc_text(PAGE, NtSetEvent)
#pragma alloc_text(PAGE, NtSetEventBoostPriority)

BOOLEAN
ExpEventInitialization (
    VOID
    )

/*++

Routine Description:

    This function creates the event object type descriptor at system
    initialization and stores the address of the object type descriptor
    in global storage.

Arguments:

    None.

Return Value:

    A value of TRUE is returned if the event object type descriptor is
    successfully initialized. Otherwise a value of FALSE is returned.

--*/

{

    OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
    NTSTATUS Status;
    UNICODE_STRING TypeName;

    //
    // Initialize string descriptor.
    //

    RtlInitUnicodeString(&TypeName, L"Event");

    //
    // Create event object type descriptor.
    //

    RtlZeroMemory(&ObjectTypeInitializer, sizeof(ObjectTypeInitializer));
    ObjectTypeInitializer.Length = sizeof(ObjectTypeInitializer);
    ObjectTypeInitializer.InvalidAttributes = OBJ_OPENLINK;
    ObjectTypeInitializer.GenericMapping = ExpEventMapping;
    ObjectTypeInitializer.PoolType = NonPagedPool;
    ObjectTypeInitializer.DefaultNonPagedPoolCharge = sizeof(KEVENT);
    ObjectTypeInitializer.ValidAccessMask = EVENT_ALL_ACCESS;
    Status = ObCreateObjectType(&TypeName,
                                &ObjectTypeInitializer,
                                (PSECURITY_DESCRIPTOR)NULL,
                                &ExEventObjectType);

    //
    // If the event object type descriptor was successfully created, then
    // return a value of TRUE. Otherwise return a value of FALSE.
    //

    return (BOOLEAN)(NT_SUCCESS(Status));
}

NTSTATUS
NtClearEvent (
    __in HANDLE EventHandle
    )

/*++

Routine Description:

    This function sets an event object to a Not-Signaled state.

Arguments:

    EventHandle - Supplies a handle to an event object.

Return Value:

    NTSTATUS.

--*/

{

    PVOID Event;
    NTSTATUS Status;

    //
    // Reference event object by handle.
    //

    Status = ObReferenceObjectByHandle(EventHandle,
                                       EVENT_MODIFY_STATE,
                                       ExEventObjectType,
                                       KeGetPreviousMode(),
                                       &Event,
                                       NULL);

    //
    // If the reference was successful, then set the state of the event
    // object to Not-Signaled and dereference event object.
    //

    if (NT_SUCCESS(Status)) {
        PERFINFO_DECLARE_OBJECT(Event);
        KeClearEvent((PKEVENT)Event);
        ObDereferenceObject(Event);
    }

    //
    // Return service status.
    //

    return Status;
}

NTSTATUS
NtCreateEvent (
    __out PHANDLE EventHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in EVENT_TYPE EventType,
    __in BOOLEAN InitialState
    )

/*++

Routine Description:

    This function creates an event object, sets it initial state to the
    specified value, and opens a handle to the object with the specified
    desired access.

Arguments:

    EventHandle - Supplies a pointer to a variable that will receive the
        event object handle.

    DesiredAccess - Supplies the desired types of access for the event object.

    ObjectAttributes - Supplies a pointer to an object attributes structure.

    EventType - Supplies the type of the event (autoclearing or notification).

    InitialState - Supplies the initial state of the event object.

Return Value:

    NTSTATUS.

--*/

{

    PVOID Event;
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
            ProbeForWriteHandle(EventHandle);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

    //
    // Check argument validity.
    //

    if ((EventType != NotificationEvent) && (EventType != SynchronizationEvent)) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Allocate event object.
    //

    Status = ObCreateObject(PreviousMode,
                            ExEventObjectType,
                            ObjectAttributes,
                            PreviousMode,
                            NULL,
                            sizeof(KEVENT),
                            0,
                            0,
                            &Event);

    //
    // If the event object was successfully allocated, then initialize the
    // event object and attempt to insert the event object in the current
    // process' handle table.
    //

    if (NT_SUCCESS(Status)) {
        KeInitializeEvent((PKEVENT)Event, EventType, InitialState);
        Status = ObInsertObject(Event,
                                NULL,
                                DesiredAccess,
                                0,
                                NULL,
                                &Handle);

        //
        // If the event object was successfully inserted in the current
        // process' handle table, then attempt to write the event object
        // handle value. If the write attempt fails, then do not report
        // an error. When the caller attempts to access the handle value,
        // an access violation will occur.
        //

        if (NT_SUCCESS(Status)) {
            if (PreviousMode != KernelMode) {
                try {
                    *EventHandle = Handle;

                } except(EXCEPTION_EXECUTE_HANDLER) {
                    NOTHING;
                }

            } else {
                *EventHandle = Handle;
            }
        }
    }

    //
    // Return service status.
    //

    return Status;
}

NTSTATUS
NtOpenEvent (
    __out PHANDLE EventHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    )

/*++

Routine Description:

    This function opens a handle to an event object with the specified
    desired access.

Arguments:

    EventHandle - Supplies a pointer to a variable that will receive the
        event object handle.

    DesiredAccess - Supplies the desired types of access for the event object.

    ObjectAttributes - Supplies a pointer to an object attributes structure.

Return Value:

    NTSTATUS.

--*/

{

    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    //
    // Get previous processor mode and probe output handle address
    // if necessary.
    //

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {
            ProbeForWriteHandle(EventHandle);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

    //
    // Open handle to the event object with the specified desired access.
    //

    Status = ObOpenObjectByName(ObjectAttributes,
                                ExEventObjectType,
                                PreviousMode,
                                NULL,
                                DesiredAccess,
                                NULL,
                                &Handle);

    //
    // If the open was successful, then attempt to write the event object
    // handle value. If the write attempt fails, then do not report an
    // error. When the caller attempts to access the handle value, an
    // access violation will occur.
    //

    if (NT_SUCCESS(Status)) {
        if (PreviousMode != KernelMode) {
            try {
                *EventHandle = Handle;

            } except(EXCEPTION_EXECUTE_HANDLER) {
                NOTHING;
            }

        } else {
            *EventHandle = Handle;
        }
    }

    //
    // Return service status.
    //

    return Status;
}

NTSTATUS
NtPulseEvent (
    __in HANDLE EventHandle,
    __out_opt PLONG PreviousState
    )

/*++

Routine Description:

    This function sets an event object to a Signaled state, attempts to
    satisfy as many waits as possible, and then resets the state of the
    event object to Not-Signaled.

Arguments:

    EventHandle - Supplies a handle to an event object.

    PreviousState - Supplies an optional pointer to a variable that will
        receive the previous state of the event object.

Return Value:

    NTSTATUS.

--*/

{

    PVOID Event;
    KPROCESSOR_MODE PreviousMode;
    LONG State;
    NTSTATUS Status;

    //
    // Get previous processor mode and probe previous state address
    // if necessary.
    //

    PreviousMode = KeGetPreviousMode();
    if (ARGUMENT_PRESENT(PreviousState) && (PreviousMode != KernelMode)) {
        try {
            ProbeForWriteLong(PreviousState);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

    //
    // Reference event object by handle.
    //

    Status = ObReferenceObjectByHandle(EventHandle,
                                       EVENT_MODIFY_STATE,
                                       ExEventObjectType,
                                       PreviousMode,
                                       &Event,
                                       NULL);

    //
    // If the reference was successful, then pulse the event object,
    // dereference event object, and write the previous state value if
    // specified. If the write of the previous state fails, then do not
    // report an error. When the caller attempts to access the previous
    // state value, an access violation will occur.
    //

    if (NT_SUCCESS(Status)) {
        PERFINFO_DECLARE_OBJECT(Event);
        State = KePulseEvent((PKEVENT)Event, ExpEventBoost, FALSE);
        ObDereferenceObject(Event);
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

NTSTATUS
NtQueryEvent (
    __in HANDLE EventHandle,
    __in EVENT_INFORMATION_CLASS EventInformationClass,
    __out_bcount(EventInformationLength) PVOID EventInformation,
    __in ULONG EventInformationLength,
    __out_opt PULONG ReturnLength
    )

/*++

Routine Description:

    This function queries the state of an event object and returns the
    requested information in the specified record structure.

Arguments:

    EventHandle - Supplies a handle to an event object.

    EventInformationClass - Supplies the class of information being requested.

    EventInformation - Supplies a pointer to a record that is to receive the
        requested information.

    EventInformationLength - Supplies the length of the record that is to
        receive the requested information.

    ReturnLength - Supplies an optional pointer to a variable that is to
        receive the actual length of information that is returned.

Return Value:

    NTSTATUS.

--*/

{

    PKEVENT Event;
    PEVENT_BASIC_INFORMATION Information;
    KPROCESSOR_MODE PreviousMode;
    LONG State;
    NTSTATUS Status;
    EVENT_TYPE EventType;

    //
    // Check argument validity.
    //

    if (EventInformationClass != EventBasicInformation) {
        return STATUS_INVALID_INFO_CLASS;
    }

    if (EventInformationLength != sizeof(EVENT_BASIC_INFORMATION)) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    //
    // Get previous processor mode and probe output arguments if necessary.
    //

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {
            ProbeForWrite(EventInformation,
                          sizeof(EVENT_BASIC_INFORMATION),
                          sizeof(ULONG));

            if (ARGUMENT_PRESENT(ReturnLength)) {
                ProbeForWriteUlong(ReturnLength);
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

    //
    // Reference event object by handle.
    //

    Status = ObReferenceObjectByHandle(EventHandle,
                                       EVENT_QUERY_STATE,
                                       ExEventObjectType,
                                       PreviousMode,
                                       &Event,
                                       NULL);

    //
    // If the reference was successful, then read the current state of
    // the event object, deference event object, fill in the information
    // structure, and return the length of the information structure if
    // specified. If the write of the event information or the return
    // length fails, then do not report an error. When the caller accesses
    // the information structure or length an access violation will occur.
    //

    if (NT_SUCCESS(Status)) {
        State = KeReadStateEvent(Event);
        EventType = Event->Header.Type;
        Information = EventInformation;
        ObDereferenceObject(Event);
        if (PreviousMode != KernelMode) {
            try {
                Information->EventType = EventType;
                Information->EventState = State;
                if (ARGUMENT_PRESENT(ReturnLength)) {
                    *ReturnLength = sizeof(EVENT_BASIC_INFORMATION);
                }

            } except(EXCEPTION_EXECUTE_HANDLER) {
                NOTHING;
            }

        } else {
            Information->EventType = EventType;
            Information->EventState = State;
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = sizeof(EVENT_BASIC_INFORMATION);
            }
        }
    }

    //
    // Return service status.
    //

    return Status;
}

NTSTATUS
NtResetEvent (
    __in HANDLE EventHandle,
    __out_opt PLONG PreviousState
    )

/*++

Routine Description:

    This function sets an event object to a Not-Signaled state.

Arguments:

    EventHandle - Supplies a handle to an event object.

    PreviousState - Supplies an optional pointer to a variable that will
        receive the previous state of the event object.

Return Value:

    NTSTATUS.

--*/

{

    PVOID Event;
    KPROCESSOR_MODE PreviousMode;
    LONG State;
    NTSTATUS Status;

    //
    // Get previous processor mode and probe previous state address
    // if necessary.
    //

    PreviousMode = KeGetPreviousMode();
    if (ARGUMENT_PRESENT(PreviousState) && (PreviousMode != KernelMode)) {
        try {
            ProbeForWriteLong(PreviousState);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

    //
    // Reference event object by handle.
    //

    Status = ObReferenceObjectByHandle(EventHandle,
                                       EVENT_MODIFY_STATE,
                                       ExEventObjectType,
                                       PreviousMode,
                                       &Event,
                                       NULL);

    //
    // If the reference was successful, then set the state of the event
    // object to Not-Signaled, dereference event object, and write the
    // previous state value if specified. If the write of the previous
    // state fails, then do not report an error. When the caller attempts
    // to access the previous state value, an access violation will occur.
    //

    if (NT_SUCCESS(Status)) {
        PERFINFO_DECLARE_OBJECT(Event);
        State = KeResetEvent((PKEVENT)Event);
        ObDereferenceObject(Event);
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

NTSTATUS
NtSetEvent (
    __in HANDLE EventHandle,
    __out_opt PLONG PreviousState
    )

/*++

Routine Description:

    This function sets an event object to a Signaled state and attempts to
    satisfy as many waits as possible.

Arguments:

    EventHandle - Supplies a handle to an event object.

    PreviousState - Supplies an optional pointer to a variable that will
        receive the previous state of the event object.

Return Value:

    NTSTATUS.

--*/

{

    PVOID Event;
    KPROCESSOR_MODE PreviousMode;
    LONG State;
    NTSTATUS Status;

    //
    // Get previous processor mode and probe previous state address
    // if necessary.
    //

    PreviousMode = KeGetPreviousMode();
    try {
        if ((PreviousMode != KernelMode) && ARGUMENT_PRESENT(PreviousState)) {
            ProbeForWriteLong(PreviousState);
        }

    } except(ExSystemExceptionFilter()) {
        return GetExceptionCode();
    }

    //
    // Reference event object by handle.
    //

    Status = ObReferenceObjectByHandle(EventHandle,
                                       EVENT_MODIFY_STATE,
                                       ExEventObjectType,
                                       PreviousMode,
                                       &Event,
                                       NULL);

    //
    // If the reference was successful, then set the event object to the
    // Signaled state, dereference event object, and write the previous
    // state value if specified. If the write of the previous state fails,
    // then do not report an error. When the caller attempts to access the
    // previous state value, an access violation will occur.
    //

    if (NT_SUCCESS(Status)) {
        PERFINFO_DECLARE_OBJECT(Event);
        State = KeSetEvent((PKEVENT)Event, ExpEventBoost, FALSE);
        ObDereferenceObject(Event);
        if (ARGUMENT_PRESENT(PreviousState)) {
            try {
                *PreviousState = State;

            } except(ExSystemExceptionFilter()) {
                NOTHING;
            }
        }
    }

    //
    // Return service status.
    //

    return Status;
}

NTSTATUS
NtSetEventBoostPriority (
    __in HANDLE EventHandle
    )

/*++

Routine Description:

    This function sets an event object to a Signaled state and performs
    a special priority boost operation.

    N.B. This service can only be performed on synchronization events.

Arguments:

    EventHandle - Supplies a handle to an event object.

    PreviousState - Supplies an optional pointer to a variable that will
        receive the previous state of the event object.

Return Value:

    NTSTATUS.

--*/

{

    PKEVENT Event;
    NTSTATUS Status;

    //
    // Reference event object by handle.
    //

    Status = ObReferenceObjectByHandle(EventHandle,
                                       EVENT_MODIFY_STATE,
                                       ExEventObjectType,
                                       KeGetPreviousMode(),
                                       &Event,
                                       NULL);

    //
    // If the reference was successful, then check the type of event object.
    // If the event object is a notification event, then return an object
    // type mismatch status. Otherwise, set the specified event and boost
    // the unwaited thread priority as appropriate.
    //

    if (NT_SUCCESS(Status)) {
        if (Event->Header.Type == NotificationEvent) {
            Status = STATUS_OBJECT_TYPE_MISMATCH;

        } else {
            KeSetEventBoostPriority(Event, NULL);
        }

        ObDereferenceObject(Event);
    }

    //
    // Return service status.
    //

    return Status;
}

