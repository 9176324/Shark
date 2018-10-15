//
//	This file was automatically generated from the IDL files 
//	included with the WBEM SDK in the \include directory.  If you
//  experience problems compiling this file you can re-generate it
//  by running NMAKE (or another MAKE utility) from within the 
//	\include directory.
//
// Copyright 1999 Microsoft Corporation
//
//
//=================================================================

#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 5.03.0279 */
/* at Fri Nov 12 15:42:09 1999
 */
/* Compiler settings for wbemtran.idl:
    Os (OptLev=s), W1, Zp8, env=Win32 (32b run), ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifndef __wbemtran_h__
#define __wbemtran_h__

/* Forward Declarations */ 

#ifndef __IWbemTransport_FWD_DEFINED__
#define __IWbemTransport_FWD_DEFINED__
typedef interface IWbemTransport IWbemTransport;
#endif 	/* __IWbemTransport_FWD_DEFINED__ */


#ifndef __IWbemLevel1Login_FWD_DEFINED__
#define __IWbemLevel1Login_FWD_DEFINED__
typedef interface IWbemLevel1Login IWbemLevel1Login;
#endif 	/* __IWbemLevel1Login_FWD_DEFINED__ */


#ifndef __IWbemClientTransport_FWD_DEFINED__
#define __IWbemClientTransport_FWD_DEFINED__
typedef interface IWbemClientTransport IWbemClientTransport;
#endif 	/* __IWbemClientTransport_FWD_DEFINED__ */


#ifndef __IWbemAddressResolution_FWD_DEFINED__
#define __IWbemAddressResolution_FWD_DEFINED__
typedef interface IWbemAddressResolution IWbemAddressResolution;
#endif 	/* __IWbemAddressResolution_FWD_DEFINED__ */


#ifndef __WbemLevel1Login_FWD_DEFINED__
#define __WbemLevel1Login_FWD_DEFINED__

#ifdef __cplusplus
typedef class WbemLevel1Login WbemLevel1Login;
#else
typedef struct WbemLevel1Login WbemLevel1Login;
#endif /* __cplusplus */

#endif 	/* __WbemLevel1Login_FWD_DEFINED__ */


#ifndef __WbemDCOMTransport_FWD_DEFINED__
#define __WbemDCOMTransport_FWD_DEFINED__

#ifdef __cplusplus
typedef class WbemDCOMTransport WbemDCOMTransport;
#else
typedef struct WbemDCOMTransport WbemDCOMTransport;
#endif /* __cplusplus */

#endif 	/* __WbemDCOMTransport_FWD_DEFINED__ */


#ifndef __WbemLocalAddrRes_FWD_DEFINED__
#define __WbemLocalAddrRes_FWD_DEFINED__

#ifdef __cplusplus
typedef class WbemLocalAddrRes WbemLocalAddrRes;
#else
typedef struct WbemLocalAddrRes WbemLocalAddrRes;
#endif /* __cplusplus */

#endif 	/* __WbemLocalAddrRes_FWD_DEFINED__ */


#ifndef __WbemUninitializedClassObject_FWD_DEFINED__
#define __WbemUninitializedClassObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class WbemUninitializedClassObject WbemUninitializedClassObject;
#else
typedef struct WbemUninitializedClassObject WbemUninitializedClassObject;
#endif /* __cplusplus */

#endif 	/* __WbemUninitializedClassObject_FWD_DEFINED__ */


#ifndef __IWbemLevel1Login_FWD_DEFINED__
#define __IWbemLevel1Login_FWD_DEFINED__
typedef interface IWbemLevel1Login IWbemLevel1Login;
#endif 	/* __IWbemLevel1Login_FWD_DEFINED__ */


#ifndef __IWbemClientTransport_FWD_DEFINED__
#define __IWbemClientTransport_FWD_DEFINED__
typedef interface IWbemClientTransport IWbemClientTransport;
#endif 	/* __IWbemClientTransport_FWD_DEFINED__ */


#ifndef __IWbemAddressResolution_FWD_DEFINED__
#define __IWbemAddressResolution_FWD_DEFINED__
typedef interface IWbemAddressResolution IWbemAddressResolution;
#endif 	/* __IWbemAddressResolution_FWD_DEFINED__ */


#ifndef __IWbemTransport_FWD_DEFINED__
#define __IWbemTransport_FWD_DEFINED__
typedef interface IWbemTransport IWbemTransport;
#endif 	/* __IWbemTransport_FWD_DEFINED__ */


#ifndef __IWbemConstructClassObject_FWD_DEFINED__
#define __IWbemConstructClassObject_FWD_DEFINED__
typedef interface IWbemConstructClassObject IWbemConstructClassObject;
#endif 	/* __IWbemConstructClassObject_FWD_DEFINED__ */


/* header files for imported files */
#include "objidl.h"
#include "oleidl.h"
#include "oaidl.h"
#include "wbemcli.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __WbemTransports_v1_LIBRARY_DEFINED__
#define __WbemTransports_v1_LIBRARY_DEFINED__

/* library WbemTransports_v1 */
/* [uuid] */ 





typedef 
enum tag_WBEM_LOGIN_TYPE
    {	WBEM_FLAG_INPROC_LOGIN	= 0,
	WBEM_FLAG_LOCAL_LOGIN	= 1,
	WBEM_FLAG_REMOTE_LOGIN	= 2,
	WBEM_AUTHENTICATION_METHOD_MASK	= 0xf,
	WBEM_FLAG_USE_MULTIPLE_CHALLENGES	= 0x10
    }	WBEM_LOGIN_TYPE;

typedef /* [length_is][size_is] */ BYTE __RPC_FAR *WBEM_128BITS;


EXTERN_C const IID LIBID_WbemTransports_v1;

#ifndef __IWbemTransport_INTERFACE_DEFINED__
#define __IWbemTransport_INTERFACE_DEFINED__

/* interface IWbemTransport */
/* [uuid][object][local][restricted] */ 


EXTERN_C const IID IID_IWbemTransport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("553fe584-2156-11d0-b6ae-00aa003240c7")
    IWbemTransport : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWbemTransportVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemTransport __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemTransport __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemTransport __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Initialize )( 
            IWbemTransport __RPC_FAR * This);
        
        END_INTERFACE
    } IWbemTransportVtbl;

    interface IWbemTransport
    {
        CONST_VTBL struct IWbemTransportVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemTransport_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemTransport_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemTransport_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemTransport_Initialize(This)	\
    (This)->lpVtbl -> Initialize(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IWbemTransport_Initialize_Proxy( 
    IWbemTransport __RPC_FAR * This);


void __RPC_STUB IWbemTransport_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWbemTransport_INTERFACE_DEFINED__ */


#ifndef __IWbemLevel1Login_INTERFACE_DEFINED__
#define __IWbemLevel1Login_INTERFACE_DEFINED__

/* interface IWbemLevel1Login */
/* [unique][uuid][restricted][object] */ 


EXTERN_C const IID IID_IWbemLevel1Login;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F309AD18-D86A-11d0-A075-00C04FB68820")
    IWbemLevel1Login : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EstablishPosition( 
            /* [string][unique][in] */ LPWSTR wszClientMachineName,
            /* [in] */ DWORD dwProcessId,
            /* [out] */ DWORD __RPC_FAR *phAuthEventHandle) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RequestChallenge( 
            /* [string][unique][in] */ LPWSTR wszNetworkResource,
            /* [string][unique][in] */ LPWSTR wszUser,
            /* [out] */ WBEM_128BITS Nonce) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WBEMLogin( 
            /* [string][unique][in] */ LPWSTR wszPreferredLocale,
            /* [unique][in] */ WBEM_128BITS AccessToken,
            /* [in] */ long lFlags,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [out] */ IWbemServices __RPC_FAR *__RPC_FAR *ppNamespace) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NTLMLogin( 
            /* [string][unique][in] */ LPWSTR wszNetworkResource,
            /* [string][unique][in] */ LPWSTR wszPreferredLocale,
            /* [in] */ long lFlags,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [out] */ IWbemServices __RPC_FAR *__RPC_FAR *ppNamespace) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWbemLevel1LoginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemLevel1Login __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemLevel1Login __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemLevel1Login __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EstablishPosition )( 
            IWbemLevel1Login __RPC_FAR * This,
            /* [string][unique][in] */ LPWSTR wszClientMachineName,
            /* [in] */ DWORD dwProcessId,
            /* [out] */ DWORD __RPC_FAR *phAuthEventHandle);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RequestChallenge )( 
            IWbemLevel1Login __RPC_FAR * This,
            /* [string][unique][in] */ LPWSTR wszNetworkResource,
            /* [string][unique][in] */ LPWSTR wszUser,
            /* [out] */ WBEM_128BITS Nonce);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WBEMLogin )( 
            IWbemLevel1Login __RPC_FAR * This,
            /* [string][unique][in] */ LPWSTR wszPreferredLocale,
            /* [unique][in] */ WBEM_128BITS AccessToken,
            /* [in] */ long lFlags,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [out] */ IWbemServices __RPC_FAR *__RPC_FAR *ppNamespace);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NTLMLogin )( 
            IWbemLevel1Login __RPC_FAR * This,
            /* [string][unique][in] */ LPWSTR wszNetworkResource,
            /* [string][unique][in] */ LPWSTR wszPreferredLocale,
            /* [in] */ long lFlags,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [out] */ IWbemServices __RPC_FAR *__RPC_FAR *ppNamespace);
        
        END_INTERFACE
    } IWbemLevel1LoginVtbl;

    interface IWbemLevel1Login
    {
        CONST_VTBL struct IWbemLevel1LoginVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemLevel1Login_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemLevel1Login_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemLevel1Login_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemLevel1Login_EstablishPosition(This,wszClientMachineName,dwProcessId,phAuthEventHandle)	\
    (This)->lpVtbl -> EstablishPosition(This,wszClientMachineName,dwProcessId,phAuthEventHandle)

#define IWbemLevel1Login_RequestChallenge(This,wszNetworkResource,wszUser,Nonce)	\
    (This)->lpVtbl -> RequestChallenge(This,wszNetworkResource,wszUser,Nonce)

#define IWbemLevel1Login_WBEMLogin(This,wszPreferredLocale,AccessToken,lFlags,pCtx,ppNamespace)	\
    (This)->lpVtbl -> WBEMLogin(This,wszPreferredLocale,AccessToken,lFlags,pCtx,ppNamespace)

#define IWbemLevel1Login_NTLMLogin(This,wszNetworkResource,wszPreferredLocale,lFlags,pCtx,ppNamespace)	\
    (This)->lpVtbl -> NTLMLogin(This,wszNetworkResource,wszPreferredLocale,lFlags,pCtx,ppNamespace)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IWbemLevel1Login_EstablishPosition_Proxy( 
    IWbemLevel1Login __RPC_FAR * This,
    /* [string][unique][in] */ LPWSTR wszClientMachineName,
    /* [in] */ DWORD dwProcessId,
    /* [out] */ DWORD __RPC_FAR *phAuthEventHandle);


void __RPC_STUB IWbemLevel1Login_EstablishPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemLevel1Login_RequestChallenge_Proxy( 
    IWbemLevel1Login __RPC_FAR * This,
    /* [string][unique][in] */ LPWSTR wszNetworkResource,
    /* [string][unique][in] */ LPWSTR wszUser,
    /* [out] */ WBEM_128BITS Nonce);


void __RPC_STUB IWbemLevel1Login_RequestChallenge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemLevel1Login_WBEMLogin_Proxy( 
    IWbemLevel1Login __RPC_FAR * This,
    /* [string][unique][in] */ LPWSTR wszPreferredLocale,
    /* [unique][in] */ WBEM_128BITS AccessToken,
    /* [in] */ long lFlags,
    /* [in] */ IWbemContext __RPC_FAR *pCtx,
    /* [out] */ IWbemServices __RPC_FAR *__RPC_FAR *ppNamespace);


void __RPC_STUB IWbemLevel1Login_WBEMLogin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemLevel1Login_NTLMLogin_Proxy( 
    IWbemLevel1Login __RPC_FAR * This,
    /* [string][unique][in] */ LPWSTR wszNetworkResource,
    /* [string][unique][in] */ LPWSTR wszPreferredLocale,
    /* [in] */ long lFlags,
    /* [in] */ IWbemContext __RPC_FAR *pCtx,
    /* [out] */ IWbemServices __RPC_FAR *__RPC_FAR *ppNamespace);


void __RPC_STUB IWbemLevel1Login_NTLMLogin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWbemLevel1Login_INTERFACE_DEFINED__ */


#ifndef __IWbemClientTransport_INTERFACE_DEFINED__
#define __IWbemClientTransport_INTERFACE_DEFINED__

/* interface IWbemClientTransport */
/* [unique][restricted][uuid][local][object] */ 


EXTERN_C const IID IID_IWbemClientTransport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F7CE2E11-8C90-11d1-9E7B-00C04FC324A8")
    IWbemClientTransport : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ConnectServer( 
            /* [in] */ BSTR strAddressType,
            /* [in] */ DWORD dwBinaryAddressLength,
            /* [size_is][in] */ BYTE __RPC_FAR *abBinaryAddress,
            /* [in] */ BSTR strNetworkResource,
            /* [in] */ BSTR strUser,
            /* [in] */ BSTR strPassword,
            /* [in] */ BSTR strLocale,
            /* [in] */ long lSecurityFlags,
            /* [in] */ BSTR strAuthority,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [out] */ IWbemServices __RPC_FAR *__RPC_FAR *ppNamespace) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWbemClientTransportVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemClientTransport __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemClientTransport __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemClientTransport __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ConnectServer )( 
            IWbemClientTransport __RPC_FAR * This,
            /* [in] */ BSTR strAddressType,
            /* [in] */ DWORD dwBinaryAddressLength,
            /* [size_is][in] */ BYTE __RPC_FAR *abBinaryAddress,
            /* [in] */ BSTR strNetworkResource,
            /* [in] */ BSTR strUser,
            /* [in] */ BSTR strPassword,
            /* [in] */ BSTR strLocale,
            /* [in] */ long lSecurityFlags,
            /* [in] */ BSTR strAuthority,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [out] */ IWbemServices __RPC_FAR *__RPC_FAR *ppNamespace);
        
        END_INTERFACE
    } IWbemClientTransportVtbl;

    interface IWbemClientTransport
    {
        CONST_VTBL struct IWbemClientTransportVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemClientTransport_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemClientTransport_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemClientTransport_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemClientTransport_ConnectServer(This,strAddressType,dwBinaryAddressLength,abBinaryAddress,strNetworkResource,strUser,strPassword,strLocale,lSecurityFlags,strAuthority,pCtx,ppNamespace)	\
    (This)->lpVtbl -> ConnectServer(This,strAddressType,dwBinaryAddressLength,abBinaryAddress,strNetworkResource,strUser,strPassword,strLocale,lSecurityFlags,strAuthority,pCtx,ppNamespace)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IWbemClientTransport_ConnectServer_Proxy( 
    IWbemClientTransport __RPC_FAR * This,
    /* [in] */ BSTR strAddressType,
    /* [in] */ DWORD dwBinaryAddressLength,
    /* [size_is][in] */ BYTE __RPC_FAR *abBinaryAddress,
    /* [in] */ BSTR strNetworkResource,
    /* [in] */ BSTR strUser,
    /* [in] */ BSTR strPassword,
    /* [in] */ BSTR strLocale,
    /* [in] */ long lSecurityFlags,
    /* [in] */ BSTR strAuthority,
    /* [in] */ IWbemContext __RPC_FAR *pCtx,
    /* [out] */ IWbemServices __RPC_FAR *__RPC_FAR *ppNamespace);


void __RPC_STUB IWbemClientTransport_ConnectServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWbemClientTransport_INTERFACE_DEFINED__ */


#ifndef __IWbemAddressResolution_INTERFACE_DEFINED__
#define __IWbemAddressResolution_INTERFACE_DEFINED__

/* interface IWbemAddressResolution */
/* [unique][restricted][uuid][local][object] */ 


EXTERN_C const IID IID_IWbemAddressResolution;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F7CE2E12-8C90-11d1-9E7B-00C04FC324A8")
    IWbemAddressResolution : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Resolve( 
            /* [in] */ LPWSTR wszNamespacePath,
            /* [out] */ LPWSTR wszAddressType,
            /* [out] */ DWORD __RPC_FAR *pdwAddressLength,
            /* [size_is][size_is][out] */ BYTE __RPC_FAR *__RPC_FAR *pabBinaryAddress) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWbemAddressResolutionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemAddressResolution __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemAddressResolution __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemAddressResolution __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Resolve )( 
            IWbemAddressResolution __RPC_FAR * This,
            /* [in] */ LPWSTR wszNamespacePath,
            /* [out] */ LPWSTR wszAddressType,
            /* [out] */ DWORD __RPC_FAR *pdwAddressLength,
            /* [size_is][size_is][out] */ BYTE __RPC_FAR *__RPC_FAR *pabBinaryAddress);
        
        END_INTERFACE
    } IWbemAddressResolutionVtbl;

    interface IWbemAddressResolution
    {
        CONST_VTBL struct IWbemAddressResolutionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemAddressResolution_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemAddressResolution_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemAddressResolution_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemAddressResolution_Resolve(This,wszNamespacePath,wszAddressType,pdwAddressLength,pabBinaryAddress)	\
    (This)->lpVtbl -> Resolve(This,wszNamespacePath,wszAddressType,pdwAddressLength,pabBinaryAddress)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IWbemAddressResolution_Resolve_Proxy( 
    IWbemAddressResolution __RPC_FAR * This,
    /* [in] */ LPWSTR wszNamespacePath,
    /* [out] */ LPWSTR wszAddressType,
    /* [out] */ DWORD __RPC_FAR *pdwAddressLength,
    /* [size_is][size_is][out] */ BYTE __RPC_FAR *__RPC_FAR *pabBinaryAddress);


void __RPC_STUB IWbemAddressResolution_Resolve_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWbemAddressResolution_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_WbemLevel1Login;

#ifdef __cplusplus

class DECLSPEC_UUID("8BC3F05E-D86B-11d0-A075-00C04FB68820")
WbemLevel1Login;
#endif

EXTERN_C const CLSID CLSID_WbemDCOMTransport;

#ifdef __cplusplus

class DECLSPEC_UUID("F7CE2E13-8C90-11d1-9E7B-00C04FC324A8")
WbemDCOMTransport;
#endif

EXTERN_C const CLSID CLSID_WbemLocalAddrRes;

#ifdef __cplusplus

class DECLSPEC_UUID("A1044801-8F7E-11d1-9E7C-00C04FC324A8")
WbemLocalAddrRes;
#endif

EXTERN_C const CLSID CLSID_WbemUninitializedClassObject;

#ifdef __cplusplus

class DECLSPEC_UUID("7a0227f6-7108-11d1-ad90-00c04fd8fdff")
WbemUninitializedClassObject;
#endif
#endif /* __WbemTransports_v1_LIBRARY_DEFINED__ */

#ifndef __IWbemConstructClassObject_INTERFACE_DEFINED__
#define __IWbemConstructClassObject_INTERFACE_DEFINED__

/* interface IWbemConstructClassObject */
/* [uuid][object][local][restricted] */ 


EXTERN_C const IID IID_IWbemConstructClassObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9ef76194-70d5-11d1-ad90-00c04fd8fdff")
    IWbemConstructClassObject : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetInheritanceChain( 
            /* [in] */ long lNumAntecedents,
            /* [string][size_is][in] */ LPWSTR __RPC_FAR *awszAntecedents) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPropertyOrigin( 
            /* [string][in] */ LPCWSTR wszPropertyName,
            /* [in] */ long lOriginIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMethodOrigin( 
            /* [string][in] */ LPCWSTR wszMethodName,
            /* [in] */ long lOriginIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetServerNamespace( 
            /* [string][in] */ LPCWSTR wszServer,
            /* [string][in] */ LPCWSTR wszNamespace) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWbemConstructClassObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemConstructClassObject __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemConstructClassObject __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemConstructClassObject __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetInheritanceChain )( 
            IWbemConstructClassObject __RPC_FAR * This,
            /* [in] */ long lNumAntecedents,
            /* [string][size_is][in] */ LPWSTR __RPC_FAR *awszAntecedents);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetPropertyOrigin )( 
            IWbemConstructClassObject __RPC_FAR * This,
            /* [string][in] */ LPCWSTR wszPropertyName,
            /* [in] */ long lOriginIndex);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetMethodOrigin )( 
            IWbemConstructClassObject __RPC_FAR * This,
            /* [string][in] */ LPCWSTR wszMethodName,
            /* [in] */ long lOriginIndex);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetServerNamespace )( 
            IWbemConstructClassObject __RPC_FAR * This,
            /* [string][in] */ LPCWSTR wszServer,
            /* [string][in] */ LPCWSTR wszNamespace);
        
        END_INTERFACE
    } IWbemConstructClassObjectVtbl;

    interface IWbemConstructClassObject
    {
        CONST_VTBL struct IWbemConstructClassObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemConstructClassObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemConstructClassObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemConstructClassObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemConstructClassObject_SetInheritanceChain(This,lNumAntecedents,awszAntecedents)	\
    (This)->lpVtbl -> SetInheritanceChain(This,lNumAntecedents,awszAntecedents)

#define IWbemConstructClassObject_SetPropertyOrigin(This,wszPropertyName,lOriginIndex)	\
    (This)->lpVtbl -> SetPropertyOrigin(This,wszPropertyName,lOriginIndex)

#define IWbemConstructClassObject_SetMethodOrigin(This,wszMethodName,lOriginIndex)	\
    (This)->lpVtbl -> SetMethodOrigin(This,wszMethodName,lOriginIndex)

#define IWbemConstructClassObject_SetServerNamespace(This,wszServer,wszNamespace)	\
    (This)->lpVtbl -> SetServerNamespace(This,wszServer,wszNamespace)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IWbemConstructClassObject_SetInheritanceChain_Proxy( 
    IWbemConstructClassObject __RPC_FAR * This,
    /* [in] */ long lNumAntecedents,
    /* [string][size_is][in] */ LPWSTR __RPC_FAR *awszAntecedents);


void __RPC_STUB IWbemConstructClassObject_SetInheritanceChain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemConstructClassObject_SetPropertyOrigin_Proxy( 
    IWbemConstructClassObject __RPC_FAR * This,
    /* [string][in] */ LPCWSTR wszPropertyName,
    /* [in] */ long lOriginIndex);


void __RPC_STUB IWbemConstructClassObject_SetPropertyOrigin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemConstructClassObject_SetMethodOrigin_Proxy( 
    IWbemConstructClassObject __RPC_FAR * This,
    /* [string][in] */ LPCWSTR wszMethodName,
    /* [in] */ long lOriginIndex);


void __RPC_STUB IWbemConstructClassObject_SetMethodOrigin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemConstructClassObject_SetServerNamespace_Proxy( 
    IWbemConstructClassObject __RPC_FAR * This,
    /* [string][in] */ LPCWSTR wszServer,
    /* [string][in] */ LPCWSTR wszNamespace);


void __RPC_STUB IWbemConstructClassObject_SetServerNamespace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWbemConstructClassObject_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



