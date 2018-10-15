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

//////////////////////////////////////////////////////////////////////////////
//
// @doc      INTERNAL
// @module   ProxyInterface | Contains definitions to separate the different
//           Proxy calls in DeviceIoControl.

// Filename: ProxyInterface.h
//
// Routines/Functions:
//
//  public:
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////

#pragma once    // Specifies that the file, in which the pragma resides, will
                // be included (opened) only once by the compiler in a
                // build.
#pragma pack(8) // Specifies packing alignment for structure and union
                // members to byte.

#include "VampWinIoctl.h" // contains the CTL_CODE macro and corresponding definitions
#include "VampError.h"
#include "VampIfTypes.h"

#define CUSTOM_IOCTL_START                    0x0800

#define CUSTOM_IOCTL_END                      0x0FFF


#define IOCTL_CUSTOM_START                      CTL_CODE( FILE_DEVICE_VIDEO,  \
                                                        CUSTOM_IOCTL_START, \
                                                        0,0)      

#define IOCTL_CUSTOM_END                        CTL_CODE( FILE_DEVICE_VIDEO,  \
                                                        CUSTOM_IOCTL_END, \
                                                        3,0)      

// macro to mask IOCTL command group
#define IOCTL_COMMAND_GROUP(ControlCommand) ((ControlCommand >> 2) & 0xFC0)

// @head3 structure holding the common command parameters |
typedef struct
{
    DWORD       dioc_IOCtlCode;
    PVOID       dioc_InBuf;
    DWORD       dioc_cbInBuf;
    PVOID       dioc_OutBuf;
    DWORD       dioc_cbOutBuf;
} IOCTL_COMMAND, *PIOCTL_COMMAND;


//@head3 IOCTL_CAUDIO | Command group
//@defitem IOCTL_CAUDIO_Construct |
//@defitem IOCTL_CAUDIO_Destruct |
//@defitem IOCTL_CAUDIO_SetLoopbackPath |
//@defitem IOCTL_CAUDIO_SetI2CPath |
//@defitem IOCTL_CAUDIO_SetStreamPath |
//@defitem IOCTL_CAUDIO_GetPath |
//@defitem IOCTL_CAUDIO_SetFormat |
//@defitem IOCTL_CAUDIO_GetFormat |
//@defitem IOCTL_CAUDIO_GetProperty |
//@defitem IOCTL_CAUDIO_GetBufferSize |
//@defitem IOCTL_CAUDIO_Open |
//@defitem IOCTL_CAUDIO_Close |
//@defitem IOCTL_CAUDIO_Start |
//@defitem IOCTL_CAUDIO_Stop |
//@defitem IOCTL_CAUDIO_GetStatus |
//@defitem IOCTL_CAUDIO_AddBuffer |
//@defitem IOCTL_CAUDIO_RemoveBuffer |
//@defitem IOCTL_CAUDIO_GetNextDoneBuffer |
//@defitem IOCTL_CAUDIO_ReleaseLastQueuedBuffer |
//@defitem IOCTL_CAUDIO_CancelAllBuffers |

// never declare a proxy command group below 0x0800 or higher than 0xFC0 as these
// are reserved for the system. They jump in DWORD steps to give function numbers
// a wider range (from 0x00 to 0x3F, bit 0 to 5)

#define IOCTL_CAUDIO                         0x0800
#define IOCTL_CAUDIO_Construct               \
CTL_CODE(FILE_DEVICE_VIDEO, ( 1 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_Destruct                \
CTL_CODE(FILE_DEVICE_VIDEO, ( 2 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_SetLoopbackPath         \
CTL_CODE(FILE_DEVICE_VIDEO, ( 3 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_SetI2SPath              \
CTL_CODE(FILE_DEVICE_VIDEO, ( 4 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_SetStreamPath           \
CTL_CODE(FILE_DEVICE_VIDEO, ( 5 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_GetPath                 \
CTL_CODE(FILE_DEVICE_VIDEO, ( 6 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_SetFormat               \
CTL_CODE(FILE_DEVICE_VIDEO, ( 7 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_GetFormat               \
CTL_CODE(FILE_DEVICE_VIDEO, ( 8 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_GetProperty             \
CTL_CODE(FILE_DEVICE_VIDEO, ( 9 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_GetBufferSize           \
CTL_CODE(FILE_DEVICE_VIDEO, (10 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_Open                    \
CTL_CODE(FILE_DEVICE_VIDEO, (11 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_Close                   \
CTL_CODE(FILE_DEVICE_VIDEO, (12 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_Start                   \
CTL_CODE(FILE_DEVICE_VIDEO, (13 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_Stop                    \
CTL_CODE(FILE_DEVICE_VIDEO, (14 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_GetStatus               \
CTL_CODE(FILE_DEVICE_VIDEO, (15 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_AddBuffer               \
CTL_CODE(FILE_DEVICE_VIDEO, (16 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_RemoveBuffer            \
CTL_CODE(FILE_DEVICE_VIDEO, (17 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_GetNextDoneBuffer       \
CTL_CODE(FILE_DEVICE_VIDEO, (18 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_ReleaseLastQueuedBuffer \
CTL_CODE(FILE_DEVICE_VIDEO, (19 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_CancelAllBuffers        \
CTL_CODE(FILE_DEVICE_VIDEO, (20 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_SetRate                 \
CTL_CODE(FILE_DEVICE_VIDEO, (21 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_SetAdjustmentRate       \
CTL_CODE(FILE_DEVICE_VIDEO, (22 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_GetAdjustmentRate       \
CTL_CODE(FILE_DEVICE_VIDEO, (23 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_EnableRateAdjust        \
CTL_CODE(FILE_DEVICE_VIDEO, (24 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CAUDIO_EnableCopy              \
CTL_CODE(FILE_DEVICE_VIDEO, (25 | IOCTL_CAUDIO), METHOD_BUFFERED, FILE_ANY_ACCESS)

// @head3 typedefs for IOCTL_CAUDIO |
// @struct IOCTL_CAUDIOStruct | IOCTL_CAUDIO structure contains all data
//         related to the command set.
typedef struct
{
    IN OUT PVOID           pKMAudioStream;     // @field Corresponding KM
                                               //        Audio Stream object
                                               //        pointer
    OUT    VAMPRET         tVampRet;           // @field Vamp return value
                                               //        <nl> used by
                                               //        Stream/Buffer()
                                               //        functions (return)
    IN     DWORD           dwDevIndex;         // @field Device Number (Index
                                               //        reference)
    IN OUT AInput          tSource;            // @field Audio Input Source
                                               //        selection <nl> used
                                               //        by SetXXXPath() and
                                               //        GetPath() (return)
    IN     AOutput         tDestination;       // @field Audio Output
                                               //        Destination selection
                                               //        <nl> used by
                                               //        GetPath()
    IN OUT AFormat         tFormat;            // @field Audio Format
                                               //        selection <nl> used
                                               //        by SetFormat() and
                                               //        GetFormat()
                                               //        (return)
    OUT    tSampleProperty tProperty;          // @field Streaming sample
                                               //        properties <nl> used
                                               //        in GetProperty()
    OUT    DWORD           dwBufferSize;       // @field Buffer Size <nl> used
                                               //        in AddBuffer()
    IN     tStreamParms    tStreamParams;      // @field Common stream input
                                               //        parameters <nl> used
                                               //        by Open()
    IN     BOOL            bReleaseStream;     // @field Release stream <nl>
                                               //        used in Stop()
    OUT    eStreamStatus   tAudioStreamStatus; // @field Stream status
                                               //        information <nl> used
                                               //        in GetStatus()
                                               //        (return)
    IN     PVOID           pKMBuffer;          // @field Buffer object
                                               //        (Pointer) <nl> used
                                               //        in AddBuffer and
                                               //        RemoveBuffer()
    IN     int             nRate;              // @field Rate, used in
                                               //        SetRate()
    IN OUT int             nSamplesPerSecond;  // @field Samples per second,
                                               //        used in Set/
                                               //        GetAdjustmentRate()
    IN     BOOL            bEnable;            // @field Boolean, used in
                                               //        EnableRateAdjust()
                                               //        and EnableCopy()
} IOCTL_CAUDIOStruct, *pIOCTL_CAUDIOStruct;


//@head3 IOCTL_CVIDEO | Command group
//@defitem IOCTL_CVIDEO_Construct |
//@defitem IOCTL_CVIDEO_Destruct |
//@defitem IOCTL_CVIDEO_SetClipList |
//@defitem IOCTL_CVIDEO_Open |
//@defitem IOCTL_CVIDEO_Close |
//@defitem IOCTL_CVIDEO_Start |
//@defitem IOCTL_CVIDEO_Stop |
//@defitem IOCTL_CVIDEO_GetStatus |
//@defitem IOCTL_CVIDEO_AddBuffer |
//@defitem IOCTL_CVIDEO_RemoveBuffer |
//@defitem IOCTL_CVIDEO_GetNextDoneBuffer |
//@defitem IOCTL_CVIDEO_ReleaseLastQueuedBuffer |
//@defitem IOCTL_CVIDEO_CancelAllBuffers |
#define IOCTL_CVIDEO                         0x0840
#define IOCTL_CVIDEO_Construct               \
CTL_CODE(FILE_DEVICE_VIDEO, ( 1 | IOCTL_CVIDEO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVIDEO_Destruct                \
CTL_CODE(FILE_DEVICE_VIDEO, ( 2 | IOCTL_CVIDEO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVIDEO_SetClipList             \
CTL_CODE(FILE_DEVICE_VIDEO, ( 3 | IOCTL_CVIDEO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVIDEO_Open                    \
CTL_CODE(FILE_DEVICE_VIDEO, ( 4 | IOCTL_CVIDEO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVIDEO_Close                   \
CTL_CODE(FILE_DEVICE_VIDEO, ( 5 | IOCTL_CVIDEO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVIDEO_Start                   \
CTL_CODE(FILE_DEVICE_VIDEO, ( 6 | IOCTL_CVIDEO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVIDEO_Stop                    \
CTL_CODE(FILE_DEVICE_VIDEO, ( 7 | IOCTL_CVIDEO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVIDEO_GetStatus               \
CTL_CODE(FILE_DEVICE_VIDEO, ( 8 | IOCTL_CVIDEO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVIDEO_AddBuffer               \
CTL_CODE(FILE_DEVICE_VIDEO, ( 9 | IOCTL_CVIDEO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVIDEO_RemoveBuffer            \
CTL_CODE(FILE_DEVICE_VIDEO, (10 | IOCTL_CVIDEO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVIDEO_GetNextDoneBuffer       \
CTL_CODE(FILE_DEVICE_VIDEO, (11 | IOCTL_CVIDEO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVIDEO_ReleaseLastQueuedBuffer \
CTL_CODE(FILE_DEVICE_VIDEO, (12 | IOCTL_CVIDEO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVIDEO_CancelAllBuffers        \
CTL_CODE(FILE_DEVICE_VIDEO, (13 | IOCTL_CVIDEO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVIDEO_SetClipMode             \
CTL_CODE(FILE_DEVICE_VIDEO, (14 | IOCTL_CVIDEO), METHOD_BUFFERED, FILE_ANY_ACCESS)

// @head3 typedefs for IOCTL_CVIDEO |
// @struct IOCTL_CVIDEOStruct | IOCTL_CVIDEO structure contains all data
//         related to the command set.
typedef struct
{
    IN OUT PVOID         pKMVideoStream;     // @field Corresponding KM Video
                                             //        Stream object pointer
    OUT    VAMPRET       tVampRet;           // @field Vamp return value <nl>
                                             //        used by Stream/Buffer()
                                             //        functions (return)
    IN     DWORD         dwDevIndex;         // @field Device Number (Index
                                             //        reference)
    IN     tRectList     RectList;           // @field Visible rectangle list,
                                             //        used in SetVisibleRect()
    IN     tStreamParms  tStreamParams;      // @field common stream input
                                             //        parameters used in
                                             //        Open()
    OUT    eStreamStatus tVideoStreamStatus; // @field contains the status
                                             //        info of the stream
    IN     BOOL          bReleaseStream;     // @field release stream, true or
                                             //        false used in Stop()
    IN     PVOID         pKMBuffer;          // @field buffer object (pointer)
                                             //        used in AddBuffer() and
                                             //        RemoveBuffer()
    IN     eClipMode     nClipMode;          // @field clip mode used in
                                             //        SetClipMode()
} IOCTL_CVIDEOStruct, *pIOCTL_CVIDEOStruct;


//@head3 IOCTL_CVBI | Command group
//@defitem IOCTL_CVBI_Construct |
//@defitem IOCTL_CVBI_Destruct |
//@defitem IOCTL_CVBI_Open |
//@defitem IOCTL_CVBI_Close |
//@defitem IOCTL_CVBI_Start |
//@defitem IOCTL_CVBI_Stop |
//@defitem IOCTL_CVBI_GetStatus |
//@defitem IOCTL_CVBI_AddBuffer |
//@defitem IOCTL_CVBI_RemoveBuffer |
//@defitem IOCTL_CVBI_GetNextDoneBuffer |
//@defitem IOCTL_CVBI_ReleaseLastQueuedBuffer |
//@defitem IOCTL_CVBI_CancelAllBuffers |
#define IOCTL_CVBI                         0x0880
#define IOCTL_CVBI_Construct               \
CTL_CODE(FILE_DEVICE_VIDEO, ( 1 | IOCTL_CVBI), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVBI_Destruct                \
CTL_CODE(FILE_DEVICE_VIDEO, ( 2 | IOCTL_CVBI), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVBI_Open                    \
CTL_CODE(FILE_DEVICE_VIDEO, ( 3 | IOCTL_CVBI), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVBI_Close                   \
CTL_CODE(FILE_DEVICE_VIDEO, ( 4 | IOCTL_CVBI), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVBI_Start                   \
CTL_CODE(FILE_DEVICE_VIDEO, ( 5 | IOCTL_CVBI), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVBI_Stop                    \
CTL_CODE(FILE_DEVICE_VIDEO, ( 6 | IOCTL_CVBI), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVBI_GetStatus               \
CTL_CODE(FILE_DEVICE_VIDEO, ( 7 | IOCTL_CVBI), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVBI_AddBuffer               \
CTL_CODE(FILE_DEVICE_VIDEO, ( 8 | IOCTL_CVBI), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVBI_RemoveBuffer            \
CTL_CODE(FILE_DEVICE_VIDEO, ( 9 | IOCTL_CVBI), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVBI_GetNextDoneBuffer       \
CTL_CODE(FILE_DEVICE_VIDEO, (10 | IOCTL_CVBI), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVBI_ReleaseLastQueuedBuffer \
CTL_CODE(FILE_DEVICE_VIDEO, (11 | IOCTL_CVBI), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CVBI_CancelAllBuffers        \
CTL_CODE(FILE_DEVICE_VIDEO, (12 | IOCTL_CVBI), METHOD_BUFFERED, FILE_ANY_ACCESS)

// @head3 typedefs for IOCTL_CVBI |
// @struct IOCTL_CVBIStruct | IOCTL_CVBI structure contains all data
//         related to the command set.
typedef struct
{
    IN OUT PVOID         pKMVbiStream;     // @field Corresponding KM Audio
                                           //        Stream object pointer
    OUT    VAMPRET       tVampRet;         // @field Vamp return value <nl>
                                           //        used by Stream/Buffer()
                                           //        functions (return)
    IN     DWORD         dwDevIndex;       // @field Device Number (Index
                                           //        reference)
    IN     tStreamParms  tStreamParams;    // @field common stream input
                                           //        parameters used in
                                           //        Open()
    OUT    eStreamStatus tVbiStreamStatus; // @field contains the status info
                                           //        of the stream
    IN     BOOL          bReleaseStream;   // @field release stream, true or
                                           //        false used in Stop()
    IN     PVOID         pKMBuffer;        // @field buffer object (pointer)
                                           //        used in AddBuffer() and
                                           //        RemoveBuffer()
} IOCTL_CVBIStruct, *pIOCTL_CVBIStruct;


//@head3 IOCTL_CTRANSPORT | Command group
//@defitem IOCTL_CTRANSPORT_Construct |
//@defitem IOCTL_CTRANSPORT_Destruct |
//@defitem IOCTL_CTRANSPORT_Open |
//@defitem IOCTL_CTRANSPORT_Close |
//@defitem IOCTL_CTRANSPORT_Start |
//@defitem IOCTL_CTRANSPORT_Stop |
//@defitem IOCTL_CTRANSPORT_GetStatus |
//@defitem IOCTL_CTRANSPORT_AddBuffer |
//@defitem IOCTL_CTRANSPORT_RemoveBuffer |
//@defitem IOCTL_CTRANSPORT_GetNextDoneBuffer |
//@defitem IOCTL_CTRANSPORT_ReleaseLastQueuedBuffer |
//@defitem IOCTL_CTRANSPORT_CancelAllBuffers |
//@defitem IOCTL_CTRANSPORT_Reset |
//@defitem IOCTL_CTRANSPORT_GetPinsConfiguration |
#define IOCTL_CTRANSPORT                         0x08C0
#define IOCTL_CTRANSPORT_Construct               \
CTL_CODE(FILE_DEVICE_VIDEO, ( 1 | IOCTL_CTRANSPORT), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CTRANSPORT_Destruct                \
CTL_CODE(FILE_DEVICE_VIDEO, ( 2 | IOCTL_CTRANSPORT), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CTRANSPORT_Open                    \
CTL_CODE(FILE_DEVICE_VIDEO, ( 3 | IOCTL_CTRANSPORT), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CTRANSPORT_Close                   \
CTL_CODE(FILE_DEVICE_VIDEO, ( 4 | IOCTL_CTRANSPORT), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CTRANSPORT_Start                   \
CTL_CODE(FILE_DEVICE_VIDEO, ( 5 | IOCTL_CTRANSPORT), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CTRANSPORT_Stop                    \
CTL_CODE(FILE_DEVICE_VIDEO, ( 6 | IOCTL_CTRANSPORT), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CTRANSPORT_GetStatus               \
CTL_CODE(FILE_DEVICE_VIDEO, ( 7 | IOCTL_CTRANSPORT), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CTRANSPORT_AddBuffer               \
CTL_CODE(FILE_DEVICE_VIDEO, ( 8 | IOCTL_CTRANSPORT), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CTRANSPORT_RemoveBuffer            \
CTL_CODE(FILE_DEVICE_VIDEO, ( 9 | IOCTL_CTRANSPORT), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CTRANSPORT_GetNextDoneBuffer       \
CTL_CODE(FILE_DEVICE_VIDEO, (10 | IOCTL_CTRANSPORT), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CTRANSPORT_ReleaseLastQueuedBuffer \
CTL_CODE(FILE_DEVICE_VIDEO, (11 | IOCTL_CTRANSPORT), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CTRANSPORT_CancelAllBuffers        \
CTL_CODE(FILE_DEVICE_VIDEO, (12 | IOCTL_CTRANSPORT), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CTRANSPORT_Reset                   \
CTL_CODE(FILE_DEVICE_VIDEO, (13 | IOCTL_CTRANSPORT), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CTRANSPORT_GetPinsConfiguration    \
CTL_CODE(FILE_DEVICE_VIDEO, (14 | IOCTL_CTRANSPORT), METHOD_BUFFERED, FILE_ANY_ACCESS)

// @head3 typedefs for IOCTL_CTRANSPORT |
// @struct IOCTL_CTRANSPORTStruct | IOCTL_CTRANSPORT structure contains all
//         data related to the command set.
typedef struct
{
    IN OUT PVOID         pKMTransportStream;     // @field Corresponding KM
                                                 //        Audio Stream object
                                                 //        pointer
    OUT    VAMPRET       tVampRet;               // @field Vamp return value
                                                 //        <nl> used by
                                                 //        Stream/Buffer()
                                                 //        functions
                                                 //        (return)
    IN     DWORD         dwDevIndex;             // @field Device Number <nl>
                                                 //        (Index reference)
    IN     eTSStreamType tStreamType;            // @field Type of Stream <nl>
                                                 //        used in
                                                 //        constructor
    IN     tStreamParms  tStreamParams;          // @field Common stream input
                                                 //        parameters used by
                                                 //        Open()
    IN     BOOL          bReleaseStream;         // @field Release stream <nl>
                                                 //        used in Stop
    OUT    eStreamStatus tTransportStreamStatus; // @field Stream status
                                                 //        information <nl>
                                                 //        used in GetStatus()
                                                 //        (return)
    IN     PVOID         pKMBuffer;              // @field Buffer object
                                                 //        (Pointer) <nl> used
                                                 //        in AddBuffer() and
                                                 //        RemoveBuffer<nl>
    OUT    DWORD         dwPinConfig;            // @field Value indicates Pin
                                                 //        configuration <nl>
                                                 //        used by
                                                 //        GetPinConfiguration()
                                                 //        (return)
} IOCTL_CTRANSPORTStruct, *pIOCTL_CTRANSPORTStruct;


//@head3 IOCTL_CGPIO_STATIC | Command group
//@defitem IOCTL_CGPIO_STATIC_Construct |
//@defitem IOCTL_CGPIO_STATIC_Destruct |
//@defitem IOCTL_CGPIO_STATIC_WriteToPinNr |
//@defitem IOCTL_CGPIO_STATIC_WriteToGPIONr |
//@defitem IOCTL_CGPIO_STATIC_ReadFromPinNr |
//@defitem IOCTL_CGPIO_STATIC_ReadFromGPIONr |
//@defitem IOCTL_CGPIO_STATIC_GetPinsConfiguration |
//@defitem IOCTL_CGPIO_STATIC_Reset |
#define IOCTL_CGPIO_STATIC                         0x0900
#define IOCTL_CGPIO_STATIC_Construct               \
CTL_CODE(FILE_DEVICE_VIDEO, ( 1 | IOCTL_CGPIO_STATIC), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CGPIO_STATIC_Destruct                \
CTL_CODE(FILE_DEVICE_VIDEO, ( 2 | IOCTL_CGPIO_STATIC), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CGPIO_STATIC_WriteToPinNr            \
CTL_CODE(FILE_DEVICE_VIDEO, ( 3 | IOCTL_CGPIO_STATIC), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CGPIO_STATIC_WriteToGPIONr           \
CTL_CODE(FILE_DEVICE_VIDEO, ( 4 | IOCTL_CGPIO_STATIC), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CGPIO_STATIC_ReadFromPinNr           \
CTL_CODE(FILE_DEVICE_VIDEO, ( 5 | IOCTL_CGPIO_STATIC), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CGPIO_STATIC_ReadFromGPIONr          \
CTL_CODE(FILE_DEVICE_VIDEO, ( 6 | IOCTL_CGPIO_STATIC), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CGPIO_STATIC_GetPinsConfiguration    \
CTL_CODE(FILE_DEVICE_VIDEO, ( 7 | IOCTL_CGPIO_STATIC), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CGPIO_STATIC_Reset    \
CTL_CODE(FILE_DEVICE_VIDEO, ( 8 | IOCTL_CGPIO_STATIC), METHOD_BUFFERED, FILE_ANY_ACCESS)

// @head3 typedefs for IOCTL_CGPIOSTATIC |
// @struct IOCTL_CGPIOSTATICStruct | IOCTL_CGPIOSTATIC structure contains all
//         data related to the command set.
typedef struct
{
    IN OUT PVOID   pKMGPIOStatic; // @field Corresponding KM GPIO object
                                  //        pointer
    OUT    VAMPRET tVampRet;      // @field Vamp return value <nl> used by
                                  //        WriteXXX()/ ReadXXX() functions
                                  //        (return)
    IN     DWORD   dwDevIndex;    // @field Device Number (Index reference)
    IN     BYTE    nPinNr;        // @field Pin number <nl> used by
                                  //        WriteToPinNr() and
                                  //        ReadFromPinNr()
    IN     BYTE    nGPIONr;       // @field GPIO number <nl> used by
                                  //        WriteToGPIONr() and
                                  //        ReadFromGPIONr()
    IN OUT BYTE    ucValue;       // @field Value to be r/w <nl> used by
                                  //        Read/WriteToXXXNr()
    OUT    DWORD   dwPinConfig;   // @field Value indicates Pin configuration
                                  //        <nl> used by GetPinConfiguration()
                                  //        (return)
} IOCTL_CGPIO_STATICStruct, *pIOCTL_CGPIO_STATICStruct;


//@head3 IOCTL_CI2C | Command group
//@defitem IOCTL_CI2C_Construct |
//@defitem IOCTL_CI2C_Destruct |
//@defitem IOCTL_CI2C_SetSlave |
//@defitem IOCTL_CI2C_GetSlave |
//@defitem IOCTL_CI2C_SetTimeout |
//@defitem IOCTL_CI2C_GetTimeout |
//@defitem IOCTL_CI2C_SetClockSel |
//@defitem IOCTL_CI2C_GetClockSel |
//@defitem IOCTL_CI2C_WriteSequence |
//@defitem IOCTL_CI2C_ReadSequence|
#define IOCTL_CI2C                  0x0940
#define IOCTL_CI2C_SetSlave         \
CTL_CODE(FILE_DEVICE_VIDEO, ( 1 | IOCTL_CI2C), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CI2C_GetSlave         \
CTL_CODE(FILE_DEVICE_VIDEO, ( 2 | IOCTL_CI2C), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CI2C_SetTimeout       \
CTL_CODE(FILE_DEVICE_VIDEO, ( 3 | IOCTL_CI2C), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CI2C_GetTimeout       \
CTL_CODE(FILE_DEVICE_VIDEO, ( 4 | IOCTL_CI2C), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CI2C_SetClockSel      \
CTL_CODE(FILE_DEVICE_VIDEO, ( 5 | IOCTL_CI2C), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CI2C_GetClockSel      \
CTL_CODE(FILE_DEVICE_VIDEO, ( 6 | IOCTL_CI2C), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CI2C_WriteSequence    \
CTL_CODE(FILE_DEVICE_VIDEO, ( 7 | IOCTL_CI2C), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CI2C_ReadSequence     \
CTL_CODE(FILE_DEVICE_VIDEO, ( 8 | IOCTL_CI2C), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CI2C_ReadSeq          \
CTL_CODE(FILE_DEVICE_VIDEO, ( 9 | IOCTL_CI2C), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CI2C_ReadByte         \
CTL_CODE(FILE_DEVICE_VIDEO, (10 | IOCTL_CI2C), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CI2C_WriteSeq         \
CTL_CODE(FILE_DEVICE_VIDEO, (11 | IOCTL_CI2C), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CI2C_WriteByte        \
CTL_CODE(FILE_DEVICE_VIDEO, (12 | IOCTL_CI2C), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CI2C_CombinedSeq      \
CTL_CODE(FILE_DEVICE_VIDEO, (13 | IOCTL_CI2C), METHOD_BUFFERED, FILE_ANY_ACCESS)

// @head3 typedefs for IOCTL_CI2C |
// @struct IOCTL_CI2CStruct | IOCTL_CI2C structure contains all data related
//         to the command set.
typedef struct
{
    IN     DWORD      dwDevIndex;       // @field Device Number <nl> (Index
                                        //        reference)
    IN OUT INT        nSlave;           // @field Slave ID (default to zero)
                                        //        <nl> used by constructor,
                                        //        SetSlave() and GetSlave()
                                        //        (return)
    IN OUT DWORD      dwTimeout;        // @field Timeout value <nl> used by
                                        //        SetTimeout() and
                                        //        GetTimeout() (return)
    IN OUT DWORD      dwClockSel;       // @field I2C clock rate <nl> used by
                                        //        SetClockSel() and
                                        //        GetClockSel() (return)
    IN     BYTE       ucSubaddress;     // @field Register subaddress <nl>
                                        //        used by XXXSequence()
    IN     BYTE*      pucSequenceWr;    // @field Data sequence (Pointer) <nl>
                                        //        used by WriteXXX()
    OUT    BYTE*      pucSequenceRd;    // @field Data sequence (Pointer) <nl>
                                        //        used by ReadXXX()
    IN     BYTE       ucByteWr;         // @field Data sequence <nl>
                                        //        used by WriteXXX()
    OUT    BYTE       ucByteRd;         // @field Data sequence <nl>
                                        //        used by ReadXXX()
    IN     INT        nNumberOfBytesWr; // @field Number of data bytes to
                                        //        send/write <nl> used by
                                        //        XXXSequence()
    IN     INT        nNumberOfBytesRd; // @field Number of data bytes to
                                        //        send/write <nl> used by
                                        //        XXXSequence()
    IN     eI2cAttr   eAttr;            // @field Atribute of sequence step
                                        //        <nl> used in XXXByte()
    OUT    eI2cStatus tI2cStatus;       // @field I2C Register Status <nl>
                                        //        used by XXXSequence()
                                        //        (return)
} IOCTL_CI2CStruct, *pIOCTL_CI2CStruct;


//@head3 IOCTL_CDECODER | Command group
//@defitem IOCTL_CDECODER_Construct |
//@defitem IOCTL_CDECODER_Destruct |
//@defitem IOCTL_CDECODER_SetStandard |
//@defitem IOCTL_CDECODER_GetStandard |
//@defitem IOCTL_CDECODER_SetColor |
//@defitem IOCTL_CDECODER_GetColor |
//@defitem IOCTL_CDECODER_SetSourceType |
//@defitem IOCTL_CDECODER_GetSourceType |
//@defitem IOCTL_CDECODER_SetVideoSource |
//@defitem IOCTL_CDECODER_GetVideoSource |
//@defitem IOCTL_CDECODER_SetVideoFormat |
//@defitem IOCTL_CDECODER_GetVideoFormat |
//@defitem IOCTL_CDECODER_GetDecoderStatus |
//@defitem IOCTL_CDECODER_SetCombFilter |
//@defitem IOCTL_CDECODER_GetCombFilter |
#define IOCTL_CDECODER                      0x0980
#define IOCTL_CDECODER_SetStandard          \
CTL_CODE(FILE_DEVICE_VIDEO, ( 1 | IOCTL_CDECODER), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CDECODER_GetStandard          \
CTL_CODE(FILE_DEVICE_VIDEO, ( 2 | IOCTL_CDECODER), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CDECODER_SetColor             \
CTL_CODE(FILE_DEVICE_VIDEO, ( 3 | IOCTL_CDECODER), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CDECODER_GetColor             \
CTL_CODE(FILE_DEVICE_VIDEO, ( 4 | IOCTL_CDECODER), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CDECODER_SetSourceType        \
CTL_CODE(FILE_DEVICE_VIDEO, ( 5 | IOCTL_CDECODER), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CDECODER_GetSourceType        \
CTL_CODE(FILE_DEVICE_VIDEO, ( 6 | IOCTL_CDECODER), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CDECODER_SetVideoSource       \
CTL_CODE(FILE_DEVICE_VIDEO, ( 7 | IOCTL_CDECODER), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CDECODER_GetVideoSource       \
CTL_CODE(FILE_DEVICE_VIDEO, ( 8 | IOCTL_CDECODER), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CDECODER_GetDecoderStatus     \
CTL_CODE(FILE_DEVICE_VIDEO, ( 9 | IOCTL_CDECODER), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CDECODER_SetCombFilter        \
CTL_CODE(FILE_DEVICE_VIDEO, (10 | IOCTL_CDECODER), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CDECODER_GetCombFilter        \
CTL_CODE(FILE_DEVICE_VIDEO, (11 | IOCTL_CDECODER), METHOD_BUFFERED, FILE_ANY_ACCESS)

// @head3 typedefs for IOCTL_CDECODER |
// @struct IOCTL_CDECODERStruct | IOCTL_CDECODER structure contains all data
//         related to the command set.
typedef struct
{
    OUT    VAMPRET        tVampRet;       // @field Vamp return value <nl>
                                          //        used by Stream/Buffer()
                                          //        functions (return)
    IN     DWORD          dwDevIndex;     // @field Device Number (Index
                                          //        reference)
    IN OUT eVideoStandard tVideoStandard; // @field actual video standard
                                          //        value used in
                                          //        Set/GetStandard()
    IN     eColorControl  tColorControl;  // @field Color control attribute
                                          //        used in Set/GetColor()
    IN     SHORT          sColorValue;    // @field Color control attribute
                                          //        value used in SetColor()
    OUT    SHORT          sCurrentValue;  // @field Color control attribute
                                          //        value used in GetColor()
    OUT    SHORT          sMinValue;      // @field Color control attribute
                                          //        value used in GetColor()
    OUT    SHORT          sMaxValue;      // @field Color control attribute
                                          //        value used in GetColor()
    OUT    SHORT          sDefValue;      // @field Color control attribute
                                          //        value used in
                                          //        GetColor()
    IN     eVideoStatus   tStatusId;      // @field Decoder register Id, which
                                          //        status should be returned
                                          //        by GetStatus()
    OUT    DWORD dwStatus;                // @field Status of the decoder
                                          //        register, returned by
                                          //        GetStatus()
    IN OUT eVideoSourceType tSourceType;  // @field Source type (e.g.
                                          //        VD_Camera, VD_TV,...) used
                                          //        in SetSourceType()
    OUT    DWORD  dwMode;                 // @field current video input mode
                                          //        used in GetVideoSource()
    IN OUT eVideoSource  tVideoSource;    // @field Video Source used in Set/
                                          //        GetVideoSource()
    OUT    BOOL           bAutoStandard;  // @field Boolean used in
                                          //        GetStandard()
    IN OUT BOOL           bLuminance;     // @field Boolean used in
                                          //        Set/GetCombFilter()
    IN OUT BOOL           bChrominance;   // @field Boolean used in
                                          //        Set/GetCombFilter()
    IN OUT BOOL           bBypass;        // @field Boolean used in
                                          //        Set/GetCombFilter()
} IOCTL_CDECODERStruct, *pIOCTL_CDECODERStruct;


//@head3 IOCTL_CBUFFER | Command group
//@defitem IOCTL_CBUFFER_Construct |
//@defitem IOCTL_CBUFFER_Destruct |
//@defitem IOCTL_CBUFFER_SetContext |
//@defitem IOCTL_CBUFFER_GetContext |
//@defitem IOCTL_CBUFFER_GetStatus |
//@defitem IOCTL_CBUFFER_GetFormat |
#define IOCTL_CBUFFER                       0x09C0
#define IOCTL_CBUFFER_Construct             \
CTL_CODE(FILE_DEVICE_VIDEO, ( 1 | IOCTL_CBUFFER), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CBUFFER_Destruct              \
CTL_CODE(FILE_DEVICE_VIDEO, ( 2 | IOCTL_CBUFFER), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CBUFFER_SetContext            \
CTL_CODE(FILE_DEVICE_VIDEO, ( 3 | IOCTL_CBUFFER), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CBUFFER_GetContext            \
CTL_CODE(FILE_DEVICE_VIDEO, ( 4 | IOCTL_CBUFFER), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CBUFFER_GetStatus             \
CTL_CODE(FILE_DEVICE_VIDEO, ( 5 | IOCTL_CBUFFER), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CBUFFER_GetFormat             \
CTL_CODE(FILE_DEVICE_VIDEO, ( 6 | IOCTL_CBUFFER), METHOD_BUFFERED, FILE_ANY_ACCESS)

// @head3 typedefs for IOCTL_CBUFFER |
// @struct IOCTL_CBUFFERStruct | IOCTL_CBUFFER structure contains all data
//         related to the command set.
typedef struct
{
    IN OUT PVOID         pKMBuffer;      // @field Corresponding KM Buffer
                                         //        object pointer
    IN     PVOID         pBufferAddress; // @field address of the buffer used
                                         //        in the Constructor
    IN     size_t        tSize;          // @field size of the buffer in bytes
                                         //        used in the Constructor
    IN OUT tBufferFormat tFormat;        // @field format of the buffer used
                                         //        in the Constructor
    OUT    tBufferStatus tStatus;        // @field Buffer status used in
                                         //        GetStatus()
    IN OUT PVOID         pContext;       // @field pointer on a driver / user
                                         //        context used in
                                         //        Set/GetContext()
    IN     BOOL bContiguousMemoryBuffer; // @field Boolean if the buffer
                                         //        resides in contiguous
                                         //        memory used in the
                                         //        Constructor
} IOCTL_CBUFFERStruct, *pIOCTL_CBUFFERStruct;


//@head3 IOCTL_CDEVICE_INFO | Command group
//@defitem IOCTL_CDEVICE_INFO_GetNumOfDevices |
//@defitem IOCTL_CDEVICE_INFO_GetInfoOfDevices |
#define IOCTL_CDEVICE_INFO                 0x0A00
#define IOCTL_CDEVICE_INFO_GetNumOfDevices \
CTL_CODE(FILE_DEVICE_VIDEO, ( 1 | IOCTL_CDEVICE_INFO), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CDEVICE_INFO_GetInfoOfDevice \
CTL_CODE(FILE_DEVICE_VIDEO, ( 2 | IOCTL_CDEVICE_INFO), METHOD_BUFFERED, FILE_ANY_ACCESS)

// @head3 typedefs for IOCTL_CDEVICE_INFO |
// @struct IOCTL_DEVICE_INFOStruct | IOCTL_CDEVICE_INFO structure contains all data
//         related to the command set.
typedef struct
{
    IN  DWORD         dwDevIndex;     // @field Device Number (Index
                                      //        reference)
    OUT DWORD         dwNumOfDevices; // @field number of devices
    OUT tDeviceParams tInfoOfDevice;  // @field devices information
} IOCTL_CDEVICE_INFOStruct, *pIOCTL_CDEVICE_INFOStruct;


//@head3 IOCTL_VM | Command group
//@defitem IOCTL_VM_InstallCommonEvent |
//@defitem IOCTL_VM_RemoveCommonEvent |
//@defitem IOCTL_VM_GetNextCallback |
#define IOCTL_VM                   0x0A40
#define IOCTL_VM_InstallEvent      \
CTL_CODE(FILE_DEVICE_VIDEO, ( 1 | IOCTL_VM), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VM_RemoveEvent       \
CTL_CODE(FILE_DEVICE_VIDEO, ( 2 | IOCTL_VM), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VM_GetNextCallback   \
CTL_CODE(FILE_DEVICE_VIDEO, ( 3 | IOCTL_VM), METHOD_BUFFERED, FILE_ANY_ACCESS)

// @head3 typedefs for IOCTL_VM |
// @struct IOCTL_VMStruct | IOCTL_VM structure contains all data related to
//         the command set.
typedef struct
{
    OUT    VAMPRET     tVampRet;        // @field Vamp return value
    IN     HANDLE      hEvent;          // @field Pointer to Event Handle <nl>
                                        //        used in
                                        //        InstallCommonEvent()
    IN OUT PVOID       pKMEventObj;     // @field Pointer to the kernel mode
                                        //        object of the common event
                                        //        <nl> used in
                                        //        Install/RemoveCommonEvent()
                                        //        and GetNextCallback()
    OUT    eEventType tEventType;       // @field Event type that was signaled
                                        //        <nl> used in
                                        //        GetNextCallback()
    OUT    PVOID      pUMCallbackObj;   // @field Pointer to the user mode
                                        //        object pointer that contains
                                        //        the callback <nl> used in
                                        //        GetNextCallback()
    OUT    PVOID      pUMCallbackParam; // @field Pointer to the user mode
                                        //        callback parameter
                                        //        <nl> used in
                                        //        GetNextCallback()
} IOCTL_VMStruct, *pIOCTL_VMStruct;


//@head3 IOCTL_CEVENTHANDLER | Command group
//@defitem IOCTL_CEVENTHANDLER_AddEventNotify |
//@defitem IOCTL_CEVENTHANDLER_RemoveEventNotify |
#define IOCTL_CEVENTHANDLER                   0x0A80
#define IOCTL_CEVENTHANDLER_AddEventNotify    \
CTL_CODE(FILE_DEVICE_VIDEO, ( 1 | IOCTL_CEVENTHANDLER), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CEVENTHANDLER_RemoveEventNotify \
CTL_CODE(FILE_DEVICE_VIDEO, ( 2 | IOCTL_CEVENTHANDLER), METHOD_BUFFERED, FILE_ANY_ACCESS)

// @head3 typedefs for IOCTL_CEVENTHANDLER |
// @struct IOCTL_CEVENTHANDLERStruct | IOCTL_CEVENTHANDLER structure contains
//         all data related to the the command set.
typedef struct
{
    OUT    VAMPRET    tVampRet;         // @field Vamp return value
    IN     DWORD      dwDevIndex;       // @field Device Number (Index
                                        //        reference)
    IN OUT PVOID      pKMEventObj;      // @field Pointer to the kernel mode
                                        //        object of the common event
                                        //        <nl> used in
                                        //        Add/RemoveEventNotify()
    IN OUT PVOID      pEventHandle;     // @field event handle, used in
                                        //        RemoveEventNotify()
    IN     eEventType tEventType;       // @field Event type that is to be
                                        //        signaled <nl> used in
                                        //        AddEventNotify()
    IN OUT PVOID      pUMCallbackObj;   // @field Pointer to the user mode.
                                        //        object pointer that contains
                                        //        the callback <nl> used in
                                        //        AddEventNotify()
    IN OUT PVOID      pUMCallbackParam; // @field Pointer to the user mode
                                        //        callback parameter
                                        //        <nl> used in
                                        //        AddEventNotify()
} IOCTL_CEVENTHANDLERStruct, *pIOCTL_CEVENTHANDLERStruct;


//@head3 IOCTL_COSDEPEND | Command group concerning the OS dependant
//       commands.
//@defitem IOCTL_RELOAD_DBGLEVEL |
//@defitem IOCTL_GET_REGISTRYPATH |
//@defitem IOCTL_COSDEPEND_SetChannel |
#define IOCTL_COSDEPEND                     0x0AC0
#define IOCTL_RELOAD_DBGLEVEL  \
CTL_CODE(FILE_DEVICE_VIDEO, ( 1 | IOCTL_COSDEPEND), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_REGISTRYPATH     \
CTL_CODE(FILE_DEVICE_VIDEO, ( 2 | IOCTL_COSDEPEND), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_COSDEPEND_SetChannel           \
CTL_CODE(FILE_DEVICE_VIDEO, ( 3 | IOCTL_COSDEPEND), METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct
{
    OUT CHAR pszRegistryPath[MAX_PATH]; // @field Contains the registry path
                                        //        to the DebugLevels <nl> used
                                        //        in GetRegistryPath()
    IN     DWORD         dwChannel;     // @field TV Channel used in
                                        //        SetChannel()
} IOCTL_COSDEPENDStruct, *pIOCTL_COSDEPENDStruct;


//@head3 IOCTL_PCI | Command group
//@defitem IOCTL_PCI_REGISTER_Set |
//@defitem IOCTL_PCI_REGISTER_Get |
#define IOCTL_DEVICE_PCI                   0x0B00
#define IOCTL_DEVICE_PCI_REGISTER_Set      \
CTL_CODE(FILE_DEVICE_VIDEO, ( 1 | IOCTL_DEVICE_PCI), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DEVICE_PCI_REGISTER_Get      \
CTL_CODE(FILE_DEVICE_VIDEO, ( 2 | IOCTL_DEVICE_PCI), METHOD_BUFFERED, FILE_ANY_ACCESS)

// @head3 typedefs for IOCTL_DEVICE_PCI
// @struct IOCTL_DEVICE_PCIStruct | IOCTL_DEVICE_PCI structure contains all
//         data related to the command set.
typedef struct
{
    IN     DWORD dwDevIndex;           // @field Device Number (Index reference)
    IN     DWORD dwPCIRegisterOffset;  // @field offset of the register to access
    IN OUT DWORD dwValue;              // @field In or Out Value
} IOCTL_DEVICE_PCIStruct, *pIOCTL_DEVICE_PCIStruct;


//@head3 IOCTL_DEVICE_MEMORY | Command group
//@defitem IOCTL_DEVICE_MEMORY_REGISTER_Set |
//@defitem IOCTL_DEVICE_MEMORY_REGISTER_Get |
//@defitem IOCTL_DEVICE_MEMORY_MapLinearToPhysical |
//@defitem IOCTL_DEVICE_MEMORY_MapPhysicalToLinear |
#define IOCTL_DEVICE_MEMORY                     0x0B40
#define IOCTL_DEVICE_MEMORY_REGISTER_Set        \
CTL_CODE(FILE_DEVICE_VIDEO, ( 1 | IOCTL_DEVICE_MEMORY), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DEVICE_MEMORY_REGISTER_Get        \
CTL_CODE(FILE_DEVICE_VIDEO, ( 2 | IOCTL_DEVICE_MEMORY), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DEVICE_MEMORY_MapLinearToPhysical \
CTL_CODE(FILE_DEVICE_VIDEO, ( 3 | IOCTL_DEVICE_MEMORY), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DEVICE_MEMORY_MapPhysicalToLinear \
CTL_CODE(FILE_DEVICE_VIDEO, ( 4 | IOCTL_DEVICE_MEMORY), METHOD_BUFFERED, FILE_ANY_ACCESS)

// typedefs for IOCTL_DEVICE_MEMORY
// @struct IOCTL_DEVICE_MEMORYStruct | IOCTL_DEVICE_MEMORY structure contains
//         all data related to the command set.
typedef struct 
{
    IN     DWORD   dwDevIndex;          // @field Device Number (Index reference)
    IN     DWORD   dwMEMRegisterOffset; // @field offset of the register to access
    IN OUT DWORD   dwValue;             // @field In or Out Value

    IN OUT ULONG_PTR ulPtrMemoryAddress;// @field Memory address to map <nl>
                                        //        used by MapXXXToXXX()
    IN     DWORD   dwMemorySizeInBytes; // @field Length of Memory <nl>
                                        //        used by MapXXXToXXX()
} IOCTL_DEVICE_MEMORYStruct, *pIOCTL_DEVICE_MEMORYStruct;

#if TEST_OSDEPEND

//@head3 IOCTL_TEST_OSDEPEND | Command group
//@defitem IOCTL_DEVICE_MEMORY_REGISTER_Set |
#define IOCTL_TEST_OSDEPEND                     0x0B80
#define IOCTL_TEST_OSDEPEND_FirstFunction     \
CTL_CODE(FILE_DEVICE_VIDEO, ( 1 | IOCTL_TEST_OSDEPEND), METHOD_BUFFERED, FILE_ANY_ACCESS)

// typedefs for IOCTL_TEST_OSDEPEND
// @struct IOCTL_TEST_OSDEPENDStruct | IOCTL_TEST_OSDEPEND structure contains
//         all data related to the command set.
typedef struct 
{
    IN     DWORD   dwDevIndex;          // @field Device Number (Index reference)
} IOCTL_TEST_OSDEPENDStruct, *pIOCTL_TEST_OSDEPENDStruct;

#endif
