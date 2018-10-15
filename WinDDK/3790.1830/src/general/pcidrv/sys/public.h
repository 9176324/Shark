/*++
    Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Author:

    Eliyas Yakub
     
Environment:

    user and kernel
Notes:


Revision History:


--*/
  
//
// Define an Interface Guid for toaster device class.
// This GUID is used to register (IoRegisterDeviceInterface) 
// an instance of an interface so that user application 
// can control the toaster device.
//

DEFINE_GUID (GUID_DEVINTERFACE_PCIDRV, 
    0xb74cfec2, 0x9366, 0x454a, 0xba, 0x71, 0x7c, 0x27, 0xb5, 0x14, 0x70, 0xa4);
// {B74CFEC2-9366-454a-BA71-7C27B51470A4}

//
// Define a WMI GUID to get toaster device info.
//

DEFINE_GUID (PCIDRV_WMI_STD_DATA_GUID, 
    0x20e35e40, 0x7179, 0x4f89, 0xa2, 0x8c, 0x12, 0xed, 0x5a, 0x3c, 0xaa, 0xa5);

// {20E35E40-7179-4f89-A28C-12ED5A3CAAA5}

//
// WMI GUID to set/get conservation idle time. The ConservationIdleTime value 
// applies when the system power policy optimizes for conservation.
// Typically, the applicable policy depends upon the power source: 
// when running with AC power, the system optimizes for performance, and when
// running off a battery, the system optimizes for conservation. 
//

DEFINE_GUID (GUID_POWER_CONSERVATION_IDLE_TIME, 
    0x106a479, 0x477c, 0x4aef, 0x91, 0xc2, 0xdd, 0x96, 0x66, 0xd0, 0xf4, 0x9);

// {0106A479-477C-4aef-91C2-DD9666D0F409}


//
// WMI GUID to set/get performance idle time. The PerformanceIdleTime value 
// applies when the system power policy optimizes for performance. 
// Typically, the applicable policy depends upon the power source: 
// when running with AC power, the system optimizes for performance, and when
// running off a battery, the system optimizes for conservation. 
//

DEFINE_GUID (GUID_POWER_PERFORMANCE_IDLE_TIME, 
    0x3e426488, 0x5b3a, 0x4353, 0xb0, 0xfc, 0x53, 0x7f, 0xe8, 0xf8, 0x2d, 0x7);
// {3E426488-5B3A-4353-B0FC-537FE8F82D07}

//
// GUID definition are required to be outside of header inclusion pragma to avoid
// error during precompiled headers.
//
