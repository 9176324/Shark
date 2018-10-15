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

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  KSPIN_MEDIUM for xbar video input tuner pin
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_MEDIUM AnlgXBarTunerVideoInMedium [] =
{
    {
        STATIC_VAMP_ANALOG_TUNER_VIDEO_OUT_MEDIUM,
        0,
        0
    }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  KSPIN_MEDIUM for xbar video input, composite pin, s-video pin and
//  xbar audio input crossbar and s-video pin
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_MEDIUM AnlgXBarWildcard [] =
{
    {
        STATIC_GUID_NULL,
        0,
        0
    }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  KSPIN_MEDIUM for xbar audio input tuner pin
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_MEDIUM AnlgXBarTunerAudioInMedium [] =
{
    {
        STATIC_VAMP_ANALOG_TV_AUDIO_OUT_MEDIUM,
        0,
        0
    }
};


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  KSPIN_MEDIUM for xbar video output pin
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_MEDIUM AnlgXBarVideoOutMedium [] =
{
    {
        VAMP_VIDEO_DECODER,
        0,
        0
    }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  KSPIN_MEDIUM for xbar audio output pin
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_MEDIUM AnlgXBarAudioOutMedium [] =
{
    {
        VAMP_AUDIO_DECODER,
        0,
        0
    }
};


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  KSPIN_MEDIUM for TV audio input pin
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_MEDIUM AnlgTVAudioInMedium [] =
{
    {
        STATIC_VAMP_ANALOG_TV_AUDIO_IN_MEDIUM,
        0,
        0
    }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Pin Medium descriptor containing all medium accepted to be connected to
//  the Transport In pin.
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_MEDIUM DgtlTransportInPinMedium[] =
{
    {
        STATIC_VAMP_DIGITAL_TUNER_OUT_MEDIUM,
        0,
        0
    },
    {
        GUID_VSB_OUT,
        0,
        0
    }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Pin Medium descriptor containing all medium accepted to be connected to
//  the tuner output pin.
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_MEDIUM DgtlTunerOutPinMedium[] =
{
    {
        STATIC_VAMP_DIGITAL_TUNER_OUT_MEDIUM,
        0,
        0
    }
};
