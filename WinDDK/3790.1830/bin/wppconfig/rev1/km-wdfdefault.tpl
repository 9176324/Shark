`**********************************************************************`
`* This is a template file for tracewpp preprocessor                  *`
`* If you need to use a custom version of this file in your project   *`
`* Please clone it from this one and point WPP to it by specifying    *`
`* -gen:{yourfile} option on RUN_WPP line in your sources file        *`
`*                                                                    *`
`*    Copyright 1999-2000 Microsoft Corporation. All Rights Reserved. *`
`**********************************************************************`
//`Compiler.Checksum` Generated File. Do not edit.
// File created by `Compiler.Name` compiler version `Compiler.Version`-`Compiler.Timestamp`
// on `System.Date` at `System.Time` UTC from a template `TemplateFile`

//
// Define anything which is needs but missing from 
// older versions of the DDK.
//
#include <evntrace.h>

#if !defined(TRACE_LEVEL_NONE)
  #define TRACE_LEVEL_NONE        0  
  #define TRACE_LEVEL_CRITICAL    1  
  #define TRACE_LEVEL_FATAL       1  
  #define TRACE_LEVEL_ERROR       2  
  #define TRACE_LEVEL_WARNING     3  
  #define TRACE_LEVEL_INFORMATION 4  
  #define TRACE_LEVEL_VERBOSE     5  
  #define TRACE_LEVEL_RESERVED6   6
  #define TRACE_LEVEL_RESERVED7   7
  #define TRACE_LEVEL_RESERVED8   8
  #define TRACE_LEVEL_RESERVED9   9
#endif
    
#if !defined(TRACE_INFORMATION_CLASS_DEFINE)
typedef enum _TRACE_INFORMATION_CLASS {
    TraceIdClass,
    TraceHandleClass,
    TraceEnableFlagsClass,
    TraceEnableLevelClass,
    GlobalLoggerHandleClass,
    EventLoggerHandleClass,
    AllLoggerHandlesClass,
    TraceHandleByNameClass
} TRACE_INFORMATION_CLASS;
  
#define TRACE_MESSAGE_SEQUENCE               1
#define TRACE_MESSAGE_GUID                   2         
#define TRACE_MESSAGE_COMPONENTID            4           
#define TRACE_MESSAGE_TIMESTAMP              8         
#define TRACE_MESSAGE_PERFORMANCE_TIMESTAMP 16  
#define TRACE_MESSAGE_SYSTEMINFO            32   
       
#else // !defined(TRACE_INFORMATION_CLASS_DEFINE)

#define KERNEL_LOGGER_ID    0xFFFF    // USHORT only
typedef struct _TRACE_ENABLE_CONTEXT {
    USHORT  LoggerId;           // Actual Id of the logger
    UCHAR   Level;              // Enable level passed by control caller
    UCHAR   InternalFlag;       // Reserved
    ULONG   EnableFlags;        // Enable flags passed by control caller
} TRACE_ENABLE_CONTEXT, *PTRACE_ENABLE_CONTEXT;

#endif // !defined(TRACE_INFORMATION_CLASS_DEFINE)


#//
// Advanced tracing APIs (XP and later) will be indirectly called.
//
typedef
NTSTATUS
(*PFN_WMIQUERYTRACEINFORMATION) (
    IN  TRACE_INFORMATION_CLASS TraceInformationClass,
    OUT PVOID  TraceInformation,
    IN  ULONG  TraceInformationLength,
    OUT PULONG RequiredLength OPTIONAL,
    IN  PVOID  Buffer OPTIONAL
    );

typedef
NTSTATUS
(*PFN_WMITRACEMESSAGEVA)(
    IN TRACEHANDLE  LoggerHandle,
    IN ULONG        MessageFlags,
    IN LPGUID       MessageGuid,
    IN USHORT       MessageNumber,
    IN va_list      MessageArgList
    );

typedef
NTSTATUS
(*PFN_WMITRACEMESSAGE)(
    IN TRACEHANDLE  LoggerHandle,
    IN ULONG   MessageFlags,
    IN LPGUID  MessageGuid,
    IN USHORT  MessageNumber,
    IN ...
    );

extern PFN_WMIQUERYTRACEINFORMATION pfnWmiQueryTraceInformation;
extern PFN_WMITRACEMESSAGEVA        pfnWmiTraceMessageVa;
extern PFN_WMITRACEMESSAGE          pfnWmiTraceMessage;


`INCLUDE km-WdfHeader.tpl` 
`INCLUDE km-WdfControl.tpl`
`INCLUDE trmacro.tpl`

`IF FOUND WPP_INIT_TRACING`
#define WPPINIT_EXPORT 
  `INCLUDE km-WdfW2k.tpl` 
  `INCLUDE km-WdfInit.tpl`
`ENDIF`

