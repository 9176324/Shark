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
// @module   VampVideoStream | Driver interface for video streaming.
// @end
// Filename: VampVideoStream.h
//
// Routines/Functions:
//
//  public:
//          CVampVideoStream
//          ~CVampVideoStream
//          SetVisibleRect
//          GetObjectStatus
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_VAMPVIDEOSTREAM_H__566E1F03_8184_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPVIDEOSTREAM_H__566E1F03_8184_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VampError.h"
#include "VampTypes.h"
#include "VampVBasicStream.h"
#include "VampClipper.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampVideoStream | Video streaming methods.
// @base public | CVampVBasicStream 
// @base public | CVampClipper
// @end
//////////////////////////////////////////////////////////////////////////////

class CVampVideoStream : public CVampVBasicStream, public CVampClipper
 
{
    //@access public
public:
    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //CVampDeviceResources *pDevice // pointer to device resources<nl>
	CVampVideoStream(
        IN CVampDeviceResources *pDevice ); 

	//@cmember Returns the error status set by the base class constructors.<nl>
    DWORD GetObjectStatus()
    {
        DWORD dwObjectStatus;
        if (( dwObjectStatus = CVampClipper::GetObjectStatus() ) != STATUS_OK )
            return dwObjectStatus;
        return CVampVBasicStream::GetObjectStatus();
    }

    //@cmember Destructor.<nl>
	virtual ~CVampVideoStream();  

    //@cmember Opens a Video stream.<nl>
    //Parameterlist:<nl>
    //tStreamParams *p_Params // Pointer to stream parameters<nl>
    VAMPRET Open( 
        tStreamParms *p_Params );

    //@cmember Save settings to registry.<nl>
    void SaveSettings() 
    {
        CVampVBasicStream::SaveSettings ();
        SaveClipperSettings ();   
    }

};


#endif // !defined(AFX_VAMPVIDEOSTREAM_H__566E1F03_8184_11D3_A66F_00E01898355B__INCLUDED_)
