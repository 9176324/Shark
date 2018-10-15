/*++

Copyright (c) 2002  Microsoft Corporation

Module Name:

    ntddsd.h

Abstract:

    This is the include file that defines the SD (aka Secure Digital)
    bus driver interface.

Author:

    Neil Sandlin (neilsa) 01/01/2002

--*/

#ifndef _NTDDSDH_
#define _NTDDSDH_

#if _MSC_VER > 1000
#pragma once
#endif

#include <sddef.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// Use this version in the interface_data structure
//

#define SDBUS_INTERFACE_VERSION 0x101

//
// Prototype of the callback routine which is called for SDIO
// devices to reflect device interrupts.
//

typedef
VOID
(*PSDBUS_CALLBACK_ROUTINE) (
    IN PVOID CallbackRoutineContext,
    IN ULONG InterruptType
    );

#define SDBUS_INTTYPE_DEVICE    0

//
// The interface parameters structure is passed to the InitializeInterface
// function in the SDBUS_INTERFACE_STANDARD
//

typedef struct _SDBUS_INTERFACE_PARAMETERS {
    USHORT Size;
    USHORT Reserved;

    //
    // Points to next lower device object in the device stack.
    //
    PDEVICE_OBJECT TargetObject;

    //
    // This flag indicates whether the caller expects any device
    // interrupts from the device.
    //
    BOOLEAN DeviceGeneratesInterrupts;

    //
    // The caller can specify here the IRQL at which the callback
    // function is entered. If this value is TRUE, the callback will
    // be entered at DPC level. If this value is FALSE, the callback
    // will be entered at passive level.
    //
    // Specifying TRUE will generally lower latency time of the interrupt
    // delivery, at the cost of complicating the device driver, which
    // must then deal with running at different IRQLs.
    //
    BOOLEAN CallbackAtDpcLevel;

    //
    // When an IO device interrupts, the SD bus driver will generate a
    // callback to this routine. The caller must supply this pointer if
    // the field DeviceGeneratesInterrupts was set to TRUE;
    //
    PSDBUS_CALLBACK_ROUTINE CallbackRoutine;

    //
    // The caller can specify here a context value which will be passed
    // to the device interrupt callback routine.
    //
    PVOID CallbackRoutineContext;

} SDBUS_INTERFACE_PARAMETERS, *PSDBUS_INTERFACE_PARAMETERS;

//
// Prototype of function used to initialize the the interface once
// it has been opened.
//

typedef
NTSTATUS
(*PSDBUS_INITIALIZE_INTERFACE_ROUTINE)(
    IN PVOID Context,
    IN PSDBUS_INTERFACE_PARAMETERS InterfaceParameters
    );


//
// Prototype of routine that the function driver must call
// when the function driver has completed its interrupt processing
//

typedef
NTSTATUS
(*PSDBUS_ACKNOWLEDGE_INT_ROUTINE)(
    IN PVOID Context
    );


//
// SDBUS_INTERFACE_STANDARD
//
// Interface Data structure used in the SdBusOpenInterface call. This
// structure defines the communication path between the SD function
// driver and the bus driver.
//

DEFINE_GUID( GUID_SDBUS_INTERFACE_STANDARD, 0x6bb24d81L, 0xe924, 0x4825, 0xaf, 0x49, 0x3a, 0xcd, 0x33, 0xc1, 0xd8, 0x20 );


typedef struct _SDBUS_INTERFACE_STANDARD {
    USHORT Size;
    USHORT Version;
    //
    // Interface Context
    //
    // This context pointer is filled in by the bus driver during the call
    // to SdBusOpenInterface(). This data must be passed as a parameter to
    // the other interface functions.
    //
    PVOID Context;

    //
    // Interface Reference/Dereference maintain the interface's reference
    // count. The reference count is automatically incremented to 1 when
    // the interface is opened. The caller must dereference the interface
    // under the following circumstances:
    //  a) the function driver receives a query_remove IRP
    //  b) the function driver receives a surprise_remove IRP
    //  c) the function driver receives a remove IRP without either a
    //     query_remove or a surprise_remove.
    //
    // In all cases, the function driver should dereference the interface
    // before passing the respective IRP down to the bus driver.
    //
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

    //
    // After the interface has been opened, the bus driver will have filled
    // in the pointer to the following routine which allows the function
    // driver to set the parameters of the interface.
    //

    PSDBUS_INITIALIZE_INTERFACE_ROUTINE InitializeInterface;

    //
    // NTSTATUS SdBusAcknowledgeInterrupt(InterfaceContext)
    //
    // This routine is filled in by the call to SdBusOpenInterface(),
    // and is used by the function driver to indicate when processing
    // of an IO device interrupt is complete.
    //
    // When an IO function of an SD device asserts an interrupt, the bus
    // driver will disable that interrupt to allow I/O operations to be
    // sent to the device at a reasonable IRQL (that is, <=DISPATCH_LEVEL).
    //
    // When the function driver's callback routine, which is equivalent to
    // an ISR, is done clearing the function's interrupt, this routine should
    // be called to re-enable the IRQ for card interrupts.
    //
    // Callers must be running at IRQL <= DISPATCH_LEVEL.
    //
    // Returns STATUS_UNSUCCESSFUL if the InterfaceContext pointer is invalid.
    //

    PSDBUS_ACKNOWLEDGE_INT_ROUTINE AcknowledgeInterrupt;

} SDBUS_INTERFACE_STANDARD, *PSDBUS_INTERFACE_STANDARD;


//
// SdBusOpenInterface()
//
// This routine establishes a connection to the SD bus driver.
// It should be called in the AddDevice routine with the FDO for
// the device stack is created.
//
// If STATUS_SUCCESS is returned, then the InterfaceData structure
// is filled in with valid function pointers and an interface context
// parameter. The context pointer must be used in all other SD bus
// driver calls.
//
// Callers must be running at IRQL < DISPATCH_LEVEL.
//

NTSTATUS
SdBusOpenInterface(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PSDBUS_INTERFACE_STANDARD InterfaceStandard,
    IN USHORT Size,
    IN USHORT Version
    );


//
// Definitions for properties used in Get/Set Property requests
//

typedef enum {
    SDP_MEDIA_CHANGECOUNT,
    SDP_MEDIA_STATE,
    SDP_WRITE_PROTECTED,
    SDP_FUNCTION_NUMBER
} SDBUS_PROPERTY;

//
// Media states defined for SDP_MEDIA_STATE
//

typedef enum {
    SDPMS_NO_MEDIA = 0,
    SDPMS_MEDIA_INSERTED
} SDPROP_MEDIA_STATE;

//
// Data structures for request packets
//

typedef enum {
    SDRF_GET_PROPERTY,
    SDRF_SET_PROPERTY,
    SDRF_DEVICE_COMMAND
} SD_REQUEST_FUNCTION;


//
// SDBUS_REQUEST_PACKET
//
// This structure specifies parameters for individual requests
// and commands sent to the bus driver by SdBusSubmitRequest()
//

typedef struct _SDBUS_REQUEST_PACKET {

    //
    // specifies the parameters for the operation, and how they
    // are interpreted
    //
    SD_REQUEST_FUNCTION RequestFunction;

    //
    // These context fields are provided for the optional use of the caller.
    // They are not referenced by the bus driver
    //
    PVOID UserContext[3];

    //
    // Information from the operation. Its usage is equivalent to
    // the Irp->IoStatus.Information field. For example, the length
    // of data transmitted for read/write operations will be filled
    // in here by the bus driver.
    //
    ULONG_PTR Information;

    //
    // Response data and length is returned by the device. Maximum
    // returned is 16 bytes. The content of this field is defined
    // in the SD spec.
    //
    union {
        UCHAR AsUCHAR[16];
        ULONG AsULONG[4];
        SDRESP_TYPE3 Type3;
    } ResponseData;
    UCHAR ResponseLength;

    //
    // Parameters to the individual functions
    //
    union {

        //
        // The property functions allow the caller to control aspects of
        // bus driver operation.
        //

        struct {
            SDBUS_PROPERTY Property;
            PVOID Buffer;
            ULONG Length;
        } GetSetProperty;

        //
        // DeviceCommand is the 'pipe' that allows SD device codes and arguments
        // to be executed. These codes are either defined in the SD spec,
        // can be based per device class, or can also be proprietary.
        //

        struct {
            SDCMD_DESCRIPTOR CmdDesc;
            ULONG Argument;
            PMDL Mdl;
            ULONG Length;
        } DeviceCommand;

    } Parameters;


} SDBUS_REQUEST_PACKET, *PSDBUS_REQUEST_PACKET;



//
// SdBusSubmitRequest()
// SdBusSubmitRequestAsync()
//
// These routines send SD request packets to the bus driver. The SubmitRequest
// version is synchronous, and must be called at IRQL < DISPATCH_LEVEL. The
// SubmitRequestAsync version is asynchronous, and the user must supply an IRP
// and completion routine.
//
// Arguments:
//
//  InterfaceContext - this is the context pointer returned by the
//                     SdBusOpenInterface() call.
//
//  Packet           - The parameters of the request are specified in this
//                     SDBUS_REQUEST_PACKET, which is allocated by the caller.
//
//  Irp              - The passed IRP will be used to transfer the request
//                     packet to the bus driver. It can either be a new irp
//                     allocated by the caller just for this request, or it can
//                     also be an IRP the caller is processing. The packet will
//                     be placed in the Parameters.Others.Argument1 of the IRP.
//
//  CompletionRoutine- When the request is completed by the bus driver, this
//                     completion routine will be called.
//
//  UserContext      - This context will be passed to the specified CompletionRoutine
//
// Notes:
//
//  Callers of SdBusSubmitRequest must be running at IRQL < DISPATCH_LEVEL.
//  Callers of SdBusSubmitRequestAsync must be running at IRQL <= DISPATCH_LEVEL.
//

NTSTATUS
SdBusSubmitRequest(
    IN PVOID InterfaceContext,
    IN PSDBUS_REQUEST_PACKET Packet
    );

NTSTATUS
SdBusSubmitRequestAsync(
    IN PVOID InterfaceContext,
    IN PSDBUS_REQUEST_PACKET Packet,
    IN PIRP Irp,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine,
    IN PVOID UserContext
    );


//
// IoControlCode values used in this interface. There normally should be no
// need to use these values directly, as SDBUS.LIB provides the support routines
// which issue these IOCTLs to the bus driver.
//
//

#define IOCTL_SDBUS_BASE                FILE_DEVICE_CONTROLLER
#define IOCTL_SD_SUBMIT_REQUEST         CTL_CODE(IOCTL_SDBUS_BASE, 3100, METHOD_NEITHER, FILE_ANY_ACCESS)


#ifdef __cplusplus
}
#endif
#endif

