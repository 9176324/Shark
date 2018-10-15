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
#include "AnlgEventHandler.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function is the entry point for the event handling thread.
//  It will be called from the PsCreateSystemThread within the constructor
//  of the EventHandler Class.
//  The notification callback function from the VampDevice is always called
//  at DISPATCH_LEVEL. But to process following action due to a notification
//  it's necessary to run at PASSIVE_LEVEL. Therefore we need a thread that
//  will be informed about events and can then evaluate them at the required
//  passive level.
//
// Remarks:
//  This function may be never called directly. Only the implicit use with
//  the thread creation is valid.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
void EventHandleSystemThread
(
    IN PVOID pContext
)
{
    _DbgPrintF(DEBUGLVL_BLAB,
        ("Info: Create called (EventHandleSystemThread)"));
    // check if context of the Thread is valid
    if( !pContext )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
        ("Error: Invalid Context (EventHandleSystemThread)"));
        //a timeout on the other side should handle this error
        return;
    }
    // create pointer to an event handler from the context
    CAnlgEventHandler* pHandler =
                static_cast <CAnlgEventHandler*> (pContext);


    // signal that the tread is started
    // valid to be called in DISPATCH_LEVEL
    // (<= DISPATCH_LEVEL if WAIT is zero)
    if( KeSetEvent(&pHandler->m_kevEventThreadStarted,
                   KSEVENT_TYPE_ONESHOT,
                   FALSE) != 0 )
    {
        _DbgPrintF(DEBUGLVL_BLAB,
            ("Info: Previous state was signaled (EventHandleSystemThread)"));
    }

    // create array for event pointers to be used in KeWaitForMultipleObjects
    KEVENT* pEvents[2];
    // store event pointers (possible events waiting for)
    pEvents[STANDARD_CHANGE_EVENT] = &pHandler->m_kevAnlgVideoChangeDetected;
    pEvents[TERMINATE_THREAD_EVENT] = &pHandler->m_kevTerminateEventThread;

    NTSTATUS status = STATUS_SUCCESS;
    // this variable is used to get rid of the PCLint info for the
    // while(1) loop, in this case the infinite loop is used on purpose
    // and for better readability of the event handling
    BOOL bPCLint = TRUE;
    // infinite loop: "while(1)", will run until "TerminateEvendThread"
    // event occures
    while( bPCLint )
    {
        // wait until any above stored Event occures
        // valid to be called in <= DISPATCH_LEVEL
        status = KeWaitForMultipleObjects(2,
                                          reinterpret_cast<PVOID*>(&pEvents[0]),
                                          WaitAny,
                                          Executive,
                                          static_cast<char> (KernelMode),
                                          false,
                                          NULL,
                                          NULL);
        if( !NT_SUCCESS(status) )
        {
            _DbgPrintF(DEBUGLVL_ERROR,
                ("Error: WaitForMultiple failed (EventHandleSystemThread)"));
            break;
        }
        // status contains the current event that was fired, we use this
        // way instead of calling KeReadStateEvent, because this function
        // is not available in W98SE
        switch( status )
        {
        case STANDARD_CHANGE_EVENT:
            // valid to be called in <= DISPATCH_LEVEL
            // clear the event (void function)
            KeClearEvent(pEvents[STANDARD_CHANGE_EVENT]);
            status = pHandler->Process();
            if( !NT_SUCCESS(status) )
            {
                _DbgPrintF(DEBUGLVL_ERROR,
                    ("Error: Process failed (EventHandleSystemThread)"));
            }
            break;
        case TERMINATE_THREAD_EVENT:
            // valid to be called in <= DISPATCH_LEVEL
            _DbgPrintF(DEBUGLVL_BLAB,
                ("Info: Terminate called (EventHandleSystemThread)"));
            // clear the event (void function)
            KeClearEvent(pEvents[TERMINATE_THREAD_EVENT]);
            // signal that the thread is ready to terminate
            // valid to be called in DISPATCH_LEVEL
            // (<= DISPATCH_LEVEL if WAIT is zero)
            // no member access after this point!!!
            if( KeSetEvent(&pHandler->m_kevEventThreadTerminated,
                           KSEVENT_TYPE_ONESHOT,
                           FALSE) != 0 )
            {
                _DbgPrintF(DEBUGLVL_ERROR,
                    ("Error: Signaled already (EventHandleSystemThread)"));
            }
            // terminate the thread
            status = PsTerminateSystemThread(STATUS_SUCCESS);
            // no code is called after this point, the following lines are
            // only for the compiler and PCLint !!!
            if( !NT_SUCCESS(status) )
            {
               _DbgPrintF(DEBUGLVL_ERROR,
                   ("Error: Termination failed (EventHandleSystemThread)"));
            }
            return;
        default:
            _DbgPrintF(DEBUGLVL_ERROR,
                ("Error: Invalid status of wait (EventHandleSystemThread)"));
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor of the analog event handler.

// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgEventHandler::CAnlgEventHandler()
{
    m_pKSFilter    = NULL;
    m_pEventHandle = NULL;

    // initialize events
    // have to be called in PASSIVE_LEVEL only
    KeInitializeEvent(  &m_kevAnlgVideoChangeDetected,
                        NotificationEvent,
                        FALSE);
    KeInitializeEvent(  &m_kevTerminateEventThread,
                        NotificationEvent,
                        FALSE);
    KeInitializeEvent(  &m_kevEventThreadStarted,
                        NotificationEvent,
                        FALSE);
    KeInitializeEvent(  &m_kevEventThreadTerminated,
                        NotificationEvent,
                        FALSE);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor of the analog event handler.
//  The destructor sends an event to terminate the related thread
//  and waits until tread is terminated.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgEventHandler::~CAnlgEventHandler()
{
    NTSTATUS status;

    // event notification is still active
    // before destroying object remove notification
    if( m_pEventHandle )
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,
            ("Warning: Event notification was not removed"));

        status = RemoveNotify();
        if( !NT_SUCCESS(status) )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: RemoveNotify failed!"));
        }
        m_pEventHandle = NULL;
    }
    // terminate the thread before destroying the class object.
    if( KeSetEvent(&m_kevTerminateEventThread, KSEVENT_TYPE_ONESHOT, FALSE) )
    {
        _DbgPrintF(DEBUGLVL_BLAB,
            ("Info: CAnlgEventHandler: previous state was signaled"));
    }
    LARGE_INTEGER liTimeOut;
    liTimeOut.QuadPart = -100000000; //wait no longer than 10 sec
    // synchronisation: thread must be terminated before leaving destructor
    // because thread accesses class members!!
    status = KeWaitForSingleObject( &m_kevEventThreadTerminated,
                                    Executive,
                                    static_cast <char> (KernelMode),
                                    FALSE,
                                    &liTimeOut);
    if ( !NT_SUCCESS(status) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Could not terminate thread (CAnlgEventHandler)"));
    }
    //timeout is part of the NT_SUCCESS macro, so we ask for it separately
    if( status == STATUS_TIMEOUT )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Timeout, could not terminate thread (CAnlgEventHandler)"));
    }
    KeClearEvent(&m_kevEventThreadTerminated); //void function
    m_pKSFilter = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Initializes the analog event handler.
//  This function sets the parent filter pointer, creates and starts the
//  event handler thread and initializes the necessary events.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameter is zero
//                          (b) if the thread creation fails
//  STATUS_SUCCESS          Initialized with success.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgEventHandler::Initialize
(
    PKSFILTER pKSFilter
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgEventHandler::Initialize"));

    // check for valid parent filter
    if( !pKSFilter )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    // the parent filter this event handler is part of.
    m_pKSFilter = pKSFilter;

    HANDLE hThreadHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;

    InitializeObjectAttributes(&ObjectAttributes,
        NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

    // create system thread for event handling
    if( PsCreateSystemThread(&hThreadHandle, THREAD_ALL_ACCESS,
        &ObjectAttributes, 0, 0, EventHandleSystemThread, this) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Could not create Thread for event handling"));
        return STATUS_UNSUCCESSFUL;
    }
    LARGE_INTEGER liTimeOut;
    liTimeOut.QuadPart = -100000000; //wait no longer than 10 sec
    // valid to be called in DISPATCH_LEVEL only (if timeout is zero)
    NTSTATUS status = KeWaitForSingleObject(&m_kevEventThreadStarted,
                                            Executive,
                                            static_cast <char> (KernelMode),
                                            FALSE,
                                            &liTimeOut);
    if( !NT_SUCCESS(status) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Could not start Thread for event handling"));
        return STATUS_UNSUCCESSFUL;
    }
    //timeout is part of the NT_SUCCESS macro, so we ask for it separately
    if( status == STATUS_TIMEOUT )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Timeout, could not start thread (CAnlgEventHandler)"));
    }
    // valid to be called in <= DISPATCH_LEVEL only
    KeClearEvent(&m_kevEventThreadStarted); //void function

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Registers the given Pin, to be notified in case of an "video standard
//  change detection" event.
//  This function adds the pointer to pin structure into the notify list.
//  It's only necessary to enable notification if there is a demand
//  (at least on pin in list).
//  If the list is empty with this call, additionally the event notification
//  for standard change (EN_FIDT_MODE_CHANGE) will be added to the Vamp
//  Device.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameter is zero
//                          (b) if no VampDevice object isavailable
//  STATUS_SUCCESS          Registered pin for notification with success.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgEventHandler::Register
(
    PKSPIN pKSPin
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgEventHandler::Register"));

    if( !pKSPin )
    {
        // no KSPIN struct available
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    // Notify list empty? then add event notification on standard change.
    if( !Count() )
    {
        NTSTATUS Status = AddNotify();
        if( !NT_SUCCESS(Status) )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: AddNotify failed!"));
            return STATUS_UNSUCCESSFUL;
        }
    }

    // add pin to notification list.
    InsertHead(static_cast<CBaseStream*>(pKSPin->Context));

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  DeRegisters the given Pin from notification in case of an "video standard
//  change detection" event.
//  This function removes the pointer to pin structure from the notify list.
//  It's only necessary to enable notification if there is a demand
//  (at least one pin in list).
//  If the list is empty after this call, additionally the event notification
//  for standard change (EN_FIDT_MODE_CHANGE) will be removed from the Vamp
//  Device.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameter is zero
//                          (b) if no VampDevice object isavailable
//  STATUS_SUCCESS          DeRegistered pin for notification with success.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgEventHandler::DeRegister
(
    PKSPIN pKSPin
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgEventHandler::DeRegister"));

    if( !pKSPin )
    {
        // no KSPIN struct available
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    // remove pin from notification list.
    if( !Remove(static_cast<CBaseStream*>(pKSPin->Context)) )
    {
        // no base stream object to remove
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: AnlgEventHandler: no base stream object to remove"));
        return STATUS_UNSUCCESSFUL;
    }

    // Notify list empty? then remove event notification on standard change too.
    if( !Count() )
    {
        NTSTATUS Status = RemoveNotify();
        if( !NT_SUCCESS(Status) )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: RemoveNotify failed!"));
            return STATUS_UNSUCCESSFUL;
        }
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function adds a notification request on the Vamp Device to be
//  notified if an "video standard detection change" occures. The member
//  function Notify will be registered as callback for this notification.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameter is zero
//                          (b) if no VampDevice object isavailable
//  STATUS_SUCCESS          Added notification with success.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgEventHandler::AddNotify
(
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgEventHandler::AddNotify"));

    // an event notification already exists
    if( m_pEventHandle )
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,
            ("Warning: Try to add a notification that already exist"));
        return STATUS_SUCCESS;
    }
    // get device resources object from filter
    CVampDeviceResources* pDevResObj = GetDeviceResource(m_pKSFilter);
    if( !pDevResObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }
    //add event notification on standard change
    if( pDevResObj->m_pStreamManager->AddEventNotify(
                        &m_pEventHandle,
                        EN_FIDT_MODE_CHANGE,
                        (dynamic_cast<CEventCallback*>(this)),
                        NULL,
                        NULL) != VAMPRET_SUCCESS )
    {
        m_pEventHandle = NULL;
        _DbgPrintF(DEBUGLVL_ERROR,("Error: AddEventNotify failed"));
        return STATUS_UNSUCCESSFUL;
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function removes the notification request on the Vamp Device to be
//  notified if an "video standard detection change" occures.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if no Event handle was registered
//                          (b) if no VampDevice object is available
//  STATUS_SUCCESS          Added notification with success.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgEventHandler::RemoveNotify
(
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgEventHandler::RemoveNotify"));

    if( !m_pEventHandle )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: No Event handle registered"));
        return STATUS_UNSUCCESSFUL;
    }
    // get device resources object from filter
    CVampDeviceResources* pDevResObj = GetDeviceResource(m_pKSFilter);
    if( !pDevResObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }
    //remove event notification on standard change
    if( pDevResObj->m_pStreamManager->RemoveEventNotify(m_pEventHandle) !=
        VAMPRET_SUCCESS )
    {
        m_pEventHandle = NULL;
        _DbgPrintF(DEBUGLVL_ERROR,("Error: RemoveEventNotify failed"));
        return STATUS_UNSUCCESSFUL;
    }
    m_pEventHandle = NULL;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Event handling routine for standard change. (50/60Hz)
//  Checks for the right notification,
//  then fires the "kevAnlgVideoChangeDetected" (will be evaluated by event
//  handle thread).
//
// Remarks:
//  Due to the fact that callbacks running at DISPATCH-LEVEL. it's not
//  possible to call an evaluate the proccess function from here immediately.

//  Return Value:
//   STATUS_UNSUCCESSFUL        Operation failed, wrong event type,
//   STATUS_SUCCESS             streams are restarted with success
//
//////////////////////////////////////////////////////////////////////////////
VAMPRET CAnlgEventHandler::Notify
(
    PVOID pArgument1,
    PVOID pArgument2,
    eEventType tEvent
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgEventHandler::Notify"));

    // only interested on "video standard change detection" event
    if( tEvent != EN_FIDT_MODE_CHANGE )
    {
         _DbgPrintF(DEBUGLVL_ERROR,
             ("Error: CAnlgEventHandler: wrong event type!"));

         return STATUS_UNSUCCESSFUL;
    }

    CBaseStream* pBaseStreamObj = Head();

    // At least one Pin to process in list
    if( pBaseStreamObj )
    {
        // fire event (will be evaluated by event handle thread)
        // valid to be called in DISPATCH_LEVEL
        // (<= DISPATCH_LEVEL if WAIT is zero)
        if( KeSetEvent(&m_kevAnlgVideoChangeDetected,
                       KSEVENT_TYPE_ONESHOT, FALSE) )
        {
            _DbgPrintF(DEBUGLVL_BLAB,
                ("Info: CAnlgEventHandler: previous state was signaled!"));
        }
    }
    else
    {
         _DbgPrintF(DEBUGLVL_BLAB,
             ("Info: CAnlgEventHandler: notify list is empty!"));
    }

    return VAMPRET_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Processing routine for standard change depending actions.
//  All active pins within the notify list will be treated.
//  The pins must be stoped and restarted again, to catch the new settings.
//
//  Return Value:
//   STATUS_UNSUCCESSFUL        Operation failed,
//                              (a) No active pins available
//                              (b) No pin structure available
//   STATUS_SUCCESS             Streams are restarted with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgEventHandler::Process
(
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgEventHandler::Process"));

    // Get head item from list
    CBaseStream* pBaseStream = Head();

    // Stop and Close all acitive pins
    while(pBaseStream)
    {
        PKSPIN pKSPin = pBaseStream->GetKSPin();
        if( !pKSPin )
        {
            _DbgPrintF(DEBUGLVL_ERROR,
                ("Error: No KSPin found (AnlgEventHandler::Process)"));
            return STATUS_UNSUCCESSFUL;
        }
        PKSGATE pKSGate = NULL;
        pKSGate = KsPinGetAndGate(pKSPin);
        if( !pKSGate )
        {
            _DbgPrintF(DEBUGLVL_ERROR,
                ("Error: No KSGate found (AnlgEventHandler::Process)"));
            return STATUS_UNSUCCESSFUL;
        }
        KsGateTurnInputOff(pKSGate);

        if( pKSPin->DeviceState == KSSTATE_RUN )
        {
            if( pBaseStream->Stop()  != STATUS_SUCCESS)
            {
                _DbgPrintF(DEBUGLVL_VERBOSE,
                    ("Warning: Could not Stop Pin (AnlgEventHandler::Process)"));
            }

            if( pBaseStream->Close()  != STATUS_SUCCESS)
            {
                _DbgPrintF(DEBUGLVL_VERBOSE,
                    ("Warning: Could not Close Pin (AnlgEventHandler::Process)"));
            }
        }

        if( (pKSPin->DeviceState == KSSTATE_ACQUIRE) ||
            (pKSPin->DeviceState == KSSTATE_PAUSE) )
        {
            if( pBaseStream->Close()  != STATUS_SUCCESS)
            {
                _DbgPrintF(DEBUGLVL_VERBOSE,
                    ("Warning: Could not Close Pin (AnlgEventHandler::Process)"));
            }
        }
        // get next object from list
        pBaseStream = Next(pBaseStream);
    }

    // Get head item from list
    pBaseStream = Head();

    // Open and Start all closed pins again
    while(pBaseStream)
    {
        PKSPIN pKSPin = pBaseStream->GetKSPin();
        if( !pKSPin )
        {
            _DbgPrintF(DEBUGLVL_ERROR,
                ("Error: No KSPin found (AnlgEventHandler::Process)"));
            return STATUS_UNSUCCESSFUL;

        }

        if( (pKSPin->DeviceState == KSSTATE_ACQUIRE) ||
            (pKSPin->DeviceState == KSSTATE_PAUSE) )
        {
            if( pBaseStream->Open()  != STATUS_SUCCESS)
            {
                _DbgPrintF(DEBUGLVL_VERBOSE,
                    ("Warning: Could not Open Pin (AnlgEventHandler::Process)"));
            }
        }

        if( pKSPin->DeviceState == KSSTATE_RUN )
        {
            if( pBaseStream->Open()  != STATUS_SUCCESS)
            {
                _DbgPrintF(DEBUGLVL_VERBOSE,
                    ("Warning: Could not Open Pin (AnlgEventHandler::Process)"));
            }

            if( pBaseStream->Start() != STATUS_SUCCESS)
            {
                _DbgPrintF(DEBUGLVL_VERBOSE,
                    ("Warning: Could not Start Pin (AnlgEventHandler::Process)"));
            }
        }

        PKSGATE pKSGate = NULL;
        pKSGate = KsPinGetAndGate(pKSPin);
        if( !pKSGate )
        {
            _DbgPrintF(DEBUGLVL_ERROR,
                ("Error: No KSGate found (AnlgEventHandler::Process)"));
            return STATUS_UNSUCCESSFUL;
        }
        //this function has no return value, so we assume it always succeeds
        KsGateTurnInputOn(pKSGate);
        KsPinAttemptProcessing(pKSPin, TRUE);
        // get next object from list
        pBaseStream = Next(pBaseStream);
    }

    return STATUS_SUCCESS;
}
