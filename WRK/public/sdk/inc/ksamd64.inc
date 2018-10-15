;
; Copyright (c) Microsoft Corporation. All rights reserved.
;
; You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
; If you do not agree to the terms, do not use the code.
;

include kxamd64.inc

;
;  Pointer size in bytes
;

SizeofPointer equ 00008H

;
;  Process State Enumerated Type Values
;

ProcessInMemory equ 00000H
ProcessOutOfMemory equ 00001H
ProcessInTransition equ 00002H

;
;  Thread State Enumerated Type Values
;

Initialized equ 00000H
Ready equ 00001H
Running equ 00002H
Standby equ 00003H
Terminated equ 00004H
Waiting equ 00005H

;
;  Wait Reason and Wait Type Enumerated Type Values
;

WrExecutive equ 00000H
WrMutex equ 0001DH
WrDispatchInt equ 0001FH
WrQuantumEnd equ 0001EH
WrEventPair equ 0000EH
WaitAny equ 00001H
WaitAll equ 00000H

;
;  Apc State Structure Offset Definitions
;

AsApcListHead equ 00000H
AsProcess equ 00020H
AsKernelApcInProgress equ 00028H
AsKernelApcPending equ 00029H
AsUserApcPending equ 0002AH

;
;  Bugcheck Code Definitions
;

APC_INDEX_MISMATCH equ 00001H
ATTEMPTED_SWITCH_FROM_DPC equ 000B8H
DATA_BUS_ERROR equ 0002EH
DATA_COHERENCY_EXCEPTION equ 00055H
HAL1_INITIALIZATION_FAILED equ 00061H
INSTRUCTION_BUS_ERROR equ 0002FH
INSTRUCTION_COHERENCY_EXCEPTION equ 00056H
INTERRUPT_EXCEPTION_NOT_HANDLED equ 0003DH
INTERRUPT_UNWIND_ATTEMPTED equ 0003CH
INVALID_AFFINITY_SET equ 00003H
INVALID_DATA_ACCESS_TRAP equ 00004H
IRQL_GT_ZERO_AT_SYSTEM_SERVICE equ 0004AH
IRQL_NOT_LESS_OR_EQUAL equ 0000AH
KMODE_EXCEPTION_NOT_HANDLED equ 0001EH
NMI_HARDWARE_FAILURE equ 00080H
NO_USER_MODE_CONTEXT equ 0000EH
PAGE_FAULT_WITH_INTERRUPTS_OFF equ 00049H
PANIC_STACK_SWITCH equ 0002BH
SPIN_LOCK_INIT_FAILURE equ 00081H
SYSTEM_EXIT_OWNED_MUTEX equ 00039H
SYSTEM_SERVICE_EXCEPTION equ 0003BH
SYSTEM_UNWIND_PREVIOUS_USER equ 0003AH
TRAP_CAUSE_UNKNOWN equ 00012H
UNEXPECTED_KERNEL_MODE_TRAP equ 0007FH
HARDWARE_INTERRUPT_STORM equ 000F2H
RECURSIVE_MACHINE_CHECK equ 000FBH
RECURSIVE_NMI equ 00111H

;
;  Breakpoint type definitions
;

DBG_STATUS_CONTROL_C equ 00001H
STATUS_ASSERTION_FAILURE equ 0C0000420H

;
;  Client Id Structure Offset Definitions
;

CidUniqueProcess equ 00000H
CidUniqueThread equ 00008H

;
;  Critical Section Structure Offset Definitions
;

CsDebugInfo equ 00000H
CsLockCount equ 00008H
CsRecursionCount equ 0000CH
CsOwningThread equ 00010H
CsLockSemaphore equ 00018H
CsSpinCount equ 00020H

;
;  Critical Section Debug Information Structure Offset Definitions
;

CsType equ 00000H
CsCreatorBackTraceIndex equ 00002H
CsCriticalSection equ 00008H
CsProcessLocksList equ 00010H
CsEntryCount equ 00020H
CsContentionCount equ 00024H

;
;  Exception Record Offset, Flag, and Enumerated Type Definitions
;

EXCEPTION_NONCONTINUABLE equ 00001H
EXCEPTION_UNWINDING equ 00002H
EXCEPTION_EXIT_UNWIND equ 00004H
EXCEPTION_STACK_INVALID equ 00008H
EXCEPTION_NESTED_CALL equ 00010H
EXCEPTION_TARGET_UNWIND equ 00020H
EXCEPTION_COLLIDED_UNWIND equ 00040H
EXCEPTION_UNWIND equ 00066H
EXCEPTION_EXECUTE_HANDLER equ 00001H
EXCEPTION_CONTINUE_SEARCH equ 00000H
EXCEPTION_CONTINUE_EXECUTION equ 0FFFFFFFFH

ExceptionContinueExecution equ 00000H
ExceptionContinueSearch equ 00001H
ExceptionNestedException equ 00002H
ExceptionCollidedUnwind equ 00003H

ErExceptionCode equ 00000H
ErExceptionFlags equ 00004H
ErExceptionRecord equ 00008H
ErExceptionAddress equ 00010H
ErNumberParameters equ 00018H
ErExceptionInformation equ 00020H
ExceptionRecordLength equ 000A0H

;
;  Fast Mutex Structure Offset Definitions
;

FmCount equ 00000H
FmOwner equ 00008H
FmContention equ 00010H
FmGate equ 00018H
FmOldIrql equ 00030H

;
;  Interrupt Priority Request Level Definitions
;

PASSIVE_LEVEL equ 00000H
APC_LEVEL equ 00001H
DISPATCH_LEVEL equ 00002H
CLOCK_LEVEL equ 0000DH
IPI_LEVEL equ 0000EH
POWER_LEVEL equ 0000EH
PROFILE_LEVEL equ 0000FH
HIGH_LEVEL equ 0000FH
ifdef NT_UP
SYNCH_LEVEL equ 00002H
else
SYNCH_LEVEL equ 0000CH
endif

;
;  Large Integer Structure Offset Definitions
;

LiLowPart equ 00000H
LiHighPart equ 00004H

;
;  List Entry Structure Offset Definitions
;

LsFlink equ 00000H
LsBlink equ 00008H

;
;  String Structure Offset Definitions
;

StrLength equ 00000H
StrMaximumLength equ 00002H
StrBuffer equ 00008H

;
;  User Shared Data Structure Offset Definitions
;

UsTickCountMultiplier equ 00004H
UsInterruptTime equ 00008H
UsSystemTime equ 00014H
UsTimeZoneBias equ 00020H
UsImageNumberLow equ 0002CH
UsImageNumberHigh equ 0002EH
UsNtSystemRoot equ 00030H
UsMaxStackTraceDepth equ 00238H
UsCryptoExponent equ 0023CH
UsTimeZoneId equ 00240H
UsLargePageMinimum equ 00244H
UsReserved2 equ 00248H
UsNtProductType equ 00264H
UsProductTypeIsValid equ 00268H
UsNtMajorVersion equ 0026CH
UsNtMinorVersion equ 00270H
UsProcessorFeatures equ 00274H
UsReserved1 equ 002B4H
UsReserved3 equ 002B8H
UsTimeSlip equ 002BCH
UsAlternativeArchitecture equ 002C0H
UsSystemExpirationDate equ 002C8H
UsSuiteMask equ 002D0H
UsKdDebuggerEnabled equ 002D4H
UsActiveConsoleId equ 002D8H
UsDismountCount equ 002DCH
UsComPlusPackage equ 002E0H
UsLastSystemRITEventTickCount equ 002E4H
UsNumberOfPhysicalPages equ 002E8H
UsSafeBootMode equ 002ECH
UsTraceLogging equ 002F0H
UsTestRetInstruction equ 002F8H
UsSystemCall equ 00300H
UsSystemCallReturn equ 00304H
UsSystemCallPad equ 00308H
UsTickCount equ 00320H
UsTickCountQuad equ 00320H
UsWow64SharedInformation equ 00334H

;
;  Time Structure Offset Definitions
;

TmLowTime equ 00000H
TmHighTime equ 00004H

;
;  Thread Switch Counter Offset Definitions
;

TwFindAny equ 00000H
TwFindIdeal equ 00004H
TwFindLast equ 00008H
TwIdleAny equ 0000CH
TwIdleCurrent equ 00010H
TwIdleIdeal equ 00014H
TwIdleLast equ 00018H
TwPreemptAny equ 0001CH
TwPreemptCurrent equ 00020H
TwPreemptLast equ 00024H
TwSwitchToIdle equ 00028H

;
;  Status Code Definitions
;

STATUS_ACCESS_VIOLATION equ 0C0000005H
STATUS_ARRAY_BOUNDS_EXCEEDED equ 0C000008CH
STATUS_BAD_COMPRESSION_BUFFER equ 0C0000242H
STATUS_BREAKPOINT equ 080000003H
STATUS_CALLBACK_POP_STACK equ 0C0000423H
STATUS_DATATYPE_MISALIGNMENT equ 080000002H
STATUS_FLOAT_DENORMAL_OPERAND equ 0C000008DH
STATUS_FLOAT_DIVIDE_BY_ZERO equ 0C000008EH
STATUS_FLOAT_INEXACT_RESULT equ 0C000008FH
STATUS_FLOAT_INVALID_OPERATION equ 0C0000090H
STATUS_FLOAT_OVERFLOW equ 0C0000091H
STATUS_FLOAT_STACK_CHECK equ 0C0000092H
STATUS_FLOAT_UNDERFLOW equ 0C0000093H
STATUS_FLOAT_MULTIPLE_FAULTS equ 0C00002B4H
STATUS_FLOAT_MULTIPLE_TRAPS equ 0C00002B5H
STATUS_GUARD_PAGE_VIOLATION equ 080000001H
STATUS_ILLEGAL_FLOAT_CONTEXT equ 0C000014AH
STATUS_ILLEGAL_INSTRUCTION equ 0C000001DH
STATUS_INSTRUCTION_MISALIGNMENT equ 0C00000AAH
STATUS_INVALID_HANDLE equ 0C0000008H
STATUS_INVALID_LOCK_SEQUENCE equ 0C000001EH
STATUS_INVALID_OWNER equ 0C000005AH
STATUS_INVALID_PARAMETER equ 0C000000DH
STATUS_INVALID_PARAMETER_1 equ 0C00000EFH
STATUS_INVALID_SYSTEM_SERVICE equ 0C000001CH
STATUS_INTEGER_DIVIDE_BY_ZERO equ 0C0000094H
STATUS_INTEGER_OVERFLOW equ 0C0000095H
STATUS_IN_PAGE_ERROR equ 0C0000006H
STATUS_KERNEL_APC equ 00100H
STATUS_LONGJUMP equ 080000026H
STATUS_NO_CALLBACK_ACTIVE equ 0C0000258H
STATUS_NO_EVENT_PAIR equ 0C000014EH
STATUS_PRIVILEGED_INSTRUCTION equ 0C0000096H
STATUS_SINGLE_STEP equ 080000004H
STATUS_STACK_BUFFER_OVERRUN equ 0C0000409H
STATUS_STACK_OVERFLOW equ 0C00000FDH
STATUS_SUCCESS equ 00000H
STATUS_THREAD_IS_TERMINATING equ 0C000004BH
STATUS_TIMEOUT equ 00102H
STATUS_UNWIND equ 0C0000027H
STATUS_UNWIND_CONSOLIDATE equ 080000029H
STATUS_USER_APC equ 000C0H
STATUS_WAKE_SYSTEM_DEBUGGER equ 080000007H

;
;  APC Object Structure Offset Definitions
;

ApType equ 00000H
ApSize equ 00002H
ApThread equ 00008H
ApApcListEntry equ 00010H
ApKernelRoutine equ 00020H
ApRundownRoutine equ 00028H
ApNormalRoutine equ 00030H
ApNormalContext equ 00038H
ApSystemArgument1 equ 00040H
ApSystemArgument2 equ 00048H
ApApcStateIndex equ 00050H
ApApcMode equ 00051H
ApInserted equ 00052H
ApcObjectLength equ 00058H

;
;  DPC object Structure Offset Definitions
;

DpType equ 00000H
DpNumber equ 00002H
DpImportance equ 00001H
DpDpcListEntry equ 00008H
DpDeferredRoutine equ 00018H
DpDeferredContext equ 00020H
DpSystemArgument1 equ 00028H
DpSystemArgument2 equ 00030H
DpDpcData equ 00038H
DpcObjectLength equ 00040H

;
;  Device Queue Object Structure Offset Definitions
;

DvType equ 00000H
DvSize equ 00002H
DvDeviceListHead equ 00008H
DvSpinLock equ 00018H
DvBusy equ 00020H
DeviceQueueObjectLength equ 00028H

;
;  Device Queue Entry Structure Offset Definitions
;

DeDeviceListEntry equ 00000H
DeSortKey equ 00010H
DeInserted equ 00014H
DeviceQueueEntryLength equ 00018H

;
;  Event Object Structure Offset Definitions
;

EvType equ 00000H
EvSize equ 00002H
EvSignalState equ 00004H
EvWaitListHead equ 00008H
EventObjectLength equ 00018H

;
;  Event Pair Object Structure Offset Definitions
;

EpType equ 00000H
EpSize equ 00002H
EpEventLow equ 00008H
EpEventHigh equ 00020H

;
;  Interrupt Object Structure Offset Definitions
;

InLevelSensitive equ 00000H
InLatched equ 00001H

InType equ 00000H
InSize equ 00002H
InInterruptListEntry equ 00008H
InServiceRoutine equ 00018H
InServiceContext equ 00020H
InSpinLock equ 00028H
InTickCount equ 00030H
InActualLock equ 00038H
InDispatchAddress equ 00040H
InVector equ 00048H
InIrql equ 0004CH
InSynchronizeIrql equ 0004DH
InFloatingSave equ 0004EH
InConnected equ 0004FH
InNumber equ 00050H
InShareVector equ 00051H
InMode equ 00054H
InServiceCount equ 00058H
InDispatchCount equ 0005CH
InTrapFrame equ 00060H
InDispatchCode equ 00070H
InterruptObjectLength equ 00080H

;
;  Process Object Structure Offset Definitions
;

PrType equ 00000H
PrSize equ 00002H
PrSignalState equ 00004H
PrProfileListHead equ 00018H
PrDirectoryTableBase equ 00028H
PrIopmOffset equ 00038H
PrActiveProcessors equ 00040H
PrKernelTime equ 00048H
PrUserTime equ 0004CH
PrReadyListHead equ 00050H
PrSwapListEntry equ 00060H
PrThreadListHead equ 00070H
PrProcessLock equ 00080H
PrAffinity equ 00088H
PrProcessFlags equ 00090H
PrBasePriority equ 00094H
PrQuantumReset equ 00095H
PrState equ 00096H
PrStackCount equ 000A0H
KernelProcessObjectLength equ 000B8H
ExecutiveProcessObjectLength equ 003E0H

;
;  Profile Object Structure Offset Definitions
;

PfType equ 00000H
PfSize equ 00002H
PfProfileListEntry equ 00008H
PfProcess equ 00018H
PfRangeBase equ 00020H
PfRangeLimit equ 00028H
PfBucketShift equ 00030H
PfBuffer equ 00038H
PfSegment equ 00040H
PfAffinity equ 00048H
PfSource equ 00050H
PfStarted equ 00052H
ProfileObjectLength equ 00058H

;
;  Queue Object Structure Offset Definitions
;

QuType equ 00000H
QuSize equ 00002H
QuSignalState equ 00004H
QuEntryListHead equ 00018H
QuCurrentCount equ 00028H
QuMaximumCount equ 0002CH
QuThreadListHead equ 00030H
QueueObjectLength equ 00040H

;
;  Thread Object Structure Offset Definitions
;

EeKernelEventPair equ 00000H
EtCid equ 00370H

ThType equ 00000H
ThNpxIrql equ 00001H
ThSize equ 00002H
ThDebugActive equ 00003H
ThSignalState equ 00004H
ThMutantListHead equ 00018H
ThInitialStack equ 00028H
ThStackLimit equ 00030H
ThKernelStack equ 00038H
ThThreadLock equ 00040H
ThApcState equ 00048H
ThApcQueueable equ 00073H
ThNextProcessor equ 00074H
ThDeferredProcessor equ 00075H
ThAdjustReason equ 00076H
ThAdjustIncrement equ 00077H
ThApcQueueLock equ 00078H
ThWaitStatus equ 00080H
ThWaitBlockList equ 00088H
ThGateObject equ 00088H
ThAlertable equ 00090H
ThWaitNext equ 00091H
ThWaitReason equ 00092H
ThPriority equ 00093H
ThEnableStackSwap equ 00094H
ThSwapBusy equ 00095H
ThAlerted equ 00096H
ThWaitListEntry equ 00098H
ThSwapListEntry equ 00098H
ThQueue equ 000A8H
ThTeb equ 000B0H
ThTimer equ 000B8H
ThThreadFlags equ 000F4H
ThWaitBlock equ 000F8H
ThSystemAffinityActive equ 00123H
ThPreviousMode equ 00153H
ThResourceIndex equ 00183H
ThLargeStack equ 001B3H
ThContextSwitches equ 00124H
ThState equ 00154H
ThNpxState equ 00155H
ThWaitIrql equ 00156H
ThWaitMode equ 00157H
ThWaitTime equ 00184H
ThCombinedApcDisable equ 001B4H
ThKernelApcDisable equ 001B4H
ThSpecialApcDisable equ 001B6H
ThQueueListEntry equ 001B8H
ThTrapFrame equ 001C8H
ThCallbackStack equ 001D0H
ThServiceTable equ 001D8H
ThKernelLimit equ 001E0H
ThApcStateIndex equ 001E4H
ThIdealProcessor equ 001E5H
ThPreempted equ 001E6H
ThProcessReadyQueue equ 001E7H
ThWin32kTable equ 001E8H
ThWin32kLimit equ 001F0H
ThKernelStackResident equ 001F4H
ThBasePriority equ 001F5H
ThPriorityDecrement equ 001F6H
ThSaturation equ 001F7H
ThUserAffinity equ 001F8H
ThProcess equ 00200H
ThAffinity equ 00208H
ThApcStatePointer equ 00210H
ThSavedApcState equ 00220H
ThFreezeCount equ 0024BH
ThSuspendCount equ 0024CH
ThUserIdealProcessor equ 0024DH
ThCalloutActive equ 0024EH
ThCodePatchInProgress equ 0024FH
ThWin32Thread equ 00250H
ThStackBase equ 00258H
ThSuspendApc equ 00260H
ThQuantum equ 00261H
ThQuantumReset equ 00263H
ThKernelTime equ 00264H
ThTlsArray equ 002A0H
ThBBTData equ 002A8H
ThPowerState equ 002B3H
ThUserTime equ 002B4H
ThSuspendSemaphore equ 002B8H
ThSListFaultCount equ 002D4H
ThThreadListEntry equ 002D8H
ThSListFaultAddress equ 002E8H

KI_SLIST_FAULT_COUNT_MAXIMUM equ 00400H
KTHREAD_AUTO_ALIGNMENT_BIT equ 00000H

KernelThreadObjectLength equ 00320H
ExecutiveThreadObjectLength equ 00428H


;
;  Timer object Structure Offset Definitions
;

TiType equ 00000H
TiSize equ 00002H
TiInserted equ 00003H
TiSignalState equ 00004H
TiDueTime equ 00018H
TiTimerListEntry equ 00020H
TiDpc equ 00030H
TiPeriod equ 00038H
TimerObjectLength equ 00040H


;
;  Timer Table Entry Structure Offset Definitions
;

TtEntry equ 00000H
TtTime equ 00010H
TIMER_ENTRY_SIZE equ 00018H
TIMER_TABLE_SIZE equ 00200H

;
;  Wait Block Structure Offset Definitions
;

WbWaitListEntry equ 00000H
WbThread equ 00010H
WbObject equ 00018H
WbNextWaitBlock equ 00020H
WbWaitKey equ 00028H
WbWaitType equ 0002AH

;
;  Fiber Structure Offset Definitions
;

FbFiberData equ 00000H
FbExceptionList equ 00008H
FbStackBase equ 00010H
FbStackLimit equ 00018H
FbDeallocationStack equ 00020H
FbFiberContext equ 00030H
FbWx86Tib equ 00500H
FbActivationContextStackPointer equ 00508H
FbFlsData equ 00510H
FbGuaranteedStackBytes equ 00518H

;
;  Process Environment Block Structure Offset Definitions
;

PeKernelCallbackTable equ 00058H
ProcessEnvironmentBlockLength equ 00358H

;
;  System Service Descriptor Table Structure Definitions
;

NUMBER_SERVICE_TABLES equ 00002H
SERVICE_NUMBER_MASK equ 00FFFH
SERVICE_TABLE_SHIFT equ 00008H
SERVICE_TABLE_MASK equ 00010H
SERVICE_TABLE_TEST equ 00010H

ThBase equ 00000H
ThLimit equ 00008H
SdBase equ 00000H
SdCount equ 00008H
SdLimit equ 00010H
SdNumber equ 00018H
SdLength equ 00020H

;
;  Thread Environment Block Structure Offset Definitions
;

TeCmTeb equ 00000H
TeStackBase equ 00008H
TeStackLimit equ 00010H
TeFiberData equ 00020H
TeSelf equ 00030H
TeEnvironmentPointer equ 00038H
TeClientId equ 00040H
TeActiveRpcHandle equ 00050H
TeThreadLocalStoragePointer equ 00058H
TeCountOfOwnedCriticalSections equ 0006CH
TePeb equ 00060H
TeCsrClientThread equ 00070H
TeWOW32Reserved equ 00100H
TeSoftFpcr equ 0010CH
TeExceptionCode equ 002C0H
TeActivationContextStackPointer equ 002C8H
TeGdiClientPID equ 007F0H
TeGdiClientTID equ 007F4H
TeGdiThreadLocalInfo equ 007F8H
TeglDispatchTable equ 009F0H
TeglReserved1 equ 01138H
TeglReserved2 equ 01220H
TeglSectionInfo equ 01228H
TeglSection equ 01230H
TeglTable equ 01238H
TeglCurrentRC equ 01240H
TeglContext equ 01248H
TeDeallocationStack equ 01478H
TeTlsSlots equ 01480H
TeTlsExpansionSlots equ 01780H
TeLastErrorValue equ 00068H
TeVdm equ 01690H
TeInstrumentation equ 016B8H
TeGdiBatchCount equ 01740H
TeGuaranteedStackBytes equ 01748H
TeSafeThunkCall equ 017D0H
TeFlsData equ 017C8H
ThreadEnvironmentBlockLength equ 017D8H
CmThreadEnvironmentBlockOffset equ 02000H
TLS_MINIMUM_AVAILABLE equ 00040H
TLS_EXPANSION_SLOTS equ 00400H

;
;  Lock Queue Structure Offset Definitions
;

LOCK_QUEUE_WAIT equ 00001H
LOCK_QUEUE_OWNER equ 00002H
LOCK_QUEUE_HEADER_SIZE equ 00010H

LockQueueDispatcherLock equ 00000H

LqNext equ 00000H
LqLock equ 00008H

LqhNext equ 00000H
LqhLock equ 00008H
LqhOldIrql equ 00010H

;
;  Performance Definitions
;

PERF_CONTEXTSWAP_OFFSET equ 00004H
PERF_CONTEXTSWAP_FLAG equ 00004H
PERF_INTERRUPT_OFFSET equ 00004H
PERF_INTERRUPT_FLAG equ 04000H
PERF_PROFILE_OFFSET equ 00004H
PERF_PROFILE_FLAG equ 00002H

;
;  Register Argument Home Address Offset Definitions
;

P1Home equ 00008H
P2Home equ 00010H
P3Home equ 00018H
P4Home equ 00020H

;
;  Apc Record Structure Offset Definitions
;

ArNormalRoutine equ 00000H
ArNormalContext equ 00008H
ArSystemArgument1 equ 00010H
ArSystemArgument2 equ 00018H
ApcRecordLength equ 00020H


;
;  Special Register Structure Offset Definition
;

SrKernelDr0 equ 00020H
SrKernelDr1 equ 00028H
SrKernelDr2 equ 00030H
SrKernelDr3 equ 00038H
SrKernelDr6 equ 00040H
SrKernelDr7 equ 00048H
SrGdtr equ 00050H
SrIdtr equ 00060H
SrTr equ 00070H
SrMxCsr equ 00074H
SrMsrGsBase equ 000A8H
SrMsrGsSwap equ 000B0H
SrMsrStar equ 000B8H
SrMsrLStar equ 000C0H
SrMsrCStar equ 000C8H
SrMsrSyscallMask equ 000D0H

;
;  Processor Control Region Structure Offset Definitions
;

PcGdt equ 00000H
PcTss equ 00008H
PcPerfGlobalGroupMask equ 00010H
PcSelf equ 00018H
PcCurrentPrcb equ 00020H
PcLockArray equ 00028H
PcTeb equ 00030H
PcIdt equ 00038H
PcIrql equ 00050H
PcStallScaleFactor equ 00064H
PcHalReserved equ 000C0H
PcPrcb equ 00180H
PcNumber equ 00184H
PcNestingLevel equ 00185H
PcInterruptRequest equ 00186H
PcIdleHalt equ 00187H
PcCurrentThread equ 00188H
PcNextThread equ 00190H
PcIdleThread equ 00198H
PcSetMember equ 001B8H
PcCr0 equ 001C0H
PcCr2 equ 001C8H
PcCr3 equ 001D0H
PcCr4 equ 001D8H
PcKernelDr0 equ 001E0H
PcKernelDr1 equ 001E8H
PcKernelDr2 equ 001F0H
PcKernelDr3 equ 001F8H
PcKernelDr7 equ 00208H
PcGdtrLimit equ 00216H
PcGdtrBase equ 00218H
PcIdtrLimit equ 00226H
PcIdtrBase equ 00228H
PcTr equ 00230H
PcLdtr equ 00232H
PcDebugControl equ 00238H
PcLastBranchToRip equ 00240H
PcLastBranchFromRip equ 00248H
PcLastExceptionToRip equ 00250H
PcLastExceptionFromRip equ 00258H
PcCr8 equ 00260H
PcMxCsr equ 00180H
PcUserRsp equ 001A0H
PcRspBase equ 001A8H
PcPrcbLock equ 001B0H
PcCpuType equ 00770H
PcCpuID equ 00771H
PcCpuStep equ 00772H
PcCpuVendor equ 007BDH
CPU_AMD equ 00001H
CPU_INTEL equ 00002H
PcCFlushSize equ 007C4H
PcDeferredReadyListHead equ 00F08H
PcSystemCalls equ 00F48H
PcInterruptCount equ 02398H
PcDebuggerSavedIRQL equ 023B1H
PcDpcRoutineActive equ 020DEH
PcTickOffset equ 020E8H
PcMasterOffset equ 020ECH
PcSkipTick equ 023B0H
ProcessorControlRegisterLength equ 02600H

;
;  Defines for user shared data
;

USER_SHARED_DATA equ 0FFFFF78000000000H
MM_SHARED_USER_DATA_VA equ 07FFE0000H
UsTickCountMultiplier equ 00004H
UsInterruptTime equ 00008H
UsSystemTime equ 00014H
UsTestRetInstruction equ 002F8H

;
;  Tss Structure Offset Definitions
;

TssRsp0 equ 00004H
TssRsp1 equ 0000CH
TssRsp2 equ 00014H
TssPanicStack equ 00024H
TssMcaStack equ 0002CH
TssNmiStack equ 00034H
TssIoMapBase equ 00066H
TssLength equ 00068H

;
;  Gdt Descriptor Offset Definitions
;

KGDT64_NULL equ 00000H
KGDT64_R0_CODE equ 00010H
KGDT64_R0_DATA equ 00018H
KGDT64_R3_CMCODE equ 00020H
KGDT64_R3_DATA equ 00028H
KGDT64_R3_CODE equ 00030H
KGDT64_SYS_TSS equ 00040H
KGDT64_R3_CMTEB equ 00050H

;
;  GDT Entry Offset Definitions
;

KgdtBaseLow equ 00002H
KgdtBaseMiddle equ 00004H
KgdtBaseHigh equ 00007H
KgdtBaseUpper equ 00008H
KgdtLimitHigh equ 00006H
KgdtLimitLow equ 00000H


;
;  Processor Block Structure Offset Definitions
;

PbMxCsr equ 00000H
PbNumber equ 00004H
PbNestingLevel equ 00005H
PbInterruptRequest equ 00006H
PbIdleHalt equ 00007H
PbCurrentThread equ 00008H
PbNextThread equ 00010H
PbIdleThread equ 00018H
PbUserRsp equ 00020H
PbRspBase equ 00028H
PbPrcbLock equ 00030H
PbSetMember equ 00038H
PbProcessorState equ 00040H
PbCpuType equ 005F0H
PbCpuID equ 005F1H
PbCpuStep equ 005F2H
PbHalReserved equ 005F8H
PbMinorVersion equ 00638H
PbMajorVersion equ 0063AH
PbBuildType equ 0063CH
PbLockQueue equ 00670H
PbPPLookasideList equ 00880H
PbPPNPagedLookasideList equ 00980H
PbPPPagedLookasideList equ 00B80H
PbPacketBarrier equ 00D80H
PbDeferredReadyListHead equ 00D88H
PbLookasideIrpFloat equ 00DC4H
PbSystemCalls equ 00DC8H
PbReadOperationCount equ 00DCCH
PbWriteOperationCount equ 00DD0H
PbOtherOperationCount equ 00DD4H
PbReadTransferCount equ 00DD8H
PbWriteTransferCount equ 00DE0H
PbOtherTransferCount equ 00DE8H
PbContextSwitches equ 00DF0H
PbTargetSet equ 00E00H
PbIpiFrozen equ 00E08H
PbRequestMailbox equ 00E80H
PbSenderSummary equ 01E80H
PbDpcListHead equ 01F00H
PbDpcLock equ 01F10H
PbDpcQueueDepth equ 01F18H
PbDpcCount equ 01F1CH
PbDpcStack equ 01F40H
PbSavedRsp equ 01F48H
PbMaximumDpcQueueDepth equ 01F50H
PbDpcRequestRate equ 01F54H
PbMinimumDpcRate equ 01F58H
PbDpcInterruptRequested equ 01F5CH
PbDpcThreadRequested equ 01F5DH
PbDpcRoutineActive equ 01F5EH
PbDpcThreadActive equ 01F5FH
PbTimerHand equ 01F60H
PbTimerRequest equ 01F60H
PbTickOffset equ 01F68H
PbMasterOffset equ 01F6CH
PbDpcLastCount equ 01F70H
PbThreadDpcEnable equ 01F74H
PbQuantumEnd equ 01F75H
PbDpcSetEventRequest equ 01F78H
PbIdleSchedule equ 01F77H
PbReadySummary equ 02010H
PbDispatcherReadyListHead equ 02018H
PbInterruptCount equ 02218H
PbKernelTime equ 0221CH
PbUserTime equ 02220H
PbDpcTime equ 02224H
PbInterruptTime equ 02228H
PbAdjustDpcThreshold equ 0222CH
PbSkipTick equ 02230H
PbPollSlot equ 02232H
PbParentNode equ 02240H
PbMultiThreadProcessorSet equ 02248H
PbMultiThreadSetMaster equ 02250H
PbSleeping equ 02258H
PbDebugDpcTime equ 02260H
PbPageColor equ 02264H
PbNodeColor equ 02268H
PbNodeShiftedColor equ 0226CH
PbSecondaryColorMask equ 02270H
PbFastReadNoWait equ 02280H
PbFastReadWait equ 02284H
PbFastReadNotPossible equ 02288H
PbCopyReadNoWait equ 0228CH
PbCopyReadWait equ 02290H
PbCopyReadNoWaitMiss equ 02294H
PbAlignmentFixupCount equ 02298H
PbDcacheFlushCount equ 0229CH
PbExceptionDispatchCount equ 022A0H
PbFirstLevelTbFills equ 022A4H
PbFloatingEmulationCount equ 022A8H
PbIcacheFlushCount equ 022ACH
PbSecondLevelTbFills equ 022B0H
PbVendorString equ 022B4H
PbPowerState equ 022D0H
ProcessorBlockLength equ 02480H

;
;  Processor Power State Offset Definitions
;

PpIdleFunction equ 00000H

;
;  Time Fields (TIME_FIELDS) Structure Offset Definitions
;

TfSecond equ 0000AH
TfMinute equ 00008H
TfHour equ 00006H
TfWeekday equ 0000EH
TfDay equ 00004H
TfMonth equ 00002H
TfYear equ 00000H
TfMilliseconds equ 0000CH


;
;  Define constants for system IRQL and IDT vector conversion
;

MAXIMUM_IDTVECTOR equ 000FFH
MAXIMUM_PRIMARY_VECTOR equ 000FFH
PRIMARY_VECTOR_BASE equ 00030H
RPL_MASK equ 00003H
MODE_BIT equ 00000H
MODE_MASK equ 00001H

;
;  Flags in the CR0 register
;

CR0_PG equ 080000000H
CR0_ET equ 00010H
CR0_TS equ 00008H
CR0_EM equ 00004H
CR0_MP equ 00002H
CR0_PE equ 00001H
CR0_CD equ 040000000H
CR0_NW equ 020000000H
CR0_AM equ 040000H
CR0_WP equ 010000H
CR0_NE equ 00020H

;
;  Flags in the CR4 register
;

CR4_VME equ 00001H
CR4_PVI equ 00002H
CR4_TSD equ 00004H
CR4_DE equ 00008H
CR4_PSE equ 00010H
CR4_PAE equ 00020H
CR4_MCE equ 00040H
CR4_PGE equ 00080H
CR4_FXSR equ 00200H
CR4_XMMEXCPT equ 00400H

;
;  Legacy Floating Status Bit Masks
;

FSW_INVALID_OPERATION equ 00001H
FSW_DENORMAL equ 00002H
FSW_ZERO_DIVIDE equ 00004H
FSW_OVERFLOW equ 00008H
FSW_UNDERFLOW equ 00010H
FSW_PRECISION equ 00020H
FSW_STACK_FAULT equ 00040H
FSW_CONDITION_CODE_0 equ 00100H
FSW_CONDITION_CODE_1 equ 00200H
FSW_CONDITION_CODE_2 equ 00400H
FSW_CONDITION_CODE_3 equ 04000H
FSW_ERROR_MASK equ 0003FH

;
;  Legacy Floating States
;

LEGACY_STATE_UNUSED equ 00000H
LEGACY_STATE_SWITCH equ 00001H

;
;  MXCSR Floating Control/Status Bit Masks
;

XSW_INVALID_OPERATION equ 00001H
XSW_DENORMAL equ 00002H
XSW_ZERO_DIVIDE equ 00004H
XSW_OVERFLOW equ 00008H
XSW_UNDERFLOW equ 00010H
XSW_PRECISION equ 00020H
XSW_ERROR_MASK equ 0003FH
XSW_ERROR_SHIFT equ 00007H
XCW_INVALID_OPERATION equ 00080H
XCW_DENORMAL equ 00100H
XCW_ZERO_DIVIDE equ 00200H
XCW_OVERFLOW equ 00400H
XCW_UNDERFLOW equ 00800H
XCW_PRECISION equ 01000H
XCW_ROUND_CONTROL equ 06000H
XCW_FLUSH_ZERO equ 08000H
INITIAL_FPCSR equ 0027FH
INITIAL_MXCSR equ 01F80H

;
;  Machine Specific Register Numbers
;

MSR_EFER equ 0C0000080H
MSR_STAR equ 0C0000081H
MSR_LSTAR equ 0C0000082H
MSR_CSTAR equ 0C0000083H
MSR_SYSCALL_MASK equ 0C0000084H
MSR_FS_BASE equ 0C0000100H
MSR_GS_BASE equ 0C0000101H
MSR_GS_SWAP equ 0C0000102H
MSR_MCG_STATUS equ 0017AH

;
;  Flags within MSR_EFER
;

MSR_LMA equ 00400H
MSR_LME equ 00100H
MSR_SCE equ 00001H
MSR_NXE equ 00800H
MSR_PAT equ 00277H
MSR_DEGUG_CTL equ 001D9H
MSR_LAST_BRANCH_FROM equ 001DBH
MSR_LAST_BRANCH_TO equ 001DCH
MSR_LAST_EXCEPTION_FROM equ 001DDH
MSR_LAST_EXCEPTION_TO equ 001DEH

;
;  Flags within MSR_DEGUG_CTL
;

MSR_DEBUG_CTL_LBR equ 00001H
MSR_DEBUG_CRL_BTF equ 00002H

;
;  Miscellaneous Definitions
;

MAXIMUM_PROCESSORS equ 00040H
INITIAL_STALL_COUNT equ 00064H
IRQL_NOT_GREATER_OR_EQUAL equ 00009H
IRQL_NOT_LESS_OR_EQUAL equ 0000AH
MUTEX_ALREADY_OWNED equ 000BFH
THREAD_NOT_MUTEX_OWNER equ 00011H
SPIN_LOCK_ALREADY_OWNED equ 0000FH
SPIN_LOCK_NOT_OWNED equ 00010H
Executive equ 00000H
KernelMode equ 00000H
UserMode equ 00001H
FALSE equ 00000H
TRUE equ 00001H
KERNEL_MCA_EXCEPTION_STACK_SIZE equ 02000H
NMI_STACK_SIZE equ 02000H
EXCEPTION_EXECUTE_FAULT equ 00008H
BASE_PRIORITY_THRESHOLD equ 00008H
EVENT_PAIR_INCREMENT equ 00001H
LOW_REALTIME_PRIORITY equ 00010H
BlackHole equ 0FFFFA000H
KERNEL_LARGE_STACK_COMMIT equ 06000H
KERNEL_LARGE_STACK_SIZE equ 012000H
KERNEL_STACK_SIZE equ 06000H
DOUBLE_FAULT_STACK_SIZE equ 02000H
BREAKPOINT_BREAK equ 00000H
BREAKPOINT_COMMAND_STRING equ 00005H
BREAKPOINT_PRINT equ 00001H
BREAKPOINT_PROMPT equ 00002H
BREAKPOINT_LOAD_SYMBOLS equ 00003H
BREAKPOINT_UNLOAD_SYMBOLS equ 00004H
CLOCK_QUANTUM_DECREMENT equ 00003H
READY_SKIP_QUANTUM equ 00002H
THREAD_QUANTUM equ 00006H
WAIT_QUANTUM_DECREMENT equ 00001H
ROUND_TRIP_DECREMENT_COUNT equ 00010H
KI_EXCEPTION_GP_FAULT equ 010000001H
KI_EXCEPTION_INVALID_OP equ 010000002H
KI_EXCEPTION_INTEGER_DIVIDE_BY_ZERO equ 010000003H
KI_EXCEPTION_ACCESS_VIOLATION equ 010000004H

;
;  Equates for exceptions which cause system fatal error
;

EXCEPTION_DIVIDED_BY_ZERO equ 00000H
EXCEPTION_DEBUG equ 00001H
EXCEPTION_NMI equ 00002H
EXCEPTION_INT3 equ 00003H
EXCEPTION_BOUND_CHECK equ 00005H
EXCEPTION_INVALID_OPCODE equ 00006H
EXCEPTION_NPX_NOT_AVAILABLE equ 00007H
EXCEPTION_DOUBLE_FAULT equ 00008H
EXCEPTION_NPX_OVERRUN equ 00009H
EXCEPTION_INVALID_TSS equ 0000AH
EXCEPTION_SEGMENT_NOT_PRESENT equ 0000BH
EXCEPTION_STACK_FAULT equ 0000CH
EXCEPTION_GP_FAULT equ 0000DH
EXCEPTION_RESERVED_TRAP equ 0000FH
EXCEPTION_NPX_ERROR equ 00010H
EXCEPTION_ALIGNMENT_CHECK equ 00011H

;
;  Exception Frame Offset Definitions and Length
;

ExP1Home equ 00000H
ExP2Home equ 00008H
ExP3Home equ 00010H
ExP4Home equ 00018H
ExP5 equ 00020H
ExXmm6 equ 00030H
ExXmm7 equ 00040H
ExXmm8 equ 00050H
ExXmm9 equ 00060H
ExXmm10 equ 00070H
ExXmm11 equ 00080H
ExXmm12 equ 00090H
ExXmm13 equ 000A0H
ExXmm14 equ 000B0H
ExXmm15 equ 000C0H
ExExceptionRecord equ 000F0H
ExMxCsr equ 00130H
ExRbp equ 00138H
ExRbx equ 00140H
ExRdi equ 00148H
ExRsi equ 00150H
ExR12 equ 00158H
ExR13 equ 00160H
ExR14 equ 00168H
ExR15 equ 00170H
ExReturn equ 00178H

KEXCEPTION_FRAME_LENGTH equ 00180H
EXCEPTION_RECORD_LENGTH equ 000A0H

;
;  Jump Offset Definitions and Length
;

JbFrame equ 00000H
JbRbx equ 00008H
JbRsp equ 00010H
JbRbp equ 00018H
JbRsi equ 00020H
JbRdi equ 00028H
JbR12 equ 00030H
JbR13 equ 00038H
JbR14 equ 00040H
JbR15 equ 00048H
JbRip equ 00050H
JbMxCsr equ 00058H
JbFpCsr equ 0005CH
JbXmm6 equ 00060H
JbXmm7 equ 00070H
JbXmm8 equ 00080H
JbXmm9 equ 00090H
JbXmm10 equ 000A0H
JbXmm11 equ 000B0H
JbXmm12 equ 000C0H
JbXmm13 equ 000D0H
JbXmm14 equ 000E0H
JbXmm15 equ 000F0H

;
;  Switch Frame Offset Definitions and Length
;

SwP5Home equ 00020H
SwApcBypass equ 00028H
SwRbp equ 00030H
SwReturn equ 00038H

KSWITCH_FRAME_LENGTH equ 00040H

;
;  Start System Thread Frame Offset Definitions and Length
;

SfP1Home equ 00000H
SfP2Home equ 00008H
SfP3Home equ 00010H
SfP4Home equ 00018H
SfReturn equ 00028H

KSTART_FRAME_LENGTH equ 00030H

;
;  Trap Frame Offset and EFLAG Definitions and Length
;

EFLAGS_TF_MASK equ 00100H
EFLAGS_TF_SHIFT equ 00008H
EFLAGS_ID_MASK equ 0200000H
EFLAGS_IF_MASK equ 00200H
EFLAGS_IF_SHIFT equ 00009H

TrP1Home equ 0FFFFFF80H
TrP2Home equ 0FFFFFF88H
TrP3Home equ 0FFFFFF90H
TrP4Home equ 0FFFFFF98H
TrP5 equ 0FFFFFFA0H
TrPreviousMode equ 0FFFFFFA8H
TrPreviousIrql equ 0FFFFFFA9H
TrFaultIndicator equ 0FFFFFFAAH
TrExceptionActive equ 0FFFFFFABH
TrMxCsr equ 0FFFFFFACH
TrRax equ 0FFFFFFB0H
TrRcx equ 0FFFFFFB8H
TrRdx equ 0FFFFFFC0H
TrR8 equ 0FFFFFFC8H
TrR9 equ 0FFFFFFD0H
TrR10 equ 0FFFFFFD8H
TrR11 equ 0FFFFFFE0H
TrGsBase equ 0FFFFFFE8H
TrGsSwap equ 0FFFFFFE8H
TrXmm0 equ 0FFFFFFF0H
TrXmm1 equ 00000H
TrXmm2 equ 00010H
TrXmm3 equ 00020H
TrXmm4 equ 00030H
TrXmm5 equ 00040H
TrFaultAddress equ 00050H
TrTimeStamp equ 00050H
TrDr0 equ 00058H
TrDr1 equ 00060H
TrDr2 equ 00068H
TrDr3 equ 00070H
TrDr6 equ 00078H
TrDr7 equ 00080H
TrDebugControl equ 00088H
TrLastBranchToRip equ 00090H
TrLastBranchFromRip equ 00098H
TrLastExceptionToRip equ 000A0H
TrLastExceptionFromRip equ 000A8H
TrLastBranchControl equ 00088H
TrLastBranchMSR equ 00090H
TrSegDs equ 000B0H
TrSegEs equ 000B2H
TrSegFs equ 000B4H
TrSegGs equ 000B6H
TrTrapFrame equ 000B8H
TrRbx equ 000C0H
TrRdi equ 000C8H
TrRsi equ 000D0H
TrRbp equ 000D8H
TrErrorCode equ 000E0H
TrRip equ 000E8H
TrSegCs equ 000F0H
TrEFlags equ 000F8H
TrRsp equ 00100H
TrSegSs equ 00108H
TrCodePatchCycle equ 0010CH

KTRAP_FRAME_LENGTH equ 00190H

;
;  CPU information structure offset definitions
;

CpuEax equ 00000H
CpuEbx equ 00004H
CpuEcx equ 00008H
CpuEdx equ 0000CH

;
;  Usermode Callout User Frame Definitions
;

CkBuffer equ 00020H
CkLength equ 00028H
CkApiNumber equ 0002CH
CkRsp equ 00048H
CkRip equ 00030H
CalloutFrameLength equ 00058H

;
;  Machine Frame Offset Definitions
;

MfRip equ 00000H
MfSegCs equ 00008H
MfEFlags equ 00010H
MfRsp equ 00018H
MfSegSs equ 00020H
MachineFrameLength equ 00028H

;
;  LPC Structure Offset Definitions
;

PmLength equ 00000H
PmZeroInit equ 00004H
PmClientId equ 00008H
PmProcess equ 00008H
PmThread equ 00010H
PmMessageId equ 00018H
PmClientViewSize equ 00020H
PortMessageLength equ 00028H

;
;  Client Id Structure Offset Definitions
;

CidUniqueProcess equ 00000H
CidUniqueThread equ 00008H

;
;  Context Frame Offset and Flag Definitions
;

CONTEXT_FULL equ 010000BH
CONTEXT_CONTROL equ 0100001H
CONTEXT_INTEGER equ 0100002H
CONTEXT_SEGMENTS equ 0100004H
CONTEXT_FLOATING_POINT equ 0100008H
CONTEXT_DEBUG_REGISTERS equ 0100010H

CxP1Home equ 00000H
CxP2Home equ 00008H
CxP3Home equ 00010H
CxP4Home equ 00018H
CxP5Home equ 00020H
CxP6Home equ 00028H
CxContextFlags equ 00030H
CxMxCsr equ 00034H
CxSegCs equ 00038H
CxSegDs equ 0003AH
CxSegEs equ 0003CH
CxSegFs equ 0003EH
CxSegGs equ 00040H
CxSegSs equ 00042H
CxEFlags equ 00044H
CxDr0 equ 00048H
CxDr1 equ 00050H
CxDr2 equ 00058H
CxDr3 equ 00060H
CxDr6 equ 00068H
CxDr7 equ 00070H
CxRax equ 00078H
CxRcx equ 00080H
CxRdx equ 00088H
CxRbx equ 00090H
CxRsp equ 00098H
CxRbp equ 000A0H
CxRsi equ 000A8H
CxRdi equ 000B0H
CxR8 equ 000B8H
CxR9 equ 000C0H
CxR10 equ 000C8H
CxR11 equ 000D0H
CxR12 equ 000D8H
CxR13 equ 000E0H
CxR14 equ 000E8H
CxR15 equ 000F0H
CxRip equ 000F8H
CxFltSave equ 00100H
CxXmm0 equ 001A0H
CxXmm1 equ 001B0H
CxXmm2 equ 001C0H
CxXmm3 equ 001D0H
CxXmm4 equ 001E0H
CxXmm5 equ 001F0H
CxXmm6 equ 00200H
CxXmm7 equ 00210H
CxXmm8 equ 00220H
CxXmm9 equ 00230H
CxXmm10 equ 00240H
CxXmm11 equ 00250H
CxXmm12 equ 00260H
CxXmm13 equ 00270H
CxXmm14 equ 00280H
CxXmm15 equ 00290H
CxDebugControl equ 004A8H
CxLastBranchToRip equ 004B0H
CxLastBranchFromRip equ 004B8H
CxLastExceptionToRip equ 004C0H
CxLastExceptionFromRip equ 004C8H
CxVectorControl equ 004A0H
CxVectorRegister equ 00300H
CONTEXT_FRAME_LENGTH equ 004D0H

;
;  Debug Control Register Masks
;

DR7_LEGAL equ 0FFFF0355H
DR7_ACTIVE equ 00355H
DR7_TRACE_BRANCH equ 00200H
DR7_LAST_BRANCH equ 00100H

;
;  Dispatcher Context Structure Offset Definitions
;

DcControlPc equ 00000H
DcImageBase equ 00008H
DcFunctionEntry equ 00010H
DcEstablisherFrame equ 00018H
DcTargetIp equ 00020H
DcContextRecord equ 00028H
DcLanguageHandler equ 00030H
DcHandlerData equ 00038H
DcHistoryTable equ 00040H
DcScopeIndex equ 00048H

;
;  Legacy Floating Save Area Structure Offset definitions
;

LfControlWord equ 00000H
LfStatusWord equ 00002H
LfTagWord equ 00004H
LfErrorOpcode equ 00006H
LfErrorOffset equ 00008H
LfErrorSelector equ 0000CH
LfDataOffset equ 00010H
LfDataSelector equ 00014H
LfMxCsr equ 00018H
LfMxCsr_Mask equ 0001CH
LfFloatRegisters equ 00020H

LEGACY_SAVE_AREA_LENGTH equ 00200H

;
;  Kernel Stack Control Structure Offset Definitions
;

KcCurrentBase equ 001B0H
KcCurrentLimit equ 001B8H
KcActualLimit equ 001D0H
KcPreviousBase equ 001D8H
KcPreviousLimit equ 001E0H
KcPreviousKernel equ 001E8H
KcPreviousInitial equ 001F0H

KERNEL_STACK_CONTROL_LENGTH equ 00200H

;
;  Processor State Frame Offset Definitions
;

PsSpecialRegisters equ 00000H
PsCr0 equ 00000H
PsCr2 equ 00008H
PsCr3 equ 00010H
PsCr4 equ 00018H
PsKernelDr0 equ 00020H
PsKernelDr1 equ 00028H
PsKernelDr2 equ 00030H
PsKernelDr3 equ 00038H
PsKernelDr6 equ 00040H
PsKernelDr7 equ 00048H
PsGdtr equ 00056H
PsIdtr equ 00066H
PsTr equ 00070H
PsLdtr equ 00072H
PsMxCsr equ 00074H
PsContextFrame equ 000E0H
PsDebugControl equ 00078H
PsLastBranchToRip equ 00080H
PsLastBranchFromRip equ 00088H
PsLastExceptionToRip equ 00090H
PsLastExceptionFromRip equ 00098H
PsCr8 equ 000A0H
ProcessorStateLength equ 005B0H

;
;  Processor Start Block Offset Definitions
;

PsbCompletionFlag equ 00004H
PsbGdt32 equ 00008H
PsbIdt32 equ 0000EH
PsbGdt equ 00018H
PsbTiledCr3 equ 00058H
PsbPmTarget equ 00060H
PsbLmTarget equ 00070H
PsbLmIdentityTarget equ 00066H
PsbSelfMap equ 00078H
PsbMsrPat equ 00080H
PsbMsrEFER equ 00088H
PsbProcessorState equ 00090H
ProcessorStartBlockLength equ 00640H

;
;  EPROCESS
;

EpDebugPort equ 00148H
EpVdmObjects equ 00230H

;
;  Machine type definitions (Temporarily)
;

MACHINE_TYPE_ISA equ 00000H
MACHINE_TYPE_EISA equ 00001H
MACHINE_TYPE_MCA equ 00002H

;
;  KeFeatureBits defines
;

KF_RDTSC equ 00002H
KF_CR4 equ 00004H
KF_GLOBAL_PAGE equ 00010H
KF_LARGE_PAGE equ 00020H
KF_CMPXCHG8B equ 00080H
KF_FAST_SYSCALL equ 01000H

;
;  LoaderParameterBlock offsets relative to base
;

LpbLoadOrderListHead equ 00000H
LpbMemoryDescriptorListHead equ 00010H
LpbKernelStack equ 00030H
LpbPrcb equ 00038H
LpbProcess equ 00040H
LpbThread equ 00048H
LpbI386 equ 000B8H
LpbRegistryLength equ 00050H
LpbRegistryBase equ 00058H
LpbConfigurationRoot equ 00060H
LpbArcBootDeviceName equ 00068H
LpbArcHalDeviceName equ 00070H
LpbLoadOptions equ 00088H
PAGE_SIZE equ 01000H

;
;  Kernel Mode Callout Frame Definitions
;

CuInitialStack equ 00028H
CuTrapFrame equ 000D0H
CuCallbackStack equ 000D8H
CuOutputBuffer equ 000E0H
CuOutputLength equ 000E8H

;
;  IO Status Block Offset Definitions
;

IoStatus equ 00000H
IoPointer equ 00000H
IoInformation equ 00008H

