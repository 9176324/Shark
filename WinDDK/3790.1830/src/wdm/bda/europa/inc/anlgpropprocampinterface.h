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

#include "defaults.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is always the same, so lets spend a define here.
//
//////////////////////////////////////////////////////////////////////////////
#define PROP_TYPE_SET                                               \
{                                                                   \
    STATICGUIDOF(KSPROPTYPESETID_General),  /* Set */               \
    VT_I4,                                  /* Id, signed 4 byte */ \
    0                                       /* Flags */             \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property range and step mapping for brightness.
//
//////////////////////////////////////////////////////////////////////////////
const KSPROPERTY_STEPPING_LONG BrightnessRangeAndStep[] =
{
    {
        1,                              // SteppingDelta
        0,                              // Reserved
        MIN_VAMP_BRIGHTNESS_UNITS,      // Minimum
        MAX_VAMP_BRIGHTNESS_UNITS       // Maximum
    }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property range and step mapping for contrast.
//
//////////////////////////////////////////////////////////////////////////////
const KSPROPERTY_STEPPING_LONG ContrastRangeAndStep[] =
{
    {
        1,                              // SteppingDelta
        0,                              // Reserved
        MIN_VAMP_CONTRAST_UNITS,        // Minimum
        MAX_VAMP_CONTRAST_UNITS         // Maximum
    }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property range and step mapping for hue.
//
//////////////////////////////////////////////////////////////////////////////
const KSPROPERTY_STEPPING_LONG HueRangeAndStep[] =
{
    {
        1,                              // SteppingDelta
        0,                              // Reserved
        MIN_VAMP_HUE_UNITS,             // Minimum
        MAX_VAMP_HUE_UNITS              // Maximum
    }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property range and step mapping for saturation.
//
//////////////////////////////////////////////////////////////////////////////
const KSPROPERTY_STEPPING_LONG SaturationRangeAndStep[] =
{
    {
        1,                              // SteppingDelta
        0,                              // Reserved
        MIN_VAMP_SATURATION_UNITS,      // Minimum
        MAX_VAMP_SATURATION_UNITS       // Maximum
    }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property range and step mapping for sharpness.
//
//////////////////////////////////////////////////////////////////////////////
const KSPROPERTY_STEPPING_LONG SharpnessRangeAndStep[] =
{
    {
        1,                              // SteppingDelta
        0,                              // Reserved
        MIN_VAMP_SHARPNESS_UNITS,       // Minimum
        MAX_VAMP_SHARPNESS_UNITS        // Maximum
    }
};


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property memberslist for brightness
//
//////////////////////////////////////////////////////////////////////////////
const KSPROPERTY_MEMBERSLIST BrightnessPropertyMembersList[] =
{
    {
        {
            KSPROPERTY_MEMBER_RANGES,
            sizeof(BrightnessRangeAndStep),
            SIZEOF_ARRAY(BrightnessRangeAndStep),
            0
        },
        (PVOID)BrightnessRangeAndStep
    },
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(BrightnessDefault),
            sizeof(BrightnessDefault),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID)&BrightnessDefault

    }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property memberslist for contrast
//
//////////////////////////////////////////////////////////////////////////////
const KSPROPERTY_MEMBERSLIST ContrastPropertyMembersList[] =
{
    {
        {
            KSPROPERTY_MEMBER_RANGES,
            sizeof(ContrastRangeAndStep),
            SIZEOF_ARRAY(ContrastRangeAndStep),
            0
        },
        (PVOID)ContrastRangeAndStep
    },
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(ContrastDefault),
            sizeof(ContrastDefault),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID)&ContrastDefault
    }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property memberslist for hue
//
//////////////////////////////////////////////////////////////////////////////
const KSPROPERTY_MEMBERSLIST HuePropertyMembersList[] =
{
    {
        {
            KSPROPERTY_MEMBER_RANGES,
            sizeof(HueRangeAndStep),
            SIZEOF_ARRAY(HueRangeAndStep),
            0
        },
        (PVOID)HueRangeAndStep
    },
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(HueDefault),
            sizeof(HueDefault),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID)&HueDefault
    }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property memberslist for saturation
//
//////////////////////////////////////////////////////////////////////////////
const KSPROPERTY_MEMBERSLIST SaturationPropertyMembersList[] =
{
    {
        {
            KSPROPERTY_MEMBER_RANGES,
            sizeof(SaturationRangeAndStep),
            SIZEOF_ARRAY(SaturationRangeAndStep),
            0
        },
        (PVOID)SaturationRangeAndStep
    },
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(SaturationDefault),
            sizeof(SaturationDefault),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID)&SaturationDefault
    }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property memberslist for sharpness
//
//////////////////////////////////////////////////////////////////////////////
const KSPROPERTY_MEMBERSLIST SharpnessPropertyMembersList[] =
{
    {
        {
            KSPROPERTY_MEMBER_RANGES,
            sizeof(SharpnessRangeAndStep),
            SIZEOF_ARRAY(SharpnessRangeAndStep),
            0
        },
        (PVOID)SharpnessRangeAndStep
    },
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(SharpnessDefault),
            sizeof(SharpnessDefault),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID)&SharpnessDefault
    }
};

NTSTATUS BrightnessGetHandler
(
    IN PIRP pIrp,
    IN PKSIDENTIFIER pRequest,
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData
);

NTSTATUS BrightnessSetHandler
(
    IN PIRP pIrp,
    IN PKSIDENTIFIER pRequest,
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData
);

NTSTATUS ContrastGetHandler
(
    IN PIRP pIrp,
    IN PKSIDENTIFIER pRequest,
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData
);

NTSTATUS ContrastSetHandler
(
    IN PIRP pIrp,
    IN PKSIDENTIFIER pRequest,
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData
);

NTSTATUS HueGetHandler
(
    IN PIRP pIrp,
    IN PKSIDENTIFIER pRequest,
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData
);

NTSTATUS HueSetHandler
(
    IN PIRP pIrp,
    IN PKSIDENTIFIER pRequest,
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData
);

NTSTATUS SaturationGetHandler
(
    IN PIRP pIrp,
    IN PKSIDENTIFIER pRequest,
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData
);

NTSTATUS SaturationSetHandler
(
    IN PIRP pIrp,
    IN PKSIDENTIFIER pRequest,
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData
);

NTSTATUS SharpnessGetHandler
(
    IN PIRP pIrp,
    IN PKSIDENTIFIER pRequest,
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData
);

NTSTATUS SharpnessSetHandler
(
    IN PIRP pIrp,
    IN PKSIDENTIFIER pRequest,
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData
);


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property values for brightness
//
//////////////////////////////////////////////////////////////////////////////
const KSPROPERTY_VALUES BrightnessPropertyValues =
{
    PROP_TYPE_SET,
    SIZEOF_ARRAY(BrightnessPropertyMembersList),
    BrightnessPropertyMembersList
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property values for contrast
//
//////////////////////////////////////////////////////////////////////////////
const KSPROPERTY_VALUES ContrastPropertyValues =
{
    PROP_TYPE_SET,
    SIZEOF_ARRAY(ContrastPropertyMembersList),
    ContrastPropertyMembersList
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property values for hue
//
//////////////////////////////////////////////////////////////////////////////
const KSPROPERTY_VALUES HuePropertyValues =
{
    PROP_TYPE_SET,
    SIZEOF_ARRAY(HuePropertyMembersList),
    HuePropertyMembersList
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property values for saturation
//
//////////////////////////////////////////////////////////////////////////////
const KSPROPERTY_VALUES SaturationPropertyValues =
{
    PROP_TYPE_SET,
    SIZEOF_ARRAY(SaturationPropertyMembersList),
    SaturationPropertyMembersList
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property values for sharpness
//
//////////////////////////////////////////////////////////////////////////////
const KSPROPERTY_VALUES SharpnessPropertyValues =
{
    PROP_TYPE_SET,
    SIZEOF_ARRAY(SharpnessPropertyMembersList),
    SharpnessPropertyMembersList
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property item for brightness
//
//////////////////////////////////////////////////////////////////////////////
#define KSPROPERTY_ITEM_BRIGHTNESS                                      \
{                                                                       \
    KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS,     /* PropertyId */            \
    (PFNKSHANDLER)(BrightnessGetHandler),   /* GetPropertyHandler */    \
    sizeof(KSPROPERTY),                     /* MinProperty */           \
    sizeof(KSPROPERTY_VIDEOPROCAMP_S),      /* MinData */               \
    (PFNKSHANDLER)(BrightnessSetHandler),   /* SetPropertyHandler */    \
    &BrightnessPropertyValues,              /* Values */                \
    0,                                      /* RelationsCount  */       \
    NULL,                                   /* Relations */             \
    NULL,                                   /* SupportHandler */        \
    0                                       /* SerializedSize */        \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property item for contrast
//
//////////////////////////////////////////////////////////////////////////////
#define KSPROPERTY_ITEM_CONTRAST                                        \
{                                                                       \
    KSPROPERTY_VIDEOPROCAMP_CONTRAST,       /* PropertyId */            \
    (PFNKSHANDLER)(ContrastGetHandler),     /* GetPropertyHandler */    \
    sizeof(KSPROPERTY),                     /* MinProperty */           \
    sizeof(KSPROPERTY_VIDEOPROCAMP_S),      /* MinData */               \
    (PFNKSHANDLER)(ContrastSetHandler),     /* SetPropertyHandler */    \
    &ContrastPropertyValues,                /* Values */                \
    0,                                      /* RelationsCount  */       \
    NULL,                                   /* Relations */             \
    NULL,                                   /* SupportHandler */        \
    0                                       /* SerializedSize */        \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property item for hue
//
//////////////////////////////////////////////////////////////////////////////
#define KSPROPERTY_ITEM_HUE                                             \
{                                                                       \
    KSPROPERTY_VIDEOPROCAMP_HUE,            /* PropertyId */            \
    (PFNKSHANDLER)(HueGetHandler),          /* GetPropertyHandler */    \
    sizeof(KSPROPERTY),                     /* MinProperty */           \
    sizeof(KSPROPERTY_VIDEOPROCAMP_S),      /* MinData */               \
    (PFNKSHANDLER)(HueSetHandler),          /* SetPropertyHandler */    \
    &HuePropertyValues,                     /* Values */                \
    0,                                      /* RelationsCount  */       \
    NULL,                                   /* Relations */             \
    NULL,                                   /* SupportHandler */        \
    0                                       /* SerializedSize */        \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property item for saturation
//
//////////////////////////////////////////////////////////////////////////////
#define KSPROPERTY_ITEM_SATURATION                                      \
{                                                                       \
    KSPROPERTY_VIDEOPROCAMP_SATURATION,     /* PropertyId */            \
    (PFNKSHANDLER)(SaturationGetHandler),   /* GetPropertyHandler */    \
    sizeof(KSPROPERTY),                     /* MinProperty */           \
    sizeof(KSPROPERTY_VIDEOPROCAMP_S),      /* MinData */               \
    (PFNKSHANDLER)(SaturationSetHandler),   /* SetPropertyHandler */    \
    &SaturationPropertyValues,              /* Values */                \
    0,                                      /* RelationsCount  */       \
    NULL,                                   /* Relations */             \
    NULL,                                   /* SupportHandler */        \
    0                                       /* SerializedSize */        \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video procamp property item for sharpness
//
//////////////////////////////////////////////////////////////////////////////
#define KSPROPERTY_ITEM_SHARPNESS                                       \
{                                                                       \
    KSPROPERTY_VIDEOPROCAMP_SHARPNESS,      /* PropertyId */            \
    (PFNKSHANDLER)(SharpnessGetHandler),    /* GetPropertyHandler */    \
    sizeof(KSPROPERTY),                     /* MinProperty */           \
    sizeof(KSPROPERTY_VIDEOPROCAMP_S),      /* MinData */               \
    (PFNKSHANDLER)(SharpnessSetHandler),    /* SetPropertyHandler */    \
    &SharpnessPropertyValues,               /* Values */                \
    0,                                      /* RelationsCount  */       \
    NULL,                                   /* Relations */             \
    NULL,                                   /* SupportHandler */        \
    0                                       /* SerializedSize */        \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video property tables
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSPROPERTY_TABLE(ProcampPropertyTable)
{
    KSPROPERTY_ITEM_BRIGHTNESS,
    KSPROPERTY_ITEM_CONTRAST,
    KSPROPERTY_ITEM_HUE,
    KSPROPERTY_ITEM_SATURATION,
    KSPROPERTY_ITEM_SHARPNESS
};

