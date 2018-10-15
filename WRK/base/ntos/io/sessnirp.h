/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    sessnirp.h

Abstract:

--*/


#ifndef _SESSIONIRP_H_
#define _SESSIONIRP_H_

PIOV_SESSION_DATA
FASTCALL
IovpSessionDataCreate(
    IN      PDEVICE_OBJECT       DeviceObject,
    IN OUT  PIOV_REQUEST_PACKET  *IovPacketPointer,
    OUT     PBOOLEAN             SurrogateSpawned
    );

VOID
FASTCALL
IovpSessionDataAdvance(
    IN      PDEVICE_OBJECT       DeviceObject,
    IN      PIOV_SESSION_DATA    IovSessionData,
    IN OUT  PIOV_REQUEST_PACKET  *IovPacketPointer,
    OUT     PBOOLEAN             SurrogateSpawned
    );

VOID
FASTCALL
IovpSessionDataReference(
    IN      PIOV_SESSION_DATA IovSessionData
    );

VOID
FASTCALL
IovpSessionDataDereference(
    IN      PIOV_SESSION_DATA IovSessionData
    );

VOID
FASTCALL
IovpSessionDataClose(
    IN      PIOV_SESSION_DATA IovSessionData
    );

VOID
IovpSessionDataDeterminePolicy(
    IN      PIOV_REQUEST_PACKET IovRequestPacket,
    IN      PDEVICE_OBJECT      DeviceObject,
    OUT     PBOOLEAN            Trackable,
    OUT     PBOOLEAN            UseSurrogateIrp
    );

BOOLEAN
FASTCALL
IovpSessionDataAttachSurrogate(
    IN OUT  PIOV_REQUEST_PACKET  *IovPacketPointer,
    IN      PIOV_SESSION_DATA    IovSessionData
    );

VOID
FASTCALL
IovpSessionDataFinalizeSurrogate(
    IN      PIOV_SESSION_DATA    IovSessionData,
    IN OUT  PIOV_REQUEST_PACKET  IovPacket,
    IN      PIRP                 Irp
    );

VOID
FASTCALL
IovpSessionDataBufferIO(
    IN OUT  PIOV_REQUEST_PACKET  IovSurrogatePacket,
    IN      PIRP                 SurrogateIrp
    );

VOID
FASTCALL
IovpSessionDataUnbufferIO(
    IN OUT  PIOV_REQUEST_PACKET  IovSurrogatePacket,
    IN      PIRP                 SurrogateIrp
    );

#endif // _SESSIONIRP_H_

