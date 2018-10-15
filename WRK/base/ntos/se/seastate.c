/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    SeAstate.c

Abstract:

    This Module implements the privilege check procedures.

--*/

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SeCreateAccessState)
#pragma alloc_text(PAGE,SeCreateAccessStateEx)
#pragma alloc_text(PAGE,SeDeleteAccessState)
#pragma alloc_text(PAGE,SeSetAccessStateGenericMapping)
#pragma alloc_text(PAGE,SeAppendPrivileges)
#pragma alloc_text(PAGE,SepConcatenatePrivileges)
#endif


//
// Define logical sum of all generic accesses.
//

#define GENERIC_ACCESS (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | GENERIC_ALL)


//
// The PRIVILEGE_SET data structure includes an array including ANYSIZE_ARRAY
// elements.  This definition provides the size of an empty PRIVILEGE_SET
// (i.e., one with no privileges in it).
//

#define SEP_PRIVILEGE_SET_HEADER_SIZE           \
            ((ULONG)sizeof(PRIVILEGE_SET) -     \
                (ANYSIZE_ARRAY * (ULONG)sizeof(LUID_AND_ATTRIBUTES)))





NTSTATUS
SeCreateAccessState(
   __out PACCESS_STATE AccessState,
   __out PAUX_ACCESS_DATA AuxData,
   __in ACCESS_MASK DesiredAccess,
   __in_opt PGENERIC_MAPPING GenericMapping
   )

/*++
Routine Description:

    This routine initializes an ACCESS_STATE structure.  This consists
    of:

    - zeroing the entire structure

    - mapping generic access types in the passed DesiredAccess
    and putting it into the structure

    - "capturing" the Subject Context, which must be held for the
    duration of the access attempt (at least until auditing is performed).

    - Allocating an Operation ID, which is an LUID that will be used
    to associate different parts of the access attempt in the audit
    log.

Arguments:

    AccessState - a pointer to the structure to be initialized.

    AuxData - Supplies a buffer big enough for an AuxData structure
        so we don't have to allocate one.

    DesiredAccess - Access mask containing the desired access

    GenericMapping - Optionally supplies a pointer to a generic mapping
        that may be used to map any generic access requests that may
        have been passed in the DesiredAccess parameter.

        Note that if this parameter is not supplied, it must be filled
        in at some later point.  The IO system does this in IopParseDevice.

Return Value:

    Error if the attempt to allocate an LUID fails.

    Note that this error may be safely ignored if it is known that all
    security checks will be performed with PreviousMode == KernelMode.
    Know what you're doing if you choose to ignore this.

--*/

{
    return SeCreateAccessStateEx (PsGetCurrentThread (),
                                  PsGetCurrentProcess (),
                                  AccessState,
                                  AuxData,
                                  DesiredAccess,
                                  GenericMapping);
}

NTSTATUS
SeCreateAccessStateEx(
   __in_opt PETHREAD Thread,
   __in PEPROCESS Process,
   __out PACCESS_STATE AccessState,
   __out PAUX_ACCESS_DATA AuxData,
   __in ACCESS_MASK DesiredAccess,
   __in_opt PGENERIC_MAPPING GenericMapping
   )

/*++
Routine Description:

    This routine initializes an ACCESS_STATE structure.  This consists
    of:

    - zeroing the entire structure

    - mapping generic access types in the passed DesiredAccess
    and putting it into the structure

    - "capturing" the Subject Context, which must be held for the
    duration of the access attempt (at least until auditing is performed).

    - Allocating an Operation ID, which is an LUID that will be used
    to associate different parts of the access attempt in the audit
    log.

Arguments:

    Thread - Optional thread to capture impersonation token from. If
             NULL no impersonation token is captured.

    Process - Process to capture primary token from.

    AccessState - a pointer to the structure to be initialized.

    AuxData - Supplies a buffer big enough for an AuxData structure
        so we don't have to allocate one.

    DesiredAccess - Access mask containing the desired access

    GenericMapping - Optionally supplies a pointer to a generic mapping
        that may be used to map any generic access requests that may
        have been passed in the DesiredAccess parameter.

        Note that if this parameter is not supplied, it must be filled
        in at some later point.  The IO system does this in IopParseDevice.

Return Value:

    Error if the attempt to allocate an LUID fails.

    Note that this error may be safely ignored if it is known that all
    security checks will be performed with PreviousMode == KernelMode.
    Know what you're doing if you choose to ignore this.

--*/

{

    ACCESS_MASK MappedAccessMask;
    PSECURITY_DESCRIPTOR InputSecurityDescriptor = NULL;

    PAGED_CODE();

    //
    // Don't modify what he passed in
    //

    MappedAccessMask = DesiredAccess;

    //
    // Map generic access to object specific access iff generic access types
    // are specified and a generic access mapping table is provided.
    //

    if ( ((DesiredAccess & GENERIC_ACCESS) != 0) &&
         ARGUMENT_PRESENT(GenericMapping) ) {

        RtlMapGenericMask(
            &MappedAccessMask,
            GenericMapping
            );
    }

    RtlZeroMemory(AccessState, sizeof(ACCESS_STATE));
    RtlZeroMemory(AuxData, sizeof(AUX_ACCESS_DATA));

    //
    // Assume RtlZeroMemory has initialized these fields properly
    //

    ASSERT( AccessState->SecurityDescriptor == NULL );
    ASSERT( AccessState->PrivilegesAllocated == FALSE );

    AccessState->AuxData = AuxData;

    SeCaptureSubjectContextEx(Thread, Process, &AccessState->SubjectSecurityContext);

    if (((PTOKEN)EffectiveToken( &AccessState->SubjectSecurityContext ))->TokenFlags & TOKEN_HAS_TRAVERSE_PRIVILEGE ) {
        AccessState->Flags = TOKEN_HAS_TRAVERSE_PRIVILEGE;
    }

    AccessState->RemainingDesiredAccess = MappedAccessMask;
    AccessState->OriginalDesiredAccess = MappedAccessMask;
    AuxData->PrivilegesUsed = (PPRIVILEGE_SET)((ULONG_PTR)AccessState +
                              (FIELD_OFFSET(ACCESS_STATE, Privileges)));

    ExAllocateLocallyUniqueId(&AccessState->OperationID);

    if (ARGUMENT_PRESENT(GenericMapping)) {
        AuxData->GenericMapping = *GenericMapping;
    }

    return( STATUS_SUCCESS );

}


VOID
SeDeleteAccessState(
    __in PACCESS_STATE AccessState
    )

/*++

Routine Description:

    This routine deallocates any memory that may have been allocated as
    part of constructing the access state (normally only for an excessive
    number of privileges), and frees the Subject Context.

Arguments:

    AccessState - a pointer to the ACCESS_STATE structure to be
        deallocated.

Return Value:

    None.

--*/

{
    PAUX_ACCESS_DATA AuxData;

    PAGED_CODE();

    AuxData = (PAUX_ACCESS_DATA)AccessState->AuxData;

    if (AccessState->PrivilegesAllocated) {
        ExFreePool( (PVOID)AuxData->PrivilegesUsed );
    }

    if (AccessState->ObjectName.Buffer != NULL) {
        ExFreePool(AccessState->ObjectName.Buffer);
    }

    if (AccessState->ObjectTypeName.Buffer != NULL) {
        ExFreePool(AccessState->ObjectTypeName.Buffer);
    }

    SeReleaseSubjectContext(&AccessState->SubjectSecurityContext);

    return;
}

VOID
SeSetAccessStateGenericMapping (
    __inout PACCESS_STATE AccessState,
    __in PGENERIC_MAPPING GenericMapping
    )

/*++

Routine Description:

    This routine sets the GenericMapping field in an AccessState structure.
    It must be called before access validation is performed if the GenericMapping
    is not passed in when the AccessState structure is created.

Arguments:

    AccessState - a pointer to the ACCESS_STATE structure to be modified.

    GenericMapping - a pointer to the GenericMapping to be copied into the AccessState.

Return Value:


--*/
{
    PAUX_ACCESS_DATA AuxData;

    PAGED_CODE();

    AuxData = (PAUX_ACCESS_DATA)AccessState->AuxData;

    AuxData->GenericMapping = *GenericMapping;

    return;
}



NTSTATUS
SeAppendPrivileges(
    __inout PACCESS_STATE AccessState,
    __in PPRIVILEGE_SET Privileges
    )
/*++

Routine Description:

    This routine takes a privilege set and adds it to the privilege set
    embedded in an ACCESS_STATE structure.

    An AccessState may contain up to three embedded privileges.  To
    add more, this routine will allocate a block of memory, copy
    the current privileges into it, and append the new privilege
    to that block.  A bit is set in the AccessState indicating that
    the pointer to the privilege set in the structure points to pool
    memory and must be deallocated.

Arguments:

    AccessState - The AccessState structure representing the current
        access attempt.

    Privileges - A pointer to a privilege set to be added.

Return Value:

    STATUS_INSUFFICIENT_RESOURCES - an attempt to allocate pool memory
        failed.

--*/

{
    ULONG NewPrivilegeSetSize;
    PPRIVILEGE_SET NewPrivilegeSet;
    PAUX_ACCESS_DATA AuxData;

    PAGED_CODE();

    AuxData = (PAUX_ACCESS_DATA)AccessState->AuxData;

    if (Privileges->PrivilegeCount + AuxData->PrivilegesUsed->PrivilegeCount >
        INITIAL_PRIVILEGE_COUNT) {

        //
        // Compute the total size of the two privilege sets
        //

        NewPrivilegeSetSize =  SepPrivilegeSetSize( Privileges ) +
                               SepPrivilegeSetSize( AuxData->PrivilegesUsed );

        NewPrivilegeSet = ExAllocatePoolWithTag( PagedPool, NewPrivilegeSetSize, 'rPeS' );

        if (NewPrivilegeSet == NULL) {
            return( STATUS_INSUFFICIENT_RESOURCES );
        }


        RtlCopyMemory(
            NewPrivilegeSet,
            AuxData->PrivilegesUsed,
            SepPrivilegeSetSize( AuxData->PrivilegesUsed )
            );

        //
        // Note that this will adjust the privilege count in the
        // structure for us.
        //

        SepConcatenatePrivileges(
            NewPrivilegeSet,
            NewPrivilegeSetSize,
            Privileges
            );

        if (AccessState->PrivilegesAllocated) {
            ExFreePool( AuxData->PrivilegesUsed );
        }

        AuxData->PrivilegesUsed = NewPrivilegeSet;

        //
        // Mark that we've allocated memory for the privilege set,
        // so we know to free it when we're cleaning up.
        //

        AccessState->PrivilegesAllocated = TRUE;

    } else {

        //
        // Note that this will adjust the privilege count in the
        // structure for us.
        //

        SepConcatenatePrivileges(
            AuxData->PrivilegesUsed,
            sizeof(INITIAL_PRIVILEGE_SET),
            Privileges
            );

    }

    return( STATUS_SUCCESS );

}


VOID
SepConcatenatePrivileges(
    IN PPRIVILEGE_SET TargetPrivilegeSet,
    IN ULONG TargetBufferSize,
    IN PPRIVILEGE_SET SourcePrivilegeSet
    )

/*++

Routine Description:

    Takes two privilege sets and appends the second to the end of the
    first.

    There must be enough space left at the end of the first privilege
    set to contain the second.

Arguments:

    TargetPrivilegeSet - Supplies a buffer containing a privilege set.
        The buffer must be large enough to contain the second privilege
        set.

    TargetBufferSize - Supplies the size of the target buffer.

    SourcePrivilegeSet - Supplies the privilege set to be copied
        into the target buffer.

Return Value:

    None

--*/

{
    PVOID Base;
    PVOID Source;
    ULONG Length;

    PAGED_CODE();

    ASSERT( ((ULONG)SepPrivilegeSetSize( TargetPrivilegeSet ) +
             (ULONG)SepPrivilegeSetSize( SourcePrivilegeSet ) -
             SEP_PRIVILEGE_SET_HEADER_SIZE  ) <=
            TargetBufferSize
          );

    Base = (PVOID)((ULONG_PTR)TargetPrivilegeSet + SepPrivilegeSetSize( TargetPrivilegeSet ));

    Source = (PVOID) ((ULONG_PTR)SourcePrivilegeSet + SEP_PRIVILEGE_SET_HEADER_SIZE);

    Length = SourcePrivilegeSet->PrivilegeCount * sizeof(LUID_AND_ATTRIBUTES);

    RtlMoveMemory(
        Base,
        Source,
        Length
        );

    TargetPrivilegeSet->PrivilegeCount += SourcePrivilegeSet->PrivilegeCount;
}

