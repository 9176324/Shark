/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    stktrace.c

Abstract:

    This module implements routines to snapshot a set of stack back traces
    in a data base.  Useful for heap allocators to track allocation requests
    cheaply.

--*/

#include <ntos.h>
#include <ntrtl.h>
#include "ntrtlp.h"
#include <nturtl.h>
#include <zwapi.h>
#include <stktrace.h>

//
// Number of buckets used for the simple chaining hash table.
//

#define NUMBER_OF_BUCKETS 1567

//
// Macros to hide the different synchronization routines for
// user mode and kernel mode runtimes. For kernel runtime
// the OKAY_TO_LOCK macro points to a real function that makes
// sure current thread is not executing a DPC routine.
//

typedef struct _KSPIN_LOCK_EX {

    KSPIN_LOCK Lock;
    KIRQL OldIrql;
    PKTHREAD Owner;

} KSPIN_LOCK_EX, *PKSPIN_LOCK_EX;

NTSTATUS
KeInitializeSpinLockEx (
    PKSPIN_LOCK_EX Lock
    )
{
    KeInitializeSpinLock (&(Lock->Lock));
    Lock->OldIrql = 0;
    Lock->Owner = NULL;
    return STATUS_SUCCESS;
}

VOID
KeAcquireSpinLockEx (
    PKSPIN_LOCK_EX Lock
    )
{
    KeAcquireSpinLock (&(Lock->Lock), &(Lock->OldIrql));
    Lock->Owner = KeGetCurrentThread();
}

VOID
KeReleaseSpinLockEx (
    PKSPIN_LOCK_EX Lock
    )
{
    Lock->Owner = NULL;
    KeReleaseSpinLock (&(Lock->Lock), (Lock->OldIrql));
}

#define INITIALIZE_DATABASE_LOCK(R) KeInitializeSpinLockEx((PKSPIN_LOCK_EX)R)
#define ACQUIRE_DATABASE_LOCK(R) KeAcquireSpinLockEx((PKSPIN_LOCK_EX)R)
#define RELEASE_DATABASE_LOCK(R) KeReleaseSpinLockEx((PKSPIN_LOCK_EX)R)
#define OKAY_TO_LOCK_DATABASE(R) ExOkayToLockRoutine(&(((PKSPIN_LOCK_EX)R)->Lock))

BOOLEAN
ExOkayToLockRoutine (
    IN PVOID Lock
    );

//
// Globals from elsewhere referred here.
//

extern BOOLEAN RtlpFuzzyStackTracesEnabled;

//
// Forward declarations of private functions.
//

USHORT
RtlpLogStackBackTraceEx(
    ULONG FramesToSkip
    );

LOGICAL
RtlpCaptureStackTraceForLogging (
    PRTL_STACK_TRACE_ENTRY Trace,
    PULONG Hash,
    ULONG FramesToSkip,
    LOGICAL UserModeStackFromKernelMode
    );

USHORT
RtlpLogCapturedStackTrace(
    PRTL_STACK_TRACE_ENTRY Trace,
    ULONG Hash
    );

PRTL_STACK_TRACE_ENTRY
RtlpExtendStackTraceDataBase(
    IN PRTL_STACK_TRACE_ENTRY InitialValue,
    IN SIZE_T Size
    );

//
// Global per process (user mode) or system wide (kernel mode) 
// stack trace database.
//

PSTACK_TRACE_DATABASE RtlpStackTraceDataBase;

//
// Resource used to control access to stack trace database. We opted for 
// this solution so that we kept change in the database structure to an 
// absolute minimal. This way the tools that depend on this structure
// (at least umhd and oh) will not need a new version and will not
// introduce backcompatibility issues.
//

KSPIN_LOCK_EX RtlpStackTraceDataBaseLock;

/////////////////////////////////////////////////////////////////////
//////////////////////////////////////// Runtime stack trace database
/////////////////////////////////////////////////////////////////////

//
// The following section implements a trace database used to store
// stack traces captured with RtlCaptureStackBackTrace(). The database
// is implemented as a hash table and does not allow deletions. It is
// sensitive to "garbage" in the sense that spurious garbage (partially
// correct stacks) will hash in different buckets and will tend to fill
// the whole table. This is a problem only on x86 if "fuzzy" stack traces
// are used. The typical function used to log the trace is
// RtlLogStackBackTrace. One of the worst limitations of this package
// is that traces are referred using a ushort index which means we cannot
// ever store more than 65535 traces (remember we never delete traces).
//

PSTACK_TRACE_DATABASE
RtlpAcquireStackTraceDataBase(
    )
{
    PSTACK_TRACE_DATABASE DataBase;

    DataBase = RtlpStackTraceDataBase;

    //
    // Sanity checks.
    //

    if (DataBase == NULL) {
        return NULL;
    }

    if (! OKAY_TO_LOCK_DATABASE (DataBase->Lock)) {
        return NULL;
    }

    ACQUIRE_DATABASE_LOCK (DataBase->Lock);

    if (DataBase->DumpInProgress) {
        
        RELEASE_DATABASE_LOCK (DataBase->Lock);
        return NULL;
    }
    else {

        return DataBase;
    }
}


VOID
RtlpReleaseStackTraceDataBase(
    )
{
    PSTACK_TRACE_DATABASE DataBase;

    DataBase = RtlpStackTraceDataBase;
    
    //
    // Sanity checks.
    //

    if (DataBase == NULL) {
        return;
    }
    
    RELEASE_DATABASE_LOCK (DataBase->Lock);
}


NTSTATUS
RtlInitializeStackTraceDataBase(
    IN PVOID CommitBase,
    IN SIZE_T CommitSize,
    IN SIZE_T ReserveSize
    )
{
    NTSTATUS Status;
    PSTACK_TRACE_DATABASE DataBase;

    DataBase = (PSTACK_TRACE_DATABASE)CommitBase;
    
    if (CommitSize == 0) {

        //
        // Initially commit enough pages to accommodate the increased
        // number of hash chains (for improved performance we switched from ~100
        // to ~1000 in the hope that the hash chains will decrease ten-fold in 
        // length).
        //

        CommitSize = ROUND_TO_PAGES (NUMBER_OF_BUCKETS * sizeof (DataBase->Buckets[ 0 ]));
            
        Status = ZwAllocateVirtualMemory (NtCurrentProcess(),
                                          (PVOID *)&CommitBase,
                                          0,
                                          &CommitSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE);

        if (! NT_SUCCESS(Status)) {

            KdPrint (("RTL: Unable to commit space to extend stack "
                      "trace data base - Status = %lx\n",
                      Status));
            return Status;
        }

        DataBase->PreCommitted = FALSE;
    }
    else if (CommitSize == ReserveSize) {

        RtlZeroMemory (DataBase, sizeof( *DataBase ));

        DataBase->PreCommitted = TRUE;
    }
    else {
        
        return STATUS_INVALID_PARAMETER;
    }

    DataBase->CommitBase = CommitBase;
    DataBase->NumberOfBuckets = NUMBER_OF_BUCKETS; 
    DataBase->NextFreeLowerMemory = (PCHAR)(&DataBase->Buckets[ DataBase->NumberOfBuckets ]);
    DataBase->NextFreeUpperMemory = (PCHAR)CommitBase + ReserveSize;

    if (! DataBase->PreCommitted) {

        DataBase->CurrentLowerCommitLimit = (PCHAR)CommitBase + CommitSize;
        DataBase->CurrentUpperCommitLimit = (PCHAR)CommitBase + ReserveSize;
    }
    else {
        
        RtlZeroMemory (&DataBase->Buckets[ 0 ],
                       DataBase->NumberOfBuckets * sizeof (DataBase->Buckets[ 0 ]));
    }

    DataBase->EntryIndexArray = (PRTL_STACK_TRACE_ENTRY *)DataBase->NextFreeUpperMemory;

    //
    // Initialize the database lock.
    //

    DataBase->Lock = &RtlpStackTraceDataBaseLock;

    Status = INITIALIZE_DATABASE_LOCK (DataBase->Lock);

    if (! NT_SUCCESS(Status)) {
        
        KdPrint(("RTL: Unable to initialize stack trace database lock (status %X)\n", Status));
        return Status;
    }

    RtlpStackTraceDataBase = DataBase;

    return STATUS_SUCCESS;
}


PRTL_STACK_TRACE_ENTRY
RtlpExtendStackTraceDataBase(
    IN PRTL_STACK_TRACE_ENTRY InitialValue,
    IN SIZE_T Size
    )
/*++

Routine Description:

    This routine extends the stack trace database in order to accommodate
    the new stack trace that has to be saved.

Arguments:

    InitialValue - stack trace to be saved.

    Size - size of the stack trace in bytes. Note that this is not the
        depth of the trace but rather `Depth * sizeof(PVOID)'.

Return Value:

    The address of the just saved stack trace or null in case we have hit
    the maximum size of the database or we get commit errors.

Environment:

    User mode.

    Note. In order to make all this code work in kernel mode we have to
    rewrite this function that relies on VirtualAlloc.

--*/

{
    NTSTATUS Status;
    PRTL_STACK_TRACE_ENTRY p, *pp;
    SIZE_T CommitSize;
    PSTACK_TRACE_DATABASE DataBase;

    DataBase = RtlpStackTraceDataBase;

    //
    // We will try to find space for one stack trace entry in the
    // upper part of the database.
    //

    pp = (PRTL_STACK_TRACE_ENTRY *)DataBase->NextFreeUpperMemory;

    if ((! DataBase->PreCommitted) &&
        ((PCHAR)(pp - 1) < (PCHAR)DataBase->CurrentUpperCommitLimit)) {

        //
        // No more committed space in the upper part of the database.
        // We need to extend it downwards.
        //

        DataBase->CurrentUpperCommitLimit =
            (PVOID)((PCHAR)DataBase->CurrentUpperCommitLimit - PAGE_SIZE);

        if (DataBase->CurrentUpperCommitLimit < DataBase->CurrentLowerCommitLimit) {

            //
            // No more space at all. We have got over the lower part of the db.
            // We failed therefore increase back the UpperCommitLimit pointer.
            //

            DataBase->CurrentUpperCommitLimit =
                (PVOID)((PCHAR)DataBase->CurrentUpperCommitLimit + PAGE_SIZE);

            return( NULL );
        }

        CommitSize = PAGE_SIZE;
        Status = ZwAllocateVirtualMemory(
            NtCurrentProcess(),
            (PVOID *)&DataBase->CurrentUpperCommitLimit,
            0,
            &CommitSize,
            MEM_COMMIT,
            PAGE_READWRITE
            );

        if (!NT_SUCCESS( Status )) {

            //
            // We tried to increase the upper part of the db by one page.
            // We failed therefore increase back the UpperCommitLimit pointer
            //

            DataBase->CurrentUpperCommitLimit =
                (PVOID)((PCHAR)DataBase->CurrentUpperCommitLimit + PAGE_SIZE);

            return NULL;
        }
    }

    //
    // We managed to make sure we have usable space in the upper part
    // therefore we take out one stack trace entry address.
    //

    DataBase->NextFreeUpperMemory -= sizeof( *pp );

    //
    // Now we will try to find space in the lower part of the database for
    // for the actual stack trace.
    //

    p = (PRTL_STACK_TRACE_ENTRY)DataBase->NextFreeLowerMemory;

    if ((! DataBase->PreCommitted) &&
        (((PCHAR)p + Size) > (PCHAR)DataBase->CurrentLowerCommitLimit)) {

        //
        // We need to extend the lower part.
        //

        if (DataBase->CurrentLowerCommitLimit >= DataBase->CurrentUpperCommitLimit) {

            //
            // We have hit the maximum size of the database.
            //

            DataBase->NextFreeUpperMemory += sizeof( *pp );
            return( NULL );
        }

        //
        // Extend the lower part of the database by one page.
        //

        CommitSize = Size;
        Status = ZwAllocateVirtualMemory(
            NtCurrentProcess(),
            (PVOID *)&DataBase->CurrentLowerCommitLimit,
            0,
            &CommitSize,
            MEM_COMMIT,
            PAGE_READWRITE
            );

        if (! NT_SUCCESS( Status )) {
            
            DataBase->NextFreeUpperMemory += sizeof( *pp );
            return NULL;
        }

        DataBase->CurrentLowerCommitLimit =
            (PCHAR)DataBase->CurrentLowerCommitLimit + CommitSize;
    }

    //
    // Take out the space for the stack trace.
    //

    DataBase->NextFreeLowerMemory += Size;

    //
    // Deal with a precommitted database case. If the lower and upper
    // pointers have crossed each other then rollback and return failure.
    //

    if (DataBase->PreCommitted &&
        DataBase->NextFreeLowerMemory >= DataBase->NextFreeUpperMemory) {

        DataBase->NextFreeUpperMemory += sizeof( *pp );
        DataBase->NextFreeLowerMemory -= Size;
        return( NULL );
    }

    //
    // Save the stack trace in the database
    //

    RtlMoveMemory( p, InitialValue, Size );
    p->HashChain = NULL;
    p->TraceCount = 0;
    p->Index = (USHORT)(++DataBase->NumberOfEntriesAdded);

    //
    // Save the address of the new stack trace entry in the
    // upper part of the databse.
    //

    *--pp = p;

    //
    // Return address of the saved stack trace entry.
    //

    return( p );
}


#pragma optimize("y", off) // disable FPO
USHORT
RtlLogStackBackTrace(
    VOID
    )
/*++

Routine Description:

    This routine will capture the current stacktrace (skipping the
    present function) and will save it in the global (per process)
    stack trace database. It should be noted that we do not save
    duplicate traces.

Arguments:

    None.

Return Value:

    Index of the stack trace saved. The index can be used by tools
    to access quickly the trace data. This is the reason at the end of
    the database we save downwards a list of pointers to trace entries.
    This index can be used to find this pointer in constant time.

    A zero index will be returned for error conditions (e.g. stack
    trace database not initialized).

Environment:

    User mode.

--*/

{
    return RtlpLogStackBackTraceEx (1);
}


#pragma optimize("y", off) // disable FPO
USHORT
RtlpLogStackBackTraceEx(
    ULONG FramesToSkip
    )
/*++

Routine Description:

    This routine will capture the current stacktrace (skipping the
    present function) and will save it in the global (per process)
    stack trace database. It should be noted that we do not save
    duplicate traces.

Arguments:

    FramesToSkip - no of frames that are not interesting and 
        should be skipped.

Return Value:

    Index of the stack trace saved. The index can be used by tools
    to access quickly the trace data. This is the reason at the end of
    the database we save downwards a list of pointers to trace entries.
    This index can be used to find this pointer in constant time.

    A zero index will be returned for error conditions (e.g. stack
    trace database not initialized).

Environment:

    User mode.

--*/

{
    RTL_STACK_TRACE_ENTRY Trace;
    USHORT TraceIndex;
    NTSTATUS Status;
    ULONG Hash;
    PSTACK_TRACE_DATABASE DataBase;

    //
    // Check the context in which we are running.
    //

    DataBase = RtlpStackTraceDataBase;

    if (DataBase == NULL) {
        return 0;
    }

    if (! OKAY_TO_LOCK_DATABASE (DataBase->Lock)) {
        return 0;
    }

    //
    // Capture stack trace. 
    //

    if (RtlpCaptureStackTraceForLogging (&Trace, &Hash, FramesToSkip + 1, FALSE) == FALSE) {
        return 0;
    }
    
    //
    // Add the trace if it is not already there.
    // Return trace index.
    //

    TraceIndex = RtlpLogCapturedStackTrace (&Trace, Hash);

    return TraceIndex;
}


#pragma optimize("y", off) // disable FPO
USHORT
RtlLogUmodeStackBackTrace(
    VOID
    )
/*++

Routine Description:

    This routine will capture the user mode stacktrace and will save 
    it in the global (per system) stack trace database. 
    It should be noted that we do not save duplicate traces.

Arguments:

    None.

Return Value:

    Index of the stack trace saved. The index can be used by tools
    to access quickly the trace data. This is the reason at the end of
    the database we save downwards a list of pointers to trace entries.
    This index can be used to find this pointer in constant time.

    A zero index will be returned for error conditions (e.g. stack
    trace database not initialized).

Environment:

    User mode.

--*/

{
    RTL_STACK_TRACE_ENTRY Trace;
    ULONG Hash;

    //
    // No database => nothing to do.
    //

    if (RtlpStackTraceDataBase == NULL) {
        return 0;
    }

    //
    // Capture user mode stack trace. 
    //

    if (RtlpCaptureStackTraceForLogging (&Trace, &Hash, 1, TRUE) == FALSE) {
        return 0;
    }
    
    //
    // Add the trace if it is not already there.
    // Return trace index.
    //

    return RtlpLogCapturedStackTrace (&Trace, Hash);
}


#pragma optimize("y", off) // disable FPO
LOGICAL
RtlpCaptureStackTraceForLogging (
    PRTL_STACK_TRACE_ENTRY Trace,
    PULONG Hash,
    ULONG FramesToSkip,
    LOGICAL UserModeStackFromKernelMode
    )
{
    if (UserModeStackFromKernelMode == FALSE) {
        
        //
        // Capture stack trace. The try/except was useful
        // in the old days when the function did not validate
        // the stack frame chain. We keep it just to be defensive.
        //

        try {

            Trace->Depth = RtlCaptureStackBackTrace (FramesToSkip + 1,
                                                    MAX_STACK_DEPTH,
                                                    Trace->BackTrace,
                                                    Hash);
        }
        except(EXCEPTION_EXECUTE_HANDLER) {

            Trace->Depth = 0;
        }

        if (Trace->Depth == 0) {

            return FALSE;
        }
        else {

            return TRUE;
        }
    }
    else {

        ULONG Index;

        //
        // Avoid weird situations.
        //

        if (KeAreAllApcsDisabled () == TRUE) {
            return FALSE;
        }

        //
        // Capture user mode stack trace and hash value.
        //

        Trace->Depth = (USHORT) RtlWalkFrameChain(Trace->BackTrace,
                                                  MAX_STACK_DEPTH,
                                                  1);
        if (Trace->Depth == 0) {
            
            return FALSE;
        }
        else {

            *Hash = 0;

            for (Index = 0; Index < Trace->Depth; Index += 1) {
                 *Hash += PtrToUlong (Trace->BackTrace[Index]);
            }

            return TRUE;
        }
    }
}


USHORT
RtlpLogCapturedStackTrace(
    PRTL_STACK_TRACE_ENTRY Trace,
    ULONG Hash
    )
{
    PSTACK_TRACE_DATABASE DataBase;
    PRTL_STACK_TRACE_ENTRY p, *pp;
    ULONG RequestedSize, DepthSize;
    USHORT ReturnValue;

    DataBase = RtlpStackTraceDataBase;

    //
    // Update statistics counters. Since they are used only for reference and do not
    // control decisions we increment them without protection even if this means we may
    // have numbers slightly out of sync.
    //

    DataBase->NumberOfEntriesLookedUp += 1;

    //
    // Lock the global per-process stack trace database.
    //

    if (RtlpAcquireStackTraceDataBase() == NULL) {

        //
        // Fail the log operation if we cannot acquire the lock.
        // This can happen only if there is a dump in progress or we are in
        // an invalid context (process shutdown (Umode) or DPC routine (Kmode).
        //

        return 0;
    }

    try {

        //
        // We will try to find out if the trace has been saved in the past.
        // We find the right hash chain and then traverse it.
        //

        DepthSize = Trace->Depth * sizeof (Trace->BackTrace[0]);

        pp = &DataBase->Buckets[ Hash % DataBase->NumberOfBuckets ];

        while (p = *pp) {

            if (p->Depth == Trace->Depth) {

                if (RtlCompareMemory( &p->BackTrace[ 0 ], &Trace->BackTrace[ 0 ], DepthSize) == DepthSize) {
                    
                    break;
                }
            }

            pp = &p->HashChain;
        }

        if (p == NULL) {

            //
            // If we get here we did not find a similar trace in the database. We need
            // to add it.
            //
            // We got the `*pp' value (address of last chain element) while the 
            // database lock was acquired shared so we need to take into consideration 
            // the case where another thread managed to acquire database exclusively 
            // and add a new trace at the end of the chain. Therefore if `*pp' is no longer
            // null we continue to traverse the chain until we get to the end.
            //

            p = NULL;

            if (*pp != NULL) {

                //
                // Somebody added some traces at the end of the chain while we
                // were trying to convert the lock from shared to exclusive.
                //

                while (p = *pp) {

                    if (p->Depth == Trace->Depth) {

                        if (RtlCompareMemory( &p->BackTrace[ 0 ], &Trace->BackTrace[ 0 ], DepthSize) == DepthSize) {

                            break;
                        }
                    }

                    pp = &p->HashChain;
                }
            }

            if (p == NULL) {
                
                //
                // Nobody added the trace and now `*pp' really points to the end
                // of the chain either because we traversed the rest of the chain
                // or it was at the end anyway.
                //

                RequestedSize = FIELD_OFFSET (RTL_STACK_TRACE_ENTRY, BackTrace) + DepthSize;

                p = RtlpExtendStackTraceDataBase (Trace, RequestedSize);

                if (p != NULL) {

                    //
                    // We added the trace no chain it as the last element.
                    //

                    *pp = p;
                }
            }
            else {

                //
                // Some other thread managed to add the same trace to the database
                // while we were trying to acquire the lock exclusive. `p' has the
                // address to the stack trace entry.
                //
            }
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER) {

        //
        // We should never get here if the algorithm is correct.
        //

        p = NULL;
    }

    //
    // Release locks and return. At this stage we may return zero (failure)
    // if we did not manage to extend the database with a new trace (e.g. due to
    // out of memory conditions).
    //

    if (p != NULL) {

        p->TraceCount += 1;

        ReturnValue = p->Index;
    }
    else {
        
        ReturnValue = 0;
    }

    RtlpReleaseStackTraceDataBase();

    return ReturnValue;
}


PVOID
RtlpGetStackTraceAddress (
    USHORT Index
    )
{
    if (RtlpStackTraceDataBase == NULL) {
        return NULL;
    }

    if (! (Index > 0 && Index <= RtlpStackTraceDataBase->NumberOfEntriesAdded)) {
        return NULL;
    }

    return (PVOID)(RtlpStackTraceDataBase->EntryIndexArray[-Index]);
}


ULONG
RtlpWalkFrameChainExceptionFilter (
    ULONG ExceptionCode,
    PVOID ExceptionRecord
    )
/*++

Routine Description:

    This routine is the exception filter used by RtlWalkFrameChain function. 
    This function is platform specific (different code for x86, amd64, ia64) but
    the exception filter is common.

Arguments:

    ExceptionCode - exception code
    ExceptionRecord - structure with pointers to .exr and .cxr

Return Value:

    Always EXCEPTION_EXECUTE_HANDLER.

--*/
{

#if DBG

    //
    // We skip STATUS_IN_PAGE because this exception can be raised by MM
    // while resolving a page fault. This can happen in an obscure corner 
    // case even if RtlWalkFrameChain called MmCanThreadFault.
    //

    if (ExceptionCode != STATUS_IN_PAGE_ERROR) {
        
        DbgPrint ("Unexpected exception while walking runtime stack (exception info @ %p). \n",
                  ExceptionRecord);

        DbgBreakPoint ();
    }

#endif

    return EXCEPTION_EXECUTE_HANDLER;
}

