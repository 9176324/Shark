/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       tbmeteor.h
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920


Abstract:

   This file contains the specific definitions related to
   1K and 4K VCs.

Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	05/19/97		awang		Initial of Toshiba ATM 155 Device Driver.

--*/

#ifndef __TBMETEOR_H
#define __TBMETEOR_H



//
// Data structure of pointer of SRAM table in 4K VC mode.
//
typedef struct _SRAM_4K_VC_MODE{
       ULONG   pRx_AAL5_Big_Free_Slot;
       ULONG   pRx_AAL5_Small_Free_Slot;
       ULONG   pRx_Raw_Free_Slot;
       ULONG   pACR_LookUp_Tbl;
       ULONG   pReserved;
       ULONG   pRx_VC_State_Tbl;
       ULONG   pTx_VC_State_Tbl;
       ULONG   pABR_Parameter_Tbl;
       ULONG   pABR_Value_Tbl;
       ULONG   pRM_Cell_Data_Tbl;
       ULONG   pTx_Slot_Descriptors;
       ULONG   pABR_Schedule_Tbl;
       ULONG   pCBR_Schedule_Tbl_1;
       ULONG   pCBR_Schedule_Tbl_2;
       ULONG   pRx_AAL5_B_Slot_Tags;
       ULONG   pRx_AAL5_S_Slot_Tags;
       ULONG   pRx_Raw_Slot_Tags;
       ULONG   pRx_Slot_Tag_VC_State_Tbl;
       ULONG   pEnd_Of_SRAM;
} 
   SRAM_4K_VC_MODE,
   *PSRAM_4K_VC_MODE;


//
// Data structure of pointer of SRAM table in 1K VC mode.
//
typedef struct _SRAM_1K_VC_MODE{
       ULONG   pRx_AAL5_Big_Free_Slot;
       ULONG   pRx_AAL5_Small_Free_Slot;
       ULONG   pRx_Raw_Free_Slot;
       ULONG   pReserved1;
       ULONG   pRx_VC_State_Tbl;
       ULONG   pTx_VC_State_Tbl;
       ULONG   pABR_Parameter_Tbl;
       ULONG   pABR_Value_Tbl;
       ULONG   pRM_Cell_Data_Tbl;
       ULONG   pTx_Slot_Descriptors;
       ULONG   pACR_LookUp_Tbl;
       ULONG   pReserved2;
       ULONG   pRx_AAL5_B_Slot_Tags;
       ULONG   pRx_AAL5_S_Slot_Tags;
       ULONG   pRx_Raw_Slot_Tags;
       ULONG   pABR_Schedule_Tbl;
       ULONG   pCBR_Schedule_Tbl_1;
       ULONG   pCBR_Schedule_Tbl_2;
       ULONG   pEnd_Of_SRAM;
}
   SRAM_1K_VC_MODE,
   *PSRAM_1K_VC_MODE;


#define TEST_WORD          6
#define TEST_PATTERN_WORD  3

//
// Constants for Meteor supporting 4K Vc
//
#define CNTRL1_4KVC        0x00006000
#define SRAM_SIZE_4KVC     0x34000

//
// Constants for Meteor supporting 4K Vc
//
#define SRAM_SIZE_1KVC     0x10000


#endif // __TBMETEOR_H


