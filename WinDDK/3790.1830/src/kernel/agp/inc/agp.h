/*++

Copyright (c) 1997  Microsoft Corporation

Module Name:

    agp.h

Abstract:

    Header file for common AGP library

Author:

    John Vert (jvert) 10/22/1997

Revision History:

--*/
//
// AGP is a driver, make sure we get the appropriate linkage.
//

#define _NTDRIVER_

#include "ntddk.h"
#include "ntagp.h"

//
// regstr.h uses things of type WORD, which isn't around in kernel mode.
//
#define _IN_KERNEL_
#include "regstr.h"

//
// Handy debugging and logging macros
//

//
// Always turned on for now
//
#if DEVL

#define AGP_ALWAYS       0
#define AGP_CRITICAL     1
#define AGP_WARNING      2
#define AGP_NOISE        3
#define AGP_VERIFY       4
#define AGP_IRPTRACE     5
#define AGP_CAP          6

#define AGP_DEBUGGING_OKAY()    \
    (KeGetCurrentIrql() < IPI_LEVEL)

#define AGP_ASSERT  \
    if (AGP_DEBUGGING_OKAY()) ASSERT

extern ULONG AgpLogLevel;
extern ULONG AgpStopLevel;
#define AGPLOG(_level_,_x_) if (((_level_) <= AgpLogLevel) && \
                                AGP_DEBUGGING_OKAY()) DbgPrint _x_; \
                            if (((_level_) <= AgpStopLevel) && \
                                AGP_DEBUGGING_OKAY()) { DbgBreakPoint(); }

#else

#define AGPLOG(_level_,_x_)

#endif

//
// Functions provided by AGPLIB for use by chipset-specific code
//

//
// Helper routines for manipulating AGP Capabilities registers
//
typedef
NTSTATUS
(*PAGP_GETSET_CONFIG_SPACE)(
    IN PVOID Context,
    IN BOOLEAN Read,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

NTSTATUS
AgpLibGetAgpCapability(
    IN PAGP_GETSET_CONFIG_SPACE pConfigFn,
    IN PVOID Context,
    IN BOOLEAN DoSpecial,
    OUT PPCI_AGP_CAPABILITY Capability
    );

NTSTATUS
AgpLibGetTargetCapability(
    IN PVOID AgpExtension,
    OUT PPCI_AGP_CAPABILITY Capability
    );

NTSTATUS
AgpLibGetExtendedTargetCapability(
    IN PVOID AgpExtension,
    IN EXTENDED_AGP_REGISTER RegSelect,
    OUT PVOID ExtCapReg
    );

NTSTATUS
AgpLibGetMasterDeviceId(
    IN PVOID AgpExtension,
    OUT PULONG DeviceId
    );

NTSTATUS
AgpLibGetMasterCapability(
    IN PVOID AgpExtension,
    OUT PPCI_AGP_CAPABILITY Capability
    );

NTSTATUS
AgpLibGetExtendedMasterCapability(
    IN PVOID AgpExtension,
    OUT PPCI_AGP_ISOCH_STATUS IsochStat
    );

VOID
AgpLibReadAgpTargetConfig(
    IN PVOID AgpExtension,
    OUT PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Size
    );

VOID
AgpLibWriteAgpTargetConfig(
    IN PVOID AgpExtension,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Size
    );

NTSTATUS
AgpLibGetPciDeviceCapability(
    IN ULONG BusId,
    IN ULONG SlotId,
    OUT PPCI_AGP_CAPABILITY Capability
    );

NTSTATUS
AgpLibSetAgpCapability(
    IN PAGP_GETSET_CONFIG_SPACE pConfigFn,
    IN PVOID Context,
    IN PPCI_AGP_CAPABILITY Capability
    );

NTSTATUS
AgpLibSetTargetCapability(
    IN PVOID AgpExtension,
    IN PPCI_AGP_CAPABILITY Capability
    );

NTSTATUS
AgpLibSetExtendedTargetCapability(
    IN PVOID AgpExtension,
    IN EXTENDED_AGP_REGISTER RegSelect,
    IN PVOID ExtCapReg
    );

NTSTATUS
AgpLibSetMasterCapability(
    IN PVOID AgpExtension,
    IN PPCI_AGP_CAPABILITY Capability
    );

NTSTATUS
AgpLibSetExtendedMasterCapability(
    IN PVOID AgpExtension,
    IN PPCI_AGP_ISOCH_COMMAND IsochCmd
    );

NTSTATUS
AgpLibSetPciDeviceCapability(
    IN ULONG BusId,
    IN ULONG SlotId,
    IN PPCI_AGP_CAPABILITY Capability
    );

PVOID
AgpLibAllocateMappedPhysicalMemory(
   IN PVOID AgpContext, 
   IN ULONG TotalBytes);

VOID
AgpLibFreeMappedPhysicalMemory(
    IN PVOID Addr,
    IN ULONG Length
    );

//
// Functions implemented by the chipset-specific code
//
typedef struct _AGP_RANGE {
    PHYSICAL_ADDRESS MemoryBase;
    ULONG NumberOfPages;
    MEMORY_CACHING_TYPE Type;
    PVOID Context;
    ULONG CommittedPages;
} AGP_RANGE, *PAGP_RANGE;

//
// These flags have been reserved under AGP_FLAG_SPECIAL_RESERVE
// defined in regstr.h
//
//      AGP_FLAG_SPECIAL_RESERVE 0x000F8000
//
#define AGP_FLAG_SET_RATE_0X     0x00008000
#define AGP_FLAG_SET_RATE_1X     0x00010000
#define AGP_FLAG_SET_RATE_2X     0x00020000
#define AGP_FLAG_SET_RATE_4X     0x00040000
#define AGP_FLAG_SET_RATE_8X     0x00080000

#define AGP_FLAG_SET_RATE_SHIFT  0x00000010

NTSTATUS
AgpSpecialTarget(
    IN PVOID AgpContext,
    IN ULONGLONG DeviceFlags
    );

NTSTATUS
AgpInitializeTarget(
    IN PVOID AgpExtension
    );

NTSTATUS
AgpInitializeMaster(
    IN  PVOID AgpExtension,
    OUT ULONG *AgpCapabilities
    );

NTSTATUS
AgpQueryAperture(
    IN PVOID AgpContext,
    OUT PHYSICAL_ADDRESS *CurrentBase,
    OUT ULONG *CurrentSizeInPages,
    OUT OPTIONAL PIO_RESOURCE_LIST *ApertureRequirements
    );

NTSTATUS
AgpSetAperture(
    IN PVOID AgpContext,
    IN PHYSICAL_ADDRESS NewBase,
    OUT ULONG NewSizeInPages
    );

VOID
AgpDisableAperture(
    IN PVOID AgpContext
    );

NTSTATUS
AgpReserveMemory(
    IN PVOID AgpContext,
    IN OUT AGP_RANGE *AgpRange
    );

NTSTATUS
AgpReleaseMemory(
    IN PVOID AgpContext,
    IN PAGP_RANGE AgpRange
    );

VOID
AgpFindFreeRun(
    IN PVOID AgpContext,
    IN PAGP_RANGE AgpRange,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    OUT ULONG *FreePages,
    OUT ULONG *FreeOffset
    );

VOID
AgpGetMappedPages(
    IN PVOID AgpContext,
    IN PAGP_RANGE AgpRange,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    OUT PMDL Mdl
    );

NTSTATUS
AgpMapMemory(
    IN PVOID AgpContext,
    IN PAGP_RANGE AgpRange,
    IN PMDL Mdl,
    IN ULONG OffsetInPages,
    OUT PHYSICAL_ADDRESS *MemoryBase
    );

NTSTATUS
AgpUnMapMemory(
    IN PVOID AgpContext,
    IN PAGP_RANGE AgpRange,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages
    );

ULONGLONG
AgpQueryImageHack(
    );

typedef
NTSTATUS
(*PAGP_FLUSH_PAGES)(
    IN PVOID AgpContext,
    IN PMDL Mdl
    );

//
// Globals defined by the chipset-specific code
//
extern ULONG AgpExtensionSize;
extern PAGP_FLUSH_PAGES AgpFlushPages;
extern PHYSICAL_ADDRESS AgpMaximumAddress;

//
// AGP Pool tag definitions
//
#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#define ExAllocatePool(_type_,_size_) ExAllocatePoolWithTag(_type_,_size_,' PGA')
#endif

//
// AGP Verifier definitions
//
#define AGP_CONFIGURATION_VERIFICATION   1
#define AGP_GART_ACCESS_VERIFICATION     2
#define AGP_GART_GUARD_VERIFICATION      4
#define AGP_GART_CORRUPTION_VERIFICATION 8

#define AGP_VERIFIER_SIGNATURE 0x33504741 // "AGP3"

#define AGP_VERIFIER_PERIOD_IN_MILISECONDS 3333
#define AGP_VERIFIER_POOL_TAG 'VpgA'

typedef
NTSTATUS
(*PAGPV_PLATFORM_INIT)(
    IN PVOID AgpContext,
    IN PVOID VerifierPage,
    IN OUT PULONG VerifierFlags
    );

typedef
VOID
(*PAGPV_PLATFORM_WORKER)(
    IN PVOID AgpContext
    );

typedef
VOID
(*PAGPV_PLATFORM_STOP)(
    IN PVOID AgpContext
    );

VOID
AgpVerifierRegisterPlatformCallbacks(
    IN PVOID AgpContext,
    IN PAGPV_PLATFORM_INIT   AgpV_PlatformInit,
    IN PAGPV_PLATFORM_WORKER AgpV_PlatformWorker,
    IN PAGPV_PLATFORM_STOP   AgpV_PlatformStop
    );

VOID
AgpVerifierUpdateFlags(
    IN PVOID AgpExtension,
    IN ULONG VerifierFlags
    );

//
// AGPLIB Image hacks.
//

#define AGPLIB_IMAGE_HACK_USE_LEGACY_BUS_INTERFACE  1

//
// Defintions needed for building in XP build environment
//
#if (WINVER < 0x501)
#define AGP_FLAG_NO_FW_ENABLE 0x200
#endif


