/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    FsFilter.c

Abstract:

    This file contains the routines that show certain file system operations
    to file system filters.  File system filters were initially bypassed
    for these operations.

--*/

#include "FsRtlP.h"

#define FS_FILTER_MAX_COMPLETION_STACK_SIZE    30

typedef struct _FS_FILTER_RESERVE {

    //
    //  The thread that currently owns the memory.
    //

    PETHREAD Owner;

    //
    //  A stack of completion node bigger than anyone should ever need.
    //

    FS_FILTER_COMPLETION_NODE Stack [FS_FILTER_MAX_COMPLETION_STACK_SIZE];

} FS_FILTER_RESERVE, *PFS_FILTER_RESERVE;

//
//  Note: Events are used to synchronize access to the reserved pool here
//  because using a faster synchronization mechanism (like a FAST_MUTEX)
//  would cause us to raise IRQL to APC_LEVEL while we hold the lock.  This is
//  not acceptable because we call out to the filters while holding this lock
//  and we wouldn't want to be at APC_LEVEL during these calls.
//

KEVENT AcquireOpsEvent;
PFS_FILTER_RESERVE AcquireOpsReservePool;

KEVENT ReleaseOpsEvent;
PFS_FILTER_RESERVE ReleaseOpsReservePool;

NTSTATUS
FsFilterInit(
    )

/*++

Routine Description:

    This routine initializes the reserve pool the FsFilter routine need to use
    when the system is in low memory conditions.

Arguments:

    None.

Return Value:

    Returns STATUS_SUCCESS if the initialization was successful, or
    STATUS_INSUFFICIENT_RESOURCES otherwise.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    AcquireOpsReservePool = ExAllocatePoolWithTag( NonPagedPool,
                                                   sizeof( FS_FILTER_RESERVE ),
                                                   FSRTL_FILTER_MEMORY_TAG );

    if (AcquireOpsReservePool == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    ReleaseOpsReservePool = ExAllocatePoolWithTag( NonPagedPool,
                                                   sizeof( FS_FILTER_RESERVE ),
                                                   FSRTL_FILTER_MEMORY_TAG );

    if (ReleaseOpsReservePool == NULL) {

        ExFreePoolWithTag( AcquireOpsReservePool,
                           FSRTL_FILTER_MEMORY_TAG );
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }


    KeInitializeEvent( &AcquireOpsEvent, SynchronizationEvent, TRUE );
    KeInitializeEvent( &ReleaseOpsEvent, SynchronizationEvent, TRUE );

    return Status;
}

NTSTATUS
FsFilterAllocateCompletionStack (
    IN PFS_FILTER_CTRL FsFilterCtrl,
    IN BOOLEAN CanFail,
    OUT PULONG AllocationSize
    )

/*++

Routine Description:

    This routine allocates a completion stack for the given FsFilterCtrl.  If
    this allocation cannot fail, then this routine will wait to allocation
    the memory from the FsFilter reserved pool.

    This routine initialized the appropriate CompletionStack parameters and
    FsFilterCtrl flags to reflect the allocation made.

Arguments:

    FsFilterCtrl - The FsFilterCtrl structure for which the completion stack
        must be allocated.
    CanFail - TRUE if the allocation is allowed to fail, FALSE otherwise.
    AllocationSize - Set to the nuber of bytes of memory allocated for the
        completion stack for this FsFilterCtrl.

Return Value:

    Returns STATUS_SUCCESS if the memory was successfully allocated for the
    completion stack, or STATUS_INSUFFICIENT_RESOURCES otherwise.

--*/

{
    PFS_FILTER_COMPLETION_NODE Stack = NULL;
    PFS_FILTER_RESERVE ReserveBlock = NULL;
    PKEVENT Event = NULL;

    ASSERT( FsFilterCtrl != NULL );
    ASSERT( AllocationSize != NULL );

    *AllocationSize = FsFilterCtrl->CompletionStack.StackLength *
                      sizeof( FS_FILTER_COMPLETION_NODE );

    Stack = ExAllocatePoolWithTag( NonPagedPool,
                                   *AllocationSize,
                                   FSRTL_FILTER_MEMORY_TAG );

    if (Stack == NULL) {

        if (CanFail) {

            return STATUS_INSUFFICIENT_RESOURCES;

        } else {

            //
            //  This allocation cannot fail, so get the needed memory from our
            //  private stash of pool.
            //

            switch (FsFilterCtrl->Data.Operation) {

            case FS_FILTER_ACQUIRE_FOR_SECTION_SYNCHRONIZATION:
            case FS_FILTER_ACQUIRE_FOR_MOD_WRITE:
            case FS_FILTER_ACQUIRE_FOR_CC_FLUSH:

                ReserveBlock = AcquireOpsReservePool;
                Event = &AcquireOpsEvent;
                break;

            case FS_FILTER_RELEASE_FOR_SECTION_SYNCHRONIZATION:
            case FS_FILTER_RELEASE_FOR_MOD_WRITE:
            case FS_FILTER_RELEASE_FOR_CC_FLUSH:

                ReserveBlock = ReleaseOpsReservePool;
                Event = &ReleaseOpsEvent;
                break;

            default:

                //
                //  This shouldn't happen since we should always cover all
                //  possible types of operations in the above cases.
                //

                ASSERTMSG( "FsFilterAllocateMemory: Unknown operation type\n",
                           FALSE );
            }

            //
            //  Wait to get on the appropriate event so that we know the reserve
            //  memory is available for use.
            //

            KeWaitForSingleObject( Event,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL );

            //
            //  We've been signaled, so the reserved block is available.
            //

            ReserveBlock->Owner = PsGetCurrentThread();
            Stack = ReserveBlock->Stack;
            SetFlag( FsFilterCtrl->Flags, FS_FILTER_USED_RESERVE_POOL );
        }
    }

    ASSERT( Stack != NULL );

    //
    //  We've now got our block of memory, so initialize the completion stack.
    //

    SetFlag( FsFilterCtrl->Flags, FS_FILTER_ALLOCATED_COMPLETION_STACK );
    FsFilterCtrl->CompletionStack.Stack = Stack;

    return STATUS_SUCCESS;
}

VOID
FsFilterFreeCompletionStack (
    IN PFS_FILTER_CTRL FsFilterCtrl
    )

/*++

Routine Description:

    This routine frees the allocated completion stack in the FsFilterCtrl
    parameter.

Arguments:

    FsFilterCtrl - The FsFilterCtrl structure for which the completion stack
        must be freed.

Return Value:

    None.

--*/

{
    PKEVENT Event = NULL;
    PFS_FILTER_RESERVE ReserveBlock = NULL;

    ASSERT( FsFilterCtrl != NULL );
    ASSERT( FlagOn( FsFilterCtrl->Flags, FS_FILTER_ALLOCATED_COMPLETION_STACK ) );

    if (!FlagOn( FsFilterCtrl->Flags, FS_FILTER_USED_RESERVE_POOL )) {

        //
        //  We were able to allocate this from the generate pool of memory,
        //  so just free the memory block used for the completion stack.
        //

        ExFreePoolWithTag( FsFilterCtrl->CompletionStack.Stack,
                           FSRTL_FILTER_MEMORY_TAG );

    } else {

        //
        //  This allocation from our private pool stash, so use the operation
        //  to figure out which private stash.
        //

        switch (FsFilterCtrl->Data.Operation) {
        case FS_FILTER_ACQUIRE_FOR_SECTION_SYNCHRONIZATION:
        case FS_FILTER_ACQUIRE_FOR_MOD_WRITE:
        case FS_FILTER_ACQUIRE_FOR_CC_FLUSH:

            Event = &AcquireOpsEvent;
            break;

        case FS_FILTER_RELEASE_FOR_SECTION_SYNCHRONIZATION:
        case FS_FILTER_RELEASE_FOR_MOD_WRITE:
        case FS_FILTER_RELEASE_FOR_CC_FLUSH:

            Event = &ReleaseOpsEvent;
            break;

        default:

            //
            //  This shouldn't happen since we should always cover all
            //  possible types of operations in the above cases.
            //

            ASSERTMSG( "FsFilterAllocateMemory: Unknown operation type\n",
                       FALSE );
        }

        ASSERT( Event != NULL );

        //
        //  Clear out the owner of the reserve block before setting the event.
        //

        ReserveBlock = CONTAINING_RECORD( FsFilterCtrl->CompletionStack.Stack,
                                          FS_FILTER_RESERVE,
                                          Stack );
        ReserveBlock->Owner = NULL;

        //
        //  Now we are ready to release the reserved block to the next thread
        //  that needs it.
        //

        KeSetEvent( Event, IO_NO_INCREMENT, FALSE );
    }
}

NTSTATUS
FsFilterCtrlInit (
    IN OUT PFS_FILTER_CTRL FsFilterCtrl,
    IN UCHAR Operation,
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT BaseFsDeviceObject,
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN CanFail
    )

/*++

Routine Description:

    This routine initializes the FsFilterCtrl structure used to store the
    contexts and post-operation callbacks for this operation.  If the
    default completion stack is not large enough, this routine will need to
    allocate a completion stack of sufficient size to store all the possible
    contexts and post-operation callbacks.

Arguments:

    FsFilterCtrl - The FsFilterCtrl to initialize.
    Operation - The operation this FsFilterCtrl is going to be used for.
    DeviceObject - The device object to which this operation will be targeted.
    BaseFsDeviceObject - The device object for the base file system at the
        bottom on this filter stack.
    FileObject - The file object to which this operation will be targeted.
    CanFail - TRUE if the call can deal with memory allocations failing,
        FALSE otherwise.

Return Value:

    STATUS_SUCCESS if the FsFilterCtrl structure could be initialized,
    STATUS_INSUFFICIENT_RESOURCES if the routine cannot allocate the needed
    memory to initialize this structure.

--*/

{
    PFS_FILTER_CALLBACK_DATA Data;
    ULONG AllocationSize;
    NTSTATUS Status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( BaseFsDeviceObject );

    ASSERT( FsFilterCtrl != NULL );
    ASSERT( DeviceObject != NULL );

    FsFilterCtrl->Flags = 0;

    Data = &(FsFilterCtrl->Data);

    Data->SizeOfFsFilterCallbackData = sizeof( FS_FILTER_CALLBACK_DATA );
    Data->Operation = Operation;
    Data->DeviceObject = DeviceObject;
    Data->FileObject = FileObject;

    //
    //  Since it is possible for a filter to redirect this operation to another
    //  stack, we must assume that the stack size of their device object is
    //  large enough to account for the large stack they would need in the
    //  redirection.  It is the stack size of the top device object that we
    //  will use to determine the size of our completion stack.
    //

    FsFilterCtrl->CompletionStack.StackLength = DeviceObject->StackSize;
    FsFilterCtrl->CompletionStack.NextStackPosition = 0;

    if (FsFilterCtrl->CompletionStack.StackLength > FS_FILTER_DEFAULT_STACK_SIZE) {

        //
        //  The stack isn't big enough, so we must dynamically allocate
        //  the completion stack.  This should happen VERY rarely.
        //

        Status = FsFilterAllocateCompletionStack( FsFilterCtrl,
                                                  CanFail,
                                                  &AllocationSize );

        //
        //  If the above allocation failed and we cannot fail this allocation,
        //  use our private pool stash.
        //

        if (!NT_SUCCESS( Status )) {

            ASSERT( CanFail );
            return Status;
        }

        ASSERT( FsFilterCtrl->CompletionStack.Stack );

    } else {

        //
        //  The default completion noded array allocated for the stack
        //  is large enough, so set Stack to point to that array.
        //

        FsFilterCtrl->CompletionStack.Stack = &(FsFilterCtrl->CompletionStack.DefaultStack[0]);
        AllocationSize = sizeof( FS_FILTER_COMPLETION_NODE ) * FS_FILTER_DEFAULT_STACK_SIZE;
        FsFilterCtrl->CompletionStack.StackLength = FS_FILTER_DEFAULT_STACK_SIZE;
    }

    RtlZeroMemory( FsFilterCtrl->CompletionStack.Stack, AllocationSize );

    return Status;
}

VOID
FsFilterCtrlFree (
    IN PFS_FILTER_CTRL FsFilterCtrl
    )

/*++

Routine Description:

    This routine frees any memory associated FsFilterCtrl.  It is possible
    that we had to allocate more memory to deal with a stack that is larger
    than the FS_FILTER_DEFAULT_STACK_SIZE.

Arguments:

    FsFilterCtrl - The FsFilterCtrl structure to free.

Return Value:

    NONE

--*/

{
    ASSERT( FsFilterCtrl != NULL );

    ASSERT( FsFilterCtrl->CompletionStack.Stack != NULL );

    if (FlagOn( FsFilterCtrl->Flags, FS_FILTER_ALLOCATED_COMPLETION_STACK )) {

        FsFilterFreeCompletionStack( FsFilterCtrl );
    }
}

VOID
FsFilterGetCallbacks (
    IN UCHAR Operation,
    IN PDEVICE_OBJECT DeviceObject,
    OUT PFS_FILTER_CALLBACK *PreOperationCallback,
    OUT PFS_FILTER_COMPLETION_CALLBACK *PostOperationCallback
    )

/*++

Routine Description:

    This routine looks up the PreOperationCallback and the PostOperationCallback
    that the filter has registered for this operation if it has registered one.
Arguments:

    Operation - The current operation of interest.
    DeviceObject - The device object that the filter attached to the file system
        filter stack.
    PreOperationCallback - Set to the PreOperationCallback that the filter
        registered for this operation if one was registered.  Otherwise, this
        is set to NULL.
    PostOperationCallback - Set to the PostOperationCallback that the filter
        registered for this operation if one was registered.  Otherwise, this
        is set to NULL.

Return Value:

    NONE

--*/

{

    PFS_FILTER_CALLBACKS FsFilterCallbacks;

    //
    //  Initialize the pre and post callbacks to NULL.  If
    //  we have valid callbacks, these output parameters will
    //  get set to the appropriate function pointers.
    //

    *PreOperationCallback = NULL;
    *PostOperationCallback = NULL;

    FsFilterCallbacks =
        DeviceObject->DriverObject->DriverExtension->FsFilterCallbacks;

    if (FsFilterCallbacks == NULL) {

        //
        //  This filter didn't register any callbacks,
        //  so just return and save switch logic that follows.
        //

        return;
    }

    //
    //  This device did register at least some callbacks, so see
    //  if there are callbacks for the current operation.
    //

    switch (Operation) {

    case FS_FILTER_ACQUIRE_FOR_SECTION_SYNCHRONIZATION:

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PreAcquireForSectionSynchronization )) {

            *PreOperationCallback = FsFilterCallbacks->PreAcquireForSectionSynchronization;

        }

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PostAcquireForSectionSynchronization)) {

            *PostOperationCallback = FsFilterCallbacks->PostAcquireForSectionSynchronization;

        }

        break;

    case FS_FILTER_RELEASE_FOR_SECTION_SYNCHRONIZATION:

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PreReleaseForSectionSynchronization )) {

            *PreOperationCallback = FsFilterCallbacks->PreReleaseForSectionSynchronization;
        }

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PostReleaseForSectionSynchronization )) {

            *PostOperationCallback = FsFilterCallbacks->PostReleaseForSectionSynchronization;
        }

        break;

    case FS_FILTER_ACQUIRE_FOR_MOD_WRITE:

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PreAcquireForModifiedPageWriter )) {

            *PreOperationCallback = FsFilterCallbacks->PreAcquireForModifiedPageWriter;
        }

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PostAcquireForModifiedPageWriter )) {

            *PostOperationCallback = FsFilterCallbacks->PostAcquireForModifiedPageWriter;
        }

        break;

    case FS_FILTER_RELEASE_FOR_MOD_WRITE:

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PreReleaseForModifiedPageWriter )) {

            *PreOperationCallback = FsFilterCallbacks->PreReleaseForModifiedPageWriter;
        }

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PostReleaseForModifiedPageWriter )) {

            *PostOperationCallback = FsFilterCallbacks->PostReleaseForModifiedPageWriter;
        }

        break;

    case FS_FILTER_ACQUIRE_FOR_CC_FLUSH:

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PreAcquireForCcFlush )) {

            *PreOperationCallback = FsFilterCallbacks->PreAcquireForCcFlush;
        }

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PostAcquireForCcFlush )) {

            *PostOperationCallback = FsFilterCallbacks->PostAcquireForCcFlush;
        }

         break;

    case FS_FILTER_RELEASE_FOR_CC_FLUSH:

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PreReleaseForCcFlush )) {

            *PreOperationCallback = FsFilterCallbacks->PreReleaseForCcFlush;
        }

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PostReleaseForCcFlush )) {

            *PostOperationCallback = FsFilterCallbacks->PostReleaseForCcFlush;
        }

        break;

    default:

        ASSERT( FALSE );
        *PreOperationCallback = NULL;
        *PostOperationCallback = NULL;
    }
}

NTSTATUS
FsFilterPerformCallbacks (
    IN PFS_FILTER_CTRL FsFilterCtrl,
    IN BOOLEAN AllowFilterToFail,
    IN BOOLEAN AllowBaseFsToFail,
    OUT BOOLEAN *BaseFsFailedOp
    )

/*++

Routine Description:

    This routine calls all the file system filters that have registered
    to see the operation described by the FsFilterCtrl.  If this
    routine returns a successful Status, the operation should be
    passed onto the base file system.  If an error Status is returned,
    the caller is responsible for call FsFilterPerformCompletionCallbacks
    to unwind any post-operations that need to be called.

Arguments:

    FsFilterCtrl - The structure describing the control information
        needed to pass this operation to each filter registered to
        see this operation.

    AllowFilterToFail - TRUE if the filter is allowed to fail this
        operation, FALSE otherwise.

    AllowBaseFsToFail - TRUE if the base file system is allowed to fail this
        operation, FALSE otherwise.

    BaseFsFailedOp - Set to TRUE if the base file system failed
        this operation, FALSE, otherwise.

Return Value:

    STATUS_SUCCESS - All filters that are interested saw the operation
        and none failed this operation.

    STATUS_INSUFFICIENT_RESOURCES - There is not enough memory
        to allocate the completion node, so this operation
        is failing.

    Other error Status - Could be returned from a filter's
        pre-operation callback if it wants to fail this operation.
*/

{
    PFS_FILTER_CALLBACK_DATA Data = &(FsFilterCtrl->Data);
    PFS_FILTER_COMPLETION_STACK CompletionStack = &(FsFilterCtrl->CompletionStack);
    PFS_FILTER_CALLBACK PreOperationCallback;
    PFS_FILTER_COMPLETION_CALLBACK PostOperationCallback;
    PFS_FILTER_COMPLETION_NODE CompletionNode;
    PDEVICE_OBJECT CurrentDeviceObject;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN isFilter = TRUE;

    //
    //  We should never be in the scenario where a filter can fail the operation
    //  but the base file system cannot.
    //

    ASSERT( !(AllowFilterToFail && !AllowBaseFsToFail) );

    //
    //  Initialize output parameters if present.
    //

    *BaseFsFailedOp = FALSE;

    //
    //  As we iterate through the device objects, we use the local
    //  CurrentDeviceObject to iterate through the list because we want
    //  Data->DeviceObject to be set to the last device object when we are
    //  finished iterating.
    //

    CurrentDeviceObject = Data->DeviceObject;

    while (CurrentDeviceObject != NULL) {

        //
        //  First remember if this device object represents a filter or a file
        //  system.
        //

        if (CurrentDeviceObject->DeviceObjectExtension->AttachedTo != NULL) {

            isFilter = TRUE;

        } else {

            isFilter = FALSE;
        }

        //
        //  Now get the callbacks for this device object
        //

        Data->DeviceObject = CurrentDeviceObject;

        FsFilterGetCallbacks( Data->Operation,
                              Data->DeviceObject,
                              &PreOperationCallback,
                              &PostOperationCallback );

        //
        //  If this device object has either a callback or completion callback
        //  for this operation, allocate a CompletionNode for it.
        //

        if ((PreOperationCallback == NULL) && (PostOperationCallback == NULL)) {

            //
            //  This device object does not have any callbacks for this operation
            //  so move onto the next device.
            //

            CurrentDeviceObject = Data->DeviceObject->DeviceObjectExtension->AttachedTo;
            CompletionNode = NULL;
            continue;

        } else if (PostOperationCallback != NULL) {

            //
            //  Since there is a PostOperationCallback, we will need to allocate
            //  a CompletionNode for this device.
            //

            CompletionNode = PUSH_COMPLETION_NODE( CompletionStack );

            if (CompletionNode == NULL) {

                //
                //  This case shouldn't happen since we should ensure
                //  that our completion stack is large enough when
                //  we first saw this operation.
                //

                if (!AllowFilterToFail) {

                    //
                    //  We cannot fail this operation, so bugcheck.
                    //

                    KeBugCheckEx( FILE_SYSTEM, 0, 0, 0, 0 );
                }

                return STATUS_INSUFFICIENT_RESOURCES;

            } else {

                CompletionNode->DeviceObject = Data->DeviceObject;
                CompletionNode->FileObject = Data->FileObject;
                CompletionNode->CompletionContext = NULL;
                CompletionNode->CompletionCallback = PostOperationCallback;
            }

        } else {

            //
            //  We just have a pre-operation, so just set the CompletionNode to
            //  NULL.
            //

            CompletionNode = NULL;
        }

        if (PreOperationCallback != NULL) {

            if (CompletionNode == NULL) {

                Status = PreOperationCallback( Data,
                                               NULL );

            } else {

                Status = PreOperationCallback( Data,
                                               &(CompletionNode->CompletionContext) );
            }

            ASSERT( Status == STATUS_SUCCESS ||
                    Status == STATUS_FSFILTER_OP_COMPLETED_SUCCESSFULLY ||
                    !NT_SUCCESS( Status ) );

            if (Status == STATUS_FSFILTER_OP_COMPLETED_SUCCESSFULLY) {

                //
                //  The filter/file system completed the operation successfully, so just
                //  return the completion status.
                //

                //
                //  If we allocated a completion node for this device,
                //  pop it now since we don't want to call it when we
                //  process the completions.
                //

                if (CompletionNode != NULL) {

                    POP_COMPLETION_NODE( CompletionStack );
                }

                return Status;

            } else if (!NT_SUCCESS( Status )) {

                //
                //  We hit an error, see if it is allowable to fail.
                //

                if (!AllowFilterToFail && isFilter) {

                    //
                    //  This device object represents a filter and filters
                    //  are not allowed to fail this operation.  Mask the
                    //  error and continue processing.
                    //
                    //  In DBG builds, we will print out an error message to
                    //  notify the filter writer.
                    //

                    KdPrint(( "FS FILTER: FsFilterPerformPrecallbacks -- filter failed operation but this operation is marked to disallow failure, so ignoring.\n" ));

                    Status = STATUS_SUCCESS;

                } else if (!AllowBaseFsToFail && !isFilter) {

                    //
                    //  This device object represents a base file system and
                    //  base file systems are not allowed to fail this
                    //  operation.  Mask the error and continue processing.
                    //
                    //  In DBG builds, we will print out an error message to
                    //  notify the file system writer.
                    //

                    KdPrint(( "FS FILTER: FsFilterPerformPrecallbacks -- base file system failed operation but this operation is marked to disallow failure, so ignoring.\n" ));

                    Status = STATUS_SUCCESS;

                } else {

                    //
                    //  This device is allowed to fail this operation, therefore
                    //  return the error.
                    //

                    if (!isFilter) {

                        *BaseFsFailedOp = TRUE;

                    }

                    //
                    //  If we allocated a completion node for this device,
                    //  pop it now since we don't want to call it when we
                    //  process the completions.
                    //

                    if (CompletionNode != NULL) {

                        POP_COMPLETION_NODE( CompletionStack );
                    }

                    return Status;
                }
            }

        } else {

            //
            //  Don't have to do anything here because the completionNode
            //  is already initialize appropriately.  We will process
            //  the PostOperationCallback later in this routine.
            //

            NOTHING;
        }

        if (CurrentDeviceObject != Data->DeviceObject) {

            //
            //  We change device stacks, therefore we need to mark this FsFilterCtrl
            //  structure so that we reevaluate the base file system parameters
            //  when calling through the legacy FastIO path.
            //

            SetFlag( FsFilterCtrl->Flags, FS_FILTER_CHANGED_DEVICE_STACKS );
            CurrentDeviceObject = Data->DeviceObject;

        } else {

            //
            //  We didn't change stacks.
            //

            //
            //  See if this is the base file system.  If it is, we want to make
            //  sure that we don't call the base file system's post-operation
            //  callback because this like the base file system is completing
            //  this operation.  In this case, we will pop the completion node
            //  if one was allocated.
            //

            if (!isFilter && CompletionNode != NULL) {

                POP_COMPLETION_NODE( CompletionStack );
            }

            //
            //  Now, iterate down the device object chain.
            //

            CurrentDeviceObject = CurrentDeviceObject->DeviceObjectExtension->AttachedTo;
        }
    }

    return Status;
}

VOID
FsFilterPerformCompletionCallbacks(
    IN PFS_FILTER_CTRL FsFilterCtrl,
    IN NTSTATUS OperationStatus
    )
{
    PFS_FILTER_CALLBACK_DATA Data = &(FsFilterCtrl->Data);
    PFS_FILTER_COMPLETION_STACK CompletionStack = &(FsFilterCtrl->CompletionStack);
    PFS_FILTER_COMPLETION_NODE CompletionNode;

    while (CompletionStack->NextStackPosition > 0) {

        CompletionNode = GET_COMPLETION_NODE( CompletionStack );

        ASSERT( CompletionNode != NULL );

        //
        //  Call the completion callback that the device object registered.
        //

        Data->DeviceObject = CompletionNode->DeviceObject;
        Data->FileObject = CompletionNode->FileObject;

        (CompletionNode->CompletionCallback)( Data,
                                              OperationStatus,
                                              CompletionNode->CompletionContext );

        //
        // Move onto the next CompletionNode.
        //

        POP_COMPLETION_NODE( CompletionStack );
    }
}

const LARGE_INTEGER FsRtlHalfSecond = {(ULONG)(-5 * 100 * 1000 * 10), -1};

NTKERNELAPI
NTSTATUS
FsRtlCreateSectionForDataScan (
    OUT PHANDLE SectionHandle,
    OUT PVOID *SectionObject,
    OUT PLARGE_INTEGER SectionFileSize OPTIONAL,
    IN PFILE_OBJECT FileObject,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize OPTIONAL,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN ULONG Flags
    )

/*++

Routine Description:

    This routine creates a section for given a file object.  It returns the
    section object, the handle which references the section object, and
    (optionally) the size of the file when it was mapped if the section was
    able to be created successfully.

    This routine functions like NtCreateSection except that it takes a
    file object as the parameter to describe which stream to use to back the
    file.  It properly synchronizes the section creation with other file system
    operations.

    NOTE: At this point, this routine can only be used to create a DATA section.
        In the future, this routine may change to support that functionality.

Arguments:

    SectionHandle - Set to the handle created to represent the section returned.

    SectionObject - Set to the section object created.

    SectionFileSize - If non-NULL, set to the file's size at the time the
        section was created.

    FileObject - The file object representing the stream to back this section.

    DesiredAccess - The desired types of access for the section.

    ObjectAttributes - Supplies a pointer to an object attributes structure.

    MaximumSize - Supplies the maximum size of the section in bytes.
                  This value is rounded up to the host page size and
                  specifies the size of the section (page file
                  backed section) or the maximum size to which a
                  file can be extended or mapped (file backed
                  section).
        NOTE: This value is ignored for now since we will be passing a file object
        to MmCreateSection (Mm will query the file system itself to find the size).

    SectionPageProtection - Supplies the protection to place on each page
                            in the section.  One of PAGE_READ, PAGE_READWRITE,
                            PAGE_EXECUTE, or PAGE_WRITECOPY and, optionally,
                            PAGE_NOCACHE or PAGE_WRITECOMBINE
                            may be specified.

    AllocationAttributes - Supplies a set of flags that describe the
                           allocation attributes of the section.
        NOTE: This routine only supports a subset of AllocationAttributes.  For
            the complete list of AllocationAttributes, see NtCreateSection.

        AllocationAttributes Flags

        SEC_COMMIT - All pages of the section are set to the commit state.

        SEC_FILE - The file specified by the file handle is a mapped
                   file.  If a file handle is supplied and neither
                   SEC_IMAGE or SEC_FILE is supplied, SEC_FILE is
                   assumed.

    Flags - Reserved for future use.

Return Value:

    Returns STATUS_SUCCESS if the section could be created, or the appropriate
    error otherwise.

    If this routine is successful, the caller must close the SectionHandle
    and dereference the SectionObject when it is done using these values.

--*/

{
    NTSTATUS Status;
    LARGE_INTEGER FileSize;
    PVOID Object = NULL;
    HANDLE Handle = NULL;
    ULONG RetryCount = 0;

    UNREFERENCED_PARAMETER( MaximumSize );
    UNREFERENCED_PARAMETER( Flags );

    //
    //  At this point, there are some options which this API does not support.
    //  If those options are passed in, FAIL the operation here.
    //
    //  For SectionPageProtection, one of these flags must be set:
    //      PAGE_READONLY
    //      PAGE_READWRITE
    //
    //  For AllocationAttributes, only these flag combinations are allowed:
    //      SEC_COMMIT must be set
    //      SEC_FILE is allowed to be set (assumed otherwise)
    //

    if (FlagOn(SectionPageProtection, ~(PAGE_READONLY | PAGE_READWRITE)) ||
        (SectionPageProtection == 0)) {

        return STATUS_INVALID_PARAMETER_8;
    }

    if ((!FlagOn( AllocationAttributes, SEC_COMMIT )) ||
        FlagOn( AllocationAttributes, ~(SEC_COMMIT | SEC_FILE) )) {

        return STATUS_INVALID_PARAMETER_9;
    }

    //
    //  We need to acquire the file exclusive to avoid deadlocks and grab its
    //  size.  Note that it can't change while we have its resource exclusive.
    //
    //  MmCreateSection() will normally take these steps, but only if a file
    //  handle is passed in, which we don't have.
    //

    ASSERT( IoGetTopLevelIrp() == NULL );

    IoSetTopLevelIrp( (PIRP)FSRTL_FSP_TOP_LEVEL_IRP );

    FsRtlEnterFileSystem();
    Status = FsRtlAcquireToCreateMappedSection( FileObject,
                                                SectionPageProtection );

    if (!NT_SUCCESS( Status )) {

        FsRtlExitFileSystem();
        IoSetTopLevelIrp( NULL );
        return Status;
    }

    //
    //  Get the file size.  Note that FsRtlGetFileSize() does *not* acquire the
    //  file object lock for synchronous file objects, which is important as
    //  we already own the file system resource, and trying now to acquire the
    //  file object lock would violate locking order and lead to deadlocks.
    //

    Status = FsRtlGetFileSize( FileObject, &FileSize );

    if (NT_SUCCESS(Status)) {

        if (FileSize.QuadPart == 0) {

            //
            //  If the file is of 0 length, we won't bother creating a section.
            //

            Status = STATUS_END_OF_FILE;

        } else {

Retry:
            //
            //  We create a section object from the now valid file object.  We
            //  need to loop here in case we hit a transient lock conflict with
            //  the file system.
            //

            Status = MmCreateSection( &Object,
                                      DesiredAccess,
                                      ObjectAttributes,
                                      &FileSize,
                                      SectionPageProtection,
                                      AllocationAttributes,
                                      NULL,
                                      FileObject );

            ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

            if ((Status == STATUS_FILE_LOCK_CONFLICT) &&
                (RetryCount < 3)) {

                //
                // The file system may have prevented this from working
                // due to log file flushing.  Delay and try again.
                //

                RetryCount += 1;

                KeDelayExecutionThread (KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER)&FsRtlHalfSecond);
                goto Retry;
            }
        }
    }

    //
    //  Now release the resource.
    //

    FsRtlReleaseFile( FileObject );
    FsRtlExitFileSystem();

    IoSetTopLevelIrp( NULL );

    if (!NT_SUCCESS( Status )) {

        return Status;
    }

    //
    //  Zero the end of the last page and mark the stream as having a mapped
    //  section.
    //

    CcZeroEndOfLastPage( FileObject );

    //
    //  Now we insert this section in the process handle table, which will
    //  yield a section handle that is useable by the user mode service,
    //  and thus suitable for a call to MapViewOfFile().
    //
    //  Before inserting the object though, we reference it an extra time
    //  so that even a service bug cannot make SectionObject invalid by
    //  closing a random handle.
    //

    ObReferenceObject( Object );

    Status = ObInsertObject( Object,
                             NULL,
                             DesiredAccess,
                             0,
                             (PVOID *)NULL,
                             &Handle);

    if (NT_SUCCESS( Status )) {

        *SectionHandle = Handle;
        *SectionObject = Object;

        if (ARGUMENT_PRESENT( SectionFileSize )) {

            SectionFileSize->QuadPart = FileSize.QuadPart;
        }

    } else {

        //
        //  Note if the insertion fails, Ob dereferences the section object
        //  once for us, so we only need to remove the reference we added.
        //

        ObDereferenceObject( Object );
    }

    return Status;
}

