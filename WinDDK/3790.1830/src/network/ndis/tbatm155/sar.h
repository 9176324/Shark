/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       sar.h
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920


Abstract:

Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	01/07/97		awang		Initial of Toshiba ATM 155 Device Driver.

--*/

#ifndef __SAR_H
#define __SAR_H



//
//	TBATM155_SAR
//
//	Description:
//     This data structure defines the Toshiba ATM 155 ASIC register set.
//     For a full description consult the Toshiba PCI ATM 155 ASIC
//     Specification.
//     If you look carefully, the member names in this structure match
//     the names used in the document.
//
//	Elements:
//     Tx_Pending_Slots
//     Rx_Pending_Slots
//     SAR_Cntrl1          
//     SAR_Cntrl2          
//     Intr_Status			
//     Intr_Enb			
//     Intr_HldoffEnb		
//     Tx_FS_List_Ptrs
//     Tx_Report_Base
//     Rx_Report_Base
//     Tx_Report_Ptr
//     Rx_Report_Ptr
//     Rx_Slot_Cong_Th
//     Tx_ABR_ADTF
//     Tx_ABR_Nrm_Trm
//     PeepHole_Cmd
//     PeepHole_Data
//     Rx_Raw_Slot_Tag
//     Rx_CellCnt
//     Rx_AAL5_DropPktCnt
//     Rx_Raw_DropPktCnt
//     Tx_CellCnt
//     Real_Time
//     Current_Time
//     Test_Cntrl
//
//
//	Note:
//     The Toshiba 155 PCI ATM is a 32-bit only device. Make sure that
//     all accesses to the registers are 32-bit accesses. The adapter will
//     assist you in ensuring this by forcing bus errors if you attmept
//     anything other than 32-bit accesses.
//     the structures that come before the TBATM155_SAR are defined as
//     a union of bit fields and a ULONG, this is so that the register can
//     be easily constructed and then copied in a single 32-bit operation
//     to the hardware register.
//
typedef union  _TX_PENDING_SLOTS_CTL_REG
{
   struct
   {
       ULONG   Slot_Size:14;         // the least significant bit
       ULONG   EOP:1;
       ULONG   CRC10_Enb:1;
       ULONG   VC:12;
       ULONG   MGMT:1;
       ULONG   Idle:1;
       ULONG   Raw:1;
       ULONG   reserved2:1;
   };

   ULONG   reg;
}
   TX_PENDING_SLOTS_CTL,
   *PTX_PENDING_SLOTS_CTL;


typedef	union	_RX_PENDING_SLOTS_CTL_REG
{
   struct
   {
       ULONG   VC:12;              // the least significant bit
       ULONG   reserved1:4;
       ULONG   Slot_Type:2;
       ULONG   Slot_Tag:12;        // Range from 0 to 0x0FFF
       ULONG   rqSlotType:2;       // Set slot type to be posted to Rx report queue.
   };

   ULONG   reg;
}
   RX_PENDING_SLOTS_CTL,
   *PRX_PENDING_SLOTS_CTL;


typedef union  _TB55PCISAR_CNTRL1_REG
{
   struct
   {
       ULONG   Rx_Enb:1;                // the least significant bit
       ULONG   Phy_GFC_A_Enb:1;
       ULONG   Phy_GFC_Enb:1;
       ULONG   Phy_Loopback_Enb:1;
       ULONG   Phy_Int_High:1;
       ULONG   GFC_Signal_Detect:1;     // ((for 4K VC Meteor Only))
       ULONG   Phy_Reset:1;
       ULONG   Phy_Cell_Mode_Enb:1;
       ULONG   Phy_IF_Reset:1;          // Phy_GFC_Override (for 1K VC meteor)
       ULONG   Tx_Enb:1;
       ULONG   VPI_VCI_Mapping:3;
       ULONG   VC_Mode_Sel:2;
       ULONG   I64K_Mode_Enb:1;
       ULONG   DMA_BSwap_Literals:1;
       ULONG   DMA_BSwap_Data:1;
       ULONG   FM_Enb:1;
       ULONG   FM_Resynch:1;            // Write-Only
       ULONG   FM_RM_Enb:1;
       ULONG   CBR_ST_Sel:1;
       ULONG   SW_ABR_Enb:1;
       ULONG   PCI_MRM_Dis:1;
       ULONG   PCI_MRM_Sel:1;
       ULONG   Rev_UTOPIA:1;            // ((for 4K VC Meteor Only))
       ULONG   reserved2:6;
   };

   ULONG   reg;
}
   TB155PCISAR_CNTRL1,
   *PTB155PCISAR_CNTRL1;


#define TBATM155_CTRL2_MIN_SMALL_BSIZE     16      // 16   LW  (64 Bytes)
#define TBATM155_CTRL2_MAX_SMALL_BSIZE     512     // 512  LW  (2K Bytes)

typedef union  _TB55PCISAR_CNTRL2_REG
{
   struct
   {
       ULONG   Soft_Reset:1;              // the least significant bit
       ULONG   EFCI_Or_Enb:1;
       ULONG   Rx_AAL5_16K_Lmt:1;
       ULONG   Rx_Bad_RM_DMA_Enb:1;
       ULONG   Rx_RATO_Time:4;
       ULONG   Small_Slot_Size:9;
       ULONG   Big_Slot_Size:3;
       ULONG   Raw_Slot_Low_Th:2;
       ULONG   Small_Slot_Low_Th:2;
       ULONG   Big_Slot_Low_Th:2;
       ULONG   Raw_CI_Enb:1;
       ULONG   Small_CI_Enb:1;
       ULONG   Big_CI_Enb:1;
       ULONG   Slot_Cong_CI_Enb:1;
       ULONG   Rx_Raw_Report_Hldoff:2;
   };

   ULONG   reg;
}
   TB155PCISAR_CNTRL2,
   *PTB155PCISAR_CNTRL2;

typedef union  _INTR_REGISTER
{
   struct
   {
       ULONG   Rx_IOC:1;              // the least significant bit
       ULONG   Rx_Unknown_Ack:1;
       ULONG   Rx_Unknown_VC:1;
       ULONG   Rx_Unopened_VC:1;
       ULONG   Rx_Raw_Slots_Low:1;
       ULONG   Rx_Small_Slots_Low:1;
       ULONG   Rx_Big_Slots_Low:1;
       ULONG   Rx_No_Raw_Slots:1;
       ULONG   Rx_No_Small_Slots:1;
       ULONG   Rx_No_Big_Slots:1;
       ULONG   Rx_Data_Fifo_Ovfl:1;
       ULONG   Rx_Free_Slot_Ovfl:1;
       ULONG   FM_Resynch_Done:1;
       ULONG   Tx_IOC:1;
       ULONG   Tx_Free_Slot_Unfl:1;
       ULONG   Host_Access_Err:1;
       ULONG   Ph_Access_Err:1;
       ULONG   Ph_Done_Intr:1;
       ULONG   Ph_Intr:1;
       ULONG   PCI_Fatal_Err:1;
       ULONG   Tx_Fatal_Err:1;
       ULONG   reserved:11;
   };

	ULONG	reg;
}
   INTR_STATUS_REGISTER,
   *PINTR_STATUS_REGISTER,
   INTR_ENB_REGISTER,
   *PINTR_ENB_REGISTER;


typedef union  _INTR_HLDOFF_REG
{
   struct
   {
       ULONG   Rx_IOC_Hldoff_Time:8;         // the least significant bit
       ULONG   Rx_IOC_Hldoff_Cnt:7;
       ULONG   Rx_IOC_Hldoff_Wr:1;
       ULONG   Tx_IOC_Hldoff_Time:8;
       ULONG   Tx_IOC_Hldoff_Cnt:7;
       ULONG   Tx_IOC_Hldoff_Wr:1;
   };

   ULONG   reg;
}
   INTR_HLDOFF_REG,
   *PINTR_HLDOFF_REG;


#define RX_IOC_HLDOFF_CNT     32
#define RX_IOC_HLDOFF_TIME    0x40        // hold off time = 3.93u * 64

#define TX_IOC_HLDOFF_CNT     20
#define TX_IOC_HLDOFF_TIME    0x20        // hold off time = 503.04u * 32


typedef union  _TX_FS_LIST_PTRS_REG
{
   struct
   {
       ULONG   Tx_FS_List_Tail:12;         // the least significant bit
       ULONG   reserved1:4;
       ULONG   Tx_FS_List_Head:12;
       ULONG   reserved2:3;
       ULONG   Tx_FS_List_Valid:1;
	};

	ULONG	reg;
}
   TX_FS_LIST_PTRS,
   *PTX_FS_LIST_PTRS;


typedef union  _REPORT_BASE_REG
{
   struct
   {
       ULONG   reserved:6;                 // the least significant bit
       ULONG   Report_Base:26;
	};

   ULONG   reg;
}
   TX_REPORT_BASE,
   *PTX_REPORT_BASE,
   RX_REPORT_BASE,
   *PRX_REPORT_BASE;


typedef union  _REPORT_PTR_REG
{
   struct
   {
       ULONG   Own_Wr_Val:1;                 // the least significant bit
       ULONG   reserved:1;
       ULONG   Report_Ptr:30;
   };

   ULONG   reg;
}
   TX_REPORT_PTR,
   *PTX_REPORT_PTR,
   RX_REPORT_PTR,
   *PRX_REPORT_PTR;


typedef union  _RX_SLOT_CONG_TH_REG
{
   struct
   {
       ULONG   Small_Slot_Cong_Thld:8;     // the least significant bit
       ULONG   Big_Slot_Cong_Thld:8;
       ULONG   Raw_Slot_Cong_Thld:8;
       ULONG   reserved:8;
	};

   ULONG   reg;
}
   RX_SLOT_CONG_TH,
   *PRX_SLOT_CONG_TH;


typedef union  _TX_ABR_ADTF_REG
{
   struct
   {
       ULONG	ADTF_1:16;              // the least significant bit
       ULONG	ADTF_2:16;
   };

   ULONG   reg;
}
   TX_ABR_ADTF,
   *PTX_ABR_ADTF;


typedef union  _TX_ABR_NRM_TRM_REG
{
   struct
   {
       ULONG   Trm_1:9;                // the least significant bit
       ULONG   Nrm_1:3;
       ULONG   Trm_2:9;
       ULONG   Nrm_2:3;
       ULONG   Line_Rate_Exp:5;
       ULONG   reserved:3;
   };

   ULONG	reg;
}
   TX_ABR_NRM_TRM,
   *PTX_ABR_NRM_TRM;


typedef union  _PEEPHOLE_CMD_REG
{
   struct
   {
       ULONG   Ph_Addr:18;              // the least significant bit
       ULONG	reserved:12;

       // 0:   Access to on-board SRAM.
       // 1:   Access to peripheral device.

       ULONG	Ph_Prf_Access:1;

       // 0:   Wish to perform Write.
       // 1:   Wish to perfor Read.

       ULONG	Ph_Read:1;
   };

   ULONG   reg;
}
   PEEPHOLE_CMD,
   *PPEEPHOLE_CMD;

//=
//     Definitions for PeepHole command register.
//=

#define TBATM155_PH_READ           BIT(31)
#define TBATM155_PH_DEV_ACCESS     BIT(30)

#define TBATM155_PH_READ_DEV_CMD   (TBATM155_PH_READ | TBATM155_PH_DEV_ACCESS)
#define TBATM155_PH_READ_SRAM_CMD  TBATM155_PH_READ
#define TBATM155_PH_WRITE_DEV_CMD  TBATM155_PH_DEV_ACCESS
#define TBATM155_PH_WRITE_SRAM_CMD 0x00000000L


//=
//     Definition for PeepHole Data register.
//=

#define TBATM155_PH_DONE           BIT(31)

//
// Time-out loop count for waiting PHY_DONE through Peephole registers.
//
#define TBATM155_TIMEOUT_PHY_DONE  0x0FFFFFFFL


typedef union  _PEEPHOLE_DATA_REG
{
   struct
   {
       ULONG   Ph_Data:16;              // the least significant bit
       ULONG   reserved:15;
       ULONG   Ph_Done:1;
   };

   ULONG	reg;
}
   PEEPHOLE_DATA,
   *PPEEPHOLE_DATA;


typedef union  _RX_RAW_SLOT_TAG_REG
{
   struct
   {
       ULONG   Rx_Raw_Slot_Valid:1;        // the least significant bit
       ULONG   reserved:1;
       ULONG   Rx_Raw_Slot_Tag:14;         // ((Based on 4K VC Meteor manul))
       ULONG   reserved2:16;
   };

   ULONG   reg;
}
   RX_RAW_SLOT_TAG,
   *PRX_RAW_SLOT_TAG;


typedef union  _CELLCNT_REG
{
   struct
   {
       ULONG   Rx_Cell_Cnt:20;             // the least significant bit
       ULONG   reserved:12;
   };

   ULONG   reg;
}
   RX_CELLCNT,
   *PRX_CELLCNT,
   TX_CELLCNT,
   *PTX_CELLCNT;


typedef union  _RX_DROPPKTCNT_REG
{
   struct
   {
       ULONG   Rx_Drop_PktCnt:20;      // the least significant bit
       ULONG	reserved:12;
   };

   ULONG   reg;
}
   RX_AAL5_DROPPKTCNT,
   *PRX_AAL5_DROPPKTCNT,
   RX_RAW_DROPPKTCNT,
   *PRX_RAW_DROPPKTCNT;


typedef union  _REAL_TIME_REG
{
   struct
   {
       ULONG   Real_Time_30ns:8;       // the least significant bit
       ULONG   Real_Time_3p30us:22;
       ULONG   reserved:2;
   };

   ULONG   reg;
}
   REAL_TIME_REG,
   *PREAL_TIME_REG;


typedef union  _CURRENT_TIME_REG
{
   struct
   {
       ULONG   CBR_Current_Time:8;     // the least significant bit
       ULONG   ABR_Current_Time:8;     // the least significant bit
       ULONG   reserved:16;
   };

   ULONG   reg;
}
   CURRENT_TIME_REG,
   *PCURRENT_TIME_REG;


typedef union  _TEST_CNTRL_REG
{
   struct
   {
       ULONG   Rx_DMA_Disable:1;       // the least significant bit
       ULONG   Tx_DMA_Disable:1;
       ULONG   Counter_Segment:1;
       ULONG   Counter_Force_Count:1;
       ULONG   Force_All_Intr:1;
       ULONG   Ph_Test:1;
       ULONG   Ph_GFC_Set_A:1;
       ULONG   Ph_GFC_Halt:1;
       ULONG   RATO_Poll_Time:1;
       ULONG   LastRM_Poll_Time:1;
       ULONG   Tac_Timer_Speedup:1;
       ULONG   Tds_Wait:1;
       ULONG   Report_Segment:1;
       ULONG   reserved:19;
   };

   ULONG   reg;
}
   TEST_CNTRL_REG,
   *PTEST_CNTRL_REG;


typedef struct	_TX_PENDING_SLOT_REGS
{
   TX_PENDING_SLOTS_CTL    Cntrl;
   ULONG                   Base_Addr;
}
   TX_PENDING_SLOT,
   *PTX_PENDING_SLOT;


typedef struct	_RX_PENDING_SLOT_REGS
{
   RX_PENDING_SLOTS_CTL    Cntrl;
   ULONG                   Base_Addr;
}
   RX_PENDING_SLOT,
   *PRX_PENDING_SLOT;


#define	MAX_SLOTS_NUMBER        8

typedef struct	_TBATM155_SAR
{
   TX_PENDING_SLOT         Tx_Pending_Slots[MAX_SLOTS_NUMBER];
   RX_PENDING_SLOT         Rx_Pending_Slots[MAX_SLOTS_NUMBER];
   TB155PCISAR_CNTRL1      SAR_Cntrl1;
   TB155PCISAR_CNTRL2      SAR_Cntrl2;
   INTR_STATUS_REGISTER    Intr_Status;
   INTR_ENB_REGISTER       Intr_Enb;
   INTR_HLDOFF_REG         Intr_Hldoff;
   TX_FS_LIST_PTRS         Tx_FS_List_ptrs;
   TX_REPORT_BASE          Tx_Report_Base;
   RX_REPORT_BASE          Rx_Report_Base;
   TX_REPORT_PTR           Tx_Report_Ptr;
   RX_REPORT_PTR           Rx_Report_Ptr;
   RX_SLOT_CONG_TH         Rx_Slot_Cong_Th;
   TX_ABR_ADTF             Tx_ABR_ADTF;
   TX_ABR_NRM_TRM          Tx_ABR_Nrm_Trm;
   PEEPHOLE_CMD            PeepHole_Cmd;
   PEEPHOLE_DATA           PeepHole_Data;
   RX_RAW_SLOT_TAG         Rx_Raw_Slot_Tag;
   RX_CELLCNT              Rx_CellCnt;
   RX_AAL5_DROPPKTCNT      Rx_AAL5_DropPktCnt;
   RX_RAW_DROPPKTCNT       Rx_Raw_DropPktCnt;
   TX_CELLCNT              Tx_CellCnt;
   REAL_TIME_REG           Real_Time;
   CURRENT_TIME_REG        Current_Time;
   TEST_CNTRL_REG          Test_Cntrl;
}
	TBATM155_SAR,
	*PTBATM155_SAR;


//
//	The following defines are used for the interrupt registers.
//
//	Intr_Status	-   If a bit is set then the interrupt is pending.
//			        Write 1 to clear status bits.
//	Intr_En     -   If a bit is set then the interrupt is enabled.
//

#define TBATM155_INT_TX_FATAL_ERR          BIT(20)
#define TBATM155_INT_PCI_FATAL_ERR         BIT(19)
#define TBATM155_INT_PHY_INTR              BIT(18)
#define TBATM155_INT_PH_DONE               BIT(17)
#define TBATM155_INT_PH_ACCESS_ERR         BIT(16)
#define TBATM155_INT_HOST_ACCESS_ERR       BIT(15)
#define TBATM155_INT_TX_FREE_SLOT_UNFL     BIT(14)
#define TBATM155_INT_TX_IOC                BIT(13)
#define TBATM155_INT_FM_RESYNCH_DONE       BIT(12)
#define TBATM155_INT_RX_FREE_SLOT_OVFL     BIT(11)
#define TBATM155_INT_RX_DATA_FIFO_OVFL     BIT(10)
#define TBATM155_INT_RX_NO_BIG_SLOTS       BIT(9)
#define TBATM155_INT_RX_NO_SMALL_SLOTS     BIT(8)
#define TBATM155_INT_RX_NO_RAW_SLOTS       BIT(7)
#define TBATM155_INT_RX_BIG_SLOTS_LOW      BIT(6)
#define TBATM155_INT_RX_SMALL_SLOTS_LOW    BIT(5)
#define TBATM155_INT_RX_RAW_SLOTS_LOW      BIT(4)
#define TBATM155_INT_RX_UNOPENED_VC        BIT(3)
#define TBATM155_INT_RX_UNKNOWN_VC         BIT(2)
#define TBATM155_INT_RX_UNKNOWN_ACK        BIT(1)
#define TBATM155_INT_RX_IOC                BIT(0)


#define TBATM155_REG_INT_VALID (TBATM155_INT_TX_FATAL_ERR          |	\
                                TBATM155_INT_PCI_FATAL_ERR         | 	\
                                TBATM155_INT_PHY_INTR              | 	\
                                TBATM155_INT_HOST_ACCESS_ERR       | 	\
                                TBATM155_INT_TX_FREE_SLOT_UNFL     | 	\
                                TBATM155_INT_TX_IOC                | 	\
                                TBATM155_INT_RX_FREE_SLOT_OVFL     | 	\
                                TBATM155_INT_RX_DATA_FIFO_OVFL     | 	\
                                TBATM155_INT_RX_NO_BIG_SLOTS       | 	\
                                TBATM155_INT_RX_NO_SMALL_SLOTS     | 	\
                                TBATM155_INT_RX_BIG_SLOTS_LOW      | 	\
                                TBATM155_INT_RX_SMALL_SLOTS_LOW    |    \
                                TBATM155_INT_RX_UNOPENED_VC        | 	\
                                TBATM155_INT_RX_UNKNOWN_VC         |	\
                                TBATM155_INT_RX_IOC)

#define TBATM155_REG_INT_ERROR (TBATM155_INT_TX_FATAL_ERR          |	\
                                TBATM155_INT_PCI_FATAL_ERR         |    \
                                TBATM155_INT_TX_FREE_SLOT_UNFL     | 	\
                                TBATM155_INT_RX_FREE_SLOT_OVFL)
   


//
//	This structure contains informaiton for managing the transmit and
//	receive DMA queues.
//
typedef struct _RECV_BUFFER_QUEUE
{
   //
   //	Count of available receive buffers in the free list.
   //
   ULONG                   BufferCount;

   //
   //	Free Receive buffer queue.
   //
   //	This is a list of buffers that are no longer in use.
   //	When the buffer is first allocated this is NULL.  Once a VC
   //  is created and activated then this list will get "larger".
   //  Also, this list will get "smaller" if a VC is deactivated.
   //
	PRECV_BUFFER_HEADER     BufListHead;
	PRECV_BUFFER_HEADER     BufListTail;

	NDIS_SPIN_LOCK		    lock;

}
   RECV_BUFFER_QUEUE,
   *PRECV_BUFFER_QUEUE;


//
//	This structure contains informaiton for managing the transmit and
//	receive DMA queues.
//
struct _RECV_DMA_QUEUE
{
	//
	//	List of "Small" buffers that are awaiting DMA completion interrupt.
	//

   RECV_BUFFER_QUEUE       CompletingSmallBufQ;

   //
   //  Note:
   //      The field "Slot Tag" in entries of Rx report queue
   //      reflects whatever the value of driver writes to
   //      "Slot Tag" bits in Rx Pending control register, so driver
   //      assignes an unique "Slot Tag" for each receive buffer in 
   //      order to search the receive buffer from the queue.
   //
   //   The current Slot Tag of "Small" buffers that will be assigned to
   //   the next allocated receive buffer.
   //   1. Slot Tag range from 1 to 4095.
   //   2. Initialize after soft reset controller.
   //
   USHORT                  CurrentSlotTagOfSmallBufQ;

   //
   //	List of "Big" buffers that are awaiting DMA completion interrupt.
   //
   RECV_BUFFER_QUEUE       CompletingBigBufQ;

   //
   //   The current Slot Tag of "Big" buffers that will be assigned to
   //   the next allocated receive buffer.
   //   1. Slot Tag range from 1 to 4095
   //   2. Initialize after soft reset controller.
   //
   USHORT                  CurrentSlotTagOfBigBufQ;
		
   //
   //	The following are for keeping track with the "Big" receive slots.
   //
   ULONG                   MaximumReceiveBigSlots;
   ULONG                   RemainingReceiveBigSlots;

	//
	//	The following are for keeping track with the "Small" receive slots.
	//
   ULONG                   MaximumReceiveSmallSlots;
   ULONG                   RemainingReceiveSmallSlots;

   //
   // The following are for keeping track with the informtion of
   // report queue handling.
   //
   // Keep track where was the stop point of last time we handled.
   //
   UINT                    PrevRxReportQIndex;

#if    DBG
   ULONG                   dbgTotalUsedSmallRxSlots;
   ULONG                   dbgTotalUsedBigRxSlots;
#endif // end of DBG

	NDIS_SPIN_LOCK			lock;
};


//
//	This structure contains informaiton for managing the transmit and
//	receive DMA queues.
//
struct _XMIT_DMA_QUEUE
{
   //
   //	List of packets that are waiting to be dma'd.
   //
   PACKET_QUEUE			DmaWait;


   //
   //	The following are for keeping track with the slots.
   //
   ULONG               MaximumTransmitSlots;

   ULONG               RemainingTransmitSlots;

#if    DBG
   ULONG               dbgTotalPostedTxSlots;
#endif // end of DBG

   //
   // The following are for keeping track with the informtion of
   // report queue handling.
   //
   // Keep track where was the stop point of last time we handled.
   //
   UINT                PrevTxReportQIndex;

	NDIS_SPIN_LOCK		lock;
};


//
//	Contains information about the Segmentation and Reassembly unit.
//
struct _SAR_INFO
{
   NDIS_SPIN_LOCK		lockFreeXmitSegment;

   //
   //	DMA queues for transmit and receive.
   //
   XMIT_DMA_QUEUE		XmitDmaQ;
   RECV_DMA_QUEUE		RecvDmaQ;


   //
   //  Free Receive "SMALL" buffer queue.
   //
   //  This is a list of buffers that are no longer in use.
   //  When the buffer is first allocated this is NULL.  Once a VC
   //  is created and activated then this list will get "larger".
   //  Also, this list will get "smaller" each VC is deactivated.
   //
   RECV_BUFFER_QUEUE      FreeSmallBufferQ;
   BOOLEAN                fSmallSlotsLowOrNone;

   //
   //	Free Receive "BIG" buffer queue.
   //
   //	This is a list of buffers that are no longer in use.
   //	When the buffer is first allocated this is NULL.  Once a VC
   //  is created and activated then this list will get "larger".
   //  Also, this list will get "smaller" each VC is deactivated.
   //
   RECV_BUFFER_QUEUE      FreeBigBufferQ;
   BOOLEAN                fBigllSlotsLowOrNone;


   //
   //	"BIG" Receive Pool queue.
   //
   //	This is a queue of RECV_BUF_POOL buffers that are allocated.
   //	When the buffer is first allocated this is NULL. After more
   //  VCs have been created, this queue will get "larger" until 
   //  allocated the maximum number of receive buffers .
	//
   PRECV_BUFFER_POOL      BigRecvPoolQ;

   //
   //  Point to the end of BigRecvPoolQ.
   //
   PRECV_BUFFER_POOL      BigRecvPoolTail;

   //
   //  Number of allocated Big Rx buffers.
   //
   USHORT                 AllocatedBigRecvBuffers;


   //
   //	"SMALL" Receive Pool queue.
   //
   //	This is a queue of RECV_BUF_POOL buffers that are allocated.
   //	When the buffer is first allocated this is NULL. After more
   //  VCs have been created, this queue will get "larger" until 
   //  allocated the maximum number of receive buffers .
   //
   PRECV_BUFFER_POOL      SmallRecvPoolQ;

   //
   //  Point to the end of SmallRecvPoolQ.
   //
   PRECV_BUFFER_POOL      SmallRecvPoolTail;

   //
   //  Number of allocated Small Rx buffers.
   //
   USHORT                  AllocatedSmallRecvBuffers;

   //
   //  The maximum number of allowed allocated Rx buffers for 
   //  each Small & Big sets of Rx buffers.
   //
   USHORT                  MaxRecvBufferCount;

#if    DBG

   //
   USHORT                  dbgTxPendingSlot;
   USHORT                  dbgRxPendingSlot;

#endif // end of DBG

};


#endif // __SAR_H

