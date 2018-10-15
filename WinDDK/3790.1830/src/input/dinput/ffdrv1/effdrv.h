/*****************************************************************************
 *
 *  EffDrv.h
 *
 *  Abstract:
 *
 *      Common header file for the template effect driver.
 *
 *****************************************************************************/

#define STRICT
#include <windows.h>
#include <objbase.h>
#include <dinput.h>
#include <mmsystem.h>
#include <dinputd.h>
#include "effpub.h"

/*****************************************************************************
 *
 *      Our imaginary hardware supports the following effects.  The
 *      numbers listed below are the effect codes used when
 *      communicating with the hardware.
 *
 *      For purposes of illustration, I've chosen random numbers to
 *      emphasize that they can be any 32-bit value of your choosing.
 *
 *      EFFECT_CONSTANT is a simple DICONSTANTFORCE effect.
 *
 *      EFFECT_BISINE is a hardware-specific effect which is like a
 *      standard DIPERIODIC sine wave, except that you can control
 *      the amplitude above and below the baseline independently.
 *
 *      EFFECT_SINE is a simple DIPERIODIC sine wave.  The hardware
 *      deals only in the EFFECT_BISINE, so we create a simple sine
 *      wave by creating an EFFECT_BISINE with the positive and
 *      negative amplitudes identical.
 *
 *      EFFECT_SPRING is a standard spring DICONDITION effect.
 *
 *****************************************************************************/

#define EFFECT_CONSTANT     371
#define EFFECT_BISINE       615
#define EFFECT_SINE         723
#define EFFECT_SPRING       912

/*****************************************************************************
 *
 *      Declare our structures as packed so the compiler won't pad them.
 *
 *      Note, the elements of these structures will be considered unaligned 
 *      so on processor architectures that require aligned memory accesses, 
 *      extra code will be produced to access them piecemeal.  This is highly 
 *      inefficient so if your real hardware interface does not need packed 
 *      structures you should removed this packing.
 *      Also, if your hardware interface uses structures that are not aligned 
 *      you must take care to appropriately decorate functions that accept 
 *      pointers to such unaligned elements.
 *
 *****************************************************************************/

#include <pshpack1.h>

/*****************************************************************************
 *
 *      Our imaginary hardware uses this structure for the RESET command.
 *
 *****************************************************************************/

typedef struct HWRESET {
    BYTE   bCommand;                /* HWCOMMAND_RESET                      */
} HWRESET, *PHWRESET;

/*****************************************************************************
 *
 *      We report information about our imaginary hardware's memory usage
 *      with the following private structure.
 *
 *****************************************************************************/

typedef struct HARDWAREINFO {
    WORD   wTotalMemory;
    WORD   wMemoryInUse;
} HARDWAREINFO;

/*****************************************************************************
 *
 *      Our imaginary hardware uses this structure for the SETLED command.
 *
 *****************************************************************************/

typedef struct HWSETLED {
    BYTE   bCommand;                /* HWCOMMAND_SETLED                     */
    BYTE   bColor;                  /* LED color                            */
} HWSETLED, *PHWSETLED;

#define LEDCOLOR_BLACK      0       /* LED off (therefore black)            */
#define LEDCOLOR_RED        1       /* LED red                              */
#define LEDCOLOR_GREEN      2       /* LED green                            */

/*****************************************************************************
 *
 *      Our imaginary hardware uses this structure for the STOPALL command.
 *
 *****************************************************************************/

typedef struct HWSTOPALL {
    BYTE   bCommand;                /* HWCOMMAND_STOPALL                    */
} HWSTOPALL, *PHWSTOPALL;

/*****************************************************************************
 *
 *      Our imaginary hardware uses this structure for the SETGAIN command.
 *
 *****************************************************************************/

typedef struct HWSETGAIN {
    BYTE   bCommand;                /* HWCOMMAND_SETGAIN                    */
    BYTE   bGain;                   /* New gain value (0 .. 255)            */
} HWSETGAIN, *PHWSETGAIN;

/*****************************************************************************
 *
 *      Our imaginary hardware uses this structure for the START command.
 *
 *      Our imaginary hardware is simple and doesn't support any special
 *      flags or iterations.
 *
 *****************************************************************************/

typedef struct HWSTART {
    BYTE   bCommand;                /* HWCOMMAND_START                      */
    BYTE   bEffect;                 /* Effect id being started              */
} HWSTART, *PHWSTART;

/*****************************************************************************
 *
 *      Our imaginary hardware uses this structure for the STOP command.
 *
 *      Our imaginary hardware is simple and doesn't support any special
 *      flags or iterations.
 *
 *****************************************************************************/

typedef struct HWSTOP {
    BYTE   bCommand;                /* HWCOMMAND_STOP                       */
    BYTE   bEffect;                 /* Effect id being stopped              */
} HWSTOP, *PHWSTOP;

/*****************************************************************************
 *
 *      Our imaginary hardware uses this structure for the
 *      GETEFFSTATUS command.
 *
 *****************************************************************************/

typedef struct HWGETEFFSTATUS {
    BYTE   bCommand;                /* HWCOMMAND_GETEFFSTATUS               */
    BYTE   bEffect;                 /* Effect id being queried              */
} HWGETEFFSTATUS, *PHWGETEFFSTATUS;

/*****************************************************************************
 *
 *      Our imaginary hardware reports information with the following
 *      private structure in response to the HWCOMMAND_GETEFFSTATUS command.
 *
 *****************************************************************************/

typedef struct EFFSTATUS {
    BYTE   bPlaying;                /* Is effect playing?                   */
} EFFSTATUS, *PEFFSTATUS;

/*****************************************************************************
 *
 *      Our imaginary hardware uses this structure for the
 *      GETEFFVALUE command.
 *
 *****************************************************************************/

typedef struct HWGETEFFVALUE {
    BYTE   bCommand;                /* HWCOMMAND_GETEFFVALUE                */
    BYTE   bEffect;                 /* Effect id being queried              */
} HWGETEFFVALUE, *PHWGETEFFVALUE;

/*****************************************************************************
 *
 *      Our imaginary hardware reports information with the following
 *      private structure in response to the HWCOMMAND_GETEFFVALUE command.
 *
 *****************************************************************************/

typedef struct EFFVALUE {
    BYTE    bValue;                 /* The instantaneous effect value       */
} EFFVALUE;

/*****************************************************************************
 *
 *      Our imaginary hardware uses this structure for envelope and
 *      trigger information.
 *
 *      Force levels are expressed in the range 0 .. 255.
 *
 *****************************************************************************/

#define HWINFINITE      0xFFFF      /* Hardware treats as "infinity"        */
#define HWMAXDURATION   0xFFFE      /* Highest non-infinite duration        */

typedef struct HWENVTRIG {
    WORD    wAttackTime;            /* Attack time in milliseconds          */
    WORD    wSustainTime;           /* Sustain time in milliseconds         */
    WORD    wFadeTime;              /* Fade time in milliseconds            */
    BYTE    bAttackLevel;           /* Signed initial level of envelope     */
    BYTE    bSustainLevel;          /* Signed sustain level of envelope     */
    BYTE    bFadeLevel;             /* Signed fade level of envelope        */
    BYTE    bTrigger;               /* 0 = none, 1 = button 0               */
} HWENVTRIG, *PHWENVTRIG;

/*****************************************************************************
 *
 *      Our imaginary hardware uses this structure for constant-force
 *      effects.
 *
 *      The semantics of the various levels are as follows:
 *
 *      env.bAttackLevel  - signed attack level at start of effect
 *      env.bSustainLevel - signed sustain level
 *      env.bFadeLevel    - signed fade level at end of effect
 *
 *      For convenience, we don't actually send negative values to
 *      our hardware.  This saves us from having to do lots of
 *      sign flips when dealing with envelopes applied to constant-force
 *      effects with negative force values.
 *
 *****************************************************************************/

typedef struct HWCONST {
    BYTE   bCommand;                /* HWCOMMAND_CREATECONST                */
    BYTE   bEffect;                 /* Effect id being updated              */
    HWENVTRIG env;                  /* Envelope/sustain information         */
    BYTE   bDirection;              /* 0 = north, 64 = east, etc.           */
} HWCONST, *PHWCONST;

/*****************************************************************************
 *
 *      Our imaginary hardware uses this structure for bisine effects.
 *
 *      The semantics of the various levels is tricky.
 *
 *      These three parameters control the magnitude when phase = 90 deg.
 *
 *      env.bAttackLevel  - signed upper attack level at start of effect
 *      env.bSustainLevel - signed upper sustain level
 *      env.bFadeLevel    - signed upper fade level at end of effect
 *
 *      These three parameters control the magnitude when phase = 270 deg.
 *
 *      bAttackLevel2     - signed lower attack level at start of effect
 *      bSustainLevel2    - signed lower sustain level
 *      bFadeLevel2       - signed lower fade level at end of effect
 *
 *      And finally, the magnitude when phase = 0 deg and 180 deg
 *
 *      bCenter           - signed center magnitude
 *
 *****************************************************************************/

typedef struct HWBISINE {
    BYTE   bCommand;                /* HWCOMMAND_CREATEBISINE               */
    BYTE   bEffect;                 /* Effect id being updated              */
    HWENVTRIG env;                  /* Envelope/sustain information         */
    WORD   wPeriod;                 /* Period in milliseconds               */
    BYTE   bCenter;                 /* Effect strength at center            */
    BYTE   bAttackLevel2;           /* Initial effect strength for bottom   */
    BYTE   bSustainLevel2;          /* Effect strength for bottom           */
    BYTE   bFadeLevel2;             /* Final effect strength for bottom     */
    BYTE   bPhase;                  /* 0=0deg, 1=90deg, 2=180deg, 3=270deg  */
    BYTE   bDirection;              /* 0 = north, 64 = east, etc.           */
} HWBISINE, *PHWBISINE;

/*****************************************************************************
 *
 *      Our imaginary hardware uses one of these for each axis involved
 *      in a spring effect.
 *
 *****************************************************************************/

typedef struct HWSPRINGAXIS {
    BYTE   bLow;                    /* Low end of dead zone                 */
    BYTE   bHigh;                   /* High end of dead zone                */
    BYTE   bCoeff;                  /* Spring coefficient                   */
    BYTE   bSaturation;             /* Saturation                           */
} HWSPRINGAXIS, *PHWSPRINGAXIS;

/*****************************************************************************
 *
 *      Our imaginary hardware uses this structure for spring effects.
 *
 *****************************************************************************/

typedef struct HWSPRING {
    BYTE   bCommand;                /* HWCOMMAND_CREATESPRING               */
    BYTE   bEffect;                 /* Effect id being updated              */
    HWSPRINGAXIS axisX;             /* Parameters for x-axis                */
    HWSPRINGAXIS axisY;             /* Parameters for y-axis                */
} HWSPRING;

/*****************************************************************************
 *
 *      Finished with our structures.  Restore original packing.
 *
 *****************************************************************************/

#include <poppack.h>

/*****************************************************************************
 *
 *      Imaginary commands to our imaginary hardware.
 *
 *****************************************************************************/

#define HWCOMMAND_RESET         0x01        /* Reset the device completely */
#define HWCOMMAND_SETLED        0x10        /* Followed by LED color */
#define HWCOMMAND_SETGAIN       0x11        /* Followed by gain 0 .. 255 */
#define HWCOMMAND_STOPALL       0x12        /* Stop all effects */
#define HWCOMMAND_START         0x13        /* Start an effect */
#define HWCOMMAND_STOP          0x14        /* Stop an effect */
#define HWCOMMAND_GETEFFSTATUS  0x15        /* Query effect status */
#define HWCOMMAND_GETEFFVALUE   0x16        /* Read force value directly */
#define HWCOMMAND_CREATECONST   0x20        /* Create a constant-force effect */
#define HWCOMMAND_CREATEBISINE  0x21        /* Create a bi-sine effect */
#define HWCOMMAND_CREATESPRING  0x22        /* Create a spring effect */

/*****************************************************************************
 *
 *      Imaginary helper functions that talk to our imaginary hardware.
 *
 *****************************************************************************/

HRESULT HWSetDeviceColor(DWORD dwUnit, DWORD dwColor);
HRESULT HWGetEffectValue(DWORD dwUnit, DWORD dwEffect,
                        ESCAPE_GETEFFECTVALUEINFO *pinfo);
HRESULT HWSetDeviceGain(DWORD dwUnit, DWORD dwGain);
HRESULT HWResetDevice(DWORD dwUnit);
HRESULT HWStopAllEffects(DWORD dwUnit);
HRESULT HWGetHardwareInfo(DWORD dwUnit, HARDWAREINFO *pinfo);
HRESULT HWDownloadConstantEffect(DWORD dwUnit, DWORD dwEffect,
                                 LPCDIEFFECT peff, DWORD dwFlags);
HRESULT HWDownloadBisineEffect(DWORD dwUnit, DWORD dwEffect,
                               LPCDIEFFECT peff, DWORD dwFlags, BOOL fBi);
HRESULT HWDownloadSineEffect(DWORD dwUnit, DWORD dwEffect,
                             LPCDIEFFECT peff, DWORD dwFlags);
HRESULT HWDownloadSpringEffect(DWORD dwUnit, DWORD dwEffect,
                               LPCDIEFFECT peff, DWORD dwFlags);
HRESULT HWDestroyEffect(DWORD dwUnit, DWORD dwEffect);
HRESULT HWStartEffect(DWORD dwUnit, DWORD dwEffect, DWORD dwMode,
                      DWORD dwCount);
HRESULT HWStopEffect(DWORD dwUnit, DWORD dwEffect);
HRESULT HWGetEffectStatus(DWORD dwUnit, DWORD dwEffect, LPDWORD pdwStatus);

/*****************************************************************************
 *
 *      For each active unit, one of these structures exists to keep
 *      track of which effects are "in use".
 *
 *      Our imaginary hardware does not do dynamic memory allocation;
 *      there are merely 16 "slots", and each "slot" can be "in use"
 *      or "free".
 *
 *****************************************************************************/

#define MAX_EFFECTS     16
#define MAX_UNITS        2

typedef struct UNITSTATE {
    BOOL    rgfBusy[MAX_EFFECTS];
} UNITSTATE, *PUNITSTATE;

/*****************************************************************************
 *
 *      Since the information to track each unit is so small, we pack them
 *      together into a single shared memory block to save memory.
 *
 *      We use our own GUID as the name of the memory block to avoid
 *      collisions with other named memory blocks.
 *
 *****************************************************************************/

typedef struct SHAREDMEMORY {
    UNITSTATE rgus[MAX_UNITS];
} SHAREDMEMORY, *PSHAREDMEMORY;

/*****************************************************************************
 *
 *      Helper functions that help us manager our imaginary hardware.
 *
 *****************************************************************************/

HRESULT HWAllocateEffectId(DWORD dwUnit, LPDWORD pdwEffect);
HRESULT HWFreeEffectId(DWORD dwUnit, DWORD dwEffect);

/*****************************************************************************
 *
 *      Static globals:  Initialized at PROCESS_ATTACH and never modified.
 *
 *****************************************************************************/

extern HINSTANCE g_hinst;       /* This DLL's instance handle */
extern PSHAREDMEMORY g_pshmem;  /* Our shared memory block */
extern HANDLE g_hfm;            /* Handle to file mapping object */
extern HANDLE g_hmtxShared;     /* Handle to mutex that protects g_pshmem */

/*****************************************************************************
 *
 *      Dll global functions
 *
 *****************************************************************************/

STDAPI_(ULONG) DllAddRef(void);
STDAPI_(ULONG) DllRelease(void);
STDAPI_(void) DllEnterCritical(void);
STDAPI_(void) DllLeaveCritical(void);

/*****************************************************************************
 *
 *      Class factory
 *
 *****************************************************************************/

STDAPI CClassFactory_New(REFIID riid, LPVOID *ppvObj);

/*****************************************************************************
 *
 *      Effect driver
 *
 *****************************************************************************/

STDAPI CEffDrv_New(REFIID riid, LPVOID *ppvObj);
