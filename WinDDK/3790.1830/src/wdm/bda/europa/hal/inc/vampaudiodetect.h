//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2000
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
// @module   VampAudioDetect | This file contains the audio standard detection
//           class definition. 
// @end
//
// Filename: VampAudioDetect.h
//
// Routines/Functions:
//
//  public:
//      CVampAudioDetect
//      ~CVampAudioDetect
//      GetFormat
//      StartDetection
//      CallbackHandler
//      GetObjectStatus
//
//  protected:
//
//  private:
//      SetAudioStandard
//      ReadMonitor
//      SearchStd
//      SearchBG
//      SearchDK
//      SearchM
//      CheckSelection
//		ReadRegistryValues
//
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _AUDDETECT_H__
#define _AUDDETECT_H__


#include "OSDepend.h"

#include "VampDeviceResources.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @module   VampAudioDetect | The audio detection class implements the audio
//           standard detection functionality.
// @base public | COSDependAbstractCB
// @end
//
//////////////////////////////////////////////////////////////////////////////

class CVampAudioDetect : public COSDependAbstractCB
{
// @access private
private:
    
	DWORD dwLevel[4];
    
    // @cmember Current Audio Format<nl>
	AFormat m_tFormat;

    // @cmember	DeviceResources object<nl>
    CVampDeviceResources* m_pDeviceResources;

    // cmember Array of DSP standards (static)<nl>
	//ULONG m_ulSampleCount;

    // @cmember OSDependTimeOut object<nl>
	COSDependTimeOut* m_pTimeOutObj;

    // @cmember Counter<nl>
	DWORD m_dwCounter;

	// @cmember last audio output mode<nl>
	DWORD m_dwLastCSM;

	// @cmember Loop Counter<nl>
	DWORD m_dwLoopCounter;

	// @cmember Prefer A2 flag<nl>
	BOOL m_bPreferA2;

    // @cmember Maximum amplitude Sound Carrier 1<nl>
    INT m_nMaxSc1Std;

    // @cmember Maximum magnitude Sound Carrier 2<nl>
    INT m_nMaxSc2Mag;

    // @cmember Search state<nl>
    ASearchState m_nSearchState;

    // @cmember Search substate<nl>
    ASearchSubState m_nSearchSubState;

    // @cmember Peak substate<nl>
    ASearchSubState m_nPeakSubState;

    // @cmember Search result<nl>
    ASearchResult m_nSearchResult;

    // @cmember Automatic standard search selection<nl>
    INT m_nStandardSelection;

    // @cmember Static standard selection or default standard<nl>
    ASearchResult m_nDefaultStandard;

    // @cmember Detection mode (static / automatic)<nl>
    BOOL m_bDetectMode;

    // @cmember Search standard.<nl>
    VOID SearchStd();

    // @cmember Search BG standard.<nl>
    VOID SearchBG();

    // @cmember Search DK standard.<nl>
    VOID SearchDK();

    // @cmember Search M standard.<nl>
    VOID SearchM();

    // @cmember Search I standard.<nl>
    VOID SearchI();

    // @cmember Search L standard.<nl>
    VOID SearchL();

    // @cmember Check current standard selection.<nl>
    // Parameterlist:<nl>
    // INT nCarrierFrequency // carrier frequency<nl>
    BOOL CheckSelection(
        DWORD dwCarrierFrequency );

    // @cmember Set the Audio standard.<nl> 
    // Parameterlist:<nl>
    // ASearchResult nAudioStandard // Audio Standard selection<nl>
    VOID SetAudioStandard( 
        ASearchResult nAudioStandard );

    // @cmember Read Monitor_SelectData register.<nl> 
    // Parameterlist:<nl>
    // DWORD *pdwLevel // return value of register<nl>
    BOOL ReadMonitor( 
        DWORD *pdwLevel );

	VOID ReadRegistryValues();

    // @cmember Error status flag, will be set during construction.<nl>
    DWORD m_dwObjectStatus;

	BOOL m_bDetectedStandardFromRegistry;
	PVOID m_pEventHandler;

	DWORD m_dwNICAM_AutoMuteSelect;

	// @cmember unmute flag for timer object 2<nl>
	BOOL m_bUnMuteAudio;

	// @cmember unmute counter for timer object 2<nl>
	DWORD m_dwUnMuteCounter;
	
	// @cmember flag to signal active detection<nl>
	BOOL m_bDetectionActive;

// @access public 
public:
   	// @cmember Constructor.<nl>
    // Parameterlist:<nl>
    // CVampDeviceResources* pDevice // pointer on DeviceResources object<nl>
    CVampAudioDetect(	
        CVampDeviceResources* pDeviceResources );

    // @cmember Destructor.<nl>
    virtual ~CVampAudioDetect();

    // @cmember Get Audio Format.<nl>
    AFormat GetFormat();

    // @cmember Start the audio detection mode.<nl>
    VOID StartDetection();

	// @cmember Callback Handler for the timer class.<nl>
    VOID CallbackHandler();

    // @cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus();
};

#endif  // _AUDDETECT_H__
