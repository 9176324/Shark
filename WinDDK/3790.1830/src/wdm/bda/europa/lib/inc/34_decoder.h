//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2000
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
// @module   Decoder | This class handles input selection and BCS
//           (Brightness, Contrast, and Saturation) control.

// 
// Filename: 34_Decoder.h
// 
// Routines/Functions:
//
//  public:
//          
//      CVampDecoder
//      ~CVampDecoder
//      SetStandard
//      GetStandard
//      SetColor
//      GetColor
//      GetDecoderStatus
//      SetSourceType
//      GetSourceType
//      SetVideoSource
//      GetVideoSource
//      SetChannel
//      SetCombFilter
//      GetCombFilter
//          
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _DECODER__
#define _DECODER__

#include "34api.h"
#include "34_error.h"
#include "34_types.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampDecoder | Methods for setting the registers of the decoder 
//         memory unit.

//
//////////////////////////////////////////////////////////////////////////////

class P34_API CVampDecoder  
{
//@access Public 
public:
	//@access Public functions
	//@cmember Constructor<nl>
    //Parameterlist:<nl>
    //DWORD dwDeviceIndex // Device Index<nl>
    CVampDecoder(
    IN DWORD dwDeviceIndex );     
    //@cmember Destructor<nl>
    virtual ~CVampDecoder();
    //@cmember Set video standard<nl>
    //Parameterlist:<nl>
    //eVideoStandard nVideoStandard // Actual video standard value<nl>
    VOID SetStandard(
        eVideoStandard nVideoStandard );
    //@cmember Get video standard<nl>
    //Parameterlist:<nl>
    //BOOL* bAutoStandard // Automatic video standard flag<nl>
    eVideoStandard GetStandard(BOOL* pbAutoStandard = NULL);
    //@cmember Set video color control<nl>
    //Parameterlist:<nl>
    //eColorControl nColorControl // Color control attribute<nl>
    //short nColorValue // Color control attribute value<nl>
    VOID SetColor(
        eColorControl nColorControl,
        SHORT        nColorValue );
    //@cmember Get video color control<nl>
    //Parameterlist:<nl>
    //eColorControl nColorControl,    // Color control attribute<nl>
    //PSHORT        nCurrentValue,    // Current color control attribute value<nl>
    //PSHORT        nMinValue,        // Minimum color control attribute value<nl>
    //PSHORT        nMaxValue,        // Maximum color control attribute value<nl>
    //PSHORT        nDefValue );      // Default color control attribute value<nl>
    VOID GetColor(
        eColorControl nColorControl,
        PSHORT        pnCurrentValue,
        PSHORT        pnMinValue = NULL,
        PSHORT        pnMaxValue = NULL,
        PSHORT        pnDefValue = NULL );
    //@cmember Get the decoder status<nl>
    //Parameterlist:<nl>
    //eVideoStatus nStatus // Desired status id<nl>
    DWORD GetDecoderStatus(
        eVideoStatus nStatus );
    //@cmember Set timing constant<nl>
    //Parameterlist:<nl>
    //eVideoSourceType nType   // Source type (e.g. VD_Camera, VD_TV,...)<nl>
    VAMPRET SetSourceType(
        eVideoSourceType nType );
    //@cmember Get video source type<nl>
    eVideoSourceType GetSourceType();
    //@cmember Set the video source<nl>
    //Parameterlist:<nl>
    //eVideoSource nSource  // Video source<nl>
    VAMPRET SetVideoSource(
        eVideoSource nSource );
    //@cmember Get the current video source<nl>
    //Parameterlist:<nl>
    //PDWORD pdwMode = NULL // Returns current video source
    eVideoSource GetVideoSource( 
			PDWORD pdwMode = NULL);
    //@cmember Set the video channel<nl>
    //Parameterlist:<nl>
    //DWORD   dwChannel  // Video channel<nl>
    VAMPRET SetChannel(
        DWORD   dwChannel );
    //@cmember Set luminance and chrominance comb filter.<nl>
    //Parameterlist:<nl>
    //BOOL bLuminance // TRUE: luminance comb filter active<nl>
    //BOOL bChrominance // TRUE: chrominance comb filter active<nl>
    //BOOL bBypass // TRUE: chrominance trap, luminance comb filter bypass (S-Video)<nl>
    VOID SetCombFilter( 
        BOOL bLuminance,
        BOOL bChrominance,
        BOOL bBypass );
    //@cmember Get luminance and chrominance comb filter.<nl>
    //Parameterlist:<nl>
    //BOOL *bLuminance // TRUE: luminance comb filter active<nl>
    //BOOL *bChrominance // TRUE: chrominance comb filter active<nl>
    //BOOL *bBypass // TRUE: chrominance trap, luminance comb filter bypass (S-Video)<nl>
    VOID GetCombFilter( 
        BOOL *bLuminance,
        BOOL *bChrominance,
        BOOL *bBypass );
//@access Private 
private:
    //@access Private variables
    // @cmember Device Index.<nl>
    DWORD m_dwDeviceIndex; 
};

#endif // _DECODER__
