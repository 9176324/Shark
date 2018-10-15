/*****************************************************************************
 *
 *  EffDrv.c
 *
 *  Abstract:
 *
 *      Effect driver.
 *
 *      WARNING!  Since the effect driver is marked ThreadingModel="Both",
 *      all methods must be thread-safe.
 *
 *****************************************************************************/

#define INITGUID
#include "effdrv.h"

/*****************************************************************************
 *
 *      CEffDrv - Effect driver
 *
 *****************************************************************************/

typedef struct CEffDrv {

    /* Supported interfaces */
    IDirectInputEffectDriver ed;

    ULONG               cRef;           /* Object reference count */

    /*
     *  !!IHV!!  Add additional instance data here.
     *  (e.g., handle to driver you want to IOCTL to)
     */

    /*
     *  We remember the unit number because that tells us
     *  which I/O port we need to send the commands to.
     */
    DWORD               dwUnit;         /* Device unit number */

} CEffDrv;

/*****************************************************************************
 *
 *      CEffDrv_AddRef
 *
 *      Increment our object reference count (thread-safely) and return
 *      the new reference count.
 *
 *****************************************************************************/

STDMETHODIMP_(ULONG)
CEffDrv_AddRef(IDirectInputEffectDriver *ped)
{
    CEffDrv *this = (CEffDrv *)ped;

    InterlockedIncrement((LPLONG)&this->cRef);
    return this->cRef;
}


/*****************************************************************************
 *
 *      CEffDrv_Release
 *
 *      Decrement our object reference count (thread-safely) and
 *      destroy ourselves if there are no more references.
 *
 *****************************************************************************/

STDMETHODIMP_(ULONG)
CEffDrv_Release(IDirectInputEffectDriver *ped)
{
    ULONG ulRc;
    CEffDrv *this = (CEffDrv *)ped;

    ulRc = InterlockedDecrement((LPLONG)&this->cRef);
    if (ulRc == 0) {
        /*
         * !!IHV!! Clean up your instance data here.
         *  (e.g., close any handles you opened)
         */
        LocalFree(this);
    }

    return ulRc;
}

/*****************************************************************************
 *
 *      CEffDrv_QueryInterface
 *
 *      Our QI is very simple because we support no interfaces beyond
 *      ourselves.
 *
 *      riid - Interface being requested
 *      ppvOut - receives new interface (if successful)
 *
 *****************************************************************************/

STDMETHODIMP
CEffDrv_QueryInterface(IDirectInputEffectDriver *ped, REFIID riid, LPVOID *ppvOut)
{
    HRESULT hres;

    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid, &IID_IDirectInputEffectDriver)) {
        CEffDrv_AddRef(ped);
        *ppvOut = ped;
        hres = S_OK;
    } else {
        *ppvOut = 0;
        hres = E_NOINTERFACE;
    }
    return hres;
}

/*****************************************************************************
 *
 *      CEffDrv_DeviceID
 *
 *          DirectInput uses this method to inform us of
 *          the identity of the device.
 *
 *          For example, if a device driver is passed
 *          dwExternalID = 2 and dwInternalID = 1,
 *          then this means the interface will be used to
 *          communicate with joystick ID number 2, which
 *          corresonds to physical unit 1 in VJOYD.
 *
 *  dwDirectInputVersion
 *
 *          The version of DirectInput that loaded the
 *          effect driver.
 *
 *  dwExternalID
 *
 *          The joystick ID number being used.
 *          The Windows joystick subsystem allocates external IDs.
 *
 *  fBegin
 *
 *          Nonzero if access to the device is beginning.
 *          Zero if the access to the device is ending.
 *
 *  dwInternalID
 *
 *          Internal joystick id.  The device driver manages
 *          internal IDs.
 *
 *  lpReserved
 *
 *          Reserved for future use (HID).
 *
 *  Returns:
 *
 *          S_OK if the operation completed successfully.
 *
 *          Any DIERR_* error code may be returned.
 *
 *          Private driver-specific error codes in the range
 *          DIERR_DRIVERFIRST through DIERR_DRIVERLAST
 *          may be returned.
 *
 *****************************************************************************/

STDMETHODIMP
CEffDrv_DeviceID(IDirectInputEffectDriver *ped,
                 DWORD dwDirectInputVersion,
                 DWORD dwExternalID, DWORD fBegin,
                 DWORD dwInternalID, LPVOID pvReserved)
{
    CEffDrv *this = (CEffDrv *)ped;
    HRESULT hres;

    /*
     *  !!IHV!! Write code here.
     *
     *  Typically, you would record dwExternalID and/or dwInternalID
     *  in the CEffDrv structure for future reference.
     *
     *  Remember to call DllEnterCrit and DllLeaveCrit as necessary
     *  to remain thread-safe.
     */

    /*
     *  Remember the unit number because that tells us which of
     *  our devices we are talking to.  The DirectInput external
     *  joystick number is useless to us.  (We don't care if we
     *  are joystick 1 or joystick 2.)
     *
     *  Note that although our other methods are given an external
     *  joystick Id, we don't use it.  Instead, we use the unit
     *  number that we were given here.
     *
     *  Our hardware supports only MAX_UNITS units.
     */
    if (dwInternalID < MAX_UNITS) {
        this->dwUnit = dwInternalID;
        hres = S_OK;
    } else {
        hres = E_FAIL;
    }

    return hres;
}

/*****************************************************************************
 *
 *      CEffDrv_GetVersions
 *
 *          Obtain version information about the force feedback
 *          hardware and driver.
 *
 *  pvers
 *
 *          A structure which should be filled in with version information
 *          describing the hardware, firmware, and driver.
 *
 *          DirectInput will set the dwSize field
 *          to sizeof(DIDRIVERVERSIONS) before calling this method.
 *
 *  Returns:
 *
 *          S_OK if the operation completed successfully.
 *
 *          E_NOTIMPL to indicate that DirectInput should retrieve
 *          version information from the VxD driver instead.
 *
 *          Any DIERR_* error code may be returned.
 *
 *          Private driver-specific error codes in the range
 *          DIERR_DRIVERFIRST through DIERR_DRIVERLAST
 *          may be returned.
 *
 *****************************************************************************/

STDMETHODIMP
CEffDrv_GetVersions(IDirectInputEffectDriver *ped, LPDIDRIVERVERSIONS pvers)
{
    CEffDrv *this = (CEffDrv *)ped;
    HRESULT hres;

    /*
     *  !!IHV!! Write code here.
     *
     *  Remember to call DllEnterCrit and DllLeaveCrit as necessary
     *  to remain thread-safe.
     */

    if (pvers->dwSize >= sizeof(DIDRIVERVERSIONS)) {
        /*
         *  Tell DirectInput how much of the structure we filled in.
         */
        pvers->dwSize = sizeof(DIDRIVERVERSIONS);

        /*
         *  In real life, we would detect the version of the hardware
         *  that is connected to unit number this->dwUnit.
         */
        pvers->dwFirmwareRevision = 3;
        pvers->dwHardwareRevision = 5;
        pvers->dwFFDriverVersion = 3871;
        hres = S_OK;
    } else {
        hres = E_INVALIDARG;
    }

    return hres;
}

/*****************************************************************************
 *
 *      CEffDrv_Escape
 *
 *          DirectInput uses this method to communicate
 *          IDirectInputDevice2::Escape and
 *          IDirectInputEFfect::Escape methods to the driver.
 *
 *  dwId
 *
 *          The joystick ID number being used.
 *
 *  dwEffect
 *
 *          If the application invoked the
 *          IDirectInputEffect::Escape method, then
 *          dwEffect contains the handle (returned by
 *          mf IDirectInputEffectDriver::DownloadEffect)
 *          of the effect at which the command is directed.
 *
 *          If the application invoked the
 *          mf IDirectInputDevice2::Escape method, then
 *          dwEffect is zero.
 *
 *  pesc
 *
 *          Pointer to a DIEFFESCAPE structure which describes
 *          the command to be sent.  On success, the
 *          cbOutBuffer field contains the number
 *          of bytes of the output buffer actually used.
 *
 *          DirectInput has already validated that the
 *          lpvOutBuffer and lpvInBuffer and fields
 *          point to valid memory.
 *
 *  Returns:
 *
 *          S_OK if the operation completed successfully.
 *
 *          Any DIERR_* error code may be returned.
 *
 *          Private driver-specific error codes in the range
 *          DIERR_DRIVERFIRST through DIERR_DRIVERLAST
 *          may be returned.
 *
 *****************************************************************************/

STDMETHODIMP
CEffDrv_Escape(IDirectInputEffectDriver *ped,
               DWORD dwId, DWORD dwEffect, LPDIEFFESCAPE pesc)
{
    CEffDrv *this = (CEffDrv *)ped;
    HRESULT hres;

    /*
     *  !!IHV!! Write code here.
     *
     *  Remember to call DllEnterCrit and DllLeaveCrit as necessary
     *  to remain thread-safe.
     */

    if (dwEffect == 0) {
        /*
         *  Command is directed at the entire device.
         */
        switch (pesc->dwCommand) {

        /*
         *  The ESCAPE_SETCOLOR escape is a device escape which
         *  changes the color of an LED on the joystick.
         *
         *  The lpvInBuffer contains a ESCAPE_SETCOLORINFO structure.
         *
         *  No output is generated.
         */

        case ESCAPE_SETCOLOR:
            if (pesc->cbInBuffer == sizeof(ESCAPE_SETCOLORINFO)) {
                ESCAPE_SETCOLORINFO *pinfo = pesc->lpvInBuffer;
                pesc->cbOutBuffer = 0;          /* No output */
                hres = HWSetDeviceColor(this->dwUnit, pinfo->dwColor);
            } else {
                hres = E_INVALIDARG;
            }
            break;

        /*
         *  All other escapes are not implemented.
         */
        default:
            hres = E_NOTIMPL;
            break;

        }
    } else {

        /*
         *  Command is directed at a specific effect.
         */

        switch (pesc->dwCommand) {

        /*
         *  The ESCAPE_GETEFFECTVALUE escape is an effect escape which
         *  returns a ESCAPE_GETEFFECTVALUEINFO structure which contains
         *  the instantaneous force value being played out the stick
         *  due to the specified effect.
         */

        case ESCAPE_GETEFFECTVALUE:

            if (pesc->cbOutBuffer >= sizeof(ESCAPE_GETEFFECTVALUEINFO)) {
                pesc->cbOutBuffer = sizeof(ESCAPE_GETEFFECTVALUEINFO);
                hres = HWGetEffectValue(this->dwUnit, dwEffect,
                                        pesc->lpvOutBuffer);
            } else {
                hres = E_INVALIDARG;
            }

        /*
         *  All other escapes are not implemented.
         */
        default:
            hres = E_NOTIMPL;
            break;

        }
    }

    return hres;
}

/*****************************************************************************
 *
 *      CEffDrv_SetGain
 *
 *          Set the overall device gain.
 *
 *  dwId
 *
 *          The joystick ID number being used.
 *
 *  dwGain
 *
 *          The new gain value.
 *
 *          If the value is out of range for the device, the device
 *          should use the nearest supported value and return
 *          DI_TRUNCATED.
 *
 *  Returns:
 *
 *
 *          S_OK if the operation completed successfully.
 *
 *          DI_TRUNCATED if the value was out of range and was
 *          changed to the nearest supported value.
 *
 *          Any DIERR_* error code may be returned.
 *
 *          Private driver-specific error codes in the range
 *          DIERR_DRIVERFIRST through DIERR_DRIVERLAST
 *          may be returned.
 *
 *****************************************************************************/

STDMETHODIMP
CEffDrv_SetGain(IDirectInputEffectDriver *ped, DWORD dwId, DWORD dwGain)
{
    CEffDrv *this = (CEffDrv *)ped;
    HRESULT hres;

    /*
     *  !!IHV!! Write code here.
     *
     *  Remember to call DllEnterCrit and DllLeaveCrit as necessary
     *  to remain thread-safe.
     */

    /*
     *  Now tell the hardware about the new device gain.
     */

    hres = HWSetDeviceGain(this->dwUnit, dwGain);

    return hres;
}

/*****************************************************************************
 *
 *      CEffDrv_SendForceFeedbackCommand
 *
 *          Send a command to the device.
 *
 *  dwId
 *
 *          The external joystick number being addressed.
 *
 *  dwCommand
 *
 *          A DISFFC_* value specifying the command to send.
 *
 *  Returns:
 *
 *          S_OK on success.
 *
 *          Any DIERR_* error code may be returned.
 *
 *          Private driver-specific error codes in the range
 *          DIERR_DRIVERFIRST through DIERR_DRIVERLAST
 *          may be returned.
 *
 *****************************************************************************/


STDMETHODIMP
CEffDrv_SendForceFeedbackCommand(IDirectInputEffectDriver *ped,
                                 DWORD dwId, DWORD dwCommand)
{
    CEffDrv *this = (CEffDrv *)ped;
    HRESULT hres;

    /*
     *  !!IHV!! Write code here.
     *
     *  Remember to call DllEnterCrit and DllLeaveCrit as necessary
     *  to remain thread-safe.
     */

    /*
     *  Our device is pretty simple.  It does not support Pause
     *  (and therefore not Continue either), nor can you enable
     *  or disable the actuators.
     */

    switch (dwCommand) {

    case DISFFC_RESET:
        hres = HWResetDevice(this->dwUnit);
        break;

    case DISFFC_STOPALL:
        hres = HWStopAllEffects(this->dwUnit);
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }

    return hres;
}

/*****************************************************************************
 *
 *      CEffDrv_GetForceFeedbackState
 *
 *          Retrieve the force feedback state for the device.
 *
 *  dwId
 *
 *          The external joystick number being addressed.
 *
 *  pds
 *
 *          Receives device state.
 *
 *          DirectInput will set the dwSize field
 *          to sizeof(DIDEVICESTATE) before calling this method.
 *
 *  Returns:
 *
 *          S_OK on success.
 *
 *          Any DIERR_* error code may be returned.
 *
 *          Private driver-specific error codes in the range
 *          DIERR_DRIVERFIRST through DIERR_DRIVERLAST
 *          may be returned.
 *
 *****************************************************************************/

STDMETHODIMP
CEffDrv_GetForceFeedbackState(IDirectInputEffectDriver *ped,
                              DWORD dwId, LPDIDEVICESTATE pds)
{
    CEffDrv *this = (CEffDrv *)ped;
    HRESULT hres;

    /*
     *  !!IHV!! Write code here.
     *
     *  Remember to call DllEnterCrit and DllLeaveCrit as necessary
     *  to remain thread-safe.
     */

    if (pds->dwSize >= sizeof(DIDEVICESTATE)) {

        HARDWAREINFO info;

        /*
         *  Tell DirectInput how much of the structure we filled in.
         */
        pds->dwSize = sizeof(DIDEVICESTATE);

        /*
         *  Ask the hardware for state information.
         */
        hres = HWGetHardwareInfo(this->dwUnit, &info);

        /*
         *  If the hardware responded, fill in the data appropriately.
         */
        if (SUCCEEDED(hres)) {

            /*
             *  Start out empty and then work our way up.
             */
            pds->dwState = 0;

            /*
             *  If there are no effects, then DIGFFS_EMPTY.
             */
            if (info.wMemoryInUse == 0) {
                pds->dwState |= DIGFFS_EMPTY;
            }

            /*
             *  Our actuators are always on, for simplicity.
             */
            pds->dwState |= DIGFFS_ACTUATORSON;

            /*
             *  We can't report any of the other states.
             */

            /*
             *  MulDiv handles the overflow and divide-by-zero cases.
             */
            pds->dwLoad = MulDiv(100, info.wMemoryInUse, info.wTotalMemory);
        }

    } else {
        hres = E_INVALIDARG;
    }

    return hres;
}

/*****************************************************************************
 *
 *      CEffDrv_DownloadEffect
 *
 *          Send an effect to the device.
 *
 *  dwId
 *
 *          The external joystick number being addressed.
 *
 *  dwEffectId
 *
 *          Internal identifier for the effect, taken from
 *          the DIEFFECTATTRIBUTES structure for the effect
 *          as stored in the registry.
 *
 *  pdwEffect
 *
 *          On entry, contains the handle of the effect being
 *          downloaded.  If the value is zero, then a new effect
 *          is downloaded.  If the value is nonzero, then an
 *          existing effect is modified.
 *
 *          On exit, contains the new effect handle.
 *
 *          On failure, set to zero if the effect is lost,
 *          or left alone if the effect is still valid with
 *          its old parameters.
 *
 *          Note that zero is never a valid effect handle.
 *
 *  peff
 *
 *          The new parameters for the effect.  The axis and button
 *          values have been converted to object identifiers
 *          as follows:
 *
 *          - One type specifier:
 *
 *              DIDFT_RELAXIS,
 *              DIDFT_ABSAXIS,
 *              DIDFT_PSHBUTTON,
 *              DIDFT_TGLBUTTON,
 *              DIDFT_POV.
 *
 *          - One instance specifier:
 *
 *              DIDFT_MAKEINSTANCE(n).
 *
 *          Other bits are reserved and should be ignored.
 *
 *          For example, the value 0x0200104 corresponds to
 *          the type specifier DIDFT_PSHBUTTON and
 *          the instance specifier DIDFT_MAKEINSTANCE(1),
 *          which together indicate that the effect should
 *          be associated with button 1.  Axes, buttons, and POVs
 *          are each numbered starting from zero.
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
 *  Returns:
 *
 *          S_OK on success.
 *
 *          DI_TRUNCATED if the parameters of the effect were
 *          successfully downloaded, but some of them were
 *          beyond the capabilities of the device and were truncated.
 *
 *          DI_EFFECTRESTARTED if the parameters of the effect
 *          were successfully downloaded, but in order to change
 *          the parameters, the effect needed to be restarted.
 *
 *          DI_TRUNCATEDANDRESTARTED if both DI_TRUNCATED and
 *          DI_EFFECTRESTARTED apply.
 *
 *          Any other DIERR_* error code may be returned.
 *
 *          Private driver-specific error codes in the range
 *          DIERR_DRIVERFIRST through DIERR_DRIVERLAST
 *          may be returned.
 *
 *****************************************************************************/

STDMETHODIMP
CEffDrv_DownloadEffect(IDirectInputEffectDriver *ped,
                       DWORD dwId, DWORD dwEffectId,
                       LPDWORD pdwEffect, LPCDIEFFECT peff, DWORD dwFlags)
{
    CEffDrv *this = (CEffDrv *)ped;
    HRESULT hres;
    DWORD dwEffect;

    /*
     *  !!IHV!! Write code here.
     *
     *  Remember to call DllEnterCrit and DllLeaveCrit as necessary
     *  to remain thread-safe.
     */

    /*
     *  If the user is changing the trigger button, then validate it.
     *  It must be a button, and it must be button 0.  We do not
     *  support triggers on button 1.  (Simple imaginary hardware.)
     */

    if (dwFlags & DIEP_TRIGGERBUTTON) {     /* User is changing trigger */
        if (peff->dwTriggerButton == DIEB_NOTRIGGER) {
            /*
             *  No trigger, so vacuously valid.
             */
        } else if ((peff->dwTriggerButton & DIDFT_PSHBUTTON) &&
                   DIDFT_GETINSTANCE(peff->dwTriggerButton) == 0) {
            /*
             *  Button 0.  Good; that's the only one we support.
             */
        } else {
            /*
             *  Somehow a bad trigger button got through.
             *  Probably a corrupted registry entry.
             */
            hres = E_NOTIMPL;
            goto done;
        }
    }

    /*
     *  We don't support trigger autorepeat.
     */
    if (dwFlags & DIEP_TRIGGERREPEATINTERVAL) {
        if (peff->dwTriggerButton != DIEB_NOTRIGGER &&
            peff->dwTriggerRepeatInterval != INFINITE) {
            hres = E_NOTIMPL;
            goto done;
        }
    }

    /*
     *  If the user is changing the axes, then validate them.
     *  We have only two axes, X (0) and Y (1).
     */

    if (dwFlags & DIEP_AXES) {              /* User is changing axes */
        DWORD dwAxis;
        for (dwAxis = 0; dwAxis < peff->cAxes; dwAxis++) {
            if ((peff->rgdwAxes[dwAxis] & DIDFT_ABSAXIS) &&
                DIDFT_GETINSTANCE(peff->rgdwAxes[dwAxis]) < 2) {
                /* Axis is valid */
            } else {
                /*
                 *  Somehow a bad axis got through.
                 *  Probably a corrupted registry entry.
                 */
            hres = E_NOTIMPL;
            goto done;
            }
        }
    }

    /*
     *  The EFFECT_SPRING effect has additional constraints.
     */
    if (dwEffectId == EFFECT_SPRING) {
        /*
         *  EFFECT_SPRING doesn't support trigger.  This is a quick
         *  check so there's no performance gain in guarding it
         *  with "dwFlags & DIEP_TRIGGERBUTTON".
         */
        if (peff->dwTriggerButton != DIEB_NOTRIGGER) {
            hres = E_NOTIMPL;
            goto done;
        }

        /*
         *  We do not support a rotated spring effect.  We can
         *  detect this by seeing how many axes are involved
         *  and how many DICONDITION structures we received.
         */
        if (peff->cAxes * sizeof(DICONDITION) != peff->cbTypeSpecificParams) {
            hres = E_NOTIMPL;
            goto done;
        }

        /*
         *  Do not enforce the fact that we don't support envelopes
         *  on springs.  According to spec, an unsupported envelope
         *  is merely ignored.
         */
    }

    /*
     *  If DIEP_NODOWNLOAD is set, then we are merely being asked
     *  to validate parameters, so we're done.
     */
    if (dwFlags & DIEP_NODOWNLOAD) {
        hres = S_OK;
        goto done;
    }

    /*
     *  Our imaginary hardware doesn't support dynamically changing
     *  the parameters of effects while they are playing.  Therefore,
     *  if somebody demands that we not auto-restart and the effect
     *  is still playing, then we must fail with DIERR_EFFECTPLAYING.
     */
    if ((dwFlags & DIEP_NORESTART) && *pdwEffect) {
        DWORD dwStatus;
        hres = HWGetEffectStatus(this->dwUnit, *pdwEffect, &dwStatus);
        if (SUCCEEDED(hres) && (dwStatus & DIEGES_PLAYING)) {
            return DIERR_EFFECTPLAYING;
        }
    }

    /*
     *  Figure out which effect is being updated.
     *
     *  If we are downloading a brand new effect, then find a new effect
     *  id number for it.
     */

    dwEffect = *pdwEffect;
    if (dwEffect == 0) {
        hres = HWAllocateEffectId(this->dwUnit, &dwEffect);
        if (FAILED(hres)) {
            goto done;
        }
    }

    switch (dwEffectId) {

    case EFFECT_CONSTANT:
        hres = HWDownloadConstantEffect(this->dwUnit, dwEffect, peff, dwFlags);
        break;

    case EFFECT_BISINE:
        hres = HWDownloadBisineEffect(this->dwUnit, dwEffect, peff, dwFlags,
                                      TRUE);
        break;

    case EFFECT_SINE:
        hres = HWDownloadBisineEffect(this->dwUnit, dwEffect, peff, dwFlags,
                                      FALSE);
        break;

    case EFFECT_SPRING:
        hres = HWDownloadSpringEffect(this->dwUnit, dwEffect, peff, dwFlags);
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }

    if (SUCCEEDED(hres)) {
        *pdwEffect = dwEffect;

        /*
         *  If the DIEP_START flag is set, then start the effect too.
         *  Our imaginary hardware doesn't have a download-and-start
         *  feature, so we just do it separately.
         */
        if (dwFlags & DIEP_START) {
            hres = HWStartEffect(this->dwUnit, dwEffect, 0, 1);
        }
    } else {
        HWFreeEffectId(this->dwUnit, dwEffect);
    }

done:;
    return hres;
}

/*****************************************************************************
 *
 *      CEffDrv_DestroyEffect
 *
 *          Remove an effect from the device.
 *
 *          If the effect is playing, the driver should stop it
 *          before unloading it.
 *
 *  dwId
 *
 *          The external joystick number being addressed.
 *
 *  dwEffect
 *
 *          The effect to be destroyed.
 *
 *  Returns:
 *
 *          S_OK on success.
 *
 *          Any other DIERR_* error code may be returned.
 *
 *          Private driver-specific error codes in the range
 *          DIERR_DRIVERFIRST through DIERR_DRIVERLAST
 *          may be returned.
 *
 *****************************************************************************/

STDMETHODIMP
CEffDrv_DestroyEffect(IDirectInputEffectDriver *ped,
                      DWORD dwId, DWORD dwEffect)
{
    CEffDrv *this = (CEffDrv *)ped;
    HRESULT hres;

    /*
     *  !!IHV!! Write code here.
     *
     *  Remember to call DllEnterCrit and DllLeaveCrit as necessary
     *  to remain thread-safe.
     */
    hres = HWDestroyEffect(this->dwUnit, dwEffect);

    return hres;
}

/*****************************************************************************
 *
 *      CEffDrv_StartEffect
 *
 *          Begin playback of an effect.
 *
 *          If the effect is already playing, then it is restarted
 *          from the beginning.
 *
 *  @cwrap  LPDIRECTINPUTEFFECTDRIVER | lpEffectDriver
 *
 *  @parm   DWORD | dwId |
 *
 *          The external joystick number being addressed.
 *
 *  @parm   DWORD | dwEffect |
 *
 *          The effect to be played.
 *
 *  @parm   DWORD | dwMode |
 *
 *          How the effect is to affect other effects.
 *
 *          This parameter consists of zero or more
 *          DIES_* flags.  Note, however, that the driver
 *          will never receive the DIES_NODOWNLOAD flag;
 *          the DIES_NODOWNLOAD flag is managed by
 *          DirectInput and not the driver.
 *
 *  @parm   DWORD | dwCount |
 *
 *          Number of times the effect is to be played.
 *
 *  Returns:
 *
 *          S_OK on success.
 *
 *          Any other DIERR_* error code may be returned.
 *
 *          Private driver-specific error codes in the range
 *          DIERR_DRIVERFIRST through DIERR_DRIVERLAST
 *          may be returned.
 *
 *
 *****************************************************************************/

STDMETHODIMP
CEffDrv_StartEffect(IDirectInputEffectDriver *ped, DWORD dwId, DWORD dwEffect,
                    DWORD dwMode, DWORD dwCount)
{
    CEffDrv *this = (CEffDrv *)ped;
    HRESULT hres;

    /*
     *  !!IHV!! Write code here.
     *
     *  Remember to call DllEnterCrit and DllLeaveCrit as necessary
     *  to remain thread-safe.
     */
    hres = HWStartEffect(this->dwUnit, dwEffect, dwMode, dwCount);

    return hres;
}

/*****************************************************************************
 *
 *      CEffDrv_StopEffect
 *
 *          Halt playback of an effect.
 *
 *  dwId
 *
 *          The external joystick number being addressed.
 *
 *  dwEffect
 *
 *          The effect to be stopped.
 *
 *  Returns:
 *
 *          S_OK on success.
 *
 *          Any other DIERR_* error code may be returned.
 *
 *          Private driver-specific error codes in the range
 *          DIERR_DRIVERFIRST through DIERR_DRIVERLAST
 *          may be returned.
 *
 *
 *****************************************************************************/

STDMETHODIMP
CEffDrv_StopEffect(IDirectInputEffectDriver *ped, DWORD dwId, DWORD dwEffect)
{
    CEffDrv *this = (CEffDrv *)ped;
    HRESULT hres;

    /*
     *  !!IHV!! Write code here.
     *
     *  Remember to call DllEnterCrit and DllLeaveCrit as necessary
     *  to remain thread-safe.
     */
    hres = HWStopEffect(this->dwUnit, dwEffect);

    return hres;
}

/*****************************************************************************
 *
 *      CEffDrv_GetEffectStatus
 *
 *          Obtain information about an effect.
 *
 *  dwId
 *
 *          The external joystick number being addressed.
 *
 *  dwEffect
 *
 *          The effect to be queried.
 *
 *  pdwStatus
 *
 *          Receives the effect status in the form of zero
 *          or more DIEGES_* flags.
 *
 *  Returns:
 *
 *          S_OK on success.
 *
 *          Any other DIERR_* error code may be returned.
 *
 *          Private driver-specific error codes in the range
 *          DIERR_DRIVERFIRST through DIERR_DRIVERLAST
 *          may be returned.
 *
 *
 *****************************************************************************/

STDMETHODIMP
CEffDrv_GetEffectStatus(IDirectInputEffectDriver *ped, DWORD dwId, DWORD dwEffect,
                        LPDWORD pdwStatus)
{
    CEffDrv *this = (CEffDrv *)ped;
    HRESULT hres;

    /*
     *  !!IHV!! Write code here.
     *
     *  Remember to call DllEnterCrit and DllLeaveCrit as necessary
     *  to remain thread-safe.
     */
    hres = HWGetEffectStatus(this->dwUnit, dwEffect, pdwStatus);

    return hres;
}

/*****************************************************************************
 *
 *      The VTBL for our effect driver
 *
 *****************************************************************************/

IDirectInputEffectDriverVtbl CEffDrv_Vtbl = {
    CEffDrv_QueryInterface,
    CEffDrv_AddRef,
    CEffDrv_Release,
    CEffDrv_DeviceID,
    CEffDrv_GetVersions,
    CEffDrv_Escape,
    CEffDrv_SetGain,
    CEffDrv_SendForceFeedbackCommand,
    CEffDrv_GetForceFeedbackState,
    CEffDrv_DownloadEffect,
    CEffDrv_DestroyEffect,
    CEffDrv_StartEffect,
    CEffDrv_StopEffect,
    CEffDrv_GetEffectStatus,
};

/*****************************************************************************
 *
 *      CEffDrv_New
 *
 *****************************************************************************/

STDMETHODIMP
CEffDrv_New(REFIID riid, LPVOID *ppvOut)
{
    HRESULT hres;
    CEffDrv *this;

    this = LocalAlloc(LPTR, sizeof(CEffDrv));
    if (this) {

        /*
         *  Initialize the basic object management goo.
         */
        this->ed.lpVtbl = &CEffDrv_Vtbl;
        this->cRef = 1;
        DllAddRef();

        /*
         *  !!IHV!! Do instance initialization here.
         *
         *  (e.g., open the driver you are going to IOCTL to)
         *
         *  DO NOT RESET THE DEVICE IN YOUR CONSTRUCTOR!
         *
         *  Wait for the SendForceFeedbackCommand(SFFC_RESET)
         *  to reset the device.  Otherwise, you may reset
         *  a device that another application is still using.
         */

        /*
         *  Attempt to obtain the desired interface.  QueryInterface
         *  will do an AddRef if it succeeds.
         */
        hres = CEffDrv_QueryInterface(&this->ed, riid, ppvOut);
        CEffDrv_Release(&this->ed);

    } else {
        hres = E_OUTOFMEMORY;
    }

    return hres;

}
