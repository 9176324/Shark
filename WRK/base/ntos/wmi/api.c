/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    api.c

Abstract:

    Api entrypoints to WMI

--*/

#include "wmikmp.h"
#include "evntrace.h"
#include "tracep.h"

BOOLEAN WMIInitialize(
    ULONG Phase,
    PVOID LoaderBlock
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,WMIInitialize)
#pragma alloc_text(PAGE,IoWMIRegistrationControl)
#pragma alloc_text(PAGE,IoWMIAllocateInstanceIds)
#pragma alloc_text(PAGE,IoWMISuggestInstanceName)

#pragma alloc_text(PAGE,IoWMIOpenBlock)
#pragma alloc_text(PAGE,IoWMIQueryAllData)
#pragma alloc_text(PAGE,IoWMIQueryAllDataMultiple)
#pragma alloc_text(PAGE,IoWMIQuerySingleInstance)
#pragma alloc_text(PAGE,IoWMIQuerySingleInstanceMultiple)
#pragma alloc_text(PAGE,IoWMISetSingleInstance)
#pragma alloc_text(PAGE,IoWMISetSingleItem)
#pragma alloc_text(PAGE,IoWMISetNotificationCallback)
#pragma alloc_text(PAGE,IoWMIExecuteMethod)
#pragma alloc_text(PAGE,IoWMIHandleToInstanceName)
#pragma alloc_text(PAGE,IoWMIDeviceObjectToInstanceName)
#endif

    //
    // Mutex used to ensure single access to InstanceId chunks
PINSTIDCHUNK WmipInstIdChunkHead;

NTSTATUS
WmipDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

BOOLEAN WMIInitialize(
    ULONG Phase,
    PVOID LoaderBlockPtr
)
/*++

Routine Description:

    This routine is the initialization routine for WMI and is called by IO
    within IoInitSystem on NT.  This routine assumes that the
    IO system is initialized enough to call IoCreateDriver. The rest of the
    initialization occurs in the DriverEntry routine.

Arguments:

    Pass specifies the pass of initialization needed

Return Value:

    TRUE if initialization was successful

--*/
{
//
// We name the driver this so that any eventlogs fired will have the
// source name WMIxWDM and thus get the eventlog messages from
// ntiologc.mc
//
#define WMIDRIVERNAME L"\\Driver\\WMIxWDM"

    UNICODE_STRING DriverName;
    NTSTATUS Status;

#if !DBG
    UNREFERENCED_PARAMETER (LoaderBlockPtr);
#endif

    if (Phase == 0)
    {
        WmipAssert(WmipServiceDeviceObject == NULL);
        WmipAssert(LoaderBlockPtr != NULL);

        RtlInitUnicodeString(&DriverName, WMIDRIVERNAME);
        Status = IoCreateDriver(&DriverName, WmipDriverEntry);

    } else {
        WmipAssert(LoaderBlockPtr == NULL);
        
        WmipInitializeRegistration(Phase);
        WmipRegisterFirmwareProviders();

        Status = STATUS_SUCCESS;
    }

#if defined(_X86_) || defined(_AMD64_)
    //
    // Give MCA a chance to init during phase 0 and 1
    //
    WmipRegisterMcaHandler(Phase);
#endif      
    
    return(NT_SUCCESS(Status) ? TRUE : FALSE);
}

NTSTATUS IoWMIRegistrationControl(
    __in PDEVICE_OBJECT DeviceObject,
    __in ULONG Action
)
/*++

Routine Description:

    This routine informs WMI of the existence and disappearance of a device
    object that would support WMI.

Arguments:

    DeviceObject - Pointer to device object  or callback address

    Action - Registration action code

        WMIREG_ACTION_REGISTER - If set action is to inform WMI that the
            device object supports and is ready to receive WMI IRPS.

        WMIREG_ACTION_DEREGISTER - If set action is to inform WMI that the
            device object no longer supports and is not ready to receive WMI
            IRPS.

        WMIREG_ACTION_REREGISTER - If set action is to requery the device
            object for the guids that it supports. This has the effect of
            deregistering followed by registering.

        WMIREG_ACTION_UPDATE_GUIDS - If set action is to query for information
            that is used to update already registered guids.

        WMIREG_ACTION_BLOCK_IRPS - If set action is to block any further irps
            from being sent to the device. The irps are failed by WMI.

        If the  WMIREG_FLAG_CALLBACK is set then DeviceObject actually specifies a callback
            address and not a DeviceObject

Return Value:

    Returns status code

--*/
{
    NTSTATUS Status;
    ULONG RegistrationFlag = 0;
    ULONG IsTraceProvider = FALSE;
    ULONG TraceClass = 0;
    PREGENTRY RegEntry;

    PAGED_CODE();

    if (WmipIsWmiNotSetupProperly())
    {
        return(STATUS_UNSUCCESSFUL);
    }

    if (Action & WMIREG_FLAG_CALLBACK)
    {
        RegistrationFlag |= WMIREG_FLAG_CALLBACK;
        Action &= ~WMIREG_FLAG_CALLBACK;
    }

    if (Action & WMIREG_FLAG_TRACE_PROVIDER)
    {
        TraceClass = Action & WMIREG_FLAG_TRACE_NOTIFY_MASK;

        Action &= ~WMIREG_FLAG_TRACE_PROVIDER & ~WMIREG_FLAG_TRACE_NOTIFY_MASK;
        IsTraceProvider = TRUE;
        RegistrationFlag |= WMIREG_FLAG_TRACE_PROVIDER | TraceClass;
    }

    switch(Action)
    {
        case WMIREG_ACTION_REGISTER:
        {
            Status = WmipRegisterDevice(
                        DeviceObject,
                        RegistrationFlag);

            if (IsTraceProvider)
            {
                WmipSetTraceNotify(DeviceObject, TraceClass, TRUE);
            }
            break;
        }

        case WMIREG_ACTION_DEREGISTER:
        {
            Status = WmipDeregisterDevice(DeviceObject);
            break;
        }

        case WMIREG_ACTION_REREGISTER:
        {
            Status = WmipDeregisterDevice(DeviceObject);
            if (NT_SUCCESS(Status))
            {
                Status = WmipRegisterDevice(
                            DeviceObject,
                            RegistrationFlag);
            }
            break;
        }

        case WMIREG_ACTION_UPDATE_GUIDS:
        {
            Status = WmipUpdateRegistration(DeviceObject);
            break;
        }

        case WMIREG_ACTION_BLOCK_IRPS:
        {
            RegEntry = WmipFindRegEntryByDevice(DeviceObject, FALSE);
            if (RegEntry != NULL)
            {
                //
                // Mark the regentry as invalid so that no more irps
                // are sent to the device and the event will set when
                // the last irp completes.
                WmipEnterSMCritSection();
                RegEntry->Flags |= REGENTRY_FLAG_NOT_ACCEPTING_IRPS;
                WmipLeaveSMCritSection();
                WmipUnreferenceRegEntry(RegEntry);
                Status = STATUS_SUCCESS;
            } else {
                Status = STATUS_INVALID_PARAMETER;
            }

            break;
        }
        default:
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
    }
    return(Status);
}


NTSTATUS IoWMIAllocateInstanceIds(
    __in GUID *Guid,
    __in ULONG InstanceCount,
    __out ULONG *FirstInstanceId
    )
/*++

    Routine Description:

    This routine allocates a range of instance ids that are unique to the
    guid. This routine is to be called only at PASSIVE_LEVEL.

    Arguments:

        Guid - Pointer to guid for which instance ids are needed.
        InstanceCount - Count of instance ids to allocate.
        *FirstInstanceId - Returns first instance id in the range.

    Return Value:

        Returns a status code

--*/
{
    PINSTIDCHUNK InstIdChunk, LastInstIdChunk = NULL;
    PINSTID InstId;
    ULONG i;

    PAGED_CODE();

    if (WmipIsWmiNotSetupProperly())
    {
        return(STATUS_UNSUCCESSFUL);
    }

    WmipEnterSMCritSection();

    //
    // See if the guid is already in the list
    InstIdChunk = WmipInstIdChunkHead;

    while (InstIdChunk != NULL)
    {
        for (i = 0; i < INSTIDSPERCHUNK; i++)
        {
            InstId = &InstIdChunk->InstId[i];
            if (InstId->BaseId == ~0)
            {
                //
                // Since InstIds are always filled sequentially and are
                // never freed, if we hit a free one then we know that
                // our guid is not in the list and we need to fill in a
                // new entry.
                goto FillInstId;
            }

            if (IsEqualGUID(Guid, &InstId->Guid))
            {
                //
                // We found an entry for our guid so use its information
                *FirstInstanceId = InstId->BaseId;
                InstId->BaseId += InstanceCount;
                WmipLeaveSMCritSection();
                return(STATUS_SUCCESS);
            }
        }
        LastInstIdChunk = InstIdChunk;
        InstIdChunk = InstIdChunk->Next;
    }

    //
    // We need to allocate a brand new chunk to accommodate the entry
    InstIdChunk = ExAllocatePoolWithTag(PagedPool,
                                        sizeof(INSTIDCHUNK),
                                        WMIIIPOOLTAG);
    if (InstIdChunk != NULL)
    {
        RtlFillMemory(InstIdChunk, sizeof(INSTIDCHUNK), 0xff);
        InstIdChunk->Next = NULL;
        if (LastInstIdChunk == NULL)
        {
            WmipInstIdChunkHead = InstIdChunk;
        } else {
            LastInstIdChunk->Next = InstIdChunk;
        }

        InstId = &InstIdChunk->InstId[0];
    } else {
        WmipLeaveSMCritSection();
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

FillInstId:
    RtlCopyMemory(&InstId->Guid, Guid, sizeof(GUID));
    InstId->BaseId = InstanceCount;
    WmipLeaveSMCritSection();
    *FirstInstanceId = 0;

    return(STATUS_SUCCESS);
}

NTSTATUS IoWMISuggestInstanceName(
    __in_opt PDEVICE_OBJECT PhysicalDeviceObject,
    __in_opt PUNICODE_STRING SymbolicLinkName,
    __in BOOLEAN CombineNames,
    __out PUNICODE_STRING SuggestedInstanceName
    )
/*++

    Routine Description:

    This routine is used by a device driver to suggest a base name with which
    to build WMI instance names for the device. A driver is not bound to
    follow the instance name returned.

    Arguments:

    PhysicalDeviceObject - PDO of device for which a suggested instance name
        is being requested

    SymbolicLinkName - Symbolic link name returned from
        IoRegisterDeviceInterface.

    CombineNames - If TRUE then the suggested names arising from the
        PhysicalDeviceObject and the SymbolicLinkName are combined to create
        the resultant suggested name.

    SuggestedInstanceName - Supplies a pointer to an empty (i.e., Buffer
        field set to NULL) UNICODE_STRING structure which, upon success, will
        be set to a newly-allocated string buffer containing the suggested
        instance name.  The caller is responsible for freeing
        SuggestedInstanceName->Buffer when it is no longer needed.


    Note:  If CombineNames is TRUE then both PhysicalDeviceObject and
           SymbolicLinkName must be specified. Otherwise only one of them
           must be specified.

    Return Value:

        Returns a status code

--*/
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER_MIX;
    ULONG DeviceDescSizeRequired;
    ULONG DeviceDescSize;
    PWCHAR DeviceDescBuffer;
    HANDLE DeviceInstanceKey;
    PKEY_VALUE_FULL_INFORMATION InfoBuffer;
    PWCHAR SymLinkDescBuffer;
    ULONG InfoSizeRequired;
    ULONG ResultDescSize;
    PWCHAR ResultDescBuffer;
    UNICODE_STRING DefaultValue;

    PAGED_CODE();

    if (WmipIsWmiNotSetupProperly())
    {
        return(STATUS_UNSUCCESSFUL);
    }

    DeviceDescBuffer = NULL;
    DeviceDescSizeRequired = 0;
    DeviceDescSize = 0;
    
    if (PhysicalDeviceObject != NULL)
    {
        Status = IoGetDeviceProperty(PhysicalDeviceObject,
                                     DevicePropertyDeviceDescription,
                                     DeviceDescSize,
                                     DeviceDescBuffer,
                                     &DeviceDescSizeRequired);

        if (Status == STATUS_BUFFER_TOO_SMALL)
        {
            DeviceDescBuffer = ExAllocatePoolWithTag(PagedPool,
                                                     DeviceDescSizeRequired,
                                                     WMIPOOLTAG);
            if (DeviceDescBuffer == NULL)
            {
                return(STATUS_INSUFFICIENT_RESOURCES);
            }
 
            DeviceDescSize = DeviceDescSizeRequired;
            Status = IoGetDeviceProperty(PhysicalDeviceObject,
                                     DevicePropertyDeviceDescription,
                                     DeviceDescSize,
                                     DeviceDescBuffer,
                                     &DeviceDescSizeRequired);
            if (! NT_SUCCESS(Status))
            {
                ExFreePool(DeviceDescBuffer);
                return(Status);
            }
        } else if (! NT_SUCCESS(Status)) {
            return(Status);
        }
    }

    if (SymbolicLinkName != NULL)
    {
        Status = IoOpenDeviceInterfaceRegistryKey(SymbolicLinkName,
                                                  KEY_ALL_ACCESS,
                                                  &DeviceInstanceKey);
        if (NT_SUCCESS(Status))
        {
            //
            // Figure out how big the data value is so that a buffer of the
            // appropriate size can be allocated.
            DefaultValue.Length = 0;
            DefaultValue.MaximumLength= 0;
            DefaultValue.Buffer = NULL;
            Status = ZwQueryValueKey( DeviceInstanceKey,
                              &DefaultValue,
                              KeyValueFullInformation,
                              (PVOID) NULL,
                              0,
                              &InfoSizeRequired );
            if (Status == STATUS_BUFFER_OVERFLOW ||
                Status == STATUS_BUFFER_TOO_SMALL)
            {
                InfoBuffer = ExAllocatePoolWithTag(PagedPool,
                                            InfoSizeRequired,
                                            WMIPOOLTAG);
                if (InfoBuffer != NULL)
                {
                    Status = ZwQueryValueKey(DeviceInstanceKey,
                                             &DefaultValue,
                                             KeyValueFullInformation,
                                             InfoBuffer,
                                             InfoSizeRequired,
                                             &InfoSizeRequired);
                    if (NT_SUCCESS(Status))
                    {
                        SymLinkDescBuffer = (PWCHAR)((PCHAR)InfoBuffer + InfoBuffer->DataOffset);
                        if (CombineNames)
                        {
                            ResultDescSize = InfoBuffer->DataLength +
                                                    DeviceDescSizeRequired +
                                                    sizeof(WCHAR);
                            ResultDescBuffer = ExAllocatePoolWithTag(PagedPool,
                                                              ResultDescSize,
                                                              WMIPOOLTAG);
                            if (ResultDescBuffer == NULL)
                            {
                                Status = STATUS_INSUFFICIENT_RESOURCES;
                            } else {
                                SuggestedInstanceName->Buffer = ResultDescBuffer;
                                SuggestedInstanceName->Length =  0;
                                SuggestedInstanceName->MaximumLength = (USHORT)ResultDescSize;
                                if (DeviceDescBuffer != NULL)
                                {
                                    RtlAppendUnicodeToString(SuggestedInstanceName,
                                                             DeviceDescBuffer);
                                }
                                RtlAppendUnicodeToString(SuggestedInstanceName,
                                                         L"_");
                                RtlAppendUnicodeToString(SuggestedInstanceName,
                                                         SymLinkDescBuffer);

                            }
                            if (DeviceDescBuffer != NULL)
                            {
                                ExFreePool(DeviceDescBuffer);
                                DeviceDescBuffer= NULL;
                            }
                        } else {
                            if (DeviceDescBuffer != NULL)
                            {
                                ExFreePool(DeviceDescBuffer);
                                DeviceDescBuffer = NULL;
                            }
                            ResultDescBuffer = ExAllocatePoolWithTag(PagedPool,
                                                    InfoBuffer->DataLength,
                                                    WMIPOOLTAG);
                            if (ResultDescBuffer == NULL)
                            {
                                Status = STATUS_INSUFFICIENT_RESOURCES;
                            } else {
                                SuggestedInstanceName->Buffer = ResultDescBuffer;
                                SuggestedInstanceName->Length =  0;
                                SuggestedInstanceName->MaximumLength = (USHORT)InfoBuffer->DataLength;
                                RtlAppendUnicodeToString(SuggestedInstanceName,
                                                         SymLinkDescBuffer);
                            }

                        }
                    }

                    ExFreePool(InfoBuffer);
                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
            ZwClose(DeviceInstanceKey);
        }

        if ((DeviceDescBuffer != NULL) && (! NT_SUCCESS(Status)))
        {
            ExFreePool(DeviceDescBuffer);
        }
    } else {
        if (DeviceDescBuffer != NULL)
        {
            //
            // Only looking for device description from PDO
            SuggestedInstanceName->Buffer = DeviceDescBuffer;
            SuggestedInstanceName->Length =  (USHORT)DeviceDescSizeRequired - sizeof(WCHAR);
            SuggestedInstanceName->MaximumLength =  (USHORT)DeviceDescSize;
        } else {
            SuggestedInstanceName->Buffer = NULL;
            SuggestedInstanceName->Length =  (USHORT)0;
            SuggestedInstanceName->MaximumLength =  0;
        }
    }

    return(Status);
}

NTSTATUS IoWMIWriteEvent(
    __inout PVOID WnodeEventItem
    )
/*++

Routine Description:

    This routine will queue the passed WNODE_EVENT_ITEM for delivery to the
    WMI user mode agent. Once the event is delivered the WNODE_EVENT_ITEM
    buffer will be returned to the pool.

    This routine may be called at DPC level

Arguments:

    WnodeEventItem - Pointer to WNODE_EVENT_ITEM that has event information.

Return Value:

    Returns STATUS_SUCCESS or an error code

--*/
{
    NTSTATUS Status;
    PWNODE_HEADER WnodeHeader = (PWNODE_HEADER)WnodeEventItem;
    PULONG TraceMarker = (PULONG) WnodeHeader;
    KIRQL OldIrql;
    PREGENTRY RegEntry;
    PEVENTWORKCONTEXT EventContext;
    ULONG ProviderId;

    if (WmipIsWmiNotSetupProperly())
    {
        return(STATUS_UNSUCCESSFUL);
    }

    //
    // Special mode with high order bit set
    //
    if ((*TraceMarker & 0xC0000000) == TRACE_HEADER_FLAG)
    {
        return WmiTraceFastEvent(WnodeHeader);
    }

    if ( (WnodeHeader->Flags & WNODE_FLAG_TRACED_GUID) ||
         (WnodeHeader->Flags & WNODE_FLAG_LOG_WNODE) )
    {
        ULONG LoggerId = WmiGetLoggerId(WnodeHeader->HistoricalContext);
        ULONG IsTrace = WnodeHeader->Flags & WNODE_FLAG_TRACED_GUID;
        ULONG SavedSize = WnodeHeader->BufferSize;
        PULONG TraceMarker = (PULONG) WnodeHeader;

        if (SavedSize < sizeof(WNODE_HEADER))
            return STATUS_BUFFER_TOO_SMALL;

        //
        // If trace header, turn higher bit on and support
        // only full header
        //
        if (IsTrace)
        {
            if (SavedSize > 0XFFFF)    // restrict to USHORT max size
                return STATUS_BUFFER_OVERFLOW;

            *TraceMarker |= TRACE_HEADER_FLAG | TRACE_HEADER_EVENT_TRACE |
                            (TRACE_HEADER_TYPE_FULL_HEADER << 16);
        }
        else
        {
            if (SavedSize & TRACE_HEADER_FLAG)
                return STATUS_BUFFER_OVERFLOW;
        }

        Status = STATUS_INVALID_HANDLE;
        if (LoggerId > 0 && LoggerId < MAXLOGGERS)
        {
            if (WmipLoggerContext[LoggerId] != NULL)
            {
                //
                // NOTE: The rule here is that IoWMIWriteEvent is always
                // called in kernel mode, and the buffer needs not be probed!
                //
                Status = WmiTraceEvent(WnodeHeader, KernelMode);
            }
        }
        // NOTE: If it is a trace, we will not go any further
        // Otherwise, if it is a regular WMI event, it will still
        // be processed by WMI.

        if (IsTrace)
        {
            WnodeHeader->BufferSize = SavedSize;
            return Status;
        }
    }

    //
    // Memory for event buffers is limited so the size of any event is also
    // limited.
#if DBG
    if (WnodeHeader->BufferSize > LARGEKMWNODEEVENTSIZE)
    {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_EVENT_INFO_LEVEL,
                          "WMI: Large event %p fired by %x via WMI\n",
                 WnodeEventItem,
                 ((PWNODE_HEADER)WnodeEventItem)->ProviderId));
    }
#endif

    if (WnodeHeader->BufferSize <= WmipMaxKmWnodeEventSize)
    {

        EventContext = ExAllocatePoolWithTag(NonPagedPool,
                                             sizeof(EVENTWORKCONTEXT),
                                             WMINWPOOLTAG);
        if (EventContext != NULL)
        {       
            //
            // Try to take a refcount on the regentry associated with the
            // provider id in the event. If we are successful then we set a
            // flag in the wnode header saying so. When processing the
            // event in the work item we check the flag and if it is set
            // we'll go looking for the regentry on the active and zombie
            // lists and then use it. At that time it will give up the ref
            // count taken here so that if the regentry really is a zombie
            // then it will go away peacefully.
            //

            ProviderId = WnodeHeader->ProviderId;
            KeAcquireSpinLock(&WmipRegistrationSpinLock,
                              &OldIrql);

            RegEntry = WmipDoFindRegEntryByProviderId(ProviderId,
                                                      REGENTRY_FLAG_RUNDOWN);
            if (RegEntry != NULL)
            {
                WmipReferenceRegEntry(RegEntry);
            }

            KeReleaseSpinLock(&WmipRegistrationSpinLock,
                          OldIrql);                 

            WnodeHeader->ClientContext = WnodeHeader->Version;

            EventContext->RegEntry = RegEntry;
            EventContext->Wnode = WnodeHeader;
            
            ExInterlockedInsertTailList(
                &WmipNPEvent,
                &EventContext->ListEntry,
                &WmipNPNotificationSpinlock);
            //
            // If the queue was empty then there was no work item outstanding
            // to move from non paged to paged memory. So fire up a work item
            // to do so.
            if (InterlockedIncrement(&WmipEventWorkItems) == 1)
            {
                ExQueueWorkItem( &WmipEventWorkQueueItem, DelayedWorkQueue );
            }
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
        Status = STATUS_BUFFER_OVERFLOW;
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_EVENT_INFO_LEVEL,
                          "WMI: IoWMIWriteEvent detected an event %p fired by %x that exceeds the maximum event size\n",
                             WnodeEventItem,
                             ((PWNODE_HEADER)WnodeEventItem)->ProviderId));
    }

    return(Status);
}

// IoWMIDeviceObjectToProviderId is in register.c

NTSTATUS IoWMIOpenBlock(
    __in GUID *Guid,
    __in ULONG DesiredAccess,
    __out PVOID *DataBlockObject
    )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    WCHAR ObjectName[WmiGuidObjectNameLength+1];
    UNICODE_STRING ObjectString;
    ULONG Ioctl;
    NTSTATUS Status;
    HANDLE DataBlockHandle;
    
    PAGED_CODE();
    
    //
    // Establish the OBJECT_ATTRIBUTES for the guid object
    //
    StringCchCopyW(ObjectName, WmiGuidObjectNameLength+1, WmiGuidObjectDirectory);
    StringCchPrintfW(&ObjectName[WmiGuidObjectDirectoryLength-1],
                         WmiGuidObjectNameLength-8,
                         L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                         Guid->Data1, Guid->Data2, 
                         Guid->Data3,
                         Guid->Data4[0], Guid->Data4[1],
                         Guid->Data4[2], Guid->Data4[3],
                         Guid->Data4[4], Guid->Data4[5],
                         Guid->Data4[6], Guid->Data4[7]);
                     
    RtlInitUnicodeString(&ObjectString, ObjectName);
    
    RtlZeroMemory(&ObjectAttributes, sizeof(OBJECT_ATTRIBUTES));
    ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    ObjectAttributes.ObjectName = &ObjectString;
    ObjectAttributes.Attributes = OBJ_KERNEL_HANDLE;
    
    if (DesiredAccess & WMIGUID_NOTIFICATION)
    {
        Ioctl = IOCTL_WMI_OPEN_GUID_FOR_EVENTS;
    } else if (DesiredAccess & WRITE_DAC) {
        Ioctl = IOCTL_WMI_OPEN_GUID;
    } else {
        Ioctl = IOCTL_WMI_OPEN_GUID_FOR_QUERYSET;
    }
    
    Status = WmipOpenBlock(Ioctl,
                           KernelMode,
                           &ObjectAttributes,
                           DesiredAccess,
                           &DataBlockHandle);

    if (NT_SUCCESS(Status))
    {
        Status = ObReferenceObjectByHandle(DataBlockHandle,
                                           DesiredAccess,
                                           WmipGuidObjectType,
                                           KernelMode,
                                           DataBlockObject,
                                           NULL);
        ZwClose(DataBlockHandle);
    }
    
    return(Status);                           
}


//
// Useful macro to establish a WNODE_HEADER quickly
#define WmipBuildWnodeHeader(Wnode, WnodeSize, FlagsUlong, Handle) { \
    ((PWNODE_HEADER)(Wnode))->Flags = FlagsUlong;                    \
    ((PWNODE_HEADER)(Wnode))->KernelHandle = Handle;                 \
    ((PWNODE_HEADER)(Wnode))->BufferSize = WnodeSize;                \
    ((PWNODE_HEADER)(Wnode))->Linkage = 0;                           \
}

NTSTATUS IoWMIQueryAllData(
    __in PVOID DataBlockObject,
    __inout ULONG *InOutBufferSize,
    __out_bcount_opt(*InOutBufferSize) /* non paged */ PVOID OutBuffer
)
{
    NTSTATUS Status;    
    WNODE_ALL_DATA WnodeAD;
    ULONG WnodeSize;
    ULONG RetSize;
    PWNODE_ALL_DATA Wnode;
    
    PAGED_CODE();
    
    //
    // See if the caller passed a buffer that is large enough
    //
    WnodeSize = *InOutBufferSize;   
    Wnode = (PWNODE_ALL_DATA)OutBuffer;
    if ((Wnode == NULL) || (WnodeSize < sizeof(WNODE_ALL_DATA)))
    {
        Wnode = &WnodeAD;
        WnodeSize = sizeof(WnodeAD);
    }
    
    //
    // Initialize buffer for query
    //
    WmipBuildWnodeHeader(Wnode,
                         sizeof(WNODE_HEADER),
                         WNODE_FLAG_ALL_DATA,
                         NULL);
        
    Status = WmipQueryAllData(DataBlockObject,
                              NULL,
                              KernelMode,
                              Wnode,
                              WnodeSize,
                              &RetSize);
                                  
    //
    // if this was a successful query then extract the results
    //
    if (NT_SUCCESS(Status))
    {
        if (Wnode->WnodeHeader.Flags & WNODE_FLAG_INTERNAL)
        {
            //
            // Internal guids are not supported in KM
            //
            Status = STATUS_NOT_SUPPORTED;
        } else if (Wnode->WnodeHeader.Flags & WNODE_FLAG_TOO_SMALL) {
            //
            // Buffer passed was too small for provider
            //
            *InOutBufferSize = ((PWNODE_TOO_SMALL)Wnode)->SizeNeeded;
            Status = STATUS_BUFFER_TOO_SMALL;
        } else {
            //
            // Buffer was large enough for provider
            //
            *InOutBufferSize = RetSize;
            
            if (Wnode == &WnodeAD)
            {
                //
                // Although there was enough room for the provider,
                // the caller didn't pass a large enough buffer
                // so we need to return a buffer too small error
                //
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        }
    }   
    
    return(Status);
}


NTSTATUS
IoWMIQueryAllDataMultiple(
    __in_ecount(ObjectCount) PVOID *DataBlockObjectList,
    __in ULONG ObjectCount,
    __inout ULONG *InOutBufferSize,
    __out_bcount_opt(*InOutBufferSize) PVOID OutBuffer
)
{
    NTSTATUS Status;
    WNODE_ALL_DATA WnodeAD;
    PWNODE_HEADER Wnode;
    ULONG WnodeSize;
    ULONG RetSize;

    PAGED_CODE();

    if (!ARGUMENT_PRESENT(DataBlockObjectList) ||
        (ObjectCount == 0) ||
        !ARGUMENT_PRESENT(InOutBufferSize)) {

        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    //
    // Make sure we have an output buffer
    //
    WnodeSize = *InOutBufferSize;   
    Wnode = (PWNODE_HEADER)OutBuffer;
    if ((Wnode == NULL) || (WnodeSize < sizeof(WNODE_ALL_DATA)))
    {
        Wnode = (PWNODE_HEADER)&WnodeAD;
        WnodeSize = sizeof(WnodeAD);
    }
    
    Status = WmipQueryAllDataMultiple(ObjectCount,
                                      (PWMIGUIDOBJECT *)DataBlockObjectList,
                                      NULL,
                                      KernelMode,
                                      (PUCHAR)Wnode,
                                      WnodeSize,
                                      NULL,
                                      &RetSize);
    //
    // if this was a successful query then extract the results
    //
    if (NT_SUCCESS(Status))
    {
        if (Wnode->Flags & WNODE_FLAG_TOO_SMALL)
        {
            //
            // Buffer passed to provider was too small
            //
            *InOutBufferSize = ((PWNODE_TOO_SMALL)Wnode)->SizeNeeded;
            Status = STATUS_BUFFER_TOO_SMALL;
        } else {
            //
            // Buffer was large enough for provider
            //
            *InOutBufferSize = RetSize;
            
            if (Wnode == (PWNODE_HEADER)&WnodeAD)
            {
                //
                // Although there was enough room for the provider,
                // the caller didn't pass a large enough buffer
                // so we need to return a buffer too small error
                //
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        }
    }

Exit:
    
    return(Status);
}


NTSTATUS
IoWMIQuerySingleInstance(
    __in PVOID DataBlockObject,
    __in PUNICODE_STRING InstanceName,
    __inout ULONG *InOutBufferSize,
    __out_bcount_opt(*InOutBufferSize) PVOID OutBuffer
)
{
    NTSTATUS Status;
    PWNODE_SINGLE_INSTANCE WnodeSI;
    ULONG WnodeSize;
    PWCHAR WPtr;
    ULONG SizeNeeded;
    ULONG RetSize;

    PAGED_CODE();
    
    //
    // Make sure we have an output buffer
    //
    SizeNeeded = (FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                 VariableData) +
                   InstanceName->Length +
                   sizeof(USHORT) + 7) & ~7;


    WnodeSize = *InOutBufferSize;   
    WnodeSI = (PWNODE_SINGLE_INSTANCE)OutBuffer;
    if ((WnodeSI == NULL) || (WnodeSize < SizeNeeded))
    {
        WnodeSI = (PWNODE_SINGLE_INSTANCE)WmipAllocNP(SizeNeeded);
        WnodeSize = SizeNeeded;
    }
            
    if (WnodeSI != NULL)
    {
        //
        // Build WNODE_SINGLE_INSTANCE appropriately and query
        //
        RtlZeroMemory(WnodeSI, FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                          VariableData));
                                      
        WmipBuildWnodeHeader(WnodeSI,
                             SizeNeeded,
                             WNODE_FLAG_SINGLE_INSTANCE,
                             NULL);

        WnodeSI->OffsetInstanceName = FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                                 VariableData);
        WnodeSI->DataBlockOffset = SizeNeeded;

        //
        // Copy InstanceName into the WnodeSingleInstance for the
        // query. Instance name takes a USHORT Length and then followed
        // by a string.
        //
        WPtr = (PWCHAR)OffsetToPtr(WnodeSI, WnodeSI->OffsetInstanceName);
        *WPtr++ = InstanceName->Length;
        RtlCopyMemory(WPtr, InstanceName->Buffer, InstanceName->Length);
                    
        
        Status = WmipQuerySetExecuteSI((PWMIGUIDOBJECT)DataBlockObject,
                                       NULL,
                                       KernelMode,
                                       IRP_MN_QUERY_SINGLE_INSTANCE,
                                       (PWNODE_HEADER)WnodeSI,
                                       WnodeSize,
                                       &RetSize);
    
        //
        // if this was a successful query then extract the results
        //
        if (NT_SUCCESS(Status))
        {
            if (WnodeSI->WnodeHeader.Flags & WNODE_FLAG_INTERNAL)
            {
                //
                // Internal guids are not supported in KM
                //
                Status = STATUS_NOT_SUPPORTED;
            } else if (WnodeSI->WnodeHeader.Flags & WNODE_FLAG_TOO_SMALL) {
                //
                // Our buffer was too small
                //
                *InOutBufferSize = ((PWNODE_TOO_SMALL)WnodeSI)->SizeNeeded;
                Status = STATUS_BUFFER_TOO_SMALL;
            } else {
                //
                // Buffer not too small, remember output size
                //
                *InOutBufferSize = RetSize;
                
                if (WnodeSI != OutBuffer)
                {
                    //
                    // Although there was enough room for the provider,
                    // the caller didn't pass a large enough buffer
                    // so we need to return a buffer too small error
                    //
                    Status = STATUS_BUFFER_TOO_SMALL;
                }
            }
        }       
        
        if (WnodeSI != OutBuffer)
        {
            WmipFree(WnodeSI);
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return(Status);
}

NTSTATUS
IoWMIQuerySingleInstanceMultiple(
    __in_ecount(ObjectCount) PVOID *DataBlockObjectList,
    __in_ecount(ObjectCount) PUNICODE_STRING InstanceNames,
    __in ULONG ObjectCount,
    __inout ULONG *InOutBufferSize,
    __out_bcount_opt(*InOutBufferSize) PVOID OutBuffer
)
{
    NTSTATUS Status;
    ULONG RetSize;
    PWNODE_HEADER Wnode;
    WNODE_TOO_SMALL WnodeTooSmall;
    ULONG WnodeSize;
    
    PAGED_CODE();

    if (!ARGUMENT_PRESENT(DataBlockObjectList) ||
        !ARGUMENT_PRESENT(InstanceNames) ||
        (ObjectCount == 0) ||
        !ARGUMENT_PRESENT(InOutBufferSize)) {

        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    WnodeSize = *InOutBufferSize;
    Wnode = (PWNODE_HEADER)OutBuffer;
    if ((Wnode == NULL) || (WnodeSize < sizeof(WNODE_TOO_SMALL)))
    {
        Wnode = (PWNODE_HEADER)&WnodeTooSmall;
        WnodeSize = sizeof(WNODE_TOO_SMALL);
    }

    Status = WmipQuerySingleMultiple(NULL,
                                     KernelMode,
                                     (PUCHAR)Wnode,
                                     WnodeSize,
                                     NULL,
                                     ObjectCount,
                                     (PWMIGUIDOBJECT *)DataBlockObjectList,
                                     InstanceNames,
                                     &RetSize);
                            
                                 
    //
    // if this was a successful query then extract the results
    //
    if (NT_SUCCESS(Status))
    {
        if (Wnode->Flags & WNODE_FLAG_TOO_SMALL)
        {
            //
            // Buffer passed to provider was too small
            //
            *InOutBufferSize = ((PWNODE_TOO_SMALL)Wnode)->SizeNeeded;
            Status = STATUS_BUFFER_TOO_SMALL;
        } else {
            //
            // Buffer was large enough for provider
            //
            *InOutBufferSize = RetSize;
            
                
            if (Wnode == (PWNODE_HEADER)&WnodeTooSmall)
            {
                //
                // Although there was enough room for the provider,
                // the caller didn't pass a large enough buffer
                // so we need to return a buffer too small error
                //
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        }
    }

Exit:
    
    return(Status);
}

NTSTATUS
IoWMISetSingleInstance(
    __in PVOID DataBlockObject,
    __in PUNICODE_STRING InstanceName,
    __in ULONG Version,
    __in ULONG ValueBufferSize,
    __in_bcount(ValueBufferSize) PVOID ValueBuffer
    )
{
    NTSTATUS Status;
    PWNODE_SINGLE_INSTANCE WnodeSI;
    PWCHAR WPtr;
    ULONG SizeNeeded;
    ULONG RetSize;
    ULONG InstanceOffset;
    ULONG DataOffset;
    PUCHAR DPtr;

    PAGED_CODE();
    
    InstanceOffset = (FIELD_OFFSET(WNODE_SINGLE_INSTANCE, 
                                   VariableData) + 1) & ~1;
                               
    DataOffset = (InstanceOffset + 
                  InstanceName->Length + sizeof(USHORT) + 7) & ~7;
                            
    SizeNeeded = DataOffset + ValueBufferSize;

    WnodeSI = (PWNODE_SINGLE_INSTANCE)WmipAllocNP(SizeNeeded);
            
    if (WnodeSI != NULL)
    {
        //
        // Build WNODE_SINGLE_INSTANCE appropriately and query
        //
        RtlZeroMemory(WnodeSI, FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                            VariableData));
                                      
        WmipBuildWnodeHeader(WnodeSI,
                             SizeNeeded,
                             WNODE_FLAG_SINGLE_INSTANCE,
                             NULL);
                         
        WnodeSI->WnodeHeader.Version = Version;

        //
        // Copy InstanceName into the WnodeSingleInstance for the query.
        //
        WnodeSI->OffsetInstanceName = InstanceOffset;
        WPtr = (PWCHAR)OffsetToPtr(WnodeSI, WnodeSI->OffsetInstanceName);
        *WPtr++ = InstanceName->Length;
        RtlCopyMemory(WPtr, InstanceName->Buffer, InstanceName->Length);
                                             
        //
        // Copy the new data into the WNODE
        //
        WnodeSI->SizeDataBlock = ValueBufferSize;
        WnodeSI->DataBlockOffset = DataOffset;
        DPtr = OffsetToPtr(WnodeSI, WnodeSI->DataBlockOffset);
        RtlCopyMemory(DPtr, ValueBuffer, ValueBufferSize);
        
        Status = WmipQuerySetExecuteSI(DataBlockObject,
                                       NULL,
                                       KernelMode,
                                       IRP_MN_CHANGE_SINGLE_INSTANCE,
                                       (PWNODE_HEADER)WnodeSI,
                                       SizeNeeded,
                                       &RetSize);
    
        WmipFree(WnodeSI);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return(Status); 
}

NTSTATUS
IoWMISetSingleItem(
    __in PVOID DataBlockObject,
    __in PUNICODE_STRING InstanceName,
    __in ULONG DataItemId,
    __in ULONG Version,
    __in ULONG ValueBufferSize,
    __in_bcount(ValueBufferSize) PVOID ValueBuffer
    )
{
    NTSTATUS Status;
    PWNODE_SINGLE_ITEM WnodeSI;
    PWCHAR WPtr;
    ULONG SizeNeeded;
    ULONG RetSize;
    ULONG InstanceOffset;
    ULONG DataOffset;
    PUCHAR DPtr;

    PAGED_CODE();
    
    InstanceOffset = (FIELD_OFFSET(WNODE_SINGLE_ITEM, 
                                   VariableData) + 1) & ~1;
                               
    DataOffset = (InstanceOffset + 
                  InstanceName->Length + sizeof(USHORT) + 7) & ~7;
                            
    SizeNeeded = DataOffset + ValueBufferSize;

    WnodeSI = (PWNODE_SINGLE_ITEM)WmipAllocNP(SizeNeeded);
            
    if (WnodeSI != NULL)
    {
        //
        // Build WNODE_SINGLE_INSTANCE appropriately and query
        //
        RtlZeroMemory(WnodeSI, FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                            VariableData));
                                      
        WmipBuildWnodeHeader(WnodeSI,
                             SizeNeeded,
                             WNODE_FLAG_SINGLE_ITEM,
                             NULL);
                         
        WnodeSI->WnodeHeader.Version = Version;
        WnodeSI->ItemId = DataItemId;

        //
        // Copy InstanceName into the WnodeSingleInstance for the query.
        //
        WnodeSI->OffsetInstanceName = InstanceOffset;
        WPtr = (PWCHAR)OffsetToPtr(WnodeSI, WnodeSI->OffsetInstanceName);
        *WPtr++ = InstanceName->Length;
        RtlCopyMemory(WPtr, InstanceName->Buffer, InstanceName->Length);
                                             
        //
        // Copy the new data into the WNODE
        //
        WnodeSI->SizeDataItem = ValueBufferSize;
        WnodeSI->DataBlockOffset = DataOffset;
        DPtr = OffsetToPtr(WnodeSI, WnodeSI->DataBlockOffset);
        RtlCopyMemory(DPtr, ValueBuffer, ValueBufferSize);
        
        Status = WmipQuerySetExecuteSI(DataBlockObject,
                                       NULL,
                                       KernelMode,
                                       IRP_MN_CHANGE_SINGLE_ITEM,
                                       (PWNODE_HEADER)WnodeSI,
                                       SizeNeeded,
                                       &RetSize);
    
        WmipFree(WnodeSI);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return(Status); 
}

NTSTATUS IoWMIExecuteMethod(
    __in PVOID DataBlockObject,
    __in PUNICODE_STRING InstanceName,
    __in ULONG MethodId,
    __in ULONG InBufferSize,
    __inout PULONG OutBufferSize,
    __inout_bcount_part_opt(*OutBufferSize, InBufferSize) PUCHAR InOutBuffer
    )
{
    NTSTATUS Status;
    PWNODE_METHOD_ITEM WnodeMI;
    PWCHAR WPtr;
    PUCHAR DPtr;
    ULONG SizeNeeded;
    ULONG RetSize;
    ULONG DataOffset;

    PAGED_CODE();
    
    //
    // Make sure we have an output buffer
    //
    DataOffset = (FIELD_OFFSET(WNODE_METHOD_ITEM,
                                 VariableData) +
                   InstanceName->Length +
                   sizeof(USHORT) +
                   7) & ~7;
    
    SizeNeeded =  DataOffset +
                   ((InBufferSize > *OutBufferSize) ? InBufferSize :
                                                      *OutBufferSize);
    
    WnodeMI = (PWNODE_METHOD_ITEM)WmipAllocNP(SizeNeeded);
            
    if (WnodeMI != NULL)
    {
        //
        // Build WNODE_SINGLE_INSTANCE appropriately and query
        //
        RtlZeroMemory(WnodeMI, FIELD_OFFSET(WNODE_METHOD_ITEM,
                                          VariableData));
                                      
        WmipBuildWnodeHeader(WnodeMI,
                             SizeNeeded,
                             WNODE_FLAG_METHOD_ITEM,
                             NULL);
        
        WnodeMI->MethodId = MethodId;

        WnodeMI->OffsetInstanceName = FIELD_OFFSET(WNODE_METHOD_ITEM,
                                                   VariableData);
        WnodeMI->DataBlockOffset = DataOffset;
        WnodeMI->SizeDataBlock = InBufferSize;

        //
        // Copy InstanceName into the WnodeMethodItem for the query.
        //
        WPtr = (PWCHAR)OffsetToPtr(WnodeMI, WnodeMI->OffsetInstanceName);
        *WPtr++ = InstanceName->Length;
        RtlCopyMemory(WPtr, InstanceName->Buffer, InstanceName->Length);

        //
        // Copy the input data into the WnodeMethodItem
        //
        DPtr = (PUCHAR)OffsetToPtr(WnodeMI, WnodeMI->DataBlockOffset);
        RtlCopyMemory(DPtr, InOutBuffer, InBufferSize);
        
        Status = WmipQuerySetExecuteSI(DataBlockObject,NULL,
                                       KernelMode,
                                       IRP_MN_EXECUTE_METHOD,
                                       (PWNODE_HEADER)WnodeMI,
                                       SizeNeeded,
                                       &RetSize);
    
        //
        // if this was a successful query then extract the results
        //
        if (NT_SUCCESS(Status))
        {
            if (WnodeMI->WnodeHeader.Flags & WNODE_FLAG_TOO_SMALL)
            {
                //
                // Our buffer was too small
                //
                *OutBufferSize = ( (((PWNODE_TOO_SMALL)WnodeMI)->SizeNeeded -
                                 DataOffset) + 7 ) & ~7;
                Status = STATUS_BUFFER_TOO_SMALL;
            } else {
                //
                // Buffer not too small, remember output size
                //
                if (*OutBufferSize >= WnodeMI->SizeDataBlock)
                {
                    *OutBufferSize = WnodeMI->SizeDataBlock;
                    DPtr = (PUCHAR)OffsetToPtr(WnodeMI,
                                               WnodeMI->DataBlockOffset);
                    RtlCopyMemory(InOutBuffer, DPtr, WnodeMI->SizeDataBlock);
                } else {
                    *OutBufferSize = (WnodeMI->SizeDataBlock + 7) & ~7;
                    Status = STATUS_BUFFER_TOO_SMALL;
                }
            }
        }       
        
        WmipFree(WnodeMI);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return(Status);
}

NTSTATUS
IoWMISetNotificationCallback(
    __inout PVOID Object,
    __in WMI_NOTIFICATION_CALLBACK Callback,
    __in_opt PVOID Context
    )
{
    PWMIGUIDOBJECT GuidObject;
    
    PAGED_CODE();

    GuidObject = (PWMIGUIDOBJECT)Object;
    
    WmipAssert(GuidObject->Flags & WMIGUID_FLAG_KERNEL_NOTIFICATION);
    
    WmipEnterSMCritSection();
    
    GuidObject->Callback = Callback;
    GuidObject->CallbackContext = Context;
    
    WmipLeaveSMCritSection();

    return(STATUS_SUCCESS);
}

NTSTATUS IoWMIHandleToInstanceName(
    __in PVOID DataBlockObject,
    __in HANDLE FileHandle,
    __out PUNICODE_STRING InstanceName
    )
{
    NTSTATUS Status;
    
    PAGED_CODE();

    Status = WmipTranslateFileHandle(NULL,
                                     NULL,
                                     FileHandle,
                                     NULL,
                                     DataBlockObject,
                                     InstanceName);
    return(Status);
}

NTSTATUS IoWMIDeviceObjectToInstanceName(
    __in PVOID DataBlockObject,
    __in PDEVICE_OBJECT DeviceObject,
    __out PUNICODE_STRING InstanceName
    )
{
    NTSTATUS Status;
    
    PAGED_CODE();

    Status = WmipTranslateFileHandle(NULL,
                                     NULL,
                                     NULL,
                                     DeviceObject,
                                     DataBlockObject,
                                     InstanceName);
    return(Status);
}

