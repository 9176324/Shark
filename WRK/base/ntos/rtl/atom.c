/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    atom.c

Abstract:

    This file contains the common code to implement atom tables.  It is called
    by both the user mode Win32 Atom API functions (Local/GlobalxxxAtom) and
    by the kernel mode window manager code to access global atoms.

--*/

#include "ntrtlp.h"
#include "atom.h"

#if defined(ALLOC_PRAGMA)
PVOID
RtlpAllocateAtom(
    IN ULONG NumberOfBytes,
    IN ULONG Tag
    );
void
RtlpFreeAtom(
    IN PVOID p
    );
void
RtlpInitializeLockAtomTable(
    IN OUT PRTL_ATOM_TABLE AtomTable
    );
BOOLEAN
RtlpLockAtomTable(
    IN PRTL_ATOM_TABLE AtomTable
    );
void
RtlpUnlockAtomTable(
    IN PRTL_ATOM_TABLE AtomTable
    );
void
RtlpDestroyLockAtomTable(
    IN OUT PRTL_ATOM_TABLE AtomTable
    );
BOOLEAN
RtlpInitializeHandleTableForAtomTable(
    PRTL_ATOM_TABLE AtomTable
    );
void
RtlpDestroyHandleTableForAtomTable(
    PRTL_ATOM_TABLE AtomTable
    );
PRTL_ATOM_TABLE_ENTRY
RtlpAtomMapAtomToHandleEntry(
    IN PRTL_ATOM_TABLE AtomTable,
    IN ULONG HandleIndex
    );
BOOLEAN
RtlpCreateHandleForAtom(
    PRTL_ATOM_TABLE p,
    PRTL_ATOM_TABLE_ENTRY a
    );
void
RtlpFreeHandleForAtom(
    PRTL_ATOM_TABLE p,
    PRTL_ATOM_TABLE_ENTRY a
    );
BOOLEAN
RtlpGetIntegerAtom(
    PWSTR Name,
    PRTL_ATOM Atom OPTIONAL
    );
PRTL_ATOM_TABLE_ENTRY
RtlpHashStringToAtom(
    IN PRTL_ATOM_TABLE p,
    IN PWSTR Name,
    OUT PRTL_ATOM_TABLE_ENTRY **PreviousAtom OPTIONAL,
    OUT PULONG NameLength
    );
#pragma alloc_text(PAGE,RtlpAllocateAtom)
#pragma alloc_text(PAGE,RtlpFreeAtom)
#pragma alloc_text(PAGE,RtlpInitializeLockAtomTable)
#pragma alloc_text(PAGE,RtlInitializeAtomPackage)
#pragma alloc_text(PAGE,RtlpLockAtomTable)
#pragma alloc_text(PAGE,RtlpUnlockAtomTable)
#pragma alloc_text(PAGE,RtlpDestroyLockAtomTable)
#pragma alloc_text(PAGE,RtlpInitializeHandleTableForAtomTable)
#pragma alloc_text(PAGE,RtlpDestroyHandleTableForAtomTable)
#pragma alloc_text(PAGE,RtlpAtomMapAtomToHandleEntry)
#pragma alloc_text(PAGE,RtlpCreateHandleForAtom)
#pragma alloc_text(PAGE,RtlpFreeHandleForAtom)
#pragma alloc_text(PAGE,RtlInitializeAtomPackage)
#pragma alloc_text(PAGE,RtlCreateAtomTable)
#pragma alloc_text(PAGE,RtlDestroyAtomTable)
#pragma alloc_text(PAGE,RtlEmptyAtomTable)
#pragma alloc_text(PAGE,RtlpGetIntegerAtom)
#pragma alloc_text(PAGE,RtlpHashStringToAtom)
#pragma alloc_text(PAGE,RtlAddAtomToAtomTable)
#pragma alloc_text(PAGE,RtlLookupAtomInAtomTable)
#pragma alloc_text(PAGE,RtlDeleteAtomFromAtomTable)
#pragma alloc_text(PAGE,RtlPinAtomInAtomTable)
#pragma alloc_text(PAGE,RtlQueryAtomInAtomTable)
#pragma alloc_text(PAGE,RtlQueryAtomsInAtomTable)
#endif

#if defined(ALLOC_DATA_PRAGMA)
#pragma data_seg("PAGEDATA")
#endif

ULONG RtlpAtomAllocateTag;

typedef struct _RTLP_ATOM_QUOTA {
    PEPROCESS_QUOTA_BLOCK QuotaBlock;
    SIZE_T PagedAmount;
} RTLP_ATOM_QUOTA, *PRTLP_ATOM_QUOTA;

PVOID
RtlpAllocateAtom(
    IN ULONG NumberOfBytes,
    IN ULONG Tag
    )
{
    PRTLP_ATOM_QUOTA AllocatedBlock;

    NumberOfBytes += sizeof(RTLP_ATOM_QUOTA);

    AllocatedBlock = ExAllocatePoolWithTag( PagedPool,
                                            NumberOfBytes,
                                            Tag );
    if (AllocatedBlock) {
        AllocatedBlock->QuotaBlock
            = PsChargeSharedPoolQuota( PsGetCurrentProcess(),
                                       NumberOfBytes,
                                       0 );
        if (! AllocatedBlock->QuotaBlock) {
            ExFreePoolWithTag( AllocatedBlock, Tag );
            AllocatedBlock = NULL;
        } else {
            AllocatedBlock->PagedAmount = NumberOfBytes;
            AllocatedBlock++;
        }
    }

    return AllocatedBlock;
}


void
RtlpFreeAtom(
    IN PVOID p
    )
{
    PRTLP_ATOM_QUOTA AllocatedBlock = (PRTLP_ATOM_QUOTA) p;

    ASSERT( AllocatedBlock );
    AllocatedBlock--;
    ASSERT( AllocatedBlock );
    ASSERT( AllocatedBlock->QuotaBlock );
    ASSERT( AllocatedBlock->PagedAmount );
    PsReturnSharedPoolQuota( AllocatedBlock->QuotaBlock,
                             AllocatedBlock->PagedAmount,
                             0 );
    
    ExFreePool( AllocatedBlock );
    return;
}


void
RtlpInitializeLockAtomTable(
    IN OUT PRTL_ATOM_TABLE AtomTable
    )
{
    ExInitializePushLock( &AtomTable->PushLock );
    return;
}

BOOLEAN
RtlpLockAtomTable(
    IN PRTL_ATOM_TABLE AtomTable
    )
{
    if (AtomTable == NULL || AtomTable->Signature != RTL_ATOM_TABLE_SIGNATURE) {
        return FALSE;
        }

    KeEnterCriticalRegion ();
    ExAcquirePushLockExclusive( &AtomTable->PushLock );

    return TRUE;
}

void
RtlpUnlockAtomTable(
    IN PRTL_ATOM_TABLE AtomTable
    )
{
    ExReleasePushLockExclusive( &AtomTable->PushLock );
    KeLeaveCriticalRegion ();
}


void
RtlpDestroyLockAtomTable(
    IN OUT PRTL_ATOM_TABLE AtomTable
    )
{
}


BOOLEAN
RtlpInitializeHandleTableForAtomTable(
    PRTL_ATOM_TABLE AtomTable
    )
{
    AtomTable->ExHandleTable = ExCreateHandleTable( NULL );
    if (AtomTable->ExHandleTable != NULL) {
        //
        // Make sure atom handle tables are NOT part of object handle enumeration
        //

        ExRemoveHandleTable( AtomTable->ExHandleTable );
        return TRUE;
        }
    else {
        return FALSE;
        }
}

void
RtlpDestroyHandleTableForAtomTable(
    PRTL_ATOM_TABLE AtomTable
    )
{
    ExDestroyHandleTable( AtomTable->ExHandleTable, NULL );
    return;
}

PRTL_ATOM_TABLE_ENTRY
RtlpAtomMapAtomToHandleEntry(
    IN PRTL_ATOM_TABLE AtomTable,
    IN ULONG HandleIndex
    )
{
    PHANDLE_TABLE_ENTRY ExHandleEntry;
    PRTL_ATOM_TABLE_ENTRY a;
    EXHANDLE ExHandle;

    ExHandle.GenericHandleOverlay = 0;
    ExHandle.Index = HandleIndex;

    ExHandleEntry = ExMapHandleToPointer( AtomTable->ExHandleTable,
                                          ExHandle.GenericHandleOverlay
                                        );
    if (ExHandleEntry != NULL) {
        a = ExHandleEntry->Object;
        ExUnlockHandleTableEntry( AtomTable->ExHandleTable, ExHandleEntry );
        return a;
        }
    return NULL;
}

BOOLEAN
RtlpCreateHandleForAtom(
    PRTL_ATOM_TABLE p,
    PRTL_ATOM_TABLE_ENTRY a
    )
{
    EXHANDLE ExHandle;
    HANDLE_TABLE_ENTRY ExHandleEntry;

    ExHandleEntry.Object = a;
    ExHandleEntry.GrantedAccess = 0;
    ExHandle.GenericHandleOverlay = ExCreateHandle( p->ExHandleTable, &ExHandleEntry );
    if (ExHandle.GenericHandleOverlay != NULL) {
        a->HandleIndex = (USHORT)ExHandle.Index;
        a->Atom = (RTL_ATOM)((USHORT)a->HandleIndex | RTL_ATOM_MAXIMUM_INTEGER_ATOM);
        return TRUE;
        }
    return FALSE;
}

void
RtlpFreeHandleForAtom(
    PRTL_ATOM_TABLE p,
    PRTL_ATOM_TABLE_ENTRY a
    )
{
    EXHANDLE ExHandle;

    ExHandle.GenericHandleOverlay = 0;
    ExHandle.Index = a->HandleIndex;
    ExDestroyHandle( p->ExHandleTable, ExHandle.GenericHandleOverlay, NULL );
    return;
}

NTSTATUS
RtlInitializeAtomPackage(
    IN ULONG AllocationTag
    )
{
    RTL_PAGED_CODE();
    RtlpAtomAllocateTag = AllocationTag;
    return STATUS_SUCCESS;
}

NTSTATUS
RtlCreateAtomTable(
    IN ULONG NumberOfBuckets,
    OUT PVOID *AtomTableHandle
    )
{
    NTSTATUS Status;
    PRTL_ATOM_TABLE p;
    ULONG Size;

    RTL_PAGED_CODE();
    Status = STATUS_SUCCESS;
    if (*AtomTableHandle == NULL) {
        if (NumberOfBuckets <= 1) {
            NumberOfBuckets = RTL_ATOM_TABLE_DEFAULT_NUMBER_OF_BUCKETS;
            }

        Size = sizeof( RTL_ATOM_TABLE ) +
               (sizeof( RTL_ATOM_TABLE_ENTRY ) * (NumberOfBuckets-1));

        p = (PRTL_ATOM_TABLE)RtlpAllocateAtom( Size, 'TmtA' );
        if (p == NULL) {
            Status = STATUS_NO_MEMORY;
            }
        else {
            RtlZeroMemory( p, Size );
            p->NumberOfBuckets = NumberOfBuckets;
            if (RtlpInitializeHandleTableForAtomTable( p )) {
                RtlpInitializeLockAtomTable( p );
                p->Signature = RTL_ATOM_TABLE_SIGNATURE;
                *AtomTableHandle = p;
                }
            else {
                Status = STATUS_NO_MEMORY;
                RtlpFreeAtom( p );
                }
            }
        }

    return Status;
}


NTSTATUS
RtlDestroyAtomTable(
    IN PVOID AtomTableHandle
    )
{
    NTSTATUS Status;
    PRTL_ATOM_TABLE p = (PRTL_ATOM_TABLE)AtomTableHandle;
    PRTL_ATOM_TABLE_ENTRY a, aNext, *pa;
    ULONG i;

    RTL_PAGED_CODE();
    Status = STATUS_SUCCESS;
    if (!RtlpLockAtomTable( p )) {
        return STATUS_INVALID_PARAMETER;
        }
    try {
        pa = &p->Buckets[ 0 ];
        for (i=0; i<p->NumberOfBuckets; i++) {
            aNext = *pa;
            *pa++ = NULL;
            while ((a = aNext) != NULL) {
                aNext = a->HashLink;
                a->HashLink = NULL;
                RtlpFreeAtom( a );
                }
            }
        p->Signature = 0;
        RtlpUnlockAtomTable( p );

        RtlpDestroyHandleTableForAtomTable( p );
        RtlpDestroyLockAtomTable( p );
        RtlZeroMemory( p, sizeof( RTL_ATOM_TABLE ) );
        RtlpFreeAtom( p );
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        }

    return Status;
}

NTSTATUS
RtlEmptyAtomTable(
    IN PVOID AtomTableHandle,
    IN BOOLEAN IncludePinnedAtoms
    )
{
    NTSTATUS Status;
    PRTL_ATOM_TABLE p = (PRTL_ATOM_TABLE)AtomTableHandle;
    PRTL_ATOM_TABLE_ENTRY a, aNext, *pa, *pa1;
    ULONG i;

    RTL_PAGED_CODE();
    Status = STATUS_SUCCESS;
    if (!RtlpLockAtomTable( p )) {
        return STATUS_INVALID_PARAMETER;
        }
    try {
        pa = &p->Buckets[ 0 ];
        for (i=0; i<p->NumberOfBuckets; i++) {
            pa1 = pa++;
            while ((a = *pa1) != NULL) {
                if (IncludePinnedAtoms || !(a->Flags & RTL_ATOM_PINNED)) {
                    *pa1 = a->HashLink;
                    a->HashLink = NULL;
                    RtlpFreeHandleForAtom( p, a );
                    RtlpFreeAtom( a );
                    }
                else {
                    pa1 = &a->HashLink;
                    }
                }
            }

        RtlpUnlockAtomTable( p );
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        }

    return Status;
}

BOOLEAN
RtlpGetIntegerAtom(
    PWSTR Name,
    PRTL_ATOM Atom OPTIONAL
    )
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    PWSTR s;
    ULONG n;
    RTL_ATOM Temp;

    if (((ULONG_PTR)Name & -0x10000) == 0) {
        Temp = (RTL_ATOM)(USHORT)PtrToUlong(Name);
        if (Temp >= RTL_ATOM_MAXIMUM_INTEGER_ATOM) {
            return FALSE;
            }
        else {
            if (Temp == RTL_ATOM_INVALID_ATOM) {
                Temp = RTL_ATOM_MAXIMUM_INTEGER_ATOM;
                }

            if (ARGUMENT_PRESENT( Atom )) {
                *Atom = Temp;
                }

            return TRUE;
            }
        }
    else
    if (*Name != L'#') {
        return FALSE;
        }

    s = ++Name;
    while (*s != UNICODE_NULL) {
        if (*s < L'0' || *s > L'9') {
            return FALSE;
            }
        else {
            s++;
            }
        }

    n = 0;
    UnicodeString.Buffer = Name;
    UnicodeString.Length = (USHORT)((PCHAR)s - (PCHAR)Name);
    UnicodeString.MaximumLength = UnicodeString.Length;
    Status = RtlUnicodeStringToInteger( &UnicodeString, 10, &n );
    if (NT_SUCCESS( Status )) {
        if (ARGUMENT_PRESENT( Atom )) {
            if (n == 0 || n > RTL_ATOM_MAXIMUM_INTEGER_ATOM) {
                *Atom = RTL_ATOM_MAXIMUM_INTEGER_ATOM;
                }
            else {
                *Atom = (RTL_ATOM)n;
                }
            }

        return TRUE;
        }
    else {
        return FALSE;
        }
}

PRTL_ATOM_TABLE_ENTRY
RtlpHashStringToAtom(
    IN PRTL_ATOM_TABLE p,
    IN PWSTR Name,
    OUT PRTL_ATOM_TABLE_ENTRY **PreviousAtom OPTIONAL,
    OUT PULONG NameLength
    )
{
    ULONG Length, Hash;
    WCHAR c;
    PWCH s;
    RTL_ATOM Atom;
    PRTL_ATOM_TABLE_ENTRY *pa, a;

    if (((ULONG_PTR)Name & -0x10000) == 0) {
        Atom = (RTL_ATOM)(USHORT)PtrToUlong(Name);
        a = NULL;
        if (Atom >= RTL_ATOM_MAXIMUM_INTEGER_ATOM) {
            a = RtlpAtomMapAtomToHandleEntry( p,
                                              (ULONG)(Atom & (USHORT)~RTL_ATOM_MAXIMUM_INTEGER_ATOM)
                                            );
            }

        if (ARGUMENT_PRESENT( PreviousAtom )) {
            *PreviousAtom = NULL;
            }

        return a;
        }

    s = Name;
    Hash = 0;
    while (*s != UNICODE_NULL) {
        c = *s++;
        if (c < 'a') {
            NOTHING;

        } else if (c > 'z') {
            c = RtlUpcaseUnicodeChar(c);

        } else {
            c -= ('a' - 'A');
        }

        Hash = Hash + (c << 1) + (c >> 1) + c;
    }

    Length = (ULONG) (s - Name);
    if (Length > RTL_ATOM_MAXIMUM_NAME_LENGTH) {
        pa = NULL;
        a = NULL;
        }
    else {
        pa = &p->Buckets[ Hash % p->NumberOfBuckets ];
        while (a = *pa) {
            if (a->NameLength == Length && !_wcsicmp( a->Name, Name )) {
                break;
                }
            else {
                pa = &a->HashLink;
                }
            }
        }

    if (ARGUMENT_PRESENT( PreviousAtom )) {
        *PreviousAtom = pa;
        }

    if (a == NULL && ARGUMENT_PRESENT( NameLength )) {
        *NameLength = Length * sizeof( WCHAR );
        }

    return a;
}


NTSTATUS
RtlAddAtomToAtomTable(
    __in PVOID AtomTableHandle,
    __in PWSTR AtomName,
    __inout_opt PRTL_ATOM Atom
    )
{
    NTSTATUS Status;
    PRTL_ATOM_TABLE p = (PRTL_ATOM_TABLE)AtomTableHandle;
    PRTL_ATOM_TABLE_ENTRY a, *pa;
    ULONG NameLength;
    RTL_ATOM Temp;

    RTL_PAGED_CODE();
    if (!RtlpLockAtomTable( p )) {
        return STATUS_INVALID_PARAMETER;
        }
    try {
        if (RtlpGetIntegerAtom( AtomName, &Temp )) {
            if (Temp >= RTL_ATOM_MAXIMUM_INTEGER_ATOM) {
                Temp = RTL_ATOM_INVALID_ATOM;
                Status = STATUS_INVALID_PARAMETER;
                }
            else {
                Status = STATUS_SUCCESS;
                }

            if (ARGUMENT_PRESENT( Atom )) {
                *Atom = Temp;
                }
            }
        else
        if (*AtomName == UNICODE_NULL) {
            Status = STATUS_OBJECT_NAME_INVALID;
            }
        else {
            a = RtlpHashStringToAtom( p, AtomName, &pa, &NameLength );
            if (a == NULL) {
                if (pa != NULL) {
                    Status = STATUS_NO_MEMORY;
                    a = RtlpAllocateAtom( FIELD_OFFSET( RTL_ATOM_TABLE_ENTRY, Name ) +
                                          NameLength + sizeof( UNICODE_NULL ),
                                          'AmtA'
                                        );
                    if (a != NULL) {
                        a->HashLink = NULL;
                        a->ReferenceCount = 1;
                        a->Flags = 0;
                        RtlCopyMemory( a->Name, AtomName, NameLength );
                        a->NameLength = (UCHAR)(NameLength / sizeof( WCHAR ));
                        a->Name[ a->NameLength ] = UNICODE_NULL;
                        if (RtlpCreateHandleForAtom( p, a )) {
                            a->Atom = (RTL_ATOM)a->HandleIndex | RTL_ATOM_MAXIMUM_INTEGER_ATOM;
                            *pa = a;
                            if (ARGUMENT_PRESENT( Atom )) {
                                *Atom = a->Atom;
                                }

                            Status = STATUS_SUCCESS;
                            }
                        else {
                            RtlpFreeAtom( a );
                            }
                        }
                    }
                else {
                    Status = STATUS_INVALID_PARAMETER;
                    }
                }
            else {
                if (!(a->Flags & RTL_ATOM_PINNED)) {
                    if (a->ReferenceCount == 0xFFFF) {
                        KdPrint(( "RTL: Pinning atom (%x) as reference count about to wrap\n", Atom ));
                        a->Flags |= RTL_ATOM_PINNED;
                        }
                    else {
                        a->ReferenceCount += 1;
                        }
                    }

                if (ARGUMENT_PRESENT( Atom )) {
                    *Atom = a->Atom;
                    }

                Status = STATUS_SUCCESS;
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        }

    RtlpUnlockAtomTable( p );

    return Status;
}

NTSTATUS
RtlLookupAtomInAtomTable(
    __in PVOID AtomTableHandle,
    __in PWSTR AtomName,
    __out_opt PRTL_ATOM Atom
    )
{
    NTSTATUS Status;
    PRTL_ATOM_TABLE p = (PRTL_ATOM_TABLE)AtomTableHandle;
    PRTL_ATOM_TABLE_ENTRY a;
    RTL_ATOM Temp;

    RTL_PAGED_CODE();
    if (!RtlpLockAtomTable( p )) {
        return STATUS_INVALID_PARAMETER;
        }
    try {
        if (RtlpGetIntegerAtom( AtomName, &Temp )) {
            if (Temp >= RTL_ATOM_MAXIMUM_INTEGER_ATOM) {
                Temp = RTL_ATOM_INVALID_ATOM;
                Status = STATUS_INVALID_PARAMETER;
                }
            else {
                Status = STATUS_SUCCESS;
                }

            if (ARGUMENT_PRESENT( Atom )) {
                *Atom = Temp;
                }
            }
        else
        if (*AtomName == UNICODE_NULL) {
            Status = STATUS_OBJECT_NAME_INVALID;
            }
        else {
            a = RtlpHashStringToAtom( p, AtomName, NULL, NULL );
            if (a == NULL) {
                Status = STATUS_OBJECT_NAME_NOT_FOUND;
                }
            else {
                if (RtlpAtomMapAtomToHandleEntry( p, (ULONG)a->HandleIndex ) != NULL) {
                    Status = STATUS_SUCCESS;
                    if (ARGUMENT_PRESENT( Atom )) {
                        *Atom = a->Atom;
                        }
                    }
                else {
                    Status = STATUS_INVALID_HANDLE;
                    }
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        }

    RtlpUnlockAtomTable( p );

    return Status;
}


NTSTATUS
RtlDeleteAtomFromAtomTable(
    IN PVOID AtomTableHandle,
    IN RTL_ATOM Atom
    )
{
    NTSTATUS Status;
    PRTL_ATOM_TABLE p = (PRTL_ATOM_TABLE)AtomTableHandle;
    PRTL_ATOM_TABLE_ENTRY a, *pa;

    RTL_PAGED_CODE();
    if (!RtlpLockAtomTable( p )) {
        return STATUS_INVALID_PARAMETER;
        }
    try {
        Status = STATUS_INVALID_HANDLE;
        if (Atom >= RTL_ATOM_MAXIMUM_INTEGER_ATOM) {
            a = RtlpAtomMapAtomToHandleEntry( p,
                                              (ULONG)(Atom & (USHORT)~RTL_ATOM_MAXIMUM_INTEGER_ATOM)
                                            );
            if (a != NULL && a->Atom == Atom) {
                Status = STATUS_SUCCESS;
                if (a->Flags & RTL_ATOM_PINNED) {
                    KdPrint(( "RTL: Ignoring attempt to delete a pinned atom (%x)\n", Atom ));
                    Status = STATUS_WAS_LOCKED;        // This is a success status code!
                    }
                else
                if (--a->ReferenceCount == 0) {
                    a = RtlpHashStringToAtom( p, a->Name, &pa, NULL );
                    if (a != NULL) {
                        if (pa != NULL) {
                            *pa = a->HashLink;
                        }
                        RtlpFreeHandleForAtom( p, a );
                        RtlpFreeAtom( a );
                        }
                    }
                }
            }
        else
        if (Atom != RTL_ATOM_INVALID_ATOM) {
            Status = STATUS_SUCCESS;
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        }

    RtlpUnlockAtomTable( p );

    return Status;
}

NTSTATUS
RtlPinAtomInAtomTable(
    IN PVOID AtomTableHandle,
    IN RTL_ATOM Atom
    )
{
    NTSTATUS Status;
    PRTL_ATOM_TABLE p = (PRTL_ATOM_TABLE)AtomTableHandle;
    PRTL_ATOM_TABLE_ENTRY a, *pa;

    RTL_PAGED_CODE();
    if (!RtlpLockAtomTable( p )) {
        return STATUS_INVALID_PARAMETER;
        }
    try {
        Status = STATUS_INVALID_HANDLE;
        if (Atom >= RTL_ATOM_MAXIMUM_INTEGER_ATOM) {
            a = RtlpAtomMapAtomToHandleEntry( p,
                                              (ULONG)(Atom & (USHORT)~RTL_ATOM_MAXIMUM_INTEGER_ATOM)
                                            );
            if (a != NULL && a->Atom == Atom) {
                Status = STATUS_SUCCESS;
                a->Flags |= RTL_ATOM_PINNED;
                }
            }
        else
        if (Atom != RTL_ATOM_INVALID_ATOM) {
            Status = STATUS_SUCCESS;
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        }

    RtlpUnlockAtomTable( p );

    return Status;
}

NTSTATUS
RtlQueryAtomInAtomTable(
    __in PVOID AtomTableHandle,
    __in RTL_ATOM Atom,
    __out_opt PULONG AtomUsage,
    __out_opt PULONG AtomFlags,
    __inout_bcount_part_opt(*AtomNameLength, *AtomNameLength) PWSTR AtomName,
    __inout_opt PULONG AtomNameLength
    )
{
    NTSTATUS Status;
    PRTL_ATOM_TABLE p = (PRTL_ATOM_TABLE)AtomTableHandle;
    PRTL_ATOM_TABLE_ENTRY a;
    WCHAR AtomNameBuffer[ 16 ];
    ULONG CopyLength;

    RTL_PAGED_CODE();
    if (!RtlpLockAtomTable( p )) {
        return STATUS_INVALID_PARAMETER;
        }
    try {
        if (Atom < RTL_ATOM_MAXIMUM_INTEGER_ATOM) {
            if (Atom == RTL_ATOM_INVALID_ATOM) {
                Status = STATUS_INVALID_PARAMETER;
                }
            else {
                Status = STATUS_SUCCESS;
                if (ARGUMENT_PRESENT( AtomUsage )) {
                    *AtomUsage = 1;
                    }

                if (ARGUMENT_PRESENT( AtomFlags )) {
                    *AtomFlags = RTL_ATOM_PINNED;
                    }

                if (ARGUMENT_PRESENT( AtomName )) {
                    CopyLength = _snwprintf( AtomNameBuffer,
                                             sizeof( AtomNameBuffer ) / sizeof( WCHAR ),
                                             L"#%u",
                                             Atom
                                           ) * sizeof( WCHAR );
                    if (CopyLength >= *AtomNameLength) {
                        if (*AtomNameLength >= sizeof( UNICODE_NULL )) {
                            CopyLength = *AtomNameLength - sizeof( UNICODE_NULL );
                            }
                        else {
                            CopyLength = 0;
                            }
                        }

                    if (CopyLength != 0) {
                        RtlCopyMemory( AtomName, AtomNameBuffer, CopyLength );
                        AtomName[ CopyLength / sizeof( WCHAR ) ] = UNICODE_NULL;
                        *AtomNameLength = CopyLength;
                        }
                    else {
                        Status = STATUS_BUFFER_TOO_SMALL;
                        }
                    }
                }
            }
        else {
            a = RtlpAtomMapAtomToHandleEntry( p,
                                              (ULONG)(Atom & (USHORT)~RTL_ATOM_MAXIMUM_INTEGER_ATOM)
                                            );
            if (a != NULL && a->Atom == Atom) {
                Status = STATUS_SUCCESS;
                if (ARGUMENT_PRESENT( AtomUsage )) {
                    *AtomUsage = a->ReferenceCount;
                    }

                if (ARGUMENT_PRESENT( AtomFlags )) {
                    *AtomFlags = a->Flags;
                    }

                if (ARGUMENT_PRESENT( AtomName )) {
                    //
                    // Fill in as much of the atom string as possible, and
                    // always zero terminate. This is what win3.1 does.
                    //

                    CopyLength = a->NameLength * sizeof( WCHAR );
                    if (CopyLength >= *AtomNameLength) {
                        if (*AtomNameLength >= sizeof( UNICODE_NULL )) {
                            CopyLength = *AtomNameLength - sizeof( UNICODE_NULL );
                            }
                        else {
                            *AtomNameLength = CopyLength;
                            CopyLength = 0;
                            }
                        }
                    if (CopyLength != 0) {
                        RtlCopyMemory( AtomName, a->Name, CopyLength );
                        AtomName[ CopyLength / sizeof( WCHAR ) ] = UNICODE_NULL;
                        *AtomNameLength = CopyLength;
                        }
                    else {
                        Status = STATUS_BUFFER_TOO_SMALL;
                        }
                    }
                }
            else {
                Status = STATUS_INVALID_HANDLE;
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        }

    RtlpUnlockAtomTable( p );

    return Status;
}

NTSTATUS
RtlQueryAtomsInAtomTable(
    IN PVOID AtomTableHandle,
    IN ULONG MaximumNumberOfAtoms,
    OUT PULONG NumberOfAtoms,
    OUT PRTL_ATOM Atoms
    )
{
    NTSTATUS Status;
    PRTL_ATOM_TABLE p = (PRTL_ATOM_TABLE)AtomTableHandle;
    PRTL_ATOM_TABLE_ENTRY a;
    ULONG i;
    ULONG CurrentAtomIndex;

    RTL_PAGED_CODE();
    if (!RtlpLockAtomTable( p )) {
        return STATUS_INVALID_PARAMETER;
        }

    Status = STATUS_SUCCESS;
    try {
        CurrentAtomIndex = 0;
        for (i=0; i<p->NumberOfBuckets; i++) {
            a = p->Buckets[ i ];
            while (a) {
                if (CurrentAtomIndex < MaximumNumberOfAtoms) {
                    Atoms[ CurrentAtomIndex ] = a->Atom;
                    }
                else {
                    Status = STATUS_INFO_LENGTH_MISMATCH;
                    }

                CurrentAtomIndex += 1;
                a = a->HashLink;
                }
            }

        *NumberOfAtoms = CurrentAtomIndex;
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        }

    RtlpUnlockAtomTable( p );

    return Status;
}

#if defined(ALLOC_DATA_PRAGMA)
#pragma data_seg()
#endif

