/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    eballoc.c

Abstract:

    Process/Thread Environment Block allocation functions

--*/

#include "ntrtlp.h"
#include <nturtl.h>

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(INIT,RtlAcquirePebLock)
#pragma alloc_text(INIT,RtlReleasePebLock)
#endif


#undef RtlAcquirePebLock

VOID
RtlAcquirePebLock( VOID )
{
}


#undef RtlReleasePebLock

VOID
RtlReleasePebLock( VOID )
{
}

#if DBG
VOID
RtlAssertPebLockOwned( VOID )
{
}
#endif

