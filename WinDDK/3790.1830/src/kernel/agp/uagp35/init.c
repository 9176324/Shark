/*++

Copyright (2) 2002 Microsoft Corporation

Module Name:

    init.c

Abstract:

    This module contains the initialization support routines for the
    MS AGP v3 Filter Driver

Author:

    Eric F. Nelson (enelson) June 6, 2002

Revision History:

--*/

#include "agp.h"
#include "uagp35.h"

ULONG AgpExtensionSize = sizeof(UAGP35_EXTENSION);
PAGP_FLUSH_PAGES AgpFlushPages = NULL; // Not implemented (yet)

#define MIN(A, B) (((A) < (B)) ? (A): (B))


NTSTATUS
AgpInitializeTarget(
    IN PVOID AgpExtension
    )
/*++

Routine Description:

    Entrypoint for target initialization, this is called first

Arguments:

    AgpExtension - Supplies our AGP3 extension

Return Value:

    STATUS_SUCCESS

--*/
{
    ULONG DeviceId;
    NTSTATUS Status;
    PCI_AGP_CAPABILITY TargetCap;
    PUAGP35_EXTENSION Extension = AgpExtension;

    //
    // 40-bit address support
    //
#ifdef _WIN64
    AgpMaximumAddress.QuadPart = 0xFFFFFFFFFF;
#endif

    //
    // Zero/init our AGP3 extension
    //
    RtlZeroMemory(AgpExtension, sizeof(UAGP35_EXTENSION));

    //
    // Figure out what type of AGP3 capability our target is
    //
    Status = AgpLibGetTargetCapability(AgpExtension, &TargetCap);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("UAGP35TargetInit: AgpLibGetTargetCapability "
                "failed %08lx\n"));
        return Status;
    }
    Extension->CapabilityId = TargetCap.Header.CapabilityID;

    //
    // Make sure the capability is v3!!!
    //
    if ((TargetCap.Major < 3) ||
        ((TargetCap.Major == 3) && (TargetCap.Minor < 5))) {
        AGPLOG(AGP_CRITICAL,
               ("UAGP35TargetInit: Capability version (%d) != 3\n",
                TargetCap.Major));
        return STATUS_NOT_SUPPORTED;
    }

    //
    // Save the vendor and device IDs in case we need to "tweak"
    // something for a particular platform
    //
    AgpLibReadAgpTargetConfig(AgpExtension, &DeviceId, 0, sizeof(DeviceId));
    Extension->DeviceId = DeviceId;
   
    //
    // Register verifier callbacks
    //
    AgpVerifierRegisterPlatformCallbacks(AgpExtension,
                                         AgpV_PlatformInit,
                                         AgpV_PlatformWorker,
                                         AgpV_PlatformStop);

    return STATUS_SUCCESS;
}



NTSTATUS
AgpInitializeMaster(
    IN  PVOID AgpExtension,
    OUT ULONG *AgpCapabilities
    )
/*++

Routine Description:

    Entrypoint for master initialization, this is called after target
    initialization and should be used to initialize the AGP capabilities
    of both master and target

    This is also called when the master transitions into the D0 state

Arguments:

    AgpExtension - Supplies our AGP3 extension

    AgpCapabilities - Returns the capabilities of this AGP device

Return Value:

    STATUS_SUCCESS, or an appropriate error status

--*/
{
    NTSTATUS Status;
    PCI_AGP_CAPABILITY MasterCap;
    PCI_AGP_CAPABILITY TargetCap;
    ULONG ApertureBase;
    PUAGP35_EXTENSION Extension = AgpExtension;
    ULONG SBAEnable;
    ULONG DataRate;
    ULONG FourGBEnable = OFF;
    ULONG FastWrite;
    BOOLEAN ReverseInit;
   
#if DBG
    PCI_AGP_CAPABILITY CurrentCap;
#endif

    //
    // Get the master and target AGP capabilities
    //
    Status = AgpLibGetMasterCapability(AgpExtension, &MasterCap);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("UAGP35MasterInit: AgpLibGetMasterCapability failed %08lx\n"));
        return Status;
    }

    Status = AgpLibGetTargetCapability(AgpExtension, &TargetCap);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("UAGP35MasterInit: AgpLibGetTargetCapability "
                "failed %08lx\n"));
        return Status;
    }

    //
    // Determine the greatest common denominator for data rate.
    //
    DataRate = TargetCap.AGPStatus.Rate & MasterCap.AGPStatus.Rate;

    AGP_ASSERT(DataRate != 0);

    //
    // Some broken cards (Matrox Millenium II "AGP") report no valid
    // supported transfer rates. These are not really AGP cards. They
    // have an AGP Capabilities structure that reports no capabilities.
    //
    if (DataRate == 0) {
        AGPLOG(AGP_CRITICAL,
               ("UAGP35MasterInit: AgpLibGetMasterCapability returned "
                "no valid transfer rate\n"));
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    //
    // Select the highest common rate
    //
    if (DataRate & PCI_AGP_RATE_4X) {
        DataRate = PCI_AGP_RATE_4X;
    } else if (DataRate & PCI_AGP_RATE_2X) {
        DataRate = PCI_AGP_RATE_2X;
    } else if (DataRate & PCI_AGP_RATE_1X) {
        DataRate = PCI_AGP_RATE_1X;
    }

    //
    // Previously a call was made to change the rate (successfully),
    // use this rate again now
    //
    if (Extension->SpecialTarget & AGP_FLAG_SPECIAL_RESERVE) {
        DataRate = (ULONG)((Extension->SpecialTarget & 
                            AGP_FLAG_SPECIAL_RESERVE) >>
                           AGP_FLAG_SET_RATE_SHIFT);

        //
        // If we're in AGP3 mode, and our rate was successfully
        // programmed, then we must convert into AGP2 rate bits
        //
        if (TargetCap.AGPStatus.Agp3Mode == ON) {
            ASSERT(MasterCap.AGPStatus.Agp3Mode == ON);
            ASSERT((DataRate == 8) || (DataRate == PCI_AGP_RATE_4X));
            DataRate >>= 2;
        }
    }

    //
    // Enable SBA if both master and target support it
    //
    SBAEnable = TargetCap.AGPStatus.SideBandAddressing &
        MasterCap.AGPStatus.SideBandAddressing;

    //
    // Enable FastWrite if both master and target support it
    //
    FastWrite = TargetCap.AGPStatus.FastWrite &
        MasterCap.AGPStatus.FastWrite;

    //
    // Enable 4GB addressing if aperture is 64-bit
    //    
    AgpLibReadAgpTargetConfig(AgpExtension,
                              &ApertureBase,
                              APERTURE_BASE,
                              sizeof(ApertureBase));
    
    if (ApertureBase & APERTURE_BASE64_MASK) {
        FourGBEnable = ((TargetCap.AGPStatus.FourGB == ON) &&
                        (MasterCap.AGPStatus.FourGB == ON));
    } else {
        FourGBEnable = FALSE;
    }
    Extension->FourGBEnable = FourGBEnable ? ON: OFF;

    //
    // Indicate whether we can map memory through the GART aperture
    //
    *AgpCapabilities = (TargetCap.AGPStatus.HostTransDisable == ON) ? 0:
        AGP_CAPABILITIES_MAP_PHYSICAL;

    //
    // Check for cached coherent support
    //
    if (TargetCap.AGPStatus.ITA_Coherent == ON) {
        Extension->CoherentEnable = TRUE;
    }

    MasterCap.AGPCommand.Rate = DataRate;
    TargetCap.AGPCommand.Rate = DataRate;
    MasterCap.AGPCommand.AGPEnable = ON;
    TargetCap.AGPCommand.AGPEnable = ON;
    MasterCap.AGPCommand.SBAEnable = SBAEnable;
    TargetCap.AGPCommand.SBAEnable = SBAEnable;
    MasterCap.AGPCommand.FastWriteEnable = FastWrite;
    TargetCap.AGPCommand.FastWriteEnable = FastWrite;
    MasterCap.AGPCommand.FourGBEnable = FourGBEnable;
    TargetCap.AGPCommand.FourGBEnable = FourGBEnable;  
    MasterCap.AGPCommand.RequestQueueDepth =
        TargetCap.AGPStatus.RequestQueueDepthMaximum;
    MasterCap.AGPCommand.AsyncReqSize = 
        TargetCap.AGPStatus.AsyncRequestSize;
    TargetCap.AGPCommand.CalibrationCycle =
        MIN((TargetCap.AGPStatus.CalibrationCycle),
            (MasterCap.AGPStatus.CalibrationCycle));

    //
    // Patch rate for early rev VIA 8X silicon errata
    //
    if ((Extension->SpecialTarget & AGP_FLAG_SPECIAL_VIA_AGP2_RATE_PATCH) &&
        (TargetCap.AGPStatus.Agp3Mode == OFF)) {
        switch (DataRate) {
            case PCI_AGP_RATE_1X:
            case PCI_AGP_RATE_2X:
                MasterCap.AGPCommand.Rate = PCI_AGP_RATE_1X;
                TargetCap.AGPCommand.Rate = PCI_AGP_RATE_4X;
                break;
           case PCI_AGP_RATE_4X:
               MasterCap.AGPCommand.Rate = PCI_AGP_RATE_4X;
               TargetCap.AGPCommand.Rate = PCI_AGP_RATE_1X;
        }
        AGPLOG(AGP_WARNING,
               ("UAGP35MasterInit: AGP_FLAG_SPECIAL_VIA_AGP2_RATE_PATCH\n"));
    }

    //
    // Enable the Master first if the reverse init bit is set
    //
    ReverseInit = 
        (Extension->SpecialTarget & AGP_FLAG_REVERSE_INITIALIZATION) ==
        AGP_FLAG_REVERSE_INITIALIZATION;
    if (ReverseInit) {
        Status = AgpLibSetMasterCapability(AgpExtension, &MasterCap);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("UAGP35MasterInit: AgpLibSetMasterCapability %08lx "
                    "failed %08lx\n",
                    &MasterCap,
                    Status));
            return Status;
        }
    }

    //
    // Otherwise enable the Target first
    //
    Status = AgpLibSetTargetCapability(AgpExtension, &TargetCap);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("UAGP35MasterInit: AgpLibSetTargetCapability %08lx "
                "for target failed %08lx\n",
                &TargetCap,
                Status));
        return Status;
    }

    if (!ReverseInit) {
        Status = AgpLibSetMasterCapability(AgpExtension, &MasterCap);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("UAGP35MasterInit: AgpLibSetMasterCapability %08lx "
                    "failed %08lx\n",
                    &MasterCap,
                    Status));
        }
    }

#if DBG
    //
    // Read them back, see if it worked
    //
    Status = AgpLibGetMasterCapability(AgpExtension, &CurrentCap);
    AGP_ASSERT(NT_SUCCESS(Status));

    //
    // If the target request queue depth is greater than the master will
    // allow, it will be trimmed, so loosen the assert to not require an
    // exact match
    //
    // We'll do the same for asynchronous request queue depth too
    //
    AGP_ASSERT(CurrentCap.AGPCommand.RequestQueueDepth <=
               MasterCap.AGPCommand.RequestQueueDepth);
    AGP_ASSERT(CurrentCap.AGPCommand.AsyncReqSize <=
               MasterCap.AGPCommand.AsyncReqSize);

    CurrentCap.AGPCommand.RequestQueueDepth =
        MasterCap.AGPCommand.RequestQueueDepth;
    CurrentCap.AGPCommand.AsyncReqSize = MasterCap.AGPCommand.AsyncReqSize;

    AGP_ASSERT(RtlEqualMemory(&CurrentCap.AGPCommand,
                              &MasterCap.AGPCommand,
                              sizeof(CurrentCap.AGPCommand)));

    AGPLOG(AGP_NOISE,
           ("UAGP35MasterInit: WroteMasterCmd=%08x, ReadMasterCmd=%08lx\n",
            MasterCap.AGPCommand,
            CurrentCap.AGPCommand));

    Status = AgpLibGetTargetCapability(AgpExtension, &CurrentCap);

    AGP_ASSERT(NT_SUCCESS(Status));
    AGP_ASSERT(RtlEqualMemory(&CurrentCap.AGPCommand,
                              &TargetCap.AGPCommand,
                              sizeof(CurrentCap.AGPCommand)));

    AGPLOG(AGP_NOISE,
           ("UAGP35MasterInit: WroteTargetCmd=%08x, ReadTargetCmd=%08lx\n",
            TargetCap.AGPCommand,
            CurrentCap.AGPCommand));
#endif

    return Status;
}

ULONGLONG
AgpQueryImageHack(
    )

/*++

Routine Description:

    This routines return global image hack that the library must implement
    in order for the resulting image to work properly. These hack are based
    on the image and don't require registry entries to have been setup.

Arguments:

    None.

Return Value:

    Returns the image hack for this image. 

--*/

{
    return 0;
}

