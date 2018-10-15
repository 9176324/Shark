/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    mminit.c

Abstract:

    This module contains the initialization for the memory management
    system.

--*/

#include "mi.h"

PVOID ExPageLockHandle;

PMMPTE MmSharedUserDataPte;

extern PMMPTE MiSpecialPoolFirstPte;
extern ULONG MmPagedPoolCommit;
extern ULONG MmInPageSupportMinimum;
extern PFN_NUMBER MiExpansionPoolPagesInitialCharge;

extern PVOID BBTBuffer;
extern PFN_COUNT BBTPagesToReserve;

//
// Registry-settable.
//

ULONG MmAllocationPreference;

ULONG_PTR MmSubsectionBase;
ULONG_PTR MmSubsectionTopPage;
ULONG MmDataClusterSize;
ULONG MmCodeClusterSize;
PFN_NUMBER MmResidentAvailableAtInit;
PPHYSICAL_MEMORY_DESCRIPTOR MmPhysicalMemoryBlock;
LIST_ENTRY MmProtectedPteList;
KSPIN_LOCK MmProtectedPteLock;

ULONG MmKeepBootRegistry = 0;

KGUARDED_MUTEX MmPagedPoolMutex;

LOGICAL MmPagedPoolMaximumDesired = FALSE;

ULONG MiFullyInitialized;

LOGICAL MiSafeBooted = FALSE;

PFN_NUMBER MmFreedExpansionPoolMaximum;

RTL_BITMAP MiPfnBitMap;

VOID
MiMapBBTMemory (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
MiEnablePagingTheExecutive (
    VOID
    );

VOID
MiEnablePagingOfDriverAtInit (
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte
    );

VOID
MiBuildPagedPool (
    );

VOID
MiWriteProtectSystemImage (
    IN PVOID DllBase
    );

VOID
MiInitializePfnTracing (
    VOID
    );

PFN_NUMBER
MiPagesInLoaderBlock (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PBOOLEAN IncludeType
    );

VOID
MiMemorySan (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
MiInitializeCacheOverrides (
    VOID
    );

//
// The thresholds can be overridden by the registry.
//

PFN_NUMBER MmLowMemoryThreshold;
PFN_NUMBER MmHighMemoryThreshold;

//
// A temporary unsignaled event for the available pages & pool events to
// point at early during phase 0.  This is because the exportable events
// with DACLs cannot be created until phase 1.
//

KEVENT  MiTempEvent;

PKEVENT MiLowMemoryEvent;
PKEVENT MiHighMemoryEvent;

PKEVENT MiLowPagedPoolEvent;
PKEVENT MiHighPagedPoolEvent;

PKEVENT MiLowNonPagedPoolEvent;
PKEVENT MiHighNonPagedPoolEvent;

PFN_NUMBER MiLowPagedPoolThreshold;
PFN_NUMBER MiHighPagedPoolThreshold;

PFN_NUMBER MiLowNonPagedPoolThreshold;
PFN_NUMBER MiHighNonPagedPoolThreshold;

NTSTATUS
MiCreateMemoryEvent (
    IN PUNICODE_STRING EventName,
    OUT PKEVENT *Event
    );

LOGICAL
MiInitializeMemoryEvents (
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,MmInitSystem)
#pragma alloc_text(INIT,MiMapBBTMemory)
#pragma alloc_text(INIT,MmInitializeMemoryLimits)
#pragma alloc_text(INIT,MmFreeLoaderBlock)
#pragma alloc_text(INIT,MiBuildPagedPool)
#pragma alloc_text(INIT,MiFindInitializationCode)
#pragma alloc_text(INIT,MiEnablePagingTheExecutive)
#pragma alloc_text(INIT,MiEnablePagingOfDriverAtInit)
#pragma alloc_text(INIT,MiPagesInLoaderBlock)
#pragma alloc_text(INIT,MiCreateMemoryEvent)
#pragma alloc_text(INIT,MiInitializeMemoryEvents)
#pragma alloc_text(INIT,MiInitializeCacheOverrides)
#pragma alloc_text(INIT,MiInitializeNonPagedPoolThresholds)
#if defined(_X86_) || defined(_AMD64_)
#pragma alloc_text(INIT,MiAddHalIoMappings)
#endif
#pragma alloc_text(PAGELK,MiFreeInitializationCode)
#endif

//
// Default is a 300 second life span for modified mapped pages -
// This can be overridden in the registry.
//

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INITDATA")
#endif
ULONG MmModifiedPageLifeInSeconds = 300;
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

LARGE_INTEGER MiModifiedPageLife;

BOOLEAN MiTimerPending = FALSE;

KEVENT MiMappedPagesTooOldEvent;

KDPC MiModifiedPageWriterTimerDpc;

KTIMER MiModifiedPageWriterTimer;

//
// The following constants are based on the number PAGES not the
// memory size.  For convenience the number of pages is calculated
// based on a 4k page size.  Hence 12mb with 4k page is 3072.
//

#define MM_SMALL_SYSTEM ((13*1024*1024) / 4096)

#define MM_MEDIUM_SYSTEM ((19*1024*1024) / 4096)

#define MM_MIN_INITIAL_PAGED_POOL ((32*1024*1024) >> PAGE_SHIFT)

#define MM_DEFAULT_IO_LOCK_LIMIT (2 * 1024 * 1024)

extern WSLE_NUMBER MmMaximumWorkingSetSize;

extern ULONG MmEnforceWriteProtection;

extern LONG MiTrimInProgressCount;

#if (_MI_PAGING_LEVELS < 3)
PFN_NUMBER MmSystemPageDirectory[PD_PER_SYSTEM];
PMMPTE MmSystemPagePtes;
#endif

ULONG MmTotalSystemCodePages;

MM_SYSTEMSIZE MmSystemSize;

ULONG MmLargeSystemCache;

ULONG MmProductType;

extern ULONG MiVerifyAllDrivers;

LIST_ENTRY MmLoadedUserImageList;
PPAGE_FAULT_NOTIFY_ROUTINE MmPageFaultNotifyRoutine;

#if defined (_WIN64)
#define MM_ALLOCATION_FRAGMENT (64 * 1024 * 1024)
#define MM_ALLOCATION_FRAGMENT_MAX MM_ALLOCATION_FRAGMENT
#else
#define MM_ALLOCATION_FRAGMENT           (64 * 1024)
#define MM_ALLOCATION_FRAGMENT_MAX (2 * 1024 * 1024)
#endif

//
// Registry-settable.
//

SIZE_T MmAllocationFragment;


#if defined(MI_MULTINODE)

HALNUMAPAGETONODE
MiNonNumaPageToNodeColor (
    IN PFN_NUMBER PageFrameIndex
    )

/*++

Routine Description:

    Return the node color of the page.

Arguments:

    PageFrameIndex - Supplies the physical page number.

Return Value:

    Node color is always zero in non-NUMA configurations.

--*/

{
    UNREFERENCED_PARAMETER (PageFrameIndex);

    return 0;
}

//
// This node determination function pointer is initialized to return 0.
//
// Architecture-dependent initialization may re-point it to a HAL routine
// for NUMA configurations.
//

PHALNUMAPAGETONODE MmPageToNode = MiNonNumaPageToNodeColor;


VOID
MiDetermineNode (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPFN Pfn
    )

/*++

Routine Description:

    This routine is called during initial freelist population or when
    physical memory is being hot-added.  It then determines which node
    (in a multinode NUMA system) the physical memory resides in, and
    marks the PFN entry accordingly.

    N.B.  The actual page to node determination is machine dependent
    and done by a routine in the chipset driver or the HAL, called
    via the MmPageToNode function pointer.

Arguments:

    PageFrameIndex - Supplies the physical page number.

    Pfn - Supplies a pointer to the PFN database element.

Return Value:

    None.

Environment:

    None although typically this routine is called with the PFN
    database locked.

--*/

{
    ULONG Temp;

    ASSERT (Pfn == MI_PFN_ELEMENT(PageFrameIndex));

    Temp = MmPageToNode (PageFrameIndex);

    ASSERT (Temp < MAXIMUM_CCNUMA_NODES);

    Pfn->u3.e1.PageColor = (USHORT) Temp;
}

#endif


BOOLEAN
MmInitSystem (
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

/*++

Routine Description:

    This function is called during Phase 0, phase 1 and at the end
    of phase 1 ("phase 2") initialization.

    Phase 0 initializes the memory management paging functions,
    nonpaged and paged pool, the PFN database, etc.

    Phase 1 initializes the section objects, the physical memory
    object, and starts the memory management system threads.

    Phase 2 frees memory used by the OsLoader.

Arguments:

    Phase - System initialization phase.

    LoaderBlock - Supplies a pointer to the system loader block.

Return Value:

    Returns TRUE if the initialization was successful.

Environment:

    Kernel Mode Only.  System initialization.

--*/

{
    ULONG Color;
    PSLIST_ENTRY SingleListEntry;
    PFN_NUMBER NumberOfPages;
    HANDLE ThreadHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE StartPde;
    PMMPTE EndPde;
    PMMPFN Pfn1;
    PFN_NUMBER i, j;
    PFN_NUMBER DeferredMdlEntries;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER DirectoryFrameIndex;
    MMPTE TempPte;
    KIRQL OldIrql;
    PLIST_ENTRY NextEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    ULONG_PTR MaximumSystemCacheSize;
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG_PTR SystemPteMultiplier;
    ULONG_PTR DefaultSystemViewSize;
    ULONG_PTR SessionEnd;
    SIZE_T SystemViewMax;
    SIZE_T HydraImageMax;
    SIZE_T HydraViewMax;
    SIZE_T HydraPoolMax;
    SIZE_T HydraSpaceUsedForSystemViews;
    BOOLEAN IncludeType[LoaderMaximum];
    LOGICAL AutosizingFragment;
    PULONG Bitmap;
    PPHYSICAL_MEMORY_RUN Run;
    ULONG VerifierFlags;
#if DBG
    MMPTE Pointer;
#endif
#if (_MI_PAGING_LEVELS >= 3)
    LOGICAL FirstPpe;
    PMMPTE StartPpe;
#endif
#if (_MI_PAGING_LEVELS >= 4)
    LOGICAL FirstPxe;
    PMMPTE StartPxe;
#endif
#if defined(_X86_)
    PCHAR ReducedUserVaOption;
    ULONG UserVaLimit;
    ULONG ReductionInBytes;
    ULONG ReductionInPages;
    LOGICAL ReduceSystemCacheForPtes;
#endif

    j = 0;
    PointerPde = NULL;

    //
    // Make sure structure alignment is okay.
    //

    if (Phase == 0) {
        MmThrottleTop = 450;
        MmThrottleBottom = 127;

        //
        // Point various event thresholds at a temp unsignaled event
        // since real event creation is not available until phase 1.
        //

        KeInitializeEvent (&MiTempEvent, NotificationEvent, FALSE);

        MiLowMemoryEvent = &MiTempEvent;
        MiHighMemoryEvent = &MiTempEvent;

        MiLowPagedPoolEvent = &MiTempEvent;
        MiHighPagedPoolEvent = &MiTempEvent;

        MiLowNonPagedPoolEvent = &MiTempEvent;
        MiHighNonPagedPoolEvent = &MiTempEvent;

        //
        // Set the highest user address, the system range start address, the
        // user probe address, and the virtual bias.
        //

#if defined(_WIN64)

        MmHighestUserAddress = MI_HIGHEST_USER_ADDRESS;
        MmUserProbeAddress = MI_USER_PROBE_ADDRESS;
        MmSystemRangeStart = MI_SYSTEM_RANGE_START;

#else

        MmHighestUserAddress = (PVOID)(KSEG0_BASE - 0x10000 - 1);
        MmUserProbeAddress = KSEG0_BASE - 0x10000;
        MmSystemRangeStart = (PVOID)KSEG0_BASE;

#endif

        MiHighestUserPte = MiGetPteAddress (MmHighestUserAddress);
        MiHighestUserPde = MiGetPdeAddress (MmHighestUserAddress);

#if (_MI_PAGING_LEVELS >= 4)
        MiHighestUserPpe = MiGetPpeAddress (MmHighestUserAddress);
        MiHighestUserPxe = MiGetPxeAddress (MmHighestUserAddress);
#endif

#if defined(_X86_) || defined(_AMD64_)

        MmBootImageSize = LoaderBlock->Extension->LoaderPagesSpanned;
        MmBootImageSize *= PAGE_SIZE;

        MmBootImageSize = MI_ROUND_TO_SIZE (MmBootImageSize,
                                            MM_VA_MAPPED_BY_PDE);

        ASSERT ((MmBootImageSize % MM_VA_MAPPED_BY_PDE) == 0);
#endif

#if defined(_X86_)
        MmVirtualBias = LoaderBlock->u.I386.VirtualBias;
#endif

        //
        // Initialize system and Hydra mapped view sizes.
        //

        DefaultSystemViewSize = MM_SYSTEM_VIEW_SIZE;
        MmSessionSize = MI_SESSION_SPACE_DEFAULT_TOTAL_SIZE;
        SessionEnd = (ULONG_PTR) MM_SESSION_SPACE_DEFAULT_END;

#define MM_MB_MAPPED_BY_PDE (MM_VA_MAPPED_BY_PDE / (1024*1024))

        //
        // A PDE of virtual space is the minimum system view size allowed.
        //

        if (MmSystemViewSize < (MM_VA_MAPPED_BY_PDE / (1024*1024))) {
            MmSystemViewSize = DefaultSystemViewSize;
        }
        else {

            //
            // The view size has been specified (in megabytes) by the registry.
            // Validate it.
            //

            if (MmVirtualBias == 0) {

                //
                // Round the system view size (in megabytes) to a PDE multiple.
                //

                MmSystemViewSize = MI_ROUND_TO_SIZE (MmSystemViewSize,
                                                     MM_MB_MAPPED_BY_PDE);

                //
                // NT64 locates system views just after systemwide paged pool,
                // so the size of the system views are not limited by session
                // space.  Arbitrarily make the maximum a PPE's worth.
                //
                //
                // NT32 shares system view VA space with session VA space due
                // to the shortage of virtual addresses.  Thus increasing the
                // system view size means potentially decreasing the maximum
                // session space size.
                //

                SystemViewMax = (MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE) / (1024*1024);

#if !defined(_WIN64)

                //
                // Ensure at least enough space is left for
                // the standard default session layout.
                //

                SystemViewMax -= (MmSessionSize / (1024*1024));
#endif

                //
                // Note a view size of -1 will be rounded to zero.  Treat -1
                // as requesting the maximum.
                //

                if ((MmSystemViewSize > SystemViewMax) ||
                    (MmSystemViewSize == 0)) {

                    MmSystemViewSize = SystemViewMax;
                }

                MmSystemViewSize *= (1024*1024);
            }
            else {
                MmSystemViewSize = DefaultSystemViewSize;
            }
        }

#if defined(_WIN64)
        HydraSpaceUsedForSystemViews = 0;
#else
        HydraSpaceUsedForSystemViews = MmSystemViewSize;
#endif
        MiSessionImageEnd = SessionEnd;

        //
        // Select reasonable Hydra image, pool and view virtual sizes.
        // A PDE of virtual space is the minimum size allowed for each type.
        //

        if (MmVirtualBias == 0) {

            if (MmSessionImageSize < MM_MB_MAPPED_BY_PDE) {
                MmSessionImageSize = MI_SESSION_DEFAULT_IMAGE_SIZE;
            }
            else {

                //
                // The Hydra image size has been specified (in megabytes)
                // by the registry.
                //
                // Round it to a PDE multiple and validate it.
                //

                MmSessionImageSize = MI_ROUND_TO_SIZE (MmSessionImageSize,
                                                        MM_MB_MAPPED_BY_PDE);

                HydraImageMax = (MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE - HydraSpaceUsedForSystemViews - (MmSessionSize - MI_SESSION_DEFAULT_IMAGE_SIZE)) / (1024*1024);

                //
                // Note a view size of -1 will be rounded to zero.
                // Treat -1 as requesting the maximum.
                //

                if ((MmSessionImageSize > HydraImageMax) ||
                    (MmSessionImageSize == 0)) {
                    MmSessionImageSize = HydraImageMax;
                }

                MmSessionImageSize *= (1024*1024);
                MmSessionSize -= MI_SESSION_DEFAULT_IMAGE_SIZE;
                MmSessionSize += MmSessionImageSize;
            }

            MiSessionImageStart = SessionEnd - MmSessionImageSize;

            //
            // The session image start and size has been established.
            //
            // Now initialize the session pool and view ranges which lie
            // virtually below it.
            //

            if (MmSessionViewSize < MM_MB_MAPPED_BY_PDE) {
                MmSessionViewSize = MI_SESSION_DEFAULT_VIEW_SIZE;
            }
            else {

                //
                // The Hydra view size has been specified (in megabytes)
                // by the registry.  Validate it.
                //
                // Round the Hydra view size to a PDE multiple.
                //

                MmSessionViewSize = MI_ROUND_TO_SIZE (MmSessionViewSize,
                                                      MM_MB_MAPPED_BY_PDE);

                HydraViewMax = (MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE - HydraSpaceUsedForSystemViews - (MmSessionSize - MI_SESSION_DEFAULT_VIEW_SIZE)) / (1024*1024);

                //
                // Note a view size of -1 will be rounded to zero.
                // Treat -1 as requesting the maximum.
                //

                if ((MmSessionViewSize > HydraViewMax) ||
                    (MmSessionViewSize == 0)) {
                    MmSessionViewSize = HydraViewMax;
                }

                MmSessionViewSize *= (1024*1024);
                MmSessionSize -= MI_SESSION_DEFAULT_VIEW_SIZE;
                MmSessionSize += MmSessionViewSize;
            }

            MiSessionViewStart = SessionEnd - MmSessionImageSize - MI_SESSION_SPACE_WS_SIZE - MI_SESSION_SPACE_STRUCT_SIZE - MmSessionViewSize;

            //
            // The session view start and size has been established.
            //
            // Now initialize the session pool start and size which lies
            // virtually just below it.
            //

            MiSessionPoolEnd = MiSessionViewStart;

            if (MmSessionPoolSize < MM_MB_MAPPED_BY_PDE) {

#if !defined(_WIN64)

                //
                // Professional and below use systemwide paged pool for session
                // allocations (this decision is made in win32k.sys).  Server
                // and above use real session pool and 16mb isn't enough to
                // play high end game applications, etc.  Since we're not
                // booted /3GB, try for an additional 16mb now.
                //

                if ((MmSessionPoolSize == 0) && (MmProductType != 0x00690057)) {

                    HydraPoolMax = MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE - HydraSpaceUsedForSystemViews - MmSessionSize;
                    if (HydraPoolMax >= 2 * MI_SESSION_DEFAULT_POOL_SIZE) {
                        MmSessionPoolSize = 2 * MI_SESSION_DEFAULT_POOL_SIZE;
                        MmSessionSize -= MI_SESSION_DEFAULT_POOL_SIZE;
                        MmSessionSize += MmSessionPoolSize;
                    }
                    else {
                        MmSessionPoolSize = MI_SESSION_DEFAULT_POOL_SIZE;
                    }
                }
                else
#endif
                MmSessionPoolSize = MI_SESSION_DEFAULT_POOL_SIZE;
            }
            else {

                //
                // The Hydra pool size has been specified (in megabytes)
                // by the registry.  Validate it.
                //
                // Round the Hydra pool size to a PDE multiple.
                //

                MmSessionPoolSize = MI_ROUND_TO_SIZE (MmSessionPoolSize,
                                                      MM_MB_MAPPED_BY_PDE);

                HydraPoolMax = (MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE - HydraSpaceUsedForSystemViews - (MmSessionSize - MI_SESSION_DEFAULT_POOL_SIZE)) / (1024*1024);

                //
                // Note a view size of -1 will be rounded to zero.
                // Treat -1 as requesting the maximum.
                //

                if ((MmSessionPoolSize > HydraPoolMax) ||
                    (MmSessionPoolSize == 0)) {
                    MmSessionPoolSize = HydraPoolMax;
                }

                MmSessionPoolSize *= (1024*1024);
                MmSessionSize -= MI_SESSION_DEFAULT_POOL_SIZE;
                MmSessionSize += MmSessionPoolSize;
            }

            MiSessionPoolStart = MiSessionPoolEnd - MmSessionPoolSize;

            MmSessionBase = (ULONG_PTR) MiSessionPoolStart;

#if defined (_WIN64)

            //
            // Session special pool immediately follows session regular pool
            // assuming the user has enabled either the verifier or special
            // pool.
            //

            if ((MmVerifyDriverBufferLength != (ULONG)-1) ||
                ((MmSpecialPoolTag != 0) && (MmSpecialPoolTag != (ULONG)-1))) {

                MmSessionSize = MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE;
                MmSessionSpecialPoolEnd = (PVOID) MiSessionPoolStart;
                MmSessionBase = MM_SESSION_SPACE_DEFAULT;
                MmSessionSpecialPoolStart = (PVOID) MmSessionBase;
            }
#endif

            ASSERT (MmSessionBase + MmSessionSize == SessionEnd);
            MiSessionSpaceEnd = SessionEnd;
            MiSessionSpacePageTables = (ULONG)(MmSessionSize / MM_VA_MAPPED_BY_PDE);
#if !defined (_WIN64)
            MiSystemViewStart = MmSessionBase - MmSystemViewSize;
#endif

        }
        else {

            //
            // When booted /3GB, no size overrides are allowed due to the
            // already severely limited virtual address space.
            // Initialize the other Hydra variables after the system cache.
            //

            MmSessionViewSize = MI_SESSION_DEFAULT_VIEW_SIZE;
            MmSessionPoolSize = MI_SESSION_DEFAULT_POOL_SIZE;
            MmSessionImageSize = MI_SESSION_DEFAULT_IMAGE_SIZE;

            MiSessionImageStart = MiSessionImageEnd - MmSessionImageSize;

#if defined(_X86PAE_)
            MmSystemCacheStart = (PVOID) ((ULONG_PTR) MmSystemCacheStart +
                                        MM_SYSTEM_CACHE_WORKING_SET_3GB_DELTA);
#endif
        }

        //
        // Set the highest section base address.
        //
        // N.B. In 32-bit systems this address must be 2gb or less even for
        //      systems that run with 3gb enabled. Otherwise, it would not
        //      be possible to map based sections identically in all processes.
        //

        MmHighSectionBase = ((PCHAR)MmHighestUserAddress - 0x800000);

        MaximumSystemCacheSize = (MM_SYSTEM_CACHE_END - (ULONG_PTR) MmSystemCacheStart) >> PAGE_SHIFT;

#if defined(_X86_)

        if (MmSizeOfPagedPoolInBytes == (SIZE_T)-1) {

            //
            // The registry indicates that paged pool should be enlarged to
            // the maximum possible.  Steal from the system cache if possible
            // to accomplish this.
            //

            if (MmVirtualBias == 0) {
                ReductionInPages = MaximumSystemCacheSize / 3;
                ReductionInPages = MI_ROUND_TO_SIZE (ReductionInPages, PTE_PER_PAGE);
                MaximumSystemCacheSize -= ReductionInPages;
                MmPagedPoolStart = (PVOID) ((PCHAR)MmPagedPoolStart - (ReductionInPages << PAGE_SHIFT));
            }
        }

        //
        // If boot.ini specified a sane number of MB that the administrator
        // wants to use for user virtual address space then use it.
        //

        UserVaLimit = 0;
        ReducedUserVaOption = strstr(LoaderBlock->LoadOptions, "USERVA");

        if (ReducedUserVaOption != NULL) {

            ReducedUserVaOption = strstr(ReducedUserVaOption,"=");

            if (ReducedUserVaOption != NULL) {

                UserVaLimit = atol(ReducedUserVaOption+1);

                UserVaLimit = MI_ROUND_TO_SIZE (UserVaLimit, ((MM_VA_MAPPED_BY_PDE) / (1024*1024)));
            }
        }

        ReduceSystemCacheForPtes = FALSE;

        if (MmVirtualBias != 0) {

            if (MmNumberOfSystemPtes == (ULONG)-1) {
    
                //
                // This registry setting indicates the maximum number of
                // system PTEs possible for this machine must be allocated.
                // Since we are booted /3GB, take some system cache VA space
                // for this.
                //
    
                MiRequestedSystemPtes = (ULONG) MmNumberOfSystemPtes;
    
                MmNumberOfSystemPtes = MM_MAXIMUM_SYSTEM_PTES;
    
                ReduceSystemCacheForPtes = TRUE;
            }
    
            //
            // If the size of the boot image (likely due to a large registry)
            // overflows into where paged pool would normally start, then
            // move paged pool up now.  This costs virtual address space (ie:
            // performance) but more importantly, allows the system to boot.
            //

            if (MmBootImageSize > 16 * 1024 * 1024) {
                MmPagedPoolStart = (PVOID)((PCHAR)MmPagedPoolStart + (MmBootImageSize - 16 * 1024 * 1024));
                ASSERT (((ULONG_PTR)MmPagedPoolStart % MM_VA_MAPPED_BY_PDE) == 0);
            }

            //
            // The system has been biased to an alternate base address to
            // allow 3gb of user address space, set the user probe address
            // and the maximum system cache size.
            //

            if ((UserVaLimit > 2048) && (UserVaLimit < 3072)) {

                //
                // Use any space between the maximum user virtual address
                // and the system for extra system PTEs.
                //
                // Convert input MB to bytes.
                //

                UserVaLimit -= 2048;
                UserVaLimit *= (1024*1024);

                //
                // Don't let the user specify a value which would cause us to
                // prematurely overwrite portions of the kernel & loader block.
                //

                if (UserVaLimit < MmBootImageSize) {
                    UserVaLimit = MmBootImageSize;
                }
            }
            else {
                UserVaLimit = 0x40000000;
            }

            MmHighestUserAddress = ((PCHAR)MmHighestUserAddress + UserVaLimit);
            MmSystemRangeStart = ((PCHAR)MmSystemRangeStart + UserVaLimit);
            MmUserProbeAddress += UserVaLimit;
            MiMaximumWorkingSet += UserVaLimit >> PAGE_SHIFT;

            if (UserVaLimit != 0x40000000) {
                MiUseMaximumSystemSpace = (ULONG_PTR)MmSystemRangeStart;
                MiUseMaximumSystemSpaceEnd = 0xC0000000;
            }

            MiHighestUserPte = MiGetPteAddress (MmHighestUserAddress);
            MiHighestUserPde = MiGetPdeAddress (MmHighestUserAddress);

            //
            // Moving to 3GB means moving session space to just above
            // the system cache (and lowering the system cache max size
            // accordingly).  Here's the visual:
            //
            //                 +------------------------------------+
            //        C1000000 | System cache resides here          |
            //                 | and grows upward.                  |
            //                 |               |                    |
            //                 |               |                    |
            //                 |              \/                    |
            //                 |                                    |
            //                 +------------------------------------+
            //                 | Session space (Hydra).             |
            //                 +------------------------------------+
            //                 | Systemwide global mapped views.    |
            //                 +------------------------------------+
            //                 |                                    |
            //                 |               ^                    |
            //                 |               |                    |
            //                 |               |                    |
            //                 |                                    |
            //                 | Kernel, HAL & boot loaded images   |
            //                 | grow downward from E1000000.       |
            //                 | Total size is specified by         |
            //                 | LoaderBlock->u.I386.BootImageSize. |
            //                 | Note only ntldrs after Build 2195  |
            //                 | are capable of loading the boot    |
            //                 | images in descending order from    |
            //                 | a hardcoded E1000000 on down.      |
            //        E1000000 +------------------------------------+
            //

            MaximumSystemCacheSize -= MmBootImageSize >> PAGE_SHIFT;

            MaximumSystemCacheSize -= MmSessionSize >> PAGE_SHIFT;

            MaximumSystemCacheSize -= MmSystemViewSize >> PAGE_SHIFT;

            MmSessionBase = (ULONG_PTR)((ULONG_PTR)MmSystemCacheStart +
                                  (MaximumSystemCacheSize << PAGE_SHIFT));

            MiSystemViewStart = MmSessionBase + MmSessionSize;

            MiSessionPoolStart = MmSessionBase;
            MiSessionPoolEnd = MiSessionPoolStart + MmSessionPoolSize;
            MiSessionViewStart = MiSessionPoolEnd;

            MiSessionSpaceEnd = (ULONG_PTR)MmSessionBase + MmSessionSize;
            MiSessionSpacePageTables = MmSessionSize / MM_VA_MAPPED_BY_PDE;

            MiSessionImageEnd = MiSessionSpaceEnd;
            MiSessionImageStart = MiSessionImageEnd - MmSessionImageSize;
        }
        else if ((UserVaLimit >= 64) && (UserVaLimit < 2048)) {

            //
            // Convert input MB to bytes.
            //
            // Note there is no protection against users that try to use
            // the USERVA option to reduce user space below 2GB.
            //

            UserVaLimit *= (1024*1024);
            ReductionInBytes = 0x80000000 - UserVaLimit;

            MmHighestUserAddress = ((PCHAR)MmHighestUserAddress - ReductionInBytes);
            MmSystemRangeStart = ((PCHAR)MmSystemRangeStart - ReductionInBytes);
            MmUserProbeAddress -= ReductionInBytes;
            MiMaximumWorkingSet -= ReductionInBytes >> PAGE_SHIFT;

            MiUseMaximumSystemSpace = (ULONG_PTR)MmSystemRangeStart;
            MiUseMaximumSystemSpaceEnd = (ULONG_PTR)MiUseMaximumSystemSpace + ReductionInBytes;

            MmHighSectionBase = (PVOID)((PCHAR)MmHighSectionBase - ReductionInBytes);

            MiHighestUserPte = MiGetPteAddress (MmHighestUserAddress);
            MiHighestUserPde = MiGetPdeAddress (MmHighestUserAddress);
        }

#else

#if !defined (_WIN64)
        MaximumSystemCacheSize -= (MmSystemViewSize >> PAGE_SHIFT);
#endif

#endif

        //
        // Initialize some global session variables.
        //

        MmSessionSpace = (PMM_SESSION_SPACE)((ULONG_PTR)MmSessionBase + MmSessionSize - MmSessionImageSize - MI_SESSION_SPACE_STRUCT_SIZE);

        MiSessionImagePteStart = MiGetPteAddress ((PVOID) MiSessionImageStart);
        MiSessionImagePteEnd = MiGetPteAddress ((PVOID) MiSessionImageEnd);

        MiSessionBasePte = MiGetPteAddress ((PVOID)MmSessionBase);

        MiSessionSpaceWs = MiSessionViewStart + MmSessionViewSize;

        MiSessionLastPte = MiGetPteAddress ((PVOID)MiSessionSpaceEnd);

#if DBG
        //
        // A few sanity checks to ensure things are as they should be.
        //

        ASSERT ((sizeof (CONTROL_AREA) % 8) == 0);
        ASSERT ((sizeof (SUBSECTION) % 8) == 0);

        //
        // Some checks to make sure prototype PTEs can be placed in
        // either paged or nonpaged (prototype PTEs for paged pool are here)
        // can be put into PTE format.
        //

        PointerPte = (PMMPTE) MmPagedPoolStart;
        Pointer.u.Long = MiProtoAddressForPte (PointerPte);
        TempPte = Pointer;
        PointerPde = MiPteToProto (&TempPte);

        ASSERT (PointerPte == PointerPde);

        PointerPte = (PMMPTE)((ULONG_PTR)MM_NONPAGED_POOL_END & ~((1 << PTE_SHIFT) - 1));

        Pointer.u.Long = MiProtoAddressForPte (PointerPte);
        TempPte = Pointer;
        PointerPde = MiPteToProto (&TempPte);
        ASSERT (PointerPte == PointerPde);

        PointerPte = (PMMPTE)(((ULONG_PTR)NON_PAGED_SYSTEM_END -
                        0x37000 + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));

        for (j = 0; j < 20; j += 1) {
            Pointer.u.Long = MiProtoAddressForPte (PointerPte);
            TempPte = Pointer;
            PointerPde = MiPteToProto(&TempPte);
            ASSERT (PointerPte == PointerPde);
            PointerPte += 1;
        }

        PointerPte = (PMMPTE)(((ULONG_PTR)MM_NONPAGED_POOL_END - 0x133448) & ~(ULONG_PTR)7);
        Pointer.u.Long = MiGetSubsectionAddressForPte (PointerPte);
        TempPte = Pointer;
        PointerPde = (PMMPTE)MiGetSubsectionAddress(&TempPte);
        ASSERT (PointerPte == PointerPde);

        //
        // End of sanity checks.
        //

#endif

        for (i = 0; i < MI_PFN_PRIORITIES; i += 1) {
            MmStandbyPageListByPriority[i].Total = 0;
            MmStandbyPageListByPriority[i].ListName = StandbyPageList;
            MmStandbyPageListByPriority[i].Flink = MM_EMPTY_LIST;
            MmStandbyPageListByPriority[i].Blink = MM_EMPTY_LIST;
        }

        InitializeListHead (&MmLoadedUserImageList);
        InitializeListHead (&MmProtectedPteList);
        InitializeListHead (&MiHotPatchList);

        KeInitializeSpinLock (&MmProtectedPteLock);

        MmCriticalSectionTimeout.QuadPart = Int32x32To64(
                                                 MmCritsectTimeoutSeconds,
                                                -10000000);

        //
        // Initialize System Address Space creation mutex.
        //

        KeInitializeGuardedMutex (&MmSectionCommitMutex);
        KeInitializeGuardedMutex (&MmSectionBasedMutex);

        MI_INITIALIZE_DYNAMIC_MEMORY_LOCK ();

        KeInitializeGuardedMutex (&MmPagedPoolMutex);

        KeInitializeMutant (&MmSystemLoadLock, FALSE);

        KeInitializeEvent (&MmAvailablePagesEvent, NotificationEvent, TRUE);
        KeInitializeEvent (&MmAvailablePagesEventHigh, NotificationEvent, TRUE);
        KeInitializeEvent (&MmMappedFileIoComplete, NotificationEvent, FALSE);

        KeInitializeEvent (&MmZeroingPageEvent, SynchronizationEvent, FALSE);
        KeInitializeEvent (&MmCollidedFlushEvent, NotificationEvent, FALSE);
        KeInitializeEvent (&MmCollidedLockEvent, NotificationEvent, FALSE);
        KeInitializeEvent (&MiMappedPagesTooOldEvent, NotificationEvent, FALSE);

        KeInitializeDpc (&MiModifiedPageWriterTimerDpc,
                         MiModifiedPageWriterTimerDispatch,
                         NULL);

        KeInitializeTimerEx (&MiModifiedPageWriterTimer, SynchronizationTimer);

        MiModifiedPageLife.QuadPart = Int32x32To64(
                                                 MmModifiedPageLifeInSeconds,
                                                -10000000);

        InitializeListHead (&MmWorkingSetExpansionHead.ListHead);

        InitializeSListHead (&MmDeadStackSListHead);

        InitializeSListHead (&MmEventCountSListHead);

        InitializeSListHead (&MmInPageSupportSListHead);

        MmZeroingPageThreadActive = FALSE;

        MiMemorySan (LoaderBlock);

        //
        // include all memory types ...
        //

        for (i = 0; i < LoaderMaximum; i += 1) {
            IncludeType[i] = TRUE;
        }

        //
        // ... except these...
        // If you change this list, please make
        // the corresponding changes over in
        // boot\lib\blmemory.c!BlDetermineOSVisibleMemory()
        //

        IncludeType[LoaderBad] = FALSE;
        IncludeType[LoaderFirmwarePermanent] = FALSE;
        IncludeType[LoaderSpecialMemory] = FALSE;
        IncludeType[LoaderBBTMemory] = FALSE;

        //
        // Compute number of pages in the system.
        //

        NumberOfPages = MiPagesInLoaderBlock (LoaderBlock, IncludeType);

        //
        // When safebooting, don't enable special pool, the verifier or any
        // other options that track corruption regardless of registry settings.
        //

        if (strstr(LoaderBlock->LoadOptions, SAFEBOOT_LOAD_OPTION_A)) {
            MmLargePageDriverBufferLength = (ULONG)-1;
            MmVerifyDriverBufferLength = (ULONG)-1;
            MiVerifyAllDrivers = 0;
            MmVerifyDriverLevel = 0;
            MmSpecialPoolTag = (ULONG)-1;
            MmSnapUnloads = FALSE;
            MmProtectFreedNonPagedPool = FALSE;
            MmEnforceWriteProtection = 0;
            MmTrackLockedPages = FALSE;
            MiSafeBooted = TRUE;
            MmTrackPtes = 0;

#if defined (_WIN64)
            MmSessionSpecialPoolEnd = NULL;
            MmSessionSpecialPoolStart = NULL;
#endif

            SharedUserData->SafeBootMode = TRUE;
        }
        else {
            MiTriageSystem (LoaderBlock);
        }

        SystemPteMultiplier = 0;

        if (MmNumberOfSystemPtes == 0) {
#if defined (_WIN64)

            //
            // 64-bit NT is not constrained by virtual address space - but
            // there is physical cost for the preallocated page table pages
            // used by system PTEs.  No other tradeoffs between nonpaged pool,
            // paged pool and system PTEs need to be made.  So just
            // allocate PTEs on a linear scale as a function of the
            // amount of RAM.
            //
            // For example on a Hydra NT64, 4gb of RAM gets 128gb of PTEs.
            // The page table cost is the inversion of the multiplier based
            // on the PTE_PER_PAGE.
            //

            if (ExpMultiUserTS == TRUE) {
                SystemPteMultiplier = 32;
            }
            else if (NumberOfPages > 0x100000) {
                SystemPteMultiplier = 16;
            }
            else {
                SystemPteMultiplier = 4;
            }
            if (NumberOfPages < 0x8000) {
                SystemPteMultiplier >>= 1;
            }
#else
            if (NumberOfPages < MM_MEDIUM_SYSTEM) {
                MmNumberOfSystemPtes = MM_MINIMUM_SYSTEM_PTES;
            }
            else {
                MmNumberOfSystemPtes = MM_DEFAULT_SYSTEM_PTES;
                if (NumberOfPages > 8192) {
                    MmNumberOfSystemPtes += MmNumberOfSystemPtes;

                    //
                    // Any reasonable Hydra machine gets the maximum.
                    //

                    if (ExpMultiUserTS == TRUE) {
                        MmNumberOfSystemPtes = MM_MAXIMUM_SYSTEM_PTES;
                    }
                }
            }
#endif
        }
        else if (MmNumberOfSystemPtes == (ULONG)-1) {

            //
            // This registry setting indicates the maximum number of
            // system PTEs possible for this machine must be allocated.
            // Snap this for later reference.
            //

            MiRequestedSystemPtes = (ULONG) MmNumberOfSystemPtes;

#if defined (_WIN64)
            SystemPteMultiplier = 256;
#else
            MmNumberOfSystemPtes = MM_MAXIMUM_SYSTEM_PTES;
#endif
        }

        if (SystemPteMultiplier != 0) {
            if (NumberOfPages * SystemPteMultiplier > MM_MAXIMUM_SYSTEM_PTES) {
                MmNumberOfSystemPtes = MM_MAXIMUM_SYSTEM_PTES;
            }
            else {
                MmNumberOfSystemPtes = (ULONG)(NumberOfPages * SystemPteMultiplier);
            }
#if defined (_WIN64)
            if (MmNumberOfSystemPtes < _1gb / PAGE_SIZE) {
                MmNumberOfSystemPtes = _1gb / PAGE_SIZE;
            }
#endif
        }

        if (MmNumberOfSystemPtes > MM_MAXIMUM_SYSTEM_PTES)  {
            MmNumberOfSystemPtes = MM_MAXIMUM_SYSTEM_PTES;
        }

        if (MmNumberOfSystemPtes < MM_MINIMUM_SYSTEM_PTES) {
            MmNumberOfSystemPtes = MM_MINIMUM_SYSTEM_PTES;
        }

        if (MmHeapSegmentReserve == 0) {
            MmHeapSegmentReserve = 1024 * 1024;
        }

        if (MmHeapSegmentCommit == 0) {
            MmHeapSegmentCommit = PAGE_SIZE * 2;
        }

        if (MmHeapDeCommitTotalFreeThreshold == 0) {
            MmHeapDeCommitTotalFreeThreshold = 64 * 1024;
        }

        if (MmHeapDeCommitFreeBlockThreshold == 0) {
            MmHeapDeCommitFreeBlockThreshold = PAGE_SIZE;
        }

        //
        // If the registry indicates drivers are in the suspect list,
        // extra system PTEs need to be allocated to support special pool
        // for their allocations.
        //

        if ((MmVerifyDriverBufferLength != (ULONG)-1) ||
            ((MmSpecialPoolTag != 0) && (MmSpecialPoolTag != (ULONG)-1))) {
            MmNumberOfSystemPtes += MM_SPECIAL_POOL_PTES;
        }

        MmNumberOfSystemPtes += BBTPagesToReserve;

#if defined(_X86_)

        //
        // The allocation preference key must be carefully managed.  This is
        // because doing every allocation top-down can cause failures if
        // an ntdll process startup allocation (like the stack trace database)
        // gets a high address which then causes a subsequent system DLL rebase
        // collision.
        //
        // This is circumvented as follows:
        //
        // 1.  For 32-bit machines, the allocation preference key is only
        //     useful when booted /3GB as only then can this key help track
        //     down apps with high virtual address bit sign extension problems.
        //     In 3GB mode, the system DLLs are based just below 2GB so ntdll
        //     would have to allocate more than 1GB of VA space before this
        //     becomes a problem.  So really the problem can only occur for
        //     machines in 2GB mode and since the key doesn't help these
        //     machines anyway, just turn it off in these cases.
        //
        // 2.  For 64-bit machines, there is plenty of VA space above the
        //     addresses system DLLs are based at so it is a non-issue.
        //     EXCEPT for wow64 binaries which run in sandboxed 2GB address
        //     spaces.  Explicit checks are made to detect a wow64 process in
        //     the Mm APIs which check this key and the key is ignored in
        //     this case as it doesn't provide any sign extension help and
        //     therefore we don't allow it to burn up any valuable VA space
        //     which could cause a collision.
        //

        if (MmVirtualBias == 0) {
            MmAllocationPreference = 0;
        }
#endif

        ExInitializePushLock (&MmSystemCacheWs.WorkingSetMutex);

        MiInitializeDriverVerifierList ();

        //
        // Set the initial commit page limit high enough so initial pool
        // allocations (which happen in the machine dependent init) can
        // succeed.
        //

        MmTotalCommitLimit = _2gb / PAGE_SIZE;
        MmTotalCommitLimitMaximum = MmTotalCommitLimit;

        //
        // Pick a reasonable size for the default prototype PTE allocation
        // chunk size.  Make sure it's always a PAGE_SIZE multiple.  The
        // registry entry is treated as the number of 1K chunks.
        //

        if (MmAllocationFragment == 0) {
            AutosizingFragment = TRUE;
            MmAllocationFragment = MM_ALLOCATION_FRAGMENT;
#if !defined (_WIN64)
            if (NumberOfPages < 64 * 1024) {
                MmAllocationFragment = MM_ALLOCATION_FRAGMENT / 4;
            }
            else if (NumberOfPages < 256 * 1024) {
                MmAllocationFragment = MM_ALLOCATION_FRAGMENT / 2;
            }
#endif
        }
        else {

            //
            // Convert the registry entry from 1K chunks into bytes.
            // Then round it to a PAGE_SIZE multiple.  Finally bound it
            // reasonably.
            //

            AutosizingFragment = FALSE;
            MmAllocationFragment *= 1024;
            MmAllocationFragment = ROUND_TO_PAGES (MmAllocationFragment);

            if (MmAllocationFragment > MM_ALLOCATION_FRAGMENT_MAX) {
                MmAllocationFragment = MM_ALLOCATION_FRAGMENT_MAX;
            }
            else if (MmAllocationFragment < PAGE_SIZE) {
                MmAllocationFragment = PAGE_SIZE;
            }
        }

        MiInitializeIoTrackers ();

        MiInitializeCacheOverrides ();

#if defined(_WIN64)
        if (MmSizeOfPagedPoolInBytes == (SIZE_T)-1) {
            MmPagedPoolMaximumDesired = TRUE;
        }
#endif

        //
        // Initialize the machine dependent portion of the hardware.
        //

        MiInitMachineDependent (LoaderBlock);

        MmPhysicalMemoryBlock = MmInitializeMemoryLimits (LoaderBlock,
                                                          IncludeType,
                                                          NULL);

        if (MmPhysicalMemoryBlock == NULL) {
            KeBugCheckEx (INSTALL_MORE_MEMORY,
                          MmNumberOfPhysicalPages,
                          MmLowestPhysicalPage,
                          MmHighestPhysicalPage,
                          0x100);
        }

#if defined(_X86_) || defined(_AMD64_)
        MiReportPhysicalMemory ();
#endif

#if defined (_MI_MORE_THAN_4GB_)
        if (MiNoLowMemory != 0) {
            MiRemoveLowPages (0);
        }
#endif

        //
        // Initialize listhead, spinlock and semaphore for
        // segment dereferencing thread.
        //

        KeInitializeSpinLock (&MmDereferenceSegmentHeader.Lock);
        InitializeListHead (&MmDereferenceSegmentHeader.ListHead);
        KeInitializeSemaphore (&MmDereferenceSegmentHeader.Semaphore, 0, MAXLONG);

        InitializeListHead (&MmUnusedSegmentList);
        InitializeListHead (&MmUnusedSubsectionList);
        KeInitializeEvent (&MmUnusedSegmentCleanup, NotificationEvent, FALSE);

        MiInitializeCommitment ();

        MiInitializePfnTracing ();

#if defined(_X86_)

        //
        // Virtual bias indicates the offset that needs to be added to
        // 0x80000000 to get to the start of the loaded images.  Update it
        // now to indicate the offset to MmSessionBase as that is the lowest
        // system address that process creation needs to make sure to duplicate.
        //
        // This is not done until after machine dependent initialization runs
        // as that initialization relies on the original meaning of VirtualBias.
        //
        // Note if the system is booted with both /3GB & /USERVA, then system
        // PTEs will be allocated below virtual 3GB and that will end up being
        // the lowest system address the process creation needs to duplicate.
        //

        if (MmVirtualBias != 0) {
            MmVirtualBias = (ULONG_PTR)MmSessionBase - CODE_START;
        }
#endif

        //
        // Create the bitmap which represents valid memory.  Note the largest
        // possible size is used because (unlike the MmPhysicalMemoryBlock) we
        // don't want to free and reallocate this bitmap during hotadds because
        // we want callers to be able to reference it lock free.
        //

        ASSERT (MmHighestPossiblePhysicalPage + 1 < _4gb);

        Bitmap = ExAllocatePoolWithTag (
                       NonPagedPool,
                       (((MmHighestPossiblePhysicalPage + 1) + 31) / 32) * 4,
                       '  mM');

        if (Bitmap == NULL) {
            KeBugCheckEx (INSTALL_MORE_MEMORY,
                          MmNumberOfPhysicalPages,
                          MmLowestPhysicalPage,
                          MmHighestPhysicalPage,
                          0x101);
        }

        RtlInitializeBitMap (&MiPfnBitMap,
                             Bitmap,
                             (ULONG)(MmHighestPossiblePhysicalPage + 1));

        RtlClearAllBits (&MiPfnBitMap);

        for (i = 0; i < MmPhysicalMemoryBlock->NumberOfRuns; i += 1) {

            Run = &MmPhysicalMemoryBlock->Run[i];

            if (Run->PageCount != 0) {
                RtlSetBits (&MiPfnBitMap,
                            (ULONG)Run->BasePage,
                            (ULONG)Run->PageCount);
            }
        }

        MiSyncCachedRanges ();

#if defined(_X86_) || defined(_AMD64_)
        MiAddHalIoMappings ();
#endif

#if !defined (_WIN64)
        if ((AutosizingFragment == TRUE) &&
            (NumberOfPages >= 256 * 1024)) {

            //
            // This is a system with at least 1GB of RAM.  Presumably it
            // will be used to cache many files.  Maybe we should factor in
            // pool size here and adjust it accordingly.
            //

            MmAllocationFragment;
        }
#endif

        //
        // Temporarily initialize resident available pages so large page
        // allocations can succeed if the memory exists.
        //

        MmResidentAvailablePages = MmAvailablePages - MM_FLUID_PHYSICAL_PAGES;

        MiInitializeLargePageSupport ();

        MiInitializeDriverLargePageList ();

        //
        // Relocate all the drivers so they can be paged (and protected) on
        // a per-page basis.
        //

        MiReloadBootLoadedDrivers (LoaderBlock);

#if defined (_MI_MORE_THAN_4GB_)
        if (MiNoLowMemory != 0) {
            MiRemoveLowPages (1);
        }
#endif
        MiInitializeVerifyingComponents (LoaderBlock);

        //
        // Setup the system size as small, medium, or large depending
        // on memory available.
        //
        // For internal MM tuning, the following applies
        //
        // 12Mb  is small
        // 12-19 is medium
        // > 19 is large
        //
        //
        // For all other external tuning,
        // < 19 is small
        // 19 - 31 is medium for workstation
        // 19 - 63 is medium for server
        // >= 32 is large for workstation
        // >= 64 is large for server
        //

        if (MmNumberOfPhysicalPages <= MM_SMALL_SYSTEM) {
            MmSystemSize = MmSmallSystem;
            MmMaximumDeadKernelStacks = 0;
            MmModifiedPageMaximum = 100;
            MmDataClusterSize = 0;
            MmCodeClusterSize = 1;
            MmReadClusterSize = 2;
            MmInPageSupportMinimum = 2;
        }
        else if (MmNumberOfPhysicalPages <= MM_MEDIUM_SYSTEM) {
            MmSystemSize = MmSmallSystem;
            MmMaximumDeadKernelStacks = 2;
            MmModifiedPageMaximum = 150;
            MmSystemCacheWsMinimum += 100;
            MmDataClusterSize = 1;
            MmCodeClusterSize = 2;
            MmReadClusterSize = 4;
            MmInPageSupportMinimum = 3;
        }
        else {
            MmSystemSize = MmMediumSystem;
            MmMaximumDeadKernelStacks = 5;
            MmModifiedPageMaximum = 300;
            MmSystemCacheWsMinimum += 400;
            MmDataClusterSize = 3;
            MmCodeClusterSize = 7;
            MmReadClusterSize = 7;
            MmInPageSupportMinimum = 4;
        }

        if (MmNumberOfPhysicalPages < ((24*1024*1024)/PAGE_SIZE)) {
            MmSystemCacheWsMinimum = 32;
        }

        if (MmNumberOfPhysicalPages >= ((32*1024*1024)/PAGE_SIZE)) {

            //
            // If we are on a workstation, 32Mb and above are considered
            // large systems.
            //

            if (MmProductType == 0x00690057) {
                MmSystemSize = MmLargeSystem;
            }
            else {

                //
                // For servers, 64Mb and greater is a large system
                //

                if (MmNumberOfPhysicalPages >= ((64*1024*1024)/PAGE_SIZE)) {
                    MmSystemSize = MmLargeSystem;
                }
            }
        }

        if (MmNumberOfPhysicalPages > ((33*1024*1024)/PAGE_SIZE)) {
            MmModifiedPageMaximum = 800;
            MmSystemCacheWsMinimum += 500;
            MmInPageSupportMinimum += 4;
        }

        if (NT_SUCCESS (MmIsVerifierEnabled (&VerifierFlags))) {

            //
            // The verifier is enabled so don't defer any MDL unlocks because
            // without state, debugging driver bugs in this area is very
            // difficult.
            //

            DeferredMdlEntries = 0;
        }
        else if (MmNumberOfPhysicalPages > ((255*1024*1024)/PAGE_SIZE)) {
            DeferredMdlEntries = 32;
        }
        else if (MmNumberOfPhysicalPages > ((127*1024*1024)/PAGE_SIZE)) {
            DeferredMdlEntries = 8;
        }
        else {
            DeferredMdlEntries = 4;
        }

#if defined(MI_MULTINODE)
        for (i = 0; i < KeNumberNodes; i += 1) {

            InitializeSListHead (&KeNodeBlock[i]->PfnDereferenceSListHead);
            KeNodeBlock[i]->PfnDeferredList = NULL;

            for (j = 0; j < DeferredMdlEntries; j += 1) {

                SingleListEntry = ExAllocatePoolWithTag (NonPagedPool,
                                             sizeof(MI_PFN_DEREFERENCE_CHUNK),
                                             'mDmM');
        
                if (SingleListEntry != NULL) {
                    InterlockedPushEntrySList (&KeNodeBlock[i]->PfnDereferenceSListHead,
                                               SingleListEntry);
                }
            }
        }
#else
        InitializeSListHead (&MmPfnDereferenceSListHead);

        for (j = 0; j < DeferredMdlEntries; j += 1) {
            SingleListEntry = ExAllocatePoolWithTag (NonPagedPool,
                                             sizeof(MI_PFN_DEREFERENCE_CHUNK),
                                             'mDmM');
        
            if (SingleListEntry != NULL) {
                InterlockedPushEntrySList (&MmPfnDereferenceSListHead,
                                           SingleListEntry);
            }
        }
#endif
        
        MmFreedExpansionPoolMaximum = 5;

        if (MmNumberOfPhysicalPages > ((1000*1024*1024)/PAGE_SIZE)) {
            MmFreedExpansionPoolMaximum = 300;
        }
        else if (MmNumberOfPhysicalPages > ((500*1024*1024)/PAGE_SIZE)) {
            MmFreedExpansionPoolMaximum = 100;
        }

        ASSERT (SharedUserData->NumberOfPhysicalPages == 0);

        SharedUserData->NumberOfPhysicalPages = (ULONG) MmNumberOfPhysicalPages;

        SharedUserData->LargePageMinimum = 0;

        //
        // Determine if we are on an AS system (Winnt is not AS).
        //

        if (MmProductType == 0x00690057) {
            SharedUserData->NtProductType = NtProductWinNt;
            MmProductType = 0;
            MmThrottleTop = 250;
            MmThrottleBottom = 30;

        }
        else {
            if (MmProductType == 0x0061004c) {
                SharedUserData->NtProductType = NtProductLanManNt;
            }
            else {
                SharedUserData->NtProductType = NtProductServer;
            }

            MmProductType = 1;
            MmThrottleTop = 450;
            MmThrottleBottom = 80;
            MmMinimumFreePages = 81;
            MmInPageSupportMinimum += 8;
        }

        MiAdjustWorkingSetManagerParameters ((LOGICAL)(MmProductType == 0 ? TRUE : FALSE));

        //
        // Set the ResidentAvailablePages to the number of available
        // pages minus the fluid value.
        //

        MmResidentAvailablePages = MmAvailablePages - MM_FLUID_PHYSICAL_PAGES;

        //
        // Subtract off the size of future nonpaged pool expansion
        // so that nonpaged pool will always be able to expand regardless of
        // prior system load activity.
        //

        MmResidentAvailablePages -= MiExpansionPoolPagesInitialCharge;

        //
        // Subtract off the size of the system cache working set.
        //

        MmResidentAvailablePages -= MmSystemCacheWsMinimum;
        MmResidentAvailableAtInit = MmResidentAvailablePages;

        if (MmResidentAvailablePages < 0) {
#if DBG
            DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                            "system cache working set too big\n");
#endif
            return FALSE;
        }

        //
        // Initialize spin lock for allowing working set expansion.
        //

        KeInitializeSpinLock (&MmExpansionLock);

        KeInitializeGuardedMutex (&MmPageFileCreationLock);

        //
        // Initialize resources for extending sections.
        //

        ExInitializeResourceLite (&MmSectionExtendResource);
        ExInitializeResourceLite (&MmSectionExtendSetResource);

        //
        // Build the system cache structures.
        //

        StartPde = MiGetPdeAddress (MmSystemCacheWorkingSetList);
        PointerPte = MiGetPteAddress (MmSystemCacheWorkingSetList);

#if (_MI_PAGING_LEVELS >= 3)

        TempPte = ValidKernelPte;

#if (_MI_PAGING_LEVELS >= 4)
        StartPxe = MiGetPdeAddress(StartPde);

        if (StartPxe->u.Hard.Valid == 0) {

            //
            // Map in a page directory parent page for the system cache working
            // set.  Note that we only populate one page table for this.
            //

            LOCK_PFN (OldIrql);
            DirectoryFrameIndex = MiRemoveZeroPage (
                MI_GET_PAGE_COLOR_FROM_PTE (StartPxe));
            TempPte.u.Hard.PageFrameNumber = DirectoryFrameIndex;

            MiInitializePfnAndMakePteValid (DirectoryFrameIndex, StartPxe, TempPte);

            MmResidentAvailablePages -= 1;
            UNLOCK_PFN (OldIrql);
        }
#endif

        StartPpe = MiGetPteAddress(StartPde);

        if (StartPpe->u.Hard.Valid == 0) {

            //
            // Map in a page directory page for the system cache working set.
            // Note that we only populate one page table for this.
            //

            LOCK_PFN (OldIrql);
            DirectoryFrameIndex = MiRemoveZeroPage (
                MI_GET_PAGE_COLOR_FROM_PTE (StartPpe));
            TempPte.u.Hard.PageFrameNumber = DirectoryFrameIndex;

            MiInitializePfnAndMakePteValid (DirectoryFrameIndex, StartPpe, TempPte);

            MmResidentAvailablePages -= 1;
            UNLOCK_PFN (OldIrql);
        }

#if (_MI_PAGING_LEVELS >= 4)

        //
        // The shared user data is already initialized and it shares the
        // page table page with the system cache working set list.
        //

        ASSERT (StartPde->u.Hard.Valid == 1);
#else

        //
        // Map in a page table page.
        //

        ASSERT (StartPde->u.Hard.Valid == 0);

        LOCK_PFN (OldIrql);
        PageFrameIndex = MiRemoveZeroPage(
                                MI_GET_PAGE_COLOR_FROM_PTE (StartPde));
        TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

        MiInitializePfnAndMakePteValid (PageFrameIndex, StartPde, TempPte);

        MmResidentAvailablePages -= 1;
        UNLOCK_PFN (OldIrql);
#endif

        StartPpe = MiGetPpeAddress (MmSystemCacheStart);
        StartPde = MiGetPdeAddress (MmSystemCacheStart);
        PointerPte = MiGetVirtualAddressMappedByPte (StartPde);

#else
#if !defined(_X86PAE_)
        ASSERT ((StartPde + 1) == MiGetPdeAddress (MmSystemCacheStart));
#endif
#endif

#if defined(_X86_)

        //
        // Make the system cache virtual size the maximum since 32 bit
        // address space limits us to far less than we'd want anyway.
        //

        MmSystemCacheEnd = (PVOID)(((PCHAR)MmSystemCacheStart +
                                MaximumSystemCacheSize * PAGE_SIZE) - 1);

        MmSizeOfSystemCacheInPages = MaximumSystemCacheSize;

        if (MiMaximumSystemCacheSizeExtra != 0) {
            MmSizeOfSystemCacheInPages += MiMaximumSystemCacheSizeExtra;
            ASSERT (MiSystemCacheStartExtra != 0);
            MiSystemCacheEndExtra = (PVOID)(((PCHAR) MiSystemCacheStartExtra +
                        (MiMaximumSystemCacheSizeExtra) * PAGE_SIZE) - 1);
        }
        else {
            MiSystemCacheStartExtra = MmSystemCacheStart;
            MiSystemCacheEndExtra = MmSystemCacheEnd;
        }
#else
        //
        // For NT64, size the system cache based on the amount of
        // physical memory so we don't go too far.  Note there is still
        // cost to for the preallocated page table pages so factor
        // that in as well (for smaller memory systems).
        //

        MmSizeOfSystemCacheInPages = (PFN_COUNT)((128*1024*1024) >> PAGE_SHIFT);

        i = (MmNumberOfPhysicalPages + 65) / 1024;

        if (i > 4) {

            //
            // System has at least 4032 pages.  Make the system
            // cache 128mb + 16mb for each additional 1024 pages.
            //

            MmSizeOfSystemCacheInPages +=
                            ((i - 4) * ((16*1024*1024) >> PAGE_SHIFT));

            if (MmSizeOfSystemCacheInPages > MaximumSystemCacheSize) {
                MmSizeOfSystemCacheInPages = MaximumSystemCacheSize;
            }
        }

        MmSystemCacheEnd = (PVOID)(((PCHAR)MmSystemCacheStart +
                                MmSizeOfSystemCacheInPages * PAGE_SIZE) - 1);

#endif

        EndPde = MiGetPdeAddress (MmSystemCacheEnd);

        TempPte = ValidKernelPte;
        Color = 0;

#if (_MI_PAGING_LEVELS >= 4)
        StartPxe = MiGetPxeAddress(MmSystemCacheStart);
        if (StartPxe->u.Hard.Valid == 0) {
            FirstPxe = TRUE;
            FirstPpe = TRUE;
        }
        else {
            FirstPxe = FALSE;
            FirstPpe = (StartPpe->u.Hard.Valid == 0) ? TRUE : FALSE;
        }
#elif (_MI_PAGING_LEVELS >= 3)
        FirstPpe = (StartPpe->u.Hard.Valid == 0) ? TRUE : FALSE;
#else
        DirectoryFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (MiGetPteAddress(PDE_BASE));
#endif

        while (StartPde <= EndPde) {

#if (_MI_PAGING_LEVELS >= 4)
            if (FirstPxe == TRUE || MiIsPteOnPpeBoundary(StartPde)) {
                FirstPxe = FALSE;
                StartPxe = MiGetPdeAddress(StartPde);

                //
                // Map in a page directory parent page.
                //

                Color = (MI_SYSTEM_PAGE_COLOR & (MmSecondaryColors - 1));
                MI_SYSTEM_PAGE_COLOR++;

                LOCK_PFN (OldIrql);

                DirectoryFrameIndex = MiRemoveZeroPage (Color);

                TempPte.u.Hard.PageFrameNumber = DirectoryFrameIndex;

                MiInitializePfnAndMakePteValid (DirectoryFrameIndex,
                                                StartPxe,
                                                TempPte);

                MmResidentAvailablePages -= 1;

                UNLOCK_PFN (OldIrql);
            }
#endif

#if (_MI_PAGING_LEVELS >= 3)
            if (FirstPpe == TRUE || MiIsPteOnPdeBoundary(StartPde)) {
                FirstPpe = FALSE;
                StartPpe = MiGetPteAddress(StartPde);

                //
                // Map in a page directory page.
                //

                Color = (MI_SYSTEM_PAGE_COLOR & (MmSecondaryColors - 1));
                MI_SYSTEM_PAGE_COLOR++;

                LOCK_PFN (OldIrql);

                DirectoryFrameIndex = MiRemoveZeroPage (Color);

                TempPte.u.Hard.PageFrameNumber = DirectoryFrameIndex;

                MiInitializePfnAndMakePteValid (DirectoryFrameIndex,
                                                StartPpe,
                                                TempPte);

                MmResidentAvailablePages -= 1;

                UNLOCK_PFN (OldIrql);
            }
#endif

            ASSERT (StartPde->u.Hard.Valid == 0);

            //
            // Map in a page table page.
            //

            Color = (MI_SYSTEM_PAGE_COLOR & (MmSecondaryColors - 1));
            MI_SYSTEM_PAGE_COLOR++;

            LOCK_PFN (OldIrql);

            PageFrameIndex = MiRemoveZeroPage (Color);

            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

            MiInitializePfnAndMakePteValid (PageFrameIndex, StartPde, TempPte);

            MmResidentAvailablePages -= 1;

            UNLOCK_PFN (OldIrql);

            StartPde += 1;
        }

#if defined(_X86_)

        if (ReduceSystemCacheForPtes == TRUE) {

            PMMPTE ExtraPtesPointer;

            ASSERT (MmVirtualBias != 0);

            ReductionInPages = MaximumSystemCacheSize / 3;
            ReductionInPages = MI_ROUND_TO_SIZE (ReductionInPages, PTE_PER_PAGE);
            MaximumSystemCacheSize -= ReductionInPages;

            MmSystemCacheEnd = (PVOID)(((PCHAR)MmSystemCacheStart +
                                MaximumSystemCacheSize * PAGE_SIZE) - 1);

            ASSERT (MiSystemCacheStartExtra == MmSystemCacheStart);
            MiSystemCacheEndExtra = MmSystemCacheEnd;

            MmSizeOfSystemCacheInPages = MaximumSystemCacheSize;

            ExtraPtesPointer = MiGetPteAddress (((ULONG) MmSystemCacheEnd + 1));

            //
            // Increment the system PTEs (for autoconfiguration purposes) but
            // don't actually add the PTEs till later (to prevent
            // fragmentation).
            //

            MiIncrementSystemPtes (ReductionInPages);

            MiAddExtraSystemPteRanges (ExtraPtesPointer, ReductionInPages);
        }

#endif
        //
        // Initialize the system cache.
        //

        MiInitializeSystemCache ((ULONG) MmSystemCacheWsMinimum,
                                 (ULONG) MmAvailablePages);


        MmAttemptForCantExtend.Segment = NULL;
        MmAttemptForCantExtend.RequestedExpansionSize = 1;
        MmAttemptForCantExtend.ActualExpansion = 0;
        MmAttemptForCantExtend.InProgress = FALSE;
        MmAttemptForCantExtend.PageFileNumber = MI_EXTEND_ANY_PAGEFILE;

        KeInitializeEvent (&MmAttemptForCantExtend.Event,
                           NotificationEvent,
                           FALSE);

        //
        // Now that we have booted far enough, replace the temporary
        // commit limits with real ones: set the initial commit page
        // limit to the number of available pages.  This value is
        // updated as paging files are created.
        //

        MmTotalCommitLimit = MmAvailablePages;

        if (MmTotalCommitLimit > 1024) {
            MmTotalCommitLimit -= 1024;
        }

        MmTotalCommitLimitMaximum = MmTotalCommitLimit;

        //
        // Set maximum working set size to 512 pages less than the
        // total available memory.
        //

        MmMaximumWorkingSetSize = (WSLE_NUMBER)(MmAvailablePages - 512);

        if (MmMaximumWorkingSetSize > (MM_MAXIMUM_WORKING_SET - 5)) {
            MmMaximumWorkingSetSize = MM_MAXIMUM_WORKING_SET - 5;
        }

        //
        // Create the modified page writer event.
        //

        KeInitializeEvent (&MmModifiedPageWriterEvent, NotificationEvent, FALSE);

        //
        // Build paged pool.
        //

        MiBuildPagedPool ();

        //
        // Initialize the loaded module list.  This cannot be done until
        // paged pool has been built.
        //

        if (MiInitializeLoadedModuleList (LoaderBlock) == FALSE) {
#if DBG
            DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                            "Loaded module list initialization failed\n");
#endif
            return FALSE;
        }

        //
        // Initialize the handle for the PAGELK section now that all drivers
        // have been relocated to their final resting place and the loaded
        // module list has been initialized.
        //
    
        ExPageLockHandle = MmLockPageableCodeSection ((PVOID)(ULONG_PTR)MmShutdownSystem);
        MmUnlockPageableImageSection (ExPageLockHandle);

        //
        // Initialize the unused segment threshold.  Attempt to keep pool usage
        // below this percentage (by trimming the cache) if pool requests
        // can fail.
        //

        if (MmConsumedPoolPercentage == 0) {
            MmConsumedPoolPercentage = 80;
        }
        else if (MmConsumedPoolPercentage < 5) {
            MmConsumedPoolPercentage = 5;
        }
        else if (MmConsumedPoolPercentage > 100) {
            MmConsumedPoolPercentage = 100;
        }
    
        //
        // Add more system PTEs if this is a large memory system.
        // Note that 64 bit systems can determine the right value at the
        // beginning since there is no virtual address space crunch.
        //

#if !defined (_WIN64)
        if (MmNumberOfPhysicalPages > ((127*1024*1024) >> PAGE_SHIFT)) {

            PMMPTE StartingPte;

            PointerPde = MiGetPdeAddress ((PCHAR)MmPagedPoolEnd + 1);
            StartingPte = MiGetPteAddress ((PCHAR)MmPagedPoolEnd + 1);
            j = 0;

            TempPte = ValidKernelPde;
            LOCK_PFN (OldIrql);
            while (PointerPde->u.Hard.Valid == 0) {

                MiChargeCommitmentCantExpand (1, TRUE);
                MM_TRACK_COMMIT (MM_DBG_COMMIT_EXTRA_SYSTEM_PTES, 1);

                PageFrameIndex = MiRemoveZeroPage (
                                    MI_GET_PAGE_COLOR_FROM_PTE (PointerPde));
                TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
                MiInitializePfnAndMakePteValid (PageFrameIndex, PointerPde, TempPte);
                PointerPde += 1;
                StartingPte += PAGE_SIZE / sizeof(MMPTE);
                j += PAGE_SIZE / sizeof(MMPTE);
                MmResidentAvailablePages -= 1;
            }

            UNLOCK_PFN (OldIrql);

            if (j != 0) {
                StartingPte = MiGetPteAddress ((PCHAR)MmPagedPoolEnd + 1);

                ASSERT (MiPteRanges[0].StartingVa == MmNonPagedSystemStart);
                MmNonPagedSystemStart = MiGetVirtualAddressMappedByPte (StartingPte);
                MiPteRanges[0].StartingVa = MmNonPagedSystemStart;
                MmNumberOfSystemPtes += j;
                MiAddSystemPtes (StartingPte, j, SystemPteSpace);
                MiIncrementSystemPtes (j);
            }
        }

        //
        // Snap a copy of the initial page directory so that when large page
        // system PTE mappings are deleted the proper values can be restored.
        //

        MiInitialSystemPageDirectory = ExAllocatePoolWithTag (
                                            NonPagedPool,
                                            PD_PER_SYSTEM * PAGE_SIZE,
                                            'dPmM');

        if (MiInitialSystemPageDirectory == NULL) { 
#if DBG
            DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                            "can't snap system page directory\n");
#endif
            return FALSE;
        }

        RtlCopyMemory (MiInitialSystemPageDirectory,
                       (PVOID) PDE_BASE,
                       PD_PER_SYSTEM * PAGE_SIZE);

#endif

        MmPageFaultNotifyRoutine = NULL;

        KeInitializeEvent (&MmWorkingSetManagerEvent,
                           SynchronizationEvent,
                           FALSE);

        return TRUE;
    }

    if (Phase == 1) {

#ifdef _X86_
        if (KeFeatureBits & KF_LARGE_PAGE)
#endif
            SharedUserData->LargePageMinimum = MM_MINIMUM_VA_FOR_LARGE_PAGE;

#if defined(_X86_) || defined(_AMD64_)
        MiInitMachineDependent (LoaderBlock);
#endif

        MiMapBBTMemory (LoaderBlock);

        if (!MiSectionInitialization ()) {
            return FALSE;
        }

        //
        // Create double mapped page between kernel and user mode.
        // The PTE is deliberately allocated from paged pool so that
        // it will always have a PTE itself instead of being superpaged.
        // This way, checks throughout the fault handler can assume that
        // the PTE can be checked without having to special case this.
        //

        MmSharedUserDataPte = ExAllocatePoolWithTag (PagedPool,
                                                     sizeof(MMPTE),
                                                     '  mM');

        if (MmSharedUserDataPte == NULL) {
            return FALSE;
        }

        PointerPte = MiGetPteAddress ((PVOID)KI_USER_SHARED_DATA);
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

        MI_MAKE_VALID_KERNEL_PTE (TempPte,
                                  PageFrameIndex,
                                  MM_READONLY,
                                  PointerPte);

        *MmSharedUserDataPte = TempPte;

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        LOCK_PFN (OldIrql);

        Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;

        UNLOCK_PFN (OldIrql);

#ifdef _X86_
        if (MmHighestUserAddress < (PVOID) MM_SHARED_USER_DATA_VA) {

            //
            // Install the PTE mapping now as faults will not because the
            // shared user data is in the system portion of the address space.
            // Note the pagetable page has already been allocated and locked
            // down.
            //

            //
            // Make the mapping user accessible.
            //

            ASSERT (MmSharedUserDataPte->u.Hard.Owner == 0);
            MmSharedUserDataPte->u.Hard.Owner = 1;

            PointerPde = MiGetPdeAddress (MM_SHARED_USER_DATA_VA);
            ASSERT (PointerPde->u.Hard.Owner == 0);
            PointerPde->u.Hard.Owner = 1;

            ASSERT (MiUseMaximumSystemSpace != 0);
            PointerPte = MiGetPteAddress (MM_SHARED_USER_DATA_VA);
            ASSERT (PointerPte->u.Hard.Valid == 0);
            MI_WRITE_VALID_PTE (PointerPte, *MmSharedUserDataPte);
        }
#endif

        MiSessionWideInitializeAddresses ();
        MiInitializeSessionWsSupport ();
        MiInitializeSessionIds ();

        //
        // Start the modified page writer.
        //

        InitializeObjectAttributes (&ObjectAttributes, NULL, 0, NULL, NULL);

        if (!NT_SUCCESS(PsCreateSystemThread (&ThreadHandle,
                                              THREAD_ALL_ACCESS,
                                              &ObjectAttributes,
                                              0L,
                                              NULL,
                                              MiModifiedPageWriter,
                                              NULL))) {
            return FALSE;
        }
        ZwClose (ThreadHandle);

        //
        // Initialize the low and high memory events.  This must be done
        // before starting the working set manager.
        //

        if (MiInitializeMemoryEvents () == FALSE) {
            return FALSE;
        }

        //
        // Start the balance set manager.
        //
        // The balance set manager performs stack swapping and working
        // set management and requires two threads.
        //

        InitializeObjectAttributes (&ObjectAttributes, NULL, 0, NULL, NULL);

        if (!NT_SUCCESS(PsCreateSystemThread (&ThreadHandle,
                                              THREAD_ALL_ACCESS,
                                              &ObjectAttributes,
                                              0L,
                                              NULL,
                                              KeBalanceSetManager,
                                              NULL))) {

            return FALSE;
        }
        ZwClose (ThreadHandle);

        if (!NT_SUCCESS(PsCreateSystemThread (&ThreadHandle,
                                              THREAD_ALL_ACCESS,
                                              &ObjectAttributes,
                                              0L,
                                              NULL,
                                              KeSwapProcessOrStack,
                                              NULL))) {

            return FALSE;
        }
        ZwClose (ThreadHandle);

#if !defined(NT_UP)
        MiStartZeroPageWorkers ();
#endif

#if defined(_X86_)
        MiEnableKernelVerifier ();
#endif

        ExAcquireResourceExclusiveLite (&PsLoadedModuleResource, TRUE);

        NextEntry = PsLoadedModuleList.Flink;

        for ( ; NextEntry != &PsLoadedModuleList; NextEntry = NextEntry->Flink) {

            DataTableEntry = CONTAINING_RECORD(NextEntry,
                                               KLDR_DATA_TABLE_ENTRY,
                                               InLoadOrderLinks);

            NtHeaders = RtlImageNtHeader(DataTableEntry->DllBase);

            if ((NtHeaders != NULL) &&
                (NtHeaders->OptionalHeader.MajorOperatingSystemVersion >= 5) &&
                (NtHeaders->OptionalHeader.MajorImageVersion >= 5)) {
                DataTableEntry->Flags |= LDRP_ENTRY_NATIVE;
            }

            MiWriteProtectSystemImage (DataTableEntry->DllBase);
        }
        ExReleaseResourceLite (&PsLoadedModuleResource);

        InterlockedDecrement (&MiTrimInProgressCount);

        MiFullyInitialized = 1;

        return TRUE;
    }

    if (Phase == 2) {
        MiEnablePagingTheExecutive ();
        return TRUE;
    }

    return FALSE;
}

VOID
MiMapBBTMemory (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

/*++

Routine Description:

    This function walks through the loader block's memory descriptor list
    and maps memory reserved for the BBT buffer into the system.

    The mapped PTEs are PDE-aligned and made user accessible.

Arguments:

    LoaderBlock - Supplies a pointer to the system loader block.

Return Value:

    None.

Environment:

    Kernel Mode Only.  System initialization.

--*/
{
    PVOID Va;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    PLIST_ENTRY NextMd;
    PFN_NUMBER NumberOfPagesMapped;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER PageFrameIndex;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE LastPde;
    MMPTE TempPte;
#if defined(_AMD64_)
    PVOID LastVa;
    PMMPTE PointerPpe;
    PMMPTE LastPpe;
    PMMPTE PointerPxe;
    PMMPTE LastPxe;
#endif

    if (BBTPagesToReserve <= 0) {
        return;
    }

    //
    // Request enough PTEs such that protection can be applied to the PDEs.
    //

    NumberOfPages = (BBTPagesToReserve + (PTE_PER_PAGE - 1)) & ~(PTE_PER_PAGE - 1);

    PointerPte = MiReserveAlignedSystemPtes ((ULONG)NumberOfPages,
                                             SystemPteSpace,
                                             MM_VA_MAPPED_BY_PDE);

    if (PointerPte == NULL) {
        BBTPagesToReserve = 0;
        return;
    }

    //
    // Allow user access to the buffer.
    //

    PointerPde = MiGetPteAddress (PointerPte);
    LastPde = MiGetPteAddress (PointerPte + NumberOfPages);

    ASSERT (LastPde != PointerPde);

    do {
        TempPte = *PointerPde;
        TempPte.u.Long |= MM_PTE_OWNER_MASK;
        MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPde, TempPte);
        PointerPde += 1;
    } while (PointerPde < LastPde);

    Va = MiGetVirtualAddressMappedByPte (PointerPte);

#if defined(_AMD64_)

    LastVa = (PVOID) ((ULONG_PTR) Va + (BBTPagesToReserve * PAGE_SIZE) - 1);

    //
    // Set owner to user for PPEs.
    //

    PointerPpe = MiGetPpeAddress (Va);
    LastPpe = MiGetPpeAddress (LastVa);

    do {
        TempPte = *PointerPpe;
        TempPte.u.Long |= MM_PTE_OWNER_MASK;
        MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPpe, TempPte);
        PointerPpe += 1;
    } while (PointerPpe <= LastPpe);

    //
    // Set owner to user for PXEs.
    //

    PointerPxe = MiGetPxeAddress (Va);
    LastPxe = MiGetPxeAddress (LastVa);

    do {
        TempPte = *PointerPxe;
        TempPte.u.Long |= MM_PTE_OWNER_MASK;
        MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPxe, TempPte);
        PointerPxe += 1;
    } while (PointerPxe <= LastPxe);

#endif

    MI_FLUSH_ENTIRE_TB (8);

    TempPte = ValidUserPte;
    NumberOfPagesMapped = 0;

    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        if (MemoryDescriptor->MemoryType == LoaderBBTMemory) {

            PageFrameIndex = MemoryDescriptor->BasePage;
            NumberOfPages = MemoryDescriptor->PageCount;

            if (NumberOfPagesMapped + NumberOfPages > BBTPagesToReserve) {
                NumberOfPages = BBTPagesToReserve - NumberOfPagesMapped;
            }

            NumberOfPagesMapped += NumberOfPages;

            do {

                TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
                MI_WRITE_VALID_PTE (PointerPte, TempPte);

                PointerPte += 1;
                PageFrameIndex += 1;
                NumberOfPages -= 1;
            } while (NumberOfPages);

            if (NumberOfPagesMapped == BBTPagesToReserve) {
                break;
            }
        }

        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

    KeZeroPages (Va, BBTPagesToReserve << PAGE_SHIFT);

    //
    // Tell BBT_Init how many pages were allocated.
    //

    if (NumberOfPagesMapped < BBTPagesToReserve) {
        BBTPagesToReserve = (ULONG)NumberOfPagesMapped;
    }

    *(PULONG)Va = BBTPagesToReserve;

    //
    // At this point instrumentation code will detect the existence of
    // buffer and initialize the structures.
    //

    BBTBuffer = Va;
}


PPHYSICAL_MEMORY_DESCRIPTOR
MmInitializeMemoryLimits (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PBOOLEAN IncludeType,
    IN OUT PPHYSICAL_MEMORY_DESCRIPTOR InputMemory OPTIONAL
    )

/*++

Routine Description:

    This function walks through the loader block's memory
    descriptor list and builds a list of contiguous physical
    memory blocks of the desired types.

Arguments:

    LoaderBlock - Supplies a pointer the system loader block.

    IncludeType - Array of BOOLEANS of size LoaderMaximum.
                  TRUE means include this type of memory in return.

    Memory - If non-NULL, supplies the physical memory blocks to place the
             search results in.  If NULL, pool is allocated to hold the
             returned search results in - the caller must free this pool.

Return Value:

    A pointer to the physical memory blocks for the requested search or NULL
    on failure.

Environment:

    Kernel Mode Only.  System initialization.

--*/
{
    PLIST_ENTRY NextMd;
    ULONG i;
    ULONG InitialAllocation;
    PFN_NUMBER NextPage;
    PFN_NUMBER TotalPages;
    PPHYSICAL_MEMORY_DESCRIPTOR Memory;
    PPHYSICAL_MEMORY_DESCRIPTOR Memory2;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;

    InitialAllocation = 0;

    if (ARGUMENT_PRESENT (InputMemory)) {
        Memory = InputMemory;
    }
    else {

        //
        // The caller wants us to allocate the return result buffer.  Size it
        // by allocating the maximum possibly needed as this should not be
        // very big (relatively).  It is the caller's responsibility to free
        // this.  Obviously this option can only be requested after pool has
        // been initialized.
        //

        NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

        while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {
            InitialAllocation += 1;
            MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                                 MEMORY_ALLOCATION_DESCRIPTOR,
                                                 ListEntry);
            NextMd = MemoryDescriptor->ListEntry.Flink;
        }

        Memory = ExAllocatePoolWithTag (NonPagedPool,
                                        sizeof(PHYSICAL_MEMORY_DESCRIPTOR) + sizeof(PHYSICAL_MEMORY_RUN) * (InitialAllocation - 1),
                                        'lMmM');

        if (Memory == NULL) {
            return NULL;
        }

        Memory->NumberOfRuns = InitialAllocation;
    }

    //
    // Walk through the memory descriptors and build the physical memory list.
    //

    i = 0;
    TotalPages = 0;
    NextPage = (PFN_NUMBER) -1;

    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        if (MemoryDescriptor->MemoryType < LoaderMaximum &&
            IncludeType [MemoryDescriptor->MemoryType]) {

            TotalPages += MemoryDescriptor->PageCount;

            //
            // Merge runs whenever possible.
            //

            if (MemoryDescriptor->BasePage == NextPage) {
                ASSERT (MemoryDescriptor->PageCount != 0);
                Memory->Run[i - 1].PageCount += MemoryDescriptor->PageCount;
                NextPage += MemoryDescriptor->PageCount;
            }
            else {
                Memory->Run[i].BasePage = MemoryDescriptor->BasePage;
                Memory->Run[i].PageCount = MemoryDescriptor->PageCount;
                NextPage = Memory->Run[i].BasePage + Memory->Run[i].PageCount;
                i += 1;
            }
        }
        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

    ASSERT (i <= Memory->NumberOfRuns);

    if (i == 0) {

        //
        // Don't bother shrinking this as the caller will be freeing it
        // shortly as it is just an empty list.
        //

        Memory->Run[i].BasePage = 0;
        Memory->Run[i].PageCount = 0;
    }
    else if (!ARGUMENT_PRESENT (InputMemory)) {

        //
        // Shrink the buffer (if possible) now that the final size is known.
        //

        if (InitialAllocation > i) {
            Memory2 = ExAllocatePoolWithTag (NonPagedPool,
                                             sizeof(PHYSICAL_MEMORY_DESCRIPTOR) + sizeof(PHYSICAL_MEMORY_RUN) * (i - 1),
                                            'lMmM');

            if (Memory2 != NULL) {
                RtlCopyMemory (Memory2->Run,
                               Memory->Run,
                               sizeof(PHYSICAL_MEMORY_RUN) * i);

                ExFreePool (Memory);
                Memory = Memory2;
            }
        }
    }

    Memory->NumberOfRuns = i;
    Memory->NumberOfPages = TotalPages;

    return Memory;
}


PFN_NUMBER
MiPagesInLoaderBlock (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PBOOLEAN IncludeType
    )

/*++

Routine Description:

    This function walks through the loader block's memory
    descriptor list and returns the number of pages of the desired type.

Arguments:

    LoaderBlock - Supplies a pointer the system loader block.

    IncludeType - Array of BOOLEANS of size LoaderMaximum.
                  TRUE means include this type of memory in the returned count.

Return Value:

    The number of pages of the requested type in the loader block list.

Environment:

    Kernel Mode Only.  System initialization.

--*/
{
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    PLIST_ENTRY NextMd;
    PFN_NUMBER TotalPages;

    //
    // Walk through the memory descriptors counting pages.
    //

    TotalPages = 0;

    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        if (MemoryDescriptor->MemoryType < LoaderMaximum &&
            IncludeType [MemoryDescriptor->MemoryType]) {

            TotalPages += MemoryDescriptor->PageCount;
        }
        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

    return TotalPages;
}


typedef struct _REGISTRY_MEMORY_RUN {
    PFN_NUMBER BasePage;
    PFN_NUMBER PageCount;
    PMMPTE PointerPte;
} REGISTRY_MEMORY_RUN, *PREGISTRY_MEMORY_RUN;

PREGISTRY_MEMORY_RUN MiBootRegistryRuns;

VOID
MmFreeBootRegistry (
    VOID
    )
{
    PMMPTE PointerPte;
    PFN_NUMBER PagesFreed;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PREGISTRY_MEMORY_RUN Runs;

    //
    // Since systemwide commitment was determined early in Phase 0 and
    // excluded the ranges just freed, add them back in now.
    //

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    if (MiBootRegistryRuns != NULL) {

        PagesFreed = 0;

        for (Runs = MiBootRegistryRuns; Runs->BasePage != 0; Runs += 1) {

            PointerPte = Runs->PointerPte;
            PageFrameIndex = Runs->BasePage;
            NumberOfPages = Runs->PageCount;

            while (NumberOfPages != 0) {

                if (MI_IS_PHYSICAL_ADDRESS (
                     MiGetVirtualAddressMappedByPte (PointerPte))) {

                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                    ASSERT (PointerPte == Pfn1->PteAddress);

                    LOCK_PFN (OldIrql);
                    MI_SET_PFN_DELETED (Pfn1);
                    MiDecrementShareCount (Pfn1, PageFrameIndex);
                    UNLOCK_PFN (OldIrql);

                    PagesFreed += 1;
                }
                else {

                    PagesFreed += MiDeleteSystemPageableVm (PointerPte,
                                                           1,
                                                           MI_DELETE_FLUSH_TB,
                                                           NULL);
                }
                NumberOfPages -= 1;
                PointerPte += 1;
                PageFrameIndex += 1;
            }
#if defined(_X86_)
            // LWFIX - pass bigger sizes to MiAddExtraSystemPtes, then rip out 
            // MmKeepBootRegistry (keep 0x1 behavior only)
#endif
        }

        if (PagesFreed != 0) {
            InterlockedExchangeAddSizeT (&MmTotalCommitLimitMaximum, PagesFreed);
            InterlockedExchangeAddSizeT (&MmTotalCommitLimit, PagesFreed);
        }

        ExFreePool (MiBootRegistryRuns);
        MiBootRegistryRuns = NULL;
    }

    return;
}


VOID
MmFreeLoaderBlock (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

/*++

Routine Description:

    This function is called as the last routine in phase 1 initialization.
    It frees memory used by the OsLoader.

Arguments:

    LoaderBlock - Supplies a pointer to the system loader block.

Return Value:

    None.

Environment:

    Kernel Mode Only.  System initialization.

--*/

{
    PVOID VirtualAddress;
    MMPTE TempPte;
    PMMPTE ExpectedPte;
    PETHREAD CurrentThread;
    PLIST_ENTRY NextMd;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    ULONG i;
    ULONG RegistryDescriptors;
    PFN_NUMBER NextPhysicalPage;
    PFN_NUMBER PagesFreed;
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PPHYSICAL_MEMORY_RUN RunBase;
    PPHYSICAL_MEMORY_RUN Runs;
    PREGISTRY_MEMORY_RUN RegistryRuns;

    i = 0;
    RegistryDescriptors = 0;
    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        if (MemoryDescriptor->MemoryType == LoaderRegistryData) {
            RegistryDescriptors += 1;
        }

        i += 1;
        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

    RunBase = ExAllocatePoolWithTag (NonPagedPool,
                                     sizeof(PHYSICAL_MEMORY_RUN) * i,
                                     'lMmM');

    if (RunBase == NULL) {
        return;
    }

    if ((MmKeepBootRegistry & 0x1) && (RegistryDescriptors != 0)) {

        MiBootRegistryRuns = ExAllocatePoolWithTag (NonPagedPool,
                                         sizeof(REGISTRY_MEMORY_RUN) * (RegistryDescriptors + 1),
                                         'lMmM');

        RegistryRuns = MiBootRegistryRuns;

        if (MiBootRegistryRuns != NULL) {

            //
            // Zero this allocation (and its NULL terminator) so it can be
            // walked later when the registry finally releases it.
            //

            RtlZeroMemory (RegistryRuns, sizeof(REGISTRY_MEMORY_RUN) * (RegistryDescriptors + 1));

            //
            // Walk through the memory descriptors and make the
            // boot-loaded registry ranges pageable.
            //
        
            NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;
        
            while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {
        
                MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                                     MEMORY_ALLOCATION_DESCRIPTOR,
                                                     ListEntry);
        
        
                if ((MemoryDescriptor->MemoryType == LoaderRegistryData) &&
                    (MemoryDescriptor->PageCount != 0)) {
    
                    RegistryRuns->BasePage = MemoryDescriptor->BasePage;
                    RegistryRuns->PageCount = MemoryDescriptor->PageCount;
                    RegistryRuns += 1;
                }
        
                NextMd = MemoryDescriptor->ListEntry.Flink;
            }
        }
    
        if (RegistryRuns != MiBootRegistryRuns) {

            CurrentThread = PsGetCurrentThread ();
            RegistryRuns -= 1;

            do {
                i = (ULONG) RegistryRuns->PageCount;
                ASSERT (i != 0);
                NextPhysicalPage = RegistryRuns->BasePage;
    
#if defined (_MI_MORE_THAN_4GB_)
                if (MiNoLowMemory != 0) {
                    if (NextPhysicalPage < MiNoLowMemory) {
    
                        //
                        // Don't make this run pageable as it is below the
                        // memory threshold configured for this system.
                        //
                        // Zero the PageCount so it is not freed later.
                        //
    
                        RegistryRuns->PageCount = 0;
                        RegistryRuns -= 1;
                        continue;
                    }
                }
#endif
    
                Pfn1 = MI_PFN_ELEMENT (NextPhysicalPage);

                //
                // Stash the starting PTE so the
                // range can be easily released later.
                //

                RegistryRuns->PointerPte = Pfn1->PteAddress;
                ASSERT (RegistryRuns->BasePage != 0);

                ExpectedPte = Pfn1->PteAddress;
                VirtualAddress = MiGetVirtualAddressMappedByPte (ExpectedPte);

                LOCK_SYSTEM_WS (CurrentThread);

                LOCK_PFN (OldIrql);

                while (i != 0) {
    
                    ASSERT (Pfn1->u1.Flink == 0);
                    ASSERT (Pfn1->u2.ShareCount == 1);
                    ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
                    ASSERT (Pfn1->OriginalPte.u.Long == 0);
                    ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);

                    ASSERT (Pfn1->PteAddress == ExpectedPte);

                    if (Pfn1->PteAddress != ExpectedPte) {

                        //
                        // The PTE for this PFN is not contiguous with the
                        // prior one so do not attempt to reclaim the VA
                        // space or physical pages later.
                        //

                        RegistryRuns->PageCount = 0;
                    }

                    //
                    // Make the page pageable and put the PTE into transition.
                    // If it is a PFN without a PTE then leave it as nonpaged
                    // until MmFreeBootRegistry is called.
                    //

                    if (!MI_IS_PHYSICAL_ADDRESS (VirtualAddress)) {

                        //
                        // Not a physical address so trim it.
                        //

                        MI_SET_MODIFIED (Pfn1, 1, 0x16);

                        Pfn1->u3.e1.CacheAttribute = MiCached;
                        Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;

                        TempPte = *(Pfn1->PteAddress);

                        ASSERT (TempPte.u.Hard.Valid == 1);

                        MI_MAKE_VALID_PTE_TRANSITION (TempPte, MM_READWRITE);

                        MI_WRITE_INVALID_PTE (Pfn1->PteAddress, TempPte);

                        MiDecrementShareCount (Pfn1, NextPhysicalPage);
                    }

                    Pfn1 += 1;
                    NextPhysicalPage += 1;
                    ExpectedPte += 1;
                    VirtualAddress = (PVOID)((PCHAR)VirtualAddress + PAGE_SIZE);
                    i -= 1;
                }

                MI_FLUSH_ENTIRE_TB (9);

                UNLOCK_PFN (OldIrql);

                UNLOCK_SYSTEM_WS (CurrentThread);

                RegistryRuns -= 1;

            } while (RegistryRuns >= MiBootRegistryRuns);
        }
    }

    Runs = RunBase;

    //
    //
    // Walk through the memory descriptors and add pages to the
    // free list in the PFN database.
    //

    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);


        switch (MemoryDescriptor->MemoryType) {
            case LoaderRegistryData:
                if (MmKeepBootRegistry & 0x1) {
                    break;
                }
            case LoaderOsloaderHeap:
            case LoaderNlsData:

                //
                // Capture the data to temporary storage so we won't
                // free memory we are referencing.
                //

                Runs->BasePage = MemoryDescriptor->BasePage;
                Runs->PageCount = MemoryDescriptor->PageCount;
                Runs += 1;

                break;

            default:

                break;
        }

        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

    PagesFreed = 0;

    LOCK_PFN (OldIrql);

    if (Runs != RunBase) {
        Runs -= 1;
        do {
            i = (ULONG)Runs->PageCount;
            NextPhysicalPage = Runs->BasePage;

#if defined (_MI_MORE_THAN_4GB_)
            if (MiNoLowMemory != 0) {
                if (NextPhysicalPage < MiNoLowMemory) {

                    //
                    // Don't free this run as it is below the memory threshold
                    // configured for this system.
                    //

                    Runs -= 1;
                    continue;
                }
            }
#endif

            Pfn1 = MI_PFN_ELEMENT (NextPhysicalPage);
            while (i != 0) {

                if ((MmKeepBootRegistry & 0x1) && (Pfn1->u2.ShareCount > 1)) {

                    //
                    // Note LoaderOsloaderHeap is used to describe page table
                    // pages used to map the registry so care must be taken
                    // not to blindly free those.  Distinguish them via
                    // the share count check above.
                    //

                    NOTHING;
                }
                else if (Pfn1->u3.e2.ReferenceCount == 0) {
                    if (Pfn1->u1.Flink == 0) {

                        //
                        // Set the PTE address to the physical page for
                        // virtual address alignment checking.
                        //

                        Pfn1->PteAddress =
                                   (PMMPTE)(NextPhysicalPage << PTE_SHIFT);

                        MiDetermineNode (NextPhysicalPage, Pfn1);

                        MiInsertPageInFreeList (NextPhysicalPage);
                        PagesFreed += 1;
                    }
                }
                else {

                    if (NextPhysicalPage != 0) {

                        //
                        // Remove PTE and insert into the free list.  If it is
                        // a physical address within the PFN database, the PTE
                        // element does not exist and therefore cannot be
                        // updated.
                        //

                        if (!MI_IS_PHYSICAL_ADDRESS (
                                MiGetVirtualAddressMappedByPte (Pfn1->PteAddress))) {

                            //
                            // Not a physical address.
                            //

                            Pfn1->PteAddress->u.Long = 0;
                        }

                        MI_SET_PFN_DELETED (Pfn1);
                        MiDecrementShareCount (Pfn1, NextPhysicalPage);
                        PagesFreed += 1;
                    }
                }

                Pfn1 += 1;
                i -= 1;
                NextPhysicalPage += 1;
            }
            Runs -= 1;
        } while (Runs >= RunBase);
    }

#if defined(_X86_)

    if (MmVirtualBias != 0) {

        //
        // If the kernel has been biased to allow for 3gb of user
        // address space, the boot images are doubly mapped at KSEG0_BASE
        // and at ALTERNATE_BASE.  Therefore, the KSEG0_BASE entries must
        // be unmapped.
        //

        PMMPTE Pde;
        ULONG NumberOfPdes;

        NumberOfPdes = MmBootImageSize / MM_VA_MAPPED_BY_PDE;

        Pde = MiGetPdeAddress ((PVOID)KSEG0_BASE);

        for (i = 0; i < NumberOfPdes; i += 1) {
            MI_WRITE_ZERO_PTE (Pde);
            Pde += 1;
        }
    }

#endif

    UNLOCK_PFN (OldIrql);

    //
    // This virtual address space is being deleted and the system cannot be
    // accessing it (or it would be a serious bug) - thus it is safe to
    // flush the TB after releasing the PFN lock.
    //

    MI_FLUSH_ENTIRE_TB (0xA);

    ExFreePool (RunBase);

    //
    // Since systemwide commitment was determined early in Phase 0 and
    // excluded the ranges just freed, add them back in now.
    //

    if (PagesFreed != 0) {
        InterlockedExchangeAddSizeT (&MmTotalCommitLimitMaximum, PagesFreed);
        InterlockedExchangeAddSizeT (&MmTotalCommitLimit, PagesFreed);
    }

    if (MmKeepBootRegistry & 0x2) {
        MmFreeBootRegistry ();
    }

    return;
}

VOID
MiBuildPagedPool (
    VOID
    )

/*++

Routine Description:

    This function is called to build the structures required for paged
    pool and initialize the pool.  Once this routine is called, paged
    pool may be allocated.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel Mode Only.  System initialization.

--*/

{
    SIZE_T Size;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE LastPde;
    PMMPTE PointerPde;
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER ContainingFrame;
    SIZE_T AdditionalCommittedPages;
    KIRQL OldIrql;
    ULONG i;
#if (_MI_PAGING_LEVELS >= 4)
    PMMPTE PointerPxe;
    PMMPTE PointerPxeEnd;
#endif
#if (_MI_PAGING_LEVELS >= 3)
    PVOID LastVa;
    PMMPTE PointerPpe;
    PMMPTE PointerPpeEnd;
#else
    PMMPFN Pfn1;
#endif

    i = 0;
    AdditionalCommittedPages = 0;

#if (_MI_PAGING_LEVELS < 3)

    //
    // Double map system page directory page.
    //

    PointerPte = MiGetPteAddress (PDE_BASE);

    for (i = 0 ; i < PD_PER_SYSTEM; i += 1) {
        MmSystemPageDirectory[i] = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        Pfn1 = MI_PFN_ELEMENT(MmSystemPageDirectory[i]);
        Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
        PointerPte += 1;
    }

    //
    // Was not mapped physically, map it virtually in system space.
    //

    PointerPte = MiReserveSystemPtes (PD_PER_SYSTEM, SystemPteSpace);

    if (PointerPte == NULL) {
        MiIssueNoPtesBugcheck (PD_PER_SYSTEM, SystemPteSpace);
    }

    MmSystemPagePtes = (PMMPTE)MiGetVirtualAddressMappedByPte (PointerPte);

    TempPte = ValidKernelPde;

    for (i = 0 ; i < PD_PER_SYSTEM; i += 1) {
        TempPte.u.Hard.PageFrameNumber = MmSystemPageDirectory[i];
        MI_WRITE_VALID_PTE (PointerPte, TempPte);
        PointerPte += 1;
    }

#endif

    if (MmPagedPoolMaximumDesired == TRUE) {
        MmSizeOfPagedPoolInBytes =
                    ((PCHAR)MmNonPagedSystemStart - (PCHAR)MmPagedPoolStart);
    }
    else if (MmSizeOfPagedPoolInBytes == 0) {

        //
        // A size of 0 means size the pool based on physical memory.
        //

        MmSizeOfPagedPoolInBytes = 2 * MmMaximumNonPagedPoolInBytes;
#if (_MI_PAGING_LEVELS >= 3)
        MmSizeOfPagedPoolInBytes *= 2;
#endif
    }

    if (MmIsThisAnNtAsSystem()) {
        if ((MmNumberOfPhysicalPages > ((24*1024*1024) >> PAGE_SHIFT)) &&
            (MmSizeOfPagedPoolInBytes < MM_MINIMUM_PAGED_POOL_NTAS)) {

            MmSizeOfPagedPoolInBytes = MM_MINIMUM_PAGED_POOL_NTAS;
        }
    }

    if (MmSizeOfPagedPoolInBytes >
              (ULONG_PTR)((PCHAR)MmNonPagedSystemStart - (PCHAR)MmPagedPoolStart)) {
        MmSizeOfPagedPoolInBytes =
                    ((PCHAR)MmNonPagedSystemStart - (PCHAR)MmPagedPoolStart);
    }

    Size = BYTES_TO_PAGES(MmSizeOfPagedPoolInBytes);

    if (Size < MM_MIN_INITIAL_PAGED_POOL) {
        Size = MM_MIN_INITIAL_PAGED_POOL;
    }

    if (Size > (MM_MAX_PAGED_POOL >> PAGE_SHIFT)) {
        Size = MM_MAX_PAGED_POOL >> PAGE_SHIFT;
    }

#if defined (_WIN64)

    //
    // NT64 places system mapped views directly after paged pool.  Ensure
    // enough VA space is available.
    //

    if (Size + (MmSystemViewSize >> PAGE_SHIFT) > (MM_MAX_PAGED_POOL >> PAGE_SHIFT)) {
        ASSERT (MmSizeOfPagedPoolInBytes > 2 * MmSystemViewSize);
        MmSizeOfPagedPoolInBytes -= MmSystemViewSize;
        Size = BYTES_TO_PAGES(MmSizeOfPagedPoolInBytes);
    }
#endif

    Size = (Size + (PTE_PER_PAGE - 1)) / PTE_PER_PAGE;
    MmSizeOfPagedPoolInBytes = (ULONG_PTR)Size * PAGE_SIZE * PTE_PER_PAGE;

    //
    // Set size to the number of pages in the pool.
    //

    Size = Size * PTE_PER_PAGE;

    //
    // If paged pool is really nonpageable then limit the size based
    // on how much physical memory is actually present.  Disable this
    // feature if not enough physical memory is present to do it.
    //

    if (MmDisablePagingExecutive & MM_PAGED_POOL_LOCKED_DOWN) {

        Size = MmSizeOfPagedPoolInBytes / PAGE_SIZE;

        if ((MI_NONPAGEABLE_MEMORY_AVAILABLE() < 2048) ||
            (MmAvailablePages < 2048)) {
                Size = 0;
        }
        else {
            if ((SPFN_NUMBER)(Size) > MI_NONPAGEABLE_MEMORY_AVAILABLE() - 2048) {
                Size = (MI_NONPAGEABLE_MEMORY_AVAILABLE() - 2048);
            }

            if (Size > MmAvailablePages - 2048) {
                Size = MmAvailablePages - 2048;
            }
        }

        Size = ((Size * PAGE_SIZE) / MM_VA_MAPPED_BY_PDE) * MM_VA_MAPPED_BY_PDE;

        if ((((Size / 5) * 4) >= MmSizeOfPagedPoolInBytes) &&
            (Size >= MM_MIN_INITIAL_PAGED_POOL)) {

            MmSizeOfPagedPoolInBytes = Size;
        }
        else {
            MmDisablePagingExecutive &= ~MM_PAGED_POOL_LOCKED_DOWN;
        }

        Size = MmSizeOfPagedPoolInBytes >> PAGE_SHIFT;
    }

    MmSizeOfPagedPoolInPages = MmSizeOfPagedPoolInBytes >> PAGE_SHIFT;

    ASSERT ((MmSizeOfPagedPoolInBytes + (PCHAR)MmPagedPoolStart) <=
            (PCHAR)MmNonPagedSystemStart);

    ASSERT64 ((MmSizeOfPagedPoolInBytes + (PCHAR)MmPagedPoolStart + MmSystemViewSize) <=
              (PCHAR)MmNonPagedSystemStart);

    MmPagedPoolEnd = (PVOID)(((PUCHAR)MmPagedPoolStart +
                            MmSizeOfPagedPoolInBytes) - 1);

    //
    // Build page table page for paged pool.
    //

    PointerPde = MiGetPdeAddress (MmPagedPoolStart);

    TempPte = ValidKernelPde;

#if (_MI_PAGING_LEVELS >= 3)

    //
    // Map in all the page directory pages to span all of paged pool.
    // This removes the need for a system lookup directory.
    //

    LastVa = (PVOID)((PCHAR)MmPagedPoolEnd + MmSystemViewSize);
    PointerPpe = MiGetPpeAddress (MmPagedPoolStart);
    PointerPpeEnd = MiGetPpeAddress (LastVa);

    MiSystemViewStart = (ULONG_PTR)MmPagedPoolEnd + 1;

    PointerPde = MiGetPdeAddress (MmPagedPoolEnd) + 1;
    LastPde = MiGetPdeAddress (LastVa);

    LOCK_PFN (OldIrql);

#if (_MI_PAGING_LEVELS >= 4)
    PointerPxe = MiGetPxeAddress (MmPagedPoolStart);
    PointerPxeEnd = MiGetPxeAddress (LastVa);

    while (PointerPxe <= PointerPxeEnd) {

        if (PointerPxe->u.Hard.Valid == 0) {
            PageFrameIndex = MiRemoveZeroPage (
                                     MI_GET_PAGE_COLOR_FROM_PTE (PointerPxe));
            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

            MiInitializePfnAndMakePteValid (PageFrameIndex, PointerPxe, TempPte);

            MmResidentAvailablePages -= 1;
            AdditionalCommittedPages += 1;
        }

        PointerPxe += 1;
    }
#endif

    while (PointerPpe <= PointerPpeEnd) {

        if (PointerPpe->u.Hard.Valid == 0) {
            PageFrameIndex = MiRemoveZeroPage (
                                     MI_GET_PAGE_COLOR_FROM_PTE (PointerPpe));
            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

            MiInitializePfnAndMakePteValid (PageFrameIndex, PointerPpe, TempPte);

            MmResidentAvailablePages -= 1;
            AdditionalCommittedPages += 1;
        }

        PointerPpe += 1;
    }

    //
    // Initialize the system view page table pages.
    //

    MmResidentAvailablePages -= (LastPde - PointerPde + 1);
    AdditionalCommittedPages += (LastPde - PointerPde + 1);

    while (PointerPde <= LastPde) {

        ASSERT (PointerPde->u.Hard.Valid == 0);

        PageFrameIndex = MiRemoveZeroPage (
                            MI_GET_PAGE_COLOR_FROM_PTE (PointerPde));
        TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

        MiInitializePfnAndMakePteValid (PageFrameIndex, PointerPde, TempPte);

        PointerPde += 1;
    }

    UNLOCK_PFN (OldIrql);

    PointerPde = MiGetPdeAddress (MmPagedPoolStart);

#endif

    PointerPte = MiGetPteAddress (MmPagedPoolStart);
    MmPagedPoolInfo.FirstPteForPagedPool = PointerPte;
    MmPagedPoolInfo.LastPteForPagedPool = MiGetPteAddress (MmPagedPoolEnd);

    MiZeroMemoryPte (PointerPde,
                     (1 + MiGetPdeAddress (MmPagedPoolEnd) - PointerPde));

    LOCK_PFN (OldIrql);

    //
    // Map in a page table page.
    //

    PageFrameIndex = MiRemoveZeroPage (MI_GET_PAGE_COLOR_FROM_PTE (PointerPde));

    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
    MI_WRITE_VALID_PTE (PointerPde, TempPte);

#if (_MI_PAGING_LEVELS >= 3)
    ContainingFrame = MI_GET_PAGE_FRAME_FROM_PTE(MiGetPpeAddress (MmPagedPoolStart));
#else
    ContainingFrame = MmSystemPageDirectory[(PointerPde - MiGetPdeAddress(0)) / PDE_PER_PAGE];
#endif

    MiInitializePfnForOtherProcess (PageFrameIndex,
                                    PointerPde,
                                    ContainingFrame);

    MmResidentAvailablePages -= 1;
    AdditionalCommittedPages += 1;

    UNLOCK_PFN (OldIrql);

    MmPagedPoolInfo.NextPdeForPagedPoolExpansion = PointerPde + 1;

    //
    // Build bitmaps for paged pool.
    //

    MiCreateBitMap (&MmPagedPoolInfo.PagedPoolAllocationMap, Size, NonPagedPool);
    RtlSetAllBits (MmPagedPoolInfo.PagedPoolAllocationMap);

    //
    // Indicate first page worth of PTEs are available.
    //

    RtlClearBits (MmPagedPoolInfo.PagedPoolAllocationMap, 0, PTE_PER_PAGE);

    MiCreateBitMap (&MmPagedPoolInfo.EndOfPagedPoolBitmap, Size, NonPagedPool);
    RtlClearAllBits (MmPagedPoolInfo.EndOfPagedPoolBitmap);

    //
    // If verifier is present then build the verifier paged pool bitmap.
    //

    if (MmVerifyDriverBufferLength != (ULONG)-1) {
        MiCreateBitMap (&VerifierLargePagedPoolMap, Size, NonPagedPool);
        RtlClearAllBits (VerifierLargePagedPoolMap);
    }

    //
    // Initialize paged pool.
    //

    InitializePool (PagedPool, 0L);

    //
    // If paged pool is really nonpageable then allocate the memory now.
    //

    if (MmDisablePagingExecutive & MM_PAGED_POOL_LOCKED_DOWN) {

        PointerPde = MiGetPdeAddress (MmPagedPoolStart);
        PointerPde += 1;
        LastPde = MiGetPdeAddress (MmPagedPoolEnd);
        TempPte = ValidKernelPde;

        PointerPte = MiGetPteAddress (MmPagedPoolStart);
        LastPte = MiGetPteAddress (MmPagedPoolEnd);

        ASSERT (MmPagedPoolCommit == 0);
        MmPagedPoolCommit = (ULONG)(LastPte - PointerPte + 1);

        ASSERT (MmPagedPoolInfo.PagedPoolCommit == 0);
        MmPagedPoolInfo.PagedPoolCommit = MmPagedPoolCommit;

#if DBG
        //
        // Ensure no paged pool has been allocated yet.
        //

        for (i = 0; i < PTE_PER_PAGE; i += 1) {
            ASSERT (!RtlCheckBit (MmPagedPoolInfo.PagedPoolAllocationMap, i));
        }

        while (i < MmSizeOfPagedPoolInBytes / PAGE_SIZE) {
            ASSERT (RtlCheckBit (MmPagedPoolInfo.PagedPoolAllocationMap, i));
            i += 1;
        }
#endif

        RtlClearAllBits (MmPagedPoolInfo.PagedPoolAllocationMap);

        LOCK_PFN (OldIrql);

        //
        // Map in the page table pages.
        //

        MmResidentAvailablePages -= (LastPde - PointerPde + 1);
        AdditionalCommittedPages += (LastPde - PointerPde + 1);

        while (PointerPde <= LastPde) {

            ASSERT (PointerPde->u.Hard.Valid == 0);

            PageFrameIndex = MiRemoveZeroPage(
                                    MI_GET_PAGE_COLOR_FROM_PTE (PointerPde));
            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
            MI_WRITE_VALID_PTE (PointerPde, TempPte);

#if (_MI_PAGING_LEVELS >= 3)
            ContainingFrame = MI_GET_PAGE_FRAME_FROM_PTE(MiGetPteAddress (PointerPde));
#else
            ContainingFrame = MmSystemPageDirectory[(PointerPde - MiGetPdeAddress(0)) / PDE_PER_PAGE];
#endif

            MiInitializePfnForOtherProcess (PageFrameIndex,
                                            MiGetPteAddress (PointerPde),
                                            ContainingFrame);

            PointerPde += 1;
        }

        MmPagedPoolInfo.NextPdeForPagedPoolExpansion = PointerPde;

        TempPte = ValidKernelPte;
        MI_SET_PTE_DIRTY (TempPte);

        ASSERT (MmAvailablePages > (PFN_COUNT)(LastPte - PointerPte + 1));
        ASSERT (MmResidentAvailablePages > (SPFN_NUMBER)(LastPte - PointerPte + 1));
        MmResidentAvailablePages -= (LastPte - PointerPte + 1);
        AdditionalCommittedPages += (LastPte - PointerPte + 1);

        while (PointerPte <= LastPte) {

            ASSERT (PointerPte->u.Hard.Valid == 0);

            PageFrameIndex = MiRemoveAnyPage(
                                    MI_GET_PAGE_COLOR_FROM_PTE (PointerPte));
            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

            MiInitializePfnAndMakePteValid (PageFrameIndex, PointerPte, TempPte);

            PointerPte += 1;
        }

        UNLOCK_PFN (OldIrql);
    }

    //
    // Since the commitment return path is lock free, the total committed
    // page count must be atomically incremented.
    //

    InterlockedExchangeAddSizeT (&MmTotalCommittedPages, AdditionalCommittedPages);

    MiInitializeSpecialPool (NonPagedPool);

    //
    // Initialize the default paged pool signaling thresholds.
    //

    MiLowPagedPoolThreshold = (30 * 1024 * 1024) >> PAGE_SHIFT;

    if ((Size / 5) < MiLowPagedPoolThreshold) {
        MiLowPagedPoolThreshold = Size / 5;
    }

    MiHighPagedPoolThreshold = (60 * 1024 * 1024) >> PAGE_SHIFT;

    if (((Size * 2) / 5) < MiHighPagedPoolThreshold) {
        MiHighPagedPoolThreshold = (Size * 2) / 5;
    }

    ASSERT (MiLowPagedPoolThreshold < MiHighPagedPoolThreshold);

    //
    // Allow mapping of views into system space.
    //

    MiInitializeSystemSpaceMap (NULL);

    return;
}


VOID
MiInitializeNonPagedPoolThresholds (
    VOID
    )

/*++

Routine Description:

    This function is called to initialize the default nonpaged pool
    signaling thresholds.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel Mode Only.  System initialization.

--*/

{
    PFN_NUMBER Size;

    Size = MmMaximumNonPagedPoolInPages;

    //
    // Initialize the default nonpaged pool signaling thresholds.
    //

    MiLowNonPagedPoolThreshold = (8 * 1024 * 1024) >> PAGE_SHIFT;

    if ((Size / 3) < MiLowNonPagedPoolThreshold) {
        MiLowNonPagedPoolThreshold = Size / 3;
    }

    MiHighNonPagedPoolThreshold = (20 * 1024 * 1024) >> PAGE_SHIFT;

    if ((Size / 2) < MiHighNonPagedPoolThreshold) {
        MiHighNonPagedPoolThreshold = Size / 2;
    }

    ASSERT (MiLowNonPagedPoolThreshold < MiHighNonPagedPoolThreshold);

    return;
}


VOID
MiFindInitializationCode (
    OUT PVOID *StartVa,
    OUT PVOID *EndVa
    )

/*++

Routine Description:

    This function locates the start and end of the initialization code for
    each loaded module list entry.  This code resides in the INIT section
    of each image.

Arguments:

    StartVa - Returns the starting address of the init section.

    EndVa - Returns the ending address of the init section.

Return Value:

    None.

Environment:

    Kernel Mode Only.  End of system initialization.

--*/

{
    ULONG Span;
    PKLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    PVOID CurrentBase;
    PVOID InitStart;
    PVOID InitEnd;
    PLIST_ENTRY Next;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_SECTION_HEADER SectionTableEntry;
    PIMAGE_SECTION_HEADER LastDiscard;
    LONG i;
    LOGICAL DiscardSection;
    PVOID MiFindInitializationCodeAddress;
    PKTHREAD CurrentThread;
    UNICODE_STRING NameString;

    MiFindInitializationCodeAddress = MmGetProcedureAddress((PVOID)(ULONG_PTR)&MiFindInitializationCode);

    *StartVa = NULL;

    //
    // Walk through the loader blocks looking for the base which
    // contains this routine.
    //

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);

    //
    // Acquire the load lock to ensure that we don't slice into a load
    // in progress (ie: fixups on INIT code may be ongoing) on a driver
    // already in the list.
    //

    KeWaitForSingleObject (&MmSystemLoadLock,
                           WrVirtualMemory,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)NULL);

    ExAcquireResourceExclusiveLite (&PsLoadedModuleResource, TRUE);

    Next = PsLoadedModuleList.Flink;

    while (Next != &PsLoadedModuleList) {
        LdrDataTableEntry = CONTAINING_RECORD (Next,
                                               KLDR_DATA_TABLE_ENTRY,
                                               InLoadOrderLinks);

        if (LdrDataTableEntry->Flags & LDRP_MM_LOADED) {

            //
            // This entry was loaded by MmLoadSystemImage so it's already
            // had its init section removed.
            //

            Next = Next->Flink;
            continue;
        }

        CurrentBase = (PVOID)LdrDataTableEntry->DllBase;
        NtHeader = RtlImageNtHeader (CurrentBase);

        if (NtHeader == NULL) {
            Next = Next->Flink;
            continue;
        }

        SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader +
                                sizeof(ULONG) +
                                sizeof(IMAGE_FILE_HEADER) +
                                NtHeader->FileHeader.SizeOfOptionalHeader);

        //
        // From the image header, locate the sections named 'INIT',
        // PAGEVRF* and PAGESPEC.  INIT always goes, the others go depending
        // on registry configuration.
        //

        i = NtHeader->FileHeader.NumberOfSections;

        InitStart = NULL;
        while (i > 0) {

#if DBG
            if ((*(PULONG)SectionTableEntry->Name == 'tini') ||
                (*(PULONG)SectionTableEntry->Name == 'egap')) {
                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_INFO_LEVEL, 
                    "driver %wZ has lower case sections (init or pagexxx)\n",
                    &LdrDataTableEntry->FullDllName);
            }
#endif

            DiscardSection = FALSE;

            //
            // Free any INIT sections (or relocation sections that haven't
            // been already).  Note a driver may have a relocation section
            // but not have any INIT code.
            //

            if ((*(PULONG)SectionTableEntry->Name == 'TINI') ||
                ((SectionTableEntry->Characteristics & IMAGE_SCN_MEM_DISCARDABLE) != 0)) {
                DiscardSection = TRUE;
            }
            else if (*(PULONG)SectionTableEntry->Name == 'EGAP') {

                if ((SectionTableEntry->Name[4] == 'V') &&
                     (SectionTableEntry->Name[5] == 'R') &&
                     (SectionTableEntry->Name[6] == 'F')) {

                    //
                    // Discard PAGEVRF* if no drivers are being instrumented.
                    //

                    if (MmVerifyDriverBufferLength == (ULONG)-1) {
                        DiscardSection = TRUE;
                    }
                }
                else if (*(PULONG)&SectionTableEntry->Name[4] == 'CEPS') {

                    //
                    // Discard PAGESPEC special pool code if it's not enabled.
                    //

                    if (MiSpecialPoolFirstPte == NULL) {
                        DiscardSection = TRUE;
                    }
                }
            }

            if (DiscardSection == TRUE) {

                InitStart = (PVOID)((PCHAR)CurrentBase + SectionTableEntry->VirtualAddress);
                //
                // Generally, SizeOfRawData is larger than VirtualSize for each
                // section because it includes the padding to get to the
                // subsection alignment boundary.  However, if the image is
                // linked with subsection alignment == native page alignment,
                // the linker will have VirtualSize be much larger than
                // SizeOfRawData because it will account for all the bss.
                //

                Span = SectionTableEntry->SizeOfRawData;

                if (Span < SectionTableEntry->Misc.VirtualSize) {
                    Span = SectionTableEntry->Misc.VirtualSize;
                }
                InitEnd = (PVOID)((PCHAR)InitStart + Span - 1);
                InitEnd = (PVOID)((PCHAR)PAGE_ALIGN ((PCHAR)InitEnd +
                        (NtHeader->OptionalHeader.SectionAlignment - 1)) - 1);
                InitStart = (PVOID)ROUND_TO_PAGES (InitStart);

                //
                // Check if more sections are discardable after this one so
                // even small INIT sections can be discarded.
                //

                if (i == 1) {
                    LastDiscard = SectionTableEntry;
                }
                else {
                    LastDiscard = NULL;
                    do {
                        i -= 1;
                        SectionTableEntry += 1;

                        if ((SectionTableEntry->Characteristics &
                             IMAGE_SCN_MEM_DISCARDABLE) != 0) {

                            //
                            // Discard this too.
                            //

                            LastDiscard = SectionTableEntry;
                        }
                        else {
                            break;
                        }
                    } while (i > 1);
                }

                if (LastDiscard) {
                    //
                    // Generally, SizeOfRawData is larger than VirtualSize for each
                    // section because it includes the padding to get to the subsection
                    // alignment boundary.  However, if the image is linked with
                    // subsection alignment == native page alignment, the linker will
                    // have VirtualSize be much larger than SizeOfRawData because it
                    // will account for all the bss.
                    //

                    Span = LastDiscard->SizeOfRawData;

                    if (Span < LastDiscard->Misc.VirtualSize) {
                        Span = LastDiscard->Misc.VirtualSize;
                    }

                    InitEnd = (PVOID)(((PCHAR)CurrentBase +
                                       LastDiscard->VirtualAddress) +
                                      (Span - 1));

                    //
                    // If this isn't the last section in the driver then the
                    // the next section is not discardable.  So the last
                    // section is not rounded down, but all others must be.
                    //

                    if (i != 1) {
                        InitEnd = (PVOID)((PCHAR)PAGE_ALIGN ((PCHAR)InitEnd +
                                                             (NtHeader->OptionalHeader.SectionAlignment - 1)) - 1);
                    }
                }

                if (InitEnd > (PVOID)((PCHAR)CurrentBase +
                                      LdrDataTableEntry->SizeOfImage)) {
                    InitEnd = (PVOID)(((ULONG_PTR)CurrentBase +
                                       (LdrDataTableEntry->SizeOfImage - 1)) |
                                      (PAGE_SIZE - 1));
                }

                if (InitStart <= InitEnd) {
                    if ((MiFindInitializationCodeAddress >= InitStart) &&
                        (MiFindInitializationCodeAddress <= InitEnd)) {

                        //
                        // This init section is in the kernel, don't free it
                        // now as it would free this code!
                        //

                        ASSERT (*StartVa == NULL);
                        *StartVa = InitStart;
                        *EndVa = InitEnd;
                    }
                    else {

                        //
                        // Don't free the INIT code for a driver mapped by
                        // large pages because if it unloads later, we'd have
                        // to deal with discontiguous ranges of pages to free.
                        // There would also be the issue of the frames getting
                        // given out and mapped with a conflicting TB attribute
                        // as well ...
                        //
                        // Make a special exception for the kernel & HAL
                        // since those never unload.
                        //

                        if (MI_IS_PHYSICAL_ADDRESS (InitStart)) {

                            NameString.Buffer = (const PUSHORT) KERNEL_NAME;
                            NameString.Length = sizeof (KERNEL_NAME) - sizeof (WCHAR);
                            NameString.MaximumLength = sizeof KERNEL_NAME;

                            if (!RtlEqualUnicodeString (&NameString,
                                                        &LdrDataTableEntry->BaseDllName,
                                                        TRUE)) {
                                MiFreeInitializationCode (InitStart, InitEnd);
                            }

                            NameString.Buffer = (const PUSHORT) HAL_NAME;
                            NameString.Length = sizeof (HAL_NAME) - sizeof (WCHAR);
                            NameString.MaximumLength = sizeof HAL_NAME;

                            if (!RtlEqualUnicodeString (&NameString,
                                                        &LdrDataTableEntry->BaseDllName,
                                                        TRUE)) {
                                MiFreeInitializationCode (InitStart, InitEnd);
                            }
                        }
                        else {
                            MiFreeInitializationCode (InitStart, InitEnd);
                        }
                    }
                }
            }
            i -= 1;
            SectionTableEntry += 1;
        }
        Next = Next->Flink;
    }

    ExReleaseResourceLite (&PsLoadedModuleResource);

    KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);

    KeLeaveCriticalRegionThread (CurrentThread);

    return;
}


VOID
MiFreeInitializationCode (
    IN PVOID StartVa,
    IN PVOID EndVa
    )

/*++

Routine Description:

    This function is called to delete the initialization code for each
    loaded module list entry.

Arguments:

    StartVa - Supplies the starting address of the range to delete.

    EndVa - Supplies the ending address of the range to delete.

Return Value:

    None.

Environment:

    Kernel Mode Only.  Runs after system initialization.

--*/

{
    PMMPTE PointerPte;
    PFN_NUMBER PagesFreed;
    PFN_NUMBER PageFrameIndex;

    ASSERT (ExPageLockHandle);

#if defined (_MI_MORE_THAN_4GB_)
    if (MiNoLowMemory != 0) {

        //
        // Don't free this range as the kernel is always below the memory
        // threshold configured for this system.
        //

        return;
    }
#endif

    PointerPte = MiGetPteAddress (StartVa);

    PagesFreed = (PFN_NUMBER) (1 + MiGetPteAddress (EndVa) - PointerPte);

    if (MI_IS_PHYSICAL_ADDRESS (StartVa)) {

        //
        // Don't put these frames on the general purpose freelist because
        // they lie inside a range being mapped with a large page (or
        // otherwise locked down TB entry).  Thus we must ensure that
        // these are always mapped cached to avoid TB attribute conflict
        // problems.
        //

        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (StartVa);

        MiAddExpansionNonPagedPool (PageFrameIndex, PagesFreed, FALSE);
    }
    else {
        PagesFreed = MiDeleteSystemPageableVm (PointerPte,
                                              PagesFreed,
                                              MI_DELETE_FLUSH_TB,
                                              NULL);
    }

    if (PagesFreed != 0) {
        MiReturnCommitment (PagesFreed);
        MI_INCREMENT_RESIDENT_AVAILABLE (PagesFreed,
                                         MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE1);
    }

    return;
}


VOID
MiEnablePagingTheExecutive (
    VOID
    )

/*++

Routine Description:

    This function locates the start and end of the pageable code for
    each loaded module entry.  This code resides in the PAGE section of
    each image.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel Mode Only.  End of system initialization.

--*/

{
    ULONG Span;
    KIRQL OldIrql;
    PVOID StartVa;
    PETHREAD CurrentThread;
    PLONG SectionLockCountPointer;
    PKLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    PVOID CurrentBase;
    PLIST_ENTRY Next;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_SECTION_HEADER StartSectionTableEntry;
    PIMAGE_SECTION_HEADER SectionTableEntry;
    LONG i;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE SubsectionStartPte;
    PMMPTE SubsectionLastPte;
    LOGICAL PageSection;
    PVOID SectionBaseAddress;
    LOGICAL AlreadyLockedOnce;
    ULONG Waited;

    //
    // Don't page kernel mode code if customer does not want it paged or if
    // this is a diskless remote boot client.
    //

    if (MmDisablePagingExecutive & MM_SYSTEM_CODE_LOCKED_DOWN) {
        return;
    }

#if defined(REMOTE_BOOT)
    if (IoRemoteBootClient && IoCscInitializationFailed) {
        return;
    }
#endif

    //
    // Initializing LastPte is not needed for correctness, but
    // without it the compiler cannot compile this code W4 to check
    // for use of uninitialized variables.
    //

    LastPte = NULL;

    //
    // Walk through the loader blocks looking for the base which
    // contains this routine.
    //

    CurrentThread = PsGetCurrentThread ();

    KeEnterCriticalRegionThread (&CurrentThread->Tcb);

    //
    // Acquire the load lock to ensure that we don't slice into a load
    // in progress (ie: fixups on INIT code may be ongoing) on a driver
    // already in the list.
    //

    KeWaitForSingleObject (&MmSystemLoadLock,
                           WrVirtualMemory,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)NULL);

    ExAcquireResourceExclusiveLite (&PsLoadedModuleResource, TRUE);

    Next = PsLoadedModuleList.Flink;

    while (Next != &PsLoadedModuleList) {

        LdrDataTableEntry = CONTAINING_RECORD (Next,
                                               KLDR_DATA_TABLE_ENTRY,
                                               InLoadOrderLinks);

        if (LdrDataTableEntry->SectionPointer != NULL) {

            //
            // This entry was loaded by MmLoadSystemImage so it's already paged.
            //

            Next = Next->Flink;
            continue;
        }

        CurrentBase = (PVOID)LdrDataTableEntry->DllBase;

        if ((MI_IS_PHYSICAL_ADDRESS (CurrentBase)) ||
            (MI_PDE_MAPS_LARGE_PAGE (MiGetPdeAddress (CurrentBase)))) {

            //
            // Mapped physically, can't be paged.
            //

            Next = Next->Flink;
            continue;
        }

        NtHeader = RtlImageNtHeader (CurrentBase);

        if (NtHeader == NULL) {
            Next = Next->Flink;
            continue;
        }

restart:

        StartSectionTableEntry = NULL;
        SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader +
                                sizeof(ULONG) +
                                sizeof(IMAGE_FILE_HEADER) +
                                NtHeader->FileHeader.SizeOfOptionalHeader);

        //
        // From the image header, locate the section named 'PAGE' or '.edata'.
        //

        i = NtHeader->FileHeader.NumberOfSections;

        PointerPte = NULL;

        while (i > 0) {

            SectionBaseAddress = SECTION_BASE_ADDRESS(SectionTableEntry);

            if ((PUCHAR)SectionBaseAddress ==
                            ((PUCHAR)CurrentBase + SectionTableEntry->VirtualAddress)) {
                AlreadyLockedOnce = TRUE;

                //
                // This subsection has already been locked down (and possibly
                // unlocked as well) at least once.  If it is NOT locked down
                // right now and the pages are not in the system working set
                // then include it in the chunk to be paged.
                //

                SectionLockCountPointer = SECTION_LOCK_COUNT_POINTER (SectionTableEntry);

                if (*SectionLockCountPointer == 0) {

                    SubsectionStartPte = MiGetPteAddress ((PVOID)(ROUND_TO_PAGES (
                                  (ULONG_PTR)CurrentBase +
                                  SectionTableEntry->VirtualAddress)));

                    //
                    // Generally, SizeOfRawData is larger than VirtualSize
                    // for each section because it includes the padding to
                    // get to the subsection alignment boundary.  However,
                    // if the image is linked with subsection alignment ==
                    // native page alignment, the linker will have VirtualSize
                    // be much larger than SizeOfRawData because it
                    // will account for all the bss.
                    //

                    Span = SectionTableEntry->SizeOfRawData;

                    if (Span < SectionTableEntry->Misc.VirtualSize) {
                        Span = SectionTableEntry->Misc.VirtualSize;
                    }

                    SubsectionLastPte = MiGetPteAddress ((PVOID)((ULONG_PTR)CurrentBase +
                                 SectionTableEntry->VirtualAddress +
                                 (NtHeader->OptionalHeader.SectionAlignment - 1) +
                                 Span -
                                 PAGE_SIZE));

                    if (SubsectionLastPte >= SubsectionStartPte) {
                        AlreadyLockedOnce = FALSE;
                    }
                }
            }
            else {
                AlreadyLockedOnce = FALSE;
            }

            PageSection = ((*(PULONG)SectionTableEntry->Name == 'EGAP') ||
                          (*(PULONG)SectionTableEntry->Name == 'ade.')) &&
                           (AlreadyLockedOnce == FALSE);

            if (*(PULONG)SectionTableEntry->Name == 'EGAP') {
                    
                if ((SectionTableEntry->Name[4] == 'K') &&
                    (SectionTableEntry->Name[5] == 'D')) {

                    //
                    // Only pageout PAGEKD if KdPitchDebugger is TRUE.
                    //

                    PageSection = KdPitchDebugger;
                }
                else if ((SectionTableEntry->Name[4] == 'V') &&
                         (SectionTableEntry->Name[5] == 'R') &&
                         (SectionTableEntry->Name[6] == 'F')) {

                    //
                    // Pageout PAGEVRF* if no drivers are being instrumented.
                    //

                    if (MmVerifyDriverBufferLength != (ULONG)-1) {
                        PageSection = FALSE;
                    }
                }
                else if (*(PULONG)&SectionTableEntry->Name[4] == 'CEPS') {

                    //
                    // Pageout PAGESPEC special pool code if it's not enabled.
                    //

                    if (MiSpecialPoolFirstPte != NULL) {
                        PageSection = FALSE;
                    }
                }
            }

            if (PageSection) {

                //
                // This section is pageable, save away the start and end.
                //

                if (PointerPte == NULL) {

                     //
                     // Previous section was NOT pageable, get the start address.
                     //

                     ASSERT (StartSectionTableEntry == NULL);
                     StartSectionTableEntry = SectionTableEntry;
                     PointerPte = MiGetPteAddress ((PVOID)(ROUND_TO_PAGES (
                                  (ULONG_PTR)CurrentBase +
                                  SectionTableEntry->VirtualAddress)));
                }

                //
                // Generally, SizeOfRawData is larger than VirtualSize for each
                // section because it includes the padding to get to the
                // subsection alignment boundary.  However, if the image is
                // linked with subsection alignment == native page alignment,
                // the linker will have VirtualSize be much larger than
                // SizeOfRawData because it will account for all the bss.
                //

                Span = SectionTableEntry->SizeOfRawData;

                if (Span < SectionTableEntry->Misc.VirtualSize) {
                    Span = SectionTableEntry->Misc.VirtualSize;
                }

                LastPte = MiGetPteAddress ((PVOID)((ULONG_PTR)CurrentBase +
                             SectionTableEntry->VirtualAddress +
                             (NtHeader->OptionalHeader.SectionAlignment - 1) +
                             Span - PAGE_SIZE));
            }
            else {

                //
                // This section is not pageable, if the previous section was
                // pageable, enable it.
                //

                if (PointerPte != NULL) {

                    ASSERT (StartSectionTableEntry != NULL);
                    LOCK_SYSTEM_WS (CurrentThread);
                    LOCK_PFN (OldIrql);

                    StartVa = PAGE_ALIGN (StartSectionTableEntry);
                    while (StartVa < (PVOID) SectionTableEntry) {

                        Waited = MiMakeSystemAddressValidPfnSystemWs (StartVa, OldIrql);

                        if (Waited != 0) {

                            //
                            // Restart at the top as the locks were released.
                            //

                            UNLOCK_PFN (OldIrql);
                            UNLOCK_SYSTEM_WS (CurrentThread);
                            goto restart;
                        }
                        StartVa = (PVOID)((PCHAR)StartVa + PAGE_SIZE);
                    }

                    //
                    // Now that we're holding the proper locks, rewalk all
                    // the sections to make sure they weren't locked down
                    // after we checked above.
                    //

                    while (StartSectionTableEntry < SectionTableEntry) {
                        SectionBaseAddress = SECTION_BASE_ADDRESS(StartSectionTableEntry);

                        SectionLockCountPointer = SECTION_LOCK_COUNT_POINTER (StartSectionTableEntry);
                        if (((PUCHAR)SectionBaseAddress ==
                                        ((PUCHAR)CurrentBase + StartSectionTableEntry->VirtualAddress)) &&
                        (*SectionLockCountPointer != 0)) {

                            //
                            // Restart at the top as the section has been
                            // explicitly locked by a driver since we first
                            // checked above.
                            //

                            UNLOCK_PFN (OldIrql);
                            UNLOCK_SYSTEM_WS (CurrentThread);
                            goto restart;
                        }
                        StartSectionTableEntry += 1;
                    }

                    MiEnablePagingOfDriverAtInit (PointerPte, LastPte);

                    UNLOCK_PFN (OldIrql);
                    UNLOCK_SYSTEM_WS (CurrentThread);

                    PointerPte = NULL;
                    StartSectionTableEntry = NULL;
                }
            }
            i -= 1;
            SectionTableEntry += 1;
        }

        if (PointerPte != NULL) {
            ASSERT (StartSectionTableEntry != NULL);
            LOCK_SYSTEM_WS (CurrentThread);
            LOCK_PFN (OldIrql);

            StartVa = PAGE_ALIGN (StartSectionTableEntry);
            while (StartVa < (PVOID) SectionTableEntry) {

                Waited = MiMakeSystemAddressValidPfnSystemWs (StartVa, OldIrql);

                if (Waited != 0) {

                    //
                    // Restart at the top as the locks were released.
                    //

                    UNLOCK_PFN (OldIrql);
                    UNLOCK_SYSTEM_WS (CurrentThread);
                    goto restart;
                }
                StartVa = (PVOID)((PCHAR)StartVa + PAGE_SIZE);
            }

            //
            // Now that we're holding the proper locks, rewalk all
            // the sections to make sure they weren't locked down
            // after we checked above.
            //

            while (StartSectionTableEntry < SectionTableEntry) {
                SectionBaseAddress = SECTION_BASE_ADDRESS(StartSectionTableEntry);

                SectionLockCountPointer = SECTION_LOCK_COUNT_POINTER (StartSectionTableEntry);
                if (((PUCHAR)SectionBaseAddress ==
                                ((PUCHAR)CurrentBase + StartSectionTableEntry->VirtualAddress)) &&
                (*SectionLockCountPointer != 0)) {

                    //
                    // Restart at the top as the section has been
                    // explicitly locked by a driver since we first
                    // checked above.
                    //

                    UNLOCK_PFN (OldIrql);
                    UNLOCK_SYSTEM_WS (CurrentThread);
                    goto restart;
                }
                StartSectionTableEntry += 1;
            }
            MiEnablePagingOfDriverAtInit (PointerPte, LastPte);

            UNLOCK_PFN (OldIrql);
            UNLOCK_SYSTEM_WS (CurrentThread);
        }

        Next = Next->Flink;
    }

    ExReleaseResourceLite (&PsLoadedModuleResource);

    KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);

    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

    return;
}


VOID
MiEnablePagingOfDriverAtInit (
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte
    )

/*++

Routine Description:

    This routine marks the specified range of PTEs as pageable.

Arguments:

    PointerPte - Supplies the starting PTE.

    LastPte - Supplies the ending PTE.

Return Value:

    None.

Environment:

    Working set pushlock AND PFN lock held.

--*/

{
    PVOID Base;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn;
    MMPTE TempPte;
    LOGICAL SessionAddress;
    ULONG FlushCount;
    PVOID VaFlushList[MM_MAXIMUM_FLUSH_COUNT];

    FlushCount = 0;

    MM_PFN_LOCK_ASSERT();

    Base = MiGetVirtualAddressMappedByPte (PointerPte);
    SessionAddress = MI_IS_SESSION_PTE (PointerPte);

    while (PointerPte <= LastPte) {

        //
        // The PTE must be carefully checked as drivers may call MmPageEntire
        // during their DriverEntry yet faults may occur prior to this routine
        // running which cause pages to already be resident and in the working
        // set at this point.  So checks for validity and wsindex must be
        // applied.
        //

        if (PointerPte->u.Hard.Valid == 1) {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
            Pfn = MI_PFN_ELEMENT (PageFrameIndex);
            ASSERT (Pfn->u2.ShareCount == 1);

            if (Pfn->u1.WsIndex == 0) {

                //
                // Set the working set index to zero.  This allows page table
                // pages to be brought back in with the proper WSINDEX.
                //

                MI_ZERO_WSINDEX (Pfn);

                //
                // Original PTE may need to be set for drivers loaded via
                // ntldr.
                //

                if (Pfn->OriginalPte.u.Long == 0) {
                    Pfn->OriginalPte.u.Long = MM_KERNEL_DEMAND_ZERO_PTE;
                    Pfn->OriginalPte.u.Soft.Protection |= MM_EXECUTE;
                }

                MI_SET_MODIFIED (Pfn, 1, 0x11);

                TempPte = *PointerPte;

                MI_MAKE_VALID_PTE_TRANSITION (TempPte,
                                              Pfn->OriginalPte.u.Soft.Protection);

                MI_WRITE_INVALID_PTE (PointerPte, TempPte);

                if (FlushCount < MM_MAXIMUM_FLUSH_COUNT) {
                    VaFlushList[FlushCount] = Base;
                    FlushCount += 1;
                }

                //
                // Flush the TB and decrement the number of valid PTEs
                // within the containing page table page.  Note that for a
                // private page, the page table page is still needed because
                // the page is in transition.
                //

                MiDecrementShareCount (Pfn, PageFrameIndex);

                MI_INCREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_FREE_PAGE_DRIVER);

                MmTotalSystemCodePages += 1;
            }
            else {

                //
                // This would need to be taken out of the WSLEs so skip it for
                // now and let the normal paging algorithms remove it if we
                // run into memory pressure.
                //
            }

        }
        Base = (PVOID)((PCHAR)Base + PAGE_SIZE);
        PointerPte += 1;
    }

    if (FlushCount == 0) {
        NOTHING;
    }
    else if (FlushCount == 1) {
        MI_FLUSH_SINGLE_TB (VaFlushList[0], TRUE);
    }
    else if (FlushCount < MM_MAXIMUM_FLUSH_COUNT) {
        MI_FLUSH_MULTIPLE_TB (FlushCount, &VaFlushList[0], TRUE);
    }
    else {
        MI_FLUSH_ENTIRE_TB (0x15);
    }

    return;
}


MM_SYSTEMSIZE
MmQuerySystemSize (
    VOID
    )
{
    //
    // 12Mb  is small
    // 12-19 is medium
    // > 19 is large
    //
    return MmSystemSize;
}

NTKERNELAPI
BOOLEAN
MmIsThisAnNtAsSystem (
    VOID
    )
{
    return (BOOLEAN)MmProductType;
}

NTKERNELAPI
VOID
FASTCALL
MmSetPageFaultNotifyRoutine (
    PPAGE_FAULT_NOTIFY_ROUTINE NotifyRoutine
    )
{
    MmPageFaultNotifyRoutine = NotifyRoutine;
}

#define CONSTANT_UNICODE_STRING(s)   { sizeof( s ) - sizeof( WCHAR ), sizeof( s ), s }

VOID
MiNotifyMemoryEvents (
    VOID
    )

// PFN lock is held.
{
    if (MmAvailablePages < MmLowMemoryThreshold) {

        if (KeReadStateEvent (MiHighMemoryEvent) != 0) {
            KeClearEvent (MiHighMemoryEvent);
        }

        if (KeReadStateEvent (MiLowMemoryEvent) == 0) {
            KeSetEvent (MiLowMemoryEvent, 0, FALSE);
        }
    }
    else if (MmAvailablePages < MmHighMemoryThreshold) {

        //
        // Gray zone, make sure both events are cleared.
        //

        if (KeReadStateEvent (MiHighMemoryEvent) != 0) {
            KeClearEvent (MiHighMemoryEvent);
        }

        if (KeReadStateEvent (MiLowMemoryEvent) != 0) {
            KeClearEvent (MiLowMemoryEvent);
        }
    }
    else {
        if (KeReadStateEvent (MiHighMemoryEvent) == 0) {
            KeSetEvent (MiHighMemoryEvent, 0, FALSE);
        }

        if (KeReadStateEvent (MiLowMemoryEvent) != 0) {
            KeClearEvent (MiLowMemoryEvent);
        }
    }

    return;
}

LOGICAL
MiInitializeMemoryEvents (
    VOID
    )
{
    KIRQL OldIrql;
    NTSTATUS Status;
    UNICODE_STRING LowMem = CONSTANT_UNICODE_STRING(L"\\KernelObjects\\LowMemoryCondition");
    UNICODE_STRING HighMem = CONSTANT_UNICODE_STRING(L"\\KernelObjects\\HighMemoryCondition");
    UNICODE_STRING LowPagedPoolMem = CONSTANT_UNICODE_STRING(L"\\KernelObjects\\LowPagedPoolCondition");
    UNICODE_STRING HighPagedPoolMem = CONSTANT_UNICODE_STRING(L"\\KernelObjects\\HighPagedPoolCondition");
    UNICODE_STRING LowNonPagedPoolMem = CONSTANT_UNICODE_STRING(L"\\KernelObjects\\LowNonPagedPoolCondition");
    UNICODE_STRING HighNonPagedPoolMem = CONSTANT_UNICODE_STRING(L"\\KernelObjects\\HighNonPagedPoolCondition");

    //
    // The thresholds may be set in the registry, if so, they are interpreted
    // in megabytes so convert them to pages now.
    //
    // If the user modifies the registry to introduce his own values, don't
    // bother error checking them as they can't hurt the system regardless (bad
    // values just may result in events not getting signaled or staying
    // signaled when they shouldn't, but that's not fatal).
    //

    if (MmLowMemoryThreshold != 0) {
        MmLowMemoryThreshold *= ((1024 * 1024) / PAGE_SIZE);
    }
    else {

        //
        // Scale the threshold so on servers the low threshold is
        // approximately 32MB per 4GB, capping it at 64MB.
        //

        MmLowMemoryThreshold = MmPlentyFreePages;

        if (MmNumberOfPhysicalPages > 0x40000) {
            MmLowMemoryThreshold = (32 * 1024 * 1024) / PAGE_SIZE;
            MmLowMemoryThreshold += ((MmNumberOfPhysicalPages - 0x40000) >> 7);
        }
        else if (MmNumberOfPhysicalPages > 0x8000) {
            MmLowMemoryThreshold += ((MmNumberOfPhysicalPages - 0x8000) >> 5);
        }

        if (MmLowMemoryThreshold > (64 * 1024 * 1024) / PAGE_SIZE) {
            MmLowMemoryThreshold = (64 * 1024 * 1024) / PAGE_SIZE;
        }
    }

    if (MmHighMemoryThreshold != 0) {
        MmHighMemoryThreshold *= ((1024 * 1024) / PAGE_SIZE);
    }
    else {
        MmHighMemoryThreshold = 3 * MmLowMemoryThreshold;
        ASSERT (MmHighMemoryThreshold > MmLowMemoryThreshold);
    }

    if (MmHighMemoryThreshold < MmLowMemoryThreshold) {
        MmHighMemoryThreshold = MmLowMemoryThreshold;
    }

    Status = MiCreateMemoryEvent (&LowMem, &MiLowMemoryEvent);

    if (!NT_SUCCESS (Status)) {
#if DBG
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
            "MM: Memory event initialization failed %x\n", Status);
#endif
        return FALSE;
    }

    Status = MiCreateMemoryEvent (&HighMem, &MiHighMemoryEvent);

    if (!NT_SUCCESS (Status)) {
#if DBG
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
            "MM: Memory event initialization failed %x\n", Status);
#endif
        return FALSE;
    }

    //
    // Create the events for the pool thresholds.
    //

    Status = MiCreateMemoryEvent (&LowPagedPoolMem, &MiLowPagedPoolEvent);

    if (!NT_SUCCESS (Status)) {
#if DBG
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
            "MM: Memory event initialization failed %x\n", Status);
#endif
        return FALSE;
    }

    Status = MiCreateMemoryEvent (&HighPagedPoolMem, &MiHighPagedPoolEvent);

    if (!NT_SUCCESS (Status)) {
#if DBG
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
            "MM: Memory event initialization failed %x\n", Status);
#endif
        return FALSE;
    }

    Status = MiCreateMemoryEvent (&LowNonPagedPoolMem, &MiLowNonPagedPoolEvent);

    if (!NT_SUCCESS (Status)) {
#if DBG
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
            "MM: Memory event initialization failed %x\n", Status);
#endif
        return FALSE;
    }

    Status = MiCreateMemoryEvent (&HighNonPagedPoolMem, &MiHighNonPagedPoolEvent);

    if (!NT_SUCCESS (Status)) {
#if DBG
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
            "MM: Memory event initialization failed %x\n", Status);
#endif
        return FALSE;
    }

    //
    // Initialize the pool threshold events based on the current system
    // values.
    //

    MiInitializePoolEvents ();

    //
    // Initialize the event values.
    //

    LOCK_PFN (OldIrql);

    MiNotifyMemoryEvents ();

    UNLOCK_PFN (OldIrql);

    return TRUE;
}

extern POBJECT_TYPE ExEventObjectType;

NTSTATUS
MiCreateMemoryEvent (
    IN PUNICODE_STRING EventName,
    OUT PKEVENT *Event
    )
{
    PACL Dacl;
    HANDLE EventHandle;
    ULONG DaclLength;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_DESCRIPTOR SecurityDescriptor;

    Status = RtlCreateSecurityDescriptor (&SecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    DaclLength = sizeof (ACL) + sizeof (ACCESS_ALLOWED_ACE) * 3 +
                 RtlLengthSid (SeLocalSystemSid) +
                 RtlLengthSid (SeAliasAdminsSid) +
                 RtlLengthSid (SeWorldSid);

    Dacl = ExAllocatePoolWithTag (PagedPool, DaclLength, 'lcaD');

    if (Dacl == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = RtlCreateAcl (Dacl, DaclLength, ACL_REVISION);

    if (!NT_SUCCESS (Status)) {
        ExFreePool (Dacl);
        return Status;
    }

    Status = RtlAddAccessAllowedAce (Dacl,
                                     ACL_REVISION,
                                     SYNCHRONIZE|EVENT_QUERY_STATE|READ_CONTROL,
                                     SeWorldSid);

    if (!NT_SUCCESS (Status)) {
        ExFreePool (Dacl);
        return Status;
    }

    Status = RtlAddAccessAllowedAce (Dacl,
                                     ACL_REVISION,
                                     EVENT_ALL_ACCESS,
                                     SeAliasAdminsSid);

    if (!NT_SUCCESS (Status)) {
        ExFreePool (Dacl);
        return Status;
    }

    Status = RtlAddAccessAllowedAce (Dacl,
                                     ACL_REVISION,
                                     EVENT_ALL_ACCESS,
                                     SeLocalSystemSid);

    if (!NT_SUCCESS (Status)) {
        ExFreePool (Dacl);
        return Status;
    }

  
    Status = RtlSetDaclSecurityDescriptor (&SecurityDescriptor,
                                           TRUE,
                                           Dacl,
                                           FALSE);

    if (!NT_SUCCESS (Status)) {
        ExFreePool (Dacl);
        return Status;
    }
  
    InitializeObjectAttributes (&ObjectAttributes,
                                EventName,
                                OBJ_KERNEL_HANDLE | OBJ_PERMANENT,
                                NULL,
                                &SecurityDescriptor);

    Status = ZwCreateEvent (&EventHandle,
                            EVENT_ALL_ACCESS,
                            &ObjectAttributes,
                            NotificationEvent,
                            FALSE);

    ExFreePool (Dacl);

    if (NT_SUCCESS (Status)) {
        Status = ObReferenceObjectByHandle (EventHandle,
                                            EVENT_MODIFY_STATE,
                                            ExEventObjectType,
                                            KernelMode,
                                            (PVOID *)Event,
                                            NULL);
    }

    ZwClose (EventHandle);

    return Status;
}

BOOLEAN MiAllMainMemoryMustBeCached = FALSE;

VOID
MiInitializeCacheOverrides (
    VOID
    )
{
#if defined (_WIN64)
    ULONG i;
    MMPTE PteContents;
    ULONG NumberOfBytes;
    NTSTATUS Status;
    HAL_PLATFORM_INFORMATION Information;

    //
    // Gather platform information from the HAL.
    //

    Status = HalQuerySystemInformation (HalPlatformInformation, 
                                        sizeof (Information),
                                        &Information,
                                        &NumberOfBytes);

    if (!NT_SUCCESS (Status)) {
        return;
    }

    //
    // Apply mapping modifications based on platform information flags.
    //
    // It would be better if the platform returned what the new cachetype
    // should be.
    //

    if (Information.PlatformFlags & HAL_PLATFORM_DISABLE_UC_MAIN_MEMORY) {

        //
        // The attribute table's noncached entries must be overridden so they
        // are equivalent to cached (for main system memory) since that's
        // all the underlying hardware supports.
        //

        MI_SET_CACHETYPE_TRANSLATION (MmNonCached, 0, MiCached);

        for (i = MM_NOCACHE + 1; i < MM_NOCACHE * 2; i += 1) {
            PteContents.u.Long = MmProtectToPteMask[i];
            MI_ENABLE_CACHING (PteContents);
            MmProtectToPteMask[i] = PteContents.u.Long;
        }
    }

    if (Information.PlatformFlags & HAL_PLATFORM_DISABLE_WRITE_COMBINING) {

        //
        // The attribute table's writecombine entries must be overridden so they
        // are equivalent to cached (for main system memory) since that's
        // all the underlying hardware supports.
        //

        MiAllMainMemoryMustBeCached = TRUE;

        MI_SET_CACHETYPE_TRANSLATION (MmWriteCombined, 0, MiCached);

        for (i = MM_WRITECOMBINE + 1; i < 32; i += 1) {
            PteContents.u.Long = MmProtectToPteMask[i];
            MI_ENABLE_CACHING (PteContents);
            MmProtectToPteMask[i] = PteContents.u.Long;
        }

        if ((Information.PlatformFlags & HAL_PLATFORM_ENABLE_WRITE_COMBINING_MMIO) == 0) {
            MI_SET_CACHETYPE_TRANSLATION (MmWriteCombined, 1, MiNonCached);
        }
    }
#endif

    return;
}

#if defined(_X86_) || defined(_AMD64_)

VOID
MiAddHalIoMappings (
    VOID
    )

/*++

Routine Description:

    This function scans the page directory and page tables for HAL I/O space
    mappings so they can be added to the page attribute table (to prevent
    any subsequent mappings from using a conflicting attribute).  This also
    lets the debugger automatically apply the correct attribute so !dd on
    any of these ranges just works.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode, Phase 0 only.

--*/

{
    ULONG i;
    ULONG j;
    ULONG PdeCount;
    MMPTE PteContents;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PFN_NUMBER PageFrameIndex;
    PVOID VirtualAddress;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

#ifdef _X86_
    VirtualAddress = (PVOID) 0xFFC00000;
#elif defined(_AMD64_)
    VirtualAddress = (PVOID) HAL_VA_START;
#endif

    PointerPde = MiGetPdeAddress (VirtualAddress);

    ASSERT (MiGetPteOffset (VirtualAddress) == 0);

    PdeCount = PDE_PER_PAGE - MiGetPdeOffset (VirtualAddress);

    for (i = 0; i < PdeCount; i += 1) {

        if ((PointerPde->u.Hard.Valid == 1) &&
            (PointerPde->u.Hard.LargePage == 0)) {

            PointerPte = MiGetPteAddress (VirtualAddress);

            for (j = 0 ; j < PTE_PER_PAGE; j += 1) {

                PteContents = *PointerPte;

                if (PteContents.u.Hard.Valid == 1) {

                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);

                    if (!MI_IS_PFN (PageFrameIndex)) {

                        CacheAttribute = MiCached;

                        if ((MI_IS_CACHING_DISABLED (&PteContents)) &&
                            (PteContents.u.Hard.WriteThrough == 1)) {

                            CacheAttribute = MiNonCached;
                        }
                        else if ((MiWriteCombiningPtes == TRUE) &&
                                (PteContents.u.Hard.CacheDisable == 0) &&
                                (PteContents.u.Hard.WriteThrough == 1)) {

                            CacheAttribute = MiWriteCombined;
                        }
                        else if ((MiWriteCombiningPtes == FALSE) &&
                                (PteContents.u.Hard.CacheDisable == 1) &&
                                (PteContents.u.Hard.WriteThrough == 0)) {

                            CacheAttribute = MiWriteCombined;
                        }

                        MiInsertIoSpaceMap (VirtualAddress,
                                            PageFrameIndex,
                                            1,
                                            CacheAttribute);
                    }
                }

                VirtualAddress = (PVOID) ((PCHAR)VirtualAddress + PAGE_SIZE);
                PointerPte += 1;
            }
        }
        else {
            VirtualAddress = (PVOID) ((PCHAR)VirtualAddress + MM_VA_MAPPED_BY_PDE);
        }

        PointerPde += 1;
    }
}

#endif

__declspec(noinline)
PVOID
MiGetInstructionPointer (
    VOID
    )
{
    return (PVOID) _ReturnAddress ();
}

