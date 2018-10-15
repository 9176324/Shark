/*++

Copyright (c) 1998-2002  Microsoft Corporation

Module Name:

    i2outil.h

Abstract:

    I2O Utility Class Message defintion file
    This file contains information presented in Chapter 6 of the I2O
    Specification.

Author:

Environment:

    kernel mode only

Revision History:
 
--*/


#if !defined(I2O_UTILITY_HDR)
#define I2O_UTILITY_HDR

#define I2OUTIL_REV 1_5_5  /* I2OUtil header file revision string */

#include   "i2omsg.h"      /* Include the Base Message file */


PRAGMA_ALIGN_PUSH

#pragma pack(push,1)

/* Utility Message class functions. */

#define    I2O_UTIL_NOP                                0x00
#define    I2O_UTIL_ABORT                              0x01
#define    I2O_UTIL_CLAIM                              0x09
#define    I2O_UTIL_CLAIM_RELEASE                      0x0B
#define    I2O_UTIL_CONFIG_DIALOG                      0x10
#define    I2O_UTIL_DEVICE_RESERVE                     0x0D
#define    I2O_UTIL_DEVICE_RELEASE                     0x0F
#define    I2O_UTIL_EVENT_ACKNOWLEDGE                  0x14
#define    I2O_UTIL_EVENT_REGISTER                     0x13
#define    I2O_UTIL_LOCK                               0x17
#define    I2O_UTIL_LOCK_RELEASE                       0x19
#define    I2O_UTIL_PARAMS_GET                         0x06
#define    I2O_UTIL_PARAMS_SET                         0x05
#define    I2O_UTIL_REPLY_FAULT_NOTIFY                 0x15

/****************************************************************************/

/* ABORT Abort type defines. */

#define    I2O_ABORT_TYPE_EXACT_ABORT                  0x00
#define    I2O_ABORT_TYPE_FUNCTION_ABORT               0x01
#define    I2O_ABORT_TYPE_TRANSACTION_ABORT            0x02
#define    I2O_ABORT_TYPE_WILD_ABORT                   0x03
#define    I2O_ABORT_TYPE_CLEAN_EXACT_ABORT            0x04
#define    I2O_ABORT_TYPE_CLEAN_FUNCTION_ABORT         0x05
#define    I2O_ABORT_TYPE_CLEAN_TRANSACTION_ABORT      0x06
#define    I2O_ABORT_TYPE_CLEAN_WILD_ABORT             0x07

/* UtilAbort Function Message Frame structure. */

typedef struct _I2O_UTIL_ABORT_MESSAGE {
    I2O_MESSAGE_FRAME          StdMessageFrame;
    I2O_TRANSACTION_CONTEXT    TransactionContext;
    U16                        reserved;
    U8                         AbortType;
    U8                         FunctionToAbort;
    I2O_TRANSACTION_CONTEXT    TransactionContextToAbort;
} I2O_UTIL_ABORT_MESSAGE, *PI2O_UTIL_ABORT_MESSAGE;


typedef struct _I2O_UTIL_ABORT_REPLY {
    I2O_MESSAGE_FRAME          StdMessageFrame;
    I2O_TRANSACTION_CONTEXT    TransactionContext;
    U32                        CountOfAbortedMessages;
} I2O_UTIL_ABORT_REPLY, *PI2O_UTIL_ABORT_REPLY;


/****************************************************************************/

/* Claim Flag defines */

#define    I2O_CLAIM_FLAGS_EXCLUSIVE                   0x0001 /* Reserved */
#define    I2O_CLAIM_FLAGS_RESET_SENSITIVE             0x0002
#define    I2O_CLAIM_FLAGS_STATE_SENSITIVE             0x0004
#define    I2O_CLAIM_FLAGS_CAPACITY_SENSITIVE          0x0008
#define    I2O_CLAIM_FLAGS_PEER_SERVICE_DISABLED       0x0010
#define    I2O_CLAIM_FLAGS_MGMT_SERVICE_DISABLED       0x0020

/* Claim Type defines */

#define    I2O_CLAIM_TYPE_PRIMARY_USER                 0x01
#define    I2O_CLAIM_TYPE_AUTHORIZED_USER              0x02
#define    I2O_CLAIM_TYPE_SECONDARY_USER               0x03
#define    I2O_CLAIM_TYPE_MANAGEMENT_USER              0x04

/* UtilClaim Function Message Frame structure. */

typedef struct _I2O_UTIL_CLAIM_MESSAGE {
    I2O_MESSAGE_FRAME          StdMessageFrame;
    I2O_TRANSACTION_CONTEXT    TransactionContext;
    U16                        ClaimFlags;
    U8                         reserved;
    U8                         ClaimType;
} I2O_UTIL_CLAIM_MESSAGE, *PI2O_UTIL_CLAIM_MESSAGE;


/****************************************************************************/

/* Claim Release Flag defines */

#define    I2O_RELEASE_FLAGS_CONDITIONAL               0x0001

/* UtilClaimRelease Function Message Frame structure. */

typedef struct _I2O_UTIL_CLAIM_RELEASE_MESSAGE {
    I2O_MESSAGE_FRAME          StdMessageFrame;
    I2O_TRANSACTION_CONTEXT    TransactionContext;
    U16                        ReleaseFlags;
    U8                         reserved;
    U8                         ClaimType;
} I2O_UTIL_CLAIM_RELEASE_MESSAGE, *PI2O_UTIL_CLAIM_RELEASE_MESSAGE;


/****************************************************************************/

/*  UtilConfigDialog Function Message Frame structure */

typedef struct _I2O_UTIL_CONFIG_DIALOG_MESSAGE {
    I2O_MESSAGE_FRAME          StdMessageFrame;
    I2O_TRANSACTION_CONTEXT    TransactionContext;
    U32                        PageNumber;
    I2O_SG_ELEMENT             SGL;
} I2O_UTIL_CONFIG_DIALOG_MESSAGE, *PI2O_UTIL_CONFIG_DIALOG_MESSAGE;


/****************************************************************************/

/*  Event Acknowledge Function Message Frame structure */

typedef struct _I2O_UTIL_EVENT_ACK_MESSAGE {
    I2O_MESSAGE_FRAME          StdMessageFrame;
    I2O_TRANSACTION_CONTEXT    TransactionContext;
    U32                        EventIndicator;
    U32                        EventData[1];
} I2O_UTIL_EVENT_ACK_MESSAGE, *PI2O_UTIL_EVENT_ACK_MESSAGE;

/* Event Ack Reply structure */

typedef struct _I2O_UTIL_EVENT_ACK_REPLY {
    I2O_MESSAGE_FRAME          StdMessageFrame;
    I2O_TRANSACTION_CONTEXT    TransactionContext;
    U32                        EventIndicator;
    U32                        EventData[1];
} I2O_UTIL_EVENT_ACK_REPLY, *PI2O_UTIL_EVENT_ACK_REPLY;


/****************************************************************************/

/* Event Indicator Mask Flags */

#define    I2O_EVENT_IND_STATE_CHANGE                  0x80000000
#define    I2O_EVENT_IND_GENERAL_WARNING               0x40000000
#define    I2O_EVENT_IND_CONFIGURATION_FLAG            0x20000000
/* #define    I2O_EVENT_IND_RESERVE_RELEASE               0x10000000 */
#define    I2O_EVENT_IND_LOCK_RELEASE                  0x10000000
#define    I2O_EVENT_IND_CAPABILITY_CHANGE             0x08000000
#define    I2O_EVENT_IND_DEVICE_RESET                  0x04000000
#define    I2O_EVENT_IND_EVENT_MASK_MODIFIED           0x02000000
#define    I2O_EVENT_IND_FIELD_MODIFIED                0x01000000
#define    I2O_EVENT_IND_VENDOR_EVENT                  0x00800000
#define    I2O_EVENT_IND_DEVICE_STATE                  0x00400000

/* Event Data for generic Events */

#define    I2O_EVENT_STATE_CHANGE_NORMAL               0x00
#define    I2O_EVENT_STATE_CHANGE_SUSPENDED            0x01
#define    I2O_EVENT_STATE_CHANGE_RESTART              0x02
#define    I2O_EVENT_STATE_CHANGE_NA_RECOVER           0x03
#define    I2O_EVENT_STATE_CHANGE_NA_NO_RECOVER        0x04
#define    I2O_EVENT_STATE_CHANGE_QUIESCE_REQUEST      0x05
#define    I2O_EVENT_STATE_CHANGE_FAILED               0x10
#define    I2O_EVENT_STATE_CHANGE_FAULTED              0x11

#define    I2O_EVENT_GEN_WARNING_NORMAL                0x00
#define    I2O_EVENT_GEN_WARNING_ERROR_THRESHOLD       0x01
#define    I2O_EVENT_GEN_WARNING_MEDIA_FAULT           0x02

#define    I2O_EVENT_CAPABILITY_OTHER                  0x01
#define    I2O_EVENT_CAPABILITY_CHANGED                0x02

#define    I2O_EVENT_SENSOR_STATE_CHANGED              0x01


/*  UtilEventRegister Function Message Frame structure */

typedef struct _I2O_UTIL_EVENT_REGISTER_MESSAGE {
    I2O_MESSAGE_FRAME          StdMessageFrame;
    I2O_TRANSACTION_CONTEXT    TransactionContext;
    U32                        EventMask;
} I2O_UTIL_EVENT_REGISTER_MESSAGE, *PI2O_UTIL_EVENT_REGISTER_MESSAGE;

/* UtilEventRegister Reply structure */

typedef struct _I2O_UTIL_EVENT_REGISTER_REPLY {
    I2O_MESSAGE_FRAME          StdMessageFrame;
    I2O_TRANSACTION_CONTEXT    TransactionContext;
    U32                        EventIndicator;
    U32                        EventData[1];
} I2O_UTIL_EVENT_REGISTER_REPLY, *PI2O_UTIL_EVENT_REGISTER_REPLY;


/****************************************************************************/

/* UtilLock Function Message Frame structure. */

typedef struct _I2O_UTIL_LOCK_MESSAGE {
    I2O_MESSAGE_FRAME          StdMessageFrame;
    I2O_TRANSACTION_CONTEXT    TransactionContext;
} I2O_UTIL_LOCK_MESSAGE, *PI2O_UTIL_LOCK_MESSAGE;

/****************************************************************************/

/* UtilLockRelease Function Message Frame structure. */

typedef struct _I2O_UTIL_LOCK_RELEASE_MESSAGE {
    I2O_MESSAGE_FRAME          StdMessageFrame;
    I2O_TRANSACTION_CONTEXT    TransactionContext;
} I2O_UTIL_LOCK_RELEASE_MESSAGE, *PI2O_UTIL_LOCK_RELEASE_MESSAGE;


/****************************************************************************/

/* UtilNOP Function Message Frame structure. */

typedef struct _I2O_UTIL_NOP_MESSAGE {
    I2O_MESSAGE_FRAME          StdMessageFrame;
} I2O_UTIL_NOP_MESSAGE, *PI2O_UTIL_NOP_MESSAGE;


/****************************************************************************/

/* UtilParamsGet Message Frame structure. */

typedef struct _I2O_UTIL_PARAMS_GET_MESSAGE {
    I2O_MESSAGE_FRAME          StdMessageFrame;
    I2O_TRANSACTION_CONTEXT    TransactionContext;
    U32                        OperationFlags;
    I2O_SG_ELEMENT             SGL;
} I2O_UTIL_PARAMS_GET_MESSAGE, *PI2O_UTIL_PARAMS_GET_MESSAGE;


/****************************************************************************/

/* UtilParamsSet Message Frame structure. */

typedef struct _I2O_UTIL_PARAMS_SET_MESSAGE {
    I2O_MESSAGE_FRAME          StdMessageFrame;
    I2O_TRANSACTION_CONTEXT    TransactionContext;
    U32                        OperationFlags;
    I2O_SG_ELEMENT             SGL;
} I2O_UTIL_PARAMS_SET_MESSAGE, *PI2O_UTIL_PARAMS_SET_MESSAGE;


/****************************************************************************/

/* UtilReplyFaultNotify Message for Message Failure. */

typedef struct _I2O_UTIL_REPLY_FAULT_NOTIFY_MESSAGE {
    I2O_MESSAGE_FRAME          StdMessageFrame;
    I2O_TRANSACTION_CONTEXT    TransactionContext;
    U8                         LowestVersion;
    U8                         HighestVersion;
    U8                         Severity;
    U8                         FailureCode;
    BF                         FailingIOP_ID:I2O_IOP_ID_SZ;
    BF                         reserved:I2O_RESERVED_4BITS;
    BF                         FailingHostUnitID:I2O_UNIT_ID_SZ;
    U32                        AgeLimit;
#if I2O_64BIT_CONTEXT
    PI2O_MESSAGE_FRAME         OriginalMFA;
#else
    PI2O_MESSAGE_FRAME         OriginalMFALowPart;
    U32                        OriginalMFAHighPart;  /* Always 0000 */
#endif
} I2O_UTIL_REPLY_FAULT_NOTIFY_MESSAGE, *PI2O_UTIL_REPLY_FAULT_NOTIFY_MESSAGE;


/****************************************************************************/

/* Device Reserve Function Message Frame structure. */
/* NOTE:  This was previously called the Reserve Message */

typedef struct _I2O_UTIL_DEVICE_RESERVE_MESSAGE {
    I2O_MESSAGE_FRAME          StdMessageFrame;
    I2O_TRANSACTION_CONTEXT    TransactionContext;
} I2O_UTIL_DEVICE_RESERVE_MESSAGE, *PI2O_UTIL_DEVICE_RESERVE_MESSAGE;


/****************************************************************************/

/* Device Release Function Message Frame structure. */
/* NOTE:  This was previously called the ReserveRelease Message */

typedef struct _I2O_UTIL_DEVICE_RELEASE_MESSAGE {
    I2O_MESSAGE_FRAME          StdMessageFrame;
    I2O_TRANSACTION_CONTEXT    TransactionContext;
} I2O_UTIL_DEVICE_RELEASE_MESSAGE, *PI2O_UTIL_DEVICE_RELEASE_MESSAGE;


/****************************************************************************/

#pragma pack(pop)
PRAGMA_ALIGN_POP

#endif    /* I2O_UTILITY_HDR  */

