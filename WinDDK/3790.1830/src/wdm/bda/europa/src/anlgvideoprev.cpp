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

#include "34avstrm.h"
#include "VampDevice.h"
#include "AnlgVideoPrev.h"
#include "AnlgVideoPrevInterface.h"
#include "AnlgEventHandler.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgVideoPrev::CAnlgVideoPrev()
{
    m_pVampVDStream         = NULL;
    m_pIKsClock             = NULL;
    m_pSpinLockObj          = NULL;
    m_pKSPin                = NULL;
    m_dwPictureNumber       = 0;
    m_dwCurrentFormatSize   = 0;
    memset(&m_VideoInfoHeader,  0, sizeof(m_VideoInfoHeader));
    memset(&m_VideoInfoHeader2, 0, sizeof(m_VideoInfoHeader2));
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgVideoPrev::~CAnlgVideoPrev()
{
    if( m_pVampVDStream )
    {
        delete m_pVampVDStream;
        m_pVampVDStream = NULL;
    }
    if( m_pIKsClock )
    {
        if( m_pIKsClock->Release() != 0 )
        {
            _DbgPrintF(DEBUGLVL_BLAB,
                ("Info: m_pIKsClock instances not zero"));
        }
        m_pIKsClock = NULL;
    }
    if( m_pSpinLockObj )
    {
        delete m_pSpinLockObj;
        m_pSpinLockObj = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates the Video preview stream object.
//  With the hardware interface (retrieved from the system,
//  stored in the Context of KSDevice by Add(), after DriverEntry()),
//  the video stream is created. This creation (every access to data)
//  is managed in a synchronized way.
//  HAL access is performed by this function.
//
// Return Value:
//  STATUS_UNSUCCESSFUL             Operation failed,
//                                  (a) the function parameters are zero,
//                                  (b) hardware interface access failed
//  STATUS_INSUFFICIENT_RESOURCES   Operation failed,
//                                  (a) synchronization failed
//                                  (b) video preview stream creation failed
//  STATUS_SUCCESS                  Creates the Video stream object
//                                  with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgVideoPrev::Create
(
    IN PKSPIN pKSPin,   // Used for system support function calls.
    IN PIRP   pIrp      // Pointer to IRP for this request. (unused)
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgVideoPrev::Create"));

    //parameters valid?
    if( !pKSPin )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //set member variables
    m_pKSPin              = pKSPin;
    m_dwCurrentFormatSize = 0;

    //get device resources object from m_pKSPin
    CVampDeviceResources* pDevResObj = GetDeviceResource(m_pKSPin);
    if( !pDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }
    else
    {
        // if pointer is valid check pointer pointed to by pDevResObj
        if( !(pDevResObj->m_pFactory) )
        {
            _DbgPrintF( DEBUGLVL_ERROR,
                        ("Error: Cannot not get factory object"));
            return STATUS_UNSUCCESSFUL;
        }
    }
    // create spin lock object
    m_pSpinLockObj = pDevResObj->m_pFactory->CreateSpinLock(NON_PAGED_POOL);
    if( !m_pSpinLockObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: CreateSpinLock failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    m_pVampVDStream = new (NON_PAGED_POOL) CVampVideoStream(pDevResObj);
    if( !m_pVampVDStream )
    {
        delete m_pSpinLockObj;
        m_pSpinLockObj = NULL;
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: CVampVideoStream failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        if( m_pVampVDStream->GetObjectStatus() != STATUS_OK )
        {
            delete m_pVampVDStream;
            m_pVampVDStream = NULL;
            delete m_pSpinLockObj;
            m_pSpinLockObj = NULL;
            _DbgPrintF( DEBUGLVL_ERROR,
                        ("Error: Invalid CVampVideoStream object status"));
            return STATUS_UNSUCCESSFUL;
        }
    }
    // every thing was just fine so we should not forget to set the
    // extended stream header size
    pKSPin->StreamHeaderSize =
        sizeof(KSSTREAM_HEADER) + sizeof(KS_FRAME_INFO);
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Remove pin dispatch function. This function cleans up all, during
//  Create() allocated resources.
//  If existing, the video preview stream and the synchronizing
//  functionality are not used any longer.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) the function parameters are zero,
//                          (b) hardware interface access failed
//                          (c) no video stream object exists
//                          (d) no synchronization object exists
//  STATUS_SUCCESS          Remove pin dispatch function with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgVideoPrev::Remove
(
    IN PIRP pIrp        //Pointer to IRP for this request. (unused)
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgVideoPrev::Remove"));

    if( m_pVampVDStream )
    {
        delete m_pVampVDStream;
        m_pVampVDStream = NULL;
    }

    if( m_pSpinLockObj )
    {
        delete m_pSpinLockObj;
        m_pSpinLockObj = NULL;
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Opens the video stream object.
//  The 'open' is executed on the video preview stream object.
//  Depending on the intersected format (retrieved formerly in SetDataFormat)
//  the format is set accordingly.
//  Depending on the standard (50Hz/625lines or 60Hz/525lines)
//  detected by the hardware,
//  the Video input rectangle and the frame rate
//  are set accordingly.
//  If the Video Output Size extents half resolution the 'interleaved'
//  video mode is used.
//  HAL access is performed by this function.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) the function parameters are zero,
//                          (b) hardware interface access failed
//                          (c) no video stream object exists
//  STATUS_SUCCESS          Opened the video stream object with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgVideoPrev::Open
(
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgVideoPrev::Open"));

    //member variables valid?
    if( !m_pKSPin || !m_pVampVDStream )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    //get device resources object from pKSPin
    CVampDeviceResources* pDevResObj = GetDeviceResource(m_pKSPin);
    if( !pDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }

    DWORD dwCompression = 0;
    LONG  lWidth        = 0;
    LONG  lHeight       = 0;
    WORD  wBitCount     = 0;

    if( IsEqualGUID(m_pKSPin->ConnectionFormat->Specifier,
        KSDATAFORMAT_SPECIFIER_VIDEOINFO) )
    {
        m_dwCurrentFormatSize = sizeof(KS_DATAFORMAT_VIDEOINFOHEADER);
        PKS_DATAFORMAT_VIDEOINFOHEADER pConnectionFormat =  reinterpret_cast
            <PKS_DATAFORMAT_VIDEOINFOHEADER>(m_pKSPin->ConnectionFormat);
        //It is not necessary to check again because the
        //pKSPin->ConnectionFormat is already checked before and the
        //reinterpret can not return zero.

        m_VideoInfoHeader = pConnectionFormat->VideoInfoHeader;
        //not ((PKS_DATARANGE_VIDEO)DataRange)->VideoInfoHeader; !!!
        //It is the wrong structure (contains the default format instead
        //of the intersected new one), BUG in the 3590 DDK...
    }
    else if( IsEqualGUID(m_pKSPin->ConnectionFormat->Specifier,
             KSDATAFORMAT_SPECIFIER_VIDEOINFO2) )
    {
        m_dwCurrentFormatSize = sizeof(KS_DATAFORMAT_VIDEOINFOHEADER2);
        PKS_DATAFORMAT_VIDEOINFOHEADER2 pConnectionFormat2 = reinterpret_cast
            <PKS_DATAFORMAT_VIDEOINFOHEADER2> (m_pKSPin->ConnectionFormat);
        //It is not necessary to check again because the
        //pKSPin->ConnectionFormat is already checked before and the
        //reinterpret can not return zero.

        m_VideoInfoHeader2 = pConnectionFormat2->VideoInfoHeader2;
        //not ((PKS_DATARANGE_VIDEO)DataRange)->VideoInfoHeader; !!!
        //It is the wrong structure (contains the default format instead
        //of the insected new one), BUG in the 3590 DDK...
    }
    else
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid format size detected"));
        return STATUS_UNSUCCESSFUL;
    }

    switch( m_dwCurrentFormatSize )
    {
    case sizeof(KS_DATAFORMAT_VIDEOINFOHEADER):
        dwCompression = m_VideoInfoHeader.bmiHeader.biCompression;
        lWidth        = m_VideoInfoHeader.bmiHeader.biWidth;
        lHeight       = m_VideoInfoHeader.bmiHeader.biHeight;
        wBitCount     = m_VideoInfoHeader.bmiHeader.biBitCount;
        break;
    case sizeof(KS_DATAFORMAT_VIDEOINFOHEADER2):
        dwCompression = m_VideoInfoHeader2.bmiHeader.biCompression;
        lWidth        = m_VideoInfoHeader2.bmiHeader.biWidth;
        lHeight       = m_VideoInfoHeader2.bmiHeader.biHeight;
        wBitCount     = m_VideoInfoHeader2.bmiHeader.biBitCount;
        break;
    default:
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid video info header"));
        return STATUS_UNSUCCESSFUL;
    }

    tStreamParms tStreamParams;

    tStreamParams.pCallbackFktn = dynamic_cast <CCallOnDPC*> (this);
    tStreamParams.pCBParam1     = m_pKSPin; // give Pin to Callback routine
    tStreamParams.pCBParam2     = NULL;
    tStreamParams.pTimeStamp    = NULL;
    tStreamParams.u.tVStreamParms.nOutputType = DMA;

    tStreamParams.u.tVStreamParms.nVideoStandard = AUTOSTD;

    // set interlaced filed as default
    tStreamParams.nField = INTERLACED_FIELD;
    tStreamParams.u.tVStreamParms.dwStreamFlags = 0;

    tStreamParams.u.tVStreamParms.dwOutputSizeX = static_cast
                                                  <DWORD> (lWidth);
    tStreamParams.u.tVStreamParms.dwOutputSizeY = static_cast
                                                  <DWORD> (lHeight);

    //50Hz/625lines standard detected
    if( pDevResObj->m_pDecoder->Is50Hz() )
    {
        tStreamParams.u.tVStreamParms.tVideoInputRect.nStartX =
            PAL_FRAME_RESOLUTION_X;
        tStreamParams.u.tVStreamParms.tVideoInputRect.nStartY =
            PAL_FRAME_RESOLUTION_Y;
        tStreamParams.u.tVStreamParms.tVideoInputRect.nWidth  =
            PAL_FRAME_RESOLUTION_WIDTH;
        tStreamParams.u.tVStreamParms.tVideoInputRect.nHeight =
            PAL_FRAME_RESOLUTION_HEIGHT;
        tStreamParams.u.tVStreamParms.u.FrameRateAndFraction.wFrameRate    =
            PAL_FRAMERATE;
        tStreamParams.u.tVStreamParms.u.FrameRateAndFraction.wFractionPart =
            PAL_FRACTION;

        //set stream properties dependant to the size of the video
        if( tStreamParams.u.tVStreamParms.dwOutputSizeY
            <= PAL_FRAME_RESOLUTION_HEIGHT )
        {
            tStreamParams.nField = EVEN_FIELD;
            tStreamParams.u.tVStreamParms.dwStreamFlags =
                                                    S_ODD_EVEN_DONT_CARE;
        }
    }
    else
    {   //60Hz/525lines standard detected
        tStreamParams.u.tVStreamParms.tVideoInputRect.nStartX =
            NTSC_FRAME_RESOLUTION_X;
        tStreamParams.u.tVStreamParms.tVideoInputRect.nStartY =
            NTSC_FRAME_RESOLUTION_Y;
        tStreamParams.u.tVStreamParms.tVideoInputRect.nWidth  =
            NTSC_FRAME_RESOLUTION_WIDTH;
        tStreamParams.u.tVStreamParms.tVideoInputRect.nHeight =
            NTSC_FRAME_RESOLUTION_HEIGHT;
        tStreamParams.u.tVStreamParms.u.FrameRateAndFraction.wFrameRate    =
            NTSC_FRAMERATE;
        tStreamParams.u.tVStreamParms.u.FrameRateAndFraction.wFractionPart =
            NTSC_FRACTION;

        //set stream properties dependant to the size of the video
        if( tStreamParams.u.tVStreamParms.dwOutputSizeY
            <= NTSC_FRAME_RESOLUTION_HEIGHT )
        {
            tStreamParams.nField = EVEN_FIELD;
            tStreamParams.u.tVStreamParms.dwStreamFlags =
                                                    S_ODD_EVEN_DONT_CARE;
        }
    }
    if( dwCompression )
    {
        //YUY2 format choosen
        tStreamParams.u.tVStreamParms.nFormat = YUY2;
    }
    else
    {
        //RGB format choosen
        switch( wBitCount )
        {
            case BIT_COUNT_32:
                tStreamParams.u.tVStreamParms.nFormat = RGB888a;
                break;
            case BIT_COUNT_24:
                tStreamParams.u.tVStreamParms.nFormat = RGB888;
                break;
            case BIT_COUNT_16:
            tStreamParams.u.tVStreamParms.nFormat = RGBa555;
                break;
            default:
                tStreamParams.u.tVStreamParms.nFormat = RGB888;
                break;
        }
    }

    //open
    if( m_pVampVDStream->Open(&tStreamParams) != VAMPRET_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Open video preview stream failed"));
        return STATUS_UNSUCCESSFUL;
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Closes the video preview stream object.
//  The 'close' is executed on the video preview stream object.
//  All stream related resources (vamp buffers) have to be released.
//  This release procedure has to be synchronized.
//  HAL access is performed by this function.
//
//  Return Value:
//  STATUS_UNSUCCESSFUL         Operation failed,
//                              (a) the function parameters are zero,
//                              (b) no video preview stream object exists
//                              (c) release vamp buffers failed
//  STATUS_SUCCESS              Closed the video stream object with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgVideoPrev::Close
(
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgVideoPrev::Close"));

    CVampBuffer* pVampBuffer        = NULL;

    if( !m_pVampVDStream || !m_pSpinLockObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    //aquire spin lock to make sure that we are the only one
    //to access the buffer queue at the moment
    m_pSpinLockObj->AcquireSpinLock(); //void function
    //cancel all buffer that are still in the queue
    if( m_pVampVDStream->CancelAllBuffers() != VAMPRET_SUCCESS )
    {
        //CancelAllBuffers failed
        m_pSpinLockObj->ReleaseSpinLock(); // void function
        _DbgPrintF(DEBUGLVL_ERROR,("Error: CancelAllBuffers failed"));
        return STATUS_UNSUCCESSFUL;
    }
    //Get all the buffer from the Vamp HAL
    while( m_pVampVDStream->GetNextDoneBuffer(&pVampBuffer) ==
            VAMPRET_SUCCESS )
    {
        if( !pVampBuffer )
        {
            m_pSpinLockObj->ReleaseSpinLock(); // void function
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid buffer pointer"));
            return STATUS_UNSUCCESSFUL;
        }

        //get KS stream pointer from the Vamp buffer object
        PKSSTREAM_POINTER pStreamPointer =
            static_cast <PKSSTREAM_POINTER> (pVampBuffer->GetContext());
        if( pStreamPointer )
        {
            //release system buffer
            KsStreamPointerDelete(pStreamPointer); //void function
            pStreamPointer = NULL;
            //remove Vamp buffer object
            delete pVampBuffer;
            pVampBuffer = NULL;
        }
        else
        {
            //The streampointer within the buffer context is corrupted
            m_pSpinLockObj->ReleaseSpinLock(); // void function
            //remove at least Vamp buffer object
            delete pVampBuffer;
            pVampBuffer = NULL;
            _DbgPrintF( DEBUGLVL_ERROR,
                        ("Error: (Buffer-)Stream pointer invalid!"));
            return STATUS_UNSUCCESSFUL;
        }
    }
    //release spin lock to give others access to the buffer queue again
    m_pSpinLockObj->ReleaseSpinLock(); //void function
    //close Capture stream
    if( m_pVampVDStream->Close() != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Close capture stream failed"));
        return STATUS_UNSUCCESSFUL;
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Starts the video preview stream.
//  The 'start' is executed on the video preview stream object.
//  First the Vamp buffers (NUM_VD_PREV_STREAM_BUFFER) are created
//  (using system buffer information/memory) and added to the stream.
//  Second the reference clock object is created here.
//  (The pin must have received notification of the master clock.)
//  The reference object is used to 'timestamp' the filled buffers.
//  Thirdly the stream is started.
//  HAL access is performed by this function.
//
//  Return Value:
//  STATUS_UNSUCCESSFUL         Operation failed,
//                              (a) the function parameters are zero,
//                              (b) no video stream object exists
//                              (c) create reference clock object failed
//  STATUS_SUCCESS              Started the video preview stream object
//                              with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgVideoPrev::Start
(
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgVideoPrev::Start"));

    //member variables valid?
    if( !m_pKSPin || !m_pVampVDStream )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
    DWORD dwIndex = 0;

    do //while system buffers available
    {
        ntStatus = CAnlgVideoPrev::Process();
        dwIndex++;
    }
    while( ntStatus == STATUS_SUCCESS );

    //check, if the number of inserted buffer is equal to
    //formerly demanded buffers
    if( dwIndex != NUM_VD_PREV_STREAM_BUFFER )
    {
        // This is not a bug but there is a Microsoft restriction
        // where sometimes we dont get all the requested buffers.
        _DbgPrintF( DEBUGLVL_VERBOSE,
                    ("Warning: Number of buffers do not match!"));
    }
    // master clock is retrieved from the system
    if( !m_pIKsClock )
    {
        if( !NT_SUCCESS(KsPinGetReferenceClockInterface(m_pKSPin,
                                                        &m_pIKsClock)) )
        {
            // If we could not get an interface to the clock,
            // don't start the stream
            m_pIKsClock = NULL;
            _DbgPrintF(DEBUGLVL_ERROR,("Error: No clock object available"));
            return STATUS_UNSUCCESSFUL;
        }
    }
    if( m_pVampVDStream->Start() != VAMPRET_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Start video preview stream failed"));
        return STATUS_UNSUCCESSFUL;
    }
    //reset picture counter
    m_dwPictureNumber = 0;
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Stops the video preview stream.
//  The 'stop' is executed on the video preview stream object.
//  This function cleans up all, during Start() allocated resources.
//  HAL access is performed by this function.
//
//  Return Value:
//  STATUS_UNSUCCESSFUL         Operation failed,
//                              (a) the function parameters are zero,
//                              (b) no video preview stream object exists
//                              (c) no reference clock object exists
//  STATUS_SUCCESS              Stopped the video preview stream object
//                              with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgVideoPrev::Stop
(
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgVideoPrev::Stop"));

    //parameters valid?
    if( !m_pVampVDStream )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }
	VAMPRET vrResult = m_pVampVDStream->Stop(FALSE);
    // release the reference to the master clock
    // which was created during start
    if( m_pIKsClock )
    {
        if( m_pIKsClock->Release() != 0 )
        {
            _DbgPrintF(DEBUGLVL_BLAB,
				("Info: m_pIKsClock instances not zero"));
        }
        m_pIKsClock = NULL;
    }
	// check whether the stop function was successful or not
    if( vrResult != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
			("Error: Stop video preview stream failed"));
        return STATUS_UNSUCCESSFUL;
    }
    return STATUS_SUCCESS;
}



//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the callback function that is called after an interrupt in the
//  systems DPC level.
//  Get the next filled buffer from the hardware (in a synchronized way),
//  'timestamp' the buffer.
//  It attempts to initiate processing on Pin by sending a processing
//  dispatch call to Pin's processing object.
//  (asynchronous call to the Process function)
//  HAL access is performed by this function.
//
// Return Value:
//  This function is registered and called from within the system,
//  respectivly VAMP RETURN values are used.
//  VAMPRET_NULL_POINTER_PARAMETER
//                              Operation failed
//                              (a) if pParam1 is NULL
//  VAMPRET_GENERAL_ERROR       Operation failed
//                              (a) Retrieved buffer pointer is NULL
//                              (b) Buffer address (stored in buffer context)
//                                  is NULL
//  VAMPRET_SUCCESS             Operation with success
//                              (a) if no more buffers are in the done queue
//
//////////////////////////////////////////////////////////////////////////////
VAMPRET CAnlgVideoPrev::CallOnDPC
(
    PVOID pStream,  //Pointer to stream object. (unused)
    PVOID pParam1,  //User defined parameter from the open function.
                    //Contains the associated KS pin in this case.
    PVOID pParam2   //User defined parameter from the open function.
                    //(unused)
)
{
    //parameter valid?
    if( !pParam1 )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return VAMPRET_NULL_POINTER_PARAMETER;
    }

    CVampBuffer*        pVampBuffer     = NULL;
    PKSSTREAM_POINTER   pStreamPointer  = NULL;

    if( !m_pVampVDStream || !m_pIKsClock || !m_pSpinLockObj)
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return VAMPRET_GENERAL_ERROR;
    }

    m_pSpinLockObj->AcquireSpinLock(); //void function

    //get all done vamp buffer out of our queue
    if( m_pVampVDStream->GetNextDoneBuffer(&pVampBuffer) ==
            VAMPRET_SUCCESS )
    {
        m_pSpinLockObj->ReleaseSpinLock(); //void function

        if( !pVampBuffer )
        {
            _DbgPrintF( DEBUGLVL_ERROR,
                        ("Error: Invalid buffer (invalid buffer pointer)"));
            return VAMPRET_GENERAL_ERROR;
        }
        //get KS stream pointer out of the vampbuffer context
        pStreamPointer =
                static_cast <PKSSTREAM_POINTER> (pVampBuffer->GetContext());
        if( !pStreamPointer )
        {
            _DbgPrintF( DEBUGLVL_ERROR,
                        ("Error: Invalid buffer (invalid streampointer)"));
            return VAMPRET_GENERAL_ERROR;
        }

        //set data context information
        switch( m_dwCurrentFormatSize )
        {
        case sizeof(KS_DATAFORMAT_VIDEOINFOHEADER):
            //number of bytes within the frame that are valid
            pStreamPointer->StreamHeader->DataUsed =
                                    m_VideoInfoHeader.bmiHeader.biSizeImage;
            //contains the duration of this stream segment
            pStreamPointer->StreamHeader->Duration =
                                    m_VideoInfoHeader.AvgTimePerFrame;
            break;
        case sizeof(KS_DATAFORMAT_VIDEOINFOHEADER2):
            //number of bytes within the frame that are valid
            pStreamPointer->StreamHeader->DataUsed =
                                    m_VideoInfoHeader2.bmiHeader.biSizeImage;
            //contains the duration of this stream segment
            pStreamPointer->StreamHeader->Duration =
                                    m_VideoInfoHeader2.AvgTimePerFrame;
            break;
        default:
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid format size"));
            return STATUS_UNSUCCESSFUL;
        }

        //the attributes of the data stream
        pStreamPointer->StreamHeader->OptionsFlags =
                KSSTREAM_HEADER_OPTIONSF_TIMEVALID   |
                KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;
        //the presentation time for the related stream buffer
        pStreamPointer->StreamHeader->PresentationTime.Time =
                m_pIKsClock->GetTime();
        pStreamPointer->StreamHeader->PresentationTime.Numerator = 1;
        pStreamPointer->StreamHeader->PresentationTime.Denominator = 1;

        //there might be an additional structure for the frame information
        PKS_FRAME_INFO pKSFrameInfoHeader =
            reinterpret_cast <PKS_FRAME_INFO> (pStreamPointer->StreamHeader+1);
        if( pKSFrameInfoHeader )
        {
            //additional header available?
            if( pKSFrameInfoHeader->ExtendedHeaderSize == sizeof(KS_FRAME_INFO) )
            {
                //this is a complete frame
                pKSFrameInfoHeader->dwFrameFlags = KS_VIDEO_FLAG_FRAME;
                //set picture number (reseted during start)
                pKSFrameInfoHeader->PictureNumber = ++m_dwPictureNumber;

                pKSFrameInfoHeader->DropCount = 0;
            }
        }
        //give buffer back to the system
        KsStreamPointerDelete(pStreamPointer); //void function
        pStreamPointer = NULL;
        //delete Vamp buffer object
        delete pVampBuffer;
        pVampBuffer = NULL;
        //Make an asynchronous system call to the process function to insert
        //a new buffer into the queue. We cannot call Process here directly,
        //because we are still in the systems DPC context.
        KsPinAttemptProcessing(static_cast <PKSPIN> (pParam1), TRUE);

        m_pSpinLockObj->AcquireSpinLock(); // void function
    }

    m_pSpinLockObj->ReleaseSpinLock(); //void function

    return VAMPRET_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  It attempts to initiate processing on a Pin.
//  A Vamp buffer is created (using system buffer informations/memory)
//  and added to the stream.
//  HAL access is performed by this function.
//
//  Return Value:
//  STATUS_UNSUCCESSFUL         Operation failed,
//                              (a) the function parameters are zero,
//                              (b) no 'next available system buffer'
//                              (c) couldn't clone the system buffer
//                              (d) create Vamp buffer object failed
//                              (e) couldn't add buffer to the stream
//  STATUS_SUCCESS              Process with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgVideoPrev::Process
(
)
{
    if( !m_pSpinLockObj || !m_pVampVDStream || !m_pKSPin )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

    DWORD dwCompression = 0;
    LONG  lWidth        = 0;
    LONG  lHeight       = 0;
    WORD  wBitCount     = 0;

    switch( m_dwCurrentFormatSize )
    {
    case sizeof(KS_DATAFORMAT_VIDEOINFOHEADER):
        dwCompression   = m_VideoInfoHeader.bmiHeader.biCompression;
        lWidth          = m_VideoInfoHeader.bmiHeader.biWidth;
        lHeight         = m_VideoInfoHeader.bmiHeader.biHeight;
        wBitCount       = m_VideoInfoHeader.bmiHeader.biBitCount;
        break;
    case sizeof(KS_DATAFORMAT_VIDEOINFOHEADER2):
        dwCompression   = m_VideoInfoHeader2.bmiHeader.biCompression;
        lWidth          = m_VideoInfoHeader2.bmiHeader.biWidth;
        lHeight         = m_VideoInfoHeader2.bmiHeader.biHeight;
        wBitCount       = m_VideoInfoHeader2.bmiHeader.biBitCount;
        break;
    default:
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid video info header"));
        return STATUS_UNSUCCESSFUL;
    }

    //get next available system buffer
    PKSSTREAM_POINTER pStreamPointer = KsPinGetLeadingEdgeStreamPointer(
                                        m_pKSPin,
                                        KSSTREAM_POINTER_STATE_LOCKED);
    if( !pStreamPointer )
    {
        //no system buffer available
        //This case can happen if it is the last pointer in the queue or
        //the system cannot give us the required buffer
        _DbgPrintF(DEBUGLVL_VERBOSE,("Warning: No system buffer available"));
        return STATUS_UNSUCCESSFUL;
    }

    PKSSTREAM_POINTER pStreamPointerClone = NULL;
    CVampBuffer* pVampBuffer = NULL;

    //clone the system buffer
    ntStatus = KsStreamPointerClone(pStreamPointer,
                                    NULL,
                                    NULL,
                                    &pStreamPointerClone);
    if( !NT_SUCCESS(ntStatus) )
    {
        //no system buffer available
        _DbgPrintF(DEBUGLVL_ERROR,(
                "Error: Streampointer cloning unsuccessful"));
        return STATUS_UNSUCCESSFUL;
    }
    //set vamp buffer format parameters from video info header information
    tBufferFormat tBufferParams;

    //set samples per line
    tBufferParams.dwSamplesPerLine = static_cast <DWORD> (ABS(lWidth));

    //set channels per sample (used for audio only, otherwise always 1)
    tBufferParams.dwChannelsPerSample = 1;

    //set bits per sample
    tBufferParams.dwBitsPerSample = static_cast<DWORD> (wBitCount);

    //set number of lines
    tBufferParams.dwNrOfLines = static_cast <DWORD> (ABS(lHeight));

    //calculate pitch in bytes
    tBufferParams.lPitchInBytes = static_cast <LONG>
        (tBufferParams.dwSamplesPerLine * tBufferParams.dwBitsPerSample/8);

    //calculate the buffer length
    tBufferParams.lBufferLength = tBufferParams.lPitchInBytes * static_cast
        <LONG> (tBufferParams.dwNrOfLines);

    //dont change the byte order
    tBufferParams.ByteOrder = KEEP_AS_IS;

    //set buffer type to queued
    tBufferParams.BufferType = CVAMP_BUFFER_QUEUED;

    //is the buffer size correct?
    if( pStreamPointerClone->StreamHeader->FrameExtent <
            static_cast <DWORD> (tBufferParams.lBufferLength) )
    {
        //give buffer back to the system, we cannot use it for some reason
        KsStreamPointerDelete(pStreamPointerClone); //void function
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Buffer size incorrect"));
        return STATUS_UNSUCCESSFUL;
    }
    //do we have a RGB format?
    if( dwCompression == 0 )
    {
        tBufferParams.lPitchInBytes *= -1;//RGB formats need negative pitch
    }
    //construct vamp buffer object
    pVampBuffer =
        new (NON_PAGED_POOL) CVampBuffer(
                            pStreamPointerClone->StreamHeader->Data,
                            pStreamPointerClone->StreamHeader->FrameExtent,
                            (PVOID) pStreamPointerClone->Offset,
                            &tBufferParams,
                            BUFFER_REGULAR_MAPPING_OS,
                            false,
                            false);
    //buffer construction successful?
    if( !pVampBuffer )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Not enough memory"));
        KsStreamPointerDelete(pStreamPointerClone); //void function
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    if( pVampBuffer->GetObjectStatus() != STATUS_OK )
    {
        //de-allocate memory for the Vamp buffer object
        delete pVampBuffer;
        pVampBuffer = NULL;
        //give buffer back to the system, we cannot use it for some reason
        KsStreamPointerDelete(pStreamPointerClone); //void function
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Buffer Object status failed"));
        return STATUS_UNSUCCESSFUL;
    }
    //set KS stream pointer into the buffer context for use in the
    //CallOnDPC function
    pVampBuffer->SetContext(static_cast <PVOID> (pStreamPointerClone));
    //aquire spin lock to be the only one that has access to the buffer queue
    m_pSpinLockObj->AcquireSpinLock(); //void function
    //add buffer to Vamp buffer queue
    if( m_pVampVDStream->AddBuffer(pVampBuffer) != VAMPRET_SUCCESS )
    {
        //de-allocate memory for the Vamp buffer object
        delete pVampBuffer;
        pVampBuffer = NULL;
        //give buffer back to the system, we cannot use it for some reason
        KsStreamPointerDelete(pStreamPointerClone); //void function
        m_pSpinLockObj->ReleaseSpinLock(); //void function
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: AddBuffer to preview stream failed"));
        return STATUS_UNSUCCESSFUL;
    }
    //release the spin lock to give others access to the buffer queue again
    m_pSpinLockObj->ReleaseSpinLock(); //void function
    //advance stream pointer to the next available system buffer
    ntStatus = KsStreamPointerAdvance(pStreamPointer);

    if( (ntStatus != STATUS_DEVICE_NOT_READY) &&
        (ntStatus != STATUS_SUCCESS) )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Video preview Streampointer advacement failed"));
    }

	//do not de-allocate memory for the Vamp buffer object
	//it's allocated by HAL and pointer is given to HAL
    pVampBuffer = NULL;

    return ntStatus;
}

