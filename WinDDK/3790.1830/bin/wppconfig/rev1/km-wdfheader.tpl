`**********************************************************************`
`* This is an include template file for tracewpp preprocessor.        *`
`*                                                                    *`
`*    Copyright 1999-2000 Microsoft Corporation. All Rights Reserved. *`
`**********************************************************************`

// template `TemplateFile`

`* Dump the definitions specified via -D on the command line to WPP *`

`FORALL def IN MacroDefinitions`
#define `def.Name` `def.Alias`
`ENDFOR`

#define WPP_THIS_FILE `SourceFile.CanonicalName`

#include <stddef.h>
#include <stdarg.h>
#include <wmistr.h>

#if defined(WPP_TRACE_W2K_COMPATABILITY)
DEFINE_GUID(WPP_TRACE_CONTROL_NULL_GUID, 0x00000000L, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
#endif

#if !defined(_NTRTL_)
#if !defined(_NTHAL_) 
      // fake RTL_TIME_ZONE_INFORMATION //
    typedef int RTL_TIME_ZONE_INFORMATION;
#endif
#   define _WMIKM_  
#endif
#ifndef WPP_TRACE
#define WPP_TRACE pfnWmiTraceMessage
#endif

__inline 
TRACEHANDLE 
WppQueryLogger(
    PWSTR LoggerName
    )
{
    if (pfnWmiQueryTraceInformation) {
        
        ULONG           ReturnLength;
        NTSTATUS        Status;
        TRACEHANDLE     TraceHandle;
        UNICODE_STRING  Buffer;

        if (!LoggerName) {
            LoggerName = L"stdout";
        }

        RtlInitUnicodeString(&Buffer, LoggerName);

        Status = pfnWmiQueryTraceInformation( TraceHandleByNameClass,
                                              (PVOID) &TraceHandle,
                                              sizeof(TraceHandle),
                                              &ReturnLength,
                                              (PVOID)&Buffer );
        if (Status != STATUS_SUCCESS) {
            return (TRACEHANDLE) 0 ;
        }

        return TraceHandle;

    } else {
        return (TRACEHANDLE) 0 ;
    }
}

typedef NTSTATUS (*WMIENTRY_NEW)(
    IN UCHAR ActionCode,
    IN PVOID DataPath,
    IN ULONG BufferSize,
    IN OUT PVOID Buffer,
    IN PVOID Context,
    OUT PULONG Size
    );



typedef struct _WPP_TRACE_CONTROL_BLOCK
{
    WMIENTRY_NEW                      Callback;
    LPCGUID                           ControlGuid;
    struct _WPP_TRACE_CONTROL_BLOCK * Next;
    UINT64                            Logger;
    LPCWSTR                           FriendlyName;
    UCHAR                             FlagsLen; 
    UCHAR                             Level; 
    USHORT                            Reserved;
    ULONG                             Flags[1];

} WPP_TRACE_CONTROL_BLOCK, *PWPP_TRACE_CONTROL_BLOCK;


typedef struct _WPP_REGISTRATION_BLOCK
{
    WMIENTRY_NEW                      Callback;
    LPCGUID                           ControlGuid;
    struct _WPP_REGISTRATION_BLOCK  * Next;   
    UINT64                            Logger;
    LPCWSTR                           FriendlyName;
    LPCWSTR                           BitNames;
    PUNICODE_STRING                   RegistryPath;
    UCHAR                             FlagsLen;
    UCHAR                             RegBlockLen;

} WPP_REGISTRATION_BLOCK, *PWPP_REGISTRATION_BLOCK;





