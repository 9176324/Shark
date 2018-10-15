`**********************************************************************`
`* This is a template file for tracewpp preprocessor                  *`
`* If you need to use a custom version of this file in your project   *`
`* Please clone it from this one and point WPP to it by specifying    *`
`* -gen:{yourfile} option on RUN_WPP line in your sources file        *`
`*                                                                    *`
`* This specific header is um-w2k.tpl and is used to define tracing   *`
`* applications which must also run under Windows 2000                *`
`*                                                                    *`
`*    Copyright 1999-2001 Microsoft Corporation. All Rights Reserved. *`
`**********************************************************************`
//`Compiler.Checksum` Generated File. Do not edit.
// File created by `Compiler.Name` compiler version `Compiler.Version`-`Compiler.Timestamp`
// on `System.Date` at `System.Time` UTC from a template `TemplateFile`

#define WPP_TRACE_W2K_COMPATABILITY
#define WPP_TRACE TraceMessageW2k

#ifdef WPP_TRACE_W2K_COMPATABILITY

#define WPP_TRACE_OPTIONS       0

#include <windows.h>
#pragma warning(disable: 4201)
#include <wmistr.h>
#include <evntrace.h>

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

void TraceMessageW2k(IN TRACEHANDLE  LoggerHandle, IN DWORD TraceOptions, IN LPGUID MessageGuid, IN USHORT MessageNumber, ...) ;

#if defined(__cplusplus)
};
#endif	

`IF FOUND WPP_INIT_TRACING`
#include "stdlib.h"
void TraceMessageW2k(IN TRACEHANDLE  LoggerHandle, IN DWORD TraceOptions, IN LPGUID MessageGuid, IN USHORT MessageNumber, ...)
{
       size_t uiBytes, uiArgCount ;
       va_list ap ;
       PVOID source ;    	
       size_t uiElemBytes = 0, uiOffset ;
       unsigned long ulResult ; 
       TRACE_BUFFER TraceBuf;
       void *pData=NULL;

       TraceOptions; // unused

       //Fill in header fields
       //Type is 0xFF to indicate that the first data is the MessageNumber
       TraceBuf.Header.GuidPtr = (ULONGLONG)MessageGuid ;
       TraceBuf.Header.Flags = WNODE_FLAG_TRACED_GUID |WNODE_FLAG_USE_GUID_PTR|WNODE_FLAG_USE_MOF_PTR ;
       TraceBuf.Header.Class.Type = 0xFF ;

       //The first Mof data is the Message Number
       TraceBuf.MofField[0].DataPtr = (ULONGLONG)&MessageNumber;
       TraceBuf.MofField[0].Length = sizeof(USHORT);
       
       //Determine the number bytes to follow header
       uiBytes = 0 ;             // For Count of Bytes
       uiArgCount = 0 ;       // For Count of Arguments
       
       va_start(ap, MessageNumber);
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
       va_end(ap);

       if (uiBytes > TRACE_MESSAGE_MAXIMUM_SIZE) 
                return;

       //This occurs infrequently
       if (uiArgCount > WPP_MAX_MOF_FIELDS)
       {
            //Allocate the blob to hold the data
            pData = LocalAlloc(0,uiBytes);
            if (pData == NULL) 
            {
                return;
            }

            TraceBuf.MofField[1].DataPtr = (ULONGLONG)pData;
            TraceBuf.MofField[1].Length = (ULONG)uiBytes;
       
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
            TraceBuf.Header.Size = (USHORT)(sizeof(EVENT_TRACE_HEADER) + 2*sizeof(MOF_FIELD));
	
       }
       else
       {
            //Fill in the total size (header + mof fields)
            TraceBuf.Header.Size = (USHORT)(sizeof(EVENT_TRACE_HEADER) + (uiArgCount+1)*sizeof(MOF_FIELD));
       }

       ulResult = TraceEvent(LoggerHandle, &TraceBuf.Header) ;
		
       if (pData) 
       {
            LocalFree(pData);
       }
	
       if(ERROR_SUCCESS != ulResult)
       {	
		// Silently ignored error
       }
   	  	  	  	  	
}
`ENDIF`
__inline TRACEHANDLE WppQueryLogger(LPTSTR LoggerName)
{
#ifndef UNICODE
    if (!LoggerName) {LoggerName = "stdout";}
#else
    if (!LoggerName) {LoggerName = L"stdout";}
#endif
    {
        ULONG status;
        EVENT_TRACE_PROPERTIES LoggerInfo; 

        ZeroMemory(&LoggerInfo, sizeof(LoggerInfo));
        LoggerInfo.Wnode.BufferSize = sizeof(LoggerInfo);
        LoggerInfo.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
        
        status = QueryTrace(0, LoggerName, &LoggerInfo);    
        if (status == ERROR_SUCCESS) {
            return (TRACEHANDLE) LoggerInfo.Wnode.HistoricalContext;
        }
    }
    return 0;
}
#endif  // #ifdef WPP_TRACE_W2K_COMPATABIlITY

`INCLUDE um-header.tpl` 
`INCLUDE control.tpl`
`INCLUDE trmacro.tpl`

`IF FOUND WPP_INIT_TRACING`
#define WPPINIT_EXPORT 
  `INCLUDE um-init.tpl`
`ENDIF`





