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

//
// Description:
//  Defines NOBITMAP, in order to include mmreg.h.
//
#define NOBITMAP
#include <mmreg.h>
#undef NOBITMAP

#define EUROPA_SAMPLE_FREQUENCY 32000

//
// Description:
//  KSDATAFORMAT_WAVEFORMATEX
//  A KSDATAFORMAT_WAVEFORMATEX structure is used to provide detailed
//  information about an audio stream consisting of wave data.
//
typedef struct 
{
    KSDATAFORMAT    DataFormat;
    WAVEFORMATEX    WaveFormatEx;
} KSDATAFORMAT_WAVEFORMATEX, *PKSDATAFORMAT_WAVEFORMATEX;

//
// Description:
//  Audio Data Formats
//  Indicates the data range that is supported on the audio out pin.
//
const KSDATARANGE_AUDIO PinWaveIoRange =
{
    //
    // KSDATARANGE
    //
    {
        sizeof(KSDATARANGE_AUDIO),              // FormatSize
        0,                                      // Flags
        0,                                      // SampleSize (unused)
        0,                                      // Reserved
        STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),  // MajorFormat
                                                // MEDIATYPE_Audio
        STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM), // SubFormat
                                                // MEDIASUBTYPE_PCM
        STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
                                                // Specifier
                                                // FORMAT_WAVEFORMATEX
    },
	//
	//
	//
    2,                                          // MaximumChannels
    16,                                         // MinimumBitsPerSample
    16,                                         // MaximumBitsPerSample
    32000,                                      // MinimumSampleFrequency
    32000                                       // MinimumSampleFrequency
};

const KSDATARANGE PinWaveIoRangeWildcard =
{
    //
    // KSDATARANGE
    //
    {
        sizeof(KSDATARANGE),                     // FormatSize
        0,                                      // Flags
        0,                                      // SampleSize (unused)
        0,                                      // Reserved
        STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),  // MajorFormat
                                                // MEDIATYPE_Audio
        STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WILDCARD), // SubFormat
                                                // MEDIASUBTYPE_PCM
        STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WILDCARD)
                                                // Specifier
                                                // FORMAT_WAVEFORMATEX
    }
};

//
// Description:
//  Audio Data Formats
//  Indicates the data range that is supported on the audio in pin.
//
const KSDATARANGE_AUDIO PinAnalogIoRange =
{
    //
    // KSDATARANGE
    //
    {
        sizeof(KSDATARANGE),                        // FormatSize
        0,                                          // Flags
        0,                                          // SampleSize
        0,                                          // Reserved
        0x482dee1, 0x7817, 0x11cf, 0x8a, 0x3, 0x0, 0xaa, 0x0, 0x6e, 0xcb, 0x65,
                                                    // MajorFormat
                                                    // MEDIATYPE_AnalogAudio
        STATICGUIDOF(KSDATAFORMAT_SUBTYPE_NONE),    // SubFormat
                                                    // MEDIASUBTYPE_None
        STATICGUIDOF(KSDATAFORMAT_SPECIFIER_NONE)   // Specifier
                                                    // FORMAT_None
    }
};
