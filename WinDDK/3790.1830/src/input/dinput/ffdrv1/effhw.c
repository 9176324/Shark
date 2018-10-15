/*****************************************************************************
 *
 *  EffHw.c
 *
 *  Abstract:
 *
 *      Fake hardware module.
 *
 *      Although the details of this file are completely irrelevant
 *      since your hardware probably is nothing like our imaginary
 *      hardware, this file does illustrate some points of detail
 *      as well as points of philosophy.
 *
 *      One over-arching philosophical point is "be generous in what
 *      you permit".  If a parameter is out of range, truncate it
 *      into range.  Applications will typically not be too particular
 *      in what they ask for.  For example, they may pass gains and
 *      magnitudes that are beyond DI_FFNOMINALMAX because the
 *      physics engine they are using happens to generate those values.
 *
 *****************************************************************************/

#include "effdrv.h"

/*****************************************************************************
 *
 *      HWSendCommand
 *
 *      Send some bytes to the channel for the unit.
 *
 *  dwUnit
 *
 *      Hardware unit number; this tells us which port to use.
 *
 *  cbCommand
 *
 *      Number of bytes to send.
 *
 *  pCommand
 *
 *      The bytes themselves.
 *
 *****************************************************************************/

void
HWSendCommand(DWORD dwUnit, int cbCommand, LPVOID pCommand)
{
    USHORT port;
    int i;
    LPBYTE rgbCommand = pCommand;

    switch (dwUnit) {
    case 0:                 /* Unit 0 is on port 4123 */
        port = 4123; break;
    case 1:                 /* Unit 1 is on port 9312 */
        port = 9312; break;
    default:
        return;             /* Other units are bogus */
    }

    /*
     *  Since we don't have any real hardware, we don't actually
     *  do anything aside from squirt to the debugger...
     */

    OutputDebugString(TEXT("EffHw: "));
    for (i = 0; i < cbCommand; i++) {
        /* Send byte rgbCommand[i] out the port. */
        TCHAR tsz[32];
        wsprintf(tsz, TEXT("%02x "), rgbCommand[i]);
        OutputDebugString(tsz);
    }
    OutputDebugString(TEXT("\r\n"));
}

/*****************************************************************************
 *
 *      HWReceiveData
 *
 *      Receive some bytes from the channel for the unit.
 *
 *  dwUnit
 *
 *      Hardware unit number; this tells us which port to use.
 *
 *  cbData
 *
 *      Number of bytes to read.
 *
 *  pData
 *
 *      Where to put the result.
 *
 *****************************************************************************/

void
HWReceiveData(DWORD dwUnit, int cbData, LPVOID pData)
{
    USHORT port;
    int i;
    LPBYTE rgbData = pData;

    switch (dwUnit) {
    case 0:                 /* Unit 0 is on port 4123 */
        port = 4123; break;
    case 1:                 /* Unit 1 is on port 9312 */
        port = 9312; break;
    default:
        return;             /* Other units are bogus */
    }

    /*
     *  Since we don't have any real hardware, we don't actually
     *  do anything...
     */

    /*for (i = 0; i < cbData; i++) {*/
        /* Read byte rgbData[i] from the port. */
    /*}*/
}

/*****************************************************************************
 *
 *      HWAllocateEffectId
 *
 *      Allocate a new effect id on the device if there is a spare one.
 *
 *      Since zero is an invalid effect id, we artificially add 1 to
 *      our internal index, so that we never return 0 by mistake.
 *
 *  dwUnit
 *
 *      Hardware unit number; this tells us which port to use.
 *
 *  pdwEffect
 *
 *      Receives the allocated effect id.
 *
 *  Returns:
 *
 *      S_OK if everything is okay.
 *
 *      DIERR_DEVICEFULL if there is no room left.
 *
 *****************************************************************************/

HRESULT
HWAllocateEffectId(DWORD dwUnit, LPDWORD pdwEffect)
{
    DWORD dwEffect;

    *pdwEffect = 0;

    WaitForSingleObject(g_hmtxShared, INFINITE);
    for (dwEffect = 0; dwEffect < MAX_EFFECTS; dwEffect++) {
        if (!g_pshmem->rgus[dwUnit].rgfBusy[dwEffect]) {
            g_pshmem->rgus[dwUnit].rgfBusy[dwEffect] = TRUE;
            *pdwEffect = 1 + dwEffect;
            break;
        }
    }
    ReleaseMutex(g_hmtxShared);

    return (*pdwEffect == 0) ? DIERR_DEVICEFULL : S_OK;
}

/*****************************************************************************
 *
 *      HWFreeEffectId
 *
 *      Release an effect id, making it available for future allocation.
 *
 *      Since zero is an invalid effect id, we artificially added 1 to
 *      our internal index.  Subtract off that 1 to get the internal
 *      index back.
 *
 *  dwUnit
 *
 *      Hardware unit number; this tells us which port to use.
 *
 *  dwEffect
 *
 *      The effect to be freed.
 *
 *  Returns:
 *
 *      S_OK if everything is okay.
 *
 *      E_HANDLE if the handle is not allocated.
 *
 *****************************************************************************/

HRESULT
HWFreeEffectId(DWORD dwUnit, DWORD dwEffect)
{
    HRESULT hres;

    WaitForSingleObject(g_hmtxShared, INFINITE);
    if (g_pshmem->rgus[dwUnit].rgfBusy[dwEffect - 1]) {
        g_pshmem->rgus[dwUnit].rgfBusy[dwEffect - 1] = FALSE;
        hres = S_OK;
    } else {
        hres = E_HANDLE;
    }
    ReleaseMutex(g_hmtxShared);

    return hres;
}

/*****************************************************************************
 *
 *      HWSetDeviceColor
 *
 *      Change the color of the LED on our imaginary joystick.
 *
 *  dwUnit
 *
 *      Hardware unit number; this tells us which port to use.
 *
 *  dwColor
 *
 *      Color to use.  The application passes an RGB, but we support
 *      only red, green, and black (off).  Black is black.
 *      Anything that is a shade of red we call red.
 *      Anything that is a shade of green we call green.
 *      Anything else is an error.
 *
 *****************************************************************************/

HRESULT
HWSetDeviceColor(DWORD dwUnit, DWORD dwColor)
{
    HWSETLED led;

    led.bCommand = HWCOMMAND_SETLED;

    if (dwColor == RGB(0, 0, 0)) {
        led.bColor = LEDCOLOR_BLACK;
    } else if ((dwColor & ~RGB(0xFF, 0, 0)) == 0) {
        led.bColor = LEDCOLOR_RED;
    } else if ((dwColor & ~RGB(0, 0xFF, 0)) == 0) {
        led.bColor = LEDCOLOR_GREEN;
    } else {
        return E_INVALIDARG;            /* Unsupported LED color */
    }

    HWSendCommand(dwUnit, sizeof(led), &led);
    return S_OK;
}


/*****************************************************************************
 *
 *      HWSetUnsignedValue
 *
 *      Convert a DirectInput scaled value (0 .. DI_FFNOMINALMAX) to
 *      a byte in the range 0 .. 255 because that's what our imaginary
 *      hardware wants.
 *
 *  dwValue
 *
 *      DirectInput scaled value.  Note that this value can be out of range,
 *      in which case DI_TRUNCATED is returned.
 *
 *  pbResult
 *
 *      The converted scaled value goes here.
 *
 *  phresTruncated
 *
 *      Points to an HRESULT which will be set to DI_TRUNCATED if
 *      any of the parameters were truncated.
 *
 *****************************************************************************/

void
HWSetUnsignedValue(DWORD dwValue, LPBYTE pbResult, HRESULT *phresTruncated)
{
    DWORD dwAdjustedValue;

    /*
     *  Convert from 0..DI_FFNOMINALMAX to 0..255.
     */
    dwAdjustedValue = MulDiv(dwValue, 255, DI_FFNOMINALMAX);
    if (dwAdjustedValue > 255) {
        dwAdjustedValue = 255;
    }
    *pbResult = (BYTE)dwAdjustedValue;

    /*
     *  If we had to truncate the value, then return DI_TRUNCATED.
     */
    if (dwValue > DI_FFNOMINALMAX) {
        *phresTruncated = DI_TRUNCATED;
    }
}

/*****************************************************************************
 *
 *      HWSetSignedValue
 *
 *      Convert a DirectInput scaled signed value
 *      (-DI_FFNOMINALMAX .. +DI_FFNOMINALMAX) to
 *      a byte in the range -127 to +127
 *      because that's what our imaginary hardware wants.
 *
 *  lValue
 *
 *      DirectInput scaled value.  Note that this value can be out of range,
 *      in which case DI_TRUNCATED is returned.
 *
 *  pbResult
 *
 *      The converted scaled value goes here.
 *
 *  phresTruncated
 *
 *      Points to an HRESULT which will be set to DI_TRUNCATED if
 *      any of the parameters were truncated.
 *
 *****************************************************************************/

void
HWSetSignedValue(LONG lValue, LPBYTE pbResult, HRESULT *phresTruncated)
{
    if (lValue < -DI_FFNOMINALMAX) {
        *pbResult = (BYTE)-127;
        *phresTruncated = DI_TRUNCATED;
    } else if (lValue < DI_FFNOMINALMAX) {
        *pbResult = (BYTE)MulDiv(lValue, 127, DI_FFNOMINALMAX);
    } else {
        *pbResult = +127;
        *phresTruncated = DI_TRUNCATED;
    }
}

/*****************************************************************************
 *
 *      HWSetMagnitude
 *
 *      Take a DirectInput gain and magnitude, both in the range
 *      (0 .. DI_FFNOMINALMAX), and combine them into a byte
 *      force value in the range (-127 .. +127).
 *
 *      Our imaginary hardware does not support separate gain and
 *      magnitude, so we must manually combine the gain and
 *      magnitude into a combined magnitude.
 *
 *  dwGain
 *
 *          DirectInput gain value (0 .. DI_FFNOMINALMAX).
 *
 *  lMagnitude
 *
 *          DirectInput magnitude value (0 .. DI_FFNOMINALMAX).
 *
 *  pbValue
 *
 *          Combined scaled value goes here.
 *
 *  phresTruncated
 *
 *      Points to an HRESULT which will be set to DI_TRUNCATED if
 *      any of the parameters were truncated.
 *
 *****************************************************************************/

void
HWSetMagnitude(DWORD dwGain, LONG lMagnitude, PBYTE pbValue,
               HRESULT *phresTruncated)
{
    HWSetSignedValue(MulDiv(dwGain, lMagnitude, DI_FFNOMINALMAX),
                     pbValue, phresTruncated);
}

/*****************************************************************************
 *
 *      HWSetDirection
 *
 *      Take a DirectInput direction parameter and convert it to a
 *      direction that our imaginary hardware supports.
 *
 *      If there is only one axis, then the direction is easy.
 *
 *      X-axis means east.
 *
 *      Y-axis means south.
 *
 *      If there are two axes, then there are two cases.
 *
 *      X-then-y:  We told DirectInput that we want directions
 *      in polar form, so the translation is easy.
 *      We merely scale (0 ... 360 * DI_DEGREES) to (0 .. 256).
 *
 *      Y-then-x:  This is just like x-then-y, but with flipped
 *      axes.
 *
 *  peff
 *
 *          DIEFFECT structure containing direction parameters.
 *
 *  pbDirection
 *
 *          Receives the computed direction.
 *
 *****************************************************************************/

void
HWSetDirection(LPCDIEFFECT peff, LPBYTE pbDirection)
{

    if (peff->cAxes == 1) {
        /*
         *  We've already validated that the axes are valid,
         *  so just run with them.
         */
        if (DIDFT_GETINSTANCE(peff->rgdwAxes[0]) == 0) {    /* X-axis */
            *pbDirection = 64;              /* East */
        } else {                                            /* Y-axis */
            *pbDirection = 128;             /* South */
        }
    } else {                            /* Multi-axis */
        LONG lDir;
        if (DIDFT_GETINSTANCE(peff->rgdwAxes[0]) == 0) {    /* X then Y */
            lDir = peff->rglDirection[0];
        } else {                                            /* Y then X */
            lDir = 270 * DI_DEGREES - peff->rglDirection[0];
            if (lDir < 0) {
                lDir += 360 * DI_DEGREES;           /* Keep lDir >= 0 */
            }
        }

        /*
         *  Now scale the value into the range 0 .. 256, which is
         *  what our hardware accepts.
         */
        *pbDirection = (BYTE)MulDiv(lDir, 256, 360 * DI_DEGREES);
    }

}

/*****************************************************************************
 *
 *      HWSetDuration
 *
 *      Take a DirectInput duration parameter and convert it to a
 *      duration that our imaginary hardware supports.
 *
 *      DirectInput durations are in microseconds, whereas our
 *      hardware wants milliseconds.
 *
 *      DirectInput uses INFINITE to mean infinite duration,
 *      whereas our hardware uses HWINFINITE.  Also, our hardware
 *      maxes out at HWMAXDURATION milliseconds,
 *      whereas DirectInput maxes out at (INFINITE-1) microseconds.
 *
 *  dwDuration
 *
 *          DirectInput duration.
 *
 *  pwDuration
 *
 *          Receives the computed duration.
 *
 *  phresTruncated
 *
 *      Points to an HRESULT which will be set to DI_TRUNCATED if
 *      any of the parameters were truncated.
 *
 *****************************************************************************/

void
HWSetDuration(DWORD dwDuration, UNALIGNED WORD * pwDuration, HRESULT *phresTruncated)
{
    if (dwDuration == INFINITE) {
        *pwDuration = HWINFINITE;
    } else {
        DWORD dwMs;
        dwMs = dwDuration / 1000;       /* Convert microseconds to ms */
        if (dwMs > HWMAXDURATION) {     /* More than we can handle */
            dwMs = HWMAXDURATION;       /* So truncate to our max */
            *phresTruncated = DI_TRUNCATED;
        }
        *pwDuration = (WORD)dwMs;
    }
}

/*****************************************************************************
 *
 *      HWSetTrigger
 *
 *      Take a DirectInput DIEFFECT structure and set the trigger
 *      parameters in the HWENVTRIG structure in a form that our
 *      imaginary hardware supports.
 *
 *  peff
 *
 *      DirectInput DIEFFECT structure.
 *
 *  phwenv
 *
 *      HWENVTRIG structure to fill in.
 *
 *  Returns:
 *
 *      None.
 *
 *****************************************************************************/

void
HWSetTrigger(LPCDIEFFECT peff, PHWENVTRIG phwenv)
{
    /*
     *  If we don't have a trigger, then use 0.
     *  If the trigger is button 1, then use 1.
     *  No other button is supported as a trigger.
     *
     *  CEffDrv_DownloadEffect has already validated
     *  the trigger and repeat interval.
     */
    if (peff->dwTriggerButton == DIEB_NOTRIGGER) {
        phwenv->bTrigger = 0;
    } else {
        phwenv->bTrigger = 1;
    }
}

/*****************************************************************************
 *
 *      HWSanitizeLevel
 *
 *      Take a DWORD magnitude and truncate it into the range
 *      0 ... DI_FFNOMINALMAX.
 *
 *      Apps might mess up their math and accidentally pass
 *      a negative number (cast to DWORD) as the dwAttackLevel
 *      or dwFadeLevel, so if we see a negative number, peg
 *      it to zero to avoid surprises and weird overflow scenarios.
 *
 *  dwLevel
 *
 *      Nominal magnitude for effect, possibly out of range.
 *
 *  pdwLevelOut
 *
 *      The result goes here.
 *
 *  phresTruncated
 *
 *      Points to an HRESULT which will be set to DI_TRUNCATED if
 *      any of the parameters were truncated.
 *
 *****************************************************************************/

void
HWSanitizeLevel(DWORD dwLevel, LPDWORD pdwLevelOut, HRESULT *phresTruncated)
{
    if ((LONG)dwLevel < 0) {
        *pdwLevelOut = 0;
        *phresTruncated = DI_TRUNCATED;
    } else if (dwLevel > DI_FFNOMINALMAX) {
        *pdwLevelOut = DI_FFNOMINALMAX;
        *phresTruncated = DI_TRUNCATED;
    } else {
        *pdwLevelOut = dwLevel;
    }
}

/*****************************************************************************
 *
 *      HWSanitizeEnv
 *
 *      Take a DirectInput DIEFFECT structure and copy its envelope
 *      to a new (scratch) DIENVELOPE structure, sanitizing the values
 *      along the way to avoid having to sanitize them later.
 *
 *  lMagnitude
 *
 *      Nominal magnitude for effect.
 *
 *  penvSrc
 *
 *      LPDIENVELOPE structure to sanitize, possibly NULL.
 *
 *  penvDst
 *
 *      LPDIENVELOPE structure to fill in.
 *
 *  phresTruncated
 *
 *      Points to an HRESULT which will be set to DI_TRUNCATED if
 *      any of the parameters were truncated.
 *
 *****************************************************************************/

void
HWSanitizeEnv(LONG lMagnitude, LPCDIENVELOPE penvSrc,
              LPDIENVELOPE penvDst, HRESULT *phresTruncated)
{
    /*
     *  This has no real effect, since the DIENVELOPE structure
     *  never goes outside our purview, but it doesn't hurt either.
     */
    penvDst->dwSize = sizeof(DIENVELOPE);

    /*
     *  Now on to the real work.
     */
    if (penvSrc) {

        /*
         *  First sanitize the envelope levels.
         */
        HWSanitizeLevel(penvSrc->dwAttackLevel,
                        &penvDst->dwAttackLevel, phresTruncated);

        HWSanitizeLevel(penvSrc->dwFadeLevel,
                        &penvDst->dwFadeLevel, phresTruncated);

        /*
         *  We'll deal with out-of-range times later.
         */
        penvDst->dwAttackTime = penvSrc->dwAttackTime;
        penvDst->dwFadeTime   = penvSrc->dwFadeTime;

    } else {
        /*
         *  If there is no envelope in the effect, then create a
         *  dummy envelope that has no perceptible effect.
         *
         *  In particular, setting the attack and fade times to
         *  zero keeps our imaginary hardware happy, and setting
         *  the attack and fade levels equal to the primary magnitude
         *  avoid discontinuities in playback.
         *
         *  Envelope magnitudes are always in absolute value, so
         *  we need to convert the signed lMagnitude to its
         *  absolute value.
         */
        if (lMagnitude < 0) {
            lMagnitude = -lMagnitude;
        }
        penvDst->dwAttackLevel = lMagnitude;
        penvDst->dwFadeLevel   = lMagnitude;
        penvDst->dwAttackTime  = 0;
        penvDst->dwFadeTime    = 0;
    }

}

/*****************************************************************************
 *
 *      HWSetEnvTimes
 *
 *      Take a DirectInput DIENVELOPE structure and set the time
 *      parameters in the HWENVTRIG structure in a form that our
 *      imaginary hardware supports.
 *
 *  dwDuration
 *
 *      Effect duration in microseconds.
 *
 *  penv
 *
 *      DirectInput DIENVELOPE structure.
 *
 *  phwenv
 *
 *      HWENVTRIG structure to fill in.
 *
 *  phresTruncated
 *
 *      Points to an HRESULT which will be set to DI_TRUNCATED if
 *      any of the parameters were truncated.
 *
 *****************************************************************************/

void
HWSetEnvTimes(DWORD dwDuration, LPCDIENVELOPE penv,
              PHWENVTRIG phwenv, HRESULT *phresTruncated)
{
    WORD wDuration;

    /*
     *  If the effect had no envelope, then HWSanitizeEnv
     *  will have created a null envelope for us, so there's
     *  no need to check here.
     */
    HWSetDuration(penv->dwAttackTime, &phwenv->wAttackTime, phresTruncated);
    HWSetDuration(penv->dwFadeTime,   &phwenv->wFadeTime,   phresTruncated);

    /*
     *  Use common worker function to convert DirectInput duration
     *  to imaginary hardware duration.
     */
    HWSetDuration(dwDuration, &wDuration, phresTruncated);

    /*
     *  Subtract the attack and fade times from the duration,
     *  leaving the sustain time.  Note that we do the math against
     *  the converted values instead of against the raw values.
     *  This avoids accumulated rounding errors.
     *
     *  If things get weird (e.g., attack time greater than sustain
     *  time), just peg at zero.
     *
     *  If the duration is infinite, then leave it alone, because
     *  infinity minus anything equals infinity.
     */

    if (wDuration < HWINFINITE) {
        /*
         *  Subtract the attack time from the sustain time, unless
         *  it would underflow, in which case we just peg at zero.
         */
        if (wDuration < phwenv->wAttackTime) {
            wDuration = 0;
            *phresTruncated = DI_TRUNCATED;
        } else {
            wDuration -= phwenv->wAttackTime;
        }

        /*
         *  Subtract the fade time from the sustain time, unless
         *  it would underflow, in which case we just peg at zero.
         */
        if (wDuration < phwenv->wFadeTime) {
            wDuration = 0;
            *phresTruncated = DI_TRUNCATED;
        } else {
            wDuration -= phwenv->wFadeTime;
        }
    }

    /*
     *  We've finished subtracting out the attack and fade times.
     *  What's left is the sustain time.
     */
    phwenv->wSustainTime = wDuration;
}

/*****************************************************************************
 *
 *      HWSetEnvLevels
 *
 *      Take a DirectInput DIENVELOPE structure and set the level
 *      parameters in the HWENVTRIG structure in a form that our
 *      imaginary hardware supports.
 *
 *  penv
 *
 *      DirectInput DIENVELOPE structure.
 *
 *  lMagnitude
 *
 *      Magnitude of sustain.
 *
 *  lOffset
 *
 *      Bias to apply to the envelope parameters and sustain magnitude.
 *      This value has already been validated to be in the range
 *      -FF_DINOMINALMAX to +FF_DINOMINALMAX.
 *
 *  dwGain
 *
 *      Gain level to apply to all magnitudes.
 *
 *  phwenv
 *
 *      HWENVTRIG structure to fill in.
 *
 *  phresTruncated
 *
 *      Points to an HRESULT which will be set to DI_TRUNCATED if
 *      any of the parameters were truncated.
 *
 *****************************************************************************/

void
HWSetEnvLevels(LPCDIENVELOPE penv, LONG lMagnitude, LONG lOffset, DWORD dwGain,
               PHWENVTRIG phwenv, HRESULT *phresTruncated)
{

    /*
     *  Use common worker function to set the sustain magnitude.
     */
    HWSetMagnitude(dwGain, lMagnitude + lOffset,
                   &phwenv->bSustainLevel, phresTruncated);

    /*
     *  If the effect had no envelope, then HWSanitizeEnv
     *  will have created a null envelope for us, so there's
     *  no need to check here.
     */

    HWSetMagnitude(dwGain, penv->dwAttackLevel + lOffset,
                   &phwenv->bAttackLevel, phresTruncated);
    HWSetMagnitude(dwGain, penv->dwFadeLevel + lOffset,
                   &phwenv->bFadeLevel, phresTruncated);

}

/*****************************************************************************
 *
 *      HWSetDeviceGain
 *
 *      Change the global gain of the device.
 *
 *  dwUnit
 *
 *      Hardware unit number; this tells us which port to use.
 *
 *  dwGain
 *
 *      Device gain, where DI_FFNOMINALMAX indicates "maximum".
 *
 *****************************************************************************/

HRESULT
HWSetDeviceGain(DWORD dwUnit, DWORD dwGain)
{
    HWSETGAIN gain;
    HRESULT hresTruncated;

    gain.bCommand = HWCOMMAND_SETGAIN;

    hresTruncated = S_OK;
    HWSetUnsignedValue(dwGain, &gain.bGain, &hresTruncated);

    HWSendCommand(dwUnit, sizeof(HWSETGAIN), &gain);

    /*
     *  If we had to truncate the gain, then return DI_TRUNCATED.
     */
    return hresTruncated;
}

/*****************************************************************************
 *
 *      HWResetDevice
 *
 *      Reset the device.
 *
 *      In addition to telling the device to reset itself, we also
 *      mark all the effect slots in the device as "Free".
 *
 *  dwUnit
 *
 *      Hardware unit number; this tells us which port to use.
 *
 *****************************************************************************/

HRESULT
HWResetDevice(DWORD dwUnit)
{
    HWRESET reset;
    int iEffect;

    /*
     *  Mark all effects as no longer busy (hence "free").
     */
    WaitForSingleObject(g_hmtxShared, INFINITE);
    for (iEffect = 0; iEffect < MAX_EFFECTS; iEffect++) {
        g_pshmem->rgus[dwUnit].rgfBusy[iEffect] = FALSE;
    }
    ReleaseMutex(g_hmtxShared);

    reset.bCommand = HWCOMMAND_RESET;

    HWSendCommand(dwUnit, sizeof(reset), &reset);

    return S_OK;
}

/*****************************************************************************
 *
 *      HWStopAllEffects
 *
 *      Stop all playing effects.
 *
 *  dwUnit
 *
 *      Hardware unit number; this tells us which port to use.
 *
 *****************************************************************************/

HRESULT
HWStopAllEffects(DWORD dwUnit)
{
    HWSTOPALL stop;

    stop.bCommand = HWCOMMAND_STOPALL;

    HWSendCommand(dwUnit, sizeof(HWSTOPALL), &stop);
    return S_OK;
}

/*****************************************************************************
 *
 *      HWGetHardwareInfo
 *
 *      Retrieve information about the hardware usage.
 *
 *  dwUnit
 *
 *      Hardware unit number; this tells us which port to use.
 *
 *  pinfo
 *
 *      Pointer to HARDWAREINFO structure that receives info about
 *      the device's memory information.
 *
 *****************************************************************************/

HRESULT
HWGetHardwareInfo(DWORD dwUnit, HARDWAREINFO *pinfo)
{
    /*
     *  We do device memory management on the host side, so no
     *  actual commands need to be sent.
     *
     *  The wMemoryInUse is the number of effects that are
     *  downloaded, and the wTotalMemory is the total number
     *  of effects we support.
     */
    int iEffect;

    WaitForSingleObject(g_hmtxShared, INFINITE);
    pinfo->wTotalMemory = MAX_EFFECTS;
    pinfo->wMemoryInUse = 0;
    for (iEffect = 0; iEffect < MAX_EFFECTS; iEffect++) {
        if (g_pshmem->rgus[dwUnit].rgfBusy[iEffect]) {
            pinfo->wMemoryInUse++;
        }
    }
    ReleaseMutex(g_hmtxShared);

    return S_OK;
}

/*****************************************************************************
 *
 *      HWDownloadConstantEffect
 *
 *      Download or update a constant-force effect.
 *
 *  dwUnit
 *
 *          Hardware unit number; this tells us which port to use.
 *
 *  dwEffect
 *
 *          The constant effect to update.
 *
 *  peff
 *
 *          The new parameters for the effect.  The axis and button
 *          values have been converted to object identifiers by
 *          DirectInput and have been validated by CEffDrv_DownloadEffect.
 *
 *  dwFlags
 *
 *          Zero or more DIEP_* flags specifying which
 *          portions of the effect information has changed from
 *          the effect already on the device.
 *
 *          This information is passed to drivers to allow for
 *          optimization of effect modification.  If an effect
 *          is being modified, a driver may be able to update
 *          the effect in situ and transmit to the device
 *          only the information that has changed.
 *
 *          Drivers are not, however, required to implement this
 *          optimization.  All fields in the DIEFFECT structure
 *          pointed to by the peff parameter are valid, and
 *          a driver may choose simply to update all parameters of
 *          the effect at each download.
 *
 *****************************************************************************/

HRESULT
HWDownloadConstantEffect(DWORD dwUnit, DWORD dwEffect,
                         LPCDIEFFECT peff, DWORD dwFlags)
{
    /*
     *  The HWCOMMAND_CREATECONST command takes a HWCONST structure.
     */
    HRESULT hresTruncated;
    HWCONST hwconst;
    LONG lMagnitude;
    LPDICONSTANTFORCE pconst = peff->lpvTypeSpecificParams;
    DIENVELOPE envSane;

    hresTruncated = S_OK;           /* Assume no truncation */

    /*
     *  Our device does not support incremental updates, so we have
     *  to update all the parameters every time.
     */

    hwconst.bCommand = HWCOMMAND_CREATECONST;
    hwconst.bEffect  = (BYTE)dwEffect;

    /*
     *  Use common worker function to set the direction.
     */
    HWSetDirection(peff, &hwconst.bDirection);

    /*
     *
     *  If the lMagnitude is negative, then flip the direction
     *  around.  This keeps the envelope parameters happy.
     */
    if (pconst->lMagnitude >= 0) {
        lMagnitude = pconst->lMagnitude;
    } else {
        hwconst.bDirection += 128;          /* Ignore overflow */
        lMagnitude = -pconst->lMagnitude;
    }

    /*
     *  Use common worker function to set the trigger.
     */
    HWSetTrigger(peff, &hwconst.env);

    /*
     *  Use common worker functions to set the envelope.
     */
    HWSanitizeEnv(lMagnitude, peff->lpEnvelope, &envSane, &hresTruncated);
    HWSetEnvTimes(peff->dwDuration, &envSane, &hwconst.env, &hresTruncated);
    HWSetEnvLevels(&envSane, lMagnitude, 0, peff->dwGain,
                   &hwconst.env, &hresTruncated);

    HWSendCommand(dwUnit, sizeof(hwconst), &hwconst);

    return hresTruncated;
}

/*****************************************************************************
 *
 *      HWDownloadBisineEffect
 *
 *      Internal function that downloads or update a sine or
 *      bisine periodic effect.
 *
 *  dwUnit
 *
 *          Hardware unit number; this tells us which port to use.
 *
 *  dwEffect
 *
 *          The constant effect to update.
 *
 *  peff
 *
 *          The new parameters for the effect.  The axis and button
 *          values have been converted to object identifiers by
 *          DirectInput and have been validated by CEffDrv_DownloadEffect.
 *
 *  dwFlags
 *
 *          Zero or more DIEP_* flags specifying which
 *          portions of the effect information has changed from
 *          the effect already on the device.
 *
 *          This information is passed to drivers to allow for
 *          optimization of effect modification.  If an effect
 *          is being modified, a driver may be able to update
 *          the effect in situ and transmit to the device
 *          only the information that has changed.
 *
 *          Drivers are not, however, required to implement this
 *          optimization.  All fields in the DIEFFECT structure
 *          pointed to by the peff parameter are valid, and
 *          a driver may choose simply to update all parameters of
 *          the effect at each download.
 *
 *  fBi
 *
 *          TRUE if the lpvTypeSpecificParams is a HWBISINEEFFECT
 *          structure.  Otherwise, it's just a DIPERIODIC.
 *
 *****************************************************************************/

HRESULT
HWDownloadBisineEffect(DWORD dwUnit, DWORD dwEffect,
                       LPCDIEFFECT peff, DWORD dwFlags, BOOL fBi)
{
    /*
     *  The HWCOMMAND_CREATEBISINE command takes a HWBISINE structure.
     */
    HRESULT hresTruncated;
    HWBISINE hwbisine;
    DWORD dwPhase;
    DIENVELOPE envSane;

    /*
     *  Note that this assumes that the HWBISINEEFFECT is an extension
     *  of the basic DIPERIODIC effect because we treat them the same
     *  except for the posisble dwMagnitude2 at the end.
     */
    LPCHWBISINEEFFECT ptsp = peff->lpvTypeSpecificParams;

    hresTruncated = S_OK;           /* Assume no truncation */

    /*
     *  Our device does not support incremental updates, so we have
     *  to update all the parameters every time.
     */

    hwbisine.bCommand = HWCOMMAND_CREATEBISINE;
    hwbisine.bEffect  = (BYTE)dwEffect;

    /*
     *  Use common worker function to set the trigger.
     */
    HWSetTrigger(peff, &hwbisine.env);

    /*
     *  Use common worker functions to set the envelope for the positive
     *  side of the effect.
     */
    HWSanitizeEnv(ptsp->periodic.dwMagnitude,
                  peff->lpEnvelope, &envSane, &hresTruncated);
    HWSetEnvTimes(peff->dwDuration, &envSane, &hwbisine.env, &hresTruncated);
    HWSetEnvLevels(&envSane,
                   ptsp->periodic.dwMagnitude,
                   ptsp->periodic.lOffset, peff->dwGain,
                   &hwbisine.env, &hresTruncated);

    /*
     *  Now set up all the "2" parameters, too.
     *
     *  The envelope parameters for the bottom half are the reflection
     *  of the parameters for the top half around the lOffset level.
     */
    HWSetMagnitude(peff->dwGain, ptsp->periodic.lOffset - envSane.dwAttackLevel,
                   &hwbisine.bAttackLevel2, &hresTruncated);
    HWSetMagnitude(peff->dwGain, ptsp->periodic.lOffset - envSane.dwFadeLevel,
                   &hwbisine.bFadeLevel2, &hresTruncated);

    if (fBi) {
        /*
         *  The sustain for the bottom half comes from
         *  the HWBISINEEFFECT structure.
         */
        HWSetMagnitude(peff->dwGain,
                       ptsp->periodic.lOffset - ptsp->dwMagnitude2,
                       &hwbisine.bSustainLevel2, &hresTruncated);
    } else {
        /*
         *  A regular sine effect, so the second magnitude is the same
         *  as the first.
         */
        HWSetMagnitude(peff->dwGain,
                       ptsp->periodic.lOffset - ptsp->periodic.dwMagnitude,
                       &hwbisine.bSustainLevel2, &hresTruncated);
    }

    /*
     *  The center of the effect is the lOffset itself.
     */
    HWSetMagnitude(peff->dwGain,
                   ptsp->periodic.lOffset,
                   &hwbisine.bCenter, &hresTruncated);

    /*
     *  Set the period of the loop.
     */
    HWSetDuration(ptsp->periodic.dwPeriod, &hwbisine.wPeriod, &hresTruncated);

    /*
     *  DirectInput reports the phase in hundredths of degrees,
     *  while our imaginary hardware accepts only multiples of
     *  90 degrees.
     *
     *  0 -   0 degrees
     *  1 -  90 degrees
     *  2 - 180 degrees
     *  3 - 270 degrees
     */
    dwPhase = ptsp->periodic.dwPhase;

    /*
     *  Convert to "number of 90-degrees units" with rounding.
     */
    dwPhase = (dwPhase + 90 * DI_DEGREES - 1) / (90 * DI_DEGREES);

    /*
     *  Be careful: We may have rounded 359 degrees up to
     *  360 degrees, which then divides to 4.  So take
     *  the result mod 4 to account for that possibility.
     */
    dwPhase = dwPhase % 4;

    hwbisine.bPhase = (BYTE)dwPhase;

    /*
     *  Use common worker function to set the direction.
     */
    HWSetDirection(peff, &hwbisine.bDirection);

    HWSendCommand(dwUnit, sizeof(hwbisine), &hwbisine);

    return hresTruncated;
}

/*****************************************************************************
 *
 *      HWSetSpringAxis
 *
 *      Convert a DirectInput DICONDITION structure into something our
 *      hardware understands.
 *
 *  pcond
 *
 *          The DirectInput DICONDITION structure.
 *
 *  paxis
 *
 *          The HWSPRINGAXIS structure to fill in.
 *
 *  Returns:
 *
 *      Returns S_OK if everything is okay.
 *
 *      Returns DI_TRUNCATED if any parameter got truncated.
 *
 *****************************************************************************/

HRESULT
HWSetSpringAxis(LPCDICONDITION pcond, PHWSPRINGAXIS paxis)
{
    HRESULT hresTruncated;

    /*
     *  Assume no truncation.
     */
    hresTruncated = S_OK;

    /*
     *  Instead of lOffset / lDeadBand, our hardware wants low/high.
     */
    HWSetSignedValue(pcond->lOffset - pcond->lDeadBand, &paxis->bLow,
                     &hresTruncated);

    HWSetSignedValue(pcond->lOffset + pcond->lDeadBand, &paxis->bHigh,
                     &hresTruncated);

    /*
     *  We don't support separate positive and negative coefficients,
     *  so we ignore the negative.
     */
    HWSetSignedValue(pcond->lPositiveCoefficient, &paxis->bCoeff,
                     &hresTruncated);

    /*
     *  And scale the saturation, too.
     */
    HWSetUnsignedValue(pcond->dwPositiveSaturation, &paxis->bSaturation,
                       &hresTruncated);

    return hresTruncated;
}

/*****************************************************************************
 *
 *      HWDownloadSpringEffect
 *
 *      Internal function that downloads or update a spring condition.
 *
 *  dwUnit
 *
 *          Hardware unit number; this tells us which port to use.
 *
 *  dwEffect
 *
 *          The constant effect to update.
 *
 *  peff
 *
 *          The new parameters for the effect.  The axis and button
 *          values have been converted to object identifiers by
 *          DirectInput and have been validated by CEffDrv_DownloadEffect.
 *
 *  dwFlags
 *
 *          Zero or more DIEP_* flags specifying which
 *          portions of the effect information has changed from
 *          the effect already on the device.
 *
 *          This information is passed to drivers to allow for
 *          optimization of effect modification.  If an effect
 *          is being modified, a driver may be able to update
 *          the effect in situ and transmit to the device
 *          only the information that has changed.
 *
 *          Drivers are not, however, required to implement this
 *          optimization.  All fields in the DIEFFECT structure
 *          pointed to by the peff parameter are valid, and
 *          a driver may choose simply to update all parameters of
 *          the effect at each download.
 *
 *****************************************************************************/

HRESULT
HWDownloadSpringEffect(DWORD dwUnit, DWORD dwEffect,
                       LPCDIEFFECT peff, DWORD dwFlags)
{
    /*
     *  The HWCOMMAND_CREATESPRING command takes a HWSPRING structure.
     */
    HRESULT hres, hresTruncated;
    HWSPRING hwspring;
    LPDICONDITION rgcond = peff->lpvTypeSpecificParams;
    DWORD iAxis;

    /*
     *  Our validation layer in CEffDrv_DownloadEffect has already
     *  filtered out rotated effects, so we can now be sure that
     *  the number of DICONDITION structures (rgcond) is exactly
     *  equal to peff->cAxes.
     */

    hresTruncated = S_OK;           /* Assume no truncation */

    /*
     *  Our device does not support incremental updates, so we have
     *  to update all the parameters every time.
     *
     *  For convenience, zero out everything.
     */
    ZeroMemory(&hwspring, sizeof(hwspring));

    hwspring.bCommand = HWCOMMAND_CREATESPRING;
    hwspring.bEffect  = (BYTE)dwEffect;

    /*
     *  Our spring effect doesn't support envelope or trigger
     *  or direction, so there's nothing to look at there.
     */

    /*
     *  If we have a one-axis effect, then we need to set zero
     *  for the axis not involved.
     *
     *  Since the app can set the axes as "Y then X" as well as
     *  "X then Y", you have to be careful how you parse the axes.
     */

    for (iAxis = 0; iAxis < peff->cAxes; iAxis++) {

        PHWSPRINGAXIS paxis;

        switch (DIDFT_GETINSTANCE(peff->rgdwAxes[iAxis])) {
        case 0:                 /* 0 = x-axis */
            paxis = &hwspring.axisX;
            break;

        case 1:                 /* 1 = y-axis */
            paxis = &hwspring.axisY;
            break;

        default:                /* Impossible; would never have         */
                                /* gotten past CEffDrv_DownloadEffect   */
            continue;           /* Um... ignore it */

        }

        if (HWSetSpringAxis(&rgcond[iAxis], paxis) == DI_TRUNCATED) {
            hresTruncated = DI_TRUNCATED;
        }
    }

    HWSendCommand(dwUnit, sizeof(hwspring), &hwspring);

    hres = hresTruncated;
    return hres;
}

/*****************************************************************************
 *
 *      HWDestroyEffect
 *
 *      Internal function that destroys an effect.
 *
 *      Our imaginary hardware is particularly simple in that there is
 *      no dynamic memory management; it's all static.  Therefore,
 *      to destroy an effect, you don't do anything!  You just mark
 *      the slot as free for the next effect to use.
 *
 *  dwUnit
 *
 *          Hardware unit number; this tells us which port to use.
 *
 *  dwEffect
 *
 *          The effect being destroyed.
 *
 *****************************************************************************/

HRESULT
HWDestroyEffect(DWORD dwUnit, DWORD dwEffect)
{
    return HWFreeEffectId(dwUnit, dwEffect);
}

/*****************************************************************************
 *
 *      HWStartEffect
 *
 *      Internal function that starts playback of an effect.
 *
 *  dwUnit
 *
 *          Hardware unit number; this tells us which port to use.
 *
 *  dwEffect
 *
 *          The effect to start.
 *
 *  dwMode
 *
 *          Flags specifying how this effect should interact with
 *          other effects.
 *
 *          DIES_SOLO - Stop all other effects when playing this one.
 *
 *          DirectInput handles the DIES_NODOWNLOAD flag internally;
 *          we will never see it.
 *
 *  dwCount
 *
 *          Number of times to play the effect.  We support only a
 *          loop count of 1.  (For simplicity, this hardware doesn't know 
 *          how to do multiple loops or INFINITE loops.)
 *
 *****************************************************************************/

HRESULT
HWStartEffect(DWORD dwUnit, DWORD dwEffect, DWORD dwMode, DWORD dwCount)
{
    /*
     *  We support only a loop count of 1.
     *
     *  Well, okay, we also support a loop count of zero because that's
     *  just a NOP.
     */
    if (dwCount > 1) {
        return E_NOTIMPL;
    }

    /*
     *  We don't support hardware DIES_SOLO, so we fake it by manually
     *  stopping all other effects first.
     */
    if (dwMode & DIES_SOLO) {
        HWStopAllEffects(dwUnit);
    }

    /*
     *  If we are actually being asked to play the effect, then play it.
     */
    if (dwCount) {
        HWSTART start;

        start.bCommand = HWCOMMAND_START;
        start.bEffect = (BYTE)dwEffect;

        HWSendCommand(dwUnit, sizeof(start), &start);
    }

    return S_OK;
}

/*****************************************************************************
 *
 *      HWStopEffect
 *
 *      Internal function that stops playback of an effect.
 *
 *  dwUnit
 *
 *          Hardware unit number; this tells us which port to use.
 *
 *  dwEffect
 *
 *          The effect to stop.
 *
 *****************************************************************************/

HRESULT
HWStopEffect(DWORD dwUnit, DWORD dwEffect)
{
    HWSTOP stop;

    stop.bCommand = HWCOMMAND_STOP;
    stop.bEffect = (BYTE)dwEffect;

    HWSendCommand(dwUnit, sizeof(stop), &stop);

    return S_OK;
}

/*****************************************************************************
 *
 *      HWGetEffectStatus
 *
 *      Internal function that determines whether an effect is still playing.
 *
 *  dwUnit
 *
 *          Hardware unit number; this tells us which port to use.
 *
 *  dwEffect
 *
 *          The effect to query.
 *
 *  pdwStatus
 *
 *          Receives effect status.
 *
 *          0 - not playing
 *          DIEGES_PLAYING - still playing
 *
 *****************************************************************************/

HRESULT
HWGetEffectStatus(DWORD dwUnit, DWORD dwEffect, LPDWORD pdwStatus)
{
    HWGETEFFSTATUS getstat;
    EFFSTATUS stat;

    getstat.bCommand = HWCOMMAND_GETEFFSTATUS;
    getstat.bEffect = (BYTE)dwEffect;
    HWSendCommand(dwUnit, sizeof(getstat), &getstat);

    HWReceiveData(dwUnit, sizeof(stat), &stat);

    if (stat.bPlaying) {
        *pdwStatus = DIEGES_PLAYING;
    } else {
        *pdwStatus = 0;
    }
    return S_OK;
}

/*****************************************************************************
 *
 *      HWGetEffectValue
 *
 *      Internal function that returns the instantaneous force
 *      value being played out the stick due to the specified effect.
 *
 *      This is not a standard DirectInput function; it is called by
 *      our private Escape.
 *
 *  dwUnit
 *
 *          Hardware unit number; this tells us which port to use.
 *
 *  dwEffect
 *
 *          The effect to query.
 *
 *  pinfo
 *
 *          Receives instantaneous effect force value.
 *
 *****************************************************************************/

HRESULT HWGetEffectValue(DWORD dwUnit, DWORD dwEffect,
                         ESCAPE_GETEFFECTVALUEINFO *pinfo)
{
    HWGETEFFVALUE getval;
    EFFVALUE val;

    getval.bCommand = HWCOMMAND_GETEFFVALUE;
    getval.bEffect = (BYTE)dwEffect;
    HWSendCommand(dwUnit, sizeof(getval), &getval);

    HWReceiveData(dwUnit, sizeof(val), &val);

    /*
     *  We scale the value back to DirectInput units (0 .. DI_FFNOMINALMAX)
     *  to allow for further device resolution in the future.
     */
    pinfo->dwEffectValue = MulDiv(val.bValue, DI_FFNOMINALMAX, 255);

    return S_OK;
}
