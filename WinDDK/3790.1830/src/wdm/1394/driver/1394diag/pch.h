/*++

Copyright (c) 1998  Microsoft Corporation

Module Name: 

    pch.h

Abstract


Author:

    Kashif Hasan (khasan) 4/30/00

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
4/13/98  pbinder   birth
4/30/00  khasan    add section for common code
--*/

#include <wdm.h>
#include <1394.h>
#include "common.h"                                
#include "1394diag.h"
#include "debug.h"

#ifdef _1394DIAG_C
#include "util.c"
#include "1394api.c"
#include "asyncapi.c"
#include "isochapi.c"
#endif

