/*++

Copyright (c) 1996 Microsoft Corporation.

Module Name:

    SplMedia.h

Abstract:

    Definitions for Sample Property, Method, and Events
    for the Generic Broadcast Driver Architecture Sample

--*/

#if !defined(_KSMEDIA_)
#error KSMEDIA.H must be included before BDAMEDIA.H
#endif // !defined(_KSMEDIA_)

#if !defined(_BDATYPES_)
#error BDATYPES.H must be included before BDAMEDIA.H
#endif // !defined(_BDATYPES_)

#if !defined(_BDAMEDIA_)
#define _BDAMEDIA_
#endif // !defined(_BDAMEDIA_)

#if !defined(_SPLMEDIA_)
#define _SPLMEDIA_

#if defined(__cplusplus)
extern "C" {
#endif // defined(__cplusplus)



//===========================================================================
//
//  Implementation GUID for BDA Generic Sample Tuner
//
//  This will match the implementation GUID in the BDA Generic Sample Capture
//
//===========================================================================

//  Define the implementation GUID.  This GUID will be the same for all
//  drivers that are used to implement a single receiver implemenation.
//  In this case there will be a capture driver that must use the same
//  implementation GUID to insure that pins are correctly connected.
//
//  If the driver set is used by only one implementation, as is the case here,
//  we can hard code the implementation GUID in the driver.  If more than
//  one receiver implementation uses the same driver, then the implementation
//  GUID should be written into the registry by the INF that installs a
//  particular implementation.
//
//  NOTE!  DON'T USE THIS GUID FOR YOUR DRIVER!
//  Generate a new guid using guidgen.exe
//
// {7036ED35-881D-4c50-ADEC-710ECA227DB3}
//
#define STATIC_KSMEDIUMSETID_MyImplementation \
    0x7036ed35L, 0x881d, 0x4c50, 0xad, 0xec, 0x71, 0x0e, 0xca, 0x22, 0x7d, 0xb3 
DEFINE_GUIDSTRUCT("7036ED35-881D-4c50-ADEC-710ECA227DB3", KSMEDIUMSETID_MyImplementation);
#define KSMEDIUMSETID_MyImplementation DEFINE_GUIDNAMED(KSMEDIUMSETID_MyImplementation)



//===========================================================================
//
//  KSProperty Set Definitions for BDA Generic Sample
//
//===========================================================================



//------------------------------------------------------------
//
//
//  BDA Sample Demodulator Node Extension Properties
//
//  NOTE!  DO NOT USE THIS GUID IN YOUR DRIVER!
//
//  You need to create a unique GUID to prevent plugin CLSID collisions.
//  Do not use the following one for BDA Sample Property Set
// {C8417B10-88FC-49d3-88DB-AD33260655D6}
//
#define STATIC_KSPROPSETID_BdaSampleDemodExtensionProperties \
    0xc8417b10, 0x88fc, 0x49d3, 0x88, 0xdb, 0xad, 0x33, 0x26, 0x6, 0x55, 0xd6
DEFINE_GUIDSTRUCT("C8417B10-88FC-49d3-88DB-AD33260655D6", KSPROPSETID_BdaSampleDemodExtensionProperties);
#define KSPROPSETID_BdaSampleDemodExtensionProperties DEFINE_GUIDNAMED(KSPROPSETID_BdaSampleDemodExtensionProperties)

typedef enum {
    KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY1 = 0,
    KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY2,
    KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY3
} KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION;

// specify the sizeof the actual property to retrieve here 
#define DEFINE_KSPROPERTY_ITEM_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY1(GetHandler, SetHandler)\
    DEFINE_KSPROPERTY_ITEM(\
        KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY1,\
        (GetHandler),\
        sizeof(KSP_NODE),\
        sizeof(ULONG),\
        (SetHandler),\
        NULL, 0, NULL, NULL, 0)

// specify the sizeof the actual property to retrieve here
#define DEFINE_KSPROPERTY_ITEM_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY2(GetHandler, SetHandler)\
    DEFINE_KSPROPERTY_ITEM(\
        KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY2,\
        (GetHandler),\
        sizeof(KSP_NODE),\
        sizeof(ULONG),\
        (SetHandler),\
        NULL, 0, NULL, NULL, 0)

// specify the sizeof the actual property to retrieve here 
#define DEFINE_KSPROPERTY_ITEM_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY3(GetHandler, SetHandler)\
    DEFINE_KSPROPERTY_ITEM(\
        KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY3,\
        (GetHandler),\
        sizeof(KSP_NODE),\
        sizeof(ULONG),\
        (SetHandler),\
        NULL, 0, NULL, NULL, 0)


#if defined(__cplusplus)
}
#endif // defined(__cplusplus)

#endif // !defined(_SPLMEDIA_)

