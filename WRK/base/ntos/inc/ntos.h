/*++ BUILD Version: 0006    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ntos.h

Abstract:

    Top level include file for the NTOS component.

--*/

#ifndef _NTOS_
#define _NTOS_

#include <nt.h>
#include <ntrtl.h>
#include "ntosdef.h"
#include "exlevels.h"
#include "exboosts.h"
#include "bugcodes.h"
#include "init.h"
#include "v86emul.h"
#include "procpowr.h"

#if defined(_AMD64_)
#include "amd64.h"

#elif defined(_X86_)
#include "i386.h"

#else
#error "no target defined"
#endif // AMD64

#include "intrlk.h"
#include "arc.h"
#include "ke.h"
#include "kd.h"
#include "ex.h"
#include "ps.h"
#include "se.h"
#include "io.h"
#include "ob.h"
#include "mm.h"
#include "lpc.h"
#include "dbgk.h"
#include "lfs.h"
#include "cache.h"
#include "pnp.h"
#include "hal.h"
#include "kx.h"
#include "cm.h"
#include "po.h"
#include "perf.h"
#include "wmi.h"
#include "verifier.h"
#define _NTDDK_

//
// Temp. Until we define a header file for types
// Outside of the kernel these are exported by reference
//

#ifdef _NTDRIVER_
extern POBJECT_TYPE *ExEventPairObjectType;
extern POBJECT_TYPE *PsProcessType;
extern POBJECT_TYPE *PsThreadType;
extern POBJECT_TYPE *PsJobType;
extern POBJECT_TYPE *LpcPortObjectType;
extern POBJECT_TYPE *LpcWaitablePortObjectType;
#else
extern POBJECT_TYPE ExEventPairObjectType;
extern POBJECT_TYPE PsProcessType;
extern POBJECT_TYPE PsThreadType;
extern POBJECT_TYPE PsJobType;
extern POBJECT_TYPE LpcPortObjectType;
extern POBJECT_TYPE LpcWaitablePortObjectType;
#endif // _NTDRIVER

#endif // _NTOS_
