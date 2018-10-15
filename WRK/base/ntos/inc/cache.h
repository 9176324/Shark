/*++ BUILD Version: 0003    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    Cache.h

Abstract:

    This module contains the public data structures and procedure
    prototypes for the cache management system.

--*/

#ifndef _CACHE_
#define _CACHE_

#include "perf.h"

// begin_ntifs
//
//  Define two constants describing the view size (and alignment)
//  that the Cache Manager uses to map files.
//

#define VACB_MAPPING_GRANULARITY         (0x40000)
#define VACB_OFFSET_SHIFT                (18)

//
// Public portion of BCB
//

typedef struct _PUBLIC_BCB {

    //
    // Type and size of this record
    //
    // NOTE: The first four fields must be the same as the BCB in cc.h.
    //

    CSHORT NodeTypeCode;
    CSHORT NodeByteSize;

    //
    // Description of range of file which is currently mapped.
    //

    ULONG MappedLength;
    LARGE_INTEGER MappedFileOffset;
} PUBLIC_BCB, *PPUBLIC_BCB;

//
//  File Sizes structure.
//

typedef struct _CC_FILE_SIZES {

    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER FileSize;
    LARGE_INTEGER ValidDataLength;

} CC_FILE_SIZES, *PCC_FILE_SIZES;

//
// Define a Cache Manager callback structure.  These routines are required
// by the Lazy Writer, so that it can acquire resources in the right order
// to avoid deadlocks.  Note that otherwise you would have most FS requests
// acquiring FS resources first and caching structures second, while the
// Lazy Writer needs to acquire its own resources first, and then FS
// structures later as it calls the file system.
//

//
// First define the procedure pointer typedefs
//

//
// This routine is called by the Lazy Writer prior to doing a write,
// since this will require some file system resources associated with
// this cached file. The context parameter supplied is whatever the FS
// passed as the LazyWriteContext parameter when is called
// CcInitializeCacheMap.
//

typedef
BOOLEAN (*PACQUIRE_FOR_LAZY_WRITE) (
     __in PVOID Context,
     __in BOOLEAN Wait
     );

//
// This routine releases the Context acquired above.
//

typedef
VOID (*PRELEASE_FROM_LAZY_WRITE) (
     __in PVOID Context
     );

//
// This routine is called by the Lazy Writer prior to doing a readahead.
//

typedef
BOOLEAN (*PACQUIRE_FOR_READ_AHEAD) (
     __in PVOID Context,
     __in BOOLEAN Wait
     );

//
// This routine releases the Context acquired above.
//

typedef
VOID (*PRELEASE_FROM_READ_AHEAD) (
     __in PVOID Context
     );

typedef struct _CACHE_MANAGER_CALLBACKS {

    PACQUIRE_FOR_LAZY_WRITE AcquireForLazyWrite;
    PRELEASE_FROM_LAZY_WRITE ReleaseFromLazyWrite;
    PACQUIRE_FOR_READ_AHEAD AcquireForReadAhead;
    PRELEASE_FROM_READ_AHEAD ReleaseFromReadAhead;

} CACHE_MANAGER_CALLBACKS, *PCACHE_MANAGER_CALLBACKS;

//
//  This structure is passed into CcUninitializeCacheMap
//  if the caller wants to know when the cache map is deleted.
//

typedef struct _CACHE_UNINITIALIZE_EVENT {
    struct _CACHE_UNINITIALIZE_EVENT *Next;
    KEVENT Event;
} CACHE_UNINITIALIZE_EVENT, *PCACHE_UNINITIALIZE_EVENT;

//
// Callback routine for retrieving dirty pages from Cache Manager.
//

typedef
VOID (*PDIRTY_PAGE_ROUTINE) (
    __in PFILE_OBJECT FileObject,
    __in PLARGE_INTEGER FileOffset,
    __in ULONG Length,
    __in PLARGE_INTEGER OldestLsn,
    __in PLARGE_INTEGER NewestLsn,
    __in PVOID Context1,
    __in PVOID Context2
    );

//
// Callback routine for doing log file flushes to Lsn.
//

typedef
VOID (*PFLUSH_TO_LSN) (
    __in PVOID LogHandle,
    __in LARGE_INTEGER Lsn
    );

//
// Macro to test whether a file is cached or not.
//

#define CcIsFileCached(FO) (                                                         \
    ((FO)->SectionObjectPointer != NULL) &&                                          \
    (((PSECTION_OBJECT_POINTERS)(FO)->SectionObjectPointer)->SharedCacheMap != NULL) \
)

// end_ntifs
//
// Throw away miss counter
//

extern ULONG CcThrowAway;

//
// Performance Counters
//

extern ULONG CcFastReadNoWait;
extern ULONG CcFastReadWait;
extern ULONG CcFastReadResourceMiss;
extern ULONG CcFastReadNotPossible;

extern ULONG CcFastMdlReadNoWait;
extern ULONG CcFastMdlReadWait;             // ntifs
extern ULONG CcFastMdlReadResourceMiss;
extern ULONG CcFastMdlReadNotPossible;

extern ULONG CcMapDataNoWait;
extern ULONG CcMapDataWait;
extern ULONG CcMapDataNoWaitMiss;
extern ULONG CcMapDataWaitMiss;

extern ULONG CcPinMappedDataCount;

extern ULONG CcPinReadNoWait;
extern ULONG CcPinReadWait;
extern ULONG CcPinReadNoWaitMiss;
extern ULONG CcPinReadWaitMiss;

extern ULONG CcCopyReadNoWait;
extern ULONG CcCopyReadWait;
extern ULONG CcCopyReadNoWaitMiss;
extern ULONG CcCopyReadWaitMiss;

extern ULONG CcMdlReadNoWait;
extern ULONG CcMdlReadWait;
extern ULONG CcMdlReadNoWaitMiss;
extern ULONG CcMdlReadWaitMiss;

extern ULONG CcReadAheadIos;

extern ULONG CcLazyWriteIos;
extern ULONG CcLazyWritePages;
extern ULONG CcDataFlushes;
extern ULONG CcDataPages;

extern ULONG CcLostDelayedWrites;

extern PULONG CcMissCounter;

//
// Global Maintenance routines
//

NTKERNELAPI
BOOLEAN
CcInitializeCacheManager (
    VOID
    );

LOGICAL
CcHasInactiveViews (
    VOID
    );

LOGICAL
CcUnmapInactiveViews (
    IN ULONG NumberOfViewsToUnmap
    );

VOID
CcWaitForUninitializeCacheMap (
    IN PFILE_OBJECT FileObject
    );

// begin_ntifs
//
// The following routines are intended for use by File Systems Only.
//

NTKERNELAPI
VOID
CcInitializeCacheMap (
    __in PFILE_OBJECT FileObject,
    __in PCC_FILE_SIZES FileSizes,
    __in BOOLEAN PinAccess,
    __in PCACHE_MANAGER_CALLBACKS Callbacks,
    __in PVOID LazyWriteContext
    );

NTKERNELAPI
BOOLEAN
CcUninitializeCacheMap (
    __in PFILE_OBJECT FileObject,
    __in_opt PLARGE_INTEGER TruncateSize,
    __in_opt PCACHE_UNINITIALIZE_EVENT UninitializeEvent
    );

NTKERNELAPI
VOID
CcSetFileSizes (
    __in PFILE_OBJECT FileObject,
    __in PCC_FILE_SIZES FileSizes
    );

//
//  VOID
//  CcFastIoSetFileSizes (
//      IN PFILE_OBJECT FileObject,
//      IN PCC_FILE_SIZES FileSizes
//      );
//

#define CcGetFileSizePointer(FO) (                                     \
    ((PLARGE_INTEGER)((FO)->SectionObjectPointer->SharedCacheMap) + 1) \
)

NTKERNELAPI
BOOLEAN
CcPurgeCacheSection (
    __in PSECTION_OBJECT_POINTERS SectionObjectPointer,
    __in_opt PLARGE_INTEGER FileOffset,
    __in ULONG Length,
    __in BOOLEAN UninitializeCacheMaps
    );

NTKERNELAPI
VOID
CcSetDirtyPageThreshold (
    __in PFILE_OBJECT FileObject,
    __in ULONG DirtyPageThreshold
    );

NTKERNELAPI
VOID
CcFlushCache (
    __in PSECTION_OBJECT_POINTERS SectionObjectPointer,
    __in_opt PLARGE_INTEGER FileOffset,
    __in ULONG Length,
    __out_opt PIO_STATUS_BLOCK IoStatus
    );

NTKERNELAPI
LARGE_INTEGER
CcGetFlushedValidData (
    __in PSECTION_OBJECT_POINTERS SectionObjectPointer,
    __in BOOLEAN BcbListHeld
    );

// end_ntifs
NTKERNELAPI
VOID
CcZeroEndOfLastPage (
    IN PFILE_OBJECT FileObject
    );

// begin_ntifs
NTKERNELAPI
BOOLEAN
CcZeroData (
    __in PFILE_OBJECT FileObject,
    __in PLARGE_INTEGER StartOffset,
    __in PLARGE_INTEGER EndOffset,
    __in BOOLEAN Wait
    );

NTKERNELAPI
PVOID
CcRemapBcb (
    __in PVOID Bcb
    );

NTKERNELAPI
VOID
CcRepinBcb (
    __in PVOID Bcb
    );

NTKERNELAPI
VOID
CcUnpinRepinnedBcb (
    __in PVOID Bcb,
    __in BOOLEAN WriteThrough,
    __out PIO_STATUS_BLOCK IoStatus
    );

NTKERNELAPI
PFILE_OBJECT
CcGetFileObjectFromSectionPtrs (
    __in PSECTION_OBJECT_POINTERS SectionObjectPointer
    );

NTKERNELAPI
PFILE_OBJECT
CcGetFileObjectFromBcb (
    __in PVOID Bcb
    );

//
// These routines are implemented to support write throttling.
//

//
//  BOOLEAN
//  CcCopyWriteWontFlush (
//      IN PFILE_OBJECT FileObject,
//      IN PLARGE_INTEGER FileOffset,
//      IN ULONG Length
//      );
//

#define CcCopyWriteWontFlush(FO,FOFF,LEN) ((LEN) <= 0X10000)

NTKERNELAPI
BOOLEAN
CcCanIWrite (
    __in PFILE_OBJECT FileObject,
    __in ULONG BytesToWrite,
    __in BOOLEAN Wait,
    __in UCHAR Retrying
    );

typedef
VOID (*PCC_POST_DEFERRED_WRITE) (
    IN PVOID Context1,
    IN PVOID Context2
    );

NTKERNELAPI
VOID
CcDeferWrite (
    __in PFILE_OBJECT FileObject,
    __in PCC_POST_DEFERRED_WRITE PostRoutine,
    __in PVOID Context1,
    __in PVOID Context2,
    __in ULONG BytesToWrite,
    __in BOOLEAN Retrying
    );

//
// The following routines provide a data copy interface to the cache, and
// are intended for use by File Servers and File Systems.
//

NTKERNELAPI
BOOLEAN
CcCopyRead (
    __in PFILE_OBJECT FileObject,
    __in PLARGE_INTEGER FileOffset,
    __in ULONG Length,
    __in BOOLEAN Wait,
    __out_bcount(Length) PVOID Buffer,
    __out PIO_STATUS_BLOCK IoStatus
    );

NTKERNELAPI
VOID
CcFastCopyRead (
    __in PFILE_OBJECT FileObject,
    __in ULONG FileOffset,
    __in ULONG Length,
    __in ULONG PageCount,
    __out_bcount(Length) PVOID Buffer,
    __out PIO_STATUS_BLOCK IoStatus
    );

NTKERNELAPI
BOOLEAN
CcCopyWrite (
    __in PFILE_OBJECT FileObject,
    __in PLARGE_INTEGER FileOffset,
    __in ULONG Length,
    __in BOOLEAN Wait,
    __in_bcount(Length) PVOID Buffer
    );

NTKERNELAPI
VOID
CcFastCopyWrite (
    __in PFILE_OBJECT FileObject,
    __in ULONG FileOffset,
    __in ULONG Length,
    __in_bcount(Length) PVOID Buffer
    );

//
//  The following routines provide an Mdl interface for transfers to and
//  from the cache, and are primarily intended for File Servers.
//
//  NOBODY SHOULD BE CALLING THESE MDL ROUTINES DIRECTLY, USE FSRTL AND
//  FASTIO INTERFACES.
//

NTKERNELAPI
VOID
CcMdlRead (
    __in PFILE_OBJECT FileObject,
    __in PLARGE_INTEGER FileOffset,
    __in ULONG Length,
    __out PMDL *MdlChain,
    __out PIO_STATUS_BLOCK IoStatus
    );

//
//  This routine is now a wrapper for FastIo if present or CcMdlReadComplete2
//

NTKERNELAPI
VOID
CcMdlReadComplete (
    __in PFILE_OBJECT FileObject,
    __in PMDL MdlChain
    );

// end_ntifs
NTKERNELAPI
VOID
CcMdlReadComplete2 (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain
    );

// begin_ntifs

NTKERNELAPI
VOID
CcPrepareMdlWrite (
    __in PFILE_OBJECT FileObject,
    __in PLARGE_INTEGER FileOffset,
    __in ULONG Length,
    __out PMDL *MdlChain,
    __out PIO_STATUS_BLOCK IoStatus
    );

//
//  This routine is now a wrapper for FastIo if present or CcMdlWriteComplete2
//

NTKERNELAPI
VOID
CcMdlWriteComplete (
    __in PFILE_OBJECT FileObject,
    __in PLARGE_INTEGER FileOffset,
    __in PMDL MdlChain
    );

NTKERNELAPI
VOID
CcMdlWriteAbort (
    __in PFILE_OBJECT FileObject,
    __in PMDL MdlChain
    );

// end_ntifs

NTKERNELAPI
VOID
CcMdlWriteComplete2 (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain
    );

// begin_ntifs
//
// Common ReadAhead call for Copy Read and Mdl Read.
//
// ReadAhead should always be invoked by calling the CcReadAhead macro,
// which tests first to see if the read is large enough to warrant read
// ahead.  Measurements have shown that, calling the read ahead routine
// actually decreases performance for small reads, such as issued by
// many compilers and linkers.  Compilers simply want all of the include
// files to stay in memory after being read the first time.
//

#define CcReadAhead(FO,FOFF,LEN) {                       \
    if ((LEN) >= 256) {                                  \
        CcScheduleReadAhead((FO),(FOFF),(LEN));          \
    }                                                    \
}

NTKERNELAPI
VOID
CcScheduleReadAhead (
    __in PFILE_OBJECT FileObject,
    __in PLARGE_INTEGER FileOffset,
    __in ULONG Length
    );

//
//  The following routine allows a caller to wait for the next batch
//  of lazy writer work to complete.  In particular, this provides a
//  mechanism for a caller to be sure that all available lazy closes
//  at the time of this call have issued.
//

NTKERNELAPI
NTSTATUS
CcWaitForCurrentLazyWriterActivity (
    VOID
    );

//
// This routine changes the read ahead granularity for a file, which is
// PAGE_SIZE by default.
//

NTKERNELAPI
VOID
CcSetReadAheadGranularity (
    __in PFILE_OBJECT FileObject,
    __in ULONG Granularity
    );

//
// The following routines provide direct access data which is pinned in the
// cache, and is primarily intended for use by File Systems.  In particular,
// this mode of access is ideal for dealing with volume structures.
//

//
//  Flags for pinning 
//
//  Note: The flags for pinning and the flags for mapping cannot overlap unless
//     the flag has the same meaning.
//

//
//  Synchronous Wait - normally specified.  This pattern may be specified as TRUE.
//

#define PIN_WAIT                         (1)

//
//  Acquire metadata Bcb exclusive (default is shared, Lazy Writer uses exclusive).
//
//  Must be set with PIN_WAIT.
//

#define PIN_EXCLUSIVE                    (2)

//
//  Acquire metadata Bcb but do not fault data in.  Default is to fault the data in.
//  This unusual flag is only used by Ntfs for cache coherency synchronization between
//  compressed and uncompressed streams for the same compressed file.
//
//  Must be set with PIN_WAIT.
//

#define PIN_NO_READ                      (4)

//
//  This option may be used to pin data only if the Bcb already exists.  If the Bcb
//  does not already exist - the pin is unsuccessful and no Bcb is returned.  This routine
//  provides a way to see if data is already pinned (and possibly dirty) in the cache,
//  without forcing a fault if the data is not there.
//

#define PIN_IF_BCB                       (8)

//
//  If this option is specified, the caller is responsible for tracking the
//  dirty ranges and calling MmSetAddressRangeModified on these ranges before
//  they are flushed.  Ranges should only be pinned via this manner if the
//  entire range will be written or purged (one or the other must occur).
//

#define PIN_CALLER_TRACKS_DIRTY_DATA      (32)

//
//  Flags for mapping
//

//
//  Synchronous Wait - normally specified.  This pattern may be specified as TRUE.
//

#define MAP_WAIT                         (1)

//
//  Acquire metadata Bcb but do not fault data in.  Default is to fault the data in.
//  This should not overlap with any of the PIN_ flags so they can be passed down to
//  CcPinFileData
//

#define MAP_NO_READ                      (16)

NTKERNELAPI
BOOLEAN
CcPinRead (
    __in PFILE_OBJECT FileObject,
    __in PLARGE_INTEGER FileOffset,
    __in ULONG Length,
    __in ULONG Flags,
    __out PVOID *Bcb,
    __deref_out_bcount(Length) PVOID *Buffer
    );

NTKERNELAPI
BOOLEAN
CcMapData (
    __in PFILE_OBJECT FileObject,
    __in PLARGE_INTEGER FileOffset,
    __in ULONG Length,
    __in ULONG Flags,
    __out PVOID *Bcb,
    __deref_out_bcount(Length) PVOID *Buffer
    );

NTKERNELAPI
BOOLEAN
CcPinMappedData (
    __in PFILE_OBJECT FileObject,
    __in PLARGE_INTEGER FileOffset,
    __in ULONG Length,
    __in ULONG Flags,
    __inout PVOID *Bcb
    );

NTKERNELAPI
BOOLEAN
CcPreparePinWrite (
    __in PFILE_OBJECT FileObject,
    __in PLARGE_INTEGER FileOffset,
    __in ULONG Length,
    __in BOOLEAN Zero,
    __in ULONG Flags,
    __out PVOID *Bcb,
    __deref_out_bcount(Length) PVOID *Buffer
    );

NTKERNELAPI
VOID
CcSetDirtyPinnedData (
    __in PVOID BcbVoid,
    __in_opt PLARGE_INTEGER Lsn
    );

NTKERNELAPI
VOID
CcUnpinData (
    __in PVOID Bcb
    );

NTKERNELAPI
VOID
CcSetBcbOwnerPointer (
    __in PVOID Bcb,
    __in PVOID OwnerPointer
    );

NTKERNELAPI
VOID
CcUnpinDataForThread (
    __in PVOID Bcb,
    __in ERESOURCE_THREAD ResourceThreadId
    );

// end_ntifs
//
// The following routines are in logsup.c, and provide special Cache Manager
// support for storing Lsns with dirty file pages, and performing subsequent
// operations based on them.
//

NTKERNELAPI
BOOLEAN
CcSetPrivateWriteFile(
    PFILE_OBJECT FileObject
    );

// begin_ntifs

NTKERNELAPI
VOID
CcSetAdditionalCacheAttributes (
    __in PFILE_OBJECT FileObject,
    __in BOOLEAN DisableReadAhead,
    __in BOOLEAN DisableWriteBehind
    );

NTKERNELAPI
VOID
CcSetLogHandleForFile (
    __in PFILE_OBJECT FileObject,
    __in PVOID LogHandle,
    __in PFLUSH_TO_LSN FlushToLsnRoutine
    );

NTKERNELAPI
LARGE_INTEGER
CcGetDirtyPages (
    __in PVOID LogHandle,
    __in PDIRTY_PAGE_ROUTINE DirtyPageRoutine,
    __in PVOID Context1,
    __in PVOID Context2
    );

NTKERNELAPI
BOOLEAN
CcIsThereDirtyData (
    __in PVPB Vpb
    );

// end_ntifs

NTKERNELAPI
LARGE_INTEGER
CcGetLsnForFileObject(
    __in PFILE_OBJECT FileObject,
    __out_opt PLARGE_INTEGER OldestLsn
    );

//
// Internal kernel interfaces for the prefetcher.
//

extern LONG CcPfNumActiveTraces;
#define CCPF_IS_PREFETCHER_ACTIVE() (CcPfNumActiveTraces)

extern LOGICAL CcPfEnablePrefetcher;
#define CCPF_IS_PREFETCHER_ENABLED() (CcPfEnablePrefetcher)

extern LOGICAL CcPfPrefetchingForBoot;
#define CCPF_IS_PREFETCHING_FOR_BOOT() (CcPfPrefetchingForBoot)

NTSTATUS
CcPfInitializePrefetcher(
    VOID
    );

//
// Define boot phase id's for use with PrefetcherBootPhase information
// subclass.
//

typedef enum _PF_BOOT_PHASE_ID {
    PfKernelInitPhase                            =   0,
    PfBootDriverInitPhase                        =  90,
    PfSystemDriverInitPhase                      = 120,
    PfSessionManagerInitPhase                    = 150,
    PfSMRegistryInitPhase                        = 180,
    PfVideoInitPhase                             = 210,
    PfPostVideoInitPhase                         = 240,
    PfBootAcceptedRegistryInitPhase              = 270,
    PfUserShellReadyPhase                        = 300,
    PfMaxBootPhaseId                             = 900,
} PF_BOOT_PHASE_ID, *PPF_BOOT_PHASE_ID;

NTSTATUS
CcPfBeginBootPhase(
    PF_BOOT_PHASE_ID Phase
    );

NTSTATUS
CcPfBeginAppLaunch(
    PEPROCESS Process,
    PVOID Section
    );

NTSTATUS
CcPfProcessExitNotification(
    PEPROCESS Process
    );

#define CCPF_TYPE_IMAGE             0x00000001  // Current fault is for an image
#define CCPF_TYPE_ROM               0x00000002  // Current fault is for a ROM

VOID
CcPfLogPageFault(
    IN PFILE_OBJECT FileObject,
    IN ULONGLONG FileOffset,
    IN ULONG Flags
    );

NTSTATUS
CcPfQueryPrefetcherInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PULONG Length
    );

NTSTATUS
CcPfSetPrefetcherInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN KPROCESSOR_MODE PreviousMode
    );

//
// Internal kernel interfaces for Perf FileName rundowns.
//

VOID
CcPerfFileRunDown (
    IN PPERFINFO_ENTRY_TABLE HashTable
    );

#endif  // CACHE
