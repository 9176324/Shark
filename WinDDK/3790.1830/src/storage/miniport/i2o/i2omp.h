/*++

Copyright (c) 1998-2002  Microsoft Corporation

Module Name:

    i2omp.h

Abstract:

    This is the main include file for the I2O miniport. It contains all storport miniport-
    specific declarations and defines, and includes the various support headers.

Author:

Environment:

    kernel mode only

Revision History:
 
--*/

#ifndef _I2OMP_H
#define _I2OMP_H

#include "storport.h"
#include "i2omsg.h"
#include "i2outil.h"
#include "i2omstor.h"
#include "i2ocontrol.h"
#include "i2obscsi.h"
#include "I2OExec.h"


#define INT_ASSERTED_OFFSET         0x30        // Interrupt Control Reg.
#define INT_CNTL_STAT_OFFSET        0x34        // Interrupt Control Reg.
#define PCI_INT_DISABLE             0x08
#define PCI_INT_PENDING             0x08

//
// This parameters are for PORT_CONFIGURATION_INFORMATION
// 
#define IOP_MAXIMUM_TARGETS        32
#define IOP_NUMBER_OF_BUSSES        1

//
// Currently Target ID 31 is reserved for a management device
// and cannot be used for an actual device.
//
#define IOP_MANAGEMENT_DEVICE_ID    31

#define MESSAGEFRAME_ALLOC          0x00000040  // Read Inbound Queue Port
#define MESSAGE_POST                0x00000040  // Write Inbound Queue Port
#define MESSAGE_REPLY               0x00000044  // Read Outbound Queue Port
#define MESSAGEFRAME_RELEASE        0x00000044  // Write Outbound Queue Port

//
// ScsiPortLogError driver unique codes
//
#define I2OMP_ERROR_INSUFFICIENT_RESOURCES  0x0001
#define I2OMP_ERROR_RESET_FAILED            0x0002
#define I2OMP_ERROR_STATUSGET_FAILED        0x0003
#define I2OMP_ERROR_INITOUTBOUND_FAILED     0x0004
#define I2OMP_ERROR_GETHRT_FAILED           0x0005
#define I2OMP_ERROR_SETSYSTAB_FAILED        0x0006
#define I2OMP_ERROR_ENABLESYS_FAILED        0x0007
#define I2OMP_ERROR_LCTSIZE_FAILED          0x0008
#define I2OMP_ERROR_LCTNOTIFY_FAILED        0x0009    
#define I2OMP_ERROR_CLAIM_FAILED            0x000A
#define I2OMP_ERROR_EVENTREGISTER_FAILED    0x000B
#define I2OMP_ERROR_ASSIGNEDID_FAILED       0x000C
#define I2OMP_ERROR_DUPLICATEMSG_FAILED     0x000D
#define I2OMP_ERROR_GETLCT_FAILED           0x000E
#define I2OMP_ERROR_GETLCTCALLBACK_FAILED   0x000F
#define I2OMP_ERROR_GETLCTTIMEOUT           0x0010
#define I2OMP_ERROR_LCTTOOLARGE             0x0011
#define I2OMP_ERROR_RESETTIMEOUT            0x0012
#define I2OMP_ERROR_DEVICERESET_TIMEOUT     0x0013

//
// Defines for private parameter groups and FieldIdx.
//
#define I2O_RAID_COMMON_CONFIG_GROUP_NO         0x8080
#define     I2O_GUI_MODE_FIELD_IDX              1
#define     I2O_TCL_FIELD_IDX                   21
#define I2O_RAID_DEVICE_CONFIG_GROUP_NO         0x8100
#define     I2O_MAP_STATE_FIELD_IDX             5
#define I2O_RAID_ISM_CONFIG_GROUP_NO            0x8800
#define     I2O_READY_FOR_SHUTDOWN_FIELD_IDX    20
#define     I2O_DELETE_VOLUME_SUPPORT_FIELD_IDX 24
#define I2O_RAID_ISM_VOLUME_TABLE               0x8900

#if 1
#if defined(_X86_)
#define PAGE_SIZE (ULONG)0x1000
#endif
#if (defined(_IA64_))
#define PAGE_SIZE (ULONG)0x2000
#endif
#if (defined(_AMD64_))
#define PAGE_SIZE (ULONG)0x1000
#endif
#endif

#if !defined(CALLBACK_OBJECT_SIG)
#define CALLBACK_OBJECT_SIG             'I2OC'

typedef struct _CALLBACK_OBJECT {
    ULONG       Signature;
    PVOID       CallbackAddress;
    PVOID       CallbackContext;
} CALLBACK_OBJECT, *PCALLBACK_OBJECT;

#endif  // CALLBACK_OBJECT

typedef struct _CALLBACK {
    VOID (*Function) (
    IN PVOID    MessageFrame,
    IN PVOID    CallbackContext
    );
} CALLBACK, *PCALLBACK;

typedef struct _IOP_CALLBACK_OBJECT {
   CALLBACK_OBJECT  Callback;    // Callback object for completions
   PVOID            Context;     // Context associated with completion
} IOP_CALLBACK_OBJECT, *PIOP_CALLBACK_OBJECT;


#define INITIATOR_CONTEXT_OFFSET (FIELD_OFFSET(I2O_MESSAGE_FRAME, InitiatorContext))

typedef union _POINTER64_TO_U32 {
   PVOID            Pointer;     // 32 or 64 bit pointer; depends on compiler
   ULONGLONG        Pointer64;   // Extended 64 bits of Pointer
   struct {
       U32          LowPart;     // Low 32 bits of Pointer
       U32          HighPart;    // High 32 bits of Pointer
   } u;
} POINTER64_TO_U32, *PPOINTER64_TO_U32;


typedef struct _I2O_PARAM_SCALAR_OPERATION {
    I2O_PARAM_OPERATIONS_LIST_HEADER      OpList;
    I2O_PARAM_OPERATION_SPECIFIC_TEMPLATE OpBlock;
} I2O_PARAM_SCALAR_OPERATION, *PI2O_PARAM_SCALAR_OPERATION;

#define SCRATCH_BUFFER_SIZE             (PAGE_SIZE * 2)
#define HRT_BUFFER_SIZE                 (PAGE_SIZE - 1024)
#define HRT_OFFSET                      (SCRATCH_BUFFER_SIZE - HRT_BUFFER_SIZE)

//
// Set the minimum expected LCT size to 1024 bytes
//
#define MIN_EXPECTED_LCT_SIZE           1024

// 
// Add IOP_ID_BASE_VALUE to IOPNumber to get the IOP_ID assigned to the IOP.
//
#define IOP_ID_BASE_VALUE       0x10

//
// The high order 4 bits of the VersionOffset field in the message header
// specifies the number of 32-bit words from the beginning of the message
// frame to the first SGL element of the message frame.
//

#define I2O_HRT_GET_SGL_VER_OFFSET    ((FIELD_OFFSET(I2O_EXEC_HRT_GET_MESSAGE,SGL)/4)       << 4)
#define I2O_OUT_INIT_SGL_VER_OFFSET   ((FIELD_OFFSET(I2O_EXEC_OUTBOUND_INIT_MESSAGE,SGL)/4) << 4)
#define I2O_SYS_MOD_SGL_VER_OFFSET    ((FIELD_OFFSET(I2O_EXEC_SYS_MODIFY_MESSAGE,SGL)/4)    << 4)
#define I2O_SYS_TAB_SGL_VER_OFFSET    ((FIELD_OFFSET(I2O_EXEC_SYS_TAB_SET_MESSAGE,SGL)/4)   << 4)

#define I2O_LCT_SGL_VER_OFFSET      ((FIELD_OFFSET(I2O_EXEC_LCT_NOTIFY_MESSAGE,SGL)/4)    << 4)
#define I2O_DOWNLOAD_SGL_VER_OFFSET ((FIELD_OFFSET(I2O_EXEC_SW_DOWNLOAD_MESSAGE,SGL)/4)   << 4)
#define I2O_UPLOAD_SGL_VER_OFFSET   ((FIELD_OFFSET(I2O_EXEC_SW_UPLOAD_MESSAGE,SGL)/4)     << 4)
#define I2O_CONFIG_SGL_VER_OFFSET   ((FIELD_OFFSET(I2O_UTIL_CONFIG_DIALOG_MESSAGE,SGL)/4) << 4)
#define I2O_PARAMS_SGL_VER_OFFSET   ((FIELD_OFFSET(I2O_UTIL_PARAMS_GET_MESSAGE,SGL)/4)    << 4)


#define I2O_READ_SGL_VER_OFFSET     ((FIELD_OFFSET(I2O_BSA_READ_MESSAGE,SGL)/4)         << 4)
#define I2O_WRITE_SGL_VER_OFFSET    ((FIELD_OFFSET(I2O_BSA_WRITE_MESSAGE,SGL)/4)        << 4)
#define I2O_VERIFY_SGL_VER_OFFSET   ((FIELD_OFFSET(I2O_BSA_WRITE_VERIFY_MESSAGE,SGL)/4) << 4)
#define I2O_PARAMS_SGL_VER_OFFSET   ((FIELD_OFFSET(I2O_UTIL_PARAMS_GET_MESSAGE,SGL)/4)  << 4)

#define I2O_SG_ELEMENT_OVERHEAD (sizeof(U32))

typedef union _READ_WRTE_MSG {
    I2O_BSA_READ_MESSAGE   Read;
    I2O_BSA_WRITE_MESSAGE  Write;
    ULONG                  Blk[128 / sizeof(ULONG)];
} READ_WRITE_MSG, *PREAD_WRITE_MSG;

typedef union _PASSTHRU_MSG {
    I2O_SCSI_SCB_EXECUTE_MESSAGE    m;
    ULONG                           blk[128/ sizeof(U32)];
} PASSTHRU_MSG, *PPASSTHRU_MSG;

#pragma pack(1)
typedef struct _I2O_VOL_ROW_INFO {
    UCHAR  VolumeName[16];
    USHORT VolumeTID;
    ULONG RaidLevel;
} I2O_VOL_ROW_INFO, *PI2O_VOL_ROW_INFO;
#pragma pack()

typedef struct _I2O_VOL_TABLE_INFO {
    ULONG ResultCount;

    //
    // Specifies the number of ULONGs in the
    // result block.
    // 
    ULONG BlockSize;

    //
    // Number of rows.
    //
    ULONG VolumesDefined;
    I2O_VOL_ROW_INFO RowInfo[1];
} I2O_VOL_TABLE_INFO, *PI2O_VOL_TABLE_INFO;

//
// This is used to describe each BSA-type device found.
//
typedef struct _I2O_DEVICE_INFO {
    ULONG LCTIndex;
    ULONG OutstandingIOCount;    
    ULONG MaxIOCount;    
    ULONG TotalRequestCount;

    //
    // Characteristics from the DEVICE_INFO scaler.
    //
    ULONG Characteristics;

    //
    // Basic READ_CAPACITY Info.
    //
    ULONG BlockSize;
    LARGE_INTEGER Capacity;

    ULONG DeviceState;
    USHORT Class;
    USHORT DeviceFlags;
    UCHAR  DeviceName[16];
    ULONG  RAIDLevel;
    I2O_LCT_ENTRY   Lct;
} I2O_DEVICE_INFO, *PI2O_DEVICE_INFO;

#define BSA_RESET_TIMER    1000000 // Microseconds (or 1 sec.).
#define ADAPTER_LCT_TIMEOUT      5 // Seconds.
#define BSA_RESET_PAUSE_TIMEOUT 10 // Seconds.

//
// Various bits to describe the state of a 
// particular device.
//
#define DFLAGS_NO_DEVICE        0x0000
#define DFLAGS_NEED_CLAIM       0x0001
#define DFLAGS_CLAIM_ISSUED     0x0002
#define DFLAGS_CLAIMED          0x0004
#define DFLAGS_OS_CAN_CLAIM     0x0008
#define DFLAGS_REMOVE_DEVICE    0x0010
#define DFLAGS_NEED_REGISTER    0x0020
#define DFLAGS_PAUSED           0x0040

//
// Device State defines for handling device state changes
//
#define DSTATE_NORMAL       0x0001
#define DSTATE_FAULTED      0x0002
#define DSTATE_NORECOVERY   0x0004
#define DSTATE_COMPLETE     0x0010


#define I2OMP_EVENT_MASK    (I2O_EVENT_IND_STATE_CHANGE | I2O_EVENT_IND_DEVICE_RESET)

//
// Various bits to describe the state of the adapter.
// 
#define AFLAGS_LOCKED         0x00000001
#define AFLAGS_SHUTDOWN       0x00000002
#define AFLAGS_IN_LDR_DUMP    0x00000004
#define AFLAGS_SHUTDOWN_RECVD 0x00000008
#define AFLAGS_NEED_VOL_INFO  0x00000010
#define AFLAGS_DEVICE_RESET   0x00000020
#define AFLAGS_RESUMED        0x00000040
#define AFLAGS_PAUSED         0x00000080

typedef struct _DEVICE_EXTENSION {

    //
    // Zero-based value of adapter count.
    //
    ULONG AdapterOrdinal;

    //
    // used during reset processing
    //
    ULONG ResetCnt;
    ULONG ResetTimer;
    ULONG Ticks;
    ULONG LinkDownTimer;

    //
    // DeviceID of the IOP 
    //
    ULONG DeviceId;

    //
    // Base address of the IOP memory.
    //
    ULONG_PTR PhysicalMemoryAddress;
    ULONG PhysicalMemorySize;

    //
    // Indicates number of devices found and a bitmap
    // of block storage devices.
    //
    ULONG DeviceMap;

    //
    // Vendor name, based on adapter id.
    //
    UCHAR VendorName[8];
    ULONG NumberVolumesDefined;
    I2O_VOL_ROW_INFO CachedVolInfo[IOP_MAXIMUM_TARGETS];
    
    //
    // Array of information concerning BSA and SCSI
    // devices found.
    //
    I2O_DEVICE_INFO DeviceInfo[IOP_MAXIMUM_TARGETS];
    ULONG NewListIndex;

    //
    // Values and buffer for request reply.
    //
    ULONG MessageFrameSize;
    ULONG ReplyBufferSize;
    ULONG NumberReplyBuffers;
    PUCHAR ReplyBuffer;
    STOR_PHYSICAL_ADDRESS ReplyBufferPA;

    //
    // Offset is used in making physical to virtual calculations.
    //
    ULONG_PTR OffsetAddress;

    //
    // Buffer used during init. for HRT, SysTab, etc.
    //
    PUCHAR ScratchBuffer;
    STOR_PHYSICAL_ADDRESS ScratchBufferPA;

    //
    // Pointer into the scratch buffer for the LCT.
    //
    PI2O_LCT Lct;
    STOR_PHYSICAL_ADDRESS LctPA;
    PI2O_LCT UpdateLct;
    STOR_PHYSICAL_ADDRESS UpdateLctPA;
    ULONG CurrentChangeIndicator;

    //
    // Address of mapped area and various control offsets into it.
    //
    PUCHAR MappedAddress;
    PULONG MessageAlloc;
    PULONG MessagePost;
    PULONG MessageReply;
    PULONG MessageRelease;
    PULONG IntControl;
    PULONG IntAsserted;

    //
    // Callback objects used during initialization. Once up and running
    // callback objects are in the srb extension.
    //
    IOP_CALLBACK_OBJECT InitCallback;
    ULONG CallBackComplete;
    IOP_CALLBACK_OBJECT LctCallback;
    IOP_CALLBACK_OBJECT AsyncClaimCallback;
    IOP_CALLBACK_OBJECT DeviceResetCallback;
    IOP_CALLBACK_OBJECT ClaimReleaseCallback;
    IOP_CALLBACK_OBJECT EventCallback;
    IOP_CALLBACK_OBJECT GetDeviceStateCallback;

    //
    // Miscellaneous configuration information about the IOP.
    // 
    ULONG ExpectedLCTSize;
    ULONG IOPCapabilities;
    ULONG IOPState;
    ULONG I2OVersion;
    ULONG MessengerType;
    ULONG MaxInboundMFrames;
    ULONG InitialInboundMFrames;
    ULONG InboundMFrameSize;

    //
    //  Various state bits of the adapter/miniport. See defines above.
    //
    ULONG AdapterFlags; 
    PSCSI_REQUEST_BLOCK DelayedSrb;
    ULONG IsmTID;

    //
    // A flag for actual shutdown vs PCI hot plug removal.  AFLAGS_SHUTDOWN is used 
    // for hot plug removal and not allowing StartIO if its set. 
    // If we get a real shutdown we still need to allow IO's. 
    //
    BOOLEAN TempShutdown;

    // 
    // Flags to support:
    // the cards that don't have the new shutdown interface, 
    // one to say we're shutdown
    // and one to say whether ot not the pause is needed on a post-shutdown IO since
    // its only needed on writes
    //
    BOOLEAN PostShutdownActivity;
    BOOLEAN SpecialShutdown;

    //
    // TCL boot drive paramter place holder for ACPI
    //
    ULONG TCLParam;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct _SRB_EXTENSION {
    //
    // Callback info for the request.
    //
    IOP_CALLBACK_OBJECT CallBack;

    //
    // Local storage for Read/Write I2O requests
    // that have been pre-built by I2OBuildIo.
    //
    union {
        READ_WRITE_MSG      RWMsg;
        PASSTHRU_MSG        PTMsg;
    }; 
} SRB_EXTENSION, *PSRB_EXTENSION;


#endif // _I2OMP_H

