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

#include "34AVStrm.h"
#include "SynchIrpCall.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  global function.
//  This function uses the context to call the right instance of the
//  IRP completion function. The expected return value is
//  STATUS_MORE_PROCESSING_REQUIRED.
//
// Return Value:
//  STATUS_MORE_PROCESSING_REQUIRED   Value of "OnCompletion" function call.
//  STATUS_UNSUCCESSFUL               Parameters invalid.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS HandleCompletion
(
    IN PDEVICE_OBJECT pDevice, //Pointer to the device related to IRP.
    IN PIRP           pIrp,    //Pointer to IRP, which will be completed.
    IN PVOID          pContext //Pointer to instance of CSyncIrpCall class
                               //related to IRP.
)
{
    // this function is static so figure out the right instance
    CSyncIrpCall* pThis = reinterpret_cast<CSyncIrpCall*>(pContext);
    // call the completion function of the instance
    return pThis->OnCompletion(pDevice, pIrp);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor
//
//////////////////////////////////////////////////////////////////////////////
CSyncIrpCall::CSyncIrpCall()
{
    m_sCall = STATUS_UNSUCCESSFUL;
    m_cInformation = 0;
    m_bCompleted = false;

    KeInitializeEvent(&m_Event, NotificationEvent, false);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function uses the passed IRP and sends it to the driver, specified
//  by the passed device object. It also handles the completion of the IRP.
//
// Return Value:
//  STATUS_SUCCESS         Irp was handled successfully.
//  NTSTATUS-Error         returned by IRP processing.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CSyncIrpCall::Call
(
    IN PDEVICE_OBJECT pTarget,//Pointer to the device related to IRP.
    IN PIRP           pIrp //Pointer to IRP, which will be handled.
)
{
    KeClearEvent(&m_Event);
    m_bCompleted = false;
    NTSTATUS     status = STATUS_SUCCESS;
    BOOLEAN     bValueTrue = true;

    IoSetCompletionRoutine(pIrp, HandleCompletion, this,
                            bValueTrue, bValueTrue, bValueTrue);

    // reset status
    pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    // retrieve status
    status = IoCallDriver(pTarget, pIrp);

    if ( !NT_SUCCESS(status) )
    {
        // the return is either set by a lower driver
        // (in the I/O status block) for the given request
        // or STATUS_PENDING if the request was queued for additional processing.
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: IoCallDriver not successful"));
        return STATUS_UNSUCCESSFUL;
    }

    if( status == STATUS_PENDING )
    {
        m_sCall = status;
        _DbgPrintF(DEBUGLVL_BLAB,("Info: STATUS PENDING"));
    }

    status = KeWaitForSingleObject(&m_Event,
                                   Executive,
                                   (static_cast <char> (KernelMode)),
                                   FALSE,
                                   0);
    if ( !NT_SUCCESS(status) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: KeWaitForSingleObject not successful"));
        return STATUS_UNSUCCESSFUL;
    }
    // m_sCall contains the status value returned by IRP IoStatus.Status
    return m_sCall;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function is called on a completion of an IRP.
//  All information of the IRP are saved in member variables.
//
// Return Value:
//  STATUS_MORE_PROCESSING_REQUIRED   Successfully completed.
//  STATUS_UNSUCCESSFUL               Parameter not valid.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CSyncIrpCall::OnCompletion
(
    IN PDEVICE_OBJECT pDevice, //Pointer to the device related to IRP.(unused)
    IN PIRP           pIrp //Pointer to IRP, which will be completed.
)
{
    // sometimes we are called several times -- only the first time seems valid
    if( !m_bCompleted )
    {
        m_bCompleted = true;
        m_sCall = pIrp->IoStatus.Status;
        m_cInformation = pIrp->IoStatus.Information;
    }

    long lReturn = KeSetEvent(&m_Event, IO_NO_INCREMENT, false);
    if( lReturn != 0 )
    {
        _DbgPrintF(DEBUGLVL_BLAB,
            ("Info: Previous state of the event object was signaled"));
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}
