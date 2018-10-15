/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       suni_lit.h
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920


Abstract:

   This file contains PMC S/UNI-Lite specific definitions.


Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	09/23/96		kyleb		Added support for NdisAllocateMemoryWithTag
	05/09/97		awang		Initial of Toshiba ATM 155 Device Driver.

--*/


#ifndef __SUNI_LIT_H
#define __SUNI_LIT_H


//////////////////////////////////////////////////////////////////////////
//																		
//						S/UNI-Lite REGISTER SET
//
//////////////////////////////////////////////////////////////////////////


typedef union _SUNI_REGISTERS
{
   struct
   {
       UCHAR   MasterResetIdentity;                    //  0x00
       UCHAR   MasterConfig;                           //  0x01
       UCHAR   MasterInterruptStatus;                  //  0x02
       UCHAR   Reserved0;                              //  0x03
       UCHAR   MasterClockMonitor;                     //  0x04
       UCHAR   MasterControl;                          //  0x05
       UCHAR   ClkSynthesisStatusControl;              //  0x06
       UCHAR   ClkRecoveryStatusControl;               //  0x07
       UCHAR   Reserved1[8];                           //  ----
       UCHAR   rsopControlIntEnable;                   //  0x10
       UCHAR   rsopStatusIntStatus;                    //  0x11
       UCHAR   rsopSectionBipLsb;                      //  0x12
       UCHAR   rsopSectionBipMsb;                      //  0x13
       UCHAR   tsopControl;                            //  0x14
       UCHAR   tsopDiag;                               //  0x15
       UCHAR   Reserved2[2];                           //  ----
       UCHAR   rlopControlStatus;                      //  0x18
       UCHAR   rlopIntEnableStatus;                    //  0x19
       UCHAR   rlopLineBibLsb;                         //  0x1a
       UCHAR   rlopLineBib;                            //  0x1b
       UCHAR   rlopLineBipMsb;                         //  0x1c
       UCHAR   rlopLineFebeLsb;                        //  0x1d
       UCHAR   rlopLineFebe;                           //  0x1e
       UCHAR   rlopLineFebeMsb;                        //  0x1f
       UCHAR   tlopControl;                            //  0x20
       UCHAR   tlopDiag;                               //  0x21
       UCHAR   Reserved3[14];                          //  ----
       UCHAR   rpopStatusControl;                      //  0x30
       UCHAR   rpopIntStatus;                          //  0x31
       UCHAR   Reserved4;                              //  ----
       UCHAR   rpopIntEnable;                          //  0x33
       UCHAR   Reserved5[3];                           //  ----
       UCHAR   rpopPathSignalLabel;                    //  0x37
       UCHAR   rpopPathBipLsb;                         //  0x38
       UCHAR   rpopPathBipMsb;                         //  0x39
       UCHAR   rpopPathFebeLsb;                        //  0x3a
       UCHAR   rpopPathFebeMsb;                        //  0x3b
       UCHAR   Reserved60;                             //  ----
       UCHAR   tpopPathBIP8Config;                     //  0x3d
       UCHAR   Reserved61[2];                          //  ----
       UCHAR   tpopControlDiag;                        //  0x40
       UCHAR   tpopPointerControl;                     //  0x41
       UCHAR   Reserved7[3];                           //  ----
       UCHAR   tpopArbitPointerLsb;                    //  0x45
       UCHAR   tpopArbitPointerMsb;                    //  0x46
       UCHAR   Reserved8;                              //  ----
       UCHAR   tpopPathSignalLable;                    //  0x48
       UCHAR   tpopPathStatus;                         //  0x49
       UCHAR   Reserved9[6];                           //  ----
       UCHAR   racpControlStatus;                      //  0x50
       UCHAR   racpIntEnableStatus;                    //  0x51
       UCHAR   racpMatchHeaderPattern;                 //  0x52
       UCHAR   racpMatchHeaderMask;                    //  0x53
       UCHAR   racpCorrectableHCSErrorCount;           //  0x54
       UCHAR   racpUncorrectableHCSErrorCount;         //  0x55
       UCHAR   racpReceiveCellCounterLsb;              //  0x56
       UCHAR   racpReceiveCellCounter;                 //  0x57
       UCHAR   racpReceiveCellCounterMsb;              //  0x58
       UCHAR   racpConfig;                             //  0x59
       UCHAR   Reserved10[6];                          //  ----
       UCHAR   tacpControlStatus;                      //  0x60
       UCHAR   tacpIdleUnassignedCellHeaderPattern;    //  0x61
       UCHAR   tacpIdleUnassignedCellPayloadPatter;    //  0x62
       UCHAR   tacpFifoConfig;                         //  0x63
       UCHAR   tacpTransmitCellCounterLsb;             //  0x64
       UCHAR   tacpTransmitCellCounter;                //  0x65
       UCHAR   tacpTransmitCellCounterMsb;             //  0x66
       UCHAR   tacpConfig;                             //  0x67
       UCHAR   Reserved11[24];                         //  ----
       UCHAR   MasterTest;                             //  0x80
   }
       Suni;

   UCHAR	RegNum[256];
}
	SUNI_REGISTERS,
	*PSUNI_REGISTERS;



#define fSUNI_MRI_RESET		BIT(7)
#define fSUNI_MRI_TYPE_ID		0x30

#define fSUNI_RACP_IES_FUDRI	BIT(0)
#define fSUNI_RACP_IES_FOVRI	BIT(1)
#define fSUNI_RACP_IES_UHCSI	BIT(2)
#define fSUNI_RACP_IES_CHCSI	BIT(3)
#define fSUNI_RACP_IES_OOCDI	BIT(4)
#define fSUNI_RACP_IES_FIFOE	BIT(5)
#define fSUNI_RACP_IES_HCSE	BIT(6)
#define fSUNI_RACP_IES_OOCDE	BIT(7)


 //
 // Bit 2 of reg 0x11: 1 -> losing signal;   0 -> not losing signal
 //
 #define fSUNI_RSOP_STATUS_LOSV    BIT(2)

 //
 // Bit 0 of reg 0x18: 1 -> no defeat detected from far end device.
 //                    0 ->    defeat detected from far end device.
 //
 #define fSUNI_RLOP_STATUS_RDIV    BIT(0)


//
//   Define constants for on-board device LED controlled through
//   the peephole interface.
//   The higher 4 bits of LED control data are defined as following,
//     Bit 7 : Rate0
//     Bit 6 : Rate1 
//       --> Select the frame format and line rate for both Rx & Tx
//           of SUNI-Lite.
//     Bit 5 : TWLOPBK     --> UTP LoopBack Enable.
//     Bit 4 : TXENBL      --> Seletc Tx enable
//     Bit 3 : LED_TXRX    --> LED_RXTX_ON_GREEN     
//     Bit 1 : LED_TXRX    --> LED_RXTX_ON_ORANGE    
//     Bit 1 : LED_LNKUP   --> LED_LINKUP_ON_GREEN     
//     Bit 0 : LED_LNKUP   --> LED_LINKUP_ON_ORANGE    
//   
#define fLED_SUNI_155MB_TXENB  0x0D0L

#endif // __SUNI_LIT_H


