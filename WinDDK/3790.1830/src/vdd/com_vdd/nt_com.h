/*
 * NT_com.h
 *
 * function and structure definitions used external to NT_com.c
 *
 * copyright 1992, 1993 by Microsoft Corporation
 * portions copyright 1991 by Insignia Solutions Ltd., used by permission.
 *
 * revision history:
 *  24-Dec-1992 John Morgan:  written based on  nt_com.h  written by D.A.Bartlett
 *   4-Jan-1993 John Morgan:  added support for buffered transmits
 *
 */

#define XMIT_BUFFER 1

typedef enum 
{
    HOST_ADAPTER_NOT_OPEN,
    HOST_ADAPTER_OPEN,
    HOST_ADAPTER_SUSPENDED,
    HOST_ADAPTER_DISABLED
} host_adapter_status_t;

// the maximum size of the receive buffer
#define RX_BUFFER_SIZE  250

// the maximum size of a transmit buffer
#define TX_BUFFER_SIZE  250

#if (XMIT_BUFFER)

typedef struct tTX_buffer
{
    HANDLE wait[2];                 // mutal exclusion & FULL buffer signal
    tAdapter adapter;               // signal what port is using buffer
    DWORD byte_count;               // number of characters in buffer
    BYTE bytes[TX_BUFFER_SIZE];     // the bytes to be output
    OVERLAPPED Overlap;             // Overlapped I/O structure
} tTX_buffer, *pTX_buffer;

#endif

typedef struct
{
    UART_STATE uart;                    // state of simulated UART

    /*...................................................... Host interface */
    host_adapter_status_t host_adapter_status;
    int ReOpenCounter;                  // Counter to restrict open attempts
    HANDLE handle;                      // Device handle
    DCB dcb;                            // device control block

    long modem_status;                  // modem status line settings
    HANDLE ModemEvent;                  // Get modem status control event

    /*.............................................. Access control objects */
    int AdapterLockCnt;                 // Adapter lock count
    CRITICAL_SECTION AdapterLock;

    DWORD EvtMask;                      // Communication events
    OVERLAPPED WaitOverlap;
    OVERLAPPED RXOverlap;

#if (!XMIT_BUFFER)
    /*......................................... TX overlapped I/O structure */
    OVERLAPPED TXOverlap;

#else
    /*......................................... TX buffer control variables */
    int TX_full_length;                 // how many characters before xmit full
    volatile pTX_buffer tx_buffer;      // TX transmit buffer

    /*............................................. TX thread handle and ID */
    DWORD TXThreadID;                   // RX thread ID
    HANDLE TXThreadHandle;              // RX thread handle

#endif

    /*......................................... RX buffer control variables */
    DWORD rx_error;                     // error for char at end of buffer
    char  rx_buffer[RX_BUFFER_SIZE];    // RX character buffer
    char *rx_end;                       // end of current chars
    char *rx_curr;                      // Next character to read
    BOOLEAN rx_waiting;                 // Waiting for rx interrupt

    /*............................................. RX thread handle and ID */
    volatile BOOL TerminateRXThread;
    DWORD RXThreadID;                   // RX thread ID
    HANDLE RXThreadHandle;              // RX thread handle

} tHostCom, *pHostCom;

extern tHostCom host_com[NUM_SERIAL_PORTS];
#define UART_ADAPTER(x) host_com[x].uart

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::: host com interface */

void host_com_lock( tAdapter );
void host_com_unlock( tAdapter );
void host_com_rx_wait( tAdapter );

void host_com_init();
void host_com_exit();

void host_com_close( tAdapter );
//void host_com_reset( tAdapter );         // not used
#define host_com_reset(x)
ULONG host_com_ioctl( tAdapter, int, long );
DWORD host_com_get_error( tAdapter );
BOOL host_com_data_avail( tAdapter );
BOOL host_com_read_rx( tAdapter, BYTE* );
BOOL host_com_write_tx( tAdapter, BYTE );

void host_com_char_read( tAdapter, int );

// short host_com_valid
//    ( BYTE hostID, ConfigValues *val, NameTable *dummy, CHAR *errString );
void host_com_change( BYTE hostID, BOOL apply );
short host_com_active( BYTE hostID, BOOL active, CHAR *errString );


/*:::::::::::::::::::::::::::::::::::::::::::::::::: Enumeration definitions */

#define HOST_COM_MODEM_CTRL     0000001
#define HOST_COM_LINE_CTRL      0000002
#define HOST_COM_BAUD           0000003 
#define HOST_COM_CBRK           0000004
#define HOST_COM_SBRK           0000005
#define HOST_COM_MSTATUS        0000010
//#define HOST_COM_FLUSH          0000011
//#define HOST_COM_INPUT_READY    0000012

// bit masks for defining host modem status
#define HOST_MS_CTS             (1 << 0)
#define HOST_MS_RI              (1 << 1)
#define HOST_MS_DSR             (1 << 2)
#define HOST_MS_RLSD            (1 << 3)

// bit masks for defining host modem control
#define HOST_MC_RTS             (1 << 0)
#define HOST_MC_DTR             (1 << 1)

// bit masks for defining host line control
#define HOST_LC_DATABITS       0x00F
#define HOST_LC_DATA_5         0x005
#define HOST_LC_DATA_6         0x006
#define HOST_LC_DATA_7         0x007
#define HOST_LC_DATA_8         0x008

#define HOST_LC_STOPBITS       0x0F0
#define HOST_LC_STOP_1         0x010
#define HOST_LC_STOP_2         0x020
#define HOST_LC_STOP_15        0x030

#define HOST_LC_PARITY         0xF00
#define HOST_LC_PARITY_NONE    0xF00
#define HOST_LC_PARITY_EVEN    0x000
#define HOST_LC_PARITY_ODD     0x100
#define HOST_LC_PARITY_SPACE   0x200
#define HOST_LC_PARITY_MARK    0x300


