/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved. 
Copyright (c) International Business Machines Corporation

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    po.h

Abstract:

    This module contains the internal structure definitions and APIs used by
    the NT Power Management.

--*/



#ifndef _PO_
#define _PO_

#if DBG

VOID
PoPowerTracePrint(
    ULONG    TracePoint,
    ULONG_PTR Caller,
    ULONG_PTR CallerCaller,
    ULONG_PTR DeviceObject,
    ULONG_PTR Irp,
    ULONG_PTR Ios
    );

#define PoPowerTrace(TracePoint,DevObj,Arg1,Arg2) \
{\
    PVOID pptcCaller;      \
    PVOID pptcCallerCaller;  \
    RtlGetCallersAddress(&pptcCaller, &pptcCallerCaller); \
    PoPowerTracePrint(TracePoint, (ULONG_PTR)pptcCaller, (ULONG_PTR)pptcCallerCaller, (ULONG_PTR)DevObj, (ULONG_PTR)Arg1, (ULONG_PTR)Arg2); \
}
#else
#define PoPowerTrace(TracePoint,DevObj,Arg1,Arg2)
#endif

#define POWERTRACE_CALL         0x1
#define POWERTRACE_PRESENT      0x2
#define POWERTRACE_STARTNEXT    0x4
#define POWERTRACE_SETSTATE     0x8
#define POWERTRACE_COMPLETE     0x10


VOID
FASTCALL
PoInitializePrcb (
    PKPRCB      Prcb
    );

BOOLEAN
PoInitSystem (
    IN ULONG    Phase
    );

VOID
PoInitDriverServices (
    IN ULONG    Phase
    );

VOID
PoInitHiberServices (
    IN BOOLEAN  Setup
    );

VOID
PoGetDevicePowerState (
    IN  PDEVICE_OBJECT      PhysicalDeviceObject,
    OUT DEVICE_POWER_STATE  *DevicePowerState
    );

VOID
PoInitializeDeviceObject (
    IN PDEVOBJ_EXTENSION   DeviceObjectExtension
    );

VOID
PoRunDownDeviceObject (
    IN PDEVICE_OBJECT   DeviceObject
    );

NTKERNELAPI
VOID
PopCleanupPowerState (
    IN OUT PUCHAR PowerState
    );

#define PoRundownThread(Thread)     \
        PopCleanupPowerState(&Thread->Tcb.PowerState)

#define PoRundownProcess(Process)   \
        PopCleanupPowerState(&Process->Pcb.PowerState)

VOID
PoNotifySystemTimeSet (
    VOID
    );

VOID
PoInvalidateDevicePowerRelations(
    PDEVICE_OBJECT  DeviceObject
    );

VOID
PoShutdownBugCheck (
    IN BOOLEAN  AllowCrashDump,
    IN ULONG    BugCheckCode,
    IN ULONG_PTR BugCheckParameter1,
    IN ULONG_PTR BugCheckParameter2,
    IN ULONG_PTR BugCheckParameter3,
    IN ULONG_PTR BugCheckParameter4
    );

// begin_nthal

NTKERNELAPI
VOID
PoSetHiberRange (
    IN PVOID     MemoryMap,
    IN ULONG     Flags,
    IN PVOID     Address,
    IN ULONG_PTR Length,
    IN ULONG     Tag
    );

// memory_range.Type
#define PO_MEM_PRESERVE         0x00000001      // memory range needs preserved
#define PO_MEM_CLONE            0x00000002      // Clone this range
#define PO_MEM_CL_OR_NCHK       0x00000004      // Either clone or do not checksum
#define PO_MEM_DISCARD          0x00008000      // This range to be removed
#define PO_MEM_PAGE_ADDRESS     0x00004000      // Arguments passed are physical pages

// end_nthal

#define PoWakeTimerSupported()  \
    (PopCapabilities.RtcWake >= PowerSystemSleeping1)

ULONG
PoSimpleCheck (
    IN ULONG                PatialSum,
    IN PVOID                StartVa,
    IN ULONG_PTR            Length
    );

BOOLEAN
PoSystemIdleWorker (
    IN BOOLEAN IdleWorker
    );

VOID
PoVolumeDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
PoSetWarmEjectDevice(
    IN PDEVICE_OBJECT DeviceObject
    ) ;

NTSTATUS
PoGetLightestSystemStateForEject(
    IN   BOOLEAN              DockBeingEjected,
    IN   BOOLEAN              HotEjectSupported,
    IN   BOOLEAN              WarmEjectSupported,
    OUT  PSYSTEM_POWER_STATE  LightestSleepState
    );

// begin_ntddk begin_wdm begin_ntosp

NTKERNELAPI
VOID
PoSetSystemState (
    __in EXECUTION_STATE Flags
    );

// begin_ntifs

NTKERNELAPI
PVOID
PoRegisterSystemState (
    __inout_opt PVOID StateHandle,
    __in EXECUTION_STATE Flags
    );

// end_ntifs

typedef
VOID
(*PREQUEST_POWER_COMPLETE) (
    __in PDEVICE_OBJECT DeviceObject,
    __in UCHAR MinorFunction,
    __in POWER_STATE PowerState,
    __in_opt PVOID Context,
    __in PIO_STATUS_BLOCK IoStatus
    );

NTKERNELAPI
NTSTATUS
PoRequestPowerIrp (
    __in PDEVICE_OBJECT DeviceObject,
    __in UCHAR MinorFunction,
    __in POWER_STATE PowerState,
    __in_opt PREQUEST_POWER_COMPLETE CompletionFunction,
    __in_opt PVOID Context,
    __out_opt PIRP *Irp
    );

NTKERNELAPI
NTSTATUS
PoRequestShutdownEvent (
    OUT PVOID *Event
    );

NTKERNELAPI
NTSTATUS
PoRequestShutdownWait (
    IN PETHREAD Thread
    );

// begin_ntifs

NTKERNELAPI
VOID
PoUnregisterSystemState (
    __in PVOID StateHandle
    );

// begin_nthal

NTKERNELAPI
POWER_STATE
PoSetPowerState (
    __in PDEVICE_OBJECT   DeviceObject,
    __in POWER_STATE_TYPE Type,
    __in POWER_STATE      State
    );

NTKERNELAPI
NTSTATUS
PoCallDriver (
    __in PDEVICE_OBJECT DeviceObject,
    __inout PIRP        Irp
    );

NTKERNELAPI
VOID
PoStartNextPowerIrp(
    __in PIRP   Irp
    );


NTKERNELAPI
PULONG
PoRegisterDeviceForIdleDetection (
    __in PDEVICE_OBJECT     DeviceObject,
    __in ULONG              ConservationIdleTime,
    __in ULONG              PerformanceIdleTime,
    __in DEVICE_POWER_STATE State
    );

#define PoSetDeviceBusy(IdlePointer) \
    *IdlePointer = 0

//
// \Callback\PowerState values
//

#define PO_CB_SYSTEM_POWER_POLICY       0
#define PO_CB_AC_STATUS                 1
#define PO_CB_BUTTON_COLLISION          2
#define PO_CB_SYSTEM_STATE_LOCK         3
#define PO_CB_LID_SWITCH_STATE          4
#define PO_CB_PROCESSOR_POWER_POLICY    5

// end_ntddk end_wdm end_nthal

// Used for queuing work items to be performed at shutdown time.  Same
// rules apply as per Ex work queues.
NTKERNELAPI
NTSTATUS
PoQueueShutdownWorkItem(
    IN PWORK_QUEUE_ITEM WorkItem
    );

// end_ntosp end_ntifs

//
// Broken functions we don't intend to keep supporting. The code backing these
// should be ripped out in NT5.1
//
typedef
VOID
(*PPO_NOTIFY) (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PVOID            Context,
    IN ULONG            Type,
    IN ULONG            Reserved
    );

#define PO_NOTIFY_D0                        0x00000001
#define PO_NOTIFY_TRANSITIONING_FROM_D0     0x00000002
#define PO_NOTIFY_INVALID                   0x80000000

NTKERNELAPI
NTSTATUS
PoRegisterDeviceNotify (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PPO_NOTIFY       NotificationFunction,
    IN PVOID            NotificationContext,
    IN ULONG            NotificationType,
    OUT PDEVICE_POWER_STATE  DeviceState,
    OUT PVOID           *NotificationHandle
    );

NTKERNELAPI
NTSTATUS
PoCancelDeviceNotify (
    IN PVOID            NotificationHandle
    );


//
// Callout set state failure notification
//

typedef struct {
    NTSTATUS            Status;
    POWER_ACTION        PowerAction;
    SYSTEM_POWER_STATE  MinState;
    ULONG               Flags;
} PO_SET_STATE_FAILURE, *PPO_SET_STATE_FAILURE;

//
// Hibernation file layout:
//      Page 0  - PO_MEMORY_IMAGE
//      Page 1  - Free page array
//      Page 2  - KPROCESSOR_CONTEXT
//      Page 3  - first memory_range_array page
//
// PO_MEMORY_IMAGE:
//      Header in file which contains some information to identify
//      the hibernation, as well as a couple of checksums.
//
// Free page array:
//      A page full of page numbers which identify 4MBs worth of
//      system pages that are not in the restoration image.  These
//      pages are used by the loader (to keep itself out of the way)
//      when restoring the memory image.
//
// KPROCESSOR_CONTEST
//      The context of the processor which hibernated the system.
//      Rest of page is empty.
//
// memory_range_array
//      A page which contains an array of memory_range_array elements
//      where element 0 is a Link entry, and all other elements are
//      Range entries.   The Link entry is used to link to the next
//      such page, and to supply a count of the number of Range entries
//      in the current page.   The range entries each describe one
//      physical memory range which needs restoration and its location
//      in the file.
//

typedef struct _PO_MEMORY_RANGE_ARRAY_RANGE {
    PFN_NUMBER      PageNo;
    PFN_NUMBER      StartPage;
    PFN_NUMBER      EndPage;
    ULONG           CheckSum;
} PO_MEMORY_RANGE_ARRAY_RANGE;

typedef struct _PO_MEMORY_RANGE_ARRAY_LINK {
    struct _PO_MEMORY_RANGE_ARRAY *Next;
    PFN_NUMBER NextTable;
    ULONG CheckSum;
    ULONG EntryCount;
} PO_MEMORY_RANGE_ARRAY_LINK;

typedef struct _PO_MEMORY_RANGE_ARRAY {
    union {
        PO_MEMORY_RANGE_ARRAY_RANGE Range;
        PO_MEMORY_RANGE_ARRAY_LINK Link;
    };
} PO_MEMORY_RANGE_ARRAY, *PPO_MEMORY_RANGE_ARRAY;

#define PO_MAX_RANGE_ARRAY  (PAGE_SIZE / sizeof(PO_MEMORY_RANGE_ARRAY))
#define PO_ENTRIES_PER_PAGE (PO_MAX_RANGE_ARRAY-1)


#define PO_IMAGE_SIGNATURE          'rbih'
#define PO_IMAGE_SIGNATURE_WAKE     'ekaw'
#define PO_IMAGE_SIGNATURE_BREAK    'pkrb'
#define PO_IMAGE_SIGNATURE_LINK     'knil'
#define PO_IMAGE_HEADER_PAGE        0
#define PO_FREE_MAP_PAGE            1
#define PO_PROCESSOR_CONTEXT_PAGE   2
#define PO_FIRST_RANGE_TABLE_PAGE   3

#define PO_COMPRESS_CHUNK_SIZE      4096

//
// Perf information
//
typedef struct _PO_HIBER_PERF {
    ULONGLONG               IoTicks;
    ULONGLONG               InitTicks;
    ULONGLONG               CopyTicks;
    ULONGLONG               StartCount;
    ULONG                   ElapsedTime;
    ULONG                   IoTime;
    ULONG                   CopyTime;
    ULONG                   InitTime;
    ULONG                   PagesWritten;
    ULONG                   PagesProcessed;
    ULONG                   BytesCopied;
    ULONG                   DumpCount;
    ULONG                   FileRuns;

} PO_HIBER_PERF, *PPO_HIBER_PERF;

//
// Define various HiberFlags to control the behavior when restoring
//
#define PO_HIBER_APM_RECONNECT      0x1
#define PO_HIBER_NO_EXECUTE         0x2

typedef struct {
    ULONG                   Signature;
    ULONG                   Version;
    ULONG                   CheckSum;
    ULONG                   LengthSelf;
    PFN_NUMBER              PageSelf;
    ULONG                   PageSize;

    ULONG                   ImageType;
    LARGE_INTEGER           SystemTime;
    ULONGLONG               InterruptTime;
    ULONG                   FeatureFlags;
    UCHAR                   HiberFlags;
    UCHAR                   spare[3];

    ULONG                   NoHiberPtes;
    ULONG_PTR               HiberVa;
    PHYSICAL_ADDRESS        HiberPte;

    ULONG                   NoFreePages;
    ULONG                   FreeMapCheck;
    ULONG                   WakeCheck;

    PFN_NUMBER              TotalPages;
    PFN_NUMBER              FirstTablePage;
    PFN_NUMBER              LastFilePage;

    //
    // Perf stuff
    //
    PO_HIBER_PERF           PerfInfo;
} PO_MEMORY_IMAGE, *PPO_MEMORY_IMAGE;


typedef struct {
    ULONG                   Signature;
    WCHAR                   Name[1];
} PO_IMAGE_LINK, *PPO_IMAGE_LINK;

//
// Returned by Io system
//

typedef struct _PO_DEVICE_NOTIFY {
    LIST_ENTRY              Link;
    PDEVICE_OBJECT          TargetDevice;

    BOOLEAN                 WakeNeeded;
    UCHAR                   OrderLevel;

    PDEVICE_OBJECT          DeviceObject;
    PVOID                   Node;
    PWCHAR                  DeviceName;
    PWCHAR                  DriverName;
    ULONG                   ChildCount;
    ULONG                   ActiveChild;

} PO_DEVICE_NOTIFY, *PPO_DEVICE_NOTIFY;

//
// A PO_DEVICE_NOTIFY_LEVEL structure holds all the PO_DEVICE_NOTIFY
// structures for a given level. Every PO_DEVICE_NOTIFY is on one of
// the lists. As we send power irps, the notify structure progresses
// through all the lists.
//
typedef struct _PO_NOTIFY_ORDER_LEVEL {
    KEVENT     LevelReady;
    ULONG      DeviceCount;     // number of devices on this notify level
    ULONG      ActiveCount;     // number of devices until this level is complete
    LIST_ENTRY WaitSleep;       // waiting for children to complete their Sx irps
    LIST_ENTRY ReadySleep;      // ready to receive a Sx irp
    LIST_ENTRY Pending;         // A Sx or S0 irp is outstanding
    LIST_ENTRY Complete;        // Fully awake.
    LIST_ENTRY ReadyS0;         // Ready to receive a S0 irp
    LIST_ENTRY WaitS0;          // waiting for parent to complete their S0 irp
} PO_NOTIFY_ORDER_LEVEL, *PPO_NOTIFY_ORDER_LEVEL;

#define PO_ORDER_NOT_VIDEO          0x0001
#define PO_ORDER_ROOT_ENUM          0x0002
#define PO_ORDER_PAGABLE            0x0004
#define PO_ORDER_MAXIMUM            0x0007

// notify GDI before this order level
#define PO_ORDER_GDI_NOTIFICATION   (PO_ORDER_PAGABLE)

typedef struct _PO_DEVICE_NOTIFY_ORDER {
    ULONG                   DevNodeSequence;
    PDEVICE_OBJECT          *WarmEjectPdoPointer;
    PO_NOTIFY_ORDER_LEVEL   OrderLevel[PO_ORDER_MAXIMUM+1];
} PO_DEVICE_NOTIFY_ORDER, *PPO_DEVICE_NOTIFY_ORDER;

extern BOOLEAN          PoEnabled;
extern ULONG            PoPowerSequence;
extern BOOLEAN          PoPageLockData;
extern KTIMER           PoSystemIdleTimer;
extern BOOLEAN          PoHiberInProgress;

// PopCapabilities used by some internal macros
extern SYSTEM_POWER_CAPABILITIES PopCapabilities;

extern ULONG        PopShutdownCleanly;

// Set this flag to make general clean shutdown-related things happen
// without setting any of the more specific things.
#define PO_CLEAN_SHUTDOWN_GENERAL  (0x1)

// PO_CLEAN_SHUTDOWN_PAGING forces unlocked pageable data to become
// unavailable once paging is shut down.
#define PO_CLEAN_SHUTDOWN_PAGING   (0x2)

// PO_CLEAN_SHUTDOWN_WORKERS causes the Ex worker threads to be torn
// down at shutdown time (ensuring that their queues are flushed and
// no more work items are posted).
#define PO_CLEAN_SHUTDOWN_WORKERS  (0x4)

// PO_CLEAN_SHUTDOWN_REGISTRY causes all open registry keys to be
// dumped to the debugger at shutdown time.
#define PO_CLEAN_SHUTDOWN_REGISTRY (0x8)

// PO_CLEAN_SHUTDOWN_OB causes the object manager namespace to be
// flushed of all permanent objects, and causes ob cleanup to occur.
#define PO_CLEAN_SHUTDOWN_OB       (0x10)

// PO_CLEAN_SHUTDOWN_PNP causes PNP to QueryRemove/Remove all the PNP devices
// in the system.
#define PO_CLEAN_SHUTDOWN_PNP      (0x20)

// This function returns non-zero if the system should be shut down cleanly.
ULONG
FORCEINLINE
PoCleanShutdownEnabled(
    VOID
    )
{
    return PopShutdownCleanly;
}

// This is the worker queue which po will use for shutdown
#define PO_SHUTDOWN_QUEUE (CriticalWorkQueue)

#endif

