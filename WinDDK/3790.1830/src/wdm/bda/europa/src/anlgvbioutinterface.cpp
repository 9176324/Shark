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
#include "AnlgVBIOutInterface.h"
#include "AnlgVBIOut.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the SetDataFormat dispatch function of the analog VBI out pin.
//  If called the first time, it creates an analog VBI out pin object
//  and stores the analog VBI out pin pointer in the pin context.
//  If not called the first time, it retrieves the analog VBI out pin
//  pointer from the pin context.
//  It also calls the setting of the data format on the instance
//  of the analog VBI out pin.
//
// Return Value:
//  STATUS_SUCCESS       The new data format was set to the VBI pin.
//  STATUS_INSUFFICIENT_RESOURCES
//                       NEW operation failed. VBI pin couldn't be created.
//  STATUS_INVALID_DEVICE_STATE
//                       Format change request while not in STOP state.
//  STATUS_UNSUCCESSFUL  Operation failed,e.g.
//                       (a) if the function parameters are zero,
//                       (b) if the analog VBI out pin pointer is zero
//                       (c) SetDataFormat failed
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgVBIOutPinSetDataFormat
(
 IN PKSPIN pKSPin,                                  //Used for system support function calls.
 IN OPTIONAL PKSDATAFORMAT pKSOldFormat,            //Pointer to previous format
                                                    //structure, is 0 during the
                                                    //first call.
 IN OPTIONAL PKSMULTIPLE_ITEM pKSOldAttributeList,  //previous attributes.
 IN const KSDATARANGE *pKSDataRange,                //Pointer to matching range
 IN OPTIONAL const KSATTRIBUTE_LIST *pKSAttributeRange //New attributes.
 )
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgVBIOutPinSetDataFormat called"));

    // Check connection format pointer
    if( !pKSPin->ConnectionFormat )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid connection format"));
        return STATUS_UNSUCCESSFUL;
    }


    // check the format
    if( !(IsEqualGUID(pKSPin->ConnectionFormat->Specifier,
        KSDATAFORMAT_SPECIFIER_VBI) &&
        (pKSPin->ConnectionFormat->FormatSize ==
        sizeof(KS_DATAFORMAT_VBIINFOHEADER))))
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid format size"));
        return STATUS_UNSUCCESSFUL;
    } 


    // get the connection format out of the KS pin
    PKS_DATAFORMAT_VBIINFOHEADER pConnectionFormat =
        reinterpret_cast<PKS_DATAFORMAT_VBIINFOHEADER>(pKSPin->ConnectionFormat);
    //It is not necessary to check again because the pKSPin->ConnectionFormat
    //is already checked before and the reinterpret can not return zero.
    const PKS_DATARANGE_VIDEO_VBI pDescriptorFormat =
        (const PKS_DATARANGE_VIDEO_VBI)(pKSDataRange);
    // VBI does not support ranges so we can check the complete
    // format for consistency
    if( RtlCompareMemory(&pConnectionFormat->VBIInfoHeader,
        &pDescriptorFormat->VBIInfoHeader,
        sizeof(KS_VBIINFOHEADER)) !=
        sizeof(KS_VBIINFOHEADER) )
    {
        _DbgPrintF(DEBUGLVL_BLAB, \
            ("AnlgVBIOutSetDataFormat returns STATUS_NO_MATCH"));
        return STATUS_NO_MATCH;
    }

    if( !(NT_SUCCESS(KsEdit(pKSPin, &pKSPin->Descriptor, 'pmaV'))) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: KS edit failed"));
        return STATUS_UNSUCCESSFUL;
    }

    // If the edits proceeded without running out of memory, adjust
    // the framing based on the video info header.
    if( !(NT_SUCCESS(KsEdit(pKSPin,
        &(pKSPin->Descriptor->AllocatorFraming),'pmaV'))) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: 2nd KS edit failed"));
        return STATUS_UNSUCCESSFUL;
    }

    // We've KsEdit'ed this...  I'm safe to cast away constness as
    // long as the edit succeeded.
    PKSALLOCATOR_FRAMING_EX Framing =
        const_cast <PKSALLOCATOR_FRAMING_EX>
        (pKSPin->Descriptor->AllocatorFraming);

    Framing -> FramingItem [0].Frames = NUM_VBI_STREAM_BUFFER;

    // The physical and optimal ranges must be biSizeImage. We only
    // support one frame size, precisely the size of each capture
    // image.
    Framing -> FramingItem [0].PhysicalRange.MinFrameSize =
        Framing -> FramingItem [0].PhysicalRange.MaxFrameSize =
        Framing -> FramingItem [0].FramingRange.Range.MinFrameSize =
        Framing -> FramingItem [0].FramingRange.Range.MaxFrameSize =
        pConnectionFormat->VBIInfoHeader.BufferSize;

    Framing -> FramingItem [0].PhysicalRange.Stepping       = 0;
    Framing -> FramingItem [0].FramingRange.Range.Stepping  = 0;

    return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the intersect handler for comparison of a data range.
//  of the analog VBI out pin.(Needs no class environment to intersect
//  the format, because the format that is supported is already
//  given in the filter descriptor).
//
// Settings:
//  A match occurs under three conditions:
//  (1) if the major format of the range passed matches a pin factory range,
//  (2) if the sub-format matches,
//  (3) if the specifier is KSDATAFORMAT_SPECIFIER_VBI.
//  Since the data range size may be variable, it is not validated beyond
//  checking that it is at least the size of a data range structure.
//  (KSDATAFORMAT_SPECIFIER_VBI)
//
//  Remarks:
//  To set up the right format, we ask the decoder for the current format
//  and if it does not match to the pCallerDataRange, we return
//  STATUS_NO_MATCH to get called again with the next format from the
//  descriptor.
//  If it matches our decoder setting, we write it to the target format
//  pData.
//
// Return Value:
//  STATUS_SUCCESS           If a matching range is found and it fits
//                           in the supplied buffer.
//  STATUS_UNSUCCESSFUL      Operation failed,
//                           the function parameters are zero.
//  STATUS_BUFFER_OVERFLOW   For successful size queries. (in that case
//                           the buffer size is zero and has to be set
//                           to the right value before return)
//  STATUS_BUFFER_TOO_SMALL  If the supplied buffer is too small.
//  STATUS_NO_MATCH             (a) if the intersection is empty, or
//                              (b) if no matching range was found
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgVBIOutIntersectDataFormat
(
 IN PKSFILTER pKSFilter,                //Pointer to KS filter structure.
 IN PIRP      pIrp,                     //Pointer to data intersection
                                        //property request.
 IN PKSP_PIN  pPinInstance,             //Pointer to structure indicating
                                        //pin in question.
 IN PKSDATARANGE pCallerDataRange,      //Pointer to the caller data
                                        //structure that should be
                                        //intersected.
 IN PKSDATARANGE pDescriptorDataRange,  //Pointer to the descriptor data
                                        //structure, see AnlgVbiFormats.h.
 IN DWORD     dwBufferSize,             //Size in bytes of the target
                                        //buffer structure. For size
                                        //queries, this value is zero.
 OUT OPTIONAL PVOID pData,              //Pointer to the target data
                                        //structure representing the best
                                        //format in the intersection.
 OUT PDWORD   pdwDataSize               //Pointer to size of target data
                                        //format structure.
 )
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgVBIOutIntersectDataFormat called"));

    //invalid parameters?
    if( !pKSFilter || !pIrp || !pPinInstance ||
        !pCallerDataRange || !pDescriptorDataRange || !pdwDataSize )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //*** start intersection ***//

    //check if given datarange GUID matches to VBI
    const GUID VBIInfoSpecifier =
    {STATICGUIDOF(KSDATAFORMAT_SPECIFIER_VBI)};
    if( !IsEqualGUID(pCallerDataRange->Specifier, VBIInfoSpecifier) ||
        pCallerDataRange->FormatSize != sizeof(KS_DATARANGE_VIDEO_VBI) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Invalid GUID or invalid format structure size"));
        return STATUS_NO_MATCH;
    }

    //map given data ranges on vbi information structures
    //for some reason only a subpart of the KS_DATARANGE_VIDEO_VBI
    //is used for intersection this subpart is casted to
    //KS_DATAFORMAT_VIDEOINFOHEADER and contains only
    //KSDATAFORMAT and KS_VBIINFOHEADER, see pTargetVideoDataRange
    PKS_DATARANGE_VIDEO_VBI pCallerVbiDataRange =
        reinterpret_cast <PKS_DATARANGE_VIDEO_VBI> (pCallerDataRange);
    PKS_DATARANGE_VIDEO_VBI pDescriptorVbiDataRange =
        reinterpret_cast <PKS_DATARANGE_VIDEO_VBI> (pDescriptorDataRange);
    PKS_DATAFORMAT_VBIINFOHEADER pTargetVbiDataRange =
        reinterpret_cast <PKS_DATAFORMAT_VBIINFOHEADER> (pData);

    //check if all other important fields match
    if( (pCallerVbiDataRange->bFixedSizeSamples !=
        pDescriptorVbiDataRange->bFixedSizeSamples) ||
        (pCallerVbiDataRange->bTemporalCompression !=
        pDescriptorVbiDataRange->bTemporalCompression) ||
        (pCallerVbiDataRange->StreamDescriptionFlags !=
        pDescriptorVbiDataRange->StreamDescriptionFlags) ||
        (pCallerVbiDataRange->MemoryAllocationFlags !=
        pDescriptorVbiDataRange->MemoryAllocationFlags)
        )
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("Warning: Important parameter does not match"));
        return STATUS_NO_MATCH;
    }

    //set output data size
    *pdwDataSize = sizeof(KS_DATAFORMAT_VBIINFOHEADER);
    //check for size only query
    if( dwBufferSize == 0 )
    {
        _DbgPrintF(DEBUGLVL_BLAB,("Info: Size only query"));
        return STATUS_BUFFER_OVERFLOW;
    }

    //Check pData in case of
    if( !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //check if output buffer size is sufficient
    if( dwBufferSize < *pdwDataSize )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Buffer too small"));
        return STATUS_BUFFER_TOO_SMALL;
    }

    // Perform only data intersection. Intersect Handler is not current format aware!

    if(pCallerVbiDataRange->VBIInfoHeader.VideoStandard != pDescriptorVbiDataRange->VBIInfoHeader.VideoStandard)
    {
        _DbgPrintF(DEBUGLVL_BLAB, \
            ("AnlgVBIOutIntersectDataFormat returns STATUS_NO_MATCH"));
        return STATUS_NO_MATCH;
    }

    //copy the data range structure from our decriptor into the
    //target format buffer, for some reason two different names
    //where given to the same structure
    //KSDATARANGE equals exactly KSDATAFORMAT
    pTargetVbiDataRange->DataFormat =
        static_cast <KSDATAFORMAT> (pDescriptorVbiDataRange->DataRange);

    //as mentioned above the target data range structure differs from the
    //caller and the descriptor structures, so the size is also different
    //and has to be set correctly
    pTargetVbiDataRange->DataFormat.FormatSize = *pdwDataSize;

    //copy the video info header structure from caller into target buffer,
    //we have to check at this time if the requested caller video format
    //fits our capabilities (not implemented right now)

    pTargetVbiDataRange->VBIInfoHeader =
        pDescriptorVbiDataRange->VBIInfoHeader;

    return STATUS_SUCCESS;
}


//*** function implementation for private use ***//

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function is used to get the device resources of the Vamp device
//  out of a KS filter object.
//  HAL access is performed by this function.
//
// Return Value:
//  CVampDeviceResources*   Pointer to the Vamp device resource object.
//  NULL                    Any error case.
//
//////////////////////////////////////////////////////////////////////////////
CVampDeviceResources* AnlgVBIOutGetVampDevResObj
(
 IN  PKSFILTER pKSFilter    //Pointer to KS filter structure.
 )
{
    //invalid parameters?
    if( !pKSFilter )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return NULL;
    }

    // get the decoder out of the filter object
    PKSDEVICE pKSDevice = KsGetDevice(pKSFilter);
    if( !pKSDevice )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Cannot get connection to KS device"));
        return NULL;
    }
    //get the VampDevice object out of the Context of device object
    CVampDevice* pVampDevice = static_cast <CVampDevice*> (pKSDevice->Context);
    if( !pVampDevice )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Cannot get connection to device object"));
        return NULL;
    }
    //get the device resource object out of the VampDevice object
    CVampDeviceResources* pVampDevResObj =
        pVampDevice->GetDeviceResourceObject();
    if( !pVampDevResObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Cannot get device resource object"));
        return NULL;
    }

    return pVampDevResObj;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the create dispatch function of the stream pin.
//  It stores the class pointer of the pin class in the pin context.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) if the pin pointer is zero
//                          (c) if stream Create failed
//  STATUS_INSUFFICIENT_RESOURCES
//                          Operation failed,
//                          if stream Create failed
//  STATUS_SUCCESS          Created pin derived from base pin with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgVBIOutPinCreate
(
 IN PKSPIN pKSPin,   //KS pin, used for system support function calls.
 IN PIRP   pIrp      //Pointer to IRP for this request.
 )
{
    NTSTATUS Status = STATUS_SUCCESS;

    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgVBIOutPinCreate called"));
    //  parameters valid?
    if( !pKSPin || !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    // construct class environment
    CAnlgVBIOut* pAnlgVBIOutPin = new (NON_PAGED_POOL) CAnlgVBIOut();
    if( !pAnlgVBIOutPin )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    //store class object pointer into the context of KS pin
    pKSPin->Context = pAnlgVBIOutPin;
    //call class function
    Status = pAnlgVBIOutPin->Create(pKSPin, pIrp);
    if( Status != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
            ("Error: Class function called without success"));
        delete pAnlgVBIOutPin;
        pAnlgVBIOutPin = NULL;
        pKSPin->Context = NULL;
        return Status;
    }
    return STATUS_SUCCESS;
}
