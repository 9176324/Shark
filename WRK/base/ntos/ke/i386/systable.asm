;++
;
; Copyright (c) Microsoft Corporation. All rights reserved. 
;
; You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
; If you do not agree to the terms, do not use the code.
;
;
; Module Name:
;
;    systable.asm
;
; Abstract:
;
;    This module implements the system service dispatch table.
;
;--

;
; To add a system service simply add the name of the service to the below
; table. If the system service has arguments, then immediately
; follow the name of the serice with a comma and following that the number
; of bytes of in memory arguments, e.g. CreateObject,40.
;

;ifdef i386

.386p
include callconv.inc

TABLE_ERROR macro t
.err    ; Maximum number of in-memory system service arguments exceeded.
        endm

TABLE_BEGIN1 macro t
    TITLE t
endm
TABLE_BEGIN2 macro t
_TEXT	SEGMENT DWORD PUBLIC 'DATA'
	ASSUME	DS:FLAT
endm
TABLE_BEGIN3 macro t
    align 4
endm
TABLE_BEGIN4 macro t
    public _KiServiceTable
_KiServiceTable label dword
endm
TABLE_BEGIN5 macro t
endm
TABLE_BEGIN6 macro t
endm
TABLE_BEGIN7 macro t
endm
TABLE_BEGIN8 macro t
endm

TABLE_ENTRY macro l,bias,numargs
        Local   Bytes

        Bytes = numargs*4

        EXTRNP  _Nt&l,&numargs
        ComposeInst <dd offset FLAT:>,_Nt,l,<@>,%(Bytes)
endm

TABLE_END macro n
    public _KiServiceLimit
_KiServiceLimit dd n+1
endm

ARGTBL_BEGIN macro
    public _KiArgumentTable
_KiArgumentTable label dword
endm

ARGTBL_ENTRY macro e0,e1,e2,e3,e4,e5,e6,e7
        db   e0,e1,e2,e3,e4,e5,e6,e7
endm

ARGTBL_END macro
_TEXT   ENDS
        end
endm

;endif

        TABLE_BEGIN1 <"System Service Dispatch Table">
        TABLE_BEGIN2 <"System Service Dispatch Table">
        TABLE_BEGIN3 <"System Service Dispatch Table">
        TABLE_BEGIN4 <"System Service Dispatch Table">
        TABLE_BEGIN5 <"System Service Dispatch Table">
        TABLE_BEGIN6 <"System Service Dispatch Table">
        TABLE_BEGIN7 <"System Service Dispatch Table">
        TABLE_BEGIN8 <"System Service Dispatch Table">
TABLE_ENTRY  AcceptConnectPort, 1, 6 
TABLE_ENTRY  AccessCheck, 1, 8 
TABLE_ENTRY  AccessCheckAndAuditAlarm, 1, 11 
TABLE_ENTRY  AccessCheckByType, 1, 11 
TABLE_ENTRY  AccessCheckByTypeAndAuditAlarm, 1, 16 
TABLE_ENTRY  AccessCheckByTypeResultList, 1, 11 
TABLE_ENTRY  AccessCheckByTypeResultListAndAuditAlarm, 1, 16 
TABLE_ENTRY  AccessCheckByTypeResultListAndAuditAlarmByHandle, 1, 17 
TABLE_ENTRY  AddAtom, 1, 3 
TABLE_ENTRY  AddBootEntry, 1, 2 
TABLE_ENTRY  AddDriverEntry, 1, 2 
TABLE_ENTRY  AdjustGroupsToken, 1, 6 
TABLE_ENTRY  AdjustPrivilegesToken, 1, 6 
TABLE_ENTRY  AlertResumeThread, 1, 2 
TABLE_ENTRY  AlertThread, 1, 1 
TABLE_ENTRY  AllocateLocallyUniqueId, 1, 1 
TABLE_ENTRY  AllocateUserPhysicalPages, 1, 3 
TABLE_ENTRY  AllocateUuids, 1, 4 
TABLE_ENTRY  AllocateVirtualMemory, 1, 6 
TABLE_ENTRY  ApphelpCacheControl, 1, 2 
TABLE_ENTRY  AreMappedFilesTheSame, 1, 2 
TABLE_ENTRY  AssignProcessToJobObject, 1, 2 
TABLE_ENTRY  CallbackReturn, 1, 3 
TABLE_ENTRY  CancelDeviceWakeupRequest, 1, 1 
TABLE_ENTRY  CancelIoFile, 1, 2 
TABLE_ENTRY  CancelTimer, 1, 2 
TABLE_ENTRY  ClearEvent, 1, 1 
TABLE_ENTRY  Close, 1, 1 
TABLE_ENTRY  CloseObjectAuditAlarm, 1, 3 
TABLE_ENTRY  CompactKeys, 1, 2 
TABLE_ENTRY  CompareTokens, 1, 3 
TABLE_ENTRY  CompleteConnectPort, 1, 1 
TABLE_ENTRY  CompressKey, 1, 1 
TABLE_ENTRY  ConnectPort, 1, 8 
TABLE_ENTRY  Continue, 1, 2 
TABLE_ENTRY  CreateDebugObject, 1, 4 
TABLE_ENTRY  CreateDirectoryObject, 1, 3 
TABLE_ENTRY  CreateEvent, 1, 5 
TABLE_ENTRY  CreateEventPair, 1, 3 
TABLE_ENTRY  CreateFile, 1, 11 
TABLE_ENTRY  CreateIoCompletion, 1, 4 
TABLE_ENTRY  CreateJobObject, 1, 3 
TABLE_ENTRY  CreateJobSet, 1, 3 
TABLE_ENTRY  CreateKey, 1, 7 
TABLE_ENTRY  CreateMailslotFile, 1, 8 
TABLE_ENTRY  CreateMutant, 1, 4 
TABLE_ENTRY  CreateNamedPipeFile, 1, 14 
TABLE_ENTRY  CreatePagingFile, 1, 4 
TABLE_ENTRY  CreatePort, 1, 5 
TABLE_ENTRY  CreateProcess, 1, 8 
TABLE_ENTRY  CreateProcessEx, 1, 9 
TABLE_ENTRY  CreateProfile, 1, 9 
TABLE_ENTRY  CreateSection, 1, 7 
TABLE_ENTRY  CreateSemaphore, 1, 5 
TABLE_ENTRY  CreateSymbolicLinkObject, 1, 4 
TABLE_ENTRY  CreateThread, 1, 8 
TABLE_ENTRY  CreateTimer, 1, 4 
TABLE_ENTRY  CreateToken, 1, 13 
TABLE_ENTRY  CreateWaitablePort, 1, 5 
TABLE_ENTRY  DebugActiveProcess, 1, 2 
TABLE_ENTRY  DebugContinue, 1, 3 
TABLE_ENTRY  DelayExecution, 1, 2 
TABLE_ENTRY  DeleteAtom, 1, 1 
TABLE_ENTRY  DeleteBootEntry, 1, 1 
TABLE_ENTRY  DeleteDriverEntry, 1, 1 
TABLE_ENTRY  DeleteFile, 1, 1 
TABLE_ENTRY  DeleteKey, 1, 1 
TABLE_ENTRY  DeleteObjectAuditAlarm, 1, 3 
TABLE_ENTRY  DeleteValueKey, 1, 2 
TABLE_ENTRY  DeviceIoControlFile, 1, 10 
TABLE_ENTRY  DisplayString, 1, 1 
TABLE_ENTRY  DuplicateObject, 1, 7 
TABLE_ENTRY  DuplicateToken, 1, 6 
TABLE_ENTRY  EnumerateBootEntries, 1, 2 
TABLE_ENTRY  EnumerateDriverEntries, 1, 2 
TABLE_ENTRY  EnumerateKey, 1, 6 
TABLE_ENTRY  EnumerateSystemEnvironmentValuesEx, 1, 3 
TABLE_ENTRY  EnumerateValueKey, 1, 6 
TABLE_ENTRY  ExtendSection, 1, 2 
TABLE_ENTRY  FilterToken, 1, 6 
TABLE_ENTRY  FindAtom, 1, 3 
TABLE_ENTRY  FlushBuffersFile, 1, 2 
TABLE_ENTRY  FlushInstructionCache, 1, 3 
TABLE_ENTRY  FlushKey, 1, 1 
TABLE_ENTRY  FlushVirtualMemory, 1, 4 
TABLE_ENTRY  FlushWriteBuffer, 0, 0 
TABLE_ENTRY  FreeUserPhysicalPages, 1, 3 
TABLE_ENTRY  FreeVirtualMemory, 1, 4 
TABLE_ENTRY  FsControlFile, 1, 10 
TABLE_ENTRY  GetContextThread, 1, 2 
TABLE_ENTRY  GetDevicePowerState, 1, 2 
TABLE_ENTRY  GetPlugPlayEvent, 1, 4 
TABLE_ENTRY  GetWriteWatch, 1, 7 
TABLE_ENTRY  ImpersonateAnonymousToken, 1, 1 
TABLE_ENTRY  ImpersonateClientOfPort, 1, 2 
TABLE_ENTRY  ImpersonateThread, 1, 3 
TABLE_ENTRY  InitializeRegistry, 1, 1 
TABLE_ENTRY  InitiatePowerAction, 1, 4 
TABLE_ENTRY  IsProcessInJob, 1, 2 
TABLE_ENTRY  IsSystemResumeAutomatic, 0, 0 
TABLE_ENTRY  ListenPort, 1, 2 
TABLE_ENTRY  LoadDriver, 1, 1 
TABLE_ENTRY  LoadKey, 1, 2 
TABLE_ENTRY  LoadKey2, 1, 3 
TABLE_ENTRY  LoadKeyEx, 1, 4 
TABLE_ENTRY  LockFile, 1, 10 
TABLE_ENTRY  LockProductActivationKeys, 1, 2 
TABLE_ENTRY  LockRegistryKey, 1, 1 
TABLE_ENTRY  LockVirtualMemory, 1, 4 
TABLE_ENTRY  MakePermanentObject, 1, 1 
TABLE_ENTRY  MakeTemporaryObject, 1, 1 
TABLE_ENTRY  MapUserPhysicalPages, 1, 3 
TABLE_ENTRY  MapUserPhysicalPagesScatter, 1, 3 
TABLE_ENTRY  MapViewOfSection, 1, 10 
TABLE_ENTRY  ModifyBootEntry, 1, 1 
TABLE_ENTRY  ModifyDriverEntry, 1, 1 
TABLE_ENTRY  NotifyChangeDirectoryFile, 1, 9 
TABLE_ENTRY  NotifyChangeKey, 1, 10 
TABLE_ENTRY  NotifyChangeMultipleKeys, 1, 12 
TABLE_ENTRY  OpenDirectoryObject, 1, 3 
TABLE_ENTRY  OpenEvent, 1, 3 
TABLE_ENTRY  OpenEventPair, 1, 3 
TABLE_ENTRY  OpenFile, 1, 6 
TABLE_ENTRY  OpenIoCompletion, 1, 3 
TABLE_ENTRY  OpenJobObject, 1, 3 
TABLE_ENTRY  OpenKey, 1, 3 
TABLE_ENTRY  OpenMutant, 1, 3 
TABLE_ENTRY  OpenObjectAuditAlarm, 1, 12 
TABLE_ENTRY  OpenProcess, 1, 4 
TABLE_ENTRY  OpenProcessToken, 1, 3 
TABLE_ENTRY  OpenProcessTokenEx, 1, 4 
TABLE_ENTRY  OpenSection, 1, 3 
TABLE_ENTRY  OpenSemaphore, 1, 3 
TABLE_ENTRY  OpenSymbolicLinkObject, 1, 3 
TABLE_ENTRY  OpenThread, 1, 4 
TABLE_ENTRY  OpenThreadToken, 1, 4 
TABLE_ENTRY  OpenThreadTokenEx, 1, 5 
TABLE_ENTRY  OpenTimer, 1, 3 
TABLE_ENTRY  PlugPlayControl, 1, 3 
TABLE_ENTRY  PowerInformation, 1, 5 
TABLE_ENTRY  PrivilegeCheck, 1, 3 
TABLE_ENTRY  PrivilegeObjectAuditAlarm, 1, 6 
TABLE_ENTRY  PrivilegedServiceAuditAlarm, 1, 5 
TABLE_ENTRY  ProtectVirtualMemory, 1, 5 
TABLE_ENTRY  PulseEvent, 1, 2 
TABLE_ENTRY  QueryAttributesFile, 1, 2 
TABLE_ENTRY  QueryBootEntryOrder, 1, 2 
TABLE_ENTRY  QueryBootOptions, 1, 2 
TABLE_ENTRY  QueryDebugFilterState, 1, 2 
TABLE_ENTRY  QueryDefaultLocale, 1, 2 
TABLE_ENTRY  QueryDefaultUILanguage, 1, 1 
TABLE_ENTRY  QueryDirectoryFile, 1, 11 
TABLE_ENTRY  QueryDirectoryObject, 1, 7 
TABLE_ENTRY  QueryDriverEntryOrder, 1, 2 
TABLE_ENTRY  QueryEaFile, 1, 9 
TABLE_ENTRY  QueryEvent, 1, 5 
TABLE_ENTRY  QueryFullAttributesFile, 1, 2 
TABLE_ENTRY  QueryInformationAtom, 1, 5 
TABLE_ENTRY  QueryInformationFile, 1, 5 
TABLE_ENTRY  QueryInformationJobObject, 1, 5 
TABLE_ENTRY  QueryInformationPort, 1, 5 
TABLE_ENTRY  QueryInformationProcess, 1, 5 
TABLE_ENTRY  QueryInformationThread, 1, 5 
TABLE_ENTRY  QueryInformationToken, 1, 5 
TABLE_ENTRY  QueryInstallUILanguage, 1, 1 
TABLE_ENTRY  QueryIntervalProfile, 1, 2 
TABLE_ENTRY  QueryIoCompletion, 1, 5 
TABLE_ENTRY  QueryKey, 1, 5 
TABLE_ENTRY  QueryMultipleValueKey, 1, 6 
TABLE_ENTRY  QueryMutant, 1, 5 
TABLE_ENTRY  QueryObject, 1, 5 
TABLE_ENTRY  QueryOpenSubKeys, 1, 2 
TABLE_ENTRY  QueryOpenSubKeysEx, 1, 4 
TABLE_ENTRY  QueryPerformanceCounter, 1, 2 
TABLE_ENTRY  QueryQuotaInformationFile, 1, 9 
TABLE_ENTRY  QuerySection, 1, 5 
TABLE_ENTRY  QuerySecurityObject, 1, 5 
TABLE_ENTRY  QuerySemaphore, 1, 5 
TABLE_ENTRY  QuerySymbolicLinkObject, 1, 3 
TABLE_ENTRY  QuerySystemEnvironmentValue, 1, 4 
TABLE_ENTRY  QuerySystemEnvironmentValueEx, 1, 5 
TABLE_ENTRY  QuerySystemInformation, 1, 4 
TABLE_ENTRY  QuerySystemTime, 1, 1 
TABLE_ENTRY  QueryTimer, 1, 5 
TABLE_ENTRY  QueryTimerResolution, 1, 3 
TABLE_ENTRY  QueryValueKey, 1, 6 
TABLE_ENTRY  QueryVirtualMemory, 1, 6 
TABLE_ENTRY  QueryVolumeInformationFile, 1, 5 
TABLE_ENTRY  QueueApcThread, 1, 5 
TABLE_ENTRY  RaiseException, 1, 3 
TABLE_ENTRY  RaiseHardError, 1, 6 
TABLE_ENTRY  ReadFile, 1, 9 
TABLE_ENTRY  ReadFileScatter, 1, 9 
TABLE_ENTRY  ReadRequestData, 1, 6 
TABLE_ENTRY  ReadVirtualMemory, 1, 5 
TABLE_ENTRY  RegisterThreadTerminatePort, 1, 1 
TABLE_ENTRY  ReleaseMutant, 1, 2 
TABLE_ENTRY  ReleaseSemaphore, 1, 3 
TABLE_ENTRY  RemoveIoCompletion, 1, 5 
TABLE_ENTRY  RemoveProcessDebug, 1, 2 
TABLE_ENTRY  RenameKey, 1, 2 
TABLE_ENTRY  ReplaceKey, 1, 3 
TABLE_ENTRY  ReplyPort, 1, 2 
TABLE_ENTRY  ReplyWaitReceivePort, 1, 4 
TABLE_ENTRY  ReplyWaitReceivePortEx, 1, 5 
TABLE_ENTRY  ReplyWaitReplyPort, 1, 2 
TABLE_ENTRY  RequestDeviceWakeup, 1, 1 
TABLE_ENTRY  RequestPort, 1, 2 
TABLE_ENTRY  RequestWaitReplyPort, 1, 3 
TABLE_ENTRY  RequestWakeupLatency, 1, 1 
TABLE_ENTRY  ResetEvent, 1, 2 
TABLE_ENTRY  ResetWriteWatch, 1, 3 
TABLE_ENTRY  RestoreKey, 1, 3 
TABLE_ENTRY  ResumeProcess, 1, 1 
TABLE_ENTRY  ResumeThread, 1, 2 
TABLE_ENTRY  SaveKey, 1, 2 
TABLE_ENTRY  SaveKeyEx, 1, 3 
TABLE_ENTRY  SaveMergedKeys, 1, 3 
TABLE_ENTRY  SecureConnectPort, 1, 9 
TABLE_ENTRY  SetBootEntryOrder, 1, 2 
TABLE_ENTRY  SetBootOptions, 1, 2 
TABLE_ENTRY  SetContextThread, 1, 2 
TABLE_ENTRY  SetDebugFilterState, 1, 3 
TABLE_ENTRY  SetDefaultHardErrorPort, 1, 1 
TABLE_ENTRY  SetDefaultLocale, 1, 2 
TABLE_ENTRY  SetDefaultUILanguage, 1, 1 
TABLE_ENTRY  SetDriverEntryOrder, 1, 2 
TABLE_ENTRY  SetEaFile, 1, 4 
TABLE_ENTRY  SetEvent, 1, 2 
TABLE_ENTRY  SetEventBoostPriority, 1, 1 
TABLE_ENTRY  SetHighEventPair, 1, 1 
TABLE_ENTRY  SetHighWaitLowEventPair, 1, 1 
TABLE_ENTRY  SetInformationDebugObject, 1, 5 
TABLE_ENTRY  SetInformationFile, 1, 5 
TABLE_ENTRY  SetInformationJobObject, 1, 4 
TABLE_ENTRY  SetInformationKey, 1, 4 
TABLE_ENTRY  SetInformationObject, 1, 4 
TABLE_ENTRY  SetInformationProcess, 1, 4 
TABLE_ENTRY  SetInformationThread, 1, 4 
TABLE_ENTRY  SetInformationToken, 1, 4 
TABLE_ENTRY  SetIntervalProfile, 1, 2 
TABLE_ENTRY  SetIoCompletion, 1, 5 
TABLE_ENTRY  SetLdtEntries, 1, 6 
TABLE_ENTRY  SetLowEventPair, 1, 1 
TABLE_ENTRY  SetLowWaitHighEventPair, 1, 1 
TABLE_ENTRY  SetQuotaInformationFile, 1, 4 
TABLE_ENTRY  SetSecurityObject, 1, 3 
TABLE_ENTRY  SetSystemEnvironmentValue, 1, 2 
TABLE_ENTRY  SetSystemEnvironmentValueEx, 1, 5 
TABLE_ENTRY  SetSystemInformation, 1, 3 
TABLE_ENTRY  SetSystemPowerState, 1, 3 
TABLE_ENTRY  SetSystemTime, 1, 2 
TABLE_ENTRY  SetThreadExecutionState, 1, 2 
TABLE_ENTRY  SetTimer, 1, 7 
TABLE_ENTRY  SetTimerResolution, 1, 3 
TABLE_ENTRY  SetUuidSeed, 1, 1 
TABLE_ENTRY  SetValueKey, 1, 6 
TABLE_ENTRY  SetVolumeInformationFile, 1, 5 
TABLE_ENTRY  ShutdownSystem, 1, 1 
TABLE_ENTRY  SignalAndWaitForSingleObject, 1, 4 
TABLE_ENTRY  StartProfile, 1, 1 
TABLE_ENTRY  StopProfile, 1, 1 
TABLE_ENTRY  SuspendProcess, 1, 1 
TABLE_ENTRY  SuspendThread, 1, 2 
TABLE_ENTRY  SystemDebugControl, 1, 6 
TABLE_ENTRY  TerminateJobObject, 1, 2 
TABLE_ENTRY  TerminateProcess, 1, 2 
TABLE_ENTRY  TerminateThread, 1, 2 
TABLE_ENTRY  TestAlert, 0, 0 
TABLE_ENTRY  TraceEvent, 1, 4 
TABLE_ENTRY  TranslateFilePath, 1, 4 
TABLE_ENTRY  UnloadDriver, 1, 1 
TABLE_ENTRY  UnloadKey, 1, 1 
TABLE_ENTRY  UnloadKey2, 1, 2 
TABLE_ENTRY  UnloadKeyEx, 1, 2 
TABLE_ENTRY  UnlockFile, 1, 5 
TABLE_ENTRY  UnlockVirtualMemory, 1, 4 
TABLE_ENTRY  UnmapViewOfSection, 1, 2 
TABLE_ENTRY  VdmControl, 1, 2 
TABLE_ENTRY  WaitForDebugEvent, 1, 4 
TABLE_ENTRY  WaitForMultipleObjects, 1, 5 
TABLE_ENTRY  WaitForSingleObject, 1, 3 
TABLE_ENTRY  WaitHighEventPair, 1, 1 
TABLE_ENTRY  WaitLowEventPair, 1, 1 
TABLE_ENTRY  WriteFile, 1, 9 
TABLE_ENTRY  WriteFileGather, 1, 9 
TABLE_ENTRY  WriteRequestData, 1, 6 
TABLE_ENTRY  WriteVirtualMemory, 1, 5 
TABLE_ENTRY  YieldExecution, 0, 0 
TABLE_ENTRY  CreateKeyedEvent, 1, 4 
TABLE_ENTRY  OpenKeyedEvent, 1, 3 
TABLE_ENTRY  ReleaseKeyedEvent, 1, 4 
TABLE_ENTRY  WaitForKeyedEvent, 1, 4 
TABLE_ENTRY  QueryPortInformationProcess, 0, 0 
TABLE_ENTRY  GetCurrentProcessorNumber, 0, 0 
TABLE_ENTRY  WaitForMultipleObjects32, 1, 5 

TABLE_END 295 

ARGTBL_BEGIN
ARGTBL_ENTRY 24,32,44,44,64,44,64,68 
ARGTBL_ENTRY 12,8,8,24,24,8,4,4 
ARGTBL_ENTRY 12,16,24,8,8,8,12,4 
ARGTBL_ENTRY 8,8,4,4,12,8,12,4 
ARGTBL_ENTRY 4,32,8,16,12,20,12,44 
ARGTBL_ENTRY 16,12,12,28,32,16,56,16 
ARGTBL_ENTRY 20,32,36,36,28,20,16,32 
ARGTBL_ENTRY 16,52,20,8,12,8,4,4 
ARGTBL_ENTRY 4,4,4,12,8,40,4,28 
ARGTBL_ENTRY 24,8,8,24,12,24,8,24 
ARGTBL_ENTRY 12,8,12,4,16,0,12,16 
ARGTBL_ENTRY 40,8,8,16,28,4,8,12 
ARGTBL_ENTRY 4,16,8,0,8,4,8,12 
ARGTBL_ENTRY 16,40,8,4,16,4,4,12 
ARGTBL_ENTRY 12,40,4,4,36,40,48,12 
ARGTBL_ENTRY 12,12,24,12,12,12,12,48 
ARGTBL_ENTRY 16,12,16,12,12,12,16,16 
ARGTBL_ENTRY 20,12,12,20,12,24,20,20 
ARGTBL_ENTRY 8,8,8,8,8,8,4,44 
ARGTBL_ENTRY 28,8,36,20,8,20,20,20 
ARGTBL_ENTRY 20,20,20,20,4,8,20,20 
ARGTBL_ENTRY 24,20,20,8,16,8,36,20 
ARGTBL_ENTRY 20,20,12,16,20,16,4,20 
ARGTBL_ENTRY 12,24,24,20,20,12,24,36 
ARGTBL_ENTRY 36,24,20,4,8,12,20,8 
ARGTBL_ENTRY 8,12,8,16,20,8,4,8 
ARGTBL_ENTRY 12,4,8,12,12,4,8,8 
ARGTBL_ENTRY 12,12,36,8,8,8,12,4 
ARGTBL_ENTRY 8,4,8,16,8,4,4,4 
ARGTBL_ENTRY 20,20,16,16,16,16,16,16 
ARGTBL_ENTRY 8,20,24,4,4,16,12,8 
ARGTBL_ENTRY 20,12,12,8,8,28,12,4 
ARGTBL_ENTRY 24,20,4,16,4,4,4,8 
ARGTBL_ENTRY 24,8,8,8,0,16,16,4 
ARGTBL_ENTRY 4,8,8,20,16,8,8,16 
ARGTBL_ENTRY 20,12,4,4,36,36,24,20 
ARGTBL_ENTRY 0,16,12,16,16,0,0,20 

ARGTBL_END
