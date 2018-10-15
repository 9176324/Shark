//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2001
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


#pragma once

#include "34AVStrm.h"
#include "VampDevice.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This is the base class for other classes. It implements
//  functionality that is common to every one.
//
//////////////////////////////////////////////////////////////////////////////
class CBaseClass
{
public:

    CBaseClass();
    virtual ~CBaseClass();

protected:

    CVampDeviceResources* GetDeviceResource(IN PIRP pIrp);

    CVampDeviceResources* GetDeviceResource(IN PKSPIN pKSPin);

    CVampDeviceResources* GetDeviceResource(IN PKSFILTER pKSFilter);

    CVampDevice* GetVampDevice(IN PIRP pIrp);

    CVampDevice* GetVampDevice(IN PKSFILTER pKSFilter);

private:

	CVampDevice* m_pVampDevice;
	CVampDeviceResources* m_pVampDevResObj;

};
