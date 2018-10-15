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
#include "AnlgVbiOut.h"
#include "AnlgVbiOutInterface.h"
#include "AnlgEventHandler.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgVBIOut::CAnlgVBIOut()
{
    m_pVampVBIStream    = NULL;
    m_pIKsClock         = NULL;
    m_pSpinLockObj      = NULL;
    m_pKSPin            = NULL;
    m_dwPictureNumber   = 0;
    m_bNewTunerInfo     = false;
    m_dwLostBuffers     = 0;
    memset(&m_TVTunerChangeInfo, 0, sizeof(m_TVTunerChangeInfo));
    memset(&m_VBIInfoHeader, 0, sizeof(m_VBIInfoHeader));
    m_bVBIInfoChanged   = FALSE;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgVBIOut::~CAnlgVBIOut()
{
    // make sure that all created objects are released
    // just for an error case to be safe
    memset(&m_TVTunerChangeInfo, 0, sizeof(m_TVTunerChangeInfo));
    memset(&m_VBIInfoHeader, 0, sizeof(m_VBIInfoHeader));
    m_bNewTunerInfo     = false;

    if( m_pVampVBIStream )
    {
        delete m_pVampVBIStream;
        m_pVampVBIStream = NULL;
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

    m_dwPictureNumber   = 0;
    m_dwLostBuffers     = 0;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates the VBI stream object.
//  With the hardware interface (retrieved from the system,
//  stored in the Context of KSDevice by Add(), after DriverEntry()),
//  the VBI stream is created. This creation (every access to data)
//  is managed in a synchronized way.
//  HAL access is performed by this function.
//
// Return Value:
//  STATUS_SUCCESS                  VBI stream object created with success.
//  STATUS_UNSUCCESSFUL             Operation failed,
//                                  (a) the function parameters are zero,
//                                  (b) hardware interface access failed
//  STATUS_INSUFFICIENT_RESOURCES   Operation failed,
//                                  (a) synchronization failed
//                                  (b) VBI stream creation failed
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgVBIOut::Create
(
    IN PKSPIN pKSPin,   // Used for system support function calls.
    IN PIRP   pIrp      // Pointer to IRP for this request. (unused)
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgVBIOut::Create"));

    // Check connection format pointer
    if( !pKSPin->ConnectionFormat )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid connection format"));
        return STATUS_UNSUCCESSFUL;
    }

    // Check format size
    if( pKSPin->ConnectionFormat->FormatSize !=
            sizeof(KS_DATAFORMAT_VBIINFOHEADER) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid format size"));
        return STATUS_UNSUCCESSFUL;
    }

    // get the connection format out of the KS pin
    PKS_DATAFORMAT_VBIINFOHEADER pConnectionFormat =
        reinterpret_cast<PKS_DATAFORMAT_VBIINFOHEADER>(pKSPin->ConnectionFormat);

    // Set member variable
    m_pKSPin        = pKSPin;
    m_VBIInfoHeader = pConnectionFormat->VBIInfoHeader;

    // get device resources object from pKSPin
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
    // create the VBI stream object
    m_pVampVBIStream = new (NON_PAGED_POOL) CVampVbiStream(pDevResObj);
    if( !m_pVampVBIStream )
    {
        delete m_pSpinLockObj;
        m_pSpinLockObj = NULL;
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: CVampVBIStream failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        if( m_pVampVBIStream->GetObjectStatus() != STATUS_OK )
        {
            delete m_pVampVBIStream;
            m_pVampVBIStream = NULL;
            delete m_pSpinLockObj;
            m_pSpinLockObj = NULL;
            _DbgPrintF( DEBUGLVL_ERROR,
                        ("Error: Invalid CVampVBIStream object status"));
            return STATUS_UNSUCCESSFUL;
        }
    }
    // every thing was just fine so we should not forget to set the
    // extended stream header size
    pKSPin->StreamHeaderSize =
        sizeof(KSSTREAM_HEADER) + sizeof(KS_VBI_FRAME_INFO);
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Remove pin dispatch function. This function cleans up all, during
//  Create() allocated resources. If existing, the VBI stream and the
//  synchronizing functionality are not used any longer.
//  HAL access is performed by this function.
//
// Return Value:
//  STATUS_SUCCESS          Remove pin dispatch function with success
//  STATUS_UNSUCCESSFUL     Operation failed, e.g.
//                          (a) the function parameters are zero,
//                          (b) hardware interface access failed
//                          (c) no VBI stream object exists
//                          (d) no synchronization object exists
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgVBIOut::Remove
(
    IN PIRP   pIrp      //Pointer to IRP for this request. (unused)
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgVBIOut::Remove"));

    // remove/delete all objects no longer used
    if( m_pVampVBIStream )
    {
        delete m_pVampVBIStream;
        m_pVampVBIStream = NULL;
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
//  Opens the VBI stream object. 'Open' is executed on the VBI stream object.
//  HAL access is performed by this function.
//
// Return Value:
//  STATUS_SUCCESS        VBI stream was opened.
//  STATUS_UNSUCCESSFUL   Operation failed, e.g.
//                          (a) the function parameters are zero,
//                          (b) hardware interface access failed
//                          (c) no VBI stream object exists
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgVBIOut::Open
(
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgVBIOut::Open"));

    //member variable valid?
    if( !m_pKSPin || !m_pVampVBIStream )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    // get device resources object from pKSPin
    CVampDeviceResources* pDevResObj = GetDeviceResource(m_pKSPin);
    if( !pDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }

    // "tStreamParams" is the struct to setup the Vamp HAL
    tStreamParms tStreamParams;

    tStreamParams.nField = EITHER_FIELD;
    tStreamParams.pCallbackFktn = dynamic_cast <CCallOnDPC*> (this);
    tStreamParams.pCBParam1  = m_pKSPin; // give Pin to Callback routine
    tStreamParams.pCBParam2  = NULL;
    tStreamParams.pTimeStamp = NULL;
    tStreamParams.u.tVStreamParms.nOutputType = DMA;

    // Decoder standard should have been already set to AUTOSTD.
    // In case only VBI pin is connected we have to do it here.
    tStreamParams.u.tVStreamParms.nVideoStandard = AUTOSTD;

    // stream format is always Y8
    tStreamParams.u.tVStreamParms.nFormat = Y8;

    // set stream flags to 0, so we make sure we get the buffers in the
    // correct order from the decoder (...odd-even-odd-even-odd...)
    tStreamParams.u.tVStreamParms.dwStreamFlags = 0;
    tStreamParams.u.tVStreamParms.dwOutputSizeX =
        m_VBIInfoHeader.SamplesPerLine;
    tStreamParams.u.tVStreamParms.tVideoInputRect.nStartX = 0;
    tStreamParams.u.tVStreamParms.tVideoInputRect.nWidth  = 720;
    tStreamParams.u.tVStreamParms.u.FrameRateAndFraction.wFractionPart = 0;

    // Is50Hz returns always the right value!
    // if a signal was detected it returns the right standard
    // if no signal could be detected it returns the default standard,
    // the decoder got from the registry

    if( pDevResObj->m_pDecoder->GetDecoderStatus( VS_FIDT ) == 0 )
    {   //50Hz/625lines standard detected
        // set member duration value
        tStreamParams.u.tVStreamParms.tVideoInputRect.nStartY =
                                                        VBI_START_LINE_PAL;
        tStreamParams.u.tVStreamParms.tVideoInputRect.nHeight =
                                                        VBI_LINES_PAL;
        tStreamParams.u.tVStreamParms.u.FrameRateAndFraction.wFrameRate =
                                                        VBI_PAL_FrameRate;
        tStreamParams.u.tVStreamParms.dwOutputSizeY = VBI_LINES_PAL;

	//update changed parameters in the info header (just in case)
	m_VBIInfoHeader.StartLine = VBI_START_LINE_PAL;
	m_VBIInfoHeader.EndLine = VBI_STOP_LINE_PAL;
	m_VBIInfoHeader.VideoStandard = KS_AnalogVideo_PAL_B;
    }
    else
    {   //60Hz/525lines standard detected
        // set member duration value
        tStreamParams.u.tVStreamParms.tVideoInputRect.nStartY =
                                                        VBI_START_LINE_NTSC;
        tStreamParams.u.tVStreamParms.tVideoInputRect.nHeight =
                                                        VBI_LINES_NTSC;
        tStreamParams.u.tVStreamParms.u.FrameRateAndFraction.wFrameRate =
                                                        VBI_NTSC_FrameRate;
        tStreamParams.u.tVStreamParms.dwOutputSizeY = VBI_LINES_NTSC;

	//update changed parameters in the info header (just in case)
	m_VBIInfoHeader.StartLine = VBI_START_LINE_NTSC;
	m_VBIInfoHeader.EndLine = VBI_STOP_LINE_NTSC;
	m_VBIInfoHeader.VideoStandard = KS_AnalogVideo_NTSC_M;
    }

    // pass the structure down to the Vamp HAL
    if( m_pVampVBIStream->Open(&tStreamParams) != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Open VBI stream failed"));
        return STATUS_UNSUCCESSFUL;
    }

    m_dwPictureNumber   = 0;
    m_dwLostBuffers     = 0;
    m_bVBIInfoChanged	= TRUE;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Closes the VBI stream object. 'Close' is executed on the VBI stream
//  object. All stream related resources (vamp buffers) have to be released.
//  This release procedure has to be synchronized.
//  HAL access is performed by this function.
//
// Return Value:
//  STATUS_SUCCESS        VBI stream was closed successfully.
//  STATUS_UNSUCCESSFUL   Operation failed, e.g.
//                          (a) the function parameters are zero,
//                          (b) no VBI stream object exists
//                          (c) release vamp buffers failed
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgVBIOut::Close
(
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgVBIOut::Close"));

    CVampBuffer* pVampBuffer = NULL;

    //member variable valid?
    if( !m_pVampVBIStream || !m_pSpinLockObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    // use spin lock while buffer access
    m_pSpinLockObj->AcquireSpinLock(); // void function
    if( m_pVampVBIStream->CancelAllBuffers() != VAMPRET_SUCCESS )
    {
        //CancelAllBuffers failed
        m_pSpinLockObj->ReleaseSpinLock(); // void function
        _DbgPrintF(DEBUGLVL_ERROR,("Error: CancelAllBuffers failed"));
        return STATUS_UNSUCCESSFUL;
    }
    //Get all the buffer from the Vamp HAL
    while( m_pVampVBIStream->GetNextDoneBuffer(&pVampBuffer) ==
            VAMPRET_SUCCESS )
    {
        if( !pVampBuffer )
        {
            m_pSpinLockObj->ReleaseSpinLock(); // void function
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid buffer pointer"));
            return STATUS_UNSUCCESSFUL;
        }

        //Get the stream pointer out of the buffer context
        PKSSTREAM_POINTER pStreamPointer =
                static_cast <PKSSTREAM_POINTER> (pVampBuffer->GetContext());
        if( pStreamPointer )
        {
            //release system buffer
            KsStreamPointerDelete(pStreamPointer); // void function
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
    m_pSpinLockObj->ReleaseSpinLock(); // void function

    //close the VBI stream
    if( m_pVampVBIStream->Close() != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Close VBI stream failed"));
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Starts the VBI stream. 'Start' is executed on the VBI stream object.
//  First the Vamp buffers are created and added to the stream.
//  Second the reference clock object is created here.
//  (The pin must have received notification of the master clock.)
//  The reference object is used to 'timestamp' the filled buffers.
//  Thirdly the stream is started.
//  HAL access is performed by this function.
//
// Return Value:
//  STATUS_SUCCESS        VBI stream was started (run state).
//  STATUS_UNSUCCESSFUL   Operation failed, e.g.
//                          (a) the function parameters are zero,
//                          (b) no VBI stream object exists
//                          (c) create reference clock object failed
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgVBIOut::Start
(
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgVBIOut::Start"));

    //member variable valid?
    if( !m_pKSPin || !m_pVampVBIStream )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
    DWORD dwIndex = 0;

    do //while system buffers available
    {
        ntStatus = CAnlgVBIOut::Process();
        //increment buffer count index
        dwIndex++;
    }while( ntStatus == STATUS_SUCCESS );

    //check, if the number of inserted buffer is equal to
    //formerly demanded buffers
    if( dwIndex != NUM_VBI_STREAM_BUFFER )
    {
        // This is not a bug but there is a Microsoft restriction
        // where sometimes we dont get all the requested buffers.
        _DbgPrintF( DEBUGLVL_VERBOSE,
                    ("Warning: Number of buffers do not match!"));
    }
    // try to get the reference to the master clock object
    if( !m_pIKsClock )
    {
        if( !NT_SUCCESS(KsPinGetReferenceClockInterface(m_pKSPin,
                                                        &m_pIKsClock)) )
        {
            // If we could not get an interface to the clock,
            // don't proceed
            m_pIKsClock = NULL;
            _DbgPrintF(DEBUGLVL_ERROR,("Error: No clock object available"));
            return STATUS_UNSUCCESSFUL;
        }
    }

    //start the VBI stream
    if( m_pVampVBIStream->Start() != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Start VBI stream failed"));
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function is called to stop the stream.
//  This function cleans up all, during Start() allocated resources.
//  HAL access is performed by this function.
//
// Return Value:
//  STATUS_SUCCESS        VBI stream was stopped (stop state).
//  STATUS_UNSUCCESSFUL   Operation failed, e.g.
//                          (a) the function parameters are zero,
//                          (b) no VBI stream object exists
//                          (c) no reference clock object exists
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgVBIOut::Stop
(
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgVBIOut::Stop"));

    //member variable valid?
    if( !m_pVampVBIStream )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }
	VAMPRET vrResult = m_pVampVBIStream->Stop(FALSE);
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
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Stop VBI stream failed"));
        return STATUS_UNSUCCESSFUL;
    }
    // reset the captured fields counter
    m_dwPictureNumber = 0;
    // reset the dropped fields counter
    m_dwLostBuffers = 0;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function is called from the video in pin, where the channel change
//  info arrived in the process function. If the tuner has changed the
//  channel, this function will be called with the new tuner information
//  structure. This information has to be passed to the vbi output pin, so
//  any vbi codec can react on it.
//
// Return Value:
//  STATUS_SUCCESS              The new tuner information has been
//                              stored.
//  STATUS_UNSUCCESSFUL         Any other case.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgVBIOut::SignalTvTunerChannelChange
(
    IN PKS_TVTUNER_CHANGE_INFO pKsChannelChangeInfo //New info we get from the
                                                 //tuner after channel change.
)
{
    //parameters valid?
    if( !pKsChannelChangeInfo )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    if( pKsChannelChangeInfo->dwFlags == KS_TVTUNER_CHANGE_BEGIN_TUNE )
    {
        _DbgPrintF(DEBUGLVL_BLAB,("Info: KS_TVTUNER_CHANGE_BEGIN_TUNE"));
    }
    else if( pKsChannelChangeInfo->dwFlags == KS_TVTUNER_CHANGE_END_TUNE )
    {
        _DbgPrintF(DEBUGLVL_BLAB,("Info: KS_TVTUNER_CHANGE_END_TUNE"));
    }
    // save new tuner info for private use
    m_TVTunerChangeInfo = *pKsChannelChangeInfo;
    // set flag that we got a new tuner info
    m_bNewTunerInfo = true;

    return STATUS_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the callback function that is called from the HAL after an
//  interrupt in the systems DPC level.
//  Get next filled buffer from the hardware (in a synchronized way),
//  'timestamp' the buffer.
//  It attempts to initiate processing on Pin by sending a processing
//  dispatch call to Pin's processing object.
//  HAL access is performed by this function.
//
// Return Value:
//  This function is registered and called from within the system,
//  respectivly VAMP RETURN values are used.
//  VAMPRET_SUCCESS     Buffer handling was successfully.
//  VAMPRET_NULL_POINTER_PARAMETER
//                      Operation failed, the function parameters are zero
//
//////////////////////////////////////////////////////////////////////////////
VAMPRET CAnlgVBIOut::CallOnDPC
(
    PVOID pStream,  //Pointer to stream object (unused).
    PVOID pParam1,  //User defined parameter from the open function.
                    //Contains the associated KS pin in this case.
    PVOID pParam2   //User defined parameter from the open function. (unused)
)
{
    //parameters valid?
    if( !pParam1 )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return VAMPRET_NULL_POINTER_PARAMETER;
    }

    CVampBuffer*        pVampBuffer     = NULL;
    PKSSTREAM_POINTER   pStreamPointer  = NULL;

    //member variable valid?
    if( !m_pVampVBIStream || !m_pIKsClock || !m_pSpinLockObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return VAMPRET_GENERAL_ERROR;
    }

    m_pSpinLockObj->AcquireSpinLock(); // void function

    //get all done vamp buffer out of our queue
    while( m_pVampVBIStream->GetNextDoneBuffer(&pVampBuffer) ==
            VAMPRET_SUCCESS )
    {
        m_pSpinLockObj->ReleaseSpinLock(); // void function

        //check buffer pointer
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
        //set data context information to the stream header
        //the data then will be available on the VBI PIN so every connected
        //codec get the information (e.g. odd field, channel cahnge, ...)
        if( FillBufferHeader( pStreamPointer->StreamHeader,
                             pVampBuffer ) != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Buffer header filling failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        //give buffer back to the system
        KsStreamPointerDelete(pStreamPointer);
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

    m_pSpinLockObj->ReleaseSpinLock(); // void function

    return VAMPRET_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  It attempts to initiate processing on Pin.
//  A Vamp buffer is created (using system buffer informations/memory)
//  and added to the stream.
//  HAL access is performed by this function.
//
// Return Value:
//  STATUS_SUCCESS      Buffer insertion was successfully.
//  an NTSTATUS - value Error value returned by KsStreamPointerAdvance
//                      call.
//  STATUS_UNSUCCESSFUL Operation failed, e.g.
//                        (a) the function parameters are zero,
//                        (b) no 'next available system buffer'
//                        (c) couldn't clone the system buffer
//                        (d) create Vamp buffer object failed
//                        (e) couldn't add buffer to the stream
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgVBIOut::Process
(
)
{
    //member variable valid?
    if( !m_pSpinLockObj || !m_pVampVBIStream || !m_pKSPin )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

    //get next available system buffer
    PKSSTREAM_POINTER pStreamPointer =
        KsPinGetLeadingEdgeStreamPointer(m_pKSPin, KSSTREAM_POINTER_STATE_LOCKED);

    //check stream pointer
    if( !pStreamPointer )
    {
        //no system buffer available
        //This case can happen if it is the last pointer in the queue or
        //the system cannot give us the required buffer
        _DbgPrintF(DEBUGLVL_VERBOSE,("Warning: No system buffer available"));
        return STATUS_UNSUCCESSFUL;
    }
    else
    {
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
            _DbgPrintF(DEBUGLVL_ERROR,
                    ("Error: Streampointer cloning unsuccessful"));
            return STATUS_UNSUCCESSFUL;
        }
        //*** set vamp buffer format parameters ***
        tBufferFormat tBufferParams;

        //set default value (used for audio only)
        tBufferParams.dwChannelsPerSample = 1;
        //Y only format (grey) has one byte per pixel
        tBufferParams.dwBitsPerSample = VBI_BITS_PER_PIXEL; //8;
        //set number of lines
        tBufferParams.dwNrOfLines =
            m_VBIInfoHeader.EndLine - m_VBIInfoHeader.StartLine + 1;
        //set samples per line
        tBufferParams.dwSamplesPerLine = m_VBIInfoHeader.SamplesPerLine;
        //calculate pitch in bytes
        tBufferParams.lPitchInBytes =
            static_cast <LONG> ( m_VBIInfoHeader.StrideInBytes );
        //calculate the buffer length
        tBufferParams.lBufferLength =
            tBufferParams.lPitchInBytes *
                static_cast <LONG> (tBufferParams.dwNrOfLines);
        //dont change the byte order
        tBufferParams.ByteOrder = KEEP_AS_IS;
        //set buffer queued
        tBufferParams.BufferType = CVAMP_BUFFER_QUEUED;
        //is the buffer size correct?
        if( pStreamPointerClone->StreamHeader->FrameExtent <
            static_cast <DWORD> (tBufferParams.lBufferLength) )
        {
            //give buffer back to the system, we cannot use it for some reason
            KsStreamPointerDelete(pStreamPointerClone);
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Buffer size incorrect"));
            return STATUS_UNSUCCESSFUL;
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
            KsStreamPointerDelete(pStreamPointerClone);
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Not enough memory"));
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        if( pVampBuffer->GetObjectStatus() != STATUS_OK )
        {
            //de-allocate memory for the Vamp buffer object
            delete pVampBuffer;
            pVampBuffer = NULL;
            //give buffer back to the system, we cannot use it for some reason
            KsStreamPointerDelete(pStreamPointerClone);
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Buffer Object status failed"));
            return STATUS_UNSUCCESSFUL;
        }
        //set KS stream pointer into the buffer context for use in the CallOnDPC function
        pVampBuffer->SetContext(static_cast <PVOID> (pStreamPointerClone));
        //aquire spin lock to be the only one that has access to the buffer queue
        m_pSpinLockObj->AcquireSpinLock(); // void function
        //add buffer to Vamp buffer queue
        if( m_pVampVBIStream->AddBuffer(pVampBuffer) != VAMPRET_SUCCESS )
        {
            //de-allocate memory for the Vamp buffer object
            delete pVampBuffer;
            pVampBuffer = NULL;
            //give buffer back to the system, we cannot use it for some reason
            KsStreamPointerDelete(pStreamPointerClone); // void function
            m_pSpinLockObj->ReleaseSpinLock(); // void function
            _DbgPrintF(DEBUGLVL_ERROR,("Error: AddBuffer to VBI stream failed"));
            return STATUS_UNSUCCESSFUL;
        }

        //release the spin lock to give others access to the buffer queue again
        m_pSpinLockObj->ReleaseSpinLock(); // void functuion
        //advance stream pointer to the next available system buffer
        ntStatus = KsStreamPointerAdvance(pStreamPointer);

        if( (ntStatus != STATUS_DEVICE_NOT_READY) &&
            (ntStatus != STATUS_SUCCESS) )
        {
            _DbgPrintF( DEBUGLVL_ERROR,
                        ("Error: VBI Streampointer advacement failed"));
        }
    }

    return ntStatus;
}


//*** function implementation for private use ***//

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Fills out the stream information & extended information structure for the
//  current buffer. The information then will be available on the VBI pin.
//
// Remarks:
//  At least, Field (odd/even), Duration, DataUsed and PictureNumber have to
//  be filled out!
//
// Return Value:
//  STATUS_SUCCESS      All information could be written successfully.
//  STATUS_UNSUCCESSFUL Any other case.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgVBIOut::FillBufferHeader
(
    IN OUT PKSSTREAM_HEADER pStreamHeader,  //Structure where we fill in all
                                            //neccessary information.
    IN CVampBuffer* pVampBuffer             //pointer to the current buffer.
)
{
    //parameters valid?
    if( !pStreamHeader || !pVampBuffer )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    //member variable valid?
    if( !m_pVampVBIStream || !m_pIKsClock )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    // ************* VBI Stream Header Information *************
    // reset flags
    pStreamHeader->OptionsFlags = 0;

    // timestamp
    pStreamHeader->PresentationTime.Time = m_pIKsClock->GetTime();
    pStreamHeader->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_TIMEVALID;

    pStreamHeader->PresentationTime.Numerator = 1;
    pStreamHeader->PresentationTime.Denominator = 1;

    // --- set field duration releated to video standard
    // >>> !!! BIT-COMPARE !!! <<<
    if( m_VBIInfoHeader.VideoStandard & VIDEO_STANDARD_50HZ_625LINES )
    {
        pStreamHeader->Duration = VBI_PAL_FieldDuration;
        pStreamHeader->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;
    }
    else if( m_VBIInfoHeader.VideoStandard & VIDEO_STANDARD_60HZ_525LINES )
    {
        pStreamHeader->Duration = VBI_NTSC_FieldDuration;
        pStreamHeader->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;
    }
    else
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Wrong video standard"));
        return STATUS_UNSUCCESSFUL;
    }

    // --- calculate dropped frames
    // if there were lost buffers set data discontinuity flag
    if( m_dwLostBuffers < pVampBuffer->GetStatus().dwLostBuffer )
    {
        pStreamHeader->OptionsFlags |=
                KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;
    }
    // save new dropped count value
    m_dwLostBuffers = pVampBuffer->GetStatus().dwLostBuffer;

    // set DataUsed to max. value to copy the whole buffer to the system
    pStreamHeader->DataUsed = m_VBIInfoHeader.BufferSize;
        // the buffer is allocated with BufferSize so we ever set this size
        // to be used. We do not use the buffer length value from the hal.
        // static_cast <DWORD> (pVampBuffer->GetStatus().lActualBufferLength);

    // ************* Extended VBI Stream Header Information *************
    //there might be an additional structure for the frame information
    PKS_VBI_FRAME_INFO pKSFrameInfoHeader =
        reinterpret_cast <PKS_VBI_FRAME_INFO> (pStreamHeader+1);
    if( !pKSFrameInfoHeader )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Frame info header invalid!"));
        return STATUS_UNSUCCESSFUL;
    }
    //additional header available?
    if( pStreamHeader->Size >= sizeof (KSSTREAM_HEADER) + sizeof (KS_VBI_FRAME_INFO) ) {
	
        pKSFrameInfoHeader->ExtendedHeaderSize = sizeof (KS_VBI_FRAME_INFO);
	
        if( ODD_FIELD == pVampBuffer->GetStatus().nFieldType )
        {
            pKSFrameInfoHeader->dwFrameFlags = KS_VIDEO_FLAG_FIELD1;
        }
        else if( EVEN_FIELD == pVampBuffer->GetStatus().nFieldType )
        {
            pKSFrameInfoHeader->dwFrameFlags = KS_VIDEO_FLAG_FIELD2;
        }
        else
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: nFieldType invalid!"));
            return STATUS_UNSUCCESSFUL;
        }
        pKSFrameInfoHeader->PictureNumber          = ++m_dwPictureNumber;
        pKSFrameInfoHeader->DropCount              = m_dwLostBuffers;
        pKSFrameInfoHeader->dwSamplingFrequency    =
                                    m_VBIInfoHeader.SamplingFrequency;
        // send on tv tuner info to vbi codec: nb the begin-tune and end-tune
        // flags are for the vbi codec to act on, not us.
        if( m_bNewTunerInfo )
        {
            //KS_TVTUNER_CHANGE_INFO
            m_bNewTunerInfo = false;
            pKSFrameInfoHeader->TvTunerChangeInfo = m_TVTunerChangeInfo;
            pKSFrameInfoHeader->dwFrameFlags |= KS_VBI_FLAG_TVTUNER_CHANGE;
        }
	if(m_bVBIInfoChanged == TRUE )
	{
	    m_bVBIInfoChanged = FALSE;
	    pKSFrameInfoHeader->dwFrameFlags |= KS_VBI_FLAG_VBIINFOHEADER_CHANGE;
	    
	    pKSFrameInfoHeader->VBIInfoHeader = m_VBIInfoHeader;
	}

    }

    return STATUS_SUCCESS;
}
