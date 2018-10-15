/*++

Copyright (c) 1990-2003  Microsoft Corporation
All rights reserved

Module Name:

    precomp.h

--*/

                      
#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <winsock2.h>
#include <wchar.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <strsafe.h>

#include "resource.h"
#include "spltypes.h"
#include "localmon.h"


#undef DBGMSG
#define DBGMSG(x,y)
#undef SPLASSERT
#define SPLASSERT(x) 

