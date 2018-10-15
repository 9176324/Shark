
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 5.03.0279 */
/* at Wed Mar 12 13:25:06 2003
 */
/* Compiler settings for netcfgn.idl:
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

#ifndef __netcfgn_h__
#define __netcfgn_h__

/* Forward Declarations */ 

#ifndef __INetCfgPnpReconfigCallback_FWD_DEFINED__
#define __INetCfgPnpReconfigCallback_FWD_DEFINED__
typedef interface INetCfgPnpReconfigCallback INetCfgPnpReconfigCallback;
#endif 	/* __INetCfgPnpReconfigCallback_FWD_DEFINED__ */


#ifndef __INetCfgComponentControl_FWD_DEFINED__
#define __INetCfgComponentControl_FWD_DEFINED__
typedef interface INetCfgComponentControl INetCfgComponentControl;
#endif 	/* __INetCfgComponentControl_FWD_DEFINED__ */


#ifndef __INetCfgComponentSetup_FWD_DEFINED__
#define __INetCfgComponentSetup_FWD_DEFINED__
typedef interface INetCfgComponentSetup INetCfgComponentSetup;
#endif 	/* __INetCfgComponentSetup_FWD_DEFINED__ */


#ifndef __INetCfgComponentPropertyUi_FWD_DEFINED__
#define __INetCfgComponentPropertyUi_FWD_DEFINED__
typedef interface INetCfgComponentPropertyUi INetCfgComponentPropertyUi;
#endif 	/* __INetCfgComponentPropertyUi_FWD_DEFINED__ */


#ifndef __INetCfgComponentNotifyBinding_FWD_DEFINED__
#define __INetCfgComponentNotifyBinding_FWD_DEFINED__
typedef interface INetCfgComponentNotifyBinding INetCfgComponentNotifyBinding;
#endif 	/* __INetCfgComponentNotifyBinding_FWD_DEFINED__ */


#ifndef __INetCfgComponentNotifyGlobal_FWD_DEFINED__
#define __INetCfgComponentNotifyGlobal_FWD_DEFINED__
typedef interface INetCfgComponentNotifyGlobal INetCfgComponentNotifyGlobal;
#endif 	/* __INetCfgComponentNotifyGlobal_FWD_DEFINED__ */


#ifndef __INetCfgComponentUpperEdge_FWD_DEFINED__
#define __INetCfgComponentUpperEdge_FWD_DEFINED__
typedef interface INetCfgComponentUpperEdge INetCfgComponentUpperEdge;
#endif 	/* __INetCfgComponentUpperEdge_FWD_DEFINED__ */


#ifndef __INetLanConnectionUiInfo_FWD_DEFINED__
#define __INetLanConnectionUiInfo_FWD_DEFINED__
typedef interface INetLanConnectionUiInfo INetLanConnectionUiInfo;
#endif 	/* __INetLanConnectionUiInfo_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"
#include "netcfgx.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_netcfgn_0000 */
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


extern RPC_IF_HANDLE __MIDL_itf_netcfgn_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_netcfgn_0000_v0_0_s_ifspec;

#ifndef __INetCfgPnpReconfigCallback_INTERFACE_DEFINED__
#define __INetCfgPnpReconfigCallback_INTERFACE_DEFINED__

/* interface INetCfgPnpReconfigCallback */
/* [unique][uuid][object][local] */ 

typedef /* [v1_enum] */ 
enum tagNCPNP_RECONFIG_LAYER
    {	NCRL_NDIS	= 1,
	NCRL_TDI	= 2
    }	NCPNP_RECONFIG_LAYER;


EXTERN_C const IID IID_INetCfgPnpReconfigCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8d84bd35-e227-11d2-b700-00a0c98a6a85")
    INetCfgPnpReconfigCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SendPnpReconfig( 
            /* [in] */ NCPNP_RECONFIG_LAYER Layer,
            /* [in] */ LPCWSTR pszwUpper,
            /* [in] */ LPCWSTR pszwLower,
            /* [in] */ PVOID pvData,
            /* [in] */ DWORD dwSizeOfData) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct INetCfgPnpReconfigCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INetCfgPnpReconfigCallback __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INetCfgPnpReconfigCallback __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INetCfgPnpReconfigCallback __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendPnpReconfig )( 
            INetCfgPnpReconfigCallback __RPC_FAR * This,
            /* [in] */ NCPNP_RECONFIG_LAYER Layer,
            /* [in] */ LPCWSTR pszwUpper,
            /* [in] */ LPCWSTR pszwLower,
            /* [in] */ PVOID pvData,
            /* [in] */ DWORD dwSizeOfData);
        
        END_INTERFACE
    } INetCfgPnpReconfigCallbackVtbl;

    interface INetCfgPnpReconfigCallback
    {
        CONST_VTBL struct INetCfgPnpReconfigCallbackVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetCfgPnpReconfigCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetCfgPnpReconfigCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetCfgPnpReconfigCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetCfgPnpReconfigCallback_SendPnpReconfig(This,Layer,pszwUpper,pszwLower,pvData,dwSizeOfData)	\
    (This)->lpVtbl -> SendPnpReconfig(This,Layer,pszwUpper,pszwLower,pvData,dwSizeOfData)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE INetCfgPnpReconfigCallback_SendPnpReconfig_Proxy( 
    INetCfgPnpReconfigCallback __RPC_FAR * This,
    /* [in] */ NCPNP_RECONFIG_LAYER Layer,
    /* [in] */ LPCWSTR pszwUpper,
    /* [in] */ LPCWSTR pszwLower,
    /* [in] */ PVOID pvData,
    /* [in] */ DWORD dwSizeOfData);


void __RPC_STUB INetCfgPnpReconfigCallback_SendPnpReconfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __INetCfgPnpReconfigCallback_INTERFACE_DEFINED__ */


#ifndef __INetCfgComponentControl_INTERFACE_DEFINED__
#define __INetCfgComponentControl_INTERFACE_DEFINED__

/* interface INetCfgComponentControl */
/* [unique][uuid][object][local] */ 


EXTERN_C const IID IID_INetCfgComponentControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("932238df-bea1-11d0-9298-00c04fc99dcf")
    INetCfgComponentControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
            /* [in] */ INetCfgComponent __RPC_FAR *pIComp,
            /* [in] */ INetCfg __RPC_FAR *pINetCfg,
            /* [in] */ BOOL fInstalling) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ApplyRegistryChanges( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ApplyPnpChanges( 
            /* [in] */ INetCfgPnpReconfigCallback __RPC_FAR *pICallback) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CancelChanges( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct INetCfgComponentControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INetCfgComponentControl __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INetCfgComponentControl __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INetCfgComponentControl __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Initialize )( 
            INetCfgComponentControl __RPC_FAR * This,
            /* [in] */ INetCfgComponent __RPC_FAR *pIComp,
            /* [in] */ INetCfg __RPC_FAR *pINetCfg,
            /* [in] */ BOOL fInstalling);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ApplyRegistryChanges )( 
            INetCfgComponentControl __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ApplyPnpChanges )( 
            INetCfgComponentControl __RPC_FAR * This,
            /* [in] */ INetCfgPnpReconfigCallback __RPC_FAR *pICallback);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CancelChanges )( 
            INetCfgComponentControl __RPC_FAR * This);
        
        END_INTERFACE
    } INetCfgComponentControlVtbl;

    interface INetCfgComponentControl
    {
        CONST_VTBL struct INetCfgComponentControlVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetCfgComponentControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetCfgComponentControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetCfgComponentControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetCfgComponentControl_Initialize(This,pIComp,pINetCfg,fInstalling)	\
    (This)->lpVtbl -> Initialize(This,pIComp,pINetCfg,fInstalling)

#define INetCfgComponentControl_ApplyRegistryChanges(This)	\
    (This)->lpVtbl -> ApplyRegistryChanges(This)

#define INetCfgComponentControl_ApplyPnpChanges(This,pICallback)	\
    (This)->lpVtbl -> ApplyPnpChanges(This,pICallback)

#define INetCfgComponentControl_CancelChanges(This)	\
    (This)->lpVtbl -> CancelChanges(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE INetCfgComponentControl_Initialize_Proxy( 
    INetCfgComponentControl __RPC_FAR * This,
    /* [in] */ INetCfgComponent __RPC_FAR *pIComp,
    /* [in] */ INetCfg __RPC_FAR *pINetCfg,
    /* [in] */ BOOL fInstalling);


void __RPC_STUB INetCfgComponentControl_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentControl_ApplyRegistryChanges_Proxy( 
    INetCfgComponentControl __RPC_FAR * This);


void __RPC_STUB INetCfgComponentControl_ApplyRegistryChanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentControl_ApplyPnpChanges_Proxy( 
    INetCfgComponentControl __RPC_FAR * This,
    /* [in] */ INetCfgPnpReconfigCallback __RPC_FAR *pICallback);


void __RPC_STUB INetCfgComponentControl_ApplyPnpChanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentControl_CancelChanges_Proxy( 
    INetCfgComponentControl __RPC_FAR * This);


void __RPC_STUB INetCfgComponentControl_CancelChanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __INetCfgComponentControl_INTERFACE_DEFINED__ */


#ifndef __INetCfgComponentSetup_INTERFACE_DEFINED__
#define __INetCfgComponentSetup_INTERFACE_DEFINED__

/* interface INetCfgComponentSetup */
/* [unique][uuid][object][local] */ 

typedef /* [v1_enum] */ 
enum tagNETWORK_INSTALL_TIME
    {	NSF_PRIMARYINSTALL	= 0x1,
	NSF_POSTSYSINSTALL	= 0x2
    }	NETWORK_INSTALL_TIME;

typedef /* [v1_enum] */ 
enum tagNETWORK_UPGRADE_TYPE
    {	NSF_WIN16_UPGRADE	= 0x10,
	NSF_WIN95_UPGRADE	= 0x20,
	NSF_WINNT_WKS_UPGRADE	= 0x40,
	NSF_WINNT_SVR_UPGRADE	= 0x80,
	NSF_WINNT_SBS_UPGRADE	= 0x100,
	NSF_COMPONENT_UPDATE	= 0x200
    }	NETWORK_UPGRADE_TYPE;


EXTERN_C const IID IID_INetCfgComponentSetup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("932238e3-bea1-11d0-9298-00c04fc99dcf")
    INetCfgComponentSetup : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Install( 
            /* [in] */ DWORD dwSetupFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Upgrade( 
            /* [in] */ DWORD dwSetupFlags,
            /* [in] */ DWORD dwUpgradeFomBuildNo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadAnswerFile( 
            /* [in] */ LPCWSTR pszwAnswerFile,
            /* [in] */ LPCWSTR pszwAnswerSections) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Removing( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct INetCfgComponentSetupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INetCfgComponentSetup __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INetCfgComponentSetup __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INetCfgComponentSetup __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Install )( 
            INetCfgComponentSetup __RPC_FAR * This,
            /* [in] */ DWORD dwSetupFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Upgrade )( 
            INetCfgComponentSetup __RPC_FAR * This,
            /* [in] */ DWORD dwSetupFlags,
            /* [in] */ DWORD dwUpgradeFomBuildNo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReadAnswerFile )( 
            INetCfgComponentSetup __RPC_FAR * This,
            /* [in] */ LPCWSTR pszwAnswerFile,
            /* [in] */ LPCWSTR pszwAnswerSections);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Removing )( 
            INetCfgComponentSetup __RPC_FAR * This);
        
        END_INTERFACE
    } INetCfgComponentSetupVtbl;

    interface INetCfgComponentSetup
    {
        CONST_VTBL struct INetCfgComponentSetupVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetCfgComponentSetup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetCfgComponentSetup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetCfgComponentSetup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetCfgComponentSetup_Install(This,dwSetupFlags)	\
    (This)->lpVtbl -> Install(This,dwSetupFlags)

#define INetCfgComponentSetup_Upgrade(This,dwSetupFlags,dwUpgradeFomBuildNo)	\
    (This)->lpVtbl -> Upgrade(This,dwSetupFlags,dwUpgradeFomBuildNo)

#define INetCfgComponentSetup_ReadAnswerFile(This,pszwAnswerFile,pszwAnswerSections)	\
    (This)->lpVtbl -> ReadAnswerFile(This,pszwAnswerFile,pszwAnswerSections)

#define INetCfgComponentSetup_Removing(This)	\
    (This)->lpVtbl -> Removing(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE INetCfgComponentSetup_Install_Proxy( 
    INetCfgComponentSetup __RPC_FAR * This,
    /* [in] */ DWORD dwSetupFlags);


void __RPC_STUB INetCfgComponentSetup_Install_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentSetup_Upgrade_Proxy( 
    INetCfgComponentSetup __RPC_FAR * This,
    /* [in] */ DWORD dwSetupFlags,
    /* [in] */ DWORD dwUpgradeFomBuildNo);


void __RPC_STUB INetCfgComponentSetup_Upgrade_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentSetup_ReadAnswerFile_Proxy( 
    INetCfgComponentSetup __RPC_FAR * This,
    /* [in] */ LPCWSTR pszwAnswerFile,
    /* [in] */ LPCWSTR pszwAnswerSections);


void __RPC_STUB INetCfgComponentSetup_ReadAnswerFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentSetup_Removing_Proxy( 
    INetCfgComponentSetup __RPC_FAR * This);


void __RPC_STUB INetCfgComponentSetup_Removing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __INetCfgComponentSetup_INTERFACE_DEFINED__ */


#ifndef __INetCfgComponentPropertyUi_INTERFACE_DEFINED__
#define __INetCfgComponentPropertyUi_INTERFACE_DEFINED__

/* interface INetCfgComponentPropertyUi */
/* [unique][uuid][object][local] */ 

typedef /* [v1_enum] */ 
enum tagDEFAULT_PAGES
    {	DPP_ADVANCED	= 1
    }	DEFAULT_PAGES;


EXTERN_C const IID IID_INetCfgComponentPropertyUi;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("932238e0-bea1-11d0-9298-00c04fc99dcf")
    INetCfgComponentPropertyUi : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryPropertyUi( 
            /* [in] */ IUnknown __RPC_FAR *pUnkReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetContext( 
            /* [in] */ IUnknown __RPC_FAR *pUnkReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MergePropPages( 
            /* [out][in] */ DWORD __RPC_FAR *pdwDefPages,
            /* [out] */ BYTE __RPC_FAR *__RPC_FAR *pahpspPrivate,
            /* [out] */ UINT __RPC_FAR *pcPages,
            /* [in] */ HWND hwndParent,
            /* [out] */ LPCWSTR __RPC_FAR *pszStartPage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ValidateProperties( 
            /* [in] */ HWND hwndSheet) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ApplyProperties( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CancelProperties( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct INetCfgComponentPropertyUiVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INetCfgComponentPropertyUi __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INetCfgComponentPropertyUi __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INetCfgComponentPropertyUi __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryPropertyUi )( 
            INetCfgComponentPropertyUi __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetContext )( 
            INetCfgComponentPropertyUi __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MergePropPages )( 
            INetCfgComponentPropertyUi __RPC_FAR * This,
            /* [out][in] */ DWORD __RPC_FAR *pdwDefPages,
            /* [out] */ BYTE __RPC_FAR *__RPC_FAR *pahpspPrivate,
            /* [out] */ UINT __RPC_FAR *pcPages,
            /* [in] */ HWND hwndParent,
            /* [out] */ LPCWSTR __RPC_FAR *pszStartPage);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ValidateProperties )( 
            INetCfgComponentPropertyUi __RPC_FAR * This,
            /* [in] */ HWND hwndSheet);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ApplyProperties )( 
            INetCfgComponentPropertyUi __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CancelProperties )( 
            INetCfgComponentPropertyUi __RPC_FAR * This);
        
        END_INTERFACE
    } INetCfgComponentPropertyUiVtbl;

    interface INetCfgComponentPropertyUi
    {
        CONST_VTBL struct INetCfgComponentPropertyUiVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetCfgComponentPropertyUi_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetCfgComponentPropertyUi_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetCfgComponentPropertyUi_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetCfgComponentPropertyUi_QueryPropertyUi(This,pUnkReserved)	\
    (This)->lpVtbl -> QueryPropertyUi(This,pUnkReserved)

#define INetCfgComponentPropertyUi_SetContext(This,pUnkReserved)	\
    (This)->lpVtbl -> SetContext(This,pUnkReserved)

#define INetCfgComponentPropertyUi_MergePropPages(This,pdwDefPages,pahpspPrivate,pcPages,hwndParent,pszStartPage)	\
    (This)->lpVtbl -> MergePropPages(This,pdwDefPages,pahpspPrivate,pcPages,hwndParent,pszStartPage)

#define INetCfgComponentPropertyUi_ValidateProperties(This,hwndSheet)	\
    (This)->lpVtbl -> ValidateProperties(This,hwndSheet)

#define INetCfgComponentPropertyUi_ApplyProperties(This)	\
    (This)->lpVtbl -> ApplyProperties(This)

#define INetCfgComponentPropertyUi_CancelProperties(This)	\
    (This)->lpVtbl -> CancelProperties(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE INetCfgComponentPropertyUi_QueryPropertyUi_Proxy( 
    INetCfgComponentPropertyUi __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnkReserved);


void __RPC_STUB INetCfgComponentPropertyUi_QueryPropertyUi_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentPropertyUi_SetContext_Proxy( 
    INetCfgComponentPropertyUi __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnkReserved);


void __RPC_STUB INetCfgComponentPropertyUi_SetContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentPropertyUi_MergePropPages_Proxy( 
    INetCfgComponentPropertyUi __RPC_FAR * This,
    /* [out][in] */ DWORD __RPC_FAR *pdwDefPages,
    /* [out] */ BYTE __RPC_FAR *__RPC_FAR *pahpspPrivate,
    /* [out] */ UINT __RPC_FAR *pcPages,
    /* [in] */ HWND hwndParent,
    /* [out] */ LPCWSTR __RPC_FAR *pszStartPage);


void __RPC_STUB INetCfgComponentPropertyUi_MergePropPages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentPropertyUi_ValidateProperties_Proxy( 
    INetCfgComponentPropertyUi __RPC_FAR * This,
    /* [in] */ HWND hwndSheet);


void __RPC_STUB INetCfgComponentPropertyUi_ValidateProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentPropertyUi_ApplyProperties_Proxy( 
    INetCfgComponentPropertyUi __RPC_FAR * This);


void __RPC_STUB INetCfgComponentPropertyUi_ApplyProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentPropertyUi_CancelProperties_Proxy( 
    INetCfgComponentPropertyUi __RPC_FAR * This);


void __RPC_STUB INetCfgComponentPropertyUi_CancelProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __INetCfgComponentPropertyUi_INTERFACE_DEFINED__ */


#ifndef __INetCfgComponentNotifyBinding_INTERFACE_DEFINED__
#define __INetCfgComponentNotifyBinding_INTERFACE_DEFINED__

/* interface INetCfgComponentNotifyBinding */
/* [unique][uuid][object][local] */ 

typedef /* [v1_enum] */ 
enum tagBIND_FLAGS1
    {	NCN_ADD	= 0x1,
	NCN_REMOVE	= 0x2,
	NCN_UPDATE	= 0x4,
	NCN_ENABLE	= 0x10,
	NCN_DISABLE	= 0x20,
	NCN_BINDING_PATH	= 0x100,
	NCN_PROPERTYCHANGE	= 0x200,
	NCN_NET	= 0x10000,
	NCN_NETTRANS	= 0x20000,
	NCN_NETCLIENT	= 0x40000,
	NCN_NETSERVICE	= 0x80000
    }	BIND_FLAGS1;


EXTERN_C const IID IID_INetCfgComponentNotifyBinding;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("932238e1-bea1-11d0-9298-00c04fc99dcf")
    INetCfgComponentNotifyBinding : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryBindingPath( 
            /* [in] */ DWORD dwChangeFlag,
            /* [in] */ INetCfgBindingPath __RPC_FAR *pIPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NotifyBindingPath( 
            /* [in] */ DWORD dwChangeFlag,
            /* [in] */ INetCfgBindingPath __RPC_FAR *pIPath) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct INetCfgComponentNotifyBindingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INetCfgComponentNotifyBinding __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INetCfgComponentNotifyBinding __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INetCfgComponentNotifyBinding __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryBindingPath )( 
            INetCfgComponentNotifyBinding __RPC_FAR * This,
            /* [in] */ DWORD dwChangeFlag,
            /* [in] */ INetCfgBindingPath __RPC_FAR *pIPath);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NotifyBindingPath )( 
            INetCfgComponentNotifyBinding __RPC_FAR * This,
            /* [in] */ DWORD dwChangeFlag,
            /* [in] */ INetCfgBindingPath __RPC_FAR *pIPath);
        
        END_INTERFACE
    } INetCfgComponentNotifyBindingVtbl;

    interface INetCfgComponentNotifyBinding
    {
        CONST_VTBL struct INetCfgComponentNotifyBindingVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetCfgComponentNotifyBinding_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetCfgComponentNotifyBinding_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetCfgComponentNotifyBinding_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetCfgComponentNotifyBinding_QueryBindingPath(This,dwChangeFlag,pIPath)	\
    (This)->lpVtbl -> QueryBindingPath(This,dwChangeFlag,pIPath)

#define INetCfgComponentNotifyBinding_NotifyBindingPath(This,dwChangeFlag,pIPath)	\
    (This)->lpVtbl -> NotifyBindingPath(This,dwChangeFlag,pIPath)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE INetCfgComponentNotifyBinding_QueryBindingPath_Proxy( 
    INetCfgComponentNotifyBinding __RPC_FAR * This,
    /* [in] */ DWORD dwChangeFlag,
    /* [in] */ INetCfgBindingPath __RPC_FAR *pIPath);


void __RPC_STUB INetCfgComponentNotifyBinding_QueryBindingPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentNotifyBinding_NotifyBindingPath_Proxy( 
    INetCfgComponentNotifyBinding __RPC_FAR * This,
    /* [in] */ DWORD dwChangeFlag,
    /* [in] */ INetCfgBindingPath __RPC_FAR *pIPath);


void __RPC_STUB INetCfgComponentNotifyBinding_NotifyBindingPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __INetCfgComponentNotifyBinding_INTERFACE_DEFINED__ */


#ifndef __INetCfgComponentNotifyGlobal_INTERFACE_DEFINED__
#define __INetCfgComponentNotifyGlobal_INTERFACE_DEFINED__

/* interface INetCfgComponentNotifyGlobal */
/* [unique][uuid][object][local] */ 


EXTERN_C const IID IID_INetCfgComponentNotifyGlobal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("932238e2-bea1-11d0-9298-00c04fc99dcf")
    INetCfgComponentNotifyGlobal : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSupportedNotifications( 
            /* [out] */ DWORD __RPC_FAR *dwNotifications) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SysQueryBindingPath( 
            /* [in] */ DWORD dwChangeFlag,
            /* [in] */ INetCfgBindingPath __RPC_FAR *pIPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SysNotifyBindingPath( 
            /* [in] */ DWORD dwChangeFlag,
            /* [in] */ INetCfgBindingPath __RPC_FAR *pIPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SysNotifyComponent( 
            /* [in] */ DWORD dwChangeFlag,
            /* [in] */ INetCfgComponent __RPC_FAR *pIComp) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct INetCfgComponentNotifyGlobalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INetCfgComponentNotifyGlobal __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INetCfgComponentNotifyGlobal __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INetCfgComponentNotifyGlobal __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSupportedNotifications )( 
            INetCfgComponentNotifyGlobal __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *dwNotifications);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SysQueryBindingPath )( 
            INetCfgComponentNotifyGlobal __RPC_FAR * This,
            /* [in] */ DWORD dwChangeFlag,
            /* [in] */ INetCfgBindingPath __RPC_FAR *pIPath);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SysNotifyBindingPath )( 
            INetCfgComponentNotifyGlobal __RPC_FAR * This,
            /* [in] */ DWORD dwChangeFlag,
            /* [in] */ INetCfgBindingPath __RPC_FAR *pIPath);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SysNotifyComponent )( 
            INetCfgComponentNotifyGlobal __RPC_FAR * This,
            /* [in] */ DWORD dwChangeFlag,
            /* [in] */ INetCfgComponent __RPC_FAR *pIComp);
        
        END_INTERFACE
    } INetCfgComponentNotifyGlobalVtbl;

    interface INetCfgComponentNotifyGlobal
    {
        CONST_VTBL struct INetCfgComponentNotifyGlobalVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetCfgComponentNotifyGlobal_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetCfgComponentNotifyGlobal_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetCfgComponentNotifyGlobal_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetCfgComponentNotifyGlobal_GetSupportedNotifications(This,dwNotifications)	\
    (This)->lpVtbl -> GetSupportedNotifications(This,dwNotifications)

#define INetCfgComponentNotifyGlobal_SysQueryBindingPath(This,dwChangeFlag,pIPath)	\
    (This)->lpVtbl -> SysQueryBindingPath(This,dwChangeFlag,pIPath)

#define INetCfgComponentNotifyGlobal_SysNotifyBindingPath(This,dwChangeFlag,pIPath)	\
    (This)->lpVtbl -> SysNotifyBindingPath(This,dwChangeFlag,pIPath)

#define INetCfgComponentNotifyGlobal_SysNotifyComponent(This,dwChangeFlag,pIComp)	\
    (This)->lpVtbl -> SysNotifyComponent(This,dwChangeFlag,pIComp)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE INetCfgComponentNotifyGlobal_GetSupportedNotifications_Proxy( 
    INetCfgComponentNotifyGlobal __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *dwNotifications);


void __RPC_STUB INetCfgComponentNotifyGlobal_GetSupportedNotifications_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentNotifyGlobal_SysQueryBindingPath_Proxy( 
    INetCfgComponentNotifyGlobal __RPC_FAR * This,
    /* [in] */ DWORD dwChangeFlag,
    /* [in] */ INetCfgBindingPath __RPC_FAR *pIPath);


void __RPC_STUB INetCfgComponentNotifyGlobal_SysQueryBindingPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentNotifyGlobal_SysNotifyBindingPath_Proxy( 
    INetCfgComponentNotifyGlobal __RPC_FAR * This,
    /* [in] */ DWORD dwChangeFlag,
    /* [in] */ INetCfgBindingPath __RPC_FAR *pIPath);


void __RPC_STUB INetCfgComponentNotifyGlobal_SysNotifyBindingPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentNotifyGlobal_SysNotifyComponent_Proxy( 
    INetCfgComponentNotifyGlobal __RPC_FAR * This,
    /* [in] */ DWORD dwChangeFlag,
    /* [in] */ INetCfgComponent __RPC_FAR *pIComp);


void __RPC_STUB INetCfgComponentNotifyGlobal_SysNotifyComponent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __INetCfgComponentNotifyGlobal_INTERFACE_DEFINED__ */


#ifndef __INetCfgComponentUpperEdge_INTERFACE_DEFINED__
#define __INetCfgComponentUpperEdge_INTERFACE_DEFINED__

/* interface INetCfgComponentUpperEdge */
/* [unique][uuid][object][local] */ 


EXTERN_C const IID IID_INetCfgComponentUpperEdge;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("932238e4-bea1-11d0-9298-00c04fc99dcf")
    INetCfgComponentUpperEdge : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetInterfaceIdsForAdapter( 
            /* [in] */ INetCfgComponent __RPC_FAR *pAdapter,
            /* [out] */ DWORD __RPC_FAR *pdwNumInterfaces,
            /* [out] */ GUID __RPC_FAR *__RPC_FAR *ppguidInterfaceIds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddInterfacesToAdapter( 
            /* [in] */ INetCfgComponent __RPC_FAR *pAdapter,
            /* [in] */ DWORD dwNumInterfaces) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveInterfacesFromAdapter( 
            /* [in] */ INetCfgComponent __RPC_FAR *pAdapter,
            /* [in] */ DWORD dwNumInterfaces,
            /* [in] */ const GUID __RPC_FAR *pguidInterfaceIds) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct INetCfgComponentUpperEdgeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INetCfgComponentUpperEdge __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INetCfgComponentUpperEdge __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INetCfgComponentUpperEdge __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInterfaceIdsForAdapter )( 
            INetCfgComponentUpperEdge __RPC_FAR * This,
            /* [in] */ INetCfgComponent __RPC_FAR *pAdapter,
            /* [out] */ DWORD __RPC_FAR *pdwNumInterfaces,
            /* [out] */ GUID __RPC_FAR *__RPC_FAR *ppguidInterfaceIds);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddInterfacesToAdapter )( 
            INetCfgComponentUpperEdge __RPC_FAR * This,
            /* [in] */ INetCfgComponent __RPC_FAR *pAdapter,
            /* [in] */ DWORD dwNumInterfaces);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveInterfacesFromAdapter )( 
            INetCfgComponentUpperEdge __RPC_FAR * This,
            /* [in] */ INetCfgComponent __RPC_FAR *pAdapter,
            /* [in] */ DWORD dwNumInterfaces,
            /* [in] */ const GUID __RPC_FAR *pguidInterfaceIds);
        
        END_INTERFACE
    } INetCfgComponentUpperEdgeVtbl;

    interface INetCfgComponentUpperEdge
    {
        CONST_VTBL struct INetCfgComponentUpperEdgeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetCfgComponentUpperEdge_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetCfgComponentUpperEdge_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetCfgComponentUpperEdge_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetCfgComponentUpperEdge_GetInterfaceIdsForAdapter(This,pAdapter,pdwNumInterfaces,ppguidInterfaceIds)	\
    (This)->lpVtbl -> GetInterfaceIdsForAdapter(This,pAdapter,pdwNumInterfaces,ppguidInterfaceIds)

#define INetCfgComponentUpperEdge_AddInterfacesToAdapter(This,pAdapter,dwNumInterfaces)	\
    (This)->lpVtbl -> AddInterfacesToAdapter(This,pAdapter,dwNumInterfaces)

#define INetCfgComponentUpperEdge_RemoveInterfacesFromAdapter(This,pAdapter,dwNumInterfaces,pguidInterfaceIds)	\
    (This)->lpVtbl -> RemoveInterfacesFromAdapter(This,pAdapter,dwNumInterfaces,pguidInterfaceIds)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE INetCfgComponentUpperEdge_GetInterfaceIdsForAdapter_Proxy( 
    INetCfgComponentUpperEdge __RPC_FAR * This,
    /* [in] */ INetCfgComponent __RPC_FAR *pAdapter,
    /* [out] */ DWORD __RPC_FAR *pdwNumInterfaces,
    /* [out] */ GUID __RPC_FAR *__RPC_FAR *ppguidInterfaceIds);


void __RPC_STUB INetCfgComponentUpperEdge_GetInterfaceIdsForAdapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentUpperEdge_AddInterfacesToAdapter_Proxy( 
    INetCfgComponentUpperEdge __RPC_FAR * This,
    /* [in] */ INetCfgComponent __RPC_FAR *pAdapter,
    /* [in] */ DWORD dwNumInterfaces);


void __RPC_STUB INetCfgComponentUpperEdge_AddInterfacesToAdapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE INetCfgComponentUpperEdge_RemoveInterfacesFromAdapter_Proxy( 
    INetCfgComponentUpperEdge __RPC_FAR * This,
    /* [in] */ INetCfgComponent __RPC_FAR *pAdapter,
    /* [in] */ DWORD dwNumInterfaces,
    /* [in] */ const GUID __RPC_FAR *pguidInterfaceIds);


void __RPC_STUB INetCfgComponentUpperEdge_RemoveInterfacesFromAdapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __INetCfgComponentUpperEdge_INTERFACE_DEFINED__ */


#ifndef __INetLanConnectionUiInfo_INTERFACE_DEFINED__
#define __INetLanConnectionUiInfo_INTERFACE_DEFINED__

/* interface INetLanConnectionUiInfo */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_INetLanConnectionUiInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C08956A6-1CD3-11D1-B1C5-00805FC1270E")
    INetLanConnectionUiInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDeviceGuid( 
            /* [out] */ GUID __RPC_FAR *pguid) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct INetLanConnectionUiInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INetLanConnectionUiInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INetLanConnectionUiInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INetLanConnectionUiInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDeviceGuid )( 
            INetLanConnectionUiInfo __RPC_FAR * This,
            /* [out] */ GUID __RPC_FAR *pguid);
        
        END_INTERFACE
    } INetLanConnectionUiInfoVtbl;

    interface INetLanConnectionUiInfo
    {
        CONST_VTBL struct INetLanConnectionUiInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INetLanConnectionUiInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INetLanConnectionUiInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INetLanConnectionUiInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INetLanConnectionUiInfo_GetDeviceGuid(This,pguid)	\
    (This)->lpVtbl -> GetDeviceGuid(This,pguid)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE INetLanConnectionUiInfo_GetDeviceGuid_Proxy( 
    INetLanConnectionUiInfo __RPC_FAR * This,
    /* [out] */ GUID __RPC_FAR *pguid);


void __RPC_STUB INetLanConnectionUiInfo_GetDeviceGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __INetLanConnectionUiInfo_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



