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

#include "BaseClass.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
CBaseClass::CBaseClass()
{
    m_pVampDevice = NULL;
    m_pVampDevResObj = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
CBaseClass::~CBaseClass()
{
    m_pVampDevice = NULL;
    m_pVampDevResObj = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function returns the CVampDeviceResources object. With this object
//  access to e.g. I2C, GPIO... is possible.
//
// Return Value:
//  CVampDeviceResources*   Pointer of the CVampDeviceResources object.
//  NULL                    Any error case.
//
//////////////////////////////////////////////////////////////////////////////
CVampDeviceResources* CBaseClass::GetDeviceResource
(
    IN PIRP pIrp   //Pointer to the IRP involved with this operation.
)
{
    if( m_pVampDevResObj )
    {
        return m_pVampDevResObj;
    }

    //parameter valid?
    if( !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return NULL;
    }

    CVampDevice* pVampDevice = GetVampDevice( pIrp );

    if( !pVampDevice )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get connection to device object"));
        return NULL;
    }

    //get the device resource object out of the VampDevice object
    CVampDeviceResources* pVampDevResObj =
        pVampDevice->GetDeviceResourceObject();
    if( !pVampDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return NULL;
    }

    m_pVampDevice = pVampDevice;
    m_pVampDevResObj = pVampDevResObj;

    return pVampDevResObj;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function returns the CVampDevice object.
//
// Return Value:
//  CVampDevice*   Pointer of the CVampDevice object.
//  NULL           Any error case.
//
//////////////////////////////////////////////////////////////////////////////
CVampDevice* CBaseClass::GetVampDevice
(
    IN PIRP pIrp   //Pointer to the IRP involved with this operation.
)
{
    if( m_pVampDevice )
    {
        return m_pVampDevice;
    }

    //Parameter valid?
    if( !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return NULL;
    }

    //get device resource object
    PKSFILTER pKsFilter = KsGetFilterFromIrp(pIrp);
    if( !pKsFilter )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get connection to KS filter"));
        return NULL;
    }
    //get the device object out of the filter object
    PKSDEVICE pKSDevice = KsGetDevice(pKsFilter);
    if( !pKSDevice )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get connection to KS device"));
        return NULL;
    }
    //get the VampDevice object out of the Context of device object
    CVampDevice* pVampDevice = static_cast <CVampDevice*>(pKSDevice->Context);
    if( !pVampDevice )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get connection to device object"));
        return NULL;
    }

    m_pVampDevice = pVampDevice;

    return pVampDevice;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function returns the CVampDeviceResources object. With this object
//  access to e.g. I2C, GPIO... is possible.
//
// Return Value:
//  CVampDeviceResources*   Pointer of the CVampDeviceResources object.
//  NULL                    Any error case.
//
//////////////////////////////////////////////////////////////////////////////
CVampDeviceResources* CBaseClass::GetDeviceResource
(
    IN PKSPIN pKSPin   //Pointer to the KSPin involved with this operation.
)
{
    if( m_pVampDevResObj )
    {
        return m_pVampDevResObj;
    }

    //parameters valid?
    if( !pKSPin )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return NULL;
    }

    // get the KS device object out of the KS pin object
    // (Device where the pin is part of)
    PKSDEVICE pKSDevice = KsGetDevice(pKSPin);
    if( !pKSDevice )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get connection to KS device"));
        return NULL;
    }
    //get the VampDevice object out of the Context of KSDevice
    CVampDevice* pVampDevice = static_cast <CVampDevice*> (pKSDevice->Context);
    if(!pVampDevice)
    {
        //no VampDevice object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get connection to device object"));
        return NULL;
    }
    //get the VampDeviceResource object out of VampDevice
    CVampDeviceResources* pVampDevResObj = pVampDevice->GetDeviceResourceObject();
    if( !pVampDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return NULL;
    }

    m_pVampDevice = pVampDevice;
    m_pVampDevResObj = pVampDevResObj;

    return pVampDevResObj;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function returns the CVampDeviceResources object. With this object
//  access to e.g. I2C, GPIO... is possible.
//
// Return Value:
//  CVampDeviceResources*   Pointer of the CVampDeviceResources object.
//  NULL                    Any error case.
//
//////////////////////////////////////////////////////////////////////////////
CVampDeviceResources* CBaseClass::GetDeviceResource
(
    IN PKSFILTER pKSFilter   //Pointer to the KSFilter involved with this operation.
)
{
    if( m_pVampDevResObj )
    {
        return m_pVampDevResObj;
    }

    //parameters valid?
    if(!pKSFilter)
    {
        _DbgPrintF( DEBUGLVL_ERROR,("Error: Invalid argument"));
        return NULL;
    }

    //get the VampDevice object out of the Context of device object
    CVampDevice* pVampDevice = GetVampDevice(pKSFilter);
    if(!pVampDevice)
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get connection to device object"));
        return NULL;
    }
    //get the device resource object out of the VampDevice object
    CVampDeviceResources* pVampDevResObj = pVampDevice->GetDeviceResourceObject();
    if( !pVampDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return NULL;
    }

    m_pVampDevice = pVampDevice;
    m_pVampDevResObj = pVampDevResObj;

    return pVampDevResObj;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function returns the CVampDevice object.
//
// Return Value:
//  CVampDevice*   Pointer of the CVampDevice object.
//  NULL           Any error case.
//
//////////////////////////////////////////////////////////////////////////////
CVampDevice* CBaseClass::GetVampDevice
(
    IN PKSFILTER pKSFilter   //Pointer to the KSFilter involved with this operation.
)
{
    if( m_pVampDevice )
    {
        return m_pVampDevice;
    }

    //parameters valid?
    if( !pKSFilter )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return NULL;
    }
    //get the device object out of the filter object
    PKSDEVICE pKSDevice = KsGetDevice(pKSFilter);
    if( !pKSDevice )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get connection to KS device"));
        return NULL;
    }
    //get the VampDevice object out of the Context of device object
    CVampDevice* pVampDevice = static_cast <CVampDevice*> (pKSDevice->Context);
    if( !pVampDevice )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get connection to device object"));
        return NULL;
    }

    m_pVampDevice = pVampDevice;

    return pVampDevice;
}
