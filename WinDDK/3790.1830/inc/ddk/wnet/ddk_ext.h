//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Copyright 1997-2002 OSR, Open Systems Resources, Inc. All rights Reserved.
// 
// Module Name:
// 
//     ddk_ext.h
// 
// Abstract:
// 
//     This module defines the Driver Vefifier Extensions that live
//     as part of the Windows DDK.
// 
// Author:
//    v-markca    6-Jun-2002  Initial Version
//
// Revision History:
//
#include <ntverp.h>

#if (VER_PRODUCTBUILD >= 2600) && !defined(_WDMDDK_)

VOID
DDK_KeAcquireInStackQueuedSpinLock(
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle,
    PVOID F, ULONG L);

VOID
DDK_KeReleaseInStackQueuedSpinLock(
    IN PKLOCK_QUEUE_HANDLE LockHandle,
    PVOID F, ULONG L);

VOID
DDK_KeAcquireInStackQueuedSpinLockAtDpcLevel(
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle,
    PVOID F, ULONG L);

VOID
DDK_KeReleaseInStackQueuedSpinLockFromDpcLevel(
    IN PKLOCK_QUEUE_HANDLE LockHandle,
    PVOID F, ULONG L);

#endif


//
//
//
VOID
NTAPI
DDK_KeInitializeSpinLock (
    IN PKSPIN_LOCK SpinLock,
    PVOID F, ULONG L);

//
//
//
VOID 
DDK_KeAcquireSpinLock(
    IN PKSPIN_LOCK  SpinLock,
    OUT PKIRQL  OldIrql,
    PVOID F, ULONG L);

//
//
//
VOID
DDK_KeReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql,
    PVOID F, ULONG L);

//
//
//
VOID
DDK_KeAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock,
    PVOID F, ULONG L);

//
//
//
VOID
DDK_KeReleaseSpinLockFromDpcLevel (
    IN PKSPIN_LOCK SpinLock,
    PVOID F, ULONG L);

KIRQL
DDK_KeAcquireInterruptSpinLock (
    IN PKINTERRUPT Interrupt,
    PVOID F, ULONG L);

VOID
DDK_KeReleaseInterruptSpinLock (
    IN PKINTERRUPT Interrupt,
    IN KIRQL OldIrql,
    PVOID F, ULONG L);

//
#if _WIN32_WINNT >= 0x0501 

#ifdef KeAcquireInStackQueuedSpinLock
    #undef KeAcquireInStackQueuedSpinLock
#endif
#define KeAcquireInStackQueuedSpinLock(a,b) DDK_KeAcquireInStackQueuedSpinLock(a,b,__FILE__,__LINE__)

#ifdef KeReleaseInStackQueuedSpinLock
    #undef KeReleaseInStackQueuedSpinLock
#endif
#define KeReleaseInStackQueuedSpinLock(a) DDK_KeReleaseInStackQueuedSpinLock(a,__FILE__,__LINE__)

#ifdef KeAcquireInStackQueuedSpinLockAtDpcLevel
    #undef KeAcquireInStackQueuedSpinLockAtDpcLevel
#endif
#define KeAcquireInStackQueuedSpinLockAtDpcLevel(a,b) DDK_KeAcquireInStackQueuedSpinLockAtDpcLevel(a,b,__FILE__,__LINE__)

#ifdef KeReleaseInStackQueuedSpinLockFromDpcLevel
    #undef KeReleaseInStackQueuedSpinLockFromDpcLevel
#endif
#define KeReleaseInStackQueuedSpinLockFromDpcLevel(a) DDK_KeReleaseInStackQueuedSpinLockFromDpcLevel(a,__FILE__,__LINE__)

#endif 

#ifdef KeAcquireSpinLock
    #undef KeAcquireSpinLock
#endif
#define KeAcquireSpinLock(a,b) DDK_KeAcquireSpinLock(a,b,__FILE__,__LINE__)

#ifdef KeReleaseSpinLock
    #undef KeReleaseSpinLock
#endif
#define KeReleaseSpinLock(a,b) DDK_KeReleaseSpinLock(a,b,__FILE__,__LINE__)

#ifdef KeAcquireSpinLockAtDpcLevel
    #undef KeAcquireSpinLockAtDpcLevel
#endif
#define KeAcquireSpinLockAtDpcLevel(a) DDK_KeAcquireSpinLockAtDpcLevel(a,__FILE__,__LINE__)

#ifdef KeReleaseSpinLockFromDpcLevel
    #undef KeReleaseSpinLockFromDpcLevel
#endif
#define KeReleaseSpinLockFromDpcLevel(a) DDK_KeReleaseSpinLockFromDpcLevel(a,__FILE__,__LINE__)

#ifdef KeInitializeSpinLock
    #undef KeInitializeSpinLock
#endif
#define KeInitializeSpinLock(a) DDK_KeInitializeSpinLock(a,__FILE__,__LINE__)

NTSTATUS
DDK_IoConnectInterrupt(
    OUT PKINTERRUPT *InterruptObject,
    IN PKSERVICE_ROUTINE ServiceRoutine,
    IN PVOID ServiceContext,
    IN PKSPIN_LOCK SpinLock OPTIONAL,
    IN ULONG Vector,
    IN KIRQL Irql,
    IN KIRQL SynchronizeIrql,
    IN KINTERRUPT_MODE InterruptMode,
    IN BOOLEAN ShareVector,
    IN KAFFINITY ProcessorEnableMask,
    IN BOOLEAN FloatingSave,
    PVOID F, ULONG L);

//
#ifdef IoConnectInterrupt
    #undef IoConnectInterrupt
#endif
#define IoConnectInterrupt(a,b,c,d,e,f,g,h,i,j,k) DDK_IoConnectInterrupt(a,b,c,d,e,f,g,h,i,j,k,__FILE__,__LINE__)

#if _WIN32_WINNT < 0x0501 
#define PSLIST_ENTRY PSINGLE_LIST_ENTRY
#define SLIST_ENTRY  SINGLE_LIST_ENTRY
#endif

VOID
DDK_ExInitializeSListHead(
    IN PSLIST_HEADER  SListHead,
    PVOID F,ULONG L);

USHORT
DDK_ExQueryDepthSList(
    IN PSLIST_HEADER  SListHead,
    PVOID F, ULONG L);

PSLIST_ENTRY
DDK_ExInterlockedPopEntrySList(
    IN PSLIST_HEADER  ListHead,
    IN PKSPIN_LOCK  Lock,
    PVOID F, ULONG L);

PSLIST_ENTRY
DDK_ExInterlockedPushEntrySList(
    IN PSLIST_HEADER  ListHead,
    IN PSLIST_ENTRY  ListEntry,
    IN PKSPIN_LOCK  Lock,
    PVOID F, ULONG L);

PSLIST_ENTRY
DDK_ExInterlockedFlushSList(
    IN PSLIST_HEADER  ListHead,
    PVOID F, ULONG L);

PSINGLE_LIST_ENTRY
DDK_ExInterlockedPushEntryList(
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PSINGLE_LIST_ENTRY ListEntry,
    IN PKSPIN_LOCK,
    PVOID F, ULONG L);

PSINGLE_LIST_ENTRY
DDK_ExInterlockedPopEntryList(
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PKSPIN_LOCK,
    PVOID F, ULONG L);

VOID
DDK_InitializeListHead(
    IN PLIST_ENTRY  ListHead,
    PVOID F, ULONG L);

VOID
DDK_InsertHeadList(
    IN PLIST_ENTRY  ListHead,
    IN PLIST_ENTRY Entry,
    PVOID F, ULONG L);

VOID
DDK_InsertTailList(
    IN PLIST_ENTRY  ListHead,
    IN PLIST_ENTRY Entry,
    PVOID F, ULONG L);

PLIST_ENTRY
DDK_RemoveHeadList(
    IN PLIST_ENTRY  ListHead,
    PVOID F, ULONG L);

PLIST_ENTRY
DDK_RemoveTailList(
    IN PLIST_ENTRY  ListHead,
    PVOID F, ULONG L);

BOOLEAN
DDK_IsListEmpty(
    IN PLIST_ENTRY  ListHead,
    PVOID F, ULONG L);

PLIST_ENTRY
DDK_ExInterlockedInsertHeadList(
    IN PLIST_ENTRY  ListHead,
    IN PLIST_ENTRY  ListEntry,
    IN PKSPIN_LOCK  Lock,
    PVOID F, ULONG L);

PLIST_ENTRY
DDK_ExInterlockedInsertTailList(
    IN PLIST_ENTRY  ListHead,
    IN PLIST_ENTRY  ListEntry,
    IN PKSPIN_LOCK  Lock,
    PVOID F, ULONG L);

PLIST_ENTRY
DDK_ExInterlockedRemoveHeadList(
    IN PLIST_ENTRY  ListHead,
    IN PKSPIN_LOCK  Lock,
    PVOID F, ULONG L);

#ifdef ExInitializeSListHead
    #undef ExInitializeSListHead
#endif
#define ExInitializeSListHead(ListHead) DDK_ExInitializeSListHead(ListHead,__FILE__,__LINE__)

#ifdef ExQueryDepthSList
    #undef ExQueryDepthSList
#endif
#define ExQueryDepthSList(ListHead) DDK_ExQueryDepthSList(ListHead,__FILE__,__LINE__)

#ifdef ExInterlockedPopEntrySList
    #undef ExInterlockedPopEntrySList
#endif
#define ExInterlockedPopEntrySList(ListHead,Lock) DDK_ExInterlockedPopEntrySList(ListHead,Lock,__FILE__,__LINE__)

#ifdef ExInterlockedPushEntrySList
    #undef ExInterlockedPushEntrySList
#endif
#define ExInterlockedPushEntrySList(ListHead,Entry,Lock) DDK_ExInterlockedPushEntrySList(ListHead,Entry,Lock,__FILE__,__LINE__)

#ifdef ExInterlockedFlushSList
    #undef ExInterlockedFlushSList
#endif
#define ExInterlockedFlushSList(ListHead) DDK_ExInterlockedFlushSList(ListHead,__FILE__,__LINE__)


//
// Doubly Linked List Routines
//
#ifdef InitializeListHead
    #undef InitializeListHead
#endif
#define InitializeListHead(ListHead) DDK_InitializeListHead(ListHead,__FILE__,__LINE__)

#ifdef InsertHeadList
    #undef InsertHeadList
#endif
#define InsertHeadList(ListHead,Entry) DDK_InsertHeadList(ListHead,Entry,__FILE__,__LINE__)

#ifdef InsertTailList
    #undef InsertTailList
#endif
#define InsertTailList(ListHead,Entry) DDK_InsertTailList(ListHead,Entry,__FILE__,__LINE__)

#ifdef RemoveHeadList
    #undef RemoveHeadList
#endif
#define RemoveHeadList(ListHead) DDK_RemoveHeadList(ListHead,__FILE__,__LINE__)

#ifdef RemoveTailList
    #undef RemoveTailList
#endif
#define RemoveTailList(ListHead) DDK_RemoveTailList(ListHead,__FILE__,__LINE__)

#ifdef IsListEmpty
    #undef IsListEmpty
#endif
#define IsListEmpty(ListHead) DDK_IsListEmpty(ListHead,__FILE__,__LINE__)

//
// Interlocked List Routines
//

#ifdef ExInterlockedInsertHeadList
    #undef ExInterlockedInsertHeadList
#endif
#define ExInterlockedInsertHeadList(_ListHead,_ListEntry,_SpinLock) DDK_ExInterlockedInsertHeadList(_ListHead,_ListEntry,_SpinLock,__FILE__,__LINE__)

#ifdef ExInterlockedInsertTailList
    #undef ExInterlockedInsertTailList
#endif
#define ExInterlockedInsertTailList(_ListHead,_ListEntry,_SpinLock) DDK_ExInterlockedInsertTailList(_ListHead,_ListEntry,_SpinLock,__FILE__,__LINE__)

#ifdef ExInterlockedRemoveHeadList
    #undef ExInterlockedRemoveHeadList
#endif
#define ExInterlockedRemoveHeadList(_ListHead,_SpinLock) DDK_ExInterlockedRemoveHeadList(_ListHead,_SpinLock,__FILE__,__LINE__)

//
// Lookaside List Routines
//
VOID
NTAPI
DDK_ExInitializePagedLookasideList (
    IN PPAGED_LOOKASIDE_LIST Lookaside,
    IN PALLOCATE_FUNCTION Allocate,
    IN PFREE_FUNCTION Free,
    IN ULONG Flags,
    IN SIZE_T Size,
    IN ULONG Tag,
    IN USHORT Depth,
    PVOID F, ULONG L);

VOID
NTAPI
DDK_ExDeletePagedLookasideList (
    IN PPAGED_LOOKASIDE_LIST Lookaside,
    PVOID F, ULONG L);

VOID
NTAPI
DDK_ExInitializeNPagedLookasideList (
    IN PNPAGED_LOOKASIDE_LIST Lookaside,
    IN PALLOCATE_FUNCTION Allocate,
    IN PFREE_FUNCTION Free,
    IN ULONG Flags,
    IN SIZE_T Size,
    IN ULONG Tag,
    IN USHORT Depth,
    PVOID F, ULONG L);


VOID
NTAPI
DDK_ExDeleteNPagedLookasideList (
    IN PNPAGED_LOOKASIDE_LIST Lookaside,
    PVOID F, ULONG L);

PVOID
NTAPI
DDK_ExAllocateFromPagedLookasideList(
    IN PPAGED_LOOKASIDE_LIST Lookaside,
    PVOID F, ULONG L);


VOID
NTAPI
DDK_ExFreeToPagedLookasideList(
    IN PPAGED_LOOKASIDE_LIST Lookaside,
    IN PVOID Entry,
    PVOID F, ULONG L);

PVOID
NTAPI
DDK_ExAllocateFromNPagedLookasideList(
    IN PNPAGED_LOOKASIDE_LIST Lookaside,
    PVOID F, ULONG L);

VOID
NTAPI
DDK_ExFreeToNPagedLookasideList(
    IN PNPAGED_LOOKASIDE_LIST Lookaside,
    IN PVOID Entry,
    PVOID F, ULONG L);


//
#ifdef ExInitializePagedLookasideList
    #undef ExInitializePagedLookasideList
#endif
#define ExInitializePagedLookasideList(a,b,c,d,e,f,g) DDK_ExInitializePagedLookasideList(a,b,c,d,e,f,g,__FILE__,__LINE__)

#ifdef ExDeletePagedLookasideList
    #undef ExDeletePagedLookasideList
#endif
#define ExDeletePagedLookasideList(a) DDK_ExDeletePagedLookasideList(a,__FILE__,__LINE__)

#ifdef ExInitializeNPagedLookasideList
    #undef ExInitializeNPagedLookasideList
#endif
#define ExInitializeNPagedLookasideList(a,b,c,d,e,f,g) DDK_ExInitializeNPagedLookasideList(a,b,c,d,e,f,g,__FILE__,__LINE__)

#ifdef ExDeleteNPagedLookasideList
    #undef ExDeleteNPagedLookasideList
#endif
#define ExDeleteNPagedLookasideList(a) DDK_ExDeleteNPagedLookasideList(a,__FILE__,__LINE__)

#ifdef ExAllocateFromPagedLookasideList
    #undef ExAllocateFromPagedLookasideList
#endif
#define ExAllocateFromPagedLookasideList(a) DDK_ExAllocateFromPagedLookasideList(a,__FILE__,__LINE__)

#ifdef ExFreeToPagedLookasideList
    #undef ExFreeToPagedLookasideList
#endif
#define ExFreeToPagedLookasideList(a,b) DDK_ExFreeToPagedLookasideList(a,b,__FILE__,__LINE__)

#ifdef ExAllocateFromNPagedLookasideList
    #undef ExAllocateFromNPagedLookasideList
#endif
#define ExAllocateFromNPagedLookasideList(a) DDK_ExAllocateFromNPagedLookasideList(a,__FILE__,__LINE__)

#ifdef ExFreeToNPagedLookasideList
    #undef ExFreeToNPagedLookasideList
#endif
#define ExFreeToNPagedLookasideList(a,b) DDK_ExFreeToNPagedLookasideList(a,b,__FILE__,__LINE__)

//IoGetCurrentIrpStackLocation;
PIO_STACK_LOCATION 
DDK_IoGetCurrentIrpStackLocation(
    IN PIRP  Irp,
    PVOID F, ULONG L);
 
//IoGetNextIrpStackLocation; 
PIO_STACK_LOCATION 
DDK_IoGetNextIrpStackLocation(
    IN PIRP  Irp,
    PVOID F, ULONG L);

//IoMarkIrpPending; 
VOID 
DDK_IoMarkIrpPending(
    IN OUT PIRP  Irp,
    PVOID F, ULONG L);

//IoSetCancelRoutine; 
PDRIVER_CANCEL 
DDK_IoSetCancelRoutine(
    IN PIRP  Irp,
    IN PDRIVER_CANCEL  CancelRoutine,
    PVOID F, ULONG L);

//IoSetCompletionRoutine; 
VOID 
DDK_IoSetCompletionRoutine(
    IN PIRP  Irp,
    IN PIO_COMPLETION_ROUTINE  CompletionRoutine,
    IN PVOID  Context,
    IN BOOLEAN    InvokeOnSuccess,
    IN BOOLEAN  InvokeOnError,
    IN BOOLEAN  InvokeOnCancel,
    PVOID F, ULONG L);

NTSTATUS
DDK_IoSetCompletionRoutineEx(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PIRP  Irp,
    IN PIO_COMPLETION_ROUTINE  CompletionRoutine,
    IN PVOID  Context,
    IN BOOLEAN    InvokeOnSuccess,
    IN BOOLEAN  InvokeOnError,
    IN BOOLEAN  InvokeOnCancel,
    PVOID F, ULONG L);


//IoSetNextIrpStackLocation; 
VOID 
DDK_IoSetNextIrpStackLocation(
    IN OUT PIRP  Irp,
    PVOID F, ULONG L);

//IoCopyCurrentIrpStackLocationToNext; 
VOID 
DDK_IoCopyCurrentIrpStackLocationToNext(
    IN PIRP  Irp,
    PVOID F, ULONG L);

//IoSkipCurrentIrpStackLocation
VOID 
DDK_IoSkipCurrentIrpStackLocation(
    IN PIRP  Irp,
    PVOID F, ULONG L);

//
#ifdef IoGetCurrentIrpStackLocation
    #undef IoGetCurrentIrpStackLocation
#endif
#define IoGetCurrentIrpStackLocation(a) DDK_IoGetCurrentIrpStackLocation(a,__FILE__,__LINE__)

#ifdef IoGetNextIrpStackLocation
    #undef IoGetNextIrpStackLocation
#endif
#define IoGetNextIrpStackLocation(a) DDK_IoGetNextIrpStackLocation(a,__FILE__,__LINE__)

#ifdef IoMarkIrpPending
    #undef IoMarkIrpPending
#endif
#define IoMarkIrpPending(a) DDK_IoMarkIrpPending(a,__FILE__,__LINE__)

#ifdef IoSetCancelRoutine
    #undef IoSetCancelRoutine
#endif
#define IoSetCancelRoutine(a,b) DDK_IoSetCancelRoutine(a,b,__FILE__,__LINE__)

#ifdef IoSetCompletionRoutine
    #undef IoSetCompletionRoutine
#endif
#define IoSetCompletionRoutine(a,b,c,d,e,f) DDK_IoSetCompletionRoutine(a,b,c,d,e,f,__FILE__,__LINE__)

#ifdef IoSetCompletionRoutineEx
    #undef IoSetCompletionRoutineEx
#endif
#define IoSetCompletionRoutineEx(a,b,c,d,e,f,g) DDK_IoSetCompletionRoutineEx(a,b,c,d,e,f,g,__FILE__,__LINE__)

#ifdef IoSetNextIrpStackLocation
    #undef IoSetNextIrpStackLocation
#endif
#define IoSetNextIrpStackLocation(a) DDK_IoSetNextIrpStackLocation(a,__FILE__,__LINE__)

#ifdef IoCopyCurrentIrpStackLocationToNext
    #undef IoCopyCurrentIrpStackLocationToNext
#endif
#define IoCopyCurrentIrpStackLocationToNext(a) DDK_IoCopyCurrentIrpStackLocationToNext(a,__FILE__,__LINE__)

#ifdef IoSkipCurrentIrpStackLocation
    #undef IoSkipCurrentIrpStackLocation
#endif
#define IoSkipCurrentIrpStackLocation(a) DDK_IoSkipCurrentIrpStackLocation(a,__FILE__,__LINE__)



//
//
PVOID
NTAPI
DDK_ExAllocatePool(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    PVOID F, ULONG L);

#ifdef ExAllocatePool
    #undef ExAllocatePool
#endif
#define ExAllocatePool(a, b) DDK_ExAllocatePool(a,b,__FILE__,__LINE__)

PVOID
NTAPI
DDK_ExAllocatePoolWithQuota(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    PVOID F, ULONG L);
#ifdef ExAllocatePoolWithQuota
    #undef ExAllocatePoolWithQuota
#endif
#define ExAllocatePoolWithQuota(a,b) DDK_ExAllocatePoolWithQuota(a,b,__FILE__,__LINE__)

PVOID
NTAPI
DDK_ExAllocatePoolWithTag(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    PVOID F, ULONG L);

#ifdef ExAllocatePoolWithTag
    #undef ExAllocatePoolWithTag
#endif
#define ExAllocatePoolWithTag(a,b,c) DDK_ExAllocatePoolWithTag(a,b,c,__FILE__,__LINE__)

PVOID
NTAPI
DDK_ExAllocatePoolWithTagPriority(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN EX_POOL_PRIORITY Priority,
    PVOID F, ULONG L);

#ifdef ExAllocatePoolWithTagPriority
    #undef ExAllocatePoolWithTagPriority
#endif
#define ExAllocatePoolWithTagPriority(a,b,c,d) DDK_ExAllocatePoolWithTagPriority(a,b,c,d,__FILE__,__LINE__)

PVOID
NTAPI
DDK_ExAllocatePoolWithQuotaTag(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    PVOID F, ULONG L);

#ifdef ExAllocatePoolWithQuotaTag
    #undef ExAllocatePoolWithQuotaTag
#endif
#define ExAllocatePoolWithQuotaTag(a,b,c) DDK_ExAllocatePoolWithQuotaTag(a,b,c,__FILE__,__LINE__)

VOID
NTAPI
DDK_ExFreePool(
    IN PVOID P,
    PVOID F, ULONG L);
#ifdef ExFreePool
    #undef ExFreePool
#endif
#define ExFreePool(a) DDK_ExFreePool(a,__FILE__,__LINE__) 

VOID
NTAPI
DDK_ExFreePoolWithTag(
    IN PVOID P,
    IN ULONG Tag,
    PVOID F, ULONG L);

#ifdef ExFreePoolWithTag
    #undef ExFreePoolWithTag
#endif
#define ExFreePoolWithTag(a,b) DDK_ExFreePoolWithTag(a,b,__FILE__,__LINE__)


