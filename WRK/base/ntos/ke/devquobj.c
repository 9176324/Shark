/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    devquobj.c

Abstract:

    This module implements the kernel device queue object. Functions are
    provided to initialize a device queue object and to insert and remove
    device queue entries in a device queue object.

--*/

#include "ki.h"

//
// The following assert macro is used to check that an input device queue
// is really a kdevice_queue and not something else, like deallocated pool.
//

#define ASSERT_DEVICE_QUEUE(E) {            \
    ASSERT((E)->Type == DeviceQueueObject); \
}

//
// Device Queue Hint is enabled for AMD64 only, because the DEVICE_QUEUE
// structure has sufficient padding in which to store the hint only on
// WIN64, and IA64 kernel mode addresses cannot have the MSB truncated
// and reconstructed via sign extension.
// 

#if defined(_AMD64_)

#define _DEVICE_QUEUE_HINT_

#endif

#if defined(_DEVICE_QUEUE_HINT_)

FORCEINLINE
PKDEVICE_QUEUE_ENTRY
KiGetDeviceQueueKeyHint (
    IN PKDEVICE_QUEUE DeviceQueue
    )

/*++

Routine Description:

    This function extracts a device queue entry address as a hint from a
    device queue object.

    N.B. Hints are stored as 56-bit sign extended value.

Arguments:

    DeviceQueue - Supplies a pointer to a control object of type device
        queue.

Return Value:

    Returns the device queue hint, or NULL of a hint does not exist.

--*/

{

    return (PKDEVICE_QUEUE_ENTRY)(DeviceQueue->Hint);
}

FORCEINLINE
VOID
KiSetDeviceQueueKeyHint (
    IN OUT PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
    )

/*++

Routine Description:

    This function stores a device queue entry address as a hint in a device
    queue object.

    N.B. Hints are stored as 56-bit sign extended value.

Arguments:

    DeviceQueue - Supplies a pointer to a control object of type device
        queue.

Return Value:

    Returns the device queue hint, or NULL of a hint does not exist.

--*/

{
    DeviceQueue->Hint = (LONG64)DeviceQueueEntry;
}

FORCEINLINE
VOID
KiInvalidateDeviceQueueKeyHint (
    IN OUT PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
    )

/*++

Routine Description:

    This function is invoked when a device queue entry is removed from a
    device queue to determine whether the device queue entry matches the
    hint value for the device queue, and if so to invalidate it.

Arguments:

    DeviceQueue - Supplies a pointer to a control object of type device
        queue.

    DeviceQueueEntry - Supplies a pointer to the queue entry that has been
        removed from the queue.

Return Value:

    None.

--*/

{
    if (DeviceQueueEntry == KiGetDeviceQueueKeyHint(DeviceQueue)) {
        KiSetDeviceQueueKeyHint(DeviceQueue,NULL);
    }

    return;
}

#else

#define KiInvalidateDeviceQueueKeyHint(q, e)
#define KiSetDeviceQueueKeyHint(q, e)

#endif

VOID
KeInitializeDeviceQueue (
    __out PKDEVICE_QUEUE DeviceQueue
    )

/*++

Routine Description:

    This function initializes a kernel device queue object.

Arguments:

    DeviceQueue - Supplies a pointer to a control object of type device
        queue.

    SpinLock - Supplies a pointer to an executive spin lock.

Return Value:

    None.

--*/

{

    //
    // Initialize standard control object header.
    //

    DeviceQueue->Type = DeviceQueueObject;
    DeviceQueue->Size = sizeof(KDEVICE_QUEUE);

    //
    // Initialize the device queue list head, spin lock, busy indicator,
    // and hint.
    //

    InitializeListHead(&DeviceQueue->DeviceListHead);
    KeInitializeSpinLock(&DeviceQueue->Lock);
    DeviceQueue->Busy = FALSE;
    KiSetDeviceQueueKeyHint(DeviceQueue, NULL);
    return;
}

BOOLEAN
KeInsertDeviceQueue (
    __inout PKDEVICE_QUEUE DeviceQueue,
    __inout PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
    )

/*++

Routine Description:

    This function inserts a device queue entry at the tail of the specified
    device queue. If the device is not busy, then it is set busy and the entry
    is not placed in the device queue. Otherwise the specified entry is placed
    at the end of the device queue.

Arguments:

    DeviceQueue - Supplies a pointer to a control object of type device queue.

    DeviceQueueEntry - Supplies a pointer to a device queue entry.

Return Value:

    If the device is not busy, then a value of FALSE is returned. Otherwise a
    value of TRUE is returned.

--*/

{

    BOOLEAN Busy;
    BOOLEAN Inserted;
    KLOCK_QUEUE_HANDLE LockHandle;

    ASSERT_DEVICE_QUEUE(DeviceQueue);

    //
    // Set inserted to FALSE and lock specified device queue.
    //

    Inserted = FALSE;
    KiAcquireInStackQueuedSpinLockForDpc(&DeviceQueue->Lock, &LockHandle);

    //
    // Insert the specified device queue entry at the end of the device queue
    // if the device queue is busy. Otherwise set the device queue busy and
    // don't insert the device queue entry.
    //

    Busy = DeviceQueue->Busy;
    DeviceQueue->Busy = TRUE;
    if (Busy == TRUE) {
        InsertTailList(&DeviceQueue->DeviceListHead,
                       &DeviceQueueEntry->DeviceListEntry);

        Inserted = TRUE;
    }

    DeviceQueueEntry->Inserted = Inserted;

    //
    // Unlock specified device queue.
    //

    KiReleaseInStackQueuedSpinLockForDpc(&LockHandle);
    return Inserted;
}

BOOLEAN
KeInsertByKeyDeviceQueue (
    __inout PKDEVICE_QUEUE DeviceQueue,
    __inout PKDEVICE_QUEUE_ENTRY DeviceQueueEntry,
    __in ULONG SortKey
    )

/*++

Routine Description:

    This function inserts a device queue entry into the specified device
    queue according to a sort key. If the device is not busy, then it is
    set busy and the entry is not placed in the device queue. Otherwise
    the specified entry is placed in the device queue at a position such
    that the specified sort key is greater than or equal to its predecessor
    and less than its successor.

Arguments:

    DeviceQueue - Supplies a pointer to a control object of type device queue.

    DeviceQueueEntry - Supplies a pointer to a device queue entry.

    SortKey - Supplies the sort key by which the position to insert the device
        queue entry is to be determined.

Return Value:

    If the device is not busy, then a value of FALSE is returned. Otherwise a
    value of TRUE is returned.

--*/

{

    BOOLEAN Busy;
    BOOLEAN Inserted;
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;
    PKDEVICE_QUEUE_ENTRY QueueEntry;

    ASSERT_DEVICE_QUEUE(DeviceQueue);

    //
    // Set inserted to FALSE and lock specified device queue.
    //

    Inserted = FALSE;
    DeviceQueueEntry->SortKey = SortKey;
    KiAcquireInStackQueuedSpinLockForDpc(&DeviceQueue->Lock, &LockHandle);

    //
    // Insert the specified device queue entry in the device queue at the
    // position specified by the sort key if the device queue is busy.
    // Otherwise set the device queue busy an don't insert the device queue
    // entry.
    //

    Busy = DeviceQueue->Busy;
    DeviceQueue->Busy = TRUE;
    if (Busy == TRUE) {
        NextEntry = &DeviceQueue->DeviceListHead;
        if (IsListEmpty(NextEntry) == FALSE) {

            //
            // Check the last queue entry in the list, which will have the
            // highest sort key. If this key is greater than or equal to
            // the specified sort key, then the insertion point has been
            // found. Otherwise, walk the list forward until the insertion
            // point is found.
            //

            QueueEntry = CONTAINING_RECORD(NextEntry->Blink,
                                           KDEVICE_QUEUE_ENTRY,
                                           DeviceListEntry);

            if (SortKey < QueueEntry->SortKey) {
                do {
                    NextEntry = NextEntry->Flink;
                    QueueEntry = CONTAINING_RECORD(NextEntry,
                                                   KDEVICE_QUEUE_ENTRY,
                                                   DeviceListEntry);

                } while (SortKey >= QueueEntry->SortKey);
            }
        }

        InsertTailList(NextEntry, &DeviceQueueEntry->DeviceListEntry);
        Inserted = TRUE;
    }

    DeviceQueueEntry->Inserted = Inserted;

    //
    // Unlock specified device queue.
    //

    KiReleaseInStackQueuedSpinLockForDpc(&LockHandle);
    return Inserted;
}

PKDEVICE_QUEUE_ENTRY
KeRemoveDeviceQueue (
    __inout PKDEVICE_QUEUE DeviceQueue
    )

/*++

Routine Description:

    This function removes an entry from the head of the specified device
    queue. If the device queue is empty, then the device is set Not-Busy
    and a NULL pointer is returned. Otherwise the next entry is removed
    from the head of the device queue and the address of device queue entry
    is returned.

Arguments:

    DeviceQueue - Supplies a pointer to a control object of type device queue.

Return Value:

    A NULL pointer is returned if the device queue is empty. Otherwise a
    pointer to a device queue entry is returned.

--*/

{

    PKDEVICE_QUEUE_ENTRY DeviceQueueEntry;
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;

    ASSERT_DEVICE_QUEUE(DeviceQueue);

    //
    // Set device queue entry NULL and lock specified device queue.
    //

    DeviceQueueEntry = NULL;
    KiAcquireInStackQueuedSpinLockForDpc(&DeviceQueue->Lock, &LockHandle);

    //
    // If the device queue is not empty, then remove the first entry from
    // the queue. Otherwise set the device queue not busy.
    //

    ASSERT(DeviceQueue->Busy == TRUE);

    if (IsListEmpty(&DeviceQueue->DeviceListHead) == TRUE) {
        DeviceQueue->Busy = FALSE;

    } else {
        NextEntry = RemoveHeadList(&DeviceQueue->DeviceListHead);
        DeviceQueueEntry = CONTAINING_RECORD(NextEntry,
                                             KDEVICE_QUEUE_ENTRY,
                                             DeviceListEntry);

        DeviceQueueEntry->Inserted = FALSE;
        KiInvalidateDeviceQueueKeyHint(DeviceQueue,DeviceQueueEntry);
    }

    //
    // Unlock specified device queue and return address of device queue
    // entry.
    //

    KiReleaseInStackQueuedSpinLockForDpc(&LockHandle);
    return DeviceQueueEntry;
}

PKDEVICE_QUEUE_ENTRY
KeRemoveByKeyDeviceQueue (
    __inout PKDEVICE_QUEUE DeviceQueue,
    __in ULONG SortKey
    )

/*++

Routine Description:

    This function removes an entry from the specified device queue. If the
    device queue is empty, then the device is set Not-Busy and a NULL pointer
    is returned. Otherwise the an entry is removed from the device queue and
    the address of device queue entry is returned. The queue is search for the
    first entry which has a value greater than or equal to the specified sort
    key. If no such entry is found, then the first entry of the queue is
    returned.

Arguments:

    DeviceQueue - Supplies a pointer to a control object of type device queue.

    SortKey - Supplies the sort key by which the position to remove the device
        queue entry is to be determined.

Return Value:

    A NULL pointer is returned if the device queue is empty. Otherwise a
    pointer to a device queue entry is returned.

--*/

{

    PKDEVICE_QUEUE_ENTRY DeviceQueueEntry;
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;

#if defined(_DEVICE_QUEUE_HINT_)

    PKDEVICE_QUEUE_ENTRY DeviceQueueHint;

#endif

    ASSERT_DEVICE_QUEUE(DeviceQueue);

    //
    // Set device queue entry NULL and lock specified device queue.
    //

    DeviceQueueEntry = NULL;
    KiAcquireInStackQueuedSpinLockForDpc(&DeviceQueue->Lock, &LockHandle);

    //
    // If the device queue is not empty, then remove the first entry from
    // the queue. Otherwise set the device queue not busy.
    //

    ASSERT(DeviceQueue->Busy == TRUE);

    if (IsListEmpty(&DeviceQueue->DeviceListHead) == TRUE) {
        DeviceQueue->Busy = FALSE;

    } else {

        NextEntry = &DeviceQueue->DeviceListHead;
        DeviceQueueEntry = CONTAINING_RECORD(NextEntry->Blink,
                                             KDEVICE_QUEUE_ENTRY,
                                             DeviceListEntry);

        //
        // First check to see whether the last entry in the sorted list
        // is <= SortKey.  If so, no need to search the list, instead
        // return the first entry directly.
        //

        if (DeviceQueueEntry->SortKey <= SortKey) {
            DeviceQueueEntry = CONTAINING_RECORD(NextEntry->Flink,
                                                 KDEVICE_QUEUE_ENTRY,
                                                 DeviceListEntry);
        } else {

            //
            // Check whether the hint provides a good starting point
            // in the list.  If not, begin the search at the start of
            // the list.
            //

#if defined(_DEVICE_QUEUE_HINT_)

            DeviceQueueHint = KiGetDeviceQueueKeyHint(DeviceQueue);
            if ((DeviceQueueHint != NULL) &&
                (SortKey > DeviceQueueHint->SortKey)) {
                NextEntry = &DeviceQueueHint->DeviceListEntry;

            } else
#endif
            {
                NextEntry = DeviceQueue->DeviceListHead.Flink;
            }

            while (TRUE) {
                DeviceQueueEntry = CONTAINING_RECORD(NextEntry,
                                                     KDEVICE_QUEUE_ENTRY,
                                                     DeviceListEntry);
    
                if (SortKey <= DeviceQueueEntry->SortKey) {
                    break;
                }
    
                NextEntry = NextEntry->Flink;
            }
        }

        //
        // We have an entry. If it is not the first entry in the list, then
        // store the address of the previous node as a hint. Otherwise. clear
        // the hint.
        //

#if defined(_DEVICE_QUEUE_HINT_)

        NextEntry = DeviceQueueEntry->DeviceListEntry.Blink;
        if (NextEntry == &DeviceQueue->DeviceListHead) {
            DeviceQueueHint = NULL;

        } else {
            DeviceQueueHint = CONTAINING_RECORD(NextEntry,
                                                KDEVICE_QUEUE_ENTRY,
                                                DeviceListEntry);
        }

        KiSetDeviceQueueKeyHint(DeviceQueue,DeviceQueueHint);

#endif

        RemoveEntryList(&DeviceQueueEntry->DeviceListEntry);
        DeviceQueueEntry->Inserted = FALSE;
    }

    //
    // Unlock specified device queue and return address of device queue
    // entry.
    //

    KiReleaseInStackQueuedSpinLockForDpc(&LockHandle);
    return DeviceQueueEntry;
}

PKDEVICE_QUEUE_ENTRY
KeRemoveByKeyDeviceQueueIfBusy (
    __inout PKDEVICE_QUEUE DeviceQueue,
    __in ULONG SortKey
    )

/*++

Routine Description:

    This function removes an entry from the specified device queue if and
    only if the device is currently busy. If the device queue is empty or
    the device is not busy, then the device is set Not-Busy and a NULL is
    returned. Otherwise, an entry is removed from the device queue and the
    address of device queue entry is returned. The queue is search for the
    first entry which has a value greater than or equal to the SortKey. If
    no such entry is found then the first entry of the queue is returned.

Arguments:

    DeviceQueue - Supplies a pointer to a control object of type device queue.

    SortKey - Supplies the sort key by which the position to remove the device
        queue entry is to be determined.

Return Value:

    A NULL pointer is returned if the device queue is empty. Otherwise a
    pointer to a device queue entry is returned.

--*/

{

    PKDEVICE_QUEUE_ENTRY DeviceQueueEntry;
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;

    ASSERT_DEVICE_QUEUE(DeviceQueue);

    //
    // Set device queue entry NULL and lock specified device queue.
    //

    DeviceQueueEntry = NULL;
    KiAcquireInStackQueuedSpinLockForDpc(&DeviceQueue->Lock, &LockHandle);

    //
    // If the device queue is busy, then attempt to remove an entry from
    // the queue using the sort key. Otherwise, set the device queue not
    // busy.
    //

    if (DeviceQueue->Busy != FALSE) {
        if (IsListEmpty(&DeviceQueue->DeviceListHead) != FALSE) {
            DeviceQueue->Busy = FALSE;

        } else {
            NextEntry = DeviceQueue->DeviceListHead.Flink;
            while (NextEntry != &DeviceQueue->DeviceListHead) {
                DeviceQueueEntry = CONTAINING_RECORD(NextEntry,
                                                     KDEVICE_QUEUE_ENTRY,
                                                     DeviceListEntry);

                if (SortKey <= DeviceQueueEntry->SortKey) {
                    break;
                }

                NextEntry = NextEntry->Flink;
            }

            if (NextEntry != &DeviceQueue->DeviceListHead) {
                RemoveEntryList(&DeviceQueueEntry->DeviceListEntry);

            } else {
                NextEntry = RemoveHeadList(&DeviceQueue->DeviceListHead);
                DeviceQueueEntry = CONTAINING_RECORD(NextEntry,
                                                     KDEVICE_QUEUE_ENTRY,
                                                     DeviceListEntry);
            }

            DeviceQueueEntry->Inserted = FALSE;
            KiInvalidateDeviceQueueKeyHint(DeviceQueue,DeviceQueueEntry);
        }
    }

    //
    // Unlock specified device queue and return address of device queue
    // entry.
    //

    KiReleaseInStackQueuedSpinLockForDpc(&LockHandle);
    return DeviceQueueEntry;
}

BOOLEAN
KeRemoveEntryDeviceQueue (
    __inout PKDEVICE_QUEUE DeviceQueue,
    __inout PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
    )

/*++

Routine Description:

    This function removes a specified entry from the the specified device
    queue. If the device queue entry is not in the device queue, then no
    operation is performed. Otherwise the specified device queue entry is
    removed from the device queue and its inserted status is set to FALSE.

Arguments:

    DeviceQueue - Supplies a pointer to a control object of type device queue.

    DeviceQueueEntry - Supplies a pointer to a device queue entry which is to
        be removed from its device queue.

Return Value:

    A value of TRUE is returned if the device queue entry is removed from its
    device queue. Otherwise a value of FALSE is returned.

--*/

{

    KLOCK_QUEUE_HANDLE LockHandle;
    BOOLEAN Removed;

    ASSERT_DEVICE_QUEUE(DeviceQueue);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Raise IRQL to dispatcher level and lock specified device queue.
    //

    KeAcquireInStackQueuedSpinLock(&DeviceQueue->Lock, &LockHandle);

    //
    // If the device queue entry is not in a device queue, then no operation
    // is performed. Otherwise remove the specified device queue entry from its
    // device queue.
    //

    Removed = DeviceQueueEntry->Inserted;
    if (Removed == TRUE) {
        DeviceQueueEntry->Inserted = FALSE;
        RemoveEntryList(&DeviceQueueEntry->DeviceListEntry);
        KiInvalidateDeviceQueueKeyHint(DeviceQueue,DeviceQueueEntry);
    }

    //
    // Unlock specified device queue, lower IRQL to its previous level, and
    // return whether the device queue entry was removed from its queue.
    //

    KeReleaseInStackQueuedSpinLock(&LockHandle);
    return Removed;
}

