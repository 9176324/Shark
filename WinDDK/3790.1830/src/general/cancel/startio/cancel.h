#ifndef __CANCEL_H
#define __CANCEL_H

#include <initguid.h>

//
// Since this driver is a legacy driver and gets installed as a service 
// (without an INF file),  we will define a class guid for use in 
// IoCreateDeviceSecure function. This would allow  the system to store
// Security, DeviceType, Characteristics and Exclusivity information of the
// deviceobject in the registery under 
// HKLM\SYSTEM\CurrentControlSet\Control\Class\ClassGUID\Properties. 
// This information can be overrided by an Administrators giving them the ability
// to control access to the device beyond what is initially allowed 
// by the driver developer.
//
 

// {5D006E1A-2631-466c-B8A0-32FD498E4424}  - generated using guidgen.exe
DEFINE_GUID (GUID_DEVCLASS_CANCEL_SAMPLE,
        0x5d006e1a, 0x2631, 0x466c, 0xb8, 0xa0, 0x32, 0xfd, 0x49, 0x8e, 0x44, 0x24);

//
// GUID definition are required to be outside of header inclusion pragma to 
// avoid error during precompiled headers.
//
#include <ntddk.h>
//
// Include this header file and link with csq.lib to use this 
// sample on Win2K, XP and above.
//
#include <csq.h> 
#include <wdmsec.h> // for IoCreateDeviceSecure

//  Debugging macros

#if DBG
#define CSAMP_KDPRINT(_x_) \
                DbgPrint("CANCEL.SYS: ");\
                DbgPrint _x_;

#define TRAP()  DbgBreakPoint()

#else

#define CSAMP_KDPRINT(_x_)

#define TRAP()


#endif



#define CSAMP_DEVICE_NAME_U     L"\\Device\\CANCELSAMP"
#define CSAMP_DOS_DEVICE_NAME_U L"\\DosDevices\\CancelSamp"
#define CSAMP_RETRY_INTERVAL    500*1000 //500 ms

typedef struct _INPUT_DATA{

    ULONG Data; //device data is stored here

} INPUT_DATA, *PINPUT_DATA;

typedef struct _DEVICE_EXTENSION{

  
    // Irps waiting to be processed are queued here
    LIST_ENTRY   PendingIrpQueue;

    //  SpinLock to protect access to the queue
    KSPIN_LOCK QueueLock;

    // SpinLock to provide exclusive access to the port
    KSPIN_LOCK DeviceLock;

    // Pointer to current device IRP. Exclusive access to this
    // field is also provided by the QueueLock.
    PIRP CurrentIrp;

    // Customtimer DPC object
    KDPC PollingDpc;

    // Time at which the device was last polled
    LARGE_INTEGER LastPollTime;  

    // Polling timer object
    KTIMER  PollingTimer;

    // Polling interval (retry interval)
    LARGE_INTEGER PollingInterval;

    IO_CSQ CancelSafeQueue;   
        
}  DEVICE_EXTENSION, *PDEVICE_EXTENSION;

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING registryPath
);

NTSTATUS
CsampCreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
CsampUnload(
    IN PDRIVER_OBJECT DriverObject
    );

VOID
CsampPollingTimerDpc(
    IN PKDPC Dpc,
    IN PVOID Context,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
);

NTSTATUS
CsampRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
);

NTSTATUS
CsampCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
);

NTSTATUS
CsampPollDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP    Irp
    );

VOID 
CsampInitiateIo(
   IN PDEVICE_OBJECT DeviceObject
);

NTSTATUS 
CsampInsertIrp (
    IN PIO_CSQ   Csq,
    IN PIRP              Irp,
    IN PVOID    InsertContext
    );

VOID 
CsampRemoveIrp(
    IN  PIO_CSQ Csq,
    IN  PIRP    Irp
    );

PIRP 
CsampPeekNextIrp(
    IN  PIO_CSQ Csq,
    IN  PIRP    Irp,
    IN  PVOID  PeekContext
    );

VOID 
CsampAcquireLock(
    IN  PIO_CSQ Csq,
    OUT PKIRQL  Irql
    );

VOID 
CsampReleaseLock(
    IN PIO_CSQ Csq,
    IN KIRQL   Irql
    );

VOID 
CsampCompleteCanceledIrp(
    IN  PIO_CSQ             pCsq,
    IN  PIRP                Irp
    );

#endif


