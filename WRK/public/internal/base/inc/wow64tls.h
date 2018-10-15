/*++

Copyright (c) Microsoft Corporation. All rights reserved.

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    wow64tls.h

Abstract:

    TLS slot definitions.

--*/

#ifndef _WOW64TLS_INCLUDE
#define _WOW64TLS_INCLUDE


//
// Thread Local Storage (TLS) support.  TLS slots are statically allocated.
//

#define WOW64_TLS_STACKPTR64                0   // contains 64-bit stack ptr when simulating 32-bit code
#define WOW64_TLS_CPURESERVED               1   // per-thread data for the CPU simulator
#define WOW64_TLS_INCPUSIMULATION           2   // Set when inside the CPU
#define WOW64_TLS_TEMPLIST                  3   // List of memory allocated in thunk call.
#define WOW64_TLS_EXCEPTIONADDR             4   // 32-bit exception address (used during exception unwinds)
#define WOW64_TLS_USERCALLBACKDATA          5   // Used by win32k callbacks
#define WOW64_TLS_EXTENDED_FLOAT            6   // Used in ia64 to pass in floating point
#define WOW64_TLS_APCLIST	            7	// List of outstanding usermode APCs
#define WOW64_TLS_FILESYSREDIR	            8	// Used to enable/disable the filesystem redirector
#define WOW64_TLS_LASTWOWCALL	            9	// Pointer to the last wow call struct (Used when wowhistory is enabled)
#define WOW64_TLS_WOW64INFO                 10  // Wow64Info address (structure shared between 32-bit and 64-bit code inside Wow64).
#define WOW64_TLS_INITIAL_TEB32             11  // A pointer to the 32-bit initial TEB
#define WOW64_TLS_PERFDATA                  12  // A pointer to temporary timestamps used in perf measurement
#define WOW64_TLS_DEBUGGER_COMM             13  // Communicate with 32bit debugger for event notification
#define WOW64_TLS_INVALID_STARTUP_CONTEXT   14  // Used by IA64 to indicate an invalid startup context. After startup, it stores a pointer to the context.
#define WOW64_TLS_SLIST_FAULT               15  // Used to retry RtlpInterlockedPopEntrySList faults
#define WOW64_TLS_UNWIND_NATIVE_STACK       16  // Forces an unwind of the native 64-bit stack after an APC
#define WOW64_TLS_APC_WRAPPER               17  // Holds the Wow64 APC jacket routine
#define WOW64_TLS_IN_SUSPEND_THREAD         18  // Indicates the current thread is in the middle of NtSuspendThread. Used by software CPUs.
#define WOW64_TLS_MAX_NUMBER                19  // Maximum number of TLS slot entries to allocate
//
// VOID Wow64TlsSetValue(DWORD dwIndex, LPVOID lpTlsValue);
//

#define Wow64TlsSetValue(dwIndex, lpTlsValue)   \
    NtCurrentTeb()->TlsSlots[dwIndex] = lpTlsValue;

//
// LPVOID Wow64TlsGetValue(DWORD dwIndex);
//

#define Wow64TlsGetValue(dwIndex)               \
    (NtCurrentTeb()->TlsSlots[dwIndex])

#endif  // _WOW64TLS_INCLUDE

