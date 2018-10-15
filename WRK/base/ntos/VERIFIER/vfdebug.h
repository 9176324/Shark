/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    vfdebug.h

Abstract:

    This header contains debugging macros used by the driver verifier code.

--*/

extern ULONG VfSpewLevel;

#if DBG
#define VERIFIER_DBGPRINT(txt,level) \
{ \
    if (VfSpewLevel>(level)) { \
        DbgPrint##txt; \
    }\
}
#else
#define VERIFIER_DBGPRINT(txt,level)
#endif

