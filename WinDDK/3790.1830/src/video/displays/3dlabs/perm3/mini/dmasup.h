/**************************************************************************\
*
*                        ************************
*                        * MINIPORT SAMPLE CODE *
*                        ************************
*
* Module Name:
*
*   dmasup.h
*
* Abstract:
*
*  This module contains all the global data used by the Permedia3 driver.
*
* Environment:
*
*   Kernel mode
*
*
* Copyright (c) 1994-1999 3Dlabs Inc. Ltd. All rights reserved.
* Copyright (c) 1995-2002 Microsoft Corporation.  All Rights Reserved.
*
\***************************************************************************/

#include "miniddk.h"


// Global function to initialize DMA manager

BOOLEAN
Perm3InitializeDMA(PVOID);

// DMA interrupt handler
BOOLEAN
Perm3DMAInterruptHandler(
    PVOID HwDeviceExtension);


