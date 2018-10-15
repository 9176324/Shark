
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 5.03.0279 */
/* at Wed Apr 23 14:14:15 2003
 */
/* Compiler settings for urlmon.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32 (32b run), ms_ext, c_ext
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

#ifndef __urlmon_h__
#define __urlmon_h__

/* Forward Declarations */ 

#ifndef __IPersistMoniker_FWD_DEFINED__
#define __IPersistMoniker_FWD_DEFINED__
typedef interface IPersistMoniker IPersistMoniker;
#endif 	/* __IPersistMoniker_FWD_DEFINED__ */


#ifndef __IMonikerProp_FWD_DEFINED__
#define __IMonikerProp_FWD_DEFINED__
typedef interface IMonikerProp IMonikerProp;
#endif 	/* __IMonikerProp_FWD_DEFINED__ */


#ifndef __IBindProtocol_FWD_DEFINED__
#define __IBindProtocol_FWD_DEFINED__
typedef interface IBindProtocol IBindProtocol;
#endif 	/* __IBindProtocol_FWD_DEFINED__ */


#ifndef __IBinding_FWD_DEFINED__
#define __IBinding_FWD_DEFINED__
typedef interface IBinding IBinding;
#endif 	/* __IBinding_FWD_DEFINED__ */


#ifndef __IBindStatusCallback_FWD_DEFINED__
#define __IBindStatusCallback_FWD_DEFINED__
typedef interface IBindStatusCallback IBindStatusCallback;
#endif 	/* __IBindStatusCallback_FWD_DEFINED__ */


#ifndef __IAuthenticate_FWD_DEFINED__
#define __IAuthenticate_FWD_DEFINED__
typedef interface IAuthenticate IAuthenticate;
#endif 	/* __IAuthenticate_FWD_DEFINED__ */


#ifndef __IHttpNegotiate_FWD_DEFINED__
#define __IHttpNegotiate_FWD_DEFINED__
typedef interface IHttpNegotiate IHttpNegotiate;
#endif 	/* __IHttpNegotiate_FWD_DEFINED__ */


#ifndef __IWindowForBindingUI_FWD_DEFINED__
#define __IWindowForBindingUI_FWD_DEFINED__
typedef interface IWindowForBindingUI IWindowForBindingUI;
#endif 	/* __IWindowForBindingUI_FWD_DEFINED__ */


#ifndef __ICodeInstall_FWD_DEFINED__
#define __ICodeInstall_FWD_DEFINED__
typedef interface ICodeInstall ICodeInstall;
#endif 	/* __ICodeInstall_FWD_DEFINED__ */


#ifndef __IWinInetInfo_FWD_DEFINED__
#define __IWinInetInfo_FWD_DEFINED__
typedef interface IWinInetInfo IWinInetInfo;
#endif 	/* __IWinInetInfo_FWD_DEFINED__ */


#ifndef __IHttpSecurity_FWD_DEFINED__
#define __IHttpSecurity_FWD_DEFINED__
typedef interface IHttpSecurity IHttpSecurity;
#endif 	/* __IHttpSecurity_FWD_DEFINED__ */


#ifndef __IWinInetHttpInfo_FWD_DEFINED__
#define __IWinInetHttpInfo_FWD_DEFINED__
typedef interface IWinInetHttpInfo IWinInetHttpInfo;
#endif 	/* __IWinInetHttpInfo_FWD_DEFINED__ */


#ifndef __IBindHost_FWD_DEFINED__
#define __IBindHost_FWD_DEFINED__
typedef interface IBindHost IBindHost;
#endif 	/* __IBindHost_FWD_DEFINED__ */


#ifndef __IInternet_FWD_DEFINED__
#define __IInternet_FWD_DEFINED__
typedef interface IInternet IInternet;
#endif 	/* __IInternet_FWD_DEFINED__ */


#ifndef __IInternetBindInfo_FWD_DEFINED__
#define __IInternetBindInfo_FWD_DEFINED__
typedef interface IInternetBindInfo IInternetBindInfo;
#endif 	/* __IInternetBindInfo_FWD_DEFINED__ */


#ifndef __IInternetProtocolRoot_FWD_DEFINED__
#define __IInternetProtocolRoot_FWD_DEFINED__
typedef interface IInternetProtocolRoot IInternetProtocolRoot;
#endif 	/* __IInternetProtocolRoot_FWD_DEFINED__ */


#ifndef __IInternetProtocol_FWD_DEFINED__
#define __IInternetProtocol_FWD_DEFINED__
typedef interface IInternetProtocol IInternetProtocol;
#endif 	/* __IInternetProtocol_FWD_DEFINED__ */


#ifndef __IInternetProtocolSink_FWD_DEFINED__
#define __IInternetProtocolSink_FWD_DEFINED__
typedef interface IInternetProtocolSink IInternetProtocolSink;
#endif 	/* __IInternetProtocolSink_FWD_DEFINED__ */


#ifndef __IInternetProtocolSinkStackable_FWD_DEFINED__
#define __IInternetProtocolSinkStackable_FWD_DEFINED__
typedef interface IInternetProtocolSinkStackable IInternetProtocolSinkStackable;
#endif 	/* __IInternetProtocolSinkStackable_FWD_DEFINED__ */


#ifndef __IInternetSession_FWD_DEFINED__
#define __IInternetSession_FWD_DEFINED__
typedef interface IInternetSession IInternetSession;
#endif 	/* __IInternetSession_FWD_DEFINED__ */


#ifndef __IInternetThreadSwitch_FWD_DEFINED__
#define __IInternetThreadSwitch_FWD_DEFINED__
typedef interface IInternetThreadSwitch IInternetThreadSwitch;
#endif 	/* __IInternetThreadSwitch_FWD_DEFINED__ */


#ifndef __IInternetPriority_FWD_DEFINED__
#define __IInternetPriority_FWD_DEFINED__
typedef interface IInternetPriority IInternetPriority;
#endif 	/* __IInternetPriority_FWD_DEFINED__ */


#ifndef __IInternetProtocolInfo_FWD_DEFINED__
#define __IInternetProtocolInfo_FWD_DEFINED__
typedef interface IInternetProtocolInfo IInternetProtocolInfo;
#endif 	/* __IInternetProtocolInfo_FWD_DEFINED__ */


#ifndef __IInternetSecurityMgrSite_FWD_DEFINED__
#define __IInternetSecurityMgrSite_FWD_DEFINED__
typedef interface IInternetSecurityMgrSite IInternetSecurityMgrSite;
#endif 	/* __IInternetSecurityMgrSite_FWD_DEFINED__ */


#ifndef __IInternetSecurityManager_FWD_DEFINED__
#define __IInternetSecurityManager_FWD_DEFINED__
typedef interface IInternetSecurityManager IInternetSecurityManager;
#endif 	/* __IInternetSecurityManager_FWD_DEFINED__ */


#ifndef __IInternetHostSecurityManager_FWD_DEFINED__
#define __IInternetHostSecurityManager_FWD_DEFINED__
typedef interface IInternetHostSecurityManager IInternetHostSecurityManager;
#endif 	/* __IInternetHostSecurityManager_FWD_DEFINED__ */


#ifndef __IInternetZoneManager_FWD_DEFINED__
#define __IInternetZoneManager_FWD_DEFINED__
typedef interface IInternetZoneManager IInternetZoneManager;
#endif 	/* __IInternetZoneManager_FWD_DEFINED__ */


#ifndef __ISoftDistExt_FWD_DEFINED__
#define __ISoftDistExt_FWD_DEFINED__
typedef interface ISoftDistExt ISoftDistExt;
#endif 	/* __ISoftDistExt_FWD_DEFINED__ */


#ifndef __ICatalogFileInfo_FWD_DEFINED__
#define __ICatalogFileInfo_FWD_DEFINED__
typedef interface ICatalogFileInfo ICatalogFileInfo;
#endif 	/* __ICatalogFileInfo_FWD_DEFINED__ */


#ifndef __IDataFilter_FWD_DEFINED__
#define __IDataFilter_FWD_DEFINED__
typedef interface IDataFilter IDataFilter;
#endif 	/* __IDataFilter_FWD_DEFINED__ */


#ifndef __IEncodingFilterFactory_FWD_DEFINED__
#define __IEncodingFilterFactory_FWD_DEFINED__
typedef interface IEncodingFilterFactory IEncodingFilterFactory;
#endif 	/* __IEncodingFilterFactory_FWD_DEFINED__ */


/* header files for imported files */
#include "objidl.h"
#include "oleidl.h"
#include "servprov.h"
#include "msxml.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_urlmon_0000 */
/* [local] */ 

//=--------------------------------------------------------------------------=
// UrlMon.h
//=--------------------------------------------------------------------------=
// (C) Copyright 1995-1998 Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=

#pragma comment(lib,"uuid.lib")

//---------------------------------------------------------------------------=
// URL Moniker Interfaces.










// These are for backwards compatibility with previous URLMON versions
#define BINDF_DONTUSECACHE BINDF_GETNEWESTVERSION
#define BINDF_DONTPUTINCACHE BINDF_NOWRITECACHE
#define BINDF_NOCOPYDATA BINDF_PULLDATA
#define PI_DOCFILECLSIDLOOKUP PI_CLSIDLOOKUP
EXTERN_C const IID IID_IAsyncMoniker;    
EXTERN_C const IID CLSID_StdURLMoniker;  
EXTERN_C const IID CLSID_HttpProtocol;   
EXTERN_C const IID CLSID_FtpProtocol;    
EXTERN_C const IID CLSID_GopherProtocol; 
EXTERN_C const IID CLSID_HttpSProtocol;  
EXTERN_C const IID CLSID_FileProtocol;   
EXTERN_C const IID CLSID_MkProtocol;     
EXTERN_C const IID CLSID_StdURLProtocol; 
EXTERN_C const IID CLSID_UrlMkBindCtx;   
EXTERN_C const IID CLSID_StdEncodingFilterFac; 
EXTERN_C const IID CLSID_DeCompMimeFilter;     
EXTERN_C const IID CLSID_CdlProtocol;          
EXTERN_C const IID CLSID_ClassInstallFilter;   
EXTERN_C const IID IID_IAsyncBindCtx;    
 
#define SZ_URLCONTEXT           OLESTR("URL Context")
#define SZ_ASYNC_CALLEE         OLESTR("AsyncCallee")
#define MKSYS_URLMONIKER    6                 
 
STDAPI CreateURLMoniker(LPMONIKER pMkCtx, LPCWSTR szURL, LPMONIKER FAR * ppmk);             
STDAPI GetClassURL(LPCWSTR szURL, CLSID *pClsID);                                           
STDAPI CreateAsyncBindCtx(DWORD reserved, IBindStatusCallback *pBSCb,                       
                                IEnumFORMATETC *pEFetc, IBindCtx **ppBC);                   
STDAPI CreateAsyncBindCtxEx(IBindCtx *pbc, DWORD dwOptions, IBindStatusCallback *pBSCb, IEnumFORMATETC *pEnum,   
                            IBindCtx **ppBC, DWORD reserved);                                                     
STDAPI MkParseDisplayNameEx(IBindCtx *pbc, LPCWSTR szDisplayName, ULONG *pchEaten,          
                                LPMONIKER *ppmk);                                           
STDAPI RegisterBindStatusCallback(LPBC pBC, IBindStatusCallback *pBSCb,                     
                                IBindStatusCallback**  ppBSCBPrev, DWORD dwReserved);       
STDAPI RevokeBindStatusCallback(LPBC pBC, IBindStatusCallback *pBSCb);                      
STDAPI GetClassFileOrMime(LPBC pBC, LPCWSTR szFilename, LPVOID pBuffer, DWORD cbSize, LPCWSTR szMime, DWORD dwReserved, CLSID *pclsid); 
STDAPI IsValidURL(LPBC pBC, LPCWSTR szURL, DWORD dwReserved);                               
STDAPI CoGetClassObjectFromURL( REFCLSID rCLASSID,
            LPCWSTR szCODE, DWORD dwFileVersionMS, 
            DWORD dwFileVersionLS, LPCWSTR szTYPE,
            LPBINDCTX pBindCtx, DWORD dwClsContext,
            LPVOID pvReserved, REFIID riid, LPVOID * ppv);
STDAPI FaultInIEFeature( HWND hWnd,
            uCLSSPEC *pClassSpec,
            QUERYCONTEXT *pQuery, DWORD dwFlags);                                           
STDAPI GetComponentIDFromCLSSPEC(uCLSSPEC *pClassspec,
             LPSTR * ppszComponentID);                                                      
// flags for FaultInIEFeature
#define FIEF_FLAG_FORCE_JITUI               0x1     // force JIT ui even if
                                                 // previoulsy rejected by 
                                                 // user in this session or
                                                 // marked as Never Ask Again
#define FIEF_FLAG_PEEK                      0x2     // just peek, don't faultin
#define FIEF_FLAG_SKIP_INSTALLED_VERSION_CHECK        0x4     // force JIT without checking local version
 
//helper apis                                                                               
STDAPI IsAsyncMoniker(IMoniker* pmk);                                                       
STDAPI CreateURLBinding(LPCWSTR lpszUrl, IBindCtx *pbc, IBinding **ppBdg);                  
 
STDAPI RegisterMediaTypes(UINT ctypes, const LPCSTR* rgszTypes, CLIPFORMAT* rgcfTypes);            
STDAPI FindMediaType(LPCSTR rgszTypes, CLIPFORMAT* rgcfTypes);                                       
STDAPI CreateFormatEnumerator( UINT cfmtetc, FORMATETC* rgfmtetc, IEnumFORMATETC** ppenumfmtetc); 
STDAPI RegisterFormatEnumerator(LPBC pBC, IEnumFORMATETC *pEFetc, DWORD reserved);          
STDAPI RevokeFormatEnumerator(LPBC pBC, IEnumFORMATETC *pEFetc);                            
STDAPI RegisterMediaTypeClass(LPBC pBC,UINT ctypes, const LPCSTR* rgszTypes, CLSID *rgclsID, DWORD reserved);    
STDAPI FindMediaTypeClass(LPBC pBC, LPCSTR szType, CLSID *pclsID, DWORD reserved);                          
STDAPI UrlMkSetSessionOption(DWORD dwOption, LPVOID pBuffer, DWORD dwBufferLength, DWORD dwReserved);       
STDAPI UrlMkGetSessionOption(DWORD dwOption, LPVOID pBuffer, DWORD dwBufferLength, DWORD *pdwBufferLength, DWORD dwReserved);       
STDAPI FindMimeFromData(                                                                                                                  
                        LPBC pBC,                           // bind context - can be NULL                                                 
                        LPCWSTR pwzUrl,                     // url - can be null                                                          
                        LPVOID pBuffer,                     // buffer with data to sniff - can be null (pwzUrl must be valid)             
                        DWORD cbSize,                       // size of buffer                                                             
                        LPCWSTR pwzMimeProposed,            // proposed mime if - can be null                                             
                        DWORD dwMimeFlags,                  // will be defined                                                            
                        LPWSTR *ppwzMimeOut,                // the suggested mime                                                         
                        DWORD dwReserved);                  // must be 0                                                                  
#define     FMFD_DEFAULT        0x00000000 
#define     FMFD_URLASFILENAME  0x00000001 
STDAPI ObtainUserAgentString(DWORD dwOption, LPSTR pszUAOut, DWORD* cbSize);       
 
// URLMON-specific defines for UrlMkSetSessionOption() above
#define URLMON_OPTION_USERAGENT  0x10000001
#define URLMON_OPTION_USERAGENT_REFRESH  0x10000002
#define URLMON_OPTION_URL_ENCODING       0x10000004
 
#define CF_NULL                 0                                  
#define CFSTR_MIME_NULL         NULL                               
#define CFSTR_MIME_TEXT         (TEXT("text/plain"))             
#define CFSTR_MIME_RICHTEXT     (TEXT("text/richtext"))          
#define CFSTR_MIME_X_BITMAP     (TEXT("image/x-xbitmap"))        
#define CFSTR_MIME_POSTSCRIPT   (TEXT("application/postscript")) 
#define CFSTR_MIME_AIFF         (TEXT("audio/aiff"))             
#define CFSTR_MIME_BASICAUDIO   (TEXT("audio/basic"))            
#define CFSTR_MIME_WAV          (TEXT("audio/wav"))              
#define CFSTR_MIME_X_WAV        (TEXT("audio/x-wav"))            
#define CFSTR_MIME_GIF          (TEXT("image/gif"))              
#define CFSTR_MIME_PJPEG        (TEXT("image/pjpeg"))            
#define CFSTR_MIME_JPEG         (TEXT("image/jpeg"))             
#define CFSTR_MIME_TIFF         (TEXT("image/tiff"))             
#define CFSTR_MIME_X_PNG        (TEXT("image/x-png"))            
#define CFSTR_MIME_BMP          (TEXT("image/bmp"))              
#define CFSTR_MIME_X_ART        (TEXT("image/x-jg"))             
#define CFSTR_MIME_X_EMF        (TEXT("image/x-emf"))            
#define CFSTR_MIME_X_WMF        (TEXT("image/x-wmf"))            
#define CFSTR_MIME_AVI          (TEXT("video/avi"))              
#define CFSTR_MIME_MPEG         (TEXT("video/mpeg"))             
#define CFSTR_MIME_FRACTALS     (TEXT("application/fractals"))   
#define CFSTR_MIME_RAWDATA      (TEXT("application/octet-stream"))
#define CFSTR_MIME_RAWDATASTRM  (TEXT("application/octet-stream"))
#define CFSTR_MIME_PDF          (TEXT("application/pdf"))        
#define CFSTR_MIME_X_AIFF       (TEXT("audio/x-aiff"))           
#define CFSTR_MIME_X_REALAUDIO  (TEXT("audio/x-pn-realaudio"))   
#define CFSTR_MIME_XBM          (TEXT("image/xbm"))              
#define CFSTR_MIME_QUICKTIME    (TEXT("video/quicktime"))        
#define CFSTR_MIME_X_MSVIDEO    (TEXT("video/x-msvideo"))        
#define CFSTR_MIME_X_SGI_MOVIE  (TEXT("video/x-sgi-movie"))      
#define CFSTR_MIME_HTML         (TEXT("text/html"))              
 
// MessageId: MK_S_ASYNCHRONOUS                                              
// MessageText: Operation is successful, but will complete asynchronously.   
//                                                                           
#define MK_S_ASYNCHRONOUS    _HRESULT_TYPEDEF_(0x000401E8L)                  
#ifndef S_ASYNCHRONOUS                                                       
#define S_ASYNCHRONOUS       MK_S_ASYNCHRONOUS                               
#endif                                                                       
                                                                             
#ifndef E_PENDING                                                            
#define E_PENDING _HRESULT_TYPEDEF_(0x8000000AL)                             
#endif                                                                       
                                                                             
//                                                                           
//                                                                           
// WinINet and protocol specific errors are mapped to one of the following   
// error which are returned in IBSC::OnStopBinding                           
//                                                                           
//                                                                           
// Note: FACILITY C is split into ranges of 1k                               
// C0000 - C03FF  INET_E_ (URLMON's original hresult)                        
// C0400 - C07FF  INET_E_CLIENT_xxx                                          
// C0800 - C0BFF  INET_E_SERVER_xxx                                          
// C0C00 - C0FFF  INET_E_????                                                
// C1000 - C13FF  INET_E_AGENT_xxx (info delivery agents)                    
#define INET_E_INVALID_URL               _HRESULT_TYPEDEF_(0x800C0002L)      
#define INET_E_NO_SESSION                _HRESULT_TYPEDEF_(0x800C0003L)      
#define INET_E_CANNOT_CONNECT            _HRESULT_TYPEDEF_(0x800C0004L)      
#define INET_E_RESOURCE_NOT_FOUND        _HRESULT_TYPEDEF_(0x800C0005L)      
#define INET_E_OBJECT_NOT_FOUND          _HRESULT_TYPEDEF_(0x800C0006L)      
#define INET_E_DATA_NOT_AVAILABLE        _HRESULT_TYPEDEF_(0x800C0007L)      
#define INET_E_DOWNLOAD_FAILURE          _HRESULT_TYPEDEF_(0x800C0008L)      
#define INET_E_AUTHENTICATION_REQUIRED   _HRESULT_TYPEDEF_(0x800C0009L)      
#define INET_E_NO_VALID_MEDIA            _HRESULT_TYPEDEF_(0x800C000AL)      
#define INET_E_CONNECTION_TIMEOUT        _HRESULT_TYPEDEF_(0x800C000BL)      
#define INET_E_INVALID_REQUEST           _HRESULT_TYPEDEF_(0x800C000CL)      
#define INET_E_UNKNOWN_PROTOCOL          _HRESULT_TYPEDEF_(0x800C000DL)      
#define INET_E_SECURITY_PROBLEM          _HRESULT_TYPEDEF_(0x800C000EL)      
#define INET_E_CANNOT_LOAD_DATA          _HRESULT_TYPEDEF_(0x800C000FL)      
#define INET_E_CANNOT_INSTANTIATE_OBJECT _HRESULT_TYPEDEF_(0x800C0010L)      
#define INET_E_REDIRECT_FAILED           _HRESULT_TYPEDEF_(0x800C0014L)      
#define INET_E_REDIRECT_TO_DIR           _HRESULT_TYPEDEF_(0x800C0015L)      
#define INET_E_CANNOT_LOCK_REQUEST       _HRESULT_TYPEDEF_(0x800C0016L)      
#define INET_E_USE_EXTEND_BINDING        _HRESULT_TYPEDEF_(0x800C0017L)      
#define INET_E_ERROR_FIRST               _HRESULT_TYPEDEF_(0x800C0002L)      
#define INET_E_ERROR_LAST                INET_E_USE_EXTEND_BINDING
#define INET_E_CODE_DOWNLOAD_DECLINED    _HRESULT_TYPEDEF_(0x800C0100L)      
#define INET_E_RESULT_DISPATCHED         _HRESULT_TYPEDEF_(0x800C0200L)      
#define INET_E_CANNOT_REPLACE_SFP_FILE   _HRESULT_TYPEDEF_(0x800C0300L)      
#ifndef _LPPERSISTMONIKER_DEFINED
#define _LPPERSISTMONIKER_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0000_v0_0_s_ifspec;

#ifndef __IPersistMoniker_INTERFACE_DEFINED__
#define __IPersistMoniker_INTERFACE_DEFINED__

/* interface IPersistMoniker */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IPersistMoniker __RPC_FAR *LPPERSISTMONIKER;


EXTERN_C const IID IID_IPersistMoniker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9c9-baf9-11ce-8c82-00aa004ba90b")
    IPersistMoniker : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetClassID( 
            /* [out] */ CLSID __RPC_FAR *pClassID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsDirty( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Load( 
            /* [in] */ BOOL fFullyAvailable,
            /* [in] */ IMoniker __RPC_FAR *pimkName,
            /* [in] */ LPBC pibc,
            /* [in] */ DWORD grfMode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Save( 
            /* [in] */ IMoniker __RPC_FAR *pimkName,
            /* [in] */ LPBC pbc,
            /* [in] */ BOOL fRemember) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveCompleted( 
            /* [in] */ IMoniker __RPC_FAR *pimkName,
            /* [in] */ LPBC pibc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurMoniker( 
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppimkName) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPersistMonikerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPersistMoniker __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPersistMoniker __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPersistMoniker __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClassID )( 
            IPersistMoniker __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsDirty )( 
            IPersistMoniker __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Load )( 
            IPersistMoniker __RPC_FAR * This,
            /* [in] */ BOOL fFullyAvailable,
            /* [in] */ IMoniker __RPC_FAR *pimkName,
            /* [in] */ LPBC pibc,
            /* [in] */ DWORD grfMode);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Save )( 
            IPersistMoniker __RPC_FAR * This,
            /* [in] */ IMoniker __RPC_FAR *pimkName,
            /* [in] */ LPBC pbc,
            /* [in] */ BOOL fRemember);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SaveCompleted )( 
            IPersistMoniker __RPC_FAR * This,
            /* [in] */ IMoniker __RPC_FAR *pimkName,
            /* [in] */ LPBC pibc);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCurMoniker )( 
            IPersistMoniker __RPC_FAR * This,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppimkName);
        
        END_INTERFACE
    } IPersistMonikerVtbl;

    interface IPersistMoniker
    {
        CONST_VTBL struct IPersistMonikerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistMoniker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistMoniker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistMoniker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistMoniker_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)

#define IPersistMoniker_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IPersistMoniker_Load(This,fFullyAvailable,pimkName,pibc,grfMode)	\
    (This)->lpVtbl -> Load(This,fFullyAvailable,pimkName,pibc,grfMode)

#define IPersistMoniker_Save(This,pimkName,pbc,fRemember)	\
    (This)->lpVtbl -> Save(This,pimkName,pbc,fRemember)

#define IPersistMoniker_SaveCompleted(This,pimkName,pibc)	\
    (This)->lpVtbl -> SaveCompleted(This,pimkName,pibc)

#define IPersistMoniker_GetCurMoniker(This,ppimkName)	\
    (This)->lpVtbl -> GetCurMoniker(This,ppimkName)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPersistMoniker_GetClassID_Proxy( 
    IPersistMoniker __RPC_FAR * This,
    /* [out] */ CLSID __RPC_FAR *pClassID);


void __RPC_STUB IPersistMoniker_GetClassID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistMoniker_IsDirty_Proxy( 
    IPersistMoniker __RPC_FAR * This);


void __RPC_STUB IPersistMoniker_IsDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistMoniker_Load_Proxy( 
    IPersistMoniker __RPC_FAR * This,
    /* [in] */ BOOL fFullyAvailable,
    /* [in] */ IMoniker __RPC_FAR *pimkName,
    /* [in] */ LPBC pibc,
    /* [in] */ DWORD grfMode);


void __RPC_STUB IPersistMoniker_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistMoniker_Save_Proxy( 
    IPersistMoniker __RPC_FAR * This,
    /* [in] */ IMoniker __RPC_FAR *pimkName,
    /* [in] */ LPBC pbc,
    /* [in] */ BOOL fRemember);


void __RPC_STUB IPersistMoniker_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistMoniker_SaveCompleted_Proxy( 
    IPersistMoniker __RPC_FAR * This,
    /* [in] */ IMoniker __RPC_FAR *pimkName,
    /* [in] */ LPBC pibc);


void __RPC_STUB IPersistMoniker_SaveCompleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistMoniker_GetCurMoniker_Proxy( 
    IPersistMoniker __RPC_FAR * This,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppimkName);


void __RPC_STUB IPersistMoniker_GetCurMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPersistMoniker_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0168 */
/* [local] */ 

#endif
#ifndef _LPMONIKERPROP_DEFINED
#define _LPMONIKERPROP_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0168_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0168_v0_0_s_ifspec;

#ifndef __IMonikerProp_INTERFACE_DEFINED__
#define __IMonikerProp_INTERFACE_DEFINED__

/* interface IMonikerProp */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IMonikerProp __RPC_FAR *LPMONIKERPROP;

typedef /* [public][public] */ 
enum __MIDL_IMonikerProp_0001
    {	MIMETYPEPROP	= 0
    }	MONIKERPROPERTY;


EXTERN_C const IID IID_IMonikerProp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a5ca5f7f-1847-4d87-9c5b-918509f7511d")
    IMonikerProp : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PutProperty( 
            /* [in] */ MONIKERPROPERTY mkp,
            /* [in] */ LPCWSTR val) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMonikerPropVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMonikerProp __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMonikerProp __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMonikerProp __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PutProperty )( 
            IMonikerProp __RPC_FAR * This,
            /* [in] */ MONIKERPROPERTY mkp,
            /* [in] */ LPCWSTR val);
        
        END_INTERFACE
    } IMonikerPropVtbl;

    interface IMonikerProp
    {
        CONST_VTBL struct IMonikerPropVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMonikerProp_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMonikerProp_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMonikerProp_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMonikerProp_PutProperty(This,mkp,val)	\
    (This)->lpVtbl -> PutProperty(This,mkp,val)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IMonikerProp_PutProperty_Proxy( 
    IMonikerProp __RPC_FAR * This,
    /* [in] */ MONIKERPROPERTY mkp,
    /* [in] */ LPCWSTR val);


void __RPC_STUB IMonikerProp_PutProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMonikerProp_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0169 */
/* [local] */ 

#endif
#ifndef _LPBINDPROTOCOL_DEFINED
#define _LPBINDPROTOCOL_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0169_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0169_v0_0_s_ifspec;

#ifndef __IBindProtocol_INTERFACE_DEFINED__
#define __IBindProtocol_INTERFACE_DEFINED__

/* interface IBindProtocol */
/* [unique][uuid][object][local] */ 

typedef /* [unique] */ IBindProtocol __RPC_FAR *LPBINDPROTOCOL;


EXTERN_C const IID IID_IBindProtocol;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9cd-baf9-11ce-8c82-00aa004ba90b")
    IBindProtocol : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateBinding( 
            /* [in] */ LPCWSTR szUrl,
            /* [in] */ IBindCtx __RPC_FAR *pbc,
            /* [out] */ IBinding __RPC_FAR *__RPC_FAR *ppb) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBindProtocolVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBindProtocol __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBindProtocol __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBindProtocol __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateBinding )( 
            IBindProtocol __RPC_FAR * This,
            /* [in] */ LPCWSTR szUrl,
            /* [in] */ IBindCtx __RPC_FAR *pbc,
            /* [out] */ IBinding __RPC_FAR *__RPC_FAR *ppb);
        
        END_INTERFACE
    } IBindProtocolVtbl;

    interface IBindProtocol
    {
        CONST_VTBL struct IBindProtocolVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBindProtocol_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBindProtocol_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBindProtocol_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBindProtocol_CreateBinding(This,szUrl,pbc,ppb)	\
    (This)->lpVtbl -> CreateBinding(This,szUrl,pbc,ppb)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IBindProtocol_CreateBinding_Proxy( 
    IBindProtocol __RPC_FAR * This,
    /* [in] */ LPCWSTR szUrl,
    /* [in] */ IBindCtx __RPC_FAR *pbc,
    /* [out] */ IBinding __RPC_FAR *__RPC_FAR *ppb);


void __RPC_STUB IBindProtocol_CreateBinding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBindProtocol_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0170 */
/* [local] */ 

#endif
#ifndef _LPBINDING_DEFINED
#define _LPBINDING_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0170_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0170_v0_0_s_ifspec;

#ifndef __IBinding_INTERFACE_DEFINED__
#define __IBinding_INTERFACE_DEFINED__

/* interface IBinding */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IBinding __RPC_FAR *LPBINDING;


EXTERN_C const IID IID_IBinding;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9c0-baf9-11ce-8c82-00aa004ba90b")
    IBinding : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Abort( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Suspend( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Resume( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPriority( 
            /* [in] */ LONG nPriority) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPriority( 
            /* [out] */ LONG __RPC_FAR *pnPriority) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetBindResult( 
            /* [out] */ CLSID __RPC_FAR *pclsidProtocol,
            /* [out] */ DWORD __RPC_FAR *pdwResult,
            /* [out] */ LPOLESTR __RPC_FAR *pszResult,
            /* [out][in] */ DWORD __RPC_FAR *pdwReserved) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBindingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBinding __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBinding __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBinding __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Abort )( 
            IBinding __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Suspend )( 
            IBinding __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Resume )( 
            IBinding __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetPriority )( 
            IBinding __RPC_FAR * This,
            /* [in] */ LONG nPriority);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPriority )( 
            IBinding __RPC_FAR * This,
            /* [out] */ LONG __RPC_FAR *pnPriority);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBindResult )( 
            IBinding __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pclsidProtocol,
            /* [out] */ DWORD __RPC_FAR *pdwResult,
            /* [out] */ LPOLESTR __RPC_FAR *pszResult,
            /* [out][in] */ DWORD __RPC_FAR *pdwReserved);
        
        END_INTERFACE
    } IBindingVtbl;

    interface IBinding
    {
        CONST_VTBL struct IBindingVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBinding_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBinding_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBinding_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBinding_Abort(This)	\
    (This)->lpVtbl -> Abort(This)

#define IBinding_Suspend(This)	\
    (This)->lpVtbl -> Suspend(This)

#define IBinding_Resume(This)	\
    (This)->lpVtbl -> Resume(This)

#define IBinding_SetPriority(This,nPriority)	\
    (This)->lpVtbl -> SetPriority(This,nPriority)

#define IBinding_GetPriority(This,pnPriority)	\
    (This)->lpVtbl -> GetPriority(This,pnPriority)

#define IBinding_GetBindResult(This,pclsidProtocol,pdwResult,pszResult,pdwReserved)	\
    (This)->lpVtbl -> GetBindResult(This,pclsidProtocol,pdwResult,pszResult,pdwReserved)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IBinding_Abort_Proxy( 
    IBinding __RPC_FAR * This);


void __RPC_STUB IBinding_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBinding_Suspend_Proxy( 
    IBinding __RPC_FAR * This);


void __RPC_STUB IBinding_Suspend_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBinding_Resume_Proxy( 
    IBinding __RPC_FAR * This);


void __RPC_STUB IBinding_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBinding_SetPriority_Proxy( 
    IBinding __RPC_FAR * This,
    /* [in] */ LONG nPriority);


void __RPC_STUB IBinding_SetPriority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBinding_GetPriority_Proxy( 
    IBinding __RPC_FAR * This,
    /* [out] */ LONG __RPC_FAR *pnPriority);


void __RPC_STUB IBinding_GetPriority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IBinding_RemoteGetBindResult_Proxy( 
    IBinding __RPC_FAR * This,
    /* [out] */ CLSID __RPC_FAR *pclsidProtocol,
    /* [out] */ DWORD __RPC_FAR *pdwResult,
    /* [out] */ LPOLESTR __RPC_FAR *pszResult,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IBinding_RemoteGetBindResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBinding_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0171 */
/* [local] */ 

#endif
#ifndef _LPBINDSTATUSCALLBACK_DEFINED
#define _LPBINDSTATUSCALLBACK_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0171_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0171_v0_0_s_ifspec;

#ifndef __IBindStatusCallback_INTERFACE_DEFINED__
#define __IBindStatusCallback_INTERFACE_DEFINED__

/* interface IBindStatusCallback */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IBindStatusCallback __RPC_FAR *LPBINDSTATUSCALLBACK;

typedef /* [public] */ 
enum __MIDL_IBindStatusCallback_0001
    {	BINDVERB_GET	= 0,
	BINDVERB_POST	= 0x1,
	BINDVERB_PUT	= 0x2,
	BINDVERB_CUSTOM	= 0x3
    }	BINDVERB;

typedef /* [public] */ 
enum __MIDL_IBindStatusCallback_0002
    {	BINDINFOF_URLENCODESTGMEDDATA	= 0x1,
	BINDINFOF_URLENCODEDEXTRAINFO	= 0x2
    }	BINDINFOF;

typedef /* [public] */ 
enum __MIDL_IBindStatusCallback_0003
    {	BINDF_ASYNCHRONOUS	= 0x1,
	BINDF_ASYNCSTORAGE	= 0x2,
	BINDF_NOPROGRESSIVERENDERING	= 0x4,
	BINDF_OFFLINEOPERATION	= 0x8,
	BINDF_GETNEWESTVERSION	= 0x10,
	BINDF_NOWRITECACHE	= 0x20,
	BINDF_NEEDFILE	= 0x40,
	BINDF_PULLDATA	= 0x80,
	BINDF_IGNORESECURITYPROBLEM	= 0x100,
	BINDF_RESYNCHRONIZE	= 0x200,
	BINDF_HYPERLINK	= 0x400,
	BINDF_NO_UI	= 0x800,
	BINDF_SILENTOPERATION	= 0x1000,
	BINDF_PRAGMA_NO_CACHE	= 0x2000,
	BINDF_GETCLASSOBJECT	= 0x4000,
	BINDF_RESERVED_1	= 0x8000,
	BINDF_FREE_THREADED	= 0x10000,
	BINDF_DIRECT_READ	= 0x20000,
	BINDF_FORMS_SUBMIT	= 0x40000,
	BINDF_GETFROMCACHE_IF_NET_FAIL	= 0x80000,
	BINDF_FROMURLMON	= 0x100000,
	BINDF_FWD_BACK	= 0x200000,
	BINDF_RESERVED_2	= 0x400000,
	BINDF_RESERVED_3	= 0x800000
    }	BINDF;

typedef /* [public] */ 
enum __MIDL_IBindStatusCallback_0004
    {	URL_ENCODING_NONE	= 0,
	URL_ENCODING_ENABLE_UTF8	= 0x10000000,
	URL_ENCODING_DISABLE_UTF8	= 0x20000000
    }	URL_ENCODING;

typedef struct _tagBINDINFO
    {
    ULONG cbSize;
    LPWSTR szExtraInfo;
    STGMEDIUM stgmedData;
    DWORD grfBindInfoF;
    DWORD dwBindVerb;
    LPWSTR szCustomVerb;
    DWORD cbstgmedData;
    DWORD dwOptions;
    DWORD dwOptionsFlags;
    DWORD dwCodePage;
    SECURITY_ATTRIBUTES securityAttributes;
    IID iid;
    IUnknown __RPC_FAR *pUnk;
    DWORD dwReserved;
    }	BINDINFO;

typedef struct _REMSECURITY_ATTRIBUTES
    {
    DWORD nLength;
    DWORD lpSecurityDescriptor;
    BOOL bInheritHandle;
    }	REMSECURITY_ATTRIBUTES;

typedef struct _REMSECURITY_ATTRIBUTES __RPC_FAR *PREMSECURITY_ATTRIBUTES;

typedef struct _REMSECURITY_ATTRIBUTES __RPC_FAR *LPREMSECURITY_ATTRIBUTES;

typedef struct _tagRemBINDINFO
    {
    ULONG cbSize;
    LPWSTR szExtraInfo;
    DWORD grfBindInfoF;
    DWORD dwBindVerb;
    LPWSTR szCustomVerb;
    DWORD cbstgmedData;
    DWORD dwOptions;
    DWORD dwOptionsFlags;
    DWORD dwCodePage;
    REMSECURITY_ATTRIBUTES securityAttributes;
    IID iid;
    IUnknown __RPC_FAR *pUnk;
    DWORD dwReserved;
    }	RemBINDINFO;

typedef struct tagRemFORMATETC
    {
    DWORD cfFormat;
    DWORD ptd;
    DWORD dwAspect;
    LONG lindex;
    DWORD tymed;
    }	RemFORMATETC;

typedef struct tagRemFORMATETC __RPC_FAR *LPREMFORMATETC;

typedef /* [public] */ 
enum __MIDL_IBindStatusCallback_0005
    {	BINDINFO_OPTIONS_WININETFLAG	= 0x10000,
	BINDINFO_OPTIONS_ENABLE_UTF8	= 0x20000,
	BINDINFO_OPTIONS_DISABLE_UTF8	= 0x40000,
	BINDINFO_OPTIONS_USE_IE_ENCODING	= 0x80000,
	BINDINFO_OPTIONS_BINDTOOBJECT	= 0x100000
    }	BINDINFO_OPTIONS;

typedef /* [public] */ 
enum __MIDL_IBindStatusCallback_0006
    {	BSCF_FIRSTDATANOTIFICATION	= 0x1,
	BSCF_INTERMEDIATEDATANOTIFICATION	= 0x2,
	BSCF_LASTDATANOTIFICATION	= 0x4,
	BSCF_DATAFULLYAVAILABLE	= 0x8,
	BSCF_AVAILABLEDATASIZEUNKNOWN	= 0x10
    }	BSCF;

typedef 
enum tagBINDSTATUS
    {	BINDSTATUS_FINDINGRESOURCE	= 1,
	BINDSTATUS_CONNECTING	= BINDSTATUS_FINDINGRESOURCE + 1,
	BINDSTATUS_REDIRECTING	= BINDSTATUS_CONNECTING + 1,
	BINDSTATUS_BEGINDOWNLOADDATA	= BINDSTATUS_REDIRECTING + 1,
	BINDSTATUS_DOWNLOADINGDATA	= BINDSTATUS_BEGINDOWNLOADDATA + 1,
	BINDSTATUS_ENDDOWNLOADDATA	= BINDSTATUS_DOWNLOADINGDATA + 1,
	BINDSTATUS_BEGINDOWNLOADCOMPONENTS	= BINDSTATUS_ENDDOWNLOADDATA + 1,
	BINDSTATUS_INSTALLINGCOMPONENTS	= BINDSTATUS_BEGINDOWNLOADCOMPONENTS + 1,
	BINDSTATUS_ENDDOWNLOADCOMPONENTS	= BINDSTATUS_INSTALLINGCOMPONENTS + 1,
	BINDSTATUS_USINGCACHEDCOPY	= BINDSTATUS_ENDDOWNLOADCOMPONENTS + 1,
	BINDSTATUS_SENDINGREQUEST	= BINDSTATUS_USINGCACHEDCOPY + 1,
	BINDSTATUS_CLASSIDAVAILABLE	= BINDSTATUS_SENDINGREQUEST + 1,
	BINDSTATUS_MIMETYPEAVAILABLE	= BINDSTATUS_CLASSIDAVAILABLE + 1,
	BINDSTATUS_CACHEFILENAMEAVAILABLE	= BINDSTATUS_MIMETYPEAVAILABLE + 1,
	BINDSTATUS_BEGINSYNCOPERATION	= BINDSTATUS_CACHEFILENAMEAVAILABLE + 1,
	BINDSTATUS_ENDSYNCOPERATION	= BINDSTATUS_BEGINSYNCOPERATION + 1,
	BINDSTATUS_BEGINUPLOADDATA	= BINDSTATUS_ENDSYNCOPERATION + 1,
	BINDSTATUS_UPLOADINGDATA	= BINDSTATUS_BEGINUPLOADDATA + 1,
	BINDSTATUS_ENDUPLOADDATA	= BINDSTATUS_UPLOADINGDATA + 1,
	BINDSTATUS_PROTOCOLCLASSID	= BINDSTATUS_ENDUPLOADDATA + 1,
	BINDSTATUS_ENCODING	= BINDSTATUS_PROTOCOLCLASSID + 1,
	BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE	= BINDSTATUS_ENCODING + 1,
	BINDSTATUS_CLASSINSTALLLOCATION	= BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE + 1,
	BINDSTATUS_DECODING	= BINDSTATUS_CLASSINSTALLLOCATION + 1,
	BINDSTATUS_LOADINGMIMEHANDLER	= BINDSTATUS_DECODING + 1,
	BINDSTATUS_CONTENTDISPOSITIONATTACH	= BINDSTATUS_LOADINGMIMEHANDLER + 1,
	BINDSTATUS_FILTERREPORTMIMETYPE	= BINDSTATUS_CONTENTDISPOSITIONATTACH + 1,
	BINDSTATUS_CLSIDCANINSTANTIATE	= BINDSTATUS_FILTERREPORTMIMETYPE + 1,
	BINDSTATUS_IUNKNOWNAVAILABLE	= BINDSTATUS_CLSIDCANINSTANTIATE + 1,
	BINDSTATUS_DIRECTBIND	= BINDSTATUS_IUNKNOWNAVAILABLE + 1,
	BINDSTATUS_RAWMIMETYPE	= BINDSTATUS_DIRECTBIND + 1,
	BINDSTATUS_PROXYDETECTING	= BINDSTATUS_RAWMIMETYPE + 1,
	BINDSTATUS_ACCEPTRANGES	= BINDSTATUS_PROXYDETECTING + 1
    }	BINDSTATUS;


EXTERN_C const IID IID_IBindStatusCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9c1-baf9-11ce-8c82-00aa004ba90b")
    IBindStatusCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnStartBinding( 
            /* [in] */ DWORD dwReserved,
            /* [in] */ IBinding __RPC_FAR *pib) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPriority( 
            /* [out] */ LONG __RPC_FAR *pnPriority) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnLowResource( 
            /* [in] */ DWORD reserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnProgress( 
            /* [in] */ ULONG ulProgress,
            /* [in] */ ULONG ulProgressMax,
            /* [in] */ ULONG ulStatusCode,
            /* [in] */ LPCWSTR szStatusText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnStopBinding( 
            /* [in] */ HRESULT hresult,
            /* [unique][in] */ LPCWSTR szError) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetBindInfo( 
            /* [out] */ DWORD __RPC_FAR *grfBINDF,
            /* [unique][out][in] */ BINDINFO __RPC_FAR *pbindinfo) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE OnDataAvailable( 
            /* [in] */ DWORD grfBSCF,
            /* [in] */ DWORD dwSize,
            /* [in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [in] */ STGMEDIUM __RPC_FAR *pstgmed) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnObjectAvailable( 
            /* [in] */ REFIID riid,
            /* [iid_is][in] */ IUnknown __RPC_FAR *punk) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBindStatusCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBindStatusCallback __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBindStatusCallback __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBindStatusCallback __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnStartBinding )( 
            IBindStatusCallback __RPC_FAR * This,
            /* [in] */ DWORD dwReserved,
            /* [in] */ IBinding __RPC_FAR *pib);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPriority )( 
            IBindStatusCallback __RPC_FAR * This,
            /* [out] */ LONG __RPC_FAR *pnPriority);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnLowResource )( 
            IBindStatusCallback __RPC_FAR * This,
            /* [in] */ DWORD reserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnProgress )( 
            IBindStatusCallback __RPC_FAR * This,
            /* [in] */ ULONG ulProgress,
            /* [in] */ ULONG ulProgressMax,
            /* [in] */ ULONG ulStatusCode,
            /* [in] */ LPCWSTR szStatusText);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnStopBinding )( 
            IBindStatusCallback __RPC_FAR * This,
            /* [in] */ HRESULT hresult,
            /* [unique][in] */ LPCWSTR szError);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBindInfo )( 
            IBindStatusCallback __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *grfBINDF,
            /* [unique][out][in] */ BINDINFO __RPC_FAR *pbindinfo);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnDataAvailable )( 
            IBindStatusCallback __RPC_FAR * This,
            /* [in] */ DWORD grfBSCF,
            /* [in] */ DWORD dwSize,
            /* [in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [in] */ STGMEDIUM __RPC_FAR *pstgmed);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnObjectAvailable )( 
            IBindStatusCallback __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][in] */ IUnknown __RPC_FAR *punk);
        
        END_INTERFACE
    } IBindStatusCallbackVtbl;

    interface IBindStatusCallback
    {
        CONST_VTBL struct IBindStatusCallbackVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBindStatusCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBindStatusCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBindStatusCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBindStatusCallback_OnStartBinding(This,dwReserved,pib)	\
    (This)->lpVtbl -> OnStartBinding(This,dwReserved,pib)

#define IBindStatusCallback_GetPriority(This,pnPriority)	\
    (This)->lpVtbl -> GetPriority(This,pnPriority)

#define IBindStatusCallback_OnLowResource(This,reserved)	\
    (This)->lpVtbl -> OnLowResource(This,reserved)

#define IBindStatusCallback_OnProgress(This,ulProgress,ulProgressMax,ulStatusCode,szStatusText)	\
    (This)->lpVtbl -> OnProgress(This,ulProgress,ulProgressMax,ulStatusCode,szStatusText)

#define IBindStatusCallback_OnStopBinding(This,hresult,szError)	\
    (This)->lpVtbl -> OnStopBinding(This,hresult,szError)

#define IBindStatusCallback_GetBindInfo(This,grfBINDF,pbindinfo)	\
    (This)->lpVtbl -> GetBindInfo(This,grfBINDF,pbindinfo)

#define IBindStatusCallback_OnDataAvailable(This,grfBSCF,dwSize,pformatetc,pstgmed)	\
    (This)->lpVtbl -> OnDataAvailable(This,grfBSCF,dwSize,pformatetc,pstgmed)

#define IBindStatusCallback_OnObjectAvailable(This,riid,punk)	\
    (This)->lpVtbl -> OnObjectAvailable(This,riid,punk)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IBindStatusCallback_OnStartBinding_Proxy( 
    IBindStatusCallback __RPC_FAR * This,
    /* [in] */ DWORD dwReserved,
    /* [in] */ IBinding __RPC_FAR *pib);


void __RPC_STUB IBindStatusCallback_OnStartBinding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindStatusCallback_GetPriority_Proxy( 
    IBindStatusCallback __RPC_FAR * This,
    /* [out] */ LONG __RPC_FAR *pnPriority);


void __RPC_STUB IBindStatusCallback_GetPriority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindStatusCallback_OnLowResource_Proxy( 
    IBindStatusCallback __RPC_FAR * This,
    /* [in] */ DWORD reserved);


void __RPC_STUB IBindStatusCallback_OnLowResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindStatusCallback_OnProgress_Proxy( 
    IBindStatusCallback __RPC_FAR * This,
    /* [in] */ ULONG ulProgress,
    /* [in] */ ULONG ulProgressMax,
    /* [in] */ ULONG ulStatusCode,
    /* [in] */ LPCWSTR szStatusText);


void __RPC_STUB IBindStatusCallback_OnProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindStatusCallback_OnStopBinding_Proxy( 
    IBindStatusCallback __RPC_FAR * This,
    /* [in] */ HRESULT hresult,
    /* [unique][in] */ LPCWSTR szError);


void __RPC_STUB IBindStatusCallback_OnStopBinding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IBindStatusCallback_RemoteGetBindInfo_Proxy( 
    IBindStatusCallback __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *grfBINDF,
    /* [unique][out][in] */ RemBINDINFO __RPC_FAR *pbindinfo,
    /* [unique][out][in] */ RemSTGMEDIUM __RPC_FAR *pstgmed);


void __RPC_STUB IBindStatusCallback_RemoteGetBindInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IBindStatusCallback_RemoteOnDataAvailable_Proxy( 
    IBindStatusCallback __RPC_FAR * This,
    /* [in] */ DWORD grfBSCF,
    /* [in] */ DWORD dwSize,
    /* [in] */ RemFORMATETC __RPC_FAR *pformatetc,
    /* [in] */ RemSTGMEDIUM __RPC_FAR *pstgmed);


void __RPC_STUB IBindStatusCallback_RemoteOnDataAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindStatusCallback_OnObjectAvailable_Proxy( 
    IBindStatusCallback __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [iid_is][in] */ IUnknown __RPC_FAR *punk);


void __RPC_STUB IBindStatusCallback_OnObjectAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBindStatusCallback_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0172 */
/* [local] */ 

#endif
#ifndef _LPAUTHENTICATION_DEFINED
#define _LPAUTHENTICATION_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0172_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0172_v0_0_s_ifspec;

#ifndef __IAuthenticate_INTERFACE_DEFINED__
#define __IAuthenticate_INTERFACE_DEFINED__

/* interface IAuthenticate */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IAuthenticate __RPC_FAR *LPAUTHENTICATION;


EXTERN_C const IID IID_IAuthenticate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9d0-baf9-11ce-8c82-00aa004ba90b")
    IAuthenticate : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Authenticate( 
            /* [out] */ HWND __RPC_FAR *phwnd,
            /* [out] */ LPWSTR __RPC_FAR *pszUsername,
            /* [out] */ LPWSTR __RPC_FAR *pszPassword) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAuthenticateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAuthenticate __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAuthenticate __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAuthenticate __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Authenticate )( 
            IAuthenticate __RPC_FAR * This,
            /* [out] */ HWND __RPC_FAR *phwnd,
            /* [out] */ LPWSTR __RPC_FAR *pszUsername,
            /* [out] */ LPWSTR __RPC_FAR *pszPassword);
        
        END_INTERFACE
    } IAuthenticateVtbl;

    interface IAuthenticate
    {
        CONST_VTBL struct IAuthenticateVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAuthenticate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAuthenticate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAuthenticate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAuthenticate_Authenticate(This,phwnd,pszUsername,pszPassword)	\
    (This)->lpVtbl -> Authenticate(This,phwnd,pszUsername,pszPassword)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IAuthenticate_Authenticate_Proxy( 
    IAuthenticate __RPC_FAR * This,
    /* [out] */ HWND __RPC_FAR *phwnd,
    /* [out] */ LPWSTR __RPC_FAR *pszUsername,
    /* [out] */ LPWSTR __RPC_FAR *pszPassword);


void __RPC_STUB IAuthenticate_Authenticate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAuthenticate_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0173 */
/* [local] */ 

#endif
#ifndef _LPHTTPNEGOTIATE_DEFINED
#define _LPHTTPNEGOTIATE_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0173_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0173_v0_0_s_ifspec;

#ifndef __IHttpNegotiate_INTERFACE_DEFINED__
#define __IHttpNegotiate_INTERFACE_DEFINED__

/* interface IHttpNegotiate */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IHttpNegotiate __RPC_FAR *LPHTTPNEGOTIATE;


EXTERN_C const IID IID_IHttpNegotiate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9d2-baf9-11ce-8c82-00aa004ba90b")
    IHttpNegotiate : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE BeginningTransaction( 
            /* [in] */ LPCWSTR szURL,
            /* [unique][in] */ LPCWSTR szHeaders,
            /* [in] */ DWORD dwReserved,
            /* [out] */ LPWSTR __RPC_FAR *pszAdditionalHeaders) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnResponse( 
            /* [in] */ DWORD dwResponseCode,
            /* [unique][in] */ LPCWSTR szResponseHeaders,
            /* [unique][in] */ LPCWSTR szRequestHeaders,
            /* [out] */ LPWSTR __RPC_FAR *pszAdditionalRequestHeaders) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IHttpNegotiateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IHttpNegotiate __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IHttpNegotiate __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IHttpNegotiate __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginningTransaction )( 
            IHttpNegotiate __RPC_FAR * This,
            /* [in] */ LPCWSTR szURL,
            /* [unique][in] */ LPCWSTR szHeaders,
            /* [in] */ DWORD dwReserved,
            /* [out] */ LPWSTR __RPC_FAR *pszAdditionalHeaders);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnResponse )( 
            IHttpNegotiate __RPC_FAR * This,
            /* [in] */ DWORD dwResponseCode,
            /* [unique][in] */ LPCWSTR szResponseHeaders,
            /* [unique][in] */ LPCWSTR szRequestHeaders,
            /* [out] */ LPWSTR __RPC_FAR *pszAdditionalRequestHeaders);
        
        END_INTERFACE
    } IHttpNegotiateVtbl;

    interface IHttpNegotiate
    {
        CONST_VTBL struct IHttpNegotiateVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHttpNegotiate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHttpNegotiate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHttpNegotiate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHttpNegotiate_BeginningTransaction(This,szURL,szHeaders,dwReserved,pszAdditionalHeaders)	\
    (This)->lpVtbl -> BeginningTransaction(This,szURL,szHeaders,dwReserved,pszAdditionalHeaders)

#define IHttpNegotiate_OnResponse(This,dwResponseCode,szResponseHeaders,szRequestHeaders,pszAdditionalRequestHeaders)	\
    (This)->lpVtbl -> OnResponse(This,dwResponseCode,szResponseHeaders,szRequestHeaders,pszAdditionalRequestHeaders)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IHttpNegotiate_BeginningTransaction_Proxy( 
    IHttpNegotiate __RPC_FAR * This,
    /* [in] */ LPCWSTR szURL,
    /* [unique][in] */ LPCWSTR szHeaders,
    /* [in] */ DWORD dwReserved,
    /* [out] */ LPWSTR __RPC_FAR *pszAdditionalHeaders);


void __RPC_STUB IHttpNegotiate_BeginningTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHttpNegotiate_OnResponse_Proxy( 
    IHttpNegotiate __RPC_FAR * This,
    /* [in] */ DWORD dwResponseCode,
    /* [unique][in] */ LPCWSTR szResponseHeaders,
    /* [unique][in] */ LPCWSTR szRequestHeaders,
    /* [out] */ LPWSTR __RPC_FAR *pszAdditionalRequestHeaders);


void __RPC_STUB IHttpNegotiate_OnResponse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IHttpNegotiate_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0174 */
/* [local] */ 

#endif
#ifndef _LPWINDOWFORBINDINGUI_DEFINED
#define _LPWINDOWFORBINDINGUI_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0174_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0174_v0_0_s_ifspec;

#ifndef __IWindowForBindingUI_INTERFACE_DEFINED__
#define __IWindowForBindingUI_INTERFACE_DEFINED__

/* interface IWindowForBindingUI */
/* [unique][uuid][object][local] */ 

typedef /* [unique] */ IWindowForBindingUI __RPC_FAR *LPWINDOWFORBINDINGUI;


EXTERN_C const IID IID_IWindowForBindingUI;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9d5-bafa-11ce-8c82-00aa004ba90b")
    IWindowForBindingUI : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetWindow( 
            /* [in] */ REFGUID rguidReason,
            /* [out] */ HWND __RPC_FAR *phwnd) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWindowForBindingUIVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWindowForBindingUI __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWindowForBindingUI __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWindowForBindingUI __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetWindow )( 
            IWindowForBindingUI __RPC_FAR * This,
            /* [in] */ REFGUID rguidReason,
            /* [out] */ HWND __RPC_FAR *phwnd);
        
        END_INTERFACE
    } IWindowForBindingUIVtbl;

    interface IWindowForBindingUI
    {
        CONST_VTBL struct IWindowForBindingUIVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWindowForBindingUI_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWindowForBindingUI_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWindowForBindingUI_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWindowForBindingUI_GetWindow(This,rguidReason,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,rguidReason,phwnd)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IWindowForBindingUI_GetWindow_Proxy( 
    IWindowForBindingUI __RPC_FAR * This,
    /* [in] */ REFGUID rguidReason,
    /* [out] */ HWND __RPC_FAR *phwnd);


void __RPC_STUB IWindowForBindingUI_GetWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWindowForBindingUI_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0175 */
/* [local] */ 

#endif
#ifndef _LPCODEINSTALL_DEFINED
#define _LPCODEINSTALL_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0175_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0175_v0_0_s_ifspec;

#ifndef __ICodeInstall_INTERFACE_DEFINED__
#define __ICodeInstall_INTERFACE_DEFINED__

/* interface ICodeInstall */
/* [unique][uuid][object][local] */ 

typedef /* [unique] */ ICodeInstall __RPC_FAR *LPCODEINSTALL;

typedef /* [public] */ 
enum __MIDL_ICodeInstall_0001
    {	CIP_DISK_FULL	= 0,
	CIP_ACCESS_DENIED	= CIP_DISK_FULL + 1,
	CIP_NEWER_VERSION_EXISTS	= CIP_ACCESS_DENIED + 1,
	CIP_OLDER_VERSION_EXISTS	= CIP_NEWER_VERSION_EXISTS + 1,
	CIP_NAME_CONFLICT	= CIP_OLDER_VERSION_EXISTS + 1,
	CIP_TRUST_VERIFICATION_COMPONENT_MISSING	= CIP_NAME_CONFLICT + 1,
	CIP_EXE_SELF_REGISTERATION_TIMEOUT	= CIP_TRUST_VERIFICATION_COMPONENT_MISSING + 1,
	CIP_UNSAFE_TO_ABORT	= CIP_EXE_SELF_REGISTERATION_TIMEOUT + 1,
	CIP_NEED_REBOOT	= CIP_UNSAFE_TO_ABORT + 1
    }	CIP_STATUS;


EXTERN_C const IID IID_ICodeInstall;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9d1-baf9-11ce-8c82-00aa004ba90b")
    ICodeInstall : public IWindowForBindingUI
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnCodeInstallProblem( 
            /* [in] */ ULONG ulStatusCode,
            /* [unique][in] */ LPCWSTR szDestination,
            /* [unique][in] */ LPCWSTR szSource,
            /* [in] */ DWORD dwReserved) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICodeInstallVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICodeInstall __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICodeInstall __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICodeInstall __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetWindow )( 
            ICodeInstall __RPC_FAR * This,
            /* [in] */ REFGUID rguidReason,
            /* [out] */ HWND __RPC_FAR *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnCodeInstallProblem )( 
            ICodeInstall __RPC_FAR * This,
            /* [in] */ ULONG ulStatusCode,
            /* [unique][in] */ LPCWSTR szDestination,
            /* [unique][in] */ LPCWSTR szSource,
            /* [in] */ DWORD dwReserved);
        
        END_INTERFACE
    } ICodeInstallVtbl;

    interface ICodeInstall
    {
        CONST_VTBL struct ICodeInstallVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICodeInstall_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICodeInstall_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICodeInstall_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICodeInstall_GetWindow(This,rguidReason,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,rguidReason,phwnd)


#define ICodeInstall_OnCodeInstallProblem(This,ulStatusCode,szDestination,szSource,dwReserved)	\
    (This)->lpVtbl -> OnCodeInstallProblem(This,ulStatusCode,szDestination,szSource,dwReserved)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ICodeInstall_OnCodeInstallProblem_Proxy( 
    ICodeInstall __RPC_FAR * This,
    /* [in] */ ULONG ulStatusCode,
    /* [unique][in] */ LPCWSTR szDestination,
    /* [unique][in] */ LPCWSTR szSource,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB ICodeInstall_OnCodeInstallProblem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICodeInstall_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0176 */
/* [local] */ 

#endif
#ifndef _LPWININETINFO_DEFINED
#define _LPWININETINFO_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0176_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0176_v0_0_s_ifspec;

#ifndef __IWinInetInfo_INTERFACE_DEFINED__
#define __IWinInetInfo_INTERFACE_DEFINED__

/* interface IWinInetInfo */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IWinInetInfo __RPC_FAR *LPWININETINFO;


EXTERN_C const IID IID_IWinInetInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9d6-bafa-11ce-8c82-00aa004ba90b")
    IWinInetInfo : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE QueryOption( 
            /* [in] */ DWORD dwOption,
            /* [size_is][out][in] */ LPVOID pBuffer,
            /* [out][in] */ DWORD __RPC_FAR *pcbBuf) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWinInetInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWinInetInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWinInetInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWinInetInfo __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryOption )( 
            IWinInetInfo __RPC_FAR * This,
            /* [in] */ DWORD dwOption,
            /* [size_is][out][in] */ LPVOID pBuffer,
            /* [out][in] */ DWORD __RPC_FAR *pcbBuf);
        
        END_INTERFACE
    } IWinInetInfoVtbl;

    interface IWinInetInfo
    {
        CONST_VTBL struct IWinInetInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWinInetInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWinInetInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWinInetInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWinInetInfo_QueryOption(This,dwOption,pBuffer,pcbBuf)	\
    (This)->lpVtbl -> QueryOption(This,dwOption,pBuffer,pcbBuf)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IWinInetInfo_RemoteQueryOption_Proxy( 
    IWinInetInfo __RPC_FAR * This,
    /* [in] */ DWORD dwOption,
    /* [size_is][out][in] */ BYTE __RPC_FAR *pBuffer,
    /* [out][in] */ DWORD __RPC_FAR *pcbBuf);


void __RPC_STUB IWinInetInfo_RemoteQueryOption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWinInetInfo_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0177 */
/* [local] */ 

#endif
#define WININETINFO_OPTION_LOCK_HANDLE 65534
#ifndef _LPHTTPSECURITY_DEFINED
#define _LPHTTPSECURITY_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0177_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0177_v0_0_s_ifspec;

#ifndef __IHttpSecurity_INTERFACE_DEFINED__
#define __IHttpSecurity_INTERFACE_DEFINED__

/* interface IHttpSecurity */
/* [unique][uuid][object][local] */ 

typedef /* [unique] */ IHttpSecurity __RPC_FAR *LPHTTPSECURITY;


EXTERN_C const IID IID_IHttpSecurity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9d7-bafa-11ce-8c82-00aa004ba90b")
    IHttpSecurity : public IWindowForBindingUI
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnSecurityProblem( 
            /* [in] */ DWORD dwProblem) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IHttpSecurityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IHttpSecurity __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IHttpSecurity __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IHttpSecurity __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetWindow )( 
            IHttpSecurity __RPC_FAR * This,
            /* [in] */ REFGUID rguidReason,
            /* [out] */ HWND __RPC_FAR *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnSecurityProblem )( 
            IHttpSecurity __RPC_FAR * This,
            /* [in] */ DWORD dwProblem);
        
        END_INTERFACE
    } IHttpSecurityVtbl;

    interface IHttpSecurity
    {
        CONST_VTBL struct IHttpSecurityVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHttpSecurity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHttpSecurity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHttpSecurity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHttpSecurity_GetWindow(This,rguidReason,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,rguidReason,phwnd)


#define IHttpSecurity_OnSecurityProblem(This,dwProblem)	\
    (This)->lpVtbl -> OnSecurityProblem(This,dwProblem)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IHttpSecurity_OnSecurityProblem_Proxy( 
    IHttpSecurity __RPC_FAR * This,
    /* [in] */ DWORD dwProblem);


void __RPC_STUB IHttpSecurity_OnSecurityProblem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IHttpSecurity_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0178 */
/* [local] */ 

#endif
#ifndef _LPWININETHTTPINFO_DEFINED
#define _LPWININETHTTPINFO_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0178_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0178_v0_0_s_ifspec;

#ifndef __IWinInetHttpInfo_INTERFACE_DEFINED__
#define __IWinInetHttpInfo_INTERFACE_DEFINED__

/* interface IWinInetHttpInfo */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IWinInetHttpInfo __RPC_FAR *LPWININETHTTPINFO;


EXTERN_C const IID IID_IWinInetHttpInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9d8-bafa-11ce-8c82-00aa004ba90b")
    IWinInetHttpInfo : public IWinInetInfo
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE QueryInfo( 
            /* [in] */ DWORD dwOption,
            /* [size_is][out][in] */ LPVOID pBuffer,
            /* [out][in] */ DWORD __RPC_FAR *pcbBuf,
            /* [out][in] */ DWORD __RPC_FAR *pdwFlags,
            /* [out][in] */ DWORD __RPC_FAR *pdwReserved) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWinInetHttpInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWinInetHttpInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWinInetHttpInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWinInetHttpInfo __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryOption )( 
            IWinInetHttpInfo __RPC_FAR * This,
            /* [in] */ DWORD dwOption,
            /* [size_is][out][in] */ LPVOID pBuffer,
            /* [out][in] */ DWORD __RPC_FAR *pcbBuf);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInfo )( 
            IWinInetHttpInfo __RPC_FAR * This,
            /* [in] */ DWORD dwOption,
            /* [size_is][out][in] */ LPVOID pBuffer,
            /* [out][in] */ DWORD __RPC_FAR *pcbBuf,
            /* [out][in] */ DWORD __RPC_FAR *pdwFlags,
            /* [out][in] */ DWORD __RPC_FAR *pdwReserved);
        
        END_INTERFACE
    } IWinInetHttpInfoVtbl;

    interface IWinInetHttpInfo
    {
        CONST_VTBL struct IWinInetHttpInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWinInetHttpInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWinInetHttpInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWinInetHttpInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWinInetHttpInfo_QueryOption(This,dwOption,pBuffer,pcbBuf)	\
    (This)->lpVtbl -> QueryOption(This,dwOption,pBuffer,pcbBuf)


#define IWinInetHttpInfo_QueryInfo(This,dwOption,pBuffer,pcbBuf,pdwFlags,pdwReserved)	\
    (This)->lpVtbl -> QueryInfo(This,dwOption,pBuffer,pcbBuf,pdwFlags,pdwReserved)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IWinInetHttpInfo_RemoteQueryInfo_Proxy( 
    IWinInetHttpInfo __RPC_FAR * This,
    /* [in] */ DWORD dwOption,
    /* [size_is][out][in] */ BYTE __RPC_FAR *pBuffer,
    /* [out][in] */ DWORD __RPC_FAR *pcbBuf,
    /* [out][in] */ DWORD __RPC_FAR *pdwFlags,
    /* [out][in] */ DWORD __RPC_FAR *pdwReserved);


void __RPC_STUB IWinInetHttpInfo_RemoteQueryInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWinInetHttpInfo_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0179 */
/* [local] */ 

#endif
#define SID_IBindHost IID_IBindHost
#define SID_SBindHost IID_IBindHost
#ifndef _LPBINDHOST_DEFINED
#define _LPBINDHOST_DEFINED
EXTERN_C const GUID SID_BindHost;


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0179_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0179_v0_0_s_ifspec;

#ifndef __IBindHost_INTERFACE_DEFINED__
#define __IBindHost_INTERFACE_DEFINED__

/* interface IBindHost */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IBindHost __RPC_FAR *LPBINDHOST;


EXTERN_C const IID IID_IBindHost;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("fc4801a1-2ba9-11cf-a229-00aa003d7352")
    IBindHost : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateMoniker( 
            /* [in] */ LPOLESTR szName,
            /* [in] */ IBindCtx __RPC_FAR *pBC,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk,
            /* [in] */ DWORD dwReserved) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE MonikerBindToStorage( 
            /* [in] */ IMoniker __RPC_FAR *pMk,
            /* [in] */ IBindCtx __RPC_FAR *pBC,
            /* [in] */ IBindStatusCallback __RPC_FAR *pBSC,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE MonikerBindToObject( 
            /* [in] */ IMoniker __RPC_FAR *pMk,
            /* [in] */ IBindCtx __RPC_FAR *pBC,
            /* [in] */ IBindStatusCallback __RPC_FAR *pBSC,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBindHostVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBindHost __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBindHost __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBindHost __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateMoniker )( 
            IBindHost __RPC_FAR * This,
            /* [in] */ LPOLESTR szName,
            /* [in] */ IBindCtx __RPC_FAR *pBC,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk,
            /* [in] */ DWORD dwReserved);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MonikerBindToStorage )( 
            IBindHost __RPC_FAR * This,
            /* [in] */ IMoniker __RPC_FAR *pMk,
            /* [in] */ IBindCtx __RPC_FAR *pBC,
            /* [in] */ IBindStatusCallback __RPC_FAR *pBSC,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MonikerBindToObject )( 
            IBindHost __RPC_FAR * This,
            /* [in] */ IMoniker __RPC_FAR *pMk,
            /* [in] */ IBindCtx __RPC_FAR *pBC,
            /* [in] */ IBindStatusCallback __RPC_FAR *pBSC,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj);
        
        END_INTERFACE
    } IBindHostVtbl;

    interface IBindHost
    {
        CONST_VTBL struct IBindHostVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBindHost_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBindHost_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBindHost_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBindHost_CreateMoniker(This,szName,pBC,ppmk,dwReserved)	\
    (This)->lpVtbl -> CreateMoniker(This,szName,pBC,ppmk,dwReserved)

#define IBindHost_MonikerBindToStorage(This,pMk,pBC,pBSC,riid,ppvObj)	\
    (This)->lpVtbl -> MonikerBindToStorage(This,pMk,pBC,pBSC,riid,ppvObj)

#define IBindHost_MonikerBindToObject(This,pMk,pBC,pBSC,riid,ppvObj)	\
    (This)->lpVtbl -> MonikerBindToObject(This,pMk,pBC,pBSC,riid,ppvObj)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IBindHost_CreateMoniker_Proxy( 
    IBindHost __RPC_FAR * This,
    /* [in] */ LPOLESTR szName,
    /* [in] */ IBindCtx __RPC_FAR *pBC,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IBindHost_CreateMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IBindHost_RemoteMonikerBindToStorage_Proxy( 
    IBindHost __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pMk,
    /* [unique][in] */ IBindCtx __RPC_FAR *pBC,
    /* [unique][in] */ IBindStatusCallback __RPC_FAR *pBSC,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObj);


void __RPC_STUB IBindHost_RemoteMonikerBindToStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IBindHost_RemoteMonikerBindToObject_Proxy( 
    IBindHost __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pMk,
    /* [unique][in] */ IBindCtx __RPC_FAR *pBC,
    /* [unique][in] */ IBindStatusCallback __RPC_FAR *pBSC,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObj);


void __RPC_STUB IBindHost_RemoteMonikerBindToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBindHost_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0180 */
/* [local] */ 

#endif
                                                                                                           
// These are for backwards compatibility with previous URLMON versions
// Flags for the UrlDownloadToCacheFile                                                                    
#define URLOSTRM_USECACHEDCOPY_ONLY             0x1      // Only get from cache                            
#define URLOSTRM_USECACHEDCOPY                  0x2      // Get from cache if available else download      
#define URLOSTRM_GETNEWESTVERSION               0x3      // Get new version only. But put it in cache too  
                                                                                                           
                                                                                                           
struct IBindStatusCallback;                                                                                
STDAPI HlinkSimpleNavigateToString(                                                                        
    /* [in] */ LPCWSTR szTarget,         // required - target document - null if local jump w/in doc       
    /* [in] */ LPCWSTR szLocation,       // optional, for navigation into middle of a doc                  
    /* [in] */ LPCWSTR szTargetFrameName,// optional, for targeting frame-sets                             
    /* [in] */ IUnknown *pUnk,           // required - we'll search this for other necessary interfaces    
    /* [in] */ IBindCtx *pbc,            // optional. caller may register an IBSC in this                  
    /* [in] */ IBindStatusCallback *,                                                                      
    /* [in] */ DWORD grfHLNF,            // flags                                                          
    /* [in] */ DWORD dwReserved          // for future use, must be NULL                                   
);                                                                                                         
                                                                                                           
STDAPI HlinkSimpleNavigateToMoniker(                                                                       
    /* [in] */ IMoniker *pmkTarget,      // required - target document - (may be null                      
    /* [in] */ LPCWSTR szLocation,       // optional, for navigation into middle of a doc                  
    /* [in] */ LPCWSTR szTargetFrameName,// optional, for targeting frame-sets                             
    /* [in] */ IUnknown *pUnk,           // required - we'll search this for other necessary interfaces    
    /* [in] */ IBindCtx *pbc,            // optional. caller may register an IBSC in this                  
    /* [in] */ IBindStatusCallback *,                                                                      
    /* [in] */ DWORD grfHLNF,            // flags                                                          
    /* [in] */ DWORD dwReserved          // for future use, must be NULL                                   
);                                                                                                         
                                                                                                           
STDAPI URLOpenStreamA(LPUNKNOWN,LPCSTR,DWORD,LPBINDSTATUSCALLBACK);                                        
STDAPI URLOpenStreamW(LPUNKNOWN,LPCWSTR,DWORD,LPBINDSTATUSCALLBACK);                                       
STDAPI URLOpenPullStreamA(LPUNKNOWN,LPCSTR,DWORD,LPBINDSTATUSCALLBACK);                                    
STDAPI URLOpenPullStreamW(LPUNKNOWN,LPCWSTR,DWORD,LPBINDSTATUSCALLBACK);                                   
STDAPI URLDownloadToFileA(LPUNKNOWN,LPCSTR,LPCSTR,DWORD,LPBINDSTATUSCALLBACK);                             
STDAPI URLDownloadToFileW(LPUNKNOWN,LPCWSTR,LPCWSTR,DWORD,LPBINDSTATUSCALLBACK);                           
STDAPI URLDownloadToCacheFileA(LPUNKNOWN,LPCSTR,LPTSTR,DWORD,DWORD,LPBINDSTATUSCALLBACK);                  
STDAPI URLDownloadToCacheFileW(LPUNKNOWN,LPCWSTR,LPWSTR,DWORD,DWORD,LPBINDSTATUSCALLBACK);                 
STDAPI URLOpenBlockingStreamA(LPUNKNOWN,LPCSTR,LPSTREAM*,DWORD,LPBINDSTATUSCALLBACK);                      
STDAPI URLOpenBlockingStreamW(LPUNKNOWN,LPCWSTR,LPSTREAM*,DWORD,LPBINDSTATUSCALLBACK);                     
                                                                                                           
#ifdef UNICODE                                                                                             
#define URLOpenStream            URLOpenStreamW                                                            
#define URLOpenPullStream        URLOpenPullStreamW                                                        
#define URLDownloadToFile        URLDownloadToFileW                                                        
#define URLDownloadToCacheFile   URLDownloadToCacheFileW                                                   
#define URLOpenBlockingStream    URLOpenBlockingStreamW                                                    
#else                                                                                                      
#define URLOpenStream            URLOpenStreamA                                                            
#define URLOpenPullStream        URLOpenPullStreamA                                                        
#define URLDownloadToFile        URLDownloadToFileA                                                        
#define URLDownloadToCacheFile   URLDownloadToCacheFileA                                                   
#define URLOpenBlockingStream    URLOpenBlockingStreamA                                                    
#endif // !UNICODE                                                                                         
                                                                                                           
                                                                                                           
STDAPI HlinkGoBack(IUnknown *pUnk);                                                                        
STDAPI HlinkGoForward(IUnknown *pUnk);                                                                     
STDAPI HlinkNavigateString(IUnknown *pUnk, LPCWSTR szTarget);                                              
STDAPI HlinkNavigateMoniker(IUnknown *pUnk, IMoniker *pmkTarget);                                          
                                                                                                           
#ifndef  _URLMON_NO_ASYNC_PLUGABLE_PROTOCOLS_   








#ifndef _LPIINTERNET
#define _LPIINTERNET


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0180_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0180_v0_0_s_ifspec;

#ifndef __IInternet_INTERFACE_DEFINED__
#define __IInternet_INTERFACE_DEFINED__

/* interface IInternet */
/* [unique][uuid][object][local] */ 

typedef /* [unique] */ IInternet __RPC_FAR *LPIINTERNET;


EXTERN_C const IID IID_IInternet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9e0-baf9-11ce-8c82-00aa004ba90b")
    IInternet : public IUnknown
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IInternetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInternet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInternet __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInternet __RPC_FAR * This);
        
        END_INTERFACE
    } IInternetVtbl;

    interface IInternet
    {
        CONST_VTBL struct IInternetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IInternet_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0181 */
/* [local] */ 

#endif
#ifndef _LPIINTERNETBINDINFO
#define _LPIINTERNETBINDINFO


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0181_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0181_v0_0_s_ifspec;

#ifndef __IInternetBindInfo_INTERFACE_DEFINED__
#define __IInternetBindInfo_INTERFACE_DEFINED__

/* interface IInternetBindInfo */
/* [unique][uuid][object][local] */ 

typedef /* [unique] */ IInternetBindInfo __RPC_FAR *LPIINTERNETBINDINFO;

typedef 
enum tagBINDSTRING
    {	BINDSTRING_HEADERS	= 1,
	BINDSTRING_ACCEPT_MIMES	= BINDSTRING_HEADERS + 1,
	BINDSTRING_EXTRA_URL	= BINDSTRING_ACCEPT_MIMES + 1,
	BINDSTRING_LANGUAGE	= BINDSTRING_EXTRA_URL + 1,
	BINDSTRING_USERNAME	= BINDSTRING_LANGUAGE + 1,
	BINDSTRING_PASSWORD	= BINDSTRING_USERNAME + 1,
	BINDSTRING_UA_PIXELS	= BINDSTRING_PASSWORD + 1,
	BINDSTRING_UA_COLOR	= BINDSTRING_UA_PIXELS + 1,
	BINDSTRING_OS	= BINDSTRING_UA_COLOR + 1,
	BINDSTRING_USER_AGENT	= BINDSTRING_OS + 1,
	BINDSTRING_ACCEPT_ENCODINGS	= BINDSTRING_USER_AGENT + 1,
	BINDSTRING_POST_COOKIE	= BINDSTRING_ACCEPT_ENCODINGS + 1,
	BINDSTRING_POST_DATA_MIME	= BINDSTRING_POST_COOKIE + 1,
	BINDSTRING_URL	= BINDSTRING_POST_DATA_MIME + 1,
	BINDSTRING_IID	= BINDSTRING_URL + 1,
	BINDSTRING_FLAG_BIND_TO_OBJECT	= BINDSTRING_IID + 1,
	BINDSTRING_PTR_BIND_CONTEXT	= BINDSTRING_FLAG_BIND_TO_OBJECT + 1
    }	BINDSTRING;


EXTERN_C const IID IID_IInternetBindInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9e1-baf9-11ce-8c82-00aa004ba90b")
    IInternetBindInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetBindInfo( 
            /* [out] */ DWORD __RPC_FAR *grfBINDF,
            /* [unique][out][in] */ BINDINFO __RPC_FAR *pbindinfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBindString( 
            /* [in] */ ULONG ulStringType,
            /* [out][in] */ LPOLESTR __RPC_FAR *ppwzStr,
            /* [in] */ ULONG cEl,
            /* [out][in] */ ULONG __RPC_FAR *pcElFetched) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IInternetBindInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInternetBindInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInternetBindInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInternetBindInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBindInfo )( 
            IInternetBindInfo __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *grfBINDF,
            /* [unique][out][in] */ BINDINFO __RPC_FAR *pbindinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBindString )( 
            IInternetBindInfo __RPC_FAR * This,
            /* [in] */ ULONG ulStringType,
            /* [out][in] */ LPOLESTR __RPC_FAR *ppwzStr,
            /* [in] */ ULONG cEl,
            /* [out][in] */ ULONG __RPC_FAR *pcElFetched);
        
        END_INTERFACE
    } IInternetBindInfoVtbl;

    interface IInternetBindInfo
    {
        CONST_VTBL struct IInternetBindInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternetBindInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternetBindInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternetBindInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternetBindInfo_GetBindInfo(This,grfBINDF,pbindinfo)	\
    (This)->lpVtbl -> GetBindInfo(This,grfBINDF,pbindinfo)

#define IInternetBindInfo_GetBindString(This,ulStringType,ppwzStr,cEl,pcElFetched)	\
    (This)->lpVtbl -> GetBindString(This,ulStringType,ppwzStr,cEl,pcElFetched)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IInternetBindInfo_GetBindInfo_Proxy( 
    IInternetBindInfo __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *grfBINDF,
    /* [unique][out][in] */ BINDINFO __RPC_FAR *pbindinfo);


void __RPC_STUB IInternetBindInfo_GetBindInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetBindInfo_GetBindString_Proxy( 
    IInternetBindInfo __RPC_FAR * This,
    /* [in] */ ULONG ulStringType,
    /* [out][in] */ LPOLESTR __RPC_FAR *ppwzStr,
    /* [in] */ ULONG cEl,
    /* [out][in] */ ULONG __RPC_FAR *pcElFetched);


void __RPC_STUB IInternetBindInfo_GetBindString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IInternetBindInfo_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0182 */
/* [local] */ 

#endif
#ifndef _LPIINTERNETPROTOCOLROOT_DEFINED
#define _LPIINTERNETPROTOCOLROOT_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0182_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0182_v0_0_s_ifspec;

#ifndef __IInternetProtocolRoot_INTERFACE_DEFINED__
#define __IInternetProtocolRoot_INTERFACE_DEFINED__

/* interface IInternetProtocolRoot */
/* [unique][uuid][object][local] */ 

typedef /* [unique] */ IInternetProtocolRoot __RPC_FAR *LPIINTERNETPROTOCOLROOT;

typedef 
enum _tagPI_FLAGS
    {	PI_PARSE_URL	= 0x1,
	PI_FILTER_MODE	= 0x2,
	PI_FORCE_ASYNC	= 0x4,
	PI_USE_WORKERTHREAD	= 0x8,
	PI_MIMEVERIFICATION	= 0x10,
	PI_CLSIDLOOKUP	= 0x20,
	PI_DATAPROGRESS	= 0x40,
	PI_SYNCHRONOUS	= 0x80,
	PI_APARTMENTTHREADED	= 0x100,
	PI_CLASSINSTALL	= 0x200,
	PI_PASSONBINDCTX	= 0x2000,
	PI_NOMIMEHANDLER	= 0x8000,
	PI_LOADAPPDIRECT	= 0x4000,
	PD_FORCE_SWITCH	= 0x10000
    }	PI_FLAGS;

typedef struct _tagPROTOCOLDATA
    {
    DWORD grfFlags;
    DWORD dwState;
    LPVOID pData;
    ULONG cbData;
    }	PROTOCOLDATA;

typedef struct _tagStartParam
    {
    IID iid;
    IBindCtx __RPC_FAR *pIBindCtx;
    IUnknown __RPC_FAR *pItf;
    }	StartParam;


EXTERN_C const IID IID_IInternetProtocolRoot;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9e3-baf9-11ce-8c82-00aa004ba90b")
    IInternetProtocolRoot : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Start( 
            /* [in] */ LPCWSTR szUrl,
            /* [in] */ IInternetProtocolSink __RPC_FAR *pOIProtSink,
            /* [in] */ IInternetBindInfo __RPC_FAR *pOIBindInfo,
            /* [in] */ DWORD grfPI,
            /* [in] */ HANDLE_PTR dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Continue( 
            /* [in] */ PROTOCOLDATA __RPC_FAR *pProtocolData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Abort( 
            /* [in] */ HRESULT hrReason,
            /* [in] */ DWORD dwOptions) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Terminate( 
            /* [in] */ DWORD dwOptions) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Suspend( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Resume( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IInternetProtocolRootVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInternetProtocolRoot __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInternetProtocolRoot __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInternetProtocolRoot __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Start )( 
            IInternetProtocolRoot __RPC_FAR * This,
            /* [in] */ LPCWSTR szUrl,
            /* [in] */ IInternetProtocolSink __RPC_FAR *pOIProtSink,
            /* [in] */ IInternetBindInfo __RPC_FAR *pOIBindInfo,
            /* [in] */ DWORD grfPI,
            /* [in] */ HANDLE_PTR dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Continue )( 
            IInternetProtocolRoot __RPC_FAR * This,
            /* [in] */ PROTOCOLDATA __RPC_FAR *pProtocolData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Abort )( 
            IInternetProtocolRoot __RPC_FAR * This,
            /* [in] */ HRESULT hrReason,
            /* [in] */ DWORD dwOptions);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Terminate )( 
            IInternetProtocolRoot __RPC_FAR * This,
            /* [in] */ DWORD dwOptions);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Suspend )( 
            IInternetProtocolRoot __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Resume )( 
            IInternetProtocolRoot __RPC_FAR * This);
        
        END_INTERFACE
    } IInternetProtocolRootVtbl;

    interface IInternetProtocolRoot
    {
        CONST_VTBL struct IInternetProtocolRootVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternetProtocolRoot_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternetProtocolRoot_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternetProtocolRoot_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternetProtocolRoot_Start(This,szUrl,pOIProtSink,pOIBindInfo,grfPI,dwReserved)	\
    (This)->lpVtbl -> Start(This,szUrl,pOIProtSink,pOIBindInfo,grfPI,dwReserved)

#define IInternetProtocolRoot_Continue(This,pProtocolData)	\
    (This)->lpVtbl -> Continue(This,pProtocolData)

#define IInternetProtocolRoot_Abort(This,hrReason,dwOptions)	\
    (This)->lpVtbl -> Abort(This,hrReason,dwOptions)

#define IInternetProtocolRoot_Terminate(This,dwOptions)	\
    (This)->lpVtbl -> Terminate(This,dwOptions)

#define IInternetProtocolRoot_Suspend(This)	\
    (This)->lpVtbl -> Suspend(This)

#define IInternetProtocolRoot_Resume(This)	\
    (This)->lpVtbl -> Resume(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IInternetProtocolRoot_Start_Proxy( 
    IInternetProtocolRoot __RPC_FAR * This,
    /* [in] */ LPCWSTR szUrl,
    /* [in] */ IInternetProtocolSink __RPC_FAR *pOIProtSink,
    /* [in] */ IInternetBindInfo __RPC_FAR *pOIBindInfo,
    /* [in] */ DWORD grfPI,
    /* [in] */ HANDLE_PTR dwReserved);


void __RPC_STUB IInternetProtocolRoot_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetProtocolRoot_Continue_Proxy( 
    IInternetProtocolRoot __RPC_FAR * This,
    /* [in] */ PROTOCOLDATA __RPC_FAR *pProtocolData);


void __RPC_STUB IInternetProtocolRoot_Continue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetProtocolRoot_Abort_Proxy( 
    IInternetProtocolRoot __RPC_FAR * This,
    /* [in] */ HRESULT hrReason,
    /* [in] */ DWORD dwOptions);


void __RPC_STUB IInternetProtocolRoot_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetProtocolRoot_Terminate_Proxy( 
    IInternetProtocolRoot __RPC_FAR * This,
    /* [in] */ DWORD dwOptions);


void __RPC_STUB IInternetProtocolRoot_Terminate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetProtocolRoot_Suspend_Proxy( 
    IInternetProtocolRoot __RPC_FAR * This);


void __RPC_STUB IInternetProtocolRoot_Suspend_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetProtocolRoot_Resume_Proxy( 
    IInternetProtocolRoot __RPC_FAR * This);


void __RPC_STUB IInternetProtocolRoot_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IInternetProtocolRoot_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0183 */
/* [local] */ 

#endif
#ifndef _LPIINTERNETPROTOCOL_DEFINED
#define _LPIINTERNETPROTOCOL_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0183_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0183_v0_0_s_ifspec;

#ifndef __IInternetProtocol_INTERFACE_DEFINED__
#define __IInternetProtocol_INTERFACE_DEFINED__

/* interface IInternetProtocol */
/* [unique][uuid][object][local] */ 

typedef /* [unique] */ IInternetProtocol __RPC_FAR *LPIINTERNETPROTOCOL;


EXTERN_C const IID IID_IInternetProtocol;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9e4-baf9-11ce-8c82-00aa004ba90b")
    IInternetProtocol : public IInternetProtocolRoot
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Read( 
            /* [length_is][size_is][out][in] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Seek( 
            /* [in] */ LARGE_INTEGER dlibMove,
            /* [in] */ DWORD dwOrigin,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *plibNewPosition) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LockRequest( 
            /* [in] */ DWORD dwOptions) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnlockRequest( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IInternetProtocolVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInternetProtocol __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInternetProtocol __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInternetProtocol __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Start )( 
            IInternetProtocol __RPC_FAR * This,
            /* [in] */ LPCWSTR szUrl,
            /* [in] */ IInternetProtocolSink __RPC_FAR *pOIProtSink,
            /* [in] */ IInternetBindInfo __RPC_FAR *pOIBindInfo,
            /* [in] */ DWORD grfPI,
            /* [in] */ HANDLE_PTR dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Continue )( 
            IInternetProtocol __RPC_FAR * This,
            /* [in] */ PROTOCOLDATA __RPC_FAR *pProtocolData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Abort )( 
            IInternetProtocol __RPC_FAR * This,
            /* [in] */ HRESULT hrReason,
            /* [in] */ DWORD dwOptions);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Terminate )( 
            IInternetProtocol __RPC_FAR * This,
            /* [in] */ DWORD dwOptions);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Suspend )( 
            IInternetProtocol __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Resume )( 
            IInternetProtocol __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Read )( 
            IInternetProtocol __RPC_FAR * This,
            /* [length_is][size_is][out][in] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Seek )( 
            IInternetProtocol __RPC_FAR * This,
            /* [in] */ LARGE_INTEGER dlibMove,
            /* [in] */ DWORD dwOrigin,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *plibNewPosition);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LockRequest )( 
            IInternetProtocol __RPC_FAR * This,
            /* [in] */ DWORD dwOptions);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UnlockRequest )( 
            IInternetProtocol __RPC_FAR * This);
        
        END_INTERFACE
    } IInternetProtocolVtbl;

    interface IInternetProtocol
    {
        CONST_VTBL struct IInternetProtocolVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternetProtocol_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternetProtocol_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternetProtocol_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternetProtocol_Start(This,szUrl,pOIProtSink,pOIBindInfo,grfPI,dwReserved)	\
    (This)->lpVtbl -> Start(This,szUrl,pOIProtSink,pOIBindInfo,grfPI,dwReserved)

#define IInternetProtocol_Continue(This,pProtocolData)	\
    (This)->lpVtbl -> Continue(This,pProtocolData)

#define IInternetProtocol_Abort(This,hrReason,dwOptions)	\
    (This)->lpVtbl -> Abort(This,hrReason,dwOptions)

#define IInternetProtocol_Terminate(This,dwOptions)	\
    (This)->lpVtbl -> Terminate(This,dwOptions)

#define IInternetProtocol_Suspend(This)	\
    (This)->lpVtbl -> Suspend(This)

#define IInternetProtocol_Resume(This)	\
    (This)->lpVtbl -> Resume(This)


#define IInternetProtocol_Read(This,pv,cb,pcbRead)	\
    (This)->lpVtbl -> Read(This,pv,cb,pcbRead)

#define IInternetProtocol_Seek(This,dlibMove,dwOrigin,plibNewPosition)	\
    (This)->lpVtbl -> Seek(This,dlibMove,dwOrigin,plibNewPosition)

#define IInternetProtocol_LockRequest(This,dwOptions)	\
    (This)->lpVtbl -> LockRequest(This,dwOptions)

#define IInternetProtocol_UnlockRequest(This)	\
    (This)->lpVtbl -> UnlockRequest(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IInternetProtocol_Read_Proxy( 
    IInternetProtocol __RPC_FAR * This,
    /* [length_is][size_is][out][in] */ void __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbRead);


void __RPC_STUB IInternetProtocol_Read_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetProtocol_Seek_Proxy( 
    IInternetProtocol __RPC_FAR * This,
    /* [in] */ LARGE_INTEGER dlibMove,
    /* [in] */ DWORD dwOrigin,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *plibNewPosition);


void __RPC_STUB IInternetProtocol_Seek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetProtocol_LockRequest_Proxy( 
    IInternetProtocol __RPC_FAR * This,
    /* [in] */ DWORD dwOptions);


void __RPC_STUB IInternetProtocol_LockRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetProtocol_UnlockRequest_Proxy( 
    IInternetProtocol __RPC_FAR * This);


void __RPC_STUB IInternetProtocol_UnlockRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IInternetProtocol_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0184 */
/* [local] */ 

#endif
#ifndef _LPIINTERNETPROTOCOLSINK_DEFINED
#define _LPIINTERNETPROTOCOLSINK_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0184_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0184_v0_0_s_ifspec;

#ifndef __IInternetProtocolSink_INTERFACE_DEFINED__
#define __IInternetProtocolSink_INTERFACE_DEFINED__

/* interface IInternetProtocolSink */
/* [unique][uuid][object][local] */ 

typedef /* [unique] */ IInternetProtocolSink __RPC_FAR *LPIINTERNETPROTOCOLSINK;


EXTERN_C const IID IID_IInternetProtocolSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9e5-baf9-11ce-8c82-00aa004ba90b")
    IInternetProtocolSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Switch( 
            /* [in] */ PROTOCOLDATA __RPC_FAR *pProtocolData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReportProgress( 
            /* [in] */ ULONG ulStatusCode,
            /* [in] */ LPCWSTR szStatusText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReportData( 
            /* [in] */ DWORD grfBSCF,
            /* [in] */ ULONG ulProgress,
            /* [in] */ ULONG ulProgressMax) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReportResult( 
            /* [in] */ HRESULT hrResult,
            /* [in] */ DWORD dwError,
            /* [in] */ LPCWSTR szResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IInternetProtocolSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInternetProtocolSink __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInternetProtocolSink __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInternetProtocolSink __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Switch )( 
            IInternetProtocolSink __RPC_FAR * This,
            /* [in] */ PROTOCOLDATA __RPC_FAR *pProtocolData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReportProgress )( 
            IInternetProtocolSink __RPC_FAR * This,
            /* [in] */ ULONG ulStatusCode,
            /* [in] */ LPCWSTR szStatusText);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReportData )( 
            IInternetProtocolSink __RPC_FAR * This,
            /* [in] */ DWORD grfBSCF,
            /* [in] */ ULONG ulProgress,
            /* [in] */ ULONG ulProgressMax);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReportResult )( 
            IInternetProtocolSink __RPC_FAR * This,
            /* [in] */ HRESULT hrResult,
            /* [in] */ DWORD dwError,
            /* [in] */ LPCWSTR szResult);
        
        END_INTERFACE
    } IInternetProtocolSinkVtbl;

    interface IInternetProtocolSink
    {
        CONST_VTBL struct IInternetProtocolSinkVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternetProtocolSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternetProtocolSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternetProtocolSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternetProtocolSink_Switch(This,pProtocolData)	\
    (This)->lpVtbl -> Switch(This,pProtocolData)

#define IInternetProtocolSink_ReportProgress(This,ulStatusCode,szStatusText)	\
    (This)->lpVtbl -> ReportProgress(This,ulStatusCode,szStatusText)

#define IInternetProtocolSink_ReportData(This,grfBSCF,ulProgress,ulProgressMax)	\
    (This)->lpVtbl -> ReportData(This,grfBSCF,ulProgress,ulProgressMax)

#define IInternetProtocolSink_ReportResult(This,hrResult,dwError,szResult)	\
    (This)->lpVtbl -> ReportResult(This,hrResult,dwError,szResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IInternetProtocolSink_Switch_Proxy( 
    IInternetProtocolSink __RPC_FAR * This,
    /* [in] */ PROTOCOLDATA __RPC_FAR *pProtocolData);


void __RPC_STUB IInternetProtocolSink_Switch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetProtocolSink_ReportProgress_Proxy( 
    IInternetProtocolSink __RPC_FAR * This,
    /* [in] */ ULONG ulStatusCode,
    /* [in] */ LPCWSTR szStatusText);


void __RPC_STUB IInternetProtocolSink_ReportProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetProtocolSink_ReportData_Proxy( 
    IInternetProtocolSink __RPC_FAR * This,
    /* [in] */ DWORD grfBSCF,
    /* [in] */ ULONG ulProgress,
    /* [in] */ ULONG ulProgressMax);


void __RPC_STUB IInternetProtocolSink_ReportData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetProtocolSink_ReportResult_Proxy( 
    IInternetProtocolSink __RPC_FAR * This,
    /* [in] */ HRESULT hrResult,
    /* [in] */ DWORD dwError,
    /* [in] */ LPCWSTR szResult);


void __RPC_STUB IInternetProtocolSink_ReportResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IInternetProtocolSink_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0185 */
/* [local] */ 

#endif
#ifndef _LPIINTERNETPROTOCOLSINKSTACKABLE_DEFINED
#define _LPIINTERNETPROTOCOLSINKSTACKABLE_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0185_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0185_v0_0_s_ifspec;

#ifndef __IInternetProtocolSinkStackable_INTERFACE_DEFINED__
#define __IInternetProtocolSinkStackable_INTERFACE_DEFINED__

/* interface IInternetProtocolSinkStackable */
/* [unique][uuid][object][local] */ 

typedef /* [unique] */ IInternetProtocolSinkStackable __RPC_FAR *LPIINTERNETPROTOCOLSINKStackable;


EXTERN_C const IID IID_IInternetProtocolSinkStackable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9f0-baf9-11ce-8c82-00aa004ba90b")
    IInternetProtocolSinkStackable : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SwitchSink( 
            /* [in] */ IInternetProtocolSink __RPC_FAR *pOIProtSink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CommitSwitch( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RollbackSwitch( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IInternetProtocolSinkStackableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInternetProtocolSinkStackable __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInternetProtocolSinkStackable __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInternetProtocolSinkStackable __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SwitchSink )( 
            IInternetProtocolSinkStackable __RPC_FAR * This,
            /* [in] */ IInternetProtocolSink __RPC_FAR *pOIProtSink);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CommitSwitch )( 
            IInternetProtocolSinkStackable __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RollbackSwitch )( 
            IInternetProtocolSinkStackable __RPC_FAR * This);
        
        END_INTERFACE
    } IInternetProtocolSinkStackableVtbl;

    interface IInternetProtocolSinkStackable
    {
        CONST_VTBL struct IInternetProtocolSinkStackableVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternetProtocolSinkStackable_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternetProtocolSinkStackable_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternetProtocolSinkStackable_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternetProtocolSinkStackable_SwitchSink(This,pOIProtSink)	\
    (This)->lpVtbl -> SwitchSink(This,pOIProtSink)

#define IInternetProtocolSinkStackable_CommitSwitch(This)	\
    (This)->lpVtbl -> CommitSwitch(This)

#define IInternetProtocolSinkStackable_RollbackSwitch(This)	\
    (This)->lpVtbl -> RollbackSwitch(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IInternetProtocolSinkStackable_SwitchSink_Proxy( 
    IInternetProtocolSinkStackable __RPC_FAR * This,
    /* [in] */ IInternetProtocolSink __RPC_FAR *pOIProtSink);


void __RPC_STUB IInternetProtocolSinkStackable_SwitchSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetProtocolSinkStackable_CommitSwitch_Proxy( 
    IInternetProtocolSinkStackable __RPC_FAR * This);


void __RPC_STUB IInternetProtocolSinkStackable_CommitSwitch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetProtocolSinkStackable_RollbackSwitch_Proxy( 
    IInternetProtocolSinkStackable __RPC_FAR * This);


void __RPC_STUB IInternetProtocolSinkStackable_RollbackSwitch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IInternetProtocolSinkStackable_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0186 */
/* [local] */ 

#endif
#ifndef _LPIINTERNETSESSION_DEFINED
#define _LPIINTERNETSESSION_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0186_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0186_v0_0_s_ifspec;

#ifndef __IInternetSession_INTERFACE_DEFINED__
#define __IInternetSession_INTERFACE_DEFINED__

/* interface IInternetSession */
/* [unique][uuid][object][local] */ 

typedef /* [unique] */ IInternetSession __RPC_FAR *LPIINTERNETSESSION;

typedef 
enum _tagOIBDG_FLAGS
    {	OIBDG_APARTMENTTHREADED	= 0x100,
	OIBDG_DATAONLY	= 0x1000
    }	OIBDG_FLAGS;


EXTERN_C const IID IID_IInternetSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9e7-baf9-11ce-8c82-00aa004ba90b")
    IInternetSession : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterNameSpace( 
            /* [in] */ IClassFactory __RPC_FAR *pCF,
            /* [in] */ REFCLSID rclsid,
            /* [in] */ LPCWSTR pwzProtocol,
            /* [in] */ ULONG cPatterns,
            /* [in] */ const LPCWSTR __RPC_FAR *ppwzPatterns,
            /* [in] */ DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterNameSpace( 
            /* [in] */ IClassFactory __RPC_FAR *pCF,
            /* [in] */ LPCWSTR pszProtocol) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterMimeFilter( 
            /* [in] */ IClassFactory __RPC_FAR *pCF,
            /* [in] */ REFCLSID rclsid,
            /* [in] */ LPCWSTR pwzType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterMimeFilter( 
            /* [in] */ IClassFactory __RPC_FAR *pCF,
            /* [in] */ LPCWSTR pwzType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateBinding( 
            /* [in] */ LPBC pBC,
            /* [in] */ LPCWSTR szUrl,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [unique][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppUnk,
            /* [unique][out] */ IInternetProtocol __RPC_FAR *__RPC_FAR *ppOInetProt,
            /* [in] */ DWORD dwOption) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSessionOption( 
            /* [in] */ DWORD dwOption,
            /* [in] */ LPVOID pBuffer,
            /* [in] */ DWORD dwBufferLength,
            /* [in] */ DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSessionOption( 
            /* [in] */ DWORD dwOption,
            /* [out][in] */ LPVOID pBuffer,
            /* [out][in] */ DWORD __RPC_FAR *pdwBufferLength,
            /* [in] */ DWORD dwReserved) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IInternetSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInternetSession __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInternetSession __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInternetSession __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RegisterNameSpace )( 
            IInternetSession __RPC_FAR * This,
            /* [in] */ IClassFactory __RPC_FAR *pCF,
            /* [in] */ REFCLSID rclsid,
            /* [in] */ LPCWSTR pwzProtocol,
            /* [in] */ ULONG cPatterns,
            /* [in] */ const LPCWSTR __RPC_FAR *ppwzPatterns,
            /* [in] */ DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UnregisterNameSpace )( 
            IInternetSession __RPC_FAR * This,
            /* [in] */ IClassFactory __RPC_FAR *pCF,
            /* [in] */ LPCWSTR pszProtocol);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RegisterMimeFilter )( 
            IInternetSession __RPC_FAR * This,
            /* [in] */ IClassFactory __RPC_FAR *pCF,
            /* [in] */ REFCLSID rclsid,
            /* [in] */ LPCWSTR pwzType);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UnregisterMimeFilter )( 
            IInternetSession __RPC_FAR * This,
            /* [in] */ IClassFactory __RPC_FAR *pCF,
            /* [in] */ LPCWSTR pwzType);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateBinding )( 
            IInternetSession __RPC_FAR * This,
            /* [in] */ LPBC pBC,
            /* [in] */ LPCWSTR szUrl,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [unique][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppUnk,
            /* [unique][out] */ IInternetProtocol __RPC_FAR *__RPC_FAR *ppOInetProt,
            /* [in] */ DWORD dwOption);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSessionOption )( 
            IInternetSession __RPC_FAR * This,
            /* [in] */ DWORD dwOption,
            /* [in] */ LPVOID pBuffer,
            /* [in] */ DWORD dwBufferLength,
            /* [in] */ DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSessionOption )( 
            IInternetSession __RPC_FAR * This,
            /* [in] */ DWORD dwOption,
            /* [out][in] */ LPVOID pBuffer,
            /* [out][in] */ DWORD __RPC_FAR *pdwBufferLength,
            /* [in] */ DWORD dwReserved);
        
        END_INTERFACE
    } IInternetSessionVtbl;

    interface IInternetSession
    {
        CONST_VTBL struct IInternetSessionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternetSession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternetSession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternetSession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternetSession_RegisterNameSpace(This,pCF,rclsid,pwzProtocol,cPatterns,ppwzPatterns,dwReserved)	\
    (This)->lpVtbl -> RegisterNameSpace(This,pCF,rclsid,pwzProtocol,cPatterns,ppwzPatterns,dwReserved)

#define IInternetSession_UnregisterNameSpace(This,pCF,pszProtocol)	\
    (This)->lpVtbl -> UnregisterNameSpace(This,pCF,pszProtocol)

#define IInternetSession_RegisterMimeFilter(This,pCF,rclsid,pwzType)	\
    (This)->lpVtbl -> RegisterMimeFilter(This,pCF,rclsid,pwzType)

#define IInternetSession_UnregisterMimeFilter(This,pCF,pwzType)	\
    (This)->lpVtbl -> UnregisterMimeFilter(This,pCF,pwzType)

#define IInternetSession_CreateBinding(This,pBC,szUrl,pUnkOuter,ppUnk,ppOInetProt,dwOption)	\
    (This)->lpVtbl -> CreateBinding(This,pBC,szUrl,pUnkOuter,ppUnk,ppOInetProt,dwOption)

#define IInternetSession_SetSessionOption(This,dwOption,pBuffer,dwBufferLength,dwReserved)	\
    (This)->lpVtbl -> SetSessionOption(This,dwOption,pBuffer,dwBufferLength,dwReserved)

#define IInternetSession_GetSessionOption(This,dwOption,pBuffer,pdwBufferLength,dwReserved)	\
    (This)->lpVtbl -> GetSessionOption(This,dwOption,pBuffer,pdwBufferLength,dwReserved)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IInternetSession_RegisterNameSpace_Proxy( 
    IInternetSession __RPC_FAR * This,
    /* [in] */ IClassFactory __RPC_FAR *pCF,
    /* [in] */ REFCLSID rclsid,
    /* [in] */ LPCWSTR pwzProtocol,
    /* [in] */ ULONG cPatterns,
    /* [in] */ const LPCWSTR __RPC_FAR *ppwzPatterns,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IInternetSession_RegisterNameSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetSession_UnregisterNameSpace_Proxy( 
    IInternetSession __RPC_FAR * This,
    /* [in] */ IClassFactory __RPC_FAR *pCF,
    /* [in] */ LPCWSTR pszProtocol);


void __RPC_STUB IInternetSession_UnregisterNameSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetSession_RegisterMimeFilter_Proxy( 
    IInternetSession __RPC_FAR * This,
    /* [in] */ IClassFactory __RPC_FAR *pCF,
    /* [in] */ REFCLSID rclsid,
    /* [in] */ LPCWSTR pwzType);


void __RPC_STUB IInternetSession_RegisterMimeFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetSession_UnregisterMimeFilter_Proxy( 
    IInternetSession __RPC_FAR * This,
    /* [in] */ IClassFactory __RPC_FAR *pCF,
    /* [in] */ LPCWSTR pwzType);


void __RPC_STUB IInternetSession_UnregisterMimeFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetSession_CreateBinding_Proxy( 
    IInternetSession __RPC_FAR * This,
    /* [in] */ LPBC pBC,
    /* [in] */ LPCWSTR szUrl,
    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
    /* [unique][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppUnk,
    /* [unique][out] */ IInternetProtocol __RPC_FAR *__RPC_FAR *ppOInetProt,
    /* [in] */ DWORD dwOption);


void __RPC_STUB IInternetSession_CreateBinding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetSession_SetSessionOption_Proxy( 
    IInternetSession __RPC_FAR * This,
    /* [in] */ DWORD dwOption,
    /* [in] */ LPVOID pBuffer,
    /* [in] */ DWORD dwBufferLength,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IInternetSession_SetSessionOption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetSession_GetSessionOption_Proxy( 
    IInternetSession __RPC_FAR * This,
    /* [in] */ DWORD dwOption,
    /* [out][in] */ LPVOID pBuffer,
    /* [out][in] */ DWORD __RPC_FAR *pdwBufferLength,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IInternetSession_GetSessionOption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IInternetSession_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0187 */
/* [local] */ 

#endif
#ifndef _LPIINTERNETTHREADSWITCH_DEFINED
#define _LPIINTERNETTHREADSWITCH_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0187_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0187_v0_0_s_ifspec;

#ifndef __IInternetThreadSwitch_INTERFACE_DEFINED__
#define __IInternetThreadSwitch_INTERFACE_DEFINED__

/* interface IInternetThreadSwitch */
/* [unique][uuid][object][local] */ 

typedef /* [unique] */ IInternetThreadSwitch __RPC_FAR *LPIINTERNETTHREADSWITCH;


EXTERN_C const IID IID_IInternetThreadSwitch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9e8-baf9-11ce-8c82-00aa004ba90b")
    IInternetThreadSwitch : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Prepare( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Continue( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IInternetThreadSwitchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInternetThreadSwitch __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInternetThreadSwitch __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInternetThreadSwitch __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Prepare )( 
            IInternetThreadSwitch __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Continue )( 
            IInternetThreadSwitch __RPC_FAR * This);
        
        END_INTERFACE
    } IInternetThreadSwitchVtbl;

    interface IInternetThreadSwitch
    {
        CONST_VTBL struct IInternetThreadSwitchVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternetThreadSwitch_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternetThreadSwitch_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternetThreadSwitch_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternetThreadSwitch_Prepare(This)	\
    (This)->lpVtbl -> Prepare(This)

#define IInternetThreadSwitch_Continue(This)	\
    (This)->lpVtbl -> Continue(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IInternetThreadSwitch_Prepare_Proxy( 
    IInternetThreadSwitch __RPC_FAR * This);


void __RPC_STUB IInternetThreadSwitch_Prepare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetThreadSwitch_Continue_Proxy( 
    IInternetThreadSwitch __RPC_FAR * This);


void __RPC_STUB IInternetThreadSwitch_Continue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IInternetThreadSwitch_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0188 */
/* [local] */ 

#endif
#ifndef _LPIINTERNETPRIORITY_DEFINED
#define _LPIINTERNETPRIORITY_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0188_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0188_v0_0_s_ifspec;

#ifndef __IInternetPriority_INTERFACE_DEFINED__
#define __IInternetPriority_INTERFACE_DEFINED__

/* interface IInternetPriority */
/* [unique][uuid][object][local] */ 

typedef /* [unique] */ IInternetPriority __RPC_FAR *LPIINTERNETPRIORITY;


EXTERN_C const IID IID_IInternetPriority;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9eb-baf9-11ce-8c82-00aa004ba90b")
    IInternetPriority : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetPriority( 
            /* [in] */ LONG nPriority) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPriority( 
            /* [out] */ LONG __RPC_FAR *pnPriority) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IInternetPriorityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInternetPriority __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInternetPriority __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInternetPriority __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetPriority )( 
            IInternetPriority __RPC_FAR * This,
            /* [in] */ LONG nPriority);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPriority )( 
            IInternetPriority __RPC_FAR * This,
            /* [out] */ LONG __RPC_FAR *pnPriority);
        
        END_INTERFACE
    } IInternetPriorityVtbl;

    interface IInternetPriority
    {
        CONST_VTBL struct IInternetPriorityVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternetPriority_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternetPriority_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternetPriority_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternetPriority_SetPriority(This,nPriority)	\
    (This)->lpVtbl -> SetPriority(This,nPriority)

#define IInternetPriority_GetPriority(This,pnPriority)	\
    (This)->lpVtbl -> GetPriority(This,pnPriority)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IInternetPriority_SetPriority_Proxy( 
    IInternetPriority __RPC_FAR * This,
    /* [in] */ LONG nPriority);


void __RPC_STUB IInternetPriority_SetPriority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetPriority_GetPriority_Proxy( 
    IInternetPriority __RPC_FAR * This,
    /* [out] */ LONG __RPC_FAR *pnPriority);


void __RPC_STUB IInternetPriority_GetPriority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IInternetPriority_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0189 */
/* [local] */ 

#endif
#ifndef _LPIINTERNETPROTOCOLINFO_DEFINED
#define _LPIINTERNETPROTOCOLINFO_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0189_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0189_v0_0_s_ifspec;

#ifndef __IInternetProtocolInfo_INTERFACE_DEFINED__
#define __IInternetProtocolInfo_INTERFACE_DEFINED__

/* interface IInternetProtocolInfo */
/* [unique][uuid][object][local] */ 

typedef /* [unique] */ IInternetProtocolInfo __RPC_FAR *LPIINTERNETPROTOCOLINFO;

typedef 
enum _tagPARSEACTION
    {	PARSE_CANONICALIZE	= 1,
	PARSE_FRIENDLY	= PARSE_CANONICALIZE + 1,
	PARSE_SECURITY_URL	= PARSE_FRIENDLY + 1,
	PARSE_ROOTDOCUMENT	= PARSE_SECURITY_URL + 1,
	PARSE_DOCUMENT	= PARSE_ROOTDOCUMENT + 1,
	PARSE_ANCHOR	= PARSE_DOCUMENT + 1,
	PARSE_ENCODE	= PARSE_ANCHOR + 1,
	PARSE_DECODE	= PARSE_ENCODE + 1,
	PARSE_PATH_FROM_URL	= PARSE_DECODE + 1,
	PARSE_URL_FROM_PATH	= PARSE_PATH_FROM_URL + 1,
	PARSE_MIME	= PARSE_URL_FROM_PATH + 1,
	PARSE_SERVER	= PARSE_MIME + 1,
	PARSE_SCHEMA	= PARSE_SERVER + 1,
	PARSE_SITE	= PARSE_SCHEMA + 1,
	PARSE_DOMAIN	= PARSE_SITE + 1,
	PARSE_LOCATION	= PARSE_DOMAIN + 1,
	PARSE_SECURITY_DOMAIN	= PARSE_LOCATION + 1,
	PARSE_ESCAPE	= PARSE_SECURITY_DOMAIN + 1,
	PARSE_UNESCAPE	= PARSE_ESCAPE + 1
    }	PARSEACTION;

typedef 
enum _tagPSUACTION
    {	PSU_DEFAULT	= 1,
	PSU_SECURITY_URL_ONLY	= PSU_DEFAULT + 1
    }	PSUACTION;

typedef 
enum _tagQUERYOPTION
    {	QUERY_EXPIRATION_DATE	= 1,
	QUERY_TIME_OF_LAST_CHANGE	= QUERY_EXPIRATION_DATE + 1,
	QUERY_CONTENT_ENCODING	= QUERY_TIME_OF_LAST_CHANGE + 1,
	QUERY_CONTENT_TYPE	= QUERY_CONTENT_ENCODING + 1,
	QUERY_REFRESH	= QUERY_CONTENT_TYPE + 1,
	QUERY_RECOMBINE	= QUERY_REFRESH + 1,
	QUERY_CAN_NAVIGATE	= QUERY_RECOMBINE + 1,
	QUERY_USES_NETWORK	= QUERY_CAN_NAVIGATE + 1,
	QUERY_IS_CACHED	= QUERY_USES_NETWORK + 1,
	QUERY_IS_INSTALLEDENTRY	= QUERY_IS_CACHED + 1,
	QUERY_IS_CACHED_OR_MAPPED	= QUERY_IS_INSTALLEDENTRY + 1,
	QUERY_USES_CACHE	= QUERY_IS_CACHED_OR_MAPPED + 1,
	QUERY_IS_SECURE	= QUERY_USES_CACHE + 1,
	QUERY_IS_SAFE	= QUERY_IS_SECURE + 1
    }	QUERYOPTION;


EXTERN_C const IID IID_IInternetProtocolInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9ec-baf9-11ce-8c82-00aa004ba90b")
    IInternetProtocolInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ParseUrl( 
            /* [in] */ LPCWSTR pwzUrl,
            /* [in] */ PARSEACTION ParseAction,
            /* [in] */ DWORD dwParseFlags,
            /* [out] */ LPWSTR pwzResult,
            /* [in] */ DWORD cchResult,
            /* [out] */ DWORD __RPC_FAR *pcchResult,
            /* [in] */ DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CombineUrl( 
            /* [in] */ LPCWSTR pwzBaseUrl,
            /* [in] */ LPCWSTR pwzRelativeUrl,
            /* [in] */ DWORD dwCombineFlags,
            /* [out] */ LPWSTR pwzResult,
            /* [in] */ DWORD cchResult,
            /* [out] */ DWORD __RPC_FAR *pcchResult,
            /* [in] */ DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CompareUrl( 
            /* [in] */ LPCWSTR pwzUrl1,
            /* [in] */ LPCWSTR pwzUrl2,
            /* [in] */ DWORD dwCompareFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryInfo( 
            /* [in] */ LPCWSTR pwzUrl,
            /* [in] */ QUERYOPTION OueryOption,
            /* [in] */ DWORD dwQueryFlags,
            /* [size_is][out][in] */ LPVOID pBuffer,
            /* [in] */ DWORD cbBuffer,
            /* [out][in] */ DWORD __RPC_FAR *pcbBuf,
            /* [in] */ DWORD dwReserved) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IInternetProtocolInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInternetProtocolInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInternetProtocolInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInternetProtocolInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ParseUrl )( 
            IInternetProtocolInfo __RPC_FAR * This,
            /* [in] */ LPCWSTR pwzUrl,
            /* [in] */ PARSEACTION ParseAction,
            /* [in] */ DWORD dwParseFlags,
            /* [out] */ LPWSTR pwzResult,
            /* [in] */ DWORD cchResult,
            /* [out] */ DWORD __RPC_FAR *pcchResult,
            /* [in] */ DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CombineUrl )( 
            IInternetProtocolInfo __RPC_FAR * This,
            /* [in] */ LPCWSTR pwzBaseUrl,
            /* [in] */ LPCWSTR pwzRelativeUrl,
            /* [in] */ DWORD dwCombineFlags,
            /* [out] */ LPWSTR pwzResult,
            /* [in] */ DWORD cchResult,
            /* [out] */ DWORD __RPC_FAR *pcchResult,
            /* [in] */ DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CompareUrl )( 
            IInternetProtocolInfo __RPC_FAR * This,
            /* [in] */ LPCWSTR pwzUrl1,
            /* [in] */ LPCWSTR pwzUrl2,
            /* [in] */ DWORD dwCompareFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInfo )( 
            IInternetProtocolInfo __RPC_FAR * This,
            /* [in] */ LPCWSTR pwzUrl,
            /* [in] */ QUERYOPTION OueryOption,
            /* [in] */ DWORD dwQueryFlags,
            /* [size_is][out][in] */ LPVOID pBuffer,
            /* [in] */ DWORD cbBuffer,
            /* [out][in] */ DWORD __RPC_FAR *pcbBuf,
            /* [in] */ DWORD dwReserved);
        
        END_INTERFACE
    } IInternetProtocolInfoVtbl;

    interface IInternetProtocolInfo
    {
        CONST_VTBL struct IInternetProtocolInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternetProtocolInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternetProtocolInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternetProtocolInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternetProtocolInfo_ParseUrl(This,pwzUrl,ParseAction,dwParseFlags,pwzResult,cchResult,pcchResult,dwReserved)	\
    (This)->lpVtbl -> ParseUrl(This,pwzUrl,ParseAction,dwParseFlags,pwzResult,cchResult,pcchResult,dwReserved)

#define IInternetProtocolInfo_CombineUrl(This,pwzBaseUrl,pwzRelativeUrl,dwCombineFlags,pwzResult,cchResult,pcchResult,dwReserved)	\
    (This)->lpVtbl -> CombineUrl(This,pwzBaseUrl,pwzRelativeUrl,dwCombineFlags,pwzResult,cchResult,pcchResult,dwReserved)

#define IInternetProtocolInfo_CompareUrl(This,pwzUrl1,pwzUrl2,dwCompareFlags)	\
    (This)->lpVtbl -> CompareUrl(This,pwzUrl1,pwzUrl2,dwCompareFlags)

#define IInternetProtocolInfo_QueryInfo(This,pwzUrl,OueryOption,dwQueryFlags,pBuffer,cbBuffer,pcbBuf,dwReserved)	\
    (This)->lpVtbl -> QueryInfo(This,pwzUrl,OueryOption,dwQueryFlags,pBuffer,cbBuffer,pcbBuf,dwReserved)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IInternetProtocolInfo_ParseUrl_Proxy( 
    IInternetProtocolInfo __RPC_FAR * This,
    /* [in] */ LPCWSTR pwzUrl,
    /* [in] */ PARSEACTION ParseAction,
    /* [in] */ DWORD dwParseFlags,
    /* [out] */ LPWSTR pwzResult,
    /* [in] */ DWORD cchResult,
    /* [out] */ DWORD __RPC_FAR *pcchResult,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IInternetProtocolInfo_ParseUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetProtocolInfo_CombineUrl_Proxy( 
    IInternetProtocolInfo __RPC_FAR * This,
    /* [in] */ LPCWSTR pwzBaseUrl,
    /* [in] */ LPCWSTR pwzRelativeUrl,
    /* [in] */ DWORD dwCombineFlags,
    /* [out] */ LPWSTR pwzResult,
    /* [in] */ DWORD cchResult,
    /* [out] */ DWORD __RPC_FAR *pcchResult,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IInternetProtocolInfo_CombineUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetProtocolInfo_CompareUrl_Proxy( 
    IInternetProtocolInfo __RPC_FAR * This,
    /* [in] */ LPCWSTR pwzUrl1,
    /* [in] */ LPCWSTR pwzUrl2,
    /* [in] */ DWORD dwCompareFlags);


void __RPC_STUB IInternetProtocolInfo_CompareUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetProtocolInfo_QueryInfo_Proxy( 
    IInternetProtocolInfo __RPC_FAR * This,
    /* [in] */ LPCWSTR pwzUrl,
    /* [in] */ QUERYOPTION OueryOption,
    /* [in] */ DWORD dwQueryFlags,
    /* [size_is][out][in] */ LPVOID pBuffer,
    /* [in] */ DWORD cbBuffer,
    /* [out][in] */ DWORD __RPC_FAR *pcbBuf,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IInternetProtocolInfo_QueryInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IInternetProtocolInfo_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0190 */
/* [local] */ 

#endif
#define IOInet               IInternet            
#define IOInetBindInfo       IInternetBindInfo    
#define IOInetProtocolRoot   IInternetProtocolRoot
#define IOInetProtocol       IInternetProtocol    
#define IOInetProtocolSink   IInternetProtocolSink
#define IOInetProtocolInfo   IInternetProtocolInfo
#define IOInetSession        IInternetSession     
#define IOInetPriority       IInternetPriority    
#define IOInetThreadSwitch   IInternetThreadSwitch
#define IOInetProtocolSinkStackable   IInternetProtocolSinkStackable
#define LPOINET              LPIINTERNET             
#define LPOINETPROTOCOLINFO  LPIINTERNETPROTOCOLINFO 
#define LPOINETBINDINFO      LPIINTERNETBINDINFO     
#define LPOINETPROTOCOLROOT  LPIINTERNETPROTOCOLROOT 
#define LPOINETPROTOCOL      LPIINTERNETPROTOCOL     
#define LPOINETPROTOCOLSINK  LPIINTERNETPROTOCOLSINK 
#define LPOINETSESSION       LPIINTERNETSESSION      
#define LPOINETTHREADSWITCH  LPIINTERNETTHREADSWITCH 
#define LPOINETPRIORITY      LPIINTERNETPRIORITY     
#define LPOINETPROTOCOLINFO  LPIINTERNETPROTOCOLINFO 
#define LPOINETPROTOCOLSINKSTACKABLE  LPIINTERNETPROTOCOLSINKSTACKABLE 
#define IID_IOInet               IID_IInternet            
#define IID_IOInetBindInfo       IID_IInternetBindInfo    
#define IID_IOInetProtocolRoot   IID_IInternetProtocolRoot
#define IID_IOInetProtocol       IID_IInternetProtocol    
#define IID_IOInetProtocolSink   IID_IInternetProtocolSink
#define IID_IOInetProtocolInfo   IID_IInternetProtocolInfo
#define IID_IOInetSession        IID_IInternetSession     
#define IID_IOInetPriority       IID_IInternetPriority    
#define IID_IOInetThreadSwitch   IID_IInternetThreadSwitch
#define IID_IOInetProtocolSinkStackable   IID_IInternetProtocolSinkStackable
STDAPI CoInternetParseUrl(               
    LPCWSTR     pwzUrl,                  
    PARSEACTION ParseAction,             
    DWORD       dwFlags,                 
    LPWSTR      pszResult,               
    DWORD       cchResult,               
    DWORD      *pcchResult,              
    DWORD       dwReserved               
    );                                   
STDAPI CoInternetCombineUrl(             
    LPCWSTR     pwzBaseUrl,              
    LPCWSTR     pwzRelativeUrl,          
    DWORD       dwCombineFlags,          
    LPWSTR      pszResult,               
    DWORD       cchResult,               
    DWORD      *pcchResult,              
    DWORD       dwReserved               
    );                                   
STDAPI CoInternetCompareUrl(             
    LPCWSTR pwzUrl1,                     
    LPCWSTR pwzUrl2,                     
    DWORD dwFlags                        
    );                                   
STDAPI CoInternetGetProtocolFlags(       
    LPCWSTR     pwzUrl,                  
    DWORD      *pdwFlags,                
    DWORD       dwReserved               
    );                                   
STDAPI CoInternetQueryInfo(              
    LPCWSTR     pwzUrl,                  
    QUERYOPTION QueryOptions,            
    DWORD       dwQueryFlags,            
    LPVOID      pvBuffer,                
    DWORD       cbBuffer,                
    DWORD      *pcbBuffer,               
    DWORD       dwReserved               
    );                                   
STDAPI CoInternetGetSession(             
    DWORD       dwSessionMode,           
    IInternetSession **ppIInternetSession,
    DWORD       dwReserved               
    );                                   
STDAPI CoInternetGetSecurityUrl(         
    LPCWSTR pwzUrl,                      
    LPWSTR  *ppwzSecUrl,                 
    PSUACTION  psuAction,                
    DWORD   dwReserved                   
    );                                   
 
STDAPI CopyStgMedium(const STGMEDIUM * pcstgmedSrc,  
                           STGMEDIUM * pstgmedDest); 
STDAPI CopyBindInfo( const BINDINFO * pcbiSrc,   
                           BINDINFO * pbiDest ); 
STDAPI_(void) ReleaseBindInfo( BINDINFO * pbindinfo );  
 
#define INET_E_USE_DEFAULT_PROTOCOLHANDLER _HRESULT_TYPEDEF_(0x800C0011L)      
#define INET_E_USE_DEFAULT_SETTING         _HRESULT_TYPEDEF_(0x800C0012L)      
#define INET_E_DEFAULT_ACTION              INET_E_USE_DEFAULT_PROTOCOLHANDLER  
#define INET_E_QUERYOPTION_UNKNOWN         _HRESULT_TYPEDEF_(0x800C0013L)      
#define INET_E_REDIRECTING                 _HRESULT_TYPEDEF_(0x800C0014L)      
#define OInetParseUrl               CoInternetParseUrl               
#define OInetCombineUrl             CoInternetCombineUrl             
#define OInetCompareUrl             CoInternetCompareUrl             
#define OInetQueryInfo              CoInternetQueryInfo              
#define OInetGetSession             CoInternetGetSession             
#endif // !_URLMON_NO_ASYNC_PLUGABLE_PROTOCOLS_ 
//
// Static Protocol flags
//
#define PROTOCOLFLAG_NO_PICS_CHECK     0x00000001

// Creates the security manager object. The first argument is the Service provider
// to allow for delegation
STDAPI CoInternetCreateSecurityManager(IServiceProvider *pSP, IInternetSecurityManager **ppSM, DWORD dwReserved);

STDAPI CoInternetCreateZoneManager(IServiceProvider *pSP, IInternetZoneManager **ppZM, DWORD dwReserved);


// Security manager CLSID's
EXTERN_C const IID CLSID_InternetSecurityManager;  
EXTERN_C const IID CLSID_InternetZoneManager;  
// This service is used for delegation support on the Security Manager interface
#define SID_SInternetSecurityManager         IID_IInternetSecurityManager

#define SID_SInternetHostSecurityManager     IID_IInternetHostSecurityManager

#ifndef _LPINTERNETSECURITYMGRSITE_DEFINED
#define _LPINTERNETSECURITYMGRSITE_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0190_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0190_v0_0_s_ifspec;

#ifndef __IInternetSecurityMgrSite_INTERFACE_DEFINED__
#define __IInternetSecurityMgrSite_INTERFACE_DEFINED__

/* interface IInternetSecurityMgrSite */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IInternetSecurityMgrSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9ed-baf9-11ce-8c82-00aa004ba90b")
    IInternetSecurityMgrSite : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetWindow( 
            /* [out] */ HWND __RPC_FAR *phwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnableModeless( 
            /* [in] */ BOOL fEnable) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IInternetSecurityMgrSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInternetSecurityMgrSite __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInternetSecurityMgrSite __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInternetSecurityMgrSite __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetWindow )( 
            IInternetSecurityMgrSite __RPC_FAR * This,
            /* [out] */ HWND __RPC_FAR *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnableModeless )( 
            IInternetSecurityMgrSite __RPC_FAR * This,
            /* [in] */ BOOL fEnable);
        
        END_INTERFACE
    } IInternetSecurityMgrSiteVtbl;

    interface IInternetSecurityMgrSite
    {
        CONST_VTBL struct IInternetSecurityMgrSiteVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternetSecurityMgrSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternetSecurityMgrSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternetSecurityMgrSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternetSecurityMgrSite_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IInternetSecurityMgrSite_EnableModeless(This,fEnable)	\
    (This)->lpVtbl -> EnableModeless(This,fEnable)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IInternetSecurityMgrSite_GetWindow_Proxy( 
    IInternetSecurityMgrSite __RPC_FAR * This,
    /* [out] */ HWND __RPC_FAR *phwnd);


void __RPC_STUB IInternetSecurityMgrSite_GetWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetSecurityMgrSite_EnableModeless_Proxy( 
    IInternetSecurityMgrSite __RPC_FAR * This,
    /* [in] */ BOOL fEnable);


void __RPC_STUB IInternetSecurityMgrSite_EnableModeless_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IInternetSecurityMgrSite_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0191 */
/* [local] */ 

#endif
#ifndef _LPINTERNETSECURITYMANANGER_DEFINED
#define _LPINTERNETSECURITYMANANGER_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0191_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0191_v0_0_s_ifspec;

#ifndef __IInternetSecurityManager_INTERFACE_DEFINED__
#define __IInternetSecurityManager_INTERFACE_DEFINED__

/* interface IInternetSecurityManager */
/* [object][unique][helpstring][uuid] */ 

#define MUTZ_NOSAVEDFILECHECK 0x00000001 // don't check file: for saved file comment
// MapUrlToZone returns the zone index given a URL
#define MAX_SIZE_SECURITY_ID 512 // bytes
typedef /* [public] */ 
enum __MIDL_IInternetSecurityManager_0001
    {	PUAF_DEFAULT	= 0,
	PUAF_NOUI	= 0x1,
	PUAF_ISFILE	= 0x2,
	PUAF_WARN_IF_DENIED	= 0x4,
	PUAF_FORCEUI_FOREGROUND	= 0x8,
	PUAF_CHECK_TIFS	= 0x10,
	PUAF_DONTCHECKBOXINDIALOG	= 0x20,
	PUAF_TRUSTED	= 0x40,
	PUAF_ACCEPT_WILDCARD_SCHEME	= 0x80,
	PUAF_ENFORCERESTRICTED	= 0x100
    }	PUAF;

// This is the wrapper function that most clients will use.
// It figures out the current Policy for the passed in Action,
// and puts up UI if the current Policy indicates that the user
// should be queried. It returns back the Policy which the caller
// will use to determine if the action should be allowed
// This is the wrapper function to conveniently read a custom policy.
typedef /* [public] */ 
enum __MIDL_IInternetSecurityManager_0002
    {	SZM_CREATE	= 0,
	SZM_DELETE	= 0x1
    }	SZM_FLAGS;

// SetZoneMapping
//    lpszPattern: string denoting a URL pattern
//        Examples of valid patterns:   
//            *://*.msn.com             
//            http://*.sony.co.jp       
//            *://et.msn.com            
//            ftp://157.54.23.41/       
//            https://localsvr          
//            file:\localsvr\share     
//            *://157.54.100-200.*      
//        Examples of invalid patterns: 
//            http://*.lcs.mit.edu      
//            ftp://*                   
//    dwFlags: SZM_FLAGS values         

EXTERN_C const IID IID_IInternetSecurityManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9ee-baf9-11ce-8c82-00aa004ba90b")
    IInternetSecurityManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetSecuritySite( 
            /* [unique][in] */ IInternetSecurityMgrSite __RPC_FAR *pSite) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSecuritySite( 
            /* [out] */ IInternetSecurityMgrSite __RPC_FAR *__RPC_FAR *ppSite) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MapUrlToZone( 
            /* [in] */ LPCWSTR pwszUrl,
            /* [out] */ DWORD __RPC_FAR *pdwZone,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSecurityId( 
            /* [in] */ LPCWSTR pwszUrl,
            /* [size_is][out] */ BYTE __RPC_FAR *pbSecurityId,
            /* [out][in] */ DWORD __RPC_FAR *pcbSecurityId,
            /* [in] */ DWORD_PTR dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ProcessUrlAction( 
            /* [in] */ LPCWSTR pwszUrl,
            /* [in] */ DWORD dwAction,
            /* [size_is][out] */ BYTE __RPC_FAR *pPolicy,
            /* [in] */ DWORD cbPolicy,
            /* [in] */ BYTE __RPC_FAR *pContext,
            /* [in] */ DWORD cbContext,
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryCustomPolicy( 
            /* [in] */ LPCWSTR pwszUrl,
            /* [in] */ REFGUID guidKey,
            /* [size_is][size_is][out] */ BYTE __RPC_FAR *__RPC_FAR *ppPolicy,
            /* [out] */ DWORD __RPC_FAR *pcbPolicy,
            /* [in] */ BYTE __RPC_FAR *pContext,
            /* [in] */ DWORD cbContext,
            /* [in] */ DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetZoneMapping( 
            /* [in] */ DWORD dwZone,
            /* [in] */ LPCWSTR lpszPattern,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetZoneMappings( 
            /* [in] */ DWORD dwZone,
            /* [out] */ IEnumString __RPC_FAR *__RPC_FAR *ppenumString,
            /* [in] */ DWORD dwFlags) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IInternetSecurityManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInternetSecurityManager __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInternetSecurityManager __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInternetSecurityManager __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSecuritySite )( 
            IInternetSecurityManager __RPC_FAR * This,
            /* [unique][in] */ IInternetSecurityMgrSite __RPC_FAR *pSite);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSecuritySite )( 
            IInternetSecurityManager __RPC_FAR * This,
            /* [out] */ IInternetSecurityMgrSite __RPC_FAR *__RPC_FAR *ppSite);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MapUrlToZone )( 
            IInternetSecurityManager __RPC_FAR * This,
            /* [in] */ LPCWSTR pwszUrl,
            /* [out] */ DWORD __RPC_FAR *pdwZone,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSecurityId )( 
            IInternetSecurityManager __RPC_FAR * This,
            /* [in] */ LPCWSTR pwszUrl,
            /* [size_is][out] */ BYTE __RPC_FAR *pbSecurityId,
            /* [out][in] */ DWORD __RPC_FAR *pcbSecurityId,
            /* [in] */ DWORD_PTR dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ProcessUrlAction )( 
            IInternetSecurityManager __RPC_FAR * This,
            /* [in] */ LPCWSTR pwszUrl,
            /* [in] */ DWORD dwAction,
            /* [size_is][out] */ BYTE __RPC_FAR *pPolicy,
            /* [in] */ DWORD cbPolicy,
            /* [in] */ BYTE __RPC_FAR *pContext,
            /* [in] */ DWORD cbContext,
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryCustomPolicy )( 
            IInternetSecurityManager __RPC_FAR * This,
            /* [in] */ LPCWSTR pwszUrl,
            /* [in] */ REFGUID guidKey,
            /* [size_is][size_is][out] */ BYTE __RPC_FAR *__RPC_FAR *ppPolicy,
            /* [out] */ DWORD __RPC_FAR *pcbPolicy,
            /* [in] */ BYTE __RPC_FAR *pContext,
            /* [in] */ DWORD cbContext,
            /* [in] */ DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetZoneMapping )( 
            IInternetSecurityManager __RPC_FAR * This,
            /* [in] */ DWORD dwZone,
            /* [in] */ LPCWSTR lpszPattern,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetZoneMappings )( 
            IInternetSecurityManager __RPC_FAR * This,
            /* [in] */ DWORD dwZone,
            /* [out] */ IEnumString __RPC_FAR *__RPC_FAR *ppenumString,
            /* [in] */ DWORD dwFlags);
        
        END_INTERFACE
    } IInternetSecurityManagerVtbl;

    interface IInternetSecurityManager
    {
        CONST_VTBL struct IInternetSecurityManagerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternetSecurityManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternetSecurityManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternetSecurityManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternetSecurityManager_SetSecuritySite(This,pSite)	\
    (This)->lpVtbl -> SetSecuritySite(This,pSite)

#define IInternetSecurityManager_GetSecuritySite(This,ppSite)	\
    (This)->lpVtbl -> GetSecuritySite(This,ppSite)

#define IInternetSecurityManager_MapUrlToZone(This,pwszUrl,pdwZone,dwFlags)	\
    (This)->lpVtbl -> MapUrlToZone(This,pwszUrl,pdwZone,dwFlags)

#define IInternetSecurityManager_GetSecurityId(This,pwszUrl,pbSecurityId,pcbSecurityId,dwReserved)	\
    (This)->lpVtbl -> GetSecurityId(This,pwszUrl,pbSecurityId,pcbSecurityId,dwReserved)

#define IInternetSecurityManager_ProcessUrlAction(This,pwszUrl,dwAction,pPolicy,cbPolicy,pContext,cbContext,dwFlags,dwReserved)	\
    (This)->lpVtbl -> ProcessUrlAction(This,pwszUrl,dwAction,pPolicy,cbPolicy,pContext,cbContext,dwFlags,dwReserved)

#define IInternetSecurityManager_QueryCustomPolicy(This,pwszUrl,guidKey,ppPolicy,pcbPolicy,pContext,cbContext,dwReserved)	\
    (This)->lpVtbl -> QueryCustomPolicy(This,pwszUrl,guidKey,ppPolicy,pcbPolicy,pContext,cbContext,dwReserved)

#define IInternetSecurityManager_SetZoneMapping(This,dwZone,lpszPattern,dwFlags)	\
    (This)->lpVtbl -> SetZoneMapping(This,dwZone,lpszPattern,dwFlags)

#define IInternetSecurityManager_GetZoneMappings(This,dwZone,ppenumString,dwFlags)	\
    (This)->lpVtbl -> GetZoneMappings(This,dwZone,ppenumString,dwFlags)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IInternetSecurityManager_SetSecuritySite_Proxy( 
    IInternetSecurityManager __RPC_FAR * This,
    /* [unique][in] */ IInternetSecurityMgrSite __RPC_FAR *pSite);


void __RPC_STUB IInternetSecurityManager_SetSecuritySite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetSecurityManager_GetSecuritySite_Proxy( 
    IInternetSecurityManager __RPC_FAR * This,
    /* [out] */ IInternetSecurityMgrSite __RPC_FAR *__RPC_FAR *ppSite);


void __RPC_STUB IInternetSecurityManager_GetSecuritySite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetSecurityManager_MapUrlToZone_Proxy( 
    IInternetSecurityManager __RPC_FAR * This,
    /* [in] */ LPCWSTR pwszUrl,
    /* [out] */ DWORD __RPC_FAR *pdwZone,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IInternetSecurityManager_MapUrlToZone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetSecurityManager_GetSecurityId_Proxy( 
    IInternetSecurityManager __RPC_FAR * This,
    /* [in] */ LPCWSTR pwszUrl,
    /* [size_is][out] */ BYTE __RPC_FAR *pbSecurityId,
    /* [out][in] */ DWORD __RPC_FAR *pcbSecurityId,
    /* [in] */ DWORD_PTR dwReserved);


void __RPC_STUB IInternetSecurityManager_GetSecurityId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetSecurityManager_ProcessUrlAction_Proxy( 
    IInternetSecurityManager __RPC_FAR * This,
    /* [in] */ LPCWSTR pwszUrl,
    /* [in] */ DWORD dwAction,
    /* [size_is][out] */ BYTE __RPC_FAR *pPolicy,
    /* [in] */ DWORD cbPolicy,
    /* [in] */ BYTE __RPC_FAR *pContext,
    /* [in] */ DWORD cbContext,
    /* [in] */ DWORD dwFlags,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IInternetSecurityManager_ProcessUrlAction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetSecurityManager_QueryCustomPolicy_Proxy( 
    IInternetSecurityManager __RPC_FAR * This,
    /* [in] */ LPCWSTR pwszUrl,
    /* [in] */ REFGUID guidKey,
    /* [size_is][size_is][out] */ BYTE __RPC_FAR *__RPC_FAR *ppPolicy,
    /* [out] */ DWORD __RPC_FAR *pcbPolicy,
    /* [in] */ BYTE __RPC_FAR *pContext,
    /* [in] */ DWORD cbContext,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IInternetSecurityManager_QueryCustomPolicy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetSecurityManager_SetZoneMapping_Proxy( 
    IInternetSecurityManager __RPC_FAR * This,
    /* [in] */ DWORD dwZone,
    /* [in] */ LPCWSTR lpszPattern,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IInternetSecurityManager_SetZoneMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetSecurityManager_GetZoneMappings_Proxy( 
    IInternetSecurityManager __RPC_FAR * This,
    /* [in] */ DWORD dwZone,
    /* [out] */ IEnumString __RPC_FAR *__RPC_FAR *ppenumString,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IInternetSecurityManager_GetZoneMappings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IInternetSecurityManager_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0192 */
/* [local] */ 

#endif
#ifndef _LPINTERNETHOSTSECURITYMANANGER_DEFINED
#define _LPINTERNETHOSTSECURITYMANANGER_DEFINED
//This is the interface MSHTML exposes to its clients
//The clients need not pass in a URL to these functions
//since MSHTML maintains the notion of the current URL


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0192_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0192_v0_0_s_ifspec;

#ifndef __IInternetHostSecurityManager_INTERFACE_DEFINED__
#define __IInternetHostSecurityManager_INTERFACE_DEFINED__

/* interface IInternetHostSecurityManager */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IInternetHostSecurityManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3af280b6-cb3f-11d0-891e-00c04fb6bfc4")
    IInternetHostSecurityManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSecurityId( 
            /* [size_is][out] */ BYTE __RPC_FAR *pbSecurityId,
            /* [out][in] */ DWORD __RPC_FAR *pcbSecurityId,
            /* [in] */ DWORD_PTR dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ProcessUrlAction( 
            /* [in] */ DWORD dwAction,
            /* [size_is][out] */ BYTE __RPC_FAR *pPolicy,
            /* [in] */ DWORD cbPolicy,
            /* [in] */ BYTE __RPC_FAR *pContext,
            /* [in] */ DWORD cbContext,
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryCustomPolicy( 
            /* [in] */ REFGUID guidKey,
            /* [size_is][size_is][out] */ BYTE __RPC_FAR *__RPC_FAR *ppPolicy,
            /* [out] */ DWORD __RPC_FAR *pcbPolicy,
            /* [in] */ BYTE __RPC_FAR *pContext,
            /* [in] */ DWORD cbContext,
            /* [in] */ DWORD dwReserved) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IInternetHostSecurityManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInternetHostSecurityManager __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInternetHostSecurityManager __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInternetHostSecurityManager __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSecurityId )( 
            IInternetHostSecurityManager __RPC_FAR * This,
            /* [size_is][out] */ BYTE __RPC_FAR *pbSecurityId,
            /* [out][in] */ DWORD __RPC_FAR *pcbSecurityId,
            /* [in] */ DWORD_PTR dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ProcessUrlAction )( 
            IInternetHostSecurityManager __RPC_FAR * This,
            /* [in] */ DWORD dwAction,
            /* [size_is][out] */ BYTE __RPC_FAR *pPolicy,
            /* [in] */ DWORD cbPolicy,
            /* [in] */ BYTE __RPC_FAR *pContext,
            /* [in] */ DWORD cbContext,
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryCustomPolicy )( 
            IInternetHostSecurityManager __RPC_FAR * This,
            /* [in] */ REFGUID guidKey,
            /* [size_is][size_is][out] */ BYTE __RPC_FAR *__RPC_FAR *ppPolicy,
            /* [out] */ DWORD __RPC_FAR *pcbPolicy,
            /* [in] */ BYTE __RPC_FAR *pContext,
            /* [in] */ DWORD cbContext,
            /* [in] */ DWORD dwReserved);
        
        END_INTERFACE
    } IInternetHostSecurityManagerVtbl;

    interface IInternetHostSecurityManager
    {
        CONST_VTBL struct IInternetHostSecurityManagerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternetHostSecurityManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternetHostSecurityManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternetHostSecurityManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternetHostSecurityManager_GetSecurityId(This,pbSecurityId,pcbSecurityId,dwReserved)	\
    (This)->lpVtbl -> GetSecurityId(This,pbSecurityId,pcbSecurityId,dwReserved)

#define IInternetHostSecurityManager_ProcessUrlAction(This,dwAction,pPolicy,cbPolicy,pContext,cbContext,dwFlags,dwReserved)	\
    (This)->lpVtbl -> ProcessUrlAction(This,dwAction,pPolicy,cbPolicy,pContext,cbContext,dwFlags,dwReserved)

#define IInternetHostSecurityManager_QueryCustomPolicy(This,guidKey,ppPolicy,pcbPolicy,pContext,cbContext,dwReserved)	\
    (This)->lpVtbl -> QueryCustomPolicy(This,guidKey,ppPolicy,pcbPolicy,pContext,cbContext,dwReserved)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IInternetHostSecurityManager_GetSecurityId_Proxy( 
    IInternetHostSecurityManager __RPC_FAR * This,
    /* [size_is][out] */ BYTE __RPC_FAR *pbSecurityId,
    /* [out][in] */ DWORD __RPC_FAR *pcbSecurityId,
    /* [in] */ DWORD_PTR dwReserved);


void __RPC_STUB IInternetHostSecurityManager_GetSecurityId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetHostSecurityManager_ProcessUrlAction_Proxy( 
    IInternetHostSecurityManager __RPC_FAR * This,
    /* [in] */ DWORD dwAction,
    /* [size_is][out] */ BYTE __RPC_FAR *pPolicy,
    /* [in] */ DWORD cbPolicy,
    /* [in] */ BYTE __RPC_FAR *pContext,
    /* [in] */ DWORD cbContext,
    /* [in] */ DWORD dwFlags,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IInternetHostSecurityManager_ProcessUrlAction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetHostSecurityManager_QueryCustomPolicy_Proxy( 
    IInternetHostSecurityManager __RPC_FAR * This,
    /* [in] */ REFGUID guidKey,
    /* [size_is][size_is][out] */ BYTE __RPC_FAR *__RPC_FAR *ppPolicy,
    /* [out] */ DWORD __RPC_FAR *pcbPolicy,
    /* [in] */ BYTE __RPC_FAR *pContext,
    /* [in] */ DWORD cbContext,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IInternetHostSecurityManager_QueryCustomPolicy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IInternetHostSecurityManager_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0193 */
/* [local] */ 

#endif

// The zone manager maintains policies for a set of standard actions. 
// These actions are identified by integral values (called action indexes)
// specified below.

// Minimum legal value for an action    
#define URLACTION_MIN                                          0x00001000

#define URLACTION_DOWNLOAD_MIN                                 0x00001000
#define URLACTION_DOWNLOAD_SIGNED_ACTIVEX                      0x00001001
#define URLACTION_DOWNLOAD_UNSIGNED_ACTIVEX                    0x00001004
#define URLACTION_DOWNLOAD_CURR_MAX                            0x00001004
#define URLACTION_DOWNLOAD_MAX                                 0x000011FF

#define URLACTION_ACTIVEX_MIN                                  0x00001200
#define URLACTION_ACTIVEX_RUN                                  0x00001200
#define URLPOLICY_ACTIVEX_CHECK_LIST                           0x00010000
#define URLACTION_ACTIVEX_OVERRIDE_OBJECT_SAFETY               0x00001201 // aggregate next four
#define URLACTION_ACTIVEX_OVERRIDE_DATA_SAFETY                 0x00001202 //
#define URLACTION_ACTIVEX_OVERRIDE_SCRIPT_SAFETY               0x00001203 //
#define URLACTION_SCRIPT_OVERRIDE_SAFETY                       0x00001401 //
#define URLACTION_ACTIVEX_CONFIRM_NOOBJECTSAFETY               0x00001204 //
#define URLACTION_ACTIVEX_TREATASUNTRUSTED                     0x00001205
#define URLACTION_ACTIVEX_NO_WEBOC_SCRIPT                      0x00001206
#define URLACTION_ACTIVEX_CURR_MAX                             0x00001206
#define URLACTION_ACTIVEX_MAX                                  0x000013ff

#define URLACTION_SCRIPT_MIN                                   0x00001400
#define URLACTION_SCRIPT_RUN                                   0x00001400
#define URLACTION_SCRIPT_JAVA_USE                              0x00001402
#define URLACTION_SCRIPT_SAFE_ACTIVEX                          0x00001405
#define URLACTION_CROSS_DOMAIN_DATA                            0x00001406
#define URLACTION_SCRIPT_PASTE                                 0x00001407
#define URLACTION_SCRIPT_CURR_MAX                              0x00001407
#define URLACTION_SCRIPT_MAX                                   0x000015ff

#define URLACTION_HTML_MIN                                     0x00001600
#define URLACTION_HTML_SUBMIT_FORMS                            0x00001601 // aggregate next two
#define URLACTION_HTML_SUBMIT_FORMS_FROM                       0x00001602 //
#define URLACTION_HTML_SUBMIT_FORMS_TO                         0x00001603 //
#define URLACTION_HTML_FONT_DOWNLOAD                           0x00001604
#define URLACTION_HTML_JAVA_RUN                                0x00001605 // derive from Java custom policy
#define URLACTION_HTML_USERDATA_SAVE                           0x00001606
#define URLACTION_HTML_SUBFRAME_NAVIGATE                       0x00001607
#define URLACTION_HTML_CURR_MAX                                0x00001607
#define URLACTION_HTML_MAX                                     0x000017ff

#define URLACTION_SHELL_MIN                                    0x00001800
#define URLACTION_SHELL_INSTALL_DTITEMS                        0x00001800
#define URLACTION_SHELL_MOVE_OR_COPY                           0x00001802
#define URLACTION_SHELL_FILE_DOWNLOAD                          0x00001803
#define URLACTION_SHELL_VERB                                   0x00001804
#define URLACTION_SHELL_WEBVIEW_VERB                           0x00001805
#define URLACTION_SHELL_CURR_MAX                               0x00001805
#define URLACTION_SHELL_MAX                                    0x000019ff

#define URLACTION_NETWORK_MIN                                  0x00001A00

#define URLACTION_CREDENTIALS_USE                              0x00001A00
#define URLPOLICY_CREDENTIALS_SILENT_LOGON_OK        0x00000000
#define URLPOLICY_CREDENTIALS_MUST_PROMPT_USER       0x00010000
#define URLPOLICY_CREDENTIALS_CONDITIONAL_PROMPT     0x00020000
#define URLPOLICY_CREDENTIALS_ANONYMOUS_ONLY         0x00030000

#define URLACTION_AUTHENTICATE_CLIENT                          0x00001A01
#define URLPOLICY_AUTHENTICATE_CLEARTEXT_OK          0x00000000
#define URLPOLICY_AUTHENTICATE_CHALLENGE_RESPONSE    0x00010000
#define URLPOLICY_AUTHENTICATE_MUTUAL_ONLY           0x00030000


#define URLACTION_COOKIES                                      0x00001A02
#define URLACTION_COOKIES_SESSION                              0x00001A03

#define URLACTION_NETWORK_CURR_MAX                             0x00001A03
#define URLACTION_NETWORK_MAX                                  0x00001Bff


#define URLACTION_JAVA_MIN                                     0x00001C00
#define URLACTION_JAVA_PERMISSIONS                             0x00001C00
#define URLPOLICY_JAVA_PROHIBIT                      0x00000000
#define URLPOLICY_JAVA_HIGH                          0x00010000
#define URLPOLICY_JAVA_MEDIUM                        0x00020000
#define URLPOLICY_JAVA_LOW                           0x00030000
#define URLPOLICY_JAVA_CUSTOM                        0x00800000
#define URLACTION_JAVA_CURR_MAX                                0x00001C00
#define URLACTION_JAVA_MAX                                     0x00001Cff


// The following Infodelivery actions should have no default policies
// in the registry.  They assume that no default policy means fall
// back to the global restriction.  If an admin sets a policy per
// zone, then it overrides the global restriction.

#define URLACTION_INFODELIVERY_MIN                           0x00001D00
#define URLACTION_INFODELIVERY_NO_ADDING_CHANNELS            0x00001D00
#define URLACTION_INFODELIVERY_NO_EDITING_CHANNELS           0x00001D01
#define URLACTION_INFODELIVERY_NO_REMOVING_CHANNELS          0x00001D02
#define URLACTION_INFODELIVERY_NO_ADDING_SUBSCRIPTIONS       0x00001D03
#define URLACTION_INFODELIVERY_NO_EDITING_SUBSCRIPTIONS      0x00001D04
#define URLACTION_INFODELIVERY_NO_REMOVING_SUBSCRIPTIONS     0x00001D05
#define URLACTION_INFODELIVERY_NO_CHANNEL_LOGGING            0x00001D06
#define URLACTION_INFODELIVERY_CURR_MAX                      0x00001D06
#define URLACTION_INFODELIVERY_MAX                           0x00001Dff
#define URLACTION_CHANNEL_SOFTDIST_MIN                       0x00001E00
#define URLACTION_CHANNEL_SOFTDIST_PERMISSIONS               0x00001E05
#define URLPOLICY_CHANNEL_SOFTDIST_PROHIBIT          0x00010000
#define URLPOLICY_CHANNEL_SOFTDIST_PRECACHE          0x00020000
#define URLPOLICY_CHANNEL_SOFTDIST_AUTOINSTALL       0x00030000
#define URLACTION_CHANNEL_SOFTDIST_MAX                       0x00001Eff

// For each action specified above the system maintains
// a set of policies for the action. 
// The only policies supported currently are permissions (i.e. is something allowed)
// and logging status. 
// IMPORTANT: If you are defining your own policies don't overload the meaning of the
// loword of the policy. You can use the hiword to store any policy bits which are only
// meaningful to your action.
// For an example of how to do this look at the URLPOLICY_JAVA above

// Permissions 
#define URLPOLICY_ALLOW                0x00
#define URLPOLICY_QUERY                0x01
#define URLPOLICY_DISALLOW             0x03

// Notifications are not done when user already queried.
#define URLPOLICY_NOTIFY_ON_ALLOW      0x10
#define URLPOLICY_NOTIFY_ON_DISALLOW   0x20

// Logging is done regardless of whether user was queried.
#define URLPOLICY_LOG_ON_ALLOW         0x40
#define URLPOLICY_LOG_ON_DISALLOW      0x80

#define URLPOLICY_MASK_PERMISSIONS     0x0f
#define GetUrlPolicyPermissions(dw)        (dw & URLPOLICY_MASK_PERMISSIONS)
#define SetUrlPolicyPermissions(dw,dw2)    ((dw) = ((dw) & ~(URLPOLICY_MASK_PERMISSIONS)) | (dw2))


#define URLPOLICY_DONTCHECKDLGBOX     0x100
// The ordinal #'s that define the predefined zones internet explorer knows about. 
// When we support user-defined zones their zone numbers should be between 
// URLZONE_USER_MIN and URLZONE_USER_MAX
#ifndef _LPINTERNETZONEMANAGER_DEFINED
#define _LPINTERNETZONEMANAGER_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0193_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0193_v0_0_s_ifspec;

#ifndef __IInternetZoneManager_INTERFACE_DEFINED__
#define __IInternetZoneManager_INTERFACE_DEFINED__

/* interface IInternetZoneManager */
/* [unique][helpstring][uuid][object][local] */ 

typedef /* [unique] */ IInternetZoneManager __RPC_FAR *LPURLZONEMANAGER;

typedef 
enum tagURLZONE
    {	URLZONE_PREDEFINED_MIN	= 0,
	URLZONE_LOCAL_MACHINE	= 0,
	URLZONE_INTRANET	= URLZONE_LOCAL_MACHINE + 1,
	URLZONE_TRUSTED	= URLZONE_INTRANET + 1,
	URLZONE_INTERNET	= URLZONE_TRUSTED + 1,
	URLZONE_UNTRUSTED	= URLZONE_INTERNET + 1,
	URLZONE_PREDEFINED_MAX	= 999,
	URLZONE_USER_MIN	= 1000,
	URLZONE_USER_MAX	= 10000
    }	URLZONE;

typedef 
enum tagURLTEMPLATE
    {	URLTEMPLATE_CUSTOM	= 0,
	URLTEMPLATE_PREDEFINED_MIN	= 0x10000,
	URLTEMPLATE_LOW	= 0x10000,
	URLTEMPLATE_MEDLOW	= 0x10500,
	URLTEMPLATE_MEDIUM	= 0x11000,
	URLTEMPLATE_HIGH	= 0x12000,
	URLTEMPLATE_PREDEFINED_MAX	= 0x20000
    }	URLTEMPLATE;


enum __MIDL_IInternetZoneManager_0001
    {	MAX_ZONE_PATH	= 260,
	MAX_ZONE_DESCRIPTION	= 200
    };
typedef /* [public] */ 
enum __MIDL_IInternetZoneManager_0002
    {	ZAFLAGS_CUSTOM_EDIT	= 0x1,
	ZAFLAGS_ADD_SITES	= 0x2,
	ZAFLAGS_REQUIRE_VERIFICATION	= 0x4,
	ZAFLAGS_INCLUDE_PROXY_OVERRIDE	= 0x8,
	ZAFLAGS_INCLUDE_INTRANET_SITES	= 0x10,
	ZAFLAGS_NO_UI	= 0x20,
	ZAFLAGS_SUPPORTS_VERIFICATION	= 0x40,
	ZAFLAGS_UNC_AS_INTRANET	= 0x80
    }	ZAFLAGS;

typedef struct _ZONEATTRIBUTES
    {
    ULONG cbSize;
    WCHAR szDisplayName[ 260 ];
    WCHAR szDescription[ 200 ];
    WCHAR szIconPath[ 260 ];
    DWORD dwTemplateMinLevel;
    DWORD dwTemplateRecommended;
    DWORD dwTemplateCurrentLevel;
    DWORD dwFlags;
    }	ZONEATTRIBUTES;

typedef struct _ZONEATTRIBUTES __RPC_FAR *LPZONEATTRIBUTES;

// Gets the zone attributes (information in registry other than actual security
// policies associated with the zone).  Zone attributes are fixed as:
// Sets the zone attributes (information in registry other than actual security
// policies associated with the zone).  Zone attributes as above.
// Returns S_OK or ??? if failed to write the zone attributes.
/* Registry Flags

    When reading, default behavior is:
        If HKLM allows override and HKCU value exists
            Then use HKCU value
            Else use HKLM value
    When writing, default behavior is same as HKCU
        If HKLM allows override
           Then Write to HKCU
           Else Fail
*/
typedef 
enum _URLZONEREG
    {	URLZONEREG_DEFAULT	= 0,
	URLZONEREG_HKLM	= URLZONEREG_DEFAULT + 1,
	URLZONEREG_HKCU	= URLZONEREG_HKLM + 1
    }	URLZONEREG;

// Gets a named custom policy associated with a zone;
// e.g. the Java VM settings can be defined with a unique key such as 'Java'.
// Custom policy support is intended to allow extensibility from the predefined
// set of policies that IE4 has built in.
// 
// pwszKey is the string name designating the custom policy.  Components are
//   responsible for having unique names.
// ppPolicy is the callee allocated buffer for the policy byte blob; caller is
//   responsible for freeing this buffer eventually.
// pcbPolicy is the size of the byte blob returned.
// dwRegFlags determines how registry is accessed (see above).
// Returns S_OK if key is found and buffer allocated; ??? if key is not found (no buffer alloced).
// Sets a named custom policy associated with a zone;
// e.g. the Java VM settings can be defined with a unique key such as 'Java'.
// Custom policy support is intended to allow extensibility from the predefined
// set of policies that IE4 has built in.  
// 
// pwszKey is the string name designating the custom policy.  Components are
//   responsible for having unique names.
// ppPolicy is the caller allocated buffer for the policy byte blob.
// pcbPolicy is the size of the byte blob to be set.
// dwRegFlags determines if HTCU or HKLM is set.
// Returns S_OK or ??? if failed to write the zone custom policy.
// Gets action policy associated with a zone, the builtin, fixed-length policies info.

// dwAction is the action code for the action as defined above.
// pPolicy is the caller allocated buffer for the policy data.
// cbPolicy is the size of the caller allocated buffer.
// dwRegFlags determines how registry is accessed (see above).
// Returns S_OK if action is valid; ??? if action is not valid.

EXTERN_C const IID IID_IInternetZoneManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79eac9ef-baf9-11ce-8c82-00aa004ba90b")
    IInternetZoneManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetZoneAttributes( 
            /* [in] */ DWORD dwZone,
            /* [unique][out][in] */ ZONEATTRIBUTES __RPC_FAR *pZoneAttributes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetZoneAttributes( 
            /* [in] */ DWORD dwZone,
            /* [in] */ ZONEATTRIBUTES __RPC_FAR *pZoneAttributes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetZoneCustomPolicy( 
            /* [in] */ DWORD dwZone,
            /* [in] */ REFGUID guidKey,
            /* [size_is][size_is][out] */ BYTE __RPC_FAR *__RPC_FAR *ppPolicy,
            /* [out] */ DWORD __RPC_FAR *pcbPolicy,
            /* [in] */ URLZONEREG urlZoneReg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetZoneCustomPolicy( 
            /* [in] */ DWORD dwZone,
            /* [in] */ REFGUID guidKey,
            /* [size_is][in] */ BYTE __RPC_FAR *pPolicy,
            /* [in] */ DWORD cbPolicy,
            /* [in] */ URLZONEREG urlZoneReg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetZoneActionPolicy( 
            /* [in] */ DWORD dwZone,
            /* [in] */ DWORD dwAction,
            /* [size_is][out] */ BYTE __RPC_FAR *pPolicy,
            /* [in] */ DWORD cbPolicy,
            /* [in] */ URLZONEREG urlZoneReg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetZoneActionPolicy( 
            /* [in] */ DWORD dwZone,
            /* [in] */ DWORD dwAction,
            /* [size_is][in] */ BYTE __RPC_FAR *pPolicy,
            /* [in] */ DWORD cbPolicy,
            /* [in] */ URLZONEREG urlZoneReg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PromptAction( 
            /* [in] */ DWORD dwAction,
            /* [in] */ HWND hwndParent,
            /* [in] */ LPCWSTR pwszUrl,
            /* [in] */ LPCWSTR pwszText,
            /* [in] */ DWORD dwPromptFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LogAction( 
            /* [in] */ DWORD dwAction,
            /* [in] */ LPCWSTR pwszUrl,
            /* [in] */ LPCWSTR pwszText,
            /* [in] */ DWORD dwLogFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateZoneEnumerator( 
            /* [out] */ DWORD __RPC_FAR *pdwEnum,
            /* [out] */ DWORD __RPC_FAR *pdwCount,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetZoneAt( 
            /* [in] */ DWORD dwEnum,
            /* [in] */ DWORD dwIndex,
            /* [out] */ DWORD __RPC_FAR *pdwZone) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DestroyZoneEnumerator( 
            /* [in] */ DWORD dwEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyTemplatePoliciesToZone( 
            /* [in] */ DWORD dwTemplate,
            /* [in] */ DWORD dwZone,
            /* [in] */ DWORD dwReserved) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IInternetZoneManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInternetZoneManager __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInternetZoneManager __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInternetZoneManager __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetZoneAttributes )( 
            IInternetZoneManager __RPC_FAR * This,
            /* [in] */ DWORD dwZone,
            /* [unique][out][in] */ ZONEATTRIBUTES __RPC_FAR *pZoneAttributes);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetZoneAttributes )( 
            IInternetZoneManager __RPC_FAR * This,
            /* [in] */ DWORD dwZone,
            /* [in] */ ZONEATTRIBUTES __RPC_FAR *pZoneAttributes);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetZoneCustomPolicy )( 
            IInternetZoneManager __RPC_FAR * This,
            /* [in] */ DWORD dwZone,
            /* [in] */ REFGUID guidKey,
            /* [size_is][size_is][out] */ BYTE __RPC_FAR *__RPC_FAR *ppPolicy,
            /* [out] */ DWORD __RPC_FAR *pcbPolicy,
            /* [in] */ URLZONEREG urlZoneReg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetZoneCustomPolicy )( 
            IInternetZoneManager __RPC_FAR * This,
            /* [in] */ DWORD dwZone,
            /* [in] */ REFGUID guidKey,
            /* [size_is][in] */ BYTE __RPC_FAR *pPolicy,
            /* [in] */ DWORD cbPolicy,
            /* [in] */ URLZONEREG urlZoneReg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetZoneActionPolicy )( 
            IInternetZoneManager __RPC_FAR * This,
            /* [in] */ DWORD dwZone,
            /* [in] */ DWORD dwAction,
            /* [size_is][out] */ BYTE __RPC_FAR *pPolicy,
            /* [in] */ DWORD cbPolicy,
            /* [in] */ URLZONEREG urlZoneReg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetZoneActionPolicy )( 
            IInternetZoneManager __RPC_FAR * This,
            /* [in] */ DWORD dwZone,
            /* [in] */ DWORD dwAction,
            /* [size_is][in] */ BYTE __RPC_FAR *pPolicy,
            /* [in] */ DWORD cbPolicy,
            /* [in] */ URLZONEREG urlZoneReg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PromptAction )( 
            IInternetZoneManager __RPC_FAR * This,
            /* [in] */ DWORD dwAction,
            /* [in] */ HWND hwndParent,
            /* [in] */ LPCWSTR pwszUrl,
            /* [in] */ LPCWSTR pwszText,
            /* [in] */ DWORD dwPromptFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LogAction )( 
            IInternetZoneManager __RPC_FAR * This,
            /* [in] */ DWORD dwAction,
            /* [in] */ LPCWSTR pwszUrl,
            /* [in] */ LPCWSTR pwszText,
            /* [in] */ DWORD dwLogFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateZoneEnumerator )( 
            IInternetZoneManager __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwEnum,
            /* [out] */ DWORD __RPC_FAR *pdwCount,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetZoneAt )( 
            IInternetZoneManager __RPC_FAR * This,
            /* [in] */ DWORD dwEnum,
            /* [in] */ DWORD dwIndex,
            /* [out] */ DWORD __RPC_FAR *pdwZone);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DestroyZoneEnumerator )( 
            IInternetZoneManager __RPC_FAR * This,
            /* [in] */ DWORD dwEnum);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CopyTemplatePoliciesToZone )( 
            IInternetZoneManager __RPC_FAR * This,
            /* [in] */ DWORD dwTemplate,
            /* [in] */ DWORD dwZone,
            /* [in] */ DWORD dwReserved);
        
        END_INTERFACE
    } IInternetZoneManagerVtbl;

    interface IInternetZoneManager
    {
        CONST_VTBL struct IInternetZoneManagerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternetZoneManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternetZoneManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternetZoneManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternetZoneManager_GetZoneAttributes(This,dwZone,pZoneAttributes)	\
    (This)->lpVtbl -> GetZoneAttributes(This,dwZone,pZoneAttributes)

#define IInternetZoneManager_SetZoneAttributes(This,dwZone,pZoneAttributes)	\
    (This)->lpVtbl -> SetZoneAttributes(This,dwZone,pZoneAttributes)

#define IInternetZoneManager_GetZoneCustomPolicy(This,dwZone,guidKey,ppPolicy,pcbPolicy,urlZoneReg)	\
    (This)->lpVtbl -> GetZoneCustomPolicy(This,dwZone,guidKey,ppPolicy,pcbPolicy,urlZoneReg)

#define IInternetZoneManager_SetZoneCustomPolicy(This,dwZone,guidKey,pPolicy,cbPolicy,urlZoneReg)	\
    (This)->lpVtbl -> SetZoneCustomPolicy(This,dwZone,guidKey,pPolicy,cbPolicy,urlZoneReg)

#define IInternetZoneManager_GetZoneActionPolicy(This,dwZone,dwAction,pPolicy,cbPolicy,urlZoneReg)	\
    (This)->lpVtbl -> GetZoneActionPolicy(This,dwZone,dwAction,pPolicy,cbPolicy,urlZoneReg)

#define IInternetZoneManager_SetZoneActionPolicy(This,dwZone,dwAction,pPolicy,cbPolicy,urlZoneReg)	\
    (This)->lpVtbl -> SetZoneActionPolicy(This,dwZone,dwAction,pPolicy,cbPolicy,urlZoneReg)

#define IInternetZoneManager_PromptAction(This,dwAction,hwndParent,pwszUrl,pwszText,dwPromptFlags)	\
    (This)->lpVtbl -> PromptAction(This,dwAction,hwndParent,pwszUrl,pwszText,dwPromptFlags)

#define IInternetZoneManager_LogAction(This,dwAction,pwszUrl,pwszText,dwLogFlags)	\
    (This)->lpVtbl -> LogAction(This,dwAction,pwszUrl,pwszText,dwLogFlags)

#define IInternetZoneManager_CreateZoneEnumerator(This,pdwEnum,pdwCount,dwFlags)	\
    (This)->lpVtbl -> CreateZoneEnumerator(This,pdwEnum,pdwCount,dwFlags)

#define IInternetZoneManager_GetZoneAt(This,dwEnum,dwIndex,pdwZone)	\
    (This)->lpVtbl -> GetZoneAt(This,dwEnum,dwIndex,pdwZone)

#define IInternetZoneManager_DestroyZoneEnumerator(This,dwEnum)	\
    (This)->lpVtbl -> DestroyZoneEnumerator(This,dwEnum)

#define IInternetZoneManager_CopyTemplatePoliciesToZone(This,dwTemplate,dwZone,dwReserved)	\
    (This)->lpVtbl -> CopyTemplatePoliciesToZone(This,dwTemplate,dwZone,dwReserved)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IInternetZoneManager_GetZoneAttributes_Proxy( 
    IInternetZoneManager __RPC_FAR * This,
    /* [in] */ DWORD dwZone,
    /* [unique][out][in] */ ZONEATTRIBUTES __RPC_FAR *pZoneAttributes);


void __RPC_STUB IInternetZoneManager_GetZoneAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetZoneManager_SetZoneAttributes_Proxy( 
    IInternetZoneManager __RPC_FAR * This,
    /* [in] */ DWORD dwZone,
    /* [in] */ ZONEATTRIBUTES __RPC_FAR *pZoneAttributes);


void __RPC_STUB IInternetZoneManager_SetZoneAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetZoneManager_GetZoneCustomPolicy_Proxy( 
    IInternetZoneManager __RPC_FAR * This,
    /* [in] */ DWORD dwZone,
    /* [in] */ REFGUID guidKey,
    /* [size_is][size_is][out] */ BYTE __RPC_FAR *__RPC_FAR *ppPolicy,
    /* [out] */ DWORD __RPC_FAR *pcbPolicy,
    /* [in] */ URLZONEREG urlZoneReg);


void __RPC_STUB IInternetZoneManager_GetZoneCustomPolicy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetZoneManager_SetZoneCustomPolicy_Proxy( 
    IInternetZoneManager __RPC_FAR * This,
    /* [in] */ DWORD dwZone,
    /* [in] */ REFGUID guidKey,
    /* [size_is][in] */ BYTE __RPC_FAR *pPolicy,
    /* [in] */ DWORD cbPolicy,
    /* [in] */ URLZONEREG urlZoneReg);


void __RPC_STUB IInternetZoneManager_SetZoneCustomPolicy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetZoneManager_GetZoneActionPolicy_Proxy( 
    IInternetZoneManager __RPC_FAR * This,
    /* [in] */ DWORD dwZone,
    /* [in] */ DWORD dwAction,
    /* [size_is][out] */ BYTE __RPC_FAR *pPolicy,
    /* [in] */ DWORD cbPolicy,
    /* [in] */ URLZONEREG urlZoneReg);


void __RPC_STUB IInternetZoneManager_GetZoneActionPolicy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetZoneManager_SetZoneActionPolicy_Proxy( 
    IInternetZoneManager __RPC_FAR * This,
    /* [in] */ DWORD dwZone,
    /* [in] */ DWORD dwAction,
    /* [size_is][in] */ BYTE __RPC_FAR *pPolicy,
    /* [in] */ DWORD cbPolicy,
    /* [in] */ URLZONEREG urlZoneReg);


void __RPC_STUB IInternetZoneManager_SetZoneActionPolicy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetZoneManager_PromptAction_Proxy( 
    IInternetZoneManager __RPC_FAR * This,
    /* [in] */ DWORD dwAction,
    /* [in] */ HWND hwndParent,
    /* [in] */ LPCWSTR pwszUrl,
    /* [in] */ LPCWSTR pwszText,
    /* [in] */ DWORD dwPromptFlags);


void __RPC_STUB IInternetZoneManager_PromptAction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetZoneManager_LogAction_Proxy( 
    IInternetZoneManager __RPC_FAR * This,
    /* [in] */ DWORD dwAction,
    /* [in] */ LPCWSTR pwszUrl,
    /* [in] */ LPCWSTR pwszText,
    /* [in] */ DWORD dwLogFlags);


void __RPC_STUB IInternetZoneManager_LogAction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetZoneManager_CreateZoneEnumerator_Proxy( 
    IInternetZoneManager __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwEnum,
    /* [out] */ DWORD __RPC_FAR *pdwCount,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IInternetZoneManager_CreateZoneEnumerator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetZoneManager_GetZoneAt_Proxy( 
    IInternetZoneManager __RPC_FAR * This,
    /* [in] */ DWORD dwEnum,
    /* [in] */ DWORD dwIndex,
    /* [out] */ DWORD __RPC_FAR *pdwZone);


void __RPC_STUB IInternetZoneManager_GetZoneAt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetZoneManager_DestroyZoneEnumerator_Proxy( 
    IInternetZoneManager __RPC_FAR * This,
    /* [in] */ DWORD dwEnum);


void __RPC_STUB IInternetZoneManager_DestroyZoneEnumerator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInternetZoneManager_CopyTemplatePoliciesToZone_Proxy( 
    IInternetZoneManager __RPC_FAR * This,
    /* [in] */ DWORD dwTemplate,
    /* [in] */ DWORD dwZone,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IInternetZoneManager_CopyTemplatePoliciesToZone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IInternetZoneManager_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0194 */
/* [local] */ 

#endif
EXTERN_C const IID CLSID_SoftDistExt;  
#ifndef _LPSOFTDISTEXT_DEFINED
#define _LPSOFTDISTEXT_DEFINED

#define SOFTDIST_FLAG_USAGE_EMAIL        0x00000001
#define SOFTDIST_FLAG_USAGE_PRECACHE     0x00000002
#define SOFTDIST_FLAG_USAGE_AUTOINSTALL  0x00000004
#define SOFTDIST_FLAG_DELETE_SUBSCRIPTION 0x00000008


#define SOFTDIST_ADSTATE_NONE                0x00000000
#define SOFTDIST_ADSTATE_AVAILABLE       0x00000001
#define SOFTDIST_ADSTATE_DOWNLOADED      0x00000002
#define SOFTDIST_ADSTATE_INSTALLED           0x00000003

typedef struct _tagCODEBASEHOLD
    {
    ULONG cbSize;
    LPWSTR szDistUnit;
    LPWSTR szCodeBase;
    DWORD dwVersionMS;
    DWORD dwVersionLS;
    DWORD dwStyle;
    }	CODEBASEHOLD;

typedef struct _tagCODEBASEHOLD __RPC_FAR *LPCODEBASEHOLD;

typedef struct _tagSOFTDISTINFO
    {
    ULONG cbSize;
    DWORD dwFlags;
    DWORD dwAdState;
    LPWSTR szTitle;
    LPWSTR szAbstract;
    LPWSTR szHREF;
    DWORD dwInstalledVersionMS;
    DWORD dwInstalledVersionLS;
    DWORD dwUpdateVersionMS;
    DWORD dwUpdateVersionLS;
    DWORD dwAdvertisedVersionMS;
    DWORD dwAdvertisedVersionLS;
    DWORD dwReserved;
    }	SOFTDISTINFO;

typedef struct _tagSOFTDISTINFO __RPC_FAR *LPSOFTDISTINFO;



extern RPC_IF_HANDLE __MIDL_itf_urlmon_0194_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0194_v0_0_s_ifspec;

#ifndef __ISoftDistExt_INTERFACE_DEFINED__
#define __ISoftDistExt_INTERFACE_DEFINED__

/* interface ISoftDistExt */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_ISoftDistExt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B15B8DC1-C7E1-11d0-8680-00AA00BDCB71")
    ISoftDistExt : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ProcessSoftDist( 
            /* [in] */ LPCWSTR szCDFURL,
            /* [in] */ IXMLElement __RPC_FAR *pSoftDistElement,
            /* [out][in] */ LPSOFTDISTINFO lpsdi) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFirstCodeBase( 
            /* [in] */ LPWSTR __RPC_FAR *szCodeBase,
            /* [in] */ LPDWORD dwMaxSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextCodeBase( 
            /* [in] */ LPWSTR __RPC_FAR *szCodeBase,
            /* [in] */ LPDWORD dwMaxSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AsyncInstallDistributionUnit( 
            /* [in] */ IBindCtx __RPC_FAR *pbc,
            /* [in] */ LPVOID pvReserved,
            /* [in] */ DWORD flags,
            /* [in] */ LPCODEBASEHOLD lpcbh) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISoftDistExtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISoftDistExt __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISoftDistExt __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISoftDistExt __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ProcessSoftDist )( 
            ISoftDistExt __RPC_FAR * This,
            /* [in] */ LPCWSTR szCDFURL,
            /* [in] */ IXMLElement __RPC_FAR *pSoftDistElement,
            /* [out][in] */ LPSOFTDISTINFO lpsdi);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFirstCodeBase )( 
            ISoftDistExt __RPC_FAR * This,
            /* [in] */ LPWSTR __RPC_FAR *szCodeBase,
            /* [in] */ LPDWORD dwMaxSize);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNextCodeBase )( 
            ISoftDistExt __RPC_FAR * This,
            /* [in] */ LPWSTR __RPC_FAR *szCodeBase,
            /* [in] */ LPDWORD dwMaxSize);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AsyncInstallDistributionUnit )( 
            ISoftDistExt __RPC_FAR * This,
            /* [in] */ IBindCtx __RPC_FAR *pbc,
            /* [in] */ LPVOID pvReserved,
            /* [in] */ DWORD flags,
            /* [in] */ LPCODEBASEHOLD lpcbh);
        
        END_INTERFACE
    } ISoftDistExtVtbl;

    interface ISoftDistExt
    {
        CONST_VTBL struct ISoftDistExtVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISoftDistExt_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISoftDistExt_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISoftDistExt_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISoftDistExt_ProcessSoftDist(This,szCDFURL,pSoftDistElement,lpsdi)	\
    (This)->lpVtbl -> ProcessSoftDist(This,szCDFURL,pSoftDistElement,lpsdi)

#define ISoftDistExt_GetFirstCodeBase(This,szCodeBase,dwMaxSize)	\
    (This)->lpVtbl -> GetFirstCodeBase(This,szCodeBase,dwMaxSize)

#define ISoftDistExt_GetNextCodeBase(This,szCodeBase,dwMaxSize)	\
    (This)->lpVtbl -> GetNextCodeBase(This,szCodeBase,dwMaxSize)

#define ISoftDistExt_AsyncInstallDistributionUnit(This,pbc,pvReserved,flags,lpcbh)	\
    (This)->lpVtbl -> AsyncInstallDistributionUnit(This,pbc,pvReserved,flags,lpcbh)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ISoftDistExt_ProcessSoftDist_Proxy( 
    ISoftDistExt __RPC_FAR * This,
    /* [in] */ LPCWSTR szCDFURL,
    /* [in] */ IXMLElement __RPC_FAR *pSoftDistElement,
    /* [out][in] */ LPSOFTDISTINFO lpsdi);


void __RPC_STUB ISoftDistExt_ProcessSoftDist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISoftDistExt_GetFirstCodeBase_Proxy( 
    ISoftDistExt __RPC_FAR * This,
    /* [in] */ LPWSTR __RPC_FAR *szCodeBase,
    /* [in] */ LPDWORD dwMaxSize);


void __RPC_STUB ISoftDistExt_GetFirstCodeBase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISoftDistExt_GetNextCodeBase_Proxy( 
    ISoftDistExt __RPC_FAR * This,
    /* [in] */ LPWSTR __RPC_FAR *szCodeBase,
    /* [in] */ LPDWORD dwMaxSize);


void __RPC_STUB ISoftDistExt_GetNextCodeBase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISoftDistExt_AsyncInstallDistributionUnit_Proxy( 
    ISoftDistExt __RPC_FAR * This,
    /* [in] */ IBindCtx __RPC_FAR *pbc,
    /* [in] */ LPVOID pvReserved,
    /* [in] */ DWORD flags,
    /* [in] */ LPCODEBASEHOLD lpcbh);


void __RPC_STUB ISoftDistExt_AsyncInstallDistributionUnit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISoftDistExt_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0195 */
/* [local] */ 

STDAPI GetSoftwareUpdateInfo( LPCWSTR szDistUnit, LPSOFTDISTINFO psdi );
STDAPI SetSoftwareUpdateAdvertisementState( LPCWSTR szDistUnit, DWORD dwAdState, DWORD dwAdvertisedVersionMS, DWORD dwAdvertisedVersionLS );
#endif
#ifndef _LPCATALOGFILEINFO_DEFINED
#define _LPCATALOGFILEINFO_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0195_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0195_v0_0_s_ifspec;

#ifndef __ICatalogFileInfo_INTERFACE_DEFINED__
#define __ICatalogFileInfo_INTERFACE_DEFINED__

/* interface ICatalogFileInfo */
/* [unique][uuid][object][local] */ 

typedef /* [unique] */ ICatalogFileInfo __RPC_FAR *LPCATALOGFILEINFO;


EXTERN_C const IID IID_ICatalogFileInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("711C7600-6B48-11d1-B403-00AA00B92AF1")
    ICatalogFileInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCatalogFile( 
            /* [out] */ LPSTR __RPC_FAR *ppszCatalogFile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetJavaTrust( 
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppJavaTrust) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICatalogFileInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICatalogFileInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICatalogFileInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICatalogFileInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCatalogFile )( 
            ICatalogFileInfo __RPC_FAR * This,
            /* [out] */ LPSTR __RPC_FAR *ppszCatalogFile);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetJavaTrust )( 
            ICatalogFileInfo __RPC_FAR * This,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppJavaTrust);
        
        END_INTERFACE
    } ICatalogFileInfoVtbl;

    interface ICatalogFileInfo
    {
        CONST_VTBL struct ICatalogFileInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICatalogFileInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICatalogFileInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICatalogFileInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICatalogFileInfo_GetCatalogFile(This,ppszCatalogFile)	\
    (This)->lpVtbl -> GetCatalogFile(This,ppszCatalogFile)

#define ICatalogFileInfo_GetJavaTrust(This,ppJavaTrust)	\
    (This)->lpVtbl -> GetJavaTrust(This,ppJavaTrust)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ICatalogFileInfo_GetCatalogFile_Proxy( 
    ICatalogFileInfo __RPC_FAR * This,
    /* [out] */ LPSTR __RPC_FAR *ppszCatalogFile);


void __RPC_STUB ICatalogFileInfo_GetCatalogFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICatalogFileInfo_GetJavaTrust_Proxy( 
    ICatalogFileInfo __RPC_FAR * This,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppJavaTrust);


void __RPC_STUB ICatalogFileInfo_GetJavaTrust_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICatalogFileInfo_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0196 */
/* [local] */ 

#endif
#ifndef _LPDATAFILTER_DEFINED
#define _LPDATAFILTER_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0196_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0196_v0_0_s_ifspec;

#ifndef __IDataFilter_INTERFACE_DEFINED__
#define __IDataFilter_INTERFACE_DEFINED__

/* interface IDataFilter */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IDataFilter __RPC_FAR *LPDATAFILTER;


EXTERN_C const IID IID_IDataFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("69d14c80-c18e-11d0-a9ce-006097942311")
    IDataFilter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DoEncode( 
            /* [in] */ DWORD dwFlags,
            /* [in] */ LONG lInBufferSize,
            /* [size_is][in] */ BYTE __RPC_FAR *pbInBuffer,
            /* [in] */ LONG lOutBufferSize,
            /* [size_is][out] */ BYTE __RPC_FAR *pbOutBuffer,
            /* [in] */ LONG lInBytesAvailable,
            /* [out] */ LONG __RPC_FAR *plInBytesRead,
            /* [out] */ LONG __RPC_FAR *plOutBytesWritten,
            /* [in] */ DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DoDecode( 
            /* [in] */ DWORD dwFlags,
            /* [in] */ LONG lInBufferSize,
            /* [size_is][in] */ BYTE __RPC_FAR *pbInBuffer,
            /* [in] */ LONG lOutBufferSize,
            /* [size_is][out] */ BYTE __RPC_FAR *pbOutBuffer,
            /* [in] */ LONG lInBytesAvailable,
            /* [out] */ LONG __RPC_FAR *plInBytesRead,
            /* [out] */ LONG __RPC_FAR *plOutBytesWritten,
            /* [in] */ DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetEncodingLevel( 
            /* [in] */ DWORD dwEncLevel) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDataFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDataFilter __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDataFilter __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDataFilter __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DoEncode )( 
            IDataFilter __RPC_FAR * This,
            /* [in] */ DWORD dwFlags,
            /* [in] */ LONG lInBufferSize,
            /* [size_is][in] */ BYTE __RPC_FAR *pbInBuffer,
            /* [in] */ LONG lOutBufferSize,
            /* [size_is][out] */ BYTE __RPC_FAR *pbOutBuffer,
            /* [in] */ LONG lInBytesAvailable,
            /* [out] */ LONG __RPC_FAR *plInBytesRead,
            /* [out] */ LONG __RPC_FAR *plOutBytesWritten,
            /* [in] */ DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DoDecode )( 
            IDataFilter __RPC_FAR * This,
            /* [in] */ DWORD dwFlags,
            /* [in] */ LONG lInBufferSize,
            /* [size_is][in] */ BYTE __RPC_FAR *pbInBuffer,
            /* [in] */ LONG lOutBufferSize,
            /* [size_is][out] */ BYTE __RPC_FAR *pbOutBuffer,
            /* [in] */ LONG lInBytesAvailable,
            /* [out] */ LONG __RPC_FAR *plInBytesRead,
            /* [out] */ LONG __RPC_FAR *plOutBytesWritten,
            /* [in] */ DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetEncodingLevel )( 
            IDataFilter __RPC_FAR * This,
            /* [in] */ DWORD dwEncLevel);
        
        END_INTERFACE
    } IDataFilterVtbl;

    interface IDataFilter
    {
        CONST_VTBL struct IDataFilterVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDataFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDataFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDataFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDataFilter_DoEncode(This,dwFlags,lInBufferSize,pbInBuffer,lOutBufferSize,pbOutBuffer,lInBytesAvailable,plInBytesRead,plOutBytesWritten,dwReserved)	\
    (This)->lpVtbl -> DoEncode(This,dwFlags,lInBufferSize,pbInBuffer,lOutBufferSize,pbOutBuffer,lInBytesAvailable,plInBytesRead,plOutBytesWritten,dwReserved)

#define IDataFilter_DoDecode(This,dwFlags,lInBufferSize,pbInBuffer,lOutBufferSize,pbOutBuffer,lInBytesAvailable,plInBytesRead,plOutBytesWritten,dwReserved)	\
    (This)->lpVtbl -> DoDecode(This,dwFlags,lInBufferSize,pbInBuffer,lOutBufferSize,pbOutBuffer,lInBytesAvailable,plInBytesRead,plOutBytesWritten,dwReserved)

#define IDataFilter_SetEncodingLevel(This,dwEncLevel)	\
    (This)->lpVtbl -> SetEncodingLevel(This,dwEncLevel)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDataFilter_DoEncode_Proxy( 
    IDataFilter __RPC_FAR * This,
    /* [in] */ DWORD dwFlags,
    /* [in] */ LONG lInBufferSize,
    /* [size_is][in] */ BYTE __RPC_FAR *pbInBuffer,
    /* [in] */ LONG lOutBufferSize,
    /* [size_is][out] */ BYTE __RPC_FAR *pbOutBuffer,
    /* [in] */ LONG lInBytesAvailable,
    /* [out] */ LONG __RPC_FAR *plInBytesRead,
    /* [out] */ LONG __RPC_FAR *plOutBytesWritten,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IDataFilter_DoEncode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataFilter_DoDecode_Proxy( 
    IDataFilter __RPC_FAR * This,
    /* [in] */ DWORD dwFlags,
    /* [in] */ LONG lInBufferSize,
    /* [size_is][in] */ BYTE __RPC_FAR *pbInBuffer,
    /* [in] */ LONG lOutBufferSize,
    /* [size_is][out] */ BYTE __RPC_FAR *pbOutBuffer,
    /* [in] */ LONG lInBytesAvailable,
    /* [out] */ LONG __RPC_FAR *plInBytesRead,
    /* [out] */ LONG __RPC_FAR *plOutBytesWritten,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IDataFilter_DoDecode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataFilter_SetEncodingLevel_Proxy( 
    IDataFilter __RPC_FAR * This,
    /* [in] */ DWORD dwEncLevel);


void __RPC_STUB IDataFilter_SetEncodingLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDataFilter_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0197 */
/* [local] */ 

#endif
#ifndef _LPENCODINGFILTERFACTORY_DEFINED
#define _LPENCODINGFILTERFACTORY_DEFINED
typedef struct _tagPROTOCOLFILTERDATA
    {
    DWORD cbSize;
    IInternetProtocolSink __RPC_FAR *pProtocolSink;
    IInternetProtocol __RPC_FAR *pProtocol;
    IUnknown __RPC_FAR *pUnk;
    DWORD dwFilterFlags;
    }	PROTOCOLFILTERDATA;



extern RPC_IF_HANDLE __MIDL_itf_urlmon_0197_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0197_v0_0_s_ifspec;

#ifndef __IEncodingFilterFactory_INTERFACE_DEFINED__
#define __IEncodingFilterFactory_INTERFACE_DEFINED__

/* interface IEncodingFilterFactory */
/* [unique][uuid][object][local] */ 

typedef /* [unique] */ IEncodingFilterFactory __RPC_FAR *LPENCODINGFILTERFACTORY;

typedef struct _tagDATAINFO
    {
    ULONG ulTotalSize;
    ULONG ulavrPacketSize;
    ULONG ulConnectSpeed;
    ULONG ulProcessorSpeed;
    }	DATAINFO;


EXTERN_C const IID IID_IEncodingFilterFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("70bdde00-c18e-11d0-a9ce-006097942311")
    IEncodingFilterFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FindBestFilter( 
            /* [in] */ LPCWSTR pwzCodeIn,
            /* [in] */ LPCWSTR pwzCodeOut,
            /* [in] */ DATAINFO info,
            /* [out] */ IDataFilter __RPC_FAR *__RPC_FAR *ppDF) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultFilter( 
            /* [in] */ LPCWSTR pwzCodeIn,
            /* [in] */ LPCWSTR pwzCodeOut,
            /* [out] */ IDataFilter __RPC_FAR *__RPC_FAR *ppDF) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEncodingFilterFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEncodingFilterFactory __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEncodingFilterFactory __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEncodingFilterFactory __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindBestFilter )( 
            IEncodingFilterFactory __RPC_FAR * This,
            /* [in] */ LPCWSTR pwzCodeIn,
            /* [in] */ LPCWSTR pwzCodeOut,
            /* [in] */ DATAINFO info,
            /* [out] */ IDataFilter __RPC_FAR *__RPC_FAR *ppDF);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDefaultFilter )( 
            IEncodingFilterFactory __RPC_FAR * This,
            /* [in] */ LPCWSTR pwzCodeIn,
            /* [in] */ LPCWSTR pwzCodeOut,
            /* [out] */ IDataFilter __RPC_FAR *__RPC_FAR *ppDF);
        
        END_INTERFACE
    } IEncodingFilterFactoryVtbl;

    interface IEncodingFilterFactory
    {
        CONST_VTBL struct IEncodingFilterFactoryVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEncodingFilterFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEncodingFilterFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEncodingFilterFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEncodingFilterFactory_FindBestFilter(This,pwzCodeIn,pwzCodeOut,info,ppDF)	\
    (This)->lpVtbl -> FindBestFilter(This,pwzCodeIn,pwzCodeOut,info,ppDF)

#define IEncodingFilterFactory_GetDefaultFilter(This,pwzCodeIn,pwzCodeOut,ppDF)	\
    (This)->lpVtbl -> GetDefaultFilter(This,pwzCodeIn,pwzCodeOut,ppDF)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IEncodingFilterFactory_FindBestFilter_Proxy( 
    IEncodingFilterFactory __RPC_FAR * This,
    /* [in] */ LPCWSTR pwzCodeIn,
    /* [in] */ LPCWSTR pwzCodeOut,
    /* [in] */ DATAINFO info,
    /* [out] */ IDataFilter __RPC_FAR *__RPC_FAR *ppDF);


void __RPC_STUB IEncodingFilterFactory_FindBestFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEncodingFilterFactory_GetDefaultFilter_Proxy( 
    IEncodingFilterFactory __RPC_FAR * This,
    /* [in] */ LPCWSTR pwzCodeIn,
    /* [in] */ LPCWSTR pwzCodeOut,
    /* [out] */ IDataFilter __RPC_FAR *__RPC_FAR *ppDF);


void __RPC_STUB IEncodingFilterFactory_GetDefaultFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEncodingFilterFactory_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_urlmon_0198 */
/* [local] */ 

#endif
#ifndef _HITLOGGING_DEFINED
#define _HITLOGGING_DEFINED
// Logging-specific apis
BOOL WINAPI IsLoggingEnabledA(IN LPCSTR  pszUrl);                    
BOOL WINAPI IsLoggingEnabledW(IN LPCWSTR  pwszUrl);                  
#ifdef UNICODE                                                       
#define IsLoggingEnabled         IsLoggingEnabledW                   
#else                                                                
#define IsLoggingEnabled         IsLoggingEnabledA                   
#endif // !UNICODE                                                   
typedef struct _tagHIT_LOGGING_INFO
    {
    DWORD dwStructSize;
    LPSTR lpszLoggedUrlName;
    SYSTEMTIME StartTime;
    SYSTEMTIME EndTime;
    LPSTR lpszExtendedInfo;
    }	HIT_LOGGING_INFO;

typedef struct _tagHIT_LOGGING_INFO __RPC_FAR *LPHIT_LOGGING_INFO;

BOOL WINAPI WriteHitLogging(IN LPHIT_LOGGING_INFO lpLogginginfo);    
#define CONFIRMSAFETYACTION_LOADOBJECT  0x00000001
struct CONFIRMSAFETY
    {
    CLSID clsid;
    IUnknown __RPC_FAR *pUnk;
    DWORD dwFlags;
    };
EXTERN_C const GUID GUID_CUSTOM_CONFIRMOBJECTSAFETY; 
#endif


extern RPC_IF_HANDLE __MIDL_itf_urlmon_0198_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlmon_0198_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long __RPC_FAR *, unsigned long            , HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long __RPC_FAR *, HWND __RPC_FAR * ); 

/* [local] */ HRESULT STDMETHODCALLTYPE IBinding_GetBindResult_Proxy( 
    IBinding __RPC_FAR * This,
    /* [out] */ CLSID __RPC_FAR *pclsidProtocol,
    /* [out] */ DWORD __RPC_FAR *pdwResult,
    /* [out] */ LPOLESTR __RPC_FAR *pszResult,
    /* [out][in] */ DWORD __RPC_FAR *pdwReserved);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IBinding_GetBindResult_Stub( 
    IBinding __RPC_FAR * This,
    /* [out] */ CLSID __RPC_FAR *pclsidProtocol,
    /* [out] */ DWORD __RPC_FAR *pdwResult,
    /* [out] */ LPOLESTR __RPC_FAR *pszResult,
    /* [in] */ DWORD dwReserved);

/* [local] */ HRESULT STDMETHODCALLTYPE IBindStatusCallback_GetBindInfo_Proxy( 
    IBindStatusCallback __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *grfBINDF,
    /* [unique][out][in] */ BINDINFO __RPC_FAR *pbindinfo);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IBindStatusCallback_GetBindInfo_Stub( 
    IBindStatusCallback __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *grfBINDF,
    /* [unique][out][in] */ RemBINDINFO __RPC_FAR *pbindinfo,
    /* [unique][out][in] */ RemSTGMEDIUM __RPC_FAR *pstgmed);

/* [local] */ HRESULT STDMETHODCALLTYPE IBindStatusCallback_OnDataAvailable_Proxy( 
    IBindStatusCallback __RPC_FAR * This,
    /* [in] */ DWORD grfBSCF,
    /* [in] */ DWORD dwSize,
    /* [in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [in] */ STGMEDIUM __RPC_FAR *pstgmed);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IBindStatusCallback_OnDataAvailable_Stub( 
    IBindStatusCallback __RPC_FAR * This,
    /* [in] */ DWORD grfBSCF,
    /* [in] */ DWORD dwSize,
    /* [in] */ RemFORMATETC __RPC_FAR *pformatetc,
    /* [in] */ RemSTGMEDIUM __RPC_FAR *pstgmed);

/* [local] */ HRESULT STDMETHODCALLTYPE IWinInetInfo_QueryOption_Proxy( 
    IWinInetInfo __RPC_FAR * This,
    /* [in] */ DWORD dwOption,
    /* [size_is][out][in] */ LPVOID pBuffer,
    /* [out][in] */ DWORD __RPC_FAR *pcbBuf);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IWinInetInfo_QueryOption_Stub( 
    IWinInetInfo __RPC_FAR * This,
    /* [in] */ DWORD dwOption,
    /* [size_is][out][in] */ BYTE __RPC_FAR *pBuffer,
    /* [out][in] */ DWORD __RPC_FAR *pcbBuf);

/* [local] */ HRESULT STDMETHODCALLTYPE IWinInetHttpInfo_QueryInfo_Proxy( 
    IWinInetHttpInfo __RPC_FAR * This,
    /* [in] */ DWORD dwOption,
    /* [size_is][out][in] */ LPVOID pBuffer,
    /* [out][in] */ DWORD __RPC_FAR *pcbBuf,
    /* [out][in] */ DWORD __RPC_FAR *pdwFlags,
    /* [out][in] */ DWORD __RPC_FAR *pdwReserved);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IWinInetHttpInfo_QueryInfo_Stub( 
    IWinInetHttpInfo __RPC_FAR * This,
    /* [in] */ DWORD dwOption,
    /* [size_is][out][in] */ BYTE __RPC_FAR *pBuffer,
    /* [out][in] */ DWORD __RPC_FAR *pcbBuf,
    /* [out][in] */ DWORD __RPC_FAR *pdwFlags,
    /* [out][in] */ DWORD __RPC_FAR *pdwReserved);

/* [local] */ HRESULT STDMETHODCALLTYPE IBindHost_MonikerBindToStorage_Proxy( 
    IBindHost __RPC_FAR * This,
    /* [in] */ IMoniker __RPC_FAR *pMk,
    /* [in] */ IBindCtx __RPC_FAR *pBC,
    /* [in] */ IBindStatusCallback __RPC_FAR *pBSC,
    /* [in] */ REFIID riid,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IBindHost_MonikerBindToStorage_Stub( 
    IBindHost __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pMk,
    /* [unique][in] */ IBindCtx __RPC_FAR *pBC,
    /* [unique][in] */ IBindStatusCallback __RPC_FAR *pBSC,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObj);

/* [local] */ HRESULT STDMETHODCALLTYPE IBindHost_MonikerBindToObject_Proxy( 
    IBindHost __RPC_FAR * This,
    /* [in] */ IMoniker __RPC_FAR *pMk,
    /* [in] */ IBindCtx __RPC_FAR *pBC,
    /* [in] */ IBindStatusCallback __RPC_FAR *pBSC,
    /* [in] */ REFIID riid,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IBindHost_MonikerBindToObject_Stub( 
    IBindHost __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pMk,
    /* [unique][in] */ IBindCtx __RPC_FAR *pBC,
    /* [unique][in] */ IBindStatusCallback __RPC_FAR *pBSC,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObj);



/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



