/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       tboid.h

               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920


Author:

   J. Cao

Environment:

	Kernel mode

Revision History:

	3/25/98     J. Cao              Initial of Toshiba ATM 155 Device Driver.

--*/

typedef struct {                   // structure to access Peep-hole
   ULONG   address;                // [17-0]
   USHORT  data;                   //	[15-0]
   USHORT  access_device;          // [0], 1-Peripheral, 0-SRAM
   ULONG   status;                 // [0], 1-Success, 0-Fail (address range over)
} TB_PEEPHOLE,  *PTB_PEEPHOLE;


typedef struct {                   // structure to access CSR
   ULONG   address;                // [5-0], in long word address
   ULONG	data;	                // [31-0]
   ULONG	status;                 // [0], 1-Success, 0-Fail (address range over)
} TB_CSR,  *PTB_CSR;


// MSB use 0xFF (indicating implementation specific OID)
// LSB     XX (the custom OID number - providing 255 possible custom oids)

#define OID_TBATM_READ_PEEPHOLE    0xFF102F02
#define OID_TBATM_READ_CSR	        0xFF102F04
#define OID_VENDOR_STRING	        0xFF102F10

