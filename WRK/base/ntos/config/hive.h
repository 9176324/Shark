/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    hive.h

Abstract:

    This module contains the private (internal) header file for the
    direct memory loaded hive manager.

--*/

#ifndef _HIVE_
#define _HIVE_

#include "hivedata.h"

#if DBG

extern ULONG HvHiveChecking;

#define DHvCheckHive(a) if(HvHiveChecking) ASSERT(HvCheckHive(a,NULL) == 0)
#define DHvCheckBin(h,a)  if(HvHiveChecking) ASSERT(HvCheckBin(h,a,NULL) == 0)

#else
#define DHvCheckHive(a)
#define DHvCheckBin(h,a)
#endif

#define ROUND_UP(a, b)  \
    ( ((ULONG)(a) + (ULONG)(b) - 1) & ~((ULONG)(b) - 1) )


//
// tombstone for an HBIN that is not resident in memory.  This list is searched
// before any new HBIN is added.
//

#define ASSERT_LISTENTRY(ListEntry) \
    ASSERT((ListEntry)->Flink->Blink==ListEntry); \
    ASSERT((ListEntry)->Blink->Flink==ListEntry);

//
// ===== Hive Private Procedure Prototypes =====
//
PHBIN
HvpAddBin(
    PHHIVE          Hive,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type
    );

PHMAP_ENTRY
HvpGetCellMap(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

VOID
HvpFreeMap(
    PHHIVE          Hive,
    PHMAP_DIRECTORY Dir,
    ULONG           Start,
    ULONG           End
    );

BOOLEAN
HvpAllocateMap(
    PHHIVE          Hive,
    PHMAP_DIRECTORY Dir,
    ULONG           Start,
    ULONG           End
    );

BOOLEAN
HvpGrowLog1(
    PHHIVE  Hive,
    ULONG   Count
    );

BOOLEAN
HvpGrowLog2(
    PHHIVE  Hive,
    ULONG   Size
    );

ULONG
HvpHeaderCheckSum(
    PHBASE_BLOCK    BaseBlock
    );

NTSTATUS
HvpBuildMap(
    PHHIVE  Hive,
    PVOID   Image
    );

NTSTATUS
HvpBuildMapAndCopy(
    PHHIVE  Hive,
    PVOID   Image
    );

NTSTATUS
HvpInitMap(
    PHHIVE  Hive
    );

VOID
HvpCleanMap(
    PHHIVE  Hive
    );

NTSTATUS
HvpEnlistBinInMap(
    PHHIVE  Hive,
    ULONG   Length,
    PHBIN   Bin,
    ULONG   Offset,
    PVOID CmView OPTIONAL
    );

VOID
HvpFreeAllocatedBins(
    PHHIVE Hive
    );

BOOLEAN
HvpDoWriteHive(
    PHHIVE          Hive,
    ULONG           FileType
    );

struct _CELL_DATA *
HvpGetCellFlat(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

struct _CELL_DATA *
HvpGetCellPaged(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

struct _CELL_DATA *
HvpGetCellMapped(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

VOID
HvpReleaseCellMapped(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

VOID
HvpEnlistFreeCell(
    PHHIVE  Hive,
    HCELL_INDEX Cell,
    ULONG      Size,
    HSTORAGE_TYPE   Type,
    BOOLEAN CoalesceForward
    );

BOOLEAN
HvpEnlistFreeCells(
    PHHIVE  Hive,
    PHBIN   Bin,
    ULONG   BinOffset
    );


VOID
HvpDelistFreeCell(
    PHHIVE  Hive,
    HCELL_INDEX  Cell,
    HSTORAGE_TYPE Type
    );

//
// ===== Hive Public Procedure Prototypes =====
//
#define HINIT_CREATE            0
#define HINIT_MEMORY            1
#define HINIT_FILE              2
#define HINIT_MEMORY_INPLACE    3
#define HINIT_FLAT              4
#define HINIT_MAPFILE           5

#define HIVE_VOLATILE                   1
#define HIVE_NOLAZYFLUSH                2
#define HIVE_HAS_BEEN_REPLACED          4
#define HIVE_HAS_BEEN_FREED             8
#define HIVE_IS_UNLOADING               0x20

NTSTATUS
HvInitializeHive(
    PHHIVE                  Hive,
    ULONG                   OperationType,
    ULONG                   HiveFlags,
    ULONG                   FileTypes,
    PVOID                   HiveData OPTIONAL,
    PALLOCATE_ROUTINE       AllocateRoutine,
    PFREE_ROUTINE           FreeRoutine,
    PFILE_SET_SIZE_ROUTINE  FileSetSizeRoutine,
    PFILE_WRITE_ROUTINE     FileWriteRoutine,
    PFILE_READ_ROUTINE      FileReadRoutine,
    PFILE_FLUSH_ROUTINE     FileFlushRoutine,
    ULONG                   Cluster,
    PUNICODE_STRING         FileName
    );

BOOLEAN
HvSyncHive(
    PHHIVE  Hive
    );

NTSTATUS
HvWriteHive(
    PHHIVE  Hive,
	BOOLEAN	DontGrow,
	BOOLEAN	WriteThroughCache,
    BOOLEAN CrashSafe
    );

NTSTATUS
HvLoadHive(
    PHHIVE  Hive
    );

NTSTATUS
HvMapHive(
    PHHIVE  Hive
    );

VOID
HvRefreshHive(
    PHHIVE  Hive
    );

NTSTATUS
HvReadInMemoryHive(
    PHHIVE  Hive,
    PVOID   *HiveImage
    );

ULONG
HvCheckHive(
    PHHIVE  Hive,
    PULONG  Storage OPTIONAL
    );

ULONG
HvCheckBin(
    PHHIVE  Hive,
    PHBIN   Bin,
    PULONG  Storage
    );

ULONG 
HvpGetBinMemAlloc(
                IN PHHIVE           Hive,
                PHBIN               Bin,
                IN HSTORAGE_TYPE    Type
                );

BOOLEAN
HvMarkCellDirty(
    PHHIVE      Hive,
    HCELL_INDEX Cell,
    BOOLEAN     LockHeld
    );

#if DBG
BOOLEAN
HvIsCellDirty(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell
    );

#define ASSERT_CELL_DIRTY(_Hive_,_Cell_) ASSERT(HvIsCellDirty(_Hive_,_Cell_) == TRUE)

#else
#define ASSERT_CELL_DIRTY(_Hive_,_Cell_) // nothing
#endif //DBG

BOOLEAN
HvMarkDirty(
    PHHIVE      Hive,
    HCELL_INDEX Start,
    ULONG       Length,
    BOOLEAN     DirtyAndPin
    );

//
// IMPORTANT:
//      Every call to HvGetCell should be matched with a call to HvReleaseCell;
//      HvReleaseCell is only valid for mapped hives.
//
#define HvGetCell(Hive, Cell)       (((Hive)->GetCellRoutine)(Hive, Cell))
#define HvReleaseCell(Hive, Cell)   if((Hive)->ReleaseCellRoutine) ((Hive)->ReleaseCellRoutine)(Hive, Cell)

PHCELL
HvpGetHCell(PHHIVE      Hive,
            HCELL_INDEX Cell
            );

LONG
HvGetCellSize(
    PHHIVE      Hive,
    PVOID       Address
    );

HCELL_INDEX
HvAllocateCell(
    PHHIVE          Hive,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type,
    HCELL_INDEX     Vicinity 
    );

VOID
HvFreeCell(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

HCELL_INDEX
HvReallocateCell(
    PHHIVE      Hive,
    HCELL_INDEX Cell,
    ULONG       NewSize
    );

BOOLEAN
HvIsCellAllocated(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

VOID
HvFreeHive(
    PHHIVE Hive
    );

VOID
HvFreeHivePartial(
    PHHIVE      Hive,
    HCELL_INDEX Start,
    HSTORAGE_TYPE Type
    );

#define CmpFindFirstSetRight KiFindFirstSetRight
#define CmpFindFirstSetLeft KiFindFirstSetLeft

#define HvpComputeIndex(Index, Size)                                    \
    {                                                                   \
        Index = (Size >> HHIVE_FREE_DISPLAY_SHIFT) - 1;                 \
        if (Index >= HHIVE_LINEAR_INDEX ) {                             \
                                                                        \
            /*                                                          \
            ** Too big for the linear lists, compute the exponential    \
            ** list. Shift the index to make sure we cover the whole    \
            ** range.                                                   \
            */                                                          \
            Index >>= 4;                                                \
                                                                        \
            if (Index > 255) {                                          \
                /*                                                      \
                ** Too big for all the lists, use the last index.       \
                */                                                      \
                Index = HHIVE_FREE_DISPLAY_SIZE-1;                      \
            } else {                                                    \
                Index = CmpFindFirstSetLeft[Index] +                    \
                        HHIVE_LINEAR_INDEX;                             \
            }                                                           \
        }                                                               \
    }

VOID
HvpFreeHiveFreeDisplay(
    IN PHHIVE           Hive
    );

NTSTATUS
HvpAdjustHiveFreeDisplay(
    IN PHHIVE           Hive,
    IN ULONG            HiveLength,
    IN HSTORAGE_TYPE    Type
    );

VOID
HvpAddFreeCellHint(
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    ULONG           Index,
    HSTORAGE_TYPE   Type
    );

VOID
HvpRemoveFreeCellHint(
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    ULONG           Index,
    HSTORAGE_TYPE   Type
    );

HCELL_INDEX
HvpFindFreeCell(
    PHHIVE          Hive,
    ULONG           Index,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type,
    HCELL_INDEX     Vicinity 
    );

BOOLEAN
HvpCheckViewBoundary(
                     IN ULONG Start,
                     IN ULONG End
    );

VOID
HvpDropPagedBins(
    PHHIVE  Hive
#if DBG
    ,
    IN BOOLEAN  Check
#endif
    );

VOID
HvpDropAllPagedBins(
    IN PHHIVE   Hive
    );

BOOLEAN
HvpWriteLog(
    PHHIVE          Hive
    );


BOOLEAN
HvHiveWillShrink(
                    IN PHHIVE Hive
                    );

BOOLEAN HvAutoCompressCheck(PHHIVE Hive);

NTSTATUS
HvCloneHive(PHHIVE  SourceHive,
            PHHIVE  DestHive,
            PULONG  NewLength
            );

NTSTATUS 
HvShrinkHive(PHHIVE  Hive,
             ULONG   NewLength
            );

HCELL_INDEX
HvShiftCell(PHHIVE Hive,HCELL_INDEX Cell);

HCELL_INDEX
HvDuplicateCell(    
                    PHHIVE          Hive,
                    HCELL_INDEX     Cell,
                    HSTORAGE_TYPE   Type,
                    BOOLEAN         CopyData
                );

typedef struct _HV_HIVE_CELL_PAIR {
    PHHIVE      Hive;
    HCELL_INDEX Cell;
} HV_HIVE_CELL_PAIR,*PHV_HIVE_CELL_PAIR;

#define STATIC_CELL_PAIR_COUNT    4

typedef struct _HV_TRACK_CELL_REF {
    USHORT              Count;
    USHORT              Max;
    PHV_HIVE_CELL_PAIR  CellArray;
    HV_HIVE_CELL_PAIR   StaticArray[STATIC_CELL_PAIR_COUNT];
    USHORT              StaticCount;
} HV_TRACK_CELL_REF,*PHV_TRACK_CELL_REF;

BOOLEAN HvTrackCellRef(PHV_TRACK_CELL_REF   CellRef,
                       PHHIVE               Hive,
                       HCELL_INDEX          Cell);

VOID
HvReleaseFreeCellRefArray(PHV_TRACK_CELL_REF   CellRef);

#endif // _HIVE_
