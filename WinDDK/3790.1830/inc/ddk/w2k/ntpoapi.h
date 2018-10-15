/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1995-1999  Microsoft Corporation

Module Name:

    ntpoapi.h

Abstract:

    This module contains the user APIs for the NT Power Management.

Author:

Revision History:

--*/

#ifndef _NTPOAPI_
#define _NTPOAPI_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// Power Management user APIs
//

// begin_ntddk begin_ntifs begin_nthal begin_ntminiport begin_wdm

#ifndef _PO_DDK_
#define _PO_DDK_

typedef enum _SYSTEM_POWER_STATE {
    PowerSystemUnspecified = 0,
    PowerSystemWorking,
    PowerSystemSleeping1,
    PowerSystemSleeping2,
    PowerSystemSleeping3,
    PowerSystemHibernate,
    PowerSystemShutdown,
    PowerSystemMaximum
} SYSTEM_POWER_STATE, *PSYSTEM_POWER_STATE;

typedef enum {
    PowerActionNone = 0,
    PowerActionReserved,
    PowerActionSleep,
    PowerActionHibernate,
    PowerActionShutdown,
    PowerActionShutdownReset,
    PowerActionShutdownOff,
    PowerActionWarmEject
} POWER_ACTION, *PPOWER_ACTION;

typedef enum _DEVICE_POWER_STATE {
    PowerDeviceUnspecified = 0,
    PowerDeviceD0,
    PowerDeviceD1,
    PowerDeviceD2,
    PowerDeviceD3,
    PowerDeviceMaximum
} DEVICE_POWER_STATE, *PDEVICE_POWER_STATE;

typedef union _POWER_STATE {
    SYSTEM_POWER_STATE SystemState;
    DEVICE_POWER_STATE DeviceState;
} POWER_STATE, *PPOWER_STATE;

typedef enum _POWER_STATE_TYPE {
    SystemPowerState = 0,
    DevicePowerState
} POWER_STATE_TYPE, *PPOWER_STATE_TYPE;


//
// Generic power related IOCTLs
//

#define IOCTL_QUERY_DEVICE_POWER_STATE  \
        CTL_CODE(FILE_DEVICE_BATTERY, 0x0, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_SET_DEVICE_WAKE           \
        CTL_CODE(FILE_DEVICE_BATTERY, 0x1, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CANCEL_DEVICE_WAKE        \
        CTL_CODE(FILE_DEVICE_BATTERY, 0x2, METHOD_BUFFERED, FILE_WRITE_ACCESS)


//
// Defines for W32 interfaces
//

// begin_winnt

#define ES_SYSTEM_REQUIRED  ((ULONG)0x00000001)
#define ES_DISPLAY_REQUIRED ((ULONG)0x00000002)
#define ES_USER_PRESENT     ((ULONG)0x00000004)
#define ES_CONTINUOUS       ((ULONG)0x80000000)

typedef ULONG EXECUTION_STATE;

typedef enum {
    LT_DONT_CARE,
    LT_LOWEST_LATENCY
} LATENCY_TIME;

// end_winnt end_ntminiport end_wdm end_ntifs

typedef enum {
    SystemPowerPolicyAc,
    SystemPowerPolicyDc,
    VerifySystemPolicyAc,
    VerifySystemPolicyDc,
    SystemPowerCapabilities,
    SystemBatteryState,
    SystemPowerStateHandler,
    ProcessorStateHandler,
    SystemPowerPolicyCurrent,
    AdministratorPowerPolicy,
    SystemReserveHiberFile,
    ProcessorInformation,
    SystemPowerInformation
} POWER_INFORMATION_LEVEL;

// begin_ntminiport begin_wdm begin_ntifs

#endif // !_PO_DDK_

// end_ntddk end_ntminiport end_wdm end_ntifs

//
// Policy manager state handler interfaces
//

// power state handlers

typedef enum {
    PowerStateSleeping1,
    PowerStateSleeping2,
    PowerStateSleeping3,
    PowerStateSleeping4,
    PowerStateSleeping4Firmware,
    PowerStateShutdownReset,
    PowerStateShutdownOff,
    PowerStateMaximum
} POWER_STATE_HANDLER_TYPE, *PPOWER_STATE_HANDLER_TYPE;

typedef
NTSTATUS
(*PENTER_STATE_SYSTEM_HANDLER)(
    IN PVOID                        SystemContext
    );

typedef
NTSTATUS
(*PENTER_STATE_HANDLER)(
    IN PVOID                        Context,
    IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler   OPTIONAL,
    IN PVOID                        SystemContext,
    IN LONG                         NumberProcessors,
    IN volatile PLONG               Number
    );

typedef struct {
    POWER_STATE_HANDLER_TYPE    Type;
    BOOLEAN                     RtcWake;
    UCHAR                       Spare[3];
    PENTER_STATE_HANDLER        Handler;
    PVOID                       Context;
} POWER_STATE_HANDLER, *PPOWER_STATE_HANDLER;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtPowerInformation(
    IN POWER_INFORMATION_LEVEL InformationLevel,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

// processor idle functions

typedef struct {
    ULONGLONG                   StartTime;
    ULONGLONG                   EndTime;
    ULONG                       IdleHandlerReserved[4];
} PROCESSOR_IDLE_TIMES, *PPROCESSOR_IDLE_TIMES;

typedef
BOOLEAN
(FASTCALL *PPROCESSOR_IDLE_HANDLER) (
    IN OUT PPROCESSOR_IDLE_TIMES IdleTimes
    );

typedef struct {
    ULONG                       HardwareLatency;
    PPROCESSOR_IDLE_HANDLER     Handler;
} PROCESSOR_IDLE_HANDLER_INFO, *PPROCESSOR_IDLE_HANDLER_INFO;

typedef
VOID
(FASTCALL *PSET_PROCESSOR_THROTTLE) (
    IN UCHAR                    Throttle
    );

#define MAX_IDLE_HANDLERS       3

typedef struct {
    UCHAR                       ThrottleScale;
    BOOLEAN                     ThrottleOnIdle;
    PSET_PROCESSOR_THROTTLE     SetThrottle;

    ULONG                       NumIdleHandlers;
    PROCESSOR_IDLE_HANDLER_INFO IdleHandler[MAX_IDLE_HANDLERS];
} PROCESSOR_STATE_HANDLER, *PPROCESSOR_STATE_HANDLER;

//
// Power Policy Management interfaces
//

typedef struct {
    POWER_ACTION    Action;
    ULONG           Flags;
    ULONG           EventCode;
} POWER_ACTION_POLICY, *PPOWER_ACTION_POLICY;

// POWER_ACTION_POLICY->Flags:
#define POWER_ACTION_QUERY_ALLOWED      0x00000001
#define POWER_ACTION_UI_ALLOWED         0x00000002
#define POWER_ACTION_OVERRIDE_APPS      0x00000004
#define POWER_ACTION_LIGHTEST_FIRST     0x10000000
#define POWER_ACTION_LOCK_CONSOLE       0x20000000
#define POWER_ACTION_DISABLE_WAKES      0x40000000
#define POWER_ACTION_CRITICAL           0x80000000

// POWER_ACTION_POLICY->EventCode flags
#define POWER_LEVEL_USER_NOTIFY_TEXT    0x00000001
#define POWER_LEVEL_USER_NOTIFY_SOUND   0x00000002
#define POWER_LEVEL_USER_NOTIFY_EXEC    0x00000004


// system battery drain policies
typedef struct {
    BOOLEAN                 Enable;
    UCHAR                   Spare[3];
    ULONG                   BatteryLevel;
    POWER_ACTION_POLICY     PowerPolicy;
    SYSTEM_POWER_STATE      MinSystemState;
} SYSTEM_POWER_LEVEL, *PSYSTEM_POWER_LEVEL;

// Discharge policy constants
#define NUM_DISCHARGE_POLICIES      4
#define DISCHARGE_POLICY_CRITICAL   0
#define DISCHARGE_POLICY_LOW        1

// system power policies
typedef struct _SYSTEM_POWER_POLICY {
    ULONG                   Revision;       // 1

    // events
    POWER_ACTION_POLICY     PowerButton;
    POWER_ACTION_POLICY     SleepButton;
    POWER_ACTION_POLICY     LidClose;
    SYSTEM_POWER_STATE      LidOpenWake;
    ULONG                   Reserved;

    // "system idle" detection
    POWER_ACTION_POLICY     Idle;
    ULONG                   IdleTimeout;
    UCHAR                   IdleSensitivity;
    UCHAR                   Spare2[3];

    // meaning of power action "sleep"
    SYSTEM_POWER_STATE      MinSleep;
    SYSTEM_POWER_STATE      MaxSleep;
    SYSTEM_POWER_STATE      ReducedLatencySleep;
    ULONG                   WinLogonFlags;

    // parameters for dozing
    ULONG                   Spare3;
    ULONG                   DozeS4Timeout;

    // battery policies
    ULONG                   BroadcastCapacityResolution;
    SYSTEM_POWER_LEVEL      DischargePolicy[NUM_DISCHARGE_POLICIES];

    // video policies
    ULONG                   VideoTimeout;
    BOOLEAN                 VideoDimDisplay;
    ULONG                   VideoReserved[3];

    // hard disk policies
    ULONG                   SpindownTimeout;

    // processor policies
    BOOLEAN                 OptimizeForPower;
    UCHAR                   FanThrottleTolerance;
    UCHAR                   ForcedThrottle;
    UCHAR                   MinThrottle;
    POWER_ACTION_POLICY     OverThrottled;

} SYSTEM_POWER_POLICY, *PSYSTEM_POWER_POLICY;

// administrator power policy overrides
typedef struct _ADMINISTRATOR_POWER_POLICY {

    // meaning of power action "sleep"
    SYSTEM_POWER_STATE      MinSleep;
    SYSTEM_POWER_STATE      MaxSleep;

    // video policies
    ULONG                   MinVideoTimeout;
    ULONG                   MaxVideoTimeout;

    // disk policies
    ULONG                   MinSpindownTimeout;
    ULONG                   MaxSpindownTimeout;
} ADMINISTRATOR_POWER_POLICY, *PADMINISTRATOR_POWER_POLICY;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetThreadExecutionState(
    IN EXECUTION_STATE esFlags,               // ES_xxx flags
    OUT EXECUTION_STATE *PreviousFlags
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtRequestWakeupLatency(
    IN LATENCY_TIME latency
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtInitiatePowerAction(
    IN POWER_ACTION SystemAction,
    IN SYSTEM_POWER_STATE MinSystemState,
    IN ULONG Flags,                 // POWER_ACTION_xxx flags
    IN BOOLEAN Asynchronous
    );


NTSYSCALLAPI                        // only called by WinLogon
NTSTATUS
NTAPI
NtSetSystemPowerState(
    IN POWER_ACTION SystemAction,
    IN SYSTEM_POWER_STATE MinSystemState,
    IN ULONG Flags                  // POWER_ACTION_xxx flags
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtGetDevicePowerState(
    IN HANDLE Device,
    OUT DEVICE_POWER_STATE *State
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCancelDeviceWakeupRequest(
    IN HANDLE Device
    );

NTSYSCALLAPI
BOOLEAN
NTAPI
NtIsSystemResumeAutomatic(
    VOID
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtRequestDeviceWakeup(
    IN HANDLE Device
    );



// WinLogonFlags:
#define WINLOGON_LOCK_ON_SLEEP  0x00000001


//
// System power manager capabilities
//

#ifndef _BATCLASS_

typedef struct {
    ULONG       Granularity;
    ULONG       Capacity;
} BATTERY_REPORTING_SCALE, *PBATTERY_REPORTING_SCALE;

#endif

typedef struct {
    // Misc supported system features
    BOOLEAN             PowerButtonPresent;
    BOOLEAN             SleepButtonPresent;
    BOOLEAN             LidPresent;
    BOOLEAN             SystemS1;
    BOOLEAN             SystemS2;
    BOOLEAN             SystemS3;
    BOOLEAN             SystemS4;           // hibernate
    BOOLEAN             SystemS5;           // off
    BOOLEAN             HiberFilePresent;
    BOOLEAN             FullWake;
    BOOLEAN             VideoDimPresent;
    BOOLEAN             ApmPresent;
    BOOLEAN             UpsPresent;

    // Processors
    BOOLEAN             ThermalControl;
    BOOLEAN             ProcessorThrottle;
    UCHAR               ProcessorMinThrottle;
    UCHAR               ProcessorThrottleScale;
    UCHAR               spare2[4];

    // Disk
    BOOLEAN             DiskSpinDown;
    UCHAR               spare3[8];

    // System Battery
    BOOLEAN             SystemBatteriesPresent;
    BOOLEAN             BatteriesAreShortTerm;
    BATTERY_REPORTING_SCALE BatteryScale[3];

    // Wake
    SYSTEM_POWER_STATE  AcOnLineWake;
    SYSTEM_POWER_STATE  SoftLidWake;
    SYSTEM_POWER_STATE  RtcWake;
    SYSTEM_POWER_STATE  MinDeviceWakeState; // note this may change on driver load
    SYSTEM_POWER_STATE  DefaultLowLatencyWake;
} SYSTEM_POWER_CAPABILITIES, *PSYSTEM_POWER_CAPABILITIES;


typedef struct {
    BOOLEAN             AcOnLine;
    BOOLEAN             BatteryPresent;
    BOOLEAN             Charging;
    BOOLEAN             Discharging;
    BOOLEAN             Spare1[4];

    ULONG               MaxCapacity;
    ULONG               RemainingCapacity;
    ULONG               Rate;
    ULONG               EstimatedTime;

    ULONG               DefaultAlert1;
    ULONG               DefaultAlert2;
} SYSTEM_BATTERY_STATE, *PSYSTEM_BATTERY_STATE;


// end_nthal

//
// Power structure in each processors PRCB
//
struct _PROCESSOR_POWER_STATE;      // forward ref

typedef
VOID
(FASTCALL *PPROCESSOR_IDLE_FUNCTION) (
    struct _PROCESSOR_POWER_STATE   *PState
    );

typedef struct _PROCESSOR_POWER_STATE {
    PPROCESSOR_IDLE_FUNCTION    IdleFunction;
    ULONG                       Idle0KernelTimeLimit;
    ULONG                       Idle0LastTime;

    PVOID                       IdleState;
    ULONGLONG                   LastCheck;
    PROCESSOR_IDLE_TIMES        IdleTimes;

    ULONG                       IdleTime1;
    ULONG                       PromotionCheck;
    ULONG                       IdleTime2;

    UCHAR                       CurrentThrottle;    // current throttle setting
    UCHAR                       ThrottleLimit;      // max available throttle setting
    UCHAR                       Spare1[2];

    ULONG                       SetMember;
    PVOID                       AbortThrottle;

// temp for debugging
    ULONGLONG                   DebugDelta;
    ULONG                       DebugCount;

    ULONG                       LastSysTime;
    ULONG                       Spare2[10];
} PROCESSOR_POWER_STATE, *PPROCESSOR_POWER_STATE;


//
// Status information
//

typedef struct _PROCESSOR_POWER_INFORMATION {
    ULONG                   Number;
    ULONG                   MaxMhz;
    ULONG                   CurrentMhz;
    ULONG                   MhzLimit;
    ULONG                   MaxIdleState;
    ULONG                   CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;

typedef struct _SYSTEM_POWER_INFORMATION {
    ULONG                   MaxIdlenessAllowed;
    ULONG                   Idleness;
    ULONG                   TimeRemaining;
    UCHAR                   CoolingMode;
} SYSTEM_POWER_INFORMATION, *PSYSTEM_POWER_INFORMATION;

#ifdef __cplusplus
}
#endif

#endif // _NTPOAPI_

