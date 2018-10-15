//
// Definition of custom property set and related COM interfaces
// for SAA7134 Audio support
//
// Geraint Davies, January 2000

#ifndef __AUDIOIDS_H__
#define  __AUDIOIDS_H__

#ifndef DEFINE_GUIDSTRUCT
#define DEFINE_GUIDSTRUCT(g, n) struct __declspec(uuid(g)) n
#define DEFINE_GUIDNAMED(n) __uuidof(struct n)
#endif

// this GUID is 
//  --the propset id for the Audio7134 custom property set
//  -- the IID of the corresponding COM interface
//  -- the CLSID for the COM object that translates the property set to the COM interface

#define STATIC_KSPROPSETID_Audio7134\
    0x50468ac0, 0xd014, 0x11d3, 0xa4, 0xd9, 0x0, 0x60, 0x8, 0xb, 0xa6, 0x34
DEFINE_GUIDSTRUCT("50468AC0-D014-11d3-A4D9-0060080BA634", KSPROPSETID_Audio7134);
#define KSPROPSETID_Audio7134 DEFINE_GUIDNAMED(KSPROPSETID_Audio7134)

// properties in this set
typedef enum {
    KSPROPERTY_AUDIO7134_LOOPBACK = 1,
    KSPROPERTY_AUDIO7134_CHANNEL,

    // this latter is only available on IDD -- under WDM, use crossbar
    KSPROPERTY_AUDIO7134_INPUT
} AudioProperties;

typedef enum {
    AUDIO7134_MONO,
    AUDIO7134_CHANNEL_A,
    AUDIO7134_CHANNEL_B,
    AUDIO7134_CHANNEL_BOTH
} AudioChannelMode;

typedef enum {
    AUDIO7134_IN_TUNER,
    AUDIO7134_IN_COMPOSITE,
    AUDIO7134_IN_S_VIDEO,
} AudioInputSelect;

// this is the CLSID for a property page that uses the COM interface IAudio7134

#define STATIC_PropPage_Audio7134\
    0x50468ac1, 0xd014, 0x11d3, 0xa4, 0xd9, 0x0, 0x60, 0x8, 0xb, 0xa6, 0x34
DEFINE_GUIDSTRUCT("50468AC1-D014-11d3-A4D9-0060080BA634", PropPage_Audio7134);
#define PropPage_Audio7134 DEFINE_GUIDNAMED(PropPage_Audio7134)

// this is the COM interface itself
#if !defined(_NTDDK_) 

#undef INTERFACE
#define INTERFACE IAudio7134
DECLARE_INTERFACE_(IAudio7134, IUnknown)
{
    // IUnknown methods

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

    // IAudio7134 methods
    STDMETHOD(SetLoopback)(THIS_ BOOL bLoopback) PURE;
    STDMETHOD(GetLoopback)(THIS_ BOOL* pbLoopback) PURE;
    STDMETHOD(SetChannelMode)(THIS_ AudioChannelMode mode) PURE;
    STDMETHOD(GetChannelMode)(THIS_ AudioChannelMode* pmode) PURE;

    STDMETHOD(SetInput)(THIS_ AudioInputSelect input) PURE;
    STDMETHOD(GetInput)(THIS_ AudioInputSelect* pinput) PURE;
    // S_FALSE for cannot, S_OK for can
    STDMETHOD(CanSetInput)(THIS) PURE;
};
#endif

// this is the CLSID for a COM object that implements this property set by 
// custom waveIn messages.
#define STATIC_CLSID_IDD7134Interface\
    0x50468ac2, 0xd014, 0x11d3, 0xa4, 0xd9, 0x0, 0x60, 0x8, 0xb, 0xa6, 0x34
DEFINE_GUIDSTRUCT("50468AC2-D014-11d3-A4D9-0060080BA634", CLSID_IDD7134Interface);
#define CLSID_IDD7134Interface DEFINE_GUIDNAMED(CLSID_IDD7134Interface)


// Accessing the AUDIO7134 property set on an IDD driver:
// use waveInMessage with A7134MSG_PROPS.
// dwParam1 is the property id (AudioProperties << 1) with
// bit 0 = 0 for read, 1 for write
//
// dwParam2 is DWORD value for Set, and DWORD* for Get
#ifndef DRVM_USER
#define DRVM_USER   0x4000
#endif

#define	A7134MSG_PROPS	(DRVM_USER + 1)
#define MAKEPROPID(prop, bGet)  ((prop << 1) | (bGet?0:1))
#define PROPID(code)	(code >> 1)
#define IS_SET(code)	(code & 1)




#endif // __AUDIOIDS_H__

