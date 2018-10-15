//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 1999
//  All rights are reserved. Reproduction in whole or in part is prohibited
//  without the written consent of the copyright owner.  Philips reserves the
//  right to make changes without notice at any time. Philips makes no
//  warranty, expressed, implied or statutory, including but not limited to
//  any implied warranty of merchantability or fitness for any particular
//  purpose, or that the use will not infringe any third party patent,
//  copyright or trademark. Philips must not be liable for any loss or damage
//  arising from its use.
//  ////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// @doc      INTERNAL EXTERNAL
// @module   VampAudioCtrl | This file contains the audio controller class
//           definition.
// @end
//
// Filename: VampAudioCtrl.h
//
// Routines/Functions:
//
//  public:
//      CVampAudioCtrl
//      ~CVampAudioCtrl
//		SetLoopbackPath
//		SetI2SPath
//		SetStreamPath
//		GetPath
//		SetFormat
//		GetFormat
//		GetProperty
//		GetDMASize
//      GetObjectStatus
//
//  private:
//      IsValidInput
//      GetLinkedInput
//
//  protected:
//		InitTunerFE
//		InitAnalogFE
//		InitDSP
//		InitDmaMux
//		InitTunerMux
//		InitCrossMux
//		InitStereoDAC
//		InitI2SOut
//		InitStandard
//		GetStandard
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _AUDCTRL_H__
#define _AUDCTRL_H__

#include "VampAudioDetect.h"

class CVampAudioDetect;

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class   CVampAudioCtrl | This class is used to describe the controller
//          function. The audio controller class implements the audio path(s)
//          of a VAMP device. It manages the input port and output
//          port configuration which includes routing.
// @end
//
//////////////////////////////////////////////////////////////////////////////

class CVampAudioCtrl 
{
// @access private
private:
	void ReadRegistry();
	DWORD m_dwXTAL;
	DWORD m_dwFM_OutputLevelOne;
	DWORD m_dwFM_OutputLevelTwo;
	DWORD m_dwNICAM_AutoMuteSelect;
	DWORD m_dwNICAM_AutoMuteMode;
	DWORD m_dwNICAM_OutputLevel;
	DWORD m_dwSignal_StereoGain;
	DWORD m_dwSignal_Volumecontrol;
	DWORD m_dwI2S_OutputLevel;
	DWORD m_dwBoardSettingAudioTuner;
	DWORD m_dwBoardSettingAudioAnalog1;
	DWORD m_dwBoardSettingAudioAnalog2;
	DWORD m_dwBoardSettingTuner;
	DWORD m_dwBoardSettingComposite1;
	DWORD m_dwBoardSettingComposite2;
	DWORD m_dwBoardSettingSVideo1;
	DWORD m_dwBoardSettingSVideo2;
	DWORD m_dwBoardSettingFM;
    // @cmember Current Audio Standard<nl>
	AStandard m_tStandard;
    // @cmember Current Audio Format<nl>
	AFormat m_tFormat;
    // @cmember Current Audio Input connected to the Loopback Output<nl>
	AInput m_tLoopbackExternal;
    // @cmember Current Audio Input connected to the I2S Output<nl>
	AInput m_tI2SExternal;
    // @cmember Current Audio Input connected to the Streaming Output<nl>
	AInput m_tStreamExternal;
    // @cmember Current Audio Input connected to the Loopback Output<nl>
	AInputIntern m_tLoopback;
    // @cmember Current Audio Input connected to the I2S Output<nl>
	AInputIntern m_tI2S;
    // @cmember Current Audio Input connected to the Streaming Output<nl>
	AInputIntern m_tStream;
    // @cmember Current Audio Internal Input connected to the Tuner mux<nl>
	AInternalInput m_tI2SMux;
    // @cmember Current Audio Internal Input connected to the Dma mux<nl>
	AInternalInput m_tDmaMux;
    // @cmember DMA streaming properties<nl>
	tSampleProperty m_tSampleProperty;
    // @cmember Size of the DMA buffer<nl>
	DWORD m_tSampleCount;
    // @cmember Tuner already initialized.<nl>
    BOOL m_bTunerInitialized;
    // @cmember	Pointer to DeviceResources object<nl>
    CVampDeviceResources *m_pDeviceResources;   
    // @cmember	Pointer to AudioDetect object<nl>
    CVampAudioDetect *m_pAudioDetect; 
    // @cmember Error status flag, will be set during construction.<nl>
    DWORD m_dwObjectStatus;
	
    // @cmember	Sample counter<nl>
	ULONG m_ulSampleCount;

    // @cmember Check if the input is activated.<nl>
    // Parameterlist:<nl>
    // AInputIntern Input // audio input<nl>
  	BOOL IsValidInput(
        AInputIntern Input );

    // @cmember Get AV link.<nl>
    // Parameterlist:<nl>
    // AInputIntern Input // audio input<nl>
    AInputIntern GetLinkedInput(AInput tInput);

// @access protected
protected:
    // @cmember Initalize the Tuner Frontend.<nl>
    VAMPRET InitTunerFE();

    // @cmember Initalize the Analog Frontend.<nl>
    // Parameterlist:<nl>
    // BOOL  bEnable // TRUE: Enable, FALSE: Disable<nl>
    VAMPRET InitAnalogFE();

    // @cmember Initalize the DSP<nl>
    // Parameterlist:<nl>
	// AStandard	tStandard // Audio standard.<nl>
    VAMPRET InitDSP();

    // @cmember Initalize the DMA Multiplexer.<nl>
    VAMPRET InitDmaMux();

    // @cmember Initalize the I2S Multiplexer.<nl>
    VAMPRET InitI2SMux();

    // @cmember Initalize the Crossbar Multiplexer.<nl>
    VAMPRET InitLoopbackMux();

    // @cmember Initalize the Stereo DAC.<nl>
    VAMPRET InitStereoDAC();

    // @cmember Initalize the I2S Output.<nl>
    // Parameterlist:<nl>
    // BOOL  bEnable // TRUE: Enable, FALSE: Disable.<nl>
    VAMPRET InitI2SOut( 
        BOOL bEnable );

    // @cmember Get the Audio Standard.<nl>
    AStandard GetStandard();
	
    // @access public 
public:
 	void MuteAudio(BOOL bEnable);
  	// @cmember Constructor.<nl>
    // Parameterlist:<nl>
    // CVampDeviceResources* pDevice // pointer on DeviceResources object<nl>
    CVampAudioCtrl(	
        CVampDeviceResources* pDeviceResources );

    // @cmember Destructor.<nl>
    virtual ~CVampAudioCtrl();

    // @cmember Set the Audio Loopback Path  
	// (hardware related configuration).<nl>
    // Parameterlist:<nl>
	// AInput tLoopbackInput // Audio Source selection<nl>
    VAMPRET SetLoopbackPath( 
        AInput tLoopbackInput );

    // @cmember Set Audio I2S Path 
	// (hardware related configuration).<nl>
    // Parameterlist:<nl>
	// AInput tI2sInput // Audio Source selection<nl>
    VAMPRET SetI2SPath( 
        AInput tI2sInput );

    // @cmember Set Audio Streaming Path 
	// (hardware related configuration).<nl>
    // This configuration has to be done before any DMA stream handling.<nl>
	// Parameterlist:<nl>
	// AInput tStreamInput // Audio Source selection<nl>
	VAMPRET SetStreamPath( AInput tStreamInput );
	// @cmember Retrieve the corresponding Audio Input 
	// to the demanded Output.<nl>
	// Parameterlist:<nl>
	// AOutput tOutput // Audio Destination selection<nl>
    AInput GetPath( AOutput tOutput );

    // @cmember Set Audio Format.<nl>
	// Parameterlist:<nl>
    // AFormat tFormat // Audio Format<nl>
	VAMPRET SetFormat( 
        AFormat tFormat );

    // @cmember Get Audio Format.<nl>
    AFormat GetFormat();

    // @cmember Get the streaming sample properties.<nl>
	tSampleProperty GetProperty();

    // @cmember Get the DMA buffer size.<nl>
	DWORD GetDMASize();
    // @cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus();
};

#endif  // _AUDCTRL_H__
