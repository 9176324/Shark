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

NTSTATUS DecoderCapsGetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY Request,
    IN OUT PKSPROPERTY_VIDEODECODER_CAPS_S pData
);

NTSTATUS StandardGetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY pRequest,
    IN OUT PKSPROPERTY_VIDEODECODER_S pData
);

NTSTATUS StandardSetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY pRequest,
    IN OUT PKSPROPERTY_VIDEODECODER_S pData
);

NTSTATUS StatusGetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY pRequest,
    IN OUT PKSPROPERTY_VIDEODECODER_STATUS_S pData
);

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video decoder property item for the decoder capabilities
//
//////////////////////////////////////////////////////////////////////////////
#define KSPROPERTY_ITEM_CAPS                                            \
{                                                                       \
    KSPROPERTY_VIDEODECODER_CAPS,           /* PropertyId           */  \
    (PFNKSHANDLER)(DecoderCapsGetHandler),  /* GetPropertyHandler   */  \
    sizeof(KSPROPERTY),                     /* MinProperty          */  \
    sizeof(KSPROPERTY_VIDEODECODER_CAPS_S), /* MinData              */  \
    NULL,                                   /* SetPropertyHandler   */  \
    NULL,                                   /* Values               */  \
    0,                                      /* RelationsCount       */  \
    NULL,                                   /* Relations            */  \
    NULL,                                   /* SupportHandler       */  \
    0                                       /* SerializedSize       */  \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video decoder property item for the video standard
//
//////////////////////////////////////////////////////////////////////////////
#define KSPROPERTY_ITEM_STANDARD                                        \
{                                                                       \
    KSPROPERTY_VIDEODECODER_STANDARD,       /* PropertyId           */  \
    (PFNKSHANDLER)(StandardGetHandler),     /* GetPropertyHandler   */  \
    sizeof(KSPROPERTY),                     /* MinProperty          */  \
    sizeof(KSPROPERTY_VIDEODECODER_S),      /* MinData              */  \
    (PFNKSHANDLER)(StandardSetHandler),     /* SetPropertyHandler   */  \
    NULL,                                   /* Values               */  \
    0,                                      /* RelationsCount       */  \
    NULL,                                   /* Relations            */  \
    NULL,                                   /* SupportHandler       */  \
    0                                       /* SerializedSize       */  \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Video decoder property item for the decoder status
//
//////////////////////////////////////////////////////////////////////////////
#define KSPROPERTY_ITEM_STATUS                                          \
{                                                                       \
    KSPROPERTY_VIDEODECODER_STATUS,         /* PropertyId           */  \
    (PFNKSHANDLER)(StatusGetHandler),       /* GetPropertyHandler   */  \
    sizeof(KSPROPERTY),                     /* MinProperty          */  \
    sizeof(KSPROPERTY_VIDEODECODER_STATUS_S),/* MinData             */  \
    NULL,                                   /* SetPropertyHandler   */  \
    NULL,                                   /* Values               */  \
    0,                                      /* RelationsCount       */  \
    NULL,                                   /* Relations            */  \
    NULL,                                   /* SupportHandler       */  \
    0                                       /* SerializedSize       */  \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Property table containing all video decoder properties items
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSPROPERTY_TABLE(DecoderPropertyTable)
{
    KSPROPERTY_ITEM_CAPS,
    KSPROPERTY_ITEM_STANDARD,
    KSPROPERTY_ITEM_STATUS
};


