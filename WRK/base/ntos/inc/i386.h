/*++ BUILD Version: 0014    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    i386.h

Abstract:

    This module contains the i386 hardware specific header file.

--*/

#if defined(__cplusplus)
extern "C" {
#endif // defined(__cplusplus)

#ifndef _i386_
#define _i386_

// begin_ntosp

#if defined(_X86_)
//
// Image header machine architecture
//

#define IMAGE_FILE_MACHINE_NATIVE   0x014c
#endif

// end_ntosp

#if !(defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_)) && !defined(_BLDR_)

#define ExRaiseException RtlRaiseException
#define ExRaiseStatus RtlRaiseStatus

#endif

// begin_ntddk begin_wdm begin_nthal begin_ntndis begin_ntosp

#if defined(_X86_)

//
// Types to use to contain PFNs and their counts.
//

typedef ULONG PFN_COUNT;

typedef LONG SPFN_NUMBER, *PSPFN_NUMBER;
typedef ULONG PFN_NUMBER, *PPFN_NUMBER;

//
// Define maximum size of flush multiple TB request.
//

#define FLUSH_MULTIPLE_MAXIMUM 32

//
// Indicate that the i386 compiler supports the pragma textout construct.
//

#define ALLOC_PRAGMA 1
//
// Indicate that the i386 compiler supports the DATA_SEG("INIT") and
// DATA_SEG("PAGE") pragmas
//

#define ALLOC_DATA_PRAGMA 1

// end_ntddk end_nthal end_ntndis end_wdm end_ntosp


//  NOTE -  KiPcr is only useful for PCR references where we know we
//          won't get context switched between the call to it and the
//          variable reference, OR, were we don't care, (ie TEB pointer)

//  NOTE - we must not macro out things we export
//      Things like KeFlushIcache and KeFlushDcache cannot be macro-ed
//      out because external code (like drivers) will want to import
//      them by name.  Therefore, the defines below that turn them into
//      nothing are inappropriate.


//
// Length on interrupt object dispatch code in longwords.
//            Reserve 9*4 space for ABIOS stack mapping.  If NO
//            ABIOS support the size of DISPATCH_LENGTH should be 74.
//

// begin_nthal

#define NORMAL_DISPATCH_LENGTH 106                  // ntddk wdm
#define DISPATCH_LENGTH NORMAL_DISPATCH_LENGTH      // ntddk wdm


//
// Define constants to access the bits in CR0.
//

#define CR0_PG  0x80000000          // paging
#define CR0_ET  0x00000010          // extension type (80387)
#define CR0_TS  0x00000008          // task switched
#define CR0_EM  0x00000004          // emulate math coprocessor
#define CR0_MP  0x00000002          // math present
#define CR0_PE  0x00000001          // protection enable

//
// More CR0 bits; these only apply to the 80486.
//

#define CR0_CD  0x40000000          // cache disable
#define CR0_NW  0x20000000          // not write-through
#define CR0_AM  0x00040000          // alignment mask
#define CR0_WP  0x00010000          // write protect
#define CR0_NE  0x00000020          // numeric error

//
// CR4 bits;  These only apply to Pentium
//
#define CR4_VME 0x00000001          // V86 mode extensions
#define CR4_PVI 0x00000002          // Protected mode virtual interrupts
#define CR4_TSD 0x00000004          // Time stamp disable
#define CR4_DE  0x00000008          // Debugging Extensions
#define CR4_PSE 0x00000010          // Page size extensions
#define CR4_PAE 0x00000020          // Physical address extensions
#define CR4_MCE 0x00000040          // Machine check enable
#define CR4_PGE 0x00000080          // Page global enable
#define CR4_FXSR 0x00000200         // FXSR used by OS
#define CR4_XMMEXCPT 0x00000400     // XMMI used by OS

// end_nthal

//
// Define constants to access ThNpxState
//

#define NPX_STATE_NOT_LOADED    (CR0_TS | CR0_MP)
#define NPX_STATE_LOADED        0

//
// External references to the labels defined in int.asm
//

extern ULONG KiInterruptTemplate[NORMAL_DISPATCH_LENGTH];
extern PULONG KiInterruptTemplateObject;
extern PULONG KiInterruptTemplateDispatch;
extern PULONG KiInterruptTemplate2ndDispatch;

// begin_ntddk begin_wdm begin_nthal begin_ntosp
//
// Interrupt Request Level definitions
//

#define PASSIVE_LEVEL 0             // Passive release level
#define LOW_LEVEL 0                 // Lowest interrupt level
#define APC_LEVEL 1                 // APC interrupt level
#define DISPATCH_LEVEL 2            // Dispatcher level

#define PROFILE_LEVEL 27            // timer used for profiling.
#define CLOCK1_LEVEL 28             // Interval clock 1 level - Not used on x86
#define CLOCK2_LEVEL 28             // Interval clock 2 level
#define IPI_LEVEL 29                // Interprocessor interrupt level
#define POWER_LEVEL 30              // Power failure level
#define HIGH_LEVEL 31               // Highest interrupt level

// end_ntddk end_wdm end_ntosp

#if defined(NT_UP)

// synchronization level - UP system
#define SYNCH_LEVEL DISPATCH_LEVEL  

#else

// synchronization level - MP system
#define SYNCH_LEVEL (IPI_LEVEL-2)   // ntddk wdm ntosp

#endif

#define KiSynchIrql SYNCH_LEVEL     // enable portable code

//
// Machine type definitions
//

#define MACHINE_TYPE_ISA 0
#define MACHINE_TYPE_EISA 1
#define MACHINE_TYPE_MCA 2

// end_nthal
//
//  The previous values are or'ed into KeI386MachineType.
//

extern ULONG KeI386MachineType;

// begin_nthal
//
// Define constants used in selector tests.
//
//  RPL_MASK is the real value for extracting RPL values.  IT IS THE WRONG
//  CONSTANT TO USE FOR MODE TESTING.
//
//  MODE_MASK is the value for deciding the current mode.
//  WARNING:    MODE_MASK assumes that all code runs at either ring-0
//              or ring-3.  Ring-1 or Ring-2 support will require changing
//              this value and all of the code that refers to it.

#define MODE_MASK    1      // ntosp
#define RPL_MASK     3

//
// SEGMENT_MASK is used to throw away trash part of segment.  Part always
// pushes or pops 32 bits to/from stack, but if it's a segment value,
// high order 16 bits are trash.
//

#define SEGMENT_MASK    0xffff

//
// Startup count value for KeStallExecution.  This value is used
// until KiInitializeStallExecution can compute the real one.
// Pick a value long enough for very fast processors.
//

#define INITIAL_STALL_COUNT 100

// end_nthal

//
// begin_nthal
//
// Macro to extract the high word of a long offset
//

#define HIGHWORD(l) \
    ((USHORT)(((ULONG)(l)>>16) & 0xffff))

//
// Macro to extract the low word of a long offset
//

#define LOWWORD(l) \
    ((USHORT)((ULONG)l & 0x0000ffff))

//
// Macro to combine two USHORT offsets into a long offset
//

#if !defined(MAKEULONG)

#define MAKEULONG(x, y) \
    (((((ULONG)(x))<<16) & 0xffff0000) | \
    ((ULONG)(y) & 0xffff))

#endif

// end_nthal

//
// Request a software interrupt.
//

#define KiRequestSoftwareInterrupt(RequestIrql) \
    HalRequestSoftwareInterrupt( RequestIrql )

// begin_ntddk begin_wdm begin_nthal begin_ntndis begin_ntosp

//
// I/O space read and write macros.
//
//  These have to be actual functions on the 386, because we need
//  to use assembler, but cannot return a value if we inline it.
//
//  The READ/WRITE_REGISTER_* calls manipulate I/O registers in MEMORY space.
//  (Use x86 move instructions, with LOCK prefix to force correct behavior
//   w.r.t. caches and write buffers.)
//
//  The READ/WRITE_PORT_* calls manipulate I/O registers in PORT space.
//  (Use x86 in/out instructions.)
//

NTKERNELAPI
UCHAR
NTAPI
READ_REGISTER_UCHAR(
    PUCHAR  Register
    );

NTKERNELAPI
USHORT
NTAPI
READ_REGISTER_USHORT(
    PUSHORT Register
    );

NTKERNELAPI
ULONG
NTAPI
READ_REGISTER_ULONG(
    PULONG  Register
    );

NTKERNELAPI
VOID
NTAPI
READ_REGISTER_BUFFER_UCHAR(
    PUCHAR  Register,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
NTAPI
READ_REGISTER_BUFFER_USHORT(
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
NTAPI
READ_REGISTER_BUFFER_ULONG(
    PULONG  Register,
    PULONG  Buffer,
    ULONG   Count
    );


NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_UCHAR(
    PUCHAR  Register,
    UCHAR   Value
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_USHORT(
    PUSHORT Register,
    USHORT  Value
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_ULONG(
    PULONG  Register,
    ULONG   Value
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_BUFFER_UCHAR(
    PUCHAR  Register,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_BUFFER_USHORT(
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_BUFFER_ULONG(
    PULONG  Register,
    PULONG  Buffer,
    ULONG   Count
    );

NTHALAPI
UCHAR
NTAPI
READ_PORT_UCHAR(
    PUCHAR  Port
    );

NTHALAPI
USHORT
NTAPI
READ_PORT_USHORT(
    PUSHORT Port
    );

NTHALAPI
ULONG
NTAPI
READ_PORT_ULONG(
    PULONG  Port
    );

NTHALAPI
VOID
NTAPI
READ_PORT_BUFFER_UCHAR(
    PUCHAR  Port,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
READ_PORT_BUFFER_USHORT(
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
READ_PORT_BUFFER_ULONG(
    PULONG  Port,
    PULONG  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_UCHAR(
    PUCHAR  Port,
    UCHAR   Value
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_USHORT(
    PUSHORT Port,
    USHORT  Value
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_ULONG(
    PULONG  Port,
    ULONG   Value
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_BUFFER_UCHAR(
    PUCHAR  Port,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_BUFFER_USHORT(
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_BUFFER_ULONG(
    PULONG  Port,
    PULONG  Buffer,
    ULONG   Count
    );

// end_ntndis
//
// Get data cache fill size.
//

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(KeGetDcacheFillSize)      // Use GetDmaAlignment
#endif

#define KeGetDcacheFillSize() 1L

// end_ntddk end_wdm end_nthal end_ntosp

//
// Fill TB entry.
//

#define KeFillEntryTb(Virtual)                  \
        KiFlushSingleTb (Virtual);

#if !defined(MIDL_PASS) && defined(_M_IX86) && !defined(_CROSS_PLATFORM_)

FORCEINLINE
VOID
KiFlushSingleTb (
    IN PVOID Virtual
    )
{
    __asm {
        mov eax, Virtual
        invlpg [eax]
    }
}

FORCEINLINE
VOID
KiFlushProcessTb (
    VOID
    )
{
    __asm {
        mov eax, cr3
        mov cr3, eax
    }
}

#endif

NTKERNELAPI                                         // nthal
VOID                                                // nthal
KeFlushCurrentTb (                                  // nthal
    VOID                                            // nthal
    );                                              // nthal
                                                    // nthal
//
// Data cache, instruction cache, I/O buffer, and write buffer flush routine
// prototypes.
//

//  386 and 486 have transparent caches, so these are noops.

#define KeSweepDcache(AllProcessors)
#define KeSweepCurrentDcache()

#define KeSweepIcache(AllProcessors)
#define KeSweepCurrentIcache()

#define KeSweepIcacheRange(AllProcessors, BaseAddress, Length)

// begin_ntddk begin_wdm begin_nthal begin_ntndis begin_ntosp

#define KeFlushIoBuffers(Mdl, ReadOperation, DmaOperation)

// end_ntddk end_wdm end_ntndis end_ntosp

#define KeYieldProcessor()    __asm { rep nop }

// end_nthal

//
// Define executive macros for acquiring and releasing executive spinlocks.
// These macros can ONLY be used by executive components and NOT by drivers.
// Drivers MUST use the kernel interfaces since they must be MP enabled on
// all systems.
//
// KeRaiseIrql is one instruction longer than KeAcquireSpinLock on x86 UP.
// KeLowerIrql and KeReleaseSpinLock are the same.
//

#if defined(NT_UP) && !DBG && !defined(_NTDDK_) && !defined(_NTIFS_)

#if !defined(_NTDRIVER_)
#define ExAcquireSpinLock(Lock, OldIrql) (*OldIrql) = KeRaiseIrqlToDpcLevel();
#define ExReleaseSpinLock(Lock, OldIrql) KeLowerIrql((OldIrql))
#else
#define ExAcquireSpinLock(Lock, OldIrql) KeAcquireSpinLock((Lock), (OldIrql))
#define ExReleaseSpinLock(Lock, OldIrql) KeReleaseSpinLock((Lock), (OldIrql))
#endif
#define ExAcquireSpinLockAtDpcLevel(Lock)
#define ExReleaseSpinLockFromDpcLevel(Lock)

#else

// begin_wdm begin_ntddk begin_ntosp

#define ExAcquireSpinLock(Lock, OldIrql) KeAcquireSpinLock((Lock), (OldIrql))
#define ExReleaseSpinLock(Lock, OldIrql) KeReleaseSpinLock((Lock), (OldIrql))
#define ExAcquireSpinLockAtDpcLevel(Lock) KeAcquireSpinLockAtDpcLevel(Lock)
#define ExReleaseSpinLockFromDpcLevel(Lock) KeReleaseSpinLockFromDpcLevel(Lock)

// end_wdm end_ntddk end_ntosp

#endif

//
// The acquire and release fast lock macros disable and enable interrupts
// on UP nondebug systems. On MP or debug systems, the spinlock routines
// are used.
//
// N.B. Extreme caution should be observed when using these routines.
//

#if defined(_M_IX86) && !defined(USER_MODE_CODE)

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

void __cdecl _disable (void);
void __cdecl _enable (void);

#pragma warning(disable:4164)
#pragma intrinsic(_disable)
#pragma intrinsic(_enable)
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4164)
#endif

#endif

#if defined(NT_UP) && !DBG && !defined(USER_MODE_CODE)
#define ExAcquireFastLock(Lock, OldIrql) _disable()
#else
#define ExAcquireFastLock(Lock, OldIrql) \
    ExAcquireSpinLock(Lock, OldIrql)
#endif

#if defined(NT_UP) && !DBG && !defined(USER_MODE_CODE)
#define ExReleaseFastLock(Lock, OldIrql) _enable()
#else
#define ExReleaseFastLock(Lock, OldIrql) \
    ExReleaseSpinLock(Lock, OldIrql)
#endif

//
// The following function prototypes must be in this module so that the
// above macros can call them directly.
//
// begin_nthal

VOID
FASTCALL
KiAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

VOID
FASTCALL
KiReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

// end_nthal

//
// Define query tick count macro.
//
// begin_ntddk begin_nthal begin_ntosp

#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_)

// begin_wdm

#define KeQueryTickCount(CurrentCount ) { \
    volatile PKSYSTEM_TIME _TickCount = *((PKSYSTEM_TIME *)(&KeTickCount)); \
    while (TRUE) {                                                          \
        (CurrentCount)->HighPart = _TickCount->High1Time;                   \
        (CurrentCount)->LowPart = _TickCount->LowPart;                      \
        if ((CurrentCount)->HighPart == _TickCount->High2Time) break;       \
        _asm { rep nop }                                                    \
    }                                                                       \
}

// end_wdm

#else

// end_ntddk end_nthal end_ntosp

//
// Define query tick count macro.
//

#define KiQueryTickCount(CurrentCount) \
    while (TRUE) {                                                      \
        (CurrentCount)->HighPart = KeTickCount.High1Time;               \
        (CurrentCount)->LowPart = KeTickCount.LowPart;                  \
        if ((CurrentCount)->HighPart == KeTickCount.High2Time) break;   \
        _asm { rep nop }                                                \
    }

//
// Define query interrupt time macro.
//

#define KiQueryInterruptTime(CurrentTime) \
    while (TRUE) {                                                                      \
        (CurrentTime)->HighPart = SharedUserData->InterruptTime.High1Time;              \
        (CurrentTime)->LowPart = SharedUserData->InterruptTime.LowPart;                 \
        if ((CurrentTime)->HighPart == SharedUserData->InterruptTime.High2Time) break;  \
        _asm { rep nop }                                                                \
    }

// begin_ntddk begin_nthal begin_ntosp

VOID
NTAPI
KeQueryTickCount (
    OUT PLARGE_INTEGER CurrentCount
    );

#endif // defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_)

// end_ntddk end_nthal end_ntosp


// begin_nthal begin_ntosp
//
// 386 hardware structures
//

//
// A Page Table Entry on an Intel 386/486 has the following definition.
//
// **** NOTE A PRIVATE COPY OF THIS EXISTS IN THE MM\I386 DIRECTORY! ****
// ****  ANY CHANGES NEED TO BE MADE TO BOTH HEADER FILES.           ****
//


typedef struct _HARDWARE_PTE_X86 {
    ULONG Valid : 1;
    ULONG Write : 1;
    ULONG Owner : 1;
    ULONG WriteThrough : 1;
    ULONG CacheDisable : 1;
    ULONG Accessed : 1;
    ULONG Dirty : 1;
    ULONG LargePage : 1;
    ULONG Global : 1;
    ULONG CopyOnWrite : 1; // software field
    ULONG Prototype : 1;   // software field
    ULONG reserved : 1;  // software field
    ULONG PageFrameNumber : 20;
} HARDWARE_PTE_X86, *PHARDWARE_PTE_X86;

typedef struct _HARDWARE_PTE_X86PAE {
    union {
        struct {
            ULONGLONG Valid : 1;
            ULONGLONG Write : 1;
            ULONGLONG Owner : 1;
            ULONGLONG WriteThrough : 1;
            ULONGLONG CacheDisable : 1;
            ULONGLONG Accessed : 1;
            ULONGLONG Dirty : 1;
            ULONGLONG LargePage : 1;
            ULONGLONG Global : 1;
            ULONGLONG CopyOnWrite : 1; // software field
            ULONGLONG Prototype : 1;   // software field
            ULONGLONG reserved0 : 1;  // software field
            ULONGLONG PageFrameNumber : 26;
            ULONGLONG reserved1 : 26;  // software field
        };
        struct {
            ULONG LowPart;
            ULONG HighPart;
        };
    };
} HARDWARE_PTE_X86PAE, *PHARDWARE_PTE_X86PAE;

//
// Special check to work around mspdb limitation
//
#if defined (_NTSYM_HARDWARE_PTE_SYMBOL_)
#if !defined (_X86PAE_)
typedef struct _HARDWARE_PTE {
    ULONG Valid : 1;
    ULONG Write : 1;
    ULONG Owner : 1;
    ULONG WriteThrough : 1;
    ULONG CacheDisable : 1;
    ULONG Accessed : 1;
    ULONG Dirty : 1;
    ULONG LargePage : 1;
    ULONG Global : 1;
    ULONG CopyOnWrite : 1; // software field
    ULONG Prototype : 1;   // software field
    ULONG reserved : 1;  // software field
    ULONG PageFrameNumber : 20;
} HARDWARE_PTE, *PHARDWARE_PTE;

#else
typedef struct _HARDWARE_PTE {
    union {
        struct {
            ULONGLONG Valid : 1;
            ULONGLONG Write : 1;
            ULONGLONG Owner : 1;
            ULONGLONG WriteThrough : 1;
            ULONGLONG CacheDisable : 1;
            ULONGLONG Accessed : 1;
            ULONGLONG Dirty : 1;
            ULONGLONG LargePage : 1;
            ULONGLONG Global : 1;
            ULONGLONG CopyOnWrite : 1; // software field
            ULONGLONG Prototype : 1;   // software field
            ULONGLONG reserved0 : 1;  // software field
            ULONGLONG PageFrameNumber : 26;
            ULONGLONG reserved1 : 26;  // software field
        };
        struct {
            ULONG LowPart;
            ULONG HighPart;
        };
    };
} HARDWARE_PTE, *PHARDWARE_PTE;
#endif

#else

#if !defined (_X86PAE_)
typedef HARDWARE_PTE_X86 HARDWARE_PTE;
typedef PHARDWARE_PTE_X86 PHARDWARE_PTE;
#else
typedef HARDWARE_PTE_X86PAE HARDWARE_PTE;
typedef PHARDWARE_PTE_X86PAE PHARDWARE_PTE;
#endif
#endif

//
// GDT Entry
//

typedef struct _KGDTENTRY {
    USHORT  LimitLow;
    USHORT  BaseLow;
    union {
        struct {
            UCHAR   BaseMid;
            UCHAR   Flags1;     // Declare as bytes to avoid alignment
            UCHAR   Flags2;     // Problems.
            UCHAR   BaseHi;
        } Bytes;
        struct {
            ULONG   BaseMid : 8;
            ULONG   Type : 5;
            ULONG   Dpl : 2;
            ULONG   Pres : 1;
            ULONG   LimitHi : 4;
            ULONG   Sys : 1;
            ULONG   Reserved_0 : 1;
            ULONG   Default_Big : 1;
            ULONG   Granularity : 1;
            ULONG   BaseHi : 8;
        } Bits;
    } HighWord;
} KGDTENTRY, *PKGDTENTRY;

#define TYPE_CODE   0x10  // 11010 = Code, Readable, NOT Conforming, Accessed
#define TYPE_DATA   0x12  // 10010 = Data, ReadWrite, NOT Expanddown, Accessed
#define TYPE_TSS    0x01  // 01001 = NonBusy TSS
#define TYPE_LDT    0x02  // 00010 = LDT

#define DPL_USER    3
#define DPL_SYSTEM  0

#define GRAN_BYTE   0
#define GRAN_PAGE   1

#define SELECTOR_TABLE_INDEX 0x04


#define IDT_NMI_VECTOR       2
#define IDT_DFH_VECTOR       8
#define NMI_TSS_DESC_OFFSET  0x58
#define DF_TSS_DESC_OFFSET   0x50


//
// Entry of Interrupt Descriptor Table (IDTENTRY)
//

typedef struct _KIDTENTRY {
   USHORT Offset;
   USHORT Selector;
   USHORT Access;
   USHORT ExtendedOffset;
} KIDTENTRY;

typedef KIDTENTRY *PKIDTENTRY;


//
// TSS (Task switch segment) NT only uses to control stack switches.
//
//  The only fields we actually care about are Esp0, Ss0, the IoMapBase
//  and the IoAccessMaps themselves.
//
//
//  N.B.    Size of TSS must be <= 0xDFFF
//

//
// The interrupt direction bitmap is used on Pentium to allow
// the processor to emulate V86 mode software interrupts for us.
// There is one for each IOPM.  It is located by subtracting
// 32 from the IOPM base in the Tss.
//
#define INT_DIRECTION_MAP_SIZE   32
typedef UCHAR   KINT_DIRECTION_MAP[INT_DIRECTION_MAP_SIZE];

#define IOPM_COUNT      1           // Number of i/o access maps that
                                    // exist (in addition to
                                    // IO_ACCESS_MAP_NONE)

#define IO_ACCESS_MAP_NONE 0

#define IOPM_SIZE           8192    // Size of map callers can set.

#define PIOPM_SIZE          8196    // Size of structure we must allocate
                                    // to hold it.

typedef UCHAR   KIO_ACCESS_MAP[IOPM_SIZE];

typedef KIO_ACCESS_MAP *PKIO_ACCESS_MAP;

typedef struct _KiIoAccessMap {
    KINT_DIRECTION_MAP DirectionMap;
    UCHAR IoMap[PIOPM_SIZE];
} KIIO_ACCESS_MAP;


typedef struct _KTSS {

    USHORT  Backlink;
    USHORT  Reserved0;

    ULONG   Esp0;
    USHORT  Ss0;
    USHORT  Reserved1;

    ULONG   NotUsed1[4];

    ULONG   CR3;
    ULONG   Eip;
    ULONG   EFlags;
    ULONG   Eax;
    ULONG   Ecx;
    ULONG   Edx;
    ULONG   Ebx;
    ULONG   Esp;
    ULONG   Ebp;
    ULONG   Esi;
    ULONG   Edi;


    USHORT  Es;
    USHORT  Reserved2;

    USHORT  Cs;
    USHORT  Reserved3;

    USHORT  Ss;
    USHORT  Reserved4;

    USHORT  Ds;
    USHORT  Reserved5;

    USHORT  Fs;
    USHORT  Reserved6;

    USHORT  Gs;
    USHORT  Reserved7;

    USHORT  LDT;
    USHORT  Reserved8;

    USHORT  Flags;

    USHORT  IoMapBase;

    KIIO_ACCESS_MAP IoMaps[IOPM_COUNT];

    //
    // This is the Software interrupt direction bitmap associated with
    // IO_ACCESS_MAP_NONE
    //
    KINT_DIRECTION_MAP IntDirectionMap;
} KTSS, *PKTSS;


#define KiComputeIopmOffset(MapNumber)          \
    (MapNumber == IO_ACCESS_MAP_NONE) ?         \
        (USHORT)(sizeof(KTSS)) :                    \
        (USHORT)(FIELD_OFFSET(KTSS, IoMaps[MapNumber-1].IoMap))

// begin_windbgkd

//
// Special Registers for i386
//

#ifdef _X86_

typedef struct _DESCRIPTOR {
    USHORT  Pad;
    USHORT  Limit;
    ULONG   Base;
} KDESCRIPTOR, *PKDESCRIPTOR;

typedef struct _KSPECIAL_REGISTERS {
    ULONG Cr0;
    ULONG Cr2;
    ULONG Cr3;
    ULONG Cr4;
    ULONG KernelDr0;
    ULONG KernelDr1;
    ULONG KernelDr2;
    ULONG KernelDr3;
    ULONG KernelDr6;
    ULONG KernelDr7;
    KDESCRIPTOR Gdtr;
    KDESCRIPTOR Idtr;
    USHORT Tr;
    USHORT Ldtr;
    ULONG Reserved[6];
} KSPECIAL_REGISTERS, *PKSPECIAL_REGISTERS;

//
// Processor State frame: Before a processor freezes itself, it
// dumps the processor state to the processor state frame for
// debugger to examine.
//

typedef struct _KPROCESSOR_STATE {
    struct _CONTEXT ContextFrame;
    struct _KSPECIAL_REGISTERS SpecialRegisters;
} KPROCESSOR_STATE, *PKPROCESSOR_STATE;

#endif // _X86_

// end_windbgkd

//
// DPC data structure definition.
//

typedef struct _KDPC_DATA {
    LIST_ENTRY DpcListHead;
    KSPIN_LOCK DpcLock;
    volatile ULONG DpcQueueDepth;
    ULONG DpcCount;
} KDPC_DATA, *PKDPC_DATA;

//
// Processor Control Block (PRCB)
//

#define PRCB_MAJOR_VERSION 1
#define PRCB_MINOR_VERSION 1
#define PRCB_BUILD_DEBUG        0x0001
#define PRCB_BUILD_UNIPROCESSOR 0x0002

typedef struct _KPRCB {

//
// Start of the architecturally defined section of the PRCB. This section
// may be directly addressed by vendor/platform specific HAL code and will
// not change from version to version of NT.
//
    USHORT MinorVersion;
    USHORT MajorVersion;

    struct _KTHREAD *CurrentThread;
    struct _KTHREAD *NextThread;
    struct _KTHREAD *IdleThread;

    CCHAR  Number;
    CCHAR  Reserved;
    USHORT BuildType;
    KAFFINITY SetMember;

    CCHAR   CpuType;
    CCHAR   CpuID;
    USHORT  CpuStep;

    struct _KPROCESSOR_STATE ProcessorState;

    ULONG   KernelReserved[16];         // For use by the kernel
    ULONG   HalReserved[16];            // For use by Hal

//
// Per processor lock queue entries.
//
// N.B. The following padding is such that the first lock entry falls in the
//      last eight bytes of a cache line. This makes the dispatcher lock and
//      the context swap lock lie in separate cache lines.
//

    UCHAR PrcbPad0[28 + 64];
    KSPIN_LOCK_QUEUE LockQueue[LockQueueMaximumLock];

// End of the architecturally defined section of the PRCB.
// end_nthal end_ntosp

//
// Miscellaneous counters - 64-byte aligned.
//

    struct _KTHREAD *NpxThread;
    ULONG   InterruptCount;
    ULONG   KernelTime;
    ULONG   UserTime;
    ULONG   DpcTime;
    ULONG   DebugDpcTime;
    ULONG   InterruptTime;
    ULONG   AdjustDpcThreshold;
    ULONG   PageColor;
    BOOLEAN SkipTick;
    KIRQL   DebuggerSavedIRQL;
    UCHAR   NodeColor;
    UCHAR   Spare1;
    ULONG   NodeShiftedColor;
    struct _KNODE *ParentNode;
    KAFFINITY MultiThreadProcessorSet;
    struct _KPRCB * MultiThreadSetMaster;
    ULONG   SecondaryColorMask;
    LONG    Sleeping;

//
// Performance counters - 64-byte aligned.
//
// Cache manager performance counters.
//

    ULONG CcFastReadNoWait;
    ULONG CcFastReadWait;
    ULONG CcFastReadNotPossible;
    ULONG CcCopyReadNoWait;
    ULONG CcCopyReadWait;
    ULONG CcCopyReadNoWaitMiss;

//
//  Kernel performance counters.
//

    ULONG KeAlignmentFixupCount;
    ULONG SpareCounter0;
    ULONG KeDcacheFlushCount;
    ULONG KeExceptionDispatchCount;
    ULONG KeFirstLevelTbFills;
    ULONG KeFloatingEmulationCount;
    ULONG KeIcacheFlushCount;
    ULONG KeSecondLevelTbFills;
    ULONG KeSystemCalls;

//
// I/O system counters.
//

    volatile LONG IoReadOperationCount;
    volatile LONG IoWriteOperationCount;
    volatile LONG IoOtherOperationCount;
    LARGE_INTEGER IoReadTransferCount;
    LARGE_INTEGER IoWriteTransferCount;
    LARGE_INTEGER IoOtherTransferCount;
    ULONG SpareCounter1[8];

//
// Nonpaged per processor lookaside lists - 64-byte aligned.
//

    PP_LOOKASIDE_LIST PPLookasideList[16];

//
// Nonpaged per processor small pool lookaside lists - 64-byte aligned.
//

    PP_LOOKASIDE_LIST PPNPagedLookasideList[POOL_SMALL_LISTS];

//
// Paged per processor small pool lookaside lists - 64-byte aligned.
//

    PP_LOOKASIDE_LIST PPPagedLookasideList[POOL_SMALL_LISTS];

//
// MP interprocessor request packet barrier - 64-byte aligned.
//

    volatile KAFFINITY PacketBarrier;
    volatile ULONG ReverseStall;
    PVOID IpiFrame;
    UCHAR PrcbPad2[52];

//
// MP interprocessor request packet and summary - 64-byte aligned.
//

    volatile PVOID CurrentPacket[3];
    volatile KAFFINITY TargetSet;
    volatile PKIPI_WORKER WorkerRoutine;
    volatile ULONG IpiFrozen;
    UCHAR PrcbPad3[40];

//
// MP interprocessor request summary and packet address - 64-byte aligned.
//

    volatile ULONG RequestSummary;
    volatile struct _KPRCB *SignalDone;
    UCHAR PrcbPad4[56];

//
// DPC listhead, counts, and batching parameters - 64-byte aligned.
//

    KDPC_DATA DpcData[2];
    PVOID DpcStack;
    ULONG MaximumDpcQueueDepth;
    ULONG DpcRequestRate;
    ULONG MinimumDpcRate;
    volatile BOOLEAN DpcInterruptRequested;
    volatile BOOLEAN DpcThreadRequested;

//
// N.B. the following two fields must be on a word boundary.
//

    volatile BOOLEAN DpcRoutineActive;
    volatile BOOLEAN DpcThreadActive;
    KSPIN_LOCK PrcbLock;
    ULONG DpcLastCount;
    volatile ULONG TimerHand;
    volatile ULONG TimerRequest;
    PVOID DpcThread;
    KEVENT DpcEvent;
    BOOLEAN ThreadDpcEnable;
    volatile BOOLEAN QuantumEnd;
    UCHAR PrcbPad50;
    volatile BOOLEAN IdleSchedule;
    LONG DpcSetEventRequest;
    UCHAR PrcbPad5[18];

//
// Number of 100ns units remaining before a tick completes on this processor.
//

    LONG TickOffset;

//
// Generic call DPC - 64-byte aligned.
//

    KDPC CallDpc;
    ULONG PrcbPad7[8];

//
// Per-processor ready summary and ready queues - 64-byte aligned.
//
// N.B. Ready summary is in the first cache line as the queue for priority
//      zero is never used.
//

    LIST_ENTRY WaitListHead;
    ULONG ReadySummary;
    ULONG QueueIndex;
    LIST_ENTRY DispatcherReadyListHead[MAXIMUM_PRIORITY];
    SINGLE_LIST_ENTRY DeferredReadyListHead;
    ULONG PrcbPad72[11];

//
// Per processor chained interrupt list - 64-byte aligned.
//

    PVOID ChainedInterruptList;

//
// I/O IRP float.
//

    LONG LookasideIrpFloat;

//
// Memory management counters.
//

    volatile LONG MmPageFaultCount;
    volatile LONG MmCopyOnWriteCount;
    volatile LONG MmTransitionCount;
    volatile LONG MmCacheTransitionCount;
    volatile LONG MmDemandZeroCount;
    volatile LONG MmPageReadCount;
    volatile LONG MmPageReadIoCount;
    volatile LONG MmCacheReadCount;
    volatile LONG MmCacheIoCount;
    volatile LONG MmDirtyPagesWriteCount;
    volatile LONG MmDirtyWriteIoCount;
    volatile LONG MmMappedPagesWriteCount;
    volatile LONG MmMappedWriteIoCount;
    
//
// Spare fields.
//

    ULONG   SpareFields0[1];

//
// Processor information.
//

    UCHAR VendorString[13];
    UCHAR InitialApicId;
    UCHAR LogicalProcessorsPerPhysicalProcessor;
    ULONG MHz;
    ULONG FeatureBits;
    LARGE_INTEGER UpdateSignature;

//
// ISR timing data.
//

    volatile ULONGLONG IsrTime;
    ULONGLONG SpareField1;

//
// Npx save area - 16-byte aligned.
//

    FX_SAVE_AREA NpxSaveArea;

//
// Processors power state
//

    PROCESSOR_POWER_STATE PowerState;

// begin_nthal begin_ntosp

} KPRCB, *PKPRCB, *RESTRICTED_POINTER PRKPRCB;

// end_nthal end_ntosp

//
// The offset of the PRCB in the PCR is 32 mod 64.
//
// The offset of the following structure must be 0 mod 64 except for the
// lock queue array which straddles two cache lines.
//

C_ASSERT(((FIELD_OFFSET(KPRCB, LockQueue) + sizeof(KSPIN_LOCK_QUEUE) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, NpxThread) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, CcFastReadNoWait) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, PPLookasideList) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, PPNPagedLookasideList) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, PPPagedLookasideList) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, PacketBarrier) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, CurrentPacket) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, DpcData) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, DpcRoutineActive)) & (1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, CallDpc) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, WaitListHead) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, ChainedInterruptList) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, NpxSaveArea) + 32) & (16 - 1)) == 0);

// begin_nthal begin_ntddk begin_ntosp

//
// Processor Control Region Structure Definition
//

#define PCR_MINOR_VERSION 1
#define PCR_MAJOR_VERSION 1

typedef struct _KPCR {

//
// Start of the architecturally defined section of the PCR. This section
// may be directly addressed by vendor/platform specific HAL code and will
// not change from version to version of NT.
//
// Certain fields in the TIB are not used in kernel mode. These include the
// stack limit, subsystem TIB, fiber data, arbitrary user pointer, and the
// self address of then PCR itself (another field has been added for that
// purpose). Therefore, these fields are overlaid with other data to get
// better cache locality.
//

    union {
        NT_TIB  NtTib;
        struct {
            struct _EXCEPTION_REGISTRATION_RECORD *Used_ExceptionList;
            PVOID Used_StackBase;
            PVOID PerfGlobalGroupMask;
            PVOID TssCopy;
            ULONG ContextSwitches;
            KAFFINITY SetMemberCopy;
            PVOID Used_Self;
        };
    };

    struct _KPCR *SelfPcr;              // flat address of this PCR
    struct _KPRCB *Prcb;                // pointer to Prcb
    KIRQL   Irql;                       // do not use 3 bytes after this as
                                        // HALs assume they are zero.
    ULONG   IRR;
    ULONG   IrrActive;
    ULONG   IDR;
    PVOID   KdVersionBlock;

    struct _KIDTENTRY *IDT;
    struct _KGDTENTRY *GDT;
    struct _KTSS      *TSS;
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    KAFFINITY SetMember;
    ULONG   StallScaleFactor;
    UCHAR   SpareUnused;
    UCHAR   Number;

// end_ntddk end_ntosp

    UCHAR   Spare0;
    UCHAR   SecondLevelCacheAssociativity;
    ULONG   VdmAlert;
    ULONG   KernelReserved[14];         // For use by the kernel
    ULONG   SecondLevelCacheSize;
    ULONG   HalReserved[16];            // For use by Hal

// End of the architecturally defined section of the PCR.
// end_nthal

    ULONG   InterruptMode;
    UCHAR   Spare1;
    ULONG   KernelReserved2[17];
    struct _KPRCB PrcbData;

// begin_nthal begin_ntddk begin_ntosp

} KPCR, *PKPCR;

// end_nthal end_ntddk end_ntosp

C_ASSERT(FIELD_OFFSET(KPCR, NtTib.ExceptionList) == FIELD_OFFSET(KPCR, Used_ExceptionList));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.StackBase) == FIELD_OFFSET(KPCR, Used_StackBase));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.StackLimit) == FIELD_OFFSET(KPCR, PerfGlobalGroupMask));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.SubSystemTib) == FIELD_OFFSET(KPCR, TssCopy));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.FiberData) == FIELD_OFFSET(KPCR, ContextSwitches));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.ArbitraryUserPointer) == FIELD_OFFSET(KPCR, SetMemberCopy));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.Self) == FIELD_OFFSET(KPCR, Used_Self));
C_ASSERT((FIELD_OFFSET(KPCR, PrcbData) & (64 - 1)) == 32);

FORCEINLINE
ULONG
KeGetContextSwitches (
    PKPRCB Prcb
    )

{

    PKPCR Pcr;

    Pcr = CONTAINING_RECORD(Prcb, KPCR, PrcbData);
    return Pcr->ContextSwitches;
}

BOOLEAN
KeDisableInterrupts (
    VOID
    );

// begin_nthal begin_ntosp

//
// bits defined in Eflags
//

#define EFLAGS_CF_MASK        0x00000001L
#define EFLAGS_PF_MASK        0x00000004L
#define EFLAGS_AF_MASK        0x00000010L
#define EFLAGS_ZF_MASK        0x00000040L
#define EFLAGS_SF_MASK        0x00000080L
#define EFLAGS_TF             0x00000100L
#define EFLAGS_INTERRUPT_MASK 0x00000200L
#define EFLAGS_DF_MASK        0x00000400L
#define EFLAGS_OF_MASK        0x00000800L
#define EFLAGS_IOPL_MASK      0x00003000L
#define EFLAGS_NT             0x00004000L
#define EFLAGS_RF             0x00010000L
#define EFLAGS_V86_MASK       0x00020000L
#define EFLAGS_ALIGN_CHECK    0x00040000L
#define EFLAGS_VIF            0x00080000L
#define EFLAGS_VIP            0x00100000L
#define EFLAGS_ID_MASK        0x00200000L

#define EFLAGS_USER_SANITIZE  0x003f4dd7L

// end_nthal

//
// Sanitize segCS and eFlags based on a processor mode.
//
// If kernel mode,
//      force CPL == 0
//
// If user mode,
//      force CPL == 3
//

#define SANITIZE_SEG(segCS, mode) (\
    ((mode) == KernelMode ? \
        ((0x00000000L) | ((segCS) & 0xfffc)) : \
        ((0x00000003L) | ((segCS) & 0xffff))))

//
// If kernel mode, then
//      let caller specify Carry, Parity, AuxCarry, Zero, Sign, Trap,
//      Direction, Overflow, Interrupt, AlignCheck.
//
// If user mode, then
//      let caller specify Carry, Parity, AuxCarry, Zero, Sign, Trap,
//      Direction, Overflow, AlignCheck.
//      force Interrupts on.
//


#define SANITIZE_FLAGS(eFlags, mode) (\
    ((mode) == KernelMode ? \
        ((0x00000000L) | ((eFlags) & 0x003f0fd7)) : \
        ((EFLAGS_INTERRUPT_MASK) | ((eFlags) & EFLAGS_USER_SANITIZE))))

//
// Masks for Dr7 and sanitize macros for various Dr registers.
//

#define DR6_LEGAL   0x0000e00f

#define DR7_LEGAL   0xffff0155  // R/W, LEN for Dr0-Dr4,
                                // Local enable for Dr0-Dr4,
                                // Le for "perfect" trapping

//
// Bits to used track the state of the various debug registers
//

#define DR7_OVERRIDE_V 0x04

#define DR_MASK(Bit) (((UCHAR)(1UL << (Bit))))


#define DR_REG_MASK (DR_MASK(0) | DR_MASK(1) | DR_MASK(2) | DR_MASK(3) | DR_MASK(6))
#define DR_VALID_MASK (DR_REG_MASK | DR_MASK (7) | DR_MASK (DR7_OVERRIDE_V))

#define DR7_MASK_SHIFT 16   // Shift to translate the valid mask to a spare region in Dr7
                                             // The region occupied is the LEN & R/W region for Dr0
                                             
#define DR7_OVERRIDE_MASK ((0x0FUL) << DR7_MASK_SHIFT)  // This corresponds to a break on R/W of 4
                                                                                                // bytes from the addres indicated by DR0
#define DR7_RESERVED_MASK 0x0000DC00    // Bits 10-12, 14-15 are reserved
#define DR7_ACTIVE  0x00000055  // If any of these bits are set, a Dr is active

C_ASSERT (sizeof(BOOLEAN) == sizeof(UCHAR));
C_ASSERT ((((ULONG)DR_VALID_MASK) & ~((ULONG)((UCHAR)0xFF))) == 0);
C_ASSERT ((DR7_ACTIVE & DR7_OVERRIDE_MASK) == 0);
C_ASSERT ((DR7_RESERVED_MASK & DR7_OVERRIDE_MASK) == 0);
C_ASSERT ((DR7_OVERRIDE_MASK & DR7_LEGAL) == DR7_OVERRIDE_MASK);
C_ASSERT ((DR7_RESERVED_MASK & DR7_LEGAL) == 0);

#define SANITIZE_DR6(Dr6, mode) (((Dr6) & DR6_LEGAL))

#define SANITIZE_DR7(Dr7, mode) (((Dr7) & DR7_LEGAL))

#define SANITIZE_DRADDR(DrReg, mode) (          \
    (mode) == KernelMode ?                      \
        (DrReg) :                               \
        (((PVOID)DrReg <= MM_HIGHEST_USER_ADDRESS) ?   \
            (DrReg) :                           \
            (0)                                 \
        )                                       \
    )

//
// Define macro to clear reserved bits from MXCSR so that we don't
// GP fault when doing an FRSTOR
//

extern ULONG KiMXCsrMask;

#define SANITIZE_MXCSR(_mxcsr_) ((_mxcsr_) & KiMXCsrMask)

//
// Nonvolatile context pointers
//

typedef struct _KNONVOLATILE_CONTEXT_POINTERS {
    ULONG   Junk;
} KNONVOLATILE_CONTEXT_POINTERS,  *PKNONVOLATILE_CONTEXT_POINTERS;

// begin_nthal
//
// Trap frame
//
//  NOTE - We deal only with 32bit registers, so the assembler equivalents
//         are always the extended forms.
//
//  NOTE - Unless you want to run like slow molasses everywhere in the
//         the system, this structure must be of DWORD length, DWORD
//         aligned, and its elements must all be DWORD aligned.
//
//  NOTE WELL   -
//
//      The i386 does not build stack frames in a consistent format, the
//      frames vary depending on whether or not a privilege transition
//      was involved.
//
//      In order to make NtContinue work for both user mode and kernel
//      mode callers, we must force a canonical stack.
//
//      If we're called from kernel mode, this structure is 8 bytes longer
//      than the actual frame!
//
//  WARNING:
//
//      KTRAP_FRAME_LENGTH needs to be 16byte integral (at present.)
//

typedef struct _KTRAP_FRAME {


//
//  Following 4 values are only used and defined for DBG systems,
//  but are always allocated to make switching from DBG to non-DBG
//  and back quicker.  They are not DEVL because they have a non-0
//  performance impact.
//

    ULONG   DbgEbp;         // Copy of User EBP set up so KB will work.
    ULONG   DbgEip;         // EIP of caller to system call, again, for KB.
    ULONG   DbgArgMark;     // Marker to show no args here.
    ULONG   DbgArgPointer;  // Pointer to the actual args

//
//  Temporary values used when frames are edited.
//
//
//  NOTE:   Any code that want's ESP must materialize it, since it
//          is not stored in the frame for kernel mode callers.
//
//          And code that sets ESP in a KERNEL mode frame, must put
//          the new value in TempEsp, make sure that TempSegCs holds
//          the real SegCs value, and put a special marker value into SegCs.
//

    ULONG   TempSegCs;
    ULONG   TempEsp;

//
//  Debug registers.
//

    ULONG   Dr0;
    ULONG   Dr1;
    ULONG   Dr2;
    ULONG   Dr3;
    ULONG   Dr6;
    ULONG   Dr7;

//
//  Segment registers
//

    ULONG   SegGs;
    ULONG   SegEs;
    ULONG   SegDs;

//
//  Volatile registers
//

    ULONG   Edx;
    ULONG   Ecx;
    ULONG   Eax;

//
//  Nesting state, not part of context record
//

    ULONG   PreviousPreviousMode;

    PEXCEPTION_REGISTRATION_RECORD ExceptionList;
                                            // Trash if caller was user mode.
                                            // Saved exception list if caller
                                            // was kernel mode or we're in
                                            // an interrupt.

//
//  FS is TIB/PCR pointer, is here to make save sequence easy
//

    ULONG   SegFs;

//
//  Non-volatile registers
//

    ULONG   Edi;
    ULONG   Esi;
    ULONG   Ebx;
    ULONG   Ebp;

//
//  Control registers
//

    ULONG   ErrCode;
    ULONG   Eip;
    ULONG   SegCs;
    ULONG   EFlags;

    ULONG   HardwareEsp;    // WARNING - segSS:esp are only here for stacks
    ULONG   HardwareSegSs;  // that involve a ring transition.

    ULONG   V86Es;          // these will be present for all transitions from
    ULONG   V86Ds;          // V86 mode
    ULONG   V86Fs;
    ULONG   V86Gs;
} KTRAP_FRAME;


typedef KTRAP_FRAME *PKTRAP_FRAME;
typedef KTRAP_FRAME *PKEXCEPTION_FRAME;

#define KTRAP_FRAME_LENGTH  (sizeof(KTRAP_FRAME))
#define KTRAP_FRAME_ALIGN   (sizeof(ULONG))
#define KTRAP_FRAME_ROUND   (KTRAP_FRAME_ALIGN-1)

//
//  Bits forced to 0 in SegCs if Esp has been edited.
//

#define FRAME_EDITED        0xfff8

// end_nthal

//
// The frame saved by KiCallUserMode is defined here to allow
// the kernel debugger to trace the entire kernel stack
// when usermode callouts are pending.
//

typedef struct _KCALLOUT_FRAME {
    ULONG   InStk;          // saved initial stack address
    ULONG   TrFr;           // saved callback trap frame
    ULONG   CbStk;          // saved callback stack address
    ULONG   Edi;            // saved nonvolatile registers
    ULONG   Esi;            //
    ULONG   Ebx;            //
    ULONG   Ebp;            //
    ULONG   Ret;            // saved return address
    ULONG   OutBf;          // address to store output buffer
    ULONG   OutLn;          // address to store output length
} KCALLOUT_FRAME;

typedef KCALLOUT_FRAME *PKCALLOUT_FRAME;


//
//  Switch Frame
//
//  386 doesn't have an "exception frame", and doesn't normally make
//  any use of nonvolatile context register structures.
//
//  However, swapcontext in ctxswap.c and KeInitializeThread in
//  thredini.c need to share common stack structure used at thread
//  startup and switch time.
//
//  This is that structure.
//

typedef struct _KSWITCHFRAME {
    ULONG   ExceptionList;
    ULONG   ApcBypassDisable;
    ULONG   RetAddr;
} KSWITCHFRAME, *PKSWITCHFRAME;


//
// Various 387 defines
//

#define I386_80387_NP_VECTOR    0x07    // trap 7 when hardware not present

// begin_ntddk begin_wdm
//
// The non-volatile 387 state
//

typedef struct _KFLOATING_SAVE {
    ULONG   ControlWord;
    ULONG   StatusWord;
    ULONG   ErrorOffset;
    ULONG   ErrorSelector;
    ULONG   DataOffset;                 // Not used in wdm
    ULONG   DataSelector;
    ULONG   Cr0NpxState;
    ULONG   Spare1;                     // Not used in wdm
} KFLOATING_SAVE, *PKFLOATING_SAVE;

// end_ntddk end_wdm end_ntosp

//
// i386 Profile values
//

#define DEFAULT_PROFILE_INTERVAL   39063

//
// The minimum acceptable profiling interval is set to 1221 which is the
// fast RTC clock rate we can get.  If this
// value is too small, the system will run very slowly.
//

#define MINIMUM_PROFILE_INTERVAL   1221


// begin_ntddk begin_wdm begin_nthal begin_ntndis begin_ntosp
//
// i386 Specific portions of mm component
//

//
// Define the page size for the Intel 386 as 4096 (0x1000).
//

#define PAGE_SIZE 0x1000

//
// Define the number of trailing zeroes in a page aligned virtual address.
// This is used as the shift count when shifting virtual addresses to
// virtual page numbers.
//

#define PAGE_SHIFT 12L

// end_ntndis end_wdm
//
// Define the number of bits to shift to right justify the Page Directory Index
// field of a PTE.
//

#define PDI_SHIFT_X86    22
#define PDI_SHIFT_X86PAE 21

#if !defined (_X86PAE_)
#define PDI_SHIFT PDI_SHIFT_X86
#else
#define PDI_SHIFT PDI_SHIFT_X86PAE
#define PPI_SHIFT 30
#endif

#define GUARD_PAGE_SIZE   PAGE_SIZE

//
// Define the number of bits to shift to right justify the Page Table Index
// field of a PTE.
//

#define PTI_SHIFT 12

//
// Define the highest user address and user probe address.
//

extern NTKERNELAPI PVOID MmHighestUserAddress;
extern NTKERNELAPI PVOID MmSystemRangeStart;
extern NTKERNELAPI ULONG MmUserProbeAddress;

#define MM_HIGHEST_USER_ADDRESS MmHighestUserAddress
#define MM_SYSTEM_RANGE_START MmSystemRangeStart

#if defined(_LOCAL_COPY_USER_PROBE_ADDRESS_)

#define MM_USER_PROBE_ADDRESS _LOCAL_COPY_USER_PROBE_ADDRESS_

extern ULONG _LOCAL_COPY_USER_PROBE_ADDRESS_;

#else

#define MM_USER_PROBE_ADDRESS MmUserProbeAddress

#endif

//
// The lowest user address reserves the low 64k.
//

#define MM_LOWEST_USER_ADDRESS (PVOID)0x10000

//
// The lowest address for system space.
//

#if !defined (_X86PAE_)
#define MM_LOWEST_SYSTEM_ADDRESS (PVOID)0xC0800000
#else
#define MM_LOWEST_SYSTEM_ADDRESS (PVOID)0xC0C00000
#endif

// begin_wdm

#define MmGetProcedureAddress(Address) (Address)
#define MmLockPageableCodeSection(Address) MmLockPageableDataSection(Address)
#define MmLockPagableCodeSection(Address) MmLockPageableDataSection(Address)
#define MmLockPagableDataSection(Address) MmLockPageableDataSection(Address)

// end_ntddk end_wdm

//
// Define the number of bits to shift to right justify the Page Directory Index
// field of a PTE.
//

#define PDI_SHIFT_X86    22
#define PDI_SHIFT_X86PAE 21

#if !defined (_X86PAE_)
#define PDI_SHIFT PDI_SHIFT_X86
#else
#define PDI_SHIFT PDI_SHIFT_X86PAE
#define PPI_SHIFT 30
#endif

//
// Define the number of bits to shift to right justify the Page Table Index
// field of a PTE.
//

#define PTI_SHIFT 12

//
// Define page directory and page base addresses.
//

#define PDE_BASE_X86    0xc0300000
#define PDE_BASE_X86PAE 0xc0600000

#define PTE_TOP_X86     0xC03FFFFF
#define PDE_TOP_X86     0xC0300FFF

#define PTE_TOP_X86PAE  0xC07FFFFF
#define PDE_TOP_X86PAE  0xC0603FFF


#if !defined (_X86PAE_)
#define PDE_BASE PDE_BASE_X86
#define PTE_TOP  PTE_TOP_X86
#define PDE_TOP  PDE_TOP_X86
#else
#define PDE_BASE PDE_BASE_X86PAE
#define PTE_TOP  PTE_TOP_X86PAE
#define PDE_TOP  PDE_TOP_X86PAE
#endif
#define PTE_BASE 0xc0000000

// end_nthal end_ntosp

//
// Define virtual base and alternate virtual base of kernel.
//

#define KSEG0_BASE 0x80000000
#define ALTERNATE_BASE (0xe1000000 - 16 * 1024 * 1024)

//
// Define macro to initialize directory table base.
//

#define INITIALIZE_DIRECTORY_TABLE_BASE(dirbase,pfn) \
     *((PULONG)(dirbase)) = ((pfn) << PAGE_SHIFT)


// begin_nthal
//
// Location of primary PCR (used only for UP kernel & hal code)
//

// addressed from 0xffdf0000 - 0xffdfffff are reserved for the system
// (ie, not for use by the hal)

#define KI_BEGIN_KERNEL_RESERVED    0xffdf0000
#define KIP0PCRADDRESS              0xffdff000  // ntddk wdm ntosp

// begin_ntddk begin_ntosp

#define KI_USER_SHARED_DATA         0xffdf0000
#define SharedUserData  ((KUSER_SHARED_DATA * const) KI_USER_SHARED_DATA)

//
// Result type definition for i386.  (Machine specific enumerate type
// which is return type for portable exinterlockedincrement/decrement
// procedures.)  In general, you should use the enumerated type defined
// in ex.h instead of directly referencing these constants.
//

// Flags loaded into AH by LAHF instruction

#define EFLAG_SIGN      0x8000
#define EFLAG_ZERO      0x4000
#define EFLAG_SELECT    (EFLAG_SIGN | EFLAG_ZERO)

#define RESULT_NEGATIVE ((EFLAG_SIGN & ~EFLAG_ZERO) & EFLAG_SELECT)
#define RESULT_ZERO     ((~EFLAG_SIGN & EFLAG_ZERO) & EFLAG_SELECT)
#define RESULT_POSITIVE ((~EFLAG_SIGN & ~EFLAG_ZERO) & EFLAG_SELECT)

//
// Convert various portable ExInterlock APIs into their architectural
// equivalents.
//

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInterlockedIncrementLong)      // Use InterlockedIncrement
#pragma deprecated(ExInterlockedDecrementLong)      // Use InterlockedDecrement
#pragma deprecated(ExInterlockedExchangeUlong)      // Use InterlockedExchange
#endif

#define ExInterlockedIncrementLong(Addend,Lock) \
        Exfi386InterlockedIncrementLong(Addend)

#define ExInterlockedDecrementLong(Addend,Lock) \
        Exfi386InterlockedDecrementLong(Addend)

#define ExInterlockedExchangeUlong(Target,Value,Lock) \
        Exfi386InterlockedExchangeUlong(Target,Value)

// begin_wdm

#define ExInterlockedAddUlong           ExfInterlockedAddUlong
#define ExInterlockedInsertHeadList     ExfInterlockedInsertHeadList
#define ExInterlockedInsertTailList     ExfInterlockedInsertTailList
#define ExInterlockedRemoveHeadList     ExfInterlockedRemoveHeadList
#define ExInterlockedPopEntryList       ExfInterlockedPopEntryList
#define ExInterlockedPushEntryList      ExfInterlockedPushEntryList

// end_wdm

//
// Prototypes for architectural specific versions of Exi386 Api
//

//
// Interlocked result type is portable, but its values are machine specific.
// Constants for value are in i386.h, mips.h, etc.
//

typedef enum _INTERLOCKED_RESULT {
    ResultNegative = RESULT_NEGATIVE,
    ResultZero     = RESULT_ZERO,
    ResultPositive = RESULT_POSITIVE
} INTERLOCKED_RESULT;

NTKERNELAPI
INTERLOCKED_RESULT
FASTCALL
Exfi386InterlockedIncrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
INTERLOCKED_RESULT
FASTCALL
Exfi386InterlockedDecrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
ULONG
FASTCALL
Exfi386InterlockedExchangeUlong (
    IN PULONG Target,
    IN ULONG Value
    );

// end_ntddk end_nthal end_ntosp

//
// UP/MP versions of interlocked intrinsics
//
// N.B. FASTCALL does NOT work with inline functions.
//

#if !defined(_WINBASE_) && !defined(NONTOSPINTERLOCK) // ntosp ntddk nthal
#if defined(_M_IX86)

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4035)               // re-enable below

// begin_ntddk begin_nthal begin_ntosp
#if !defined(MIDL_PASS) // wdm
#if defined(NO_INTERLOCKED_INTRINSICS) || defined(_CROSS_PLATFORM_)
// begin_wdm

NTKERNELAPI
LONG
FASTCALL
InterlockedIncrement(
    IN LONG volatile *Addend
    );

NTKERNELAPI
LONG
FASTCALL
InterlockedDecrement(
    IN LONG volatile *Addend
    );

NTKERNELAPI
LONG
FASTCALL
InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value
    );

#define InterlockedExchangePointer(Target, Value) \
   (PVOID)InterlockedExchange((PLONG)(Target), (LONG)(Value))

LONG
FASTCALL
InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Increment
    );

NTKERNELAPI
LONG
FASTCALL
InterlockedCompareExchange(
    IN OUT LONG volatile *Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );

#define InterlockedCompareExchangePointer(Destination, ExChange, Comperand) \
    (PVOID)InterlockedCompareExchange((PLONG)Destination, (LONG)ExChange, (LONG)Comperand)

#define InterlockedCompareExchange64(Destination, ExChange, Comperand) \
    ExfInterlockedCompareExchange64(Destination, &(ExChange), &(Comperand))


NTKERNELAPI
LONGLONG
FASTCALL
ExfInterlockedCompareExchange64(
    IN OUT LONGLONG volatile *Destination,
    IN PLONGLONG ExChange,
    IN PLONGLONG Comperand
    );

// end_wdm

#else       // NO_INTERLOCKED_INTRINSICS || _CROSS_PLATFORM_

#define InterlockedExchangePointer(Target, Value) \
   (PVOID)InterlockedExchange((PLONG)Target, (LONG)Value)

// end_ntddk end_nthal end_ntosp

#if defined(NT_UP) && !defined (_NTDDK_) && !defined(_NTIFS_)

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedIncrement(
    IN LONG volatile *Addend
    );

#pragma intrinsic (_InterlockedIncrement)
#define InterlockedIncrement _InterlockedIncrement
#else
#define InterlockedIncrement(Addend) (InterlockedExchangeAdd (Addend, 1)+1)
#endif

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedDecrement(
    IN LONG volatile *Addend
    );

#pragma intrinsic (_InterlockedDecrement)
#define InterlockedDecrement _InterlockedDecrement
#else
#define InterlockedDecrement(Addend) (InterlockedExchangeAdd (Addend, -1)-1)
#endif

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value
    );

#pragma intrinsic (_InterlockedExchange)
#define InterlockedExchange _InterlockedExchange
#else
FORCEINLINE
LONG
FASTCALL
InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value
    )
{
    __asm {
        mov     eax, Value
        mov     ecx, Target
        xchg    [ecx], eax
    }
}
#endif

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Increment
    );

#pragma intrinsic (_InterlockedExchangeAdd)
#define InterlockedExchangeAdd _InterlockedExchangeAdd
#else
FORCEINLINE
LONG
FASTCALL
InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Increment
    )
{
    __asm {
        mov     eax, Increment
        mov     ecx, Addend
        xadd    [ecx], eax
    }
}
#endif

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedCompareExchange (
    IN OUT LONG volatile *Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );

#pragma intrinsic (_InterlockedCompareExchange)
#define InterlockedCompareExchange (LONG)_InterlockedCompareExchange
#else

FORCEINLINE
LONG
FASTCALL
InterlockedCompareExchange(
    IN OUT LONG volatile *Destination,
    IN LONG Exchange,
    IN LONG Comperand
    )
{
    __asm {
        mov     eax, Comperand
        mov     ecx, Destination
        mov     edx, Exchange
        cmpxchg [ecx], edx
    }
}

#endif

#define InterlockedCompareExchangePointer(Destination, ExChange, Comperand) \
    (PVOID)InterlockedCompareExchange((PLONG)Destination, (LONG)ExChange, (LONG)Comperand)

#define InterlockedCompareExchange64(Destination, ExChange, Comperand) \
    ExfInterlockedCompareExchange64(Destination, &(ExChange), &(Comperand))


NTKERNELAPI
LONGLONG
FASTCALL
ExfInterlockedCompareExchange64(
    IN OUT LONGLONG volatile *Destination,
    IN PLONGLONG ExChange,
    IN PLONGLONG Comperand
    );


#else   // NT_UP

// begin_ntosp begin_ntddk begin_nthal

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value
    );

#pragma intrinsic (_InterlockedExchange)
#define InterlockedExchange _InterlockedExchange
#else
FORCEINLINE
LONG
FASTCALL
InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value
    )
{
    __asm {
        mov     eax, Value
        mov     ecx, Target
        xchg    [ecx], eax
    }
}
#endif

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedIncrement(
    IN LONG volatile *Addend
    );

#pragma intrinsic (_InterlockedIncrement)
#define InterlockedIncrement _InterlockedIncrement
#else
#define InterlockedIncrement(Addend) (InterlockedExchangeAdd (Addend, 1)+1)
#endif

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedDecrement(
    IN LONG volatile *Addend
    );

#pragma intrinsic (_InterlockedDecrement)
#define InterlockedDecrement _InterlockedDecrement
#else
#define InterlockedDecrement(Addend) (InterlockedExchangeAdd (Addend, -1)-1)
#endif

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Increment
    );

#pragma intrinsic (_InterlockedExchangeAdd)
#define InterlockedExchangeAdd _InterlockedExchangeAdd
#else
// begin_wdm
FORCEINLINE
LONG
FASTCALL
InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Increment
    )
{
    __asm {
         mov     eax, Increment
         mov     ecx, Addend
    lock xadd    [ecx], eax
    }
}
// end_wdm
#endif

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedCompareExchange (
    IN OUT LONG volatile *Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );

#pragma intrinsic (_InterlockedCompareExchange)
#define InterlockedCompareExchange (LONG)_InterlockedCompareExchange
#else
FORCEINLINE
LONG
FASTCALL
InterlockedCompareExchange(
    IN OUT LONG volatile *Destination,
    IN LONG Exchange,
    IN LONG Comperand
    )
{
    __asm {
        mov     eax, Comperand
        mov     ecx, Destination
        mov     edx, Exchange
   lock cmpxchg [ecx], edx
    }
}
#endif

#define InterlockedCompareExchangePointer(Destination, ExChange, Comperand) \
    (PVOID)InterlockedCompareExchange((PLONG)Destination, (LONG)ExChange, (LONG)Comperand)

#define InterlockedCompareExchange64(Destination, ExChange, Comperand) \
    ExfInterlockedCompareExchange64(Destination, &(ExChange), &(Comperand))

NTKERNELAPI
LONGLONG
FASTCALL
ExfInterlockedCompareExchange64(
    IN OUT LONGLONG volatile *Destination,
    IN PLONGLONG ExChange,
    IN PLONGLONG Comperand
    );


// end_ntosp end_ntddk end_nthal
#endif      // NT_UP
// begin_ntddk begin_nthal begin_ntosp
#endif      // INTERLOCKED_INTRINSICS || _CROSS_PLATFORM_
// begin_wdm
#endif      // MIDL_PASS

#define InterlockedIncrementAcquire InterlockedIncrement
#define InterlockedIncrementRelease InterlockedIncrement
#define InterlockedDecrementAcquire InterlockedDecrement
#define InterlockedDecrementRelease InterlockedDecrement
#define InterlockedExchangeAcquire64 InterlockedExchange64
#define InterlockedCompareExchangeAcquire InterlockedCompareExchange
#define InterlockedCompareExchangeRelease InterlockedCompareExchange
#define InterlockedCompareExchangeAcquire64 InterlockedCompareExchange64
#define InterlockedCompareExchangeRelease64 InterlockedCompareExchange64
#define InterlockedCompareExchangePointerAcquire InterlockedCompareExchangePointer
#define InterlockedCompareExchangePointerRelease InterlockedCompareExchangePointer

#define InterlockedExchangeAddSizeT(a, b) InterlockedExchangeAdd((LONG *)a, b)
#define InterlockedIncrementSizeT(a) InterlockedIncrement((LONG *)a)
#define InterlockedDecrementSizeT(a) InterlockedDecrement((LONG *)a)

// end_ntosp end_ntddk end_nthal end_wdm
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4035)
#endif
#endif      // _M_IX86 && !CROSS_PLATFORM
// begin_ntddk begin_nthal begin_ntosp
#endif      // __WINBASE__ && !NONTOSPINTERLOCK
// end_ntosp end_ntddk end_nthal

// begin_nthal begin_ntddk begin_ntosp

//
// Turn these instrinsics off until the compiler can handle them
//
#if (_MSC_FULL_VER > 13009037)

LONG
_InterlockedOr (
    IN OUT LONG volatile *Target,
    IN LONG Set
    );

#pragma intrinsic (_InterlockedOr)

#define InterlockedOr _InterlockedOr
#define InterlockedOrAffinity InterlockedOr

LONG
_InterlockedAnd (
    IN OUT LONG volatile *Target,
    IN LONG Set
    );

#pragma intrinsic (_InterlockedAnd)

#define InterlockedAnd _InterlockedAnd
#define InterlockedAndAffinity InterlockedAnd

LONG
_InterlockedXor (
    IN OUT LONG volatile *Target,
    IN LONG Set
    );

#pragma intrinsic (_InterlockedXor)

#define InterlockedXor _InterlockedXor

#else // compiler version

FORCEINLINE
LONG
InterlockedAnd (
    IN OUT LONG volatile *Target,
    LONG Set
    )
{
    LONG i;
    LONG j;

    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange(Target,
                                       i & Set,
                                       i);

    } while (i != j);

    return j;
}

FORCEINLINE
LONG
InterlockedOr (
    IN OUT LONG volatile *Target,
    IN LONG Set
    )
{
    LONG i;
    LONG j;

    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange(Target,
                                       i | Set,
                                       i);

    } while (i != j);

    return j;
}


#endif // compiler version



// end_nthal end_ntddk end_ntosp

//
// Structure for Ldt information in x86 processes
//
typedef struct _LDTINFORMATION {
    ULONG Size;
    ULONG AllocatedSize;
    PLDT_ENTRY Ldt;
} LDTINFORMATION, *PLDTINFORMATION;

//
// SetProcessInformation Structure for ProcessSetIoHandlers info class
//

// begin_ntosp

typedef struct _PROCESS_IO_PORT_HANDLER_INFORMATION {
    BOOLEAN Install;            // true if handlers to be installed
    ULONG NumEntries;
    ULONG Context;
    PEMULATOR_ACCESS_ENTRY EmulatorAccessEntries;
} PROCESS_IO_PORT_HANDLER_INFORMATION, *PPROCESS_IO_PORT_HANDLER_INFORMATION;


//
//    Vdm Objects and Io handling structure
//

typedef struct _VDM_IO_HANDLER_FUNCTIONS {
    PDRIVER_IO_PORT_ULONG  UlongIo;
    PDRIVER_IO_PORT_ULONG_STRING UlongStringIo;
    PDRIVER_IO_PORT_USHORT UshortIo[2];
    PDRIVER_IO_PORT_USHORT_STRING UshortStringIo[2];
    PDRIVER_IO_PORT_UCHAR UcharIo[4];
    PDRIVER_IO_PORT_UCHAR_STRING UcharStringIo[4];
} VDM_IO_HANDLER_FUNCTIONS, *PVDM_IO_HANDLER_FUNCTIONS;

typedef struct _VDM_IO_HANDLER {
    struct _VDM_IO_HANDLER *Next;
    ULONG PortNumber;
    VDM_IO_HANDLER_FUNCTIONS IoFunctions[2];
} VDM_IO_HANDLER, *PVDM_IO_HANDLER;



// begin_nthal begin_ntddk begin_wdm


#if !defined(MIDL_PASS) && defined(_M_IX86)

//
// i386 function definitions
//

// end_wdm

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4035)               // re-enable below

// end_ntddk end_ntosp
#if NT_UP
    #define _PCR   ds:[KIP0PCRADDRESS]
#else
    #define _PCR   fs:[0]                   // ntddk ntosp
#endif


//
// Get address of current processor block.
//
// WARNING: This inline macro can only be used by the kernel or hal
//
#define KiPcr() KeGetPcr()
FORCEINLINE
PKPCR
NTAPI
KeGetPcr(VOID)
{
#if NT_UP
    return (PKPCR)KIP0PCRADDRESS;
#else

#if (_MSC_FULL_VER >= 13012035)
    return (PKPCR) (ULONG_PTR) __readfsdword (FIELD_OFFSET (KPCR, SelfPcr));
#else
    __asm {  mov eax, _PCR KPCR.SelfPcr  }
#endif

#endif
}

// end_nthal
//
// Get current node shifted color.
//

FORCEINLINE
ULONG
KeGetCurrentNodeShiftedColor (
    VOID
    )

{
    return __readfsdword(FIELD_OFFSET(KPCR, PrcbData.NodeShiftedColor));
}

// begin_nthal begin_ntosp

//
// Get address of current processor block.
//
// WARNING: This inline macro can only be used by the kernel or hal
//
FORCEINLINE
PKPRCB
NTAPI
KeGetCurrentPrcb (VOID)
{
#if (_MSC_FULL_VER >= 13012035)
    return (PKPRCB) (ULONG_PTR) __readfsdword (FIELD_OFFSET (KPCR, Prcb));
#else
    __asm {  mov eax, _PCR KPCR.Prcb     }
#endif
}

// begin_ntddk begin_wdm

//
// Get current IRQL.
//
// On x86 this function resides in the HAL
//

// end_ntddk end_wdm

#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || !defined(_APIC_TPR_)

// begin_ntddk begin_wdm

NTHALAPI
KIRQL
NTAPI
KeGetCurrentIrql();

// end_ntddk end_wdm

#endif

// begin_ntddk begin_wdm

// end_wdm
//
// Get the current processor number
//

FORCEINLINE
ULONG
NTAPI
KeGetCurrentProcessorNumber(VOID)
{
#if (_MSC_FULL_VER >= 13012035)
    return (ULONG) __readfsbyte (FIELD_OFFSET (KPCR, Number));
#else
    __asm {  movzx eax, _PCR KPCR.Number  }
#endif
}

// end_nthal end_ntddk end_ntosp
//
// Get address of current kernel thread object.
//
// WARNING: This inline macro can not be used for device drivers or HALs
// they must call the kernel function KeGetCurrentThread.
// WARNING: This inline macro is always MP enabled because filesystems
// utilize it
//
//
FORCEINLINE
struct _KTHREAD *
NTAPI KeGetCurrentThread (VOID)
{
#if (_MSC_FULL_VER >= 13012035)
    return (struct _KTHREAD *) (ULONG_PTR) __readfsdword (FIELD_OFFSET (KPCR, PrcbData.CurrentThread));
#else
    __asm {  mov eax, fs:[0] KPCR.PrcbData.CurrentThread }
#endif
}

//
// If processor executing DPC?
// WARNING: This inline macro is always MP enabled because filesystems
// utilize it
//
FORCEINLINE
ULONG
NTAPI
KeIsExecutingDpc(VOID)
{
#if (_MSC_FULL_VER >= 13012035)
    return (ULONG) __readfsword (FIELD_OFFSET (KPCR, PrcbData.DpcRoutineActive));
#else
    __asm {  movzx eax, word ptr fs:[0] KPCR.PrcbData.DpcRoutineActive }
#endif
}

// begin_nthal begin_ntddk begin_ntosp

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4035)
#endif

// begin_wdm
#endif // !defined(MIDL_PASS) && defined(_M_IX86)

// end_nthal end_ntddk end_wdm end_ntosp

#define KeIsIdleHaltSet(Prcb, Number) ((Prcb)->Sleeping != 0)

// begin_ntddk begin_nthal begin_ntndis begin_wdm begin_ntosp

//++
//
// VOID
// KeMemoryBarrier (
//    VOID
//    )
//
// VOID
// KeMemoryBarrierWithoutFence (
//    VOID
//    )
//
//
// Routine Description:
//
//    These functions order memory accesses as seen by other processors.
//
// Arguments:
//
//    None.
//
// Return Value:
//
//    None.
//
//--

#ifdef __cplusplus
extern "C" {
#endif

VOID
_ReadWriteBarrier(
    VOID
    );

#ifdef __cplusplus
}
#endif

#pragma intrinsic (_ReadWriteBarrier)

#pragma warning( push )
#pragma warning( disable : 4793 )

FORCEINLINE
VOID
KeMemoryBarrier (
    VOID
    )
{
    LONG Barrier;
    __asm {
        xchg Barrier, eax
    }
}

#pragma warning( pop )

#define KeMemoryBarrierWithoutFence() _ReadWriteBarrier()

// end_ntddk end_nthal end_ntndis end_wdm end_ntosp

//
// For the UP kernel don't generate the locked reference
//
#if defined (NT_UP)
#define KeMemoryBarrier() _ReadWriteBarrier()
#endif

// begin_nthal
//
// Macro to set address of a trap/interrupt handler to IDT
//
#define KiSetHandlerAddressToIDT(Vector, HandlerAddress) {\
    UCHAR IDTEntry = HalVectorToIDTEntry(Vector); \
    ULONG Ha = (ULONG)HandlerAddress; \
    KeGetPcr()->IDT[IDTEntry].ExtendedOffset = HIGHWORD(Ha); \
    KeGetPcr()->IDT[IDTEntry].Offset = LOWWORD(Ha); \
}

//
// Macro to return address of a trap/interrupt handler in IDT
//
#define KiReturnHandlerAddressFromIDT(Vector) \
   MAKEULONG(KiPcr()->IDT[HalVectorToIDTEntry(Vector)].ExtendedOffset, KiPcr()->IDT[HalVectorToIDTEntry(Vector)].Offset)

// end_nthal

//++
//
// BOOLEAN
// KiIsThreadNumericStateSaved(
//     IN PKTHREAD Address
//     )
//
//--
#define KiIsThreadNumericStateSaved(a) \
    (a->NpxState != NPX_STATE_LOADED)

//++
//
// VOID
// KiRundownThread(
//     IN PKTHREAD Address
//     )
//
//--

#if defined(NT_UP)

//
// On UP x86 systems, FP state is lazy saved and loaded.  If this
// thread owns the current FP context, clear the ownership field
// so we will not try to save to this thread after it has been
// terminated.
//

#define KiRundownThread(a)                          \
    if (KeGetCurrentPrcb()->NpxThread == (a))   {   \
        KeGetCurrentPrcb()->NpxThread = NULL;       \
        __asm { fninit }                            \
    }

#else

#define KiRundownThread(a)

#endif

//
// functions specific to 386 structure
//

VOID
NTAPI
KiSetIRR (
    IN ULONG SWInterruptMask
    );

//
// Procedures to support frame manipulation
//

ULONG
NTAPI
KiEspFromTrapFrame(
    IN PKTRAP_FRAME TrapFrame
    );

VOID
NTAPI
KiEspToTrapFrame(
    IN PKTRAP_FRAME TrapFrame,
    IN ULONG Esp
    );

ULONG
NTAPI
KiSegSsFromTrapFrame(
    IN PKTRAP_FRAME TrapFrame
    );

VOID
NTAPI
KiSegSsToTrapFrame(
    IN PKTRAP_FRAME TrapFrame,
    IN ULONG SegSs
    );

//
// Define prototypes for i386 specific clock and profile interrupt routines.
//

VOID
NTAPI
KiUpdateRunTime (
    VOID
    );

VOID
NTAPI
KiUpdateSystemTime (
    VOID
    );

// begin_ntddk begin_wdm begin_ntosp

NTKERNELAPI
NTSTATUS
NTAPI
KeSaveFloatingPointState (
    OUT PKFLOATING_SAVE     FloatSave
    );

NTKERNELAPI
NTSTATUS
NTAPI
KeRestoreFloatingPointState (
    IN PKFLOATING_SAVE      FloatSave
    );

// end_ntddk end_wdm
// begin_nthal

NTKERNELAPI
VOID
NTAPI
KeProfileInterruptWithSource (
    IN struct _KTRAP_FRAME *TrapFrame,
    IN KPROFILE_SOURCE ProfileSource
    );

// end_ntosp

VOID
NTAPI
KeProfileInterrupt (
    IN KIRQL OldIrql,
    IN KTRAP_FRAME TrapFrame
    );

VOID
NTAPI
KeUpdateRuntime (
    IN KIRQL OldIrql,
    IN KTRAP_FRAME TrapFrame
    );

VOID
NTAPI
KeUpdateSystemTime (
    IN KIRQL OldIrql,
    IN KTRAP_FRAME TrapFrame
    );

// begin_ntddk begin_wdm begin_ntndis begin_ntosp

#endif // defined(_X86_)

// end_nthal end_ntddk end_wdm end_ntndis end_ntosp

// begin_nthal begin_ntddk

// Use the following for kernel mode runtime checks of X86 system architecture

#ifdef _X86_

#ifdef IsNEC_98
#undef IsNEC_98
#endif

#ifdef IsNotNEC_98
#undef IsNotNEC_98
#endif

#ifdef SetNEC_98
#undef SetNEC_98
#endif

#ifdef SetNotNEC_98
#undef SetNotNEC_98
#endif

#define IsNEC_98     (SharedUserData->AlternativeArchitecture == NEC98x86)
#define IsNotNEC_98  (SharedUserData->AlternativeArchitecture != NEC98x86)
#define SetNEC_98    SharedUserData->AlternativeArchitecture = NEC98x86
#define SetNotNEC_98 SharedUserData->AlternativeArchitecture = StandardDesign

#endif

// end_nthal end_ntddk

//
// i386 arch. specific kernel functions.
//

// begin_ntosp
#ifdef _X86_
VOID
NTAPI
Ke386SetLdtProcess (
    struct _KPROCESS  *Process,
    PLDT_ENTRY  Ldt,
    ULONG       Limit
    );

VOID
NTAPI
Ke386SetDescriptorProcess (
    struct _KPROCESS  *Process,
    ULONG       Offset,
    LDT_ENTRY   LdtEntry
    );

VOID
NTAPI
Ke386GetGdtEntryThread (
    struct _KTHREAD *Thread,
    ULONG Offset,
    PKGDTENTRY Descriptor
    );

BOOLEAN
NTAPI
Ke386SetIoAccessMap (
    ULONG               MapNumber,
    PKIO_ACCESS_MAP     IoAccessMap
    );

BOOLEAN
NTAPI
Ke386QueryIoAccessMap (
    ULONG              MapNumber,
    PKIO_ACCESS_MAP    IoAccessMap
    );

BOOLEAN
NTAPI
Ke386IoSetAccessProcess (
    struct _KPROCESS    *Process,
    ULONG       MapNumber
    );

VOID
NTAPI
Ke386SetIOPL(
    VOID
    );

NTSTATUS
NTAPI
Ke386CallBios (
    IN ULONG BiosCommand,
    IN OUT PCONTEXT BiosArguments
    );

VOID
NTAPI
KiEditIopmDpc (
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

BOOLEAN
NTAPI
Ki386GetSelectorParameters(
    IN USHORT Selector,
    OUT PULONG Flags,
    OUT PULONG Base,
    OUT PULONG Limit
    );

ULONG
Ki386DispatchOpcodeV86 (
    IN PKTRAP_FRAME TrapFrame
    );

ULONG
Ki386DispatchOpcode (
    IN PKTRAP_FRAME TrapFrame
    );

NTSTATUS
NTAPI
Ke386SetVdmInterruptHandler (
    IN struct _KPROCESS *Process,
    IN ULONG Interrupt,
    IN USHORT Selector,
    IN ULONG  Offset,
    IN BOOLEAN Gate32
    );
#endif //_X86_
// end_ntosp
//
// i386 ABIOS specific routines.
//

NTSTATUS
NTAPI
KeI386GetLid(
    IN USHORT DeviceId,
    IN USHORT RelativeLid,
    IN BOOLEAN SharedLid,
    IN struct _DRIVER_OBJECT *DeviceObject,
    OUT PUSHORT LogicalId
    );

NTSTATUS
NTAPI
KeI386ReleaseLid(
    IN USHORT LogicalId,
    IN struct _DRIVER_OBJECT *DeviceObject
    );

NTSTATUS
NTAPI
KeI386AbiosCall(
    IN USHORT LogicalId,
    IN struct _DRIVER_OBJECT *DriverObject,
    IN PUCHAR RequestBlock,
    IN USHORT EntryPoint
    );

//
// i386 misc routines
//
NTSTATUS
NTAPI
KeI386AllocateGdtSelectors(
    OUT PUSHORT SelectorArray,
    IN USHORT NumberOfSelectors
    );

VOID
NTAPI
KeI386Call16BitFunction (
    IN OUT PCONTEXT Regs
    );

USHORT
NTAPI
KeI386Call16BitCStyleFunction (
    IN ULONG EntryOffset,
    IN ULONG EntrySelector,
    IN PUCHAR Parameters,
    IN ULONG Size
    );

NTSTATUS
NTAPI
KeI386FlatToGdtSelector(
    IN ULONG SelectorBase,
    IN USHORT Length,
    IN USHORT Selector
    );

NTSTATUS
NTAPI
KeI386ReleaseGdtSelectors(
    OUT PUSHORT SelectorArray,
    IN USHORT NumberOfSelectors
    );

NTSTATUS
NTAPI
KeI386SetGdtSelector (
    ULONG       Selector,
    PKGDTENTRY  GdtValue
    );


VOID
NTAPI
KeOptimizeProcessorControlState (
    VOID
    );

//
// Vdm specific functions.
//

BOOLEAN
NTAPI
KeVdmInsertQueueApc (
    IN PKAPC             Apc,
    IN struct _KTHREAD  *Thread,
    IN KPROCESSOR_MODE   ApcMode,
    IN PKKERNEL_ROUTINE  KernelRoutine,
    IN PKRUNDOWN_ROUTINE RundownRoutine OPTIONAL,
    IN PKNORMAL_ROUTINE  NormalRoutine OPTIONAL,
    IN PVOID             NormalContext OPTIONAL,
    IN KPRIORITY         Increment
    );

FORCEINLINE
VOID
NTAPI
KeVdmClearApcThreadAddress (
    IN PKAPC Apc
    )

{
    if (Apc->Inserted == FALSE) {
        Apc->Thread = NULL;
    }
}

VOID
NTAPI
KeI386VdmInitialize (
    VOID
    );

//
// x86 functions for special instructions
//

VOID
NTAPI
CPUID (
    ULONG   InEax,
    PULONG  OutEax,
    PULONG  OutEbx,
    PULONG  OutEcx,
    PULONG  OutEdx
    );

LONGLONG
NTAPI
RDTSC (
    VOID
    );

ULONGLONG
FASTCALL
RDMSR (
    IN ULONG MsrRegister
    );

VOID
NTAPI
WRMSR (
    IN ULONG MsrRegister,
    IN ULONGLONG MsrValue
    );

//
// i386 Vdm specific data
//
extern ULONG KeI386EFlagsAndMaskV86;
extern ULONG KeI386EFlagsOrMaskV86;
extern ULONG KeI386VirtualIntExtensions;


extern ULONG KeI386CpuType;
extern ULONG KeI386CpuStep;
extern BOOLEAN KeI386NpxPresent;
extern BOOLEAN KeI386FxsrPresent;


//
// i386 Feature bit definitions
//
// N.B. The no execute feature flags must be identical on all platforms.

#define KF_V86_VIS          0x00000001
#define KF_RDTSC            0x00000002
#define KF_CR4              0x00000004
#define KF_CMOV             0x00000008
#define KF_GLOBAL_PAGE      0x00000010
#define KF_LARGE_PAGE       0x00000020
#define KF_MTRR             0x00000040
#define KF_CMPXCHG8B        0x00000080
#define KF_MMX              0x00000100
#define KF_WORKING_PTE      0x00000200
#define KF_PAT              0x00000400
#define KF_FXSR             0x00000800
#define KF_FAST_SYSCALL     0x00001000
#define KF_XMMI             0x00002000
#define KF_3DNOW            0x00004000
#define KF_AMDK6MTRR        0x00008000
#define KF_XMMI64           0x00010000
#define KF_DTS              0x00020000
#define KF_NOEXECUTE        0x20000000
#define KF_GLOBAL_32BIT_EXECUTE 0x40000000
#define KF_GLOBAL_32BIT_NOEXECUTE 0x80000000

//
// Define macro to test if x86 feature is present.
//

extern ULONG KiBootFeatureBits;

#define Isx86FeaturePresent(_f_) ((KiBootFeatureBits & (_f_)) != 0)

#endif // _i386_

#if defined(__cplusplus)
} // extern "C"
#endif // defined(__cplusplus)
