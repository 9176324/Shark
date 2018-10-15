/*++

Copyright (2) 2003 Microsoft Corporation

Module Name:

    verifier.c

Abstract:

    This module implements the AGP verifier support functions

Author:

    Eric F. Nelson (enelson) Febraury 19, 2003

Revision History:

--*/

#include "agplib.h"

VOID
AgpVerifierWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AgpVerifierRegisterPlatformCallbacks)
#pragma alloc_text(PAGE, AgpVerifierInitialize)
#pragma alloc_text(PAGE, AgpVerifierStop)
#pragma alloc_text(PAGE, AgpVerifierWorker)
#pragma alloc_text(PAGE, AgpVerifierUpdateFlags)
#pragma alloc_text(PAGE, AgpVerifierPowerCallback)
#endif

static PVOID AgpV_PowerCallbackHandle = NULL;

//
// External function prototypes
//
NTSTATUS
ApGetSetDeviceBusData(
    IN PCOMMON_EXTENSION Extension,
    IN BOOLEAN Read,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );


__forceinline
NTSTATUS
AgpVerifierCreatePowerCallback(
    IN PCALLBACK_OBJECT *Callback,
    IN PCALLBACK_FUNCTION CallbackFunction,
    IN PVOID Context
    )
/*++

Routine Description:

    Allocates a callback object, and opens a handle for power
    notifications

Arguemnts:

    Callback - A callback object

    CallbackFunction - Function to register for power callbacks

    Context - Context for callback function

Return Value:

    None

--*/
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjAttributes;
    UNICODE_STRING    CallbackName;
            
    RtlInitUnicodeString(&CallbackName, L"\\Callback\\PowerState");
    
    InitializeObjectAttributes(&ObjAttributes,
                               &CallbackName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ExCreateCallback(Callback,
                              &ObjAttributes,
                              FALSE,
                              TRUE);
            
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    AgpV_PowerCallbackHandle = ExRegisterCallback(*Callback,
                                                  CallbackFunction,
                                                  Context);
                
    if (AgpV_PowerCallbackHandle == NULL) {
        ObDereferenceObject(*Callback);
        *Callback = NULL;

        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}


__forceinline
VOID
AgpVerifierFreePowerCallback(
    IN PCALLBACK_OBJECT Callback
    )
/*++

Routine Description:

    Free a callback object

Arguemnts:

    Callback - A callback object

Return Value:

    None

--*/
{
    //
    // Cancel power callback notification
    //
    if (AgpV_PowerCallbackHandle != NULL) {
        ExUnregisterCallback(AgpV_PowerCallbackHandle);
        AgpV_PowerCallbackHandle = NULL;
    }
    
    //
    // Dereference (free) callback object
    //
    if (Callback != NULL) {
        ObDereferenceObject(Callback);
        Callback = NULL;
    }
}


NTSTATUS
AgpVerifierSetTargetCapability(
    IN PVOID AgpExtension,
    IN PPCI_AGP_CAPABILITY Capability
    )
/*++

Routine Description:

    Sets the AGP capability for the AGP target (AGP bridge)

Arguments:

    AgpExtension - Supplies the AGP extension

    Capability - Returns the AGP capability

Return Value:

    NTSTATUS

--*/

{
    NTSTATUS Status;
    PTARGET_EXTENSION Extension;

    GET_TARGET_EXTENSION(Extension, AgpExtension);

    //
    // We need to synchronize with the AGP Verifier when
    // modifying the Target's configuration
    //
    ExAcquireFastMutex(&Extension->AgpV_Ctx.VerifierLock);

    Status = AgpLibSetAgpCapability(ApGetSetDeviceBusData,
                                    Extension,
                                    Capability);
    if (NT_SUCCESS(Status)) {        
        Globals.AgpCommand = *(PULONG)&Capability->AGPCommand;
        Extension->AgpV_Ctx.VerifierCommand = Globals.AgpCommand;
    }

    ExReleaseFastMutex(&Extension->AgpV_Ctx.VerifierLock);

    return Status;
}


VOID
AgpVerifierRegisterPlatformCallbacks(
    IN PVOID AgpExtension,
    IN PAGPV_PLATFORM_INIT   AgpV_PlatformInit,
    IN PAGPV_PLATFORM_WORKER AgpV_PlatformWorker,
    IN PAGPV_PLATFORM_STOP   AgpV_PlatformStop
    )
/*++

Routine Description:

    Called during the platform's AgpInitializeTarget function,
    registers the routines that will enable platform-based
    verification

Arguments:

    AgpExtension - AGP Extension

    AgpV_PlatformInit - The platform's verifier initialization
                        callback function

    AgpV_PlatformWorker - Platform callback function for
                          verification as a periodic work item

    AgpV_PlatformStop - Platform callback to release any
                        resources, and disable verification

Return Value:

    None

--*/
{
    PTARGET_EXTENSION Extension;

    PAGED_CODE();

    GET_TARGET_EXTENSION(Extension, AgpExtension);

    Extension->AgpV_Ctx.PlatformInit = AgpV_PlatformInit;
    Extension->AgpV_Ctx.PlatformWorker = AgpV_PlatformWorker;
    Extension->AgpV_Ctx.PlatformStop = AgpV_PlatformStop;
}



VOID
AgpVerifierDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
/*++

Routine Description:

    A periodic timer DPC that simply schedules a verifier work item

Arguemnts:

    Dpc - Not used

    DeferredContext - Our Target Extension

    SystemArgument1 - Not used

    SystemArgument2 - Not used

Return Value:

    None

--*/
{
    PIO_WORKITEM VerifierWorkItem;
    PTARGET_EXTENSION Extension = DeferredContext;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);
    
    VerifierWorkItem = IoAllocateWorkItem(Extension->Self);
    
    if (VerifierWorkItem != NULL) {
        IoQueueWorkItem(VerifierWorkItem,
                        AgpVerifierWorker,
                        DelayedWorkQueue,
                        VerifierWorkItem);
    
    } else {
        AGPLOG(AGP_WARNING, ("AgpVerifierDpc: failed to allocate "
                             "work item!\n"));
    }
}

#define GART_CONFIG_INTERVAL 9
#define GART_CACHE_INTERVAL 3

static ULONG AgpV_Period = 1;


VOID
AgpVerifierWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
/*++

Routine Description:

    Test the AGP Verifier Page to see if it has been accessed, and
    also read the AGP command register and compare it against a saved    
    copy of its last written value, and if the register contents have
    changed, call KeBugCheckEx with the appropriate AGP error code
    (refer to AGP Library section above), also calls the platform's
    AGPV_PLATFORM_WORKER callback function if one has been registered

Arguemnts:

    DeviceObject - Our AGP Device

    Context - Work Item

Return Value:

    None

--*/
{
    BOOLEAN DoAccess;
    BOOLEAN DoConfig;
    BOOLEAN DoPlatform;
    PTARGET_EXTENSION Extension = DeviceObject->DeviceExtension;
 
    PAGED_CODE();

    //
    // Free this work item
    //
    IoFreeWorkItem(Context);

    ExAcquireFastMutex(&Extension->AgpV_Ctx.VerifierLock);

    if (Extension->AgpV_Ctx.VerifierFlags != 0) {

        AGPLOG(AGP_VERIFY, (".AGP"));

        //
        // Determine verification tasks to be performed
        //
        DoPlatform = (Extension->AgpV_Ctx.PlatformWorker != NULL);
        DoAccess = (Extension->AgpV_Ctx.VerifierFlags &
                    (AGP_GART_ACCESS_VERIFICATION |
                     AGP_GART_GUARD_VERIFICATION)) != 0;
        DoConfig = (Extension->AgpV_Ctx.VerifierFlags &
                    AGP_CONFIGURATION_VERIFICATION) != 0;
        
        //
        // Corruption verification is very expensive, so when it is enabled,
        // we'll do it less frequently, and bypass access/guard verification
        // when we do
        //
        if (Extension->AgpV_Ctx.VerifierFlags &
            AGP_GART_CORRUPTION_VERIFICATION) {
            DoPlatform =
                DoPlatform && ((AgpV_Period % GART_CACHE_INTERVAL) == 0);
            DoAccess = DoAccess && !DoPlatform;
        }

        //
        // Configuration verification doesn't need to happen very often
        //
        DoConfig =
            DoConfig && (((AgpV_Period + 1 ) % GART_CONFIG_INTERVAL) == 0);
        
        if (DoAccess) {
            ULONG Index;
            PULONG AgpVerifierPageOffset;
            
            AgpVerifierPageOffset =
                (PULONG)Extension->AgpV_Ctx.VerifierPage;
            
            //
            // Bugcheck if there's any corruption in our verifier page
            //
            for (Index = 0; Index < (PAGE_SIZE / sizeof(ULONG)); Index++) {
                
                if (*AgpVerifierPageOffset++ != AGP_VERIFIER_SIGNATURE) {
                    
                    AGPLOG(AGP_CRITICAL, ("AgpVerify: Gart corruption "
                                          "detected at %x\n",
                                          AgpVerifierPageOffset - 1));
                    
#if DBG
                    ASSERT(0);
#elif (WINVER > 0x501)
                    KeBugCheckEx(AGP_INVALID_ACCESS,
                                 (ULONG_PTR)(AgpVerifierPageOffset - 1),
                                 0,
                                 0,
                                 0);
#endif
                }
            }
            
            AGPLOG(AGP_VERIFY, ("a")); // Access
        }
        
        //
        // Make sure AGP settings haven't been tampered with
        //
        if (DoConfig) {
            NTSTATUS Status;
            PCI_AGP_CAPABILITY TargetCapability;
            
            Status = AgpLibGetTargetCapability(GET_AGP_CONTEXT(Extension),
                                               &TargetCapability);
            
            if (NT_SUCCESS(Status)) {
                ULONG CommandVal = *(PULONG)&TargetCapability.AGPCommand;
                
                if (CommandVal !=
                    Extension->AgpV_Ctx.VerifierCommand) {
                    
                    AGPLOG(AGP_CRITICAL, ("AgpVerify: AGP command register"
                                          " has been reprogrammed %x => "
                                          "%x\n",
                                          Extension->AgpV_Ctx.VerifierCommand,
                                          CommandVal));
                    
#if DBG
                    ASSERT(0);
#elif (WINVER > 0x501)
                    KeBugCheckEx(AGP_ILLEGALLY_REPROGRAMMED,
                                 Extension->AgpV_Ctx.VerifierCommand,
                                 CommandVal,
                                 0,
                                 0);
#endif
                }
                
            } else {
                AGPLOG(AGP_WARNING, ("AgpVerify: AGP command register "
                                     "cannot be verified (%d)!\n",
                                     Status));
            }
            
            AGPLOG(AGP_VERIFY, ("c")); // Config
        }
        
        //
        // Call the platform verifier's worker function
        //
        if (DoPlatform) {
            (Extension->AgpV_Ctx.PlatformWorker)(GET_AGP_CONTEXT(Extension)); 
        }
   
        AgpV_Period++;
     
        AGPLOG(AGP_VERIFY, ("OK."));
    }

    ExReleaseFastMutex(&Extension->AgpV_Ctx.VerifierLock);    
}


VOID
AgpVerifierInitialize(
    IN PTARGET_EXTENSION Extension
    )
/*++

Routine Description:

    This routine initializes the common AGP library verification
    engine, and also calls the platform-dependent initialization
    callback, AGPV_PLATFORM_INIT, if one has been registered

Arguments:

    Extension - The AGP Extension

Return Value:

    None

--*/
{
    PHYSICAL_ADDRESS HighAddr = { 0xFFFFFFFF, 0xFFFFFFFF };
    PHYSICAL_ADDRESS LowAddr = { 0, 0 };
    PHYSICAL_ADDRESS BoundaryAddr = { 0, 0 };
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;
    
    PAGED_CODE();

    Extension->AgpV_Ctx.VerifierPage =
        MmAllocateContiguousMemorySpecifyCache(PAGE_SIZE,
                                               LowAddr,
                                               HighAddr,
                                               BoundaryAddr,
                                               MmNonCached);

    if (Extension->AgpV_Ctx.VerifierPage == NULL) {

        if (Extension->AgpV_Ctx.VerifierFlags &
            (AGP_GART_ACCESS_VERIFICATION | AGP_GART_GUARD_VERIFICATION)) {

            AGPLOG(AGP_WARNING, ("AgpVerifierInitialize failed to "
                                 "allocate\n\tVerifier Page, Gart "
                                 "Verification disabled\n")); 
            
            Extension->AgpV_Ctx.VerifierFlags &=
                ~(AGP_GART_ACCESS_VERIFICATION | AGP_GART_GUARD_VERIFICATION);
        }

    //
    // Initialize verifier page with our signature
    //
    } else {
        ULONG Index;
        PULONG AgpVerifierPageOffset =
            (PULONG)Extension->AgpV_Ctx.VerifierPage;
        
        for (Index = 0; Index < (PAGE_SIZE / sizeof(ULONG)); Index++) {
            *AgpVerifierPageOffset++ = AGP_VERIFIER_SIGNATURE;
        }
    }

    //
    // Call the platform's verifier init callback
    //
    if (Extension->AgpV_Ctx.PlatformInit) {
        Status = (Extension->AgpV_Ctx.PlatformInit)(
            GET_AGP_CONTEXT(Extension),
            Extension->AgpV_Ctx.VerifierPage,
            &Extension->AgpV_Ctx.VerifierFlags);
    }

    if (!NT_SUCCESS(Status)) {
        
        //
        // We can't do Gart verification without the platform's help...
        //
        if (Extension->AgpV_Ctx.VerifierFlags &
            (AGP_GART_ACCESS_VERIFICATION |
             AGP_GART_GUARD_VERIFICATION |
             AGP_GART_CORRUPTION_VERIFICATION)) {
     
            AGPLOG(AGP_WARNING, ("AgpVerifierInitialize: Gart Verification "
                                 "could not be enabled\n", Status)); 
        }

        Extension->AgpV_Ctx.VerifierFlags &=
            ~(AGP_GART_ACCESS_VERIFICATION |
              AGP_GART_GUARD_VERIFICATION |
              AGP_GART_CORRUPTION_VERIFICATION);

        if (Extension->AgpV_Ctx.VerifierPage != NULL) {
            MmFreeContiguousMemory(Extension->AgpV_Ctx.VerifierPage);
            Extension->AgpV_Ctx.VerifierPage = NULL;
        }

        //
        // We could enable configuration verification without implementing
        // the platform verifier callbacks, but we would have to port all
        // the AGP2 drivers to use the new AGP3 library functions, i.e.,
        // AgpLibSetTargetCapability, and AgpLibWriteAgpTargetConfig, so
        // until then we must disable it in this case
        //
        if (Extension->AgpV_Ctx.VerifierFlags &
            AGP_CONFIGURATION_VERIFICATION) {
            Extension->AgpV_Ctx.VerifierFlags &=
                ~AGP_CONFIGURATION_VERIFICATION;

            AGPLOG(AGP_WARNING, ("AgpVerifierInitialize: Configuration "
                                 "Verification could not be enabled\n",
                                 Status));
        }
    }

    //
    // Cache the AGP command register for configuration verification
    //
    if (Extension->AgpV_Ctx.VerifierFlags & AGP_CONFIGURATION_VERIFICATION) {
        PCI_AGP_CAPABILITY TargetCapability;

        Status = AgpLibGetTargetCapability(GET_AGP_CONTEXT(Extension),
                                           &TargetCapability);

        if (NT_SUCCESS(Status)) {
            Extension->AgpV_Ctx.VerifierCommand =
                *(PULONG)&TargetCapability.AGPCommand;

            //
            // We need to register for a power callback so we can disable
            // configuration verification during power
            //
            Status = AgpVerifierCreatePowerCallback(
                &Extension->AgpV_Ctx.PowerCallbackObj,
                AgpVerifierPowerCallback,
                Extension);

            if (!NT_SUCCESS(Status)) {
                Extension->AgpV_Ctx.VerifierFlags &=
                    ~AGP_CONFIGURATION_VERIFICATION;
                
                AGPLOG(AGP_WARNING, ("AgpVerifierInitialize: Could not create "
                                     "power callback %d, Configuration "
                                     "Verification disabled\n",
                                     Status));

            //
            // Now that we're all poised to do config verification,
            // override the AgpLibSetTargetCapability function with
            // our verifier version
            //
            } else {
                extern PSTCAP_ROUTINE AgpLibSetTargetCapRoutine;

                AgpLibSetTargetCapRoutine = AgpVerifierSetTargetCapability;
            }
            
        } else {
            Extension->AgpV_Ctx.VerifierFlags &=
                ~AGP_CONFIGURATION_VERIFICATION;
            
            AGPLOG(AGP_WARNING, ("AgpVerifierInitialize: configuration "
                                 "data could not be initialized %d, "
                                 "Configuration Verification disabled\n",
                                 Status));
        }
    }

    //
    // Schedule our periodic AGP Verifier DPC
    //
    if (Extension->AgpV_Ctx.VerifierFlags != 0) {
        ULONG VerifierPeriodInMiliseconds = AGP_VERIFIER_PERIOD_IN_MILISECONDS;
        LARGE_INTEGER DueTimeIncrement;
        
        //
        // Add a couple minutes to the initial due time to help speed
        // up boot/init
        //
        DueTimeIncrement.QuadPart = 24 *
            VerifierPeriodInMiliseconds * 10000; // Convert to 100ns units
        
        KeSetTimerEx(&Extension->AgpV_Ctx.VerifierTimer,
                     DueTimeIncrement,
                     VerifierPeriodInMiliseconds,
                     &Extension->AgpV_Ctx.VerifierDpc);

        Extension->AgpV_Ctx.VerifierDpcEnable = TRUE;
        
        AGPLOG(AGP_NOISE, ("\nAGP Verifier enabled, period %ds, "
                           "context %x...\n",
                           (VerifierPeriodInMiliseconds / 1000),
                           &Extension->AgpV_Ctx));
        
        if (Extension->AgpV_Ctx.VerifierFlags & AGP_GART_ACCESS_VERIFICATION) {
            AGPLOG(AGP_NOISE, ("\tAGP_GART_ACCESS_VERIFICATION\n"));
        }

        if (Extension->AgpV_Ctx.VerifierFlags &
            AGP_CONFIGURATION_VERIFICATION) {    
            AGPLOG(AGP_NOISE, ("\tAGP_CONFIGURATION_VERIFICATION\n"));
        }        
        
        AGPLOG(AGP_NOISE, ("\n"));
    }
}

VOID
AgpVerifierUpdateFlags(
    IN PVOID AgpExtension,
    IN ULONG VerifierFlags
    )
/*++

Routine Description:

    Update the verifier options, and take appropriate action

Arguments:

    AgpExtension - AGP Extension

    VerifierFlags - Verifier options to set

Return Value:

    None

--*/
{
    PTARGET_EXTENSION Extension;

    PAGED_CODE();

    GET_TARGET_EXTENSION(Extension, AgpExtension);

    ExAcquireFastMutex(&Extension->AgpV_Ctx.VerifierLock);

    Extension->AgpV_Ctx.VerifierFlags = VerifierFlags;

    if (Extension->AgpV_Ctx.VerifierFlags == 0) {
        
        //
        // Cancel our periodic verifier timer
        //
        if (Extension->AgpV_Ctx.VerifierDpcEnable) {
            KeCancelTimer(&Extension->AgpV_Ctx.VerifierTimer);
            Extension->AgpV_Ctx.VerifierDpcEnable = FALSE;
        }
        
        //
        // Nuke our power callback
        //
        AgpVerifierFreePowerCallback(Extension->AgpV_Ctx.PowerCallbackObj);
    }
    
    ExReleaseFastMutex(&Extension->AgpV_Ctx.VerifierLock);
}

static ULONG AgpV_RestoreFlags = 0;


VOID
AgpVerifierPowerCallback(
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    )
/*++

Routine Description:

    Callback for power state changes, disables configuration verification
    for low power states

Arguments:

    CallbackContext - AGP Target Extension

    Argument1 - 

    Argument2 -

Return Value:

    None

--*/
{
    ULONG_PTR Action = (ULONG_PTR)Argument1;
    ULONG_PTR State = (ULONG_PTR)Argument2;
    PTARGET_EXTENSION Extension = CallbackContext;

    PAGED_CODE();

    if (Action == PO_CB_SYSTEM_STATE_LOCK) {

        ExAcquireFastMutex(&Extension->AgpV_Ctx.VerifierLock);
      
        //
        // Leaving S0
        //
        if (State == 0) {
            AgpV_RestoreFlags = Extension->AgpV_Ctx.VerifierFlags;
            Extension->AgpV_Ctx.VerifierFlags &=
                ~AGP_CONFIGURATION_VERIFICATION;
            
            if (AgpV_RestoreFlags & AGP_CONFIGURATION_VERIFICATION) {
                AGPLOG(AGP_VERIFY, ("AgpVerifierPowerCallback: "
                                    "Leaving S0, Configuration Verification "
                                    "disabled\n"));
            }

        //
        // Just entered S0
        //
        } else if (State == 1) {
            Extension->AgpV_Ctx.VerifierFlags = AgpV_RestoreFlags;

            //
            // Cache the AGP command register for configuration verification
            //
            if (Extension->AgpV_Ctx.VerifierFlags &
                AGP_CONFIGURATION_VERIFICATION) {
                NTSTATUS Status;
                PCI_AGP_CAPABILITY TargetCapability;

                Status = AgpLibGetTargetCapability(GET_AGP_CONTEXT(Extension),
                                                   &TargetCapability);
                
                if (NT_SUCCESS(Status)) {
                    Extension->AgpV_Ctx.VerifierCommand =
                        *(PULONG)&TargetCapability.AGPCommand;
                    
                    AGPLOG(AGP_VERIFY, ("AgpVerifierPowerCallback: Entering "
                                        "S0, Configuration Verification "
                                        "reenabled\n"));

                } else {
                    Extension->AgpV_Ctx.VerifierFlags &=
                        ~AGP_CONFIGURATION_VERIFICATION;
                    
                    //
                    // Nuke our power callback
                    //
                    AgpVerifierFreePowerCallback(
                        Extension->AgpV_Ctx.PowerCallbackObj);
                    
                    AGPLOG(AGP_WARNING, ("AgpVerifierPowerCallback: "
                                         "configuration data could not be "
                                         "initialized %d, Configuration "
                                         "Verification not reenabled\n",
                                         Status));
                }
            }
        }

        ExReleaseFastMutex(&Extension->AgpV_Ctx.VerifierLock);
    }
}


VOID
AgpVerifierStop(
    IN PTARGET_EXTENSION Extension
    )
/*++

Routine Description:

    This routine performs any necessary verifier cleanup, and also
    calls the platform-dependent stop function, AGPV_PLATFORM_STOP,
    if one has been registerd

Arguments:

    Extension - The AGP Extension

Return Value:

    None

--*/
{
    PAGED_CODE();

    ExAcquireFastMutex(&Extension->AgpV_Ctx.VerifierLock);

    //
    // Cancel our periodic verifier timer
    //
    if (Extension->AgpV_Ctx.VerifierDpcEnable) {
        KeCancelTimer(&Extension->AgpV_Ctx.VerifierTimer);
        Extension->AgpV_Ctx.VerifierDpcEnable = FALSE;
    }

    Extension->AgpV_Ctx.VerifierFlags = 0;

    //
    // Nuke our power callback
    //
    AgpVerifierFreePowerCallback(Extension->AgpV_Ctx.PowerCallbackObj);

    //
    // Call the platform's stop routine
    //
    if (Extension->AgpV_Ctx.PlatformStop) {
        (Extension->AgpV_Ctx.PlatformStop)(GET_AGP_CONTEXT(Extension));
    }

    //
    // Free the verifier page
    //
    if (Extension->AgpV_Ctx.VerifierPage != NULL) {
        MmFreeContiguousMemory(Extension->AgpV_Ctx.VerifierPage);
        Extension->AgpV_Ctx.VerifierPage = NULL;
    }

    //
    // Walk/free the allocation list
    //
    ASSERT(IsListEmpty(&Extension->AgpV_Ctx.AllocationList));

    ExReleaseFastMutex(&Extension->AgpV_Ctx.VerifierLock);
}

