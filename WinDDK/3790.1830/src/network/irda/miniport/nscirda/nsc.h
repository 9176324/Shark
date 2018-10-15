/*
 ************************************************************************
 *
 *	NSC.h
 *
 *
 * Portions Copyright (C) 1996-1998 National Semiconductor Corp.
 * All rights reserved.
 * Copyright (C) 1996-1998 Microsoft Corporation. All Rights Reserved.
 *
 *
 *
 *************************************************************************
 */



#ifndef NSC_H

#define NSC_H

#include <ndis.h>
#include "dmautil.h"
#include <ntddndis.h>  // defines OID's

#include "settings.h"
#include "comm.h"
#include "sync.h"
#include "newdong.h"

#define NSC_MAJOR_VERSION 1
#define NSC_MINOR_VERSION 11
#define NSC_LETTER_VERSION 's'
#define NDIS_MAJOR_VERSION 5
#define NDIS_MINOR_VERSION 0

extern ULONG DebugSpeed;


#define TIMER_RESOLTION_IN_US (125)

#define TIMER_PERIODS         (10)

#define RECEIVE_TIMEOUT       (TIMER_RESOLTION_IN_US * TIMER_PERIODS)
//
// Registry Keywords.
//
#define CARDTYPE	    NDIS_STRING_CONST("BoardType")
#define DONGLE_A_TYPE	NDIS_STRING_CONST("Dongle_A_Type")
#define DONGLE_B_TYPE	NDIS_STRING_CONST("Dongle_B_Type")
#define MAXCONNECTRATE  NDIS_STRING_CONST("MaxConnectRate")


//
//  bank 0
//
#define BANK_0           (0)
#define TXD_RXD_OFFSET   (0)
#define LCR_BSR_OFFSET   (3)
#define LSR_OFFSET       (5)
#define ASCR_OFFSET      (7)

//
//  bank 2
//
#define BANK_2           (2)
#define TXFLV_OFFSET     (6)

#define BANK_4           (4)
#define TFRCCL_OFFSET    (4)
#define TFRCCH_OFFSET    (4)

#define BKSE             (1 << 7)

#define FRM_ST            (5)
#define RFRL_L            (6)
#define RFRL_H            (7)

#define ST_FIFO_LOST_FR   (1 << 6)
#define ST_FIFO_VALID     (1 << 7)


#define LSR_OE            (1 << 1)
#define LSR_TXRDY         (1 << 5)
#define LSR_TXEMP         (1 << 6)
#define LSR_FR_END        (1 << 7)

#define ASCR_TXUR         (1 << 6)


//
// Valid value ranges for the DMA Channels.
//
    #define VALID_DMACHANNELS {0xFF,0x0,0x1,0x3}

    #define FIR_INT_MASK 0x14
//#define FIR_INT_MASK 0x50

enum NSC_EXT_INTS {
    RXHDL_EV    = (1 << 0),
    TXLDL_EV    = (1 << 1),
    LS_EV       = (1 << 2),
    MS_EV       = (1 << 3),
    DMA_EV      = (1 << 4),
    TXEMP_EV    = (1 << 5),
    SFIF_EV     = (1 << 6),
    TMR_EV      = (1 << 7)
};


typedef struct DebugCounters {
    ULONG TxPacketsStarted;
    ULONG TxPacketsCompleted;
    ULONG ReceivedPackets;
    ULONG WindowSize;
    ULONG StatusFIFOOverflows;
    ULONG TxUnderruns;
    ULONG ReceiveFIFOOverflows;
    ULONG MissedPackets;
    ULONG ReceiveCRCErrors;
    ULONG ReturnPacketHandlerCalled;
    ULONG RxWindow;
    ULONG RxWindowMax;
    ULONG RxDPC_Window;
    ULONG RxDPC_WindowMax;
    ULONG RxDPC_G1_Count;
} DebugCounters;

/*
 *  A receive buffer is either FREE (not holding anything) FULL
 * (holding undelivered data) or PENDING (holding data delivered
 * asynchronously)
 */
typedef enum rcvbufferStates {
    STATE_FREE,
    STATE_FULL,
    STATE_PENDING
} rcvBufferState;

typedef struct {
    LIST_ENTRY listEntry;
    rcvBufferState state;
    PNDIS_PACKET packet;
    UINT dataLen;
    PUCHAR dataBuf;
    BOOLEAN isDmaBuf;
} rcvBuffer;


typedef struct _NSC_DMA_BUFFER_INFO {

    NDIS_HANDLE             AdapterHandle;
    ULONG                   Length;
    PVOID                   VirtualAddress;
    NDIS_PHYSICAL_ADDRESS   PhysicalAddress;
    BOOLEAN                 SharedAllocation;

} NSC_DMA_BUFFER_INFO, *PNSC_DMA_BUFFER_INFO;

typedef struct IrDevice {
    /*
     * This is the handle that the NDIS wrapper associates with a
     * connection.  The handle that the miniport driver associates with
     * the connection is just an index into the devStates array).
     */
    NDIS_HANDLE ndisAdapterHandle;

    int CardType;

    /*
     *  Current speed setting, in bits/sec.
     *  (Note: this is updated when we ACTUALLY change the speed,
     *         not when we get the request to change speed via
     *         MiniportSetInformation).
     */
    UINT currentSpeed;

    // Current dongle setting, 0 for dongle A, 1 for dongle B
    // and so on.
    //
    UCHAR DonglesSupported;
    UCHAR currentDongle;
    UCHAR DongleTypes[2];

    UIR      IrDongleResource;
    DongleParam  Dingle[2];

    UINT AllowedSpeedMask;
    /*
     *  This structure holds information about our ISR.
     *  It is used to synchronize with the ISR.
     */
    BOOLEAN                 InterruptRegistered;
    NDIS_MINIPORT_INTERRUPT interruptObj;


    /*
     *  Circular queue of pending receive buffers
     */
#define NUM_RCV_BUFS 16
//    #define NEXT_RCV_BUF_INDEX(i) (((i)==NO_BUF_INDEX) ? 0 : (((i)+1)%NUM_RCV_BUFS))
    LIST_ENTRY rcvBufBuf;       // Protected by SyncWithInterrupt
    LIST_ENTRY rcvBufFree;      // Protected by SyncWithInterrupt
    LIST_ENTRY rcvBufFull;      // Protected by SyncWithInterrupt
    LIST_ENTRY rcvBufPend;      // Protected by QueueLock



    NDIS_SPIN_LOCK QueueLock;
    LIST_ENTRY SendQueue;
    PNDIS_PACKET CurrentPacket;
    BOOLEAN      FirTransmitPending;
    BOOLEAN      FirReceiveDmaActive;
    BOOLEAN      TransmitIsIdle;
    BOOLEAN      Halting;

    BOOLEAN      TestingInterrupt;
    volatile BOOLEAN      GotTestInterrupt;

    LONG         PacketsSentToProtocol;

    NDIS_EVENT   ReceiveStopped;
    NDIS_EVENT   SendStoppedOnHalt;

    /*
     *  Handle to NDIS packet pool, from which packets are
     *  allocated.
     */
    NDIS_HANDLE packetPoolHandle;
    NDIS_HANDLE bufferPoolHandle;


    /*
     * mediaBusy is set TRUE any time that this miniport driver moves a
     * data frame.  It can be reset by the protocol via
     * MiniportSetInformation and later checked via
     * MiniportQueryInformation to detect interleaving activity.
     */
    LONG    RxInterrupts;
    BOOLEAN mediaBusy;
    BOOLEAN haveIndicatedMediaBusy;

    /*
     * nowReceiving is set while we are receiving a frame.
     * It (not mediaBusy) is returned to the protocol when the protocol
     * queries OID_MEDIA_BUSY
     */
    BOOLEAN nowReceiving;


    //
    // Interrupt Mask.
    //
    UCHAR FirIntMask;

    UCHAR LineStatus;
    UCHAR InterruptMask;
    UCHAR InterruptStatus;
    UCHAR AuxStatus;

    BOOLEAN    ForceTurnAroundTimeout;
#if DBG
    BOOLEAN    WaitingForTurnAroundTimer;

#endif

    /*
     *  Current link speed information.
     */
    const baudRateInfo *linkSpeedInfo;

    /*
     *  When speed is changed, we have to clear the send queue before
     *  setting the new speed on the hardware.
     *  These vars let us remember to do it.
     */
    PNDIS_PACKET lastPacketAtOldSpeed;
    BOOLEAN setSpeedAfterCurrentSendPacket;

    /*
     *  Information on the COM port and send/receive FSM's.
     */
    comPortInfo portInfo;

    UINT hardwareStatus;

    /*
     *  UIR Module ID.
     */
    int UIR_ModuleId;

    /*
     *  Maintain statistical debug info.
     */
    UINT packetsRcvd;
    UINT packetsDropped;
    UINT packetsSent;
    UINT interruptCount;


    /*
     *  DMA handles
     */
    NDIS_HANDLE DmaHandle;
    NDIS_HANDLE dmaBufferPoolHandle;
    PNDIS_BUFFER rcvDmaBuffer;
    PUCHAR    dmaReadBuf;
    ULONG_PTR rcvDmaOffset;
    ULONG_PTR rcvDmaSize;
    ULONG_PTR rcvPktOffset;
    ULONG_PTR LastReadDMACount;

    PNDIS_BUFFER xmitDmaBuffer;
//    PUCHAR       xmitDmaBufferVirtualAddress;

    NDIS_TIMER TurnaroundTimer;

    ULONG HangChk;

    BOOLEAN DiscardNextPacketSet;

    DMA_UTIL    DmaUtil;

    NSC_DMA_BUFFER_INFO    ReceiveDmaBufferInfo;
    NSC_DMA_BUFFER_INFO    TransmitDmaBufferInfo;
    PUCHAR                 TransmitDmaBuffer;
    ULONG                  TransmitDmaLength;


} IrDevice;

/*
 *  We use a pointer to the IrDevice structure as the miniport's device context.
 */
    #define CONTEXT_TO_DEV(__deviceContext) ((IrDevice *)(__deviceContext))
    #define DEV_TO_CONTEXT(__irdev) ((NDIS_HANDLE)(__irdev))

    #define ON  TRUE
    #define OFF FALSE

    #include "externs.h"


VOID
SyncWriteBankReg(
    PNDIS_MINIPORT_INTERRUPT InterruptObject,
    PUCHAR                   PortBase,
    UINT                     BankNumber,
    UINT                     RegisterIndex,
    UCHAR                    Value
    );

UCHAR
SyncReadBankReg(
    PNDIS_MINIPORT_INTERRUPT InterruptObject,
    PUCHAR                   PortBase,
    UINT                     BankNumber,
    UINT                     RegisterIndex
    );

VOID
SyncSetInterruptMask(
    IrDevice *thisDev,
    BOOLEAN enable
    );

BOOLEAN
SyncGetFifoStatus(
    PNDIS_MINIPORT_INTERRUPT InterruptObject,
    PUCHAR                   PortBase,
    PUCHAR                   Status,
    PULONG                   Size
    );


VOID
ProcessSendQueue(
    IrDevice *thisDev
    );

PVOID
NscAllocateDmaBuffer(
    NDIS_HANDLE     AdapterHandle,
    ULONG           Size,
    PNSC_DMA_BUFFER_INFO    DmaBufferInfo
    );

VOID
NscFreeDmaBuffer(
    PNSC_DMA_BUFFER_INFO    DmaBufferInfo
    );



#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(CtlGuid,(57AC2B71,75CD,4043,9B00,B51674B0DC71), \
       WPP_DEFINE_BIT(DBG_LOG_ERROR)               \
       WPP_DEFINE_BIT(DBG_LOG_INFO)                \
       WPP_DEFINE_BIT(DBG_LOG_FIR)                 \
       WPP_DEFINE_BIT(DBG_LOG_SIR)                 \
       )

#endif NSC_H

