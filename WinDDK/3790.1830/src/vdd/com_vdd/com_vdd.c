/*
 * Com_VDD.c - Windows NT serial port Virtual Device Driver
 *
 * This module handles the init/exit of the VDD.
 *
 * copyright 1992 by Microsoft Corporation
 *
 * revision history:
 *  24-Dec-1992 John Morgan:  written
 *
 */

#include "com_vdd.h"
#include "pc_com.h"
#include "nt_com.h"

#include <vddsvc.h>

/*
 *
 * VDDInitialize
 *
 * Arguments:
 *     DllHandle - Identifies this VDD
 *     Reason - Attach or Detach
 *     Context - Not Used
 *
 * Return Value:
 *     TRUE: SUCCESS
 *     FALSE: FAILURE
 *
 */
BOOL DllMain(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )
{
    VDD_IO_PORTRANGE PortDefs[NUM_SERIAL_PORTS] = {
        {0x3F8, 0x3FF},
        {0x2F8, 0x2FF},
        {0x3E8, 0x3EF},
        {0x2E8, 0x2EF}
    };

    VDD_IO_HANDLERS PortHandlers[NUM_SERIAL_PORTS] = {
        {com_inb, NULL, NULL, NULL, com_outb, NULL, NULL, NULL},
        {com_inb, NULL, NULL, NULL, com_outb, NULL, NULL, NULL},
        {com_inb, NULL, NULL, NULL, com_outb, NULL, NULL, NULL},
        {com_inb, NULL, NULL, NULL, com_outb, NULL, NULL, NULL}
    };

    switch ( Reason ) {

    case DLL_PROCESS_ATTACH:
        host_com_init();

        // Attach inb & outb functions to correct ports
        VDDInstallIOHook(DllHandle, 4, PortDefs, PortHandlers);

        break;

    case DLL_PROCESS_DETACH:
        // Free comm ports
        VDDDeInstallIOHook(DllHandle, 4, PortDefs);

        host_com_exit();

        break;

    default:
        break;
    }

    return TRUE;
}


/*
 *
 * VDDTerminateVDM
 *
 */
VOID VDDTerminateVDM( VOID )
{
    return;
}


/*
 *
 * VDDInit
 *
 */
VOID VDDInit( VOID )
{

    // Called from the BOP manager. If VDDInitialize has done all the
    // checking and resources alloaction, just return success.

    setCF( 0 );

    return;
}


/*
 *
 * VDDDispatch
 *
 * Arguments:
 *     Client (DX)    = Address / command code
 *                       This is essentially an extended port number.  The
 *                       lowest hex digit identifies a function, while the
 *                       upper two digits identify the port base address.
 *                      Command 0 is send buffer. It writes CX
 *                       characters from the buffer at DS:SI.
 *                      Command 1 is read buffer. It reads up to
 *                       CX characters into the buffer at ES:DI. CX is
 *                       returned as the number of characters read.
 *                      Command 7 is set baud rate.  It sets the baud
 *                       rate as passed in CX.
 *                      Commands 8-F access the virtual UART registers.
 *                       CX indicates how the register is to be changed.
 *                       CH indicates which bits to clear in the register,
 *                       while CL indicates which bits to complement. If
 *                       CX = 0 the register is read but not written. If
 *                       CH = 0xFF the value in CL is written to the register.
 *                       In all other cases the register is both read and
 *                       written to.  CL is always set to the value written
 *                       to the register if any, or the value read if not.
 *                       Note that 0 is always THR/RBR and 1 is IER.
 *                       Use command 7 to change the baud rate.
 *
 *     Client (CX)    = Port Data or Buffer Size
 *     Client (DS:SI) = Read Message Buffer
 *     Client (ES:DI) = Send Message Buffer
 *
 * Return Value:
 *     Client Carry Clear:  SUCCESS
 *     Client Carry Set:    FAILURE
 * 
 */
VOID VDDDispatch( VOID )
{
    USHORT  PortCommand;
    PCHAR   Buffer;
    tAdapter adapter;
    USHORT  PassCX;
    BYTE    temp;

    setCF( 0 );                         // Assume success

    PortCommand = getDX();
    adapter = adapter_for_port( (PortCommand & ~0xF) | 8 );

    if (adapter >= NUM_SERIAL_PORTS) {
        setCF( 1 );                     // ERROR: unknown serial port
    }
    else {
        PassCX = getCX();               // all commands us CX
        switch (PortCommand & 0xF) {
        case 0: // Write buffer
            Buffer = (PCHAR) GetVDMPointer((ULONG)(getDS()<<16|getSI()),PassCX,FALSE);
            break;
        case 1: // Read buffer
            Buffer = (PCHAR) GetVDMPointer((ULONG)(getES()<<16|getDI()),PassCX,FALSE);
            break;
        case 7: // Set baud rate
            break;
        case 8:  // THR & RBR
        case 9:  // IED
        case 10: // IIR
        case 11: // LCR
        case 12: // MCR
        case 13: // LSR
        case 14: // MSR
        case 15: // SCR
            if (PassCX == 0)
            {
                com_inb( PortCommand, &temp );
                // do not write to port
                setCL( temp );
            }
            else if ((PassCX & 0xFF00) == 0xFF00)
            {
                // do not read from port
                com_outb( PortCommand, (BYTE) PassCX );
            }
            else
            {
                com_inb( PortCommand, &temp );
                temp = (BYTE)((temp & ~(PassCX >> 8)) ^ PassCX);
                com_outb( PortCommand, temp );
                setCL( temp );
            }
            break;
        default: // unknown command
            break;
        }
    }

    return;
}


