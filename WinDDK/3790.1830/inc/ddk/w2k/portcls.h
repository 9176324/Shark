/*****************************************************************************
 * portcls.h - WDM Streaming port class driver
 *****************************************************************************
 * Copyright (c) 1996-1998 Microsoft Corporation.  All Rights Reserved.
 */

#ifndef _PORTCLS_H_
#define _PORTCLS_H_

#ifdef __cplusplus
// WDM.H does not play well with C++.
extern "C"
{
#include <wdm.h>
}
#else
#include <wdm.h>
#endif

#ifndef IRP_MN_FILTER_RESOURCE_REQUIREMENTS
#define IRP_MN_FILTER_RESOURCE_REQUIREMENTS 0x0D
#endif

#include <windef.h>
#define NOBITMAP
#include <mmreg.h>
#undef NOBITMAP
#include "ks.h"
#include "ksmedia.h"
#include <punknown.h>

#define PORTCLASSAPI EXTERN_C

#define _100NS_UNITS_PER_SECOND 10000000L
#define PORT_CLASS_DEVICE_EXTENSION_SIZE 256


#ifndef PC_NEW_NAMES
#define PC_OLD_NAMES
#endif


/*****************************************************************************
 * Interface identifiers.
 */

DEFINE_GUID(IID_IMiniport,
0xb4c90a24, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IPort,
0xb4c90a25, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IResourceList,
0x22C6AC60L, 0x851B, 0x11D0, 0x9A, 0x7F, 0x00, 0xAA, 0x00, 0x38, 0xAC, 0xFE);
DEFINE_GUID(IID_IDmaChannel,
0x22C6AC61L, 0x851B, 0x11D0, 0x9A, 0x7F, 0x00, 0xAA, 0x00, 0x38, 0xAC, 0xFE);
DEFINE_GUID(IID_IDmaChannelSlave,
0x22C6AC62L, 0x851B, 0x11D0, 0x9A, 0x7F, 0x00, 0xAA, 0x00, 0x38, 0xAC, 0xFE);
DEFINE_GUID(IID_IInterruptSync,
0x22C6AC63L, 0x851B, 0x11D0, 0x9A, 0x7F, 0x00, 0xAA, 0x00, 0x38, 0xAC, 0xFE);
DEFINE_GUID(IID_IServiceSink,
0x22C6AC64L, 0x851B, 0x11D0, 0x9A, 0x7F, 0x00, 0xAA, 0x00, 0x38, 0xAC, 0xFE);
DEFINE_GUID(IID_IServiceGroup,
0x22C6AC65L, 0x851B, 0x11D0, 0x9A, 0x7F, 0x00, 0xAA, 0x00, 0x38, 0xAC, 0xFE);
DEFINE_GUID(IID_IRegistryKey,
0xE8DA4302, 0xF304, 0x11D0, 0x95, 0x8B, 0x00, 0xC0, 0x4F, 0xB9, 0x25, 0xD3);
DEFINE_GUID(IID_IPortMidi,
0xb4c90a40, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IMiniportMidi,
0xb4c90a41, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IMiniportMidiStream,
0xb4c90a42, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IPortTopology,
0xb4c90a30, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IMiniportTopology,
0xb4c90a31, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IPortWaveCyclic,
0xb4c90a26, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IMiniportWaveCyclic,
0xb4c90a27, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IMiniportWaveCyclicStream,
0xb4c90a28, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IPortWavePci,
0xb4c90a50, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IPortWavePciStream,
0xb4c90a51, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IMiniportWavePci,
0xb4c90a52, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IMiniportWavePciStream,
0xb4c90a53, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IAdapterPowerManagment,
0x793417D0, 0x35FE, 0x11D1, 0xAD, 0x08, 0x0, 0xA0, 0xC9, 0x0A, 0xB1, 0xB0);
DEFINE_GUID(IID_IPowerNotify,
0x3DD648B8, 0x969F, 0x11D1, 0x95, 0xA9, 0x00, 0xC0, 0x4F, 0xB9, 0x25, 0xD3);
DEFINE_GUID(IID_IWaveCyclicClock,
0xdec1ec78, 0x419a, 0x11d1, 0xad, 0x09, 0x0, 0xc0, 0x4f, 0xb9, 0x1b, 0xc4 );
DEFINE_GUID(IID_IWavePciClock,
0xd5d7a256, 0x5d10, 0x11d1, 0xad, 0xae, 0x0, 0xc0, 0x4f, 0xb9, 0x1b, 0xc4 );
DEFINE_GUID(IID_IPortEvents,
0xA80F29C4, 0x5498, 0x11D2, 0x95, 0xD9, 0x00, 0xC0, 0x4F, 0xB9, 0x25, 0xD3 );

/*****************************************************************************
 * Class identifiers.
 */

DEFINE_GUID(CLSID_PortMidi,
0xb4c90a43, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(CLSID_PortTopology,
0xb4c90a32, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(CLSID_PortWaveCyclic,
0xb4c90a2a, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(CLSID_PortWavePci,
0xb4c90a54, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(CLSID_MiniportDriverFmSynth,
0xb4c90ae0, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(CLSID_MiniportDriverUart,
0xb4c90ae1, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(CLSID_MiniportDriverFmSynthWithVol,     
0xe5a3c139, 0xf0f2, 0x11d1, 0x81, 0xaf, 0x0, 0x60, 0x8, 0x33, 0x16, 0xc1);


/*****************************************************************************
 * Interfaces
 */

/*****************************************************************************
 * IResourceList
 *****************************************************************************
 * List of resources.
 */
DECLARE_INTERFACE_(IResourceList,IUnknown)
{
    STDMETHOD_(ULONG,NumberOfEntries)
    (   THIS
    )   PURE;

    STDMETHOD_(ULONG,NumberOfEntriesOfType)
    (   THIS_
        IN      CM_RESOURCE_TYPE    Type
    )   PURE;

    STDMETHOD_(PCM_PARTIAL_RESOURCE_DESCRIPTOR,FindTranslatedEntry)
    (   THIS_
        IN      CM_RESOURCE_TYPE    Type,
        IN      ULONG               Index
    )   PURE;

    STDMETHOD_(PCM_PARTIAL_RESOURCE_DESCRIPTOR,FindUntranslatedEntry)
    (   THIS_
        IN      CM_RESOURCE_TYPE    Type,
        IN      ULONG               Index
    )   PURE;

    STDMETHOD(AddEntry)
    (   THIS_
        IN      PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated,
        IN      PCM_PARTIAL_RESOURCE_DESCRIPTOR Untranslated
    )   PURE;

    STDMETHOD(AddEntryFromParent)
    (   THIS_
        IN      struct IResourceList *  Parent,
        IN      CM_RESOURCE_TYPE        Type,
        IN      ULONG                   Index
    )   PURE;

    STDMETHOD_(PCM_RESOURCE_LIST,TranslatedList)
    (   THIS
    )   PURE;

    STDMETHOD_(PCM_RESOURCE_LIST,UntranslatedList)
    (   THIS
    )   PURE;
};

typedef IResourceList *PRESOURCELIST;

#ifdef PC_IMPLEMENTATION
#define IMP_IResourceList\
    STDMETHODIMP_(ULONG)NumberOfEntries\
    (   void\
    );\
    STDMETHODIMP_(ULONG) NumberOfEntriesOfType\
    (   IN      CM_RESOURCE_TYPE    Type\
    );\
    STDMETHODIMP_(PCM_PARTIAL_RESOURCE_DESCRIPTOR) FindTranslatedEntry\
    (   IN      CM_RESOURCE_TYPE    Type,\
        IN      ULONG               Index\
    );\
    STDMETHODIMP_(PCM_PARTIAL_RESOURCE_DESCRIPTOR) FindUntranslatedEntry\
    (   IN      CM_RESOURCE_TYPE    Type,\
        IN      ULONG               Index\
    );\
    STDMETHODIMP AddEntry\
    (   IN      PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated,\
        IN      PCM_PARTIAL_RESOURCE_DESCRIPTOR Untranslated\
    );\
    STDMETHODIMP AddEntryFromParent\
    (   IN      struct IResourceList *  Parent,\
        IN      CM_RESOURCE_TYPE        Type,\
        IN      ULONG                   Index\
    );\
    STDMETHODIMP_(PCM_RESOURCE_LIST) TranslatedList\
    (   void\
    );\
    STDMETHODIMP_(PCM_RESOURCE_LIST) UntranslatedList\
    (   void\
    )
#endif


#define NumberOfPorts()         NumberOfEntriesOfType(CmResourceTypePort)
#define FindTranslatedPort(n)   FindTranslatedEntry(CmResourceTypePort,(n))
#define FindUntranslatedPort(n) FindUntranslatedEntry(CmResourceTypePort,(n))
#define AddPortFromParent(p,n)  AddEntryFromParent((p),CmResourceTypePort,(n))

#define NumberOfInterrupts()         NumberOfEntriesOfType(CmResourceTypeInterrupt)
#define FindTranslatedInterrupt(n)   FindTranslatedEntry(CmResourceTypeInterrupt,(n))
#define FindUntranslatedInterrupt(n) FindUntranslatedEntry(CmResourceTypeInterrupt,(n))
#define AddInterruptFromParent(p,n)  AddEntryFromParent((p),CmResourceTypeInterrupt,(n))

#define NumberOfMemories()        NumberOfEntriesOfType(CmResourceTypeMemory)
#define FindTranslatedMemory(n)   FindTranslatedEntry(CmResourceTypeMemory,(n))
#define FindUntranslatedMemory(n) FindUntranslatedEntry(CmResourceTypeMemory,(n))
#define AddMemoryFromParent(p,n)  AddEntryFromParent((p),CmResourceTypeMemory,(n))

#define NumberOfDmas()         NumberOfEntriesOfType(CmResourceTypeDma)
#define FindTranslatedDma(n)   FindTranslatedEntry(CmResourceTypeDma,(n))
#define FindUntranslatedDma(n) FindUntranslatedEntry(CmResourceTypeDma,(n))
#define AddDmaFromParent(p,n)  AddEntryFromParent((p),CmResourceTypeDma,(n))

#define NumberOfDeviceSpecifics()         NumberOfEntriesOfType(CmResourceTypeDeviceSpecific)
#define FindTranslatedDeviceSpecific(n)   FindTranslatedEntry(CmResourceTypeDeviceSpecific,(n))
#define FindUntranslatedDeviceSpecific(n) FindUntranslatedEntry(CmResourceTypeDeviceSpecific,(n))
#define AddDeviceSpecificFromParent(p,n)  AddEntryFromParent((p),CmResourceTypeDeviceSpecific,(n))

#define NumberOfBusNumbers()         NumberOfEntriesOfType(CmResourceTypeBusNumber)
#define FindTranslatedBusNumber(n)   FindTranslatedEntry(CmResourceTypeBusNumber,(n))
#define FindUntranslatedBusNumber(n) FindUntranslatedEntry(CmResourceTypeBusNumber,(n))
#define AddBusNumberFromParent(p,n)  AddEntryFromParent((p),CmResourceTypeBusNumber,(n))

#define NumberOfDevicePrivates()         NumberOfEntriesOfType(CmResourceTypeDevicePrivate)
#define FindTranslatedDevicePrivate(n)   FindTranslatedEntry(CmResourceTypeDevicePrivate,(n))
#define FindUntranslatedDevicePrivate(n) FindUntranslatedEntry(CmResourceTypeDevicePrivate,(n))
#define AddDevicePrivateFromParent(p,n)  AddEntryFromParent((p),CmResourceTypeDevicePrivate,(n))

#define NumberOfAssignedResources()         NumberOfEntriesOfType(CmResourceTypeAssignedResource)
#define FindTranslatedAssignedResource(n)   FindTranslatedEntry(CmResourceTypeAssignedResource,(n))
#define FindUntranslatedAssignedResource(n) FindUntranslatedEntry(CmResourceTypeAssignedResource,(n))
#define AddAssignedResourceFromParent(p,n)  AddEntryFromParent((p),CmResourceTypeAssignedResource,(n))

#define NumberOfSubAllocateFroms()         NumberOfEntriesOfType(CmResourceTypeSubAllocateFrom)
#define FindTranslatedSubAllocateFrom(n)   FindTranslatedEntry(CmResourceTypeSubAllocateFrom,(n))
#define FindUntranslatedSubAllocateFrom(n) FindUntranslatedEntry(CmResourceTypeSubAllocateFrom,(n))
#define AddSubAllocateFromFromParent(p,n)  AddEntryFromParent((p),CmResourceTypeSubAllocateFrom,(n))

/*****************************************************************************
 * IDmaChannel
 *****************************************************************************
 * Interface for DMA channel.
 */
DECLARE_INTERFACE_(IDmaChannel,IUnknown)
{
    STDMETHOD(AllocateBuffer)
    (   THIS_
        IN      ULONG               BufferSize,
        IN      PPHYSICAL_ADDRESS   PhysicalAddressConstraint   OPTIONAL
    )   PURE;

    STDMETHOD_(void,FreeBuffer)
    (   THIS
    )   PURE;

    STDMETHOD_(ULONG,TransferCount)
    (   THIS
    )   PURE;

    STDMETHOD_(ULONG,MaximumBufferSize)
    (   THIS
    )   PURE;

    STDMETHOD_(ULONG,AllocatedBufferSize)
    (   THIS
    )   PURE;

    STDMETHOD_(ULONG,BufferSize)
    (   THIS
    )   PURE;

    STDMETHOD_(void,SetBufferSize)
    (   THIS_
        IN      ULONG   BufferSize
    )   PURE;

    STDMETHOD_(PVOID,SystemAddress)
    (   THIS
    )   PURE;

    STDMETHOD_(PHYSICAL_ADDRESS,PhysicalAddress)
    (   THIS
    )   PURE;

    STDMETHOD_(PADAPTER_OBJECT,GetAdapterObject)
    (   THIS
    )   PURE;

    STDMETHOD_(void,CopyTo)
    (   THIS_
        IN      PVOID   Destination,
        IN      PVOID   Source,
        IN      ULONG   ByteCount
    )   PURE;

    STDMETHOD_(void,CopyFrom)
    (   THIS_
        IN      PVOID   Destination,
        IN      PVOID   Source,
        IN      ULONG   ByteCount
    )   PURE;
};

typedef IDmaChannel *PDMACHANNEL;

#ifdef PC_IMPLEMENTATION
#define IMP_IDmaChannel\
    STDMETHODIMP AllocateBuffer\
    (   IN      ULONG               BufferSize,\
        IN      PPHYSICAL_ADDRESS   PhysicalAddressConstraint   OPTIONAL\
    );\
    STDMETHODIMP_(void) FreeBuffer\
    (   void\
    );\
    STDMETHODIMP_(ULONG) TransferCount\
    (   void\
    );\
    STDMETHODIMP_(ULONG) MaximumBufferSize\
    (   void\
    );\
    STDMETHODIMP_(ULONG) AllocatedBufferSize\
    (   void\
    );\
    STDMETHODIMP_(ULONG) BufferSize\
    (   void\
    );\
    STDMETHODIMP_(void) SetBufferSize\
    (   IN      ULONG   BufferSize\
    );\
    STDMETHODIMP_(PVOID) SystemAddress\
    (   void\
    );\
    STDMETHODIMP_(PHYSICAL_ADDRESS) PhysicalAddress\
    (   void\
    );\
    STDMETHODIMP_(PADAPTER_OBJECT) GetAdapterObject\
    (   void\
    );\
    STDMETHODIMP_(void) CopyTo\
    (   IN      PVOID   Destination,\
        IN      PVOID   Source,\
        IN      ULONG   ByteCount\
    );\
    STDMETHODIMP_(void) CopyFrom\
    (   IN      PVOID   Destination,\
        IN      PVOID   Source,\
        IN      ULONG   ByteCount\
    )
#endif

/*****************************************************************************
 * IDmaChannelSlave
 *****************************************************************************
 * Interface for slave DMA channel.
 */
DECLARE_INTERFACE_(IDmaChannelSlave,IDmaChannel)
{
    STDMETHOD(Start)
    (   THIS_
        IN      ULONG               MapSize,
        IN      BOOLEAN             WriteToDevice
    )   PURE;

    STDMETHOD(Stop)
    (   THIS
    )   PURE;

    STDMETHOD_(ULONG,ReadCounter)
    (   THIS
    )   PURE;

    STDMETHOD(WaitForTC)
    (   THIS_
        ULONG Timeout
    )   PURE;
};

typedef IDmaChannelSlave *PDMACHANNELSLAVE;

#ifdef PC_IMPLEMENTATION
#define IMP_IDmaChannelSlave\
    IMP_IDmaChannel;\
    STDMETHODIMP Start\
    (   IN      ULONG               MapSize,\
        IN      BOOLEAN             WriteToDevice\
    );\
    STDMETHODIMP Stop\
    (   void\
    );\
    STDMETHODIMP_(ULONG) ReadCounter\
    (   void\
    );\
    STDMETHODIMP WaitForTC\
    (   ULONG Timeout\
    )
#endif

/*****************************************************************************
 * INTERRUPTSYNCMODE
 *****************************************************************************
 * Interrupt sync mode of operation.
 */
typedef enum
{
    InterruptSyncModeNormal = 1,    // One pass, stop when successful.
    InterruptSyncModeAll,           // One pass regardless of success.
    InterruptSyncModeRepeat         // Repeat until all return unsuccessful.
} INTERRUPTSYNCMODE;

/*****************************************************************************
 * PINTERRUPTSYNCROUTINE
 *****************************************************************************
 * Pointer to an interrupt synchronization routine.  Both interrupt service
 * routines and routines that are synchronized with ISRs use this type.
 */
typedef NTSTATUS
(*PINTERRUPTSYNCROUTINE)
(
    IN      struct IInterruptSync * InterruptSync,
    IN      PVOID                   DynamicContext
);

/*****************************************************************************
 * IInterruptSync
 *****************************************************************************
 * Interface for objects providing access synchronization with interrupts.
 */
DECLARE_INTERFACE_(IInterruptSync,IUnknown)
{
    STDMETHOD(CallSynchronizedRoutine)
    (   THIS_
        IN      PINTERRUPTSYNCROUTINE   Routine,
        IN      PVOID                   DynamicContext
    )   PURE;
    STDMETHOD_(PKINTERRUPT,GetKInterrupt)
    (   THIS
    )   PURE;
    STDMETHOD(Connect)
    (   THIS
    )   PURE;
    STDMETHOD_(void,Disconnect)
    (   THIS
    )   PURE;
    STDMETHOD(RegisterServiceRoutine)
    (   THIS_
        IN      PINTERRUPTSYNCROUTINE   Routine,
        IN      PVOID                   DynamicContext,
        IN      BOOLEAN                 First
    )   PURE;
};

typedef IInterruptSync *PINTERRUPTSYNC;

#ifdef PC_IMPLEMENTATION
#define IMP_IInterruptSync\
    STDMETHODIMP CallSynchronizedRoutine\
    (   IN      PINTERRUPTSYNCROUTINE   Routine,\
        IN      PVOID                   DynamicContext\
    );\
    STDMETHODIMP_(PKINTERRUPT) GetKInterrupt\
    (   void\
    );\
    STDMETHODIMP Connect\
    (   void\
    );\
    STDMETHODIMP_(void) Disconnect\
    (   void\
    );\
    STDMETHODIMP RegisterServiceRoutine\
    (   IN      PINTERRUPTSYNCROUTINE   Routine,\
        IN      PVOID                   DynamicContext,\
        IN      BOOLEAN                 First\
    )
#endif

/*****************************************************************************
 * IServiceSink
 *****************************************************************************
 * Interface for notification sinks for service groups.
 */
DECLARE_INTERFACE_(IServiceSink,IUnknown)
{
    STDMETHOD_(void,RequestService)
    (   THIS
    )   PURE;
};

typedef IServiceSink *PSERVICESINK;

#ifdef PC_IMPLEMENTATION
#define IMP_IServiceSink\
    STDMETHODIMP_(void) RequestService\
    (   void\
    )
#endif

/*****************************************************************************
 * IServiceGroup
 *****************************************************************************
 * Interface for objects representing a group that is serviced collectively.
 */
DECLARE_INTERFACE_(IServiceGroup,IServiceSink)
{
    STDMETHOD(AddMember)
    (   THIS_
        IN      PSERVICESINK    pServiceSink
    )   PURE;

    STDMETHOD_(void,RemoveMember)
    (   THIS_
        IN      PSERVICESINK    pServiceSink
    )   PURE;

    STDMETHOD_(void,SupportDelayedService)
    (   THIS
    )   PURE;

    STDMETHOD_(void,RequestDelayedService)
    (   THIS_
        IN      ULONGLONG   ullDelay
    )   PURE;

    STDMETHOD_(void,CancelDelayedService)
    (   THIS
    )   PURE;
};

typedef IServiceGroup *PSERVICEGROUP;

#ifdef PC_IMPLEMENTATION
#define IMP_IServiceGroup\
    IMP_IServiceSink;\
    STDMETHODIMP AddMember\
    (   IN  PSERVICESINK    pServiceSink\
    );\
    STDMETHODIMP_(void) RemoveMember\
    (   IN  PSERVICESINK    pServiceSink\
    );\
    STDMETHODIMP_(void) SupportDelayedService\
    (   void\
    );\
    STDMETHODIMP_(void) RequestDelayedService\
    (   IN  ULONGLONG   ullDelay\
    );\
    STDMETHODIMP_(void) CancelDelayedService\
    (   void\
    )
#endif

/*****************************************************************************
 * IRegistryKey
 *****************************************************************************
 * Interface for objects providing access to a registry key.
 */
DECLARE_INTERFACE_(IRegistryKey,IUnknown)
{
    STDMETHOD(QueryKey)
    (   THIS_
        IN      KEY_INFORMATION_CLASS   KeyInformationClass,
        OUT     PVOID                   KeyInformation,
        IN      ULONG                   Length,
        OUT     PULONG                  ResultLength
    )   PURE;

    STDMETHOD(EnumerateKey)
    (   THIS_
        IN      ULONG                   Index,
        IN      KEY_INFORMATION_CLASS   KeyInformationClass,
        OUT     PVOID                   KeyInformation,
        IN      ULONG                   Length,
        OUT     PULONG                  ResultLength
    )   PURE;

    STDMETHOD(QueryValueKey)
    (   THIS_
        IN      PUNICODE_STRING             ValueName,
        IN      KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
        OUT     PVOID                       KeyValueInformation,
        IN      ULONG                       Length,
        OUT     PULONG                      ResultLength
    )   PURE;

    STDMETHOD(EnumerateValueKey)
    (   THIS_
        IN      ULONG                       Index,
        IN      KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
        OUT     PVOID                       KeyValueInformation,
        IN      ULONG                       Length,
        OUT     PULONG                      ResultLength
    )   PURE;

    STDMETHOD(SetValueKey)
    (   THIS_
        IN      PUNICODE_STRING     ValueName OPTIONAL,
        IN      ULONG               Type,
        IN      PVOID               Data,
        IN      ULONG               DataSize
    )   PURE;

    STDMETHOD(QueryRegistryValues)
    (   THIS_
        IN      PRTL_QUERY_REGISTRY_TABLE   QueryTable,
        IN      PVOID                       Context OPTIONAL
    )   PURE;

    STDMETHOD(NewSubKey)
    (   THIS_
        OUT     IRegistryKey **     RegistrySubKey,
        IN      PUNKNOWN            OuterUnknown,
        IN      ACCESS_MASK         DesiredAccess,
        IN      PUNICODE_STRING     SubKeyName,
        IN      ULONG               CreateOptions,
        OUT     PULONG              Disposition     OPTIONAL
    )   PURE;

    STDMETHOD(DeleteKey)
    (   THIS
    )   PURE;
};

typedef IRegistryKey *PREGISTRYKEY;

#ifdef PC_IMPLEMENTATION
#define IMP_IRegistryKey\
    STDMETHODIMP QueryKey\
    (   IN      KEY_INFORMATION_CLASS   KeyInformationClass,\
        OUT     PVOID                   KeyInformation,\
        IN      ULONG                   Length,\
        OUT     PULONG                  ResultLength\
    );\
    STDMETHODIMP EnumerateKey\
    (   IN      ULONG                   Index,\
        IN      KEY_INFORMATION_CLASS   KeyInformationClass,\
        OUT     PVOID                   KeyInformation,\
        IN      ULONG                   Length,\
        OUT     PULONG                  ResultLength\
    );\
    STDMETHODIMP QueryValueKey\
    (   IN      PUNICODE_STRING             ValueName,\
        IN      KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,\
        OUT     PVOID                       KeyValueInformation,\
        IN      ULONG                       Length,\
        OUT     PULONG                      ResultLength\
    );\
    STDMETHODIMP EnumerateValueKey\
    (   IN      ULONG                       Index,\
        IN      KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,\
        OUT     PVOID                       KeyValueInformation,\
        IN      ULONG                       Length,\
        OUT     PULONG                      ResultLength\
    );\
    STDMETHODIMP SetValueKey\
    (   IN      PUNICODE_STRING     ValueName OPTIONAL,\
        IN      ULONG               Type,\
        IN      PVOID               Data,\
        IN      ULONG               DataSize\
    );\
    STDMETHODIMP QueryRegistryValues\
    (   IN      PRTL_QUERY_REGISTRY_TABLE   QueryTable,\
        IN      PVOID                       Context OPTIONAL\
    );\
    STDMETHODIMP NewSubKey\
    (   OUT     IRegistryKey **     RegistrySubKey,\
        IN      PUNKNOWN            OuterUnknown,\
        IN      ACCESS_MASK         DesiredAccess,\
        IN      PUNICODE_STRING     SubKeyName,\
        IN      ULONG               CreateOptions,\
        OUT     PULONG              Disposition     OPTIONAL\
    );\
    STDMETHODIMP DeleteKey\
    (   void\
    )
#endif

typedef struct _PCPROPERTY_REQUEST PCPROPERTY_REQUEST, *PPCPROPERTY_REQUEST;
typedef struct _PCMETHOD_REQUEST PCMETHOD_REQUEST, *PPCMETHOD_REQUEST;
typedef struct _PCEVENT_REQUEST PCEVENT_REQUEST, *PPCEVENT_REQUEST;

/*****************************************************************************
 * PCPFNPROPERTY_HANDLER
 *****************************************************************************
 * Property handler function prototype.
 *
 * All property accesses and support queries for a given property on a given
 * filter, pin or node are routed to a single handler.  The parameter contains
 * complete information regarding the request.  The handler may return 
 * STATUS_PENDING, in which case it must eventually call 
 * PcCompletePendingPropertyRequest() to complete the request.
 */
typedef 
NTSTATUS 
(*PCPFNPROPERTY_HANDLER)
(
    IN      PPCPROPERTY_REQUEST PropertyRequest
);

/*****************************************************************************
 * PCPFNMETHOD_HANDLER
 *****************************************************************************
 * Method handler function prototype.
 *
 * All method calls and support queries for a given method on a given filter,
 * pin or node are routed to a single handler.  The parameter contains 
 * complete information regarding the request.  The handler may return 
 * STATUS_PENDING, in which case it must eventually call 
 * PcCompletePendingMethodRequest() to complete the request.
 */
typedef 
NTSTATUS 
(*PCPFNMETHOD_HANDLER)
(
    IN      PPCMETHOD_REQUEST   MethodRequest
);

/*****************************************************************************
 * PCPFNEVENT_HANDLER
 *****************************************************************************
 * Event handler function prototype.
 *
 * All event add and remove requests and all event support queries for a 
 * given event on a given filter, pin or node are routed to a single handler.
 * The parameter contains complete information regarding the request.  The 
 * handler may return STATUS_PENDING, in which case it must eventually call 
 * PcCompletePendingEventRequest() to complete the request.
 */
typedef 
NTSTATUS 
(*PCPFNEVENT_HANDLER)
(
    IN      PPCEVENT_REQUEST    EventRequest
);

/*****************************************************************************
 * PCPROPERTY_ITEM
 *****************************************************************************
 * Property table entry.
 *
 * A property item describes a property supported by a given filter, pin or
 * node.  The flags indicate what operations regarding the property are
 * supported and specify selected options with respect to the port's handling
 * of property requests.
 */
typedef struct
{
    const GUID *            Set;
    ULONG                   Id;
    ULONG                   Flags;
#define PCPROPERTY_ITEM_FLAG_GET            KSPROPERTY_TYPE_GET             
#define PCPROPERTY_ITEM_FLAG_SET            KSPROPERTY_TYPE_SET             
#define PCPROPERTY_ITEM_FLAG_BASICSUPPORT   KSPROPERTY_TYPE_BASICSUPPORT    
//not supported #define PCPROPERTY_ITEM_FLAG_RELATIONS      KSPROPERTY_TYPE_RELATIONS       
#define PCPROPERTY_ITEM_FLAG_SERIALIZERAW   KSPROPERTY_TYPE_SERIALIZERAW    
#define PCPROPERTY_ITEM_FLAG_UNSERIALIZERAW KSPROPERTY_TYPE_UNSERIALIZERAW  
#define PCPROPERTY_ITEM_FLAG_SERIALIZESIZE  KSPROPERTY_TYPE_SERIALIZESIZE   
#define PCPROPERTY_ITEM_FLAG_SERIALIZE\
	(PCPROPERTY_ITEM_FLAG_SERIALIZERAW\
	|PCPROPERTY_ITEM_FLAG_UNSERIALIZERAW\
	|PCPROPERTY_ITEM_FLAG_SERIALIZESIZE\
	)
#define PCPROPERTY_ITEM_FLAG_DEFAULTVALUES  KSPROPERTY_TYPE_DEFAULTVALUES   
    PCPFNPROPERTY_HANDLER   Handler;
}
PCPROPERTY_ITEM, *PPCPROPERTY_ITEM;

/*****************************************************************************
 * PCMETHOD_ITEM
 *****************************************************************************
 * Method table entry.
 *
 * A method item describes a method supported by a given filter, pin or node.
 * The flags indicate what operations regarding the method are supported and 
 * specify selected options with respect to the port's handling of method
 * requests.
 */
typedef struct
{
    const GUID *            Set;
    ULONG                   Id;
    ULONG                   Flags;
#define PCMETHOD_ITEM_FLAG_NONE         KSMETHOD_TYPE_NONE
#define PCMETHOD_ITEM_FLAG_READ         KSMETHOD_TYPE_READ
#define PCMETHOD_ITEM_FLAG_WRITE        KSMETHOD_TYPE_WRITE
#define PCMETHOD_ITEM_FLAG_MODIFY       KSMETHOD_TYPE_MODIFY
#define PCMETHOD_ITEM_FLAG_SOURCE       KSMETHOD_TYPE_SOURCE
#define PCMETHOD_ITEM_FLAG_BASICSUPPORT KSMETHOD_TYPE_BASICSUPPORT
    PCPFNMETHOD_HANDLER     Handler;
}
PCMETHOD_ITEM, *PPCMETHOD_ITEM;

/*****************************************************************************
 * PCEVENT_ITEM
 *****************************************************************************
 * Event table entry.
 *
 * An event item describes an event supported by a given filter, pin or node.
 * The flags indicate what operations regarding the event are supported and 
 * specify selected options with respect to the port's handling of event
 * requests.
 */
typedef struct
{
    const GUID *            Set;
    ULONG                   Id;
    ULONG                   Flags;
#define PCEVENT_ITEM_FLAG_ENABLE        KSEVENT_TYPE_ENABLE
#define PCEVENT_ITEM_FLAG_ONESHOT       KSEVENT_TYPE_ONESHOT
#define PCEVENT_ITEM_FLAG_BASICSUPPORT  KSEVENT_TYPE_BASICSUPPORT
    PCPFNEVENT_HANDLER      Handler;
}
PCEVENT_ITEM, *PPCEVENT_ITEM;

/*****************************************************************************
 * PCPROPERTY_REQUEST
 *****************************************************************************
 * Property request submitted to a property handler.
 *
 * This is the form that a property request takes.  Although the major target
 * is generic, in the case of miniports, it will be a pointer to the miniport
 * object.  Likewise, the minor target is the stream or voice if the request
 * is specific to a stream or voice.  Otherwise, the minor target is NULL.
 * If the request is targeted at a node, the Node parameter will specify which
 * one, otherwise it will be ULONG(-1).  If the target is a node, the minor
 * target may be specified to indicate the stream or voice with which the 
 * targeted node instance is associated.
 */
typedef struct _PCPROPERTY_REQUEST
{
    PUNKNOWN                MajorTarget;
    PUNKNOWN                MinorTarget;
    ULONG                   Node;
    const PCPROPERTY_ITEM * PropertyItem;
    ULONG                   Verb;
    ULONG                   InstanceSize;
    PVOID                   Instance;
    ULONG                   ValueSize;
    PVOID                   Value;
    PIRP                    Irp;
}
PCPROPERTY_REQUEST, *PPCPROPERTY_REQUEST;

/*****************************************************************************
 * PCMETHOD_REQUEST
 *****************************************************************************
 * Method request submitted to a property handler.
 *
 * Comments in the description of PCPROPERTY_REQUEST regarding the target
 * fields apply to this structure as well.
 */
typedef struct _PCMETHOD_REQUEST
{
    PUNKNOWN                MajorTarget;
    PUNKNOWN                MinorTarget;
    ULONG                   Node;
    const PCMETHOD_ITEM *   MethodItem;
    ULONG                   Verb;
    // TODO
}
PCMETHOD_REQUEST, *PPCMETHOD_REQUEST;

/*****************************************************************************
 * PCEVENT_REQUEST
 *****************************************************************************
 * Event request submitted to a property handler.
 *
 * Comments in the description of PCPROPERTY_REQUEST regarding the target
 * fields apply to this structure as well.
 */
typedef struct _PCEVENT_REQUEST
{
    PUNKNOWN                MajorTarget;
    PUNKNOWN                MinorTarget;
    ULONG                   Node;
    const PCEVENT_ITEM *    EventItem;
    PKSEVENT_ENTRY          EventEntry;
    ULONG                   Verb;
    PIRP                    Irp;
}
PCEVENT_REQUEST, *PPCEVENT_REQUEST;

#define PCEVENT_VERB_NONE          0
#define PCEVENT_VERB_ADD           1
#define PCEVENT_VERB_REMOVE        2
#define PCEVENT_VERB_SUPPORT       4

/*****************************************************************************
 * PCAUTOMATION_TABLE
 *****************************************************************************
 * Master table of properties, methods and events.
 *
 * Any of the item pointers may be NULL, in which case, corresponding counts
 * must be zero.  For item tables that are not zero length, the item size must
 * not be smaller than the size of the item structure defined by port class.
 * The item size may be larger, in which case the port class item structure is
 * assumed to be followed by private data.  Item sizes must be a multiple of
 * 8.
 */
typedef struct
{
    ULONG			        PropertyItemSize;
    ULONG			        PropertyCount;
    const PCPROPERTY_ITEM * Properties;
	ULONG			        MethodItemSize;
	ULONG			        MethodCount;
	const PCMETHOD_ITEM *	Methods;
	ULONG			        EventItemSize;
	ULONG			        EventCount;
	const PCEVENT_ITEM *    Events;
    ULONG                   Reserved;
}
PCAUTOMATION_TABLE, *PPCAUTOMATION_TABLE;

#define DEFINE_PCAUTOMATION_TABLE_PROP(AutomationTable,PropertyTable)\
const PCAUTOMATION_TABLE AutomationTable =\
{\
    sizeof(PropertyTable[0]),\
    SIZEOF_ARRAY(PropertyTable),\
    (const PCPROPERTY_ITEM *) PropertyTable,\
    0,0,NULL,\
    0,0,NULL,\
    0\
}

#define DEFINE_PCAUTOMATION_TABLE_PROP_EVENT(AutomationTable,PropertyTable,EventTable)\
const PCAUTOMATION_TABLE AutomationTable =\
{\
    sizeof(PropertyTable[0]),\
    SIZEOF_ARRAY(PropertyTable),\
    (const PCPROPERTY_ITEM *) PropertyTable,\
    0,0,NULL,\
    sizeof(EventTable[0]),\
    SIZEOF_ARRAY(EventTable),\
    (const PCEVENT_ITEM *) EventTable,\
    0\
}

/*****************************************************************************
 * PCPIN_DESCRIPTOR for IMiniport::GetDescription()
 *****************************************************************************
 * Description of a pin on the filter implemented by the miniport.
 *
 * MaxGlobalInstanceCount and MaxFilterInstanceCount may be zero to indicate
 * that the pin may not be instantiated, ULONG(-1) to indicate the pin may be
 * allocated any number of times, or any other value to indicate a specific 
 * number of times the pin may be allocated.  MinFilterInstanceCount may not
 * be ULONG(-1) because it specifies a definite lower bound on the number of
 * instances of a pin that must exist in order for a filter to function.
 *
 * The KS pin descriptor may have zero interfaces and zero mediums.  The list
 * of interfaces is ignored in all cases.  The medium list will default to
 * a list containing only the standard medium (device I/O).
 *
 * The automation table pointer may be NULL indicating that no automation is
 * supported.
 */
typedef struct
{
    ULONG                       MaxGlobalInstanceCount;
    ULONG                       MaxFilterInstanceCount;
    ULONG                       MinFilterInstanceCount;
    const PCAUTOMATION_TABLE *  AutomationTable;
    KSPIN_DESCRIPTOR            KsPinDescriptor;
} 
PCPIN_DESCRIPTOR, *PPCPIN_DESCRIPTOR;

/*****************************************************************************
 * PCNODE_DESCRIPTOR for IMiniport::GetDescription()
 *****************************************************************************
 * Description of a node in the filter implemented by the miniport.
 *
 * The automation table pointer may be NULL indicating that no automation is
 * supported.  The name GUID pointer may be NULL indicating that the type GUID
 * should be used to determine the node name.
 */
typedef struct
{
    ULONG                       Flags;
	const PCAUTOMATION_TABLE *  AutomationTable;
	const GUID *                Type;
	const GUID *                Name;
} 
PCNODE_DESCRIPTOR, *PPCNODE_DESCRIPTOR;

/*****************************************************************************
 * PCCONNECTION_DESCRIPTOR for IMiniport::GetDescription()
 *****************************************************************************
 * Description of a node connection in the topology of the filter implemented
 * by the miniport.
 */
typedef KSTOPOLOGY_CONNECTION 
PCCONNECTION_DESCRIPTOR, *PPCCONNECTION_DESCRIPTOR;

/*****************************************************************************
 * PCFILTER_DESCRIPTOR for IMiniport::GetDescription()
 *****************************************************************************
 * Description of the of the filter implemented by a miniport, including 
 * pins, nodes, connections and properties.
 *
 * The version number should be zero.
 */
typedef struct
{
    ULONG                           Version;
	const PCAUTOMATION_TABLE *      AutomationTable;
    ULONG                           PinSize;
    ULONG                           PinCount;
    const PCPIN_DESCRIPTOR *        Pins;
    ULONG                           NodeSize;
    ULONG                           NodeCount;
    const PCNODE_DESCRIPTOR *       Nodes;
    ULONG                           ConnectionCount;
    const PCCONNECTION_DESCRIPTOR * Connections;
    ULONG                           CategoryCount;
    const GUID *                    Categories;
}
PCFILTER_DESCRIPTOR, *PPCFILTER_DESCRIPTOR;

/*****************************************************************************
 * PCFILTER_NODE for IMiniport::GetTopology()
 *****************************************************************************
 * The placeholder for the FromNode or ToNode fields in connections which
 * describe connections to the filter's pins.
 */
#define PCFILTER_NODE KSFILTER_NODE

/*****************************************************************************
 * IMiniport
 *****************************************************************************
 * Interface common to all miniports.
 */
DECLARE_INTERFACE_(IMiniport,IUnknown)
{
    STDMETHOD(GetDescription)
    (   THIS_
        OUT     PPCFILTER_DESCRIPTOR *  Description
    )   PURE;

    STDMETHOD(DataRangeIntersection)
    (   THIS_
        IN      ULONG           PinId,
        IN      PKSDATARANGE    DataRange,
        IN      PKSDATARANGE    MatchingDataRange,
        IN      ULONG           OutputBufferLength,
        OUT     PVOID           ResultantFormat     OPTIONAL,
        OUT     PULONG          ResultantFormatLength
    )   PURE;
};

typedef IMiniport *PMINIPORT;

#define IMP_IMiniport\
    STDMETHODIMP GetDescription\
    (   OUT     PPCFILTER_DESCRIPTOR *  Description\
    );\
    STDMETHODIMP DataRangeIntersection\
    (   IN      ULONG           PinId,\
        IN      PKSDATARANGE    DataRange,\
        IN      PKSDATARANGE    MatchingDataRange,\
        IN      ULONG           OutputBufferLength,\
        OUT     PVOID           ResultantFormat     OPTIONAL,\
        OUT     PULONG          ResultantFormatLength\
    )

/*****************************************************************************
 * IPort
 *****************************************************************************
 * Interface common to all port lower edges.
 */
DECLARE_INTERFACE_(IPort,IUnknown)
{
    STDMETHOD(Init)
    (   THIS_
#ifdef PC_OLD_NAMES
        IN      PVOID           DeviceObject,
        IN      PVOID           Irp,
#else
        IN      PDEVICE_OBJECT  DeviceObject,
        IN      PIRP            Irp,
#endif
        IN      PUNKNOWN        UnknownMiniport,
        IN      PUNKNOWN        UnknownAdapter      OPTIONAL,
        IN      PRESOURCELIST   ResourceList
    )   PURE;

    STDMETHOD(GetDeviceProperty)
    (   THIS_
        IN      DEVICE_REGISTRY_PROPERTY    DeviceProperty,
        IN      ULONG                       BufferLength,
        OUT     PVOID                       PropertyBuffer,
        OUT     PULONG                      ResultLength
    )   PURE;

    STDMETHOD(NewRegistryKey)
    (   THIS_
        OUT     PREGISTRYKEY *      OutRegistryKey,
        IN      PUNKNOWN            OuterUnknown        OPTIONAL,
        IN      ULONG               RegistryKeyType,
        IN      ACCESS_MASK         DesiredAccess,
        IN      POBJECT_ATTRIBUTES  ObjectAttributes    OPTIONAL,
        IN      ULONG               CreateOptions       OPTIONAL,
        OUT     PULONG              Disposition         OPTIONAL
    )   PURE;
};

typedef IPort *PPORT;

#ifdef PC_IMPLEMENTATION
#define IMP_IPort\
    STDMETHODIMP Init\
    (   IN      PDEVICE_OBJECT  DeviceObject,\
        IN      PIRP            Irp,\
        IN      PUNKNOWN        UnknownMiniport,\
        IN      PUNKNOWN        UnknownAdapter      OPTIONAL,\
        IN      PRESOURCELIST   ResourceList\
    );\
    STDMETHODIMP GetDeviceProperty\
    (   IN      DEVICE_REGISTRY_PROPERTY    DeviceProperty,\
        IN      ULONG                       BufferLength,\
        OUT     PVOID                       PropertyBuffer,\
        OUT     PULONG                      ResultLength\
    );\
    STDMETHODIMP NewRegistryKey\
    (   OUT     PREGISTRYKEY *      OutRegistryKey,\
        IN      PUNKNOWN            OuterUnknown        OPTIONAL,\
        IN      ULONG               RegistryKeyType,\
        IN      ACCESS_MASK         DesiredAccess,\
        IN      POBJECT_ATTRIBUTES  ObjectAttributes    OPTIONAL,\
        IN      ULONG               CreateOptions       OPTIONAL,\
        OUT     PULONG              Disposition         OPTIONAL\
    )
#endif

/*****************************************************************************
 * IPortMidi
 *****************************************************************************
 * Interface for MIDI port lower edge.
 */
DECLARE_INTERFACE_(IPortMidi,IPort)
{
    STDMETHOD_(void,Notify)
    (   THIS_
        IN      PSERVICEGROUP   ServiceGroup    OPTIONAL
    )   PURE;

    STDMETHOD_(void,RegisterServiceGroup)
    (   THIS_
        IN      PSERVICEGROUP   ServiceGroup
    )   PURE;
};

typedef IPortMidi *PPORTMIDI;

#ifdef PC_IMPLEMENTATION
#define IMP_IPortMidi\
    IMP_IPort;\
    STDMETHODIMP_(void) Notify\
    (   IN      PSERVICEGROUP   ServiceGroup    OPTIONAL\
    );\
    STDMETHODIMP_(void) RegisterServiceGroup\
    (   IN      PSERVICEGROUP   ServiceGroup\
    )
#endif

/*****************************************************************************
 * IMiniportMidiStream
 *****************************************************************************
 * Interface for MIDI miniport streams.
 */
DECLARE_INTERFACE_(IMiniportMidiStream,IUnknown)
{
    STDMETHOD(SetFormat)
    (   THIS_
        IN      PKSDATAFORMAT   DataFormat
    )   PURE;

    STDMETHOD(SetState)
    (   THIS_
        IN      KSSTATE     State
    )   PURE;

    STDMETHOD(Read)
    (   THIS_
        IN      PVOID       BufferAddress,
        IN      ULONG       BufferLength,
        OUT     PULONG      BytesRead
    )   PURE;

    STDMETHOD(Write)
    (   THIS_
        IN      PVOID       BufferAddress,
        IN      ULONG       BytesToWrite,
        OUT     PULONG      BytesWritten
    )   PURE;
};

typedef IMiniportMidiStream *PMINIPORTMIDISTREAM;

#define IMP_IMiniportMidiStream\
    STDMETHODIMP SetFormat\
    (   IN      PKSDATAFORMAT   DataFormat\
    );\
    STDMETHODIMP SetState\
    (   IN      KSSTATE     State\
    );\
    STDMETHODIMP Read\
    (   IN      PVOID       BufferAddress,\
        IN      ULONG       BufferLength,\
        OUT     PULONG      BytesRead\
    );\
    STDMETHODIMP Write\
    (   IN      PVOID       BufferAddress,\
        IN      ULONG       BytesToWrite,\
        OUT     PULONG      BytesWritten\
    )

/*****************************************************************************
 * IMiniportMidi
 *****************************************************************************
 * Interface for MIDI miniports.
 */
DECLARE_INTERFACE_(IMiniportMidi,IMiniport)
{
    STDMETHOD(Init)
    (   THIS_
        IN      PUNKNOWN        UnknownAdapter,
        IN      PRESOURCELIST   ResourceList,
        IN      PPORTMIDI       Port,
        OUT     PSERVICEGROUP * ServiceGroup
    )   PURE;

    STDMETHOD_(void,Service)
    (   THIS
    )   PURE;

    STDMETHOD(NewStream)
    (   THIS_
        OUT     PMINIPORTMIDISTREAM *   Stream,
        IN      PUNKNOWN                OuterUnknown    OPTIONAL,
        IN      POOL_TYPE               PoolType,
        IN      ULONG                   Pin,
        IN      BOOLEAN                 Capture,
        IN      PKSDATAFORMAT           DataFormat,
        OUT     PSERVICEGROUP *         ServiceGroup
    )   PURE;
};

typedef IMiniportMidi *PMINIPORTMIDI;

#define IMP_IMiniportMidi\
    IMP_IMiniport;\
    STDMETHODIMP Init\
    (   IN      PUNKNOWN        UnknownAdapter,\
        IN      PRESOURCELIST   ResourceList,\
        IN      PPORTMIDI       Port,\
        OUT     PSERVICEGROUP * ServiceGroup\
    );\
    STDMETHODIMP_(void) Service\
    (   void\
    );\
    STDMETHODIMP NewStream\
    (   OUT     PMINIPORTMIDISTREAM *   Stream,\
        IN      PUNKNOWN                OuterUnknown    OPTIONAL,\
        IN      POOL_TYPE               PoolType,\
        IN      ULONG                   Pin,\
        IN      BOOLEAN                 Capture,\
        IN      PKSDATAFORMAT           DataFormat,\
        OUT     PSERVICEGROUP *         ServiceGroup\
    )

/*****************************************************************************
 * IPortDMus
 *****************************************************************************
 * See DMusicKS.h
 */

/*****************************************************************************
 * IMXF
 *****************************************************************************
 * See DMusicKS.h
 */

/*****************************************************************************
 * IAllocatorMXF
 *****************************************************************************
 * See DMusicKS.h
 */

/*****************************************************************************
 * IMiniportDMus
 *****************************************************************************
 * See DMusicKS.h
 */


/*****************************************************************************
 * IPortTopology
 *****************************************************************************
 * Interface for topology port lower edge.
 */
DECLARE_INTERFACE_(IPortTopology,IPort)
{
};

typedef IPortTopology *PPORTTOPOLOGY;

#ifdef PC_IMPLEMENTATION
#define IMP_IPortTopology IMP_IPort
#endif

/*****************************************************************************
 * IMiniportTopology
 *****************************************************************************
 * Interface for topology miniports.
 */
DECLARE_INTERFACE_(IMiniportTopology,IMiniport)
{
    STDMETHOD(Init)
    (   THIS_
        IN      PUNKNOWN                UnknownAdapter,
        IN      PRESOURCELIST           ResourceList,
        IN      PPORTTOPOLOGY           Port
    )   PURE;
};

typedef IMiniportTopology *PMINIPORTTOPOLOGY;

#define IMP_IMiniportTopology\
    IMP_IMiniport;\
    STDMETHODIMP Init\
    (   IN      PUNKNOWN        UnknownAdapter,\
        IN      PRESOURCELIST   ResourceList,\
        IN      PPORTTOPOLOGY   Port\
    )

/*****************************************************************************
 * IPortWaveCyclic
 *****************************************************************************
 * Interface for cyclic wave port lower edge.
 */
DECLARE_INTERFACE_(IPortWaveCyclic,IPort)
{
    STDMETHOD_(void,Notify)
    (   THIS_
        IN      PSERVICEGROUP   ServiceGroup
    )   PURE;

    STDMETHOD(NewSlaveDmaChannel)
    (   THIS_
        OUT     PDMACHANNELSLAVE *  DmaChannel,
        IN      PUNKNOWN            OuterUnknown,
        IN      PRESOURCELIST       ResourceList,
        IN      ULONG               DmaIndex,
        IN      ULONG               MaximumLength,
        IN      BOOLEAN             DemandMode,
        IN      DMA_SPEED           DmaSpeed
    )   PURE;

    STDMETHOD(NewMasterDmaChannel)
    (   THIS_
        OUT     PDMACHANNEL *   DmaChannel,
        IN      PUNKNOWN        OuterUnknown,
        IN      PRESOURCELIST   ResourceList    OPTIONAL,
        IN      ULONG           MaximumLength,
        IN      BOOLEAN         Dma32BitAddresses,
        IN      BOOLEAN         Dma64BitAddresses,
        IN      DMA_WIDTH       DmaWidth,
        IN      DMA_SPEED       DmaSpeed
    )   PURE;
};

typedef IPortWaveCyclic *PPORTWAVECYCLIC;

#ifdef PC_IMPLEMENTATION
#define IMP_IPortWaveCyclic\
    IMP_IPort;\
    STDMETHODIMP_(void) Notify\
    (   IN      PSERVICEGROUP   ServiceGroup\
    );\
    STDMETHODIMP NewSlaveDmaChannel\
    (   OUT     PDMACHANNELSLAVE *  DmaChannel,\
        IN      PUNKNOWN            OuterUnknown,\
        IN      PRESOURCELIST       ResourceList,\
        IN      ULONG               DmaIndex,\
        IN      ULONG               MaximumLength,\
        IN      BOOLEAN             DemandMode,\
        IN      DMA_SPEED           DmaSpeed\
    );\
    STDMETHODIMP NewMasterDmaChannel\
    (   OUT     PDMACHANNEL *   DmaChannel,\
        IN      PUNKNOWN        OuterUnknown,\
        IN      PRESOURCELIST   ResourceList    OPTIONAL,\
        IN      ULONG           MaximumLength,\
        IN      BOOLEAN         Dma32BitAddresses,\
        IN      BOOLEAN         Dma64BitAddresses,\
        IN      DMA_WIDTH       DmaWidth,\
        IN      DMA_SPEED       DmaSpeed\
    )
#endif

/*****************************************************************************
 * IMiniportWaveCyclicStream
 *****************************************************************************
 * Interface for cyclic wave miniport streams.
 */
DECLARE_INTERFACE_(IMiniportWaveCyclicStream,IUnknown)
{
    STDMETHOD(SetFormat)
    (   THIS_
        IN      PKSDATAFORMAT   DataFormat
    )   PURE;

    STDMETHOD_(ULONG,SetNotificationFreq)
    (   THIS_
        IN      ULONG           Interval,
        OUT     PULONG          FrameSize
    )   PURE;

    STDMETHOD(SetState)
    (   THIS_
        IN      KSSTATE         State
    )   PURE;

    STDMETHOD(GetPosition)
    (   THIS_
        OUT     PULONG          Position
    )   PURE;
    
    STDMETHOD(NormalizePhysicalPosition)
    (   THIS_
        IN OUT  PLONGLONG       PhysicalPosition
    )   PURE;
    
    STDMETHOD_(void,Silence)
    (   THIS_
        IN      PVOID           Buffer,
        IN      ULONG           ByteCount
    )   PURE;
};

typedef IMiniportWaveCyclicStream *PMINIPORTWAVECYCLICSTREAM;

#define IMP_IMiniportWaveCyclicStream\
    STDMETHODIMP SetFormat\
    (   IN      PKSDATAFORMAT   DataFormat\
    );\
    STDMETHODIMP_(ULONG) SetNotificationFreq\
    (   IN      ULONG           Interval,\
        OUT     PULONG          FrameSize\
    );\
    STDMETHODIMP SetState\
    (   IN      KSSTATE         State\
    );\
    STDMETHODIMP GetPosition\
    (   OUT     PULONG          Position\
    );\
    STDMETHODIMP NormalizePhysicalPosition\
    (   IN OUT PLONGLONG        PhysicalPosition\
    );\
    STDMETHODIMP_(void) Silence\
    (   IN      PVOID           Buffer,\
        IN      ULONG           ByteCount\
    )

/*****************************************************************************
 * IMiniportWaveCyclic
 *****************************************************************************
 * Interface for cyclic wave miniports.
 */
DECLARE_INTERFACE_(IMiniportWaveCyclic,IMiniport)
{
    STDMETHOD(Init)
    (   THIS_
        IN      PUNKNOWN        UnknownAdapter,
        IN      PRESOURCELIST   ResourceList,
        IN      PPORTWAVECYCLIC Port
    )   PURE;

    STDMETHOD(NewStream)
    (   THIS_
        OUT     PMINIPORTWAVECYCLICSTREAM * Stream,
        IN      PUNKNOWN                    OuterUnknown    OPTIONAL,
        IN      POOL_TYPE                   PoolType,
        IN      ULONG                       Pin,
        IN      BOOLEAN                     Capture,
        IN      PKSDATAFORMAT               DataFormat,
        OUT     PDMACHANNEL *               DmaChannel,
        OUT     PSERVICEGROUP *             ServiceGroup
    )   PURE;
};

typedef IMiniportWaveCyclic *PMINIPORTWAVECYCLIC;

#define IMP_IMiniportWaveCyclic\
    IMP_IMiniport;\
    STDMETHODIMP Init\
    (   IN      PUNKNOWN        UnknownAdapter,\
        IN      PRESOURCELIST   ResourceList,\
        IN      PPORTWAVECYCLIC Port\
    );\
    STDMETHODIMP NewStream\
    (   OUT     PMINIPORTWAVECYCLICSTREAM * Stream,\
        IN      PUNKNOWN                    OuterUnknown    OPTIONAL,\
        IN      POOL_TYPE                   PoolType,\
        IN      ULONG                       Pin,\
        IN      BOOLEAN                     Capture,\
        IN      PKSDATAFORMAT               DataFormat,\
        OUT     PDMACHANNEL *               DmaChannel,\
        OUT     PSERVICEGROUP *             ServiceGroup\
    )

/*****************************************************************************
 * IPortWavePci
 *****************************************************************************
 * Interface for PCI wave port lower edge.
 */
DECLARE_INTERFACE_(IPortWavePci,IPort)
{
    STDMETHOD_(void,Notify)
    (   THIS_
        IN      PSERVICEGROUP       ServiceGroup
    )   PURE;

    STDMETHOD(NewMasterDmaChannel)
    (   THIS_
        OUT     PDMACHANNEL *       OutDmaChannel,
        IN      PUNKNOWN            OuterUnknown    OPTIONAL,
        IN      POOL_TYPE           PoolType,
        IN      PRESOURCELIST       ResourceList    OPTIONAL,
        IN      BOOLEAN             ScatterGather,
        IN      BOOLEAN             Dma32BitAddresses,
        IN      BOOLEAN             Dma64BitAddresses,
        IN      BOOLEAN             IgnoreCount,
        IN      DMA_WIDTH           DmaWidth,
        IN      DMA_SPEED           DmaSpeed,
        IN      ULONG               MaximumLength,
        IN      ULONG               DmaPort
    )   PURE;
};

typedef IPortWavePci *PPORTWAVEPCI;

#ifdef PC_IMPLEMENTATION
#define IMP_IPortWavePci\
    IMP_IPort;\
    STDMETHODIMP_(void) Notify\
    (   IN      PSERVICEGROUP       ServiceGroup\
    );\
    STDMETHODIMP NewMasterDmaChannel\
    (   OUT     PDMACHANNEL *       OutDmaChannel,\
        IN      PUNKNOWN            OuterUnknown    OPTIONAL,\
        IN      POOL_TYPE           PoolType,\
        IN      PRESOURCELIST       ResourceList    OPTIONAL,\
        IN      BOOLEAN             ScatterGather,\
        IN      BOOLEAN             Dma32BitAddresses,\
        IN      BOOLEAN             Dma64BitAddresses,\
        IN      BOOLEAN             IgnoreCount,\
        IN      DMA_WIDTH           DmaWidth,\
        IN      DMA_SPEED           DmaSpeed,\
        IN      ULONG               MaximumLength,\
        IN      ULONG               DmaPort\
    )
#endif

/*****************************************************************************
 * IPortWavePciStream
 *****************************************************************************
 * Interface for PCI wave port pin lower edge.
 */
DECLARE_INTERFACE_(IPortWavePciStream,IUnknown)
{
    STDMETHOD(GetMapping)
    (   THIS_
        IN      PVOID               Tag,
        OUT     PPHYSICAL_ADDRESS   PhysicalAddress,
        OUT     PVOID *             VirtualAddress,
        OUT     PULONG              ByteCount,
        OUT     PULONG              Flags
    )   PURE;

    STDMETHOD(ReleaseMapping)
    (   THIS_
        IN      PVOID               Tag
    )   PURE;

    STDMETHOD(TerminatePacket)
    (   THIS
    )   PURE;
};

typedef IPortWavePciStream *PPORTWAVEPCISTREAM;

#ifdef PC_IMPLEMENTATION
#define IMP_IPortWavePciStream\
    STDMETHODIMP GetMapping\
    (   IN      PVOID               Tag,\
        OUT     PPHYSICAL_ADDRESS   PhysicalAddress,\
        OUT     PVOID *             VirtualAddress,\
        OUT     PULONG              ByteCount,\
        OUT     PULONG              Flags\
    );\
    STDMETHODIMP ReleaseMapping\
    (   IN      PVOID               Tag\
    );\
    STDMETHODIMP TerminatePacket\
    (   void\
    )
#endif

/*****************************************************************************
 * IMiniportWavePciStream
 *****************************************************************************
 * Interface for PCI wave miniport streams.
 */
DECLARE_INTERFACE_(IMiniportWavePciStream,IUnknown)
{
    STDMETHOD(SetFormat)
    (   THIS_
        IN      PKSDATAFORMAT   DataFormat
    )   PURE;

    STDMETHOD(SetState)
    (   THIS_
        IN      KSSTATE         State
    )   PURE;

    STDMETHOD(GetPosition)
    (   THIS_
        OUT     PULONGLONG      Position
    )   PURE;
    
    STDMETHOD(NormalizePhysicalPosition)
    (
        THIS_
        IN OUT PLONGLONG        PhysicalPosition
    )   PURE;
    
    STDMETHOD(GetAllocatorFraming)
    (
        THIS_
        OUT PKSALLOCATOR_FRAMING AllocatorFraming
    ) PURE;

    STDMETHOD(RevokeMappings)
    (   THIS_
        IN      PVOID           FirstTag,
        IN      PVOID           LastTag,
        OUT     PULONG          MappingsRevoked
    )   PURE;

    STDMETHOD_(void,MappingAvailable)
    (   THIS
    )   PURE;

    STDMETHOD_(void,Service)
    (   THIS
    )   PURE;
};

typedef IMiniportWavePciStream *PMINIPORTWAVEPCISTREAM;

#define IMP_IMiniportWavePciStream\
    STDMETHODIMP SetFormat\
    (   IN      PKSDATAFORMAT   DataFormat\
    );\
    STDMETHODIMP SetState\
    (   IN      KSSTATE         State\
    );\
    STDMETHODIMP GetPosition\
    (   OUT     PULONGLONG      Position\
    );\
    STDMETHODIMP NormalizePhysicalPosition\
    (   IN OUT PLONGLONG        PhysicalPosition\
    );\
    STDMETHODIMP GetAllocatorFraming\
    (   OUT PKSALLOCATOR_FRAMING AllocatorFraming\
    );\
    STDMETHODIMP RevokeMappings\
    (   IN      PVOID           FirstTag,\
        IN      PVOID           LastTag,\
        OUT     PULONG          MappingsRevoked\
    );\
    STDMETHODIMP_(void) MappingAvailable\
    (   void\
    );\
    STDMETHODIMP_(void) Service\
    (   void\
    )

/*****************************************************************************
 * IMiniportWavePci
 *****************************************************************************
 * Interface for PCI wave miniports.
 */
DECLARE_INTERFACE_(IMiniportWavePci,IMiniport)
{
    STDMETHOD(Init)
    (   THIS_
        IN      PUNKNOWN            UnknownAdapter,
        IN      PRESOURCELIST       ResourceList,
        IN      PPORTWAVEPCI        Port,
        OUT     PSERVICEGROUP *     ServiceGroup
    )   PURE;

    STDMETHOD(NewStream)
    (   THIS_
        OUT     PMINIPORTWAVEPCISTREAM *    Stream,
        IN      PUNKNOWN                    OuterUnknown    OPTIONAL,
        IN      POOL_TYPE                   PoolType,
        IN      PPORTWAVEPCISTREAM          PortStream,
        IN      ULONG                       Pin,
        IN      BOOLEAN                     Capture,
        IN      PKSDATAFORMAT               DataFormat,
        OUT     PDMACHANNEL *               DmaChannel,
        OUT     PSERVICEGROUP *             ServiceGroup
    )   PURE;

    STDMETHOD_(void,Service)
    (   THIS
    )   PURE;
};

typedef IMiniportWavePci *PMINIPORTWAVEPCI;

#define IMP_IMiniportWavePci\
    IMP_IMiniport;\
    STDMETHODIMP Init\
    (   IN      PUNKNOWN            UnknownAdapter,\
        IN      PRESOURCELIST       ResourceList,\
        IN      PPORTWAVEPCI        Port,\
        OUT     PSERVICEGROUP *     ServiceGroup\
    );\
    STDMETHODIMP NewStream\
    (   OUT     PMINIPORTWAVEPCISTREAM *    Stream,\
        IN      PUNKNOWN                    OuterUnknown    OPTIONAL,\
        IN      POOL_TYPE                   PoolType,\
        IN      PPORTWAVEPCISTREAM          PortStream,\
        IN      ULONG                       Pin,\
        IN      BOOLEAN                     Capture,\
        IN      PKSDATAFORMAT               DataFormat,\
        OUT     PDMACHANNEL *               DmaChannel,\
        OUT     PSERVICEGROUP *             ServiceGroup\
    );\
    STDMETHODIMP_(void) Service\
    (   void\
    )

/*****************************************************************************
 * IAdapterPowerManagment
 *****************************************************************************
 * Interface that adapters should implment and register
 * if they want power managment messages.
 * Register this interface w/ portcls via the
 * PcRegisterAdapterPowerManagement() call.
 * NOTE: If you want to fill in the caps struct
 * for you device, register the interface
 * w/ portclas in or before your AddDevice()
 * function. The OS queries devices before
 * StartDevice() gets called.
 */
DECLARE_INTERFACE_(IAdapterPowerManagement,IUnknown)
{
    // Called by portcls requested
    // that the device change to
    // the new power state
    STDMETHOD_(void,PowerChangeState)
    (   THIS_
        IN      POWER_STATE     NewState
    )   PURE;
    
    // Caled by portcls to see
    // if the device can change to
    // the requested device state.
    STDMETHOD(QueryPowerChangeState)
    (   THIS_
        IN      POWER_STATE     NewStateQuery
    )   PURE;


    // Called by portcls to get the
    // power managment capabilities
    // for the device. See ACPI
    // documentation for data about
    // DEVICE_CAPABILITIES struct.
    STDMETHOD(QueryDeviceCapabilities)
    (   THIS_
        IN      PDEVICE_CAPABILITIES    PowerDeviceCaps
    )   PURE;
};

typedef IAdapterPowerManagement *PADAPTERPOWERMANAGMENT;

#define IMP_IAdapterPowerManagement\
    STDMETHODIMP_(void) PowerChangeState\
    (   IN      POWER_STATE     NewState\
    );\
    STDMETHODIMP QueryPowerChangeState\
    (   IN      POWER_STATE     NewStateQuery\
    );\
    STDMETHODIMP QueryDeviceCapabilities\
    (   IN      PDEVICE_CAPABILITIES    PowerDeviceCaps\
    )
    
/*****************************************************************************
 * IPowerNotify
 *****************************************************************************
 * An OPTIONAL interface for miniports and pinsto implement to enable them to get
 * device power state change notifications.
 */
DECLARE_INTERFACE_(IPowerNotify,IUnknown)
{
    // Called by the port to notify interested miniports and pins
    // of device power state changes so that appropriate context
    // save/restore can take place.
    STDMETHOD_(void,PowerChangeNotify)
    (   THIS_
        IN      POWER_STATE     PowerState
    )   PURE;
};

typedef IPowerNotify *PPOWERNOTIFY;

#define IMP_IPowerNotify\
    STDMETHODIMP_(void) PowerChangeNotify\
    (   IN  POWER_STATE     PowerState\
    )
    
/*****************************************************************************
 * IPortEvents
 *****************************************************************************
 * An interface implemented by ports to provide notification event helpers
 * to miniports.
 */
DECLARE_INTERFACE_(IPortEvents,IUnknown)
{
    STDMETHOD_(void,AddEventToEventList)
    (   THIS_
        IN  PKSEVENT_ENTRY      EventEntry
    )   PURE;
    STDMETHOD_(void,GenerateEventList)
    (   THIS_
        IN  GUID*   Set     OPTIONAL,
        IN  ULONG   EventId,
        IN  BOOL    PinEvent,
        IN  ULONG   PinId,
        IN  BOOL    NodeEvent,
        IN  ULONG   NodeId
    )   PURE;
};

typedef IPortEvents *PPORTEVENTS;

#define IMP_IPortEvents\
    STDMETHODIMP_(void) AddEventToEventList\
    (   IN  PKSEVENT_ENTRY  EventEntry\
    );\
    STDMETHODIMP_(void) GenerateEventList\
    (   IN  GUID*   Set     OPTIONAL,\
        IN  ULONG   EventId,\
        IN  BOOL    PinEvent,\
        IN  ULONG   PinId,\
        IN  BOOL    NodeEvent,\
        IN  ULONG   NodeId\
    )

/*****************************************************************************
 * Functions.
 */

/*****************************************************************************
 * PCPFNSTARTDEVICE
 *****************************************************************************
 * Type for start device callback.
 */
typedef
NTSTATUS
(*PCPFNSTARTDEVICE)
(
#ifdef PC_OLD_NAMES
    IN      PVOID           DeviceObject,
    IN      PVOID           Irp,
#else
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PIRP            Irp,
#endif
    IN      PRESOURCELIST   ResourceList
);

/*****************************************************************************
 * PCPFNIRPHANDLER
 *****************************************************************************
 * Type for IRP handlers.
 */
typedef
NTSTATUS
(*PCPFNIRPHANDLER)
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PIRP            Irp,
    OUT     PULONG          Action
#define IRP_HANDLER_ACTION_DEFAULT 0
#define IRP_HANDLER_ACTION_FINISH  1
#define IRP_HANDLER_ACTION_FORWARD 2
);

/*****************************************************************************
 * PcInitializeAdapterDriver()
 *****************************************************************************
 * Initializes an adapter driver.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcInitializeAdapterDriver
(
    IN      PDRIVER_OBJECT      DriverObject,
    IN      PUNICODE_STRING     RegistryPathName,
    IN      PDRIVER_ADD_DEVICE  AddDevice
);

/*****************************************************************************
 * PcDispatchIrp()
 *****************************************************************************
 * Dispatch an IRP.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcDispatchIrp
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PIRP            pIrp
);

/*****************************************************************************
 * PcAddAdapterDevice()
 *****************************************************************************
 * Adds an adapter device.  DeviceExtensionSize may be zero for default size.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcAddAdapterDevice
(
    IN      PDRIVER_OBJECT      DriverObject,
    IN      PDEVICE_OBJECT      PhysicalDeviceObject,
    IN      PCPFNSTARTDEVICE    StartDevice,
    IN      ULONG               MaxObjects,
    IN      ULONG               DeviceExtensionSize
);

/*****************************************************************************
 * PcCompleteIrp()
 *****************************************************************************
 * Complete an IRP unless status is STATUS_PENDING.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcCompleteIrp
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PIRP            pIrp,
    IN      NTSTATUS        ntStatus
);

/*****************************************************************************
 * PcForwardIrpSynchronous()
 *****************************************************************************
 * Forward a PnP IRP to the PDO.  The IRP is not completed at this level, 
 * this function does not return until the lower driver has completed the IRP,
 * and DecrementPendingIrpCount() is not called.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcForwardIrpSynchronous
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PIRP            Irp
);

/*****************************************************************************
 * PcRegisterSubdevice()
 *****************************************************************************
 * Registers a subdevice.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterSubdevice
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PWCHAR          Name,
    IN      PUNKNOWN        Unknown
);

/*****************************************************************************
 * PcRegisterPhysicalConnection()
 *****************************************************************************
 * Registers a physical connection between subdevices.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterPhysicalConnection
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PUNKNOWN        FromUnknown,
    IN      ULONG           FromPin,
    IN      PUNKNOWN        ToUnknown,
    IN      ULONG           ToPin
);

/*****************************************************************************
 * PcRegisterPhysicalConnectionToExternal()
 *****************************************************************************
 * Registers a physical connection from a subdevice to an external device.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterPhysicalConnectionToExternal
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PUNKNOWN        FromUnknown,
    IN      ULONG           FromPin,
    IN      PUNICODE_STRING ToString,
    IN      ULONG           ToPin
);

/*****************************************************************************
 * PcRegisterPhysicalConnectionFromExternal()
 *****************************************************************************
 * Registers a physical connection to a subdevice from an external device.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterPhysicalConnectionFromExternal
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PUNICODE_STRING FromString,
    IN      ULONG           FromPin,
    IN      PUNKNOWN        ToUnknown,
    IN      ULONG           ToPin
);

/*****************************************************************************
 * PcNewPort()
 *****************************************************************************
 * Creates an instance of a port driver.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewPort
(
    OUT     PPORT *     OutPort,
    IN      REFCLSID    ClassID
);

/*****************************************************************************
 * PcNewMiniport()
 *****************************************************************************
 * Creates an instance of a system-supplied miniport driver.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewMiniport
(
    OUT     PMINIPORT * OutMiniPort,
    IN      REFCLSID    ClassID
);

/*****************************************************************************
 * PcCompletePendingPropertyRequest()
 *****************************************************************************
 * Completes a pending property request.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcCompletePendingPropertyRequest
(
    IN      PPCPROPERTY_REQUEST PropertyRequest,
    IN      NTSTATUS            NtStatus
);

/*****************************************************************************
 * PcGetTimeInterval
 *****************************************************************************
 * Gets the system time interval
 */
PORTCLASSAPI
ULONGLONG
NTAPI
PcGetTimeInterval
(
    IN  ULONGLONG   Since
);

#define GTI_SECONDS(t)      (ULONGLONG(t)*10000000)
#define GTI_MILLISECONDS(t) (ULONGLONG(t)*10000)
#define GTI_MICROSECONDS(t) (ULONGLONG(t)*10)

/*****************************************************************************
 * PcNewResourceList()
 *****************************************************************************
 * Creates and initializes a resource list.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewResourceList
(
    OUT     PRESOURCELIST *     OutResourceList,
    IN      PUNKNOWN            OuterUnknown            OPTIONAL,
    IN      POOL_TYPE           PoolType,
    IN      PCM_RESOURCE_LIST   TranslatedResources,
    IN      PCM_RESOURCE_LIST   UntranslatedResources
);

/*****************************************************************************
 * PcNewResourceSublist()
 *****************************************************************************
 * Creates and initializes an empty resource list derived from another
 * resource list.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewResourceSublist
(
    OUT     PRESOURCELIST *     OutResourceList,
    IN      PUNKNOWN            OuterUnknown            OPTIONAL,
    IN      POOL_TYPE           PoolType,
    IN      PRESOURCELIST       ParentList,
    IN      ULONG               MaximumEntries
);

/*****************************************************************************
 * PcNewInterruptSync()
 *****************************************************************************
 * Creates and initializes an interrupt-level synchronization object.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewInterruptSync
(
    OUT     PINTERRUPTSYNC *        OutInterruptSync,
    IN      PUNKNOWN                OuterUnknown            OPTIONAL,
    IN      PRESOURCELIST           ResourceList,
    IN      ULONG                   ResourceIndex,
    IN      INTERRUPTSYNCMODE       Mode
);

/*****************************************************************************
 * PcNewServiceGroup()
 *****************************************************************************
 * Creates and initializes a service group object.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewServiceGroup
(
    OUT     PSERVICEGROUP *     OutServiceGroup,
    IN      PUNKNOWN            OuterUnknown            OPTIONAL
);

/*****************************************************************************
 * PcNewRegistryKey()
 *****************************************************************************
 * Creates and initializes a registry key object.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewRegistryKey
(
    OUT     PREGISTRYKEY *      OutRegistryKey,
    IN      PUNKNOWN            OuterUnknown        OPTIONAL,
    IN      ULONG               RegistryKeyType,
    IN      ACCESS_MASK         DesiredAccess,
    IN      PVOID               DeviceObject        OPTIONAL,
    IN      PVOID               SubDevice           OPTIONAL,
    IN      POBJECT_ATTRIBUTES  ObjectAttributes    OPTIONAL,
    IN      ULONG               CreateOptions       OPTIONAL,
    OUT     PULONG              Disposition         OPTIONAL
);

/*****************************************************************************
 * RegistryKeyType for NewRegistryKey()
 *****************************************************************************
 * Enumeration of key types.
 */
enum
{
    GeneralRegistryKey,     // ObjectAttributes and CreateOptions are req'd.
    DeviceRegistryKey,      // Device Object is required
    DriverRegistryKey,      // Device Object is required
    HwProfileRegistryKey,   // Device Object is required
    DeviceInterfaceRegistryKey  // Device Object and SubDevice are required
};

/*****************************************************************************
 * PcGetDeviceProperty()
 *****************************************************************************
 * This returns the requested device property from the registry.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcGetDeviceProperty
(
    IN      PVOID                       DeviceObject,
    IN      DEVICE_REGISTRY_PROPERTY    DeviceProperty,
    IN      ULONG                       BufferLength,
    OUT     PVOID                       PropertyBuffer,
    OUT     PULONG                      ResultLength
);

/*****************************************************************************
 * PcRegisterAdapterPowerManagement()
 *****************************************************************************
 * Register the adapter's power managment interface
 * with portcls.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterAdapterPowerManagement
(
    IN      PUNKNOWN    Unknown,
    IN      PVOID       pvContext1
);

/*****************************************************************************
 * PcRequestNewPowerState()
 *****************************************************************************
 * This routine is used to request a new power state for the device.  It is
 * normally not needed by adapter drivers but is exported in order to
 * support unusual circumstances.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcRequestNewPowerState
(
    IN      PDEVICE_OBJECT      pDeviceObject,
    IN      DEVICE_POWER_STATE  RequestedNewState
);

/*****************************************************************************
 * PcRegisterIoTimeout()
 *****************************************************************************
 * Registers an IoTimeout callback.  This routine registers an IO timeout
 * callback that will be called approximately once per second while the
 * device is active (see IoInitializeTimer, IoStartTimer, and IoStopTimer
 * in the DDK).  This routine must be called at PASSIVE_LEVEL.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterIoTimeout
(
    IN      PDEVICE_OBJECT      pDeviceObject,
    IN      PIO_TIMER_ROUTINE   pTimerRoutine,
    IN      PVOID               pContext
);

/*****************************************************************************
 * PcUnregisterIoTimeout()
 *****************************************************************************
 * Unregisters an IoTimeout callback.  This routine unregisters an IO timeout
 * callback that was registered with PcRegisterIoTimeout.  This routine must 
 * be called at PASSIVE_LEVEL.
 */
PORTCLASSAPI
NTSTATUS
NTAPI
PcUnregisterIoTimeout
(
    IN      PDEVICE_OBJECT      pDeviceObject,
    IN      PIO_TIMER_ROUTINE   pTimerRoutine,
    IN      PVOID               pContext
);

#ifdef PC_OLD_NAMES

#define InitializeAdapterDriver(c1,c2,a)        \
    PcInitializeAdapterDriver(PDRIVER_OBJECT(c1),PUNICODE_STRING(c2),PDRIVER_ADD_DEVICE(a))
#define AddAdapterDevice(c1,c2,s,m)             \
    PcAddAdapterDevice(PDRIVER_OBJECT(c1),PDEVICE_OBJECT(c2),s,m,0)
#define RegisterSubdevice(c1,c2,n,u)            \
    PcRegisterSubdevice(PDEVICE_OBJECT(c1),n,u)
#define RegisterPhysicalConnection(c1,c2,fs,fp,ts,tp) \
    PcRegisterPhysicalConnection(PDEVICE_OBJECT(c1),fs,fp,ts,tp)
#define RegisterPhysicalConnectionToExternal(c1,c2,fs,fp,ts,tp) \
    PcRegisterPhysicalConnectionToExternal(PDEVICE_OBJECT(c1),fs,fp,ts,tp)
#define RegisterPhysicalConnectionFromExternal(c1,c2,fs,fp,ts,tp) \
    PcRegisterPhysicalConnectionFromExternal(PDEVICE_OBJECT(c1),fs,fp,ts,tp)

#define NewPort                                 PcNewPort
#define NewMiniport                             PcNewMiniport
#define CompletePendingPropertyRequest          PcCompletePendingPropertyRequest
#define NewIrpStreamVirtual                     PcNewIrpStreamVirtual
#define NewIrpStreamPhysical                    PcNewIrpStreamPhysical
#define NewResourceList                         PcNewResourceList
#define NewResourceSublist                      PcNewResourceSublist
#define NewDmaChannel                           PcNewDmaChannel
#define NewServiceGroup                         PcNewServiceGroup
#define GetTimeInterval                         PcGetTimeInterval

#define WIN95COMPAT_ReadPortUChar(Port)         READ_PORT_UCHAR(Port)
#define WIN95COMPAT_WritePortUChar(Port,Value)  WRITE_PORT_UCHAR(Port,Value)
#define WIN95COMPAT_ReadPortUShort(Port)        READ_PORT_USHORT(Port)
#define WIN95COMPAT_WritePortUShort(Port,Value) WRITE_PORT_USHORT(Port,Value)

#endif  //PC_OLD_NAMES



#endif //_PORTCLS_H_

