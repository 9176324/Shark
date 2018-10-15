
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 5.03.0279 */
/* at Wed Mar 12 13:25:07 2003
 */
/* Compiler settings for netcfgx.idl:
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

#ifndef __netcfgx_h__
#define __netcfgx_h__

/* Forward Declarations */ 

#ifndef __IEnumNetCfgBindingInterface_FWD_DEFINED__
#define __IEnumNetCfgBindingInterface_FWD_DEFINED__
typedef interface IEnumNetCfgBindingInterface IEnumNetCfgBindingInterface;
#endif 	/* __IEnumNetCfgBindingInterface_FWD_DEFINED__ */


#ifndef __IEnumNetCfgBindingPath_FWD_DEFINED__
#define __IEnumNetCfgBindingPath_FWD_DEFINED__
typedef interface IEnumNetCfgBindingPath IEnumNetCfgBindingPath;
#endif 	/* __IEnumNetCfgBindingPath_FWD_DEFINED__ */


#ifndef __IEnumNetCfgComponent_FWD_DEFINED__
#define __IEnumNetCfgComponent_FWD_DEFINED__
typedef interface IEnumNetCfgComponent IEnumNetCfgComponent;
#endif 	/* __IEnumNetCfgComponent_FWD_DEFINED__ */


#ifndef __INetCfg_FWD_DEFINED__
#define __INetCfg_FWD_DEFINED__
typedef interface INetCfg INetCfg;
#endif 	/* __INetCfg_FWD_DEFINED__ */


#ifndef __INetCfgLock_FWD_DEFINED__
#define __INetCfgLock_FWD_DEFINED__
typedef interface INetCfgLock INetCfgLock;
#endif 	/* __INetCfgLock_FWD_DEFINED__ */


#ifndef __INetCfgBindingInterface_FWD_DEFINED__
#define __INetCfgBindingInterface_FWD_DEFINED__
typedef interface INetCfgBindingInterface INetCfgBindingInterface;
#endif 	/* __INetCfgBindingInterface_FWD_DEFINED__ */


#ifndef __INetCfgBindingPath_FWD_DEFINED__
#define __INetCfgBindingPath_FWD_DEFINED__
typedef interface INetCfgBindingPath INetCfgBindingPath;
#endif 	/* __INetCfgBindingPath_FWD_DEFINED__ */


#ifndef __INetCfgClass_FWD_DEFINED__
#define __INetCfgClass_FWD_DEFINED__
typedef interface INetCfgClass INetCfgClass;
#endif 	/* __INetCfgClass_FWD_DEFINED__ */


#ifndef __INetCfgClassSetup_FWD_DEFINED__
#define __INetCfgClassSetup_FWD_DEFINED__
typedef interface INetCfgClassSetup INetCfgClassSetup;
#endif 	/* __INetCfgClassSetup_FWD_DEFINED__ */


#ifndef __INetCfgComponent_FWD_DEFINED__
#define __INetCfgComponent_FWD_DEFINED__
typedef interface INetCfgComponent INetCfgComponent;
#endif 	/* __INetCfgComponent_FWD_DEFINED__ */


#ifndef __INetCfgComponentBindings_FWD_DEFINED__
#define __INetCfgComponentBindings_FWD_DEFINED__
typedef interface INetCfgComponentBindings INetCfgComponentBindings;
#endif 	/* __INetCfgComponentBindings_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"
#include "prsht.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_netcfgx_0000 */
/* [local] */ 

//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992-1999.
//
//--------------------------------------------------------------------------
#if ( _MSC_VER >= 800 )
#pragma warning(disable:4201)
#endif

EXTERN_C const CLSID CLSID_CNetCfg;

#define NETCFG_E_ALREADY_INITIALIZED                 MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA020)
#define NETCFG_E_NOT_INITIALIZED                     MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA021)
#define NETCFG_E_IN_USE                              MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA022)
#define NETCFG_E_NO_WRITE_LOCK                       MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA024)
#define NETCFG_E_NEED_REBOOT                         MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA025)
#define NETCFG_E_ACTIVE_RAS_CONNECTIONS              MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA026)
#define NETCFG_E_ADAPTER_NOT_FOUND                   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA027)
#define NETCFG_E_COMPONENT_REMOVED_PENDING_REBOOT    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xA028)
#define NETCFG_S_REBOOT                              MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0xA020)
#define NETCFG_S_DISABLE_QUERY                       MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0xA022)
#define NETCFG_S_STILL_REFERENCED                    MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0xA023)
#define NETCFG_S_CAUSED_SETUP_CHANGE                 MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0xA024)

#define NETCFG_CLIENT_CID_MS_MSClient        TEXT("ms_msclient")
#define NETCFG_SERVICE_CID_MS_SERVER         TEXT("ms_server")
#define NETCFG_SERVICE_CID_MS_NETBIOS        TEXT("ms_netbios")
#define NETCFG_TRANS_CID_MS_APPLETALK        TEXT("ms_appletalk")
#define NETCFG_TRANS_CID_MS_NETBEUI          TEXT("ms_netbeui")
#define NETCFG_TRANS_CID_MS_NETMON           TEXT("ms_netmon")
#define NETCFG_TRANS_CID_MS_NWIPX            TEXT("ms_nwipx")
#define NETCFG_TRANS_CID_MS_NWSPX            TEXT("ms_nwspx")
#define NETCFG_TRANS_CID_MS_TCPIP            TEXT("ms_tcpip")
















extern RPC_IF_HANDLE __MIDL_itf_netcfgx_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_netcfgx_0000_v0_0_s_ifspec;

#ifndef __IEnumNetCfgBindingInterface_INTERFACE_DEFINED__
#define __IEnumNetCfgBindingInterface_INTERFACE_DEFINED__

/* interface IEnumNetCfgBindingInterface */
/* [unique][uuid][object][local] */ 


EXTERN_C const IID IID_IEnumNetCfgBindingInterface;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C0E8AE90-306E-11D1-AACF-00805FC1270E")
    IEnumNetCfgBindingInterface : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ INetCfgBindingInterface __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumNetCfgBindingInterface __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumNetCfgBindingInterfaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumNetCfgBindingInterface __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumNetCfgBindingInterface __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumNetCfgBindingInterface __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumNetCfgBindingInterface __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ INetCfgBindingInterface __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumNetCfgBindingInterface __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumNetCfgBindingInterface __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumNetCfgBindingInterface __RPC_FAR * This,
            /* [out] */ IEnumNetCfgBindingInterface __RPC_FAR *__RPC_FAR *ppenum);
        
        END_INTERFACE
    } IEnumNetCfgBindingInterfaceVtbl;

    interface IEnumNetCfgBindingInterface
    {
        CONST_VTBL struct IEnumNetCfgBindingInterfaceVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumNetCfgBindingInterface_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumNetCfgBindingInterface_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumNetCfgBindingInterface_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumNetCfgBindingInterface_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumNetCfgBindingInterface_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumNetCfgBindingInterface_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumNetCfgBindingInterface_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IEnumNetCfgBindingInterface_Next_Proxy( 
    IEnumNetCfgBindingInterface __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ INetCfgBindingInterface __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumNetCfgBindingInterface_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetCfgBindingInterface_Skip_Proxy( 
    IEnumNetCfgBindingInterface __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumNetCfgBindingInterface_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetCfgBindingInterface_Reset_Proxy( 
    IEnumNetCfgBindingInterface __RPC_FAR * This);


void __RPC_STUB IEnumNetCfgBindingInterface_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetCfgBindingInterface_Clone_Proxy( 
    IEnumNetCfgBindingInterface __RPC_FAR * This,
    /* [out] */ IEnumNetCfgBindingInterface __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumNetCfgBindingInterface_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumNetCfgBindingInterface_INTERFACE_DEFINED__ */


#ifndef __IEnumNetCfgBindingPath_INTERFACE_DEFINED__
#define __IEnumNetCfgBindingPath_INTERFACE_DEFINED__

/* interface IEnumNetCfgBindingPath */
/* [unique][uuid][object][local] */ 


EXTERN_C const IID IID_IEnumNetCfgBindingPath;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C0E8AE91-306E-11D1-AACF-00805FC1270E")
    IEnumNetCfgBindingPath : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ INetCfgBindingPath __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumNetCfgBindingPath __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumNetCfgBindingPathVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumNetCfgBindingPath __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumNetCfgBindingPath __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumNetCfgBindingPath __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumNetCfgBindingPath __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ INetCfgBindingPath __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumNetCfgBindingPath __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumNetCfgBindingPath __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumNetCfgBindingPath __RPC_FAR * This,
            /* [out] */ IEnumNetCfgBindingPath __RPC_FAR *__RPC_FAR *ppenum);
        
        END_INTERFACE
    } IEnumNetCfgBindingPathVtbl;

    interface IEnumNetCfgBindingPath
    {
        CONST_VTBL struct IEnumNetCfgBindingPathVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumNetCfgBindingPath_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumNetCfgBindingPath_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumNetCfgBindingPath_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumNetCfgBindingPath_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumNetCfgBindingPath_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumNetCfgBindingPath_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumNetCfgBindingPath_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IEnumNetCfgBindingPath_Next_Proxy( 
    IEnumNetCfgBindingPath __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ INetCfgBindingPath __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumNetCfgBindingPath_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetCfgBindingPath_Skip_Proxy( 
    IEnumNetCfgBindingPath __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumNetCfgBindingPath_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetCfgBindingPath_Reset_Proxy( 
    IEnumNetCfgBindingPath __RPC_FAR * This);


void __RPC_STUB IEnumNetCfgBindingPath_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetCfgBindingPath_Clone_Proxy( 
    IEnumNetCfgBindingPath __RPC_FAR * This,
    /* [out] */ IEnumNetCfgBindingPath __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumNetCfgBindingPath_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumNetCfgBindingPath_INTERFACE_DEFINED__ */


#ifndef __IEnumNetCfgComponent_INTERFACE_DEFINED__
#define __IEnumNetCfgComponent_INTERFACE_DEFINED__

/* interface IEnumNetCfgComponent */
/* [unique][uuid][object][local] */ 


EXTERN_C const IID IID_IEnumNetCfgComponent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C0E8AE92-306E-11D1-AACF-00805FC1270E")
    IEnumNetCfgComponent : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumNetCfgComponent __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumNetCfgComponentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumNetCfgComponent __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumNetCfgComponent __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumNetCfgComponent __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumNetCfgComponent __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumNetCfgComponent __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumNetCfgComponent __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumNetCfgComponent __RPC_FAR * This,
            /* [out] */ IEnumNetCfgComponent __RPC_FAR *__RPC_FAR *ppenum);
        
        END_INTERFACE
    } IEnumNetCfgComponentVtbl;

    interface IEnumNetCfgComponent
    {
        CONST_VTBL struct IEnumNetCfgComponentVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumNetCfgComponent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumNetCfgComponent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumNetCfgComponent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumNetCfgComponent_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumNetCfgComponent_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumNetCfgComponent_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumNetCfgComponent_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IEnumNetCfgComponent_Next_Proxy( 
    IEnumNetCfgComponent __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumNetCfgComponent_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetCfgComponent_Skip_Proxy( 
    IEnumNetCfgComponent __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumNetCfgComponent_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetCfgComponent_Reset_Proxy( 
    IEnumNetCfgComponent __RPC_FAR * This);


void __RPC_STUB IEnumNetCfgComponent_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNetCfgComponent_Clone_Proxy( 
    IEnumNetCfgComponent __RPC_FAR * This,
    /* [out] */ IEnumNetCfgComponent __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumNetCfgComponent_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumNetCfgComponent_INTERFACE_DEFINED__ */


#ifndef __INetCfg_INTERFACE_DEFINED__
#define __INetCfg_INTERFACE_DEFINED__

/* interface INetCfg */
/* [unique][uuid][object][local] */ 


EXTERN_C const IID IID_INetCfg;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C0E8AE93-306E-11D1-AACF-00805FC1270E")
    INetCfg : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
            /* [in] */ PVOID pvReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Uninitialize( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Apply( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumComponents( 
            /* [in] */ const GUID __RPC_FAR *pguidClass,
            /* [out] */ IEnumNetCfgComponent __RPC_FAR *__RPC_FAR *ppenumComponent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindComponent( 
            /* [string][in] */ LPCWSTR pszwInfId,
            /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *pComponent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryNetCfgClass( 
            /* [in] */ const GUID __RPC_FAR *pguidClass,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct INetCfgVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INetCfg __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INetCfg __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INetCfg __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Initialize )( 
            INetCfg __RPC_FAR * This,
            /* [in] */ PVOID pvReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Uninitialize )( 
            INetCfg __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Apply )( 
            INetCfg __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Cancel )( 
            INetCfg __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumComponents )( 
            INetCfg __RPC_FAR * This,
            /* [in] */ const GUID __RPC_FAR *pguidClass,
            /* [out] */ IEnumNetCfgComponent __RPC_FAR *__RPC_FAR *ppenumComponent);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindComponent )( 
            INetCfg __RPC_FAR * This,
            /* [string][in] */ LPCWSTR pszwInfId,
            /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *pComponent);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryNetCfgClass )( 
            INetCfg __RPC_FAR * This,
            /* [in] */ const GUID __RPC_FAR *pguidClass,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        END_INTERFACE
    } INetCfgVtbl;

    interface INetCfg
    {
        CONST_VTBL struct INetCfgVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetCfg_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetCfg_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetCfg_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetCfg_Initialize(This,pvReserved)	\
    (This)->lpVtbl -> Initialize(This,pvReserved)

#define INetCfg_Uninitialize(This)	\
    (This)->lpVtbl -> Uninitialize(This)

#define INetCfg_Apply(This)	\
    (This)->lpVtbl -> Apply(This)

#define INetCfg_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)

#define INetCfg_EnumComponents(This,pguidClass,ppenumComponent)	\
    (This)->lpVtbl -> EnumComponents(This,pguidClass,ppenumComponent)

#define INetCfg_FindComponent(This,pszwInfId,pComponent)	\
    (This)->lpVtbl -> FindComponent(This,pszwInfId,pComponent)

#define INetCfg_QueryNetCfgClass(This,pguidClass,riid,ppvObject)	\
    (This)->lpVtbl -> QueryNetCfgClass(This,pguidClass,riid,ppvObject)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE INetCfg_Initialize_Proxy( 
    INetCfg __RPC_FAR * This,
    /* [in] */ PVOID pvReserved);


void __RPC_STUB INetCfg_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfg_Uninitialize_Proxy( 
    INetCfg __RPC_FAR * This);


void __RPC_STUB INetCfg_Uninitialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfg_Apply_Proxy( 
    INetCfg __RPC_FAR * This);


void __RPC_STUB INetCfg_Apply_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfg_Cancel_Proxy( 
    INetCfg __RPC_FAR * This);


void __RPC_STUB INetCfg_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfg_EnumComponents_Proxy( 
    INetCfg __RPC_FAR * This,
    /* [in] */ const GUID __RPC_FAR *pguidClass,
    /* [out] */ IEnumNetCfgComponent __RPC_FAR *__RPC_FAR *ppenumComponent);


void __RPC_STUB INetCfg_EnumComponents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfg_FindComponent_Proxy( 
    INetCfg __RPC_FAR * This,
    /* [string][in] */ LPCWSTR pszwInfId,
    /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *pComponent);


void __RPC_STUB INetCfg_FindComponent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfg_QueryNetCfgClass_Proxy( 
    INetCfg __RPC_FAR * This,
    /* [in] */ const GUID __RPC_FAR *pguidClass,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);


void __RPC_STUB INetCfg_QueryNetCfgClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __INetCfg_INTERFACE_DEFINED__ */


#ifndef __INetCfgLock_INTERFACE_DEFINED__
#define __INetCfgLock_INTERFACE_DEFINED__

/* interface INetCfgLock */
/* [unique][uuid][object][local] */ 


EXTERN_C const IID IID_INetCfgLock;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C0E8AE9F-306E-11D1-AACF-00805FC1270E")
    INetCfgLock : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AcquireWriteLock( 
            /* [in] */ DWORD cmsTimeout,
            /* [string][in] */ LPCWSTR pszwClientDescription,
            /* [string][out] */ LPWSTR __RPC_FAR *ppszwClientDescription) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseWriteLock( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsWriteLocked( 
            /* [string][out] */ LPWSTR __RPC_FAR *ppszwClientDescription) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct INetCfgLockVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INetCfgLock __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INetCfgLock __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INetCfgLock __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AcquireWriteLock )( 
            INetCfgLock __RPC_FAR * This,
            /* [in] */ DWORD cmsTimeout,
            /* [string][in] */ LPCWSTR pszwClientDescription,
            /* [string][out] */ LPWSTR __RPC_FAR *ppszwClientDescription);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseWriteLock )( 
            INetCfgLock __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsWriteLocked )( 
            INetCfgLock __RPC_FAR * This,
            /* [string][out] */ LPWSTR __RPC_FAR *ppszwClientDescription);
        
        END_INTERFACE
    } INetCfgLockVtbl;

    interface INetCfgLock
    {
        CONST_VTBL struct INetCfgLockVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetCfgLock_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetCfgLock_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetCfgLock_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetCfgLock_AcquireWriteLock(This,cmsTimeout,pszwClientDescription,ppszwClientDescription)	\
    (This)->lpVtbl -> AcquireWriteLock(This,cmsTimeout,pszwClientDescription,ppszwClientDescription)

#define INetCfgLock_ReleaseWriteLock(This)	\
    (This)->lpVtbl -> ReleaseWriteLock(This)

#define INetCfgLock_IsWriteLocked(This,ppszwClientDescription)	\
    (This)->lpVtbl -> IsWriteLocked(This,ppszwClientDescription)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE INetCfgLock_AcquireWriteLock_Proxy( 
    INetCfgLock __RPC_FAR * This,
    /* [in] */ DWORD cmsTimeout,
    /* [string][in] */ LPCWSTR pszwClientDescription,
    /* [string][out] */ LPWSTR __RPC_FAR *ppszwClientDescription);


void __RPC_STUB INetCfgLock_AcquireWriteLock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgLock_ReleaseWriteLock_Proxy( 
    INetCfgLock __RPC_FAR * This);


void __RPC_STUB INetCfgLock_ReleaseWriteLock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgLock_IsWriteLocked_Proxy( 
    INetCfgLock __RPC_FAR * This,
    /* [string][out] */ LPWSTR __RPC_FAR *ppszwClientDescription);


void __RPC_STUB INetCfgLock_IsWriteLocked_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __INetCfgLock_INTERFACE_DEFINED__ */


#ifndef __INetCfgBindingInterface_INTERFACE_DEFINED__
#define __INetCfgBindingInterface_INTERFACE_DEFINED__

/* interface INetCfgBindingInterface */
/* [unique][uuid][object][local] */ 


EXTERN_C const IID IID_INetCfgBindingInterface;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C0E8AE94-306E-11D1-AACF-00805FC1270E")
    INetCfgBindingInterface : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName( 
            /* [string][out] */ LPWSTR __RPC_FAR *ppszwInterfaceName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUpperComponent( 
            /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppnccItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLowerComponent( 
            /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppnccItem) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct INetCfgBindingInterfaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INetCfgBindingInterface __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INetCfgBindingInterface __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INetCfgBindingInterface __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            INetCfgBindingInterface __RPC_FAR * This,
            /* [string][out] */ LPWSTR __RPC_FAR *ppszwInterfaceName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetUpperComponent )( 
            INetCfgBindingInterface __RPC_FAR * This,
            /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppnccItem);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLowerComponent )( 
            INetCfgBindingInterface __RPC_FAR * This,
            /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppnccItem);
        
        END_INTERFACE
    } INetCfgBindingInterfaceVtbl;

    interface INetCfgBindingInterface
    {
        CONST_VTBL struct INetCfgBindingInterfaceVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetCfgBindingInterface_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetCfgBindingInterface_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetCfgBindingInterface_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetCfgBindingInterface_GetName(This,ppszwInterfaceName)	\
    (This)->lpVtbl -> GetName(This,ppszwInterfaceName)

#define INetCfgBindingInterface_GetUpperComponent(This,ppnccItem)	\
    (This)->lpVtbl -> GetUpperComponent(This,ppnccItem)

#define INetCfgBindingInterface_GetLowerComponent(This,ppnccItem)	\
    (This)->lpVtbl -> GetLowerComponent(This,ppnccItem)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE INetCfgBindingInterface_GetName_Proxy( 
    INetCfgBindingInterface __RPC_FAR * This,
    /* [string][out] */ LPWSTR __RPC_FAR *ppszwInterfaceName);


void __RPC_STUB INetCfgBindingInterface_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgBindingInterface_GetUpperComponent_Proxy( 
    INetCfgBindingInterface __RPC_FAR * This,
    /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppnccItem);


void __RPC_STUB INetCfgBindingInterface_GetUpperComponent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgBindingInterface_GetLowerComponent_Proxy( 
    INetCfgBindingInterface __RPC_FAR * This,
    /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppnccItem);


void __RPC_STUB INetCfgBindingInterface_GetLowerComponent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __INetCfgBindingInterface_INTERFACE_DEFINED__ */


#ifndef __INetCfgBindingPath_INTERFACE_DEFINED__
#define __INetCfgBindingPath_INTERFACE_DEFINED__

/* interface INetCfgBindingPath */
/* [unique][uuid][object][local] */ 


EXTERN_C const IID IID_INetCfgBindingPath;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C0E8AE96-306E-11D1-AACF-00805FC1270E")
    INetCfgBindingPath : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsSamePathAs( 
            /* [in] */ INetCfgBindingPath __RPC_FAR *pPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsSubPathOf( 
            /* [in] */ INetCfgBindingPath __RPC_FAR *pPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsEnabled( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enable( 
            /* [in] */ BOOL fEnable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPathToken( 
            /* [string][out] */ LPWSTR __RPC_FAR *ppszwPathToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOwner( 
            /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppComponent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDepth( 
            /* [out] */ ULONG __RPC_FAR *pcInterfaces) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumBindingInterfaces( 
            /* [out] */ IEnumNetCfgBindingInterface __RPC_FAR *__RPC_FAR *ppenumInterface) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct INetCfgBindingPathVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INetCfgBindingPath __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INetCfgBindingPath __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INetCfgBindingPath __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsSamePathAs )( 
            INetCfgBindingPath __RPC_FAR * This,
            /* [in] */ INetCfgBindingPath __RPC_FAR *pPath);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsSubPathOf )( 
            INetCfgBindingPath __RPC_FAR * This,
            /* [in] */ INetCfgBindingPath __RPC_FAR *pPath);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsEnabled )( 
            INetCfgBindingPath __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Enable )( 
            INetCfgBindingPath __RPC_FAR * This,
            /* [in] */ BOOL fEnable);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPathToken )( 
            INetCfgBindingPath __RPC_FAR * This,
            /* [string][out] */ LPWSTR __RPC_FAR *ppszwPathToken);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOwner )( 
            INetCfgBindingPath __RPC_FAR * This,
            /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppComponent);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDepth )( 
            INetCfgBindingPath __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pcInterfaces);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumBindingInterfaces )( 
            INetCfgBindingPath __RPC_FAR * This,
            /* [out] */ IEnumNetCfgBindingInterface __RPC_FAR *__RPC_FAR *ppenumInterface);
        
        END_INTERFACE
    } INetCfgBindingPathVtbl;

    interface INetCfgBindingPath
    {
        CONST_VTBL struct INetCfgBindingPathVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetCfgBindingPath_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetCfgBindingPath_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetCfgBindingPath_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetCfgBindingPath_IsSamePathAs(This,pPath)	\
    (This)->lpVtbl -> IsSamePathAs(This,pPath)

#define INetCfgBindingPath_IsSubPathOf(This,pPath)	\
    (This)->lpVtbl -> IsSubPathOf(This,pPath)

#define INetCfgBindingPath_IsEnabled(This)	\
    (This)->lpVtbl -> IsEnabled(This)

#define INetCfgBindingPath_Enable(This,fEnable)	\
    (This)->lpVtbl -> Enable(This,fEnable)

#define INetCfgBindingPath_GetPathToken(This,ppszwPathToken)	\
    (This)->lpVtbl -> GetPathToken(This,ppszwPathToken)

#define INetCfgBindingPath_GetOwner(This,ppComponent)	\
    (This)->lpVtbl -> GetOwner(This,ppComponent)

#define INetCfgBindingPath_GetDepth(This,pcInterfaces)	\
    (This)->lpVtbl -> GetDepth(This,pcInterfaces)

#define INetCfgBindingPath_EnumBindingInterfaces(This,ppenumInterface)	\
    (This)->lpVtbl -> EnumBindingInterfaces(This,ppenumInterface)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE INetCfgBindingPath_IsSamePathAs_Proxy( 
    INetCfgBindingPath __RPC_FAR * This,
    /* [in] */ INetCfgBindingPath __RPC_FAR *pPath);


void __RPC_STUB INetCfgBindingPath_IsSamePathAs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgBindingPath_IsSubPathOf_Proxy( 
    INetCfgBindingPath __RPC_FAR * This,
    /* [in] */ INetCfgBindingPath __RPC_FAR *pPath);


void __RPC_STUB INetCfgBindingPath_IsSubPathOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgBindingPath_IsEnabled_Proxy( 
    INetCfgBindingPath __RPC_FAR * This);


void __RPC_STUB INetCfgBindingPath_IsEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgBindingPath_Enable_Proxy( 
    INetCfgBindingPath __RPC_FAR * This,
    /* [in] */ BOOL fEnable);


void __RPC_STUB INetCfgBindingPath_Enable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgBindingPath_GetPathToken_Proxy( 
    INetCfgBindingPath __RPC_FAR * This,
    /* [string][out] */ LPWSTR __RPC_FAR *ppszwPathToken);


void __RPC_STUB INetCfgBindingPath_GetPathToken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgBindingPath_GetOwner_Proxy( 
    INetCfgBindingPath __RPC_FAR * This,
    /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppComponent);


void __RPC_STUB INetCfgBindingPath_GetOwner_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgBindingPath_GetDepth_Proxy( 
    INetCfgBindingPath __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pcInterfaces);


void __RPC_STUB INetCfgBindingPath_GetDepth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgBindingPath_EnumBindingInterfaces_Proxy( 
    INetCfgBindingPath __RPC_FAR * This,
    /* [out] */ IEnumNetCfgBindingInterface __RPC_FAR *__RPC_FAR *ppenumInterface);


void __RPC_STUB INetCfgBindingPath_EnumBindingInterfaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __INetCfgBindingPath_INTERFACE_DEFINED__ */


#ifndef __INetCfgClass_INTERFACE_DEFINED__
#define __INetCfgClass_INTERFACE_DEFINED__

/* interface INetCfgClass */
/* [unique][uuid][object][local] */ 


EXTERN_C const IID IID_INetCfgClass;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C0E8AE97-306E-11D1-AACF-00805FC1270E")
    INetCfgClass : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FindComponent( 
            /* [string][in] */ LPCWSTR pszwInfId,
            /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppnccItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumComponents( 
            /* [out] */ IEnumNetCfgComponent __RPC_FAR *__RPC_FAR *ppenumComponent) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct INetCfgClassVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INetCfgClass __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INetCfgClass __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INetCfgClass __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindComponent )( 
            INetCfgClass __RPC_FAR * This,
            /* [string][in] */ LPCWSTR pszwInfId,
            /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppnccItem);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumComponents )( 
            INetCfgClass __RPC_FAR * This,
            /* [out] */ IEnumNetCfgComponent __RPC_FAR *__RPC_FAR *ppenumComponent);
        
        END_INTERFACE
    } INetCfgClassVtbl;

    interface INetCfgClass
    {
        CONST_VTBL struct INetCfgClassVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetCfgClass_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetCfgClass_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetCfgClass_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetCfgClass_FindComponent(This,pszwInfId,ppnccItem)	\
    (This)->lpVtbl -> FindComponent(This,pszwInfId,ppnccItem)

#define INetCfgClass_EnumComponents(This,ppenumComponent)	\
    (This)->lpVtbl -> EnumComponents(This,ppenumComponent)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE INetCfgClass_FindComponent_Proxy( 
    INetCfgClass __RPC_FAR * This,
    /* [string][in] */ LPCWSTR pszwInfId,
    /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppnccItem);


void __RPC_STUB INetCfgClass_FindComponent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgClass_EnumComponents_Proxy( 
    INetCfgClass __RPC_FAR * This,
    /* [out] */ IEnumNetCfgComponent __RPC_FAR *__RPC_FAR *ppenumComponent);


void __RPC_STUB INetCfgClass_EnumComponents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __INetCfgClass_INTERFACE_DEFINED__ */


#ifndef __INetCfgClassSetup_INTERFACE_DEFINED__
#define __INetCfgClassSetup_INTERFACE_DEFINED__

/* interface INetCfgClassSetup */
/* [unique][uuid][object][local] */ 

typedef 
enum tagOBO_TOKEN_TYPE
    {	OBO_USER	= 1,
	OBO_COMPONENT	= 2,
	OBO_SOFTWARE	= 3
    }	OBO_TOKEN_TYPE;

typedef struct tagOBO_TOKEN
    {
    OBO_TOKEN_TYPE Type;
    INetCfgComponent __RPC_FAR *pncc;
    LPCWSTR pszwManufacturer;
    LPCWSTR pszwProduct;
    LPCWSTR pszwDisplayName;
    BOOL fRegistered;
    }	OBO_TOKEN;


EXTERN_C const IID IID_INetCfgClassSetup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C0E8AE9D-306E-11D1-AACF-00805FC1270E")
    INetCfgClassSetup : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SelectAndInstall( 
            /* [in] */ HWND hwndParent,
            /* [in] */ OBO_TOKEN __RPC_FAR *pOboToken,
            /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppnccItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Install( 
            /* [string][in] */ LPCWSTR pszwInfId,
            /* [in] */ OBO_TOKEN __RPC_FAR *pOboToken,
            /* [in] */ DWORD dwSetupFlags,
            /* [in] */ DWORD dwUpgradeFromBuildNo,
            /* [string][in] */ LPCWSTR pszwAnswerFile,
            /* [string][in] */ LPCWSTR pszwAnswerSections,
            /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppnccItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeInstall( 
            /* [in] */ INetCfgComponent __RPC_FAR *pComponent,
            /* [in] */ OBO_TOKEN __RPC_FAR *pOboToken,
            /* [out] */ LPWSTR __RPC_FAR *pmszwRefs) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct INetCfgClassSetupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INetCfgClassSetup __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INetCfgClassSetup __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INetCfgClassSetup __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SelectAndInstall )( 
            INetCfgClassSetup __RPC_FAR * This,
            /* [in] */ HWND hwndParent,
            /* [in] */ OBO_TOKEN __RPC_FAR *pOboToken,
            /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppnccItem);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Install )( 
            INetCfgClassSetup __RPC_FAR * This,
            /* [string][in] */ LPCWSTR pszwInfId,
            /* [in] */ OBO_TOKEN __RPC_FAR *pOboToken,
            /* [in] */ DWORD dwSetupFlags,
            /* [in] */ DWORD dwUpgradeFromBuildNo,
            /* [string][in] */ LPCWSTR pszwAnswerFile,
            /* [string][in] */ LPCWSTR pszwAnswerSections,
            /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppnccItem);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeInstall )( 
            INetCfgClassSetup __RPC_FAR * This,
            /* [in] */ INetCfgComponent __RPC_FAR *pComponent,
            /* [in] */ OBO_TOKEN __RPC_FAR *pOboToken,
            /* [out] */ LPWSTR __RPC_FAR *pmszwRefs);
        
        END_INTERFACE
    } INetCfgClassSetupVtbl;

    interface INetCfgClassSetup
    {
        CONST_VTBL struct INetCfgClassSetupVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetCfgClassSetup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetCfgClassSetup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetCfgClassSetup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetCfgClassSetup_SelectAndInstall(This,hwndParent,pOboToken,ppnccItem)	\
    (This)->lpVtbl -> SelectAndInstall(This,hwndParent,pOboToken,ppnccItem)

#define INetCfgClassSetup_Install(This,pszwInfId,pOboToken,dwSetupFlags,dwUpgradeFromBuildNo,pszwAnswerFile,pszwAnswerSections,ppnccItem)	\
    (This)->lpVtbl -> Install(This,pszwInfId,pOboToken,dwSetupFlags,dwUpgradeFromBuildNo,pszwAnswerFile,pszwAnswerSections,ppnccItem)

#define INetCfgClassSetup_DeInstall(This,pComponent,pOboToken,pmszwRefs)	\
    (This)->lpVtbl -> DeInstall(This,pComponent,pOboToken,pmszwRefs)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE INetCfgClassSetup_SelectAndInstall_Proxy( 
    INetCfgClassSetup __RPC_FAR * This,
    /* [in] */ HWND hwndParent,
    /* [in] */ OBO_TOKEN __RPC_FAR *pOboToken,
    /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppnccItem);


void __RPC_STUB INetCfgClassSetup_SelectAndInstall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgClassSetup_Install_Proxy( 
    INetCfgClassSetup __RPC_FAR * This,
    /* [string][in] */ LPCWSTR pszwInfId,
    /* [in] */ OBO_TOKEN __RPC_FAR *pOboToken,
    /* [in] */ DWORD dwSetupFlags,
    /* [in] */ DWORD dwUpgradeFromBuildNo,
    /* [string][in] */ LPCWSTR pszwAnswerFile,
    /* [string][in] */ LPCWSTR pszwAnswerSections,
    /* [out] */ INetCfgComponent __RPC_FAR *__RPC_FAR *ppnccItem);


void __RPC_STUB INetCfgClassSetup_Install_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgClassSetup_DeInstall_Proxy( 
    INetCfgClassSetup __RPC_FAR * This,
    /* [in] */ INetCfgComponent __RPC_FAR *pComponent,
    /* [in] */ OBO_TOKEN __RPC_FAR *pOboToken,
    /* [out] */ LPWSTR __RPC_FAR *pmszwRefs);


void __RPC_STUB INetCfgClassSetup_DeInstall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __INetCfgClassSetup_INTERFACE_DEFINED__ */


#ifndef __INetCfgComponent_INTERFACE_DEFINED__
#define __INetCfgComponent_INTERFACE_DEFINED__

/* interface INetCfgComponent */
/* [unique][uuid][object][local] */ 

typedef 
enum tagCOMPONENT_CHARACTERISTICS
    {	NCF_VIRTUAL	= 0x1,
	NCF_SOFTWARE_ENUMERATED	= 0x2,
	NCF_PHYSICAL	= 0x4,
	NCF_HIDDEN	= 0x8,
	NCF_NO_SERVICE	= 0x10,
	NCF_NOT_USER_REMOVABLE	= 0x20,
	NCF_MULTIPORT_INSTANCED_ADAPTER	= 0x40,
	NCF_HAS_UI	= 0x80,
	NCF_FILTER	= 0x400,
	NCF_DONTEXPOSELOWER	= 0x1000,
	NCF_HIDE_BINDING	= 0x2000,
	NCF_FIXED_BINDING	= 0x20000
    }	COMPONENT_CHARACTERISTICS;

typedef 
enum tagNCRP_FLAGS
    {	NCRP_QUERY_PROPERTY_UI	= 0x1,
	NCRP_SHOW_PROPERTY_UI	= 0x2
    }	NCRP_FLAGS;


EXTERN_C const IID IID_INetCfgComponent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C0E8AE99-306E-11D1-AACF-00805FC1270E")
    INetCfgComponent : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDisplayName( 
            /* [string][out] */ LPWSTR __RPC_FAR *ppszwDisplayName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDisplayName( 
            /* [string][in] */ LPCWSTR pszwDisplayName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHelpText( 
            /* [string][out] */ LPWSTR __RPC_FAR *pszwHelpText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetId( 
            /* [string][out] */ LPWSTR __RPC_FAR *ppszwId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCharacteristics( 
            /* [out] */ LPDWORD pdwCharacteristics) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInstanceGuid( 
            /* [out] */ GUID __RPC_FAR *pGuid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPnpDevNodeId( 
            /* [string][out] */ LPWSTR __RPC_FAR *ppszwDevNodeId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetClassGuid( 
            /* [out] */ GUID __RPC_FAR *pGuid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBindName( 
            /* [string][out] */ LPWSTR __RPC_FAR *ppszwBindName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDeviceStatus( 
            /* [out] */ ULONG __RPC_FAR *pulStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenParamKey( 
            /* [out] */ HKEY __RPC_FAR *phkey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RaisePropertyUi( 
            /* [in] */ HWND hwndParent,
            /* [in] */ DWORD dwFlags,
            /* [in] */ IUnknown __RPC_FAR *punkContext) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct INetCfgComponentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INetCfgComponent __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INetCfgComponent __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INetCfgComponent __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDisplayName )( 
            INetCfgComponent __RPC_FAR * This,
            /* [string][out] */ LPWSTR __RPC_FAR *ppszwDisplayName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDisplayName )( 
            INetCfgComponent __RPC_FAR * This,
            /* [string][in] */ LPCWSTR pszwDisplayName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetHelpText )( 
            INetCfgComponent __RPC_FAR * This,
            /* [string][out] */ LPWSTR __RPC_FAR *pszwHelpText);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetId )( 
            INetCfgComponent __RPC_FAR * This,
            /* [string][out] */ LPWSTR __RPC_FAR *ppszwId);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCharacteristics )( 
            INetCfgComponent __RPC_FAR * This,
            /* [out] */ LPDWORD pdwCharacteristics);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInstanceGuid )( 
            INetCfgComponent __RPC_FAR * This,
            /* [out] */ GUID __RPC_FAR *pGuid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPnpDevNodeId )( 
            INetCfgComponent __RPC_FAR * This,
            /* [string][out] */ LPWSTR __RPC_FAR *ppszwDevNodeId);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClassGuid )( 
            INetCfgComponent __RPC_FAR * This,
            /* [out] */ GUID __RPC_FAR *pGuid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBindName )( 
            INetCfgComponent __RPC_FAR * This,
            /* [string][out] */ LPWSTR __RPC_FAR *ppszwBindName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDeviceStatus )( 
            INetCfgComponent __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pulStatus);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenParamKey )( 
            INetCfgComponent __RPC_FAR * This,
            /* [out] */ HKEY __RPC_FAR *phkey);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RaisePropertyUi )( 
            INetCfgComponent __RPC_FAR * This,
            /* [in] */ HWND hwndParent,
            /* [in] */ DWORD dwFlags,
            /* [in] */ IUnknown __RPC_FAR *punkContext);
        
        END_INTERFACE
    } INetCfgComponentVtbl;

    interface INetCfgComponent
    {
        CONST_VTBL struct INetCfgComponentVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetCfgComponent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetCfgComponent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetCfgComponent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetCfgComponent_GetDisplayName(This,ppszwDisplayName)	\
    (This)->lpVtbl -> GetDisplayName(This,ppszwDisplayName)

#define INetCfgComponent_SetDisplayName(This,pszwDisplayName)	\
    (This)->lpVtbl -> SetDisplayName(This,pszwDisplayName)

#define INetCfgComponent_GetHelpText(This,pszwHelpText)	\
    (This)->lpVtbl -> GetHelpText(This,pszwHelpText)

#define INetCfgComponent_GetId(This,ppszwId)	\
    (This)->lpVtbl -> GetId(This,ppszwId)

#define INetCfgComponent_GetCharacteristics(This,pdwCharacteristics)	\
    (This)->lpVtbl -> GetCharacteristics(This,pdwCharacteristics)

#define INetCfgComponent_GetInstanceGuid(This,pGuid)	\
    (This)->lpVtbl -> GetInstanceGuid(This,pGuid)

#define INetCfgComponent_GetPnpDevNodeId(This,ppszwDevNodeId)	\
    (This)->lpVtbl -> GetPnpDevNodeId(This,ppszwDevNodeId)

#define INetCfgComponent_GetClassGuid(This,pGuid)	\
    (This)->lpVtbl -> GetClassGuid(This,pGuid)

#define INetCfgComponent_GetBindName(This,ppszwBindName)	\
    (This)->lpVtbl -> GetBindName(This,ppszwBindName)

#define INetCfgComponent_GetDeviceStatus(This,pulStatus)	\
    (This)->lpVtbl -> GetDeviceStatus(This,pulStatus)

#define INetCfgComponent_OpenParamKey(This,phkey)	\
    (This)->lpVtbl -> OpenParamKey(This,phkey)

#define INetCfgComponent_RaisePropertyUi(This,hwndParent,dwFlags,punkContext)	\
    (This)->lpVtbl -> RaisePropertyUi(This,hwndParent,dwFlags,punkContext)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE INetCfgComponent_GetDisplayName_Proxy( 
    INetCfgComponent __RPC_FAR * This,
    /* [string][out] */ LPWSTR __RPC_FAR *ppszwDisplayName);


void __RPC_STUB INetCfgComponent_GetDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponent_SetDisplayName_Proxy( 
    INetCfgComponent __RPC_FAR * This,
    /* [string][in] */ LPCWSTR pszwDisplayName);


void __RPC_STUB INetCfgComponent_SetDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponent_GetHelpText_Proxy( 
    INetCfgComponent __RPC_FAR * This,
    /* [string][out] */ LPWSTR __RPC_FAR *pszwHelpText);


void __RPC_STUB INetCfgComponent_GetHelpText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponent_GetId_Proxy( 
    INetCfgComponent __RPC_FAR * This,
    /* [string][out] */ LPWSTR __RPC_FAR *ppszwId);


void __RPC_STUB INetCfgComponent_GetId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponent_GetCharacteristics_Proxy( 
    INetCfgComponent __RPC_FAR * This,
    /* [out] */ LPDWORD pdwCharacteristics);


void __RPC_STUB INetCfgComponent_GetCharacteristics_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponent_GetInstanceGuid_Proxy( 
    INetCfgComponent __RPC_FAR * This,
    /* [out] */ GUID __RPC_FAR *pGuid);


void __RPC_STUB INetCfgComponent_GetInstanceGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponent_GetPnpDevNodeId_Proxy( 
    INetCfgComponent __RPC_FAR * This,
    /* [string][out] */ LPWSTR __RPC_FAR *ppszwDevNodeId);


void __RPC_STUB INetCfgComponent_GetPnpDevNodeId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponent_GetClassGuid_Proxy( 
    INetCfgComponent __RPC_FAR * This,
    /* [out] */ GUID __RPC_FAR *pGuid);


void __RPC_STUB INetCfgComponent_GetClassGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponent_GetBindName_Proxy( 
    INetCfgComponent __RPC_FAR * This,
    /* [string][out] */ LPWSTR __RPC_FAR *ppszwBindName);


void __RPC_STUB INetCfgComponent_GetBindName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponent_GetDeviceStatus_Proxy( 
    INetCfgComponent __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pulStatus);


void __RPC_STUB INetCfgComponent_GetDeviceStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponent_OpenParamKey_Proxy( 
    INetCfgComponent __RPC_FAR * This,
    /* [out] */ HKEY __RPC_FAR *phkey);


void __RPC_STUB INetCfgComponent_OpenParamKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponent_RaisePropertyUi_Proxy( 
    INetCfgComponent __RPC_FAR * This,
    /* [in] */ HWND hwndParent,
    /* [in] */ DWORD dwFlags,
    /* [in] */ IUnknown __RPC_FAR *punkContext);


void __RPC_STUB INetCfgComponent_RaisePropertyUi_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __INetCfgComponent_INTERFACE_DEFINED__ */


#ifndef __INetCfgComponentBindings_INTERFACE_DEFINED__
#define __INetCfgComponentBindings_INTERFACE_DEFINED__

/* interface INetCfgComponentBindings */
/* [unique][uuid][object][local] */ 

typedef 
enum tagSUPPORTS_BINDING_INTERFACE_FLAGS
    {	NCF_LOWER	= 0x1,
	NCF_UPPER	= 0x2
    }	SUPPORTS_BINDING_INTERFACE_FLAGS;

typedef 
enum tagENUM_BINDING_PATHS_FLAGS
    {	EBP_ABOVE	= 0x1,
	EBP_BELOW	= 0x2
    }	ENUM_BINDING_PATHS_FLAGS;


EXTERN_C const IID IID_INetCfgComponentBindings;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C0E8AE9E-306E-11D1-AACF-00805FC1270E")
    INetCfgComponentBindings : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE BindTo( 
            /* [in] */ INetCfgComponent __RPC_FAR *pnccItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnbindFrom( 
            /* [in] */ INetCfgComponent __RPC_FAR *pnccItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SupportsBindingInterface( 
            /* [in] */ DWORD dwFlags,
            /* [in] */ LPCWSTR pszwInterfaceName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsBoundTo( 
            /* [in] */ INetCfgComponent __RPC_FAR *pnccItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsBindableTo( 
            /* [in] */ INetCfgComponent __RPC_FAR *pnccItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumBindingPaths( 
            /* [in] */ DWORD dwFlags,
            /* [out] */ IEnumNetCfgBindingPath __RPC_FAR *__RPC_FAR *ppIEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MoveBefore( 
            /* [in] */ INetCfgBindingPath __RPC_FAR *pncbItemSrc,
            /* [in] */ INetCfgBindingPath __RPC_FAR *pncbItemDest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MoveAfter( 
            /* [in] */ INetCfgBindingPath __RPC_FAR *pncbItemSrc,
            /* [in] */ INetCfgBindingPath __RPC_FAR *pncbItemDest) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct INetCfgComponentBindingsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INetCfgComponentBindings __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INetCfgComponentBindings __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INetCfgComponentBindings __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BindTo )( 
            INetCfgComponentBindings __RPC_FAR * This,
            /* [in] */ INetCfgComponent __RPC_FAR *pnccItem);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UnbindFrom )( 
            INetCfgComponentBindings __RPC_FAR * This,
            /* [in] */ INetCfgComponent __RPC_FAR *pnccItem);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SupportsBindingInterface )( 
            INetCfgComponentBindings __RPC_FAR * This,
            /* [in] */ DWORD dwFlags,
            /* [in] */ LPCWSTR pszwInterfaceName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsBoundTo )( 
            INetCfgComponentBindings __RPC_FAR * This,
            /* [in] */ INetCfgComponent __RPC_FAR *pnccItem);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsBindableTo )( 
            INetCfgComponentBindings __RPC_FAR * This,
            /* [in] */ INetCfgComponent __RPC_FAR *pnccItem);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumBindingPaths )( 
            INetCfgComponentBindings __RPC_FAR * This,
            /* [in] */ DWORD dwFlags,
            /* [out] */ IEnumNetCfgBindingPath __RPC_FAR *__RPC_FAR *ppIEnum);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveBefore )( 
            INetCfgComponentBindings __RPC_FAR * This,
            /* [in] */ INetCfgBindingPath __RPC_FAR *pncbItemSrc,
            /* [in] */ INetCfgBindingPath __RPC_FAR *pncbItemDest);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveAfter )( 
            INetCfgComponentBindings __RPC_FAR * This,
            /* [in] */ INetCfgBindingPath __RPC_FAR *pncbItemSrc,
            /* [in] */ INetCfgBindingPath __RPC_FAR *pncbItemDest);
        
        END_INTERFACE
    } INetCfgComponentBindingsVtbl;

    interface INetCfgComponentBindings
    {
        CONST_VTBL struct INetCfgComponentBindingsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetCfgComponentBindings_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetCfgComponentBindings_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetCfgComponentBindings_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetCfgComponentBindings_BindTo(This,pnccItem)	\
    (This)->lpVtbl -> BindTo(This,pnccItem)

#define INetCfgComponentBindings_UnbindFrom(This,pnccItem)	\
    (This)->lpVtbl -> UnbindFrom(This,pnccItem)

#define INetCfgComponentBindings_SupportsBindingInterface(This,dwFlags,pszwInterfaceName)	\
    (This)->lpVtbl -> SupportsBindingInterface(This,dwFlags,pszwInterfaceName)

#define INetCfgComponentBindings_IsBoundTo(This,pnccItem)	\
    (This)->lpVtbl -> IsBoundTo(This,pnccItem)

#define INetCfgComponentBindings_IsBindableTo(This,pnccItem)	\
    (This)->lpVtbl -> IsBindableTo(This,pnccItem)

#define INetCfgComponentBindings_EnumBindingPaths(This,dwFlags,ppIEnum)	\
    (This)->lpVtbl -> EnumBindingPaths(This,dwFlags,ppIEnum)

#define INetCfgComponentBindings_MoveBefore(This,pncbItemSrc,pncbItemDest)	\
    (This)->lpVtbl -> MoveBefore(This,pncbItemSrc,pncbItemDest)

#define INetCfgComponentBindings_MoveAfter(This,pncbItemSrc,pncbItemDest)	\
    (This)->lpVtbl -> MoveAfter(This,pncbItemSrc,pncbItemDest)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE INetCfgComponentBindings_BindTo_Proxy( 
    INetCfgComponentBindings __RPC_FAR * This,
    /* [in] */ INetCfgComponent __RPC_FAR *pnccItem);


void __RPC_STUB INetCfgComponentBindings_BindTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentBindings_UnbindFrom_Proxy( 
    INetCfgComponentBindings __RPC_FAR * This,
    /* [in] */ INetCfgComponent __RPC_FAR *pnccItem);


void __RPC_STUB INetCfgComponentBindings_UnbindFrom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentBindings_SupportsBindingInterface_Proxy( 
    INetCfgComponentBindings __RPC_FAR * This,
    /* [in] */ DWORD dwFlags,
    /* [in] */ LPCWSTR pszwInterfaceName);


void __RPC_STUB INetCfgComponentBindings_SupportsBindingInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentBindings_IsBoundTo_Proxy( 
    INetCfgComponentBindings __RPC_FAR * This,
    /* [in] */ INetCfgComponent __RPC_FAR *pnccItem);


void __RPC_STUB INetCfgComponentBindings_IsBoundTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentBindings_IsBindableTo_Proxy( 
    INetCfgComponentBindings __RPC_FAR * This,
    /* [in] */ INetCfgComponent __RPC_FAR *pnccItem);


void __RPC_STUB INetCfgComponentBindings_IsBindableTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentBindings_EnumBindingPaths_Proxy( 
    INetCfgComponentBindings __RPC_FAR * This,
    /* [in] */ DWORD dwFlags,
    /* [out] */ IEnumNetCfgBindingPath __RPC_FAR *__RPC_FAR *ppIEnum);


void __RPC_STUB INetCfgComponentBindings_EnumBindingPaths_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentBindings_MoveBefore_Proxy( 
    INetCfgComponentBindings __RPC_FAR * This,
    /* [in] */ INetCfgBindingPath __RPC_FAR *pncbItemSrc,
    /* [in] */ INetCfgBindingPath __RPC_FAR *pncbItemDest);


void __RPC_STUB INetCfgComponentBindings_MoveBefore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentBindings_MoveAfter_Proxy( 
    INetCfgComponentBindings __RPC_FAR * This,
    /* [in] */ INetCfgBindingPath __RPC_FAR *pncbItemSrc,
    /* [in] */ INetCfgBindingPath __RPC_FAR *pncbItemDest);


void __RPC_STUB INetCfgComponentBindings_MoveAfter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __INetCfgComponentBindings_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



