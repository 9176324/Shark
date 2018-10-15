/*++

Copyright (c) 1998-1998 Microsoft Corporation

Module Name:

    DEVNODE.C

--*/

//*****************************************************************************
// I N C L U D E S
//*****************************************************************************

#include <windows.h>
#include <basetyps.h>
#include <cfgmgr32.h>

#include <string.h>

//*****************************************************************************
// G L O B A L S
//*****************************************************************************

CHAR buf[MAX_DEVICE_ID_LEN];  // (Dynamically size this instead?)

//*****************************************************************************
//
// DriverNameToDeviceDesc()
//
// Returns the Device Description of the DevNode with the matching DriverName.
// Returns NULL if the matching DevNode is not found.
//
// The caller should copy the returned string buffer instead of just saving
// the pointer value.  (Dynamically allocate return buffer instead?)
//
//*****************************************************************************

PCHAR DriverNameToDeviceDesc (PCHAR DriverName, BOOLEAN DeviceId)
{
    DEVINST     devInst;
    DEVINST     devInstNext;
    CONFIGRET   cr;
    ULONG       walkDone = 0;
    ULONG       len;

    // Get Root DevNode
    //
    cr = CM_Locate_DevNode(&devInst,
                           NULL,
                           0);

    if (cr != CR_SUCCESS)
    {
        return NULL;
    }

    // Do a depth first search for the DevNode with a matching
    // DriverName value
    //
    while (!walkDone)
    {
        // Get the DriverName value
        //
        len = sizeof(buf);
        cr = CM_Get_DevNode_Registry_Property(devInst,
                                              CM_DRP_DRIVER,
                                              NULL,
                                              buf,
                                              &len,
                                              0);

        // If the DriverName value matches, return the DeviceDescription
        //
        if (cr == CR_SUCCESS && _stricmp(DriverName, buf) == 0)
        {
            len = sizeof(buf);

            if (DeviceId)
            {
                cr = CM_Get_Device_ID(devInst,
                                      buf,
                                      len,
                                      0);
            }
            else
            {
                cr = CM_Get_DevNode_Registry_Property(devInst,
                                                      CM_DRP_DEVICEDESC,
                                                      NULL,
                                                      buf,
                                                      &len,
                                                      0);
            }

            if (cr == CR_SUCCESS)
            {
                return buf;
            }
            else
            {
                return NULL;
            }
        }

        // This DevNode didn't match, go down a level to the first child.
        //
        cr = CM_Get_Child(&devInstNext,
                          devInst,
                          0);

        if (cr == CR_SUCCESS)
        {
            devInst = devInstNext;
            continue;
        }

        // Can't go down any further, go across to the next sibling.  If
        // there are no more siblings, go back up until there is a sibling.
        // If we can't go up any further, we're back at the root and we're
        // done.
        //
        for (;;)
        {
            cr = CM_Get_Sibling(&devInstNext,
                                devInst,
                                0);

            if (cr == CR_SUCCESS)
            {
                devInst = devInstNext;
                break;
            }

            cr = CM_Get_Parent(&devInstNext,
                               devInst,
                               0);


            if (cr == CR_SUCCESS)
            {
                devInst = devInstNext;
            }
            else
            {
                walkDone = 1;
                break;
            }
        }
    }

    return NULL;
}

