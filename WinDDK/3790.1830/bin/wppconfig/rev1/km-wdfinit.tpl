`**********************************************************************`
`* This is an include template file for tracewpp preprocessor.        *`
`*                                                                    *`
`*    Copyright (C) Microsoft Corporation. All Rights Reserved.       *`
`**********************************************************************`

// template `TemplateFile`
//
//     Defines a set of functions that simplifies
//     kernel mode registration for tracing
//
#if !defined(NTSTRSAFE_LIB)
#define NTSTRSAFE_LIB
#endif
#include <ntstrsafe.h>

#if !defined(WppDebug)
#define WppDebug(a,b)
#endif

#define WMIREG_FLAG_CALLBACK        0x80000000 // not exposed in DDK


#ifndef WPPINIT_EXPORT
#define WPPINIT_EXPORT
#endif


#if defined(__cplusplus)
extern "C" {
#endif

PFN_WMIQUERYTRACEINFORMATION pfnWmiQueryTraceInformation = NULL;
PFN_WMITRACEMESSAGEVA        pfnWmiTraceMessageVa        = NULL;
PFN_WMITRACEMESSAGE          pfnWmiTraceMessage          = NULL;

ULONG_PTR   WPP_Global_NextDeviceOffsetInDeviceExtension = (ULONG) -1;

#if defined(__cplusplus)
};
#endif


WPPINIT_EXPORT
VOID
WppIntToHex(
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
LPWSTR
WppGuidToStr(
    LPWSTR  buf, 
    LPCGUID guid
    ) 
{
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
    return buf;
}


VOID
WppLoadAdvancedTracingSupport(
    VOID
    )
{
    UNICODE_STRING name;

    RtlInitUnicodeString(&name, L"WmiTraceMessage");
    pfnWmiTraceMessage = (PFN_WMITRACEMESSAGE) (INT_PTR) 
        MmGetSystemRoutineAddress(&name);

    if (pfnWmiTraceMessage == NULL) {
        //
        // Subsitute the W2K verion of WmiTraceMessage
        //
        pfnWmiTraceMessage = W2kTraceMessage;

        WppDebug(0,("WppLoadAdvancedTracingSupport: "
                    "subsitute pWmiTraceMessage %p\n",
                    pfnWmiTraceMessage));

        return; // no point in continuing.
    }

    RtlInitUnicodeString(&name, L"WmiQueryTraceInformation");
    pfnWmiQueryTraceInformation = (PFN_WMIQUERYTRACEINFORMATION) (INT_PTR) 
        MmGetSystemRoutineAddress(&name);

    RtlInitUnicodeString(&name, L"WmiTraceMessageVa");
    pfnWmiTraceMessageVa = (PFN_WMITRACEMESSAGEVA) (INT_PTR) 
        MmGetSystemRoutineAddress(&name);
}

BOOLEAN
WppIsAdvancedTracingSupported(
    VOID
    )
{
    BOOLEAN answer = (pfnWmiTraceMessageVa) ? TRUE:FALSE;
    return answer;
}


BOOLEAN
WppAreAutoLoggersSupported(
    VOID
    )
{
    if (!WppIsAdvancedTracingSupported() ) {
        return FALSE;
    }

    return FALSE;
}


#ifndef WPP_POOLTYPE
#define WPP_POOLTYPE NonPagedPool
#endif
#ifndef WPP_POOLTAG
#define WPP_POOLTAG     'ECTS'
#endif


#define DEFAULT_GLOBAL_LOGGER_KEY       L"WMI\\GlobalLogger\\"
#define DEFAULT_LOGGER_KEY              L"WMI\\AutoLogger\\"

#define MAX_LOGGERNAMESIZE              32


WPPINIT_EXPORT
VOID
WppInitGlobalLogger(
    PWCHAR         LoggerName,
    LPCGUID        pControlGuid,
    PTRACEHANDLE   pLogger,
    PULONG         pFlags,
    PUCHAR         pLevel 
    )
{
    PWCHAR                     GRegValueName;
    ULONG                      GregValueNameLength = 0;
    RTL_QUERY_REGISTRY_TABLE   parms[3];
    ULONG                      Lflags = 0;
    ULONG                      Llevel = 0;
    ULONG                      Lstart = 0;
    NTSTATUS                   status;
    ULONG                      aZero = 0;
    ULONG                      IsGlobalLogger = FALSE;


    WppDebug(0,("WPP checking \"%S\" Logger\n",LoggerName));
    
    GregValueNameLength = 
        sizeof(WCHAR) * (ULONG)(wcslen(DEFAULT_GLOBAL_LOGGER_KEY) + 
        wcslen(LoggerName) + wcslen(L"\\") +  WPP_TEXTGUID_LEN ) + 
        sizeof(UNICODE_NULL) ;
  
    //Allocate the blob to hold the data
    GRegValueName = (PWCHAR) ExAllocatePoolWithTag( WPP_POOLTYPE,
                                                    GregValueNameLength,
                                                    WPP_POOLTAG) ;
  
    if (!GRegValueName) {
       WppDebug(0,("Could not get memory for Registry Value Name \"%S\"\n",
                   LoggerName));
       return;
    }
    
    if (wcscmp(LoggerName,L"GlobalLogger") == 0) {
        IsGlobalLogger = TRUE;
   
        status = RtlStringCbCopyW( GRegValueName, 
                                   GregValueNameLength,
                                   DEFAULT_GLOBAL_LOGGER_KEY );

        if (!NT_SUCCESS(status)) {
            WppDebug(0,("WPP Failed to make Key name for \"GlobalLogger\"\n"));
            ExFreePool(GRegValueName);
            return ;
        }

    } else {
        ULONG    remainder = GregValueNameLength;
        ULONG    length;
        BOOLEAN  errors = TRUE;

        //
        // Build up the string "WMI\AutoLogger\<LoggerName>\"
        //
        status = RtlStringCbCopyW( GRegValueName, remainder, DEFAULT_LOGGER_KEY );
        if (NT_SUCCESS(status)) {
            length = (sizeof(DEFAULT_LOGGER_KEY) - sizeof(UNICODE_NULL));
            if (remainder >= length) {
                remainder -= length;
                status = RtlStringCbCatW( GRegValueName, remainder, LoggerName );
                length = (ULONG)(wcslen(LoggerName) - 1);
                if (remainder >= length) {
                    if (NT_SUCCESS(status)) {
                        remainder -= (ULONG)(wcslen(LoggerName) - 1);
                        status = RtlStringCbCatW( GRegValueName, remainder, L"\\" );
                        errors = FALSE;
                    }
                }
            }
        }

        if (errors || !NT_SUCCESS(status)) {
            WppDebug(0,("WPP Failed to make Key name for AutoLogger "
                        "\"%S\"\n", LoggerName));
            ExFreePool(GRegValueName);
            return ;
        }
    }

    WppDebug(0,("WPP Made Key name \"%S\" for \"%S\"\n",
                GRegValueName,LoggerName));
    
    //
    // Fill in the query table to find out if the Logger is Started
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
        WppDebug(0,("WPP Auto Logger \"%S\" is not defined in registry: "
                    "\"%S\"\n", LoggerName, GRegValueName));
        ExFreePool(GRegValueName);  
        return ;
    }

    //
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
    parms[2].QueryRoutine  = NULL;
    parms[2].Flags         = 0;
  
    WppGuidToStr( &GRegValueName[(ULONG)wcslen(GRegValueName)], pControlGuid );
  
    //
    // Perform the query
    //
    WppDebug(0,("WPP Querying for \"%S\"\n",GRegValueName));
  
    status = RtlQueryRegistryValues(RTL_REGISTRY_CONTROL | RTL_REGISTRY_OPTIONAL,
                                    GRegValueName,
                                    parms,
                                    NULL,
                                    NULL);
  
    // finished with this avoid having to include on all paths
    ExFreePool(GRegValueName);  
  
    if (NT_SUCCESS(status)) {
         
        if (Lstart==1) {
            
            if (IsGlobalLogger) {
                
                *pLogger= WMI_GLOBAL_LOGGER_ID;
  
            } else {

                if (pfnWmiQueryTraceInformation) {

                    ULONG          ReturnLength = 0;
                    UNICODE_STRING LoggerNameStr;

                    RtlInitUnicodeString( &LoggerNameStr, LoggerName);

                    status = pfnWmiQueryTraceInformation( TraceHandleByNameClass,
                                                          (PVOID)pLogger,
                                                          sizeof(TRACEHANDLE),
                                                          &ReturnLength,
                                                          (PVOID) &LoggerNameStr );
                    if (!NT_SUCCESS(status)) {
                       *pLogger = (TRACEHANDLE) NULL;

                       WppDebug(0,("WPP Could not get TraceHandle for \"%S\", "
                                   "Status 0x%08X\n", LoggerName, status));
                    }
                
                } else {
                    *pLogger = (TRACEHANDLE) NULL;
                    WppDebug(0,("WPP pfnWmiQueryTraceInformation NULL\n"));
                }
            }
  
            *pFlags = Lflags & 0x7FFFFFFF;
            *pLevel = (UCHAR)(Llevel & 0xFF);
            
            if (*pLogger != (TRACEHANDLE) NULL) {
               WppDebug(0,("WPP Enabled GlobalLogger, LoggerId %d, "
                           "Flags 0x%08X, Level %d\n",
                           (USHORT) *pLogger, Lflags, Llevel));
            } else {
               WppDebug(0,("WPP Not enabled as Logger \"%S\" is not active, "
                           "Flags 0x%08X, Level %d\n",
                           LoggerName, Lflags, Llevel));
            }
         }
    } else {
         WppDebug(0,("WPP Auto Logger \"%S\": No Flags/Levels, Status %08X\n",
                     LoggerName,status));
    }                                  
}



WPPINIT_EXPORT
NTSTATUS
WppTraceCallback(
    IN  UCHAR  minorFunction,
    IN  PVOID  DataPath,
    IN  ULONG  BufferLength,
    IN  PVOID  Buffer,
    IN  PVOID  Context,
    OUT PULONG Size
    )
{
    WPP_PROJECT_CONTROL_BLOCK * cb;
    NTSTATUS   status = STATUS_SUCCESS;
    WCHAR      guid [WPP_TEXTGUID_LEN];


    UNREFERENCED_PARAMETER(DataPath);
    UNREFERENCED_PARAMETER(guid);

    WppDebug(0,("WppTraceCallBack: enter: "
                "MinorFunction 0x%02X, Context %p\n", 
                minorFunction, Context));
    
    cb = (WPP_PROJECT_CONTROL_BLOCK*) Context;
    *Size = 0;

    switch(minorFunction)
    {
        case IRP_MN_REGINFO:
        {
            PWMIREGINFOW     wmiRegInfo;
            PCUNICODE_STRING regPath;
            PWCHAR           stringPtr;
            ULONG            registryPathOffset;
            ULONG            bufferNeeded;
            ULONG            guidCount = 0;
            PWCHAR           loggerName;
            
            PWPP_TRACE_CONTROL_BLOCK cntl;

            //
            // Initialize locals 
            //
            regPath = cb->Registration.RegistryPath;

            //
            // Count the number of guid to be identified.
            //
            cntl = (PWPP_TRACE_CONTROL_BLOCK) cb;
            while(cntl) { guidCount++; cntl = cntl->Next; }
            WppDebug(0,("WppTraceCallBack: GUID count %d\n", guidCount)); 

            //
            // Calculate buffer size need to hold all info.
            // Calculate offset to where RegistryPath parm will be copied.
            //
            if (regPath == NULL)
            {
                // No registry path specified. This is a bad thing for
                // the device to do, but is not fatal
				           
                registryPathOffset = 0;

                bufferNeeded = FIELD_OFFSET(WMIREGINFOW, WmiRegGuid) + 
                               guidCount * sizeof(WMIREGGUIDW);
                
            } else {
	            registryPathOffset = FIELD_OFFSET(WMIREGINFOW, WmiRegGuid) + 
                                     guidCount * sizeof(WMIREGGUIDW);

	            bufferNeeded = registryPathOffset +
                               regPath->Length + sizeof(USHORT);
            }            

            //
            // If the provided buffer is large enough, then fill with info.
            //
            if (bufferNeeded <= BufferLength)
            {
                ULONG  i;

				RtlZeroMemory(Buffer, BufferLength);

                wmiRegInfo = (PWMIREGINFO)Buffer;

                wmiRegInfo->BufferSize   = bufferNeeded;
                wmiRegInfo->RegistryPath = registryPathOffset;
                wmiRegInfo->GuidCount    = guidCount;

                cntl = (PWPP_TRACE_CONTROL_BLOCK) cb;

                for (i=0; i<guidCount; i++) {

                    wmiRegInfo->WmiRegGuid[i].Guid  = *cntl->ControlGuid;  
                    wmiRegInfo->WmiRegGuid[i].Flags = WMIREG_FLAG_TRACE_CONTROL_GUID | 
                                                      WMIREG_FLAG_TRACED_GUID;

                    WppDebug(0,("WppTraceCallBack: GUID[%d] {%S}\n", i,
                                WppGuidToStr((LPWSTR)&guid, 
                                    &wmiRegInfo->WmiRegGuid[i].Guid)));

                    cntl = cntl->Next;
                }

                if (regPath != NULL) {
                    stringPtr    = (PWCHAR)((PUCHAR)Buffer + registryPathOffset);
                    *stringPtr++ = regPath->Length;
                    
                    RtlCopyMemory(stringPtr, regPath->Buffer, regPath->Length);
                }

                status = STATUS_SUCCESS;
                *Size  = bufferNeeded;

            } else {
                status = STATUS_BUFFER_TOO_SMALL;

                if (BufferLength >= sizeof(ULONG)) {
                    *((PULONG)Buffer) = bufferNeeded;
                    *Size = sizeof(ULONG);
                }
            }

            if (WppAreAutoLoggersSupported() == TRUE) {
                //
                // Get the AutoLogger's name from the registry
                //
                loggerName = NULL;   // REVIEW: get name from registry
                
            } else {
                //
                // Use the GlobalLogger for W2K, XP, WS2003.
                //
                loggerName = L"GlobalLogger";
            }

            //
            // Check if GlobalLogger/AutoLogger is active            
            //
            if (loggerName) {

                WppInitGlobalLogger( loggerName,
                                     cb->Registration.ControlGuid,
                                     (PTRACEHANDLE)&cb->Control.Logger,
                                     &cb->Control.Flags[0],
                                     &cb->Control.Level );
            } else {

                WppDebug(0,("WppTraceCallBack: no service name found\n"));
            }

            break;
        }

        case IRP_MN_ENABLE_EVENTS:
        case IRP_MN_DISABLE_EVENTS:
        {
            PWNODE_HEADER             Wnode;
            ULONG                     Level;
            ULONG                     ReturnLength;
            ULONG                     index;
            PWPP_TRACE_CONTROL_BLOCK  this;

            if (cb == NULL ) {
                status = STATUS_WMI_GUID_NOT_FOUND;
                break;
            }

            //
            // Initialize locals 
            //
            Wnode = (PWNODE_HEADER)Buffer;
            
            //
            // Traverse this ProjectControlBlock's ControlBlock list and 
            // find the "this" ControlBlock which matches the Wnode GUID.
            //
            this  = (PWPP_TRACE_CONTROL_BLOCK) cb;
            index = 0;
            while(this) { 
                if (RtlEqualLuid( (LUID*)this->ControlGuid, (LUID*)&Wnode->Guid )) {
                    break;
                }
                index++;
                this = this->Next; 
            }

            if (this == NULL) {
                status = STATUS_WMI_GUID_NOT_FOUND;
                break;
            }

            WppDebug(0,("WppTraceCallBack: event for GUID[%d] {%S}\n",
                        index, WppGuidToStr((LPWSTR)&guid, this->ControlGuid)));

            //
            // Do the requested event action
            //
            if (BufferLength >= sizeof(WNODE_HEADER)) {
                status = STATUS_SUCCESS;

                if (minorFunction == IRP_MN_DISABLE_EVENTS) {

                    WppDebug(0,("WppTraceCallBack: DISABLE_EVENTS\n")); 

                    this->Level    = 0;
                    this->Flags[0] = 0;
                    this->Logger   = 0;

                } else {

                    TRACEHANDLE  lh;

                    lh = (TRACEHANDLE)( Wnode->HistoricalContext );
                    this->Logger = lh;

                    if (WppIsAdvancedTracingSupported()) {

                        status = pfnWmiQueryTraceInformation( TraceEnableLevelClass,
                                                              &Level,
                                                              sizeof(Level),
                                                              &ReturnLength,
                                                              (PVOID) Wnode );

                        if (status == STATUS_SUCCESS) {
                            this->Level = (UCHAR)Level;
                        }

                        status = pfnWmiQueryTraceInformation( TraceEnableFlagsClass,
                                                              &this->Flags[0],
                                                              sizeof(this->Flags[0]),
                                                              &ReturnLength,
                                                              (PVOID) Wnode );

                    } else {
                        this->Flags[0] = ((PTRACE_ENABLE_CONTEXT) &lh)->EnableFlags;
                        this->Level = (UCHAR) ((PTRACE_ENABLE_CONTEXT) &lh)->Level;
                    }

                    WppDebug(0,("WppTraceCallBack: ENABLE_EVENTS "
                                "LoggerId %d, Flags 0x%08X, Level 0x%02X\n",
                                (USHORT) this->Logger,
                                this->Flags[0],
                                this->Level));

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
    return(status);
}


WPPINIT_EXPORT
VOID
WppInitKm(
    IN     PDRIVER_OBJECT           DriverObject,
    IN     PUNICODE_STRING          RegistryPath,
    IN OUT WPP_REGISTRATION_BLOCK * WppReg
    )
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    WppReg->Callback     = WppTraceCallback;
    WppReg->RegistryPath = NULL;

#if 0  // REVIEW this
    {
        WPP_TRACE_CONTROL_BLOCK * cb;

        cb = (WPP_TRACE_CONTROL_BLOCK*) WppReg;

        cb->FlagsLen = WppReg->FlagsLen;
        cb->Level    = 0;
        cb->Flags[0] = 0;
    }
#endif
    status = IoWMIRegistrationControl( (PDEVICE_OBJECT) WppReg,
                                        WMIREG_ACTION_REGISTER | 
                                        WMIREG_FLAG_CALLBACK );

    WppDebug(0,("WppInitKm: REGISTER WppReg %p, status %08X\n",
                WppReg, status));
}


WPPINIT_EXPORT
VOID
WppCleanupKm(
    PDRIVER_OBJECT          DriverObject, 
    PWPP_REGISTRATION_BLOCK Registration 
    )
{
    NTSTATUS status;

    if (WppIsAdvancedTracingSupported()) {

        status = IoWMIRegistrationControl( (PDEVICE_OBJECT) Registration, 
                                           WMIREG_ACTION_DEREGISTER | 
                                           WMIREG_FLAG_CALLBACK );

        WppDebug(0,("WppCleanupKm: DEREGISTER WppReg %p, status %08X\n", 
                    Registration, status ));
    } else {

        PDEVICE_OBJECT * ppDevObj;

        ppDevObj = (PDEVICE_OBJECT*) 
            IoGetDriverObjectExtension( DriverObject, (PVOID) WDF_TRACE_ID );

        if (ppDevObj == NULL) {
            WppDebug(0,("WppCleanupKm: WDF Trace DeviceObject not available\n"));
            return;
        }

        status = IoWMIRegistrationControl( *ppDevObj, WMIREG_ACTION_DEREGISTER );

        //
        // NOTE: A status of  STATUS_INVALID_PARAMETER is returned for the
        //       case of a double DEREGISTER.  This is benign, WDF already 
        //       DEREGISTERed on unload.
        //
        WppDebug(0,("WppCleanupKm: DEREGISTER pDevObj %p, status %08X\n", 
                    *ppDevObj, status ));
    }
}


