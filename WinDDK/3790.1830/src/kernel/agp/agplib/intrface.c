/*++

Copyright (c) 1997  Microsoft Corporation

Module Name:

    intrface.c

Abstract:

    Routines for implementing the AGP_BUS_INTERFACE_STANDARD interface

Author:

    John Vert (jvert) 10/26/1997

Revision History:

   Elliot Shmukler (elliots) 3/24/1999 - Added support for "favored" memory
                                          ranges for AGP physical memory allocation,
                                          fixed some bugs.

--*/
#define INITGUID 1
#include "agplib.h"

VOID
AgpLibFlushDcacheMdl(
    PMDL Mdl
    );

VOID
ApFlushDcache(
    IN PKDPC Dpc,
    IN PKEVENT Event,
    IN PMDL Mdl,
    IN PVOID SystemArgument2
    );

PMDL
AgpCombineMdlList(IN PMDL MdlList);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AgpInterfaceReference)
#pragma alloc_text(PAGE, AgpInterfaceDereference)
#pragma alloc_text(PAGE, AgpInterfaceReserveMemory)
#pragma alloc_text(PAGE, AgpInterfaceReleaseMemory)
#pragma alloc_text(PAGE, AgpInterfaceSetRate)
#pragma alloc_text(PAGE, AgpInterfaceCommitMemory)
#pragma alloc_text(PAGE, AgpInterfaceMapMemory)
#pragma alloc_text(PAGE, AgpInterfaceUnMapMemory)
#pragma alloc_text(PAGE, AgpInterfaceFreeMemory)
#pragma alloc_text(PAGE, AgpInterfaceGetAgpSize)
#pragma alloc_text(PAGE, AgpLibFlushDcacheMdl)
#pragma alloc_text(PAGE, AgpLibAllocatePhysicalMemory)
#pragma alloc_text(PAGE, AgpLibFreeMappedPhysicalMemory)
#pragma alloc_text(PAGE, AgpLibAllocateMappedPhysicalMemory)
#pragma alloc_text(PAGE, AgpCombineMdlList)
#endif


VOID
AgpInterfaceReference(
    IN PMASTER_EXTENSION Extension
    )
/*++

Routine Description:

    References an interface. Currently a NOP.

Arguments:

    Extension - Supplies the device extension

Return Value:

    None

--*/

{

    PAGED_CODE();

    InterlockedIncrement(&Extension->InterfaceCount);

}


VOID
AgpInterfaceDereference(
    IN PMASTER_EXTENSION Extension
    )
/*++

Routine Description:

    Dereferences an interface. Currently a NOP.

Arguments:

    Extension - Supplies the device extension

Return Value:

    None

--*/

{

    PAGED_CODE();

    InterlockedDecrement(&Extension->InterfaceCount);

}


NTSTATUS
AgpInterfaceReserveMemory(
    IN PMASTER_EXTENSION Extension,
    IN ULONG NumberOfPages,
    IN MEMORY_CACHING_TYPE MemoryType,
    OUT PVOID *MapHandle,
    OUT OPTIONAL PHYSICAL_ADDRESS *PhysicalAddress
    )
/*++

Routine Description:

    Reserves memory in the specified aperture

Arguments:

    Extension - Supplies the device extension where physical address space should be reserved.

    NumberOfPages - Supplies the number of pages to reserve.

    MemoryType - Supplies the memory caching type.

    MapHandle - Returns the mapping handle to be used on subsequent calls.

    PhysicalAddress - If present, returns the physical address in the aperture of the reserved 
            space

Return Value:

    NTSTATUS

--*/

{
    PVOID AgpContext;
    NTSTATUS Status;
    PHYSICAL_ADDRESS MemoryBase;
    PAGP_RANGE Range;

    PAGED_CODE();                              

    AgpContext = GET_AGP_CONTEXT_FROM_MASTER(Extension);

    Range = ExAllocatePoolWithTag(PagedPool,
                                  sizeof(AGP_RANGE),
                                  'RpgA');
    if (Range == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    Range->CommittedPages = 0;
    Range->NumberOfPages = NumberOfPages;
    Range->Type = MemoryType;

    LOCK_MASTER(Extension);
    Status = AgpReserveMemory(AgpContext,
                              Range);
    UNLOCK_MASTER(Extension);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AgpInterfaceReserveMemory - reservation of %x pages of type %d failed %08lx\n",
                NumberOfPages,
                MemoryType,
                Status));
    } else {
        AGPLOG(AGP_NOISE,
               ("AgpInterfaceReserveMemory - reserved %x pages of type %d at %I64X\n",
                NumberOfPages,
                MemoryType,
                Range->MemoryBase.QuadPart));
    }

    *MapHandle = Range;
    if (ARGUMENT_PRESENT(PhysicalAddress)) {
        *PhysicalAddress = Range->MemoryBase;
    }
    return(Status);
}


NTSTATUS
AgpInterfaceReleaseMemory(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle
    )
/*++

Routine Description:

    Releases memory in the specified aperture that was previously reserved by
    AgpInterfaceReserveMemory

Arguments:

    Extension - Supplies the device extension where physical address space should be reserved.

    MapHandle - Supplies the mapping handle returned from AgpInterfaceReserveMemory

Return Value:

    NTSTATUS

--*/

{
    PAGP_RANGE Range;
    PVOID AgpContext;
    NTSTATUS Status;
    PHYSICAL_ADDRESS MemoryBase;

    PAGED_CODE();

    AgpContext = GET_AGP_CONTEXT_FROM_MASTER(Extension);
    Range = (PAGP_RANGE)MapHandle;

    LOCK_MASTER(Extension);
    //
    // Make sure the range is empty
    //
    ASSERT(Range->CommittedPages == 0);
    if (Range->CommittedPages != 0) {
        AGPLOG(AGP_CRITICAL,
               ("AgpInterfaceReleaseMemory - Invalid attempt to release non-empty range %08lx\n",
                Range));
        UNLOCK_MASTER(Extension);
        return(STATUS_INVALID_PARAMETER);
    }

    AGPLOG(AGP_NOISE,
           ("AgpInterfaceReleaseMemory - releasing range %08lx, %lx pages at %08lx\n",
            Range,
            Range->NumberOfPages,
            Range->MemoryBase.QuadPart));
    Status = AgpReleaseMemory(AgpContext,
                              Range);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AgpInterfaceReleaseMemory - release failed %08lx\n",
                Status));
    }
    UNLOCK_MASTER(Extension);
    ExFreePool(Range);
    return(Status);
}


NTSTATUS
AgpInterfaceCommitMemory(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    IN OUT PMDL Mdl OPTIONAL,
    OUT PHYSICAL_ADDRESS *MemoryBase
    )
/*++

Routine Description:

    Commits memory into the specified aperture that was previously reserved by
    AgpInterfaceReserveMemory

Arguments:

    Extension - Supplies the device extension where physical address space should
        be committed.

    MapHandle - Supplies the mapping handle returned from AgpInterfaceReserveMemory

    NumberOfPages - Supplies the number of pages to be committed.

    OffsetInPages - Supplies the offset, in pages, into the aperture reserved by
        AgpInterfaceReserveMemory

    Mdl - Returns the MDL describing the pages of memory committed.

    MemoryBase - Returns the physical memory address of the committed memory.

Return Value:

    NTSTATUS

--*/

{
    PAGP_RANGE Range = (PAGP_RANGE)MapHandle;
    PMDL NewMdl;
    PVOID AgpContext;
    NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;
    ULONG RunLength, RunOffset;
    ULONG CurrentLength, CurrentOffset;
    PMDL FirstMdl=NULL;

    PAGED_CODE();

    AgpContext = GET_AGP_CONTEXT_FROM_MASTER(Extension);

    ASSERT(NumberOfPages <= Range->NumberOfPages);
    ASSERT(NumberOfPages > 0);
    ASSERT((Mdl == NULL) || (Mdl->ByteCount == PAGE_SIZE * NumberOfPages));

    CurrentLength = NumberOfPages;
    CurrentOffset = OffsetInPages;

    LOCK_MASTER(Extension);
    do {

        //
        // Save ourselves the trouble...
        //
        if (!(CurrentLength > 0)) {
            break;
        }

        //
        // Find the first free run in the supplied range.
        //
        AgpFindFreeRun(AgpContext,
                       Range,
                       CurrentLength,
                       CurrentOffset,
                       &RunLength,
                       &RunOffset);

        if (RunLength > 0) {
            ASSERT(RunLength <= CurrentLength);
            ASSERT(RunOffset >= CurrentOffset);
            ASSERT(RunOffset < CurrentOffset + CurrentLength);
            ASSERT(RunOffset + RunLength <= CurrentOffset + CurrentLength);

            //
            // Compute the next offset and length
            //
            CurrentLength -= (RunOffset - CurrentOffset) + RunLength;
            CurrentOffset = RunOffset + RunLength;

            //
            // Get an MDL from memory management big enough to map the 
            // requested range.
            //

            NewMdl = AgpLibAllocatePhysicalMemory(AgpContext, RunLength * PAGE_SIZE);
            
            //
            // This can fail in two ways, either no memory is available at all (NewMdl == NULL)
            // or some pages were available, but not enough. (NewMdl->ByteCount < Length)
            //
            if (NewMdl == NULL) {
                AGPLOG(AGP_CRITICAL,
                       ("AgpInterfaceReserveMemory - Couldn't allocate pages for %lx bytes\n",
                        RunLength));
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            } else if (BYTES_TO_PAGES(NewMdl->ByteCount) < RunLength) {
                AGPLOG(AGP_CRITICAL,
                       ("AgpInterfaceCommitMemory - Only allocated enough pages for %lx of %lx bytes\n",
                        NewMdl->ByteCount,
                        RunLength));
                Status = STATUS_INSUFFICIENT_RESOURCES;
                MmFreePagesFromMdl(NewMdl);
                ExFreePool(NewMdl);
                break;
            }

            //
            // Now that we have our MDL, we can map this into the specified
            // range.
            //
            if (AgpFlushPages != NULL) {
                if (!NT_SUCCESS((AgpFlushPages)(AgpContext, NewMdl))) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    MmFreePagesFromMdl(NewMdl);
                    ExFreePool(NewMdl);
                    break;
                }
            } else {
#if (WINVER < 0x502)
                AgpLibFlushDcacheMdl(NewMdl);
#else
#ifdef _IA64_
                ASSERT(FALSE);
#else
                KeInvalidateAllCaches(); //AgpLibFlushDcacheMdl(NewMdl);
#endif // _IA64_
#endif // (WINVER < 0x502)
            }
            Status = AgpMapMemory(AgpContext,
                                  Range,
                                  NewMdl,
                                  RunOffset,
                                  MemoryBase);
            if (!NT_SUCCESS(Status)) {
                AGPLOG(AGP_CRITICAL,
                       ("AgpInterfaceCommitMemory - AgpMapMemory for Mdl %08lx in range %08lx failed %08lx\n",
                        NewMdl,
                        Range,
                        Status));
                MmFreePagesFromMdl(NewMdl);
                ExFreePool(NewMdl);
                break;
            }
            Range->CommittedPages += RunLength;

            //
            // Add this MDL to our list of allocated MDLs for cleanup
            // If we need to cleanup, we will also need to know the page offset
            // so that we can unmap the memory. Stash that value in the ByteOffset
            // field of the MDL (ByteOffset is always 0 for our MDLs)
            //
            NewMdl->ByteOffset = RunOffset;
            NewMdl->Next = FirstMdl;
            FirstMdl = NewMdl;
        }

    } while (RunLength > 0);

    //
    // Cleanup the MDLs. If the allocation failed, we need to
    // unmap them and free the pages and the MDL itself. If the
    // operation completed successfully, we just need to free the
    // MDL.
    //
    while (FirstMdl) {
        NewMdl = FirstMdl;
        FirstMdl = NewMdl->Next;
        if (!NT_SUCCESS(Status)) {

            //
            // Unmap the memory that was mapped. The ByteOffset field
            // of the MDL is overloaded here to store the offset in pages
            // into the range.
            //
            AgpUnMapMemory(AgpContext,
                           Range,
                           NewMdl->ByteCount / PAGE_SIZE,
                           NewMdl->ByteOffset);
            NewMdl->ByteOffset = 0;
            Range->CommittedPages -= NewMdl->ByteCount / PAGE_SIZE;
            MmFreePagesFromMdl(NewMdl);
        }
        ExFreePool(NewMdl);
    }

    if (NT_SUCCESS(Status)) {

        if (Mdl) {
            //
            // Get the MDL that describes the entire mapped range.
            //
            AgpGetMappedPages(AgpContext,
                              Range,
                              NumberOfPages,
                              OffsetInPages,
                              Mdl);
        }

        MemoryBase->QuadPart = Range->MemoryBase.QuadPart + OffsetInPages * PAGE_SIZE;
    }

    UNLOCK_MASTER(Extension);
    return(Status);
}


NTSTATUS
AgpInterfaceMapMemory(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    IN PMDL Mdl,
    OUT PHYSICAL_ADDRESS *MemoryBase
    )
/*++

Routine Description:

    Maps the specified Mdl in the aperture that was previously reserved by 
    ApgInterfaceReservedMemory.

Arguments:

    Extension - Supplies the device extension where physical address space should
        be committed.

    MapHandle - Supplies the mapping handle returned from AgpInterfaceReserveMemory

    NumberOfPages - Supplies the number of pages to be committed.

    OffsetInPages - Supplies the offset, in pages, into the aperture reserved by
        AgpInterfaceReserveMemory.

    Mdl - Contains an MDL describing the pages of memory to be mapped.

    MemoryBase - Returns the physical memory address of the mapped memory.

Return Value:

    NTSTATUS

--*/
{
    PAGP_RANGE Range = (PAGP_RANGE)MapHandle;
    PVOID AgpContext;
    NTSTATUS Status=STATUS_SUCCESS;
    ULONG RunLength, RunOffset;
    PPFN_NUMBER Pages;
    PFN_NUMBER MaxPfn;
    ULONG i;

    PAGED_CODE();

    AgpContext = GET_AGP_CONTEXT_FROM_MASTER(Extension);

    ASSERT(NumberOfPages <= Range->NumberOfPages);
    ASSERT(NumberOfPages > 0);
    ASSERT(MmGetMdlByteCount(Mdl)==(PAGE_SIZE*NumberOfPages));
    
    //
    // Assume failure.
    //

    MemoryBase->QuadPart = 0;

    //
    // Verify that no pfn fall outside the valid range addressable by this device.
    //

    Pages = (PPFN_NUMBER)(Mdl + 1);
    MaxPfn = (PFN_NUMBER)(AgpMaximumAddress.QuadPart >> PAGE_SHIFT);
    for (i=0; i<NumberOfPages; i++)
    {
        if (Pages[i] > MaxPfn)
        {
            AGPLOG(AGP_CRITICAL,
                   ("AgpInterfaceMapMemory - Can't map requested Mdl, some pfn are out of range.\n"));
            return STATUS_INVALID_PARAMETER;
        }
    }

    LOCK_MASTER(Extension);

    //
    // Find the first free run in the supplied range.
    //

    AgpFindFreeRun(AgpContext,
                   Range,
                   NumberOfPages,
                   OffsetInPages,
                   &RunLength,
                   &RunOffset);


    if ((RunLength == NumberOfPages) && (RunOffset == OffsetInPages))
    {
        //
        // Flush all the pages in the Mdl from the processor data cache.
        //

        if (AgpFlushPages != NULL) {
            if (!NT_SUCCESS((AgpFlushPages)(AgpContext, Mdl))) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {
#if (WINVER < 0x502)
            AgpLibFlushDcacheMdl(Mdl);
#else
#ifdef _IA64_
            ASSERT(FALSE);
#else
            KeInvalidateAllCaches();
#endif // _IA64_
#endif // (WINVER < 0x502)
        }
        
        //
        // Map the Mdl in the GART.
        //

        if (NT_SUCCESS(Status))
        {
            Status = AgpMapMemory(AgpContext,
                                  Range,
                                  Mdl,
                                  RunOffset,
                                  MemoryBase);
        }

        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AgpInterfaceMapMemory - AgpMapMemory for Mdl %08lx in range %08lx failed %08lx\n",
                    Mdl,
                    Range,
                    Status));
        }
        Range->CommittedPages += RunLength;
    }
    else
    {
        //
        // The runlenght and runoffset aren't the same as the current length
        // and offset, which means some pages in the range where already commited. 
        // In this case we refuse to map the new Mdl in the range.
        //

        AGPLOG(AGP_CRITICAL,
               ("AgpInterfaceMapMemory - Range to map already contains commited pages.\n"));

        Status = STATUS_INVALID_PARAMETER;
    }

    UNLOCK_MASTER(Extension);
    return(Status);
}


NTSTATUS
AgpInterfaceFreeMemory(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages
    )
/*++

Routine Description:

    Frees memory previously committed by AgpInterfaceCommitMemory

Arguments:

    Extension - Supplies the device extension where physical address space should
        be freed.

    MapHandle - Supplies the mapping handle returned from AgpInterfaceReserveMemory

    NumberOfPages - Supplies the number of pages to be freed.

    OffsetInPages - Supplies the start of the range to be freed.

Return Value:

    NTSTATUS

--*/

{
    PAGP_RANGE Range = (PAGP_RANGE)MapHandle;
    PVOID AgpContext;
    NTSTATUS Status;
    PMDL FreeMdl;

    PAGED_CODE();

    AgpContext = GET_AGP_CONTEXT_FROM_MASTER(Extension);

    ASSERT(OffsetInPages < Range->NumberOfPages);
    ASSERT(OffsetInPages + NumberOfPages <= Range->NumberOfPages);
    //
    // Make sure the supplied address is within the reserved range
    //
    if ((OffsetInPages >= Range->NumberOfPages) ||
        (OffsetInPages + NumberOfPages > Range->NumberOfPages)) {
        AGPLOG(AGP_WARNING,
               ("AgpInterfaceFreeMemory - Invalid free of %x pages at offset %x from range %I64X (%x pages)\n",
                NumberOfPages,
                OffsetInPages,
                Range->MemoryBase.QuadPart,
                Range->NumberOfPages));
        return(STATUS_INVALID_PARAMETER);
    }

    //
    // Allocate an MDL big enough to contain the pages to be unmapped.
    //
    FreeMdl =
        IoAllocateMdl(NULL, NumberOfPages * PAGE_SIZE, FALSE, TRUE, NULL);
    
    if (FreeMdl == NULL) {

        //
        // This is kind of a sticky situation. We can't allocate the memory
        // that we need to free up some memory! I guess we could have a small
        // MDL on our stack and free things that way.
        //
        // John Vert (jvert) 11/11/1997
        // implement this
        //
        ASSERT(FreeMdl != NULL);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    LOCK_MASTER(Extension);

    //
    // Get the MDL that describes the entire mapped range
    //
    AgpGetMappedPages(AgpContext, 
                      Range,
                      NumberOfPages,
                      OffsetInPages,
                      FreeMdl);
    //
    // Unmap the memory
    //
    Status = AgpUnMapMemory(AgpContext,
                            Range,
                            NumberOfPages,
                            OffsetInPages);
    UNLOCK_MASTER(Extension);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AgpInterfaceFreeMemory - UnMapMemory for %x pages at %I64X failed %08lx\n",
                NumberOfPages,
                Range->MemoryBase.QuadPart + OffsetInPages * PAGE_SIZE,
                Status));
    } else {
        //
        // Free the pages
        //
        MmFreePagesFromMdl(FreeMdl);
        ASSERT(Range->CommittedPages >= NumberOfPages);
        Range->CommittedPages -= NumberOfPages;
    }

    //
    // Free the MDL we allocated.
    //
    IoFreeMdl(FreeMdl);
    return(Status);
}


NTSTATUS
AgpInterfaceUnMapMemory(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    IN PMDL Mdl
    )
/*++

Routine Description:

    Unmap memory previously mapped through AgpInterfaceMapMemory.
    
    Note that this function validates that the pages being unmapped are really
    those that were mapped by a previous call to AgpInterfaceMapMemory. This
    is done by validating that the pages that are going to be unmapped corresponds
    to those in the Mdl received in parameter. It is necessary to do this validation
    to catch potential page frame leak in the scenario where a client would 
    incorrectly unmapped a previously commited range (AgpInterfaceCommitMemory).
    In this later case we would lose track of the allocated pages and thus they
    would leak.

Arguments:

    Extension - Supplies the device extension where physical address space should
        be freed.

    MapHandle - Supplies the mapping handle returned from AgpInterfaceReserveMemory

    NumberOfPages - Supplies the number of pages to be freed.

    OffsetInPages - Supplies the start of the range to be freed.

    Mdl - Contains an MDL specifying the pages that were mapped and now need to be 
        unmapped.

Return Value:

    NTSTATUS

--*/
{
    PAGP_RANGE Range = (PAGP_RANGE)MapHandle;
    PVOID AgpContext;
    NTSTATUS Status = STATUS_SUCCESS;
    PMDL FreeMdl;
    PPFN_NUMBER Pages;
    PPFN_NUMBER PagesToFree;
    ULONG i;

    PAGED_CODE();

    AgpContext = GET_AGP_CONTEXT_FROM_MASTER(Extension);

    ASSERT(OffsetInPages < Range->NumberOfPages);
    ASSERT(OffsetInPages + NumberOfPages <= Range->NumberOfPages);
    ASSERT(Mdl->ByteCount == PAGE_SIZE * NumberOfPages);
    
    //
    // Make sure the supplied address is within the reserved range.
    //
    if ((OffsetInPages >= Range->NumberOfPages) ||
        (OffsetInPages + NumberOfPages > Range->NumberOfPages) ||
        (Mdl->ByteCount != PAGE_SIZE * NumberOfPages)) {
        AGPLOG(AGP_WARNING,
               ("AgpInterfaceUnmapMemory - Invalid unmapping of %x pages at offset %x from range %I64X (%x pages)\n",
                NumberOfPages,
                OffsetInPages,
                Range->MemoryBase.QuadPart,
                Range->NumberOfPages));
        return(STATUS_INVALID_PARAMETER);
    }

    //
    // Allocate an MDL big enough to contain the pages to be unmapped.
    //
    FreeMdl =
        IoAllocateMdl(NULL, NumberOfPages * PAGE_SIZE, FALSE, TRUE, NULL);
    
    if (FreeMdl == NULL) {

        //
        // This is kind of a sticky situation. We can't allocate the memory
        // that we need to free up some memory! I guess we could have a small
        // MDL on our stack and free things that way.
        //
        // John Vert (jvert) 11/11/1997
        // implement this
        //
        ASSERT(FreeMdl != NULL);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    LOCK_MASTER(Extension);

    //
    // Get the MDL that describes the entire mapped range.
    //
    AgpGetMappedPages(AgpContext, 
                      Range,
                      NumberOfPages,
                      OffsetInPages,
                      FreeMdl);
    //
    // Verify that the Mdl that the pages that were mapped inside the GART are
    // really those the client wanted to unmap. 
    //
    Pages = (PPFN_NUMBER)(Mdl + 1);
    PagesToFree = (PPFN_NUMBER)(FreeMdl + 1);

    for (i=0; i<NumberOfPages; i++)
    {
        if (Pages[i] != PagesToFree[i])
        {
            AGPLOG(AGP_CRITICAL,
                   ("AgpInterfaceUnmapMemory - Mismatch pfn for unmapped operation.\n"));
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
    }

    //
    // Unmap the memory
    //
    if (NT_SUCCESS(Status))
    {
        Status = AgpUnMapMemory(AgpContext,
                                Range,
                                NumberOfPages,
                                OffsetInPages);    
    }
    
    UNLOCK_MASTER(Extension);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AgpInterfaceUnmapMemory - UnMapMemory for %x pages at %I64X failed %08lx\n",
                NumberOfPages,
                Range->MemoryBase.QuadPart + OffsetInPages * PAGE_SIZE,
                Status));
    } else {
        //
        // Free the pages.
        //
        ASSERT(Range->CommittedPages >= NumberOfPages);
        Range->CommittedPages -= NumberOfPages;
    }

    //
    // Free the MDL we allocated.
    //
    IoFreeMdl(FreeMdl);
    return(Status);
}


NTSTATUS
AgpInterfaceGetMappedPages(
    IN PMASTER_EXTENSION Extension,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    OUT PMDL Mdl
    )
/*++

Routine Description:

    Returns the list of physical pages mapped backing the specified range.

Arguments:

    Extension - Supplies the device extension where physical address space should
        be freed.

    MapHandle - Supplies the mapping handle returned from AgpInterfaceReserveMemory

    NumberOfPages - Supplies the number of pages to be returned

    OffsetInPages - Supplies the start of the rangion

Return Value:

    NTSTATUS

--*/

{
    PAGP_RANGE Range = (PAGP_RANGE)MapHandle;
    PVOID AgpContext;
    NTSTATUS Status;

    PAGED_CODE();

    AgpContext = GET_AGP_CONTEXT_FROM_MASTER(Extension);

    ASSERT(NumberOfPages <= Range->NumberOfPages);
    ASSERT(NumberOfPages > 0);
    ASSERT(OffsetInPages < Range->NumberOfPages);
    ASSERT(OffsetInPages + NumberOfPages <= Range->NumberOfPages);
    ASSERT(Mdl->ByteCount == PAGE_SIZE * NumberOfPages);

    //
    // Make sure the supplied address is within the reserved range
    //
    if ((OffsetInPages >= Range->NumberOfPages) ||
        (OffsetInPages + NumberOfPages > Range->NumberOfPages)) {
        AGPLOG(AGP_WARNING,
               ("AgpInterfaceGetMappedPages - Invalid 'get' of %x pages at offset %x from range %I64X (%x pages)\n",
                NumberOfPages,
                OffsetInPages,
                Range->MemoryBase.QuadPart,
                Range->NumberOfPages));
        return(STATUS_INVALID_PARAMETER);
    }

    //
    // Get the MDL that describes the entire mapped range
    //
    LOCK_MASTER(Extension);

    AgpGetMappedPages(AgpContext, 
                      Range,
                      NumberOfPages,
                      OffsetInPages,
                      Mdl);

    UNLOCK_MASTER(Extension);
    return(STATUS_SUCCESS);
}

//
// 32-bit default AGP address space
//
PHYSICAL_ADDRESS AgpMaximumAddress = { (ULONG)-1, 0 };


PMDL
AgpLibAllocatePhysicalMemory(IN PVOID AgpContext, IN ULONG TotalBytes)
/*++

Routine Description:

    Allocates a set of physical memory pages for use by the AGP driver.
    
    This routine uses MmAllocatePagesForMdl to attempt to allocate
    as many of the pages as possible within favored AGP memory
    ranges (if any).

Arguments:

    AgpContext   - The AgpContext

    TotalBytes   - The total amount of bytes to allocate.

Return Value:

    An MDL that describes the allocated physical pages or NULL
    if this function is unsuccessful. 
    
    NOTE: Just like MmAllocatePagesForMdl, this function can return
    an MDL that describes an allocation smaller than TotalBytes in size.

--*/
{
   PHYSICAL_ADDRESS ZeroAddress;
   PMDL MdlList = NULL, NewMdl = NULL;
   PTARGET_EXTENSION Extension;
   ULONG i, PagesNeeded;

   PAGED_CODE();

   AGPLOG(AGP_NOISE, ("AGPLIB: Attempting to allocate memory = %u pages.\n", 
            BYTES_TO_PAGES(TotalBytes)));

   // Initialize some stuff

   ZeroAddress.QuadPart = 0;
   
   AGPLOG(AGP_NOISE, ("AGPLIB: Max memory set to %I64x.\n", AgpMaximumAddress.QuadPart));

   GET_TARGET_EXTENSION(Extension, AgpContext);

   // How many pages do we need?

   PagesNeeded = BYTES_TO_PAGES(TotalBytes);

   //
   // Loop through each favored range, attempting to allocate
   // as much as possible from that range within the bounds
   // of what we actually need.
   //

   for (i = 0; i < Extension->FavoredMemory.NumRanges; i++) {
      AGPLOG(AGP_NOISE, 
             ("AGPLIB: Trying to allocate %u pages from range %I64x - %I64x.\n",
               PagesNeeded, 
               Extension->FavoredMemory.Ranges[i].Lower,               
               Extension->FavoredMemory.Ranges[i].Upper));      

      NewMdl = MmAllocatePagesForMdl(Extension->FavoredMemory.Ranges[i].Lower,
                                     Extension->FavoredMemory.Ranges[i].Upper,
                                     ZeroAddress,                                     
                                     PagesNeeded << PAGE_SHIFT);
      if (NewMdl) {
         AGPLOG(AGP_NOISE, ("AGPLIB: %u pages allocated in range.\n",
                  NewMdl->ByteCount >> PAGE_SHIFT));
         
         PagesNeeded -= BYTES_TO_PAGES(NewMdl->ByteCount);
         
         //
         // Build a list of the MDls used
         // for each range-based allocation
         //

         NewMdl->Next = MdlList;
         MdlList = NewMdl;

         // Stop allocating if we are finished.

         if (PagesNeeded == 0) break;
         

      } else {
         AGPLOG(AGP_NOISE, ("AGPLIB: NO pages allocated in range.\n"));
      }
      
   }

   //
   // Attempt to allocate from ALL of physical memory
   // if we could not complete our allocation with only
   // the favored memory ranges.
   //

   if (PagesNeeded > 0) {

      AGPLOG(AGP_NOISE, ("AGPLIB: Global Memory allocation for %u pages.\n", 
               PagesNeeded));

      NewMdl = MmAllocatePagesForMdl(ZeroAddress,
                                     AgpMaximumAddress,
                                     ZeroAddress,
                                     PagesNeeded << PAGE_SHIFT);
      if (NewMdl) {

         AGPLOG(AGP_NOISE, ("AGPLIB: Good Global Memory Alloc for %u pages.\n",
                  NewMdl->ByteCount >> PAGE_SHIFT));

         //
         // Add this MDL to the list as well
         //

         NewMdl->Next = MdlList;
         MdlList = NewMdl;
      } else {

         AGPLOG(AGP_NOISE, ("AGPLIB: Failed Global Memory Alloc.\n"));

      }

   }

   // We now have a list of Mdls in MdlList that give us the best
   // possible memory allocation taking favored ranges into account.

   // What we now need to do is combine this Mdl list into one mdl.

   NewMdl = AgpCombineMdlList(MdlList);

   if (!NewMdl && MdlList) {
      AGPLOG(AGP_WARNING, ("AGPLIB: Could not combine MDL List!\n"));

      // This is bad. The mdl list could not be combined probably 
      // because a large enough mdl could not be allocated for 
      // the combination.

      // This is not the end of the world however, since the mdl list
      // is not modified until its combination has succeeded so we 
      // still have a valid list. But we need it in one Mdl, so 
      // we just fall back to the simplest allocation strategy
      // we have available:

      // 1. Destroy the list and all of its allocations.
      
      while(MdlList)
      {
         MmFreePagesFromMdl(MdlList);
         NewMdl = MdlList->Next;
         ExFreePool(MdlList);
         MdlList = NewMdl;
      }

      // 2. Allocate a single Mdl with our pages without regard
      // for favored memory ranges. 

      NewMdl = MmAllocatePagesForMdl(ZeroAddress, 
                                     AgpMaximumAddress,
                                     ZeroAddress,
                                     TotalBytes);

   }

   return NewMdl;

   
}

PMDL
AgpCombineMdlList(IN PMDL MdlList)
/*++

Routine Description:

    Combines a list of MDLs that describe some set of physical memory
    pages into a single MDL that describes the same set of pages.
    
    The MDLs in the list should be of the type produced by
    MmAllocatePagesForMdl (i.e. MDLs that are useful for nothing more
    than as an array of PFNs)

    This function is used by AgpLibAllocatePhysicalMemory in order
    to combine its multiple range-based allocations into 1 MDL.
    
Arguments:

    MdlList - A list of MDLs to be combines

Return Value:

    A single MDL that describes the same set of physical pages as
    the MDLs in MdlList or NULL if this function is unsuccessful.
    
    NOTE: This function will deallocate the Mdls in MdlList if it
    is successful. If it is unsuccessful, however, it will leave
    the MdlList intact.

--*/
{
   PMDL NewMdl = NULL, Mdl, MdlTemp;
   ULONG Pages = 0;
   PPFN_NUMBER NewPageArray, PageArray;

   ULONG i; // for debugging only

   PAGED_CODE();

   if ((MdlList == NULL) || (MdlList->Next == NULL)) {

      // List of 0 or 1 elements, no need for this 
      // function to do anything.

      return MdlList;
   }

   // Calculate the number of pages spanned by this MdlList.

   for(Mdl = MdlList; Mdl; Mdl = Mdl->Next)
      Pages += BYTES_TO_PAGES(Mdl->ByteCount);

   // Allocate a new Mdl of the proper size.

   NewMdl = IoAllocateMdl(NULL, Pages << PAGE_SHIFT, FALSE, TRUE, NULL);

   if (!NewMdl) {

      // Chances are that the system will bugcheck before
      // this actually happens ... but whatever.

      return NULL;
   }

   // Run through the mdl list, combining the mdls found
   // into a new mdl.

   //
   // First, get a pointer to the PFN array of the new Mdl
   //

   NewPageArray = MmGetMdlPfnArray(NewMdl);

   for(Mdl = MdlList; Mdl; Mdl = Mdl->Next)
   {
      // Get a pointer to the physical page number array in this Mdl.

      PageArray = MmGetMdlPfnArray(Mdl);
      Pages = BYTES_TO_PAGES(Mdl->ByteCount);

      // Copy this array into a proper slot in the array area of the new Mdl.

      RtlCopyMemory((PVOID)NewPageArray, 
                    (PVOID)PageArray,
                    sizeof(PFN_NUMBER) * Pages);

      // Adjust new array slot pointer appropriately for the next copy

      NewPageArray += Pages;
         
   }

   // The list has been combined, now we need to destroy the Mdls
   // in the list.

   Mdl = MdlList;
   while(Mdl)
   {
      MdlTemp = Mdl->Next;
      ExFreePool(Mdl);
      Mdl = MdlTemp;
   }

   // All done. Return the new combined Mdl.

   return NewMdl;
}

VOID
AgpLibFreeMappedPhysicalMemory(
    IN PVOID Addr,
    IN ULONG Length
    )
/*++

Routine Description:

Arguments:

    Addr - The virtual address of the allocation

    Length - Length of allocation in bytes

Return Value:

    None

--*/
{
    ULONG Index;
    PMDL FreeMdl;
    ULONG Pages;
    PPFN_NUMBER Page;

    PAGED_CODE();

    //
    // Allocate an MDL big enough to contain the pages to be unmapped
    //
    FreeMdl = IoAllocateMdl(Addr, Length, FALSE, TRUE, NULL);
    
    //
    // We could not allocate a MDL to free memory, we will free one page at
    // a time using a MDL on our stack
    //
    if (FreeMdl == NULL) {
        PCCHAR VAddr;
        MDL MdlBuf[2]; // NOTE: We use this second MDL to store a
                       //       single PFN_NUMBER

        ASSERT(sizeof(PFN_NUMBER) <= sizeof(MDL));

        FreeMdl = &MdlBuf[0];
        RtlZeroMemory(FreeMdl, 2 * sizeof(MDL));

        Pages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(Addr, Length);
        Page = (PPFN_NUMBER)(FreeMdl + 1);

        //
        // Take care not to create a MDL that spans more than a page
        //
        VAddr = PAGE_ALIGN(Addr);
        for (Index = 0; Index < Pages; Index++) {
            MmInitializeMdl(FreeMdl, VAddr, PAGE_SIZE);
            *Page = (PFN_NUMBER)(MmGetPhysicalAddress(VAddr).QuadPart >>
                                 PAGE_SHIFT);
            MmFreePagesFromMdl(FreeMdl);
            VAddr += PAGE_SIZE;
        }

        return;
    }

    Page = (PPFN_NUMBER)(FreeMdl + 1);
    Pages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(Addr, Length);
    
    //
    // Fill in the PFN array for the MDL
    //
    for (Index = 0; Index < Pages; Index++) {
        *Page++ = (PFN_NUMBER)(MmGetPhysicalAddress((PCCHAR)Addr + (Index * PAGE_SIZE)).QuadPart >> PAGE_SHIFT);
    }

    MmFreePagesFromMdl(FreeMdl);
    IoFreeMdl(FreeMdl);
}


PVOID
AgpLibAllocateMappedPhysicalMemory(IN PVOID AgpContext, IN ULONG TotalBytes)
/*++

Routine Description:

    Same as AgpLibAllocatePhysicalMemory, except this function will
    also map the allocated memory to a virtual address.

Arguments:

    Same as AgpLibAllocatePhysicalMemory.

Return Value:

    A virtual address of the allocated memory or NULL if unsuccessful.

--*/
{
   PMDL Mdl;
   PVOID Ret;

   PAGED_CODE();
   
   AGPLOG(AGP_NOISE, 
          ("AGPLIB: Attempting to allocate mapped memory = %u.\n", TotalBytes));

   //
   // Call the real memory allocator.
   //
   
   Mdl = AgpLibAllocatePhysicalMemory(AgpContext, TotalBytes);

   // Two possible failures 

   // 1. MDL is NULL. No memory could be allocated.

   if (Mdl == NULL) {

      AGPLOG(AGP_WARNING, ("AGPMAP: Could not allocate anything.\n"));

      return NULL;
   }

   // 2. MDL has some pages allocated but not enough.

   if (Mdl->ByteCount < TotalBytes) {

      AGPLOG(AGP_WARNING, ("AGPMAP: Could not allocate enough.\n"));

      MmFreePagesFromMdl(Mdl);
      ExFreePool(Mdl);
      return NULL;
   }

   // Ok. Our allocation succeeded. Map it to a virtual address.
   
   // Step 1: Map the locked Pages. (will return NULL if failed)

   Mdl->MdlFlags |= MDL_PAGES_LOCKED;
   Ret = MmMapLockedPagesSpecifyCache (Mdl,
                                         KernelMode,
                                         MmNonCached,
                                         NULL,
                                         FALSE,
                                         HighPagePriority);

   // Don't need the Mdl anymore, whether we succeeded or failed. 

   ExFreePool(Mdl);

   if (Ret == NULL) {
      AGPLOG(AGP_WARNING, ("AGPMAP: Could not map.\n"));
   } 

   return Ret;
}

#if (WINVER < 0x502)

#if defined (_X86_)
#define FLUSH_DCACHE(Mdl) __asm{ wbinvd }
#else
#define FLUSH_DCACHE(Mdl)   \
            AGPLOG(AGP_CRITICAL,    \
                   ("AgpLibFlushDcacheMdl - NEED TO IMPLEMENT DCACHE FLUSH FOR THIS ARCHITECTURE!!\n"))
#endif


VOID
AgpLibFlushDcacheMdl(
    PMDL Mdl
    )
/*++

Routine Description:

    Flushes the specified MDL from the D-caches of all processors
    in the system.

    Current algorithm is to set the current thread's affinity to each 
    processor in turn and flush the dcache. This could be made a lot
    more efficient if this turns out to be a hot codepath
    
Arguments:

    Mdl - Supplies the MDL to be flushed.

Return Value:

    None.

--*/

{
    NTSTATUS Status;
    KAFFINITY Processors;
    UCHAR Number;
    KEVENT Event;
    KDPC Dpc;

    PAGED_CODE();
    Processors = KeQueryActiveProcessors();
    //
    // Quick out for the UP case.
    //
    if (Processors == 1) {
        FLUSH_DCACHE(Mdl);
        return;
    }

    //
    // We will invoke a DPC on each processor. That DPC will flush the cache,
    // set the event and return. 
    //
    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    Number = 0;
    while (Processors) {
        if (Processors & 1) {
            //
            // Initialize the DPC and set it to run on the specified
            // processor.
            //
            KeInitializeDpc(&Dpc,ApFlushDcache, &Event);
            KeSetTargetProcessorDpc(&Dpc, Number);

            //
            // Queue the DPC and wait for it to finish its work.
            //
            KeClearEvent(&Event);
            KeInsertQueueDpc(&Dpc, Mdl, NULL);
            KeWaitForSingleObject(&Event, 
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }
        Processors = Processors >> 1;
        ++Number;
    }
}



VOID
ApFlushDcache(
    IN PKDPC Dpc,
    IN PKEVENT Event,
    IN PMDL Mdl,
    IN PVOID SystemArgument2
    )
/*++

Routine Description:

    DPC which executes on each processor in turn to flush the
    specified MDL out of the dcache on each.

Arguments:

    Dpc - supplies the DPC object

    Event - Supplies the event to signal when the DPC is complete

    Mdl - Supplies the MDL to be flushed from the dcache

Return Value:

    None

--*/

{
    FLUSH_DCACHE(Mdl);
    KeSetEvent(Event, 0, FALSE);
}
#endif // (WINVER < 0x502)

#define AGP_SET_RATE_MASK        0x0000000F


NTSTATUS
AgpInterfaceSetRate(
    IN PMASTER_EXTENSION Extension,
    IN ULONG AgpRate
    )
/*++

Routine Description:

    This routine sets the AGP rate

Arguments:

    Extension - Supplies the device extension

    AgpRate - Rate to set

Return Value:

    STATUS_SUCCESS, or error status

--*/
{
    ULONGLONG DeviceFlags = 0;

    PAGED_CODE();

    //
    // Allow video driver to disable side band addressing, and fast writes
    //

    //
    // Check if AGP3 mode is enabled
    //
    if ((AgpRate & AGP_SET_RATE_DISABLE_SBA) == AGP_SET_RATE_DISABLE_SBA) {
        NTSTATUS Status;
        PCI_AGP_CAPABILITY Capability;

        Status =
            AgpLibGetMasterCapability(GET_AGP_CONTEXT_FROM_MASTER(Extension),
                                      &Capability);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        //
        // AGP3 mode is enabled, therefore SBA cannot be disabled, and this
        // is an invalid request
        //
        if (Capability.AGPStatus.Agp3Mode == 1) {
            return STATUS_INVALID_PARAMETER;
        }

        Globals.VpOverrideFlags |= AGP_FLAG_NO_SBA_ENABLE;
    }

    if ((AgpRate & AGP_SET_RATE_DISABLE_FW) == AGP_SET_RATE_DISABLE_FW) {
        Globals.VpOverrideFlags |= AGP_FLAG_NO_FW_ENABLE;
    }

    switch (AgpRate & AGP_SET_RATE_MASK) {
        case 0:
            DeviceFlags = AGP_FLAG_SET_RATE_0X;
            break;
        case PCI_AGP_RATE_1X:
            DeviceFlags = AGP_FLAG_SET_RATE_1X;
            break;
        case PCI_AGP_RATE_2X:
            DeviceFlags = AGP_FLAG_SET_RATE_2X;
            break;
        case PCI_AGP_RATE_4X:
            DeviceFlags = AGP_FLAG_SET_RATE_4X;
            break;
        case 8:
            DeviceFlags = AGP_FLAG_SET_RATE_8X;
            break;
    }

    if (DeviceFlags != 0) {
        return AgpSpecialTarget(GET_AGP_CONTEXT_FROM_MASTER(Extension),
                                DeviceFlags);
    }

    return STATUS_INVALID_PARAMETER;
}

#define __2MB (2 * 1024 * 1024)
#define MIN_AGP_PAGES __2MB / PAGE_SIZE


NTSTATUS
AgpInterfaceGetAgpSize(
    IN PMASTER_EXTENSION Extension,
    OUT PHYSICAL_ADDRESS *AgpBase,
    OUT SIZE_T *AgpSize
    )
/*++

Routine Description:

    Determines the effective base and size of AGP memory, and does not
    necessarily reflect the actual HW settings, particularly when certain AGP
    verifier options are in effect

Arguments:

    Extension - Master AGP Extension

    AgpBase - Returns the effective AGP base address

    AgpSize - Returns the maximum size for AGP memory allocations

Return Value:

    NTSTATUS

--*/
{
    PTARGET_EXTENSION Target = Extension->Target;
    PHYSICAL_ADDRESS BaseAddr = { 0, 0 };
    PVOID MapHandle = NULL;
    ULONG MaxPages = MIN_AGP_PAGES;
    NTSTATUS Status;

    PAGED_CODE();
    
    if ((Target->AgpSize != 0) &&
        (Target->AgpBase.QuadPart != (ULONGLONG)-1)) {
        *AgpBase = Target->AgpBase;
        *AgpSize = (SIZE_T)Target->AgpSize;
        
        AGPLOG(AGP_NOISE, ("AgpInterfaceGetAgpSize: GartBase=%I64X, Size="
                           "%x\n", AgpBase->QuadPart,
                           *AgpSize));
        
        return STATUS_SUCCESS;
    }

    Target->AgpSize = 0;
    Target->AgpBase.QuadPart = (ULONGLONG)-1;

    AgpBase->QuadPart = (ULONGLONG)-1;
    *AgpSize = 0;

    Status = STATUS_SUCCESS;

    while (NT_SUCCESS(Status)) {

        Status = AgpInterfaceReserveMemory(Extension,
                                           MaxPages,
                                           MmWriteCombined,
                                           &MapHandle,
                                           &BaseAddr);
        
        if (NT_SUCCESS(Status)) {
            Target->AgpBase = BaseAddr;
            Target->AgpSize = MaxPages * PAGE_SIZE;
            *AgpBase = Target->AgpBase;
            *AgpSize = (SIZE_T)Target->AgpSize;
            AgpInterfaceReleaseMemory(Extension, MapHandle);
        }

        MaxPages *= 2;
    }
    
    if ((Target->AgpSize != 0) &&
        (Target->AgpBase.QuadPart != (ULONGLONG)-1)) {
        AGPLOG(AGP_NOISE, ("AgpInterfaceGetAgpSize: GartBase=%I64X, Size="
                           "%x\n", AgpBase->QuadPart,
                           *AgpSize));
        
        return STATUS_SUCCESS;
    }

    return Status;
}




