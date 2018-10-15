/*++

Copyright (2) 2002 Microsoft Corporation

Module Name:

    gart.c

Abstract:

    This module contains the Graphics Aperture Redirection Table support
    routines for the MS AGP v3 Filter Driver

Author:

    Eric F. Nelson (enelson) June 6, 2002

Revision History:

--*/

#include "agp.h"
#include "uagp35.h"

NTSTATUS
Uagp35CreateGart(
    IN PUAGP35_EXTENSION AgpContext,
    IN ULONG MinimumPages
    );

VOID
Uagp35EnableGTLB(
    IN PUAGP35_EXTENSION AgpContext,
    IN BOOLEAN Enable
    );

NTSTATUS
Uagp35SetRate(
    IN PUAGP35_EXTENSION AgpContext,
    IN ULONG AgpRate
    );

PGART_PTE32
Uagp35FindFreeRangeInGart32(
    IN PVOID StartPte,
    IN PVOID EndPte,
    IN ULONG Length,
    IN BOOLEAN SearchBackward
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AgpDisableAperture)
#pragma alloc_text(PAGE, AgpQueryAperture)
#pragma alloc_text(PAGE, AgpReserveMemory)
#pragma alloc_text(PAGE, AgpReleaseMemory)
#pragma alloc_text(PAGE, Uagp35CreateGart)
#pragma alloc_text(PAGE, AgpMapMemory)
#pragma alloc_text(PAGE, AgpUnMapMemory)
#pragma alloc_text(PAGE, Uagp35FindFreeRangeInGart32)
#pragma alloc_text(PAGE, AgpFindFreeRun)
#pragma alloc_text(PAGE, AgpGetMappedPages)
#endif

#define Uagp35EnableTB(_x_) Uagp35EnableGTLB((_x_), TRUE)
#define Uagp35DisableTB(_x_) Uagp35EnableGTLB((_x_), FALSE)


NTSTATUS
AgpQueryAperture(
    IN PUAGP35_EXTENSION AgpContext,
    OUT PHYSICAL_ADDRESS *CurrentBase,
    OUT ULONG *CurrentSizeInPages,
    OUT OPTIONAL PIO_RESOURCE_LIST *pApertureRequirements
    )
/*++

Routine Description:

    Queries the current size of the GART aperture, and optionally returns
    the possible GART settings

Arguments:

    AgpContext - Supplies our AGP3 context

    CurrentBase - Returns the current physical address of the GART

    CurrentSizeInPages - Returns the current GART size

    ApertureRequirements - If present, returns the possible GART settings

Return Value:

    STATUS_SUCCESS, or an appropriate error status

--*/
{
    PHYSICAL_ADDRESS ApertureBase;
    PIO_RESOURCE_LIST Requirements;
    ULONG Index;
    ULONG Length;
    USHORT ApSize;
    ULONG AltCount;

    PAGED_CODE();

    //
    // Get the current aperture base and size settings
    //
    if (!AgpContext->FourGBEnable) {
        AgpLibReadAgpTargetConfig(AgpContext,
                                  &ApertureBase.LowPart,
                                  APERTURE_BASE,
                                  sizeof(ApertureBase.LowPart));
        
    } else {
        AgpLibReadAgpTargetConfig(AgpContext,
                                  &ApertureBase,
                                  APERTURE_BASE,
                                  sizeof(ApertureBase));
    }

    AgpLibGetExtendedTargetCapability(AgpContext, ApertureSize, &ApSize);

    ASSERT(ApertureBase.QuadPart != 0);

    CurrentBase->QuadPart =
        ApertureBase.QuadPart & PCI_ADDRESS_MEMORY_ADDRESS_MASK;

    //
    // Convert APSIZE into the actual size of the aperture
    //
    switch (ApSize) {
        case AP_SIZE_4MB:
            AltCount = 1;
            *CurrentSizeInPages = 4 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_8MB:
            AltCount = 2;
            *CurrentSizeInPages = 8 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_16MB:
            AltCount = 3;
            *CurrentSizeInPages = 16 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_32MB:
            AltCount = 4;
            *CurrentSizeInPages = 32 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_64MB:
            AltCount = 5;
            *CurrentSizeInPages = 64 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_128MB:
            AltCount = 6;
            *CurrentSizeInPages = 128 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_256MB:
            AltCount = 7;
            *CurrentSizeInPages = 256 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_512MB:
            AltCount = 8;
            *CurrentSizeInPages = 512 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_1024MB:
            AltCount = 9;
            *CurrentSizeInPages = 1024 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_2048MB:
            AltCount = 10;
            *CurrentSizeInPages = 2048 * (1024 * 1024 / PAGE_SIZE);
            break;

//
// IO Resource descriptor's size is only 32-bits!
//
//        case AP_SIZE_4096MB:
//            AltCount = 11;
//            *CurrentSizeInPages = 4096 * (1024 * 1024 / PAGE_SIZE);
//            break;

        default:
            AGPLOG(AGP_CRITICAL,
                   ("AgpQueryAperture - Unexpected value %x for ApSize!\n",
                    ApSize));
            ASSERT(FALSE);
            AgpContext->ApertureStart.QuadPart = 0;
            AgpContext->ApertureLength = 0;
            return STATUS_UNSUCCESSFUL;
    }

    //
    // Remember the current aperture settings
    //
    AgpContext->ApertureStart.QuadPart = CurrentBase->QuadPart;
    AgpContext->ApertureLength = *CurrentSizeInPages * PAGE_SIZE;

    if (pApertureRequirements != NULL) {

        //
        // Since the BIOS probably describes the desired size, we
        // will only attempt alternates of the same size, or less
        //
        Requirements =
            ExAllocatePoolWithTag(PagedPool,
                                  sizeof(IO_RESOURCE_LIST) +
                                  (AltCount - 1) *
                                  sizeof(IO_RESOURCE_DESCRIPTOR),
                                  'RpgA');
        if (Requirements == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        Requirements->Version = Requirements->Revision = 1;
        Requirements->Count = AltCount;
        Length = AgpContext->ApertureLength;

        for (Index = 0; Index < Requirements->Count; Index++) {
            Requirements->Descriptors[Index].Option = IO_RESOURCE_ALTERNATIVE;
            Requirements->Descriptors[Index].Type = CmResourceTypeMemory;
            Requirements->Descriptors[Index].ShareDisposition =
                CmResourceShareDeviceExclusive;
            Requirements->Descriptors[Index].Flags =
                CM_RESOURCE_MEMORY_READ_WRITE |
                CM_RESOURCE_MEMORY_PREFETCHABLE;
            Requirements->Descriptors[Index].u.Memory.Length = Length;
            Requirements->Descriptors[Index].u.Memory.Alignment = Length;
            Requirements->Descriptors[Index].u.Memory.MinimumAddress.QuadPart =
                0;
            Requirements->Descriptors[Index].u.Memory.MaximumAddress.QuadPart =
                AgpContext->FourGBEnable ? (ULONGLONG)-1: (ULONG)-1;
            Length /= 2;
        }
        *pApertureRequirements = Requirements;
    }

    return STATUS_SUCCESS;
}



NTSTATUS
AgpSetAperture(
    IN PUAGP35_EXTENSION AgpContext,
    IN PHYSICAL_ADDRESS NewBase,
    IN ULONG NewSizeInPages
    )
/*++

Routine Description:

    Sets the GART aperture to the supplied settings

Arguments:

    AgpContext - Supplies our AGP3 context

    NewBase - Supplies the new physical memory base for the GART

    NewSizeInPages - Supplies the new size for the GART

Return Value:

    STATUS_SUCCESS, or an appropriate error status

--*/
{
    PCI_AGP_CONTROL AgpCtrl;
    USHORT ApSize;
    PHYSICAL_ADDRESS ApBase;

    //
    // Figure out the new APSIZE setting, make sure it is valid.
    //
    switch (NewSizeInPages) {
        case 4 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_4MB;
            break;
        case 8 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_8MB;
            break;
        case 16 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_16MB;
            break;
        case 32 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_32MB;
            break;
        case 64 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_64MB;
            break;
        case 128 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_128MB;
            break;
        case 256 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_256MB;
            break;
        case 512 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_512MB;
            break;
        case 1024 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_1024MB;
            break;
        case 2048 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_2048MB;
            break;

//
// IO Resource descriptor's size is only 32-bits!
//
//        case AP_SIZE_4096MB:
//            AltCount = 11;
//            *CurrentSizeInPages = 4096 * (1024 * 1024 / PAGE_SIZE);
//            break;
//        case 4096 * (1024 * 1024 / PAGE_SIZE):
//            ApSize = AP_SIZE_4096MB;
//            break;

        default:
            AGPLOG(AGP_CRITICAL,
                   ("AgpSetAperture - invalid GART size of %lx pages "
                    "specified, aperture at %I64X.\n",
                    NewSizeInPages,
                    NewBase.QuadPart));
            ASSERT(FALSE);
            return STATUS_INVALID_PARAMETER;
    }

    //
    // Make sure the supplied size is aligned on the appropriate boundary
    //
    ASSERT((NewBase.QuadPart & ((NewSizeInPages * PAGE_SIZE) - 1)) == 0);
    if ((NewBase.QuadPart & ((NewSizeInPages * PAGE_SIZE) - 1)) != 0 ) {
        AGPLOG(AGP_CRITICAL,
               ("AgpSetAperture - invalid base %I64X specified for GART "
                "aperture of %lx pages\n",
                NewBase.QuadPart,
                NewSizeInPages));
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Reprogram Special Target settings when the chip
    // is powered off, but ignore rate changes as those were already
    // applied during MasterInit
    //
    if (AgpContext->SpecialTarget & ~AGP_FLAG_SPECIAL_RESERVE) {
        AgpSpecialTarget(AgpContext,
                         AgpContext->SpecialTarget &
                         ~AGP_FLAG_SPECIAL_RESERVE);
    }
    
    //
    // Need to reset the hardware to match the supplied settings
    //
    // If the aperture is enabled, disable it, write the new settings,
    // then reenable the aperture
    //
    //
    // Disable the aperture
    //
    AgpLibGetExtendedTargetCapability(AgpContext, AgpControl, &AgpCtrl);

    if (AgpCtrl.AP_Enable == ON) {
        AgpCtrl.AP_Enable = OFF;

        if (!(AgpContext->SpecialTarget &
              AGP_FLAG_SPECIAL_NO_APERTURE_DISABLE)) {
            AgpLibSetExtendedTargetCapability(AgpContext,
                                              AgpControl,
                                              &AgpCtrl);
        } else {
            AGPLOG(AGP_WARNING,
                   ("UAGP35MasterInit: AGP_FLAG_SPECIAL_NO_APERTURE_DISABLE\n")
                   );
        }
    }

    //
    // Write APSIZE first, as this will enable the correct bits in APBASE
    // that need to be written next
    //
    AgpLibSetExtendedTargetCapability(AgpContext, ApertureSize, &ApSize);

    //
    // Now we can update APBASE
    //
    ApBase.QuadPart = NewBase.LowPart & PCI_ADDRESS_MEMORY_ADDRESS_MASK;

    if (AgpContext->CapabilityId != PCI_CAPABILITY_ID_AGP_TARGET) {
        if (!AgpContext->FourGBEnable) {
            ASSERT(ApBase.HighPart == 0);
            
            AgpLibWriteAgpTargetConfig(AgpContext,
                                       &ApBase.LowPart,
                                       APERTURE_BASE,
                                       sizeof(ApBase.LowPart));
            
        } else {
            AgpLibWriteAgpTargetConfig(AgpContext,
                                       &ApBase,
                                       APERTURE_BASE,
                                       sizeof(ApBase));       
        }
    }

#if DBG
    //
    // Read back what we wrote, make sure it worked
    //
    {
        PHYSICAL_ADDRESS DbgBase;
        USHORT DbgSize;
        ULONGLONG ApBaseMask;

        ApBaseMask = AP_BASE_MASK(ApSize);

        AgpLibGetExtendedTargetCapability(AgpContext, ApertureSize, &DbgSize);
        
        if (!AgpContext->FourGBEnable) {
            DbgBase.HighPart = 0;
            AgpLibReadAgpTargetConfig(AgpContext,
                                      &DbgBase.LowPart,
                                      APERTURE_BASE,
                                      sizeof(DbgBase.LowPart));

        } else {
            AgpLibReadAgpTargetConfig(AgpContext,
                                      &DbgBase,
                                      APERTURE_BASE,
                                      sizeof(DbgBase));
        }

        ASSERT(DbgSize == ApSize);

        //
        // If this fires, it means our BAR has moved, might be good to track
        // this during AGP3 debug
        //
        ASSERT((DbgBase.QuadPart & ApBaseMask) == ApBase.QuadPart);
    }
#endif

    //
    // Now enable the aperture if it was enabled before
    //
    if (AgpContext->GlobalEnable) {
        AgpLibGetExtendedTargetCapability(AgpContext,
                                          AgpControl,
                                          &AgpCtrl);
        
        ASSERT((AgpCtrl.AP_Enable == OFF) ||
               (AgpContext->SpecialTarget &
                AGP_FLAG_SPECIAL_NO_APERTURE_DISABLE));
        
        AgpCtrl.AP_Enable = ON;
        AgpLibSetExtendedTargetCapability(AgpContext,
                                          AgpControl,
                                          &AgpCtrl);
    }

    //
    // Update our extension to reflect the new GART setting
    //
    AgpContext->ApertureStart = NewBase;
    AgpContext->ApertureLength = NewSizeInPages * PAGE_SIZE;

    //
    // Enable the TB in case we are resuming from S3 or S4
    //
    Uagp35EnableTB(AgpContext);

    //
    // If the GART has been allocated, rewrite the ATTBASE
    //
    if (AgpContext->Gart != NULL) {
        AgpLibSetExtendedTargetCapability(AgpContext,
                                          GartLow,
                                          &AgpContext->GartPhysical.LowPart);

#ifdef _WIN64
        AgpLibSetExtendedTargetCapability(AgpContext,
                                          GartHigh,
                                          &AgpContext->GartPhysical.HighPart);
#endif
    }

    return STATUS_SUCCESS;
}



VOID
AgpDisableAperture(
    IN PUAGP35_EXTENSION AgpContext
    )
/*++

Routine Description:

    Disables the GART aperture so that this resource is available
    for other devices

Arguments:

    AgpContext - Supplies our AGP3 context

Return Value:

    None

--*/

{
    PCI_AGP_CONTROL AgpCtrl;

    //
    // Disable the aperture
    //
    AgpLibGetExtendedTargetCapability(AgpContext,
                                      AgpControl,
                                      &AgpCtrl);

    if (AgpCtrl.AP_Enable == ON) {
        AgpCtrl.AP_Enable = OFF;
        
        if (!(AgpContext->SpecialTarget &
              AGP_FLAG_SPECIAL_NO_APERTURE_DISABLE)) {
            
            AgpLibSetExtendedTargetCapability(AgpContext,
                                              AgpControl,
                                              &AgpCtrl);
            
        } else {
            AGPLOG(AGP_WARNING,
                   ("UAGP35DisableAperture: "
                    "AGP_FLAG_SPECIAL_NO_APERTURE_DISABLE\n")
                   );
        }
    }
    
    AgpContext->GlobalEnable = FALSE;

    //
    // Nuke the Gart!  (It's meaningless now...)
    //
    if (AgpContext->Gart != NULL) {
        MmFreeContiguousMemory(AgpContext->Gart);
        AgpContext->Gart = NULL;
        AgpContext->GartLength = 0;
    }
}



NTSTATUS
AgpReserveMemory(
    IN PUAGP35_EXTENSION AgpContext,
    IN OUT AGP_RANGE *Range
    )
/*++

Routine Description:

    Reserves a range of memory in the GART

Arguments:

    AgpContext - Supplies our AGP3 Context

    Range - Supplies the AGP_RANGE structure (AGPLIB will have filled
            in NumberOfPages and Type, and this routine will fill in
            MemoryBase and Context)

Return Value:

    STATUS_SUCCESS, or an appropriate error status

--*/
{
    ULONG Index;
    ULONG NewState;
    NTSTATUS Status;
    PGART_PTE32 FoundRange;
    BOOLEAN Backwards;
    GART_PTE32 NewPte;

    PAGED_CODE();

    ASSERT((Range->Type == MmNonCached) || (Range->Type == MmWriteCombined));

    if (Range->NumberOfPages > (AgpContext->ApertureLength / PAGE_SIZE)) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // If we have not allocated our GART yet, now is the time to do so
    //
    if (AgpContext->Gart == NULL) {
        ASSERT(AgpContext->GartLength == 0);

        Status = Uagp35CreateGart(AgpContext, Range->NumberOfPages);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("UAGP35CreateGart failed %08lx to create GART of size "
                    "%lx\n",
                    Status,
                    AgpContext->ApertureLength));
            return Status;
        }
    }
    ASSERT(AgpContext->GartLength != 0);

    //
    // Now that we have a GART, try and find enough contiguous entries to
    // satisfy the request, requests for uncached memory will scan from high
    // addresses to low addresses, requests for write-combined memory will
    // scan from low addresses to high addresses, we will use a first-fit
    // algorithm to try and keep the allocations
    // packed and contiguous
    //
    Backwards = (Range->Type == MmNonCached) ? TRUE : FALSE;
    FoundRange =
        Uagp35FindFreeRangeInGart32(&AgpContext->Gart[0],
                                    &AgpContext->Gart[(AgpContext->GartLength /
                                                       sizeof(GART_PTE32)) -1 ],
                                    Range->NumberOfPages,
                                    Backwards);
    if (FoundRange == NULL) {

        //
        // A big enough chunk was not found.
        //
        AGPLOG(AGP_CRITICAL,
               ("AgpReserveMemory: Could not find %d contiguous free pages "
                "of type %d in GART at %08lx\n",
                Range->NumberOfPages,
                Range->Type,
                AgpContext->Gart));

        //
        //  This is where we could try and grow the GART
        //
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    AGPLOG(AGP_NOISE,
           ("AgpReserveMemory: reserved %d pages at GART PTE %08lx\n",
            Range->NumberOfPages,
            FoundRange));

    //
    // Set these pages to reserved
    //
    if (Range->Type == MmNonCached) {
        NewState = GART_ENTRY_RESERVED_UC;

    } else if (Range->Type == MmHardwareCoherentCached) {
        if (AgpContext->CoherentEnable) {
            NewState = GART_ENTRY_RESERVED_CC;

        } else {
            AGPLOG(AGP_NOISE,
                   ("AgpReserveMemory: Coherent memory not supported.\n"));   

            return STATUS_NOT_SUPPORTED;
        }

    } else if (Range->Type == MmWriteCombined) {
        NewState = GART_ENTRY_RESERVED_WC;

    } else {
        AGPLOG(AGP_WARNING,
               ("AgpReserveMemory: Unknown cache type %d\n",
                Range->Type));

        return STATUS_INVALID_PARAMETER;
    }

    NewPte.AsULONG = AgpV_PTE32.AsULONG;
    NewPte.Soft.State = NewState;
 
    for (Index = 0; Index < Range->NumberOfPages; Index++) {
        ASSERT(GartEntryFree(&FoundRange[Index]));
        FoundRange[Index] = NewPte;
    }

    //
    // Update the verifier Gart-cache PTEs in this case
    //
    if (AgpV_Flags & AGP_GART_CORRUPTION_VERIFICATION) {
        
        for (Index = 0; Index < Range->NumberOfPages; Index++) {
            FoundRange[Index + AgpV_GartCachePteOffset] = FoundRange[Index];
        }
    }

    Range->MemoryBase.QuadPart = AgpContext->ApertureStart.QuadPart +
        (FoundRange - &AgpContext->Gart[0]) * PAGE_SIZE;
    Range->Context = FoundRange;

    ASSERT(Range->MemoryBase.HighPart == 0);
    AGPLOG(AGP_NOISE,
           ("AgpReserveMemory: reserved memory handle %lx at PA %08lx\n",
            FoundRange,
            Range->MemoryBase.LowPart));

    return STATUS_SUCCESS;
}



NTSTATUS
AgpReleaseMemory(
    IN PUAGP35_EXTENSION AgpContext,
    IN PAGP_RANGE Range
    )
/*++

Routine Description:

    Releases memory previously reserved with AgpReserveMemory

Arguments:

    AgpContext - Supplies our AGP3 context

    AgpRange - Supplies the range to be released

Return Value:

    STATUS_SUCCESS, or an appropriate error status    

--*/
{
    PGART_PTE32 Pte;
    PGART_PTE32 AgpV_Pte;
    GART_PTE32 NewPte;

    PAGED_CODE();

    NewPte.AsULONG = AgpV_PTE32.AsULONG;
    NewPte.Soft.State = GART_ENTRY_FREE;

    //
    // Go through and free all the PTEs, none of these should still
    // be valid at this point
    //

    for (Pte = Range->Context;
         Pte < ((PGART_PTE32)Range->Context + Range->NumberOfPages);
         Pte++) {
        
        ASSERT(((Range->Type == MmNonCached) &&
                (GartEntryReservedAs(Pte, GART_ENTRY_RESERVED_UC))) ||
               ((Range->Type == MmWriteCombined) &&
                (GartEntryReservedAs(Pte, GART_ENTRY_RESERVED_WC))) ||
               ((Range->Type == MmHardwareCoherentCached) &&
                (GartEntryReservedAs(Pte, GART_ENTRY_RESERVED_CC))));
        
        *Pte = NewPte;
    }

    //
    // Update the verifier Gart-cache PTEs in this case
    //    
    if (AgpV_Flags & AGP_GART_CORRUPTION_VERIFICATION) {

        for (Pte = Range->Context;
             Pte < ((PGART_PTE32)Range->Context + Range->NumberOfPages);
             Pte++) {

            AgpV_Pte = Pte + AgpV_GartCachePteOffset;
            *AgpV_Pte = *Pte;
        }
    }

    Range->MemoryBase.QuadPart = 0;

    return STATUS_SUCCESS;
}


NTSTATUS
Uagp35CreateGart(
    IN PUAGP35_EXTENSION AgpContext,
    IN ULONG MinimumPages
    )
/*++

Routine Description:

    Allocates and initializes an empty GART, the current implementation
    attempts to allocate the entire GART on the first reserve

Arguments:

    AgpContext - Supplies our AGP3 context

    MinimumPages - Supplies the minimum size (in pages) of the GART to be
                   created

Return Value:

    STATUS_SUCCESS, or an appropriate error status

--*/
{
    PGART_PTE32 Gart;
    ULONG GartLength;
    PHYSICAL_ADDRESS HighestAcceptable;
    PHYSICAL_ADDRESS LowestAcceptable;
    PHYSICAL_ADDRESS BoundaryMultiple;
    PHYSICAL_ADDRESS GartPhysical;
    ULONG Index;

    PAGED_CODE();

    //
    // Try and get a chunk of contiguous memory big enough to map the
    // entire aperture
    //
    LowestAcceptable.QuadPart = 0;
    BoundaryMultiple.QuadPart = 0;

#ifdef _WIN64
    HighestAcceptable.QuadPart = 0xFFFFFFFFFF; // 40-bit (32-bit PTE format)
#else
    HighestAcceptable.QuadPart = (ULONG)-1;
#endif

    GartLength =
        BYTES_TO_PAGES(AgpContext->ApertureLength) * sizeof(GART_PTE32);

    //
    // Attempt to allocate a Gart-cache to guard against corruption
    //
    Gart = NULL;
    if (AgpV_Flags & AGP_GART_CORRUPTION_VERIFICATION) {
        Gart = MmAllocateContiguousMemorySpecifyCache(GartLength * 2,
                                                      LowestAcceptable,
                                                      HighestAcceptable,
                                                      BoundaryMultiple,
                                                      MmNonCached);
        if (Gart == NULL) {
            AgpV_Flags |= ~AGP_GART_CORRUPTION_VERIFICATION;

            AGPLOG(AGP_WARNING,
                   ("UAGP35CreateGart: Couldn't allocate Gart-cache, "
                    "AGP_GART_CORRUPTION_VERIFICATION disabled\n"));

            //
            // Notify AGP lib we couldn't set this verifier option 
            //
            AgpVerifierUpdateFlags(AgpContext, AgpV_Flags);

        } else {
            AgpV_GartCachePteOffset = GartLength / sizeof(GART_PTE32);
        }
    }

    //
    // Allocate the standard size Gart, no AGP_GART_CORRUPTION_VERIFICATION
    //
    if (Gart == NULL) {
        Gart = MmAllocateContiguousMemorySpecifyCache(GartLength,
                                                      LowestAcceptable,
                                                      HighestAcceptable,
                                                      BoundaryMultiple,
                                                      MmNonCached);
    }

    if (Gart == NULL) {
        AGPLOG(AGP_CRITICAL,
               ("UAGP35CreateGart: MmAllocateContiguousMemorySpecifyCache %lx "
                "failed\n",
                GartLength));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // We successfully allocated a contiguous chunk of memory.
    // It should be page aligned already.
    //
    ASSERT(((ULONG_PTR)Gart & (PAGE_SIZE - 1)) == 0);

    //
    // Get the physical address
    //
    GartPhysical = MmGetPhysicalAddress(Gart);
    AGPLOG(AGP_NOISE,
           ("UAGP35CreateGart: GART of length %lx created at VA %08lx, "
            "PA %08lx\n",
            GartLength,
            Gart,
            GartPhysical.LowPart));
    ASSERT(GartPhysical.HighPart == 0);
    ASSERT((GartPhysical.LowPart & (PAGE_SIZE - 1)) == 0);

    //
    // Initialize all the PTEs to free
    //
    for (Index = 0; Index < (GartLength / sizeof(GART_PTE32)); Index++) {
        Gart[Index].AsULONG = AgpV_PTE32.AsULONG;
        Gart[Index].Soft.State = GART_ENTRY_FREE;
    }
    
    //
    // Setup verifier guard regions as follows:
    //
    // +-+-+-+-+-+-+-+-+
    // |X|x| Guard |x|X|
    // +---------------+
    // |O|O|o|o|o|o|O|O|
    // +-+-+ Free -+-+-+
    // |O|O|o|o|o|o|O|O|
    // +-+-+-+-+-+-+-+-+
    // |X|x| Guard |x|X|
    // +-+-+-+-+-+-+-+-+
    //
    // This method will ensure that alignment requirements
    // of video devices are satisfied
    //
    if (AgpV_Flags & AGP_GART_GUARD_VERIFICATION) {
        ULONG Offset;
        ULONG Ap4thPages;

        Ap4thPages = BYTES_TO_PAGES(AgpContext->ApertureLength / 4);

        for (Offset = 0; Offset < Ap4thPages; Offset++) {
            Gart[Offset].Soft.State = GART_ENTRY_GUARD;
            Gart[Index - (1 + Offset)].Soft.State = GART_ENTRY_GUARD;
        }
    }

    //
    // Initialize the verifier Gart-cache
    //    
    if (AgpV_Flags & AGP_GART_CORRUPTION_VERIFICATION) {

        AGP_ASSERT(AgpV_GartCachePteOffset);

        for (Index = 0; Index < AgpV_GartCachePteOffset; Index++) {
            Gart[Index + AgpV_GartCachePteOffset] = Gart[Index];
        }
    }

    if (AgpV_Flags &
        (AGP_GART_CORRUPTION_VERIFICATION | AGP_GART_GUARD_VERIFICATION)) {
     
        AGPLOG(AGP_NOISE, ("\nAGP Verifier additional platform enables...\n"));
    
        if (AgpV_Flags & AGP_GART_GUARD_VERIFICATION) {
            AGPLOG(AGP_NOISE, ("\tAGP_GART_GUARD_VERIFICATION\n"));
        }
    
        if (AgpV_Flags & AGP_GART_CORRUPTION_VERIFICATION) {    
            AGPLOG(AGP_NOISE, ("\tAGP_CORRUPTION_VERIFICATION\n"));
        }  
      
        AGPLOG(AGP_NOISE, ("\n"));
    }

    AgpLibSetExtendedTargetCapability(AgpContext,
                                      GartLow,
                                      &GartPhysical.LowPart);

#ifdef _WIN64
    AgpLibSetExtendedTargetCapability(AgpContext,
                                      GartHigh,
                                      &GartPhysical.HighPart);
#endif

    //
    // Update our extension to reflect the current state.
    //
    AgpContext->Gart = Gart;
    AgpContext->GartLength = GartLength;
    AgpContext->GartPhysical = GartPhysical;

    return STATUS_SUCCESS;
}



NTSTATUS
AgpMapMemory(
    IN PUAGP35_EXTENSION AgpContext,
    IN PAGP_RANGE Range,
    IN PMDL Mdl,
    IN ULONG OffsetInPages,
    OUT PHYSICAL_ADDRESS *MemoryBase
    )
/*++

Routine Description:

    Maps physical memory into the GART somewhere in the specified range

Arguments:

    AgpContext - Supplies our AGP3 context

    Range - Supplies the AGP range that the memory should be mapped into

    Mdl - Supplies the MDL describing the physical pages to be mapped

    OffsetInPages - Supplies the offset into the reserved range where the 
                    mapping should begin

    MemoryBase - Returns the physical memory in the aperture where the pages
                 were mapped

Return Value:

    STATUS_SUCCESS, or an appropriate error status    

--*/
{
    ULONG PageCount;
    PGART_PTE32 Pte;
    PGART_PTE32 StartPte;
    ULONG Index;
    ULONG TargetState;
    PPFN_NUMBER Page;
    BOOLEAN Backwards;
    GART_PTE32 NewPte;
    PCI_AGP_CONTROL AgpCtrl;

    PAGED_CODE();

    ASSERT(Mdl->Next == NULL);

    NewPte.AsULONG = 0;
    StartPte = Range->Context;
    PageCount = BYTES_TO_PAGES(Mdl->ByteCount);
    ASSERT(PageCount <= Range->NumberOfPages);
    ASSERT(OffsetInPages <= Range->NumberOfPages);
    ASSERT(PageCount + OffsetInPages <= Range->NumberOfPages);
    ASSERT(PageCount > 0);

    switch (Range->Type) {
        case MmNonCached:
            TargetState = GART_ENTRY_RESERVED_UC;
            NewPte.Soft.State = GART_ENTRY_VALID_UC;
            break;
            
        case MmWriteCombined:
            TargetState = GART_ENTRY_RESERVED_WC;
            NewPte.Soft.State = GART_ENTRY_VALID_WC;
            break;
            
        case MmHardwareCoherentCached:
            TargetState = GART_ENTRY_RESERVED_CC;
            NewPte.Soft.State = GART_ENTRY_VALID_CC;
            break;

            //
            // Something went wrong! (This range wasn't reserved with
            // this cache type)
            //
        default:
            AGPLOG(AGP_WARNING,
                   ("AgpMapMemory: Invalid cache type %d\n",
                    Range->Type));
            return STATUS_INVALID_PARAMETER;
    }

    Pte = StartPte + OffsetInPages;

    //
    // We have a suitable range, now fill it in with the supplied MDL
    //
    ASSERT(Pte >= StartPte);
    ASSERT((Pte + PageCount) <= (StartPte + Range->NumberOfPages));
    Page = (PPFN_NUMBER)(Mdl + 1);

    //
    // Disable the TB as per the (440?) spec, this is probably unnecessary
    // as there should be no valid entries in this range, and there should
    // be no invalid entries still in the TB, so flushing the TB seems
    // a little gratuitous but that's what the old spec said to do
    //
    Uagp35DisableTB(AgpContext);

    for (Index = 0; Index < PageCount; Index++) {
        ASSERT(GartEntryReservedAs(&Pte[Index], TargetState));
        
#ifndef _WIN64
        NewPte.Hard.PageLow = *Page++;
#else
        NewPte.Hard.PageLow = (ULONG)(*Page & PAGE_LOW_MASK);
        NewPte.Hard.PageHigh =
            (ULONG)((*Page++ >> PAGE_HIGH_SHIFT) & PAGE_HIGH_MASK);
#endif
        Pte[Index] = NewPte;
        ASSERT(Pte[Index].Hard.Valid == ON);
    }

    //
    // Update the verifier Gart-cache PTEs in this case
    //    
    if (AgpV_Flags & AGP_GART_CORRUPTION_VERIFICATION) {

        for (Index = 0; Index < PageCount; Index++) {
            Pte[Index + AgpV_GartCachePteOffset] = Pte[Index];
        }
    }

    //
    // We have filled in all the PTEs, read back the last one we wrote
    // in order to flush the write buffers
    //
    NewPte.AsULONG = *(volatile ULONG *)&Pte[PageCount - 1].AsULONG;

    //
    // Re-enable the TB
    //
    Uagp35EnableTB(AgpContext);

    //
    // If we have not yet gotten around to enabling the GART aperture,
    // do it now
    //
    if (!AgpContext->GlobalEnable) {
        AGPLOG(AGP_NOISE, ("AgpMapMemory: Enabling global aperture access\n"));

        AgpLibGetExtendedTargetCapability(AgpContext, AgpControl, &AgpCtrl);

        AgpCtrl.AP_Enable = ON;
        
        AgpLibSetExtendedTargetCapability(AgpContext,
                                          AgpControl,
                                          &AgpCtrl);
        AgpContext->GlobalEnable = TRUE;
    }

    MemoryBase->QuadPart = Range->MemoryBase.QuadPart +
        ((Pte - StartPte) * PAGE_SIZE);

    return STATUS_SUCCESS;
}



NTSTATUS
AgpUnMapMemory(
    IN PUAGP35_EXTENSION AgpContext,
    IN PAGP_RANGE AgpRange,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages
    )
/*++

Routine Description:

    Unmaps previously mapped memory in the GART

Arguments:

    AgpContext - Supplies our AGP3 context

    AgpRange - Supplies the AGP range that the memory should be freed from

    NumberOfPages - Supplies the number of pages in the range to be freed

    OffsetInPages - Supplies the offset into the range where the freeing
                    should begin

Return Value:

    STATUS_SUCCESS, or an appropriate error status

--*/
{
    ULONG Index;
    PGART_PTE32 Pte;
    PGART_PTE32 LastChanged = NULL;
    PGART_PTE32 StartPte;
    ULONG NewState;
    GART_PTE32 NewPte;

    PAGED_CODE();

    ASSERT(OffsetInPages + NumberOfPages <= AgpRange->NumberOfPages);

    StartPte = AgpRange->Context;
    Pte = &StartPte[OffsetInPages];

    if (AgpRange->Type == MmNonCached) {
        NewState = GART_ENTRY_RESERVED_UC;
    } else if (AgpRange->Type == MmWriteCombined) {
        NewState = GART_ENTRY_RESERVED_WC;
    } else if (AgpRange->Type == MmHardwareCoherentCached) {
        NewState = GART_ENTRY_RESERVED_CC;
    } else {

        //
        // Something went wrong! (This range wasn't reserved with
        // this cache type)
        //
        AGPLOG(AGP_WARNING,
               ("AgpUnMapMemory: Invalid cache type %d\n",
                AgpRange->Type));
        
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Disable the TB to flush it
    //
    Uagp35DisableTB(AgpContext);

    NewPte.AsULONG = AgpV_PTE32.AsULONG;
    NewPte.Soft.State = NewState;

    //
    // Update the verifier Gart-cache PTEs first, in this case
    //
    if (AgpV_Flags & AGP_GART_CORRUPTION_VERIFICATION) {

        for (Index = 0; Index < NumberOfPages; Index++) {

            if (GartEntryMapped(&Pte[Index])) {
                Pte[Index + AgpV_GartCachePteOffset] = NewPte; 
            }
        }
    }

    for (Index = 0; Index < NumberOfPages; Index++) {
        
        if (GartEntryMapped(&Pte[Index])) {
            Pte[Index] = NewPte;
            LastChanged = &Pte[Index];
            
        } else {
            
            //
            // This page is not mapped, just skip it
            //
            AGPLOG(AGP_NOISE,
                   ("AgpUnMapMemory: PTE %08lx (%08lx) at offset %d not "
                    "mapped\n",
                    &Pte[Index],
                    Pte[Index].AsULONG,
                    Index));
            ASSERT(Pte[Index].Soft.State == NewState);
        }
    }

    //
    // We have invalidated all the PTEs, read back the last one we wrote
    // in order to flush the write buffers
    //
    if (LastChanged != NULL) {
        ULONG FlushWB;

        FlushWB = *(volatile ULONG *)&LastChanged->AsULONG;
    }

    //
    // Reenable the TB
    //
    Uagp35EnableTB(AgpContext);

    return STATUS_SUCCESS;
}



PGART_PTE32
Uagp35FindFreeRangeInGart32(
    IN PGART_PTE32 StartPte,
    IN PGART_PTE32 EndPte,
    IN ULONG Length,
    IN BOOLEAN SearchBackward
    )
/*++

Routine Description:

    Finds a contiguous free range in a 32-bit PTE format GART, this
    routine can search either from the beginning of the GART
    forwards or the end of the GART backwards

Arguments:

    StartIndex - Supplies the first GART PTE to search

    EndPte - Supplies the last GART PTE to search (inclusive)

    Length - Supplies the number of contiguous free entries
             to search for

    SearchBackward - TRUE indicates that the search should begin
                     at EndPte and search backwards, FALSE indicates
                     that the search should begin at StartPte and
                     search forwards

Return Value:

    Pointer to the first PTE in the GART if a suitable range
    is found, and NULL if no suitable range exists

--*/
{
    PGART_PTE32 Current;
    PGART_PTE32 Last;
    LONG Delta;
    ULONG Found;
    PGART_PTE32 Candidate;

    PAGED_CODE();

    ASSERT(EndPte >= StartPte);
    ASSERT(Length <= (ULONG)(EndPte - StartPte + 1));
    ASSERT(Length != 0);

    if (SearchBackward) {
        Current = EndPte;
        Last = StartPte - 1;
        Delta = -1;
    
    } else {
        Current = StartPte;
        Last = EndPte + 1;
        Delta = 1;
    }

    Found = 0;
    while (Current != Last) {

        if (GartEntryFree(Current)) {

            if (++Found == Length) {

                //
                // A suitable range was found, return it
                //
                if (SearchBackward) {
                    return Current;

                } else {
                    return(Current - Length + 1);
                }
            }

        } else {
            Found = 0;
        }
        Current += Delta;
    }

    //
    // A suitable range was not found
    //
    return NULL;
}



VOID
Uagp35EnableGTLB(
    IN PUAGP35_EXTENSION AgpContext,
    IN BOOLEAN Enable
    )
/*++

Routine Description:

    Enables or disables the GTLB by setting or clearing the GTLB_Enable bit
    in the AGPCTRL register

Arguments:

    AgpContext - Supplies our AGP3 context

    Enable - TRUE, GTLB_Enable is set to ON
             FALSE, GTLB_Enable is set to OFF

Return Value:

    None

--*/
{
    PCI_AGP_CONTROL AgpCtrl;

    //
    // VIA errata, don't enable TLB
    //
    if ((AgpContext->SpecialTarget & AGP_FLAG_SPECIAL_VIA_DISABLE_GTLB) ==
        AGP_FLAG_SPECIAL_VIA_DISABLE_GTLB) {
        return;
    }

    AgpLibGetExtendedTargetCapability(AgpContext, AgpControl, &AgpCtrl);

    AgpCtrl.GTLB_Enable = (Enable) ? ON: OFF;
    
    AgpLibSetExtendedTargetCapability(AgpContext, AgpControl, &AgpCtrl);
}



VOID
AgpFindFreeRun(
    IN PVOID AgpContext,
    IN PAGP_RANGE AgpRange,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    OUT ULONG *FreePages,
    OUT ULONG *FreeOffset
    )
/*++

Routine Description:

    Finds the first contiguous run of free pages in the specified
    part of the reserved range

Arguments:

    AgpContext - Supplies our AGP3 context

    AgpRange - Supplies the AGP range

    NumberOfPages - Supplies the size of the region to be searched for free
                    pages

    OffsetInPages - Supplies the start of the region to be searched for free
                    pages

    FreePages - Returns the length of the first contiguous run of free pages

    FreeOffset - Returns the start of the first contiguous run of free pages

Return Value:

    None

--*/
{
    PGART_PTE32 Pte;
    ULONG Index;
    
    Pte = (PGART_PTE32)(AgpRange->Context) + OffsetInPages;

    //
    // Find the first free PTE
    //
    for (Index = 0; Index < NumberOfPages; Index++) {

        if (GartEntryReserved(&Pte[Index])) {

            //
            // Found a free PTE, count the contiguous ones.
            //
            *FreeOffset = Index + OffsetInPages;
            *FreePages = 0;
            while ((Index < NumberOfPages) && GartEntryReserved(&Pte[Index])) {
                *FreePages += 1;
                ++Index;
            }
            return;
        }
    }

    //
    // No free PTEs in the specified range
    //
    *FreePages = 0;
}



VOID
AgpGetMappedPages(
    IN PVOID AgpContext,
    IN PAGP_RANGE AgpRange,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    OUT PMDL Mdl
    )
/*++

Routine Description:

    Returns the list of physical pages mapped into the specified 
    range in the GART

Arguments:

    AgpContext - Supplies our AGP3 context

    AgpRange - Supplies the AGP range

    NumberOfPages - Supplies the number of pages to be returned

    OffsetInPages - Supplies the start of the region 

    Mdl - Returns the list of physical pages mapped in the specified range

Return Value:

    None

--*/
{
    PGART_PTE32 Pte;
    ULONG Index;
    PPFN_NUMBER Pages;
    
    ASSERT((NumberOfPages * PAGE_SIZE) == Mdl->ByteCount);

    Pages = (PPFN_NUMBER)(Mdl + 1);
    Pte = (PGART_PTE32)(AgpRange->Context) + OffsetInPages;

    for (Index = 0; Index < NumberOfPages; Index++) {
        ASSERT(GartEntryMapped(&Pte[Index]));
        Pages[Index] = Pte[Index].Hard.PageLow;
#ifdef _WIN64
        Pages[Index] |= ((ULONG)Pte[Index].Hard.PageHigh << PAGE_HIGH_SHIFT);
#endif
    }
}



NTSTATUS
AgpSpecialTarget(
    IN PUAGP35_EXTENSION AgpContext,
    IN ULONGLONG DeviceFlags
    )
/*++

Routine Description:

    This routine makes "special" tweaks to the AGP chipset

Arguments:

    AgpContext - Supplies the AGP context
 
    DeviceFlags - Flags indicating what tweaks to perform

Return Value:

    STATUS_SUCCESS, or error

--*/
{
    NTSTATUS Status;

    Status = STATUS_SUCCESS;

    //
    // Should we change the AGP rate?
    //
    if (DeviceFlags & AGP_FLAG_SPECIAL_RESERVE) {
        Status = Uagp35SetRate(AgpContext,
                               (ULONG)((DeviceFlags & AGP_FLAG_SPECIAL_RESERVE)
                                       >> AGP_FLAG_SET_RATE_SHIFT));
        
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    //
    // Add more tweaks here...
    //

    AgpContext->SpecialTarget |= DeviceFlags;

    return Status;
}

#define MIN(A, B) (((A) < (B)) ? (A): (B))


NTSTATUS
Uagp35SetRate(
    IN PUAGP35_EXTENSION AgpContext,
    IN ULONG AgpRate
    )
/*++

Routine Description:

    This routine sets the AGP rate

Arguments:

    AgpContext - Supplies our AGP3 context
 
    AgpRate - Rate to set

Return Value:

    STATUS_SUCCESS, or error status

--*/
{
    NTSTATUS Status;
    ULONG TargetEnable;
    ULONG MasterEnable;
    PCI_AGP_CAPABILITY TargetCap;
    PCI_AGP_CAPABILITY MasterCap;
    BOOLEAN ReverseInit;
    BOOLEAN DisableAgp;
    ULONG SBAEnable;
    ULONG FastWrite;

    //
    // Read capabilities
    //
    Status = AgpLibGetTargetCapability(AgpContext, &TargetCap);

    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING, ("UAGP35SetRate: AgpLibGetTargetCapability "
                             "failed %08lx\n", Status));
        return Status;
    }

    Status = AgpLibGetMasterCapability(AgpContext, &MasterCap);

    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING, ("UAGP35SetRate: AgpLibGetMasterCapability "
                             "failed %08lx\n", Status));
        return Status;
    }

    DisableAgp = (AgpRate == 0);

    if (!DisableAgp) {

        //
        // Map AGP3 mode rates (4X/8X) into AGP2 rate bits, checking one should
        // be good enough, so we'll just ASSERT for busted video cards
        //
        if (TargetCap.AGPStatus.Agp3Mode == ON) {
            ASSERT(MasterCap.AGPStatus.Agp3Mode == ON);
            
            if ((AgpRate != PCI_AGP_RATE_4X) && (AgpRate != 8)) {
                return STATUS_INVALID_PARAMETER;
            }
            
            AgpRate >>= 2;
        }
        
        //
        // Verify the requested rate is supported by both master and target
        //
        if (!(AgpRate & TargetCap.AGPStatus.Rate & MasterCap.AGPStatus.Rate)) {
            return STATUS_INVALID_PARAMETER;
        }

    } else {

        //
        // Before we attempt to disable AGP, make sure there are no
        // valid entries in the gart, then free it
        //
        if (AgpContext->Gart != NULL) {
            ULONG Index;

            for (Index = 0;
                 Index < (AgpContext->GartLength / sizeof(GART_PTE32));
                 Index++) {

                if (AgpContext->Gart[Index].Soft.State != GART_ENTRY_FREE) {

                    AGPLOG(AGP_CRITICAL,
                           ("UAGP35AgpSetRate: Cannot disable AGP with "
                            "allocations outstanding\n"));

                    return STATUS_INVALID_DEVICE_REQUEST;
                }
            }
        }

        //
        // Also disable the aperture
        //
        AgpDisableAperture(AgpContext);
    }

    //
    // Disable AGP while the pull the rug out from underneath
    //
    TargetEnable = TargetCap.AGPCommand.AGPEnable;
    TargetCap.AGPCommand.AGPEnable = 0;

    Status = AgpLibSetTargetCapability(AgpContext, &TargetCap);
    
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING,
               ("UAGP35SetRate: AgpLibSetTargetCapability %08lx for "
                "Target failed %08lx\n",
                &TargetCap,
                Status));
        return Status;
    }
    
    MasterEnable = MasterCap.AGPCommand.AGPEnable;
    MasterCap.AGPCommand.AGPEnable = 0;

    Status = AgpLibSetMasterCapability(AgpContext, &MasterCap);
    
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING,
               ("UAGP35SetRate: AgpLibSetMasterCapability %08lx failed "
                "%08lx\n",
                &MasterCap,
                Status));
        return Status;
    }

    if (DisableAgp) {
        return Status;
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

    MasterCap.AGPCommand.Rate = AgpRate;
    TargetCap.AGPCommand.Rate = AgpRate;
    MasterCap.AGPCommand.AGPEnable = MasterEnable;
    TargetCap.AGPCommand.AGPEnable = TargetEnable;
    MasterCap.AGPCommand.SBAEnable = SBAEnable;
    TargetCap.AGPCommand.SBAEnable = SBAEnable;
    MasterCap.AGPCommand.FastWriteEnable = FastWrite;
    TargetCap.AGPCommand.FastWriteEnable = FastWrite;
    MasterCap.AGPCommand.FourGBEnable = AgpContext->FourGBEnable;
    TargetCap.AGPCommand.FourGBEnable = AgpContext->FourGBEnable;
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
    if ((AgpContext->SpecialTarget & AGP_FLAG_SPECIAL_VIA_AGP2_RATE_PATCH) &&
        (TargetCap.AGPStatus.Agp3Mode == OFF)) {
        switch (AgpRate) {
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
               ("UAGP35SetRate: AGP_FLAG_SPECIAL_VIA_AGP2_RATE_PATCH\n"));
    }

    //
    // Fire up AGP with new rate
    //
    ReverseInit =
        (AgpContext->SpecialTarget & AGP_FLAG_REVERSE_INITIALIZATION) ==
        AGP_FLAG_REVERSE_INITIALIZATION;
    if (ReverseInit) {
        Status = AgpLibSetMasterCapability(AgpContext, &MasterCap);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_WARNING,
                   ("UAGP35SetRate: AgpLibSetMasterCapability %08lx failed "
                    "%08lx\n",
                    &MasterCap,
                    Status));
            return Status;
        }
    }

    Status = AgpLibSetTargetCapability(AgpContext, &TargetCap);    
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING,
               ("UAGP35SetRate: AgpLibSetTargetCapability %08lx for "
                "Target failed %08lx\n",
                &TargetCap,
                Status));
        return Status;
    }

    if (!ReverseInit) {        
        Status = AgpLibSetMasterCapability(AgpContext, &MasterCap);        
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_WARNING,
                   ("UAGP35SetRate: AgpLibSetMasterCapability %08lx failed "
                    "%08lx\n",
                    &MasterCap,
                    Status));
        }
    }

    return Status;
}


