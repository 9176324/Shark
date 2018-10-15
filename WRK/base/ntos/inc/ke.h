/*++ BUILD Version: 0028    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ke.h

Abstract:

    This module contains the public (external) header file for the kernel.

--*/

#ifndef _KE_
#define _KE_

//
// Define the default quantum decrement values.
//

#define CLOCK_QUANTUM_DECREMENT 3
#define WAIT_QUANTUM_DECREMENT 1
#define LOCK_OWNERSHIP_QUANTUM (WAIT_QUANTUM_DECREMENT * 4)

//
// Define the default ready skip and thread quantum values.
//

#define READY_SKIP_QUANTUM 2
#define THREAD_QUANTUM (READY_SKIP_QUANTUM * CLOCK_QUANTUM_DECREMENT)

//
// Define the round trip decrement count.
//

#define ROUND_TRIP_DECREMENT_COUNT 16

//
// Define thread switch performance data structure.
//

typedef struct _KTHREAD_SWITCH_COUNTERS {
    ULONG FindAny;
    ULONG FindIdeal;
    ULONG FindLast;
    ULONG IdleAny;
    ULONG IdleCurrent;
    ULONG IdleIdeal;
    ULONG IdleLast;
    ULONG PreemptAny;
    ULONG PreemptCurrent;
    ULONG PreemptLast;
    ULONG SwitchToIdle;
} KTHREAD_SWITCH_COUNTERS, *PKTHREAD_SWITCH_COUNTERS;

//
// Public (external) constant definitions.
//

#define BASE_PRIORITY_THRESHOLD NORMAL_BASE_PRIORITY // fast path base threshold

// begin_ntddk begin_wdm begin_ntosp

#define THREAD_WAIT_OBJECTS 3           // Builtin usable wait blocks

// end_ntddk end_wdm end_ntosp

#define EVENT_WAIT_BLOCK 2              // Builtin event pair wait block
#define SEMAPHORE_WAIT_BLOCK 2          // Builtin semaphore wait block
#define TIMER_WAIT_BLOCK 3              // Builtin timer wait block

#if (EVENT_WAIT_BLOCK != SEMAPHORE_WAIT_BLOCK)

#error "wait event and wait semaphore must use same wait block"

#endif

//
// Get APC environment of current thread.
//

#define KeGetCurrentApcEnvironment() \
    KeGetCurrentThread()->ApcStateIndex

//
// begin_ntddk begin_nthal begin_ntosp begin_ntifs
//

#if defined(_X86_)

#define PAUSE_PROCESSOR _asm { rep nop }

#elif defined(_AMD64_)

#define PAUSE_PROCESSOR YieldProcessor();

#else

#error "No target architecture defined"

#endif

// end_ntddk end_nthal end_ntosp end_ntifs

// begin_nthal begin_ntosp

//
// Define macro to generate an affinity mask.
//

#if defined(_NTHAL_) || defined(_NTOSP_) || defined(_AMD64_)

#define AFFINITY_MASK(n) ((ULONG_PTR)1 << (n))

#else

//
// KiMask32Array - This is an array of 32-bit masks that have one bit set
//      in each mask.
//

extern DECLSPEC_CACHEALIGN DECLSPEC_SELECTANY const ULONG KiMask32Array[32] = {
        0x00000001,
        0x00000002,
        0x00000004,
        0x00000008,
        0x00000010,
        0x00000020,
        0x00000040,
        0x00000080,
        0x00000100,
        0x00000200,
        0x00000400,
        0x00000800,
        0x00001000,
        0x00002000,
        0x00004000,
        0x00008000,
        0x00010000,
        0x00020000,
        0x00040000,
        0x00080000,
        0x00100000,
        0x00200000,
        0x00400000,
        0x00800000,
        0x01000000,
        0x02000000,
        0x04000000,
        0x08000000,
        0x10000000,
        0x20000000,
        0x40000000,
        0x80000000};

#if defined(_WIN64)

extern DECLSPEC_CACHEALIGN DECLSPEC_SELECTANY const ULONG64 KiAffinityArray[64] = {
        0x0000000000000001UI64,
        0x0000000000000002UI64,
        0x0000000000000004UI64,
        0x0000000000000008UI64,
        0x0000000000000010UI64,
        0x0000000000000020UI64,
        0x0000000000000040UI64,
        0x0000000000000080UI64,
        0x0000000000000100UI64,
        0x0000000000000200UI64,
        0x0000000000000400UI64,
        0x0000000000000800UI64,
        0x0000000000001000UI64,
        0x0000000000002000UI64,
        0x0000000000004000UI64,
        0x0000000000008000UI64,
        0x0000000000010000UI64,
        0x0000000000020000UI64,
        0x0000000000040000UI64,
        0x0000000000080000UI64,
        0x0000000000100000UI64,
        0x0000000000200000UI64,
        0x0000000000400000UI64,
        0x0000000000800000UI64,
        0x0000000001000000UI64,
        0x0000000002000000UI64,
        0x0000000004000000UI64,
        0x0000000008000000UI64,
        0x0000000010000000UI64,
        0x0000000020000000UI64,
        0x0000000040000000UI64,
        0x0000000080000000UI64,
        0x0000000100000000UI64,
        0x0000000200000000UI64,
        0x0000000400000000UI64,
        0x0000000800000000UI64,
        0x0000001000000000UI64,
        0x0000002000000000UI64,
        0x0000004000000000UI64,
        0x0000008000000000UI64,
        0x0000010000000000UI64,
        0x0000020000000000UI64,
        0x0000040000000000UI64,
        0x0000080000000000UI64,
        0x0000100000000000UI64,
        0x0000200000000000UI64,
        0x0000400000000000UI64,
        0x0000800000000000UI64,
        0x0001000000000000UI64,
        0x0002000000000000UI64,
        0x0004000000000000UI64,
        0x0008000000000000UI64,
        0x0010000000000000UI64,
        0x0020000000000000UI64,
        0x0040000000000000UI64,
        0x0080000000000000UI64,
        0x0100000000000000UI64,
        0x0200000000000000UI64,
        0x0400000000000000UI64,
        0x0800000000000000UI64,
        0x1000000000000000UI64,
        0x2000000000000000UI64,
        0x4000000000000000UI64,
        0x8000000000000000UI64};

#else

#define KiAffinityArray KiMask32Array

#endif

extern const ULONG_PTR KiAffinityArray[];

#define AFFINITY_MASK(n) (KiAffinityArray[n])

#endif

// end_nthal end_ntosp

//
// Define macro to generate priority mask.
//

#if defined(_AMD64_)

#define PRIORITY_MASK(n) ((ULONG)1 << (n))

#else

extern const ULONG KiMask32Array[];

#define PRIORITY_MASK(n) (KiMask32Array[n])

#endif

//
// Define query system time macro.
//
// The following AMD64 code reads an unaligned quadword value. The quadword
// value, however, is guaranteed to be within a cache line, and therefore,
// the value will be read atomically.
//

#if defined(_AMD64_)

#define KiQuerySystemTime(CurrentTime) \
    (CurrentTime)->QuadPart = *((LONG64 volatile *)(&SharedUserData->SystemTime))

#else

#define KiQuerySystemTime(CurrentTime) \
    while (TRUE) {                                                                  \
        (CurrentTime)->HighPart = SharedUserData->SystemTime.High1Time;             \
        (CurrentTime)->LowPart = SharedUserData->SystemTime.LowPart;                \
        if ((CurrentTime)->HighPart == SharedUserData->SystemTime.High2Time) break; \
        PAUSE_PROCESSOR                                                             \
    }

#endif

#if defined(_AMD64_)

#define KiQueryLowTickCount() SharedUserData->TickCount.LowPart

#else

#define KiQueryLowTickCount() KeTickCount.LowPart

#endif

//
// Enumerated kernel types
//
// Kernel object types.
//
//  N.B. There are really two types of event objects; NotificationEvent and
//       SynchronizationEvent. The type value for a notification event is 0,
//       and that for a synchronization event 1.
//
//  N.B. There are two types of new timer objects; NotificationTimer and
//       SynchronizationTimer. The type value for a notification timer is
//       8, and that for a synchronization timer is 9. These values are
//       very carefully chosen so that the dispatcher object type AND'ed
//       with 0x7 yields 0 or 1 for event objects and the timer objects.
//

#define DISPATCHER_OBJECT_TYPE_MASK 0x7

typedef enum _KOBJECTS {
    EventNotificationObject = 0,
    EventSynchronizationObject = 1,
    MutantObject = 2,
    ProcessObject = 3,
    QueueObject = 4,
    SemaphoreObject = 5,
    ThreadObject = 6,
    GateObject = 7,
    TimerNotificationObject = 8,
    TimerSynchronizationObject = 9,
    Spare2Object = 10,
    Spare3Object = 11,
    Spare4Object = 12,
    Spare5Object = 13,
    Spare6Object = 14,
    Spare7Object = 15,
    Spare8Object = 16,
    Spare9Object = 17,
    ApcObject,
    DpcObject,
    DeviceQueueObject,
    EventPairObject,
    InterruptObject,
    ProfileObject,
    ThreadedDpcObject,
    MaximumKernelObject
} KOBJECTS;

#define KOBJECT_LOCK_BIT 0x80
#define KOBJECT_LOCK_BIT_NUMBER 7
#define KOBJECT_TYPE_MASK 0x7f

C_ASSERT((MaximumKernelObject & KOBJECT_LOCK_BIT) == 0);

//
// APC environments.
//

// begin_ntosp

typedef enum _KAPC_ENVIRONMENT {
    OriginalApcEnvironment,
    AttachedApcEnvironment,
    CurrentApcEnvironment,
    InsertApcEnvironment
} KAPC_ENVIRONMENT;

// begin_ntddk begin_wdm begin_nthal begin_ntminiport begin_ntifs begin_ntndis

//
// Interrupt modes.
//

typedef enum _KINTERRUPT_MODE {
    LevelSensitive,
    Latched
} KINTERRUPT_MODE;

// end_ntddk end_wdm end_nthal end_ntminiport end_ntifs end_ntndis end_ntosp

//
// Process states.
//

typedef enum _KPROCESS_STATE {
    ProcessInMemory,
    ProcessOutOfMemory,
    ProcessInTransition,
    ProcessOutTransition,
    ProcessInSwap,
    ProcessOutSwap
} KPROCESS_STATE;

//
// Thread scheduling states.
//

typedef enum _KTHREAD_STATE {
    Initialized,
    Ready,
    Running,
    Standby,
    Terminated,
    Waiting,
    Transition,
    DeferredReady,
    GateWait
} KTHREAD_STATE;

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp
//
// Wait reasons
//

typedef enum _KWAIT_REASON {
    Executive,
    FreePage,
    PageIn,
    PoolAllocation,
    DelayExecution,
    Suspended,
    UserRequest,
    WrExecutive,
    WrFreePage,
    WrPageIn,
    WrPoolAllocation,
    WrDelayExecution,
    WrSuspended,
    WrUserRequest,
    WrEventPair,
    WrQueue,
    WrLpcReceive,
    WrLpcReply,
    WrVirtualMemory,
    WrPageOut,
    WrRendezvous,
    Spare2,
    Spare3,
    Spare4,
    Spare5,
    Spare6,
    WrKernel,
    WrResource,
    WrPushLock,
    WrMutex,
    WrQuantumEnd,
    WrDispatchInt,
    WrPreempted,
    WrYieldExecution,
    WrFastMutex,
    WrGuardedMutex,
    WrRundown,
    MaximumWaitReason
} KWAIT_REASON;

// end_ntddk end_wdm end_nthal

//
// Miscellaneous type definitions
//
// APC state
//
// N.B. The user APC pending field must be the last member of this structure.
//

typedef struct _KAPC_STATE {
    LIST_ENTRY ApcListHead[MaximumMode];
    struct _KPROCESS *Process;
    BOOLEAN KernelApcInProgress;
    BOOLEAN KernelApcPending;
    BOOLEAN UserApcPending;
} KAPC_STATE, *PKAPC_STATE, *PRKAPC_STATE;

#define KAPC_STATE_ACTUAL_LENGTH                                             \
    (FIELD_OFFSET(KAPC_STATE, UserApcPending) + sizeof(BOOLEAN))

// end_ntifs

NTKERNELAPI
BOOLEAN
KeIsWaitListEmpty (
    __in PVOID Object
    );

// end_ntosp

//
// Page frame
//

typedef ULONG KPAGE_FRAME;

//
// Wait block
//
// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp

typedef struct _KWAIT_BLOCK {
    LIST_ENTRY WaitListEntry;
    struct _KTHREAD *Thread;
    PVOID Object;
    struct _KWAIT_BLOCK *NextWaitBlock;
    USHORT WaitKey;
    UCHAR WaitType;
    UCHAR SpareByte;

#if defined(_AMD64_)

    LONG SpareLong;

#endif

} KWAIT_BLOCK, *PKWAIT_BLOCK, *PRKWAIT_BLOCK;

// end_ntddk end_wdm end_nthal end_ntifs end_ntosp

#define KWAIT_BLOCK_OFFSET_TO_BYTE0                                          \
    (FIELD_OFFSET(KWAIT_BLOCK, SpareByte) + sizeof(KWAIT_BLOCK) * 0)

#define KWAIT_BLOCK_OFFSET_TO_BYTE1                                          \
    (FIELD_OFFSET(KWAIT_BLOCK, SpareByte) + sizeof(KWAIT_BLOCK) * 1)

#define KWAIT_BLOCK_OFFSET_TO_BYTE2                                          \
    (FIELD_OFFSET(KWAIT_BLOCK, SpareByte) + sizeof(KWAIT_BLOCK) * 2)

#define KWAIT_BLOCK_OFFSET_TO_BYTE3                                          \
    (FIELD_OFFSET(KWAIT_BLOCK, SpareByte) + sizeof(KWAIT_BLOCK) * 3)

#if defined(_AMD64_)

#define KWAIT_BLOCK_OFFSET_TO_LONG0                                          \
    (FIELD_OFFSET(KWAIT_BLOCK, SpareLong) + sizeof(KWAIT_BLOCK) * 0)

#define KWAIT_BLOCK_OFFSET_TO_LONG1                                          \
    (FIELD_OFFSET(KWAIT_BLOCK, SpareLong) + sizeof(KWAIT_BLOCK) * 1)

#define KWAIT_BLOCK_OFFSET_TO_LONG2                                          \
    (FIELD_OFFSET(KWAIT_BLOCK, SpareLong) + sizeof(KWAIT_BLOCK) * 2)

#define KWAIT_BLOCK_OFFSET_TO_LONG3                                          \
    (FIELD_OFFSET(KWAIT_BLOCK, SpareLong) + sizeof(KWAIT_BLOCK) * 3)

#endif

C_ASSERT(THREAD_WAIT_OBJECTS >= 3);

C_ASSERT(MAXIMUM_WAIT_OBJECTS <= 255);

//
// System service table descriptor.
//
// N.B. A system service number has a 12-bit service table offset and a
//      3-bit service table number.
//
// N.B. Descriptor table entries must be a power of 2 in size. Currently
//      this is 16 bytes on a 32-bit system and 32 bytes on a 64-bit
//      system.
//

#define NUMBER_SERVICE_TABLES 2
#define SERVICE_NUMBER_MASK ((1 << 12) -  1)

#if defined(_WIN64)

#if defined(_AMD64_)

#define SERVICE_TABLE_SHIFT (12 - 4)
#define SERVICE_TABLE_MASK (((1 << 1) - 1) << 4)
#define SERVICE_TABLE_TEST (WIN32K_SERVICE_INDEX << 4)

#else

#define SERVICE_TABLE_SHIFT (12 - 5)
#define SERVICE_TABLE_MASK (((1 << 1) - 1) << 5)
#define SERVICE_TABLE_TEST (WIN32K_SERVICE_INDEX << 5)

#endif

#else

#define SERVICE_TABLE_SHIFT (12 - 4)
#define SERVICE_TABLE_MASK (((1 << 1) - 1) << 4)
#define SERVICE_TABLE_TEST (WIN32K_SERVICE_INDEX << 4)

#endif

typedef struct _KSERVICE_TABLE_DESCRIPTOR {
    PULONG_PTR Base;
    PULONG Count;
    ULONG Limit;
    PUCHAR Number;
} KSERVICE_TABLE_DESCRIPTOR, *PKSERVICE_TABLE_DESCRIPTOR;

//
// Procedure type definitions
//
// Debug routine
//

typedef
BOOLEAN
(*PKDEBUG_ROUTINE) (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN SecondChance
    );

BOOLEAN
KdpStub (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN SecondChance
    );

typedef
BOOLEAN
(*PKDEBUG_SWITCH_ROUTINE) (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN BOOLEAN SecondChance
    );

typedef enum {
    ContinueError = FALSE,
    ContinueSuccess = TRUE,
    ContinueProcessorReselected,
    ContinueNextProcessor
} KCONTINUE_STATUS;

#if defined(_AMD64_)

LONG
KiKernelDpcFilter (
    IN PKDPC Dpc,
    IN PEXCEPTION_POINTERS Information
    );

#endif    

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp
//
// Thread start function
//

typedef
VOID
(*PKSTART_ROUTINE) (
    IN PVOID StartContext
    );

// end_ntddk end_wdm end_nthal end_ntifs end_ntosp

//
// Thread system function
//

typedef
VOID
(*PKSYSTEM_ROUTINE) (
    IN PKSTART_ROUTINE StartRoutine OPTIONAL,
    IN PVOID StartContext OPTIONAL
    );

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp
//
// Kernel object structure definitions
//

//
// Device Queue object and entry
//

typedef struct _KDEVICE_QUEUE {
    CSHORT Type;
    CSHORT Size;
    LIST_ENTRY DeviceListHead;
    KSPIN_LOCK Lock;

#if defined(_AMD64_)

    union {
        BOOLEAN Busy;
        struct {
            LONG64 Reserved : 8;
            LONG64 Hint : 56;
        };
    };

#else

    BOOLEAN Busy;

#endif

} KDEVICE_QUEUE, *PKDEVICE_QUEUE, *PRKDEVICE_QUEUE;

typedef struct _KDEVICE_QUEUE_ENTRY {
    LIST_ENTRY DeviceListEntry;
    ULONG SortKey;
    BOOLEAN Inserted;
} KDEVICE_QUEUE_ENTRY, *PKDEVICE_QUEUE_ENTRY, *PRKDEVICE_QUEUE_ENTRY;

// end_ntddk end_wdm end_nthal end_ntifs end_ntosp

#if !defined(_X86AMD64_)

#if defined(_WIN64)

C_ASSERT(sizeof(KDEVICE_QUEUE) == 0x28);
C_ASSERT(sizeof(KDEVICE_QUEUE_ENTRY) == 0x18);

#else

C_ASSERT(sizeof(KDEVICE_QUEUE) == 0x14);
C_ASSERT(sizeof(KDEVICE_QUEUE_ENTRY) == 0x10);

#endif

#endif

//
// Event pair object
//

typedef struct _KEVENT_PAIR {
    CSHORT Type;
    CSHORT Size;
    KEVENT EventLow;
    KEVENT EventHigh;
} KEVENT_PAIR, *PKEVENT_PAIR, *PRKEVENT_PAIR;

// begin_nthal begin_ntddk begin_wdm begin_ntifs begin_ntosp
//
// Define the interrupt service function type and the empty struct
// type.
//
// end_ntddk end_wdm end_ntifs end_ntosp

struct _KINTERRUPT;

// begin_ntddk begin_wdm begin_ntifs begin_ntosp

typedef
BOOLEAN
(*PKSERVICE_ROUTINE) (
    IN struct _KINTERRUPT *Interrupt,
    IN PVOID ServiceContext
    );

// end_ntddk end_wdm end_ntifs end_ntosp

//
// Interrupt object
//

typedef struct _KINTERRUPT {
    CSHORT Type;
    CSHORT Size;
    LIST_ENTRY InterruptListEntry;
    PKSERVICE_ROUTINE ServiceRoutine;
    PVOID ServiceContext;
    KSPIN_LOCK SpinLock;
    ULONG TickCount;
    PKSPIN_LOCK ActualLock;
    PKINTERRUPT_ROUTINE DispatchAddress;
    ULONG Vector;
    KIRQL Irql;
    KIRQL SynchronizeIrql;
    BOOLEAN FloatingSave;
    BOOLEAN Connected;
    CCHAR Number;
    BOOLEAN ShareVector;
    KINTERRUPT_MODE Mode;
    ULONG ServiceCount;
    ULONG DispatchCount;

#if defined(_AMD64_)

    PKTRAP_FRAME TrapFrame;
    PVOID Reserved;
    ULONG DispatchCode[DISPATCH_LENGTH];

#else

    ULONG DispatchCode[DISPATCH_LENGTH];

#endif

} KINTERRUPT;

#if !defined(_X86AMD64_) && defined(_AMD64_)

C_ASSERT((FIELD_OFFSET(KINTERRUPT, DispatchCode) % 16) == 0);
C_ASSERT((sizeof(KINTERRUPT) % 16) == 0);

#endif

typedef struct _KINTERRUPT *PKINTERRUPT, *PRKINTERRUPT; // ntndis ntosp

// begin_ntifs begin_ntddk begin_wdm begin_ntosp
//
// Mutant object
//

typedef struct _KMUTANT {
    DISPATCHER_HEADER Header;
    LIST_ENTRY MutantListEntry;
    struct _KTHREAD *OwnerThread;
    BOOLEAN Abandoned;
    UCHAR ApcDisable;
} KMUTANT, *PKMUTANT, *PRKMUTANT, KMUTEX, *PKMUTEX, *PRKMUTEX;

// end_ntddk end_wdm end_ntosp
//
// Queue object
//

#define ASSERT_QUEUE(Q) ASSERT(((Q)->Header.Type & KOBJECT_TYPE_MASK) == QueueObject);

// begin_ntosp

typedef struct _KQUEUE {
    DISPATCHER_HEADER Header;
    LIST_ENTRY EntryListHead;
    ULONG CurrentCount;
    ULONG MaximumCount;
    LIST_ENTRY ThreadListHead;
} KQUEUE, *PKQUEUE, *PRKQUEUE;

// end_ntosp

// begin_ntddk begin_wdm begin_ntosp
//
//
// Semaphore object
//
// N.B. The limit field must be the last member of this structure.
//

typedef struct _KSEMAPHORE {
    DISPATCHER_HEADER Header;
    LONG Limit;
} KSEMAPHORE, *PKSEMAPHORE, *PRKSEMAPHORE;

#define KSEMAPHORE_ACTUAL_LENGTH                                             \
    (FIELD_OFFSET(KSEMAPHORE, Limit) + sizeof(LONG))

// end_ntddk end_wdm end_nthal end_ntifs end_ntosp

//
// ALIGNMENT_EXCEPTION_TABLE is used to track alignment exceptions in
// processes that are attached to a debugger.
//

#if !defined(_X86_) && !defined(_AMD64_)

#define ALIGNMENT_RECORDS_PER_TABLE 64
#define MAXIMUM_ALIGNMENT_TABLES    16

typedef struct _ALIGNMENT_EXCEPTION_RECORD {
    PVOID ProgramCounter;
    ULONG Count;
    BOOLEAN AutoFixup;
} ALIGNMENT_EXCEPTION_RECORD, *PALIGNMENT_EXCEPTION_RECORD;

typedef struct _ALIGNMENT_EXCEPTION_TABLE *PALIGNMENT_EXCEPTION_TABLE;
typedef struct _ALIGNMENT_EXCEPTION_TABLE {
    PALIGNMENT_EXCEPTION_TABLE Next;
    ALIGNMENT_EXCEPTION_RECORD RecordArray[ ALIGNMENT_RECORDS_PER_TABLE ];
} ALIGNMENT_EXCEPTION_TABLE;

#endif

// begin_nthal
//
// Define the maximum number of nodes supported.
//
// N.B. Node number must fit in the page color field of a PFN entry.
//

#define MAXIMUM_CCNUMA_NODES    16

// end_nthal
//
// Define node structure for multinode systems.
//
// N.B. The x86 SLIST_HEADER is a single quadword.
//      The AMD64 SLIST_HEADER is 16-byte aligned and contains quadword
//      header - the region field is not used. The below packing for AMD64
//      allows the NUMA node structure to fit in a single cache line.
//

#define KeGetCurrentNode() (KeGetCurrentPrcb()->ParentNode)

typedef struct DECLSPEC_CACHEALIGN _KNODE {
    SLIST_HEADER DeadStackList;         // node dead stack list

#if defined(_AMD64_)

    union {
        SLIST_HEADER PfnDereferenceSListHead; // node deferred PFN freelist
        struct {
            ULONGLONG Alignment;
            KAFFINITY ProcessorMask;
        };
    };

#else

    SLIST_HEADER PfnDereferenceSListHead; // node deferred PFN freelist
    KAFFINITY ProcessorMask;

#endif

    UCHAR Color;                        // zero based node color
    UCHAR Seed;                         // ideal processor seed
    UCHAR NodeNumber;
    struct _flags {
        UCHAR Removable : 1;            // node can be removed
        UCHAR Fill : 7;
    } Flags;

    ULONG MmShiftedColor;               // private shifted color
    PFN_NUMBER FreeCount[2];            // number of colored pages free
    PSLIST_ENTRY PfnDeferredList;       // node deferred PFN list
} KNODE, *PKNODE;

extern PKNODE KeNodeBlock[];

//
// Process object structure definition
//

#define ASSERT_PROCESS(object) ASSERT((object)->Header.Type == ProcessObject)

typedef struct _KEXECUTE_OPTIONS {
    UCHAR ExecuteDisable : 1;
    UCHAR ExecuteEnable : 1;
    UCHAR DisableThunkEmulation : 1;
    UCHAR Permanent : 1;
    UCHAR ExecuteDispatchEnable : 1;
    UCHAR ImageDispatchEnable : 1;
    UCHAR Spare : 2;
} KEXECUTE_OPTIONS, PKEXECUTE_OPTIONS;

typedef struct _KPROCESS {

    //
    // The dispatch header and profile listhead are fairly infrequently
    // referenced.
    //

    DISPATCHER_HEADER Header;
    LIST_ENTRY ProfileListHead;

    //
    // The following fields are referenced during context switches.
    //

    ULONG_PTR DirectoryTableBase[2];

#if defined(_X86_)

    KGDTENTRY LdtDescriptor;
    KIDTENTRY Int21Descriptor;
    USHORT IopmOffset;
    UCHAR Iopl;
    BOOLEAN Unused;

#endif

#if defined(_AMD64_)

    USHORT IopmOffset;

#endif

    volatile KAFFINITY ActiveProcessors;

    //
    // The following fields are referenced during clock interrupts.
    //

    ULONG KernelTime;
    ULONG UserTime;

    //
    // The following fields are referenced infrequently.
    //

    LIST_ENTRY ReadyListHead;
    SINGLE_LIST_ENTRY SwapListEntry;

#if defined(_X86_)

    PVOID VdmTrapcHandler;

#else

    PVOID Reserved1;

#endif

    LIST_ENTRY ThreadListHead;
    KSPIN_LOCK ProcessLock;
    KAFFINITY Affinity;

    //
    // N.B. The following bit number definitions must match the following
    //      bit field.
    //
    // N.B. These bits can only be written with interlocked operations.
    //

#define KPROCESS_AUTO_ALIGNMENT_BIT 0
#define KPROCESS_DISABLE_BOOST_BIT 1
#define KPROCESS_DISABLE_QUANTUM_BIT 2

    union {
        struct {
            LONG AutoAlignment : 1;
            LONG DisableBoost : 1;
            LONG DisableQuantum : 1;
            LONG ReservedFlags : 29;
        };
   
        LONG ProcessFlags;
    };

    SCHAR BasePriority;
    SCHAR QuantumReset;
    UCHAR State;
    UCHAR ThreadSeed;
    UCHAR PowerState;
    UCHAR IdealNode;
    BOOLEAN Visited;
    union {
        KEXECUTE_OPTIONS Flags;
        UCHAR ExecuteOptions;
    };

#if !defined(_X86_) && !defined(_AMD64_)

    PALIGNMENT_EXCEPTION_TABLE AlignmentExceptionTable;

#endif

    ULONG_PTR StackCount;
    LIST_ENTRY ProcessListEntry;
} KPROCESS, *PKPROCESS, *PRKPROCESS;

//
// Thread object
//

typedef enum _ADJUST_REASON {
    AdjustNone = 0,
    AdjustUnwait = 1,
    AdjustBoost = 2
} ADJUST_REASON;

#define ASSERT_THREAD(object) ASSERT((object)->Header.Type == ThreadObject)

//
// Define the number of times a user mode SLIST pop fault is permitted to be
// retried before raising an exception.
// 

#define KI_SLIST_FAULT_COUNT_MAXIMUM 1024

typedef struct _KTHREAD {

    //
    // The dispatcher header and mutant listhead are fairly infrequently
    // referenced.
    //

    DISPATCHER_HEADER Header;
    LIST_ENTRY MutantListHead;

    //
    // The following fields are referenced during context switches and wait
    // operatings. They have been carefully laid out to get the best cache
    // hit ratios.
    //

    PVOID InitialStack;
    PVOID StackLimit;
    PVOID KernelStack;

    KSPIN_LOCK ThreadLock;
    union {
        KAPC_STATE ApcState;
        struct {
            UCHAR ApcStateFill[KAPC_STATE_ACTUAL_LENGTH];
            BOOLEAN ApcQueueable;
            volatile UCHAR NextProcessor;
            volatile UCHAR DeferredProcessor;
            UCHAR AdjustReason;
            SCHAR AdjustIncrement;
        };
    };

    KSPIN_LOCK ApcQueueLock;

#if !defined(_AMD64_)

    ULONG ContextSwitches;
    volatile UCHAR State;
    UCHAR NpxState;
    KIRQL WaitIrql;
    KPROCESSOR_MODE WaitMode;

#endif

    LONG_PTR WaitStatus;
    union {
        PKWAIT_BLOCK WaitBlockList;
        PKGATE GateObject;
    };

    BOOLEAN Alertable;
    BOOLEAN WaitNext;
    UCHAR WaitReason;
    SCHAR Priority;
    UCHAR EnableStackSwap;
    volatile UCHAR SwapBusy;
    BOOLEAN Alerted[MaximumMode];
    union {
        LIST_ENTRY WaitListEntry;
        SINGLE_LIST_ENTRY SwapListEntry;
    };

    PRKQUEUE Queue;

#if !defined(_AMD64_)

    ULONG WaitTime;
    union {
        struct {
            SHORT KernelApcDisable;
            SHORT SpecialApcDisable;
        };

        ULONG CombinedApcDisable;
    };

#endif

    PVOID Teb;
    union {
        KTIMER Timer;
        struct {
            UCHAR TimerFill[KTIMER_ACTUAL_LENGTH];

            //
            // N.B. The following bit number definitions must match the
            //      following bit field.
            //
            // N.B. These bits can only be written with interlocked
            //      operations.
            //
    
#define KTHREAD_AUTO_ALIGNMENT_BIT 0
#define KTHREAD_DISABLE_BOOST_BIT 1
    
            union {
                struct {
                    LONG AutoAlignment : 1;
                    LONG DisableBoost : 1;
                    LONG ReservedFlags : 30;
                };
        
                LONG ThreadFlags;
            };
        };
    };

    union {
        KWAIT_BLOCK WaitBlock[THREAD_WAIT_OBJECTS + 1];
        struct {
            UCHAR WaitBlockFill0[KWAIT_BLOCK_OFFSET_TO_BYTE0];
            BOOLEAN SystemAffinityActive;
        };

        struct {
            UCHAR WaitBlockFill1[KWAIT_BLOCK_OFFSET_TO_BYTE1];
            CCHAR PreviousMode;
        };

        struct {
            UCHAR WaitBlockFill2[KWAIT_BLOCK_OFFSET_TO_BYTE2];
            UCHAR ResourceIndex;
        };

        struct {
            UCHAR WaitBlockFill3[KWAIT_BLOCK_OFFSET_TO_BYTE3];
            UCHAR LargeStack;
        };

#if defined(_AMD64_)

        struct {
            UCHAR WaitBlockFill4[KWAIT_BLOCK_OFFSET_TO_LONG0];
            ULONG ContextSwitches;
        };

        struct {
            UCHAR WaitBlockFill5[KWAIT_BLOCK_OFFSET_TO_LONG1];
            volatile UCHAR State;
            UCHAR NpxState;
            KIRQL WaitIrql;
            KPROCESSOR_MODE WaitMode;
        };

        struct {
            UCHAR WaitBlockFill6[KWAIT_BLOCK_OFFSET_TO_LONG2];
            ULONG WaitTime;
        };

        struct {
            UCHAR WaitBlockFill7[KWAIT_BLOCK_OFFSET_TO_LONG3];
             union {
                 struct {
                     SHORT KernelApcDisable;
                     SHORT SpecialApcDisable;
                 };
         
                 ULONG CombinedApcDisable;
             };
        };

#endif

    };

    LIST_ENTRY QueueListEntry;

    //
    // The following fields are accessed during system service dispatch.
    //

    PKTRAP_FRAME TrapFrame;
    PVOID CallbackStack;
    PVOID ServiceTable;

#if defined(_AMD64_)

    ULONG KernelLimit;

#endif

    //
    // The following fields are referenced during ready thread and wait
    // completion.
    //

    UCHAR ApcStateIndex;
    UCHAR IdealProcessor;
    BOOLEAN Preempted;
    BOOLEAN ProcessReadyQueue;

#if defined(_AMD64_)

    PVOID Win32kTable;
    ULONG Win32kLimit;

#endif

    BOOLEAN KernelStackResident;
    SCHAR BasePriority;
    SCHAR PriorityDecrement;
    CHAR Saturation;
    KAFFINITY UserAffinity;
    PKPROCESS Process;
    KAFFINITY Affinity;

    //
    // The below fields are infrequently referenced.
    //

    PKAPC_STATE ApcStatePointer[2];
    union {
        KAPC_STATE SavedApcState;
        struct {
            UCHAR SavedApcStateFill[KAPC_STATE_ACTUAL_LENGTH];
            CCHAR FreezeCount;
            CCHAR SuspendCount;
            UCHAR UserIdealProcessor;
            UCHAR CalloutActive;

#if defined(_AMD64_)

            BOOLEAN CodePatchInProgress;

#elif defined(_X86_)

            UCHAR Iopl;

#else

            UCHAR OtherPlatformFill;

#endif

        };
    };

    PVOID Win32Thread;
    PVOID StackBase;
    union {
        KAPC SuspendApc;
        struct {
            UCHAR SuspendApcFill0[KAPC_OFFSET_TO_SPARE_BYTE0];
            SCHAR Quantum;
        };

        struct {
            UCHAR SuspendApcFill1[KAPC_OFFSET_TO_SPARE_BYTE1];
            UCHAR QuantumReset;
        };

        struct {
            UCHAR SuspendApcFill2[KAPC_OFFSET_TO_SPARE_LONG];
            ULONG KernelTime;
        };

        struct {
            UCHAR SuspendApcFill3[KAPC_OFFSET_TO_SYSTEMARGUMENT1];
            PVOID TlsArray;
        };

        struct {
            UCHAR SuspendApcFill4[KAPC_OFFSET_TO_SYSTEMARGUMENT2];
            PVOID BBTData;
        };

        struct {
            UCHAR SuspendApcFill5[KAPC_ACTUAL_LENGTH];
            UCHAR PowerState;
            ULONG UserTime;
        };
    };

    union {
        KSEMAPHORE SuspendSemaphore;
        struct {
            UCHAR SuspendSemaphorefill[KSEMAPHORE_ACTUAL_LENGTH];
            ULONG SListFaultCount;
        };
    };

    LIST_ENTRY ThreadListEntry;
    PVOID SListFaultAddress;

#if defined(_WIN64)

    LONG64 ReadOperationCount;
    LONG64 WriteOperationCount;
    LONG64 OtherOperationCount;
    LONG64 ReadTransferCount;
    LONG64 WriteTransferCount;
    LONG64 OtherTransferCount;

#endif

} KTHREAD, *PKTHREAD, *PRKTHREAD;

#if !defined(_X86AMD64_) && defined(_AMD64_)

C_ASSERT((FIELD_OFFSET(KTHREAD, ServiceTable) + 16) == FIELD_OFFSET(KTHREAD, Win32kTable));
C_ASSERT((FIELD_OFFSET(KTHREAD, ServiceTable) + 8) == FIELD_OFFSET(KTHREAD, KernelLimit));
C_ASSERT((FIELD_OFFSET(KTHREAD, Win32kTable) + 8) == FIELD_OFFSET(KTHREAD, Win32kLimit));

#endif


//
// ccNUMA supported in multiprocessor PAE and WIN64 systems only.
//

#if (defined(_WIN64) || defined(_X86PAE_)) && !defined(NT_UP)

#define KE_MULTINODE

#endif

//
// Profile object structure definition
//

typedef struct _KPROFILE {
    CSHORT Type;
    CSHORT Size;
    LIST_ENTRY ProfileListEntry;
    PKPROCESS Process;
    PVOID RangeBase;
    PVOID RangeLimit;
    ULONG BucketShift;
    PVOID Buffer;
    ULONG Segment;
    KAFFINITY Affinity;
    CSHORT Source;
    BOOLEAN Started;
} KPROFILE, *PKPROFILE, *PRKPROFILE;

//
// Kernel control object functions
//
// APC object
//

// begin_ntosp

NTKERNELAPI
VOID
KeInitializeApc (
    __out PRKAPC Apc,
    __in PRKTHREAD Thread,
    __in KAPC_ENVIRONMENT Environment,
    __in PKKERNEL_ROUTINE KernelRoutine,
    __in_opt PKRUNDOWN_ROUTINE RundownRoutine,
    __in_opt PKNORMAL_ROUTINE NormalRoutine,
    __in_opt KPROCESSOR_MODE ProcessorMode,
    __in_opt PVOID NormalContext
    );

PLIST_ENTRY
KeFlushQueueApc (
    __inout PKTHREAD Thread,
    __in KPROCESSOR_MODE ProcessorMode
    );

NTKERNELAPI
BOOLEAN
KeInsertQueueApc (
    __inout PRKAPC Apc,
    __in_opt PVOID SystemArgument1,
    __in_opt PVOID SystemArgument2,
    __in KPRIORITY Increment
    );

BOOLEAN
KeRemoveQueueApc (
    __inout PKAPC Apc
    );

VOID
KeGenericCallDpc (
    __in PKDEFERRED_ROUTINE Routine,
    __in_opt PVOID Context
    );

VOID
KeSignalCallDpcDone (
    __in PVOID SystemArgument1
    );

LOGICAL
KeSignalCallDpcSynchronize (
    __in PVOID SystemArgument2
    );

// end_ntosp

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp
//
// DPC object
//

NTKERNELAPI
VOID
KeInitializeDpc (
    __out PRKDPC Dpc,
    __in PKDEFERRED_ROUTINE DeferredRoutine,
    __in_opt PVOID DeferredContext
    );

// end_ntddk end_wdm end_nthal end_ntifs

NTKERNELAPI
VOID
KeInitializeThreadedDpc (
    __out PRKDPC Dpc,
    __in PKDEFERRED_ROUTINE DeferredRoutine,
    __in_opt PVOID DeferredContext
    );

// begin_ntddk begin_wdm begin_nthal begin_ntifs

NTKERNELAPI
BOOLEAN
KeInsertQueueDpc (
    __inout PRKDPC Dpc,
    __in_opt PVOID SystemArgument1,
    __in_opt PVOID SystemArgument2
    );

NTKERNELAPI
BOOLEAN
KeRemoveQueueDpc (
    __inout PRKDPC Dpc
    );

// end_wdm

// end_ntddk end_ntifs end_nthal

#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)

// begin_ntddk begin_ntifs begin_nthal

NTKERNELAPI
VOID
KeSetImportanceDpc (
    __inout PRKDPC Dpc,
    __in KDPC_IMPORTANCE Importance
    );

NTKERNELAPI
VOID
KeSetTargetProcessorDpc (
    __inout PRKDPC Dpc,
    __in CCHAR Number
    );

// end_ntddk end_ntifs end_nthal

#else

FORCEINLINE
VOID
KeSetImportanceDpc (
    __inout PRKDPC Dpc,
    __in KDPC_IMPORTANCE Importance
    )

/*++

Routine Description:

    This function sets the importance of a DPC.

Arguments:

    Dpc - Supplies a pointer to a control object of type DPC.

    Number - Supplies the importance of the DPC.

Return Value:

    None.

--*/

{

    ASSERT_DPC(Dpc);

    //
    // Set the importance of the DPC.
    //

    Dpc->Importance = (UCHAR)Importance;
    return;
}

FORCEINLINE
VOID
KeSetTargetProcessorDpc (
    __inout PRKDPC Dpc,
    __in CCHAR Number
    )

/*++

Routine Description:

    This function sets the processor number to which the DPC is targeted.

Arguments:

    Dpc - Supplies a pointer to a control object of type DPC.

    Number - Supplies the target processor number.

Return Value:

    None.

--*/

{

    ASSERT_DPC(Dpc);

    //
    // The target processor number is biased by the maximum number of
    // processors that are supported.
    //

    Dpc->Number = MAXIMUM_PROCESSORS + Number;
    return;
}

#endif

// begin_ntddk begin_ntifs begin_nthal

// begin_wdm

NTKERNELAPI
VOID
KeFlushQueuedDpcs (
    VOID
    );

//
// Device queue object
//

NTKERNELAPI
VOID
KeInitializeDeviceQueue (
    __out PKDEVICE_QUEUE DeviceQueue
    );

NTKERNELAPI
BOOLEAN
KeInsertDeviceQueue (
    __inout PKDEVICE_QUEUE DeviceQueue,
    __inout PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
    );

NTKERNELAPI
BOOLEAN
KeInsertByKeyDeviceQueue (
    __inout PKDEVICE_QUEUE DeviceQueue,
    __inout PKDEVICE_QUEUE_ENTRY DeviceQueueEntry,
    __in ULONG SortKey
    );

NTKERNELAPI
PKDEVICE_QUEUE_ENTRY
KeRemoveDeviceQueue (
    __inout PKDEVICE_QUEUE DeviceQueue
    );

NTKERNELAPI
PKDEVICE_QUEUE_ENTRY
KeRemoveByKeyDeviceQueue (
    __inout PKDEVICE_QUEUE DeviceQueue,
    __in ULONG SortKey
    );

NTKERNELAPI
PKDEVICE_QUEUE_ENTRY
KeRemoveByKeyDeviceQueueIfBusy (
    __inout PKDEVICE_QUEUE DeviceQueue,
    __in ULONG SortKey
    );

NTKERNELAPI
BOOLEAN
KeRemoveEntryDeviceQueue (
    __inout PKDEVICE_QUEUE DeviceQueue,
    __inout PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
    );

// end_ntddk end_wdm end_ntifs end_ntosp

//
// Interrupt object
//

NTKERNELAPI                                         
VOID                                                
KeInitializeInterrupt (                             
    __out PKINTERRUPT Interrupt,                       
    __in PKSERVICE_ROUTINE ServiceRoutine,            
    __in_opt PVOID ServiceContext,                        
    __out_opt PKSPIN_LOCK SpinLock,               
    __in ULONG Vector,                                
    __in KIRQL Irql,                                  
    __in KIRQL SynchronizeIrql,                       
    __in KINTERRUPT_MODE InterruptMode,               
    __in BOOLEAN ShareVector,                         
    __in CCHAR ProcessorNumber,                       
    __in BOOLEAN FloatingSave                         
    );

#if defined(_AMD64_)

#define NO_INTERRUPT_SPINLOCK ((PKSPIN_LOCK)-1I64)
#define NO_END_OF_INTERRUPT ((PKSPIN_LOCK)-2I64)
#define INTERRUPT_PERFORMANCE ((PKSPIN_LOCK)-3I64)

#endif

                                                    
NTKERNELAPI                                         
BOOLEAN                                             
KeConnectInterrupt (                                
    __inout PKINTERRUPT Interrupt                        
    );                                              

// end_nthal

NTKERNELAPI
BOOLEAN
KeDisconnectInterrupt (
    __inout PKINTERRUPT Interrupt
    );

// begin_ntddk begin_wdm begin_nthal begin_ntosp

NTKERNELAPI
BOOLEAN
KeSynchronizeExecution (
    __inout PKINTERRUPT Interrupt,
    __in PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    __in_opt PVOID SynchronizeContext
    );

NTKERNELAPI
KIRQL
KeAcquireInterruptSpinLock (
    __inout PKINTERRUPT Interrupt
    );

NTKERNELAPI
VOID
KeReleaseInterruptSpinLock (
    __inout PKINTERRUPT Interrupt,
    __in KIRQL OldIrql
    );

// end_ntddk end_wdm end_nthal end_ntosp

//
// Profile object
//

VOID
KeInitializeProfile (
    __out PKPROFILE Profile,
    __in_opt PKPROCESS Process,
    __in_opt PVOID RangeBase,
    __in SIZE_T RangeSize,
    __in ULONG BucketSize,
    __in ULONG Segment,
    __in KPROFILE_SOURCE ProfileSource,
    __in KAFFINITY Affinity
    );

BOOLEAN
KeStartProfile (
    __inout PKPROFILE Profile,
    __out_opt PULONG Buffer
    );

BOOLEAN
KeStopProfile (
    __inout PKPROFILE Profile
    );

VOID
KeSetIntervalProfile (
    __in ULONG Interval,
    __in KPROFILE_SOURCE Source
    );

ULONG
KeQueryIntervalProfile (
    __in KPROFILE_SOURCE Source
    );

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp
//
// Kernel dispatcher object functions
//
// Event Object
//

// end_wdm end_ntddk end_nthal end_ntifs end_ntosp

#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)

// begin_wdm begin_ntddk begin_nthal begin_ntifs begin_ntosp

NTKERNELAPI
VOID
KeInitializeEvent (
    __out PRKEVENT Event,
    __in EVENT_TYPE Type,
    __in BOOLEAN State
    );

NTKERNELAPI
VOID
KeClearEvent (
    __inout PRKEVENT Event
    );

// end_wdm end_ntddk end_nthal end_ntifs end_ntosp

#else

#define KeInitializeEvent(_Event, _Type, _State)            \
    (_Event)->Header.Type = (UCHAR)_Type;                   \
    (_Event)->Header.Size =  sizeof(KEVENT) / sizeof(LONG); \
    (_Event)->Header.SignalState = _State;                  \
    InitializeListHead(&(_Event)->Header.WaitListHead)

#define KeClearEvent(Event) (Event)->Header.SignalState = 0

#endif

// begin_ntddk begin_ntifs begin_ntosp

NTKERNELAPI
LONG
KePulseEvent (
    __inout PRKEVENT Event,
    __in KPRIORITY Increment,
    __in BOOLEAN Wait
    );

// end_ntddk end_ntifs end_ntosp

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp

NTKERNELAPI
LONG
KeReadStateEvent (
    __in PRKEVENT Event
    );

NTKERNELAPI
LONG
KeResetEvent (
    __inout PRKEVENT Event
    );

NTKERNELAPI
LONG
KeSetEvent (
    __inout PRKEVENT Event,
    __in KPRIORITY Increment,
    __in BOOLEAN Wait
    );

// end_ntddk end_wdm end_nthal end_ntifs end_ntosp

VOID
KeSetEventBoostPriority (
    __inout PRKEVENT Event,
    __in_opt PRKTHREAD *Thread
    );

VOID
KeInitializeEventPair (
    __inout PKEVENT_PAIR EventPair
    );

#define KeSetHighEventPair(EventPair, Increment, Wait) \
    KeSetEvent(&((EventPair)->EventHigh),              \
               Increment,                              \
               Wait)

#define KeSetLowEventPair(EventPair, Increment, Wait)  \
    KeSetEvent(&((EventPair)->EventLow),               \
               Increment,                              \
               Wait)

//
// Mutant object
//
// begin_ntifs

NTKERNELAPI
VOID
KeInitializeMutant (
    __out PRKMUTANT Mutant,
    __in BOOLEAN InitialOwner
    );

LONG
KeReadStateMutant (
    __in PRKMUTANT Mutant
    );

NTKERNELAPI
LONG
KeReleaseMutant (
    __inout PRKMUTANT Mutant,
    __in KPRIORITY Increment,
    __in BOOLEAN Abandoned,
    __in BOOLEAN Wait
    );

// begin_ntddk begin_wdm begin_nthal begin_ntosp
//
// Mutex object
//

NTKERNELAPI
VOID
KeInitializeMutex (
    __out PRKMUTEX Mutex,
    __in ULONG Level
    );

NTKERNELAPI
LONG
KeReadStateMutex (
    __in PRKMUTEX Mutex
    );

NTKERNELAPI
LONG
KeReleaseMutex (
    __inout PRKMUTEX Mutex,
    __in BOOLEAN Wait
    );

// end_ntddk end_wdm
//
// Queue Object.
//

NTKERNELAPI
VOID
KeInitializeQueue (
    __out PRKQUEUE Queue,
    __in ULONG Count
    );

NTKERNELAPI
LONG
KeReadStateQueue (
    __in PRKQUEUE Queue
    );

NTKERNELAPI
LONG
KeInsertQueue (
    __inout PRKQUEUE Queue,
    __inout PLIST_ENTRY Entry
    );

NTKERNELAPI
LONG
KeInsertHeadQueue (
    __inout PRKQUEUE Queue,
    __inout PLIST_ENTRY Entry
    );

NTKERNELAPI
PLIST_ENTRY
KeRemoveQueue (
    __inout PRKQUEUE Queue,
    __in KPROCESSOR_MODE WaitMode,
    __in_opt PLARGE_INTEGER Timeout
    );

NTKERNELAPI
PLIST_ENTRY
KeRundownQueue (
    __inout PRKQUEUE Queue
    );

// begin_ntddk begin_wdm
//
// Semaphore object
//

NTKERNELAPI
VOID
KeInitializeSemaphore (
    __out PRKSEMAPHORE Semaphore,
    __in LONG Count,
    __in LONG Limit
    );

NTKERNELAPI
LONG
KeReadStateSemaphore (
    __in PRKSEMAPHORE Semaphore
    );

NTKERNELAPI
LONG
KeReleaseSemaphore (
    __inout PRKSEMAPHORE Semaphore,
    __in KPRIORITY Increment,
    __in LONG Adjustment,
    __in BOOLEAN Wait
    );

// end_ntddk end_wdm end_nthal end_ntifs end_ntosp

//
// Process object
//

VOID
KeInitializeProcess (
    __out PRKPROCESS Process,
    __in KPRIORITY Priority,
    __in KAFFINITY Affinity,
    __in ULONG_PTR DirectoryTableBase[2],
    __in BOOLEAN Enable
    );

LOGICAL
KeForceAttachProcess (
    __inout PKPROCESS Process
    );

// begin_ntifs begin_ntosp

NTKERNELAPI
VOID
KeAttachProcess (
    __inout PRKPROCESS Process
    );

NTKERNELAPI
VOID
KeDetachProcess (
    VOID
    );

NTKERNELAPI
VOID
KeStackAttachProcess (
    __inout PRKPROCESS PROCESS,
    __out PRKAPC_STATE ApcState
    );

NTKERNELAPI
VOID
KeUnstackDetachProcess (
    __in PRKAPC_STATE ApcState
    );

// end_ntifs end_ntosp

#define KiIsAttachedProcess() \
    (KeGetCurrentThread()->ApcStateIndex == AttachedApcEnvironment)

#if !defined(_NTOSP_)

#define KeIsAttachedProcess() KiIsAttachedProcess()

#else

// begin_ntosp

NTKERNELAPI
BOOLEAN
KeIsAttachedProcess(
    VOID
    );

// end_ntosp

#endif

ULONG
KeQueryRuntimeProcess (
    __in PKPROCESS Process,
    __out PULONG UserTime
    );

typedef struct _KPROCESS_VALUES {
    ULONG64 KernelTime;
    ULONG64 UserTime;
    LONG64 ReadOperationCount;
    LONG64 WriteOperationCount;
    LONG64 OtherOperationCount;
    LONG64 ReadTransferCount;
    LONG64 WriteTransferCount;
    LONG64 OtherTransferCount;
} KPROCESS_VALUES, *PKPROCESS_VALUES;

VOID
KeQueryValuesProcess (
    __in PKPROCESS Process,
    __out PKPROCESS_VALUES Values
    );

LONG
KeReadStateProcess (
    __in PKPROCESS Process
    );

LOGICAL
KeSetAutoAlignmentProcess (
    __inout PKPROCESS Process,
    __in LOGICAL Enable
    );

LONG
KeSetProcess (
    __inout PKPROCESS Process,
    __in KPRIORITY Increment,
    __in BOOLEAN Wait
    );

KAFFINITY
KeSetAffinityProcess (
    __inout PKPROCESS Process,
    __in KAFFINITY Affinity
    );

KPRIORITY
KeSetPriorityAndQuantumProcess (
    __inout PKPROCESS Process,
    __in KPRIORITY BasePriority,
    __in SCHAR QuantumReset
    );

VOID
KeSetQuantumProcess (
    __inout PKPROCESS Process,
    __in SCHAR QuantumReset
    );

LOGICAL
KeSetDisableBoostProcess (
    __inout PKPROCESS Process,
    __in LOGICAL Disable
    );

LOGICAL
KeSetDisableQuantumProcess (
    __inout PKPROCESS Process,
    __in LOGICAL Disable
    );

#define KeTerminateProcess(Process) \
    (Process)->StackCount += 1;

//
// Gate object
//

VOID
FASTCALL
KeInitializeGate (
    __out PKGATE Gate
    );

VOID
FASTCALL
KeSignalGateBoostPriority (
    __inout PKGATE Gate
    );

VOID
FASTCALL
KeWaitForGate (
    __inout PKGATE Gate,
    __in KWAIT_REASON WaitReason,
    __in KPROCESSOR_MODE WaitMode
    );

//
// Thread object
//

NTSTATUS
KeInitializeThread (
    __out PKTHREAD Thread,
    __in_opt PVOID KernelStack,
    __in PKSYSTEM_ROUTINE SystemRoutine,
    __in_opt PKSTART_ROUTINE StartRoutine,
    __in_opt PVOID StartContext,
    __in_opt PCONTEXT ContextFrame,
    __in_opt PVOID Teb,
    __in PKPROCESS Process
    );

NTSTATUS
KeInitThread (
    __out PKTHREAD Thread,
    __in_opt PVOID KernelStack,
    __in PKSYSTEM_ROUTINE SystemRoutine,
    __in_opt PKSTART_ROUTINE StartRoutine,
    __in_opt PVOID StartContext,
    __in_opt PCONTEXT ContextFrame,
    __in_opt PVOID Teb,
    __in PKPROCESS Process
    );

VOID
KeUninitThread (
    __inout PKTHREAD Thread
    );

VOID
KeStartThread (
    __inout PKTHREAD Thread
    );

BOOLEAN
KeAlertThread (
    __inout PKTHREAD Thread,
    __in KPROCESSOR_MODE ProcessorMode
    );

ULONG
KeAlertResumeThread (
    __inout PKTHREAD Thread
    );

VOID
KeBoostPriorityThread (
    __inout PKTHREAD Thread,
    __in KPRIORITY Increment
    );

// begin_ntosp

NTKERNELAPI                                         // ntddk wdm nthal ntifs
NTSTATUS                                            // ntddk wdm nthal ntifs
KeDelayExecutionThread (                            // ntddk wdm nthal ntifs
    __in KPROCESSOR_MODE WaitMode,                  // ntddk wdm nthal ntifs
    __in BOOLEAN Alertable,                         // ntddk wdm nthal ntifs
    __in PLARGE_INTEGER Interval                    // ntddk wdm nthal ntifs
    );                                              // ntddk wdm nthal ntifs
                                                    // ntddk wdm nthal ntifs
// end_ntosp

#if defined(_AMD64_)

ULONG_PTR
KeGetCurrentStackPointer (
    VOID
    );

VOID
KeCheckIfStackExpandCalloutActive (
    VOID
    );

FORCEINLINE
VOID
KeGetActualStackLimits (
    __out PULONG64 LowLimit,
    __out PULONG64 HighLimit
    )

/*++

Routine Description:

    This function returns the actual stack limits of the current thread.

    N.B. The actual stack limits are not stored in the thread object.

Arguments:

    LowLimit - Supplies a pointer to a variable that receives the low stack
        limit.

    HighLimit - Supplies a pointer to a variable that receives the high stack
        limit.

Return Value:

    None.

--*/

{

    PKERNEL_STACK_CONTROL StackControl;

    StackControl = (PKERNEL_STACK_CONTROL)(KeGetCurrentThread()->InitialStack);
    *LowLimit = StackControl->Current.ActualLimit;
    *HighLimit = StackControl->Current.StackBase;
    return;
}

FORCEINLINE
PKERNEL_STACK_SEGMENT
KeGetFirstKernelStackSegment (
    __in PKTHREAD Thread
    )

/*++

Routine Description:

    This function returns a pointer to the first kernel stack segment for an
    in-memory thread.

Arguments:

    Thread - Supplies a pointer to a kernel thread object.

Return Value:

    A pointer to a kernel stack segment structure is returned as the function
    value.

--*/

{

    PKERNEL_STACK_CONTROL StackControl;

    StackControl = (PKERNEL_STACK_CONTROL)Thread->InitialStack;
    StackControl->Current.StackLimit = (ULONG64)Thread->StackLimit;
    StackControl->Current.KernelStack = (ULONG64)Thread->KernelStack;
    return &StackControl->Current;
}
    
FORCEINLINE
PKERNEL_STACK_SEGMENT
KeGetNextKernelStackSegment (
    __in PKERNEL_STACK_SEGMENT StackSegment
    )

/*++

Routine Description:

    This function returns a pointer to the next kernel stack segment for
    an in-memory thread.

Arguments:

    StackStack - Supplies a pointer to a kernel stack segment structure
        that was either returned as the first or subsequent kernel stack
        segment structure address.

Return Value:

    If another kernel stack segment is required to describe the kernel stack
    of the specified thread, then a pointer to the next kernel stack segment
    structure is returned as the function value. Otherwise, a value of NULL
    is returned.

--*/

{

    PKERNEL_STACK_SEGMENT PreviousSegment;
    PKERNEL_STACK_CONTROL StackControl;

    PreviousSegment = StackSegment + 1;
    if (PreviousSegment->StackBase == 0) {
        return NULL;

    } else {
        StackControl = (PKERNEL_STACK_CONTROL)PreviousSegment->InitialStack;
        StackControl->Current.StackLimit = PreviousSegment->StackLimit;
        StackControl->Current.KernelStack = PreviousSegment->KernelStack;
        return &StackControl->Current;
    }
}

FORCEINLINE
BOOLEAN
KeIsKernelStackTrimable (
    __in PKTHREAD Thread
    )

/*++

Routine Description:

    This function determines whether the kernel stack for the specified
    thread is trimable.

Arguments:

    Thread - Supplies a pointer to a kernel thread object.

Return Value:

    A value of TRUE is returned if the kernel stack of the specified thread
    is trimable. Otherwise, a value of FALSE is returned.

--*/

{

    return (BOOLEAN)((Thread->LargeStack == TRUE) && (Thread->CalloutActive == FALSE));
}

typedef enum _KERNEL_STACK_LIMITS {
    BugcheckStackLimits,
    DPCStackLimits,
    ExpandedStackLimits,
    NormalStackLimits,
    Win32kStackLimits,
    MaximumStackLimits
} KERNEL_STACK_LIMITS, *PKERNEL_STACK_LIMITS;

BOOLEAN
KeQueryCurrentStackInformation (
    __out PKERNEL_STACK_LIMITS Type,
    __out PULONG64 LowLimit,
    __out PULONG64 HighLimit
    );

#endif

// begin_ntosp begin_ntddk begin_ntifs

#if defined(_AMD64_)

#define MAXIMUM_EXPANSION_SIZE (KERNEL_LARGE_STACK_SIZE - (PAGE_SIZE / 2))

typedef
VOID
(*PEXPAND_STACK_CALLOUT) (
    __in_opt PVOID Parameter
    );

NTKERNELAPI
NTSTATUS
KeExpandKernelStackAndCallout (
    __in PEXPAND_STACK_CALLOUT Callout,
    __in_opt PVOID Parameter,
    __in SIZE_T Size
    );

#endif

// end_ntosp end_ntddk end_ntifs

LOGICAL
KeSetDisableBoostThread (
    __inout PKTHREAD Thread,
    __in LOGICAL Disable
    );

ULONG
KeForceResumeThread (
    __inout PKTHREAD Thread
    );

VOID
KeFreezeAllThreads (
    VOID
    );

LOGICAL
KeQueryAutoAlignmentThread (
    __in PKTHREAD Thread
    );

LONG
KeQueryBasePriorityThread (
    __in PKTHREAD Thread
    );

NTKERNELAPI                                         // ntddk wdm nthal ntifs
KPRIORITY                                           // ntddk wdm nthal ntifs
KeQueryPriorityThread (                             // ntddk wdm nthal ntifs
    __in PKTHREAD Thread                            // ntddk wdm nthal ntifs
    );                                              // ntddk wdm nthal ntifs
                                                    // ntddk wdm nthal ntifs
NTKERNELAPI                                         // ntddk wdm nthal ntifs
ULONG                                               // ntddk wdm nthal ntifs
KeQueryRuntimeThread (                              // ntddk wdm nthal ntifs
    __in PKTHREAD Thread,                           // ntddk wdm nthal ntifs
    __out PULONG UserTime                           // ntddk wdm nthal ntifs
    );                                              // ntddk wdm nthal ntifs
                                                    // ntddk wdm nthal ntifs
BOOLEAN
KeReadStateThread (
    __in PKTHREAD Thread
    );

VOID
KeReadyThread (
    __inout PKTHREAD Thread
    );

ULONG
KeResumeThread (
    __inout PKTHREAD Thread
    );

// begin_nthal begin_ntosp

NTKERNELAPI
VOID
KeRevertToUserAffinityThread (
    VOID
    );

// end_nthal end_ntosp

VOID
KeRundownThread (
    VOID
    );

KAFFINITY
KeSetAffinityThread (
    __inout PKTHREAD Thread,
    __in KAFFINITY Affinity
    );

// begin_nthal begin_ntosp

NTKERNELAPI
VOID
KeSetSystemAffinityThread (
    __in KAFFINITY Affinity
    );

// end_nthal end_ntosp

LOGICAL
KeSetAutoAlignmentThread (
    __inout PKTHREAD Thread,
    __in LOGICAL Enable
    );

NTKERNELAPI                                         // ntddk nthal ntifs ntosp
LONG                                                // ntddk nthal ntifs ntosp
KeSetBasePriorityThread (                           // ntddk nthal ntifs ntosp
    __inout PKTHREAD Thread,                        // ntddk nthal ntifs ntosp
    __in LONG Increment                             // ntddk nthal ntifs ntosp
    );                                              // ntddk nthal ntifs ntosp
                                                    // ntddk nthal ntifs ntosp

// begin_ntifs

NTKERNELAPI
UCHAR
KeSetIdealProcessorThread (
    __inout PKTHREAD Thread,
    __in UCHAR Processor
    );

// begin_ntosp
NTKERNELAPI
BOOLEAN
KeSetKernelStackSwapEnable (
    __in BOOLEAN Enable
    );

// end_ntifs

NTKERNELAPI                                         // ntddk wdm nthal ntifs
KPRIORITY                                           // ntddk wdm nthal ntifs
KeSetPriorityThread (                               // ntddk wdm nthal ntifs
    __inout PKTHREAD Thread,                        // ntddk wdm nthal ntifs
    __in KPRIORITY Priority                         // ntddk wdm nthal ntifs
    );                                              // ntddk wdm nthal ntifs
                                                    // ntddk wdm nthal ntifs

// end_ntosp

VOID
KeSetPriorityZeroPageThread (
    __in KPRIORITY Priority
    );

ULONG
KeSuspendThread (
    __inout PKTHREAD Thread
    );

NTKERNELAPI
VOID
KeTerminateThread (
    __in KPRIORITY Increment
    );

BOOLEAN
KeTestAlertThread (
    __in KPROCESSOR_MODE
    );

VOID
KeThawAllThreads (
    VOID
    );

// begin_ntddk begin_nthal begin_ntifs begin_ntosp

#if ((defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)) && !defined(_NTSYSTEM_DRIVER_) || defined(_NTOSP_))

// begin_wdm

NTKERNELAPI
VOID
KeEnterCriticalRegion (
    VOID
    );

NTKERNELAPI
VOID
KeLeaveCriticalRegion (
    VOID
    );

NTKERNELAPI
VOID
KeEnterGuardedRegion (
    VOID
    );

NTKERNELAPI
VOID
KeLeaveGuardedRegion (
    VOID
    );

NTKERNELAPI
BOOLEAN
KeAreApcsDisabled (
    VOID
    );

// end_wdm

#endif

// begin_wdm

//
// Timer object
//

NTKERNELAPI
VOID
KeInitializeTimer (
    __out PKTIMER Timer
    );

NTKERNELAPI
VOID
KeInitializeTimerEx (
    __out PKTIMER Timer,
    __in TIMER_TYPE Type
    );

NTKERNELAPI
BOOLEAN
KeCancelTimer (
    __inout PKTIMER
    );

NTKERNELAPI
BOOLEAN
KeReadStateTimer (
    __in PKTIMER Timer
    );

NTKERNELAPI
BOOLEAN
KeSetTimer (
    __inout PKTIMER Timer,
    __in LARGE_INTEGER DueTime,
    __in_opt PKDPC Dpc
    );

NTKERNELAPI
BOOLEAN
KeSetTimerEx (
    __inout PKTIMER Timer,
    __in LARGE_INTEGER DueTime,
    __in LONG Period,
    __in_opt PKDPC Dpc
    );

// end_ntddk end_nthal end_ntifs end_wdm end_ntosp

extern volatile KAFFINITY KiIdleSummary;

FORCEINLINE
BOOLEAN
KeIsSMTSetIdle (
    __in PKPRCB Prcb
    )

/*++

Routine Description:

    This routine determines whether the complete SMT set associated with the
    specified processor is idle.

Arguments:

    Prcb - Supplies a pointer to a processor control block (PRCB).

Return Value:

    If the specified SMT set is idle, then TRUE is returned. Otherwise, FALSE
    is returned.

--*/

{

#if !defined(NT_UP) && defined(NT_SMT)

    if ((KiIdleSummary & Prcb->MultiThreadProcessorSet) == Prcb->MultiThreadProcessorSet) {
        return TRUE;

    } else {
        return FALSE;
    }

#else

    UNREFERENCED_PARAMETER(Prcb);

    return TRUE;

#endif

}

/*++

KPROCESSOR_MODE
KeGetPreviousMode (
    VOID
    )

Routine Description:

    This function gets the threads previous mode from the trap frame


Arguments:

   None.

Return Value:

    KPROCESSOR_MODE - Previous mode for this thread.

--*/

#define KeGetPreviousMode() (KeGetCurrentThread()->PreviousMode)

/*++

KPROCESSOR_MODE
KeGetPReviousModeByThread (
    __in PKTHREAD xxCurrentThread
    )

Routine Description:

    This function gets the threads previous mode from the trap frame.


Arguments:

   xxCurrentThread - Current thread.

   N.B. This must be the current thread.

Return Value:

    KPROCESSOR_MODE - Previous mode for this thread.

--*/

#define KeGetPreviousModeByThread(xxCurrentThread)                          \
    (ASSERT (xxCurrentThread == KeGetCurrentThread ()),                     \
    (xxCurrentThread)->PreviousMode)

VOID
KeCheckForTimer(
    __in_bcount(BlockSize) PVOID BlockStart,
    __in SIZE_T BlockSize
    );

VOID
KeClearTimer (
    __inout PKTIMER Timer
    );

ULONGLONG
KeQueryTimerDueTime (
    __in PKTIMER Timer
    );

//
// Wait functions
//

NTSTATUS
KiSetServerWaitClientEvent (
    __inout PKEVENT SeverEvent,
    __inout PKEVENT ClientEvent,
    __in ULONG WaitMode
    );

#define KeSetHighWaitLowEventPair(EventPair, WaitMode)                       \
    KiSetServerWaitClientEvent(&((EventPair)->EventHigh),                    \
                               &((EventPair)->EventLow),                     \
                               WaitMode)

#define KeSetLowWaitHighEventPair(EventPair, WaitMode)                       \
    KiSetServerWaitClientEvent(&((EventPair)->EventLow),                     \
                               &((EventPair)->EventHigh),                    \
                               WaitMode)

#define KeWaitForHighEventPair(EventPair, WaitMode, Alertable, TimeOut)      \
    KeWaitForSingleObject(&((EventPair)->EventHigh),                         \
                          WrEventPair,                                       \
                          WaitMode,                                          \
                          Alertable,                                         \
                          TimeOut)

#define KeWaitForLowEventPair(EventPair, WaitMode, Alertable, TimeOut)       \
    KeWaitForSingleObject(&((EventPair)->EventLow),                          \
                          WrEventPair,                                       \
                          WaitMode,                                          \
                          Alertable,                                         \
                          TimeOut)

FORCEINLINE
VOID
KeWaitForContextSwap (
    __in PKTHREAD Thread
    )

/*++

Routine Description:

    This routine waits until context swap is idle for the specified thread.

Arguments:

    Thread - Supplies a pointer to a dispatcher object of type thread.

Return Value:

    None.

--*/

{

#if !defined(NT_UP)

    while (Thread->SwapBusy != FALSE) {
        KeYieldProcessor();
    }

#else

    UNREFERENCED_PARAMETER(Thread);

#endif

    return;
}

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp

#define KeWaitForMutexObject KeWaitForSingleObject

NTKERNELAPI
NTSTATUS
KeWaitForMultipleObjects (
    __in ULONG Count,
    __in_ecount(Count) PVOID Object[],
    __in WAIT_TYPE WaitType,
    __in KWAIT_REASON WaitReason,
    __in KPROCESSOR_MODE WaitMode,
    __in BOOLEAN Alertable,
    __in_opt PLARGE_INTEGER Timeout,
    __out_opt PKWAIT_BLOCK WaitBlockArray
    );

NTKERNELAPI
NTSTATUS
KeWaitForSingleObject (
    __in PVOID Object,
    __in KWAIT_REASON WaitReason,
    __in KPROCESSOR_MODE WaitMode,
    __in BOOLEAN Alertable,
    __in_opt PLARGE_INTEGER Timeout
    );

//
// Define interprocess interrupt generic call types.
//

typedef
ULONG_PTR
(*PKIPI_BROADCAST_WORKER)(
    IN ULONG_PTR Argument
    );

ULONG_PTR
KeIpiGenericCall (
    IN PKIPI_BROADCAST_WORKER BroadcastFunction,
    IN ULONG_PTR Context
    );

// end_ntosp end_ntddk end_wdm end_nthal end_ntifs

//
// Define internal kernel functions.
//
// N.B. These definitions are not public and are used elsewhere only under
//      very special circumstances.
//

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntndis begin_ntosp

//
// On X86 the following routines are defined in the HAL and imported by
// all other modules.
//

#if defined(_X86_) && !defined(_NTHAL_)

#define _DECL_HAL_KE_IMPORT  __declspec(dllimport)

#elif defined(_X86_)

#define _DECL_HAL_KE_IMPORT

#else

#define _DECL_HAL_KE_IMPORT NTKERNELAPI

#endif

// end_ntddk end_wdm end_nthal end_ntifs end_ntndis end_ntosp

#if defined(NT_UP)

#define KeTestForWaitersQueuedSpinLock(Number) FALSE

#define KeAcquireQueuedSpinLockRaiseToSynch(Number) \
    KeRaiseIrqlToSynchLevel()

#define KeAcquireQueuedSpinLock(Number) \
    KeRaiseIrqlToDpcLevel()

#define KeReleaseQueuedSpinLock(Number, OldIrql) \
    KeLowerIrql(OldIrql)

#define KeTryToAcquireQueuedSpinLockRaiseToSynch(Number, OldIrql) \
    (*(OldIrql) = KeRaiseIrqlToSynchLevel(), TRUE)

#define KeTryToAcquireQueuedSpinLock(Number, OldIrql) \
    (KeRaiseIrql(DISPATCH_LEVEL, OldIrql), TRUE)

#define KeAcquireQueuedSpinLockAtDpcLevel(LockQueue)

#define KeReleaseQueuedSpinLockFromDpcLevel(LockQueue)

#define KeTryToAcquireQueuedSpinLockAtRaisedIrql(LockQueue) (TRUE)

#else // NT_UP

//
// Queued spin lock functions.
//

FORCEINLINE
LOGICAL
KeTestForWaitersQueuedSpinLock (
    __in KSPIN_LOCK_QUEUE_NUMBER Number
    )

{

    PKSPIN_LOCK Spinlock;
    PKPRCB Prcb;

    Prcb = KeGetCurrentPrcb();
    Spinlock =
        (PKSPIN_LOCK)((ULONG_PTR)Prcb->LockQueue[Number].Lock & ~(LOCK_QUEUE_WAIT | LOCK_QUEUE_OWNER));

    return (*Spinlock != 0);
}

VOID
FASTCALL
KeAcquireQueuedSpinLockAtDpcLevel (
    __inout PKSPIN_LOCK_QUEUE LockQueue
    );

VOID
FASTCALL
KeReleaseQueuedSpinLockFromDpcLevel (
    __inout PKSPIN_LOCK_QUEUE LockQueue
    );

LOGICAL
FASTCALL
KeTryToAcquireQueuedSpinLockAtRaisedIrql (
    __inout PKSPIN_LOCK_QUEUE QueuedLock
    );

// begin_ntifs begin_ntosp

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KeAcquireQueuedSpinLock (
    __in KSPIN_LOCK_QUEUE_NUMBER Number
    );

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KeReleaseQueuedSpinLock (
    __in KSPIN_LOCK_QUEUE_NUMBER Number,
    __in KIRQL OldIrql
    );

_DECL_HAL_KE_IMPORT
LOGICAL
FASTCALL
KeTryToAcquireQueuedSpinLock (
    __in KSPIN_LOCK_QUEUE_NUMBER Number,
    __out PKIRQL OldIrql
    );

// end_ntifs end_ntosp

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KeAcquireQueuedSpinLockRaiseToSynch (
    __in KSPIN_LOCK_QUEUE_NUMBER Number
    );

_DECL_HAL_KE_IMPORT
LOGICAL
FASTCALL
KeTryToAcquireQueuedSpinLockRaiseToSynch (
    __in KSPIN_LOCK_QUEUE_NUMBER Number,
    __out PKIRQL OldIrql
    );

#endif  // NT_UP

#if defined(_AMD64_)

#define KeQueuedSpinLockContext(n)  (&(KiGetLockQueue()[n]))

#else

#define KeQueuedSpinLockContext(n)  (&(KeGetCurrentPrcb()->LockQueue[n]))

#endif

//
// On Uni-processor systems there is no real Dispatcher Database Lock
// so raising to SYNCH won't help get the lock released any sooner.
//

#if defined(NT_UP)

#if defined(_X86_)

#define KiLockDispatcherDatabase(OldIrql)                                    \
    *(OldIrql) = KeRaiseIrqlToDpcLevel()

#define KiTryToLockDispatcherDatabase(OldIrql)                               \
    *(OldIrql) = KeRaiseIrqlToDpcLevel(), TRUE

#else

#define KiLockDispatcherDatabase(OldIrql)                                    \
    *(OldIrql) = KeRaiseIrqlToSynchLevel()

#define KiTryToLockDispatcherDatabase(OldIrql)                               \
    *(OldIrql) = KeRaiseIrqlToSynchLevel(), TRUE

#endif

#else   // NT_UP

#if defined(_AMD64_)

KIRQL
KiAcquireDispatcherLockRaiseToSynch (
    VOID
    );

LOGICAL
KiTryToAcquireDispatcherLockRaiseToSynch (
    __out PKIRQL OldIrql
    );

#define KiLockDispatcherDatabase(OldIrql)                                    \
    *(OldIrql) = KiAcquireDispatcherLockRaiseToSynch()

#define KiTryToLockDispatcherDatabase(OldIrql)                               \
    KiTryToAcquireDispatcherLockRaiseToSynch(OldIrql)

#else

#define KiLockDispatcherDatabase(OldIrql)                                    \
    *(OldIrql) = KeAcquireQueuedSpinLockRaiseToSynch(LockQueueDispatcherLock)

#define KiTryToLockDispatcherDatabase(OldIrql)                               \
    KeTryToAcquireQueuedSpinLockRaiseToSynch(LockQueueDispatcherLock,OldIrql)

#endif

#endif  // NT_UP

#if defined(NT_UP)

#define KiLockDispatcherDatabaseAtSynchLevel()
#define KiUnlockDispatcherDatabaseFromSynchLevel()

#else

#if defined(_AMD64_)

VOID
KiAcquireDispatcherLockAtSynchLevel (
    VOID
    );

VOID
KiReleaseDispatcherLockFromSynchLevel (
    VOID
    );

#define KiLockDispatcherDatabaseAtSynchLevel()                               \
    KiAcquireDispatcherLockAtSynchLevel()

#define KiUnlockDispatcherDatabaseFromSynchLevel()                           \
    KiReleaseDispatcherLockFromSynchLevel()

#else

#define KiLockDispatcherDatabaseAtSynchLevel()                               \
    KeAcquireQueuedSpinLockAtDpcLevel(&KeGetCurrentPrcb()->LockQueue[LockQueueDispatcherLock])

#define KiUnlockDispatcherDatabaseFromSynchLevel()                           \
    KeReleaseQueuedSpinLockFromDpcLevel(&KeGetCurrentPrcb()->LockQueue[LockQueueDispatcherLock])

#endif

#endif

VOID
FASTCALL
KiSetPriorityThread (
    __inout PRKTHREAD Thread,
    __in KPRIORITY Priority
    );

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntndis begin_ntosp
//
// spin lock functions
//

#if defined(_X86_) && (defined(_WDMDDK_) || defined(_NTDDK_) || defined _NTIFS_ || defined(WIN9X_COMPAT_SPINLOCK))

NTKERNELAPI
VOID
NTAPI
KeInitializeSpinLock (
    __out PKSPIN_LOCK SpinLock
    );

#else

FORCEINLINE
VOID
NTAPI
KeInitializeSpinLock (
    __out PKSPIN_LOCK SpinLock
    ) 
{
    *SpinLock = 0;
}

#endif

#if defined(_X86_)

NTKERNELAPI
VOID
FASTCALL
KefAcquireSpinLockAtDpcLevel (
    __inout PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
VOID
FASTCALL
KefReleaseSpinLockFromDpcLevel (
    __inout PKSPIN_LOCK SpinLock
    );

#define KeAcquireSpinLockAtDpcLevel(a) KefAcquireSpinLockAtDpcLevel(a)
#define KeReleaseSpinLockFromDpcLevel(a) KefReleaseSpinLockFromDpcLevel(a)

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KfAcquireSpinLock (
    __inout PKSPIN_LOCK SpinLock
    );

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KfReleaseSpinLock (
    __inout PKSPIN_LOCK SpinLock,
    __in KIRQL NewIrql
    );

// end_wdm end_ntddk

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KeAcquireSpinLockRaiseToSynch (
    __inout PKSPIN_LOCK SpinLock
    );

// begin_wdm begin_ntddk

#define KeAcquireSpinLock(a,b) *(b) = KfAcquireSpinLock(a)
#define KeReleaseSpinLock(a,b) KfReleaseSpinLock(a,b)

NTKERNELAPI
BOOLEAN
FASTCALL
KeTestSpinLock (
    __in PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
BOOLEAN
FASTCALL
KeTryToAcquireSpinLockAtDpcLevel (
    __inout PKSPIN_LOCK SpinLock
    );

#else

//
// These functions are imported for ntddk, ntifs, nthal, ntosp, and wdm.
// They can be inlined for the system on AMD64.
//

#define KeAcquireSpinLock(SpinLock, OldIrql) \
    *(OldIrql) = KeAcquireSpinLockRaiseToDpc(SpinLock)

#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_) || defined(_WDMDDK_)

// end_wdm end_ntddk

NTKERNELAPI
KIRQL
FASTCALL
KeAcquireSpinLockRaiseToSynch (
    __inout PKSPIN_LOCK SpinLock
    );

// begin_wdm begin_ntddk

NTKERNELAPI
VOID
KeAcquireSpinLockAtDpcLevel (
    __inout PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
KIRQL
KeAcquireSpinLockRaiseToDpc (
    __inout PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
VOID
KeReleaseSpinLock (
    __inout PKSPIN_LOCK SpinLock,
    __in KIRQL NewIrql
    );

NTKERNELAPI
VOID
KeReleaseSpinLockFromDpcLevel (
    __inout PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
BOOLEAN
FASTCALL
KeTestSpinLock (
    __in PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
BOOLEAN
FASTCALL
KeTryToAcquireSpinLockAtDpcLevel (
    __inout PKSPIN_LOCK SpinLock
    );

#else

#if defined(_AMD64_)

//
// The system version of these functions are defined in amd64.h for AMD64.
//

#endif

#endif

#endif

// end_wdm end_ntddk end_nthal end_ntifs

NTKERNELAPI
KIRQL
FASTCALL
KeAcquireSpinLockForDpc (
    __inout PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
VOID
FASTCALL
KeReleaseSpinLockForDpc (
    __inout PKSPIN_LOCK SpinLock,
    __in KIRQL OldIrql
    );

// end_ntndis end_ntosp

#if !defined(_AMD64_)

BOOLEAN
KeTryToAcquireSpinLock (
    __inout PKSPIN_LOCK SpinLock,
    __out PKIRQL OldIrql
    );

#endif

//
// Enable interrupts.
//

#if !defined(USER_MODE_CODE)

FORCEINLINE
VOID
KeEnableInterrupts (
    __in BOOLEAN Enable
    )

/*++

Routine Description:

    This function enables interrupts based on the specified enable state.

Arguments:

    Enable - Supplies a boolean value that determines whether interrupts
        are to be enabled.

Return Value:

    None.

--*/

{

    if (Enable != FALSE) {
        _enable();
    }

    return;
}

#endif

//
// Raise and lower IRQL functions.
//

#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || !defined(_APIC_TPR_)

// begin_nthal begin_wdm begin_ntddk begin_ntifs begin_ntosp

#if defined(_X86_)

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KfLowerIrql (
    __in KIRQL NewIrql
    );

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KfRaiseIrql (
    __in KIRQL NewIrql
    );

// end_wdm

_DECL_HAL_KE_IMPORT
KIRQL
KeRaiseIrqlToDpcLevel (
    VOID
    );

// end_ntddk

_DECL_HAL_KE_IMPORT
KIRQL
KeRaiseIrqlToSynchLevel (
    VOID
    );

// begin_wdm begin_ntddk

#define KeLowerIrql(a) KfLowerIrql(a)
#define KeRaiseIrql(a,b) *(b) = KfRaiseIrql(a)

// end_wdm

// begin_wdm

#elif defined(_AMD64_)

//
// These function are defined in amd64.h for the AMD64 platform.
//

#else

#error "no target architecture"

#endif

// end_nthal end_wdm end_ntddk end_ntifs end_ntosp

#else

extern PUCHAR HalpIRQLToTPR;
extern PUCHAR HalpVectorToIRQL;
#define APIC_TPR ((volatile ULONG *)0xFFFE0080)

#define KeGetCurrentIrql _KeGetCurrentIrql
#define KfLowerIrql _KfLowerIrql
#define KfRaiseIrql _KfRaiseIrql

KIRQL
FORCEINLINE
KeGetCurrentIrql (
    VOID
    )
{
    ULONG tprValue;
    KIRQL currentIrql;

    tprValue = *APIC_TPR;
    currentIrql = HalpVectorToIRQL[ tprValue / 16 ];
    return currentIrql;
}

VOID
FORCEINLINE
KfLowerIrql (
    __in KIRQL NewIrql
    )
{
    ULONG tprValue;

    ASSERT( NewIrql <= KeGetCurrentIrql() );

    tprValue = HalpIRQLToTPR[NewIrql];
    KeMemoryBarrier();
    *APIC_TPR = tprValue;
    *APIC_TPR;
    KeMemoryBarrier();
}   

KIRQL
FORCEINLINE
KfRaiseIrql (
    __in KIRQL NewIrql
    )
{
    KIRQL oldIrql;
    ULONG tprValue;

    oldIrql = KeGetCurrentIrql();

    ASSERT( NewIrql >= oldIrql );

    tprValue = HalpIRQLToTPR[NewIrql];
    KeMemoryBarrier();
    *APIC_TPR = tprValue;
    KeMemoryBarrier();
    return oldIrql;
}

KIRQL
FORCEINLINE
KeRaiseIrqlToDpcLevel (
    VOID
    )
{
    return KfRaiseIrql(DISPATCH_LEVEL);
}

KIRQL
FORCEINLINE
KeRaiseIrqlToSynchLevel (
    VOID
    )
{
    return KfRaiseIrql(SYNCH_LEVEL);
}

#define KeLowerIrql(a) KfLowerIrql(a)
#define KeRaiseIrql(a,b) *(b) = KfRaiseIrql(a)

#endif

// begin_ntddk begin_nthal begin_ntifs begin_ntosp
//
// Queued spin lock functions for "in stack" lock handles.
//
// The following three functions RAISE and LOWER IRQL when a queued
// in stack spin lock is acquired or released using these routines.
//

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KeAcquireInStackQueuedSpinLock (
    __inout PKSPIN_LOCK SpinLock,
    __out PKLOCK_QUEUE_HANDLE LockHandle
    );

// end_ntddk end_nthal end_ntifs end_ntosp

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KeAcquireInStackQueuedSpinLockRaiseToSynch (
    __inout PKSPIN_LOCK SpinLock,
    __out PKLOCK_QUEUE_HANDLE LockHandle
    );

// begin_ntddk begin_nthal begin_ntifs begin_ntosp

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KeReleaseInStackQueuedSpinLock (
    __in PKLOCK_QUEUE_HANDLE LockHandle
    );

//
// The following two functions do NOT raise or lower IRQL when a queued
// in stack spin lock is acquired or released using these functions.
//

NTKERNELAPI
VOID
FASTCALL
KeAcquireInStackQueuedSpinLockAtDpcLevel (
    __inout PKSPIN_LOCK SpinLock,
    __out PKLOCK_QUEUE_HANDLE LockHandle
    );

NTKERNELAPI
VOID
FASTCALL
KeReleaseInStackQueuedSpinLockFromDpcLevel (
    __in PKLOCK_QUEUE_HANDLE LockHandle
    );

// end_ntddk end_nthal end_ntifs
//
// The following two functions conditionally raise or lower IRQL when a
// queued in-stack spin lock is acquired or released using these functions.
//

NTKERNELAPI
VOID
FASTCALL
KeAcquireInStackQueuedSpinLockForDpc (
    __inout PKSPIN_LOCK SpinLock,
    __out PKLOCK_QUEUE_HANDLE LockHandle
    );

NTKERNELAPI
VOID
FASTCALL
KeReleaseInStackQueuedSpinLockForDpc (
    __in PKLOCK_QUEUE_HANDLE LockHandle
    );

// end_ntosp

//
// Initialize kernel in phase 1.
//

BOOLEAN
KeInitSystem(
    VOID
    );

VOID
KeNumaInitialize(
    VOID
    );

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp
//
// Miscellaneous kernel functions
//

typedef enum _KBUGCHECK_BUFFER_DUMP_STATE {
    BufferEmpty,
    BufferInserted,
    BufferStarted,
    BufferFinished,
    BufferIncomplete
} KBUGCHECK_BUFFER_DUMP_STATE;

typedef
VOID
(*PKBUGCHECK_CALLBACK_ROUTINE) (
    IN PVOID Buffer,
    IN ULONG Length
    );

typedef struct _KBUGCHECK_CALLBACK_RECORD {
    LIST_ENTRY Entry;
    PKBUGCHECK_CALLBACK_ROUTINE CallbackRoutine;
    PVOID Buffer;
    ULONG Length;
    PUCHAR Component;
    ULONG_PTR Checksum;
    UCHAR State;
} KBUGCHECK_CALLBACK_RECORD, *PKBUGCHECK_CALLBACK_RECORD;

#define KeInitializeCallbackRecord(CallbackRecord) \
    (CallbackRecord)->State = BufferEmpty

NTKERNELAPI
BOOLEAN
KeDeregisterBugCheckCallback (
    __inout PKBUGCHECK_CALLBACK_RECORD CallbackRecord
    );

NTKERNELAPI
BOOLEAN
KeRegisterBugCheckCallback (
    __out PKBUGCHECK_CALLBACK_RECORD CallbackRecord,
    __in PKBUGCHECK_CALLBACK_ROUTINE CallbackRoutine,
    __in PVOID Buffer,
    __in ULONG Length,
    __in PUCHAR Component
    );

typedef enum _KBUGCHECK_CALLBACK_REASON {
    KbCallbackInvalid,
    KbCallbackReserved1,
    KbCallbackSecondaryDumpData,
    KbCallbackDumpIo,
} KBUGCHECK_CALLBACK_REASON;

typedef
VOID
(*PKBUGCHECK_REASON_CALLBACK_ROUTINE) (
    IN KBUGCHECK_CALLBACK_REASON Reason,
    IN struct _KBUGCHECK_REASON_CALLBACK_RECORD* Record,
    IN OUT PVOID ReasonSpecificData,
    IN ULONG ReasonSpecificDataLength
    );

typedef struct _KBUGCHECK_REASON_CALLBACK_RECORD {
    LIST_ENTRY Entry;
    PKBUGCHECK_REASON_CALLBACK_ROUTINE CallbackRoutine;
    PUCHAR Component;
    ULONG_PTR Checksum;
    KBUGCHECK_CALLBACK_REASON Reason;
    UCHAR State;
} KBUGCHECK_REASON_CALLBACK_RECORD, *PKBUGCHECK_REASON_CALLBACK_RECORD;

typedef struct _KBUGCHECK_SECONDARY_DUMP_DATA {
    IN PVOID InBuffer;
    IN ULONG InBufferLength;
    IN ULONG MaximumAllowed;
    OUT GUID Guid;
    OUT PVOID OutBuffer;
    OUT ULONG OutBufferLength;
} KBUGCHECK_SECONDARY_DUMP_DATA, *PKBUGCHECK_SECONDARY_DUMP_DATA;

typedef enum _KBUGCHECK_DUMP_IO_TYPE
{
    KbDumpIoInvalid,
    KbDumpIoHeader,
    KbDumpIoBody,
    KbDumpIoSecondaryData,
    KbDumpIoComplete
} KBUGCHECK_DUMP_IO_TYPE;

typedef struct _KBUGCHECK_DUMP_IO {
    IN ULONG64 Offset;
    IN PVOID Buffer;
    IN ULONG BufferLength;
    IN KBUGCHECK_DUMP_IO_TYPE Type;
} KBUGCHECK_DUMP_IO, *PKBUGCHECK_DUMP_IO;

//
// Equates for exceptions which cause system fatal error
//

#define EXCEPTION_DIVIDED_BY_ZERO       0
#define EXCEPTION_DEBUG                 1
#define EXCEPTION_NMI                   2
#define EXCEPTION_INT3                  3
#define EXCEPTION_BOUND_CHECK           5
#define EXCEPTION_INVALID_OPCODE        6
#define EXCEPTION_NPX_NOT_AVAILABLE     7
#define EXCEPTION_DOUBLE_FAULT          8
#define EXCEPTION_NPX_OVERRUN           9
#define EXCEPTION_INVALID_TSS           0x0A
#define EXCEPTION_SEGMENT_NOT_PRESENT   0x0B
#define EXCEPTION_STACK_FAULT           0x0C
#define EXCEPTION_GP_FAULT              0x0D
#define EXCEPTION_RESERVED_TRAP         0x0F
#define EXCEPTION_NPX_ERROR             0x010
#define EXCEPTION_ALIGNMENT_CHECK       0x011

NTKERNELAPI
BOOLEAN
KeDeregisterBugCheckReasonCallback (
    __inout PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord
    );

NTKERNELAPI
BOOLEAN
KeRegisterBugCheckReasonCallback (
    __out PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord,
    __in PKBUGCHECK_REASON_CALLBACK_ROUTINE CallbackRoutine,
    __in KBUGCHECK_CALLBACK_REASON Reason,
    __in PUCHAR Component
    );

typedef
BOOLEAN
(*PNMI_CALLBACK)(
    __in_opt PVOID Context,
    __in BOOLEAN Handled
    );

NTKERNELAPI
PVOID
KeRegisterNmiCallback (
    __in PNMI_CALLBACK CallbackRoutine,
    __in_opt PVOID Context
    );

NTSTATUS
KeDeregisterNmiCallback (
    __in PVOID Handle
    );

// end_wdm

NTKERNELAPI
DECLSPEC_NORETURN
VOID
NTAPI
KeBugCheck (
    __in ULONG BugCheckCode
    );

// end_ntddk end_nthal end_ntifs end_ntosp

#if defined(_AMD64_)

NTKERNELAPI
DECLSPEC_NORETURN
VOID
NTAPI
KiBugCheck3 (
    __in ULONG BugCheckCode,
    __in ULONG_PTR BugCheckParameter1,
    __in ULONG_PTR BugCheckParameter2,
    __in ULONG_PTR BugCheckParameter3
    );

#else

#define KiBugCheck3(b,p1,p2,p3) KeBugCheckEx(b,p1,p2,p3,0)

#endif

VOID
KeBugCheck2 (
    __in ULONG BugCheckCode,
    __in ULONG_PTR BugCheckParameter1,
    __in ULONG_PTR BugCheckParameter2,
    __in ULONG_PTR BugCheckParameter3,
    __in ULONG_PTR BugCheckParameter4,
    __in_opt PKTRAP_FRAME TrapFrame
    );

BOOLEAN
KeGetBugMessageText (
    __in ULONG MessageId,
    __out_opt PANSI_STRING ReturnedString OPTIONAL
    );

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp

NTKERNELAPI
DECLSPEC_NORETURN
VOID
KeBugCheckEx(
    __in ULONG BugCheckCode,
    __in ULONG_PTR BugCheckParameter1,
    __in ULONG_PTR BugCheckParameter2,
    __in ULONG_PTR BugCheckParameter3,
    __in ULONG_PTR BugCheckParameter4
    );

// end_ntddk end_wdm end_ntifs end_ntosp

NTKERNELAPI
VOID
KeEnterKernelDebugger (
    VOID
    );

// end_nthal

typedef
PCHAR
(*PKE_BUGCHECK_UNICODE_TO_ANSI) (
    IN PUNICODE_STRING UnicodeString,
    OUT PCHAR AnsiBuffer,
    IN ULONG MaxAnsiLength
    );

VOID
KeContextFromKframes (
    __in PKTRAP_FRAME TrapFrame,

#if defined(_X86_)

    __in_opt PKEXCEPTION_FRAME ExceptionFrame,

#else

    __in PKEXCEPTION_FRAME ExceptionFrame,

#endif

    __inout PCONTEXT ContextFrame
    );

VOID
KeContextToKframes (
    __inout PKTRAP_FRAME TrapFrame,

#if defined(_X86_)

    __inout_opt PKEXCEPTION_FRAME ExceptionFrame,

#else

    __inout PKEXCEPTION_FRAME ExceptionFrame,

#endif

    __in PCONTEXT ContextFrame,
    __in ULONG ContextFlags,
    __in KPROCESSOR_MODE PreviousMode
    );

// begin_nthal

NTKERNELAPI
VOID
__cdecl
KeSaveStateForHibernate (
    __out PKPROCESSOR_STATE ProcessorState
    );

// end_nthal

BOOLEAN
FASTCALL
KeInvalidAccessAllowed (
    __in_opt PVOID TrapInformation
    );

//
//  GDI TEB Batch Flush routine
//

typedef
VOID
(*PGDI_BATCHFLUSH_ROUTINE) (
    VOID
    );

//
// Find first set left in affinity mask.
//

#if defined(_WIN64)

#if (defined(_AMD64_) && !defined(_X86AMD64_))

#define KeFindFirstSetLeftAffinity(Set, Member) BitScanReverse64(Member, Set)

#else

#define KeFindFirstSetLeftAffinity(Set, Member) {                      \
    ULONG _Mask_;                                                      \
    ULONG _Offset_ = 32;                                               \
    if ((_Mask_ = (ULONG)(Set >> 32)) == 0) {                          \
        _Offset_ = 0;                                                  \
        _Mask_ = (ULONG)Set;                                           \
    }                                                                  \
    KeFindFirstSetLeftMember(_Mask_, Member);                          \
    *(Member) += _Offset_;                                             \
}

#endif

#else

#define KeFindFirstSetLeftAffinity(Set, Member)                        \
    KeFindFirstSetLeftMember(Set, Member)

#endif // defined(_WIN64)

//
// Find first set left in 32-bit set.
//
// KiFindFirstSetLeft - This is an array tha this used to lookup the left
//      most bit in a byte.
//

extern DECLSPEC_CACHEALIGN DECLSPEC_SELECTANY const CCHAR KiFindFirstSetLeft[256] = {
        0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};

#if defined(_WIN64)

#if defined(_AMD64_) && !defined(_X86AMD64_)

#define KeFindFirstSetLeftMember(Set, Member) BitScanReverse(Member, Set)

#else

#define KeFindFirstSetLeftMember(Set, Member) {                        \
    ULONG _Mask;                                                       \
    ULONG _Offset = 16;                                                \
    if ((_Mask = Set >> 16) == 0) {                                    \
        _Offset = 0;                                                   \
        _Mask = Set;                                                   \
    }                                                                  \
    if (_Mask >> 8) {                                                  \
        _Offset += 8;                                                  \
    }                                                                  \
    *(Member) = KiFindFirstSetLeft[Set >> _Offset] + _Offset;          \
}

#endif

#else

FORCEINLINE
ULONG
KiFindFirstSetLeftMemberInt (
    __in ULONG Set
    )
{
    __asm {
        bsr eax, Set
    }
}

FORCEINLINE
void
KeFindFirstSetLeftMember (
    __in ULONG Set,
    __out PULONG Member
    )
{
    *Member = KiFindFirstSetLeftMemberInt (Set);
}

#endif

ULONG
KeFindNextRightSetAffinity (
    __in ULONG Number,
    __in KAFFINITY Set
    );

//
// Find first set right in 32-bit set.
//
// KiFindFirstSetRight - This is an array that this used to lookup the right
//      most bit in a byte.
//

extern DECLSPEC_CACHEALIGN DECLSPEC_SELECTANY const CCHAR KiFindFirstSetRight[256] = {
        0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};

#if defined(_X86_)

FORCEINLINE
ULONG
KeFindFirstSetRightMember (
    __in ULONG Set
    )
{
    __asm {
        bsf eax, Set
    }
}

#elif defined(_AMD64_) && !defined(_X86AMD64_)

FORCEINLINE
ULONG
KeFindFirstSetRightMember (
    __in ULONG Set
    )

{

    ULONG Member;

    BitScanForward(&Member, Set);
    return Member;
}

#else

#define KeFindFirstSetRightMember(Set) \
    ((Set & 0xFF) ? KiFindFirstSetRight[Set & 0xFF] : \
    ((Set & 0xFF00) ? KiFindFirstSetRight[(Set >> 8) & 0xFF] + 8 : \
    ((Set & 0xFF0000) ? KiFindFirstSetRight[(Set >> 16) & 0xFF] + 16 : \
                           KiFindFirstSetRight[Set >> 24] + 24 )))

#endif

//
// TB Flush routines
//

extern volatile LONG KiTbFlushTimeStamp;

VOID
KxFlushEntireTb (
    VOID
    );

FORCEINLINE
VOID
KeFlushEntireTb (
    __in BOOLEAN Invalid,
    __in BOOLEAN AllProcessors
    )

{
    UNREFERENCED_PARAMETER(Invalid);
    UNREFERENCED_PARAMETER(AllProcessors);

    KxFlushEntireTb();
    return;
}

#if (defined(_M_IX86) || defined(_M_AMD64)) && defined(NT_UP)

FORCEINLINE
VOID
KeFlushProcessTb (
    VOID
    )

{

    KiFlushProcessTb();
    return;
}

FORCEINLINE
VOID
FASTCALL
KeFlushSingleTb (
    __in PVOID Virtual,
    __in BOOLEAN AllProcesors
    )

{

    UNREFERENCED_PARAMETER (AllProcesors);

#if _MSC_FULL_VER >= 13008806

#if defined(_M_AMD64)

    InvalidatePage(Virtual);

#else

    __asm {
        mov eax, Virtual
        invlpg [eax]
    }

#endif

#else

    KiFlushSingleTb(Virtual);

#endif

    return;
}

#define KeFlushMultipleTb(Number, Virtual, AllProcessors)                    \
{                                                                            \
    ULONG _Index_;                                                           \
    PVOID _VA_;                                                              \
                                                                             \
    for (_Index_ = 0; _Index_ < (Number); _Index_ += 1) {                    \
        _VA_ = (Virtual)[_Index_];                                           \
        KiFlushSingleTb(_VA_);                                               \
    }                                                                        \
}

#else

#if defined(_AMD64_) || defined(_X86_)

VOID
KeFlushProcessTb (
    VOID
    );

#else

#define KeFlushProcessTb() KeFlushEntireTb(FALSE, FALSE)

#endif

VOID
KeFlushMultipleTb (
    IN ULONG Number,
    IN PVOID *Virtual,
    IN BOOLEAN AllProcesors
    );

VOID
FASTCALL
KeFlushSingleTb (
    IN PVOID Virtual,
    IN BOOLEAN AllProcesors
    );

#endif

// begin_nthal

#if !defined(_AMD64_)

BOOLEAN
KiIpiServiceRoutine (
    IN struct _KTRAP_FRAME *TrapFrame,
    IN struct _KEXCEPTION_FRAME *ExceptionFrame
    );

#endif

// end_nthal

BOOLEAN
KeFreezeExecution (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    );

KCONTINUE_STATUS
KeSwitchFrozenProcessor (
    IN ULONG ProcessorNumber
    );

VOID
KeGetNonVolatileContextPointers (
    IN PKNONVOLATILE_CONTEXT_POINTERS NonVolatileContext
    );

// begin_ntddk

#if defined(_AMD64_) || defined(_X86_)

NTKERNELAPI
BOOLEAN
KeInvalidateAllCaches (
    VOID
    );

#endif

// end_ntddk

#define DMA_READ_DCACHE_INVALIDATE 0x1              // nthal
#define DMA_READ_ICACHE_INVALIDATE 0x2              // nthal
#define DMA_WRITE_DCACHE_SNOOP 0x4                  // nthal
                                                    // nthal
NTKERNELAPI                                         // nthal
VOID                                                // nthal
KeSetDmaIoCoherency (                               // nthal
    IN ULONG Attributes                             // nthal
    );                                              // nthal
                                                    // nthal

#if defined(_AMD64_) || defined(_X86_)

NTKERNELAPI                                         // nthal
VOID                                                // nthal
KeSetProfileIrql (                                  // nthal
    IN KIRQL ProfileIrql                            // nthal
    );                                              // nthal
                                                    // nthal
#endif

//
// Interlocked read TB flush entire timestamp.
//

FORCEINLINE
ULONG
KeReadTbFlushTimeStamp (
    VOID
    )

{

    //
    // While the TB flush time stamp counter is being updated the low order
    // bit of the time stamp value is set (MP only). Otherwise, the bit is
    // clear.
    //
    // N.B. Memory ordering is required so modifications to page tables are
    //      completed before reading the timestamp.
    //

    KeMemoryBarrier();
    return KiTbFlushTimeStamp;
}

FORCEINLINE
VOID
KeLoopTbFlushTimeStampUnlocked (
    VOID
    )

{

    //
    // Wait until the time stamp counter is unlocked.
    //

    KeMemoryBarrier();

#if defined(_AMD64_)

    while (BitTest((PLONG)&KiTbFlushTimeStamp, 0)) {

#else

    while ((KiTbFlushTimeStamp & 1) == 1) {

#endif

        KeYieldProcessor();
    }

    return;
}

VOID
KeSetSystemTime (
    IN PLARGE_INTEGER NewTime,
    OUT PLARGE_INTEGER OldTime,
    IN BOOLEAN AdjustInterruptTime,
    IN PLARGE_INTEGER HalTimeToSet OPTIONAL
    );

#define SYSTEM_SERVICE_INDEX 0

// begin_ntosp

#define WIN32K_SERVICE_INDEX 1

// end_ntosp

// begin_ntosp

NTKERNELAPI
BOOLEAN
KeAddSystemServiceTable(
    IN PULONG_PTR Base,
    IN PULONG Count OPTIONAL,
    IN ULONG Limit,
    IN PUCHAR Number,
    IN ULONG Index
    );

NTKERNELAPI
BOOLEAN
KeRemoveSystemServiceTable(
    IN ULONG Index
    );

// end_ntosp

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp

#if !defined(_AMD64_)

NTKERNELAPI
ULONGLONG
KeQueryInterruptTime (
    VOID
    );

NTKERNELAPI
VOID
KeQuerySystemTime (
    OUT PLARGE_INTEGER CurrentTime
    );

#endif

NTKERNELAPI
ULONG
KeQueryTimeIncrement (
    VOID
    );

NTKERNELAPI
ULONG
KeGetRecommendedSharedDataAlignment (
    VOID
    );

// end_wdm

NTKERNELAPI
KAFFINITY
KeQueryActiveProcessors (
    VOID
    );

// end_ntddk end_ntifs end_ntosp

#if defined(_AMD64_)

NTKERNELAPI
KAFFINITY
KeQueryMultiThreadProcessorSet (
    ULONG Number
    );

#endif

// end_nthal

NTSTATUS
KeQueryLogicalProcessorInformation(
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength
    );

PKPRCB
KeGetPrcb(
    IN ULONG ProcessorNumber
    );

BOOLEAN
KeAdjustInterruptTime (
    IN LONGLONG TimeDelta
    );

// begin_nthal

NTKERNELAPI
VOID
KeSetTimeIncrement (
    IN ULONG MaximumIncrement,
    IN ULONG MimimumIncrement
    );

// end_nthal

VOID
KeThawExecution (
    IN BOOLEAN Enable
    );

// begin_nthal begin_ntosp

//
// Define the firmware routine types
//

typedef enum _FIRMWARE_REENTRY {
    HalHaltRoutine,
    HalPowerDownRoutine,
    HalRestartRoutine,
    HalRebootRoutine,
    HalInteractiveModeRoutine,
    HalMaximumRoutine
} FIRMWARE_REENTRY, *PFIRMWARE_REENTRY;

// end_nthal end_ntosp

VOID
KeStartAllProcessors (
    VOID
    );

//
// Balance set manager thread startup function.
//

VOID
KeBalanceSetManager (
    IN PVOID Context
    );

VOID
KeSwapProcessOrStack (
    IN PVOID Context
    );

//
// User mode callback.
//

// begin_ntosp

NTKERNELAPI
NTSTATUS
KeUserModeCallback (
    IN ULONG ApiNumber,
    IN PVOID InputBuffer,
    IN ULONG InputLength,
    OUT PVOID *OutputBuffer,
    OUT PULONG OutputLength
    );

// end_ntosp


PVOID
KeSwitchKernelStack (
    IN PVOID StackBase,
    IN PVOID StackLimit
    );

NTSTATUS
KeRaiseUserException(
    IN NTSTATUS ExceptionCode
    );

// begin_nthal
//
// Find ARC configuration information function.
//

NTKERNELAPI
PCONFIGURATION_COMPONENT_DATA
KeFindConfigurationEntry (
    IN PCONFIGURATION_COMPONENT_DATA Child,
    IN CONFIGURATION_CLASS Class,
    IN CONFIGURATION_TYPE Type,
    IN PULONG Key OPTIONAL
    );

NTKERNELAPI
PCONFIGURATION_COMPONENT_DATA
KeFindConfigurationNextEntry (
    IN PCONFIGURATION_COMPONENT_DATA Child,
    IN CONFIGURATION_CLASS Class,
    IN CONFIGURATION_TYPE Type,
    IN PULONG Key OPTIONAL,
    IN PCONFIGURATION_COMPONENT_DATA *Resume
    );

// end_nthal

//
// External references to public kernel data structures
//

extern KAFFINITY KeActiveProcessors;
extern LARGE_INTEGER KeBootTime;
extern ULONGLONG KeBootTimeBias;
extern ULONG KeThreadDpcEnable;
extern ULONG KeErrorMask;
extern ULONGLONG KeInterruptTimeBias;
extern BOOLEAN KeBugCheckActive;
extern LIST_ENTRY KeBugCheckCallbackListHead;
extern LIST_ENTRY KeBugCheckReasonCallbackListHead;
extern KSPIN_LOCK KeBugCheckCallbackLock;
extern PGDI_BATCHFLUSH_ROUTINE KeGdiFlushUserBatch;
extern PLOADER_PARAMETER_BLOCK KeLoaderBlock;       // ntosp
extern ULONG KeMaximumIncrement;
extern ULONG KeMinimumIncrement;
extern NTSYSAPI CCHAR KeNumberProcessors;           // nthal ntosp
extern UCHAR KeNumberNodes;
extern USHORT KeProcessorArchitecture;
extern USHORT KeProcessorLevel;
extern USHORT KeProcessorRevision;
extern ULONG KeFeatureBits;
extern ALIGNED_SPINLOCK KiDispatcherLock;
extern ULONG KiDPCTimeout;
extern PKPRCB KiProcessorBlock[];
extern ULONG KiSpinlockTimeout;
extern ULONG KiStackProtectTime;
extern KTHREAD_SWITCH_COUNTERS KeThreadSwitchCounters;
extern ULONG KeTimerCheckFlags;
extern ULONG KeLargestCacheLine;

#if !defined(NT_UP)

extern ULONG KeNumprocSpecified;
extern UCHAR KeProcessNodeSeed;

#endif

extern PULONG KeServiceCountTable;
extern KSERVICE_TABLE_DESCRIPTOR KeServiceDescriptorTable[NUMBER_SERVICE_TABLES];
extern KSERVICE_TABLE_DESCRIPTOR KeServiceDescriptorTableShadow[NUMBER_SERVICE_TABLES];

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp

#if defined(_X86_)

extern volatile KSYSTEM_TIME KeTickCount;

#endif

// end_ntddk end_wdm end_nthal end_ntifs end_ntosp

// begin_nthal

extern PVOID KeUserApcDispatcher;
extern PVOID KeUserCallbackDispatcher;
extern PVOID KeUserExceptionDispatcher;

extern PVOID KeUserPopEntrySListEnd;
extern PVOID KeUserPopEntrySListFault;
extern PVOID KeUserPopEntrySListResume;

#if defined(_WIN64)

extern PVOID KeUserPopEntrySListEndWow64;
extern PVOID KeUserPopEntrySListFaultWow64;
extern PVOID KeUserPopEntrySListResumeWow64;

#endif  // _WIN64

extern PVOID KeRaiseUserExceptionDispatcher;
extern ULONG KeTimeAdjustment;
extern ULONG KeTimeIncrement;
extern BOOLEAN KeTimeSynchronization;

// end_nthal

#if defined(_X86_)

VOID
KeOptimizeProcessorControlState (
    VOID
    );

#endif

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp

typedef enum _MEMORY_CACHING_TYPE_ORIG {
    MmFrameBufferCached = 2
} MEMORY_CACHING_TYPE_ORIG;

typedef enum _MEMORY_CACHING_TYPE {
    MmNonCached = FALSE,
    MmCached = TRUE,
    MmWriteCombined = MmFrameBufferCached,
    MmHardwareCoherentCached,
    MmNonCachedUnordered,       // IA64
    MmUSWCCached,
    MmMaximumCacheType
} MEMORY_CACHING_TYPE;

// end_ntddk end_wdm end_nthal end_ntifs end_ntosp

//
// Routine for setting memory type for physical address ranges.
//

#if defined(_X86_)

NTSTATUS
KeSetPhysicalCacheTypeRange (
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
    );

#endif

//
// Routines for zeroing a physical page.
//
// These are defined as calls through a function pointer which is set to
// point at the optimal routine for this processor implementation.
//

#if defined(_X86_)

typedef
VOID
(FASTCALL *KE_ZERO_PAGE_ROUTINE)(
    IN PVOID PageBase,
    IN SIZE_T NumberOfBytes
    );

extern KE_ZERO_PAGE_ROUTINE KeZeroPages;
extern KE_ZERO_PAGE_ROUTINE KeZeroPagesFromIdleThread;

#define KeZeroSinglePage(v) KeZeroPages((v), PAGE_SIZE)

#else

#define KeZeroPagesFromIdleThread KeZeroPages

VOID
KeZeroPages (
    IN PVOID PageBase,
    IN SIZE_T NumberOfBytes
    );

VOID
KeZeroSinglePage (
    IN PVOID PageBase
    );

#endif

//
// Routine for copying a page.
//

#if defined(_AMD64_)

VOID
KeCopyPage (
    IN PVOID Destination,
    IN PVOID Source
    );

#else

#define KeCopyPage(d,s) RtlCopyMemory((d),(s),PAGE_SIZE)

#endif

//
// Verifier functions
//

NTSTATUS
KevUtilAddressToFileHeader (
    IN  PVOID Address,
    OUT UINT_PTR *OffsetIntoImage,
    OUT PUNICODE_STRING *DriverName,
    OUT BOOLEAN *InVerifierList
    );

//
// Define guarded mutex structure.
//

// begin_ntifs begin_ntddk begin_wdm begin_nthal begin_ntosp

#define GM_LOCK_BIT          0x1 // Actual lock bit, 0 = Unlocked, 1 = Locked
#define GM_LOCK_BIT_V        0x0 // Lock bit as a bit number
#define GM_LOCK_WAITER_WOKEN 0x2 // A single waiter has been woken to acquire this lock
#define GM_LOCK_WAITER_INC   0x4 // Increment value to change the waiters count

typedef struct _KGUARDED_MUTEX {
    LONG Count;
    PKTHREAD Owner;
    ULONG Contention;
    KGATE Gate;
    union {
        struct {
            SHORT KernelApcDisable;
            SHORT SpecialApcDisable;
        };

        ULONG CombinedApcDisable;
    };

} KGUARDED_MUTEX, *PKGUARDED_MUTEX;

// end_wdm

#if ((defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)) && !defined(_NTSYSTEM_DRIVER_) || defined(_NTOSP_))

// begin_wdm

NTKERNELAPI
BOOLEAN
KeAreAllApcsDisabled (
    VOID
    );

NTKERNELAPI
VOID
FASTCALL
KeInitializeGuardedMutex (
    IN PKGUARDED_MUTEX Mutex
    );

NTKERNELAPI
VOID
FASTCALL
KeAcquireGuardedMutex (
    IN PKGUARDED_MUTEX Mutex
    );

NTKERNELAPI
VOID
FASTCALL
KeReleaseGuardedMutex (
    IN PKGUARDED_MUTEX Mutex
    );

NTKERNELAPI
BOOLEAN
FASTCALL
KeTryToAcquireGuardedMutex (
    IN PKGUARDED_MUTEX Mutex
    );

NTKERNELAPI
VOID
FASTCALL
KeAcquireGuardedMutexUnsafe (
    IN PKGUARDED_MUTEX FastMutex
    );

NTKERNELAPI
VOID
FASTCALL
KeReleaseGuardedMutexUnsafe (
    IN PKGUARDED_MUTEX FastMutex
    );

// end_wdm

#endif

//
// end_ntifs end_ntddk end_nthal end_ntosp
//

ULARGE_INTEGER
KeComputeReciprocal (
    IN LONG Divisor,
    OUT PCCHAR Shift
    );

ULONG
KeComputeReciprocal32 (
    IN LONG Divisor,
    OUT PCCHAR Shift
    );

#endif // _KE_
