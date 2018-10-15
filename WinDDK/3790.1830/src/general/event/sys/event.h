/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.


Module Name:

   Event.h

--*/

#ifndef __EVENT__
#define __EVENT__

//
// DEFINES
//

#define NTDEVICE_NAME_STRING      L"\\Device\\Event_Sample"
#define SYMBOLIC_NAME_STRING      L"\\DosDevices\\Event_Sample"
#define TAG (ULONG)'TEVE'

#if DBG
#define DebugPrint(_x_) \
                DbgPrint("EVENT.SYS: ");\
                DbgPrint _x_;

#else

#define DebugPrint(_x_)

#endif

//
// DATA
//
typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT Self;
    LIST_ENTRY EventQueueHead; // Queue where all the user notification requests are queued
    KSPIN_LOCK  QueueLock;
    
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


typedef struct _NOTIFY_RECORD{
    NOTIFY_TYPE   Type;
    LIST_ENTRY  ListEntry;
    union {
        PKEVENT     Event;
        PIRP             PendingIrp;
    };
    KDPC            Dpc;
    KTIMER          Timer;   
    PFILE_OBJECT    FileObject;
    PDEVICE_EXTENSION   DeviceExtension;
    
} NOTIFY_RECORD, *PNOTIFY_RECORD;

//
// PROTOS
//
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
EventUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
EventCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
EventCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
EventDispatchIoControl( 
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
EventCancelRoutine( 
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
CustomTimerDPC(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

NTSTATUS
RegisterEventBasedNotification(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RegisterIrpBasedNotification(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#endif // __EVENT__
