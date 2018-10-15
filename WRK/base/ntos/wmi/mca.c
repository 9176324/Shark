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
#define MCA_EVENT_INSTANCE_NAME L"McaEvent"
#define MCA_UNDEFINED_CPU 0xffffffff


#if defined(_X86_) || defined(_AMD64_)
#define HalpGetFwMceLogProcessorNumber( /* PMCA_EXCEPTION */ _Log ) \
    ( (_Log)->ProcessorNumber )
typedef MCA_EXCEPTION ERROR_LOGRECORD, *PERROR_LOGRECORD;
typedef MCA_EXCEPTION ERROR_RECORD_HEADER, *PERROR_RECORD_HEADER;
#endif

BOOLEAN WmipMceEventDelivery(
    IN PVOID Reserved,
    IN KERNEL_MCE_DELIVERY_OPERATION Operation,
    IN PVOID Argument2
    );

BOOLEAN WmipMceDelivery(
    IN PVOID Reserved,
    IN KERNEL_MCE_DELIVERY_OPERATION Operation,
    IN PVOID Argument2
    );

void WmipMceWorkerRoutine(    
    IN PVOID Context             // Not Used
    );

NTSTATUS WmipGetLogFromHal(
    HAL_QUERY_INFORMATION_CLASS InfoClass,
    PVOID Token,
    PWNODE_SINGLE_INSTANCE *Wnode,
    PERROR_LOGRECORD *Mca,
    PULONG McaSize,
    ULONG MaxSize,
    LPGUID Guid
    );

NTSTATUS WmipRegisterMcaHandler(
    ULONG Phase
    );

NTSTATUS WmipBuildMcaCmcEvent(
    OUT PWNODE_SINGLE_INSTANCE Wnode,
    IN LPGUID EventGuid,
    IN PERROR_LOGRECORD McaCmcEvent,
    IN ULONG McaCmcSize
    );

NTSTATUS WmipGetRawMCAInfo(
    OUT PUCHAR Buffer,
    IN OUT PULONG BufferSize
    );

NTSTATUS WmipWriteMCAEventLogEvent(
    PUCHAR Event
    );

NTSTATUS WmipSetupWaitForWbem(
    void
    );

void WmipIsWbemRunningDispatch(    
    IN PKDPC Dpc,
    IN PVOID DeferredContext,     // Not Used
    IN PVOID SystemArgument1,     // Not Used
    IN PVOID SystemArgument2      // Not Used
    );

void WmipPollingDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,     // MCEQUERYINFO
    IN PVOID SystemArgument1,     // New polling interval
    IN PVOID SystemArgument2      // Not used
    );

void WmipIsWbemRunningWorker(
    PVOID Context
    );

BOOLEAN WmipCheckIsWbemRunning(
    void
    );

void WmipProcessPrevMcaLogs(
    void
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,WmipRegisterMcaHandler)
#pragma alloc_text(PAGE,WmipMceWorkerRoutine)
#pragma alloc_text(PAGE,WmipGetLogFromHal)
#pragma alloc_text(PAGE,WmipBuildMcaCmcEvent)
#pragma alloc_text(PAGE,WmipGetRawMCAInfo)
#pragma alloc_text(PAGE,WmipWriteMCAEventLogEvent)
#pragma alloc_text(PAGE,WmipGenerateMCAEventlog)
#pragma alloc_text(PAGE,WmipIsWbemRunningWorker)
#pragma alloc_text(PAGE,WmipCheckIsWbemRunning)
#pragma alloc_text(PAGE,WmipSetupWaitForWbem)
#pragma alloc_text(PAGE,WmipProcessPrevMcaLogs)
#endif

//
// Set to TRUE when the registry indicates that popups should be
// disabled. HKLM\System\CurrentControlSet\Control\WMI\DisableMCAPopups
//
ULONG WmipDisableMCAPopups;

GUID WmipMSMCAEvent_InvalidErrorGuid = MSMCAEvent_InvalidErrorGuid;

//
// Each type of MCE has a control structure that is used to determine
// whether to poll or wait for an interrupt to determine when to query
// for the logs.  This is needed since we can get a callback from the
// HAL at high IRQL to inform us that a MCE log is available.
// Additionally Ke Timer used for polling will calls us at DPC level.
// So in the case of an interrupt we will queue a DPC. Within the DPC
// routine we will queue a work item so that we can get back to
// passive level and be able to call the hal to get the logs (Can only
// call hal at passive). The DPC and work item routines are common so a
// MCEQUERYINFO struct is passed around so that it can operate on the
// correct log type. Note that this implies that there may be multiple
// work items querying the hal for different log types at the same
// time. In addition this struct also contains useful log related
// information including the maximum log size (as reported by the HAL),
// the token that must be passed to the HAL when querying for the
// logs and the HAL InfoClass to use when querying for the logs.
//
// PollFrequency keeps track of the number of seconds before initiating a
// query. If it is 0 (HAL_CPE_DISABLED / HAL_CMC_DISABLED) then no
// polling occurs and if it is -1 (HAL_CPE_INTERRUPTS_BASED /
// HAL_CMC_INTERRUPTS_BASED) then no polling occurs either. There is
// only one work item active for each log type and this is enforced via
// ItemsOutstanding in that only whenever it transitions from 0 to 1 is
// the work item queued.
//
#define DEFAULT_MAX_MCA_SIZE 0x1000
#define DEFAULT_MAX_CMC_SIZE 0x1000
#define DEFAULT_MAX_CPE_SIZE 0x1000

typedef struct
{
    HAL_QUERY_INFORMATION_CLASS InfoClass;  // HAL Info class to use in MCE query
    ULONG PollFrequency;                    // Polling Frequency in seconds
    PVOID Token;                            // HAL Token to use in MCE Queries
    LONG ItemsOutstanding;                  // Number of interrupts or poll requests to process
    ULONG MaxSize;                          // Max size for log (as reported by HAL)
    GUID WnodeGuid;                         // GUID to use for the raw data event
    GUID SwitchToPollGuid;                  // GUID to use to fire event for switching to polled mode
    NTSTATUS SwitchToPollErrorCode;         // Eventlog error code that indicates a switch to polled mode
    ULONG WorkerInProgress;                 // Set to 1 if worker routine is running
    KSPIN_LOCK DpcLock;
    KDPC DeliveryDpc;                       // DPC to handle delivery
    KTIMER PollingTimer;                    // KTIMER used for polling
    KDPC PollingDpc;                        // DPC to use for polling
    WORK_QUEUE_ITEM WorkItem;               // Work item used to query for log
} MCEQUERYINFO, *PMCEQUERYINFO;

MCEQUERYINFO WmipMcaQueryInfo =
{
    HalMcaLogInformation,
    HAL_MCA_INTERRUPTS_BASED,               // Corrected MCA are delivered by interrupts
    NULL,
    0,
    DEFAULT_MAX_MCA_SIZE,
    MSMCAInfo_RawMCAEventGuid
};

MCEQUERYINFO WmipCmcQueryInfo =
{
    HalCmcLogInformation,
    HAL_CMC_DISABLED,
    NULL,
    0,
    DEFAULT_MAX_CMC_SIZE,
    MSMCAInfo_RawCMCEventGuid,
    MSMCAEvent_SwitchToCMCPollingGuid,
    MCA_WARNING_CMC_THRESHOLD_EXCEEDED,
    0
};
                               
MCEQUERYINFO WmipCpeQueryInfo =
{
    HalCpeLogInformation,
    HAL_CPE_DISABLED,
    NULL,
    0,
    DEFAULT_MAX_CPE_SIZE,
    MSMCAInfo_RawCorrectedPlatformEventGuid,
    MSMCAEvent_SwitchToCPEPollingGuid,
    MCA_WARNING_CPE_THRESHOLD_EXCEEDED,
    0
};

//
// Used for waiting until WBEM is ready to receive events
//
KTIMER WmipIsWbemRunningTimer;
KDPC WmipIsWbemRunningDpc;
WORK_QUEUE_ITEM WmipIsWbemRunningWorkItem;
LIST_ENTRY WmipWaitingMCAEvents = {&WmipWaitingMCAEvents, &WmipWaitingMCAEvents};

#define WBEM_STATUS_UNKNOWN 0   // Polling process for waiting is not started
#define WBEM_IS_RUNNING 1       // WBEM is currently running
#define WAITING_FOR_WBEM  2     // Polling process for waiting is started
UCHAR WmipIsWbemRunningFlag;


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

//
// MCA information obtained at boot and holds the MCA that caused the
// system to bugcheck on the previous boot
//
ULONG WmipRawMCASize;
PMSMCAInfo_RawMCAData WmipRawMCA;

//
// Status of the MCE registration process
//
#define MCE_STATE_UNINIT     0
#define MCE_STATE_REGISTERED 1
#define MCE_STATE_RUNNING    2
#define MCE_STATE_ERROR      -1
ULONG WmipMCEState;

//
// Configurable parameters for managing thresholds for eventlog
// suppression and recovery action for corrected MCE
//

//
// Interval within which multiple identical errors will be reported as
// a single error to the system eventlog. Can be configured under
// HKLM\System\CurrentControlSet\Control\WMI\CoalesceCorrectedErrorInterval
// A value of 0 will cause no coalesce of identical errors
//
ULONG WmipCoalesceCorrectedErrorInterval = 5000;

//
// Number of single bit ecc errors that can occur in the same page
// before it is attempted to map out the page. Can be configured under : 
// HKLM\System\CurrentControlSet\Control\WMI\SingleBitEccErrorThreshold
// A value of 0 will cause no attempt to map out pages
//
ULONG WmipSingleBitEccErrorThreshold = 6;

//
// Maxiumum number of MCE events being tracked at one time. If there is
// more than this limit then the oldest ones are recycled. Can be
// configured under :
// HKLM\System\CurrentControlSet\Control\WMI\MaxCorrectedMCEOutstanding
// A value of 0 will disable tracking of corrected errors
//
ULONG WmipMaxCorrectedMCEOutstanding = 5;

//
// List of corrected MCE that are being tracked
//
LIST_ENTRY WmipCorrectedMCEHead = {&WmipCorrectedMCEHead, &WmipCorrectedMCEHead};
ULONG WmipCorrectedMCECount;

//
// Counter of maximum eventlog entries generated by any source. Can be
// configured under:
// HKLM\System\CurrentControlSet\Control\WMI\MaxCorrectedEventlogs
//
ULONG WmipCorrectedEventlogCounter = 20;

//
// Check if WBEM is already running and if not check if we've already
// kicked off the timer that will wait for wbem to start
//
#define WmipIsWbemRunning() ((WmipIsWbemRunningFlag == WBEM_IS_RUNNING) ? \
                                                       TRUE : \
                                                       FALSE)
void WmipInsertQueueMCEDpc(
    PMCEQUERYINFO QueryInfo
    );



NTSTATUS WmipWriteToEventlog(
    NTSTATUS ErrorCode,
    NTSTATUS FinalStatus
    )
{
    PIO_ERROR_LOG_PACKET ErrLog;
    NTSTATUS Status;

    ErrLog = IoAllocateErrorLogEntry(WmipServiceDeviceObject,
                                     sizeof(IO_ERROR_LOG_PACKET));

    if (ErrLog != NULL) {

        //
        // Fill it in and write it out as a single string.
        //
        ErrLog->ErrorCode = ErrorCode;
        ErrLog->FinalStatus = FinalStatus;

        ErrLog->StringOffset = 0;
        ErrLog->NumberOfStrings = 0;

        IoWriteErrorLogEntry(ErrLog);
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    
    return(Status);
}

NTSTATUS WmipFireOffWmiEvent(
    LPGUID Guid,
    ULONG DataSize,
    PVOID DataPtr            
    )
{
    PVOID Ptr;
    PWNODE_SINGLE_INSTANCE Wnode;
    PWCHAR Wptr;
    ULONG RoundedDataSize;
    NTSTATUS Status;

    RoundedDataSize = (DataSize + 1) & ~1;
    
    Wnode = ExAllocatePoolWithTag(NonPagedPool,
                                  sizeof(WNODE_SINGLE_INSTANCE) +
                                       RoundedDataSize +
                                      sizeof(USHORT) +
                                       sizeof(MCA_EVENT_INSTANCE_NAME),
                                  WmipMCAPoolTag);

    if (Wnode != NULL)
    {
        Wnode->WnodeHeader.BufferSize = sizeof(WNODE_SINGLE_INSTANCE) +
                                       sizeof(USHORT) +
                                        RoundedDataSize +
                                       sizeof(MCA_EVENT_INSTANCE_NAME);
        Wnode->WnodeHeader.Guid = *Guid;

        Wnode->WnodeHeader.Flags = WNODE_FLAG_SINGLE_INSTANCE |
                                   WNODE_FLAG_EVENT_ITEM;
        KeQuerySystemTime(&Wnode->WnodeHeader.TimeStamp);

        Wnode->DataBlockOffset = sizeof(WNODE_SINGLE_INSTANCE);
        Wnode->SizeDataBlock = DataSize;
        if (DataPtr != NULL)
        {
            Ptr = OffsetToPtr(Wnode, Wnode->DataBlockOffset);
            memcpy(Ptr, DataPtr, DataSize);
        }
        Wnode->OffsetInstanceName = sizeof(WNODE_SINGLE_INSTANCE) + RoundedDataSize;

        Wptr = (PWCHAR)OffsetToPtr(Wnode, Wnode->OffsetInstanceName);
        *Wptr++ = sizeof(MCA_EVENT_INSTANCE_NAME);
        RtlCopyMemory(Wptr,
                      MCA_EVENT_INSTANCE_NAME,
                      sizeof(MCA_EVENT_INSTANCE_NAME));

        Status = IoWMIWriteEvent(Wnode);
        if (! NT_SUCCESS(Status))
        {
            ExFreePool(Wnode);
        }
    }
    else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return(Status);
}

NTSTATUS WmipBuildMcaCmcEvent(
    OUT PWNODE_SINGLE_INSTANCE Wnode,
    IN LPGUID EventGuid,
    IN PERROR_LOGRECORD McaCmcEvent,
    IN ULONG McaCmcSize
    )
/*++

Routine Description:


    This routine will take a MCA or CMC log and build a
    WNODE_EVENT_ITEM for it.

    This routine may be called at DPC

Arguments:

    Wnode is the wnode buffer in which to build the event

    EventGuid is the guid to use in the event wnode

    McaCmcEvent is the MCA, CMC or CPE data payload to put into the
            event

    McaCmcSize is the size of the event data


Return Value:

    NT status code

--*/
{
    PMSMCAInfo_RawCMCEvent Ptr;
    ULONG Size;

    PAGED_CODE();

    Size = McaCmcSize + FIELD_OFFSET(MSMCAInfo_RawCMCEvent,
                                                     Records) +
                                        FIELD_OFFSET(MSMCAInfo_Entry, Data);
    
    RtlZeroMemory(Wnode, sizeof(WNODE_SINGLE_INSTANCE));
    Wnode->WnodeHeader.BufferSize = Size + sizeof(WNODE_SINGLE_INSTANCE);
    Wnode->WnodeHeader.ProviderId = IoWMIDeviceObjectToProviderId(WmipServiceDeviceObject);
    KeQuerySystemTime(&Wnode->WnodeHeader.TimeStamp);       
    Wnode->WnodeHeader.Guid = *EventGuid;
    Wnode->WnodeHeader.Flags = WNODE_FLAG_SINGLE_INSTANCE |
                               WNODE_FLAG_EVENT_ITEM |
                               WNODE_FLAG_STATIC_INSTANCE_NAMES;
    Wnode->DataBlockOffset = FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                          VariableData);
    Wnode->SizeDataBlock = Size;
    Ptr = (PMSMCAInfo_RawCMCEvent)&Wnode->VariableData;
    Ptr->Count = 1;                           // 1 Record in this event
    Ptr->Records[0].Length = McaCmcSize;       // Size of log record in bytes
    if (McaCmcEvent != NULL)
    {
        RtlCopyMemory(Ptr->Records[0].Data, McaCmcEvent, McaCmcSize);
    }
    
    return(STATUS_SUCCESS);
}

NTSTATUS WmipQueryLogAndFireEvent(
    PMCEQUERYINFO QueryInfo
    )
/*++

Routine Description:

    Utility routine that will query the hal for a log and then if one
    is returned successfully then will fire the appropriate WMI events 

Arguments:

    QueryInfo is a pointer to the MCEQUERYINFO for the type of log that
    needs to be queried.

Return Value:

--*/
{
    PWNODE_SINGLE_INSTANCE Wnode;
    NTSTATUS Status, Status2;
    ULONG Size;
    PERROR_LOGRECORD Log;   

    PAGED_CODE();

    //
    // Call HAL to get the log
    //
    Status = WmipGetLogFromHal(QueryInfo->InfoClass,
                               QueryInfo->Token,
                               &Wnode,
                               &Log,
                               &Size,
                               QueryInfo->MaxSize,
                               &QueryInfo->WnodeGuid);

    if (NT_SUCCESS(Status))
    {
        //
        // Look at the event and fire it off as WMI events that
        // will generate eventlog events
        //
        WmipGenerateMCAEventlog((PUCHAR)Log,
                                Size,
                                FALSE);

        //
        // Fire the log off as a WMI event
        //
        Status2 = IoWMIWriteEvent(Wnode);
        if (! NT_SUCCESS(Status2))
        {
            //
            // IoWMIWriteEvent will free the wnode back to pool,
            // but not if it fails
            //
            ExFreePool(Wnode);
        }
        
        WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                          DPFLTR_MCA_LEVEL,
                          "WMI: MCE Event fired to WMI -> %x\n",
                          Status));
        
    } else {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                          DPFLTR_MCA_LEVEL,
                          "WMI: MCE Event for %p not available %x\n",
                          QueryInfo, Status));
    }
    return(Status);
}

void WmipMceWorkerRoutine(    
    IN PVOID Context             // MCEQUERYINFO
    )
/*++

Routine Description:

    Worker routine that handles polling for corrected MCA, CMC and CPE
    logs from the HAL and then firing them as WMI events.

Arguments:

    Context is a pointer to the MCEQUERYINFO for the type of log that
    needs to be queried.

Return Value:

--*/
{
    PMCEQUERYINFO QueryInfo = (PMCEQUERYINFO)Context;
    NTSTATUS Status;
    ULONG i;
    LONG x, Count;

    PAGED_CODE();

    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                      "WMI: WmipMceWorkerRoutine %p enter\n",
                     QueryInfo));

    //
    // If the worker is already in progress then we just exit
    //
    WmipEnterSMCritSection();
    if (QueryInfo->WorkerInProgress == 0)
    {
        QueryInfo->WorkerInProgress = 1;
        WmipLeaveSMCritSection();
    } else {
        WmipLeaveSMCritSection();
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                          "WMI: WmipMceWorkerRoutine %p in progress\n",
                         QueryInfo));
        return;
    }

    //
    // Check to see if access has already been disabled
    //
    if (QueryInfo->PollFrequency != HAL_MCE_DISABLED)
    {
        //
        // We get all of the records by calling into the hal and querying
        // for the logs until the hal returns an error or we've
        // retrieved 256 records. We want to protect ourselves from the
        // case where a repeated corrected error would cause the loop
        // to be infinite.
        //
        i = 0;
        do
        {
            //
            // Remember how many corrected errors we have received up until
            // this point. We guarantee that we've handled them up
            // until this point
            //
            Count = QueryInfo->ItemsOutstanding;

            Status = WmipQueryLogAndFireEvent(QueryInfo);           
        } while ((NT_SUCCESS(Status) && (i++ < 256)));

        //
        // Reset counter back to 0, but check if any errors
        // had occured while we were processing. If so we go
        // back and make sure they are handled. Note that this
        // could cause a new worker thread to be created while we
        // are still processing these, but that is ok since we only
        // allow one worker thread to run at one time.
        //
        WmipEnterSMCritSection();
        x = InterlockedExchange(&QueryInfo->ItemsOutstanding,
                                0);
        if ((x > Count) && (i < 257))
        {
            //
            // Since there are still more corrected errors to
            // process, queue a new DPC to cause a new worker
            // routine to be run.
            //
            WmipInsertQueueMCEDpc(QueryInfo);
        }

        QueryInfo->WorkerInProgress = 0;
        WmipLeaveSMCritSection();
    }
}

void WmipMceDispatchRoutine(
    PMCEQUERYINFO QueryInfo
    )
{

    ULONG x;

    //
    // Increment the number of items that are outstanding for this info
    // class. If the number of items outstanding transitions from 0 to
    // 1 then this implies that a work item for this info class needs
    // to be queued
    //
    x = InterlockedIncrement(&QueryInfo->ItemsOutstanding);

    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                      "WMI: WmipMceDispatchRoutine %p transition to %d\n",
                     QueryInfo,
                     x));

    if (x == 1)
    {
        ExQueueWorkItem(&QueryInfo->WorkItem,
                        DelayedWorkQueue);
    }
}

void WmipMceDpcRoutine(    
    IN PKDPC Dpc,
    IN PVOID DeferredContext,     // Not Used
    IN PVOID SystemArgument1,     // MCEQUERYINFO
    IN PVOID SystemArgument2      // Not used
    )
{
    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument2);

    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                      "WMI: WmipMceDpcRoutine %p Enter\n",
                     SystemArgument1));
    
    WmipMceDispatchRoutine((PMCEQUERYINFO)SystemArgument1);
}


void WmipPollingDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,     // MCEQUERYINFO
    IN PVOID SystemArgument1,     // New polling Interval
    IN PVOID SystemArgument2      // Not used
    )
{
    PMCEQUERYINFO QueryInfo = (PMCEQUERYINFO)DeferredContext;
    LARGE_INTEGER li;
    ULONG PollingInterval = PtrToUlong(SystemArgument1);

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (SystemArgument2);

    if (QueryInfo->PollFrequency == HAL_MCE_INTERRUPTS_BASED)
    {
        //
        // HAL has instructed us to switch into polled mode and has
        // informed us of the new polling interval.
        //

        QueryInfo->PollFrequency = PollingInterval;

        li.QuadPart = -1 * (QueryInfo->PollFrequency * 1000000000);
        KeSetTimerEx(&QueryInfo->PollingTimer,
                     li,
                     QueryInfo->PollFrequency * 1000,
                     &QueryInfo->PollingDpc);

        //
        // Make a note in the eventlog that this has occured. 
        //
        WmipWriteToEventlog(QueryInfo->SwitchToPollErrorCode,
                            STATUS_SUCCESS
                           );
        
        //
        // Inform any WMI consumers that the switch has occured
        //
        WmipFireOffWmiEvent(&QueryInfo->SwitchToPollGuid,
                           0,
                           NULL);
    } else {
        //
        // Our timer fired so we need to poll
        //
        WmipMceDispatchRoutine(QueryInfo);
    }
}

BOOLEAN WmipMceDelivery(
    IN PVOID Reserved,
    IN KERNEL_MCE_DELIVERY_OPERATION Operation,
    IN PVOID Argument2
    )
/*++

Routine Description:


    This routine is called by the HAL when a CMC or CPE occurs. It is called
    at high irql

Arguments:

    Operation is the operation that the HAL is instructing us to do

    Reserved is the CMC token

    Parameter for operation specified.
        For CmcSwitchToPolledMode and CpeSwitchToPolledMode, Parameter
        specifies the number of seconds to between polling.


Return Value:

    TRUE to indicate that we handled the delivery

--*/
{
    PMCEQUERYINFO QueryInfo;
    BOOLEAN ret;
    
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                      "WMI: MceDelivery Operation %d(%p)\n",
                     Operation, Argument2));

    //
    // First figure out which type of MCE we are dealing with
    //
    switch (Operation)
    {
        case CmcAvailable:
        case CmcSwitchToPolledMode:
        {
            QueryInfo = &WmipCmcQueryInfo;
            break;
        }
        
        case CpeAvailable:
        case CpeSwitchToPolledMode:
        {
            QueryInfo = &WmipCpeQueryInfo;
            break;
        }

        case McaAvailable:
        {
            QueryInfo = &WmipMcaQueryInfo;
            break;
        }
        
        default:
        {
            WmipAssert(FALSE);
            return(FALSE);
        }
    }


    //
    // Next determine what action to perform
    //
    switch (Operation)
    {
        case CmcAvailable:
        case CpeAvailable:
        case McaAvailable:
        {
            //
            // Store the HAL token which is needed to retrieve the logs from
            // the hal
            //
            QueryInfo->Token = Reserved;

            //
            // If we are ready to handle the logs and we are dealing with thse
            // logs  on an interrupt basis, then go ahead and queue a DPC to handle
            // processing the log
            //
            if ((WmipMCEState == MCE_STATE_RUNNING) &&
                (QueryInfo->PollFrequency == HAL_MCE_INTERRUPTS_BASED))

            {
                KeAcquireSpinLockAtDpcLevel(&QueryInfo->DpcLock);
                KeInsertQueueDpc(&QueryInfo->DeliveryDpc,
                                 QueryInfo,
                                 NULL);
                KeReleaseSpinLockFromDpcLevel(&QueryInfo->DpcLock);
                ret = TRUE;
            } else {
                ret = FALSE;
            }
            break;
        }

        case CmcSwitchToPolledMode:
        case CpeSwitchToPolledMode:
        {
            KeInsertQueueDpc(&QueryInfo->PollingDpc,
                             Argument2,
                             NULL);
            ret = TRUE;
            break;
        }
        default:
        {
            ret = FALSE;
            break;
        }
    }

    return(ret);
}

BOOLEAN WmipMceEventDelivery(
    IN PVOID Reserved,
    IN KERNEL_MCE_DELIVERY_OPERATION Operation,
    IN PVOID Argument2
    )
/*++

Routine Description:


    This routine is called by the HAL when a situation occurs between
    the HAL and SAL interface. It is called at high irql

Arguments:

    Reserved has the Operation and EventType

    Argument2 has the SAL return code

Return Value:


--*/
{
    USHORT MceOperation;
    LONGLONG SalStatus;
    ULONG MceType;
    PMCEQUERYINFO QueryInfo;
    
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                      "WMI: MCEDelivery %p %d %p\n",
                      Reserved,
                      Operation,
                      Argument2
                     ));

    MceOperation = KERNEL_MCE_OPERATION(Reserved);
    MceType = KERNEL_MCE_EVENTTYPE(Reserved);
    SalStatus = (LONGLONG)Argument2;

    //
    // If the hal is notifying us that a GetStateInfo failed with
    // SalStatus == -15 then we need to retry our query later
    //
    if ((MceOperation == KERNEL_MCE_OPERATION_GET_STATE_INFO) &&
        (Operation == MceNotification) &&
        (SalStatus == (LONGLONG)-15))
    {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                          "WMI: Sal is asking us to retry getstateinfo for type %x\n",
                          MceType));

        switch(MceType)
        {
            case KERNEL_MCE_EVENTTYPE_CMC:
            {
                QueryInfo = &WmipCmcQueryInfo;              
                break;
            }

            case KERNEL_MCE_EVENTTYPE_CPE:
            {
                QueryInfo = &WmipCpeQueryInfo;
                break;
            }

            default:
            {
                QueryInfo = NULL;
            }
        }

        if (QueryInfo != NULL)
        {
            //
            // If CMC or CPE are interrupt based then queue up a new
            // DPC for performing the query. If polling based then
            // there are no worries, we just wait for the next polling
            // interval.
            //
            if ((WmipMCEState == MCE_STATE_RUNNING) &&
                (QueryInfo->PollFrequency == HAL_MCE_INTERRUPTS_BASED))

            {
                KeAcquireSpinLockAtDpcLevel(&QueryInfo->DpcLock);
                KeInsertQueueDpc(&QueryInfo->DeliveryDpc,
                                 QueryInfo,
                                 NULL);
                KeReleaseSpinLockFromDpcLevel(&QueryInfo->DpcLock);
            }
        }
        
    }
    
    return(FALSE);
    
}

void WmipProcessPrevMcaLogs(
    void
    )
/*++

Routine Description:

    This routine will flush out any of the previous MCA logs and then
    hang onto them for WMI to report.


Arguments:


Return Value:


--*/
{
    NTSTATUS status;
    PERROR_LOGRECORD log;
    PMSMCAInfo_RawMCAEvent event;
    ULONG size;
    PWNODE_SINGLE_INSTANCE wnode;
    LIST_ENTRY list;
    ULONG prevLogCount;
    PMSMCAInfo_Entry record;
    ULONG sizeNeeded;
    
    PAGED_CODE();

    InitializeListHead(&list);
    
    sizeNeeded = sizeof(ULONG);      // Need space for count of records
    prevLogCount = 0;
    do
    {
        //
        // Read a MCA log out of the HAL
        //
        status = WmipGetLogFromHal(HalMcaLogInformation,
                                   WmipMcaQueryInfo.Token,
                                   &wnode,
                                   &log,
                                   &size,
                                   WmipMcaQueryInfo.MaxSize,
                                   &WmipMcaQueryInfo.WnodeGuid);

        if (NT_SUCCESS(status))
        {
            //
            // Previous logs have a ErrorSeverity of Fatal since they
            // were fatal and brought down the system in last boot.
            // keep track of how much memory we will need           
            //
            prevLogCount++;
                                   // Need space for record length and
                                   // record padded to DWORD                                   
            sizeNeeded += sizeof(ULONG) + ((size +3)&~3);
            
            InsertTailList(&list, (PLIST_ENTRY)wnode);

            WmipGenerateMCAEventlog((PUCHAR)log,
                                    size,
                                    TRUE);                
        }
        
    } while (NT_SUCCESS(status));

    if (! IsListEmpty(&list))
    {
        //
        // We have collected a set of previous logs, so we need to
        // build the buffer containing the aggregation of those logs.
        // The buffer will correspond to the entire MOF structure for
        // the MSMCAInfo_RawMCAData class
        //
        WmipRawMCA = (PMSMCAInfo_RawMCAData)ExAllocatePoolWithTag(PagedPool,
                                                                  sizeNeeded,
                                                                  WmipMCAPoolTag);


        //
        // Fill in the count of logs that follow
        //
        if (WmipRawMCA != NULL)
        {
            WmipRawMCA->Count = prevLogCount;
        }

        //
        // Loop over all previous logs
        //
        WmipRawMCASize = sizeNeeded;
        record = &WmipRawMCA->Records[0];
        
        while (! IsListEmpty(&list))
        {           
            wnode = (PWNODE_SINGLE_INSTANCE)RemoveHeadList(&list);
            if (WmipRawMCA != NULL)
            {
                //
                // Get the log back from within the wnode
                //
                event = (PMSMCAInfo_RawMCAEvent)OffsetToPtr(wnode, wnode->DataBlockOffset);

                //
                // Copy the log data into our buffer. Note that we
                // assume there will only be 1 record within the event
                //
                size = event->Records[0].Length;
                record->Length = size;
                
                RtlCopyMemory(&record->Data[0], &event->Records[0].Data[0], size);
                
                size = FIELD_OFFSET(MSMCAInfo_Entry, Data) + (size +3)&~3;
                
                record = (PMSMCAInfo_Entry)((PUCHAR)record + size);
            }

            ExFreePool(wnode);
        }
    }
}

void WmipInsertQueueMCEDpc(
    PMCEQUERYINFO QueryInfo
    )
{
    KIRQL OldIrql;

    KeAcquireSpinLock(&QueryInfo->DpcLock,
                      &OldIrql);
    KeInsertQueueDpc(&QueryInfo->DeliveryDpc,
                     QueryInfo,
                     NULL);
    KeReleaseSpinLock(&QueryInfo->DpcLock,
                      OldIrql);
}

NTSTATUS WmipRegisterMcaHandler(
    ULONG Phase
    )
/*++

Routine Description:


    This routine will register a kernel MCA and CMC handler with the
    hal

Arguments:


Return Value:

    NT status code

--*/
{
    KERNEL_ERROR_HANDLER_INFO KernelMcaHandlerInfo;
    NTSTATUS Status;
    HAL_ERROR_INFO HalErrorInfo;
    ULONG ReturnSize;
    LARGE_INTEGER li;

    PAGED_CODE();

    if (Phase == 0)
    {
        //
        // Phase 0 initialization is done before device drivers are
        // loaded so that the kernel can register its kernel error
        // handler before any driver gets a chance to do so.
        //


        //
        // Validate registry values
        //
        if (WmipCorrectedEventlogCounter == 0)
        {
            //
            // set corrected eventlog counter to -1 to indicate that no
            // eventlog suppression should occur
            //
            WmipCorrectedEventlogCounter = 0xffffffff;
        }
        
        //
        // Get the size of the logs and any polling/interrupt policies
        //
        HalErrorInfo.Version = HAL_ERROR_INFO_VERSION;

        Status = HalQuerySystemInformation(HalErrorInformation,
                                           sizeof(HAL_ERROR_INFO),
                                           &HalErrorInfo,
                                           &ReturnSize);

        if ((NT_SUCCESS(Status)) &&
            (ReturnSize >= sizeof(HAL_ERROR_INFO)))
        {
            //
            // Initialize MCA QueryInfo structure
            //
            if (HalErrorInfo.McaMaxSize != 0)
            {
                WmipMcaQueryInfo.MaxSize = HalErrorInfo.McaMaxSize;
            }

            
            WmipMcaQueryInfo.Token = (PVOID)(ULONG_PTR) HalErrorInfo.McaKernelToken;

            //
            // Initialize DPC and Workitem for processing
            //
            KeInitializeDpc(&WmipMcaQueryInfo.DeliveryDpc,
                            WmipMceDpcRoutine,
                            NULL);

            KeInitializeDpc(&WmipMcaQueryInfo.PollingDpc,
                            WmipPollingDpcRoutine,
                            &WmipMcaQueryInfo);

            ExInitializeWorkItem(&WmipMcaQueryInfo.WorkItem,
                                 WmipMceWorkerRoutine,
                                 &WmipMcaQueryInfo);


            //
            // Initialize CMC QueryInfo structure
            //          
            if (HalErrorInfo.CmcMaxSize != 0)
            {
                WmipCmcQueryInfo.MaxSize = HalErrorInfo.CmcMaxSize;
            }
           
            WmipCmcQueryInfo.PollFrequency = HalErrorInfo.CmcPollingInterval;
            
            WmipCmcQueryInfo.Token = (PVOID)(ULONG_PTR) HalErrorInfo.CmcKernelToken;

            //
            // Initialize DPC and Workitem for processing
            //
            KeInitializeSpinLock(&WmipCmcQueryInfo.DpcLock);
            KeInitializeDpc(&WmipCmcQueryInfo.DeliveryDpc,
                            WmipMceDpcRoutine,
                            NULL);

            KeInitializeDpc(&WmipCmcQueryInfo.PollingDpc,
                            WmipPollingDpcRoutine,
                            &WmipCmcQueryInfo);

            ExInitializeWorkItem(&WmipCmcQueryInfo.WorkItem,
                                 WmipMceWorkerRoutine,
                                 &WmipCmcQueryInfo);

            KeInitializeTimerEx(&WmipCmcQueryInfo.PollingTimer,
                                NotificationTimer);

            //
            // Initialize CPE QueryInfo structure
            //          
            if (HalErrorInfo.CpeMaxSize != 0)
            {
                WmipCpeQueryInfo.MaxSize = HalErrorInfo.CpeMaxSize;
            }

            WmipCpeQueryInfo.PollFrequency = HalErrorInfo.CpePollingInterval;
            
            WmipCpeQueryInfo.Token = (PVOID)(ULONG_PTR) HalErrorInfo.CpeKernelToken;

            //
            // Initialize DPC and Workitem for processing
            //
            KeInitializeSpinLock(&WmipCpeQueryInfo.DpcLock);
            KeInitializeDpc(&WmipCpeQueryInfo.DeliveryDpc,
                            WmipMceDpcRoutine,
                            NULL);

            KeInitializeDpc(&WmipCpeQueryInfo.PollingDpc,
                            WmipPollingDpcRoutine,
                            &WmipCpeQueryInfo);

            ExInitializeWorkItem(&WmipCpeQueryInfo.WorkItem,
                                 WmipMceWorkerRoutine,
                                 &WmipCpeQueryInfo);
            
            KeInitializeTimerEx(&WmipCpeQueryInfo.PollingTimer,
                                NotificationTimer);

            //
            // Register our CMC and MCA callbacks. And if interrupt driven CPE
            // callbacks are enabled register them too
            //
            KernelMcaHandlerInfo.Version = KERNEL_ERROR_HANDLER_VERSION;
            KernelMcaHandlerInfo.KernelMcaDelivery = WmipMceDelivery;
            KernelMcaHandlerInfo.KernelCmcDelivery = WmipMceDelivery;
            KernelMcaHandlerInfo.KernelCpeDelivery = WmipMceDelivery;
            KernelMcaHandlerInfo.KernelMceDelivery = WmipMceEventDelivery;

            Status = HalSetSystemInformation(HalKernelErrorHandler,
                                             sizeof(KERNEL_ERROR_HANDLER_INFO),
                                             &KernelMcaHandlerInfo);

            if (NT_SUCCESS(Status))
            {
                WmipMCEState = MCE_STATE_REGISTERED;
            } else {
                WmipMCEState = (ULONG) MCE_STATE_ERROR;
                WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                                  DPFLTR_MCA_LEVEL | DPFLTR_ERROR_LEVEL,
                                  "WMI: Error %x registering MCA error handlers\n",
                                  Status));
            }
        }

    } else if (WmipMCEState != MCE_STATE_ERROR) {
        //
        // Phase 1 initialization is done after all of the boot drivers
        // have loaded and have had a chance to register for WMI event
        // notifications. At this point it is safe to go ahead and send
        // wmi events for MCA, CMC, CPE, etc

        //
        // If there were any MCA logs generated prior to boot then get
        // them out of the HAL and process them. Do this before
        // starting any polling since the SAL likes to have the
        // previous MCA records removed before being polled for CPE and
        // CMC
        //


        HalErrorInfo.Version = HAL_ERROR_INFO_VERSION;

        Status = HalQuerySystemInformation(HalErrorInformation,
                                           sizeof(HAL_ERROR_INFO),
                                           &HalErrorInfo,
                                           &ReturnSize);

        if ((NT_SUCCESS(Status)) &&
            (ReturnSize >= sizeof(HAL_ERROR_INFO)))
        {
            if (HalErrorInfo.McaPreviousEventsCount != 0)
            {
                //
                // We need to flush out any previous MCA logs and then
                // make them available via WMI
                //
                WmipProcessPrevMcaLogs();                
            }           
        }        

        //
        // Establish polling timer for CMC, if needed
        //
        if ((WmipCmcQueryInfo.PollFrequency != HAL_CMC_DISABLED) &&
            (WmipCmcQueryInfo.PollFrequency != HAL_CMC_INTERRUPTS_BASED))
        {
            li.QuadPart = -1 * (WmipCmcQueryInfo.PollFrequency * 1000000000);
            KeSetTimerEx(&WmipCmcQueryInfo.PollingTimer,
                         li,
                         WmipCmcQueryInfo.PollFrequency * 1000,
                         &WmipCmcQueryInfo.PollingDpc);
        } else if (WmipCmcQueryInfo.PollFrequency == HAL_CMC_INTERRUPTS_BASED) {
            //
            // CMC is interrupt based so we need to kick off an attempt
            // to read any CMC that had previously occured
            //
            WmipInsertQueueMCEDpc(&WmipCmcQueryInfo);
        }

        //
        // Establish polling timer for Cpe, if needed
        //
        if ((WmipCpeQueryInfo.PollFrequency != HAL_CPE_DISABLED) &&
            (WmipCpeQueryInfo.PollFrequency != HAL_CPE_INTERRUPTS_BASED))
        {
            li.QuadPart = -1 * (WmipCpeQueryInfo.PollFrequency * 1000000000);
            KeSetTimerEx(&WmipCpeQueryInfo.PollingTimer,
                         li,
                         WmipCpeQueryInfo.PollFrequency * 1000,
                         &WmipCpeQueryInfo.PollingDpc);
        } else if (WmipCpeQueryInfo.PollFrequency == HAL_CPE_INTERRUPTS_BASED) {
            //
            // Cpe is interrupt based so we need to kick off an attempt
            // to read any Cpe that had previously occured
            //
            WmipInsertQueueMCEDpc(&WmipCpeQueryInfo);
        }

        //
        // Flag that we are now able to start firing events
        //
        WmipMCEState = MCE_STATE_RUNNING;
        
        Status = STATUS_SUCCESS;
    }
    else {
        Status = STATUS_UNSUCCESSFUL;
    }
    
    return(Status);
}

NTSTATUS WmipGetRawMCAInfo(
    OUT PUCHAR Buffer,
    IN OUT PULONG BufferSize
    )
/*++

Routine Description:

    Return raw MCA log that was already retrieved from hal

Arguments:


Return Value:

    NT status code

--*/
{
    NTSTATUS status;

    PAGED_CODE();

    if (WmipRawMCA != NULL)
    {
        //
        // THere are logs so copy over all of the logs
        //
        if (*BufferSize >= WmipRawMCASize)
        {
            RtlCopyMemory(Buffer, WmipRawMCA, WmipRawMCASize);
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_BUFFER_TOO_SMALL;
        }
        *BufferSize = WmipRawMCASize;
    } else {
        //
        // There are no logs so return no records
        //
        if (*BufferSize >= sizeof(ULONG))
        {
            *(PULONG)Buffer = 0;
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_BUFFER_TOO_SMALL;
        }
        *BufferSize = sizeof(ULONG);        
    }
    
    return(status);
}


NTSTATUS WmipGetLogFromHal(
    IN HAL_QUERY_INFORMATION_CLASS InfoClass,
    IN PVOID Token,                        
    IN OUT PWNODE_SINGLE_INSTANCE *Wnode,
    OUT PERROR_LOGRECORD *Mca,
    OUT PULONG McaSize,
    IN ULONG MaxSize,
    IN LPGUID Guid                         
    )
/*++

Routine Description:

    This routine will call the HAL to get a log and possibly build a
    wnode event for it.

Arguments:

    InfoClass is the HalInformationClass that specifies the log
        information to retrieve

    Token is the HAL token for the log type

    *Wnode returns a pointer to a WNODE_EVENT_ITEM containing the log
        information if Wnode is not NULL

    *Mca returns a pointer to the log read from the hal. It may point
        into the memory pointed to by *Wnode

    *McaSize returns with the size of the log information.

    MaxSize has the maximum size to allocate for the log data

    Guid points to the guid to use if a Wnode is built

Return Value:

    NT status code

--*/
{
    NTSTATUS Status;
    PERROR_LOGRECORD Log;
    PWNODE_SINGLE_INSTANCE WnodeSI;
    PULONG Ptr;
    ULONG Size, LogSize, WnodeSize;

    PAGED_CODE();

    //
    // If we are reading directly into a wnode then set this up
    //
    if (Wnode != NULL)
    {
        WnodeSize = FIELD_OFFSET(WNODE_SINGLE_INSTANCE, VariableData) +
                    2 * sizeof(ULONG);
    } else {
        WnodeSize = 0;
    }

    //
    // Allocate a buffer to store the log reported from the hal. Note
    // that this must be in non paged pool as per the HAL.
    //
    Size = MaxSize + WnodeSize;
                                    
    Ptr = ExAllocatePoolWithTag(NonPagedPool,
                                Size,
                                WmipMCAPoolTag);
    if (Ptr != NULL)
    {
        Log = (PERROR_LOGRECORD)((PUCHAR)Ptr + WnodeSize);
        LogSize = Size - WnodeSize;

        *(PVOID *)Log = Token;
        
        Status = HalQuerySystemInformation(InfoClass,
                                           LogSize,
                                           Log,
                                           &LogSize);

        if (Status == STATUS_BUFFER_TOO_SMALL)
        {
            //
            // If our buffer was too small then the Hal lied to us when
            // it told us the maximum buffer size. This is ok as we'll
            // handle this situation by reallocating and trying again
            //
            ExFreePool(Log);

            //
            // Reallocate the buffer and call the hal to get the log
            //
            Size = LogSize + WnodeSize;
            Ptr = ExAllocatePoolWithTag(NonPagedPool,
                                        Size,
                                        WmipMCAPoolTag);
            if (Ptr != NULL)
            {
                Log = (PERROR_LOGRECORD)((PUCHAR)Ptr + WnodeSize);
                LogSize = Size - WnodeSize;

                *(PVOID *)Log = Token;
                Status = HalQuerySystemInformation(InfoClass,
                                                    LogSize,
                                                    Log,
                                                    &LogSize);

                //
                // The hal gave us a buffer size needed that was too
                // small, so lets stop right here and let him know]
                //
                WmipAssert(Status != STATUS_BUFFER_TOO_SMALL);
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if (NT_SUCCESS(Status))
        {
            //
            // We successfully read the data from the hal so build up
            // output buffers.
            //
            if (Wnode != NULL)
            {
                //
                // Caller requested buffer returned within a WNODE, so
                // build up the wnode around the log data
                //
                
                WnodeSI = (PWNODE_SINGLE_INSTANCE)Ptr;
                Status = WmipBuildMcaCmcEvent(WnodeSI,
                                              Guid,
                                              NULL,
                                              LogSize);
                *Wnode = WnodeSI;
            }
            
            *Mca = Log;
            *McaSize = LogSize;
        }

        if ((! NT_SUCCESS(Status)) && (Ptr != NULL))
        {
            //
            // If the function failed, but we have an allocated buffer
            // then clean it up
            //
            ExFreePool(Ptr);
        }
        
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return(Status);
}

#define MAX_ERROR_EVENT_SIZE \
    (((sizeof(WNODE_SINGLE_INSTANCE) + \
       (sizeof(USHORT) + sizeof(MCA_EVENT_INSTANCE_NAME)) + 7) & ~7) + \
     sizeof(MSMCAEvent_MemoryError))
                               
void WmipGenerateMCAEventlog(
    PUCHAR ErrorLog,
    ULONG ErrorLogSize,
    BOOLEAN IsFatal
    )
{
    PERROR_RECORD_HEADER RecordHeader;
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    PWCHAR w;
    ULONG BufferSize;
    PUCHAR Buffer, RawPtr = NULL;
    PWNODE_SINGLE_INSTANCE Wnode;
    PMSMCAEvent_Header Header;

    PAGED_CODE();
    
    RecordHeader = (PERROR_RECORD_HEADER)ErrorLog;

    //
    // Allocate a buffer large enough to accommodate any type of MCA.
    // Right now the largest is MSMCAEvent_MemoryError. If this changes
    // then this code should be updated
    //  
    BufferSize = MAX_ERROR_EVENT_SIZE + ErrorLogSize;

    //
    // Allocate a buffer to build the event
    //
    Buffer = ExAllocatePoolWithTag(PagedPool,
                                   BufferSize,
                                   WmipMCAPoolTag);
    
    if (Buffer != NULL)
    {
        //
        // Fill in the common fields of the WNODE
        //
        Wnode = (PWNODE_SINGLE_INSTANCE)Buffer;
        Wnode->WnodeHeader.BufferSize = BufferSize;
        Wnode->WnodeHeader.Linkage = 0;
        WmiInsertTimestamp(&Wnode->WnodeHeader);
        Wnode->WnodeHeader.Flags = WNODE_FLAG_SINGLE_INSTANCE |
                                   WNODE_FLAG_EVENT_ITEM;
        Wnode->OffsetInstanceName = sizeof(WNODE_SINGLE_INSTANCE);
        Wnode->DataBlockOffset = ((sizeof(WNODE_SINGLE_INSTANCE) +
                       (sizeof(USHORT) + sizeof(MCA_EVENT_INSTANCE_NAME)) +7) & ~7);

        w = (PWCHAR)OffsetToPtr(Wnode, Wnode->OffsetInstanceName);
        *w++ = sizeof(MCA_EVENT_INSTANCE_NAME);
        wcscpy(w, MCA_EVENT_INSTANCE_NAME);

        //
        // Fill in the common fields of the event data
        //
        Header = (PMSMCAEvent_Header)OffsetToPtr(Wnode, Wnode->DataBlockOffset);
        Header->Cpu = MCA_UNDEFINED_CPU;   // assume CPU will be undefined
        Header->AdditionalErrors = 0;
        Header->LogToEventlog = 1;
            
        //
        // Construct the error event using the data in the error log we
        // retrieved from the HAL.
        //
#if defined(_AMD64_) || defined(i386)
        if (IsFatal) {
#endif
        Status = WmipConstructMCAErrorEvent(
                     (PVOID) ErrorLog,
                     ErrorLogSize,
                     Wnode,
                     Header,
                     &RawPtr,
                     &IsFatal
                     );
#if defined(_AMD64_) || defined(i386)
        }
#endif

        //
        // If we were not able to build a specific event type then
        // we fallback and fire a generic one
        //
        if (! NT_SUCCESS(Status))
        {
            //
            // Build event for Unknown MCA
            //
            PMSMCAEvent_InvalidError Event;

            WmipAssert( sizeof(MSMCAEvent_MemoryError) >=
                        sizeof(MSMCAEvent_InvalidError) );

            Event = (PMSMCAEvent_InvalidError)Header;

            //
            // Fill in the data from the MCA within the WMI event
            //
            if (Header->Cpu == MCA_UNDEFINED_CPU)
            {
                Event->Type = IsFatal ? MCA_ERROR_UNKNOWN_NO_CPU :
                                        MCA_WARNING_UNKNOWN_NO_CPU;
            } else {
                Event->Type = IsFatal ? MCA_ERROR_UNKNOWN :
                                        MCA_WARNING_UNKNOWN;
            }

            Event->Size = ErrorLogSize;
            RawPtr = Event->RawRecord;

            //
            // Finish filling in WNODE fields
            //
            Wnode->WnodeHeader.Guid = WmipMSMCAEvent_InvalidErrorGuid;
            Wnode->SizeDataBlock = FIELD_OFFSET(MSMCAEvent_InvalidError,
                                               RawRecord) + ErrorLogSize;
        }

        //
        // Adjust the Error event count
        //
        if (Header->AdditionalErrors > 0)
        {
            Header->AdditionalErrors--;
        }
        
        //
        // Put the entire MCA record into the event
        //
        RtlCopyMemory(RawPtr,
                      RecordHeader,
                      ErrorLogSize);

        if ((! IsFatal) && (Header->LogToEventlog == 1))

        {
            if (WmipCorrectedEventlogCounter != 0)
            {
                //
                // Since this is a corrected error that is getting
                // logged to the eventlog we need to account for it
                //
                if ((WmipCorrectedEventlogCounter != 0xffffffff) &&
                    (--WmipCorrectedEventlogCounter == 0))
                {
                    WmipWriteToEventlog(MCA_INFO_NO_MORE_CORRECTED_ERROR_LOGS,
                                        STATUS_SUCCESS);
                }
            } else {
                //
                // We have exceeded the limit of corrected errors that
                // we are allowed to write into the eventlog, so we
                // just suppress it
                //
                Header->LogToEventlog = 0;
            }           
        }
        
        //
        // Now go and fire off the event
        //
        if ((WmipDisableMCAPopups == 0) &&
           (Header->LogToEventlog != 0))
        {
            IoRaiseInformationalHardError(STATUS_MCA_OCCURED,
                                          NULL,
                                          NULL);
        }

        if ((Header->LogToEventlog == 1) ||
            (WmipIsWbemRunning()))
        {
            //
            // Only fire off a WMI event if we want to log to the
            // eventlog or WBEM is up and running
            //
            Status = WmipWriteMCAEventLogEvent((PUCHAR)Wnode);
        }

        if (! NT_SUCCESS(Status))
        {
            ExFreePool(Wnode);
        }

    } else {

        //
        // Not enough memory to do a full MCA event so lets just do a
        // generic one.
        //
        WmipWriteToEventlog(
            IsFatal ? MCA_WARNING_UNKNOWN_NO_CPU : MCA_ERROR_UNKNOWN_NO_CPU,
            STATUS_INSUFFICIENT_RESOURCES
            );
    }
}



NTSTATUS WmipWriteMCAEventLogEvent(
    PUCHAR Event
    )
{
    PWNODE_HEADER Wnode = (PWNODE_HEADER)Event;
    NTSTATUS Status;

    PAGED_CODE();
    
    WmipEnterSMCritSection();
    
    if (WmipIsWbemRunning() ||
        WmipCheckIsWbemRunning())
    {
        //
        // We know WBEM is running so we can just fire off our event
        //
        WmipLeaveSMCritSection();
        Status = IoWMIWriteEvent(Event);
    } else {
        //
        // WBEM is not currently running and so startup a timer that
        // will keep polling it
        //
        if (WmipIsWbemRunningFlag == WBEM_STATUS_UNKNOWN)
        {
            //
            // No one has kicked off the waiting process for wbem so we
            // do that here. Note we need to maintain the critical
            // section to guard against another thread that might be
            // trying to startup the waiting process as well. Note that
            // if the setup fails we want to stay in the unknown state
            // so that the next time an event is fired we can retry
            // waiting for wbem
            //
            Status = WmipSetupWaitForWbem();
            if (NT_SUCCESS(Status))
            {
                WmipIsWbemRunningFlag = WAITING_FOR_WBEM;
            }
        }
        
        Wnode->ClientContext = Wnode->BufferSize;
        InsertTailList(&WmipWaitingMCAEvents,
                       (PLIST_ENTRY)Event);
        WmipLeaveSMCritSection();
        Status = STATUS_SUCCESS;
    }
    return(Status);
}

ULONG WmipWbemMinuteWait = 1;

NTSTATUS WmipSetupWaitForWbem(
    void
    )
{
    LARGE_INTEGER TimeOut;
    NTSTATUS Status;
    
    PAGED_CODE();

    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                      "WMI: SetupWaitForWbem starting\n"));

    //
    // Initialize a kernel time to fire periodically so we can
    // check if WBEM has started or not
    //
    KeInitializeTimer(&WmipIsWbemRunningTimer);

    KeInitializeDpc(&WmipIsWbemRunningDpc,
                    WmipIsWbemRunningDispatch,
                    NULL);

    ExInitializeWorkItem(&WmipIsWbemRunningWorkItem,
                         WmipIsWbemRunningWorker,
                         NULL);

    TimeOut.HighPart = -1;
    TimeOut.LowPart = -1 * (WmipWbemMinuteWait * 60 * 1000 * 10000);    // 1 minutes
    KeSetTimer(&WmipIsWbemRunningTimer,
               TimeOut,
               &WmipIsWbemRunningDpc);

    Status = STATUS_SUCCESS;

    return(Status);
}

void WmipIsWbemRunningDispatch(    
    IN PKDPC Dpc,
    IN PVOID DeferredContext,     // Not Used
    IN PVOID SystemArgument1,     // Not Used
    IN PVOID SystemArgument2      // Not Used
    )
{
    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

    ExQueueWorkItem(&WmipIsWbemRunningWorkItem,
                    DelayedWorkQueue);
}

void WmipIsWbemRunningWorker(
    PVOID Context
    )
{
    LARGE_INTEGER TimeOut;
    
    PAGED_CODE();
    
    UNREFERENCED_PARAMETER (Context);

    if (! WmipCheckIsWbemRunning())
    {
        //
        // WBEM is not yet started, so timeout in another minute to
        // check again
        //
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                          "WMI: IsWbemRunningWorker starting -> WBEM not started\n"));

        TimeOut.HighPart = -1;
        TimeOut.LowPart = (ULONG)(-1 * (1 *60 *1000 *10000));   // 1 minutes
        KeSetTimer(&WmipIsWbemRunningTimer,
                   TimeOut,
                   &WmipIsWbemRunningDpc);
        
    } else {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                          "WMI: WbemRunningWorker found wbem started\n"));

    }
}

BOOLEAN WmipCheckIsWbemRunning(
    void
    )
{
    OBJECT_ATTRIBUTES Obj;
    UNICODE_STRING Name;
    HANDLE Handle;
    LARGE_INTEGER TimeOut;
    BOOLEAN IsWbemRunning = FALSE;
    NTSTATUS Status;
    PWNODE_HEADER Wnode;

    PAGED_CODE();

    RtlInitUnicodeString(&Name,
                         L"\\BaseNamedObjects\\WBEM_ESS_OPEN_FOR_BUSINESS");

    
    InitializeObjectAttributes(
        &Obj,
        &Name,
        FALSE,
        NULL,
        NULL
        );

    Status = ZwOpenEvent(
                &Handle,
                SYNCHRONIZE,
                &Obj
                );

    if (NT_SUCCESS(Status))
    {
        TimeOut.QuadPart = 0;
        Status = ZwWaitForSingleObject(Handle,
                                       FALSE,
                                       &TimeOut);
        if (Status == STATUS_SUCCESS)
        {
            IsWbemRunning = TRUE;

            //
            // We've determined that WBEM is running so now lets see if
            // another thread has made that determination as well. If not
            // then we can flush the MCA event queue and set the flag
            // that WBEM is running
            //
            WmipEnterSMCritSection();
            if (WmipIsWbemRunningFlag != WBEM_IS_RUNNING)
            {
                //
                // Flush the list of all MCA events waiting to be fired
                //
                while (! IsListEmpty(&WmipWaitingMCAEvents))
                {
                    Wnode = (PWNODE_HEADER)RemoveHeadList(&WmipWaitingMCAEvents);
                    WmipLeaveSMCritSection();
                    Wnode->BufferSize = Wnode->ClientContext;
                    Wnode->Linkage = 0;
                    Status = IoWMIWriteEvent(Wnode);
                    if (! NT_SUCCESS(Status))
                    {
                        ExFreePool(Wnode);
                    }
                    WmipEnterSMCritSection();
                }
                
                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                  "WMI: WBEM is Running and queus flushed\n"));
                
                WmipIsWbemRunningFlag = WBEM_IS_RUNNING;
            }
            WmipLeaveSMCritSection();
        }
        ZwClose(Handle);
    }
    return(IsWbemRunning);
}

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

