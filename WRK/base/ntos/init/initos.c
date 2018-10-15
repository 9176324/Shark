/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    initos.c

Abstract:

    Main source file of the NTOS system initialization subcomponent.

--*/

#include "ntos.h"
#include "ntimage.h"
#include <zwapi.h>
#include <ntdddisk.h>
#include <kddll.h>
#include <setupblk.h>
#include <fsrtl.h>

#include "stdlib.h"
#include "stdio.h"
#include <string.h>

#include <inbv.h>

UNICODE_STRING NtSystemRoot;

NTSTATUS
RtlFormatBuffer2 (
    PCHAR   Buffer,
    size_t  BufferLen,
    PCHAR   FormatString,
    PVOID   Param0,
    PVOID   Param1
    );

VOID
ExpInitializeExecutive(
    IN ULONG Number,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTKERNELAPI
BOOLEAN
ExpRefreshTimeZoneInformation(
    IN PLARGE_INTEGER CurrentUniversalTime
    );

NTSTATUS
CreateSystemRootLink(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

static USHORT
NameToOrdinal (
    IN PSZ NameOfEntryPoint,
    IN ULONG_PTR DllBase,
    IN ULONG NumberOfNames,
    IN PULONG NameTableBase,
    IN PUSHORT NameOrdinalTableBase
    );

NTSTATUS
LookupEntryPoint (
    IN PVOID DllBase,
    IN PSZ NameOfEntryPoint,
    OUT PVOID *AddressOfEntryPoint
    );

PFN_COUNT
ExBurnMemory(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PFN_COUNT NumberOfPagesToBurn,
    IN TYPE_OF_MEMORY MemoryTypeForRemovedPages,
    IN PMEMORY_ALLOCATION_DESCRIPTOR NewMemoryDescriptor OPTIONAL
    );

BOOLEAN
ExpIsLoaderValid(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
FinalizeBootLogo(VOID);

VOID
DisplayBootBitmap (
    IN BOOLEAN DisplayOnScreen
    );

NTSTATUS
RtlInitializeStackTraceDataBase (
    IN PVOID CommitBase,
    IN SIZE_T CommitSize,
    IN SIZE_T ReserveSize
    );


#if defined(_X86_)

VOID
KiInitializeInterruptTimers(
    VOID
    );

#endif

VOID
Phase1InitializationDiscard (
    IN PVOID Context
    );

VOID
CmpInitSystemVersion(
    IN ULONG stage,
    IN PVOID param
    );

VOID
HeadlessInit(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

//
// The INIT section is not pageable during initialization, so these
// functions can be in INIT rather than in .text.
//

#pragma alloc_text(INIT, ExpInitializeExecutive)
#pragma alloc_text(PAGE, Phase1Initialization)
#pragma alloc_text(INIT, Phase1InitializationDiscard)
#pragma alloc_text(INIT, CreateSystemRootLink)

#if !defined(_AMD64_)

#pragma alloc_text(INIT,LookupEntryPoint)
#pragma alloc_text(INIT,NameToOrdinal)

#endif

//
// Define global static data used during initialization.
//

ULONG NtGlobalFlag;
extern UCHAR CmProcessorMismatch;

ULONG InitializationPhase;

extern ULONG KiServiceLimit;
extern PMESSAGE_RESOURCE_DATA  KiBugCodeMessages;
extern ULONG KdpTimeSlipPending;
extern BOOLEAN KdBreakAfterSymbolLoad;

extern PVOID BBTBuffer;
extern MEMORY_ALLOCATION_DESCRIPTOR BBTMemoryDescriptor;

extern BOOLEAN InbvBootDriverInstalled;

WCHAR NtInitialUserProcessBuffer[128] = L"\\SystemRoot\\System32\\smss.exe";
ULONG NtInitialUserProcessBufferLength =
    sizeof(NtInitialUserProcessBuffer) - sizeof(WCHAR);
ULONG NtInitialUserProcessBufferType = REG_SZ;


typedef struct _EXLOCK {
    KSPIN_LOCK SpinLock;
    KIRQL Irql;
} EXLOCK, *PEXLOCK;

#ifdef ALLOC_PRAGMA
NTSTATUS
ExpInitializeLockRoutine(
    PEXLOCK Lock
    );
#pragma alloc_text(INIT,ExpInitializeLockRoutine)
#endif

BOOLEAN
ExpOkayToLockRoutine(
    IN PEXLOCK Lock
    )
{
    return TRUE;
}

NTSTATUS
ExpInitializeLockRoutine(
    PEXLOCK Lock
    )
{
    KeInitializeSpinLock(&Lock->SpinLock);
    return STATUS_SUCCESS;
}

NTSTATUS
ExpAcquireLockRoutine(
    PEXLOCK Lock
    )
{
    ExAcquireSpinLock(&Lock->SpinLock,&Lock->Irql);
    return STATUS_SUCCESS;
}

NTSTATUS
ExpReleaseLockRoutine(
    PEXLOCK Lock
    )
{
    ExReleaseSpinLock(&Lock->SpinLock,Lock->Irql);
    return STATUS_SUCCESS;
}

extern BOOLEAN InitIsWinPEMode;
extern ULONG InitWinPEModeType;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INITDATA")
#endif

ULONG PoolHitTag;
extern ULONG InitNlsTableSize;
extern PVOID InitNlsTableBase;

volatile BOOLEAN InitForceInline = FALSE;
NLSTABLEINFO InitTableInfo;
PFN_COUNT BBTPagesToReserve;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif


VOID
ExpInitializeExecutive(
    IN ULONG Number,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

/*++

Routine Description:

    This routine is called from the kernel initialization routine during
    bootstrap to initialize the executive and all of its subcomponents.
    Each subcomponent is potentially called twice to perform Phase 0, and
    then Phase 1 initialization. During Phase 0 initialization, the only
    activity that may be performed is the initialization of subcomponent
    specific data. Phase 0 initialization is performed in the context of
    the kernel start up routine with interrupts disabled. During Phase 1
    initialization, the system is fully operational and subcomponents may
    do any initialization that is necessary.

Arguments:

    Number - Supplies the processor number currently initializing.

    LoaderBlock - Supplies a pointer to a loader parameter block.

Return Value:

    None.

--*/

{
    PFN_COUNT PagesToBurn;
    PCHAR Options;
    PCHAR MemoryOption;
    NTSTATUS Status;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PLIST_ENTRY NextEntry;
    ANSI_STRING AnsiString;
    STRING NameString;
    CHAR Buffer[ 256 ];
    BOOLEAN BufferSizeOk;
    ULONG ImageCount;
    ULONG i;
    ULONG_PTR ResourceIdPath[3];
    PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry;
    PMESSAGE_RESOURCE_DATA  MessageData;
    PLIST_ENTRY NextMd;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;

    if (!ExpIsLoaderValid(LoaderBlock)) {

        KeBugCheckEx(MISMATCHED_HAL,
                     3,
                     LoaderBlock->Extension->Size,
                     LoaderBlock->Extension->MajorVersion,
                     LoaderBlock->Extension->MinorVersion
                     );
    }

    //
    // Initialize PRCB pool lookaside pointers.
    //

#if !defined(_AMD64_)

    ExInitPoolLookasidePointers();

#endif

    if (Number == 0) {
        extern BOOLEAN ExpInTextModeSetup;

        //
        // Determine whether this is textmode setup and whether this is a
        // remote boot client.
        //

        ExpInTextModeSetup = FALSE;
        IoRemoteBootClient = FALSE;

        if (LoaderBlock->SetupLoaderBlock != NULL) {

            if ((LoaderBlock->SetupLoaderBlock->Flags & SETUPBLK_FLAGS_IS_TEXTMODE) != 0) {
                ExpInTextModeSetup = TRUE;
            }

            if ((LoaderBlock->SetupLoaderBlock->Flags & SETUPBLK_FLAGS_IS_REMOTE_BOOT) != 0) {
                IoRemoteBootClient = TRUE;
                ASSERT( _memicmp( LoaderBlock->ArcBootDeviceName, "net(0)", 6 ) == 0 );
            }
        }

#if defined(REMOTE_BOOT)
        SharedUserData->SystemFlags = 0;
        if (IoRemoteBootClient) {
            SharedUserData->SystemFlags |= SYSTEM_FLAG_REMOTE_BOOT_CLIENT;
        }
#endif // defined(REMOTE_BOOT)

        //
        // Indicate that we are in phase 0.
        //

        InitializationPhase = 0L;

        Options = LoaderBlock->LoadOptions;

        if (Options != NULL) {

            //
            // If in BBT mode, remove the requested amount of memory from the
            // loader block and use it for BBT purposes instead.
            //

            _strupr(Options);

            MemoryOption = strstr(Options, "PERFMEM");

            if (MemoryOption != NULL) {
                MemoryOption = strstr (MemoryOption,"=");
                if (MemoryOption != NULL) {
                    PagesToBurn = (PFN_COUNT) atol (MemoryOption + 1);

                    //
                    // Convert MB to pages.
                    //

                    PagesToBurn *= ((1024 * 1024) / PAGE_SIZE);

                    if (PagesToBurn != 0) {

                        PERFINFO_INIT_TRACEFLAGS(Options, MemoryOption);

                        BBTPagesToReserve = ExBurnMemory (LoaderBlock,
                                                          PagesToBurn,
                                                          LoaderBBTMemory,
                                                          &BBTMemoryDescriptor);
                    }
                }
            }

            //
            // Burn memory - consume the amount of memory
            // specified in the OS Load Options.  This is used
            // for testing reduced memory configurations.
            //

            MemoryOption = strstr(Options, "BURNMEMORY");

            if (MemoryOption != NULL) {
                MemoryOption = strstr(MemoryOption,"=");
                if (MemoryOption != NULL ) {

                    PagesToBurn = (PFN_COUNT) atol (MemoryOption + 1);

                    //
                    // Convert MB to pages.
                    //

                    PagesToBurn *= ((1024 * 1024) / PAGE_SIZE);

                    if (PagesToBurn != 0) {
                        ExBurnMemory (LoaderBlock,
                                      PagesToBurn,
                                      LoaderBad,
                                      NULL);
                    }
                }
            }
        }

        //
        // Initialize the translation tables using the loader
        // loaded tables.
        //

        InitNlsTableBase = LoaderBlock->NlsData->AnsiCodePageData;
        InitAnsiCodePageDataOffset = 0;
        InitOemCodePageDataOffset = (ULONG)((PUCHAR)LoaderBlock->NlsData->OemCodePageData - (PUCHAR)LoaderBlock->NlsData->AnsiCodePageData);
        InitUnicodeCaseTableDataOffset = (ULONG)((PUCHAR)LoaderBlock->NlsData->UnicodeCaseTableData - (PUCHAR)LoaderBlock->NlsData->AnsiCodePageData);

        RtlInitNlsTables(
            (PVOID)((PUCHAR)InitNlsTableBase+InitAnsiCodePageDataOffset),
            (PVOID)((PUCHAR)InitNlsTableBase+InitOemCodePageDataOffset),
            (PVOID)((PUCHAR)InitNlsTableBase+InitUnicodeCaseTableDataOffset),
            &InitTableInfo
            );

        RtlResetRtlTranslations(&InitTableInfo);

        //
        // Initialize the Hardware Architecture Layer (HAL).
        //

        if (HalInitSystem(InitializationPhase, LoaderBlock) == FALSE) {
            KeBugCheck(HAL_INITIALIZATION_FAILED);
        }

#if defined(_APIC_TPR_)

        HalpIRQLToTPR = LoaderBlock->Extension->HalpIRQLToTPR;
        HalpVectorToIRQL = LoaderBlock->Extension->HalpVectorToIRQL;

#endif

        //
        // Enable interrupts now that the HAL has initialized.
        //

#if defined(_X86_)

        _enable();

#endif

        //
        // Set the interrupt time forward so the Win32 tick count will wrap
        // within one hour to make rollover errors show up in fewer than 49.7
        // days.
        //

#if DBG

        KeAdjustInterruptTime((LONGLONG)(MAXULONG - (60 * 60 * 1000)) * 10 * 1000);

#endif

        SharedUserData->CryptoExponent = 0;

#if DBG
        NtGlobalFlag |= FLG_ENABLE_CLOSE_EXCEPTIONS |
                        FLG_ENABLE_KDEBUG_SYMBOL_LOAD;
#endif

        Status = RtlFormatBuffer2( Buffer,
                                  sizeof(Buffer),
                                  "C:%s",
                                  LoaderBlock->NtBootPathName,
                                  0
                                 );
        if (! NT_SUCCESS(Status)) {
            KeBugCheck(SESSION3_INITIALIZATION_FAILED);
        }
        RtlInitString( &AnsiString, Buffer );
        Buffer[ --AnsiString.Length ] = '\0';
        NtSystemRoot.Buffer = SharedUserData->NtSystemRoot;
        NtSystemRoot.MaximumLength = sizeof( SharedUserData->NtSystemRoot );
        NtSystemRoot.Length = 0;
        Status = RtlAnsiStringToUnicodeString( &NtSystemRoot,
                                               &AnsiString,
                                               FALSE
                                             );
        if (!NT_SUCCESS( Status )) {
            KeBugCheck(SESSION3_INITIALIZATION_FAILED);
            }

        //
        // Find the address of BugCheck message block resource and put it
        // in KiBugCodeMessages.
        //
        // WARNING: This code assumes that the KLDR_DATA_TABLE_ENTRY for
        // ntoskrnl.exe is always the first in the loaded module list.
        //

        DataTableEntry = CONTAINING_RECORD(LoaderBlock->LoadOrderListHead.Flink,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);

        ResourceIdPath[0] = 11;
        ResourceIdPath[1] = 1;
        ResourceIdPath[2] = 0;

        Status = LdrFindResource_U (DataTableEntry->DllBase,
                                    ResourceIdPath,
                                    3,
                                    (VOID *) &ResourceDataEntry);

        if (NT_SUCCESS(Status)) {

            Status = LdrAccessResource (DataTableEntry->DllBase,
                                        ResourceDataEntry,
                                        &MessageData,
                                        NULL);

            if (NT_SUCCESS(Status)) {
                KiBugCodeMessages = MessageData;
            }
        }

#if !defined(NT_UP)

        //
        // Verify that the kernel and HAL images are suitable for MP systems.
        //
        // N.B. Loading of kernel and HAL symbols now occurs in kdinit.
        //

        ImageCount = 0;
        NextEntry = LoaderBlock->LoadOrderListHead.Flink;
        while ((NextEntry != &LoaderBlock->LoadOrderListHead) && (ImageCount < 2)) {
            DataTableEntry = CONTAINING_RECORD(NextEntry,
                                               KLDR_DATA_TABLE_ENTRY,
                                               InLoadOrderLinks);
            ImageCount += 1;
            if ( !MmVerifyImageIsOkForMpUse(DataTableEntry->DllBase) ) {
                KeBugCheckEx(UP_DRIVER_ON_MP_SYSTEM,
                            (ULONG_PTR)DataTableEntry->DllBase,
                            0,
                            0,
                            0);

            }

            NextEntry = NextEntry->Flink;

        }

#endif // !defined(NT_UP)

        CmpInitSystemVersion(0, LoaderBlock);

        //
        // Initialize the ExResource package.
        //

        if (!ExInitSystem()) {
            KeBugCheck(PHASE0_INITIALIZATION_FAILED);
        }

        //
        // Get multinode configuration (if any).
        //

        KeNumaInitialize();

        //
        // Initialize memory management and the memory allocation pools.
        //

        if (MmInitSystem (0, LoaderBlock) == FALSE) {
            KeBugCheck(PHASE0_INITIALIZATION_FAILED);
        }

        //
        // Scan the loaded module list and load the driver image symbols.
        //

        ImageCount = 0;
        NextEntry = LoaderBlock->LoadOrderListHead.Flink;
        while (NextEntry != &LoaderBlock->LoadOrderListHead) {

            BufferSizeOk = TRUE;

            if (ImageCount >= 2) {
                ULONG Count;
                WCHAR *Filename;
                ULONG Length;

                //
                // Get the address of the data table entry for the next component.
                //

                DataTableEntry = CONTAINING_RECORD(NextEntry,
                                                   KLDR_DATA_TABLE_ENTRY,
                                                   InLoadOrderLinks);

                //
                // Load the symbols via the kernel debugger
                // for the next component.
                //
                if (DataTableEntry->FullDllName.Buffer[0] == L'\\') {
                    //
                    // Correct fullname already available
                    //
                    Filename = DataTableEntry->FullDllName.Buffer;
                    Length = DataTableEntry->FullDllName.Length / sizeof(WCHAR);
                    if (sizeof(Buffer) < Length + sizeof(ANSI_NULL)) {
                        //
                        // DllName too long.
                        //
                        BufferSizeOk = FALSE;
                    } else {
                        Count = 0;
                        do {
                            Buffer[Count++] = (CHAR)*Filename++;
                        } while (Count < Length);

                        Buffer[Count] = 0;
                    }
                } else {
                    //
                    // Assume drivers
                    //
                    if (sizeof(Buffer) < 18 + NtSystemRoot.Length / sizeof(WCHAR) - 2
                                            + DataTableEntry->BaseDllName.Length / sizeof(WCHAR)
                                            + sizeof(ANSI_NULL)) {
                        //
                        // ignore the driver entry, it must have been corrupt.
                        //
                        BufferSizeOk = FALSE;

                    } else {
                        Status = RtlFormatBuffer2 (Buffer,
                                                     sizeof(Buffer),
                                                     "%ws\\System32\\Drivers\\%wZ",
                                                     &SharedUserData->NtSystemRoot[2],
                                                     &DataTableEntry->BaseDllName);
                        if (! NT_SUCCESS(Status)) {
                            KeBugCheck(PHASE0_INITIALIZATION_FAILED);
                        }
                    }
                }
                if (BufferSizeOk) {
                    RtlInitString (&NameString, Buffer );
                    DbgLoadImageSymbols (&NameString,
                                         DataTableEntry->DllBase,
                                         (ULONG)-1);

#if !defined(NT_UP)
                    if (!MmVerifyImageIsOkForMpUse(DataTableEntry->DllBase)) {
                        KeBugCheckEx(UP_DRIVER_ON_MP_SYSTEM,(ULONG_PTR)DataTableEntry->DllBase,0,0,0);
                    }
#endif // NT_UP
                }

            }
            ImageCount += 1;
            NextEntry = NextEntry->Flink;
        }

        //
        // If break after symbol load is specified, then break into the
        // debugger.
        //

        if (KdBreakAfterSymbolLoad != FALSE) {
            DbgBreakPointWithStatus(DBG_STATUS_CONTROL_C);
        }


        //
        // Turn on the headless terminal now, if we are of a sufficiently
        // new vintage of loader
        //
        if (LoaderBlock->Extension->Size >= sizeof (LOADER_PARAMETER_EXTENSION)) {
            HeadlessInit(LoaderBlock);
        }


        //
        // These fields are supported for legacy 3rd party 32-bit software
        // only.  New code should call NtQueryInformationSystem() to get them.
        //

#if defined(_WIN64)

        SharedUserData->Reserved1 = 0x7ffeffff; // 2gb HighestUserAddress
        SharedUserData->Reserved3 = 0x80000000; // 2gb SystemRangeStart

#else

        //
        // Set the highest user address and the start of the system range in
        // the shared memory block.
        //
        // N.B. This is not a constant value if the target system is an x86
        //      with 3gb of user virtual address space.
        //

        SharedUserData->Reserved1 = (ULONG)MM_HIGHEST_USER_ADDRESS;
        SharedUserData->Reserved3 = (ULONG)MmSystemRangeStart;

#endif

        //
        // Snapshot the NLS tables into paged pool and then
        // reset the translation tables.
        //
        // Walk through the memory descriptors and size the NLS data.
        //

        NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

        while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

            MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                                 MEMORY_ALLOCATION_DESCRIPTOR,
                                                 ListEntry);

            if (MemoryDescriptor->MemoryType == LoaderNlsData) {
                InitNlsTableSize += MemoryDescriptor->PageCount*PAGE_SIZE;
            }

            NextMd = MemoryDescriptor->ListEntry.Flink;
        }

        InitNlsTableBase = ExAllocatePoolWithTag (NonPagedPool,
                                                  InitNlsTableSize,
                                                  ' slN');

        if (InitNlsTableBase == NULL) {
            KeBugCheck(PHASE0_INITIALIZATION_FAILED);
        }

        //
        // Copy the NLS data into the dynamic buffer so that we can
        // free the buffers allocated by the loader. The loader guarantees
        // contiguous buffers and the base of all the tables is the ANSI
        // code page data.
        //

        RtlCopyMemory (InitNlsTableBase,
                       LoaderBlock->NlsData->AnsiCodePageData,
                       InitNlsTableSize);

        RtlInitNlsTables ((PVOID)((PUCHAR)InitNlsTableBase+InitAnsiCodePageDataOffset),
            (PVOID)((PUCHAR)InitNlsTableBase+InitOemCodePageDataOffset),
            (PVOID)((PUCHAR)InitNlsTableBase+InitUnicodeCaseTableDataOffset),
            &InitTableInfo);

        RtlResetRtlTranslations (&InitTableInfo);

        CmpInitSystemVersion(1, NULL);

        if (NtGlobalFlag & FLG_KERNEL_STACK_TRACE_DB) {

            PVOID StackTraceDataBase;
            ULONG StackTraceDataBaseLength;
            NTSTATUS Status;

            StackTraceDataBaseLength =  512 * 1024;
            switch ( MmQuerySystemSize() ) {
                case MmMediumSystem :
                    StackTraceDataBaseLength = 1024 * 1024;
                    break;

                case MmLargeSystem :
                    StackTraceDataBaseLength = 2048 * 1024;
                    break;
            }

            StackTraceDataBase = ExAllocatePoolWithTag( NonPagedPool,
                                         StackTraceDataBaseLength,
                                         'catS');

            if (StackTraceDataBase != NULL) {

                KdPrint(( "INIT: Kernel mode stack back trace enabled "
                          "with %u KB buffer.\n", StackTraceDataBaseLength / 1024 ));

                Status = RtlInitializeStackTraceDataBase (StackTraceDataBase,
                                                          StackTraceDataBaseLength,
                                                          StackTraceDataBaseLength);
            } else {
                Status = STATUS_NO_MEMORY;
            }

            if (!NT_SUCCESS( Status )) {
                KdPrint(( "INIT: Unable to initialize stack trace data base - Status == %lx\n", Status ));
            }
        }

        if (NtGlobalFlag & FLG_ENABLE_EXCEPTION_LOGGING) {
            RtlInitializeExceptionLog(MAX_EXCEPTION_LOG);
        }

        ExInitializeHandleTablePackage();

#if DBG
        //
        // Allocate and zero the system service count table.
        //

        KeServiceDescriptorTable[0].Count =
                    (PULONG)ExAllocatePoolWithTag(NonPagedPool,
                                           KiServiceLimit * sizeof(ULONG),
                                           'llac');
        KeServiceDescriptorTableShadow[0].Count = KeServiceDescriptorTable[0].Count;
        if (KeServiceDescriptorTable[0].Count != NULL ) {
            RtlZeroMemory((PVOID)KeServiceDescriptorTable[0].Count,
                          KiServiceLimit * sizeof(ULONG));
        }
#endif

        if (!ObInitSystem()) {
            KeBugCheck(OBJECT_INITIALIZATION_FAILED);
        }

        if (!SeInitSystem()) {
            KeBugCheck(SECURITY_INITIALIZATION_FAILED);
        }

        if (PsInitSystem(0, LoaderBlock) == FALSE) {
            KeBugCheck(PROCESS_INITIALIZATION_FAILED);
        }

        if (!PpInitSystem()) {
            KeBugCheck(PP0_INITIALIZATION_FAILED);
        }

        //
        // Initialize debug system.
        //

        DbgkInitialize ();

        //
        // Compute the tick count multiplier that is used for computing the
        // windows millisecond tick count and copy the resultant value to
        // the memory that is shared between user and kernel mode.
        //

        ExpTickCountMultiplier = ExComputeTickCountMultiplier(KeMaximumIncrement);
        SharedUserData->TickCountMultiplier = ExpTickCountMultiplier;

        //
        // Set the base os version into shared memory
        //

        SharedUserData->NtMajorVersion = NtMajorVersion;
        SharedUserData->NtMinorVersion = NtMinorVersion;

        //
        // Set the supported image number range used to determine by the
        // loader if a particular image can be executed on the host system.
        // Eventually this will need to be dynamically computed. Also set
        // the architecture specific feature bits.
        //

#if defined(_AMD64_)

        SharedUserData->ImageNumberLow = IMAGE_FILE_MACHINE_AMD64;
        SharedUserData->ImageNumberHigh = IMAGE_FILE_MACHINE_AMD64;

#elif defined(_X86_)

        SharedUserData->ImageNumberLow = IMAGE_FILE_MACHINE_I386;
        SharedUserData->ImageNumberHigh = IMAGE_FILE_MACHINE_I386;

#else

#error "no target architecture"

#endif

    }
    else {

        //
        // Initialize the Hardware Architecture Layer (HAL).
        //

        if (HalInitSystem(InitializationPhase, LoaderBlock) == FALSE) {
            KeBugCheck(HAL_INITIALIZATION_FAILED);
        }
    }

    return;
}

/*++

Routine Description:

    This routine is a paged stub that calls a discardable routine to do
    the real work.  This is because Mm reuses the discardable routine's
    (INIT) memory after phase 1 completes, but must keep this routine
    around so that the stack walker can obtain correct prologues/epilogues.

Arguments:

    Context - Supplies the loader block.

Return Value:

    None.

--*/

VOID
Phase1Initialization (
    IN PVOID Context
    )

{
    Phase1InitializationDiscard (Context);
    MmZeroPageThread();
    return;
}

VOID
Phase1InitializationDiscard (
    IN PVOID Context
    )

{
    PLOADER_PARAMETER_BLOCK LoaderBlock;
    PETHREAD Thread;
    PKPRCB Prcb;
    KPRIORITY Priority;
    NTSTATUS Status;
    UNICODE_STRING SessionManager;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PVOID Address;
    SIZE_T Size;
    LARGE_INTEGER UniversalTime;
    LARGE_INTEGER CmosTime;
    LARGE_INTEGER OldTime;
    TIME_FIELDS TimeFields;
    UNICODE_STRING EnvString, NullString, UnicodeSystemDriveString;
    PWSTR Src, Dst;
    BOOLEAN ResetActiveTimeBias;
    HANDLE NlsSection;
    LARGE_INTEGER SectionSize;
    LARGE_INTEGER SectionOffset;
    PVOID SectionBase;
    PVOID ViewBase;
    ULONG CacheViewSize;
    SIZE_T CapturedViewSize;
    ULONG SavedViewSize;
    LONG BootTimeZoneBias;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
#ifndef NT_UP
    PMESSAGE_RESOURCE_ENTRY MessageEntry1;
#endif
    PCHAR MPKernelString;
    PCHAR Options;
    PCHAR YearOverrideOption;
    LONG  CurrentYear = 0;
    BOOLEAN NOGUIBOOT;
    BOOLEAN SOS;
    PVOID Environment;

    PRTL_USER_PROCESS_INFORMATION ProcessInformation;
    
    ProcessInformation = ExAllocatePoolWithTag(NonPagedPool,
                                        sizeof(*ProcessInformation),
                                        'tinI');

    if (ProcessInformation == NULL) {
        KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,
                     STATUS_NO_MEMORY,
                     8,
                     0,
                     0);
    }

    //
    // The following is a dummy reference section to inline functions that
    // need to have a reference forced. This code is never executed, but the
    // compiler can never assume it isn't.
    //
    // N.B. The control variable is always false.
    //

    if (InitForceInline == TRUE) {
        KGUARDED_MUTEX Mutex;
        extern ULONG volatile *VpPoolHitTag;

        KeTryToAcquireGuardedMutex(&Mutex);
        KeEnterGuardedRegion();
        KeLeaveGuardedRegion();
        KeAreApcsDisabled();
        KeRaiseIrqlToDpcLevel();
        VpPoolHitTag = &PoolHitTag;
    }

    //
    // Set the phase number and raise the priority of current thread to
    // a high priority so it will not be preempted during initialization.
    //

    ResetActiveTimeBias = FALSE;
    InitializationPhase = 1;
    Thread = PsGetCurrentThread();
    Priority = KeSetPriorityThread( &Thread->Tcb,MAXIMUM_PRIORITY - 1 );

    LoaderBlock = (PLOADER_PARAMETER_BLOCK)Context;

    //
    // Put Phase 1 initialization calls here.
    //

    if (HalInitSystem(InitializationPhase, LoaderBlock) == FALSE) {
        KeBugCheck(HAL1_INITIALIZATION_FAILED);
    }

    //
    // Allow the boot video driver to behave differently based on the
    // OsLoadOptions.
    //

    Options = LoaderBlock->LoadOptions ? _strupr(LoaderBlock->LoadOptions) : NULL;

    if (Options) {
        NOGUIBOOT = (BOOLEAN)(strstr(Options, "NOGUIBOOT") != NULL);
    } else {
        NOGUIBOOT = FALSE;
    }

    InbvEnableBootDriver((BOOLEAN)!NOGUIBOOT);

    //
    // There is now enough functionality for the system Boot Video
    // Driver to run.
    //

    InbvDriverInitialize(LoaderBlock, 18);

    if (NOGUIBOOT) {

        //
        // If the user specified the noguiboot switch we don't want to
        // use the bootvid driver, so release display ownership.
        //

        InbvNotifyDisplayOwnershipLost(NULL);
    }

    if (Options) {
        SOS = (BOOLEAN)(strstr(Options, "SOS") != NULL);
    } else {
        SOS = FALSE;
    }

    if (NOGUIBOOT) {
        InbvEnableDisplayString(FALSE);
    } else {
        InbvEnableDisplayString(SOS);
        DisplayBootBitmap(SOS);
    }

    //
    // Check whether we are booting into WinPE
    //
    if (Options) {
        if (strstr(Options, "MININT") != NULL) {
            InitIsWinPEMode = TRUE;

            if (strstr(Options, "INRAM") != NULL) {
                InitWinPEModeType |= INIT_WINPEMODE_INRAM;
            } else {
                InitWinPEModeType |= INIT_WINPEMODE_REGULAR;
            }
        }
    }

    CmpInitSystemVersion(2, NULL);

    //
    // Initialize the Power subsystem.
    //

    if (!PoInitSystem(0)) {
        KeBugCheck(INTERNAL_POWER_ERROR);
    }

    //
    // The user may have put a /YEAR=2000 switch on
    // the OSLOADOPTIONS line.  This allows us to
    // enforce a particular year on hardware that
    // has a broken clock.
    //

    if (Options) {
        YearOverrideOption = strstr(Options, "YEAR");
        if (YearOverrideOption != NULL) {
            YearOverrideOption = strstr(YearOverrideOption,"=");
        }
        if (YearOverrideOption != NULL) {
            CurrentYear = atol(YearOverrideOption + 1);
        }
    }

    //
    // Initialize the system time and set the time the system was booted.
    //
    // N.B. This cannot be done until after the phase one initialization
    //      of the HAL Layer.
    //

    if (ExCmosClockIsSane
        && HalQueryRealTimeClock(&TimeFields)) {

        //
        // If appropriate, override the year.
        //
        if (YearOverrideOption) {
            TimeFields.Year = (SHORT)CurrentYear;
        }

        RtlTimeFieldsToTime(&TimeFields, &CmosTime);
        UniversalTime = CmosTime;
        if ( !ExpRealTimeIsUniversal ) {

            //
            // If the system stores time in local time. This is converted to
            // universal time before going any further
            //
            // If we have previously set the time through NT, then
            // ExpLastTimeZoneBias should contain the timezone bias in effect
            // when the clock was set.  Otherwise, we will have to resort to
            // our next best guess which would be the programmed bias stored in
            // the registry
            //

            if ( ExpLastTimeZoneBias == -1 ) {
                ResetActiveTimeBias = TRUE;
                ExpLastTimeZoneBias = ExpAltTimeZoneBias;
                }

            ExpTimeZoneBias.QuadPart = Int32x32To64(
                                ExpLastTimeZoneBias*60,   // Bias in seconds
                                10000000
                                );
            SharedUserData->TimeZoneBias.High2Time = ExpTimeZoneBias.HighPart;
            SharedUserData->TimeZoneBias.LowPart = ExpTimeZoneBias.LowPart;
            SharedUserData->TimeZoneBias.High1Time = ExpTimeZoneBias.HighPart;
            UniversalTime.QuadPart = CmosTime.QuadPart + ExpTimeZoneBias.QuadPart;
        }
        KeSetSystemTime(&UniversalTime, &OldTime, FALSE, NULL);

        //
        // Notify other components that the system time has been set
        //

        PoNotifySystemTimeSet();

        KeBootTime = UniversalTime;
        KeBootTimeBias = 0;
    }

    MPKernelString = "";

    CmpInitSystemVersion(8, Options);

#ifndef NT_UP

    CmpInitSystemVersion(3, NULL);

    //
    // If this is an MP build of the kernel start any other processors now
    //

    KeStartAllProcessors();

    //
    // Since starting processors has thrown off the system time, get it again
    // from the RTC and set the system time again.
    //

    if (ExCmosClockIsSane
        && HalQueryRealTimeClock(&TimeFields)) {

        if (YearOverrideOption) {
            TimeFields.Year = (SHORT)CurrentYear;
        }

        RtlTimeFieldsToTime(&TimeFields, &CmosTime);

        if ( !ExpRealTimeIsUniversal ) {
            UniversalTime.QuadPart = CmosTime.QuadPart + ExpTimeZoneBias.QuadPart;
        }

        KeSetSystemTime(&UniversalTime, &OldTime, TRUE, NULL);
    }

    //
    // Set the affinity of the system process and all of its threads to
    // all processors in the host configuration.
    //

    KeSetAffinityProcess(KeGetCurrentThread()->ApcState.Process,
                         KeActiveProcessors);

    DataTableEntry = CONTAINING_RECORD(LoaderBlock->LoadOrderListHead.Flink,
                                   KLDR_DATA_TABLE_ENTRY,
                                   InLoadOrderLinks);

    Status = RtlFindMessage (DataTableEntry->DllBase, 11, 0,
                        WINDOWS_NT_MP_STRING, &MessageEntry1);

    if (NT_SUCCESS( Status )) {
        MPKernelString = MessageEntry1->Text;
    }
    else {
        MPKernelString = "MultiProcessor Kernel\r\n";
    }

    CmpInitSystemVersion(9, MPKernelString);
#endif

    //
    // Signify to the HAL that all processors have been started and any
    // post initialization should be performed.
    //

    if (!HalAllProcessorsStarted()) {
        KeBugCheck(HAL1_INITIALIZATION_FAILED);
    }

    CmpInitSystemVersion(4, DataTableEntry);

    //
    // Initialize OB, EX, KE, and KD.
    //

    if (!ObInitSystem()) {
        KeBugCheck(OBJECT1_INITIALIZATION_FAILED);
    }

    if (!ExInitSystem()) {
        KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,STATUS_UNSUCCESSFUL,0,1,0);
    }

    if (!KeInitSystem()) {
        KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,STATUS_UNSUCCESSFUL,0,2,0);
    }

    if (!KdInitSystem(InitializationPhase, NULL)) {
        KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,STATUS_UNSUCCESSFUL,0,3,0);
    }

    //
    // SE expects directory and executive objects to be available, but
    // must be before device drivers are initialized.
    //

    if (!SeInitSystem()) {
        KeBugCheck(SECURITY1_INITIALIZATION_FAILED);
    }

    InbvUpdateProgressBar(10);

    //
    // Create the symbolic link to \SystemRoot.
    //

    Status = CreateSystemRootLink(LoaderBlock);
    if ( !NT_SUCCESS(Status) ) {
        KeBugCheckEx(SYMBOLIC_INITIALIZATION_FAILED,Status,0,0,0);
    }

    if (MmInitSystem(1, LoaderBlock) == FALSE) {
        KeBugCheck(MEMORY1_INITIALIZATION_FAILED);
    }

    //
    // Snapshot the NLS tables into a page file backed section, and then
    // reset the translation tables.
    //

    SectionSize.HighPart = 0;
    SectionSize.LowPart = InitNlsTableSize;

    Status = ZwCreateSection(
                &NlsSection,
                SECTION_ALL_ACCESS,
                NULL,
                &SectionSize,
                PAGE_READWRITE,
                SEC_COMMIT,
                NULL
                );

    if (!NT_SUCCESS(Status)) {
        KdPrint(("INIT: Nls Section Creation Failed %x\n",Status));
        KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,Status,1,0,0);
    }

    Status = ObReferenceObjectByHandle(
                NlsSection,
                SECTION_ALL_ACCESS,
                MmSectionObjectType,
                KernelMode,
                &InitNlsSectionPointer,
                NULL
                );

    ZwClose(NlsSection);

    if ( !NT_SUCCESS(Status) ) {
        KdPrint(("INIT: Nls Section Reference Failed %x\n",Status));
        KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,Status,2,0,0);
    }

    SectionBase = NULL;
    CacheViewSize = SectionSize.LowPart;
    SavedViewSize = CacheViewSize;
    SectionSize.LowPart = 0;

    Status = MmMapViewInSystemCache (InitNlsSectionPointer,
                                     &SectionBase,
                                     &SectionSize,
                                     &CacheViewSize);

    if (!NT_SUCCESS(Status)) {
        KdPrint(("INIT: Map In System Cache Failed %x\n",Status));
        KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,Status,3,0,0);
    }

    //
    // Copy the NLS data into the dynamic buffer so that we can
    // free the buffers allocated by the loader. The loader guarantees
    // contiguous buffers and the base of all the tables is the ANSI
    // code page data.
    //

    RtlCopyMemory (SectionBase, InitNlsTableBase, InitNlsTableSize);

    //
    // Unmap the view to remove all pages from memory.  This prevents
    // these tables from consuming memory in the system cache while
    // the system cache is underutilized during bootup.
    //

    MmUnmapViewInSystemCache (SectionBase, InitNlsSectionPointer, FALSE);

    SectionBase = NULL;

    //
    // Map it back into the system cache, but now the pages will no
    // longer be valid.
    //

    Status = MmMapViewInSystemCache(
                InitNlsSectionPointer,
                &SectionBase,
                &SectionSize,
                &SavedViewSize
                );

    if ( !NT_SUCCESS(Status) ) {
        KdPrint(("INIT: Map In System Cache Failed %x\n",Status));
        KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,Status,4,0,0);
    }

    ExFreePool(InitNlsTableBase);

    InitNlsTableBase = SectionBase;

    RtlInitNlsTables(
        (PVOID)((PUCHAR)InitNlsTableBase+InitAnsiCodePageDataOffset),
        (PVOID)((PUCHAR)InitNlsTableBase+InitOemCodePageDataOffset),
        (PVOID)((PUCHAR)InitNlsTableBase+InitUnicodeCaseTableDataOffset),
        &InitTableInfo
        );

    RtlResetRtlTranslations(&InitTableInfo);

    ViewBase = NULL;
    SectionOffset.LowPart = 0;
    SectionOffset.HighPart = 0;
    CapturedViewSize = 0;

    //
    // Map the system dll into the user part of the address space
    //

    Status = MmMapViewOfSection (InitNlsSectionPointer,
                                 PsGetCurrentProcess(),
                                 &ViewBase,
                                 0L,
                                 0L,
                                 &SectionOffset,
                                 &CapturedViewSize,
                                 ViewShare,
                                 0L,
                                 PAGE_READWRITE);

    if (!NT_SUCCESS(Status)) {
        KdPrint(("INIT: Map In User Portion Failed %x\n",Status));
        KeBugCheckEx(PHASE1_INITIALIZATION_FAILED,Status,5,0,0);
    }

    RtlCopyMemory (ViewBase, InitNlsTableBase, InitNlsTableSize);

    InitNlsTableBase = ViewBase;

    //
    // Initialize the cache manager.
    //

    if (!CcInitializeCacheManager()) {
        KeBugCheck(CACHE_INITIALIZATION_FAILED);
    }

    //
    // Config management (particularly the registry) gets initialized in
    // two parts.  Part 1 makes \REGISTRY\MACHINE\SYSTEM and
    // \REGISTRY\MACHINE\HARDWARE available.  These are needed to
    // complete IO init.
    //

    if (!CmInitSystem1(LoaderBlock)) {
        KeBugCheck(CONFIG_INITIALIZATION_FAILED);
    }

    //
    // Initialize the prefetcher after registry is initialized so we can
    // query the prefetching parameters.
    //

    CcPfInitializePrefetcher();

    InbvUpdateProgressBar(15);

    //
    // Compute timezone bias and next cutover date.
    //

    BootTimeZoneBias = ExpLastTimeZoneBias;
    ExpRefreshTimeZoneInformation(&CmosTime);

    if (ResetActiveTimeBias) {
        ExLocalTimeToSystemTime(&CmosTime,&UniversalTime);
        KeBootTime = UniversalTime;
        KeBootTimeBias = 0;
        KeSetSystemTime(&UniversalTime, &OldTime, FALSE, NULL);
    }
    else {

        //
        // Check to see if a timezone switch occurred prior to boot...
        //

        if (BootTimeZoneBias != ExpLastTimeZoneBias) {
            ZwSetSystemTime(NULL,NULL);
        }
    }


    if (!FsRtlInitSystem()) {
        KeBugCheck(FILE_INITIALIZATION_FAILED);
    }

    //
    // Initialize the range list package - this must be before PNP
    // initialization as PNP uses range lists.
    //

    RtlInitializeRangeListPackage();

    HalReportResourceUsage();

    KdDebuggerInitialize1(LoaderBlock);

    //
    // Perform phase1 initialization of the Plug and Play manager.  This
    // must be done before the I/O system initializes.
    //

    if (!PpInitSystem()) {
        KeBugCheck(PP1_INITIALIZATION_FAILED);
    }

    InbvUpdateProgressBar(20);

    //
    // LPC needs to be initialized before the I/O system, since
    // some drivers may create system threads that will terminate
    // and cause LPC to be called.
    //

    if (!LpcInitSystem()) {
        KeBugCheck(LPC_INITIALIZATION_FAILED);
    }

    //
    // Now that system time is running, initialize more of the Executive.
    //

    ExInitSystemPhase2();

    //
    // Allow time slip notification changes.
    //

    KdpTimeSlipPending = 0;

    //
    // Initialize the Io system.
    //
    // IoInitSystem updates progress bar updates from 25 to 75 %.
    //

    InbvSetProgressBarSubset(25, 75);

    if (!IoInitSystem(LoaderBlock)) {
        KeBugCheck(IO1_INITIALIZATION_FAILED);
    }

    //
    // Clear progress bar subset, goes back to absolute mode.
    //

    InbvSetProgressBarSubset(0, 100);

    CmpInitSystemVersion(6, NULL);

    //
    // Begin paging the executive if desired.
    //

    MmInitSystem(2, LoaderBlock);

    InbvUpdateProgressBar(80);


#if defined(_X86_)

    //
    // Initialize Vdm specific stuff
    //
    // Note:  If this fails, Vdms may not be able to run, but it isn't
    //        necessary to bugcheck the system because of this.
    //

    KeI386VdmInitialize();

#if !defined(NT_UP)

    //
    // Now that the error log interface has been initialized, write
    // an informational message if it was determined that the
    // processors in the system are at differing revision levels.
    //

    if (CmProcessorMismatch != 0) {

        PIO_ERROR_LOG_PACKET ErrLog;

        ErrLog = IoAllocateGenericErrorLogEntry(ERROR_LOG_MAXIMUM_SIZE);

        if (ErrLog) {

            //
            // Fill it in and write it out.
            //

            ErrLog->FinalStatus = STATUS_MP_PROCESSOR_MISMATCH;
            ErrLog->ErrorCode = STATUS_MP_PROCESSOR_MISMATCH;
            ErrLog->UniqueErrorValue = CmProcessorMismatch;

            IoWriteErrorLogEntry(ErrLog);
        }
    }

#endif // !NT_UP

#endif // _X86_

    if (!PoInitSystem(1)) {
        KeBugCheck(INTERNAL_POWER_ERROR);
    }

    //
    // Okay to call PsInitSystem now that \SystemRoot is defined so it can
    // locate NTDLL.DLL and SMSS.EXE.
    //

    if (PsInitSystem(1, LoaderBlock) == FALSE) {
        KeBugCheck(PROCESS1_INITIALIZATION_FAILED);
    }

    InbvUpdateProgressBar(85);

    //
    // Force KeBugCheck to look at PsLoadedModuleList now that it is setup.
    //

    if (LoaderBlock == KeLoaderBlock) {
        KeLoaderBlock = NULL;
    }

    //
    // Free loader block.
    //

    MmFreeLoaderBlock (LoaderBlock);
    LoaderBlock = NULL;
    Context = NULL;

    //
    // Perform Phase 1 Reference Monitor Initialization.  This includes
    // creating the Reference Monitor Command Server Thread, a permanent
    // thread of the System Init process.  That thread will create an LPC
    // port called the Reference Monitor Command Port through which
    // commands sent by the Local Security Authority Subsystem will be
    // received.  These commands (e.g. Enable Auditing) change the Reference
    // Monitor State.
    //

    if (!SeRmInitPhase1()) {
        KeBugCheck(REFMON_INITIALIZATION_FAILED);
    }

    InbvUpdateProgressBar(90);

    //
    // Set up process parameters for the Session Manager Subsystem.
    //
    // NOTE: Remote boot allocates an extra DOS_MAX_PATH_LENGTH number of
    // WCHARs in order to hold command line arguments to smss.exe.
    //

    Size = sizeof( *ProcessParameters ) +
           ((DOS_MAX_PATH_LENGTH * 6) * sizeof( WCHAR ));
    ProcessParameters = NULL;
    Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                      (PVOID *)&ProcessParameters,
                                      0,
                                      &Size,
                                      MEM_COMMIT,
                                      PAGE_READWRITE
                                    );
    if (!NT_SUCCESS( Status )) {
        KeBugCheckEx(SESSION1_INITIALIZATION_FAILED,Status,0,0,0);
    }

    ProcessParameters->Length = (ULONG)Size;
    ProcessParameters->MaximumLength = (ULONG)Size;

    //
    // Reserve the low 1 MB of address space in the session manager.
    // Setup gets started using a replacement for the session manager
    // and that process needs to be able to use the vga driver on x86,
    // which uses int10 and thus requires the low 1 meg to be reserved
    // in the process. The cost is so low that we just do this all the
    // time, even when setup isn't running.
    //

    ProcessParameters->Flags = RTL_USER_PROC_PARAMS_NORMALIZED | RTL_USER_PROC_RESERVE_1MB;

    Size = PAGE_SIZE;
    Environment = NULL;
    Status = ZwAllocateVirtualMemory( NtCurrentProcess(),
                                      &Environment,
                                      0,
                                      &Size,
                                      MEM_COMMIT,
                                      PAGE_READWRITE
                                    );
    if (!NT_SUCCESS( Status )) {
        KeBugCheckEx(SESSION2_INITIALIZATION_FAILED,Status,0,0,0);
    }

    ProcessParameters->Environment = Environment;

    Dst = (PWSTR)(ProcessParameters + 1);
    ProcessParameters->CurrentDirectory.DosPath.Buffer = Dst;
    ProcessParameters->CurrentDirectory.DosPath.MaximumLength = DOS_MAX_PATH_LENGTH * sizeof( WCHAR );
    RtlCopyUnicodeString( &ProcessParameters->CurrentDirectory.DosPath,
                          &NtSystemRoot
                        );

    Dst = (PWSTR)((PCHAR)ProcessParameters->CurrentDirectory.DosPath.Buffer +
                  ProcessParameters->CurrentDirectory.DosPath.MaximumLength
                 );
    ProcessParameters->DllPath.Buffer = Dst;
    ProcessParameters->DllPath.MaximumLength = DOS_MAX_PATH_LENGTH * sizeof( WCHAR );
    RtlCopyUnicodeString( &ProcessParameters->DllPath,
                          &ProcessParameters->CurrentDirectory.DosPath
                        );
    RtlAppendUnicodeToString( &ProcessParameters->DllPath, L"\\System32" );

    Dst = (PWSTR)((PCHAR)ProcessParameters->DllPath.Buffer +
                  ProcessParameters->DllPath.MaximumLength
                 );
    ProcessParameters->ImagePathName.Buffer = Dst;
    ProcessParameters->ImagePathName.MaximumLength = DOS_MAX_PATH_LENGTH * sizeof( WCHAR );

    if (NtInitialUserProcessBufferType != REG_SZ ||
        (NtInitialUserProcessBufferLength != (ULONG)-1 &&
         (NtInitialUserProcessBufferLength < sizeof(WCHAR) ||
          NtInitialUserProcessBufferLength >
          sizeof(NtInitialUserProcessBuffer) - sizeof(WCHAR)))) {

        KeBugCheckEx(SESSION2_INITIALIZATION_FAILED,
                     STATUS_INVALID_PARAMETER,
                     NtInitialUserProcessBufferType,
                     NtInitialUserProcessBufferLength,
                     sizeof(NtInitialUserProcessBuffer));
    }

    // Executable names with spaces don't need to
    // be supported so just find the first space and
    // assume it terminates the process image name.
    Src = NtInitialUserProcessBuffer;
    while (*Src && *Src != L' ') {
        Src++;
    }

    ProcessParameters->ImagePathName.Length =
        (USHORT)((PUCHAR)Src - (PUCHAR)NtInitialUserProcessBuffer);
    RtlCopyMemory(ProcessParameters->ImagePathName.Buffer,
                  NtInitialUserProcessBuffer,
                  ProcessParameters->ImagePathName.Length);
    ProcessParameters->ImagePathName.Buffer[ProcessParameters->ImagePathName.Length / sizeof(WCHAR)] = UNICODE_NULL;

    Dst = (PWSTR)((PCHAR)ProcessParameters->ImagePathName.Buffer +
                  ProcessParameters->ImagePathName.MaximumLength
                 );
    ProcessParameters->CommandLine.Buffer = Dst;
    ProcessParameters->CommandLine.MaximumLength = DOS_MAX_PATH_LENGTH * sizeof( WCHAR );
    RtlAppendUnicodeToString(&ProcessParameters->CommandLine,
                             NtInitialUserProcessBuffer);

    CmpInitSystemVersion(7, NULL);

    NullString.Buffer = L"";
    NullString.Length = sizeof(WCHAR);
    NullString.MaximumLength = sizeof(WCHAR);

    EnvString.Buffer = ProcessParameters->Environment;
    EnvString.Length = 0;
    EnvString.MaximumLength = (USHORT)Size;

    RtlAppendUnicodeToString( &EnvString, L"Path=" );
    RtlAppendUnicodeStringToString( &EnvString, &ProcessParameters->DllPath );
    RtlAppendUnicodeStringToString( &EnvString, &NullString );

    UnicodeSystemDriveString = NtSystemRoot;
    UnicodeSystemDriveString.Length = 2 * sizeof( WCHAR );
    RtlAppendUnicodeToString( &EnvString, L"SystemDrive=" );
    RtlAppendUnicodeStringToString( &EnvString, &UnicodeSystemDriveString );
    RtlAppendUnicodeStringToString( &EnvString, &NullString );

    RtlAppendUnicodeToString( &EnvString, L"SystemRoot=" );
    RtlAppendUnicodeStringToString( &EnvString, &NtSystemRoot );
    RtlAppendUnicodeStringToString( &EnvString, &NullString );


    SessionManager = ProcessParameters->ImagePathName;
    Status = RtlCreateUserProcess(
                &SessionManager,
                OBJ_CASE_INSENSITIVE,
                RtlDeNormalizeProcessParams( ProcessParameters ),
                NULL,
                NULL,
                NULL,
                FALSE,
                NULL,
                NULL,
                ProcessInformation);

    if (InbvBootDriverInstalled)
    {
        FinalizeBootLogo();
    }

    if (!NT_SUCCESS(Status)) {
        KeBugCheckEx(SESSION3_INITIALIZATION_FAILED,Status,0,0,0);
    }

    Status = ZwResumeThread(ProcessInformation->Thread, NULL);

    if ( !NT_SUCCESS(Status) ) {
        KeBugCheckEx(SESSION4_INITIALIZATION_FAILED,Status,0,0,0);
    }

    InbvUpdateProgressBar(100);

    //
    // Turn on debug output so that we can see chkdsk run.
    //

    InbvEnableDisplayString(TRUE);

    //
    // Wait five seconds for the session manager to get started or
    // terminate. If the wait times out, then the session manager
    // is assumed to be healthy and the zero page thread is called.
    //

    OldTime.QuadPart = Int32x32To64(5, -(10 * 1000 * 1000));
    Status = ZwWaitForSingleObject(
                ProcessInformation->Process,
                FALSE,
                &OldTime
                );

    if (Status == STATUS_SUCCESS) {
        KeBugCheck(SESSION5_INITIALIZATION_FAILED);
    }

    //
    // Don't need these handles anymore.
    //

    ZwClose( ProcessInformation->Thread );
    ZwClose( ProcessInformation->Process );

    //
    // Free up memory used to pass arguments to session manager.
    //

    Size = 0;
    Address = Environment;
    ZwFreeVirtualMemory( NtCurrentProcess(),
                         (PVOID *)&Address,
                         &Size,
                         MEM_RELEASE
                       );

    Size = 0;
    Address = ProcessParameters;
    ZwFreeVirtualMemory( NtCurrentProcess(),
                         (PVOID *)&Address,
                         &Size,
                         MEM_RELEASE
                       );

    InitializationPhase += 1;

#if defined(_X86_)

    KiInitializeInterruptTimers();

#endif

    ExFreePool(ProcessInformation);
}


NTSTATUS
CreateSystemRootLink(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

{
    HANDLE handle;
    UNICODE_STRING nameString;
    OBJECT_ATTRIBUTES objectAttributes;
    STRING linkString;
    UNICODE_STRING linkUnicodeString;
    NTSTATUS status;
    UCHAR deviceNameBuffer[256];
    STRING deviceNameString;
    UNICODE_STRING deviceNameUnicodeString;
    HANDLE linkHandle;

#if DBG

    WCHAR debugUnicodeBuffer[256];
    UNICODE_STRING debugUnicodeString;

#endif

    //
    // Create the root directory object for the \ArcName directory.
    //

    RtlInitUnicodeString( &nameString, L"\\ArcName" );

    InitializeObjectAttributes( &objectAttributes,
                                &nameString,
                                OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
                                NULL,
                                SePublicDefaultUnrestrictedSd );

    status = NtCreateDirectoryObject( &handle,
                                      DIRECTORY_ALL_ACCESS,
                                      &objectAttributes );
    if (!NT_SUCCESS( status )) {
        KeBugCheckEx(SYMBOLIC_INITIALIZATION_FAILED,status,1,0,0);
        return status;
    } else {
        (VOID) NtClose( handle );
    }

    //
    // Create the root directory object for the \Device directory.
    //

    RtlInitUnicodeString( &nameString, L"\\Device" );


    InitializeObjectAttributes( &objectAttributes,
                                &nameString,
                                OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
                                NULL,
                                SePublicDefaultUnrestrictedSd );

    status = NtCreateDirectoryObject( &handle,
                                      DIRECTORY_ALL_ACCESS,
                                      &objectAttributes );
    if (!NT_SUCCESS( status )) {
        KeBugCheckEx(SYMBOLIC_INITIALIZATION_FAILED,status,2,0,0);
        return status;
    } else {
        (VOID) NtClose( handle );
    }

    //
    // Create the symbolic link to the root of the system directory.
    //

    RtlInitAnsiString( &linkString, INIT_SYSTEMROOT_LINKNAME );

    status = RtlAnsiStringToUnicodeString( &linkUnicodeString,
                                           &linkString,
                                           TRUE);

    if (!NT_SUCCESS( status )) {
        KeBugCheckEx(SYMBOLIC_INITIALIZATION_FAILED,status,3,0,0);
        return status;
    }

    InitializeObjectAttributes( &objectAttributes,
                                &linkUnicodeString,
                                OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
                                NULL,
                                SePublicDefaultUnrestrictedSd );

    //
    // Use ARC device name and system path from loader.
    //

    status = RtlFormatBuffer2( deviceNameBuffer,
                                 sizeof(deviceNameBuffer),
                                 "\\ArcName\\%s%s",
                                 LoaderBlock->ArcBootDeviceName,
                                 LoaderBlock->NtBootPathName );

    if (!NT_SUCCESS(status)) {
        KeBugCheckEx(SYMBOLIC_INITIALIZATION_FAILED,status,3,1,0);
    }

    RtlInitString( &deviceNameString, deviceNameBuffer );

    status = RtlAnsiStringToUnicodeString( &deviceNameUnicodeString,
                                           &deviceNameString,
                                           TRUE );

    if (!NT_SUCCESS(status)) {
        RtlFreeUnicodeString( &linkUnicodeString );
        KeBugCheckEx(SYMBOLIC_INITIALIZATION_FAILED,status,4,0,0);
        return status;
    }

    status = NtCreateSymbolicLinkObject( &linkHandle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &objectAttributes,
                                         &deviceNameUnicodeString );

    RtlFreeUnicodeString( &linkUnicodeString );
    RtlFreeUnicodeString( &deviceNameUnicodeString );

    if (!NT_SUCCESS(status)) {
        KeBugCheckEx(SYMBOLIC_INITIALIZATION_FAILED,status,5,0,0);
        return status;
    }

    NtClose( linkHandle );

    return STATUS_SUCCESS;
}

NTSTATUS
LookupEntryPoint (
    IN PVOID DllBase,
    IN PSZ NameOfEntryPoint,
    OUT PVOID *AddressOfEntryPoint
    )
/*++

Routine Description:

    Returns the address of an entry point given the DllBase and PSZ
    name of the entry point in question

--*/

{
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    ULONG ExportSize;
    USHORT Ordinal;
    PULONG Addr;
    CHAR NameBuffer[64];

    ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)
        RtlImageDirectoryEntryToData(
            DllBase,
            TRUE,
            IMAGE_DIRECTORY_ENTRY_EXPORT,
            &ExportSize);

#if DBG
    if (!ExportDirectory) {
        DbgPrint("LookupENtryPoint: Can't locate system Export Directory\n");
    }
#endif

    if ( strlen(NameOfEntryPoint) > sizeof(NameBuffer)-2 ) {
        return STATUS_INVALID_PARAMETER;
    }

    strcpy(NameBuffer,NameOfEntryPoint);

    Ordinal = NameToOrdinal(
                NameBuffer,
                (ULONG_PTR)DllBase,
                ExportDirectory->NumberOfNames,
                (PULONG)((ULONG_PTR)DllBase + ExportDirectory->AddressOfNames),
                (PUSHORT)((ULONG_PTR)DllBase + ExportDirectory->AddressOfNameOrdinals)
                );

    //
    // If Ordinal is not within the Export Address Table,
    // then DLL does not implement function.
    //

    if ( (ULONG)Ordinal >= ExportDirectory->NumberOfFunctions ) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }

    Addr = (PULONG)((ULONG_PTR)DllBase + ExportDirectory->AddressOfFunctions);
    *AddressOfEntryPoint = (PVOID)((ULONG_PTR)DllBase + Addr[Ordinal]);
    return STATUS_SUCCESS;
}

static USHORT
NameToOrdinal (
    IN PSZ NameOfEntryPoint,
    IN ULONG_PTR DllBase,
    IN ULONG NumberOfNames,
    IN PULONG NameTableBase,
    IN PUSHORT NameOrdinalTableBase
    )
{

    ULONG SplitIndex;
    LONG CompareResult;

    if ( NumberOfNames == 0 ) {
        return (USHORT)-1;
    }

    SplitIndex = NumberOfNames >> 1;

    CompareResult = strcmp(NameOfEntryPoint, (PSZ)(DllBase + NameTableBase[SplitIndex]));

    if ( CompareResult == 0 ) {
        return NameOrdinalTableBase[SplitIndex];
    }

    if ( NumberOfNames == 1 ) {
        return (USHORT)-1;
    }

    if ( CompareResult < 0 ) {
        NumberOfNames = SplitIndex;
    } else {
        NameTableBase = &NameTableBase[SplitIndex+1];
        NameOrdinalTableBase = &NameOrdinalTableBase[SplitIndex+1];
        NumberOfNames = NumberOfNames - SplitIndex - 1;
    }

    return NameToOrdinal(NameOfEntryPoint,DllBase,NumberOfNames,NameTableBase,NameOrdinalTableBase);

}

