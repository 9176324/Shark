/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.
    
Module Name:

    TRACE.h

Abstract:

    Header file for the debug tracing related function defintions and macros.
    HEXDUMP is not defined by default. So we have to provide a localwpp.ini file 
    that defines type. 

Environment:

    Kernel mode

Revision History:

    Eliyas Yakub Apr 30, 2003

--*/


//
// Define debug levels
//
#define     NONE      0 //  Tracing is not on
#define     FATAL      1 // Abnormal exit or termination
#define     ERROR      2 // Severe errors that need logging
#define     WARNING    3  // Warnings such as allocation failure
#define     INFO       4  // Includes non-error cases such as Entry-Exit
#define     TRACE      5 // Detailed traces from intermediate steps
#define     LOUD       6  // Detailed trace from every step


#if !defined(EVENT_TRACING)

//
// Define Debug Flags
//
#define DBG_INIT             0x00000001
#define DBG_PNP    0x00000002
#define DBG_POWER              0x00000004
#define DBG_WMI            0x00000008
#define DBG_CREATE_CLOSE              0x00000010
#define DBG_IOCTLS             0x00000020
#define DBG_WRITE            0x00000040
#define DBG_READ        0x00000080
#define DBG_DPC              0x00000100
#define DBG_INTERRUPT           0x00000200
#define DBG_LOCKS            0x00000400
#define DBG_QUEUEING         0x00000800
#define DBG_HW_ACCESS        0x00001000

VOID
DebugPrint    (
    IN ULONG   DebugPrintLevel,
    IN ULONG   DebugPrintFlag,
    IN PCCHAR  DebugMessage,
    ...
    );

#define DebugPrintEx(x) // Used for HEXDUMP in case tracing is enabled

#else 
//    
// If software tracing is defined in the sources file..
// WPP_DEFINE_CONTROL_GUID specifies the GUID used for this driver.
// *** REPLACE THE GUID WITH YOUR OWN UNIQUE ID ***
// WPP_DEFINE_BIT allows setting debug bit masks to selectively print.   
// The names defined in the WPP_DEFINE_BIT call define the actual names 
// that are used to control the level of tracing for the control guid 
// specified. 
//
// Name of the logger is PciDrv and the guid is
//   {BC6C9364-FC67-42c5-ACF7-ABED3B12ECC6}
//   (0xbc6c9364, 0xfc67, 0x42c5, 0xac, 0xf7, 0xab, 0xed, 0x3b, 0x12, 0xec, 0xc6);
//

#define WPP_CHECK_FOR_NULL_STRING  //to prevent exceptions due to NULL strings

#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(PCIDRV,(bc6c9364,fc67,42c5,acf7,abed3b12ecc6), \
        WPP_DEFINE_BIT(DBG_INIT)             /* bit  0 = 0x00000001 */ \
        WPP_DEFINE_BIT(DBG_PNP)              /* bit  1 = 0x00000002 */ \
        WPP_DEFINE_BIT(DBG_POWER)            /* bit  2 = 0x00000004 */ \
        WPP_DEFINE_BIT(DBG_WMI)              /* bit  3 = 0x00000008 */ \
        WPP_DEFINE_BIT(DBG_CREATE_CLOSE)     /* bit  4 = 0x00000010 */ \
        WPP_DEFINE_BIT(DBG_IOCTLS)           /* bit  5 = 0x00000020 */ \
        WPP_DEFINE_BIT(DBG_WRITE)            /* bit  6 = 0x00000040 */ \
        WPP_DEFINE_BIT(DBG_READ)             /* bit  7 = 0x00000080 */ \
        WPP_DEFINE_BIT(DBG_DPC)              /* bit  8 = 0x00000100 */ \
        WPP_DEFINE_BIT(DBG_INTERRUPT)        /* bit  9 = 0x00000200 */ \
        WPP_DEFINE_BIT(DBG_LOCKS)            /* bit 10 = 0x00000400 */ \
        WPP_DEFINE_BIT(DBG_QUEUEING)         /* bit 11 = 0x00000800 */ \
        WPP_DEFINE_BIT(DBG_HW_ACCESS)        /* bit 12 = 0x00001000 */ \
        WPP_DEFINE_BIT(DebugFlag13)          /* bit 13 = 0x00002000 */ \
        WPP_DEFINE_BIT(DebugFlag14)          /* bit 14 = 0x00004000 */ \
        WPP_DEFINE_BIT(DebugFlag15)          /* bit 15 = 0x00008000 */ \
        WPP_DEFINE_BIT(DebugFlag16)          /* bit 16 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag17)          /* bit 17 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag18)          /* bit 18 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag19)          /* bit 19 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag20)          /* bit 20 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag21)          /* bit 21 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag22)          /* bit 22 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag23)          /* bit 23 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag24)          /* bit 24 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag25)          /* bit 25 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag26)          /* bit 26 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag27)          /* bit 27 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag28)          /* bit 28 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag29)          /* bit 29 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag30)          /* bit 30 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag31)          /* bit 31 = 0x00000000 */ \
        )


#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) WPP_LEVEL_LOGGER(flags)
#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level  >= lvl)
//
// Define the 'xstr' structure for logging buffer and length pairs
// and the 'log_xstr' function which returns it to create one in-place.
// this enables logging of complex data types.
//
typedef struct xstr { char * _buf; short  _len; } xstr_t;
__inline xstr_t log_xstr(void * p, short l) { xstr_t xs = {(char*)p,l}; return xs; }

//
// Define the macro required for a hexdump use as:
//
//   DebugTraceEx((LEVEL, FLAG,"%!HEXDUMP!\n", log_xstr(buffersize,(char *)buffer) ));
//
//
#define WPP_LOGHEXDUMP(x) WPP_LOGPAIR(2, &((x)._len)) WPP_LOGPAIR((x)._len, (x)._buf)

//
// This enables kd printing on top of WMI logging
//
//#if defined(DBG)
//#define WPP_DEBUG(b)  DbgPrint("[%s] ","PCIDRV"), DbgPrint b
//#endif

#endif

