#include "common.h"

#include "bdadebug.h"

#define IsEqualGUID(rguid1, rguid2) (!memcmp(rguid1, rguid2, sizeof(GUID)))

/**************************************************************/
/* Driver Name - Change the driver name to reflect your executable name! */
/**************************************************************/

#define MODULENAME           "BDA Generic Tuner Sample"
#define MODULENAMEUNICODE   L"BDA Generic Tuner Sample"

#define STR_MODULENAME      MODULENAME

// This defines the name of the WMI device that manages service IOCTLS
#define DEVICENAME (L"\\\\.\\" MODULENAMEUNICODE)
#define SYMBOLICNAME (L"\\DosDevices\\" MODULENAMEUNICODE)

#define ATSC_RECEIVER   1
//#define DVBS_RECEIVER    1
// #define DVBT_RECEIVER    1
// #define CABLE_RECEIVER   1

// Must define exactly one of the 4 above
# if !(defined(ATSC_RECEIVER) || defined(DVBT_RECEIVER) || defined(DVBS_RECEIVER) || defined (CABLE_RECEIVER))
#error "Must define exactly one of ATSC, DVBT, DVBS or CABLE"
#endif
# if defined(ATSC_RECEIVER) && (defined(DVBT_RECEIVER) || defined(DVBS_RECEIVER) || defined (CABLE_RECEIVER))
#error “Multiple tranport definitions"
# elif defined(DVBT_RECEIVER) && (defined(ATSC_RECEIVER) || defined(DVBS_RECEIVER) || defined (CABLE_RECEIVER))
#error “Multiple tranport definitions"
# elif defined(DVBS_RECEIVER) && (defined(ATSC_RECEIVER) || defined(DVBT_RECEIVER) || defined (CABLE_RECEIVER))
#error “Multiple tranport definitions"
# elif defined(CABLE_RECEIVER) && (defined(ATSC_RECEIVER) || defined(DVBS_RECEIVER) || defined (DVBT_RECEIVER))
#error “Multiple tranport definitions"
#endif

#define MS_SAMPLE_TUNER_POOL_TAG 'TadB'


//  Define a structure that represents what the underlying device can do.
//
//  Note -  It is possible to set conflicting settings.  In this case, the 
//  CFilter::CheckChanges method should return an error. Only a 
//  self-consistent resource should be submitted to the underlying device.
//
typedef struct _BDATUNER_DEVICE_RESOURCE
{
    //  Tuner Resources
    //
    ULONG               ulCarrierFrequency;
    ULONG               ulFrequencyMultiplier;
    GUID                guidDemodulator;

    //  Demodulator Resources
    //
    ULONG               ulDemodProperty1;
    ULONG               ulDemodProperty2;
    ULONG               ulDemodProperty3;
} BDATUNER_DEVICE_RESOURCE, * PBDATUNER_DEVICE_RESOURCE;


//  Define a structure that represents the underlying device status.
//
typedef struct _BDATUNER_DEVICE_STATUS
{
    //  Tuner Status
    //
    BOOLEAN             fCarrierPresent;

    //  Demodulator Status
    //
    BOOLEAN             fSignalLocked;
} BDATUNER_DEVICE_STATUS, * PBDATUNER_DEVICE_STATUS;


extern const KSDEVICE_DESCRIPTOR DeviceDescriptor;

//
// Define the filter class.
//
class CFilter {
public:

    //
    //  Define the AVStream Filter Dispatch Functions
    //  Network provider and AVStream use these functions 
    //  to create or remove a filter instance
    //
    static
    STDMETHODIMP_(NTSTATUS)
    Create(
        IN OUT PKSFILTER Filter,
        IN PIRP Irp
        );

    static
    STDMETHODIMP_(NTSTATUS)
    FilterClose(
        IN OUT PKSFILTER Filter,
        IN PIRP Irp
        );

/**************************************************************/
/* Only used to process frames. 
 *  Filters that transport data do not implement this dispatch function.
    static
    STDMETHODIMP
    Process(
        IN PKSFILTER Filter,
        IN PKSPROCESSPIN_INDEXENTRY ProcessPinsIndex
        );*/
/**************************************************************/

    //
    //  KSMETHODSETID_BdaChangeSync 
    //  Filter change sync methods
    //
    static
    STDMETHODIMP_(NTSTATUS)
    StartChanges(
        IN PIRP         pIrp,
        IN PKSMETHOD    pKSMethod,
        OPTIONAL PVOID  pvIgnored
        );

    static
    STDMETHODIMP_(NTSTATUS)
    CheckChanges(
        IN PIRP         pIrp,
        IN PKSMETHOD    pKSMethod,
        OPTIONAL PVOID  pvIgnored
        );

    static
    STDMETHODIMP_(NTSTATUS)
    CommitChanges(
        IN PIRP         pIrp,
        IN PKSMETHOD    pKSMethod,
        OPTIONAL PVOID  pvIgnored
        );

    static
    STDMETHODIMP_(NTSTATUS)
    GetChangeState(
        IN PIRP         pIrp,
        IN PKSMETHOD    pKSMethod,
        OUT PULONG      pulChangeState
        );

    static
    STDMETHODIMP_(NTSTATUS)
    GetMedium(
        IN PIRP             pIrp,
        IN PKSPROPERTY      pKSProperty,
        IN KSPIN_MEDIUM *   pulProperty
        );

    //
    //  KSMETHODSETID_BdaDeviceConfiguration 
    //  Methods to modify filter topology.
    //
    static
    STDMETHODIMP_(NTSTATUS)
    CreateTopology(
        IN PIRP         pIrp,
        IN PKSMETHOD    pKSMethod,
        OPTIONAL PVOID  pvIgnored
        );

    //
    //  Filter Implementation Methods
    //
    STDMETHODIMP_(class CDevice *)
    GetDevice() { return m_pDevice;};

    STDMETHODIMP_(NTSTATUS)
    PutFrequency(
        IN ULONG        ulBdaParameter
        )
        {
            m_NewResource.ulCarrierFrequency = ulBdaParameter;
            m_BdaChangeState = BDA_CHANGES_PENDING;

            return STATUS_SUCCESS;
        };

    STDMETHODIMP_(NTSTATUS)
    GetFrequency(
        IN PULONG        pulBdaParameter
        )
        {
            *pulBdaParameter = m_CurResource.ulCarrierFrequency;
            
            return STATUS_SUCCESS;
        };

    STDMETHODIMP_(NTSTATUS)
    SetDemodulator(
        IN const GUID *       pguidDemodulator
        );

    STDMETHODIMP_(NTSTATUS)
    SetDemodProperty1(
        IN ULONG        ulDemodProperty1
        )
    {
            m_NewResource.ulDemodProperty1 = ulDemodProperty1;
            m_BdaChangeState = BDA_CHANGES_PENDING;

            return STATUS_SUCCESS;
    }

    STDMETHODIMP_(NTSTATUS)
    GetDemodProperty1(
        IN PULONG       pulDemodProperty1
        )
    {
        if (pulDemodProperty1)
        {
            *pulDemodProperty1 = m_CurResource.ulDemodProperty1;
            return STATUS_SUCCESS;
        }
        else
        {
            return STATUS_INVALID_PARAMETER;
        }
    }

    STDMETHODIMP_(NTSTATUS)
    SetDemodProperty2(
        IN ULONG        ulDemodProperty2
        )
    {
            m_NewResource.ulDemodProperty2 = ulDemodProperty2;
            m_BdaChangeState = BDA_CHANGES_PENDING;

            return STATUS_SUCCESS;
    }

    STDMETHODIMP_(NTSTATUS)
    GetDemodProperty3(
        IN PULONG       pulDemodProperty3
        )
    {
        if (pulDemodProperty3)
        {
            *pulDemodProperty3 = m_CurResource.ulDemodProperty3;
            return STATUS_SUCCESS;
        }
        else
        {
            return STATUS_INVALID_PARAMETER;
        }
    }

    STDMETHODIMP_(NTSTATUS)
    GetStatus(
        PBDATUNER_DEVICE_STATUS     pDeviceStatus
        );

    STDMETHODIMP_(NTSTATUS)
    SetDeviceState(
        KSSTATE     newKsState
        )
    {
        m_KsState = newKsState;
        return STATUS_SUCCESS;
    };

    STDMETHODIMP_(NTSTATUS)
    AcquireResources();

    STDMETHODIMP_(NTSTATUS)
    ReleaseResources();

private:
    class CDevice * m_pDevice;

    //  Filter Properties
    //

    //  Filter Resources
    //
    KSSTATE                     m_KsState;
    BDA_CHANGE_STATE            m_BdaChangeState;
    BDATUNER_DEVICE_RESOURCE    m_CurResource;
    BDATUNER_DEVICE_RESOURCE    m_NewResource;
    ULONG                       m_ulResourceID;
    BOOLEAN                     m_fResourceAcquired;
};

//
// Define the device class.
//
class CDevice {
public:

    //
    //  Define the AVStream Device Dispatch Functions
    //  AVStream uses these functions to create and start the device
    //
    static
    STDMETHODIMP_(NTSTATUS)
    Create(
        IN PKSDEVICE    pKSDevice
        );

    static
    STDMETHODIMP_(NTSTATUS)
    Start(
        IN PKSDEVICE            pKSDevice,
        IN PIRP                 pIrp,
        IN PCM_RESOURCE_LIST    pTranslatedResourceList OPTIONAL,
        IN PCM_RESOURCE_LIST    pUntranslatedResourceList OPTIONAL
        );

    //
    //  Utility functions for the device
    //  An instance of the filter uses these functions 
    //  to manage resources on the device. 
    //

    STDMETHODIMP_(NTSTATUS)
    InitializeHW(
        );

    STDMETHODIMP_(NTSTATUS)
    GetStatus(
        PBDATUNER_DEVICE_STATUS     pDeviceStatus
        );

    STDMETHODIMP_(NTSTATUS)
    GetImplementationGUID(
        GUID *                      pguidImplementation
        )
    {
        if (pguidImplementation)
        {
            *pguidImplementation = m_guidImplemenation;
            return STATUS_SUCCESS;
        }
        else
        {
            return STATUS_INVALID_PARAMETER;
        }
    }

    STDMETHODIMP_(NTSTATUS)
    GetDeviceInstance(
        ULONG *                     pulDeviceInstance
        )
    {
        if (pulDeviceInstance)
        {
            *pulDeviceInstance = m_ulDeviceInstance;
            return STATUS_SUCCESS;
        }
        else
        {
            return STATUS_INVALID_PARAMETER;
        }
    }

    NTSTATUS
    AcquireResources(
        PBDATUNER_DEVICE_RESOURCE     pNewResource,
        PULONG                        pulAcquiredResourceID
        );

    NTSTATUS
    UpdateResources(
        PBDATUNER_DEVICE_RESOURCE     pNewResource,
        ULONG                         ulResourceID
        );

    NTSTATUS
    ReleaseResources(
        ULONG                   ulResourceID
        );


private:

    PKSDEVICE                 m_pKSDevice;

    GUID                      m_guidImplemenation;
    ULONG                     m_ulDeviceInstance;
    BDATUNER_DEVICE_RESOURCE  m_CurResource;
    ULONG                     m_ulCurResourceID; 
    ULONG                     m_ulcResourceUsers;
};


//
// Define the Input-pin class.
//
class CAntennaPin {
public:
    //
    //  Define the AVStream Pin Dispatch Functions
    //  Network provider and AVStream use these functions 
    //  to create or remove a pin instance or to change the pin's 
    //  state after the minidriver receives a connection state 
    //  property 'set' IOCTL. 
    //
    static
    STDMETHODIMP_(NTSTATUS)
    PinCreate(
        IN OUT PKSPIN Pin,
        IN PIRP Irp
        );

    static
    STDMETHODIMP_(NTSTATUS)
    PinClose(
        IN OUT PKSPIN Pin,
        IN PIRP Irp
        );

    static
    STDMETHODIMP_(NTSTATUS)
    PinSetDeviceState(
        IN PKSPIN Pin,
        IN KSSTATE ToState,
        IN KSSTATE FromState
        );

    //
    //  Define a data intersection handler function for the 
    //  pin (KSPIN_DESCRIPTOR_EX structure). 
    //  Network provider and AVStream use this function 
    //  to connect the input pin with an upstream filter.   
    //
    static
    STDMETHODIMP_(NTSTATUS)
    IntersectDataFormat(
        IN PVOID pContext,
        IN PIRP pIrp,
        IN PKSP_PIN Pin,
        IN PKSDATARANGE DataRange,
        IN PKSDATARANGE MatchingDataRange,
        IN ULONG DataBufferSize,
        OUT PVOID Data OPTIONAL,
        OUT PULONG DataSize
        );

    //
    //  Network provider and AVStream use these functions 
    //  to set and get properties of nodes that are controlled 
    //  by the input pin. 
    //
    static
    STDMETHODIMP_(NTSTATUS)
    GetCenterFrequency(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );

    static
    STDMETHODIMP_(NTSTATUS)
    PutCenterFrequency(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );

    static
    STDMETHODIMP_(NTSTATUS)
    GetSignalStatus(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );


    //
    //  Utility functions for the pin
    //
    STDMETHODIMP_(class CFilter *)
    GetFilter() { return m_pFilter;};

    STDMETHODIMP_(void)
    SetFilter(class CFilter * pFilter) { m_pFilter = pFilter;};

private:
    class CFilter*  m_pFilter;
    ULONG           ulReserved;
    KSSTATE         m_KsState;

    //  Node Properties
    //
    BOOLEAN         m_fResourceChanged;
    ULONG           m_ulCurrentFrequency;
    ULONG           m_ulPendingFrequency;
};


//
// Define the Output-pin class.
//
class CTransportPin{
public:
    //
    //  Define the AVStream Pin Dispatch Functions
    //  Network provider and AVStream use these functions 
    //  to create or remove a pin instance or to change the pin's 
    //  state after the minidriver receives a connection state 
    //  property 'set' IOCTL. 
    //
    static
    STDMETHODIMP_(NTSTATUS)
    PinCreate(
        IN OUT PKSPIN Pin,
        IN PIRP Irp
        );

    static
    STDMETHODIMP_(NTSTATUS)
    PinClose(
        IN OUT PKSPIN Pin,
        IN PIRP Irp
        );

    //
    //  Define a data intersection handler function for the 
    //  pin (KSPIN_DESCRIPTOR_EX structure). 
    //  Network provider and AVStream use this function 
    //  to connect the output pin with a downstream filter.   
    //
    static
    STDMETHODIMP_(NTSTATUS)
    IntersectDataFormat(
        IN PVOID pContext,
        IN PIRP pIrp,
        IN PKSP_PIN Pin,
        IN PKSDATARANGE DataRange,
        IN PKSDATARANGE MatchingDataRange,
        IN ULONG DataBufferSize,
        OUT PVOID Data OPTIONAL,
        OUT PULONG DataSize
        );

    //
    //  BDA Signal Properties
    //
    static
    STDMETHODIMP_(NTSTATUS)
    GetSignalStatus(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );

    static
    STDMETHODIMP_(NTSTATUS)
    PutAutoDemodProperty(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );

#if !ATSC_RECEIVER
    static
    STDMETHODIMP_(NTSTATUS)
    GetDigitalDemodProperty(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );

    static
    STDMETHODIMP_(NTSTATUS)
    PutDigitalDemodProperty(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );
#endif // !ATSC_RECEIVER

    static
    STDMETHODIMP_(NTSTATUS)
    GetExtensionProperties(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );

    static
    STDMETHODIMP_(NTSTATUS)
    PutExtensionProperties(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );

    STDMETHODIMP_(class CFilter *)
    GetFilter() { return m_pFilter;};

    STDMETHODIMP_(void)
    SetFilter(class CFilter * pFilter) { m_pFilter = pFilter;};

private:
    class CFilter*  m_pFilter;
    ULONG           ulReserved;
    KSSTATE         m_KsState;

    //  Node Properties
    //
    BOOLEAN         m_fResourceChanged;
    ULONG           m_ulCurrentProperty1;
    ULONG           m_ulPendingProperty1;

    ULONG           m_ulCurrentProperty2;

    ULONG           m_ulCurrentProperty3;
    ULONG           m_ulPendingProperty3;
};

//
//  Topology Constants
//
typedef enum {
    PIN_TYPE_ANTENNA = 0,
    PIN_TYPE_TRANSPORT
} FilterPinTypes;

typedef enum {
    INITIAL_ANNTENNA_PIN_ID = 0
} InitialPinIDs;

//
//  Data declarations
//

extern const BDA_FILTER_TEMPLATE    BdaFilterTemplate;
extern const KSFILTER_DESCRIPTOR    InitialFilterDescriptor;
extern const KSFILTER_DESCRIPTOR    TemplateFilterDescriptor;


