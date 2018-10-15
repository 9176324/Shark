/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    DockIntf.h

Abstract:

    This header defines the Dock Interface

--*/

DEFINE_GUID(GUID_DOCK_INTERFACE,
            0xa9956ff5L, 0x13da, 0x11d3,
            0x97, 0xdb, 0x00, 0xa0, 0xc9, 0x40, 0x52, 0x2e );

#ifndef _DOCKINTF_H_
#define _DOCKINTF_H_

//
// The interface returned consists of the following structure and functions.
//

#define DOCK_INTRF_STANDARD_VER   1

typedef enum {

    PDS_UPDATE_DEFAULT = 1,
    PDS_UPDATE_ON_REMOVE,
    PDS_UPDATE_ON_INTERFACE,
    PDS_UPDATE_ON_EJECT

} PROFILE_DEPARTURE_STYLE;

typedef ULONG (* PFN_PROFILE_DEPARTURE_SET_MODE)(
    IN  PVOID                   Context,
    IN  PROFILE_DEPARTURE_STYLE Style
    );

typedef ULONG (* PFN_PROFILE_DEPARTURE_UPDATE)(
    IN  PVOID   Context
    );

typedef struct {

    struct _INTERFACE; // Unnamed struct

    PFN_PROFILE_DEPARTURE_SET_MODE  ProfileDepartureSetMode;
    PFN_PROFILE_DEPARTURE_UPDATE    ProfileDepartureUpdate;

} DOCK_INTERFACE, *PDOCK_INTERFACE;

#endif // _DOCKINTF_H_

