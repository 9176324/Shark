/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    Public.h

Abstract:

    Header file common to app and the driver. Contains ioctl definitions
    
Author:

     Eliyas Yakub   
     
Environment:

     User & Kernel-mode

Revision History:


--*/

#define IOCTL_NETVMINI_READ_DATA \
    CTL_CODE (FILE_DEVICE_UNKNOWN, 0, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_NETVMINI_WRITE_DATA \
    CTL_CODE (FILE_DEVICE_UNKNOWN, 1, METHOD_BUFFERED, FILE_WRITE_ACCESS)


