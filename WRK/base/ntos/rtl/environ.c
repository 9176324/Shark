/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    environ.c

Abstract:

    Environment Variable support

--*/

#include "ntrtlp.h"
#include "zwapi.h"
#include "nturtl.h"
#include "string.h"
#include "ntrtlpath.h"
#include "wow64t.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(INIT,RtlCreateEnvironment          )
#pragma alloc_text(INIT,RtlDestroyEnvironment         )
#pragma alloc_text(INIT,RtlSetCurrentEnvironment      )
#pragma alloc_text(INIT,RtlQueryEnvironmentVariable_U )
#pragma alloc_text(INIT,RtlSetEnvironmentVariable     )
#pragma alloc_text(INIT,RtlSetEnvironmentStrings)
#endif

BOOLEAN RtlpEnvironCacheValid;

NTSTATUS
RtlCreateEnvironment(
    IN BOOLEAN CloneCurrentEnvironment OPTIONAL,
    OUT PVOID *Environment
    )
{
    NTSTATUS Status;
    MEMORY_BASIC_INFORMATION MemoryInformation;
    PVOID pNew, pOld;
    PPEB Peb;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;

    //
    // If not cloning a copy of the current process's environment variable
    // block, just allocate a block of committed memory and return its
    // address.
    //

    pNew = NULL;
    if (!CloneCurrentEnvironment) {
createEmptyEnvironment:
        MemoryInformation.RegionSize = 1;
        Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                          &pNew,
                                          0,
                                          &MemoryInformation.RegionSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE
                                        );
        if (NT_SUCCESS( Status )) {
            *Environment = pNew;
        }

        return( Status );
    }

    Peb = NtCurrentPeb ();

    ProcessParameters = Peb->ProcessParameters;

    //
    // Acquire the Peb Lock for the duration while we munge the environment
    // variable storage block.
    //

    RtlAcquirePebLock();

    //
    // Capture the pointer to the current process's environment variable
    // block and initialize the new pointer to null for our finally clause.
    //

    pOld = ProcessParameters->Environment;
    if (pOld == NULL) {
        RtlReleasePebLock();
        goto createEmptyEnvironment;
    }

    try {
        try {
            //
            // Query the current size of the current process's environment
            // variable block.  Return status if failure.
            //

            Status = ZwQueryVirtualMemory (NtCurrentProcess (),
                                           pOld,
                                           MemoryBasicInformation,
                                           &MemoryInformation,
                                           sizeof (MemoryInformation),
                                           NULL);
            if (!NT_SUCCESS (Status)) {
                leave;
            }

            //
            // Allocate memory to contain a copy of the current process's
            // environment variable block.  Return status if failure.
            //

            Status = ZwAllocateVirtualMemory (NtCurrentProcess (),
                                              &pNew,
                                              0,
                                              &MemoryInformation.RegionSize,
                                              MEM_COMMIT,
                                              PAGE_READWRITE);
            if (!NT_SUCCESS (Status)) {
                leave;
            }

            //
            // Copy the current process's environment to the allocated memory
            // and return a pointer to the copy.
            //

            RtlCopyMemory (pNew, pOld, MemoryInformation.RegionSize);
            *Environment = pNew;
        } except (EXCEPTION_EXECUTE_HANDLER) {
              Status = STATUS_ACCESS_VIOLATION;
        }
    } finally {
        RtlReleasePebLock ();

        if (Status == STATUS_ACCESS_VIOLATION) {
            if (pNew != NULL) {
                ZwFreeVirtualMemory (NtCurrentProcess(),
                                     &pNew,
                                     &MemoryInformation.RegionSize,
                                     MEM_RELEASE);
            }
        }

    }

    return (Status);
}


NTSTATUS
RtlDestroyEnvironment(
    IN PVOID Environment
    )
{
    NTSTATUS Status;
    SIZE_T RegionSize;

    //
    // Free the specified environment variable block.
    //

    RtlpEnvironCacheValid = FALSE;

    RegionSize = 0;
    Status = ZwFreeVirtualMemory( NtCurrentProcess(),
                                  &Environment,
                                  &RegionSize,
                                  MEM_RELEASE
                                );
    //
    // Return status.
    //

    return( Status );
}


NTSTATUS
RtlSetCurrentEnvironment(
    IN PVOID Environment,
    OUT PVOID *PreviousEnvironment OPTIONAL
    )
{
    NTSTATUS Status;
    PVOID pOld;
    PPEB Peb;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;

    //
    // Acquire the Peb Lock for the duration while we munge the environment
    // variable storage block.
    //

    Peb = NtCurrentPeb ();

    ProcessParameters = Peb->ProcessParameters;


    Status = STATUS_SUCCESS;

    RtlAcquirePebLock ();

    RtlpEnvironCacheValid = FALSE;

    //
    // Capture current process's environment variable block pointer to
    // return to caller or destroy.
    //

    pOld = ProcessParameters->Environment;

    //
    // Change current process's environment variable block pointer to
    // point to the passed block.
    //

    ProcessParameters->Environment = Environment;

    //
    // Release the Peb Lock
    //

    RtlReleasePebLock ();

    //
    // If caller requested it, return the pointer to the previous
    // process environment variable block and set the local variable
    // to NULL so we dont destroy it below.
    //

    if (ARGUMENT_PRESENT (PreviousEnvironment)) {
        *PreviousEnvironment = pOld;
    } else {
        //
        // If old environment not returned to caller, destroy it.
        //
 
        if (pOld != NULL) {
            RtlDestroyEnvironment (pOld);
        }
    }


    //
    // Return status
    //

    return (Status);
}

UNICODE_STRING RtlpEnvironCacheName;
UNICODE_STRING RtlpEnvironCacheValue;

NTSTATUS
RtlQueryEnvironmentVariable_U(
    IN PVOID Environment OPTIONAL,
    IN PCUNICODE_STRING Name,
    IN OUT PUNICODE_STRING Value
    )
{
    NTSTATUS Status;
    UNICODE_STRING CurrentName;
    UNICODE_STRING CurrentValue;
    PWSTR p, q;
    PPEB Peb;
    BOOLEAN PebLockLocked = FALSE;
    SIZE_T len;
    SIZE_T NameLength, NameChars;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;

    Status = STATUS_VARIABLE_NOT_FOUND;

    Peb = NtCurrentPeb();

    ProcessParameters = Peb->ProcessParameters;

    try {
        if (ARGUMENT_PRESENT (Environment)) {
            p = Environment;
            if (*p == UNICODE_NULL) {
                leave;
            }
        } else {


            //
            // Acquire the Peb Lock for the duration while we munge the
            // environment variable storage block.
            //

            PebLockLocked = TRUE;

            RtlAcquirePebLock ();

            //
            // Capture the pointer to the current process's environment variable
            // block.
            //

            p = ProcessParameters->Environment;

        }

        if (RtlpEnvironCacheValid && p == ProcessParameters->Environment) {
            if (RtlEqualUnicodeString (Name, &RtlpEnvironCacheName, TRUE)) {

                //
                // Names are equal.  Always return the length of the
                // value string, excluding the terminating null.  If
                // there is room in the caller's buffer, return a copy
                // of the value string and success status.  Otherwise
                // return an error status.  In the latter case, the caller
                // can examine the length field of their value string
                // so they can determine much memory is needed.
                //

                Value->Length = RtlpEnvironCacheValue.Length;
                if (Value->MaximumLength >= RtlpEnvironCacheValue.Length) {
                    RtlCopyMemory (Value->Buffer,
                                   RtlpEnvironCacheValue.Buffer,
                                   RtlpEnvironCacheValue.Length);
                    //
                    // Null terminate returned string if there is room.
                    //

                    if (Value->MaximumLength > RtlpEnvironCacheValue.Length) {
                        Value->Buffer[RtlpEnvironCacheValue.Length/sizeof(WCHAR)] = L'\0';
                    }

                    Status = STATUS_SUCCESS;
                } else {
                    Status = STATUS_BUFFER_TOO_SMALL;
                }
                leave;
            }
        }

        //
        // The environment variable block consists of zero or more null
        // terminated UNICODE strings.  Each string is of the form:
        //
        //      name=value
        //
        // where the null termination is after the value.
        //
        NameLength = Name->Length;
        NameChars = NameLength / sizeof (WCHAR);

        if (p != NULL) while (1) {

            //
            // Get the length of the terminated string. This should be in the
            // form 'keyword=value'
            //
            len = wcslen (p);
            if (len == 0) {
                break;
            }

            //
            // See if this environment variable is big enough to be our target.
            // If must be at least one bigger than the name we are searching
            // for since it must contain an '=' sign.
            //

            if (NameChars < len) {
                q = &p[NameChars];
                //
                // We have a possible match. See if there is an '=' at the correct point.
                //
                if (*q == L'=') {
                    //
                    // We have a possible match. Now compare the string out right
                    //
                    CurrentName.Length = (USHORT) NameLength;
                    CurrentName.Buffer = p;

                    //
                    // After comparing the strings we want to make sure we are not
                    // matching something that we shouldn't. For example if somebody
                    // did a lookup of "FRED=BOB" we don't want this to match "FRED=BOB=ALBERT".
                    // The lookup name is invalid and the environment variable is really FRED here
                    // not "FRED=BOB". To eliminate this case we make sure that the "=" character
                    // is at the appropriate place
                    // There are environment variable of the for =C:
                    // In order not to eliminate these we skip the first character in our search.
                    //
                    if (RtlEqualUnicodeString (Name, &CurrentName, TRUE) &&
                        (wcschr (p+1, L'=') == q)) {
                        //
                        // Names are equal.  Always return the length of the
                        // value string, excluding the terminating null.  If
                        // there is room in the caller's buffer, return a copy
                        // of the value string and success status.  Otherwise
                        // return an error status.  In the latter case, the caller
                        // can examine the length field of their value string
                        // so they can determine much memory is needed.
                        //
                        CurrentValue.Buffer = q+1;
                        CurrentValue.Length = (USHORT) ((len - 1) * sizeof (WCHAR) - NameLength);

                        Value->Length = CurrentValue.Length;
                        if (Value->MaximumLength >= CurrentValue.Length) {
                            RtlCopyMemory( Value->Buffer,
                                           CurrentValue.Buffer,
                                           CurrentValue.Length
                                         );
                            //
                            // Null terminate returned string if there is room.
                            //

                            if (Value->MaximumLength > CurrentValue.Length) {
                                Value->Buffer[ CurrentValue.Length/sizeof(WCHAR) ] = L'\0';
                            }

                            if (Environment == ProcessParameters->Environment) {
                                RtlpEnvironCacheValid = TRUE;
                                RtlpEnvironCacheName = CurrentName;
                                RtlpEnvironCacheValue = CurrentValue;
                            }

                            Status = STATUS_SUCCESS;
                        } else {
                            Status = STATUS_BUFFER_TOO_SMALL;
                        }

                        break;
                    }

                }
            }
            p += len + 1;
        }

        // If it's not in the real env block, let's see if it's a pseudo environment variable
        if (Status == STATUS_VARIABLE_NOT_FOUND) {
            static const UNICODE_STRING CurrentWorkingDirectoryPseudoVariable = RTL_CONSTANT_STRING(L"__CD__");
            static const UNICODE_STRING ApplicationDirectoryPseudoVariable = RTL_CONSTANT_STRING(L"__APPDIR__");

            if (RtlEqualUnicodeString(Name, &CurrentWorkingDirectoryPseudoVariable, TRUE)) {
                // Get the PEB lock if we don't already have it.
                if (!PebLockLocked) {
                    PebLockLocked = TRUE;
                    RtlAcquirePebLock();
                }

                // get cdw here...
                CurrentValue = ProcessParameters->CurrentDirectory.DosPath;
                Status = STATUS_SUCCESS;
            } else if (RtlEqualUnicodeString(Name, &ApplicationDirectoryPseudoVariable, TRUE)) {
                USHORT PrefixLength = 0;

                if (!PebLockLocked) {
                    PebLockLocked = TRUE;
                    RtlAcquirePebLock();
                }

                // get appdir here 
                CurrentValue = ProcessParameters->ImagePathName;

                Status = RtlFindCharInUnicodeString(
                                RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END,
                                &CurrentValue,
                                &RtlDosPathSeparatorsString,
                                &PrefixLength);
                if (NT_SUCCESS(Status)) {
                    CurrentValue.Length = PrefixLength + sizeof(WCHAR);
                } else if (Status == STATUS_NOT_FOUND) {
                    // Use the whole thing; just translate the status to success.
                    Status = STATUS_SUCCESS;
                }
            }

            if (NT_SUCCESS(Status)) {
                Value->Length = CurrentValue.Length;
                if (Value->MaximumLength >= CurrentValue.Length) {
                    RtlCopyMemory(Value->Buffer, CurrentValue.Buffer, CurrentValue.Length);

                    //
                    // Null terminate returned string if there is room.
                    //

                    if (Value->MaximumLength > CurrentValue.Length)
                        Value->Buffer[ CurrentValue.Length/sizeof(WCHAR) ] = L'\0';
                }
            }
        }



    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_ACCESS_VIOLATION;
    }


    //
    // Release the Peb lock.
    //

    if (PebLockLocked) {
        RtlReleasePebLock();
    }

    //
    // Return status.
    //

    return Status;
}


NTSTATUS
RtlSetEnvironmentVariable(
    IN OUT PVOID *Environment OPTIONAL,
    IN PCUNICODE_STRING Name,
    IN PCUNICODE_STRING Value OPTIONAL
    )
{
    NTSTATUS Status;
    MEMORY_BASIC_INFORMATION MemoryInformation;
    UNICODE_STRING CurrentName;
    UNICODE_STRING CurrentValue;
    PVOID pOld, pNew;
    ULONG n, Size;
    SIZE_T NewSize;
    LONG CompareResult;
    PWSTR p, pStart, pEnd;
    PWSTR InsertionPoint;
    PPEB Peb;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;

    //
    // Validate passed in name and reject if zero length or anything but the first
    // character is an equal sign.
    //
    n = Name->Length / sizeof( WCHAR );
    if (n == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    try {
        p = Name->Buffer;
        while (--n) {
            if (*++p == L'=') {
                return STATUS_INVALID_PARAMETER;
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

    Peb = NtCurrentPeb ();

    ProcessParameters = Peb->ProcessParameters;

    Status = STATUS_VARIABLE_NOT_FOUND;

    pNew = NULL;
    InsertionPoint = NULL;

    if (ARGUMENT_PRESENT (Environment)) {
        pOld = *Environment;
    } else {
        //
        // Acquire the Peb Lock for the duration while we munge the
        // environment variable storage block.
        //

        RtlAcquirePebLock();

        //
        // Capture the pointer to the current process's environment variable
        // block.
        //

        pOld = ProcessParameters->Environment;
    }

    RtlpEnvironCacheValid = FALSE;

    try {
        try {

            //
            // The environment variable block consists of zero or more null
            // terminated UNICODE strings.  Each string is of the form:
            //
            //      name=value
            //
            // where the null termination is after the value.
            //

            p = pOld;
            pEnd = NULL;
            if (p != NULL) while (*p) {
                //
                // Determine the size of the name and value portions of
                // the current string of the environment variable block.
                //

                CurrentName.Buffer = p;
                CurrentName.Length = 0;
                CurrentName.MaximumLength = 0;
                while (*p) {
                    //
                    // If we see an equal sign, then compute the size of
                    // the name portion and scan for the end of the value.
                    //

                    if (*p == L'=' && p != CurrentName.Buffer) {
                        CurrentName.Length = (USHORT)(p - CurrentName.Buffer) * sizeof(WCHAR);
                        CurrentName.MaximumLength = (USHORT)(CurrentName.Length+sizeof(WCHAR));
                        CurrentValue.Buffer = ++p;

                        while(*p) {
                            p++;
                        }
                        CurrentValue.Length = (USHORT)(p - CurrentValue.Buffer) * sizeof(WCHAR);
                        CurrentValue.MaximumLength = (USHORT)(CurrentValue.Length+sizeof(WCHAR));

                        //
                        // At this point we have the length of both the name
                        // and value portions, so exit the loop so we can
                        // do the compare.
                        //
                        break;
                    }
                    else {
                        p++;
                    }
                }

                //
                // Skip over the terminating null character for this name=value
                // pair in preparation for the next iteration of the loop.
                //

                p++;

                //
                // Compare the current name with the one requested, ignore
                // case.
                //

                if (!(CompareResult = RtlCompareUnicodeString( Name, &CurrentName, TRUE ))) {
                    //
                    // Names are equal.  Now find the end of the current
                    // environment variable block.
                    //

                    pEnd = p;
                    while (*pEnd) {
                        while (*pEnd++) {
                        }
                    }
                    pEnd++;

                    if (!ARGUMENT_PRESENT( Value )) {
                        //
                        // If the caller did not specify a new value, then delete
                        // the entire name=value pair by copying up the remainder
                        // of the environment variable block.
                        //

                        RtlMoveMemory( CurrentName.Buffer,
                                       p,
                                       (ULONG) ((pEnd - p)*sizeof(WCHAR))
                                     );
                        Status = STATUS_SUCCESS;

                    } else if (Value->Length <= CurrentValue.Length) {
                        //
                        // New value is smaller, so copy new value, then null
                        // terminate it, and then move up the remainder of the
                        // variable block so it is immediately after the new
                        // null terminated value.
                        //

                        pStart = CurrentValue.Buffer;
                        RtlMoveMemory( pStart, Value->Buffer, Value->Length );
                        pStart += Value->Length/sizeof(WCHAR);
                        *pStart++ = L'\0';

                        RtlMoveMemory( pStart, p,(ULONG)((pEnd - p)*sizeof(WCHAR)) );
                        Status = STATUS_SUCCESS;
                    } else {
                        //
                        // New value is larger, so query the current size of the
                        // environment variable block.  Return status if failure.
                        //

                        Status = ZwQueryVirtualMemory( NtCurrentProcess(),
                                                       pOld,
                                                       MemoryBasicInformation,
                                                       &MemoryInformation,
                                                       sizeof( MemoryInformation ),
                                                       NULL
                                                     );
                        if (!NT_SUCCESS( Status )) {
                            leave;
                        }

                        //
                        // See if there is room for new, larger value.  If not
                        // allocate a new copy of the environment variable
                        // block.
                        //

                        NewSize = (pEnd - (PWSTR)pOld)*sizeof(WCHAR) +
                                    Value->Length - CurrentValue.Length;
                        if (NewSize >= MemoryInformation.RegionSize) {
                            //
                            // Allocate memory to contain a copy of the current
                            // process's environment variable block.  Return
                            // status if failure.
                            //

                            Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                                              &pNew,
                                                              0,
                                                              &NewSize,
                                                              MEM_COMMIT,
                                                              PAGE_READWRITE
                                                            );
                            if (!NT_SUCCESS( Status )) {
                                leave;
                            }

                            //
                            // Copy the current process's environment to the allocated memory
                            // inserting the new value as we do the copy.
                            //

                            Size = (ULONG) (CurrentValue.Buffer - (PWSTR)pOld);
                            RtlMoveMemory( pNew, pOld, Size*sizeof(WCHAR) );
                            pStart = (PWSTR)pNew + Size;
                            RtlMoveMemory( pStart, Value->Buffer, Value->Length );
                            pStart += Value->Length/sizeof(WCHAR);
                            *pStart++ = L'\0';
                            RtlMoveMemory( pStart, p,(ULONG)((pEnd - p)*sizeof(WCHAR)));

    			            if (ARGUMENT_PRESENT( Environment )) {
    			                *Environment = pNew;
                            } else {
    			                ProcessParameters->Environment = pNew;

#if defined(BUILD_WOW6432)
                                ((PRTL_USER_PROCESS_PARAMETERS64)(NtCurrentPeb64()->ProcessParameters))->Environment = (ULONGLONG)(ULONG)pNew;
#endif
                                Peb->EnvironmentUpdateCount += 1;
                            }

                            ZwFreeVirtualMemory (NtCurrentProcess(),
                                                 &pOld,
                                                 &MemoryInformation.RegionSize,
                                                 MEM_RELEASE);
                            pNew = pOld;
                        } else {
                            pStart = CurrentValue.Buffer + Value->Length/sizeof(WCHAR) + 1;
                            RtlMoveMemory (pStart, p, (ULONG)((pEnd - p)*sizeof(WCHAR)));
                            *--pStart = L'\0';

                            RtlMoveMemory (pStart - Value->Length/sizeof(WCHAR),
                                           Value->Buffer,
                                           Value->Length);
                        }
                    }

                    break;
                } else if (CompareResult < 0) {
                    //
                    // Requested name is less than the current name.  Save this
                    // spot in case the variable is not in a sorted position.
                    // The insertion point for the new variable is before the
                    // variable just examined.
                    //

                    if (InsertionPoint == NULL) {
                        InsertionPoint = CurrentName.Buffer;
                    }
                }
            }

            //
            // If we found an insertion point, reset the string
            // pointer back to it.
            //

            if (InsertionPoint != NULL) {
                p = InsertionPoint;
            }

            //
            // If variable name not found and a new value parameter was specified
            // then insert the new variable name and its value at the appropriate
            // place in the environment variable block (i.e. where p points to).
            //

            if (pEnd == NULL && ARGUMENT_PRESENT( Value )) {
                if (p != NULL) {
                    //
                    // Name not found.  Now find the end of the current
                    // environment variable block.
                    //

                    pEnd = p;
                    while (*pEnd) {
                        while (*pEnd++) {
                        }
                    }
                    pEnd++;

                    //
                    // New value is present, so query the current size of the
                    // environment variable block.  Return status if failure.
                    //

                    Status = ZwQueryVirtualMemory( NtCurrentProcess(),
                                                   pOld,
                                                   MemoryBasicInformation,
                                                   &MemoryInformation,
                                                   sizeof( MemoryInformation ),
                                                   NULL
                                                 );
                    if (!NT_SUCCESS( Status )) {
                        leave;
                    }

                    //
                    // See if there is room for new, larger value.  If not
                    // allocate a new copy of the environment variable
                    // block.
                    //

                    NewSize = (pEnd - (PWSTR)pOld) * sizeof(WCHAR) +
                              Name->Length +
                              sizeof(WCHAR) +
                              Value->Length +
                              sizeof(WCHAR);
                } else {
                    NewSize = Name->Length +
                              sizeof(WCHAR) +
                              Value->Length +
                              sizeof(WCHAR);
                    MemoryInformation.RegionSize = 0;
                }

                if (NewSize >= MemoryInformation.RegionSize) {
                    //
                    // Allocate memory to contain a copy of the current
                    // process's environment variable block.  Return
                    // status if failure.
                    //

                    Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                                      &pNew,
                                                      0,
                                                      &NewSize,
                                                      MEM_COMMIT,
                                                      PAGE_READWRITE
                                                    );
                    if (!NT_SUCCESS( Status )) {
                        leave;
                    }

                    //
                    // Copy the current process's environment to the allocated memory
                    // inserting the new value as we do the copy.
                    //

                    if (p != NULL) {
                        Size = (ULONG)(p - (PWSTR)pOld);
                        RtlMoveMemory( pNew, pOld, Size*sizeof(WCHAR) );
                    } else {
                        Size = 0;
                    }
                    pStart = (PWSTR)pNew + Size;
                    RtlMoveMemory( pStart, Name->Buffer, Name->Length );
                    pStart += Name->Length/sizeof(WCHAR);
                    *pStart++ = L'=';
                    RtlMoveMemory( pStart, Value->Buffer, Value->Length );
                    pStart += Value->Length/sizeof(WCHAR);
                    *pStart++ = L'\0';
                    if (p != NULL) {
                        RtlMoveMemory( pStart, p,(ULONG)((pEnd - p)*sizeof(WCHAR)) );
                    }

                    if (ARGUMENT_PRESENT( Environment )) {
    		            *Environment = pNew;
                    } else {
    		            ProcessParameters->Environment = pNew;
#if defined(BUILD_WOW6432)
                        ((PRTL_USER_PROCESS_PARAMETERS64)(NtCurrentPeb64()->ProcessParameters))->Environment = (ULONGLONG)(ULONG)pNew;
#endif
                        Peb->EnvironmentUpdateCount += 1;
                    }

                    ZwFreeVirtualMemory (NtCurrentProcess(),
                                         &pOld,
                                         &MemoryInformation.RegionSize,
                                         MEM_RELEASE);
                } else {
                    pStart = p + Name->Length/sizeof(WCHAR) + 1 + Value->Length/sizeof(WCHAR) + 1;
                    RtlMoveMemory( pStart, p,(ULONG)((pEnd - p)*sizeof(WCHAR)) );
                    RtlMoveMemory( p, Name->Buffer, Name->Length );
                    p += Name->Length/sizeof(WCHAR);
                    *p++ = L'=';
                    RtlMoveMemory( p, Value->Buffer, Value->Length );
                    p += Value->Length/sizeof(WCHAR);
                    *p++ = L'\0';
                }
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
              //
              // If abnormally terminating, assume access violation.
              //

              Status = STATUS_ACCESS_VIOLATION;
        }
    } finally {
        //
        // Release the Peb lock.
        //

        if (!ARGUMENT_PRESENT( Environment )) {
            RtlReleasePebLock();
        }
    }

    //
    // Return status.
    //

    return( Status );
}

NTSTATUS
NTAPI
RtlSetEnvironmentStrings(
    IN PWCHAR NewEnvironment,
    IN SIZE_T NewEnvironmentSize
    )
/*++

Routine Description:

    This routine allows the replacement of the current environment block with a new one.

Arguments:

    NewEnvironment - Pointer to a set of zero terminated strings terminated by two terminators

    NewEnvironmentSize - Size of the block to put in place in bytes

Return Value:

    NTSTATUS - Status of function call

--*/
{
    PPEB Peb;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PVOID pOld, pNew;
    NTSTATUS Status, Status1;
    SIZE_T NewSize, OldSize;
    MEMORY_BASIC_INFORMATION MemoryInformation;


    //
    // Assert if the block is not well formed
    //
    ASSERT (NewEnvironmentSize > sizeof (WCHAR) * 2);
    ASSERT ((NewEnvironmentSize & (sizeof (WCHAR) - 1)) == 0);
    ASSERT (NewEnvironment[NewEnvironmentSize/sizeof(WCHAR)-1] == L'\0');
    ASSERT (NewEnvironment[NewEnvironmentSize/sizeof(WCHAR)-2] == L'\0');

    Peb = NtCurrentPeb ();

    ProcessParameters = Peb->ProcessParameters;

    RtlAcquirePebLock ();

    pOld = ProcessParameters->Environment;

    Status = ZwQueryVirtualMemory (NtCurrentProcess (),
                                   pOld,
                                   MemoryBasicInformation,
                                   &MemoryInformation,
                                   sizeof (MemoryInformation),
                                   NULL);
    if (!NT_SUCCESS (Status)) {
        goto unlock_and_exit;
    }

    if (MemoryInformation.RegionSize >= NewEnvironmentSize) {
        RtlpEnvironCacheValid = FALSE;
        RtlCopyMemory (pOld, NewEnvironment, NewEnvironmentSize);
        Status = STATUS_SUCCESS;
        goto unlock_and_exit;
    }

    //
    // Drop the lock around expensive operations
    //

    RtlReleasePebLock ();

    pOld = NULL;
    pNew = NULL;

    NewSize = NewEnvironmentSize;

    Status = ZwAllocateVirtualMemory (NtCurrentProcess (),
                                      &pNew,
                                      0,
                                      &NewSize,
                                      MEM_COMMIT,
                                      PAGE_READWRITE);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    //
    // Fill in the new block.
    //
    RtlCopyMemory (pNew, NewEnvironment, NewEnvironmentSize);

    //
    // Reacquire the lock. The existing block may have been reallocated
    // and so may now be big enough. Ignore this and use the block we
    // have created anyway.
    //

    RtlAcquirePebLock ();

    pOld = ProcessParameters->Environment;

    ProcessParameters->Environment = pNew;

#if defined(BUILD_WOW6432)
    ((PRTL_USER_PROCESS_PARAMETERS64)(NtCurrentPeb64()->ProcessParameters))->Environment = (ULONGLONG)(ULONG)pNew;
#endif

    RtlpEnvironCacheValid = FALSE;

    RtlReleasePebLock ();


    //
    // Release the old block.
    //

    OldSize = 0;

    Status1 = ZwFreeVirtualMemory (NtCurrentProcess(),
                                   &pOld,
                                   &OldSize,
                                   MEM_RELEASE);

    ASSERT (NT_SUCCESS (Status1));

    return STATUS_SUCCESS;


unlock_and_exit:;
    RtlReleasePebLock ();
    return Status;
}

