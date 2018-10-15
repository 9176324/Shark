/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1989-1999  Microsoft Corporation

Module Name:

    ioevent.h

Abstract:

    This module contains the GUIDS and event structures for io system
    initiated events.  These events are reported in kernel mode and are
    available to both user mode and kernel mode clients.

Author:

    Mark Zbikowski (markz) 3/18/98


Revision History:


--*/

//
//  Label change event.  This event is signalled upon successful completion
//  of a label change.  There is no additional data.
//

DEFINE_GUID( GUID_IO_VOLUME_CHANGE, 0x7373654aL, 0x812a, 0x11d0, 0xbe, 0xc7, 0x08, 0x00, 0x2b, 0xe2, 0x09, 0x2f );

//
//  Volume dismount event.  This event is signalled when an attempt is made to
//  dismount a volume.  There is no additional data.  Note that this will not
//  necessarily be preceded by a GUID_IO_VOLUME_LOCK notification.
//

DEFINE_GUID( GUID_IO_VOLUME_DISMOUNT, 0xd16a55e8L, 0x1059, 0x11d2, 0x8f, 0xfd, 0x00, 0xa0, 0xc9, 0xa0, 0x6d, 0x32 );

//
//  Volume dismount failed event.  This event is signalled when a volume dismount fails.
//  There is no additional data.
//

DEFINE_GUID( GUID_IO_VOLUME_DISMOUNT_FAILED, 0xe3c5b178L, 0x105d, 0x11d2, 0x8f, 0xfd, 0x00, 0xa0, 0xc9, 0xa0, 0x6d, 0x32 );

//
//  Volume mount event.  This event is signalled when a volume mount occurs.
//  There is no additional data.
//

DEFINE_GUID( GUID_IO_VOLUME_MOUNT, 0xb5804878L, 0x1a96, 0x11d2, 0x8f, 0xfd, 0x00, 0xa0, 0xc9, 0xa0, 0x6d, 0x32 );

//
//  Volume lock event.  This event is signalled when an attempt is made to
//  lock a volume.  There is no additional data.
//

DEFINE_GUID( GUID_IO_VOLUME_LOCK, 0x50708874L, 0xc9af, 0x11d1, 0x8f, 0xef, 0x00, 0xa0, 0xc9, 0xa0, 0x6d, 0x32 );

//
//  Volume lock failed event.  This event is signalled when an attempt is made to
//  lock a volume, but it fails.  There is no additional data.
//

DEFINE_GUID( GUID_IO_VOLUME_LOCK_FAILED, 0xae2eed10L, 0x0ba8, 0x11d2, 0x8f, 0xfb, 0x00, 0xa0, 0xc9, 0xa0, 0x6d, 0x32 );


//
//  Volume unlock event.  This event is signalled when an attempt is made to
//  unlock a volume.  There is no additional data.
//

DEFINE_GUID( GUID_IO_VOLUME_UNLOCK, 0x9a8c3d68L, 0xd0cb, 0x11d1, 0x8f, 0xef, 0x00, 0xa0, 0xc9, 0xa0, 0x6d, 0x32 );


//
//  Volume name change.  This event is signalled when the list of persistent
//  names (like drive letters) for a volume changes.  There is no additional
//  data.
//

DEFINE_GUID( GUID_IO_VOLUME_NAME_CHANGE, 0x2de97f83, 0x4c06, 0x11d2, 0xa5, 0x32, 0x0, 0x60, 0x97, 0x13, 0x5, 0x5a);


//
//  Volume physical configuration change.  This event is signalled when the
//  physical makeup or current physical state of the volume changes.
//

DEFINE_GUID( GUID_IO_VOLUME_PHYSICAL_CONFIGURATION_CHANGE, 0x2de97f84, 0x4c06, 0x11d2, 0xa5, 0x32, 0x0, 0x60, 0x97, 0x13, 0x5, 0x5a);


//
//  Volume device interface.  This is a device interface GUID that appears
//  when the device object associated with a volume is created and disappears
//  when the device object associated with the volume is destroyed.
//

DEFINE_GUID( GUID_IO_VOLUME_DEVICE_INTERFACE, 0x53f5630d, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);



//
//  Sent when the removable media is changed (added, removed) from a device
//  (such as a CDROM, tape, changer, etc).
//
//  The additional data is a DWORD representing the data event.
//

DEFINE_GUID( GUID_IO_MEDIA_ARRIVAL, 0xd07433c0, 0xa98e, 0x11d2, 0x91, 0x7a, 0x00, 0xa0, 0xc9, 0x06, 0x8f, 0xf3);
DEFINE_GUID( GUID_IO_MEDIA_REMOVAL, 0xd07433c1, 0xa98e, 0x11d2, 0x91, 0x7a, 0x00, 0xa0, 0xc9, 0x06, 0x8f, 0xf3);

typedef struct _DEVICE_EVENT_GENERIC_DATA {
    ULONG EventNumber;
} DEVICE_EVENT_GENERIC_DATA, *PDEVICE_EVENT_GENERIC_DATA;


//
//  Represents any asynchronous notification coming from a device driver whose
//  notification protocol is RBC
//  Additional data is provided

DEFINE_GUID( GUID_DEVICE_EVENT_RBC, 0xd0744792, 0xa98e, 0x11d2, 0x91, 0x7a, 0x00, 0xa0, 0xc9, 0x06, 0x8f, 0xf3);

typedef struct _DEVICE_EVENT_RBC_DATA {
    ULONG EventNumber;
    UCHAR SenseQualifier;
    UCHAR SenseCode;
    UCHAR SenseKey;
    UCHAR Reserved;
    ULONG Information;
} DEVICE_EVENT_RBC_DATA, *PDEVICE_EVENT_RBC_DATA;

