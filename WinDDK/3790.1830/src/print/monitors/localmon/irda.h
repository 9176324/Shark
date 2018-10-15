/*++

Copyright (c) 1997-2003  Microsoft Corporation
All rights reserved.

Module Name:

    irda.h

Abstract:

    Definitions used for IRDA printing

--*/

VOID
CheckAndAddIrdaPort(
    PINILOCALMON    pIniLocalMon
    );

VOID
CheckAndDeleteIrdaPort(
    PINILOCALMON    pIniLocalMon
    );

BOOL
IrdaStartDocPort(
    IN OUT  PINIPORT    pIniPort
    );

BOOL
IrdaWritePort(
    IN  HANDLE  hPort,
    IN  LPBYTE  pBuf,
    IN  DWORD   cbBuf,
    IN  LPDWORD pcbWritten
    );

VOID
IrdaEndDocPort(
    PINIPORT    pIniPort
    );

