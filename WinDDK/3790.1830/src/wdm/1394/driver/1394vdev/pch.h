/*++

Copyright (c) 1998  Microsoft Corporation

Module Name: 

    pch.h

Abstract


Author:

    Peter Binder (pbinder) 4/13/98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
4/13/98  pbinder   birth
--*/

#include <wdm.h>

#include <1394.h>
#include "common.h"
#include "1394vdev.h"
#include "debug.h"


#ifdef _1394VDEV_C
#include "util.c"
#include "1394api.c"
#include "asyncapi.c"
#include "isochapi.c"
#endif

