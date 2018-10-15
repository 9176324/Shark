/*++

Copyright (c) 1991 - 1993 Microsoft Corporation

Module Name:

    sffprtcl.h

Abstract:

    Definition for Small Form Factor disk (SFFDISK) protocol layer interface

Author:

    Neil Sandlin (neilsa) 1-Dec-02

Environment:

    Kernel mode only.

Notes:


--*/

#ifndef _SFFPRTCL_H_
#define _SFFPRTCL_H_


DEFINE_GUID(GUID_SFF_PROTOCOL_INTERFACE_STANDARD,  0xc7ec8da0L, 0xdbe3, 0x43fd, 0xa9, 0xeb, 0x7e, 0x4c, 0x70, 0x7c, 0x35, 0xac);

#define SFF_PROTOCOL_INTERFACE_VERSION 0x0102

//
// Property types used in SFFPROT_Get/Set_Property
//

typedef enum {
    SFFP_PROP_MEDIA_CAPACITY,
    SFFP_PROP_PARTITION_SIZE,
    SFFP_PROP_WRITE_PROTECTED,
    SFFP_PROP_MEDIA_STATE,
    SFFP_PROP_MEDIA_CHANGECOUNT,
    SFFP_PROP_MEDIA_ID,
    SFFP_PROP_PROTOCOL_GUID,
    SFFP_PROP_VERIFY_STATE,
    SFFP_PROP_PARTITION_START_OFFSET
} SFFPROT_PROPERTY;

//
// Media states defined for SFFP_PROP_MEDIA_STATE
//

typedef enum {
    SFFMS_NO_MEDIA = 0,
    SFFMS_MEDIA_PRESENT
} SFFPROT_MEDIA_STATE;

//
// Media states defined for SFFP_PROP_MEDIA_STATE
//

typedef enum {
    SFFVS_VERIFY_REQUIRED = 0,            // the bus interface layer has noticed the media changed
    SFFVS_VERIFY_ACKNOWLEDGED             // the file system has started the verify
} SFFPROT_VERIFY_STATE;

//
// types used in DeviceControl
//

typedef enum {
    SFFDC_DEVICE_COMMAND,
    SFFDC_DEVICE_PASSWORD
} SFFPROT_DCTYPE;

//
// Prototypes for the get/set property calls
//

typedef
NTSTATUS
(*PSFFPROT_GET_PROPERTY)(
    IN PVOID Context,
    IN SFFPROT_PROPERTY Property,
    IN ULONG BufferLength,
    OUT PVOID PropertyBuffer,
    OUT PULONG ResultLength
    );

typedef
NTSTATUS
(*PSFFPROT_SET_PROPERTY)(
    IN PVOID Context,
    IN SFFPROT_PROPERTY Property,
    IN ULONG BufferLength,
    IN PVOID PropertyBuffer
    );

typedef
NTSTATUS
(*PSFFPROT_DEVICE_CONTROL)(
    IN PVOID Context,
    IN SFFPROT_DCTYPE Type,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG LengthReturned
    );

typedef
NTSTATUS
(*PSFFPROT_READ)(
    IN PVOID Context,
    IN PMDL Mdl,
    IN ULONGLONG Offset,
    IN ULONG Length,
    OUT PULONG LengthReturned
    );

typedef
NTSTATUS
(*PSFFPROT_WRITE)(
    IN PVOID Context,
    IN PMDL Mdl,
    IN ULONGLONG Offset,
    IN ULONG Length,
    OUT PULONG LengthReturned
    );

//
// This typedef defines the interface structure to be returned by
// the pnp QUERY_INTERFACE call.
//


typedef struct _SFF_PROTOCOL_INTERFACE_STANDARD {
   USHORT Size;
   USHORT Version;
   PINTERFACE_REFERENCE    InterfaceReference;
   PINTERFACE_DEREFERENCE  InterfaceDereference;
   PVOID                   Context;
   PSFFPROT_GET_PROPERTY   GetProperty;
   PSFFPROT_SET_PROPERTY   SetProperty;
   PSFFPROT_READ           Read;
   PSFFPROT_WRITE          Write;
   PSFFPROT_DEVICE_CONTROL DeviceControl;
} SFF_PROTOCOL_INTERFACE_STANDARD, *PSFF_PROTOCOL_INTERFACE_STANDARD;


#endif  // _SFFPRTCL_H_

