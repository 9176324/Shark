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


#pragma once

#include "ProxyKM.h"
#include "AVSyncCall.h"
#include "AVFactory.h"
//#include "VampI2CInterface.h"
//#include "VampGPIOInterface.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Describes the service structure which consists of the device
//  resource object and the deferred procedure call (DPC) object.
//
//////////////////////////////////////////////////////////////////////////////
typedef struct
{
    CVampDeviceResources* pDevResObj; //Pointer to the resources object
                                      //of the device.
    PRKDPC  pDPC; //Pointer to deferred procedure call structure.
} tServiceContext;

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This is the VampDevice class. It is responsible for the handling of
//  the Plug N Play and device creation messages from the system. As the
//  heart of the driver this class should be touched very carefully.
//
//////////////////////////////////////////////////////////////////////////////
class CVampDevice
{
public:
    CVampDevice();
    ~CVampDevice();

    NTSTATUS Start( IN          PKSDEVICE         pKSDevice,
                    IN          PIRP              pIrp,
                    IN OPTIONAL PCM_RESOURCE_LIST pTranslatedResourceList,
                    IN OPTIONAL PCM_RESOURCE_LIST pUntranslatedResourceList);

    void     Stop(  IN PKSDEVICE pKSDevice,
                    IN PIRP      pIrp);

    void     SetPower(
                    IN PKSDEVICE          pKSDevice,
                    IN PIRP               pIrp,
                    IN DEVICE_POWER_STATE To,
                    IN DEVICE_POWER_STATE From );

//    NTSTATUS QueryInterface(IN PKSDEVICE Device,
//                          IN PIRP Irp);

    tDeviceParams         GetInfoOfDevice();

    CVampDeviceResources* GetDeviceResourceObject();

    COSDependRegistry*    GetRegistryAccessObject();
    
    UINT GetDriverMediumInstanceCount();
    VOID SetDriverMediumInstanceCount(UINT);
    PDMA_ADAPTER GetDMAAdapter();

    PEPROCESS GetOwnerProcessHandle();
    NTSTATUS  SetOwnerProcessHandle(PEPROCESS phOwnerProcess);

private:
    NTSTATUS    IOCTL_DevicePCI(
                    IN  IOCTL_COMMAND* pCommandStructure,
                    IN  PDEVICE_OBJECT pPDO);

    NTSTATUS    IOCTL_DeviceMemory(
                    IN  IOCTL_COMMAND* pCommandStructure,
                    IN  PDEVICE_OBJECT pPDO);

    DWORD       GetConfigSpaceByOffset(
                    IN  PDEVICE_OBJECT pPDO,
                    IN  DWORD          dwOffset);

    NTSTATUS    GetConfigSpace(
                    IN  PDEVICE_OBJECT     pPDO,
                    OUT PPCI_COMMON_CONFIG pPCIConfig);

    void        SetConfigSpaceByOffset(
                    IN  PDEVICE_OBJECT pPDO,
                    IN  DWORD          dwOffset,
                    IN  DWORD          dwValue);

    // **** helper functions for splitting longer functions ****

    // Only called by "Start"
    NTSTATUS    InitializeResources(
                    IN  PKSDEVICE         pKSDevice,
                    IN  PCM_RESOURCE_LIST pTranslatedResourceList );

    // Only called by "InitializeResources"
    NTSTATUS    InitializeObjects(
                    IN  PKSDEVICE pKSDevice );

	// Only called by "InitializeObjects"
    NTSTATUS    UpdateVBIDataRanges(
					IN OUT PKSDEVICE pKSDevice,
                    IN DWORD dwVBISampleFreq );

    // Only called by "Start"
    NTSTATUS CreateFilterFactories(
        IN PKSDEVICE pKSDevice);

    // Only called by "CreateFilterFactories"
    NTSTATUS AllocateFilterDescriptorForMediums(
        IN KSFILTER_DESCRIPTOR FilterDescriptor,
        IN KSOBJECT_BAG ObjectBag,
        OUT PKSFILTER_DESCRIPTOR *pFilterDescriptor);

	NTSTATUS CleanUpObjects();	
	void CleanUpResources();	
	
	// **** end of helper functions ****


    PVOID              m_pBaseAddress;
    DWORD              m_dwMemoryLength;
    COSDependRegistry* m_pOSDependRegistryObj;
    CAVSyncCall*       m_pSyncObject;
    CVampFactory*      m_pFactoryObj;
    PKINTERRUPT        m_pInterruptObject;
    KDPC               m_Dpc;
    tServiceContext*   m_pServiceContext;
    DWORD m_dwDeviceIndex;
//    CVampI2CInterface*    m_pVampI2CInterface;
//    CVampGpioInterface*   m_pVampGpioInterface;
	//interrupt variables
    ULONG              m_ulVector;
    KIRQL              m_Irql;
    KIRQL              m_SynchronizeIrql;
    KINTERRUPT_MODE    m_InterruptMode;
    KAFFINITY          m_ProcessorEnableMask;
    BOOLEAN            m_bShareVector;
	// flag to indicate that a device was added successfully
	BOOLEAN            m_bDeviceWasAdded;
    UINT               m_uiDriverMediumInstanceCount;
    // one owner process sufficient for a single tuner device
    PEPROCESS          m_phOwnerProcess;
    KSPIN_LOCK         m_ProcessSpinLock;
    PDMA_ADAPTER       m_pDMAAdapter;
};
