//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 1999
//  All rights are reserved. Reproduction in whole or in part is prohibited
//  without the written consent of the copyright owner.
//
//  Philips reserves the right to make changes without notice at any time.
//  Philips makes no warranty, expressed, implied or statutory, including but
//  not limited to any implied warranty of merchantability or fitness for any
//  particular purpose, or that the use will not infringe any third party
//  patent, copyright or trademark. Philips must not be liable for any loss
//  or damage arising from its use.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// @doc      INTERNAL EXTERNAL
// @module   VampPowerCtrl | This module provides the functionality for 
//           changing the power states of the device.
// @end
//
// Filename: VampPowerCtrl.h
//
// Routines/Functions:
//
//  public:
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////

#if !defined _VAMPPOWERCRTL_H_ 
#define _VAMPPOWERCRTL_H_ 


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "OStypes.h"
#include "VampError.h"
#include "VampTypes.h"
#include "VampIo.h"

class CVampDeviceResources;



//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampPowerCtrl | Controls the clocks and PLL's for Audio and Video 
//         units. Saves and restores register data for power state change.
// @end
//
//////////////////////////////////////////////////////////////////////////////
class CVampPowerCtrl  
{
//@access private
private:
    //@cmember Pointer to device resources object.
    CVampDeviceResources *m_pDevRes;
    //@cmember Video PLL.
    ONOFF m_VideoPLL;
    //@cmember Video frontend 1.
    ONOFF m_VideoFrontend1;
    //@cmember Video frontend 2.
    ONOFF m_VideoFrontend2;
    //@cmember Audio PLL.
    ONOFF m_AudioPLL;
    //@cmember Audio frontend.
    ONOFF m_AudioFrontend;
    //@cmember Baseband DAC and ADC.
    ONOFF m_AudioAdcDac;
    //@cmember Current power state.
    eVampPowerState m_nCurrentPowerState;

    // @cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

//@access public
public:
    //@cmember Pointer to the register space of the device.
    DWORD *m_pdwRegSpace;

    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //CVampDeviceResources *pDevRes // Pointer to device resources object.<nl>
    CVampPowerCtrl( IN CVampDeviceResources *pDevRes );

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }
	
    //@cmember Destructor.<nl>
    virtual ~CVampPowerCtrl();

    //@cmember Checks whether the video clocks are turned on.<nl>
    BOOL IsVideoStreamingEnabled() 
    {
        return ( m_VideoPLL && m_VideoFrontend1 && m_VideoFrontend2 );
    }

    //@cmember Checks whether the audio clocks are turned on.<nl>
    BOOL IsAudioStreamingEnabled() 
    {
        return ( m_AudioPLL && m_AudioFrontend && m_AudioAdcDac );
    }
   
    //@cmember Sets the hardware power state.<nl>
    //Parameterlist:<nl>
    //ONOFF VideoPLL // Video PLL.<nl>
    //ONOFF VideoFrontend1 // Video frontend 1.<nl>
    //ONOFF VideoFrontend2 // Video frontend 2.<nl>
    //ONOFF AudioPLL // Audio PLL.<nl>
    //ONOFF AudioFrontend // Audio frontend.<nl>
    //ONOFF AudioAdcDac // Baseband DAC and ADC.<nl>
    VAMPRET SetPowerState( 
        IN ONOFF VideoPLL = ON,
        IN ONOFF VideoFrontend1 = ON,
        IN ONOFF VideoFrontend2 = ON,
        IN ONOFF AudioPLL = OFF,
        IN ONOFF AudioFrontend = OFF,
        IN ONOFF AudioAdcDac = OFF );

    //@cmember Returns the current hardware power state.<nl>
    //Parameterlist:<nl>
    //ONOFF *VideoPLL // Video PLL.<nl>
    //ONOFF *VideoFrontend1 // Video frontend 1.<nl>
    //ONOFF *VideoFrontend2 // Video frontend 2.<nl>
    //ONOFF *AudioPLL // Audio PLL.<nl>
    //ONOFF *AudioFrontend // Audio frontend.<nl>
    //ONOFF *AudioAdcDac // Baseband DAC and ADC.<nl>
    VAMPRET GetPowerState( 
        IN ONOFF *VideoPLL,
        IN ONOFF *VideoFrontend1,
        IN ONOFF *VideoFrontend2,
        IN ONOFF *AudioPLL,
        IN ONOFF *AudioFrontend,
        IN ONOFF *AudioAdcDac );

    //@cmember Saves and restores the register space when changing the power state.<nl>
    //Parameterlist:<nl>
    //eVampPowerState nPowerState // Power state to be set.<nl>
    VAMPRET ChangePowerState( 
        eVampPowerState nPowerState ); 

};
#endif //_VAMPPOWERCRTL_H_
