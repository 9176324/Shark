/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    kernldat.c

Abstract:

    This module contains the declaration and allocation of kernel data
    structures.

--*/

#include "ki.h"

//
// KiTimerTableListHead - This is an array of tiemr table entries that anchor
//      the individual timer lists.
//

DECLSPEC_CACHEALIGN KTIMER_TABLE_ENTRY KiTimerTableListHead[TIMER_TABLE_SIZE];

//
// KiQueuedLockTableSize - This is the size of the PRCB based numbered queued
//      lock table used by the kernel debugger extensions.
//

#if defined(_WIN64)

#pragma comment(linker, "/include:KiQueuedLockTableSize")

#endif

ULONG KiQueuedLockTableSize = LockQueueMaximumLock;

//
// KiTimerSystemSharedData - This is the address of the kernel view of the
//      user shared data.
//

#if defined(_WIN64)

#pragma comment(linker, "/include:KiSystemSharedData")

#endif

ULONG_PTR KiSystemSharedData = KI_USER_SHARED_DATA;

//
// KiTimerTableSize - This is the size of the timer table and is used by the
//      kernel debugger extensions.
//

#if defined(_WIN64)

#pragma comment(linker, "/include:KiTimerTableSize")

#endif

ULONG KiTimerTableSize = TIMER_TABLE_SIZE;

//
//
// Public kernel data declaration and allocation.
//
// KeActiveProcessors - This is the set of processors that active in the
//      system.
//

KAFFINITY KeActiveProcessors = 0;

//
// KeBootTime - This is the absolute time when the system was booted.
//

LARGE_INTEGER KeBootTime;

//
// KeBootTimeBias - The time for which KeBootTime has ever been biased
//

ULONGLONG KeBootTimeBias;

//
// KeInterruptTimeBias - The time for which InterrupTime has ever been biased
//

ULONGLONG KeInterruptTimeBias;

//
// KeBugCheckCallbackListHead - This is the list head for registered
//      bugcheck callback routines.
//

LIST_ENTRY KeBugCheckCallbackListHead;
LIST_ENTRY KeBugCheckReasonCallbackListHead;

//
// KeBugCheckCallbackLock - This is the spin lock that guards the bugcheck
//      callback list.
//

KSPIN_LOCK KeBugCheckCallbackLock;

//
// KeGdiFlushUserBatch - This is the address of the GDI user batch flush
//      routine which is initialized when the win32k subsystem is loaded.
//

PGDI_BATCHFLUSH_ROUTINE KeGdiFlushUserBatch;

//
// KeLoaderBlock - This is a pointer to the loader parameter block which is
//      constructed by the OS Loader.
//

PLOADER_PARAMETER_BLOCK KeLoaderBlock = NULL;

//
// KeMinimumIncrement - This is the minimum time between clock interrupts
//      in 100ns units that is supported by the host HAL.
//

ULONG KeMinimumIncrement;

//
// KeThreadDpcEnable - This is the system wide enable for threaded DPCs that
//      is read from the registry.
//

ULONG KeThreadDpcEnable = FALSE; // TRUE;

//
// KeNumberProcessors - This is the number of processors in the configuration.
//      If is used by the ready thread and spin lock code to determine if a
//      faster algorithm can be used for the case of a single processor system.
//      The value of this variable is set when processors are initialized.
//

CCHAR KeNumberProcessors = 0;

//
// KeNumprocSpecified - This is the number of processors specified by the
//      /NUMPROC=x osloader option. If this value is set with the number
//      of processors option, then it specifies the total number of logical
//      processors that can be started.
//    

#if !defined(NT_UP)

ULONG KeNumprocSpecified = 0;

#endif

//
// KeProcessorArchitecture - Architecture of all processors present in system.
//      See PROCESSOR_ARCHITECTURE_ defines in ntexapi.h
//

USHORT KeProcessorArchitecture = PROCESSOR_ARCHITECTURE_UNKNOWN;

//
// KeProcessorLevel - Architectural specific processor level of all processors
//      present in system.
//

USHORT KeProcessorLevel = 0;

//
// KeProcessorRevision - Architectural specific processor revision number that is
//      the least common denominator of all processors present in system.
//

USHORT KeProcessorRevision = 0;

//
// KeFeatureBits - Architectural specific processor features present
// on all processors.
//

ULONG KeFeatureBits = 0;

//
// KeServiceDescriptorTable - This is a table of descriptors for system
//      service providers. Each entry in the table describes the base
//      address of the dispatch table and the number of services provided.
//

DECLSPEC_CACHEALIGN KSERVICE_TABLE_DESCRIPTOR KeServiceDescriptorTable[NUMBER_SERVICE_TABLES];
DECLSPEC_CACHEALIGN KSERVICE_TABLE_DESCRIPTOR KeServiceDescriptorTableShadow[NUMBER_SERVICE_TABLES];

//
// KeThreadSwitchCounters - These counters record the number of times a
//      thread can be scheduled on the current processor, any processor,
//      or the last processor it ran on.
//

KTHREAD_SWITCH_COUNTERS KeThreadSwitchCounters;

//
// KeTimeIncrement - This is the nominal number of 100ns units that are to
//      be added to the system time at each interval timer interupt. This
//      value is set by the HAL and is used to compute the due time for
//      timer table entries.
//

ULONG KeTimeIncrement;

//
// KeTimeSynchronization - This variable controls whether time synchronization
//      is performed using the realtime clock (TRUE) or whether it is under the
//      control of a service (FALSE).
//

BOOLEAN KeTimeSynchronization = TRUE;

//
// KeUserApcDispatcher - This is the address of the user mode APC dispatch
//      code. This address is looked up in NTDLL.DLL during initialization
//      of the system.
//

PVOID KeUserApcDispatcher;

//
// KeUserCallbackDispatcher - This is the address of the user mode callback
//      dispatch code. This address is looked up in NTDLL.DLL during
//      initialization of the system.
//

PVOID KeUserCallbackDispatcher;

//
// KeUserExceptionDispatcher - This is the address of the user mode exception
//      dispatch code. This address is looked up in NTDLL.DLL during system
//      initialization.
//

PVOID KeUserExceptionDispatcher;

//
// KeUserPopEntrySListEnd - This is the ending address of the user mode SLIST
//      code.
//

PVOID KeUserPopEntrySListEnd;

//
// KeUserPopEntrySListFault - This is the address of the user mode code that
//      may generate a "benign" pagefault.
//

PVOID KeUserPopEntrySListFault;

//
// KeUserPopEntrySListResume - This is the address of the user mode code that
//      execution will be restarted at upon a fault at the above address.
//

PVOID KeUserPopEntrySListResume;

//
// Same as above, for 32-bit ntdll.
//

#if defined(_WIN64)

PVOID KeUserPopEntrySListEndWow64;
PVOID KeUserPopEntrySListFaultWow64;
PVOID KeUserPopEntrySListResumeWow64;

#endif

//
// KeRaiseUserExceptionDispatcher - This is the address of the raise user
//      mode exception dispatch code. This address is looked up in NTDLL.DLL
//      during system initialization.
//

PVOID KeRaiseUserExceptionDispatcher;

//
// KeLargestCacheLine - This variable contains the size in bytes of
//      the largest cache line discovered during system initialization.
//      It is used to provide the recommend alignment (and padding)
//      for data that may be used heavily by more than one processor.
//      The initial value was chosen as a reasonable value to use on
//      systems where the discovery process doesn't find a value.
//

ULONG KeLargestCacheLine = 64;

//
// Private kernel data declaration and allocation.
//
// KiBugCodeMessages - Address of where the BugCode messages can be found.
//

PMESSAGE_RESOURCE_DATA KiBugCodeMessages = NULL;

//
// KiDmaIoCoherency - This determines whether the host platform supports
//      coherent DMA I/O.
//

ULONG KiDmaIoCoherency;

//
// KiDPCTimeout - This is the DPC time out time in ticks on checked builds.
//

ULONG KiDPCTimeout = 110;

//
// KiDebugSwitchRoutine - This is the address of the kernel debugger
//      processor switch routine.  This is used on an MP system to
//      switch host processors while debugging.
//

PKDEBUG_SWITCH_ROUTINE KiDebugSwitchRoutine;

//
// KiGenericCallDpcMutex - This is the fast mutex that guards generic DPC calls.
//

FAST_MUTEX KiGenericCallDpcMutex;

//
// KiFreezeFlag - For debug builds only.  Flags to track and signal non-
//      normal freezelock conditions.
//

ULONG KiFreezeFlag;

//
// KiInitialProcess - This is the initial process that is created when the
//      system is booted.
//

DECLSPEC_CACHEALIGN EPROCESS KiInitialProcess;

//
// KiInitialThread - This is the initial thread that is created when the
//      system is booted.
//

DECLSPEC_CACHEALIGN ETHREAD KiInitialThread;

//
// KiSpinlockTimeout - This is the spin lock time out time in ticks on checked
//      builds.
//

ULONG KiSpinlockTimeout = 55;

//
// KiSuspenState - Flag to track suspend/resume state of processors.
//

volatile ULONG KiSuspendState;

//
// KiProcessorBlock - This is an array of pointers to processor control blocks.
//      The elements of the array are indexed by processor number. Each element
//      is a pointer to the processor control block for one of the processors
//      in the configuration. This array is used by various sections of code
//      that need to effect the execution of another processor.
//

DECLSPEC_CACHEALIGN PKPRCB KiProcessorBlock[MAXIMUM_PROCESSORS];

//
// KeNumberNodes - This is the number of ccNUMA nodes in the system. Logically
// an SMP system is the same as a single node ccNUMA system.
//

UCHAR KeNumberNodes = 1;

//
// KeNodeBlock - This is an array of pointers to KNODE structures. A KNODE
// structure describes the resources of a NODE in a ccNUMA system.
//

KNODE KiNode0;

UCHAR KeProcessNodeSeed;

#if defined(KE_MULTINODE)

DECLSPEC_CACHEALIGN PKNODE KeNodeBlock[MAXIMUM_CCNUMA_NODES];

#else

PKNODE KeNodeBlock[1] = {&KiNode0};

#endif

//
// KiSwapEvent - This is the event that is used to wake up the balance set
//      thread to inswap processes, outswap processes, and to inswap kernel
//      stacks.
//

KEVENT KiSwapEvent;

//
// KiSwappingThread - This is a pointer to the swap thread object.
//

PKTHREAD KiSwappingThread;

//
// KiProcessListHead - This is the list of processes that have active threads.
//

LIST_ENTRY KiProcessListHead;

//
// KiProcessInSwapListHead - This is the list of processes that are waiting
//      to be inswapped.
//

SINGLE_LIST_ENTRY KiProcessInSwapListHead;

//
// KiProcessOutSwapListHead - This is the list of processes that are waiting
//      to be outswapped.
//

SINGLE_LIST_ENTRY KiProcessOutSwapListHead;

//
// KiStackInSwapListHead - This is the list of threads that are waiting
//      to get their stack inswapped before they can run. Threads are
//      inserted in this list in ready thread and removed by the balance
//      set thread.
//

SINGLE_LIST_ENTRY KiStackInSwapListHead;

//
// KiProfileSourceListHead - The list of profile sources that are currently
//      active.
//

LIST_ENTRY KiProfileSourceListHead;

//
// KiProfileAlignmentFixup - Indicates whether alignment fixup profiling
//      is active.
//

BOOLEAN KiProfileAlignmentFixup;

//
// KiProfileAlignmentFixupInterval - Indicates the current alignment fixup
//      profiling interval.
//

ULONG KiProfileAlignmentFixupInterval;

//
// KiProfileAlignmentFixupCount - Indicates the current alignment fixup
//      count.
//

ULONG KiProfileAlignmentFixupCount;

//
// KiProfileInterval - The profile interval in 100ns units.
//

ULONG KiProfileInterval = DEFAULT_PROFILE_INTERVAL;

//
// KiProfileListHead - This is the list head for the profile list.
//

LIST_ENTRY KiProfileListHead;

//
// KiTimerExpireDpc - This is the Deferred Procedure Call (DPC) object that
//      is used to process the timer queue when a timer has expired.
//

KDPC KiTimerExpireDpc;

//
// KiEnableTimerWatchdog at one point controlled a HAL clock interrupt
// watchdog that is now obsolete.  This symbol was present in Server 2003,
// meaning that it still has to be exported from the Server 2003 service pack
// kernels so that custom HALs built for Server 2003 RTM won't fail with an
// unresolved import after applying the service pack.  We only need to do this
// for x86 since the other architectures don't allow custom HALs.
//

#if defined(_X86_)

ULONG KiEnableTimerWatchdog = 0;

#endif

#if defined(_APIC_TPR_)

PUCHAR HalpIRQLToTPR;
PUCHAR HalpVectorToIRQL;

#endif

//
// Flag to indicate that frozen processors should resume to a benign looping
// state in preparation for a reboot.
//

#if defined(_AMD64_) && !defined(NT_UP)

BOOLEAN KiResumeForReboot = FALSE;

#endif

