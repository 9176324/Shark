/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.
 
   File:       tbAtm155.h
 
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

#ifndef __TBATM155_H
#define __TBATM155_H

//
//	Module identifiers.
//	
#define MODULE_DEBUG       0x00010000
#define MODULE_INIT        0x00020000
#define MODULE_INT         0x00030000
#define MODULE_RECEIVE     0x00040000
#define MODULE_REQUEST     0x00050000
#define MODULE_RESET       0x00060000
#define MODULE_SEND        0x00070000
#define MODULE_VC          0x00080000
#define MODULE_SUPPORT     0x00090000
#define MODULE_DATA        0x000a0000
#define MODULE_PEEPHOLE    0x000b0000
#define MODULE_SUNI_LITE   0x000c0000
#define MODULE_PLC2        0x000d0000
#define MODULE_TBMETEOR    0x000e0000


//
//	Extern definitions for global data.
//

extern	NDIS_HANDLE	gWrapperHandle;
extern	NDIS_STRING	gaRegistryParameterString[];
extern	ULONG gDmaAlignmentRequired;


#endif // __TBATM155_H

