/*****************************************************************************
 *
 *  EffPub.h
 *
 *  Abstract:
 *
 *      Header file that you would ship with your hardware SDK.
 *
 *****************************************************************************/

/*****************************************************************************
 *
 *      Our imaginary hardware supports the following escapes.
 *      The numbers listed below are private to our hardware.
 *
 *      I've chosen random numbers to emphasize that they
 *      can be any 32-bit value of your choosing.
 *
 *****************************************************************************/

#define ESCAPE_SETCOLOR         314
#define ESCAPE_GETEFFECTVALUE   461

/*
 *  For ESCAPE_SETCOLOR, lpvInBuffer must point to the following structure
 */
typedef struct ESCAPE_SETCOLORINFO {
    DWORD   dwColor;
} ESCAPE_SETCOLORINFO;

/*
 *  For ESCAPE_GETEFFECTVALUE, lpvOutBuffer must point to the
 *  following structure
 */
typedef struct ESCAPE_GETEFFECTVALUEINFO {
    DWORD   dwEffectValue;
} ESCAPE_GETEFFECTVALUEINFO;

/*****************************************************************************
 *
 *      Custom effects
 *
 *****************************************************************************/

/*
 *  GUID_Bisine - This effect is like GUID_Sine, except that you can specify
 *  different amplitudes above and below the baseline.
 */

DEFINE_GUID(GUID_Bisine,
            0x67EE5040,0xE1A1,0x11D0,0x9A,0xD0,0x00,0x00,0xC0,0x01,0x58,0x0E);

/*
 *  Type-specific data for the GUID_Bisine effect which is an extension
 *  of the standard DirectInput DIPERIODIC structure.
 */
typedef struct HWBISINEEFFECT {
    DIPERIODIC periodic;            /* Basic DirectInput structure */
    DWORD dwMagnitude2;             /* Magnitude below baseline */
} HWBISINEEFFECT, *LPHWBISINEEFFECT;
typedef const HWBISINEEFFECT *LPCHWBISINEEFFECT;
