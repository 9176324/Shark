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

#pragma once

#include "AnlgCaptureFilterInterface.h"
#include "AnlgXBarFilterInterface.h"
#include "AnlgTVAudioFilterInterface.h"
#include "AnlgTunerFilterInterface.h"
#include "DgtlCaptureFilterInterface.h"
#include "DgtlTunerFilterInterface.h"

NTSTATUS Add( IN PKSDEVICE pKSDevice );

NTSTATUS Start( IN          PKSDEVICE         pKSDevice,
                IN          PIRP              pIrp,
                IN OPTIONAL PCM_RESOURCE_LIST pTranslatedResourceList,
                IN OPTIONAL PCM_RESOURCE_LIST pUntranslatedResourceList );

void Stop( IN PKSDEVICE pKSDevice,
           IN PIRP      pIrp );

void Remove( IN PKSDEVICE pKSDevice,
             IN PIRP      pIrp );

void SetPower( IN PKSDEVICE          pDevice,
               IN PIRP               pIrp,
               IN DEVICE_POWER_STATE To,
               IN DEVICE_POWER_STATE From );

//NTSTATUS QueryInterface(IN PKSDEVICE Device,
//                      IN PIRP Irp);

BOOLEAN InterruptService( PKINTERRUPT pInterruptObject,
                          PVOID       pServiceContext);

void DPCService( IN PKDPC pDpc,
                 IN PVOID pDeferredContext,
                 IN PVOID pSystemArgument1,
                 IN PVOID pSystemArgument2 );

CVampDeviceResources* GetVampDevResObj
(
    IN  PVOID pKSObject //Pointer to KS object structure.
);

//
// Description:
//  Device Dispatch Table.
//  Lists the dispatch routines for the major events in the life
//  of the capture device.
//
const KSDEVICE_DISPATCH CapDeviceDispatch =
{
    Add,            // Add
    Start,          // Start
    NULL,           // PostStart
    NULL,           // QueryStop
    NULL,           // CancelStop
    Stop,           // Stop
    NULL,           // QueryRemove
    NULL,           // CancelRemove
    Remove,         // Remove
    NULL,           // QueryCapabilities
    NULL,           // SurpriseRemoval
    NULL,           // QueryPower
    SetPower,       // SetPower
    NULL		    // QueryInterface
};


//
// Description:
//  Device Descriptor.
//  It describes the device with all of its dispatch functions and filters.
//
const KSDEVICE_DESCRIPTOR DeviceDescriptor =
{
    &CapDeviceDispatch,
    0,
    NULL, //All the filter descriptors are created dynamically
    KSDEVICE_DESCRIPTOR_VERSION
};


