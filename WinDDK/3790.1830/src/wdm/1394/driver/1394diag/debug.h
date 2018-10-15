/*++

Copyright (c) 1998 Microsoft Corporation

Module Name:

    debug.h

Abstract:

Author:

    Peter Binder (pbinder) 3/16/98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
3/16/98  pbinder   starting...
4/13/98  pbinder   taken from another project...
--*/

#if DBG

#define _DRIVERNAME_    "1394DIAG"

#define TL_TRACE        0
#define TL_WARNING      1
#define TL_ERROR        2
#define TL_FATAL        3

extern unsigned char t1394DiagDebugLevel;

#define TRACE( l, x )                       \
    if( (l) >= t1394DiagDebugLevel ) {      \
        KdPrint( (_DRIVERNAME_ ": ") );     \
        KdPrint( x );                       \
    }

#define ENTER(n)        TRACE(TL_TRACE, ("%s Enter\n", n))

#define EXIT(n,x)                                   \
    if(NT_SUCCESS(x)) {                             \
        TRACE(TL_TRACE, ("%s Exit = %x\n", n, x));  \
    }                                               \
    else {                                          \
        TRACE(TL_ERROR, ("%s Exit = %x\n", n, x));  \
    }

#else  // DBG

#define TRACE( l, x )

#define ENTER(n)
#define EXIT(n,x)

#endif // DBG


