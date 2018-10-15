/*++

Copyright (c) 1996-2000 Microsoft Corporation

Module Name:

    utils.c

Abstract:

    This module contains assorted utility functions for PCI.SYS.

Author:

    Peter Johnston (peterj)  20-Nov-1996

Revision History:

    Eric Nelson (enelson)  20-Mar-2000 - kidnap registry function

--*/

#include "agplib.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AgpOpenKey)
#pragma alloc_text(PAGE, AgpStringToUSHORT)
#endif


ULONGLONG
AgpGetDeviceFlags(
    IN PAGP_HACK_TABLE_ENTRY AgpHackTable,
    IN USHORT VendorID,
    IN USHORT DeviceID,
    IN USHORT SubVendorID,
    IN USHORT SubSystemID,
    IN UCHAR  RevisionID
    )
/*++

Description:

    Look in the registry for any flags for this VendorId/DeviceId.

Arguments:

    VendorId      PCI Vendor ID (16 bits) of the manufacturer of the
                  device.

    DeviceId      PCI Device ID (16 bits) of the device.

    SubVendorID   PCI SubVendorID representing the manufacturer of the
                  subsystem

    SubSystemID   PCI SubSystemID representing subsystem

    RevisionID    PCI Revision denoting the revision of the device

Return Value:

    64 bit flags value or 0 if not found.

--*/
{
    PAGP_HACK_TABLE_ENTRY current;
    ULONGLONG hackFlags = 0;
    ULONG match, bestMatch = 0;

    if (AgpHackTable == NULL) {
        return hackFlags;
    }

    // 
    // We want to do a best-case match:
    // VVVVDDDDSSSSssssRR
    // VVVVDDDDSSSSssss
    // VVVVDDDDRR
    // VVVVDDDD
    //
    // List is currently unsorted, so keep updating current best match.
    //

    for (current = AgpHackTable; current->VendorID != 0xFFFF; current++) {
        match = 0;

        //
        // Must at least match vendor/dev
        //

        if ((current->DeviceID != DeviceID) ||
            (current->VendorID != VendorID)) {
            continue;
        }

        match = 1;

        //
        // If this entry specifies a revision, check that it is consistent.
        // 

        if (current->Flags & AGP_HACK_FLAG_REVISION) {
            if (current->RevisionID == RevisionID) {
                match += 2;
            } else {
                continue;
            }
        }

        //
        // If this entry specifies subsystems, check that they are consistent
        //

        if (current->Flags & AGP_HACK_FLAG_SUBSYSTEM) {
            if (current->SubVendorID == SubVendorID &&
                current->SubSystemID == SubSystemID) {
                match += 4;
            } else {
                continue;
            }
        }

        if (match > bestMatch) {
            bestMatch = match;
            hackFlags = current->DeviceFlags;
        }
    }

    return hackFlags;
}



BOOLEAN
AgpOpenKey(
    IN  PWSTR   KeyName,
    IN  HANDLE  ParentHandle,
    OUT PHANDLE Handle,
    OUT PNTSTATUS Status
    )
/*++

Description:

    Open a registry key.

Arguments:

    KeyName      Name of the key to be opened.
    ParentHandle Pointer to the parent handle (OPTIONAL)
    Handle       Pointer to a handle to recieve the opened key.

Return Value:

    TRUE is key successfully opened, FALSE otherwise.

--*/
{
    UNICODE_STRING    nameString;
    OBJECT_ATTRIBUTES nameAttributes;
    NTSTATUS localStatus;

    PAGED_CODE();

    RtlInitUnicodeString(&nameString, KeyName);

    InitializeObjectAttributes(&nameAttributes,
                               &nameString,
                               OBJ_CASE_INSENSITIVE,
                               ParentHandle,
                               (PSECURITY_DESCRIPTOR)NULL
                               );
    localStatus = ZwOpenKey(Handle,
                            KEY_READ,
                            &nameAttributes
                            );

    if (Status != NULL) {

        //
        // Caller wants underlying status.
        //

        *Status = localStatus;
    }

    //
    // Return status converted to a boolean, TRUE if
    // successful.
    //

    return NT_SUCCESS(localStatus);
}



BOOLEAN
AgpStringToUSHORT(
    IN PWCHAR String,
    OUT PUSHORT Result
    )
/*++

Description:

    Takes a 4 character hexidecimal sting and converts it into a USHORT.

Arguments:

    String - the string

    Result - the USHORT

Return Value:

    TRUE is success, FASLE otherwise

--*/
{
    ULONG count;
    USHORT number = 0;
    PWCHAR current;

    current = String;

    for (count = 0; count < 4; count++) {

        number <<= 4;

        if (*current >= L'0' && *current <= L'9') {
            number |= *current - L'0';
        } else if (*current >= L'A' && *current <= L'F') {
            number |= *current + 10 - L'A';
        } else if (*current >= L'a' && *current <= L'f') {
            number |= *current + 10 - L'a';
        } else {
            return FALSE;
        }

        current++;
    }

    *Result = number;
    return TRUE;
}



ULONG_PTR
AgpExecuteCriticalSystemRoutine(
    IN ULONG_PTR Context
    )
/*++

Routine Description:

    This routine is called in the context of KeIpiGenericCall, which
    executes it on all processors.  It is used to execute
    a critical routine which needs all processors synchronized, such
    as probing the BARs of a device that could not otherwise be turned off.
    Only one context parameter is allowed in this routine, so it must
    contain both the routine to execute and any context that routine
    requires.

    When this routine is entered, it is guaranteed that all processors will
    already have been targeted with an IPI, and will be running at IPI_LEVEL.
    All processors will either be running this routine, or will be about to
    enter the routine.  No arbitrary threads can possibly be running.  No
    devices can interrupt the execution of this routine, since IPI_LEVEL is
    above all device IRQLs.

    Because this routine runs at IPI_LEVEL, no debug prints, asserts or other
    debugging can occur in this function without hanging MP machines.

Arguments:

    Context - the context passed into the call to KeIpiGenericCall.
        It contains the critical routine to execute, any context required
        in that routine and a gate and a barrier to ensure that the critical
        routine is executed on only one processor, even though this function
        is executed on all processors.

Return Value:

    STATUS_SUCCESS, or error

--*/
{
    NTSTATUS Status;
    PAGP_CRITICAL_ROUTINE_CONTEXT routineContext =
        (PAGP_CRITICAL_ROUTINE_CONTEXT)Context;

    Status = STATUS_SUCCESS;

    //
    // The Gate parameter in the routineContext is preinitialized
    // to 1, meaning that the first processor to reach this point
    // in the routine will decrement it to 0, and succeed the if
    // statement.
    //
    if (InterlockedDecrement(&routineContext->Gate) == 0) {

        //
        // This is only executed on one processor.
        //
        Status = (NTSTATUS)routineContext->Routine(routineContext->Extension,
                                                   routineContext->Context
                                                   );

        //
        // Free other processors.
        //
        routineContext->Barrier = 0;

    } else {

        //
        // Wait for gated function to complete.
        //
        do {
        } while (routineContext->Barrier != 0);
    }

    return Status;
}

