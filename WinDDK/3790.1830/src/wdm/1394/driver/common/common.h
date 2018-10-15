/*++

Copyright (c) 1999  Microsoft Corporation

Module Name: 

    common.h

Abstract


Author:

    Kashif Hasan (khasan) 3/18/01

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
7/13/99  pbinder   birth (parts taken from 1394diag.h)
3/18/01  khasan    removed 1394vdev/1394diag internal information
--*/


#ifndef _1394_COMMON_H_
#define _1394_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "1394api.h"

// {C459DF55-DB08-11d1-B009-00A0C9081FF6}
DEFINE_GUID(GUID_1394DIAG, 0xc459df55, 0xdb08, 0x11d1, 0xb0, 0x9, 0x0, 0xa0, 0xc9, 0x8, 0x1f, 0xf6);

// {737613E5-69EA-4b96-9C2A-EEBC220F4C39}
DEFINE_GUID(GUID_1394VDEV, 0x737613e5, 0x69ea, 0x4b96, 0x9c, 0x2a, 0xee, 0xbc, 0x22, 0xf, 0x4c, 0x39);

//
// these guys are meant to be called from a ring 3 app
// call through the port device object
//
#define IOCTL_1394_TOGGLE_ENUM_TEST_ON          CTL_CODE( \
                                                FILE_DEVICE_UNKNOWN, \
                                                0x88, \
                                                METHOD_BUFFERED, \
                                                FILE_ANY_ACCESS \
                                                )

#define IOCTL_1394_TOGGLE_ENUM_TEST_OFF         CTL_CODE( \
                                                FILE_DEVICE_UNKNOWN, \
                                                0x89, \
                                                METHOD_BUFFERED, \
                                                FILE_ANY_ACCESS \
                                                )

//
// IOCTL info, needs to be visible for application
//
#define DIAG1394_IOCTL_INDEX                            0x0800


#define IOCTL_ALLOCATE_ADDRESS_RANGE                    CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 0,       \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_FREE_ADDRESS_RANGE                        CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 1,       \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ASYNC_READ                                CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 2,       \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ASYNC_WRITE                               CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 3,       \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ASYNC_LOCK                                CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 4,       \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ISOCH_ALLOCATE_BANDWIDTH                  CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 5,       \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ISOCH_ALLOCATE_CHANNEL                    CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 6,       \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ISOCH_ALLOCATE_RESOURCES                  CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 7,       \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ISOCH_ATTACH_BUFFERS                      CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 8,       \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ISOCH_DETACH_BUFFERS                      CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 9,       \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ISOCH_FREE_BANDWIDTH                      CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 10,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ISOCH_FREE_CHANNEL                        CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 11,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ISOCH_FREE_RESOURCES                      CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 12,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ISOCH_LISTEN                              CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 13,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ISOCH_QUERY_CURRENT_CYCLE_TIME            CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 14,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ISOCH_QUERY_RESOURCES                     CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 15,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ISOCH_SET_CHANNEL_BANDWIDTH               CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 16,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ISOCH_STOP                                CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 17,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ISOCH_TALK                                CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 18,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_GET_LOCAL_HOST_INFORMATION                CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 19,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_GET_1394_ADDRESS_FROM_DEVICE_OBJECT       CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 20,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_CONTROL                                   CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 21,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_GET_MAX_SPEED_BETWEEN_DEVICES             CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 22,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_SET_DEVICE_XMIT_PROPERTIES                CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 23,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_GET_CONFIGURATION_INFORMATION             CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 24,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_BUS_RESET                                 CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 25,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_GET_GENERATION_COUNT                      CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 26,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_SEND_PHY_CONFIGURATION_PACKET             CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 27,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_BUS_RESET_NOTIFICATION                    CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 28,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ASYNC_STREAM                              CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 29,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_SET_LOCAL_HOST_INFORMATION                CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 30,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_SET_ADDRESS_DATA                          CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 40,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_GET_ADDRESS_DATA                          CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 41,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_BUS_RESET_NOTIFY                          CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 50,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)
                                                        
#define IOCTL_GET_DIAG_VERSION                          CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 51,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)

#define IOCTL_ISOCH_MODIFY_STREAM_PROPERTIES            CTL_CODE( FILE_DEVICE_UNKNOWN,  \
                                                        DIAG1394_IOCTL_INDEX + 52,      \
                                                        METHOD_BUFFERED,                \
                                                        FILE_ANY_ACCESS)


#ifdef __cplusplus
}
#endif

#endif // #ifndef _1394_COMMON_H_


