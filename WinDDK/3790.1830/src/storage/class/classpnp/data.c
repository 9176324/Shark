/*++

Copyright (C) Microsoft Corporation, 1991 - 1999

Module Name:

    disk.c

Abstract:

    SCSI disk class driver

Environment:

    kernel mode only

Notes:

Revision History:

--*/

#include "classp.h"


/*
 *  Entry in static list used by debug extension to quickly find all class FDOs.
 */
LIST_ENTRY AllFdosList = {&AllFdosList, &AllFdosList};

#ifdef ALLOC_DATA_PRAGMA
    #pragma data_seg("PAGE")
#endif

/*
#define FDO_HACK_CANNOT_LOCK_MEDIA (0x00000001)
#define FDO_HACK_GESN_IS_BAD       (0x00000002)
*/

CLASSPNP_SCAN_FOR_SPECIAL_INFO ClassBadItems[] = {
    { ""        , "MITSUMI CD-ROM FX240"           , NULL  ,   0x02 },
    { ""        , "MITSUMI CD-ROM FX320"           , NULL  ,   0x02 },
    { ""        , "MITSUMI CD-ROM FX322"           , NULL  ,   0x02 },
    { ""        , "TEAC DV-28E-A"                  , "2.0A",   0x02 },
    { ""        , "HP CD-Writer cd16h"             , "Q000",   0x02 },
    { ""        , "_NEC NR-7800A"                  , "1.33",   0x02 },
    { ""        , "COMPAQ CRD-8481B"               , NULL  ,   0x04 },
    { NULL      , NULL                             , NULL  ,   0x0  }
};


GUID ClassGuidQueryRegInfoEx = GUID_CLASSPNP_QUERY_REGINFOEX;

#ifdef ALLOC_DATA_PRAGMA
    #pragma data_seg()
#endif

