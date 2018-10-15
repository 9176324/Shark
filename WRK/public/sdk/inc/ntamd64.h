/*++

Copyright (c) Microsoft Corporation. All rights reserved.

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ntamd64.w

Abstract:

    User mode visible AMD64 specific structures and constants.

--*/

#ifndef _NTAMD64_
#define _NTAMD64_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

// begin_ntddk begin_wdm begin_nthal begin_winnt begin_ntminiport begin_wx86

#if defined(_AMD64_)

// end_ntddk end_wdm end_nthal end_winnt end_ntminiport end_wx86

//
//  ?? Values put in ExceptionRecord.ExceptionInformation[0]
//  ?? First parameter is always in ExceptionInformation[1],
//  ?? Second parameter is always in ExceptionInformation[2]
//

#define BREAKPOINT_BREAK 0
#define BREAKPOINT_PRINT 1
#define BREAKPOINT_PROMPT 2
#define BREAKPOINT_LOAD_SYMBOLS 3
#define BREAKPOINT_UNLOAD_SYMBOLS 4
#define BREAKPOINT_COMMAND_STRING 5

//
// Define AMD64 specific control space.
//

typedef enum _DEBUG_CONTROL_SPACE_ITEM {
    DEBUG_CONTROL_SPACE_PCR,
    DEBUG_CONTROL_SPACE_PRCB,
    DEBUG_CONTROL_SPACE_KSPECIAL,
    DEBUG_CONTROL_SPACE_THREAD,
    DEBUG_CONTROL_SPACE_MAXIMUM
} DEBUG_CONTROL_SPACE_ITEM;

//
// Define Address of User Shared Data.
//

#define MM_SHARED_USER_DATA_VA 0x7FFE0000

#define USER_SHARED_DATA ((KUSER_SHARED_DATA * const)MM_SHARED_USER_DATA_VA)

//
// Define address of the WOW64 reserved compatibility area.
//

#define WOW64_COMPATIBILITY_AREA_ADDRESS (MM_SHARED_USER_DATA_VA - 0x1000000)

//
// Define address of the system-wide csrss shared section.
//

#define CSR_SYSTEM_SHARED_ADDRESS (WOW64_COMPATIBILITY_AREA_ADDRESS)

// begin_winnt begin_ntddk begin_wdm begin_nthal begin_ntndis begin_ntosp begin_ntminiport

#if defined(_M_AMD64) && !defined(RC_INVOKED) && !defined(MIDL_PASS)

//
// Define bit test intrinsics.
//

#ifdef __cplusplus
extern "C" {
#endif

#define BitTest _bittest
#define BitTestAndComplement _bittestandcomplement
#define BitTestAndSet _bittestandset
#define BitTestAndReset _bittestandreset
#define InterlockedBitTestAndSet _interlockedbittestandset
#define InterlockedBitTestAndReset _interlockedbittestandreset

#define BitTest64 _bittest64
#define BitTestAndComplement64 _bittestandcomplement64
#define BitTestAndSet64 _bittestandset64
#define BitTestAndReset64 _bittestandreset64
#define InterlockedBitTestAndSet64 _interlockedbittestandset64
#define InterlockedBitTestAndReset64 _interlockedbittestandreset64

BOOLEAN
_bittest (
    IN LONG const *Base,
    IN LONG Offset
    );

BOOLEAN
_bittestandcomplement (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_bittestandset (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_bittestandreset (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_interlockedbittestandset (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_interlockedbittestandreset (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_bittest64 (
    IN LONG64 const *Base,
    IN LONG64 Offset
    );

BOOLEAN
_bittestandcomplement64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_bittestandset64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_bittestandreset64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_interlockedbittestandset64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_interlockedbittestandreset64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

#pragma intrinsic(_bittest)
#pragma intrinsic(_bittestandcomplement)
#pragma intrinsic(_bittestandset)
#pragma intrinsic(_bittestandreset)
#pragma intrinsic(_interlockedbittestandset)
#pragma intrinsic(_interlockedbittestandreset)

#pragma intrinsic(_bittest64)
#pragma intrinsic(_bittestandcomplement64)
#pragma intrinsic(_bittestandset64)
#pragma intrinsic(_bittestandreset64)
#pragma intrinsic(_interlockedbittestandset64)
#pragma intrinsic(_interlockedbittestandreset64)

//
// Define bit scan intrinsics.
//

#define BitScanForward _BitScanForward
#define BitScanReverse _BitScanReverse
#define BitScanForward64 _BitScanForward64
#define BitScanReverse64 _BitScanReverse64

BOOLEAN
_BitScanForward (
    OUT ULONG *Index,
    IN ULONG Mask
    );

BOOLEAN
_BitScanReverse (
    OUT ULONG *Index,
    IN ULONG Mask
    );

BOOLEAN
_BitScanForward64 (
    OUT ULONG *Index,
    IN ULONG64 Mask
    );

BOOLEAN
_BitScanReverse64 (
    OUT ULONG *Index,
    IN ULONG64 Mask
    );

#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(_BitScanReverse64)

//
// Interlocked intrinsic functions.
//

#define InterlockedIncrement16 _InterlockedIncrement16
#define InterlockedDecrement16 _InterlockedDecrement16
#define InterlockedCompareExchange16 _InterlockedCompareExchange16

#define InterlockedAnd _InterlockedAnd
#define InterlockedOr _InterlockedOr
#define InterlockedXor _InterlockedXor
#define InterlockedIncrement _InterlockedIncrement
#define InterlockedIncrementAcquire InterlockedIncrement
#define InterlockedIncrementRelease InterlockedIncrement
#define InterlockedDecrement _InterlockedDecrement
#define InterlockedDecrementAcquire InterlockedDecrement
#define InterlockedDecrementRelease InterlockedDecrement
#define InterlockedAdd _InterlockedAdd
#define InterlockedExchange _InterlockedExchange
#define InterlockedExchangeAdd _InterlockedExchangeAdd
#define InterlockedCompareExchange _InterlockedCompareExchange
#define InterlockedCompareExchangeAcquire InterlockedCompareExchange
#define InterlockedCompareExchangeRelease InterlockedCompareExchange

#define InterlockedAnd64 _InterlockedAnd64
#define InterlockedAndAffinity InterlockedAnd64
#define InterlockedOr64 _InterlockedOr64
#define InterlockedOrAffinity InterlockedOr64
#define InterlockedXor64 _InterlockedXor64
#define InterlockedIncrement64 _InterlockedIncrement64
#define InterlockedDecrement64 _InterlockedDecrement64
#define InterlockedAdd64 _InterlockedAdd64
#define InterlockedExchange64 _InterlockedExchange64
#define InterlockedExchangeAcquire64 InterlockedExchange64
#define InterlockedExchangeAdd64 _InterlockedExchangeAdd64
#define InterlockedCompareExchange64 _InterlockedCompareExchange64
#define InterlockedCompareExchangeAcquire64 InterlockedCompareExchange64
#define InterlockedCompareExchangeRelease64 InterlockedCompareExchange64

#define InterlockedExchangePointer _InterlockedExchangePointer
#define InterlockedCompareExchangePointer _InterlockedCompareExchangePointer
#define InterlockedCompareExchangePointerAcquire _InterlockedCompareExchangePointer
#define InterlockedCompareExchangePointerRelease _InterlockedCompareExchangePointer

#define InterlockedExchangeAddSizeT(a, b) InterlockedExchangeAdd64((LONG64 *)a, b)
#define InterlockedIncrementSizeT(a) InterlockedIncrement64((LONG64 *)a)
#define InterlockedDecrementSizeT(a) InterlockedDecrement64((LONG64 *)a)

SHORT
InterlockedIncrement16 (
    IN OUT SHORT volatile *Addend
    );

SHORT
InterlockedDecrement16 (
    IN OUT SHORT volatile *Addend
    );

SHORT
InterlockedCompareExchange16 (
    IN OUT SHORT volatile *Destination,
    IN SHORT ExChange,
    IN SHORT Comperand
    );

LONG
InterlockedAnd (
    IN OUT LONG volatile *Destination,
    IN LONG Value
    );

LONG
InterlockedOr (
    IN OUT LONG volatile *Destination,
    IN LONG Value
    );

LONG
InterlockedXor (
    IN OUT LONG volatile *Destination,
    IN LONG Value
    );

LONG64
InterlockedAnd64 (
    IN OUT LONG64 volatile *Destination,
    IN LONG64 Value
    );

LONG64
InterlockedOr64 (
    IN OUT LONG64 volatile *Destination,
    IN LONG64 Value
    );

LONG64
InterlockedXor64 (
    IN OUT LONG64 volatile *Destination,
    IN LONG64 Value
    );

LONG
InterlockedIncrement(
    IN OUT LONG volatile *Addend
    );

LONG
InterlockedDecrement(
    IN OUT LONG volatile *Addend
    );

LONG
InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value
    );

LONG
InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Value
    );

#if !defined(_X86AMD64_)

__forceinline
LONG
InterlockedAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Value
    )

{
    return InterlockedExchangeAdd(Addend, Value) + Value;
}

#endif

LONG
InterlockedCompareExchange (
    IN OUT LONG volatile *Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );

LONG64
InterlockedIncrement64(
    IN OUT LONG64 volatile *Addend
    );

LONG64
InterlockedDecrement64(
    IN OUT LONG64 volatile *Addend
    );

LONG64
InterlockedExchange64(
    IN OUT LONG64 volatile *Target,
    IN LONG64 Value
    );

LONG64
InterlockedExchangeAdd64(
    IN OUT LONG64 volatile *Addend,
    IN LONG64 Value
    );

#if !defined(_X86AMD64_)

__forceinline
LONG64
InterlockedAdd64(
    IN OUT LONG64 volatile *Addend,
    IN LONG64 Value
    )

{
    return InterlockedExchangeAdd64(Addend, Value) + Value;
}

#endif

LONG64
InterlockedCompareExchange64 (
    IN OUT LONG64 volatile *Destination,
    IN LONG64 ExChange,
    IN LONG64 Comperand
    );

PVOID
InterlockedCompareExchangePointer (
    IN OUT PVOID volatile *Destination,
    IN PVOID Exchange,
    IN PVOID Comperand
    );

PVOID
InterlockedExchangePointer(
    IN OUT PVOID volatile *Target,
    IN PVOID Value
    );

#pragma intrinsic(_InterlockedIncrement16)
#pragma intrinsic(_InterlockedDecrement16)
#pragma intrinsic(_InterlockedCompareExchange16)
#pragma intrinsic(_InterlockedAnd)
#pragma intrinsic(_InterlockedOr)
#pragma intrinsic(_InterlockedXor)
#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedExchangeAdd)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedAnd64)
#pragma intrinsic(_InterlockedOr64)
#pragma intrinsic(_InterlockedXor64)
#pragma intrinsic(_InterlockedIncrement64)
#pragma intrinsic(_InterlockedDecrement64)
#pragma intrinsic(_InterlockedExchange64)
#pragma intrinsic(_InterlockedExchangeAdd64)
#pragma intrinsic(_InterlockedCompareExchange64)
#pragma intrinsic(_InterlockedExchangePointer)
#pragma intrinsic(_InterlockedCompareExchangePointer)

//
// Define function to flush a cache line.
//

#define CacheLineFlush(Address) _mm_clflush(Address)

VOID
_mm_clflush (
    VOID const *Address
    );

#pragma intrinsic(_mm_clflush)

VOID
_ReadWriteBarrier (
    VOID
    );

#pragma intrinsic(_ReadWriteBarrier)

//
// Define memory fence intrinsics
//

#define FastFence __faststorefence
#define LoadFence _mm_lfence
#define MemoryFence _mm_mfence
#define StoreFence _mm_sfence

VOID
__faststorefence (
    VOID
    );

VOID
_mm_lfence (
    VOID
    );

VOID
_mm_mfence (
    VOID
    );

VOID
_mm_sfence (
    VOID
    );

VOID
_mm_pause (
    VOID
    );

VOID 
_mm_prefetch (
    CHAR CONST *a, 
    int sel
    );

VOID
_m_prefetchw (
    volatile CONST VOID *Source
    );

//
// Define constants for use with _mm_prefetch.
//

#define _MM_HINT_T0     1
#define _MM_HINT_T1     2
#define _MM_HINT_T2     3
#define _MM_HINT_NTA    0

#pragma intrinsic(__faststorefence)
#pragma intrinsic(_mm_pause)
#pragma intrinsic(_mm_prefetch)
#pragma intrinsic(_mm_lfence)
#pragma intrinsic(_mm_mfence)
#pragma intrinsic(_mm_sfence)
#pragma intrinsic(_m_prefetchw)

#define YieldProcessor _mm_pause
#define MemoryBarrier __faststorefence
#define PreFetchCacheLine(l, a)  _mm_prefetch((CHAR CONST *) a, l)
#define PrefetchForWrite(p) _m_prefetchw(p)
#define ReadForWriteAccess(p) (_m_prefetchw(p), *(p))

//
// PreFetchCacheLine level defines.
//

#define PF_TEMPORAL_LEVEL_1 _MM_HINT_T0
#define PF_TEMPORAL_LEVEL_2 _MM_HINT_T1
#define PF_TEMPORAL_LEVEL_3 _MM_HINT_T2
#define PF_NON_TEMPORAL_LEVEL_ALL _MM_HINT_NTA

//
// Define get/set MXCSR intrinsics.
//

#define ReadMxCsr _mm_getcsr
#define WriteMxCsr _mm_setcsr

unsigned int
_mm_getcsr (
    VOID
    );

VOID
_mm_setcsr (
    unsigned int MxCsr
    );

#pragma intrinsic(_mm_getcsr)
#pragma intrinsic(_mm_setcsr)

//
// Assert exception.
//

VOID
__int2c (
    VOID
    );

#pragma intrinsic(__int2c)

#define DbgRaiseAssertionFailure() __int2c()

//
// Define function to get the caller's EFLAGs value.
//

#define GetCallersEflags() __getcallerseflags()

unsigned __int32
__getcallerseflags (
    VOID
    );

#pragma intrinsic(__getcallerseflags)

//
// Define function to get segment limit.
//

#define GetSegmentLimit __segmentlimit

ULONG
__segmentlimit (
    IN ULONG Selector
    );

#pragma intrinsic(__segmentlimit)
    
//
// Define function to read the value of the time stamp counter
//

#define ReadTimeStampCounter() __rdtsc()

ULONG64
__rdtsc (
    VOID
    );

#pragma intrinsic(__rdtsc)

//
// Define functions to move strings as bytes, words, dwords, and qwords.
//

VOID
__movsb (
    IN PUCHAR Destination,
    IN UCHAR const *Source,
    IN SIZE_T Count
    );

VOID
__movsw (
    IN PUSHORT Destination,
    IN USHORT const *Source,
    IN SIZE_T Count
    );

VOID
__movsd (
    IN PULONG Destination,
    IN ULONG const *Source,
    IN SIZE_T Count
    );

VOID
__movsq (
    IN PULONG64 Destination,
    IN ULONG64 const *Source,
    IN SIZE_T Count
    );

#pragma intrinsic(__movsb)
#pragma intrinsic(__movsw)
#pragma intrinsic(__movsd)
#pragma intrinsic(__movsq)

//
// Define functions to store strings as bytes, words, dwords, and qwords.
//

VOID
__stosb (
    IN PUCHAR Destination,
    IN UCHAR Value,
    IN SIZE_T Count
    );

VOID
__stosw (
    IN PUSHORT Destination,
    IN USHORT Value,
    IN SIZE_T Count
    );

VOID
__stosd (
    IN PULONG Destination,
    IN ULONG Value,
    IN SIZE_T Count
    );

VOID
__stosq (
    IN PULONG64 Destination,
    IN ULONG64 Value,
    IN SIZE_T Count
    );

#pragma intrinsic(__stosb)
#pragma intrinsic(__stosw)
#pragma intrinsic(__stosd)
#pragma intrinsic(__stosq)

//
// Define functions to capture the high 64-bits of a 128-bit multiply.
//

#define MultiplyHigh __mulh
#define UnsignedMultiplyHigh __umulh

LONGLONG
MultiplyHigh (
    IN LONGLONG Multiplier,
    IN LONGLONG Multiplicand
    );

ULONGLONG
UnsignedMultiplyHigh (
    IN ULONGLONG Multiplier,
    IN ULONGLONG Multiplicand
    );

#pragma intrinsic(__mulh)
#pragma intrinsic(__umulh)

//
// Define functions to perform 128-bit shifts
//

#define ShiftLeft128 __shiftleft128
#define ShiftRight128 __shiftright128

ULONG64
ShiftLeft128 (
    IN ULONG64 LowPart,
    IN ULONG64 HighPart,
    IN UCHAR Shift
    );

ULONG64
ShiftRight128 (
    IN ULONG64 LowPart,
    IN ULONG64 HighPart,
    IN UCHAR Shift
    );

#pragma intrinsic(__shiftleft128)
#pragma intrinsic(__shiftright128)

//
// Define functions to perform 128-bit multiplies.
//

#define Multiply128 _mul128

LONG64
Multiply128 (
    IN  LONG64  Multiplier,
    IN  LONG64  Multiplicand,
    OUT LONG64 *HighProduct
    );

#pragma intrinsic(_mul128)

#define UnsignedMultiply128 _umul128

ULONG64
UnsignedMultiply128 (
    IN  ULONG64  Multiplier,
    IN  ULONG64  Multiplicand,
    OUT ULONG64 *HighProduct
    );

#pragma intrinsic(_umul128)

__forceinline
LONG64
MultiplyExtract128 (
    IN LONG64 Multiplier,
    IN LONG64 Multiplicand,
    IN UCHAR Shift
    )
{
    LONG64 extractedProduct;
    LONG64 highProduct;
    LONG64 lowProduct;

    lowProduct = Multiply128( Multiplier,
                              Multiplicand,
                              &highProduct );

    extractedProduct = (LONG64)ShiftRight128( (LONG64)lowProduct,
                                              (LONG64)highProduct,
                                              Shift );
    return extractedProduct;
}

__forceinline
ULONG64
UnsignedMultiplyExtract128 (
    IN ULONG64 Multiplier,
    IN ULONG64 Multiplicand,
    IN UCHAR Shift
    )
{
    ULONG64 extractedProduct;
    ULONG64 highProduct;
    ULONG64 lowProduct;

    lowProduct = UnsignedMultiply128( Multiplier,
                                      Multiplicand,
                                      &highProduct );

    extractedProduct = ShiftRight128( lowProduct,
                                      highProduct,
                                      Shift );
    return extractedProduct;
}

//
// Define functions to read and write the uer TEB and the system PCR/PRCB.
//

UCHAR
__readgsbyte (
    IN ULONG Offset
    );

USHORT
__readgsword (
    IN ULONG Offset
    );

ULONG
__readgsdword (
    IN ULONG Offset
    );

ULONG64
__readgsqword (
    IN ULONG Offset
    );

VOID
__writegsbyte (
    IN ULONG Offset,
    IN UCHAR Data
    );

VOID
__writegsword (
    IN ULONG Offset,
    IN USHORT Data
    );

VOID
__writegsdword (
    IN ULONG Offset,
    IN ULONG Data
    );

VOID
__writegsqword (
    IN ULONG Offset,
    IN ULONG64 Data
    );

#pragma intrinsic(__readgsbyte)
#pragma intrinsic(__readgsword)
#pragma intrinsic(__readgsdword)
#pragma intrinsic(__readgsqword)
#pragma intrinsic(__writegsbyte)
#pragma intrinsic(__writegsword)
#pragma intrinsic(__writegsdword)
#pragma intrinsic(__writegsqword)

#ifdef __cplusplus
}
#endif 

#endif // defined(_M_AMD64) && !defined(RC_INVOKED) && !defined(MIDL_PASS)

// end_winnt end_ntddk end_wdm end_nthal end_ntndis end_ntosp end_ntminiport

// begin_ntddk begin_nthal
//
// Size of kernel mode stack.
//

#define KERNEL_STACK_SIZE 0x6000

//
// Define size of large kernel mode stack for callbacks.
//

#define KERNEL_LARGE_STACK_SIZE 0x12000

//
// Define number of pages to initialize in a large kernel stack.
//

#define KERNEL_LARGE_STACK_COMMIT KERNEL_STACK_SIZE

//
// Define the size of the stack used for processing an MCA exception.
//

#define KERNEL_MCA_EXCEPTION_STACK_SIZE 0x2000

// end_ntddk end_nthal

#define DOUBLE_FAULT_STACK_SIZE 0x2000

#define NMI_STACK_SIZE 0x2000

// begin_nthal
//              
// Define stack alignment and rounding values.
//

#define STACK_ALIGN (16UI64)
#define STACK_ROUND (STACK_ALIGN - 1)

//
// Define constants for system IDTs
//

#define MAXIMUM_IDTVECTOR 0xff
#define MAXIMUM_PRIMARY_VECTOR 0xff
#define PRIMARY_VECTOR_BASE 0x30        // 0-2f are AMD64 trap vectors

// begin_winnt begin_ntddk 
//
// The following values specify the type of access in the first parameter
// of the exception record whan the exception code specifies an access
// violation.
//

#define EXCEPTION_READ_FAULT 0          // exception caused by a read
#define EXCEPTION_WRITE_FAULT 1         // exception caused by a write
#define EXCEPTION_EXECUTE_FAULT 8       // exception caused by an instruction fetch

// begin_wx86
//
// The following flags control the contents of the CONTEXT structure.
//

#if !defined(RC_INVOKED)

#define CONTEXT_AMD64   0x100000

// end_wx86

#define CONTEXT_CONTROL (CONTEXT_AMD64 | 0x1L)
#define CONTEXT_INTEGER (CONTEXT_AMD64 | 0x2L)
#define CONTEXT_SEGMENTS (CONTEXT_AMD64 | 0x4L)
#define CONTEXT_FLOATING_POINT  (CONTEXT_AMD64 | 0x8L)
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_AMD64 | 0x10L)

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT)

#define CONTEXT_ALL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS)

#define CONTEXT_EXCEPTION_ACTIVE 0x8000000
#define CONTEXT_SERVICE_ACTIVE 0x10000000
#define CONTEXT_EXCEPTION_REQUEST 0x40000000
#define CONTEXT_EXCEPTION_REPORTING 0x80000000

// begin_wx86

#endif // !defined(RC_INVOKED)

//
// Define initial MxCsr and FpCsr control.
//

#define INITIAL_MXCSR 0x1f80            // initial MXCSR value
#define INITIAL_FPCSR 0x027f            // initial FPCSR value

//
// Define 128-bit 16-byte aligned xmm register type.
//

typedef struct DECLSPEC_ALIGN(16) _M128A {
    ULONGLONG Low;
    LONGLONG High;
} M128A, *PM128A;

//
// Format of data for 32-bit fxsave/fxrstor instructions.
//

typedef struct _XMM_SAVE_AREA32 {
    USHORT ControlWord;
    USHORT StatusWord;
    UCHAR TagWord;
    UCHAR Reserved1;
    USHORT ErrorOpcode;
    ULONG ErrorOffset;
    USHORT ErrorSelector;
    USHORT Reserved2;
    ULONG DataOffset;
    USHORT DataSelector;
    USHORT Reserved3;
    ULONG MxCsr;
    ULONG MxCsr_Mask;
    M128A FloatRegisters[8];
    M128A XmmRegisters[16];
    UCHAR Reserved4[96];
} XMM_SAVE_AREA32, *PXMM_SAVE_AREA32;

#define LEGACY_SAVE_AREA_LENGTH sizeof(XMM_SAVE_AREA32)

//
// Context Frame
//
//  This frame has a several purposes: 1) it is used as an argument to
//  NtContinue, 2) is is used to constuct a call frame for APC delivery,
//  and 3) it is used in the user level thread creation routines.
//
//
// The flags field within this record controls the contents of a CONTEXT
// record.
//
// If the context record is used as an input parameter, then for each
// portion of the context record controlled by a flag whose value is
// set, it is assumed that that portion of the context record contains
// valid context. If the context record is being used to modify a threads
// context, then only that portion of the threads context is modified.
//
// If the context record is used as an output parameter to capture the
// context of a thread, then only those portions of the thread's context
// corresponding to set flags will be returned.
//
// CONTEXT_CONTROL specifies SegSs, Rsp, SegCs, Rip, and EFlags.
//
// CONTEXT_INTEGER specifies Rax, Rcx, Rdx, Rbx, Rbp, Rsi, Rdi, and R8-R15.
//
// CONTEXT_SEGMENTS specifies SegDs, SegEs, SegFs, and SegGs.
//
// CONTEXT_DEBUG_REGISTERS specifies Dr0-Dr3 and Dr6-Dr7.
//
// CONTEXT_MMX_REGISTERS specifies the floating point and extended registers
//     Mm0/St0-Mm7/St7 and Xmm0-Xmm15).
//

typedef struct DECLSPEC_ALIGN(16) _CONTEXT {

    //
    // Register parameter home addresses.
    //
    // N.B. These fields are for convenience - they could be used to extend the
    //      context record in the future.
    //

    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5Home;
    ULONG64 P6Home;

    //
    // Control flags.
    //

    ULONG ContextFlags;
    ULONG MxCsr;

    //
    // Segment Registers and processor flags.
    //

    USHORT SegCs;
    USHORT SegDs;
    USHORT SegEs;
    USHORT SegFs;
    USHORT SegGs;
    USHORT SegSs;
    ULONG EFlags;

    //
    // Debug registers
    //

    ULONG64 Dr0;
    ULONG64 Dr1;
    ULONG64 Dr2;
    ULONG64 Dr3;
    ULONG64 Dr6;
    ULONG64 Dr7;

    //
    // Integer registers.
    //

    ULONG64 Rax;
    ULONG64 Rcx;
    ULONG64 Rdx;
    ULONG64 Rbx;
    ULONG64 Rsp;
    ULONG64 Rbp;
    ULONG64 Rsi;
    ULONG64 Rdi;
    ULONG64 R8;
    ULONG64 R9;
    ULONG64 R10;
    ULONG64 R11;
    ULONG64 R12;
    ULONG64 R13;
    ULONG64 R14;
    ULONG64 R15;

    //
    // Program counter.
    //

    ULONG64 Rip;

    //
    // Floating point state.
    //

    union {
        XMM_SAVE_AREA32 FltSave;
        struct {
            M128A Header[2];
            M128A Legacy[8];
            M128A Xmm0;
            M128A Xmm1;
            M128A Xmm2;
            M128A Xmm3;
            M128A Xmm4;
            M128A Xmm5;
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
        };
    };

    //
    // Vector registers.
    //

    M128A VectorRegister[26];
    ULONG64 VectorControl;

    //
    // Special debug control registers.
    //

    ULONG64 DebugControl;
    ULONG64 LastBranchToRip;
    ULONG64 LastBranchFromRip;
    ULONG64 LastExceptionToRip;
    ULONG64 LastExceptionFromRip;
} CONTEXT, *PCONTEXT;

// end_ntddk end_nthal end_winnt end_wx86

#define CONTEXT_TO_PROGRAM_COUNTER(Context) ((Context)->Rip)
#define PROGRAM_COUNTER_TO_CONTEXT(Context, ProgramCounter) \
    ((Context)->Rip = (ProgramCounter))

#define CONTEXT_ALIGN STACK_ALIGN
#define CONTEXT_LENGTH sizeof(CONTEXT)

//
// Nonvolatile context pointer record.
//

typedef struct _KNONVOLATILE_CONTEXT_POINTERS {
    union {
        PM128A FloatingContext[16];
        struct {
            PM128A Xmm0;
            PM128A Xmm1;
            PM128A Xmm2;
            PM128A Xmm3;
            PM128A Xmm4;
            PM128A Xmm5;
            PM128A Xmm6;
            PM128A Xmm7;
            PM128A Xmm8;
            PM128A Xmm9;
            PM128A Xmm10;
            PM128A Xmm11;
            PM128A Xmm12;
            PM128A Xmm13;
            PM128A Xmm14;
            PM128A Xmm15;
        };
    };

    union {
        PULONG64 IntegerContext[16];
        struct {
            PULONG64 Rax;
            PULONG64 Rcx;
            PULONG64 Rdx;
            PULONG64 Rbx;
            PULONG64 Rsp;
            PULONG64 Rbp;
            PULONG64 Rsi;
            PULONG64 Rdi;
            PULONG64 R8;
            PULONG64 R9;
            PULONG64 R10;
            PULONG64 R11;
            PULONG64 R12;
            PULONG64 R13;
            PULONG64 R14;
            PULONG64 R15;
        };
    };

} KNONVOLATILE_CONTEXT_POINTERS, *PKNONVOLATILE_CONTEXT_POINTERS;

// begin_wx86 begin_nthal
//
//  GDT selector numbers.
//
// N.B. There is code in context swap that "cleanses" the user segment
//      registers ds, es, fs, and gs. If these values are changed or
//      added to, then it is very likely the code in context swap will
//      have to be change.
//
     
#define KGDT64_NULL (0 * 16)            // NULL descriptor
#define KGDT64_R0_CODE (1 * 16)         // kernel mode 64-bit code
#define KGDT64_R0_DATA (1 * 16) + 8     // kernel mode 64-bit data (stack)
#define KGDT64_R3_CMCODE (2 * 16)       // user mode 32-bit code
#define KGDT64_R3_DATA (2 * 16) + 8     // user mode 32-bit data
#define KGDT64_R3_CODE (3 * 16)         // user mode 64-bit code
#define KGDT64_SYS_TSS (4 * 16)         // kernel mode system task state
#define KGDT64_R3_CMTEB (5 * 16)        // user mode 32-bit TEB
#define KGDT64_R0_CMCODE (6 * 16)       // kernel mode 32-bit code
#define KGDT64_LAST (7 * 16)            // last entry

#define KGDT_NUMBER KGDT_LAST

// end_wx86 end_nthal

// begin_ntddk begin_wdm begin_nthal begin_ntminiport

#endif // _AMD64_

// end_ntddk end_wdm end_nthal end_ntminiport

//
// Define AMD64 exception handling structures and function prototypes.
//
// Define unwind operation codes.
//

typedef enum _UNWIND_OP_CODES {
    UWOP_PUSH_NONVOL = 0,
    UWOP_ALLOC_LARGE,
    UWOP_ALLOC_SMALL,
    UWOP_SET_FPREG,
    UWOP_SAVE_NONVOL,
    UWOP_SAVE_NONVOL_FAR,
    UWOP_SPARE_CODE1,
    UWOP_SPARE_CODE2,
    UWOP_SAVE_XMM128,
    UWOP_SAVE_XMM128_FAR,
    UWOP_PUSH_MACHFRAME
} UNWIND_OP_CODES, *PUNWIND_OP_CODES;

//
// Define unwind code structure.
//

typedef union _UNWIND_CODE {
    struct {
        UCHAR CodeOffset;
        UCHAR UnwindOp : 4;
        UCHAR OpInfo : 4;
    };

    USHORT FrameOffset;
} UNWIND_CODE, *PUNWIND_CODE;

//
// Define unwind information flags.
//

#define UNW_FLAG_NHANDLER 0x0
#define UNW_FLAG_EHANDLER 0x1
#define UNW_FLAG_UHANDLER 0x2
#define UNW_FLAG_CHAININFO 0x4

//
// Define unwind information structure.
//

typedef struct _UNWIND_INFO {
    UCHAR Version : 3;
    UCHAR Flags : 5;
    UCHAR SizeOfProlog;
    UCHAR CountOfCodes;
    UCHAR FrameRegister : 4;
    UCHAR FrameOffset : 4;
    UNWIND_CODE UnwindCode[1];

//
// The unwind codes are followed by an optional DWORD aligned field that
// contains the exception handler address or a function table entry if
// chained unwind information is specified. If an exception handler address
// is specified, then it is followed by the language specified exception
// handler data.
//
//  union {
//      struct {
//          ULONG ExceptionHandler;
//          ULONG ExceptionData[];
//      };
//
//      RUNTIME_FUNCTION FunctionEntry;
//  };
//

} UNWIND_INFO, *PUNWIND_INFO;

// begin_winnt
//
// Define function table entry - a function table entry is generated for
// each frame function.
//

#define RUNTIME_FUNCTION_INDIRECT 0x1

typedef struct _RUNTIME_FUNCTION {
    ULONG BeginAddress;
    ULONG EndAddress;
    ULONG UnwindData;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;

// end_winnt
//
// Scope table structure definition.
//

typedef struct _SCOPE_TABLE {
    ULONG Count;
    struct
    {
        ULONG BeginAddress;
        ULONG EndAddress;
        ULONG HandlerAddress;
        ULONG JumpTarget;
    } ScopeRecord[1];
} SCOPE_TABLE, *PSCOPE_TABLE;

// begin_winnt
//
// Define dynamic function table entry.
//

typedef
PRUNTIME_FUNCTION
(*PGET_RUNTIME_FUNCTION_CALLBACK) (
    IN ULONG64 ControlPc,
    IN PVOID Context
    );

// end_winnt

typedef enum _FUNCTION_TABLE_TYPE {
    RF_SORTED,
    RF_UNSORTED,
    RF_CALLBACK
} FUNCTION_TABLE_TYPE;

typedef struct _DYNAMIC_FUNCTION_TABLE {
    LIST_ENTRY ListEntry;
    PRUNTIME_FUNCTION FunctionTable;
    LARGE_INTEGER TimeStamp;
    ULONG64 MinimumAddress;
    ULONG64 MaximumAddress;
    ULONG64 BaseAddress;
    PGET_RUNTIME_FUNCTION_CALLBACK Callback;
    PVOID Context;
    PWSTR OutOfProcessCallbackDll;
    FUNCTION_TABLE_TYPE Type;
    ULONG EntryCount;
} DYNAMIC_FUNCTION_TABLE, *PDYNAMIC_FUNCTION_TABLE;

// begin_winnt

typedef
NTSTATUS
(*POUT_OF_PROCESS_FUNCTION_TABLE_CALLBACK) (
    IN HANDLE Process,
    IN PVOID TableAddress,
    OUT PULONG Entries,
    OUT PRUNTIME_FUNCTION* Functions
    );

#define OUT_OF_PROCESS_FUNCTION_TABLE_CALLBACK_EXPORT_NAME \
    "OutOfProcessFunctionTableCallback"

// end_winnt
//
// Define unwind history table structure.
//

#define UNWIND_HISTORY_TABLE_SIZE 12

typedef struct _UNWIND_HISTORY_TABLE_ENTRY {
        ULONG64 ImageBase;
        PRUNTIME_FUNCTION FunctionEntry;
} UNWIND_HISTORY_TABLE_ENTRY, *PUNWIND_HISTORY_TABLE_ENTRY;

#define UNWIND_HISTORY_TABLE_NONE 0
#define UNWIND_HISTORY_TABLE_GLOBAL 1
#define UNWIND_HISTORY_TABLE_LOCAL 2

typedef struct _UNWIND_HISTORY_TABLE {
        ULONG Count;
        UCHAR Search;
        ULONG64 LowAddress;
        ULONG64 HighAddress;
        UNWIND_HISTORY_TABLE_ENTRY Entry[UNWIND_HISTORY_TABLE_SIZE];
} UNWIND_HISTORY_TABLE, *PUNWIND_HISTORY_TABLE;

//
// Define exception dispatch context structure.
//

typedef struct _DISPATCHER_CONTEXT {
    ULONG64 ControlPc;
    ULONG64 ImageBase;
    PRUNTIME_FUNCTION FunctionEntry;
    ULONG64 EstablisherFrame;
    ULONG64 TargetIp;
    PCONTEXT ContextRecord;
    PEXCEPTION_ROUTINE LanguageHandler;
    PVOID HandlerData;
    PUNWIND_HISTORY_TABLE HistoryTable;
    ULONG ScopeIndex;
    ULONG Fill0;
} DISPATCHER_CONTEXT, *PDISPATCHER_CONTEXT;

// begin_winnt
//
// Define runtime exception handling prototypes.
//

NTSYSAPI
VOID
__cdecl
RtlRestoreContext (
    IN PCONTEXT ContextRecord,
    IN struct _EXCEPTION_RECORD *ExceptionRecord OPTIONAL
    );

// end_winnt

VOID
RtlInitializeHistoryTable (
    VOID
    );

NTSYSAPI
PRUNTIME_FUNCTION
RtlLookupFunctionEntry (
    IN ULONG64 ControlPc,
    OUT PULONG64 ImageBase,
    IN OUT PUNWIND_HISTORY_TABLE HistoryTable OPTIONAL
    );

NTSYSAPI
PRUNTIME_FUNCTION
RtlLookupFunctionTable (
    IN PVOID ControlPc,
    OUT PVOID *ImageBase,
    OUT PULONG SizeOfTable
    );

NTSYSAPI
PLIST_ENTRY
RtlGetFunctionTableListHead (
    VOID
    );

#if defined(_AMD64_)

// begin_winnt

NTSYSAPI
BOOLEAN
__cdecl
RtlAddFunctionTable (
    IN PRUNTIME_FUNCTION FunctionTable,
    IN ULONG EntryCount,
    IN ULONG64 BaseAddress
    );

NTSYSAPI
BOOLEAN
__cdecl
RtlInstallFunctionTableCallback (
    IN ULONG64 TableIdentifier,
    IN ULONG64 BaseAddress,
    IN ULONG Length,
    IN PGET_RUNTIME_FUNCTION_CALLBACK Callback,
    IN PVOID Context,
    IN PCWSTR OutOfProcessCallbackDll OPTIONAL
    );

NTSYSAPI
BOOLEAN
__cdecl
RtlDeleteFunctionTable (
    IN PRUNTIME_FUNCTION FunctionTable
    );

#endif // _AMD64_

// end_winnt

NTSYSAPI
PEXCEPTION_ROUTINE
RtlVirtualUnwind (
    IN ULONG HandlerType,
    IN ULONG64 ImageBase,
    IN ULONG64 ControlPc,
    IN PRUNTIME_FUNCTION FunctionEntry,
    IN OUT PCONTEXT ContextRecord,
    OUT PVOID *HandlerData,
    OUT PULONG64 EstablisherFrame,
    IN OUT PKNONVOLATILE_CONTEXT_POINTERS ContextPointers OPTIONAL
    );

//
// Define exception filter and termination handler function types.
//

typedef
LONG
(*PEXCEPTION_FILTER) (
    struct _EXCEPTION_POINTERS *ExceptionPointers,
    PVOID EstablisherFrame
    );

typedef
VOID
(*PTERMINATION_HANDLER) (
    BOOLEAN AbnormalTermination,
    PVOID EstablisherFrame
    );

//
// Additional information supplied in QuerySectionInformation for images.
//

#define SECTION_ADDITIONAL_INFO_USED 0

#ifdef __cplusplus
}
#endif

#endif // _NTAMD64_  

