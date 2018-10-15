/*
 * PC_def.h
 *
 * serial port definitions used external to PC_com.c
 *
 * copyright 1992 by Microsoft Corporation
 * portions copyright 1991 by Insignia Solutions Ltd., used by permission.
 *
 * revision history:
 *  24-Dec-1992 John Morgan:  written based on  XT.h  from Insignia Solutions
 *
 */

#define PC_BIOS_VAR_START 0x400

#define PC_COM1_PORT_START 0x3F8
#define PC_COM2_PORT_START 0x2F8
#define PC_COM3_PORT_START 0x3E8
#define PC_COM4_PORT_START 0x2E8

#define PC_COM1_PORT_END 0x3FF
#define PC_COM2_PORT_END 0x2FF
#define PC_COM3_PORT_END 0x3EF
#define PC_COM4_PORT_END 0x2EF

#define PC_COM1_INT 4
#define PC_COM2_INT 3
#define PC_COM3_INT 4
#define PC_COM4_INT 3

#define PC_COM1_TIMEOUT (PC_BIOS_VAR_START + 0x7c)
#define PC_COM2_TIMEOUT (PC_BIOS_VAR_START + 0x7d)
#define PC_COM3_TIMEOUT (PC_BIOS_VAR_START + 0x7e)
#define PC_COM4_TIMEOUT (PC_BIOS_VAR_START + 0x7f)


