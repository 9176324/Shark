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

#include "BasePinInterface.h"
#include "BaseStream.h"
#include "AnlgEventHandler.h"


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the remove dispatch function of the base stream pin.
//  It retrieves the pointer of an base pin derived pin from the pin context.
//  It also calls remove on the instance of the base pin derived pin.
//
//  Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) if the pin pointer is zero
//                          (c) remove failed
//  STATUS_SUCCESS          Removed an base pin derived pin with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS BasePinRemove
(
    IN PKSPIN pKSPin,   //KS pin, used for system support function calls.
    IN PIRP pIrp        //Pointer to IRP for this request.
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: BasePinRemove called"));
    //parameters valid?
    if( !pKSPin || !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    //get the target class object out of the KS pin context
    CBaseStream* pBaseStreamPin =
        static_cast <CBaseStream*> (pKSPin->Context);
    //class object found?
    if( !pBaseStreamPin )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: No pin class object found"));
        return STATUS_UNSUCCESSFUL;
    }
    //call class function
    Status = pBaseStreamPin->Remove(pIrp);

    //de-allocate memory for our class object even if remove failed
    delete pBaseStreamPin;
    pBaseStreamPin = NULL;
    //remove the class object from the context structure of the KS pin
    pKSPin->Context = NULL;

    if( Status != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Class function called without success"));
        return Status;
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the process dispatch function of the base stream pin.
//  It retrieves the pointer of an base pin derived pin from the pin context.
//  It also calls process on the instance of the base pin derived pin.
//
//  Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) if the pin pointer is zero
//                          (c) Process() failed
//  STATUS_SUCCESS          Process an base pin derived pin with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS BasePinProcess
(
    IN PKSPIN pKSPin    //KS pin, used for system support function calls.
)
{
    //parameters valid?
    if( !pKSPin )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    //get the target class object out of the KS pin context
    CBaseStream* pBaseStreamPin =
        static_cast <CBaseStream*> (pKSPin->Context);
    //class object found?
    if( !pBaseStreamPin )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: No stream pin class object found"));
        return STATUS_UNSUCCESSFUL;
    }
    //call class function
    return pBaseStreamPin->Process();
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the SetDeviceState dispatch function of the base stream pin.
//  It retrieves the pointer of an base pin derived pin from the pin context.
//  It also calls the setting of the device state on the instance of the base
//  pin derived pin depending on the ToState and the FromState.
// Settings:
//  All combinations and the appriopriate out pin calls (to the stream) are
//  listed below.
//
//  ToState             FromState           Stream call
//  KSSTATE_ACQUIRE     KSSTATE_RUN         Stop()
//  KSSTATE_ACQUIRE     KSSTATE_PAUSE       nothing
//  KSSTATE_ACQUIRE     KSSTATE_STOP        Open()
//  KSSTATE_ACQUIRE     KSSTATE_ACQUIRE     nothing
//  KSSTATE_RUN         KSSTATE_ACQUIRE     Start()
//  KSSTATE_RUN         KSSTATE_PAUSE       Start()
//  KSSTATE_RUN         KSSTATE_STOP        Open() and Start()
//  KSSTATE_RUN         KSSTATE_RUN         nothing
//  KSSTATE_PAUSE       KSSTATE_ACQUIRE     nothing
//  KSSTATE_PAUSE       KSSTATE_RUN         Stop()
//  KSSTATE_PAUSE       KSSTATE_STOP        Open()
//  KSSTATE_PAUSE       KSSTATE_PAUSE       nothing
//  KSSTATE_STOP        KSSTATE_ACQUIRE     Close()
//  KSSTATE_STOP        KSSTATE_RUN         Stop() and Close()
//  KSSTATE_STOP        KSSTATE_PAUSE       Close()
//  KSSTATE_STOP        KSSTATE_STOP        nothing
//
// Return Value:
//  STATUS_UNSUCCESSFUL    Operation failed,
//                         (a) if the function parameters are zero,
//                         (b) if the  pin pointer is zero
//                         (c) if 'Stream call' failed
//  STATUS_SUCCESS         SetDeviceState on an base pin derived pin with
//                         success.
//                         (a) if ToState and FromState are identical
//                         (b) if the 'Stream call' succeeded
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS BasePinSetDeviceState
(
    IN PKSPIN pKSPin,           // Pointer to KSPIN.
    IN KSSTATE ToState,         // State that has to be active after this call.
    IN KSSTATE FromState        // State that was active before this call.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: BasePinSetDeviceState called"));
    _DbgPrintF(DEBUGLVL_BLAB,
        ("Info: From state 0x%02X to state 0x%02X", FromState, ToState));
    //parameters valid?
    if( !pKSPin )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    //get the target class object out of the KS pin context
    CBaseStream* pBaseStreamPin =
        static_cast <CBaseStream*> (pKSPin->Context);
    //class object found?
    if( !pBaseStreamPin )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: No stream pin class object found"));
        return STATUS_UNSUCCESSFUL;
    }

    // get the KS device object out of the KS pin object
    // (Device where the pin is part of)
    PKSDEVICE pKSDevice = KsGetDevice(pKSPin);
    if( !pKSDevice )
    {
        _DbgPrintF( DEBUGLVL_ERROR,("Error: Cannot get connection to KS device"));
        return NULL;
    }
    //get the VampDevice object out of the Context of KSDevice
    CVampDevice* pVampDevice = static_cast <CVampDevice*> (pKSDevice->Context);

    if (!pVampDevice)
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error: cannot get connection to device object\n"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgEventHandler* pEventHandler = pBaseStreamPin->GetEventHandler();
    if( !pEventHandler )
    {
        _DbgPrintF( DEBUGLVL_VERBOSE,
                    ("Warning: No event handler class object found"));
    }

    NTSTATUS status = STATUS_SUCCESS;
    //call class function due to state transition
    switch(ToState)
    {
        case KSSTATE_ACQUIRE:
            _DbgPrintF(DEBUGLVL_BLAB,
                ("Info: KSSTATE_ACQUIRE base stream called"));
            switch(FromState)
            {
            case KSSTATE_RUN:
                if( pEventHandler )
                {
                    status = pEventHandler->DeRegister(pKSPin);
                    if( status != STATUS_SUCCESS )
                    {
                        _DbgPrintF(DEBUGLVL_ERROR,("Error: DeRegister failed"));
                        break;
                    }
                }
                //stream is running, stop it
                status = pBaseStreamPin->Stop();
                if( status != STATUS_SUCCESS )
                {
                    _DbgPrintF(DEBUGLVL_ERROR,("Error: Stop failed"));
                    break;
                }
                break;
            case KSSTATE_PAUSE:
                //stream is opened/stopped, do nothing
                break;
            case KSSTATE_STOP:

                // this works for single tuner devices. for multi-tuner devices, one would
                // need a separate owner for each tuner
                status = pVampDevice->SetOwnerProcessHandle(PsGetCurrentProcess());
                if (status != STATUS_SUCCESS) break;

                //stream is closed, aquire it
                status = pBaseStreamPin->Open();
                if( status != STATUS_SUCCESS )
                {
                    pVampDevice->SetOwnerProcessHandle(NULL);
                    _DbgPrintF(DEBUGLVL_ERROR,("Error: Open failed"));
                    break;
                }
                break;
            default:
                _DbgPrintF(DEBUGLVL_ERROR,
                    ("Error: Invalid state transition"));
                status = STATUS_UNSUCCESSFUL;
                break;
            }
            break;
        case KSSTATE_RUN:
            _DbgPrintF( DEBUGLVL_BLAB,
                        ("Info: KSSTATE_RUN base stream called"));
            switch(FromState)
            {
            case KSSTATE_ACQUIRE:
                //stream is open, start it
                if( pEventHandler )
                {
                    status = pEventHandler->Register(pKSPin);
                    if( status != STATUS_SUCCESS )
                    {
                        _DbgPrintF(DEBUGLVL_ERROR,("Error: Register failed"));
                        break;
                    }
                }
                status = pBaseStreamPin->Start();
                if( status != STATUS_SUCCESS )
                {
                    _DbgPrintF(DEBUGLVL_ERROR,("Error: Start failed"));
                    break;
                }
                break;
            case KSSTATE_PAUSE:
                //stream is opened/stopped, start it
                if( pEventHandler )
                {
                    status = pEventHandler->Register(pKSPin);
                    if( status != STATUS_SUCCESS )
                    {
                        _DbgPrintF(DEBUGLVL_ERROR,("Error: Register failed"));
                        break;
                    }
                }
                status = pBaseStreamPin->Start();
                if( status != STATUS_SUCCESS )
                {
                    _DbgPrintF(DEBUGLVL_ERROR,("Error: Start failed"));
                    break;
                }
                break;
            case KSSTATE_STOP:
                //stream is closed, open and start it
                status = pBaseStreamPin->Open();
                if( status != STATUS_SUCCESS )
                {
                    _DbgPrintF(DEBUGLVL_ERROR,("Error: open failed"));
                    break;
                }
                if( pEventHandler )
                {
                    status = pEventHandler->Register(pKSPin);
                    if( status != STATUS_SUCCESS )
                    {
                        _DbgPrintF(DEBUGLVL_ERROR,("Error: Register failed"));
                        break;
                    }
                }
                status = pBaseStreamPin->Start();
                if( status != STATUS_SUCCESS )
                {
                    _DbgPrintF(DEBUGLVL_ERROR,("Error: Start failed"));
                    break;
                }
                break;
            default:
                _DbgPrintF(DEBUGLVL_ERROR,
                    ("Error: Invalid state transition"));
                status = STATUS_UNSUCCESSFUL;
                break;
            }
            break;
        case KSSTATE_PAUSE:
            _DbgPrintF( DEBUGLVL_BLAB,
                ("Info: KSSTATE_PAUSE base stream called"));
            switch(FromState)
            {
            case KSSTATE_ACQUIRE:
                //stream is open, do nothing
                break;
            case KSSTATE_RUN:
                if( pEventHandler )
                {
                    status = pEventHandler->DeRegister(pKSPin);
                    if( status != STATUS_SUCCESS )
                    {
                        _DbgPrintF(DEBUGLVL_ERROR,("Error: DeRegister failed"));
                        break;
                    }
                }
                //stream is opened/started, stop it
                status = pBaseStreamPin->Stop();
                if( status != STATUS_SUCCESS )
                {
                    _DbgPrintF(DEBUGLVL_ERROR,("Error: Stop failed"));
                    break;
                }
                break;
            case KSSTATE_STOP:
                //stream is closed, open it
                status = pBaseStreamPin->Open();
                if( status != STATUS_SUCCESS )
                {
                    _DbgPrintF(DEBUGLVL_ERROR,("Error: Open failed"));
                    break;
                }
                break;
            default:
                _DbgPrintF(DEBUGLVL_ERROR,
                    ("Error: Invaild state transition"));
                status = STATUS_UNSUCCESSFUL;
                break;
            }
            break;
        case KSSTATE_STOP:
            _DbgPrintF( DEBUGLVL_BLAB,
                ("Info: KSSTATE_STOP base stream called"));
            pVampDevice->SetOwnerProcessHandle(NULL);

            switch(FromState)
            {
            case KSSTATE_ACQUIRE:                

                //stream is open, close it
                status = pBaseStreamPin->Close();
                if( status != STATUS_SUCCESS )
                {
                    _DbgPrintF(DEBUGLVL_ERROR,("Error: Close failed"));
                    break;
                }
                break;
            case KSSTATE_RUN:
                if( pEventHandler )
                {
                    status = pEventHandler->DeRegister(pKSPin);
                    if( status != STATUS_SUCCESS )
                    {
                        _DbgPrintF(DEBUGLVL_ERROR,("Error: DeRegister failed"));
                        break;
                    }
                }
                //stream is opened/started, stop and close it
                status = pBaseStreamPin->Stop();
                if( status != STATUS_SUCCESS )
                {
                    _DbgPrintF(DEBUGLVL_ERROR,("Error: Stop failed"));
                    break;
                }
                status = pBaseStreamPin->Close();
                if( status != STATUS_SUCCESS )
                {
                    _DbgPrintF(DEBUGLVL_ERROR,("Error: Close failed"));
                    break;
                }
                break;
            case KSSTATE_PAUSE:
                //stream is opened, close it
                status = pBaseStreamPin->Close();
                if( status != STATUS_SUCCESS )
                {
                    _DbgPrintF(DEBUGLVL_ERROR,("Error: Close failed"));
                    break;
                }
                break;
            default:
                _DbgPrintF(DEBUGLVL_ERROR,
                    ("Error: Invalid state transition"));
                status = STATUS_UNSUCCESSFUL;
                break;
            }
            break;
        default:
            _DbgPrintF(DEBUGLVL_ERROR,("Error: invalid state transition"));
            status = STATUS_UNSUCCESSFUL;
            break;
    }
    return status;
}
