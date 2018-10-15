/*++

Copyright (C) Microsoft Corporation, 1991 - 1999

Module Name:

    classpnp.h

Abstract:

    These are the structures and defines that are used in the
    SCSI class drivers.

Author:

    Mike Glass (mglass)
    Jeff Havens (jhavens)

Revision History:

--*/

#ifndef _CLASS_
#define _CLASS_

#include <ntdddisk.h>
#include <ntddcdrm.h>
#include <ntddtape.h>
#include <ntddscsi.h>
#include "ntddstor.h"

#include <stdio.h>

#include <scsi.h>

#if defined DebugPrint
    #undef DebugPrint
#endif

#ifdef TRY
    #undef TRY
#endif
#ifdef LEAVE
    #undef LEAVE
#endif
#ifdef FINALLY
    #undef FINALLY
#endif

#define TRY
#define LEAVE   goto __tryLabel;
#define FINALLY __tryLabel:

// #define ALLOCATE_SRB_FROM_POOL

#if DBG

#define DebugPrint(x) ClassDebugPrint x

#else

#define DebugPrint(x)

#endif // DBG

#define DEBUG_BUFFER_LENGTH 256

//
// Define our private SRB flags.  The high nibble of the flag field is
// reserved for class drivers's private use.
//

//
// Used to indicate that this request shouldn't invoke any power type operations
// like spinning up the drive.
//

#define SRB_CLASS_FLAGS_LOW_PRIORITY      0x10000000

//
// Used to indicate that the completion routine should not free the srb.
//

#define SRB_CLASS_FLAGS_PERSISTANT        0x20000000

//
// Used to indicate that an SRB is the result of a paging operation.
//

#define SRB_CLASS_FLAGS_PAGING            0x40000000

//
// Random macros which should probably be in the system header files
// somewhere.
//

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

//
// Bit Flag Macros
//

#define SET_FLAG(Flags, Bit)    ((Flags) |= (Bit))
#define CLEAR_FLAG(Flags, Bit)  ((Flags) &= ~(Bit))
#define TEST_FLAG(Flags, Bit)   (((Flags) & (Bit)) != 0)

//
// neat little hacks to count number of bits set efficiently
//
__inline ULONG CountOfSetBitsUChar(UCHAR _X)
{ ULONG i = 0; while (_X) { _X &= _X - 1; i++; } return i; }
__inline ULONG CountOfSetBitsULong(ULONG _X)
{ ULONG i = 0; while (_X) { _X &= _X - 1; i++; } return i; }
__inline ULONG CountOfSetBitsULong32(ULONG32 _X)
{ ULONG i = 0; while (_X) { _X &= _X - 1; i++; } return i; }
__inline ULONG CountOfSetBitsULong64(ULONG64 _X)
{ ULONG i = 0; while (_X) { _X &= _X - 1; i++; } return i; }
__inline ULONG CountOfSetBitsUlongPtr(ULONG_PTR _X)
{ ULONG i = 0; while (_X) { _X &= _X - 1; i++; } return i; }


//
// Helper macros to verify data types and cleanup the code.
//

#define ASSERT_FDO(x) \
    ASSERT(((PCOMMON_DEVICE_EXTENSION) (x)->DeviceExtension)->IsFdo)

#define ASSERT_PDO(x) \
    ASSERT(!(((PCOMMON_DEVICE_EXTENSION) (x)->DeviceExtension)->IsFdo))

#define IS_CLEANUP_REQUEST(majorFunction)   \
        ((majorFunction == IRP_MJ_CLOSE) ||     \
         (majorFunction == IRP_MJ_CLEANUP) ||   \
         (majorFunction == IRP_MJ_SHUTDOWN))

#define DO_MCD(fdoExtension)                                        \
    (((fdoExtension)->MediaChangeDetectionInfo != NULL) &&          \
     ((fdoExtension)->MediaChangeDetectionInfo->MediaChangeDetectionDisableCount == 0))

#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'nUcS')
//#define ExAllocatePool(a,b) #assert(0)
#define ExAllocatePoolWithQuota(a,b) ExAllocatePoolWithQuotaTag(a,b,'nUcS')
#endif

#define CLASS_TAG_AUTORUN_DISABLE           'ALcS'
#define CLASS_TAG_FILE_OBJECT_EXTENSION     'FLcS'
#define CLASS_TAG_MEDIA_CHANGE_DETECTION    'MLcS'
#define CLASS_TAG_MOUNT                     'mLcS'
#define CLASS_TAG_RELEASE_QUEUE             'qLcS'
#define CLASS_TAG_POWER                     'WLcS'
#define CLASS_TAG_WMI                       'wLcS'
#define CLASS_TAG_FAILURE_PREDICT           'fLcS'
#define CLASS_TAG_DEVICE_CONTROL            'OIcS'

#define MAXIMUM_RETRIES 4

#define CLASS_DRIVER_EXTENSION_KEY ((PVOID) ClassInitialize)

struct _CLASS_INIT_DATA;
typedef struct _CLASS_INIT_DATA CLASS_INIT_DATA, *PCLASS_INIT_DATA;

//
// Possible values for the IsRemoved flag
//

#define NO_REMOVE 0
#define REMOVE_PENDING 1
#define REMOVE_COMPLETE 2

#define ClassAcquireRemoveLock(devobj, tag) \
    ClassAcquireRemoveLockEx(devobj, tag, __FILE__, __LINE__)

//
// Define start unit timeout to be 4 minutes.
//

#define START_UNIT_TIMEOUT  (60 * 4)

//
// Define media change test time to be 4 seconds

#define MEDIA_CHANGE_DEFAULT_TIME    4

#ifdef DBG

//
// Used to detect the loss of the autorun irp.  The driver prints out a message
// (debug level 0) if this timeout ever occurs
//
#define MEDIA_CHANGE_TIMEOUT_TIME  300

#endif

//
// Define the various states that media can be in for autorun.
//

typedef enum _MEDIA_CHANGE_DETECTION_STATE {
    MediaUnknown,
    MediaPresent,
    MediaNotPresent
} MEDIA_CHANGE_DETECTION_STATE, *PMEDIA_CHANGE_DETECTION_STATE;

struct _MEDIA_CHANGE_DETECTION_INFO;
typedef struct _MEDIA_CHANGE_DETECTION_INFO
    MEDIA_CHANGE_DETECTION_INFO, *PMEDIA_CHANGE_DETECTION_INFO;

//
// Structures for maintaining a dictionary list (list of objects
// referenced by a key value)
//

struct _DICTIONARY_HEADER;
typedef struct _DICTIONARY_HEADER DICTIONARY_HEADER, *PDICTIONARY_HEADER;

typedef struct _DICTIONARY {
    ULONGLONG Signature;
    PDICTIONARY_HEADER List;
    KSPIN_LOCK SpinLock;
} DICTIONARY, *PDICTIONARY;



#ifdef ALLOCATE_SRB_FROM_POOL

#define ClasspAllocateSrb(ext)
    ExAllocatePoolWithTag(NonPagedPool,                 \
                          sizeof(SCSI_REQUEST_BLOCK),   \
                          'sBRS')

#define ClasspFreeSrb(ext, srb)     ExFreePool((srb));

#else

#define ClasspAllocateSrb(ext)                      \
    ExAllocateFromNPagedLookasideList(              \
        &((ext)->CommonExtension.SrbLookasideList))

#define ClasspFreeSrb(ext, srb)                     \
    ExFreeToNPagedLookasideList(                    \
        &((ext)->CommonExtension.SrbLookasideList), \
        (srb))

#endif


typedef
VOID
(*PCLASS_ERROR) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN OUT NTSTATUS *Status,
    IN OUT BOOLEAN *Retry
    );

typedef
NTSTATUS
(*PCLASS_ADD_DEVICE) (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    );

typedef
NTSTATUS
(*PCLASS_POWER_DEVICE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

typedef
NTSTATUS
(*PCLASS_START_DEVICE) (
    IN PDEVICE_OBJECT DeviceObject
    );

typedef
NTSTATUS
(*PCLASS_STOP_DEVICE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    );

typedef
NTSTATUS
(*PCLASS_INIT_DEVICE) (
    IN PDEVICE_OBJECT DeviceObject
    );

typedef
NTSTATUS
(*PCLASS_ENUM_DEVICE) (
    IN PDEVICE_OBJECT DeviceObject
    );

typedef
NTSTATUS
(*PCLASS_READ_WRITE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

typedef
NTSTATUS
(*PCLASS_DEVICE_CONTROL) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

typedef
NTSTATUS
(*PCLASS_SHUTDOWN_FLUSH) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

typedef
NTSTATUS
(*PCLASS_CREATE_CLOSE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

typedef
NTSTATUS
(*PCLASS_QUERY_ID) (
    IN PDEVICE_OBJECT DeviceObject,
    IN BUS_QUERY_ID_TYPE IdType,
    IN PUNICODE_STRING IdString
    );

typedef
NTSTATUS
(*PCLASS_REMOVE_DEVICE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    );

typedef
VOID
(*PCLASS_UNLOAD) (
    IN PDRIVER_OBJECT DriverObject
    );

typedef
NTSTATUS
(*PCLASS_QUERY_PNP_CAPABILITIES) (
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PDEVICE_CAPABILITIES Capabilities
    );

typedef
VOID
(*PCLASS_TICK) (
    IN PDEVICE_OBJECT DeviceObject
    );

typedef
NTSTATUS
(*PCLASS_QUERY_WMI_REGINFO) (
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING Name
    );
/*++

Routine Description:

    This routine is a callback into the driver to retrieve information about
    the guids being registered.

Arguments:

    DeviceObject is the device whose registration information is needed

    *RegFlags returns with a set of flags that describe the guids being
        registered for this device. If the device wants enable and disable
        collection callbacks before receiving queries for the registered
        guids then it should return the WMIREG_FLAG_EXPENSIVE flag. Also the
        returned flags may specify WMIREG_FLAG_INSTANCE_PDO in which case
        the instance name is determined from the PDO associated with the
        device object. Note that the PDO must have an associated devnode. If
        WMIREG_FLAG_INSTANCE_PDO is not set then Name must return a unique
        name for the device.

    Name returns with the instance name for the guids if
        WMIREG_FLAG_INSTANCE_PDO is not set in the returned *RegFlags. The
        caller will call ExFreePool with the buffer returned.


Return Value:

    status

--*/

typedef
NTSTATUS
(*PCLASS_QUERY_WMI_DATABLOCK) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );
/*++

Routine Description:

    This routine is a callback into the driver to query for the contents of
    a data block. When the driver has finished filling the data block it
    must call ClassWmiCompleteRequest to complete the irp. The driver can
    return STATUS_PENDING if the irp cannot be completed immediately.

Arguments:

    DeviceObject is the device whose data block is being queried

    Irp is the Irp that makes this request

    GuidIndex is the index into the list of guids provided when the
        device registered

    BufferAvail on has the maximum size available to write the data
        block.

    Buffer on return is filled with the returned data block


Return Value:

    status

--*/

typedef
NTSTATUS
(*PCLASS_SET_WMI_DATABLOCK) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );
/*++

Routine Description:

    This routine is a callback into the driver to query for the contents of
    a data block. When the driver has finished filling the data block it
    must call ClassWmiCompleteRequest to complete the irp. The driver can
    return STATUS_PENDING if the irp cannot be completed immediately.

Arguments:

    DeviceObject is the device whose data block is being queried

    Irp is the Irp that makes this request

    GuidIndex is the index into the list of guids provided when the
        device registered

    BufferSize has the size of the data block passed

    Buffer has the new values for the data block


Return Value:

    status

--*/

typedef
NTSTATUS
(*PCLASS_SET_WMI_DATAITEM) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );
/*++

Routine Description:

    This routine is a callback into the driver to query for the contents of
    a data block. When the driver has finished filling the data block it
    must call ClassWmiCompleteRequest to complete the irp. The driver can
    return STATUS_PENDING if the irp cannot be completed immediately.

Arguments:

    DeviceObject is the device whose data block is being queried

    Irp is the Irp that makes this request

    GuidIndex is the index into the list of guids provided when the
        device registered

    DataItemId has the id of the data item being set

    BufferSize has the size of the data item passed

    Buffer has the new values for the data item


Return Value:

    status

--*/

typedef
NTSTATUS
(*PCLASS_EXECUTE_WMI_METHOD) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    );
/*++

Routine Description:

    This routine is a callback into the driver to execute a method. When the
    driver has finished filling the data block it must call
    ClassWmiCompleteRequest to complete the irp. The driver can
    return STATUS_PENDING if the irp cannot be completed immediately.

Arguments:

    DeviceObject is the device whose data block is being queried

    Irp is the Irp that makes this request

    GuidIndex is the index into the list of guids provided when the
        device registered

    MethodId has the id of the method being called

    InBufferSize has the size of the data block passed in as the input to
        the method.

    OutBufferSize on entry has the maximum size available to write the
        returned data block.

    Buffer is filled with the returned data block


Return Value:

    status

--*/

typedef enum
{
    EventGeneration,
    DataBlockCollection
} CLASSENABLEDISABLEFUNCTION;
typedef
NTSTATUS
(*PCLASS_WMI_FUNCTION_CONTROL) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN CLASSENABLEDISABLEFUNCTION Function,
    IN BOOLEAN Enable
    );
/*++

Routine Description:

    This routine is a callback into the driver to enabled or disable event
    generation or data block collection. A device should only expect a
    single enable when the first event or data consumer enables events or
    data collection and a single disable when the last event or data
    consumer disables events or data collection. Data blocks will only
    receive collection enable/disable if they were registered as requiring
    it.

Arguments:

    DeviceObject is the device whose data block is being queried

    GuidIndex is the index into the list of guids provided when the
        device registered

    Function specifies which functionality is being enabled or disabled

    Enable is TRUE then the function is being enabled else disabled

Return Value:

    status

--*/

typedef struct
{
    GUID Guid;               // Guid to registered
    ULONG InstanceCount;     // Count of Instances of Datablock
    ULONG Flags;             // Additional flags (see WMIREGINFO in wmistr.h)
} GUIDREGINFO, *PGUIDREGINFO;


typedef struct _CLASS_WMI_INFO {
    ULONG GuidCount;
    PGUIDREGINFO GuidRegInfo;

    PCLASS_QUERY_WMI_REGINFO      ClassQueryWmiRegInfo;
    PCLASS_QUERY_WMI_DATABLOCK    ClassQueryWmiDataBlock;
    PCLASS_SET_WMI_DATABLOCK      ClassSetWmiDataBlock;
    PCLASS_SET_WMI_DATAITEM       ClassSetWmiDataItem;
    PCLASS_EXECUTE_WMI_METHOD     ClassExecuteWmiMethod;
    PCLASS_WMI_FUNCTION_CONTROL   ClassWmiFunctionControl;
} CLASS_WMI_INFO, *PCLASS_WMI_INFO;


typedef struct _CLASS_DEV_INFO {

    //
    // Bytes needed by the class driver
    // for it's extension.
    // If this is zero, the driver does not expect to have any PDO's
    //

    ULONG DeviceExtensionSize;

    DEVICE_TYPE DeviceType;

    UCHAR StackSize;

    //
    // Device Characteristics flags
    //  eg.:
    //
    //  FILE_REMOVABLE_MEDIA
    //  FILE_READ_ONLY_DEVICE
    //  FILE_FLOPPY_DISKETTE
    //  FILE_WRITE_ONCE_MEDIA
    //  FILE_REMOTE_DEVICE
    //  FILE_DEVICE_IS_MOUNTED
    //  FILE_VIRTUAL_VOLUME
    //

    ULONG DeviceCharacteristics;

    PCLASS_ERROR                    ClassError;
    PCLASS_READ_WRITE               ClassReadWriteVerification;
    PCLASS_DEVICE_CONTROL           ClassDeviceControl;
    PCLASS_SHUTDOWN_FLUSH           ClassShutdownFlush;
    PCLASS_CREATE_CLOSE             ClassCreateClose;

    PCLASS_INIT_DEVICE              ClassInitDevice;
    PCLASS_START_DEVICE             ClassStartDevice;
    PCLASS_POWER_DEVICE             ClassPowerDevice;
    PCLASS_STOP_DEVICE              ClassStopDevice;
    PCLASS_REMOVE_DEVICE            ClassRemoveDevice;

    PCLASS_QUERY_PNP_CAPABILITIES   ClassQueryPnpCapabilities;

    //
    // Registered Data Block info for wmi
    //
    CLASS_WMI_INFO                  ClassWmiInfo;

} CLASS_DEV_INFO, *PCLASS_DEV_INFO;

struct _CLASS_INIT_DATA {

    //
    // This structure size - version checking.
    //

    ULONG InitializationDataSize;

    //
    // Specific init data for functional and physical device objects.
    //

    CLASS_DEV_INFO FdoData;
    CLASS_DEV_INFO PdoData;

    //
    // Device-specific driver routines
    //

    PCLASS_ADD_DEVICE             ClassAddDevice;
    PCLASS_ENUM_DEVICE            ClassEnumerateDevice;

    PCLASS_QUERY_ID               ClassQueryId;

    PDRIVER_STARTIO               ClassStartIo;
    PCLASS_UNLOAD                 ClassUnload;

    PCLASS_TICK                   ClassTick;
};

typedef struct _FILE_OBJECT_EXTENSION {
    PFILE_OBJECT FileObject;
    PDEVICE_OBJECT DeviceObject;
    ULONG LockCount;
    ULONG McnDisableCount;
} FILE_OBJECT_EXTENSION, *PFILE_OBJECT_EXTENSION;

typedef struct _CLASS_DRIVER_EXTENSION {

    UNICODE_STRING RegistryPath;

    CLASS_INIT_DATA InitData;

    ULONG DeviceCount;

} CLASS_DRIVER_EXTENSION, *PCLASS_DRIVER_EXTENSION;

typedef struct _COMMON_DEVICE_EXTENSION COMMON_DEVICE_EXTENSION, *PCOMMON_DEVICE_EXTENSION;
typedef struct _FUNCTIONAL_DEVICE_EXTENSION FUNCTIONAL_DEVICE_EXTENSION, *PFUNCTIONAL_DEVICE_EXTENSION;
typedef struct _PHYSICAL_DEVICE_EXTENSION PHYSICAL_DEVICE_EXTENSION, *PPHYSICAL_DEVICE_EXTENSION;

typedef struct _COMMON_DEVICE_EXTENSION {

    //
    // Version control field
    //
    // Note - this MUST be the first thing in the device extension
    // for any class driver using classpnp or a later version.
    //

    ULONG Version;

    //
    // Back pointer to device object
    //
    // NOTE - this MUST be the second field in the common device extension.
    // Users of this structure will include it in a union with the DeviceObject
    // pointer so they can reference this with a bit of syntactic sugar.
    //

    PDEVICE_OBJECT DeviceObject;

    //
    // Pointer to lower device object - send all requests through this
    //

    PDEVICE_OBJECT LowerDeviceObject;

    //
    // Pointer to the partition zero device extension.
    // There are several flags stored there that pdo
    // routines need to access
    //

    PFUNCTIONAL_DEVICE_EXTENSION PartitionZeroExtension;

    //
    // Pointer to the initialization data for this driver.  This is more
    // efficient than constantly getting the driver extension.
    //

    PCLASS_DRIVER_EXTENSION DriverExtension;

    //
    // INTERLOCKED counter of the number of requests/function calls outstanding
    // which will need to use this device object.  When this count goes to
    // zero the RemoveEvent will be set.
    //
    // This variable is only manipulated by ClassIncrementRemoveLock and
    // ClassDecrementRemoveLock.
    //

    LONG RemoveLock;

    //
    // This event will be signalled when it is safe to remove the device object
    //

    KEVENT RemoveEvent;

    //
    // The spinlock and the list are only used in checked builds to track
    // who has acquired the remove lock.  Free systems will leave these
    // initialized to ff
    //

    KSPIN_LOCK RemoveTrackingSpinlock;

    PVOID RemoveTrackingList;

    LONG RemoveTrackingUntrackedCount;

    //
    // Pointer to the driver specific data area
    //

    PVOID DriverData;

    //
    // Flag indicates whether this device object is
    // an FDO or a PDO
    //

    struct {
        BOOLEAN IsFdo : 1;
        BOOLEAN IsInitialized : 1;

        //
        // Flag indicating whether the lookaside listhead for srbs has been
        // initialized.
        //

        BOOLEAN IsSrbLookasideListInitialized : 1;
    };

    //
    // Contains the IRP_MN_CODE of the last state-changing pnp irps we
    // recieved (XXX_STOP, XXX_REMOVE, START, etc...).  Used in concert
    // with IsRemoved.
    //

    UCHAR PreviousState;
    UCHAR CurrentState;

    //
    // interlocked flag indicating that the device has been removed.
    //

    ULONG IsRemoved;

    //
    // The name of the object
    //
    UNICODE_STRING DeviceName;

    //
    // The next child device (or if this is an FDO, the first child device).
    //

    PPHYSICAL_DEVICE_EXTENSION ChildList;

    //
    // Number of the partition or -1L if not partitionable.
    //

    ULONG PartitionNumber;

    //
    // Length of partition in bytes
    //

    LARGE_INTEGER PartitionLength;

    //
    // Number of bytes before start of partition
    //

    LARGE_INTEGER StartingOffset;

    //
    // Dev-Info structure for this type of device object
    // Contains call-out routines for the class driver.
    //

    PCLASS_DEV_INFO DevInfo;

    //
    // Count of page files going through this device object
    // and event to synchronize them with.
    //

    ULONG PagingPathCount;
    ULONG DumpPathCount;
    ULONG HibernationPathCount;
    KEVENT PathCountEvent;

#ifndef ALLOCATE_SRB_FROM_POOL
    //
    // Lookaside listhead for srbs.
    //

    NPAGED_LOOKASIDE_LIST SrbLookasideList;
#endif

    //
    // Interface name string returned by IoRegisterDeviceInterface.
    //

    UNICODE_STRING MountedDeviceInterfaceName;


    //
    // Registered Data Block info for wmi
    //
    ULONG GuidCount;
    PGUIDREGINFO GuidRegInfo;

    //
    // File object dictionary for this device object.  Extensions are stored
    // in here rather than off the actual file object.
    //

    DICTIONARY FileObjectDictionary;

    //
    // The following will be in the released product as reserved.
    // Leave these at the end of the structure.
    //

    ULONG_PTR Reserved1;
    ULONG_PTR Reserved2;
    ULONG_PTR Reserved3;
    ULONG_PTR Reserved4;

} COMMON_DEVICE_EXTENSION, *PCOMMON_DEVICE_EXTENSION;

typedef enum {
    FailurePredictionNone = 0,     // No failure detection polling needed
    FailurePredictionIoctl,        // Do failure detection via IOCTL
    FailurePredictionSmart,        // Do failure detection via SMART
    FailurePredictionSense         // Do failure detection via sense data
} FAILURE_PREDICTION_METHOD, *PFAILURE_PREDICTION_METHOD;

//
// Default failure prediction polling interval is every hour
//

#define DEFAULT_FAILURE_PREDICTION_PERIOD 60 * 60 * 1

//
// The failure prediction structure is internal to classpnp - drivers do not
// need to know what it contains.
//

struct _FAILURE_PREDICTION_INFO;
typedef struct _FAILURE_PREDICTION_INFO *PFAILURE_PREDICTION_INFO;

//
// this is to allow for common code to handle
// every option.
//

typedef struct _CLASS_POWER_OPTIONS {
    ULONG PowerDown              :  1;
    ULONG LockQueue              :  1;
    ULONG HandleSpinDown         :  1;
    ULONG HandleSpinUp           :  1;
    ULONG Reserved               : 27;
} CLASS_POWER_OPTIONS, *PCLASS_POWER_OPTIONS;

typedef enum {
    PowerDownDeviceInitial,
    PowerDownDeviceLocked,
    PowerDownDeviceStopped,
    PowerDownDeviceOff,
    PowerDownDeviceUnlocked
} CLASS_POWER_DOWN_STATE;

typedef enum {
    PowerUpDeviceInitial,
    PowerUpDeviceLocked,
    PowerUpDeviceOn,
    PowerUpDeviceStarted,
    PowerUpDeviceUnlocked
} CLASS_POWER_UP_STATE;

typedef struct _CLASS_POWER_CONTEXT {

    union {
        CLASS_POWER_DOWN_STATE PowerDown;
        CLASS_POWER_UP_STATE PowerUp;
    } PowerChangeState;

    CLASS_POWER_OPTIONS Options;

    BOOLEAN InUse;
    BOOLEAN QueueLocked;

    NTSTATUS FinalStatus;

    ULONG RetryCount;
    ULONG RetryInterval;

    PIO_COMPLETION_ROUTINE CompletionRoutine;
    PDEVICE_OBJECT DeviceObject;
    PIRP Irp;

    SCSI_REQUEST_BLOCK Srb;

} CLASS_POWER_CONTEXT, *PCLASS_POWER_CONTEXT;


typedef struct _FUNCTIONAL_DEVICE_EXTENSION {

    //
    // Common device extension header
    //

    union {
        struct {
            ULONG Version;
            PDEVICE_OBJECT DeviceObject;
        };
        COMMON_DEVICE_EXTENSION CommonExtension;
    };

    //
    // Pointer to the physical device object we attached to - use this
    // for Pnp calls which need a PDO
    //

    PDEVICE_OBJECT LowerPdo;

    //
    // Device capabilities
    //

    PSTORAGE_DEVICE_DESCRIPTOR DeviceDescriptor;

    //
    // SCSI port driver capabilities
    //

    PSTORAGE_ADAPTER_DESCRIPTOR AdapterDescriptor;

    //
    // Current Power state of the device
    //

    DEVICE_POWER_STATE DevicePowerState;

    //
    // Bytes to skew all requests, since DM Driver has been placed on an IDE drive.
    //

    ULONG DMByteSkew;

    //
    // Sectors to skew all requests.
    //

    ULONG DMSkew;

    //
    // Flag to indicate whether DM driver has been located on an IDE drive.
    //

    BOOLEAN DMActive;

    //
    // Buffer for drive parameters returned in IO device control.
    //

    DISK_GEOMETRY DiskGeometry;

    //
    // Request Sense Buffer
    //

    PSENSE_DATA SenseData;

    //
    // Request timeout in seconds;
    //

    ULONG TimeOutValue;

    //
    // System device number
    //

    ULONG DeviceNumber;

    //
    // Add default Srb Flags.
    //

    ULONG SrbFlags;

    //
    // Total number of SCSI protocol errors on the device.
    //

    ULONG ErrorCount;

    //
    // Lock count for removable media.
    //

    LONG LockCount;
    LONG ProtectedLockCount;
    LONG InternalLockCount;

    KEVENT EjectSynchronizationEvent;

    //
    // Values for the flags are below.
    //

    USHORT  DeviceFlags;

    //
    // Log2 of sector size
    //

    UCHAR SectorShift;

    UCHAR ReservedByte;

    //
    // Indicates that the necessary data structures for media change
    // detection have been initialized.
    //

    PMEDIA_CHANGE_DETECTION_INFO MediaChangeDetectionInfo;

    PKEVENT Unused1;
    HANDLE  Unused2;

    //
    // File system context. Used for kernel-mode requests to disable autorun.
    //

    FILE_OBJECT_EXTENSION KernelModeMcnContext;

    //
    // Count of media changes.  This field is only valid for the root partition
    // (ie. if PhysicalDevice == NULL).
    //

    ULONG MediaChangeCount;

    //
    // Storage for a handle to the directory the PDO's are placed in
    //

    HANDLE DeviceDirectory;

    //
    // Storage for a release queue request.
    //

    KSPIN_LOCK ReleaseQueueSpinLock;

    PIRP ReleaseQueueIrp;

    SCSI_REQUEST_BLOCK ReleaseQueueSrb;

    BOOLEAN ReleaseQueueNeeded;

    BOOLEAN ReleaseQueueInProgress;

    BOOLEAN ReleaseQueueIrpFromPool;
    //
    // Failure detection storage
    //

    BOOLEAN FailurePredicted;

    ULONG FailureReason;
    PFAILURE_PREDICTION_INFO FailurePredictionInfo;

    BOOLEAN PowerDownInProgress;

    //
    // Interlock for ensuring we don't recurse during enumeration.
    //

    ULONG EnumerationInterlock;

    //
    // Synchronization object for manipulating the child list.
    //

    KEVENT ChildLock;

    //
    // The thread which currently owns the ChildLock.  This is used to
    // avoid recursive acquisition.
    //

    PKTHREAD ChildLockOwner;

    //
    // The number of times this event has been acquired.
    //

    ULONG ChildLockAcquisitionCount;

    //
    // Flags for special behaviour required by
    // different hardware, such as never spinning down
    // or disabling advanced features such as write cache
    //

    ULONG ScanForSpecialFlags;

    //
    // For delayed retry of power requests at DPC level
    //

    KDPC PowerRetryDpc;
    KTIMER PowerRetryTimer;

    //
    // Context structure for power operations.  Since we can only have
    // one D irp at any time in the stack we don't need to worry about
    // allocating multiple of these structures.
    //

    CLASS_POWER_CONTEXT PowerContext;

    //
    // Indicates the number of successfully completed
    // requests, if error throttling has been applied.
    //
    ULONG CompletionSuccessCount;

    //
    // When too many errors occur and features are turned off
    // the old SrbFlags are saved here, so that if the condition 
    // is fixed, we can restore them to their proper state.
    //
    ULONG SavedSrbFlags;

    //
    // Once recovery has been initiated, cache the old error count value.
    // If new errors occur, go back to the feature set as was earlier used.
    //
    ULONG SavedErrorCount;

    //
    // For future expandability
    // leave these at the end of the structure.
    //

    ULONG_PTR Reserved1;

} FUNCTIONAL_DEVICE_EXTENSION, *PFUNCTIONAL_DEVICE_EXTENSION;

// Never Spin Up/Down the drive (may not handle properly)
#define CLASS_SPECIAL_DISABLE_SPIN_DOWN             0x00000001
#define CLASS_SPECIAL_DISABLE_SPIN_UP               0x00000002

// Don't bother to lock the queue when powering down
// (used mostly to send a quick stop to a cdrom to abort audio playback)
#define CLASS_SPECIAL_NO_QUEUE_LOCK                 0x00000008

// Disable write cache due to known bugs
#define CLASS_SPECIAL_DISABLE_WRITE_CACHE           0x00000010

//
// Special interpretation of "device not ready / cause not reportable" for
// devices which don't tell us they need to be spun up manually after they
// spin themselves down behind our back.
//
// The down side of this is that if the drive chooses to report
// "device not ready / cause not reportable" to mean "no media in device"
// or any other error which really does require user intervention NT will
// waste a large amount of time trying to spin up a disk which can't be spun
// up.
//

#define CLASS_SPECIAL_CAUSE_NOT_REPORTABLE_HACK     0x00000020

#define CLASS_SPECIAL_MODIFY_CACHE_UNSUCCESSFUL     0x00000040
#define CLASS_SPECIAL_FUA_NOT_SUPPORTED             0x00000080

#define CLASS_SPECIAL_VALID_MASK                    0x000000FB
#define CLASS_SPECIAL_RESERVED       (~CLASS_SPECIAL_VALID_MASK)


typedef struct _PHYSICAL_DEVICE_EXTENSION {

    //
    // Common extension data
    //

    union {
        struct {
            ULONG Version;
            PDEVICE_OBJECT DeviceObject;
        };
        COMMON_DEVICE_EXTENSION CommonExtension;
    };

    //
    // Indicates that the pdo no longer physically exits.
    //

    BOOLEAN IsMissing;

    //
    // Indicates that the PDO has been handed out to the PNP system.
    //

    BOOLEAN IsEnumerated;

    //
    // for future expandability
    // leave these at the end of the structure.
    //

    ULONG_PTR Reserved1;
    ULONG_PTR Reserved2;
    ULONG_PTR Reserved3;
    ULONG_PTR Reserved4;

} PHYSICAL_DEVICE_EXTENSION, *PPHYSICAL_DEVICE_EXTENSION;

//
// Indicates that the device has write caching enabled.
//

#define DEV_WRITE_CACHE     0x00000001


//
// Build SCSI 1 or SCSI 2 CDBs
//

#define DEV_USE_SCSI1       0x00000002

//
// Indicates whether is is safe to send StartUnit commands
// to this device. It will only be off for some removeable devices.
//

#define DEV_SAFE_START_UNIT 0x00000004

//
// Indicates whether it is unsafe to send SCSIOP_MECHANISM_STATUS commands to
// this device.  Some devices don't like these 12 byte commands
//

#define DEV_NO_12BYTE_CDB 0x00000008

//
// Indicates that the device is connected to a backup power supply
// and hence write-through and synch cache requests may be ignored
//

#define DEV_POWER_PROTECTED 0x00000010

//
// Define context structure for asynchronous completions.
//

typedef struct _COMPLETION_CONTEXT {
    PDEVICE_OBJECT DeviceObject;
    SCSI_REQUEST_BLOCK Srb;
}COMPLETION_CONTEXT, *PCOMPLETION_CONTEXT;

//
// Class dll routines called by class drivers
//

SCSIPORT_API
ULONG
ClassInitialize(
    IN  PVOID            Argument1,
    IN  PVOID            Argument2,
    IN  PCLASS_INIT_DATA InitializationData
    );

SCSIPORT_API
NTSTATUS
ClassCreateDeviceObject(
    IN PDRIVER_OBJECT          DriverObject,
    IN PCCHAR                  ObjectNameBuffer,
    IN PDEVICE_OBJECT          LowerDeviceObject,
    IN BOOLEAN                 IsFdo,
    IN OUT PDEVICE_OBJECT      *DeviceObject
    );

SCSIPORT_API
ULONG
ClassFindUnclaimedDevices(
    IN PCLASS_INIT_DATA InitializationData,
    IN PSCSI_ADAPTER_BUS_INFO  AdapterInformation
    );

SCSIPORT_API
NTSTATUS
ClassReadDriveCapacity(
    IN PDEVICE_OBJECT DeviceObject
    );

SCSIPORT_API
VOID
ClassReleaseQueue(
    IN PDEVICE_OBJECT DeviceObject
    );

SCSIPORT_API
NTSTATUS
ClassAsynchronousCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    );

SCSIPORT_API
VOID
ClassSplitRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG MaximumBytes
    );

SCSIPORT_API
NTSTATUS
ClassDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

SCSIPORT_API
NTSTATUS
ClassIoComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

SCSIPORT_API
NTSTATUS
ClassCheckVerifyComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

SCSIPORT_API
NTSTATUS
ClassIoCompleteAssociated(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

SCSIPORT_API
BOOLEAN
ClassInterpretSenseInfo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN UCHAR MajorFunctionCode,
    IN ULONG IoDeviceCode,
    IN ULONG RetryCount,
    OUT NTSTATUS *Status,
    OUT ULONG *RetryInterval
    );

VOID
ClassSendDeviceIoControlSynchronous(
    IN ULONG IoControlCode,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength,
    IN BOOLEAN InternalDeviceIoControl,
    OUT PIO_STATUS_BLOCK IoStatus
    );

SCSIPORT_API
NTSTATUS
ClassSendIrpSynchronous(
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PIRP Irp
    );

SCSIPORT_API
NTSTATUS
ClassForwardIrpSynchronous(
    IN PCOMMON_DEVICE_EXTENSION Device,
    IN PIRP Irp
    );

SCSIPORT_API
NTSTATUS
ClassSendSrbSynchronous(
        PDEVICE_OBJECT DeviceObject,
        PSCSI_REQUEST_BLOCK Srb,
        PVOID BufferAddress,
        ULONG BufferLength,
        BOOLEAN WriteToDevice
        );

SCSIPORT_API
NTSTATUS
ClassSendSrbAsynchronous(
        PDEVICE_OBJECT DeviceObject,
        PSCSI_REQUEST_BLOCK Srb,
        PIRP Irp,
        PVOID BufferAddress,
        ULONG BufferLength,
        BOOLEAN WriteToDevice
        );

SCSIPORT_API
NTSTATUS
ClassBuildRequest(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

SCSIPORT_API
ULONG
ClassModeSense(
    IN PDEVICE_OBJECT DeviceObject,
    IN PCHAR ModeSenseBuffer,
    IN ULONG Length,
    IN UCHAR PageMode
    );

SCSIPORT_API
BOOLEAN
ClassModeSelect(
    IN PDEVICE_OBJECT DeviceObject,
    IN PCHAR ModeSelectBuffer,
    IN ULONG Length,
    IN BOOLEAN SavePage
    );

SCSIPORT_API
PVOID
ClassFindModePage(
    IN PCHAR ModeSenseBuffer,
    IN ULONG Length,
    IN UCHAR PageMode,
    IN BOOLEAN Use6Byte
    );

SCSIPORT_API
NTSTATUS
ClassClaimDevice(
    IN PDEVICE_OBJECT LowerDeviceObject,
    IN BOOLEAN Release
    );

SCSIPORT_API
NTSTATUS
ClassInternalIoControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

SCSIPORT_API
VOID
ClassInitializeSrbLookasideList(
    IN PCOMMON_DEVICE_EXTENSION DeviceExtension,
    IN ULONG NumberElements
    );

SCSIPORT_API
VOID
ClassDeleteSrbLookasideList(
    IN PCOMMON_DEVICE_EXTENSION CommonExtension
    );

SCSIPORT_API
ULONG
ClassQueryTimeOutRegistryValue(
    IN PDEVICE_OBJECT DeviceObject
    );

SCSIPORT_API
NTSTATUS
ClassGetDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSTORAGE_PROPERTY_ID PropertyId,
    OUT PVOID *Descriptor
    );

SCSIPORT_API
VOID
ClassInvalidateBusRelations(
    IN PDEVICE_OBJECT Fdo
    );

SCSIPORT_API
VOID
ClassMarkChildrenMissing(
    IN PFUNCTIONAL_DEVICE_EXTENSION Fdo
    );

SCSIPORT_API
BOOLEAN
ClassMarkChildMissing(
    IN PPHYSICAL_DEVICE_EXTENSION PdoExtension,
    IN BOOLEAN AcquireChildLock
    );

SCSIPORT_API
VOID
ClassDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    );

SCSIPORT_API
PCLASS_DRIVER_EXTENSION
ClassGetDriverExtension(
    IN PDRIVER_OBJECT DriverObject
    );

SCSIPORT_API
VOID
ClassCompleteRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN CCHAR PriorityBoost
    );

SCSIPORT_API
VOID
ClassReleaseRemoveLock(
    IN PDEVICE_OBJECT DeviceObject,
    IN OPTIONAL PIRP Irp
    );

SCSIPORT_API
ULONG
ClassAcquireRemoveLockEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN OPTIONAL PVOID Tag,
    IN PCSTR File,
    IN ULONG Line
    );

SCSIPORT_API
NTSTATUS
ClassRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR RemoveType
    );

SCSIPORT_API
VOID
ClassUpdateInformationInRegistry(
    IN PDEVICE_OBJECT     Fdo,
    IN PCHAR              DeviceName,
    IN ULONG              DeviceNumber,
    IN PINQUIRYDATA       InquiryData,
    IN ULONG              InquiryDataLength
    );

SCSIPORT_API
NTSTATUS
ClassWmiCompleteRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN NTSTATUS Status,
    IN ULONG BufferUsed,
    IN CCHAR PriorityBoost
    );

SCSIPORT_API
NTSTATUS
ClassWmiFireEvent(
    IN PDEVICE_OBJECT DeviceObject,
    IN LPGUID Guid,
    IN ULONG InstanceIndex,
    IN ULONG EventDataSize,
    IN PVOID EventData
    );


SCSIPORT_API
VOID
ClassCheckMediaState(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );

SCSIPORT_API
VOID
ClassSetMediaChangeState(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN MEDIA_CHANGE_DETECTION_STATE State,
    IN BOOLEAN Wait
    );

SCSIPORT_API
VOID
ClassResetMediaChangeTimer(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );

SCSIPORT_API
VOID
ClassInitializeMediaChangeDetection(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PUCHAR EventPrefix
    );

SCSIPORT_API
NTSTATUS
ClassInitializeTestUnitPolling(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN BOOLEAN AllowDriveToSleep
    );

SCSIPORT_API
VOID
ClassDestroyMediaChangeDetection(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );

SCSIPORT_API
VOID
ClassEnableMediaChangeDetection(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );

SCSIPORT_API
VOID
ClassDisableMediaChangeDetection(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );

SCSIPORT_API
VOID
ClassCleanupMediaChangeDetection(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );

SCSIPORT_API
PVPB
ClassGetVpb(
    IN PDEVICE_OBJECT DeviceObject
    );

SCSIPORT_API
NTSTATUS
ClassCopyPdoName(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING Name
    );

SCSIPORT_API
NTSTATUS
ClassSpinDownPowerHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClassStopUnitPowerHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClassSetFailurePredictionPoll(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    FAILURE_PREDICTION_METHOD FailurePredictionMethod,
    ULONG PollingPeriod
    );

VOID
ClassNotifyFailurePredicted(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    PUCHAR Buffer,
    ULONG BufferSize,
    BOOLEAN LogError,
    ULONG UniqueErrorValue,
    UCHAR PathId,
    UCHAR TargetId,
    UCHAR Lun
    );

SCSIPORT_API
VOID
ClassAcquireChildLock(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );

SCSIPORT_API
VOID
ClassReleaseChildLock(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );

NTSTATUS
ClassSignalCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    );

VOID
ClassSendStartUnit(
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
ClassGetDeviceParameter(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PWSTR SubkeyName OPTIONAL,
    IN PWSTR ParameterName,
    IN OUT PULONG ParameterValue
    );

NTSTATUS
ClassSetDeviceParameter(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PWSTR SubkeyName OPTIONAL,
    IN PWSTR ParameterName,
    IN ULONG ParameterValue
    );

#endif /* _CLASS_ */

