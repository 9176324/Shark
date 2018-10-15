// Copyright (c) Microsoft 1992-1999, All Rights Reserved
// portions copyright 1991 by Insignia Solutions Ltd., used by permission.
//   

/*
 * PC_com.c
 *
 * This module handles the virtual UART I/O and interrupt interface
 *
 * Note:
 *  Refer to National Semiconductor literature for a detailed description
 *  of the NS16450/NS8250A UART.  Refer to the PC-XT Tech Ref Manual 
 *  Section 1-185 for a description of the Asynchronous Adaptor Card.
 *
 *
 * revision history:
 *  24-Dec-1992 John Morgan:  written based (in part) on 
 *                             serial driver support from Windows NT VDM.
 *
 */


//
//    useful utility macros
//
#include "util_def.h"

//
//    standard library include files
//
#include <windows.h>

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <string.h>

//
//    COM_VDD specific include files
//
#include "com_vdd.h"
#include "PC_def.h"
#include "PC_com.h"
#include "NT_com.h"
#include "vddsvc.h"

/*
 * =====================================================================
 * Subsidiary functions - for interrupt emulation
 * =====================================================================
 */
static void check_interrupt( UART_STATE *asp )
{
    /*
     * Follow somewhat dubious advice on Page 1-188 of XT Tech Ref
     * regarding the adapter card sending interrupts to the system.
     * Apparently confirmed by the logic diagram.
     */
    if (asp->out2_state
        && !asp->loopback_state
        && (asp->data_available_interrupt_state
            || asp->tx_holding_register_empty_interrupt_state
            || asp->receiver_line_status_interrupt_state
            || asp->modem_status_interrupt_state))
    {
        if (!asp->interrupt_line_state)
        {
            asp->interrupt_line_state = TRUE;
            VDDSimulateInterrupt( 0, asp->hw_interrupt_priority, 1);
        }
    }     
    else
    {
        asp->interrupt_line_state = FALSE;
    }
}

extern void raise_rls_interrupt( UART_STATE *asp )
{
    /*
     * Check if receiver line status interrupt is enabled
     */
    if (asp->int_enable_reg.bits.rx_line)
    {
        asp->receiver_line_status_interrupt_state = TRUE;
        check_interrupt(asp);
    }
}

extern void raise_rda_interrupt( UART_STATE *asp )
{
    /*
     * Check if data available interrupt is enabled
     */
    if (asp->int_enable_reg.bits.data_available)
    {
        asp->data_available_interrupt_state = TRUE;
        check_interrupt(asp);
    }
}

extern void raise_ms_interrupt( UART_STATE *asp )
{
    /*
     * Check if modem status interrupt is enabled
     */
    if (asp->int_enable_reg.bits.modem_status)
    {
        asp->modem_status_interrupt_state = TRUE;
        check_interrupt(asp);
    }
    asp->modem_status_changed = TRUE;
}

extern void raise_thre_interrupt( UART_STATE *asp )
{
    /*
     * Check if tx holding register empty interrupt is enabled
     */
    if (asp->int_enable_reg.bits.tx_holding)
    {
        asp->tx_holding_register_empty_interrupt_state = TRUE;
        check_interrupt(asp);
    }
}


/*
 * =====================================================================
 * Subsidiary functions - for data available
 * =====================================================================
 */

/*
 * signal all characters transmitted!
 */
void clear_tbr_flag( UART_STATE *asp )
{
    asp->line_status_reg.bits.tx_shift_empty = 1;
}

/*
 * check for data available and line status changes
 */
static void check_data_available( tAdapter adapter )
{
    UART_STATE *asp = & UART_ADAPTER(adapter);
    DWORD error;
    
    if (!asp->loopback_state)
    {
        if (!asp->line_status_reg.bits.data_ready)
        {
            if (!(asp->line_status_reg.bits.data_ready = (BYTE_BIT_FIELD)host_com_read_rx( adapter, &asp->rx_buff_reg ))
                && asp->int_enable_reg.bits.data_available)
            {
                host_com_rx_wait( adapter );
            }
        }

        if ((error = host_com_get_error( adapter )) != 0)
        {
            /*
             * Set line status register and raise line status interrupt
             */
            if (error & (CE_OVERRUN | CE_RXOVER))
                asp->line_status_reg.bits.overrun_error = 1;

            if (error & CE_FRAME)
                asp->line_status_reg.bits.framing_error = 1;

            if (error & CE_RXPARITY)
                asp->line_status_reg.bits.parity_error = 1;

            if (error & CE_BREAK)
                asp->line_status_reg.bits.break_interrupt = 1;

            raise_rls_interrupt( asp );
        }
    }
}


/*
 * flush all received input
 */
static void com_flush_input(tAdapter adapter)
{
    BYTE dummy;
    
    while (host_com_read_rx( adapter, &dummy ))
        /* do nothing */;

    UART_ADAPTER(adapter).line_status_reg.bits.data_ready = FALSE;
}

/*
 * set the IIR for current interrupts pending
 */
static BYTE generate_iir( tAdapter adapter )
{
    UART_STATE *asp = & UART_ADAPTER(adapter);

    /*
     * Set up interrupt identification register with highest priority
     * pending interrupt.
     */

    check_data_available( adapter );

    return 
        (asp->receiver_line_status_interrupt_state) ?           UART_RLS_INT
        : (asp->line_status_reg.bits.data_ready) ?              UART_RDA_INT
          : (asp->tx_holding_register_empty_interrupt_state) ?  UART_THRE_INT
            : (asp->modem_status_interrupt_state) ?             UART_MS_INT
              :                                                 UART_NO_INT;
}


/*
 * set modem status register to new status
 */
static void set_modem_status( tAdapter adapter, long modem_status )
{
    UART_STATE *asp = & UART_ADAPTER( adapter );
    int cts_state, dsr_state, rlsd_state, ri_state;

    cts_state  = ((modem_status & HOST_MS_CTS)  != 0);
    dsr_state  = ((modem_status & HOST_MS_DSR)  != 0);
    rlsd_state = ((modem_status & HOST_MS_RLSD) != 0);
    ri_state   = ((modem_status & HOST_MS_RI)   != 0);

    /*
     * Establish CTS state
     */
    if (cts_state != asp->modem_status_reg.bits.CTS)
    {
        asp->modem_status_reg.bits.CTS = (BYTE_BIT_FIELD)cts_state;
        asp->modem_status_reg.bits.delta_CTS = TRUE;
        raise_ms_interrupt(asp);
    }

    /*
     * Establish DSR state
     */
    if (dsr_state != asp->modem_status_reg.bits.DSR)
    {
        asp->modem_status_reg.bits.DSR = (BYTE_BIT_FIELD)dsr_state;
        asp->modem_status_reg.bits.delta_DSR = TRUE;
        raise_ms_interrupt(asp);
    }

    /*
     * Establish RLSD state
     */
    if (rlsd_state != asp->modem_status_reg.bits.RLSD)
    {
        asp->modem_status_reg.bits.RLSD = (BYTE_BIT_FIELD)rlsd_state;
        asp->modem_status_reg.bits.delta_RLSD = TRUE;
        raise_ms_interrupt(asp);
    }

    /*
     * Establish RI state
     */
    if (ri_state != asp->modem_status_reg.bits.RI)
    {
        if ((asp->modem_status_reg.bits.RI = (BYTE_BIT_FIELD)ri_state) == FALSE)
        {
            asp->modem_status_reg.bits.TERI = TRUE;
            raise_ms_interrupt(asp);
        }
    }
}

/*
 * One of the modem control input lines has changed state
 */
static void refresh_modem_status( tAdapter adapter )
{
    /*
     * Update the modem status register after a change to one of the
     * modem status input lines
     */
    UART_STATE *asp = & UART_ADAPTER(adapter);

    if (! UART_ADAPTER(adapter).loopback_state)
    {
        /* get current modem input state */
        set_modem_status( adapter, host_com_ioctl( adapter, HOST_COM_MSTATUS, 0 ) );
    }
}


/*
 * =====================================================================
 * Subsidiary functions - for setting comms parameters
 * =====================================================================
 */

static void set_baud_rate( tAdapter adapter )
{
    const long UART_bit_clock = 115200;
    UART_STATE *asp = & UART_ADAPTER(adapter);

    if (UART_ADAPTER(adapter).divisor_latch.all != 0)
    {
        com_flush_input( adapter );
        host_com_ioctl( adapter, 
                        HOST_COM_BAUD, 
                        UART_bit_clock / UART_ADAPTER(adapter).divisor_latch.all
                      );
    }
}

static void set_break( tAdapter adapter )
{
    /*
     * Process the set break control bit. Bit 6 of the Line Control
     * Register.
     */
    UART_STATE *asp = & UART_ADAPTER(adapter);

    if (asp->line_control_reg.bits.set_break != asp->break_state )
    {
        asp->break_state = asp->line_control_reg.bits.set_break;
        host_com_ioctl( adapter, HOST_COM_SBRK, 0 );
    }
}

static void set_line_control( tAdapter adapter )
{
    /*
     * Set Number of data bits
     *     Parity bits
     *     Number of stop bits
     */
    UART_STATE *asp = & UART_ADAPTER(adapter);
    int host_line_ctrl;
    const int host_bits[4][2] =
        { HOST_LC_DATA_5 | HOST_LC_STOP_1,
          HOST_LC_DATA_5 | HOST_LC_STOP_15,
          HOST_LC_DATA_6 | HOST_LC_STOP_1,
          HOST_LC_DATA_6 | HOST_LC_STOP_2,
          HOST_LC_DATA_7 | HOST_LC_STOP_1,
          HOST_LC_DATA_7 | HOST_LC_STOP_2,
          HOST_LC_DATA_8 | HOST_LC_STOP_1,
          HOST_LC_DATA_8 | HOST_LC_STOP_2};

    host_line_ctrl =

    // the number of data bits
        host_bits[asp->line_control_reg.bits.word_length]

    // and stop bits
                    [asp->line_control_reg.bits.no_of_stop_bits]

    // and the parity settings
        | ((asp->line_control_reg.bits.parity_enabled == UART_PARITY_OFF)
            ? HOST_LC_PARITY_NONE

            : (asp->line_control_reg.bits.stick_parity == UART_PARITY_STICK)
                ? (asp->line_control_reg.bits.even_parity == UART_PARITY_ODD)
                    ? HOST_LC_PARITY_MARK

                    : HOST_LC_PARITY_SPACE

                : (asp->line_control_reg.bits.even_parity == UART_PARITY_ODD)
                    ? HOST_LC_PARITY_ODD

                    : HOST_LC_PARITY_EVEN
          );

    host_com_ioctl(adapter, HOST_COM_LINE_CTRL, host_line_ctrl);
}

void set_modem_control( tAdapter adapter )
{
    UART_STATE *asp = & UART_ADAPTER( adapter );

    /*
     * Process the loopback control bit
     */

    if (asp->modem_control_reg.bits.loop != asp->loopback_state)
    {
        if (!(asp->loopback_state = asp->modem_control_reg.bits.loop))
        {
            /*
             * Reset the modem status inputs according to the real
             * modem status
             */
            refresh_modem_status( adapter );
        }
    }

    /*
     * Modem control function depends on the loopback control bit
     */
    
    if (asp->loopback_state)
    {
        set_modem_status( adapter, 

        // loop DTR back to DSR
            (asp->modem_control_reg.bits.DTR && HOST_MS_DSR)

        // loop RTS back to CTS
            | (asp->modem_control_reg.bits.RTS && HOST_MS_CTS)
    
        // loop OUT1 back to RI
            | (asp->modem_control_reg.bits.OUT1 && HOST_MS_RI)

        // loop OUT2 back to RLSD
            | (asp->modem_control_reg.bits.OUT2 && HOST_MS_RLSD) );

    }
    else  // not in loopback state
    {
        BYTE host_modem_control = 
            (asp->modem_control_reg.bits.DTR ? HOST_MC_DTR : 0)
            | (asp->modem_control_reg.bits.RTS ? HOST_MC_RTS : 0);

        /*
         * In the real adapter, the OUT1 control bit is not connected
         * so no real modem control change is required
         */

        /*
         * In the real adapter the OUT2 control bit is used to determine
         * whether the communications card can send interrupts; so no
         * real modem control change is required
         */
        asp->out2_state = asp->modem_control_reg.bits.OUT2;

        if (asp->modem_ctrl_state != host_modem_control)
        {
            asp->modem_ctrl_state = host_modem_control;
            host_com_ioctl( adapter, HOST_COM_MODEM_CTRL, host_modem_control );
        }
    }
}

void com_reset(tAdapter adapter)
{
    /*
     * Set host devices to current state
     */
    set_baud_rate( adapter );
    set_line_control( adapter );
    set_break( adapter );
    set_modem_control( adapter );
}

static int port_start[4] = {PC_COM1_PORT_START,
                            PC_COM2_PORT_START,
                            PC_COM3_PORT_START,
                            PC_COM4_PORT_START};
static int port_end[4] = {PC_COM1_PORT_END,
                          PC_COM2_PORT_END,
                          PC_COM3_PORT_END,
                          PC_COM4_PORT_END};
static BYTE int_pri[4] = {PC_COM1_INT,
                         PC_COM2_INT,
                         PC_COM3_INT,
                         PC_COM4_INT};
static int timeout[4] = {PC_COM1_TIMEOUT,
                         PC_COM2_TIMEOUT,
                         PC_COM3_TIMEOUT,
                         PC_COM4_TIMEOUT};

void com_init( tAdapter adapter )
{
    UART_STATE *asp = & UART_ADAPTER(adapter);

    asp->had_first_read = FALSE;
    asp->hw_interrupt_priority = int_pri[adapter];

    /*
     * Set default state of all adapter registers
     */
    asp->int_enable_reg.all = 0;

    asp->line_control_reg.all = 0;
    asp->line_control_reg.bits.word_length = 3;

    asp->line_status_reg.all = 0;
    asp->line_status_reg.bits.tx_holding_empty = 1;
    asp->line_status_reg.bits.tx_shift_empty = 1;

    asp->break_state = FALSE;

    /*
     * set up modem control reg so set_modem_control 
     * will set host adapter to the reset state
     */
    asp->modem_control_reg.all = 0;

    asp->modem_control_reg.bits.DTR = TRUE;
    asp->modem_control_reg.bits.RTS = TRUE;
    asp->modem_control_reg.bits.OUT1 = TRUE;
    asp->modem_control_reg.bits.OUT2 = TRUE;

    asp->modem_ctrl_state = 0;      // different from DTR & RTS
    asp->loopback_state = FALSE;
    asp->divisor_latch_state = asp->line_control_reg.bits.DLAB;

    asp->modem_status_reg.all = 0;
    asp->modem_status_changed = TRUE;

    /*
     * Set up default state of our state variables
     */

    asp->receiver_line_status_interrupt_state = FALSE;
    asp->data_available_interrupt_state = FALSE;
    asp->tx_holding_register_empty_interrupt_state = FALSE;
    asp->modem_status_interrupt_state = FALSE;

    check_interrupt( asp );         /* sets interrupt line state */

    return;
}

void com_close(tAdapter adapter)
{
}


/*
 * =====================================================================
 * The Adaptor functions
 * =====================================================================
 */

static void inb_RBR( tAdapter adapter, BYTE *value )
{
    UART_STATE *asp = & UART_ADAPTER(adapter);

    if (asp->divisor_latch_state)
        *value = (BYTE)asp->divisor_latch.byte.LSB;
    else
    {
        host_com_lock(adapter);

        //
        // Read of rx buffer
        //
        check_data_available( adapter );               // get char if available

        *value = asp->rx_buff_reg;
        asp->line_status_reg.bits.data_ready = FALSE;  // signal character read

        check_data_available( adapter );               // check next char
        
        asp->data_available_interrupt_state = 
            (asp->int_enable_reg.bits.data_available
             && asp->line_status_reg.bits.data_ready);
        check_interrupt(asp);                          // set interrupt line

        host_com_unlock(adapter);
    }
}

static void inb_IER( tAdapter adapter, BYTE *value )
{
    UART_STATE *asp = & UART_ADAPTER(adapter);

    if (asp->divisor_latch_state)
        *value = (BYTE)asp->divisor_latch.byte.MSB;
    else
        *value = asp->int_enable_reg.all;
}

static void inb_IIR( tAdapter adapter, BYTE *value )
{
    UART_STATE *asp = & UART_ADAPTER(adapter);

    host_com_lock( adapter );

    if ((*value = generate_iir( adapter )) == UART_THRE_INT)
    {
        asp->tx_holding_register_empty_interrupt_state = FALSE;
        check_interrupt( asp ); 
    }
 
    host_com_unlock(adapter);
}

static void inb_LCR( tAdapter adapter, BYTE *value )
{
    *value = UART_ADAPTER(adapter).line_control_reg.all;
}

static void inb_MCR( tAdapter adapter, BYTE *value )
{
    *value = UART_ADAPTER(adapter).modem_control_reg.all;
}

static void inb_LSR( tAdapter adapter, BYTE *value )
{
    UART_STATE *asp = & UART_ADAPTER(adapter);
    host_com_lock(adapter);
 
    *value = asp->line_status_reg.all;

    asp->line_status_reg.bits.overrun_error = 0;
    asp->line_status_reg.bits.parity_error = 0;
    asp->line_status_reg.bits.framing_error = 0;
    asp->line_status_reg.bits.break_interrupt = 0;
    asp->receiver_line_status_interrupt_state = FALSE;
    check_interrupt(asp);

    host_com_unlock(adapter);
}

static void inb_MSR( tAdapter adapter, BYTE *value )
{
    UART_STATE *asp = & UART_ADAPTER(adapter);

    if (!asp->modem_status_changed && !asp->loopback_state)
    {
        *value = asp->last_modem_status_value.all;
    }
    else
    {
        host_com_lock(adapter);

        if (asp->loopback_state)
        {
            *value = asp->modem_status_reg.all;
            asp->modem_status_changed = TRUE;
        }
        else
        {
            refresh_modem_status( adapter );
            *value = asp->modem_status_reg.all;
            asp->modem_status_changed = FALSE;
        }

        asp->modem_status_reg.bits.delta_CTS = 0;
        asp->modem_status_reg.bits.delta_DSR = 0;
        asp->modem_status_reg.bits.delta_RLSD = 0;
        asp->modem_status_reg.bits.TERI = 0;
        asp->modem_status_interrupt_state = FALSE;
        check_interrupt(asp);
        asp->last_modem_status_value.all = asp->modem_status_reg.all;

        host_com_unlock(adapter);
    }
}

static void inb_SCR( tAdapter adapter, BYTE *value )
{
    *value = UART_ADAPTER(adapter).scratch; // Just read the value stored.
}

void com_inb( WORD port, BYTE *value )
{
    void (*(inb_func[]))( tAdapter, BYTE * ) =
    {
        inb_RBR,
        inb_IER,
        inb_IIR,
        inb_LCR,
        inb_MCR,
        inb_LSR,
        inb_MSR,
        inb_SCR
    };
    tAdapter adapter = adapter_for_port(port);

    if (adapter < NUM_SERIAL_PORTS)
        inb_func[port & 0x7] (adapter, value);
    else
        *value = 0xFF;
}


static void outb_THR( tAdapter adapter, BYTE value )
{
    const BYTE selectBits[4] = { 0x1f, 0x3f, 0x7f, 0xff } ;
    UART_STATE *asp = & UART_ADAPTER(adapter);

    host_com_lock(adapter);

    if (asp->divisor_latch_state)
    {
        if (asp->divisor_latch.byte.LSB != value)
        {
            asp->divisor_latch.byte.LSB = value;
            asp->baud_rate_changed = TRUE;
        }
    }
    else
    {
        /*
         * Write char to tx buffer
         */
        if (!asp->loopback_state)
        {
            asp->line_status_reg.bits.tx_shift_empty = 0;
            host_com_write_tx( adapter, value );
        }
        else
        {
            /* Loopback case requires masking off */
            /* of bits based upon word length.    */
            asp->rx_buff_reg = value 
                             & selectBits[ asp->line_control_reg.bits.word_length ];

            /*
             * Check for data overrun and set up correct interrupt
             */
            if ( asp->line_status_reg.bits.data_ready)
            {
                asp->line_status_reg.bits.overrun_error = TRUE;
                raise_rls_interrupt(asp);
            }
            else
            {
                asp->line_status_reg.bits.data_ready = TRUE;
                raise_rda_interrupt(asp);
            }
            asp->line_status_reg.bits.tx_shift_empty = 1;
        }

        // holding register permanently empty!!!
        asp->line_status_reg.bits.tx_holding_empty = 1;
        raise_thre_interrupt( asp );
    }

    host_com_unlock(adapter);
}

static void outb_IER( tAdapter adapter, BYTE value )
{
    UART_STATE *asp = & UART_ADAPTER(adapter);
    host_com_lock(adapter);

    if (asp->divisor_latch_state)
    {
        if (asp->divisor_latch.byte.MSB != value)
        {
            asp->divisor_latch.byte.MSB = value;
            asp->baud_rate_changed = TRUE;
        }
    }
    else
    {
        int org_da = asp->int_enable_reg.bits.data_available;

        asp->int_enable_reg.all = value & 0xf;

        //
        // Kill off any pending interrupts for those items
        // which are set now as disabled
        //
        if (!asp->int_enable_reg.bits.data_available)
            asp->data_available_interrupt_state = FALSE;

        if (!asp->int_enable_reg.bits.tx_holding)
            asp->tx_holding_register_empty_interrupt_state = FALSE;

        if (!asp->int_enable_reg.bits.rx_line)
            asp->receiver_line_status_interrupt_state = FALSE;

        if (!asp->int_enable_reg.bits.modem_status)
            asp->modem_status_interrupt_state = FALSE;

        //
        // Check for immediately actionable interrupts
        //
        check_data_available( adapter );
        if ( asp->line_status_reg.bits.data_ready == 1 )
            raise_rda_interrupt( asp );
        if ( asp->line_status_reg.bits.tx_holding_empty == 1 )
            raise_thre_interrupt( asp );

        check_interrupt(asp);       // lower int line if no outstanding interrupts

    }

    host_com_unlock(adapter);
}

static void outb_IIR( tAdapter adapter, BYTE value )
{
    //
    // Essentially a READ ONLY register
    //
}

static void outb_LCR( tAdapter adapter, BYTE value )
{
    const LINE_CONTROL_REG LCRFlushMask = 
    {
        (unsigned) ~0,    // word_length:2;
        (unsigned)  0,    // no_of_stop_bits:1;
        (unsigned) ~0,    // parity_enabled:1;
        (unsigned) ~0,    // even_parity:1;
        (unsigned) ~0,    // stick_parity:1;
        (unsigned)  0,    // set_break:1;
        (unsigned)  0     // DLAB:1;
    };
    UART_STATE *asp = & UART_ADAPTER(adapter);

    host_com_lock(adapter);
    if (((value ^ asp->line_control_reg.all) & LCRFlushMask.all) != 0)
        com_flush_input(adapter);

    if (asp->line_control_reg.all != value)
    {
        asp->line_control_reg.all = value;
        set_line_control( adapter );
    }

    if (asp->divisor_latch_state != asp->line_control_reg.bits.DLAB)
    {
        asp->divisor_latch_state = asp->line_control_reg.bits.DLAB;
        if (asp->divisor_latch_state)
            asp->baud_rate_changed = FALSE;
        else
        {
            if (asp -> baud_rate_changed)
            {
                set_baud_rate(adapter);
            }
        }
    }

    set_break(adapter);

    host_com_unlock(adapter);
}

static void outb_MCR( tAdapter adapter, BYTE value )
{
    UART_STATE *asp = & UART_ADAPTER(adapter);
    host_com_lock(adapter);

    // Optimisation - DOS keeps re-writing this register
    if ( asp->modem_control_reg.all != value )
    {
        asp->modem_control_reg.all = value;
        asp->modem_control_reg.bits.pad = 0;

        set_modem_control(adapter);
    }

    host_com_unlock(adapter);
}

static void outb_LSR( tAdapter adapter, BYTE value )
{
    UART_STATE *asp = & UART_ADAPTER(adapter);
    int temp;

    host_com_lock(adapter);

    temp = asp->line_status_reg.bits.tx_shift_empty;   /* READ ONLY */
    asp->line_status_reg.all = value;
    asp->line_status_reg.bits.tx_shift_empty = (BYTE_BIT_FIELD)temp;

    host_com_unlock(adapter);
}

static void outb_MSR( tAdapter adapter, BYTE value )
{
    UART_STATE *asp = & UART_ADAPTER(adapter);
    host_com_lock(adapter);

    //
    // Essentially a READ ONLY register, except that DR-DOS
    // writes to this reg after setting int on MSR change and
    // expects to get an interrupt back!!! So we will oblige.
    // Writing to this reg only seems to affect the delta bits
    // (bits 0-3) of the reg.
    //
    if (((value ^ asp->modem_status_reg.all) & 0xf) != 0)
    {
        asp->modem_status_reg.all &= 0xf0;
        asp->modem_status_reg.all |= value & 0xf;
        if (!asp->loopback_state)
            raise_ms_interrupt(asp);
    }

    host_com_unlock(adapter);
}

static void outb_SCR( tAdapter adapter, BYTE value )
{
    UART_STATE *asp = & UART_ADAPTER(adapter);
    asp->scratch = value;           // Scratch register - just store the value.
}

void com_outb(WORD port, BYTE value)
{
    void (*(outb_func[]))(tAdapter, BYTE) =
    {
        outb_THR,
        outb_IER,
        outb_IIR,
        outb_LCR,
        outb_MCR,
        outb_LSR,
        outb_MSR,
        outb_SCR
    };
    tAdapter adapter = adapter_for_port(port);

    if (adapter < NUM_SERIAL_PORTS)
    {
        outb_func[port & 0x7] (adapter, value);
    }
}


/********************************************************/

