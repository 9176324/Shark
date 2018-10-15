
/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.
    
Module Name:

    PciDrv.h

Abstract:

    Header file for the driver modules.

Environment:

    Kernel mode

Revision History:

    Eliyas Yakub Feb 13, 2003

--*/


#if !defined(_PCIDRV_H_)
#define _PCIDRV_H_


//
// Let us use newly introduced (.NET DDK) safe string function to avoid 
// security issues related buffer overrun. 
// The advantages of the RtlStrsafe functions include: 
// 1) The size of the destination buffer is always provided to the 
// function to ensure that the function does not write past the end of
// the buffer.
// 2) Buffers are guaranteed to be null-terminated, even if the 
// operation truncates the intended result.
// 

//
// In this driver we are using a safe version vsnprintf, which is 
// RtlStringCbVPrintfA. To use strsafe function on 9x, ME, and Win2K Oses, we 
// have to define NTSTRSAFE_LIB before including this header file and explicitly
// link to ntstrsafe.lib. If your driver is just target for XP and above, there is
// no define NTSTRSAFE_LIB and link to the lib.
// 
#define NTSTRSAFE_LIB
#include <ntstrsafe.h>

//
// InterlockedOr is not defined in Win2K header files. 
// If not defined, we will define it to use intrinsic function.
// 
#if !defined(InterlockedOr) && (_WIN32_WINNT==0x0500)
#define InterlockedOr _InterlockedOr
#endif

#define PCIDRV_POOL_TAG (ULONG) 'DICP'
#define PCIDRV_FDO_INSTANCE_SIGNATURE (ULONG) 'odFT'

#define PCIDRV_WAIT_WAKE_ENABLE L"WaitWakeEnabled"
#define PCIDRV_POWER_SAVE_ENABLE L"PowerSaveEnabled"

#define MILLISECONDS_TO_100NS   (10000)
#define SECOND_TO_MILLISEC      (1000)
#define SECOND_TO_100NS         (SECOND_TO_MILLISEC * MILLISECONDS_TO_100NS)

#define PCIDRV_MIN_IDLE_TIME       1 //  minimum 1 second
#define PCIDRV_DEF_IDLE_TIME       15 // default 15 seconds

//
// Bit Flag Macros
//

#define SET_FLAG(Flags, Bit)    ((Flags) |= (Bit))
#define CLEAR_FLAG(Flags, Bit)  ((Flags) &= ~(Bit))
#define TEST_FLAG(Flags, Bit)   (((Flags) & (Bit)) != 0)

typedef struct _GLOBALS {

    //
    // Path to the driver's Services Key in the registry
    //

    UNICODE_STRING RegistryPath;

} GLOBALS;

extern GLOBALS Globals;

//
// Connector Types
//

typedef struct _PCIDRV_WMI_STD_DATA {

    //
    // Debug Print Level
    //

    UINT32  DebugPrintLevel;

} PCIDRV_WMI_STD_DATA, * PPCIDRV_WMI_STD_DATA;

//
// These are the states FDO transition to upon
// receiving a specific PnP Irp. Refer to the PnP Device States
// diagram in DDK documentation for better understanding.
//

typedef enum _DEVICE_PNP_STATE {

    NotStarted = 0,         // Not started yet
    Started,                // Device has received the START_DEVICE IRP
    StopPending,            // Device has received the QUERY_STOP IRP
    Stopped,                // Device has received the STOP_DEVICE IRP
    RemovePending,          // Device has received the QUERY_REMOVE IRP
    SurpriseRemovePending,  // Device has received the SURPRISE_REMOVE IRP
    Deleted                 // Device has received the REMOVE_DEVICE IRP

} DEVICE_PNP_STATE;

//
// This enum used in implementing the wait-lock
//
typedef enum _WAIT_REASON {
    REMOVE = 0,
    STOP = 1
} WAIT_REASON;

#define INITIALIZE_PNP_STATE(_Data_)    \
        (_Data_)->DevicePnPState =  NotStarted;\
        (_Data_)->PreviousPnPState = NotStarted;

#define SET_NEW_PNP_STATE(_Data_, _state_) \
        (_Data_)->PreviousPnPState =  (_Data_)->DevicePnPState;\
        (_Data_)->DevicePnPState = (_state_);

#define RESTORE_PREVIOUS_PNP_STATE(_Data_)   \
        (_Data_)->DevicePnPState =   (_Data_)->PreviousPnPState;\

typedef enum _QUEUE_STATE {

    HoldRequests = 0,        // Device is not started yet, temporarily
                            // stopped for resource rebalancing, or
                            // entering a sleep state.
    AllowRequests,         // Device is ready to process pending requests
                            // and take in new requests.
    FailRequests             // Fail both existing and queued up requests.

} QUEUE_STATE;

//
// Following are the Wake states.
// Even states are cancelled by atomically OR’ing in 1
// WAKESTATE_WAITING -> WAKESTATE_WAITING_CANCELLED
// WAKESTATE_ARMED -> WAKESTATE_ARMING_CANCELLED
// Others unchanged
//

typedef enum {

    WAKESTATE_DISARMED          = 1,     // No outstanding Wait-Wake IRP

    WAKESTATE_WAITING           = 2,      // Wait-Wake IRP requested, not yet seen

    WAKESTATE_WAITING_CANCELLED = 3, // Wait-Wake cancelled before IRP seen again

    WAKESTATE_ARMED             = 4,      // Wait-Wake IRP seen and forwarded. Device is *probably* armed

    WAKESTATE_ARMING_CANCELLED  = 5, // Wait-Wake IRP seen and cancelled. Hasn't reached completion yet

    WAKESTATE_COMPLETING        = 7 // Wait-Wake IRP has passed the completion routine
} WAKESTATE;


//
// General purpose workitem context used in dispatching work to 
// system worker thread to be executed at PASSIVE_LEVEL.
//
typedef struct _WORKER_ITEM_CONTEXT {
    PIO_WORKITEM   WorkItem;
    PVOID          Callback; // Callback pointer
    PVOID          Argument1;
    PVOID          Argument2;
} WORKER_ITEM_CONTEXT, *PWORK_ITEM_CONTEXT;



//
// The device extension for the device object
//
typedef struct _FDO_DATA
{
    ULONG                   Signature; // must be PCIDRV_FDO_INSTANCE_SIGNATURE
    PDEVICE_OBJECT          Self; // a back pointer to the DeviceObject.
    PDEVICE_OBJECT          UnderlyingPDO;// The underlying PDO
    PDEVICE_OBJECT          NextLowerDriver; // The top of the device stack just
                                         // beneath this device object.
    DEVICE_PNP_STATE        DevicePnPState;   // Track the state of the device
    DEVICE_PNP_STATE        PreviousPnPState; // Remembers the previous pnp state  
    UNICODE_STRING          InterfaceName; // The name returned from
                                       // IoRegisterDeviceInterface,                                      
    QUEUE_STATE             QueueState;      // This flag is set whenever the
                                         // device needs to queue incoming
                                         // requests (when it receives a
                                         // QUERY_STOP or QUERY_REMOVE).
    LIST_ENTRY              NewRequestsQueue; // The queue where the incoming
                                          // requests are held when
                                          // QueueState is set to HoldRequest,
                                          // the device is busy or sleeping.
    KSPIN_LOCK              QueueLock;        // The spin lock that protects
                                          // the queue
    KEVENT                  RemoveEvent; // an event to sync outstandingIO to zero.
    KEVENT                  StopEvent;  // an event to sync outstandingIO to 1.
    ULONG                   OutstandingIO; // 1-biased count of reasons why
                                       // this object should stick around.                                       
    DEVICE_CAPABILITIES     DeviceCaps;   // Copy of the device capability
                                       // Used to find S to D mappings

    // Power Management
    MP_POWER_MGMT           PoMgmt;
    SYSTEM_POWER_STATE      SystemPowerState;   // Current power state of the system (S0-S5)
    DEVICE_POWER_STATE      DevicePowerState; // Current power state of the device(D0 - D3)    
    PIRP                    PendingSIrp; // S-IRP is saved here before generating
                                      // correspoding D-IRP
    PVOID                   PowerCodeLockHandle;                                      
                                          
    // Wait-Wake                                          
    WAKESTATE               WakeState; //Current wakestate
    PIRP                    WakeIrp; //current wait-wake IRP
    KEVENT                  WakeCompletedEvent; //Event to flush outstanding wait-wake IRPs
    KEVENT                  WakeDisableEnableLock; //Event to sync simultaneous arming & disarming requests                                          
    SYSTEM_POWER_STATE      WaitWakeSystemState;
    BOOLEAN                 AllowWakeArming;  // This variable is TRUE if WMI
                                          // requests to arm for wake should be
                                          // acknowledged
    // Idle Detection
    BOOLEAN                 IsDeviceIdle;
    KEVENT                  PowerSaveDisableEnableLock;
    KDPC                    IdleDetectionTimerDpc;
    KTIMER                  IdleDetectionTimer;
    BOOLEAN                 IdlePowerUpRequested;
    KEVENT                  IdlePowerUpCompleteEvent;
    KEVENT                  IdlePowerDownCompleteEvent;
    BOOLEAN                 IdleDetectionEnabled;
    BOOLEAN                 AllowIdleDetectionRegistration;  // This variable tracks the
                                          // current state of checkbox "..save power"in the
                                          // power management tab. TRUE means
                                          // the box is checked. Initial value of
                                          // this variable should be read from
                                          // the device registry.
    LONGLONG                ConservationIdleTime; // Use this time when running on Battery
    LONGLONG                PerformanceIdleTime; // Use this time when running on AC power
    BOOLEAN                 RunningOnBattery; // Are we running on Battery?
    PCALLBACK_OBJECT        PowerStateCallbackObject;
    PVOID                   PowerStateCallbackRegistrationHandle;                                          
    
    // WMI Information
    WMILIB_CONTEXT          WmiLibInfo;
    PCIDRV_WMI_STD_DATA     StdDeviceData;

    // Following fields are specific to the hardware                                       
    // Configuration 
    ULONG                   Flags;
    ULONG                   IsUpperEdgeNdis;    
    UCHAR                   PermanentAddress[ETH_LENGTH_OF_ADDRESS];
    UCHAR                   CurrentAddress[ETH_LENGTH_OF_ADDRESS];
    BOOLEAN                 bOverrideAddress;
    USHORT                  AiTxFifo;           // TX FIFO Threshold
    USHORT                  AiRxFifo;           // RX FIFO Threshold
    UCHAR                   AiTxDmaCount;       // Tx dma count
    UCHAR                   AiRxDmaCount;       // Rx dma count
    UCHAR                   AiUnderrunRetry;    // The underrun retry mechanism
    UCHAR                   AiForceDpx;         // duplex setting
    USHORT                  AiTempSpeed;        // 'Speed', user over-ride of line speed
    USHORT                  AiThreshold;        // 'Threshold', Transmit Threshold
    BOOLEAN                 MWIEnable;          // Memory Write Invalidate bit in the PCI command word
    UCHAR                   Congest;            // Enables congestion control
    ULONG                   SpeedDuplex;        // New reg value for speed/duplex


    // IDs
    UCHAR                   RevsionID;
    USHORT                  SubVendorID;
    USHORT                  SubSystemID;

    // HW Resources
    ULONG                   CacheFillSize;    
    PULONG                  IoBaseAddress;    
    ULONG                   IoRange;           
    PHYSICAL_ADDRESS        MemPhysAddress;
    PKINTERRUPT             Interrupt;
    UCHAR                   InterruptLevel;
    ULONG                   InterruptVector;
    KAFFINITY               InterruptAffinity;
    ULONG                   InterruptMode;
    BOOLEAN                 MappedPorts;
    PHW_CSR                 CSRAddress;
    BUS_INTERFACE_STANDARD  BusInterface;
    PREAD_PORT              ReadPort;
    PWRITE_PORT             WritePort;

    // Media Link State
    UCHAR                   CurrentScanPhyIndex;
    UCHAR                   LinkDetectionWaitCount;
    UCHAR                   FoundPhyAt;
    USHORT                  EepromAddressSize;
    MEDIA_STATE             MediaState; 

    
    // DMA Resources
    ULONG                   AllocatedMapRegisters;
    ULONG                   ScatterGatherListSize;
    NPAGED_LOOKASIDE_LIST   SGListLookasideList;    
    PDMA_ADAPTER            DmaAdapterObject;
    PALLOCATE_COMMON_BUFFER AllocateCommonBuffer;
    PFREE_COMMON_BUFFER     FreeCommonBuffer;
    
    // SEND                       
    PMP_TCB                 CurrSendHead;
    PMP_TCB                 CurrSendTail;
    LONG                    nBusySend;
    LONG                    nWaitSend;
    LONG                    nCancelSend;
    LIST_ENTRY              SendQueueHead;
    KSPIN_LOCK              SendLock;

    LONG                    NumTcb;             // Total number of TCBs
    LONG                    RegNumTcb;          // 'NumTcb'
    LONG                    NumTbd;
    LONG                    NumBuffers;


    PUCHAR                  MpTcbMem;
    ULONG                   MpTcbMemSize;

    PUCHAR                  HwSendMemAllocVa;
    ULONG                   HwSendMemAllocSize;
    PHYSICAL_ADDRESS        HwSendMemAllocPa;

    // command unit status flags
    BOOLEAN                 TransmitIdle;
    BOOLEAN                 ResumeWait;

    // RECV
    LIST_ENTRY              RecvList;
    LONG                    nReadyRecv;
    LONG                    RefCount;

    LONG                    NumRfd;
    LONG                    CurrNumRfd;
    LONG                    MaxNumRfd;
    ULONG                   HwRfdSize;
    LONG                    RfdShrinkCount;

    LIST_ENTRY              RecvQueueHead;
    KSPIN_LOCK              RecvQueueLock;
    KSPIN_LOCK              RcvLock;
    
    NPAGED_LOOKASIDE_LIST   RecvLookaside;
    BOOLEAN                 AllocNewRfd;


    // spin locks for protecting misc variables
    KSPIN_LOCK              Lock;

    // Packet Filter and look ahead size.
    ULONG                   PacketFilter;
    ULONG                   OldPacketFilter;
    ULONG                   ulLookAhead;
    USHORT                  usLinkSpeed;
    USHORT                  usDuplexMode;

    // multicast list
    UINT                    MCAddressCount;
    UCHAR                   MCList[NIC_MAX_MCAST_LIST][ETH_LENGTH_OF_ADDRESS];

    PUCHAR                  HwMiscMemAllocVa;
    ULONG                   HwMiscMemAllocSize;
    PHYSICAL_ADDRESS        HwMiscMemAllocPa;

    PSELF_TEST_STRUC        SelfTest;           // 82558 SelfTest
    ULONG                   SelfTestPhys;

    PNON_TRANSMIT_CB        NonTxCmdBlock;      // 82558 (non transmit) Command Block
    ULONG                   NonTxCmdBlockPhys;

    PDUMP_AREA_STRUC        DumpSpace;          // 82558 dump buffer area
    ULONG                   DumpSpacePhys;

    PERR_COUNT_STRUC        StatsCounters;
    ULONG                   StatsCounterPhys;

    UINT                    PhyAddress;         // Address of the phy component 
    UCHAR                   Connector;          // 0=Auto, 1=TPE, 2=MII

    UCHAR                   OldParameterField;

    ULONG                   HwErrCount;

    // WatchDog timer related fields    
    KDPC                    WatchDogTimerDpc;
    KTIMER                  WatchDogTimer;
    KEVENT                  WatchDogTimerEvent;
    BOOLEAN                 bLinkDetectionWait;
    BOOLEAN                 bLookForLink;
    BOOLEAN                 CheckForHang;

    // For handling IOCTLs - required if the upper edge is NDIS        
    PIRP                    QueryRequest;
    PIRP                    SetRequest;
    PIRP                    StatusIndicationIrp;

    // Packet counts
    ULONG64                 GoodTransmits;
    ULONG64                 GoodReceives;
    ULONG                   NumTxSinceLastAdjust;

    // Count of transmit errors
    ULONG                   TxAbortExcessCollisions;
    ULONG                   TxLateCollisions;
    ULONG                   TxDmaUnderrun;
    ULONG                   TxLostCRS;
    ULONG                   TxOKButDeferred;
    ULONG                   OneRetry;
    ULONG                   MoreThanOneRetry;
    ULONG                   TotalRetries;

    // Count of receive errors
    ULONG                   RcvCrcErrors;
    ULONG                   RcvAlignmentErrors;
    ULONG                   RcvResourceErrors;
    ULONG                   RcvDmaOverrunErrors;
    ULONG                   RcvCdtFrames;
    ULONG                   RcvRuntErrors;
}  FDO_DATA, *PFDO_DATA;

#define CLRMASK(x, mask)     ((x) &= ~(mask));
#define SETMASK(x, mask)     ((x) |=  (mask));


//
// Function prototypes
//
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    );


NTSTATUS
PciDrvAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );


NTSTATUS
PciDrvDispatchPnp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PciDrvDispatchPower (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

PciDrvDispatchPnpStartComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
PciDrvDispatchIO(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
PciDrvDispatchIoctl(
    IN  PFDO_DATA       FdoData,
    IN  PIRP            Irp
    );

VOID 
PciDrvWithdrawIoctlIrps(
    PFDO_DATA FdoData
    );

VOID 
PciDrvWithdrawReadIrps(
    PFDO_DATA FdoData
    );

VOID 
PciDrvWithdrawIrps(
    PFDO_DATA FdoData
    );

NTSTATUS
PciDrvCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PciDrvClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PciDrvRead (
    IN  PFDO_DATA       FdoData,
    IN  PIRP            Irp
    );

NTSTATUS 
PciDrvWrite(
    IN  PFDO_DATA       FdoData,
    IN  PIRP            Irp
);

NTSTATUS
PciDrvCleanup (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PciDrvReturnResources (
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
PciDrvQueueRequest    (
    IN OUT PFDO_DATA FdoData,
    IN PIRP Irp
    );


VOID
PciDrvProcessQueuedRequests    (
    IN OUT PFDO_DATA FdoData
    );



NTSTATUS
PciDrvStartDevice (
    IN PFDO_DATA     FdoData,
    IN PIRP             Irp
    );

VOID
PciDrvCancelRoutineForReadIrp(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

VOID 
PciDrvCancelQueuedReadIrps(
    PFDO_DATA FdoData
    );

VOID
PciDrvCancelQueuedIoctlIrps(
    IN PFDO_DATA FdoData
    );

NTSTATUS
PciDrvQueueIoctlIrp(
    IN  PFDO_DATA               FdoData,
    IN  PIRP                    Irp
    );

VOID
PciDrvCancelRoutineForIoctlIrp(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
PciDrvGetDeviceCapabilities(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PDEVICE_CAPABILITIES    DeviceCapabilities
    );

NTSTATUS
PciDrvDispatchPnpComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


VOID
PciDrvUnload(
    IN PDRIVER_OBJECT DriverObject
    );



VOID
PciDrvCancelRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

BOOLEAN
PciDrvReadRegistryValue(
    IN PFDO_DATA   FdoData,
    IN PWCHAR     Name,
    OUT PLONG    Value
    );

BOOLEAN
PciDrvWriteRegistryValue(
    IN PFDO_DATA  FdoData,
    IN PWCHAR     Name,
    IN ULONG      Value
    );

LONG
PciDrvIoIncrement    (
    IN  OUT PFDO_DATA   FdoData
    );


LONG
PciDrvIoDecrement    (
    IN  OUT PFDO_DATA   FdoData
    );

VOID
PciDrvReleaseAndWait(
    IN  OUT PFDO_DATA   FdoData,
    IN  ULONG           ReleaseCount,
    IN  WAIT_REASON     Reason
    );

ULONG
PciDrvGetOutStandingIoCount(
    IN PFDO_DATA FdoData
    );

PCHAR
PnPMinorFunctionString (
    UCHAR MinorFunction
);


NTSTATUS
PciDrvSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
PciDrvSendIrpSynchronously (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PciDrvForwardAndForget(
    IN PFDO_DATA     FdoData,
    IN PIRP          Irp
    );

NTSTATUS
PciDrvSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
PciDrvQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );

NTSTATUS
PciDrvQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );

NTSTATUS
PciDrvWmiRegistration(
    IN PFDO_DATA               FdoData
);

NTSTATUS
PciDrvWmiDeRegistration(
    IN PFDO_DATA               FdoData
);


NTSTATUS
PciDrvDispatchWaitWake(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PciDrvSetWaitWakeEnableState(
    IN PFDO_DATA FdoData,
    IN BOOLEAN WakeState
    );

BOOLEAN
PciDrvGetWaitWakeEnableState(
    IN PFDO_DATA   FdoData
    );

VOID
PciDrvAdjustCapabilities(
    IN OUT PDEVICE_CAPABILITIES DeviceCapabilities
    );


BOOLEAN
PciDrvCanWakeUpDevice(
    IN PFDO_DATA FdoData,
    IN SYSTEM_POWER_STATE PowerState
    );

BOOLEAN
PciDrvArmForWake(
    IN  PFDO_DATA   FdoData,
    IN  BOOLEAN     DeviceStateChange
    );

VOID
PciDrvDisarmWake(
    IN  PFDO_DATA   FdoData,
    IN  BOOLEAN     DeviceStateChange
    );

NTSTATUS
PciDrvWaitWakeIoCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

VOID
PciDrvWaitWakePoCompletionRoutine(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  UCHAR               MinorFunction,
    IN  POWER_STATE         State,
    IN  PVOID               Context,
    IN  PIO_STATUS_BLOCK    IoStatus
    );

VOID
PciDrvPassiveLevelReArmCallbackWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PWORK_ITEM_CONTEXT Context
    );

VOID
PciDrvPassiveLevelClearWaitWakeEnableState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PWORK_ITEM_CONTEXT Context
    );

NTSTATUS
PciDrvQueuePassiveLevelCallback(
    IN PFDO_DATA    FdoData,
    IN PIO_WORKITEM_ROUTINE CallbackFunction,
    IN PVOID Context1,
    IN PVOID Context2
    );

VOID
PciDrvRegisterForIdleDetection( 
    IN PFDO_DATA   FdoData,
    IN BOOLEAN      DeviceStateChange
    );

VOID
PciDrvDeregisterIdleDetection( 
    IN PFDO_DATA   FdoData,
    IN BOOLEAN      DeviceStateChange
    );

NTSTATUS
PciDrvSetPowerSaveEnableState(
    IN PFDO_DATA FdoData,
    IN BOOLEAN State
    );

BOOLEAN
PciDrvGetPowerSaveEnableState(
    IN PFDO_DATA   FdoData
    );

NTSTATUS
PciDrvPowerUpDevice(
    IN PFDO_DATA FdoData,
    IN BOOLEAN   Wait
    );

VOID
PciDrvSetIdleTimer(
    IN PFDO_DATA FdoData
    );

#define PciDrvSetDeviceBusy(FdoData) \
                            FdoData->IsDeviceIdle = FALSE;

VOID
PciDrvPowerDownDeviceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PWORK_ITEM_CONTEXT Context
);

VOID
PciDrvPowerUpDeviceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PWORK_ITEM_CONTEXT Context
);

VOID
PciDrvIdleDetectionTimerDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    );


VOID
PciDrvReStartIdleDetectionTimer(
    IN PFDO_DATA FdoData
    );


VOID
PciDrvCancelIdleDetectionTimer(
    IN PFDO_DATA FdoData
    );

VOID
PciDrvPowerStateCallback(
    IN  PVOID CallbackContext,
    IN  PVOID Argument1,
    IN  PVOID Argument2
    );

VOID
PciDrvRegisterPowerStateNotification(
    IN PFDO_DATA   FdoData
    );

VOID
PciDrvUnregisterPowerStateNotification(
    IN PFDO_DATA   FdoData
    );

VOID
PciDrvStartDeviceWorker (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PWORK_ITEM_CONTEXT   Context
    );

#if defined(WIN2K)

NTKERNELAPI
VOID
ExFreePoolWithTag(
    IN PVOID P,
    IN ULONG Tag
    );

//
// This value should be returned from completion routines to continue
// completing the IRP upwards. Otherwise, STATUS_MORE_PROCESSING_REQUIRED
// should be returned.
//
#define STATUS_CONTINUE_COMPLETION      STATUS_SUCCESS


#endif

#endif  // _PCIDRV_H_

