/*
 * PC_com.h
 *
 * function and structure definitions used external to PC_com.c
 *
 * copyright 1992 by Microsoft Corporation
 * portions copyright 1991 by Insignia Solutions Ltd., used by permission.
 *
 * revision history:
 *  24-Dec-1992 John Morgan:  written based on  com.h  from Insignia Solutions
 *
 */

#define NUM_SERIAL_PORTS 4

typedef unsigned tAdapter;
#define INVALID_UART ((tAdapter) ~0)
#define adapter_for_port(port) \
    (((port & 0x3F8) == 0x3F8) ? COM1 : \
     ((port & 0x3F8) == 0x2F8) ? COM2 : \
     ((port & 0x3F8) == 0x3E8) ? COM3 : \
     ((port & 0x3F8) == 0x2E8) ? COM4 : INVALID_UART)

#include "UART.h"

typedef struct
{
        BUFFER_REG          rx_buff_reg;
        DIVISOR_LATCH       divisor_latch;
        INT_ENABLE_REG      int_enable_reg;
        LINE_CONTROL_REG    line_control_reg;
        MODEM_CONTROL_REG   modem_control_reg;
        LINE_STATUS_REG     line_status_reg;
        MODEM_STATUS_REG    modem_status_reg;
        BYTE                scratch;           /* scratch register */

        MODEM_STATUS_REG    last_modem_status_value;
        BOOLEAN             modem_status_changed;
        BOOLEAN             divisor_latch_state;
        BOOLEAN             baud_rate_changed;

        BOOLEAN             break_state;        /* either OFF or ON */

        BYTE                modem_ctrl_state;
        BOOLEAN             loopback_state;     /* either OFF or ON */
        BOOLEAN             out2_state;         /* either OFF or ON */

        BOOLEAN             receiver_line_status_interrupt_state;
        BOOLEAN             data_available_interrupt_state;
        BOOLEAN             tx_holding_register_empty_interrupt_state;
        BOOLEAN             modem_status_interrupt_state;
        BOOLEAN             interrupt_line_state;
        
        BYTE                hw_interrupt_priority;
        BYTE                com_baud_ind;
        BOOLEAN             had_first_read;

} UART_STATE;


extern void com_init( tAdapter );
extern void com_close( tAdapter );
extern void com_reset( tAdapter );

extern void com_flush_printer( tAdapter );

extern void com_inb( WORD, BYTE * );
extern void com_outb( WORD, BYTE );

extern void com_recv_char( tAdapter );
extern void com_modem_change( tAdapter );

extern void com_int_data( tAdapter, int *, int * );

extern void clear_tbr_flag( UART_STATE * );
extern void set_modem_control( tAdapter );

extern void raise_rls_interrupt( UART_STATE * );
extern void raise_rda_interrupt( UART_STATE * );
extern void raise_ms_interrupt( UART_STATE * );
extern void raise_thre_interrupt( UART_STATE * );

#if 0
extern BOOL check_rda_interrupt( UART_STATE * );
#endif
#define check_rda_interrupt(pUART) ((pUART)->data_available_interrupt_state)

#define COM1 0
#define COM2 1
#define COM3 2
#define COM4 3


