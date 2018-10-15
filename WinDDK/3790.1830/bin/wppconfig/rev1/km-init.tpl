`**********************************************************************`
`* This is an include template file for tracewpp preprocessor.        *`
`*                                                                    *`
`*    Copyright 1999-2001 Microsoft Corporation. All Rights Reserved. *`
`**********************************************************************`

// template `TemplateFile`
//
//     Defines a set of functions that simplifies
//     kernel mode registration for tracing
//

#if !defined(WppDebug)
#  define WppDebug(a,b)
#endif

#define WMIREG_FLAG_CALLBACK        0x80000000 // not exposed in DDK


#ifndef WPPINIT_EXPORT
#  define WPPINIT_EXPORT
#endif

#ifdef WPP_GLOBALLOGGER
WPPINIT_EXPORT
void WppIntToHex(
    LPWSTR Buf,
    unsigned int value,
    int digits
    )
{
    static LPCWSTR hexDigit = L"0123456789abcdef";
    while (--digits >= 0) {
        Buf[digits] = hexDigit[ value & 15 ];
        value /= 16; // compiler is smart enough to change it to bitshift
    }
}

#define WPP_TEXTGUID_LEN 37

// b1e5deaf-1524-4a04-82c4-c9dfbce6cf97<NULL>
// 0         1         2         3
// 0123456789012345678901234567890123456
WPPINIT_EXPORT
void WppGuidToStr(LPWSTR buf, LPCGUID guid) {
    WppIntToHex(buf + 0, guid->Data1, 8);
    buf[8]  = '-';
    WppIntToHex(buf + 9, guid->Data2, 4);
    buf[13] = '-';
    WppIntToHex(buf + 14, guid->Data3, 4);
    buf[18] = '-';
    WppIntToHex(buf + 19, guid->Data4[0], 2);
    WppIntToHex(buf + 21, guid->Data4[1], 2);
    buf[23] = '-';
    WppIntToHex(buf + 24, guid->Data4[2], 2);
    WppIntToHex(buf + 26, guid->Data4[3], 2);
    WppIntToHex(buf + 28, guid->Data4[4], 2);
    WppIntToHex(buf + 30, guid->Data4[5], 2);
    WppIntToHex(buf + 32, guid->Data4[6], 2);
    WppIntToHex(buf + 34, guid->Data4[7], 2);
    buf[36] = 0;
}

#define GREGVALUENAMELENGTH 18 + WPP_TEXTGUID_LEN + 1 // wslen(L"WMI\\GlobalLogger\\") + GUIDLENGTH

WPPINIT_EXPORT
void WppInitGlobalLogger(
        LPCGUID pControlGuid,
        PTRACEHANDLE pLogger,
        PULONG pFlags,
        PUCHAR pLevel )
{
WCHAR                    GRegValueName[GREGVALUENAMELENGTH] ; // L"WMI\\GlobalLogger\\d58c126f-b309-11d1-969e-0000f875a5bc" ;
RTL_QUERY_REGISTRY_TABLE   parms[3];
ULONG                      Lflags = 0,
                           Llevel = 0,
                           Lstart = 0;
NTSTATUS                   status ;
ULONG                      aZero = 0 ;


   WppDebug(0,("WPP checking Global Logger"));
   
#ifdef WPP_STRSAFE
#include <ntstrsafe.h>
        status = RtlStringCbPrintfW(GRegValueName,GREGVALUENAMELENGTH,L"%ws",L"WMI\\GlobalLogger\\");
        if (!NT_SUCCESS(status)) {
           WppDebug(0,("WPP Create of Registry name failed 0x%X"));
           return ;
        }
#else
        swprintf(GRegValueName,L"WMI\\GlobalLogger\\");
#endif

   //
   // Fill in the query table to find out if the Global Logger is Started
   //
   // Trace Flags
      parms[0].QueryRoutine  = NULL ;
      parms[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
      parms[0].Name          = L"Start";
      parms[0].EntryContext  = &Lstart;
      parms[0].DefaultType   = REG_DWORD;
      parms[0].DefaultData   = &aZero;
      parms[0].DefaultLength = sizeof(ULONG);
      // Termination
      parms[1].QueryRoutine  = NULL ;
      parms[1].Flags         = 0 ;
   //
   // Perform the query
   //

   status = RtlQueryRegistryValues(RTL_REGISTRY_CONTROL | RTL_REGISTRY_OPTIONAL,
                                   GRegValueName,
                                   parms,
                                   NULL,
                                   NULL);
    if (!NT_SUCCESS(status) || Lstart == 0 ) {  
        return ;
    }
    // Fill in the query table to find out if we should use the Global logger
    //
      // Trace Flags
      parms[0].QueryRoutine  = NULL ;
      parms[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
      parms[0].Name          = L"Flags";
      parms[0].EntryContext  = &Lflags;
      parms[0].DefaultType   = REG_DWORD;
      parms[0].DefaultData   = &aZero;
      parms[0].DefaultLength = sizeof(ULONG);
      // Trace level
      parms[1].QueryRoutine  = NULL ;
      parms[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
      parms[1].Name          = L"Level";
      parms[1].EntryContext  = &Llevel;
      parms[1].DefaultType   = REG_DWORD;
      parms[1].DefaultData   = &aZero;
      parms[1].DefaultLength = sizeof(UCHAR);
      // Termination
      parms[2].QueryRoutine  = NULL ;
      parms[2].Flags         = 0 ;

 
   WppGuidToStr(&GRegValueName[wcslen(GRegValueName)], pControlGuid) ;       

   //
   // Perform the query
   //

   status = RtlQueryRegistryValues(RTL_REGISTRY_CONTROL | RTL_REGISTRY_OPTIONAL,
                                   GRegValueName,
                                   parms,
                                   NULL,
                                   NULL);
   if (NT_SUCCESS(status)) {
        if (Lstart==1) {
           *pLogger= WMI_GLOBAL_LOGGER_ID ;
           *pFlags = Lflags & 0x7FFFFFFF ;
           *pLevel = (UCHAR)(Llevel & 0xFF) ;
           WppDebug(0,("WPP Enabled via Global Logger Flags=0x%08X Level=0x%02X",Lflags,Llevel));

        }
   }                                   
}
#endif  //#ifdef WPP_GLOBALLOGGER

WPPINIT_EXPORT
NTSTATUS
WppTraceCallback(
    IN UCHAR minorFunction,
    IN PVOID DataPath,
    IN ULONG BufferLength,
    IN PVOID Buffer,
    IN PVOID Context,
    OUT PULONG Size
    )
/*++

Routine Description:

    Callback routine for IoWMIRegistrationControl.

Arguments:

Return Value:

    status

Comments:

    if return value is STATUS_BUFFER_TOO_SMALL and BufferLength >= 4,
    then first ulong of buffer contains required size


--*/

{
    WPP_PROJECT_CONTROL_BLOCK *cb = (WPP_PROJECT_CONTROL_BLOCK*)Context;
    NTSTATUS                   status = STATUS_SUCCESS;
#if defined(WPP_TRACE_W2K_COMPATABILITY)
    static LPCGUID regGuid = NULL;
#endif

    UNREFERENCED_PARAMETER(DataPath);

    WppDebug(0,("WppTraceCallBack 0x%08X %p\n", minorFunction, Context));
    
    *Size = 0;

    switch(minorFunction)
    {
        case IRP_MN_REGINFO:
        {
            PWMIREGINFOW wmiRegInfo;
            PCUNICODE_STRING regPath;
            PWCHAR stringPtr;
            ULONG registryPathOffset;
            ULONG bufferNeeded;
            
#if defined(WPP_TRACE_W2K_COMPATABILITY)

            wmiRegInfo = (PWMIREGINFO)Buffer;
            
            // Replace the null with the driver's trace control GUID	    
            // regGuid is initialized the first time, so the GUID is saved for the next 
            // IRP_MN_REGINFO call
            if (regGuid == NULL) {
                    regGuid = cb->Registration.ControlGuid;          
            }
            
            if (wmiRegInfo->GuidCount >= 1) {
                // Replace the null trace GUID with the driver's trace control GUID	    
                wmiRegInfo->WmiRegGuid[wmiRegInfo->GuidCount-1].Guid  = *regGuid;
                wmiRegInfo->WmiRegGuid[wmiRegInfo->GuidCount-1].Flags =
                    WMIREG_FLAG_TRACE_CONTROL_GUID | WMIREG_FLAG_TRACED_GUID;
                *Size= wmiRegInfo->BufferSize;
                status = STATUS_SUCCESS;
#ifdef WPP_GLOBALLOGGER
                // Check if Global logger is active            
                WppInitGlobalLogger(
#if defined(WPP_TRACE_W2K_COMPATABILITY)
                                regGuid,
#else
                                cb->Registration.ControlGuid,
#endif
                                (PTRACEHANDLE)&cb->Control.Logger,
                                &cb->Control.Flags[0],
                                &cb->Control.Level);
#endif  //#ifdef WPP_GLOBALLOGGER
	            break;
	        }
#endif

            regPath   = cb->Registration.RegistryPath;

            if (regPath == NULL)
            {
                // No registry path specified. This is a bad thing for
                // the device to do, but is not fatal

				registryPathOffset = 0;
                bufferNeeded = FIELD_OFFSET(WMIREGINFOW, WmiRegGuid)
                               + 1 * sizeof(WMIREGGUIDW);
            }
			else {
				registryPathOffset = FIELD_OFFSET(WMIREGINFOW, WmiRegGuid)
                               + 1 * sizeof(WMIREGGUIDW);

				bufferNeeded = registryPathOffset +
					regPath->Length + sizeof(USHORT);
            }

            if (bufferNeeded <= BufferLength)
            {
				RtlZeroMemory(Buffer, BufferLength);

                wmiRegInfo = (PWMIREGINFO)Buffer;
                wmiRegInfo->BufferSize   = bufferNeeded;
                wmiRegInfo->RegistryPath = registryPathOffset;
                wmiRegInfo->GuidCount    = 1;
                
#if defined(WPP_TRACE_W2K_COMPATABILITY)
                wmiRegInfo->WmiRegGuid[0].Guid  = *regGuid;
#else
                wmiRegInfo->WmiRegGuid[0].Guid = *cb->Registration.ControlGuid;

#endif // #ifdef WPP_TRACE_W2K_COMPATABILITY

                wmiRegInfo->WmiRegGuid[0].Flags =
                    WMIREG_FLAG_TRACE_CONTROL_GUID | WMIREG_FLAG_TRACED_GUID;

				if (regPath != NULL) {
					stringPtr = (PWCHAR)((PUCHAR)Buffer + registryPathOffset);
					*stringPtr++ = regPath->Length;
					RtlCopyMemory(stringPtr,
                          regPath->Buffer,
                          regPath->Length);

				}
                status = STATUS_SUCCESS;
                *Size = bufferNeeded;
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
                if (BufferLength >= sizeof(ULONG)) {
                    *((PULONG)Buffer) = bufferNeeded;
                    *Size = sizeof(ULONG);
                }
            }

#ifdef WPP_GLOBALLOGGER
            // Check if Global logger is active            
            WppInitGlobalLogger(
#if defined(WPP_TRACE_W2K_COMPATABILITY)
                                regGuid,
#else
                                cb->Registration.ControlGuid,
#endif
                                (PTRACEHANDLE)&cb->Control.Logger,
                                &cb->Control.Flags[0],
                                &cb->Control.Level);
#endif  //#ifdef WPP_GLOBALLOGGER
            
            break;
        }

        case IRP_MN_ENABLE_EVENTS:
        case IRP_MN_DISABLE_EVENTS:
        {
            PWNODE_HEADER            Wnode = (PWNODE_HEADER)Buffer;
            ULONG                    Level;
            ULONG                    ReturnLength ;

            if (cb == NULL )
            {
                status = STATUS_WMI_GUID_NOT_FOUND;
                break;
            }

            if (BufferLength >= sizeof(WNODE_HEADER)) {
                status = STATUS_SUCCESS;

                if (minorFunction == IRP_MN_DISABLE_EVENTS) {
                    cb->Control.Level = 0;
                    cb->Control.Flags[0] = 0;
                    cb->Control.Logger = 0;
                } else {
                    TRACEHANDLE    lh;
                    lh = (TRACEHANDLE)( Wnode->HistoricalContext );
                    cb->Control.Logger = lh;
#if !defined(WPP_TRACE_W2K_COMPATABILITY)
                    if ((status = WmiQueryTraceInformation( TraceEnableLevelClass,
                                                           &Level,
                                                           sizeof(Level),
                                                           &ReturnLength,
                                                           (PVOID)Wnode)) == STATUS_SUCCESS)
                    {
                        cb->Control.Level = (UCHAR)Level;
                    }

                    status = WmiQueryTraceInformation( TraceEnableFlagsClass,
                                                       &cb->Control.Flags[0],
                                                       sizeof(cb->Control.Flags[0]),
                                                       &ReturnLength,
                                                       (PVOID)Wnode);
#else //  #ifndef WPP_TRACE_W2K_COMPATABILITY
                    cb->Control.Flags[0] = WmiGetLoggerEnableFlags(lh) ;
                    cb->Control.Level = (UCHAR)WmiGetLoggerEnableLevel(lh) ;
                    WppDebug(0,("Enable/Disable Logger = %p, Flags = 0x%8x, Level = %x08X\n",
                        cb->Control.Logger,cb->Control.Flags[0],cb->Control.Level));
#endif // #ifndef WPP_TRACE_W2K_COMPATABILITY
                }
            } else {
                status = STATUS_INVALID_PARAMETER;
            }

            break;
        }

        case IRP_MN_ENABLE_COLLECTION:
        case IRP_MN_DISABLE_COLLECTION:
        {
            status = STATUS_SUCCESS;
            break;
        }

        case IRP_MN_QUERY_ALL_DATA:
        case IRP_MN_QUERY_SINGLE_INSTANCE:
        case IRP_MN_CHANGE_SINGLE_INSTANCE:
        case IRP_MN_CHANGE_SINGLE_ITEM:
        case IRP_MN_EXECUTE_METHOD:
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

        default:
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

    }
//    DbgPrintEx(XX_FLTR, DPFLTR_TRACE_LEVEL,
//        "%!FUNC!(%!SYSCTRL!) => %!status! (size = %d)", minorFunction, status, *Size);
    return(status);
}

WPPINIT_EXPORT
void WppInitKm(
#if defined(WPP_TRACE_W2K_COMPATABILITY)
    IN PDEVICE_OBJECT pDevObject,
#endif // #if defined(WPP_TRACE_W2K_COMPATABILITY)
    IN PUNICODE_STRING RegistryPath,
    IN OUT WPP_REGISTRATION_BLOCK* WppReg
    )
{
    RegistryPath; // unused
    while(WppReg) {
        WPP_TRACE_CONTROL_BLOCK *cb = (WPP_TRACE_CONTROL_BLOCK*)WppReg;
        NTSTATUS status ;
#if !defined(WPP_TRACE_W2K_COMPATABILITY)
        WppReg -> Callback = WppTraceCallback;
#else // #if !defined(WPP_TRACE_W2K_COMPATABILITY)
        WppReg -> Callback = NULL ;
#endif // #if !defined(WPP_TRACE_W2K_COMPATABILITY)
        WppReg -> RegistryPath = NULL;
        cb -> FlagsLen = WppReg -> FlagsLen;
        cb -> Level = 0;
        cb -> Flags[0] = 0;
#if !defined(WPP_TRACE_W2K_COMPATABILITY)
         status = IoWMIRegistrationControl((PDEVICE_OBJECT)WppReg,
            WMIREG_ACTION_REGISTER | WMIREG_FLAG_CALLBACK
#else  // #if !defined(WPP_TRACE_W2K_COMPATABILITY)
        status = IoWMIRegistrationControl(pDevObject,
            WMIREG_ACTION_REGISTER
#endif // #if !defined(WPP_TRACE_W2K_COMPATABILITY)
        );
        WppDebug(0,("IoWMIRegistrationControl status = %08X\n"));
        WppReg = WppReg->Next;
    }
}

#if !defined(WPP_TRACE_W2K_COMPATABILITY)
WPPINIT_EXPORT
void WppCleanupKm(
    WPP_REGISTRATION_BLOCK* WppReg
    )
{
    while (WppReg) {
        IoWMIRegistrationControl((PDEVICE_OBJECT)WppReg, WMIREG_ACTION_DEREGISTER | WMIREG_FLAG_CALLBACK );
            WppReg = WppReg -> Next;
    }
}
#else  // #if !defined(WPP_TRACE_W2K_COMPATABILITY)
WPPINIT_EXPORT
void WppCleanupKm(
        PDEVICE_OBJECT     pDO
    )
{
        IoWMIRegistrationControl(pDO, WMIREG_ACTION_DEREGISTER );
}
#endif // #if !defined(WPP_TRACE_W2K_COMPATABILITY)

#if !defined(WPP_TRACE_W2K_COMPATABILITY)
#define WPP_SYSTEMCONTROL(PDO)
#define WPP_SYSTEMCONTROL2(PDO, offset)
#else  // #if !defined(WPP_TRACE_W2K_COMPATABILITY)

ULONG_PTR WPP_Global_NextDeviceOffsetInDeviceExtension = -1;

#define WPP_SYSTEMCONTROL(PDO) \
        PDO->MajorFunction[ IRP_MJ_SYSTEM_CONTROL ] = WPPSystemControlDispatch;
#define WPP_SYSTEMCONTROL2(PDO, offset) \
        WPP_SYSTEMCONTROL(PDO); WPP_Global_NextDeviceOffsetInDeviceExtension = (ULONG_PTR)offset;

#ifdef __cplusplus
extern "C"
{
#endif __cplusplus

// Routine to handle the System Control in W2K
NTSTATUS
WPPSystemControlDispatch(
    IN PDEVICE_OBJECT pDO,
    IN PIRP Irp
    );

#ifdef __cplusplus
}
#endif __cplusplus


#ifdef ALLOC_PRAGMA
    #pragma alloc_text( PAGE, WPPSystemControlDispatch)
#endif // ALLOC_PRAGMA

// Routine to handle the System Control in W2K
NTSTATUS
WPPSystemControlDispatch(
    IN PDEVICE_OBJECT pDO,
    IN PIRP Irp
    )
{

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG BufferSize = irpSp->Parameters.WMI.BufferSize;
    PVOID Buffer = irpSp->Parameters.WMI.Buffer;
    ULONG ReturnSize = 0;
    NTSTATUS Status = STATUS_SUCCESS;
    PWNODE_HEADER Wnode=NULL;
    HANDLE ThreadHandle;

    WppDebug(0,("WPPSYSTEMCONTROL\n"));

    if (pDO == (PDEVICE_OBJECT)irpSp->Parameters.WMI.ProviderId) {
#if defined(WPP_TRACE_W2K_COMPATABILITY)
		//To differentiate between the case where wmilib has already filled in parts of the buffer
		if (irpSp->MinorFunction == IRP_MN_REGINFO) RtlZeroMemory(Buffer, BufferSize);
#endif
        Status = WppTraceCallback((UCHAR)(irpSp->MinorFunction),
                                             NULL,
                                             BufferSize,
                                             Buffer,
                                             &WPP_CB[0],
                                        &ReturnSize);

        WppDebug(0,("WPPSYSTEMCONTROL Status 0x%08X\n",Status));

        Irp->IoStatus.Status = Status;
        Irp->IoStatus.Information = ReturnSize;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return Status;
    } else if (WPP_Global_NextDeviceOffsetInDeviceExtension != -1) {

        ULONG_PTR t;

        WppDebug(0,("WPPSYSTEMCONTROL - not for us\n"));

        //
        // Set current stack back one.
        //
        IoSkipCurrentIrpStackLocation( Irp );
        //
        // Pass the call to the next driver.
        //
        t = (ULONG_PTR)pDO->DeviceExtension;
        t += WPP_Global_NextDeviceOffsetInDeviceExtension;
        return IoCallDriver(*(PDEVICE_OBJECT*)t,Irp);

    } else {
        //unable to pass down -- what to do?
        //don't change irp status - IO defaults to failure
        return Irp->IoStatus.Status;
    }
}

#endif // #if !defined(WPP_TRACE_W2K_COMPATABILITY)

