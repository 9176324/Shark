
#include <ndis.h>

#include "dmautil.h"

BOOLEAN
IsIrqlGreaterThanDispatch(
    VOID
    );



VOID
InitializeDmaUtil(
    PDMA_UTIL      DmaUtil,
    NDIS_HANDLE    DmaHandle
    )

{
    RtlZeroMemory(
        DmaUtil,
        sizeof(*DmaUtil)
        );

    DmaUtil->NdisDmaHandle=DmaHandle;

    return;

}



NTSTATUS
StartDmaTransfer(
    PDMA_UTIL     DmaUtil,
    PNDIS_BUFFER  Buffer,
    ULONG         Offset,
    ULONG         Length,
    BOOLEAN       ToDevice
    )

{
    NDIS_STATUS    Status;

    if (IsIrqlGreaterThanDispatch()) {

#if DBG
        DbgPrint("IR-DMAUTIL: Transfer started at raised IRQL\n");
#endif
        ASSERT(0);
        return STATUS_UNSUCCESSFUL;
    }


    if (DmaUtil->Buffer != NULL) {

#if DBG
        DbgPrint("IR-DMAUTIL: Transfer started when one is already in progress\n");
#endif
        ASSERT(0);
        return STATUS_UNSUCCESSFUL;
    }

    DmaUtil->Buffer=Buffer;
    DmaUtil->Offset=Offset;
    DmaUtil->Length=Length;
    DmaUtil->Direction=ToDevice;


    NdisMSetupDmaTransfer(
        &Status,
        DmaUtil->NdisDmaHandle,
        Buffer,
        Offset,
        Length,
        ToDevice
        );

    if (Status != STATUS_SUCCESS) {
#if DBG
        DbgPrint("IR-DMAUTIL: NdisMSetupDmaTransfer() failed %08lx\n",Status);
#endif
        DmaUtil->Buffer=NULL;
    }

    return Status;

}



NTSTATUS
CompleteDmaTransfer(
    PDMA_UTIL    DmaUtil,
    BOOLEAN      ToDevice
    )

{
    NDIS_STATUS    Status;

    if (IsIrqlGreaterThanDispatch()) {
#if DBG
        DbgPrint("IR-DMAUTIL: CompleteTransfer called at raised IRQL\n");
#endif
        ASSERT(0);
        return STATUS_UNSUCCESSFUL;
    }


    if (DmaUtil->Buffer == NULL) {
#if DBG
        DbgPrint("IR-DMAUTIL: CompleteTransfer called when no transfer it active\n");
#endif
        ASSERT(0);
        return STATUS_UNSUCCESSFUL;
    }

    if (ToDevice != DmaUtil->Direction) {
#if DBG
        DbgPrint("IR-DMAUTIL: CompleteTransfer called for the wrong direction of transfer\n");
#endif
        ASSERT(0);
        return STATUS_UNSUCCESSFUL;
    }

    NdisMCompleteDmaTransfer(
        &Status,
        DmaUtil->NdisDmaHandle,
        DmaUtil->Buffer,
        DmaUtil->Offset,
        DmaUtil->Length,
        DmaUtil->Direction
        );

    if (Status != STATUS_SUCCESS) {
#if DBG
        DbgPrint("IR-DMAUTIL: NdisMCompleteDmaTransfer() failed %08lx\n",Status);
#endif
    }

    DmaUtil->Buffer=NULL;

    return Status;

}

