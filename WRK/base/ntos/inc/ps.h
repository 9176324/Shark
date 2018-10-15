/*++ BUILD Version: 0009    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ps.h

Abstract:

    This module contains the process structure public data structures and
    procedure prototypes to be used within the NT system.

--*/

#ifndef _PS_
#define _PS_


//
// Process Object
//

//
// Process object body.  A pointer to this structure is returned when a handle
// to a process object is referenced.  This structure contains a process control
// block (PCB) which is the kernel's representation of a process.
//

#define MEMORY_PRIORITY_BACKGROUND 0
#define MEMORY_PRIORITY_WASFOREGROUND 1
#define MEMORY_PRIORITY_FOREGROUND 2

typedef struct _MMSUPPORT_FLAGS {

    //
    // The next 8 bits are protected by the expansion lock.
    //

    UCHAR SessionSpace : 1;
    UCHAR BeingTrimmed : 1;
    UCHAR SessionLeader : 1;
    UCHAR TrimHard : 1;
    UCHAR MaximumWorkingSetHard : 1;
    UCHAR ForceTrim : 1;
    UCHAR MinimumWorkingSetHard : 1;
    UCHAR Available0 : 1;

    UCHAR MemoryPriority : 8;

    //
    // The next 16 bits are protected by the working set mutex.
    //

    USHORT GrowWsleHash : 1;
    USHORT AcquiredUnsafe : 1;
    USHORT Available : 14;
} MMSUPPORT_FLAGS;

typedef ULONG WSLE_NUMBER, *PWSLE_NUMBER;

typedef struct _MMSUPPORT {
    LIST_ENTRY WorkingSetExpansionLinks;
    LARGE_INTEGER LastTrimTime;

    MMSUPPORT_FLAGS Flags;
    ULONG PageFaultCount;
    WSLE_NUMBER PeakWorkingSetSize;
    WSLE_NUMBER GrowthSinceLastEstimate;

    WSLE_NUMBER MinimumWorkingSetSize;
    WSLE_NUMBER MaximumWorkingSetSize;
    struct _MMWSL *VmWorkingSetList;
    WSLE_NUMBER Claim;

    WSLE_NUMBER NextEstimationSlot;
    WSLE_NUMBER NextAgingSlot;
    WSLE_NUMBER EstimatedAvailable;
    WSLE_NUMBER WorkingSetSize;

    EX_PUSH_LOCK WorkingSetMutex;

} MMSUPPORT, *PMMSUPPORT;

typedef struct _MMADDRESS_NODE {
    union {
        LONG_PTR Balance : 2;
        struct _MMADDRESS_NODE *Parent;
    } u1;
    struct _MMADDRESS_NODE *LeftChild;
    struct _MMADDRESS_NODE *RightChild;
    ULONG_PTR StartingVpn;
    ULONG_PTR EndingVpn;
} MMADDRESS_NODE, *PMMADDRESS_NODE;

//
// A pair of macros to deal with the packing of parent & balance in the
// MMADDRESS_NODE.
//

#define SANITIZE_PARENT_NODE(Parent) ((PMMADDRESS_NODE)(((ULONG_PTR)(Parent)) & ~0x3))

//
// Macro to carefully preserve the balance while updating the parent.
//

#define MI_MAKE_PARENT(ParentNode,ExistingBalance) \
                (PMMADDRESS_NODE)((ULONG_PTR)(ParentNode) | ((ExistingBalance) & 0x3))

typedef struct _MM_AVL_TABLE {
    MMADDRESS_NODE  BalancedRoot;
    ULONG_PTR DepthOfTree: 5;
    ULONG_PTR Unused: 3;
#if defined (_WIN64)
    ULONG_PTR NumberGenericTableElements: 56;
#else
    ULONG_PTR NumberGenericTableElements: 24;
#endif
    PVOID NodeHint;
    PVOID NodeFreeHint;
} MM_AVL_TABLE, *PMM_AVL_TABLE;

//
// Client impersonation information.
//

typedef struct _PS_IMPERSONATION_INFORMATION {
    PACCESS_TOKEN Token;
    BOOLEAN CopyOnOpen;
    BOOLEAN EffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
} PS_IMPERSONATION_INFORMATION, *PPS_IMPERSONATION_INFORMATION;

//
// Audit Information structure: this is a member of the EPROCESS structure
// and currently contains only the name of the exec'ed image file.
//

typedef struct _SE_AUDIT_PROCESS_CREATION_INFO {
    POBJECT_NAME_INFORMATION ImageFileName;
} SE_AUDIT_PROCESS_CREATION_INFO, *PSE_AUDIT_PROCESS_CREATION_INFO;

typedef enum _PS_QUOTA_TYPE {
    PsNonPagedPool = 0,
    PsPagedPool    = 1,
    PsPageFile     = 2,
    PsQuotaTypes   = 3
} PS_QUOTA_TYPE, *PPS_QUOTA_TYPE;

typedef struct _EPROCESS_QUOTA_ENTRY {
    SIZE_T Usage;  // Current usage count
    SIZE_T Limit;  // Unhindered progress may be made to this point
    SIZE_T Peak;   // Peak quota usage
    SIZE_T Return; // Quota value to return to the pool once its big enough
} EPROCESS_QUOTA_ENTRY, *PEPROCESS_QUOTA_ENTRY;

//#define PS_TRACK_QUOTA 1

#define EPROCESS_QUOTA_TRACK_MAX 10000

typedef struct _EPROCESS_QUOTA_TRACK {
    SIZE_T Charge;
    PVOID Caller;
    PVOID FreeCaller;
    PVOID Process;
} EPROCESS_QUOTA_TRACK, *PEPROCESS_QUOTA_TRACK;

typedef struct _EPROCESS_QUOTA_BLOCK {
    EPROCESS_QUOTA_ENTRY QuotaEntry[PsQuotaTypes];
    LIST_ENTRY QuotaList; // All additional quota blocks are chained through here
    ULONG ReferenceCount;
    ULONG ProcessCount; // Total number of processes still referencing this block
#if defined (PS_TRACK_QUOTA)
    EPROCESS_QUOTA_TRACK Tracker[2][EPROCESS_QUOTA_TRACK_MAX];
#endif
} EPROCESS_QUOTA_BLOCK, *PEPROCESS_QUOTA_BLOCK;

//
// Pagefault monitoring.
//

typedef struct _PAGEFAULT_HISTORY {
    ULONG CurrentIndex;
    ULONG MaxIndex;
    KSPIN_LOCK SpinLock;
    PVOID Reserved;
    PROCESS_WS_WATCH_INFORMATION WatchInfo[1];
} PAGEFAULT_HISTORY, *PPAGEFAULT_HISTORY;

#define PS_WS_TRIM_FROM_EXE_HEADER        1
#define PS_WS_TRIM_BACKGROUND_ONLY_APP    2

//
// Wow64 process structure.
//



typedef struct _WOW64_PROCESS {
    PVOID Wow64;
} WOW64_PROCESS, *PWOW64_PROCESS;

#if defined (_WIN64)
#define PS_GET_WOW64_PROCESS(Process) ((Process)->Wow64Process)
#else
#define PS_GET_WOW64_PROCESS(Process) ((Process), ((PWOW64_PROCESS)NULL))
#endif

#define PS_SET_BITS(Flags, Flag) \
    RtlInterlockedSetBitsDiscardReturn (Flags, Flag)

#define PS_TEST_SET_BITS(Flags, Flag) \
    RtlInterlockedSetBits (Flags, Flag)

#define PS_CLEAR_BITS(Flags, Flag) \
    RtlInterlockedClearBitsDiscardReturn (Flags, Flag)

#define PS_TEST_CLEAR_BITS(Flags, Flag) \
    RtlInterlockedClearBits (Flags, Flag)

#define PS_SET_CLEAR_BITS(Flags, sFlag, cFlag) \
    RtlInterlockedSetClearBits (Flags, sFlag, cFlag)

#define PS_TEST_ALL_BITS_SET(Flags, Bits) \
    ((Flags&(Bits)) == (Bits))

// Process structure.
//
// If you remove a field from this structure, please also
// remove the reference to it from within the kernel debugger
// (nt\private\sdktools\ntsd\ntkext.c)
//

typedef struct _EPROCESS {
    KPROCESS Pcb;

    //
    // Lock used to protect:
    // The list of threads in the process.
    // Process token.
    // Win32 process field.
    // Process and thread affinity setting.
    //

    EX_PUSH_LOCK ProcessLock;

    LARGE_INTEGER CreateTime;
    LARGE_INTEGER ExitTime;

    //
    // Structure to allow lock free cross process access to the process
    // handle table, process section and address space. Acquire rundown
    // protection with this if you do cross process handle table, process
    // section or address space references.
    //

    EX_RUNDOWN_REF RundownProtect;

    HANDLE UniqueProcessId;

    //
    // Global list of all processes in the system. Processes are removed
    // from this list in the object deletion routine.  References to
    // processes in this list must be done with ObReferenceObjectSafe
    // because of this.
    //

    LIST_ENTRY ActiveProcessLinks;

    //
    // Quota Fields.
    //

    SIZE_T QuotaUsage[PsQuotaTypes];
    SIZE_T QuotaPeak[PsQuotaTypes];
    SIZE_T CommitCharge;

    //
    // VmCounters.
    //

    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;

    LIST_ENTRY SessionProcessLinks;

    PVOID DebugPort;
    PVOID ExceptionPort;
    PHANDLE_TABLE ObjectTable;

    //
    // Security.
    //

    EX_FAST_REF Token;

    PFN_NUMBER WorkingSetPage;
    KGUARDED_MUTEX AddressCreationLock;
    KSPIN_LOCK HyperSpaceLock;

    struct _ETHREAD *ForkInProgress;
    ULONG_PTR HardwareTrigger;

    PMM_AVL_TABLE PhysicalVadRoot;
    PVOID CloneRoot;
    PFN_NUMBER NumberOfPrivatePages;
    PFN_NUMBER NumberOfLockedPages;
    PVOID Win32Process;
    struct _EJOB *Job;
    PVOID SectionObject;

    PVOID SectionBaseAddress;

    PEPROCESS_QUOTA_BLOCK QuotaBlock;

    PPAGEFAULT_HISTORY WorkingSetWatch;
    HANDLE Win32WindowStation;
    HANDLE InheritedFromUniqueProcessId;

    PVOID LdtInformation;
    PVOID VadFreeHint;
    PVOID VdmObjects;
    PVOID DeviceMap;

    PVOID Spare0[3];
    union {
        HARDWARE_PTE PageDirectoryPte;
        ULONGLONG Filler;
    };
    PVOID Session;
    UCHAR ImageFileName[ 16 ];

    LIST_ENTRY JobLinks;
    PVOID LockedPagesList;

    LIST_ENTRY ThreadListHead;

    //
    // Used by rdr/security for authentication.
    //

    PVOID SecurityPort;

#ifdef _WIN64
    PWOW64_PROCESS Wow64Process;
#else
    PVOID PaeTop;
#endif

    ULONG ActiveThreads;

    ACCESS_MASK GrantedAccess;

    ULONG DefaultHardErrorProcessing;

    NTSTATUS LastThreadExitStatus;

    //
    // Peb
    //

    PPEB Peb;

    //
    // Pointer to the prefetches trace block.
    //
    EX_FAST_REF PrefetchTrace;

    LARGE_INTEGER ReadOperationCount;
    LARGE_INTEGER WriteOperationCount;
    LARGE_INTEGER OtherOperationCount;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;

    SIZE_T CommitChargeLimit;
    SIZE_T CommitChargePeak;

    PVOID AweInfo;

    //
    // This is used for SeAuditProcessCreation.
    // It contains the full path to the image file.
    //

    SE_AUDIT_PROCESS_CREATION_INFO SeAuditProcessCreationInfo;

    MMSUPPORT Vm;

#if !defined(_WIN64)
    LIST_ENTRY MmProcessLinks;
#else
    ULONG Spares[2];
#endif

    ULONG ModifiedPageCount;

    #define PS_JOB_STATUS_NOT_REALLY_ACTIVE      0x00000001UL
    #define PS_JOB_STATUS_ACCOUNTING_FOLDED      0x00000002UL
    #define PS_JOB_STATUS_NEW_PROCESS_REPORTED   0x00000004UL
    #define PS_JOB_STATUS_EXIT_PROCESS_REPORTED  0x00000008UL
    #define PS_JOB_STATUS_REPORT_COMMIT_CHANGES  0x00000010UL
    #define PS_JOB_STATUS_LAST_REPORT_MEMORY     0x00000020UL
    #define PS_JOB_STATUS_REPORT_PHYSICAL_PAGE_CHANGES  0x00000040UL

    ULONG JobStatus;


    //
    // Process flags. Use interlocked operations with PS_SET_BITS, etc
    // to modify these.
    //

    #define PS_PROCESS_FLAGS_CREATE_REPORTED        0x00000001UL // Create process debug call has occurred
    #define PS_PROCESS_FLAGS_NO_DEBUG_INHERIT       0x00000002UL // Don't inherit debug port
    #define PS_PROCESS_FLAGS_PROCESS_EXITING        0x00000004UL // PspExitProcess entered
    #define PS_PROCESS_FLAGS_PROCESS_DELETE         0x00000008UL // Delete process has been issued
    #define PS_PROCESS_FLAGS_WOW64_SPLIT_PAGES      0x00000010UL // Wow64 split pages
    #define PS_PROCESS_FLAGS_VM_DELETED             0x00000020UL // VM is deleted
    #define PS_PROCESS_FLAGS_OUTSWAP_ENABLED        0x00000040UL // Outswap enabled
    #define PS_PROCESS_FLAGS_OUTSWAPPED             0x00000080UL // Outswapped
    #define PS_PROCESS_FLAGS_FORK_FAILED            0x00000100UL // Fork status
    #define PS_PROCESS_FLAGS_WOW64_4GB_VA_SPACE     0x00000200UL // Wow64 process with 4gb virtual address space
    #define PS_PROCESS_FLAGS_ADDRESS_SPACE1         0x00000400UL // Addr space state1
    #define PS_PROCESS_FLAGS_ADDRESS_SPACE2         0x00000800UL // Addr space state2
    #define PS_PROCESS_FLAGS_SET_TIMER_RESOLUTION   0x00001000UL // SetTimerResolution has been called
    #define PS_PROCESS_FLAGS_BREAK_ON_TERMINATION   0x00002000UL // Break on process termination
    #define PS_PROCESS_FLAGS_CREATING_SESSION       0x00004000UL // Process is creating a session
    #define PS_PROCESS_FLAGS_USING_WRITE_WATCH      0x00008000UL // Process is using the write watch APIs
    #define PS_PROCESS_FLAGS_IN_SESSION             0x00010000UL // Process is in a session
    #define PS_PROCESS_FLAGS_OVERRIDE_ADDRESS_SPACE 0x00020000UL // Process must use native address space (Win64 only)
    #define PS_PROCESS_FLAGS_HAS_ADDRESS_SPACE      0x00040000UL // This process has an address space
    #define PS_PROCESS_FLAGS_LAUNCH_PREFETCHED      0x00080000UL // Process launch was prefetched
    #define PS_PROCESS_INJECT_INPAGE_ERRORS         0x00100000UL // Process should be given inpage errors - hardcoded in trap.asm too
    #define PS_PROCESS_FLAGS_VM_TOP_DOWN            0x00200000UL // Process memory allocations default to top-down
    #define PS_PROCESS_FLAGS_IMAGE_NOTIFY_DONE      0x00400000UL // We have sent a message for this image
    #define PS_PROCESS_FLAGS_PDE_UPDATE_NEEDED      0x00800000UL // The system PDEs need updating for this process (NT32 only)
    #define PS_PROCESS_FLAGS_VDM_ALLOWED            0x01000000UL // Process allowed to invoke NTVDM support
    #define PS_PROCESS_FLAGS_SMAP_ALLOWED           0x02000000UL // Process allowed to invoke SMAP support
    #define PS_PROCESS_FLAGS_CREATE_FAILED          0x04000000UL // Process create failed

    #define PS_PROCESS_FLAGS_DEFAULT_IO_PRIORITY    0x38000000UL // The default I/O priority for created threads. (3 bits)

    #define PS_PROCESS_FLAGS_PRIORITY_SHIFT         27
    
    #define PS_PROCESS_FLAGS_EXECUTE_SPARE1         0x40000000UL //
    #define PS_PROCESS_FLAGS_EXECUTE_SPARE2         0x80000000UL //


    union {

        ULONG Flags;

        //
        // Fields can only be set by the PS_SET_BITS and other interlocked
        // macros.  Reading fields is best done via the bit definitions so
        // references are easy to locate.
        //

        struct {
            ULONG CreateReported            : 1;
            ULONG NoDebugInherit            : 1;
            ULONG ProcessExiting            : 1;
            ULONG ProcessDelete             : 1;
            ULONG Wow64SplitPages           : 1;
            ULONG VmDeleted                 : 1;
            ULONG OutswapEnabled            : 1;
            ULONG Outswapped                : 1;
            ULONG ForkFailed                : 1;
            ULONG Wow64VaSpace4Gb           : 1;
            ULONG AddressSpaceInitialized   : 2;
            ULONG SetTimerResolution        : 1;
            ULONG BreakOnTermination        : 1;
            ULONG SessionCreationUnderway   : 1;
            ULONG WriteWatch                : 1;
            ULONG ProcessInSession          : 1;
            ULONG OverrideAddressSpace      : 1;
            ULONG HasAddressSpace           : 1;
            ULONG LaunchPrefetched          : 1;
            ULONG InjectInpageErrors        : 1;
            ULONG VmTopDown                 : 1;
            ULONG ImageNotifyDone           : 1;
            ULONG PdeUpdateNeeded           : 1;    // NT32 only
            ULONG VdmAllowed                : 1;
            ULONG SmapAllowed               : 1;
            ULONG CreateFailed              : 1;
            ULONG DefaultIoPriority         : 3;
            ULONG Spare1                    : 1;
            ULONG Spare2                    : 1;
        };
    };

    NTSTATUS ExitStatus;

    USHORT NextPageColor;
    union {
        struct {
            UCHAR SubSystemMinorVersion;
            UCHAR SubSystemMajorVersion;
        };
        USHORT SubSystemVersion;
    };
    UCHAR PriorityClass;

    MM_AVL_TABLE VadRoot;

    ULONG Cookie;

} EPROCESS, *PEPROCESS; 

C_ASSERT( FIELD_OFFSET(EPROCESS, Pcb) == 0 );
           
//
// Thread termination port
//

typedef struct _TERMINATION_PORT {
    struct _TERMINATION_PORT *Next;
    PVOID Port;
} TERMINATION_PORT, *PTERMINATION_PORT;


// Thread Object
//
// Thread object body.  A pointer to this structure is returned when a handle
// to a thread object is referenced.  This structure contains a thread control
// block (TCB) which is the kernel's representation of a thread.
//

#define PS_GET_THREAD_CREATE_TIME(Thread) ((Thread)->CreateTime.QuadPart)

#define PS_SET_THREAD_CREATE_TIME(Thread, InputCreateTime) \
            ((Thread)->CreateTime.QuadPart = (InputCreateTime.QuadPart))

//
// Macro to return TRUE if the specified thread is impersonating.
//

#define PS_IS_THREAD_IMPERSONATING(Thread) (((Thread)->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_IMPERSONATING) != 0)

typedef struct _ETHREAD {
    KTHREAD Tcb;

    LARGE_INTEGER CreateTime;

    union {
        LARGE_INTEGER ExitTime;
        LIST_ENTRY LpcReplyChain;
        LIST_ENTRY KeyedWaitChain;
    };
    union {
        NTSTATUS ExitStatus;
        PVOID OfsChain;
    };

    //
    // Registry
    //

    LIST_ENTRY PostBlockList;

    //
    // Single linked list of termination blocks
    //

    union {
        //
        // List of termination ports
        //

        PTERMINATION_PORT TerminationPort;

        //
        // List of threads to be reaped. Only used at thread exit
        //

        struct _ETHREAD *ReaperLink;

        //
        // Keyvalue being waited for
        //
        PVOID KeyedWaitValue;

    };

    KSPIN_LOCK ActiveTimerListLock;
    LIST_ENTRY ActiveTimerListHead;

    CLIENT_ID Cid;

    //
    // Lpc
    //

    union {
        KSEMAPHORE LpcReplySemaphore;
        KSEMAPHORE KeyedWaitSemaphore;
    };

    union {
        PVOID LpcReplyMessage;          // -> Message that contains the reply
        PVOID LpcWaitingOnPort;
    };

    //
    // Security
    //
    //
    //    Client - If non null, indicates the thread is impersonating
    //        a client.
    //

    PPS_IMPERSONATION_INFORMATION ImpersonationInfo;

    //
    // Io
    //

    LIST_ENTRY IrpList;

    //
    //  File Systems
    //

    ULONG_PTR TopLevelIrp;  // either NULL, an Irp or a flag defined in FsRtl.h
    struct _DEVICE_OBJECT *DeviceToVerify;

    PEPROCESS ThreadsProcess;
    PVOID StartAddress;
    union {
        PVOID Win32StartAddress;
        ULONG LpcReceivedMessageId;
    };
    //
    // Ps
    //

    LIST_ENTRY ThreadListEntry;

    //
    // Rundown protection structure. Acquire this to do cross thread
    // TEB, TEB32 or stack references.
    //

    EX_RUNDOWN_REF RundownProtect;

    //
    // Lock to protect thread impersonation information
    //
    EX_PUSH_LOCK ThreadLock;

    ULONG LpcReplyMessageId;    // MessageId this thread is waiting for reply to

    ULONG ReadClusterSize;

    //
    // Client/server
    //

    ACCESS_MASK GrantedAccess;

    //
    // Flags for cross thread access. Use interlocked operations
    // via PS_SET_BITS etc.
    //

    //
    // Used to signify that the delete APC has been queued or the
    // thread has called PspExitThread itself.
    //

    #define PS_CROSS_THREAD_FLAGS_TERMINATED           0x00000001UL

    //
    // Thread create failed
    //

    #define PS_CROSS_THREAD_FLAGS_DEADTHREAD           0x00000002UL

    //
    // Debugger isn't shown this thread
    //

    #define PS_CROSS_THREAD_FLAGS_HIDEFROMDBG          0x00000004UL

    //
    // Thread is impersonating
    //

    #define PS_CROSS_THREAD_FLAGS_IMPERSONATING        0x00000008UL

    //
    // This is a system thread
    //

    #define PS_CROSS_THREAD_FLAGS_SYSTEM               0x00000010UL

    //
    // Hard errors are disabled for this thread
    //

    #define PS_CROSS_THREAD_FLAGS_HARD_ERRORS_DISABLED 0x00000020UL

    //
    // We should break in when this thread is terminated
    //

    #define PS_CROSS_THREAD_FLAGS_BREAK_ON_TERMINATION 0x00000040UL

    //
    // This thread should skip sending its create thread message
    //
    #define PS_CROSS_THREAD_FLAGS_SKIP_CREATION_MSG    0x00000080UL

    //
    // This thread should skip sending its final thread termination message
    //
    #define PS_CROSS_THREAD_FLAGS_SKIP_TERMINATION_MSG 0x00000100UL

    union {

        ULONG CrossThreadFlags;

        //
        // The following fields are for the debugger only. Do not use.
        // Use the bit definitions instead.
        //

        struct {
            ULONG Terminated              : 1;
            ULONG DeadThread              : 1;
            ULONG HideFromDebugger        : 1;
            ULONG ActiveImpersonationInfo : 1;
            ULONG SystemThread            : 1;
            ULONG HardErrorsAreDisabled   : 1;
            ULONG BreakOnTermination      : 1;
            ULONG SkipCreationMsg         : 1;
            ULONG SkipTerminationMsg      : 1;
        };
    };

    //
    // Flags to be accessed in this thread's context only at PASSIVE
    // level -- no need to use interlocked operations.
    //

    union {
        ULONG SameThreadPassiveFlags;

        struct {

            //
            // This thread is an active Ex worker thread; it should
            // not terminate.
            //

            ULONG ActiveExWorker : 1;
            ULONG ExWorkerCanWaitUser : 1;
            ULONG MemoryMaker : 1;

            //
            // Thread is active in the keyed event code. LPC should not run above this in an APC.
            //
            ULONG KeyedEventInUse : 1;
        };
    };

    //
    // Flags to be accessed in this thread's context only at APC_LEVEL.
    // No need to use interlocked operations.
    //

    union {
        ULONG SameThreadApcFlags;
        struct {

            //
            // The stored thread's MSGID is valid. This is only accessed
            // while the LPC mutex is held so it's an APC_LEVEL flag.
            //

            BOOLEAN LpcReceivedMsgIdValid : 1;
            BOOLEAN LpcExitThreadCalled   : 1;
            BOOLEAN AddressSpaceOwner     : 1;
            BOOLEAN OwnsProcessWorkingSetExclusive  : 1;
            BOOLEAN OwnsProcessWorkingSetShared     : 1;
            BOOLEAN OwnsSystemWorkingSetExclusive   : 1;
            BOOLEAN OwnsSystemWorkingSetShared      : 1;
            BOOLEAN OwnsSessionWorkingSetExclusive  : 1;
            BOOLEAN OwnsSessionWorkingSetShared     : 1;

            #define PS_SAME_THREAD_FLAGS_OWNS_A_WORKING_SET    0x000001F8UL

            BOOLEAN ApcNeeded                       : 1;
        };
    };

    BOOLEAN ForwardClusterOnly;
    BOOLEAN DisablePageFaultClustering;
    UCHAR ActiveFaultCount;

#if defined (PERF_DATA)
    ULONG PerformanceCountLow;
    LONG PerformanceCountHigh;
#endif

} ETHREAD, *PETHREAD;

C_ASSERT( FIELD_OFFSET(ETHREAD, Tcb) == 0 );

//
// The following two inline functions allow a thread or process object to
// be converted into a kernel thread or process, respectively, without
// having to expose the ETHREAD and EPROCESS definitions to the world.
//
// These functions take advantage of the fact that the kernel structures
// appear as the first element in the respective object structures.
//
// begin_ntosp

PKTHREAD
FORCEINLINE
PsGetKernelThread(
    IN PETHREAD ThreadObject
    )
{
    return (PKTHREAD)ThreadObject;
}

PKPROCESS
FORCEINLINE
PsGetKernelProcess(
    IN PEPROCESS ProcessObject
    )
{
    return (PKPROCESS)ProcessObject;
}

NTKERNELAPI
NTSTATUS
PsGetContextThread(
    __in PETHREAD Thread,
    __inout PCONTEXT ThreadContext,
    __in KPROCESSOR_MODE Mode
    );

NTKERNELAPI
NTSTATUS
PsSetContextThread(
    __in PETHREAD Thread,
    __in PCONTEXT ThreadContext,
    __in KPROCESSOR_MODE Mode
    );

// end_ntosp

//
// Initial PEB
//

typedef struct _INITIAL_PEB {
    BOOLEAN InheritedAddressSpace;      // These four fields cannot change unless the
    BOOLEAN ReadImageFileExecOptions;   //
    BOOLEAN BeingDebugged;              //
    union {
        BOOLEAN BitField;                  //
        struct {
            BOOLEAN ImageUsesLargePages : 1;
            BOOLEAN SpareBits : 7;
         };
    };
    HANDLE Mutant;                      // PEB structure is also updated.
} INITIAL_PEB, *PINITIAL_PEB;

#if defined(_WIN64)
typedef struct _INITIAL_PEB32 {
    BOOLEAN InheritedAddressSpace;      // These four fields cannot change unless the
    BOOLEAN ReadImageFileExecOptions;   //
    BOOLEAN BeingDebugged;              //
    union {
        BOOLEAN BitField;                  //
        struct {
            BOOLEAN ImageUsesLargePages : 1;
            BOOLEAN SpareBits : 7;
         };
    };
    LONG Mutant;                        // PEB structure is also updated.
} INITIAL_PEB32, *PINITIAL_PEB32;
#endif

typedef struct _PS_JOB_TOKEN_FILTER {
    ULONG CapturedSidCount ;
    PSID_AND_ATTRIBUTES CapturedSids ;
    ULONG CapturedSidsLength ;

    ULONG CapturedGroupCount ;
    PSID_AND_ATTRIBUTES CapturedGroups ;
    ULONG CapturedGroupsLength ;

    ULONG CapturedPrivilegeCount ;
    PLUID_AND_ATTRIBUTES CapturedPrivileges ;
    ULONG CapturedPrivilegesLength ;
} PS_JOB_TOKEN_FILTER, * PPS_JOB_TOKEN_FILTER ;

//
// Job Object
//

typedef struct _EJOB {
    KEVENT Event;

    //
    // All jobs are chained together via this list.
    // Protected by the global lock PspJobListLock
    //

    LIST_ENTRY JobLinks;

    //
    // All processes within this job. Processes are removed from this
    // list at last dereference. Safe object referencing needs to be done.
    // Protected by the joblock.
    //

    LIST_ENTRY ProcessListHead;
    ERESOURCE JobLock;

    //
    // Accounting Info
    //

    LARGE_INTEGER TotalUserTime;
    LARGE_INTEGER TotalKernelTime;
    LARGE_INTEGER ThisPeriodTotalUserTime;
    LARGE_INTEGER ThisPeriodTotalKernelTime;
    ULONG TotalPageFaultCount;
    ULONG TotalProcesses;
    ULONG ActiveProcesses;
    ULONG TotalTerminatedProcesses;

    //
    // Limitable Attributes
    //

    LARGE_INTEGER PerProcessUserTimeLimit;
    LARGE_INTEGER PerJobUserTimeLimit;
    ULONG LimitFlags;
    SIZE_T MinimumWorkingSetSize;
    SIZE_T MaximumWorkingSetSize;
    ULONG ActiveProcessLimit;
    KAFFINITY Affinity;
    UCHAR PriorityClass;

    //
    // UI restrictions
    //

    ULONG UIRestrictionsClass;

    //
    // Security Limitations:  write once, read always
    //

    ULONG SecurityLimitFlags;
    PACCESS_TOKEN Token;
    PPS_JOB_TOKEN_FILTER Filter;

    //
    // End Of Job Time Limit
    //

    ULONG EndOfJobTimeAction;
    PVOID CompletionPort;
    PVOID CompletionKey;

    ULONG SessionId;

    ULONG SchedulingClass;

    ULONGLONG ReadOperationCount;
    ULONGLONG WriteOperationCount;
    ULONGLONG OtherOperationCount;
    ULONGLONG ReadTransferCount;
    ULONGLONG WriteTransferCount;
    ULONGLONG OtherTransferCount;

    //
    // Extended Limits
    //

    IO_COUNTERS IoInfo;         // not used yet
    SIZE_T ProcessMemoryLimit;
    SIZE_T JobMemoryLimit;
    SIZE_T PeakProcessMemoryUsed;
    SIZE_T PeakJobMemoryUsed;
    SIZE_T CurrentJobMemoryUsed;

    KGUARDED_MUTEX MemoryLimitsLock;

    //
    // List of jobs in a job set. Processes within a job in a job set
    // can create processes in the same or higher members of the jobset.
    // Protected by the global lock PspJobListLock
    //

    LIST_ENTRY JobSetLinks;

    //
    // Member level for this job in the jobset.
    //

    ULONG MemberLevel;

    //
    // This job has had its last handle closed.
    //

#define PS_JOB_FLAGS_CLOSE_DONE 0x1UL

    ULONG JobFlags;
} EJOB;
typedef EJOB *PEJOB;


//
// Global Variables
//

extern ULONG PsPrioritySeparation;
extern ULONG PsRawPrioritySeparation;
extern LIST_ENTRY PsActiveProcessHead;
extern const UNICODE_STRING PsNtDllPathName;
extern PVOID PsSystemDllBase;
extern PEPROCESS PsInitialSystemProcess;
extern PVOID PsNtosImageBase;
extern PVOID PsHalImageBase;

#if defined(_AMD64_)

extern INVERTED_FUNCTION_TABLE PsInvertedFunctionTable;

#endif

extern LIST_ENTRY PsLoadedModuleList;
extern ERESOURCE PsLoadedModuleResource;
extern ALIGNED_SPINLOCK PsLoadedModuleSpinLock;
extern LCID PsDefaultSystemLocaleId;
extern LCID PsDefaultThreadLocaleId;
extern LANGID PsDefaultUILanguageId;
extern LANGID PsInstallUILanguageId;
extern PEPROCESS PsIdleProcess;
extern SINGLE_LIST_ENTRY PsReaperListHead;
extern WORK_QUEUE_ITEM PsReaperWorkItem;

#define PS_EMBEDDED_NO_USERMODE 1 // no user mode code will run on the system

extern ULONG PsEmbeddedNTMask;

BOOLEAN
PsChangeJobMemoryUsage(
    IN ULONG Flags,
    IN SSIZE_T Amount
    );

VOID
PsReportProcessMemoryLimitViolation(
    VOID
    );

#define THREAD_HIT_SLOTS 750

extern ULONG PsThreadHits[THREAD_HIT_SLOTS];

VOID
PsThreadHit(
    IN PETHREAD Thread
    );

VOID
PsEnforceExecutionTimeLimits(
    VOID
    );

BOOLEAN
PsInitSystem (
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
PsMapSystemDll (
    IN PEPROCESS Process,
    OUT PVOID *DllBase OPTIONAL,
    IN LOGICAL UseLargePages
    );

VOID
PsInitializeQuotaSystem (
    VOID
    );

LOGICAL
PsShutdownSystem (
    VOID
    );

BOOLEAN
PsWaitForAllProcesses (
    VOID);

NTSTATUS
PsLocateSystemDll (
    BOOLEAN ReplaceExisting
    );

VOID
PsChangeQuantumTable(
    BOOLEAN ModifyActiveProcesses,
    ULONG PrioritySeparation
    );

//
// Get Current Prototypes
//
#define THREAD_TO_PROCESS(Thread) ((Thread)->ThreadsProcess)
#define IS_SYSTEM_THREAD(Thread)  (((Thread)->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_SYSTEM) != 0)

#define _PsGetCurrentProcess() (CONTAINING_RECORD(((KeGetCurrentThread())->ApcState.Process),EPROCESS,Pcb))
#define PsGetCurrentProcessByThread(xCurrentThread) (ASSERT((xCurrentThread) == PsGetCurrentThread ()),CONTAINING_RECORD(((xCurrentThread)->Tcb.ApcState.Process),EPROCESS,Pcb))

#define _PsGetCurrentThread() ((PETHREAD)KeGetCurrentThread())

//
// N.B. The kernel thread object is architecturally defined as being at offset
//      zero of the executive thread object. This assumption has been exported
//      from ntddk.h for some time.
//

C_ASSERT(FIELD_OFFSET(ETHREAD, Tcb) == 0);

#if defined(_NTOSP_)

// begin_ntosp

NTKERNELAPI
PEPROCESS
PsGetCurrentProcess(
    VOID
    );

#define PsGetCurrentThread() ((PETHREAD)KeGetCurrentThread())

// end_ntosp

#else

#define PsGetCurrentProcess() _PsGetCurrentProcess()

#define PsGetCurrentThread() _PsGetCurrentThread()

#endif

//
// Exit kernel mode APC routine.
//

VOID
PsExitSpecialApc(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    );

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp
//
// System Thread and Process Creation and Termination
//

NTKERNELAPI
NTSTATUS
PsCreateSystemThread(
    __out PHANDLE ThreadHandle,
    __in ULONG DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in_opt  HANDLE ProcessHandle,
    __out_opt PCLIENT_ID ClientId,
    __in PKSTART_ROUTINE StartRoutine,
    __in_opt PVOID StartContext
    );

NTKERNELAPI
NTSTATUS
PsTerminateSystemThread(
    __in NTSTATUS ExitStatus
    );

NTKERNELAPI
NTSTATUS
PsWrapApcWow64Thread (
    __inout PVOID *ApcContext,
    __inout PVOID *ApcRoutine);


// end_ntddk end_wdm end_nthal end_ntifs end_ntosp

NTKERNELAPI
NTSTATUS
PsCreateSystemProcess(
    __out PHANDLE ProcessHandle,
    __in ULONG DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes
    );

typedef
VOID (*PBBT_NOTIFY_ROUTINE)(
    PKTHREAD Thread
    );

NTKERNELAPI
ULONG
PsSetBBTNotifyRoutine(
    __in PBBT_NOTIFY_ROUTINE BBTNotifyRoutine
    );

// begin_ntifs begin_ntddk

typedef
VOID
(*PCREATE_PROCESS_NOTIFY_ROUTINE)(
    IN HANDLE ParentId,
    IN HANDLE ProcessId,
    IN BOOLEAN Create
    );

NTKERNELAPI
NTSTATUS
PsSetCreateProcessNotifyRoutine(
    __in PCREATE_PROCESS_NOTIFY_ROUTINE NotifyRoutine,
    __in BOOLEAN Remove
    );

typedef
VOID
(*PCREATE_THREAD_NOTIFY_ROUTINE)(
    IN HANDLE ProcessId,
    IN HANDLE ThreadId,
    IN BOOLEAN Create
    );

NTKERNELAPI
NTSTATUS
PsSetCreateThreadNotifyRoutine(
    __in PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine
    );

NTKERNELAPI
NTSTATUS
PsRemoveCreateThreadNotifyRoutine (
    __in PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine
    );

//
// Structures for Load Image Notify
//

typedef struct _IMAGE_INFO {
    union {
        ULONG Properties;
        struct {
            ULONG ImageAddressingMode  : 8;  // code addressing mode
            ULONG SystemModeImage      : 1;  // system mode image
            ULONG ImageMappedToAllPids : 1;  // image mapped into all processes
            ULONG Reserved             : 22;
        };
    };
    PVOID       ImageBase;
    ULONG       ImageSelector;
    SIZE_T      ImageSize;
    ULONG       ImageSectionNumber;
} IMAGE_INFO, *PIMAGE_INFO;

#define IMAGE_ADDRESSING_MODE_32BIT     3

typedef
VOID
(*PLOAD_IMAGE_NOTIFY_ROUTINE)(
    IN PUNICODE_STRING FullImageName,
    IN HANDLE ProcessId,                // pid into which image is being mapped
    IN PIMAGE_INFO ImageInfo
    );

NTKERNELAPI
NTSTATUS
PsSetLoadImageNotifyRoutine(
    __in PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine
    );

NTKERNELAPI
NTSTATUS
PsRemoveLoadImageNotifyRoutine(
    __in PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine
    );

// end_ntddk

//
// Security Support
//

NTKERNELAPI
NTSTATUS
PsAssignImpersonationToken(
    __in PETHREAD Thread,
    __in HANDLE Token
    );

// begin_ntosp

NTKERNELAPI
PACCESS_TOKEN
PsReferencePrimaryToken(
    __inout PEPROCESS Process
    );

NTKERNELAPI
VOID
PsDereferencePrimaryToken(
    __in PACCESS_TOKEN PrimaryToken
    );

NTKERNELAPI
VOID
PsDereferenceImpersonationToken(
    __in PACCESS_TOKEN ImpersonationToken
    );

// end_ntifs
// end_ntosp


#define PsDereferencePrimaryTokenEx(P,T) (ObFastDereferenceObject (&P->Token,(T)))

#define PsDereferencePrimaryToken(T) (ObDereferenceObject((T)))

#define PsDereferenceImpersonationToken(T)                                    \
            {if (ARGUMENT_PRESENT((T))) {                                     \
                (ObDereferenceObject((T)));                                   \
             } else {                                                         \
                ;                                                             \
             }                                                                \
            }


#define PsProcessAuditId(Process)    ((Process)->UniqueProcessId)

// begin_ntosp
// begin_ntifs

NTKERNELAPI
PACCESS_TOKEN
PsReferenceImpersonationToken(
    __inout PETHREAD Thread,
    __out PBOOLEAN CopyOnOpen,
    __out PBOOLEAN EffectiveOnly,
    __out PSECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );

// end_ntifs

PACCESS_TOKEN
PsReferenceEffectiveToken(
    IN PETHREAD Thread,
    OUT PTOKEN_TYPE TokenType,
    OUT PBOOLEAN EffectiveOnly,
    OUT PSECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );

// begin_ntifs

NTKERNELAPI
LARGE_INTEGER
PsGetProcessExitTime(
    VOID
    );

// end_ntifs
// end_ntosp

#if defined(_NTDDK_) || defined(_NTIFS_)

// begin_ntifs begin_ntosp

NTKERNELAPI
BOOLEAN
PsIsThreadTerminating(
    __in PETHREAD Thread
    );

// end_ntifs end_ntosp

#else

//
// BOOLEAN
// PsIsThreadTerminating(
//   IN PETHREAD Thread
//   )
//
//  Returns TRUE if thread is in the process of terminating.
//

#define PsIsThreadTerminating(T)                                            \
    (((T)->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_TERMINATED) != 0)

#endif

extern BOOLEAN PsImageNotifyEnabled;

VOID
PsCallImageNotifyRoutines(
    IN PUNICODE_STRING FullImageName,
    IN HANDLE ProcessId,                // pid into which image is being mapped
    IN PIMAGE_INFO ImageInfo
    );

// begin_ntifs
// begin_ntosp

NTKERNELAPI
NTSTATUS
PsImpersonateClient(
    __inout PETHREAD Thread,
    __in PACCESS_TOKEN Token,
    __in BOOLEAN CopyOnOpen,
    __in BOOLEAN EffectiveOnly,
    __in SECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );

// end_ntosp

NTKERNELAPI
BOOLEAN
PsDisableImpersonation(
    __inout PETHREAD Thread,
    __inout PSE_IMPERSONATION_STATE ImpersonationState
    );

NTKERNELAPI
VOID
PsRestoreImpersonation(
    __inout PETHREAD Thread,
    __in PSE_IMPERSONATION_STATE ImpersonationState
    );

// end_ntifs

// begin_ntosp begin_ntifs

NTKERNELAPI
VOID
PsRevertToSelf(
    VOID
    );

// end_ntifs

NTKERNELAPI
VOID
PsRevertThreadToSelf(
    __inout PETHREAD Thread
    );

// end_ntosp

NTSTATUS
PsOpenTokenOfThread(
    IN HANDLE ThreadHandle,
    IN BOOLEAN OpenAsSelf,
    OUT PACCESS_TOKEN *Token,
    OUT PBOOLEAN CopyOnOpen,
    OUT PBOOLEAN EffectiveOnly,
    OUT PSECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );

NTSTATUS
PsOpenTokenOfProcess(
    IN HANDLE ProcessHandle,
    OUT PACCESS_TOKEN *Token
    );

NTSTATUS
PsOpenTokenOfJob(
    IN HANDLE JobHandle,
    OUT PACCESS_TOKEN * Token
    );

//
// Cid
//

NTKERNELAPI
NTSTATUS
PsLookupProcessThreadByCid(
    __in PCLIENT_ID Cid,
    __deref_opt_out PEPROCESS *Process,
    __deref_out PETHREAD *Thread
    );

// begin_ntosp

NTKERNELAPI
NTSTATUS
PsLookupProcessByProcessId(
    __in HANDLE ProcessId,
    __deref_out PEPROCESS *Process
    );

NTKERNELAPI
NTSTATUS
PsLookupThreadByThreadId(
    __in HANDLE ThreadId,
    __deref_out PETHREAD *Thread
    );

// begin_ntifs
//
// Quota Operations
//

NTKERNELAPI
VOID
PsChargePoolQuota(
    __in PEPROCESS Process,
    __in POOL_TYPE PoolType,
    __in ULONG_PTR Amount
    );

NTKERNELAPI
NTSTATUS
PsChargeProcessPoolQuota(
    __in PEPROCESS Process,
    __in POOL_TYPE PoolType,
    __in ULONG_PTR Amount
    );

NTKERNELAPI
VOID
PsReturnPoolQuota(
    __in PEPROCESS Process,
    __in POOL_TYPE PoolType,
    __in ULONG_PTR Amount
    );

// end_ntifs
// end_ntosp

NTSTATUS
PsChargeProcessQuota (
    IN PEPROCESS Process,
    IN PS_QUOTA_TYPE QuotaType,
    IN SIZE_T Amount
    );

VOID
PsReturnProcessQuota (
    IN PEPROCESS Process,
    IN PS_QUOTA_TYPE QuotaType,
    IN SIZE_T Amount
    );

NTKERNELAPI
NTSTATUS
PsChargeProcessNonPagedPoolQuota(
    __in PEPROCESS Process,
    __in SIZE_T Amount
    );

NTKERNELAPI
VOID
PsReturnProcessNonPagedPoolQuota(
    __in PEPROCESS Process,
    __in SIZE_T Amount
    );

NTKERNELAPI
NTSTATUS
PsChargeProcessPagedPoolQuota(
    __in PEPROCESS Process,
    __in SIZE_T Amount
    );

NTKERNELAPI
VOID
PsReturnProcessPagedPoolQuota(
    __in PEPROCESS Process,
    __in SIZE_T Amount
    );

NTSTATUS
PsChargeProcessPageFileQuota(
    IN PEPROCESS Process,
    IN SIZE_T Amount
    );

VOID
PsReturnProcessPageFileQuota(
    IN PEPROCESS Process,
    IN SIZE_T Amount
    );

//
// Context Management
//

VOID
PspContextToKframes(
    OUT PKTRAP_FRAME TrapFrame,
    OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN PCONTEXT Context
    );

VOID
PspContextFromKframes(
    OUT PKTRAP_FRAME TrapFrame,
    OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN PCONTEXT Context
    );

VOID
PsReturnSharedPoolQuota(
    IN PEPROCESS_QUOTA_BLOCK QuotaBlock,
    IN ULONG_PTR PagedAmount,
    IN ULONG_PTR NonPagedAmount
    );

PEPROCESS_QUOTA_BLOCK
PsChargeSharedPoolQuota(
    IN PEPROCESS Process,
    IN ULONG_PTR PagedAmount,
    IN ULONG_PTR NonPagedAmount
    );

//
// Exception Handling
//

BOOLEAN
PsForwardException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN BOOLEAN DebugException,
    IN BOOLEAN SecondChance
    );

// begin_ntosp

typedef
NTSTATUS
(*PKWIN32_PROCESS_CALLOUT) (
    IN PEPROCESS Process,
    IN BOOLEAN Initialize
    );

typedef enum _PSW32JOBCALLOUTTYPE {
    PsW32JobCalloutSetInformation,
    PsW32JobCalloutAddProcess,
    PsW32JobCalloutTerminate
} PSW32JOBCALLOUTTYPE;

typedef struct _WIN32_JOBCALLOUT_PARAMETERS {
    PVOID Job;
    PSW32JOBCALLOUTTYPE CalloutType;
    IN PVOID Data;
} WIN32_JOBCALLOUT_PARAMETERS, *PKWIN32_JOBCALLOUT_PARAMETERS;

typedef
NTSTATUS
(*PKWIN32_JOB_CALLOUT) (
    IN PKWIN32_JOBCALLOUT_PARAMETERS Parm
     );

typedef enum _PSW32THREADCALLOUTTYPE {
    PsW32ThreadCalloutInitialize,
    PsW32ThreadCalloutExit
} PSW32THREADCALLOUTTYPE;

typedef
NTSTATUS
(*PKWIN32_THREAD_CALLOUT) (
    IN PETHREAD Thread,
    IN PSW32THREADCALLOUTTYPE CalloutType
    );

typedef enum _PSPOWEREVENTTYPE {
    PsW32FullWake,
    PsW32EventCode,
    PsW32PowerPolicyChanged,
    PsW32SystemPowerState,
    PsW32SystemTime,
    PsW32DisplayState,
    PsW32CapabilitiesChanged,
    PsW32SetStateFailed,
    PsW32GdiOff,
    PsW32GdiOn
} PSPOWEREVENTTYPE;

typedef struct _WIN32_POWEREVENT_PARAMETERS {
    PSPOWEREVENTTYPE EventNumber;
    ULONG_PTR Code;
} WIN32_POWEREVENT_PARAMETERS, *PKWIN32_POWEREVENT_PARAMETERS;

typedef enum _POWERSTATETASK {
    PowerState_BlockSessionSwitch,
    PowerState_Init,
    PowerState_QueryApps,
    PowerState_QueryFailed,
    PowerState_SuspendApps,
    PowerState_ShowUI,
    PowerState_NotifyWL,
    PowerState_ResumeApps,
    PowerState_UnBlockSessionSwitch

} POWERSTATETASK;

typedef struct _WIN32_POWERSTATE_PARAMETERS {
    BOOLEAN Promotion;
    POWER_ACTION SystemAction;
    SYSTEM_POWER_STATE MinSystemState;
    ULONG Flags;
    BOOLEAN fQueryDenied;
    POWERSTATETASK PowerStateTask;
} WIN32_POWERSTATE_PARAMETERS, *PKWIN32_POWERSTATE_PARAMETERS;

typedef
NTSTATUS
(*PKWIN32_POWEREVENT_CALLOUT) (
    IN PKWIN32_POWEREVENT_PARAMETERS Parm
    );

typedef
NTSTATUS
(*PKWIN32_POWERSTATE_CALLOUT) (
    IN PKWIN32_POWERSTATE_PARAMETERS Parm
    );

typedef
NTSTATUS
(*PKWIN32_OBJECT_CALLOUT) (
    IN PVOID Parm
    );

typedef struct _WIN32_CALLOUTS_FPNS {
    PKWIN32_PROCESS_CALLOUT ProcessCallout;
    PKWIN32_THREAD_CALLOUT ThreadCallout;
    PKWIN32_GLOBALATOMTABLE_CALLOUT GlobalAtomTableCallout;
    PKWIN32_POWEREVENT_CALLOUT PowerEventCallout;
    PKWIN32_POWERSTATE_CALLOUT PowerStateCallout;
    PKWIN32_JOB_CALLOUT JobCallout;
    PVOID BatchFlushRoutine;
    PKWIN32_OBJECT_CALLOUT DesktopOpenProcedure;
    PKWIN32_OBJECT_CALLOUT DesktopOkToCloseProcedure;
    PKWIN32_OBJECT_CALLOUT DesktopCloseProcedure;
    PKWIN32_OBJECT_CALLOUT DesktopDeleteProcedure;
    PKWIN32_OBJECT_CALLOUT WindowStationOkToCloseProcedure;
    PKWIN32_OBJECT_CALLOUT WindowStationCloseProcedure;
    PKWIN32_OBJECT_CALLOUT WindowStationDeleteProcedure;
    PKWIN32_OBJECT_CALLOUT WindowStationParseProcedure;
    PKWIN32_OBJECT_CALLOUT WindowStationOpenProcedure;
} WIN32_CALLOUTS_FPNS, *PKWIN32_CALLOUTS_FPNS;

NTKERNELAPI
VOID
PsEstablishWin32Callouts(
    __in PKWIN32_CALLOUTS_FPNS pWin32Callouts
    );

typedef enum _PSPROCESSPRIORITYMODE {
    PsProcessPriorityBackground,
    PsProcessPriorityForeground,
    PsProcessPrioritySpinning
} PSPROCESSPRIORITYMODE;

NTKERNELAPI
VOID
PsSetProcessPriorityByClass(
    __inout PEPROCESS Process,
    __in PSPROCESSPRIORITYMODE PriorityMode
    );

// end_ntosp

VOID
PsWatchWorkingSet(
    IN NTSTATUS Status,
    IN PVOID PcValue,
    IN PVOID Va
    );

// begin_ntddk begin_nthal begin_ntifs begin_ntosp

NTKERNELAPI
HANDLE
PsGetCurrentProcessId(
    VOID
    );

NTKERNELAPI
HANDLE
PsGetCurrentThreadId(
    VOID
    );

// end_ntosp

NTKERNELAPI
BOOLEAN
PsGetVersion(
    __out_opt PULONG MajorVersion,
    __out_opt PULONG MinorVersion,
    __out_opt PULONG BuildNumber,
    __out_opt PUNICODE_STRING CSDVersion
    );

// end_ntddk end_nthal end_ntifs

// begin_ntosp

NTKERNELAPI
ULONG
PsGetCurrentProcessSessionId(
    VOID
    );

NTKERNELAPI
PVOID
PsGetCurrentThreadStackLimit(
    VOID
    );

NTKERNELAPI
PVOID
PsGetCurrentThreadStackBase(
    VOID
    );

NTKERNELAPI
CCHAR
PsGetCurrentThreadPreviousMode(
    VOID
    );

NTKERNELAPI
PERESOURCE
PsGetJobLock(
    __in PEJOB Job
    );

NTKERNELAPI
ULONG
PsGetJobSessionId(
    __in PEJOB Job
    );

NTKERNELAPI
ULONG
PsGetJobUIRestrictionsClass(
    __in PEJOB Job
    );

NTKERNELAPI
LONGLONG
PsGetProcessCreateTimeQuadPart(
    __in PEPROCESS Process
    );

NTKERNELAPI
PVOID
PsGetProcessDebugPort(
    __in PEPROCESS Process
    );

NTKERNELAPI
BOOLEAN
PsIsProcessBeingDebugged(
    __in PEPROCESS Process
    );

NTKERNELAPI
BOOLEAN
PsGetProcessExitProcessCalled(
    __in PEPROCESS Process
    );

NTKERNELAPI
NTSTATUS
PsGetProcessExitStatus(
    __in PEPROCESS Process
    );

NTKERNELAPI
HANDLE
PsGetProcessId(
    __in PEPROCESS Process
    );

NTKERNELAPI
UCHAR *
PsGetProcessImageFileName(
    __in PEPROCESS Process
    );

#define PsGetCurrentProcessImageFileName() PsGetProcessImageFileName(PsGetCurrentProcess())

NTKERNELAPI
HANDLE
PsGetProcessInheritedFromUniqueProcessId(
    __in PEPROCESS Process
    );

NTKERNELAPI
PEJOB
PsGetProcessJob(
    __in PEPROCESS Process
    );

NTKERNELAPI
ULONG
PsGetProcessSessionId(
    __in PEPROCESS Process
    );

NTKERNELAPI
ULONG
PsGetProcessSessionIdEx(
    __in PEPROCESS Process
    );

NTKERNELAPI
PVOID
PsGetProcessSectionBaseAddress(
    __in PEPROCESS Process
    );

#define PsGetProcessPcb(Process) ((PKPROCESS)(Process))

NTKERNELAPI
PPEB
PsGetProcessPeb(
    __in PEPROCESS Process
    );

NTKERNELAPI
UCHAR
PsGetProcessPriorityClass(
    __in PEPROCESS Process
    );

NTKERNELAPI
HANDLE
PsGetProcessWin32WindowStation(
    __in PEPROCESS Process
    );

#define PsGetCurrentProcessWin32WindowStation() PsGetProcessWin32WindowStation(PsGetCurrentProcess())

NTKERNELAPI
PVOID
PsGetProcessWin32Process(
    __in PEPROCESS Process
    );

NTKERNELAPI
PVOID
PsGetCurrentProcessWin32Process(
    VOID
    );

#if defined(_WIN64)

NTKERNELAPI
PVOID
PsGetProcessWow64Process(
    __in PEPROCESS Process
    );

NTKERNELAPI
PVOID
PsGetCurrentProcessWow64Process(
    VOID
    );

#endif

NTKERNELAPI
HANDLE
PsGetThreadId(
    __in PETHREAD Thread
     );

NTKERNELAPI
CCHAR
PsGetThreadFreezeCount(
    __in PETHREAD Thread
    );

NTKERNELAPI
BOOLEAN
PsGetThreadHardErrorsAreDisabled(
    __in PETHREAD Thread
    );

NTKERNELAPI
PEPROCESS
PsGetThreadProcess(
    __in PETHREAD Thread
    );

NTKERNELAPI
PEPROCESS
PsGetCurrentThreadProcess(
    VOID
    );

NTKERNELAPI
HANDLE
PsGetThreadProcessId(
    __in PETHREAD Thread
    );

NTKERNELAPI
HANDLE
PsGetCurrentThreadProcessId(
    VOID
    );

NTKERNELAPI
ULONG
PsGetThreadSessionId(
    __in PETHREAD Thread
    );

#define  PsGetThreadTcb(Thread) ((PKTHREAD)(Thread))

NTKERNELAPI
PVOID
PsGetThreadTeb(
    __in PETHREAD Thread
    );

NTKERNELAPI
PVOID
PsGetCurrentThreadTeb(
    VOID
    );

NTKERNELAPI
PVOID
PsGetThreadWin32Thread(
    __in PETHREAD Thread
    );

NTKERNELAPI
PVOID
PsGetCurrentThreadWin32Thread(
    VOID
    );

NTKERNELAPI
PVOID
PsGetCurrentThreadWin32ThreadAndEnterCriticalRegion(
    __out PHANDLE ProcessId
    );

NTKERNELAPI                         //ntifs
BOOLEAN                             //ntifs
PsIsSystemThread(                   //ntifs
    __in PETHREAD Thread                 //ntifs
    );                              //ntifs

NTKERNELAPI
BOOLEAN
PsIsSystemProcess(
    __in PEPROCESS Process
     );   

NTKERNELAPI
BOOLEAN
PsIsThreadImpersonating (
    __in PETHREAD Thread
    );

NTSTATUS
PsReferenceProcessFilePointer (
    IN PEPROCESS Process,
    OUT PVOID *pFilePointer
    );

NTKERNELAPI
VOID
PsSetJobUIRestrictionsClass(
    __out PEJOB Job,
    __in ULONG UIRestrictionsClass
    );

NTKERNELAPI
VOID
PsSetProcessPriorityClass(
    __out PEPROCESS Process,
    __in UCHAR PriorityClass
    );

NTKERNELAPI
NTSTATUS
PsSetProcessWin32Process(
    __in PEPROCESS Process,
    __in PVOID Win32Process,
    __in PVOID PrevWin32Process
    );

NTKERNELAPI
VOID
PsSetProcessWindowStation(
    __out PEPROCESS Process,
    __in HANDLE Win32WindowStation
    );

NTKERNELAPI
VOID
PsSetThreadHardErrorsAreDisabled(
    __in PETHREAD Thread,
    __in BOOLEAN HardErrorsAreDisabled
    );

NTKERNELAPI
VOID
PsSetThreadWin32Thread(
    __inout PETHREAD Thread,
    __in PVOID Win32Thread,
    __in PVOID PrevWin32Thread
    );

NTKERNELAPI
PVOID
PsGetProcessSecurityPort(
    __in PEPROCESS Process
    );

NTKERNELAPI
NTSTATUS
PsSetProcessSecurityPort(
    __out PEPROCESS Process,
    __in PVOID Port
    );

typedef
NTSTATUS
(*PROCESS_ENUM_ROUTINE)(
    IN PEPROCESS Process,
    IN PVOID Context
    );

typedef
NTSTATUS
(*THREAD_ENUM_ROUTINE)(
    IN PEPROCESS Process,
    IN PETHREAD Thread,
    IN PVOID Context
    );

NTSTATUS
PsEnumProcesses (
    IN PROCESS_ENUM_ROUTINE CallBack,
    IN PVOID Context
    );

NTSTATUS
PsEnumProcessThreads (
    IN PEPROCESS Process,
    IN THREAD_ENUM_ROUTINE CallBack,
    IN PVOID Context
    );

PEPROCESS
PsGetNextProcess (
    IN PEPROCESS Process
    );

PETHREAD
PsGetNextProcessThread (
    IN PEPROCESS Process,
    IN PETHREAD Thread
    );

VOID
PsQuitNextProcess (
    IN PEPROCESS Process
    );

VOID
PsQuitNextProcessThread (
    IN PETHREAD Thread
    );

PEJOB
PsGetNextJob (
    IN PEJOB Job
    );

PEPROCESS
PsGetNextJobProcess (
    IN PEJOB Job,
    IN PEPROCESS Process
    );

VOID
PsQuitNextJob (
    IN PEJOB Job
    );

VOID
PsQuitNextJobProcess (
    IN PEPROCESS Process
    );

NTSTATUS
PsSuspendProcess (
    IN PEPROCESS Process
    );

NTSTATUS
PsResumeProcess (
    IN PEPROCESS Process
    );

NTSTATUS
PsTerminateProcess(
    IN PEPROCESS Process,
    IN NTSTATUS Status
    );

NTSTATUS
PsSuspendThread (
    IN PETHREAD Thread,
    OUT PULONG PreviousSuspendCount OPTIONAL
    );

NTSTATUS
PsResumeThread (
    IN PETHREAD Thread,
    OUT PULONG PreviousSuspendCount OPTIONAL
    );

#ifndef _WIN64

NTSTATUS
PsSetLdtEntries (
    IN ULONG Selector0,
    IN ULONG Entry0Low,
    IN ULONG Entry0Hi,
    IN ULONG Selector1,
    IN ULONG Entry1Low,
    IN ULONG Entry1Hi
    );

NTSTATUS
PsSetProcessLdtInfo (
    IN PPROCESS_LDT_INFORMATION LdtInformation,
    IN ULONG LdtInformationLength
    );

#endif

// end_ntosp

#endif // _PS_P
