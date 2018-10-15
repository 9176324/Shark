/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       data.c
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920


Abstract:

	This file contains global definitions.

Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	01/07/97		awang		Initial of Toshiba ATM 155 Device Driver.
	05/25/97		awang		Removed Registry SlotNumber & BusNumber.

--*/

#include "precomp.h"
#pragma hdrstop

#define	MODULE_NUMBER	MODULE_DATA

NDIS_HANDLE		gWrapperHandle = NULL;
NDIS_STRING		gaRegistryParameterString[TbAtm155MaxRegistryEntry] =
{
	NDIS_STRING_CONST("VcHashTableSize"),
	NDIS_STRING_CONST("TotalRxBuffs"),
	NDIS_STRING_CONST("BigReceiveBufferSize"),
	NDIS_STRING_CONST("SmallReceiveBufferSize"),
	NDIS_STRING_CONST("NumberofMapRegisters")
};


ULONG			gDmaAlignmentRequired = 0;
