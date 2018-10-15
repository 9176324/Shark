/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       peephole.h
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920


Abstract:
   
   This file contains related definitions of PeepHole registers'
   access devices, such as on-board SRAM & peripheral devices

Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	01/16/97		awang		Initial of Toshiba ATM 155 Device Driver.

--*/

#ifndef __PEEPHOLE_H
#define __PEEPHOLE_H


//////////////////////////////////////////////////////////////////////////
//																		
//              Definitions for SRAM tables.
//
//////////////////////////////////////////////////////////////////////////


//
// Definitions of Receive Slot
//
#define RECV_SLOT_TYPE_SMALL       BIT(0)
#define RECV_SLOT_TYPE_BIG         BIT(1)

#define RX_STAT_SLOTTYPE_SMALL     0
#define RX_STAT_SLOTTYPE_BIG       1
#define RX_STAT_SLOTTYPE_RAW       2
#define RX_STAT_SLOTTYPE_RESV      3

//
// Definitions of Receive Slot
//
#define TX_STAT_RC_CBR             0
#define TX_STAT_RC_ABR             1
#define TX_STAT_RC_UBR             2

#define TX_STAT_CHTYPE_RAW         0
#define TX_STAT_CHTYPE_RESV        1
#define TX_STAT_CHTYPE_AAL5        2
#define TX_STAT_CHTYPE_MPEG        3


//
// Definitions of CBR schedule tables
//
#define CBR_SCHEDULE_ENTRY_EOT     BIT(14)
#define MAX_CBR_SCHEDULE_ENTRIES   4096


//
// The maximum of prescal value for CBR schedule table.
//
#define MAX_PRESCALE_VAL           15


//
// The range of Missing RM cell count in ABR value table.
//
#define TBATM155_MAXIMUM_LOG_CRMX  7
#define TBATM155_MINIMUM_CRM       2
#define TBATM155_MAXIMUM_CRM       256

//
// Default values of Signaling Parameters for SAR registers
//
#define TBATM155_TRM_100MS         397     // UNI 4.0 default is 100 ms
#define TBATM155_NRM_32CELLS       4       // UNI 4.0 default is 32
#define TBATM155_ADTF_500MS        1988    // UNI 4.0 default is 500 ms



//////////////////////////////////////////////////////////////////////////
//																		
//                 Data Structures of SRAM tables
//
//////////////////////////////////////////////////////////////////////////



//
// 1 entry of Tx State Table structure
//     8 words/entry,  1 entry/VC
//
typedef struct _TBATM155_TX_STATE_ENTRY
{
   union       // word 0
   {
       struct
       {
           USHORT  prescal_val:4;
           USHORT  reserved1:4;
           USHORT  CLP:1;
           USHORT  RC:2;
           USHORT  ABS:1;
           USHORT  FM:1;
           USHORT  Tx_AAL5_SM:1;
           USHORT  chtype:2;
       };
       USHORT      data;
   }
       TxStateWord0;

   union       // word 1
   {
       struct
       {
           USHORT  credit:8;
           USHORT  prescal_count:4;
           USHORT  MPEG_count:2;
           USHORT  reserved2:2;
       };
       USHORT      data;
   }
       TxStateWord1;

   union       // word2
   {
       struct
       {
           USHORT  tx_AAL5_CRC_Low:16;
       };
       USHORT      data;
   }
       TxStateWord2;

   union       // word3
   {
       struct
       {
           USHORT  tx_AAL5_CRC_High:16;
       };
       USHORT      data;
   }
       TxStateWord3;

   union       // word4
   {
       struct
       {
           USHORT  tx_A_Slot_H:12;
           USHORT  Turn:1;
           USHORT  OR_BRM:1;
           USHORT  sch:1;
           USHORT  Active:1;
       };
       USHORT      data;
   }
       TxStateWord4;

   union       // word5
   {
       struct
       {
           USHORT  tx_A_Slot_T:12;
           USHORT  reserved3:4;
       };
       USHORT      data;
   }
       TxStateWord5;

   union       // word6
   {
       struct
       {
           USHORT  tx_Slot_Ptr:14;
           USHORT  Idle:1;
           USHORT  reserved4:1;
       };
       USHORT      data;
   }
       TxStateWord6;

   union       // word7
   {
       struct
       {
           USHORT  VC_link:12;
           USHORT  reserved5:4;
       };
       USHORT      data;
   }
       TxStateWord7;

}
   TBATM155_TX_STATE_ENTRY,
   *PTBATM155_TX_STATE_ENTRY;



//
// Structure of Rx State structure
//     8 words/entry,  1 entry/VC
//
typedef    struct  _TBATM155_RX_STATE_ENTRY
{
   union       // word 0
   {
       struct
       {
           USHORT  Slots_Consumed:8;
           USHORT  Slot_Type:2;
           USHORT  Rx_AAL5_SM:1;
           USHORT  Cell_loss:1;
           USHORT  Congestion:1;
           USHORT  Open:1;
           USHORT  Drop:1;
           USHORT  CRC10_En:1;
       };
       USHORT      data;
   }
       RxStateWord0;

   union       // word 1
   {
       struct
       {
           USHORT  AAL5_Length:16;
       };
       USHORT      data;
   }
       RxStateWord1;

   union       // word2
   {
       struct
       {
           USHORT  AAL5_Slot_Address_Low:16;
       };
       USHORT      data;
   }
       RxStateWord2;

   union       // word3
   {
       struct
       {
           USHORT  AAL5_Slot_Address_High:16;
       };
       USHORT      data;
   }
       RxStateWord3;

   union       // word4
   {
       struct
       {
           USHORT  Rx_AAL5_CRC_Low:16;
       };
       USHORT      data;
   }
       RxStateWord4;

   union       // word5
   {
       struct
       {
           USHORT  Rx_AAL5_CRC_High:16;
       };
       USHORT      data;
   }
       RxStateWord5;

   union       // word6
   {
       struct
       {
           USHORT  Slot_ptr:13;
           USHORT  reserved1:3;
       };
       USHORT      data;
   }
       RxStateWord6;

   union       // word7
   {
       struct
       {
           USHORT  Ack_count:8;
           USHORT  Ex_RATO:4;
           USHORT  FM:1;
           USHORT  Or_CLP:1;
           USHORT  Or_CI:1;
           USHORT  In_AAL5_pkt:1;
       };
       USHORT      data;
   }
       RxStateWord7;

}
   TBATM155_RX_STATE_ENTRY,
   *PTBATM155_RX_STATE_ENTRY;




//
// structure of Tx Slot Descriptor
//   4 words/entry,  1 entry/VC
//
typedef    struct  _TBATM155_TX_SLOT_DESC
{
   union       // word 0
   {
       struct
       {
           USHORT  Link:12;
           USHORT  Mgmt:1;
           USHORT  Idle:1;
           USHORT  Raw:1;
           USHORT  reserved1:1;
       };
       USHORT      data;
   }
       TxSlotDescWord0;

   union       // word 1
   {
       struct
       {
           USHORT  Slot_size:14;
           USHORT  EOP:1;
           USHORT  CRC10_En:1;
       };
       USHORT      data;
   }
       TxSlotDescWord1;

   union       // word2
   {
       struct
       {
           USHORT  Base_Address_Low:16;
       };
       USHORT      data;
   }
       TxSlotDescWord2;

   union       // word3
   {
       struct
       {
           USHORT  Base_Address_High:16;
       };
       USHORT      data;
   }
       TxSlotDescWord3;

}
   TBATM155_TX_SLOT_DESC,
   *PTBATM155_TX_SLOT_DESC;


//
// structure of entry of ACR Lookup Table
//   1 word/entry,  512 entries
//
typedef struct     _TBATM155_ACR_LOOKUP_TBL
{
   union
   {
       struct
       {
           USHORT  Cell_Interval:10;
           USHORT  reserved1:6;
       };
       USHORT      data;
   }
        ACRLookupEntry;
}
   TBATM155_ACR_LOOKUP_TBL,
   *PTBATM155_ACR_LOOKUP_TBL;


//
// structure of entry of CBR Schedule Table
//   1 word/entry,  2-4K entries/table, 2 tables
//

typedef struct _TBATM155_CBR_SCHEDULE_ENTRY
{
   union
   {
       struct
       {
           USHORT  VC:12;
           USHORT  reserved1:2;
           USHORT  EOT:1;
           USHORT  Active:1;
       };
       USHORT      data;
   };
}
   TBATM155_CBR_SCHEDULE_ENTRY,
   *PTBATM155_CBR_SCHEDULE_ENTRY;


//
// structure of entry of ABR Value Table
//   4 words/entry,  1 entry/VC
//

typedef    struct  _TBATM155_ABR_VALUE_ENTRY
{
   union       // word 0
   {
       struct
       {
           USHORT  Acr:15;
           USHORT  reserved1:1;
       };
       USHORT      data;
   }
       AbrValueEntryWord0;

   union       // word 1
   {
       struct
       {
           USHORT  Fraction:9;
           USHORT  reserved1:2;
           USHORT  LastRmValid:1;
           USHORT  ForceFRM:1;
           USHORT  AbrValue:1;
           USHORT  FirstTrn:1;
           USHORT  reserved2:1;
       };
       USHORT      data;
   }
       AbrValueEntryWord1;

   union       // word 2
   {
       struct
       {
           USHORT  LastRM:16;
       };
       USHORT      data;
   }
       AbrValueEntryWord2;

   union       // word 3
   {
       struct
       {
           USHORT  NrmCnt:8;
           USHORT  CrmCnt:8;
       };
       USHORT      data;
   }
       AbrValueEntryWord3;
}
   TBATM155_ABR_VALUE_ENTRY,
   *PTBATM155_ABR_VALUE_ENTRY;

typedef    struct  _TBATM155_ABR_PARAM_ENTRY
{
   union       // word 0
   {
       struct
       {
           USHORT  reserved00:5;
           USHORT  Crmx:3;
           USHORT  RDFx:5;
           USHORT  CDFx:3;
           USHORT  reserved01:1;
       };
       USHORT      data;
   }
       AbrParamW0;

   union       // word 1
   {
       struct
       {
           USHORT  AIR:15;
           USHORT  reserved10:1;
       };
       USHORT      data;
   }
       AbrParamW1;

   union       // word 2
   {
       struct
       {
           USHORT  ICR:15;
           USHORT  reserved20:1;
       };
       USHORT      data;
   }
       AbrParamW2;

   union       // word 3
   {
       struct
       {
           USHORT  MCR:15;
           USHORT  reserved30:1;
       };
       USHORT      data;
   }
       AbrParamW3;

   union       // word 4
   {
       struct
       {
           USHORT  PeakCellRate:15;
           USHORT  reserved40:1;
       };
       USHORT      data;
   }
       AbrParamW4;

   union       // word 5
   {
       struct
       {
           USHORT  CI_VC:1;
           USHORT  reserved50:15;
       };
       USHORT      data;
   }
       AbrParamW5;

   union       // word 6
   {
       struct
       {
           USHORT  reserved60:16;
       };
       USHORT      data;
   }
       AbrParamW6;

   union       // word 7
   {
       struct
       {
           USHORT  reserved70:16;
       };
       USHORT      data;
   }
       AbrParamW7;
}
   TBATM155_ABR_PARAM_ENTRY,
   *PTBATM155_ABR_PARAM_ENTRY;


//
// Since Data access is by word in SRM, adjust the entry size of each table
//
#define SIZEOF_RX_FS_ENTRY         2
#define SIZEOF_TX_STATE_ENTRY      (sizeof(TBATM155_TX_STATE_ENTRY) / sizeof(USHORT))
#define SIZEOF_RX_STATE_ENTRY      (sizeof(TBATM155_RX_STATE_ENTRY) / sizeof(USHORT))
#define SIZEOF_TX_SLOT_DESC        (sizeof(TBATM155_TX_SLOT_DESC) / sizeof(USHORT))
#define SIZEOF_ACR_LOOKUP_TBL      (sizeof(TBATM155_ACR_LOOKUP_TBL) / sizeof(USHORT))
#define SIZEOF_CBR_SCHEDULE_ENTRY  (sizeof(TBATM155_CBR_SCHEDULE_ENTRY) / sizeof(USHORT))
#define SIZEOF_ABR_VALUE_ENTRY     (sizeof(TBATM155_ABR_VALUE_ENTRY) / sizeof(USHORT))
#define SIZEOF_ABR_PARAM_ENTRY     (sizeof(TBATM155_ABR_PARAM_ENTRY) / sizeof(USHORT))




//////////////////////////////////////////////////////////////////////////
//																		
//         Definitions of Peripheral Devices
//
//////////////////////////////////////////////////////////////////////////

//
// Address Mapping of Peripheral Devices
//
#define PCI_SUBSYSTEM_ID_OFFSET    0x00L
#define LED_OFFSET                 0x20L
#define MAC_ADDR_ROM_OFFSET        0x40L
#define PHY_DEVICE_OFFSET          0x80L


//
//   Define constants for on-board device LED controlled through
//   the peephole interface.
//
#define LED_TXRX_ON_GREEN      0x00000008L          // Tx/Rx data now.
#define LED_TXRX_ON_ORANGE     0x00000004L          // not use now.
#define LED_LNKUP_ON_GREEN     0x00000002L          // signal on cable is OK
#define LED_LNKUP_ON_ORANGE    0x00000001L          // losing signal on cable

#define LEDS_OFF_ALL           0xFFFFFFF0L
#define LED_TXRX_ALL_BITS      (LED_TXRX_ON_GREEN | LED_TXRX_ON_ORANGE)
#define LED_LNKUP_ALL_BITS     (LED_LNKUP_ON_GREEN | LED_LNKUP_ON_ORANGE)

//
// flags to indicate if Meteor is transmitting or receiving data.
//
#define fMETEOR_TX_DATA        0x00000100L
#define fMETEOR_RX_DATA        0x00000200L


//
// Defined constants for on-board serial EEPROM to be access through
// the peephole interface.
//
#define CHIP_SELECT            0x4
#define SERIAL_DATA_CLOCK      0x1
#define SERIAL_DATA_INPUT      0x2
#define SERIAL_DATA_OUTPUT     0x8


//
// Defined constants for 93C46 serial EEPROM.
// These are based on standard 93C46 serial EEPROM chip speciality.
//
#define RD_INSTRUCTION_93C46       0x6
#define WR_INSTRUCTION_93C46       0x5
#define RD_INSTRUCTION_LEN_93C46   0x3
#define WR_INSTRUCTION_LEN_93C46   0x3
#define ADDRESS_LENGTH1_93C46      0x7
#define ADDRESS_LENGTH2_93C46      0x6


extern USHORT AcrLookUpTbl[];


//////////////////////////////////////////////////////////////////////////
//																		
//                     Function Declarations
//
//////////////////////////////////////////////////////////////////////////


NDIS_STATUS
tbAtm155_Ph_Write_WORD(
	IN  PADAPTER_BLOCK		pAdapter,
   IN  ULONG               Dst_and_CntrlCmd,
   IN  USHORT              Data
   );


NDIS_STATUS
tbAtm155_Ph_Read_WORD(
	IN  PADAPTER_BLOCK		pAdapter,
   IN  ULONG               Dst_and_CntrlCmd,
   OUT PUSHORT             pData
   );


NDIS_STATUS
tbAtm155Read_PermanentAddress(
	IN  PADAPTER_BLOCK  pAdapter,
   OUT PUCHAR          pNodeaddress
   );



//////////////////////////////////////////////////////////////////////////
//																		
//      Macros of Access SRAM & devices through Peephole interface.
//
//////////////////////////////////////////////////////////////////////////

//
// macro for accessing SRAM through Peephole interface
//
#define TBATM155_PH_READ_SRAM(pAdapter, Dst, pData, pStatus)                   \
{                                                                              \
       *(pStatus) = tbAtm155_Ph_Read_WORD(                                     \
                       (pAdapter),                                             \
                       ((ULONG)((ULONG_PTR)Dst) | TBATM155_PH_READ_SRAM_CMD),  \
                       (PUSHORT) (pData));                                     \
                                                                               \
       if (NDIS_STATUS_SUCCESS != *(pStatus))                                  \
       {                                                                       \
           DBGPRINT(DBG_COMP_PEEPHOLE, DBG_LEVEL_ERR,                          \
               ("TBATM155_PH_READ_SRAM failed.\n"));                           \
       }                                                                       \
}

#define TBATM155_PH_WRITE_SRAM(pAdapter, Dst, Data, pStatus)                   \
{                                                                              \
       *(pStatus) = tbAtm155_Ph_Write_WORD(                                    \
                       (pAdapter),                                             \
                       ((ULONG)((ULONG_PTR)Dst) | TBATM155_PH_WRITE_SRAM_CMD), \
                       (USHORT) (Data));                                       \
                                                                               \
       if (NDIS_STATUS_SUCCESS != *(pStatus))                                  \
       {                                                                       \
           DBGPRINT(DBG_COMP_PEEPHOLE, DBG_LEVEL_ERR,                          \
               ("TBATM155_PH_WRITE_SRAM failed.\n"));                          \
       }                                                                       \
}


//
// Macros for accessing Devices through Peephole interface
//

#define TBATM155_PH_READ_DEV(pAdapter, Dst, pData, pStatus)                    \
{                                                                              \
       *(pStatus) = tbAtm155_Ph_Read_WORD(                                     \
                       (pAdapter),                                             \
                       ((ULONG)((ULONG_PTR)Dst) | TBATM155_PH_READ_DEV_CMD),   \
                       (PUSHORT) (pData));                                     \
       if (NDIS_STATUS_SUCCESS != *(pStatus))                                  \
       {                                                                       \
           DBGPRINT(DBG_COMP_PEEPHOLE, DBG_LEVEL_ERR,                          \
               ("TBATM155_PH_READ_DEV failed.\n"));                            \
       }                                                                       \
}

#define TBATM155_PH_WRITE_DEV(pAdapter, Dst, Data, pStatus)                    \
{                                                                              \
       *(pStatus) = tbAtm155_Ph_Write_WORD(                                    \
                       (pAdapter),                                             \
                       ((ULONG)((ULONG_PTR)Dst) | TBATM155_PH_WRITE_DEV_CMD),  \
                       (USHORT)(Data));                                        \
       if (NDIS_STATUS_SUCCESS != *(pStatus))                                  \
       {                                                                       \
           DBGPRINT(DBG_COMP_PEEPHOLE, DBG_LEVEL_ERR,                          \
               ("TBATM155_PH_WRITE_DEV failed.\n"));                           \
       }                                                                       \
}

#endif // __PEEPHOLE_H




