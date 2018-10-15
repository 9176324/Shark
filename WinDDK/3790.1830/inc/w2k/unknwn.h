
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 5.03.0279 */
/* at Wed Mar 12 13:27:27 2003
 */
/* Compiler settings for unknwn.idl:
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

#ifndef __unknwn_h__
#define __unknwn_h__

/* Forward Declarations */ 

#ifndef __IUnknown_FWD_DEFINED__
#define __IUnknown_FWD_DEFINED__
typedef interface IUnknown IUnknown;
#endif 	/* __IUnknown_FWD_DEFINED__ */


#ifndef __AsyncIUnknown_FWD_DEFINED__
#define __AsyncIUnknown_FWD_DEFINED__
typedef interface AsyncIUnknown AsyncIUnknown;
#endif 	/* __AsyncIUnknown_FWD_DEFINED__ */


#ifndef __IClassFactory_FWD_DEFINED__
#define __IClassFactory_FWD_DEFINED__
typedef interface IClassFactory IClassFactory;
#endif 	/* __IClassFactory_FWD_DEFINED__ */


/* header files for imported files */
#include "wtypes.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_unknwn_0000 */
/* [local] */ 

//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992-1999.
//
//--------------------------------------------------------------------------
#if ( _MSC_VER >= 1020 )
#pragma once
#endif


extern RPC_IF_HANDLE __MIDL_itf_unknwn_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_unknwn_0000_v0_0_s_ifspec;

#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__

/* interface IUnknown */
/* [unique][uuid][object][local] */ 

typedef /* [unique] */ IUnknown __RPC_FAR *LPUNKNOWN;

//////////////////////////////////////////////////////////////////
// IID_IUnknown and all other system IIDs are provided in UUID.LIB
// Link that library in with your proxies, clients and servers
//////////////////////////////////////////////////////////////////

#if (_MSC_VER >= 1100) && defined(__cplusplus) && !defined(CINTERFACE)
    EXTERN_C const IID IID_IUnknown;
    extern "C++"
    {
        MIDL_INTERFACE("00000000-0000-0000-C000-000000000046")
        IUnknown
        {
        public:
            BEGIN_INTERFACE
            virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
                /* [in] */ REFIID riid,
                /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject) = 0;
            
            virtual ULONG STDMETHODCALLTYPE AddRef( void) = 0;
            
            virtual ULONG STDMETHODCALLTYPE Release( void) = 0;
    	
            template<class Q>
    	HRESULT STDMETHODCALLTYPE QueryInterface(Q** pp)
    	{
    	    return QueryInterface(__uuidof(Q), (void **)pp);
    	}
            
            END_INTERFACE
        };
    } // extern C++
    HRESULT STDMETHODCALLTYPE IUnknown_QueryInterface_Proxy(
        IUnknown __RPC_FAR * This,
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
    
    void __RPC_STUB IUnknown_QueryInterface_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    ULONG STDMETHODCALLTYPE IUnknown_AddRef_Proxy(
        IUnknown __RPC_FAR * This);
    
    void __RPC_STUB IUnknown_AddRef_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
    
    ULONG STDMETHODCALLTYPE IUnknown_Release_Proxy(
        IUnknown __RPC_FAR * This);
    
    void __RPC_STUB IUnknown_Release_Stub(
        IRpcStubBuffer *This,
        IRpcChannelBuffer *_pRpcChannelBuffer,
        PRPC_MESSAGE _pRpcMessage,
        DWORD *_pdwStubPhase);
#else // VC6 hack

EXTERN_C const IID IID_IUnknown;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000000-0000-0000-C000-000000000046")
    IUnknown
    {
    public:
        BEGIN_INTERFACE
        virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject) = 0;
        
        virtual ULONG STDMETHODCALLTYPE AddRef( void) = 0;
        
        virtual ULONG STDMETHODCALLTYPE Release( void) = 0;
        
        END_INTERFACE
    };
    
#else 	/* C style interface */

    typedef struct IUnknownVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IUnknown __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IUnknown __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IUnknown __RPC_FAR * This);
        
        END_INTERFACE
    } IUnknownVtbl;

    interface IUnknown
    {
        CONST_VTBL struct IUnknownVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUnknown_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUnknown_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUnknown_Release(This)	\
    (This)->lpVtbl -> Release(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IUnknown_QueryInterface_Proxy( 
    IUnknown __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);


void __RPC_STUB IUnknown_QueryInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG STDMETHODCALLTYPE IUnknown_AddRef_Proxy( 
    IUnknown __RPC_FAR * This);


void __RPC_STUB IUnknown_AddRef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG STDMETHODCALLTYPE IUnknown_Release_Proxy( 
    IUnknown __RPC_FAR * This);


void __RPC_STUB IUnknown_Release_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IUnknown_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_unknwn_0005 */
/* [local] */ 

#endif // VC6 hack


extern RPC_IF_HANDLE __MIDL_itf_unknwn_0005_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_unknwn_0005_v0_0_s_ifspec;

#ifndef __AsyncIUnknown_INTERFACE_DEFINED__
#define __AsyncIUnknown_INTERFACE_DEFINED__

/* interface AsyncIUnknown */
/* [unique][uuid][object][local] */ 


EXTERN_C const IID IID_AsyncIUnknown;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000e0000-0000-0000-C000-000000000046")
    AsyncIUnknown : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Begin_QueryInterface( 
            /* [in] */ REFIID riid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_QueryInterface( 
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Begin_AddRef( void) = 0;
        
        virtual ULONG STDMETHODCALLTYPE Finish_AddRef( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Begin_Release( void) = 0;
        
        virtual ULONG STDMETHODCALLTYPE Finish_Release( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct AsyncIUnknownVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            AsyncIUnknown __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            AsyncIUnknown __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            AsyncIUnknown __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Begin_QueryInterface )( 
            AsyncIUnknown __RPC_FAR * This,
            /* [in] */ REFIID riid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Finish_QueryInterface )( 
            AsyncIUnknown __RPC_FAR * This,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Begin_AddRef )( 
            AsyncIUnknown __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Finish_AddRef )( 
            AsyncIUnknown __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Begin_Release )( 
            AsyncIUnknown __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Finish_Release )( 
            AsyncIUnknown __RPC_FAR * This);
        
        END_INTERFACE
    } AsyncIUnknownVtbl;

    interface AsyncIUnknown
    {
        CONST_VTBL struct AsyncIUnknownVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define AsyncIUnknown_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define AsyncIUnknown_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define AsyncIUnknown_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define AsyncIUnknown_Begin_QueryInterface(This,riid)	\
    (This)->lpVtbl -> Begin_QueryInterface(This,riid)

#define AsyncIUnknown_Finish_QueryInterface(This,ppvObject)	\
    (This)->lpVtbl -> Finish_QueryInterface(This,ppvObject)

#define AsyncIUnknown_Begin_AddRef(This)	\
    (This)->lpVtbl -> Begin_AddRef(This)

#define AsyncIUnknown_Finish_AddRef(This)	\
    (This)->lpVtbl -> Finish_AddRef(This)

#define AsyncIUnknown_Begin_Release(This)	\
    (This)->lpVtbl -> Begin_Release(This)

#define AsyncIUnknown_Finish_Release(This)	\
    (This)->lpVtbl -> Finish_Release(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE AsyncIUnknown_Begin_QueryInterface_Proxy( 
    AsyncIUnknown __RPC_FAR * This,
    /* [in] */ REFIID riid);


void __RPC_STUB AsyncIUnknown_Begin_QueryInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIUnknown_Finish_QueryInterface_Proxy( 
    AsyncIUnknown __RPC_FAR * This,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);


void __RPC_STUB AsyncIUnknown_Finish_QueryInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIUnknown_Begin_AddRef_Proxy( 
    AsyncIUnknown __RPC_FAR * This);


void __RPC_STUB AsyncIUnknown_Begin_AddRef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG STDMETHODCALLTYPE AsyncIUnknown_Finish_AddRef_Proxy( 
    AsyncIUnknown __RPC_FAR * This);


void __RPC_STUB AsyncIUnknown_Finish_AddRef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIUnknown_Begin_Release_Proxy( 
    AsyncIUnknown __RPC_FAR * This);


void __RPC_STUB AsyncIUnknown_Begin_Release_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG STDMETHODCALLTYPE AsyncIUnknown_Finish_Release_Proxy( 
    AsyncIUnknown __RPC_FAR * This);


void __RPC_STUB AsyncIUnknown_Finish_Release_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __AsyncIUnknown_INTERFACE_DEFINED__ */


#ifndef __IClassFactory_INTERFACE_DEFINED__
#define __IClassFactory_INTERFACE_DEFINED__

/* interface IClassFactory */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IClassFactory __RPC_FAR *LPCLASSFACTORY;


EXTERN_C const IID IID_IClassFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000001-0000-0000-C000-000000000046")
    IClassFactory : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE CreateInstance( 
            /* [unique][in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE LockServer( 
            /* [in] */ BOOL fLock) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IClassFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IClassFactory __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IClassFactory __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IClassFactory __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateInstance )( 
            IClassFactory __RPC_FAR * This,
            /* [unique][in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LockServer )( 
            IClassFactory __RPC_FAR * This,
            /* [in] */ BOOL fLock);
        
        END_INTERFACE
    } IClassFactoryVtbl;

    interface IClassFactory
    {
        CONST_VTBL struct IClassFactoryVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClassFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClassFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClassFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClassFactory_CreateInstance(This,pUnkOuter,riid,ppvObject)	\
    (This)->lpVtbl -> CreateInstance(This,pUnkOuter,riid,ppvObject)

#define IClassFactory_LockServer(This,fLock)	\
    (This)->lpVtbl -> LockServer(This,fLock)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IClassFactory_RemoteCreateInstance_Proxy( 
    IClassFactory __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObject);


void __RPC_STUB IClassFactory_RemoteCreateInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT __stdcall IClassFactory_RemoteLockServer_Proxy( 
    IClassFactory __RPC_FAR * This,
    /* [in] */ BOOL fLock);


void __RPC_STUB IClassFactory_RemoteLockServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IClassFactory_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* [local] */ HRESULT STDMETHODCALLTYPE IClassFactory_CreateInstance_Proxy( 
    IClassFactory __RPC_FAR * This,
    /* [unique][in] */ IUnknown __RPC_FAR *pUnkOuter,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IClassFactory_CreateInstance_Stub( 
    IClassFactory __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObject);

/* [local] */ HRESULT STDMETHODCALLTYPE IClassFactory_LockServer_Proxy( 
    IClassFactory __RPC_FAR * This,
    /* [in] */ BOOL fLock);


/* [call_as] */ HRESULT __stdcall IClassFactory_LockServer_Stub( 
    IClassFactory __RPC_FAR * This,
    /* [in] */ BOOL fLock);



/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



