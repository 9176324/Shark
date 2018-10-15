/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    exp.h

Abstract:

    This module contains the private (internal) header file for the
    executive.

--*/

#ifndef _EXP_
#define _EXP_

#pragma warning(disable:4214)   // bit field types other than int
#pragma warning(disable:4201)   // nameless struct/union
#pragma warning(disable:4127)   // condition expression is constant
#pragma warning(disable:4115)   // named type definition in parentheses
#pragma warning(disable:4206)   // translation unit empty
#pragma warning(disable:4706)   // assignment within conditional
#pragma warning(disable:4324)   // structure was padded
#pragma warning(disable:4328)   // greater alignment than needed
#pragma warning(disable:4054)   // cast of function pointer to PVOID

#include "ntos.h"
#include "zwapi.h"
#define NOEXTAPI
#include "wdbgexts.h"
#include "ntdbg.h"
#include "ki.h"
#include "stdio.h"
#include "pool.h"


//
// Executive information initialization structure
//

typedef struct {
    PCALLBACK_OBJECT    *CallBackObject;
    PWSTR               CallbackName;
} EXP_INITIALIZE_GLOBAL_CALLBACKS;

//
// Executive macros
//
//

// This macro returns the address of a field given the type of the structure, pointer to
// the base of the structure and the name of the field whose address is to be computed.
// This returns the same value as &Base->Member
//

#define EX_FIELD_ADDRESS(Type, Base, Member) ((PUCHAR)Base + FIELD_OFFSET(Type, Member))

//
// This macro is used like a C for loop and iterates over all the entries in the list.
//
// _Type - Name of the data type for each element in the list
// _Link - Name of the field of type LIST_ENTRY used to link the elements.
// _Head - Pointer to the head of the list.
// _Current - Points to the iterated element.
//
// Example:     EX_FOR_EACH_IN_LIST(SYSTEM_FIRMWARE_TABLE_HANDLER_NODE,
//                                  FirmwareTableProviderList,
//                                  &ExpFirmwareTableProviderListHead,
//                                  HandlerListCurrent) {
//
//                  /* Do something with Current */
//
//              }
//

#define EX_FOR_EACH_IN_LIST(_Type, _Link, _Head, _Current)                                             \
    for((_Current) = CONTAINING_RECORD((_Head)->Flink, _Type, _Link);                                   \
       (_Head) != (PLIST_ENTRY)EX_FIELD_ADDRESS(_Type, _Current, _Link);                               \
       (_Current) = CONTAINING_RECORD(((PLIST_ENTRY)EX_FIELD_ADDRESS(_Type, _Current, _Link))->Flink,  \
                                     _Type,                                                          \
                                     _Link)                                                          \
       )


//
// Executive object and other initialization function definitions.
//

NTSTATUS
ExpWorkerInitialization (
    VOID
    );

BOOLEAN
ExpEventInitialization (
    VOID
    );

BOOLEAN
ExpEventPairInitialization (
    VOID
    );

BOOLEAN
ExpMutantInitialization (
    VOID
    );

BOOLEAN
ExpSemaphoreInitialization (
    VOID
    );

BOOLEAN
ExpTimerInitialization (
    VOID
    );

BOOLEAN
ExpWin32Initialization (
    VOID
    );

BOOLEAN
ExpResourceInitialization (
    VOID
    );

PVOID
ExpCheckForResource (
    IN PVOID p,
    IN SIZE_T Size
    );

BOOLEAN
ExpInitSystemPhase0 (
    VOID
    );

BOOLEAN
ExpInitSystemPhase1 (
    VOID
    );

BOOLEAN
ExpProfileInitialization (
    );

BOOLEAN
ExpUuidInitialization (
    );

VOID
ExpProfileDelete (
    IN PVOID Object
    );


BOOLEAN
ExpInitializeCallbacks (
    VOID
    );

BOOLEAN
ExpSysEventInitialization (
    VOID
    );

VOID
ExpCheckSystemInfoWork (
    IN PVOID Context
    );

NTSTATUS
ExpKeyedEventInitialization (
    VOID
    );

VOID
ExpCheckSystemInformation (
    PVOID Context,
    PVOID InformationClass,
    PVOID Argument2
    );


VOID
ExpTimeZoneWork (
    IN PVOID Context
    );

VOID
ExpTimeRefreshDpcRoutine (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
ExpTimeRefreshWork (
    IN PVOID Context
    );

VOID
ExpTimeZoneDpcRoutine (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
ExInitializeTimeRefresh (
    VOID
    );

VOID
ExpCheckForWorker (
    IN PVOID p,
    IN SIZE_T Size
    );

VOID
ExpShutdownWorkerThreads (
    VOID
    );

BOOLEAN
ExpSystemOK (
    IN int     safelevel,
    IN BOOLEAN evalflag
    );

#if defined(_WIN64)

NTSTATUS
ExpGetSystemEmulationProcessorInformation (
    OUT PSYSTEM_PROCESSOR_INFORMATION ProcessorInformation
    );

#endif

NTSTATUS
ExpGetSystemProcessorInformation (
    OUT PSYSTEM_PROCESSOR_INFORMATION ProcessorInformation
    );

NTSTATUS
ExApplyCodePatch (
    IN PVOID PatchInfoPtr,
    IN SIZE_T PatchSize
    );

VOID
ExpInitializePushLocks (
    VOID
    );

ULONG ExpNtExpirationData[3];
BOOLEAN ExpSetupModeDetected;
LARGE_INTEGER ExpSetupSystemPrefix;
HANDLE ExpSetupKey;
BOOLEAN ExpSystemPrefixValid;

#ifdef _PNP_POWER_

extern WORK_QUEUE_ITEM ExpCheckSystemInfoWorkItem;
extern LONG ExpCheckSystemInfoBusy;
extern const WCHAR ExpWstrSystemInformation[];
extern const WCHAR ExpWstrSystemInformationValue[];

#endif // _PNP_POWER_

extern const WCHAR ExpWstrCallback[];
extern const EXP_INITIALIZE_GLOBAL_CALLBACKS  ExpInitializeCallback[];

extern BOOLEAN ExpInTextModeSetup;
extern BOOLEAN ExpTooLateForErrors;

extern FAST_MUTEX ExpEnvironmentLock;
extern ERESOURCE ExpKeyManipLock;

extern GENERAL_LOOKASIDE ExpSmallPagedPoolLookasideLists[POOL_SMALL_LISTS];
extern GENERAL_LOOKASIDE ExpSmallNPagedPoolLookasideLists[POOL_SMALL_LISTS];

extern LIST_ENTRY ExNPagedLookasideListHead;
extern KSPIN_LOCK ExNPagedLookasideLock;
extern LIST_ENTRY ExPagedLookasideListHead;
extern KSPIN_LOCK ExPagedLookasideLock;
extern LIST_ENTRY ExPoolLookasideListHead;
extern PEPROCESS ExpDefaultErrorPortProcess;
extern HANDLE ExpDefaultErrorPort;
extern HANDLE ExpProductTypeKey;
extern PVOID ExpControlKey[2];
extern LIST_ENTRY ExpFirmwareTableProviderListHead;
extern ERESOURCE  ExpFirmwareTableResource;

#endif // _EXP_

