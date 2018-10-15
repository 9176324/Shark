/*++

Copyright (c) 1998-2002  Microsoft Corporation

Module Name:

    i2omstor.h

Abstract:

    I2O Random Block Storage Devices Class Message defintion file that
    contains information presented in Chapter 6, Section 4 of the I2O Specification.

Author:

Environment:

    kernel mode only

Revision History:
 
--*/

#if !defined(I2O_MSTOR_HDR)
#define I2O_MSTOR_HDR

#include    "i2omsg.h"          /* Include the Base Message file */

#define I2OMSTOR_REV 1_5_1      /* Header file revision string */

PRAGMA_ALIGN_PUSH

#pragma pack(push,1)


//
//  Random Block Storage Class specific functions
//    
//  Although the names are block storage class specific, the values 
//  assigned are common with other classes when applicable.
//
#define     I2O_BSA_BLOCK_READ              0x30
#define     I2O_BSA_BLOCK_REASSIGN          0x71
#define     I2O_BSA_BLOCK_WRITE             0x31
#define     I2O_BSA_BLOCK_WRITE_VERIFY      0x33
#define     I2O_BSA_CACHE_FLUSH             0x37
#define     I2O_BSA_DEVICE_RESET            0x27
#define     I2O_BSA_MEDIA_EJECT             0x43
#define     I2O_BSA_MEDIA_FORMAT            0x45
#define     I2O_BSA_MEDIA_LOCK              0x49
#define     I2O_BSA_MEDIA_MOUNT             0x41
#define     I2O_BSA_MEDIA_UNLOCK            0x4B
#define     I2O_BSA_MEDIA_VERIFY            0x35
#define     I2O_BSA_POWER_MANAGEMENT        0x70
#define     I2O_BSA_STATUS_CHECK            0x25

//
// RAID Additions.
//
#define I2O_MANAGEMENT              0x81
#define I2O_DIAGNOSTICS             0x83
#define I2O_INSTRUMENTATION         0x85


//
// Detailed Status Codes for Random Block Storage operations
//
#define I2O_BSA_DSC_SUCCESS             0x0000
#define I2O_BSA_DSC_MEDIA_ERROR         0x0001
#define I2O_BSA_DSC_ACCESS_ERROR        0x0002
#define I2O_BSA_DSC_DEVICE_FAILURE      0x0003
#define I2O_BSA_DSC_DEVICE_NOT_READY    0x0004
#define I2O_BSA_DSC_MEDIA_NOT_PRESENT   0x0005
#define I2O_BSA_DSC_MEDIA_LOCKED        0x0006
#define I2O_BSA_DSC_MEDIA_FAILURE       0x0007
#define I2O_BSA_DSC_PROTOCOL_FAILURE    0x0008
#define I2O_BSA_DSC_BUS_FAILURE         0x0009
#define I2O_BSA_DSC_ACCESS_VIOLATION    0x000A
#define I2O_BSA_DSC_WRITE_PROTECTED     0x000B
#define I2O_BSA_DSC_DEVICE_RESET        0x000C
#define I2O_BSA_DSC_VOLUME_CHANGED      0x000D
#define I2O_BSA_DSC_TIMEOUT             0x000E


//
// Block Storage Parameter Groups
// 
// Block Storage Configuration and Operating Structures and Defines
// 
//
// Block Storage Parameter Groups
//
#define     I2O_BSA_DEVICE_INFO_GROUP_NO                0x0000
#define     I2O_BSA_OPERATIONAL_CONTROL_GROUP_NO        0x0001
#define     I2O_BSA_POWER_CONTROL_GROUP_NO              0x0002
#define     I2O_BSA_CACHE_CONTROL_GROUP_NO              0x0003
#define     I2O_BSA_MEDIA_INFO_GROUP_NO                 0x0004
#define     I2O_BSA_ERROR_LOG_GROUP_NO                  0x0005

//
// Block Storage Optional Historical Statistics Parameter Groups
//
#define     I2O_BSA_HISTORICAL_STATS_CONTROL_GROUP_NO   0x0180
#define     I2O_BSA_STORAGE_HISTORICAL_STATS_GROUP_NO   0x0181
#define     I2O_BSA_CACHE_HISTORICAL_STATS_GROUP_NO     0x0182

//
// Block Storage Optional RAID Disk Parameter Groups
//
#define     I2O_BSA_VOLUME_SET_INFORMATION_GROUP_NO     0x0200
#define     I2O_BSA_PROTECTED_SPACE_EXTENT_GROUP_NO     0x0201
#define     I2O_BSA_AGGREGATE_PROT_SPACE_EXT_GROUP_NO   0x0202
#define     I2O_BSA_PHYSICAL_EXTENT_GROUP_NO            0x0203
#define     I2O_BSA_AGGREGATE_PHYSICAL_EXT_GROUP_NO     0x0204
#define     I2O_BSA_REDUNDANCY_GROUP_NO                 0x0205
#define     I2O_BSA_COMPONENT_SPARES_GROUP_NO           0x0206
#define     I2O_BSA_ASSOCIATION_TABLE_GROUP_NO          0x0207


//
// 0000h - Device Information Parameters Group defines 
// 
//
// Device Type
//
#define I2O_BSA_DEVICE_TYPE_DIRECT          0x00
#define I2O_BSA_DEVICE_TYPE_WORM            0x04
#define I2O_BSA_DEVICE_TYPE_CDROM           0x05
#define I2O_BSA_DEVICE_TYPE_OPTICAL         0x07

//
// Device Capability Support
//
#define I2O_BSA_DEV_CAP_CACHING             0x00000001
#define I2O_BSA_DEV_CAP_MULTI_PATH          0x00000002
#define I2O_BSA_DEV_CAP_DYNAMIC_CAPACITY    0x00000004
#define I2O_BSA_DEV_CAP_REMOVABLE_MEDIA     0x00000008
#define I2O_BSA_DEV_CAP_REMOVEABLE_DEVICE   0x00000010
#define I2O_BSA_DEV_CAP_READ_ONLY           0x00000020
#define I2O_BSA_DEV_CAP_LOCKOUT             0x00000040
#define I2O_BSA_DEV_CAP_BOOT_BYPASS         0x00000080
#define I2O_BSA_DEV_CAP_COMPRESSION         0x00000100
#define I2O_BSA_DEV_CAP_DATA_SECURITY       0x00000200
#define I2O_BSA_DEV_CAP_RAID                0x00000400

//
// Device States 
//
#define I2O_BSA_DEV_STATE_CACHING           0x00000001
#define I2O_BSA_DEV_STATE_POWERED_ON        0x00000002
#define I2O_BSA_DEV_STATE_READY             0x00000004
#define I2O_BSA_DEV_STATE_MEDIA_LOADED      0x00000008
#define I2O_BSA_DEV_STATE_DEVICE_LOADED     0x00000010
#define I2O_BSA_DEV_STATE_READ_ONLY         0x00000020
#define I2O_BSA_DEV_STATE_LOCKOUT           0x00000040
#define I2O_BSA_DEV_STATE_BOOT_BYPASS       0x00000080
#define I2O_BSA_DEV_STATE_COMPRESSION       0x00000100
#define I2O_BSA_DEV_STATE_DATA_SECURITY     0x00000200
#define I2O_BSA_DEV_STATE_RAID              0x00000400


/* - 0001h - Operational Control Parameters Group defines */

/* No definition required */


/* - 0002h - Power Control Parameters Group defines */

/* On Access */

#define I2O_BSA_POWERED_UP_ON_ACCESS        0x00000001
#define I2O_BSA_LOAD_ON_ACCESS              0x00000002


/* - 0003h - Cache Control Parameters Group defines */

/* Write Policy */

#define I2O_BSA_NO_WRITE_CACHE              0x00
#define I2O_BSA_WRITE_TO_CACHE              0x01
#define I2O_BSA_WRITE_THRU_CACHE            0x02

/* Read Policy */

#define I2O_BSA_NO_READ_CACHE               0x00
#define I2O_BSA_READ_CACHE                  0x01
#define I2O_BSA_READ_AHEAD_CACHE            0x02
#define I2O_BSA_READ_READ_AHEAD_CACHE       0x03

/* Error Correction */

#define I2O_BSA_ERR_COR_NONE                0x00
#define I2O_BSA_ERR_COR_UNKNOWN             0x01
#define I2O_BSA_ERR_COR_OTHER               0x02
#define I2O_BSA_ERR_COR_PARITY              0x03
#define I2O_BSA_ERR_COR_SINGLE_BIT_ECC      0x04
#define I2O_BSA_ERR_COR_MULTI_BIT_ECC       0x05


/* - 0004h - Media Information Parameters Group defines */

/* No definition required */


/* - 0005h - Error Log Parameters Group defines */

/* No definition required */


/* - 0180h - Historical Statistics Control Parameters Group defines */

/* Statistis Control */

#define I2O_BSA_STAT_CTL_STORAGE_ENABLE     0x01
#define I2O_BSA_STAT_CTL_CACHE_ENABLE       0x02


/* - 0181h - Storage Historical Statistics Parameter Group defines */

/* No definition required */


/* - 0182h - Cache Historical Statistics Parameter Group defines */

/* No definition required */


/* - 0200H - Volume Set Information Parameter Group defines */

/* No definition required */


/* - 0201h - Protected Space Extent Parameter Group defines */

/* Data Stripe Granularity */

#define I2O_BSA_DATA_STRIPE_OTHER           0x00
#define I2O_BSA_DATA_STRIPE_UNKNOWN         0x01
#define I2O_BSA_DATA_STRIPE_BITS            0x02
#define I2O_BSA_DATA_STRIPE_BYTES           0x03
#define I2O_BSA_DATA_STRIPE_16BIT_WORDS     0x04
#define I2O_BSA_DATA_STRIPE_32BIT_DWORDS    0x05
#define I2O_BSA_DATA_STRIPE_BLOCKS          0x06

/* - 0202h - Aggregate Protected Space Extent Parameter Group defines */

/* No definition required */


/* - 0203h - Physical Extent Parameter Group defines */

/* Granularity Unit */

#define I2O_BSA_GRANULARITY_OTHER           0x00
#define I2O_BSA_GRANULARITY_UNKNOWN         0x01
#define I2O_BSA_GRANULARITY_BITS            0x02
#define I2O_BSA_GRANULARITY_BYTES           0x03
#define I2O_BSA_GRANULARITY_16BIT_WORDS     0x04
#define I2O_BSA_GRANULARITY_32BIT_DWORDS    0x05
#define I2O_BSA_GRANULARITY_BLOCKS          0x06


/* - 0204h - Aggregate Physical Extent Parameter Group defines */

/* No definition required */


/* - 0205h - Redundancy Parameter Group defines */

/* Redundancy Type */

#define I2O_BSA_REDUNDANCY_OTHER            0x00
#define I2O_BSA_REDUNDANCY_UNKNOWN          0x01
#define I2O_BSA_REDUNDANCY_NONE             0x02
#define I2O_BSA_REDUNDANCY_COPY             0x03
#define I2O_BSA_REDUNDANCY_XOR              0x04
#define I2O_BSA_REDUNDANCY_P_Q              0x05
#define I2O_BSA_REDUNDANCY_S                0x06
#define I2O_BSA_REDUNDANCY_P_S              0x07


/* - 0206h - Component Spares Parameter Group defines */

/* Spare Functioning State */

#define I2O_BSA_SPARE_STATE_OTHER           0x00
#define I2O_BSA_SPARE_STATE_UNKNOWN         0x01
#define I2O_BSA_SPARE_STATE_INACTIVE        0x02
#define I2O_BSA_SPARE_STATE_ACTIVE          0x03
#define I2O_BSA_SPARE_STATE_LOAD_BALANCE    0x04


/* - 0207h - Association Table Parameter Group defines */

/* Type */

#define I2O_BSA_ASSOC_TYPE_PHYSICAL         0x00
#define I2O_BSA_ASSOC_TYPE_LOGICAL          0x01
#define I2O_BSA_ASSOC_TYPE_LOG_TO_PHYS      0x02
#define I2O_BSA_ASSOC_TYPE_PROTECTION       0x03
#define I2O_BSA_ASSOC_TYPE_SPARE            0x04
#define I2O_BSA_ASSOC_TYPE_CACHE            0x05
#define I2O_BSA_ASSOC_TYPE_SOFTWARE         0x06

/* Object 1 Type */

#define I2O_BSA_OBJECT_1_CONTROLLER         0x00
#define I2O_BSA_OBJECT_1_DEVICE             0x01
#define I2O_BSA_OBJECT_1_BUS_PORT           0x02
#define I2O_BSA_OBJECT_1_VOLUME_SET         0x03
#define I2O_BSA_OBJECT_1_PROT_SPACE_EXT     0x04
#define I2O_BSA_OBJECT_1_AGG_PROT_SPACE_EXT 0x05
#define I2O_BSA_OBJECT_1_PHYSICAL_EXT       0x06
#define I2O_BSA_OBJECT_1_AGG_PHYSICAL_EXT   0x07
#define I2O_BSA_OBJECT_1_REDUNDANCY         0x08
#define I2O_BSA_OBJECT_1_CACHE              0x09
#define I2O_BSA_OBJECT_1_SOFTWARE           0x0A

/* Object 2 Type */

#define I2O_BSA_OBJECT_2_CONTROLLER         0x00
#define I2O_BSA_OBJECT_2_DEVICE             0x01
#define I2O_BSA_OBJECT_2_BUS_PORT           0x02
#define I2O_BSA_OBJECT_2_VOLUME_SET         0x03
#define I2O_BSA_OBJECT_2_PROT_SPACE_EXT     0x04
#define I2O_BSA_OBJECT_2_AGG_PROT_SPACE_EXT 0x05
#define I2O_BSA_OBJECT_2_PHYSICAL_EXT       0x06
#define I2O_BSA_OBJECT_2_AGG_PHYSICAL_EXT   0x07
#define I2O_BSA_OBJECT_2_REDUNDANCY         0x08
#define I2O_BSA_OBJECT_2_CACHE              0x09
#define I2O_BSA_OBJECT_2_SOFTWARE           0x0A



/* Block Storage Group 0000h - Device Information Parameter Group */

typedef struct _I2O_BSA_DEVICE_INFO_SCALAR {
    U8          DeviceType;
    U8          NumberOfPaths;
    U16         PowerState;     
    U32         BlockSize;
    U64         DeviceCapacity;
    U32         DeviceCapabilitySupport;
    U32         DeviceState;
} I2O_BSA_DEVICE_INFO_SCALAR, *PI2O_BSA_DEVICE_INFO_SCALAR;


/* Block Storage Group 0001h - Operational Control Parameter Group */

typedef struct _I2O_BSA_OPERATIONAL_CONTROL_SCALAR {
    U8          AutoReassign;
    U8          ReassignTolerance;
    U8          RetryAttempts;
    U8          reserved1;
    U32         ReassignSize;
    U32         ExpectedTimeout;
    U32         RWVTimeout;
    U32         RWVTimeoutBase;
    U32         TimeoutBase;
    U32         OrderedRequestDepth;
    U32         AtomicWriteSize;
} I2O_BSA_OPERATIONAL_CONTROL_SCALAR, *PI2O_BSA_OPERATIONAL_CONTROL_SCALAR;


/* Block Storage Group 0002h - Power Control Parameter Group */

typedef struct _I2O_BSA_POWER_CONTROL_SCALAR {
    U32         PowerdownTimeout;
    U32         OnAccess;
} I2O_BSA_POWER_CONTROL_SCALAR, *PI2O_BSA_POWER_CONTROL_SCALAR;


/* Block Storage Group 0003h - Cache Control Parameter Group */

typedef struct _I2O_BSA_CACHE_CONTROL_SCALAR {
    U32         TotalCacheSize;
    U32         ReadCacheSize;
    U32         WriteCacheSize; 
    U8          WritePolicy;
    U8          ReadPolicy;
    U8          ErrorCorrection;
    U8          reserved1;          /* Note: not in 1.5 spec. */
} I2O_BSA_CACHE_CONTROL_SCALAR, *PI2O_BSA_CACHE_CONTROL_SCALAR;


/* Block Storage Group 0004h - Media Information Parameter Group */

typedef struct _I2O_BSA_MEDIA_INFO_SCALAR {
    U64         Capacity;
    U32         BlockSize;
} I2O_BSA_MEDIA_INFO_SCALAR, *PI2O_BSA_MEDIA_INFO_SCALAR;


/* Block Storage Group 0005h - Error Log Parameter Group */ 

typedef struct _I2O_BSA_ERROR_LOG_TABLE {
    U16         ErrorDataIndex;
    U8          Function;
    U8          RetryCount;
    U16         DetailedErrorCode;
    U16         reserved2;
    U64         TimeStamp;
    U32         UserInfo;
} I2O_BSA_ERROR_LOG_TABLE, *PI2O_BSA_ERROR_LOG_TABLE;


/* Block Storage Group 0180h - Optional Historical STATS Support/Control */

typedef struct _I2O_BSA_HIST_STATS_SCALAR {
    U8          StatisticsControl;
    U8          reserved1;
    U16         reserved2;
    U32         StorageStatistics;
    U32         CacheStatistics;
} I2O_BSA_HIST_STATS_SCALAR, *PI2O_BSA_HIST_STATS_SCALAR;


/* Block Storage Group 0181h - Optional Storage Historical STATS */

typedef struct _I2O_BSA_STORAGE_HIST_STATS_SCALAR {
    U64         ReadCommands;
    U64         WriteCommands;
    U8          DataUnit;
    U8          reserved1;
    U16         reserved2;
    U64         IORange1Read;
    U64         IORange2Read;
    U64         IORange3Read;
    U64         IORange4Read;
    U64         IORange1Write;
    U64         IORange2Write;
    U64         IORange3Write;
    U64         IORange4Write;
    U64         NumberSeeks;
} I2O_BSA_STORAGE_HIST_STATS_SCALAR, *PI2O_BSA_STORAGE_HIST_STATS_SCALAR;


/* Block Storage Group 0182h - Optional Cache Historical STATS */

typedef struct _I2O_BSA_CACHE_HIST_STATS_SCALAR {
    U64         CacheAccess;
    U64         CacheHit;
    U64         PartialCacheHit;
    U64         HitDataSize;
    U32         ValidUsage;
    U32         DirtyUsage;
    U32         TimeLastFault;
    U32         LastFaultFailure;
} I2O_BSA_CACHE_HIST_STATS_SCALAR, *PI2O_BSA_CACHE_HIST_STATS_SCALAR;


/* Block Storage Group 0200h - Optional Volume Set Information */

typedef struct _I2O_BSA_VOLUME_INFO_SCALAR {
    U8          Name[64];
    U64         TotalStorageCapacity;
    U64         StripeLength;
    U64         InterleaveDepth;
} I2O_BSA_VOLUME_INFO_SCALAR, *PI2O_BSA_VOLUME_INFO_SCALAR;


/* Block Storage Group 0201h - Optional Protected Space Extent */

typedef struct _I2O_BSA_PROT_SPACE_EXT_SCALAR {
    U64         StartAddress;
    U64         NumberBlocks;
    U32         BlockSize;
    U32         DataStripeGranularity;
    U32         DataStripeLength;
} I2O_BSA_PROT_SPACE_EXT_SCALAR, *PI2O_BSA_PROT_SPACE_EXT_SCALAR;


/* Block Storage Group 0202h - Optional Aggregate Protected Space Extent */

typedef struct _I2O_BSA_AGG_PROT_SPACE_EXT_SCALAR {
    U64         NumberBlocks;
} I2O_BSA_AGG_PROT_SPACE_EXT_SCALAR, *PI2O_BSA_AGG_PROT_SPACE_EXT_SCALAR;


/* Block Storage Group 0203h - Optional Physical Extent */

typedef struct _I2O_BSA_PHYS_EXT_SCALAR {
    U64         StartAddress;
    U64         NumberBlocks;
    U32         BlockSize;
    U32         GranularityUnit;
    U64         CheckDataInterleave;
    U64         CheckData;
    U64         UserData;
} I2O_BSA_PHYS_EXT_SCALAR, *PI2O_BSA_PHYS_EXT_SCALAR;


/* Block Storage Group 0204h - Optional Aggregate Physical Extent */

typedef struct _I2O_BSA_AGG_PHYS_EXT_SCALAR {
    U64         NumberBlocks;
    U64         CheckData;
} I2O_BSA_AGG_PHYS_EXT_SCALAR, *PI2O_BSA_AGG_PHYS_EXT_SCALAR;


/* Block Storage Group 0205h - Optional Redundancy Table */

typedef struct _I2O_BSA_REDUNDANCY_SCALAR {
    U8          RedundancyType;
    U8          reserved1;      /* Note: not in 1.5 spec. */
    U16         reserved2;      /* Note: not in 1.5 spec. */
} I2O_BSA_REDUNDANCY_SCALAR, *PI2O_BSA_REDUNDANCY_SCALAR;


/* Block Storage Group 0206h - Optional Component Spares */

typedef struct _I2O_BSA_COMPONENT_SPARES_TABLE {
    U8          RowNumber;
    U8          SpareType;
    U8          ToBeSparedIndex;
    U8          SparedIndex;
    U8          SpareFunctioningState;
    U8          reserved1;      /* Note: not in 1.5 spec. */
    U16         reserved2;      /* Note: not in 1.5 spec. */
} I2O_BSA_COMPONENT_SPARES_TABLE, *PI2O_BSA_COMPONENT_SPARES_TABLE;


/* Block Storage Group 0207h - Optional Association Table */

typedef struct _I2O_BSA_ASSOCIATION_TABLE {
    U8          RowNumber;
    U8          Type;
    U8          Object1Type;
    U8          Object1Index;
    U8          Object2Type;
    U8          Object2Index;
    U16         reserved2;      /* Note: not in 1.5 spec. */
} I2O_BSA_ASSOCIATION_TABLE, *PI2O_BSA_ASSOCIATION_TABLE;


/****************************************************************************/

/* I2O BSA Block Storage Event Indicator Assignment */

#define I2O_BSA_EVENT_VOLUME_LOAD               0x00000001
#define I2O_BSA_EVENT_VOLUME_UNLOAD             0x00000002
#define I2O_BSA_EVENT_VOLUME_UNLOAD_REQUEST     0x00000004
#define I2O_BSA_EVENT_CAPACITY_CHANGE           0x00000008
#define I2O_BSA_EVENT_SCSI_SMART                0x00000010



/****************************************************************************/

/* Block Storage Class Specific Message Definitions */

/****************************************************************************/


/****************************************************************************/

/* I2O Block Storage Reply Message Frame Template */

typedef struct _I2O_BSA_REPLY_MESSAGE_FRAME {
    I2O_MESSAGE_FRAME           StdMessageFrame;
    I2O_TRANSACTION_CONTEXT     TransactionContext;
    U16                         DetailedStatusCode;
    U8                          RetryCount;
    U8                          ReqStatus;
/*                              ReplyPayload        */
} I2O_BSA_REPLY_MESSAGE_FRAME, *PI2O_BSA_REPLY_MESSAGE_FRAME;


/****************************************************************************/

/* I2O Block Storage Successful Completion Reply Message Frame */

typedef struct _I2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME {
    I2O_BSA_REPLY_MESSAGE_FRAME BsaReplyFrame;
    U32                         TransferCount;
} I2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME, *PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME;


/****************************************************************************/

/* I2O Block Storage Aborted Operation Reply Message Frame */

typedef struct _I2O_BSA_ABORT_REPLY_MESSAGE_FRAME {
    I2O_BSA_REPLY_MESSAGE_FRAME BsaReplyFrame;
} I2O_BSA_ABORT_REPLY_MESSAGE_FRAME, *PI2O_BSA_ABORT_REPLY_MESSAGE_FRAME;


/****************************************************************************/

/* I2O Block Storage Progress Report Reply Message Frame */

typedef struct _I2O_BSA_PROGRESS_REPLY_MESSAGE_FRAME {
    I2O_BSA_REPLY_MESSAGE_FRAME BsaReplyFrame;
    U8                          PercentComplete;
    U8                          Reserved[3];
} I2O_BSA_PROGRESS_REPLY_MESSAGE_FRAME, *PI2O_BSA_PROGRESS_REPLY_MESSAGE_FRAME;

/****************************************************************************/

/* I2O Block Storage Error Report Reply Message Frame */

typedef struct _I2O_BSA_ERROR_REPLY_MESSAGE_FRAME {
    I2O_BSA_REPLY_MESSAGE_FRAME BsaReplyFrame;
    U32                         TransferCount;
    U64                         LogicalByteAddress;
} I2O_BSA_ERROR_REPLY_MESSAGE_FRAME, *PI2O_BSA_ERROR_REPLY_MESSAGE_FRAME;



/****************************************************************************/

/* I2O BSA request message flag definitions */

/* I2O BSA Control Flags */

typedef U16     I2O_BSA_CTL_FLAGS;

#define I2O_BSA_FLAG_PROGRESS_REPORT    0x0080

/* I2O BSA Block Read Message Control Flags */

typedef U16     I2O_BSA_READ_FLAGS;
#define I2O_BSA_RD_FLAG_DONT_RETRY      0x0001
#define I2O_BSA_RD_FLAG_SOLO            0x0002
#define I2O_BSA_RD_FLAG_CACHE_READ      0x0004
#define I2O_BSA_RD_FLAG_READ_PREFETCH   0x0008
#define I2O_BSA_RD_FLAG_CACHE_DATA      0x0010

/* I2O BSA Block Write Message Control Flags */

typedef U16     I2O_BSA_WRITE_FLAGS;
#define I2O_BSA_WR_FLAG_DONT_RETRY      0x0001
#define I2O_BSA_WR_FLAG_SOLO            0x0002
#define I2O_BSA_WR_FLAG_DONT_CACHE      0x0004
#define I2O_BSA_WR_FLAG_WRITE_THRU      0x0008
#define I2O_BSA_WR_FLAG_WRITE_TO        0x0010

/* I2O BSA Device Reset Message Control Flags */

typedef U16     I2O_BSA_RESET_FLAGS;
#define I2O_BSA_FLAG_HARD_RESET         0x0001

/* I2O BSA Media Verify Message Control Flags */

typedef U16     I2O_BSA_VERIFY_FLAGS;
/* Progress Report flag definition is valid  */
#define I2O_BSA_ERROR_CORRECTION        0x0040


/* I2O BSA Removeable Media Identifier values */

typedef U32     I2O_BSA_MEDIA_ID;
#define I2O_BSA_MEDIA_ID_CURRENT_MOUNTED    0xFFFFFFFF


/* I2O BSA Removeable Media Load Flags */

typedef U8      I2O_BSA_LOAD_FLAGS;
#define I2O_BSA_LOAD_FLAG_MEDIA_LOCK    0x80


/* I2O BSA Power Management Operation values */

typedef U8      I2O_BSA_OPERATION;
#define I2O_BSA_POWER_MGT_PARTIAL_POWER_UP          0x01
#define I2O_BSA_POWER_MGT_POWER_UP                  0x02
#define I2O_BSA_POWER_MGT_POWER_UP_LOAD             0x03
#define I2O_BSA_POWER_MGT_QUIESCE_DEVICE            0x20
#define I2O_BSA_POWER_MGT_PARTIAL_POWER_DOWN        0x21
#define I2O_BSA_POWER_MGT_PARTIAL_POWER_DOWN_UNLOAD 0x22
#define I2O_BSA_POWER_MGT_POWER_DOWN_UNLOAD         0x23
#define I2O_BSA_POWER_MGT_POWER_DOWN_RETAIN         0x24


/****************************************************************************/

/* I2O BSA Block Read Message Frame */

typedef struct _I2O_BSA_READ_MESSAGE {
    I2O_MESSAGE_FRAME       StdMessageFrame;
    I2O_TRANSACTION_CONTEXT TransactionContext;
    I2O_BSA_READ_FLAGS      ControlFlags;
    U8                      TimeMultiplier;
    U8                      FetchAhead;
    U32                     TransferByteCount;
    U64                     LogicalByteAddress;
    I2O_SG_ELEMENT          SGL;
} I2O_BSA_READ_MESSAGE, *PI2O_BSA_READ_MESSAGE;


/****************************************************************************/

/* I2O BSA Block Reassign Message Frame */

typedef struct _I2O_BSA_BLOCK_REASSIGN_MESSAGE {
    I2O_MESSAGE_FRAME       StdMessageFrame;
    I2O_TRANSACTION_CONTEXT TransactionContext;
    U16                     Reserved1;
    U8                      TimeMultiplier;
    U8                      Reserved2;
//
//  I2O_SG_ELEMENT          SGL;
//  In the v1.5 Errata doc, the SGL field has been replaced by the following:
//
    U32                     ByteCount;
    U64                     LogicalByteAddress;
} I2O_BSA_BLOCK_REASSIGN_MESSAGE, *PI2O_BSA_BLOCK_REASSIGN_MESSAGE;


/****************************************************************************/

/* I2O BSA Block Write Message Frame */

typedef struct _I2O_BSA_WRITE_MESSAGE {
    I2O_MESSAGE_FRAME       StdMessageFrame;
    I2O_TRANSACTION_CONTEXT TransactionContext;
    I2O_BSA_WRITE_FLAGS     ControlFlags;
    U8                      TimeMultiplier;
    U8                      Reserved;
    U32                     TransferByteCount;
    U64                     LogicalByteAddress;
    I2O_SG_ELEMENT          SGL;
} I2O_BSA_WRITE_MESSAGE, *PI2O_BSA_WRITE_MESSAGE;


/****************************************************************************/

/* I2O BSA Block Write and Verify Message Frame */

typedef struct _I2O_BSA_WRITE_VERIFY_MESSAGE {
    I2O_MESSAGE_FRAME       StdMessageFrame;
    I2O_TRANSACTION_CONTEXT TransactionContext;
    I2O_BSA_WRITE_FLAGS     ControlFlags;
    U8                      TimeMultiplier;
    U8                      Reserved;
    U32                     TransferByteCount;
    U64                     LogicalByteAddress;
    I2O_SG_ELEMENT          SGL;
} I2O_BSA_WRITE_VERIFY_MESSAGE, *PI2O_BSA_WRITE_VERIFY_MESSAGE;


/****************************************************************************/

/* I2O BSA Cache Flush Message Frame */

typedef struct _I2O_BSA_CACHE_FLUSH_MESSAGE {
    I2O_MESSAGE_FRAME       StdMessageFrame;
    I2O_TRANSACTION_CONTEXT TransactionContext;
    I2O_BSA_CTL_FLAGS       ControlFlags;
    U8                      TimeMultiplier;
    U8                      Reserved;
} I2O_BSA_CACHE_FLUSH_MESSAGE, *PI2O_BSA_CACHE_FLUSH_MESSAGE;


/****************************************************************************/

/* I2O BSA Device Reset Message Frame */

typedef struct _I2O_BSA_DEVICE_RESET_MESSAGE {
    I2O_MESSAGE_FRAME       StdMessageFrame;
    I2O_TRANSACTION_CONTEXT TransactionContext;
    I2O_BSA_RESET_FLAGS     ControlFlags;
    U8                      TimeMultiplier;
    U8                      Reserved;
} I2O_BSA_DEVICE_RESET_MESSAGE, *PI2O_BSA_DEVICE_RESET_MESSAGE;


/****************************************************************************/

/* I2O BSA Media Eject for Removeable Media Message Frame */

typedef struct _I2O_BSA_MEDIA_EJECT_MESSAGE {
    I2O_MESSAGE_FRAME       StdMessageFrame;
    I2O_TRANSACTION_CONTEXT TransactionContext;
    I2O_BSA_MEDIA_ID        MediaIdentifier;
} I2O_BSA_MEDIA_EJECT_MESSAGE, *PI2O_BSA_MEDIA_EJECT_MESSAGE;


/****************************************************************************/

/* I2O BSA Media Lock Message Frame */

typedef struct _I2O_BSA_MEDIA_LOCK_MESSAGE {
    I2O_MESSAGE_FRAME       StdMessageFrame;
    I2O_TRANSACTION_CONTEXT TransactionContext;
    I2O_BSA_MEDIA_ID        MediaIdentifier;
} I2O_BSA_MEDIA_LOCK_MESSAGE, *PI2O_BSA_MEDIA_LOCK_MESSAGE;


/****************************************************************************/

/* I2O BSA Media Mount for Removeable Media Message Frame */

typedef struct _I2O_BSA_MEDIA_MOUNT_MESSAGE {
    I2O_MESSAGE_FRAME       StdMessageFrame;
    I2O_TRANSACTION_CONTEXT TransactionContext;
    I2O_BSA_MEDIA_ID        MediaIdentifier;
    I2O_BSA_LOAD_FLAGS      LoadFlags;
    U8                      Reserved[3];
} I2O_BSA_MEDIA_MOUNT_MESSAGE, *PI2O_BSA_MEDIA_MOUNT_MESSAGE;


/****************************************************************************/

/* I2O BSA Media Unlock Message Frame */

typedef struct _I2O_BSA_MEDIA_UNLOCK_MESSAGE {
    I2O_MESSAGE_FRAME       StdMessageFrame;
    I2O_TRANSACTION_CONTEXT TransactionContext;
    I2O_BSA_MEDIA_ID        MediaIdentifier;
} I2O_BSA_MEDIA_UNLOCK_MESSAGE, *PI2O_BSA_MEDIA_UNLOCK_MESSAGE;


/****************************************************************************/

/* I2O BSA Media Verify Message Frame */

typedef struct _I2O_BSA_MEDIA_VERIFY_MESSAGE {
    I2O_MESSAGE_FRAME       StdMessageFrame;
    I2O_TRANSACTION_CONTEXT TransactionContext;
    I2O_BSA_VERIFY_FLAGS    ControlFlags;
    U8                      TimeMultiplier;
    U8                      Reserved;
    U32                     ByteCount;
    U64                     LogicalByteAddress;
} I2O_BSA_MEDIA_VERIFY_MESSAGE, *PI2O_BSA_MEDIA_VERIFY_MESSAGE;


/****************************************************************************/

/* I2O BSA Power Management Message Frame */

typedef struct _I2O_BSA_POWER_MANAGEMENT_MESSAGE {
    I2O_MESSAGE_FRAME       StdMessageFrame;
    I2O_TRANSACTION_CONTEXT TransactionContext;
    I2O_BSA_CTL_FLAGS       ControlFlags;
    U8                      TimeMultiplier;
    U8                      ReplyType;
    I2O_BSA_OPERATION       Operation;
} I2O_BSA_POWER_MANAGEMENT_MESSAGE, *PI2O_BSA_POWER_MANAGEMENT_MESSAGE;


/****************************************************************************/

/* I2O BSA Status Check Message Frame */

typedef struct _I2O_BSA_STATUS_CHECK_MESSAGE {
    I2O_MESSAGE_FRAME       StdMessageFrame;
    I2O_TRANSACTION_CONTEXT TransactionContext;
} I2O_BSA_STATUS_CHECK_MESSAGE, *PI2O_BSA_STATUS_CHECK_MESSAGE;


#pragma pack(pop)

PRAGMA_ALIGN_POP

#endif      /* I2O_MSTOR_HDR  */


