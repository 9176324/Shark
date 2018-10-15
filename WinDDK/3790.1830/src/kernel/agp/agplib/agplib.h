/*++

Copyright (c) 1997  Microsoft Corporation

Module Name:

    agplib.h

Abstract:

    Private header file for the common AGP library

Author:

    John Vert (jvert) 10/22/1997

Revision History:

   Elliot Shmukler (elliots) 3/24/1999 - Added support for "favored" memory
                                          ranges for AGP physical memory allocation,
                                          fixed some bugs.

--*/
#include "agp.h"
#include "wdmguid.h"
#include "wmilib.h"
#include <devioctl.h>
#include <acpiioct.h>

//
// regstr.h uses things of type WORD, which isn't around in kernel mode.
//

#define _IN_KERNEL_

#include "regstr.h"

#define AGP_HACK_FLAG_SUBSYSTEM 0x01
#define AGP_HACK_FLAG_REVISION  0x02

typedef struct _AGP_HACK_TABLE_ENTRY {
    USHORT VendorID;
    USHORT DeviceID;
    USHORT SubVendorID;
    USHORT SubSystemID;
    ULONGLONG DeviceFlags;
    UCHAR   RevisionID;
    UCHAR   Flags;
} AGP_HACK_TABLE_ENTRY, *PAGP_HACK_TABLE_ENTRY;

extern PAGP_HACK_TABLE_ENTRY AgpDeviceHackTable;
extern PAGP_HACK_TABLE_ENTRY AgpGlobalHackTable;

typedef
ULONG_PTR
(*PCRITICALROUTINE)(
    IN PVOID Extension,
    IN PVOID Context
    );

typedef
NTSTATUS
(*PSTCAP_ROUTINE)( // (S)et (T)arget (C)apability
    IN PVOID AgpExtension,
    IN PPCI_AGP_CAPABILITY Capability
    );

typedef struct _AGP_CRITICAL_ROUTINE_CONTEXT {

    volatile LONG Gate;
    volatile LONG Barrier;

    PCRITICALROUTINE Routine;
    PVOID Extension;
    PVOID Context;

} AGP_CRITICAL_ROUTINE_CONTEXT, *PAGP_CRITICAL_ROUTINE_CONTEXT;

//
// Verifier data
//
typedef enum _AGP_ALLOCATION_TYPE {
    AgpFree     = 0,
    AgpAllocate = 1
} AGP_ALLOCATION_TYPE, *PAGP_ALLOCATION_TYPE;

typedef struct _AGP_ALLOCATION_CONTEXT {
    AGP_ALLOCATION_TYPE AgpAllocationType;
    ULONGLONG TickCount;
    PHYSICAL_ADDRESS BaseAddress;
    LIST_ENTRY AllocationList;
    MDL Mdl;
} AGP_ALLOCATION_CONTEXT, *PAGP_ALLOCATION_CONTEXT;

typedef struct _AGP_VERIFIER_CONTEXT {
    BOOLEAN VerifierDpcEnable;
    ULONG VerifierFlags;
    FAST_MUTEX VerifierLock;
    LIST_ENTRY AllocationList;
    PVOID VerifierPage;
    ULONG VerifierCommand;
    KTIMER VerifierTimer;
    KDPC VerifierDpc;
    PAGPV_PLATFORM_INIT   PlatformInit;
    PAGPV_PLATFORM_WORKER PlatformWorker;
    PAGPV_PLATFORM_STOP   PlatformStop;
    PCALLBACK_OBJECT PowerCallbackObj;
} AGP_VERIFIER_CONTEXT, *PAGP_VERIFIER_CONTEXT;

//
// Define common device extension
//
typedef enum _AGP_EXTENSION_TYPE {
    AgpTargetFilter,
    AgpMasterFilter
} AGP_EXTENSION_TYPE;

#define TARGET_SIG 'TpgA'
#define MASTER_SIG 'MpgA'

typedef struct _COMMON_EXTENSION {
    ULONG               Signature;
    BOOLEAN             Deleted;
    AGP_EXTENSION_TYPE  Type;
    PDEVICE_OBJECT      AttachedDevice;
    BUS_INTERFACE_STANDARD BusInterface;
} COMMON_EXTENSION, *PCOMMON_EXTENSION;

// Structures to maintain a list of "favored" memory ranges
// for AGP allocation.

typedef struct _AGP_MEMORY_RANGE
{
   PHYSICAL_ADDRESS Lower;
   PHYSICAL_ADDRESS Upper;
} AGP_MEMORY_RANGE, *PAGP_MEMORY_RANGE;

typedef struct _AGP_FAVORED_MEMORY
{
   ULONG NumRanges;
   PAGP_MEMORY_RANGE Ranges;
} AGP_FAVORED_MEMORY;

//
// WMI data types
//
#define AGP_WMI_STD_DATA_GUID \
    { 0x8c27fbed, 0x1c7b, 0x47e4, 0xa6, 0x49, 0x0e, 0x38, 0x9d, 0x3a, 0xda, 0x4f }

typedef struct _AGP_STD_DATA {
    PHYSICAL_ADDRESS   ApertureBase;
    ULONG              ApertureLength;
    ULONG              AgpStatus;
    ULONG              AgpCommand;
} AGP_STD_DATA, *PAGP_STD_DATA;


//
// Redirected functions for different flavors of resource handling
//
typedef struct _TARGET_EXTENSION *PTARGET_EXTENSION;

typedef
NTSTATUS
(*PAGP_RESOURCE_FILTER_ROUTINE)(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

typedef
NTSTATUS
(*PAGP_START_TARGET_ROUTINE)(
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

typedef struct _TARGET_EXTENSION {
    COMMON_EXTENSION            CommonExtension;
    //PFAST_MUTEX                 Lock;
    struct _MASTER_EXTENSION    *ChildDevice;
    PCM_RESOURCE_LIST           Resources;
    PCM_RESOURCE_LIST           ResourcesTranslated;
    AGP_FAVORED_MEMORY          FavoredMemory;

    //
    // The following describe the actual hardware resource, and are
    // private to AGP
    //
    PHYSICAL_ADDRESS            GartBase;
    ULONG                       GartLengthInPages;

    ULONG                       Agp3BridgeResourceIndex;
    PAGP_START_TARGET_ROUTINE    StartTarget;
    PAGP_RESOURCE_FILTER_ROUTINE FilterResourceRquirements;
    PDEVICE_OBJECT              PDO;
    PDEVICE_OBJECT              Self;
    WMILIB_CONTEXT              WmiLibInfo;
    AGP_VERIFIER_CONTEXT        AgpV_Ctx;

    //
    // These may be different from GartBase and GartLengthInPages,
    // depending on what AGP verification options are in effect, and
    // are exported in the AGPv3 interface
    //
    PHYSICAL_ADDRESS            AgpBase;
    ULONGLONG                   AgpSize;

    //
    // This must be last, d'oh!
    //
    ULONGLONG                   AgpContext;
} TARGET_EXTENSION, *PTARGET_EXTENSION;

typedef struct _MASTER_EXTENSION {
    COMMON_EXTENSION    CommonExtension;
    PTARGET_EXTENSION   Target;
    ULONG               Capabilities;
    ULONG               InterfaceCount;         // tracks the number of interfaces handed out
    ULONG               ReservedPages;          // tracks the number of pages reserved in the aperture
    BOOLEAN             StopPending;            // TRUE if we have seen a QUERY_STOP
    BOOLEAN             RemovePending;          // TRUE if we have seen a QUERY_REMOVE
    ULONG               DisableCount;           // non-zero if we are in a state where we cannot service requests
} MASTER_EXTENSION, *PMASTER_EXTENSION;

typedef struct _GLOBALS {
   
    // 
    // Path to the driver's Services Key in the registry
    //
    UNICODE_STRING RegistryPath;

    //
    // Cached target capability settings
    //
    ULONG AgpStatus;
    ULONG AgpCommand;

    //
    // AGP Verifier Flags
    //
    ULONG VerifierFlags;

    //
    // Video driver can also disable SBA or FW thru AgpSetRate
    //
    ULONGLONG           VpOverrideFlags;

} GLOBALS;


extern GLOBALS Globals;

//
// The MBAT - used to retrieve "favored" memory ranges from
// the AGP northbridge via an ACPI BANK method
//

#include <pshpack1.h>

typedef struct _MBAT
{
   UCHAR TableVersion;
   UCHAR AgpBusNumber;
   UCHAR ValidEntryBitmap;
   AGP_MEMORY_RANGE DecodeRange[ANYSIZE_ARRAY];
} MBAT, *PMBAT;

#include <poppack.h>

#define MBAT_VERSION 1

#define MAX_MBAT_SIZE sizeof(MBAT) + ((sizeof(UCHAR) * 8) - ANYSIZE_ARRAY) \
                        * sizeof(AGP_MEMORY_RANGE)

#define CM_BANK_METHOD (ULONG)('KNAB')

#define RELEASE_BUS_INTERFACE(_ext_) (_ext_)->CommonExtension.BusInterface.InterfaceDereference((_ext_)->CommonExtension.BusInterface.Context)

//
// Macros for getting from the chipset-specific context to our structures
//
#define GET_TARGET_EXTENSION(_target_,_agp_context_)  (_target_) = (CONTAINING_RECORD((_agp_context_),    \
                                                                                      TARGET_EXTENSION,   \
                                                                                      AgpContext));       \
                                                      ASSERT_TARGET(_target_)
#define GET_MASTER_EXTENSION(_master_,_agp_context_)    {                                                       \
                                                            PTARGET_EXTENSION _targetext_;                      \
                                                            GET_TARGET_EXTENSION(_targetext_, (_agp_context_)); \
                                                            (_master_) = _targetext_->ChildDevice;              \
                                                            ASSERT_MASTER(_master_);                            \
                                                        }
#define GET_TARGET_PDO(_pdo_,_agp_context_) {                                                           \
                                                PTARGET_EXTENSION _targetext_;                          \
                                                GET_TARGET_EXTENSION(_targetext_,(_agp_context_));      \
                                                (_pdo_) = _targetext_->CommonExtension.AttachedDevice;  \
                                            }

#define GET_MASTER_PDO(_pdo_,_agp_context_) {                                                           \
                                                PMASTER_EXTENSION _masterext_;                          \
                                                GET_MASTER_EXTENSION(_masterext_, (_agp_context_));     \
                                                (_pdo_) = _masterext_->CommonExtension.AttachedDevice;  \
                                            }

#define GET_AGP_CONTEXT(_targetext_) ((PVOID)(&(_targetext_)->AgpContext))
#define GET_AGP_CONTEXT_FROM_MASTER(_masterext_) GET_AGP_CONTEXT((_masterext_)->Target)

//
// Some debugging macros
//
#define ASSERT_TARGET(_target_) ASSERT((_target_)->CommonExtension.Signature == TARGET_SIG);
//
// We may not have all our context filled out to this point yet...
//
//                                ASSERT((_target_)->ChildDevice->CommonExtension.Signature == MASTER_SIG)
#define ASSERT_MASTER(_master_) ASSERT((_master_)->CommonExtension.Signature == MASTER_SIG); \
                                ASSERT((_master_)->Target->CommonExtension.Signature == TARGET_SIG)

//
// Locking macros
//
#define LOCK_MUTEX_ASSERT(_fm_) ExAcquireFastMutex(_fm_); \
                                ASSERT((_fm_)->Count == 0)

#define UNLOCK_MUTEX_ASSERT(_fm_) ASSERT((_fm_)->Count == 0);  \
                                  ExReleaseFastMutex(_fm_)

#define LOCK_MUTEX(_fm_) ExAcquireFastMutex(_fm_)

#define UNLOCK_MUTEX(_fm_) ExReleaseFastMutex(_fm_)

//
// The AGP verifier worker must synchronize with the APIs that would
// attempt to modify the Gart, so rather than have two mutexes (originally
// there was a mutex in the Target to synchronize calls to the interface
// APIs) we will simply use the verifier's mutex now
//
#define LOCK_TARGET(_targetext_) ASSERT_TARGET(_targetext_); \
                                 LOCK_MUTEX_ASSERT(&(_targetext_)->AgpV_Ctx.VerifierLock)

#define LOCK_MASTER(_masterext_) \
    ASSERT_MASTER(_masterext_); \
    if (&(_masterext_)->Target->AgpV_Ctx.VerifierDpcEnable) { \
        LOCK_MUTEX(&(_masterext_)->Target->AgpV_Ctx.VerifierLock); } else { \
        LOCK_MUTEX_ASSERT(&(_masterext_)->Target->AgpV_Ctx.VerifierLock); }

#define UNLOCK_TARGET(_targetext_) ASSERT_TARGET(_targetext_); \
                                   UNLOCK_MUTEX_ASSERT(&(_targetext_)->AgpV_Ctx.VerifierLock)

#define UNLOCK_MASTER(_masterext_) \
    ASSERT_MASTER(_masterext_); \
    if (&(_masterext_)->Target->AgpV_Ctx.VerifierDpcEnable) { \
        UNLOCK_MUTEX(&(_masterext_)->Target->AgpV_Ctx.VerifierLock); } else { \
        UNLOCK_MUTEX_ASSERT(&(_masterext_)->Target->AgpV_Ctx.VerifierLock); }

//
// Private resource type definition
//
typedef enum {
    AgpPrivateResource = '0PGA'
} AGP_PRIVATE_RESOURCE_TYPES;

#define JUNK_INDEX 0xBADCEEDE

//
// Define function prototypes
//

//
// Driver and device initialization - init.c
//
NTSTATUS
AgpAttachDeviceRelations(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );


//
// IRP Dispatch routines - dispatch.c
//
NTSTATUS
AgpDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
AgpDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
AgpDispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
AgpDispatchWmi(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
AgpSetEventCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    );

//
// Config space handling routines - config.c
//

ULONG
ApLegacyGetBusData(
    IN PVOID Context,
    IN ULONG DataType,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG
ApLegacySetBusData(
    IN PVOID Context,
    IN ULONG DataType,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

VOID
ApLegacyInterfaceReference(
    PVOID Context
    );

VOID
ApLegacyInterfaceDereference(
    PVOID Context
    );

NTSTATUS
ApGetExtendedAgpCapability(
    IN PAGP_GETSET_CONFIG_SPACE pConfigFn,
    IN PVOID Context,
    IN EXTENDED_AGP_REGISTER RegSelect,
    OUT PVOID ExtCapReg
    );

NTSTATUS
ApSetExtendedAgpCapability(
    IN PAGP_GETSET_CONFIG_SPACE pConfigFn,
    IN PVOID Context,
    IN EXTENDED_AGP_REGISTER RegSelect,
    IN PVOID ExtCapReg
    );

NTSTATUS
ApQueryBusInterface(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PBUS_INTERFACE_STANDARD BusInterface
    );

NTSTATUS
ApQueryAgpTargetBusInterface(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PBUS_INTERFACE_STANDARD BusInterface,
    OUT PUCHAR CapabilityID
    );

//
// Resource handing routines - resource.c
//
NTSTATUS
AgpFilterResourceRequirementsHost(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
Agp3FilterResourceRequirementsBridge(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
AgpFilterResourceRequirements(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
AgpQueryResources(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
AgpStartTarget(
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
Agp3StartTargetBridge(
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
AgpStartTargetHost(
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

VOID
AgpStopTarget(
    IN PTARGET_EXTENSION Extension
    );

//
// AGP Interface functions
//
NTSTATUS
AgpInterfaceSetRate(
    IN PMASTER_EXTENSION Extension,
    IN ULONG AgpRate
    );

VOID
AgpInterfaceReference(
    IN PMASTER_EXTENSION Extension
    );

VOID
AgpInterfaceDereference(
    IN PMASTER_EXTENSION Extension
    );


NTSTATUS
AgpInterfaceReserveMemory(
    IN PMASTER_EXTENSION Extension,
    IN ULONG NumberOfPages,
    IN MEMORY_CACHING_TYPE MemoryType,
    OUT PVOID *MapHandle,
    OUT OPTIONAL PHYSICAL_ADDRESS *PhysicalAddress
    );

NTSTATUS
AgpInterfaceReleaseMemory(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle
    );

NTSTATUS
AgpInterfaceCommitMemory(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    IN OUT PMDL Mdl OPTIONAL,
    OUT PHYSICAL_ADDRESS *MemoryBase
    );

NTSTATUS
AgpInterfaceMapMemory(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    IN PMDL Mdl,
    OUT PHYSICAL_ADDRESS *MemoryBase
    );

NTSTATUS
AgpInterfaceUnMapMemory(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    IN PMDL Mdl
    );

NTSTATUS
AgpInterfaceFreeMemory(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages
    );

NTSTATUS
AgpInterfaceGetMappedPages(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    OUT PMDL Mdl
    );

NTSTATUS
AgpInterfaceGetAgpSize(
    IN PMASTER_EXTENSION Extension,
    OUT PHYSICAL_ADDRESS *AgpBase,
    OUT SIZE_T *AgpSize
    );

//
// Verifier functions
//
VOID
AgpVerifierInitialize(
    IN PTARGET_EXTENSION Extension
    );

VOID
AgpVerifierStop(
    IN PTARGET_EXTENSION Extension
    );

VOID
AgpVerifierDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
AgpVerifierPowerCallback(
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    );

//
// Misc utils.c
//
BOOLEAN
AgpOpenKey(
    IN  PWSTR   KeyName,
    IN  HANDLE  ParentHandle,
    OUT PHANDLE Handle,
    OUT PNTSTATUS Status
    );

BOOLEAN
AgpStringToUSHORT(
    IN PWCHAR String,
    OUT PUSHORT Result
    );

ULONGLONG
AgpGetDeviceFlags(
    IN PAGP_HACK_TABLE_ENTRY AgpHackTable,
    IN USHORT VendorID,
    IN USHORT DeviceID,
    IN USHORT SubVendorID,
    IN USHORT SubSystemID,
    IN UCHAR  RevisionID
    );

ULONG_PTR
AgpExecuteCriticalSystemRoutine(
    IN ULONG_PTR Context
    );

//
// wmi.c
//
NTSTATUS
AgpSystemControl(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP           Irp
    );

NTSTATUS
AgpWmiRegistration(
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
AgpWmiDeRegistration(
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
AgpSetWmiDataItem(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            DataItemId,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );

NTSTATUS
AgpSetWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );

NTSTATUS
AgpQueryWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            InstanceCount,
    IN OUT PULONG       InstanceLengthArray,
    IN ULONG            OutBufferSize,
    OUT PUCHAR          Buffer
    );

NTSTATUS
AgpQueryWmiRegInfo(
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PULONG          RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT  *Pdo
    );

//
// AGP Physical Memory allocator
//
PMDL
AgpLibAllocatePhysicalMemory(
    IN PVOID AgpContext,
    IN ULONG TotalBytes);

//
// Handy structures for mucking about in PCI config space
//

//
// The PCI_COMMON_CONFIG includes the 192 bytes of device specific
// data.  The following structure is used to get only the first 64
// bytes which is all we care about most of the time anyway.  We cast
// to PCI_COMMON_CONFIG to get at the actual fields.
//

typedef struct {
    ULONG Reserved[PCI_COMMON_HDR_LENGTH/sizeof(ULONG)];
} PCI_COMMON_HEADER, *PPCI_COMMON_HEADER;



