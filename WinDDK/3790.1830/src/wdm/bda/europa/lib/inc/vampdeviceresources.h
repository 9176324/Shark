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
// @module   VampDeviceResources | This class inherits all basic resources
//           that are available as 'one instance per chip'. An object of it
//           must be created at device initialization time. It contains
//           information about the chip instance. This information is
//           essential for accessing the registry and also for debugging.
// @end
// Filename: VampDeviceResources.h
// 
// Routines/Functions:
//
//  public:
//          CVampDeviceResources
//          ~CVampDeviceResources
//          
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_VAMPDEVICERESOURCES_H__2269FFA0_9299_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPDEVICERESOURCES_H__2269FFA0_9299_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VampIo.h"
#include "VampStreamManager.h"
#include "VampDecoder.h"
#include "VampI2C.h"
#include "VampPowerCtrl.h"
#include "VampGPIO.h"
#include "VampAudioCtrl.h"

class CVampStreamManager;
class CVampDecoder;
class CVampGPIO;
class CVampI2c;
class CVampAudioCtrl;

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampDeviceResources | Container for device dependent resources.<nl>
// @end 
//////////////////////////////////////////////////////////////////////////////

class CVampDeviceResources
{
// @access private
private:
    // @cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

// @access public
public:
    // @cmember Factory object for creation of COSDepend objects from inside the HAL.<nl>
    COSDependObjectFactory* m_pFactory;
	// @cmember Device instance ID.<nl>
    tDeviceParams *m_pDeviceParams; 
    // @cmember Pointer to COSDependRegistry object.<nl>
    COSDependRegistry *m_pRegistry;
    // @cmember Pointer to CVampIo class.<nl>
    CVampIo *m_pVampIo;  
    // @cmember Pointer to CVampStreamManager object.<nl>
    CVampStreamManager *m_pStreamManager;
    // @cmember Pointer to CVampDecoder object.<nl>
    CVampDecoder *m_pDecoder;
    // @cmember Pointer to CVampGPIO object.<nl>
    CVampGPIO *m_pGPIO;
    // @cmember Pointer to CVampI2C object.<nl>
    CVampI2c *m_pI2c;
    // @cmember Pointer to CVampAudioCtrl class.<nl>
    CVampAudioCtrl *m_pAudioCtrl;
    // @cmember Pointer to CVampPowerCtrl object.<nl>
    CVampPowerCtrl *m_pPowerCtrl;
    // @cmember Pointer to os/dependent synchronisation object. <nl>
    COSDependSyncCall *m_pSyncCall;

    tPageAddressInfo m_p32BitPagePool[2 * NUM_DMA_CHANNELS];

    // @cmember Constructor.<nl>
    // Parameterlist:<nl>
    // IN COSDependObjectFactory *pFactory // factory object for creation of COSDepend objects<nl>
    // IN tDeviceParams *p_DevPar // device parameters<nl>
    // IN COSDependRegistry *pRegistry // registry access class<nl>
    // IN COSDependSyncCall* pSyncCall // interrupt synchronisation object<nl>
    // <nl>
    CVampDeviceResources(
        IN COSDependObjectFactory *pFactory,
        IN tDeviceParams *pDevPar,
        IN COSDependRegistry *pRegistry,
	    IN COSDependSyncCall* pSyncCall );

    // @cmember Destructor.
	virtual ~CVampDeviceResources();

    //@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }
};

#endif // !defined(AFX_VAMPDEVICERESOURCES_H__2269FFA0_9299_11D3_A66F_00E01898355B__INCLUDED_)
