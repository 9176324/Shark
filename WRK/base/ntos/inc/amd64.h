/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    amd64.h

Abstract:

    This module contains the AMD64 hardware specific header file.

--*/

#ifndef __amd64_
#define __amd64_


// begin_ntosp

#if defined(_M_AMD64)

VOID
KeCompactServiceTable (
    IN PVOID Table,
    IN ULONG limit,
    IN BOOLEAN Win32k
    );

//
// Image header machine architecture
//

#define IMAGE_FILE_MACHINE_NATIVE   0x8664

#endif

// end_ntosp

#if !(defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_)) && !defined(_BLDR_)

#define ExRaiseException RtlRaiseException
#define ExRaiseStatus RtlRaiseStatus

#endif

// begin_ntddk begin_wdm begin_nthal begin_ntndis begin_ntosp
// begin_ntminiport

#if defined(_M_AMD64) && !defined(RC_INVOKED) && !defined(MIDL_PASS)

//
// Define intrinsic function to do in's and out's.
//

#ifdef __cplusplus
extern "C" {
#endif

UCHAR
__inbyte (
    IN USHORT Port
    );

USHORT
__inword (
    IN USHORT Port
    );

ULONG
__indword (
    IN USHORT Port
    );

VOID
__outbyte (
    IN USHORT Port,
    IN UCHAR Data
    );

VOID
__outword (
    IN USHORT Port,
    IN USHORT Data
    );

VOID
__outdword (
    IN USHORT Port,
    IN ULONG Data
    );

VOID
__inbytestring (
    IN USHORT Port,
    IN PUCHAR Buffer,
    IN ULONG Count
    );

VOID
__inwordstring (
    IN USHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    );

VOID
__indwordstring (
    IN USHORT Port,
    IN PULONG Buffer,
    IN ULONG Count
    );

VOID
__outbytestring (
    IN USHORT Port,
    IN PUCHAR Buffer,
    IN ULONG Count
    );

VOID
__outwordstring (
    IN USHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    );

VOID
__outdwordstring (
    IN USHORT Port,
    IN PULONG Buffer,
    IN ULONG Count
    );

#ifdef __cplusplus
}
#endif

#pragma intrinsic(__inbyte)
#pragma intrinsic(__inword)
#pragma intrinsic(__indword)
#pragma intrinsic(__outbyte)
#pragma intrinsic(__outword)
#pragma intrinsic(__outdword)
#pragma intrinsic(__inbytestring)
#pragma intrinsic(__inwordstring)
#pragma intrinsic(__indwordstring)
#pragma intrinsic(__outbytestring)
#pragma intrinsic(__outwordstring)
#pragma intrinsic(__outdwordstring)

#endif // defined(_M_AMD64) && !defined(RC_INVOKED) && !defined(MIDL_PASS)
// end_ntminiport

#if defined(_AMD64_)

//
// Types to use to contain PFNs and their counts.
//

typedef ULONG PFN_COUNT;

typedef LONG64 SPFN_NUMBER, *PSPFN_NUMBER;
typedef ULONG64 PFN_NUMBER, *PPFN_NUMBER;

//
// Define maximum size of flush multiple TB request.
//

#define FLUSH_MULTIPLE_MAXIMUM 19

//
// Indicate that the AMD64 compiler supports the allocate pragmas.
//

#define ALLOC_PRAGMA 1
#define ALLOC_DATA_PRAGMA 1

// end_ntddk end_nthal end_ntndis end_wdm end_ntosp


//
// Length on interrupt object dispatch code in longwords.
//

// begin_nthal

#define NORMAL_DISPATCH_LENGTH 4                    // ntddk wdm
#define DISPATCH_LENGTH NORMAL_DISPATCH_LENGTH      // ntddk wdm
                                                    // ntddk wdm

// begin_ntosp
//
// Define constants for bits in CR0.
//

#define CR0_PE 0x00000001               // protection enable
#define CR0_MP 0x00000002               // math present
#define CR0_EM 0x00000004               // emulate math coprocessor
#define CR0_TS 0x00000008               // task switched
#define CR0_ET 0x00000010               // extension type (80387)
#define CR0_NE 0x00000020               // numeric error
#define CR0_WP 0x00010000               // write protect
#define CR0_AM 0x00040000               // alignment mask
#define CR0_NW 0x20000000               // not write-through
#define CR0_CD 0x40000000               // cache disable
#define CR0_PG 0x80000000               // paging

//
// Define functions to read and write CR0.
//

// begin_wdm

#ifdef __cplusplus
extern "C" {
#endif

// end_wdm

#define ReadCR0() __readcr0()

ULONG64
__readcr0 (
    VOID
    );

#define WriteCR0(Data) __writecr0(Data)

VOID
__writecr0 (
    IN ULONG64 Data
    );

#pragma intrinsic(__readcr0)
#pragma intrinsic(__writecr0)

//
// Define functions to read and write CR3.
//

#define ReadCR3() __readcr3()

ULONG64
__readcr3 (
    VOID
    );

#define WriteCR3(Data) __writecr3(Data)

VOID
__writecr3 (
    IN ULONG64 Data
    );

#pragma intrinsic(__readcr3)
#pragma intrinsic(__writecr3)

//
// Define constants for bits in CR4.
//

#define CR4_VME 0x00000001              // V86 mode extensions
#define CR4_PVI 0x00000002              // Protected mode virtual interrupts
#define CR4_TSD 0x00000004              // Time stamp disable
#define CR4_DE  0x00000008              // Debugging Extensions
#define CR4_PSE 0x00000010              // Page size extensions
#define CR4_PAE 0x00000020              // Physical address extensions
#define CR4_MCE 0x00000040              // Machine check enable
#define CR4_PGE 0x00000080              // Page global enable
#define CR4_FXSR 0x00000200             // FXSR used by OS
#define CR4_XMMEXCPT 0x00000400         // XMMI used by OS

//
// Define functions to read and write CR4.
//

#define ReadCR4() __readcr4()

ULONG64
__readcr4 (
    VOID
    );

#define WriteCR4(Data) __writecr4(Data)

VOID
__writecr4 (
    IN ULONG64 Data
    );

#pragma intrinsic(__readcr4)
#pragma intrinsic(__writecr4)

// begin_ntddk begin_ntifs begin_wdm
//
// Define functions to read and write CR8.
//
// CR8 is the APIC TPR register.
//

#define ReadCR8() __readcr8()

ULONG64
__readcr8 (
    VOID
    );

#define WriteCR8(Data) __writecr8(Data)

VOID
__writecr8 (
    IN ULONG64 Data
    );

#pragma intrinsic(__readcr8)
#pragma intrinsic(__writecr8)

// end_ntddk end_ntifs end_wdm

// begin_wdm

#ifdef __cplusplus
}
#endif

// end_nthal end_ntosp end_wdm

//
// External references to the code labels.
//

extern ULONG KiInterruptTemplate[NORMAL_DISPATCH_LENGTH];
extern ULONG KiSpuriousInterruptTemplate[NORMAL_DISPATCH_LENGTH];

// begin_ntddk begin_wdm begin_nthal begin_ntosp
//
// Interrupt Request Level definitions
//

#define PASSIVE_LEVEL 0                 // Passive release level
#define LOW_LEVEL 0                     // Lowest interrupt level
#define APC_LEVEL 1                     // APC interrupt level
#define DISPATCH_LEVEL 2                // Dispatcher level

#define CLOCK_LEVEL 13                  // Interval clock level
#define IPI_LEVEL 14                    // Interprocessor interrupt level
#define POWER_LEVEL 14                  // Power failure level
#define PROFILE_LEVEL 15                // timer used for profiling.
#define HIGH_LEVEL 15                   // Highest interrupt level

// end_ntddk end_wdm end_ntosp

#if defined(NT_UP)

// synchronization level (UP)
#define SYNCH_LEVEL DISPATCH_LEVEL      

#else

// synchronization level (MP)
#define SYNCH_LEVEL (IPI_LEVEL-2)       // ntddk wdm ntosp

#endif

#define IRQL_VECTOR_OFFSET 2            // offset from IRQL to vector / 16

#define KiSynchIrql SYNCH_LEVEL         // enable portable code

//
// Machine type definitions
//

#define MACHINE_TYPE_ISA 0
#define MACHINE_TYPE_EISA 1
#define MACHINE_TYPE_MCA 2

// end_nthal
//
//  The previous values and the following are or'ed in KeI386MachineType.
//

#define MACHINE_TYPE_PC_AT_COMPATIBLE      0x00000000
#define MACHINE_TYPE_PC_9800_COMPATIBLE    0x00000100
#define MACHINE_TYPE_FMR_COMPATIBLE        0x00000200

extern ULONG KeI386MachineType;

// begin_nthal 
//
// Define constants used in selector tests.
//
//  N.B. MODE_MASK and MODE_BIT assumes that all code runs at either ring-0
//       or ring-3 and is used to test the mode. RPL_MASK is used for merging
//       or extracting RPL values.
//

#define MODE_BIT 0
#define MODE_MASK 1                                                 // ntosp
#define RPL_MASK 3

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

#define HIGHWORD(l) ((USHORT)((ULONG)(l) >> 16))

//
// Macro to extract the low word of a long offset
//

#define LOWWORD(l) ((USHORT)((ULONG)l))

//
// Macro to combine two USHORT offsets into a long offset
//

#if !defined(MAKEULONG)

#define MAKEULONG(x, y) ((((ULONG)(x)) << 16) | (USHORT)((ULONG)(y)))

#endif

// end_nthal

// begin_ntddk begin_wdm begin_nthal begin_ntndis begin_ntosp

// begin_ntminiport

#if defined(_AMD64_)

//
// I/O space read and write macros.
//
//  The READ/WRITE_REGISTER_* calls manipulate I/O registers in MEMORY space.
//
//  The READ/WRITE_PORT_* calls manipulate I/O registers in PORT space.
//

__forceinline
UCHAR
READ_REGISTER_UCHAR (
    volatile UCHAR *Register
    )
{
    _ReadWriteBarrier();
    return *Register;
}

__forceinline
USHORT
READ_REGISTER_USHORT (
    volatile USHORT *Register
    )
{
    _ReadWriteBarrier();
    return *Register;
}

__forceinline
ULONG
READ_REGISTER_ULONG (
    volatile ULONG *Register
    )
{
    _ReadWriteBarrier();
    return *Register;
}

__forceinline
VOID
READ_REGISTER_BUFFER_UCHAR (
    PUCHAR Register,
    PUCHAR Buffer,
    ULONG Count
    )
{
    _ReadWriteBarrier();
    __movsb(Buffer, Register, Count);
    return;
}

__forceinline
VOID
READ_REGISTER_BUFFER_USHORT (
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG Count
    )
{
    _ReadWriteBarrier();
    __movsw(Buffer, Register, Count);
    return;
}

__forceinline
VOID
READ_REGISTER_BUFFER_ULONG (
    PULONG Register,
    PULONG Buffer,
    ULONG Count
    )
{
    _ReadWriteBarrier();
    __movsd(Buffer, Register, Count);
    return;
}

__forceinline
VOID
WRITE_REGISTER_UCHAR (
    volatile UCHAR *Register,
    UCHAR Value
    )
{

    *Register = Value;
    FastFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_USHORT (
    volatile USHORT *Register,
    USHORT Value
    )
{

    *Register = Value;
    FastFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_ULONG (
    volatile ULONG *Register,
    ULONG Value
    )
{

    *Register = Value;
    FastFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_BUFFER_UCHAR (
    PUCHAR Register,
    PUCHAR Buffer,
    ULONG Count
    )
{

    __movsb(Register, Buffer, Count);
    FastFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_BUFFER_USHORT (
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG Count
    )
{

    __movsw(Register, Buffer, Count);
    FastFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_BUFFER_ULONG (
    PULONG Register,
    PULONG Buffer,
    ULONG Count
    )
{

    __movsd(Register, Buffer, Count);
    FastFence();
    return;
}

__forceinline
UCHAR
READ_PORT_UCHAR (
    PUCHAR Port
    )

{
    return __inbyte((USHORT)((ULONG64)Port));
}

__forceinline
USHORT
READ_PORT_USHORT (
    PUSHORT Port
    )

{
    return __inword((USHORT)((ULONG64)Port));
}

__forceinline
ULONG
READ_PORT_ULONG (
    PULONG Port
    )

{
    return __indword((USHORT)((ULONG64)Port));
}


__forceinline
VOID
READ_PORT_BUFFER_UCHAR (
    PUCHAR Port,
    PUCHAR Buffer,
    ULONG Count
    )

{
    __inbytestring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
READ_PORT_BUFFER_USHORT (
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG Count
    )

{
    __inwordstring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
READ_PORT_BUFFER_ULONG (
    PULONG Port,
    PULONG Buffer,
    ULONG Count
    )

{
    __indwordstring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
WRITE_PORT_UCHAR (
    PUCHAR Port,
    UCHAR Value
    )

{
    __outbyte((USHORT)((ULONG64)Port), Value);
    return;
}

__forceinline
VOID
WRITE_PORT_USHORT (
    PUSHORT Port,
    USHORT Value
    )

{
    __outword((USHORT)((ULONG64)Port), Value);
    return;
}

__forceinline
VOID
WRITE_PORT_ULONG (
    PULONG Port,
    ULONG Value
    )

{
    __outdword((USHORT)((ULONG64)Port), Value);
    return;
}

__forceinline
VOID
WRITE_PORT_BUFFER_UCHAR (
    PUCHAR Port,
    PUCHAR Buffer,
    ULONG Count
    )

{
    __outbytestring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
WRITE_PORT_BUFFER_USHORT (
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG Count
    )

{
    __outwordstring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
WRITE_PORT_BUFFER_ULONG (
    PULONG Port,
    PULONG Buffer,
    ULONG Count
    )

{
    __outdwordstring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

#endif

// end_ntminiport

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
// Fill TB entry and flush single TB entry.
//

#define KeFillEntryTb(Virtual)                              \
        InvalidatePage(Virtual);

__forceinline
VOID
KeFlushCurrentTb (
    VOID
    )

{

    ULONG64 Cr4;

    Cr4 = ReadCR4();
    WriteCR4(Cr4 & ~CR4_PGE);
    WriteCR4(Cr4);
    return;
}

__forceinline
VOID
KiFlushProcessTb (
    VOID
    )

{

    ULONG64 Cr3;

    Cr3 = ReadCR3();
    WriteCR3(Cr3);
    return;
}

#define KiFlushSingleTb(Virtual) InvalidatePage(Virtual)

//
// Data cache, instruction cache, I/O buffer, and write buffer flush routine
// prototypes.
//

//  AMD64 has transparent caches, so these are noops.

#define KeSweepDcache(AllProcessors)
#define KeSweepCurrentDcache()

#define KeSweepIcache(AllProcessors)
#define KeSweepCurrentIcache()

#define KeSweepIcacheRange(AllProcessors, BaseAddress, Length)

// begin_ntddk begin_wdm begin_nthal begin_ntndis begin_ntosp

#define KeFlushIoBuffers(Mdl, ReadOperation, DmaOperation)

// end_ntddk end_wdm end_ntndis end_ntosp

#define KeYieldProcessor YieldProcessor

// end_nthal

//
// Define executive macros for acquiring and releasing executive spinlocks.
// These macros can ONLY be used by executive components and NOT by drivers.
// Drivers MUST use the kernel interfaces since they must be MP enabled on
// all systems.
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

// begin_nthal

//
// The acquire and release fast lock macros disable and enable interrupts
// on UP nondebug systems. On MP or debug systems, the spinlock routines
// are used.
//
// N.B. Extreme caution should be observed when using these routines.
//

#if defined(_M_AMD64) && !defined(USER_MODE_CODE)

VOID
_disable (
    VOID
    );

VOID
_enable (
    VOID
    );

#pragma intrinsic(_disable)
#pragma intrinsic(_enable)

#endif

// end_nthal

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

#if defined(NT_UP)

#define KiAcquireSpinLock(SpinLock)
#define KiReleaseSpinLock(SpinLock)

#else

#define KiAcquireSpinLock(SpinLock) KeAcquireSpinLockAtDpcLevel(SpinLock)
#define KiReleaseSpinLock(SpinLock) KeReleaseSpinLockFromDpcLevel(SpinLock)

#endif // defined(NT_UP)

// end_nthal

//
// Define query tick count macro.
//
// begin_ntddk begin_nthal begin_ntosp begin_wdm

#define KI_USER_SHARED_DATA 0xFFFFF78000000000UI64

#define SharedUserData ((KUSER_SHARED_DATA * const)KI_USER_SHARED_DATA)

#define SharedInterruptTime (KI_USER_SHARED_DATA + 0x8)
#define SharedSystemTime (KI_USER_SHARED_DATA + 0x14)
#define SharedTickCount (KI_USER_SHARED_DATA + 0x320)

#define KeQueryInterruptTime() *((volatile ULONG64 *)(SharedInterruptTime))

#define KeQuerySystemTime(CurrentCount)                                     \
    *((PULONG64)(CurrentCount)) = *((volatile ULONG64 *)(SharedSystemTime))
    
#define KeQueryTickCount(CurrentCount)                                      \
    *((PULONG64)(CurrentCount)) = *((volatile ULONG64 *)(SharedTickCount))

// end_ntddk end_nthal end_ntosp end_wdm

C_ASSERT((FIELD_OFFSET(KUSER_SHARED_DATA, InterruptTime) & 7) == 0);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, InterruptTime) == 0x8);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, SystemTime) == 0x14);
C_ASSERT((FIELD_OFFSET(KUSER_SHARED_DATA, TickCount) & 7) == 0);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TickCount) == 0x320);

//
// Define query interrupt time macro.
//

C_ASSERT((FIELD_OFFSET(KUSER_SHARED_DATA, InterruptTime) & 7) == 0); 

#define KiQueryInterruptTime(CurrentTime)                                   \
    ((PLARGE_INTEGER)(CurrentTime))->QuadPart = *(PLONG64)(&SharedUserData->InterruptTime)

// begin_nthal begin_ntosp
//
// AMD64 hardware structures
//
// A Page Table Entry on an AMD64 has the following definition.
//

#define _HARDWARE_PTE_WORKING_SET_BITS  11

typedef struct _HARDWARE_PTE {
    ULONG64 Valid : 1;
    ULONG64 Write : 1;                // UP version
    ULONG64 Owner : 1;
    ULONG64 WriteThrough : 1;
    ULONG64 CacheDisable : 1;
    ULONG64 Accessed : 1;
    ULONG64 Dirty : 1;
    ULONG64 LargePage : 1;
    ULONG64 Global : 1;
    ULONG64 CopyOnWrite : 1;          // software field
    ULONG64 Prototype : 1;            // software field
    ULONG64 reserved0 : 1;            // software field
    ULONG64 PageFrameNumber : 28;
    ULONG64 reserved1 : 24 - (_HARDWARE_PTE_WORKING_SET_BITS+1);
    ULONG64 SoftwareWsIndex : _HARDWARE_PTE_WORKING_SET_BITS;
    ULONG64 NoExecute : 1;
} HARDWARE_PTE, *PHARDWARE_PTE;

//
// Define macro to initialize directory table base.
//

#define INITIALIZE_DIRECTORY_TABLE_BASE(dirbase,pfn) \
     *((PULONG64)(dirbase)) = (((ULONG64)(pfn)) << PAGE_SHIFT)

//
// Define Global Descriptor Table (GDT) entry structure and constants.
//
// Define descriptor type codes.
//

#define TYPE_CODE 0x1B                  // 11011 = code, read only, accessed
#define TYPE_DATA 0x13                  // 10011 = data, read and write, accessed
#define TYPE_TSS64 0x09                 // 01001 = task state segment

//
// Define descriptor privilege levels for user and system.
//

#define DPL_USER 3
#define DPL_SYSTEM 0

//
// Define limit granularity.
//

#define GRANULARITY_BYTE 0
#define GRANULARITY_PAGE 1

#define SELECTOR_TABLE_INDEX 0x04

typedef union _KGDTENTRY64 {
    struct {
        USHORT  LimitLow;
        USHORT  BaseLow;
        union {
            struct {
                UCHAR   BaseMiddle;
                UCHAR   Flags1;
                UCHAR   Flags2;
                UCHAR   BaseHigh;
            } Bytes;

            struct {
                ULONG   BaseMiddle : 8;
                ULONG   Type : 5;
                ULONG   Dpl : 2;
                ULONG   Present : 1;
                ULONG   LimitHigh : 4;
                ULONG   System : 1;
                ULONG   LongMode : 1;
                ULONG   DefaultBig : 1;
                ULONG   Granularity : 1;
                ULONG   BaseHigh : 8;
            } Bits;
        };

        ULONG BaseUpper;
        ULONG MustBeZero;
    };

    ULONG64 Alignment;
} KGDTENTRY64, *PKGDTENTRY64;

//
// Define Interrupt Descriptor Table (IDT) entry structure and constants.
//

typedef union _KIDTENTRY64 {
   struct {
       USHORT OffsetLow;
       USHORT Selector;
       USHORT IstIndex : 3;
       USHORT Reserved0 : 5;
       USHORT Type : 5;
       USHORT Dpl : 2;
       USHORT Present : 1;
       USHORT OffsetMiddle;
       ULONG OffsetHigh;
       ULONG Reserved1;
   };

   ULONG64 Alignment;
} KIDTENTRY64, *PKIDTENTRY64;

//
// Define two union definitions used for parsing addresses into the
// component fields required by a GDT.
//

typedef union _KGDT_BASE {
    struct {
        USHORT BaseLow;
        UCHAR BaseMiddle;
        UCHAR BaseHigh;
        ULONG BaseUpper;
    };

    ULONG64 Base;
} KGDT_BASE, *PKGDT_BASE;

C_ASSERT(sizeof(KGDT_BASE) == sizeof(ULONG64));


typedef union _KGDT_LIMIT {
    struct {
        USHORT LimitLow;
        USHORT LimitHigh : 4;
        USHORT MustBeZero : 12;
    };

    ULONG Limit;
} KGDT_LIMIT, *PKGDT_LIMIT;

C_ASSERT(sizeof(KGDT_LIMIT) == sizeof(ULONG));

//
// Define Task State Segment (TSS) structure and constants.
//
// Task switches are not supported by the AMD64, but a task state segment
// must be present to define the kernel stack pointer and I/O map base.
//
// N.B. This structure is misaligned as per the AMD64 specification.
//
// N.B. The size of TSS must be <= 0xDFFF.
//

#pragma pack(push, 4)
typedef struct _KTSS64 {
    ULONG Reserved0;
    ULONG64 Rsp0;
    ULONG64 Rsp1;
    ULONG64 Rsp2;

    //
    // Element 0 of the Ist is reserved.
    //

    ULONG64 Ist[8];
    ULONG64 Reserved1;
    USHORT Reserved2;
    USHORT IoMapBase;
} KTSS64, *PKTSS64;
#pragma pack(pop)

C_ASSERT((sizeof(KTSS64) % sizeof(PVOID)) == 0);

#define TSS_IST_RESERVED 0
#define TSS_IST_PANIC 1
#define TSS_IST_MCA 2
#define TSS_IST_NMI 3

#define IO_ACCESS_MAP_NONE FALSE

#define KiComputeIopmOffset(Enable)  (sizeof(KTSS64))

// begin_windbgkd

#if defined(_AMD64_)

//
// Define pseudo descriptor structures for both 64- and 32-bit mode.
//

typedef struct _KDESCRIPTOR {
    USHORT Pad[3];
    USHORT Limit;
    PVOID Base;
} KDESCRIPTOR, *PKDESCRIPTOR;

typedef struct _KDESCRIPTOR32 {
    USHORT Pad[3];
    USHORT Limit;
    ULONG Base;
} KDESCRIPTOR32, *PKDESCRIPTOR32;

//
// Define special kernel registers and the initial MXCSR value.
//

typedef struct _KSPECIAL_REGISTERS {
    ULONG64 Cr0;
    ULONG64 Cr2;
    ULONG64 Cr3;
    ULONG64 Cr4;
    ULONG64 KernelDr0;
    ULONG64 KernelDr1;
    ULONG64 KernelDr2;
    ULONG64 KernelDr3;
    ULONG64 KernelDr6;
    ULONG64 KernelDr7;
    KDESCRIPTOR Gdtr;
    KDESCRIPTOR Idtr;
    USHORT Tr;
    USHORT Ldtr;
    ULONG MxCsr;
    ULONG64 DebugControl;
    ULONG64 LastBranchToRip;
    ULONG64 LastBranchFromRip;
    ULONG64 LastExceptionToRip;
    ULONG64 LastExceptionFromRip;
    ULONG64 Cr8;
    ULONG64 MsrGsBase;
    ULONG64 MsrGsSwap;
    ULONG64 MsrStar;
    ULONG64 MsrLStar;
    ULONG64 MsrCStar;
    ULONG64 MsrSyscallMask;
} KSPECIAL_REGISTERS, *PKSPECIAL_REGISTERS;

//
// Define processor state structure.
//

typedef struct _KPROCESSOR_STATE {
    KSPECIAL_REGISTERS SpecialRegisters;
    CONTEXT ContextFrame;
} KPROCESSOR_STATE, *PKPROCESSOR_STATE;

// end_nthal end_ntosp
//
// Define kernel stack control area.
//
// N.B. The kernel stack control area occupies the ending unused space in the
//      NPX save area.
//
// N.B. The current kernel stack segment structure must immediately precede
//      the previous kernel stack segment structure. This adjacency is taken
//      advantage of in the kernel stack segment enumeration routines.
//

typedef struct _KERNEL_STACK_CONTROL {
    union {
        XMM_SAVE_AREA32 XmmSaveArea;
        struct {
            UCHAR Fill[sizeof(XMM_SAVE_AREA32) - 2 * sizeof(KERNEL_STACK_SEGMENT)];
            KERNEL_STACK_SEGMENT Current;
            KERNEL_STACK_SEGMENT Previous;
        };
    };

} KERNEL_STACK_CONTROL, *PKERNEL_STACK_CONTROL;

#define KERNEL_STACK_CONTROL_LENGTH sizeof(KERNEL_STACK_CONTROL)

C_ASSERT(sizeof(XMM_SAVE_AREA32) == sizeof(KERNEL_STACK_CONTROL));
C_ASSERT(FIELD_OFFSET(KERNEL_STACK_CONTROL, Previous) == (FIELD_OFFSET(KERNEL_STACK_CONTROL, Current) + sizeof(KERNEL_STACK_SEGMENT)));

// begin_nthal begin_ntosp

#endif // _AMD64_

// end_windbgkd

//
// DPC data structure definition.
//

typedef struct _KDPC_DATA {
    LIST_ENTRY DpcListHead;
    KSPIN_LOCK DpcLock;
    volatile LONG DpcQueueDepth;
    ULONG DpcCount;
} KDPC_DATA, *PKDPC_DATA;

//
// Define request packet structure.
//

typedef struct _KREQUEST_PACKET {
    PVOID CurrentPacket[3];
    PKIPI_WORKER WorkerRoutine;
} KREQUEST_PACKET, *PKREQUEST_PACKET;

//
// Define request mailbox structure.
//

typedef struct _REQUEST_MAILBOX {
    LONG64 RequestSummary;
    union {
        KREQUEST_PACKET RequestPacket;
        PVOID Virtual[7];
    };

} REQUEST_MAILBOX, *PREQUEST_MAILBOX;

//
// Define processor vendors.
//

typedef enum {
    CPU_UNKNOWN,
    CPU_AMD,
    CPU_INTEL
} CPU_VENDORS;

//
// Processor Control Block (PRCB)
//

#define PRCB_MAJOR_VERSION 1
#define PRCB_MINOR_VERSION 1

#define PRCB_BUILD_DEBUG 0x1
#define PRCB_BUILD_UNIPROCESSOR 0x2

typedef struct _KPRCB {

//
// Start of the architecturally defined section of the PRCB. This section
// may be directly addressed by vendor/platform specific HAL code and will
// not change from version to version of NT.
//

    ULONG MxCsr;
    UCHAR Number;
    UCHAR NestingLevel;
    BOOLEAN InterruptRequest;
    BOOLEAN IdleHalt;
    struct _KTHREAD *CurrentThread;
    struct _KTHREAD *NextThread;
    struct _KTHREAD *IdleThread;
    ULONG64 UserRsp;
    ULONG64 RspBase;
    KSPIN_LOCK PrcbLock;
    KAFFINITY SetMember;
    KPROCESSOR_STATE ProcessorState;
    CCHAR CpuType;
    CCHAR CpuID;
    USHORT CpuStep;
    ULONG MHz;
    ULONG64 HalReserved[8];
    USHORT MinorVersion;
    USHORT MajorVersion;
    UCHAR BuildType;
    UCHAR CpuVendor;
    UCHAR InitialApicId;
    UCHAR LogicalProcessorsPerPhysicalProcessor;
    ULONG ApicMask;
    UCHAR CFlushSize;
    UCHAR PrcbPad0x[3];
    PVOID AcpiReserved;
    ULONG64 PrcbPad00[4];

//
// End of the architecturally defined section of the PRCB.
//
// end_nthal end_ntosp
//
// Numbered queued spin locks - 128-byte aligned.
//

    KSPIN_LOCK_QUEUE LockQueue[LockQueueMaximumLock];

//
// Nonpaged per processor lookaside lists - 128-byte aligned.
//

    PP_LOOKASIDE_LIST PPLookasideList[16];

//
// Nonpaged per processor small pool lookaside lists - 128-byte aligned.
//

    PP_LOOKASIDE_LIST PPNPagedLookasideList[POOL_SMALL_LISTS];

//
// Paged per processor small pool lookaside lists.
//

    PP_LOOKASIDE_LIST PPPagedLookasideList[POOL_SMALL_LISTS];

//
// MP interprocessor request packet barrier - 128-byte aligned.
//
// This cache line shares per processor data with the packet barrier which
// is used to signal the completion of an IPI request.
//
// The packet barrier variable is written by the initiating processor when
// an IPI request is distributed to more than one target processor (sharing
// other data in the cache line increases the probability that the write will
// hit in the cache).
//
// The initiating processor waits (at elevated IRQL - generally SYNCH level)
// for the last finishing processor to clear packet barrier which will cause
// the packet barrier cache line to transfer to the last finishing processor
// then back to respective processor. 
//
// N.B. This results in minimal sharing of the cache line (no more than would
// have occurred if the packet barrier was in a cache line all by itself)) and
// increases the probability of a cache hit when packet barrier is initialized.
//

    volatile KAFFINITY PacketBarrier;
    SINGLE_LIST_ENTRY DeferredReadyListHead;

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
// I/O IRP float.
//

    LONG LookasideIrpFloat;

//
// Number of system calls.
//

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

//
// Context switch count.
//

    ULONG KeContextSwitches;
    UCHAR PrcbPad2[12];

//
// MP interprocessor request packet and summary - 128-byte aligned.
//

    volatile KAFFINITY TargetSet;
    volatile ULONG IpiFrozen;
    UCHAR PrcbPad3[116];

//
// Interprocessor request summary - 128-byte aligned.
//

    REQUEST_MAILBOX RequestMailbox[MAXIMUM_PROCESSORS];

//
// Interprocessor sender summary;
//

    volatile KAFFINITY SenderSummary;
    UCHAR PrcbPad4[120];

//
// DPC listhead, counts, and batching parameters - 128-byte aligned.
//

    KDPC_DATA DpcData[2];
    PVOID DpcStack;
    PVOID SavedRsp;
    LONG MaximumDpcQueueDepth;
    ULONG DpcRequestRate;
    ULONG MinimumDpcRate;
    volatile BOOLEAN DpcInterruptRequested;
    volatile BOOLEAN DpcThreadRequested;

//
// N.B. the following two fields must be on a word boundary.
//

    volatile BOOLEAN DpcRoutineActive;
    volatile BOOLEAN DpcThreadActive;
    union {
        volatile ULONG64 TimerHand;
        volatile ULONG64 TimerRequest;
    };

    LONG TickOffset;
    LONG MasterOffset;
    ULONG DpcLastCount;
    BOOLEAN ThreadDpcEnable;
    volatile BOOLEAN QuantumEnd;
    UCHAR PrcbPad50;
    volatile BOOLEAN IdleSchedule;
    LONG DpcSetEventRequest;
    LONG PrcbPad40;

//
// DPC thread and generic call DPC - 128-byte aligned
//

    PVOID DpcThread;
    KEVENT DpcEvent;
    KDPC CallDpc;
    ULONG64 PrcbPad7[4];

//
// Per-processor ready summary and ready queues - 128-byte aligned.
//
// N.B. Ready summary is in the first cache line as the queue for priority
//      zero is never used.
//

    LIST_ENTRY WaitListHead;
    ULONG ReadySummary;
    ULONG QueueIndex;
    LIST_ENTRY DispatcherReadyListHead[MAXIMUM_PRIORITY];

//
// Miscellaneous counters.
//

    ULONG InterruptCount;
    ULONG KernelTime;
    ULONG UserTime;
    ULONG DpcTime;
    ULONG InterruptTime;
    ULONG AdjustDpcThreshold;
    BOOLEAN SkipTick;
    KIRQL DebuggerSavedIRQL;
    UCHAR PollSlot;
    UCHAR PrcbPad8[13];
    struct _KNODE * ParentNode;
    KAFFINITY MultiThreadProcessorSet;
    struct _KPRCB * MultiThreadSetMaster;
    LONG Sleeping;
    ULONG PrcbPad90[1];
    ULONG DebugDpcTime;
    ULONG PageColor;
    ULONG NodeColor;
    ULONG NodeShiftedColor;
    ULONG SecondaryColorMask;
    UCHAR PrcbPad9[12];

//
// Performance counters - 128-byte aligned.
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
// Kernel performance counters.
//

    ULONG KeAlignmentFixupCount;
    ULONG KeDcacheFlushCount;
    ULONG KeExceptionDispatchCount;
    ULONG KeFirstLevelTbFills;
    ULONG KeFloatingEmulationCount;
    ULONG KeIcacheFlushCount;
    ULONG KeSecondLevelTbFills;

//
// Processor information.
//

    UCHAR VendorString[13];
    UCHAR PrcbPad10[2];
    ULONG FeatureBits;
    LARGE_INTEGER UpdateSignature;

//
// Processors power state
//

    PROCESSOR_POWER_STATE PowerState;

//
// Logical Processor Cache Information  
//

    CACHE_DESCRIPTOR Cache[5];
    ULONG CacheCount;

// begin_nthal begin_ntosp

} KPRCB, *PKPRCB, *RESTRICTED_POINTER PRKPRCB;

// end_nthal end_ntosp

#if !defined(_X86AMD64_)

C_ASSERT(((FIELD_OFFSET(KPRCB, LockQueue) + 16) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, PPLookasideList) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, PPNPagedLookasideList) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, PacketBarrier) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, RequestMailbox) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, DpcData) & (128 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, DpcRoutineActive)) & (1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, DpcThread) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, WaitListHead) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, CcFastReadNoWait) & (128 - 1)) == 0);

#endif

// begin_nthal begin_ntosp begin_ntddk

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
// exception list, stack base, stack limit, subsystem TIB, fiber data, and
// the arbitrary user pointer. Therefore, these fields are overlaid with
// other data to get better cache locality.
//
// N.B. The offset to the PRCB in the PCR is fixed for all time.
//

    union {
        NT_TIB NtTib;
        struct {
            union _KGDTENTRY64 *GdtBase;
            struct _KTSS64 *TssBase;
            PVOID PerfGlobalGroupMask;
            struct _KPCR *Self;
            struct _KPRCB *CurrentPrcb;
            PKSPIN_LOCK_QUEUE LockArray;
            PVOID Used_Self;
        };
    };

    union _KIDTENTRY64 *IdtBase;
    ULONG64 Unused[2];
    KIRQL Irql;
    UCHAR SecondLevelCacheAssociativity;
    UCHAR ObsoleteNumber;
    UCHAR Fill0;
    ULONG Unused0[3];
    USHORT MajorVersion;
    USHORT MinorVersion;
    ULONG StallScaleFactor;
    PVOID Unused1[3];
    ULONG KernelReserved[15];
    ULONG SecondLevelCacheSize;
    ULONG HalReserved[16];
    ULONG Unused2;
    PVOID KdVersionBlock;
    PVOID Unused3;
    ULONG PcrAlign1[24];

// end_ntddk

    KPRCB Prcb;

//
// End of the architecturally defined section of the PCR.
//
// end_nthal end_ntosp
//
// N.B. This is the start of the architecturally defined part of the PRCB.
//      The preceding PCR layout cannot change for all time. The initial
//      architecturally defined part of the PRCB cannot change for all time
//      either.
//

// begin_nthal begin_ntddk begin_ntosp

} KPCR, *PKPCR;

// end_nthal end_ntddk end_ntosp

#if !defined (_X86AMD64_)

C_ASSERT(FIELD_OFFSET(KPCR, NtTib.ExceptionList) == FIELD_OFFSET(KPCR, GdtBase));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.StackBase) == FIELD_OFFSET(KPCR, TssBase));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.StackLimit) == FIELD_OFFSET(KPCR, PerfGlobalGroupMask));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.SubSystemTib) == FIELD_OFFSET(KPCR, Self));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.FiberData) == FIELD_OFFSET(KPCR, CurrentPrcb));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.ArbitraryUserPointer) == FIELD_OFFSET(KPCR, LockArray));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.Self) == FIELD_OFFSET(KPCR, Used_Self));
C_ASSERT((FIELD_OFFSET(KPCR, Prcb) == 0x180));
C_ASSERT((FIELD_OFFSET(KPCR, Prcb.CurrentThread) == 0x188));

//
// The offset of the DebuggerDataBlock must not change.
//

C_ASSERT(FIELD_OFFSET(KPCR, KdVersionBlock) == 0x108);

//
// The offset to the PRCB must not change.
//

C_ASSERT(FIELD_OFFSET(KPCR, Prcb) == 0x180);

#endif

__forceinline
ULONG
KeGetContextSwitches (
    PKPRCB Prcb
    )

{
    return Prcb->KeContextSwitches;
}

VOID
KeSaveLegacyFloatingPointState (
    PXMM_SAVE_AREA32 NpxFrame
    );

// begin_nthal begin_ntosp
//
// Define legacy floating status word bit masks.
//

#define FSW_INVALID_OPERATION 0x1
#define FSW_DENORMAL 0x2
#define FSW_ZERO_DIVIDE 0x4
#define FSW_OVERFLOW 0x8
#define FSW_UNDERFLOW 0x10
#define FSW_PRECISION 0x20
#define FSW_STACK_FAULT 0x40
#define FSW_CONDITION_CODE_0 0x100
#define FSW_CONDITION_CODE_1 0x200
#define FSW_CONDITION_CODE_2 0x400
#define FSW_CONDITION_CODE_3 0x4000

#define FSW_ERROR_MASK (FSW_INVALID_OPERATION | FSW_DENORMAL |              \
                        FSW_ZERO_DIVIDE | FSW_OVERFLOW | FSW_UNDERFLOW |    \
                        FSW_PRECISION)

//
// Define legacy floating states.
//
// N.B. The following values cannot be changed because the way in which
//      compares are performed on these values.
//

#define LEGACY_STATE_UNUSED 0
#define LEGACY_STATE_SWITCH 1

//
// Define MxCsr floating control/mode/status word bit masks.
//
// No flush to zero, round to nearest, and all exception masked.
//

#define XSW_INVALID_OPERATION 0x1
#define XSW_DENORMAL 0x2
#define XSW_ZERO_DIVIDE 0x4
#define XSW_OVERFLOW 0x8
#define XSW_UNDERFLOW 0x10
#define XSW_PRECISION 0x20

#define XSW_ERROR_MASK (XSW_INVALID_OPERATION |  XSW_DENORMAL |             \
                        XSW_ZERO_DIVIDE | XSW_OVERFLOW | XSW_UNDERFLOW |    \
                        XSW_PRECISION)

#define XSW_ERROR_SHIFT 7

#define XCW_DAZ 0x40
#define XCW_INVALID_OPERATION 0x80
#define XCW_DENORMAL 0x100
#define XCW_ZERO_DIVIDE 0x200
#define XCW_OVERFLOW 0x400
#define XCW_UNDERFLOW 0x800
#define XCW_PRECISION 0x1000
#define XCW_ROUND_CONTROL 0x6000
#define XCW_FLUSH_ZERO 0x8000

//
// Define EFLAG bit masks and shift offsets.
//

#define EFLAGS_CF_MASK 0x00000001       // carry flag
#define EFLAGS_PF_MASK 0x00000004       // parity flag
#define EFLAGS_AF_MASK 0x00000010       // auxiliary carry flag
#define EFLAGS_ZF_MASK 0x00000040       // zero flag
#define EFLAGS_SF_MASK 0x00000080       // sign flag
#define EFLAGS_TF_MASK 0x00000100       // trap flag
#define EFLAGS_IF_MASK 0x00000200       // interrupt flag
#define EFLAGS_DF_MASK 0x00000400       // direction flag
#define EFLAGS_OF_MASK 0x00000800       // overflow flag
#define EFLAGS_IOPL_MASK 0x00003000     // I/O privilege level
#define EFLAGS_NT_MASK 0x00004000       // nested task
#define EFLAGS_RF_MASK 0x00010000       // resume flag
#define EFLAGS_VM_MASK 0x00020000       // virtual 8086 mode
#define EFLAGS_AC_MASK 0x00040000       // alignment check
#define EFLAGS_VIF_MASK 0x00080000      // virtual interrupt flag
#define EFLAGS_VIP_MASK 0x00100000      // virtual interrupt pending
#define EFLAGS_ID_MASK 0x00200000       // identification flag

#define EFLAGS_TF_SHIFT 8               // trap
#define EFLAGS_IF_SHIFT 9               // interrupt enable

#define EFLAGS_SYSCALL_CLEAR (EFLAGS_IF_MASK | EFLAGS_DF_MASK |              \
                              EFLAGS_TF_MASK | EFLAGS_NT_MASK |              \
                              EFLAGS_RF_MASK)

// end_nthal

// end_ntosp

#if !defined(USER_MODE_CODE)

FORCEINLINE
BOOLEAN
KeDisableInterrupts (
    VOID
    )

/*++

Routine Description:

    This function disables interrupts and returns whether interrupts were
    previously enabled.

Arguments:

    None.

Return Value:

    TRUE is returned if interrupts were previously enabled. Otherwise, FALSE
    is returned.

--*/

{

    ULONG Flags;

    Flags = GetCallersEflags();
    _disable();
    return (BOOLEAN)((Flags >> EFLAGS_IF_SHIFT) & 1);
}

#endif

// begin_ntosp

//
// Define sanitize EFLAGS macro.
//
// If kernel mode, then
//      caller can specify Carry, Parity, AuxCarry, Zero, Sign, Trap,
//      Interrupt, Direction, Overflow, and identification.
//
// If user mode, then
//      caller can specify Carry, Parity, AuxCarry, Zero, Sign, Trap,
//      Interrupt, Direction, Overflow, identification, but Interrupt
//      will always be forced on.
//

#define EFLAGS_SANITIZE 0x00210fd5L

#define SANITIZE_EFLAGS(eFlags, mode) (                                      \
    ((mode) == KernelMode ?                                                  \
        ((eFlags) & EFLAGS_SANITIZE) :                                       \
        (((eFlags) & EFLAGS_SANITIZE) | EFLAGS_IF_MASK)))

//
// Define sanitize debug register macros.
//
// Define control register settable bits and active mask.
//

#define DR7_LEGAL 0xffff0355
#define DR7_ACTIVE 0x0355
#define DR7_TRACE_BRANCH 0x200
#define DR7_LAST_BRANCH 0x100

//
// Define macro to sanitize the debug control register.
//

#define SANITIZE_DR7(Dr7, mode) ((Dr7) & DR7_LEGAL)

//
// Define macro to sanitize debug address registers.
//

#define SANITIZE_DRADDR(DrReg, mode)                                         \
    ((mode) == KernelMode ?                                                  \
        (DrReg) :                                                            \
        (((PVOID)(DrReg) <= MM_HIGHEST_USER_ADDRESS) ? (DrReg) : 0))                                 \

//
// Define macro to clear reserved bits from MXCSR.
//

#define SANITIZE_MXCSR(_mxcsr_) ((_mxcsr_) & KiMxCsrMask)

//
// Define macro to clear reserved bits for legacy FP control word.
//

#define SANITIZE_FCW(_fcw_) ((_fcw_) & 0x1f3f)

//
// Structure of AMD cache information returned by CPUID instruction
//

typedef union _AMD_L1_CACHE_INFO {
    ULONG Ulong;
    struct {
        UCHAR LineSize;
        UCHAR LinesPerTag;
        UCHAR Associativity;
        UCHAR Size;
    };
} AMD_L1_CACHE_INFO, *PAMD_L1_CACHE_INFO;

typedef union _AMD_L2_CACHE_INFO {
    ULONG Ulong;
    struct {
        UCHAR  LineSize;
        UCHAR  LinesPerTag   : 4;
        UCHAR  Associativity : 4;
        USHORT Size;
    };
} AMD_L2_CACHE_INFO, *PAMD_L2_CACHE_INFO;

//
// Structure of Intel deterministic cache information returned by
// CPUID instruction
//

typedef enum _INTEL_CACHE_TYPE {
    IntelCacheNull,
    IntelCacheData,
    IntelCacheInstruction,
    IntelCacheUnified,
    IntelCacheRam,
    IntelCacheTrace
} INTEL_CACHE_TYPE;

typedef union INTEL_CACHE_INFO_EAX {
    ULONG Ulong;
    struct {
        INTEL_CACHE_TYPE Type : 5;
        ULONG Level : 3;
        ULONG SelfInitializing : 1;
        ULONG FullyAssociative : 1;
        ULONG Reserved : 4;
        ULONG ThreadsSharing : 12;
        ULONG ProcessorCores : 6;
    };
} INTEL_CACHE_INFO_EAX, *PINTEL_CACHE_INFO_EAX;

typedef union INTEL_CACHE_INFO_EBX {
    ULONG Ulong;
    struct {
        ULONG LineSize      : 12;
        ULONG Partitions    : 10;
        ULONG Associativity : 10;
    };
} INTEL_CACHE_INFO_EBX, *PINTEL_CACHE_INFO_EBX;

// end_ntosp
//
// Define macro to sign extend a specified bit.
// 

#define SIGN_EXTEND_BIT(_va_, _bit_) \
    (ULONG64)(((LONG64)(_va_) << (64 - (_bit_))) >> (64 - (_bit_)))

//
// Define routine to sanitize a virtual address based on previous mode and
// the specified segment selector.
//

FORCEINLINE
ULONG64
SANITIZE_VA (
    IN ULONG64 VirtualAddress,
    IN USHORT Segment,
    IN KPROCESSOR_MODE PreviousMode
    )

/*++

Routine Description:

    This routine canonicalizes a 64-bit virtual address according to the
    supplied segment selector.

Arguments:

    VirtualAddress - Supplies the 64-bit virtual address to canonicalize.

    Segment - Supplies the selector for for the virtual address.

    PreviousMode - Supplies the processor mode for which the exception and
        trap frames are being built.

Return Value:

    Returns the canonicalized virtual address.

--*/

{

    ULONG64 Va;

    if (PreviousMode == UserMode) {

        //
        // Zero-extend 32-bit addresses, sign extend bit 48 of 64-bit
        // addresses.
        // 

        if ((Segment == (KGDT64_R3_CMCODE | RPL_MASK)) ||
            (Segment == (KGDT64_R3_DATA | RPL_MASK))) {

            Va = (ULONG)VirtualAddress;

        } else {
            Va = SIGN_EXTEND_BIT(VirtualAddress, 48);
        }

    } else {
        Va = VirtualAddress;
    }

    return Va;
}

// begin_nthal begin_ntddk begin_ntosp
//
// Exception frame
//
//  This frame is established when handling an exception. It provides a place
//  to save all nonvolatile registers. The volatile registers will already
//  have been saved in a trap frame.
//
// N.B. The exception frame has a built in exception record capable of
//      storing information for four parameter values. This exception
//      record is used exclusively within the trap handling code.
//

#define EXCEPTION_AREA_SIZE 64

typedef struct _KEXCEPTION_FRAME {

//
// Home address for the parameter registers.
//

    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5;

//
// Kernel callout initial stack value.
//

    ULONG64 InitialStack;

//
// Saved nonvolatile floating registers.
//

    M128A Xmm6;
    M128A Xmm7;
    M128A Xmm8;
    M128A Xmm9;
    M128A Xmm10;
    M128A Xmm11;
    M128A Xmm12;
    M128A Xmm13;
    M128A Xmm14;
    M128A Xmm15;

//
// Kernel callout frame variables.
//

    ULONG64 TrapFrame;
    ULONG64 CallbackStack;
    ULONG64 OutputBuffer;
    ULONG64 OutputLength;

//
// Exception record for exceptions.
//

    UCHAR ExceptionRecord[EXCEPTION_AREA_SIZE];

//
// Saved MXCSR when a thread is interrupted in kernel mode via a dispatch
// interrupt.
//

    ULONG64 MxCsr;

//
// Saved nonvolatile register - not always saved.
//

    ULONG64 Rbp;

//
// Saved nonvolatile registers.
//

    ULONG64 Rbx;
    ULONG64 Rdi;
    ULONG64 Rsi;
    ULONG64 R12;
    ULONG64 R13;
    ULONG64 R14;
    ULONG64 R15;

//
// EFLAGS and return address.
//

    ULONG64 Return;
} KEXCEPTION_FRAME, *PKEXCEPTION_FRAME;

// end_ntddk

#define KEXCEPTION_FRAME_LENGTH sizeof(KEXCEPTION_FRAME)

C_ASSERT((sizeof(KEXCEPTION_FRAME) & STACK_ROUND) == 0);

#define EXCEPTION_RECORD_LENGTH                                              \
    ((sizeof(EXCEPTION_RECORD) + STACK_ROUND) & ~STACK_ROUND)

#if !defined(_X86AMD64_)

C_ASSERT(EXCEPTION_AREA_SIZE == (FIELD_OFFSET(EXCEPTION_RECORD, ExceptionInformation) + (4 * sizeof(ULONG_PTR))));

#endif

//
// Machine Frame
//
// This frame is established by code that trampolines to user mode (e.g. user
// APC, user callback, dispatch user exception, etc.). The purpose of this
// frame is to allow unwinding through these callbacks if an exception occurs.
//
// N.B. This frame is identical to the frame that is pushed for a trap without
//      an error code and is identical to the hardware part of a trap frame.
//

typedef struct _MACHINE_FRAME {
    ULONG64 Rip;
    USHORT SegCs;
    USHORT Fill1[3];
    ULONG EFlags;
    ULONG Fill2;
    ULONG64 Rsp;
    USHORT SegSs;
    USHORT Fill3[3];
} MACHINE_FRAME, *PMACHINE_FRAME;

#define MACHINE_FRAME_LENGTH sizeof(MACHINE_FRAME)

C_ASSERT((sizeof(MACHINE_FRAME) & STACK_ROUND) == 8);

//
// Switch Frame
//
// This frame is established by the code that switches context from one
// thread to the next and is used by the thread initialization code to
// construct a stack that will start the execution of a thread in the
// thread start up code.
//

typedef struct _KSWITCH_FRAME {
    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5Home;
    KIRQL ApcBypass;
    UCHAR Fill1[7];
    ULONG64 Rbp;
    ULONG64 Return;
} KSWITCH_FRAME, *PKSWITCH_FRAME;

#define KSWITCH_FRAME_LENGTH sizeof(KSWITCH_FRAME)

C_ASSERT((sizeof(KSWITCH_FRAME) & STACK_ROUND) == 0);

//
// Start system thread frame.
//
// This frame is established by the AMD64 specific thread initialization
// code. It is used to store the initial context for starting a system
// thread.
//

typedef struct _KSTART_FRAME {
    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 Reserved;
    ULONG64 Return;
} KSTART_FRAME, *PKSTART_FRAME;

#define KSTART_FRAME_LENGTH sizeof(KSTART_FRAME)

C_ASSERT((sizeof(KSTART_FRAME) & STACK_ROUND) == 0);

// begin_ntddk
//
// Trap frame
//
// This frame is established when handling a trap. It provides a place to
// save all volatile registers. The nonvolatile registers are saved in an
// exception frame or through the normal C calling conventions for saved
// registers.
//

typedef struct _KTRAP_FRAME {

//
// Home address for the parameter registers.
//

    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5;

//
// Previous processor mode (system services only) and previous IRQL
// (interrupts only).
//

    KPROCESSOR_MODE PreviousMode;
    KIRQL PreviousIrql;

//
// Page fault load/store indicator.
//

    UCHAR FaultIndicator;

//
// Exception active indicator.
//
//    0 - interrupt frame.
//    1 - exception frame.
//    2 - service frame.
//

    UCHAR ExceptionActive;

//
// Floating point state.
//

    ULONG MxCsr;

//
//  Volatile registers.
//
// N.B. These registers are only saved on exceptions and interrupts. They
//      are not saved for system calls.
//

    ULONG64 Rax;
    ULONG64 Rcx;
    ULONG64 Rdx;
    ULONG64 R8;
    ULONG64 R9;
    ULONG64 R10;
    ULONG64 R11;

//
// Gsbase is only used if the previous mode was kernel.
//
// GsSwap is only used if the previous mode was user.
//

    union {
        ULONG64 GsBase;
        ULONG64 GsSwap;
    };

//
// Volatile floating registers.
//
// N.B. These registers are only saved on exceptions and interrupts. They
//      are not saved for system calls.
//

    M128A Xmm0;
    M128A Xmm1;
    M128A Xmm2;
    M128A Xmm3;
    M128A Xmm4;
    M128A Xmm5;

//
// Page fault address or context record address if user APC bypass.
//

    union {
        ULONG64 FaultAddress;
        ULONG64 ContextRecord;
        ULONG64 TimeStamp;
    };

//
//  Debug registers.
//

    ULONG64 Dr0;
    ULONG64 Dr1;
    ULONG64 Dr2;
    ULONG64 Dr3;
    ULONG64 Dr6;
    ULONG64 Dr7;

//
// Special debug registers.
//
// N.B. Either AMD64 or EM64T information is stored in the following locations.

    union {
        struct {
            ULONG64 DebugControl;
            ULONG64 LastBranchToRip;
            ULONG64 LastBranchFromRip;
            ULONG64 LastExceptionToRip;
            ULONG64 LastExceptionFromRip;
        };

        struct {
            ULONG64 LastBranchControl;
            ULONG LastBranchMSR;
        };
    };

//
//  Segment registers
//

    USHORT SegDs;
    USHORT SegEs;
    USHORT SegFs;
    USHORT SegGs;

//
// Previous trap frame address.
//

    ULONG64 TrapFrame;

//
// Saved nonvolatile registers RBX, RDI and RSI. These registers are only
// saved in system service trap frames.
//

    ULONG64 Rbx;
    ULONG64 Rdi;
    ULONG64 Rsi;

//
// Saved nonvolatile register RBP. This register is used as a frame
// pointer during trap processing and is saved in all trap frames.
//

    ULONG64 Rbp;

//
// Information pushed by hardware.
//
// N.B. The error code is not always pushed by hardware. For those cases
//      where it is not pushed by hardware a dummy error code is allocated
//      on the stack.
//

    union {
        ULONG64 ErrorCode;
        ULONG64 ExceptionFrame;
    };

    ULONG64 Rip;
    USHORT SegCs;
    USHORT Fill1[3];
    ULONG EFlags;
    ULONG Fill2;
    ULONG64 Rsp;
    USHORT SegSs;
    USHORT Fill3[1];

//
// Copy of the global patch cycle at the time of the fault. Filled in by the
// invalid opcode and general protection fault routines.
//

    LONG CodePatchCycle;
} KTRAP_FRAME, *PKTRAP_FRAME;

// end_ntddk

#define KTRAP_FRAME_LENGTH sizeof(KTRAP_FRAME)

C_ASSERT((sizeof(KTRAP_FRAME) & STACK_ROUND) == 0);

//
// Profile, update run time, and update system time interrupt routines.
//

NTKERNELAPI
VOID
KeProfileInterruptWithSource (
    IN PKTRAP_FRAME TrapFrame,
    IN KPROFILE_SOURCE ProfileSource
    );

NTKERNELAPI
VOID
KeUpdateRunTime (
    IN PKTRAP_FRAME TrapFrame,
    IN LONG Increment
    );

NTKERNELAPI
VOID
KeUpdateSystemTime (
    IN PKTRAP_FRAME TrapFrame,
    IN ULONG64 Increment
    );

// end_nthal

//
// The frame saved by the call out to user mode code is defined here to allow
// the kernel debugger to trace the entire kernel stack when user mode callouts
// are active.
//
// N.B. The kernel callout frame is the same as an exception frame.
//

typedef KEXCEPTION_FRAME KCALLOUT_FRAME;
typedef PKEXCEPTION_FRAME PKCALLOUT_FRAME;

typedef struct _UCALLOUT_FRAME {
    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    PVOID Buffer;
    ULONG Length;
    ULONG ApiNumber;
    MACHINE_FRAME MachineFrame;
} UCALLOUT_FRAME, *PUCALLOUT_FRAME;

#define UCALLOUT_FRAME_LENGTH sizeof(UCALLOUT_FRAME)

C_ASSERT((sizeof(UCALLOUT_FRAME) & STACK_ROUND) == 8);

// begin_ntddk begin_wdm
//
// Dummy nonvolatile floating state structure.
//

typedef struct _KFLOATING_SAVE {
    ULONG Dummy;
} KFLOATING_SAVE, *PKFLOATING_SAVE;

// end_ntddk end_wdm end_ntosp

//
// Define profile values.
//

#define DEFAULT_PROFILE_INTERVAL  39063

//
// The minimum acceptable profiling interval is set to 1221 which is the
// fast RTC clock rate we can get.  If this
// value is too small, the system will run very slowly.
//

#define MINIMUM_PROFILE_INTERVAL   1221

// begin_ntddk begin_wdm begin_nthal begin_ntndis begin_ntosp
//
// AMD64 Specific portions of mm component.
//
// Define the page size for the AMD64 as 4096 (0x1000).
//

#define PAGE_SIZE 0x1000

//
// Define the number of trailing zeroes in a page aligned virtual address.
// This is used as the shift count when shifting virtual addresses to
// virtual page numbers.
//

#define PAGE_SHIFT 12L

// end_ntndis end_wdm

#define PXE_BASE          0xFFFFF6FB7DBED000UI64
#define PXE_SELFMAP       0xFFFFF6FB7DBEDF68UI64
#define PPE_BASE          0xFFFFF6FB7DA00000UI64
#define PDE_BASE          0xFFFFF6FB40000000UI64
#define PTE_BASE          0xFFFFF68000000000UI64

#define PXE_TOP           0xFFFFF6FB7DBEDFFFUI64
#define PPE_TOP           0xFFFFF6FB7DBFFFFFUI64
#define PDE_TOP           0xFFFFF6FB7FFFFFFFUI64
#define PTE_TOP           0xFFFFF6FFFFFFFFFFUI64

#define PDE_KTBASE_AMD64  PPE_BASE

#define PTI_SHIFT 12
#define PDI_SHIFT 21
#define PPI_SHIFT 30
#define PXI_SHIFT 39

#define PTE_PER_PAGE 512
#define PDE_PER_PAGE 512
#define PPE_PER_PAGE 512
#define PXE_PER_PAGE 512

#define PTI_MASK_AMD64 (PTE_PER_PAGE - 1)
#define PDI_MASK_AMD64 (PDE_PER_PAGE - 1)
#define PPI_MASK (PPE_PER_PAGE - 1)
#define PXI_MASK (PXE_PER_PAGE - 1)

#define GUARD_PAGE_SIZE (PAGE_SIZE * 2)

//
// Define the last branch control MSR address.
//

extern NTKERNELAPI ULONG KeLastBranchMSR;

//
// Define the highest user address and user probe address.
//

extern NTKERNELAPI PVOID MmHighestUserAddress;
extern NTKERNELAPI PVOID MmSystemRangeStart;
extern NTKERNELAPI ULONG64 MmUserProbeAddress;

#define MM_HIGHEST_USER_ADDRESS MmHighestUserAddress
#define MM_SYSTEM_RANGE_START MmSystemRangeStart

//
// Allow non-kernel components to capture the user probe address and use a
// local copy for efficiency.
//

#if defined(_LOCAL_COPY_USER_PROBE_ADDRESS_)

#define MM_USER_PROBE_ADDRESS _LOCAL_COPY_USER_PROBE_ADDRESS_

extern ULONG64 _LOCAL_COPY_USER_PROBE_ADDRESS_;

#else

#define MM_USER_PROBE_ADDRESS MmUserProbeAddress

#endif

// end_ntddk end_nthal end_ntosp

#define MI_HIGHEST_USER_ADDRESS (PVOID) (ULONG_PTR)((0x80000000000 - 0x10000 - 1)) // highest user address
#define MI_SYSTEM_RANGE_START (PVOID)(0xFFFF080000000000) // start of system space
#define MI_USER_PROBE_ADDRESS ((ULONG_PTR)(0x80000000000UI64 - 0x10000)) // starting address of guard page

#define MM_KSEG0_BASE  0xFFFFF80000000000UI64
#define MM_SYSTEM_SPACE_END 0xFFFFFFFFFFFFFFFFUI64

// begin_nthal
//
// 4MB at the top of VA space is reserved for the HAL's use.
//

#define HAL_VA_START 0xFFFFFFFFFFC00000UI64
#define HAL_VA_SIZE  (4 * 1024 * 1024)

// end_nthal

// begin_ntddk begin_nthal begin_ntosp
//
// The lowest user address reserves the low 64k.
//

#define MM_LOWEST_USER_ADDRESS (PVOID)0x10000

//
// The lowest address for system space.
//

#define MM_LOWEST_SYSTEM_ADDRESS (PVOID)0xFFFF080000000000

// begin_wdm

#define MmGetProcedureAddress(Address) (Address)
#define MmLockPageableCodeSection(Address) MmLockPageableDataSection(Address)
#define MmLockPagableCodeSection(Address) MmLockPageableDataSection(Address)
#define MmLockPagableDataSection(Address) MmLockPageableDataSection(Address)

// end_ntddk end_wdm end_ntosp

//
// Define virtual base and alternate virtual base of kernel.
//

#define KSEG0_BASE 0xFFFFF80000000000UI64

//
// Generate kernel segment physical address.
//

#define KSEG_ADDRESS(PAGE) ((PVOID)(KSEG0_BASE | ((ULONG_PTR)(PAGE) << PAGE_SHIFT)))


// begin_ntddk begin_ntosp

//
// Intrinsic functions
//

// begin_wdm

#if defined(_M_AMD64) && !defined(RC_INVOKED)  && !defined(MIDL_PASS)

// end_wdm

//
// The following routines are provided for backward compatibility with old
// code. They are no longer the preferred way to accomplish these functions.
//

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInterlockedIncrementLong)      // Use InterlockedIncrement
#pragma deprecated(ExInterlockedDecrementLong)      // Use InterlockedDecrement
#pragma deprecated(ExInterlockedExchangeUlong)      // Use InterlockedExchange
#endif

#define RESULT_ZERO 0
#define RESULT_NEGATIVE 1
#define RESULT_POSITIVE 2

typedef enum _INTERLOCKED_RESULT {
    ResultNegative = RESULT_NEGATIVE,
    ResultZero = RESULT_ZERO,
    ResultPositive = RESULT_POSITIVE
} INTERLOCKED_RESULT;

#define ExInterlockedDecrementLong(Addend, Lock)                            \
    _ExInterlockedDecrementLong(Addend)

__forceinline
LONG
_ExInterlockedDecrementLong (
    IN OUT PLONG Addend
    )

{

    LONG Result;

    Result = InterlockedDecrement(Addend);
    if (Result < 0) {
        return ResultNegative;

    } else if (Result > 0) {
        return ResultPositive;

    } else {
        return ResultZero;
    }
}

#define ExInterlockedIncrementLong(Addend, Lock)                            \
    _ExInterlockedIncrementLong(Addend)

__forceinline
LONG
_ExInterlockedIncrementLong (
    IN OUT PLONG Addend
    )

{

    LONG Result;

    Result = InterlockedIncrement(Addend);
    if (Result < 0) {
        return ResultNegative;

    } else if (Result > 0) {
        return ResultPositive;

    } else {
        return ResultZero;
    }
}

#define ExInterlockedExchangeUlong(Target, Value, Lock)                     \
    _ExInterlockedExchangeUlong(Target, Value)

__forceinline
_ExInterlockedExchangeUlong (
    IN OUT PULONG Target,
    IN ULONG Value
    )

{

    return (ULONG)InterlockedExchange((PLONG)Target, (LONG)Value);
}

// begin_wdm

#endif // defined(_M_AMD64) && !defined(RC_INVOKED)  && !defined(MIDL_PASS)

// end_wdm end_ntddk end_nthal end_ntosp

// begin_ntosp begin_nthal begin_ntddk begin_wdm

#if !defined(MIDL_PASS) && defined(_M_AMD64)

//
// AMD646 function prototype definitions
//

// end_wdm

// end_ntddk end_ntosp

//
// Get address of current processor block.
//

__forceinline
PKPCR
KeGetPcr (
    VOID
    )

{
    return (PKPCR)__readgsqword(FIELD_OFFSET(KPCR, Self));
}

// end_nthal
//
// Get current node shifted color.
//

__forceinline
ULONG
KeGetCurrentNodeShiftedColor (
    VOID
    )

{
    return __readgsdword(FIELD_OFFSET(KPCR, Prcb.NodeShiftedColor));
}

// begin_nthal begin_ntosp
//
// Get address of current processor block.
//

__forceinline
PKPRCB
KeGetCurrentPrcb (
    VOID
    )

{

    return (PKPRCB)__readgsqword(FIELD_OFFSET(KPCR, CurrentPrcb));
}

// begin_ntddk

//
// Get the current processor number
//

__forceinline
ULONG
KeGetCurrentProcessorNumber (
    VOID
    )

{

    return (ULONG)__readgsbyte(0x184);
}

// end_ntddk

NTKERNELAPI
PKPRCB
KeQueryPrcbAddress (
    __in ULONG Number
    );

// end_nthal end_ntosp

//
// N.B. The current processor number is stored in the architecturally defined
//      region of the PRCB. The offset cannot change for all time.
//

#if !defined (_X86AMD64_)

C_ASSERT(FIELD_OFFSET(KPCR, Prcb.Number) == 0x184);

#endif

//
// Get address of current kernel thread object.
//
//

__forceinline
struct _KTHREAD *
KeGetCurrentThread (
    VOID
    )

{
    return (struct _KTHREAD *)__readgsqword(FIELD_OFFSET(KPCR, Prcb.CurrentThread));
}

//
// Is the current processor executing a DPC (either a threaded DPC or a
// legacy DPC).
//

__forceinline
ULONG
KeIsExecutingDpc (
    VOID
    )

{
    return (__readgsword(FIELD_OFFSET(KPCR, Prcb.DpcRoutineActive)) != 0);
}

//
// Is the current processor executing a legacy DPC.
//

__forceinline
ULONG
KeIsExecutingLegacyDpc (
    VOID
    )

{
    return (__readgsbyte(FIELD_OFFSET(KPCR, Prcb.DpcRoutineActive)) != 0);
}

//
// Get current DPC stack base.
//

__forceinline
ULONG64
KeGetDpcStackBase (
    VOID
    )

{
    return __readgsqword(FIELD_OFFSET(KPCR, Prcb.DpcStack));
}

// begin_nthal begin_ntddk begin_ntosp

// begin_wdm

#endif // !defined(MIDL_PASS) && defined(_M_AMD64)

// end_nthal end_ntddk end_wdm end_ntosp

#define KeIsIdleHaltSet(Prcb, Number) (((Prcb)->IdleHalt != 0) &&                   \
                                       ((Prcb)->Sleeping != 0))

// begin_ntddk begin_nthal begin_ntndis begin_wdm begin_ntosp

//++
//
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

#if !defined(_CROSS_PLATFORM_)

FORCEINLINE
VOID
KeMemoryBarrier (
    VOID
    )
{
    FastFence();
    return;
}

#define KeMemoryBarrierWithoutFence() _ReadWriteBarrier()

#else

#define KeMemoryBarrier()
#define KeMemoryBarrierWithoutFence()

#endif

// end_ntddk end_nthal end_ntndis end_wdm end_ntosp

//
// Request a software interrupt.
//

NTHALAPI
VOID
FASTCALL
HalRequestSoftwareInterrupt (
    KIRQL RequestIrql
    );

//
// Send an NMI interrupt to a set of processors.
//

NTHALAPI
VOID
FASTCALL
HalSendNMI (
    KAFFINITY Affinity
    );

NTHALAPI
VOID
FASTCALL
HalSendSoftwareInterrupt (
    KAFFINITY Affinity,
    KIRQL RequestIrql
    );

FORCEINLINE
VOID
KiRequestSoftwareInterrupt (
    KIRQL RequestIrql
    )
{
    PKPRCB Prcb;

    if (RequestIrql == DISPATCH_LEVEL) {
        Prcb = KeGetCurrentPrcb();
        if (Prcb->NestingLevel != 0) {
            Prcb->InterruptRequest = TRUE;
            return;
        }
    }

    HalRequestSoftwareInterrupt(RequestIrql);
    return;
}

// begin_nthal
//
// Define inline functions to get and set the handler address in and IDT
// entry.
//

typedef union _KIDT_HANDLER_ADDRESS {
    struct {
        USHORT OffsetLow;
        USHORT OffsetMiddle;
        ULONG OffsetHigh;
    };

    ULONG64 Address;
} KIDT_HANDLER_ADDRESS, *PKIDT_HANDLER_ADDRESS;

#define KiGetIdtFromVector(Vector)                  \
    &KeGetPcr()->IdtBase[HalVectorToIDTEntry(Vector)]

#define KeGetIdtHandlerAddress(Vector,Addr) {       \
    KIDT_HANDLER_ADDRESS Handler;                   \
    PKIDTENTRY64 Idt;                               \
                                                    \
    Idt = KiGetIdtFromVector(Vector);               \
    Handler.OffsetLow = Idt->OffsetLow;             \
    Handler.OffsetMiddle = Idt->OffsetMiddle;       \
    Handler.OffsetHigh = Idt->OffsetHigh;           \
    *(Addr) = (PVOID)(Handler.Address);             \
}

#define KeSetIdtHandlerAddress(Vector,Addr) {      \
    KIDT_HANDLER_ADDRESS Handler;                  \
    PKIDTENTRY64 Idt;                              \
                                                   \
    Idt = KiGetIdtFromVector(Vector);              \
    Handler.Address = (ULONG64)(Addr);             \
    Idt->OffsetLow = Handler.OffsetLow;            \
    Idt->OffsetMiddle = Handler.OffsetMiddle;      \
    Idt->OffsetHigh = Handler.OffsetHigh;          \
}


// end_nthal

//++
//
// BOOLEAN
// KiIsThreadNumericStateSaved(
//     IN PKTHREAD Address
//     )
//
//--

#define KiIsThreadNumericStateSaved(a) TRUE

//++
//
// VOID
// KiRundownThread(
//     IN PKTHREAD Address
//     )
//
//--

#define KiRundownThread(a)

//
// Legacy floating save restore functions.
//

// begin_ntddk begin_wdm begin_ntosp

__forceinline
NTSTATUS
KeSaveFloatingPointState (
    __out PVOID FloatingState
    )

{

    UNREFERENCED_PARAMETER(FloatingState);

    return STATUS_SUCCESS;
}

__forceinline
NTSTATUS
KeRestoreFloatingPointState (
    __in PVOID FloatingState
    )

{

    UNREFERENCED_PARAMETER(FloatingState);

    return STATUS_SUCCESS;
}

// end_ntddk end_wdm end_ntosp

// begin_nthal begin_ntddk begin_wdm begin_ntndis begin_ntosp

#endif // defined(_AMD64_)

// end_nthal end_ntddk end_wdm end_ntndis end_ntosp

//
// Architecture specific kernel functions.
//

// begin_ntosp begin_nthal begin_ntddk begin_wdm

//
// Platform specific kernel functions to raise and lower IRQL.
//


#if defined(_AMD64_) && !defined(MIDL_PASS)

__forceinline
KIRQL
KeGetCurrentIrql (
    VOID
    )

/*++

Routine Description:

    This function return the current IRQL.

Arguments:

    None.

Return Value:

    The current IRQL is returned as the function value.

--*/

{

    return (KIRQL)ReadCR8();
}

__forceinline
VOID
KeLowerIrql (
   __in KIRQL NewIrql
   )

/*++

Routine Description:

    This function lowers the IRQL to the specified value.

Arguments:

    NewIrql  - Supplies the new IRQL value.

Return Value:

    None.

--*/

{

    ASSERT(KeGetCurrentIrql() >= NewIrql);

    WriteCR8(NewIrql);
    return;
}

#define KeRaiseIrql(a,b) *(b) = KfRaiseIrql(a)

__forceinline
KIRQL
KfRaiseIrql (
    __in KIRQL NewIrql
    )

/*++

Routine Description:

    This function raises the current IRQL to the specified value and returns
    the previous IRQL.

Arguments:

    NewIrql (cl) - Supplies the new IRQL value.

Return Value:

    The previous IRQL is returned as the function value.

--*/

{

    KIRQL OldIrql;

    OldIrql = KeGetCurrentIrql();

    ASSERT(OldIrql <= NewIrql);

    WriteCR8(NewIrql);
    return OldIrql;
}

// end_wdm

__forceinline
KIRQL
KeRaiseIrqlToDpcLevel (
    VOID
    )

/*++

Routine Description:

    This function raises the current IRQL to DPC_LEVEL and returns the
    previous IRQL.

Arguments:

    None.

Return Value:

    The previous IRQL is returned as the function value.

--*/

{

    return KfRaiseIrql(DISPATCH_LEVEL);
}

__forceinline
KIRQL
KeRaiseIrqlToSynchLevel (
    VOID
    )

/*++

Routine Description:

    This function raises the current IRQL to SYNCH_LEVEL and returns the
    previous IRQL.

Arguments:

Return Value:

    The previous IRQL is returned as the function value.

--*/

{

    return KfRaiseIrql(SYNCH_LEVEL);
}

// begin_wdm

#endif // defined(_AMD64_) && !defined(MIDL_PASS)

// end_ntosp end_nthal end_ntddk end_wdm

//
// misc routines
//

VOID
KeOptimizeProcessorControlState (
    VOID
    );

// begin_nthal

#if defined(_AMD64_)

//
// Structure to aid in booting secondary processors
//

#pragma pack(push,2)

typedef struct _FAR_JMP_16 {
    UCHAR  OpCode;  // = 0xe9
    USHORT Offset;
} FAR_JMP_16;

typedef struct _FAR_TARGET_32 {
    ULONG Offset;
    USHORT Selector;
} FAR_TARGET_32;

typedef struct _PSEUDO_DESCRIPTOR_32 {
    USHORT Limit;
    ULONG Base;
} PSEUDO_DESCRIPTOR_32;

#pragma pack(pop)

#define PSB_GDT32_NULL      0 * 16
#define PSB_GDT32_CODE64    1 * 16
#define PSB_GDT32_DATA32    2 * 16
#define PSB_GDT32_CODE32    3 * 16
#define PSB_GDT32_MAX       3

typedef struct _PROCESSOR_START_BLOCK *PPROCESSOR_START_BLOCK;
typedef struct _PROCESSOR_START_BLOCK {

    //
    // The block starts with a jmp instruction to the end of the block
    //

    FAR_JMP_16 Jmp;

    //
    // Completion flag is set to non-zero when the target processor has
    // started
    //

    ULONG CompletionFlag;

    //
    // Pseudo descriptors for GDT and IDT.
    //

    PSEUDO_DESCRIPTOR_32 Gdt32;
    PSEUDO_DESCRIPTOR_32 Idt32;

    //
    // The temporary 32-bit GDT itself resides here.
    //

    KGDTENTRY64 Gdt[PSB_GDT32_MAX + 1];

    //
    // Physical address of the 64-bit top-level identity-mapped page table.
    //

    ULONG64 TiledCr3;

    //
    // Far jump target from Rm to Pm code
    //

    FAR_TARGET_32 PmTarget;

    //
    // Far jump target from Pm to Lm code
    //

    FAR_TARGET_32 LmIdentityTarget;

    //
    // Address of LmTarget
    //

    PVOID LmTarget;

    //
    // Linear address of this structure
    //

    PPROCESSOR_START_BLOCK SelfMap;

    //
    // Contents of the PAT msr
    //

    ULONG64 MsrPat;

    //
    // Contents of the EFER msr
    //

    ULONG64 MsrEFER;

    //
    // Initial processor state for the processor to be started
    //

    KPROCESSOR_STATE ProcessorState;

} PROCESSOR_START_BLOCK;

//
// AMD64 functions for special instructions
//

typedef struct _CPU_INFO {
    ULONG Eax;
    ULONG Ebx;
    ULONG Ecx;
    ULONG Edx;
} CPU_INFO, *PCPU_INFO;

NTKERNELAPI
VOID
KiCpuId (
    IN ULONG Function,
    IN ULONG Index,
    OUT PCPU_INFO CpuInfo
    );

//
// Define read/write MSR functions and register definitions.
//

#define MSR_TSC 0x10                    // time stamp counter
#define MSR_BIOS_SIGN 0x8B              // microcode signature
#define MSR_PAT 0x277                   // page attributes table
#define MSR_MCG_CAP 0x179               // machine check capabilities
#define MSR_MCG_STATUS 0x17a            // machine check status
#define MSR_MCG_CTL 0x17b               // machine check control
#define MSR_MC0_CTL 0x400               // machine check control, status,
#define MSR_MC0_STATUS 0x401            //   address, and miscellaneous
#define MSR_MC0_ADDR 0x402              //   registers for machine check
#define MSR_MC0_MISC 0x403              //   sources
#define MSR_EFER 0xc0000080             // extended function enable register
#define MSR_STAR 0xc0000081             // system call selectors
#define MSR_LSTAR 0xc0000082            // system call 64-bit entry
#define MSR_CSTAR 0xc0000083            // system call 32-bit entry
#define MSR_SYSCALL_MASK 0xc0000084     // system call flags mask
#define MSR_FS_BASE 0xc0000100          // fs long mode base address register
#define MSR_GS_BASE 0xc0000101          // gs long mode base address register
#define MSR_GS_SWAP 0xc0000102          // gs long mode swap GS base register

//
// Define AMD specific debug control registers.
//

#define MSR_DEGUG_CTL 0x1d9             // debug control
#define MSR_LAST_BRANCH_FROM 0x1db      // last branch from RIP
#define MSR_LAST_BRANCH_TO 0x1dc        // last branch to RIP
#define MSR_LAST_EXCEPTION_FROM 0x1dd   // last exception from RIP
#define MSR_LAST_EXCEPTION_TO 0x1de     // last exception

//
// Flags within MSR_DEBUG_CTL.
//

#define MSR_DEBUG_CTL_LBR 0x1           // last branch/exception record
#define MSR_DEBUG_CRL_BTF 0x2           // branch trace control

//
// Define AMD specific performance event selection/counter registers.
//

#define MSR_PERF_EVT_SEL0 0xc0010000    // performance event select registers
#define MSR_PERF_EVT_SEL1 0xc0010001    // 
#define MSR_PERF_EVT_SEL2 0xc0010002    // 
#define MSR_PERF_EVT_SEL3 0xc0010003    //
#define MSR_PERF_CTR0 0xc0010004        // performance counter registers
#define MSR_PERF_CTR1 0xc0010005        //
#define MSR_PERF_CTR2 0xc0010006        //
#define MSR_PERF_CTR3 0xc0010007        //

//
// Define Intel specific performance event selection/control/counter registers.
//

#define MSR_LAST_BRANCH     0x1d9       // last branch control 
#define MSR_BPU_COUNTER0    0x300       // performance counter registers
#define MSR_BPU_COUNTER1    0x301       //
#define MSR_BPU_COUNTER2    0x302       //
#define MSR_BPU_COUNTER3    0x303       //
#define MSR_MS_COUNTER0     0x304       //
#define MSR_MS_COUNTER1     0x305       //
#define MSR_MS_COUNTER2     0x306       //
#define MSR_MS_COUNTER3     0x307       //
#define MSR_FLAME_COUNTER0  0x308       //
#define MSR_FLAME_COUNTER1  0x309       //
#define MSR_FLAME_COUNTER2  0x30a       //
#define MSR_FLAME_COUNTER3  0x30b       //
#define MSR_IQ_COUNTER0     0x30c       //
#define MSR_IQ_COUNTER1     0x30d       //
#define MSR_IQ_COUNTER2     0x30e       //
#define MSR_IQ_COUNTER3     0x30f       //
#define MSR_IQ_COUNTER4     0x310       //
#define MSR_IQ_COUNTER5     0x311       //
#define MSR_BPU_CCCR0       0x360       // counter configuration control registers
#define MSR_BPU_CCCR1       0x361       // 
#define MSR_BPU_CCCR2       0x362       //
#define MSR_BPU_CCCR3       0x363       //
#define MSR_MS_CCCR0        0x364       //
#define MSR_MS_CCCR1        0x365       //
#define MSR_MS_CCCR2        0x366       //
#define MSR_MS_CCCR3        0x367       //
#define MSR_FLAME_CCCR0     0x368       //
#define MSR_FLAME_CCCR1     0x369       //
#define MSR_FLAME_CCCR2     0x36a       //
#define MSR_FLAME_CCCR3     0x36b       //
#define MSR_IQ_CCCR0        0x36c       //
#define MSR_IQ_CCCR1        0x36d       //
#define MSR_IQ_CCCR2        0x36e       //
#define MSR_IQ_CCCR3        0x36f       //
#define MSR_IQ_CCCR4        0x370       //
#define MSR_IQ_CCCR5        0x371       //
#define MSR_BSU_ESCR0       0x3a0       // event selection control registers
#define MSR_BSU_ESCR1       0x3a1       // 
#define MSR_FSB_ESCR0       0x3a2       //
#define MSR_FSB_ESCR1       0x3a3       //
#define MSR_FIRM_ESCR0      0x3a4       //
#define MSR_FIRM_ESCR1      0x3a5       //
#define MSR_FLAME_ESCR0     0x3a6       //
#define MSR_FLAME_ESCR1     0x3a7       //
#define MSR_DAC_ESCR0       0x3a8       //
#define MSR_DAC_ESCR1       0x3a9       //
#define MSR_MOB_ESCR0       0x3aa       //
#define MSR_MOB_ESCR1       0x3ab       //
#define MSR_PMH_ESCR0       0x3ac       //
#define MSR_PMH_ESCR1       0x3ad       //
#define MSR_SAAT_ESCR0      0x3ae       //
#define MSR_SAAT_ESCR1      0x3af       //
#define MSR_U2L_ESCR0       0x3b0       //
#define MSR_U2L_ESCR1       0x3b1       //
#define MSR_BPU_ESCR0       0x3b2       //
#define MSR_BPU_ESCR1       0x3b3       //
#define MSR_IS_ESCR0        0x3b4       //
#define MSR_IS_ESCR1        0x3b5       //
#define MSR_ITLB_ESCR0      0x3b6       //
#define MSR_ITLB_ESCR1      0x3b7       //
#define MSR_CRU_ESCR0       0x3b8       //
#define MSR_CRU_ESCR1       0x3b9       //
#define MSR_IQ_ESCR0        0x3ba       //
#define MSR_IQ_ESCR1        0x3bb       //
#define MSR_RAT_ESCR0       0x3bc       //
#define MSR_RAT_ESCR1       0x3bd       //
#define MSR_SSU_ESCR0       0x3be       //
#define MSR_MS_ESCR0        0x3c0       //
#define MSR_MS_ESCR1        0x3c1       //
#define MSR_TBPU_ESCR0      0x3c2       //
#define MSR_TBPU_ESCR1      0x3c3       //
#define MSR_TC_ESCR0        0x3c4       //
#define MSR_TC_ESCR1        0x3c5       //
#define MSR_IX_ESCR0        0x3c8       //
#define MSR_IX_ESCR1        0x3c9       //
#define MSR_ALF_ESCR0       0x3ca       //
#define MSR_ALF_ESCR1       0x3cb       //
#define MSR_CRU_ESCR2       0x3cc       //
#define MSR_CRU_ESCR3       0x3cd       //
#define MSR_CRU_ESCR4       0x3e0       //
#define MSR_CRU_ESCR5       0x3e1       //

//
// Flags within MSR_EFER.
//

#define MSR_SCE 0x00000001              // system call enable
#define MSR_LME 0x00000100              // long mode enable
#define MSR_LMA 0x00000400              // long mode active
#define MSR_NXE 0x00000800              // no execute enable
#define MSR_FFXSR 0x00004000            // fast floating save/restore

//
// Page attributes table.
//

#define PAT_TYPE_STRONG_UC  0           // uncacheable/strongly ordered
#define PAT_TYPE_USWC       1           // write combining/weakly ordered
#define PAT_TYPE_WT         4           // write through
#define PAT_TYPE_WP         5           // write protected
#define PAT_TYPE_WB         6           // write back
#define PAT_TYPE_WEAK_UC    7           // uncacheable/weakly ordered

//
// Page attributes table structure.
//

typedef union _PAT_ATTRIBUTES {
    struct {
        UCHAR Pat[8];
    } hw;

    ULONG64 QuadPart;
} PAT_ATTRIBUTES, *PPAT_ATTRIBUTES;

#define ReadMSR(Msr) __readmsr(Msr)

ULONG64
__readmsr (
    IN ULONG Msr
    );

#define WriteMSR(Msr, Data) __writemsr(Msr, Data)

VOID
__writemsr (
    IN ULONG Msr,
    IN ULONG64 Value
    );

#define ReadPMC(Counter) __readpmc(Counter)

ULONG64
__readpmc (
    IN ULONG Counter
    );

#define InvalidatePage(Page) __invlpg(Page)

VOID
__invlpg (
    IN PVOID Page
    );

#define WritebackInvalidate() __wbinvd()

VOID
__wbinvd (
    VOID
    );

#pragma intrinsic(__readmsr)
#pragma intrinsic(__writemsr)
#pragma intrinsic(__readpmc)
#pragma intrinsic(__invlpg)
#pragma intrinsic(__wbinvd)

#endif  // _AMD64_

// end_nthal

#if !(defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_) || defined(_WDMDDK_))

ULONG64
KxWaitForSpinLockAndAcquire (
    __inout PKSPIN_LOCK SpinLock
    );

__forceinline
VOID
KxAcquireSpinLock (
    __inout PKSPIN_LOCK SpinLock
    )

/*++

Routine Description:

    This function acquires a spin lock at the current IRQL.

Arguments:

    SpinLock - Supplies a pointer to an spin lock.

Return Value:

    None.

--*/

{

    //
    // Acquire the specified spin lock at the current IRQL.
    //

#if !defined(NT_UP)

#if DBG
    LONG64 Thread;

    Thread = (LONG64)KeGetCurrentThread() + 1;
    if (InterlockedCompareExchange64((LONG64 *)SpinLock, Thread, 0) != 0)
#else
    if (InterlockedBitTestAndSet64((LONG64 *)SpinLock, 0))
#endif
    {

        KxWaitForSpinLockAndAcquire(SpinLock);
    }

#else

    UNREFERENCED_PARAMETER(SpinLock);

#endif // !defined(NT_UP)

    return;
}

__forceinline
BOOLEAN
KxTryToAcquireSpinLock (
    __inout PKSPIN_LOCK SpinLock
    )

/*++

Routine Description:

    This function attempts acquires a spin lock at the current IRQL. If
    the spinlock is already owned, then FALSE is returned. Otherwise,
    TRUE is returned.

Arguments:

    SpinLock - Supplies a pointer to a spin lock.

Return Value:

    If the spin lock is acquired a value TRUE is returned. Otherwise, FALSE
    is returned as the function value.

--*/

{

    //
    // Try to acquire the specified spin lock at the current IRQL.
    //

#if !defined(NT_UP)

    if (*(volatile LONG64 *)SpinLock == 0) {

#if DBG

        LONG64 Thread;

        Thread = (LONG64)KeGetCurrentThread() + 1;
        return InterlockedCompareExchange64((PLONG64)SpinLock,
                                            Thread,
                                            0) == 0 ? TRUE : FALSE;

#else

        return !InterlockedBitTestAndSet64((LONG64 *)SpinLock, 0);

#endif // DBG

    } else {
        KeYieldProcessor();
        return FALSE;
    }

#else

    UNREFERENCED_PARAMETER(SpinLock);

    return TRUE;

#endif // !defined(NT_UP)

}

__forceinline
PKSPIN_LOCK_QUEUE
KiGetLockQueue (
    VOID
    )

{
    return (PKSPIN_LOCK_QUEUE)__readgsqword(FIELD_OFFSET(KPCR, LockArray));
}

__forceinline
KIRQL
KeAcquireSpinLockRaiseToDpc (
    __inout PKSPIN_LOCK SpinLock
    )

/*++

Routine Description:

    This function raises IRQL to DISPATCH_LEVEL and acquires the specified
    spin lock.

Arguments:

    SpinLock - Supplies a pointer to a spin lock.

Return Value:

    The previous IRQL is returned.

--*/

{

    KIRQL OldIrql;

    //
    // Raise IRQL to DISPATCH_LEVEL and acquire the specified spin lock.
    //

    OldIrql = KfRaiseIrql(DISPATCH_LEVEL);
    KxAcquireSpinLock(SpinLock);
    return OldIrql;
}

__forceinline
KIRQL
KeAcquireSpinLockRaiseToSynch (
    __inout PKSPIN_LOCK SpinLock
    )

/*++

Routine Description:

    This function raises IRQL to SYNCH_LEVEL and acquires the specified
    spin lock.

Arguments:

    SpinLock - Supplies a pointer to a spin lock.

Return Value:

    The previous IRQL is returned as the function value.

--*/

{

    KIRQL OldIrql;

    //
    // Raise IRQL to SYNCH_LEVEL and acquire the specified spin lock.
    //

    OldIrql = KfRaiseIrql(SYNCH_LEVEL);
    KxAcquireSpinLock(SpinLock);
    return OldIrql;
}

__forceinline
VOID
KeAcquireSpinLockAtDpcLevel (
    __inout PKSPIN_LOCK SpinLock
    )

/*++

Routine Description:

    This function acquires a spin lock at the current IRQL.

Arguments:

    SpinLock - Supplies a pointer to an spin lock.

Return Value:

    None.

--*/

{

    //
    // Acquired the specified spin lock at the current IRQL.
    //

    KxAcquireSpinLock(SpinLock);
    return;
}

__forceinline
VOID
KxReleaseSpinLock (
    __inout PKSPIN_LOCK SpinLock
    )

/*++

Routine Description:

    This function releases the specified spin lock at the current IRQL.

Arguments:

    SpinLock - Supplies a pointer to a spin lock.

Return Value:

    None.

--*/

{

#if !defined(NT_UP)

#if DBG

    ASSERT(*(volatile LONG64 *)SpinLock == (LONG64)KeGetCurrentThread() + 1);

#endif // DBG

    InterlockedAnd64((LONG64 *)SpinLock, 0);

#else

    UNREFERENCED_PARAMETER(SpinLock);

#endif // !defined(NT_UP)

    return;
}

__forceinline
VOID
KeReleaseSpinLock (
    __inout PKSPIN_LOCK SpinLock,
    __in KIRQL OldIrql
    )

/*++

Routine Description:

    This function releases the specified spin lock and lowers IRQL to a
    previous value.

Arguments:

    SpinLock - Supplies a pointer to a spin lock.

    OldIrql - Supplies the previous IRQL value.

Return Value:

    None.

--*/

{

    KxReleaseSpinLock(SpinLock);
    KeLowerIrql(OldIrql);
    return;
}

__forceinline
VOID
KeReleaseSpinLockFromDpcLevel (
    __inout PKSPIN_LOCK SpinLock
    )

/*++

Routine Description:

    This function releases a spin lock at the current IRQL.

Arguments:

    SpinLock - Supplies a pointer to a spin lock.

Return Value:

    None.

--*/

{

    KxReleaseSpinLock(SpinLock);
    return;
}

__forceinline
BOOLEAN
KeTestSpinLock (
    __in PKSPIN_LOCK SpinLock
    )

/*++

Routine Description:

    This function tests a spin lock to determine if it is currently owned.
    If the spinlock is already owned, then FALSE is returned. Otherwise,
    TRUE is returned.

Arguments:

    SpinLock - Supplies a pointer to a spin lock.

Return Value:

    If the spin lock is currently owned, then a value of FALSE is returned.
    Otherwise, a value of TRUE is returned.

--*/

{

    KeMemoryBarrierWithoutFence();
    if (*SpinLock != 0) {
        KeYieldProcessor();
        return FALSE;

    } else {
        return TRUE;
    }
}

__forceinline
BOOLEAN
KeTryToAcquireSpinLock (
    __inout PKSPIN_LOCK SpinLock,
    __out PKIRQL OldIrql
    )

/*++

Routine Description:

    This function raises IRQL to DISPATCH level and attempts to acquire a
    spin lock. If the spin lock is already owned, then IRQL is restored to
    its previous value and FALSE is returned. Otherwise, the spin lock is
    acquired and TRUE is returned.

Arguments:

    SpinLock - Supplies a pointer to a spin lock.

    OldIrql - Supplies a pointer to a variable that receives the old IRQL.

Return Value:

    If the spin lock is acquired a value TRUE is returned. Otherwise, FALSE
    is returned.

--*/

{

    //
    // Raise IRQL to DISPATCH level and attempt to acquire the specified
    // spin lock.
    //

    *OldIrql = KfRaiseIrql(DISPATCH_LEVEL);
    if (KxTryToAcquireSpinLock(SpinLock) == FALSE) {
        KeLowerIrql(*OldIrql);
        return FALSE;
    }

    return TRUE;
}

__forceinline
BOOLEAN
KeTryToAcquireSpinLockAtDpcLevel (
    __inout PKSPIN_LOCK SpinLock
    )

/*++

Routine Description:

    This function attempts acquires a spin lock at the current IRQL. If
    the spinlock is already owned, then FALSE is returned. Otherwise,
    TRUE is returned.

Arguments:

    SpinLock - Supplies a pointer to a spin lock.

Return Value:

    If the spin lock is acquired a value TRUE is returned. Otherwise, FALSE
    is returned as the function value.

--*/

{

    //
    // Try to acquire the specified spin lock at the current IRQL.
    //

    return KxTryToAcquireSpinLock(SpinLock);
}

#endif

//
// Define software feature bit definitions.
//
// The no execute feature flags must be identical on all platforms.
//

#define KF_SMT          0x00000001
#define KF_RDTSC        0x00000002
#define KF_CR4          0x00000004
#define KF_CMOV         0x00000008
#define KF_GLOBAL_PAGE  0x00000010
#define KF_LARGE_PAGE   0x00000020
#define KF_MTRR         0x00000040
#define KF_CMPXCHG8B    0x00000080
#define KF_MMX          0x00000100
#define KF_DTS          0x00000200
#define KF_PAT          0x00000400
#define KF_FXSR         0x00000800
#define KF_FAST_SYSCALL 0x00001000
#define KF_XMMI         0x00002000
#define KF_3DNOW        0x00004000
#define KF_AMDK6MTRR    0x00008000
#define KF_XMMI64       0x00010000
#define KF_NOEXECUTE    0x20000000
#define KF_GLOBAL_32BIT_EXECUTE 0x40000000
#define KF_GLOBAL_32BIT_NOEXECUTE 0x80000000

//
// Define required software feature bits.
//

#define KF_REQUIRED (KF_RDTSC | KF_CR4 | KF_CMOV | KF_GLOBAL_PAGE |          \
                     KF_LARGE_PAGE | KF_MTRR | KF_CMPXCHG8B | KF_MMX |       \
                     KF_PAT | KF_FXSR | KF_FAST_SYSCALL | KF_XMMI |          \
                     KF_XMMI64)

//
// Define standard hardware feature bits definitions (cpuid(1, ...).
//

#define HF_FPU          0x00000001      // FPU is on chip
#define HF_VME          0x00000002      // virtual 8086 mode enhancement
#define HF_DE           0x00000004      // debugging extension
#define HF_PSE          0x00000008      // page size extension
#define HF_TSC          0x00000010      // time stamp counter
#define HF_MSR          0x00000020      // rdmsr and wrmsr support
#define HF_PAE          0x00000040      // physical address extension
#define HF_MCE          0x00000080      // machine check exception
#define HF_CX8          0x00000100      // cmpxchg8b instruction supported
#define HF_APIC         0x00000200      // APIC on chip
#define HF_UNUSED0      0x00000400      // unused bit
#define HF_SYSENTER     0x00000800      // sysenter/sysesxit instructions
#define HF_MTRR         0x00001000      // memory type range registers
#define HF_PGE          0x00002000      // global page TB support
#define HF_MCA          0x00004000      // machine check architecture
#define HF_CMOV         0x00008000      // cmov instruction supported
#define HF_PAT          0x00010000      // physical attributes table
#define HF_PSE2         0x00020000      // page size extension (2)
#define HF_PSN          0x00040000      // processor serial number
#define HF_CFLUSH       0x00080000      // cache line flush
#define HF_UNUSED1      0x00100000      // unused bit
#define HF_DS           0x00200000      // debug store
#define HF_ACPI_THMON   0x00400000      // ACPI thermal monitor
#define HF_MMX          0x00800000      // MMX technology supported
#define HF_FXSR         0x01000000      // fxsr instruction supported
#define HF_XMMI         0x02000000      // SSE supported
#define HF_XMMI64       0x04000000      // SSE2 supported
#define HF_SS           0x08000000      // self snoop
#define HF_SMT          0x10000000      // symmetric multithreading
#define HF_THERMMON     0x20000000      // thermal monitor
#define HF_UNUSED2      0x40000000      // unused bit
#define HF_PBE          0x80000000      // pending break enable

//
// Define required hardware feature bits.
//

#define HF_REQUIRED (HF_FPU | HF_DE | HF_PSE | HF_TSC | HF_MSR |             \
                     HF_PAE | HF_MCE | HF_CX8 | HF_APIC | HF_MTRR |          \
                     HF_PGE | HF_MCA | HF_CMOV | HF_PAT | HF_MMX |           \
                     HF_FXSR |  HF_XMMI | HF_XMMI64 | HF_CFLUSH)

//
// Define extended hardware feature bit definitions (cpuid(80000001, ...).
//

#define XHF_FPU         0x00000001      // FPU is on chip
#define XHF_VME         0x00000002      // virtual 8086 mode enhancement
#define XHF_DE          0x00000004      // debugging extension
#define XHF_PSE         0x00000008      // page size extension
#define XHF_TSC         0x00000010      // time stamp counter
#define XHF_MSR         0x00000020      // rdmsr and wrmsr support
#define XHF_PAE         0x00000040      // physical address extension
#define XHF_MCE         0x00000080      // machine check exception
#define XHF_CX8         0x00000100      // cmpxchg8b instruction supported
#define XHF_APIC        0x00000200      // APIC on chip
#define XHF_UNUSED0     0x00000400      // unused bit
#define XHF_SYSCALL     0x00000800      // syscall/sysret instructions
#define XHF_MTRR        0x00001000      // memory type range registers
#define XHF_PGE         0x00002000      // global page TB support
#define XHF_MCA         0x00004000      // machine check architecture
#define XHF_CMOV        0x00008000      // cmov instruction supported
#define XHF_PAT         0x00010000      // physical attributes table
#define XHF_PSE2        0x00020000      // page size extension (2)
#define XHF_UNUSED1     0x00040000      // unused bit
#define XHF_UNUSED2     0x00080000      // unused bit
#define XHF_NOEXECUTE   0x00100000      // no execute protection
#define XHF_UNUSED3     0x00200000      // unused bit
#define XHF_MMX_EXT     0x00400000      // MMX extensions
#define XHF_MMX_INT     0x00800000      // MMX technology supported
#define XHF_FXSR        0x01000000      // fxsr instruction supported
#define XHF_FFXSR       0x02000000      // fast floating save/restore
#define XHF_UNUSED5     0x04000000      // unused bit
#define XHF_UNUSED6     0x08000000      // unused bit
#define XHF_UNUSED7     0x10000000      // unused bit
#define XHF_LONGMODE    0x20000000      // long mode supported
#define XHF_3DNOW_EXT   0x40000000      // 3DNOW extensions
#define XHF_3DNOW       0x80000000      // 3DNOW supported

#endif // __amd64_
