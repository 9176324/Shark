/*++

Copyright (C) Microsoft Corporation, 1991 - 1999

Module Name:

    xferpkt.c

Abstract:

    Packet routines for CLASSPNP

Environment:

    kernel mode only

Notes:


Revision History:

--*/

#include "classp.h"
#include "debug.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, InitializeTransferPackets)
    #pragma alloc_text(PAGE, DestroyAllTransferPackets)
    #pragma alloc_text(PAGE, SetupEjectionTransferPacket)
    #pragma alloc_text(PAGE, SetupModeSenseTransferPacket)
#endif


ULONG MinWorkingSetTransferPackets = MIN_WORKINGSET_TRANSFER_PACKETS_Consumer;
ULONG MaxWorkingSetTransferPackets = MAX_WORKINGSET_TRANSFER_PACKETS_Consumer;


/*
 *  InitializeTransferPackets
 *
 *      Allocate/initialize TRANSFER_PACKETs and related resources.
 */
NTSTATUS InitializeTransferPackets(PDEVICE_OBJECT Fdo)
{
    PCOMMON_DEVICE_EXTENSION commonExt = Fdo->DeviceExtension;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    PSTORAGE_ADAPTER_DESCRIPTOR adapterDesc = commonExt->PartitionZeroExtension->AdapterDescriptor;
    ULONG hwMaxPages;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    /*
     *  Precompute the maximum transfer length
     */
    ASSERT(adapterDesc->MaximumTransferLength);

    hwMaxPages = adapterDesc->MaximumPhysicalPages ? adapterDesc->MaximumPhysicalPages-1 : 0;

    fdoData->HwMaxXferLen = MIN(adapterDesc->MaximumTransferLength, hwMaxPages << PAGE_SHIFT);
    fdoData->HwMaxXferLen = MAX(fdoData->HwMaxXferLen, PAGE_SIZE);

    fdoData->NumTotalTransferPackets = 0;
    fdoData->NumFreeTransferPackets = 0;
    InitializeSListHead(&fdoData->FreeTransferPacketsList);
    InitializeListHead(&fdoData->AllTransferPacketsList);
    InitializeListHead(&fdoData->DeferredClientIrpList);

    /*
     *  Set the packet threshold numbers based on the Windows SKU.
     */
    if (ExVerifySuite(Personal)){
        // this is Windows Personal
        MinWorkingSetTransferPackets = MIN_WORKINGSET_TRANSFER_PACKETS_Consumer;
        MaxWorkingSetTransferPackets = MAX_WORKINGSET_TRANSFER_PACKETS_Consumer;
    }
    else if (ExVerifySuite(Enterprise) || ExVerifySuite(DataCenter)){
        // this is Advanced Server or Datacenter
        MinWorkingSetTransferPackets = MIN_WORKINGSET_TRANSFER_PACKETS_Enterprise;
        MaxWorkingSetTransferPackets = MAX_WORKINGSET_TRANSFER_PACKETS_Enterprise;
    }
    else if (ExVerifySuite(TerminalServer)){
        // this is standard Server or Pro with terminal server
        MinWorkingSetTransferPackets = MIN_WORKINGSET_TRANSFER_PACKETS_Server;
        MaxWorkingSetTransferPackets = MAX_WORKINGSET_TRANSFER_PACKETS_Server;
    }
    else {
        // this is Professional without terminal server
        MinWorkingSetTransferPackets = MIN_WORKINGSET_TRANSFER_PACKETS_Consumer;
        MaxWorkingSetTransferPackets = MAX_WORKINGSET_TRANSFER_PACKETS_Consumer;
    }

    while (fdoData->NumFreeTransferPackets < MIN_INITIAL_TRANSFER_PACKETS){
        PTRANSFER_PACKET pkt = NewTransferPacket(Fdo);
        if (pkt){
            InterlockedIncrement(&fdoData->NumTotalTransferPackets);
            EnqueueFreeTransferPacket(Fdo, pkt);
        }
        else {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
    }
    fdoData->DbgPeakNumTransferPackets = fdoData->NumTotalTransferPackets;

    /*
     *  Pre-initialize our SCSI_REQUEST_BLOCK template with all
     *  the constant fields.  This will save a little time for each xfer.
     *  NOTE: a CdbLength field of 10 may not always be appropriate
     */
    RtlZeroMemory(&fdoData->SrbTemplate, sizeof(SCSI_REQUEST_BLOCK));
    fdoData->SrbTemplate.Length = sizeof(SCSI_REQUEST_BLOCK);
    fdoData->SrbTemplate.Function = SRB_FUNCTION_EXECUTE_SCSI;
    fdoData->SrbTemplate.QueueAction = SRB_SIMPLE_TAG_REQUEST;
    fdoData->SrbTemplate.SenseInfoBufferLength = sizeof(SENSE_DATA);
    fdoData->SrbTemplate.CdbLength = 10;

    return status;
}


VOID DestroyAllTransferPackets(PDEVICE_OBJECT Fdo)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    TRANSFER_PACKET *pkt;

    PAGED_CODE();

    ASSERT(IsListEmpty(&fdoData->DeferredClientIrpList));

    while (pkt = DequeueFreeTransferPacket(Fdo, FALSE)){
        DestroyTransferPacket(pkt);
        InterlockedDecrement(&fdoData->NumTotalTransferPackets);
    }

    ASSERT(fdoData->NumTotalTransferPackets == 0);
}


PTRANSFER_PACKET NewTransferPacket(PDEVICE_OBJECT Fdo)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    PTRANSFER_PACKET newPkt;

    newPkt = ExAllocatePoolWithTag(NonPagedPool, sizeof(TRANSFER_PACKET), 'pnPC');

    if (newPkt){
        RtlZeroMemory(newPkt, sizeof(TRANSFER_PACKET)); // just to be sure

        /*
         *  Allocate resources for the packet.
         */
        newPkt->Irp = IoAllocateIrp(Fdo->StackSize, FALSE);
        if (newPkt->Irp){
            KIRQL oldIrql;

            //
            // Allocate a MDL.  Add one page to the length to insure an extra page
            // entry is allocated if the buffer does not start on page boundaries.
            //
            newPkt->PartialMdl = IoAllocateMdl(NULL,
                                               fdoData->HwMaxXferLen + PAGE_SIZE,
                                               FALSE,
                                               FALSE,
                                               NULL);

            if (newPkt->PartialMdl) {

                ASSERT(newPkt->PartialMdl->Size >= (CSHORT)(sizeof(MDL) + BYTES_TO_PAGES(fdoData->HwMaxXferLen) * sizeof(PFN_NUMBER)));

                newPkt->Fdo = Fdo;

#if DBG
                newPkt->DbgPktId = InterlockedIncrement(&fdoData->DbgMaxPktId);
#endif

                /*
                 *  Enqueue the packet in our static AllTransferPacketsList
                 *  (just so we can find it during debugging if its stuck somewhere).
                 */
                KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);
                InsertTailList(&fdoData->AllTransferPacketsList, &newPkt->AllPktsListEntry);
                KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);

            } else {
                IoFreeIrp(newPkt->Irp);
                ExFreePool(newPkt);
                newPkt = NULL;
            }
        }
        else {
            ExFreePool(newPkt);
            newPkt = NULL;
        }
    }

    return newPkt;
}


/*
 *  DestroyTransferPacket
 *
 */
VOID DestroyTransferPacket(PTRANSFER_PACKET Pkt)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Pkt->Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    KIRQL oldIrql;

    ASSERT(!Pkt->SlistEntry.Next);
    ASSERT(!Pkt->OriginalIrp);

    KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);

    /*
     *  Delete the packet from our all-packets queue.
     */
    ASSERT(!IsListEmpty(&Pkt->AllPktsListEntry));
    ASSERT(!IsListEmpty(&fdoData->AllTransferPacketsList));
    RemoveEntryList(&Pkt->AllPktsListEntry);
    InitializeListHead(&Pkt->AllPktsListEntry);

    KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);

    IoFreeMdl(Pkt->PartialMdl);
    IoFreeIrp(Pkt->Irp);
    ExFreePool(Pkt);
}


VOID EnqueueFreeTransferPacket(PDEVICE_OBJECT Fdo, PTRANSFER_PACKET Pkt)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    KIRQL oldIrql;
    ULONG newNumPkts;

    ASSERT(!Pkt->SlistEntry.Next);

    InterlockedPushEntrySList(&fdoData->FreeTransferPacketsList, &Pkt->SlistEntry);
    newNumPkts = InterlockedIncrement(&fdoData->NumFreeTransferPackets);
    ASSERT(newNumPkts <= fdoData->NumTotalTransferPackets);

    /*
     *  If the total number of packets is larger than MinWorkingSetTransferPackets,
     *  that means that we've been in stress.  If all those packets are now
     *  free, then we are now out of stress and can free the extra packets.
     *  Free down to MaxWorkingSetTransferPackets immediately, and
     *  down to MinWorkingSetTransferPackets lazily (one at a time).
     */
    if (fdoData->NumFreeTransferPackets >= fdoData->NumTotalTransferPackets){

        /*
         *  1.  Immediately snap down to our UPPER threshold.
         */
        if (fdoData->NumTotalTransferPackets > MaxWorkingSetTransferPackets){
            SINGLE_LIST_ENTRY pktList;
            PSINGLE_LIST_ENTRY slistEntry;
            PTRANSFER_PACKET pktToDelete;

            DBGTRACE(ClassDebugTrace, ("Exiting stress, block freeing (%d-%d) packets.", fdoData->NumTotalTransferPackets, MaxWorkingSetTransferPackets));

            /*
             *  Check the counter again with lock held.  This eliminates a race condition
             *  while still allowing us to not grab the spinlock in the common codepath.
             *
             *  Note that the spinlock does not synchronize with threads dequeuing free
             *  packets to send (DequeueFreeTransferPacket does that with a lightweight
             *  interlocked exchange); the spinlock prevents multiple threads in this function
             *  from deciding to free too many extra packets at once.
             */
            SimpleInitSlistHdr(&pktList);
            KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);
            while ((fdoData->NumFreeTransferPackets >= fdoData->NumTotalTransferPackets) &&
                   (fdoData->NumTotalTransferPackets > MaxWorkingSetTransferPackets)){

                pktToDelete = DequeueFreeTransferPacket(Fdo, FALSE);
                if (pktToDelete){
                    SimplePushSlist(&pktList,
                                    (PSINGLE_LIST_ENTRY)&pktToDelete->SlistEntry);
                    InterlockedDecrement(&fdoData->NumTotalTransferPackets);
                }
                else {
                    DBGTRACE(ClassDebugTrace, ("Extremely unlikely condition (non-fatal): %d packets dequeued at once for Fdo %p. NumTotalTransferPackets=%d (1).", MaxWorkingSetTransferPackets, Fdo, fdoData->NumTotalTransferPackets));
                    break;
                }
            }
            KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);

            while (slistEntry = SimplePopSlist(&pktList)){
                pktToDelete = CONTAINING_RECORD(slistEntry, TRANSFER_PACKET, SlistEntry);
                DestroyTransferPacket(pktToDelete);
            }

        }

        /*
         *  2.  Lazily work down to our LOWER threshold (by only freeing one packet at a time).
         */
        if (fdoData->NumTotalTransferPackets > MinWorkingSetTransferPackets){
            /*
             *  Check the counter again with lock held.  This eliminates a race condition
             *  while still allowing us to not grab the spinlock in the common codepath.
             *
             *  Note that the spinlock does not synchronize with threads dequeuing free
             *  packets to send (DequeueFreeTransferPacket does that with a lightweight
             *  interlocked exchange); the spinlock prevents multiple threads in this function
             *  from deciding to free too many extra packets at once.
             */
            PTRANSFER_PACKET pktToDelete = NULL;

            DBGTRACE(ClassDebugTrace, ("Exiting stress, lazily freeing one of %d/%d packets.", fdoData->NumTotalTransferPackets, MinWorkingSetTransferPackets));

            KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);
            if ((fdoData->NumFreeTransferPackets >= fdoData->NumTotalTransferPackets) &&
                (fdoData->NumTotalTransferPackets > MinWorkingSetTransferPackets)){

                pktToDelete = DequeueFreeTransferPacket(Fdo, FALSE);
                if (pktToDelete){
                    InterlockedDecrement(&fdoData->NumTotalTransferPackets);
                }
                else {
                    DBGTRACE(ClassDebugTrace, ("Extremely unlikely condition (non-fatal): %d packets dequeued at once for Fdo %p. NumTotalTransferPackets=%d (2).", MinWorkingSetTransferPackets, Fdo, fdoData->NumTotalTransferPackets));
                }
            }
            KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);

            if (pktToDelete){
                DestroyTransferPacket(pktToDelete);
            }
        }

    }

}


PTRANSFER_PACKET DequeueFreeTransferPacket(PDEVICE_OBJECT Fdo, BOOLEAN AllocIfNeeded)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    PTRANSFER_PACKET pkt;
    PSLIST_ENTRY slistEntry;

    slistEntry = InterlockedPopEntrySList(&fdoData->FreeTransferPacketsList);
    if (slistEntry){
        slistEntry->Next = NULL;
        pkt = CONTAINING_RECORD(slistEntry, TRANSFER_PACKET, SlistEntry);
        InterlockedDecrement(&fdoData->NumFreeTransferPackets);
    }
    else {
        if (AllocIfNeeded){
            /*
             *  We are in stress and have run out of lookaside packets.
             *  In order to service the current transfer,
             *  allocate an extra packet.
             *  We will free it lazily when we are out of stress.
             */
            pkt = NewTransferPacket(Fdo);
            if (pkt){
                InterlockedIncrement(&fdoData->NumTotalTransferPackets);
                fdoData->DbgPeakNumTransferPackets = max(fdoData->DbgPeakNumTransferPackets, fdoData->NumTotalTransferPackets);
            }
            else {
                DBGWARN(("DequeueFreeTransferPacket: packet allocation failed"));
            }
        }
        else {
            pkt = NULL;
        }
    }

    return pkt;
}



/*
 *  SetupReadWriteTransferPacket
 *
 *        This function is called once to set up the first attempt to send a packet.
 *        It is not called before a retry, as SRB fields may be modified for the retry.
 *
 *      Set up the Srb of the TRANSFER_PACKET for the transfer.
 *        The Irp is set up in SubmitTransferPacket because it must be reset
 *        for each packet submission.
 */
VOID SetupReadWriteTransferPacket(  PTRANSFER_PACKET Pkt,
                                            PVOID Buf,
                                            ULONG Len,
                                            LARGE_INTEGER DiskLocation,
                                            PIRP OriginalIrp)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Pkt->Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    PIO_STACK_LOCATION origCurSp = IoGetCurrentIrpStackLocation(OriginalIrp);
    UCHAR majorFunc = origCurSp->MajorFunction;
    LARGE_INTEGER logicalBlockAddr;
    ULONG numTransferBlocks;
    PCDB pCdb;

    logicalBlockAddr.QuadPart = Int64ShrlMod32(DiskLocation.QuadPart, fdoExt->SectorShift);
    numTransferBlocks = Len >> fdoExt->SectorShift;

    /*
     *  Slap the constant SRB fields in from our pre-initialized template.
     *  We'll then only have to fill in the unique fields for this transfer.
     *  Tell lower drivers to sort the SRBs by the logical block address
     *  so that disk seeks are minimized.
     */
    Pkt->Srb = fdoData->SrbTemplate;    // copies _contents_ of SRB blocks
    Pkt->Srb.DataBuffer = Buf;
    Pkt->Srb.DataTransferLength = Len;
    Pkt->Srb.QueueSortKey = logicalBlockAddr.LowPart;
    if (logicalBlockAddr.QuadPart > 0xFFFFFFFF) {
        //
        // If the requested LBA is more than max ULONG set the
        // QueueSortKey to the maximum value, so that these
        // requests can be added towards the end of the queue.
        //

        Pkt->Srb.QueueSortKey = 0xFFFFFFFF;
    }
    Pkt->Srb.OriginalRequest = Pkt->Irp;
    Pkt->Srb.SenseInfoBuffer = &Pkt->SrbErrorSenseData;
    Pkt->Srb.TimeOutValue = (Len/0x10000) + ((Len%0x10000) ? 1 : 0);
    Pkt->Srb.TimeOutValue *= fdoExt->TimeOutValue;

    /*
     *  Arrange values in CDB in big-endian format.
     */
    pCdb = (PCDB)Pkt->Srb.Cdb;
    if (TEST_FLAG(fdoExt->DeviceFlags, DEV_USE_16BYTE_CDB)) {
        REVERSE_BYTES_QUAD(&pCdb->CDB16.LogicalBlock, &logicalBlockAddr);
        REVERSE_BYTES(&pCdb->CDB16.TransferLength, &numTransferBlocks);
        pCdb->CDB16.OperationCode = (majorFunc==IRP_MJ_READ) ? SCSIOP_READ16 : SCSIOP_WRITE16;
        Pkt->Srb.CdbLength = 16;
    } else {
        pCdb->CDB10.LogicalBlockByte0 = ((PFOUR_BYTE)&logicalBlockAddr.LowPart)->Byte3;
        pCdb->CDB10.LogicalBlockByte1 = ((PFOUR_BYTE)&logicalBlockAddr.LowPart)->Byte2;
        pCdb->CDB10.LogicalBlockByte2 = ((PFOUR_BYTE)&logicalBlockAddr.LowPart)->Byte1;
        pCdb->CDB10.LogicalBlockByte3 = ((PFOUR_BYTE)&logicalBlockAddr.LowPart)->Byte0;
        pCdb->CDB10.TransferBlocksMsb = ((PFOUR_BYTE)&numTransferBlocks)->Byte1;
        pCdb->CDB10.TransferBlocksLsb = ((PFOUR_BYTE)&numTransferBlocks)->Byte0;
        pCdb->CDB10.OperationCode = (majorFunc==IRP_MJ_READ) ? SCSIOP_READ : SCSIOP_WRITE;
    }

    /*
     *  Set SRB and IRP flags
     */
    Pkt->Srb.SrbFlags = fdoExt->SrbFlags;
    if (TEST_FLAG(OriginalIrp->Flags, IRP_PAGING_IO) ||
        TEST_FLAG(OriginalIrp->Flags, IRP_SYNCHRONOUS_PAGING_IO)){
        SET_FLAG(Pkt->Srb.SrbFlags, SRB_CLASS_FLAGS_PAGING);
    }
    SET_FLAG(Pkt->Srb.SrbFlags, (majorFunc==IRP_MJ_READ) ? SRB_FLAGS_DATA_IN : SRB_FLAGS_DATA_OUT);

    /*
     *  Allow caching only if this is not a write-through request.
     *  If write-through and caching is enabled on the device, force
     *  media access.
     *  Ignore SL_WRITE_THROUGH for reads; it's only set because the file handle was opened with WRITE_THROUGH.
     */
    if (TEST_FLAG(origCurSp->Flags, SL_WRITE_THROUGH) && (majorFunc != IRP_MJ_READ))
    {
        if (TEST_FLAG(fdoExt->DeviceFlags, DEV_WRITE_CACHE) &&
            !TEST_FLAG(fdoExt->DeviceFlags, DEV_POWER_PROTECTED) &&
            !TEST_FLAG(fdoExt->ScanForSpecialFlags, CLASS_SPECIAL_FUA_NOT_SUPPORTED) )
        {
            pCdb->CDB10.ForceUnitAccess = TRUE;
        }
    }
    else {
        SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_ADAPTER_CACHE_ENABLE);
    }

    /*
     *  Remember the buf and len in the SRB because miniports
     *  can overwrite SRB.DataTransferLength and we may need it again
     *  for the retry.
     */
    Pkt->BufPtrCopy = Buf;
    Pkt->BufLenCopy = Len;
    Pkt->TargetLocationCopy = DiskLocation;

    Pkt->OriginalIrp = OriginalIrp;
    Pkt->NumRetries = NUM_IO_RETRIES;
    Pkt->SyncEventPtr = NULL;
    Pkt->CompleteOriginalIrpWhenLastPacketCompletes = TRUE;

    DBGLOGFLUSHINFO(fdoData, TRUE, (BOOLEAN)(pCdb->CDB10.ForceUnitAccess), FALSE);
}


/*
 *  SubmitTransferPacket
 *
 *        Set up the IRP for the TRANSFER_PACKET submission and send it down.
 */
NTSTATUS SubmitTransferPacket(PTRANSFER_PACKET Pkt)
{
    PCOMMON_DEVICE_EXTENSION commonExtension = Pkt->Fdo->DeviceExtension;
    PDEVICE_OBJECT nextDevObj = commonExtension->LowerDeviceObject;
    PIO_STACK_LOCATION nextSp;

    ASSERT(Pkt->Irp->CurrentLocation == Pkt->Irp->StackCount+1);

    /*
     *  Attach the SRB to the IRP.
     *  The reused IRP's stack location has to be rewritten for each retry
     *  call because IoCompleteRequest clears the stack locations.
     */
    IoReuseIrp(Pkt->Irp, STATUS_NOT_SUPPORTED);
    nextSp = IoGetNextIrpStackLocation(Pkt->Irp);
    nextSp->MajorFunction = IRP_MJ_SCSI;
    nextSp->Parameters.Scsi.Srb = &Pkt->Srb;
    Pkt->Srb.ScsiStatus = Pkt->Srb.SrbStatus = 0;
    Pkt->Srb.SenseInfoBufferLength = sizeof(SENSE_DATA);
    if (Pkt->CompleteOriginalIrpWhenLastPacketCompletes){
        /*
         *  Only dereference the "original IRP"'s stack location
         *  if its a real client irp (as opposed to a static irp
         *  we're using just for result status for one of the non-IO scsi commands).
         *
         *  For read/write, propagate the storage-specific IRP stack location flags
         *  (e.g. SL_OVERRIDE_VERIFY_VOLUME, SL_WRITE_THROUGH).
         */
        PIO_STACK_LOCATION origCurSp = IoGetCurrentIrpStackLocation(Pkt->OriginalIrp);
        nextSp->Flags = origCurSp->Flags;
    }

    //
    // If the request is not split, we can use the original IRP MDL.  If the
    // request needs to be split, we need to use a partial MDL.  The partial MDL
    // is needed because more than one driver might be mapping the same MDL
    // and this causes problems.
    //
    if (Pkt->UsePartialMdl == FALSE) {
        Pkt->Irp->MdlAddress = Pkt->OriginalIrp->MdlAddress;
    } else {
        IoBuildPartialMdl(Pkt->OriginalIrp->MdlAddress, Pkt->PartialMdl, Pkt->Srb.DataBuffer, Pkt->Srb.DataTransferLength);
        Pkt->Irp->MdlAddress = Pkt->PartialMdl;
    }

    DBGLOGSENDPACKET(Pkt);

    IoSetCompletionRoutine(Pkt->Irp, TransferPktComplete, Pkt, TRUE, TRUE, TRUE);
    return IoCallDriver(nextDevObj, Pkt->Irp);
}


NTSTATUS TransferPktComplete(IN PDEVICE_OBJECT NullFdo, IN PIRP Irp, IN PVOID Context)
{
    PTRANSFER_PACKET pkt = (PTRANSFER_PACKET)Context;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = pkt->Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    PIO_STACK_LOCATION origCurrentSp = IoGetCurrentIrpStackLocation(pkt->OriginalIrp);
    BOOLEAN packetDone = FALSE;

    /*
     *  Put all the assertions and spew in here so we don't have to look at them.
     */
    DBGLOGRETURNPACKET(pkt);
    DBGCHECKRETURNEDPKT(pkt);

    //
    // If partial MDL was used, unmap the pages.  When the packet is retried, the
    // MDL will be recreated.  If the packet is done, the MDL will be ready to be reused.
    //
    if (pkt->UsePartialMdl) {
        MmPrepareMdlForReuse(pkt->PartialMdl);
    }

    if (SRB_STATUS(pkt->Srb.SrbStatus) == SRB_STATUS_SUCCESS){

        fdoData->LoggedTURFailureSinceLastIO = FALSE;

        /*
         *  The port driver should not have allocated a sense buffer
         *  if the SRB succeeded.
         */
        ASSERT(!PORT_ALLOCATED_SENSE(fdoExt, &pkt->Srb));

        /*
         *  Add this packet's transferred length to the original IRP's.
         */
        InterlockedExchangeAdd((PLONG)&pkt->OriginalIrp->IoStatus.Information,
                              (LONG)pkt->Srb.DataTransferLength);

        if (pkt->InLowMemRetry){
            packetDone = StepLowMemRetry(pkt);
        }
        else {
            packetDone = TRUE;
        }

    }
    else {
        /*
         *  The packet failed.  We may retry it if possible.
         */
        BOOLEAN shouldRetry;

        /*
         *  Make sure IRP status matches SRB error status (since we propagate it).
         */
        if (NT_SUCCESS(Irp->IoStatus.Status)){
            Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        }

        /*
         *  The packet failed.
         *  So when sending the packet down we either saw either an error or STATUS_PENDING,
         *  and so we returned STATUS_PENDING for the original IRP.
         *  So now we must mark the original irp pending to match that, _regardless_ of
         *  whether we actually switch threads here by retrying.
         *  (We also have to mark the irp pending if the lower driver marked the irp pending;
         *   that is dealt with farther down).
         */
        if (pkt->CompleteOriginalIrpWhenLastPacketCompletes){
            IoMarkIrpPending(pkt->OriginalIrp);
        }

        /*
         *  Interpret the SRB error (to a meaningful IRP status)
         *  and determine if we should retry this packet.
         *  This call looks at the returned SENSE info to figure out what to do.
         */
        shouldRetry = InterpretTransferPacketError(pkt);

        /*
         *  Sometimes the port driver can allocates a new 'sense' buffer
         *  to report transfer errors, e.g. when the default sense buffer
         *  is too small.  If so, it is up to us to free it.
         *  Now that we're done interpreting the sense info, free it if appropriate.
         *  Then clear the sense buffer so it doesn't pollute future errors returned in this packet.
         */
        if (PORT_ALLOCATED_SENSE(fdoExt, &pkt->Srb)) {
            DBGTRACE(ClassDebugSenseInfo, ("Freeing port-allocated sense buffer for pkt %ph.", pkt));
            FREE_PORT_ALLOCATED_SENSE_BUFFER(fdoExt, &pkt->Srb);
            pkt->Srb.SenseInfoBuffer = &pkt->SrbErrorSenseData;
            pkt->Srb.SenseInfoBufferLength = sizeof(SENSE_DATA);
        }
        else {
            ASSERT(pkt->Srb.SenseInfoBuffer == &pkt->SrbErrorSenseData);
            ASSERT(pkt->Srb.SenseInfoBufferLength <= sizeof(SENSE_DATA));
        }
        RtlZeroMemory(&pkt->SrbErrorSenseData, sizeof(SENSE_DATA));

        /*
         *  If the SRB queue is locked-up, release it.
         *  Do this after calling the error handler.
         */
        if (pkt->Srb.SrbStatus & SRB_STATUS_QUEUE_FROZEN){
            ClassReleaseQueue(pkt->Fdo);
        }

        if (NT_SUCCESS(Irp->IoStatus.Status)){
            /*
             *  The error was recovered above in the InterpretTransferPacketError() call.
             */

            ASSERT(!shouldRetry);

            /*
             *  In the case of a recovered error,
             *  add the transfer length to the original Irp as we would in the success case.
             */
            InterlockedExchangeAdd((PLONG)&pkt->OriginalIrp->IoStatus.Information,
                                  (LONG)pkt->Srb.DataTransferLength);

            if (pkt->InLowMemRetry){
                packetDone = StepLowMemRetry(pkt);
            }
            else {
                packetDone = TRUE;
            }
        }
        else {
            if (shouldRetry && (pkt->NumRetries > 0)){
                packetDone = RetryTransferPacket(pkt);
            }
            else {
                packetDone = TRUE;
            }
        }
    }

    /*
     *  If the packet is completed, put it back in the free list.
     *  If it is the last packet servicing the original request, complete the original irp.
     */
    if (packetDone){
        LONG numPacketsRemaining;
        PIRP deferredIrp;
        PDEVICE_OBJECT Fdo = pkt->Fdo;
        UCHAR uniqueAddr;

        /*
         *  In case a remove is pending, bump the lock count so we don't get freed
         *  right after we complete the original irp.
         */
        ClassAcquireRemoveLock(Fdo, (PIRP)&uniqueAddr);

        /*
         *  The original IRP should get an error code
         *  if any one of the packets failed.
         */
        if (!NT_SUCCESS(Irp->IoStatus.Status)){
            pkt->OriginalIrp->IoStatus.Status = Irp->IoStatus.Status;

            /*
             *  If the original I/O originated in user space (i.e. it is thread-queued),
             *  and the error is user-correctable (e.g. media is missing, for removable media),
             *  alert the user.
             *  Since this is only one of possibly several packets completing for the original IRP,
             *  we may do this more than once for a single request.  That's ok; this allows
             *  us to test each returned status with IoIsErrorUserInduced().
             */
            if (IoIsErrorUserInduced(Irp->IoStatus.Status) &&
                pkt->CompleteOriginalIrpWhenLastPacketCompletes &&
                pkt->OriginalIrp->Tail.Overlay.Thread){

                IoSetHardErrorOrVerifyDevice(pkt->OriginalIrp, Fdo);
            }
        }

        /*
         *  We use a field in the original IRP to count
         *  down the transfer pieces as they complete.
         */
        numPacketsRemaining = InterlockedDecrement(
            (PLONG)&pkt->OriginalIrp->Tail.Overlay.DriverContext[0]);

        if (numPacketsRemaining > 0){
            /*
             *  More transfer pieces remain for the original request.
             *  Wait for them to complete before completing the original irp.
             */
        }
        else {

            /*
             *  All the transfer pieces are done.
             *  Complete the original irp if appropriate.
             */
            ASSERT(numPacketsRemaining == 0);
            if (pkt->CompleteOriginalIrpWhenLastPacketCompletes){

                IO_PAGING_PRIORITY priority = (TEST_FLAG(pkt->OriginalIrp->Flags, IRP_PAGING_IO)) ? IoGetPagingIoPriority(pkt->OriginalIrp) : IoPagingPriorityInvalid;
                KIRQL oldIrql;

                if (NT_SUCCESS(pkt->OriginalIrp->IoStatus.Status)){
                    ASSERT((ULONG)pkt->OriginalIrp->IoStatus.Information == origCurrentSp->Parameters.Read.Length);
                    ClasspPerfIncrementSuccessfulIo(fdoExt);
                }
                ClassReleaseRemoveLock(Fdo, pkt->OriginalIrp);

                /*
                 *  We submitted all the downward irps, including this last one, on the thread
                 *  that the OriginalIrp came in on.  So the OriginalIrp is completing on a
                 *  different thread iff this last downward irp is completing on a different thread.
                 *  If BlkCache is loaded, for example, it will often complete
                 *  requests out of the cache on the same thread, therefore not marking the downward
                 *  irp pending and not requiring us to do so here.  If the downward request is completing
                 *  on the same thread, then by not marking the OriginalIrp pending we can save an APC
                 *  and get extra perf benefit out of BlkCache.
                 *  Note that if the packet ever cycled due to retry or LowMemRetry,
                 *  we set the pending bit in those codepaths.
                 */
                if (pkt->Irp->PendingReturned){
                    IoMarkIrpPending(pkt->OriginalIrp);
                }

                ClassCompleteRequest(Fdo, pkt->OriginalIrp, IO_DISK_INCREMENT);

                //
                // Drop the count only after completing the request, to give
                // Mm some amount of time to issue its next critical request
                //

                if (priority == IoPagingPriorityHigh)
                {
                    KeAcquireSpinLock(&fdoData->SpinLock, &oldIrql);

                    if (fdoData->MaxInterleavedNormalIo < ClassMaxInterleavePerCriticalIo)
                    {
                        fdoData->MaxInterleavedNormalIo = 0;
                    }
                    else
                    {
                        fdoData->MaxInterleavedNormalIo -= ClassMaxInterleavePerCriticalIo;
                    }

                    fdoData->NumHighPriorityPagingIo--;

                    if (fdoData->NumHighPriorityPagingIo == 0)
                    {
                        LARGE_INTEGER period;

                        //
                        // Exiting throttle mode
                        //

                        KeQuerySystemTime(&fdoData->ThrottleStopTime);

                        period.QuadPart = fdoData->ThrottleStopTime.QuadPart - fdoData->ThrottleStartTime.QuadPart;
                        fdoData->LongestThrottlePeriod.QuadPart = max(fdoData->LongestThrottlePeriod.QuadPart, period.QuadPart);
                    }

                    KeReleaseSpinLock(&fdoData->SpinLock, oldIrql);
                }

                /*
                 *  We may have been called by one of the class drivers (e.g. cdrom)
                 *  via the legacy API ClassSplitRequest.
                 *  This is the only case for which the packet engine is called for an FDO
                 *  with a StartIo routine; in that case, we have to call IoStartNextPacket
                 *  now that the original irp has been completed.
                 */
                if (fdoExt->CommonExtension.DriverExtension->InitData.ClassStartIo) {
                    if (TEST_FLAG(pkt->Srb.SrbFlags, SRB_FLAGS_DONT_START_NEXT_PACKET)){
                        DBGTRAP(("SRB_FLAGS_DONT_START_NEXT_PACKET should never be set here (??)"));
                    }
                    else {
                        KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
                        IoStartNextPacket(Fdo, FALSE);
                        KeLowerIrql(oldIrql);
                    }
                }
            }
        }

        /*
         *  If the packet was synchronous, write the final result back to the issuer's status buffer
         *  and signal his event.
         */
        if (pkt->SyncEventPtr){
            KeSetEvent(pkt->SyncEventPtr, 0, FALSE);
            pkt->SyncEventPtr = NULL;
        }

        /*
         *  Free the completed packet.
         */
        pkt->UsePartialMdl = FALSE;
        pkt->OriginalIrp = NULL;
        pkt->InLowMemRetry = FALSE;
        EnqueueFreeTransferPacket(Fdo, pkt);

        /*
         *  Now that we have freed some resources,
         *  try again to send one of the previously deferred irps.
         */
        deferredIrp = DequeueDeferredClientIrp(fdoData);
        if (deferredIrp){
            ServiceTransferRequest(Fdo, deferredIrp);
        }

        ClassReleaseRemoveLock(Fdo, (PIRP)&uniqueAddr);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}


/*
 *  SetupEjectionTransferPacket
 *
 *      Set up a transferPacket for a synchronous Ejection Control transfer.
 */
VOID SetupEjectionTransferPacket(   TRANSFER_PACKET *Pkt,
                                        BOOLEAN PreventMediaRemoval,
                                        PKEVENT SyncEventPtr,
                                        PIRP OriginalIrp)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Pkt->Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    PCDB pCdb;

    PAGED_CODE();

    RtlZeroMemory(&Pkt->Srb, sizeof(SCSI_REQUEST_BLOCK));

    Pkt->Srb.Length = sizeof(SCSI_REQUEST_BLOCK);
    Pkt->Srb.Function = SRB_FUNCTION_EXECUTE_SCSI;
    Pkt->Srb.QueueAction = SRB_SIMPLE_TAG_REQUEST;
    Pkt->Srb.CdbLength = 6;
    Pkt->Srb.OriginalRequest = Pkt->Irp;
    Pkt->Srb.SenseInfoBuffer = &Pkt->SrbErrorSenseData;
    Pkt->Srb.SenseInfoBufferLength = sizeof(SENSE_DATA);
    Pkt->Srb.TimeOutValue = fdoExt->TimeOutValue;

    Pkt->Srb.SrbFlags = fdoExt->SrbFlags;
    SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
    SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE);

    pCdb = (PCDB)Pkt->Srb.Cdb;
    pCdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
    pCdb->MEDIA_REMOVAL.Prevent = PreventMediaRemoval;

    Pkt->BufPtrCopy = NULL;
    Pkt->BufLenCopy = 0;

    Pkt->OriginalIrp = OriginalIrp;
    Pkt->NumRetries = NUM_LOCKMEDIAREMOVAL_RETRIES;
    Pkt->SyncEventPtr = SyncEventPtr;
    Pkt->CompleteOriginalIrpWhenLastPacketCompletes = FALSE;
}


/*
 *  SetupModeSenseTransferPacket
 *
 *      Set up a transferPacket for a synchronous Mode Sense transfer.
 */
VOID SetupModeSenseTransferPacket(   TRANSFER_PACKET *Pkt,
                                        PKEVENT SyncEventPtr,
                                        PVOID ModeSenseBuffer,
                                        UCHAR ModeSenseBufferLen,
                                        UCHAR PageMode,
                                        PIRP OriginalIrp)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Pkt->Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    PCDB pCdb;

    PAGED_CODE();

    RtlZeroMemory(&Pkt->Srb, sizeof(SCSI_REQUEST_BLOCK));

    Pkt->Srb.Length = sizeof(SCSI_REQUEST_BLOCK);
    Pkt->Srb.Function = SRB_FUNCTION_EXECUTE_SCSI;
    Pkt->Srb.QueueAction = SRB_SIMPLE_TAG_REQUEST;
    Pkt->Srb.CdbLength = 6;
    Pkt->Srb.OriginalRequest = Pkt->Irp;
    Pkt->Srb.SenseInfoBuffer = &Pkt->SrbErrorSenseData;
    Pkt->Srb.SenseInfoBufferLength = sizeof(SENSE_DATA);
    Pkt->Srb.TimeOutValue = fdoExt->TimeOutValue;
    Pkt->Srb.DataBuffer = ModeSenseBuffer;
    Pkt->Srb.DataTransferLength = ModeSenseBufferLen;

    Pkt->Srb.SrbFlags = fdoExt->SrbFlags;
    SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_DATA_IN);
    SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
    SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE);

    pCdb = (PCDB)Pkt->Srb.Cdb;
    pCdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    pCdb->MODE_SENSE.PageCode = PageMode;
    pCdb->MODE_SENSE.AllocationLength = (UCHAR)ModeSenseBufferLen;

    Pkt->BufPtrCopy = ModeSenseBuffer;
    Pkt->BufLenCopy = ModeSenseBufferLen;

    Pkt->OriginalIrp = OriginalIrp;
    Pkt->NumRetries = NUM_MODESENSE_RETRIES;
    Pkt->SyncEventPtr = SyncEventPtr;
    Pkt->CompleteOriginalIrpWhenLastPacketCompletes = FALSE;
}


/*
 *  SetupDriveCapacityTransferPacket
 *
 *      Set up a transferPacket for a synchronous Drive Capacity transfer.
 */
VOID SetupDriveCapacityTransferPacket(   TRANSFER_PACKET *Pkt,
                                        PVOID ReadCapacityBuffer,
                                        ULONG ReadCapacityBufferLen,
                                        PKEVENT SyncEventPtr,
                                        PIRP OriginalIrp,
                                        BOOLEAN Use16ByteCdb)
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Pkt->Fdo->DeviceExtension;
    PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
    PCDB pCdb;

    RtlZeroMemory(&Pkt->Srb, sizeof(SCSI_REQUEST_BLOCK));

    Pkt->Srb.Length = sizeof(SCSI_REQUEST_BLOCK);
    Pkt->Srb.Function = SRB_FUNCTION_EXECUTE_SCSI;
    Pkt->Srb.QueueAction = SRB_SIMPLE_TAG_REQUEST;
    Pkt->Srb.OriginalRequest = Pkt->Irp;
    Pkt->Srb.SenseInfoBuffer = &Pkt->SrbErrorSenseData;
    Pkt->Srb.SenseInfoBufferLength = sizeof(SENSE_DATA);
    Pkt->Srb.TimeOutValue = fdoExt->TimeOutValue;
    Pkt->Srb.DataBuffer = ReadCapacityBuffer;
    Pkt->Srb.DataTransferLength = ReadCapacityBufferLen;

    Pkt->Srb.SrbFlags = fdoExt->SrbFlags;
    SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_DATA_IN);
    SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
    SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE);

    pCdb = (PCDB)Pkt->Srb.Cdb;
    if (Use16ByteCdb == TRUE) {
        ASSERT(ReadCapacityBufferLen >= sizeof(READ_CAPACITY_DATA_EX));
        Pkt->Srb.CdbLength = 16;
        pCdb->CDB16.OperationCode = SCSIOP_READ_CAPACITY16;
        REVERSE_BYTES(&pCdb->CDB16.TransferLength, &ReadCapacityBufferLen);
        pCdb->AsByte[1] = 0x10; // Service Action
    } else {
        Pkt->Srb.CdbLength = 10;
        pCdb->CDB10.OperationCode = SCSIOP_READ_CAPACITY;
    }

    Pkt->BufPtrCopy = ReadCapacityBuffer;
    Pkt->BufLenCopy = ReadCapacityBufferLen;

    Pkt->OriginalIrp = OriginalIrp;
    Pkt->NumRetries = NUM_DRIVECAPACITY_RETRIES;
    Pkt->SyncEventPtr = SyncEventPtr;
    Pkt->CompleteOriginalIrpWhenLastPacketCompletes = FALSE;
}


#if 0
    /*
     *  SetupSendStartUnitTransferPacket
     *
     *      Set up a transferPacket for a synchronous Send Start Unit transfer.
     */
    VOID SetupSendStartUnitTransferPacket(   TRANSFER_PACKET *Pkt,
                                                    PIRP OriginalIrp)
    {
        PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Pkt->Fdo->DeviceExtension;
        PCLASS_PRIVATE_FDO_DATA fdoData = fdoExt->PrivateFdoData;
        PCDB pCdb;

        PAGED_CODE();

        RtlZeroMemory(&Pkt->Srb, sizeof(SCSI_REQUEST_BLOCK));

        /*
         *  Initialize the SRB.
         *  Use a very long timeout value to give the drive time to spin up.
         */
        Pkt->Srb.Length = sizeof(SCSI_REQUEST_BLOCK);
        Pkt->Srb.Function = SRB_FUNCTION_EXECUTE_SCSI;
        Pkt->Srb.TimeOutValue = START_UNIT_TIMEOUT;
        Pkt->Srb.CdbLength = 6;
        Pkt->Srb.OriginalRequest = Pkt->Irp;
        Pkt->Srb.SenseInfoBuffer = &Pkt->SrbErrorSenseData;
        Pkt->Srb.SenseInfoBufferLength = sizeof(SENSE_DATA);
        Pkt->Srb.Lun = 0;

        SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_NO_DATA_TRANSFER);
        SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_DISABLE_AUTOSENSE);
        SET_FLAG(Pkt->Srb.SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);

        pCdb = (PCDB)Pkt->Srb.Cdb;
        pCdb->START_STOP.OperationCode = SCSIOP_START_STOP_UNIT;
        pCdb->START_STOP.Start = 1;
        pCdb->START_STOP.Immediate = 0;
        pCdb->START_STOP.LogicalUnitNumber = 0;

        Pkt->OriginalIrp = OriginalIrp;
        Pkt->NumRetries = 0;
        Pkt->SyncEventPtr = NULL;
        Pkt->CompleteOriginalIrpWhenLastPacketCompletes = FALSE;
    }
#endif


