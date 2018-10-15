;++
;
; Copyright (c) Microsoft Corporation. All rights reserved. 
;
; You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
; If you do not agree to the terms, do not use the code.
;
; Module Name:
;
;   systable.asm
;
; Abstract:
;
;   This module implements the system service dispatch table.
;
;--

include ksamd64.inc

TABLE_ERROR macro t
.err    ; Maximum number of in-memory system service arguments exceeded.
        endm

TABLE_BEGIN1 macro t
        title   t
        endm

TABLE_BEGIN2 macro t
_TEXT$00 segment page 'code'
        endm

TABLE_BEGIN3 macro t
        endm

TABLE_BEGIN4 macro t
        public  KiServiceTable
KiServiceTable label qword
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
.erre   numargs le 14
        extrn   Nt&l:proc
        dq      offset Nt&l+numargs
        endm

TABLE_END macro n
        public  KiServiceLimit
KiServiceLimit label dword
        dd     n + 1
        endm

ARGTBL_BEGIN macro
        endm

ARGTBL_ENTRY macro e0, e1, e2, e3, e4, e5, e6, e7
        endm

ARGTBL_END macro
_TEXT$00 ends
        end
        endm

        TABLE_BEGIN1 <"System Service Dispatch Table">
        TABLE_BEGIN2 <"System Service Dispatch Table">
        TABLE_BEGIN3 <"System Service Dispatch Table">
        TABLE_BEGIN4 <"System Service Dispatch Table">
        TABLE_BEGIN5 <"System Service Dispatch Table">
        TABLE_BEGIN6 <"System Service Dispatch Table">
        TABLE_BEGIN7 <"System Service Dispatch Table">
        TABLE_BEGIN8 <"System Service Dispatch Table">
TABLE_ENTRY  MapUserPhysicalPagesScatter, 0, 0 
TABLE_ENTRY  WaitForSingleObject, 0, 0 
TABLE_ENTRY  CallbackReturn, 0, 0 
TABLE_ENTRY  ReadFile, 1, 5 
TABLE_ENTRY  DeviceIoControlFile, 1, 6 
TABLE_ENTRY  WriteFile, 1, 5 
TABLE_ENTRY  RemoveIoCompletion, 1, 1 
TABLE_ENTRY  ReleaseSemaphore, 0, 0 
TABLE_ENTRY  ReplyWaitReceivePort, 0, 0 
TABLE_ENTRY  ReplyPort, 0, 0 
TABLE_ENTRY  SetInformationThread, 0, 0 
TABLE_ENTRY  SetEvent, 0, 0 
TABLE_ENTRY  Close, 0, 0 
TABLE_ENTRY  QueryObject, 1, 1 
TABLE_ENTRY  QueryInformationFile, 1, 1 
TABLE_ENTRY  OpenKey, 0, 0 
TABLE_ENTRY  EnumerateValueKey, 1, 2 
TABLE_ENTRY  FindAtom, 0, 0 
TABLE_ENTRY  QueryDefaultLocale, 0, 0 
TABLE_ENTRY  QueryKey, 1, 1 
TABLE_ENTRY  QueryValueKey, 1, 2 
TABLE_ENTRY  AllocateVirtualMemory, 1, 2 
TABLE_ENTRY  QueryInformationProcess, 1, 1 
TABLE_ENTRY  WaitForMultipleObjects32, 1, 1 
TABLE_ENTRY  WriteFileGather, 1, 5 
TABLE_ENTRY  SetInformationProcess, 0, 0 
TABLE_ENTRY  CreateKey, 1, 3 
TABLE_ENTRY  FreeVirtualMemory, 0, 0 
TABLE_ENTRY  ImpersonateClientOfPort, 0, 0 
TABLE_ENTRY  ReleaseMutant, 0, 0 
TABLE_ENTRY  QueryInformationToken, 1, 1 
TABLE_ENTRY  RequestWaitReplyPort, 0, 0 
TABLE_ENTRY  QueryVirtualMemory, 1, 2 
TABLE_ENTRY  OpenThreadToken, 0, 0 
TABLE_ENTRY  QueryInformationThread, 1, 1 
TABLE_ENTRY  OpenProcess, 0, 0 
TABLE_ENTRY  SetInformationFile, 1, 1 
TABLE_ENTRY  MapViewOfSection, 1, 6 
TABLE_ENTRY  AccessCheckAndAuditAlarm, 1, 7 
TABLE_ENTRY  UnmapViewOfSection, 0, 0 
TABLE_ENTRY  ReplyWaitReceivePortEx, 1, 1 
TABLE_ENTRY  TerminateProcess, 0, 0 
TABLE_ENTRY  SetEventBoostPriority, 0, 0 
TABLE_ENTRY  ReadFileScatter, 1, 5 
TABLE_ENTRY  OpenThreadTokenEx, 1, 1 
TABLE_ENTRY  OpenProcessTokenEx, 0, 0 
TABLE_ENTRY  QueryPerformanceCounter, 0, 0 
TABLE_ENTRY  EnumerateKey, 1, 2 
TABLE_ENTRY  OpenFile, 1, 2 
TABLE_ENTRY  DelayExecution, 0, 0 
TABLE_ENTRY  QueryDirectoryFile, 1, 7 
TABLE_ENTRY  QuerySystemInformation, 0, 0 
TABLE_ENTRY  OpenSection, 0, 0 
TABLE_ENTRY  QueryTimer, 1, 1 
TABLE_ENTRY  FsControlFile, 1, 6 
TABLE_ENTRY  WriteVirtualMemory, 1, 1 
TABLE_ENTRY  CloseObjectAuditAlarm, 0, 0 
TABLE_ENTRY  DuplicateObject, 1, 3 
TABLE_ENTRY  QueryAttributesFile, 0, 0 
TABLE_ENTRY  ClearEvent, 0, 0 
TABLE_ENTRY  ReadVirtualMemory, 1, 1 
TABLE_ENTRY  OpenEvent, 0, 0 
TABLE_ENTRY  AdjustPrivilegesToken, 1, 2 
TABLE_ENTRY  DuplicateToken, 1, 2 
TABLE_ENTRY  Continue, 0, 0 
TABLE_ENTRY  QueryDefaultUILanguage, 0, 0 
TABLE_ENTRY  QueueApcThread, 1, 1 
TABLE_ENTRY  YieldExecution, 0, 0 
TABLE_ENTRY  AddAtom, 0, 0 
TABLE_ENTRY  CreateEvent, 1, 1 
TABLE_ENTRY  QueryVolumeInformationFile, 1, 1 
TABLE_ENTRY  CreateSection, 1, 3 
TABLE_ENTRY  FlushBuffersFile, 0, 0 
TABLE_ENTRY  ApphelpCacheControl, 0, 0 
TABLE_ENTRY  CreateProcessEx, 1, 5 
TABLE_ENTRY  CreateThread, 1, 4 
TABLE_ENTRY  IsProcessInJob, 0, 0 
TABLE_ENTRY  ProtectVirtualMemory, 1, 1 
TABLE_ENTRY  QuerySection, 1, 1 
TABLE_ENTRY  ResumeThread, 0, 0 
TABLE_ENTRY  TerminateThread, 0, 0 
TABLE_ENTRY  ReadRequestData, 1, 2 
TABLE_ENTRY  CreateFile, 1, 7 
TABLE_ENTRY  QueryEvent, 1, 1 
TABLE_ENTRY  WriteRequestData, 1, 2 
TABLE_ENTRY  OpenDirectoryObject, 0, 0 
TABLE_ENTRY  AccessCheckByTypeAndAuditAlarm, 1, 12 
TABLE_ENTRY  QuerySystemTime, 0, 0 
TABLE_ENTRY  WaitForMultipleObjects, 1, 1 
TABLE_ENTRY  SetInformationObject, 0, 0 
TABLE_ENTRY  CancelIoFile, 0, 0 
TABLE_ENTRY  TraceEvent, 0, 0 
TABLE_ENTRY  PowerInformation, 1, 1 
TABLE_ENTRY  SetValueKey, 1, 2 
TABLE_ENTRY  CancelTimer, 0, 0 
TABLE_ENTRY  SetTimer, 1, 3 
TABLE_ENTRY  AcceptConnectPort, 1, 2 
TABLE_ENTRY  AccessCheck, 1, 4 
TABLE_ENTRY  AccessCheckByType, 1, 7 
TABLE_ENTRY  AccessCheckByTypeResultList, 1, 7 
TABLE_ENTRY  AccessCheckByTypeResultListAndAuditAlarm, 1, 12 
TABLE_ENTRY  AccessCheckByTypeResultListAndAuditAlarmByHandle, 1, 13 
TABLE_ENTRY  AddBootEntry, 0, 0 
TABLE_ENTRY  AddDriverEntry, 0, 0 
TABLE_ENTRY  AdjustGroupsToken, 1, 2 
TABLE_ENTRY  AlertResumeThread, 0, 0 
TABLE_ENTRY  AlertThread, 0, 0 
TABLE_ENTRY  AllocateLocallyUniqueId, 0, 0 
TABLE_ENTRY  AllocateUserPhysicalPages, 0, 0 
TABLE_ENTRY  AllocateUuids, 0, 0 
TABLE_ENTRY  AreMappedFilesTheSame, 0, 0 
TABLE_ENTRY  AssignProcessToJobObject, 0, 0 
TABLE_ENTRY  CancelDeviceWakeupRequest, 0, 0 
TABLE_ENTRY  CompactKeys, 0, 0 
TABLE_ENTRY  CompareTokens, 0, 0 
TABLE_ENTRY  CompleteConnectPort, 0, 0 
TABLE_ENTRY  CompressKey, 0, 0 
TABLE_ENTRY  ConnectPort, 1, 4 
TABLE_ENTRY  CreateDebugObject, 0, 0 
TABLE_ENTRY  CreateDirectoryObject, 0, 0 
TABLE_ENTRY  CreateEventPair, 0, 0 
TABLE_ENTRY  CreateIoCompletion, 0, 0 
TABLE_ENTRY  CreateJobObject, 0, 0 
TABLE_ENTRY  CreateJobSet, 0, 0 
TABLE_ENTRY  CreateKeyedEvent, 0, 0 
TABLE_ENTRY  CreateMailslotFile, 1, 4 
TABLE_ENTRY  CreateMutant, 0, 0 
TABLE_ENTRY  CreateNamedPipeFile, 1, 10 
TABLE_ENTRY  CreatePagingFile, 0, 0 
TABLE_ENTRY  CreatePort, 1, 1 
TABLE_ENTRY  CreateProcess, 1, 4 
TABLE_ENTRY  CreateProfile, 1, 5 
TABLE_ENTRY  CreateSemaphore, 1, 1 
TABLE_ENTRY  CreateSymbolicLinkObject, 0, 0 
TABLE_ENTRY  CreateTimer, 0, 0 
TABLE_ENTRY  CreateToken, 1, 9 
TABLE_ENTRY  CreateWaitablePort, 1, 1 
TABLE_ENTRY  DebugActiveProcess, 0, 0 
TABLE_ENTRY  DebugContinue, 0, 0 
TABLE_ENTRY  DeleteAtom, 0, 0 
TABLE_ENTRY  DeleteBootEntry, 0, 0 
TABLE_ENTRY  DeleteDriverEntry, 0, 0 
TABLE_ENTRY  DeleteFile, 0, 0 
TABLE_ENTRY  DeleteKey, 0, 0 
TABLE_ENTRY  DeleteObjectAuditAlarm, 0, 0 
TABLE_ENTRY  DeleteValueKey, 0, 0 
TABLE_ENTRY  DisplayString, 0, 0 
TABLE_ENTRY  EnumerateBootEntries, 0, 0 
TABLE_ENTRY  EnumerateDriverEntries, 0, 0 
TABLE_ENTRY  EnumerateSystemEnvironmentValuesEx, 0, 0 
TABLE_ENTRY  ExtendSection, 0, 0 
TABLE_ENTRY  FilterToken, 1, 2 
TABLE_ENTRY  FlushInstructionCache, 0, 0 
TABLE_ENTRY  FlushKey, 0, 0 
TABLE_ENTRY  FlushVirtualMemory, 0, 0 
TABLE_ENTRY  FlushWriteBuffer, 0, 0 
TABLE_ENTRY  FreeUserPhysicalPages, 0, 0 
TABLE_ENTRY  GetContextThread, 0, 0 
TABLE_ENTRY  GetCurrentProcessorNumber, 0, 0 
TABLE_ENTRY  GetDevicePowerState, 0, 0 
TABLE_ENTRY  GetPlugPlayEvent, 0, 0 
TABLE_ENTRY  GetWriteWatch, 1, 3 
TABLE_ENTRY  ImpersonateAnonymousToken, 0, 0 
TABLE_ENTRY  ImpersonateThread, 0, 0 
TABLE_ENTRY  InitializeRegistry, 0, 0 
TABLE_ENTRY  InitiatePowerAction, 0, 0 
TABLE_ENTRY  IsSystemResumeAutomatic, 0, 0 
TABLE_ENTRY  ListenPort, 0, 0 
TABLE_ENTRY  LoadDriver, 0, 0 
TABLE_ENTRY  LoadKey, 0, 0 
TABLE_ENTRY  LoadKey2, 0, 0 
TABLE_ENTRY  LoadKeyEx, 0, 0 
TABLE_ENTRY  LockFile, 1, 6 
TABLE_ENTRY  LockProductActivationKeys, 0, 0 
TABLE_ENTRY  LockRegistryKey, 0, 0 
TABLE_ENTRY  LockVirtualMemory, 0, 0 
TABLE_ENTRY  MakePermanentObject, 0, 0 
TABLE_ENTRY  MakeTemporaryObject, 0, 0 
TABLE_ENTRY  MapUserPhysicalPages, 0, 0 
TABLE_ENTRY  ModifyBootEntry, 0, 0 
TABLE_ENTRY  ModifyDriverEntry, 0, 0 
TABLE_ENTRY  NotifyChangeDirectoryFile, 1, 5 
TABLE_ENTRY  NotifyChangeKey, 1, 6 
TABLE_ENTRY  NotifyChangeMultipleKeys, 1, 8 
TABLE_ENTRY  OpenEventPair, 0, 0 
TABLE_ENTRY  OpenIoCompletion, 0, 0 
TABLE_ENTRY  OpenJobObject, 0, 0 
TABLE_ENTRY  OpenKeyedEvent, 0, 0 
TABLE_ENTRY  OpenMutant, 0, 0 
TABLE_ENTRY  OpenObjectAuditAlarm, 1, 8 
TABLE_ENTRY  OpenProcessToken, 0, 0 
TABLE_ENTRY  OpenSemaphore, 0, 0 
TABLE_ENTRY  OpenSymbolicLinkObject, 0, 0 
TABLE_ENTRY  OpenThread, 0, 0 
TABLE_ENTRY  OpenTimer, 0, 0 
TABLE_ENTRY  PlugPlayControl, 0, 0 
TABLE_ENTRY  PrivilegeCheck, 0, 0 
TABLE_ENTRY  PrivilegeObjectAuditAlarm, 1, 2 
TABLE_ENTRY  PrivilegedServiceAuditAlarm, 1, 1 
TABLE_ENTRY  PulseEvent, 0, 0 
TABLE_ENTRY  QueryBootEntryOrder, 0, 0 
TABLE_ENTRY  QueryBootOptions, 0, 0 
TABLE_ENTRY  QueryDebugFilterState, 0, 0 
TABLE_ENTRY  QueryDirectoryObject, 1, 3 
TABLE_ENTRY  QueryDriverEntryOrder, 0, 0 
TABLE_ENTRY  QueryEaFile, 1, 5 
TABLE_ENTRY  QueryFullAttributesFile, 0, 0 
TABLE_ENTRY  QueryInformationAtom, 1, 1 
TABLE_ENTRY  QueryInformationJobObject, 1, 1 
TABLE_ENTRY  QueryInformationPort, 1, 1 
TABLE_ENTRY  QueryInstallUILanguage, 0, 0 
TABLE_ENTRY  QueryIntervalProfile, 0, 0 
TABLE_ENTRY  QueryIoCompletion, 1, 1 
TABLE_ENTRY  QueryMultipleValueKey, 1, 2 
TABLE_ENTRY  QueryMutant, 1, 1 
TABLE_ENTRY  QueryOpenSubKeys, 0, 0 
TABLE_ENTRY  QueryOpenSubKeysEx, 0, 0 
TABLE_ENTRY  QueryPortInformationProcess, 0, 0 
TABLE_ENTRY  QueryQuotaInformationFile, 1, 5 
TABLE_ENTRY  QuerySecurityObject, 1, 1 
TABLE_ENTRY  QuerySemaphore, 1, 1 
TABLE_ENTRY  QuerySymbolicLinkObject, 0, 0 
TABLE_ENTRY  QuerySystemEnvironmentValue, 0, 0 
TABLE_ENTRY  QuerySystemEnvironmentValueEx, 1, 1 
TABLE_ENTRY  QueryTimerResolution, 0, 0 
TABLE_ENTRY  RaiseException, 0, 0 
TABLE_ENTRY  RaiseHardError, 1, 2 
TABLE_ENTRY  RegisterThreadTerminatePort, 0, 0 
TABLE_ENTRY  ReleaseKeyedEvent, 0, 0 
TABLE_ENTRY  RemoveProcessDebug, 0, 0 
TABLE_ENTRY  RenameKey, 0, 0 
TABLE_ENTRY  ReplaceKey, 0, 0 
TABLE_ENTRY  ReplyWaitReplyPort, 0, 0 
TABLE_ENTRY  RequestDeviceWakeup, 0, 0 
TABLE_ENTRY  RequestPort, 0, 0 
TABLE_ENTRY  RequestWakeupLatency, 0, 0 
TABLE_ENTRY  ResetEvent, 0, 0 
TABLE_ENTRY  ResetWriteWatch, 0, 0 
TABLE_ENTRY  RestoreKey, 0, 0 
TABLE_ENTRY  ResumeProcess, 0, 0 
TABLE_ENTRY  SaveKey, 0, 0 
TABLE_ENTRY  SaveKeyEx, 0, 0 
TABLE_ENTRY  SaveMergedKeys, 0, 0 
TABLE_ENTRY  SecureConnectPort, 1, 5 
TABLE_ENTRY  SetBootEntryOrder, 0, 0 
TABLE_ENTRY  SetBootOptions, 0, 0 
TABLE_ENTRY  SetContextThread, 0, 0 
TABLE_ENTRY  SetDebugFilterState, 0, 0 
TABLE_ENTRY  SetDefaultHardErrorPort, 0, 0 
TABLE_ENTRY  SetDefaultLocale, 0, 0 
TABLE_ENTRY  SetDefaultUILanguage, 0, 0 
TABLE_ENTRY  SetDriverEntryOrder, 0, 0 
TABLE_ENTRY  SetEaFile, 0, 0 
TABLE_ENTRY  SetHighEventPair, 0, 0 
TABLE_ENTRY  SetHighWaitLowEventPair, 0, 0 
TABLE_ENTRY  SetInformationDebugObject, 1, 1 
TABLE_ENTRY  SetInformationJobObject, 0, 0 
TABLE_ENTRY  SetInformationKey, 0, 0 
TABLE_ENTRY  SetInformationToken, 0, 0 
TABLE_ENTRY  SetIntervalProfile, 0, 0 
TABLE_ENTRY  SetIoCompletion, 1, 1 
TABLE_ENTRY  SetLdtEntries, 1, 2 
TABLE_ENTRY  SetLowEventPair, 0, 0 
TABLE_ENTRY  SetLowWaitHighEventPair, 0, 0 
TABLE_ENTRY  SetQuotaInformationFile, 0, 0 
TABLE_ENTRY  SetSecurityObject, 0, 0 
TABLE_ENTRY  SetSystemEnvironmentValue, 0, 0 
TABLE_ENTRY  SetSystemEnvironmentValueEx, 1, 1 
TABLE_ENTRY  SetSystemInformation, 0, 0 
TABLE_ENTRY  SetSystemPowerState, 0, 0 
TABLE_ENTRY  SetSystemTime, 0, 0 
TABLE_ENTRY  SetThreadExecutionState, 0, 0 
TABLE_ENTRY  SetTimerResolution, 0, 0 
TABLE_ENTRY  SetUuidSeed, 0, 0 
TABLE_ENTRY  SetVolumeInformationFile, 1, 1 
TABLE_ENTRY  ShutdownSystem, 0, 0 
TABLE_ENTRY  SignalAndWaitForSingleObject, 0, 0 
TABLE_ENTRY  StartProfile, 0, 0 
TABLE_ENTRY  StopProfile, 0, 0 
TABLE_ENTRY  SuspendProcess, 0, 0 
TABLE_ENTRY  SuspendThread, 0, 0 
TABLE_ENTRY  SystemDebugControl, 1, 2 
TABLE_ENTRY  TerminateJobObject, 0, 0 
TABLE_ENTRY  TestAlert, 0, 0 
TABLE_ENTRY  TranslateFilePath, 0, 0 
TABLE_ENTRY  UnloadDriver, 0, 0 
TABLE_ENTRY  UnloadKey, 0, 0 
TABLE_ENTRY  UnloadKey2, 0, 0 
TABLE_ENTRY  UnloadKeyEx, 0, 0 
TABLE_ENTRY  UnlockFile, 1, 1 
TABLE_ENTRY  UnlockVirtualMemory, 0, 0 
TABLE_ENTRY  VdmControl, 0, 0 
TABLE_ENTRY  WaitForDebugEvent, 0, 0 
TABLE_ENTRY  WaitForKeyedEvent, 0, 0 
TABLE_ENTRY  WaitHighEventPair, 0, 0 
TABLE_ENTRY  WaitLowEventPair, 0, 0 

TABLE_END 295 

ARGTBL_BEGIN
ARGTBL_ENTRY 0,0,0,20,24,20,4,0 
ARGTBL_ENTRY 0,0,0,0,0,4,4,0 
ARGTBL_ENTRY 8,0,0,4,8,8,4,4 
ARGTBL_ENTRY 20,0,12,0,0,0,4,0 
ARGTBL_ENTRY 8,0,4,0,4,24,28,0 
ARGTBL_ENTRY 4,0,0,20,4,0,0,8 
ARGTBL_ENTRY 8,0,28,0,0,4,24,4 
ARGTBL_ENTRY 0,12,0,0,4,0,8,8 
ARGTBL_ENTRY 0,0,4,0,0,4,4,12 
ARGTBL_ENTRY 0,0,20,16,0,4,4,0 
ARGTBL_ENTRY 0,8,28,4,8,0,48,0 
ARGTBL_ENTRY 4,0,0,0,4,8,0,12 
ARGTBL_ENTRY 8,16,28,28,48,52,0,0 
ARGTBL_ENTRY 8,0,0,0,0,0,0,0 
ARGTBL_ENTRY 0,0,0,0,0,16,0,0 
ARGTBL_ENTRY 0,0,0,0,0,16,0,40 
ARGTBL_ENTRY 0,4,16,20,4,0,0,36 
ARGTBL_ENTRY 4,0,0,0,0,0,0,0 
ARGTBL_ENTRY 0,0,0,0,0,0,0,8 
ARGTBL_ENTRY 0,0,0,0,0,0,0,0 
ARGTBL_ENTRY 0,12,0,0,0,0,0,0 
ARGTBL_ENTRY 0,0,0,0,24,0,0,0 
ARGTBL_ENTRY 0,0,0,0,0,20,24,32 
ARGTBL_ENTRY 0,0,0,0,0,32,0,0 
ARGTBL_ENTRY 0,0,0,0,0,8,4,0 
ARGTBL_ENTRY 0,0,0,12,0,20,0,4 
ARGTBL_ENTRY 4,4,0,0,4,8,4,0 
ARGTBL_ENTRY 0,0,20,4,4,0,0,4 
ARGTBL_ENTRY 0,0,8,0,0,0,0,0 
ARGTBL_ENTRY 0,0,0,0,0,0,0,0 
ARGTBL_ENTRY 0,0,0,20,0,0,0,0 
ARGTBL_ENTRY 0,0,0,0,0,0,0,4 
ARGTBL_ENTRY 0,0,0,0,4,8,0,0 
ARGTBL_ENTRY 0,0,0,4,0,0,0,0 
ARGTBL_ENTRY 0,0,4,0,0,0,0,0 
ARGTBL_ENTRY 0,8,0,0,0,0,0,0 
ARGTBL_ENTRY 0,4,0,0,0,0,0,0 

ARGTBL_END
