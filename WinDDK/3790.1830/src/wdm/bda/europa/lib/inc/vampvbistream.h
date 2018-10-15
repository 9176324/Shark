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
// @module   VampVbiStream | Driver interface for VBI streaming.
// @end
// Filename: VampVbiStream.h
//
// Routines/Functions:
//
//  public:
//          CVampVbiStream
//          ~CVampVbiStream
//          GetObjectStatus
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_VAMPVBISTREAM_H__566E1F04_8184_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPVBISTREAM_H__566E1F04_8184_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VampDeviceResources.h"
#include "VampVBasicStream.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  VampVbiStream | Driver interface for VBI streaming.
// @base public | CVampVBasicStream 
// @end
//
//////////////////////////////////////////////////////////////////////////////

class CVampVbiStream : public CVampVBasicStream 
{
    //@access public
public:
    //@cmember Constructor.<nl> 
    //Parameterlist:<nl>
    //CVampDeviceResources *pDevice // pointer to device resources<nl> 
	CVampVbiStream(
        IN CVampDeviceResources *pDevice );

    //@cmember Destructor.<nl> 
	virtual ~CVampVbiStream();

    //@cmember Overloaded Start() method from CVampVBasicStream.<nl> 
    VAMPRET Start();

    //@cmember Overloaded Stop() method from CVampVBasicStream.<nl> 
    //Parameterlist:<nl>
    //BOOL bRelease // if TRUE: release Channels<nl>
    VAMPRET Stop(
        IN BOOL bRelease );
};

#endif // !defined(AFX_VAMPVBISTREAM_H__566E1F04_8184_11D3_A66F_00E01898355B__INCLUDED_)
