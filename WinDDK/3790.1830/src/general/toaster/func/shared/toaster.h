/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.
    
Module Name:

    Toaster.h

Abstract:

    Toaster.h defines the data types used in the different stages of the function
    driver.

Environment:

    Kernel mode

Revision History:

    Eliyas Yakub  06-Oct-1998

    Kevin Shirley  01-Jul-2003  Commented for tutorial and learning purposes

--*/


#if !defined(_TOASTER_H_)
#define _TOASTER_H_

//
// Include the main DDK header, ntddk.h. Ntddk.h defines the data types required by
// drivers.
//
#include <ntddk.h> 

//
// Include the WMI library header, wmilib.h. Wmilib.h defines the data type required
// to compile Wmi.c.
//
#include <wmilib.h>

//
// Include the GUID library header, initguid.h. Initguid.h is required to use the
// GUID definitions that appear throughout the driver.
//
#include <initguid.h> // required for GUID definitions

//
// Disables few warnings so that we can build our driver with MSC W4 level.
// Disable warning C4057; X differs in indirection to slightly different base types from Y 
// Disable warning C4100: unreferenced formal parameter
//
#pragma warning(disable:4100 4057)

//
// Include the driver.h which define the common data used by both 
// the underlying Toaster bus driver and the Toaster sample function driver.
//
#include "driver.h"

//
// Include the public.h which define the common data used by both 
// the driver and the Toaster sample applications.
//
#include "public.h"

//
// The function driver uses the safe string functions introduced with the .NET 
// DDK to avoid security issues related to buffer overruns.
//
// The function drivers calls RtlStringCbVPrintfA instead of vsnprintf_because
// the function driver can specify the size of the destination buffer to ensure that
// RtlStringCbVPrintfA does not write past the end of the buffer, and because
// RtlStringCbVPrintfA guarantees that the resulting string to be NULL terminated,
// even if the operation truncates the intended result.
//
// To use RtlStringCbVPrintfA on Windows 9x/Me or Windows 2000, the NTSTRSAFE_LIB
// token must be defined before including the ntstrsafe.h file.
//
#define NTSTRSAFE_LIB
#include <ntstrsafe.h>


//
// The TOASTER_POOL_TAG define is a 4 byte tag that is used to identify memory pool
// allocation requests from the function driver. "saoT" is "Toas" in reverse. When
// using a debugger to search for memory allocated by the Toaster function driver,
// specify "Toas".
//
#define TOASTER_POOL_TAG (ULONG) 'saoT'
//
// The TOASTER_FDO_INSTANCE_SIGNATURE define is a 4 byte tag that is used to identify
// Toaster function driver device extension. "odFT" is "TFdo" in reverse and stands
// for Toaster Fdo. When using a debugger to search for Toaster device extensions,
// specify "TFdo".
//
#define TOASTER_FDO_INSTANCE_SIGNATURE (ULONG) 'odFT'

//
// The TOASTER_WAIT_WAKE_ENABLE define specifies the Registry key that indicates
// whether the hardware instance is armed to signal wake-up.
//
#define TOASTER_WAIT_WAKE_ENABLE L"WaitWakeEnabled"

//
// The TOASTER_POWER_SAVE_ENABLE define specifies the Registry key that indicates
// whether the hardware instance is power management enabled.
//
#define TOASTER_POWER_SAVE_ENABLE L"PowerSaveEnabled"

//
// The InterlockedOr system call is required when the function driver processes the
// IRP_MN_WAIT_WAKE power IRP. Because InterlockedOr is not defined in the Windows
// 2000 DDK header files, it must be defined to use the intrinsic system call 
// if the function driver is compiled under Windows 2000.
//
#if !defined(InterlockedOr) && (_WIN32_WINNT==0x0500)
#define InterlockedOr _InterlockedOr
#endif

#if !defined(EVENT_TRACING)
//
// If the source. file does not define the EVENT_TRACING token when the BUILD
// utility compiles the sources to the function driver, then the function driver
// uses the ToasterDebugPrint routine to output debugging information to a connected
// kernel debugger, and also uses the debug error level #defines.
//
#define     ERROR    0 
#define     WARNING  1
#define     TRACE    2
#define     INFO     3

VOID
ToasterDebugPrint    (
    IN ULONG   DebugPrintLevel,
    IN PCCHAR  DebugMessage,
    ...
    );

#else
//
// If the source. file defines the EVENT_TRACING token when the BUILD utility
// compiles the sources to the function driver, then the function driver uses WPP
// software tracing.
//
// The WPP_DEFINE_CONTROL_GUID macro defines function driver's WPP software tracing
// GUID. The function driver's GUID is {C56386BD-7C67-4264-B8D9-C4A53B93CBEB}.
// However, because this GUID is associated with the Toaster sample function driver,
// a new GUID must be created if using the Toaster sample function driver for another
// purpose. A new GUID can be created using the GuidGen.exe utility.
//
// The WPP_DEFINE_BIT macro allows setting debug bit masks to selectively print debug
// output. The names specified in the WPP_DEFINE_BIT macro defines the actual names
// that are used to control the level of tracing for the GUID specified in the
// WPP_DEFINE_CONTROL_GUID macro.
//
// The name of the logger is Toaster and the GUID is:
// {C56386BD-7C67-4264-B8D9-C4A53B93CBEB}
// (0xc56386bd, 0x7c67, 0x4264, 0xb8, 0xd9, 0xc4, 0xa5, 0x3b, 0x93, 0xcb, 0xeb);
//
#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(Toaster,(C56386BD,7C67,4264,B8D9,C4A53B93CBEB), \
        WPP_DEFINE_BIT(ERROR)                /* bit  0 = 0x00000001 */ \
        WPP_DEFINE_BIT(WARNING)              /* bit  1 = 0x00000002 */ \
        WPP_DEFINE_BIT(TRACE)                /* bit  2 = 0x00000004 */ \
        WPP_DEFINE_BIT(INFO)                 /* bit  3 = 0x00000008 */ \
        WPP_DEFINE_BIT(DebugFlag04)          /* bit  4 = 0x00000010 */ \
        WPP_DEFINE_BIT(DebugFlag05)          /* bit  5 = 0x00000020 */ \
        WPP_DEFINE_BIT(DebugFlag06)          /* bit  6 = 0x00000040 */ \
        WPP_DEFINE_BIT(DebugFlag07)          /* bit  7 = 0x00000080 */ \
        WPP_DEFINE_BIT(DebugFlag08)          /* bit  8 = 0x00000100 */ \
        WPP_DEFINE_BIT(DebugFlag09)          /* bit  9 = 0x00000200 */ \
        WPP_DEFINE_BIT(DebugFlag10)          /* bit 10 = 0x00000400 */ \
        WPP_DEFINE_BIT(DebugFlag11)          /* bit 11 = 0x00000800 */ \
        WPP_DEFINE_BIT(DebugFlag12)          /* bit 12 = 0x00001000 */ \
        WPP_DEFINE_BIT(DebugFlag13)          /* bit 13 = 0x00002000 */ \
        WPP_DEFINE_BIT(DebugFlag14)          /* bit 14 = 0x00004000 */ \
        WPP_DEFINE_BIT(DebugFlag15)          /* bit 15 = 0x00008000 */ \
        WPP_DEFINE_BIT(DebugFlag16)          /* bit 16 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag17)          /* bit 17 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag18)          /* bit 18 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag19)          /* bit 19 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag20)          /* bit 20 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag21)          /* bit 21 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag22)          /* bit 22 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag23)          /* bit 23 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag24)          /* bit 24 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag25)          /* bit 25 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag26)          /* bit 26 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag27)          /* bit 27 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag28)          /* bit 28 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag29)          /* bit 29 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag30)          /* bit 30 = 0x00000000 */ \
        WPP_DEFINE_BIT(DebugFlag31)          /* bit 31 = 0x00000000 */ \
        )
//
// Uncomment the following lines if want to enable kd printing on top 
// of WMI logging. 
//
//#if defined(DBG)
//#define WPP_DEBUG(b)  DbgPrint("[%ws] ", L"Featured2"), DbgPrint b
//#endif
#endif

//
// The GLOBALS structure defines a single structure to hold all the global variables
// used throughout the function driver. This keeps all the global information together
// and makes it easy to look at all the variables in the debugger.
//
typedef struct _GLOBALS {

    //
    // RegistryPath holds a copy of the path to the function drivers Services key in
    // the Registry, as passed to the DriverEntry routine.
    //
    UNICODE_STRING RegistryPath;

} GLOBALS;

extern GLOBALS Globals;

//
// Define the different physical connector types of a toaster device. Because the
// toaster hardware is imaginary, the following list contains several standard
// connector types.
//
#define TOASTER_WMI_STD_I8042 0
#define TOASTER_WMI_STD_SERIAL 1
#define TOASTER_WMI_STD_PARALEL 2
#define TOASTER_WMI_STD_USB 3

//
// The TOASTER_WMI_STD_DATA structure defines the standard data corresponding to a
// toaster device that is exposed to the WMI library. The variables in this structure
// correspond to the data items in the Toaster.mof file.
//
typedef struct _TOASTER_WMI_STD_DATA {

    //
    // ConnectorType specifies the physical connector type of the toaster device, as
    // defined by the four #define connector type tokens above. The function driver
    // initializes the value for this member, which can then be returned to a calling
    // user-mode application. However, this member is read-only (as specified in the
    // Toaster.mof file) and therefore cannot be modified by the calling application
    // through the WMI library.
    // WMI supports a limited number of datatypes, such as UINT32. That is why the
    // #define TOASTER_WMI_STD_Xxx tokens are not placed in an enumeration.
    //
    UINT32   ConnectorType;

    //
    // Capacity defines the capacity, in kilowatts, of the toaster device. The
    // function driver initializes the value for this member, which can then be
    // returned to a calling user-mode application. However, this member is read-only
    // (as specified in the Toaster.mof file) and therefore cannot be modified by the
    // calling application through the WMI library.
    //
    UINT32   Capacity;

    //
    // ErrorCount specifies the number of errors that occurred on the toaster device.
    // The function driver initializes the value for this member, which can then be
    // returned to a calling user-mode application. However, this member is read-only
    // (as specified in the Toaster.mof file) and therefore cannot be modified by the
    // calling application through the WMI library.
    //
    UINT32   ErrorCount;

    //
    // Controls specifies the number of controls on the toaster device. The function
    // driver initializes the value for this member, which can then be returned to a
    // calling user-mode application. However, this member is read-only (as specified
    // in the Toaster.mof file) and therefore cannot be modified by the calling
    // application through the WMI library.
    //
    UINT32   Controls;

    //
    // DebugPrintLevel specifies the debug output level of toaster device. The function
    // driver initializes the value for this member, which can then be returned to a
    // calling user-mode application.
    // Because this member is read/write (as specified in the Toaster.mof file), the
    // calling application can therefore change the value of this member through the
    // WMI library.
    //
    UINT32  DebugPrintLevel;

} TOASTER_WMI_STD_DATA, * PTOASTER_WMI_STD_DATA;

//
// The DEVICE_PNP_STATE enumeration defines the different PnP states that the function
// driver supports upon receiving PnP IRPs and represents a state-model. Refer to the
// "PnP Device States" topic in the DDK documentation for a more thorough explanation
// of the different PnP IRPs and how they affect a driver's PnP state.
//
// The function driver initializes the PnP state model to NotStarted when
// ToasterAddDevice initializes the FDO.
//
// When the function driver receives IRP_MN_START_DEVICE, it changes the state to
// Started. Whenever the state is in Started the function driver immediately
// processes any new read/write/device control IRPs instead of adding them to the
// driver-managed IRP queue and then processes any IRP in the queue. Whenever the
// state is not Started the function driver either adds new IRPs to the queue, or
// fails them.
//
// When the function driver receives IRP_MN_QUERY_STOP_DEVICE, it changes the state
// to StopPending and begins to queue IRPs. The function driver begins to queue
// IRPs instead of failing them, because the system might send a
// IRP_MN_CANCEL_STOP_DEVICE which restores the previous state to Started, thus
// the function driver can process new IRPs and process any queued IRPs.
//
// When the function driver receives IRP_MN_STOP_DEVICE, it changes the state to
// Stopped and begins to (or remains to) queue IRPs.
//
// When the function driver receives IRP_MN_QUERY_REMOVE_DEVICE, it changes the state
// to RemovePending and begins to queue IRPs. The function driver begins to queue
// IRPs instead of failing them, because the system might send a
// IRP_MN_CANCEL_REMOVE_DEVICE which restores the previous state to Started, thus
// the function driver can process new IRPs and process any queued IRPs.
//
// When the function driver receives IRP_MN_SURPRISE_REMOVE, it changes the state
// to SurpriseRemovePending. The system then sends IRP_MN_REMOVE_DEVICE.
//
// When the function driver receives IRP_MN_REMOVE_DEVICE, it changes the state
// to Deleted and fails any new IRPs as well as fails any IRPs in the driver-managed
// IRP queue.
//
typedef enum _DEVICE_PNP_STATE {

    NotStarted = 0,
    Started,
    StopPending,
    Stopped,
    RemovePending,
    SurpriseRemovePending,
    Deleted

} DEVICE_PNP_STATE;

//
// The INITIALIZE_PNP_STATE macro initialized both the device extension's
// DevicePnPState as well as PreviousPnPState members to NotStarted. The function
// driver's ToasterAddDevice routine calls this macro.
//
#define INITIALIZE_PNP_STATE(_Data_)    \
        (_Data_)->DevicePnPState =  NotStarted;\
        (_Data_)->PreviousPnPState = NotStarted;

//
// The SET_NEW_PNP_STATE macro saves the device extension's present DevicePnPState
// member into the PreviousPnPState member, so it can be later restored when the
// function driver calls the RESTORE_PREVIOUS_PNP_STATE macro. The SET_NEW_PNP_STATE
// macro then sets the DevicePnPState member to the new PnP state.
//
#define SET_NEW_PNP_STATE(_Data_, _state_) \
        (_Data_)->PreviousPnPState =  (_Data_)->DevicePnPState;\
        (_Data_)->DevicePnPState = (_state_);

//
// The RESTORE_PREVIOUS_PNP_STATE macro restores the device extension's previous
// saved PreviousPnPState member to the DevicePnPState member.
//
#define RESTORE_PREVIOUS_PNP_STATE(_Data_)   \
        (_Data_)->DevicePnPState =   (_Data_)->PreviousPnPState;\

//
// The QUEUE_STATE enumeration defines the different states the driver-managed IRP
// queue can be set to.
//
// The function driver initializes the queue state to HoldRequests when
// ToasterAddDevice initializes the FDO.
//
// The function driver sets the queue state to HoldRequests when the hardware
// instance is not yet started, temporarily stopped for resource rebalancing, or
// entering a sleep state.
//
// The function driver sets the queue state to AllowRequests when the hardware
// instance is active and ready to processing new or pending read/write/device control
// IRPs.
//
// The function driver sets the queue state to FailRequests when the hardware
// instance is no longer present.
//
typedef enum _QUEUE_STATE {

    HoldRequests = 0,
    AllowRequests,
    FailRequests

} QUEUE_STATE;

//
// The WAKESTATE enumeration defines the different wait/wake arming states that the
// function driver supports.
//
// The function driver initializes the wake state to WAKESTATE_DISARMED when
// ToasterAddDevice initializes the FDO, or when there is no outstanding wake/wake
// power IRP.
//
// The function driver sets the wake state to WAKESTATE_WAITING when
// ToasterArmForWake requests a IRP_MN_WAIT_WAKE power IRP, but the function driver
// has not yet received the IRP.
//
// The function driver sets the wake state to WAKESTATE_WAITING_CANCELLED if the
// requested IRP_MN_WAIT_WAKE power IRP has been cancelled before the function driver
// receives it.
//
// The function driver sets the wake state to WAKESTATE_ARMED when the
// IRP_MN_WAIT_WAKE power IRP has been received and passed down the device stack to
// be processed by the underlying bus driver. The function driver considers the
// hardware instance armed from this point. However, it is possible that a lower
// driver has failed IRP_MN_WAIT_WAKE and that the IRP is working its way back to
// the function driver.
//
// The function driver sets the wake state to WAKESTATE_ARMED_CANCELLED if the
// requested IRP_MN_WAIT_WAKE power IRP has been received and passed down the device
// stack, but the underlying bus driver has not yet completed it.
//
// The function driver sets the wake state to WAKESTATE_COMPLETING when the
// IRP_MN_WAIT_WAKE power IRP has passed the completion routine.
//
// The even numbered wake states, WAKESTATE_WAITING and WAKESTATE_ARMED, are cancelled
// by atomically OR'ing in the integer 1. The other wake states are unaffected by the
// OR'ing. The OR'ing operation relates to the InterlockedOr system call.
//
typedef enum {

    WAKESTATE_DISARMED          = 1, 
    WAKESTATE_WAITING           = 2, 
    WAKESTATE_WAITING_CANCELLED = 3, 
    WAKESTATE_ARMED             = 4, 
    WAKESTATE_ARMING_CANCELLED  = 5, 
    WAKESTATE_COMPLETING        = 7
} WAKESTATE;

//
// The FDO_DATA structure describes the Toaster sample function driver's device
// extension. The device extension is where all per-device-instance information 
// is kept. 
//
typedef struct _FDO_DATA
{

    //
    // All the members below this point are present in all the stages of the function
    // driver.
    // ----------------------------------------------------------------------------
    //

    //
    // Signature specifies a 4 byte tag that can e used to identify Toaster device
    // extensions. ToasterAddDevice initializes this member to
    // TOASTER_FDO_INSTANCE_SIGNATURE.
    //
    ULONG   Signature;

    //
    // Self represents a pointer back to the device object that is the parent of
    // the device extension instance. Saving a pointer to the parent device object
    // allows the device extension to be passed to the function driver's routines
    // instead of the requiring passing both the device object and its
    // device extension.
    //
    PDEVICE_OBJECT      Self;

    //
    // UnderlyingPDO specifies the pointer to the physical device object (PDO)
    // created by the underlying bus driver to represent the hardware instance.
    //
    PDEVICE_OBJECT      UnderlyingPDO;

    //
    // NextLowerDriver specifies the pointer to the driver layered immediately under
    // the function driver in the device stack.
    //
    PDEVICE_OBJECT      NextLowerDriver;

    //
    // DevicePnPState describes the present PnP state of the hardware instance. The
    // ToasterDispatchPnP routines changes the PnP state in response to various PnP
    // IRPs.
    //
    DEVICE_PNP_STATE    DevicePnPState;

    //
    // PreviousPnPState represents the old PnP state of the hardware instance before
    // the ToasterDispatchPnP routine updates the present PnP state in response to
    // various PnP IRPs.
    //
    DEVICE_PNP_STATE    PreviousPnPState;
    
    //
    // InterfaceName describes the device interface name returned when
    // ToasterAddDevice calls IoRegisterDeviceInterface.
    //
    UNICODE_STRING      InterfaceName;

    //
    // All the members below this point are present in the Incomplete2, Featured1,
    // and Featured2 stages of the function driver.
    // ----------------------------------------------------------------------------
    //

    //
    // QueueState describes the state of the driver-managed IRP queue. The state
    // changes in response to certain PnP IRPs.
    //
    QUEUE_STATE         QueueState;

    //
    // NewRequestsQueue represents the driver-managed IRP queue. New
    // read/write/device control IRPs are added to the queue whenever the hardware
    // instance is not able to process the IRPs.
    //
    LIST_ENTRY          NewRequestsQueue;

    //
    // QueueLock represents the spin lock that protects and synchronizes multiple
    // threads access to the driver-managed IRP queue, so that only one thread at
    // a time can manipulate the queue's contents.
    //
    KSPIN_LOCK          QueueLock;

    //
    // RemoveEvent represents the kernel event that synchronizes the function
    // driver's processing of IRP_MN_REMOVE_DEVICE. RemoveEvent is only signaled
    // when OutstandingIO equals 0.
    //
    KEVENT              RemoveEvent;

    //
    // StopEvent represents the kernel event the synchronizes the function driver's
    // processing of IRP_MN_QUERY_STOP_DEVICE and IRP_MN_QUERY_REMOVE_DEVICE.
    // StopEvent is only signaled when OutstandingIO equals 1.
    //
    KEVENT              StopEvent;

    //
    // OutstandingIO represents a 1 based count of unprocessed IRPs the function
    // driver has received. When OutstandingIO equals 1 the function driver has no
    // unprocessed IRPs and the hardware can be stopped. When OutstandingIO equals
    // 0, the hardware instance is being removed.
    //
    ULONG               OutstandingIO;


    //
    // All the members below this point are present in the Featured1 and Featured2
    // stages of the function driver.
    // ----------------------------------------------------------------------------
    //

    //
    // DontDisplayInUI represents whether the Device Manager displays the hardware
    // instance to the user. When this member equals TRUE, the hardware instance is
    // not displayed in the Device Manager, unless the "Show hidden devices" menu
    // item is selected.
    //
    BOOLEAN             DontDisplayInUI;

    //
    // SystemPowerState describes the overall system power level. Possible values for
    // this member range from S0 (fully on and operational) to S5 (power off).
    //
    SYSTEM_POWER_STATE  SystemPowerState;

    //
    // DevicePowerState describes the power level of the hardware instance. Possible
    // values for this member range from D0 (fully on and operation) to D3 (power
    // off).
    //
    DEVICE_POWER_STATE  DevicePowerState;

    //
    // WmiLibInfo dewscribes the function driver's context information for the WMI
    // library. The ToasterWmiRegistration routine initializes this member to
    // register the function driver as a WMI data provider.
    //
    WMILIB_CONTEXT      WmiLibInfo;

    //
    // StdDeviceData represents information that applies to standard toaster devices.
    //
    TOASTER_WMI_STD_DATA   StdDeviceData;

    //
    // DeviceCaps represents the hardware instances device capabilities. This member
    // is used to find the system power state to device power state mappings.
    //
    DEVICE_CAPABILITIES DeviceCaps;

    //
    // PendingSIrp represents the IRP_MN_SET_POWER S-IRP that the function driver
    // processes a corresponding IRP_MN_SET_POWER D-IRP. When this member equals
    // NULL, the function driver is not processing any IRP_MN_SET_POWER operation.
    //
    PIRP                PendingSIrp;

    //
    // AllowIdleDetectionRegistration represents the current state of the "Allow the
    // computer to turn off this device to save power." checkbox in the Power
    // Management tab of the hardware instance's Properties dialog. This member
    // equals TRUE when the check box is checked. The initial value of this member
    // should be read from the deviec registry.
    //
    BOOLEAN             AllowIdleDetectionRegistration;

    //
    // AllowWakeArming represents whether requests to arm the hardware instance to
    // signal wake-up are acknowledged. This member equals TRUE if WMI requests to
    // arm the hardware instance to signal wake-up should be acknowledged.
    //
    BOOLEAN             AllowWakeArming;

    //
    // All the members below this point are present in the Featured2 stage of the
    // function driver.
    // ----------------------------------------------------------------------------
    //

    //
    // WakeState describes the present wait/wake power state of the function driver.
    //
    WAKESTATE           WakeState;

    //
    // WakeIrp represents the present IRP_MN_WAIT_WAKE power IRP that the function
    // driver is processing.
    //
    PIRP                WakeIrp;

    //
    // WakeCompletedEvent represents the kernel event to flush outstanding
    // IRP_MN_WAIT_WAKE power IRPs.
    //
    KEVENT              WakeCompletedEvent;

    //
    // WakeDisableEnableLock represents the kernel event to prevent against
    // simultaneous arming and disarming requests.
    //
    KEVENT              WakeDisableEnableLock;

}  FDO_DATA, *PFDO_DATA;

//
// The CLRMASK and SETMASK macros clear and set the passed mask of the passed
// variable.
//
#define CLRMASK(x, mask)     ((x) &= ~(mask));
#define SETMASK(x, mask)     ((x) |=  (mask));

//
// Declare the function prototypes for all of the function driver's routines in all
// the stages of the function driver.
//

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    );


NTSTATUS
ToasterAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );


NTSTATUS
ToasterDispatchPnp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ToasterDispatchPower (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
ToasterDispatchIO(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ToasterDispatchIoctl(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ToasterCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ToasterClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ToasterReadWrite (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ToasterCleanup (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
ToasterStartDevice (
    IN PFDO_DATA     FdoData,
    IN PIRP             Irp
    );



NTSTATUS
ToasterDispatchPnpComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


VOID
ToasterUnload(
    IN PDRIVER_OBJECT DriverObject
    );



VOID
ToasterCancelQueued (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );



LONG
ToasterIoIncrement    (
    IN  OUT PFDO_DATA   FdoData
    );


LONG
ToasterIoDecrement    (
    IN  OUT PFDO_DATA   FdoData
    );

NTSTATUS
ToasterSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
ToasterSendIrpSynchronously (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ToasterGetDeviceCapabilities(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PDEVICE_CAPABILITIES    DeviceCapabilities
    );

NTSTATUS
ToasterSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
ToasterSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
ToasterQueryWmiDataBlock(
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
ToasterQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );

PCHAR
WMIMinorFunctionString (
    UCHAR MinorFunction
);

NTSTATUS
GetDeviceFriendlyName(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PUNICODE_STRING DeviceName
    );

NTSTATUS
ToasterWmiRegistration(
    IN PFDO_DATA               FdoData
);

NTSTATUS
ToasterWmiDeRegistration(
    IN PFDO_DATA               FdoData
);

NTSTATUS
ToasterReturnResources (
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
ToasterQueueRequest    (
    IN OUT PFDO_DATA FdoData,
    IN PIRP Irp
    );


VOID
ToasterProcessQueuedRequests    (
    IN OUT PFDO_DATA FdoData
    );


NTSTATUS
ToasterCanStopDevice    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
ToasterCanRemoveDevice    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
ToasterFireArrivalEvent(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
ToasterFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLECONTROL Function,
    IN BOOLEAN Enable
    );

NTSTATUS
ToasterDispatchWaitWake(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ToasterSetWaitWakeEnableState(
    IN PFDO_DATA FdoData,
    IN BOOLEAN WakeState
    );

BOOLEAN
ToasterGetWaitWakeEnableState(
    IN PFDO_DATA   FdoData
    );

VOID
ToasterAdjustCapabilities(
    IN OUT PDEVICE_CAPABILITIES DeviceCapabilities
    );

BOOLEAN
ToasterArmForWake(
    IN  PFDO_DATA   FdoData,
    IN  BOOLEAN     DeviceStateChange
    );

VOID
ToasterDisarmWake(
    IN  PFDO_DATA   FdoData,
    IN  BOOLEAN     DeviceStateChange
    );

NTSTATUS
ToasterWaitWakeIoCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

VOID
ToasterWaitWakePoCompletionRoutine(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  UCHAR               MinorFunction,
    IN  POWER_STATE         PowerState,
    IN  PVOID               PowerContext,
    IN  PIO_STATUS_BLOCK    IoStatus
    );

VOID
ToasterPassiveLevelReArmCallbackWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

VOID
ToasterPassiveLevelClearWaitWakeEnableState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

VOID
ToasterQueuePassiveLevelCallback(
    IN PFDO_DATA    FdoData,
    IN PIO_WORKITEM_ROUTINE CallbackFunction
    );

NTSTATUS
ToasterGetStandardInterface(
    IN PDEVICE_OBJECT Pdo,
    OUT PTOASTER_INTERFACE_STANDARD BusInterface
    );

VOID
ToasterRegisterForIdleDetection( 
    IN PFDO_DATA   FdoData,
    IN BOOLEAN      DeviceStateChange
    );

VOID
ToasterDeregisterIdleDetection( 
    IN PFDO_DATA   FdoData,
    IN BOOLEAN      DeviceStateChange
    );

NTSTATUS
ToasterSetPowerSaveEnableState(
    IN PFDO_DATA FdoData,
    IN BOOLEAN State
    );

BOOLEAN
ToasterGetPowerSaveEnableState(
    IN PFDO_DATA   FdoData
    );

VOID
ToasterPowerUpDevice(
    IN PFDO_DATA FdoData
    );

PCHAR
PnPMinorFunctionString (
    UCHAR MinorFunction
);


#endif  // _TOASTER_H_

