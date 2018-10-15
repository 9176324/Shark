/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       plc_2.h
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920


Abstract:

   This file contains Toshiba's PLC2 specific definitions.


Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	09/23/96		kyleb		Added support for NdisAllocateMemoryWithTag
	05/09/97		awang		Initial of Toshiba ATM 155 Device Driver.

--*/

#ifndef __PLC2_H
#define __PLC2_H


//////////////////////////////////////////////////////////////////////////
//																		
//						PLC_2 REGISTER SET
//
//////////////////////////////////////////////////////////////////////////


typedef union _PLC2_REGISTERS
{
   struct
   {
       UCHAR   confgContResetStop;             //  0x00
       UCHAR   confgFrameManagement;           //  0x01
       UCHAR   confgCellManagement1;           //  0x02
       UCHAR   confgCellManagement2;           //  0x03
       UCHAR   confgLoopModeSelect;            //  0x04
       UCHAR   confgFrameSync;                 //  0x05
       UCHAR   confgCellSync;                  //  0x06
       UCHAR   confgLosControl1;               //  0x07
       UCHAR   confgLosControl2;               //  0x08
       UCHAR   confgJ1PositionH;               //  0x09
       UCHAR   confgJ1PositionV;               //  0x0A
       UCHAR   confgCIADR;                     //  0x0B
       UCHAR   confgAlarmStatus1;              //  0x0C
       UCHAR   confgAlarmStatus2;              //  0x0D
       UCHAR   confgReserved0;                 //  0x0E
       UCHAR   confgCellManagement3;           //  0x0F

       UCHAR   stIntrMask1;                    //  0x10
       UCHAR   stIntrMask2;                    //  0x11
       UCHAR   stIntrMask3;                    //  0x12
       UCHAR   stReserved0[2];                 // --
       UCHAR   stIntrStatus1;                  //  0x15
       UCHAR   stIntrStatus2;                  //  0x16
       UCHAR   stIntrStatus3;                  //  0x17
       UCHAR   stReserved1[2];                 // --
       UCHAR   stReceiveStatus1;               //  0x1a
       UCHAR   stAlarmStatus2;                 //  0x1b
       UCHAR   stPLC2Status;                   //  0x1c
       UCHAR   stReserved2[2];                 // --
       UCHAR   stReceiveGFCStatus;             //  0x1f

       UCHAR   ecB1Sbip8ErrCounterL;           //  0x20
       UCHAR   ecB1Sbip8ErrCounterH;           //  0x21
       UCHAR   ecB2Sbip24ErrCounterL;          //  0x22
       UCHAR   ecB2Sbip24ErrCounterM;          //  0x23
       UCHAR   ecB2Sbip24ErrCounterH;          //  0x24
       UCHAR   ecB3Sbip8ErrCounterL;           //  0x25
       UCHAR   ecB3Sbip8ErrCounterH;           //  0x26
       UCHAR   ecRSfebeCounterL;               //  0x27
       UCHAR   ecRSfebeCounterM;               //  0x28
       UCHAR   ecRSfebeCounterH;               //  0x29
       UCHAR   ecRPfebeCounterL;               //  0x2a
       UCHAR   ecRPfebeCounterH;               //  0x2b
       UCHAR   ecReserved0[3];                 // --
       UCHAR   ecErrCounterUpdateFlag;         //  0x2f

       UCHAR   ccCoCellHeaderErrCounter;       //  0x30
       UCHAR   ccUnCoCellHeaderErrCounter;     //  0x31
       UCHAR   ccRxValidCellCounterL;          //  0x32
       UCHAR   ccRxValidCellCounterM;          //  0x33
       UCHAR   ccRxValidCellCounterH;          //  0x34
       UCHAR   ccTxValidCellCounterL;          //  0x35
       UCHAR   ccTxValidCellCounterM;          //  0x36
       UCHAR   ccTxValidCellCounterH;          //  0x37
       UCHAR   ccReserved[7];                  // --
       UCHAR   ccCellCounterUpdateFlag;        //  0x3f

       UCHAR   memAddressL;                    //  0x40
       UCHAR   memAddressH;                    //  0x41
       UCHAR   memData;                        //  0x42
       UCHAR   Reserved7[13];                  // --

       UCHAR   testTMODE;                      //  0x50
       UCHAR   testTSGEM;                      //  0x51
       UCHAR   testSFEBEI;                     //  0x52
       UCHAR   testLFEBEI;                     //  0x53
       UCHAR   testPFEBEI;                     //  0x54
   }
       Plc2;

   UCHAR	RegNum[256];
}
	PLC2_REGISTERS,
	*PPLC2_REGISTERS;


// 
// Definitions for bits of PLC2 registers
// 
//     Register 1
// 
#define fPLC2_CONT_AUTOCSE     BIT(7)
#define fPLC2_CONT_LOOCDE      BIT(6)
#define fPLC2_CONT_OOLDE       BIT(4)
#define fPLC2_CONT_TXFRES      BIT(3)
#define fPLC2_CONT_RXFRES      BIT(2)
#define fPLC2_CONT_STOP        BIT(1)
#define fPLC2_MRI_RESET		BIT(0)


// 
//     Register 3
// 
#define fPLC2_CMS2_DROP        BIT(0)

// 
//     Register 0x15 (Interrupt Status 1)
// 
#define fPLC2_INTIND1_OOF      BIT(1)
#define fPLC2_INTIND1_LOF      BIT(2)

// 
//     Register 0x1B (Alarm Status 2)
// 
#define fPLC2_St2ALARM_LRDI    BIT(0)

//
//   Define constants for on-board device LED controlled through
//   the peephole interface.
//   The higher 4 bits of LED control data are defined as following,
//     Bit 7 : Rate
//     Bit 6 : LPBK
//     Bit 5 : TWLOPBK     --> UTP LoopBack Enable
//     Bit 4 : TXENBL      --> Seletc Tx enable
//     Bit 3 : LED_TXRX    --> LED_RXTX_ON_GREEN     
//     Bit 1 : LED_TXRX    --> LED_RXTX_ON_ORANGE    
//     Bit 1 : LED_LNKUP   --> LED_LINKUP_ON_GREEN     
//     Bit 0 : LED_LNKUP   --> LED_LINKUP_ON_ORANGE    
//   
#define fLED_PLC2_155MB_TXENB  0x010L

#endif // __PLC2_H


