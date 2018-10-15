/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    exatom.c

Abstract:

    This file contains functions for manipulating global atom tables
    stored in kernel space.

--*/

#include "exp.h"
#pragma hdrstop

//
// Define maximum size of an atom name.
//

#define COPY_STACK_SIZE (RTL_ATOM_MAXIMUM_NAME_LENGTH * sizeof(WCHAR))

C_ASSERT(COPY_STACK_SIZE <= 1024);

//
// Define dummy global atom table address prototype.
//

PVOID
ExpDummyGetAtomTable (
    VOID
    );

#pragma alloc_text(PAGE, NtAddAtom)
#pragma alloc_text(PAGE, NtDeleteAtom)
#pragma alloc_text(PAGE, NtFindAtom)
#pragma alloc_text(PAGE, NtQueryInformationAtom)
#pragma alloc_text(PAGE, ExpDummyGetAtomTable)

//
// Define function variable that holds the address of the callout routine.
//

PKWIN32_GLOBALATOMTABLE_CALLOUT ExGlobalAtomTableCallout = ExpDummyGetAtomTable;

NTSTATUS
NtAddAtom (
    __in_bcount_opt(Length) PWSTR AtomName,
    __in ULONG Length,
    __out_opt PRTL_ATOM Atom
    )

/*++

Routine Description:

    This function adds the specified atom to the global atom table.

Arguments:

    AtomName - Supplies a pointer to an unicode atom name.

    Length - Supplies the length of the atom name in bytes.

    Atom - Supplies an optional pointer to a variable that receives the atom
        value.

Return Value:

    The service status is returned as the function value.

--*/

{

    PVOID AtomTable = (ExGlobalAtomTableCallout)();
    PWSTR CapturedAtomName;
    KPROCESSOR_MODE PreviousMode;
    RTL_ATOM ReturnAtom;
    ULONG_PTR StackArray[(COPY_STACK_SIZE / sizeof(ULONG_PTR)) + 1];
    NTSTATUS Status;

    PAGED_CODE();

    //
    // If the atom table is not defined, then return access denied.
    //

    if (AtomTable == NULL) {
        return STATUS_ACCESS_DENIED;
    }

    //
    // If the atom length is too large, then return invalid parameter. 
    //

    if (Length > COPY_STACK_SIZE) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // If the previous mode is user, then probe and copy the arguments as
    // appropriate.
    //
    // N.B. It is known that the length is less than a page from the above
    //      check and, therefore, a small buffer probe can be used.
    //

    PreviousMode = KeGetPreviousMode();
    CapturedAtomName = AtomName;
    if (PreviousMode != KernelMode) {
        try {
            if (ARGUMENT_PRESENT(Atom)) {
                ProbeForWriteUshort(Atom);
            }
    
            if (ARGUMENT_PRESENT(AtomName)) {
                ProbeForRead(AtomName, Length, sizeof(WCHAR));
                CapturedAtomName = (PWSTR)&StackArray[0];
                RtlCopyMemory(CapturedAtomName, AtomName, Length);
                CapturedAtomName[Length / sizeof (WCHAR)] = UNICODE_NULL;
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

    //
    // Add the specified name to the global atom table.
    //

    Status = RtlAddAtomToAtomTable(AtomTable, CapturedAtomName, &ReturnAtom);
    if ((ARGUMENT_PRESENT(Atom)) && (NT_SUCCESS(Status))) {
        if (PreviousMode != KernelMode) {
            try {
                *Atom = ReturnAtom;

            } except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
            }

        } else {
            *Atom = ReturnAtom;
        }
    }

    return Status;
}

NTSTATUS
NtFindAtom (
    __in_bcount_opt(Length) PWSTR AtomName,
    __in ULONG Length,
    __out_opt PRTL_ATOM Atom
    )

/*++

Routine Description:

    This function looks up the specified atom in the global atom table.

Arguments:

    AtomName - Supplies a pointer to an unicode atom name.

    Length - Supplies the length of the atom name in bytes.

    Atom - Supplies an optional pointer to a variable that receives the atom
        value.

Return Value:

    The service status is returned as the function value.

--*/

{

    PVOID AtomTable = (ExGlobalAtomTableCallout)();
    PWSTR CapturedAtomName;
    KPROCESSOR_MODE PreviousMode;
    RTL_ATOM ReturnAtom;
    ULONG_PTR StackArray[(COPY_STACK_SIZE / sizeof(ULONG_PTR)) + 1];
    NTSTATUS Status;

    PAGED_CODE();

    //
    // If the atom table is not defined, then return access denied.
    //

    if (AtomTable == NULL) {
        return STATUS_ACCESS_DENIED;
    }

    //
    // If the atom length is too large, then return invalid parameter. 
    //

    if (Length > COPY_STACK_SIZE) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // If the previous mode is user, then probe and copy the arguments as
    // appropriate.
    //
    // N.B. It is known that the length is less than a page from the above
    //      check and, therefore, a small buffer probe can be used.
    //

    PreviousMode = KeGetPreviousMode();
    CapturedAtomName = AtomName;
    if (PreviousMode != KernelMode) {
        try {
            if (ARGUMENT_PRESENT(Atom)) {
                ProbeForWriteUshort(Atom);
            }
    
            if (ARGUMENT_PRESENT(AtomName)) {
                ProbeForRead(AtomName, Length, sizeof(WCHAR));
                CapturedAtomName = (PWSTR)&StackArray[0];
                RtlCopyMemory(CapturedAtomName, AtomName, Length);
                CapturedAtomName[Length / sizeof (WCHAR)] = UNICODE_NULL;
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

    //
    // Lookup the specified name in the global atom table.
    //

    Status = RtlLookupAtomInAtomTable(AtomTable, CapturedAtomName, &ReturnAtom);
    if ((ARGUMENT_PRESENT(Atom)) && (NT_SUCCESS(Status))) {
        if (PreviousMode != KernelMode) {
            try {
                *Atom = ReturnAtom;

            } except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
            }

        } else {
            *Atom = ReturnAtom;
        }
    }

    return Status;
}

NTSTATUS
NtDeleteAtom (
    __in RTL_ATOM Atom
    )

/*++

Routine Description:

    This function deletes the specified atom from the global atom table.

Arguments:

    Atom - Supplies the atom identification.

Return Value:

    The deletion status is returned as the function value.

--*/

{

    PVOID AtomTable = (ExGlobalAtomTableCallout)();

    PAGED_CODE();

    //
    // If the atom table is not defined, then return access denied.
    //

    if (AtomTable == NULL) {
        return STATUS_ACCESS_DENIED;
    }

    return RtlDeleteAtomFromAtomTable(AtomTable, Atom);
}

NTSTATUS
NtQueryInformationAtom (
    __in RTL_ATOM Atom,
    __in ATOM_INFORMATION_CLASS InformationClass,
    __out_bcount(AtomInformationLength) PVOID AtomInformation,
    __in ULONG AtomInformationLength,
    __out_opt PULONG ReturnLength
    )

/*++

Routine Description:

    This function queries the specified information for the specified atom.

Arguments:

    Atom - Supplies the atom identification.

    
Return Value:

--*/

{

    ULONG AtomFlags;
    PVOID AtomTable = (ExGlobalAtomTableCallout)();
    PATOM_BASIC_INFORMATION BasicInfo;
    ULONG NameLength;
    KPROCESSOR_MODE PreviousMode;
    ULONG RequiredLength;
    NTSTATUS Status;
    PATOM_TABLE_INFORMATION TableInfo;
    ULONG UsageCount;

    PAGED_CODE();

    //
    // If the atom table is not defined, then return access denied.
    //

    if (AtomTable == NULL) {
        return STATUS_ACCESS_DENIED;
    }

    //
    // Query atom information.
    //

    Status = STATUS_SUCCESS;
    try {

        //
        // Get previous processor mode and probe output argument if necessary.
        //

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForWrite(AtomInformation, AtomInformationLength, sizeof(ULONG));
            if (ARGUMENT_PRESENT(ReturnLength)) {
                ProbeForWriteUlong(ReturnLength);
            }
        }

        RequiredLength = 0;
        switch (InformationClass) {

            //
            // Query basic atom information.
            //

        case AtomBasicInformation:
            RequiredLength = FIELD_OFFSET(ATOM_BASIC_INFORMATION, Name);
            if (AtomInformationLength < RequiredLength) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            BasicInfo = (PATOM_BASIC_INFORMATION)AtomInformation;
            UsageCount = 0;
            NameLength = AtomInformationLength - RequiredLength;
            BasicInfo->Name[ 0 ] = UNICODE_NULL;
            Status = RtlQueryAtomInAtomTable(AtomTable,
                                             Atom,
                                             &UsageCount,
                                             &AtomFlags,
                                             &BasicInfo->Name[0],
                                             &NameLength );

            if (NT_SUCCESS(Status)) {
                BasicInfo->UsageCount = (USHORT)UsageCount;
                BasicInfo->Flags = (USHORT)AtomFlags;
                BasicInfo->NameLength = (USHORT)NameLength;
                RequiredLength += NameLength + sizeof(WCHAR);
            }

            break;

            //
            // Query atom table information.
            //

        case AtomTableInformation:
            RequiredLength = FIELD_OFFSET(ATOM_TABLE_INFORMATION, Atoms);
            if (AtomInformationLength < RequiredLength) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            TableInfo = (PATOM_TABLE_INFORMATION)AtomInformation;
            Status = RtlQueryAtomsInAtomTable(AtomTable,
                                              (AtomInformationLength - RequiredLength) / sizeof(RTL_ATOM),
                                              &TableInfo->NumberOfAtoms,
                                              &TableInfo->Atoms[0]);

            if (NT_SUCCESS(Status)) {
                RequiredLength += TableInfo->NumberOfAtoms * sizeof(RTL_ATOM);
            }

            break;

            //
            // Invalid information class.
            //

        default:
            Status = STATUS_INVALID_INFO_CLASS;
            break;
        }

        if (ARGUMENT_PRESENT(ReturnLength)) {
            *ReturnLength = RequiredLength;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    return Status;
}

PVOID
ExpDummyGetAtomTable (
    VOID
    )

/*++

Routine Description:

    This function is a dummy function that is used until the win32k subsystem
    defines the atom table call back address.

Arguments:

    None.

Return Value:

    NULL.

--*/

{

    return NULL;
}

