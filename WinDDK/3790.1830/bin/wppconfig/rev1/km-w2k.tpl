`**********************************************************************`
`* This is a template file for tracewpp preprocessor                  *`
`* If you need to use a custom version of this file in your project   *`
`* Please clone it from this one and point WPP to it by specifying    *`
`* -gen:{yourfile} option on RUN_WPP line in your sources file        *`
`*                                                                    *`
`* This specific header is km-w2k.tpl and is used to define tracing   *`
`* applications which must also run under Windows 2000                *`
`*                                                                    *`
`*    Copyright 1999-2001 Microsoft Corporation. All Rights Reserved. *`
`**********************************************************************`
//`Compiler.Checksum` Generated File. Do not edit.
// File created by `Compiler.Name` compiler version `Compiler.Version`-`Compiler.Timestamp`
// on `System.Date` at `System.Time` UTC from a template `TemplateFile`

#define WPP_TRACE_W2K_COMPATABILITY
#define WPP_TRACE W2kTraceMessage

#ifdef WPP_TRACE_W2K_COMPATABILITY

#define WPP_TRACE_OPTIONS       0

#pragma warning(disable: 4201)
#include <stddef.h>
#include <stdarg.h>
#include <ntddk.h>
#include <wmistr.h>
#include <evntrace.h>

#ifndef WPP_POOLTAG
#define WPP_POOLTAG     'ECTS'
#endif

#ifndef WPP_POOLTYPE
#define WPP_POOLTYPE PagedPool
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef WPP_MAX_MOF_FIELDS
#define WPP_MAX_MOF_FIELDS 8
#endif

typedef struct _TRACE_BUFFER {
    EVENT_TRACE_HEADER Header;
    MOF_FIELD MofField[WPP_MAX_MOF_FIELDS+1];
} TRACE_BUFFER;

#ifndef TRACE_MESSAGE_MAXIMUM_SIZE
#define TRACE_MESSAGE_MAXIMUM_SIZE  8*1024
#endif

NTSTATUS W2kTraceMessage(IN TRACEHANDLE  LoggerHandle, IN ULONG TraceOptions, IN LPGUID MessageGuid, IN USHORT MessageNumber, ...) ;

#if defined(__cplusplus)
};
#endif	

`IF FOUND WPP_INIT_TRACING`
NTSTATUS W2kTraceMessage(IN TRACEHANDLE  LoggerHandle, IN ULONG TraceOptions, IN LPGUID MessageGuid, IN USHORT MessageNumber, ...)
{
	size_t uiBytes, uiArgCount ;
    	va_list ap ;
	PVOID source ;    	
    	size_t uiElemBytes = 0, uiTotSize, uiOffset ;
       NTSTATUS ulResult = STATUS_SUCCESS ; 
       TRACE_BUFFER TraceBuf;
       void *pData=NULL;

       TraceOptions; // unused

       RtlZeroMemory(&(TraceBuf.Header), sizeof(EVENT_TRACE_HEADER));
       
       //Fill in header fields
       ((PWNODE_HEADER)&(TraceBuf.Header))->HistoricalContext = LoggerHandle;
       TraceBuf.Header.GuidPtr = (ULONGLONG)MessageGuid ;
       TraceBuf.Header.Flags = WNODE_FLAG_TRACED_GUID |WNODE_FLAG_USE_GUID_PTR|WNODE_FLAG_USE_MOF_PTR ;
       //Type is 0xFF to indicate that the first data is the MessageNumber
       TraceBuf.Header.Class.Type = 0xFF ;
       
       //The first Mof data is the Message Number
       TraceBuf.MofField[0].DataPtr = (ULONGLONG)&MessageNumber;
       TraceBuf.MofField[0].Length = sizeof(USHORT);

    
    	// Determine the number bytes to follow header
	uiBytes = 0 ;             // For Count of Bytes
	uiArgCount = 0 ;          // For Count of Arguments
	va_start(ap, MessageNumber) ;
       while ((source = va_arg (ap, PVOID)) != NULL)
       {
            uiElemBytes = va_arg (ap, size_t) ;
            uiBytes += uiElemBytes ;
            uiArgCount++ ;
            if (uiArgCount <= WPP_MAX_MOF_FIELDS)
            {
                  TraceBuf.MofField[uiArgCount].DataPtr = (ULONGLONG)source;
                  TraceBuf.MofField[uiArgCount].Length = (ULONG)uiElemBytes;
            }
       }  	
  	va_end(ap) ;

       if (uiBytes > TRACE_MESSAGE_MAXIMUM_SIZE) 
       {
                return STATUS_UNSUCCESSFUL;	
       }

       //This occurs infrequently
       if (uiArgCount > WPP_MAX_MOF_FIELDS)
       {
            //Allocate the blob to hold the data
            pData = ExAllocatePoolWithTag(WPP_POOLTYPE,uiBytes,WPP_POOLTAG) ;
            if (pData == NULL) 
            {
                return STATUS_NO_MEMORY;
            }

            TraceBuf.MofField[1].DataPtr = (ULONGLONG)pData;
            TraceBuf.MofField[1].Length = uiBytes;
       
            uiOffset = 0 ;
            uiElemBytes = 0 ; 
            va_start(ap, MessageNumber) ;
	     while ((source = va_arg (ap, PVOID)) != NULL)
            {
                uiElemBytes = va_arg (ap, size_t) ;
                memcpy((char*)pData + uiOffset, source, uiElemBytes) ;
                uiOffset += uiElemBytes ;
            }
	     va_end(ap) ;

            //Fill in the total size (header + 2 mof fields)
            TraceBuf.Header.Size = sizeof(EVENT_TRACE_HEADER) + 2*sizeof(MOF_FIELD);
	
       }
       else
       {
            //Fill in the total size (header + mof fields)
            TraceBuf.Header.Size = sizeof(EVENT_TRACE_HEADER) + (uiArgCount+1)*sizeof(MOF_FIELD);
       }

    
	ulResult = IoWMIWriteEvent(&TraceBuf.Header) ;
		
       if (pData) ExFreePool(pData);
		    
       return ulResult ;  	
  	  	  	  	  	
}

`ENDIF`

//
// Public routines to break down the Loggerhandle
//
#define KERNEL_LOGGER_ID                      0xFFFF    // USHORT only

#if(_WIN32_WINNT >= 0x0501)
typedef struct _TRACE_ENABLE_CONTEXT {
    USHORT  LoggerId;           // Actual Id of the logger
    UCHAR   Level;              // Enable level passed by control caller
    UCHAR   InternalFlag;       // Reserved
    ULONG   EnableFlags;        // Enable flags passed by control caller
} TRACE_ENABLE_CONTEXT, *PTRACE_ENABLE_CONTEXT;
#endif

#define WmiGetLoggerId(LoggerContext) \
    (((PTRACE_ENABLE_CONTEXT) (&LoggerContext))->LoggerId == \
        (USHORT)KERNEL_LOGGER_ID) ? \
        KERNEL_LOGGER_ID : \
        ((PTRACE_ENABLE_CONTEXT) (&LoggerContext))->LoggerId

#define WmiGetLoggerEnableFlags(LoggerContext) \
   ((PTRACE_ENABLE_CONTEXT) (&LoggerContext))->EnableFlags
#define WmiGetLoggerEnableLevel(LoggerContext) \
    ((PTRACE_ENABLE_CONTEXT) (&LoggerContext))->Level

#endif  // #ifdef WPP_TRACE_W2K_COMPATABILITY

`INCLUDE km-header.tpl` 
`INCLUDE control.tpl`
`INCLUDE trmacro.tpl`

`IF FOUND WPP_INIT_TRACING`
#define WPPINIT_EXPORT 
  `INCLUDE km-init.tpl`

`ENDIF`





