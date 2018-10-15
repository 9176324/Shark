/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    Mca.c

Abstract:

    Machine Check Architecture interface

--*/

#pragma warning(disable:4206)   // translation unit empty

#include "wmikmp.h"
#include <mce.h>
#include "hal.h"
#include "ntiologc.h"
#include "mcaevent.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,WmipConstructMCAErrorEvent)
#endif

GUID WmipMSMCAEvent_InvalidErrGuid = MSMCAEvent_InvalidErrorGuid;
GUID WmipMSMCAEvent_MemoryHierarchyErrorGuid = MSMCAEvent_MemoryHierarchyErrorGuid;
GUID WmipMSMCAEvent_TLBErrorGuid = MSMCAEvent_TLBErrorGuid;
GUID WmipMSMCAEvent_BusErrorGuid = MSMCAEvent_BusErrorGuid;

#define MCA_CODE_MICROCODE_ROM_PARITY_ERROR  0x0002
#define MCA_CODE_EXTERNAL_ERROR              0x0003
#define MCA_CODE_FRC_ERROR                   0x0004
#define MCA_CODE_INTERNAL_TIMER_ERROR        0x0400

#define MCA_CODE_TLB_ERR_MASK                0x0010
#define MCA_CODE_MEM_HIERARCHY_ERR_MASK      0x0100
#define MCA_CODE_BUS_ERR_MASK                0x0800

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

NTSTATUS
WmipConstructMCAErrorEvent(
    IN PMCA_EXCEPTION McaException,
    IN ULONG ErrorLogSize,
    IN OUT PWNODE_SINGLE_INSTANCE Wnode,
    IN OUT PMSMCAEvent_Header Header,
    IN OUT PUCHAR *RawPtr,
    IN OUT BOOLEAN *IsFatal
    )
/*++

Routine Description:

    This routine attempts to build an MCA error log event from the information
    in the supplied MCA_EXCEPTION.

Arguments:

    McaException - Pointer to a MCA_EXCEPTION that encapsulates the machine
                   check event.
    
    ErrorLogSize - The total size in bytes of the raw error information.

    Wnode        - Pointer to the error event WNODE.

    Header       - Pointer to the common header for all MCA events.

    RawPtr       - Address into which a pointer to the raw MCA data is to
                   be copied.

Return Value:

    STATUS_SUCCESS - If an event is successfully constructed.
    
    STATUS_NOT_SUPPORTED - If the routine does not create an event.

--*/
{
    PMSMCAEvent_MemoryHierarchyError memoryEvent;
    PMSMCAEvent_InvalidError invalidEvent;
    PMSMCAEvent_TLBError tlbEvent;
    PMSMCAEvent_BusError busEvent;
    NTSTATUS status;
    ULONG mcaCode;
    
    PAGED_CODE();

    if (*IsFatal) {
        if (McaException->ExceptionType == HAL_MCA_RECORD) {
            if (!McaException->u.Mca.Status.MciStats.UnCorrected) {
                *IsFatal = FALSE;
            }
        }
    }

    //
    // Sanity checks on the size of the event structures. None should be
    // larger than the size of a memory error event.
    //
    WmipAssert(sizeof(MSMCAEvent_MemoryError) >=
               sizeof(MSMCAEvent_InvalidError));
    WmipAssert(sizeof(MSMCAEvent_MemoryError) >=
               sizeof(MSMCAEvent_MemoryHierarchyError));
    WmipAssert(sizeof(MSMCAEvent_MemoryError) >=
               sizeof(MSMCAEvent_TLBError));
    WmipAssert(sizeof(MSMCAEvent_MemoryError) >=
               sizeof(MSMCAEvent_BusError));

    //
    // This routine is only to be called whe the MCA status bits are valid.
    //
    WmipAssert(McaException->u.Mca.Status.MciStats.Valid);
    mcaCode = McaException->u.Mca.Status.MciStats.McaCod;
    status = STATUS_SUCCESS;

    if (mcaCode == MCA_CODE_MICROCODE_ROM_PARITY_ERROR ||
        mcaCode == MCA_CODE_EXTERNAL_ERROR ||
        mcaCode == MCA_CODE_FRC_ERROR ||
        mcaCode == MCA_CODE_INTERNAL_TIMER_ERROR) {

        //
        // For Microcode ROM parity errors, external errors, FRC errors, and 
        // internal timer errors, we will use the InvalidError event since it
        // is useful for generating error events with no parameters.
        //
        invalidEvent = (PMSMCAEvent_InvalidError)Header;

        //
        // Indicate the error type.
        //
        if (mcaCode == MCA_CODE_MICROCODE_ROM_PARITY_ERROR) {
            invalidEvent->Type = (ULONG)MCA_MICROCODE_ROM_PARITY_ERROR;
        } else if (mcaCode == MCA_CODE_EXTERNAL_ERROR) {
            invalidEvent->Type = (ULONG)MCA_EXTERNAL_ERROR;
        } else if (mcaCode == MCA_CODE_FRC_ERROR) {
            invalidEvent->Type = (ULONG)MCA_FRC_ERROR;
        } else {
            invalidEvent->Type = (ULONG)MCA_INTERNALTIMER_ERROR;
        }

        //
        // Fill in the GUID and the size of the data block.
        //
        Wnode->WnodeHeader.Guid = WmipMSMCAEvent_InvalidErrGuid;
        Wnode->SizeDataBlock = 
            FIELD_OFFSET(MSMCAEvent_InvalidError, RawRecord) + ErrorLogSize;

        //
        // Copy the address of the raw record into the supplied address.
        //
        *RawPtr = invalidEvent->RawRecord;

        //
        // Indicate the size of the entire record.
        //
        invalidEvent->Size = ErrorLogSize;

    } else if (mcaCode & MCA_CODE_BUS_ERR_MASK) {

        //
        // Bus/Interconnect error. Extract the details of the error information
        // from the preserved MCI_STATS and save it in the bus error event.
        //

        busEvent = (PMSMCAEvent_BusError)Header;

        if (mcaCode & 0x0100) {
            busEvent->Type = (ULONG)MCA_BUS_TIMEOUT_ERROR;
        } else {
            busEvent->Type = (ULONG)MCA_BUS_ERROR;
        }

        busEvent->Participation = ((mcaCode & 0x00000600) >> 9);
        busEvent->MemoryHierarchyLevel = (mcaCode & 0x00000003);
        busEvent->RequestType = ((mcaCode & 0x000000F0) >> 4);
        busEvent->MemOrIo = ((mcaCode & 0x0000000C) >> 2);

        if (McaException->u.Mca.Status.MciStats.AddressValid) {
            busEvent->Address = McaException->u.Mca.Address.QuadPart;
        } else {
            busEvent->Address = (ULONG64)0; 
        }

        Wnode->WnodeHeader.Guid = WmipMSMCAEvent_BusErrorGuid;
        Wnode->SizeDataBlock = FIELD_OFFSET(MSMCAEvent_BusError,
                                            RawRecord) + ErrorLogSize;

        busEvent->Size = ErrorLogSize;
        *RawPtr = busEvent->RawRecord;

    } else if (mcaCode & MCA_CODE_MEM_HIERARCHY_ERR_MASK) {

        //
        // Memory hierarchy error.
        //

        memoryEvent = (PMSMCAEvent_MemoryHierarchyError)Header;

        memoryEvent->Type = (ULONG)MCA_MEMORYHIERARCHY_ERROR;
        memoryEvent->TransactionType = ((mcaCode & 0x0000000C) >> 2);
        memoryEvent->MemoryHierarchyLevel = (mcaCode & 0x00000003);
        memoryEvent->RequestType = ((mcaCode & 0x000000F0) >> 4);

        if (McaException->u.Mca.Status.MciStats.AddressValid) {
            memoryEvent->Address = McaException->u.Mca.Address.QuadPart;
        } else {
            memoryEvent->Address = (ULONG64)0; 
        }

        Wnode->WnodeHeader.Guid = WmipMSMCAEvent_MemoryHierarchyErrorGuid;
        Wnode->SizeDataBlock = FIELD_OFFSET(MSMCAEvent_MemoryHierarchyError,
                                            RawRecord) + ErrorLogSize;

        memoryEvent->Size = ErrorLogSize;
        *RawPtr = memoryEvent->RawRecord;

    } else if (mcaCode & MCA_CODE_TLB_ERR_MASK) {

        //
        // TLB error.
        //

        tlbEvent = (PMSMCAEvent_TLBError)Header;

        tlbEvent->Type = (ULONG)MCA_TLB_ERROR;
        tlbEvent->TransactionType = ((mcaCode & 0x0000000C) >> 2);
        tlbEvent->MemoryHierarchyLevel = (mcaCode & 0x00000003);

        if (McaException->u.Mca.Status.MciStats.AddressValid) {
            tlbEvent->Address = McaException->u.Mca.Address.QuadPart;
        } else {
            tlbEvent->Address = (ULONG64)0; 
        }

        Wnode->WnodeHeader.Guid = WmipMSMCAEvent_TLBErrorGuid;
        Wnode->SizeDataBlock = FIELD_OFFSET(MSMCAEvent_TLBError,
                                            RawRecord) + ErrorLogSize;

        tlbEvent->Size = ErrorLogSize;
        *RawPtr = tlbEvent->RawRecord;

    } else {

        //
        // No error, unclassified errors, and internal unclassified errors are
        // treated the same way... we ignore them.
        //
        status = STATUS_NOT_SUPPORTED;
    }

    return status;
}

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

