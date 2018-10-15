/*++

Copyright (c) 1998  Microsoft Corporation

Module Name:

   intelcam.c

Abstract:

   driver for the intel camera.

Author:

Notes:

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
  PURPOSE.
  

Environment:

   Kernel mode only


Revision History:

--*/

#include "warn.h"
#include "wdm.h"
#include "intelcam.h"
#include "strmini.h"
#include "prpobj.h"
#include "prpftn.h"

extern const GUID PROPSETID_CUSTOM_PROP;

ULONG INTELCAM_DebugTraceLevel
#ifdef MAX_DEBUG
    = MAX_TRACE;
#else
    = MIN_TRACE;
#endif

KSPIN_MEDIUM StandardMedium = {
    STATIC_KSMEDIUMSETID_Standard,
    0, 0
};

MaxPktSizePerInterface BusBWArray[BUS_BW_ARRAY_SIZE] = {
    {7, 0,NUM_100NANOSEC_UNITS_PERFRAME(0)},   // Interface # 7, Max. Pkt size 0, 0 fps (QCIF) 
    {1, 240,NUM_100NANOSEC_UNITS_PERFRAME(6)}, // Interface # 1, Max. Pkt size 240, 6 fps
    {0, 384,NUM_100NANOSEC_UNITS_PERFRAME(10)}, // Interface # 0, Max. pkt size 384, 10 fps
    {3, 464,NUM_100NANOSEC_UNITS_PERFRAME(12)}, // Interface # 3, Max. Pkt size 464, 12 fps
    {2, 576,NUM_100NANOSEC_UNITS_PERFRAME(15)}, // Interface # 2, Max. Pkt size 576, 15 fps
    {4, 688,NUM_100NANOSEC_UNITS_PERFRAME(18)}, // Interface # 4, Max. pkt size 688, 18 fps
    {5, 768,NUM_100NANOSEC_UNITS_PERFRAME(20)}, // Interface # 5, Max. Pkt size 768, 20 fps
    {6, 960,NUM_100NANOSEC_UNITS_PERFRAME(25)}, // Interface # 6, Max. Pkt size 960, 25 fps
};

#ifdef USBCAMD2

// this array is used for single-pin cameras
USBCAMD_Pipe_Config_Descriptor INTELCAM_PipeConfigArray1[INTELCAM_PIPECONFIG_LISTSIZE] = {
    {0, USBCAMD_SYNC_PIPE},
    {USBCAMD_VIDEO_STREAM,USBCAMD_DATA_PIPE  }
};
// this array is used for dual-pins cameras. 
USBCAMD_Pipe_Config_Descriptor INTELCAM_PipeConfigArray2[INTELCAM_PIPECONFIG_LISTSIZE] = {
    {0, USBCAMD_SYNC_PIPE},
    {USBCAMD_VIDEO_STILL_STREAM,USBCAMD_MULTIPLEX_PIPE  }
};

#endif

// ------------------------------------------------------------------------
// Property sets for all video capture streams
// ------------------------------------------------------------------------

DEFINE_KSPROPERTY_TABLE(VideoStreamConnectionProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CONNECTION_ALLOCATORFRAMING,
        TRUE,                                   // GetSupported or Handler
        sizeof(KSPROPERTY),                     // MinProperty
        sizeof(KSALLOCATOR_FRAMING),            // MinData
        FALSE,                                  // SetSupported or Handler
        NULL,                                   // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
};

DEFINE_KSPROPERTY_TABLE(VideoStreamDroppedFramesProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_DROPPEDFRAMES_CURRENT,
        TRUE,                                   // GetSupported or Handler
        sizeof(KSPROPERTY_DROPPEDFRAMES_CURRENT_S),// MinProperty
        sizeof(KSPROPERTY_DROPPEDFRAMES_CURRENT_S),// MinData
        FALSE,                                  // SetSupported or Handler
        NULL,                                   // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        0                                       // SerializedSize
    ),
};


// ------------------------------------------------------------------------
// Array of all of the property sets supported by video streams
// ------------------------------------------------------------------------

DEFINE_KSPROPERTY_SET_TABLE(VideoStreamProperties)
{
    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_Connection,                        // Set
        SIZEOF_ARRAY(VideoStreamConnectionProperties),  // PropertiesCount
        VideoStreamConnectionProperties,                // PropertyItem
        0,                                              // FastIoCount
        NULL                                            // FastIoTable
    ),
    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_DROPPEDFRAMES,                // Set
        SIZEOF_ARRAY(VideoStreamDroppedFramesProperties),  // PropertiesCount
        VideoStreamDroppedFramesProperties,                // PropertyItem
        0,                                              // FastIoCount
        NULL                                            // FastIoTable
    ),
};

#define NUMBER_VIDEO_STREAM_PROPERTIES (SIZEOF_ARRAY(VideoStreamProperties))


/*
 * The following are for FOURCC IYUV
 */
#define D_X 176
#define D_Y 144

KS_DATARANGE_VIDEO INTELCAM_StreamFormat_0 = 
{
    // KSDATARANGE
    {   
        sizeof (KS_DATARANGE_VIDEO),
        0,                      // Flags
        (D_X * D_Y * 12)/8,     // SampleSize
        0,                      // Reserved
//MEDIATYPE_Video
        STATIC_KSDATAFORMAT_TYPE_VIDEO,      
//MEDIASUBTYPE_IYUV        
        FCC_FORMAT_YUV12N, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    KS_VIDEOSTREAM_CAPTURE,   // StreamDescriptionFlags
    0,                  // MemoryAllocationFlags   (KS_VIDEO_ALLOC_*)

    // _KS_VIDEO_STREAM_CONFIG_CAPS  
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,
        KS_AnalogVideo_None,    // VideoStandard
        D_X, D_Y,               // InputSize, (the inherent size of the incoming signal
                                //             with every digitized pixel unique)
        D_X, D_Y,               // MinCroppingSize, smallest rcSrc cropping rect allowed
        D_X, D_Y,               // MaxCroppingSize, largest  rcSrc cropping rect allowed
        1,                      // CropGranularityX, granularity of cropping size
        1,                      // CropGranularityY
        1,                      // CropAlignX, alignment of cropping rect 
        1,                      // CropAlignY;
        D_X, D_Y,               // MinOutputSize, smallest bitmap stream can produce
        D_X, D_Y,               // MaxOutputSize, largest  bitmap stream can produce
        1,                      // OutputGranularityX, granularity of output bitmap size
        1,                      // OutputGranularityY;
        0,                      // StretchTapsX
    	0,                      // StretchTapsY
        0,                      // ShrinkTapsX
    	0,                      // ShrinkTapsY
        400000,                 // MinFrameInterval, 100 nS units
        1666667,                 // MaxFrameInterval, 100 nS units
        12 * 6 * D_X * D_Y,     // MinBitsPerSecond;
        12 * 25 * D_X * D_Y      // MaxBitsPerSecond;
    }, 
        

    // KS_VIDEOINFOHEADER (default format)
    {
        0,0,0,0,                        // RECT  rcSource; 
        0,0,0,0,                        // RECT  rcTarget; 
        D_X * D_Y * 12 * 25,                 // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate; 
        400000,                             // REFERENCE_TIME  AvgTimePerFrame;   
        sizeof (KS_BITMAPINFOHEADER),       //    DWORD      biSize;
        D_X,                                //    LONG       biWidth;
        D_Y,                                //    LONG       biHeight;
        1,                                  //    WORD       biPlanes;
        12,                                 //    WORD       biBitCount;
        FCC_FORMAT_YUV12N,                  //    DWORD      biCompression;
        (D_X * D_Y * 12)/8,                 //    DWORD      biSizeImage;
        0,                                  //    LONG       biXPelsPerMeter;
        0,                                  //    LONG       biYPelsPerMeter;
        0,                                  //    DWORD      biClrUsed;
        0                                   //    DWORD      biClrImportant;

    }
}; 

#undef D_X
#undef D_Y
        

#define D_X 160
#define D_Y 120

KS_DATARANGE_VIDEO INTELCAM_StreamFormat_1 = 
{
    // KSDATARANGE
    {   
        sizeof (KS_DATARANGE_VIDEO),
        0,                      // Flags
        (D_X * D_Y * 12)/8,     // SampleSize
        0,                      // Reserved
//MEDIATYPE_Video
        STATIC_KSDATAFORMAT_TYPE_VIDEO,      
//  MEDIASUBTYPE_IYUV
        FCC_FORMAT_YUV12N, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    KS_VIDEOSTREAM_CAPTURE,   // StreamDescriptionFlags
    0,                  // MemoryAllocationFlags   (KS_VIDEO_ALLOC_*)

    // _KS_VIDEO_STREAM_CONFIG_CAPS  
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,
        KS_AnalogVideo_None,    // VideoStandard
        D_X, D_Y,               // InputSize, (the inherent size of the incoming signal
                                //             with every digitized pixel unique)
        D_X, D_Y,               // MinCroppingSize, smallest rcSrc cropping rect allowed
        D_X, D_Y,               // MaxCroppingSize, largest  rcSrc cropping rect allowed
        1,                      // CropGranularityX, granularity of cropping size
        1,                      // CropGranularityY
        1,                      // CropAlignX, alignment of cropping rect 
        1,                      // CropAlignY;
        D_X, D_Y,               // MinOutputSize, smallest bitmap stream can produce
        D_X, D_Y,               // MaxOutputSize, largest  bitmap stream can produce
        1,                      // OutputGranularityX, granularity of output bitmap size
        1,                      // OutputGranularityY;
        0,                      // StretchTapsX
    	0,                      // StretchTapsY
        0,                      // ShrinkTapsX
    	0,                      // ShrinkTapsY
        300000,         // MinFrameInterval, 100 nS units
        1249999,         // MaxFrameInterval, 100 nS units
        12 * 8 * D_X * D_Y,  // MinBitsPerSecond;
        12 * 33 * D_X * D_Y   // MaxBitsPerSecond;
    }, 
        
    // KS_VIDEOINFOHEADER (default format)
    {
        0,0,0,0,                        // RECT  rcSource; 
        0,0,0,0,                        // RECT  rcTarget; 
        D_X * D_Y * 12 * 24,                 // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate; 
        416666,                             // REFERENCE_TIME  AvgTimePerFrame;   
        sizeof (KS_BITMAPINFOHEADER),       //    DWORD      biSize;
        D_X,                                //    LONG       biWidth;
        D_Y,                                //    LONG       biHeight;
        1,                                  //    WORD       biPlanes;
        12,                                 //    WORD       biBitCount;
        FCC_FORMAT_YUV12N,                  //    DWORD      biCompression;
        (D_X * D_Y * 12)/8,                 //    DWORD      biSizeImage;
        0,                                  //    LONG       biXPelsPerMeter;
        0,                                  //    LONG       biYPelsPerMeter;
        0,                                  //    DWORD      biClrUsed;
        0                                   //    DWORD      biClrImportant;

    }
}; 
    

#undef D_X
#undef D_Y

#define D_X 320
#define D_Y 240

KS_DATARANGE_VIDEO INTELCAM_StreamFormat_2 = 
{
    // KSDATARANGE
    {   
        sizeof (KS_DATARANGE_VIDEO),
        0,                      // Flags
        (D_X * D_Y * 12)/8,     // SampleSize
        0,                      // Reserved
//MEDIATYPE_Video
        STATIC_KSDATAFORMAT_TYPE_VIDEO,      
//MEDIASUBTYPE_IYUV        
        FCC_FORMAT_YUV12N, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    KS_VIDEOSTREAM_CAPTURE,   // StreamDescriptionFlags
    0,                  // MemoryAllocationFlags   (KS_VIDEO_ALLOC_*)

    // _KS_VIDEO_STREAM_CONFIG_CAPS  
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,
        KS_AnalogVideo_None,    // VideoStandard
        D_X, D_Y,               // InputSize, (the inherent size of the incoming signal
                                //             with every digitized pixel unique)
        D_X, D_Y,               // MinCroppingSize, smallest rcSrc cropping rect allowed
        D_X, D_Y,               // MaxCroppingSize, largest  rcSrc cropping rect allowed
        1,                      // CropGranularityX, granularity of cropping size
        1,                      // CropGranularityY
        1,                      // CropAlignX, alignment of cropping rect 
        1,                      // CropAlignY;
        D_X, D_Y,               // MinOutputSize, smallest bitmap stream can produce
        D_X, D_Y,               // MaxOutputSize, largest  bitmap stream can produce
        1,                      // OutputGranularityX, granularity of output bitmap size
        1,                      // OutputGranularityY;
        0,                      // StretchTapsX
    	0,                      // StretchTapsY
        0,                      // ShrinkTapsX
    	0,                      // ShrinkTapsY
        1200000,                 // MinFrameInterval, 100 nS units
        4999998,                 // MaxFrameInterval, 100 nS units
        12 * 2 * D_X * D_Y,  // MinBitsPerSecond;
        12 * 8 * D_X * D_Y   // MaxBitsPerSecond;
    }, 
        
    // KS_VIDEOINFOHEADER (default format)
    {
        0,0,0,0,                        // RECT  rcSource; 
        0,0,0,0,                        // RECT  rcTarget; 
        D_X * D_Y * 12 * 6,                 // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate; 
        1666665,                             // REFERENCE_TIME  AvgTimePerFrame;   

        sizeof (KS_BITMAPINFOHEADER),       //    DWORD      biSize;
        D_X,                                //    LONG       biWidth;
        D_Y,                                //    LONG       biHeight;
        1,                                  //    WORD       biPlanes;
        12,                                 //    WORD       biBitCount;
        FCC_FORMAT_YUV12N,                  //    DWORD      biCompression;
        (D_X * D_Y * 12)/8,                 //    DWORD      biSizeImage;
        0,                                  //    LONG       biXPelsPerMeter;
        0,                                  //    LONG       biYPelsPerMeter;
        0,                                  //    DWORD      biClrUsed;
        0                                   //    DWORD      biClrImportant;

    }
}; 
    
#undef D_X
#undef D_Y



/*
 * The following are for FOURCC I420
 */
#define D_X 176
#define D_Y 144

KS_DATARANGE_VIDEO INTELCAM_StreamFormat_3 = 
{
    // KSDATARANGE
    {   
        sizeof (KS_DATARANGE_VIDEO),
        0,                      // Flags
        (D_X * D_Y * 12)/8,     // SampleSize
        0,                      // Reserved
//MEDIATYPE_Video
        STATIC_KSDATAFORMAT_TYPE_VIDEO,      
//MEDIASUBTYPE_I420        
        FCC_FORMAT_YUV12A, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    KS_VIDEOSTREAM_CAPTURE,   // StreamDescriptionFlags
    0,                  // MemoryAllocationFlags   (KS_VIDEO_ALLOC_*)

    // _KS_VIDEO_STREAM_CONFIG_CAPS  
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,
        KS_AnalogVideo_None,    // VideoStandard
        D_X, D_Y,               // InputSize, (the inherent size of the incoming signal
                                //             with every digitized pixel unique)
        D_X, D_Y,               // MinCroppingSize, smallest rcSrc cropping rect allowed
        D_X, D_Y,               // MaxCroppingSize, largest  rcSrc cropping rect allowed
        1,                      // CropGranularityX, granularity of cropping size
        1,                      // CropGranularityY
        1,                      // CropAlignX, alignment of cropping rect 
        1,                      // CropAlignY;
        D_X, D_Y,               // MinOutputSize, smallest bitmap stream can produce
        D_X, D_Y,               // MaxOutputSize, largest  bitmap stream can produce
        1,                      // OutputGranularityX, granularity of output bitmap size
        1,                      // OutputGranularityY;
        0,                      // StretchTapsX
    	0,                      // StretchTapsY
        0,                      // ShrinkTapsX
    	0,                      // ShrinkTapsY
        400000,                 // MinFrameInterval, 100 nS units
        1666667,                 // MaxFrameInterval, 100 nS units
        12 * 6 * D_X * D_Y,     // MinBitsPerSecond;
        12 * 25 * D_X * D_Y      // MaxBitsPerSecond;
    }, 
        

    // KS_VIDEOINFOHEADER (default format)
    {
        0,0,0,0,                        // RECT  rcSource; 
        0,0,0,0,                        // RECT  rcTarget; 
        D_X * D_Y * 12 * 25,                 // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate; 
        400000,                             // REFERENCE_TIME  AvgTimePerFrame;   
        sizeof (KS_BITMAPINFOHEADER),       //    DWORD      biSize;
        D_X,                                //    LONG       biWidth;
        D_Y,                                //    LONG       biHeight;
        1,                                  //    WORD       biPlanes;
        12,                                 //    WORD       biBitCount;
        FCC_FORMAT_YUV12A,                  //    DWORD      biCompression;
        (D_X * D_Y * 12)/8,                 //    DWORD      biSizeImage;
        0,                                  //    LONG       biXPelsPerMeter;
        0,                                  //    LONG       biYPelsPerMeter;
        0,                                  //    DWORD      biClrUsed;
        0                                   //    DWORD      biClrImportant;

    }
}; 

#undef D_X
#undef D_Y


#define D_X 160
#define D_Y 120

KS_DATARANGE_VIDEO INTELCAM_StreamFormat_4 = 
//??KS_DATARANGE_VIDEO INTELCAM_StreamFormat_0 = 
{
    // KSDATARANGE
    {   
        sizeof (KS_DATARANGE_VIDEO),
        0,                      // Flags
        (D_X * D_Y * 12)/8,     // SampleSize
        0,                      // Reserved
//MEDIATYPE_Video
        STATIC_KSDATAFORMAT_TYPE_VIDEO,      
//  MEDIASUBTYPE_I420
        FCC_FORMAT_YUV12A, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    KS_VIDEOSTREAM_CAPTURE,   // StreamDescriptionFlags
    0,                  // MemoryAllocationFlags   (KS_VIDEO_ALLOC_*)

    // _KS_VIDEO_STREAM_CONFIG_CAPS  
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,
        KS_AnalogVideo_None,    // VideoStandard
        D_X, D_Y,               // InputSize, (the inherent size of the incoming signal
                                //             with every digitized pixel unique)
        D_X, D_Y,               // MinCroppingSize, smallest rcSrc cropping rect allowed
        D_X, D_Y,               // MaxCroppingSize, largest  rcSrc cropping rect allowed
        1,                      // CropGranularityX, granularity of cropping size
        1,                      // CropGranularityY
        1,                      // CropAlignX, alignment of cropping rect 
        1,                      // CropAlignY;
        D_X, D_Y,               // MinOutputSize, smallest bitmap stream can produce
        D_X, D_Y,               // MaxOutputSize, largest  bitmap stream can produce
        1,                      // OutputGranularityX, granularity of output bitmap size
        1,                      // OutputGranularityY;
        0,                      // StretchTapsX
    	0,                      // StretchTapsY
        0,                      // ShrinkTapsX
    	0,                      // ShrinkTapsY
        300000,         // MinFrameInterval, 100 nS units
        1249999,         // MaxFrameInterval, 100 nS units
        12 * 8 * D_X * D_Y,  // MinBitsPerSecond;
        12 * 33 * D_X * D_Y   // MaxBitsPerSecond;
    }, 
        
    // KS_VIDEOINFOHEADER (default format)
    {
        0,0,0,0,                        // RECT  rcSource; 
        0,0,0,0,                        // RECT  rcTarget; 
        D_X * D_Y * 12 * 24,                 // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate; 
        416666,                             // REFERENCE_TIME  AvgTimePerFrame;   
        sizeof (KS_BITMAPINFOHEADER),       //    DWORD      biSize;
        D_X,                                //    LONG       biWidth;
        D_Y,                                //    LONG       biHeight;
        1,                                  //    WORD       biPlanes;
        12,                                 //    WORD       biBitCount;
        FCC_FORMAT_YUV12A,                  //    DWORD      biCompression;
        (D_X * D_Y * 12)/8,                 //    DWORD      biSizeImage;
        0,                                  //    LONG       biXPelsPerMeter;
        0,                                  //    LONG       biYPelsPerMeter;
        0,                                  //    DWORD      biClrUsed;
        0                                   //    DWORD      biClrImportant;

    }
}; 
    
#undef D_X
#undef D_Y
    

#define D_X 320
#define D_Y 240

KS_DATARANGE_VIDEO INTELCAM_StreamFormat_5 = 
{
    // KSDATARANGE
    {   
        sizeof (KS_DATARANGE_VIDEO),
        0,                      // Flags
        (D_X * D_Y * 12)/8,     // SampleSize
        0,                      // Reserved
//MEDIATYPE_Video
        STATIC_KSDATAFORMAT_TYPE_VIDEO,      
//MEDIASUBTYPE_I420        
        FCC_FORMAT_YUV12A, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    KS_VIDEOSTREAM_CAPTURE,   // StreamDescriptionFlags
    0,                  // MemoryAllocationFlags   (KS_VIDEO_ALLOC_*)

    // _KS_VIDEO_STREAM_CONFIG_CAPS  
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,
        KS_AnalogVideo_None,    // VideoStandard
        D_X, D_Y,               // InputSize, (the inherent size of the incoming signal
                                //             with every digitized pixel unique)
        D_X, D_Y,               // MinCroppingSize, smallest rcSrc cropping rect allowed
        D_X, D_Y,               // MaxCroppingSize, largest  rcSrc cropping rect allowed
        1,                      // CropGranularityX, granularity of cropping size
        1,                      // CropGranularityY
        1,                      // CropAlignX, alignment of cropping rect 
        1,                      // CropAlignY;
        D_X, D_Y,               // MinOutputSize, smallest bitmap stream can produce
        D_X, D_Y,               // MaxOutputSize, largest  bitmap stream can produce
        1,                      // OutputGranularityX, granularity of output bitmap size
        1,                      // OutputGranularityY;
        0,                      // StretchTapsX
    	0,                      // StretchTapsY
        0,                      // ShrinkTapsX
    	0,                      // ShrinkTapsY
        1200000,                 // MinFrameInterval, 100 nS units
        4999998,                 // MaxFrameInterval, 100 nS units
        12 * 2 * D_X * D_Y,  // MinBitsPerSecond;
        12 * 8 * D_X * D_Y   // MaxBitsPerSecond;
    }, 
        
    // KS_VIDEOINFOHEADER (default format)
    {
        0,0,0,0,                        // RECT  rcSource; 
        0,0,0,0,                        // RECT  rcTarget; 
        D_X * D_Y * 12 * 6,                 // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate; 
        1666665,                             // REFERENCE_TIME  AvgTimePerFrame;   

        sizeof (KS_BITMAPINFOHEADER),       //    DWORD      biSize;
        D_X,                                //    LONG       biWidth;
        D_Y,                                //    LONG       biHeight;
        1,                                  //    WORD       biPlanes;
        12,                                 //    WORD       biBitCount;
        FCC_FORMAT_YUV12A,                  //    DWORD      biCompression;
        (D_X * D_Y * 12)/8,                 //    DWORD      biSizeImage;
        0,                                  //    LONG       biXPelsPerMeter;
        0,                                  //    LONG       biYPelsPerMeter;
        0,                                  //    DWORD      biClrUsed;
        0                                   //    DWORD      biClrImportant;

    }
}; 
    
#undef D_X
#undef D_Y


//---------------------------------------------------------------------------
//  INTELCAM_Stream Formats
//---------------------------------------------------------------------------
PKSDATAFORMAT INTELCAM_VideoStreamFormats[] = 
{
    (PKSDATAFORMAT) &INTELCAM_StreamFormat_0,
    (PKSDATAFORMAT) &INTELCAM_StreamFormat_1,
    (PKSDATAFORMAT) &INTELCAM_StreamFormat_2,
    (PKSDATAFORMAT) &INTELCAM_StreamFormat_3,
    (PKSDATAFORMAT) &INTELCAM_StreamFormat_4,
    (PKSDATAFORMAT) &INTELCAM_StreamFormat_5
};

#define NUM_INTELCAM_VIDEO_STREAM_FORMATS \
             (sizeof (INTELCAM_VideoStreamFormats) / sizeof (PKSDATAFORMAT))

/*
 *  Still Stream Format
 */

#define D_X 160
#define D_Y 120

KS_DATARANGE_VIDEO INTELCAM_StillFormat_0 = 
{
    // KSDATARANGE
    {   
        sizeof (KS_DATARANGE_VIDEO),
        0,                      // Flags
        (D_X * D_Y * 12)/8,     // SampleSize
        0,                      // Reserved
//MEDIATYPE_Video
        STATIC_KSDATAFORMAT_TYPE_VIDEO,      
//  MEDIASUBTYPE_IYUV
        FCC_FORMAT_YUV12N, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    KS_VIDEOSTREAM_STILL,   // StreamDescriptionFlags
    0,                  // MemoryAllocationFlags   (KS_VIDEO_ALLOC_*)

    // _KS_VIDEO_STREAM_CONFIG_CAPS  
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,
        KS_AnalogVideo_None,    // VideoStandard
        D_X, D_Y,               // InputSize, (the inherent size of the incoming signal
                                //             with every digitized pixel unique)
        D_X, D_Y,               // MinCroppingSize, smallest rcSrc cropping rect allowed
        D_X, D_Y,               // MaxCroppingSize, largest  rcSrc cropping rect allowed
        1,                      // CropGranularityX, granularity of cropping size
        1,                      // CropGranularityY
        1,                      // CropAlignX, alignment of cropping rect 
        1,                      // CropAlignY;
        D_X, D_Y,               // MinOutputSize, smallest bitmap stream can produce
        D_X, D_Y,               // MaxOutputSize, largest  bitmap stream can produce
        1,                      // OutputGranularityX, granularity of output bitmap size
        1,                      // OutputGranularityY;
        0,                      // StretchTapsX
    	0,                      // StretchTapsY
        0,                      // ShrinkTapsX
    	0,                      // ShrinkTapsY
        10000000/1,         // MinFrameInterval, 100 nS units
        10000000/1,         // MaxFrameInterval, 100 nS units
        12 * 1 * D_X * D_Y,  // MinBitsPerSecond;
        12 * 1 * D_X * D_Y   // MaxBitsPerSecond;
    }, 
        
    // KS_VIDEOINFOHEADER (default format)
    {
        0,0,0,0,                        // RECT  rcSource; 
        0,0,0,0,                        // RECT  rcTarget; 
        D_X * D_Y * 12 * 1,                 // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate; 
        10000000 /1,                             // REFERENCE_TIME  AvgTimePerFrame;   
        sizeof (KS_BITMAPINFOHEADER),       //    DWORD      biSize;
        D_X,                                //    LONG       biWidth;
        D_Y,                                //    LONG       biHeight;
        1,                                  //    WORD       biPlanes;
        12,                                 //    WORD       biBitCount;
        FCC_FORMAT_YUV12N,                  //    DWORD      biCompression;
        (D_X * D_Y * 12)/8,                 //    DWORD      biSizeImage;
        0,                                  //    LONG       biXPelsPerMeter;
        0,                                  //    LONG       biYPelsPerMeter;
        0,                                  //    DWORD      biClrUsed;
        0                                   //    DWORD      biClrImportant;

    }
}; 
   
#undef D_X
#undef D_Y

#define D_X 176
#define D_Y 144

KS_DATARANGE_VIDEO INTELCAM_StillFormat_1 = 
{
    // KSDATARANGE
    {   
        sizeof (KS_DATARANGE_VIDEO),
        0,                      // Flags
        (D_X * D_Y * 12)/8,     // SampleSize
        0,                      // Reserved
//MEDIATYPE_Video
        STATIC_KSDATAFORMAT_TYPE_VIDEO,      
//MEDIASUBTYPE_IYUV        
        FCC_FORMAT_YUV12N, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    KS_VIDEOSTREAM_STILL,   // StreamDescriptionFlags
    0,                  // MemoryAllocationFlags   (KS_VIDEO_ALLOC_*)

    // _KS_VIDEO_STREAM_CONFIG_CAPS  
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,
        KS_AnalogVideo_None,    // VideoStandard
        D_X, D_Y,               // InputSize, (the inherent size of the incoming signal
                                //             with every digitized pixel unique)
        D_X, D_Y,               // MinCroppingSize, smallest rcSrc cropping rect allowed
        D_X, D_Y,               // MaxCroppingSize, largest  rcSrc cropping rect allowed
        1,                      // CropGranularityX, granularity of cropping size
        1,                      // CropGranularityY
        1,                      // CropAlignX, alignment of cropping rect 
        1,                      // CropAlignY;
        D_X, D_Y,               // MinOutputSize, smallest bitmap stream can produce
        D_X, D_Y,               // MaxOutputSize, largest  bitmap stream can produce
        1,                      // OutputGranularityX, granularity of output bitmap size
        1,                      // OutputGranularityY;
        0,                      // StretchTapsX
    	0,                      // StretchTapsY
        0,                      // ShrinkTapsX
    	0,                      // ShrinkTapsY
        10000000/1,         // MinFrameInterval, 100 nS units
        10000000/1,         // MaxFrameInterval, 100 nS units
        12 * 1 * D_X * D_Y,  // MinBitsPerSecond;
        12 * 1 * D_X * D_Y   // MaxBitsPerSecond;
    }, 
        
    // KS_VIDEOINFOHEADER (default format)
    {
        0,0,0,0,                        // RECT  rcSource; 
        0,0,0,0,                        // RECT  rcTarget; 
        D_X * D_Y * 12 * 1,                 // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate; 
        10000000 /1,                             // REFERENCE_TIME  AvgTimePerFrame;   
        sizeof (KS_BITMAPINFOHEADER),       //    DWORD      biSize;
        D_X,                                //    LONG       biWidth;
        D_Y,                                //    LONG       biHeight;
        1,                                  //    WORD       biPlanes;
        12,                                 //    WORD       biBitCount;
        FCC_FORMAT_YUV12N,                  //    DWORD      biCompression;
        (D_X * D_Y * 12)/8,                 //    DWORD      biSizeImage;
        0,                                  //    LONG       biXPelsPerMeter;
        0,                                  //    LONG       biYPelsPerMeter;
        0,                                  //    DWORD      biClrUsed;
        0                                   //    DWORD      biClrImportant;

    }
}; 

#undef D_X
#undef D_Y

#define D_X 320
#define D_Y 240

KS_DATARANGE_VIDEO INTELCAM_StillFormat_2 = 
{
    // KSDATARANGE
    {   
        sizeof (KS_DATARANGE_VIDEO),
        0,                      // Flags
        (D_X * D_Y * 12)/8,     // SampleSize
        0,                      // Reserved
//MEDIATYPE_Video
        STATIC_KSDATAFORMAT_TYPE_VIDEO,      
//MEDIASUBTYPE_IYUV        
        FCC_FORMAT_YUV12N, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    KS_VIDEOSTREAM_STILL,   // StreamDescriptionFlags
    0,                  // MemoryAllocationFlags   (KS_VIDEO_ALLOC_*)

    // _KS_VIDEO_STREAM_CONFIG_CAPS  
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,
        KS_AnalogVideo_None,    // VideoStandard
        D_X, D_Y,               // InputSize, (the inherent size of the incoming signal
                                //             with every digitized pixel unique)
        D_X, D_Y,               // MinCroppingSize, smallest rcSrc cropping rect allowed
        D_X, D_Y,               // MaxCroppingSize, largest  rcSrc cropping rect allowed
        1,                      // CropGranularityX, granularity of cropping size
        1,                      // CropGranularityY
        1,                      // CropAlignX, alignment of cropping rect 
        1,                      // CropAlignY;
        D_X, D_Y,               // MinOutputSize, smallest bitmap stream can produce
        D_X, D_Y,               // MaxOutputSize, largest  bitmap stream can produce
        1,                      // OutputGranularityX, granularity of output bitmap size
        1,                      // OutputGranularityY;
        0,                      // StretchTapsX
    	0,                      // StretchTapsY
        0,                      // ShrinkTapsX
    	0,                      // ShrinkTapsY
        10000000/1,         // MinFrameInterval, 100 nS units
        10000000/1,         // MaxFrameInterval, 100 nS units
        12 * 1 * D_X * D_Y,  // MinBitsPerSecond;
        12 * 1 * D_X * D_Y   // MaxBitsPerSecond;
    }, 
        
    // KS_VIDEOINFOHEADER (default format)
    {
        0,0,0,0,                        // RECT  rcSource; 
        0,0,0,0,                        // RECT  rcTarget; 
        D_X * D_Y * 12 * 1,                 // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate; 
        10000000 /1,                             // REFERENCE_TIME  AvgTimePerFrame;   
        sizeof (KS_BITMAPINFOHEADER),       //    DWORD      biSize;
        D_X,                                //    LONG       biWidth;
        D_Y,                                //    LONG       biHeight;
        1,                                  //    WORD       biPlanes;
        12,                                 //    WORD       biBitCount;
        FCC_FORMAT_YUV12N,                  //    DWORD      biCompression;
        (D_X * D_Y * 12)/8,                 //    DWORD      biSizeImage;
        0,                                  //    LONG       biXPelsPerMeter;
        0,                                  //    LONG       biYPelsPerMeter;
        0,                                  //    DWORD      biClrUsed;
        0                                   //    DWORD      biClrImportant;

    }
}; 
   
#undef D_X
#undef D_Y

/*
 * The following are for FOURCC I420
 */

#define D_X 160
#define D_Y 120

KS_DATARANGE_VIDEO INTELCAM_StillFormat_3 = 
//??KS_DATARANGE_VIDEO INTELCAM_StreamFormat_0 = 
{
    // KSDATARANGE
    {   
        sizeof (KS_DATARANGE_VIDEO),
        0,                      // Flags
        (D_X * D_Y * 12)/8,     // SampleSize
        0,                      // Reserved
//MEDIATYPE_Video
        STATIC_KSDATAFORMAT_TYPE_VIDEO,      
//  MEDIASUBTYPE_I420
        FCC_FORMAT_YUV12A, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    KS_VIDEOSTREAM_STILL,   // StreamDescriptionFlags
    0,                  // MemoryAllocationFlags   (KS_VIDEO_ALLOC_*)

    // _KS_VIDEO_STREAM_CONFIG_CAPS  
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,
        KS_AnalogVideo_None,    // VideoStandard
        D_X, D_Y,               // InputSize, (the inherent size of the incoming signal
                                //             with every digitized pixel unique)
        D_X, D_Y,               // MinCroppingSize, smallest rcSrc cropping rect allowed
        D_X, D_Y,               // MaxCroppingSize, largest  rcSrc cropping rect allowed
        1,                      // CropGranularityX, granularity of cropping size
        1,                      // CropGranularityY
        1,                      // CropAlignX, alignment of cropping rect 
        1,                      // CropAlignY;
        D_X, D_Y,               // MinOutputSize, smallest bitmap stream can produce
        D_X, D_Y,               // MaxOutputSize, largest  bitmap stream can produce
        1,                      // OutputGranularityX, granularity of output bitmap size
        1,                      // OutputGranularityY;
        0,                      // StretchTapsX
    	0,                      // StretchTapsY
        0,                      // ShrinkTapsX
    	0,                      // ShrinkTapsY
        10000000/1,         // MinFrameInterval, 100 nS units
        10000000/1,         // MaxFrameInterval, 100 nS units
        12 * 1 * D_X * D_Y,  // MinBitsPerSecond;
        12 * 1 * D_X * D_Y   // MaxBitsPerSecond;
    }, 
        
    // KS_VIDEOINFOHEADER (default format)
    {
        0,0,0,0,                        // RECT  rcSource; 
        0,0,0,0,                        // RECT  rcTarget; 
        D_X * D_Y * 12 * 1,                 // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate; 
        10000000 /1,                             // REFERENCE_TIME  AvgTimePerFrame;   
        sizeof (KS_BITMAPINFOHEADER),       //    DWORD      biSize;
        D_X,                                //    LONG       biWidth;
        D_Y,                                //    LONG       biHeight;
        1,                                  //    WORD       biPlanes;
        12,                                 //    WORD       biBitCount;
        FCC_FORMAT_YUV12A,                  //    DWORD      biCompression;
        (D_X * D_Y * 12)/8,                 //    DWORD      biSizeImage;
        0,                                  //    LONG       biXPelsPerMeter;
        0,                                  //    LONG       biYPelsPerMeter;
        0,                                  //    DWORD      biClrUsed;
        0                                   //    DWORD      biClrImportant;

    }
}; 
   
#undef D_X
#undef D_Y
 
#define D_X 176
#define D_Y 144

KS_DATARANGE_VIDEO INTELCAM_StillFormat_4 = 
{
    // KSDATARANGE
    {   
        sizeof (KS_DATARANGE_VIDEO),
        0,                      // Flags
        (D_X * D_Y * 12)/8,     // SampleSize
        0,                      // Reserved
//MEDIATYPE_Video
        STATIC_KSDATAFORMAT_TYPE_VIDEO,      
//MEDIASUBTYPE_I420        
        FCC_FORMAT_YUV12A, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    KS_VIDEOSTREAM_STILL,   // StreamDescriptionFlags
    0,                  // MemoryAllocationFlags   (KS_VIDEO_ALLOC_*)

    // _KS_VIDEO_STREAM_CONFIG_CAPS  
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,
        KS_AnalogVideo_None,    // VideoStandard
        D_X, D_Y,               // InputSize, (the inherent size of the incoming signal
                                //             with every digitized pixel unique)
        D_X, D_Y,               // MinCroppingSize, smallest rcSrc cropping rect allowed
        D_X, D_Y,               // MaxCroppingSize, largest  rcSrc cropping rect allowed
        1,                      // CropGranularityX, granularity of cropping size
        1,                      // CropGranularityY
        1,                      // CropAlignX, alignment of cropping rect 
        1,                      // CropAlignY;
        D_X, D_Y,               // MinOutputSize, smallest bitmap stream can produce
        D_X, D_Y,               // MaxOutputSize, largest  bitmap stream can produce
        1,                      // OutputGranularityX, granularity of output bitmap size
        1,                      // OutputGranularityY;
        0,                      // StretchTapsX
    	0,                      // StretchTapsY
        0,                      // ShrinkTapsX
    	0,                      // ShrinkTapsY
        10000000/1,         // MinFrameInterval, 100 nS units
        10000000/1,         // MaxFrameInterval, 100 nS units
        12 * 1 * D_X * D_Y,  // MinBitsPerSecond;
        12 * 1 * D_X * D_Y   // MaxBitsPerSecond;
    }, 
        
    // KS_VIDEOINFOHEADER (default format)
    {
        0,0,0,0,                        // RECT  rcSource; 
        0,0,0,0,                        // RECT  rcTarget; 
        D_X * D_Y * 12 * 1,                 // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate; 
        10000000 /1,                             // REFERENCE_TIME  AvgTimePerFrame;   
        sizeof (KS_BITMAPINFOHEADER),       //    DWORD      biSize;
        D_X,                                //    LONG       biWidth;
        D_Y,                                //    LONG       biHeight;
        1,                                  //    WORD       biPlanes;
        12,                                 //    WORD       biBitCount;
        FCC_FORMAT_YUV12A,                  //    DWORD      biCompression;
        (D_X * D_Y * 12)/8,                 //    DWORD      biSizeImage;
        0,                                  //    LONG       biXPelsPerMeter;
        0,                                  //    LONG       biYPelsPerMeter;
        0,                                  //    DWORD      biClrUsed;
        0                                   //    DWORD      biClrImportant;

    }
}; 
#undef D_X
#undef D_Y

#define D_X 320
#define D_Y 240

KS_DATARANGE_VIDEO INTELCAM_StillFormat_5 = 
{
    // KSDATARANGE
    {   
        sizeof (KS_DATARANGE_VIDEO),
        0,                      // Flags
        (D_X * D_Y * 12)/8,     // SampleSize
        0,                      // Reserved
//MEDIATYPE_Video
        STATIC_KSDATAFORMAT_TYPE_VIDEO,      
//MEDIASUBTYPE_I420        
        FCC_FORMAT_YUV12A, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71,
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO
    },

    TRUE,               // BOOL,  bFixedSizeSamples (all samples same size?)
    TRUE,               // BOOL,  bTemporalCompression (all I frames?)
    KS_VIDEOSTREAM_STILL,   // StreamDescriptionFlags
    0,                  // MemoryAllocationFlags   (KS_VIDEO_ALLOC_*)

    // _KS_VIDEO_STREAM_CONFIG_CAPS  
    {
        STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO,
        KS_AnalogVideo_None,    // VideoStandard
        D_X, D_Y,               // InputSize, (the inherent size of the incoming signal
                                //             with every digitized pixel unique)
        D_X, D_Y,               // MinCroppingSize, smallest rcSrc cropping rect allowed
        D_X, D_Y,               // MaxCroppingSize, largest  rcSrc cropping rect allowed
        1,                      // CropGranularityX, granularity of cropping size
        1,                      // CropGranularityY
        1,                      // CropAlignX, alignment of cropping rect 
        1,                      // CropAlignY;
        D_X, D_Y,               // MinOutputSize, smallest bitmap stream can produce
        D_X, D_Y,               // MaxOutputSize, largest  bitmap stream can produce
        1,                      // OutputGranularityX, granularity of output bitmap size
        1,                      // OutputGranularityY;
        0,                      // StretchTapsX
    	0,                      // StretchTapsY
        0,                      // ShrinkTapsX
    	0,                      // ShrinkTapsY
        10000000/1,         // MinFrameInterval, 100 nS units
        10000000/1,         // MaxFrameInterval, 100 nS units
        12 * 1 * D_X * D_Y,  // MinBitsPerSecond;
        12 * 1 * D_X * D_Y   // MaxBitsPerSecond;
    }, 
        
    // KS_VIDEOINFOHEADER (default format)
    {
        0,0,0,0,                        // RECT  rcSource; 
        0,0,0,0,                        // RECT  rcTarget; 
        D_X * D_Y * 12 * 1,                 // DWORD dwBitRate;
        0L,                                 // DWORD dwBitErrorRate; 
        10000000 /1,                             // REFERENCE_TIME  AvgTimePerFrame;   
        sizeof (KS_BITMAPINFOHEADER),       //    DWORD      biSize;
        D_X,                                //    LONG       biWidth;
        D_Y,                                //    LONG       biHeight;
        1,                                  //    WORD       biPlanes;
        12,                                 //    WORD       biBitCount;
        FCC_FORMAT_YUV12A,                  //    DWORD      biCompression;
        (D_X * D_Y * 12)/8,                 //    DWORD      biSizeImage;
        0,                                  //    LONG       biXPelsPerMeter;
        0,                                  //    LONG       biYPelsPerMeter;
        0,                                  //    DWORD      biClrUsed;
        0                                   //    DWORD      biClrImportant;

    }
}; 
 
#undef D_X
#undef D_Y



PKSDATAFORMAT INTELCAM_StillStreamFormats[] = 
{
    (PKSDATAFORMAT) &INTELCAM_StillFormat_0,
    (PKSDATAFORMAT) &INTELCAM_StillFormat_1,
    (PKSDATAFORMAT) &INTELCAM_StillFormat_2,
    (PKSDATAFORMAT) &INTELCAM_StillFormat_3,
    (PKSDATAFORMAT) &INTELCAM_StillFormat_4,
    (PKSDATAFORMAT) &INTELCAM_StillFormat_5
};

#define NUM_INTELCAM_STILL_STREAM_FORMATS \
             (sizeof (INTELCAM_StillStreamFormats) / sizeof (PKSDATAFORMAT))


//---------------------------------------------------------------------------
// Create an array that holds the list of all of the streams supported
//---------------------------------------------------------------------------

HW_STREAM_INFORMATION Streams [] =
{
    // -----------------------------------------------------------------
    // INTELCAM_VideoStream
    // -----------------------------------------------------------------

    {
    // HW_STREAM_INFORMATION -------------------------------------------
    1,                                      // NumberOfPossibleInstances
    KSPIN_DATAFLOW_OUT,                     // DataFlow
    TRUE,                                   // DataAccessible
    NUM_INTELCAM_VIDEO_STREAM_FORMATS,            // NumberOfFormatArrayEntries
    INTELCAM_VideoStreamFormats,                 // StreamFormatsArray
    NULL,                                      // ClassReserved[0]
    NULL,                                      // ClassReserved[1]
    NULL,                                      // ClassReserved[2]
    NULL,                                      // ClassReserved[3]
    NUMBER_VIDEO_STREAM_PROPERTIES,         // NumStreamPropArrayEntries
    (PKSPROPERTY_SET) VideoStreamProperties,// StreamPropertiesArray
    0,                                      // NumStreamEventArrayEntries;
    0,                                      // StreamEventsArray;
    (GUID *) &PINNAME_VIDEO_CAPTURE,        // Category
    (GUID *) &PINNAME_VIDEO_CAPTURE,        // Name
    0,                                      // MediumsCount
    &StandardMedium,                        // Mediums
    FALSE,									// BridgeStream 
    {0,0}
	},

    // -----------------------------------------------------------------
    // INTELCAM_StillStream
    // -----------------------------------------------------------------

    {
    // HW_STREAM_INFORMATION -------------------------------------------
    1,                                      // NumberOfPossibleInstances
    KSPIN_DATAFLOW_OUT,                     // DataFlow
    TRUE,                                   // DataAccessible
    NUM_INTELCAM_STILL_STREAM_FORMATS,            // NumberOfFormatArrayEntries
    INTELCAM_StillStreamFormats,                 // StreamFormatsArray
    NULL,                                      // ClassReserved[0]
    NULL,                                      // ClassReserved[1]
    NULL,                                      // ClassReserved[2]
    NULL,                                      // ClassReserved[3]
    NUMBER_VIDEO_STREAM_PROPERTIES,         // NumStreamPropArrayEntries
    (PKSPROPERTY_SET) VideoStreamProperties,// StreamPropertiesArray
    0,                                      // NumStreamEventArrayEntries;
    0,                                      // StreamEventsArray;
    (GUID *) &PINNAME_VIDEO_STILL,        // Category
    (GUID *) &PINNAME_VIDEO_STILL,        // Name
    0,                                      // MediumsCount
    &StandardMedium,                        // Mediums
    FALSE,									// BridgeStream 
    {0,0}
	}
};

#define DRIVER_STREAM_COUNT (sizeof (Streams) / sizeof (HW_STREAM_INFORMATION))


/*
** AdapterCompareGUIDsAndFormatSize()
**
**   Checks for a match on the three GUIDs and FormatSize
**
** Arguments:
**
**         IN DataRange1
**         IN DataRange2
**
** Returns:
** 
**   TRUE if all elements match
**   FALSE if any are different
**
** Side Effects:  none
*/

BOOLEAN 
AdapterCompareGUIDsAndFormatSize(
    IN PKSDATARANGE DataRange1,
    IN PKSDATARANGE DataRange2)
{
    return (
        IsEqualGUID (
            &DataRange1->MajorFormat, 
            &DataRange2->MajorFormat) &&
        IsEqualGUID (
            &DataRange1->SubFormat, 
            &DataRange2->SubFormat) &&
        IsEqualGUID (
            &DataRange1->Specifier, 
            &DataRange2->Specifier) &&
        (DataRange1->FormatSize == DataRange2->FormatSize));
}


/*
** AdapterFormatFromRange()
**
**   Returns a DATAFORMAT from a DATARANGE
**
** Arguments:
**
**         IN PHW_STREAM_REQUEST_BLOCK pSrb 
**
** Returns:
** 
**  STATUS_SUCCESS if format is supported
**
** Side Effects:  none
*/


NTSTATUS 
AdapterFormatFromRange(
        IN PHW_STREAM_REQUEST_BLOCK Srb)
{
    PSTREAM_DATA_INTERSECT_INFO intersectInfo;
    PKSDATARANGE  dataRange;
    ULONG formatSize;
    ULONG streamNumber;
    ULONG j;
    ULONG numberOfFormatArrayEntries;
    PKSDATAFORMAT *availableFormats;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOL bMatchFound = FALSE;
    

    intersectInfo = Srb->CommandData.IntersectInfo;
    streamNumber = intersectInfo->StreamNumber;
    dataRange = intersectInfo->DataRange;

    //
    // Check that the stream number is valid
    //
    if (streamNumber >= DRIVER_STREAM_COUNT) {

        ntStatus = Srb->Status = STATUS_INVALID_PARAMETER;
        return ntStatus;
    }


    INTELCAM_KdPrint (MAX_TRACE, ("'AdapterFormatFromRange: Stream=%d\n", 
            streamNumber));
    
    numberOfFormatArrayEntries = 
            Streams[streamNumber].NumberOfFormatArrayEntries;

    //
    // Get the pointer to the array of available formats
    //

    availableFormats =
		Streams[streamNumber].StreamFormatsArray;


    //
    // Walk the formats supported by the stream searching for a match
    // of the three GUIDs which together define a DATARANGE
    //

    for (j = 0; j < numberOfFormatArrayEntries; j++, availableFormats++) {

        INTELCAM_KdPrint (MAX_TRACE, ("'checking format %d\n",j));

        if (!AdapterCompareGUIDsAndFormatSize(dataRange,*availableFormats)) {
            // not the format we want
            continue;
        }

        //
        // Now that the three GUIDs match, switch on the Specifier
        // to do a further type specific check
        //

        // -------------------------------------------------------------------
        // Specifier FORMAT_VideoInfo for VIDEOINFOHEADER
        // -------------------------------------------------------------------

        if ( IsEqualGUID(&dataRange->Specifier, &KSDATAFORMAT_SPECIFIER_VIDEOINFO)) 
        {
            PKS_DATARANGE_VIDEO   dataRangeVideoToVerify = (PKS_DATARANGE_VIDEO) dataRange;
            PKS_DATARANGE_VIDEO   dataRangeVideo = (PKS_DATARANGE_VIDEO) *availableFormats;
            PKS_DATAFORMAT_VIDEOINFOHEADER   DataFormatVideoInfoHeaderOut;
      
            INTELCAM_KdPrint (MAX_TRACE, ("'guid matched\n")); 
        
            //
            // Check that the other fields match
            //
            if ((dataRangeVideoToVerify->bFixedSizeSamples !=
                    dataRangeVideo->bFixedSizeSamples) ||
                (dataRangeVideoToVerify->bTemporalCompression !=
                    dataRangeVideo->bTemporalCompression) ||
                (dataRangeVideoToVerify->StreamDescriptionFlags !=
                    dataRangeVideo->StreamDescriptionFlags) ||
                (dataRangeVideoToVerify->MemoryAllocationFlags !=
                    dataRangeVideo->MemoryAllocationFlags) ||
                (RtlCompareMemory (&dataRangeVideoToVerify->ConfigCaps,
                    &dataRangeVideo->ConfigCaps,
                    sizeof (KS_VIDEO_STREAM_CONFIG_CAPS)) != 
                    sizeof (KS_VIDEO_STREAM_CONFIG_CAPS))) {
                // not the format want                        
                INTELCAM_KdPrint (MAX_TRACE, ("'format does not match\n")); 
                continue;
            }

            if ( (dataRangeVideoToVerify->VideoInfoHeader.bmiHeader.biWidth != 
                  dataRangeVideo->VideoInfoHeader.bmiHeader.biWidth ) ||
                (dataRangeVideoToVerify->VideoInfoHeader.bmiHeader.biHeight != 
                  dataRangeVideo->VideoInfoHeader.bmiHeader.biHeight )) {
                continue;
            }

            //
            // KS_SIZE_VIDEOHEADER() below is relying on bmiHeader.biSize from
            // the caller's data range.  This **MUST** be validated; the
            // extended bmiHeader size (biSize) must not extend past the end
            // of the range buffer. Arithmetic overflow is also checked for.
            // (overflow occurs if (a + b) < a, where a and b are unsigned values)

            {
                DWORD dwVideoHeaderSize = 
                    KS_SIZE_VIDEOHEADER (&dataRangeVideoToVerify->VideoInfoHeader);
                DWORD dwDataRangeSize = FIELD_OFFSET (KS_DATARANGE_VIDEO, VideoInfoHeader) +
                    dwVideoHeaderSize;

                if (dwVideoHeaderSize < dataRangeVideoToVerify->VideoInfoHeader.bmiHeader.biSize ||
                    dwDataRangeSize < dwVideoHeaderSize ||
                    dwDataRangeSize > dataRangeVideoToVerify->DataRange.FormatSize) {

                    ntStatus = Srb->Status = STATUS_INVALID_PARAMETER;
                    return ntStatus;
                }
            }
        
            INTELCAM_KdPrint (MAX_TRACE, ("'match found\n"));                 
            formatSize = sizeof (KSDATAFORMAT) + 
                KS_SIZE_VIDEOHEADER (&dataRangeVideoToVerify->VideoInfoHeader);

            // we found a match.
            bMatchFound = TRUE;
            //    
            // Is the return buffer size = 0 ?
            //

            if(intersectInfo->SizeOfDataFormatBuffer == 0) {

                ntStatus = Srb->Status = STATUS_BUFFER_OVERFLOW;
                // the proxy wants to know the actuall buffer size to allocate.
                Srb->ActualBytesTransferred = formatSize;
                return ntStatus;
            }


            //
            // this time the proxy should have allocated enough space...
            //

            if (intersectInfo->SizeOfDataFormatBuffer < formatSize) {
                ntStatus = Srb->Status = STATUS_BUFFER_TOO_SMALL;
                INTELCAM_KdPrint (MIN_TRACE, ("size of data format buffer is too small\n"));    
                return ntStatus;
            }

            DataFormatVideoInfoHeaderOut = (PKS_DATAFORMAT_VIDEOINFOHEADER) intersectInfo->DataFormatBuffer;


            // Copy over the KSDATAFORMAT, followed by the 
            // actual VideoInfoHeader
            
            RtlCopyMemory(
                &DataFormatVideoInfoHeaderOut->DataFormat,
                &dataRangeVideoToVerify->DataRange,
                sizeof (KSDATARANGE));

            DataFormatVideoInfoHeaderOut->DataFormat.FormatSize = formatSize;
            Srb->ActualBytesTransferred = formatSize;

            RtlCopyMemory(
                &DataFormatVideoInfoHeaderOut->VideoInfoHeader,
                &dataRangeVideoToVerify->VideoInfoHeader,
                KS_SIZE_VIDEOHEADER (&dataRangeVideoToVerify->VideoInfoHeader));


            // Calculate biSizeImage for this request, and put the result in both
            // the biSizeImage field of the bmiHeader AND in the SampleSize field
            // of the DataFormat.
            //
            // Note that for compressed sizes, this calculation will probably not
            // be just width * height * bitdepth

            DataFormatVideoInfoHeaderOut->VideoInfoHeader.bmiHeader.biSizeImage =
                DataFormatVideoInfoHeaderOut->DataFormat.SampleSize = 
                KS_DIBSIZE(DataFormatVideoInfoHeaderOut->VideoInfoHeader.bmiHeader);

            //
            // Perform other validation such as cropping and scaling checks
            // 

            // we will not allow setting FPS below our minimum FPS.
            if ((DataFormatVideoInfoHeaderOut->VideoInfoHeader.AvgTimePerFrame >
                dataRangeVideo->ConfigCaps.MaxFrameInterval) ) {

                DataFormatVideoInfoHeaderOut->VideoInfoHeader.AvgTimePerFrame =
            	    dataRangeVideo->ConfigCaps.MaxFrameInterval;
                DataFormatVideoInfoHeaderOut->VideoInfoHeader.dwBitRate = 
                    dataRangeVideo->ConfigCaps.MinBitsPerSecond;
            }

		    // we will not allow setting FPS above our maximum FPS.
            if ((DataFormatVideoInfoHeaderOut->VideoInfoHeader.AvgTimePerFrame <
                dataRangeVideo->ConfigCaps.MinFrameInterval) ) {

                DataFormatVideoInfoHeaderOut->VideoInfoHeader.AvgTimePerFrame =
            	    dataRangeVideo->ConfigCaps.MinFrameInterval;
                DataFormatVideoInfoHeaderOut->VideoInfoHeader.dwBitRate = 
                    dataRangeVideo->ConfigCaps.MaxBitsPerSecond;
            }

            break;
        } 
    } // End of loop on all formats for this stream

    if (!bMatchFound) {
        Srb->Status = ntStatus = STATUS_NOT_FOUND;
        INTELCAM_KdPrint (MIN_TRACE, ("no Format match\n"));
    }

    INTELCAM_KdPrint (MAX_TRACE, ("'AdapterFormatFromRange ret %x\n", ntStatus));
    
    return ntStatus;
}


/*
** AdapterVerifyFormat()
**
**   Checks the validity of a format request by walking through the
**       array of supported KSDATA_RANGEs for a given stream.
**
** Arguments:
**
**   pKSDataFormat - pointer of a KS_DATAFORMAT_VIDEOINFOHEADER structure.
**   StreamNumber - index of the stream being queried / opened.
**
** Returns:
** 
**   TRUE if the format is supported
**   FALSE if the format cannot be suppored
**
** Side Effects:  none
*/

BOOL 
AdapterVerifyFormat(
    PKS_DATAFORMAT_VIDEOINFOHEADER pKSDataFormatToVerify, 
    ULONG StreamNumber
    )
{
    PKSDATAFORMAT                       *pAvailableFormats;
    int                                 NumberOfFormatArrayEntries;
    int                                 j;
     
    // Make sure a format has been specified
    if (!pKSDataFormatToVerify)
    {
        return FALSE;
    }

    //
    // Make sure the stream index is valid
    //
    if (StreamNumber >= DRIVER_STREAM_COUNT) {
        return FALSE;
    }

    if (!pKSDataFormatToVerify) {
    	INTELCAM_KdPrint (MIN_TRACE, ("WARNING: 'AdapterVerifyFormat: Received NULL pKSDataFormatToVerify param\n")); 
    	return FALSE;
   	}

    //
    // How many formats does this stream support?
    //
    NumberOfFormatArrayEntries = 
                Streams[StreamNumber].NumberOfFormatArrayEntries;

    INTELCAM_KdPrint (MAX_TRACE, ("'AdapterVerifyFormat: Stream=%d\n", 
            StreamNumber));

    INTELCAM_KdPrint (MAX_TRACE, ("'AdapterVerifyFormat: FormatSize=%d\n", 
            pKSDataFormatToVerify->DataFormat.FormatSize));

    INTELCAM_KdPrint (MAX_TRACE, ("'AdapterVerifyFormat: MajorFormat=%x\n", 
            pKSDataFormatToVerify->DataFormat.MajorFormat));

    //
    // Get the pointer to the array of available formats
    //
    pAvailableFormats = Streams[StreamNumber].StreamFormatsArray;

    //
    // Walk the array, searching for a match
    //
    for (j = 0; j < NumberOfFormatArrayEntries; j++, pAvailableFormats++)
	{
        PKS_DATARANGE_VIDEO         pKSDataRange = (PKS_DATARANGE_VIDEO) *pAvailableFormats;
        PKS_VIDEOINFOHEADER         pVideoInfoHdr = &pKSDataRange->VideoInfoHeader;
        KS_VIDEO_STREAM_CONFIG_CAPS *pConfigCaps = &pKSDataRange->ConfigCaps;
        
        //
        // Check for matching size, Major Type, Sub Type, and Specifier
        //

        if (!IsEqualGUID (&pKSDataRange->DataRange.MajorFormat, 
            &pKSDataFormatToVerify->DataFormat.MajorFormat))
		{
            continue;
        }

        if (!IsEqualGUID (&pKSDataRange->DataRange.SubFormat, 
            &pKSDataFormatToVerify->DataFormat.SubFormat))
		{
            continue;
        }

        if (!IsEqualGUID (&pKSDataRange->DataRange.Specifier,
            &pKSDataFormatToVerify->DataFormat.Specifier))
		{
            continue;
        }

        // -------------------------------------------------------------------
        // Specifier FORMAT_VideoInfo for VIDEOINFOHEADER
        // -------------------------------------------------------------------

        if (IsEqualGUID(&pKSDataRange->DataRange.Specifier, &KSDATAFORMAT_SPECIFIER_VIDEOINFO))
        {
            PKS_VIDEOINFOHEADER pVideoInfoHdrToVerify;

            if (pKSDataFormatToVerify->DataFormat.FormatSize < sizeof(KS_DATAFORMAT_VIDEOINFOHEADER))
            {
                break; // considered a fatal error for this format
            }

            pVideoInfoHdrToVerify = &pKSDataFormatToVerify->VideoInfoHeader;

            INTELCAM_KdPrint (MAX_TRACE, ("'AdapterVerifyFormat: pVideoInfoHdrToVerify=%x\n", 
                        pVideoInfoHdrToVerify));

            INTELCAM_KdPrint (MIN_TRACE, ("'AdapterVerifyFormat: Width=%d Height=%d  biBitCount=%d\n", 
                        pVideoInfoHdrToVerify->bmiHeader.biWidth, 
                        pVideoInfoHdrToVerify->bmiHeader.biHeight,
                        pVideoInfoHdrToVerify->bmiHeader.biBitCount));

            INTELCAM_KdPrint (MIN_TRACE, ("'AdapterVerifyFormat: biSizeImage =%d\n", 
                        pVideoInfoHdrToVerify->bmiHeader.biSizeImage));

            //
            // KS_SIZE_VIDEOHEADER() below is relying on bmiHeader.biSize from
            // the caller's data range.  This **MUST** be validated; the
            // extended bmiHeader size (biSize) must not extend past the end
            // of the range buffer. Arithmetic overflow is also checked for.
            // (overflow occurs if (a + b) < a, where a and b are unsigned values)

            {
                DWORD dwVideoHeaderSize = KS_SIZE_VIDEOHEADER(pVideoInfoHdrToVerify);
                DWORD dwFormatSize = FIELD_OFFSET(KS_DATAFORMAT_VIDEOINFOHEADER, VideoInfoHeader) +
                    dwVideoHeaderSize;

                if (dwVideoHeaderSize < pVideoInfoHdrToVerify->bmiHeader.biSize ||
                    dwFormatSize < dwVideoHeaderSize ||
                    dwFormatSize > pKSDataFormatToVerify->DataFormat.FormatSize) {

                    break; // considered a fatal error for this format
                }
            }

            if ( (pVideoInfoHdrToVerify->bmiHeader.biWidth != pVideoInfoHdr->bmiHeader.biWidth ) ||
                (pVideoInfoHdrToVerify->bmiHeader.biHeight != pVideoInfoHdr->bmiHeader.biHeight )) {
                continue;
            }

            if ( pVideoInfoHdrToVerify->bmiHeader.biSizeImage != pVideoInfoHdr->bmiHeader.biSizeImage ||
                pVideoInfoHdrToVerify->bmiHeader.biSizeImage > pKSDataFormatToVerify->DataFormat.SampleSize) {
                    INTELCAM_KdPrint (MIN_TRACE, ("***Error**:Format mismatch Width=%d Height=%d  image size=%d\n", 
                    pVideoInfoHdrToVerify->bmiHeader.biWidth, 
                    pVideoInfoHdrToVerify->bmiHeader.biHeight,
                    pVideoInfoHdrToVerify->bmiHeader.biSizeImage));

                continue;
            }

            //
            // HOORAY, the format passed all of the tests, so we support it
            //

            return TRUE;
        }
    } 

    //
    // The format requested didn't match any of our listed ranges (or the format was invalid),
    // so refuse the connection.
    //

    return FALSE;
}


//
// hooks for stream SRBs
//

VOID STREAMAPI
INTELCAM_ReceiveDataPacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb,
    IN PVOID DeviceContext,
    IN PBOOLEAN Completed
    )
{
     INTELCAM_KdPrint (MAX_TRACE, ("'INTELCAM_ReceiveDataPacket\n"));
} 


VOID STREAMAPI
INTELCAM_ReceiveCtrlPacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb,
    IN PVOID DeviceContext,
    IN PBOOLEAN Completed
    )
{

	PINTELCAM_DEVICE_CONTEXT deviceContext = DeviceContext;

    INTELCAM_KdPrint (MAX_TRACE, ("'INTELCAM: Receive Ctrl SRB  %x\n", Srb->Command));
	
	*Completed = TRUE; 
    Srb->Status = STATUS_SUCCESS;

    switch (Srb->Command)
    {

	case SRB_PROPOSE_DATA_FORMAT:
		INTELCAM_KdPrint(MIN_TRACE, ("'Receiving SRB_PROPOSE_DATA_FORMAT  SRB  \n"));
		if (!(AdapterVerifyFormat (
				(PKS_DATAFORMAT_VIDEOINFOHEADER)Srb->CommandData.OpenFormat, 
				Srb->StreamObject->StreamNumber))) {
			Srb->Status = STATUS_NO_MATCH;
			INTELCAM_KdPrint(MIN_TRACE,("SRB_PROPOSE_DATA_FORMAT FAILED\n"));
		}
		break;

	case SRB_SET_DATA_FORMAT:
    {
        PKS_DATAFORMAT_VIDEOINFOHEADER  pKSDataFormat = 
                (PKS_DATAFORMAT_VIDEOINFOHEADER) Srb->CommandData.OpenFormat;
        PKS_VIDEOINFOHEADER  pVideoInfoHdrRequested = 
                &pKSDataFormat->VideoInfoHeader;

		INTELCAM_KdPrint(MIN_TRACE, ("'SRB_SET_DATA_FORMAT\n"));

		if ((AdapterVerifyFormat (pKSDataFormat,Srb->StreamObject->StreamNumber))) {
            if (deviceContext->UsbcamdInterface.Interface.Version != 0) {
                if(deviceContext->UsbcamdInterface.USBCAMD_SetVideoFormat(DeviceContext,Srb)) {
                    deviceContext->CurrentProperty.Format.lWidth = 
                        pVideoInfoHdrRequested->bmiHeader.biWidth;
                    deviceContext->CurrentProperty.Format.lHeight =
                        pVideoInfoHdrRequested->bmiHeader.biHeight;
                }
            }
		} 
        else {
			Srb->Status = STATUS_NO_MATCH;
			INTELCAM_KdPrint(MIN_TRACE,(" SRB_SET_DATA_FORMAT FAILED\n"));
        }
		
        break;
    }
	case SRB_GET_DATA_FORMAT:
		INTELCAM_KdPrint(MIN_TRACE, ("' SRB_GET_DATA_FORMAT\n"));
		Srb->Status = STATUS_NOT_IMPLEMENTED;
		break;


	case SRB_SET_STREAM_STATE:

	case SRB_GET_STREAM_STATE:

	case SRB_GET_STREAM_PROPERTY:

	case SRB_SET_STREAM_PROPERTY:

	case SRB_INDICATE_MASTER_CLOCK:

	default:

 		*Completed = FALSE; // let USBCAMD handle these control SRBs
    }

    if (*Completed == TRUE) {
        StreamClassStreamNotification(StreamRequestComplete,Srb->StreamObject,Srb);
    }

} 

#ifdef USBCAMD2

// 
// Describe the camera
//

USBCAMD_DEVICE_DATA2 INTELCAM_DeviceData2 = {
    0,
    INTELCAM_Initialize,
    INTELCAM_UnInitialize,
    INTELCAM_ProcessUSBPacketEx,
    INTELCAM_NewFrameEx,
    INTELCAM_ProcessRawVideoFrameEx,
    INTELCAM_StartVideoCaptureEx,
    INTELCAM_StopVideoCaptureEx,
    INTELCAM_ConfigureEx,
    INTELCAM_SaveState,
    INTELCAM_RestoreState,
    INTELCAM_AllocateBandwidthEx,
    INTELCAM_FreeBandwidthEx
};

#endif

// 
// Describe the camera
//

USBCAMD_DEVICE_DATA INTELCAM_DeviceData = {
    0,
    INTELCAM_Initialize,
    INTELCAM_UnInitialize,
    INTELCAM_ProcessUSBPacket,
    INTELCAM_NewFrame,
    INTELCAM_ProcessRawVideoFrame,
    INTELCAM_StartVideoCapture,
    INTELCAM_StopVideoCapture,
    INTELCAM_Configure,
    INTELCAM_SaveState,
    INTELCAM_RestoreState,
    INTELCAM_AllocateBandwidth,
    INTELCAM_FreeBandwidth
};


#ifdef USBCAMD2
/*
** INTELCAM_HwInitialize()
**
**
**   This routine is called when an SRB_INITIALIZE_DEVICE request is received
**
** Arguments:
**
**   pSrb - pointer to stream request block for the Initialize command
**
** Returns:
**
** Side Effects:  none
*/


BOOLEAN 
INTELCAM_HwInitialize(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    )
{
    PPORT_CONFIGURATION_INFORMATION configInfo = Srb->CommandData.ConfigInfo;
    PINTELCAM_DEVICE_CONTEXT DeviceContext;
    ULONG version = 0;

    // get our device ext. from USBCAMD.
    DeviceContext = USBCAMD_AdapterReceivePacket(Srb,NULL,NULL,FALSE);
    // save our device object: shared with Stream class and USBCAMD.
    DeviceContext->pDeviceObject = configInfo->ClassDeviceObject;
    // save our PnP device object for registry access.
    DeviceContext->pPnPDeviceObject = configInfo->RealPhysicalDeviceObject;
    // just in case we are dealing with old version of stream class.
    if (!DeviceContext->pPnPDeviceObject) {
        DeviceContext->pPnPDeviceObject = configInfo->PhysicalDeviceObject;
    }

    ASSERT ( DeviceContext->pPnPDeviceObject != NULL) ;

    // call USBCAMD to initialize the new interface.
    version = USBCAMD_InitializeNewInterface(DeviceContext,&INTELCAM_DeviceData2,
                                   USBCAMD_VERSION_200,
                                   USBCAMD_CamControlFlag_AssociatedFormat |
                                   USBCAMD_CamControlFlag_EnableDeviceEvents);

    return TRUE;
}

/*
** INTELCAM_CompleteInitialization()
**
**
**   This routine is called when an SRB_COMPLETE_INITIALIZATION request is received
**
** Arguments:
**
**   pSrb - pointer to stream request block for the Initialize command
**
** Returns:
**
** Side Effects:  none
*/

NTSTATUS 
INTELCAM_CompleteInitialization(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBCAMD_INTERFACE pUsbcamdInterface;
    PINTELCAM_DEVICE_CONTEXT DeviceContext;
    
    // get our device ext. from USBCAMD.
    DeviceContext = USBCAMD_AdapterReceivePacket(Srb,NULL,NULL,FALSE);

#if DBG
    pUsbcamdInterface = ExAllocatePoolWithTag ( NonPagedPool,sizeof(USBCAMD_INTERFACE), 'MACM' );
#else
    pUsbcamdInterface = ExAllocatePool(NonPagedPool,sizeof(USBCAMD_INTERFACE));
#endif

    if (pUsbcamdInterface == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(pUsbcamdInterface,sizeof(USBCAMD_INTERFACE));

    //
    // Attempt to acquire the new usbcamd interface.
    //

    INTELCAM_QueryUSBCAMDInterface(DeviceContext->pDeviceObject,pUsbcamdInterface);
    if ( pUsbcamdInterface->USBCAMD_SetIsoPipeState != NULL )   { 
        // new interface found. pass the new expanded function table to USBCAMD.
        DeviceContext->UsbcamdInterface = *pUsbcamdInterface;
    }  
    else {
        INTELCAM_KdPrint(MIN_TRACE, ("***ERROR***: QI Failed\n"));
        // new intrface not found. 
        DeviceContext->UsbcamdInterface.Interface.Version = 0;
    }
    
    ExFreePool(pUsbcamdInterface);
    return (ntStatus);
}

#endif


VOID 
INTELCAM_AdapterReceivePacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    )
{
    PINTELCAM_DEVICE_CONTEXT deviceContext;
    PHW_STREAM_INFORMATION streamInformation =
        &(Srb->CommandData.StreamBuffer->StreamInfo);
    PHW_STREAM_HEADER streamHeader =
        &(Srb->CommandData.StreamBuffer->StreamHeader);        
    PDEVICE_OBJECT deviceObject;   
    ULONG i;
    
    
    INTELCAM_KdPrint (MAX_TRACE, ("'AdapterReceivePacket\n"));
     
    switch (Srb->Command)
    {
#ifdef USBCAMD2
    case SRB_INITIALIZE_DEVICE:

        INTELCAM_KdPrint(MIN_TRACE, ("'SRB_INITIALIZE_DEVICE\n"));

        if (!INTELCAM_HwInitialize(Srb)) {
            INTELCAM_KdPrint(MIN_TRACE, ("'Bailing out - wrong usbcamd version\n"));
            StreamClassDeviceNotification(DeviceRequestComplete,
                              Srb->HwDeviceExtension,Srb);
        }
        else {
	        USBCAMD_AdapterReceivePacket(Srb,&INTELCAM_DeviceData,NULL,TRUE);
        }

        break;
#endif
    case SRB_GET_STREAM_INFO:
        //
        // this is a request for the driver to enumerate requested streams
        //
        INTELCAM_KdPrint (MAX_TRACE, ("'SRB_GET_STREAM_INFO\n"));
       
        // get our device ext. from USBCAMD.
        deviceContext = USBCAMD_AdapterReceivePacket(Srb,NULL,NULL,FALSE);

        //
        // we can support up to two streams
        //
        
        ASSERT (Srb->NumberOfBytesToTransfer >=
              sizeof (HW_STREAM_HEADER) +
			             sizeof (HW_STREAM_INFORMATION) * deviceContext->PinCount);

		streamHeader->NumberOfStreams = deviceContext->PinCount;
        
        for ( i=0; i < deviceContext->PinCount; i++ ) {
            *streamInformation++ = Streams[i];
        }

        //
        // set the property information for the video stream
        //

        streamHeader->DevicePropertiesArray =
            INTELCAM_GetAdapterPropertyTable(
			    &streamHeader->NumDevPropArrayEntries); 

        // pass to usbcamd to finish the job
        deviceContext =     
            USBCAMD_AdapterReceivePacket(Srb,
			                             &INTELCAM_DeviceData,
										 NULL,
										 TRUE);
            
        ASSERT_DEVICE_CONTEXT(deviceContext); 
        break;

    case SRB_GET_DEVICE_PROPERTY:
        //
        // we handle all the property stuff 
        //
        INTELCAM_KdPrint (MAX_TRACE, ("'SRB_GET_DEVICE_PROPERTY\n"));
        
        deviceContext =     
            USBCAMD_AdapterReceivePacket(Srb,
			                             &INTELCAM_DeviceData,
			                             &deviceObject,
										 FALSE);   
        ASSERT_DEVICE_CONTEXT(deviceContext);  
        
        INTELCAM_KdPrint (MAX_TRACE, ("'SRB_GET_STREAM_INFO\n"));
        INTELCAM_PropertyRequest(FALSE,
                                 deviceObject,
                                 deviceContext,
                                 Srb);
        StreamClassDeviceNotification(DeviceRequestComplete,
                                      Srb->HwDeviceExtension,
                                      Srb);
        break;            
            
    case SRB_SET_DEVICE_PROPERTY:
        //
        // we handle all the property stuff 
        //
        INTELCAM_KdPrint (MAX_TRACE, ("'SRB_SET_DEVICE_PROPERTY\n"));
        
        deviceContext =     
            USBCAMD_AdapterReceivePacket(Srb,
			                             &INTELCAM_DeviceData,
										 &deviceObject,
										 FALSE);   
        ASSERT_DEVICE_CONTEXT(deviceContext);  
        
        INTELCAM_PropertyRequest(TRUE,
                                 deviceObject,
                                 deviceContext,
                                 Srb);

        StreamClassDeviceNotification(DeviceRequestComplete,
                                      Srb->HwDeviceExtension,
                                      Srb);
        break;

    case SRB_OPEN_STREAM:        
        {
        PKS_DATAFORMAT_VIDEOINFOHEADER  pKSDataFormat = 
                (PKS_DATAFORMAT_VIDEOINFOHEADER) Srb->CommandData.OpenFormat;
        PKS_VIDEOINFOHEADER  pVideoInfoHdrRequested = 
                &pKSDataFormat->VideoInfoHeader;

        // pass to usbcamd to finish the job
        Srb->StreamObject->ReceiveDataPacket =
		                       (PVOID) INTELCAM_ReceiveDataPacket;
        Srb->StreamObject->ReceiveControlPacket =
		                       (PVOID) INTELCAM_ReceiveCtrlPacket;

  
        if (AdapterVerifyFormat(pKSDataFormat, 
                                Srb->StreamObject->StreamNumber))
		{
			deviceContext =     
				USBCAMD_AdapterReceivePacket(Srb,
			                             &INTELCAM_DeviceData,
										 NULL,
										 TRUE);

			deviceContext->StreamOpen = TRUE;
            // save the dimension of the open stream format.
            deviceContext->CurrentProperty.Format.lWidth = 
                pVideoInfoHdrRequested->bmiHeader.biWidth;
            deviceContext->CurrentProperty.Format.lHeight =
                pVideoInfoHdrRequested->bmiHeader.biHeight;

		}
		else
		{
            Srb->Status = STATUS_INVALID_PARAMETER;
            StreamClassDeviceNotification(DeviceRequestComplete,
                                      Srb->HwDeviceExtension,
                                      Srb);
        }
        }
        break;                    
        
	case SRB_GET_DATA_INTERSECTION:
		//
		// Return a format, given a range
		//
        //deviceContext =     
        //    USBCAMD_AdapterReceivePacket(Srb,
		//	                             &INTELCAM_DeviceData,
		//	                             &deviceObject,
		//								 FALSE);   

		Srb->Status = AdapterFormatFromRange(Srb);
		
        StreamClassDeviceNotification(DeviceRequestComplete,
                                      Srb->HwDeviceExtension,
                                      Srb);
		break;

#ifdef USBCAMD2
    case SRB_INITIALIZATION_COMPLETE:

        //
        // Stream class has finished initialization.
        // Get USBCAMD inetrface.
        //
        Srb->Status = INTELCAM_CompleteInitialization(Srb);
        StreamClassDeviceNotification(DeviceRequestComplete,
                                      Srb->HwDeviceExtension,
                                      Srb);
        break;
#endif

	default:
        //
        // let usbcamd handle it
        //
    
        deviceContext =     
            USBCAMD_AdapterReceivePacket(Srb,
			                             &INTELCAM_DeviceData,
										 NULL,
										 TRUE);   
        ASSERT_DEVICE_CONTEXT(deviceContext);  
		break;
	}
}


/*
** DriverEntry()
**
** This routine is called when the mini driver is first loaded.  The driver
** should then call the StreamClassRegisterAdapter function to register with
** the stream class driver
**
** Arguments:
**
**  Context1:  The context arguments are private plug and play structures
**             used by the stream class driver to find the resources for this
**             adapter
**  Context2:
**
** Returns:
**
** This routine returns an NT_STATUS value indicating the result of the
** registration attempt. If a value other than STATUS_SUCCESS is returned, the
** minidriver will be unloaded.
**
** Side Effects:  none
*/

ULONG
DriverEntry(
    PVOID Context1,
    PVOID Context2
    )
{

    INTELCAM_KdPrint (MAX_TRACE, ("'Driver Entry\n")); 


    return USBCAMD_DriverEntry(Context1, 
                               Context2, 
                               sizeof(INTELCAM_DEVICE_CONTEXT),
                               sizeof(INTELCAM_FRAME_CONTEXT),
                               INTELCAM_AdapterReceivePacket);
}



/*
** INTELCAM_Initialize()
**
** On entry the device has been configured and the initial alt
** interface selected -- this is where we may send additional 
** vendor commands to enable the device.
**
** Arguments:
**
** BusDeviceObject - pdo associated with this device
**
** DeviceContext - driver specific context
**
** Returns:
**
** NTSTATUS code
**
** Side Effects:  none
*/

NTSTATUS
INTELCAM_Initialize(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    )
{
    PINTELCAM_DEVICE_CONTEXT deviceContext;
    LARGE_INTEGER dueTime;
    ULONG wait = 0;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    USHORT powerState = 0;
    ULONG length;

    deviceContext = DeviceContext;

    ASSERT_DEVICE_CONTEXT(deviceContext);
    //
    // perform any hardware specific 
    // initialization
    //

    INTELCAM_KdPrint (MAX_TRACE, ("'power up the camera\n"));

    //
    // NOTE
    // This only works on the production level cameras.
    //

    do {

        dueTime.QuadPart = -10000 * 1000;

        (VOID) KeDelayExecutionThread(KernelMode,
                                      FALSE,
                                      &dueTime);
        length = sizeof(powerState);
        
        ntStatus = USBCAMD_ControlVendorCommand(DeviceContext,
                                         REQUEST_GET_PREFERENCE,
                                         0,
                                         INDEX_PREF_POWER,
                                         &powerState,
                                         &length,
                                         TRUE,
										 NULL,
										 NULL);

        ILOGENTRY("PWck", wait, powerState, ntStatus);

        INTELCAM_KdPrint (MIN_TRACE, ("'get power state power = 0x%x, ntStatus %x\n",
            powerState, ntStatus));

        // timeout ater ~10 seconds
        wait++;
        if (wait > 9 ) {
            ILOGENTRY("HWto", wait, 0, ntStatus);
            ntStatus = STATUS_UNSUCCESSFUL;
        }

    } while (NT_SUCCESS(ntStatus) && powerState < 3);

    //
    // initialize the camera to default settings
    //

    if (NT_SUCCESS(ntStatus)) {
        ntStatus = USBCAMD_ControlVendorCommand(
                                DeviceContext,
                                REQUEST_SET_PREFERENCE,
                                0,
                                INDEX_PREF_INITIALIZE,
                                NULL,
                                NULL,
                                FALSE,
								NULL,
								NULL);
    }

    
    deviceContext->StreamOpen = FALSE;
	
    INTELCAM_KdPrint (MIN_TRACE, ("InitializeHardware 0x%x\n", ntStatus));
	//
    // chk for usb stack failure. If so, we need to free our allocated resources
    // since the driver will unload upon returning from this function w/o having a 
    // chance to clean up thru the regular SRB_UNITIALIZE_DEVICE handler.
    //
    if ( ntStatus != STATUS_SUCCESS ) {

        if (deviceContext->Interface) {

    	    ExFreePool(deviceContext->Interface);
    	    deviceContext->Interface = NULL;
        }
    }

    ILOGENTRY("inHW", 0, 0, ntStatus);
    
    return ntStatus;
}


/*
** INTELCAM_UnInitialize()
**
** Assume the device hardware is gone -- all that needs to be done is to 
** free any allocated resources (like memory).
**
** Arguments:
**
** BusDeviceObject - pdo associated with this device
**
** DeviceContext - driver specific context
**
** Returns:
**
** NTSTATUS code
**
** Side Effects:  none
*/

NTSTATUS
INTELCAM_UnInitialize(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    )
{
    PINTELCAM_DEVICE_CONTEXT deviceContext;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    deviceContext = DeviceContext;
    ASSERT_DEVICE_CONTEXT(deviceContext);

	if ( deviceContext->Interface) { 
    	ExFreePool(deviceContext->Interface);
    	deviceContext->Interface = NULL;
    }

    INTELCAM_KdPrint (MIN_TRACE, ("UnInitialize \n"));
    
    return ntStatus;
}

#ifdef USBCAMD2
/*
** INTELCAM_ConfigureEx()
**
** Configure the iso streaming Interface:
**
** Called just before the device is configured, this is where we tell
** usbcamd which interface and alternate setting to use for the idle state.
**
** NOTE: The assumption here is that the device will have a single interface 
**  with multiple alt settings and each alt setting has the same number of 
**  pipes.
**
** Arguments:
**
**  BusDeviceObject - device object created by the hub whe can submit
**                  urbs to our device through this deviceObject
**
**  DeviceContext - minidriver device  context 
**
**  Interface - USBD interface structure initialized with the proper values
**              for select_configuration. This Interface structure corresponds 
**              a single iso interafce on the device.  This is the drivers 
**              chance to pick a particular alternate setting and pipe 
**              parameters.
**
**
**  ConfigurationDescriptor - USB configuration Descriptor for
**      this device.
**
**  PipeConfigListSize  - size of the array of USBCAMD_Pipe_Config_Descriptor.
**
**  pConfigDesc - pointer to an array of USBCAMD_Pipe_Config_Descriptors.
**
**  DeviceDescriptor - USB device descriptor  
** Returns:
**
**  NTSTATUS code
**
** Side Effects:  none
*/

NTSTATUS
INTELCAM_ConfigureEx(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PUSBD_INTERFACE_INFORMATION Interface,
    PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    ULONG   PipeConfigListSize,
    PUSBCAMD_Pipe_Config_Descriptor pConfigDesc,
    PUSB_DEVICE_DESCRIPTOR pDeviceDesc
    )
{

    ULONG i;
    LONG DataPipe, SyncPipe;
    PINTELCAM_DEVICE_CONTEXT deviceContext;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    deviceContext = DeviceContext;
    
    ntStatus = INTELCAM_Configure(
                            BusDeviceObject,      
                            DeviceContext,
                            Interface,
                            ConfigurationDescriptor,
                            &DataPipe,
                            &SyncPipe);

	if (Interface == NULL )
		return ntStatus;
		
    if (pDeviceDesc->bcdDevice == 0x101 ) {
        // camera is the square shapped one. no snapshot button
        deviceContext->PinCount = 1;
        INTELCAM_KdPrint (MIN_TRACE, ("Model YC72 detected \n"));
    }
    else {
        // camera is the new oval shapped one.snapshot button is provided.
        deviceContext->PinCount = 2;
        INTELCAM_KdPrint (MIN_TRACE, ("Model YC76 detected \n"));
    }

    for ( i=0 ; i < PipeConfigListSize ; i++) {
        pConfigDesc[i] = ( deviceContext->PinCount == 1) ? INTELCAM_PipeConfigArray1[i]:
                                                           INTELCAM_PipeConfigArray2[i];
    }
    return ntStatus;
}

#endif

/*
** INTELCAM_Configure()
**
** Configure the iso streaming Interface:
**
** Called just before the device is configured, this is where we tell
** usbcamd which interface and alternate setting to use for the idle state.
**
** NOTE: The assumption here is that the device will have a single interface 
**  with multiple alt settings and each alt setting has the same number of 
**  pipes.
**
** Arguments:
**
**  BusDeviceObject - device object created by the hub whe can submit
**                  urbs to our device through this deviceObject
**
**  DeviceContext - minidriver device  context 
**
**  Interface - USBD interface structure initialized with the proper values
**              for select_configuration. This Interface structure corresponds 
**              a single iso interafce on the device.  This is the drivers 
**              chance to pick a particular alternate setting and pipe 
**              parameters.
**
**
**  ConfigurationDescriptor - USB configuration Descriptor for
**      this device.
**
** Returns:
**
**  NTSTATUS code
**
** Side Effects:  none
*/

NTSTATUS
INTELCAM_Configure(
    IN PDEVICE_OBJECT BusDeviceObject,
    IN PVOID DeviceContext,
    IN OUT PUSBD_INTERFACE_INFORMATION Interface,
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN OUT PLONG DataPipeIndex,
    IN OUT PLONG SyncPipeIndex
    )
{
    PINTELCAM_DEVICE_CONTEXT deviceContext;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    deviceContext = DeviceContext;

    deviceContext->Sig = INTELCAM_DEVICE_SIG;
    deviceContext->DefaultFrameRate = INTELCAM_DEFAULT_FRAME_RATE;

    //
    // initilialize any other context stuff
    //
    if ( Interface == NULL) {
    	//
    	// this is a signal from usbcamd that I need to free my previousely 
    	// allocated space for interface descriptor due to error conditions
    	// during IRP_MN_START_DEVICE processing and driver will be unloaded soon.
    	//
    	if (deviceContext->Interface) {
    		ExFreePool(deviceContext->Interface);
    		deviceContext->Interface = NULL;
    	}
    	return ntStatus;
    }
    
#if DBG
    deviceContext->Interface = ExAllocatePoolWithTag ( NonPagedPool,
                                              Interface->Length, 'MACM' );
#else
    deviceContext->Interface = ExAllocatePool(NonPagedPool,
                                              Interface->Length);
#endif

    *DataPipeIndex = 1;
    *SyncPipeIndex = 0;
    
    if (deviceContext->Interface) {
    
        Interface->AlternateSetting = INTELCAM_IDLE_ALT_SETTING;

        // This interface has two pipes,
        // initialize input parameters to USBD for both pipes.
        // The MaximumTransferSize is the size of the largest
        // buffer we want to submit for an aingle iso urb
        // request.
        //
        Interface->Pipes[INTELCAM_SYNC_PIPE].MaximumTransferSize =
            USBD_DEFAULT_MAXIMUM_TRANSFER_SIZE;
        Interface->Pipes[INTELCAM_DATA_PIPE].MaximumTransferSize =
            1024*32;       // 32k transfer per urb

        RtlCopyMemory(deviceContext->Interface,
                      Interface,
                      Interface->Length);                 

        INTELCAM_KdPrint (MAX_TRACE, ("''size of interface request = %d\n", Interface->Length));

                
    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    

    //
    // return interface number and alternate setting
    //
   
    INTELCAM_KdPrint (MAX_TRACE, ("''Configure 0x%x\n", ntStatus));

    return ntStatus;
}

#ifdef USBCAMD2
/*
** INTELCAM_StartVideoCaptureEx()
**
** Arguments:
**
**  BusDeviceObject - device object created by the hub we can submit
**                  urbs to our device through this deviceObject
**
**  DeviceContext - minidriver device  context
**
**  StreamNumber - stream number.
**
** Returns:
**
** NTSTATUS code
**
** Side Effects:  none
*/

NTSTATUS
INTELCAM_StartVideoCaptureEx(
    IN PDEVICE_OBJECT BusDeviceObject,
    IN PVOID DeviceContext,
    IN ULONG StreamNumber
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (StreamNumber == STREAM_Capture) {
        ntStatus = INTELCAM_StartVideoCapture(BusDeviceObject,DeviceContext);
    }
    return ntStatus;
}

#endif

/*
** INTELCAM_StartVideoCapture()
**
** Arguments:
**
**  BusDeviceObject - device object created by the hub we can submit
**                  urbs to our device through this deviceObject
**
**  DeviceContext - minidriver device  context
**
** Returns:
**
** NTSTATUS code
**
** Side Effects:  none
*/

NTSTATUS
INTELCAM_StartVideoCapture(
    IN PDEVICE_OBJECT BusDeviceObject,
    IN PVOID DeviceContext
    )
{
    PINTELCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
    NTSTATUS ntStatus;
    
    ASSERT_DEVICE_CONTEXT(deviceContext);
    
    //
    // This is where we select the interface we need and send
    // commands to start capturing
    //
    INTELCAM_KdPrint (MAX_TRACE, ("'starting capture \n"));
   
    // we need to restore the hw setting for scaling in case we are here after restoring power.

    ntStatus = USBCAMD_ControlVendorCommand(DeviceContext,
                                            REQUEST_SET_PREFERENCE,
                                            deviceContext->hwSetting,
                                            INDEX_PREF_SCALING,
                                            NULL,
                                            NULL,
                                            FALSE,
											NULL,
											NULL);

    if ( ntStatus == STATUS_SUCCESS) {
        ntStatus = USBCAMD_ControlVendorCommand(DeviceContext,
                                            REQUEST_SET_PREFERENCE,
                                            1,
                                            INDEX_PREF_CAPMOTION,
                                            NULL,
                                            NULL,
                                            FALSE,
											NULL,
											NULL);
    }
    else {
        INTELCAM_KdPrint (MIN_TRACE, ("Restoring HW scaling failed 0x%x\n", ntStatus));
    }
    return ntStatus;        
}

#ifdef USBCAMD2
/*
** INTELCAM_AllocateBandwidthEx()
**
** Called just before the iso video capture stream is
** started, here is where we select the appropriate 
** alternate interface and set up the device to stream.
**
**  Called in connection with the stream class RUN command
**
** Arguments:
**
**  BusDeviceObject - device object created by the hub we can submit
**                  urbs to our device through this deviceObject
**
**  DeviceContext - minidriver device  context
**
**  RawFrameLength - pointer to be filled in with size of buffer needed to 
**                  receive the raw frame data from the packet stream.
**
**  Format - pointer to PKS_DATAFORMAT_VIDEOINFOHEADER associated with this 
**          stream.          
**
**  StreamNumber - stream number.
**
** Returns:
**
** NTSTATUS code
**
** Side Effects:  none
*/

NTSTATUS
INTELCAM_AllocateBandwidthEx(
    IN PDEVICE_OBJECT BusDeviceObject,
    IN PVOID DeviceContext,
    OUT PULONG RawFrameLength,
    IN PVOID Format,
    IN ULONG StreamNumber
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PINTELCAM_DEVICE_CONTEXT deviceContext = DeviceContext;

    if (StreamNumber == STREAM_Capture) {
        ntStatus = INTELCAM_AllocateBandwidth(BusDeviceObject,DeviceContext,
                        RawFrameLength,Format);
    }
    else {
        // we allocate the max. possible frame that can be produced by this camera.
        *RawFrameLength = (320 * 240 * 12 ) / 8 ;
    }
    return ntStatus;
}
#endif

/*
** INTELCAM_AllocateBandwidth()
**
** Called just before the iso video capture stream is
** started, here is where we select the appropriate 
** alternate interface and set up the device to stream.
**
**  Called in connection with the stream class RUN command
**
** Arguments:
**
**  BusDeviceObject - device object created by the hub we can submit
**                  urbs to our device through this deviceObject
**
**  DeviceContext - minidriver device  context
**
**  RawFrameLength - pointer to be filled in with size of buffer needed to 
**                  receive the raw frame data from the packet stream.
**
**  Format - pointer to PKS_DATAFORMAT_VIDEOINFOHEADER associated with this 
**          stream.          
**
** Returns:
**
** NTSTATUS code
**
** Side Effects:  none
*/

NTSTATUS
INTELCAM_AllocateBandwidth(
    IN PDEVICE_OBJECT BusDeviceObject,
    IN PVOID DeviceContext,
    OUT PULONG RawFrameLength,
    IN PVOID Format            
    )
{
    PINTELCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
    ULONG PixelCount, bytesInImage, maxDataPacketSize;
    ULONG AvgTimePerFrame, FrameRate,MicroSecPerFrame,RequestedPktSize;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PKS_DATAFORMAT_VIDEOINFOHEADER dataFormatHeader;
    PKS_BITMAPINFOHEADER bmInfoHeader;
    LONGLONG val1,val2;
    LONG  Index;
    ULONG Numerator, Denominator;
    
    
    ASSERT_DEVICE_CONTEXT(deviceContext);
    
    //
    // This is where we select the interface we need and send
    // commands to start capturing
    //
    INTELCAM_KdPrint (MAX_TRACE, ("starting capture %x\n", Format));

    *RawFrameLength = 0;
    dataFormatHeader = Format;
    bmInfoHeader = &dataFormatHeader->VideoInfoHeader.bmiHeader;

    if (bmInfoHeader->biWidth == 176){
    	deviceContext->hwSetting = 0x4;
        Numerator = 1;
        Denominator =1;
    }
    else if (bmInfoHeader->biWidth == 160){
    	deviceContext->hwSetting = 0xa;
        Numerator = 3;
        Denominator =4;
    }
	else{
    	deviceContext->hwSetting = 0x5;
        Numerator = 3;
        Denominator =1;
    }

    AvgTimePerFrame = (ULONG) dataFormatHeader->VideoInfoHeader.AvgTimePerFrame; // 100NS units
    MicroSecPerFrame = AvgTimePerFrame/10; 

	if (MicroSecPerFrame == 0) // Prevent divide-by-zero error
		return STATUS_INVALID_PARAMETER;
    
    FrameRate =  (1000000/MicroSecPerFrame);
    
    INTELCAM_KdPrint (MAX_TRACE, ("'format %d x %d\n", bmInfoHeader->biWidth,
                            bmInfoHeader->biHeight));
    
    PixelCount = bmInfoHeader->biWidth * bmInfoHeader->biHeight;
    bytesInImage = (bmInfoHeader->biWidth * bmInfoHeader->biHeight * 12)/8;
    RequestedPktSize = bytesInImage * FrameRate / 1000; // pkt size (ms)
    INTELCAM_KdPrint (MIN_TRACE, ("Req.Pkt Size =%d, FPS=%d \n", 
        				RequestedPktSize,FrameRate));
  
	//
	// validate the requested FPS.
	// we will not allow FPS lower than the lowest setting for the window size.
	//
	val1 = dataFormatHeader->VideoInfoHeader.AvgTimePerFrame & 0xfffffff0;
	for ( Index= BUS_BW_ARRAY_SIZE-1; Index > 0; Index-- ) {

		val2 = (BusBWArray[Index].QCIFFrameRate * Numerator / Denominator) & 0xfffffff0;
		if ( (Index == BUS_BW_ARRAY_SIZE-1 ) && ( val1 < val2) )
				break;
		else 
			if ( val1 <= val2 )  
	 			break;
	}
	
	if ( val1 > val2) {
		INTELCAM_KdPrint (MIN_TRACE, ("***ERROR*** Requested FPS is below Minimum.\n"));    
		ntStatus = STATUS_INVALID_PARAMETER;
		return ntStatus;        	
	}
       
    deviceContext->StartOffset_y = 0;
    deviceContext->StartOffset_u = PixelCount;
    deviceContext->StartOffset_v = PixelCount + PixelCount/4;        

    ntStatus = USBCAMD_ControlVendorCommand(DeviceContext,
	                                        REQUEST_SET_PREFERENCE,
    	                                    deviceContext->hwSetting,
        	                                INDEX_PREF_SCALING,
            	                            NULL,
                	                        NULL,
                    	                    FALSE,
											NULL,
											NULL);

    if (NT_SUCCESS(ntStatus)) {
        deviceContext->CurrentProperty.BusSaturation = FALSE;

ChangeFrameRate:

		deviceContext->Interface->AlternateSetting = BusBWArray[Index].AlternateSetting;

		ntStatus = USBCAMD_SelectAlternateInterface(deviceContext,
													deviceContext->Interface);
        if (NT_SUCCESS(ntStatus))
			deviceContext->CurrentProperty.RateIndex = Index;
        else {
            // No bus BW is available for this interface. Choose the next alt. interface
            // that uses less bus BW.
           deviceContext->CurrentProperty.BusSaturation = TRUE;
           if (--Index > 0) 
                goto ChangeFrameRate;
           else {
           	ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        	INTELCAM_KdPrint (MIN_TRACE, ("***ERROR*** No more ISO BW\n"));          	
           }
        }
    }			

    if (NT_SUCCESS(ntStatus)){
    
        maxDataPacketSize = 
            deviceContext->Interface->Pipes[INTELCAM_DATA_PIPE].MaximumPacketSize;            
        
        if (maxDataPacketSize != 0)
			*RawFrameLength = ( (bytesInImage / maxDataPacketSize) + 
							   ((bytesInImage % maxDataPacketSize) ? 1:0)) * maxDataPacketSize;
                
		//
		// Set the camera controls to desired values
		//
		ntStatus = INTELCAM_CameraToDriverDefaults(BusDeviceObject,deviceContext);

#ifdef USBCAMD2
        deviceContext->PrevStillIndicator = 0;
        deviceContext->SoftTrigger = 0;
#else
		deviceContext->IsVideoFrameGood = TRUE;
        deviceContext->BadPackets = 0;  
#endif
           
        AvgTimePerFrame = (ULONG) (BusBWArray[Index].QCIFFrameRate * Numerator / Denominator);
        MicroSecPerFrame = AvgTimePerFrame/10; 
        FrameRate =  (1000000/MicroSecPerFrame);
        INTELCAM_KdPrint (MIN_TRACE, ("current actual frame rate = %d\n",FrameRate)); 
    }

    return ntStatus;        
}

#ifdef USBCAMD2
/*
** INTELCAM_FreeBandwidthEx()
**
** Called after the iso video stream is stopped, this is where we 
** select an alternate interface that uses no bandwidth.
**
** Arguments:
**
**  BusDeviceObject - device object created by the hub we can submit
**                  urbs to our device through this deviceObject
**
**  DeviceContext - minidriver device  context
**
**  StreamNumber - stream number.
**
** Returns:
**
**  NTSTATUS code
**
** Side Effects:  none
*/

NTSTATUS
INTELCAM_FreeBandwidthEx(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    ULONG StreamNumber
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (StreamNumber == STREAM_Capture) {
        ntStatus = INTELCAM_FreeBandwidth(BusDeviceObject,DeviceContext);
    }
    return ntStatus;
}
#endif


/*
** INTELCAM_FreeBandwidth()
**
** Called after the iso video stream is stopped, this is where we 
** select an alternate interface that uses no bandwidth.
**
** Arguments:
**
**  BusDeviceObject - device object created by the hub we can submit
**                  urbs to our device through this deviceObject
**
**  DeviceContext - minidriver device  context
**
** Returns:
**
**  NTSTATUS code
**
** Side Effects:  none
*/

NTSTATUS
INTELCAM_FreeBandwidth(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    )
{
    NTSTATUS ntStatus;
    PINTELCAM_DEVICE_CONTEXT deviceContext = DeviceContext;

    // turn off streaming on the device

    ASSERT_DEVICE_CONTEXT(deviceContext);

    deviceContext->Interface->AlternateSetting = 
        INTELCAM_IDLE_ALT_SETTING;

    ntStatus = USBCAMD_SelectAlternateInterface(
                    deviceContext,
                    deviceContext->Interface);

    return ntStatus;
}

#ifdef USBCAMD2
/*
** INTELCAM_StopVideoCaptureEx()
**
** Called after the iso video stream is stopped, this is where we 
** select an alternate interface that uses no bandwidth.
**
** Arguments:
**
**  BusDeviceObject - device object created by the hub we can submit
**                  urbs to our device through this deviceObject
**
**  DeviceContext - minidriver device  context
**
**
**  StreamNumber - stream number.
**
** Returns:
**
**  NTSTATUS code
**
** Side Effects:  none
*/

NTSTATUS
INTELCAM_StopVideoCaptureEx(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    ULONG StreamNumber
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (StreamNumber == STREAM_Capture) {
        ntStatus = INTELCAM_StopVideoCapture(BusDeviceObject,DeviceContext);
    }
    return ntStatus;
}


#endif

/*
** INTELCAM_StopVideoCapture()
**
** Called after the iso video stream is stopped, this is where we 
** select an alternate interface that uses no bandwidth.
**
** Arguments:
**
**  BusDeviceObject - device object created by the hub we can submit
**                  urbs to our device through this deviceObject
**
**  DeviceContext - minidriver device  context
**
** Returns:
**
**  NTSTATUS code
**
** Side Effects:  none
*/

NTSTATUS
INTELCAM_StopVideoCapture(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    )
{
    NTSTATUS ntStatus;
    PINTELCAM_DEVICE_CONTEXT deviceContext = DeviceContext;

    // turn off streaming on the device

    ASSERT_DEVICE_CONTEXT(deviceContext);

    ntStatus = USBCAMD_ControlVendorCommand(DeviceContext,
                                            REQUEST_SET_PREFERENCE,
                                            0,
                                            INDEX_PREF_CAPMOTION,
                                            NULL,
                                            NULL,
                                            FALSE,
											NULL,
											NULL);
    if ( ntStatus == STATUS_SUCCESS ) {
        ntStatus = INTELCAM_SaveControlsToRegistry(BusDeviceObject,
		                                       deviceContext);
    }

    return ntStatus;
}


/*
** INTELCAM_NewFrame()
**
**  called at DPC level to allow driver to initialize a new video frame
**  context structure
**
** Arguments:
**
**  DeviceContext - minidriver device  context
**
**  FrameContext - frame context to be initialized
**
** Returns:
**
**  NTSTATUS code
**  
** Side Effects:  none
*/

VOID
INTELCAM_NewFrame(
    PVOID DeviceContext,
    PVOID FrameContext
    )
{
    PINTELCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
     
    INTELCAM_KdPrint (MAX_TRACE, ("'INTELCAM_NewFrame\n"));
    ASSERT_DEVICE_CONTEXT(deviceContext); 
}

#ifdef USBCAMD2

/*
** INTELCAM_NewFrameEx()
**
**  called at DPC level to allow driver to initialize a new video frame
**  context structure
**
** Arguments:
**
**  DeviceContext - minidriver device  context
**
**  FrameContext - frame context to be initialized
**
**  StreamNumber - stream number
**
** Returns:
**
**  NTSTATUS code
**  
** Side Effects:  none
*/

VOID
INTELCAM_NewFrameEx(
    PVOID DeviceContext,
    PVOID FrameContext,
    ULONG StreamNumber,
    PULONG FrameLength
    )
{
    PINTELCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
     
    INTELCAM_KdPrint (MAX_TRACE, ("'INTELCAM_NewFrameEx\n"));
    ASSERT_DEVICE_CONTEXT(deviceContext); 
}

#endif


/*
** INTELCAM_ProcessUSBPacket()
**
**  called at DPC level to allow driver to determine if this packet is part
**  of the current video frame or a new video frame.
**
**  This function should complete as quickly as possible, any image processing 
**  should be deferred to the ProcessRawFrame routine.
**
** Arguments:
**
**  BusDeviceObject - device object created by the hub we can submit
**                  urbs to our device through this deviceObject
**
**  DeviceContext - minidriver device  context
**
**  CurrentFrameContext - some context for this particular frame
**
**  SyncPacket - iso packet descriptor from sync pipe, not used if the interface
**              has only one pipe.
**
**  SyncBuffer - pointer to data for the sync packet
**
**  DataPacket - iso packet descriptor from data pipe
**
**  DataBuffer - pointer to data for the data packet
**
**  FrameComplete - indicates to usbcamd that this is the first data packet 
**          for a new video frame 
**
**
** Returns:
**  
** number of bytes that should be copied in to the rawFrameBuffer of FrameBuffer.
**
** Side Effects:  none
*/

ULONG
INTELCAM_ProcessUSBPacket(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PVOID CurrentFrameContext,
    PUSBD_ISO_PACKET_DESCRIPTOR SyncPacket,
    PVOID SyncBuffer,
    PUSBD_ISO_PACKET_DESCRIPTOR DataPacket,
    PVOID DataBuffer,
    PBOOLEAN FrameComplete,
    PBOOLEAN NextFrameIsStill
    )
{
    ULONG PacketFlag, ValidDataOffset;

    PacketFlag = 0;
    ValidDataOffset = 0;
    *NextFrameIsStill = FALSE;

    return  INTELCAM_ProcessUSBPacketEx(BusDeviceObject,
                                         DeviceContext,
                                         CurrentFrameContext,
                                         SyncPacket,
                                         SyncBuffer,
                                         DataPacket,
                                         DataBuffer,
                                         FrameComplete,
                                         &PacketFlag,
                                         &ValidDataOffset);

}

/*
** INTELCAM_ProcessUSBPacketEx()
**
**  called at DPC level to allow driver to determine if this packet is part
**  of the current video frame or a new video frame.
**
**  This function should complete as quickly as possible, any image processing 
**  should be deferred to the ProcessRawFrame routine.
**
** Arguments:
**
**  BusDeviceObject - device object created by the hub we can submit
**                  urbs to our device through this deviceObject
**
**  DeviceContext - minidriver device  context
**
**  CurrentFrameContext - some context for this particular frame
**
**  SyncPacket - iso packet descriptor from sync pipe, not used if the interface
**              has only one pipe.
**
**  SyncBuffer - pointer to data for the sync packet
**
**  DataPacket - iso packet descriptor from data pipe
**
**  DataBuffer - pointer to data for the data packet
**
**  FrameComplete - indicates to usbcamd that this is the first data packet 
**          for a new video frame 
**
**  PacketFlag - Handshake error code returned in this flag.
**
**  ValidDataOffset - offset from whithin the pkt USBCAMD should start copying
**          from. This is to eliminate extera buffer copy in case of inband signal.
** Returns:
**  
** number of bytes that should be copied in to the rawFrameBuffer of FrameBuffer.
**
** Side Effects:  none
*/
ULONG
INTELCAM_ProcessUSBPacketEx(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PVOID CurrentFrameContext,
    PUSBD_ISO_PACKET_DESCRIPTOR SyncPacket,
    PVOID SyncBuffer,
    PUSBD_ISO_PACKET_DESCRIPTOR DataPacket,
    PVOID DataBuffer,
    PBOOLEAN FrameComplete,
    PULONG PacketFlag,
    PULONG ValidDataOffset
    )
{
    UCHAR syncByte,StillIndicator;
    ULONG length = 0, maxDataPacketSize;
    PINTELCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
    *ValidDataOffset = 0;
    *PacketFlag =0;

    INTELCAM_KdPrint (MAX_TRACE, ("'INTELCAM_ProcessUSBPacket\n"));

    ASSERT_DEVICE_CONTEXT(deviceContext);

    maxDataPacketSize = 
        deviceContext->Interface->Pipes[INTELCAM_DATA_PIPE].MaximumPacketSize; 

    syncByte = * ((PUCHAR) SyncBuffer);

    if (!USBD_SUCCESS(SyncPacket->Status)) {

        //
        // if we get an error on the sync frame process the data packet 
        // anyway so that our buffer pointers will be updated
        //
        
        syncByte = 0;
        
    }  
    
    
    if (syncByte & USBCAMD_SOV) {
        
        //                       
        // start of new video frame
        //

        *FrameComplete = TRUE;
        INTELCAM_KdPrint (MAX_TRACE, ("'Start of New Frame #%d\n",syncByte & 0x7f)); 

        //check if existing frame has bad packets.
#ifndef USBCAMD2
        if ( deviceContext->BadPackets > 0 ) {
            // if so, set the flag.
            deviceContext->IsVideoFrameGood = FALSE;
            // and reset the bad pckt counter.
            deviceContext->BadPackets = 0;
        }
        else {
            deviceContext->IsVideoFrameGood = TRUE;
        }
#endif
		if ((DataPacket->Length == 0) || (DataPacket->Length != maxDataPacketSize)) {
			INTELCAM_KdPrint (MIN_TRACE, ("**Error** packet length = %d,  Expected Pkt Len= %d\n", 
				DataPacket->Length, maxDataPacketSize));
#ifdef USBCAMD2
                *PacketFlag |=  USBCAMD_PROCESSPACKETEX_DropFrame ;
#else
				deviceContext->BadPackets++;
#endif
			length= 0;
		}
		else {
			length = maxDataPacketSize;
			
		}

    } 
    else {
     
        if (syncByte & USBCAMD_ERROR) {

            //
            // camera went haywire. need to stop/start ISO stream.
#ifdef USBCAMD2
//            TEST_TRAP();
            INTELCAM_KdPrint (MIN_TRACE, ("Camera error. HW reset is in progress. \n"));
            
            if (deviceContext->UsbcamdInterface.Interface.Version != 0) {

                deviceContext->UsbcamdInterface.USBCAMD_SetIsoPipeState(deviceContext,
                                                                        USBCAMD_STOP_STREAM);
                deviceContext->UsbcamdInterface.USBCAMD_SetIsoPipeState(deviceContext,
                                                                        USBCAMD_START_STREAM);
            }
#endif

#if DBG
            deviceContext->IgnorePacketCount++;  
#endif                
            
            
        } else if (syncByte & USBCAMD_I) {
        
            //
            // interfield data reported for this packet, I beleive we are 
            // supposed to ignore the packet.
            //
#if DBG
            deviceContext->IgnorePacketCount++;  
#endif                
//            ILOGENTRY("igPi", FrameContext, 0, 0);
        } else {

#ifdef USBCAMD2
            // get the value of the still indicator
            StillIndicator = INTELCAM_STILL_BUTTON_PRESSED(syncByte) ^ deviceContext->PrevStillIndicator;
            // if there is a zero-to-one transition set the flag.
            if ((StillIndicator &&
                 (INTELCAM_STILL_BUTTON_PRESSED(syncByte))) || deviceContext->SoftTrigger) {
                INTELCAM_KdPrint (MAX_TRACE, ("Still button pressed. \n"));
                *PacketFlag |=  USBCAMD_PROCESSPACKETEX_CurrentFrameIsStill;
                deviceContext->SoftTrigger = FALSE;
            }

            deviceContext->PrevStillIndicator = INTELCAM_STILL_BUTTON_PRESSED(syncByte);
#endif
            
            //
            // copy the packet to the raw frame buffer
            //
			// if the packet length is 0, return 0 bytes.
			if ((DataPacket->Length == 0) || (DataPacket->Length != maxDataPacketSize)) {
				INTELCAM_KdPrint (MAX_TRACE, ("**Error** packet length = %d,  Expected Pkt Len= %d\n", 
					DataPacket->Length, maxDataPacketSize));
#ifdef USBCAMD2
                *PacketFlag |=  USBCAMD_PROCESSPACKETEX_DropFrame ;
#else
				deviceContext->BadPackets++;
#endif
				length= 0;
			}
			else {
				length = maxDataPacketSize;
			}
        }
        
    } 

    if (!USBD_SUCCESS(SyncPacket->Status) ||
	    !USBD_SUCCESS(DataPacket->Status))
	{

#ifdef USBCAMD2
                *PacketFlag |=  USBCAMD_PROCESSPACKETEX_DropFrame ;
#else
				deviceContext->BadPackets++;
#endif
	}

    return length;
}

#ifdef USBCAMD2
/*
** INTELCAM_ProcessRawVideoFrameEx()
**
**  Called at PASSIVE level to allow driver to perform any decoding of the 
**  raw video frame.
**
**    This routine will convert the packetized data in to the fromat 
**    the CODEC expects, ie y,u,v
**
**    data is always of the form 256y 64u 64v (384 byte chunks) regardless of USB
**    packet size.
**
**
** Arguments:
**
**  DeviceContext - driver context
**
**  FrameContext - driver context for this frame
**
**  FrameBuffer - pointer to the buffer that should receive the final 
**              processed video frame.
**
**  FrameLength - length of the Frame buffer (from the original read 
**                  request)
**
**  RawFrameBuffer - pointer to buffer containing the received USB packets
** 
**  RawFrameLength - length of the raw frame.
**
**  NumberOfPackets - number of USB packets received in to the RawFrameBuffer
**
**  BytesReturned - pointer to value to return for number of bytes read.
**              
** Returns:
**
**  NT status completion code for the read irp
**  
** Side Effects:  none
*/

NTSTATUS
INTELCAM_ProcessRawVideoFrameEx(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PVOID FrameContext,
    PVOID FrameBuffer,
    ULONG FrameLength,
    PVOID RawFrameBuffer,
    ULONG RawFrameLength,
    ULONG NumberOfPackets,
    PULONG BytesReturned,
    ULONG ActualRawFrameLength,
    ULONG StreamNumber
    )
{

    return INTELCAM_ProcessRawVideoFrame(BusDeviceObject,
                                         DeviceContext,
                                         FrameContext,
                                         FrameBuffer,
                                         FrameLength,
                                         RawFrameBuffer,
                                         RawFrameLength,
                                         NumberOfPackets,
                                         BytesReturned);

}
#endif



/*
** INTELCAM_ProcessRawVideoFrame()
**
**  Called at PASSIVE level to allow driver to perform any decoding of the 
**  raw video frame.
**
**    This routine will convert the packetized data in to the fromat 
**    the CODEC expects, ie y,u,v
**
**    data is always of the form 256y 64u 64v (384 byte chunks) regardless of USB
**    packet size.
**
**
** Arguments:
**
**  DeviceContext - driver context
**
**  FrameContext - driver context for this frame
**
**  FrameBuffer - pointer to the buffer that should receive the final 
**              processed video frame.
**
**  FrameLength - length of the Frame buffer (from the original read 
**                  request)
**
**  RawFrameBuffer - pointer to buffer containing the received USB packets
** 
**  RawFrameLength - length of the raw frame.
**
**  NumberOfPackets - number of USB packets received in to the RawFrameBuffer
**
**  BytesReturned - pointer to value to return for number of bytes read.
**              
** Returns:
**
**  NT status completion code for the read irp
**  
** Side Effects:  none
*/

NTSTATUS
INTELCAM_ProcessRawVideoFrame(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PVOID FrameContext,
    PVOID FrameBuffer,
    ULONG FrameLength,
    PVOID RawFrameBuffer,
    ULONG RawFrameLength,
    ULONG NumberOfPackets,
    PULONG BytesReturned
    )
{
    PINTELCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
    PINTELCAM_FRAME_CONTEXT frameContext = FrameContext;
    ULONG i, rawDataLength, processedDataLength, maxDataPacketSize;
    PUCHAR frameBuffer, rawFrameBuffer, frameBufferEnd;

    //TEST_TRAP();
    ASSERT_DEVICE_CONTEXT(deviceContext);

    frameBuffer = FrameBuffer;
    rawFrameBuffer = RawFrameBuffer;
    frameBufferEnd = frameBuffer+FrameLength;
    processedDataLength = 0;        

#ifndef USBCAMD2    
	if (deviceContext->IsVideoFrameGood == FALSE){
		*BytesReturned = 0;
        return STATUS_SUCCESS;
	}
#endif	

    ASSERT(RawFrameLength != 0);
    
    //
    // raw frame is in the format 256 bytes y 64 bytes u 64 bytes v
    //
    // we need to convert this to y u v in the frame buffer
    //

    INTELCAM_KdPrint (MAX_TRACE, ("'frameBuffer = 0x%x  max = %d numpackets = %d\n", frameBuffer,
        FrameLength, NumberOfPackets)); 

    maxDataPacketSize = 
            deviceContext->Interface->Pipes[INTELCAM_DATA_PIPE].MaximumPacketSize;            

    rawDataLength = NumberOfPackets * maxDataPacketSize;        

    INTELCAM_KdPrint (MAX_TRACE, ("'maxDataPacketSize = 0x%x  rawDataLength = 0x%x\n",
        maxDataPacketSize, rawDataLength)); 
    ILOGENTRY("prfr", 0, 0, 0);        

    INTELCAM_KdPrint (MAX_TRACE, ("'so_u = 0x%x so_v 0x%x\n",
        deviceContext->StartOffset_u, deviceContext->StartOffset_v)); 
    
    for (i=0; i<rawDataLength/384; i++) {

        //
        // copy the y
        //
        
        // check for buffer overrun
        //ASSERT(frameBuffer + (i*256) + 256 <= frameBufferEnd);

        if (i*256+256 <= deviceContext->StartOffset_u) {
            RtlCopyMemory(frameBuffer + (i*256), 
                          rawFrameBuffer,
                          256);
            processedDataLength+=256;                          
        }                          

        //
        // copy the u
        //

        // check for buffer overrun
        //ASSERT(frameBuffer + deviceContext->StartOffset_u + 
        //     (i*64) + 64 <= frameBufferEnd);

        if (i*64+deviceContext->StartOffset_u+64 <= deviceContext->StartOffset_v) {
            RtlCopyMemory(frameBuffer +
                          deviceContext->StartOffset_u + (i*64), 
                          rawFrameBuffer+256,
                          64);                        
            processedDataLength+=64;              
        }                          

        //
        // copy the v
        //

        // check for buffer overrun
        //ASSERT(frameBuffer + deviceContext->StartOffset_v + 
        //    (i*64) + 64 <= frameBufferEnd);

        if (deviceContext->StartOffset_v+(i*64)+64 <= FrameLength) {
            RtlCopyMemory(frameBuffer+
                          deviceContext->StartOffset_v + (i*64), 
                          rawFrameBuffer+256+64,
                          64);      
            processedDataLength+=64;                          
        }                          

        rawFrameBuffer+=384;
        //processedDataLength+=384;

    }

	*BytesReturned = processedDataLength;
	

    INTELCAM_KdPrint (MAX_TRACE, ("'ProcessRawVideoFrame return length = %d\n", 
        processedDataLength));

    return STATUS_SUCCESS;
    
}

/*
** INTELCAM_PropertyRequest()
**
** Arguments:
**
**  DeviceContext - driver context
**
** Returns:
**
**  NT status completion code for the read irp
**  
** Side Effects:  none
*/

NTSTATUS
INTELCAM_PropertyRequest(
    BOOLEAN SetProperty,
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PVOID PropertyContext
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PHW_STREAM_REQUEST_BLOCK srb = (PHW_STREAM_REQUEST_BLOCK)PropertyContext;
    PSTREAM_PROPERTY_DESCRIPTOR propertyDescriptor;

    propertyDescriptor = srb->CommandData.PropertyInfo;


    //
    // identify the property to set
    //
    
    if (IsEqualGUID(&PROPSETID_VIDCAP_VIDEOPROCAMP,
        &propertyDescriptor->Property->Set)){

        if (SetProperty){
            ntStatus = INTELCAM_SetCameraProperty(DeviceContext, srb);
        } else {
            ntStatus = INTELCAM_GetCameraProperty(DeviceContext, srb);
        }
    }

    else if (IsEqualGUID(&PROPSETID_CUSTOM_PROP,
                         &propertyDescriptor->Property->Set)) {
        if (SetProperty) {
            ntStatus = INTELCAM_SetCustomProperty(DeviceContext, srb);
        } else {
            ntStatus = INTELCAM_GetCustomProperty(DeviceContext, srb);
        }
    }
    
    else if (IsEqualGUID(&PROPSETID_VIDCAP_VIDEOCONTROL,
                         &propertyDescriptor->Property->Set))    {
        if (SetProperty)    {
            ntStatus = INTELCAM_SetVideoControlProperty(DeviceContext, srb);
        } else {
            ntStatus = INTELCAM_GetVideoControlProperty(DeviceContext, srb);
        }
    }
    
    else {
        ntStatus = STATUS_NOT_SUPPORTED;
    }

    return ntStatus;
}

/*
** INTELCAM_SaveState()
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/

NTSTATUS
INTELCAM_SaveState(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    )
{
    return STATUS_SUCCESS;
}    


/*
** INTELCAM_RestoreState()
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/

NTSTATUS
INTELCAM_RestoreState(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    )
{
#if 0
    PINTELCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
    LARGE_INTEGER dueTime;
    ULONG wait = 0;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    USHORT powerState;
    ULONG length;


    //
    // NOTE
    // This only works on the production level cameras.
    //

    do {

        dueTime.QuadPart = -10000 * 1000;

        (VOID) KeDelayExecutionThread(KernelMode,
                                      FALSE,
                                      &dueTime);
        length = sizeof(powerState);
        
        ntStatus = USBCAMD_ControlVendorCommand(DeviceContext,
                                         REQUEST_GET_PREFERENCE,
                                         0,
                                         INDEX_PREF_POWER,
                                         &powerState,
                                         &length,
                                         TRUE,
										 NULL,
										 NULL);

        ILOGENTRY("PWck", wait, powerState, ntStatus);

        INTELCAM_KdPrint (MIN_TRACE, ("get power state power = 0x%x, ntStatus %x\n",
            powerState, ntStatus));

        // timeout after ~10 seconds
        wait++;
        if (wait > 9 ) {
            ILOGENTRY("HWto", wait, 0, ntStatus);
            ntStatus = STATUS_DEVICE_NOT_READY;
			break;
        }

    } while ( powerState < 3);
   
    // we need to restore the hw setting for scaling after restoring power.

    ntStatus = USBCAMD_ControlVendorCommand(DeviceContext,
                                            REQUEST_SET_PREFERENCE,
                                            deviceContext->hwSetting,
                                            INDEX_PREF_SCALING,
                                            NULL,
                                            NULL,
                                            FALSE,
											NULL,
											NULL);

    return ntStatus;
#endif

    return STATUS_SUCCESS;

}    




