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

#define EUROPA_KS_SIZE_PREHEADER2 (FIELD_OFFSET(KS_VIDEOINFOHEADER2 ,bmiHeader))
#define EUROPA_KS_SIZE_VIDEOHEADER2(pbmi) ((pbmi)->bmiHeader.biSize + EUROPA_KS_SIZE_PREHEADER2)

//
// Description:
//  Macro which calculates the absolute value.
//
// Parameters:
//  x - signed value to get the absolute value of
//
#define ABS(x) ((x) < 0 ? (-(x)) : (x))

#ifndef mmioFOURCC

//
// Description:
//  Macro which builds a FOURCC.
//
// Parameters:
//  ch0 - first ascii letter to change into FourCC code
//  ch1 - second ascii letter to change into FourCC code
//  ch2 - third ascii letter to change into FourCC code
//  ch3 - fourth ascii letter to change into FourCC code
//
#define mmioFOURCC( ch0, ch1, ch2, ch3 )                \
        ( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |    \
        ( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

//
// Description:
//  FOURCC YUV2 definition.
//
#define FOURCC_YUY2         mmioFOURCC('Y', 'U', 'Y', '2')

//
// Description:
//  Frame time in ns units.
//  Equals 'about' 29.97 frames per second.
//
#define FRAME_TIME_NTSC             333667

//
// Description:
//  Frame time in 'ns' units.
//  Equals 'about' 25 frames per second.
//
#define FRAME_TIME_PAL              400000

//
// Description:
//  Frame rate in 'frames per second' units.
//  NTSC equals 30 frames per second (full frame rate).
//
#define FRAME_RATE_NTSC             30

//
// Description:
//  Frame rate in 'frames per second' units.
//  PAL equals 25 frames per second (full frame rate).
//
#define FRAME_RATE_PAL              25

//
// Description:
//  One pixel contains 16 bit.
//
#define BIT_COUNT_16                16

//
// Description:
//  One pixel contains 24 bit.
//
#define BIT_COUNT_24                24

//
// Description:
//  One pixel contains 32 bit.
//
#define BIT_COUNT_32                32

//
// Description:
//  Default output width of the video.
//
#define D_X                         320

//
// Description:
//  Default output height of the video.
//
#define D_Y                         240

//
// Description:
//  Deinterlaced Video Format 50Hz YUY2 definition
//
const KS_DATARANGE_VIDEO2 DeinterlacedFormat_50Hz_YUY2 =
{
    //
    // KSDATARANGE
    //
    {
        sizeof(KS_DATARANGE_VIDEO2),                // FormatSize
        0,                                          // Flags
        0,                                          // SampleSize (unused)
        0,                                          // Reserved

        STATICGUIDOF (KSDATAFORMAT_TYPE_VIDEO),     // aka. MEDIATYPE_Video
        0x32595559, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,                 // aka. MEDIASUBTYPE_RGB24,
        STATICGUIDOF (KSDATAFORMAT_SPECIFIER_VIDEOINFO2) // aka. FORMAT_VideoInfo
    },
	//
	// 
	//
    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    0,                  // Reserved (was StreamDescriptionFlags)
    0,                  // Reserved (was MemoryAllocationFlags
                        //           (KS_VIDEO_ALLOC_*))
    //
    // KS_VIDEO_STREAM_CONFIG_CAPS
    //
    {
        STATICGUIDOF( KSDATAFORMAT_SPECIFIER_VIDEOINFO2 ), // GUID
        KS_AnalogVideo_PAL_B   |
		KS_AnalogVideo_SECAM_B |
		KS_AnalogVideo_SECAM_L, //AnalogVideoStandard, must match with decoder property
        720,480,        // InputSize, (the inherent size of the incoming signal
                        //             with every digitized pixel unique)
        160, 120,       // MinCroppingSize, smallest rcSrc cropping rect allowed
        640, 480,       // MaxCroppingSize, largest  rcSrc cropping rect allowed
        160, 120,       // CropGranularity, granularity of cropping size
        1,   1,         // CropAlign, alignment of cropping rect
        160, 120,       // MinOutputSize, smallest bitmap stream can produce
        640, 480,       // MaxOutputSize, largest  bitmap stream can produce
        160, 120,       // OutputGranularity, granularity of output bitmap size
        0,   0,           // StretchTaps (0 no stretch, 1 pix dup, 2 interp...)
        0,   0,           // ShrinkTaps (0 no shrink, 1 pix dup, 2 interp...)
        FRAME_TIME_PAL, // MinFrameInterval, 100 nS units
        FRAME_TIME_PAL, // MaxFrameInterval, 100 nS units
        BIT_COUNT_16 * FRAME_RATE_PAL * 160 * 120,  // MinBitsPerSecond;
        BIT_COUNT_16 * FRAME_RATE_PAL * 640 * 480   // MaxBitsPerSecond;
    },
    //
    // KS_VIDEOINFOHEADER2 (default format)
    //
    {
        0,0,D_X,D_Y,                            // RECT  rcSource;
        0,0,D_X,D_Y,                            // RECT  rcTarget;
        D_X * D_Y * BIT_COUNT_16 * FRAME_RATE_PAL, // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate;
        FRAME_TIME_PAL,                         // REFERENCE_TIME  AvgTimePerFrame;
        KS_INTERLACE_IsInterlaced|
        KS_INTERLACE_DisplayModeBobOnly, // use AMINTERLACE_* defines. Reject connection if undefined bits are not 0
        0, // use AMCOPYPROTECT_* defines. Reject connection if undefined bits are not 0
        4, // X dimension of picture aspect ratio, e.g. 16 for 16x9 display
        3, // Y dimension of picture aspect ratio, e.g.  9 for 16x9 display
        0, // must be 0; reject connection otherwise
        0, // must be 0; reject connection otherwise
        sizeof(KS_BITMAPINFOHEADER),       // DWORD biSize;
        D_X,                                // LONG  biWidth;
        D_Y,                                // LONG  biHeight;
        1,                                  // WORD  biPlanes;
        BIT_COUNT_16,                          // WORD  biBitCount;
        FOURCC_YUY2,                        // DWORD biCompression;
        D_X * D_Y * BIT_COUNT_16 / 8,          // DWORD biSizeImage;
        0,                                  // LONG  biXPelsPerMeter;
        0,                                  // LONG  biYPelsPerMeter;
        0,                                  // DWORD biClrUsed;
        0                                   // DWORD biClrImportant;
    }
};

//
// Description:
//  Deinterlaced Video Format 60Hz YUY2 definition
//
const KS_DATARANGE_VIDEO2 DeinterlacedFormat_60Hz_YUY2 =
{
    //
    // KSDATARANGE
    //
    {
        sizeof(KS_DATARANGE_VIDEO2),                // FormatSize
        0,                                          // Flags
        0,                                          // SampleSize (unused)
        0,                                          // Reserved

        STATICGUIDOF (KSDATAFORMAT_TYPE_VIDEO),     // aka. MEDIATYPE_Video
        0x32595559, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,                 // aka. MEDIASUBTYPE_RGB24,
        STATICGUIDOF (KSDATAFORMAT_SPECIFIER_VIDEOINFO2) // aka. FORMAT_VideoInfo
    },
    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    0,                  // Reserved (was StreamDescriptionFlags)
    0,                  // Reserved (was MemoryAllocationFlags
                        //           (KS_VIDEO_ALLOC_*))
    //
    // KS_VIDEO_STREAM_CONFIG_CAPS
    //
    {
        STATICGUIDOF( KSDATAFORMAT_SPECIFIER_VIDEOINFO2 ), // GUID
        KS_AnalogVideo_NTSC_M, //AnalogVideoStandard, must match with decoder property
        720,480,        // InputSize, (the inherent size of the incoming signal
                        //             with every digitized pixel unique)
        160, 120,       // MinCroppingSize, smallest rcSrc cropping rect allowed
        640, 480,       // MaxCroppingSize, largest  rcSrc cropping rect allowed
        160, 120,       // CropGranularity, granularity of cropping size
        1,   1,         // CropAlign, alignment of cropping rect
        160, 120,       // MinOutputSize, smallest bitmap stream can produce
        640, 480,       // MaxOutputSize, largest  bitmap stream can produce
        160, 120,       // OutputGranularity, granularity of output bitmap size
        0,   0,           // StretchTaps (0 no stretch, 1 pix dup, 2 interp...)
        0,   0,           // ShrinkTaps (0 no shrink, 1 pix dup, 2 interp...)
        FRAME_TIME_NTSC, // MinFrameInterval, 100 nS units
        FRAME_TIME_NTSC, // MaxFrameInterval, 100 nS units
        BIT_COUNT_16 * FRAME_RATE_NTSC * 160 * 120,  // MinBitsPerSecond;
        BIT_COUNT_16 * FRAME_RATE_NTSC * 640 * 480   // MaxBitsPerSecond;
    },
    //
    // KS_VIDEOINFOHEADER2 (default format)
    //
    {
        0,0,D_X,D_Y,                            // RECT  rcSource;
        0,0,D_X,D_Y,                            // RECT  rcTarget;
        D_X * D_Y * BIT_COUNT_16 * FRAME_RATE_NTSC, // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate;
        FRAME_TIME_NTSC,                         // REFERENCE_TIME  AvgTimePerFrame;
        KS_INTERLACE_IsInterlaced|
        KS_INTERLACE_DisplayModeBobOnly, // use AMINTERLACE_* defines. Reject connection if undefined bits are not 0
        0, // use AMCOPYPROTECT_* defines. Reject connection if undefined bits are not 0
        4, // X dimension of picture aspect ratio, e.g. 16 for 16x9 display
        3, // Y dimension of picture aspect ratio, e.g.  9 for 16x9 display
        0, // must be 0; reject connection otherwise
        0, // must be 0; reject connection otherwise
        sizeof(KS_BITMAPINFOHEADER),       // DWORD biSize;
        D_X,                                // LONG  biWidth;
        D_Y,                                // LONG  biHeight;
        1,                                  // WORD  biPlanes;
        BIT_COUNT_16,                          // WORD  biBitCount;
        FOURCC_YUY2,                        // DWORD biCompression;
        D_X * D_Y * BIT_COUNT_16 / 8,          // DWORD biSizeImage;
        0,                                  // LONG  biXPelsPerMeter;
        0,                                  // LONG  biYPelsPerMeter;
        0,                                  // DWORD biClrUsed;
        0                                   // DWORD biClrImportant;
    }
};

//
// Description:
//  Video Format 50Hz RGB15 definition
//
const KS_DATARANGE_VIDEO Format_50Hz_RGB15 =
{
    //
    // KSDATARANGE
    //
    {
        sizeof(KS_DATARANGE_VIDEO),                 // FormatSize
        0,                                          // Flags
        0,                                          // SampleSize (unused)
        0,                                          // Reserved

        STATICGUIDOF (KSDATAFORMAT_TYPE_VIDEO),     // aka. MEDIATYPE_Video
        0xe436eb7c, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70,                 // aka. MEDIASUBTYPE_RGB24,
        STATICGUIDOF (KSDATAFORMAT_SPECIFIER_VIDEOINFO) // aka. FORMAT_VideoInfo
    },
    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    0,                  // Reserved (was StreamDescriptionFlags)
    0,                  // Reserved (was MemoryAllocationFlags
                        //           (KS_VIDEO_ALLOC_*))
    //
    // KS_VIDEO_STREAM_CONFIG_CAPS
    //
    {
        STATICGUIDOF( KSDATAFORMAT_SPECIFIER_VIDEOINFO ), // GUID
        KS_AnalogVideo_PAL_B   |
		KS_AnalogVideo_SECAM_B |
		KS_AnalogVideo_SECAM_L, //AnalogVideoStandard, must match with decoder property
        720,480,        // InputSize, (the inherent size of the incoming signal
                        //             with every digitized pixel unique)
        160, 120,       // MinCroppingSize, smallest rcSrc cropping rect allowed
        640, 480,       // MaxCroppingSize, largest  rcSrc cropping rect allowed
        160, 120,       // CropGranularity, granularity of cropping size
        1,   1,         // CropAlign, alignment of cropping rect
        160, 120,       // MinOutputSize, smallest bitmap stream can produce
        640, 480,       // MaxOutputSize, largest  bitmap stream can produce
        160, 120,       // OutputGranularity, granularity of output bitmap size
        0,   0,           // StretchTaps (0 no stretch, 1 pix dup, 2 interp...)
        0,   0,           // ShrinkTaps (0 no shrink, 1 pix dup, 2 interp...)
        FRAME_TIME_PAL,   // MinFrameInterval, 100 nS units
        FRAME_TIME_PAL,   // MaxFrameInterval, 100 nS units
        BIT_COUNT_16 * FRAME_RATE_PAL * 160 * 120, // MinBitsPerSecond;
        BIT_COUNT_16 * FRAME_RATE_PAL * 640 * 480  // MaxBitsPerSecond;
    },
    //
    // KS_VIDEOINFOHEADER (default format)
    //
    {
        0,0,D_X,D_Y,                            // RECT  rcSource;
        0,0,D_X,D_Y,                            // RECT  rcTarget;
        D_X * D_Y * BIT_COUNT_16 * FRAME_RATE_PAL, // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate;
        FRAME_TIME_PAL,                         // REFERENCE_TIME  AvgTimePerFrame;
//    0,   // use AMINTERLACE_* defines. Reject connection if undefined bits are not 0
//    0, // use AMCOPYPROTECT_* defines. Reject connection if undefined bits are not 0
//    0, // X dimension of picture aspect ratio, e.g. 16 for 16x9 display
//    0, // Y dimension of picture aspect ratio, e.g.  9 for 16x9 display
//    0,        // must be 0; reject connection otherwise
//    0,        // must be 0; reject connection otherwise
        sizeof (KS_BITMAPINFOHEADER),       // DWORD biSize;
        D_X,                                // LONG  biWidth;
        D_Y,                                // LONG  biHeight;
        1,                                  // WORD  biPlanes;
        BIT_COUNT_16,                       // WORD  biBitCount;
        KS_BI_RGB,                          // DWORD biCompression;
        D_X * D_Y * BIT_COUNT_16 / 8,       // DWORD biSizeImage;
        0,                                  // LONG  biXPelsPerMeter;
        0,                                  // LONG  biYPelsPerMeter;
        0,                                  // DWORD biClrUsed;
        0                                   // DWORD biClrImportant;
    }
};

//
// Description:
//  Video Format 60Hz RGB15 definition
//
const KS_DATARANGE_VIDEO Format_60Hz_RGB15 =
{
    //
    // KSDATARANGE
    //
    {
        sizeof(KS_DATARANGE_VIDEO),                 // FormatSize
        0,                                          // Flags
        0,                                          // SampleSize (unused)
        0,                                          // Reserved

        STATICGUIDOF (KSDATAFORMAT_TYPE_VIDEO),     // aka. MEDIATYPE_Video
        0xe436eb7c, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70,                 // aka. MEDIASUBTYPE_RGB24,
        STATICGUIDOF (KSDATAFORMAT_SPECIFIER_VIDEOINFO) // aka. FORMAT_VideoInfo
    },
    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    0,                  // Reserved (was StreamDescriptionFlags)
    0,                  // Reserved (was MemoryAllocationFlags
                        //           (KS_VIDEO_ALLOC_*))
    //
    // KS_VIDEO_STREAM_CONFIG_CAPS
    //
    {
        STATICGUIDOF( KSDATAFORMAT_SPECIFIER_VIDEOINFO ), // GUID
        KS_AnalogVideo_NTSC_M, //AnalogVideoStandard, must match with decoder property
        720,480,        // InputSize, (the inherent size of the incoming signal
                        //             with every digitized pixel unique)
        160, 120,       // MinCroppingSize, smallest rcSrc cropping rect allowed
        640, 480,       // MaxCroppingSize, largest  rcSrc cropping rect allowed
        160, 120,       // CropGranularity, granularity of cropping size
        1,   1,         // CropAlign, alignment of cropping rect
        160, 120,       // MinOutputSize, smallest bitmap stream can produce
        640, 480,       // MaxOutputSize, largest  bitmap stream can produce
        160, 120,       // OutputGranularity, granularity of output bitmap size
        0,   0,           // StretchTaps (0 no stretch, 1 pix dup, 2 interp...)
        0,   0,           // ShrinkTaps (0 no shrink, 1 pix dup, 2 interp...)
        FRAME_TIME_NTSC, // MinFrameInterval, 100 nS units
        FRAME_TIME_NTSC, // MaxFrameInterval, 100 nS units
        BIT_COUNT_16 * FRAME_RATE_NTSC * 160 * 120,  // MinBitsPerSecond;
        BIT_COUNT_16 * FRAME_RATE_NTSC * 640 * 480   // MaxBitsPerSecond;
    },
    //
    // KS_VIDEOINFOHEADER (default format)
    //
    {
        0,0,D_X,D_Y,                            // RECT  rcSource;
        0,0,D_X,D_Y,                            // RECT  rcTarget;
        D_X * D_Y * BIT_COUNT_16 * FRAME_RATE_NTSC, // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate;
        FRAME_TIME_NTSC,                         // REFERENCE_TIME  AvgTimePerFrame;
//    0,   // use AMINTERLACE_* defines. Reject connection if undefined bits are not 0
//    0, // use AMCOPYPROTECT_* defines. Reject connection if undefined bits are not 0
//    0, // X dimension of picture aspect ratio, e.g. 16 for 16x9 display
//    0, // Y dimension of picture aspect ratio, e.g.  9 for 16x9 display
//    0,        // must be 0; reject connection otherwise
//    0,        // must be 0; reject connection otherwise
        sizeof (KS_BITMAPINFOHEADER),       // DWORD biSize;
        D_X,                                // LONG  biWidth;
        D_Y,                                // LONG  biHeight;
        1,                                  // WORD  biPlanes;
        BIT_COUNT_16,                       // WORD  biBitCount;
        KS_BI_RGB,                          // DWORD biCompression;
        D_X * D_Y * BIT_COUNT_16 / 8,       // DWORD biSizeImage;
        0,                                  // LONG  biXPelsPerMeter;
        0,                                  // LONG  biYPelsPerMeter;
        0,                                  // DWORD biClrUsed;
        0                                   // DWORD biClrImportant;
    }
};
//
// Description:
//  Deinterlaced Video Format 60Hz RGB15 definition
//
const KS_DATARANGE_VIDEO2 DeinterlacedFormat_60Hz_RGB15 =
{
    //
    // KSDATARANGE
    //
    {
        sizeof(KS_DATARANGE_VIDEO2),                // FormatSize
        0,                                          // Flags
        0,                                          // SampleSize (unused)
        0,                                          // Reserved

        STATICGUIDOF (KSDATAFORMAT_TYPE_VIDEO),     // aka. MEDIATYPE_Video
        0xe436eb7c, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70,                 // aka. MEDIASUBTYPE_RGB24,
        STATICGUIDOF (KSDATAFORMAT_SPECIFIER_VIDEOINFO2) // aka. FORMAT_VideoInfo
    },
    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    0,                  // Reserved (was StreamDescriptionFlags)
    0,                  // Reserved (was MemoryAllocationFlags
                        //           (KS_VIDEO_ALLOC_*))
    //
    // KS_VIDEO_STREAM_CONFIG_CAPS
    //
    {
        STATICGUIDOF( KSDATAFORMAT_SPECIFIER_VIDEOINFO2 ), // GUID
        KS_AnalogVideo_NTSC_M, //AnalogVideoStandard, must match with decoder property
        720,480,        // InputSize, (the inherent size of the incoming signal
                        //             with every digitized pixel unique)
        160, 120,       // MinCroppingSize, smallest rcSrc cropping rect allowed
        640, 480,       // MaxCroppingSize, largest  rcSrc cropping rect allowed
        160, 120,       // CropGranularity, granularity of cropping size
        1,   1,         // CropAlign, alignment of cropping rect
        160, 120,       // MinOutputSize, smallest bitmap stream can produce
        640, 480,       // MaxOutputSize, largest  bitmap stream can produce
        160, 120,       // OutputGranularity, granularity of output bitmap size
        0,   0,           // StretchTaps (0 no stretch, 1 pix dup, 2 interp...)
        0,   0,           // ShrinkTaps (0 no shrink, 1 pix dup, 2 interp...)
        FRAME_TIME_NTSC, // MinFrameInterval, 100 nS units
        FRAME_TIME_NTSC, // MaxFrameInterval, 100 nS units
        BIT_COUNT_16 * FRAME_RATE_NTSC * 160 * 120,  // MinBitsPerSecond;
        BIT_COUNT_16 * FRAME_RATE_NTSC * 640 * 480   // MaxBitsPerSecond;
    },
    //
    // KS_VIDEOINFOHEADER2 (default format)
    //
    {
        0,0,D_X,D_Y,                            // RECT  rcSource;
        0,0,D_X,D_Y,                            // RECT  rcTarget;
        D_X * D_Y * BIT_COUNT_16 * FRAME_RATE_NTSC, // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate;
        FRAME_TIME_NTSC,                         // REFERENCE_TIME  AvgTimePerFrame;
        KS_INTERLACE_IsInterlaced|
        KS_INTERLACE_DisplayModeBobOnly, // use AMINTERLACE_* defines. Reject connection if undefined bits are not 0
        0, // use AMCOPYPROTECT_* defines. Reject connection if undefined bits are not 0
        4, // X dimension of picture aspect ratio, e.g. 16 for 16x9 display
        3, // Y dimension of picture aspect ratio, e.g.  9 for 16x9 display
        0, // must be 0; reject connection otherwise
        0, // must be 0; reject connection otherwise
        sizeof (KS_BITMAPINFOHEADER),       // DWORD biSize;
        D_X,                                // LONG  biWidth;
        D_Y,                                // LONG  biHeight;
        1,                                  // WORD  biPlanes;
        BIT_COUNT_16,                       // WORD  biBitCount;
        KS_BI_RGB,                          // DWORD biCompression;
        D_X * D_Y * BIT_COUNT_16 / 8,       // DWORD biSizeImage;
        0,                                  // LONG  biXPelsPerMeter;
        0,                                  // LONG  biYPelsPerMeter;
        0,                                  // DWORD biClrUsed;
        0                                   // DWORD biClrImportant;
    }
};

//
// Description:
//  Video Format 50Hz RGB24 definition
//
const KS_DATARANGE_VIDEO Format_50Hz_RGB24 =
{
    //
    // KSDATARANGE
    //
    {
        sizeof(KS_DATARANGE_VIDEO),                 // FormatSize
        0,                                          // Flags
        0,                                          // SampleSize (unused)
        0,                                          // Reserved

        STATICGUIDOF (KSDATAFORMAT_TYPE_VIDEO),     // aka. MEDIATYPE_Video
        0xe436eb7d, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70,                 // aka. MEDIASUBTYPE_RGB24,
        STATICGUIDOF (KSDATAFORMAT_SPECIFIER_VIDEOINFO) // aka. FORMAT_VideoInfo
    },
    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    0,                  // Reserved (was StreamDescriptionFlags)
    0,                  // Reserved (was MemoryAllocationFlags
                        //           (KS_VIDEO_ALLOC_*))
    //
    // KS_VIDEO_STREAM_CONFIG_CAPS
    //
    {
        STATICGUIDOF( KSDATAFORMAT_SPECIFIER_VIDEOINFO ), // GUID
        KS_AnalogVideo_PAL_B   |
		KS_AnalogVideo_SECAM_B |
		KS_AnalogVideo_SECAM_L, //AnalogVideoStandard, must match with decoder property
        720,480,        // InputSize, (the inherent size of the incoming signal
                        //             with every digitized pixel unique)
        160, 120,       // MinCroppingSize, smallest rcSrc cropping rect allowed
        640, 480,       // MaxCroppingSize, largest  rcSrc cropping rect allowed
        160, 120,       // CropGranularity, granularity of cropping size
        1,   1,         // CropAlign, alignment of cropping rect
        160, 120,       // MinOutputSize, smallest bitmap stream can produce
        640, 480,       // MaxOutputSize, largest  bitmap stream can produce
        160, 120,       // OutputGranularity, granularity of output bitmap size
        0,   0,           // StretchTaps (0 no stretch, 1 pix dup, 2 interp...)
        0,   0,           // ShrinkTaps (0 no shrink, 1 pix dup, 2 interp...)
        FRAME_TIME_PAL, // MinFrameInterval, 100 nS units
        FRAME_TIME_PAL, // MaxFrameInterval, 100 nS units
        BIT_COUNT_24 * FRAME_RATE_PAL * 160 * 120,  // MinBitsPerSecond;
        BIT_COUNT_24 * FRAME_RATE_PAL * 640 * 480   // MaxBitsPerSecond;
    },
    //
    // KS_VIDEOINFOHEADER (default format)
    //
    {
        0,0,D_X,D_Y,                            // RECT  rcSource;
        0,0,D_X,D_Y,                            // RECT  rcTarget;
        D_X * D_Y * BIT_COUNT_24 * FRAME_RATE_PAL, // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate;
        FRAME_TIME_PAL,                         // REFERENCE_TIME  AvgTimePerFrame;
//    0,   // use AMINTERLACE_* defines. Reject connection if undefined bits are not 0
//    0, // use AMCOPYPROTECT_* defines. Reject connection if undefined bits are not 0
//    0, // X dimension of picture aspect ratio, e.g. 16 for 16x9 display
//    0, // Y dimension of picture aspect ratio, e.g.  9 for 16x9 display
//    0,        // must be 0; reject connection otherwise
//    0,        // must be 0; reject connection otherwise
        sizeof (KS_BITMAPINFOHEADER),       // DWORD biSize;
        D_X,                                // LONG  biWidth;
        D_Y,                                // LONG  biHeight;
        1,                                  // WORD  biPlanes;
        BIT_COUNT_24,                       // WORD  biBitCount;
        KS_BI_RGB,                          // DWORD biCompression;
        D_X * D_Y * BIT_COUNT_24 / 8,       // DWORD biSizeImage;
        0,                                  // LONG  biXPelsPerMeter;
        0,                                  // LONG  biYPelsPerMeter;
        0,                                  // DWORD biClrUsed;
        0                                   // DWORD biClrImportant;
    }
};

//
// Description:
//  Video Format 60Hz RGB24 definition
//
const KS_DATARANGE_VIDEO Format_60Hz_RGB24 =
{
    //
    // KSDATARANGE
    //
    {
        sizeof(KS_DATARANGE_VIDEO),                 // FormatSize
        0,                                          // Flags
        0,                                          // SampleSize (unused)
        0,                                          // Reserved

        STATICGUIDOF (KSDATAFORMAT_TYPE_VIDEO),     // aka. MEDIATYPE_Video
        0xe436eb7d, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70,                 // aka. MEDIASUBTYPE_RGB24,
        STATICGUIDOF (KSDATAFORMAT_SPECIFIER_VIDEOINFO) // aka. FORMAT_VideoInfo
    },
    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    0,                  // Reserved (was StreamDescriptionFlags)
    0,                  // Reserved (was MemoryAllocationFlags
                        //           (KS_VIDEO_ALLOC_*))
    //
    // KS_VIDEO_STREAM_CONFIG_CAPS
    //
    {
        STATICGUIDOF( KSDATAFORMAT_SPECIFIER_VIDEOINFO ), // GUID
        KS_AnalogVideo_NTSC_M, //AnalogVideoStandard, must match with decoder property
        720,480,        // InputSize, (the inherent size of the incoming signal
                        //             with every digitized pixel unique)
        160, 120,       // MinCroppingSize, smallest rcSrc cropping rect allowed
        640, 480,       // MaxCroppingSize, largest  rcSrc cropping rect allowed
        160, 120,       // CropGranularity, granularity of cropping size
        1,   1,         // CropAlign, alignment of cropping rect
        160, 120,       // MinOutputSize, smallest bitmap stream can produce
        640, 480,       // MaxOutputSize, largest  bitmap stream can produce
        160, 120,       // OutputGranularity, granularity of output bitmap size
        0,   0,           // StretchTaps (0 no stretch, 1 pix dup, 2 interp...)
        0,   0,           // ShrinkTaps (0 no shrink, 1 pix dup, 2 interp...)
        FRAME_TIME_NTSC, // MinFrameInterval, 100 nS units
        FRAME_TIME_NTSC, // MaxFrameInterval, 100 nS units
        BIT_COUNT_24 * FRAME_RATE_NTSC * 160 * 120,  // MinBitsPerSecond;
        BIT_COUNT_24 * FRAME_RATE_NTSC * 640 * 480   // MaxBitsPerSecond;
    },
    //
    // KS_VIDEOINFOHEADER (default format)
    //
    {
        0,0,D_X,D_Y,                            // RECT  rcSource;
        0,0,D_X,D_Y,                            // RECT  rcTarget;
        D_X * D_Y * BIT_COUNT_24 * FRAME_RATE_NTSC, // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate;
        FRAME_TIME_NTSC,                         // REFERENCE_TIME  AvgTimePerFrame;
//    0,   // use AMINTERLACE_* defines. Reject connection if undefined bits are not 0
//    0, // use AMCOPYPROTECT_* defines. Reject connection if undefined bits are not 0
//    0, // X dimension of picture aspect ratio, e.g. 16 for 16x9 display
//    0, // Y dimension of picture aspect ratio, e.g.  9 for 16x9 display
//    0,        // must be 0; reject connection otherwise
//    0,        // must be 0; reject connection otherwise
        sizeof (KS_BITMAPINFOHEADER),       // DWORD biSize;
        D_X,                                // LONG  biWidth;
        D_Y,                                // LONG  biHeight;
        1,                                  // WORD  biPlanes;
        BIT_COUNT_24,                       // WORD  biBitCount;
        KS_BI_RGB,                          // DWORD biCompression;
        D_X * D_Y * BIT_COUNT_24 / 8,       // DWORD biSizeImage;
        0,                                  // LONG  biXPelsPerMeter;
        0,                                  // LONG  biYPelsPerMeter;
        0,                                  // DWORD biClrUsed;
        0                                   // DWORD biClrImportant;
    }
};

//
// Description:
//  Video Format 50Hz YUV2 definition
//
const KS_DATARANGE_VIDEO Format_50Hz_YUY2 =
{
    //
    // KSDATARANGE
    //
    {
        sizeof(KS_DATARANGE_VIDEO),                 // FormatSize
        0,                                          // Flags
        0,                                          // SampleSize (unused)
        0,                                          // Reserved

        STATICGUIDOF (KSDATAFORMAT_TYPE_VIDEO),     // aka. MEDIATYPE_Video
        0x32595559, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,                 // aka. MEDIASUBTYPE_RGB24,
        STATICGUIDOF (KSDATAFORMAT_SPECIFIER_VIDEOINFO) // aka. FORMAT_VideoInfo
    },
    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    0,                  // Reserved (was StreamDescriptionFlags)
    0,                  // Reserved (was MemoryAllocationFlags
                        //           (KS_VIDEO_ALLOC_*))
    //
    // KS_VIDEO_STREAM_CONFIG_CAPS
    //
    {
        STATICGUIDOF( KSDATAFORMAT_SPECIFIER_VIDEOINFO ), // GUID
        KS_AnalogVideo_PAL_B   |
		KS_AnalogVideo_SECAM_B |
		KS_AnalogVideo_SECAM_L, //AnalogVideoStandard, must match with decoder property
        720,480,        // InputSize, (the inherent size of the incoming signal
                        //             with every digitized pixel unique)
        160, 120,       // MinCroppingSize, smallest rcSrc cropping rect allowed
        640, 480,       // MaxCroppingSize, largest  rcSrc cropping rect allowed
        160, 120,       // CropGranularity, granularity of cropping size
        1,   1,         // CropAlign, alignment of cropping rect
        160, 120,       // MinOutputSize, smallest bitmap stream can produce
        640, 480,       // MaxOutputSize, largest  bitmap stream can produce
        160, 120,       // OutputGranularity, granularity of output bitmap size
        0,   0,           // StretchTaps (0 no stretch, 1 pix dup, 2 interp...)
        0,   0,           // ShrinkTaps (0 no shrink, 1 pix dup, 2 interp...)
        FRAME_TIME_PAL, // MinFrameInterval, 100 nS units
        FRAME_TIME_PAL, // MaxFrameInterval, 100 nS units
        BIT_COUNT_16 * FRAME_RATE_PAL * 160 * 120,  // MinBitsPerSecond;
        BIT_COUNT_16 * FRAME_RATE_PAL * 640 * 480   // MaxBitsPerSecond;
    },
    //
    // KS_VIDEOINFOHEADER (default format)
    //
    {
        0,0,D_X,D_Y,                            // RECT  rcSource;
        0,0,D_X,D_Y,                            // RECT  rcTarget;
        D_X * D_Y * BIT_COUNT_16 * FRAME_RATE_PAL, // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate;
        FRAME_TIME_PAL,                         // REFERENCE_TIME  AvgTimePerFrame;
//    0,   // use AMINTERLACE_* defines. Reject connection if undefined bits are not 0
//    0, // use AMCOPYPROTECT_* defines. Reject connection if undefined bits are not 0
//    0, // X dimension of picture aspect ratio, e.g. 16 for 16x9 display
//    0, // Y dimension of picture aspect ratio, e.g.  9 for 16x9 display
//    0,        // must be 0; reject connection otherwise
//    0,        // must be 0; reject connection otherwise
        sizeof (KS_BITMAPINFOHEADER),       // DWORD biSize;
        D_X,                                // LONG  biWidth;
        D_Y,                                // LONG  biHeight;
        1,                                  // WORD  biPlanes;
        BIT_COUNT_16,                       // WORD  biBitCount;
        FOURCC_YUY2,                        // DWORD biCompression;
        D_X * D_Y * BIT_COUNT_16 / 8,       // DWORD biSizeImage;
        0,                                  // LONG  biXPelsPerMeter;
        0,                                  // LONG  biYPelsPerMeter;
        0,                                  // DWORD biClrUsed;
        0                                   // DWORD biClrImportant;
    }
};


//
// Description:
//  Video Format 50Hz YUV2 definition
//
const KS_DATARANGE_VIDEO2 InterlacedFormat_50Hz_YUY2 =
{
    //
    // KSDATARANGE
    //
    {
        sizeof(KS_DATARANGE_VIDEO2),                 // FormatSize
        0,                                          // Flags
        0,                                          // SampleSize (unused)
        0,                                          // Reserved

        STATICGUIDOF (KSDATAFORMAT_TYPE_VIDEO),     // aka. MEDIATYPE_Video
        0x32595559, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,                 
        STATICGUIDOF (KSDATAFORMAT_SPECIFIER_VIDEOINFO2) // aka. FORMAT_VideoInfo
    },
    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    0,                  // Reserved (was StreamDescriptionFlags)
    0,                  // Reserved (was MemoryAllocationFlags
                        //           (KS_VIDEO_ALLOC_*))
    //
    // KS_VIDEO_STREAM_CONFIG_CAPS
    //
    {
        STATICGUIDOF( KSDATAFORMAT_SPECIFIER_VIDEOINFO2 ), // GUID
        KS_AnalogVideo_PAL_B   |
		KS_AnalogVideo_SECAM_B |
		KS_AnalogVideo_SECAM_L, //AnalogVideoStandard, must match with decoder property
        720,480,        // InputSize, (the inherent size of the incoming signal
                        //             with every digitized pixel unique)
        160, 120,       // MinCroppingSize, smallest rcSrc cropping rect allowed
        640, 480,       // MaxCroppingSize, largest  rcSrc cropping rect allowed
        160, 120,       // CropGranularity, granularity of cropping size
        1,   1,         // CropAlign, alignment of cropping rect
        160, 120,       // MinOutputSize, smallest bitmap stream can produce
        640, 480,       // MaxOutputSize, largest  bitmap stream can produce
        160, 120,       // OutputGranularity, granularity of output bitmap size
        0,   0,           // StretchTaps (0 no stretch, 1 pix dup, 2 interp...)
        0,   0,           // ShrinkTaps (0 no shrink, 1 pix dup, 2 interp...)
        FRAME_TIME_PAL, // MinFrameInterval, 100 nS units
        FRAME_TIME_PAL, // MaxFrameInterval, 100 nS units
        BIT_COUNT_16 * FRAME_RATE_PAL * 160 * 120,  // MinBitsPerSecond;
        BIT_COUNT_16 * FRAME_RATE_PAL * 640 * 480   // MaxBitsPerSecond;
    },
    //
    // KS_VIDEOINFOHEADER2 (default format)
    //
    {
        0,0,D_X,D_Y,                            // RECT  rcSource;
        0,0,D_X,D_Y,                            // RECT  rcTarget;
        D_X * D_Y * BIT_COUNT_16 * FRAME_RATE_PAL, // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate;
        FRAME_TIME_PAL,                         // REFERENCE_TIME  AvgTimePerFrame;
        KS_INTERLACE_IsInterlaced ,   // use AMINTERLACE_* defines. Reject connection if undefined bits are not 0
        0, // use AMCOPYPROTECT_* defines. Reject connection if undefined bits are not 0
        4, // X dimension of picture aspect ratio, e.g. 16 for 16x9 display
        3, // Y dimension of picture aspect ratio, e.g.  9 for 16x9 display
        0,        // must be 0; reject connection otherwise
        0,        // must be 0; reject connection otherwise
        sizeof (KS_BITMAPINFOHEADER),       // DWORD biSize;
        D_X,                                // LONG  biWidth;
        D_Y,                                // LONG  biHeight;
        1,                                  // WORD  biPlanes;
        BIT_COUNT_16,                       // WORD  biBitCount;
        FOURCC_YUY2,                        // DWORD biCompression;
        D_X * D_Y * BIT_COUNT_16 / 8,       // DWORD biSizeImage;
        0,                                  // LONG  biXPelsPerMeter;
        0,                                  // LONG  biYPelsPerMeter;
        0,                                  // DWORD biClrUsed;
        0                                   // DWORD biClrImportant;
    }
};


//
// Description:
//  Video Format 60Hz YUV2 definition
//
const KS_DATARANGE_VIDEO Format_60Hz_YUY2 =
{
    //
    // KSDATARANGE
    //
    {
        sizeof(KS_DATARANGE_VIDEO),                 // FormatSize
        0,                                          // Flags
        0,                                          // SampleSize (unused)
        0,                                          // Reserved

        STATICGUIDOF (KSDATAFORMAT_TYPE_VIDEO),     // aka. MEDIATYPE_Video
        0x32595559, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,                 // aka. MEDIASUBTYPE_RGB24,
        STATICGUIDOF (KSDATAFORMAT_SPECIFIER_VIDEOINFO) // aka. FORMAT_VideoInfo
    },
    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    0,                  // Reserved (was StreamDescriptionFlags)
    0,                  // Reserved (was MemoryAllocationFlags
                        //           (KS_VIDEO_ALLOC_*))
    //
    // KS_VIDEO_STREAM_CONFIG_CAPS
    //
    {
        STATICGUIDOF( KSDATAFORMAT_SPECIFIER_VIDEOINFO ), // GUID
        KS_AnalogVideo_NTSC_M, //AnalogVideoStandard, must match with decoder property
        720,480,        // InputSize, (the inherent size of the incoming signal
                        //             with every digitized pixel unique)
        160, 120,       // MinCroppingSize, smallest rcSrc cropping rect allowed
        640, 480,       // MaxCroppingSize, largest  rcSrc cropping rect allowed
        160, 120,       // CropGranularity, granularity of cropping size
        1,   1,         // CropAlign, alignment of cropping rect
        160, 120,       // MinOutputSize, smallest bitmap stream can produce
        640, 480,       // MaxOutputSize, largest  bitmap stream can produce
        160, 120,       // OutputGranularity, granularity of output bitmap size
        0,   0,           // StretchTaps (0 no stretch, 1 pix dup, 2 interp...)
        0,   0,           // ShrinkTaps (0 no shrink, 1 pix dup, 2 interp...)
        FRAME_TIME_NTSC, // MinFrameInterval, 100 nS units
        FRAME_TIME_NTSC, // MaxFrameInterval, 100 nS units
        BIT_COUNT_16 * FRAME_RATE_NTSC * 160 * 120,  // MinBitsPerSecond;
        BIT_COUNT_16 * FRAME_RATE_NTSC * 640 * 480   // MaxBitsPerSecond;
    },
    //
    // KS_VIDEOINFOHEADER (default format)
    //
    {
        0,0,D_X,D_Y,                            // RECT  rcSource;
        0,0,D_X,D_Y,                            // RECT  rcTarget;
        D_X * D_Y * BIT_COUNT_16 * FRAME_RATE_NTSC, // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate;
        FRAME_TIME_NTSC,                         // REFERENCE_TIME  AvgTimePerFrame;
//    0,   // use AMINTERLACE_* defines. Reject connection if undefined bits are not 0
//    0, // use AMCOPYPROTECT_* defines. Reject connection if undefined bits are not 0
//    0, // X dimension of picture aspect ratio, e.g. 16 for 16x9 display
//    0, // Y dimension of picture aspect ratio, e.g.  9 for 16x9 display
//    0,        // must be 0; reject connection otherwise
//    0,        // must be 0; reject connection otherwise
        sizeof (KS_BITMAPINFOHEADER),       // DWORD biSize;
        D_X,                                // LONG  biWidth;
        D_Y,                                // LONG  biHeight;
        1,                                  // WORD  biPlanes;
        BIT_COUNT_16,                       // WORD  biBitCount;
        FOURCC_YUY2,                        // DWORD biCompression;
        D_X * D_Y * BIT_COUNT_16 / 8,       // DWORD biSizeImage;
        0,                                  // LONG  biXPelsPerMeter;
        0,                                  // LONG  biYPelsPerMeter;
        0,                                  // DWORD biClrUsed;
        0                                   // DWORD biClrImportant;
    }
};
