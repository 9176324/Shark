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
// @module   VampDecoder | This class handles all video decoder settings like 
//           input selection or BCS (Brightness, Contrast, and Saturation) control.
// @end
// Filename: VampDecoder.h
// 
// Routines/Functions:
//
//  public:
//          
//      CVampDecoder
//      ~CVampDecoder
//      SetDecoderDefaults
//      SetStandard
//      GetStandard
//      Is50Hz
//      SetNoiseReduction
//      GetNoiseReduction
//      SetHTC
//      GetHTC
//      SetOddEvenToggle
//      GetOddEvenToggle
//      SetHorizontalPLL
//      GetHorizontalPLL
//      SetVideoInputMode
//      GetVideoInputMode
//      SetFunctionSel
//      GetFunctionSel
//      SetAPCKDelay
//      GetAPCKDelay
//      SetStdDetLatency
//      GetStdDetLatency
//      OutOfRange 
//      SetTest
//      GetTest  
//      SetColor
//      GetColor
//      SetGain
//      GetGain
//      SetHorIncDelay
//      GetHorIncDelay
//      SetSync
//      GetSync
//      GetDecoderStatus
//      SetSourceType
//      GetSourceType
//      SetVideoSource
//      GetVideoSource
//      SetCombFilter 
//      GetCombFilter 
//      SetYCBandwidth
//      GetYCBandwidth
//      SetUVOffset
//      GetUVOffset
//      SetYDelay  
//      GetYDelay
//      SetVGate 
//      GetVGate 
//      SaveSettings
//      GetObjectStatus
//          
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_VAMPDECODER_H__B3D41840_7C91_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPDECODER_H__B3D41840_7C91_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VampError.h"
#include "VampTypes.h"
#include "VampDeviceResources.h"


#undef  TYPEID
#define TYPEID CVAMPDECODER

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampDecoder | Methods and member variables for setting the
//         registers of the decoder  memory unit. The register(field)s which
//         are concerned are quoted  in equal signes: == register names ==.
// @end
//////////////////////////////////////////////////////////////////////////////


// NOTE:
// These RegFields are not yet covered by member variables/methods:
// HIncDel GainUpdL WPOff VBlankSL HLNoRefSel YBandw LDel FColTC
// DCVFil ClearDTO RTS0Pol RTS1Pol ColOn RTSEn OFtSel HLockSel XRefVSel
// XRefHSel RTCEn AOutSel_l UpdTC Cm99 LLClkEn

class CVampDecoder
{
private:
//@access private 
    //@cmember Pointer to DeviceResources object.
    CVampDeviceResources *m_pDevice;

	//@cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

    //@cmember Pointer to CVampIo object.
    CVampIo *m_pVampIo;

    //@cmember Video standard == FSel CStdSel Auto0 Auto1 AutoFDet == 
    eVideoStandard m_eVideoStandard;

    //@cmember Preferred video standard - set at driver installation time
    eVideoStandard m_eDefaultVideoStandard;

    //@cmember Video source type (e.g. CAMERA, TV,...).
    eVideoSourceType m_eVideoSourceType;

    //@cmember Video input source (e.g. TUNER, COMPOSITE_1,..).
    eVideoSource  m_eVideoSource;

    //@cmember Flag indicating the possible change of the video standard.
    BOOL m_bInputChanged;

    //@cmember Tuner Channel.
    DWORD m_dwTunerChannel;

    //@cmember First CVBS Channel.
    DWORD m_dwCVBSChannel1;

    //@cmember Second CVBS Channel.
    DWORD m_dwCVBSChannel2;

    //@cmember First SVHS Channel1.
    DWORD m_dwSVHSChannel1;

	//@cmember Second SVHS Channel2.
    DWORD m_dwSVHSChannel2;

    //@cmember Brightness == DecBri == 
    short m_sBrightness;
    //@cmember Contrast == DecCon == 
    short m_sContrast;
    //@cmember Saturation == DecSat ==  
    short m_sSaturation;
    //@cmember Hue == HueCtr == 
    short m_sHue;
    //@cmember Sharpness == YFilter ==
    short m_sSharpness;
    //@cmember Analog video input mode == YFilter ==
    DWORD m_dwVideoInputMode;
    //@cmember Analog function select == Fuse == 
    DWORD m_dwFunctionSel;
    //@cmember Analog gain control automatic (= 0) == GainFix == 
    DWORD m_dwGainAuto;
    //@cmember Analog gain control freeze == HoldGain == 
    DWORD m_dwGainFreeze;
    //@cmember Analog gain control channel 1 == Gain1_0 Gain1_8 == 
    DWORD m_dwGain1;
    //@cmember Analog gain control channel 2 == Gain1_0 Gain1_8 == 
    DWORD m_dwGain2;
    //@cmember Chroma gain control automatic (= 0) == AutoCGCtr ==
    DWORD m_dwCGainAuto;
    //@cmember Chroma gain control == CGain == 
    DWORD m_dwCGain;
    //@cmember Raw data gain control == RawGain == 
    DWORD m_dwRawGain;
    //@cmember Raw data offset control == RawOffs == 
    DWORD m_dwRawOffset;
    //@cmember Sync start control == HSBegin ==  
    short m_sSyncStart;
    //@cmember Sync stop control == HSStop == 
    short m_sSyncStop;
    //@cmember Noise reduction mode == VNoiseRed ==
    DWORD m_dwNoiseReduction;
    //@cmember Horizontal Time Constant Selection == HTimeCon ==
    DWORD m_dwHTCSelect;
    //@cmember Forced odd/even toggle == ForceOETog ==
    DWORD m_dwForcedOddEvenToggle;
    //@cmember Horizontal PLL (closed, open) == HPll ==
    DWORD m_dwHorizontalPLL;
    //@cmember Luminance comb filter == YComb ==  
    BOOL m_bCombfilterLum;
    //@cmember Chrominance comb filter == CComb == 
    BOOL m_bCombfilterChrom;
    //@cmember Chrominance trap, luminance comb filter bypass == CByps == 
    BOOL m_bCombfilterBypass;
    //@cmember Chrominance/luminance bandwidth adjustment == YCBandw == 
    DWORD m_dwYCBandwidth;
    //@cmember Chrominance bandwidth == CBandw == 
    DWORD m_dwCBandwidth;
    //@cmember Fine offset adjustment for V component == OffV ==
    DWORD m_dwOffsetV;
    //@cmember Fine offset adjustment for U component == OffU == 
    DWORD m_dwOffsetU;
    //@cmember Luminance delay == YDel == 
    DWORD m_dwYDelay;
    //@cmember HS fine position == HSDel == 
    DWORD m_dwHDelay;
    //@cmember ADC sample clock phase delay == APhClk ==
    DWORD m_dwAPCKDelay;
    //@cmember V-Gate start position == VgStart0 VgStart8 ==
    DWORD m_dwVGateStart;
    //@cmember V-Gate stop position == VgStop0 VgStop8 == 
    DWORD m_dwVGateStop;
    //@cmember Video standard detection search loop latency == Latency == 
    DWORD m_dwLatency;
    //@cmember Color peak control switch == ColPeakOff == 
    BOOL m_bColPeakOff;
    //@cmember White peak control switch == WhitePeakOff == 
    BOOL m_bWhitePeakOff;
    //@cmember Horizontal increment delay == HIncDel == 
    DWORD m_dwHIncDelay;
    //@cmember Alternative V-Gate position == VgPos == 
    BOOL m_bVGateAltPos;
    //@cmember Force color ON (automatic color killer) == ColOn == 
    BOOL m_bColorOn;
    
//@access protected 
protected:
    //@cmember Set the horizontal increment delay.<nl>
    //Parameterlist:<nl>
    //DWORD dwHIncDelay // horizontal increment delay 
    VOID SetHorIncDelay(
        DWORD dwHIncDelay )
    {
        DBGPRINT((1,"In - CVampDecoder::SetHorIncDelay\n"));
        m_pVampIo->MunitComDec.HIncDel = m_dwHIncDelay = dwHIncDelay;
   
        ResetDTO();
    }

    //@cmember Get the current horizontal increment delay.<nl>
    DWORD GetHorIncDelay()
    {
        DBGPRINT((1,"In - CVampDecoder::GetHorIncDelay\n"));
        return m_dwHIncDelay = (DWORD)m_pVampIo->MunitComDec.HIncDel;
    }


    //@cmember Set the analog video input mode.<nl>
    //Parameterlist:<nl>
    //DWORD dwMode // Video analog input mode 
    VOID SetVideoInputMode(
        DWORD dwMode );

    //@cmember Get the current analog video input mode.<nl>
    DWORD GetVideoInputMode()
    {
        DBGPRINT((1,"In - CVampDecoder::GetVideoInputMode\n"));
        return m_dwVideoInputMode = (DWORD)m_pVampIo->MunitComDec.AnCtrMode;
    }

    //@cmember Set horizontal PLL: 0 = closed, 1 = open (fixed hor.frequency).<nl>
    //Parameterlist:<nl>
    //DWORD dwHorizontalPLL // horizontal PLL value.<nl>
    VOID SetHorizontalPLL(
        DWORD dwHorizontalPLL )
    {
        DBGPRINT((1,"In - CVampDecoder::SetHorizontalPLL\n"));
        m_pVampIo->MunitComDec.HPll = m_dwHorizontalPLL = dwHorizontalPLL;

        ResetDTO();
    }

    //@cmember Get horizontal PLL.<nl>
    DWORD GetHorizontalPLL()
    {
        DBGPRINT((1,"In - CVampDecoder::GetHorizontalPLL\n"));
        return m_dwHorizontalPLL = (DWORD)m_pVampIo->MunitComDec.HPll;
    }


    //@cmember Set the analog function select.<nl>
    //Parameterlist:<nl>
    //DWORD dwFunctionSel // current analog function select 
    VOID SetFunctionSel(
        DWORD dwFunctionSel )
    {
        DBGPRINT((1,"In - CVampDecoder::SetFunctionSel\n"));
        m_pVampIo->MunitComDec.Fuse = m_dwFunctionSel = dwFunctionSel;

        ResetDTO();
    }

    //@cmember Get the current analog function select.<nl>
    DWORD GetFunctionSel()
    {    
        DBGPRINT((1,"In - CVampDecoder::GetFunctionSel\n"));
        return m_dwFunctionSel = (DWORD)m_pVampIo->MunitComDec.Fuse;
    }

    //@cmember Set the color peak control.<nl>
    //Parameterlist:<nl>
    //BOOL bColPeakOff // sets Color peak control off, if TRUE 
    VOID SetColorPeakControl(
        BOOL bColPeakOff )
    {
        DBGPRINT((1,"In - CVampDecoder::SetColorPeakControl\n"));
        m_pVampIo->MunitComDec.ColPeakOff = ((m_bColPeakOff = bColPeakOff ) != 0) ? 1 : 0;
        ResetDTO();
    }

    //@cmember Get the color peak control switch.<nl>
    BOOL GetColorPeakControl()
    {    
        DBGPRINT((1,"In - CVampDecoder::GetColorPeakControl\n"));
        return ( m_bColPeakOff = (BOOL)( (DWORD)m_pVampIo->MunitComDec.ColPeakOff ));
    }

    //@cmember Set the white peak control.<nl>
    //Parameterlist:<nl>
    //BOOL bWhitePeakOff // sets Color peak control off, if TRUE 
    VOID SetWhitePeakControl(
        BOOL bWhitePeakOff )
    {
        DBGPRINT((1,"In - CVampDecoder::SetWhitePeakControl\n"));
        m_pVampIo->MunitComDec.WPOff = (( m_bWhitePeakOff = bWhitePeakOff ) != 0) ? 1 : 0 ;

        //ResetDTO();
    }

    //@cmember Get the white peak control switch.<nl>
    BOOL GetWhitePeakControl()
    {    
        DBGPRINT((1,"In - CVampDecoder::GetWhitePeakControl\n"));
        return ( m_bWhitePeakOff = (BOOL)( (DWORD)m_pVampIo->MunitComDec.WPOff ));
    }

    //@cmember Set the forced color on.<nl>
    //Parameterlist:<nl>
    //BOOL bColorOn // sets forced color on active, if TRUE 
    VOID SetColorOn(
        BOOL bColorOn )
    {
        DBGPRINT((1,"In - CVampDecoder::SetColorOn\n"));
        m_pVampIo->MunitComDec.ColOn = (( m_bColorOn = bColorOn ) != 0) ? 1 : 0 ;
    }

    //@cmember Get the forced color on switch.<nl>
    BOOL GetColorOn()
    {    
        DBGPRINT((1,"In - CVampDecoder::GetColorOn\n"));
        return ( m_bColorOn = (BOOL)( (DWORD)m_pVampIo->MunitComDec.ColOn ));
    }

    //@cmember Set ADC sample clock phase delay.<nl>
    //Parameterlist:<nl>
    //DWORD dwAPCKDelay // clock phase delay
    VOID SetAPCKDelay(
        DWORD dwAPCKDelay )
    {
        DBGPRINT((1,"In - CVampDecoder::SetAPCKDelay\n"));
        m_pVampIo->MunitComDec.APhClk = m_dwAPCKDelay = dwAPCKDelay;

        ResetDTO();
    }

    //@cmember Get ADC sample clock phase delay.<nl>
    DWORD GetAPCKDelay()
    {
        DBGPRINT((1,"In - CVampDecoder::GetAPCKDelay\n"));
        return m_dwAPCKDelay = (DWORD)m_pVampIo->MunitComDec.APhClk;
    }

    //@cmember Set video standard detection search loop latency.<nl>
    //Parameterlist:<nl>
    //DWORD dwLatency // latency (1..7)
    VOID SetStdDetLatency(
        DWORD dwLatency )
    {
        DBGPRINT((1,"In - CVampDecoder::SetStdDetLatency\n"));
        m_pVampIo->MunitComDec.Latency = m_dwLatency = dwLatency;

        ResetDTO();
    }

    //@cmember Get video standard detection search loop latency.<nl>
    DWORD GetStdDetLatency()
    {
        DBGPRINT((1,"In - CVampDecoder::GetStdDetLatency\n"));
        return m_dwLatency = (DWORD)m_pVampIo->MunitComDec.Latency;
    }
     
    //@cmember Check if a value is out of range according to control flags.<nl>
    //Parameterlist:<nl>
    //DWORD dwControlFlags // control flags for range definition<nl>
    //short sValue // value to check 
    BOOL OutOfRange( 
        DWORD dwControlFlags, 
        short sValue );

    // == Test AnalogTest ==
    //@cmember Set decoder test environment registers.<nl>
    //Parameterlist:<nl>
    //BOOL bTestActive // set environment active / inactive<nl>
    //DWORD dwTestValue // set test values
    VOID SetTest(         
        BOOL bTestActive, 
        DWORD dwTestValue );
    
    //@cmember Get decoder test environment registers; returns TRUE if active.<nl>
    //Parameterlist:<nl>
    //BOOL bTestActive // set environment active / inactive<nl>
    //DWORD dwTestValue // get test values
    BOOL GetTest(  
        DWORD *pdwTestValue ); 

    //@cmember Reset the internal subcarrier DTO phase to 0 degrees.<nl>
    VOID ResetDTO();

//@access public 
public:
    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //CVampDeviceResources *pDevice // pointer on DeviceResources object
	CVampDecoder(
        CVampDeviceResources *pDevice );

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Destructor.
	virtual ~CVampDecoder();


    //@cmember Set default values to decoder registers.<nl>
    VOID SetDecoderDefaults();

    //@cmember Save the decoder settings to registry.<nl>
    VAMPRET SaveSettings();

    //@cmember Restore the decoder settings from Registry.<nl>
    VAMPRET ReadSettings();

    //@cmember Set video standard.<nl>
    //Parameterlist:<nl>
    //eVideoStandard nVideoStandard // video standard to be set
    VOID SetStandard(
        eVideoStandard nVideoStandard );

    //@cmember Get current video standard.<nl>
    //Parameterlist:<nl>
    //BOOL *bAutoStandard // TRUE if video standard is set automatically
    eVideoStandard GetStandard( 
        BOOL *bAutoStandard = NULL );

    //@cmember Checks whether a desired video standards is supported.<nl>
    //Returns TRUE, if supported; FALSE, if not.<nl>
    //Parameterlist:<nl>
    //eVideoStandard nVideoStandard // video standard to be checked
    BOOL IsStandardSupported(      
        eVideoStandard nVideoStandard );

    //@cmember Check for 50 Hz field freqency.<nl>
    //Returns TRUE, if 50 Hz - FALSE, if not.<nl>
    BOOL Is50Hz()
    {
        DBGPRINT((1,"In - CVampDecoder::Is50Hz\n"));

        switch (GetStandard())
        {
        case PAL:
        case NTSC_443_50:
        case PAL_N:
        case PAL_50HZ:
        case NTSC_50HZ:
        case SECAM_50HZ:
        case NTSC_N:
        case SECAM:
        case NO_COLOR_50HZ:
            return TRUE;
            break;

        case NTSC_M:
        case PAL_443_60:
        case NTSC_443_60:
        case PAL_M:
        case NTSC_JAP:        
        case NTSC_60HZ:
        case PAL_60HZ:   
        case NO_COLOR_60HZ:
            return FALSE;
            break;

        default:
            DBGPRINT((1,"In - CVampDecoder::Is50Hz : ERROR unknown\n"));
            return FALSE;
            break;
        }
    }

    //@cmember Set noise reduction mode.<nl>
    //Parameterlist:<nl>
    //DWORD dwNoiseRed // noise reduction mode
    VOID SetNoiseReduction(
        DWORD dwNoiseRed )
    {
        DBGPRINT((1,"In - CVampDecoder::SetNoiseReduction\n"));
        m_pVampIo->MunitComDec.VNoiseRed = m_dwNoiseReduction = dwNoiseRed;

        ResetDTO();
    }

    //@cmember Get the current noise reduction mode.<nl>
    DWORD GetNoiseReduction()
    {
        DBGPRINT((1,"In - CVampDecoder::GetNoiseReduction\n"));
        return m_dwNoiseReduction = (DWORD)m_pVampIo->MunitComDec.VNoiseRed;
    }

    //@cmember Set Horizontal Time Constant Selection.<nl>
    //Parameterlist:<nl>
    //DWORD dwSelHTC // Horizontal Time Constant bits
    VOID SetHTC(
        DWORD dwSelHTC )
    {
        DBGPRINT((1,"In - CVampDecoder::SetHTC\n"));
        m_pVampIo->MunitComDec.HTimeCon = m_dwHTCSelect = dwSelHTC;

        ResetDTO();
    }

    //@cmember Get current Horizontal Time Constant Selection.<nl>
    DWORD GetHTC()
    {
        DBGPRINT((1,"In - CVampDecoder::GetHTC\n"));
        return m_dwHTCSelect = (DWORD)m_pVampIo->MunitComDec.HTimeCon;
    }

    //@cmember Set forced odd/even toggle.<nl>
    //Parameterlist:<nl>
    //DWORD dwOddEvenToggle // forced odd/even toggle
    VOID SetOddEvenToggle(
        DWORD dwOddEvenToggle )
    {
        DBGPRINT((1,"In - CVampDecoder::SetOddEvenToggle\n"));
        m_pVampIo->MunitComDec.ForceOETog = m_dwForcedOddEvenToggle = dwOddEvenToggle;

        ResetDTO();
    }

    //@cmember Get forced odd/even toggle.<nl>
    DWORD GetOddEvenToggle()
    {
        DBGPRINT((1,"In - CVampDecoder::GetOddEvenToggle\n"));
        return m_dwForcedOddEvenToggle = (DWORD)m_pVampIo->MunitComDec.ForceOETog;
    }

    
    //@cmember Set video color control.<nl>
    //Parameterlist:<nl>
    //eColorControl nColorControl // color control attribute<nl>
    //short nColorValue // color control attribute value<nl>
    VOID SetColor(
        eColorControl nColorControl,
        short nColorValue );

    //@cmember Get video color control.<nl>
    //Parameterlist:<nl>
    //eColorControl nColorControl // Color control attribute<nl>
    //short *pnCurrentValue // current color control attribute value<nl>
    //short *pnMinValue // minimum color control attribute value (opt.)<nl>
    //short *pnMaxValue // maximum color control attribute value (opt.)<nl>
    //short *pnDefValue // default color control attribute value (opt.)<nl>
    VOID GetColor(
        eColorControl nColorControl,
        short *pnCurrentValue,
        short *pnMinValue = NULL,
        short *pnMaxValue = NULL,
        short *pnDefValue = NULL );
    
    //@cmember Set video gain control.<nl>
    //Parameterlist:<nl>
    //eColorControl nGainControl // gain control attribute<nl>
    //DWORD nGainValue // gain control attribute value<nl>
    VOID SetGain(
        eColorControl nGainControl,
        DWORD nGainValue );

    //@cmember Get video gain control.<nl>
    //Parameterlist:<nl>
    //eColorControl nGainControl // Gain control attribute<nl>
    //DWORD *pnCurrentValue // current gain control attribute value<nl>
    //DWORD *pnMinValue // minimum gain control attribute value (opt.)<nl>
    //DWORD *pnMaxValue // maximum gain control attribute value (opt.)<nl>
    //DWORD *pnDefValue // default gain control attribute value (opt.)<nl>
    VOID GetGain(
        eColorControl nGainControl,
        DWORD *pdwCurrentValue,
        DWORD *pdwMinValue = NULL,
        DWORD *pdwMaxValue = NULL,
        DWORD *pdwDefValue = NULL );
    
    //@cmember Set video sync control.<nl>
    //Parameterlist:<nl>
    //short sSyncStart // sync start value<nl>
    //short sSyncStop // sync stop value<nl>
    VAMPRET SetSync(
        short sSyncStart,
        short sSyncStop );

    //@cmember Get video sync control.<nl>
    //Parameterlist:<nl>
    //short *psSyncStart // current sync start value<nl>
    //short *psSyncStop // current sync stop value<nl>
    //short *pnMinValue // minimum sync start/stop value (opt.)<nl>
    //short *pnMaxValue // maximum sync start/stop value (opt.)<nl>
    //short *pnDefValue // default sync start/stop value (opt.)<nl>
    VOID GetSync(
        short *psSyncStart, 
        short *psSyncStop, 
        short *pnMinValue = NULL,
        short *pnMaxValue = NULL,
        short *pnDefValue = NULL );

    // == ChipVer DetColStd WPAct GainLimBot GainLimTop SlowTCAct HLock == 
    // == RdCap MVCopyProt MVColStripe MVProtType3 FidT HVLoopN Intl == 
    //@cmember Get the decoder status.<nl>
    //Parameterlist:<nl>
    //eVideoStatus nStatus // desired status control id.<nl>
    DWORD GetDecoderStatus(
        eVideoStatus nStatus );


    //@cmember Set timing constant depending on source type.<nl>
    //Parameterlist:<nl>
    //eVideoSourceType nType // Source type (VCR, CAMERA, TV)
    VAMPRET SetSourceType(
        eVideoSourceType nType );

    //@cmember Get current source type.<nl>
    eVideoSourceType GetSourceType();

    //@cmember Set the video source.<nl>
    //Parameterlist:<nl>
    //eVideoSource nSource // Video source 
    VAMPRET SetVideoSource(
        eVideoSource nSource );

    //@cmember Returns the current video source.<nl>
    //Parameterlist:<nl>
    //DWORD *dwMode // Current video input mode  
    eVideoSource GetVideoSource( 
        DWORD *dwMode = NULL );

    //@cmember Set luminance and chrominance comb filter.<nl>
    //Parameterlist:<nl>
    // BOOL bLuminance // TRUE: luminance comb filter active<nl>
    // BOOL bChrominance // TRUE: chrominance comb filter active<nl>
    // BOOL bBypass // TRUE: chrominance trap, luminance comb filter bypass (S-Video)<nl>
    VOID SetCombFilter( 
        BOOL bLuminance,
        BOOL bChrominance,
        BOOL bBypass );

    //@cmember Get luminance and chrominance comb filter.<nl>
    //Parameterlist:<nl>
    // BOOL *bLuminance // TRUE: luminance comb filter active<nl>
    // BOOL *bChrominance // TRUE: chrominance comb filter active<nl>
    // BOOL *bBypass // TRUE: chrominance trap, luminance comb filter bypass (S-Video)<nl>
    VOID GetCombFilter( 
        BOOL *bLuminance,
        BOOL *bChrominance,
        BOOL *bBypass );

    //@cmember Set luminance/chrominance bandwidth adjustment.<nl>
    //Parameterlist:<nl>
    //DWORD dwYCBandw // luminance/chrominance bandwidth adjustment (0..7)<nl>
    //DWORD dwCBandw // chrominance bandwidth (0/1)<nl>
    VOID SetYCBandwidth(
        DWORD dwYCBandw,
        DWORD dwCBandw );

    //@cmember Get luminance/chrominance bandwidth adjustment.<nl>
    //Parameterlist:<nl>
    //DWORD *pdwYCBandw // luminance/chrominance bandwidth adjustment (0..7)<nl>
    //DWORD *pdwCBandw // chrominance bandwidth (0/1)<nl>
    VOID GetYCBandwidth(
        DWORD *pdwYCBandw,
        DWORD *pdwCBandw );

    //@cmember Set U/V offset fine adjustment.<nl>
    //Parameterlist:<nl>
    //DWORD dwOffsetU // U offset (0..3)<nl>
    //DWORD dwOffsetV // V offset (0..3)<nl>
    VOID SetUVOffset(
        DWORD dwOffsetU,
        DWORD dwOffsetV );

    //@cmember Get U/V fine offset adjustment.<nl>
    //Parameterlist:<nl>
    //DWORD *pdwOffsetU // U offset (0..3)<nl>
    //DWORD *pdwOffsetV // V offset (0..3)<nl>
    VOID GetUVOffset(
        DWORD *pdwOffsetU,
        DWORD *pdwOffsetV );

    //@cmember Set luminance delay compensation.<nl>
    //Parameterlist:<nl>
    //DWORD dwYDelay // Y delay adjustment (0..7)<nl>
    //DWORD dwHDelay // HS fine position (0..3)<nl>
    VOID SetYDelay(  
        DWORD dwYDelay,  
        DWORD dwHDelay ); 

    //@cmember Get luminance delay compensation.<nl>
    //Parameterlist:<nl>
    //DWORD *pdwYDelay // Y delay adjustment (0..7)<nl>
    //DWORD *pdwHDelay // HS fine position (0..3)<nl>
    VOID GetYDelay(
        DWORD *pdwYDelay, 
        DWORD *pdwHDelay ); 

    //@cmember Set the V-Gate start/stop position.<nl>
    //Parameterlist:<nl>
    //BOOL bVGateAltPos // alternative position<nl>
    //DWORD DWORD dwVGateStart // V-Gate start<nl>
    //DWORD dwVGateStop // V-Gate stop<nl>
    VAMPRET SetVGate( 
        BOOL bVGateAltPos,  
        DWORD dwVGateStart, 
        DWORD dwVGateStop );

    //@cmember Get the V-Gate start/stop position.<nl>
    //Parameterlist:<nl>
    //BOOL *pbVGateAltPos // alternative position<nl>
    //DWORD *pdwVGateStart // V-Gate start<nl>
    //DWORD *pdwVGateStop // V-Gate stop<nl>
    //DWORD *pdwMinValue // minimum V-Gate value (opt.)<nl>
    //DWORD *pdwMaxValue // maximum V-Gate value (opt.)<nl>
    //DWORD *pdwDefValue // default V-Gate value (opt.)<nl>
    VOID GetVGate( 
        BOOL *pbVGateAltPos,
        DWORD *pdwVGateStart,
        DWORD *pdwVGateStop,
        DWORD *pdwMinValue = NULL, 
        DWORD *pdwMaxValue = NULL, 
        DWORD *pdwDefValue = NULL );


};

#endif // !defined(AFX_VAMPDECODER_H__B3D41840_7C91_11D3_A66F_00E01898355B__INCLUDED_)

