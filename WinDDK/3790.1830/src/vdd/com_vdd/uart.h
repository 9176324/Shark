/*
 * UART.h
 *
 * Definitions for the NS8250 & compatible UARTs
 *
 * copyright 1990 by Insignia Solutions Ltd. Used by permission.
 *
 * revision history:
 *  24-Dec-1992 John Morgan:  written based on  UART.h  written by Paul Huckle
 *
 */

#ifndef _UART_H
#define _UART_H

/*
 * ============================================================================
 * Structure/Data definitions
 * ============================================================================
 */

/* register type definitions follow: */
#ifndef WORD_BIT_FIELD
#define WORD_BIT_FIELD WORD
#endif

#ifndef BYTE_BIT_FIELD
#define BYTE_BIT_FIELD BYTE
#endif

typedef BYTE BUFFER_REG;

typedef union {
   WORD all;
   struct {
      WORD_BIT_FIELD LSB:8;
      WORD_BIT_FIELD MSB:8;
   } byte;
} DIVISOR_LATCH;

typedef union {
    struct {
         BYTE_BIT_FIELD data_available:1;
         BYTE_BIT_FIELD tx_holding:1;
         BYTE_BIT_FIELD rx_line:1;
         BYTE_BIT_FIELD modem_status:1;
         BYTE_BIT_FIELD pad:4;
           } bits;
    BYTE all;
      } INT_ENABLE_REG;

// define the meaning of IIR values instead of defining its structure
#define UART_RLS_INT    6
#define UART_RDA_INT    4
#define UART_THRE_INT   2
#define UART_MS_INT     0
#define UART_NO_INT     1

typedef union {
    struct {
         BYTE_BIT_FIELD word_length:2;
         BYTE_BIT_FIELD no_of_stop_bits:1;
         BYTE_BIT_FIELD parity_enabled:1;
         BYTE_BIT_FIELD even_parity:1;
         BYTE_BIT_FIELD stick_parity:1;
         BYTE_BIT_FIELD set_break:1;
         BYTE_BIT_FIELD DLAB:1;
           } bits;
    BYTE all;
      } LINE_CONTROL_REG;

typedef union {
    struct {
         BYTE_BIT_FIELD DTR:1;
         BYTE_BIT_FIELD RTS:1;
         BYTE_BIT_FIELD OUT1:1;
         BYTE_BIT_FIELD OUT2:1;
         BYTE_BIT_FIELD loop:1;
         BYTE_BIT_FIELD pad:3;
           } bits;
    BYTE all;
      } MODEM_CONTROL_REG;

typedef union {
    struct {
         BYTE_BIT_FIELD data_ready:1;
         BYTE_BIT_FIELD overrun_error:1;
         BYTE_BIT_FIELD parity_error:1;
         BYTE_BIT_FIELD framing_error:1;
         BYTE_BIT_FIELD break_interrupt:1;
         BYTE_BIT_FIELD tx_holding_empty:1;
         BYTE_BIT_FIELD tx_shift_empty:1;
         BYTE_BIT_FIELD pad:1;
           } bits;
    BYTE all;
      } LINE_STATUS_REG;

typedef union {
    struct {
         BYTE_BIT_FIELD delta_CTS:1;
         BYTE_BIT_FIELD delta_DSR:1;
         BYTE_BIT_FIELD TERI:1;
         BYTE_BIT_FIELD delta_RLSD:1;
         BYTE_BIT_FIELD CTS:1;
         BYTE_BIT_FIELD DSR:1;
         BYTE_BIT_FIELD RI:1;
         BYTE_BIT_FIELD RLSD:1;
           } bits;
    BYTE all;
      } MODEM_STATUS_REG;

/* register select code definitions follow: */

#define UART_TX_RX     0
#define UART_THR       0
#define UART_RBR       0
#define UART_IER       1
#define UART_IIR       2
#define UART_LCR       3
#define UART_MCR       4
#define UART_LSR       5
#define UART_MSR       6
#define UART_SCRATCH   7

#define UART_PARITY_ON 1   /* line control setting for parity enabled */
#define UART_PARITY_OFF 0  /* line control setting for parity disabled */

#define UART_PARITY_ODD 0  /* line control setting for odd parity */
#define UART_PARITY_EVEN 1 /* line control setting for even parity */

#define UART_PARITY_STICK 1  /* line control setting for stick(y) parity */

#endif /* _UART_H */

