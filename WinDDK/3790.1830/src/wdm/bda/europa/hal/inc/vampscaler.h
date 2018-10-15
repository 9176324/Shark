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
// @module   VampScaler | This module encapsulates the scaler functionality.
//           The scaler unit controls acquisition windows, horizontal
//           prescaling, vertical scaling, horizontal fine scaling and gamma
//           curve correction.
// @end
// Filename: VampScaler.h
//
// Routines/Functions:
//
//  public:
//          CVampScaler
//          ~CVampScaler
//          SetScalerDefaults
//          SetFieldFrequency
//          GetScalerStatus
//          SetOddEven
//          GetOddEven
//          SetAcquisitionWindow
//          GetAcquisitionWindow
//          SetOutputSize
//          GetOutputSize
//          SetGammaCurves
//          GetGammaCurves
//          SetGammaActive
//          GetGammaActive
//
//          SetClipModeNormal
//          SetClipMode
//          GetClipMode
//          SetOutputFormat
//          GetClipperStatus
//
//          GetObjectStatus
//          
//  private:
//          SetFilterControl
//          GetFilterControl
//          SetColor
//          GetColor
//          GetHorizontalPreScale
//          GetHorizontalPreScale
//          SetHorizontalFineScale
//          GetHorizontalFineScale
//          SetVerticalScale
//          GetVerticalScale
//          SetReferenceControl
//          GetReferenceControl
//          SetPostProcessing
//          GetPostProcessing
//          
//  protected:
//          Reset
//          SetScalerDefaults
//          ReadScalerSettings
//          SaveScalerSettings
//          ScalerSetRegs
//          ScalerGetRegs
//          
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_VAMPSCALER_H__B3D41841_7C91_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPSCALER_H__B3D41841_7C91_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VampError.h"
#include "VampTypes.h"
#include "VampDeviceResources.h"
#include "VampScalerData.h"

#undef  TYPEID
#define TYPEID CVAMPSCALER


//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampScaler | Methods and member variables for setting the scaler
//         registers. The register(field)s which are concerned are quoted in
//         equal signes: == register names ==.
// @end
//////////////////////////////////////////////////////////////////////////////

class CVampScaler  
{
	//@access private 
private:
    // @cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

    //@cmember Sets appropriate filter register.<nl>
    //Parameterlist:<nl>
    //DWORD dwFilterLuminance // FIR prefilter for luminance<nl>
    //DWORD dwFilterChrominance // FIR prefilter for chrominance<nl>
    VOID SetFilterControl(
        DWORD dwFilterLuminance,
        DWORD dwFilterChrominance )
    {
        m_dwFilterLuminance = dwFilterLuminance;
        m_dwFilterChrominance = dwFilterChrominance;

        DBGPRINT((1,"In - CVampScaler::SetFilterControl\n"));
    }

    //@cmember Gets appropriate filter register.<nl>
    //Parameterlist:<nl>
    //DWORD *pdwFilterLuminance // FIR prefilter for luminance<nl>
    //DWORD *pdwFilterChrominance // FIR prefilter for chrominance<nl>
    VOID GetFilterControl(
        DWORD *pdwFilterLuminance,
        DWORD *pdwFilterChrominance )
    {
        *pdwFilterLuminance = m_dwFilterLuminance;
        *pdwFilterChrominance = m_dwFilterChrominance;

        DBGPRINT((1,"In - CVampScaler::GetFilterControl\n"));
    }

    //@cmember  Set scaler color control.<nl>
    //Parameterlist:<nl>
    //eScalerColorControl nColorControl // Color control attribute<nl>
    //short nColorValue // Color control attribute value<nl>
    VOID SetColor(
        eScalerColorControl nColorControl,
        short nColorValue )
    {
        switch ( nColorControl )
        {
        case SC_BRIGHTNESS:
            m_dwBrightness = (DWORD)nColorValue;
            break;
        case SC_CONTRAST:
            m_dwContrast = (DWORD)nColorValue;
            break;
        case SC_SATURATION:
            m_dwSaturation = (DWORD)nColorValue;
            break;
        }

         DBGPRINT((1,"In - CVampScaler::SetColor\n"));
    }

    //@cmember  Get scaler color control.<nl>
    //Parameterlist:<nl>
    //eScalerColorControl nColorControl // Color control attribute<nl>
    short GetColor(
        eScalerColorControl nColorControl )
    {
        switch ( nColorControl )
        {
        case SC_BRIGHTNESS:
            return (short)m_dwBrightness;
        case SC_CONTRAST:
            return (short)m_dwContrast;
        case SC_SATURATION:
            return (short)m_dwSaturation;
        default:
            return 0;
        }

         DBGPRINT((1,"In - CVampScaler::GetColor\n"));
    }

    //@cmember Set horizontal prescaling and accumulation.<nl>
    //Parameterlist:<nl>
    //DWORD dwHorizontalPreScale // integer prescale ratio setting<nl>
    //DWORD dwAccumulationSeqLen // accumulation length setting<nl>
    //DWORD dwSeqWeight // sequence weighting bit<nl>
    //DWORD dwGainAdjust // gain factor and weighting bit<nl>
    VOID SetHorizontalPreScale(
        DWORD dwHorizontalPreScale,
        DWORD dwAccumulationSeqLen,
        DWORD dwSeqWeight,
        DWORD dwGainAdjust )
    {
        m_dwHorizontalPreScale = dwHorizontalPreScale;
        m_dwAccumulationSeqLen = dwAccumulationSeqLen;
        m_dwSeqWeight = dwSeqWeight;
        m_dwGainAdjust = dwGainAdjust;

        DBGPRINT((1,"In - CVampScaler::SetHorizontalPreScale\n"));
    }

    //@cmember Get horizontal prescaling and accumulation settings.<nl>
    //Parameterlist:<nl>
    //DWORD *pdwHorizontalPreScale // integer prescale ratio setting<nl>
    //DWORD *pdwAccumulationSeqLen // accumulation length setting<nl>
    //DWORD *pdwSeqWeight // sequence weighting bit<nl>
    //DWORD *pdwGainAdjust // gain factor and weighting bit<nl>
    VOID GetHorizontalPreScale(
        DWORD *pdwHorizontalPreScale,
        DWORD *pdwAccumulationSeqLen,
        DWORD *pdwSeqWeight,
        DWORD *pdwGainAdjust )
    {
        if ( !pdwHorizontalPreScale || !pdwAccumulationSeqLen 
                || !pdwSeqWeight || !pdwGainAdjust )
            return;

        *pdwHorizontalPreScale = m_dwHorizontalPreScale;
        *pdwAccumulationSeqLen = m_dwAccumulationSeqLen;
        *pdwSeqWeight = m_dwSeqWeight;
        *pdwGainAdjust = m_dwGainAdjust;

        DBGPRINT((1,"In - CVampScaler::GetHorizontalPreScale\n"));
    }

    //@cmember Set Horizontal fine scaling.<nl>
    //Parameterlist:<nl>
    //DWORD dwHorizontalScaleInc // hor. scaling increment<nl>
    //DWORD dwPhaseShiftY // hor. luminance phase offset<nl>
    //DWORD dwPhaseShiftC // hor. chrominance phase offset<nl>
    VOID SetHorizontalFineScale(
        DWORD dwHorizontalScaleInc,
        DWORD dwPhaseShiftY,
        DWORD dwPhaseShiftC )
    {
        m_dwHorizontalScaleInc = dwHorizontalScaleInc;
        m_dwPhaseShiftY = dwPhaseShiftY;
        m_dwPhaseShiftC = dwPhaseShiftC;

        DBGPRINT((1,"In - CVampScaler::SetHorizontalFineScale\n"));
    }

    //@cmember Get Horizontal fine scale settings.<nl>
    //Parameterlist:<nl>
    //DWORD *pdwHorizontalScaleInc // hor. scaling increment<nl>
    //DWORD *pdwPhaseShiftY // hor. luminance phase offset<nl>
    //DWORD *pdwPhaseShiftC // hor. chrominance phase offset<nl>
    VOID GetHorizontalFineScale(
        DWORD *pdwHorizontalScaleInc,
        DWORD *pdwPhaseShiftY,
        DWORD *pdwPhaseShiftC )
    {
        if ( !pdwHorizontalScaleInc || !pdwPhaseShiftY || !pdwPhaseShiftC )
            return;
        *pdwHorizontalScaleInc = m_dwHorizontalScaleInc;
        *pdwPhaseShiftY = m_dwPhaseShiftY;
        *pdwPhaseShiftC = m_dwPhaseShiftC;

        DBGPRINT((1,"In - CVampScaler::GetHorizontalFineScale\n"));
    }

    //@cmember Set Vertical scaling.<nl>
    //Parameterlist:<nl>
    //BOOL bMirror // line mirroring <nl>
    //DWORD dwInterpolationMode // LPI/ACM mode<nl>
    //DWORD dwVerticalScaleInc // ver. scaling increment<nl>
    //DWORD dwVerticalPhaseOffset // ver. phase offset<nl>
    VOID SetVerticalScale(
        BOOL bMirror,
        DWORD dwInterpolationMode,
        DWORD dwVerticalScaleInc,
        DWORD dwVerticalPhaseOffset )
    {
        m_bMirror = bMirror;
        m_dwInterpolationMode = dwInterpolationMode;
        m_dwVerticalScaleInc = dwVerticalScaleInc;
        m_dwVerticalPhaseOffset = dwVerticalPhaseOffset;

        DBGPRINT((1,"In - CVampScaler::SetVerticalScale\n"));
    }

    //@cmember Get vertical scale settings.<nl>
    //Parameterlist:<nl>
    //BOOL *pbMirror // line mirroring <nl>
    //DWORD *pInterpolationMode // LPI/ACM mode<nl>
    //DWORD *pdwVerticalScaleInc // ver. scaling increment<nl>
    //DWORD *pdwVerticalPhaseOffset // ver. phase offset<nl>
    VOID GetVerticalScale(
        BOOL *pbMirror,
        DWORD *pdwInterpolationMode,
        DWORD *pdwVerticalScaleInc,
        DWORD *pdwVerticalPhaseOffset )
    {
        if ( !pbMirror || !pdwInterpolationMode || !pdwVerticalScaleInc 
                || !pdwVerticalPhaseOffset )
            return;
        *pbMirror = m_bMirror;
        *pdwInterpolationMode = m_dwInterpolationMode;
        *pdwVerticalScaleInc = m_dwVerticalScaleInc;
        *pdwVerticalPhaseOffset = m_dwVerticalPhaseOffset;

        DBGPRINT((1,"In - CVampScaler::GetVerticalScale\n"));
    }

    //@cmember Set reference Bits.<nl>
    //Parameterlist:<nl>
    //DWORD dwHorTriggerEdge // Horizontal trigger edge (0/1) <nl>
    //DWORD dwVerTriggerEdge // Vertical trigger edge (0/1) <nl>
    //DWORD dwVerTriggerEdgePos // line on which vertical trigger edge occurs<nl>
    VOID SetReferenceControl(
        DWORD dwHorTriggerEdge,
        DWORD dwVerTriggerEdge,
        DWORD dwVerTriggerEdgePos )
    {
        m_dwHorTriggerEdge = dwHorTriggerEdge;
        m_dwVerTriggerEdge = dwVerTriggerEdge;
        m_dwVerTriggerEdgePos = dwVerTriggerEdgePos;

        DBGPRINT((1,"In - CVampScaler::SetReferenceControl\n"));
    }

    //@cmember Get reference Bits.<nl>
    //Parameterlist:<nl>
    //DWORD *pdwHorTriggerEdge // Horizontal trigger edge (0/1)<nl>
    //DWORD *pdwVerTriggerEdge // Vertical trigger edge (0/1)<nl>
    //DWORD *pdwVerTriggerEdgePos // line on which vertical trigger edge occurs <nl>
    VOID GetReferenceControl(
        DWORD *pdwHorTriggerEdge,
        DWORD *pdwVerTriggerEdge,
        DWORD *pdwVerTriggerEdgePos )
    {
        if ( !pdwHorTriggerEdge || !pdwVerTriggerEdge || !pdwVerTriggerEdgePos )
            return;
        *pdwHorTriggerEdge = m_dwHorTriggerEdge;
        *pdwVerTriggerEdge = m_dwVerTriggerEdge;
        *pdwVerTriggerEdgePos = m_dwVerTriggerEdgePos;

        DBGPRINT((1,"In - CVampScaler::GetReferenceControl\n"));
    }

    //@cmember Set post processing control regs.<nl>
    //Parameterlist:<nl>
    //BOOL ucRgbToYuvBypass // RGB to YUV matrix bypass (or active)<nl>
    //BOOL ucYuvToRgbBypass // YUV to RGB matrix bypass (or active)<nl>
    //BOOL ucGammaToDma // Gamma curve (or RGB to YUV) output switched to DMA port<nl>
    //BOOL ucGammaToGpio // Gamma curve (or RGB to YUV) output switched to GPIO port<nl>
    //BOOL ucFidC0 // vertical phase offset switched according field/page processing<nl>
    //DWORD dwSignalSourceSelect // data input selection: YUV from decoder or raw from ADC1/ADC2<nl>
    VOID SetPostProcessing(
        BOOL ucRgbToYuvBypass,
        BOOL ucYuvToRgbBypass, 
        BOOL ucGammaToDma,
        BOOL ucGammaToGpio, 
        BOOL ucFidC0, 
        DWORD dwSignalSourceSelect )
    {
        m_ucRgbToYuvBypass = ucRgbToYuvBypass;
        m_ucYuvToRgbBypass = ucYuvToRgbBypass; 
        m_ucGammaToDma = ucGammaToDma;
        m_ucGammaToGpio = ucGammaToGpio; 
        m_ucFidC0 =  ucFidC0; 
        m_dwSignalSourceSelect = dwSignalSourceSelect;

        DBGPRINT((1,"In - CVampScaler::SetPostProcessing\n"));
    }

    //@cmember Get post processing control bits.<nl>
    //Parameterlist:<nl>
    //BOOL *pucRgbToYuvBypass // RGB to YUV matrix bypass (or active)<nl>
    //BOOL *pucYuvToRgbBypass // YUV to RGB matrix bypass (or active)<nl>
    //BOOL *pucGammaToDma // Gamma curve (or RGB to YUV) output switched to DMA port<nl>
    //BOOL *pucGammaToGpio // Gamma curve (or RGB to YUV) output switched to GPIO port<nl>
    //BOOL *pucFidC0 // vertical phase offset switched according field/page processing<nl>
    //DWORD *pdwSignalSourceSelect // data input selection: YUV from decoder or raw from ADC1/ADC2 <nl>
    VOID GetPostProcessing(
        BOOL *pucRgbToYuvBypass,
        BOOL *pucYuvToRgbBypass, 
        BOOL *pucGammaToDma,
        BOOL *pucGammaToGpio, 
        BOOL *pucFidC0, 
        DWORD *pdwSignalSourceSelect )
    {
        if ( !pucRgbToYuvBypass || !pucYuvToRgbBypass || !pucGammaToDma 
                || !pucGammaToGpio || !pucFidC0 || !pdwSignalSourceSelect )
            return;
        *pucRgbToYuvBypass = m_ucRgbToYuvBypass;
        *pucYuvToRgbBypass = m_ucYuvToRgbBypass; 
        *pucGammaToDma = m_ucGammaToDma;
        *pucGammaToGpio = m_ucGammaToGpio; 
        *pucFidC0 = m_ucFidC0; 
        *pdwSignalSourceSelect = m_dwSignalSourceSelect;

        DBGPRINT((1,"In - CVampScaler::GetPostProcessing\n"));
    }

    // == SLock ==
    //@cmember Switch page access locking condition on/off.<nl>
    //Parameterlist:<nl>
    //BOOL bSLock  // TRUE: on,  FALSE: off<nl>
    VOID SetLockingCondition(
        BOOL bSLock )
    {
        DBGPRINT((1,"In - CVampScaler::SetLockingCondition\n"));

        m_bSLock = bSLock;
    }

    // == SLock ==
    //@cmember Get page access locking condition switch status.<nl>
    BOOL GetLockingCondition()
    {
        DBGPRINT((1,"In - CVampScaler::GetLockingCondition\n"));

        return m_bSLock;
    }

    //@cmember Check whether clipper is currently available.<nl>
    BOOL CheckClipperAvailable();

//@access protected
protected:
    //@cmember Pointer on DeviceResources object
    CVampDeviceResources *m_pDevice;
    //@cmember Pointer on CVampIo object
    CVampIo *m_pVampIo;
    //@cmember Primary scaler page (Task)
    CPage *m_Page;
    //@cmember Secondary scaler page for dual programming (VBI)
    CPage *m_PageSec;

    //@cmember Control flags to determine the scaler registers, 
    //which should be set
    DWORD m_dwScalerControlFlags;

    //@cmember Defines horizontal trigger edge  == DHEdge ==
    DWORD m_dwHorTriggerEdge;

    //@cmember Defines vertical trigger edge  == DVEdge ==
    DWORD m_dwVerTriggerEdge;

    //@cmember Field sequence: odd first / even first  == DFid ==
    eFieldSequence m_eFieldSequence;

    //@cmember Line number on which vertical trigger edge occurs  == YPos ==
    DWORD m_dwVerTriggerEdgePos;

    //@cmember Gamma correction curves structure 
    // == PGa0Start PGa1Start PGa2Start PGa0[4] PGa1[4] PGa2[4] ==
    tGammaCurves m_tGammaCurves;

    //@cmember Task A / B  == Page1 Page2 ==
    eTaskType m_eTask;

    //@cmember Task A / B  == Page1 Page2 ==
    eTaskType m_eActiveTask;

    //@cmember V-Field Type ODD, EVEN, etc.  == UOdd UEven ==
    eFieldType m_eField;

    //@cmember Field frequency 50/60 Hz  == U50 U60 ==
    eFieldFrequency m_eFieldFreq;

    //@cmember Signal source is locked  == SLock ==
    BOOL m_bSLock;

    //@cmember V-Stream Type: Video, VBI  == VidEn VbiEn ==
    eStreamType m_eStreamType;

    //@cmember RGB to YUV matrix bypass (or active)  == MaCon ==
    BOOL m_ucRgbToYuvBypass;

    //@cmember YUV to RGB matrix bypass (or active)  == CsCon ==
    BOOL m_ucYuvToRgbBypass;

    //@cmember Gamma curve (or RGB to YUV) output switched to DMA port  == PcSel ==
    BOOL m_ucGammaToDma;

    //@cmember Gamma curve (or RGB to YUV) output switched to GPIO port  == GpSel ==
    BOOL m_ucGammaToGpio;

    //@cmember Vertical phase offset switched according field/page processing
    //(or input field ID)  == FidC0 ==
    BOOL m_ucFidC0;

    //@cmember Data input selection: YUV from decoder or raw from ADC1/ADC2  == DRaw ==
    DWORD m_dwSignalSourceSelect;

    //@cmember Skip/Proc value of Task  == FSkip ==
    DWORD m_dwFSkip;

    //@cmember frame rate of tasks
    WORD m_wFrameRate[2];

    //@cmember input acquisition window  == VidEn DxStart DyStart 
    // DxStop DyStop VbiEn VxStart VyStart VxStop VyStop ==
    tVideoRect m_tVideoInputRect;

    //@cmember video output window width  == DxDif VxDif ==
    DWORD m_dwOutputSizeX;

    //@cmember video output window height  == DyDif VyDif ==
    DWORD m_dwOutputSizeY;

    //@cmember Horizontal integer down scaling (1...64)  == XPSc ==
    DWORD m_dwHorizontalPreScale;

    //@cmember Accumulation sequence length  == XAcL ==
    DWORD m_dwAccumulationSeqLen;

    //@cmember DC gain adjustment  == XDcG ==
    DWORD m_dwGainAdjust;

    //@cmember Sequence weighting  == XC21 ==
    DWORD m_dwSeqWeight;

    //@cmember luminance FIR prefilter control   == DPfY VPfY ==
    DWORD m_dwFilterLuminance;

    //@cmember chrominance FIR prefilter control   == DPfC VPfC ==
    DWORD m_dwFilterChrominance;

    //@cmember luminance Brightness control (nominal 0x80 / 128)  == Brightness ==
    DWORD m_dwBrightness;

    //@cmember luminance Contrast control (nominal 0x40 / 64, range 1/64 - 127/64)  == Contrast ==
    DWORD m_dwContrast;
    //
    //@cmember chrominance Saturation control (nominal 0x40 / 64, range 1/64 - 127/64)  == Saturation ==
    DWORD m_dwSaturation;

    //@cmember Horizontal scaling increment (incr. 65535...1 = factor ~1/64...~3.5)  == DXInc VXInc ==
    DWORD m_dwHorizontalScaleInc;

    //@cmember Phase shift luminance (1/32 - 255/32)  == DXPhShiftY VXPhShiftY ==
    DWORD m_dwPhaseShiftY;

    //@cmember Phase shift chrominance (m_dwPhaseShiftY/2)  == DXPhShiftC VXPhShiftC ==
    DWORD m_dwPhaseShiftC;

    //@cmember Vertical scaling increment (incr. 65535...1 = factor ~1/64...~1024)  == YInc ==
    DWORD m_dwVerticalScaleInc;

    //@cmember Horizontal image mirroring  == YMir ==
    BOOL m_bMirror;

    //@cmember linear interpolation (0) / phase correct accumulation (1)  == YMode ==
    DWORD m_dwInterpolationMode;

    //@cmember Vertical Phase Offsets 0...3  == VerPhaseOffset ==
    DWORD m_dwVerticalPhaseOffset;

    //@cmember Video output format  == PackModeDA PackModeDB PackModeVA PackModeVB ==
    eHWVideoOutputFormat m_nOutputFormat;

    //@cmember Flag for dithering  == DitherDA DitherDB DitherVA DitherVB ==
    BOOL m_bDitherEnable;

    //@cmember Cliplist inversion  == ClipListInvert ==
    BOOL m_bClipInvert;

    //@cmember Clipping mode  == ClipModeDA ClipModeDB ClipModeVA ClipModeVB ==
    eClipMode m_nClipMode;

    // Start Line of the Input Window
    //@cmember Start Line of the Input Window
    DWORD m_dwStartLineInYDirection;


    //@cmember Set the field freuency according to video standard.<nl>
    //Parameterlist:<nl>
    //eVideoStandard nVideoStandard // video standard<nl>
    VAMPRET SetFieldFrequency( 
        eVideoStandard nVideoStandard );

    
    //@cmember Get current state of scaler.<nl>
    DWORD GetScalerStatus()
    {
        DBGPRINT((1,"In - CVampScaler::GetScalerStatus\n"));

        return (DWORD)m_pVampIo->MunitScaler.Status_1;
    }

    // == DFid ==
    //@cmember set Odd or Even Field first<nl>
    //Parameterlist:<nl>
    //eFieldSequence nFieldSequence // flag for field sequence<nl>
    VOID SetOddEven(
        eFieldSequence nFieldSequence )
    {
        DBGPRINT((1,"In - CVampScaler::SetOddEven\n"));

        m_eFieldSequence = nFieldSequence;
    }

    // == DFid ==
    //@cmember Get Odd or Even Field sequence.<nl>
    eFieldSequence GetOddEven()
    {
        DBGPRINT((1,"In - CVampScaler::GetOddEven\n"));

        return m_eFieldSequence;
    }


    // == DxStart DyStart DxStop DyStop ==
    // == VxStart VyStart VxStop VyStop ==
    //@cmember Get video/VBI input window.<nl>
    tVideoRect *GetAcquisitionWindow()
    {
        DBGPRINT((1,"In - CVampScaler::GetAcquisitionWindow\n"));

        return (tVideoRect *)&m_tVideoInputRect;
    }

    //@cmember Set size of output window.<nl>
    //Parameterlist:<nl>
    //DWORD Xsize // horizontal size (pixel)<nl>
    //DWORD Ysize // vertical size (lines)<nl>
    VOID SetOutputSize(
        DWORD Xsize,
        DWORD Ysize )
    {
        DBGPRINT((1,"In - CVampScaler::SetOutputSize\n"));

        m_dwOutputSizeX = Xsize;                // set video output window width
        m_dwOutputSizeY = Ysize;                // set video output window height
        m_dwScalerControlFlags |= (SCA_CTR_OUTPUT_WIN|SCA_CTR_SCALE|SCA_CTR_FILTER);
    }

    // == DxDif DyDif VxDif VyDif ==
    //@cmember Get size of output window.<nl>
    //Parameterlist:<nl>
    //DWORD *Xsize // horizontal size (pixel)<nl>
    //DWORD *Ysize // vertical size (lines)<nl>
    VOID GetOutputSize(
        DWORD *Xsize,
        DWORD *Ysize )
    {
        DBGPRINT((1,"In - CVampScaler::GetOutputSize\n"));

        if ( !Xsize || !Ysize )
            return;
        *Xsize = m_dwOutputSizeX;               // get video output window width
        *Ysize = m_dwOutputSizeY;               // get video output window height
    }

    // == PGa0Start PGa1Start PGa2Start PGa0[4] PGa1[4] PGa2[4] ==
    //@cmember Set gamma curves.<nl>
    //Parameterlist:<nl>
    //tGammaCurves GammaCurves // struct of gamma curves arrays<nl>
    VOID SetGammaCurves(
        tGammaCurves nGammaCurves )
    {
        DBGPRINT((1,"In - CVampScaler::SetGammaCurves\n"));

        m_tGammaCurves = nGammaCurves;
    }

    // == PGa0Start PGa1Start PGa2Start PGa0[4] PGa1[4] PGa2[4] ==
    //@cmember Get gamma curves.<nl>
    tGammaCurves GetGammaCurves()
    {
        DBGPRINT((1,"In - CVampScaler::GetGammaCurves\n"));

        return m_tGammaCurves;
    }

    // == GaCon ==
    //@cmember Switch gamma post processing on/off.<nl>
    //Parameterlist:<nl>
    //BOOL bActive // TRUE: on,  FALSE: off<nl>
    VOID SetGammaActive(
        BOOL bActive )
    {
        DBGPRINT((1,"In - CVampScaler::SetGammaActive\n"));

        m_tGammaCurves.ucGammaActivate = bActive;
    }

    // == GaCon ==
    //@cmember Get gamma post processing switch status.<nl>
    BOOL GetGammaActive()
    {
        DBGPRINT((1,"In - CVampScaler::GetGammaActive\n"));

        return m_tGammaCurves.ucGammaActivate;
    }

    //@cmember Scale horizontal.<nl>
    //Parameterlist:<nl>
    //DWORD dwInputSize // size of original window<nl>
    //DWORD dwOutputSize // size of scaled window<nl>
    VAMPRET ScaleHorizontal( 
        DWORD dwInputSize,
        DWORD dwOutputSize );

    //@cmember Scale vertical.<nl>
    //Parameterlist:<nl>
    //DWORD dwInputSize // size of original window<nl>
    //DWORD dwOutputSize // size of scaled window<nl>
    VAMPRET ScaleVertical( 
        DWORD dwInputSize,
        DWORD dwOutputSize );

    //@cmember Set scaler registers to class member variables.<nl>
    //Parameterlist:<nl>
    //DWORD dwControlFlags // control flags for concerned registers to be set<nl>
    VAMPRET ScalerSetRegs( 
        DWORD dwControlFlags );

    //@cmember Get clipping mode.<nl>
    eClipMode GetClipMode()
    {
        return m_nClipMode;
    }

    //@cmember Get cliplist inversion flag.<nl>
    BOOL GetClipListInversion()
    {
        return m_bClipInvert;
    }

    //@cmember Set output format and dither.<nl>
    //Parameterlist:<nl>
    //BOOL bDitherEnable // dithering enable<nl>
    //eVideoOutputFormat nOutputFormat // output format<nl>
    VOID SetOutputFormat(
        BOOL bDitherEnable,
        eHWVideoOutputFormat nOutputFormat )
    {
        m_nOutputFormat = nOutputFormat;
        m_bDitherEnable = bDitherEnable ;
    }

    //@cmember Software reset.<nl>
    VOID Reset()
    {
        m_pVampIo->MunitScaler.SwRst = 1;
        m_pVampIo->MunitScaler.SwRst = 0;
    }

    //@cmember Set scaler register defaults.<nl>
    VOID SetScalerDefaults();

    //@cmember Read scaler register settings from registry.<nl>
    VAMPRET ReadScalerSettings();

    //@cmember Save scaler register settings to registry.<nl>
    VAMPRET SaveScalerSettings();

   //@cmember Get class member variables from scaler registers.<nl>
    VAMPRET ScalerGetRegs( DWORD dwControlFlags );

#if 0 // no longer necessary
    // enable scaler path of video/VBI for active page.<nl>
    VAMPRET ScalerEnable( 
        DWORD dwChannelRequest );

    // disable scaler path of video/VBI for active page.<nl>
    VAMPRET ScalerDisable( 
        DWORD dwChannelRequest );
#endif

    //@cmember Adjust enabled VBI tasks according to enabled video tasks.<nl>
    VAMPRET AdjustVbiToVideo();

    //@cmember set clip mode register of current stream to 0 to make 
    //clipper available for other streams.<nl>
    VOID ReleaseClipper();

//@access public 
public:
    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //CVampDeviceResources *pDevice // pointer on DeviceResources object<nl>
    CVampScaler(
        CVampDeviceResources *pDevice );

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Destructor.<nl>
    virtual ~CVampScaler();

    //@cmember Set clipping mode.<nl>
    //Parameterlist:<nl>
    //eClipMode nClipMode // clipping mode to set<nl>
    VOID SetClipMode(
        eClipMode nClipMode )
    {
        m_nClipMode = nClipMode;
        ScalerSetRegs( SCA_CTR_SC2DMA );
    }
    
    //@cmember Set cliplist inversion.<nl>
    //Parameterlist:<nl>
    //BOOL bInvert // cliplist inversion to be set<nl>
    VOID SetClipListInversion(
        BOOL bInvert )
    {
        m_bClipInvert = bInvert;
    }
    
    //@cmember Set video frame rate.<nl>
    //Parameterlist:<nl>
    //DWORD dwFrameRate // frame rate and fraction<nl>
    VOID SetFrameRate(
        DWORD dwFrameRate );

    //@cmember Get video frame rate<nl>
    DWORD GetFrameRate();

    //@cmember Adjust YPos for the different Videostandards.<nl>
    //Parameterlist:<nl>
    //eVideoStandard nVideoStandard // video standard<nl>
    DWORD AdjustYPos( 
        eVideoStandard nVideoStandard );

    // == VidEn DxStart DyStart DxStop DyStop ==
    // == VbiEn VxStart VyStart VxStop VyStop ==
    //@cmember set video/VBI input window.<nl>
    //Parameterlist:<nl>
    //tVideoRect InputWindow // rectangle input area to set<nl>
    VOID SetAcquisitionWindow(
        tVideoRect InputWindow );
};


#endif // !defined(AFX_VAMPSCALER_H__B3D41841_7C91_11D3_A66F_00E01898355B__INCLUDED_)
