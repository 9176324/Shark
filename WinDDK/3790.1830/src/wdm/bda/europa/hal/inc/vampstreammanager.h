//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 1999
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
// @doc      INTERNAL EXTERNAL
// @module   VampStreamManager | This class is the main manager for the
//           hardware streaming resources, which are the FIFO RAM, the DMA
//           register sets, the video tasks and GPIO input and output. It
//           inherits also the IRQ classes. Furthermore it contains the video
//           task management, so we have one static control instance for all
//           possible streams. It handles five different streams: video streams
//           task A, video streams task B, VBI streams, audio streams and
//           transport streams. If the video stream is subdivided into two
//           different tasks, the VBI stream will be also subdivided. However,
//           it makes no sense to have a divided VBI stream because you need
//           continuous video fields to get the entire VBI information. So it
//           handles the two VBI tasks as one stream with two DMA register
//           sets. The manager allows opening several instances of a dedicated
//           stream, but it allows only the starting of a stream if the
//           resources are not blocked. For example, a single
//           packed video stream occupies only FIFO 0, whereby a planar video
//           stream occupies from FIFO 0 to 2, so the transport stream is
//           blocked (or vice versa, the transport stream can block the YUV
//           data in planar mode). If a second video task wants to be
//           attached, the stream manager checks if it can start the task and
//           allows the access to the CVampCoreStream object. Furthermore a
//           VSB stream can block the video tasks. Moreover, the abstraction
//           allows us to hide the partitioning of the FIFO dependent on the
//           incoming streams. So resizing the FIFO RAM area at runtime might
//           be possible in some cases. The stream manager will allow streams
//           to be created (CreateCoreStream), so it is possible to build
//           video buffer lists and to store streaming attributes. There is no
//           DMA channel allocated at this moment. There is a check for
//           resource conflicts when the streams are requested
//           (RequestStream). If none, the streams could do all the register
//           settings, before the stream manager will start the stream
//           (EnableTransfer). So streams could be swapped very quickly from
//           the background to a running state and they are allowed to be
//           initialized and prepared before switching them in, which helps in
//           support of Microsoft's new resource management API.
// @end
// Filename: VampStreamManager.h
//
// Routines/Functions:
// public:
//  CVampStreamManager
//  virtual ~CVampStreamManager();
//  CreateCoreStream
//  OpenVideoStream
//  OpenVideoStreamPlanar
//  OpenVBIStream
//  OpenAudioStream
//  OpenTransportStream
//  ReleaseStream
//  RequestStream
//  EnableTransfer
//  DisableTransfer
//  ReprogramSkipParams
//  CalculateSkipParams
//  ReleaseCoreStream
//
//  GetObjectStatus
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_VAMPSTREAMMANAGER_H__566E1F00_8184_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPSTREAMMANAGER_H__566E1F00_8184_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VampError.h"
#include "VampTypes.h"
#include "VampIrq.h"
#include "VampFifoAlloc.h"
#include "VampEventHandler.h"
#include "VampCoreStream.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampStreamManager | This is the control module for streaming devices.
// @base private | CVampFifoAlloc 
// @base public  | CVampEventHandler
// @end 
//
// Stream types:    | Restrictions                                    | DMA register 
//                  | Stream Id's                   |                 | sets
//                  |3|3|2|2|2|2|2|2|2|2|2|2|1|1|1|1|1|1|1|1|1|1|
//                  |1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|
// 0:Video A        | | | | | | | | | | | | | | | |X| | | | | | | | | | | | | | | |X|
// 1:Video B        | | | | | | | | | | | | | | |X| | | | | | | | | | | | | | | |X| |
// 2:VBI A          | | | | | | | | | | | | | |X| | | | | | | | | | | | | | | |X| | |
// 3:VBI B          | | | | | | | | | | | | |X| | | | | | | | | | | | | | | |X| | | |
// 4:Video A planar | | | | | | | | | | | |X| | | | | | | | | | | | | | |X|X| | | |X|
// 5:Video B planar | | | | | | | | | | |X| | | | | | | | | | | | | | | |X|X| | |X| |
// 6:Audio          | | | | | | | | | |X| | | | | | | | | | | | | | | |X| | | | | | |
// 7:               | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
// 8:TS serial      | | | | | | | |X| | | | | | | | | | | | | | | | | | |X| | | | | |
// 9:TS parallel    | | | | | | |X| | | | | | | | | | | | | | | | | |X| |X| | | | | |
// 10:VSB_A/AB      | | | | | |X| | | | | | | | | | | | | | | | | |X|X| | |X|X|X|X|X|
// 11:VSB_B         | | | | |X| | | | | | | | | | | | | | | | | | |X| | | |X|X|X|X|X|
// 12:ITU656        | | | |X| | | | | | | | | | | | | | | | | | | |X| | | | | | | | |
// 13:VIP11         | | |X| | | | | | | | | | | | | | | | | | | | |X| | | | | | | | |
// 14:VIP20         | |X| | | | | | | | | | | | | | | | | | | | | |X| | | | | | | | |
//
//////////////////////////////////////////////////////////////////////////////

class CVampStreamManager : private CVampFifoAlloc, 
                           public  CVampEventHandler
{
	// @access private 
private:
    //@cmember Pointer to device resources object.
    CVampDeviceResources *m_pDevice;
    //@cmember Flags for channels in use.
    DWORD m_dwChannelFlags;
    //@cmember Flags for paused channels.
    DWORD m_dwPauseChannelFlags;
    //@cmember Flag for automatic configuration of ram areas.
    BOOL m_bAutoConfigureRamAreas;
    //@cmember Current ram configuration type.
    BYTE m_ucRamConfiguration;

    //@cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

    //There are four kinds of streams allocating specific FIFOs (audio, 
    //transport, packed video/VBI, planar video) and combinations of them.
    //According the actual situation the partitioning of the FIFOs is done this way:
    //                                                                              <nl>
    //Situation:                        size of FIFO #  1   2   3   4               <nl>
    //                                                                              <nl>
    //RAM_DEFAULT (RAM_AUDIO + RAM_PACKED):             7   0   0   1   * 128 bytes <nl>
    //RAM_AUDIO:                                        0   0   0   8               <nl>
    //RAM_TRANSPORT:                                    0   0   8   0               <nl>
    //RAM_PACKED:                                       8   0   0   0               <nl>
    //RAM_PLANAR:                                       4   2   2   0               <nl>    
    //RAM_AUDIO + RAM_TRANSPORT:                        0   0   7   1               <nl>
    //RAM_AUDIO + RAM_PACKED:                           7   0   0   1               <nl>
    //RAM_TRANSPORT + RAM_PACKED:                       6   0   2   0               <nl>
    //RAM_AUDIO + RAM_TRANSPORT + RAM_PACKED:           5   0   2   1               <nl>
    //RAM_AUDIO + RAM_PLANAR:                           3   2   2   1               <nl>
    //@cmember Method for configuring the FIFOs, called in case of automatic 
    //configuration.<nl>
    //Parameterlist:<nl>
    //DWORD dwChannelFlags // channels to configure<nl>
    VOID ConfigureRamAreas( IN DWORD dwChannelFlags );

    //@cmember This method adapts the FSkip register of a task, when a second
    //task has to be integrated. Therefore the running stream has to
    //be paused and restarted again.<nl>
    //Parameterlist:<nl>
    //DWORD dwChannelRequest // channel to restart<nl>
    //DWORD dwFSkip0 // new fields to skip / process<nl>
    VAMPRET ReprogramSkipParams(
        IN DWORD dwChannelRequest,
        IN DWORD dwFSkip0 );

    //@cmember This method tries to match the skip parmeters from both 
    //video tasks.<nl>
    //Parameterlist:<nl>
    //DWORD *pdwFSkip // requested fields to skip / process<nl>
    //DWORD *pdwFSkip0 // concurrent fields to skip / process<nl>
    //BOOL  bFrameRateReduction // flag for frame rate reduction//<nl>
    VAMPRET CalculateSkipParams(
        IN OUT DWORD *pdwFSkip,
        IN OUT DWORD *pdwFSkip0,
        BOOL bFrameRateReduction );

    // @cmember This object is used to put the current thread or
    // the whole system in a wait state for a given time.<nl>
    COSDependDelayExecution* m_pDelayExecution;

    // @cmember SpinLock object that syncronizes calls to the
    // RequestStream and ReleaseStream methods.
    COSDependSpinLock*   m_pLock;

    // @access public 
public:
	//@cmember Constructor.<nl>
	CVampStreamManager( 
        IN CVampDeviceResources *pDevice );

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

	//@cmember Destructor.<nl>
	virtual ~CVampStreamManager();

	//@cmember Set FIFO default settings.<nl>
    VOID SetFifoDefaults();
	//@cmember Save FIFO settings to registry.<nl>
    VOID SaveSettings();
	//@cmember Get FIFO settings from registry.<nl>
    VOID ReadSettings();

    //@cmember Get the capabilities of the device.<nl>
    DWORD GetCapabilities();                  

    //@cmember Create a core stream object.<nl>
    //Parameterlist:<nl> 
    //CVampCoreStream **ppCoreStream // pointer to vacant stream (optional)<nl>
    //BOOL bPlanar // planar flag (opt.)<nl>
    VAMPRET CreateCoreStream(                         
        IN OUT CVampCoreStream **ppCoreStream,
        IN BOOL bPlanar = FALSE );                           

    //@cmember Assign vacant DmaChannels to stream.<nl>
    //Parameterlist:<nl>
    //DWORD *pdwChannelRequest // flags for requested channel<nl>
    //tStreamManagerInfo *ptStreamManagerInfo // info structure<nl>
    //CVampCoreStream *pCoreStream // pointer to core stream (opt.)<nl>
    VAMPRET RequestStream(                
        IN OUT DWORD *pdwChannelRequest,         
        IN OUT tStreamManagerInfo *ptStreamManagerInfo,
        IN CVampCoreStream *pCoreStream = NULL ); 

    //@cmember This method starts the DMA transfer of a stream.<nl>
    //Parameterlist:<nl> 
    //DWORD dwChannelRequest // channel(s) to be started<nl>
    VAMPRET EnableTransfer(
        IN DWORD dwChannelRequest); 

    //@cmember This method pauses a stream, eg. when the stream runs 
    //out of buffers.<nl>
    // Parameterlist:<nl> 
    // DWORD dwChannelRequest // channel(s) to be paused<nl> 
    VAMPRET Pause(
        IN DWORD dwChannelRequest ); 

    //@cmember This method restarts the paused streams.<nl>
    // Parameterlist:<nl> 
    // DWORD dwChannelRequest // channel(s) to be restarted<nl> 
    VAMPRET Resume(
        IN DWORD dwChannelRequest ); 

    //@cmember Adjusts the VBI input window to the current video standard.<nl>
    // Parameterlist:<nl> 
    // CPage *pPage // pointer to scaler page<nl> 
    VOID FixVbiAcquisitionWindow(
        IN CPage *pPage );

    //@cmember This method sets the scaler pages registers according to active 
    //stream conditions.<nl>
    VAMPRET AdjustScaler();

    //@cmember This method enables the video/VBI source within the scaler 
    //register pages.<nl>
    //Parameterlist:<nl>
    //DWORD dwChannelRequest // channel(s) to be enabled<nl>
    VAMPRET EnableScaler( 
        DWORD dwChannelRequest );

    //@cmember This method disables the video/VBI source within the scaler 
    //register pages.<nl>
    //Parameterlist:<nl>
    //DWORD dwChannelRequest // channel(s) to be disabled<nl>
    VAMPRET DisableScaler( 
        DWORD dwChannelRequest );

    //@cmember This method disables a stream. The resources remain blocked 
    //until a ReleaseStream will be called.<nl>
    //Parameterlist:<nl>
    //DWORD dwChannelRequest // channel(s) to be stopped<nl>
    VAMPRET DisableTransfer(                         
        IN DWORD dwChannelRequest );

    //@cmember Releases DMA channels of a stream.<nl>
    //Parameterlist:<nl>
    //DWORD dwChannelRequest // flags for requested channels<nl>
    //CVampCoreStream *pCoreStream // core stream pointer to be released (opt.)<nl>
    VAMPRET ReleaseStream(                    
        IN DWORD dwChannelRequest,
        IN CVampCoreStream *pCoreStream = NULL ); 

    //@cmember This method deletes the CVampCoreStream object.<nl>
    //Parameterlist:<nl>
    //CVampCoreStream **ppCoreStream // core stream pointer to be released (opt.)<nl>
    VAMPRET ReleaseCoreStream(
        IN CVampCoreStream **ppCoreStream ); 

    //@cmember Add core stream callbacks to interrupt objects.<nl>
    //Parameterlist:<nl>
    //DWORD dwChannelRequest // flags for requested channels<nl>
    //eFieldType nFieldType // field type (EITHER for VBI)<nl>
    //DWORD dwFSkip // field skipping parameter<nl>
    //CVampCoreStream *pCoreStream // core stream pointer<nl>
    VAMPRET EnableInterrupts ( 
        IN DWORD dwChannelRequest,           
        IN eFieldType nFieldType,            
        IN DWORD dwFSkip,  
        IN CVampCoreStream *pCoreStream );

    //@cmember Get the actual MacroVision protection level.<nl>
    eMacroVisionMode GetMacroVisionMode()
    {
        return m_MVMode;
    }
};

#endif // !defined(AFX_VAMPSTREAMMANAGER_H__566E1F00_8184_11D3_A66F_00E01898355B__INCLUDED_)
