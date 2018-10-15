 /*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    genxx.inc

Abstract:

    This file contains common code to generate assembler definitions.

--*/

    //
    // Size of a pointer in bytes.
    //

    genCom("Pointer size in bytes")

    genVal(SizeofPointer, sizeof(PVOID))

    //
    // Process state enumerated type definitions.
    //

    genCom("Process State Enumerated Type Values")

    genVal(ProcessInMemory, ProcessInMemory)
    genVal(ProcessOutOfMemory, ProcessOutOfMemory)
    genVal(ProcessInTransition, ProcessInTransition)

    //
    // Thread state enumerated type definitions.
    //

    genCom("Thread State Enumerated Type Values")

    genVal(Initialized, Initialized)
    genVal(Ready, Ready)
    genVal(Running, Running)
    genVal(Standby, Standby)
    genVal(Terminated, Terminated)
    genVal(Waiting, Waiting)

    //
    // Wait reason and wait type enumerated type definitions.
    //

    EnableInc(HAL)

    genCom("Wait Reason and Wait Type Enumerated Type Values")

    genVal(WrExecutive, Executive)
    
    DisableInc(HAL)

    genVal(WrMutex, WrMutex)
    genVal(WrDispatchInt, WrDispatchInt)
    genVal(WrQuantumEnd, WrQuantumEnd)
    genVal(WrEventPair, WrEventPair)
    genVal(WaitAny, WaitAny)
    genVal(WaitAll, WaitAll)

    //
    // APC state structure offset definitions.
    //

    genCom("Apc State Structure Offset Definitions")

    genDef(As, KAPC_STATE, ApcListHead)
    EnableInc(HAL)
    genDef(As, KAPC_STATE, Process)
    DisableInc(HAL)
    genDef(As, KAPC_STATE, KernelApcInProgress)
    genDef(As, KAPC_STATE, KernelApcPending)
    genDef(As, KAPC_STATE, UserApcPending)

    //
    // Bugcheck code definitions
    //

    EnableInc(HAL)

    genCom("Bugcheck Code Definitions")

    genVal(APC_INDEX_MISMATCH, APC_INDEX_MISMATCH)
    genVal(ATTEMPTED_SWITCH_FROM_DPC, ATTEMPTED_SWITCH_FROM_DPC)
    genVal(DATA_BUS_ERROR, DATA_BUS_ERROR)
    genVal(DATA_COHERENCY_EXCEPTION, DATA_COHERENCY_EXCEPTION)
    genVal(HAL1_INITIALIZATION_FAILED, HAL1_INITIALIZATION_FAILED)
    genVal(INSTRUCTION_BUS_ERROR, INSTRUCTION_BUS_ERROR)
    genVal(INSTRUCTION_COHERENCY_EXCEPTION, INSTRUCTION_COHERENCY_EXCEPTION)
    genVal(INTERRUPT_EXCEPTION_NOT_HANDLED, INTERRUPT_EXCEPTION_NOT_HANDLED)
    genVal(INTERRUPT_UNWIND_ATTEMPTED, INTERRUPT_UNWIND_ATTEMPTED)
    genVal(INVALID_AFFINITY_SET, INVALID_AFFINITY_SET)
    genVal(INVALID_DATA_ACCESS_TRAP, INVALID_DATA_ACCESS_TRAP)
    genVal(IRQL_GT_ZERO_AT_SYSTEM_SERVICE, IRQL_GT_ZERO_AT_SYSTEM_SERVICE)
    genVal(IRQL_NOT_LESS_OR_EQUAL, IRQL_NOT_LESS_OR_EQUAL)
    genVal(KMODE_EXCEPTION_NOT_HANDLED, KMODE_EXCEPTION_NOT_HANDLED)
    genVal(NMI_HARDWARE_FAILURE, NMI_HARDWARE_FAILURE)
    genVal(NO_USER_MODE_CONTEXT, NO_USER_MODE_CONTEXT)
    genVal(PAGE_FAULT_WITH_INTERRUPTS_OFF, PAGE_FAULT_WITH_INTERRUPTS_OFF)
    genVal(PANIC_STACK_SWITCH, PANIC_STACK_SWITCH)
    genVal(SPIN_LOCK_INIT_FAILURE, SPIN_LOCK_INIT_FAILURE)
    genVal(SYSTEM_EXIT_OWNED_MUTEX, SYSTEM_EXIT_OWNED_MUTEX)
    genVal(SYSTEM_SERVICE_EXCEPTION, SYSTEM_SERVICE_EXCEPTION)
    genVal(SYSTEM_UNWIND_PREVIOUS_USER, SYSTEM_UNWIND_PREVIOUS_USER)
    genVal(TRAP_CAUSE_UNKNOWN, TRAP_CAUSE_UNKNOWN)
    genVal(UNEXPECTED_KERNEL_MODE_TRAP, UNEXPECTED_KERNEL_MODE_TRAP)
    genVal(HARDWARE_INTERRUPT_STORM, HARDWARE_INTERRUPT_STORM)
    genVal(RECURSIVE_MACHINE_CHECK, RECURSIVE_MACHINE_CHECK)
    genVal(RECURSIVE_NMI, RECURSIVE_NMI)

    DisableInc(HAL)

    //
    // Breakpoint types
    //

    EnableInc(HAL)

    genCom("Breakpoint type definitions")

    genVal(DBG_STATUS_CONTROL_C, DBG_STATUS_CONTROL_C)
    genVal(STATUS_ASSERTION_FAILURE, STATUS_ASSERTION_FAILURE)

    DisableInc(HAL)

    //
    // Client Id structure offset definitions.
    //

    genCom("Client Id Structure Offset Definitions")

    genDef(Cid, CLIENT_ID, UniqueProcess)
    genDef(Cid, CLIENT_ID, UniqueThread)

    //
    // Critical section structure offset definitions.
    //

    genCom("Critical Section Structure Offset Definitions")

    genDef(Cs, RTL_CRITICAL_SECTION, DebugInfo)
    genDef(Cs, RTL_CRITICAL_SECTION, LockCount)
    genDef(Cs, RTL_CRITICAL_SECTION, RecursionCount)
    genDef(Cs, RTL_CRITICAL_SECTION, OwningThread)
    genDef(Cs, RTL_CRITICAL_SECTION, LockSemaphore)
    genDef(Cs, RTL_CRITICAL_SECTION, SpinCount)

    //
    // Critical section debug information structure offset definitions.
    //

    genCom("Critical Section Debug Information Structure Offset Definitions")

    genDef(Cs, RTL_CRITICAL_SECTION_DEBUG, Type)
    genDef(Cs, RTL_CRITICAL_SECTION_DEBUG, CreatorBackTraceIndex)
    genDef(Cs, RTL_CRITICAL_SECTION_DEBUG, CriticalSection)
    genDef(Cs, RTL_CRITICAL_SECTION_DEBUG, ProcessLocksList)
    genDef(Cs, RTL_CRITICAL_SECTION_DEBUG, EntryCount)
    genDef(Cs, RTL_CRITICAL_SECTION_DEBUG, ContentionCount)

    //
    // Exception record offset, flag, and enumerated type definitions.
    //

    EnableInc(HAL)

    genCom("Exception Record Offset, Flag, and Enumerated Type Definitions")

    genVal(EXCEPTION_NONCONTINUABLE, EXCEPTION_NONCONTINUABLE)
    genVal(EXCEPTION_UNWINDING, EXCEPTION_UNWINDING)
    genVal(EXCEPTION_EXIT_UNWIND, EXCEPTION_EXIT_UNWIND)
    genVal(EXCEPTION_STACK_INVALID, EXCEPTION_STACK_INVALID)
    genVal(EXCEPTION_NESTED_CALL, EXCEPTION_NESTED_CALL)
    genVal(EXCEPTION_TARGET_UNWIND, EXCEPTION_TARGET_UNWIND)
    genVal(EXCEPTION_COLLIDED_UNWIND, EXCEPTION_COLLIDED_UNWIND)
    genVal(EXCEPTION_UNWIND, EXCEPTION_UNWIND)
    genVal(EXCEPTION_EXECUTE_HANDLER, EXCEPTION_EXECUTE_HANDLER)
    genVal(EXCEPTION_CONTINUE_SEARCH, EXCEPTION_CONTINUE_SEARCH)
    genVal(EXCEPTION_CONTINUE_EXECUTION, EXCEPTION_CONTINUE_EXECUTION)

#if defined(_X86_)

    genVal(EXCEPTION_CHAIN_END, (ULONG)EXCEPTION_CHAIN_END)

    genVal(FIXED_NTVDMSTATE_LINEAR, (ULONG)FIXED_NTVDMSTATE_LINEAR_PC_AT)
#endif

    genSpc()

    genVal(ExceptionContinueExecution, ExceptionContinueExecution)
    genVal(ExceptionContinueSearch, ExceptionContinueSearch)
    genVal(ExceptionNestedException, ExceptionNestedException)
    genVal(ExceptionCollidedUnwind, ExceptionCollidedUnwind)

    genSpc()

    genDef(Er, EXCEPTION_RECORD, ExceptionCode)
    genDef(Er, EXCEPTION_RECORD, ExceptionFlags)
    genDef(Er, EXCEPTION_RECORD, ExceptionRecord)
    genDef(Er, EXCEPTION_RECORD, ExceptionAddress)
    genDef(Er, EXCEPTION_RECORD, NumberParameters)
    genDef(Er, EXCEPTION_RECORD, ExceptionInformation)
    genVal(ExceptionRecordLength, (sizeof(EXCEPTION_RECORD) + 15) & (~15))

    DisableInc(HAL)

    //
    // Fast Mutex structure offset definitions.
    //

    EnableInc(HAL)

    genCom("Fast Mutex Structure Offset Definitions")

    genDef(Fm, FAST_MUTEX, Count)
    genDef(Fm, FAST_MUTEX, Owner)
    genDef(Fm, FAST_MUTEX, Contention)
    genDef(Fm, FAST_MUTEX, Gate)
    genDef(Fm, FAST_MUTEX, OldIrql)

    //
    // Interrupt priority request level definitions
    //

    genCom("Interrupt Priority Request Level Definitions")

    genVal(PASSIVE_LEVEL, PASSIVE_LEVEL)
    genVal(APC_LEVEL, APC_LEVEL)
    genVal(DISPATCH_LEVEL, DISPATCH_LEVEL)

#if defined(_AMD64_)

    genVal(CLOCK_LEVEL, CLOCK_LEVEL)

#endif

#if defined(_X86_)

    genVal(CLOCK1_LEVEL, CLOCK1_LEVEL)
    genVal(CLOCK2_LEVEL, CLOCK2_LEVEL)

#endif

    genVal(IPI_LEVEL, IPI_LEVEL)
    genVal(POWER_LEVEL, POWER_LEVEL)
    genVal(PROFILE_LEVEL, PROFILE_LEVEL)
    genVal(HIGH_LEVEL, HIGH_LEVEL)

#if defined(_X86_) || defined(_AMD64_)

    genTxt("ifdef NT_UP\n")
    genVal(SYNCH_LEVEL, DISPATCH_LEVEL)
    genTxt("else\n")
    genVal(SYNCH_LEVEL, IPI_LEVEL - 2)
    genTxt("endif\n")

#else

    genTxt("#ifdef NT_UP\n")
    genVal(SYNCH_LEVEL, DISPATCH_LEVEL)
    genTxt("#else\n")
    genVal(SYNCH_LEVEL, IPI_LEVEL - 2)
    genTxt("#endif\n")

#endif

    //
    // Large integer structure offset definitions.
    //

    genCom("Large Integer Structure Offset Definitions")

    genDef(Li, LARGE_INTEGER, LowPart)
    genDef(Li, LARGE_INTEGER, HighPart)

    //
    // List entry structure offset definitions.
    //

    genCom("List Entry Structure Offset Definitions")

    genDef(Ls, LIST_ENTRY, Flink)
    genDef(Ls, LIST_ENTRY, Blink)

    //
    // String structure offset definitions.
    //

    genCom("String Structure Offset Definitions")

    genDef(Str, STRING, Length)
    genDef(Str, STRING, MaximumLength)
    genDef(Str, STRING, Buffer)

    //
    // System time structure offset definitions.
    //

#if defined(_X86_)

    genCom("System Time Structure Offset Definitions")

    genAlt(StLowTime, KSYSTEM_TIME, LowPart)
    genDef(St, KSYSTEM_TIME, High1Time)
    genDef(St, KSYSTEM_TIME, High2Time)

#endif

    //
    // User shared data offset definitions.
    //

    genCom("User Shared Data Structure Offset Definitions")

    genDef(Us, KUSER_SHARED_DATA, TickCountMultiplier)
    genDef(Us, KUSER_SHARED_DATA, InterruptTime)
    genDef(Us, KUSER_SHARED_DATA, SystemTime)
    genDef(Us, KUSER_SHARED_DATA, TimeZoneBias)
    genDef(Us, KUSER_SHARED_DATA, ImageNumberLow)
    genDef(Us, KUSER_SHARED_DATA, ImageNumberHigh)
    genDef(Us, KUSER_SHARED_DATA, NtSystemRoot)
    genDef(Us, KUSER_SHARED_DATA, MaxStackTraceDepth)
    genDef(Us, KUSER_SHARED_DATA, CryptoExponent)
    genDef(Us, KUSER_SHARED_DATA, TimeZoneId)
    genDef(Us, KUSER_SHARED_DATA, LargePageMinimum)
    genDef(Us, KUSER_SHARED_DATA, Reserved2)
    genDef(Us, KUSER_SHARED_DATA, NtProductType)
    genDef(Us, KUSER_SHARED_DATA, ProductTypeIsValid)
    genDef(Us, KUSER_SHARED_DATA, NtMajorVersion)
    genDef(Us, KUSER_SHARED_DATA, NtMinorVersion)
    genDef(Us, KUSER_SHARED_DATA, ProcessorFeatures)
    genDef(Us, KUSER_SHARED_DATA, Reserved1)
    genDef(Us, KUSER_SHARED_DATA, Reserved3)
    genDef(Us, KUSER_SHARED_DATA, TimeSlip)
    genDef(Us, KUSER_SHARED_DATA, AlternativeArchitecture)
    genDef(Us, KUSER_SHARED_DATA, SystemExpirationDate)
    genDef(Us, KUSER_SHARED_DATA, SuiteMask)
    genDef(Us, KUSER_SHARED_DATA, KdDebuggerEnabled)
    genDef(Us, KUSER_SHARED_DATA, ActiveConsoleId)
    genDef(Us, KUSER_SHARED_DATA, DismountCount)
    genDef(Us, KUSER_SHARED_DATA, ComPlusPackage)
    genDef(Us, KUSER_SHARED_DATA, LastSystemRITEventTickCount)
    genDef(Us, KUSER_SHARED_DATA, NumberOfPhysicalPages)
    genDef(Us, KUSER_SHARED_DATA, SafeBootMode)
    genDef(Us, KUSER_SHARED_DATA, TraceLogging)
    genDef(Us, KUSER_SHARED_DATA, TestRetInstruction)
    genDef(Us, KUSER_SHARED_DATA, SystemCall)
    genDef(Us, KUSER_SHARED_DATA, SystemCallReturn)
    genDef(Us, KUSER_SHARED_DATA, SystemCallPad)
    genDef(Us, KUSER_SHARED_DATA, TickCount)
    genDef(Us, KUSER_SHARED_DATA, TickCountQuad)
    genDef(Us, KUSER_SHARED_DATA, Wow64SharedInformation)

    //
    // Time structure offset definitions.
    //

    genCom("Time Structure Offset Definitions")

    genAlt(TmLowTime, LARGE_INTEGER, LowPart)
    genAlt(TmHighTime, LARGE_INTEGER , HighPart)

    DisableInc(HAL)

    //
    // Thread switch counter structure offset definitions.
    //

    genCom("Thread Switch Counter Offset Definitions")

    genDef(Tw, KTHREAD_SWITCH_COUNTERS, FindAny)
    genDef(Tw, KTHREAD_SWITCH_COUNTERS, FindIdeal)
    genDef(Tw, KTHREAD_SWITCH_COUNTERS, FindLast)
    genDef(Tw, KTHREAD_SWITCH_COUNTERS, IdleAny)
    genDef(Tw, KTHREAD_SWITCH_COUNTERS, IdleCurrent)
    genDef(Tw, KTHREAD_SWITCH_COUNTERS, IdleIdeal)
    genDef(Tw, KTHREAD_SWITCH_COUNTERS, IdleLast)
    genDef(Tw, KTHREAD_SWITCH_COUNTERS, PreemptAny)
    genDef(Tw, KTHREAD_SWITCH_COUNTERS, PreemptCurrent)
    genDef(Tw, KTHREAD_SWITCH_COUNTERS, PreemptLast)
    genDef(Tw, KTHREAD_SWITCH_COUNTERS, SwitchToIdle)

    //
    // Status code definitions
    //

    genCom("Status Code Definitions")

    genVal(STATUS_ACCESS_VIOLATION, STATUS_ACCESS_VIOLATION)
    genVal(STATUS_ARRAY_BOUNDS_EXCEEDED, STATUS_ARRAY_BOUNDS_EXCEEDED)
    genVal(STATUS_BAD_COMPRESSION_BUFFER, STATUS_BAD_COMPRESSION_BUFFER)
    genVal(STATUS_BREAKPOINT, STATUS_BREAKPOINT)
    genVal(STATUS_CALLBACK_POP_STACK, STATUS_CALLBACK_POP_STACK)
    genVal(STATUS_DATATYPE_MISALIGNMENT, STATUS_DATATYPE_MISALIGNMENT)
    genVal(STATUS_FLOAT_DENORMAL_OPERAND, STATUS_FLOAT_DENORMAL_OPERAND)
    genVal(STATUS_FLOAT_DIVIDE_BY_ZERO, STATUS_FLOAT_DIVIDE_BY_ZERO)
    genVal(STATUS_FLOAT_INEXACT_RESULT, STATUS_FLOAT_INEXACT_RESULT)
    genVal(STATUS_FLOAT_INVALID_OPERATION, STATUS_FLOAT_INVALID_OPERATION)
    genVal(STATUS_FLOAT_OVERFLOW, STATUS_FLOAT_OVERFLOW)
    genVal(STATUS_FLOAT_STACK_CHECK, STATUS_FLOAT_STACK_CHECK)
    genVal(STATUS_FLOAT_UNDERFLOW, STATUS_FLOAT_UNDERFLOW)
    genVal(STATUS_FLOAT_MULTIPLE_FAULTS, STATUS_FLOAT_MULTIPLE_FAULTS)
    genVal(STATUS_FLOAT_MULTIPLE_TRAPS, STATUS_FLOAT_MULTIPLE_TRAPS)
    genVal(STATUS_GUARD_PAGE_VIOLATION, STATUS_GUARD_PAGE_VIOLATION)
    genVal(STATUS_ILLEGAL_FLOAT_CONTEXT, STATUS_ILLEGAL_FLOAT_CONTEXT)
    genVal(STATUS_ILLEGAL_INSTRUCTION, STATUS_ILLEGAL_INSTRUCTION)
    genVal(STATUS_INSTRUCTION_MISALIGNMENT, STATUS_INSTRUCTION_MISALIGNMENT)
    genVal(STATUS_INVALID_HANDLE, STATUS_INVALID_HANDLE)
    genVal(STATUS_INVALID_LOCK_SEQUENCE, STATUS_INVALID_LOCK_SEQUENCE)
    genVal(STATUS_INVALID_OWNER, STATUS_INVALID_OWNER)
    genVal(STATUS_INVALID_PARAMETER, STATUS_INVALID_PARAMETER)
    genVal(STATUS_INVALID_PARAMETER_1, STATUS_INVALID_PARAMETER_1)
    genVal(STATUS_INVALID_SYSTEM_SERVICE, STATUS_INVALID_SYSTEM_SERVICE)
    genVal(STATUS_INTEGER_DIVIDE_BY_ZERO, STATUS_INTEGER_DIVIDE_BY_ZERO)
    genVal(STATUS_INTEGER_OVERFLOW, STATUS_INTEGER_OVERFLOW)
    genVal(STATUS_IN_PAGE_ERROR, STATUS_IN_PAGE_ERROR)
    genVal(STATUS_KERNEL_APC, STATUS_KERNEL_APC)
    genVal(STATUS_LONGJUMP, STATUS_LONGJUMP)
    genVal(STATUS_NO_CALLBACK_ACTIVE, STATUS_NO_CALLBACK_ACTIVE)
    genVal(STATUS_NO_EVENT_PAIR, STATUS_NO_EVENT_PAIR)
    genVal(STATUS_PRIVILEGED_INSTRUCTION, STATUS_PRIVILEGED_INSTRUCTION)
    genVal(STATUS_SINGLE_STEP, STATUS_SINGLE_STEP)
    genVal(STATUS_STACK_BUFFER_OVERRUN, STATUS_STACK_BUFFER_OVERRUN)
    genVal(STATUS_STACK_OVERFLOW, STATUS_STACK_OVERFLOW)
    genVal(STATUS_SUCCESS, STATUS_SUCCESS)
    genVal(STATUS_THREAD_IS_TERMINATING, STATUS_THREAD_IS_TERMINATING)
    genVal(STATUS_TIMEOUT, STATUS_TIMEOUT)
    genVal(STATUS_UNWIND, STATUS_UNWIND)
    genVal(STATUS_UNWIND_CONSOLIDATE, STATUS_UNWIND_CONSOLIDATE)
    genVal(STATUS_USER_APC, STATUS_USER_APC)
    genVal(STATUS_WAKE_SYSTEM_DEBUGGER, STATUS_WAKE_SYSTEM_DEBUGGER)

    //
    // Define kernel object structure definitions.
    //
    // APC object structure defintions.
    //

    genCom("APC Object Structure Offset Definitions")

    genDef(Ap, KAPC, Type)
    genDef(Ap, KAPC, Size)
    genDef(Ap, KAPC, Thread)
    genDef(Ap, KAPC, ApcListEntry)
    genDef(Ap, KAPC, KernelRoutine)
    genDef(Ap, KAPC, RundownRoutine)
    genDef(Ap, KAPC, NormalRoutine)
    genDef(Ap, KAPC, NormalContext)
    genDef(Ap, KAPC, SystemArgument1)
    genDef(Ap, KAPC, SystemArgument2)
    genDef(Ap, KAPC, ApcStateIndex)
    genDef(Ap, KAPC, ApcMode)
    genDef(Ap, KAPC, Inserted)
    genVal(ApcObjectLength, sizeof(KAPC))

    //
    // DPC Object structure definitions.
    //

    EnableInc(HAL)

    genCom("DPC object Structure Offset Definitions")

    genDef(Dp, KDPC, Type)
    genDef(Dp, KDPC, Number)
    genDef(Dp, KDPC, Importance)
    genDef(Dp, KDPC, DpcListEntry)
    genDef(Dp, KDPC, DeferredRoutine)
    genDef(Dp, KDPC, DeferredContext)
    genDef(Dp, KDPC, SystemArgument1)
    genDef(Dp, KDPC, SystemArgument2)
    genDef(Dp, KDPC, DpcData)
    genVal(DpcObjectLength, sizeof(KDPC))

    DisableInc(HAL)

    //
    // Device queue object structure definitions.
    //

    genCom("Device Queue Object Structure Offset Definitions")

    genDef(Dv, KDEVICE_QUEUE, Type)
    genDef(Dv, KDEVICE_QUEUE, Size)
    genDef(Dv, KDEVICE_QUEUE, DeviceListHead)
    genAlt(DvSpinLock, KDEVICE_QUEUE, Lock)
    genDef(Dv, KDEVICE_QUEUE, Busy)
    genVal(DeviceQueueObjectLength, sizeof(KDEVICE_QUEUE))

    //
    // Device object entry structure definitions.
    //

    genCom("Device Queue Entry Structure Offset Definitions")

    genDef(De, KDEVICE_QUEUE_ENTRY, DeviceListEntry)
    genDef(De, KDEVICE_QUEUE_ENTRY, SortKey)
    genDef(De, KDEVICE_QUEUE_ENTRY, Inserted)
    genVal(DeviceQueueEntryLength, sizeof(KDEVICE_QUEUE_ENTRY))

    //
    // Event object structure definitions.
    //

    genCom("Event Object Structure Offset Definitions")

    genDef(Ev, DISPATCHER_HEADER, Type)
    genDef(Ev, DISPATCHER_HEADER, Size)
    genDef(Ev, DISPATCHER_HEADER, SignalState)
    genAlt(EvWaitListHead, KEVENT, Header.WaitListHead)
    genVal(EventObjectLength, sizeof(KEVENT))

    //
    // Event pair object structure definitions.
    //

    genCom("Event Pair Object Structure Offset Definitions")

    genDef(Ep, KEVENT_PAIR, Type)
    genDef(Ep, KEVENT_PAIR, Size)
    genDef(Ep, KEVENT_PAIR, EventLow)
    genDef(Ep, KEVENT_PAIR, EventHigh)

    //
    // Interrupt object structure definitions.
    //

#if defined(_AMD64_)

    EnableInc(HAL)

#endif

    genCom("Interrupt Object Structure Offset Definitions")

    genVal(InLevelSensitive, LevelSensitive)
    genVal(InLatched, Latched)

    genSpc()

    genDef(In, KINTERRUPT, Type)
    genDef(In, KINTERRUPT, Size)
    genDef(In, KINTERRUPT, InterruptListEntry)
    genDef(In, KINTERRUPT, ServiceRoutine)
    genDef(In, KINTERRUPT, ServiceContext)
    genDef(In, KINTERRUPT, SpinLock)
    genDef(In, KINTERRUPT, TickCount)
    genDef(In, KINTERRUPT, ActualLock)
    genDef(In, KINTERRUPT, DispatchAddress)
    genDef(In, KINTERRUPT, Vector)
    genDef(In, KINTERRUPT, Irql)
    genDef(In, KINTERRUPT, SynchronizeIrql)
    genDef(In, KINTERRUPT, FloatingSave)
    genDef(In, KINTERRUPT, Connected)
    genDef(In, KINTERRUPT, Number)
    genDef(In, KINTERRUPT, ShareVector)
    genDef(In, KINTERRUPT, Mode)
    genDef(In, KINTERRUPT, ServiceCount)
    genDef(In, KINTERRUPT, DispatchCount)

#if defined(_AMD64_)

    genDef(In, KINTERRUPT, TrapFrame)

#endif

    genDef(In, KINTERRUPT, DispatchCode)
    genVal(InterruptObjectLength, sizeof(KINTERRUPT))

#if defined(_X86_)

    genSpc()

    genVal(NORMAL_DISPATCH_LENGTH, NORMAL_DISPATCH_LENGTH * sizeof(ULONG))
    genVal(DISPATCH_LENGTH, DISPATCH_LENGTH * sizeof(ULONG))

#endif

#if defined(_AMD64_)

    DisableInc(HAL)

#endif

    //
    // Process object structure offset definitions.
    //

    genCom("Process Object Structure Offset Definitions")

    genDef(Pr, DISPATCHER_HEADER, Type)
    genDef(Pr, DISPATCHER_HEADER, Size)
    genDef(Pr, DISPATCHER_HEADER, SignalState)
    genDef(Pr, KPROCESS, ProfileListHead)
    EnableInc(HAL)
    genDef(Pr, KPROCESS, DirectoryTableBase)
    DisableInc(HAL)

#if defined(_X86_)

    EnableInc(HAL)
    genDef(Pr, KPROCESS, LdtDescriptor)
    genDef(Pr, KPROCESS, IopmOffset)
    DisableInc(HAL)
    genDef(Pr, KPROCESS, Int21Descriptor)
    genDef(Pr, KPROCESS, Iopl)
    genDef(Pr, KPROCESS, VdmTrapcHandler)
    genDef(Pr, EPROCESS, VdmObjects)
    genDef(Pr, EPROCESS, Flags)

#endif

#if defined(_AMD64_)

    genDef(Pr, KPROCESS, IopmOffset)

#endif

    genDef(Pr, KPROCESS, ActiveProcessors)
    genDef(Pr, KPROCESS, KernelTime)
    genDef(Pr, KPROCESS, UserTime)
    genDef(Pr, KPROCESS, ReadyListHead)
    genDef(Pr, KPROCESS, SwapListEntry)
    genDef(Pr, KPROCESS, ThreadListHead)
    genDef(Pr, KPROCESS, ProcessLock)
    genDef(Pr, KPROCESS, Affinity)
    genDef(Pr, KPROCESS, ProcessFlags)
    genDef(Pr, KPROCESS, BasePriority)
    genDef(Pr, KPROCESS, QuantumReset)
    genDef(Pr, KPROCESS, State)
    genDef(Pr, KPROCESS, StackCount)
    genVal(KernelProcessObjectLength, sizeof(KPROCESS))
    genVal(ExecutiveProcessObjectLength, sizeof(EPROCESS))

    //
    // Profile object structure offset definitions.
    //

    genCom("Profile Object Structure Offset Definitions")

    genDef(Pf, KPROFILE, Type)
    genDef(Pf, KPROFILE, Size)
    genDef(Pf, KPROFILE, ProfileListEntry)
    genDef(Pf, KPROFILE, Process)
    genDef(Pf, KPROFILE, RangeBase)
    genDef(Pf, KPROFILE, RangeLimit)
    genDef(Pf, KPROFILE, BucketShift)
    genDef(Pf, KPROFILE, Buffer)
    genDef(Pf, KPROFILE, Segment)
    genDef(Pf, KPROFILE, Affinity)
    genDef(Pf, KPROFILE, Source)
    genDef(Pf, KPROFILE, Started)
    genVal(ProfileObjectLength, sizeof(KPROFILE))

    //
    // Queue object structure offset definitions.
    //

    genCom("Queue Object Structure Offset Definitions")

    genDef(Qu, DISPATCHER_HEADER, Type)
    genDef(Qu, DISPATCHER_HEADER, Size)
    genDef(Qu, DISPATCHER_HEADER, SignalState)
    genDef(Qu, KQUEUE, EntryListHead)
    genDef(Qu, KQUEUE, CurrentCount)
    genDef(Qu, KQUEUE, MaximumCount)
    genDef(Qu, KQUEUE, ThreadListHead)
    genVal(QueueObjectLength, sizeof(KQUEUE))

    //
    // Thread object structure offset definitions
    //

    genCom("Thread Object Structure Offset Definitions")

    genDef(Ee, EEVENT_PAIR, KernelEventPair)
    genDef(Et, ETHREAD, Cid)

#if defined (PERF_DATA)

    genDef(Et, ETHREAD, PerformanceCountLow)
    genDef(Et, ETHREAD, PerformanceCountHigh)

#endif

    genSpc()

    genDef(Th, DISPATCHER_HEADER, Type)
    genDef(Th, DISPATCHER_HEADER, NpxIrql)
    genDef(Th, DISPATCHER_HEADER, Size)

    EnableInc(HAL)

    genDef(Th, DISPATCHER_HEADER, DebugActive)

    DisableInc(HAL)

    genDef(Th, DISPATCHER_HEADER, SignalState)
    genDef(Th, KTHREAD, MutantListHead)
    genDef(Th, KTHREAD, InitialStack)

    EnableInc(HAL)

    genDef(Th, KTHREAD, StackLimit)

    DisableInc(HAL)

    genDef(Th, KTHREAD, KernelStack)
    genDef(Th, KTHREAD, ThreadLock)

    EnableInc(HAL)

    genDef(Th, KTHREAD, ApcState)

    DisableInc(HAL)

    genDef(Th, KTHREAD, ApcQueueable)
    genDef(Th, KTHREAD, NextProcessor)
    genDef(Th, KTHREAD, DeferredProcessor)
    genDef(Th, KTHREAD, AdjustReason)
    genDef(Th, KTHREAD, AdjustIncrement)
    genDef(Th, KTHREAD, ApcQueueLock)

#if !defined(_AMD64_)

    genDef(Th, KTHREAD, ContextSwitches)
    genDef(Th, KTHREAD, State)
    genDef(Th, KTHREAD, NpxState)
    genDef(Th, KTHREAD, WaitIrql)
    genDef(Th, KTHREAD, WaitMode)

#endif

    genDef(Th, KTHREAD, WaitStatus)
    genDef(Th, KTHREAD, WaitBlockList)
    genDef(Th, KTHREAD, GateObject)
    genDef(Th, KTHREAD, Alertable)
    genDef(Th, KTHREAD, WaitNext)
    genDef(Th, KTHREAD, WaitReason)
    genDef(Th, KTHREAD, Priority)
    genDef(Th, KTHREAD, EnableStackSwap)
    genDef(Th, KTHREAD, SwapBusy)
    genDef(Th, KTHREAD, Alerted)
    genDef(Th, KTHREAD, WaitListEntry)
    genDef(Th, KTHREAD, SwapListEntry)
    genDef(Th, KTHREAD, Queue)

#if !defined(_AMD64_)

    genDef(Th, KTHREAD, WaitTime)
    genDef(Th, KTHREAD, CombinedApcDisable)
    genDef(Th, KTHREAD, KernelApcDisable)
    genDef(Th, KTHREAD, SpecialApcDisable)

#endif

    genDef(Th, KTHREAD, Teb)
    genDef(Th, KTHREAD, Timer)
    genDef(Th, KTHREAD, ThreadFlags)
    genDef(Th, KTHREAD, WaitBlock)
    genDef(Th, KTHREAD, SystemAffinityActive)
    genDef(Th, KTHREAD, PreviousMode)
    genDef(Th, KTHREAD, ResourceIndex)
    genDef(Th, KTHREAD, LargeStack)

#if defined(_AMD64_)

    genDef(Th, KTHREAD, ContextSwitches)
    genDef(Th, KTHREAD, State)
    genDef(Th, KTHREAD, NpxState)
    genDef(Th, KTHREAD, WaitIrql)
    genDef(Th, KTHREAD, WaitMode)
    genDef(Th, KTHREAD, WaitTime)
    genDef(Th, KTHREAD, CombinedApcDisable)
    genDef(Th, KTHREAD, KernelApcDisable)
    genDef(Th, KTHREAD, SpecialApcDisable)

#endif

    genDef(Th, KTHREAD, QueueListEntry)
    genDef(Th, KTHREAD, TrapFrame)
    genDef(Th, KTHREAD, CallbackStack)
    genDef(Th, KTHREAD, ServiceTable)

#if defined(_AMD64_)

    genDef(Th, KTHREAD, KernelLimit)

#endif

    genDef(Th, KTHREAD, ApcStateIndex)
    genDef(Th, KTHREAD, IdealProcessor)
    genDef(Th, KTHREAD, Preempted)
    genDef(Th, KTHREAD, ProcessReadyQueue)

#if defined(_AMD64_)

    genDef(Th, KTHREAD, Win32kTable)
    genDef(Th, KTHREAD, Win32kLimit)

#endif

    genDef(Th, KTHREAD, KernelStackResident)
    genDef(Th, KTHREAD, BasePriority)
    genDef(Th, KTHREAD, PriorityDecrement)
    genDef(Th, KTHREAD, Saturation)
    genDef(Th, KTHREAD, UserAffinity)
    genDef(Th, KTHREAD, Process)
    genDef(Th, KTHREAD, Affinity)
    genDef(Th, KTHREAD, ApcStatePointer)
    genDef(Th, KTHREAD, SavedApcState)
    genDef(Th, KTHREAD, FreezeCount)
    genDef(Th, KTHREAD, SuspendCount)
    genDef(Th, KTHREAD, UserIdealProcessor)
    genDef(Th, KTHREAD, CalloutActive)

#if defined(_AMD64_)

    genDef(Th, KTHREAD, CodePatchInProgress)

#elif defined(_X86_)

    genDef(Th, KTHREAD, Iopl)

#endif

    genDef(Th, KTHREAD, Win32Thread)
    genDef(Th, KTHREAD, StackBase)
    genDef(Th, KTHREAD, SuspendApc)
    genDef(Th, KTHREAD, Quantum)
    genDef(Th, KTHREAD, QuantumReset)
    genDef(Th, KTHREAD, KernelTime)
    genDef(Th, KTHREAD, TlsArray)
    genDef(Th, KTHREAD, BBTData)
    genDef(Th, KTHREAD, PowerState)
    genDef(Th, KTHREAD, UserTime)
    genDef(Th, KTHREAD, SuspendSemaphore)
    genDef(Th, KTHREAD, SListFaultCount)
    genDef(Th, KTHREAD, ThreadListEntry)
    genDef(Th, KTHREAD, SListFaultAddress)
    genSpc()
    genVal(KI_SLIST_FAULT_COUNT_MAXIMUM, KI_SLIST_FAULT_COUNT_MAXIMUM)
    genVal(KTHREAD_AUTO_ALIGNMENT_BIT, KTHREAD_AUTO_ALIGNMENT_BIT)
    genSpc()
    genVal(KernelThreadObjectLength, sizeof(KTHREAD))
    genVal(ExecutiveThreadObjectLength, sizeof(ETHREAD))
    genSpc()

#if defined(_X86_)

    genVal(NPX_STATE_NOT_LOADED, NPX_STATE_NOT_LOADED)
    genVal(NPX_STATE_LOADED, NPX_STATE_LOADED)

#endif

    //
    // Timer object structure offset definitions
    //

    genCom("Timer object Structure Offset Definitions")

    genDef(Ti, DISPATCHER_HEADER, Type)
    genDef(Ti, DISPATCHER_HEADER, Size)
    genDef(Ti, DISPATCHER_HEADER, Inserted)
    genDef(Ti, DISPATCHER_HEADER, SignalState)
    genDef(Ti, KTIMER, DueTime)
    genDef(Ti, KTIMER, TimerListEntry)
    genDef(Ti, KTIMER, Dpc)
    genDef(Ti, KTIMER, Period)
    genVal(TimerObjectLength, sizeof(KTIMER))

    genSpc()

    //
    // Timer table entry offset definitions.
    //

    genCom("Timer Table Entry Structure Offset Definitions")

    genDef(Tt, KTIMER_TABLE_ENTRY, Entry)
    genDef(Tt, KTIMER_TABLE_ENTRY, Time)
    genVal(TIMER_ENTRY_SIZE, sizeof(KTIMER_TABLE_ENTRY))
    genVal(TIMER_TABLE_SIZE, TIMER_TABLE_SIZE)

    //
    // Wait block structure offset definitions
    //

    genCom("Wait Block Structure Offset Definitions")

    genDef(Wb, KWAIT_BLOCK, WaitListEntry)
    genDef(Wb, KWAIT_BLOCK, Thread)
    genDef(Wb, KWAIT_BLOCK, Object)
    genDef(Wb, KWAIT_BLOCK, NextWaitBlock)
    genDef(Wb, KWAIT_BLOCK, WaitKey)
    genDef(Wb, KWAIT_BLOCK, WaitType)

    //
    // Fiber structure offset definitions.
    //

    genCom("Fiber Structure Offset Definitions")

    genDef(Fb, FIBER, FiberData)
    genDef(Fb, FIBER, ExceptionList)
    genDef(Fb, FIBER, StackBase)
    genDef(Fb, FIBER, StackLimit)
    genDef(Fb, FIBER, DeallocationStack)
    genDef(Fb, FIBER, FiberContext)
    genDef(Fb, FIBER, Wx86Tib)
    genDef(Fb, FIBER, ActivationContextStackPointer)
    genDef(Fb, FIBER, FlsData)
    genDef(Fb, FIBER, GuaranteedStackBytes)

    //
    // Process environment block structure offset definitions.
    //

    genCom("Process Environment Block Structure Offset Definitions")

    genDef(Pe, PEB, KernelCallbackTable)

    genVal(ProcessEnvironmentBlockLength, sizeof(PEB))

    //
    // Define System Service Descriptor Table structures.
    //

    genCom("System Service Descriptor Table Structure Definitions")

    genVal(NUMBER_SERVICE_TABLES, NUMBER_SERVICE_TABLES)
    genVal(SERVICE_NUMBER_MASK, SERVICE_NUMBER_MASK)
    genVal(SERVICE_TABLE_SHIFT, SERVICE_TABLE_SHIFT)
    genVal(SERVICE_TABLE_MASK, SERVICE_TABLE_MASK)
    genVal(SERVICE_TABLE_TEST, SERVICE_TABLE_TEST)

    genSpc()

#if defined(_AMD64_)

    genVal(ThBase, 0)
    genVal(ThLimit, 8)

#endif

    genDef(Sd, KSERVICE_TABLE_DESCRIPTOR, Base)
    genDef(Sd, KSERVICE_TABLE_DESCRIPTOR, Count)
    genDef(Sd, KSERVICE_TABLE_DESCRIPTOR, Limit)
    genDef(Sd, KSERVICE_TABLE_DESCRIPTOR, Number)
    genVal(SdLength, sizeof(KSERVICE_TABLE_DESCRIPTOR))

    //
    // Common TEB structure offset definitions
    //

    genCom("Thread Environment Block Structure Offset Definitions")

#if defined(_WIN64)

    genAlt(TeCmTeb, NT_TIB, ExceptionList)

#endif

#if defined(_X86_)

    genDef(Te, NT_TIB, ExceptionList)

#endif

    genDef(Te, NT_TIB, StackBase)
    genDef(Te, NT_TIB, StackLimit)
    genDef(Te, NT_TIB, FiberData)
    genDef(Te, NT_TIB, Self)
    genDef(Te, TEB, EnvironmentPointer)
    genDef(Te, TEB, ClientId)
    genDef(Te, TEB, ActiveRpcHandle)
    genDef(Te, TEB, ThreadLocalStoragePointer)
    genDef(Te, TEB, CountOfOwnedCriticalSections)
    genAlt(TePeb, TEB, ProcessEnvironmentBlock)
    genDef(Te, TEB, CsrClientThread)
    genDef(Te, TEB, WOW32Reserved)
    genAlt(TeSoftFpcr, TEB, FpSoftwareStatusRegister)
    genDef(Te, TEB, ExceptionCode)
    genDef(Te, TEB, ActivationContextStackPointer)
    genDef(Te, TEB, GdiClientPID)
    genDef(Te, TEB, GdiClientTID)
    genDef(Te, TEB, GdiThreadLocalInfo)
    genDef(Te, TEB, glDispatchTable)
    genDef(Te, TEB, glReserved1)
    genDef(Te, TEB, glReserved2)
    genDef(Te, TEB, glSectionInfo)
    genDef(Te, TEB, glSection)
    genDef(Te, TEB, glTable)
    genDef(Te, TEB, glCurrentRC)
    genDef(Te, TEB, glContext)
    genDef(Te, TEB, DeallocationStack)
    genDef(Te, TEB, TlsSlots)
    genDef(Te, TEB, TlsExpansionSlots)
    genDef(Te, TEB, LastErrorValue)
    genDef(Te, TEB, Vdm)
    genDef(Te, TEB, Instrumentation)
    genDef(Te, TEB, GdiBatchCount)
    genDef(Te, TEB, GuaranteedStackBytes)
    genDef(Te, TEB, SafeThunkCall)
    genDef(Te, TEB, FlsData)

    genVal(ThreadEnvironmentBlockLength, sizeof(TEB))

    genVal(CmThreadEnvironmentBlockOffset, ROUND_TO_PAGES(sizeof(TEB)))

    genNam(TLS_MINIMUM_AVAILABLE)
    genNam(TLS_EXPANSION_SLOTS)

    //
    // Lock Queue structure definitions.
    //

#if defined(_X86_)

    EnableInc(HAL)

#endif

    genCom("Lock Queue Structure Offset Definitions")

    genNam(LOCK_QUEUE_WAIT)
    genNam(LOCK_QUEUE_OWNER)
    genVal(LOCK_QUEUE_HEADER_SIZE, sizeof(KSPIN_LOCK_QUEUE))
    genSpc()

    genNam(LockQueueDispatcherLock)
    genSpc()

    genDef(Lq, KSPIN_LOCK_QUEUE, Next)
    genDef(Lq, KSPIN_LOCK_QUEUE, Lock)
    genSpc()

    genAlt(LqhNext, KLOCK_QUEUE_HANDLE, LockQueue.Next)
    genAlt(LqhLock, KLOCK_QUEUE_HANDLE, LockQueue.Lock)
    genAlt(LqhOldIrql, KLOCK_QUEUE_HANDLE, OldIrql)

#if defined(_X86_)

    DisableInc(HAL)

#endif

    genCom("Performance Definitions")

    genVal(PERF_CONTEXTSWAP_OFFSET,
           PERF_GET_MASK_INDEX(PERF_CONTEXT_SWITCH) * sizeof(ULONG))

    genVal(PERF_CONTEXTSWAP_FLAG, PERF_GET_MASK_GROUP(PERF_CONTEXT_SWITCH))
    genVal(PERF_INTERRUPT_OFFSET,
           PERF_GET_MASK_INDEX(PERF_INTERRUPT) * sizeof(ULONG))

    genVal(PERF_INTERRUPT_FLAG, PERF_GET_MASK_GROUP(PERF_INTERRUPT))
    genVal(PERF_PROFILE_OFFSET,
           PERF_GET_MASK_INDEX(PERF_PROFILE) * sizeof(ULONG))

    genVal(PERF_PROFILE_FLAG, PERF_GET_MASK_GROUP(PERF_PROFILE))

