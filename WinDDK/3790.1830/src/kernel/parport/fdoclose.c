#include "pch.h"

NTSTATUS
PptFdoClose(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
{
    PFDO_EXTENSION   fdx = DeviceObject->DeviceExtension;

    PAGED_CODE();

    //
    // Always succeed an IRP_MJ_CLOSE.
    //

    //
    // Keep running count of CREATE requests vs CLOSE requests.
    //
    ExAcquireFastMutex( &fdx->OpenCloseMutex );
    if( fdx->OpenCloseRefCount > 0 ) {
        //
        // prevent rollover -  strange as it may seem, it is perfectly
        //   legal for us to receive more closes than creates - this
        //   info came directly from Mr. PnP himself
        //
        if( ((LONG)InterlockedDecrement( &fdx->OpenCloseRefCount )) < 0 ) {
            // handle underflow
            InterlockedIncrement( &fdx->OpenCloseRefCount );
        }
    }
    ExReleaseFastMutex( &fdx->OpenCloseMutex );
    
    DD((PCE)fdx,DDT,"PptFdoClose - OpenCloseRefCount after close = %d\n", fdx->OpenCloseRefCount);

    return P4CompleteRequest( Irp, STATUS_SUCCESS, 0 );
}

