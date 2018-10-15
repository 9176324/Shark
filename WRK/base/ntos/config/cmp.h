/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmp.h

Abstract:

    This module contains the private (internal) header file for the
    configuration manager.

--*/

#ifndef _CMP_
#define _CMP_

#pragma warning(disable:4214)   // bit field types other than int
#pragma warning(disable:4201)   // nameless struct/union
#pragma warning(disable:4324)   // alignment sensitive to declspec
#pragma warning(disable:4127)   // condition expression is constant
#pragma warning(disable:4115)   // named type definition in parentheses
#pragma warning(disable:4706)   // assignment within conditional expression

#define     _64K    (64L*1024L)   //64K
#define     _256K   (256L*1024L)  //256K
#define		IO_BUFFER_SIZE  _64K  //64K

#include "ntos.h"
#include "hive.h"
#include "wchar.h"
#include "zwapi.h"
#include <stdio.h>
#include <profiles.h>

// bugcheck description and defines
#include "cmpbug.h"

#include "kddll.h"

// CM data structure declarations
// file location: \nt\private\ntos\inc
#include "cmdata.h"


#define CmKdPrintEx(_x_)  KdPrintEx(_x_)

//
// this constant defines the size of a Cc view that is mapped -in every time a cell
// is accessed; It can be any power of 2, no less than 16K and no bigger than 256K
//
#define     CM_VIEW_SIZE            16L*1024L  //16K

//
// control the granularity the primary file grows;
// Warning: this should be multiple of 4K (HBLOCK_SIZE) !!!
//
#define     CM_FILE_GROW_INCREMENT  256L*1024L  //256K

//
// this controls the maximum address space allowed per hive. It should be specified in
// multiples of 256K
//
//  4  means 1   MB
//  6  means 1.5 MB
//  12 means 3   MB
//  .....
//
#define     MAX_MB_PER_HIVE     16          // 4MB


#define MAX_NAME    128

#define CmpRemoveEntryList(a) RemoveEntryList(a)
#define CmpClearListEntry(a) //nothing
#define CmpIsListEmpty(a) IsListEmpty(a)

extern PCM_TRACE_NOTIFY_ROUTINE CmpTraceRoutine;

VOID
CmpWmiDumpKcb(
    PCM_KEY_CONTROL_BLOCK       kcb
);

#define CmpWmiFireEvent(Status,Kcb,ElapsedTime,Index,KeyName,Type)  \
{                                                               \
    PCM_TRACE_NOTIFY_ROUTINE TraceRoutine = CmpTraceRoutine;        \
    if( TraceRoutine != NULL ) {                                    \
        (*TraceRoutine)(Status,Kcb,ElapsedTime,Index,KeyName,Type); \
    }                                                               \
}

#define StartWmiCmTrace()\
    LARGE_INTEGER   StartSystemTime = {0};\
    LARGE_INTEGER   EndSystemTime;\
    PVOID           HookKcb = NULL;\
    if (CmpTraceRoutine) {\
        PerfTimeStamp(StartSystemTime); \
    }


#define EndWmiCmTrace(Status,Index,KeyName,Type)\
    if (CmpTraceRoutine) {\
        PerfTimeStamp(EndSystemTime); \
        CmpWmiFireEvent(Status,HookKcb,EndSystemTime.QuadPart - StartSystemTime.QuadPart,Index,KeyName,Type);\
    }

#define HookKcbForWmiCmTrace(KeyBody) \
    if (CmpTraceRoutine) {\
        if(KeyBody) {\
            HookKcb = KeyBody->KeyControlBlock;\
        }\
    }

#define HookKcbFromHandleForWmiCmTrace(KeyHandle) \
    if (CmpTraceRoutine && (KeyHandle)) {\
        PCM_KEY_BODY KeyBody;\
        NTSTATUS status;\
        status = ObReferenceObjectByHandle(\
                    KeyHandle,\
                    0,\
                    CmpKeyObjectType,\
                    KeGetPreviousMode(),\
                    (PVOID *)(&KeyBody),\
                    NULL\
                    );\
        if (NT_SUCCESS(status)) {\
            HookKcb = KeyBody->KeyControlBlock;\
            ObDereferenceObject((PVOID)KeyBody);\
        }\
    }

#define CmpTraceKcbCreate(kcb) \
    if (CmpTraceRoutine) {\
        CmpWmiDumpKcb(kcb);\
    }

#define CmpMakeSpecialPoolReadOnly(a)  //nothing
#define CmpMakeSpecialPoolReadWrite(a)  //nothing
#define CmpMakeValueCacheReadOnly(a,b) //nothing
#define CmpMakeValueCacheReadWrite(a,b) //nothing

#define HvpChangeBinAllocation(a,b) //nothing
#define HvpMarkBinReadWrite(a,b) //nothing
#define CmpMarkAllBinsReadOnly(a) //nothing

#ifdef POOL_TAGGING
//
// Pool Tag
//
#define  CM_POOL_TAG        '  MC'
#define  CM_KCB_TAG         'bkMC'
#define  CM_POSTBLOCK_TAG   'bpMC'
#define  CM_NOTIFYBLOCK_TAG 'bnMC'
#define  CM_POSTEVENT_TAG   'epMC'
#define  CM_POSTAPC_TAG     'apMC'
#define  CM_MAPPEDVIEW_TAG  'wVMC'
#define  CM_SECCACHE_TAG    'cSMC'
#define  CM_DELAYCLOSE_TAG  'cDMC'
#define  CM_STASHBUFFER_TAG 'bSMC'
#define  CM_HVBIN_TAG       'bHMC'
#define  CM_ALLOCATE_TAG    'lAMC'

//
// Find leaks
//
#define  CM_FIND_LEAK_TAG1    ' 1MC'
#define  CM_FIND_LEAK_TAG2    ' 2MC'
#define  CM_FIND_LEAK_TAG3    ' 3MC'
#define  CM_FIND_LEAK_TAG4    ' 4MC'
#define  CM_FIND_LEAK_TAG5    ' 5MC'
#define  CM_FIND_LEAK_TAG6    ' 6MC'
#define  CM_FIND_LEAK_TAG7    ' 7MC'
#define  CM_FIND_LEAK_TAG8    ' 8MC'
#define  CM_FIND_LEAK_TAG9    ' 9MC'
#define  CM_FIND_LEAK_TAG10    '01MC'
#define  CM_FIND_LEAK_TAG11    '11MC'
#define  CM_FIND_LEAK_TAG12    '21MC'
#define  CM_FIND_LEAK_TAG13    '31MC'
#define  CM_FIND_LEAK_TAG14    '41MC'
#define  CM_FIND_LEAK_TAG15    '51MC'
#define  CM_FIND_LEAK_TAG16    '61MC'
#define  CM_FIND_LEAK_TAG17    '71MC'
#define  CM_FIND_LEAK_TAG18    '81MC'
#define  CM_FIND_LEAK_TAG19    '91MC'
#define  CM_FIND_LEAK_TAG20    '02MC'
#define  CM_FIND_LEAK_TAG21    '12MC'
#define  CM_FIND_LEAK_TAG22    '22MC'
#define  CM_FIND_LEAK_TAG23    '32MC'
#define  CM_FIND_LEAK_TAG24    '42MC'
#define  CM_FIND_LEAK_TAG25    '52MC'
#define  CM_FIND_LEAK_TAG26    '62MC'
#define  CM_FIND_LEAK_TAG27    '72MC'
#define  CM_FIND_LEAK_TAG28    '82MC'
#define  CM_FIND_LEAK_TAG29    '92MC'
#define  CM_FIND_LEAK_TAG30    '03MC'
#define  CM_FIND_LEAK_TAG31    '13MC'
#define  CM_FIND_LEAK_TAG32    '23MC'
#define  CM_FIND_LEAK_TAG33    '33MC'
#define  CM_FIND_LEAK_TAG34    '43MC'
#define  CM_FIND_LEAK_TAG35    '53MC'
#define  CM_FIND_LEAK_TAG36    '63MC'
#define  CM_FIND_LEAK_TAG37    '73MC'
#define  CM_FIND_LEAK_TAG38    '83MC'
#define  CM_FIND_LEAK_TAG39    '93MC'
#define  CM_FIND_LEAK_TAG40    '04MC'
#define  CM_FIND_LEAK_TAG41    '14MC'
#define  CM_FIND_LEAK_TAG42    '24MC'
#define  CM_FIND_LEAK_TAG43    '34MC'
#define  CM_FIND_LEAK_TAG44    '44MC'
#define  CM_FIND_LEAK_TAG45    '54MC'
#define  CM_FIND_LEAK_TAG46    '64MC'

#ifdef _WANT_MACHINE_IDENTIFICATION

#define CM_PARSEINI_TAG 'ipMC'
#define CM_GENINST_TAG  'igMC'

#endif

//
// Extra Tags for cache.
// We may want to merge these tags later.
//
#define  CM_CACHE_VALUE_INDEX_TAG 'IVMC'
#define  CM_CACHE_VALUE_TAG       'aVMC'
#define  CM_CACHE_INDEX_TAG       'nIMC'
#define  CM_CACHE_VALUE_DATA_TAG  'aDMC'
#define  CM_NAME_TAG              'bNMC'


#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,CM_POOL_TAG)
#define ExAllocatePoolWithQuota(a,b) ExAllocatePoolWithQuotaTag(a,b,CM_POOL_TAG)

PVOID
CmpAllocateTag(
    ULONG   Size,
    BOOLEAN UseForIo,
    ULONG   Tag
    );
#else
#define CmpAllocateTag(a,b,c) CmpAllocate(a,b,c)
#endif

#define CmRetryExAllocatePoolWithTag(a,b,c,Result)  \
    {                                               \
        ULONG   RetryCount = 10;                    \
        do {                                        \
            Result = ExAllocatePoolWithTag(a,b,c);  \
        } while ((!Result) && (RetryCount--));      \
    }
    

//
// A variable so can turn on/off certain performance features.
//
extern const ULONG CmpCacheOnFlag;

#define CM_CACHE_FAKE_KEY  0x00000001      // Create Fake key KCB

//
// This is \REGISTRY
//
extern HANDLE CmpRegistryRootHandle;

//
// Logging: remember, first 4 levels (0-3) are reserved system-wide
//
#define CML_BUGCHECK    4   // fatal errors
#define CML_EXCEPTION   5   // all exception's
#define CML_NTAPI       6   // NtApi calls
#define CML_NTAPI_ARGS  7   // NtApi parameters
#define CML_CM          8   // Cm level, general
#define CML_NOTIFY      9   // Notify level, general
#define CML_HIVE        10  // Hv level, general
#define CML_IO          11  // IO level
#define CML_SEC         12  // Security level
#define CML_INIT        13  // Init level, general
#define CML_INDEX       14  // Index level, general
#define CML_BIN_MAP     15  // bin mapping level
#define CML_FREECELL    16  // Free cell hints
#define CML_POOL        17  // Pool
#define CML_LOCKING     18  // Lock/unlock level
#define CML_FLOW        19  // General flow
#define CML_PARSE       20  // Parse algorithm
#define CML_SAVRES      21  // SavRes operations
#define CML_CHECK_HIVE  23  // check hive spew


#define REGCHECKING 1

#if DBG

#if REGCHECKING
#define DCmCheckRegistry(a) if(HvHiveChecking) ASSERT(CmCheckRegistry(a, CM_CHECK_REGISTRY_HIVE_CHECK) == 0)
#else
#define DCmCheckRegistry(a)
#endif

#else
#define DCmCheckRegistry(a)
#endif

#define LogKCBReference(kcb,reason) //nothing
#define CmpCheckIfResourceOwned() //nothing

#define BEGIN_LOCK_CHECKPOINT
#define END_LOCK_CHECKPOINT

extern BOOLEAN CmpSpecialBootCondition;

#if DBG
#define ASSERT_CM_LOCK_OWNED() \
    ASSERT( (CmpSpecialBootCondition == TRUE) || (CmpTestRegistryLock() == TRUE) )
#define ASSERT_CM_LOCK_OWNED_EXCLUSIVE() \
    ASSERT((CmpSpecialBootCondition == TRUE) || (CmpTestRegistryLockExclusive() == TRUE) )
#define ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive) \
    ASSERT((CmpSpecialBootCondition == TRUE) || (CmpTestRegistryLockExclusive() == TRUE) || (Hive->ReleaseCellRoutine == NULL) )
#define ASSERT_CM_LOCK_OWNED_OR_HIVE_LOADING(_CmHive_) \
    ASSERT( (CmpSpecialBootCondition == TRUE) || (((PCMHIVE)(_CmHive_))->HiveIsLoading) || (CmpTestRegistryLock() == TRUE) )
#define ASSERT_CM_LOCK_OWNED_EXCLUSIVE_OR_HIVE_LOADING(_CmHive_) \
    ASSERT( (CmpSpecialBootCondition == TRUE) || (((PCMHIVE)(_CmHive_))->HiveIsLoading) || (CmpTestRegistryLockExclusive() == TRUE) )
#define ASSERT_CM_LOCK_NOT_OWNED() \
    ASSERT( (CmpTestRegistryLock() == FALSE)  && (CmpTestRegistryLockExclusive() == FALSE) )
#else
#define ASSERT_CM_LOCK_OWNED()
#define ASSERT_CM_LOCK_OWNED_EXCLUSIVE()
#define ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive)
#define ASSERT_CM_LOCK_OWNED_OR_HIVE_LOADING(_CmHive_) 
#define ASSERT_CM_LOCK_OWNED_EXCLUSIVE_OR_HIVE_LOADING(_CmHive_) 
#define ASSERT_CM_LOCK_NOT_OWNED() 
#endif

#if DBG
#define ASSERT_PASSIVE_LEVEL()                                              \
    {                                                                       \
        KIRQL   Irql;                                                       \
        Irql = KeGetCurrentIrql();                                          \
        if( KeGetCurrentIrql() != PASSIVE_LEVEL ) {                         \
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"ASSERT_PASSIVE_LEVEL failed ... Irql = %lu\n",Irql);  \
            ASSERT( FALSE );                                                \
        }                                                                   \
    }
#else
#define ASSERT_PASSIVE_LEVEL()
#endif

#define VALIDATE_CELL_MAP(LINE,Map,Hive,Address)                                                    \
    if( Map == NULL ) {                                                                             \
            CM_BUGCHECK (REGISTRY_ERROR,BAD_CELL_MAP,(ULONG_PTR)(Hive),(ULONG)(Address),(ULONG)(LINE)) ;      \
    }

#if DBG
VOID
SepDumpSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSZ TitleString
    );

extern BOOLEAN SepDumpSD;

#define CmpDumpSecurityDescriptor(x,y) \
        { \
            SepDumpSD=TRUE;     \
            SepDumpSecurityDescriptor(x, y);  \
            SepDumpSD=FALSE;    \
        }
#else

#define CmpDumpSecurityDescriptor(x,y)

#endif


//
// misc stuff
//

extern  UNICODE_STRING  CmRegistrySystemCloneName;

//
// Determines whether the Current Control Set used during booting
// is cloned in order to fully preserve it for being saved
// as the LKG Control Set.
//

#define     CM_NUMBER_OF_MACHINE_HIVES  6

#define NUMBER_TYPES (MaximumType + 1)

#define CM_WRAP_LIMIT               0x7fffffff


//
// Tuning and control constants
//
#define CM_MAX_STASH           1024*1024        // If size of data for a set
                                                // is bigger than this,

#define CM_MAX_REASONABLE_VALUES    100         // If number of values for a
                                                // key is greater than this,
                                                // round up value list size


//
// Limit on the number of layers of hive there may be.  We allow only
// the master hive and hives directly linked into it for now, for currently
// value is always 2..
//

#define MAX_HIVE_LAYERS         2


//
// structure used to create and sort ordered list of drivers to be loaded.
// This is also used by the OS Loader when loading the boot drivers.
// (Particularly the ErrorControl field)
//

typedef struct _BOOT_DRIVER_NODE {
    BOOT_DRIVER_LIST_ENTRY ListEntry;
    UNICODE_STRING Group;
    UNICODE_STRING Name;
    ULONG Tag;
    ULONG ErrorControl;
} BOOT_DRIVER_NODE, *PBOOT_DRIVER_NODE;

//
// extern for object type pointer
//

extern  POBJECT_TYPE CmpKeyObjectType;
extern  POBJECT_TYPE IoFileObjectType;

//
// indexes in CmpMachineHiveList
//
#define SYSTEM_HIVE_INDEX 3
#define CLONE_HIVE_INDEX 6

//
// Miscellaneous Hash routines
//
#define RNDM_CONSTANT   314159269    /* default value for "scrambling constant" */
#define RNDM_PRIME     1000000007    /* prime number, also used for scrambling  */

#define HASH_KEY(_convkey_) ((RNDM_CONSTANT * (_convkey_)) % RNDM_PRIME)

#define GET_HASH_INDEX(Key) HASH_KEY(Key) % CmpHashTableSize
#define GET_HASH_ENTRY(Table, Key) Table[GET_HASH_INDEX(Key)]
#define GET_KCB_HASH_ENTRY(Table, Key)  GET_HASH_ENTRY(Table, Key).Entry
#define GET_KCB_HASH_ENTRY_LOCK(kcb)    (GET_HASH_ENTRY(CmpCacheTable,(kcb)->ConvKey).Lock)
#define GET_KCB_HASH_ENTRY_OWNER(kcb)    (GET_HASH_ENTRY(CmpCacheTable,(kcb)->ConvKey).Owner)

//
// CM_KEY_BODY
//
//  Same structure used for KEY_ROOT and KEY objects.  This is the
//  Cm defined part of the object.
//
//  This object represents an open instance, several of them could refer
//  to a single key control block.
//
#define KEY_BODY_TYPE           0x6b793032      // "ky02"

#define KEY_BODY_HIVE_UNLOADED          0x00000001               // hive for this has been force unloaded or replaced

struct _CM_NOTIFY_BLOCK; //forward

typedef struct _CM_KEY_BODY {
    ULONG                   Type;
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock;
    struct _CM_NOTIFY_BLOCK *NotifyBlock;
    HANDLE                  ProcessID;        // the owner process
    LIST_ENTRY              KeyBodyList;    // key_nodes using the same kcb
} CM_KEY_BODY, *PCM_KEY_BODY;

#define CmpSetNoCallers(KeyBody) // nothing
#define CmpAddKeyTracker(KeyHandle,mode) // nothing yet

VOID InitializeKCBKeyBodyList(IN PCM_KEY_CONTROL_BLOCK kcb);

#if DBG
#define ASSERT_KEYBODY_LIST_EMPTY(kcb)                          \
{                                                               \
    ULONG   i;                                                  \
    ASSERT(IsListEmpty(&(kcb->KeyBodyListHead)) == TRUE);       \
    for(i=0;i<CMP_LOCK_FREE_KEY_BODY_ARRAY_SIZE;i++) {          \
        ASSERT( kcb->KeyBodyArray[i] == NULL );                 \
    }                                                           \
}
#else
#define ASSERT_KEYBODY_LIST_EMPTY(kcb) //nothing
#endif

#define CMP_ENLIST_KCB_LOCKED_SHARED        1
#define CMP_ENLIST_KCB_LOCKED_EXCLUSIVE     2

VOID EnlistKeyBodyWithKCB(IN PCM_KEY_BODY   KeyBody,
                          IN ULONG          ControlFlags );

VOID DelistKeyBodyFromKCB(IN PCM_KEY_BODY   KeyBody,
                          IN BOOLEAN        LockHeld );


#define ASSERT_KEY_OBJECT(x) ASSERT(((PCM_KEY_BODY)x)->Type == KEY_BODY_TYPE)
#define ASSERT_NODE(x) ASSERT(((PCM_KEY_NODE)x)->Signature == CM_KEY_NODE_SIGNATURE)
#define ASSERT_SECURITY(x) ASSERT(((PCM_KEY_SECURITY)x)->Signature == CM_KEY_SECURITY_SIGNATURE)

//
// CM_POST_KEY_BODY
//
// A post block can have attached a keybody which has to be dereferenced
// when the post block goes out of scope. This structure allows the
// implementation of keybody "delayed dereferencing". (see CmpPostNotify for comments)
//

typedef struct _CM_POST_KEY_BODY {
    LIST_ENTRY                  KeyBodyList;
    struct _CM_KEY_BODY         *KeyBody;        // this key body object
} CM_POST_KEY_BODY, *PCM_POST_KEY_BODY;


//
// CM_NOTIFY_BLOCK
//
//  A notify block tracks an active notification waiting for notification.
//  Any one open instance (CM_KEY_BODY) will refer to at most one
//  notify block.  A given key control block may have as many notify
//  blocks referring to it as there are CM_KEY_BODYs referring to it.
//  Notify blocks are attached to hives and sorted by length of name.
//

typedef struct _CM_NOTIFY_BLOCK {
    LIST_ENTRY                  HiveList;        // sorted list of notifies
    LIST_ENTRY                  PostList;        // Posts to fill
    PCM_KEY_CONTROL_BLOCK       KeyControlBlock; // Open instance notify is on
    struct _CM_KEY_BODY         *KeyBody;        // our owning key handle object
    struct {
        ULONG                       Filter          : 30;    // Events of interest
        ULONG                       WatchTree       : 1;
        ULONG                       NotifyPending   : 1;
    };
    SECURITY_SUBJECT_CONTEXT    SubjectContext;  // Security stuff
} CM_NOTIFY_BLOCK, *PCM_NOTIFY_BLOCK;

//
// CM_POST_BLOCK
//
//  Whenever a notify call is made, a post block is created and attached
//  to the notify block.  Each time an event is posted against the notify,
//  the waiter described by the post block is signaled.  (i.e. APC enqueued,
//  event signaled, etc.)
//

//
//  The NotifyType ULONG is a combination of POST_BLOCK_TYPE enum and flags
//

typedef enum _POST_BLOCK_TYPE {
    PostSynchronous = 1,
    PostAsyncUser = 2,
    PostAsyncKernel = 3
} POST_BLOCK_TYPE;

typedef struct _CM_SYNC_POST_BLOCK {
    PKEVENT                 SystemEvent;
    NTSTATUS                Status;
} CM_SYNC_POST_BLOCK, *PCM_SYNC_POST_BLOCK;

typedef struct _CM_ASYNC_USER_POST_BLOCK {
    ULONG                   Dummy;
    PKEVENT                 UserEvent;
    PKAPC                   Apc;
    PIO_STATUS_BLOCK        IoStatusBlock;
} CM_ASYNC_USER_POST_BLOCK, *PCM_ASYNC_USER_POST_BLOCK;

typedef struct _CM_ASYNC_KERNEL_POST_BLOCK {
    PKEVENT                 Event;
    PWORK_QUEUE_ITEM        WorkItem;
    WORK_QUEUE_TYPE         QueueType;
} CM_ASYNC_KERNEL_POST_BLOCK, *PCM_ASYNC_KERNEL_POST_BLOCK;

typedef union _CM_POST_BLOCK_UNION {
    CM_SYNC_POST_BLOCK  Sync;
    CM_ASYNC_USER_POST_BLOCK AsyncUser;
    CM_ASYNC_KERNEL_POST_BLOCK AsyncKernel;
} CM_POST_BLOCK_UNION, *PCM_POST_BLOCK_UNION;

typedef struct _CM_POST_BLOCK {
#if DBG
    BOOLEAN                     TraceIntoDebugger;
#endif
    LIST_ENTRY                  NotifyList;
    LIST_ENTRY                  ThreadList;
    LIST_ENTRY                  CancelPostList; // slave notifications that are attached to this notification
    struct _CM_POST_KEY_BODY    *PostKeyBody;
    ULONG                       NotifyType;
    PCM_POST_BLOCK_UNION        u;
} CM_POST_BLOCK, *PCM_POST_BLOCK;

#define REG_NOTIFY_POST_TYPE_MASK (0x0000FFFFL)   // mask for finding out the type of the post block

#define REG_NOTIFY_MASTER_POST    (0x00010000L)   // The current post block is a master one

//
// Useful macros to manipulate the NotifyType field in CM_POST_BLOCK
//
#define PostBlockType(_post_) ((POST_BLOCK_TYPE)( ((_post_)->NotifyType) & REG_NOTIFY_POST_TYPE_MASK ))

#define IsMasterPostBlock(_post_)           ( ((_post_)->NotifyType) &   REG_NOTIFY_MASTER_POST )
#define SetMasterPostBlockFlag(_post_)      ( ((_post_)->NotifyType) |=  REG_NOTIFY_MASTER_POST )
#define ClearMasterPostBlockFlag(_post_)    ( ((_post_)->NotifyType) &= ~REG_NOTIFY_MASTER_POST )

//
// This lock protects the PostList(s) in Notification objects.
// It is used to prevent attempts for simultaneous changes of
// CancelPostList list in PostBlocks
//

extern EX_PUSH_LOCK CmpPostLock;
#define LOCK_POST_LIST() ExAcquirePushLockExclusive(&CmpPostLock)
#define UNLOCK_POST_LIST() ExReleasePushLock(&CmpPostLock)


extern EX_PUSH_LOCK CmpStashBufferLock;
#define LOCK_STASH_BUFFER() ExAcquirePushLockExclusive(&CmpStashBufferLock)
#define UNLOCK_STASH_BUFFER() ExReleasePushLock(&CmpStashBufferLock)


//
// protection for CmpHiveListHead
//
extern EX_PUSH_LOCK CmpHiveListHeadLock;
#define CmpLockHiveListShared()     ExAcquirePushLockShared(&CmpHiveListHeadLock)
#define CmpLockHiveListExclusive()  ExAcquirePushLockExclusive(&CmpHiveListHeadLock)
#define CmpUnlockHiveList()         ExReleasePushLock(&CmpHiveListHeadLock)

//
// protection for CmLoadKey
//
extern EX_PUSH_LOCK    CmpLoadHiveLock;
#define LOCK_HIVE_LOAD() ExAcquirePushLockExclusive(&CmpLoadHiveLock)
#define UNLOCK_HIVE_LOAD() ExReleasePushLock(&CmpLoadHiveLock)

//
// used by CmpFileWrite, so it doesn't take up so much stack.
//
typedef struct _CM_WRITE_BLOCK {
    HANDLE          EventHandles[MAXIMUM_WAIT_OBJECTS];
    PKEVENT         EventObjects[MAXIMUM_WAIT_OBJECTS];
    KWAIT_BLOCK     WaitBlockArray[MAXIMUM_WAIT_OBJECTS];
    IO_STATUS_BLOCK IoStatus[MAXIMUM_WAIT_OBJECTS];
} CM_WRITE_BLOCK, *PCM_WRITE_BLOCK;

//
// CM data to manipulate views inside the primary hive file
//

//#define MAPPED_VIEWS_PER_HIVE   12 * (_256K / CM_VIEW_SIZE ) // max 3 MB per hive ; we don't really need this
#define MAX_VIEWS_PER_HIVE      MAX_MB_PER_HIVE * ( (_256K) / (CM_VIEW_SIZE) )

#define ASSERT_VIEW_MAPPED(a)                           \
    ASSERT((a)->Size != 0);                             \
    ASSERT((a)->ViewAddress != 0);                      \
    ASSERT((a)->Bcb != 0);                              \
    ASSERT( IsListEmpty(&((a)->LRUViewList)) == FALSE); \
    ASSERT( IsListEmpty(&((a)->PinViewList)) == TRUE)

#define ASSERT_VIEW_PINNED(a)                           \
    ASSERT((a)->Size != 0);                             \
    ASSERT((a)->ViewAddress != 0);                      \
    ASSERT((a)->Bcb != 0);                              \
    ASSERT( IsListEmpty(&((a)->LRUViewList)) == TRUE)

typedef struct _CM_VIEW_OF_FILE {
    LIST_ENTRY      LRUViewList;        // LRU connection ==> when this is empty, the view is pinned
    LIST_ENTRY      PinViewList;        // list of views pinned into memory ==> when this is empty, the view is in LRU list
    ULONG           FileOffset;         // file offset at which the mapping starts
    ULONG           Size;               // size the view maps
    PULONG_PTR      ViewAddress;        // memory address containing the mapping
    PVOID           Bcb;                // BCB needed for map/pin/unpin access
    ULONG           UseCount;           // how many cells are currently in use inside this view
} CM_VIEW_OF_FILE, *PCM_VIEW_OF_FILE;


//
// security hash manipulation
//
#define CmpSecHashTableSize             64      // size of the hash table

typedef struct _CM_KCB_REMAP_BLOCK {
    LIST_ENTRY              RemapList;
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock;
    HCELL_INDEX             OldCellIndex;
    HCELL_INDEX             NewCellIndex;
    ULONG                   ValueCount;
    HCELL_INDEX             ValueList;
} CM_KCB_REMAP_BLOCK, *PCM_KCB_REMAP_BLOCK;

typedef struct _CM_CELL_REMAP_BLOCK {
    HCELL_INDEX             OldCell;
    HCELL_INDEX             NewCell;
} CM_CELL_REMAP_BLOCK, *PCM_CELL_REMAP_BLOCK;

typedef struct _CM_KNODE_REMAP_BLOCK {
    LIST_ENTRY              RemapList;
    PCM_KEY_NODE            KeyNode;
    HCELL_INDEX             NewParent;
} CM_KNODE_REMAP_BLOCK, *PCM_KNODE_REMAP_BLOCK;

#define CM_CMHIVE_FLAG_UNTRUSTED    1   // hive is untrusted (but it may be inside of a trusted class).
// ----- Cm version of Hive structure (CMHIVE) -----
//
typedef struct _CMHIVE {
    HHIVE                           Hive;
    HANDLE                          FileHandles[HFILE_TYPE_MAX];
    LIST_ENTRY                      NotifyList;
    LIST_ENTRY                      HiveList;           // Used to find hives at shutdown
    EX_PUSH_LOCK                    HiveLock;           // Used to synchronize operations on the hive (NotifyList and Flush)
#if DBG
    PKTHREAD                        HiveLockOwner;      // debug only
#endif
    PKGUARDED_MUTEX                 ViewLock;           // Used to control access over the view list, UseCount
#if DBG
    PKTHREAD                        ViewLockOwner;      // debug only
#endif
    EX_PUSH_LOCK                    WriterLock;         // Used to synchronize writers (and get rid of global registry lock).
#if DBG
    PKTHREAD                        WriterLockOwner;    // debug only
#endif
#if DBG
    PERESOURCE                      FlusherLock;        // Used to synchronize flushes, read and writes ( make sure the image we flush is atomic).
#else
    EX_PUSH_LOCK                    FlusherLock;        // Used to synchronize flushes, read and writes ( make sure the image we flush is atomic).
#endif
    EX_PUSH_LOCK                    SecurityLock;       // Used to synchronize security on the hive
#if DBG
    PKTHREAD                        HiveSecurityLockOwner;      // debug only
#endif
    LIST_ENTRY                      LRUViewListHead;    // Head of the same list as above but ordered (LRU)
    LIST_ENTRY                      PinViewListHead;    // Head of the List of Views pinned into memory inside the primary hive file
    PFILE_OBJECT                    FileObject;         // FileObject needed for Cc operations on the mapped views
    UNICODE_STRING                  FileFullPath;       // full path of the hive file- needed for CmPrefetchHivePages
    UNICODE_STRING                  FileUserName;       // file name as passed onto NtLoadKey 
    USHORT                          MappedViews;        // number of mapped (but not pinned views) i.e. the number of elements in LRUViewList
    USHORT                          PinnedViews;        // number of pinned views i.e. the number of elements in PinViewList
    ULONG                           UseCount;           // how many cells are currently in use inside this hive
    ULONG                           SecurityCount;      // number of security cells cached
    ULONG                           SecurityCacheSize;  // number of entries in the cache (to avoid memory fragmentation)
    LONG                            SecurityHitHint;    // index of the last cell we've searched on
    PCM_KEY_SECURITY_CACHE_ENTRY    SecurityCache;      // the security cache

                                                        // hash table (to retrieve the security cells by descriptor)
    LIST_ENTRY                      SecurityHash[CmpSecHashTableSize];

    PKEVENT                         UnloadEvent;        // the event to be signaled when the hive unloads
                                                        // this may be valid (not NULL) only in conjunction with
                                                        // a not NULL RootKcb and a TRUE Frozen (below)

    PCM_KEY_CONTROL_BLOCK           RootKcb;            // kcb to the root of the hive. We keep a reference on it, which
                                                        // will be released at the time the hive unloads (i.e. it is the last
                                                        // reference somebody has on this kcb); This is should be valid (not NULL)
                                                        // only when the Frozen flag is set to TRUE

    BOOLEAN                         Frozen;             // set to TRUE when the hive is frozen (no further operations are allowed on
                                                        // this hive

    PWORK_QUEUE_ITEM                UnloadWorkItem;     // Work Item to actually perform the late unload

    BOOLEAN                         GrowOnlyMode;       // the hive is in "grow only" mode; new cells are allocated past GrowOffset
    ULONG                           GrowOffset;

    LIST_ENTRY                      KcbConvertListHead; // list of CM_KCB_REMAP_BLOCK storing the associations to the new hive.
    LIST_ENTRY                      KnodeConvertListHead;
    PCM_CELL_REMAP_BLOCK            CellRemapArray;     // array of mappings used for security cells

    ULONG                           Flags;              // CMHIVE specific flags
    LIST_ENTRY                      TrustClassEntry;    // links together the UNTRUSTED hives in the same 'class of trust'
    ULONG                           FlushCount;
#if DBG
    BOOLEAN                         HiveIsLoading;      // hive is loading; not yet in the hive list; safe to work on it alone
#endif
    PKTHREAD                        CreatorOwner;       // so we can mount the hive safely
} CMHIVE, *PCMHIVE;

#define CmpUnJoinClassOfTrust(CmHive)                       \
if( !IsListEmpty(&(CmHive->TrustClassEntry)) ) {            \
    ASSERT(CmHive->Flags&CM_CMHIVE_FLAG_UNTRUSTED);         \
    CmpLockHiveListExclusive();                             \
    RemoveEntryList(&(CmHive->TrustClassEntry));            \
    CmpUnlockHiveList();                                    \
} 
#define CmpJoinClassOfTrust(_NewHive,_OtherHive)                            \
CmpLockHiveListExclusive();                                                 \
InsertTailList(&(_OtherHive->TrustClassEntry),&(_NewHive->TrustClassEntry));\
CmpUnlockHiveList()


#define CmLogCellRef( HIVE, CELL )   
#define CmLogCellDeRef( HIVE, CELL )

#define CmLogLockHive( HIVE)
#define CmLogUnlockHive( HIVE)

//
// Hive's own writer lock
//
#define CmpLockHiveWriter(_hive_)  ASSERT_CM_LOCK_OWNED_OR_HIVE_LOADING((PCMHIVE)(_hive_));\
                                    ExAcquirePushLockExclusive(&(((PCMHIVE)(_hive_))->WriterLock));\
                                    ASSERT( ((PCMHIVE)(_hive_))->WriterLockOwner = KeGetCurrentThread() )

#define CmpUnlockHiveWriter(_hive_) ASSERT_CM_LOCK_OWNED_OR_HIVE_LOADING((PCMHIVE)(_hive_));\
                             ASSERT_HIVE_WRITER_LOCK_OWNED((PCMHIVE)(_hive_));\
                             ASSERT( (((PCMHIVE)(_hive_))->WriterLockOwner = NULL) == NULL);\
                             ExReleasePushLock(&(((PCMHIVE)(_hive_))->WriterLock))
#if DBG
#define ASSERT_HIVE_WRITER_LOCK_OWNED(_CmHive_) ASSERT( (CmpSpecialBootCondition == TRUE) || (((PCMHIVE)(_CmHive_))->HiveIsLoading) || (((PCMHIVE)(_CmHive_))->WriterLockOwner == KeGetCurrentThread()) || (CmpTestRegistryLockExclusive() == TRUE))
#else
#define ASSERT_HIVE_WRITER_LOCK_OWNED(_CmHive_) //nothing
#endif //DBG

//
// Hive's flusher lock
//
#if DBG
VOID CmpLockHiveFlusherShared(PCMHIVE CmHive);
VOID CmpLockHiveFlusherExclusive(PCMHIVE CmHive);
VOID CmpUnlockHiveFlusher(PCMHIVE CmHive);

BOOLEAN CmpTestHiveFlusherLockShared(PCMHIVE CmHive);
BOOLEAN CmpTestHiveFlusherLockExclusive(PCMHIVE CmHive);

#define ASSERT_HIVE_FLUSHER_LOCKED(_CmHive_) ASSERT( (CmpSpecialBootCondition == TRUE) || (((PCMHIVE)(_CmHive_))->HiveIsLoading) || (CmpTestHiveFlusherLockShared(_CmHive_) == TRUE) || (CmpTestHiveFlusherLockExclusive(_CmHive_) == TRUE) || (CmpTestRegistryLockExclusive() == TRUE))
#define ASSERT_HIVE_FLUSHER_LOCKED_EXCLUSIVE(_CmHive_) ASSERT( (CmpSpecialBootCondition == TRUE) || (((PCMHIVE)(_CmHive_))->HiveIsLoading) || (CmpTestHiveFlusherLockExclusive(_CmHive_) == TRUE) || (CmpTestRegistryLockExclusive() == TRUE))

#else 
#define CmpLockHiveFlusherShared(CmHive)    ExAcquirePushLockShared(&(((PCMHIVE)CmHive)->FlusherLock))
#define CmpLockHiveFlusherExclusive(CmHive) ExAcquirePushLockExclusive(&(((PCMHIVE)CmHive)->FlusherLock))
#define CmpUnlockHiveFlusher(CmHive)        ExReleasePushLock(&(((PCMHIVE)CmHive)->FlusherLock))

#define ASSERT_HIVE_FLUSHER_LOCKED(_CmHive_) //nothing
#define ASSERT_HIVE_FLUSHER_LOCKED_EXCLUSIVE(_CmHive_) //nothing
#endif //DBG

#define IsHiveFrozen(_CmHive_) (((PCMHIVE)(_CmHive_))->Frozen == TRUE)

#define HiveWritesThroughCache(Hive,FileType) ((FileType == HFILE_TYPE_PRIMARY) && (((PCMHIVE)CONTAINING_RECORD(Hive, CMHIVE, Hive))->FileObject != NULL))


//
// Delayed close kcb list
//
typedef struct _CM_DELAYED_CLOSE_ENTRY {
    LIST_ENTRY              DelayedLRUList;     //  LRU list of entries in the Delayed Close Table
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock;    //  KCB in this entry; NULL if the entry is available
} CM_DELAYED_CLOSE_ENTRY, *PCM_DELAYED_CLOSE_ENTRY;


//
// Hive locking support
//
//
#if DBG
#define ASSERT_HIVE_LOCK_OWNED(_hive_)  ASSERT( (CmpSpecialBootCondition == TRUE) || (((PCMHIVE)(_hive_))->HiveIsLoading) || ((_hive_)->HiveLockOwner == KeGetCurrentThread()) || (CmpTestRegistryLockExclusive() == TRUE) )
#else
#define ASSERT_HIVE_LOCK_OWNED(_hive_) //nothing
#endif

#define CmLockHive(_hive_)  ExAcquirePushLockExclusive(&((_hive_)->HiveLock));\
                            CmLogLockHive(_hive_);\
                            ASSERT( (_hive_)->HiveLockOwner = KeGetCurrentThread() )

#define CmUnlockHive(_hive_) ASSERT_HIVE_LOCK_OWNED(_hive_);\
                             ASSERT( ((_hive_)->HiveLockOwner = NULL) == NULL);\
                             CmLogUnlockHive(_hive_);\
                             ExReleasePushLock(&((_hive_)->HiveLock))


#if DBG
#define ASSERT_HIVE_SECURITY_LOCK_OWNED(_hive_)  ASSERT( ((_hive_)->HiveSecurityLockOwner == KeGetCurrentThread()) || (CmpTestRegistryLockExclusive() == TRUE) )
#else
#define ASSERT_HIVE_SECURITY_LOCK_OWNED(_hive_) //nothing
#endif

#define CmLockHiveSecurityShared(_hive_) ExAcquirePushLockShared(&((_hive_)->SecurityLock))

#define CmLockHiveSecurityExclusive(_hive_) ExAcquirePushLockExclusive(&((_hive_)->SecurityLock));\
                                            ASSERT( (_hive_)->HiveSecurityLockOwner = KeGetCurrentThread() )

#define CmUnlockHiveSecurity(_hive_)    ASSERT( ((_hive_)->HiveSecurityLockOwner == NULL) || ((_hive_)->HiveSecurityLockOwner == KeGetCurrentThread()) );\
                                        ASSERT( ((_hive_)->HiveSecurityLockOwner = NULL) == NULL);\
                                        ExReleasePushLock(&((_hive_)->SecurityLock))

//
// View locking support
//
#if DBG
#define ASSERT_VIEW_LOCK_OWNED(_hive_)  ASSERT( (CmpSpecialBootCondition == TRUE) || (((PCMHIVE)(_hive_))->HiveIsLoading) || (((PCMHIVE)(_hive_))->ViewLockOwner == KeGetCurrentThread()) || (CmpTestRegistryLockExclusive() == TRUE))
#else
#define ASSERT_VIEW_LOCK_OWNED(_hive_) //nothing
#endif

#define CmLockHiveViews(_hive_)     ASSERT( ((PCMHIVE)(_hive_))->ViewLock );\
                                    KeAcquireGuardedMutex(((PCMHIVE)(_hive_))->ViewLock);\
                                    ASSERT( ((PCMHIVE)(_hive_))->ViewLockOwner = KeGetCurrentThread() )

#define CmUnlockHiveViews(_hive_)   ASSERT( ((PCMHIVE)(_hive_))->ViewLock );\
                                    ASSERT_VIEW_LOCK_OWNED((PCMHIVE)(_hive_));\
                                    ASSERT( (((PCMHIVE)(_hive_))->ViewLockOwner = NULL) == NULL);\
                                    KeReleaseGuardedMutex(((PCMHIVE)(_hive_))->ViewLock)

//
// Macros
//

//
// ----- CM_KEY_NODE -----
//
#define CmpHKeyNameLen(Key) \
        (((Key)->Flags & KEY_COMP_NAME) ? \
            CmpCompressedNameSize((Key)->Name,(Key)->NameLength) : \
            (Key)->NameLength)

#define CmpNcbNameLen(Ncb) \
        (((Ncb)->Compressed) ? \
            CmpCompressedNameSize((Ncb)->Name,(Ncb)->NameLength) : \
            (Ncb)->NameLength)

#define CmpHKeyNodeSize(Hive, KeyName) \
    (FIELD_OFFSET(CM_KEY_NODE, Name) + CmpNameSize(Hive, KeyName))


//
// ----- CM_KEY_VALUE -----
//


#define CmpValueNameLen(Value)                                       \
        (((Value)->Flags & VALUE_COMP_NAME) ?                           \
            CmpCompressedNameSize((Value)->Name,(Value)->NameLength) :  \
            (Value)->NameLength)

#define CmpHKeyValueSize(Hive, ValueName) \
    (FIELD_OFFSET(CM_KEY_VALUE, Name) + CmpNameSize(Hive, ValueName))


//
// ----- Procedure Prototypes -----
//

//
// Configuration Manager private procedure prototypes
//

#define REG_OPTION_PREDEF_HANDLE (0x01000000L)
#define REG_PREDEF_HANDLE_MASK   (0x80000000L)

typedef struct _CM_PARSE_CONTEXT {
    ULONG                   TitleIndex;
    UNICODE_STRING          Class;
    ULONG                   CreateOptions;
    ULONG                   Disposition;
    CM_KEY_REFERENCE        ChildHive;
    HANDLE                  PredefinedHandle;
    BOOLEAN                 CreateLink;
    BOOLEAN                 CreateOperation;
    PCMHIVE                 OriginatingPoint;
} CM_PARSE_CONTEXT, *PCM_PARSE_CONTEXT;

#define CmpParseRecordOriginatingPoint(_Context,_CmHive)                                            \
if( ARGUMENT_PRESENT(_Context) && (((PCM_PARSE_CONTEXT)(_Context))->OriginatingPoint == NULL) &&    \
    (((PCMHIVE)_CmHive)->Flags&CM_CMHIVE_FLAG_UNTRUSTED) ){                                         \
    ((PCM_PARSE_CONTEXT)(_Context))->OriginatingPoint = (PCMHIVE)_CmHive;                           \
}

#define CmpParseGetOriginatingPoint(_Context) ARGUMENT_PRESENT(_Context)?((PCM_PARSE_CONTEXT)(_Context))->OriginatingPoint:NULL

NTSTATUS
CmpParseKey(
    IN PVOID ParseObject,
    IN PVOID ObjectType,
    IN OUT PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN OUT PUNICODE_STRING CompleteName,
    IN OUT PUNICODE_STRING RemainingName,
    IN OUT PVOID Context OPTIONAL,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
    OUT PVOID *Object
    );

NTSTATUS
CmpDoCreate(
    IN PHHIVE                   Hive,
    IN HCELL_INDEX              Cell,
    IN PACCESS_STATE            AccessState,
    IN PUNICODE_STRING          Name,
    IN KPROCESSOR_MODE          AccessMode,
    IN PCM_PARSE_CONTEXT        Context,
    IN PCM_KEY_CONTROL_BLOCK    ParentKcb,
    IN PCMHIVE                  OriginatingHive OPTIONAL,
    OUT PVOID                   *Object
    );

NTSTATUS
CmpDoCreateChild(
    IN PHHIVE Hive,
    IN HCELL_INDEX ParentCell,
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PACCESS_STATE AccessState,
    IN PUNICODE_STRING Name,
    IN KPROCESSOR_MODE AccessMode,
    IN PCM_PARSE_CONTEXT Context,
    IN PCM_KEY_CONTROL_BLOCK ParentKcb,
    IN USHORT Flags,
    OUT PHCELL_INDEX KeyCell,
    OUT PVOID *Object
    );

NTSTATUS
CmpQueryKeyName(
    IN PVOID Object,
    IN BOOLEAN HasObjectName,
    OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength,
    IN KPROCESSOR_MODE Mode
    );

VOID
CmpDeleteKeyObject(
    IN  PVOID   Object
    );

VOID
CmpCloseKeyObject(
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG_PTR ProcessHandleCount,
    IN ULONG_PTR SystemHandleCount
    );

NTSTATUS
CmpAssignSecurityDescriptorWrapper(
    IN PVOID                    Object,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTSTATUS
CmpSecurityMethod (
    IN PVOID Object,
    IN SECURITY_OPERATION_CODE OperationCode,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG CapturedLength,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    );

#define KCB_WORKER_CONTINUE     0
#define KCB_WORKER_DONE         1
#define KCB_WORKER_DELETE       2
#define KCB_WORKER_ERROR        3

typedef
ULONG
(*PKCB_WORKER_ROUTINE) (
    PCM_KEY_CONTROL_BLOCK Current,
    PVOID                 Context1,
    PVOID                 Context2
    );


BOOLEAN
CmpSearchKeyControlBlockTree(
    PKCB_WORKER_ROUTINE WorkerRoutine,
    PVOID               Context1,
    PVOID               Context2
    );

//
// Wrappers
//

PVOID
CmpAllocate(
    ULONG   Size,
    BOOLEAN UseForIo,
    ULONG   Tag
    );

VOID
CmpFree(
    PVOID   MemoryBlock,
    ULONG   GlobalQuotaSize
    );

BOOLEAN
CmpFileSetSize(
    PHHIVE      Hive,
    ULONG       FileType,
    ULONG       FileSize,
    ULONG       OldFileSize
    );

NTSTATUS
CmpDoFileSetSize(
    PHHIVE      Hive,
    ULONG       FileType,
    ULONG       FileSize,
    ULONG       OldFileSize
    );

BOOLEAN
CmpFileWrite(
    PHHIVE      Hive,
    ULONG       FileType,
    PCMP_OFFSET_ARRAY offsetArray,
    ULONG offsetArrayCount,
    PULONG      FileOffset
    );

BOOLEAN
CmpFileWriteThroughCache(
    PHHIVE              Hive,
    ULONG               FileType,
    PCMP_OFFSET_ARRAY   offsetArray,
    ULONG               offsetArrayCount
    );

BOOLEAN
CmpFileRead (
    PHHIVE      Hive,
    ULONG       FileType,
    PULONG      FileOffset,
    PVOID       DataBuffer,
    ULONG       DataLength
    );

BOOLEAN
CmpFileFlush (
    PHHIVE          Hive,
    ULONG           FileType,
    PLARGE_INTEGER  FileOffset,
    ULONG           Length
    );

NTSTATUS
CmpCreateEvent(
    IN EVENT_TYPE  eventType,
    OUT PHANDLE eventHandle,
    OUT PKEVENT *event
    );


//
// Configuration Manager CM level registry functions
//

NTSTATUS
CmDeleteKey(
    IN PCM_KEY_BODY KeyBody
    );

NTSTATUS
CmDeleteValueKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN UNICODE_STRING ValueName
    );

NTSTATUS
CmEnumerateKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    IN PVOID KeyInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

NTSTATUS
CmEnumerateValueKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

NTSTATUS
CmFlushKey(
    IN PCM_KEY_CONTROL_BLOCK    Kcb,
    IN BOOLEAN                  RegistryLockOwnedExclusive
    );

NTSTATUS
CmQueryKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    IN PVOID KeyInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

NTSTATUS
CmQueryValueKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN UNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

NTSTATUS
CmQueryMultipleValueKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PKEY_VALUE_ENTRY ValueEntries,
    IN ULONG EntryCount,
    IN PVOID ValueBuffer,
    IN OUT PULONG BufferLength,
    IN OPTIONAL PULONG ResultLength
    );

NTSTATUS
CmRenameValueKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN UNICODE_STRING SourceValueName,
    IN UNICODE_STRING TargetValueName,
    IN ULONG TargetIndex
    );

NTSTATUS
CmReplaceKey(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell,
    IN PUNICODE_STRING NewHiveName,
    IN PUNICODE_STRING OldFileName
    );

NTSTATUS
CmRestoreKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN HANDLE  FileHandle,
    IN ULONG Flags
    );

NTSTATUS
CmSaveKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN HANDLE                   FileHandle,
    IN ULONG                    HiveVersion
    );

NTSTATUS
CmDumpKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN HANDLE                   FileHandle
    );

NTSTATUS
CmSaveMergedKeys(
    IN PCM_KEY_CONTROL_BLOCK    HighPrecedenceKcb,
    IN PCM_KEY_CONTROL_BLOCK    LowPrecedenceKcb,
    IN HANDLE   FileHandle
    );

NTSTATUS
CmpShiftHiveFreeBins(
                      PCMHIVE           CmHive,
                      PCMHIVE           *NewHive
                      );

NTSTATUS
CmpOverwriteHive(
                    PCMHIVE         CmHive,
                    PCMHIVE         NewHive,
                    HCELL_INDEX     LinkCell
                    );

VOID
CmpSwitchStorageAndRebuildMappings(PCMHIVE  OldCmHive,
                                   PCMHIVE  NewHive
                                   );

NTSTATUS
CmSetValueKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PUNICODE_STRING ValueName,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    );

NTSTATUS
CmSetLastWriteTimeKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PLARGE_INTEGER LastWriteTime
    );

NTSTATUS
CmSetKeyUserFlags(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN ULONG                    UserFlags
    );

NTSTATUS
CmpNotifyChangeKey(
    IN PCM_KEY_BODY     KeyBody,
    IN PCM_POST_BLOCK   PostBlock,
    IN ULONG            CompletionFilter,
    IN BOOLEAN          WatchTree,
    IN PVOID            Buffer,
    IN ULONG            BufferSize,
    IN PCM_POST_BLOCK   MasterPostBlock
    );

NTSTATUS
CmLoadKey(
    IN POBJECT_ATTRIBUTES   TargetKey,
    IN POBJECT_ATTRIBUTES   SourceFile,
    IN ULONG                Flags,
    IN PCM_KEY_BODY         KeyBody
    );

#define CM_UNLOAD_KCB_LOCKED    1
#define CM_UNLOAD_REG_LOCKED_EX 2
                  
NTSTATUS
CmUnloadKey(
    IN PCM_KEY_CONTROL_BLOCK    Kcb,
    IN ULONG                    Flags,
    IN ULONG                    ControlFlags
    );

NTSTATUS
CmMoveKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock
    );

NTSTATUS
CmCompressKey(
    IN PHHIVE Hive
    );

//
// Procedures private to CM
//

BOOLEAN
CmpMarkKeyDirty(
    PHHIVE Hive,
    HCELL_INDEX Cell
#if DBG
    ,
    BOOLEAN CheckNoSubkeys
#endif
    );

BOOLEAN
CmpDoFlushAll(
    BOOLEAN ForceFlush
    );

VOID
CmpFixHiveUsageCount(
                    IN  PCMHIVE             CmHive
                    );

VOID
CmpLazyFlush(
    VOID
    );

VOID
CmpQuotaWarningWorker(
    IN PVOID WorkItem
    );

VOID
CmpComputeGlobalQuotaAllowed(
    VOID
    );

BOOLEAN
CmpClaimGlobalQuota(
    IN ULONG    Size
    );

VOID
CmpReleaseGlobalQuota(
    IN ULONG    Size
    );

VOID
CmpSetGlobalQuotaAllowed(
    VOID
    );

VOID
CmpSystemQuotaWarningWorker(
    IN PVOID WorkItem
    );

BOOLEAN
CmpCanGrowSystemHive(
                     IN PHHIVE  Hive,
                     IN ULONG   NewLength
                     );

//
// security functions (cmse.c)
//

NTSTATUS
CmpAssignSecurityDescriptor(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell,
    IN PCM_KEY_NODE Node,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );

BOOLEAN
CmpCheckCreateAccess(
    IN PUNICODE_STRING RelativeName,
    IN PSECURITY_DESCRIPTOR Descriptor,
    IN PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE PreviousMode,
    IN ACCESS_MASK AdditionalAccess,
    OUT PNTSTATUS AccessStatus
    );

BOOLEAN
CmpCheckNotifyAccess(
    IN PCM_NOTIFY_BLOCK NotifyBlock,
    IN PHHIVE Hive,
    IN PCM_KEY_NODE Node
    );

PSECURITY_DESCRIPTOR
CmpHiveRootSecurityDescriptor(
    VOID
    );

VOID
CmpFreeSecurityDescriptor(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell
    );

NTSTATUS
CmpCheckKeyAccess(
    IN PHHIVE           Hive,
    IN HCELL_INDEX      NodeCell,
    IN KPROCESSOR_MODE  PreviousMode,
    IN ACCESS_MASK      DesiredAccess
    );

NTSTATUS
CmpDoAccessCheckOnSubtree(
    PHHIVE          HiveToCheck,
    HCELL_INDEX     Cell,
    KPROCESSOR_MODE PreviousMode,
    ACCESS_MASK     DesiredAccess,
    BOOLEAN         CheckRoot
    );

//
// Access to the registry is serialized by a shared resource, CmpRegistryLock.
//
extern ERESOURCE    CmpRegistryLock;

//
// Support for "StarveExclusive" mode ensuring a flush
//
extern LONG        CmpFlushStarveWriters;

#define ENTER_FLUSH_MODE()  InterlockedIncrement (&CmpFlushStarveWriters);

#if DBG
#define EXIT_FLUSH_MODE()                                                       \
{                                                                               \
    LONG LocalIncrement = (LONG)InterlockedDecrement (&CmpFlushStarveWriters);  \
    ASSERT( LocalIncrement >= 0 );                                              \
}
#else
#define EXIT_FLUSH_MODE() InterlockedDecrement (&CmpFlushStarveWriters)
#endif


VOID
CmpLockRegistryExclusive(
    VOID
    );
VOID
CmpLockRegistry(
    VOID
    );

VOID
CmpUnlockRegistry(
    );

#if DBG
BOOLEAN
CmpTestRegistryLock(
    VOID
    );
BOOLEAN
CmpTestRegistryLockExclusive(
    VOID
    );

#endif

NTSTATUS
CmpQueryKeyData(
    PHHIVE Hive,
    PCM_KEY_NODE Node,
    KEY_INFORMATION_CLASS KeyInformationClass,
    PVOID KeyInformation,
    ULONG Length,
    PULONG ResultLength
    );

NTSTATUS
CmpQueryKeyDataFromCache(
    PCM_KEY_CONTROL_BLOCK   Kcb,
    KEY_INFORMATION_CLASS   KeyInformationClass,
    PVOID                   KeyInformation,
    ULONG                   Length,
    PULONG                  ResultLength
    );


BOOLEAN
CmpFreeKeyBody(
    PHHIVE Hive,
    HCELL_INDEX Cell
    );

BOOLEAN
CmpFreeValue(
    PHHIVE Hive,
    HCELL_INDEX Cell
    );

HCELL_INDEX
CmpFindValueByName(
    PHHIVE Hive,
    PCM_KEY_NODE KeyNode,
    PUNICODE_STRING Name
    );

NTSTATUS
CmpDeleteChildByName(
    PHHIVE  Hive,
    HCELL_INDEX Cell,
    UNICODE_STRING  Name,
    PHCELL_INDEX    ChildCell
    );

NTSTATUS
CmpFreeKeyByCell(
    PHHIVE Hive,
    HCELL_INDEX Cell,
    BOOLEAN Unlink
    );

BOOLEAN
CmpFindNameInList(
    IN PHHIVE  Hive,
    IN PCHILD_LIST ChildList,
    IN PUNICODE_STRING Name,
    IN OPTIONAL PULONG ChildIndex,
    OUT PHCELL_INDEX    CellIndex
    );

HCELL_INDEX
CmpCopyCell(
    PHHIVE  SourceHive,
    HCELL_INDEX SourceCell,
    PHHIVE  TargetHive,
    HSTORAGE_TYPE   Type
    );

HCELL_INDEX
CmpCopyValue(
    PHHIVE  SourceHive,
    HCELL_INDEX SourceValueCell,
    PHHIVE  TargetHive,
    HSTORAGE_TYPE Type
    );

HCELL_INDEX
CmpCopyKeyPartial(
    PHHIVE  SourceHive,
    HCELL_INDEX SourceKeyCell,
    PHHIVE  TargetHive,
    HCELL_INDEX Parent,
    BOOLEAN CopyValues
    );

BOOLEAN
CmpCopySyncTree(
    PHHIVE                  SourceHive,
    HCELL_INDEX             SourceCell,
    PHHIVE                  TargetHive,
    HCELL_INDEX             TargetCell,
    BOOLEAN                 CopyVolatile,
    CMP_COPY_TYPE           CopyType
    );

//
// BOOLEAN
// CmpCopyTree(
//    PHHIVE      SourceHive,
//    HCELL_INDEX SourceCell,
//    PHHIVE      TargetHive,
//    HCELL_INDEX TargetCell
//    );
//

#define CmpCopyTree(s,c,t,l) CmpCopySyncTree(s,c,t,l,FALSE,Copy)

//
// BOOLEAN
// CmpCopyTreeEx(
//    PHHIVE      SourceHive,
//    HCELL_INDEX SourceCell,
//    PHHIVE      TargetHive,
//    HCELL_INDEX TargetCell,
//    BOOLEAN     CopyVolatile
//    );
//

#define CmpCopyTreeEx(s,c,t,l,f) CmpCopySyncTree(s,c,t,l,f,Copy)

//
// BOOLEAN
// CmpSyncTrees(
//   PHHIVE      SourceHive,
//   HCELL_INDEX SourceCell,
//   PHHIVE      TargetHive,
//   HCELL_INDEX TargetCell,
//   BOOLEAN     CopyVolatile);
//

#define CmpSyncTrees(s,c,t,l,f) CmpCopySyncTree(s,c,t,l,f,Sync)


//
// BOOLEAN
// CmpMergeTrees(
//   PHHIVE      SourceHive,
//   HCELL_INDEX SourceCell,
//   PHHIVE      TargetHive,
//   HCELL_INDEX TargetCell);
//

#define CmpMergeTrees(s,c,t,l) CmpCopySyncTree(s,c,t,l,FALSE,Merge)

VOID
CmpDeleteTree(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

VOID
CmpSetVersionData(
    VOID
    );

NTSTATUS
CmpInitializeHardwareConfiguration(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
CmpInitializeMachineDependentConfiguration(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
CmpInitializeRegistryNode(
    IN PCONFIGURATION_COMPONENT_DATA CurrentEntry,
    IN HANDLE ParentHandle,
    OUT PHANDLE NewHandle,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN PUSHORT DeviceIndexTable
    );

NTSTATUS
CmpInitializeHive(
    PCMHIVE         *CmHive,
    ULONG           OperationType,
    ULONG           HiveFlags,
    ULONG           FileType,
    PVOID           HiveData OPTIONAL,
    HANDLE          Primary,
    HANDLE          Log,
    HANDLE          External,
    PUNICODE_STRING FileName OPTIONAL,
    ULONG           CheckFlags
    );

LOGICAL
CmpDestroyHive(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell
    );

VOID
CmpInitializeRegistryNames(
    VOID
    );

VOID
CmpInitializeCache(
    VOID
    );

#define CMP_CREATE_KCB_FAKE                     1
#define CMP_CREATE_KCB_KCB_LOCKED               2
#define CMP_DO_OPEN_COMPLETE_KEY_CACHED         4

PCM_KEY_CONTROL_BLOCK
CmpCreateKeyControlBlock(
    PHHIVE                  Hive,
    HCELL_INDEX             Cell,
    PCM_KEY_NODE            Node,
    PCM_KEY_CONTROL_BLOCK   ParentKcb,
    ULONG                   ControlFlags,
    PUNICODE_STRING         KeyName
    );

VOID CmpCleanUpKCBCacheTable(PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
                             BOOLEAN                    RegLockHeldEx);

typedef struct _QUERY_OPEN_SUBKEYS_CONTEXT {
    ULONG       BufferLength;
    PVOID       Buffer;
    ULONG       RequiredSize;
    NTSTATUS    StatusCode;
    ULONG       UsedLength;
	PVOID		KeyBodyToIgnore;
    PVOID       CurrentNameBuffer;
} QUERY_OPEN_SUBKEYS_CONTEXT, *PQUERY_OPEN_SUBKEYS_CONTEXT;

ULONG
CmpSearchForOpenSubKeys(
    IN PCM_KEY_CONTROL_BLOCK    SearchKey,
    IN SUBKEY_SEARCH_TYPE       SearchType,
    IN BOOLEAN                  RegLockHeldEx,
    IN OUT PVOID                SearchContext OPTIONAL
    );

VOID
CmpDereferenceKeyControlBlock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );

VOID
CmpRemoveKeyControlBlock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );

VOID
CmpReportNotify(
    PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    ULONG           NotifyMask
    );

VOID
CmpPostNotify(
    PCM_NOTIFY_BLOCK    NotifyBlock,
    PUNICODE_STRING     Name OPTIONAL,
    ULONG               Filter,
    NTSTATUS            Status,
    BOOLEAN             PostListLockHeld,
    PLIST_ENTRY         ExternalKeyDeref OPTIONAL
#if DBG
    ,
    PCMHIVE             CmHive
#endif 
    );

PCM_POST_BLOCK
CmpAllocatePostBlock(
    IN POST_BLOCK_TYPE BlockType,
    IN ULONG           PostFlags,
    IN PCM_KEY_BODY    KeyBody,
    IN PCM_POST_BLOCK  MasterBlock
    );

//
// PCM_POST_BLOCK
// CmpAllocateMasterPostBlock(
//    IN POST_BLOCK_TYPE BlockType
//     );
//
#define CmpAllocateMasterPostBlock(b) CmpAllocatePostBlock(b,REG_NOTIFY_MASTER_POST,NULL,NULL)

//
// PCM_POST_BLOCK
// CmpAllocateSlavePostBlock(
//    IN POST_BLOCK_TYPE BlockType,
//    IN PCM_KEY_BODY     KeyBody,
//    IN PCM_POST_BLOCK  MasterBlock
//     );
//
#define CmpAllocateSlavePostBlock(b,k,m) CmpAllocatePostBlock(b,0,k,m)

VOID
CmpFreePostBlock(
    IN PCM_POST_BLOCK PostBlock
    );

VOID
CmpPostApc(
    struct _KAPC *Apc,
    PKNORMAL_ROUTINE *NormalRoutine,
    PVOID *NormalContext,
    PVOID *SystemArgument1,
    PVOID *SystemArgument2
    );

VOID
CmpFlushNotify(
    PCM_KEY_BODY    KeyBody,
    BOOLEAN         LockHeld
    );

VOID
CmpPostApcRunDown(
    struct _KAPC *Apc
    );

NTSTATUS
CmpOpenHiveFiles(
    PUNICODE_STRING     BaseName,
    PWSTR               Extension OPTIONAL,
    PHANDLE             Primary,
    PHANDLE             Secondary,
    PULONG              PrimaryDisposition,
    PULONG              SecondaryDispoition,
    BOOLEAN             CreateAllowed,
    BOOLEAN             MarkAsSystemHive,
    BOOLEAN             NoBuffering,
    PULONG              ClusterSize
    );

NTSTATUS
CmpLinkHiveToMaster(
    PUNICODE_STRING LinkName,
    HANDLE RootDirectory,
    PCMHIVE CmHive,
    BOOLEAN Allocate,
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTSTATUS
CmpSaveBootControlSet(
     IN USHORT ControlSetNum
     );

//
// checkout procedure
//

//
// Flags to be passed to CmCheckRegistry
//
#define     CM_CHECK_REGISTRY_CHECK_CLEAN       0x00000001
#define     CM_CHECK_REGISTRY_FORCE_CLEAN       0x00000002
#define     CM_CHECK_REGISTRY_LOADER_CLEAN      0x00000004
#define     CM_CHECK_REGISTRY_SYSTEM_CLEAN      0x00000008
#define     CM_CHECK_REGISTRY_HIVE_CHECK        0x00010000
#define     CM_DONT_ADD_TO_HIVE_LIST            0x01000000

ULONG
CmCheckRegistry(
    PCMHIVE CmHive,
    ULONG   Flags
    );

BOOLEAN
CmpValidateHiveSecurityDescriptors(
    IN PHHIVE       Hive,
    OUT PBOOLEAN    ResetSD
    );

//
// cmboot - functions for determining driver load lists
//

#define CM_HARDWARE_PROFILE_STR_DATABASE L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\IDConfigDB"
#define CM_HARDWARE_PROFILE_STR_CCS_HWPROFILE L"\\Registry\\Machine\\System\\CurrentControlSet\\Hardware Profiles"
#define CM_HARDWARE_PROFILE_STR_CCS_CURRENT L"\\Registry\\Machine\\System\\CurrentControlSet\\Hardware Profiles\\Current"
//
// Alias table key names in IDConfigDB
//
#define CM_HARDWARE_PROFILE_STR_ALIAS L"Alias"
#define CM_HARDWARE_PROFILE_STR_ACPI_ALIAS L"AcpiAlias"
#define CM_HARDWARE_PROFILE_STR_HARDWARE_PROFILES L"Hardware Profiles"

//
// Entries in the alias tables (value names)
//
#define CM_HARDWARE_PROFILE_STR_DOCKING_STATE L"DockingState"
#define CM_HARDWARE_PROFILE_STR_CAPABILITIES L"Capabilities"
#define CM_HARDWARE_PROFILE_STR_DOCKID L"DockID"
#define CM_HARDWARE_PROFILE_STR_SERIAL_NUMBER L"SerialNumber"
#define CM_HARDWARE_PROFILE_STR_ACPI_SERIAL_NUMBER L"AcpiSerialNumber"
#define CM_HARDWARE_PROFILE_STR_PROFILE_NUMBER L"ProfileNumber"
#define CM_HARDWARE_PROFILE_STR_ALIASABLE L"Aliasable"
#define CM_HARDWARE_PROFILE_STR_CLONED L"Cloned"
//
// Entries in the profile tables.
//
#define CM_HARDWARE_PROFILE_STR_PRISTINE L"Pristine"
#define CM_HARDWARE_PROFILE_STR_PREFERENCE_ORDER L"PreferenceOrder"
#define CM_HARDWARE_PROFILE_STR_FRIENDLY_NAME L"FriendlyName"
#define CM_HARDWARE_PROFILE_STR_CURRENT_DOCK_INFO L"CurrentDockInfo"
#define CM_HARDWARE_PROFILE_STR_HW_PROFILE_GUID L"HwProfileGuid"
//
// Entries for the root Hardware Profiles key.
//
#define CM_HARDWARE_PROFILE_STR_DOCKED L"Docked"
#define CM_HARDWARE_PROFILE_STR_UNDOCKED L"Undocked"
#define CM_HARDWARE_PROFILE_STR_UNKNOWN L"Unknown"

//
// List structure used in config manager init
//

typedef struct _HIVE_LIST_ENTRY {
    PWSTR       Name;
    PWSTR       BaseName;                       // MACHINE or USER
    PCMHIVE     CmHive;
    ULONG       HHiveFlags;
    ULONG       CmHiveFlags;
    PCMHIVE     CmHive2;
    BOOLEAN     ThreadFinished;
    BOOLEAN     ThreadStarted;
    BOOLEAN     Allocate;
} HIVE_LIST_ENTRY, *PHIVE_LIST_ENTRY;

//
// structure definitions shared with the boot loader
// to select the hardware profile.
//
typedef struct _CM_HARDWARE_PROFILE {
    ULONG   NameLength;
    PWSTR   FriendlyName;
    ULONG   PreferenceOrder;
    ULONG   Id;
    ULONG   Flags;
} CM_HARDWARE_PROFILE, *PCM_HARDWARE_PROFILE;

#define CM_HP_FLAGS_ALIASABLE  1
#define CM_HP_FLAGS_TRUE_MATCH 2
#define CM_HP_FLAGS_PRISTINE   4
#define CM_HP_FLAGS_DUPLICATE  8

typedef struct _CM_HARDWARE_PROFILE_LIST {
    ULONG MaxProfileCount;
    ULONG CurrentProfileCount;
    CM_HARDWARE_PROFILE Profile[1];
} CM_HARDWARE_PROFILE_LIST, *PCM_HARDWARE_PROFILE_LIST;

typedef struct _CM_HARDWARE_PROFILE_ALIAS {
    ULONG   ProfileNumber;
    ULONG   DockState;
    ULONG   DockID;
    ULONG   SerialNumber;
} CM_HARDWARE_PROFILE_ALIAS, *PCM_HARDWARE_PROFILE_ALIAS;

typedef struct _CM_HARDWARE_PROFILE_ALIAS_LIST {
    ULONG MaxAliasCount;
    ULONG CurrentAliasCount;
    CM_HARDWARE_PROFILE_ALIAS Alias[1];
} CM_HARDWARE_PROFILE_ALIAS_LIST, *PCM_HARDWARE_PROFILE_ALIAS_LIST;

typedef struct _CM_HARDWARE_PROFILE_ACPI_ALIAS {
    ULONG   ProfileNumber;
    ULONG   DockState;
    ULONG   SerialLength;
    PCHAR   SerialNumber;
} CM_HARDWARE_PROFILE_ACPI_ALIAS, *PCM_HARDWARE_PROFILE_ACPI_ALIAS;

typedef struct _CM_HARDWARE_PROFILE_ACPI_ALIAS_LIST {
    ULONG   MaxAliasCount;
    ULONG   CurrentAliasCount;
    CM_HARDWARE_PROFILE_ACPI_ALIAS Alias[1];
} CM_HARDWARE_PROFILE_ACPI_ALIAS_LIST, *PCM_HARDWARE_PROFILE_ACPI_ALIAS_LIST;

HCELL_INDEX
CmpFindControlSet(
     IN PHHIVE SystemHive,
     IN HCELL_INDEX RootCell,
     IN PUNICODE_STRING SelectName,
     OUT PBOOLEAN AutoSelect
     );

BOOLEAN
CmpValidateSelect(
     IN PHHIVE SystemHive,
     IN HCELL_INDEX RootCell
     );

BOOLEAN
CmpFindDrivers(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    IN SERVICE_LOAD_TYPE LoadType,
    IN PWSTR BootFileSystem OPTIONAL,
    IN PLIST_ENTRY DriverListHead
    );

BOOLEAN
CmpFindNLSData(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    OUT PUNICODE_STRING AnsiFilename,
    OUT PUNICODE_STRING OemFilename,
    OUT PUNICODE_STRING CaseTableFilename,
    OUT PUNICODE_STRING OemHalFilename
    );

HCELL_INDEX
CmpFindProfileOption(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    OUT PCM_HARDWARE_PROFILE_LIST *ProfileList,
    OUT PCM_HARDWARE_PROFILE_ALIAS_LIST *AliasList,
    OUT PULONG Timeout
    );

VOID
CmpSetCurrentProfile(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    IN PCM_HARDWARE_PROFILE Profile
    );

BOOLEAN
CmpResolveDriverDependencies(
    IN PLIST_ENTRY DriverListHead
    );

BOOLEAN
CmpSortDriverList(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    IN PLIST_ENTRY DriverListHead
    );

HCELL_INDEX
CmpFindSubKeyByName(
    PHHIVE          Hive,
    PCM_KEY_NODE    Parent,
    PUNICODE_STRING SearchName
    );

HCELL_INDEX
CmpFindSubKeyByNumber(
    PHHIVE          Hive,
    PCM_KEY_NODE    Parent,
    ULONG           Number
    );

BOOLEAN
CmpAddSubKey(
    PHHIVE          Hive,
    HCELL_INDEX     Parent,
    HCELL_INDEX     Child
    );

BOOLEAN
CmpMarkIndexDirty(
    PHHIVE          Hive,
    HCELL_INDEX     ParentKey,
    HCELL_INDEX     TargetKey
    );

BOOLEAN
CmpRemoveSubKey(
    PHHIVE          Hive,
    HCELL_INDEX     ParentKey,
    HCELL_INDEX     TargetKey
    );

BOOLEAN
CmpGetNextName(
    IN OUT PUNICODE_STRING  RemainingName,
    OUT    PUNICODE_STRING  NextName,
    OUT    PBOOLEAN  Last
    );

NTSTATUS
CmpAddToHiveFileList(
    PCMHIVE CmHive
    );

VOID
CmpRemoveFromHiveFileList(
                          PUNICODE_STRING HiveName
    );

NTSTATUS
CmpInitHiveFromFile(
    IN PUNICODE_STRING FileName,
    IN ULONG HiveFlags,
    OUT PCMHIVE *CmHive,
    IN OUT PBOOLEAN Allocate,
    IN  ULONG       CheckFlags
    );

NTSTATUS
CmpCloneHwProfile (
    IN HANDLE IDConfigDB,
    IN HANDLE Parent,
    IN HANDLE OldProfile,
    IN ULONG  OldProfileNumber,
    IN USHORT DockingState,
    OUT PHANDLE NewProfile,
    OUT PULONG  NewProfileNumber
    );

NTSTATUS
CmpCreateHwProfileFriendlyName (
    IN HANDLE IDConfigDB,
    IN ULONG  DockingState,
    IN ULONG  NewProfileNumber,
    OUT PUNICODE_STRING FriendlyName
    );

typedef
NTSTATUS
(*PCM_ACPI_SELECTION_ROUTINE) (
    IN  PCM_HARDWARE_PROFILE_LIST ProfileList,
    OUT PULONG ProfileIndexToUse, // Set to -1 for none.
    IN  PVOID Context
    );

NTSTATUS
CmSetAcpiHwProfile (
    IN  PPROFILE_ACPI_DOCKING_STATE DockState,
    IN  PCM_ACPI_SELECTION_ROUTINE,
    IN  PVOID Context,
    OUT PHANDLE NewProfile,
    OUT PBOOLEAN ProfileChanged
    );

NTSTATUS
CmpAddAcpiAliasEntry (
    IN HANDLE                       IDConfigDB,
    IN PPROFILE_ACPI_DOCKING_STATE  NewDockState,
    IN ULONG                        ProfileNumber,
    IN PWCHAR                       nameBuffer,
    IN PVOID                        valueBuffer,
    IN ULONG                        valueBufferLength,
    IN BOOLEAN                      PreventDuplication
    );

//
// Routines for handling registry compressed names
//
USHORT
CmpNameSize(
    IN PHHIVE Hive,
    IN PUNICODE_STRING Name
    );

USHORT
CmpCopyName(
    IN PHHIVE Hive,
    IN PWCHAR Destination,
    IN PUNICODE_STRING Source
    );

VOID
CmpCopyCompressedName(
    IN PWCHAR Destination,
    IN ULONG DestinationLength,
    IN PWCHAR Source,
    IN ULONG SourceLength
    );

USHORT
CmpCompressedNameSize(
    IN PWCHAR Name,
    IN ULONG Length
    );


//
// ----- CACHED_DATA -----
//
// When values are not cached, List in ValueCache is the Hive cell index to the value list.
// When they are cached, List will be pointer to the allocation.  We distinguish them by
// marking the lowest bit in the variable to indicate it is a cached allocation.
//
// Note that the cell index for value list
// is stored in the cached allocation.  It is not used now but may be in further performance
// optimization.
//
// When value key and vaule data are cached, there is only one allocation for both.
// Value data is appended that the end of value key.  DataCacheType indicates
// whether data is cached and ValueKeySize tells how big is the value key (so
// we can calculate the address of cached value data)
//
//

PCM_NAME_CONTROL_BLOCK
CmpGetNameControlBlock(
    PUNICODE_STRING NodeName,
    PBOOLEAN        NameUpCase
    );

VOID
CmpDereferenceKeyControlBlockWithLock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock,
    BOOLEAN                 RegLockHeldEx
    );

VOID
CmpCleanUpSubKeyInfo(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );

VOID
CmpCleanUpKcbValueCache(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );


VOID
CmpRebuildKcbCache(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );



/* - macro
VOID
CmpSetUpKcbValueCache(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock,
    ULONG                   Count,
    ULONG_PTR               ValueList
    )
*/
#define CmpSetUpKcbValueCache(KeyControlBlock,_Count,_List)                 \
    ASSERT( !(CMP_IS_CELL_CACHED(KeyControlBlock->ValueCache.ValueList)) ); \
    ASSERT( !(KeyControlBlock->ExtFlags & CM_KCB_SYM_LINK_FOUND) );         \
    KeyControlBlock->ValueCache.Count = (ULONG)(_Count);                    \
    KeyControlBlock->ValueCache.ValueList = (ULONG_PTR)(_List)

VOID
CmpCleanUpKcbCacheWithLock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock,
    BOOLEAN                 RegLockHeldEx
    );

VOID
CmpRemoveFromDelayedClose(
    IN PCM_KEY_CONTROL_BLOCK kcb
    );

PUNICODE_STRING
CmpConstructName(
    PCM_KEY_CONTROL_BLOCK kcb
);

typedef enum _VALUE_SEARCH_RETURN_TYPE {
    SearchSuccess = 0,
    SearchNeedExclusiveLock = 1,
    SearchFail = 2
} VALUE_SEARCH_RETURN_TYPE;

VALUE_SEARCH_RETURN_TYPE
CmpGetValueListFromCache(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    OUT PCELL_DATA          *List,
    OUT BOOLEAN             *IndexCached,
    OUT PHCELL_INDEX        ValueListToRelease
);

VALUE_SEARCH_RETURN_TYPE
CmpGetValueKeyFromCache(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PCELL_DATA           List,
    IN ULONG                Index,
    OUT PPCM_CACHED_VALUE   *ContainingList,
    OUT PCM_KEY_VALUE       *Value,
    IN BOOLEAN              IndexCached,
    OUT BOOLEAN             *ValueCached,
    OUT PHCELL_INDEX        CellToRelease
);

VALUE_SEARCH_RETURN_TYPE
CmpFindValueByNameFromCache(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN PUNICODE_STRING          Name,
    OUT PPCM_CACHED_VALUE       *ContainingList,
    OUT ULONG                   *Index,
    OUT PCM_KEY_VALUE           *Value,
    OUT BOOLEAN                 *ValueCached,
    OUT PHCELL_INDEX            CellToRelease
    );

VALUE_SEARCH_RETURN_TYPE
CmpGetValueDataFromCache(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PPCM_CACHED_VALUE    ContainingList,
    IN PCELL_DATA           ValueKey,
    IN BOOLEAN              ValueCached,
    OUT PUCHAR              *DataPointer,
    OUT PBOOLEAN            Allocated,
    OUT PHCELL_INDEX        CellToRelease
);

VALUE_SEARCH_RETURN_TYPE
CmpQueryKeyValueData(
    PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    PPCM_CACHED_VALUE   ContainingList,
    PCM_KEY_VALUE       ValueKey,
    BOOLEAN             ValueCached,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    PVOID               KeyValueInformation,
    ULONG               Length,
    PULONG              ResultLength,
    NTSTATUS            *status
    );

BOOLEAN
CmpReferenceKeyControlBlock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );

VOID
CmpInitializeKeyNameString(PCM_KEY_NODE Cell,
                           PUNICODE_STRING KeyName,
                           WCHAR *NameBuffer
                           );

VOID
CmpInitializeValueNameString(PCM_KEY_VALUE Cell,
                             PUNICODE_STRING ValueName,
                             WCHAR *NameBuffer
                             );

VOID
CmpFlushNotifiesOnKeyBodyList(
    IN PCM_KEY_CONTROL_BLOCK    kcb,
    IN BOOLEAN                  LockHeld
    );

extern ULONG CmpHashTableSize;
extern  PCM_KEY_HASH_TABLE_ENTRY    CmpCacheTable;
extern  PCM_NAME_HASH_TABLE_ENTRY   CmpNameCacheTable;

#ifdef _WANT_MACHINE_IDENTIFICATION

BOOLEAN
CmpGetBiosDateFromRegistry(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    OUT PUNICODE_STRING Date
    );

BOOLEAN
CmpGetBiosinfoFileNameFromRegistry(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    OUT PUNICODE_STRING InfName
    );


#endif


// Utility macro to set the fields of an IO_STATUS_BLOCK.  On Win64, 32bit processes
// will pass in a 32bit Iosb, and 64bit processes will pass in a 64bit Iosb.
#if defined(_WIN64)

#define CmpSetIoStatus(Iosb, s, i, UseIosb32)                              \
if ((UseIosb32)) {                                                         \
    ((PIO_STATUS_BLOCK32)(Iosb))->Status = (NTSTATUS)(s);                  \
    ((PIO_STATUS_BLOCK32)(Iosb))->Information = (ULONG)(i);                \
}                                                                          \
else {                                                                     \
    (Iosb)->Status = (s);                                                  \
    (Iosb)->Information = (i);                                             \
}                                                                          \

#else

#define CmpSetIoStatus(Iosb, s, i, UseIosb32)                              \
(Iosb)->Status = (s);                                                      \
(Iosb)->Information = (i);                                                 \

#endif

#define CmpCheckIoStatusPointer(AsyncUser)                                              \
    if( (PVOID)((AsyncUser).IoStatusBlock) == (PVOID)(&((AsyncUser).IoStatusBlock)) ) { \
        DbgPrint("IoStatusBlock pointing onto itself AsyncUser = %p\n",&(AsyncUser));   \
        DbgBreakPoint();                                                                \
    }


// new function prototypes

NTSTATUS
CmpAcquireFileObjectForFile(
        IN  PCMHIVE         CmHive,
        IN HANDLE           FileHandle,
        OUT PFILE_OBJECT    *FileObject
            );

VOID
CmpDropFileObjectForHive(
        IN  PCMHIVE             CmHive
            );

VOID
CmpTouchView(
    IN PCMHIVE              CmHive,
    IN PCM_VIEW_OF_FILE     CmView,
    IN ULONG                Cell
            );

NTSTATUS
CmpMapCmView(
    IN  PCMHIVE             CmHive,
    IN  ULONG               FileOffset,
    OUT PCM_VIEW_OF_FILE    *CmView,
    IN  BOOLEAN             MapInited
    );

VOID
CmpInitHiveViewList (
        IN  PCMHIVE             CmHive
                             );

VOID
CmpDestroyHiveViewList (
        IN  PCMHIVE             CmHive
                             );

NTSTATUS
CmpPinCmView (
        IN  PCMHIVE             CmHive,
        PCM_VIEW_OF_FILE        CmView
                             );
VOID
CmpUnPinCmView (
        IN  PCMHIVE             CmHive,
        IN  PCM_VIEW_OF_FILE    CmView,
        IN  BOOLEAN             SetClean,
        IN  BOOLEAN             MapIsValid
                             );

NTSTATUS
CmpMapThisBin(
                PCMHIVE         CmHive,
                HCELL_INDEX     Cell,
                BOOLEAN         Touch
              );
VOID
CmpReferenceHiveView(   IN PCMHIVE          CmHive,
                        IN PCM_VIEW_OF_FILE CmView
                     );
VOID
CmpDereferenceHiveView(   IN PCMHIVE          CmHive,
                        IN PCM_VIEW_OF_FILE CmView
                     );

VOID
CmpReferenceHiveViewWithLock(   IN PCMHIVE          CmHive,
                                IN PCM_VIEW_OF_FILE CmView
                            );

VOID
CmpDereferenceHiveViewWithLock(     IN PCMHIVE          CmHive,
                                    IN PCM_VIEW_OF_FILE CmView
                                );

VOID
CmpInitializeDelayedCloseTable();

VOID
CmpAddToDelayedClose(
    IN PCM_KEY_CONTROL_BLOCK    kcb,
    IN BOOLEAN                  RegLockHeldEx
    );

NTSTATUS
CmpAddValueToList(
    IN PHHIVE  Hive,
    IN HCELL_INDEX ValueCell,
    IN ULONG Index,
    IN ULONG Type,
    IN OUT PCHILD_LIST ChildList
    );

NTSTATUS
CmpRemoveValueFromList(
    IN PHHIVE  Hive,
    IN ULONG Index,
    IN OUT PCHILD_LIST ChildList
    );

BOOLEAN
CmpGetValueData(IN PHHIVE Hive,
                IN PCM_KEY_VALUE Value,
                OUT PULONG realsize,
                IN OUT PVOID *Buffer,
                OUT PBOOLEAN Allocated,
                OUT PHCELL_INDEX CellToRelease
               );

PCELL_DATA
CmpValueToData(IN PHHIVE Hive,
               IN PCM_KEY_VALUE Value,
               OUT PULONG realsize
               );

BOOLEAN
CmpMarkValueDataDirty(  IN PHHIVE Hive,
                        IN PCM_KEY_VALUE Value
                      );

NTSTATUS
CmpSetValueDataNew(
    IN PHHIVE           Hive,
    IN PVOID            Data,
    IN ULONG            DataSize,
    IN ULONG            StorageType,
    IN HCELL_INDEX      ValueCell,
    OUT PHCELL_INDEX    DataCell
    );

NTSTATUS
CmpSetValueDataExisting(
    IN PHHIVE           Hive,
    IN PVOID            Data,
    IN ULONG            DataSize,
    IN ULONG            StorageType,
    IN HCELL_INDEX      OldDataCell
    );

BOOLEAN
CmpFreeValueData(
    PHHIVE      Hive,
    HCELL_INDEX DataCell,
    ULONG       DataLength
    );


NTSTATUS
CmpAddSecurityCellToCache (
    IN OUT PCMHIVE              CmHive,
    IN HCELL_INDEX              SecurityCell,
    IN BOOLEAN                  BuildUp,
    IN PCM_KEY_SECURITY_CACHE   SecurityCached
    );

BOOLEAN
CmpFindSecurityCellCacheIndex (
    IN PCMHIVE      CmHive,
    IN HCELL_INDEX  SecurityCell,
    OUT PULONG      Index
    );

BOOLEAN
CmpAdjustSecurityCacheSize (
    IN PCMHIVE      CmHive
    );

VOID
CmpRemoveFromSecurityCache (
    IN OUT PCMHIVE      CmHive,
    IN HCELL_INDEX      SecurityCell
    );

VOID
CmpDestroySecurityCache (
    IN OUT PCMHIVE      CmHive
    );


VOID
CmpInitSecurityCache(
    IN OUT PCMHIVE      CmHive
    );

BOOLEAN
CmpRebuildSecurityCache(
                        IN OUT PCMHIVE      CmHive
                        );

ULONG
CmpSecConvKey(
              IN ULONG  DescriptorLength,
              IN PULONG Descriptor
              );

VOID
CmpAssignSecurityToKcb(
    IN PCM_KEY_CONTROL_BLOCK    Kcb,
    IN HCELL_INDEX              SecurityCell,
    IN BOOLEAN                  SecurityLocked
    );

BOOLEAN
CmpBuildSecurityCellMappingArray(
    IN PCMHIVE CmHive
    );


//
// new function replacing CmpWorker
//
VOID
CmpCmdHiveClose(
                     PCMHIVE    CmHive
                     );

VOID
CmpCmdInit(
           BOOLEAN SetupBoot
            );

NTSTATUS
CmpCmdRenameHive(
            PCMHIVE                     CmHive,
            POBJECT_NAME_INFORMATION    OldName,
            PUNICODE_STRING             NewName,
            ULONG                       NameInfoLength
            );

NTSTATUS
CmpCmdHiveOpen(
            POBJECT_ATTRIBUTES          FileAttributes,
            PSECURITY_CLIENT_CONTEXT    ImpersonationContext,
            PBOOLEAN                    Allocate,
            PCMHIVE                     *NewHive,
            ULONG                       CheckFlags
            );

ULONG
CmpComputeKcbConvKey(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );

HCELL_INDEX
CmpDuplicateIndex(
    PHHIVE          Hive,
    HCELL_INDEX     IndexCell,
    ULONG           StorageType
    );

NTSTATUS
CmRenameKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN UNICODE_STRING           NewKeyName,         
    IN KPROCESSOR_MODE          PreviousMode
    );

BOOLEAN
CmpUpdateParentForEachSon(
    PHHIVE          Hive,
    HCELL_INDEX     Parent
    );

NTSTATUS
CmUnloadKeyEx(
    IN PCM_KEY_CONTROL_BLOCK Kcb,
    IN PKEVENT UserEvent
    );

VOID
CmpShutdownWorkers(
    VOID
    );

VOID
CmpPrefetchHiveFile(
                    IN PFILE_OBJECT FileObject,
                    IN ULONG        Length
                    );

#define CmpCheckForOrphanedKcbs(Hive) //nothing

#define CM_HIVE_COMPRESS_LEVEL   (25)


#define CMP_MAX_REGISTRY_DEPTH      512        // levels

typedef struct {
    HCELL_INDEX Cell;
    HCELL_INDEX ParentCell;
    HCELL_INDEX PriorSibling;
    ULONG       ChildIndex;
    BOOLEAN     CellChecked;
} CMP_CHECK_REGISTRY_STACK_ENTRY, *PCMP_CHECK_REGISTRY_STACK_ENTRY;


#define CmIsKcbReadOnly(kcb)        ((kcb)->ExtFlags & CM_KCB_READ_ONLY_KEY)

NTSTATUS
CmLockKcbForWrite(PCM_KEY_CONTROL_BLOCK KeyControlBlock);

//
// Wrapper to RtlCompareUnicodeString; uses CompareFlags to avoid upcasing names
//

#define CMP_SOURCE_UP       0x00000001
#define CMP_DEST_UP         0x00000002

LONG
CmpCompareUnicodeString(
    IN PUNICODE_STRING  SourceName,
    IN PUNICODE_STRING  DestName,
    IN ULONG            CompareFlags
    );

LONG
CmpCompareCompressedName(
    IN PUNICODE_STRING  SearchName,
    IN PWCHAR           CompressedName,
    IN ULONG            NameLength,
    IN ULONG            CompareFlags
    );

LONG
CmpCompareTwoCompressedNames(
    IN PWCHAR           CompressedName1,
    IN ULONG            NameLength1,
    IN PWCHAR           CompressedName2,
    IN ULONG            NameLength2
    );

#define INIT_SYSTEMROOT_HIVEPATH L"\\SystemRoot\\System32\\Config\\"


ULONG
CmpComputeHashKey(
    IN ULONG            HashStart,
    IN PUNICODE_STRING  Name
#if DBG
    ,IN BOOLEAN        AllowSeparators
#endif
    );


ULONG
CmpComputeHashKeyForCompressedName(
                                    IN ULONG    HashStart,
                                    IN PWCHAR   Source,
                                    IN ULONG SourceLength
                                    );
//
// KCB allocator routines
//
VOID CmpInitCmPrivateAlloc();
VOID CmpDestroyCmPrivateAlloc();
PCM_KEY_CONTROL_BLOCK CmpAllocateKeyControlBlock( );
VOID CmpFreeKeyControlBlock( PCM_KEY_CONTROL_BLOCK kcb );

//
// delay related allocator routines
//
VOID CmpInitCmPrivateDelayAlloc();
VOID CmpDestroyCmPrivateDelayAlloc();
PVOID CmpAllocateDelayItem( );
VOID CmpFreeDelayItem( PVOID Item );

//
// make handles protected, so we control handle closure
//

#define CmpSetHandleProtection(Handle,Protection)                       \
{                                                                       \
    OBJECT_HANDLE_FLAG_INFORMATION  Ohfi = {    FALSE,                  \
                                                FALSE                   \
                                            };                          \
    Ohfi.ProtectFromClose = Protection;                                 \
    ZwSetInformationObject( Handle,                                     \
                            ObjectHandleFlagInformation,                \
                            &Ohfi,                                      \
                            sizeof (OBJECT_HANDLE_FLAG_INFORMATION));   \
}

#define CmCloseHandle(Handle)               \
    CmpSetHandleProtection(Handle,FALSE);   \
    ZwClose(Handle)


VOID
CmpUpdateSystemHiveHysteresis(  PHHIVE  Hive,
                                ULONG   NewLength,
                                ULONG   OldLength
                                );

NTSTATUS
CmpCallCallBacks (
    IN REG_NOTIFY_CLASS Type,
    IN PVOID            Argument,
    IN BOOLEAN          Wind,
    IN REG_NOTIFY_CLASS PostType,
    IN PVOID            Object
    );

extern ULONG CmpCallBackCount;

#define CmAreCallbacksRegistered() ((CmpCallBackCount != 0) && (0 == ExIsResourceAcquiredShared(&CmpRegistryLock)))

#define CmPostCallbackNotification(Type,_Object_,_Status_)      \
    if( CmAreCallbacksRegistered() ) {                          \
        REG_POST_OPERATION_INFORMATION PostInfo;                \
        PostInfo.Object = _Object_;                             \
        PostInfo.Status = _Status_;                             \
        CmpCallCallBacks(Type,&PostInfo,FALSE,Type,_Object_);   \
    }

//
// Self healing hives control switch
//
extern BOOLEAN  CmpSelfHeal;
extern ULONG    CmpBootType;

#define CmDoSelfHeal() (CmpSelfHeal || (CmpBootType & (HBOOT_BACKUP|HBOOT_SELFHEAL)))

#define CmMarkSelfHeal(Hive) ( (Hive)->BaseBlock->BootType |= HBOOT_SELFHEAL )

BOOLEAN
CmpRemoveSubKeyCellNoCellRef(
    PHHIVE          Hive,
    HCELL_INDEX     Parent,
    HCELL_INDEX     Child
    );

VOID 
CmpRaiseSelfHealWarning( 
                        IN PUNICODE_STRING  HiveName
                        );

VOID 
CmpRaiseSelfHealWarningForSystemHives();


//
// Tracking quota leaks helpers
//
#define CM_TRACK_QUOTA_START() //nothing
#define CM_TRACK_QUOTA_STOP()  //nothing

//
// PERF: try inline ascii upcase
//
#define CmUpcaseUnicodeChar(c)          \
( ((c) < 'a') ? (c) : ( ((c) > 'z') ? RtlUpcaseUnicodeChar(c) : ((c) - ('a'-'A')) ) )


//
// Mini NT boot indicator
//
extern BOOLEAN CmpMiniNTBoot;
extern BOOLEAN CmpShareSystemHives;


//
// new per- KCB lock
//
#define CmpLockKCBShared(Kcb)               ExAcquirePushLockShared(&(GET_KCB_HASH_ENTRY_LOCK((Kcb))))  

#define CmpLockKCBExclusive(Kcb)            ExAcquirePushLockExclusive(&(GET_KCB_HASH_ENTRY_LOCK((Kcb))));\
                                            GET_KCB_HASH_ENTRY_OWNER(Kcb) = KeGetCurrentThread()

#define CmpUnlockKCB(Kcb)                   GET_KCB_HASH_ENTRY_OWNER(Kcb) = NULL;\
                                            ExReleasePushLock(&(GET_KCB_HASH_ENTRY_LOCK((Kcb))))

#define CmpIsKCBLockedExclusive(Kcb)        (GET_KCB_HASH_ENTRY_OWNER(Kcb) == KeGetCurrentThread())

#define CmpLockHashEntryShared(ConvKey)     ExAcquirePushLockShared(&(GET_HASH_ENTRY(CmpCacheTable,(ConvKey)).Lock))

#define CmpLockHashEntryExclusive(ConvKey)  ExAcquirePushLockExclusive(&(GET_HASH_ENTRY(CmpCacheTable,(ConvKey)).Lock));\
                                            GET_HASH_ENTRY(CmpCacheTable,(ConvKey)).Owner = KeGetCurrentThread()

#define CmpUnlockHashEntry(ConvKey)         GET_HASH_ENTRY(CmpCacheTable,(ConvKey)).Owner = NULL;\
                                            ExReleasePushLock(&(GET_HASH_ENTRY(CmpCacheTable,(ConvKey)).Lock))

#define CmpLockHashEntryByIndexShared(Index)    ExAcquirePushLockShared(&(CmpCacheTable[(Index)].Lock))

#define CmpLockHashEntryByIndexExclusive(Index) ExAcquirePushLockExclusive(&(CmpCacheTable[(Index)].Lock)); \
                                                CmpCacheTable[(Index)].Owner = KeGetCurrentThread()

BOOLEAN CmpTryToLockHashEntryByIndexExclusive(ULONG   Index);

#define CmpUnlockHashEntryByIndex(Index)        CmpCacheTable[(Index)].Owner = NULL; \
                                                ExReleasePushLock(&(CmpCacheTable[(Index)].Lock))

#define CmpLockNameHashEntryExclusive(ConvKey)  ExAcquirePushLockExclusive(&(GET_HASH_ENTRY(CmpNameCacheTable,(ConvKey)).Lock))
#define CmpUnlockNameHashEntry(ConvKey)         ExReleasePushLock(&(GET_HASH_ENTRY(CmpNameCacheTable,(ConvKey)).Lock))

BOOLEAN CmpTryConvertKCBLockSharedToExclusive(PCM_KEY_CONTROL_BLOCK    KeyControlBlock);

#define ASSERT_KCB_LOCKED(kcb) //nothing
#define ASSERT_HASH_ENTRY_LOCKED(ConvKey) //nothing

#define ASSERT_KCB_LOCKED_EXCLUSIVE(kcb) ASSERT( (CmpIsKCBLockedExclusive(kcb) == TRUE) || (CmpTestRegistryLockExclusive() == TRUE) )
#define ASSERT_HASH_ENTRY_LOCKED_EXCLUSIVE(ConvKey) ASSERT( (GET_HASH_ENTRY(CmpCacheTable,(ConvKey)).Owner == KeGetCurrentThread()) || (CmpTestRegistryLockExclusive() == TRUE) )


#define CmpUpgradeKCBLockToExclusive(kcb)   ASSERT( CmpIsKCBLockedExclusive(kcb) == FALSE ); \
                                            CmpUnlockKCB(kcb);                               \
                                            CmpLockKCBExclusive(kcb)

BOOLEAN
CmpKCBLockForceAcquireAllowed(ULONG Index1,
                              ULONG Index2,
                              ULONG NewIndex);

VOID
CmpLockTwoHashEntriesExclusive(
    ULONG   ConvKey1,
    ULONG   ConvKey2
    );

VOID
CmpLockTwoHashEntriesShared(
    ULONG   ConvKey1,
    ULONG   ConvKey2
    );

VOID
CmpUnlockTwoHashEntries(
    ULONG   ConvKey1,
    ULONG   ConvKey2
    );
//
// DelayDerefKCB engine
//
VOID CmpInitDelayDerefKCBEngine();
VOID CmpRunDownDelayDerefKCBEngine(PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
                                   BOOLEAN                  RegLockHeldEx);
VOID CmpDelayDerefKeyControlBlock( PCM_KEY_CONTROL_BLOCK KeyControlBlock);

#define SECOND_MULT 10*1000*1000        // 10->mic, 1000->mil, 1000->second

#define ASSERT_RESOURCE_NOT_OWNED(Res) ASSERT( (ExIsResourceAcquiredShared(Res) == 0) && (ExIsResourceAcquiredExclusiveLite(Res) == 0) )
#define ASSERT_RESOURCE_OWNED(Res) ASSERT( (ExIsResourceAcquiredShared(Res) != 0) || (ExIsResourceAcquiredExclusiveLite(Res) != 0) )

#define CmpFreeResource(Resource)   ASSERT( Resource );                     \
                                    ASSERT_RESOURCE_NOT_OWNED( Resource );  \
                                    ExDeleteResourceLite( Resource );       \
                                    ExFreePool(Resource)

#define CmpFreeMutex( _mutex_ )     ASSERT( _mutex_ );                      \
                                    ExFreePool(_mutex_)


typedef struct _CM_DELAY_ALLOC {
    LIST_ENTRY              ListEntry;
    PCM_KEY_CONTROL_BLOCK   Kcb;
} CM_DELAY_ALLOC, *PCM_DELAY_ALLOC;

#define CM_PAGED_CODE() PAGED_CODE()

extern  PCMHIVE     CmpMasterHive;
extern  LIST_ENTRY  CmpHiveListHead;   // List of CMHIVEs

#endif //_CMP_
