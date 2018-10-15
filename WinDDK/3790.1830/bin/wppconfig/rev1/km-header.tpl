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
      // fake RTL_TIME_ZONE_INFORMATION //
    typedef int RTL_TIME_ZONE_INFORMATION;
#   define _WMIKM_  
#endif
#ifndef WPP_TRACE
#define WPP_TRACE WmiTraceMessage
#endif

__inline TRACEHANDLE WppQueryLogger(PWSTR LoggerName)
{
#ifndef WPP_TRACE_W2K_COMPATABILITY
    ULONG ReturnLength ;
    NTSTATUS Status ;
    TRACEHANDLE TraceHandle ;
    UNICODE_STRING  Buffer  ;
       
    if (!LoggerName) {
        LoggerName = L"stdout";
    }

    RtlInitUnicodeString(&Buffer, LoggerName);

    
    if ((Status = WmiQueryTraceInformation(TraceHandleByNameClass,
                                                 (PVOID)&TraceHandle,
                                                  sizeof(TraceHandle),
                                                  &ReturnLength,
                                                  (PVOID)&Buffer)) != STATUS_SUCCESS) {
       return 0 ;
    }
    
    return TraceHandle ;
#else
    return (TRACEHANDLE) 0 ;
#endif  // #ifdef WPP_TRACE_W2K_COMPATABILITY

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
    WMIENTRY_NEW                     Callback;
    struct _WPP_TRACE_CONTROL_BLOCK *Next;

    __int64 Logger;
    UCHAR FlagsLen; UCHAR Level; USHORT Reserved;
    ULONG  Flags[1];
} WPP_TRACE_CONTROL_BLOCK, *PWPP_TRACE_CONTROL_BLOCK;

typedef struct _WPP_REGISTRATION_BLOCK
{
    WMIENTRY_NEW                    Callback;
    struct _WPP_REGISTRATION_BLOCK *Next;   

    LPCGUID ControlGuid;
    LPCWSTR  FriendlyName;
    LPCWSTR  BitNames;
    PUNICODE_STRING RegistryPath;

    UCHAR   FlagsLen, RegBlockLen;
} WPP_REGISTRATION_BLOCK, *PWPP_REGISTRATION_BLOCK;




