/*++

Copyright (c) 1991, 1992  Microsoft Corporation

Module Name:

    krnldrvr.h

Abstract:

    Definitions for the krnldrvr sample device driver

--*/

#if DBG
#define KDVRDIAG1              ((ULONG)0x00000001)
#define KDVRDIAG2              ((ULONG)0x00000002)
#define KDVRDIAG3              ((ULONG)0x00000004)
#define KDVRDIAG4              ((ULONG)0x00000008)
#define KDVRDIAG5              ((ULONG)0x00000010)
#define KDVRFLOW               ((ULONG)0x20000000)
#define KDVRERRORS             ((ULONG)0x40000000)
#define KDVRBUGCHECK           ((ULONG)0x80000000)


// Change the initial value of this variable to produce debugging
// messages.

ULONG KrnldrvrDebugLevel = 0;


#define KrnldrvrDump(LEVEL,STRING) \
   do { \
     if (KrnldrvrDebugLevel & LEVEL) { \
         DbgPrint STRING; \
         } \
     if (LEVEL == KDVRBUGCHECK) { \
         ASSERT(FALSE); \
         } \
     } while (0)
#else
#define KrnldrvrDump(LEVEL,STRING) do {NOTHING;} while (0)
#endif


// PLEASE NOTE:
// The following driver device number and IOCTL code has been chosen
// at random, and is not intended to be used as is in any real product.
// Refer to the NT DDK Kernel mode Driver Design Guide for more information
// about IOCTL code formats. Contact Microsoft Product Support if you have
// questions about assigning your own IOCTL codes.

#define FILE_DEVICE_KRNLDRVR 0x80ff

#define IOCTL_KRNLDRVR_GET_INFORMATION  CTL_CODE(FILE_DEVICE_KRNLDRVR, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)


