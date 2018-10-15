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
// @module   VampClipper | The clipping class handles rectangle overlay
//           clipping with a clip table and color space conversion.
// @end
// Filename: VampClipper.h
// 
// Routines/Functions:
//
//  public:
//          CVampClipper
//          ~CVampClipper
//          SortIntegerArray
//          RemoveDuplicatedArrayElement
//          SetClipperDefaults
//          SetClipList
//          SetClipModeAlpha
//          SetClipModeColor
//          GetObjectStatus
//          
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_VAMPCLIPPER_H__B3D41842_7C91_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPCLIPPER_H__B3D41842_7C91_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VampError.h"
#include "VampTypes.h"
#include "VampDeviceResources.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampClipper | Methods for setting the registers of the Scaler_to_DMA 
//         memory unit.
// @end
//////////////////////////////////////////////////////////////////////////////

class CVampClipper  
{
//@access private
private:
    //@cmember Pointer on DeviceResources object.
    CVampDeviceResources *m_pDevice;
    //@cmember Pointer on CVampIo object.
    CVampIo *m_pVampIo;
	// @cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

	// @cmember Clipping info object pointer array.
    CScalerClippingInfo *pClipRectangles[16];

    //@cmember Sort array of integers.<nl>
    //Parameterlist:<nl>
    //DWORD *dwArray // pointer to an array of DWORDs<nl>
    //DWORD dwSizeOfArray // Size of the array<nl>
    VAMPRET SortIntegerArray(
        DWORD *dwArray,
        DWORD dwSizeOfArray );

    //@cmember Remove duplicate entry from array.<nl>
    //Parameterlist:<nl>
    //DWORD *dwArray // pointer to an array of DWORDs<nl>
    //DWORD dwSizeOfArray // Size of the array<nl>
    VAMPRET RemoveDuplicatedArrayElement(
        DWORD *dwArray,
        DWORD &dwSizeOfArray );

//@access protected
protected:
    //@cmember Factor for interlaced clipping (software workaround for hardware bug).
    LONG m_lFactor;

//@access public
public:
    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //  CVampDeviceResources *pDevice // pointer on DeviceResources object<nl>
	CVampClipper(
        CVampDeviceResources *pDevice );

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Destructor.<nl>
	virtual ~CVampClipper();

    //@cmember Setup cliplist.<nl>
    //Parameterlist:<nl>
    //tRectList *pRectList // list of visible rectangles<nl>
    VAMPRET SetClipList(
        tRectList *pRectList );

    //@cmember Enable alpha clipping.<nl>
    //Parameterlist:<nl>
    //BYTE ucAlphaNonClip // alpha value for non-clipping<nl>
    //BYTE ucAlphaClip // alpha value for clipping<nl>
    VAMPRET SetClipModeAlpha(
        BYTE ucAlphaNonClip,
        BYTE ucAlphaClip );
    
    //@cmember Enable color clipping.<nl>
    //Parameterlist:<nl>
    //BYTE ucRedColor // clip color portion red<nl>
    //BYTE ucGreenColor // clip color portion green<nl>
    //BYTE ucBlueColor // clip color portion blue<nl>
    VAMPRET SetClipModeColor(
        BYTE ucRedColor,
        BYTE ucGreenColor,
        BYTE ucBlueColor );
    
    //@cmember Set default values for clipping.<nl>
    VOID SetClipperDefaults();

    //@cmember Save clipper settings to registry.<nl>
    VAMPRET SaveClipperSettings();

    //@cmember Restore clipper settings from registry.<nl>
    VAMPRET ReadClipperSettings();
};

#endif // !defined(AFX_VAMPCLIPPER_H__B3D41842_7C91_11D3_A66F_00E01898355B__INCLUDED_)
