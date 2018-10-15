/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       peephole.c
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920


Abstract:
   
   This file contains routines to read/write off-chip SRAM and devices.

Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	01/16/97		awang		Initial of Toshiba ATM 155 Device Driver.

--*/


#include "precomp.h"
#pragma hdrstop

#define	MODULE_NUMBER	MODULE_PEEPHOLE



//
// The contents of this table is related to line speed.
// (no document provided about the setting of this table.)
//
USHORT AcrLookUpTbl[0x200] = {
/*** PCR = 64b1 ***/
/*** Line rate ( 1 + 0xb1/0x200)*2^18 = 353208 cell/sec ***/
	0x2b2, 0x2b1, 0x2b0, 0x2ae, 0x2ad, 0x2ac, 0x2aa, 0x2a9,     // 0x00
	0x2a8, 0x2a6, 0x2a5, 0x2a4, 0x2a3, 0x2a1, 0x2a0, 0x29f,     // 0x08
	0x29d, 0x29c, 0x29b, 0x29a, 0x298, 0x297, 0x296, 0x295,     // 0x10
	0x293, 0x292, 0x291, 0x290, 0x28f, 0x28d, 0x28c, 0x28b,     // 0x18
	0x28a, 0x289, 0x287, 0x286, 0x285, 0x284, 0x283, 0x282,     // 0x20
	0x280, 0x27f, 0x27e, 0x27d, 0x27c, 0x27b, 0x279, 0x278,     // 0x28
	0x277, 0x276, 0x275, 0x274, 0x273, 0x272, 0x271, 0x26f,     // 0x30
	0x26e, 0x26d, 0x26c, 0x26b, 0x26a, 0x269, 0x268, 0x267,     // 0x38
	0x266, 0x265, 0x264, 0x263, 0x261, 0x260, 0x25f, 0x25e,     // 0x40
	0x25d, 0x25c, 0x25b, 0x25a, 0x259, 0x258, 0x257, 0x256,     // 0x48
	0x255, 0x254, 0x253, 0x252, 0x251, 0x250, 0x24f, 0x24e,     // 0x50
	0x24d, 0x24c, 0x24b, 0x24a, 0x249, 0x248, 0x247, 0x246,     // 0x58
	0x245, 0x244, 0x244, 0x243, 0x242, 0x241, 0x240, 0x23f,     // 0x60
	0x23e, 0x23d, 0x23c, 0x23b, 0x23a, 0x239, 0x238, 0x237,     // 0x68
	0x237, 0x236, 0x235, 0x234, 0x233, 0x232, 0x231, 0x230,     // 0x70
	0x22f, 0x22e, 0x22e, 0x22d, 0x22c, 0x22b, 0x22a, 0x229,     // 0x78
	0x228, 0x228, 0x227, 0x226, 0x225, 0x224, 0x223, 0x222,     // 0x80
	0x222, 0x221, 0x220, 0x21f, 0x21e, 0x21d, 0x21d, 0x21c,     // 0x88
	0x21b, 0x21a, 0x219, 0x218, 0x218, 0x217, 0x216, 0x215,     // 0x90
	0x214, 0x214, 0x213, 0x212, 0x211, 0x210, 0x210, 0x20f,     // 0x98
	0x20e, 0x20d, 0x20d, 0x20c, 0x20b, 0x20a, 0x209, 0x209,     // 0xA0
	0x208, 0x207, 0x206, 0x206, 0x205, 0x204, 0x203, 0x203,     // 0xA8
	0x202, 0x201, 0x200, 0x200, 0x1ff, 0x1fe, 0x1fd, 0x1fd,     // 0xB0
	0x1fc, 0x1fb, 0x1fb, 0x1fa, 0x1f9, 0x1f8, 0x1f8, 0x1f7,     // 0xB8
	0x1f6, 0x1f6, 0x1f5, 0x1f4, 0x1f3, 0x1f3, 0x1f2, 0x1f1,     // 0xC0
	0x1f1, 0x1f0, 0x1ef, 0x1ee, 0x1ee, 0x1ed, 0x1ec, 0x1ec,     // 0xC8
	0x1eb, 0x1ea, 0x1ea, 0x1e9, 0x1e8, 0x1e8, 0x1e7, 0x1e6,     // 0xD0
	0x1e6, 0x1e5, 0x1e4, 0x1e4, 0x1e3, 0x1e2, 0x1e2, 0x1e1,     // 0xD8
	0x1e0, 0x1e0, 0x1df, 0x1de, 0x1de, 0x1dd, 0x1dd, 0x1dc,     // 0xE0
	0x1db, 0x1db, 0x1da, 0x1d9, 0x1d9, 0x1d8, 0x1d7, 0x1d7,     // 0xE8
	0x1d6, 0x1d6, 0x1d5, 0x1d4, 0x1d4, 0x1d3, 0x1d2, 0x1d2,     // 0xF0
	0x1d1, 0x1d1, 0x1d0, 0x1cf, 0x1cf, 0x1ce, 0x1ce, 0x1cd,     // 0xF8
	0x1cc, 0x1cc, 0x1cb, 0x1cb, 0x1ca, 0x1c9, 0x1c9, 0x1c8,     // 0x100
	0x1c8, 0x1c7, 0x1c6, 0x1c6, 0x1c5, 0x1c5, 0x1c4, 0x1c4,     // 0x108
	0x1c3, 0x1c2, 0x1c2, 0x1c1, 0x1c1, 0x1c0, 0x1c0, 0x1bf,     // 0x110
	0x1be, 0x1be, 0x1bd, 0x1bd, 0x1bc, 0x1bc, 0x1bb, 0x1bb,     // 0x118
	0x1ba, 0x1b9, 0x1b9, 0x1b8, 0x1b8, 0x1b7, 0x1b7, 0x1b6,     // 0x120
	0x1b6, 0x1b5, 0x1b5, 0x1b4, 0x1b3, 0x1b3, 0x1b2, 0x1b2,     // 0x128
	0x1b1, 0x1b1, 0x1b0, 0x1b0, 0x1af, 0x1af, 0x1ae, 0x1ae,     // 0x130
	0x1ad, 0x1ad, 0x1ac, 0x1ac, 0x1ab, 0x1ab, 0x1aa, 0x1aa,     // 0x138
	0x1a9, 0x1a9, 0x1a8, 0x1a8, 0x1a7, 0x1a6, 0x1a6, 0x1a5,     // 0x140
	0x1a5, 0x1a4, 0x1a4, 0x1a3, 0x1a3, 0x1a2, 0x1a2, 0x1a2,     // 0x148
	0x1a1, 0x1a1, 0x1a0, 0x1a0, 0x19f, 0x19f, 0x19e, 0x19e,     // 0x150
	0x19d, 0x19d, 0x19c, 0x19c, 0x19b, 0x19b, 0x19a, 0x19a,     // 0x158
	0x199, 0x199, 0x198, 0x198, 0x197, 0x197, 0x196, 0x196,     // 0x160
	0x196, 0x195, 0x195, 0x194, 0x194, 0x193, 0x193, 0x192,     // 0x168
	0x192, 0x191, 0x191, 0x191, 0x190, 0x190, 0x18f, 0x18f,     // 0x170
	0x18e, 0x18e, 0x18d, 0x18d, 0x18c, 0x18c, 0x18c, 0x18b,     // 0x178
	0x18b, 0x18a, 0x18a, 0x189, 0x189, 0x189, 0x188, 0x188,     // 0x180
	0x187, 0x187, 0x186, 0x186, 0x185, 0x185, 0x185, 0x184,     // 0x188
	0x184, 0x183, 0x183, 0x183, 0x182, 0x182, 0x181, 0x181,     // 0x190
	0x180, 0x180, 0x180, 0x17f, 0x17f, 0x17e, 0x17e, 0x17e,     // 0x198
	0x17d, 0x17d, 0x17c, 0x17c, 0x17b, 0x17b, 0x17b, 0x17a,     // 0x1A0
	0x17a, 0x179, 0x179, 0x179, 0x178, 0x178, 0x177, 0x177,     // 0x1A8
	0x177, 0x176, 0x176, 0x175, 0x175, 0x175, 0x174, 0x174,     // 0x1B0
	0x174, 0x173, 0x173, 0x172, 0x172, 0x172, 0x171, 0x171,     // 0x1B8
	0x170, 0x170, 0x170, 0x16f, 0x16f, 0x16f, 0x16e, 0x16e,     // 0x1C0
	0x16d, 0x16d, 0x16d, 0x16c, 0x16c, 0x16c, 0x16b, 0x16b,     // 0x1C8
	0x16a, 0x16a, 0x16a, 0x169, 0x169, 0x169, 0x168, 0x168,     // 0x1D0
	0x167, 0x167, 0x167, 0x166, 0x166, 0x166, 0x165, 0x165,     // 0x1D8
	0x165, 0x164, 0x164, 0x163, 0x163, 0x163, 0x162, 0x162,     // 0x1E0
	0x162, 0x161, 0x161, 0x161, 0x160, 0x160, 0x160, 0x15f,     // 0x1E8
	0x15f, 0x15f, 0x15e, 0x15e, 0x15e, 0x15d, 0x15d, 0x15c,     // 0x1F0
	0x15c, 0x15c, 0x15b, 0x15b, 0x15b, 0x15a, 0x15a, 0x15a,     // 0x1F8
};


NDIS_STATUS
tbAtm155_ChkPhCompleteStatus(
   IN  PHARDWARE_INFO      pHwInfo,
   OUT PULONG              pData
)

/*++
Routine Description:

   This routine will check peephole command has been completed
   by checking ph_done bit or time-out happened.

Arguments:

   pHwInfo     -   Pointer to the Hardware Information block.
   pData       -   Point to be written to Peephole_Data.

Return Value:

   NDIS_STATUS_SUCCESS -   if data has been written successfully. 
   NDIS_STATUS_FAILURE -   if the Peephole operation failed.

--*/

{
   NDIS_STATUS     Status = NDIS_STATUS_FAILURE;
   ULONG           counter;    

   //
   // if peephole operation is done.           
   //
   for (*pData = 0, counter = 0;
        counter != TBATM155_TIMEOUT_PHY_DONE;
        counter++)
   {
       TBATM155_READ_PORT(
           &pHwInfo->TbAtm155_SAR->PeepHole_Data,
           pData);                                     

       if (*pData & TBATM155_PH_DONE)
       {
           Status = NDIS_STATUS_SUCCESS;
           break;
       }
   }

   return(Status);
}


NDIS_STATUS
tbAtm155_Ph_Write_WORD(
   IN  PADAPTER_BLOCK      pAdapter,
   IN  ULONG               Dst_and_CntrlCmd,
   IN  USHORT              Data
)

/*++
Routine Description:

   This routine will write data to off-chip SRAM & off-chip devices
   through Peephole registers and make sure the peephole operation
   is done by checking the Ph_Done bit of Peephole_Data register.

Arguments:

   pAdapter            -   Pointer to the adapter block.
   Dst_and_CntrlCmd    -   Destination address and control command
                           to issue to Peephole_Cmd.
   Data                -   To be written to Peephole_Data.

Return Value:

   NDIS_STATUS_SUCCESS -   if data has been written successfully. 
   NDIS_STATUS_FAILURE -   if the Peephole operation failed.

--*/

{
   PHARDWARE_INFO          pHwInfo = pAdapter->HardwareInfo;
   ULONG                   temp;
   NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;

   
   temp = (ULONG)Data;

   NdisAcquireSpinLock(&pHwInfo->Lock);

   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->PeepHole_Data,
       temp);               
                               
   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->PeepHole_Cmd,
       Dst_and_CntrlCmd);

   NdisReleaseSpinLock(&pHwInfo->Lock);

   Status = tbAtm155_ChkPhCompleteStatus(pHwInfo, &temp);

   return (Status);

}



NDIS_STATUS
tbAtm155_Ph_Read_WORD(
   IN  PADAPTER_BLOCK      pAdapter,
   IN  ULONG               Dst_and_CntrlCmd,
   OUT PUSHORT             pData
   )

/*++
Routine Description:

   This routine will read data from off-chip SRAM & devices through
   Peephole registers and make sure the peephole operation is done by
   checking the Ph_Done bit of Peephole_Data.

Arguments:

   pAdapter            -   Pointer to the adapter block.
   Dst_and_CntrlCmd    -   Destination address in SRAM address.
   pData               -   Pointer of SRAM data for return to caller.

Return Value:

   NDIS_STATUS_SUCCESS -   if data has been read successfully. 
   NDIS_STATUS_FAILURE -   if the Peephole operation failed.

--*/

{
   PHARDWARE_INFO          pHwInfo = pAdapter->HardwareInfo;
   ULONG                   temp;
   NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;

   NdisAcquireSpinLock(&pHwInfo->Lock);

   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->PeepHole_Cmd,
       Dst_and_CntrlCmd);

   Status = tbAtm155_ChkPhCompleteStatus(pHwInfo, &temp);

   NdisReleaseSpinLock(&pHwInfo->Lock);

   // clear Ph_Done bit
   *pData = (USHORT)(temp & 0x0FFFF);

   return (Status);
}




//////////////////////////////////////////////////////////////////////////
//																		
//         Related Routines of reading node address from EEPROM
//
//////////////////////////////////////////////////////////////////////////



VOID
SetRead93C46StructureWord(
   IN  PUSHORT OperateArray,
   IN  USHORT  address,
   OUT PUCHAR   length
)
/*++

Routine Description:

   This routine sets up array of EEPORM operation for reading node address.

Arguments:

Return Value:

--*/
{

   UCHAR   OpBitStr[RD_INSTRUCTION_LEN_93C46];
   UCHAR   AddrBitStr[ADDRESS_LENGTH2_93C46];
   UCHAR   i, j, temp;
   USHORT  ConvertData;

   //
   //  Convert RD_INSTRUCTION_93C46 command into a bit string.
   //
   for (i = RD_INSTRUCTION_LEN_93C46, ConvertData = RD_INSTRUCTION_93C46;
        i > 0;
        i--, ConvertData >>= 1)
   {
       OpBitStr[i-1] = (UCHAR)(ConvertData & 1);
   }

   //
   //  The instruction followed is to set up clock pulse in advance for
   //  reading serial EEPROM for circuit repuirements before reading
   //  the chip.
   //
   OperateArray[0] = 0;
   OperateArray[1] = CHIP_SELECT;
   j = 2;
   for (i = 0; i < RD_INSTRUCTION_LEN_93C46; i++)
   {
       temp = (i * 2) + j;
       OperateArray[temp] = (OpBitStr[i] == 0)?
                               (USHORT)(CHIP_SELECT) :
                               (USHORT)(CHIP_SELECT | SERIAL_DATA_INPUT);

       OperateArray[temp+1] = OperateArray[temp] | SERIAL_DATA_CLOCK;
   }

   j += i*2;

   //
   //  Convert "address" into a bit string.
   //
   for (i = ADDRESS_LENGTH2_93C46, ConvertData = address;
        i > 0;
        i--, ConvertData >>= 1)
   {
       AddrBitStr[i-1] = (UCHAR)(ConvertData & 1);
   }

   //
   //  The instruction followed keeps on setting up clock pulse in advance
   //  for reading serial EEPROM for circuit repuirements before reading
   //  the chip.
   //
   for (i = 0; i < ADDRESS_LENGTH2_93C46; i++)
   {
       temp = (i * 2) + j;
       OperateArray[temp] = (AddrBitStr[i] == 0)?
                               (USHORT)(CHIP_SELECT) :
                               (USHORT)(CHIP_SELECT | SERIAL_DATA_INPUT);

       OperateArray[temp+1] = OperateArray[temp] | SERIAL_DATA_CLOCK;
   }

   *length = j + (i * 2);

}


NDIS_STATUS
ReadSerialEEPROM93C46Word(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  USHORT          address,
   OUT PUSHORT         pData
)
/*++

Routine Description:

   This routine reads in 1-word of node address from serial EEPROM.   

Arguments:

   pAdapter    -   Pointer to the adapter to program.
   address     -   Offset of node address will be read.
   pData       -   Pointer to the read-in node address.

Return Value:

   NDIS_STATUS_FAILURE -   fail to read a word size of the node address
                           from EEPROM.
   NDIS_STATUS_SUCCESS -   otherwise.

--*/

{
   NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
   USHORT          OperateArray[ 2 + (2*RD_INSTRUCTION_93C46) + (2*ADDRESS_LENGTH2_93C46)];
   UCHAR           length, i;
	ULONG           data1;


   DBGPRINT(DBG_COMP_PEEPHOLE, DBG_LEVEL_INFO,
       ("==>ReadSerialEEPROM93C46Word\n"));


   SetRead93C46StructureWord(OperateArray, address, &length);
   
   do 
   {
       for ( i = 0; ( i < length ); i++ )
       {
           TBATM155_PH_WRITE_DEV(
               pAdapter,
               MAC_ADDR_ROM_OFFSET,
               OperateArray[i],
               &Status);

           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_PEEPHOLE, DBG_LEVEL_ERR,
                   ("Failed to access mac address 1.\n"));

               break;
           }

	    }

       if (NDIS_STATUS_SUCCESS != Status)
           break;


       TBATM155_PH_WRITE_DEV(
           pAdapter,
           MAC_ADDR_ROM_OFFSET,
           CHIP_SELECT,
           &Status);

       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_PEEPHOLE, DBG_LEVEL_ERR,
               ("Failed to access mac address 2.\n"));

           break;
       }

       //
       // read 1 word data from EEPROM now.
       //
       for ( i = 0, *pData = 0; (i < 16); i++ ) 
       {
           //
           // raise data clock to get ready of 
           // reading data from EEPROM 93C46.
           //
           TBATM155_PH_WRITE_DEV(
               pAdapter,
               MAC_ADDR_ROM_OFFSET,
               CHIP_SELECT | SERIAL_DATA_CLOCK,
               &Status);

           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_PEEPHOLE, DBG_LEVEL_ERR,
                  ("Failed to access mac address 3.\n"));

               break;
           }

           //
           // read 1 bit in from serial EEPROM.
           //
           TBATM155_PH_READ_DEV(
               pAdapter,
               MAC_ADDR_ROM_OFFSET,
               &data1,
               &Status);

           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_PEEPHOLE, DBG_LEVEL_ERR,
                  ("Failed to access mac address 4.\n"));

               break;
           }

           //
           // To merge the input 1-bit data into word data.
           //
           *pData <<= 1;
           *pData |= (data1 & 0x8)? 1 : 0;

           //
           // Drop the data clock of EEPROM.
           //
           TBATM155_PH_WRITE_DEV(
               pAdapter,
               MAC_ADDR_ROM_OFFSET,
               CHIP_SELECT,
               &Status);

           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_PEEPHOLE, DBG_LEVEL_ERR,
                  ("Failed to access mac address 5.\n"));

               break;
           }
   	}

   } while (FALSE);

   DBGPRINT(DBG_COMP_PEEPHOLE, DBG_LEVEL_INFO,
       ("<==ReadSerialEEPROM93C46Word: odata=0x%x\n", *pData));

   return (Status);

}


NDIS_STATUS
tbAtm155Read_PermanentAddress(
   IN  PADAPTER_BLOCK  pAdapter,
   OUT PUCHAR          pNodeaddress
)
/*++

Routine Description:

   This routine reads node address from EEPROM 93C46.

Arguments:

   pPermanentAddress	-	Pointer to the node address.

Return Value:

   NDIS_STATUS_FAILURE -   if fail to read the node address from EEPROM.
   NDIS_STATUS_SUCCESS -   otherwise.

--*/

{
   NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
   USHORT              data;
   UCHAR               i;


   DBGPRINT(DBG_COMP_PEEPHOLE, DBG_LEVEL_INFO,
       ("==>tbAtm155Read_PermanentAddress\n"));


   for ( i = 0; i < ATM_MAC_ADDRESS_LENGTH; i += 2 )
   {
       Status = ReadSerialEEPROM93C46Word(pAdapter, (USHORT)(i >> 1), &data);

       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_PEEPHOLE, DBG_LEVEL_ERR,
               ("tbAtm155Read_PermanentAddress error at offset (%d).\n", i));

           break;
       }
       
       pNodeaddress[i] = (UCHAR)( data >> 8 );
       pNodeaddress[i+1] = (UCHAR)( data & 0x00FF );
   }

   if (NDIS_STATUS_SUCCESS == Status)
   {
       // 
       //  check if node address is invalid. 
       //  (an invalid node address: ff-ff-ff-ff-ff-ff)
       // 
       for (Status = NDIS_STATUS_FAILURE, i = ATM_MAC_ADDRESS_LENGTH - 1;
            i >= 0;
            i--)
       {
           if (pNodeaddress[i] != 0x0FF)
           {
               Status = NDIS_STATUS_SUCCESS;
               break;
           }
   
       }
   }


   if (NDIS_STATUS_SUCCESS != Status)
   {
       DBGPRINT(DBG_COMP_PEEPHOLE, DBG_LEVEL_ERR,
           ("tbAtm155Read_PermanentAddress is bad.\n"));
   }
       
   DBGPRINT(DBG_COMP_PEEPHOLE, DBG_LEVEL_INFO,
       ("<==tbAtm155Read_PermanentAddress\n"));

	return(Status);

}



