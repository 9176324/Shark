/*
 * NT_com.c - Windows NT serial interface
 *
 * This module handles serial input and output to Windows NT.
 *
 * copyright 1992, 1993 by Microsoft Corporation
 * portions copyright 1991 by Insignia Solutions Ltd., used by permission.
 *
 * revision history:
 *  24-Dec-1992 John Morgan:  written
 *              based (in part) on serial driver support from Windows NT VDM.
 *   4-Jan-1993 John Morgan:  added support for transmit buffering
 */

//
//    useful utility macros
//
#include "util_def.h"

//
//    standard library include files
//
#include <windows.h>

#include <math.h>
#include <malloc.h>
#include <stdlib.h>

//
//    COM_VDD specific include files
//
#include "com_vdd.h"
#include "pc_com.h"
#include "nt_com.h"
#include "vddsvc.h"


//
// Serial driver magic numbers
//
#define INPUT_QUEUE_SIZE    (4*1024)
#define OUTPUT_QUEUE_SIZE   100

#define  XON_CHARACTER   (17)           /* XON character, Cntrl-Q */
#define  XOFF_CHARACTER  (19)           /* XOFF character, Cntrl-S */

#define  REOPEN_DELAY    (36)           /* Reopen delay in 55ms (2 secs) */

//
// Serial status structure
//  this incorporates the UART status as a substucture
//
tHostCom host_com[NUM_SERIAL_PORTS];    /* 4 comm ports - the insignia MAX */


/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::: Wait for Comm Event ::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
static BOOL wait_comm( pHostCom current )
{
    ResetEvent( current->WaitOverlap.hEvent );

    if (WaitCommEvent( current->handle, &current->EvtMask, &current->WaitOverlap ))
        return TRUE;

    if (GetLastError() == ERROR_IO_PENDING)
        return GetOverlappedResult( current->handle, &current->WaitOverlap, NULL, TRUE );

    return FALSE;
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::: Read Comm Port :::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
static BOOL read_comm( pHostCom current, DWORD *bytesread )
{

    ResetEvent( current->WaitOverlap.hEvent );

    if (ReadFile( current->handle, current->rx_buffer, RX_BUFFER_SIZE, bytesread, &current->RXOverlap ))
        return TRUE;

    if (GetLastError() == ERROR_IO_PENDING)
        return GetOverlappedResult( current->handle, &current->RXOverlap, NULL, TRUE );

    return FALSE;
}

#if (!XMIT_BUFFER)

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::: Write Comm Port ::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
static BOOL write_comm( pHostCom current, BYTE *buffer, DWORD length )
{
    DWORD byteswritten;

    ResetEvent( current->WaitOverlap.hEvent );

    if (WriteFile( current->handle, buffer, length, &byteswritten, &current->TXOverlap ))
        return TRUE;

    if (GetLastError() == ERROR_IO_PENDING)
        return GetOverlappedResult( current->handle, &current->TXOverlap, NULL, TRUE );

    return FALSE;
}

#else

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::: TX buffer pool :::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
// the number of transmit buffers available
#define NUM_TX_BUFFERS 4

typedef struct tTX_qitem
{
    struct tTX_qitem *next;      // pointer to next buffer (for queues)
    tTX_buffer buffer;
} tTX_qitem, *pTX_qitem;

static tTX_qitem  TX_q_buff[NUM_TX_BUFFERS];


/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::: TX transmit queues :::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
// queue selector type
typedef DWORD tQueueSel;

// the queue number for the free pool
#define NUM_TX_QUEUES (NUM_SERIAL_PORTS+1)
#define TX_FREE_QUEUE (NUM_TX_QUEUES-1)

typedef struct tTX_queue
{
    HANDLE wait[2];              // mutal exclusion & buffer available
    pTX_qitem head;
    pTX_qitem tail;
} tTX_queue, *pTX_queue;

static tTX_queue  TX_queue[NUM_TX_QUEUES];

static pTX_qitem TX_dequeue( tQueueSel queue_sel )
{
    pTX_queue queue = &TX_queue[queue_sel];
    pTX_qitem item;

    // wait until buffer available
    WaitForMultipleObjects( 2, queue -> wait, TRUE, INFINITE );
    if ((item = queue -> tail) != NULL)
    {
        if ((queue -> tail = item -> next) == NULL)
            queue -> head = NULL;
        item -> next = NULL;
    }
    ReleaseMutex( queue -> wait[0] );

    return item;
}

static void TX_enqueue( tQueueSel queue_sel, pTX_qitem item )
{
    pTX_queue queue = &TX_queue[queue_sel];

    WaitForSingleObject( queue -> wait[0], INFINITE );
    item -> next = NULL;
    if (queue -> head == NULL)
        queue -> tail = item;
    else
        queue -> head -> next = item;
    queue -> head = item;
    ReleaseSemaphore( queue -> wait[1], 1, NULL );
    ReleaseMutex( queue -> wait[0] );
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*:::::::::::::::::::::::::::: transmit buffers ::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

// the maximum wait time before writing a buffer even if it isn't full
#define TX_BUFF_DELAY 130

static DWORD transmit_buffers( DWORD adapter )
{
    DWORD byteswritten;
    pTX_qitem item;

    while ((item = TX_dequeue( adapter )) != NULL)      // Get next buffer to transmit
    {
        // wait until full, or timeout
        if (WaitForMultipleObjects( 2, item -> buffer.wait, TRUE, TX_BUFF_DELAY ) == WAIT_TIMEOUT)
        {
            // must have mutual exclusion regardless
            WaitForSingleObject( item -> buffer.wait[0], INFINITE );
        }

        // signal that buffer has been written
        item -> buffer.adapter = TX_FREE_QUEUE;         // not a legal port number

        // OK to release now (WE DON'T HAVE TO WAIT FOR ACTUAL WRITE!)
        ReleaseMutex( item -> buffer.wait[0] );

        // Wait for previous write (if any to complete)
        WaitForSingleObject( item -> buffer.Overlap.hEvent, INFINITE);
        ResetEvent( item -> buffer.Overlap.hEvent );

        // write buffer to com port
        WriteFile( host_com[adapter].handle, item -> buffer.bytes, item -> buffer.byte_count,
            &byteswritten, &item -> buffer.Overlap );

        // now buffer should be moved to free pool
        TX_enqueue( TX_FREE_QUEUE, item );

    }

    // close buffer queue
    CloseHandle( TX_queue[adapter].wait[0] );
    CloseHandle( TX_queue[adapter].wait[1] );

    return TRUE;
}

static void post_transmit( tAdapter adapter, BYTE data )
{
    register pHostCom current = & host_com[adapter];
    pTX_buffer buffer;
    pTX_qitem item;

    // check to see if buffer available
    if ((buffer = current -> tx_buffer) != NULL && buffer -> adapter == adapter)
    {
        // get mutual exclusion on buffer
        WaitForSingleObject( buffer -> wait[0], INFINITE );

        // check for buffer written before we could get MX
        if (buffer -> adapter == adapter)
        {
            // add another byte
            buffer -> bytes[(buffer -> byte_count)++] = data;

            // signal if buffer full
            if ((int)(buffer -> byte_count) >= current -> TX_full_length)
            {
                SetEvent( buffer -> wait[1] );
                current -> tx_buffer = NULL;
            }

            // release buffer MX
            ReleaseMutex( buffer -> wait[0] );

            return;
        }

        // Oops buffer written, but no harm done.
        ReleaseMutex( buffer -> wait[0] );
    }

    // expand buffer size by one character
    if (++(current -> TX_full_length) > TX_BUFFER_SIZE)
        current -> TX_full_length = TX_BUFFER_SIZE;

    // buffer written or unavailable.
    item = TX_dequeue( TX_FREE_QUEUE );
    item -> buffer.adapter = adapter;
    item -> buffer.bytes[0] = data;
    if ((int)(item -> buffer.byte_count = 1) >= current -> TX_full_length)
    {
        SetEvent( item -> buffer.wait[1] );        // signal buffer full
        current -> tx_buffer = NULL;
    }
    else
    {
        ResetEvent( item -> buffer.wait[1] );      // buffer not full
        current -> tx_buffer = &(item -> buffer);
    }
    TX_enqueue( adapter, item );
}

static void shrink_TX_buffer( tAdapter adapter )
{
    // collapse buffer by one character
    if (--(host_com[adapter].TX_full_length) < 0)
        host_com[adapter].TX_full_length = 0;
}

#endif //(XMIT_BUFFER)


/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::: Enter critical section for adapter :::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
void host_com_lock( tAdapter adapter )
{
    register pHostCom current = & host_com[adapter];
    if (current->host_adapter_status != HOST_ADAPTER_OPEN)
        return;  /* Exit, NULL adapter */

    EnterCriticalSection(&current->AdapterLock);
    current->AdapterLockCnt++;
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::: Leave critical section for adapter :::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
void host_com_unlock( tAdapter adapter )
{
    register pHostCom current = & host_com[adapter];

    if(current->host_adapter_status != HOST_ADAPTER_OPEN)
        return; /* Exit, NULL adapter */

    if (current->AdapterLockCnt != 0)
    {
        current->AdapterLockCnt--;
        LeaveCriticalSection(&current->AdapterLock);
    }
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::: Interrupt detection thread, one per comm port ::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
static DWORD monitor_comms( DWORD adapter )
{
    register pHostCom current = & host_com[adapter];       // Set ptr to current adapter
    BOOL com_locked = FALSE;                                // not locked!

#define EV_LS_MS_TX (EV_ERR | EV_BREAK | EV_CTS | EV_DSR | EV_RING | EV_RLSD | EV_TXEMPTY)
    /* Set up event mask (without RX event) */
    SetCommMask( current->handle, EV_LS_MS_TX );

    forever
    {
        if (com_locked)
        {
            host_com_unlock( adapter );
            com_locked = FALSE;
        }

        wait_comm( current );

        if (current->TerminateRXThread)
        {
            return 0;                                       // Terminate thread
        }
                                                                 
        if (current->EvtMask & (EV_ERR | EV_BREAK))
        {
            if (!com_locked)
            {
                host_com_lock( adapter );
                com_locked = TRUE;
            }
            raise_rls_interrupt( &host_com[adapter].uart );
        }
        if (current->EvtMask & (EV_CTS | EV_DSR | EV_RING | EV_RLSD))
        {
            if (!com_locked)
            {
                host_com_lock( adapter );
                com_locked = TRUE;
            }
            raise_ms_interrupt( &host_com[adapter].uart );
        }
        if (current->EvtMask & (EV_RXCHAR))
        {
            if (!com_locked)
            {
                host_com_lock( adapter );
                com_locked = TRUE;
            }
            if (check_rda_interrupt( &host_com[adapter].uart ))
            {
                // reset event mask (without RX event)
                // to reduce overhead from this thread.
                SetCommMask( current->handle, EV_LS_MS_TX );
                current->rx_waiting = FALSE;
            }
            else
            {
                raise_rda_interrupt( &host_com[adapter].uart );
            }
        }
        if (current->EvtMask & (EV_TXEMPTY))
        {
            if (!com_locked)
            {
                host_com_lock( adapter );
                com_locked = TRUE;
            }
            clear_tbr_flag( & host_com[adapter].uart );
        }
    }
}

void host_com_rx_wait( tAdapter adapter )
{
    register pHostCom current = & host_com[adapter];     // Set ptr to current adapter

    if (!current->rx_waiting)
    {
        SetCommMask( current->handle, EV_LS_MS_TX | EV_RXCHAR );// reset event mask with RX event
        current->rx_waiting = TRUE;
    }
}


/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*:::::::::::::::::::::::: Close comms port ::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
void host_com_close( tAdapter adapter )
{
    register pHostCom current;
    
    com_close( adapter );

    current = & host_com[adapter];

    /*:::::::::::::::::::::::::::::::::::::::: Dealing with NULL adapter ? */
    if (current->host_adapter_status == HOST_ADAPTER_OPEN)
    {
        /*................................................. Close RX thread */
        if (current->RXThreadHandle != NULL)
        {
            /*................................. Tell RX thread to terminate */

            current->TerminateRXThread = TRUE;  // Tell RX thread to terminate

            /*....................... Signal RX Thread to break out of Wait */
            SetCommMask(current->handle, 0 );

            /*..... Wait for RX thread to close itself, max wait 30 seconds */
            WaitForSingleObject(current->RXThreadHandle, 30000);
            CloseHandle(current->RXThreadHandle);

            current->RXThreadHandle = NULL;  // Mark thread as closed
        }

#if (XMIT_BUFFER)
        /*................................................. Close TX thread */
        if (current->TXThreadHandle != NULL)
        {
            /*................................. Tell TX thread to terminate */
            ReleaseSemaphore( TX_queue[adapter].wait[1], 1, NULL );

            /*..... Wait for RX thread to close itself, max wait 30 seconds */
            WaitForSingleObject(current->TXThreadHandle, 30000);
            CloseHandle(current->TXThreadHandle);

            current->TXThreadHandle = NULL;  // Mark thread as closed
        }
#endif //(XMIT_BUFFER)

        /*.............................................. Close Comms device */
        CloseHandle(current->handle);
        current->handle = NULL;

        /*. This makes sure that the next access to the port will reopen it */
        current->ReOpenCounter = 0;
        current->host_adapter_status = HOST_ADAPTER_NOT_OPEN;   /* Mark adapter as closed */
     }
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::: Open comms port ::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
static BOOL host_com_open( tAdapter adapter )
{
    COMMTIMEOUTS comout;            /* Comms time out settings */
    char adapter_name[] = "COM?";
    register const pHostCom current = & host_com[adapter];

    switch (current->host_adapter_status) {
    case HOST_ADAPTER_DISABLED:
        return FALSE;

    case HOST_ADAPTER_OPEN:
        return TRUE;

    case HOST_ADAPTER_NOT_OPEN:
        /*:::::::::: Attempting to open the port too soon after a failed open ? */
        if (current->ReOpenCounter != 0)
            return FALSE;              /* Yes */

        /*::::::::::::::::::::::::::: We have a vaild adapter so try to open it */
        adapter_name[3] = '1'+adapter;
        current->handle = CreateFile( adapter_name,
                                      GENERIC_READ | GENERIC_WRITE, 0, NULL,
                                      OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                                      NULL
                                    );

        /*............................................... Validate open attempt */
        if (current->handle == (HANDLE) -1)
        {
            current->host_adapter_status = HOST_ADAPTER_DISABLED;       /* Unable to open adapter */
            return FALSE;
        }

        /*:::::::::::::::::::::::::::::::::::::::::::::::: adapter port is open */
        current->host_adapter_status = HOST_ADAPTER_OPEN;

        /*::::::::::::::::::::::::::::::::::::::: Set Comms port to binary mode */
        if (!GetCommState( current->handle, &(current->dcb) ))
        {
            host_com_close( adapter );    /* turn it into a NULL adapter */
            current->ReOpenCounter = REOPEN_DELAY;   /* Delay next open attempt */
            return FALSE;
        }
    
        /*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: Setup DCB */
        current->dcb.fBinary = 1;                   /* run in RAW mode */
        current->dcb.fOutxCtsFlow = FALSE;          /* Disable CTS */
        current->dcb.fOutxDsrFlow = FALSE;          /* Disable DSR */
        current->dcb.fDtrControl = DTR_CONTROL_DISABLE;
        current->dcb.fOutX = FALSE;                 /* Disable XON/XOFF */
        current->dcb.fInX = FALSE;
        current->dcb.fRtsControl = RTS_CONTROL_DISABLE;

        current->dcb.XonChar = XON_CHARACTER;       /* Define XON/XOFF chars */
        current->dcb.XoffChar = XOFF_CHARACTER;

        /* Turn off error char replacement */
        current->dcb.fErrorChar = FALSE;

        /*:::::::::::::::::::::::::::::::::::::::::::::::: Set Comms port state */
        if (!SetCommState( current->handle, &(current->dcb) ))
        {
            host_com_close( adapter );
            current->ReOpenCounter = REOPEN_DELAY;   /* Delay next open attempt */
            return FALSE;
        }

        /*::::::::::::::::::::::::::::::::::::::::::::: Setup comms queue sizes */
        if (!SetupComm( current->handle, INPUT_QUEUE_SIZE, OUTPUT_QUEUE_SIZE) )
        {
            host_com_close( adapter );
            current->ReOpenCounter = REOPEN_DELAY;   /* Delay next open attempt */
            return FALSE;
        }

        /*::::::::::::::::::::: Set communication port up for non-blocking read */
        GetCommTimeouts( current->handle, &comout );

        comout.ReadIntervalTimeout = MAXDWORD;
        comout.ReadTotalTimeoutMultiplier = 0;
        comout.ReadTotalTimeoutConstant = 0;

        SetCommTimeouts( current->handle, &comout );

        /*:::::::::::::::::::::::::::::::: reset baud rate, line & modem control */
        com_reset( adapter );

#if (XMIT_BUFFER)
        /*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: TX buffer */
        current -> TX_full_length = 0;

        /*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: TX queue */
        TX_queue[adapter].head = NULL;
        TX_queue[adapter].tail = NULL;
        TX_queue[adapter].wait[0] = CreateMutex( NULL, FALSE, NULL);
        TX_queue[adapter].wait[1] = CreateSemaphore( NULL, 0, NUM_TX_BUFFERS, NULL);

        /*::::::::::::::::::::::::::::::::::::::::::::::: Create Comms TX thread */
        if (!(current->TXThreadHandle = CreateThread( NULL,
                                                      10*1024,
                                                      (LPTHREAD_START_ROUTINE)transmit_buffers,
                                                      (LPVOID)adapter,
                                                      0L,
                                                      &current->TXThreadID )))
        {
            host_com_close( adapter );        /* Unable to create TX thread */
            current->ReOpenCounter = REOPEN_DELAY;   /* Delay next open attempt */
            return FALSE;
        }
        SetThreadPriority( current -> TXThreadHandle, THREAD_PRIORITY_TIME_CRITICAL );
#endif //(XMIT_BUFFER)

        /*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: RX buffer */
        current->rx_error = 0;              // no errors pending
        current->rx_curr = current->rx_end = current->rx_buffer;
                                            // buffer empty
        current->rx_waiting = FALSE;        // not waiting for RX interrupt

        /*::::::::::::::::::::::::::::::::::::::::::::::: Create Comms RX thread */
        current->TerminateRXThread = FALSE;
        if (!(current->RXThreadHandle = CreateThread( NULL,
                                                      10*1024,
                                                      (LPTHREAD_START_ROUTINE)monitor_comms,
                                                      (LPVOID)adapter,
                                                      0L,
                                                      &current->RXThreadID )))
        {
            host_com_close( adapter );        /* Unable to create RX thread */
            current->ReOpenCounter = REOPEN_DELAY;   /* Delay next open attempt */
            return FALSE;
        }

        return TRUE;

    default:
        return FALSE;
    }
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::: RX routines ::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
DWORD host_com_get_error( tAdapter adapter )
{
    register pHostCom current = & host_com[adapter];     // Set ptr to current adapter
    DWORD error;

    if (current->host_adapter_status == HOST_ADAPTER_NOT_OPEN && !host_com_open( adapter ))
        return FALSE;                             /* Exit, unable to open adapter */

    if (current->rx_curr == current->rx_end)
    {
        error = current->rx_error;
        current->rx_error = 0;
        return error;
    }
    else
        return 0;
}

BOOL host_com_rx_avail( tAdapter adapter )
{
    register pHostCom current = & host_com[adapter];     // Set ptr to current adapter
    DWORD bytesread = 0;
    DWORD error = 0;

    if (current->host_adapter_status != HOST_ADAPTER_OPEN && !host_com_open( adapter ))
        return FALSE;                             /* Exit, unable to open adapter */

    if (current->rx_curr != current->rx_end)
        return TRUE;

    read_comm( current, &bytesread );

    // get communication error status
    ClearCommError( current->handle, &(current->rx_error), NULL );

    current->rx_curr = current->rx_buffer;
    current->rx_end = current->rx_buffer + bytesread;

    return (current->rx_curr != current->rx_end);
}

BOOL host_com_read_rx( tAdapter adapter, BYTE *data )
{
    if (host_com_rx_avail( adapter ))
    {
        *data = *(host_com[adapter].rx_curr++);
#if (XMIT_BUFFER)
        shrink_TX_buffer( adapter );
#endif

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*:::::::::::::::::::::::: Write to comms port :::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
BOOL host_com_write_tx( tAdapter adapter, BYTE data )
{
    if (host_com[adapter].host_adapter_status == HOST_ADAPTER_NOT_OPEN && !host_com_open( adapter ))
        return FALSE;                             /* Exit, unable to open adapter */

#if (XMIT_BUFFER)
    post_transmit( adapter, data );
#else //(XMIT_BUFFER)
    write_comm( &host_com[adapter], &data, 1 );
#endif //(XMIT_BUFFER)
    return TRUE;
}


/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
ULONG host_com_ioctl( tAdapter adapter, int request, long arg )
{
    DWORD ModemState;
    ULONG ReturnState = 0;
    register pHostCom current = & host_com[adapter];      /* Define and set 'current' adaptor pointer */

    /*:::::::::::::::::::::::::::::::::: Are we dealing with a null adapter */

    if (current->host_adapter_status != HOST_ADAPTER_OPEN)
    {
        // Attempt to open adapter !

        if (request == HOST_COM_MSTATUS || !host_com_open(adapter))
        {
            return ReturnState;
        }
    }

    /*:::::::::::::::::::::::::::::::::::::::::::::: Identify ioctl request */

    switch (request)
    {
        /*:::::::::::::::::::::::::::::::::::::::::: Process break requests */

        case HOST_COM_SBRK:         /* Set BREAK */
            SetCommBreak(current->handle);
            break;

        case HOST_COM_CBRK:        /* Clear BREAK */
            ClearCommBreak(current->handle);
            break;

        /*::::::::::::::::::::::::::::::::::::::::: Process baud rate change */
        case HOST_COM_BAUD:
            if (GetCommState( current->handle, &(current->dcb) ))
            {
                current->dcb.BaudRate = arg;
                SetCommState( current->handle, &(current->dcb) );
            }
            break;


        /*:::::::::::::::::::::::::::::::::: Process modem control requests */
        case HOST_COM_MODEM_CTRL:                 /* DTR & RTS lines */
            if (arg & HOST_MC_DTR)
                EscapeCommFunction( current->handle, SETDTR );
            else
                EscapeCommFunction( current->handle, CLRDTR );

            if (arg & HOST_MC_RTS)
                EscapeCommFunction( current->handle, CLRRTS );
            else
                EscapeCommFunction( current->handle, SETRTS );
            break;

        /*::::::::::::::::::::::::::::::::::::::::::::: Return modem status */

        case HOST_COM_MSTATUS:              /* Get modem state */
            GetCommModemStatus( current->handle, &ModemState );

            if (ModemState & MS_CTS_ON)  ReturnState |= HOST_MS_CTS;
            if (ModemState & MS_RING_ON) ReturnState |= HOST_MS_RI;
            if (ModemState & MS_DSR_ON)  ReturnState |= HOST_MS_DSR;
            if (ModemState & MS_RLSD_ON) ReturnState |= HOST_MS_RLSD;

            current->modem_status = ReturnState;

            break;

        /*:::::::::::::::::::::::::::::::::::::::::::::::: Set line control */
        case HOST_COM_LINE_CTRL:
            if (GetCommState( current->handle, &(current->dcb) ))
            {
                current->dcb.ByteSize = (BYTE)arg & HOST_LC_DATABITS;

                switch (arg & HOST_LC_STOPBITS)
                {
                    case HOST_LC_STOP_1:
                        current->dcb.StopBits = ONESTOPBIT; 
                        break;
                    case HOST_LC_STOP_2:  
                        current->dcb.StopBits = TWOSTOPBITS;
                        break;
                    case HOST_LC_STOP_15:  
                        current->dcb.StopBits = ONE5STOPBITS;
                        break;
                    default:
                        break;
                }

                switch (arg & HOST_LC_PARITY)
                {
                    case HOST_LC_PARITY_EVEN:
                        current->dcb.Parity=EVENPARITY;
                        current->dcb.fParity=1;   /* ensure parity enabled */
                        break;

                    case HOST_LC_PARITY_ODD:
                        current->dcb.Parity=ODDPARITY;
                        current->dcb.fParity=1;   /* ensure parity enabled */
                        break;

                    case HOST_LC_PARITY_MARK:
                        current->dcb.Parity=MARKPARITY;
                        current->dcb.fParity=1;   /* ensure parity enabled */
                        break;

                    case HOST_LC_PARITY_SPACE:
                        current->dcb.Parity=SPACEPARITY;
                        current->dcb.fParity=1;   /* ensure parity enabled */
                        break;

                    case HOST_LC_PARITY_NONE:
                        current->dcb.Parity=NOPARITY;
                        current->dcb.fParity=0;   /* disable parity */
                        break;
                }

                SetCommState( current->handle, &(current->dcb) );
            }
            break;

        /*::::::::::::::::::::::::::::::::::::::: Unrecognised host_com ioctl */

        default:
            break;
    }

    return ReturnState;
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*:::::::::::::::::::::: Initialize comms port :::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
void host_com_init()
{
    tAdapter adapter;
    register pHostCom current;

#if (XMIT_BUFFER)
    int buffer_num;
    // set up transmit buffer free pool
    TX_queue[TX_FREE_QUEUE].head = NULL;
    TX_queue[TX_FREE_QUEUE].tail = NULL;
    TX_queue[TX_FREE_QUEUE].wait[0] = CreateMutex( NULL, FALSE, NULL);
    TX_queue[TX_FREE_QUEUE].wait[1] = CreateSemaphore( NULL, 0, NUM_TX_BUFFERS, NULL);

    // set up transmit buffers
    for (buffer_num = 0; buffer_num < NUM_TX_BUFFERS; buffer_num++)
    {
        TX_q_buff[buffer_num].buffer.wait[0] = CreateMutex( NULL, FALSE, NULL);
        TX_q_buff[buffer_num].buffer.wait[1] = CreateEvent( NULL, TRUE, FALSE, NULL);
        TX_q_buff[buffer_num].buffer.adapter = TX_FREE_QUEUE;
        TX_q_buff[buffer_num].buffer.Overlap.hEvent = CreateEvent( NULL, TRUE, TRUE, NULL );
        TX_enqueue( TX_FREE_QUEUE, &TX_q_buff[buffer_num] );
    }
#endif //(XMIT_BUFFER)

    // set up ports
    for (adapter = 0; adapter < NUM_SERIAL_PORTS; adapter++)
    {
        current = & host_com[adapter];

        current->host_adapter_status = HOST_ADAPTER_NOT_OPEN;
        current->handle = (HANDLE) -1;
        current->ReOpenCounter = 0;
        current->RXThreadHandle = NULL;

        /* critical section used to lock access to adapter from the base */
        InitializeCriticalSection(&current->AdapterLock);
        current->AdapterLockCnt = 0;

        current->WaitOverlap.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        current->RXOverlap.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
#if (!XMIT_BUFFER)
        current->TXOverlap.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
#endif

        com_init( adapter );
    }
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::: Deinitialize comms port ::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
void host_com_exit()
{
    tAdapter adapter;
    int buffer_num;

    // close ports
    for (adapter = 0; adapter < NUM_SERIAL_PORTS; adapter++)
    {
        com_close( adapter );
        host_com_close( adapter );

        CloseHandle( host_com[adapter].WaitOverlap.hEvent );
        CloseHandle( host_com[adapter].RXOverlap.hEvent );
#if (!XMIT_BUFFER)
        CloseHandle( host_com[adapter].TXOverlap.hEvent );
#endif

        /*............... Delete RX critical section and RX control objects */
        DeleteCriticalSection( &host_com[adapter].AdapterLock );
    }

#if (XMIT_BUFFER)
    // close transmit buffers
    for (buffer_num = 0; buffer_num < NUM_TX_BUFFERS; buffer_num++)
    {
        CloseHandle( TX_q_buff[buffer_num].buffer.wait[0] );
        CloseHandle( TX_q_buff[buffer_num].buffer.wait[1] );
    }

    // close transmit buffer free pool
    CloseHandle( TX_queue[TX_FREE_QUEUE].wait[0] );
    CloseHandle( TX_queue[TX_FREE_QUEUE].wait[1] );
#endif //(XMIT_BUFFER)
}


