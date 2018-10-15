//==========================================================================;
//
//  Video Decoder Device abstract base class implementation
//
//      $Date:   28 Aug 1998 14:43:00  $
//  $Revision:   1.2  $
//    $Author:   Tashjian  $
//
// $Copyright:  (c) 1997 - 1998  ATI Technologies Inc.  All Rights Reserved.  $
//
//==========================================================================;


extern "C"
{
#include "strmini.h"
#include "ksmedia.h"
}

#include "wdmdrv.h"
#include "decdev.h"

#include "capdebug.h"

#include "wdmvdec.h"

/*^^*
 *      CVideoDecoderDevice()
 * Purpose  : CVideoDecoderDevice class constructor
 *
 * Inputs   : PDEVICE_OBJECT pDeviceObject      : pointer to the Driver object to access the Registry
 *
 * Outputs  : none
 * Author   : IKLEBANOV
 *^^*/

CVideoDecoderDevice::CVideoDecoderDevice()
        : m_pDecoder(NULL),
          m_bOutputEnabledSet(FALSE)
{
}

CVideoDecoderDevice::~CVideoDecoderDevice()
{
}

// -------------------------------------------------------------------
// XBar Property Set functions
// -------------------------------------------------------------------

//
// The only property to set on the XBar selects the input to use
//

/* Method: CVideoDecoderDevice::SetCrossbarProperty
 * Purpose:
 */
VOID CVideoDecoderDevice::SetCrossbarProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
   PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
   ULONG Id  = pSPD->Property->Id;              // index of the property

   pSrb->Status = STATUS_SUCCESS;

   switch (Id) {
   case KSPROPERTY_CROSSBAR_ROUTE:
      ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_CROSSBAR_ROUTE_S));
      {
         PKSPROPERTY_CROSSBAR_ROUTE_S  pRoute =
            (PKSPROPERTY_CROSSBAR_ROUTE_S)pSPD->PropertyInfo;

         ULONG InPin, OutPin;
         InPin  = pRoute->IndexInputPin;
         OutPin = pRoute->IndexOutputPin;

         if (GoodPins(InPin, OutPin) && TestRoute(InPin, OutPin)) {

            SetVideoInput((Connector)InPin);

            // this just sets the association
            Route(OutPin, InPin);
         }
         else {

           pSrb->Status = STATUS_INVALID_PARAMETER;
         }
      }
      break;
   default:
      TRAP();
      pSrb->Status = STATUS_NOT_IMPLEMENTED;
      break;
   }
}

/* Method: CVideoDecoderDevice::GetCrossbarProperty
 * Purpose:
 */
VOID CVideoDecoderDevice::GetCrossbarProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
   PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
   ULONG Id = pSPD->Property->Id;              // index of the property

   pSrb->Status = STATUS_SUCCESS;

   // Property set specific structure

   switch (Id)
   {
   case KSPROPERTY_CROSSBAR_CAPS:                  // R
      ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_CROSSBAR_CAPS_S));
      {
         PKSPROPERTY_CROSSBAR_CAPS_S  pCaps =
            (PKSPROPERTY_CROSSBAR_CAPS_S)pSPD->PropertyInfo;

         // Copy the input property info to the output property info
         RtlCopyMemory(pCaps, pSPD->Property, sizeof KSPROPERTY_CROSSBAR_CAPS_S);

         pCaps->NumberOfInputs  = GetNoInputs();
         pCaps->NumberOfOutputs = GetNoOutputs();

         pSrb->ActualBytesTransferred = sizeof KSPROPERTY_CROSSBAR_CAPS_S;
      }
      break;

   case KSPROPERTY_CROSSBAR_CAN_ROUTE:
      ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_CROSSBAR_ROUTE_S));
      {
         PKSPROPERTY_CROSSBAR_ROUTE_S  pRoute =
            (PKSPROPERTY_CROSSBAR_ROUTE_S)pSPD->PropertyInfo;

         // Copy the input property info to the output property info
         RtlCopyMemory(pRoute, pSPD->Property, sizeof KSPROPERTY_CROSSBAR_ROUTE_S);

         ULONG InPin, OutPin;
         InPin  = pRoute->IndexInputPin;
         OutPin = pRoute->IndexOutputPin;

         if (GoodPins(InPin, OutPin)) {
            pRoute->CanRoute = TestRoute(InPin, OutPin);
         } else {
            pRoute->CanRoute = FALSE;
         }
         pSrb->ActualBytesTransferred = sizeof KSPROPERTY_CROSSBAR_ROUTE_S;
      }
      break;

   case KSPROPERTY_CROSSBAR_ROUTE:
      ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_CROSSBAR_ROUTE_S));
      {
         PKSPROPERTY_CROSSBAR_ROUTE_S  pRoute =
            (PKSPROPERTY_CROSSBAR_ROUTE_S)pSPD->PropertyInfo;

         // Copy the input property info to the output property info
         RtlCopyMemory(pRoute, pSPD->Property, sizeof KSPROPERTY_CROSSBAR_ROUTE_S);

         ULONG OutPin = pRoute->IndexOutputPin;

         if (OutPin < GetNoOutputs())
            pRoute->IndexInputPin = GetRoute(OutPin);
         else
            pRoute->IndexInputPin = (ULONG)-1;

         pSrb->ActualBytesTransferred = sizeof KSPROPERTY_CROSSBAR_ROUTE_S;
      }
      break;

   case KSPROPERTY_CROSSBAR_PININFO:                     // R
      ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_CROSSBAR_PININFO_S));
      {
         PKSPROPERTY_CROSSBAR_PININFO_S  pPinInfo =
            (PKSPROPERTY_CROSSBAR_PININFO_S)pSPD->PropertyInfo;

         // Copy the input property info to the output property info
         RtlCopyMemory(pPinInfo, pSPD->Property, sizeof KSPROPERTY_CROSSBAR_PININFO_S);

         if (pPinInfo->Direction == KSPIN_DATAFLOW_IN) {

            if (pPinInfo->Index >= GetNoInputs()) {

               pSrb->Status = STATUS_INVALID_PARAMETER;
               break;
            }
         }
         else
         if (pPinInfo->Direction == KSPIN_DATAFLOW_OUT) {

            if (pPinInfo->Index >= GetNoOutputs()) {

               pSrb->Status = STATUS_INVALID_PARAMETER;
               break;
            }
         }
         else {

            pSrb->Status = STATUS_INVALID_PARAMETER;
            break;
         }

         pPinInfo->PinType = GetPinInfo(pPinInfo->Direction,
            pPinInfo->Index, 
            pPinInfo->RelatedPinIndex);

         pPinInfo->Medium = * GetPinMedium(pPinInfo->Direction,
            pPinInfo->Index);

         pSrb->ActualBytesTransferred = sizeof KSPROPERTY_CROSSBAR_PININFO_S;
      }
      break;

   default:
      TRAP();
      pSrb->Status = STATUS_NOT_IMPLEMENTED;
      break;
   }
}

// -------------------------------------------------------------------
// Decoder functions
// -------------------------------------------------------------------

/*
** CVideoDecoderDevice::SetDecoderProperty ()
**
**    Handles Set operations on the Decoder property set.
**
** Arguments:
**
**      pSRB -
**          Pointer to the HW_STREAM_REQUEST_BLOCK 
**
** Returns:
**
** Side Effects:  none
*/

VOID CVideoDecoderDevice::SetDecoderProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;              // index of the property

    switch (Id)
    {
    case KSPROPERTY_VIDEODECODER_STANDARD:
        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEODECODER_S));
        {
            PKSPROPERTY_VIDEODECODER_S pS = (PKSPROPERTY_VIDEODECODER_S) pSPD->PropertyInfo;

            DBGTRACE(("KSPROPERTY_VIDEODECODER_STANDARD.\n"));

            if (!SetVideoDecoderStandard(pS->Value))
            {
                DBGERROR(("Unsupported video standard.\n"));
                pSrb->Status = STATUS_UNSUCCESSFUL;
            }
        }
        break;

    case KSPROPERTY_VIDEODECODER_OUTPUT_ENABLE:
        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEODECODER_S));
        {
            PKSPROPERTY_VIDEODECODER_S pS = (PKSPROPERTY_VIDEODECODER_S) pSPD->PropertyInfo;

            DBGTRACE(("KSPROPERTY_VIDEODECODER_OUTPUT_ENABLE.\n"));

            // Should we leave this property as it was and add a new
            // property that supports the new behavior? 

            // We probably should allow this if the filter is stopped because
            // the transition to Acquire/Pause/Run will fail if the
            // PreEvent has not been cleared by then. We'll have to add
            // some logic to this class to track the filter's state.

            if (pS->Value && m_pDecoder && m_pDecoder->PreEventOccurred())
            {
                DBGERROR(("Output enabled when preevent has occurred.\n"));
                pSrb->Status = STATUS_UNSUCCESSFUL;
            }
            else
            {
                SetOutputEnabled(pS->Value);
                SetOutputEnabledOverridden(TRUE);
            }
        }
        break;

    default:
        TRAP();
        break;
    }
}

/*
** CVideoDecoderDevice::GetDecoderProperty ()
**
**    Handles Get operations on the Decoder property set.
**
** Arguments:
**
**      pSRB -
**          Pointer to the HW_STREAM_REQUEST_BLOCK 
**
** Returns:
**
** Side Effects:  none
*/

VOID CVideoDecoderDevice::GetDecoderProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;              // index of the property

    switch (Id)
    {
    case KSPROPERTY_VIDEODECODER_CAPS:
        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEODECODER_CAPS_S));
        {
            DBGTRACE(("KSPROPERTY_VIDEODECODER_CAPS\n"));

            PKSPROPERTY_VIDEODECODER_CAPS_S  pCaps =
                (PKSPROPERTY_VIDEODECODER_CAPS_S)pSPD->PropertyInfo;

            // Copy the input property info to the output property info
            RtlCopyMemory(pCaps, pSPD->Property, sizeof KSPROPERTY);

            GetVideoDecoderCaps(pCaps);

            pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEODECODER_CAPS_S);
        }
        break;

    case KSPROPERTY_VIDEODECODER_STANDARD:
        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEODECODER_S));
        {
            DBGTRACE(("KSPROPERTY_VIDEODECODER_STANDARD\n"));

            PKSPROPERTY_VIDEODECODER_S  pS =
                (PKSPROPERTY_VIDEODECODER_S)pSPD->PropertyInfo;

            // Copy the input property info to the output property info
            RtlCopyMemory(pS, pSPD->Property, sizeof KSPROPERTY);

            pS->Value = GetVideoDecoderStandard();

            pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEODECODER_S);
        }
        break;

    case KSPROPERTY_VIDEODECODER_STATUS:
        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEODECODER_STATUS_S));
        {
            DBGTRACE(("KSPROPERTY_VIDEODECODER_STATUS\n"));

            PKSPROPERTY_VIDEODECODER_STATUS_S  pS =
                (PKSPROPERTY_VIDEODECODER_STATUS_S)pSPD->PropertyInfo;

            // Copy the input property info to the output property info
            RtlCopyMemory(pS, pSPD->Property, sizeof KSPROPERTY);

            GetVideoDecoderStatus(pS);

            pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEODECODER_STATUS_S);
        }
        break;

    case KSPROPERTY_VIDEODECODER_OUTPUT_ENABLE:
        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEODECODER_S));
        {
            DBGTRACE(("KSPROPERTY_VIDEODECODER_OUTPUT_ENABLE\n"));

            PKSPROPERTY_VIDEODECODER_S pS = (PKSPROPERTY_VIDEODECODER_S) pSPD->PropertyInfo;    // pointer to the data

            // Copy the input property info to the output property info
            RtlCopyMemory(pS, pSPD->Property, sizeof KSPROPERTY);

            pS->Value = IsOutputEnabled();

            pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEODECODER_S);
        }
        break;

    default:
        TRAP();
        break;
    }
}

