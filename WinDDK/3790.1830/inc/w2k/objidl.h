
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 5.03.0279 */
/* at Wed Mar 12 13:27:27 2003
 */
/* Compiler settings for objidl.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32 (32b run), ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __objidl_h__
#define __objidl_h__

/* Forward Declarations */ 

#ifndef __IMarshal_FWD_DEFINED__
#define __IMarshal_FWD_DEFINED__
typedef interface IMarshal IMarshal;
#endif 	/* __IMarshal_FWD_DEFINED__ */


#ifndef __IMarshal2_FWD_DEFINED__
#define __IMarshal2_FWD_DEFINED__
typedef interface IMarshal2 IMarshal2;
#endif 	/* __IMarshal2_FWD_DEFINED__ */


#ifndef __IMalloc_FWD_DEFINED__
#define __IMalloc_FWD_DEFINED__
typedef interface IMalloc IMalloc;
#endif 	/* __IMalloc_FWD_DEFINED__ */


#ifndef __IMallocSpy_FWD_DEFINED__
#define __IMallocSpy_FWD_DEFINED__
typedef interface IMallocSpy IMallocSpy;
#endif 	/* __IMallocSpy_FWD_DEFINED__ */


#ifndef __IStdMarshalInfo_FWD_DEFINED__
#define __IStdMarshalInfo_FWD_DEFINED__
typedef interface IStdMarshalInfo IStdMarshalInfo;
#endif 	/* __IStdMarshalInfo_FWD_DEFINED__ */


#ifndef __IExternalConnection_FWD_DEFINED__
#define __IExternalConnection_FWD_DEFINED__
typedef interface IExternalConnection IExternalConnection;
#endif 	/* __IExternalConnection_FWD_DEFINED__ */


#ifndef __IMultiQI_FWD_DEFINED__
#define __IMultiQI_FWD_DEFINED__
typedef interface IMultiQI IMultiQI;
#endif 	/* __IMultiQI_FWD_DEFINED__ */


#ifndef __AsyncIMultiQI_FWD_DEFINED__
#define __AsyncIMultiQI_FWD_DEFINED__
typedef interface AsyncIMultiQI AsyncIMultiQI;
#endif 	/* __AsyncIMultiQI_FWD_DEFINED__ */


#ifndef __IInternalUnknown_FWD_DEFINED__
#define __IInternalUnknown_FWD_DEFINED__
typedef interface IInternalUnknown IInternalUnknown;
#endif 	/* __IInternalUnknown_FWD_DEFINED__ */


#ifndef __IEnumUnknown_FWD_DEFINED__
#define __IEnumUnknown_FWD_DEFINED__
typedef interface IEnumUnknown IEnumUnknown;
#endif 	/* __IEnumUnknown_FWD_DEFINED__ */


#ifndef __IBindCtx_FWD_DEFINED__
#define __IBindCtx_FWD_DEFINED__
typedef interface IBindCtx IBindCtx;
#endif 	/* __IBindCtx_FWD_DEFINED__ */


#ifndef __IEnumMoniker_FWD_DEFINED__
#define __IEnumMoniker_FWD_DEFINED__
typedef interface IEnumMoniker IEnumMoniker;
#endif 	/* __IEnumMoniker_FWD_DEFINED__ */


#ifndef __IRunnableObject_FWD_DEFINED__
#define __IRunnableObject_FWD_DEFINED__
typedef interface IRunnableObject IRunnableObject;
#endif 	/* __IRunnableObject_FWD_DEFINED__ */


#ifndef __IRunningObjectTable_FWD_DEFINED__
#define __IRunningObjectTable_FWD_DEFINED__
typedef interface IRunningObjectTable IRunningObjectTable;
#endif 	/* __IRunningObjectTable_FWD_DEFINED__ */


#ifndef __IPersist_FWD_DEFINED__
#define __IPersist_FWD_DEFINED__
typedef interface IPersist IPersist;
#endif 	/* __IPersist_FWD_DEFINED__ */


#ifndef __IPersistStream_FWD_DEFINED__
#define __IPersistStream_FWD_DEFINED__
typedef interface IPersistStream IPersistStream;
#endif 	/* __IPersistStream_FWD_DEFINED__ */


#ifndef __IMoniker_FWD_DEFINED__
#define __IMoniker_FWD_DEFINED__
typedef interface IMoniker IMoniker;
#endif 	/* __IMoniker_FWD_DEFINED__ */


#ifndef __IROTData_FWD_DEFINED__
#define __IROTData_FWD_DEFINED__
typedef interface IROTData IROTData;
#endif 	/* __IROTData_FWD_DEFINED__ */


#ifndef __IEnumString_FWD_DEFINED__
#define __IEnumString_FWD_DEFINED__
typedef interface IEnumString IEnumString;
#endif 	/* __IEnumString_FWD_DEFINED__ */


#ifndef __ISequentialStream_FWD_DEFINED__
#define __ISequentialStream_FWD_DEFINED__
typedef interface ISequentialStream ISequentialStream;
#endif 	/* __ISequentialStream_FWD_DEFINED__ */


#ifndef __IStream_FWD_DEFINED__
#define __IStream_FWD_DEFINED__
typedef interface IStream IStream;
#endif 	/* __IStream_FWD_DEFINED__ */


#ifndef __IEnumSTATSTG_FWD_DEFINED__
#define __IEnumSTATSTG_FWD_DEFINED__
typedef interface IEnumSTATSTG IEnumSTATSTG;
#endif 	/* __IEnumSTATSTG_FWD_DEFINED__ */


#ifndef __IStorage_FWD_DEFINED__
#define __IStorage_FWD_DEFINED__
typedef interface IStorage IStorage;
#endif 	/* __IStorage_FWD_DEFINED__ */


#ifndef __IPersistFile_FWD_DEFINED__
#define __IPersistFile_FWD_DEFINED__
typedef interface IPersistFile IPersistFile;
#endif 	/* __IPersistFile_FWD_DEFINED__ */


#ifndef __IPersistStorage_FWD_DEFINED__
#define __IPersistStorage_FWD_DEFINED__
typedef interface IPersistStorage IPersistStorage;
#endif 	/* __IPersistStorage_FWD_DEFINED__ */


#ifndef __ILockBytes_FWD_DEFINED__
#define __ILockBytes_FWD_DEFINED__
typedef interface ILockBytes ILockBytes;
#endif 	/* __ILockBytes_FWD_DEFINED__ */


#ifndef __IEnumFORMATETC_FWD_DEFINED__
#define __IEnumFORMATETC_FWD_DEFINED__
typedef interface IEnumFORMATETC IEnumFORMATETC;
#endif 	/* __IEnumFORMATETC_FWD_DEFINED__ */


#ifndef __IEnumSTATDATA_FWD_DEFINED__
#define __IEnumSTATDATA_FWD_DEFINED__
typedef interface IEnumSTATDATA IEnumSTATDATA;
#endif 	/* __IEnumSTATDATA_FWD_DEFINED__ */


#ifndef __IRootStorage_FWD_DEFINED__
#define __IRootStorage_FWD_DEFINED__
typedef interface IRootStorage IRootStorage;
#endif 	/* __IRootStorage_FWD_DEFINED__ */


#ifndef __IAdviseSink_FWD_DEFINED__
#define __IAdviseSink_FWD_DEFINED__
typedef interface IAdviseSink IAdviseSink;
#endif 	/* __IAdviseSink_FWD_DEFINED__ */


#ifndef __AsyncIAdviseSink_FWD_DEFINED__
#define __AsyncIAdviseSink_FWD_DEFINED__
typedef interface AsyncIAdviseSink AsyncIAdviseSink;
#endif 	/* __AsyncIAdviseSink_FWD_DEFINED__ */


#ifndef __IAdviseSink2_FWD_DEFINED__
#define __IAdviseSink2_FWD_DEFINED__
typedef interface IAdviseSink2 IAdviseSink2;
#endif 	/* __IAdviseSink2_FWD_DEFINED__ */


#ifndef __AsyncIAdviseSink2_FWD_DEFINED__
#define __AsyncIAdviseSink2_FWD_DEFINED__
typedef interface AsyncIAdviseSink2 AsyncIAdviseSink2;
#endif 	/* __AsyncIAdviseSink2_FWD_DEFINED__ */


#ifndef __IDataObject_FWD_DEFINED__
#define __IDataObject_FWD_DEFINED__
typedef interface IDataObject IDataObject;
#endif 	/* __IDataObject_FWD_DEFINED__ */


#ifndef __IDataAdviseHolder_FWD_DEFINED__
#define __IDataAdviseHolder_FWD_DEFINED__
typedef interface IDataAdviseHolder IDataAdviseHolder;
#endif 	/* __IDataAdviseHolder_FWD_DEFINED__ */


#ifndef __IMessageFilter_FWD_DEFINED__
#define __IMessageFilter_FWD_DEFINED__
typedef interface IMessageFilter IMessageFilter;
#endif 	/* __IMessageFilter_FWD_DEFINED__ */


#ifndef __IRpcChannelBuffer_FWD_DEFINED__
#define __IRpcChannelBuffer_FWD_DEFINED__
typedef interface IRpcChannelBuffer IRpcChannelBuffer;
#endif 	/* __IRpcChannelBuffer_FWD_DEFINED__ */


#ifndef __IRpcChannelBuffer2_FWD_DEFINED__
#define __IRpcChannelBuffer2_FWD_DEFINED__
typedef interface IRpcChannelBuffer2 IRpcChannelBuffer2;
#endif 	/* __IRpcChannelBuffer2_FWD_DEFINED__ */


#ifndef __IAsyncRpcChannelBuffer_FWD_DEFINED__
#define __IAsyncRpcChannelBuffer_FWD_DEFINED__
typedef interface IAsyncRpcChannelBuffer IAsyncRpcChannelBuffer;
#endif 	/* __IAsyncRpcChannelBuffer_FWD_DEFINED__ */


#ifndef __IRpcChannelBuffer3_FWD_DEFINED__
#define __IRpcChannelBuffer3_FWD_DEFINED__
typedef interface IRpcChannelBuffer3 IRpcChannelBuffer3;
#endif 	/* __IRpcChannelBuffer3_FWD_DEFINED__ */


#ifndef __IRpcProxyBuffer_FWD_DEFINED__
#define __IRpcProxyBuffer_FWD_DEFINED__
typedef interface IRpcProxyBuffer IRpcProxyBuffer;
#endif 	/* __IRpcProxyBuffer_FWD_DEFINED__ */


#ifndef __IRpcStubBuffer_FWD_DEFINED__
#define __IRpcStubBuffer_FWD_DEFINED__
typedef interface IRpcStubBuffer IRpcStubBuffer;
#endif 	/* __IRpcStubBuffer_FWD_DEFINED__ */


#ifndef __IPSFactoryBuffer_FWD_DEFINED__
#define __IPSFactoryBuffer_FWD_DEFINED__
typedef interface IPSFactoryBuffer IPSFactoryBuffer;
#endif 	/* __IPSFactoryBuffer_FWD_DEFINED__ */


#ifndef __IChannelHook_FWD_DEFINED__
#define __IChannelHook_FWD_DEFINED__
typedef interface IChannelHook IChannelHook;
#endif 	/* __IChannelHook_FWD_DEFINED__ */


#ifndef __IClientSecurity_FWD_DEFINED__
#define __IClientSecurity_FWD_DEFINED__
typedef interface IClientSecurity IClientSecurity;
#endif 	/* __IClientSecurity_FWD_DEFINED__ */


#ifndef __IServerSecurity_FWD_DEFINED__
#define __IServerSecurity_FWD_DEFINED__
typedef interface IServerSecurity IServerSecurity;
#endif 	/* __IServerSecurity_FWD_DEFINED__ */


#ifndef __IClassActivator_FWD_DEFINED__
#define __IClassActivator_FWD_DEFINED__
typedef interface IClassActivator IClassActivator;
#endif 	/* __IClassActivator_FWD_DEFINED__ */


#ifndef __IRpcOptions_FWD_DEFINED__
#define __IRpcOptions_FWD_DEFINED__
typedef interface IRpcOptions IRpcOptions;
#endif 	/* __IRpcOptions_FWD_DEFINED__ */


#ifndef __IFillLockBytes_FWD_DEFINED__
#define __IFillLockBytes_FWD_DEFINED__
typedef interface IFillLockBytes IFillLockBytes;
#endif 	/* __IFillLockBytes_FWD_DEFINED__ */


#ifndef __IProgressNotify_FWD_DEFINED__
#define __IProgressNotify_FWD_DEFINED__
typedef interface IProgressNotify IProgressNotify;
#endif 	/* __IProgressNotify_FWD_DEFINED__ */


#ifndef __ILayoutStorage_FWD_DEFINED__
#define __ILayoutStorage_FWD_DEFINED__
typedef interface ILayoutStorage ILayoutStorage;
#endif 	/* __ILayoutStorage_FWD_DEFINED__ */


#ifndef __IBlockingLock_FWD_DEFINED__
#define __IBlockingLock_FWD_DEFINED__
typedef interface IBlockingLock IBlockingLock;
#endif 	/* __IBlockingLock_FWD_DEFINED__ */


#ifndef __ITimeAndNoticeControl_FWD_DEFINED__
#define __ITimeAndNoticeControl_FWD_DEFINED__
typedef interface ITimeAndNoticeControl ITimeAndNoticeControl;
#endif 	/* __ITimeAndNoticeControl_FWD_DEFINED__ */


#ifndef __IOplockStorage_FWD_DEFINED__
#define __IOplockStorage_FWD_DEFINED__
typedef interface IOplockStorage IOplockStorage;
#endif 	/* __IOplockStorage_FWD_DEFINED__ */


#ifndef __ISurrogate_FWD_DEFINED__
#define __ISurrogate_FWD_DEFINED__
typedef interface ISurrogate ISurrogate;
#endif 	/* __ISurrogate_FWD_DEFINED__ */


#ifndef __IGlobalInterfaceTable_FWD_DEFINED__
#define __IGlobalInterfaceTable_FWD_DEFINED__
typedef interface IGlobalInterfaceTable IGlobalInterfaceTable;
#endif 	/* __IGlobalInterfaceTable_FWD_DEFINED__ */


#ifndef __IDirectWriterLock_FWD_DEFINED__
#define __IDirectWriterLock_FWD_DEFINED__
typedef interface IDirectWriterLock IDirectWriterLock;
#endif 	/* __IDirectWriterLock_FWD_DEFINED__ */


#ifndef __ISynchronize_FWD_DEFINED__
#define __ISynchronize_FWD_DEFINED__
typedef interface ISynchronize ISynchronize;
#endif 	/* __ISynchronize_FWD_DEFINED__ */


#ifndef __ISynchronizeHandle_FWD_DEFINED__
#define __ISynchronizeHandle_FWD_DEFINED__
typedef interface ISynchronizeHandle ISynchronizeHandle;
#endif 	/* __ISynchronizeHandle_FWD_DEFINED__ */


#ifndef __ISynchronizeEvent_FWD_DEFINED__
#define __ISynchronizeEvent_FWD_DEFINED__
typedef interface ISynchronizeEvent ISynchronizeEvent;
#endif 	/* __ISynchronizeEvent_FWD_DEFINED__ */


#ifndef __ISynchronizeContainer_FWD_DEFINED__
#define __ISynchronizeContainer_FWD_DEFINED__
typedef interface ISynchronizeContainer ISynchronizeContainer;
#endif 	/* __ISynchronizeContainer_FWD_DEFINED__ */


#ifndef __ISynchronizeMutex_FWD_DEFINED__
#define __ISynchronizeMutex_FWD_DEFINED__
typedef interface ISynchronizeMutex ISynchronizeMutex;
#endif 	/* __ISynchronizeMutex_FWD_DEFINED__ */


#ifndef __ICancelMethodCalls_FWD_DEFINED__
#define __ICancelMethodCalls_FWD_DEFINED__
typedef interface ICancelMethodCalls ICancelMethodCalls;
#endif 	/* __ICancelMethodCalls_FWD_DEFINED__ */


#ifndef __IAsyncManager_FWD_DEFINED__
#define __IAsyncManager_FWD_DEFINED__
typedef interface IAsyncManager IAsyncManager;
#endif 	/* __IAsyncManager_FWD_DEFINED__ */


#ifndef __ICallFactory_FWD_DEFINED__
#define __ICallFactory_FWD_DEFINED__
typedef interface ICallFactory ICallFactory;
#endif 	/* __ICallFactory_FWD_DEFINED__ */


#ifndef __IRpcHelper_FWD_DEFINED__
#define __IRpcHelper_FWD_DEFINED__
typedef interface IRpcHelper IRpcHelper;
#endif 	/* __IRpcHelper_FWD_DEFINED__ */


#ifndef __IReleaseMarshalBuffers_FWD_DEFINED__
#define __IReleaseMarshalBuffers_FWD_DEFINED__
typedef interface IReleaseMarshalBuffers IReleaseMarshalBuffers;
#endif 	/* __IReleaseMarshalBuffers_FWD_DEFINED__ */


#ifndef __IWaitMultiple_FWD_DEFINED__
#define __IWaitMultiple_FWD_DEFINED__
typedef interface IWaitMultiple IWaitMultiple;
#endif 	/* __IWaitMultiple_FWD_DEFINED__ */


#ifndef __IUrlMon_FWD_DEFINED__
#define __IUrlMon_FWD_DEFINED__
typedef interface IUrlMon IUrlMon;
#endif 	/* __IUrlMon_FWD_DEFINED__ */


#ifndef __IForegroundTransfer_FWD_DEFINED__
#define __IForegroundTransfer_FWD_DEFINED__
typedef interface IForegroundTransfer IForegroundTransfer;
#endif 	/* __IForegroundTransfer_FWD_DEFINED__ */


#ifndef __IPipeByte_FWD_DEFINED__
#define __IPipeByte_FWD_DEFINED__
typedef interface IPipeByte IPipeByte;
#endif 	/* __IPipeByte_FWD_DEFINED__ */


#ifndef __AsyncIPipeByte_FWD_DEFINED__
#define __AsyncIPipeByte_FWD_DEFINED__
typedef interface AsyncIPipeByte AsyncIPipeByte;
#endif 	/* __AsyncIPipeByte_FWD_DEFINED__ */


#ifndef __IPipeLong_FWD_DEFINED__
#define __IPipeLong_FWD_DEFINED__
typedef interface IPipeLong IPipeLong;
#endif 	/* __IPipeLong_FWD_DEFINED__ */


#ifndef __AsyncIPipeLong_FWD_DEFINED__
#define __AsyncIPipeLong_FWD_DEFINED__
typedef interface AsyncIPipeLong AsyncIPipeLong;
#endif 	/* __AsyncIPipeLong_FWD_DEFINED__ */


#ifndef __IPipeDouble_FWD_DEFINED__
#define __IPipeDouble_FWD_DEFINED__
typedef interface IPipeDouble IPipeDouble;
#endif 	/* __IPipeDouble_FWD_DEFINED__ */


#ifndef __AsyncIPipeDouble_FWD_DEFINED__
#define __AsyncIPipeDouble_FWD_DEFINED__
typedef interface AsyncIPipeDouble AsyncIPipeDouble;
#endif 	/* __AsyncIPipeDouble_FWD_DEFINED__ */


#ifndef __IThumbnailExtractor_FWD_DEFINED__
#define __IThumbnailExtractor_FWD_DEFINED__
typedef interface IThumbnailExtractor IThumbnailExtractor;
#endif 	/* __IThumbnailExtractor_FWD_DEFINED__ */


#ifndef __IDummyHICONIncluder_FWD_DEFINED__
#define __IDummyHICONIncluder_FWD_DEFINED__
typedef interface IDummyHICONIncluder IDummyHICONIncluder;
#endif 	/* __IDummyHICONIncluder_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_objidl_0000 */
/* [local] */ 

//+-------------------------------------------------------------------------
//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992-1999.
//
//--------------------------------------------------------------------------



#if ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)
#endif
#if ( _MSC_VER >= 1020 )
#pragma once
#endif













typedef struct _COSERVERINFO
    {
    DWORD dwReserved1;
    LPWSTR pwszName;
    COAUTHINFO __RPC_FAR *pAuthInfo;
    DWORD dwReserved2;
    }	COSERVERINFO;



extern RPC_IF_HANDLE __MIDL_itf_objidl_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_objidl_0000_v0_0_s_ifspec;

#ifndef __IMarshal_INTERFACE_DEFINED__
#define __IMarshal_INTERFACE_DEFINED__

/* interface IMarshal */
/* [uuid][object][local] */ 

typedef /* [unique] */ IMarshal __RPC_FAR *LPMARSHAL;


EXTERN_C const IID IID_IMarshal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000003-0000-0000-C000-000000000046")
    IMarshal : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetUnmarshalClass( 
            /* [in] */ REFIID riid,
            /* [unique][in] */ void __RPC_FAR *pv,
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [in] */ DWORD mshlflags,
            /* [out] */ CLSID __RPC_FAR *pCid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMarshalSizeMax( 
            /* [in] */ REFIID riid,
            /* [unique][in] */ void __RPC_FAR *pv,
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [in] */ DWORD mshlflags,
            /* [out] */ DWORD __RPC_FAR *pSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MarshalInterface( 
            /* [unique][in] */ IStream __RPC_FAR *pStm,
            /* [in] */ REFIID riid,
            /* [unique][in] */ void __RPC_FAR *pv,
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [in] */ DWORD mshlflags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnmarshalInterface( 
            /* [unique][in] */ IStream __RPC_FAR *pStm,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseMarshalData( 
            /* [unique][in] */ IStream __RPC_FAR *pStm) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisconnectObject( 
            /* [in] */ DWORD dwReserved) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMarshalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMarshal __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMarshal __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMarshal __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetUnmarshalClass )( 
            IMarshal __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [unique][in] */ void __RPC_FAR *pv,
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [in] */ DWORD mshlflags,
            /* [out] */ CLSID __RPC_FAR *pCid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMarshalSizeMax )( 
            IMarshal __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [unique][in] */ void __RPC_FAR *pv,
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [in] */ DWORD mshlflags,
            /* [out] */ DWORD __RPC_FAR *pSize);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MarshalInterface )( 
            IMarshal __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm,
            /* [in] */ REFIID riid,
            /* [unique][in] */ void __RPC_FAR *pv,
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [in] */ DWORD mshlflags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UnmarshalInterface )( 
            IMarshal __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseMarshalData )( 
            IMarshal __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DisconnectObject )( 
            IMarshal __RPC_FAR * This,
            /* [in] */ DWORD dwReserved);
        
        END_INTERFACE
    } IMarshalVtbl;

    interface IMarshal
    {
        CONST_VTBL struct IMarshalVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMarshal_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMarshal_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMarshal_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMarshal_GetUnmarshalClass(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pCid)	\
    (This)->lpVtbl -> GetUnmarshalClass(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pCid)

#define IMarshal_GetMarshalSizeMax(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pSize)	\
    (This)->lpVtbl -> GetMarshalSizeMax(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pSize)

#define IMarshal_MarshalInterface(This,pStm,riid,pv,dwDestContext,pvDestContext,mshlflags)	\
    (This)->lpVtbl -> MarshalInterface(This,pStm,riid,pv,dwDestContext,pvDestContext,mshlflags)

#define IMarshal_UnmarshalInterface(This,pStm,riid,ppv)	\
    (This)->lpVtbl -> UnmarshalInterface(This,pStm,riid,ppv)

#define IMarshal_ReleaseMarshalData(This,pStm)	\
    (This)->lpVtbl -> ReleaseMarshalData(This,pStm)

#define IMarshal_DisconnectObject(This,dwReserved)	\
    (This)->lpVtbl -> DisconnectObject(This,dwReserved)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IMarshal_GetUnmarshalClass_Proxy( 
    IMarshal __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [unique][in] */ void __RPC_FAR *pv,
    /* [in] */ DWORD dwDestContext,
    /* [unique][in] */ void __RPC_FAR *pvDestContext,
    /* [in] */ DWORD mshlflags,
    /* [out] */ CLSID __RPC_FAR *pCid);


void __RPC_STUB IMarshal_GetUnmarshalClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMarshal_GetMarshalSizeMax_Proxy( 
    IMarshal __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [unique][in] */ void __RPC_FAR *pv,
    /* [in] */ DWORD dwDestContext,
    /* [unique][in] */ void __RPC_FAR *pvDestContext,
    /* [in] */ DWORD mshlflags,
    /* [out] */ DWORD __RPC_FAR *pSize);


void __RPC_STUB IMarshal_GetMarshalSizeMax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMarshal_MarshalInterface_Proxy( 
    IMarshal __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pStm,
    /* [in] */ REFIID riid,
    /* [unique][in] */ void __RPC_FAR *pv,
    /* [in] */ DWORD dwDestContext,
    /* [unique][in] */ void __RPC_FAR *pvDestContext,
    /* [in] */ DWORD mshlflags);


void __RPC_STUB IMarshal_MarshalInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMarshal_UnmarshalInterface_Proxy( 
    IMarshal __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pStm,
    /* [in] */ REFIID riid,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB IMarshal_UnmarshalInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMarshal_ReleaseMarshalData_Proxy( 
    IMarshal __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pStm);


void __RPC_STUB IMarshal_ReleaseMarshalData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMarshal_DisconnectObject_Proxy( 
    IMarshal __RPC_FAR * This,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IMarshal_DisconnectObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMarshal_INTERFACE_DEFINED__ */


#ifndef __IMarshal2_INTERFACE_DEFINED__
#define __IMarshal2_INTERFACE_DEFINED__

/* interface IMarshal2 */
/* [uuid][object][local] */ 

typedef /* [unique] */ IMarshal2 __RPC_FAR *LPMARSHAL2;


EXTERN_C const IID IID_IMarshal2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001cf-0000-0000-C000-000000000046")
    IMarshal2 : public IMarshal
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IMarshal2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMarshal2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMarshal2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMarshal2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetUnmarshalClass )( 
            IMarshal2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [unique][in] */ void __RPC_FAR *pv,
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [in] */ DWORD mshlflags,
            /* [out] */ CLSID __RPC_FAR *pCid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMarshalSizeMax )( 
            IMarshal2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [unique][in] */ void __RPC_FAR *pv,
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [in] */ DWORD mshlflags,
            /* [out] */ DWORD __RPC_FAR *pSize);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MarshalInterface )( 
            IMarshal2 __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm,
            /* [in] */ REFIID riid,
            /* [unique][in] */ void __RPC_FAR *pv,
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [in] */ DWORD mshlflags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UnmarshalInterface )( 
            IMarshal2 __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseMarshalData )( 
            IMarshal2 __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DisconnectObject )( 
            IMarshal2 __RPC_FAR * This,
            /* [in] */ DWORD dwReserved);
        
        END_INTERFACE
    } IMarshal2Vtbl;

    interface IMarshal2
    {
        CONST_VTBL struct IMarshal2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMarshal2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMarshal2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMarshal2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMarshal2_GetUnmarshalClass(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pCid)	\
    (This)->lpVtbl -> GetUnmarshalClass(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pCid)

#define IMarshal2_GetMarshalSizeMax(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pSize)	\
    (This)->lpVtbl -> GetMarshalSizeMax(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pSize)

#define IMarshal2_MarshalInterface(This,pStm,riid,pv,dwDestContext,pvDestContext,mshlflags)	\
    (This)->lpVtbl -> MarshalInterface(This,pStm,riid,pv,dwDestContext,pvDestContext,mshlflags)

#define IMarshal2_UnmarshalInterface(This,pStm,riid,ppv)	\
    (This)->lpVtbl -> UnmarshalInterface(This,pStm,riid,ppv)

#define IMarshal2_ReleaseMarshalData(This,pStm)	\
    (This)->lpVtbl -> ReleaseMarshalData(This,pStm)

#define IMarshal2_DisconnectObject(This,dwReserved)	\
    (This)->lpVtbl -> DisconnectObject(This,dwReserved)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IMarshal2_INTERFACE_DEFINED__ */


#ifndef __IMalloc_INTERFACE_DEFINED__
#define __IMalloc_INTERFACE_DEFINED__

/* interface IMalloc */
/* [uuid][object][local] */ 

typedef /* [unique] */ IMalloc __RPC_FAR *LPMALLOC;


EXTERN_C const IID IID_IMalloc;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000002-0000-0000-C000-000000000046")
    IMalloc : public IUnknown
    {
    public:
        virtual void __RPC_FAR *STDMETHODCALLTYPE Alloc( 
            /* [in] */ SIZE_T cb) = 0;
        
        virtual void __RPC_FAR *STDMETHODCALLTYPE Realloc( 
            /* [in] */ void __RPC_FAR *pv,
            /* [in] */ SIZE_T cb) = 0;
        
        virtual void STDMETHODCALLTYPE Free( 
            /* [in] */ void __RPC_FAR *pv) = 0;
        
        virtual SIZE_T STDMETHODCALLTYPE GetSize( 
            /* [in] */ void __RPC_FAR *pv) = 0;
        
        virtual int STDMETHODCALLTYPE DidAlloc( 
            void __RPC_FAR *pv) = 0;
        
        virtual void STDMETHODCALLTYPE HeapMinimize( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMallocVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMalloc __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMalloc __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMalloc __RPC_FAR * This);
        
        void __RPC_FAR *( STDMETHODCALLTYPE __RPC_FAR *Alloc )( 
            IMalloc __RPC_FAR * This,
            /* [in] */ SIZE_T cb);
        
        void __RPC_FAR *( STDMETHODCALLTYPE __RPC_FAR *Realloc )( 
            IMalloc __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pv,
            /* [in] */ SIZE_T cb);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *Free )( 
            IMalloc __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pv);
        
        SIZE_T ( STDMETHODCALLTYPE __RPC_FAR *GetSize )( 
            IMalloc __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pv);
        
        int ( STDMETHODCALLTYPE __RPC_FAR *DidAlloc )( 
            IMalloc __RPC_FAR * This,
            void __RPC_FAR *pv);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *HeapMinimize )( 
            IMalloc __RPC_FAR * This);
        
        END_INTERFACE
    } IMallocVtbl;

    interface IMalloc
    {
        CONST_VTBL struct IMallocVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMalloc_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMalloc_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMalloc_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMalloc_Alloc(This,cb)	\
    (This)->lpVtbl -> Alloc(This,cb)

#define IMalloc_Realloc(This,pv,cb)	\
    (This)->lpVtbl -> Realloc(This,pv,cb)

#define IMalloc_Free(This,pv)	\
    (This)->lpVtbl -> Free(This,pv)

#define IMalloc_GetSize(This,pv)	\
    (This)->lpVtbl -> GetSize(This,pv)

#define IMalloc_DidAlloc(This,pv)	\
    (This)->lpVtbl -> DidAlloc(This,pv)

#define IMalloc_HeapMinimize(This)	\
    (This)->lpVtbl -> HeapMinimize(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



void __RPC_FAR *STDMETHODCALLTYPE IMalloc_Alloc_Proxy( 
    IMalloc __RPC_FAR * This,
    /* [in] */ SIZE_T cb);


void __RPC_STUB IMalloc_Alloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __RPC_FAR *STDMETHODCALLTYPE IMalloc_Realloc_Proxy( 
    IMalloc __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pv,
    /* [in] */ SIZE_T cb);


void __RPC_STUB IMalloc_Realloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IMalloc_Free_Proxy( 
    IMalloc __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pv);


void __RPC_STUB IMalloc_Free_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SIZE_T STDMETHODCALLTYPE IMalloc_GetSize_Proxy( 
    IMalloc __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pv);


void __RPC_STUB IMalloc_GetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


int STDMETHODCALLTYPE IMalloc_DidAlloc_Proxy( 
    IMalloc __RPC_FAR * This,
    void __RPC_FAR *pv);


void __RPC_STUB IMalloc_DidAlloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IMalloc_HeapMinimize_Proxy( 
    IMalloc __RPC_FAR * This);


void __RPC_STUB IMalloc_HeapMinimize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMalloc_INTERFACE_DEFINED__ */


#ifndef __IMallocSpy_INTERFACE_DEFINED__
#define __IMallocSpy_INTERFACE_DEFINED__

/* interface IMallocSpy */
/* [uuid][object][local] */ 

typedef /* [unique] */ IMallocSpy __RPC_FAR *LPMALLOCSPY;


EXTERN_C const IID IID_IMallocSpy;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000001d-0000-0000-C000-000000000046")
    IMallocSpy : public IUnknown
    {
    public:
        virtual SIZE_T STDMETHODCALLTYPE PreAlloc( 
            /* [in] */ SIZE_T cbRequest) = 0;
        
        virtual void __RPC_FAR *STDMETHODCALLTYPE PostAlloc( 
            /* [in] */ void __RPC_FAR *pActual) = 0;
        
        virtual void __RPC_FAR *STDMETHODCALLTYPE PreFree( 
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ BOOL fSpyed) = 0;
        
        virtual void STDMETHODCALLTYPE PostFree( 
            /* [in] */ BOOL fSpyed) = 0;
        
        virtual SIZE_T STDMETHODCALLTYPE PreRealloc( 
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ SIZE_T cbRequest,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppNewRequest,
            /* [in] */ BOOL fSpyed) = 0;
        
        virtual void __RPC_FAR *STDMETHODCALLTYPE PostRealloc( 
            /* [in] */ void __RPC_FAR *pActual,
            /* [in] */ BOOL fSpyed) = 0;
        
        virtual void __RPC_FAR *STDMETHODCALLTYPE PreGetSize( 
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ BOOL fSpyed) = 0;
        
        virtual SIZE_T STDMETHODCALLTYPE PostGetSize( 
            /* [in] */ SIZE_T cbActual,
            /* [in] */ BOOL fSpyed) = 0;
        
        virtual void __RPC_FAR *STDMETHODCALLTYPE PreDidAlloc( 
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ BOOL fSpyed) = 0;
        
        virtual int STDMETHODCALLTYPE PostDidAlloc( 
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ BOOL fSpyed,
            /* [in] */ int fActual) = 0;
        
        virtual void STDMETHODCALLTYPE PreHeapMinimize( void) = 0;
        
        virtual void STDMETHODCALLTYPE PostHeapMinimize( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMallocSpyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMallocSpy __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMallocSpy __RPC_FAR * This);
        
        SIZE_T ( STDMETHODCALLTYPE __RPC_FAR *PreAlloc )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ SIZE_T cbRequest);
        
        void __RPC_FAR *( STDMETHODCALLTYPE __RPC_FAR *PostAlloc )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pActual);
        
        void __RPC_FAR *( STDMETHODCALLTYPE __RPC_FAR *PreFree )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ BOOL fSpyed);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *PostFree )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ BOOL fSpyed);
        
        SIZE_T ( STDMETHODCALLTYPE __RPC_FAR *PreRealloc )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ SIZE_T cbRequest,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppNewRequest,
            /* [in] */ BOOL fSpyed);
        
        void __RPC_FAR *( STDMETHODCALLTYPE __RPC_FAR *PostRealloc )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pActual,
            /* [in] */ BOOL fSpyed);
        
        void __RPC_FAR *( STDMETHODCALLTYPE __RPC_FAR *PreGetSize )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ BOOL fSpyed);
        
        SIZE_T ( STDMETHODCALLTYPE __RPC_FAR *PostGetSize )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ SIZE_T cbActual,
            /* [in] */ BOOL fSpyed);
        
        void __RPC_FAR *( STDMETHODCALLTYPE __RPC_FAR *PreDidAlloc )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ BOOL fSpyed);
        
        int ( STDMETHODCALLTYPE __RPC_FAR *PostDidAlloc )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ BOOL fSpyed,
            /* [in] */ int fActual);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *PreHeapMinimize )( 
            IMallocSpy __RPC_FAR * This);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *PostHeapMinimize )( 
            IMallocSpy __RPC_FAR * This);
        
        END_INTERFACE
    } IMallocSpyVtbl;

    interface IMallocSpy
    {
        CONST_VTBL struct IMallocSpyVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMallocSpy_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMallocSpy_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMallocSpy_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMallocSpy_PreAlloc(This,cbRequest)	\
    (This)->lpVtbl -> PreAlloc(This,cbRequest)

#define IMallocSpy_PostAlloc(This,pActual)	\
    (This)->lpVtbl -> PostAlloc(This,pActual)

#define IMallocSpy_PreFree(This,pRequest,fSpyed)	\
    (This)->lpVtbl -> PreFree(This,pRequest,fSpyed)

#define IMallocSpy_PostFree(This,fSpyed)	\
    (This)->lpVtbl -> PostFree(This,fSpyed)

#define IMallocSpy_PreRealloc(This,pRequest,cbRequest,ppNewRequest,fSpyed)	\
    (This)->lpVtbl -> PreRealloc(This,pRequest,cbRequest,ppNewRequest,fSpyed)

#define IMallocSpy_PostRealloc(This,pActual,fSpyed)	\
    (This)->lpVtbl -> PostRealloc(This,pActual,fSpyed)

#define IMallocSpy_PreGetSize(This,pRequest,fSpyed)	\
    (This)->lpVtbl -> PreGetSize(This,pRequest,fSpyed)

#define IMallocSpy_PostGetSize(This,cbActual,fSpyed)	\
    (This)->lpVtbl -> PostGetSize(This,cbActual,fSpyed)

#define IMallocSpy_PreDidAlloc(This,pRequest,fSpyed)	\
    (This)->lpVtbl -> PreDidAlloc(This,pRequest,fSpyed)

#define IMallocSpy_PostDidAlloc(This,pRequest,fSpyed,fActual)	\
    (This)->lpVtbl -> PostDidAlloc(This,pRequest,fSpyed,fActual)

#define IMallocSpy_PreHeapMinimize(This)	\
    (This)->lpVtbl -> PreHeapMinimize(This)

#define IMallocSpy_PostHeapMinimize(This)	\
    (This)->lpVtbl -> PostHeapMinimize(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



SIZE_T STDMETHODCALLTYPE IMallocSpy_PreAlloc_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ SIZE_T cbRequest);


void __RPC_STUB IMallocSpy_PreAlloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __RPC_FAR *STDMETHODCALLTYPE IMallocSpy_PostAlloc_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pActual);


void __RPC_STUB IMallocSpy_PostAlloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __RPC_FAR *STDMETHODCALLTYPE IMallocSpy_PreFree_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pRequest,
    /* [in] */ BOOL fSpyed);


void __RPC_STUB IMallocSpy_PreFree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IMallocSpy_PostFree_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ BOOL fSpyed);


void __RPC_STUB IMallocSpy_PostFree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SIZE_T STDMETHODCALLTYPE IMallocSpy_PreRealloc_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pRequest,
    /* [in] */ SIZE_T cbRequest,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppNewRequest,
    /* [in] */ BOOL fSpyed);


void __RPC_STUB IMallocSpy_PreRealloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __RPC_FAR *STDMETHODCALLTYPE IMallocSpy_PostRealloc_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pActual,
    /* [in] */ BOOL fSpyed);


void __RPC_STUB IMallocSpy_PostRealloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __RPC_FAR *STDMETHODCALLTYPE IMallocSpy_PreGetSize_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pRequest,
    /* [in] */ BOOL fSpyed);


void __RPC_STUB IMallocSpy_PreGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SIZE_T STDMETHODCALLTYPE IMallocSpy_PostGetSize_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ SIZE_T cbActual,
    /* [in] */ BOOL fSpyed);


void __RPC_STUB IMallocSpy_PostGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __RPC_FAR *STDMETHODCALLTYPE IMallocSpy_PreDidAlloc_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pRequest,
    /* [in] */ BOOL fSpyed);


void __RPC_STUB IMallocSpy_PreDidAlloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


int STDMETHODCALLTYPE IMallocSpy_PostDidAlloc_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pRequest,
    /* [in] */ BOOL fSpyed,
    /* [in] */ int fActual);


void __RPC_STUB IMallocSpy_PostDidAlloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IMallocSpy_PreHeapMinimize_Proxy( 
    IMallocSpy __RPC_FAR * This);


void __RPC_STUB IMallocSpy_PreHeapMinimize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IMallocSpy_PostHeapMinimize_Proxy( 
    IMallocSpy __RPC_FAR * This);


void __RPC_STUB IMallocSpy_PostHeapMinimize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMallocSpy_INTERFACE_DEFINED__ */


#ifndef __IStdMarshalInfo_INTERFACE_DEFINED__
#define __IStdMarshalInfo_INTERFACE_DEFINED__

/* interface IStdMarshalInfo */
/* [uuid][object][local] */ 

typedef /* [unique] */ IStdMarshalInfo __RPC_FAR *LPSTDMARSHALINFO;


EXTERN_C const IID IID_IStdMarshalInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000018-0000-0000-C000-000000000046")
    IStdMarshalInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetClassForHandler( 
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [out] */ CLSID __RPC_FAR *pClsid) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IStdMarshalInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IStdMarshalInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IStdMarshalInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IStdMarshalInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClassForHandler )( 
            IStdMarshalInfo __RPC_FAR * This,
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [out] */ CLSID __RPC_FAR *pClsid);
        
        END_INTERFACE
    } IStdMarshalInfoVtbl;

    interface IStdMarshalInfo
    {
        CONST_VTBL struct IStdMarshalInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStdMarshalInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStdMarshalInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStdMarshalInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStdMarshalInfo_GetClassForHandler(This,dwDestContext,pvDestContext,pClsid)	\
    (This)->lpVtbl -> GetClassForHandler(This,dwDestContext,pvDestContext,pClsid)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IStdMarshalInfo_GetClassForHandler_Proxy( 
    IStdMarshalInfo __RPC_FAR * This,
    /* [in] */ DWORD dwDestContext,
    /* [unique][in] */ void __RPC_FAR *pvDestContext,
    /* [out] */ CLSID __RPC_FAR *pClsid);


void __RPC_STUB IStdMarshalInfo_GetClassForHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IStdMarshalInfo_INTERFACE_DEFINED__ */


#ifndef __IExternalConnection_INTERFACE_DEFINED__
#define __IExternalConnection_INTERFACE_DEFINED__

/* interface IExternalConnection */
/* [uuid][local][object] */ 

typedef /* [unique] */ IExternalConnection __RPC_FAR *LPEXTERNALCONNECTION;

typedef 
enum tagEXTCONN
    {	EXTCONN_STRONG	= 0x1,
	EXTCONN_WEAK	= 0x2,
	EXTCONN_CALLABLE	= 0x4
    }	EXTCONN;


EXTERN_C const IID IID_IExternalConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000019-0000-0000-C000-000000000046")
    IExternalConnection : public IUnknown
    {
    public:
        virtual DWORD STDMETHODCALLTYPE AddConnection( 
            /* [in] */ DWORD extconn,
            /* [in] */ DWORD reserved) = 0;
        
        virtual DWORD STDMETHODCALLTYPE ReleaseConnection( 
            /* [in] */ DWORD extconn,
            /* [in] */ DWORD reserved,
            /* [in] */ BOOL fLastReleaseCloses) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IExternalConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IExternalConnection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IExternalConnection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IExternalConnection __RPC_FAR * This);
        
        DWORD ( STDMETHODCALLTYPE __RPC_FAR *AddConnection )( 
            IExternalConnection __RPC_FAR * This,
            /* [in] */ DWORD extconn,
            /* [in] */ DWORD reserved);
        
        DWORD ( STDMETHODCALLTYPE __RPC_FAR *ReleaseConnection )( 
            IExternalConnection __RPC_FAR * This,
            /* [in] */ DWORD extconn,
            /* [in] */ DWORD reserved,
            /* [in] */ BOOL fLastReleaseCloses);
        
        END_INTERFACE
    } IExternalConnectionVtbl;

    interface IExternalConnection
    {
        CONST_VTBL struct IExternalConnectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IExternalConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IExternalConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IExternalConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IExternalConnection_AddConnection(This,extconn,reserved)	\
    (This)->lpVtbl -> AddConnection(This,extconn,reserved)

#define IExternalConnection_ReleaseConnection(This,extconn,reserved,fLastReleaseCloses)	\
    (This)->lpVtbl -> ReleaseConnection(This,extconn,reserved,fLastReleaseCloses)

#endif /* COBJMACROS */


#endif 	/* C style interface */



DWORD STDMETHODCALLTYPE IExternalConnection_AddConnection_Proxy( 
    IExternalConnection __RPC_FAR * This,
    /* [in] */ DWORD extconn,
    /* [in] */ DWORD reserved);


void __RPC_STUB IExternalConnection_AddConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD STDMETHODCALLTYPE IExternalConnection_ReleaseConnection_Proxy( 
    IExternalConnection __RPC_FAR * This,
    /* [in] */ DWORD extconn,
    /* [in] */ DWORD reserved,
    /* [in] */ BOOL fLastReleaseCloses);


void __RPC_STUB IExternalConnection_ReleaseConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IExternalConnection_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_objidl_0015 */
/* [local] */ 

typedef /* [unique] */ IMultiQI __RPC_FAR *LPMULTIQI;

typedef struct tagMULTI_QI
    {
    const IID __RPC_FAR *pIID;
    IUnknown __RPC_FAR *pItf;
    HRESULT hr;
    }	MULTI_QI;



extern RPC_IF_HANDLE __MIDL_itf_objidl_0015_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_objidl_0015_v0_0_s_ifspec;

#ifndef __IMultiQI_INTERFACE_DEFINED__
#define __IMultiQI_INTERFACE_DEFINED__

/* interface IMultiQI */
/* [async_uuid][uuid][local][object] */ 


EXTERN_C const IID IID_IMultiQI;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000020-0000-0000-C000-000000000046")
    IMultiQI : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryMultipleInterfaces( 
            /* [in] */ ULONG cMQIs,
            /* [out][in] */ MULTI_QI __RPC_FAR *pMQIs) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMultiQIVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMultiQI __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMultiQI __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMultiQI __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryMultipleInterfaces )( 
            IMultiQI __RPC_FAR * This,
            /* [in] */ ULONG cMQIs,
            /* [out][in] */ MULTI_QI __RPC_FAR *pMQIs);
        
        END_INTERFACE
    } IMultiQIVtbl;

    interface IMultiQI
    {
        CONST_VTBL struct IMultiQIVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMultiQI_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMultiQI_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMultiQI_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMultiQI_QueryMultipleInterfaces(This,cMQIs,pMQIs)	\
    (This)->lpVtbl -> QueryMultipleInterfaces(This,cMQIs,pMQIs)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IMultiQI_QueryMultipleInterfaces_Proxy( 
    IMultiQI __RPC_FAR * This,
    /* [in] */ ULONG cMQIs,
    /* [out][in] */ MULTI_QI __RPC_FAR *pMQIs);


void __RPC_STUB IMultiQI_QueryMultipleInterfaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMultiQI_INTERFACE_DEFINED__ */


#ifndef __AsyncIMultiQI_INTERFACE_DEFINED__
#define __AsyncIMultiQI_INTERFACE_DEFINED__

/* interface AsyncIMultiQI */
/* [uuid][local][object] */ 


EXTERN_C const IID IID_AsyncIMultiQI;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000e0020-0000-0000-C000-000000000046")
    AsyncIMultiQI : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Begin_QueryMultipleInterfaces( 
            /* [in] */ ULONG cMQIs,
            /* [out][in] */ MULTI_QI __RPC_FAR *pMQIs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_QueryMultipleInterfaces( 
            /* [out][in] */ MULTI_QI __RPC_FAR *pMQIs) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct AsyncIMultiQIVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            AsyncIMultiQI __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            AsyncIMultiQI __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            AsyncIMultiQI __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Begin_QueryMultipleInterfaces )( 
            AsyncIMultiQI __RPC_FAR * This,
            /* [in] */ ULONG cMQIs,
            /* [out][in] */ MULTI_QI __RPC_FAR *pMQIs);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Finish_QueryMultipleInterfaces )( 
            AsyncIMultiQI __RPC_FAR * This,
            /* [out][in] */ MULTI_QI __RPC_FAR *pMQIs);
        
        END_INTERFACE
    } AsyncIMultiQIVtbl;

    interface AsyncIMultiQI
    {
        CONST_VTBL struct AsyncIMultiQIVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define AsyncIMultiQI_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define AsyncIMultiQI_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define AsyncIMultiQI_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define AsyncIMultiQI_Begin_QueryMultipleInterfaces(This,cMQIs,pMQIs)	\
    (This)->lpVtbl -> Begin_QueryMultipleInterfaces(This,cMQIs,pMQIs)

#define AsyncIMultiQI_Finish_QueryMultipleInterfaces(This,pMQIs)	\
    (This)->lpVtbl -> Finish_QueryMultipleInterfaces(This,pMQIs)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE AsyncIMultiQI_Begin_QueryMultipleInterfaces_Proxy( 
    AsyncIMultiQI __RPC_FAR * This,
    /* [in] */ ULONG cMQIs,
    /* [out][in] */ MULTI_QI __RPC_FAR *pMQIs);


void __RPC_STUB AsyncIMultiQI_Begin_QueryMultipleInterfaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIMultiQI_Finish_QueryMultipleInterfaces_Proxy( 
    AsyncIMultiQI __RPC_FAR * This,
    /* [out][in] */ MULTI_QI __RPC_FAR *pMQIs);


void __RPC_STUB AsyncIMultiQI_Finish_QueryMultipleInterfaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __AsyncIMultiQI_INTERFACE_DEFINED__ */


#ifndef __IInternalUnknown_INTERFACE_DEFINED__
#define __IInternalUnknown_INTERFACE_DEFINED__

/* interface IInternalUnknown */
/* [uuid][local][object] */ 


EXTERN_C const IID IID_IInternalUnknown;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000021-0000-0000-C000-000000000046")
    IInternalUnknown : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryInternalInterface( 
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IInternalUnknownVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInternalUnknown __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInternalUnknown __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInternalUnknown __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInternalInterface )( 
            IInternalUnknown __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        END_INTERFACE
    } IInternalUnknownVtbl;

    interface IInternalUnknown
    {
        CONST_VTBL struct IInternalUnknownVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternalUnknown_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternalUnknown_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternalUnknown_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternalUnknown_QueryInternalInterface(This,riid,ppv)	\
    (This)->lpVtbl -> QueryInternalInterface(This,riid,ppv)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IInternalUnknown_QueryInternalInterface_Proxy( 
    IInternalUnknown __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB IInternalUnknown_QueryInternalInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IInternalUnknown_INTERFACE_DEFINED__ */


#ifndef __IEnumUnknown_INTERFACE_DEFINED__
#define __IEnumUnknown_INTERFACE_DEFINED__

/* interface IEnumUnknown */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IEnumUnknown __RPC_FAR *LPENUMUNKNOWN;


EXTERN_C const IID IID_IEnumUnknown;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000100-0000-0000-C000-000000000046")
    IEnumUnknown : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG celt,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumUnknown __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumUnknownVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumUnknown __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumUnknown __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumUnknown __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumUnknown __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumUnknown __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumUnknown __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumUnknown __RPC_FAR * This,
            /* [out] */ IEnumUnknown __RPC_FAR *__RPC_FAR *ppenum);
        
        END_INTERFACE
    } IEnumUnknownVtbl;

    interface IEnumUnknown
    {
        CONST_VTBL struct IEnumUnknownVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumUnknown_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumUnknown_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumUnknown_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumUnknown_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumUnknown_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumUnknown_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumUnknown_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumUnknown_RemoteNext_Proxy( 
    IEnumUnknown __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumUnknown_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumUnknown_Skip_Proxy( 
    IEnumUnknown __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumUnknown_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumUnknown_Reset_Proxy( 
    IEnumUnknown __RPC_FAR * This);


void __RPC_STUB IEnumUnknown_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumUnknown_Clone_Proxy( 
    IEnumUnknown __RPC_FAR * This,
    /* [out] */ IEnumUnknown __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumUnknown_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumUnknown_INTERFACE_DEFINED__ */


#ifndef __IBindCtx_INTERFACE_DEFINED__
#define __IBindCtx_INTERFACE_DEFINED__

/* interface IBindCtx */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IBindCtx __RPC_FAR *LPBC;

typedef /* [unique] */ IBindCtx __RPC_FAR *LPBINDCTX;

typedef struct tagBIND_OPTS
    {
    DWORD cbStruct;
    DWORD grfFlags;
    DWORD grfMode;
    DWORD dwTickCountDeadline;
    }	BIND_OPTS;

typedef struct tagBIND_OPTS __RPC_FAR *LPBIND_OPTS;

#if defined(__cplusplus)
    typedef struct tagBIND_OPTS2 : tagBIND_OPTS{
    DWORD           dwTrackFlags;
    DWORD           dwClassContext;
    LCID            locale;
    COSERVERINFO *  pServerInfo;
    } BIND_OPTS2, * LPBIND_OPTS2;
#else
typedef struct tagBIND_OPTS2
    {
    DWORD cbStruct;
    DWORD grfFlags;
    DWORD grfMode;
    DWORD dwTickCountDeadline;
    DWORD dwTrackFlags;
    DWORD dwClassContext;
    LCID locale;
    COSERVERINFO __RPC_FAR *pServerInfo;
    }	BIND_OPTS2;

typedef struct tagBIND_OPTS2 __RPC_FAR *LPBIND_OPTS2;

#endif
typedef 
enum tagBIND_FLAGS
    {	BIND_MAYBOTHERUSER	= 1,
	BIND_JUSTTESTEXISTENCE	= 2
    }	BIND_FLAGS;


EXTERN_C const IID IID_IBindCtx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000000e-0000-0000-C000-000000000046")
    IBindCtx : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterObjectBound( 
            /* [unique][in] */ IUnknown __RPC_FAR *punk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevokeObjectBound( 
            /* [unique][in] */ IUnknown __RPC_FAR *punk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseBoundObjects( void) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE SetBindOptions( 
            /* [in] */ BIND_OPTS __RPC_FAR *pbindopts) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetBindOptions( 
            /* [out][in] */ BIND_OPTS __RPC_FAR *pbindopts) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRunningObjectTable( 
            /* [out] */ IRunningObjectTable __RPC_FAR *__RPC_FAR *pprot) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterObjectParam( 
            /* [in] */ LPOLESTR pszKey,
            /* [unique][in] */ IUnknown __RPC_FAR *punk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjectParam( 
            /* [in] */ LPOLESTR pszKey,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumObjectParam( 
            /* [out] */ IEnumString __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevokeObjectParam( 
            /* [in] */ LPOLESTR pszKey) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBindCtxVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBindCtx __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBindCtx __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBindCtx __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RegisterObjectBound )( 
            IBindCtx __RPC_FAR * This,
            /* [unique][in] */ IUnknown __RPC_FAR *punk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RevokeObjectBound )( 
            IBindCtx __RPC_FAR * This,
            /* [unique][in] */ IUnknown __RPC_FAR *punk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseBoundObjects )( 
            IBindCtx __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetBindOptions )( 
            IBindCtx __RPC_FAR * This,
            /* [in] */ BIND_OPTS __RPC_FAR *pbindopts);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBindOptions )( 
            IBindCtx __RPC_FAR * This,
            /* [out][in] */ BIND_OPTS __RPC_FAR *pbindopts);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRunningObjectTable )( 
            IBindCtx __RPC_FAR * This,
            /* [out] */ IRunningObjectTable __RPC_FAR *__RPC_FAR *pprot);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RegisterObjectParam )( 
            IBindCtx __RPC_FAR * This,
            /* [in] */ LPOLESTR pszKey,
            /* [unique][in] */ IUnknown __RPC_FAR *punk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetObjectParam )( 
            IBindCtx __RPC_FAR * This,
            /* [in] */ LPOLESTR pszKey,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumObjectParam )( 
            IBindCtx __RPC_FAR * This,
            /* [out] */ IEnumString __RPC_FAR *__RPC_FAR *ppenum);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RevokeObjectParam )( 
            IBindCtx __RPC_FAR * This,
            /* [in] */ LPOLESTR pszKey);
        
        END_INTERFACE
    } IBindCtxVtbl;

    interface IBindCtx
    {
        CONST_VTBL struct IBindCtxVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBindCtx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBindCtx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBindCtx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBindCtx_RegisterObjectBound(This,punk)	\
    (This)->lpVtbl -> RegisterObjectBound(This,punk)

#define IBindCtx_RevokeObjectBound(This,punk)	\
    (This)->lpVtbl -> RevokeObjectBound(This,punk)

#define IBindCtx_ReleaseBoundObjects(This)	\
    (This)->lpVtbl -> ReleaseBoundObjects(This)

#define IBindCtx_SetBindOptions(This,pbindopts)	\
    (This)->lpVtbl -> SetBindOptions(This,pbindopts)

#define IBindCtx_GetBindOptions(This,pbindopts)	\
    (This)->lpVtbl -> GetBindOptions(This,pbindopts)

#define IBindCtx_GetRunningObjectTable(This,pprot)	\
    (This)->lpVtbl -> GetRunningObjectTable(This,pprot)

#define IBindCtx_RegisterObjectParam(This,pszKey,punk)	\
    (This)->lpVtbl -> RegisterObjectParam(This,pszKey,punk)

#define IBindCtx_GetObjectParam(This,pszKey,ppunk)	\
    (This)->lpVtbl -> GetObjectParam(This,pszKey,ppunk)

#define IBindCtx_EnumObjectParam(This,ppenum)	\
    (This)->lpVtbl -> EnumObjectParam(This,ppenum)

#define IBindCtx_RevokeObjectParam(This,pszKey)	\
    (This)->lpVtbl -> RevokeObjectParam(This,pszKey)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IBindCtx_RegisterObjectBound_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [unique][in] */ IUnknown __RPC_FAR *punk);


void __RPC_STUB IBindCtx_RegisterObjectBound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindCtx_RevokeObjectBound_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [unique][in] */ IUnknown __RPC_FAR *punk);


void __RPC_STUB IBindCtx_RevokeObjectBound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindCtx_ReleaseBoundObjects_Proxy( 
    IBindCtx __RPC_FAR * This);


void __RPC_STUB IBindCtx_ReleaseBoundObjects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IBindCtx_RemoteSetBindOptions_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [in] */ BIND_OPTS2 __RPC_FAR *pbindopts);


void __RPC_STUB IBindCtx_RemoteSetBindOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IBindCtx_RemoteGetBindOptions_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [out][in] */ BIND_OPTS2 __RPC_FAR *pbindopts);


void __RPC_STUB IBindCtx_RemoteGetBindOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindCtx_GetRunningObjectTable_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [out] */ IRunningObjectTable __RPC_FAR *__RPC_FAR *pprot);


void __RPC_STUB IBindCtx_GetRunningObjectTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindCtx_RegisterObjectParam_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [in] */ LPOLESTR pszKey,
    /* [unique][in] */ IUnknown __RPC_FAR *punk);


void __RPC_STUB IBindCtx_RegisterObjectParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindCtx_GetObjectParam_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [in] */ LPOLESTR pszKey,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);


void __RPC_STUB IBindCtx_GetObjectParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindCtx_EnumObjectParam_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [out] */ IEnumString __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IBindCtx_EnumObjectParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindCtx_RevokeObjectParam_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [in] */ LPOLESTR pszKey);


void __RPC_STUB IBindCtx_RevokeObjectParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBindCtx_INTERFACE_DEFINED__ */


#ifndef __IEnumMoniker_INTERFACE_DEFINED__
#define __IEnumMoniker_INTERFACE_DEFINED__

/* interface IEnumMoniker */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IEnumMoniker __RPC_FAR *LPENUMMONIKER;


EXTERN_C const IID IID_IEnumMoniker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000102-0000-0000-C000-000000000046")
    IEnumMoniker : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ IMoniker __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumMonikerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumMoniker __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumMoniker __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumMoniker __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumMoniker __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ IMoniker __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumMoniker __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumMoniker __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumMoniker __RPC_FAR * This,
            /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenum);
        
        END_INTERFACE
    } IEnumMonikerVtbl;

    interface IEnumMoniker
    {
        CONST_VTBL struct IEnumMonikerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumMoniker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumMoniker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumMoniker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumMoniker_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumMoniker_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumMoniker_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumMoniker_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumMoniker_RemoteNext_Proxy( 
    IEnumMoniker __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IMoniker __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumMoniker_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMoniker_Skip_Proxy( 
    IEnumMoniker __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumMoniker_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMoniker_Reset_Proxy( 
    IEnumMoniker __RPC_FAR * This);


void __RPC_STUB IEnumMoniker_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMoniker_Clone_Proxy( 
    IEnumMoniker __RPC_FAR * This,
    /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumMoniker_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumMoniker_INTERFACE_DEFINED__ */


#ifndef __IRunnableObject_INTERFACE_DEFINED__
#define __IRunnableObject_INTERFACE_DEFINED__

/* interface IRunnableObject */
/* [uuid][object] */ 

typedef /* [unique] */ IRunnableObject __RPC_FAR *LPRUNNABLEOBJECT;


EXTERN_C const IID IID_IRunnableObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000126-0000-0000-C000-000000000046")
    IRunnableObject : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRunningClass( 
            /* [out] */ LPCLSID lpClsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Run( 
            /* [in] */ LPBINDCTX pbc) = 0;
        
        virtual /* [local] */ BOOL STDMETHODCALLTYPE IsRunning( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LockRunning( 
            /* [in] */ BOOL fLock,
            /* [in] */ BOOL fLastUnlockCloses) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetContainedObject( 
            /* [in] */ BOOL fContained) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRunnableObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRunnableObject __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRunnableObject __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRunnableObject __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRunningClass )( 
            IRunnableObject __RPC_FAR * This,
            /* [out] */ LPCLSID lpClsid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Run )( 
            IRunnableObject __RPC_FAR * This,
            /* [in] */ LPBINDCTX pbc);
        
        /* [local] */ BOOL ( STDMETHODCALLTYPE __RPC_FAR *IsRunning )( 
            IRunnableObject __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LockRunning )( 
            IRunnableObject __RPC_FAR * This,
            /* [in] */ BOOL fLock,
            /* [in] */ BOOL fLastUnlockCloses);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetContainedObject )( 
            IRunnableObject __RPC_FAR * This,
            /* [in] */ BOOL fContained);
        
        END_INTERFACE
    } IRunnableObjectVtbl;

    interface IRunnableObject
    {
        CONST_VTBL struct IRunnableObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRunnableObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRunnableObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRunnableObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRunnableObject_GetRunningClass(This,lpClsid)	\
    (This)->lpVtbl -> GetRunningClass(This,lpClsid)

#define IRunnableObject_Run(This,pbc)	\
    (This)->lpVtbl -> Run(This,pbc)

#define IRunnableObject_IsRunning(This)	\
    (This)->lpVtbl -> IsRunning(This)

#define IRunnableObject_LockRunning(This,fLock,fLastUnlockCloses)	\
    (This)->lpVtbl -> LockRunning(This,fLock,fLastUnlockCloses)

#define IRunnableObject_SetContainedObject(This,fContained)	\
    (This)->lpVtbl -> SetContainedObject(This,fContained)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRunnableObject_GetRunningClass_Proxy( 
    IRunnableObject __RPC_FAR * This,
    /* [out] */ LPCLSID lpClsid);


void __RPC_STUB IRunnableObject_GetRunningClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunnableObject_Run_Proxy( 
    IRunnableObject __RPC_FAR * This,
    /* [in] */ LPBINDCTX pbc);


void __RPC_STUB IRunnableObject_Run_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IRunnableObject_RemoteIsRunning_Proxy( 
    IRunnableObject __RPC_FAR * This);


void __RPC_STUB IRunnableObject_RemoteIsRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunnableObject_LockRunning_Proxy( 
    IRunnableObject __RPC_FAR * This,
    /* [in] */ BOOL fLock,
    /* [in] */ BOOL fLastUnlockCloses);


void __RPC_STUB IRunnableObject_LockRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunnableObject_SetContainedObject_Proxy( 
    IRunnableObject __RPC_FAR * This,
    /* [in] */ BOOL fContained);


void __RPC_STUB IRunnableObject_SetContainedObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRunnableObject_INTERFACE_DEFINED__ */


#ifndef __IRunningObjectTable_INTERFACE_DEFINED__
#define __IRunningObjectTable_INTERFACE_DEFINED__

/* interface IRunningObjectTable */
/* [uuid][object] */ 

typedef /* [unique] */ IRunningObjectTable __RPC_FAR *LPRUNNINGOBJECTTABLE;


EXTERN_C const IID IID_IRunningObjectTable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000010-0000-0000-C000-000000000046")
    IRunningObjectTable : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Register( 
            /* [in] */ DWORD grfFlags,
            /* [unique][in] */ IUnknown __RPC_FAR *punkObject,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
            /* [out] */ DWORD __RPC_FAR *pdwRegister) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Revoke( 
            /* [in] */ DWORD dwRegister) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsRunning( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObject( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NoteChangeTime( 
            /* [in] */ DWORD dwRegister,
            /* [in] */ FILETIME __RPC_FAR *pfiletime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTimeOfLastChange( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
            /* [out] */ FILETIME __RPC_FAR *pfiletime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumRunning( 
            /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenumMoniker) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRunningObjectTableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRunningObjectTable __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRunningObjectTable __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRunningObjectTable __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Register )( 
            IRunningObjectTable __RPC_FAR * This,
            /* [in] */ DWORD grfFlags,
            /* [unique][in] */ IUnknown __RPC_FAR *punkObject,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
            /* [out] */ DWORD __RPC_FAR *pdwRegister);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Revoke )( 
            IRunningObjectTable __RPC_FAR * This,
            /* [in] */ DWORD dwRegister);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsRunning )( 
            IRunningObjectTable __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetObject )( 
            IRunningObjectTable __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkObject);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NoteChangeTime )( 
            IRunningObjectTable __RPC_FAR * This,
            /* [in] */ DWORD dwRegister,
            /* [in] */ FILETIME __RPC_FAR *pfiletime);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTimeOfLastChange )( 
            IRunningObjectTable __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
            /* [out] */ FILETIME __RPC_FAR *pfiletime);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumRunning )( 
            IRunningObjectTable __RPC_FAR * This,
            /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenumMoniker);
        
        END_INTERFACE
    } IRunningObjectTableVtbl;

    interface IRunningObjectTable
    {
        CONST_VTBL struct IRunningObjectTableVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRunningObjectTable_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRunningObjectTable_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRunningObjectTable_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRunningObjectTable_Register(This,grfFlags,punkObject,pmkObjectName,pdwRegister)	\
    (This)->lpVtbl -> Register(This,grfFlags,punkObject,pmkObjectName,pdwRegister)

#define IRunningObjectTable_Revoke(This,dwRegister)	\
    (This)->lpVtbl -> Revoke(This,dwRegister)

#define IRunningObjectTable_IsRunning(This,pmkObjectName)	\
    (This)->lpVtbl -> IsRunning(This,pmkObjectName)

#define IRunningObjectTable_GetObject(This,pmkObjectName,ppunkObject)	\
    (This)->lpVtbl -> GetObject(This,pmkObjectName,ppunkObject)

#define IRunningObjectTable_NoteChangeTime(This,dwRegister,pfiletime)	\
    (This)->lpVtbl -> NoteChangeTime(This,dwRegister,pfiletime)

#define IRunningObjectTable_GetTimeOfLastChange(This,pmkObjectName,pfiletime)	\
    (This)->lpVtbl -> GetTimeOfLastChange(This,pmkObjectName,pfiletime)

#define IRunningObjectTable_EnumRunning(This,ppenumMoniker)	\
    (This)->lpVtbl -> EnumRunning(This,ppenumMoniker)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRunningObjectTable_Register_Proxy( 
    IRunningObjectTable __RPC_FAR * This,
    /* [in] */ DWORD grfFlags,
    /* [unique][in] */ IUnknown __RPC_FAR *punkObject,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
    /* [out] */ DWORD __RPC_FAR *pdwRegister);


void __RPC_STUB IRunningObjectTable_Register_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunningObjectTable_Revoke_Proxy( 
    IRunningObjectTable __RPC_FAR * This,
    /* [in] */ DWORD dwRegister);


void __RPC_STUB IRunningObjectTable_Revoke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunningObjectTable_IsRunning_Proxy( 
    IRunningObjectTable __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName);


void __RPC_STUB IRunningObjectTable_IsRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunningObjectTable_GetObject_Proxy( 
    IRunningObjectTable __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkObject);


void __RPC_STUB IRunningObjectTable_GetObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunningObjectTable_NoteChangeTime_Proxy( 
    IRunningObjectTable __RPC_FAR * This,
    /* [in] */ DWORD dwRegister,
    /* [in] */ FILETIME __RPC_FAR *pfiletime);


void __RPC_STUB IRunningObjectTable_NoteChangeTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunningObjectTable_GetTimeOfLastChange_Proxy( 
    IRunningObjectTable __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
    /* [out] */ FILETIME __RPC_FAR *pfiletime);


void __RPC_STUB IRunningObjectTable_GetTimeOfLastChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunningObjectTable_EnumRunning_Proxy( 
    IRunningObjectTable __RPC_FAR * This,
    /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenumMoniker);


void __RPC_STUB IRunningObjectTable_EnumRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRunningObjectTable_INTERFACE_DEFINED__ */


#ifndef __IPersist_INTERFACE_DEFINED__
#define __IPersist_INTERFACE_DEFINED__

/* interface IPersist */
/* [uuid][object] */ 

typedef /* [unique] */ IPersist __RPC_FAR *LPPERSIST;


EXTERN_C const IID IID_IPersist;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000010c-0000-0000-C000-000000000046")
    IPersist : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetClassID( 
            /* [out] */ CLSID __RPC_FAR *pClassID) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPersistVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPersist __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPersist __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPersist __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClassID )( 
            IPersist __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pClassID);
        
        END_INTERFACE
    } IPersistVtbl;

    interface IPersist
    {
        CONST_VTBL struct IPersistVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersist_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersist_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersist_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersist_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPersist_GetClassID_Proxy( 
    IPersist __RPC_FAR * This,
    /* [out] */ CLSID __RPC_FAR *pClassID);


void __RPC_STUB IPersist_GetClassID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPersist_INTERFACE_DEFINED__ */


#ifndef __IPersistStream_INTERFACE_DEFINED__
#define __IPersistStream_INTERFACE_DEFINED__

/* interface IPersistStream */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IPersistStream __RPC_FAR *LPPERSISTSTREAM;


EXTERN_C const IID IID_IPersistStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000109-0000-0000-C000-000000000046")
    IPersistStream : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsDirty( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Load( 
            /* [unique][in] */ IStream __RPC_FAR *pStm) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Save( 
            /* [unique][in] */ IStream __RPC_FAR *pStm,
            /* [in] */ BOOL fClearDirty) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSizeMax( 
            /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbSize) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPersistStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPersistStream __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPersistStream __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPersistStream __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClassID )( 
            IPersistStream __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsDirty )( 
            IPersistStream __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Load )( 
            IPersistStream __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Save )( 
            IPersistStream __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm,
            /* [in] */ BOOL fClearDirty);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSizeMax )( 
            IPersistStream __RPC_FAR * This,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbSize);
        
        END_INTERFACE
    } IPersistStreamVtbl;

    interface IPersistStream
    {
        CONST_VTBL struct IPersistStreamVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistStream_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistStream_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IPersistStream_Load(This,pStm)	\
    (This)->lpVtbl -> Load(This,pStm)

#define IPersistStream_Save(This,pStm,fClearDirty)	\
    (This)->lpVtbl -> Save(This,pStm,fClearDirty)

#define IPersistStream_GetSizeMax(This,pcbSize)	\
    (This)->lpVtbl -> GetSizeMax(This,pcbSize)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPersistStream_IsDirty_Proxy( 
    IPersistStream __RPC_FAR * This);


void __RPC_STUB IPersistStream_IsDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStream_Load_Proxy( 
    IPersistStream __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pStm);


void __RPC_STUB IPersistStream_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStream_Save_Proxy( 
    IPersistStream __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pStm,
    /* [in] */ BOOL fClearDirty);


void __RPC_STUB IPersistStream_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStream_GetSizeMax_Proxy( 
    IPersistStream __RPC_FAR * This,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbSize);


void __RPC_STUB IPersistStream_GetSizeMax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPersistStream_INTERFACE_DEFINED__ */


#ifndef __IMoniker_INTERFACE_DEFINED__
#define __IMoniker_INTERFACE_DEFINED__

/* interface IMoniker */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IMoniker __RPC_FAR *LPMONIKER;

typedef 
enum tagMKSYS
    {	MKSYS_NONE	= 0,
	MKSYS_GENERICCOMPOSITE	= 1,
	MKSYS_FILEMONIKER	= 2,
	MKSYS_ANTIMONIKER	= 3,
	MKSYS_ITEMMONIKER	= 4,
	MKSYS_POINTERMONIKER	= 5,
	MKSYS_CLASSMONIKER	= 7,
	MKSYS_OBJREFMONIKER	= 8,
	MKSYS_SESSIONMONIKER	= 9
    }	MKSYS;

typedef /* [v1_enum] */ 
enum tagMKREDUCE
    {	MKRREDUCE_ONE	= 3 << 16,
	MKRREDUCE_TOUSER	= 2 << 16,
	MKRREDUCE_THROUGHUSER	= 1 << 16,
	MKRREDUCE_ALL	= 0
    }	MKRREDUCE;


EXTERN_C const IID IID_IMoniker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000000f-0000-0000-C000-000000000046")
    IMoniker : public IPersistStream
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE BindToObject( 
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [in] */ REFIID riidResult,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvResult) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE BindToStorage( 
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObj) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reduce( 
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [in] */ DWORD dwReduceHowFar,
            /* [unique][out][in] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkToLeft,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkReduced) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ComposeWith( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmkRight,
            /* [in] */ BOOL fOnlyIfNotGeneric,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkComposite) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enum( 
            /* [in] */ BOOL fForward,
            /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenumMoniker) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsEqual( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmkOtherMoniker) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Hash( 
            /* [out] */ DWORD __RPC_FAR *pdwHash) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsRunning( 
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkNewlyRunning) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTimeOfLastChange( 
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [out] */ FILETIME __RPC_FAR *pFileTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Inverse( 
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CommonPrefixWith( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmkOther,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkPrefix) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RelativePathTo( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmkOther,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkRelPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDisplayName( 
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [out] */ LPOLESTR __RPC_FAR *ppszDisplayName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ParseDisplayName( 
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [in] */ LPOLESTR pszDisplayName,
            /* [out] */ ULONG __RPC_FAR *pchEaten,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsSystemMoniker( 
            /* [out] */ DWORD __RPC_FAR *pdwMksys) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMonikerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMoniker __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMoniker __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMoniker __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClassID )( 
            IMoniker __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsDirty )( 
            IMoniker __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Load )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Save )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm,
            /* [in] */ BOOL fClearDirty);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSizeMax )( 
            IMoniker __RPC_FAR * This,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbSize);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BindToObject )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [in] */ REFIID riidResult,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvResult);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BindToStorage )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObj);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reduce )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [in] */ DWORD dwReduceHowFar,
            /* [unique][out][in] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkToLeft,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkReduced);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ComposeWith )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkRight,
            /* [in] */ BOOL fOnlyIfNotGeneric,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkComposite);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Enum )( 
            IMoniker __RPC_FAR * This,
            /* [in] */ BOOL fForward,
            /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenumMoniker);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsEqual )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkOtherMoniker);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Hash )( 
            IMoniker __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwHash);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsRunning )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkNewlyRunning);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTimeOfLastChange )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [out] */ FILETIME __RPC_FAR *pFileTime);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Inverse )( 
            IMoniker __RPC_FAR * This,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CommonPrefixWith )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkOther,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkPrefix);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RelativePathTo )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkOther,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkRelPath);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDisplayName )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [out] */ LPOLESTR __RPC_FAR *ppszDisplayName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ParseDisplayName )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [in] */ LPOLESTR pszDisplayName,
            /* [out] */ ULONG __RPC_FAR *pchEaten,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkOut);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsSystemMoniker )( 
            IMoniker __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwMksys);
        
        END_INTERFACE
    } IMonikerVtbl;

    interface IMoniker
    {
        CONST_VTBL struct IMonikerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMoniker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMoniker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMoniker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMoniker_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IMoniker_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IMoniker_Load(This,pStm)	\
    (This)->lpVtbl -> Load(This,pStm)

#define IMoniker_Save(This,pStm,fClearDirty)	\
    (This)->lpVtbl -> Save(This,pStm,fClearDirty)

#define IMoniker_GetSizeMax(This,pcbSize)	\
    (This)->lpVtbl -> GetSizeMax(This,pcbSize)


#define IMoniker_BindToObject(This,pbc,pmkToLeft,riidResult,ppvResult)	\
    (This)->lpVtbl -> BindToObject(This,pbc,pmkToLeft,riidResult,ppvResult)

#define IMoniker_BindToStorage(This,pbc,pmkToLeft,riid,ppvObj)	\
    (This)->lpVtbl -> BindToStorage(This,pbc,pmkToLeft,riid,ppvObj)

#define IMoniker_Reduce(This,pbc,dwReduceHowFar,ppmkToLeft,ppmkReduced)	\
    (This)->lpVtbl -> Reduce(This,pbc,dwReduceHowFar,ppmkToLeft,ppmkReduced)

#define IMoniker_ComposeWith(This,pmkRight,fOnlyIfNotGeneric,ppmkComposite)	\
    (This)->lpVtbl -> ComposeWith(This,pmkRight,fOnlyIfNotGeneric,ppmkComposite)

#define IMoniker_Enum(This,fForward,ppenumMoniker)	\
    (This)->lpVtbl -> Enum(This,fForward,ppenumMoniker)

#define IMoniker_IsEqual(This,pmkOtherMoniker)	\
    (This)->lpVtbl -> IsEqual(This,pmkOtherMoniker)

#define IMoniker_Hash(This,pdwHash)	\
    (This)->lpVtbl -> Hash(This,pdwHash)

#define IMoniker_IsRunning(This,pbc,pmkToLeft,pmkNewlyRunning)	\
    (This)->lpVtbl -> IsRunning(This,pbc,pmkToLeft,pmkNewlyRunning)

#define IMoniker_GetTimeOfLastChange(This,pbc,pmkToLeft,pFileTime)	\
    (This)->lpVtbl -> GetTimeOfLastChange(This,pbc,pmkToLeft,pFileTime)

#define IMoniker_Inverse(This,ppmk)	\
    (This)->lpVtbl -> Inverse(This,ppmk)

#define IMoniker_CommonPrefixWith(This,pmkOther,ppmkPrefix)	\
    (This)->lpVtbl -> CommonPrefixWith(This,pmkOther,ppmkPrefix)

#define IMoniker_RelativePathTo(This,pmkOther,ppmkRelPath)	\
    (This)->lpVtbl -> RelativePathTo(This,pmkOther,ppmkRelPath)

#define IMoniker_GetDisplayName(This,pbc,pmkToLeft,ppszDisplayName)	\
    (This)->lpVtbl -> GetDisplayName(This,pbc,pmkToLeft,ppszDisplayName)

#define IMoniker_ParseDisplayName(This,pbc,pmkToLeft,pszDisplayName,pchEaten,ppmkOut)	\
    (This)->lpVtbl -> ParseDisplayName(This,pbc,pmkToLeft,pszDisplayName,pchEaten,ppmkOut)

#define IMoniker_IsSystemMoniker(This,pdwMksys)	\
    (This)->lpVtbl -> IsSystemMoniker(This,pdwMksys)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IMoniker_RemoteBindToObject_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [in] */ REFIID riidResult,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvResult);


void __RPC_STUB IMoniker_RemoteBindToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IMoniker_RemoteBindToStorage_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObj);


void __RPC_STUB IMoniker_RemoteBindToStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_Reduce_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [in] */ DWORD dwReduceHowFar,
    /* [unique][out][in] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkToLeft,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkReduced);


void __RPC_STUB IMoniker_Reduce_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_ComposeWith_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkRight,
    /* [in] */ BOOL fOnlyIfNotGeneric,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkComposite);


void __RPC_STUB IMoniker_ComposeWith_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_Enum_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [in] */ BOOL fForward,
    /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenumMoniker);


void __RPC_STUB IMoniker_Enum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_IsEqual_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkOtherMoniker);


void __RPC_STUB IMoniker_IsEqual_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_Hash_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwHash);


void __RPC_STUB IMoniker_Hash_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_IsRunning_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkNewlyRunning);


void __RPC_STUB IMoniker_IsRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_GetTimeOfLastChange_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [out] */ FILETIME __RPC_FAR *pFileTime);


void __RPC_STUB IMoniker_GetTimeOfLastChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_Inverse_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);


void __RPC_STUB IMoniker_Inverse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_CommonPrefixWith_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkOther,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkPrefix);


void __RPC_STUB IMoniker_CommonPrefixWith_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_RelativePathTo_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkOther,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkRelPath);


void __RPC_STUB IMoniker_RelativePathTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_GetDisplayName_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [out] */ LPOLESTR __RPC_FAR *ppszDisplayName);


void __RPC_STUB IMoniker_GetDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_ParseDisplayName_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [in] */ LPOLESTR pszDisplayName,
    /* [out] */ ULONG __RPC_FAR *pchEaten,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkOut);


void __RPC_STUB IMoniker_ParseDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_IsSystemMoniker_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwMksys);


void __RPC_STUB IMoniker_IsSystemMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMoniker_INTERFACE_DEFINED__ */


#ifndef __IROTData_INTERFACE_DEFINED__
#define __IROTData_INTERFACE_DEFINED__

/* interface IROTData */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IROTData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f29f6bc0-5021-11ce-aa15-00006901293f")
    IROTData : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetComparisonData( 
            /* [size_is][out] */ byte __RPC_FAR *pbData,
            /* [in] */ ULONG cbMax,
            /* [out] */ ULONG __RPC_FAR *pcbData) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IROTDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IROTData __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IROTData __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IROTData __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetComparisonData )( 
            IROTData __RPC_FAR * This,
            /* [size_is][out] */ byte __RPC_FAR *pbData,
            /* [in] */ ULONG cbMax,
            /* [out] */ ULONG __RPC_FAR *pcbData);
        
        END_INTERFACE
    } IROTDataVtbl;

    interface IROTData
    {
        CONST_VTBL struct IROTDataVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IROTData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IROTData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IROTData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IROTData_GetComparisonData(This,pbData,cbMax,pcbData)	\
    (This)->lpVtbl -> GetComparisonData(This,pbData,cbMax,pcbData)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IROTData_GetComparisonData_Proxy( 
    IROTData __RPC_FAR * This,
    /* [size_is][out] */ byte __RPC_FAR *pbData,
    /* [in] */ ULONG cbMax,
    /* [out] */ ULONG __RPC_FAR *pcbData);


void __RPC_STUB IROTData_GetComparisonData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IROTData_INTERFACE_DEFINED__ */


#ifndef __IEnumString_INTERFACE_DEFINED__
#define __IEnumString_INTERFACE_DEFINED__

/* interface IEnumString */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IEnumString __RPC_FAR *LPENUMSTRING;


EXTERN_C const IID IID_IEnumString;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000101-0000-0000-C000-000000000046")
    IEnumString : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ LPOLESTR __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumString __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumStringVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumString __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumString __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumString __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumString __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ LPOLESTR __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumString __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumString __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumString __RPC_FAR * This,
            /* [out] */ IEnumString __RPC_FAR *__RPC_FAR *ppenum);
        
        END_INTERFACE
    } IEnumStringVtbl;

    interface IEnumString
    {
        CONST_VTBL struct IEnumStringVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumString_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumString_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumString_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumString_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumString_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumString_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumString_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumString_RemoteNext_Proxy( 
    IEnumString __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ LPOLESTR __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumString_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumString_Skip_Proxy( 
    IEnumString __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumString_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumString_Reset_Proxy( 
    IEnumString __RPC_FAR * This);


void __RPC_STUB IEnumString_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumString_Clone_Proxy( 
    IEnumString __RPC_FAR * This,
    /* [out] */ IEnumString __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumString_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumString_INTERFACE_DEFINED__ */


#ifndef __ISequentialStream_INTERFACE_DEFINED__
#define __ISequentialStream_INTERFACE_DEFINED__

/* interface ISequentialStream */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_ISequentialStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a30-2a1c-11ce-ade5-00aa0044773d")
    ISequentialStream : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Read( 
            /* [length_is][size_is][out] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Write( 
            /* [size_is][in] */ const void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISequentialStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISequentialStream __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISequentialStream __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISequentialStream __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Read )( 
            ISequentialStream __RPC_FAR * This,
            /* [length_is][size_is][out] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Write )( 
            ISequentialStream __RPC_FAR * This,
            /* [size_is][in] */ const void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten);
        
        END_INTERFACE
    } ISequentialStreamVtbl;

    interface ISequentialStream
    {
        CONST_VTBL struct ISequentialStreamVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISequentialStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISequentialStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISequentialStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISequentialStream_Read(This,pv,cb,pcbRead)	\
    (This)->lpVtbl -> Read(This,pv,cb,pcbRead)

#define ISequentialStream_Write(This,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> Write(This,pv,cb,pcbWritten)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE ISequentialStream_RemoteRead_Proxy( 
    ISequentialStream __RPC_FAR * This,
    /* [length_is][size_is][out] */ byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbRead);


void __RPC_STUB ISequentialStream_RemoteRead_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE ISequentialStream_RemoteWrite_Proxy( 
    ISequentialStream __RPC_FAR * This,
    /* [size_is][in] */ const byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);


void __RPC_STUB ISequentialStream_RemoteWrite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISequentialStream_INTERFACE_DEFINED__ */


#ifndef __IStream_INTERFACE_DEFINED__
#define __IStream_INTERFACE_DEFINED__

/* interface IStream */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IStream __RPC_FAR *LPSTREAM;

//FSSpec is Macintosh only, defined in macos\files.h
#ifdef _MAC
    typedef struct tagSTATSTG
    {                      
        LPOLESTR pwcsName;
            FSSpec *pspec;
        DWORD type;
        ULARGE_INTEGER cbSize;
        FILETIME mtime;
        FILETIME ctime;
        FILETIME atime;
        DWORD grfMode;
        DWORD grfLocksSupported;
        CLSID clsid;
        DWORD grfStateBits;
        DWORD reserved;
    } STATSTG;
#else //_MAC
typedef struct tagSTATSTG
    {
    LPOLESTR pwcsName;
    DWORD type;
    ULARGE_INTEGER cbSize;
    FILETIME mtime;
    FILETIME ctime;
    FILETIME atime;
    DWORD grfMode;
    DWORD grfLocksSupported;
    CLSID clsid;
    DWORD grfStateBits;
    DWORD reserved;
    }	STATSTG;

#endif //_MAC
typedef 
enum tagSTGTY
    {	STGTY_STORAGE	= 1,
	STGTY_STREAM	= 2,
	STGTY_LOCKBYTES	= 3,
	STGTY_PROPERTY	= 4
    }	STGTY;

typedef 
enum tagSTREAM_SEEK
    {	STREAM_SEEK_SET	= 0,
	STREAM_SEEK_CUR	= 1,
	STREAM_SEEK_END	= 2
    }	STREAM_SEEK;

typedef 
enum tagLOCKTYPE
    {	LOCK_WRITE	= 1,
	LOCK_EXCLUSIVE	= 2,
	LOCK_ONLYONCE	= 4
    }	LOCKTYPE;


EXTERN_C const IID IID_IStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000000c-0000-0000-C000-000000000046")
    IStream : public ISequentialStream
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Seek( 
            /* [in] */ LARGE_INTEGER dlibMove,
            /* [in] */ DWORD dwOrigin,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *plibNewPosition) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSize( 
            /* [in] */ ULARGE_INTEGER libNewSize) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE CopyTo( 
            /* [unique][in] */ IStream __RPC_FAR *pstm,
            /* [in] */ ULARGE_INTEGER cb,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbRead,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbWritten) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( 
            /* [in] */ DWORD grfCommitFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Revert( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LockRegion( 
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnlockRegion( 
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stat( 
            /* [out] */ STATSTG __RPC_FAR *pstatstg,
            /* [in] */ DWORD grfStatFlag) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IStream __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IStream __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IStream __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Read )( 
            IStream __RPC_FAR * This,
            /* [length_is][size_is][out] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Write )( 
            IStream __RPC_FAR * This,
            /* [size_is][in] */ const void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Seek )( 
            IStream __RPC_FAR * This,
            /* [in] */ LARGE_INTEGER dlibMove,
            /* [in] */ DWORD dwOrigin,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *plibNewPosition);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSize )( 
            IStream __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER libNewSize);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CopyTo )( 
            IStream __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pstm,
            /* [in] */ ULARGE_INTEGER cb,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbRead,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbWritten);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Commit )( 
            IStream __RPC_FAR * This,
            /* [in] */ DWORD grfCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Revert )( 
            IStream __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LockRegion )( 
            IStream __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UnlockRegion )( 
            IStream __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Stat )( 
            IStream __RPC_FAR * This,
            /* [out] */ STATSTG __RPC_FAR *pstatstg,
            /* [in] */ DWORD grfStatFlag);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IStream __RPC_FAR * This,
            /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);
        
        END_INTERFACE
    } IStreamVtbl;

    interface IStream
    {
        CONST_VTBL struct IStreamVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStream_Read(This,pv,cb,pcbRead)	\
    (This)->lpVtbl -> Read(This,pv,cb,pcbRead)

#define IStream_Write(This,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> Write(This,pv,cb,pcbWritten)


#define IStream_Seek(This,dlibMove,dwOrigin,plibNewPosition)	\
    (This)->lpVtbl -> Seek(This,dlibMove,dwOrigin,plibNewPosition)

#define IStream_SetSize(This,libNewSize)	\
    (This)->lpVtbl -> SetSize(This,libNewSize)

#define IStream_CopyTo(This,pstm,cb,pcbRead,pcbWritten)	\
    (This)->lpVtbl -> CopyTo(This,pstm,cb,pcbRead,pcbWritten)

#define IStream_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define IStream_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define IStream_LockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> LockRegion(This,libOffset,cb,dwLockType)

#define IStream_UnlockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> UnlockRegion(This,libOffset,cb,dwLockType)

#define IStream_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#define IStream_Clone(This,ppstm)	\
    (This)->lpVtbl -> Clone(This,ppstm)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IStream_RemoteSeek_Proxy( 
    IStream __RPC_FAR * This,
    /* [in] */ LARGE_INTEGER dlibMove,
    /* [in] */ DWORD dwOrigin,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *plibNewPosition);


void __RPC_STUB IStream_RemoteSeek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStream_SetSize_Proxy( 
    IStream __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER libNewSize);


void __RPC_STUB IStream_SetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStream_RemoteCopyTo_Proxy( 
    IStream __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pstm,
    /* [in] */ ULARGE_INTEGER cb,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbRead,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbWritten);


void __RPC_STUB IStream_RemoteCopyTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStream_Commit_Proxy( 
    IStream __RPC_FAR * This,
    /* [in] */ DWORD grfCommitFlags);


void __RPC_STUB IStream_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStream_Revert_Proxy( 
    IStream __RPC_FAR * This);


void __RPC_STUB IStream_Revert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStream_LockRegion_Proxy( 
    IStream __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER libOffset,
    /* [in] */ ULARGE_INTEGER cb,
    /* [in] */ DWORD dwLockType);


void __RPC_STUB IStream_LockRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStream_UnlockRegion_Proxy( 
    IStream __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER libOffset,
    /* [in] */ ULARGE_INTEGER cb,
    /* [in] */ DWORD dwLockType);


void __RPC_STUB IStream_UnlockRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStream_Stat_Proxy( 
    IStream __RPC_FAR * This,
    /* [out] */ STATSTG __RPC_FAR *pstatstg,
    /* [in] */ DWORD grfStatFlag);


void __RPC_STUB IStream_Stat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStream_Clone_Proxy( 
    IStream __RPC_FAR * This,
    /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);


void __RPC_STUB IStream_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IStream_INTERFACE_DEFINED__ */


#ifndef __IEnumSTATSTG_INTERFACE_DEFINED__
#define __IEnumSTATSTG_INTERFACE_DEFINED__

/* interface IEnumSTATSTG */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IEnumSTATSTG __RPC_FAR *LPENUMSTATSTG;


EXTERN_C const IID IID_IEnumSTATSTG;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000000d-0000-0000-C000-000000000046")
    IEnumSTATSTG : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ STATSTG __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumSTATSTGVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumSTATSTG __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumSTATSTG __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumSTATSTG __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumSTATSTG __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ STATSTG __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumSTATSTG __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumSTATSTG __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumSTATSTG __RPC_FAR * This,
            /* [out] */ IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum);
        
        END_INTERFACE
    } IEnumSTATSTGVtbl;

    interface IEnumSTATSTG
    {
        CONST_VTBL struct IEnumSTATSTGVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSTATSTG_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSTATSTG_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSTATSTG_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSTATSTG_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumSTATSTG_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSTATSTG_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSTATSTG_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumSTATSTG_RemoteNext_Proxy( 
    IEnumSTATSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumSTATSTG_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATSTG_Skip_Proxy( 
    IEnumSTATSTG __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumSTATSTG_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATSTG_Reset_Proxy( 
    IEnumSTATSTG __RPC_FAR * This);


void __RPC_STUB IEnumSTATSTG_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATSTG_Clone_Proxy( 
    IEnumSTATSTG __RPC_FAR * This,
    /* [out] */ IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumSTATSTG_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumSTATSTG_INTERFACE_DEFINED__ */


#ifndef __IStorage_INTERFACE_DEFINED__
#define __IStorage_INTERFACE_DEFINED__

/* interface IStorage */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IStorage __RPC_FAR *LPSTORAGE;

typedef struct tagRemSNB
    {
    unsigned long ulCntStr;
    unsigned long ulCntChar;
    /* [size_is] */ OLECHAR rgString[ 1 ];
    }	RemSNB;

typedef /* [unique] */ RemSNB __RPC_FAR *wireSNB;

typedef /* [wire_marshal] */ OLECHAR __RPC_FAR *__RPC_FAR *SNB;


EXTERN_C const IID IID_IStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000000b-0000-0000-C000-000000000046")
    IStorage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateStream( 
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [in] */ DWORD grfMode,
            /* [in] */ DWORD reserved1,
            /* [in] */ DWORD reserved2,
            /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE OpenStream( 
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ void __RPC_FAR *reserved1,
            /* [in] */ DWORD grfMode,
            /* [in] */ DWORD reserved2,
            /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateStorage( 
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [in] */ DWORD grfMode,
            /* [in] */ DWORD reserved1,
            /* [in] */ DWORD reserved2,
            /* [out] */ IStorage __RPC_FAR *__RPC_FAR *ppstg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenStorage( 
            /* [string][unique][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ IStorage __RPC_FAR *pstgPriority,
            /* [in] */ DWORD grfMode,
            /* [unique][in] */ SNB snbExclude,
            /* [in] */ DWORD reserved,
            /* [out] */ IStorage __RPC_FAR *__RPC_FAR *ppstg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyTo( 
            /* [in] */ DWORD ciidExclude,
            /* [size_is][unique][in] */ const IID __RPC_FAR *rgiidExclude,
            /* [unique][in] */ SNB snbExclude,
            /* [unique][in] */ IStorage __RPC_FAR *pstgDest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MoveElementTo( 
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ IStorage __RPC_FAR *pstgDest,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsNewName,
            /* [in] */ DWORD grfFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( 
            /* [in] */ DWORD grfCommitFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Revert( void) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE EnumElements( 
            /* [in] */ DWORD reserved1,
            /* [size_is][unique][in] */ void __RPC_FAR *reserved2,
            /* [in] */ DWORD reserved3,
            /* [out] */ IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DestroyElement( 
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RenameElement( 
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsOldName,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsNewName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetElementTimes( 
            /* [string][unique][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ const FILETIME __RPC_FAR *pctime,
            /* [unique][in] */ const FILETIME __RPC_FAR *patime,
            /* [unique][in] */ const FILETIME __RPC_FAR *pmtime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetClass( 
            /* [in] */ REFCLSID clsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetStateBits( 
            /* [in] */ DWORD grfStateBits,
            /* [in] */ DWORD grfMask) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stat( 
            /* [out] */ STATSTG __RPC_FAR *pstatstg,
            /* [in] */ DWORD grfStatFlag) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IStorage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IStorage __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IStorage __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateStream )( 
            IStorage __RPC_FAR * This,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [in] */ DWORD grfMode,
            /* [in] */ DWORD reserved1,
            /* [in] */ DWORD reserved2,
            /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenStream )( 
            IStorage __RPC_FAR * This,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ void __RPC_FAR *reserved1,
            /* [in] */ DWORD grfMode,
            /* [in] */ DWORD reserved2,
            /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateStorage )( 
            IStorage __RPC_FAR * This,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [in] */ DWORD grfMode,
            /* [in] */ DWORD reserved1,
            /* [in] */ DWORD reserved2,
            /* [out] */ IStorage __RPC_FAR *__RPC_FAR *ppstg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenStorage )( 
            IStorage __RPC_FAR * This,
            /* [string][unique][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ IStorage __RPC_FAR *pstgPriority,
            /* [in] */ DWORD grfMode,
            /* [unique][in] */ SNB snbExclude,
            /* [in] */ DWORD reserved,
            /* [out] */ IStorage __RPC_FAR *__RPC_FAR *ppstg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CopyTo )( 
            IStorage __RPC_FAR * This,
            /* [in] */ DWORD ciidExclude,
            /* [size_is][unique][in] */ const IID __RPC_FAR *rgiidExclude,
            /* [unique][in] */ SNB snbExclude,
            /* [unique][in] */ IStorage __RPC_FAR *pstgDest);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveElementTo )( 
            IStorage __RPC_FAR * This,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ IStorage __RPC_FAR *pstgDest,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsNewName,
            /* [in] */ DWORD grfFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Commit )( 
            IStorage __RPC_FAR * This,
            /* [in] */ DWORD grfCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Revert )( 
            IStorage __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumElements )( 
            IStorage __RPC_FAR * This,
            /* [in] */ DWORD reserved1,
            /* [size_is][unique][in] */ void __RPC_FAR *reserved2,
            /* [in] */ DWORD reserved3,
            /* [out] */ IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DestroyElement )( 
            IStorage __RPC_FAR * This,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RenameElement )( 
            IStorage __RPC_FAR * This,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsOldName,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsNewName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetElementTimes )( 
            IStorage __RPC_FAR * This,
            /* [string][unique][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ const FILETIME __RPC_FAR *pctime,
            /* [unique][in] */ const FILETIME __RPC_FAR *patime,
            /* [unique][in] */ const FILETIME __RPC_FAR *pmtime);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetClass )( 
            IStorage __RPC_FAR * This,
            /* [in] */ REFCLSID clsid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetStateBits )( 
            IStorage __RPC_FAR * This,
            /* [in] */ DWORD grfStateBits,
            /* [in] */ DWORD grfMask);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Stat )( 
            IStorage __RPC_FAR * This,
            /* [out] */ STATSTG __RPC_FAR *pstatstg,
            /* [in] */ DWORD grfStatFlag);
        
        END_INTERFACE
    } IStorageVtbl;

    interface IStorage
    {
        CONST_VTBL struct IStorageVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStorage_CreateStream(This,pwcsName,grfMode,reserved1,reserved2,ppstm)	\
    (This)->lpVtbl -> CreateStream(This,pwcsName,grfMode,reserved1,reserved2,ppstm)

#define IStorage_OpenStream(This,pwcsName,reserved1,grfMode,reserved2,ppstm)	\
    (This)->lpVtbl -> OpenStream(This,pwcsName,reserved1,grfMode,reserved2,ppstm)

#define IStorage_CreateStorage(This,pwcsName,grfMode,reserved1,reserved2,ppstg)	\
    (This)->lpVtbl -> CreateStorage(This,pwcsName,grfMode,reserved1,reserved2,ppstg)

#define IStorage_OpenStorage(This,pwcsName,pstgPriority,grfMode,snbExclude,reserved,ppstg)	\
    (This)->lpVtbl -> OpenStorage(This,pwcsName,pstgPriority,grfMode,snbExclude,reserved,ppstg)

#define IStorage_CopyTo(This,ciidExclude,rgiidExclude,snbExclude,pstgDest)	\
    (This)->lpVtbl -> CopyTo(This,ciidExclude,rgiidExclude,snbExclude,pstgDest)

#define IStorage_MoveElementTo(This,pwcsName,pstgDest,pwcsNewName,grfFlags)	\
    (This)->lpVtbl -> MoveElementTo(This,pwcsName,pstgDest,pwcsNewName,grfFlags)

#define IStorage_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define IStorage_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define IStorage_EnumElements(This,reserved1,reserved2,reserved3,ppenum)	\
    (This)->lpVtbl -> EnumElements(This,reserved1,reserved2,reserved3,ppenum)

#define IStorage_DestroyElement(This,pwcsName)	\
    (This)->lpVtbl -> DestroyElement(This,pwcsName)

#define IStorage_RenameElement(This,pwcsOldName,pwcsNewName)	\
    (This)->lpVtbl -> RenameElement(This,pwcsOldName,pwcsNewName)

#define IStorage_SetElementTimes(This,pwcsName,pctime,patime,pmtime)	\
    (This)->lpVtbl -> SetElementTimes(This,pwcsName,pctime,patime,pmtime)

#define IStorage_SetClass(This,clsid)	\
    (This)->lpVtbl -> SetClass(This,clsid)

#define IStorage_SetStateBits(This,grfStateBits,grfMask)	\
    (This)->lpVtbl -> SetStateBits(This,grfStateBits,grfMask)

#define IStorage_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IStorage_CreateStream_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
    /* [in] */ DWORD grfMode,
    /* [in] */ DWORD reserved1,
    /* [in] */ DWORD reserved2,
    /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);


void __RPC_STUB IStorage_CreateStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStorage_RemoteOpenStream_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
    /* [in] */ unsigned long cbReserved1,
    /* [size_is][unique][in] */ byte __RPC_FAR *reserved1,
    /* [in] */ DWORD grfMode,
    /* [in] */ DWORD reserved2,
    /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);


void __RPC_STUB IStorage_RemoteOpenStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_CreateStorage_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
    /* [in] */ DWORD grfMode,
    /* [in] */ DWORD reserved1,
    /* [in] */ DWORD reserved2,
    /* [out] */ IStorage __RPC_FAR *__RPC_FAR *ppstg);


void __RPC_STUB IStorage_CreateStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_OpenStorage_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][unique][in] */ const OLECHAR __RPC_FAR *pwcsName,
    /* [unique][in] */ IStorage __RPC_FAR *pstgPriority,
    /* [in] */ DWORD grfMode,
    /* [unique][in] */ SNB snbExclude,
    /* [in] */ DWORD reserved,
    /* [out] */ IStorage __RPC_FAR *__RPC_FAR *ppstg);


void __RPC_STUB IStorage_OpenStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_CopyTo_Proxy( 
    IStorage __RPC_FAR * This,
    /* [in] */ DWORD ciidExclude,
    /* [size_is][unique][in] */ const IID __RPC_FAR *rgiidExclude,
    /* [unique][in] */ SNB snbExclude,
    /* [unique][in] */ IStorage __RPC_FAR *pstgDest);


void __RPC_STUB IStorage_CopyTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_MoveElementTo_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
    /* [unique][in] */ IStorage __RPC_FAR *pstgDest,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsNewName,
    /* [in] */ DWORD grfFlags);


void __RPC_STUB IStorage_MoveElementTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_Commit_Proxy( 
    IStorage __RPC_FAR * This,
    /* [in] */ DWORD grfCommitFlags);


void __RPC_STUB IStorage_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_Revert_Proxy( 
    IStorage __RPC_FAR * This);


void __RPC_STUB IStorage_Revert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStorage_RemoteEnumElements_Proxy( 
    IStorage __RPC_FAR * This,
    /* [in] */ DWORD reserved1,
    /* [in] */ unsigned long cbReserved2,
    /* [size_is][unique][in] */ byte __RPC_FAR *reserved2,
    /* [in] */ DWORD reserved3,
    /* [out] */ IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IStorage_RemoteEnumElements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_DestroyElement_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName);


void __RPC_STUB IStorage_DestroyElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_RenameElement_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsOldName,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsNewName);


void __RPC_STUB IStorage_RenameElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_SetElementTimes_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][unique][in] */ const OLECHAR __RPC_FAR *pwcsName,
    /* [unique][in] */ const FILETIME __RPC_FAR *pctime,
    /* [unique][in] */ const FILETIME __RPC_FAR *patime,
    /* [unique][in] */ const FILETIME __RPC_FAR *pmtime);


void __RPC_STUB IStorage_SetElementTimes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_SetClass_Proxy( 
    IStorage __RPC_FAR * This,
    /* [in] */ REFCLSID clsid);


void __RPC_STUB IStorage_SetClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_SetStateBits_Proxy( 
    IStorage __RPC_FAR * This,
    /* [in] */ DWORD grfStateBits,
    /* [in] */ DWORD grfMask);


void __RPC_STUB IStorage_SetStateBits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_Stat_Proxy( 
    IStorage __RPC_FAR * This,
    /* [out] */ STATSTG __RPC_FAR *pstatstg,
    /* [in] */ DWORD grfStatFlag);


void __RPC_STUB IStorage_Stat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IStorage_INTERFACE_DEFINED__ */


#ifndef __IPersistFile_INTERFACE_DEFINED__
#define __IPersistFile_INTERFACE_DEFINED__

/* interface IPersistFile */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IPersistFile __RPC_FAR *LPPERSISTFILE;


EXTERN_C const IID IID_IPersistFile;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000010b-0000-0000-C000-000000000046")
    IPersistFile : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsDirty( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Load( 
            /* [in] */ LPCOLESTR pszFileName,
            /* [in] */ DWORD dwMode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Save( 
            /* [unique][in] */ LPCOLESTR pszFileName,
            /* [in] */ BOOL fRemember) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveCompleted( 
            /* [unique][in] */ LPCOLESTR pszFileName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurFile( 
            /* [out] */ LPOLESTR __RPC_FAR *ppszFileName) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPersistFileVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPersistFile __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPersistFile __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPersistFile __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClassID )( 
            IPersistFile __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsDirty )( 
            IPersistFile __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Load )( 
            IPersistFile __RPC_FAR * This,
            /* [in] */ LPCOLESTR pszFileName,
            /* [in] */ DWORD dwMode);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Save )( 
            IPersistFile __RPC_FAR * This,
            /* [unique][in] */ LPCOLESTR pszFileName,
            /* [in] */ BOOL fRemember);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SaveCompleted )( 
            IPersistFile __RPC_FAR * This,
            /* [unique][in] */ LPCOLESTR pszFileName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCurFile )( 
            IPersistFile __RPC_FAR * This,
            /* [out] */ LPOLESTR __RPC_FAR *ppszFileName);
        
        END_INTERFACE
    } IPersistFileVtbl;

    interface IPersistFile
    {
        CONST_VTBL struct IPersistFileVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistFile_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistFile_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistFile_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistFile_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistFile_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IPersistFile_Load(This,pszFileName,dwMode)	\
    (This)->lpVtbl -> Load(This,pszFileName,dwMode)

#define IPersistFile_Save(This,pszFileName,fRemember)	\
    (This)->lpVtbl -> Save(This,pszFileName,fRemember)

#define IPersistFile_SaveCompleted(This,pszFileName)	\
    (This)->lpVtbl -> SaveCompleted(This,pszFileName)

#define IPersistFile_GetCurFile(This,ppszFileName)	\
    (This)->lpVtbl -> GetCurFile(This,ppszFileName)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPersistFile_IsDirty_Proxy( 
    IPersistFile __RPC_FAR * This);


void __RPC_STUB IPersistFile_IsDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistFile_Load_Proxy( 
    IPersistFile __RPC_FAR * This,
    /* [in] */ LPCOLESTR pszFileName,
    /* [in] */ DWORD dwMode);


void __RPC_STUB IPersistFile_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistFile_Save_Proxy( 
    IPersistFile __RPC_FAR * This,
    /* [unique][in] */ LPCOLESTR pszFileName,
    /* [in] */ BOOL fRemember);


void __RPC_STUB IPersistFile_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistFile_SaveCompleted_Proxy( 
    IPersistFile __RPC_FAR * This,
    /* [unique][in] */ LPCOLESTR pszFileName);


void __RPC_STUB IPersistFile_SaveCompleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistFile_GetCurFile_Proxy( 
    IPersistFile __RPC_FAR * This,
    /* [out] */ LPOLESTR __RPC_FAR *ppszFileName);


void __RPC_STUB IPersistFile_GetCurFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPersistFile_INTERFACE_DEFINED__ */


#ifndef __IPersistStorage_INTERFACE_DEFINED__
#define __IPersistStorage_INTERFACE_DEFINED__

/* interface IPersistStorage */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IPersistStorage __RPC_FAR *LPPERSISTSTORAGE;


EXTERN_C const IID IID_IPersistStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000010a-0000-0000-C000-000000000046")
    IPersistStorage : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsDirty( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitNew( 
            /* [unique][in] */ IStorage __RPC_FAR *pStg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Load( 
            /* [unique][in] */ IStorage __RPC_FAR *pStg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Save( 
            /* [unique][in] */ IStorage __RPC_FAR *pStgSave,
            /* [in] */ BOOL fSameAsLoad) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveCompleted( 
            /* [unique][in] */ IStorage __RPC_FAR *pStgNew) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandsOffStorage( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPersistStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPersistStorage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPersistStorage __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPersistStorage __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClassID )( 
            IPersistStorage __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsDirty )( 
            IPersistStorage __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InitNew )( 
            IPersistStorage __RPC_FAR * This,
            /* [unique][in] */ IStorage __RPC_FAR *pStg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Load )( 
            IPersistStorage __RPC_FAR * This,
            /* [unique][in] */ IStorage __RPC_FAR *pStg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Save )( 
            IPersistStorage __RPC_FAR * This,
            /* [unique][in] */ IStorage __RPC_FAR *pStgSave,
            /* [in] */ BOOL fSameAsLoad);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SaveCompleted )( 
            IPersistStorage __RPC_FAR * This,
            /* [unique][in] */ IStorage __RPC_FAR *pStgNew);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HandsOffStorage )( 
            IPersistStorage __RPC_FAR * This);
        
        END_INTERFACE
    } IPersistStorageVtbl;

    interface IPersistStorage
    {
        CONST_VTBL struct IPersistStorageVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistStorage_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistStorage_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IPersistStorage_InitNew(This,pStg)	\
    (This)->lpVtbl -> InitNew(This,pStg)

#define IPersistStorage_Load(This,pStg)	\
    (This)->lpVtbl -> Load(This,pStg)

#define IPersistStorage_Save(This,pStgSave,fSameAsLoad)	\
    (This)->lpVtbl -> Save(This,pStgSave,fSameAsLoad)

#define IPersistStorage_SaveCompleted(This,pStgNew)	\
    (This)->lpVtbl -> SaveCompleted(This,pStgNew)

#define IPersistStorage_HandsOffStorage(This)	\
    (This)->lpVtbl -> HandsOffStorage(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPersistStorage_IsDirty_Proxy( 
    IPersistStorage __RPC_FAR * This);


void __RPC_STUB IPersistStorage_IsDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStorage_InitNew_Proxy( 
    IPersistStorage __RPC_FAR * This,
    /* [unique][in] */ IStorage __RPC_FAR *pStg);


void __RPC_STUB IPersistStorage_InitNew_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStorage_Load_Proxy( 
    IPersistStorage __RPC_FAR * This,
    /* [unique][in] */ IStorage __RPC_FAR *pStg);


void __RPC_STUB IPersistStorage_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStorage_Save_Proxy( 
    IPersistStorage __RPC_FAR * This,
    /* [unique][in] */ IStorage __RPC_FAR *pStgSave,
    /* [in] */ BOOL fSameAsLoad);


void __RPC_STUB IPersistStorage_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStorage_SaveCompleted_Proxy( 
    IPersistStorage __RPC_FAR * This,
    /* [unique][in] */ IStorage __RPC_FAR *pStgNew);


void __RPC_STUB IPersistStorage_SaveCompleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStorage_HandsOffStorage_Proxy( 
    IPersistStorage __RPC_FAR * This);


void __RPC_STUB IPersistStorage_HandsOffStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPersistStorage_INTERFACE_DEFINED__ */


#ifndef __ILockBytes_INTERFACE_DEFINED__
#define __ILockBytes_INTERFACE_DEFINED__

/* interface ILockBytes */
/* [unique][uuid][object] */ 

typedef /* [unique] */ ILockBytes __RPC_FAR *LPLOCKBYTES;


EXTERN_C const IID IID_ILockBytes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000000a-0000-0000-C000-000000000046")
    ILockBytes : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE ReadAt( 
            /* [in] */ ULARGE_INTEGER ulOffset,
            /* [length_is][size_is][out] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE WriteAt( 
            /* [in] */ ULARGE_INTEGER ulOffset,
            /* [size_is][in] */ const void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Flush( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSize( 
            /* [in] */ ULARGE_INTEGER cb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LockRegion( 
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnlockRegion( 
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stat( 
            /* [out] */ STATSTG __RPC_FAR *pstatstg,
            /* [in] */ DWORD grfStatFlag) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ILockBytesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ILockBytes __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ILockBytes __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ILockBytes __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReadAt )( 
            ILockBytes __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER ulOffset,
            /* [length_is][size_is][out] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WriteAt )( 
            ILockBytes __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER ulOffset,
            /* [size_is][in] */ const void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Flush )( 
            ILockBytes __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSize )( 
            ILockBytes __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER cb);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LockRegion )( 
            ILockBytes __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UnlockRegion )( 
            ILockBytes __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Stat )( 
            ILockBytes __RPC_FAR * This,
            /* [out] */ STATSTG __RPC_FAR *pstatstg,
            /* [in] */ DWORD grfStatFlag);
        
        END_INTERFACE
    } ILockBytesVtbl;

    interface ILockBytes
    {
        CONST_VTBL struct ILockBytesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILockBytes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILockBytes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILockBytes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILockBytes_ReadAt(This,ulOffset,pv,cb,pcbRead)	\
    (This)->lpVtbl -> ReadAt(This,ulOffset,pv,cb,pcbRead)

#define ILockBytes_WriteAt(This,ulOffset,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> WriteAt(This,ulOffset,pv,cb,pcbWritten)

#define ILockBytes_Flush(This)	\
    (This)->lpVtbl -> Flush(This)

#define ILockBytes_SetSize(This,cb)	\
    (This)->lpVtbl -> SetSize(This,cb)

#define ILockBytes_LockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> LockRegion(This,libOffset,cb,dwLockType)

#define ILockBytes_UnlockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> UnlockRegion(This,libOffset,cb,dwLockType)

#define ILockBytes_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall ILockBytes_RemoteReadAt_Proxy( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER ulOffset,
    /* [length_is][size_is][out] */ byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbRead);


void __RPC_STUB ILockBytes_RemoteReadAt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE ILockBytes_RemoteWriteAt_Proxy( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER ulOffset,
    /* [size_is][in] */ const byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);


void __RPC_STUB ILockBytes_RemoteWriteAt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ILockBytes_Flush_Proxy( 
    ILockBytes __RPC_FAR * This);


void __RPC_STUB ILockBytes_Flush_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ILockBytes_SetSize_Proxy( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER cb);


void __RPC_STUB ILockBytes_SetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ILockBytes_LockRegion_Proxy( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER libOffset,
    /* [in] */ ULARGE_INTEGER cb,
    /* [in] */ DWORD dwLockType);


void __RPC_STUB ILockBytes_LockRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ILockBytes_UnlockRegion_Proxy( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER libOffset,
    /* [in] */ ULARGE_INTEGER cb,
    /* [in] */ DWORD dwLockType);


void __RPC_STUB ILockBytes_UnlockRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ILockBytes_Stat_Proxy( 
    ILockBytes __RPC_FAR * This,
    /* [out] */ STATSTG __RPC_FAR *pstatstg,
    /* [in] */ DWORD grfStatFlag);


void __RPC_STUB ILockBytes_Stat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ILockBytes_INTERFACE_DEFINED__ */


#ifndef __IEnumFORMATETC_INTERFACE_DEFINED__
#define __IEnumFORMATETC_INTERFACE_DEFINED__

/* interface IEnumFORMATETC */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IEnumFORMATETC __RPC_FAR *LPENUMFORMATETC;

typedef struct tagDVTARGETDEVICE
    {
    DWORD tdSize;
    WORD tdDriverNameOffset;
    WORD tdDeviceNameOffset;
    WORD tdPortNameOffset;
    WORD tdExtDevmodeOffset;
    /* [size_is] */ BYTE tdData[ 1 ];
    }	DVTARGETDEVICE;

typedef CLIPFORMAT __RPC_FAR *LPCLIPFORMAT;

typedef struct tagFORMATETC
    {
    CLIPFORMAT cfFormat;
    /* [unique] */ DVTARGETDEVICE __RPC_FAR *ptd;
    DWORD dwAspect;
    LONG lindex;
    DWORD tymed;
    }	FORMATETC;

typedef struct tagFORMATETC __RPC_FAR *LPFORMATETC;


EXTERN_C const IID IID_IEnumFORMATETC;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000103-0000-0000-C000-000000000046")
    IEnumFORMATETC : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ FORMATETC __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumFORMATETCVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumFORMATETC __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumFORMATETC __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumFORMATETC __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumFORMATETC __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ FORMATETC __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumFORMATETC __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumFORMATETC __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumFORMATETC __RPC_FAR * This,
            /* [out] */ IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenum);
        
        END_INTERFACE
    } IEnumFORMATETCVtbl;

    interface IEnumFORMATETC
    {
        CONST_VTBL struct IEnumFORMATETCVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumFORMATETC_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumFORMATETC_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumFORMATETC_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumFORMATETC_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumFORMATETC_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumFORMATETC_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumFORMATETC_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumFORMATETC_RemoteNext_Proxy( 
    IEnumFORMATETC __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ FORMATETC __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumFORMATETC_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumFORMATETC_Skip_Proxy( 
    IEnumFORMATETC __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumFORMATETC_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumFORMATETC_Reset_Proxy( 
    IEnumFORMATETC __RPC_FAR * This);


void __RPC_STUB IEnumFORMATETC_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumFORMATETC_Clone_Proxy( 
    IEnumFORMATETC __RPC_FAR * This,
    /* [out] */ IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumFORMATETC_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumFORMATETC_INTERFACE_DEFINED__ */


#ifndef __IEnumSTATDATA_INTERFACE_DEFINED__
#define __IEnumSTATDATA_INTERFACE_DEFINED__

/* interface IEnumSTATDATA */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IEnumSTATDATA __RPC_FAR *LPENUMSTATDATA;

typedef 
enum tagADVF
    {	ADVF_NODATA	= 1,
	ADVF_PRIMEFIRST	= 2,
	ADVF_ONLYONCE	= 4,
	ADVF_DATAONSTOP	= 64,
	ADVFCACHE_NOHANDLER	= 8,
	ADVFCACHE_FORCEBUILTIN	= 16,
	ADVFCACHE_ONSAVE	= 32
    }	ADVF;

typedef struct tagSTATDATA
    {
    FORMATETC formatetc;
    DWORD advf;
    /* [unique] */ IAdviseSink __RPC_FAR *pAdvSink;
    DWORD dwConnection;
    }	STATDATA;

typedef STATDATA __RPC_FAR *LPSTATDATA;


EXTERN_C const IID IID_IEnumSTATDATA;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000105-0000-0000-C000-000000000046")
    IEnumSTATDATA : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ STATDATA __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumSTATDATAVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumSTATDATA __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumSTATDATA __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumSTATDATA __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumSTATDATA __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ STATDATA __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumSTATDATA __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumSTATDATA __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumSTATDATA __RPC_FAR * This,
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenum);
        
        END_INTERFACE
    } IEnumSTATDATAVtbl;

    interface IEnumSTATDATA
    {
        CONST_VTBL struct IEnumSTATDATAVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSTATDATA_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSTATDATA_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSTATDATA_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSTATDATA_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumSTATDATA_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSTATDATA_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSTATDATA_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumSTATDATA_RemoteNext_Proxy( 
    IEnumSTATDATA __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATDATA __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumSTATDATA_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATDATA_Skip_Proxy( 
    IEnumSTATDATA __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumSTATDATA_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATDATA_Reset_Proxy( 
    IEnumSTATDATA __RPC_FAR * This);


void __RPC_STUB IEnumSTATDATA_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATDATA_Clone_Proxy( 
    IEnumSTATDATA __RPC_FAR * This,
    /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumSTATDATA_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumSTATDATA_INTERFACE_DEFINED__ */


#ifndef __IRootStorage_INTERFACE_DEFINED__
#define __IRootStorage_INTERFACE_DEFINED__

/* interface IRootStorage */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IRootStorage __RPC_FAR *LPROOTSTORAGE;


EXTERN_C const IID IID_IRootStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000012-0000-0000-C000-000000000046")
    IRootStorage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SwitchToFile( 
            /* [in] */ LPOLESTR pszFile) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRootStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRootStorage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRootStorage __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRootStorage __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SwitchToFile )( 
            IRootStorage __RPC_FAR * This,
            /* [in] */ LPOLESTR pszFile);
        
        END_INTERFACE
    } IRootStorageVtbl;

    interface IRootStorage
    {
        CONST_VTBL struct IRootStorageVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRootStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRootStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRootStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRootStorage_SwitchToFile(This,pszFile)	\
    (This)->lpVtbl -> SwitchToFile(This,pszFile)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRootStorage_SwitchToFile_Proxy( 
    IRootStorage __RPC_FAR * This,
    /* [in] */ LPOLESTR pszFile);


void __RPC_STUB IRootStorage_SwitchToFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRootStorage_INTERFACE_DEFINED__ */


#ifndef __IAdviseSink_INTERFACE_DEFINED__
#define __IAdviseSink_INTERFACE_DEFINED__

/* interface IAdviseSink */
/* [unique][async_uuid][uuid][object] */ 

typedef IAdviseSink __RPC_FAR *LPADVISESINK;

typedef /* [v1_enum] */ 
enum tagTYMED
    {	TYMED_HGLOBAL	= 1,
	TYMED_FILE	= 2,
	TYMED_ISTREAM	= 4,
	TYMED_ISTORAGE	= 8,
	TYMED_GDI	= 16,
	TYMED_MFPICT	= 32,
	TYMED_ENHMF	= 64,
	TYMED_NULL	= 0
    }	TYMED;

#ifndef RC_INVOKED
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4200)
#endif
typedef struct tagRemSTGMEDIUM
    {
    DWORD tymed;
    DWORD dwHandleType;
    unsigned long pData;
    unsigned long pUnkForRelease;
    unsigned long cbData;
    /* [size_is] */ byte data[ 1 ];
    }	RemSTGMEDIUM;

#ifndef RC_INVOKED
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4200)
#endif
#endif
#ifdef NONAMELESSUNION
typedef struct tagSTGMEDIUM {
    DWORD tymed;
    union {
        HBITMAP hBitmap;
        HMETAFILEPICT hMetaFilePict;
        HENHMETAFILE hEnhMetaFile;
        HGLOBAL hGlobal;
        LPOLESTR lpszFileName;
        IStream *pstm;
        IStorage *pstg;
        } u;
    IUnknown *pUnkForRelease;
}uSTGMEDIUM;
#else
typedef struct tagSTGMEDIUM
    {
    DWORD tymed;
    /* [switch_is][switch_type] */ union 
        {
        /* [case()] */ HBITMAP hBitmap;
        /* [case()] */ HMETAFILEPICT hMetaFilePict;
        /* [case()] */ HENHMETAFILE hEnhMetaFile;
        /* [case()] */ HGLOBAL hGlobal;
        /* [case()] */ LPOLESTR lpszFileName;
        /* [case()] */ IStream __RPC_FAR *pstm;
        /* [case()] */ IStorage __RPC_FAR *pstg;
        /* [default] */  /* Empty union arm */ 
        }	;
    /* [unique] */ IUnknown __RPC_FAR *pUnkForRelease;
    }	uSTGMEDIUM;

#endif /* !NONAMELESSUNION */
typedef struct _GDI_OBJECT
    {
    DWORD ObjectType;
    /* [switch_is] */ /* [switch_type] */ union __MIDL_IAdviseSink_0002
        {
        /* [case()] */ wireHBITMAP hBitmap;
        /* [case()] */ wireHPALETTE hPalette;
        /* [default] */ wireHGLOBAL hGeneric;
        }	u;
    }	GDI_OBJECT;

typedef struct _userSTGMEDIUM
    {
    struct _STGMEDIUM_UNION
        {
        DWORD tymed;
        /* [switch_is] */ /* [switch_type] */ union __MIDL_IAdviseSink_0003
            {
            /* [case()] */  /* Empty union arm */ 
            /* [case()] */ wireHMETAFILEPICT hMetaFilePict;
            /* [case()] */ wireHENHMETAFILE hHEnhMetaFile;
            /* [case()] */ GDI_OBJECT __RPC_FAR *hGdiHandle;
            /* [case()] */ wireHGLOBAL hGlobal;
            /* [case()] */ LPOLESTR lpszFileName;
            /* [case()] */ BYTE_BLOB __RPC_FAR *pstm;
            /* [case()] */ BYTE_BLOB __RPC_FAR *pstg;
            }	u;
        }	;
    IUnknown __RPC_FAR *pUnkForRelease;
    }	userSTGMEDIUM;

typedef /* [unique] */ userSTGMEDIUM __RPC_FAR *wireSTGMEDIUM;

typedef /* [wire_marshal] */ uSTGMEDIUM STGMEDIUM;

typedef /* [unique] */ userSTGMEDIUM __RPC_FAR *wireASYNC_STGMEDIUM;

typedef /* [wire_marshal] */ STGMEDIUM ASYNC_STGMEDIUM;

typedef STGMEDIUM __RPC_FAR *LPSTGMEDIUM;

typedef struct _userFLAG_STGMEDIUM
    {
    long ContextFlags;
    long fPassOwnership;
    userSTGMEDIUM Stgmed;
    }	userFLAG_STGMEDIUM;

typedef /* [unique] */ userFLAG_STGMEDIUM __RPC_FAR *wireFLAG_STGMEDIUM;

typedef /* [wire_marshal] */ struct _FLAG_STGMEDIUM
    {
    long ContextFlags;
    long fPassOwnership;
    STGMEDIUM Stgmed;
    }	FLAG_STGMEDIUM;


EXTERN_C const IID IID_IAdviseSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000010f-0000-0000-C000-000000000046")
    IAdviseSink : public IUnknown
    {
    public:
        virtual /* [local] */ void STDMETHODCALLTYPE OnDataChange( 
            /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
            /* [unique][in] */ STGMEDIUM __RPC_FAR *pStgmed) = 0;
        
        virtual /* [local] */ void STDMETHODCALLTYPE OnViewChange( 
            /* [in] */ DWORD dwAspect,
            /* [in] */ LONG lindex) = 0;
        
        virtual /* [local] */ void STDMETHODCALLTYPE OnRename( 
            /* [in] */ IMoniker __RPC_FAR *pmk) = 0;
        
        virtual /* [local] */ void STDMETHODCALLTYPE OnSave( void) = 0;
        
        virtual /* [local] */ void STDMETHODCALLTYPE OnClose( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAdviseSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAdviseSink __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAdviseSink __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAdviseSink __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *OnDataChange )( 
            IAdviseSink __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
            /* [unique][in] */ STGMEDIUM __RPC_FAR *pStgmed);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *OnViewChange )( 
            IAdviseSink __RPC_FAR * This,
            /* [in] */ DWORD dwAspect,
            /* [in] */ LONG lindex);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *OnRename )( 
            IAdviseSink __RPC_FAR * This,
            /* [in] */ IMoniker __RPC_FAR *pmk);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *OnSave )( 
            IAdviseSink __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *OnClose )( 
            IAdviseSink __RPC_FAR * This);
        
        END_INTERFACE
    } IAdviseSinkVtbl;

    interface IAdviseSink
    {
        CONST_VTBL struct IAdviseSinkVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAdviseSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAdviseSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAdviseSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAdviseSink_OnDataChange(This,pFormatetc,pStgmed)	\
    (This)->lpVtbl -> OnDataChange(This,pFormatetc,pStgmed)

#define IAdviseSink_OnViewChange(This,dwAspect,lindex)	\
    (This)->lpVtbl -> OnViewChange(This,dwAspect,lindex)

#define IAdviseSink_OnRename(This,pmk)	\
    (This)->lpVtbl -> OnRename(This,pmk)

#define IAdviseSink_OnSave(This)	\
    (This)->lpVtbl -> OnSave(This)

#define IAdviseSink_OnClose(This)	\
    (This)->lpVtbl -> OnClose(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IAdviseSink_RemoteOnDataChange_Proxy( 
    IAdviseSink __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
    /* [unique][in] */ ASYNC_STGMEDIUM __RPC_FAR *pStgmed);


void __RPC_STUB IAdviseSink_RemoteOnDataChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IAdviseSink_RemoteOnViewChange_Proxy( 
    IAdviseSink __RPC_FAR * This,
    /* [in] */ DWORD dwAspect,
    /* [in] */ LONG lindex);


void __RPC_STUB IAdviseSink_RemoteOnViewChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IAdviseSink_RemoteOnRename_Proxy( 
    IAdviseSink __RPC_FAR * This,
    /* [in] */ IMoniker __RPC_FAR *pmk);


void __RPC_STUB IAdviseSink_RemoteOnRename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IAdviseSink_RemoteOnSave_Proxy( 
    IAdviseSink __RPC_FAR * This);


void __RPC_STUB IAdviseSink_RemoteOnSave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IAdviseSink_RemoteOnClose_Proxy( 
    IAdviseSink __RPC_FAR * This);


void __RPC_STUB IAdviseSink_RemoteOnClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAdviseSink_INTERFACE_DEFINED__ */


#ifndef __AsyncIAdviseSink_INTERFACE_DEFINED__
#define __AsyncIAdviseSink_INTERFACE_DEFINED__

/* interface AsyncIAdviseSink */
/* [uuid][unique][object] */ 


EXTERN_C const IID IID_AsyncIAdviseSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000150-0000-0000-C000-000000000046")
    AsyncIAdviseSink : public IUnknown
    {
    public:
        virtual /* [local] */ void STDMETHODCALLTYPE Begin_OnDataChange( 
            /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
            /* [unique][in] */ STGMEDIUM __RPC_FAR *pStgmed) = 0;
        
        virtual /* [local] */ void STDMETHODCALLTYPE Finish_OnDataChange( void) = 0;
        
        virtual /* [local] */ void STDMETHODCALLTYPE Begin_OnViewChange( 
            /* [in] */ DWORD dwAspect,
            /* [in] */ LONG lindex) = 0;
        
        virtual /* [local] */ void STDMETHODCALLTYPE Finish_OnViewChange( void) = 0;
        
        virtual /* [local] */ void STDMETHODCALLTYPE Begin_OnRename( 
            /* [in] */ IMoniker __RPC_FAR *pmk) = 0;
        
        virtual /* [local] */ void STDMETHODCALLTYPE Finish_OnRename( void) = 0;
        
        virtual /* [local] */ void STDMETHODCALLTYPE Begin_OnSave( void) = 0;
        
        virtual /* [local] */ void STDMETHODCALLTYPE Finish_OnSave( void) = 0;
        
        virtual /* [local] */ void STDMETHODCALLTYPE Begin_OnClose( void) = 0;
        
        virtual /* [local] */ void STDMETHODCALLTYPE Finish_OnClose( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct AsyncIAdviseSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            AsyncIAdviseSink __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            AsyncIAdviseSink __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            AsyncIAdviseSink __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Begin_OnDataChange )( 
            AsyncIAdviseSink __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
            /* [unique][in] */ STGMEDIUM __RPC_FAR *pStgmed);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Finish_OnDataChange )( 
            AsyncIAdviseSink __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Begin_OnViewChange )( 
            AsyncIAdviseSink __RPC_FAR * This,
            /* [in] */ DWORD dwAspect,
            /* [in] */ LONG lindex);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Finish_OnViewChange )( 
            AsyncIAdviseSink __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Begin_OnRename )( 
            AsyncIAdviseSink __RPC_FAR * This,
            /* [in] */ IMoniker __RPC_FAR *pmk);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Finish_OnRename )( 
            AsyncIAdviseSink __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Begin_OnSave )( 
            AsyncIAdviseSink __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Finish_OnSave )( 
            AsyncIAdviseSink __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Begin_OnClose )( 
            AsyncIAdviseSink __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Finish_OnClose )( 
            AsyncIAdviseSink __RPC_FAR * This);
        
        END_INTERFACE
    } AsyncIAdviseSinkVtbl;

    interface AsyncIAdviseSink
    {
        CONST_VTBL struct AsyncIAdviseSinkVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define AsyncIAdviseSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define AsyncIAdviseSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define AsyncIAdviseSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define AsyncIAdviseSink_Begin_OnDataChange(This,pFormatetc,pStgmed)	\
    (This)->lpVtbl -> Begin_OnDataChange(This,pFormatetc,pStgmed)

#define AsyncIAdviseSink_Finish_OnDataChange(This)	\
    (This)->lpVtbl -> Finish_OnDataChange(This)

#define AsyncIAdviseSink_Begin_OnViewChange(This,dwAspect,lindex)	\
    (This)->lpVtbl -> Begin_OnViewChange(This,dwAspect,lindex)

#define AsyncIAdviseSink_Finish_OnViewChange(This)	\
    (This)->lpVtbl -> Finish_OnViewChange(This)

#define AsyncIAdviseSink_Begin_OnRename(This,pmk)	\
    (This)->lpVtbl -> Begin_OnRename(This,pmk)

#define AsyncIAdviseSink_Finish_OnRename(This)	\
    (This)->lpVtbl -> Finish_OnRename(This)

#define AsyncIAdviseSink_Begin_OnSave(This)	\
    (This)->lpVtbl -> Begin_OnSave(This)

#define AsyncIAdviseSink_Finish_OnSave(This)	\
    (This)->lpVtbl -> Finish_OnSave(This)

#define AsyncIAdviseSink_Begin_OnClose(This)	\
    (This)->lpVtbl -> Begin_OnClose(This)

#define AsyncIAdviseSink_Finish_OnClose(This)	\
    (This)->lpVtbl -> Finish_OnClose(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_RemoteOnDataChange_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
    /* [unique][in] */ ASYNC_STGMEDIUM __RPC_FAR *pStgmed);


void __RPC_STUB AsyncIAdviseSink_Begin_RemoteOnDataChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_RemoteOnDataChange_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This);


void __RPC_STUB AsyncIAdviseSink_Finish_RemoteOnDataChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_RemoteOnViewChange_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This,
    /* [in] */ DWORD dwAspect,
    /* [in] */ LONG lindex);


void __RPC_STUB AsyncIAdviseSink_Begin_RemoteOnViewChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_RemoteOnViewChange_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This);


void __RPC_STUB AsyncIAdviseSink_Finish_RemoteOnViewChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_RemoteOnRename_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This,
    /* [in] */ IMoniker __RPC_FAR *pmk);


void __RPC_STUB AsyncIAdviseSink_Begin_RemoteOnRename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_RemoteOnRename_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This);


void __RPC_STUB AsyncIAdviseSink_Finish_RemoteOnRename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_RemoteOnSave_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This);


void __RPC_STUB AsyncIAdviseSink_Begin_RemoteOnSave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_RemoteOnSave_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This);


void __RPC_STUB AsyncIAdviseSink_Finish_RemoteOnSave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_RemoteOnClose_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This);


void __RPC_STUB AsyncIAdviseSink_Begin_RemoteOnClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_RemoteOnClose_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This);


void __RPC_STUB AsyncIAdviseSink_Finish_RemoteOnClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __AsyncIAdviseSink_INTERFACE_DEFINED__ */


#ifndef __IAdviseSink2_INTERFACE_DEFINED__
#define __IAdviseSink2_INTERFACE_DEFINED__

/* interface IAdviseSink2 */
/* [unique][async_uuid][uuid][object] */ 

typedef /* [unique] */ IAdviseSink2 __RPC_FAR *LPADVISESINK2;


EXTERN_C const IID IID_IAdviseSink2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000125-0000-0000-C000-000000000046")
    IAdviseSink2 : public IAdviseSink
    {
    public:
        virtual /* [local] */ void STDMETHODCALLTYPE OnLinkSrcChange( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmk) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAdviseSink2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAdviseSink2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAdviseSink2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAdviseSink2 __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *OnDataChange )( 
            IAdviseSink2 __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
            /* [unique][in] */ STGMEDIUM __RPC_FAR *pStgmed);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *OnViewChange )( 
            IAdviseSink2 __RPC_FAR * This,
            /* [in] */ DWORD dwAspect,
            /* [in] */ LONG lindex);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *OnRename )( 
            IAdviseSink2 __RPC_FAR * This,
            /* [in] */ IMoniker __RPC_FAR *pmk);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *OnSave )( 
            IAdviseSink2 __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *OnClose )( 
            IAdviseSink2 __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *OnLinkSrcChange )( 
            IAdviseSink2 __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmk);
        
        END_INTERFACE
    } IAdviseSink2Vtbl;

    interface IAdviseSink2
    {
        CONST_VTBL struct IAdviseSink2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAdviseSink2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAdviseSink2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAdviseSink2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAdviseSink2_OnDataChange(This,pFormatetc,pStgmed)	\
    (This)->lpVtbl -> OnDataChange(This,pFormatetc,pStgmed)

#define IAdviseSink2_OnViewChange(This,dwAspect,lindex)	\
    (This)->lpVtbl -> OnViewChange(This,dwAspect,lindex)

#define IAdviseSink2_OnRename(This,pmk)	\
    (This)->lpVtbl -> OnRename(This,pmk)

#define IAdviseSink2_OnSave(This)	\
    (This)->lpVtbl -> OnSave(This)

#define IAdviseSink2_OnClose(This)	\
    (This)->lpVtbl -> OnClose(This)


#define IAdviseSink2_OnLinkSrcChange(This,pmk)	\
    (This)->lpVtbl -> OnLinkSrcChange(This,pmk)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IAdviseSink2_RemoteOnLinkSrcChange_Proxy( 
    IAdviseSink2 __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmk);


void __RPC_STUB IAdviseSink2_RemoteOnLinkSrcChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAdviseSink2_INTERFACE_DEFINED__ */


#ifndef __AsyncIAdviseSink2_INTERFACE_DEFINED__
#define __AsyncIAdviseSink2_INTERFACE_DEFINED__

/* interface AsyncIAdviseSink2 */
/* [uuid][unique][object] */ 


EXTERN_C const IID IID_AsyncIAdviseSink2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000151-0000-0000-C000-000000000046")
    AsyncIAdviseSink2 : public AsyncIAdviseSink
    {
    public:
        virtual /* [local] */ void STDMETHODCALLTYPE Begin_OnLinkSrcChange( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmk) = 0;
        
        virtual /* [local] */ void STDMETHODCALLTYPE Finish_OnLinkSrcChange( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct AsyncIAdviseSink2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            AsyncIAdviseSink2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            AsyncIAdviseSink2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            AsyncIAdviseSink2 __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Begin_OnDataChange )( 
            AsyncIAdviseSink2 __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
            /* [unique][in] */ STGMEDIUM __RPC_FAR *pStgmed);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Finish_OnDataChange )( 
            AsyncIAdviseSink2 __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Begin_OnViewChange )( 
            AsyncIAdviseSink2 __RPC_FAR * This,
            /* [in] */ DWORD dwAspect,
            /* [in] */ LONG lindex);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Finish_OnViewChange )( 
            AsyncIAdviseSink2 __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Begin_OnRename )( 
            AsyncIAdviseSink2 __RPC_FAR * This,
            /* [in] */ IMoniker __RPC_FAR *pmk);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Finish_OnRename )( 
            AsyncIAdviseSink2 __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Begin_OnSave )( 
            AsyncIAdviseSink2 __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Finish_OnSave )( 
            AsyncIAdviseSink2 __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Begin_OnClose )( 
            AsyncIAdviseSink2 __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Finish_OnClose )( 
            AsyncIAdviseSink2 __RPC_FAR * This);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Begin_OnLinkSrcChange )( 
            AsyncIAdviseSink2 __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmk);
        
        /* [local] */ void ( STDMETHODCALLTYPE __RPC_FAR *Finish_OnLinkSrcChange )( 
            AsyncIAdviseSink2 __RPC_FAR * This);
        
        END_INTERFACE
    } AsyncIAdviseSink2Vtbl;

    interface AsyncIAdviseSink2
    {
        CONST_VTBL struct AsyncIAdviseSink2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define AsyncIAdviseSink2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define AsyncIAdviseSink2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define AsyncIAdviseSink2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define AsyncIAdviseSink2_Begin_OnDataChange(This,pFormatetc,pStgmed)	\
    (This)->lpVtbl -> Begin_OnDataChange(This,pFormatetc,pStgmed)

#define AsyncIAdviseSink2_Finish_OnDataChange(This)	\
    (This)->lpVtbl -> Finish_OnDataChange(This)

#define AsyncIAdviseSink2_Begin_OnViewChange(This,dwAspect,lindex)	\
    (This)->lpVtbl -> Begin_OnViewChange(This,dwAspect,lindex)

#define AsyncIAdviseSink2_Finish_OnViewChange(This)	\
    (This)->lpVtbl -> Finish_OnViewChange(This)

#define AsyncIAdviseSink2_Begin_OnRename(This,pmk)	\
    (This)->lpVtbl -> Begin_OnRename(This,pmk)

#define AsyncIAdviseSink2_Finish_OnRename(This)	\
    (This)->lpVtbl -> Finish_OnRename(This)

#define AsyncIAdviseSink2_Begin_OnSave(This)	\
    (This)->lpVtbl -> Begin_OnSave(This)

#define AsyncIAdviseSink2_Finish_OnSave(This)	\
    (This)->lpVtbl -> Finish_OnSave(This)

#define AsyncIAdviseSink2_Begin_OnClose(This)	\
    (This)->lpVtbl -> Begin_OnClose(This)

#define AsyncIAdviseSink2_Finish_OnClose(This)	\
    (This)->lpVtbl -> Finish_OnClose(This)


#define AsyncIAdviseSink2_Begin_OnLinkSrcChange(This,pmk)	\
    (This)->lpVtbl -> Begin_OnLinkSrcChange(This,pmk)

#define AsyncIAdviseSink2_Finish_OnLinkSrcChange(This)	\
    (This)->lpVtbl -> Finish_OnLinkSrcChange(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink2_Begin_RemoteOnLinkSrcChange_Proxy( 
    AsyncIAdviseSink2 __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmk);


void __RPC_STUB AsyncIAdviseSink2_Begin_RemoteOnLinkSrcChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink2_Finish_RemoteOnLinkSrcChange_Proxy( 
    AsyncIAdviseSink2 __RPC_FAR * This);


void __RPC_STUB AsyncIAdviseSink2_Finish_RemoteOnLinkSrcChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __AsyncIAdviseSink2_INTERFACE_DEFINED__ */


#ifndef __IDataObject_INTERFACE_DEFINED__
#define __IDataObject_INTERFACE_DEFINED__

/* interface IDataObject */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IDataObject __RPC_FAR *LPDATAOBJECT;

typedef 
enum tagDATADIR
    {	DATADIR_GET	= 1,
	DATADIR_SET	= 2
    }	DATADIR;


EXTERN_C const IID IID_IDataObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000010e-0000-0000-C000-000000000046")
    IDataObject : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetData( 
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetcIn,
            /* [out] */ STGMEDIUM __RPC_FAR *pmedium) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetDataHere( 
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [out][in] */ STGMEDIUM __RPC_FAR *pmedium) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryGetData( 
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc( 
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatectIn,
            /* [out] */ FORMATETC __RPC_FAR *pformatetcOut) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE SetData( 
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [unique][in] */ STGMEDIUM __RPC_FAR *pmedium,
            /* [in] */ BOOL fRelease) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumFormatEtc( 
            /* [in] */ DWORD dwDirection,
            /* [out] */ IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenumFormatEtc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DAdvise( 
            /* [in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [in] */ DWORD advf,
            /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvSink,
            /* [out] */ DWORD __RPC_FAR *pdwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DUnadvise( 
            /* [in] */ DWORD dwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumDAdvise( 
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDataObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDataObject __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDataObject __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDataObject __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetData )( 
            IDataObject __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetcIn,
            /* [out] */ STGMEDIUM __RPC_FAR *pmedium);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDataHere )( 
            IDataObject __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [out][in] */ STGMEDIUM __RPC_FAR *pmedium);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryGetData )( 
            IDataObject __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCanonicalFormatEtc )( 
            IDataObject __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatectIn,
            /* [out] */ FORMATETC __RPC_FAR *pformatetcOut);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetData )( 
            IDataObject __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [unique][in] */ STGMEDIUM __RPC_FAR *pmedium,
            /* [in] */ BOOL fRelease);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumFormatEtc )( 
            IDataObject __RPC_FAR * This,
            /* [in] */ DWORD dwDirection,
            /* [out] */ IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenumFormatEtc);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DAdvise )( 
            IDataObject __RPC_FAR * This,
            /* [in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [in] */ DWORD advf,
            /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvSink,
            /* [out] */ DWORD __RPC_FAR *pdwConnection);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DUnadvise )( 
            IDataObject __RPC_FAR * This,
            /* [in] */ DWORD dwConnection);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumDAdvise )( 
            IDataObject __RPC_FAR * This,
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise);
        
        END_INTERFACE
    } IDataObjectVtbl;

    interface IDataObject
    {
        CONST_VTBL struct IDataObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDataObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDataObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDataObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDataObject_GetData(This,pformatetcIn,pmedium)	\
    (This)->lpVtbl -> GetData(This,pformatetcIn,pmedium)

#define IDataObject_GetDataHere(This,pformatetc,pmedium)	\
    (This)->lpVtbl -> GetDataHere(This,pformatetc,pmedium)

#define IDataObject_QueryGetData(This,pformatetc)	\
    (This)->lpVtbl -> QueryGetData(This,pformatetc)

#define IDataObject_GetCanonicalFormatEtc(This,pformatectIn,pformatetcOut)	\
    (This)->lpVtbl -> GetCanonicalFormatEtc(This,pformatectIn,pformatetcOut)

#define IDataObject_SetData(This,pformatetc,pmedium,fRelease)	\
    (This)->lpVtbl -> SetData(This,pformatetc,pmedium,fRelease)

#define IDataObject_EnumFormatEtc(This,dwDirection,ppenumFormatEtc)	\
    (This)->lpVtbl -> EnumFormatEtc(This,dwDirection,ppenumFormatEtc)

#define IDataObject_DAdvise(This,pformatetc,advf,pAdvSink,pdwConnection)	\
    (This)->lpVtbl -> DAdvise(This,pformatetc,advf,pAdvSink,pdwConnection)

#define IDataObject_DUnadvise(This,dwConnection)	\
    (This)->lpVtbl -> DUnadvise(This,dwConnection)

#define IDataObject_EnumDAdvise(This,ppenumAdvise)	\
    (This)->lpVtbl -> EnumDAdvise(This,ppenumAdvise)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IDataObject_RemoteGetData_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetcIn,
    /* [out] */ STGMEDIUM __RPC_FAR *pRemoteMedium);


void __RPC_STUB IDataObject_RemoteGetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IDataObject_RemoteGetDataHere_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [out][in] */ STGMEDIUM __RPC_FAR *pRemoteMedium);


void __RPC_STUB IDataObject_RemoteGetDataHere_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataObject_QueryGetData_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc);


void __RPC_STUB IDataObject_QueryGetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataObject_GetCanonicalFormatEtc_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatectIn,
    /* [out] */ FORMATETC __RPC_FAR *pformatetcOut);


void __RPC_STUB IDataObject_GetCanonicalFormatEtc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IDataObject_RemoteSetData_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [unique][in] */ FLAG_STGMEDIUM __RPC_FAR *pmedium,
    /* [in] */ BOOL fRelease);


void __RPC_STUB IDataObject_RemoteSetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataObject_EnumFormatEtc_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [in] */ DWORD dwDirection,
    /* [out] */ IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenumFormatEtc);


void __RPC_STUB IDataObject_EnumFormatEtc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataObject_DAdvise_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [in] */ DWORD advf,
    /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvSink,
    /* [out] */ DWORD __RPC_FAR *pdwConnection);


void __RPC_STUB IDataObject_DAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataObject_DUnadvise_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [in] */ DWORD dwConnection);


void __RPC_STUB IDataObject_DUnadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataObject_EnumDAdvise_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise);


void __RPC_STUB IDataObject_EnumDAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDataObject_INTERFACE_DEFINED__ */


#ifndef __IDataAdviseHolder_INTERFACE_DEFINED__
#define __IDataAdviseHolder_INTERFACE_DEFINED__

/* interface IDataAdviseHolder */
/* [uuid][object][local] */ 

typedef /* [unique] */ IDataAdviseHolder __RPC_FAR *LPDATAADVISEHOLDER;


EXTERN_C const IID IID_IDataAdviseHolder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000110-0000-0000-C000-000000000046")
    IDataAdviseHolder : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Advise( 
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObject,
            /* [unique][in] */ FORMATETC __RPC_FAR *pFetc,
            /* [in] */ DWORD advf,
            /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvise,
            /* [out] */ DWORD __RPC_FAR *pdwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unadvise( 
            /* [in] */ DWORD dwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumAdvise( 
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SendOnDataChange( 
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObject,
            /* [in] */ DWORD dwReserved,
            /* [in] */ DWORD advf) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDataAdviseHolderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDataAdviseHolder __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDataAdviseHolder __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDataAdviseHolder __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Advise )( 
            IDataAdviseHolder __RPC_FAR * This,
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObject,
            /* [unique][in] */ FORMATETC __RPC_FAR *pFetc,
            /* [in] */ DWORD advf,
            /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvise,
            /* [out] */ DWORD __RPC_FAR *pdwConnection);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Unadvise )( 
            IDataAdviseHolder __RPC_FAR * This,
            /* [in] */ DWORD dwConnection);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumAdvise )( 
            IDataAdviseHolder __RPC_FAR * This,
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendOnDataChange )( 
            IDataAdviseHolder __RPC_FAR * This,
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObject,
            /* [in] */ DWORD dwReserved,
            /* [in] */ DWORD advf);
        
        END_INTERFACE
    } IDataAdviseHolderVtbl;

    interface IDataAdviseHolder
    {
        CONST_VTBL struct IDataAdviseHolderVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDataAdviseHolder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDataAdviseHolder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDataAdviseHolder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDataAdviseHolder_Advise(This,pDataObject,pFetc,advf,pAdvise,pdwConnection)	\
    (This)->lpVtbl -> Advise(This,pDataObject,pFetc,advf,pAdvise,pdwConnection)

#define IDataAdviseHolder_Unadvise(This,dwConnection)	\
    (This)->lpVtbl -> Unadvise(This,dwConnection)

#define IDataAdviseHolder_EnumAdvise(This,ppenumAdvise)	\
    (This)->lpVtbl -> EnumAdvise(This,ppenumAdvise)

#define IDataAdviseHolder_SendOnDataChange(This,pDataObject,dwReserved,advf)	\
    (This)->lpVtbl -> SendOnDataChange(This,pDataObject,dwReserved,advf)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDataAdviseHolder_Advise_Proxy( 
    IDataAdviseHolder __RPC_FAR * This,
    /* [unique][in] */ IDataObject __RPC_FAR *pDataObject,
    /* [unique][in] */ FORMATETC __RPC_FAR *pFetc,
    /* [in] */ DWORD advf,
    /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvise,
    /* [out] */ DWORD __RPC_FAR *pdwConnection);


void __RPC_STUB IDataAdviseHolder_Advise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataAdviseHolder_Unadvise_Proxy( 
    IDataAdviseHolder __RPC_FAR * This,
    /* [in] */ DWORD dwConnection);


void __RPC_STUB IDataAdviseHolder_Unadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataAdviseHolder_EnumAdvise_Proxy( 
    IDataAdviseHolder __RPC_FAR * This,
    /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise);


void __RPC_STUB IDataAdviseHolder_EnumAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataAdviseHolder_SendOnDataChange_Proxy( 
    IDataAdviseHolder __RPC_FAR * This,
    /* [unique][in] */ IDataObject __RPC_FAR *pDataObject,
    /* [in] */ DWORD dwReserved,
    /* [in] */ DWORD advf);


void __RPC_STUB IDataAdviseHolder_SendOnDataChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDataAdviseHolder_INTERFACE_DEFINED__ */


#ifndef __IMessageFilter_INTERFACE_DEFINED__
#define __IMessageFilter_INTERFACE_DEFINED__

/* interface IMessageFilter */
/* [uuid][object][local] */ 

typedef /* [unique] */ IMessageFilter __RPC_FAR *LPMESSAGEFILTER;

typedef 
enum tagCALLTYPE
    {	CALLTYPE_TOPLEVEL	= 1,
	CALLTYPE_NESTED	= 2,
	CALLTYPE_ASYNC	= 3,
	CALLTYPE_TOPLEVEL_CALLPENDING	= 4,
	CALLTYPE_ASYNC_CALLPENDING	= 5
    }	CALLTYPE;

typedef 
enum tagSERVERCALL
    {	SERVERCALL_ISHANDLED	= 0,
	SERVERCALL_REJECTED	= 1,
	SERVERCALL_RETRYLATER	= 2
    }	SERVERCALL;

typedef 
enum tagPENDINGTYPE
    {	PENDINGTYPE_TOPLEVEL	= 1,
	PENDINGTYPE_NESTED	= 2
    }	PENDINGTYPE;

typedef 
enum tagPENDINGMSG
    {	PENDINGMSG_CANCELCALL	= 0,
	PENDINGMSG_WAITNOPROCESS	= 1,
	PENDINGMSG_WAITDEFPROCESS	= 2
    }	PENDINGMSG;

typedef struct tagINTERFACEINFO
    {
    IUnknown __RPC_FAR *pUnk;
    IID iid;
    WORD wMethod;
    }	INTERFACEINFO;

typedef struct tagINTERFACEINFO __RPC_FAR *LPINTERFACEINFO;


EXTERN_C const IID IID_IMessageFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000016-0000-0000-C000-000000000046")
    IMessageFilter : public IUnknown
    {
    public:
        virtual DWORD STDMETHODCALLTYPE HandleInComingCall( 
            /* [in] */ DWORD dwCallType,
            /* [in] */ HTASK htaskCaller,
            /* [in] */ DWORD dwTickCount,
            /* [in] */ LPINTERFACEINFO lpInterfaceInfo) = 0;
        
        virtual DWORD STDMETHODCALLTYPE RetryRejectedCall( 
            /* [in] */ HTASK htaskCallee,
            /* [in] */ DWORD dwTickCount,
            /* [in] */ DWORD dwRejectType) = 0;
        
        virtual DWORD STDMETHODCALLTYPE MessagePending( 
            /* [in] */ HTASK htaskCallee,
            /* [in] */ DWORD dwTickCount,
            /* [in] */ DWORD dwPendingType) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMessageFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMessageFilter __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMessageFilter __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMessageFilter __RPC_FAR * This);
        
        DWORD ( STDMETHODCALLTYPE __RPC_FAR *HandleInComingCall )( 
            IMessageFilter __RPC_FAR * This,
            /* [in] */ DWORD dwCallType,
            /* [in] */ HTASK htaskCaller,
            /* [in] */ DWORD dwTickCount,
            /* [in] */ LPINTERFACEINFO lpInterfaceInfo);
        
        DWORD ( STDMETHODCALLTYPE __RPC_FAR *RetryRejectedCall )( 
            IMessageFilter __RPC_FAR * This,
            /* [in] */ HTASK htaskCallee,
            /* [in] */ DWORD dwTickCount,
            /* [in] */ DWORD dwRejectType);
        
        DWORD ( STDMETHODCALLTYPE __RPC_FAR *MessagePending )( 
            IMessageFilter __RPC_FAR * This,
            /* [in] */ HTASK htaskCallee,
            /* [in] */ DWORD dwTickCount,
            /* [in] */ DWORD dwPendingType);
        
        END_INTERFACE
    } IMessageFilterVtbl;

    interface IMessageFilter
    {
        CONST_VTBL struct IMessageFilterVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMessageFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMessageFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMessageFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMessageFilter_HandleInComingCall(This,dwCallType,htaskCaller,dwTickCount,lpInterfaceInfo)	\
    (This)->lpVtbl -> HandleInComingCall(This,dwCallType,htaskCaller,dwTickCount,lpInterfaceInfo)

#define IMessageFilter_RetryRejectedCall(This,htaskCallee,dwTickCount,dwRejectType)	\
    (This)->lpVtbl -> RetryRejectedCall(This,htaskCallee,dwTickCount,dwRejectType)

#define IMessageFilter_MessagePending(This,htaskCallee,dwTickCount,dwPendingType)	\
    (This)->lpVtbl -> MessagePending(This,htaskCallee,dwTickCount,dwPendingType)

#endif /* COBJMACROS */


#endif 	/* C style interface */



DWORD STDMETHODCALLTYPE IMessageFilter_HandleInComingCall_Proxy( 
    IMessageFilter __RPC_FAR * This,
    /* [in] */ DWORD dwCallType,
    /* [in] */ HTASK htaskCaller,
    /* [in] */ DWORD dwTickCount,
    /* [in] */ LPINTERFACEINFO lpInterfaceInfo);


void __RPC_STUB IMessageFilter_HandleInComingCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD STDMETHODCALLTYPE IMessageFilter_RetryRejectedCall_Proxy( 
    IMessageFilter __RPC_FAR * This,
    /* [in] */ HTASK htaskCallee,
    /* [in] */ DWORD dwTickCount,
    /* [in] */ DWORD dwRejectType);


void __RPC_STUB IMessageFilter_RetryRejectedCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD STDMETHODCALLTYPE IMessageFilter_MessagePending_Proxy( 
    IMessageFilter __RPC_FAR * This,
    /* [in] */ HTASK htaskCallee,
    /* [in] */ DWORD dwTickCount,
    /* [in] */ DWORD dwPendingType);


void __RPC_STUB IMessageFilter_MessagePending_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMessageFilter_INTERFACE_DEFINED__ */


#ifndef __IRpcChannelBuffer_INTERFACE_DEFINED__
#define __IRpcChannelBuffer_INTERFACE_DEFINED__

/* interface IRpcChannelBuffer */
/* [uuid][object][local] */ 

typedef unsigned long RPCOLEDATAREP;

typedef struct tagRPCOLEMESSAGE
    {
    void __RPC_FAR *reserved1;
    RPCOLEDATAREP dataRepresentation;
    void __RPC_FAR *Buffer;
    ULONG cbBuffer;
    ULONG iMethod;
    void __RPC_FAR *reserved2[ 5 ];
    ULONG rpcFlags;
    }	RPCOLEMESSAGE;

typedef RPCOLEMESSAGE __RPC_FAR *PRPCOLEMESSAGE;


EXTERN_C const IID IID_IRpcChannelBuffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D5F56B60-593B-101A-B569-08002B2DBF7A")
    IRpcChannelBuffer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetBuffer( 
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
            /* [in] */ REFIID riid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SendReceive( 
            /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
            /* [out] */ ULONG __RPC_FAR *pStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeBuffer( 
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDestCtx( 
            /* [out] */ DWORD __RPC_FAR *pdwDestContext,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvDestContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsConnected( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRpcChannelBufferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRpcChannelBuffer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRpcChannelBuffer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRpcChannelBuffer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBuffer )( 
            IRpcChannelBuffer __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
            /* [in] */ REFIID riid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendReceive )( 
            IRpcChannelBuffer __RPC_FAR * This,
            /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
            /* [out] */ ULONG __RPC_FAR *pStatus);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FreeBuffer )( 
            IRpcChannelBuffer __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDestCtx )( 
            IRpcChannelBuffer __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwDestContext,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvDestContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsConnected )( 
            IRpcChannelBuffer __RPC_FAR * This);
        
        END_INTERFACE
    } IRpcChannelBufferVtbl;

    interface IRpcChannelBuffer
    {
        CONST_VTBL struct IRpcChannelBufferVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcChannelBuffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcChannelBuffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcChannelBuffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcChannelBuffer_GetBuffer(This,pMessage,riid)	\
    (This)->lpVtbl -> GetBuffer(This,pMessage,riid)

#define IRpcChannelBuffer_SendReceive(This,pMessage,pStatus)	\
    (This)->lpVtbl -> SendReceive(This,pMessage,pStatus)

#define IRpcChannelBuffer_FreeBuffer(This,pMessage)	\
    (This)->lpVtbl -> FreeBuffer(This,pMessage)

#define IRpcChannelBuffer_GetDestCtx(This,pdwDestContext,ppvDestContext)	\
    (This)->lpVtbl -> GetDestCtx(This,pdwDestContext,ppvDestContext)

#define IRpcChannelBuffer_IsConnected(This)	\
    (This)->lpVtbl -> IsConnected(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRpcChannelBuffer_GetBuffer_Proxy( 
    IRpcChannelBuffer __RPC_FAR * This,
    /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
    /* [in] */ REFIID riid);


void __RPC_STUB IRpcChannelBuffer_GetBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer_SendReceive_Proxy( 
    IRpcChannelBuffer __RPC_FAR * This,
    /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
    /* [out] */ ULONG __RPC_FAR *pStatus);


void __RPC_STUB IRpcChannelBuffer_SendReceive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer_FreeBuffer_Proxy( 
    IRpcChannelBuffer __RPC_FAR * This,
    /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage);


void __RPC_STUB IRpcChannelBuffer_FreeBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer_GetDestCtx_Proxy( 
    IRpcChannelBuffer __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwDestContext,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppvDestContext);


void __RPC_STUB IRpcChannelBuffer_GetDestCtx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer_IsConnected_Proxy( 
    IRpcChannelBuffer __RPC_FAR * This);


void __RPC_STUB IRpcChannelBuffer_IsConnected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRpcChannelBuffer_INTERFACE_DEFINED__ */


#ifndef __IRpcChannelBuffer2_INTERFACE_DEFINED__
#define __IRpcChannelBuffer2_INTERFACE_DEFINED__

/* interface IRpcChannelBuffer2 */
/* [uuid][object][local] */ 


EXTERN_C const IID IID_IRpcChannelBuffer2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("594f31d0-7f19-11d0-b194-00a0c90dc8bf")
    IRpcChannelBuffer2 : public IRpcChannelBuffer
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetProtocolVersion( 
            /* [out][in] */ DWORD __RPC_FAR *pdwVersion) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRpcChannelBuffer2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRpcChannelBuffer2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRpcChannelBuffer2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRpcChannelBuffer2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBuffer )( 
            IRpcChannelBuffer2 __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
            /* [in] */ REFIID riid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendReceive )( 
            IRpcChannelBuffer2 __RPC_FAR * This,
            /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
            /* [out] */ ULONG __RPC_FAR *pStatus);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FreeBuffer )( 
            IRpcChannelBuffer2 __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDestCtx )( 
            IRpcChannelBuffer2 __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwDestContext,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvDestContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsConnected )( 
            IRpcChannelBuffer2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProtocolVersion )( 
            IRpcChannelBuffer2 __RPC_FAR * This,
            /* [out][in] */ DWORD __RPC_FAR *pdwVersion);
        
        END_INTERFACE
    } IRpcChannelBuffer2Vtbl;

    interface IRpcChannelBuffer2
    {
        CONST_VTBL struct IRpcChannelBuffer2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcChannelBuffer2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcChannelBuffer2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcChannelBuffer2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcChannelBuffer2_GetBuffer(This,pMessage,riid)	\
    (This)->lpVtbl -> GetBuffer(This,pMessage,riid)

#define IRpcChannelBuffer2_SendReceive(This,pMessage,pStatus)	\
    (This)->lpVtbl -> SendReceive(This,pMessage,pStatus)

#define IRpcChannelBuffer2_FreeBuffer(This,pMessage)	\
    (This)->lpVtbl -> FreeBuffer(This,pMessage)

#define IRpcChannelBuffer2_GetDestCtx(This,pdwDestContext,ppvDestContext)	\
    (This)->lpVtbl -> GetDestCtx(This,pdwDestContext,ppvDestContext)

#define IRpcChannelBuffer2_IsConnected(This)	\
    (This)->lpVtbl -> IsConnected(This)


#define IRpcChannelBuffer2_GetProtocolVersion(This,pdwVersion)	\
    (This)->lpVtbl -> GetProtocolVersion(This,pdwVersion)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRpcChannelBuffer2_GetProtocolVersion_Proxy( 
    IRpcChannelBuffer2 __RPC_FAR * This,
    /* [out][in] */ DWORD __RPC_FAR *pdwVersion);


void __RPC_STUB IRpcChannelBuffer2_GetProtocolVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRpcChannelBuffer2_INTERFACE_DEFINED__ */


#ifndef __IAsyncRpcChannelBuffer_INTERFACE_DEFINED__
#define __IAsyncRpcChannelBuffer_INTERFACE_DEFINED__

/* interface IAsyncRpcChannelBuffer */
/* [unique][uuid][object][local] */ 


EXTERN_C const IID IID_IAsyncRpcChannelBuffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a5029fb6-3c34-11d1-9c99-00c04fb998aa")
    IAsyncRpcChannelBuffer : public IRpcChannelBuffer2
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Send( 
            /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [in] */ ISynchronize __RPC_FAR *pSync,
            /* [out] */ ULONG __RPC_FAR *pulStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Receive( 
            /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [out] */ ULONG __RPC_FAR *pulStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDestCtxEx( 
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [out] */ DWORD __RPC_FAR *pdwDestContext,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvDestContext) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAsyncRpcChannelBufferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAsyncRpcChannelBuffer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAsyncRpcChannelBuffer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAsyncRpcChannelBuffer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBuffer )( 
            IAsyncRpcChannelBuffer __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
            /* [in] */ REFIID riid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendReceive )( 
            IAsyncRpcChannelBuffer __RPC_FAR * This,
            /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
            /* [out] */ ULONG __RPC_FAR *pStatus);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FreeBuffer )( 
            IAsyncRpcChannelBuffer __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDestCtx )( 
            IAsyncRpcChannelBuffer __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwDestContext,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvDestContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsConnected )( 
            IAsyncRpcChannelBuffer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProtocolVersion )( 
            IAsyncRpcChannelBuffer __RPC_FAR * This,
            /* [out][in] */ DWORD __RPC_FAR *pdwVersion);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Send )( 
            IAsyncRpcChannelBuffer __RPC_FAR * This,
            /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [in] */ ISynchronize __RPC_FAR *pSync,
            /* [out] */ ULONG __RPC_FAR *pulStatus);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Receive )( 
            IAsyncRpcChannelBuffer __RPC_FAR * This,
            /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [out] */ ULONG __RPC_FAR *pulStatus);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDestCtxEx )( 
            IAsyncRpcChannelBuffer __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [out] */ DWORD __RPC_FAR *pdwDestContext,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvDestContext);
        
        END_INTERFACE
    } IAsyncRpcChannelBufferVtbl;

    interface IAsyncRpcChannelBuffer
    {
        CONST_VTBL struct IAsyncRpcChannelBufferVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAsyncRpcChannelBuffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAsyncRpcChannelBuffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAsyncRpcChannelBuffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAsyncRpcChannelBuffer_GetBuffer(This,pMessage,riid)	\
    (This)->lpVtbl -> GetBuffer(This,pMessage,riid)

#define IAsyncRpcChannelBuffer_SendReceive(This,pMessage,pStatus)	\
    (This)->lpVtbl -> SendReceive(This,pMessage,pStatus)

#define IAsyncRpcChannelBuffer_FreeBuffer(This,pMessage)	\
    (This)->lpVtbl -> FreeBuffer(This,pMessage)

#define IAsyncRpcChannelBuffer_GetDestCtx(This,pdwDestContext,ppvDestContext)	\
    (This)->lpVtbl -> GetDestCtx(This,pdwDestContext,ppvDestContext)

#define IAsyncRpcChannelBuffer_IsConnected(This)	\
    (This)->lpVtbl -> IsConnected(This)


#define IAsyncRpcChannelBuffer_GetProtocolVersion(This,pdwVersion)	\
    (This)->lpVtbl -> GetProtocolVersion(This,pdwVersion)


#define IAsyncRpcChannelBuffer_Send(This,pMsg,pSync,pulStatus)	\
    (This)->lpVtbl -> Send(This,pMsg,pSync,pulStatus)

#define IAsyncRpcChannelBuffer_Receive(This,pMsg,pulStatus)	\
    (This)->lpVtbl -> Receive(This,pMsg,pulStatus)

#define IAsyncRpcChannelBuffer_GetDestCtxEx(This,pMsg,pdwDestContext,ppvDestContext)	\
    (This)->lpVtbl -> GetDestCtxEx(This,pMsg,pdwDestContext,ppvDestContext)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IAsyncRpcChannelBuffer_Send_Proxy( 
    IAsyncRpcChannelBuffer __RPC_FAR * This,
    /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
    /* [in] */ ISynchronize __RPC_FAR *pSync,
    /* [out] */ ULONG __RPC_FAR *pulStatus);


void __RPC_STUB IAsyncRpcChannelBuffer_Send_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAsyncRpcChannelBuffer_Receive_Proxy( 
    IAsyncRpcChannelBuffer __RPC_FAR * This,
    /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
    /* [out] */ ULONG __RPC_FAR *pulStatus);


void __RPC_STUB IAsyncRpcChannelBuffer_Receive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAsyncRpcChannelBuffer_GetDestCtxEx_Proxy( 
    IAsyncRpcChannelBuffer __RPC_FAR * This,
    /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
    /* [out] */ DWORD __RPC_FAR *pdwDestContext,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppvDestContext);


void __RPC_STUB IAsyncRpcChannelBuffer_GetDestCtxEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAsyncRpcChannelBuffer_INTERFACE_DEFINED__ */


#ifndef __IRpcChannelBuffer3_INTERFACE_DEFINED__
#define __IRpcChannelBuffer3_INTERFACE_DEFINED__

/* interface IRpcChannelBuffer3 */
/* [uuid][object][local] */ 


EXTERN_C const IID IID_IRpcChannelBuffer3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("25B15600-0115-11d0-BF0D-00AA00B8DFD2")
    IRpcChannelBuffer3 : public IRpcChannelBuffer2
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Send( 
            /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [out] */ ULONG __RPC_FAR *pulStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Receive( 
            /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [in] */ ULONG ulSize,
            /* [out] */ ULONG __RPC_FAR *pulStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Cancel( 
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCallContext( 
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *pInterface) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDestCtxEx( 
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [out] */ DWORD __RPC_FAR *pdwDestContext,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvDestContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetState( 
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [out] */ DWORD __RPC_FAR *pState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterAsync( 
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [in] */ IAsyncManager __RPC_FAR *pAsyncMgr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRpcChannelBuffer3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRpcChannelBuffer3 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRpcChannelBuffer3 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRpcChannelBuffer3 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBuffer )( 
            IRpcChannelBuffer3 __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
            /* [in] */ REFIID riid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendReceive )( 
            IRpcChannelBuffer3 __RPC_FAR * This,
            /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
            /* [out] */ ULONG __RPC_FAR *pStatus);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FreeBuffer )( 
            IRpcChannelBuffer3 __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDestCtx )( 
            IRpcChannelBuffer3 __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwDestContext,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvDestContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsConnected )( 
            IRpcChannelBuffer3 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProtocolVersion )( 
            IRpcChannelBuffer3 __RPC_FAR * This,
            /* [out][in] */ DWORD __RPC_FAR *pdwVersion);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Send )( 
            IRpcChannelBuffer3 __RPC_FAR * This,
            /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [out] */ ULONG __RPC_FAR *pulStatus);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Receive )( 
            IRpcChannelBuffer3 __RPC_FAR * This,
            /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [in] */ ULONG ulSize,
            /* [out] */ ULONG __RPC_FAR *pulStatus);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Cancel )( 
            IRpcChannelBuffer3 __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCallContext )( 
            IRpcChannelBuffer3 __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *pInterface);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDestCtxEx )( 
            IRpcChannelBuffer3 __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [out] */ DWORD __RPC_FAR *pdwDestContext,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvDestContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetState )( 
            IRpcChannelBuffer3 __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [out] */ DWORD __RPC_FAR *pState);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RegisterAsync )( 
            IRpcChannelBuffer3 __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [in] */ IAsyncManager __RPC_FAR *pAsyncMgr);
        
        END_INTERFACE
    } IRpcChannelBuffer3Vtbl;

    interface IRpcChannelBuffer3
    {
        CONST_VTBL struct IRpcChannelBuffer3Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcChannelBuffer3_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcChannelBuffer3_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcChannelBuffer3_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcChannelBuffer3_GetBuffer(This,pMessage,riid)	\
    (This)->lpVtbl -> GetBuffer(This,pMessage,riid)

#define IRpcChannelBuffer3_SendReceive(This,pMessage,pStatus)	\
    (This)->lpVtbl -> SendReceive(This,pMessage,pStatus)

#define IRpcChannelBuffer3_FreeBuffer(This,pMessage)	\
    (This)->lpVtbl -> FreeBuffer(This,pMessage)

#define IRpcChannelBuffer3_GetDestCtx(This,pdwDestContext,ppvDestContext)	\
    (This)->lpVtbl -> GetDestCtx(This,pdwDestContext,ppvDestContext)

#define IRpcChannelBuffer3_IsConnected(This)	\
    (This)->lpVtbl -> IsConnected(This)


#define IRpcChannelBuffer3_GetProtocolVersion(This,pdwVersion)	\
    (This)->lpVtbl -> GetProtocolVersion(This,pdwVersion)


#define IRpcChannelBuffer3_Send(This,pMsg,pulStatus)	\
    (This)->lpVtbl -> Send(This,pMsg,pulStatus)

#define IRpcChannelBuffer3_Receive(This,pMsg,ulSize,pulStatus)	\
    (This)->lpVtbl -> Receive(This,pMsg,ulSize,pulStatus)

#define IRpcChannelBuffer3_Cancel(This,pMsg)	\
    (This)->lpVtbl -> Cancel(This,pMsg)

#define IRpcChannelBuffer3_GetCallContext(This,pMsg,riid,pInterface)	\
    (This)->lpVtbl -> GetCallContext(This,pMsg,riid,pInterface)

#define IRpcChannelBuffer3_GetDestCtxEx(This,pMsg,pdwDestContext,ppvDestContext)	\
    (This)->lpVtbl -> GetDestCtxEx(This,pMsg,pdwDestContext,ppvDestContext)

#define IRpcChannelBuffer3_GetState(This,pMsg,pState)	\
    (This)->lpVtbl -> GetState(This,pMsg,pState)

#define IRpcChannelBuffer3_RegisterAsync(This,pMsg,pAsyncMgr)	\
    (This)->lpVtbl -> RegisterAsync(This,pMsg,pAsyncMgr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRpcChannelBuffer3_Send_Proxy( 
    IRpcChannelBuffer3 __RPC_FAR * This,
    /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
    /* [out] */ ULONG __RPC_FAR *pulStatus);


void __RPC_STUB IRpcChannelBuffer3_Send_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer3_Receive_Proxy( 
    IRpcChannelBuffer3 __RPC_FAR * This,
    /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
    /* [in] */ ULONG ulSize,
    /* [out] */ ULONG __RPC_FAR *pulStatus);


void __RPC_STUB IRpcChannelBuffer3_Receive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer3_Cancel_Proxy( 
    IRpcChannelBuffer3 __RPC_FAR * This,
    /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg);


void __RPC_STUB IRpcChannelBuffer3_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer3_GetCallContext_Proxy( 
    IRpcChannelBuffer3 __RPC_FAR * This,
    /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
    /* [in] */ REFIID riid,
    /* [out] */ void __RPC_FAR *__RPC_FAR *pInterface);


void __RPC_STUB IRpcChannelBuffer3_GetCallContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer3_GetDestCtxEx_Proxy( 
    IRpcChannelBuffer3 __RPC_FAR * This,
    /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
    /* [out] */ DWORD __RPC_FAR *pdwDestContext,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppvDestContext);


void __RPC_STUB IRpcChannelBuffer3_GetDestCtxEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer3_GetState_Proxy( 
    IRpcChannelBuffer3 __RPC_FAR * This,
    /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
    /* [out] */ DWORD __RPC_FAR *pState);


void __RPC_STUB IRpcChannelBuffer3_GetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer3_RegisterAsync_Proxy( 
    IRpcChannelBuffer3 __RPC_FAR * This,
    /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
    /* [in] */ IAsyncManager __RPC_FAR *pAsyncMgr);


void __RPC_STUB IRpcChannelBuffer3_RegisterAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRpcChannelBuffer3_INTERFACE_DEFINED__ */


#ifndef __IRpcProxyBuffer_INTERFACE_DEFINED__
#define __IRpcProxyBuffer_INTERFACE_DEFINED__

/* interface IRpcProxyBuffer */
/* [uuid][object][local] */ 


EXTERN_C const IID IID_IRpcProxyBuffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D5F56A34-593B-101A-B569-08002B2DBF7A")
    IRpcProxyBuffer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Connect( 
            /* [unique][in] */ IRpcChannelBuffer __RPC_FAR *pRpcChannelBuffer) = 0;
        
        virtual void STDMETHODCALLTYPE Disconnect( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRpcProxyBufferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRpcProxyBuffer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRpcProxyBuffer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRpcProxyBuffer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Connect )( 
            IRpcProxyBuffer __RPC_FAR * This,
            /* [unique][in] */ IRpcChannelBuffer __RPC_FAR *pRpcChannelBuffer);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *Disconnect )( 
            IRpcProxyBuffer __RPC_FAR * This);
        
        END_INTERFACE
    } IRpcProxyBufferVtbl;

    interface IRpcProxyBuffer
    {
        CONST_VTBL struct IRpcProxyBufferVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcProxyBuffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcProxyBuffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcProxyBuffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcProxyBuffer_Connect(This,pRpcChannelBuffer)	\
    (This)->lpVtbl -> Connect(This,pRpcChannelBuffer)

#define IRpcProxyBuffer_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRpcProxyBuffer_Connect_Proxy( 
    IRpcProxyBuffer __RPC_FAR * This,
    /* [unique][in] */ IRpcChannelBuffer __RPC_FAR *pRpcChannelBuffer);


void __RPC_STUB IRpcProxyBuffer_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IRpcProxyBuffer_Disconnect_Proxy( 
    IRpcProxyBuffer __RPC_FAR * This);


void __RPC_STUB IRpcProxyBuffer_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRpcProxyBuffer_INTERFACE_DEFINED__ */


#ifndef __IRpcStubBuffer_INTERFACE_DEFINED__
#define __IRpcStubBuffer_INTERFACE_DEFINED__

/* interface IRpcStubBuffer */
/* [uuid][object][local] */ 


EXTERN_C const IID IID_IRpcStubBuffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D5F56AFC-593B-101A-B569-08002B2DBF7A")
    IRpcStubBuffer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Connect( 
            /* [in] */ IUnknown __RPC_FAR *pUnkServer) = 0;
        
        virtual void STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Invoke( 
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *_prpcmsg,
            /* [in] */ IRpcChannelBuffer __RPC_FAR *_pRpcChannelBuffer) = 0;
        
        virtual IRpcStubBuffer __RPC_FAR *STDMETHODCALLTYPE IsIIDSupported( 
            /* [in] */ REFIID riid) = 0;
        
        virtual ULONG STDMETHODCALLTYPE CountRefs( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DebugServerQueryInterface( 
            void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual void STDMETHODCALLTYPE DebugServerRelease( 
            void __RPC_FAR *pv) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRpcStubBufferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRpcStubBuffer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRpcStubBuffer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRpcStubBuffer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Connect )( 
            IRpcStubBuffer __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkServer);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *Disconnect )( 
            IRpcStubBuffer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IRpcStubBuffer __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *_prpcmsg,
            /* [in] */ IRpcChannelBuffer __RPC_FAR *_pRpcChannelBuffer);
        
        IRpcStubBuffer __RPC_FAR *( STDMETHODCALLTYPE __RPC_FAR *IsIIDSupported )( 
            IRpcStubBuffer __RPC_FAR * This,
            /* [in] */ REFIID riid);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *CountRefs )( 
            IRpcStubBuffer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DebugServerQueryInterface )( 
            IRpcStubBuffer __RPC_FAR * This,
            void __RPC_FAR *__RPC_FAR *ppv);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *DebugServerRelease )( 
            IRpcStubBuffer __RPC_FAR * This,
            void __RPC_FAR *pv);
        
        END_INTERFACE
    } IRpcStubBufferVtbl;

    interface IRpcStubBuffer
    {
        CONST_VTBL struct IRpcStubBufferVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcStubBuffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcStubBuffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcStubBuffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcStubBuffer_Connect(This,pUnkServer)	\
    (This)->lpVtbl -> Connect(This,pUnkServer)

#define IRpcStubBuffer_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#define IRpcStubBuffer_Invoke(This,_prpcmsg,_pRpcChannelBuffer)	\
    (This)->lpVtbl -> Invoke(This,_prpcmsg,_pRpcChannelBuffer)

#define IRpcStubBuffer_IsIIDSupported(This,riid)	\
    (This)->lpVtbl -> IsIIDSupported(This,riid)

#define IRpcStubBuffer_CountRefs(This)	\
    (This)->lpVtbl -> CountRefs(This)

#define IRpcStubBuffer_DebugServerQueryInterface(This,ppv)	\
    (This)->lpVtbl -> DebugServerQueryInterface(This,ppv)

#define IRpcStubBuffer_DebugServerRelease(This,pv)	\
    (This)->lpVtbl -> DebugServerRelease(This,pv)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRpcStubBuffer_Connect_Proxy( 
    IRpcStubBuffer __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnkServer);


void __RPC_STUB IRpcStubBuffer_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IRpcStubBuffer_Disconnect_Proxy( 
    IRpcStubBuffer __RPC_FAR * This);


void __RPC_STUB IRpcStubBuffer_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcStubBuffer_Invoke_Proxy( 
    IRpcStubBuffer __RPC_FAR * This,
    /* [in] */ RPCOLEMESSAGE __RPC_FAR *_prpcmsg,
    /* [in] */ IRpcChannelBuffer __RPC_FAR *_pRpcChannelBuffer);


void __RPC_STUB IRpcStubBuffer_Invoke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


IRpcStubBuffer __RPC_FAR *STDMETHODCALLTYPE IRpcStubBuffer_IsIIDSupported_Proxy( 
    IRpcStubBuffer __RPC_FAR * This,
    /* [in] */ REFIID riid);


void __RPC_STUB IRpcStubBuffer_IsIIDSupported_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG STDMETHODCALLTYPE IRpcStubBuffer_CountRefs_Proxy( 
    IRpcStubBuffer __RPC_FAR * This);


void __RPC_STUB IRpcStubBuffer_CountRefs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcStubBuffer_DebugServerQueryInterface_Proxy( 
    IRpcStubBuffer __RPC_FAR * This,
    void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB IRpcStubBuffer_DebugServerQueryInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IRpcStubBuffer_DebugServerRelease_Proxy( 
    IRpcStubBuffer __RPC_FAR * This,
    void __RPC_FAR *pv);


void __RPC_STUB IRpcStubBuffer_DebugServerRelease_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRpcStubBuffer_INTERFACE_DEFINED__ */


#ifndef __IPSFactoryBuffer_INTERFACE_DEFINED__
#define __IPSFactoryBuffer_INTERFACE_DEFINED__

/* interface IPSFactoryBuffer */
/* [uuid][object][local] */ 


EXTERN_C const IID IID_IPSFactoryBuffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D5F569D0-593B-101A-B569-08002B2DBF7A")
    IPSFactoryBuffer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateProxy( 
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ REFIID riid,
            /* [out] */ IRpcProxyBuffer __RPC_FAR *__RPC_FAR *ppProxy,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateStub( 
            /* [in] */ REFIID riid,
            /* [unique][in] */ IUnknown __RPC_FAR *pUnkServer,
            /* [out] */ IRpcStubBuffer __RPC_FAR *__RPC_FAR *ppStub) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPSFactoryBufferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPSFactoryBuffer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPSFactoryBuffer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPSFactoryBuffer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateProxy )( 
            IPSFactoryBuffer __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ REFIID riid,
            /* [out] */ IRpcProxyBuffer __RPC_FAR *__RPC_FAR *ppProxy,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateStub )( 
            IPSFactoryBuffer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [unique][in] */ IUnknown __RPC_FAR *pUnkServer,
            /* [out] */ IRpcStubBuffer __RPC_FAR *__RPC_FAR *ppStub);
        
        END_INTERFACE
    } IPSFactoryBufferVtbl;

    interface IPSFactoryBuffer
    {
        CONST_VTBL struct IPSFactoryBufferVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPSFactoryBuffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPSFactoryBuffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPSFactoryBuffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPSFactoryBuffer_CreateProxy(This,pUnkOuter,riid,ppProxy,ppv)	\
    (This)->lpVtbl -> CreateProxy(This,pUnkOuter,riid,ppProxy,ppv)

#define IPSFactoryBuffer_CreateStub(This,riid,pUnkServer,ppStub)	\
    (This)->lpVtbl -> CreateStub(This,riid,pUnkServer,ppStub)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPSFactoryBuffer_CreateProxy_Proxy( 
    IPSFactoryBuffer __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
    /* [in] */ REFIID riid,
    /* [out] */ IRpcProxyBuffer __RPC_FAR *__RPC_FAR *ppProxy,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB IPSFactoryBuffer_CreateProxy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPSFactoryBuffer_CreateStub_Proxy( 
    IPSFactoryBuffer __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [unique][in] */ IUnknown __RPC_FAR *pUnkServer,
    /* [out] */ IRpcStubBuffer __RPC_FAR *__RPC_FAR *ppStub);


void __RPC_STUB IPSFactoryBuffer_CreateStub_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPSFactoryBuffer_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_objidl_0049 */
/* [local] */ 

#if  (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM) // DCOM
// This interface is only valid on Windows NT 4.0
typedef struct SChannelHookCallInfo
    {
    IID iid;
    DWORD cbSize;
    GUID uCausality;
    DWORD dwServerPid;
    DWORD iMethod;
    void __RPC_FAR *pObject;
    }	SChannelHookCallInfo;



extern RPC_IF_HANDLE __MIDL_itf_objidl_0049_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_objidl_0049_v0_0_s_ifspec;

#ifndef __IChannelHook_INTERFACE_DEFINED__
#define __IChannelHook_INTERFACE_DEFINED__

/* interface IChannelHook */
/* [uuid][object][local] */ 


EXTERN_C const IID IID_IChannelHook;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1008c4a0-7613-11cf-9af1-0020af6e72f4")
    IChannelHook : public IUnknown
    {
    public:
        virtual void STDMETHODCALLTYPE ClientGetSize( 
            /* [in] */ REFGUID uExtent,
            /* [in] */ REFIID riid,
            /* [out] */ ULONG __RPC_FAR *pDataSize) = 0;
        
        virtual void STDMETHODCALLTYPE ClientFillBuffer( 
            /* [in] */ REFGUID uExtent,
            /* [in] */ REFIID riid,
            /* [out][in] */ ULONG __RPC_FAR *pDataSize,
            /* [in] */ void __RPC_FAR *pDataBuffer) = 0;
        
        virtual void STDMETHODCALLTYPE ClientNotify( 
            /* [in] */ REFGUID uExtent,
            /* [in] */ REFIID riid,
            /* [in] */ ULONG cbDataSize,
            /* [in] */ void __RPC_FAR *pDataBuffer,
            /* [in] */ DWORD lDataRep,
            /* [in] */ HRESULT hrFault) = 0;
        
        virtual void STDMETHODCALLTYPE ServerNotify( 
            /* [in] */ REFGUID uExtent,
            /* [in] */ REFIID riid,
            /* [in] */ ULONG cbDataSize,
            /* [in] */ void __RPC_FAR *pDataBuffer,
            /* [in] */ DWORD lDataRep) = 0;
        
        virtual void STDMETHODCALLTYPE ServerGetSize( 
            /* [in] */ REFGUID uExtent,
            /* [in] */ REFIID riid,
            /* [in] */ HRESULT hrFault,
            /* [out] */ ULONG __RPC_FAR *pDataSize) = 0;
        
        virtual void STDMETHODCALLTYPE ServerFillBuffer( 
            /* [in] */ REFGUID uExtent,
            /* [in] */ REFIID riid,
            /* [out][in] */ ULONG __RPC_FAR *pDataSize,
            /* [in] */ void __RPC_FAR *pDataBuffer,
            /* [in] */ HRESULT hrFault) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IChannelHookVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IChannelHook __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IChannelHook __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IChannelHook __RPC_FAR * This);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *ClientGetSize )( 
            IChannelHook __RPC_FAR * This,
            /* [in] */ REFGUID uExtent,
            /* [in] */ REFIID riid,
            /* [out] */ ULONG __RPC_FAR *pDataSize);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *ClientFillBuffer )( 
            IChannelHook __RPC_FAR * This,
            /* [in] */ REFGUID uExtent,
            /* [in] */ REFIID riid,
            /* [out][in] */ ULONG __RPC_FAR *pDataSize,
            /* [in] */ void __RPC_FAR *pDataBuffer);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *ClientNotify )( 
            IChannelHook __RPC_FAR * This,
            /* [in] */ REFGUID uExtent,
            /* [in] */ REFIID riid,
            /* [in] */ ULONG cbDataSize,
            /* [in] */ void __RPC_FAR *pDataBuffer,
            /* [in] */ DWORD lDataRep,
            /* [in] */ HRESULT hrFault);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *ServerNotify )( 
            IChannelHook __RPC_FAR * This,
            /* [in] */ REFGUID uExtent,
            /* [in] */ REFIID riid,
            /* [in] */ ULONG cbDataSize,
            /* [in] */ void __RPC_FAR *pDataBuffer,
            /* [in] */ DWORD lDataRep);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *ServerGetSize )( 
            IChannelHook __RPC_FAR * This,
            /* [in] */ REFGUID uExtent,
            /* [in] */ REFIID riid,
            /* [in] */ HRESULT hrFault,
            /* [out] */ ULONG __RPC_FAR *pDataSize);
        
        void ( STDMETHODCALLTYPE __RPC_FAR *ServerFillBuffer )( 
            IChannelHook __RPC_FAR * This,
            /* [in] */ REFGUID uExtent,
            /* [in] */ REFIID riid,
            /* [out][in] */ ULONG __RPC_FAR *pDataSize,
            /* [in] */ void __RPC_FAR *pDataBuffer,
            /* [in] */ HRESULT hrFault);
        
        END_INTERFACE
    } IChannelHookVtbl;

    interface IChannelHook
    {
        CONST_VTBL struct IChannelHookVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IChannelHook_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IChannelHook_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IChannelHook_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IChannelHook_ClientGetSize(This,uExtent,riid,pDataSize)	\
    (This)->lpVtbl -> ClientGetSize(This,uExtent,riid,pDataSize)

#define IChannelHook_ClientFillBuffer(This,uExtent,riid,pDataSize,pDataBuffer)	\
    (This)->lpVtbl -> ClientFillBuffer(This,uExtent,riid,pDataSize,pDataBuffer)

#define IChannelHook_ClientNotify(This,uExtent,riid,cbDataSize,pDataBuffer,lDataRep,hrFault)	\
    (This)->lpVtbl -> ClientNotify(This,uExtent,riid,cbDataSize,pDataBuffer,lDataRep,hrFault)

#define IChannelHook_ServerNotify(This,uExtent,riid,cbDataSize,pDataBuffer,lDataRep)	\
    (This)->lpVtbl -> ServerNotify(This,uExtent,riid,cbDataSize,pDataBuffer,lDataRep)

#define IChannelHook_ServerGetSize(This,uExtent,riid,hrFault,pDataSize)	\
    (This)->lpVtbl -> ServerGetSize(This,uExtent,riid,hrFault,pDataSize)

#define IChannelHook_ServerFillBuffer(This,uExtent,riid,pDataSize,pDataBuffer,hrFault)	\
    (This)->lpVtbl -> ServerFillBuffer(This,uExtent,riid,pDataSize,pDataBuffer,hrFault)

#endif /* COBJMACROS */


#endif 	/* C style interface */



void STDMETHODCALLTYPE IChannelHook_ClientGetSize_Proxy( 
    IChannelHook __RPC_FAR * This,
    /* [in] */ REFGUID uExtent,
    /* [in] */ REFIID riid,
    /* [out] */ ULONG __RPC_FAR *pDataSize);


void __RPC_STUB IChannelHook_ClientGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IChannelHook_ClientFillBuffer_Proxy( 
    IChannelHook __RPC_FAR * This,
    /* [in] */ REFGUID uExtent,
    /* [in] */ REFIID riid,
    /* [out][in] */ ULONG __RPC_FAR *pDataSize,
    /* [in] */ void __RPC_FAR *pDataBuffer);


void __RPC_STUB IChannelHook_ClientFillBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IChannelHook_ClientNotify_Proxy( 
    IChannelHook __RPC_FAR * This,
    /* [in] */ REFGUID uExtent,
    /* [in] */ REFIID riid,
    /* [in] */ ULONG cbDataSize,
    /* [in] */ void __RPC_FAR *pDataBuffer,
    /* [in] */ DWORD lDataRep,
    /* [in] */ HRESULT hrFault);


void __RPC_STUB IChannelHook_ClientNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IChannelHook_ServerNotify_Proxy( 
    IChannelHook __RPC_FAR * This,
    /* [in] */ REFGUID uExtent,
    /* [in] */ REFIID riid,
    /* [in] */ ULONG cbDataSize,
    /* [in] */ void __RPC_FAR *pDataBuffer,
    /* [in] */ DWORD lDataRep);


void __RPC_STUB IChannelHook_ServerNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IChannelHook_ServerGetSize_Proxy( 
    IChannelHook __RPC_FAR * This,
    /* [in] */ REFGUID uExtent,
    /* [in] */ REFIID riid,
    /* [in] */ HRESULT hrFault,
    /* [out] */ ULONG __RPC_FAR *pDataSize);


void __RPC_STUB IChannelHook_ServerGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IChannelHook_ServerFillBuffer_Proxy( 
    IChannelHook __RPC_FAR * This,
    /* [in] */ REFGUID uExtent,
    /* [in] */ REFIID riid,
    /* [out][in] */ ULONG __RPC_FAR *pDataSize,
    /* [in] */ void __RPC_FAR *pDataBuffer,
    /* [in] */ HRESULT hrFault);


void __RPC_STUB IChannelHook_ServerFillBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IChannelHook_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_objidl_0050 */
/* [local] */ 

#endif //DCOM

// Well-known Property Set Format IDs
extern const FMTID FMTID_SummaryInformation;

extern const FMTID FMTID_DocSummaryInformation;

extern const FMTID FMTID_UserDefinedProperties;

extern const FMTID FMTID_DiscardableInformation;

extern const FMTID FMTID_ImageSummaryInformation;

extern const FMTID FMTID_AudioSummaryInformation;

extern const FMTID FMTID_VideoSummaryInformation;

extern const FMTID FMTID_MediaFileSummaryInformation;

#if  (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM) // DCOM
// This interface is only valid on Windows NT 4.0


extern RPC_IF_HANDLE __MIDL_itf_objidl_0050_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_objidl_0050_v0_0_s_ifspec;

#ifndef __IClientSecurity_INTERFACE_DEFINED__
#define __IClientSecurity_INTERFACE_DEFINED__

/* interface IClientSecurity */
/* [uuid][object][local] */ 

typedef struct tagSOLE_AUTHENTICATION_SERVICE
    {
    DWORD dwAuthnSvc;
    DWORD dwAuthzSvc;
    OLECHAR __RPC_FAR *pPrincipalName;
    HRESULT hr;
    }	SOLE_AUTHENTICATION_SERVICE;

typedef SOLE_AUTHENTICATION_SERVICE __RPC_FAR *PSOLE_AUTHENTICATION_SERVICE;

typedef 
enum tagEOLE_AUTHENTICATION_CAPABILITIES
    {	EOAC_NONE	= 0,
	EOAC_MUTUAL_AUTH	= 0x1,
	EOAC_STATIC_CLOAKING	= 0x20,
	EOAC_DYNAMIC_CLOAKING	= 0x40,
	EOAC_ANY_AUTHORITY	= 0x80,
	EOAC_MAKE_FULLSIC	= 0x100,
	EOAC_DEFAULT	= 0x800,
	EOAC_SECURE_REFS	= 0x2,
	EOAC_ACCESS_CONTROL	= 0x4,
	EOAC_APPID	= 0x8,
	EOAC_DYNAMIC	= 0x10,
	EOAC_REQUIRE_FULLSIC	= 0x200,
	EOAC_AUTO_IMPERSONATE	= 0x400,
	EOAC_NO_CUSTOM_MARSHAL	= 0x2000,
	EOAC_DISABLE_AAA	= 0x1000
    }	EOLE_AUTHENTICATION_CAPABILITIES;

#define	COLE_DEFAULT_PRINCIPAL	( ( OLECHAR __RPC_FAR * )-1 )

#define	COLE_DEFAULT_AUTHINFO	( ( void __RPC_FAR * )-1 )

typedef struct tagSOLE_AUTHENTICATION_INFO
    {
    DWORD dwAuthnSvc;
    DWORD dwAuthzSvc;
    void __RPC_FAR *pAuthInfo;
    }	SOLE_AUTHENTICATION_INFO;

typedef struct tagSOLE_AUTHENTICATION_INFO __RPC_FAR *PSOLE_AUTHENTICATION_INFO;

typedef struct tagSOLE_AUTHENTICATION_LIST
    {
    DWORD cAuthInfo;
    SOLE_AUTHENTICATION_INFO __RPC_FAR *aAuthInfo;
    }	SOLE_AUTHENTICATION_LIST;

typedef struct tagSOLE_AUTHENTICATION_LIST __RPC_FAR *PSOLE_AUTHENTICATION_LIST;


EXTERN_C const IID IID_IClientSecurity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000013D-0000-0000-C000-000000000046")
    IClientSecurity : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryBlanket( 
            /* [in] */ IUnknown __RPC_FAR *pProxy,
            /* [out] */ DWORD __RPC_FAR *pAuthnSvc,
            /* [out] */ DWORD __RPC_FAR *pAuthzSvc,
            /* [out] */ OLECHAR __RPC_FAR *__RPC_FAR *pServerPrincName,
            /* [out] */ DWORD __RPC_FAR *pAuthnLevel,
            /* [out] */ DWORD __RPC_FAR *pImpLevel,
            /* [out] */ void __RPC_FAR *__RPC_FAR *pAuthInfo,
            /* [out] */ DWORD __RPC_FAR *pCapabilites) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBlanket( 
            /* [in] */ IUnknown __RPC_FAR *pProxy,
            /* [in] */ DWORD dwAuthnSvc,
            /* [in] */ DWORD dwAuthzSvc,
            /* [in] */ OLECHAR __RPC_FAR *pServerPrincName,
            /* [in] */ DWORD dwAuthnLevel,
            /* [in] */ DWORD dwImpLevel,
            /* [in] */ void __RPC_FAR *pAuthInfo,
            /* [in] */ DWORD dwCapabilities) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyProxy( 
            /* [in] */ IUnknown __RPC_FAR *pProxy,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppCopy) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IClientSecurityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IClientSecurity __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IClientSecurity __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IClientSecurity __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryBlanket )( 
            IClientSecurity __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pProxy,
            /* [out] */ DWORD __RPC_FAR *pAuthnSvc,
            /* [out] */ DWORD __RPC_FAR *pAuthzSvc,
            /* [out] */ OLECHAR __RPC_FAR *__RPC_FAR *pServerPrincName,
            /* [out] */ DWORD __RPC_FAR *pAuthnLevel,
            /* [out] */ DWORD __RPC_FAR *pImpLevel,
            /* [out] */ void __RPC_FAR *__RPC_FAR *pAuthInfo,
            /* [out] */ DWORD __RPC_FAR *pCapabilites);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetBlanket )( 
            IClientSecurity __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pProxy,
            /* [in] */ DWORD dwAuthnSvc,
            /* [in] */ DWORD dwAuthzSvc,
            /* [in] */ OLECHAR __RPC_FAR *pServerPrincName,
            /* [in] */ DWORD dwAuthnLevel,
            /* [in] */ DWORD dwImpLevel,
            /* [in] */ void __RPC_FAR *pAuthInfo,
            /* [in] */ DWORD dwCapabilities);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CopyProxy )( 
            IClientSecurity __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pProxy,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppCopy);
        
        END_INTERFACE
    } IClientSecurityVtbl;

    interface IClientSecurity
    {
        CONST_VTBL struct IClientSecurityVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClientSecurity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClientSecurity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClientSecurity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClientSecurity_QueryBlanket(This,pProxy,pAuthnSvc,pAuthzSvc,pServerPrincName,pAuthnLevel,pImpLevel,pAuthInfo,pCapabilites)	\
    (This)->lpVtbl -> QueryBlanket(This,pProxy,pAuthnSvc,pAuthzSvc,pServerPrincName,pAuthnLevel,pImpLevel,pAuthInfo,pCapabilites)

#define IClientSecurity_SetBlanket(This,pProxy,dwAuthnSvc,dwAuthzSvc,pServerPrincName,dwAuthnLevel,dwImpLevel,pAuthInfo,dwCapabilities)	\
    (This)->lpVtbl -> SetBlanket(This,pProxy,dwAuthnSvc,dwAuthzSvc,pServerPrincName,dwAuthnLevel,dwImpLevel,pAuthInfo,dwCapabilities)

#define IClientSecurity_CopyProxy(This,pProxy,ppCopy)	\
    (This)->lpVtbl -> CopyProxy(This,pProxy,ppCopy)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IClientSecurity_QueryBlanket_Proxy( 
    IClientSecurity __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pProxy,
    /* [out] */ DWORD __RPC_FAR *pAuthnSvc,
    /* [out] */ DWORD __RPC_FAR *pAuthzSvc,
    /* [out] */ OLECHAR __RPC_FAR *__RPC_FAR *pServerPrincName,
    /* [out] */ DWORD __RPC_FAR *pAuthnLevel,
    /* [out] */ DWORD __RPC_FAR *pImpLevel,
    /* [out] */ void __RPC_FAR *__RPC_FAR *pAuthInfo,
    /* [out] */ DWORD __RPC_FAR *pCapabilites);


void __RPC_STUB IClientSecurity_QueryBlanket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSecurity_SetBlanket_Proxy( 
    IClientSecurity __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pProxy,
    /* [in] */ DWORD dwAuthnSvc,
    /* [in] */ DWORD dwAuthzSvc,
    /* [in] */ OLECHAR __RPC_FAR *pServerPrincName,
    /* [in] */ DWORD dwAuthnLevel,
    /* [in] */ DWORD dwImpLevel,
    /* [in] */ void __RPC_FAR *pAuthInfo,
    /* [in] */ DWORD dwCapabilities);


void __RPC_STUB IClientSecurity_SetBlanket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSecurity_CopyProxy_Proxy( 
    IClientSecurity __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pProxy,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppCopy);


void __RPC_STUB IClientSecurity_CopyProxy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IClientSecurity_INTERFACE_DEFINED__ */


#ifndef __IServerSecurity_INTERFACE_DEFINED__
#define __IServerSecurity_INTERFACE_DEFINED__

/* interface IServerSecurity */
/* [uuid][object][local] */ 


EXTERN_C const IID IID_IServerSecurity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000013E-0000-0000-C000-000000000046")
    IServerSecurity : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryBlanket( 
            /* [out] */ DWORD __RPC_FAR *pAuthnSvc,
            /* [out] */ DWORD __RPC_FAR *pAuthzSvc,
            /* [out] */ OLECHAR __RPC_FAR *__RPC_FAR *pServerPrincName,
            /* [out] */ DWORD __RPC_FAR *pAuthnLevel,
            /* [out] */ DWORD __RPC_FAR *pImpLevel,
            /* [out] */ void __RPC_FAR *__RPC_FAR *pPrivs,
            /* [out][in] */ DWORD __RPC_FAR *pCapabilities) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ImpersonateClient( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevertToSelf( void) = 0;
        
        virtual BOOL STDMETHODCALLTYPE IsImpersonating( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IServerSecurityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IServerSecurity __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IServerSecurity __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IServerSecurity __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryBlanket )( 
            IServerSecurity __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pAuthnSvc,
            /* [out] */ DWORD __RPC_FAR *pAuthzSvc,
            /* [out] */ OLECHAR __RPC_FAR *__RPC_FAR *pServerPrincName,
            /* [out] */ DWORD __RPC_FAR *pAuthnLevel,
            /* [out] */ DWORD __RPC_FAR *pImpLevel,
            /* [out] */ void __RPC_FAR *__RPC_FAR *pPrivs,
            /* [out][in] */ DWORD __RPC_FAR *pCapabilities);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ImpersonateClient )( 
            IServerSecurity __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RevertToSelf )( 
            IServerSecurity __RPC_FAR * This);
        
        BOOL ( STDMETHODCALLTYPE __RPC_FAR *IsImpersonating )( 
            IServerSecurity __RPC_FAR * This);
        
        END_INTERFACE
    } IServerSecurityVtbl;

    interface IServerSecurity
    {
        CONST_VTBL struct IServerSecurityVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServerSecurity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServerSecurity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServerSecurity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServerSecurity_QueryBlanket(This,pAuthnSvc,pAuthzSvc,pServerPrincName,pAuthnLevel,pImpLevel,pPrivs,pCapabilities)	\
    (This)->lpVtbl -> QueryBlanket(This,pAuthnSvc,pAuthzSvc,pServerPrincName,pAuthnLevel,pImpLevel,pPrivs,pCapabilities)

#define IServerSecurity_ImpersonateClient(This)	\
    (This)->lpVtbl -> ImpersonateClient(This)

#define IServerSecurity_RevertToSelf(This)	\
    (This)->lpVtbl -> RevertToSelf(This)

#define IServerSecurity_IsImpersonating(This)	\
    (This)->lpVtbl -> IsImpersonating(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IServerSecurity_QueryBlanket_Proxy( 
    IServerSecurity __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pAuthnSvc,
    /* [out] */ DWORD __RPC_FAR *pAuthzSvc,
    /* [out] */ OLECHAR __RPC_FAR *__RPC_FAR *pServerPrincName,
    /* [out] */ DWORD __RPC_FAR *pAuthnLevel,
    /* [out] */ DWORD __RPC_FAR *pImpLevel,
    /* [out] */ void __RPC_FAR *__RPC_FAR *pPrivs,
    /* [out][in] */ DWORD __RPC_FAR *pCapabilities);


void __RPC_STUB IServerSecurity_QueryBlanket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerSecurity_ImpersonateClient_Proxy( 
    IServerSecurity __RPC_FAR * This);


void __RPC_STUB IServerSecurity_ImpersonateClient_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerSecurity_RevertToSelf_Proxy( 
    IServerSecurity __RPC_FAR * This);


void __RPC_STUB IServerSecurity_RevertToSelf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IServerSecurity_IsImpersonating_Proxy( 
    IServerSecurity __RPC_FAR * This);


void __RPC_STUB IServerSecurity_IsImpersonating_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IServerSecurity_INTERFACE_DEFINED__ */


#ifndef __IClassActivator_INTERFACE_DEFINED__
#define __IClassActivator_INTERFACE_DEFINED__

/* interface IClassActivator */
/* [uuid][object] */ 


EXTERN_C const IID IID_IClassActivator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000140-0000-0000-C000-000000000046")
    IClassActivator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetClassObject( 
            /* [in] */ REFCLSID rclsid,
            /* [in] */ DWORD dwClassContext,
            /* [in] */ LCID locale,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IClassActivatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IClassActivator __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IClassActivator __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IClassActivator __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClassObject )( 
            IClassActivator __RPC_FAR * This,
            /* [in] */ REFCLSID rclsid,
            /* [in] */ DWORD dwClassContext,
            /* [in] */ LCID locale,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        END_INTERFACE
    } IClassActivatorVtbl;

    interface IClassActivator
    {
        CONST_VTBL struct IClassActivatorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClassActivator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClassActivator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClassActivator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClassActivator_GetClassObject(This,rclsid,dwClassContext,locale,riid,ppv)	\
    (This)->lpVtbl -> GetClassObject(This,rclsid,dwClassContext,locale,riid,ppv)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IClassActivator_GetClassObject_Proxy( 
    IClassActivator __RPC_FAR * This,
    /* [in] */ REFCLSID rclsid,
    /* [in] */ DWORD dwClassContext,
    /* [in] */ LCID locale,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB IClassActivator_GetClassObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IClassActivator_INTERFACE_DEFINED__ */


#ifndef __IRpcOptions_INTERFACE_DEFINED__
#define __IRpcOptions_INTERFACE_DEFINED__

/* interface IRpcOptions */
/* [uuid][local][object] */ 


EXTERN_C const IID IID_IRpcOptions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000144-0000-0000-C000-000000000046")
    IRpcOptions : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Set( 
            /* [in] */ IUnknown __RPC_FAR *pPrx,
            /* [in] */ DWORD dwProperty,
            /* [in] */ ULONG_PTR dwValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Query( 
            /* [in] */ IUnknown __RPC_FAR *pPrx,
            /* [in] */ DWORD dwProperty,
            /* [out] */ ULONG_PTR __RPC_FAR *pdwValue) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRpcOptionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRpcOptions __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRpcOptions __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRpcOptions __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Set )( 
            IRpcOptions __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pPrx,
            /* [in] */ DWORD dwProperty,
            /* [in] */ ULONG_PTR dwValue);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Query )( 
            IRpcOptions __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pPrx,
            /* [in] */ DWORD dwProperty,
            /* [out] */ ULONG_PTR __RPC_FAR *pdwValue);
        
        END_INTERFACE
    } IRpcOptionsVtbl;

    interface IRpcOptions
    {
        CONST_VTBL struct IRpcOptionsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcOptions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcOptions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcOptions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcOptions_Set(This,pPrx,dwProperty,dwValue)	\
    (This)->lpVtbl -> Set(This,pPrx,dwProperty,dwValue)

#define IRpcOptions_Query(This,pPrx,dwProperty,pdwValue)	\
    (This)->lpVtbl -> Query(This,pPrx,dwProperty,pdwValue)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRpcOptions_Set_Proxy( 
    IRpcOptions __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pPrx,
    /* [in] */ DWORD dwProperty,
    /* [in] */ ULONG_PTR dwValue);


void __RPC_STUB IRpcOptions_Set_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcOptions_Query_Proxy( 
    IRpcOptions __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pPrx,
    /* [in] */ DWORD dwProperty,
    /* [out] */ ULONG_PTR __RPC_FAR *pdwValue);


void __RPC_STUB IRpcOptions_Query_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRpcOptions_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_objidl_0054 */
/* [local] */ 


enum __MIDL___MIDL_itf_objidl_0054_0001
    {	COMBND_RPCTIMEOUT	= 0x1
    };
#endif //DCOM


extern RPC_IF_HANDLE __MIDL_itf_objidl_0054_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_objidl_0054_v0_0_s_ifspec;

#ifndef __IFillLockBytes_INTERFACE_DEFINED__
#define __IFillLockBytes_INTERFACE_DEFINED__

/* interface IFillLockBytes */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IFillLockBytes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("99caf010-415e-11cf-8814-00aa00b569f5")
    IFillLockBytes : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE FillAppend( 
            /* [size_is][in] */ const void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE FillAt( 
            /* [in] */ ULARGE_INTEGER ulOffset,
            /* [size_is][in] */ const void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFillSize( 
            /* [in] */ ULARGE_INTEGER ulSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Terminate( 
            /* [in] */ BOOL bCanceled) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IFillLockBytesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IFillLockBytes __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IFillLockBytes __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IFillLockBytes __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FillAppend )( 
            IFillLockBytes __RPC_FAR * This,
            /* [size_is][in] */ const void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FillAt )( 
            IFillLockBytes __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER ulOffset,
            /* [size_is][in] */ const void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetFillSize )( 
            IFillLockBytes __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER ulSize);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Terminate )( 
            IFillLockBytes __RPC_FAR * This,
            /* [in] */ BOOL bCanceled);
        
        END_INTERFACE
    } IFillLockBytesVtbl;

    interface IFillLockBytes
    {
        CONST_VTBL struct IFillLockBytesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFillLockBytes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFillLockBytes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFillLockBytes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFillLockBytes_FillAppend(This,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> FillAppend(This,pv,cb,pcbWritten)

#define IFillLockBytes_FillAt(This,ulOffset,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> FillAt(This,ulOffset,pv,cb,pcbWritten)

#define IFillLockBytes_SetFillSize(This,ulSize)	\
    (This)->lpVtbl -> SetFillSize(This,ulSize)

#define IFillLockBytes_Terminate(This,bCanceled)	\
    (This)->lpVtbl -> Terminate(This,bCanceled)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IFillLockBytes_RemoteFillAppend_Proxy( 
    IFillLockBytes __RPC_FAR * This,
    /* [size_is][in] */ const byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);


void __RPC_STUB IFillLockBytes_RemoteFillAppend_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT __stdcall IFillLockBytes_RemoteFillAt_Proxy( 
    IFillLockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER ulOffset,
    /* [size_is][in] */ const byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);


void __RPC_STUB IFillLockBytes_RemoteFillAt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFillLockBytes_SetFillSize_Proxy( 
    IFillLockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER ulSize);


void __RPC_STUB IFillLockBytes_SetFillSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFillLockBytes_Terminate_Proxy( 
    IFillLockBytes __RPC_FAR * This,
    /* [in] */ BOOL bCanceled);


void __RPC_STUB IFillLockBytes_Terminate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IFillLockBytes_INTERFACE_DEFINED__ */


#ifndef __IProgressNotify_INTERFACE_DEFINED__
#define __IProgressNotify_INTERFACE_DEFINED__

/* interface IProgressNotify */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IProgressNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a9d758a0-4617-11cf-95fc-00aa00680db4")
    IProgressNotify : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnProgress( 
            /* [in] */ DWORD dwProgressCurrent,
            /* [in] */ DWORD dwProgressMaximum,
            /* [in] */ BOOL fAccurate,
            /* [in] */ BOOL fOwner) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IProgressNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IProgressNotify __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IProgressNotify __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IProgressNotify __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnProgress )( 
            IProgressNotify __RPC_FAR * This,
            /* [in] */ DWORD dwProgressCurrent,
            /* [in] */ DWORD dwProgressMaximum,
            /* [in] */ BOOL fAccurate,
            /* [in] */ BOOL fOwner);
        
        END_INTERFACE
    } IProgressNotifyVtbl;

    interface IProgressNotify
    {
        CONST_VTBL struct IProgressNotifyVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProgressNotify_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProgressNotify_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProgressNotify_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProgressNotify_OnProgress(This,dwProgressCurrent,dwProgressMaximum,fAccurate,fOwner)	\
    (This)->lpVtbl -> OnProgress(This,dwProgressCurrent,dwProgressMaximum,fAccurate,fOwner)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IProgressNotify_OnProgress_Proxy( 
    IProgressNotify __RPC_FAR * This,
    /* [in] */ DWORD dwProgressCurrent,
    /* [in] */ DWORD dwProgressMaximum,
    /* [in] */ BOOL fAccurate,
    /* [in] */ BOOL fOwner);


void __RPC_STUB IProgressNotify_OnProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IProgressNotify_INTERFACE_DEFINED__ */


#ifndef __ILayoutStorage_INTERFACE_DEFINED__
#define __ILayoutStorage_INTERFACE_DEFINED__

/* interface ILayoutStorage */
/* [unique][uuid][object][local] */ 

typedef struct tagStorageLayout
    {
    DWORD LayoutType;
    OLECHAR __RPC_FAR *pwcsElementName;
    LARGE_INTEGER cOffset;
    LARGE_INTEGER cBytes;
    }	StorageLayout;


EXTERN_C const IID IID_ILayoutStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0e6d4d90-6738-11cf-9608-00aa00680db4")
    ILayoutStorage : public IUnknown
    {
    public:
        virtual HRESULT __stdcall LayoutScript( 
            /* [in] */ StorageLayout __RPC_FAR *pStorageLayout,
            /* [in] */ DWORD nEntries,
            /* [in] */ DWORD glfInterleavedFlag) = 0;
        
        virtual HRESULT __stdcall BeginMonitor( void) = 0;
        
        virtual HRESULT __stdcall EndMonitor( void) = 0;
        
        virtual HRESULT __stdcall ReLayoutDocfile( 
            /* [in] */ OLECHAR __RPC_FAR *pwcsNewDfName) = 0;
        
        virtual HRESULT __stdcall ReLayoutDocfileOnILockBytes( 
            /* [in] */ ILockBytes __RPC_FAR *pILockBytes) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ILayoutStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ILayoutStorage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ILayoutStorage __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ILayoutStorage __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *LayoutScript )( 
            ILayoutStorage __RPC_FAR * This,
            /* [in] */ StorageLayout __RPC_FAR *pStorageLayout,
            /* [in] */ DWORD nEntries,
            /* [in] */ DWORD glfInterleavedFlag);
        
        HRESULT ( __stdcall __RPC_FAR *BeginMonitor )( 
            ILayoutStorage __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *EndMonitor )( 
            ILayoutStorage __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *ReLayoutDocfile )( 
            ILayoutStorage __RPC_FAR * This,
            /* [in] */ OLECHAR __RPC_FAR *pwcsNewDfName);
        
        HRESULT ( __stdcall __RPC_FAR *ReLayoutDocfileOnILockBytes )( 
            ILayoutStorage __RPC_FAR * This,
            /* [in] */ ILockBytes __RPC_FAR *pILockBytes);
        
        END_INTERFACE
    } ILayoutStorageVtbl;

    interface ILayoutStorage
    {
        CONST_VTBL struct ILayoutStorageVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILayoutStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILayoutStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILayoutStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILayoutStorage_LayoutScript(This,pStorageLayout,nEntries,glfInterleavedFlag)	\
    (This)->lpVtbl -> LayoutScript(This,pStorageLayout,nEntries,glfInterleavedFlag)

#define ILayoutStorage_BeginMonitor(This)	\
    (This)->lpVtbl -> BeginMonitor(This)

#define ILayoutStorage_EndMonitor(This)	\
    (This)->lpVtbl -> EndMonitor(This)

#define ILayoutStorage_ReLayoutDocfile(This,pwcsNewDfName)	\
    (This)->lpVtbl -> ReLayoutDocfile(This,pwcsNewDfName)

#define ILayoutStorage_ReLayoutDocfileOnILockBytes(This,pILockBytes)	\
    (This)->lpVtbl -> ReLayoutDocfileOnILockBytes(This,pILockBytes)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall ILayoutStorage_LayoutScript_Proxy( 
    ILayoutStorage __RPC_FAR * This,
    /* [in] */ StorageLayout __RPC_FAR *pStorageLayout,
    /* [in] */ DWORD nEntries,
    /* [in] */ DWORD glfInterleavedFlag);


void __RPC_STUB ILayoutStorage_LayoutScript_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ILayoutStorage_BeginMonitor_Proxy( 
    ILayoutStorage __RPC_FAR * This);


void __RPC_STUB ILayoutStorage_BeginMonitor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ILayoutStorage_EndMonitor_Proxy( 
    ILayoutStorage __RPC_FAR * This);


void __RPC_STUB ILayoutStorage_EndMonitor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ILayoutStorage_ReLayoutDocfile_Proxy( 
    ILayoutStorage __RPC_FAR * This,
    /* [in] */ OLECHAR __RPC_FAR *pwcsNewDfName);


void __RPC_STUB ILayoutStorage_ReLayoutDocfile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ILayoutStorage_ReLayoutDocfileOnILockBytes_Proxy( 
    ILayoutStorage __RPC_FAR * This,
    /* [in] */ ILockBytes __RPC_FAR *pILockBytes);


void __RPC_STUB ILayoutStorage_ReLayoutDocfileOnILockBytes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ILayoutStorage_INTERFACE_DEFINED__ */


#ifndef __IBlockingLock_INTERFACE_DEFINED__
#define __IBlockingLock_INTERFACE_DEFINED__

/* interface IBlockingLock */
/* [uuid][object] */ 


EXTERN_C const IID IID_IBlockingLock;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("30f3d47a-6447-11d1-8e3c-00c04fb9386d")
    IBlockingLock : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Lock( 
            /* [in] */ DWORD dwTimeout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unlock( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBlockingLockVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBlockingLock __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBlockingLock __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBlockingLock __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Lock )( 
            IBlockingLock __RPC_FAR * This,
            /* [in] */ DWORD dwTimeout);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Unlock )( 
            IBlockingLock __RPC_FAR * This);
        
        END_INTERFACE
    } IBlockingLockVtbl;

    interface IBlockingLock
    {
        CONST_VTBL struct IBlockingLockVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBlockingLock_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBlockingLock_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBlockingLock_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBlockingLock_Lock(This,dwTimeout)	\
    (This)->lpVtbl -> Lock(This,dwTimeout)

#define IBlockingLock_Unlock(This)	\
    (This)->lpVtbl -> Unlock(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IBlockingLock_Lock_Proxy( 
    IBlockingLock __RPC_FAR * This,
    /* [in] */ DWORD dwTimeout);


void __RPC_STUB IBlockingLock_Lock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBlockingLock_Unlock_Proxy( 
    IBlockingLock __RPC_FAR * This);


void __RPC_STUB IBlockingLock_Unlock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBlockingLock_INTERFACE_DEFINED__ */


#ifndef __ITimeAndNoticeControl_INTERFACE_DEFINED__
#define __ITimeAndNoticeControl_INTERFACE_DEFINED__

/* interface ITimeAndNoticeControl */
/* [uuid][object] */ 


EXTERN_C const IID IID_ITimeAndNoticeControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("bc0bf6ae-8878-11d1-83e9-00c04fc2c6d4")
    ITimeAndNoticeControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SuppressChanges( 
            /* [in] */ DWORD res1,
            /* [in] */ DWORD res2) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITimeAndNoticeControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITimeAndNoticeControl __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITimeAndNoticeControl __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITimeAndNoticeControl __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SuppressChanges )( 
            ITimeAndNoticeControl __RPC_FAR * This,
            /* [in] */ DWORD res1,
            /* [in] */ DWORD res2);
        
        END_INTERFACE
    } ITimeAndNoticeControlVtbl;

    interface ITimeAndNoticeControl
    {
        CONST_VTBL struct ITimeAndNoticeControlVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITimeAndNoticeControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITimeAndNoticeControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITimeAndNoticeControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITimeAndNoticeControl_SuppressChanges(This,res1,res2)	\
    (This)->lpVtbl -> SuppressChanges(This,res1,res2)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITimeAndNoticeControl_SuppressChanges_Proxy( 
    ITimeAndNoticeControl __RPC_FAR * This,
    /* [in] */ DWORD res1,
    /* [in] */ DWORD res2);


void __RPC_STUB ITimeAndNoticeControl_SuppressChanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITimeAndNoticeControl_INTERFACE_DEFINED__ */


#ifndef __IOplockStorage_INTERFACE_DEFINED__
#define __IOplockStorage_INTERFACE_DEFINED__

/* interface IOplockStorage */
/* [uuid][object] */ 


EXTERN_C const IID IID_IOplockStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8d19c834-8879-11d1-83e9-00c04fc2c6d4")
    IOplockStorage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateStorageEx( 
            /* [in] */ LPCWSTR pwcsName,
            /* [in] */ DWORD grfMode,
            /* [in] */ DWORD stgfmt,
            /* [in] */ DWORD grfAttrs,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppstgOpen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenStorageEx( 
            /* [in] */ LPCWSTR pwcsName,
            /* [in] */ DWORD grfMode,
            /* [in] */ DWORD stgfmt,
            /* [in] */ DWORD grfAttrs,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppstgOpen) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOplockStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IOplockStorage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IOplockStorage __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IOplockStorage __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateStorageEx )( 
            IOplockStorage __RPC_FAR * This,
            /* [in] */ LPCWSTR pwcsName,
            /* [in] */ DWORD grfMode,
            /* [in] */ DWORD stgfmt,
            /* [in] */ DWORD grfAttrs,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppstgOpen);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenStorageEx )( 
            IOplockStorage __RPC_FAR * This,
            /* [in] */ LPCWSTR pwcsName,
            /* [in] */ DWORD grfMode,
            /* [in] */ DWORD stgfmt,
            /* [in] */ DWORD grfAttrs,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppstgOpen);
        
        END_INTERFACE
    } IOplockStorageVtbl;

    interface IOplockStorage
    {
        CONST_VTBL struct IOplockStorageVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOplockStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOplockStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOplockStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOplockStorage_CreateStorageEx(This,pwcsName,grfMode,stgfmt,grfAttrs,riid,ppstgOpen)	\
    (This)->lpVtbl -> CreateStorageEx(This,pwcsName,grfMode,stgfmt,grfAttrs,riid,ppstgOpen)

#define IOplockStorage_OpenStorageEx(This,pwcsName,grfMode,stgfmt,grfAttrs,riid,ppstgOpen)	\
    (This)->lpVtbl -> OpenStorageEx(This,pwcsName,grfMode,stgfmt,grfAttrs,riid,ppstgOpen)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IOplockStorage_CreateStorageEx_Proxy( 
    IOplockStorage __RPC_FAR * This,
    /* [in] */ LPCWSTR pwcsName,
    /* [in] */ DWORD grfMode,
    /* [in] */ DWORD stgfmt,
    /* [in] */ DWORD grfAttrs,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppstgOpen);


void __RPC_STUB IOplockStorage_CreateStorageEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOplockStorage_OpenStorageEx_Proxy( 
    IOplockStorage __RPC_FAR * This,
    /* [in] */ LPCWSTR pwcsName,
    /* [in] */ DWORD grfMode,
    /* [in] */ DWORD stgfmt,
    /* [in] */ DWORD grfAttrs,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppstgOpen);


void __RPC_STUB IOplockStorage_OpenStorageEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOplockStorage_INTERFACE_DEFINED__ */


#ifndef __ISurrogate_INTERFACE_DEFINED__
#define __ISurrogate_INTERFACE_DEFINED__

/* interface ISurrogate */
/* [object][unique][version][uuid] */ 

typedef /* [unique] */ ISurrogate __RPC_FAR *LPSURROGATE;


EXTERN_C const IID IID_ISurrogate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000022-0000-0000-C000-000000000046")
    ISurrogate : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE LoadDllServer( 
            /* [in] */ REFCLSID Clsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeSurrogate( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISurrogateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISurrogate __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISurrogate __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISurrogate __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LoadDllServer )( 
            ISurrogate __RPC_FAR * This,
            /* [in] */ REFCLSID Clsid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FreeSurrogate )( 
            ISurrogate __RPC_FAR * This);
        
        END_INTERFACE
    } ISurrogateVtbl;

    interface ISurrogate
    {
        CONST_VTBL struct ISurrogateVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISurrogate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISurrogate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISurrogate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISurrogate_LoadDllServer(This,Clsid)	\
    (This)->lpVtbl -> LoadDllServer(This,Clsid)

#define ISurrogate_FreeSurrogate(This)	\
    (This)->lpVtbl -> FreeSurrogate(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ISurrogate_LoadDllServer_Proxy( 
    ISurrogate __RPC_FAR * This,
    /* [in] */ REFCLSID Clsid);


void __RPC_STUB ISurrogate_LoadDllServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISurrogate_FreeSurrogate_Proxy( 
    ISurrogate __RPC_FAR * This);


void __RPC_STUB ISurrogate_FreeSurrogate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISurrogate_INTERFACE_DEFINED__ */


#ifndef __IGlobalInterfaceTable_INTERFACE_DEFINED__
#define __IGlobalInterfaceTable_INTERFACE_DEFINED__

/* interface IGlobalInterfaceTable */
/* [uuid][object][local] */ 

typedef /* [unique] */ IGlobalInterfaceTable __RPC_FAR *LPGLOBALINTERFACETABLE;


EXTERN_C const IID IID_IGlobalInterfaceTable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000146-0000-0000-C000-000000000046")
    IGlobalInterfaceTable : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterInterfaceInGlobal( 
            /* [in] */ IUnknown __RPC_FAR *pUnk,
            /* [in] */ REFIID riid,
            /* [out] */ DWORD __RPC_FAR *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevokeInterfaceFromGlobal( 
            /* [in] */ DWORD dwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInterfaceFromGlobal( 
            /* [in] */ DWORD dwCookie,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGlobalInterfaceTableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IGlobalInterfaceTable __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IGlobalInterfaceTable __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IGlobalInterfaceTable __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RegisterInterfaceInGlobal )( 
            IGlobalInterfaceTable __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnk,
            /* [in] */ REFIID riid,
            /* [out] */ DWORD __RPC_FAR *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RevokeInterfaceFromGlobal )( 
            IGlobalInterfaceTable __RPC_FAR * This,
            /* [in] */ DWORD dwCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInterfaceFromGlobal )( 
            IGlobalInterfaceTable __RPC_FAR * This,
            /* [in] */ DWORD dwCookie,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        END_INTERFACE
    } IGlobalInterfaceTableVtbl;

    interface IGlobalInterfaceTable
    {
        CONST_VTBL struct IGlobalInterfaceTableVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGlobalInterfaceTable_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGlobalInterfaceTable_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGlobalInterfaceTable_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGlobalInterfaceTable_RegisterInterfaceInGlobal(This,pUnk,riid,pdwCookie)	\
    (This)->lpVtbl -> RegisterInterfaceInGlobal(This,pUnk,riid,pdwCookie)

#define IGlobalInterfaceTable_RevokeInterfaceFromGlobal(This,dwCookie)	\
    (This)->lpVtbl -> RevokeInterfaceFromGlobal(This,dwCookie)

#define IGlobalInterfaceTable_GetInterfaceFromGlobal(This,dwCookie,riid,ppv)	\
    (This)->lpVtbl -> GetInterfaceFromGlobal(This,dwCookie,riid,ppv)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IGlobalInterfaceTable_RegisterInterfaceInGlobal_Proxy( 
    IGlobalInterfaceTable __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnk,
    /* [in] */ REFIID riid,
    /* [out] */ DWORD __RPC_FAR *pdwCookie);


void __RPC_STUB IGlobalInterfaceTable_RegisterInterfaceInGlobal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGlobalInterfaceTable_RevokeInterfaceFromGlobal_Proxy( 
    IGlobalInterfaceTable __RPC_FAR * This,
    /* [in] */ DWORD dwCookie);


void __RPC_STUB IGlobalInterfaceTable_RevokeInterfaceFromGlobal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGlobalInterfaceTable_GetInterfaceFromGlobal_Proxy( 
    IGlobalInterfaceTable __RPC_FAR * This,
    /* [in] */ DWORD dwCookie,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB IGlobalInterfaceTable_GetInterfaceFromGlobal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGlobalInterfaceTable_INTERFACE_DEFINED__ */


#ifndef __IDirectWriterLock_INTERFACE_DEFINED__
#define __IDirectWriterLock_INTERFACE_DEFINED__

/* interface IDirectWriterLock */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDirectWriterLock;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0e6d4d92-6738-11cf-9608-00aa00680db4")
    IDirectWriterLock : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE WaitForWriteAccess( 
            /* [in] */ DWORD dwTimeout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseWriteAccess( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HaveWriteAccess( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDirectWriterLockVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDirectWriterLock __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDirectWriterLock __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDirectWriterLock __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WaitForWriteAccess )( 
            IDirectWriterLock __RPC_FAR * This,
            /* [in] */ DWORD dwTimeout);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseWriteAccess )( 
            IDirectWriterLock __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HaveWriteAccess )( 
            IDirectWriterLock __RPC_FAR * This);
        
        END_INTERFACE
    } IDirectWriterLockVtbl;

    interface IDirectWriterLock
    {
        CONST_VTBL struct IDirectWriterLockVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirectWriterLock_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirectWriterLock_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirectWriterLock_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirectWriterLock_WaitForWriteAccess(This,dwTimeout)	\
    (This)->lpVtbl -> WaitForWriteAccess(This,dwTimeout)

#define IDirectWriterLock_ReleaseWriteAccess(This)	\
    (This)->lpVtbl -> ReleaseWriteAccess(This)

#define IDirectWriterLock_HaveWriteAccess(This)	\
    (This)->lpVtbl -> HaveWriteAccess(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDirectWriterLock_WaitForWriteAccess_Proxy( 
    IDirectWriterLock __RPC_FAR * This,
    /* [in] */ DWORD dwTimeout);


void __RPC_STUB IDirectWriterLock_WaitForWriteAccess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectWriterLock_ReleaseWriteAccess_Proxy( 
    IDirectWriterLock __RPC_FAR * This);


void __RPC_STUB IDirectWriterLock_ReleaseWriteAccess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectWriterLock_HaveWriteAccess_Proxy( 
    IDirectWriterLock __RPC_FAR * This);


void __RPC_STUB IDirectWriterLock_HaveWriteAccess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDirectWriterLock_INTERFACE_DEFINED__ */


#ifndef __ISynchronize_INTERFACE_DEFINED__
#define __ISynchronize_INTERFACE_DEFINED__

/* interface ISynchronize */
/* [uuid][object] */ 


EXTERN_C const IID IID_ISynchronize;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000030-0000-0000-C000-000000000046")
    ISynchronize : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Wait( 
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwMilliseconds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Signal( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISynchronizeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISynchronize __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISynchronize __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISynchronize __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Wait )( 
            ISynchronize __RPC_FAR * This,
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwMilliseconds);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Signal )( 
            ISynchronize __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            ISynchronize __RPC_FAR * This);
        
        END_INTERFACE
    } ISynchronizeVtbl;

    interface ISynchronize
    {
        CONST_VTBL struct ISynchronizeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISynchronize_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISynchronize_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISynchronize_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISynchronize_Wait(This,dwFlags,dwMilliseconds)	\
    (This)->lpVtbl -> Wait(This,dwFlags,dwMilliseconds)

#define ISynchronize_Signal(This)	\
    (This)->lpVtbl -> Signal(This)

#define ISynchronize_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ISynchronize_Wait_Proxy( 
    ISynchronize __RPC_FAR * This,
    /* [in] */ DWORD dwFlags,
    /* [in] */ DWORD dwMilliseconds);


void __RPC_STUB ISynchronize_Wait_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISynchronize_Signal_Proxy( 
    ISynchronize __RPC_FAR * This);


void __RPC_STUB ISynchronize_Signal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISynchronize_Reset_Proxy( 
    ISynchronize __RPC_FAR * This);


void __RPC_STUB ISynchronize_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISynchronize_INTERFACE_DEFINED__ */


#ifndef __ISynchronizeHandle_INTERFACE_DEFINED__
#define __ISynchronizeHandle_INTERFACE_DEFINED__

/* interface ISynchronizeHandle */
/* [uuid][object][local] */ 


EXTERN_C const IID IID_ISynchronizeHandle;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000031-0000-0000-C000-000000000046")
    ISynchronizeHandle : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetHandle( 
            /* [out] */ HANDLE __RPC_FAR *ph) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISynchronizeHandleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISynchronizeHandle __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISynchronizeHandle __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISynchronizeHandle __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetHandle )( 
            ISynchronizeHandle __RPC_FAR * This,
            /* [out] */ HANDLE __RPC_FAR *ph);
        
        END_INTERFACE
    } ISynchronizeHandleVtbl;

    interface ISynchronizeHandle
    {
        CONST_VTBL struct ISynchronizeHandleVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISynchronizeHandle_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISynchronizeHandle_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISynchronizeHandle_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISynchronizeHandle_GetHandle(This,ph)	\
    (This)->lpVtbl -> GetHandle(This,ph)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ISynchronizeHandle_GetHandle_Proxy( 
    ISynchronizeHandle __RPC_FAR * This,
    /* [out] */ HANDLE __RPC_FAR *ph);


void __RPC_STUB ISynchronizeHandle_GetHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISynchronizeHandle_INTERFACE_DEFINED__ */


#ifndef __ISynchronizeEvent_INTERFACE_DEFINED__
#define __ISynchronizeEvent_INTERFACE_DEFINED__

/* interface ISynchronizeEvent */
/* [uuid][object][local] */ 


EXTERN_C const IID IID_ISynchronizeEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000032-0000-0000-C000-000000000046")
    ISynchronizeEvent : public ISynchronizeHandle
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetEventHandle( 
            /* [in] */ HANDLE __RPC_FAR *ph) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISynchronizeEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISynchronizeEvent __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISynchronizeEvent __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISynchronizeEvent __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetHandle )( 
            ISynchronizeEvent __RPC_FAR * This,
            /* [out] */ HANDLE __RPC_FAR *ph);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetEventHandle )( 
            ISynchronizeEvent __RPC_FAR * This,
            /* [in] */ HANDLE __RPC_FAR *ph);
        
        END_INTERFACE
    } ISynchronizeEventVtbl;

    interface ISynchronizeEvent
    {
        CONST_VTBL struct ISynchronizeEventVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISynchronizeEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISynchronizeEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISynchronizeEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISynchronizeEvent_GetHandle(This,ph)	\
    (This)->lpVtbl -> GetHandle(This,ph)


#define ISynchronizeEvent_SetEventHandle(This,ph)	\
    (This)->lpVtbl -> SetEventHandle(This,ph)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ISynchronizeEvent_SetEventHandle_Proxy( 
    ISynchronizeEvent __RPC_FAR * This,
    /* [in] */ HANDLE __RPC_FAR *ph);


void __RPC_STUB ISynchronizeEvent_SetEventHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISynchronizeEvent_INTERFACE_DEFINED__ */


#ifndef __ISynchronizeContainer_INTERFACE_DEFINED__
#define __ISynchronizeContainer_INTERFACE_DEFINED__

/* interface ISynchronizeContainer */
/* [uuid][object][local] */ 


EXTERN_C const IID IID_ISynchronizeContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000033-0000-0000-C000-000000000046")
    ISynchronizeContainer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddSynchronize( 
            /* [in] */ ISynchronize __RPC_FAR *pSync) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WaitMultiple( 
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwTimeOut,
            /* [out] */ ISynchronize __RPC_FAR *__RPC_FAR *ppSync) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISynchronizeContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISynchronizeContainer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISynchronizeContainer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISynchronizeContainer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddSynchronize )( 
            ISynchronizeContainer __RPC_FAR * This,
            /* [in] */ ISynchronize __RPC_FAR *pSync);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WaitMultiple )( 
            ISynchronizeContainer __RPC_FAR * This,
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwTimeOut,
            /* [out] */ ISynchronize __RPC_FAR *__RPC_FAR *ppSync);
        
        END_INTERFACE
    } ISynchronizeContainerVtbl;

    interface ISynchronizeContainer
    {
        CONST_VTBL struct ISynchronizeContainerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISynchronizeContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISynchronizeContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISynchronizeContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISynchronizeContainer_AddSynchronize(This,pSync)	\
    (This)->lpVtbl -> AddSynchronize(This,pSync)

#define ISynchronizeContainer_WaitMultiple(This,dwFlags,dwTimeOut,ppSync)	\
    (This)->lpVtbl -> WaitMultiple(This,dwFlags,dwTimeOut,ppSync)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ISynchronizeContainer_AddSynchronize_Proxy( 
    ISynchronizeContainer __RPC_FAR * This,
    /* [in] */ ISynchronize __RPC_FAR *pSync);


void __RPC_STUB ISynchronizeContainer_AddSynchronize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISynchronizeContainer_WaitMultiple_Proxy( 
    ISynchronizeContainer __RPC_FAR * This,
    /* [in] */ DWORD dwFlags,
    /* [in] */ DWORD dwTimeOut,
    /* [out] */ ISynchronize __RPC_FAR *__RPC_FAR *ppSync);


void __RPC_STUB ISynchronizeContainer_WaitMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISynchronizeContainer_INTERFACE_DEFINED__ */


#ifndef __ISynchronizeMutex_INTERFACE_DEFINED__
#define __ISynchronizeMutex_INTERFACE_DEFINED__

/* interface ISynchronizeMutex */
/* [uuid][object][local] */ 


EXTERN_C const IID IID_ISynchronizeMutex;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000025-0000-0000-C000-000000000046")
    ISynchronizeMutex : public ISynchronize
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ReleaseMutex( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISynchronizeMutexVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISynchronizeMutex __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISynchronizeMutex __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISynchronizeMutex __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Wait )( 
            ISynchronizeMutex __RPC_FAR * This,
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwMilliseconds);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Signal )( 
            ISynchronizeMutex __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            ISynchronizeMutex __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseMutex )( 
            ISynchronizeMutex __RPC_FAR * This);
        
        END_INTERFACE
    } ISynchronizeMutexVtbl;

    interface ISynchronizeMutex
    {
        CONST_VTBL struct ISynchronizeMutexVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISynchronizeMutex_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISynchronizeMutex_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISynchronizeMutex_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISynchronizeMutex_Wait(This,dwFlags,dwMilliseconds)	\
    (This)->lpVtbl -> Wait(This,dwFlags,dwMilliseconds)

#define ISynchronizeMutex_Signal(This)	\
    (This)->lpVtbl -> Signal(This)

#define ISynchronizeMutex_Reset(This)	\
    (This)->lpVtbl -> Reset(This)


#define ISynchronizeMutex_ReleaseMutex(This)	\
    (This)->lpVtbl -> ReleaseMutex(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ISynchronizeMutex_ReleaseMutex_Proxy( 
    ISynchronizeMutex __RPC_FAR * This);


void __RPC_STUB ISynchronizeMutex_ReleaseMutex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISynchronizeMutex_INTERFACE_DEFINED__ */


#ifndef __ICancelMethodCalls_INTERFACE_DEFINED__
#define __ICancelMethodCalls_INTERFACE_DEFINED__

/* interface ICancelMethodCalls */
/* [uuid][object][local] */ 

typedef /* [unique] */ ICancelMethodCalls __RPC_FAR *LPCANCELMETHODCALLS;


EXTERN_C const IID IID_ICancelMethodCalls;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000029-0000-0000-C000-000000000046")
    ICancelMethodCalls : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Cancel( 
            /* [in] */ ULONG ulSeconds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TestCancel( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICancelMethodCallsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICancelMethodCalls __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICancelMethodCalls __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICancelMethodCalls __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Cancel )( 
            ICancelMethodCalls __RPC_FAR * This,
            /* [in] */ ULONG ulSeconds);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TestCancel )( 
            ICancelMethodCalls __RPC_FAR * This);
        
        END_INTERFACE
    } ICancelMethodCallsVtbl;

    interface ICancelMethodCalls
    {
        CONST_VTBL struct ICancelMethodCallsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICancelMethodCalls_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICancelMethodCalls_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICancelMethodCalls_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICancelMethodCalls_Cancel(This,ulSeconds)	\
    (This)->lpVtbl -> Cancel(This,ulSeconds)

#define ICancelMethodCalls_TestCancel(This)	\
    (This)->lpVtbl -> TestCancel(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ICancelMethodCalls_Cancel_Proxy( 
    ICancelMethodCalls __RPC_FAR * This,
    /* [in] */ ULONG ulSeconds);


void __RPC_STUB ICancelMethodCalls_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICancelMethodCalls_TestCancel_Proxy( 
    ICancelMethodCalls __RPC_FAR * This);


void __RPC_STUB ICancelMethodCalls_TestCancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICancelMethodCalls_INTERFACE_DEFINED__ */


#ifndef __IAsyncManager_INTERFACE_DEFINED__
#define __IAsyncManager_INTERFACE_DEFINED__

/* interface IAsyncManager */
/* [uuid][object][local] */ 

typedef 
enum tagDCOM_CALL_STATE
    {	DCOM_NONE	= 0,
	DCOM_CALL_COMPLETE	= 0x1,
	DCOM_CALL_CANCELED	= 0x2
    }	DCOM_CALL_STATE;


EXTERN_C const IID IID_IAsyncManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000002A-0000-0000-C000-000000000046")
    IAsyncManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CompleteCall( 
            /* [in] */ HRESULT Result) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCallContext( 
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *pInterface) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetState( 
            /* [out] */ ULONG __RPC_FAR *pulStateFlags) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAsyncManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAsyncManager __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAsyncManager __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAsyncManager __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CompleteCall )( 
            IAsyncManager __RPC_FAR * This,
            /* [in] */ HRESULT Result);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCallContext )( 
            IAsyncManager __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *pInterface);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetState )( 
            IAsyncManager __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pulStateFlags);
        
        END_INTERFACE
    } IAsyncManagerVtbl;

    interface IAsyncManager
    {
        CONST_VTBL struct IAsyncManagerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAsyncManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAsyncManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAsyncManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAsyncManager_CompleteCall(This,Result)	\
    (This)->lpVtbl -> CompleteCall(This,Result)

#define IAsyncManager_GetCallContext(This,riid,pInterface)	\
    (This)->lpVtbl -> GetCallContext(This,riid,pInterface)

#define IAsyncManager_GetState(This,pulStateFlags)	\
    (This)->lpVtbl -> GetState(This,pulStateFlags)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IAsyncManager_CompleteCall_Proxy( 
    IAsyncManager __RPC_FAR * This,
    /* [in] */ HRESULT Result);


void __RPC_STUB IAsyncManager_CompleteCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAsyncManager_GetCallContext_Proxy( 
    IAsyncManager __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [out] */ void __RPC_FAR *__RPC_FAR *pInterface);


void __RPC_STUB IAsyncManager_GetCallContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAsyncManager_GetState_Proxy( 
    IAsyncManager __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pulStateFlags);


void __RPC_STUB IAsyncManager_GetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAsyncManager_INTERFACE_DEFINED__ */


#ifndef __ICallFactory_INTERFACE_DEFINED__
#define __ICallFactory_INTERFACE_DEFINED__

/* interface ICallFactory */
/* [unique][uuid][object][local] */ 


EXTERN_C const IID IID_ICallFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1c733a30-2a1c-11ce-ade5-00aa0044773d")
    ICallFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateCall( 
            /* [in] */ REFIID riid,
            /* [in] */ IUnknown __RPC_FAR *pCtrlUnk,
            /* [in] */ REFIID riid2,
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppv) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICallFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICallFactory __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICallFactory __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICallFactory __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateCall )( 
            ICallFactory __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [in] */ IUnknown __RPC_FAR *pCtrlUnk,
            /* [in] */ REFIID riid2,
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppv);
        
        END_INTERFACE
    } ICallFactoryVtbl;

    interface ICallFactory
    {
        CONST_VTBL struct ICallFactoryVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICallFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICallFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICallFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICallFactory_CreateCall(This,riid,pCtrlUnk,riid2,ppv)	\
    (This)->lpVtbl -> CreateCall(This,riid,pCtrlUnk,riid2,ppv)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ICallFactory_CreateCall_Proxy( 
    ICallFactory __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [in] */ IUnknown __RPC_FAR *pCtrlUnk,
    /* [in] */ REFIID riid2,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB ICallFactory_CreateCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICallFactory_INTERFACE_DEFINED__ */


#ifndef __IRpcHelper_INTERFACE_DEFINED__
#define __IRpcHelper_INTERFACE_DEFINED__

/* interface IRpcHelper */
/* [object][local][unique][version][uuid] */ 


EXTERN_C const IID IID_IRpcHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000149-0000-0000-C000-000000000046")
    IRpcHelper : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDCOMProtocolVersion( 
            /* [out] */ DWORD __RPC_FAR *pComVersion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIIDFromOBJREF( 
            /* [in] */ void __RPC_FAR *pObjRef,
            /* [out] */ IID __RPC_FAR *__RPC_FAR *piid) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRpcHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRpcHelper __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRpcHelper __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRpcHelper __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDCOMProtocolVersion )( 
            IRpcHelper __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pComVersion);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIIDFromOBJREF )( 
            IRpcHelper __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pObjRef,
            /* [out] */ IID __RPC_FAR *__RPC_FAR *piid);
        
        END_INTERFACE
    } IRpcHelperVtbl;

    interface IRpcHelper
    {
        CONST_VTBL struct IRpcHelperVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcHelper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcHelper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcHelper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcHelper_GetDCOMProtocolVersion(This,pComVersion)	\
    (This)->lpVtbl -> GetDCOMProtocolVersion(This,pComVersion)

#define IRpcHelper_GetIIDFromOBJREF(This,pObjRef,piid)	\
    (This)->lpVtbl -> GetIIDFromOBJREF(This,pObjRef,piid)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRpcHelper_GetDCOMProtocolVersion_Proxy( 
    IRpcHelper __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pComVersion);


void __RPC_STUB IRpcHelper_GetDCOMProtocolVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcHelper_GetIIDFromOBJREF_Proxy( 
    IRpcHelper __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pObjRef,
    /* [out] */ IID __RPC_FAR *__RPC_FAR *piid);


void __RPC_STUB IRpcHelper_GetIIDFromOBJREF_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRpcHelper_INTERFACE_DEFINED__ */


#ifndef __IReleaseMarshalBuffers_INTERFACE_DEFINED__
#define __IReleaseMarshalBuffers_INTERFACE_DEFINED__

/* interface IReleaseMarshalBuffers */
/* [uuid][object][local] */ 


EXTERN_C const IID IID_IReleaseMarshalBuffers;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eb0cb9e8-7996-11d2-872e-0000f8080859")
    IReleaseMarshalBuffers : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ReleaseMarshalBuffer( 
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [in] */ DWORD dwFlags,
            /* [unique][in] */ IUnknown __RPC_FAR *pChnl) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IReleaseMarshalBuffersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IReleaseMarshalBuffers __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IReleaseMarshalBuffers __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IReleaseMarshalBuffers __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseMarshalBuffer )( 
            IReleaseMarshalBuffers __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
            /* [in] */ DWORD dwFlags,
            /* [unique][in] */ IUnknown __RPC_FAR *pChnl);
        
        END_INTERFACE
    } IReleaseMarshalBuffersVtbl;

    interface IReleaseMarshalBuffers
    {
        CONST_VTBL struct IReleaseMarshalBuffersVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IReleaseMarshalBuffers_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IReleaseMarshalBuffers_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IReleaseMarshalBuffers_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IReleaseMarshalBuffers_ReleaseMarshalBuffer(This,pMsg,dwFlags,pChnl)	\
    (This)->lpVtbl -> ReleaseMarshalBuffer(This,pMsg,dwFlags,pChnl)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IReleaseMarshalBuffers_ReleaseMarshalBuffer_Proxy( 
    IReleaseMarshalBuffers __RPC_FAR * This,
    /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMsg,
    /* [in] */ DWORD dwFlags,
    /* [unique][in] */ IUnknown __RPC_FAR *pChnl);


void __RPC_STUB IReleaseMarshalBuffers_ReleaseMarshalBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IReleaseMarshalBuffers_INTERFACE_DEFINED__ */


#ifndef __IWaitMultiple_INTERFACE_DEFINED__
#define __IWaitMultiple_INTERFACE_DEFINED__

/* interface IWaitMultiple */
/* [uuid][object][local] */ 


EXTERN_C const IID IID_IWaitMultiple;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000002B-0000-0000-C000-000000000046")
    IWaitMultiple : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE WaitMultiple( 
            /* [in] */ DWORD timeout,
            /* [out] */ ISynchronize __RPC_FAR *__RPC_FAR *pSync) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddSynchronize( 
            /* [in] */ ISynchronize __RPC_FAR *pSync) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWaitMultipleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWaitMultiple __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWaitMultiple __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWaitMultiple __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WaitMultiple )( 
            IWaitMultiple __RPC_FAR * This,
            /* [in] */ DWORD timeout,
            /* [out] */ ISynchronize __RPC_FAR *__RPC_FAR *pSync);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddSynchronize )( 
            IWaitMultiple __RPC_FAR * This,
            /* [in] */ ISynchronize __RPC_FAR *pSync);
        
        END_INTERFACE
    } IWaitMultipleVtbl;

    interface IWaitMultiple
    {
        CONST_VTBL struct IWaitMultipleVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWaitMultiple_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWaitMultiple_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWaitMultiple_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWaitMultiple_WaitMultiple(This,timeout,pSync)	\
    (This)->lpVtbl -> WaitMultiple(This,timeout,pSync)

#define IWaitMultiple_AddSynchronize(This,pSync)	\
    (This)->lpVtbl -> AddSynchronize(This,pSync)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IWaitMultiple_WaitMultiple_Proxy( 
    IWaitMultiple __RPC_FAR * This,
    /* [in] */ DWORD timeout,
    /* [out] */ ISynchronize __RPC_FAR *__RPC_FAR *pSync);


void __RPC_STUB IWaitMultiple_WaitMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWaitMultiple_AddSynchronize_Proxy( 
    IWaitMultiple __RPC_FAR * This,
    /* [in] */ ISynchronize __RPC_FAR *pSync);


void __RPC_STUB IWaitMultiple_AddSynchronize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWaitMultiple_INTERFACE_DEFINED__ */


#ifndef __IUrlMon_INTERFACE_DEFINED__
#define __IUrlMon_INTERFACE_DEFINED__

/* interface IUrlMon */
/* [uuid][object] */ 


EXTERN_C const IID IID_IUrlMon;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000026-0000-0000-C000-000000000046")
    IUrlMon : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AsyncGetClassBits( 
            /* [in] */ REFCLSID rclsid,
            /* [unique][in] */ LPCWSTR pszTYPE,
            /* [unique][in] */ LPCWSTR pszExt,
            /* [in] */ DWORD dwFileVersionMS,
            /* [in] */ DWORD dwFileVersionLS,
            /* [unique][in] */ LPCWSTR pszCodeBase,
            /* [in] */ IBindCtx __RPC_FAR *pbc,
            /* [in] */ DWORD dwClassContext,
            /* [in] */ REFIID riid,
            /* [in] */ DWORD flags) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUrlMonVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IUrlMon __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IUrlMon __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IUrlMon __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AsyncGetClassBits )( 
            IUrlMon __RPC_FAR * This,
            /* [in] */ REFCLSID rclsid,
            /* [unique][in] */ LPCWSTR pszTYPE,
            /* [unique][in] */ LPCWSTR pszExt,
            /* [in] */ DWORD dwFileVersionMS,
            /* [in] */ DWORD dwFileVersionLS,
            /* [unique][in] */ LPCWSTR pszCodeBase,
            /* [in] */ IBindCtx __RPC_FAR *pbc,
            /* [in] */ DWORD dwClassContext,
            /* [in] */ REFIID riid,
            /* [in] */ DWORD flags);
        
        END_INTERFACE
    } IUrlMonVtbl;

    interface IUrlMon
    {
        CONST_VTBL struct IUrlMonVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUrlMon_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUrlMon_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUrlMon_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUrlMon_AsyncGetClassBits(This,rclsid,pszTYPE,pszExt,dwFileVersionMS,dwFileVersionLS,pszCodeBase,pbc,dwClassContext,riid,flags)	\
    (This)->lpVtbl -> AsyncGetClassBits(This,rclsid,pszTYPE,pszExt,dwFileVersionMS,dwFileVersionLS,pszCodeBase,pbc,dwClassContext,riid,flags)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IUrlMon_AsyncGetClassBits_Proxy( 
    IUrlMon __RPC_FAR * This,
    /* [in] */ REFCLSID rclsid,
    /* [unique][in] */ LPCWSTR pszTYPE,
    /* [unique][in] */ LPCWSTR pszExt,
    /* [in] */ DWORD dwFileVersionMS,
    /* [in] */ DWORD dwFileVersionLS,
    /* [unique][in] */ LPCWSTR pszCodeBase,
    /* [in] */ IBindCtx __RPC_FAR *pbc,
    /* [in] */ DWORD dwClassContext,
    /* [in] */ REFIID riid,
    /* [in] */ DWORD flags);


void __RPC_STUB IUrlMon_AsyncGetClassBits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IUrlMon_INTERFACE_DEFINED__ */


#ifndef __IForegroundTransfer_INTERFACE_DEFINED__
#define __IForegroundTransfer_INTERFACE_DEFINED__

/* interface IForegroundTransfer */
/* [uuid][object][local] */ 


EXTERN_C const IID IID_IForegroundTransfer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000145-0000-0000-C000-000000000046")
    IForegroundTransfer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AllowForegroundTransfer( 
            /* [in] */ void __RPC_FAR *lpvReserved) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IForegroundTransferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IForegroundTransfer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IForegroundTransfer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IForegroundTransfer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AllowForegroundTransfer )( 
            IForegroundTransfer __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *lpvReserved);
        
        END_INTERFACE
    } IForegroundTransferVtbl;

    interface IForegroundTransfer
    {
        CONST_VTBL struct IForegroundTransferVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IForegroundTransfer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IForegroundTransfer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IForegroundTransfer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IForegroundTransfer_AllowForegroundTransfer(This,lpvReserved)	\
    (This)->lpVtbl -> AllowForegroundTransfer(This,lpvReserved)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IForegroundTransfer_AllowForegroundTransfer_Proxy( 
    IForegroundTransfer __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *lpvReserved);


void __RPC_STUB IForegroundTransfer_AllowForegroundTransfer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IForegroundTransfer_INTERFACE_DEFINED__ */


#ifndef __IPipeByte_INTERFACE_DEFINED__
#define __IPipeByte_INTERFACE_DEFINED__

/* interface IPipeByte */
/* [unique][async_uuid][uuid][object] */ 


EXTERN_C const IID IID_IPipeByte;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DB2F3ACA-2F86-11d1-8E04-00C04FB9989A")
    IPipeByte : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Pull( 
            /* [length_is][size_is][out] */ BYTE __RPC_FAR *buf,
            /* [in] */ ULONG cRequest,
            /* [out] */ ULONG __RPC_FAR *pcReturned) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Push( 
            /* [size_is][in] */ BYTE __RPC_FAR *buf,
            /* [in] */ ULONG cSent) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPipeByteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPipeByte __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPipeByte __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPipeByte __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Pull )( 
            IPipeByte __RPC_FAR * This,
            /* [length_is][size_is][out] */ BYTE __RPC_FAR *buf,
            /* [in] */ ULONG cRequest,
            /* [out] */ ULONG __RPC_FAR *pcReturned);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Push )( 
            IPipeByte __RPC_FAR * This,
            /* [size_is][in] */ BYTE __RPC_FAR *buf,
            /* [in] */ ULONG cSent);
        
        END_INTERFACE
    } IPipeByteVtbl;

    interface IPipeByte
    {
        CONST_VTBL struct IPipeByteVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPipeByte_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPipeByte_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPipeByte_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPipeByte_Pull(This,buf,cRequest,pcReturned)	\
    (This)->lpVtbl -> Pull(This,buf,cRequest,pcReturned)

#define IPipeByte_Push(This,buf,cSent)	\
    (This)->lpVtbl -> Push(This,buf,cSent)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPipeByte_Pull_Proxy( 
    IPipeByte __RPC_FAR * This,
    /* [length_is][size_is][out] */ BYTE __RPC_FAR *buf,
    /* [in] */ ULONG cRequest,
    /* [out] */ ULONG __RPC_FAR *pcReturned);


void __RPC_STUB IPipeByte_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPipeByte_Push_Proxy( 
    IPipeByte __RPC_FAR * This,
    /* [size_is][in] */ BYTE __RPC_FAR *buf,
    /* [in] */ ULONG cSent);


void __RPC_STUB IPipeByte_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPipeByte_INTERFACE_DEFINED__ */


#ifndef __AsyncIPipeByte_INTERFACE_DEFINED__
#define __AsyncIPipeByte_INTERFACE_DEFINED__

/* interface AsyncIPipeByte */
/* [uuid][unique][object] */ 


EXTERN_C const IID IID_AsyncIPipeByte;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DB2F3ACB-2F86-11d1-8E04-00C04FB9989A")
    AsyncIPipeByte : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Begin_Pull( 
            /* [in] */ ULONG cRequest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_Pull( 
            /* [length_is][size_is][out] */ BYTE __RPC_FAR *buf,
            /* [out] */ ULONG __RPC_FAR *pcReturned) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Begin_Push( 
            /* [size_is][in] */ BYTE __RPC_FAR *buf,
            /* [in] */ ULONG cSent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_Push( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct AsyncIPipeByteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            AsyncIPipeByte __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            AsyncIPipeByte __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            AsyncIPipeByte __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Begin_Pull )( 
            AsyncIPipeByte __RPC_FAR * This,
            /* [in] */ ULONG cRequest);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Finish_Pull )( 
            AsyncIPipeByte __RPC_FAR * This,
            /* [length_is][size_is][out] */ BYTE __RPC_FAR *buf,
            /* [out] */ ULONG __RPC_FAR *pcReturned);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Begin_Push )( 
            AsyncIPipeByte __RPC_FAR * This,
            /* [size_is][in] */ BYTE __RPC_FAR *buf,
            /* [in] */ ULONG cSent);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Finish_Push )( 
            AsyncIPipeByte __RPC_FAR * This);
        
        END_INTERFACE
    } AsyncIPipeByteVtbl;

    interface AsyncIPipeByte
    {
        CONST_VTBL struct AsyncIPipeByteVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define AsyncIPipeByte_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define AsyncIPipeByte_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define AsyncIPipeByte_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define AsyncIPipeByte_Begin_Pull(This,cRequest)	\
    (This)->lpVtbl -> Begin_Pull(This,cRequest)

#define AsyncIPipeByte_Finish_Pull(This,buf,pcReturned)	\
    (This)->lpVtbl -> Finish_Pull(This,buf,pcReturned)

#define AsyncIPipeByte_Begin_Push(This,buf,cSent)	\
    (This)->lpVtbl -> Begin_Push(This,buf,cSent)

#define AsyncIPipeByte_Finish_Push(This)	\
    (This)->lpVtbl -> Finish_Push(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE AsyncIPipeByte_Begin_Pull_Proxy( 
    AsyncIPipeByte __RPC_FAR * This,
    /* [in] */ ULONG cRequest);


void __RPC_STUB AsyncIPipeByte_Begin_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeByte_Finish_Pull_Proxy( 
    AsyncIPipeByte __RPC_FAR * This,
    /* [length_is][size_is][out] */ BYTE __RPC_FAR *buf,
    /* [out] */ ULONG __RPC_FAR *pcReturned);


void __RPC_STUB AsyncIPipeByte_Finish_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeByte_Begin_Push_Proxy( 
    AsyncIPipeByte __RPC_FAR * This,
    /* [size_is][in] */ BYTE __RPC_FAR *buf,
    /* [in] */ ULONG cSent);


void __RPC_STUB AsyncIPipeByte_Begin_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeByte_Finish_Push_Proxy( 
    AsyncIPipeByte __RPC_FAR * This);


void __RPC_STUB AsyncIPipeByte_Finish_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __AsyncIPipeByte_INTERFACE_DEFINED__ */


#ifndef __IPipeLong_INTERFACE_DEFINED__
#define __IPipeLong_INTERFACE_DEFINED__

/* interface IPipeLong */
/* [unique][async_uuid][uuid][object] */ 


EXTERN_C const IID IID_IPipeLong;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DB2F3ACC-2F86-11d1-8E04-00C04FB9989A")
    IPipeLong : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Pull( 
            /* [length_is][size_is][out] */ LONG __RPC_FAR *buf,
            /* [in] */ ULONG cRequest,
            /* [out] */ ULONG __RPC_FAR *pcReturned) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Push( 
            /* [size_is][in] */ LONG __RPC_FAR *buf,
            /* [in] */ ULONG cSent) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPipeLongVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPipeLong __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPipeLong __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPipeLong __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Pull )( 
            IPipeLong __RPC_FAR * This,
            /* [length_is][size_is][out] */ LONG __RPC_FAR *buf,
            /* [in] */ ULONG cRequest,
            /* [out] */ ULONG __RPC_FAR *pcReturned);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Push )( 
            IPipeLong __RPC_FAR * This,
            /* [size_is][in] */ LONG __RPC_FAR *buf,
            /* [in] */ ULONG cSent);
        
        END_INTERFACE
    } IPipeLongVtbl;

    interface IPipeLong
    {
        CONST_VTBL struct IPipeLongVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPipeLong_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPipeLong_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPipeLong_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPipeLong_Pull(This,buf,cRequest,pcReturned)	\
    (This)->lpVtbl -> Pull(This,buf,cRequest,pcReturned)

#define IPipeLong_Push(This,buf,cSent)	\
    (This)->lpVtbl -> Push(This,buf,cSent)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPipeLong_Pull_Proxy( 
    IPipeLong __RPC_FAR * This,
    /* [length_is][size_is][out] */ LONG __RPC_FAR *buf,
    /* [in] */ ULONG cRequest,
    /* [out] */ ULONG __RPC_FAR *pcReturned);


void __RPC_STUB IPipeLong_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPipeLong_Push_Proxy( 
    IPipeLong __RPC_FAR * This,
    /* [size_is][in] */ LONG __RPC_FAR *buf,
    /* [in] */ ULONG cSent);


void __RPC_STUB IPipeLong_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPipeLong_INTERFACE_DEFINED__ */


#ifndef __AsyncIPipeLong_INTERFACE_DEFINED__
#define __AsyncIPipeLong_INTERFACE_DEFINED__

/* interface AsyncIPipeLong */
/* [uuid][unique][object] */ 


EXTERN_C const IID IID_AsyncIPipeLong;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DB2F3ACD-2F86-11d1-8E04-00C04FB9989A")
    AsyncIPipeLong : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Begin_Pull( 
            /* [in] */ ULONG cRequest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_Pull( 
            /* [length_is][size_is][out] */ LONG __RPC_FAR *buf,
            /* [out] */ ULONG __RPC_FAR *pcReturned) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Begin_Push( 
            /* [size_is][in] */ LONG __RPC_FAR *buf,
            /* [in] */ ULONG cSent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_Push( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct AsyncIPipeLongVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            AsyncIPipeLong __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            AsyncIPipeLong __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            AsyncIPipeLong __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Begin_Pull )( 
            AsyncIPipeLong __RPC_FAR * This,
            /* [in] */ ULONG cRequest);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Finish_Pull )( 
            AsyncIPipeLong __RPC_FAR * This,
            /* [length_is][size_is][out] */ LONG __RPC_FAR *buf,
            /* [out] */ ULONG __RPC_FAR *pcReturned);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Begin_Push )( 
            AsyncIPipeLong __RPC_FAR * This,
            /* [size_is][in] */ LONG __RPC_FAR *buf,
            /* [in] */ ULONG cSent);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Finish_Push )( 
            AsyncIPipeLong __RPC_FAR * This);
        
        END_INTERFACE
    } AsyncIPipeLongVtbl;

    interface AsyncIPipeLong
    {
        CONST_VTBL struct AsyncIPipeLongVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define AsyncIPipeLong_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define AsyncIPipeLong_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define AsyncIPipeLong_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define AsyncIPipeLong_Begin_Pull(This,cRequest)	\
    (This)->lpVtbl -> Begin_Pull(This,cRequest)

#define AsyncIPipeLong_Finish_Pull(This,buf,pcReturned)	\
    (This)->lpVtbl -> Finish_Pull(This,buf,pcReturned)

#define AsyncIPipeLong_Begin_Push(This,buf,cSent)	\
    (This)->lpVtbl -> Begin_Push(This,buf,cSent)

#define AsyncIPipeLong_Finish_Push(This)	\
    (This)->lpVtbl -> Finish_Push(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE AsyncIPipeLong_Begin_Pull_Proxy( 
    AsyncIPipeLong __RPC_FAR * This,
    /* [in] */ ULONG cRequest);


void __RPC_STUB AsyncIPipeLong_Begin_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeLong_Finish_Pull_Proxy( 
    AsyncIPipeLong __RPC_FAR * This,
    /* [length_is][size_is][out] */ LONG __RPC_FAR *buf,
    /* [out] */ ULONG __RPC_FAR *pcReturned);


void __RPC_STUB AsyncIPipeLong_Finish_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeLong_Begin_Push_Proxy( 
    AsyncIPipeLong __RPC_FAR * This,
    /* [size_is][in] */ LONG __RPC_FAR *buf,
    /* [in] */ ULONG cSent);


void __RPC_STUB AsyncIPipeLong_Begin_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeLong_Finish_Push_Proxy( 
    AsyncIPipeLong __RPC_FAR * This);


void __RPC_STUB AsyncIPipeLong_Finish_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __AsyncIPipeLong_INTERFACE_DEFINED__ */


#ifndef __IPipeDouble_INTERFACE_DEFINED__
#define __IPipeDouble_INTERFACE_DEFINED__

/* interface IPipeDouble */
/* [unique][async_uuid][uuid][object] */ 


EXTERN_C const IID IID_IPipeDouble;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DB2F3ACE-2F86-11d1-8E04-00C04FB9989A")
    IPipeDouble : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Pull( 
            /* [length_is][size_is][out] */ DOUBLE __RPC_FAR *buf,
            /* [in] */ ULONG cRequest,
            /* [out] */ ULONG __RPC_FAR *pcReturned) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Push( 
            /* [size_is][in] */ DOUBLE __RPC_FAR *buf,
            /* [in] */ ULONG cSent) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPipeDoubleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPipeDouble __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPipeDouble __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPipeDouble __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Pull )( 
            IPipeDouble __RPC_FAR * This,
            /* [length_is][size_is][out] */ DOUBLE __RPC_FAR *buf,
            /* [in] */ ULONG cRequest,
            /* [out] */ ULONG __RPC_FAR *pcReturned);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Push )( 
            IPipeDouble __RPC_FAR * This,
            /* [size_is][in] */ DOUBLE __RPC_FAR *buf,
            /* [in] */ ULONG cSent);
        
        END_INTERFACE
    } IPipeDoubleVtbl;

    interface IPipeDouble
    {
        CONST_VTBL struct IPipeDoubleVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPipeDouble_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPipeDouble_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPipeDouble_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPipeDouble_Pull(This,buf,cRequest,pcReturned)	\
    (This)->lpVtbl -> Pull(This,buf,cRequest,pcReturned)

#define IPipeDouble_Push(This,buf,cSent)	\
    (This)->lpVtbl -> Push(This,buf,cSent)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPipeDouble_Pull_Proxy( 
    IPipeDouble __RPC_FAR * This,
    /* [length_is][size_is][out] */ DOUBLE __RPC_FAR *buf,
    /* [in] */ ULONG cRequest,
    /* [out] */ ULONG __RPC_FAR *pcReturned);


void __RPC_STUB IPipeDouble_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPipeDouble_Push_Proxy( 
    IPipeDouble __RPC_FAR * This,
    /* [size_is][in] */ DOUBLE __RPC_FAR *buf,
    /* [in] */ ULONG cSent);


void __RPC_STUB IPipeDouble_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPipeDouble_INTERFACE_DEFINED__ */


#ifndef __AsyncIPipeDouble_INTERFACE_DEFINED__
#define __AsyncIPipeDouble_INTERFACE_DEFINED__

/* interface AsyncIPipeDouble */
/* [uuid][unique][object] */ 


EXTERN_C const IID IID_AsyncIPipeDouble;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DB2F3ACF-2F86-11d1-8E04-00C04FB9989A")
    AsyncIPipeDouble : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Begin_Pull( 
            /* [in] */ ULONG cRequest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_Pull( 
            /* [length_is][size_is][out] */ DOUBLE __RPC_FAR *buf,
            /* [out] */ ULONG __RPC_FAR *pcReturned) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Begin_Push( 
            /* [size_is][in] */ DOUBLE __RPC_FAR *buf,
            /* [in] */ ULONG cSent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_Push( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct AsyncIPipeDoubleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            AsyncIPipeDouble __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            AsyncIPipeDouble __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            AsyncIPipeDouble __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Begin_Pull )( 
            AsyncIPipeDouble __RPC_FAR * This,
            /* [in] */ ULONG cRequest);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Finish_Pull )( 
            AsyncIPipeDouble __RPC_FAR * This,
            /* [length_is][size_is][out] */ DOUBLE __RPC_FAR *buf,
            /* [out] */ ULONG __RPC_FAR *pcReturned);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Begin_Push )( 
            AsyncIPipeDouble __RPC_FAR * This,
            /* [size_is][in] */ DOUBLE __RPC_FAR *buf,
            /* [in] */ ULONG cSent);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Finish_Push )( 
            AsyncIPipeDouble __RPC_FAR * This);
        
        END_INTERFACE
    } AsyncIPipeDoubleVtbl;

    interface AsyncIPipeDouble
    {
        CONST_VTBL struct AsyncIPipeDoubleVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define AsyncIPipeDouble_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define AsyncIPipeDouble_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define AsyncIPipeDouble_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define AsyncIPipeDouble_Begin_Pull(This,cRequest)	\
    (This)->lpVtbl -> Begin_Pull(This,cRequest)

#define AsyncIPipeDouble_Finish_Pull(This,buf,pcReturned)	\
    (This)->lpVtbl -> Finish_Pull(This,buf,pcReturned)

#define AsyncIPipeDouble_Begin_Push(This,buf,cSent)	\
    (This)->lpVtbl -> Begin_Push(This,buf,cSent)

#define AsyncIPipeDouble_Finish_Push(This)	\
    (This)->lpVtbl -> Finish_Push(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE AsyncIPipeDouble_Begin_Pull_Proxy( 
    AsyncIPipeDouble __RPC_FAR * This,
    /* [in] */ ULONG cRequest);


void __RPC_STUB AsyncIPipeDouble_Begin_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeDouble_Finish_Pull_Proxy( 
    AsyncIPipeDouble __RPC_FAR * This,
    /* [length_is][size_is][out] */ DOUBLE __RPC_FAR *buf,
    /* [out] */ ULONG __RPC_FAR *pcReturned);


void __RPC_STUB AsyncIPipeDouble_Finish_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeDouble_Begin_Push_Proxy( 
    AsyncIPipeDouble __RPC_FAR * This,
    /* [size_is][in] */ DOUBLE __RPC_FAR *buf,
    /* [in] */ ULONG cSent);


void __RPC_STUB AsyncIPipeDouble_Begin_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeDouble_Finish_Push_Proxy( 
    AsyncIPipeDouble __RPC_FAR * This);


void __RPC_STUB AsyncIPipeDouble_Finish_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __AsyncIPipeDouble_INTERFACE_DEFINED__ */


#ifndef __IThumbnailExtractor_INTERFACE_DEFINED__
#define __IThumbnailExtractor_INTERFACE_DEFINED__

/* interface IThumbnailExtractor */
/* [object][uuid] */ 


EXTERN_C const IID IID_IThumbnailExtractor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("969dc708-5c76-11d1-8d86-0000f804b057")
    IThumbnailExtractor : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ExtractThumbnail( 
            /* [in] */ IStorage __RPC_FAR *pStg,
            /* [in] */ ULONG ulLength,
            /* [in] */ ULONG ulHeight,
            /* [out] */ ULONG __RPC_FAR *pulOutputLength,
            /* [out] */ ULONG __RPC_FAR *pulOutputHeight,
            /* [out] */ HBITMAP __RPC_FAR *phOutputBitmap) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnFileUpdated( 
            /* [in] */ IStorage __RPC_FAR *pStg) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IThumbnailExtractorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IThumbnailExtractor __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IThumbnailExtractor __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IThumbnailExtractor __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExtractThumbnail )( 
            IThumbnailExtractor __RPC_FAR * This,
            /* [in] */ IStorage __RPC_FAR *pStg,
            /* [in] */ ULONG ulLength,
            /* [in] */ ULONG ulHeight,
            /* [out] */ ULONG __RPC_FAR *pulOutputLength,
            /* [out] */ ULONG __RPC_FAR *pulOutputHeight,
            /* [out] */ HBITMAP __RPC_FAR *phOutputBitmap);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnFileUpdated )( 
            IThumbnailExtractor __RPC_FAR * This,
            /* [in] */ IStorage __RPC_FAR *pStg);
        
        END_INTERFACE
    } IThumbnailExtractorVtbl;

    interface IThumbnailExtractor
    {
        CONST_VTBL struct IThumbnailExtractorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IThumbnailExtractor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IThumbnailExtractor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IThumbnailExtractor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IThumbnailExtractor_ExtractThumbnail(This,pStg,ulLength,ulHeight,pulOutputLength,pulOutputHeight,phOutputBitmap)	\
    (This)->lpVtbl -> ExtractThumbnail(This,pStg,ulLength,ulHeight,pulOutputLength,pulOutputHeight,phOutputBitmap)

#define IThumbnailExtractor_OnFileUpdated(This,pStg)	\
    (This)->lpVtbl -> OnFileUpdated(This,pStg)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IThumbnailExtractor_ExtractThumbnail_Proxy( 
    IThumbnailExtractor __RPC_FAR * This,
    /* [in] */ IStorage __RPC_FAR *pStg,
    /* [in] */ ULONG ulLength,
    /* [in] */ ULONG ulHeight,
    /* [out] */ ULONG __RPC_FAR *pulOutputLength,
    /* [out] */ ULONG __RPC_FAR *pulOutputHeight,
    /* [out] */ HBITMAP __RPC_FAR *phOutputBitmap);


void __RPC_STUB IThumbnailExtractor_ExtractThumbnail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IThumbnailExtractor_OnFileUpdated_Proxy( 
    IThumbnailExtractor __RPC_FAR * This,
    /* [in] */ IStorage __RPC_FAR *pStg);


void __RPC_STUB IThumbnailExtractor_OnFileUpdated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IThumbnailExtractor_INTERFACE_DEFINED__ */


#ifndef __IDummyHICONIncluder_INTERFACE_DEFINED__
#define __IDummyHICONIncluder_INTERFACE_DEFINED__

/* interface IDummyHICONIncluder */
/* [uuid][unique][object] */ 


EXTERN_C const IID IID_IDummyHICONIncluder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("947990de-cc28-11d2-a0f7-00805f858fb1")
    IDummyHICONIncluder : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Dummy( 
            /* [in] */ HICON h1,
            /* [in] */ HDC h2) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDummyHICONIncluderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDummyHICONIncluder __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDummyHICONIncluder __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDummyHICONIncluder __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Dummy )( 
            IDummyHICONIncluder __RPC_FAR * This,
            /* [in] */ HICON h1,
            /* [in] */ HDC h2);
        
        END_INTERFACE
    } IDummyHICONIncluderVtbl;

    interface IDummyHICONIncluder
    {
        CONST_VTBL struct IDummyHICONIncluderVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDummyHICONIncluder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDummyHICONIncluder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDummyHICONIncluder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDummyHICONIncluder_Dummy(This,h1,h2)	\
    (This)->lpVtbl -> Dummy(This,h1,h2)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDummyHICONIncluder_Dummy_Proxy( 
    IDummyHICONIncluder __RPC_FAR * This,
    /* [in] */ HICON h1,
    /* [in] */ HDC h2);


void __RPC_STUB IDummyHICONIncluder_Dummy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDummyHICONIncluder_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_objidl_0081 */
/* [local] */ 

#if ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201)
#endif
#endif


extern RPC_IF_HANDLE __MIDL_itf_objidl_0081_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_objidl_0081_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  ASYNC_STGMEDIUM_UserSize(     unsigned long __RPC_FAR *, unsigned long            , ASYNC_STGMEDIUM __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  ASYNC_STGMEDIUM_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, ASYNC_STGMEDIUM __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  ASYNC_STGMEDIUM_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, ASYNC_STGMEDIUM __RPC_FAR * ); 
void                      __RPC_USER  ASYNC_STGMEDIUM_UserFree(     unsigned long __RPC_FAR *, ASYNC_STGMEDIUM __RPC_FAR * ); 

unsigned long             __RPC_USER  CLIPFORMAT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , CLIPFORMAT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  CLIPFORMAT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, CLIPFORMAT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  CLIPFORMAT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, CLIPFORMAT __RPC_FAR * ); 
void                      __RPC_USER  CLIPFORMAT_UserFree(     unsigned long __RPC_FAR *, CLIPFORMAT __RPC_FAR * ); 

unsigned long             __RPC_USER  FLAG_STGMEDIUM_UserSize(     unsigned long __RPC_FAR *, unsigned long            , FLAG_STGMEDIUM __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  FLAG_STGMEDIUM_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, FLAG_STGMEDIUM __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  FLAG_STGMEDIUM_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, FLAG_STGMEDIUM __RPC_FAR * ); 
void                      __RPC_USER  FLAG_STGMEDIUM_UserFree(     unsigned long __RPC_FAR *, FLAG_STGMEDIUM __RPC_FAR * ); 

unsigned long             __RPC_USER  HBITMAP_UserSize(     unsigned long __RPC_FAR *, unsigned long            , HBITMAP __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HBITMAP_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HBITMAP __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HBITMAP_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HBITMAP __RPC_FAR * ); 
void                      __RPC_USER  HBITMAP_UserFree(     unsigned long __RPC_FAR *, HBITMAP __RPC_FAR * ); 

unsigned long             __RPC_USER  HDC_UserSize(     unsigned long __RPC_FAR *, unsigned long            , HDC __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HDC_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HDC __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HDC_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HDC __RPC_FAR * ); 
void                      __RPC_USER  HDC_UserFree(     unsigned long __RPC_FAR *, HDC __RPC_FAR * ); 

unsigned long             __RPC_USER  HICON_UserSize(     unsigned long __RPC_FAR *, unsigned long            , HICON __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HICON_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HICON __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HICON_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HICON __RPC_FAR * ); 
void                      __RPC_USER  HICON_UserFree(     unsigned long __RPC_FAR *, HICON __RPC_FAR * ); 

unsigned long             __RPC_USER  SNB_UserSize(     unsigned long __RPC_FAR *, unsigned long            , SNB __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  SNB_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, SNB __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  SNB_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, SNB __RPC_FAR * ); 
void                      __RPC_USER  SNB_UserFree(     unsigned long __RPC_FAR *, SNB __RPC_FAR * ); 

unsigned long             __RPC_USER  STGMEDIUM_UserSize(     unsigned long __RPC_FAR *, unsigned long            , STGMEDIUM __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  STGMEDIUM_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, STGMEDIUM __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  STGMEDIUM_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, STGMEDIUM __RPC_FAR * ); 
void                      __RPC_USER  STGMEDIUM_UserFree(     unsigned long __RPC_FAR *, STGMEDIUM __RPC_FAR * ); 

/* [local] */ HRESULT STDMETHODCALLTYPE IEnumUnknown_Next_Proxy( 
    IEnumUnknown __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumUnknown_Next_Stub( 
    IEnumUnknown __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT STDMETHODCALLTYPE IBindCtx_SetBindOptions_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [in] */ BIND_OPTS __RPC_FAR *pbindopts);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IBindCtx_SetBindOptions_Stub( 
    IBindCtx __RPC_FAR * This,
    /* [in] */ BIND_OPTS2 __RPC_FAR *pbindopts);

/* [local] */ HRESULT STDMETHODCALLTYPE IBindCtx_GetBindOptions_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [out][in] */ BIND_OPTS __RPC_FAR *pbindopts);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IBindCtx_GetBindOptions_Stub( 
    IBindCtx __RPC_FAR * This,
    /* [out][in] */ BIND_OPTS2 __RPC_FAR *pbindopts);

/* [local] */ HRESULT STDMETHODCALLTYPE IEnumMoniker_Next_Proxy( 
    IEnumMoniker __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IMoniker __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumMoniker_Next_Stub( 
    IEnumMoniker __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IMoniker __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ BOOL STDMETHODCALLTYPE IRunnableObject_IsRunning_Proxy( 
    IRunnableObject __RPC_FAR * This);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IRunnableObject_IsRunning_Stub( 
    IRunnableObject __RPC_FAR * This);

/* [local] */ HRESULT STDMETHODCALLTYPE IMoniker_BindToObject_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [in] */ REFIID riidResult,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvResult);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IMoniker_BindToObject_Stub( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [in] */ REFIID riidResult,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvResult);

/* [local] */ HRESULT STDMETHODCALLTYPE IMoniker_BindToStorage_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObj);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IMoniker_BindToStorage_Stub( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObj);

/* [local] */ HRESULT STDMETHODCALLTYPE IEnumString_Next_Proxy( 
    IEnumString __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ LPOLESTR __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumString_Next_Stub( 
    IEnumString __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ LPOLESTR __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT STDMETHODCALLTYPE ISequentialStream_Read_Proxy( 
    ISequentialStream __RPC_FAR * This,
    /* [length_is][size_is][out] */ void __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbRead);


/* [call_as] */ HRESULT STDMETHODCALLTYPE ISequentialStream_Read_Stub( 
    ISequentialStream __RPC_FAR * This,
    /* [length_is][size_is][out] */ byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbRead);

/* [local] */ HRESULT STDMETHODCALLTYPE ISequentialStream_Write_Proxy( 
    ISequentialStream __RPC_FAR * This,
    /* [size_is][in] */ const void __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);


/* [call_as] */ HRESULT STDMETHODCALLTYPE ISequentialStream_Write_Stub( 
    ISequentialStream __RPC_FAR * This,
    /* [size_is][in] */ const byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);

/* [local] */ HRESULT STDMETHODCALLTYPE IStream_Seek_Proxy( 
    IStream __RPC_FAR * This,
    /* [in] */ LARGE_INTEGER dlibMove,
    /* [in] */ DWORD dwOrigin,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *plibNewPosition);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStream_Seek_Stub( 
    IStream __RPC_FAR * This,
    /* [in] */ LARGE_INTEGER dlibMove,
    /* [in] */ DWORD dwOrigin,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *plibNewPosition);

/* [local] */ HRESULT STDMETHODCALLTYPE IStream_CopyTo_Proxy( 
    IStream __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pstm,
    /* [in] */ ULARGE_INTEGER cb,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbRead,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbWritten);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStream_CopyTo_Stub( 
    IStream __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pstm,
    /* [in] */ ULARGE_INTEGER cb,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbRead,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbWritten);

/* [local] */ HRESULT STDMETHODCALLTYPE IEnumSTATSTG_Next_Proxy( 
    IEnumSTATSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumSTATSTG_Next_Stub( 
    IEnumSTATSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT STDMETHODCALLTYPE IStorage_OpenStream_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
    /* [unique][in] */ void __RPC_FAR *reserved1,
    /* [in] */ DWORD grfMode,
    /* [in] */ DWORD reserved2,
    /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStorage_OpenStream_Stub( 
    IStorage __RPC_FAR * This,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
    /* [in] */ unsigned long cbReserved1,
    /* [size_is][unique][in] */ byte __RPC_FAR *reserved1,
    /* [in] */ DWORD grfMode,
    /* [in] */ DWORD reserved2,
    /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);

/* [local] */ HRESULT STDMETHODCALLTYPE IStorage_EnumElements_Proxy( 
    IStorage __RPC_FAR * This,
    /* [in] */ DWORD reserved1,
    /* [size_is][unique][in] */ void __RPC_FAR *reserved2,
    /* [in] */ DWORD reserved3,
    /* [out] */ IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStorage_EnumElements_Stub( 
    IStorage __RPC_FAR * This,
    /* [in] */ DWORD reserved1,
    /* [in] */ unsigned long cbReserved2,
    /* [size_is][unique][in] */ byte __RPC_FAR *reserved2,
    /* [in] */ DWORD reserved3,
    /* [out] */ IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum);

/* [local] */ HRESULT STDMETHODCALLTYPE ILockBytes_ReadAt_Proxy( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER ulOffset,
    /* [length_is][size_is][out] */ void __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbRead);


/* [call_as] */ HRESULT __stdcall ILockBytes_ReadAt_Stub( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER ulOffset,
    /* [length_is][size_is][out] */ byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbRead);

/* [local] */ HRESULT STDMETHODCALLTYPE ILockBytes_WriteAt_Proxy( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER ulOffset,
    /* [size_is][in] */ const void __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);


/* [call_as] */ HRESULT STDMETHODCALLTYPE ILockBytes_WriteAt_Stub( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER ulOffset,
    /* [size_is][in] */ const byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);

/* [local] */ HRESULT STDMETHODCALLTYPE IEnumFORMATETC_Next_Proxy( 
    IEnumFORMATETC __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ FORMATETC __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumFORMATETC_Next_Stub( 
    IEnumFORMATETC __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ FORMATETC __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT STDMETHODCALLTYPE IEnumSTATDATA_Next_Proxy( 
    IEnumSTATDATA __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATDATA __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumSTATDATA_Next_Stub( 
    IEnumSTATDATA __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATDATA __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ void STDMETHODCALLTYPE IAdviseSink_OnDataChange_Proxy( 
    IAdviseSink __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
    /* [unique][in] */ STGMEDIUM __RPC_FAR *pStgmed);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IAdviseSink_OnDataChange_Stub( 
    IAdviseSink __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
    /* [unique][in] */ ASYNC_STGMEDIUM __RPC_FAR *pStgmed);

/* [local] */ void STDMETHODCALLTYPE IAdviseSink_OnViewChange_Proxy( 
    IAdviseSink __RPC_FAR * This,
    /* [in] */ DWORD dwAspect,
    /* [in] */ LONG lindex);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IAdviseSink_OnViewChange_Stub( 
    IAdviseSink __RPC_FAR * This,
    /* [in] */ DWORD dwAspect,
    /* [in] */ LONG lindex);

/* [local] */ void STDMETHODCALLTYPE IAdviseSink_OnRename_Proxy( 
    IAdviseSink __RPC_FAR * This,
    /* [in] */ IMoniker __RPC_FAR *pmk);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IAdviseSink_OnRename_Stub( 
    IAdviseSink __RPC_FAR * This,
    /* [in] */ IMoniker __RPC_FAR *pmk);

/* [local] */ void STDMETHODCALLTYPE IAdviseSink_OnSave_Proxy( 
    IAdviseSink __RPC_FAR * This);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IAdviseSink_OnSave_Stub( 
    IAdviseSink __RPC_FAR * This);

/* [local] */ void STDMETHODCALLTYPE IAdviseSink_OnClose_Proxy( 
    IAdviseSink __RPC_FAR * This);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IAdviseSink_OnClose_Stub( 
    IAdviseSink __RPC_FAR * This);

/* [local] */ void STDMETHODCALLTYPE IAdviseSink2_OnLinkSrcChange_Proxy( 
    IAdviseSink2 __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmk);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IAdviseSink2_OnLinkSrcChange_Stub( 
    IAdviseSink2 __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmk);

/* [local] */ HRESULT STDMETHODCALLTYPE IDataObject_GetData_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetcIn,
    /* [out] */ STGMEDIUM __RPC_FAR *pmedium);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IDataObject_GetData_Stub( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetcIn,
    /* [out] */ STGMEDIUM __RPC_FAR *pRemoteMedium);

/* [local] */ HRESULT STDMETHODCALLTYPE IDataObject_GetDataHere_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [out][in] */ STGMEDIUM __RPC_FAR *pmedium);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IDataObject_GetDataHere_Stub( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [out][in] */ STGMEDIUM __RPC_FAR *pRemoteMedium);

/* [local] */ HRESULT STDMETHODCALLTYPE IDataObject_SetData_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [unique][in] */ STGMEDIUM __RPC_FAR *pmedium,
    /* [in] */ BOOL fRelease);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IDataObject_SetData_Stub( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [unique][in] */ FLAG_STGMEDIUM __RPC_FAR *pmedium,
    /* [in] */ BOOL fRelease);

/* [local] */ HRESULT STDMETHODCALLTYPE IFillLockBytes_FillAppend_Proxy( 
    IFillLockBytes __RPC_FAR * This,
    /* [size_is][in] */ const void __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);


/* [call_as] */ HRESULT __stdcall IFillLockBytes_FillAppend_Stub( 
    IFillLockBytes __RPC_FAR * This,
    /* [size_is][in] */ const byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);

/* [local] */ HRESULT STDMETHODCALLTYPE IFillLockBytes_FillAt_Proxy( 
    IFillLockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER ulOffset,
    /* [size_is][in] */ const void __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);


/* [call_as] */ HRESULT __stdcall IFillLockBytes_FillAt_Stub( 
    IFillLockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER ulOffset,
    /* [size_is][in] */ const byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);

/* [local] */ void STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnDataChange_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
    /* [unique][in] */ STGMEDIUM __RPC_FAR *pStgmed);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnDataChange_Stub( 
    AsyncIAdviseSink __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
    /* [unique][in] */ ASYNC_STGMEDIUM __RPC_FAR *pStgmed);

/* [local] */ void STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnDataChange_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnDataChange_Stub( 
    AsyncIAdviseSink __RPC_FAR * This);

/* [local] */ void STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnViewChange_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This,
    /* [in] */ DWORD dwAspect,
    /* [in] */ LONG lindex);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnViewChange_Stub( 
    AsyncIAdviseSink __RPC_FAR * This,
    /* [in] */ DWORD dwAspect,
    /* [in] */ LONG lindex);

/* [local] */ void STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnViewChange_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnViewChange_Stub( 
    AsyncIAdviseSink __RPC_FAR * This);

/* [local] */ void STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnRename_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This,
    /* [in] */ IMoniker __RPC_FAR *pmk);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnRename_Stub( 
    AsyncIAdviseSink __RPC_FAR * This,
    /* [in] */ IMoniker __RPC_FAR *pmk);

/* [local] */ void STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnRename_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnRename_Stub( 
    AsyncIAdviseSink __RPC_FAR * This);

/* [local] */ void STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnSave_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnSave_Stub( 
    AsyncIAdviseSink __RPC_FAR * This);

/* [local] */ void STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnSave_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnSave_Stub( 
    AsyncIAdviseSink __RPC_FAR * This);

/* [local] */ void STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnClose_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnClose_Stub( 
    AsyncIAdviseSink __RPC_FAR * This);

/* [local] */ void STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnClose_Proxy( 
    AsyncIAdviseSink __RPC_FAR * This);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnClose_Stub( 
    AsyncIAdviseSink __RPC_FAR * This);

/* [local] */ void STDMETHODCALLTYPE AsyncIAdviseSink2_Begin_OnLinkSrcChange_Proxy( 
    AsyncIAdviseSink2 __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmk);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink2_Begin_OnLinkSrcChange_Stub( 
    AsyncIAdviseSink2 __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmk);

/* [local] */ void STDMETHODCALLTYPE AsyncIAdviseSink2_Finish_OnLinkSrcChange_Proxy( 
    AsyncIAdviseSink2 __RPC_FAR * This);


/* [call_as] */ HRESULT STDMETHODCALLTYPE AsyncIAdviseSink2_Finish_OnLinkSrcChange_Stub( 
    AsyncIAdviseSink2 __RPC_FAR * This);



/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



