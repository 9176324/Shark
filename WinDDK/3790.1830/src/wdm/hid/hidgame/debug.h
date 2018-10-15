/*++

Copyright (c) 1998 - 1999  Microsoft Corporation

Module Name:

    debug.h

Abstract:  Contains definitions and macros to aid debugging
           data types for the joystick driver.


Environment:

    Kernel mode


--*/
/*
 *  Debugging macros
 */
    #undef  C_ASSERT
    #define C_ASSERT(e) switch(0) case(e): case(0):

    #define FILE_HIDGAME                0x00010000
    #define FILE_PNP                    0x00020000
    #define FILE_POLL                   0x00040000
    #define FILE_IOCTL                  0x00080000
    #define FILE_HIDJOY                 0x00100000
    #define FILE_TIMING                 0x00200000

    #define HGM_ERROR                 0x00000001
    #define HGM_WARN                  0x00000002
    #define HGM_BABBLE                0x00000004
    #define HGM_BABBLE2               0x00000008
    #define HGM_FENTRY                0x00000010
    #define HGM_FEXIT                 0x00000020
    #define HGM_GEN_REPORT            0x00008000
/*
 *  Squak if the return status is not SUCCEESS
 */
    #define HGM_FEXIT_STATUSOK        0x00001000

    #define HGM_DEFAULT_DEBUGLEVEL    0x0000001


    /* WDM.H defines DBG.  Make sure DBG is both defined and non-zero  */
    #ifdef DBG
        #if DBG
            #define TRAP()  DbgBreakPoint()
        #endif
    #endif

    #ifdef TRAP
        extern ULONG debugLevel;
        #define HGM_DBGPRINT( _debugMask_,  _x_) \
            if( (((_debugMask_) & debugLevel)) ){ \
                DbgPrint("HIDGAME.SYS: ");\
                DbgPrint _x_  ; \
                DbgPrint("\n"); \
            }
        #define HGM_EXITPROC(_debugMask_, _x_, ntStatus) \
            if( ((_debugMask_)&HGM_FEXIT_STATUSOK) && !NT_SUCCESS(ntStatus) ) {\
                HGM_DBGPRINT( (_debugMask_|HGM_ERROR), (_x_ "  ntStatus(0x%x)", ntStatus) ); }\
            else { HGM_DBGPRINT((_debugMask_), (_x_ " ntStatus(0x%x)", ntStatus));}

    #else

        #define HGM_DBGPRINT(_x_,_y_)
        #define HGM_EXITPROC(_x_,_y_,_z_)
        #define TRAP()
    #endif


