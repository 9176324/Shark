/*++

Copyright (c) 1999  Microsoft Corporation

Module Name: 

    util.c

Abstract


Author:

    Peter Binder (pbinder) 7/13/99

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
7/13/99  pbinder   birth (functions taken from 1394diag)
--*/

NTSTATUS
t1394_SubmitIrpAsync(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIRB                 Irb
    )
{
    PIO_STACK_LOCATION  NextIrpStack;
    NTSTATUS            ntStatus = STATUS_SUCCESS;

    ENTER("t1394_SubmitIrpAsync");

    if (Irb) {

        NextIrpStack = IoGetNextIrpStackLocation(Irp);
        NextIrpStack->Parameters.Others.Argument1 = Irb;
    }
    else {

        IoCopyCurrentIrpStackLocationToNext(Irp);
    }

    ntStatus = IoCallDriver (DeviceObject, Irp);
    return ntStatus;
}

NTSTATUS
t1394_SubmitIrpSynch(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIRB                 Irb
    )
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    KEVENT              Event;
    PIO_STACK_LOCATION  NextIrpStack;

    ENTER("t1394_SubmitIrpSynch");

    TRACE(TL_TRACE, ("DeviceObject = 0x%x\n", DeviceObject));
    TRACE(TL_TRACE, ("Irp = 0x%x\n", Irp));
    TRACE(TL_TRACE, ("Irb = 0x%x\n", Irb));

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    if (Irb) {

        NextIrpStack = IoGetNextIrpStackLocation(Irp);
        NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
        NextIrpStack->Parameters.Others.Argument1 = Irb;
    }
    else {

        IoCopyCurrentIrpStackLocationToNext(Irp);
    }

    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

    IoSetCompletionRoutine( Irp,
                            t1394_SynchCompletionRoutine,
                            &Event,
                            TRUE,
                            TRUE,
                            TRUE
                            );

    ntStatus = IoCallDriver(DeviceObject, Irp);

    if (ntStatus == STATUS_PENDING) {

        TRACE(TL_TRACE, ("t1394_SubmitIrpSynch: Irp is pending...\n"));

        KeWaitForSingleObject( &Event,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL
                               );
	}
    ntStatus = Irp->IoStatus.Status;
    EXIT("t1394_SubmitIrpSynch", ntStatus);
    return(ntStatus);
} // t1394_SubmitIrpSynch

NTSTATUS
t1394_SynchCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PKEVENT          Event
    )
{
    NTSTATUS        ntStatus = STATUS_SUCCESS;

    ENTER("t1394_SynchCompletionRoutine");

    if (Event)
        KeSetEvent(Event, 0, FALSE);
    
    EXIT("t1394_SynchCompletionRoutine", ntStatus);
    return(STATUS_MORE_PROCESSING_REQUIRED);
} // t1394_SynchCompletionRoutine


BOOLEAN
t1394_IsOnList(
	PLIST_ENTRY		Entry,
	PLIST_ENTRY		List
	)
{
	PLIST_ENTRY TempEntry;

    for(
        TempEntry = List->Flink;
        TempEntry != List;
        TempEntry = TempEntry->Flink
        )
    {
        if (TempEntry == Entry) 
        {
			TRACE(TL_TRACE, ("Entry 0x%x found on list 0x%x\n", Entry, List));
			return TRUE;
        }
    }

	TRACE(TL_TRACE, ("Entry 0x%x not found on list 0x%x\n", Entry, List));
    return FALSE;
}


VOID
t1394_UpdateGenerationCount (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
{
	PDEVICE_EXTENSION       deviceExtension = DeviceObject->DeviceExtension;
	PIO_WORKITEM			ioWorkItem		= Context;
	NTSTATUS				ntStatus		= STATUS_SUCCESS;
	PIRB					Irb				= NULL;
	PIRP					Irp				= NULL;
		

	ENTER("t1394_UpdateGenerationCountWorkItem");

	// allocate irp
    Irp = IoAllocateIrp (deviceExtension->StackDeviceObject->StackSize, FALSE);
	if (!Irp)
	{
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		TRACE(TL_ERROR, ("Failed to allocate Irp!\n"));
		goto Exit_UpdateGenerationCountWorkItem;
	}

	// allocate irb
	Irb = ExAllocatePool(NonPagedPool, sizeof (IRB));
	if (!Irb)
	{
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		TRACE(TL_ERROR, ("Failed to allocate Irb!\n"));
		goto Exit_UpdateGenerationCountWorkItem;
	}

	// send request down stack
    RtlZeroMemory (Irb, sizeof (IRB));
    Irb->FunctionNumber = REQUEST_GET_GENERATION_COUNT;
    Irb->Flags = 0;

	ntStatus = t1394_SubmitIrpSynch(deviceExtension->StackDeviceObject, Irp, Irb);
    
	// update DeviceExtension->GenerationCount
	if (NT_SUCCESS(ntStatus))
	{
        deviceExtension->GenerationCount = Irb->u.GetGenerationCount.GenerationCount;
        TRACE(TL_TRACE, ("GenerationCount = 0x%x\n", deviceExtension->GenerationCount));
    }
    else
	{
        TRACE(TL_ERROR, ("SubmitIrpSync failed = 0x%x\n", ntStatus));
    }
	
Exit_UpdateGenerationCountWorkItem:

	if (Irp)
	{
		IoFreeIrp(Irp);
	}

	if (Irb)
	{
		ExFreePool (Irb);
	}

	if (ioWorkItem)
	{
		IoFreeWorkItem(ioWorkItem);
	}

	EXIT("t1394_UpdateGenerationCountWorkItem", ntStatus);

	return;
}


