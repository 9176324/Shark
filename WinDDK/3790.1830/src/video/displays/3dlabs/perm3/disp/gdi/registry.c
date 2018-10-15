/******************************Module*Header**********************************\
*
*                           *******************
*                           * GDI SAMPLE CODE *
*                           *******************
*
* Module Name: registry.c
*
* Content: Routines to initialize the registry and lookup string values.
*
* Copyright (c) 1994-1999 3Dlabs Inc. Ltd. All rights reserved.
* Copyright (c) 1995-2002 Microsoft Corporation.  All rights reserved.
\*****************************************************************************/

#include "precomp.h"

/******************************Public*Data*Struct**************************\
* BOOL bGlintQueryRegistryValueUlong
*
* Take a string and look up its value in the registry. We assume that the
* value fits into 4 bytes. Fill in the supplied DWORD pointer with the value.
*
* Returns:
*   TRUE if we found the string, FALSE if not. Note, if we failed to init
*   the registry the query funtion will simply fail and we act as though
*   the string was not defined.
*
\**************************************************************************/

BOOL
bGlintQueryRegistryValueUlong(
    PPDEV   ppdev, 
    LPWSTR  valueStr, 
    PULONG  pData)
{
    ULONG   ReturnedDataLength;
    ULONG   inSize;
    ULONG   outData;
    PWCHAR  inStr;
    
    // get the string length including the NULL
    for (inSize = 2, inStr = valueStr; *inStr != 0; ++inStr, inSize += 2);

    if (EngDeviceIoControl(
            ppdev->hDriver,
            IOCTL_VIDEO_QUERY_REGISTRY_DWORD,
            valueStr,                        // input buffer
            inSize,
            &outData,                        // output buffer
            sizeof(ULONG),
            &ReturnedDataLength))
    {
        DISPDBG((WRNLVL, "bGlintQueryRegistryValueUlong failed"));
        return(FALSE);
    }
    
    *pData = outData;
    DISPDBG((DBGLVL, "bGlintQueryRegistryValueUlong "
                     "returning 0x%x (ReturnedDataLength = %d)",
                     *pData, ReturnedDataLength));
    return (TRUE);
}


