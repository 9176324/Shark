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

//ab: use "KS_VBISAMPLINGRATE_47X_NABTS" for 27,000000 MHz
//    and "KS_VBISAMPLINGRATE_5X_NABTS"  for 28,636363 MHz

//#define VBI_SAMPLING_FREQ 27000000

//
// Description:
//  Define for how many samples per line we get from the decoder.
//
#define VBI_SAMPLES_PER_LINE 1440 // 720*2

//
// Description:
//  Define for how many samples per line we get from the decoder.
//
#define VBI_SAMPLE_RATIO_BT 1527 
// (KS_VBISAMPLINGRATE_5X_NABTS/13500) * 720/1000

//
// Description:
//  Define for simulation of other sample rates.
//
#define VBI_PITCH_BT 1600
// upstream VBI filter requires this value 
// (instead of VBI_SAMPLE_RATIO_BT)

//
// Description:
//  Defines the start line of the VBI range for 50Hz standards.
//
#define VBI_START_LINE_PAL 7

//
// Description:
//  Defines the end line of the VBI range for 50Hz standards.
//
#define VBI_STOP_LINE_PAL  22 /*line 23 is needed for decoding e.g. Premiere*/

//
// Description:
//  Defines the number of VBI lines for 50Hz standards.
//
#define VBI_LINES_PAL (VBI_STOP_LINE_PAL - VBI_START_LINE_PAL + 1)

//
// Description:
//  Defines the start line of the VBI range for 60Hz standards.
//
#define VBI_START_LINE_NTSC 10

//
// Description:
//  Defines the end line of the VBI range for 60Hz standards.
//
#define VBI_STOP_LINE_NTSC  21

//
// Description:
//  Defines the number of VBI lines for 60Hz standards.
//
#define VBI_LINES_NTSC (VBI_STOP_LINE_NTSC - VBI_START_LINE_NTSC + 1)

//
// Description:
//  Defines the 60Hz 525 lines VBI standard.
//
#define VIDEO_STANDARD_60HZ_525LINES ( \
                (static_cast <DWORD> (KS_AnalogVideo_NTSC_Mask)) & \
                ~(static_cast <DWORD> (KS_AnalogVideo_NTSC_433)) | \
                (static_cast <DWORD> (KS_AnalogVideo_PAL_60))    | \
                (static_cast <DWORD> (KS_AnalogVideo_PAL_M)) )

//
// Description:
//  Defines the 50Hz 625 lines VBI standard.
//
#define VIDEO_STANDARD_50HZ_625LINES ( \
                (static_cast <DWORD> (KS_AnalogVideo_PAL_Mask)) & \
                ~(static_cast <DWORD> (KS_AnalogVideo_PAL_60))  & \
                ~(static_cast <DWORD> (KS_AnalogVideo_PAL_M))   | \
                (static_cast <DWORD> (KS_AnalogVideo_NTSC_433)) | \
                (static_cast <DWORD> (KS_AnalogVideo_SECAM_Mask))  )
//
// Description:
//  Defines the duration of one frame for 60Hz standards
//
const int VBI_NTSC_FrameDuration    = 333667;

//
// Description:
//  Defines the duration of one field for 60Hz standards
//
const int VBI_NTSC_FieldDuration    = VBI_NTSC_FrameDuration/2;

//
// Description:
//  Defines how many frames will be processed in one second for 60Hz standards
//
const int VBI_NTSC_FrameRate        = 30;

//
// Description:
//  Defines how many fields will be processed in one second for 60Hz standards
//
const int VBI_NTSC_FieldRate        = VBI_NTSC_FrameRate * 2; // = 60

//
// Description:
//  Defines the duration of one frame for 50Hz standards
//
const int VBI_PAL_FrameDuration     = 400000;

//
// Description:
//  Defines the duration of one field for 50Hz standards
//
const int VBI_PAL_FieldDuration     = VBI_PAL_FrameDuration/2;

//
// Description:
//  Defines how many frames will be processed in one second for 50Hz standards
//
const int VBI_PAL_FrameRate         = 25;

//
// Description:
//  Defines how many fields will be processed in one second for 50Hz standards
//
const int VBI_PAL_FieldRate         = VBI_PAL_FrameRate * 2; // = 50

//
// Description:
//  Defines how many bits are used for one pixel
//
const int VBI_BITS_PER_PIXEL        = 8; // 8 bits used for RAW 8 format

// FormatVBI_PAL:
//
// ab: changed from const to static for possibility to change:
// "VideoStandard", "SamplesPerLine", "StrideInBytes", "BufferSize"

//
// Description:
//  This is the data range description of the VBI format we support.
//  It describes all 50 Hz and 625 lines video standards.
//
static KS_DATARANGE_VIDEO_VBI FormatVBI_PAL =
{
    //
    // KSDATARANGE
    //
    {
        sizeof(KS_DATARANGE_VIDEO_VBI),            // FormatSize
        0,                                          // Flags
        VBI_SAMPLES_PER_LINE * VBI_LINES_PAL * 1,   // SampleSize
        0,                                          // Reserved

        STATIC_KSDATAFORMAT_TYPE_VBI,       // aka. MEDIATYPE_VBI
        STATIC_KSDATAFORMAT_SUBTYPE_RAW8,   // aka. MEDIASUBTYPE_RAW8,
        STATIC_KSDATAFORMAT_SPECIFIER_VBI,  // aka. FORMAT_VideoInfo
    },


    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    0,                  // Reserved (was StreamDescriptionFlags) ??????????
    0,                  // Reserved (was MemoryAllocationFlags
                        //           (KS_VIDEO_ALLOC_*))
    //
    // KS_VIDEO_STREAM_CONFIG_CAPS
    //
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VBI,  // GUID
        VIDEO_STANDARD_50HZ_625LINES,       // AnalogVideoStandard
        VBI_SAMPLES_PER_LINE,VBI_LINES_PAL, // InputSize
        VBI_SAMPLES_PER_LINE,VBI_LINES_PAL, // MinCroppingSize
        VBI_SAMPLES_PER_LINE,VBI_LINES_PAL, // MaxCroppingSize
        1,              // CropGranularityX, granularity of cropping size
        1,              // CropGranularityY
        1,              // CropAlignX, alignment of cropping rect
        1,              // CropAlignY;
        VBI_SAMPLES_PER_LINE,VBI_LINES_PAL, // MinOutputSize
        VBI_SAMPLES_PER_LINE,VBI_LINES_PAL, // MaxOutputSize
        1,              // OutputGranularityX, granularity of output size
        1,              // OutputGranularityY;
        0,              // StretchTapsX  (0 no stretch, 1 pix dup, 2 interp..)
        0,              // StretchTapsY
        0,              // ShrinkTapsX
        0,              // ShrinkTapsY
        VBI_PAL_FrameDuration, // MinFrameInterval, 100 nS units
        VBI_PAL_FrameDuration, // MaxFrameInterval, 100 nS units
        VBI_BITS_PER_PIXEL * VBI_PAL_FieldRate *
            VBI_SAMPLES_PER_LINE * VBI_LINES_PAL,  // MinBitsPerSecond;
        VBI_BITS_PER_PIXEL * VBI_PAL_FieldRate *
            VBI_SAMPLES_PER_LINE * VBI_LINES_PAL   // MaxBitsPerSecond;
    },

    //
    // KS_VBIINFOHEADER (default format)
    //
    {
        VBI_START_LINE_PAL,           // ULONG       StartLine;
        VBI_STOP_LINE_PAL,            // ULONG       EndLine;
        KS_VBISAMPLINGRATE_47X_NABTS, // ULONG       SamplingFrequency;
        900,                          // ULONG       MinLineStartTime;
        900,                          // ULONG       MaxLineStartTime;
        900,                          // ULONG       ActualLineStartTime;
        5902,                         // ULONG       ActualLineEndTime;
        KS_AnalogVideo_PAL_B,         // ULONG       VideoStandard;
        VBI_SAMPLES_PER_LINE,         // ULONG       SamplesPerLine;
        1600,         // ULONG       StrideInBytes;
        1600 * 16 // ULONG       BufferSize;
    }
};

// FormatVBI_NTSC:
//
// ab: changed from const to static for possibility to change:
// "VideoStandard", "SamplesPerLine", "StrideInBytes", "BufferSize"

//
// Description:
//  This is the data range description of the VBI format we support.
//  It describes all 60 Hz and 525 lines video standards.
//
static KS_DATARANGE_VIDEO_VBI FormatVBI_NTSC =
{

    //
    // KSDATARANGE
    //
    {
        sizeof (KS_DATARANGE_VIDEO_VBI),            // FormatSize
        0,                                          // Flags
        0,											// SampleSize
        0,                                          // Reserved

        STATIC_KSDATAFORMAT_TYPE_VBI,       // aka. MEDIATYPE_VBI
        STATIC_KSDATAFORMAT_SUBTYPE_RAW8,   // aka. MEDIASUBTYPE_RAW8,
        STATIC_KSDATAFORMAT_SPECIFIER_VBI   // aka. FORMAT_VideoInfo
    },

    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    0,                  // Reserved (was StreamDescriptionFlags) ??????????
    0,                  // Reserved (was MemoryAllocationFlags
                        //           (KS_VIDEO_ALLOC_*))

    //
    // _KS_VIDEO_STREAM_CONFIG_CAPS
    //
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VBI,  // GUID
        VIDEO_STANDARD_60HZ_525LINES,       // AnalogVideoStandard
        VBI_SAMPLES_PER_LINE,VBI_LINES_NTSC,// InputSize
        VBI_SAMPLES_PER_LINE,VBI_LINES_NTSC,// MinCroppingSize
        VBI_SAMPLES_PER_LINE,VBI_LINES_NTSC,// MaxCroppingSize
        1,              // CropGranularityX, granularity of cropping size
        1,              // CropGranularityY
        1,              // CropAlignX, alignment of cropping rect
        1,              // CropAlignY;
        VBI_SAMPLES_PER_LINE,VBI_LINES_NTSC,// MinOutputSize
        VBI_SAMPLES_PER_LINE,VBI_LINES_NTSC,// MaxOutputSize
        1,              // OutputGranularityX, granularity of output size
        1,              // OutputGranularityY;
        0,              // StretchTapsX  (0 no stretch, 1 pix dup, 2 interp..)
        0,              // StretchTapsY
        0,              // ShrinkTapsX
        0,              // ShrinkTapsY
        VBI_NTSC_FieldDuration, // MinFrameInterval, 100 nS units
        VBI_NTSC_FieldDuration, // MaxFrameInterval, 100 nS units
        VBI_BITS_PER_PIXEL * VBI_NTSC_FieldRate *
            VBI_SAMPLES_PER_LINE * VBI_LINES_NTSC,  // MinBitsPerSecond;
        VBI_BITS_PER_PIXEL * VBI_NTSC_FieldRate *
            VBI_SAMPLES_PER_LINE * VBI_LINES_NTSC   // MaxBitsPerSecond;
    },

    //
    // KS_VBIINFOHEADER (default format)
    //
    {
        VBI_START_LINE_NTSC,    // ULONG       StartLine;
        VBI_STOP_LINE_NTSC,     // ULONG       EndLine;
        KS_VBISAMPLINGRATE_47X_NABTS,      // ULONG       SamplingFrequency;
        900,                    // ULONG       MinLineStartTime;
        900,                    // ULONG       MaxLineStartTime;
        900,                    // ULONG       ActualLineStartTime;
        5902,                   // ULONG       ActualLineEndTime;
        KS_AnalogVideo_NTSC_M,  // ULONG       VideoStandard;
        1440,                   // ULONG       SamplesPerLine;
        1600,                   // ULONG       StrideInBytes;
        1600 * 16               // ULONG       BufferSize;
    }
};
