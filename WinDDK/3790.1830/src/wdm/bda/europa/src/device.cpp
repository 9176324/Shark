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


#include "34AVStrm.h"
#include "Device.h"
#include "VampDevice.h"

// A global used by this driver to keep track of the instance count of the number 
// of times the driver is loaded.  This is used to create unique Mediums so that
// the correct capture, crossbar, tuner, and tvaudio filters all get connected together.

UINT g_uiDriverMediumInstanceCount = 0;

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Sets up the driver object. This is the starting point for the driver.
//  If the system loads the driver, this function is called.
//
// Return Value:
//  STATUS_SUCCESS     The driver was initialized successfully.
//  NTSTATUS - value   Error value returned by "KsInitializeDriver" function.
//
//////////////////////////////////////////////////////////////////////////////
extern "C" NTSTATUS DriverEntry
(
    IN PDRIVER_OBJECT  pDriverObject,       //Driver object for this instance.
    IN PUNICODE_STRING pRegistryPathName    //Contains the registry path which
                                            //was used to load this instance.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: DriverEntry"));

    //parameters valid?
    if( !pDriverObject || !pRegistryPathName )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS ntStatus = KsInitializeDriver(
                                            pDriverObject,
                                            pRegistryPathName,
                                            &DeviceDescriptor );

    if( ntStatus != STATUS_SUCCESS )
    {
        //STATUS_INSUFFICIENT_RESOURCES
        //STATUS_OBJECT_NAME_EXISTS
        //STATUS_OBJECT_NAME_COLLISION
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: KSInitializeDriver not successful (%u)", ntStatus));
        return ntStatus;
    }

    return ntStatus;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function creates a new (vamp-)device (class-)object which supports
//  necessary information and functionality we need to control our hardware.
//  The new object is stored to the KS device object for later use.
//
// Return Value:
//  STATUS_SUCCESS       New device object created and stored to "PKSDEVICE"
//  STATUS_UNSUCCESSFUL  Any error case.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS Add
(
    IN PKSDEVICE pKSDevice //Pointer to the device object
                           //provided by the system.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: Add device called"));

    if( !pKSDevice )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    //allocate memory for our VampDevice class
    CVampDevice* pVampDevice = new (NON_PAGED_POOL) CVampDevice();
    if( !pVampDevice )
    {
        //memory allocation failed
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get connection to device object"));
        return STATUS_UNSUCCESSFUL;
    }

    pVampDevice->SetDriverMediumInstanceCount(g_uiDriverMediumInstanceCount);
    g_uiDriverMediumInstanceCount++;

    //store the VampDevice object for later use
    //in the Context of KSDevice
    pKSDevice->Context = pVampDevice;
    //dispatch the add device function call to our VampDevice class
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function starts the device. Therefore it calls the class function
//  of the VamDevice which is given within the KS device context.
//
// Return Value:
//  STATUS_SUCCESS        Device initialized successfully.
//  STATUS_UNSUCCESSFUL   Any error case.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS Start
(
    IN          PKSDEVICE         pKSDevice, //Pointer to the device object
                                             //provided by the system.
    IN          PIRP              pIrp,      //Currently processed IRP.
    IN OPTIONAL PCM_RESOURCE_LIST pTranslatedResourceList, //List of
                                                           //translated
                                                           //HW resources to
                                                           //this device.
    IN OPTIONAL PCM_RESOURCE_LIST pUntranslatedResourceList//List of
                                                           //untranslated
                                                           //HW resources to
                                                           //this device.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: Start device called"));

    if( !pKSDevice || !pIrp ||
        !pTranslatedResourceList || !pUntranslatedResourceList )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the VampDevice object out of
    //the Context of KSDevice
    CVampDevice* pCVampDevice =
        static_cast <CVampDevice*>(pKSDevice->Context);
    if( !pCVampDevice )
    {
        //no VampDevice object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get connection to device object"));
        return STATUS_UNSUCCESSFUL;
    }
    //dispatch the start device function call to our VampDevice class
    return pCVampDevice->Start(pKSDevice,
                               pIrp,
                               pTranslatedResourceList,
                               pUntranslatedResourceList);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function stops the device and frees/deletes all structures/objects.
//  Therefore it calls the class function of the VamDevice which is given
//  within the KS device context.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
void Stop
(
    IN PKSDEVICE pKSDevice,   //Pointer to the device object
                              //provided by the system.
    IN PIRP      pIrp         //Currently processed IRP.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: Stop device called"));

    if( !pKSDevice || !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return;
    }
    //get the VampDevice object out of
    //the Context of KSDevice
    CVampDevice* pVampDevice = static_cast <CVampDevice*>(pKSDevice->Context);
    if( !pVampDevice )
    {
        //no VampDevice object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get connection to device object"));
        return;
    }
    //dispatch the Stop device function call to our VampDevice class
    pVampDevice->Stop(pKSDevice, pIrp);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function removes the (vamp-)device (class-)object out of the KS
//  device object.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
void Remove
(
    IN PKSDEVICE pKSDevice,   //Pointer to the device object
                              //provided by the system.
    IN PIRP      pIrp         //Pointer to the IRP related to this request.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: Remove device called"));

    if( !pKSDevice || !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return;
    }
    //get the VampDevice object out of
    //the Context of KSDevice
    CVampDevice* pVampDevice =
        static_cast <CVampDevice*> (pKSDevice->Context);
    if( !pVampDevice )
    {
        //no VampDevice object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get connection to device object"));
        return;
    }
    //de-allocate memory for our VampDevice class
    delete pVampDevice;
    //remove the VampDevice object from the
    //context structure of KSDevice
    pKSDevice->Context = NULL;

    return;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function is called if we have to enter a new power state.
//  Therefore it calls the class function of the VamDevice which is given
//  within the KS device context.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
void SetPower
(
    IN PKSDEVICE          pKSDevice, //Pointer to the device object
                                     //provided by the system.
    IN PIRP               pIrp,//Pointer to the IRP related to this request.
    IN DEVICE_POWER_STATE To,  //Requested power state.
    IN DEVICE_POWER_STATE From //Current power state.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: SetPower called"));

    if( !pKSDevice || !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return;
    }
    //get the VampDevice object out of
    //the Context of KSDevice
    CVampDevice* pVampDevice =
        static_cast <CVampDevice*> (pKSDevice->Context);
    if( !pVampDevice )
    {
        //no VampDevice object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get connection to device object"));
        return;
    }
    //dispatch the Stop device function call to our VampDevice class
    pVampDevice->SetPower(pKSDevice, pIrp, To, From);
}

//NTSTATUS QueryInterface
//(
//  IN PKSDEVICE pKSDevice,
//  IN PIRP pIrp
//)
//{
//    _DbgPrintF(DEBUGLVL_BLAB,(""));
//    _DbgPrintF(DEBUGLVL_BLAB,("Info: QueryInterface called"));
//
//    if( !pKSDevice || !pIrp )
//    {
//      _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
//      return STATUS_UNSUCCESSFUL;
//    }
//    //get the VampDevice object out of
//    //the Context of KSDevice
//    CVampDevice* pVampDevice =
//        static_cast <CVampDevice*>(pKSDevice->Context);
//    if( !pVampDevice )
//    {
//      //no VampDevice object available
//      _DbgPrintF( DEBUGLVL_ERROR,
//                  ("Error: Cannot get connection to device object"));
//      return STATUS_UNSUCCESSFUL;
//    }
//    //dispatch the QueryInterface function call to our VampDevice class
//    return pVampDevice->QueryInterface(pKSDevice, pIrp);
//}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function checks if the interrupt was raised by the Vamp hardware
//  and scedules a DPC in that case. Otherwise false is returned and the
//  system calls the next hardware that is registered for this interrupt
//  ("shared interrupts").
//
// Return Value:
//  TRUE   Our device is responsible for this IRQ.
//  FALSE  Our device is not responsible for this IRQ or an error occurred.
//
//////////////////////////////////////////////////////////////////////////////
BOOLEAN InterruptService
(
    PKINTERRUPT pInterruptObject, //Pointer to the interrupt object (unused).
    PVOID       pServiceContext   //Pointer to interrupt request information.
)
{
    // _DbgPrintF(DEBUGLVL_BLAB,(""));
    // _DbgPrintF(DEBUGLVL_BLAB,("Info: Interrupt service called"));

    if( !pServiceContext )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Invalid argument (IRQ)"));
        return FALSE;
    }
    // get device resource object from service context
    CVampDeviceResources* pDevResObj =
        (static_cast <tServiceContext*> (pServiceContext))->pDevResObj;
    if( !pDevResObj )
    {
        // this is hopefully not our interrupt,
        // we do not even have a device object!
        _DbgPrintF(DEBUGLVL_ERROR,("Error: No Device Resource object (IRQ)"));
        return FALSE;
    }
    // get device resource object from service context
    PRKDPC pDPC = (static_cast <tServiceContext*> (pServiceContext))->pDPC;
    if( !pDPC )
    {
        // this is hopefully not our interrupt, I cannot find a service context
        _DbgPrintF(DEBUGLVL_ERROR,("Error: No service context (IRQ)"));
        return FALSE;
    }
    // call HAL interrupt routine and see if it was our interrupt
    VAMPRET vrStatus = pDevResObj->m_pStreamManager->OnInterrupt();
    if( vrStatus == VAMPRET_UNKNOWN_INTERRUPT )
    {
        // signal the system, that some other device is responsible for the IRQ
        // _DbgPrintF(DEBUGLVL_BLAB,("Info: Not our interrupt (IRQ)"));
        return FALSE;
    }
    // it was our interrupt, so check whether a DPC is requested or not
    switch(vrStatus)
    {
    case VAMPRET_DPC_REQUESTED:
    {
        // insert DPC into queue
        PVOID  SystemArgument1 =
            static_cast <PVOID>(pDevResObj->m_pStreamManager);
        PVOID  SystemArgument2 = NULL;
        BOOL bReturn = KeInsertQueueDpc(pDPC,
                                           SystemArgument1,
                                           SystemArgument2);
        if( bReturn == FALSE )
        {
            // DPC is already in the queue, so do nothing special here
            // the warning is uncommented because it occurs when system
            // performance is low and increases system load heavily
            // _DbgPrintF(DEBUGLVL_VERBOSE,
            //  ("Warning: DPC already in the queue (IRQ)"));
        }
        break;
    }
    case VAMPRET_SUCCESS:
        // DPC is not requested, so do nothing special here
        // the warning is uncommented because it occurs when system
        // performance is low and increases system load heavily
        // _DbgPrintF(DEBUGLVL_VERBOSE,("Warning: No DPC requested (IRQ)"));
        break;
    default:
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: OnInterrupt failed with %x (IRQ)", vrStatus));
        break;
    }
    // signal the system, that our device is responsible for the IRQ
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is our function for deferred procedure calls (DPC's). It calls
//  method which checks all event notification flags including
//  outstanding DMA transfer notifications. HAL access is performed in
//  this function.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
void DPCService
(
    IN PKDPC pDpc, //Pointer to the DPC object(unused).
    IN PVOID pDeferredContext, //Pointer to context of DPC(unused).
    IN PVOID pSystemArgument1, //Pointer to the Vamp stream manager (HAL)
                               //given during interrupt service call.
    IN PVOID pSystemArgument2  //Pointer to additional information(unused).
)
{
    //_DbgPrintF(DEBUGLVL_BLAB,(""));
    //_DbgPrintF(DEBUGLVL_BLAB,("Info: DPC service called"));

    //Parameter valid?
    if( !pSystemArgument1 )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument (DPC)"));
        return;
    }
     //call our internal DPC routine
    VAMPRET vrSuccess =
        (static_cast <CVampStreamManager*>(pSystemArgument1))->OnDPC();
    if( vrSuccess != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: OnDPC failed (DPC)"));
        return;
    }
    return;
}


//*** function implementation for private use ***//

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function is used to get the device resources of the Vamp device
//  out of a KS object.
//  HAL access is performed by this function.
//
// Return Value:
//  CVampDeviceResources*   Pointer to the Vamp device resource object.
//  NULL                    Any error case.
//
//////////////////////////////////////////////////////////////////////////////
CVampDeviceResources* GetVampDevResObj
(
    IN  PVOID pKSObject //Pointer to KS object structure.
)
{
    //invalid parameters?
    if( !pKSObject )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return NULL;
    }

    // get the decoder out of the filter object
    PKSDEVICE pKSDevice = KsGetDevice(pKSObject);
    if( !pKSDevice )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Cannot get connection to KS device"));
        return NULL;
    }
    //get the VampDevice object out of the Context of device object
    CVampDevice* pVampDevice = static_cast <CVampDevice*> (pKSDevice->Context);
    if( !pVampDevice )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
                ("Error: Cannot get connection to device object"));
        return NULL;
    }
    //get the device resource object out of the VampDevice object
    CVampDeviceResources* pVampDevResObj =
        pVampDevice->GetDeviceResourceObject();
    if( !pVampDevResObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Cannot get device resource object"));
        return NULL;
    }

    return pVampDevResObj;
}

