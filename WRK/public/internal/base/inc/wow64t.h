/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved.

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    wow64t.h

Abstract:

    32-bit structure definitions for 64-bit NT.

--*/

#ifndef _WOW64T_
#define _WOW64T_


//
// Wow64 TLS-slots definitions
//

#include "wow64tls.h"


#if _MSC_VER > 1000
#pragma once
#endif

//
// X86-processor supported features
//
// The following features are considered:
// - Address Windowing Extension (AWE)
// - LARGE_PAGE Allocations
// - WriteWatch pages
// - Read/Write Scatter/Gather IO
//

#if defined(_WIN64)
#if defined(_AMD64_)
#define WOW64_IS_LARGE_PAGES_SUPPORTED()            (TRUE)
#define WOW64_IS_AWE_SUPPORTED()                    (TRUE)
#define WOW64_IS_RDWR_SCATTER_GATHER_SUPPORTED()    (TRUE)
#define WOW64_IS_WRITE_WATCH_SUPPORTED()            (TRUE)
#define _WOW64_ALIGN_LARGE_INTEGER                  0
#define WOW64_UNALIGNED
#else
#error "No Target Architecture"
#endif
#endif

//
// Page size on x86 NT
//

#define PAGE_SIZE_X86NT    0x1000
#define PAGE_SHIFT_X86NT   12L
#define WOW64_SPLITS_PER_PAGE (PAGE_SIZE / PAGE_SIZE_X86NT)

//
// Convert the number of native pages to sub x86-pages
//

#define Wow64GetNumberOfX86Pages(NativePages)    \
        (NativePages * (PAGE_SIZE >> PAGE_SHIFT_X86NT))
        
//
// Macro to round to the nearest page size
//

#define WOW64_ROUND_TO_PAGES(Size)  \
        (((ULONG_PTR)(Size) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
        
//
// Get number of native pages
//

#define WOW64_BYTES_TO_PAGES(Size)  (((ULONG)(Size) >> PAGE_SHIFT) + \
                                     (((ULONG)(Size) & (PAGE_SIZE - 1)) != 0))

//
// The name of the 32-bit system directory, which is a child of %SystemRoot%
//

#define WOW64_SYSTEM_DIRECTORY      "SysWOW64"
#define WOW64_SYSTEM_DIRECTORY_U   L"SysWOW64"

// Length in bytes of the new system directory, not counting a
// null terminator
//

#define WOW64_SYSTEM_DIRECTORY_SIZE (sizeof(WOW64_SYSTEM_DIRECTORY)-sizeof(CHAR))
#define WOW64_SYSTEM_DIRECTORY_U_SIZE (sizeof(WOW64_SYSTEM_DIRECTORY_U)-sizeof(WCHAR))

//
// IA64 delayed debugger notification support
//

#define WOW64_DEBUG_SIGNATURE_32BIT_DBG         0xABCDABDE000      // 32bit debugger is there
#define WOW64_DEBUG_EVENT_64BIT_DLL_UNLOAD      0x00000000001      // 64bit dll unload event not in use
#define WOW64_DEBUG_EVENT_32BIT_DLL_UNLOAD      0x00000000002      // 32bit DLL unload notification
#define WOW64_DEBUG_EVENT_DELAY_CREATE_PROCESS  0x00000000004      // Delay CreateProcess Event
#define WOW64_DEBUG_EVENT_DELAY_DLLLOAD         0x00000000008      // DElay Dll Load Event
#define WOW64_DEBUG_EVENT_NOTIFY_WX86_DONE      0x00000000010      // Notify FMT Image is done
#define WOW64_STATUS_WX86_FMT_DONE              0xCABCDE10

//
// Wow64 shared information 
//

typedef enum _WOW64_SHARED_INFORMATION 
{
    SharedNtdll32LdrInitializeThunk,
    SharedNtdll32KiUserExceptionDispatcher,
    SharedNtdll32KiUserApcDispatcher,
    SharedNtdll32KiUserCallbackDispatcher,
    SharedNtdll32LdrHotPatchRoutine,
    SharedNtdll32ExpInterlockedPopEntrySListFault,
    SharedNtdll32ExpInterlockedPopEntrySListResume,
    SharedNtdll32ExpInterlockedPopEntrySListEnd,
    SharedNtdll32Reserved2,
    Wow64SharedPageEntriesCount
} WOW64_SHARED_INFORMATION;

#define Wow64GetSharedInformation(entry)    \
        (USER_SHARED_DATA->Wow64SharedInformation [entry])

#if !defined(SORTPP_PASS) && !defined(MIDL_PASS) && !defined(RC_INVOKED)
C_ASSERT (Wow64SharedPageEntriesCount <= MAX_WOW64_SHARED_ENTRIES);
#endif


//
// Turbo-Thunk data structure definition
//

typedef enum _WOW64_TURBO_SERVICE_TYPE {
    ServiceNoTurbo = 0,
    Service0Arg,
    Service0ArgReloadState,
    Service1ArgSp,
    Service1ArgNSp,
    Service2ArgNSpNSp,
    Service2ArgNSpNSpReloadState,
    Service2ArgSpNSp,
    Service2ArgSpSp,
    Service2ArgNSpSp,
    Service3ArgNSpNSpNSp,
    Service3ArgSpSpSp,
    Service3ArgSpNSpNSp,
    Service3ArgSpNSpNSpReloadState,
    Service3ArgSpSpNSp,
    Service3ArgNSpSpNSp,
    Service3ArgSpNSpSp,
    Service4ArgNSpNSpNSpNSp,
    Service4ArgSpSpNSpNSp,
    Service4ArgSpSpNSpNSpReloadState,
    Service4ArgSpNSpNSpNSp,
    Service4ArgSpNSpNSpNSpReloadState,
    Service4ArgNSpSpNSpNSp,
    Service4ArgSpSpSpNSp,
    ServiceCpupTdQuerySystemTime,
    ServiceCpupTdGetCurrentProcessorNumber,
    ServiceCpupTdReadWriteFile,
    ServiceCpupTdDeviceIoControlFile,
    ServiceCpupTdRemoveIoCompletion,
    ServiceCpupTdWaitForMultipleObjects,
    ServiceCpupTdWaitForMultipleObjects32,
    Wow64ServiceTypesCount
    
} WOW64_TURBO_SERVICE_TYPE;

typedef union _TURBO_THUNK_DESCRIPTION {
     struct {
         UCHAR ServiceType;
     };
     
} TURBO_THUNK_DESCRIPTION, *PTURBO_THUNK_DESCRIPTION;

//
// Wow64 Registry Configuration 
//

//
// Wow64 Execution Flags 
//
//  31   28 27          15     11  8 7           0
// +---------------------------------------------+
// |   4   |       13   |1|1|1|  4  |     8      |
// +---------------------------------------------+
//     |           |     | | |   |        |    
//     |           |     | | |   |        +---------> 64-bit Stack reserve (native PAGE_SIZE multiples) 
//     |           |     | | |   |    
//     |           |     | | |   +-----------> Initial 64-bit Stack commit (native PAGE_SIZE multiples)
//     |           |     | | |
//     |           |     | | +-------> Log event to eventlog when launching a wow64 process
//     |           |     | |
//     |           |     | +----> Disable assert messages
//     |           |     |
//     |           |     +---> Disable turbo dispatch (used only for forked/posix processes)
//     |           |
//     |           +----------------> Unused bits (15 bits)
//     |
//     |
//     +-----------> Reserved bits (4 bits)
//
//
//

typedef union _WOW64_EXECUTE_OPTIONS {
    
    ULONG Flags;
    
    struct {
        
        ULONG StackReserveSize              : 8;
        ULONG StackCommitSize               : 4;
        ULONG LogLaunchEvent                : 1;
        ULONG DisableWowAssert              : 1;
        ULONG DisableTurboDispatch          : 1;
        ULONG DisableDebugRegistersSwitch   : 1;
        ULONG Unused                        : 12;
        ULONG Reserved0                     : 1;
        ULONG Reserved1                     : 1;
        ULONG Reserved2                     : 1;
        ULONG Reserved3                     : 1;
    };
} WOW64_EXECUTE_OPTIONS, *PWOW64_EXECUTE_OPTIONS;

#define WOW64_DEFAULT_EXECUTE_OPTIONS           0
#define WOW64_REGISTRY_CONFIG_ROOT              L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\WOW64"
#define WOW64_CONFIG_EXECUTE_OPTIONS            L"Wow64ExecuteFlags"

#define WOW64_X86_TAG               " (x86)"
#define WOW64_X86_TAG_U            L" (x86)"

//
// File system redirection values
//

#define WOW64_FILE_SYSTEM_ENABLE_REDIRECT          (UlongToPtr(0x00))   // enable file-system redirection for the currently executing thread
#define WOW64_FILE_SYSTEM_DISABLE_REDIRECT         (UlongToPtr(0x01))   // disable file-system redirection for the currently executing thread
#define WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY  ((PVOID)L"[<__wow64_disable_redirect_all__]>")


#define TYPE32(x)   ULONG
#define TYPE64(x)   ULONGLONG


#if !_WIN64
__inline
void *
ULonglongToPtr(
    const ULONGLONG ull
    )
{
#pragma warning (push)
#pragma warning (disable : 4305)
    return((void *) ull );
#pragma warning (pop)
}
#endif

//
// Wow64Info structure is shared between 32-bit and 64-bit modules inside a Wow64 process.
// NOTE : This structure shouldn't contain any pointer-dependent data, as 
// it is viewed from 32-bit and 64-bit code.
//

#define WOW64_CPUFLAGS_MSFT64           0x00000001
#define WOW64_CPUFLAGS_SOFTWARE         0x00000002


#if defined(_AMD64_)
#define Wow64pRunningSoftwareCpu(cpufl) (FALSE)
#else
#define Wow64pRunningSoftwareCpu(cpufl) (cpufl & WOW64_CPUFLAGS_SOFTWARE)
#endif


typedef struct _WOW64INFO {

    ULONG NativeSystemPageSize;         // Page size of the native system the emulator is running on.
    
    ULONG CpuFlags;
    
    WOW64_EXECUTE_OPTIONS Wow64ExecuteFlags;
    
} WOW64INFO, *PWOW64INFO;


typedef struct _PEB_LDR_DATA32 {
    ULONG Length;
    BOOLEAN Initialized;
    TYPE32(HANDLE) SsHandle;
    LIST_ENTRY32 InLoadOrderModuleList;
    LIST_ENTRY32 InMemoryOrderModuleList;
    LIST_ENTRY32 InInitializationOrderModuleList;
    TYPE32(PVOID) EntryInProgress;
} PEB_LDR_DATA32, *PPEB_LDR_DATA32;

typedef struct _GDI_TEB_BATCH32 {
    ULONG    Offset;
    TYPE32(ULONG_PTR) HDC;
    ULONG    Buffer[GDI_BATCH_BUFFER_SIZE];
} GDI_TEB_BATCH32,*PGDI_TEB_BATCH32;


typedef struct _GDI_TEB_BATCH64 {
    ULONG    Offset;
    TYPE64(ULONG_PTR) HDC;
    ULONG    Buffer[GDI_BATCH_BUFFER_SIZE];
} GDI_TEB_BATCH64,*PGDI_TEB_BATCH64;


typedef struct _Wx86ThreadState32 {
    TYPE32(PULONG)  CallBx86Eip;
    TYPE32(PVOID)   DeallocationCpu;
    BOOLEAN UseKnownWx86Dll;
    char    OleStubInvoked;
} WX86THREAD32, *PWX86THREAD32;

typedef struct _Wx86ThreadState64 {
    TYPE64(PULONG)  CallBx86Eip;
    TYPE64(PVOID)   DeallocationCpu;
    BOOLEAN UseKnownWx86Dll;
    char    OleStubInvoked;
} WX86THREAD64, *PWX86THREAD64;

typedef struct _CLIENT_ID32 {
    TYPE32(HANDLE)  UniqueProcess;
    TYPE32(HANDLE)  UniqueThread;
} CLIENT_ID32;

typedef CLIENT_ID32 *PCLIENT_ID32;

#if !defined(CLIENT_ID64_DEFINED)

typedef struct _CLIENT_ID64 {
    TYPE64(HANDLE)  UniqueProcess;
    TYPE64(HANDLE)  UniqueThread;
} CLIENT_ID64;

typedef CLIENT_ID64 *PCLIENT_ID64;

#define CLIENT_ID64_DEFINED

#endif

typedef ULONG GDI_HANDLE_BUFFER32[GDI_HANDLE_BUFFER_SIZE32];
typedef ULONG GDI_HANDLE_BUFFER64[GDI_HANDLE_BUFFER_SIZE64];

#define PEBTEB_BITS 32
#include "pebteb.h"
#undef PEBTEB_BITS

#define PEBTEB_BITS 64
#include "pebteb.h"
#undef PEBTEB_BITS

#if !defined(SORTPP_PASS) && !defined(MIDL_PASS) && !defined(RC_INVOKED) && !defined(_X86AMD64_) && !defined(WOW64EXTS_386)
C_ASSERT(FIELD_OFFSET(TEB32, GdiTebBatch) == 0x1d4);
C_ASSERT(FIELD_OFFSET(TEB64, GdiTebBatch) == 0x2f0);
#endif


#if !defined(BUILD_WOW6432)

//
// Get the 32-bit TEB without doing a memory reference.
//

#define WOW64_GET_TEB32_SAFE(teb64) \
        ((PTEB32) ((ULONGLONG)teb64 + WOW64_ROUND_TO_PAGES (sizeof (TEB))))
        
#define WOW64_GET_TEB32(teb64) \
        WOW64_GET_TEB32_SAFE(teb64)

//
// Update the first qword in the 64-bit TEB.  The 32-bit rdteb instruction
// reads the TEB32 pointer value directly from this field.
//
#define WOW64_SET_TEB32(teb64, teb32) \
   (teb64)->NtTib.ExceptionList = (struct _EXCEPTION_REGISTRATION_RECORD *)(teb32);


#define WOW64_TEB32_POINTER_ADDRESS(teb64) \
        (PVOID)&((teb64)->NtTib.ExceptionList)


#endif

#if defined(_AMD64_)

#define WOW64P_EXCEPTION_RECORD_SOURCE32        (0x24681357)

BOOLEAN
FORCEINLINE
Wow64pIsExceptionRecordSource32 (PEXCEPTION_RECORD ExceptionRecord64)
{
    PEXCEPTION_RECORD64 Exr64 = (PEXCEPTION_RECORD64) ExceptionRecord64;
    
    return (BOOLEAN)(Exr64->__unusedAlignment == WOW64P_EXCEPTION_RECORD_SOURCE32);
}

VOID
FORCEINLINE
Wow64pMarkExceptionRecordSource32 (PEXCEPTION_RECORD ExceptionRecord64)
{
    PEXCEPTION_RECORD64 Exr64 = (PEXCEPTION_RECORD64) ExceptionRecord64;
    
    if (ARGUMENT_PRESENT (Exr64)) {
        Exr64->__unusedAlignment = (WOW64P_EXCEPTION_RECORD_SOURCE32);
    }    
    return;
}

VOID
FORCEINLINE
Wow64pClearExceptionRecordSource32 (PEXCEPTION_RECORD ExceptionRecord64)
{
    PEXCEPTION_RECORD64 Exr64 = (PEXCEPTION_RECORD64) ExceptionRecord64;
    
    if (ARGUMENT_PRESENT (Exr64)) {
        Exr64->__unusedAlignment = 0;
    }    
    return;    
}

#else

#define Wow64pIsExceptionRecordSource32(Exr64)          (FALSE)
#define Wow64pMarkExceptionRecordSource32(Exr64)
#define Wow64pClearExceptionRecordSource32(Exr64)

#endif

//
// Thunk macros

#define UStr32ToUStr(dst, src) { (dst)->Length = (src)->Length; \
                                 (dst)->MaximumLength = (src)->MaximumLength; \
                                 (dst)->Buffer = (PWSTR) UlongToPtr ((src)->Buffer); }

#define UStrToUStr32(dst, src) { (dst)->Length = (src)->Length; \
                                 (dst)->MaximumLength = (src)->MaximumLength; \
                                 (dst)->Buffer = (ULONG) PtrToUlong ((src)->Buffer); }

#define NtCurrentTeb32()  ((PTEB32) WOW64_GET_TEB32_SAFE (NtCurrentTeb ()))
#define NtCurrentPeb32()  ((PPEB32) UlongToPtr ((NtCurrentTeb32()->ProcessEnvironmentBlock)) )

//
// This is currently defined in windows\core\w32inc\w32wow64.h:
//

#define NtCurrentTeb64()   ((PTEB64)((PTEB32)NtCurrentTeb())->GdiBatchCount)

#define NtCurrentPeb64()   ((PPEB64)NtCurrentTeb64()->ProcessEnvironmentBlock) 

//
// Wow64 file-system redirection support.
//

//
// These should only be called from Win32 code known to be running on Win64.
//

#if !defined(_WIN64)
#define Wow64EnableFilesystemRedirector()   \
    NtCurrentTeb64()->TlsSlots[WOW64_TLS_FILESYSREDIR] = 0;
    
#define Wow64DisableFilesystemRedirector(filename)  \
    NtCurrentTeb64()->TlsSlots[WOW64_TLS_FILESYSREDIR] = (ULONGLONG)PtrToUlong(filename);


FORCEINLINE
PVOID 
Wow64SetFilesystemRedirectorEx (
    __in PVOID NewValue
    )
/*++

Routine Description:

    This routine allows a thread running inside Wow64 to disable file-system 
    redirection for all calls happening in the context of this thread.
    
    
    NOTE: This routine should only called from a wow64 process, and is only available 
          when running on .NET server platforms and beyond. If you component will 
          run on downlevel platforms (XP 2600 for example), you shouldn't use WOW64_FILE_SYSTEM_DISABLE_REDIRECT (see below).

Example (Enumerating files under c:\windows\system32):
    
    {
        HANDLE File;
        WIN32_FIND_DATA FindData;
#ifndef _WIN64        
        BOOL bWow64Process = FALSE;
        PVOID Wow64RedirectionOld;
#endif        

        //
        // Disable Wow64 file system redirection
        //
#ifndef _WIN64        
        IsWow64Process (GetCurrentProcess (), &bWow64Process);
        if (bWow64Process == TRUE) {
            Wow64RedirectionOld = Wow64SetFilesystemRedirectorEx (WOW64_FILE_SYSTEM_DISABLE_REDIRECT);
        }
#endif        
        File = FindFirstFileA ("c:\\windows\\system32\\*.*", &FindData);
        
        do {
        .
        .
        } while (FindNextFileA (File, &FindData) != 0);
        
        FindClose (File);
        
        //
        // Enable Wow64 file-system redirection
        //
#ifndef _WIN64        
        if (bWow64Process == TRUE) {
            Wow64SetFilesystemRedirectorEx (Wow64RedirectionOld);
        }
#endif        
    }


Arguments:

    NewValue - New Wow64 file-system redirection value. This can either be:
               a- pointer to a unicode string with a fully-qualified path name (e.g. L"c:\\windows\\notepad.exe").
               b- any of the following predefined values :
                  * WOW64_FILE_SYSTEM_ENABLE_REDIRECT : Enables file-system redirection (default)
                  * WOW64_FILE_SYSTEM_DISABLE_REDIRECT : Disables file-system redirection on all
                    file I/O operations happening within the context of the current thread.
                  * WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY: Use this only if you want to run on 
                    download level platforms (for example XP 2600), as it will have no effect
                    and prevents your program from malfunctioning.
    
Return:

    Old Wow64 file-system redirection value

--*/
{
    PVOID OldValue;
    OldValue = (PVOID)(ULONG_PTR)NtCurrentTeb64()->TlsSlots[WOW64_TLS_FILESYSREDIR];
    NtCurrentTeb64()->TlsSlots[WOW64_TLS_FILESYSREDIR] = (ULONGLONG)PtrToUlong(NewValue);
    return OldValue;
}

//
// Wow64Info is accessed only from compiled code for x86 on win64.
// NOTE: Only Wow64 processes are allowed to call these macros.
//

#define Wow64GetSharedInfo()    ((PWOW64INFO)NtCurrentTeb64()->TlsSlots[WOW64_TLS_WOW64INFO])

#define Wow64GetSystemNativePageSize() \
    ((PWOW64INFO)ULonglongToPtr((NtCurrentTeb64()->TlsSlots[WOW64_TLS_WOW64INFO])))->NativeSystemPageSize
    
#define Wow64UnwindNativeThreadStack() (NtCurrentTeb64()->TlsSlots[WOW64_TLS_UNWIND_NATIVE_STACK] = 1)

#define Wow64KiWrapApcProc(ApcContext,ApcRoutine)

#else

#define Wow64GetSharedInfo()                    ((PWOW64INFO)NtCurrentTeb()->TlsSlots[WOW64_TLS_WOW64INFO])
#define Wow64GetInitialTeb32()                  ((PINITIAL_TEB)NtCurrentTeb()->TlsSlots[WOW64_TLS_INITIAL_TEB32])
#define Wow64SetInvalidStartupContext(teb64,x)  (teb64->TlsSlots[WOW64_TLS_INVALID_STARTUP_CONTEXT] = UlongToPtr ((ULONG) x))
#define Wow64UnwindNativeThreadStack()          (NtCurrentTeb()->TlsSlots[WOW64_TLS_UNWIND_NATIVE_STACK] = UlongToPtr (1))


FORCEINLINE
VOID
Wow64KiWrapApcProc (
    __inout PVOID *ApcContext,
    __inout PVOID *ApcRoutine)

/*++

Routine Description:

    This routine is used by kernel mode callers to queue APCs to a thread
    running inside a Wow64 process. It wraps the original APC routine
    with a jacket routine inside Wow64. The target Apc routine must be
    inside 32-bit code. This routine must be executed in the context of the 
    Wow64 thread where the target APC is going to run in.
    
    This routine, as it accesses a TEB-relative offset, has to be executed
    within a try/except block. 
    
Environment:
    
    Kernel mode only.
        
Arguments:
    
    ApcContext - Pointer to the original ApcContext parameter.
    
    ApcRoutine - Pointer to the original ApcRoutine that is targeted to run
        32-bit code.
    
Return:

    None.

--*/
{
    *ApcContext = (PVOID)((ULONG_PTR)*ApcContext | ((ULONG_PTR)*ApcRoutine << 32));
    *ApcRoutine = Wow64TlsGetValue (WOW64_TLS_APC_WRAPPER);
}        

#endif


//
// Macros for reading the INCPUSIMULATION flag
//

#if defined(_AMD64_)

#define Wow64SetCpuSimulationFlag(NewValue)                
#define Wow64GetCpuSimulationFlag()             (NULL)

#else

#define Wow64SetCpuSimulationFlag(NewValue)     \
        Wow64TlsSetValue(WOW64_TLS_INCPUSIMULATION, (PVOID)NewValue)
        
#define Wow64GetCpuSimulationFlag()             \
        Wow64TlsGetValue(WOW64_TLS_INCPUSIMULATION)        
#endif


typedef ULONGLONG SIZE_T64, *PSIZE_T64;

#if defined(BUILD_WOW6432)

typedef VOID * __ptr64 NATIVE_PVOID;
typedef ULONG64 NATIVE_ULONG_PTR;
typedef SIZE_T64 NATIVE_SIZE_T;
typedef PSIZE_T64 PNATIVE_SIZE_T;
typedef struct _PEB64 NATIVE_PEB;
typedef struct _PROCESS_BASIC_INFORMATION64 NATIVE_PROCESS_BASIC_INFORMATION;
typedef struct _MEMORY_BASIC_INFORMATION64 NATIVE_MEMORY_BASIC_INFORMATION;

#else

typedef ULONG_PTR NATIVE_ULONG_PTR;
typedef SIZE_T NATIVE_SIZE_T;
typedef PSIZE_T PNATIVE_SIZE_T;
typedef PVOID NATIVE_PVOID;
typedef struct _PEB NATIVE_PEB;
typedef struct _PROCESS_BASIC_INFORMATION NATIVE_PROCESS_BASIC_INFORMATION;
typedef struct _MEMORY_BASIC_INFORMATION NATIVE_MEMORY_BASIC_INFORMATION;

#endif

#endif  // _WOW64T_

