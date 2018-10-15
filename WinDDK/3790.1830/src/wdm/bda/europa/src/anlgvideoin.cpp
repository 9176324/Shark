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


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  Interface functions for our class environment, to support C interface
//  from Microsoft. The following functions are all global, so they have to
//  have a unique name and must never use any global/static resources
//  to avoid multiboard problems.
//
//////////////////////////////////////////////////////////////////////////////

#include "34avstrm.h"
#include "VampDevice.h"

#include "AnlgVideoIn.h"
#include "AnlgVbiOut.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Process dispatch function. This is a very special version of a process
//  function. During a channel change on the tuner, you need to inform the VBI
//  pin about this channel change and also the audio standard detection about
//  the possibly new standard. The VBI pin needs this information to
//  actualize the already stored VBI data e.g. to get new Teletext pages.
//  To receive this information there is a "small streaming" set up on this
//  pin that consists of only one buffer that contains the channel change
//  information.
//
// Return Value:
//  STATUS_SUCCESS          Got ChannelChangeInfo and sent it to VbiOutPin.
//  STATUS_UNSUCCESSFUL     Any error case.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgVideoInPinProcess
(
    IN PKSPIN pKSPin    //KS pin, used for system support function calls
)
{
    //_DbgPrintF(DEBUGLVL_BLAB,(""));
    //_DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgVideoInPinProcess called"));

    // pointer to parent filter (AnlgVideoCaptureFilter)
    PKSFILTER         pKsFilter      = NULL;
    PKSSTREAM_POINTER pStreamPointer = NULL;
    NTSTATUS          ntStatus       = STATUS_UNSUCCESSFUL;

    //parameters valid?
    if( !pKSPin )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: AnlgVideoInPinProcess: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get next available system buffer
    pStreamPointer = KsPinGetLeadingEdgeStreamPointer(
                                           pKSPin,
                                           KSSTREAM_POINTER_STATE_LOCKED);
    if( !pStreamPointer )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: AnlgVideoInPinProcess: Streampointer invalid"));
        return STATUS_UNSUCCESSFUL;
    }
    if( !(pStreamPointer->StreamHeader) )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: AnlgVideoInPinProcess: Streampointer header invalid"));
        if(KsStreamPointerAdvance(pStreamPointer) != STATUS_SUCCESS)
        {
            _DbgPrintF(
                DEBUGLVL_VERBOSE,
                ("Warning: AnlgVideoInPinProcess:\
				Streampointer advacement failed"));
        }
        return STATUS_UNSUCCESSFUL;
    }
    if( pStreamPointer->StreamHeader->FrameExtent !=
            sizeof(KS_TVTUNER_CHANGE_INFO) )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: AnlgVideoInPinProcess:\
			Buffer size (channel change info) incorrect"));
        if(KsStreamPointerAdvance(pStreamPointer) != STATUS_SUCCESS)
        {
            _DbgPrintF(
                DEBUGLVL_VERBOSE,
                ("Warning: AnlgVideoInPinProcess:\
				Streampointer advacement failed"));
        }
        return STATUS_UNSUCCESSFUL;
    }
    // get the TV Tuner Channel Change Info
    PKS_TVTUNER_CHANGE_INFO pChannelChangeInfo =
				static_cast <KS_TVTUNER_CHANGE_INFO*>
						(pStreamPointer->StreamHeader->Data);
    // parameter  valid?
    if( !pChannelChangeInfo )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: AnlgVideoInPinProcess: Channel change info invalid"));
        // we have to UNLOCK the stream pointer before returning
        if(KsStreamPointerAdvance(pStreamPointer) != STATUS_SUCCESS)
        {
            _DbgPrintF(
                DEBUGLVL_VERBOSE,
                ("Warning: AnlgVideoInPinProcess:\
				Streampointer advacement failed"));
        }
        return STATUS_UNSUCCESSFUL;
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++
    // now tell the VBI pin class the new channel info

    // get the filter where the pin is part of
    pKsFilter = KsPinGetParentFilter( pKSPin );

    if( !pKsFilter )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
			("Error: Could not get parent Filter-Object"));
        // we have to UNLOCK the stream pointer before returning
        if(KsStreamPointerAdvance(pStreamPointer) != STATUS_SUCCESS)
        {
            _DbgPrintF(
                DEBUGLVL_VERBOSE,
                ("Warning: AnlgVideoInPinProcess:\
				Streampointer advacement failed"));
        }
        return STATUS_UNSUCCESSFUL;
    }
    if( !pKsFilter->Descriptor )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: AnlgVideoInPinProcess:\
			Could not get parent Filter-Descriptor"));
        // we have to UNLOCK the stream pointer before returning
        if(KsStreamPointerAdvance(pStreamPointer) != STATUS_SUCCESS)
        {
            _DbgPrintF(
                DEBUGLVL_VERBOSE,
                ("Warning: AnlgVideoInPinProcess:\
				Streampointer advacement failed"));
        }
        return STATUS_UNSUCCESSFUL;
    }

    // get the VBI pin out of the filter
    CAnlgVBIOut* pAnlgVBIOutPin = NULL;
    PKSPIN pKsVbiPin = NULL;
    DWORD dwIndex = pKsFilter->Descriptor->PinDescriptorsCount;

    // go through the array of pins and search the vbi pin
    // we guess 6 pins, so go from 5 down to 0!
    for(int i = (dwIndex-1); i >= 0; i--)
    {
		KsFilterAcquireControl(pKsFilter);

        pKsVbiPin = KsFilterGetFirstChildPin(pKsFilter,
                                             static_cast<ULONG>(i));
		KsFilterReleaseControl(pKsFilter);

        if( pKsVbiPin &&
            pKsVbiPin->Descriptor &&
            IsEqualGUID(*(pKsVbiPin->Descriptor->PinDescriptor.Name),
                        PINNAME_VIDEO_VBI) )
        {
            // get vbi class out of the vbi pin
            pAnlgVBIOutPin = static_cast <CAnlgVBIOut*> (pKsVbiPin->Context);
            break;
        }
    }

    // return, if the vbi pin class does not exists
    if( !pAnlgVBIOutPin )
    {
        _DbgPrintF(
            DEBUGLVL_VERBOSE,
            ("Warning: AnlgVideoInPinProcess: Could not get VBI pin class"));
        // we have to UNLOCK the stream pointer before returning
        if(KsStreamPointerAdvance(pStreamPointer) != STATUS_SUCCESS)
        {
            _DbgPrintF(
                DEBUGLVL_VERBOSE,
                ("Warning: AnlgVideoInPinProcess:\
				Streampointer advacement failed"));
        }
        //If there is no VBI pin in the filter just return with success
        return STATUS_SUCCESS;
    }

    // now we found the vbi pin and a pointer to the VBI class
    // call VBI class function for storing the new information
    if( pAnlgVBIOutPin->
		SignalTvTunerChannelChange( pChannelChangeInfo ) != STATUS_SUCCESS)
    {
        _DbgPrintF( DEBUGLVL_ERROR,
			("Error: AnlgVideoInPinProcess: SignalTvTunerChannelChange failed"));
        // we have to UNLOCK the stream pointer before returning
        if(KsStreamPointerAdvance(pStreamPointer) != STATUS_SUCCESS)
        {
            _DbgPrintF( DEBUGLVL_VERBOSE,
                        ("Warning: AnlgVideoInPinProcess:\
						Streampointer advacement failed"));
        }
        return STATUS_UNSUCCESSFUL;
    }

    // we have to UNLOCK the stream pointer before returning
    ntStatus = KsStreamPointerAdvance( pStreamPointer );
    if( (ntStatus != STATUS_DEVICE_NOT_READY) &&
        (ntStatus != STATUS_SUCCESS))
    {
        _DbgPrintF(DEBUGLVL_ERROR,
			("Error: AnlgVideoInPinProcess: Streampointer advacement failed"));
        return STATUS_UNSUCCESSFUL;
    }
    return STATUS_SUCCESS;
}

