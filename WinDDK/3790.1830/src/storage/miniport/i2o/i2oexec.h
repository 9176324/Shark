/*++

Copyright (c) 1998-2002  Microsoft Corporation

Module Name:

    i2oexec.h

Abstract:

    This file is the I2O Executive Class Message definition file and
    contains information presented in Chapter 4 of the I2O(tm) Specification.

Author:

Environment:

    kernel mode only

Revision History:
 
--*/


#if !defined(I2O_EXECUTIVE_HDR)
#define I2O_EXECUTIVE_HDR

#define I2OEXEC_REV 1_5_5  /* I2OExec header file revision string */

#include   "i2omsg.h"      /* Include the Base Message file */
#include   "i2outil.h"

//
//  NOTES:
//    
//    Gets, reads, receives, etc. are all even numbered functions.
//    Sets, writes, sends, etc. are all odd numbered functions.
//    Functions that both send and receive data can be either but an attempt is made
//        to use the function number that indicates the greater transfer amount.
//    Functions that do not send or receive data use odd function numbers.
//
//    Some functions are synonyms like read, receive and send, write.
//    
//    All common functions will have a code of less than 0x80.
//    Unique functions to a class will start at 0x80.
//    Executive Functions start at 0xA0.
//    
//    Utility Message function codes range from 0 - 0x1f
//    Base Message function codes range from 0x20 - 0xfe
//    Private Message function code is 0xff.
//

PRAGMA_ALIGN_PUSH
#pragma pack(push,1)

/*  I2O Executive Function Codes.  */

#define    I2O_EXEC_ADAPTER_ASSIGN                     0xB3
#define    I2O_EXEC_ADAPTER_READ                       0xB2
#define    I2O_EXEC_ADAPTER_RELEASE                    0xB5
#define    I2O_EXEC_BIOS_INFO_SET                      0xA5
#define    I2O_EXEC_BOOT_DEVICE_SET                    0xA7
#define    I2O_EXEC_CONFIG_VALIDATE                    0xBB
#define    I2O_EXEC_CONN_SETUP                         0xCA
#define    I2O_EXEC_DDM_DESTROY                        0xB1
#define    I2O_EXEC_DDM_ENABLE                         0xD5
#define    I2O_EXEC_DDM_QUIESCE                        0xC7
#define    I2O_EXEC_DDM_RESET                          0xD9
#define    I2O_EXEC_DDM_SUSPEND                        0xAF
#define    I2O_EXEC_DEVICE_ASSIGN                      0xB7
#define    I2O_EXEC_DEVICE_RELEASE                     0xB9
#define    I2O_EXEC_HRT_GET                            0xA8
#define    I2O_EXEC_IOP_CLEAR                          0xBE
#define    I2O_EXEC_IOP_CONNECT                        0xC9
#define    I2O_EXEC_IOP_RESET                          0xBD
#define    I2O_EXEC_LCT_NOTIFY                         0xA2
#define    I2O_EXEC_OUTBOUND_INIT                      0xA1
#define    I2O_EXEC_PATH_ENABLE                        0xD3
#define    I2O_EXEC_PATH_QUIESCE                       0xC5
#define    I2O_EXEC_PATH_RESET                         0xD7
#define    I2O_EXEC_STATIC_MF_CREATE                   0xDD
#define    I2O_EXEC_STATIC_MF_RELEASE                  0xDF
#define    I2O_EXEC_STATUS_GET                         0xA0
#define    I2O_EXEC_SW_DOWNLOAD                        0xA9
#define    I2O_EXEC_SW_UPLOAD                          0xAB
#define    I2O_EXEC_SW_REMOVE                          0xAD
#define    I2O_EXEC_SYS_ENABLE                         0xD1
#define    I2O_EXEC_SYS_MODIFY                         0xC1
#define    I2O_EXEC_SYS_QUIESCE                        0xC3
#define    I2O_EXEC_SYS_TAB_SET                        0xA3


/* I2O Get Status State values */

#define    I2O_IOP_STATE_INITIALIZING                  0x01
#define    I2O_IOP_STATE_RESET                         0x02
#define    I2O_IOP_STATE_HOLD                          0x04
#define    I2O_IOP_STATE_READY                         0x05
#define    I2O_IOP_STATE_OPERATIONAL                   0x08
#define    I2O_IOP_STATE_FAILED                        0x10
#define    I2O_IOP_STATE_FAULTED                       0x11


/* Event Indicator Assignments for the Executive Class. */

#define    I2O_EVENT_IND_RESOURCE_LIMIT                0x00000001
#define    I2O_EVENT_IND_CONNECTION_FAIL               0x00000002
#define    I2O_EVENT_IND_ADAPTER_FAULT                 0x00000004
#define    I2O_EVENT_IND_POWER_FAIL                    0x00000008
#define    I2O_EVENT_IND_RESET_PENDING                 0x00000010
#define    I2O_EVENT_IND_RESET_IMMINENT                0x00000020
#define    I2O_EVENT_IND_HARDWARE_FAIL                 0x00000040
#define    I2O_EVENT_IND_XCT_CHANGE                    0x00000080
#define    I2O_EVENT_IND_NEW_LCT_ENTRY                 0x00000100
#define    I2O_EVENT_IND_MODIFIED_LCT                  0x00000200
#define    I2O_EVENT_IND_DDM_AVAILABILITY              0x00000400

/* Resource Limit Event Data */

#define    I2O_EVENT_RESOURCE_LIMIT_LOW_MEMORY         0x00000001
#define    I2O_EVENT_RESOURCE_LIMIT_INBOUND_POOL_LOW   0x00000002
#define    I2O_EVENT_RESOURCE_LIMIT_OUTBOUND_POOL_LOW  0x00000004

/* Connection Fail Event Data */

#define    I2O_EVENT_CONNECTION_FAIL_REPOND_NORMAL     0x00000000
#define    I2O_EVENT_CONNECTION_FAIL_NOT_REPONDING     0x00000001
#define    I2O_EVENT_CONNECTION_FAIL_NO_AVAILABLE_FRAMES 0x00000002

/* Reset Pending Event Data */

#define    I2O_EVENT_RESET_PENDING_POWER_LOSS          0x00000001
#define    I2O_EVENT_RESET_PENDING_CODE_VIOLATION      0x00000002

/* Reset Imminent Event Data */

#define    I2O_EVENT_RESET_IMMINENT_UNKNOWN_CAUSE      0x00000000
#define    I2O_EVENT_RESET_IMMINENT_POWER_LOSS         0x00000001
#define    I2O_EVENT_RESET_IMMINENT_CODE_VIOLATION     0x00000002
#define    I2O_EVENT_RESET_IMMINENT_PARITY_ERROR       0x00000003
#define    I2O_EVENT_RESET_IMMINENT_CODE_EXCEPTION     0x00000004
#define    I2O_EVENT_RESET_IMMINENT_WATCHDOG_TIMEOUT   0x00000005

/* Hardware Fail Event Data */

#define    I2O_EVENT_HARDWARE_FAIL_UNKNOWN_CAUSE       0x00000000
#define    I2O_EVENT_HARDWARE_FAIL_CPU_FAILURE         0x00000001
#define    I2O_EVENT_HARDWARE_FAIL_MEMORY_FAULT        0x00000002
#define    I2O_EVENT_HARDWARE_FAIL_DMA_FAILURE         0x00000003
#define    I2O_EVENT_HARDWARE_FAIL_IO_BUS_FAILURE      0x00000004

/* DDM Availability Event Data */

#define    I2O_EVENT_DDM_AVAILIBILITY_RESPOND_NORMAL   0x00000000
#define    I2O_EVENT_DDM_AVAILIBILITY_CONGESTED        0x00000001
#define    I2O_EVENT_DDM_AVAILIBILITY_NOT_RESPONDING   0x00000002
#define    I2O_EVENT_DDM_AVAILIBILITY_PROTECTION_VIOLATION 0x00000003
#define    I2O_EVENT_DDM_AVAILIBILITY_CODE_VIOLATION   0x00000004

/****************************************************************************/

#define    I2O_OPERATION_FLAG_ASSIGN_PERMANENT         0x01

/* ExecAdapterAssign Function Message Frame structure. */

typedef struct _I2O_EXEC_ADAPTER_ASSIGN_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   BF                          DdmTID:I2O_TID_SZ;
   BF                          reserved:I2O_RESERVED_12BITS;
   BF                          OperationFlags:I2O_8BIT_FLAGS_SZ;
   I2O_HRT_ENTRY               HRTEntry;
} I2O_EXEC_ADAPTER_ASSIGN_MESSAGE, *PI2O_EXEC_ADAPTER_ASSIGN_MESSAGE;


/****************************************************************************/

#define    I2O_REQUEST_FLAG_CONFIG_REGISTER            0x00000000
#define    I2O_REQUEST_FLAG_IO_REGISTER                0x00000001
#define    I2O_REQUEST_FLAG_ADAPTER_MEMORY             0x00000002

/* ExecAdapterRead Function Message Frame structure. */

typedef struct _I2O_EXEC_ADAPTER_READ_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   U32                         AdapterID;
   U32                         RequestFlags;
   U32                         Offset;
   U32                         Length;
   I2O_SG_ELEMENT              SGL;
} I2O_EXEC_ADAPTER_READ_MESSAGE, *PI2O_EXEC_ADAPTER_READ_MESSAGE;


/****************************************************************************/

#define    I2O_OPERATION_FLAG_RELEASE_PERMANENT        0x01

/* ExecAdapterRelease Function Message Frame structure. */

typedef struct _I2O_EXEC_ADAPTER_RELEASE_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   U8                          reserved[3];
   U8                          OperationFlags;
   I2O_HRT_ENTRY               HRTEntry;
} I2O_EXEC_ADAPTER_RELEASE_MESSAGE, *PI2O_EXEC_ADAPTER_RELEASE_MESSAGE;


/****************************************************************************/

/* ExecBiosInfoSet Function Message Frame structure. */

typedef struct _I2O_EXEC_BIOS_INFO_SET_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   BF                          DeviceTID:I2O_TID_SZ;
   BF                          reserved:I2O_RESERVED_12BITS;
   BF                          BiosInfo:I2O_BIOS_INFO_SZ;
} I2O_EXEC_BIOS_INFO_SET_MESSAGE, *PI2O_EXEC_BIOS_INFO_SET_MESSAGE;


/****************************************************************************/

/* ExecBootDeviceSet Function Message Frame structure. */

typedef struct _I2O_EXEC_BOOT_DEVICE_SET_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   BF                          BootDevice:I2O_TID_SZ;
   BF                          reserved:I2O_RESERVED_20BITS;
} I2O_EXEC_BOOT_DEVICE_SET_MESSAGE, *PI2O_EXEC_BOOT_DEVICE_SET_MESSAGE;


/****************************************************************************/

/* ExecConfigValidate Function Message Frame structure. */

typedef struct _I2O_EXEC_CONFIG_VALIDATE_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
} I2O_EXEC_CONFIG_VALIDATE_MESSAGE, *PI2O_EXEC_CONFIG_VALIDATE_MESSAGE;


/****************************************************************************/

/* ExecConnSetup Requestor  */

typedef struct _I2O_ALIAS_CONNECT_SETUP {
   BF                          IOP1AliasForTargetDevice:I2O_TID_SZ;
   BF                          IOP2AliasForInitiatorDevice:I2O_TID_SZ;
   BF                          reserved:I2O_RESERVED_8BITS;
} I2O_ALIAS_CONNECT_SETUP, *PI2O_ALIAS_CONNECT_SETUP;

#define    I2O_OPERATION_FLAG_PEER_TO_PEER_BIDIRECTIONAL   0x01

/* ExecConnSetup Object  */

typedef struct _I2O_OBJECT_CONNECT_SETUP {
   BF                          TargetDevice:I2O_TID_SZ;
   BF                          InitiatorDevice:I2O_TID_SZ;
   BF                          OperationFlags:I2O_8BIT_FLAGS_SZ;
} I2O_OBJECT_CONNECT_SETUP, *PI2O_OBJECT_CONNECT_SETUP;


/* ExecConnSetup Function Message Frame structure. */

typedef struct _I2O_EXEC_CONN_SETUP_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   I2O_OBJECT_CONNECT_SETUP    ObjectInfo;
   I2O_ALIAS_CONNECT_SETUP     AliasInfo;
   U16                         IOP1InboundMFrameSize;
   U16                         reserved;
   U32                         MessageClass;
} I2O_EXEC_CONN_SETUP_MESSAGE, *PI2O_EXEC_CONN_SETUP_MESSAGE;


/* ExecConnSetup Object Reply */

typedef struct _I2O_OBJECT_CONNECT_REPLY {
   BF                          TargetDevice:I2O_TID_SZ;
   BF                          InitiatorDevice:I2O_TID_SZ;
   BF                          ReplyStatusCode:I2O_8BIT_FLAGS_SZ;
} I2O_OBJECT_CONNECT_REPLY, *PI2O_OBJECT_CONNECT_REPLY;


/* ExecConnSetup reply structure. */

typedef struct _I2O_EXEC_CONN_SETUP_REPLY {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   I2O_OBJECT_CONNECT_REPLY    ObjectInfo;
   I2O_ALIAS_CONNECT_SETUP     AliasInfo;
   U16                         IOP2InboundMFrameSize;
   U16                         reserved;
} I2O_EXEC_CONN_SETUP_REPLY, *PI2O_EXEC_CONN_SETUP_REPLY;


/****************************************************************************/

/* ExecDdmDestroy Function Message Frame structure. */

typedef struct _I2O_EXEC_DDM_DESTROY_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   BF                          DdmTID:I2O_TID_SZ;
   BF                          reserved:I2O_RESERVED_20BITS;
} I2O_EXEC_DDM_DESTROY_MESSAGE, *PI2O_EXEC_DDM_DESTROY_MESSAGE;


/****************************************************************************/

/* ExecDdmEnable Function Message Frame structure. */

typedef struct _I2O_EXEC_DDM_ENABLE_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   BF                          DeviceTID:I2O_TID_SZ;
   BF                          reserved1:I2O_RESERVED_20BITS;
   BF                          IOP_ID:I2O_IOP_ID_SZ;
   BF                          reserved:I2O_RESERVED_4BITS;
   BF                          HostUnitID:I2O_UNIT_ID_SZ;
} I2O_EXEC_DDM_ENABLE_MESSAGE, *PI2O_EXEC_DDM_ENABLE_MESSAGE;


/****************************************************************************/

/* ExecDdmQuiesce Function Message Frame structure. */

typedef struct _I2O_EXEC_DDM_QUIESCE_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   BF                          DeviceTID:I2O_TID_SZ;
   BF                          reserved1:I2O_RESERVED_20BITS;
   BF                          IOP_ID:I2O_IOP_ID_SZ;
   BF                          reserved:I2O_RESERVED_4BITS;
   BF                          HostUnitID:I2O_UNIT_ID_SZ;
} I2O_EXEC_DDM_QUIESCE_MESSAGE, *PI2O_EXEC_DDM_QUIESCE_MESSAGE;


/****************************************************************************/

/* ExecDdmReset Function Message Frame structure. */

typedef struct _I2O_EXEC_DDM_RESET_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   BF                          DeviceTID:I2O_TID_SZ;
   BF                          reserved1:I2O_RESERVED_20BITS;
   BF                          IOP_ID:I2O_IOP_ID_SZ;
   BF                          reserved:I2O_RESERVED_4BITS;
   BF                          HostUnitID:I2O_UNIT_ID_SZ;
} I2O_EXEC_DDM_RESET_MESSAGE, *PI2O_EXEC_DDM_RESET_MESSAGE;


/****************************************************************************/

/* ExecDdmSuspend Function Message Frame structure. */

typedef struct _I2O_EXEC_DDM_SUSPEND_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   BF                          DdmTID:I2O_TID_SZ;
   BF                          reserved:I2O_RESERVED_20BITS;
} I2O_EXEC_DDM_SUSPEND_MESSAGE, *PI2O_EXEC_DDM_SUSPEND_MESSAGE;


/****************************************************************************/

#define    I2O_OPERATION_FLAG_ASSIGN_PERMANENT         0x01

/* ExecDeviceAssign Function Message Frame structure. */

typedef struct _I2O_EXEC_DEVICE_ASSIGN_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   BF                          DeviceTID:I2O_TID_SZ;
   BF                          DdmTID:I2O_TID_SZ;
   BF                          OperationFlags:I2O_8BIT_FLAGS_SZ;
   BF                          IOP_ID:I2O_IOP_ID_SZ;
   BF                          reserved:I2O_RESERVED_4BITS;
   BF                          HostUnitID:I2O_UNIT_ID_SZ;
} I2O_EXEC_DEVICE_ASSIGN_MESSAGE, *PI2O_EXEC_DEVICE_ASSIGN_MESSAGE;


/****************************************************************************/

#define    I2O_OPERATION_FLAG_RELEASE_PERMANENT        0x01

/* ExecDeviceRelease Function Message Frame structure. */

typedef struct _I2O_EXEC_DEVICE_RELEASE_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   BF                          DeviceTID:I2O_TID_SZ;
   BF                          DdmTID:I2O_TID_SZ;
   BF                          OperationFlags:I2O_8BIT_FLAGS_SZ;
   BF                          IOP_ID:I2O_IOP_ID_SZ;
   BF                          reserved:I2O_RESERVED_4BITS;
   BF                          HostUnitID:I2O_UNIT_ID_SZ;
} I2O_EXEC_DEVICE_RELEASE_MESSAGE, *PI2O_EXEC_DEVICE_RELEASE_MESSAGE;


/****************************************************************************/

/* HRT Entry Structure defined in I2OMSG.H */ 

/* ExecHrtGet Function Message Frame structure. */

typedef struct _I2O_EXEC_HRT_GET_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   I2O_SG_ELEMENT              SGL;
} I2O_EXEC_HRT_GET_MESSAGE, *PI2O_EXEC_HRT_GET_MESSAGE;


/****************************************************************************/


/* ExecIopClear Function Message Frame structure. */

typedef struct _I2O_EXEC_IOP_CLEAR_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
} I2O_EXEC_IOP_CLEAR_MESSAGE, *PI2O_EXEC_IOP_CLEAR_MESSAGE;


/****************************************************************************/


/* ExecIopConnect Function Message Frame structure. */

typedef struct _I2O_EXEC_IOP_CONNECT_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   BF                          reserved:I2O_RESERVED_24BITS;
   BF                          IOP1MsgerType:I2O_MESSENGER_TYPE_SZ;
   U16                         IOP1InboundMFrameSize;
   BF                          IOP1AliasForIOP2:I2O_TID_SZ;
   BF                          reserved1:I2O_RESERVED_4BITS;
   BF                          IOP_ID1:I2O_IOP_ID_SZ;
   BF                          reserved2:I2O_RESERVED_4BITS;
   BF                          HostUnitID1:I2O_UNIT_ID_SZ;
} I2O_EXEC_IOP_CONNECT_MESSAGE, *PI2O_EXEC_IOP_CONNECT_MESSAGE;


    /* ExecIopConnect reply structure */

typedef struct _I2O_EXEC_IOP_CONNECT_IOP_REPLY {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   U16                         DetailedStatusCode;
   U8                          reserved;
   U8                          ReqStatus;
   U16                         IOP2InboundMFrameSize;
   BF                          IOP2AliasForIOP1:I2O_TID_SZ;
   BF                          reserved1:I2O_RESERVED_4BITS;
   BF                          IOP_ID2:I2O_IOP_ID_SZ;
   BF                          reserved2:I2O_RESERVED_4BITS;
   BF                          HostUnitID2:I2O_UNIT_ID_SZ;
} I2O_EXEC_IOP_CONNECT_REPLY, *PI2O_EXEC_IOP_CONNECT_REPLY;


/****************************************************************************/


#define    I2O_EXEC_IOP_RESET_RESERVED_SZ              16

#define    I2O_EXEC_IOP_RESET_IN_PROGRESS              0x01
#define    I2O_EXEC_IOP_RESET_REJECTED                 0x02

#define    I2O_EXEC_IOP_RESET_STATUS_RESERVED_SZ       3

typedef struct _I2O_EXEC_IOP_RESET_STATUS {
   U8                          ResetStatus;
   U8                          reserved[I2O_EXEC_IOP_RESET_STATUS_RESERVED_SZ];
} I2O_EXEC_IOP_RESET_STATUS, *PI2O_EXEC_IOP_RESET_STATUS;


/* ExecIopReset Function Message Frame structure. */

typedef struct _I2O_EXEC_IOP_RESET_MESSAGE {
   U8                          VersionOffset;
   U8                          MsgFlags;
   U16                         MessageSize;
   BF                          TargetAddress:I2O_TID_SZ;
   BF                          InitiatorAddress:I2O_TID_SZ;
   BF                          Function:I2O_FUNCTION_SZ;
   U8                          Reserved[I2O_EXEC_IOP_RESET_RESERVED_SZ];
   U32                         StatusWordLowAddress;
   U32                         StatusWordHighAddress;
} I2O_EXEC_IOP_RESET_MESSAGE, *PI2O_EXEC_IOP_RESET_MESSAGE;


/****************************************************************************/

/* LCT Entry Structure defined in I2OMSG.H */ 

/* ExecLCTNotify Function Message Frame structure. */

typedef struct _I2O_EXEC_LCT_NOTIFY_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   U32                         ClassIdentifier;
   U32                         LastReportedChangeIndicator;
   I2O_SG_ELEMENT              SGL;
} I2O_EXEC_LCT_NOTIFY_MESSAGE, *PI2O_EXEC_LCT_NOTIFY_MESSAGE;


/****************************************************************************/


/* ExecOutboundInit Function Message Frame structure. */

typedef struct _I2O_EXEC_OUTBOUND_INIT_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   U32                         HostPageFrameSize;
   U8                          InitCode;
   U8                          reserved;
   U16                         OutboundMFrameSize;
   I2O_SG_ELEMENT              SGL;
} I2O_EXEC_OUTBOUND_INIT_MESSAGE, *PI2O_EXEC_OUTBOUND_INIT_MESSAGE;


#define    I2O_EXEC_OUTBOUND_INIT_IN_PROGRESS          0x01
#define    I2O_EXEC_OUTBOUND_INIT_REJECTED             0x02
#define    I2O_EXEC_OUTBOUND_INIT_FAILED               0x03
#define    I2O_EXEC_OUTBOUND_INIT_COMPLETE             0x04

#define    I2O_EXEC_OUTBOUND_INIT_RESERVED_SZ          3


typedef struct _I2O_EXEC_OUTBOUND_INIT_STATUS {
   U8                          InitStatus;
   U8                          reserved[I2O_EXEC_OUTBOUND_INIT_RESERVED_SZ];
} I2O_EXEC_OUTBOUND_INIT_STATUS, *PI2O_EXEC_OUTBOUND_INIT_STATUS;


typedef struct _I2O_EXEC_OUTBOUND_INIT_RECLAIM_LIST {
   U32                         MFACount;
   U32                         MFAReleaseCount;
   U32                         MFAAddress[1];
} I2O_EXEC_OUTBOUND_INIT_RECLAIM_LIST, *PI2O_EXEC_OUTBOUND_INIT_RECLAIM_LIST;


/****************************************************************************/

/* ExecPathEnable Function Message Frame structure. */

typedef struct _I2O_EXEC_PATH_ENABLE_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   BF                          IOP_ID:I2O_IOP_ID_SZ;
   BF                          reserved:I2O_RESERVED_4BITS;
   BF                          HostUnitID:I2O_UNIT_ID_SZ;
} I2O_EXEC_PATH_ENABLE_MESSAGE, *PI2O_EXEC_PATH_ENABLE_MESSAGE;


/****************************************************************************/

/* ExecPathQuiesce Function Message Frame structure. */

typedef struct _I2O_EXEC_PATH_QUIESCE_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   BF                          IOP_ID:I2O_IOP_ID_SZ;
   BF                          reserved:I2O_RESERVED_4BITS;
   BF                          HostUnitID:I2O_UNIT_ID_SZ;
} I2O_EXEC_PATH_QUIESCE_MESSAGE, *PI2O_EXEC_PATH_QUIESCE_MESSAGE;


/****************************************************************************/

/* ExecPathReset Function Message Frame structure. */

typedef struct _I2O_EXEC_PATH_RESET_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   BF                          IOP_ID:I2O_IOP_ID_SZ;
   BF                          reserved:I2O_RESERVED_4BITS;
   BF                          HostUnitID:I2O_UNIT_ID_SZ;
} I2O_EXEC_PATH_RESET_MESSAGE, *PI2O_EXEC_PATH_RESET_MESSAGE;


/****************************************************************************/

#define    I2O_EXEC_STATIC_MF_CREATE_RESERVED_SZ        3

/* ExecStaticMfCreate Message Frame  structure */

typedef struct _I2O_EXEC_STATIC_MF_CREATE_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   U8                          MaxOutstanding;
   U8                          reserved[I2O_EXEC_STATIC_MF_CREATE_RESERVED_SZ];
   I2O_MESSAGE_FRAME           StaticMessageFrame;
} I2O_EXEC_STATIC_MF_CREATE_MESSAGE, *PI2O_EXEC_STATIC_MF_CREATE_MESSAGE;


/* ExecStaticMfCreate Message Frame reply */

typedef struct _I2O_EXEC_STATIC_MF_CREATE_REPLY {
   I2O_SINGLE_REPLY_MESSAGE_FRAME  StdReplyFrame;
   PI2O_MESSAGE_FRAME              StaticMFA;
} I2O_EXEC_STATIC_MF_CREATE_REPLY, *PI2O_EXEC_STATIC_MF_CREATE_REPLY;


/* ExecStaticMfRelease Message Frame structure */

typedef struct _I2O_EXEC_STATIC_MF_RELEASE_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   PI2O_MESSAGE_FRAME          StaticMFA;
} I2O_EXEC_STATIC_MF_RELEASE_MESSAGE, *PI2O_EXEC_STATIC_MF_RELEASE_MESSAGE;


/****************************************************************************/

#define    I2O_EXEC_STATUS_GET_RESERVED_SZ             16

/* ExecStatusGet Function Message Frame structure. */

typedef struct _I2O_EXEC_STATUS_GET_MESSAGE {
   U8                          VersionOffset;
   U8                          MsgFlags;
   U16                         MessageSize;
   BF                          TargetAddress:I2O_TID_SZ;
   BF                          InitiatorAddress:I2O_TID_SZ;
   BF                          Function:I2O_FUNCTION_SZ;
   U8                          Reserved[I2O_EXEC_STATUS_GET_RESERVED_SZ];
   U32                         ReplyBufferAddressLow;
   U32                         ReplyBufferAddressHigh;
   U32                         ReplyBufferLength;
} I2O_EXEC_STATUS_GET_MESSAGE, *PI2O_EXEC_STATUS_GET_MESSAGE;


#define    I2O_IOP_STATUS_PROD_ID_STR_SZ               24
#define    I2O_EXEC_STATUS_GET_REPLY_RESERVED_SZ       6

/* ExecStatusGet reply Structure */

#define    I2O_IOP_CAP_CONTEXT_32_ONLY                 0x00000000
#define    I2O_IOP_CAP_CONTEXT_64_ONLY                 0x00000001
#define    I2O_IOP_CAP_CONTEXT_32_64_NOT_CURRENTLY     0x00000002
#define    I2O_IOP_CAP_CONTEXT_32_64_CURRENTLY         0x00000003
#define    I2O_IOP_CAP_CURRENT_CONTEXT_NOT_CONFIG      0x00000000
#define    I2O_IOP_CAP_CURRENT_CONTEXT_32_ONLY         0x00000004
#define    I2O_IOP_CAP_CURRENT_CONTEXT_64_ONLY         0x00000008
#define    I2O_IOP_CAP_CURRENT_CONTEXT_32_64           0x0000000C
#define    I2O_IOP_CAP_INBOUND_PEER_SUPPORT            0x00000010
#define    I2O_IOP_CAP_OUTBOUND_PEER_SUPPORT           0x00000020
#define    I2O_IOP_CAP_PEER_TO_PEER_SUPPORT            0x00000040

typedef struct _I2O_EXEC_STATUS_GET_REPLY {
   U16                         OrganizationID;
   U16                         reserved;
   BF                          IOP_ID:I2O_IOP_ID_SZ;
   BF                          reserved1:I2O_RESERVED_4BITS;
   BF                          HostUnitID:I2O_UNIT_ID_SZ;
   BF                          SegmentNumber:I2O_SEGMENT_NUMBER_SZ;
   BF                          I2oVersion:I2O_4BIT_VERSION_SZ;
   BF                          IopState:I2O_IOP_STATE_SZ;
   BF                          MessengerType:I2O_MESSENGER_TYPE_SZ;
   U16                         InboundMFrameSize;
   U8                          InitCode;
   U8                          reserved2;
   U32                         MaxInboundMFrames;
   U32                         CurrentInboundMFrames;
   U32                         MaxOutboundMFrames;
   U8                          ProductIDString[I2O_IOP_STATUS_PROD_ID_STR_SZ];
   U32                         ExpectedLCTSize;
   U32                         IopCapabilities;
   U32                         DesiredPrivateMemSize;
   U32                         CurrentPrivateMemSize;
   U32                         CurrentPrivateMemBase;
   U32                         DesiredPrivateIOSize;
   U32                         CurrentPrivateIOSize;
   U32                         CurrentPrivateIOBase;
   U8                          reserved3[3];
   U8                          SyncByte;
} I2O_EXEC_STATUS_GET_REPLY, *PI2O_EXEC_STATUS_GET_REPLY;


/****************************************************************************/

#define    I2O_EXEC_SW_DOWNLOAD_FLAG_LOAD_MEMORY       0x00
#define    I2O_EXEC_SW_DOWNLOAD_FLAG_PERMANENT_STORE   0x01
#define    I2O_EXEC_SW_DOWNLOAD_FLAG_EXPERIMENTAL      0x00
#define    I2O_EXEC_SW_DOWNLOAD_FLAG_OVERRIDE          0x02

#define    I2O_EXEC_SW_TYPE_DDM                        0x01
#define    I2O_EXEC_SW_TYPE_DDM_MPB                    0x02
#define    I2O_EXEC_SW_TYPE_DDM_CONFIG_TABLE           0x03
#define    I2O_EXEC_SW_TYPE_IRTOS                      0x11
#define    I2O_EXEC_SW_TYPE_IRTOS_PRIVATE_MODULE       0x12
#define    I2O_EXEC_SW_TYPE_IRTOS_DIALOG_TABLE         0x13
#define    I2O_EXEC_SW_TYPE_IOP_PRIVATE_MODULE         0x22
#define    I2O_EXEC_SW_TYPE_IOP_DIALOG_TABLE           0x23


/* I2O ExecSwDownload/Upload/Remove SwID Structure */

typedef struct _I2O_SW_ID {
   U16                                      ModuleID;
   U16                                      OrganizationID;
} I2O_SW_ID, *PI2O_SW_ID;


/* ExecSwDownload Function Message Frame structure. */

typedef struct _I2O_EXEC_SW_DOWNLOAD_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   U8                          CurrentFragment;
   U8                          TotalFragments;
   U8                          SwType;
   U8                          DownloadFlags;
   U32                         SWSize;
   I2O_SW_ID                   SwID;
   I2O_SG_ELEMENT              SGL;
} I2O_EXEC_SW_DOWNLOAD_MESSAGE, *PI2O_EXEC_SW_DOWNLOAD_MESSAGE;


/****************************************************************************/


/* ExecSwUpload Function Message Frame structure. */

typedef struct _I2O_EXEC_SW_UPLOAD_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   U8                          CurrentFragment;
   U8                          TotalFragments;
   U8                          SwType;
   U8                          UploadFlags;
   U32                         SWSize;
   I2O_SW_ID                   SwID;
   I2O_SG_ELEMENT              SGL;
} I2O_EXEC_SW_UPLOAD_MESSAGE, *PI2O_EXEC_SW_UPLOAD_MESSAGE;


/****************************************************************************/


/* ExecSwRemove Function Message Frame structure. */

typedef struct _I2O_EXEC_SW_REMOVE_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   U16                         reserved;
   U8                          SwType;
   U8                          RemoveFlags;
   U32                         SWSize;
   I2O_SW_ID                   SwID;
} I2O_EXEC_SW_REMOVE_MESSAGE, *PI2O_EXEC_SW_REMOVE_MESSAGE;


/****************************************************************************/


/* ExecSysEnable Function Message Frame structure. */

typedef struct _I2O_EXEC_SYS_ENABLE_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
} I2O_EXEC_SYS_ENABLE_MESSAGE, *PI2O_EXEC_SYS_ENABLE_MESSAGE;


/****************************************************************************/


/* ExecSysModify Function Message Frame structure. */

typedef struct _I2O_EXEC_SYS_MODIFY_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   I2O_SG_ELEMENT              SGL;
} I2O_EXEC_SYS_MODIFY_MESSAGE, *PI2O_EXEC_SYS_MODIFY_MESSAGE;


/****************************************************************************/


/* ExecSysQuiesce Function Message Frame structure. */

typedef struct _I2O_EXEC_SYS_QUIESCE_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
} I2O_EXEC_SYS_QUIESCE_MESSAGE, *PI2O_EXEC_SYS_QUIESCE_MESSAGE;


/****************************************************************************/


/* ExecSysTabSet (System Table) Function Message Frame structure. */

#define I2O_EXEC_SYS_TAB_IOP_ID_LOCAL_IOP           0x000
#define I2O_EXEC_SYS_TAB_IOP_ID_LOCAL_HOST          0x001
#define I2O_EXEC_SYS_TAB_IOP_ID_UNKNOWN_IOP         0xFFF
#define I2O_EXEC_SYS_TAB_HOST_UNIT_ID_LOCAL_UNIT    0x0000
#define I2O_EXEC_SYS_TAB_HOST_UNIT_ID_UNKNOWN_UNIT  0xffff
#define I2O_EXEC_SYS_TAB_SEG_NUMBER_LOCAL_SEGMENT   0x000
#define I2O_EXEC_SYS_TAB_SEG_NUMBER_UNKNOWN_SEGMENT 0xfff

typedef struct _I2O_EXEC_SYS_TAB_SET_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRANSACTION_CONTEXT     TransactionContext;
   BF                          IOP_ID:I2O_IOP_ID_SZ;
   BF                          reserved:I2O_RESERVED_4BITS;
   BF                          HostUnitID:I2O_UNIT_ID_SZ;
   BF                          SegmentNumber:I2O_SEGMENT_NUMBER_SZ;
   BF                          reserved2:I2O_RESERVED_20BITS;
   I2O_SG_ELEMENT              SGL;
} I2O_EXEC_SYS_TAB_SET_MESSAGE, *PI2O_EXEC_SYS_TAB_SET_MESSAGE;


/* ExecSysTabSet (System Table) Header Reply structure. */

#define    I2O_SET_SYSTAB_RESERVED_SZ                  8

typedef struct _I2O_SET_SYSTAB_HEADER {
   U8                          NumberEntries;
   U8                          SysTabVersion;
   U16                         reserved;
   U32                         CurrentChangeIndicator;
   U8                          reserved1[I2O_SET_SYSTAB_RESERVED_SZ];
/*    I2O_SYSTAB_ENTRY    SysTabEntry[1]; */
} I2O_SET_SYSTAB_HEADER, *PI2O_SET_SYSTAB_HEADER;


#define    I2O_RESOURCE_MANAGER_VERSION        0

typedef struct _MESSENGER_INFO {
   U32                         InboundMessagePortAddressLow;
   U32                         InboundMessagePortAddressHigh;
   } I2O_MESSENGER_INFO, *PI2O_MESSENGER_INFO;

/* ExecSysTabSet IOP Descriptor Entry structure. */

typedef struct _I2O_IOP_ENTRY {
   U16                         OrganizationID;
   U16                         reserved;
   BF                          IOP_ID:I2O_IOP_ID_SZ;
   BF                          reserved1:I2O_RESERVED_20BITS;
   BF                          SegmentNumber:I2O_SEGMENT_NUMBER_SZ;
   BF                          I2oVersion:I2O_4BIT_VERSION_SZ;
   BF                          IopState:I2O_IOP_STATE_SZ;
   BF                          MessengerType:I2O_MESSENGER_TYPE_SZ;
   U16                         InboundMessageFrameSize;
   U16                         reserved2;
   U32                         LastChanged;
   U32                         IopCapabilities;
   I2O_MESSENGER_INFO          MessengerInfo;              
} I2O_IOP_ENTRY, *PI2O_IOP_ENTRY;


/****************************************************************************/
/* Executive Parameter Groups */
/****************************************************************************/


#define    I2O_EXEC_IOP_HARDWARE_GROUP_NO              0x0000
#define    I2O_EXEC_IOP_MESSAGE_IF_GROUP_NO            0x0001
#define    I2O_EXEC_EXECUTING_ENVIRONMENT_GROUP_NO     0x0002
#define    I2O_EXEC_EXECUTING_DDM_LIST_GROUP_NO        0x0003
#define    I2O_EXEC_DRIVER_STORE_GROUP_NO              0x0004
#define    I2O_EXEC_DRIVER_STORE_TABLE_GROUP_NO        0x0005
#define    I2O_EXEC_IOP_BUS_ATTRIBUTES_GROUP_NO        0x0006
#define    I2O_EXEC_IOP_SW_ATTRIBUTES_GROUP_NO         0x0007
#define    I2O_EXEC_HARDWARE_RESOURCE_TABLE_GROUP_NO   0x0100
#define    I2O_EXEC_LCT_SCALAR_GROUP_NO                0x0101
#define    I2O_EXEC_LCT_TABLE_GROUP_NO                 0x0102
#define    I2O_EXEC_SYSTEM_TABLE_GROUP_NO              0x0103
#define    I2O_EXEC_EXTERNAL_CONN_TABLE_GROUP_NO       0x0104


/* EXEC Group 0000h - IOP Hardware Parameter Group */

/* IOP HardWare Capabilities defines */

#define    I2O_IOP_HW_CAP_SELF_BOOT                    0x00000001
#define    I2O_IOP_HW_CAP_IRTOS_UPGRADEABLE            0x00000002
#define    I2O_IOP_HW_CAP_DOWNLOADABLE_DDM             0x00000004
#define    I2O_IOP_HW_CAP_INSTALLABLE_DDM              0x00000008
#define    I2O_IOP_HW_CAP_BATTERY_BACKUP_RAM           0x00000010

/* IOP Processor Type defines */

#define    I2O_IOP_PROC_TYPE_INTEL_80960               0x00
#define    I2O_IOP_PROC_TYPE_AMD_29000                 0x01
#define    I2O_IOP_PROC_TYPE_MOTOROLA_68000            0x02
#define    I2O_IOP_PROC_TYPE_ARM                       0x03
#define    I2O_IOP_PROC_TYPE_MIPS                      0x04
#define    I2O_IOP_PROC_TYPE_SPARC                     0x05
#define    I2O_IOP_PROC_TYPE_POWER_PC                  0x06
#define    I2O_IOP_PROC_TYPE_ALPHA                     0x07
#define    I2O_IOP_PROC_TYPE_INTEL_X86                 0x08
#define    I2O_IOP_PROC_TYPE_OTHER                     0xFF


typedef struct _I2O_EXEC_IOP_HARDWARE_SCALAR {
   U16                         I2oVendorID;
   U16                         ProductID;
   U32                         ProcessorMemory;
   U32                         PermMemory;
   U32                         HWCapabilities;
   U8                          ProcessorType;
   U8                          ProcessorVersion;
} I2O_EXEC_IOP_HARDWARE_SCALAR, *PI2O_EXEC_IOP_HARDWARE_SCALAR;


/* EXEC Group 0001h - IOP Message Interface Parameter Group */

/* InitCode defines */
#define    I2O_MESSAGE_IF_INIT_CODE_NO_OWNER           0x00
#define    I2O_MESSAGE_IF_INIT_CODE_BIOS               0x10
#define    I2O_MESSAGE_IF_INIT_CODE_OEM_BIOS_EXTENSION 0x20
#define    I2O_MESSAGE_IF_INIT_CODE_ROM_BIOS_EXTENSION 0x30
#define    I2O_MESSAGE_IF_INIT_CODE_OS                 0x80

typedef struct _I2O_EXEC_IOP_MESSAGE_IF_SCALAR {
   U32                     InboundFrameSize;
   U32                     InboundSizeTarget;
   U32                     InboundMax;
   U32                     InboundTarget;
   U32                     InboundPoolCount;
   U32                     InboundCurrentFree;
   U32                     InboundCurrentPost;
   U16                     StaticCount;
   U16                     StaticInstanceCount;
   U16                     StaticLimit;
   U16                     StaticInstanceLimit;
   U32                     OutboundFrameSize;
   U32                     OutboundMax;
   U32                     OutboundMaxTarget;
   U32                     OutboundCurrentFree;
   U32                     OutboundCurrentPost;
   U8                      InitCode;
} I2O_EXEC_IOP_MESSAGE_IF_SCALAR, *PI2O_EXEC_IOP_MESSAGE_IF_SCALAR;


/* EXEC Group 0002h - Executing Environment Parameter Group */

typedef struct _I2O_EXEC_EXECUTE_ENVIRONMENT_SCALAR {
   U32                     MemTotal;
   U32                     MemFree;
   U32                     PageSize;
   U32                     EventQMax;
   U32                     EventQCurrent;
   U32                     DDMLoadMax;
} I2O_EXEC_EXECUTE_ENVIRONMENT_SCALAR, *PI2O_EXEC_EXECUTE_ENVIRONMENT_SCALAR;


/* EXEC Group 0003h - Executing DDM's Parameter Group */

/* ModuleType Defines */

#define    I2O_EXEC_DDM_MODULE_TYPE_OTHER              0x00
#define    I2O_EXEC_DDM_MODULE_TYPE_DOWNLOAD           0x01
#define    I2O_EXEC_DDM_MODULE_TYPE_EMBEDDED           0x22


typedef struct _I2O_EXEC_EXECUTE_DDM_TABLE {
   U16                     DdmTID;
   U8                      ModuleType;
   U8                      reserved;
   U16                     I2oVendorID;
   U16                     ModuleID;
   U8                      ModuleName[I2O_MODULE_NAME_SZ];
   U32                     ModuleVersion;
   U32                     DataSize;
   U32                     CodeSize;   
} I2O_EXEC_EXECUTE_DDM_TABLE, *PI2O_EXEC_EXECUTE_DDM_TABLE;


/* EXEC Group 0004h - Driver Store Environment Parameter Group */


typedef struct _I2O_EXEC_DRIVER_STORE_SCALAR {
   U32                     ModuleLimit;
   U32                     ModuleCount;
   U32                     CurrentSpace;
   U32                     FreeSpace;
} I2O_EXEC_DRIVER_STORE_SCALAR, *PI2O_EXEC_DRIVER_STORE_SCALAR;


/* EXEC Group 0005h - Driver Store Parameter Group */


typedef struct _I2O_EXEC_DRIVER_STORE_TABLE {
   U16                     StoredDdmIndex;
   U8                      ModuleType;
   U8                      reserved;
   U16                     I2oVendorID;
   U16                     ModuleID;
   U8                      ModuleName[I2O_MODULE_NAME_SZ];
   U32                     ModuleVersion;
   U16                     DateDay;
   U16                     DateMonth;
   U32                     DateYear;
   U32                     ModuleSize;
   U32                     MpbSize;
   U32                     ModuleFlags;
} I2O_EXEC_DRIVER_STORE_TABLE, *PI2O_EXEC_DRIVER_STORE_TABLE;


/* EXEC Group 0006h - IOP's Bus Attributes Parameter Group */

#define    I2O_EXEC_IOP_BUS_ATTRIB_SYSTEM_BUS          0x00
#define    I2O_EXEC_IOP_BUS_ATTRIB_BRIDGED_SYSTEM_BUS  0x01
#define    I2O_EXEC_IOP_BUS_ATTRIB_PRIVATE             0x02

typedef struct _I2O_EXEC_IOP_BUS_ATTRIBUTE_TABLE {
   U32                     BusID;
   U8                      BusType;
   U8                      MaxAdapters;
   U8                      AdapterCount;
   U8                      BusAttributes;
} I2O_EXEC_IOP_BUS_ATTRIBUTE_TABLE, *PI2O_EXEC_IOP_BUS_ATTRIBUTE_TABLE;


/* EXEC Group 0007h - IOP's Bus Attributes Parameter Group */

#define    I2O_EXEC_IOP_SW_CAP_IRTOS_I2O_COMPLIANT     0x00000001
#define    I2O_EXEC_IOP_SW_CAP_IRTOS_UPGRADEABLE       0x00000002
#define    I2O_EXEC_IOP_SW_CAP_DOWNLOADABLE_DDM        0x00000004
#define    I2O_EXEC_IOP_SW_CAP_INSTALLABLE_DDM         0x00000008

typedef struct _I2O_EXEC_IOP_SW_ATTRIBUTES_SCALAR {
   U16                     I2oVendorID;
   U16                     ProductID;
   U32                     CodeSize;
   U32                     SWCapabilities;
} I2O_EXEC_IOP_SW_ATTRIBUTES_SCALAR, *PI2O_EXEC_IOP_SW_ATTRIBUTES_SCALAR;


/* EXEC Group 0100h - Hardware Resource Table Parameter Group */

typedef struct _I2O_EXEC_HARDWARE_RESOURCE_TABLE {
   U32                         AdapterID;
   U16                         StateInfo;  /* AdapterState plus Local TID */
   U8                          BusNumber;
   U8                          BusType;
   U64                         PhysicalLocation;
   U32                         MemorySpace;
   U32                         IoSpace;
} I2O_EXEC_HARDWARE_RESOURCE_TABLE, *PI2O_EXEC_HARDWARE_RESOURCE_TABLE;

/* EXEC Group 0101h - Logical Configuration Table Scalar Parameter Group */

typedef struct _I2O_EXEC_LCT_SCALAR {
   U16                         BootDevice;
   U32                         IopFlags;
   U32                         CurrentChangeIndicator;
} I2O_EXEC_LCT_SCALAR, *PI2O_EXEC_LCT_SCALAR;

/* EXEC Group 0102h - Logical Configuration Table Parameter Group */

typedef struct _I2O_EXEC_LCT_TABLE {
   U16                         LocalTID;
   U16                         UserTID;
   U16                         ParentTID;
   U16                         DdmTID;
   U32                         ChangeIndicator;
   U32                         DeviceFlags;
   U32                         ClassID;
   U32                         SubClass;
   U8                          IdentityTag[I2O_IDENTITY_TAG_SZ];
   U32                         EventCapabilities;
   U8                          BiosInfo;
} I2O_EXEC_LCT_TABLE, *PI2O_EXEC_LCT_TABLE;

/* EXEC Group 0103h - System Table Parameter Group */

#define    I2O_MESSENGER_TYPE_MEMORY_MAPPED_MESSAGE_UNIT  0x0

typedef struct _I2O_EXEC_SYSTEM_TABLE {
   U16                         IOP_ID;
   U16                         OrganizationID;
   U16                         SegmentNumber;
   U8                          Version;
   U8                          IopState;
   U8                          MessengerType;
   U8                          reserved;
   U32                         InboundMessagePortAddress;
   U16                         InboundMessageFrameSize;
   U32                         IopCapabilities;
   I2O_MESSENGER_INFO          MessengerInfo;              
} I2O_EXEC_SYSTEM_TABLE, *PI2O_EXEC_SYSTEM_TABLE;


/* EXEC Group 0104h - External Connection Table Parameter Group */

#define  I2O_EXEC_XCT_FLAGS_REMOTE_IOP_CREATED_CONNECTION     0x00
#define  I2O_EXEC_XCT_FLAGS_THIS_IOP_CREATED_CONNECTION       0x01

typedef struct _I2O_EXEC_EXTERNAL_CONNECTION_TABLE {
   U16                         LocalAliasTID;
   U16                         RemoteTID;
   U16                         RemoteIOP;
   U16                         RemoteUnitID;
   U8                          Flags;
   U8                          reserved;
} I2O_EXEC_EXTERNAL_CONNECTION_TABLE, *PI2O_EXEC_EXTERNAL_CONNECTION_TABLE;


/****************************************************************************/

#pragma pack(pop)

PRAGMA_ALIGN_POP

#endif        /* I2O_EXECUTIVE_HDR */

