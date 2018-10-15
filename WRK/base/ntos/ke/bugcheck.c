/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    bugcheck.c

Abstract:

    This module implements bugcheck and system shutdown code.

--*/

#include "ki.h"
#define NOEXTAPI
#include "wdbgexts.h"
#include <inbv.h>
#include <hdlsterm.h>


extern KDDEBUGGER_DATA64 KdDebuggerDataBlock;

extern PVOID ExPoolCodeStart;
extern PVOID ExPoolCodeEnd;
extern PVOID MmPoolCodeStart;
extern PVOID MmPoolCodeEnd;
extern PVOID MmPteCodeStart;
extern PVOID MmPteCodeEnd;

extern PWD_HANDLER ExpWdHandler;
extern PVOID       ExpWdHandlerContext;

#if defined(_AMD64_)

#define PROGRAM_COUNTER(_trapframe) ((_trapframe)->Rip)

#elif defined(_X86_)

#define PROGRAM_COUNTER(_trapframe) ((_trapframe)->Eip)

#else

#error "no target architecture"

#endif

//
// Define max buffer size for the file - this value is 
// based on the previous max value used in this file
//
#define MAX_BUFFER      128

//
// Define external referenced prototypes.
//

VOID
KiDumpParameterImages (
    IN PCHAR Buffer,
    IN PULONG_PTR BugCheckParameters,
    IN ULONG NumberOfParameters,
    IN PKE_BUGCHECK_UNICODE_TO_ANSI UnicodeToAnsiRoutine
    );

VOID
KiDisplayBlueScreen (
    IN ULONG PssMessage,
    IN BOOLEAN HardErrorCalled,
    IN PCHAR HardErrorCaption,
    IN PCHAR HardErrorMessage,
    IN PCHAR StateString
    );

//
// Define forward referenced prototypes.
//

VOID
KiScanBugCheckCallbackList (
    VOID
    );

VOID
KiInvokeBugCheckEntryCallbacks (
    VOID
    );

//
// Define bugcheck recursion counter.
//

LONG KeBugCheckCount = 1;

//
// Define an owner recursion counter is used to count the number of recursive
// bugchecks taken on the bugcheck owner processor.
//

ULONG KeBugCheckOwnerRecursionCount;

//
// Define the bugcheck owner flag.  This flag is used to track the processor
// that "owns" an active bugcheck operation by virtue of the fact that it
// decremented the bugcheck recursion counter to zero.
//

#if !defined(NT_UP)

#define BUGCHECK_UNOWNED ((ULONG) -1)

volatile ULONG KeBugCheckOwner = BUGCHECK_UNOWNED;

#endif

//
// Define bugcheck active indicator.
//

BOOLEAN KeBugCheckActive = FALSE;

#if !defined(_AMD64_)

VOID
KeBugCheck (
    __in ULONG BugCheckCode
    )
{
    KeBugCheck2(BugCheckCode, 0, 0, 0, 0, NULL);
}

VOID
KeBugCheckEx (
    __in ULONG BugCheckCode,
    __in ULONG_PTR P1,
    __in ULONG_PTR P2,
    __in ULONG_PTR P3,
    __in ULONG_PTR P4
    )
{
    KeBugCheck2(BugCheckCode, P1, P2, P3, P4, NULL);
}

#endif

ULONG_PTR KiBugCheckData[5];
PUNICODE_STRING KiBugCheckDriver;

BOOLEAN
KeGetBugMessageText (
    __in ULONG MessageId,
    __out_opt PANSI_STRING ReturnedString
    )
{
    SIZE_T  i;
    PCHAR   s;
    PMESSAGE_RESOURCE_BLOCK MessageBlock;
    PCHAR Buffer;
    BOOLEAN Result;

    Result = FALSE;
    try {
        if (KiBugCodeMessages != NULL) {
            MmMakeKernelResourceSectionWritable ();
            MessageBlock = &KiBugCodeMessages->Blocks[0];
            for (i = KiBugCodeMessages->NumberOfBlocks; i; i -= 1) {
                if (MessageId >= MessageBlock->LowId &&
                    MessageId <= MessageBlock->HighId) {

                    s = (PCHAR)KiBugCodeMessages + MessageBlock->OffsetToEntries;
                    for (i = MessageId - MessageBlock->LowId; i; i -= 1) {
                        s += ((PMESSAGE_RESOURCE_ENTRY)s)->Length;
                    }

                    Buffer = (PCHAR)((PMESSAGE_RESOURCE_ENTRY)s)->Text;

                    i = strlen(Buffer) - 1;
                    while (i > 0 && (Buffer[i] == '\n'  ||
                                     Buffer[i] == '\r'  ||
                                     Buffer[i] == 0
                                    )
                          ) {
                        if (!ARGUMENT_PRESENT( ReturnedString )) {
                            Buffer[i] = 0;
                        }
                        i -= 1;
                    }

                    if (!ARGUMENT_PRESENT( ReturnedString )) {
                        InbvDisplayString((PUCHAR)Buffer);
                        InbvDisplayString((PUCHAR)"\r");
                        }
                    else {
                        ReturnedString->Buffer = Buffer;
                        ReturnedString->Length = (USHORT)(i+1);
                        ReturnedString->MaximumLength = (USHORT)(i+1);
                    }
                    Result = TRUE;
                    break;
                }
                MessageBlock += 1;
            }
        }
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        ;
    }

    return Result;
}

PCHAR
KeBugCheckUnicodeToAnsi (
    IN PUNICODE_STRING UnicodeString,
    OUT PCHAR AnsiBuffer,
    IN ULONG MaxAnsiLength
    )
{
    PCHAR Dst;
    PWSTR Src;
    ULONG Length;

    Length = UnicodeString->Length / sizeof( WCHAR );
    if (Length >= MaxAnsiLength) {
        Length = MaxAnsiLength - 1;
        }
    Src = UnicodeString->Buffer;
    Dst = AnsiBuffer;
    while (Length--) {
        *Dst++ = (UCHAR)*Src++;
        }
    *Dst = '\0';
    return AnsiBuffer;
}

VOID
KiBugCheckDebugBreak (
    IN ULONG    BreakStatus
    )
{
    do {

        try {

            //
            // Issue a breakpoint
            //

            DbgBreakPointWithStatus (BreakStatus);

        } except(EXCEPTION_EXECUTE_HANDLER) {

            HEADLESS_RSP_QUERY_INFO Response;
            NTSTATUS Status;
            SIZE_T Length;

            //
            // Failed to issue the breakpoint, must be no debugger.  Now, give
            // the headless terminal a chance to reboot the system, if there is one.
            //
            Length = sizeof(HEADLESS_RSP_QUERY_INFO);
            Status = HeadlessDispatch(HeadlessCmdQueryInformation,
                                      NULL,
                                      0,
                                      &Response,
                                      &Length
                                     );

            if (NT_SUCCESS(Status) &&
                (Response.PortType == HeadlessSerialPort) &&
                Response.Serial.TerminalAttached) {

                HeadlessDispatch(HeadlessCmdPutString,
                                 "\r\n",
                                 sizeof("\r\n"),
                                 NULL,
                                 NULL
                                );

                for (;;) {
                    HeadlessDispatch(HeadlessCmdDoBugCheckProcessing, NULL, 0, NULL, NULL);
                }

            }

            //
            // No terminal, or it failed, halt the system
            //

            try {

                HalHaltSystem();

            } except(EXCEPTION_EXECUTE_HANDLER) {

                for (;;) {
                }

            }
        }
    } while (BreakStatus != DBG_STATUS_BUGCHECK_FIRST);
}

PVOID
KiPcToFileHeader (
    IN PVOID PcValue,
    OUT PLDR_DATA_TABLE_ENTRY *DataTableEntry,
    IN LOGICAL DriversOnly,
    OUT PBOOLEAN InKernelOrHal
    )

/*++

Routine Description:

    This function returns the base of an image that contains the
    specified PcValue. An image contains the PcValue if the PcValue
    is within the ImageBase, and the ImageBase plus the size of the
    virtual image.

Arguments:

    PcValue - Supplies a PcValue.

    DataTableEntry - Supplies a pointer to a variable that receives the
        address of the data table entry that describes the image.

    DriversOnly - Supplies TRUE if the kernel and HAL should be skipped.

    InKernelOrHal - Set to TRUE if the PcValue is in the kernel or the HAL.
        This only has meaning if DriversOnly is FALSE.

Return Value:

    NULL - No image was found that contains the PcValue.

    NON-NULL - Returns the base address of the image that contains the
        PcValue.

--*/

{

    ULONG i;
    PLIST_ENTRY ModuleListHead;
    PLDR_DATA_TABLE_ENTRY Entry;
    PLIST_ENTRY Next;
    ULONG_PTR Bounds;
    PVOID ReturnBase, Base;

    //
    // If the module list has been initialized, then scan the list to
    // locate the appropriate entry.
    //

    if (KeLoaderBlock != NULL) {
        ModuleListHead = &KeLoaderBlock->LoadOrderListHead;

    } else {
        ModuleListHead = &PsLoadedModuleList;
    }

    *InKernelOrHal = FALSE;
    ReturnBase = NULL;
    Next = ModuleListHead->Flink;
    if (Next != NULL) {
        i = 0;
        while (Next != ModuleListHead) {
            if (MmIsAddressValid(Next) == FALSE) {
                return NULL;
            }

            i += 1;
            if ((i <= 2) && (DriversOnly == TRUE)) {
                Next = Next->Flink;
                continue;
            }

            Entry = CONTAINING_RECORD(Next,
                                      LDR_DATA_TABLE_ENTRY,
                                      InLoadOrderLinks);

            Next = Next->Flink;
            Base = Entry->DllBase;
            Bounds = (ULONG_PTR)Base + Entry->SizeOfImage;
            if ((ULONG_PTR)PcValue >= (ULONG_PTR)Base && (ULONG_PTR)PcValue < Bounds) {
                *DataTableEntry = Entry;
                ReturnBase = Base;
                if (i <= 2) {
                    *InKernelOrHal = TRUE;
                }

                break;
            }
        }
    }

    return ReturnBase;
}

#ifdef _X86_
#pragma optimize("y", off)      // RtlCaptureContext needs EBP to be correct
#endif

VOID
KeBugCheck2 (
    __in ULONG BugCheckCode,
    __in ULONG_PTR BugCheckParameter1,
    __in ULONG_PTR BugCheckParameter2,
    __in ULONG_PTR BugCheckParameter3,
    __in ULONG_PTR BugCheckParameter4,
    __in_opt PKTRAP_FRAME TrapFrame
    )

/*++

Routine Description:

    This function crashes the system in a controlled manner.

Arguments:

    BugCheckCode - Supplies the reason for the bugcheck.

    BugCheckParameter1-4 - Supplies additional bugcheck information.

    TrapFrame - Kernel trap frame associated with the system failure (can be NULL).

Return Value:

    None.

--*/

{
    CONTEXT ContextSave;
    ULONG PssMessage;
    PCHAR HardErrorCaption;
    PCHAR HardErrorMessage;
    KIRQL OldIrql;
    PVOID ExecutionAddress;
    PVOID ImageBase;
    PVOID VirtualAddress;
    PLDR_DATA_TABLE_ENTRY DataTableEntry;
    CHAR AnsiBuffer[MAX_BUFFER];
    PKTHREAD Thread;
    BOOLEAN InKernelOrHal;
    BOOLEAN Reboot;
    BOOLEAN HardErrorCalled;

#if !defined(NT_UP)

    KAFFINITY TargetSet;
    ULONG CurrentProcessor;
    ULONG WaitCount;

#endif

    HardErrorCalled = FALSE;
    HardErrorCaption = NULL;
    HardErrorMessage = NULL;
    ExecutionAddress = NULL;
    Thread = KeGetCurrentThread();

    //
    // Initialization
    //

    Reboot = FALSE;
    KiBugCheckDriver = NULL;
    KeBugCheckActive = TRUE;

    //
    // Try to simulate a power failure for Cluster testing
    //

    if (BugCheckCode == POWER_FAILURE_SIMULATE) {
        KiScanBugCheckCallbackList();
        HalReturnToFirmware(HalRebootRoutine);
    }

#if !defined(_AMD64_)

    //
    // Save the current IRQL in the Prcb so the debugger can extract it
    // later on for debugging purposes.
    //

    KeGetCurrentPrcb()->DebuggerSavedIRQL = KeGetCurrentIrql();

    //
    // Capture the callers context as closely as possible into the debugger's
    // processor state area of the Prcb.
    //
    // N.B. There may be some prologue code that shuffles registers such that
    //      they get destroyed.
    //

    InterlockedIncrement(&KiHardwareTrigger);
    RtlCaptureContext(&KeGetCurrentPrcb()->ProcessorState.ContextFrame);
    KiSaveProcessorControlState(&KeGetCurrentPrcb()->ProcessorState);

#endif

    //
    // This is necessary on machines where the virtual unwind that happens
    // during KeDumpMachineState() destroys the context record.
    //

    ContextSave = KeGetCurrentPrcb()->ProcessorState.ContextFrame;


    //
    // Stop the watchdog timer
    //

    if (ExpWdHandler != NULL) {
        ExpWdHandler( WdActionStopTimer, ExpWdHandlerContext, NULL, TRUE );
    }

    //
    // Get the correct string for bugchecks
    //


    switch (BugCheckCode) {

        case SYSTEM_THREAD_EXCEPTION_NOT_HANDLED:
        case KERNEL_MODE_EXCEPTION_NOT_HANDLED:
        case KMODE_EXCEPTION_NOT_HANDLED:
            PssMessage = KMODE_EXCEPTION_NOT_HANDLED;
            break;

        case DATA_BUS_ERROR:
        case NO_MORE_SYSTEM_PTES:
        case INACCESSIBLE_BOOT_DEVICE:
        case UNEXPECTED_KERNEL_MODE_TRAP:
        case ACPI_BIOS_ERROR:
        case ACPI_BIOS_FATAL_ERROR:
        case FAT_FILE_SYSTEM:
        case DRIVER_CORRUPTED_EXPOOL:
        case THREAD_STUCK_IN_DEVICE_DRIVER:
            PssMessage = BugCheckCode;
            break;

        case DRIVER_CORRUPTED_MMPOOL:
            PssMessage = DRIVER_CORRUPTED_EXPOOL;
            break;

        case NTFS_FILE_SYSTEM:
            PssMessage = FAT_FILE_SYSTEM;
            break;

        case STATUS_SYSTEM_IMAGE_BAD_SIGNATURE:
            PssMessage = BUGCODE_PSS_MESSAGE_SIGNATURE;
            break;
        default:
            PssMessage = BUGCODE_PSS_MESSAGE;
        break;
    }

    //
    // Do further processing on bugcheck codes
    //

    KiBugCheckData[0] = BugCheckCode;
    KiBugCheckData[1] = BugCheckParameter1;
    KiBugCheckData[2] = BugCheckParameter2;
    KiBugCheckData[3] = BugCheckParameter3;
    KiBugCheckData[4] = BugCheckParameter4;

    switch (BugCheckCode) {

    case FATAL_UNHANDLED_HARD_ERROR:
        //
        // If we are called by hard error then we don't want to dump the
        // processor state on the machine.
        //
        // We know that we are called by hard error because the bugcheck
        // code will be FATAL_UNHANDLED_HARD_ERROR.  If this is so then the
        // error status passed to harderr is the first parameter, and a pointer
        // to the parameter array from hard error is passed as the second
        // argument.
        //
        // The third argument is the OemCaption to be printed.
        // The last argument is the OemMessage to be printed.
        //
        {
        PULONG_PTR parameterArray;

        HardErrorCalled = TRUE;

        HardErrorCaption = (PCHAR)BugCheckParameter3;
        HardErrorMessage = (PCHAR)BugCheckParameter4;
        parameterArray = (PULONG_PTR)BugCheckParameter2;
        KiBugCheckData[0] = (ULONG)BugCheckParameter1;
        KiBugCheckData[1] = parameterArray[0];
        KiBugCheckData[2] = parameterArray[1];
        KiBugCheckData[3] = parameterArray[2];
        KiBugCheckData[4] = parameterArray[3];
        }
        break;

    case IRQL_NOT_LESS_OR_EQUAL:

        ExecutionAddress = (PVOID)BugCheckParameter4;

        if (ExecutionAddress >= ExPoolCodeStart && ExecutionAddress < ExPoolCodeEnd) {
            KiBugCheckData[0] = DRIVER_CORRUPTED_EXPOOL;
        }
        else if (ExecutionAddress >= MmPoolCodeStart && ExecutionAddress < MmPoolCodeEnd) {
            KiBugCheckData[0] = DRIVER_CORRUPTED_MMPOOL;
        }
        else if (ExecutionAddress >= MmPteCodeStart && ExecutionAddress < MmPteCodeEnd) {
            KiBugCheckData[0] = DRIVER_CORRUPTED_SYSPTES;
        }
        else {
            ImageBase = KiPcToFileHeader (ExecutionAddress,
                                          &DataTableEntry,
                                          FALSE,
                                          &InKernelOrHal);
            if (InKernelOrHal == TRUE) {

                //
                // The kernel faulted at raised IRQL.  Quite often this
                // is a driver that has unloaded without deleting its
                // lookaside lists or other resources.  Or its resources
                // are marked pageable and shouldn't be.  Detect both
                // cases here and identify the offending driver
                // whenever possible.
                //

                VirtualAddress = (PVOID)BugCheckParameter1;

                ImageBase = KiPcToFileHeader (VirtualAddress,
                                              &DataTableEntry,
                                              TRUE,
                                              &InKernelOrHal);

                if (ImageBase != NULL) {
                    KiBugCheckDriver = &DataTableEntry->BaseDllName;
                    KiBugCheckData[0] = DRIVER_PORTION_MUST_BE_NONPAGED;
                }
                else {
                    KiBugCheckDriver = MmLocateUnloadedDriver (VirtualAddress);
                    if (KiBugCheckDriver != NULL) {
                        KiBugCheckData[0] = SYSTEM_SCAN_AT_RAISED_IRQL_CAUGHT_IMPROPER_DRIVER_UNLOAD;
                    }
                }
            }
            else {
                KiBugCheckData[0] = DRIVER_IRQL_NOT_LESS_OR_EQUAL;
            }
        }

        ExecutionAddress = NULL;
        break;

    case ATTEMPTED_WRITE_TO_READONLY_MEMORY:
    case ATTEMPTED_EXECUTE_OF_NOEXECUTE_MEMORY:
    case KERNEL_MODE_EXCEPTION_NOT_HANDLED:

        if ((TrapFrame == NULL) && (BugCheckParameter3 != (ULONG_PTR)NULL)) {
            TrapFrame = (PKTRAP_FRAME)BugCheckParameter3;
        }

        //
        // Extract the execution address from the trap frame to
        // identify the component.
        //

        if ((TrapFrame != NULL) && 
            (BugCheckCode != KERNEL_MODE_EXCEPTION_NOT_HANDLED)) {
            ExecutionAddress = (PVOID) PROGRAM_COUNTER (TrapFrame);
        }

        break;

    case DRIVER_LEFT_LOCKED_PAGES_IN_PROCESS:

        ExecutionAddress = (PVOID)BugCheckParameter1;
        break;

    case DRIVER_USED_EXCESSIVE_PTES:

        DataTableEntry = (PLDR_DATA_TABLE_ENTRY)BugCheckParameter1;
        KiBugCheckDriver = &DataTableEntry->BaseDllName;

        break;

    case PAGE_FAULT_IN_NONPAGED_AREA:

        ImageBase = NULL;

        //
        // Extract the execution address from the trap frame to identify
        // component. Note a passed trap frame takes precedence over the
        // the third bugcheck parameter.
        //
        if ((TrapFrame == NULL) && (BugCheckParameter3 != (ULONG_PTR)NULL)) {
            TrapFrame = (PKTRAP_FRAME)BugCheckParameter3;
        }

        if (TrapFrame != NULL) {

            ExecutionAddress = (PVOID)PROGRAM_COUNTER
                ((PKTRAP_FRAME)TrapFrame);

            KiBugCheckData[3] = (ULONG_PTR)ExecutionAddress;

            ImageBase = KiPcToFileHeader (ExecutionAddress,
                                          &DataTableEntry,
                                          FALSE,
                                          &InKernelOrHal);
        }
        else {

            //
            // No trap frame, so no execution address either.
            //

            InKernelOrHal = TRUE;
        }

        VirtualAddress = (PVOID)BugCheckParameter1;

        if (MmIsSpecialPoolAddress (VirtualAddress) == TRUE) {

            //
            // Update the bugcheck number so the administrator gets
            // useful feedback that enabling special pool has enabled
            // the system to locate the corruptor.
            //

            if (MmIsSpecialPoolAddressFree (VirtualAddress) == TRUE) {
                if (InKernelOrHal == TRUE) {
                    KiBugCheckData[0] = PAGE_FAULT_IN_FREED_SPECIAL_POOL;
                }
                else {
                    KiBugCheckData[0] = DRIVER_PAGE_FAULT_IN_FREED_SPECIAL_POOL;
                }
            }
            else {
                if (InKernelOrHal == TRUE) {
                    KiBugCheckData[0] = PAGE_FAULT_BEYOND_END_OF_ALLOCATION;
                }
                else {
                    KiBugCheckData[0] = DRIVER_PAGE_FAULT_BEYOND_END_OF_ALLOCATION;
                }
            }
        }
        else if ((ExecutionAddress == VirtualAddress) &&
                (MmIsSessionAddress (VirtualAddress) == TRUE) &&
                ((Thread->Teb == NULL) || (IS_SYSTEM_ADDRESS(Thread->Teb)))) {
            //
            // This is a driver reference to session space from a
            // worker thread.  Since the system process has no session
            // space this is illegal and the driver must be fixed.
            //

            KiBugCheckData[0] = TERMINAL_SERVER_DRIVER_MADE_INCORRECT_MEMORY_REFERENCE;
        }
        else if (ImageBase == NULL) {
            KiBugCheckDriver = MmLocateUnloadedDriver (VirtualAddress);
            if (KiBugCheckDriver != NULL) {
                KiBugCheckData[0] = DRIVER_UNLOADED_WITHOUT_CANCELLING_PENDING_OPERATIONS;
            }
        }

        break;

    case THREAD_STUCK_IN_DEVICE_DRIVER:

        KiBugCheckDriver = (PUNICODE_STRING) BugCheckParameter3;
        break;

    default:
        break;
    }

    if (KiBugCheckDriver) {
        KeBugCheckUnicodeToAnsi(KiBugCheckDriver,
                                AnsiBuffer,
                                sizeof(AnsiBuffer));
    } else {

        //
        // This will set KiBugCheckDriver to 1 if successful.
        //

        if (ExecutionAddress) {
            KiDumpParameterImages(AnsiBuffer,
                                  (PULONG_PTR)&ExecutionAddress,
                                  1,
                                  KeBugCheckUnicodeToAnsi);
        }
    }

    if (KdPitchDebugger == FALSE ) {
        KdDebuggerDataBlock.SavedContext = (ULONG_PTR) &ContextSave;
    }

    //
    // If the user manually crashed the machine, skips the DbgPrints and
    // go to the crashdump.
    // Trying to do DbgPrint causes us to reeeter the debugger which causes
    // some problems.
    //
    // Otherwise, if the debugger is enabled, print out the information and
    // stop.
    //

    if ((BugCheckCode != MANUALLY_INITIATED_CRASH) &&
        (KdDebuggerEnabled)) {

        DbgPrint("\n*** Fatal System Error: 0x%08lx\n"
                 "                       (0x%p,0x%p,0x%p,0x%p)\n\n",
                 (ULONG)KiBugCheckData[0],
                 KiBugCheckData[1],
                 KiBugCheckData[2],
                 KiBugCheckData[3],
                 KiBugCheckData[4]);

        //
        // If the debugger is not actually connected, or the user manually
        // crashed the machine by typing .crash in the debugger, proceed to
        // "blue screen" the system.
        //
        // The call to DbgPrint above will have set the state of
        // KdDebuggerNotPresent if the debugger has become disconnected
        // since the system was booted.
        //

        if (KdDebuggerNotPresent == FALSE) {

            if (KiBugCheckDriver != NULL) {
                DbgPrint("Driver at fault: %s.\n", AnsiBuffer);
            }

            if (HardErrorCalled != FALSE) {
                if (HardErrorCaption) {
                    DbgPrint(HardErrorCaption);
                }
                if (HardErrorMessage) {
                    DbgPrint(HardErrorMessage);
                }
            }

            KiBugCheckDebugBreak (DBG_STATUS_BUGCHECK_FIRST);
        }
    }

    //
    // Freeze execution of the system by disabling interrupts and looping.
    //

    KeDisableInterrupts();
    KeRaiseIrql(HIGH_LEVEL, &OldIrql);


    //
    // Retrieve the current processor number.
    //

#if !defined(NT_UP)

    CurrentProcessor = (ULONG) KeGetCurrentPrcb()->Number;

#endif

    //
    // Don't attempt to display message more than once.
    //

    if (InterlockedDecrement (&KeBugCheckCount) == 0) {

        //
        // Set the bugcheck owner flag so that we'll be able to distinguish
        // between recursive bugchecks and bugchecks occurring on different
        // processors.
        //

#if !defined(NT_UP)

        KeBugCheckOwner = CurrentProcessor;

        //
        // Attempt to get the other processors frozen now, but don't wait
        // for them to freeze (in case someone is stuck).
        //

        TargetSet = KeActiveProcessors & ~KeGetCurrentPrcb()->SetMember;
        if (TargetSet != 0) {
            KiSendFreeze(&TargetSet, FALSE);

            //
            // Indicate to KeFreezeExecution() that KiSendFreeze() must
            // not wait for targets to be in a RUNNING state before
            // transitioning to FREEZE.
            //
    
            KeBugCheckActive = 2;

            //
            // Give the other processors one second to flush their data caches.
            //
            // N.B. This cannot be synchronized since the reason for the bugcheck
            //      may be one of the other processors failed.
            //

            KeStallExecutionProcessor(1000 * 1000);
        }

#endif

        //
        // Display the blue screen.
        //

        KiDisplayBlueScreen (PssMessage,
                             HardErrorCalled,
                             HardErrorCaption,
                             HardErrorMessage,
                             AnsiBuffer);

        //
        // Invoke bugcheck callbacks.
        //

        KiInvokeBugCheckEntryCallbacks();

        //
        // If the debugger is not enabled, attempt to enable it.
        //

        if (KdDebuggerEnabled == FALSE && KdPitchDebugger == FALSE ) {
            KdEnableDebuggerWithLock(FALSE);
        } else {
            InbvDisplayString((PUCHAR)"\r\n");
        }

        // Restore the original Context frame
        KeGetCurrentPrcb()->ProcessorState.ContextFrame = ContextSave;

        //
        // For some bugchecks we want to change the thread and context before
        // it is written to the dump file IFF it is a minidump.
        // Look at the original bugcheck data, not the processed data from
        // above
        //

#define MINIDUMP_BUGCHECK 0x10000000

        if (IoIsTriageDumpEnabled()) {
#if defined(_X86_)
            if (TrapFrame != NULL) {      
                
                //
                // Build a context frame from the provided trap frame.
                //
                
                ContextSave.ContextFlags = 
                    CONTEXT_CONTROL | CONTEXT_SEGMENTS | CONTEXT_INTEGER;
                
                KeContextFromKframes(TrapFrame, NULL, &ContextSave);
                KiBugCheckData[0] |= MINIDUMP_BUGCHECK;
            } else 
#endif
            {
                switch (BugCheckCode) {

                //
                // System thread stores a context record as the 4th parameter.
                // use that.
                // Also save the context record in case someone needs to look
                // at it.
                //

                case SYSTEM_THREAD_EXCEPTION_NOT_HANDLED:
                    if (BugCheckParameter4) {
                        ContextSave = *((PCONTEXT)BugCheckParameter4);

                        KiBugCheckData[0] |= MINIDUMP_BUGCHECK;
                    }
                    break;

#if defined (_X86_)

                case THREAD_STUCK_IN_DEVICE_DRIVER:

                    // Extract the address of the spinning code from the thread
                    // object, so the dump is based off this thread.

                    Thread = (PKTHREAD) BugCheckParameter1;

                    if (Thread->State == Running)
                    {
                        //
                        // If the thread was running, the thread is now in a
                        // frozen state and the registers are in the PRCB
                        // context
                        //
                        ULONG Processor = Thread->NextProcessor;
                        ASSERT(Processor < (ULONG) KeNumberProcessors);
                        ContextSave =
                          KiProcessorBlock[Processor]->ProcessorState.ContextFrame;
                    }
                    else
                    {
                        //
                        // This should be a uniproc machine, and the thread
                        // should be suspended.  Just get the data off the
                        // switch frame.
                        //

                        PKSWITCHFRAME SwitchFrame = (PKSWITCHFRAME)Thread->KernelStack;

                        ASSERT(Thread->State == Ready);

                        ContextSave.Esp = (ULONG)Thread->KernelStack + sizeof(KSWITCHFRAME);
                        ContextSave.Ebp = *((PULONG)(ContextSave.Esp));
                        ContextSave.Eip = SwitchFrame->RetAddr;
                    }

                    KiBugCheckData[0] |= MINIDUMP_BUGCHECK;
                    break;

                case UNEXPECTED_KERNEL_MODE_TRAP:

                    //
                    // Double fault (fatal exceptions should have passed a trap frame above)
                    //

                    if (BugCheckParameter1 == 0x8)
                    {
                        // The thread is correct in this case.
                        // Second parameter is the TSS.  If we have a TSS, convert
                        // the context and mark the bugcheck as converted.

                        PKTSS Tss = (PKTSS) BugCheckParameter2;

                        if (Tss)
                        {
                            if (Tss->EFlags & EFLAGS_V86_MASK)
                            {
                                ContextSave.SegSs = Tss->Ss & 0xffff;
                            }
                            else if (Tss->Cs & 1)
                            {
                                //
                                // It's user mode.
                                // The HardwareSegSs contains R3 data selector.
                                //

                                ContextSave.SegSs = (Tss->Ss | 3) & 0xffff;
                            }
                            else
                            {
                                ContextSave.SegSs = KGDT_R0_DATA;
                            }

                            ContextSave.SegGs = Tss->Gs & 0xffff;
                            ContextSave.SegFs = Tss->Fs & 0xffff;
                            ContextSave.SegEs = Tss->Es & 0xffff;
                            ContextSave.SegDs = Tss->Ds & 0xffff;
                            ContextSave.SegCs = Tss->Cs & 0xffff;
                            ContextSave.Esp = Tss->Esp;
                            ContextSave.Eip = Tss->Eip;
                            ContextSave.Ebp = Tss->Ebp;
                            ContextSave.Eax = Tss->Eax;
                            ContextSave.Ebx = Tss->Ebx;
                            ContextSave.Ecx = Tss->Ecx;
                            ContextSave.Edx = Tss->Edx;
                            ContextSave.Edi = Tss->Edi;
                            ContextSave.Esi = Tss->Esi;
                            ContextSave.EFlags = Tss->EFlags;
                        }

                        KiBugCheckData[0] |= MINIDUMP_BUGCHECK;
                        break;
                    }
#endif
                default:
                    break;
                }
            }

            //
            // Write a crash dump and optionally reboot if the system has been
            // so configured.
            //

            IoAddTriageDumpDataBlock(PAGE_ALIGN(KiBugCheckData[1]), PAGE_SIZE);
            IoAddTriageDumpDataBlock(PAGE_ALIGN(KiBugCheckData[2]), PAGE_SIZE);
            IoAddTriageDumpDataBlock(PAGE_ALIGN(KiBugCheckData[3]), PAGE_SIZE);
            IoAddTriageDumpDataBlock(PAGE_ALIGN(KiBugCheckData[4]), PAGE_SIZE);

            //
            // Check out if we need to save additional pages for 
            // special pool situations.
            //

            {
                ULONG_PTR CrashCode;

                CrashCode = KiBugCheckData[0] & ~MINIDUMP_BUGCHECK;

                if (CrashCode == PAGE_FAULT_BEYOND_END_OF_ALLOCATION ||
                    CrashCode == DRIVER_PAGE_FAULT_BEYOND_END_OF_ALLOCATION) {

                    //
                    // If this is a special pool buffer overrun bugcheck save the previous
                    // page containing the overran buffer. 
                    //

                    IoAddTriageDumpDataBlock(PAGE_ALIGN(KiBugCheckData[1] - PAGE_SIZE), PAGE_SIZE);
                }
                else if (CrashCode == DRIVER_IRQL_NOT_LESS_OR_EQUAL &&
                         MmIsSpecialPoolAddress((PVOID)(KiBugCheckData[1]))){

                    //
                    // A special pool buffer overrun at >=DPC level is classified as
                    // DRIVER_IRQL_NOT_LESS_OR_EQUAL. Take care of saving the previous
                    // page for better clues during debugging.
                    //

                    IoAddTriageDumpDataBlock(PAGE_ALIGN(KiBugCheckData[1] - PAGE_SIZE), PAGE_SIZE);
                }
            }

            //
            // If the DPC stack is active, save that data page as well.
            //

#if defined (_X86_)
            if (KeGetCurrentPrcb()->DpcRoutineActive)
            {
                IoAddTriageDumpDataBlock(PAGE_ALIGN(KeGetCurrentPrcb()->DpcRoutineActive), PAGE_SIZE);
            }
#endif
        }

        IoWriteCrashDump((ULONG)KiBugCheckData[0],
                         KiBugCheckData[1],
                         KiBugCheckData[2],
                         KiBugCheckData[3],
                         KiBugCheckData[4],
                         &ContextSave,
                         Thread,
                         &Reboot);

    } else {

        //
        // If we reach this point then the bugcheck recursion count had
        // already been decremented by the time we decremented it above.  This
        // means that we're either bugchecking on multiple processors
        // simultaneously or that we took a recursive fault causing us to
        // reenter the bugcheck routine.
        //
        // In the multiprocessor kernel, start by waiting for the bugcheck
        // owner (that is, the agent that decremented the bugcheck recursion
        // count to 0) to move the bugcheck owner flag away from its initial
        // distinguished value.
        //
        // N.B. There is a very small window where the owner could take a
        //      recursive bugcheck after decrementing the bugcheck recursion
        //      counter and before setting the bugcheck owner flag.  If we
        //      time out waiting for the flag to change, then assume we've hit
        //      this window and try to enter the kernel debugger.
        //

#if !defined(NT_UP)

        WaitCount = 0;
        while (KeBugCheckOwner == BUGCHECK_UNOWNED) {
            if (WaitCount == 200) {
                KiBugCheckDebugBreak(DBG_STATUS_BUGCHECK_SECOND);
            }

            KeStallExecutionProcessor(1000);
            WaitCount += 1;
        }

        //
        // If this is a recursive bugcheck, then continue to the recursive
        // bugcheck handling code.  Otherwise, wait here forever in order to
        // let the bugcheck owner carry out the bugcheck operation,
        // periodically checking for freeze requests from the bugcheck owner.
        //

        if (CurrentProcessor != KeBugCheckOwner) {
            while (1) {
                KiPollFreezeExecution();
            }
        }

#endif // !defined(NT_UP)

        //
        // Let the first recursive bugcheck fall through to the remainder of
        // the bugcheck processing code.  If this generates a second recursive
        // bugcheck, then try to halt the processor, breaking into the
        // debugger if possible.  If this generates a third recursive
        // bugcheck, then halt the processor here.
        //

        KeBugCheckOwnerRecursionCount += 1;
        if (KeBugCheckOwnerRecursionCount == 1) {
            NOTHING;

        } else if (KeBugCheckOwnerRecursionCount == 2) {
            KiBugCheckDebugBreak(DBG_STATUS_BUGCHECK_SECOND);

        } else {
            while (1) {
                PAUSE_PROCESSOR;
            }
        }
    }

    //
    // Invoke bugcheck callbacks after crashdump, so the callbacks will
    // not prevent us from crashdumping.
    //

    KiScanBugCheckCallbackList();

    //
    // Start the watchdog timer
    //

    if (ExpWdHandler != NULL) {
        ExpWdHandler( WdActionStartTimer, ExpWdHandlerContext, NULL, TRUE );
    }

    //
    // Reboot the machine if necessary.
    //

    if (Reboot) {

#if defined(_AMD64_) && !defined(NT_UP)

        //
        // Unfreeze any frozen processors.  This is needed for the two 
        // functions below working properly.
        //

        KiResumeForReboot = TRUE;
        KiSendThawExecution (FALSE);
#endif

        DbgUnLoadImageSymbols (NULL, (PVOID)-1, 0);
        HalReturnToFirmware (HalRebootRoutine);
    }

    //
    // Attempt to enter the kernel debugger.
    //

    KiBugCheckDebugBreak (DBG_STATUS_BUGCHECK_SECOND);
}

#ifdef _X86_
#pragma optimize("", on)
#endif

VOID
KeEnterKernelDebugger (
    VOID
    )

/*++

Routine Description:

    This function crashes the system in a controlled manner attempting
    to invoke the kernel debugger.

Arguments:

    None.

Return Value:

    None.

--*/

{

#if !defined(i386)

    KIRQL OldIrql;

#endif

    //
    // Freeze execution of the system by disabling interrupts and looping.
    //

    KiHardwareTrigger = 1;
    KeDisableInterrupts();
#if !defined(i386)
    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
#endif
    if (InterlockedDecrement (&KeBugCheckCount) == 0) {
        if (KdDebuggerEnabled == FALSE) {
            if ( KdPitchDebugger == FALSE ) {
                KdInitSystem(0, NULL);
            }
        }
    }

    KiBugCheckDebugBreak (DBG_STATUS_FATAL);
}

NTKERNELAPI
BOOLEAN
KeDeregisterBugCheckCallback (
    __inout PKBUGCHECK_CALLBACK_RECORD CallbackRecord
    )

/*++

Routine Description:

    This function deregisters a bugcheck callback record.

Arguments:

    CallbackRecord - Supplies a pointer to a bugcheck callback record.

Return Value:

    If the specified bugcheck callback record is successfully deregistered,
    then a value of TRUE is returned. Otherwise, a value of FALSE is returned.

--*/

{

    BOOLEAN Deregister;
    KIRQL OldIrql;

    //
    // Raise IRQL to HIGH_LEVEL and acquire the bugcheck callback list
    // spinlock.
    //

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
    KiAcquireSpinLock(&KeBugCheckCallbackLock);

    //
    // If the specified callback record is currently registered, then
    // deregister the callback record.
    //

    Deregister = FALSE;
    if (CallbackRecord->State == BufferInserted) {
        CallbackRecord->State = BufferEmpty;
        RemoveEntryList(&CallbackRecord->Entry);
        Deregister = TRUE;
    }

    //
    // Release the bugcheck callback spinlock, lower IRQL to its previous
    // value, and return whether the callback record was successfully
    // deregistered.
    //

    KiReleaseSpinLock(&KeBugCheckCallbackLock);
    KeLowerIrql(OldIrql);
    return Deregister;
}

NTKERNELAPI
BOOLEAN
KeRegisterBugCheckCallback (
    __out PKBUGCHECK_CALLBACK_RECORD CallbackRecord,
    __in PKBUGCHECK_CALLBACK_ROUTINE CallbackRoutine,
    __in PVOID Buffer,
    __in ULONG Length,
    __in PUCHAR Component
    )

/*++

Routine Description:

    This function registers a bugcheck callback record. If the system
    crashes, then the specified function will be called during bugcheck
    processing so it may dump additional state in the specified bugcheck
    buffer.

    N.B. Bugcheck callback routines are called in reverse order of
         registration, i.e., in LIFO order.

Arguments:

    CallbackRecord - Supplies a pointer to a callback record.

    CallbackRoutine - Supplies a pointer to the callback routine.

    Buffer - Supplies a pointer to the bugcheck buffer.

    Length - Supplies the length of the bugcheck buffer in bytes.

    Component - Supplies a pointer to a zero terminated component
        identifier.

Return Value:

    If the specified bugcheck callback record is successfully registered,
    then a value of TRUE is returned. Otherwise, a value of FALSE is returned.

--*/

{

    BOOLEAN Inserted;
    KIRQL OldIrql;

    //
    // Raise IRQL to HIGH_LEVEL and acquire the bugcheck callback list
    // spinlock.
    //

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
    KiAcquireSpinLock(&KeBugCheckCallbackLock);

    //
    // If the specified callback record is currently not registered, then
    // register the callback record.
    //

    Inserted = FALSE;
    if (CallbackRecord->State == BufferEmpty) {
        CallbackRecord->CallbackRoutine = CallbackRoutine;
        CallbackRecord->Buffer = Buffer;
        CallbackRecord->Length = Length;
        CallbackRecord->Component = Component;
        CallbackRecord->Checksum =
            ((ULONG_PTR)CallbackRoutine + (ULONG_PTR)Buffer + Length + (ULONG_PTR)Component);

        CallbackRecord->State = BufferInserted;
        InsertHeadList(&KeBugCheckCallbackListHead, &CallbackRecord->Entry);
        Inserted = TRUE;
    }

    //
    // Release the bugcheck callback spinlock, lower IRQL to its previous
    // value, and return whether the callback record was successfully
    // registered.
    //

    KiReleaseSpinLock(&KeBugCheckCallbackLock);
    KeLowerIrql(OldIrql);
    return Inserted;
}

VOID
KiScanBugCheckCallbackList (
    VOID
    )

/*++

Routine Description:

    This function scans the bugcheck callback list and calls each bugcheck
    callback routine so it can dump component specific information
    that may identify the cause of the bugcheck.

    N.B. The scan of the bugcheck callback list is performed VERY
        carefully. Bugcheck callback routines are called at HIGH_LEVEL
        and may not acquire ANY resources.

Arguments:

    None.

Return Value:

    None.

--*/

{

    PKBUGCHECK_CALLBACK_RECORD CallbackRecord;
    ULONG_PTR Checksum;
    ULONG Index;
    PLIST_ENTRY LastEntry;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    PUCHAR Source;

    //
    // If the bugcheck callback listhead is not initialized, then the
    // bugcheck has occured before the system has gotten far enough
    // in the initialization code to enable anyone to register a callback.
    //

    ListHead = &KeBugCheckCallbackListHead;
    if ((ListHead->Flink != NULL) && (ListHead->Blink != NULL)) {

        //
        // Scan the bugcheck callback list.
        //

        LastEntry = ListHead;
        NextEntry = ListHead->Flink;
        while (NextEntry != ListHead) {

            //
            // The next entry address must be aligned properly, the
            // callback record must be readable, and the callback record
            // must have back link to the last entry.
            //

            if (((ULONG_PTR)NextEntry & (sizeof(ULONG_PTR) - 1)) != 0) {
                return;

            } else {
                CallbackRecord = CONTAINING_RECORD(NextEntry,
                                                   KBUGCHECK_CALLBACK_RECORD,
                                                   Entry);

                Source = (PUCHAR)CallbackRecord;
                for (Index = 0; Index < sizeof(KBUGCHECK_CALLBACK_RECORD); Index += 1) {
                    if (MmIsAddressValid((PVOID)Source) == FALSE) {
                        return;
                    }

                    Source += 1;
                }

                if (CallbackRecord->Entry.Blink != LastEntry) {
                    return;
                }

                //
                // If the callback record has a state of inserted and the
                // computed checksum matches the callback record checksum,
                // then call the specified bugcheck callback routine.
                //

                Checksum = (ULONG_PTR)CallbackRecord->CallbackRoutine;
                Checksum += (ULONG_PTR)CallbackRecord->Buffer;
                Checksum += CallbackRecord->Length;
                Checksum += (ULONG_PTR)CallbackRecord->Component;
                if ((CallbackRecord->State == BufferInserted) &&
                    (CallbackRecord->Checksum == Checksum)) {

                    //
                    // Call the specified bugcheck callback routine and
                    // handle any exceptions that occur.
                    //

                    CallbackRecord->State = BufferStarted;
                    try {
                        (CallbackRecord->CallbackRoutine)(CallbackRecord->Buffer,
                                                          CallbackRecord->Length);

                        CallbackRecord->State = BufferFinished;

                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        CallbackRecord->State = BufferIncomplete;
                    }
                }
            }

            LastEntry = NextEntry;
            NextEntry = NextEntry->Flink;
        }
    }

    return;
}

NTKERNELAPI
BOOLEAN
KeDeregisterBugCheckReasonCallback (
    __inout PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord
    )

/*++

Routine Description:

    This function deregisters a bugcheck callback record.

Arguments:

    CallbackRecord - Supplies a pointer to a bugcheck callback record.

Return Value:

    If the specified bugcheck callback record is successfully deregistered,
    then a value of TRUE is returned. Otherwise, a value of FALSE is returned.

--*/

{

    BOOLEAN Deregister;
    KIRQL OldIrql;

    //
    // Raise IRQL to HIGH_LEVEL and acquire the bugcheck callback list
    // spinlock.
    //

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
    KiAcquireSpinLock(&KeBugCheckCallbackLock);

    //
    // If the specified callback record is currently registered, then
    // deregister the callback record.
    //

    Deregister = FALSE;
    if (CallbackRecord->State == BufferInserted) {
        CallbackRecord->State = BufferEmpty;
        RemoveEntryList(&CallbackRecord->Entry);
        Deregister = TRUE;
    }

    //
    // Release the bugcheck callback spinlock, lower IRQL to its previous
    // value, and return whether the callback record was successfully
    // deregistered.
    //

    KiReleaseSpinLock(&KeBugCheckCallbackLock);
    KeLowerIrql(OldIrql);
    return Deregister;
}

NTKERNELAPI
BOOLEAN
KeRegisterBugCheckReasonCallback (
    __out PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord,
    __in PKBUGCHECK_REASON_CALLBACK_ROUTINE CallbackRoutine,
    __in KBUGCHECK_CALLBACK_REASON Reason,
    __in PUCHAR Component
    )

/*++

Routine Description:

    This function registers a bugcheck callback record. If the system
    crashes, then the specified function will be called during bugcheck
    processing.

    N.B. Bugcheck callback routines are called in reverse order of
         registration, i.e., in LIFO order.

Arguments:

    CallbackRecord - Supplies a pointer to a callback record.

    CallbackRoutine - Supplies a pointer to the callback routine.

    Reason - Specifies the conditions under which the callback
             should be called.

    Component - Supplies a pointer to a zero terminated component
        identifier.

Return Value:

    If the specified bugcheck callback record is successfully registered,
    then a value of TRUE is returned. Otherwise, a value of FALSE is returned.

--*/

{

    BOOLEAN Inserted;
    KIRQL OldIrql;

    //
    // Raise IRQL to HIGH_LEVEL and acquire the bugcheck callback list
    // spinlock.
    //

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
    KiAcquireSpinLock(&KeBugCheckCallbackLock);

    //
    // If the specified callback record is currently not registered, then
    // register the callback record.
    //

    Inserted = FALSE;
    if (CallbackRecord->State == BufferEmpty) {
        CallbackRecord->CallbackRoutine = CallbackRoutine;
        CallbackRecord->Reason = Reason;
        CallbackRecord->Component = Component;
        CallbackRecord->Checksum =
            ((ULONG_PTR)CallbackRoutine + Reason + (ULONG_PTR)Component);

        CallbackRecord->State = BufferInserted;
        InsertHeadList(&KeBugCheckReasonCallbackListHead,
                       &CallbackRecord->Entry);
        Inserted = TRUE;
    }

    //
    // Release the bugcheck callback spinlock, lower IRQL to its previous
    // value, and return whether the callback record was successfully
    // registered.
    //

    KiReleaseSpinLock(&KeBugCheckCallbackLock);
    KeLowerIrql(OldIrql);

    return Inserted;
}

VOID
KiInvokeBugCheckEntryCallbacks (
    VOID
    )

/*++

Routine Description:

    This function scans the bugcheck reason callback list and calls
    each bugcheck entry callback routine.

    This may seem like a duplication of KiScanBugCheckCallbackList
    but the critical difference is that the bugcheck entry callbacks
    are called immediately upon entry to KeBugCheck2 whereas
    KSBCCL does not invoke its callbacks until after all bugcheck
    processing has finished.

    In order to avoid people from abusing this callback it's
    semi-private and the reason -- KbCallbackReserved1 -- has
    an obscure name.

    N.B. The scan of the bugcheck callback list is performed VERY
        carefully. Bugcheck callback routines may be called at HIGH_LEVEL
        and may not acquire ANY resources.

Arguments:

    None.

Return Value:

    None.

--*/

{

    PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord;
    ULONG_PTR Checksum;
    PLIST_ENTRY LastEntry;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    PUCHAR Va;
    ULONG Pages;

    //
    // If the bugcheck callback listhead is not initialized, then the
    // bugcheck has occured before the system has gotten far enough
    // in the initialization code to enable anyone to register a callback.
    //

    ListHead = &KeBugCheckReasonCallbackListHead;
    if (ListHead->Flink == NULL || ListHead->Blink == NULL) {
        return;
    }

    //
    // Scan the bugcheck callback list.
    //

    LastEntry = ListHead;
    NextEntry = ListHead->Flink;
    while (NextEntry != ListHead) {

        //
        // The next entry address must be aligned properly, the
        // callback record must be readable, and the callback record
        // must have back link to the last entry.
        //

        if (((ULONG_PTR)NextEntry & (sizeof(ULONG_PTR) - 1)) != 0) {
            return;
        }

        CallbackRecord = CONTAINING_RECORD(NextEntry,
                                           KBUGCHECK_REASON_CALLBACK_RECORD,
                                           Entry);

        //
        // Verify that the callback record is still valid.
        //

        Va = (PUCHAR) PAGE_ALIGN (CallbackRecord);
        Pages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (CallbackRecord,
                                                sizeof (*CallbackRecord));

        while (Pages) {
            if (!MmIsAddressValid (Va)) {
                return;
            }
            Va += PAGE_SIZE;
            Pages--;
        }

        if (CallbackRecord->Entry.Blink != LastEntry) {
            return;
        }

        LastEntry = NextEntry;
        NextEntry = NextEntry->Flink;

        //
        // If the callback record has a state of inserted and the
        // computed checksum matches the callback record checksum,
        // then call the specified bugcheck callback routine.
        //

        Checksum = (ULONG_PTR)CallbackRecord->CallbackRoutine;
        Checksum += (ULONG_PTR)CallbackRecord->Reason;
        Checksum += (ULONG_PTR)CallbackRecord->Component;
        if ((CallbackRecord->State != BufferInserted) ||
            (CallbackRecord->Checksum != Checksum) ||
            (CallbackRecord->Reason != KbCallbackReserved1) ||
            MmIsAddressValid((PVOID)(ULONG_PTR)CallbackRecord->
                             CallbackRoutine) == FALSE) {

            continue;
        }

        //
        // Call the specified bugcheck callback routine and
        // handle any exceptions that occur.
        //

        try {
            (CallbackRecord->CallbackRoutine)(KbCallbackReserved1,
                                              CallbackRecord,
                                              NULL, 0);
            CallbackRecord->State = BufferFinished;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            CallbackRecord->State = BufferIncomplete;
        }
    }
}

