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
#include "VampDevice.h"
#include "Device.h"
#include "SynchIrpCall.h"

//
// Description:
//  The proxy handles device instances.
//  (accessable via a single proxy, that refers to the instances)
//  It will be created with adding the first device and
//  destroyed if all devices are deleted.
//
CProxyKM* g_pKMProxy = NULL;       

// 
// Description:
// Global variable to track multiple hardware instances. This is used to create
// unique Medium Id's so that the Tuner, TV Audio, XBar and Capture Filters connect
// up correctly when there are multiple device instances. Updated on DeviceAdd. 
//
extern UINT g_uiDriverMediumInstanceCount;

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor
//
//////////////////////////////////////////////////////////////////////////////
CVampDevice::CVampDevice()
{
    //initialize member variables
    m_pBaseAddress         = NULL;
    m_dwMemoryLength       = 0;
    m_dwDeviceIndex        = 0;
    m_pOSDependRegistryObj = NULL;
    m_pSyncObject          = NULL;
    m_pFactoryObj          = NULL;
    m_pInterruptObject     = NULL;
    m_pServiceContext      = NULL;
    memset(&m_Dpc,  0, sizeof(m_Dpc));
//    m_pVampI2CInterface    = NULL;
//    m_pVampGpioInterface   = NULL;
   //interrupt variables
    m_ulVector  = 0;
    m_Irql      = 0;
    m_SynchronizeIrql     = 0;
    m_InterruptMode       = Latched;
    m_ProcessorEnableMask = 0;
    m_bShareVector        = TRUE;

    m_bDeviceWasAdded     = FALSE;
    m_phOwnerProcess    = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor
//
//////////////////////////////////////////////////////////////////////////////
CVampDevice::~CVampDevice()
{
    m_pBaseAddress         = NULL;
    m_dwMemoryLength       = 0;
    m_dwDeviceIndex        = 0;
    m_pOSDependRegistryObj = NULL;
    m_pSyncObject          = NULL;
    m_pFactoryObj          = NULL;
    m_pInterruptObject     = NULL;
    m_pServiceContext      = NULL;
    memset(&m_Dpc,  0, sizeof(m_Dpc));
//    m_pVampI2CInterface    = NULL;
//    m_pVampGpioInterface   = NULL;
    m_ulVector  = 0;
    m_Irql      = 0;
    m_SynchronizeIrql     = 0;
    m_InterruptMode       = Latched;
    m_ProcessorEnableMask = 0;
    m_bShareVector        = FALSE;

    m_bDeviceWasAdded     = FALSE;
}

NTSTATUS
AllocatePrivateMemory(
    PDMA_ADAPTER pAdapter,
    CVampDeviceResources *pDevRes)
{
    PVOID pVirtualAddress, pAlignedVirtualAddress;
    PHYSICAL_ADDRESS PhysicalAddress;
    DWORD dwOffset = 0;

    RtlZeroMemory(pDevRes->m_p32BitPagePool, sizeof(pDevRes->m_p32BitPagePool));

    // allocate two aligned pages for every DMA channel
    for (int i = 0; i < 2 * NUM_DMA_CHANNELS; i++)
    {
        pVirtualAddress = pAdapter->DmaOperations->AllocateCommonBuffer(pAdapter, 2 * ONE_PAGE, &PhysicalAddress, TRUE);
        if (!pVirtualAddress)
        {
            if (i > 0) 
                for (int j = i - 1;  j >=0; j--)
                {
                    PhysicalAddress.HighPart = 0;
                    PhysicalAddress.LowPart = pDevRes->m_p32BitPagePool[j].dwPhysicalAddress;
                    pAdapter->DmaOperations->FreeCommonBuffer(pAdapter, 2 * ONE_PAGE, 
                        PhysicalAddress, pDevRes->m_p32BitPagePool[j].pVirtualAddress, TRUE);
                }
            RtlZeroMemory(pDevRes->m_p32BitPagePool, sizeof(pDevRes->m_p32BitPagePool));
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        ASSERT(PhysicalAddress.HighPart == 0);
        pDevRes->m_p32BitPagePool[i].pVirtualAddress = pVirtualAddress;
        pDevRes->m_p32BitPagePool[i].dwPhysicalAddress = PhysicalAddress.LowPart;
        pDevRes->m_p32BitPagePool[i].pAlignedVirtualAddress = pAlignedVirtualAddress = ALIGN_PAGE(pVirtualAddress);
        dwOffset = (DWORD) ((ULONG_PTR)pAlignedVirtualAddress - (ULONG_PTR)pVirtualAddress);
        pDevRes->m_p32BitPagePool[i].dwAlignedPhysicalAddress = PhysicalAddress.LowPart + dwOffset;

    }
    return STATUS_SUCCESS;
}

NTSTATUS
ReleasePrivateMemory(
    PDMA_ADAPTER pAdapter,
    CVampDeviceResources *pDevRes)
{
    PHYSICAL_ADDRESS PhysicalAddress;
    if (!pDevRes->m_p32BitPagePool[0].pVirtualAddress) return STATUS_SUCCESS;

    for (int i = 0; i < 2 * NUM_DMA_CHANNELS; i++)
    {
        PhysicalAddress.HighPart = 0;
        PhysicalAddress.LowPart = pDevRes->m_p32BitPagePool[i].dwPhysicalAddress;
        pAdapter->DmaOperations->FreeCommonBuffer(pAdapter, 2 * ONE_PAGE, 
            PhysicalAddress, 
            pDevRes->m_p32BitPagePool[i].pVirtualAddress, TRUE);
    }
    RtlZeroMemory(pDevRes->m_p32BitPagePool, sizeof(pDevRes->m_p32BitPagePool));
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function initializes and creates all necessary structures/objects.
//  It initializes IO memory, interrupts and creates objects e.g. registry
//  object, GPIO, I2C... and connects filters to the device.
//  This function uses additional functions because it becomes too large
//  otherwise.
//  All operations are executed only if all previous operations
//  were successfully. In any error case this function is responsible to
//  free/delete all created objects/memory... also for objects created
//  by sub-functions.
//
// Return Value:
//  STATUS_SUCCESS        All objects and structures created successfully.
//  STATUS_UNSUCCESSFUL   Any error case
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CVampDevice::Start
(
    IN          PKSDEVICE         pKSDevice,//Pointer to the KS device object
                                            //provided by the system.
    IN          PIRP              pIrp, //Pointer to IRP for this request.
                                        //(unused)
    IN OPTIONAL PCM_RESOURCE_LIST pTranslatedResourceList, //List of
                               //translated HW resources to the device.
    IN OPTIONAL PCM_RESOURCE_LIST pUntranslatedResourceList //List of
                               //untranslated HW resources to the device (unused).
)
{
    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

    // input parameters valid?
    if( !pTranslatedResourceList || !pKSDevice)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: No device resources assigned"));
        return STATUS_UNSUCCESSFUL;
    }
    // reset flag
    m_bDeviceWasAdded = FALSE;

    // this functionality was separated to reduce the size of this function
    // it setup's interrupt connection and memory IO space
    ntStatus = InitializeResources( pKSDevice, pTranslatedResourceList );
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: InitializeResources failed"));
        // cleanup for InitializeResources already done internally
        return STATUS_UNSUCCESSFUL;
    }

    // this functionality was separated to reduce the size of this function
    // it initializes all objects of the device and connects it to the filter
    ntStatus = InitializeObjects( pKSDevice );
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: InitializeObjects failed"));
        // cleanup for InitializeObjects already done internally,
        // do cleanup for InitializeResources,
        // function has no return value, so we assume that it never fails
        CleanUpResources();
        return STATUS_UNSUCCESSFUL;
    }
    // check if members are initialized correctly
    if ( !m_pServiceContext || !g_pKMProxy || !m_pSyncObject )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: No service context available"));
        // do cleanup for InitializeObjects
        if( CleanUpObjects() != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Clean up objects failed"));
        }
        // do cleanup for InitializeResources,
        // function has no return value, so we assume that it never fails
        CleanUpResources();
        return STATUS_UNSUCCESSFUL;
    }
    //set the device resource object in the interrupt context structure
    m_pServiceContext->pDevResObj =
        g_pKMProxy->GetDeviceResourceObject(m_dwDeviceIndex);

    //initialize DPC object
    KeInitializeDpc( &m_Dpc,
                     DPCService,
                     static_cast <PVOID> (m_pServiceContext) );

    //set the DPC object in the interrupt context structure
    m_pServiceContext->pDPC = &m_Dpc;

    //connect interrupt to ISR
    //and generate interrupt object for later use
    ntStatus = IoConnectInterrupt(&m_pInterruptObject,
                        InterruptService,
                        m_pServiceContext,  // all these members were filled
                        NULL,               // in InitializeResource
                        m_ulVector,
                        m_Irql,
                        m_SynchronizeIrql,
                        m_InterruptMode,
                        m_bShareVector,
                        m_ProcessorEnableMask,
                        FALSE);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: IoConnectInterrupt not successful"));
        // do cleanup for InitializeObjects
        if( CleanUpObjects() != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Clean up objects failed"));
        }
        // do cleanup for InitializeResources,
        // function has no return value, so we assume that it never fails
        CleanUpResources();
        return STATUS_UNSUCCESSFUL;
    }
    // insert the IRQ object which was actualized by IoConnectInterrupt
    // function has no return value, so we assume that it never fails
    m_pSyncObject->InsertIRQObject(m_pInterruptObject);
    //create DMA adapter
    DEVICE_DESCRIPTION  DeviceDescription;
    memset(&DeviceDescription, 0, sizeof(DeviceDescription));

    DeviceDescription.Version           = DEVICE_DESCRIPTION_VERSION;
    DeviceDescription.Master            = TRUE;
    DeviceDescription.ScatterGather     = TRUE;
    DeviceDescription.Dma32BitAddresses = TRUE;
    DeviceDescription.Dma64BitAddresses = FALSE;
    DeviceDescription.DmaChannel        = ((ULONG) ~0);
    DeviceDescription.InterfaceType     = PCIBus;
    DeviceDescription.MaximumLength     = 0xfffffff8;

    //not used
    DeviceDescription.IgnoreCount;    
    DeviceDescription.DemandMode;     
    DeviceDescription.AutoInitialize; 
    DeviceDescription.DmaWidth;       
    DeviceDescription.Reserved1;      
    DeviceDescription.DoNotUse2;      
    DeviceDescription.DmaSpeed;       
    DeviceDescription.DmaPort;        

    DWORD dwNumRegisters = 0;

    m_pDMAAdapter = IoGetDmaAdapter(pKSDevice->PhysicalDeviceObject,
                                               &DeviceDescription,
                                               &dwNumRegisters);
    if( !m_pDMAAdapter )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: IoGetDmaAdapter failed"));
        // do cleanup for InitializeObjects
        if( CleanUpObjects() != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Clean up objects failed"));
        }
        // do cleanup for InitializeResources,
        // function has no return value, so we assume that it never fails
        CleanUpResources();
        return STATUS_UNSUCCESSFUL;
    }
    if( !m_pDMAAdapter->DmaOperations )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: DmaOperations invalid"));
        return STATUS_UNSUCCESSFUL;
    }
    KsDeviceRegisterAdapterObject(pKSDevice,
                                  m_pDMAAdapter,
                                  0xfffffff8,
                                  sizeof(KSMAPPING));

    ntStatus = AllocatePrivateMemory(m_pDMAAdapter, m_pServiceContext->pDevResObj);

    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = CreateFilterFactories(pKSDevice);
        if (!NT_SUCCESS(ntStatus))
        {
            ReleasePrivateMemory(m_pDMAAdapter, m_pServiceContext->pDevResObj);
        }
    }

    if (!NT_SUCCESS(ntStatus))
    {
        // remove IRQ object from sync object
        if( m_pSyncObject )
        {
            // function has no return value, so we assume that it never
            // fails
            m_pSyncObject->RemoveIRQObject();
        }
        // disconnect interrupt service routine
        if( m_pInterruptObject )
        {
            // function has no return value, so we assume that it never
            // fails
            IoDisconnectInterrupt(m_pInterruptObject);
            m_pInterruptObject = NULL;
        }
        // do cleanup for InitializeObjects
        if( CleanUpObjects() != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Clean up objects failed"));
        }
        // do cleanup for InitializeResources,
        // function has no return value, so we assume that it never fails
        CleanUpResources();
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

//*** private helper functions for start device function  ***//

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function initializes interrupt connections and memory IO ranges.
//  In any error case the caller function is responsible to clear resources!
//
// Return Value:
//  STATUS_SUCCESS       Operation was successfully.
//  STATUS_UNSUCCESSFUL  Any error case.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CVampDevice::InitializeResources
(
    IN PKSDEVICE         pKSDevice,//Pointer to the KS device object
                                   //provided by the system.
    IN PCM_RESOURCE_LIST pTranslatedResourceList //List of translated HW
                                                 //resources to the device.
)
{
    PCM_PARTIAL_RESOURCE_LIST pList =
        &pTranslatedResourceList->List[0].PartialResourceList;
    if( !pList )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: No PCM_PARTIAL_RESOURCE_LIST"));
        return STATUS_UNSUCCESSFUL;
    }
    DWORD dwNumResources = pList->Count;

    PCM_PARTIAL_RESOURCE_DESCRIPTOR pResource =
        pList->PartialDescriptors;

    // scan all resources for memory mapped io memory range
    for ( DWORD dwIndex = 0; dwIndex < dwNumResources; dwIndex++, pResource++ )
    {
        switch( pResource->Type )
        {
        case CmResourceTypeMemory:
            //map register io space into virtual memory
            m_pBaseAddress = MmMapIoSpace(
                pResource->u.Memory.Start,
                pResource->u.Memory.Length,
                MmNonCached);
            if( !m_pBaseAddress )
            {
                _DbgPrintF(DEBUGLVL_ERROR,
                    ("Error: Base address cannot be mapped"));
                m_dwMemoryLength = 0;
                return STATUS_UNSUCCESSFUL;
            }
            //store memory length for later use
            //(deallocation in Stop function)
            m_dwMemoryLength = pResource->u.Memory.Length;
            break;
        case CmResourceTypeInterrupt:
            //prepare interrupt connection for later use
            m_ulVector = pResource->u.Interrupt.Vector;
            m_Irql = static_cast <KIRQL>(pResource->u.Interrupt.Level);
            m_SynchronizeIrql = static_cast <KIRQL>(pResource->u.Interrupt.Level);
            switch(pResource->Flags)
            {
            case CM_RESOURCE_INTERRUPT_LATCHED:
                m_InterruptMode = Latched;
                break;
            case CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE:
                m_InterruptMode = LevelSensitive;
                break;
            default:
                _DbgPrintF(DEBUGLVL_ERROR,
                    ("Error: InterruptMode unsupported"));
                //de-allocate memory for the register io space
                if( m_pBaseAddress )
                {
                    // oponent (MmMapIoSpace) called in InitializeResources
                    // function has no return value, so we assume that it
                    // never fails
                    MmUnmapIoSpace(m_pBaseAddress, m_dwMemoryLength);
                    m_pBaseAddress   = NULL;
                    m_dwMemoryLength = 0;
                }
                return STATUS_UNSUCCESSFUL;
            }
            m_ProcessorEnableMask = pResource->u.Interrupt.Affinity;
            m_bShareVector =
                (pResource->ShareDisposition == static_cast <UCHAR> (CmResourceShareShared));
            break;
        default:
            _DbgPrintF(DEBUGLVL_VERBOSE,
                ("Warning: pResource->Type unsupported"));
            break;
        }
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function is called by "Start" to create and initializes objects
//  neede to use the device. Also it creates the connection between the
//  filter and the device.
//  In any error case the caller function is responsible to clear resources!
//
// Return Value:
//  STATUS_SUCCESS       Objects created successfully.
//  STATUS_UNSUCCESSFUL  Any error case.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CVampDevice::InitializeObjects
(
    IN PKSDEVICE pKSDevice //Pointer to the KS device object
                           //provided by the system.
)
{
    //*** get the device params from the system ***

    PCI_COMMON_CONFIG tPCIInfoOfDevice;
    NTSTATUS          status = STATUS_UNSUCCESSFUL;
    status = GetConfigSpace(pKSDevice->FunctionalDeviceObject, &tPCIInfoOfDevice);
    if( !NT_SUCCESS(status) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: GetConfigSpace failed"));
        return STATUS_UNSUCCESSFUL;
    }
    tDeviceParams tInfoOfDevice;
    //copy base address from PCI configuration space
    tInfoOfDevice.pBaseAddress =
        static_cast <PBYTE>(m_pBaseAddress);
    //copy device node, FDO is stored for IOCTL PCI function
    tInfoOfDevice.ulPtrDeviceNode =
        reinterpret_cast <ULONG_PTR> (pKSDevice->FunctionalDeviceObject);
    //copy device ID (0x7134) from PCI configuration space
    tInfoOfDevice.dwDeviceId =
        tPCIInfoOfDevice.DeviceID;
    //copy vendor ID (0x1131) from PCI configuration space
    tInfoOfDevice.dwVendorId =
        tPCIInfoOfDevice.VendorID;
    //copy RevisionID from PCI configuration space
    tInfoOfDevice.dwDeviceRevision =
        tPCIInfoOfDevice.RevisionID;
    //copy SubsystemID from PCI configuration space
    tInfoOfDevice.dwSubsystemId =
        tPCIInfoOfDevice.u.type0.SubSystemID;
    //copy SubvendorID from PCI configuration space
    tInfoOfDevice.dwSubvendorId =
        tPCIInfoOfDevice.u.type0.SubVendorID;
    //set device friendly name to default value
    tInfoOfDevice.pszFriendlyName[0] =
        '\0';

    //create an OSDependRegistry object
    m_pOSDependRegistryObj =
        new (NON_PAGED_POOL) COSDependRegistry(
                                static_cast <PVOID> (pKSDevice->PhysicalDeviceObject));
    if( !m_pOSDependRegistryObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Cannot create registry object"));
        return STATUS_UNSUCCESSFUL;
    }

    //*** VBI SampleRate Update ***//
    DWORD dwVBISampleFreq = 0;
    // now we have access to the registry, so we will check whether we can
    // find a frequency which the user wants to have
    status = m_pOSDependRegistryObj->ReadRegistry(
                   static_cast <PCHAR> ("VBI Sampling Frequency"),
                   &dwVBISampleFreq,
                   static_cast <PCHAR> ("Decoder"),
                   KS_VBISAMPLINGRATE_47X_NABTS );
    if( status != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_BLAB,
            ("Info: No special VBI sample rate defined, use standard 27MHz"));
    }
    // if we find something != 27MHz (default) we have to change the NTSC VBI
    // data ranges
    switch( dwVBISampleFreq )
    {
        case KS_VBISAMPLINGRATE_47X_NABTS:
            // standard sample rate is selected, so do nothing
            _DbgPrintF(DEBUGLVL_BLAB,("Info: VBI 27MHz"));
            break;
        case KS_VBISAMPLINGRATE_5X_NABTS:
            // special (old standard) sample rate is selected
            _DbgPrintF(DEBUGLVL_BLAB,("Info: VBI 28.636360MHz"));
            status = UpdateVBIDataRanges( pKSDevice, dwVBISampleFreq );
            if( status != VAMPRET_SUCCESS )
            {
                _DbgPrintF(DEBUGLVL_ERROR,
                    ("Error: UpdateVBIDataRanges failed"));
                if( CleanUpObjects() != STATUS_SUCCESS )
                {
                    _DbgPrintF(DEBUGLVL_ERROR,("Error: Clean up failed"));
                }
                return STATUS_UNSUCCESSFUL;
            }
            break;
        default:
            _DbgPrintF(DEBUGLVL_ERROR,
                ("Error: VBI %uMHz NOT supported", dwVBISampleFreq));
            break;
    }
    // create an interrupt serialization object from the interrupt object
    // remember it, so we can destroy it when we shutdown
    m_pSyncObject = new (NON_PAGED_POOL) CAVSyncCall(m_pInterruptObject);
    if( !m_pSyncObject )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Cannot create sync object"));
        if( CleanUpObjects() != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Clean up failed"));
        }
        return STATUS_UNSUCCESSFUL;
    }
    //create Factory object
    m_pFactoryObj = new (NON_PAGED_POOL) CVampFactory;
    if( !m_pFactoryObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Cannot create factory object"));
        if( CleanUpObjects() != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Clean up failed"));
        }
        return STATUS_UNSUCCESSFUL;
    }

    KeInitializeSpinLock(&m_ProcessSpinLock);

    //initialize proxy
    if( !g_pKMProxy )
    {
        //first time that we are called,
        //so allocate memory for the proxy
        g_pKMProxy = new (NON_PAGED_POOL) CProxyKM();
        //allocation successful?
        if( !g_pKMProxy )
        {
            _DbgPrintF(DEBUGLVL_ERROR,
                       ("Error: Cannot create KMProxy object"));
            if( CleanUpObjects() != STATUS_SUCCESS )
            {
                _DbgPrintF(DEBUGLVL_ERROR,("Error: Clean up failed"));
            }
            return STATUS_UNSUCCESSFUL;
        }
    }
    //tell proxy that a new device arrived
    if( VAMPRET_SUCCESS != g_pKMProxy->AddDevice(
                                        &m_dwDeviceIndex,
                                        &tInfoOfDevice,
                                        m_pFactoryObj,
                                        m_pOSDependRegistryObj,
                                        m_pSyncObject) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: AddDevice failed"));
        if( CleanUpObjects() != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Clean up failed"));
        }
        return STATUS_UNSUCCESSFUL;
    }
    // signal that AddDevice succeeded
    m_bDeviceWasAdded = TRUE;

    m_pServiceContext = new (NON_PAGED_POOL) tServiceContext;
    if( !m_pServiceContext )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
                   ("Error: Not enough memory for m_pServiceContext"));
        if( CleanUpObjects() != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Clean up failed"));
        }
        return STATUS_UNSUCCESSFUL;
    }

    // Initialize XBar inputs. This is done to support legacy apps 
    // using the WaveIn Audio filter which dont initialize the XBar

    //get the device resource object out of the Irp
    CVampDeviceResources* pVampDevResObj = GetDeviceResourceObject();
    VAMPRET vSuccess;

    if( !pVampDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        if( CleanUpObjects() != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Clean up failed"));
        }
        return STATUS_UNSUCCESSFUL;
    }

    if (!pVampDevResObj->m_pDecoder)
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error: Decoder Control is NULL"));
        if( CleanUpObjects() != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Clean up failed"));
        }
        return STATUS_UNSUCCESSFUL;
    }
    
    // Set the XBar Video Input
    vSuccess = pVampDevResObj->m_pDecoder->SetVideoSource(S_VIDEO_1);
    if( vSuccess != VAMPRET_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
            ("Error: Failed to set video source"));
        if( CleanUpObjects() != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Clean up failed"));
        }

        return STATUS_UNSUCCESSFUL;
    }

    if (!pVampDevResObj->m_pAudioCtrl)
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error: Audio Control is NULL"));
        if( CleanUpObjects() != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Clean up failed"));
        }
        return STATUS_UNSUCCESSFUL;
    }

    // Set the XBar Audio Input
    vSuccess = pVampDevResObj->m_pAudioCtrl->SetStreamPath(IN_AUDIO_S_VIDEO_1);
    if( vSuccess != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Failed to set audio stream source"));
        if( CleanUpObjects() != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Clean up failed"));
        }

        return STATUS_UNSUCCESSFUL;
    }
    vSuccess = pVampDevResObj->m_pAudioCtrl->SetLoopbackPath(IN_AUDIO_S_VIDEO_1);
    if( vSuccess != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Failed to set audio loopback source"));
        if( CleanUpObjects() != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Clean up failed"));
        }
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The VBI video dataranges are updated by this function. Only dataranges
//  corresponding to 60Hz will be updated!
//
// Return Value:
//  STATUS_SUCCESS      VBI format was updated successfully to user frequency.
//  STATUS_UNSUCCESSFUL Any error case.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CVampDevice::UpdateVBIDataRanges
(
        IN OUT PKSDEVICE pKSDevice, // Describes our device that is managed
                                    // by AVStream.
        IN DWORD dwVBISampleFreq    // user defined sampling frequency read
                                    // from registry
)
{
    // Change VBI format settings, because user wants to have 28,636360MHz
    if( dwVBISampleFreq != KS_VBISAMPLINGRATE_5X_NABTS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid VBI Sample Rate"));
        return STATUS_UNSUCCESSFUL;
    }
    //*** get the VBI data range (start) ***//
    // check if parameters are valid
    if( !pKSDevice )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    if( !pKSDevice->Descriptor )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    // store number of filters that are part of this device (used for
    // filter search)
    DWORD dwFilterCount = pKSDevice->Descriptor->FilterDescriptorsCount;

    const KSFILTER_DESCRIPTOR *const *const
        pFilterDescriptor = pKSDevice->Descriptor->FilterDescriptors;
    // check the pointer
    if( !pFilterDescriptor )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid pointer"));
        return STATUS_UNSUCCESSFUL;
    }

    // if the pointer seems to be valid,
    // try to find the analog capture filter
    BOOL    bAnlCaptureFilterWasFound = FALSE;
    DWORD   dwFilterIndex = 0;

    // go through the array of filters and search for analog capture filter
    for(dwFilterIndex = 0; dwFilterIndex < dwFilterCount; dwFilterIndex++)
    {
        if( pFilterDescriptor[ dwFilterIndex ] != 0 )
        {
            // if the GUID matches the right filter, leave the loop and
            // set flag
            if( IsEqualGUID( *( pFilterDescriptor[ dwFilterIndex ]->
                                    ReferenceGuid ),
                                VAMP_ANLG_CAP_FILTER ) )
            {
                bAnlCaptureFilterWasFound = TRUE;
                break;
            }
        }
    }
    //check if the Vamp filter was found
    if( !bAnlCaptureFilterWasFound )
    {
        //no Anlg. video capture filter was found so return
        //it is an error even if it is just digital,
        //at least the INF file is wrong
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: No analog capture filter was found"));
        return STATUS_SUCCESS;
    }
    // If the AnlgVideoCaptureFilter was found,
    // and the pointer is valid, look for the VBI pin
    // on the just found anlg video capture filter
    DWORD dwPinDescCount =
            pFilterDescriptor[ dwFilterIndex ]->PinDescriptorsCount;

    const KSPIN_DESCRIPTOR_EX *const
        pPinDescEx =
            pFilterDescriptor[ dwFilterIndex ]->PinDescriptors;
    if( !pPinDescEx )
    {
        // no pin descriptor was defined
        _DbgPrintF(DEBUGLVL_VERBOSE,("Warning: Invalid pointer"));
        return STATUS_SUCCESS;
    }
    DWORD   dwPinIndex      = 0;
    BOOL    bVbiPinWasFound = FALSE;

    // try to find the VBI pin out of the filter
    for( dwPinIndex = 0; dwPinIndex < dwPinDescCount; dwPinIndex++ )
    {
        if( IsEqualGUID(*( pPinDescEx[ dwPinIndex ].PinDescriptor.Category ),\
                        PINNAME_VIDEO_VBI ) )
        {
            bVbiPinWasFound = TRUE;
            break;
        }
    }
    // if the VBI pin was found check the Pin description size
    // to make sure we edit the right structure
    if( !bVbiPinWasFound )
    {
        //no Anlg. VBI pin was found so return
        //it is not an error
        _DbgPrintF(DEBUGLVL_VERBOSE,("Warning: Could not find VBI pin"));
        return STATUS_SUCCESS;
    }
    DWORD dwDataRangesCount =
            pPinDescEx[ dwPinIndex ].PinDescriptor.DataRangesCount;
    const PKSDATARANGE *const pDataRanges =
            pPinDescEx[ dwPinIndex ].PinDescriptor.DataRanges;

    //*** get the VBI data range (end) ***//

    if( !pDataRanges )
    {
        // no dataranges were defined
        _DbgPrintF(DEBUGLVL_VERBOSE,
            ("Warning: No VBI data ranges defined"));
        return STATUS_UNSUCCESSFUL;
    }

    // change settings for 28,636363 MHz inside the
    // KS_DATARANGE_VIDEO_VBI formats, but only for 60Hz format
    for( DWORD dwIndex = 0; dwIndex < dwDataRangesCount; dwIndex++ )
    {
        // check the pointer and the size before using it
        if( pDataRanges[ dwIndex ] == NULL )
        {
            break;
        }

        if( pDataRanges[ dwIndex ]->FormatSize ==
            sizeof(KS_DATARANGE_VIDEO_VBI) )
        {
            PKSDATARANGE                  pDataRange;
            PKS_VIDEO_STREAM_CONFIG_CAPS  pConfigCaps;
            PKS_VBIINFOHEADER             pVBIInfoHeader;

            // get the VBI data range "KS_DATARANGE_VIDEO_VBI"
            PKS_DATARANGE_VIDEO_VBI pDataRangeVbi =
                reinterpret_cast <PKS_DATARANGE_VIDEO_VBI>(pDataRanges[ dwIndex ]);
            // get the KSDATARANGE out of the KS_DATARANGE_VIDEO_VBI
            pDataRange = &(pDataRangeVbi->DataRange);
            // get the KS_VIDEO_STREAM_CONFIG_CAPS out of the
            // KS_DATARANGE_VIDEO_VBI
            pConfigCaps = &(pDataRangeVbi->ConfigCaps);
            // get the KS_VBIINFOHEADER out of the KS_DATARANGE_VIDEO_VBI
            pVBIInfoHeader = &(pDataRangeVbi->VBIInfoHeader);


            // figure out the video standard supported by this format
            // changes for only 60Hz and 525 lines formats (like NTSC)
            if( !pDataRange || !pConfigCaps || !pVBIInfoHeader )
            {
                _DbgPrintF(DEBUGLVL_ERROR,
                    ("Error: Important parameter invalid"));
                return STATUS_UNSUCCESSFUL;
            }
            if( pConfigCaps->VideoStandard == VIDEO_STANDARD_60HZ_525LINES)
            {
                _DbgPrintF(DEBUGLVL_VERBOSE,
                    ("Warning: Update VBI 60Hz format"));

                //*** change settings so we support 28,636363 MHz ***//
                // update values inside the KSDATARANGE
                pDataRange->SampleSize = VBI_SAMPLE_RATIO_BT * VBI_LINES_NTSC;
                // update values inside the KS_VIDEO_STREAM_CONFIG_CAPS
//                pConfigCaps->InputSize.cx       = dwSamplingRatio;
                pConfigCaps->MinCroppingSize.cx = VBI_SAMPLE_RATIO_BT;
                pConfigCaps->MaxCroppingSize.cx = VBI_SAMPLE_RATIO_BT;
                pConfigCaps->MinOutputSize.cx   = VBI_SAMPLE_RATIO_BT;
                pConfigCaps->MaxOutputSize.cx   = VBI_SAMPLE_RATIO_BT;
                pConfigCaps->MinBitsPerSecond   = \
                pConfigCaps->MaxBitsPerSecond   = \
                    (VBI_BITS_PER_PIXEL * VBI_NTSC_FieldRate *
                        VBI_SAMPLE_RATIO_BT * VBI_LINES_NTSC);
                // update values inside the KS_VBIINFOHEADER
                pVBIInfoHeader->SamplingFrequency = dwVBISampleFreq;
                pVBIInfoHeader->SamplesPerLine    = VBI_SAMPLE_RATIO_BT;
                pVBIInfoHeader->StrideInBytes = VBI_PITCH_BT;
                // upstream VBI filter requirement
                pVBIInfoHeader->BufferSize = VBI_PITCH_BT * VBI_LINES_NTSC;
            } // if( 60Hz format )
        } // if( KS_DATARANGE_VIDEO_VBI )
    } // for( VBI DATA RANGES )

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description: Clean up the Resources
//
// Return Value:
// NONE
//
//////////////////////////////////////////////////////////////////////////////
void CVampDevice::CleanUpResources
(
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CleanUpResources called"));

    //de-allocate memory for the register io space
    if( m_pBaseAddress )
    {
        // oponent (MmMapIoSpace) called in InitializeResources
        // function has no return value, so we assume that it never fails
        MmUnmapIoSpace(m_pBaseAddress, m_dwMemoryLength);
        m_pBaseAddress   = NULL;
        m_dwMemoryLength = 0;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description: Clean up the Objects
//
// Return Value:
//  STATUS_SUCCESS       Objects cleaned up successfully.
//  STATUS_UNSUCCESSFUL  Any error case.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CVampDevice::CleanUpObjects
(
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CleanUpObjects called"));

    if( m_pServiceContext )
    {
        //remove service context
        m_pServiceContext->pDevResObj = NULL;
        m_pServiceContext->pDPC = NULL;
        delete m_pServiceContext;
        m_pServiceContext = NULL;
    }
    // check if a device was added to proxy in this function
    if( m_bDeviceWasAdded )
    {
        //remove device from proxy
        if( g_pKMProxy->RemoveDevice(m_dwDeviceIndex) != VAMPRET_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR, ("Error: Cannot remove device"));
            // do not return directly, continue cleanup anyway
            return STATUS_UNSUCCESSFUL;
        }
    }
    //de-allocate memory for the proxy object
    //if we are the last device
    if( g_pKMProxy )
    {
        DWORD dwNumOfDevices = 0;
        if( g_pKMProxy->GetNumberOfDevices( &dwNumOfDevices ) !=
            VAMPRET_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,
                ("Error: GetNumberOfDevices failed"));
            return STATUS_UNSUCCESSFUL;
        }
        // check if this was the last device
        if( dwNumOfDevices == 0 )
        {
            delete g_pKMProxy;
            g_pKMProxy = NULL;
        }
    }
    if( m_pFactoryObj )
    {
        //deallocate memory for the factory object
        delete m_pFactoryObj;
        m_pFactoryObj = NULL;
    }
    if( m_pSyncObject )
    {
        //deallocate memory for the sync object
        delete m_pSyncObject;
        m_pSyncObject = NULL;
    }
    if( m_pOSDependRegistryObj )
    {
        //deallocate memory for the registry object
        delete m_pOSDependRegistryObj;
        m_pOSDependRegistryObj = NULL;
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function frees all resources and deletes all objects which were
//  created by "Start".
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
void CVampDevice::Stop
(
    IN PKSDEVICE   pKSDevice, //Pointer to the KS device object
                              //provided by the system.(unused)
    IN PIRP        pIrp //Pointer to IRP for this request.(unused)
)
{
    ReleasePrivateMemory(m_pDMAAdapter, m_pServiceContext->pDevResObj);

    // remove IRQ object from sync object
    if( m_pSyncObject )
    {
        // function has no return value, so we assume that it never
        // fails
        m_pSyncObject->RemoveIRQObject();
    }
    // disconnect interrupt service routine
    if( m_pInterruptObject )
    {
        // function has no return value, so we assume that it never
        // fails
        IoDisconnectInterrupt(m_pInterruptObject);
        m_pInterruptObject = NULL;
    }
    // do cleanup for InitializeObjects
    if( CleanUpObjects() != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Clean up objects failed"));
    }
    // do cleanup for InitializeResources,
    // function has no return value, so we assume that it never fails
    CleanUpResources();
    return;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function returns the device parameter e.g. device ID, friendly name
//  vendor ID....
//
// Return Value:
//  Structure of all device parameters (tDeviceParams),
//  bDeviceIsPresent (part of tDeviceParams structure) is false if call fails.
//
//////////////////////////////////////////////////////////////////////////////
tDeviceParams CVampDevice::GetInfoOfDevice()
{
    tDeviceParams tDevPara = {NULL, 0, 0, 0, 0, 0, 0, FALSE, NULL};

    if( g_pKMProxy == NULL )
    {
        // no KM proxy available
        _DbgPrintF(DEBUGLVL_ERROR,("Error: No KM proxy available"));
        return tDevPara;
    }
    return g_pKMProxy->GetInfoOfDevice(m_dwDeviceIndex);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function returns the resource object of the device. With this object
//  you get access to all objects of the device e.g. GPIO, I2C....
//
// Return Value:
//  Pointer to CVampDeviceResources or NULL if function fails.
//
//////////////////////////////////////////////////////////////////////////////
CVampDeviceResources* CVampDevice::GetDeviceResourceObject()
{
    if( g_pKMProxy == NULL )
    {
        // no KM proxy available
        _DbgPrintF(DEBUGLVL_ERROR,("Error: No KM proxy available"));
        return NULL;
    }
    return g_pKMProxy->GetDeviceResourceObject(m_dwDeviceIndex);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function returns the registry object. It is needed if you want to
//  access (read, write) the registry.
//
// Return Value:
//  Pointer to COSDependRegistry or NULL if function fails.
//
//////////////////////////////////////////////////////////////////////////////
COSDependRegistry* CVampDevice::GetRegistryAccessObject()
{
    if( g_pKMProxy == NULL)
    {
        // no KM proxy available
        _DbgPrintF(DEBUGLVL_ERROR,("Error: No KM proxy available"));
        return NULL;
    }
    return g_pKMProxy->GetRegistryAccessObject(m_dwDeviceIndex);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function handles memory (Buffer) for the device.
//
// Return Value:
//  STATUS_SUCCESS           Request successfully procceded.
//  STATUS_NOT_IMPLEMENTED   Switch cases we do not support.
//  STATUS_UNSUCCESSFUL      Any other error case.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CVampDevice::IOCTL_DeviceMemory
(
    IN IOCTL_COMMAND* pCommandStructure,//Pointer to request info structure.
    IN PDEVICE_OBJECT pPDO //Pointer to the physical device object.
)
{
    if( (sizeof(IOCTL_DEVICE_MEMORYStruct) !=
         pCommandStructure->dioc_cbInBuf) ||
        (sizeof(IOCTL_DEVICE_MEMORYStruct) !=
         pCommandStructure->dioc_cbOutBuf) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: IOCTL_DevicePCI failed (wrong size)"));
        return STATUS_UNSUCCESSFUL;
    }

    pIOCTL_DEVICE_MEMORYStruct pInBuffer =
                static_cast <pIOCTL_DEVICE_MEMORYStruct> (pCommandStructure->dioc_InBuf);
    pIOCTL_DEVICE_MEMORYStruct pOutBuffer =
                static_cast <pIOCTL_DEVICE_MEMORYStruct> (pCommandStructure->dioc_OutBuf);

    switch(pCommandStructure->dioc_IOCtlCode)
    {
    case IOCTL_DEVICE_MEMORY_REGISTER_Set:
        if( g_pKMProxy == NULL )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: KM proxy not available"));
            return STATUS_UNSUCCESSFUL;
        }
        if( g_pKMProxy->
                    GetInfoOfDevice(pInBuffer->dwDevIndex).bDeviceIsPresent )
        {
            PBYTE pAddress =
                static_cast <BYTE*>(g_pKMProxy->GetInfoOfDevice(pInBuffer->dwDevIndex). \
                                    pBaseAddress + pInBuffer->dwMEMRegisterOffset);
            //create an OSDepend RegisterAccess object
            COSDependRegisterAccess* pOSDependRegisterAccess =
                new (NON_PAGED_POOL) COSDependRegisterAccess();
            if( !pOSDependRegisterAccess )
            {
                _DbgPrintF(DEBUGLVL_ERROR,
                    ("Error: Cannot create register access"));
                return STATUS_UNSUCCESSFUL;
            }

            pOSDependRegisterAccess->SetReg( pAddress, pInBuffer->dwValue);

            delete pOSDependRegisterAccess;
            pOSDependRegisterAccess = NULL;

            return STATUS_SUCCESS;
        }
        else
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Device not present"));
            return STATUS_UNSUCCESSFUL;
        }
    case IOCTL_DEVICE_MEMORY_REGISTER_Get:
        if( g_pKMProxy == NULL )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: KM proxy not available"));
            return STATUS_UNSUCCESSFUL;
        }
        if( g_pKMProxy->
                    GetInfoOfDevice(pInBuffer->dwDevIndex).bDeviceIsPresent )
        {
            PBYTE pAddress =
                static_cast <BYTE*>(g_pKMProxy->GetInfoOfDevice(pInBuffer->dwDevIndex). \
                                    pBaseAddress + pInBuffer->dwMEMRegisterOffset);
            //create an OSDepend RegisterAccess object
            COSDependRegisterAccess* pOSDependRegisterAccess =
                new (NON_PAGED_POOL) COSDependRegisterAccess();
            if( !pOSDependRegisterAccess )
            {
                _DbgPrintF(DEBUGLVL_ERROR,
                    ("Error: Cannot create register access"));
                return STATUS_UNSUCCESSFUL;
            }

            pOutBuffer->dwValue = pOSDependRegisterAccess->GetReg( pAddress );

            delete pOSDependRegisterAccess;
            pOSDependRegisterAccess = NULL;

            return STATUS_SUCCESS;
        }
        else
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Device not present"));
            pOutBuffer->dwValue = 0;
            return STATUS_UNSUCCESSFUL;
        }
    case IOCTL_DEVICE_MEMORY_MapPhysicalToLinear:
        // this call causes memory leaks so we do not allow this anymore

//        pOutBuffer->dwMemoryAddress = MmMapIoSpace(
//              (PVOID)(pInBuffer->dwMemoryAddress),
//              pInBuffer->dwMemorySizeInBytes,
//              MmNonCached);
        return STATUS_NOT_IMPLEMENTED;
    case IOCTL_DEVICE_MEMORY_MapLinearToPhysical:
        {
            PMDL pMDL = IoAllocateMdl(reinterpret_cast <PVOID>(pInBuffer->ulPtrMemoryAddress),
                                       1,
                                       false,
                                       false,
                                       NULL);
            if( !pMDL )
            {
                _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
                return STATUS_UNSUCCESSFUL;
            }
            MmBuildMdlForNonPagedPool(pMDL);
            ULONG* pFixedPages = reinterpret_cast <ULONG*>(&pMDL[1]);
            ULONG_PTR ulPtrOffset = (0xfff & (pInBuffer->ulPtrMemoryAddress));
            pOutBuffer->ulPtrMemoryAddress = (pFixedPages[0]<<12) + ulPtrOffset;
            IoFreeMdl(pMDL);
            return STATUS_SUCCESS;
        }
    default:
        return STATUS_NOT_IMPLEMENTED;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This functon is called to get or set the PCI config space information
//  of that device.
//
// Return Value:
//  STATUS_SUCCESS           Request successfully procceded.
//  STATUS_UNSUCCESSFUL      Any error case.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CVampDevice::IOCTL_DevicePCI
(
    IN IOCTL_COMMAND* pCommandStructure,//Pointer to request info structure.
    IN PDEVICE_OBJECT pPDO //Pointer to the physical device object.
)
{
    if( (sizeof(IOCTL_DEVICE_PCIStruct) != pCommandStructure->dioc_cbInBuf) ||
        (sizeof(IOCTL_DEVICE_PCIStruct) != pCommandStructure->dioc_cbOutBuf) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: IOCTL_DevicePCI failed (wrong size)"));
        return STATUS_UNSUCCESSFUL;
    }

    pIOCTL_DEVICE_PCIStruct pInBuffer =
                    static_cast <pIOCTL_DEVICE_PCIStruct>(pCommandStructure->dioc_InBuf);
    pIOCTL_DEVICE_PCIStruct pOutBuffer =
                    static_cast <pIOCTL_DEVICE_PCIStruct>(pCommandStructure->dioc_OutBuf);

    switch(pCommandStructure->dioc_IOCtlCode)
    {
        case IOCTL_DEVICE_PCI_REGISTER_Set:
            _DbgPrintF(DEBUGLVL_BLAB,
                       ("Info: IOCTL_DEVICE_PCI_REGISTER_Set called"));
            if( g_pKMProxy == NULL )
            {
                _DbgPrintF(DEBUGLVL_ERROR,("Error: KM proxy not available"));
                return STATUS_UNSUCCESSFUL;
            }
            if( g_pKMProxy->
                    GetInfoOfDevice(pInBuffer->dwDevIndex).bDeviceIsPresent )
            {
                SetConfigSpaceByOffset(
                    reinterpret_cast <PDEVICE_OBJECT>((g_pKMProxy->
                        GetInfoOfDevice(pInBuffer->dwDevIndex).ulPtrDeviceNode)),
                    pInBuffer->dwPCIRegisterOffset,
                    pInBuffer->dwValue);
                return STATUS_SUCCESS;
            }
            else
            {
                _DbgPrintF(DEBUGLVL_ERROR,("Error: Device not present"));
                return STATUS_UNSUCCESSFUL;
            }
        case IOCTL_DEVICE_PCI_REGISTER_Get:
            _DbgPrintF(DEBUGLVL_BLAB,
                       ("Info: IOCTL_DEVICE_PCI_REGISTER_Get called"));
            if( g_pKMProxy == NULL )
            {
                _DbgPrintF(DEBUGLVL_ERROR,("Error: KM proxy not available"));
                return STATUS_UNSUCCESSFUL;
            }
            if( g_pKMProxy->
                    GetInfoOfDevice(pInBuffer->dwDevIndex).bDeviceIsPresent )
            {
                pOutBuffer->dwValue =
                    GetConfigSpaceByOffset(
                        reinterpret_cast <PDEVICE_OBJECT>((g_pKMProxy->GetInfoOfDevice(
                            pInBuffer->dwDevIndex).ulPtrDeviceNode)),
                        pInBuffer->dwPCIRegisterOffset);
                return STATUS_SUCCESS;
            }
            else
            {
                _DbgPrintF(DEBUGLVL_ERROR,("Error: Device not present"));
                return STATUS_UNSUCCESSFUL;
            }
        default:
            break;
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function is used by "IOCTL_DevicePCI" to get the config space
//  information of the device.
//
// Return Value:
//  Buffer with information.
//  0 in any error case.
//
//////////////////////////////////////////////////////////////////////////////
DWORD CVampDevice::GetConfigSpaceByOffset
(
    IN PDEVICE_OBJECT pPDO, //Pointer to the physical device object.
    IN DWORD          dwOffset //From which position the info is needed.
)
{
    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
    DWORD dwValue = 0;

    CCHAR cStack = pPDO->StackSize;

    PIRP pIrp = IoAllocateIrp( cStack, false );
    if( pIrp )
    {
        PIO_STACK_LOCATION pIoStack = IoGetNextIrpStackLocation( pIrp );

        pIoStack->MajorFunction = IRP_MJ_PNP;
        pIoStack->MinorFunction = IRP_MN_READ_CONFIG;

        pIoStack->Parameters.ReadWriteConfig.WhichSpace = 0;
        pIoStack->Parameters.ReadWriteConfig.Buffer = static_cast <PVOID>(&dwValue);
        pIoStack->Parameters.ReadWriteConfig.Offset = dwOffset;
        pIoStack->Parameters.ReadWriteConfig.Length = sizeof( dwValue );

        CSyncIrpCall scall;
        ntStatus = scall.Call( pPDO, pIrp );
        if( !NT_SUCCESS(ntStatus) )
        {
            dwValue = 0;
        }
        IoFreeIrp(pIrp);
        return dwValue;
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function requests the whole config space information and returns
//  the information in the passed PCI config buffer.
//
// Return Value:
//  NTSTATUS - value returned by SyncIrpCall. On success it
//             should be STATUS_SUCCESS.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CVampDevice::GetConfigSpace
(
    IN  PDEVICE_OBJECT     pPDO, //Pointer to the physical device object.
    OUT PPCI_COMMON_CONFIG pPCIConfig //Buffer for config space information.
)
{
    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

    CCHAR cStack = pPDO->StackSize;

    PIRP pIrp = IoAllocateIrp( cStack, false );
    if( !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IoAllocateIrp failed"));
        return STATUS_UNSUCCESSFUL;

    }
    PIO_STACK_LOCATION pIoStack = IoGetNextIrpStackLocation( pIrp );

    pIoStack->MajorFunction = IRP_MJ_PNP;
    pIoStack->MinorFunction = IRP_MN_READ_CONFIG;

    pIoStack->Parameters.ReadWriteConfig.WhichSpace = 0;
    pIoStack->Parameters.ReadWriteConfig.Buffer =
        static_cast <PVOID> (pPCIConfig);
    pIoStack->Parameters.ReadWriteConfig.Offset = 0;
    pIoStack->Parameters.ReadWriteConfig.Length = sizeof( PCI_COMMON_CONFIG );

    CSyncIrpCall scall;
    ntStatus = scall.Call( pPDO, pIrp );
    IoFreeIrp(pIrp);
    return ntStatus;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function writes information to the config space on a positioin
//  defined by an offset.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
void CVampDevice::SetConfigSpaceByOffset
(
    IN PDEVICE_OBJECT pPDO, //Pointer to the physical device object.
    IN DWORD          dwOffset, //Position where to write.
    IN DWORD          dwValue //Value to write.
)
{
    CCHAR cStack = pPDO->StackSize;

    PIRP pIrp = IoAllocateIrp( cStack, false );
    if( pIrp )
    {
        PIO_STACK_LOCATION pIoStack = IoGetNextIrpStackLocation( pIrp );

        pIoStack->MajorFunction = IRP_MJ_PNP;
        pIoStack->MinorFunction = IRP_MN_WRITE_CONFIG;

        pIoStack->Parameters.ReadWriteConfig.WhichSpace = 0;
        pIoStack->Parameters.ReadWriteConfig.Buffer = static_cast <PVOID>(&dwValue);
        pIoStack->Parameters.ReadWriteConfig.Offset = dwOffset;
        pIoStack->Parameters.ReadWriteConfig.Length = sizeof( dwValue );

        CSyncIrpCall scall;
        static_cast <void>(scall.Call( pPDO, pIrp ));
        IoFreeIrp(pIrp);
    }
    return;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function is called to set a new power state to the device.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
void CVampDevice::SetPower
(
    IN PKSDEVICE pKSDevice,    //Pointer to the KS device object
                               //provided by the system.(unused)
    IN PIRP      pIrp,         //Pointer to IRP for this request.(unused)
    IN DEVICE_POWER_STATE To,  //Power state we have to enter.
    IN DEVICE_POWER_STATE From //Power state we are currently in.(unused)
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: SetPower called to state: %d",To));

    CVampDeviceResources* pDevResObj = GetDeviceResourceObject();

    if( pDevResObj == NULL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Invalid device resource object"));
        return;
    }

    if( pDevResObj->m_pPowerCtrl->
            ChangePowerState(static_cast <eVampPowerState>(To)) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: SetPower to state: %d failed",To));
    }
    //TuneToFrequency(pDevResObj);

    return;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function is called to get the driver medium instance for this instance
//  of the device
//
// Return Value:
//  m_uiDriverMediumInstanceCount - instance specific medium count
//
//////////////////////////////////////////////////////////////////////////////
UINT CVampDevice::GetDriverMediumInstanceCount
(
 )
{
    return m_uiDriverMediumInstanceCount;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function is called to set the driver medium instance for this instance
//  of the device
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
VOID CVampDevice::SetDriverMediumInstanceCount
(
    UINT uiDriverMediumInstanceCount
 )
{
    m_uiDriverMediumInstanceCount = uiDriverMediumInstanceCount;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function creates filter factories for all the filters exported by the 
//  by the device and sets the right mediums on the pins to support multi-instancing
//
//  Using the medium id to distinguish different device instances has two implications:
//  (i) The medium id in the filter descriptor should be in writable memory (currently the filter
//  descriptor is defined to be const and hence read-only)
//  (ii) The different device instances should point to different instances of the filter 
//  descriptor (otherwise they would be using the same medium id).
//
//  Based on this reasoning we go through the following cycle for each filter:
//  (a) Allocate a new (writable) copy of the filter descriptor and set the medium id's
//  (b) Create filter factory
//  (c) Update the registry 
//
// Return Value:
//  STATUS_SUCCESS       All filter factories were created successfully.
//  STATUS_*             Error code for failure
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS 
CVampDevice::CreateFilterFactories(
   IN PKSDEVICE pKSDevice
   )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    // Newly allocated filter descriptor
    KSFILTER_DESCRIPTOR* pNewFilterDescriptor = NULL;
    // Newly created filter factory
    PKSFILTERFACTORY pFilterFactory = NULL;


    // Hold all the filter descriptors in an array 
    // The (BDA) digital tuner filter alone is handled separately
    const KSFILTER_DESCRIPTOR *pFilterDescriptors[] = 
    {
        &AnlgTunerFilterDescriptor, 
        &AnlgTVAudioFilterDescriptor,
        &AnlgXBarFilterDescriptor,
        &AnlgCaptureFilterDescriptor,
        &WaveInFilterDescriptor,
        &DgtlCaptureFilterDescriptor
    };
    
    // Loop through all the filter descriptors in the array
    for (int i = 0; i < SIZEOF_ARRAY(pFilterDescriptors); i++)
    {
        // Create a new copy of the filter descriptor with the correct medium id
        ntStatus = AllocateFilterDescriptorForMediums(*pFilterDescriptors[i], 
            pKSDevice->Bag,
            &pNewFilterDescriptor);
        if (!NT_SUCCESS(ntStatus)) 
        {
            _DbgPrintF(DEBUGLVL_ERROR, ("Error : Allocate filter descriptor failed"));
            return ntStatus;
        }

        // Create filter factory
        ntStatus = KsCreateFilterFactory(pKSDevice->FunctionalDeviceObject,
            pNewFilterDescriptor,
            NULL, NULL, 0, NULL, NULL, &pFilterFactory);
        if (!NT_SUCCESS(ntStatus)) 
        {
            _DbgPrintF(DEBUGLVL_ERROR, ("Error : Failed to create filter factory"));
            return ntStatus;
        }

        // Update the registry - FilterData and MediumCache 
        ntStatus = KsFilterFactoryUpdateCacheData(pFilterFactory, NULL);
        if (!NT_SUCCESS(ntStatus))
        {
            _DbgPrintF(DEBUGLVL_VERBOSE, ("Warning : Update cache data failed"));
            return ntStatus;
        }
    }

    // The digital tuner filter is handled separately because it is a BDA filter with
    // a template descriptor and nodes

    // Create a new copy of the filter descriptor with the correct medium id
    ntStatus = AllocateFilterDescriptorForMediums(DgtlTunerFilterDescriptor,
                                                  pKSDevice->Bag,
                                                  &pNewFilterDescriptor);
    if (!NT_SUCCESS(ntStatus))
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error : Allocate filter descriptor failed - digital tuner"));
        return ntStatus;
    }

    // Allocate dynamic memory for the template since we need to modify the embedded descriptor
    BDA_FILTER_TEMPLATE *pNewBdaTemplateDgtlTunerFilter = NULL;
    pNewBdaTemplateDgtlTunerFilter = (BDA_FILTER_TEMPLATE *) ExAllocatePoolWithTag(
                                            NonPagedPool, sizeof(BDA_FILTER_TEMPLATE), 'pmaV');
    if (!pNewBdaTemplateDgtlTunerFilter)
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error : Insufficient memory for digital tuner template"));
        ntStatus = STATUS_NO_MEMORY;
        return ntStatus;
    }

    ntStatus = KsAddItemToObjectBag(pKSDevice->Bag, pNewBdaTemplateDgtlTunerFilter, NULL);
    if (!NT_SUCCESS(ntStatus))
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error : Failed to add bda template filter to object bag"));
        ExFreePool(pNewBdaTemplateDgtlTunerFilter);
        return ntStatus;
    }

    pNewBdaTemplateDgtlTunerFilter->ulcPinPairs = BdaTemplateDgtlTunerFilter.ulcPinPairs;
    pNewBdaTemplateDgtlTunerFilter->pPinPairs = BdaTemplateDgtlTunerFilter.pPinPairs;
    pNewBdaTemplateDgtlTunerFilter->pFilterDescriptor = pNewFilterDescriptor;

    // Create filter factory
    ntStatus = BdaCreateFilterFactoryEx( pKSDevice,
                                       pNewFilterDescriptor,
                                       pNewBdaTemplateDgtlTunerFilter,
                                       &pFilterFactory);

    if (!NT_SUCCESS(ntStatus)) 
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Cannot create digital tuner filter factory"));
        return ntStatus;
    }

    // Update the registry - FilterData and MediumCache 
    // Explicitly pass filter descriptor because pins are created only at FilterCreate (by BdaInit)
    ntStatus = BdaFilterFactoryUpdateCacheData(pFilterFactory, pNewFilterDescriptor);
    if (!NT_SUCCESS(ntStatus))
    {
        _DbgPrintF(DEBUGLVL_VERBOSE, ("Warning : Update cache data failed"));
        return ntStatus;
    }

    return ntStatus;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function creates a filter descriptor. Notice that by default the 
//  filter and pin descriptors are static values in the code segment and so 
//  the medium is hardcoded. By dynamically creating the filter descriptor 
//  (and filter factory) we are able to update the medium id for every new 
//  instance of the device
//
// Return Value:
//  Status
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CVampDevice::AllocateFilterDescriptorForMediums
(
    IN KSFILTER_DESCRIPTOR FilterDescriptor,
    IN KSOBJECT_BAG ObjectBag,
    OUT PKSFILTER_DESCRIPTOR *ppFilterDescriptor
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;    

    // Dynamically allocate filter descriptor. 
    *ppFilterDescriptor = (PKSFILTER_DESCRIPTOR) 
        ExAllocatePoolWithTag(NonPagedPool, sizeof(KSFILTER_DESCRIPTOR), 'pmaV');
    if (!*ppFilterDescriptor)
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error : Insufficient memory"));
        ntStatus = STATUS_NO_MEMORY;
        return ntStatus;
    }
    ntStatus = KsAddItemToObjectBag(ObjectBag, *ppFilterDescriptor, NULL);
    if (!NT_SUCCESS(ntStatus))
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error : Failed to add filter descriptor to object bag"));
        ExFreePool(*ppFilterDescriptor);
        return ntStatus;
    }
    // Use the values from the original descriptor by default (even if they are static)
    // Only those fields will be reallocated that need to have updated values - in this
    // case the pin descriptors and the mediums
    **ppFilterDescriptor = FilterDescriptor;

    if (FilterDescriptor.PinDescriptorsCount == 0)
    {
        return STATUS_SUCCESS;
    }

    // Allocate the pin descriptor list dynamically
    KSPIN_DESCRIPTOR_EX* pPinDescriptorsEx = 
        (KSPIN_DESCRIPTOR_EX *) ExAllocatePoolWithTag(NonPagedPool, 
                                  FilterDescriptor.PinDescriptorSize * FilterDescriptor.PinDescriptorsCount,
                                  'pmaV');
    if (!pPinDescriptorsEx)
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error : Insufficient memory for pin descriptors"));
        ntStatus = STATUS_NO_MEMORY;
        return ntStatus;
    }
    ntStatus = KsAddItemToObjectBag(ObjectBag, pPinDescriptorsEx, NULL);
    if (!NT_SUCCESS(ntStatus))
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error : Failed to add pin descriptors to object bag"));
        ExFreePool(pPinDescriptorsEx);
        return ntStatus;
    }

    // Copy default values over even if the default pointers are static. Update/reallocate
    // only required fields - namely the mediums
    RtlCopyMemory(pPinDescriptorsEx, FilterDescriptor.PinDescriptors, 
        FilterDescriptor.PinDescriptorSize * FilterDescriptor.PinDescriptorsCount);


    // Update/reallocate the relevant fields - the mediums
    for (unsigned int i = 0; i < FilterDescriptor.PinDescriptorsCount; i++)
    {

        // The size of the pin descriptors is specified in the filter descriptor
        KSPIN_DESCRIPTOR_EX* pCurrentPinDescriptorEx = 
            (KSPIN_DESCRIPTOR_EX *) (((BYTE *)pPinDescriptorsEx) + i * FilterDescriptor.PinDescriptorSize);
        KSPIN_DESCRIPTOR* pPinDescriptor = &pCurrentPinDescriptorEx->PinDescriptor;
        KSPIN_MEDIUM *pPinMediums = NULL;

        // If this pin has no mediums continue
        if (pPinDescriptor->MediumsCount == 0) {
            continue;
        }

        // Allocate memory for the pin mediums
        pPinMediums = (KSPIN_MEDIUM *)ExAllocatePoolWithTag(NonPagedPool, 
                                        sizeof(KSPIN_MEDIUM) * pPinDescriptor->MediumsCount, 
                                        'pmaV');
        if (!pPinMediums)
        {
            _DbgPrintF(DEBUGLVL_ERROR, ("Error : Insufficient memory for pin mediums"));
            ntStatus = STATUS_NO_MEMORY;
            return ntStatus;
        }
        ntStatus = KsAddItemToObjectBag(ObjectBag, pPinMediums, NULL);
        if (!NT_SUCCESS(ntStatus))
        {
            _DbgPrintF(DEBUGLVL_ERROR, ("Error : Failed to add pin mediums to object bag"));
            ExFreePool(pPinMediums);
            return ntStatus;
        }

        // Copy over all the mediums. The Medium.Set guid is the same across all instances.
        RtlCopyMemory(pPinMediums, pPinDescriptor->Mediums, 
            sizeof(KSPIN_MEDIUM) * pPinDescriptor->MediumsCount);

        for (unsigned int j = 0; j < pPinDescriptor->MediumsCount; j++)
        {
            // The core of this function. Update the Medium.Id

            // This allows multiple instances to be uniquely identified and
            // connected.  The value used in .Id is not important, only that
            // it is unique for each hardware connection. For example, one could
            // use serial numbers to generate such a unique Id.

            ((KSPIN_MEDIUM *)pPinMediums)[j].Id = m_uiDriverMediumInstanceCount;
        }
        pPinDescriptor->Mediums = pPinMediums;
    }
    (*ppFilterDescriptor)->PinDescriptors = pPinDescriptorsEx;

    return ntStatus;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function a pointer to the owning process handle. The owning process is
//  the one that takes one of the device filters to acquire state first.
//
// Return Value:
//  Owning process
//
//////////////////////////////////////////////////////////////////////////////
PEPROCESS 
CVampDevice::GetOwnerProcessHandle()
{
    return m_phOwnerProcess;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function sets the current owning process handle. The owning process
//  is the one that takes one of the device filters to acquire. When there are 
//  multiple processes trying to own the device, the first one wins and a spin lock
//  is used to ensure that the winning process returns success, and a losing process
//  returns a failure code.
//
//  The method employed to set the owning process needs to be robust, and shouldn't
//  make the device to be unusable in low-resource, error prone situations by setting
//  an incorrect process owner. The current method works for single tuner devices only.
//  For multi-tuner devices each tuner may be associated with a separate owning process.
//
// Return Value:
//  Status
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS 
CVampDevice::SetOwnerProcessHandle(
    PEPROCESS phOwnerProcess)
{
    NTSTATUS status = STATUS_SUCCESS;
    KIRQL OldIrql;
    PEPROCESS phCurrentProcess = PsGetCurrentProcess();

    KeAcquireSpinLock(&m_ProcessSpinLock, &OldIrql);

    if ((phOwnerProcess == NULL) && (m_phOwnerProcess == phCurrentProcess))    
        m_phOwnerProcess = NULL;

    else if (m_phOwnerProcess == NULL) 
        m_phOwnerProcess = phOwnerProcess;

    else if (m_phOwnerProcess != phOwnerProcess)
        status = STATUS_UNSUCCESSFUL;

    KeReleaseSpinLock(&m_ProcessSpinLock, OldIrql);

    return status;
}

PDMA_ADAPTER CVampDevice::GetDMAAdapter()
{
    return m_pDMAAdapter;
}


//NTSTATUS CVampDevice::QueryInterface
//(
//  IN PKSDEVICE pKSDevice,
//  IN PIRP pIrp
//)
//{
//  NTSTATUS ntStatus = STATUS_NOT_SUPPORTED;
//
//  //*** get information about the interface that is asked for ***//
//
//  //get the interface guid
//  const GUID* pQueryInterfaceGuid =
//      IoGetCurrentIrpStackLocation(pIrp)->Parameters.QueryInterface.InterfaceType;
//  //get the size of the interface structure
//  USHORT usSize =
//      IoGetCurrentIrpStackLocation(pIrp)->Parameters.QueryInterface.Size;
//  //get the version of the interface
//  USHORT usVersion =
//      IoGetCurrentIrpStackLocation(pIrp)->Parameters.QueryInterface.Version;
//  //get the interface pointer for later use
//    PINTERFACE pInterface =
//      IoGetCurrentIrpStackLocation(pIrp)->Parameters.QueryInterface.Interface;
//
//  //*** check interface support and answer apropriate ***//
//
//  //check if interface is supported
//  if( IsEqualGUID(*pQueryInterfaceGuid, GUID_I2C_INTERFACE) &&
//      (usSize == sizeof(I2CINTERFACE)) &&
//      (usVersion == 1) )
//  {
//      reinterpret_cast <I2CINTERFACE *>(pInterface)->i2cOpen =   CVampI2CInterface::I2COpen;
//      reinterpret_cast <I2CINTERFACE *>(pInterface)->i2cAccess = CVampI2CInterface::I2CAccess;
//      IoGetCurrentIrpStackLocation(pIrp)->Parameters.QueryInterface.InterfaceSpecificData =
//          NULL;
//  }
//  else if( IsEqualGUID(*pQueryInterfaceGuid, GUID_GPIO_INTERFACE) &&
//           (usSize == sizeof(I2CINTERFACE)) &&
//           (usVersion == 1) )
//  {
//      reinterpret_cast <GPIOINTERFACE *>(pInterface)->gpioOpen =   CVampGpioInterface::GpioOpen;
//      reinterpret_cast <GPIOINTERFACE *>(pInterface)->gpioAccess = CVampGpioInterface::GpioAccess;
//      IoGetCurrentIrpStackLocation(pIrp)->Parameters.QueryInterface.InterfaceSpecificData =
//          NULL;
//  }
//  return ntStatus;
//}
