
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 5.03.0279 */
/* at Wed Apr 23 14:12:30 2003
 */
/* Compiler settings for exdisp.idl:
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

#ifndef __exdisp_h__
#define __exdisp_h__

/* Forward Declarations */ 

#ifndef __IWebBrowser_FWD_DEFINED__
#define __IWebBrowser_FWD_DEFINED__
typedef interface IWebBrowser IWebBrowser;
#endif 	/* __IWebBrowser_FWD_DEFINED__ */


#ifndef __DWebBrowserEvents_FWD_DEFINED__
#define __DWebBrowserEvents_FWD_DEFINED__
typedef interface DWebBrowserEvents DWebBrowserEvents;
#endif 	/* __DWebBrowserEvents_FWD_DEFINED__ */


#ifndef __IWebBrowserApp_FWD_DEFINED__
#define __IWebBrowserApp_FWD_DEFINED__
typedef interface IWebBrowserApp IWebBrowserApp;
#endif 	/* __IWebBrowserApp_FWD_DEFINED__ */


#ifndef __IWebBrowser2_FWD_DEFINED__
#define __IWebBrowser2_FWD_DEFINED__
typedef interface IWebBrowser2 IWebBrowser2;
#endif 	/* __IWebBrowser2_FWD_DEFINED__ */


#ifndef __DWebBrowserEvents2_FWD_DEFINED__
#define __DWebBrowserEvents2_FWD_DEFINED__
typedef interface DWebBrowserEvents2 DWebBrowserEvents2;
#endif 	/* __DWebBrowserEvents2_FWD_DEFINED__ */


#ifndef __WebBrowser_V1_FWD_DEFINED__
#define __WebBrowser_V1_FWD_DEFINED__

#ifdef __cplusplus
typedef class WebBrowser_V1 WebBrowser_V1;
#else
typedef struct WebBrowser_V1 WebBrowser_V1;
#endif /* __cplusplus */

#endif 	/* __WebBrowser_V1_FWD_DEFINED__ */


#ifndef __WebBrowser_FWD_DEFINED__
#define __WebBrowser_FWD_DEFINED__

#ifdef __cplusplus
typedef class WebBrowser WebBrowser;
#else
typedef struct WebBrowser WebBrowser;
#endif /* __cplusplus */

#endif 	/* __WebBrowser_FWD_DEFINED__ */


#ifndef __InternetExplorer_FWD_DEFINED__
#define __InternetExplorer_FWD_DEFINED__

#ifdef __cplusplus
typedef class InternetExplorer InternetExplorer;
#else
typedef struct InternetExplorer InternetExplorer;
#endif /* __cplusplus */

#endif 	/* __InternetExplorer_FWD_DEFINED__ */


#ifndef __ShellBrowserWindow_FWD_DEFINED__
#define __ShellBrowserWindow_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellBrowserWindow ShellBrowserWindow;
#else
typedef struct ShellBrowserWindow ShellBrowserWindow;
#endif /* __cplusplus */

#endif 	/* __ShellBrowserWindow_FWD_DEFINED__ */


#ifndef __DShellWindowsEvents_FWD_DEFINED__
#define __DShellWindowsEvents_FWD_DEFINED__
typedef interface DShellWindowsEvents DShellWindowsEvents;
#endif 	/* __DShellWindowsEvents_FWD_DEFINED__ */


#ifndef __IShellWindows_FWD_DEFINED__
#define __IShellWindows_FWD_DEFINED__
typedef interface IShellWindows IShellWindows;
#endif 	/* __IShellWindows_FWD_DEFINED__ */


#ifndef __ShellWindows_FWD_DEFINED__
#define __ShellWindows_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellWindows ShellWindows;
#else
typedef struct ShellWindows ShellWindows;
#endif /* __cplusplus */

#endif 	/* __ShellWindows_FWD_DEFINED__ */


#ifndef __IShellUIHelper_FWD_DEFINED__
#define __IShellUIHelper_FWD_DEFINED__
typedef interface IShellUIHelper IShellUIHelper;
#endif 	/* __IShellUIHelper_FWD_DEFINED__ */


#ifndef __ShellUIHelper_FWD_DEFINED__
#define __ShellUIHelper_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellUIHelper ShellUIHelper;
#else
typedef struct ShellUIHelper ShellUIHelper;
#endif /* __cplusplus */

#endif 	/* __ShellUIHelper_FWD_DEFINED__ */


#ifndef ___ShellFavoritesNameSpaceEvents_FWD_DEFINED__
#define ___ShellFavoritesNameSpaceEvents_FWD_DEFINED__
typedef interface _ShellFavoritesNameSpaceEvents _ShellFavoritesNameSpaceEvents;
#endif 	/* ___ShellFavoritesNameSpaceEvents_FWD_DEFINED__ */


#ifndef __IShellFavoritesNameSpace_FWD_DEFINED__
#define __IShellFavoritesNameSpace_FWD_DEFINED__
typedef interface IShellFavoritesNameSpace IShellFavoritesNameSpace;
#endif 	/* __IShellFavoritesNameSpace_FWD_DEFINED__ */


#ifndef __ShellFavoritesNameSpace_FWD_DEFINED__
#define __ShellFavoritesNameSpace_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellFavoritesNameSpace ShellFavoritesNameSpace;
#else
typedef struct ShellFavoritesNameSpace ShellFavoritesNameSpace;
#endif /* __cplusplus */

#endif 	/* __ShellFavoritesNameSpace_FWD_DEFINED__ */


#ifndef __IScriptErrorList_FWD_DEFINED__
#define __IScriptErrorList_FWD_DEFINED__
typedef interface IScriptErrorList IScriptErrorList;
#endif 	/* __IScriptErrorList_FWD_DEFINED__ */


#ifndef __CScriptErrorList_FWD_DEFINED__
#define __CScriptErrorList_FWD_DEFINED__

#ifdef __cplusplus
typedef class CScriptErrorList CScriptErrorList;
#else
typedef struct CScriptErrorList CScriptErrorList;
#endif /* __cplusplus */

#endif 	/* __CScriptErrorList_FWD_DEFINED__ */


#ifndef __ISearch_FWD_DEFINED__
#define __ISearch_FWD_DEFINED__
typedef interface ISearch ISearch;
#endif 	/* __ISearch_FWD_DEFINED__ */


#ifndef __ISearches_FWD_DEFINED__
#define __ISearches_FWD_DEFINED__
typedef interface ISearches ISearches;
#endif 	/* __ISearches_FWD_DEFINED__ */


#ifndef __ISearchAssistantOC_FWD_DEFINED__
#define __ISearchAssistantOC_FWD_DEFINED__
typedef interface ISearchAssistantOC ISearchAssistantOC;
#endif 	/* __ISearchAssistantOC_FWD_DEFINED__ */


#ifndef __ISearchAssistantOC2_FWD_DEFINED__
#define __ISearchAssistantOC2_FWD_DEFINED__
typedef interface ISearchAssistantOC2 ISearchAssistantOC2;
#endif 	/* __ISearchAssistantOC2_FWD_DEFINED__ */


#ifndef ___SearchAssistantEvents_FWD_DEFINED__
#define ___SearchAssistantEvents_FWD_DEFINED__
typedef interface _SearchAssistantEvents _SearchAssistantEvents;
#endif 	/* ___SearchAssistantEvents_FWD_DEFINED__ */


#ifndef __SearchAssistantOC_FWD_DEFINED__
#define __SearchAssistantOC_FWD_DEFINED__

#ifdef __cplusplus
typedef class SearchAssistantOC SearchAssistantOC;
#else
typedef struct SearchAssistantOC SearchAssistantOC;
#endif /* __cplusplus */

#endif 	/* __SearchAssistantOC_FWD_DEFINED__ */


/* header files for imported files */
#include "ocidl.h"
#include "docobj.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __SHDocVw_LIBRARY_DEFINED__
#define __SHDocVw_LIBRARY_DEFINED__

/* library SHDocVw */
/* [version][lcid][helpstring][uuid] */ 

typedef /* [helpstring][uuid] */  DECLSPEC_UUID("34A226E0-DF30-11CF-89A9-00A0C9054129") 
enum CommandStateChangeConstants
    {	CSC_UPDATECOMMANDS	= 0xffffffff,
	CSC_NAVIGATEFORWARD	= 0x1,
	CSC_NAVIGATEBACK	= 0x2
    }	CommandStateChangeConstants;

typedef /* [helpstring][uuid] */  DECLSPEC_UUID("F41E6981-28E5-11d0-82B4-00A0C90C29C5") 
enum ShellWindowTypeConstants
    {	SWC_EXPLORER	= 0,
	SWC_BROWSER	= 0x1,
	SWC_3RDPARTY	= 0x2,
	SWC_CALLBACK	= 0x4
    }	ShellWindowTypeConstants;

typedef /* [hidden][helpstring][uuid] */  DECLSPEC_UUID("7716a370-38ca-11d0-a48b-00a0c90a8f39") 
enum ShellWindowFindWindowOptions
    {	SWFO_NEEDDISPATCH	= 0x1,
	SWFO_INCLUDEPENDING	= 0x2,
	SWFO_COOKIEPASSED	= 0x4
    }	ShellWindowFindWindowOptions;


EXTERN_C const IID LIBID_SHDocVw;

#ifndef __IWebBrowser_INTERFACE_DEFINED__
#define __IWebBrowser_INTERFACE_DEFINED__

/* interface IWebBrowser */
/* [object][oleautomation][dual][hidden][helpcontext][helpstring][uuid] */ 

typedef /* [helpstring][uuid] */  DECLSPEC_UUID("14EE5380-A378-11cf-A731-00A0C9082637") 
enum BrowserNavConstants
    {	navOpenInNewWindow	= 0x1,
	navNoHistory	= 0x2,
	navNoReadFromCache	= 0x4,
	navNoWriteToCache	= 0x8,
	navAllowAutosearch	= 0x10,
	navBrowserBar	= 0x20
    }	BrowserNavConstants;

typedef /* [helpstring][uuid] */  DECLSPEC_UUID("C317C261-A991-11cf-A731-00A0C9082637") 
enum RefreshConstants
    {	REFRESH_NORMAL	= 0,
	REFRESH_IFEXPIRED	= 1,
	REFRESH_COMPLETELY	= 3
    }	RefreshConstants;


EXTERN_C const IID IID_IWebBrowser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EAB22AC1-30C1-11CF-A7EB-0000C05BAE0B")
    IWebBrowser : public IDispatch
    {
    public:
        virtual /* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE GoBack( void) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE GoForward( void) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE GoHome( void) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE GoSearch( void) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE Navigate( 
            /* [in] */ BSTR URL,
            /* [optional][in] */ VARIANT __RPC_FAR *Flags,
            /* [optional][in] */ VARIANT __RPC_FAR *TargetFrameName,
            /* [optional][in] */ VARIANT __RPC_FAR *PostData,
            /* [optional][in] */ VARIANT __RPC_FAR *Headers) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE Refresh2( 
            /* [optional][in] */ VARIANT __RPC_FAR *Level) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Container( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Document( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_TopLevelContainer( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Type( 
            /* [retval][out] */ BSTR __RPC_FAR *Type) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Left( 
            /* [retval][out] */ long __RPC_FAR *pl) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Left( 
            /* [in] */ long Left) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Top( 
            /* [retval][out] */ long __RPC_FAR *pl) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Top( 
            /* [in] */ long Top) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Width( 
            /* [retval][out] */ long __RPC_FAR *pl) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Width( 
            /* [in] */ long Width) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Height( 
            /* [retval][out] */ long __RPC_FAR *pl) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Height( 
            /* [in] */ long Height) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_LocationName( 
            /* [retval][out] */ BSTR __RPC_FAR *LocationName) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_LocationURL( 
            /* [retval][out] */ BSTR __RPC_FAR *LocationURL) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Busy( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWebBrowserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWebBrowser __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWebBrowser __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWebBrowser __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWebBrowser __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWebBrowser __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWebBrowser __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWebBrowser __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GoBack )( 
            IWebBrowser __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GoForward )( 
            IWebBrowser __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GoHome )( 
            IWebBrowser __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GoSearch )( 
            IWebBrowser __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Navigate )( 
            IWebBrowser __RPC_FAR * This,
            /* [in] */ BSTR URL,
            /* [optional][in] */ VARIANT __RPC_FAR *Flags,
            /* [optional][in] */ VARIANT __RPC_FAR *TargetFrameName,
            /* [optional][in] */ VARIANT __RPC_FAR *PostData,
            /* [optional][in] */ VARIANT __RPC_FAR *Headers);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Refresh )( 
            IWebBrowser __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Refresh2 )( 
            IWebBrowser __RPC_FAR * This,
            /* [optional][in] */ VARIANT __RPC_FAR *Level);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Stop )( 
            IWebBrowser __RPC_FAR * This);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            IWebBrowser __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            IWebBrowser __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Container )( 
            IWebBrowser __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Document )( 
            IWebBrowser __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TopLevelContainer )( 
            IWebBrowser __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Type )( 
            IWebBrowser __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *Type);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Left )( 
            IWebBrowser __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Left )( 
            IWebBrowser __RPC_FAR * This,
            /* [in] */ long Left);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Top )( 
            IWebBrowser __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Top )( 
            IWebBrowser __RPC_FAR * This,
            /* [in] */ long Top);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Width )( 
            IWebBrowser __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Width )( 
            IWebBrowser __RPC_FAR * This,
            /* [in] */ long Width);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Height )( 
            IWebBrowser __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Height )( 
            IWebBrowser __RPC_FAR * This,
            /* [in] */ long Height);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocationName )( 
            IWebBrowser __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *LocationName);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocationURL )( 
            IWebBrowser __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *LocationURL);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Busy )( 
            IWebBrowser __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);
        
        END_INTERFACE
    } IWebBrowserVtbl;

    interface IWebBrowser
    {
        CONST_VTBL struct IWebBrowserVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWebBrowser_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWebBrowser_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWebBrowser_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWebBrowser_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWebBrowser_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWebBrowser_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWebBrowser_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWebBrowser_GoBack(This)	\
    (This)->lpVtbl -> GoBack(This)

#define IWebBrowser_GoForward(This)	\
    (This)->lpVtbl -> GoForward(This)

#define IWebBrowser_GoHome(This)	\
    (This)->lpVtbl -> GoHome(This)

#define IWebBrowser_GoSearch(This)	\
    (This)->lpVtbl -> GoSearch(This)

#define IWebBrowser_Navigate(This,URL,Flags,TargetFrameName,PostData,Headers)	\
    (This)->lpVtbl -> Navigate(This,URL,Flags,TargetFrameName,PostData,Headers)

#define IWebBrowser_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IWebBrowser_Refresh2(This,Level)	\
    (This)->lpVtbl -> Refresh2(This,Level)

#define IWebBrowser_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IWebBrowser_get_Application(This,ppDisp)	\
    (This)->lpVtbl -> get_Application(This,ppDisp)

#define IWebBrowser_get_Parent(This,ppDisp)	\
    (This)->lpVtbl -> get_Parent(This,ppDisp)

#define IWebBrowser_get_Container(This,ppDisp)	\
    (This)->lpVtbl -> get_Container(This,ppDisp)

#define IWebBrowser_get_Document(This,ppDisp)	\
    (This)->lpVtbl -> get_Document(This,ppDisp)

#define IWebBrowser_get_TopLevelContainer(This,pBool)	\
    (This)->lpVtbl -> get_TopLevelContainer(This,pBool)

#define IWebBrowser_get_Type(This,Type)	\
    (This)->lpVtbl -> get_Type(This,Type)

#define IWebBrowser_get_Left(This,pl)	\
    (This)->lpVtbl -> get_Left(This,pl)

#define IWebBrowser_put_Left(This,Left)	\
    (This)->lpVtbl -> put_Left(This,Left)

#define IWebBrowser_get_Top(This,pl)	\
    (This)->lpVtbl -> get_Top(This,pl)

#define IWebBrowser_put_Top(This,Top)	\
    (This)->lpVtbl -> put_Top(This,Top)

#define IWebBrowser_get_Width(This,pl)	\
    (This)->lpVtbl -> get_Width(This,pl)

#define IWebBrowser_put_Width(This,Width)	\
    (This)->lpVtbl -> put_Width(This,Width)

#define IWebBrowser_get_Height(This,pl)	\
    (This)->lpVtbl -> get_Height(This,pl)

#define IWebBrowser_put_Height(This,Height)	\
    (This)->lpVtbl -> put_Height(This,Height)

#define IWebBrowser_get_LocationName(This,LocationName)	\
    (This)->lpVtbl -> get_LocationName(This,LocationName)

#define IWebBrowser_get_LocationURL(This,LocationURL)	\
    (This)->lpVtbl -> get_LocationURL(This,LocationURL)

#define IWebBrowser_get_Busy(This,pBool)	\
    (This)->lpVtbl -> get_Busy(This,pBool)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_GoBack_Proxy( 
    IWebBrowser __RPC_FAR * This);


void __RPC_STUB IWebBrowser_GoBack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_GoForward_Proxy( 
    IWebBrowser __RPC_FAR * This);


void __RPC_STUB IWebBrowser_GoForward_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_GoHome_Proxy( 
    IWebBrowser __RPC_FAR * This);


void __RPC_STUB IWebBrowser_GoHome_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_GoSearch_Proxy( 
    IWebBrowser __RPC_FAR * This);


void __RPC_STUB IWebBrowser_GoSearch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_Navigate_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [in] */ BSTR URL,
    /* [optional][in] */ VARIANT __RPC_FAR *Flags,
    /* [optional][in] */ VARIANT __RPC_FAR *TargetFrameName,
    /* [optional][in] */ VARIANT __RPC_FAR *PostData,
    /* [optional][in] */ VARIANT __RPC_FAR *Headers);


void __RPC_STUB IWebBrowser_Navigate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_Refresh_Proxy( 
    IWebBrowser __RPC_FAR * This);


void __RPC_STUB IWebBrowser_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_Refresh2_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [optional][in] */ VARIANT __RPC_FAR *Level);


void __RPC_STUB IWebBrowser_Refresh2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_Stop_Proxy( 
    IWebBrowser __RPC_FAR * This);


void __RPC_STUB IWebBrowser_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_get_Application_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);


void __RPC_STUB IWebBrowser_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_get_Parent_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);


void __RPC_STUB IWebBrowser_get_Parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_get_Container_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);


void __RPC_STUB IWebBrowser_get_Container_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_get_Document_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);


void __RPC_STUB IWebBrowser_get_Document_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_get_TopLevelContainer_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);


void __RPC_STUB IWebBrowser_get_TopLevelContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_get_Type_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *Type);


void __RPC_STUB IWebBrowser_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_get_Left_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pl);


void __RPC_STUB IWebBrowser_get_Left_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_put_Left_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [in] */ long Left);


void __RPC_STUB IWebBrowser_put_Left_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_get_Top_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pl);


void __RPC_STUB IWebBrowser_get_Top_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_put_Top_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [in] */ long Top);


void __RPC_STUB IWebBrowser_put_Top_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_get_Width_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pl);


void __RPC_STUB IWebBrowser_get_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_put_Width_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [in] */ long Width);


void __RPC_STUB IWebBrowser_put_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_get_Height_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pl);


void __RPC_STUB IWebBrowser_get_Height_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_put_Height_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [in] */ long Height);


void __RPC_STUB IWebBrowser_put_Height_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_get_LocationName_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *LocationName);


void __RPC_STUB IWebBrowser_get_LocationName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_get_LocationURL_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *LocationURL);


void __RPC_STUB IWebBrowser_get_LocationURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser_get_Busy_Proxy( 
    IWebBrowser __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);


void __RPC_STUB IWebBrowser_get_Busy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWebBrowser_INTERFACE_DEFINED__ */


#ifndef __DWebBrowserEvents_DISPINTERFACE_DEFINED__
#define __DWebBrowserEvents_DISPINTERFACE_DEFINED__

/* dispinterface DWebBrowserEvents */
/* [hidden][helpstring][uuid] */ 


EXTERN_C const IID DIID_DWebBrowserEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("EAB22AC2-30C1-11CF-A7EB-0000C05BAE0B")
    DWebBrowserEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct DWebBrowserEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            DWebBrowserEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            DWebBrowserEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            DWebBrowserEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            DWebBrowserEvents __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            DWebBrowserEvents __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            DWebBrowserEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            DWebBrowserEvents __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } DWebBrowserEventsVtbl;

    interface DWebBrowserEvents
    {
        CONST_VTBL struct DWebBrowserEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DWebBrowserEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DWebBrowserEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DWebBrowserEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DWebBrowserEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DWebBrowserEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DWebBrowserEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DWebBrowserEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __DWebBrowserEvents_DISPINTERFACE_DEFINED__ */


#ifndef __IWebBrowserApp_INTERFACE_DEFINED__
#define __IWebBrowserApp_INTERFACE_DEFINED__

/* interface IWebBrowserApp */
/* [object][dual][oleautomation][hidden][helpcontext][helpstring][uuid] */ 


EXTERN_C const IID IID_IWebBrowserApp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0002DF05-0000-0000-C000-000000000046")
    IWebBrowserApp : public IWebBrowser
    {
    public:
        virtual /* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE Quit( void) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE ClientToWindow( 
            /* [out][in] */ int __RPC_FAR *pcx,
            /* [out][in] */ int __RPC_FAR *pcy) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE PutProperty( 
            /* [in] */ BSTR Property,
            /* [in] */ VARIANT vtValue) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE GetProperty( 
            /* [in] */ BSTR Property,
            /* [retval][out] */ VARIANT __RPC_FAR *pvtValue) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR __RPC_FAR *Name) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_HWND( 
            /* [retval][out] */ long __RPC_FAR *pHWND) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_FullName( 
            /* [retval][out] */ BSTR __RPC_FAR *FullName) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Path( 
            /* [retval][out] */ BSTR __RPC_FAR *Path) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ VARIANT_BOOL Value) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_StatusBar( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_StatusBar( 
            /* [in] */ VARIANT_BOOL Value) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_StatusText( 
            /* [retval][out] */ BSTR __RPC_FAR *StatusText) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_StatusText( 
            /* [in] */ BSTR StatusText) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_ToolBar( 
            /* [retval][out] */ int __RPC_FAR *Value) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_ToolBar( 
            /* [in] */ int Value) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_MenuBar( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *Value) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_MenuBar( 
            /* [in] */ VARIANT_BOOL Value) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_FullScreen( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbFullScreen) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_FullScreen( 
            /* [in] */ VARIANT_BOOL bFullScreen) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWebBrowserAppVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWebBrowserApp __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWebBrowserApp __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GoBack )( 
            IWebBrowserApp __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GoForward )( 
            IWebBrowserApp __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GoHome )( 
            IWebBrowserApp __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GoSearch )( 
            IWebBrowserApp __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Navigate )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [in] */ BSTR URL,
            /* [optional][in] */ VARIANT __RPC_FAR *Flags,
            /* [optional][in] */ VARIANT __RPC_FAR *TargetFrameName,
            /* [optional][in] */ VARIANT __RPC_FAR *PostData,
            /* [optional][in] */ VARIANT __RPC_FAR *Headers);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Refresh )( 
            IWebBrowserApp __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Refresh2 )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [optional][in] */ VARIANT __RPC_FAR *Level);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Stop )( 
            IWebBrowserApp __RPC_FAR * This);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Container )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Document )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TopLevelContainer )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Type )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *Type);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Left )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Left )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [in] */ long Left);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Top )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Top )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [in] */ long Top);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Width )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Width )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [in] */ long Width);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Height )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Height )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [in] */ long Height);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocationName )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *LocationName);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocationURL )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *LocationURL);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Busy )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Quit )( 
            IWebBrowserApp __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ClientToWindow )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [out][in] */ int __RPC_FAR *pcx,
            /* [out][in] */ int __RPC_FAR *pcy);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PutProperty )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [in] */ BSTR Property,
            /* [in] */ VARIANT vtValue);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProperty )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [in] */ BSTR Property,
            /* [retval][out] */ VARIANT __RPC_FAR *pvtValue);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *Name);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HWND )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pHWND);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FullName )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *FullName);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Path )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *Path);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Visible )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Visible )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL Value);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StatusBar )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StatusBar )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL Value);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StatusText )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *StatusText);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StatusText )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [in] */ BSTR StatusText);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ToolBar )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ int __RPC_FAR *Value);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ToolBar )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [in] */ int Value);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MenuBar )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *Value);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MenuBar )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL Value);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FullScreen )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbFullScreen);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_FullScreen )( 
            IWebBrowserApp __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bFullScreen);
        
        END_INTERFACE
    } IWebBrowserAppVtbl;

    interface IWebBrowserApp
    {
        CONST_VTBL struct IWebBrowserAppVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWebBrowserApp_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWebBrowserApp_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWebBrowserApp_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWebBrowserApp_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWebBrowserApp_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWebBrowserApp_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWebBrowserApp_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWebBrowserApp_GoBack(This)	\
    (This)->lpVtbl -> GoBack(This)

#define IWebBrowserApp_GoForward(This)	\
    (This)->lpVtbl -> GoForward(This)

#define IWebBrowserApp_GoHome(This)	\
    (This)->lpVtbl -> GoHome(This)

#define IWebBrowserApp_GoSearch(This)	\
    (This)->lpVtbl -> GoSearch(This)

#define IWebBrowserApp_Navigate(This,URL,Flags,TargetFrameName,PostData,Headers)	\
    (This)->lpVtbl -> Navigate(This,URL,Flags,TargetFrameName,PostData,Headers)

#define IWebBrowserApp_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IWebBrowserApp_Refresh2(This,Level)	\
    (This)->lpVtbl -> Refresh2(This,Level)

#define IWebBrowserApp_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IWebBrowserApp_get_Application(This,ppDisp)	\
    (This)->lpVtbl -> get_Application(This,ppDisp)

#define IWebBrowserApp_get_Parent(This,ppDisp)	\
    (This)->lpVtbl -> get_Parent(This,ppDisp)

#define IWebBrowserApp_get_Container(This,ppDisp)	\
    (This)->lpVtbl -> get_Container(This,ppDisp)

#define IWebBrowserApp_get_Document(This,ppDisp)	\
    (This)->lpVtbl -> get_Document(This,ppDisp)

#define IWebBrowserApp_get_TopLevelContainer(This,pBool)	\
    (This)->lpVtbl -> get_TopLevelContainer(This,pBool)

#define IWebBrowserApp_get_Type(This,Type)	\
    (This)->lpVtbl -> get_Type(This,Type)

#define IWebBrowserApp_get_Left(This,pl)	\
    (This)->lpVtbl -> get_Left(This,pl)

#define IWebBrowserApp_put_Left(This,Left)	\
    (This)->lpVtbl -> put_Left(This,Left)

#define IWebBrowserApp_get_Top(This,pl)	\
    (This)->lpVtbl -> get_Top(This,pl)

#define IWebBrowserApp_put_Top(This,Top)	\
    (This)->lpVtbl -> put_Top(This,Top)

#define IWebBrowserApp_get_Width(This,pl)	\
    (This)->lpVtbl -> get_Width(This,pl)

#define IWebBrowserApp_put_Width(This,Width)	\
    (This)->lpVtbl -> put_Width(This,Width)

#define IWebBrowserApp_get_Height(This,pl)	\
    (This)->lpVtbl -> get_Height(This,pl)

#define IWebBrowserApp_put_Height(This,Height)	\
    (This)->lpVtbl -> put_Height(This,Height)

#define IWebBrowserApp_get_LocationName(This,LocationName)	\
    (This)->lpVtbl -> get_LocationName(This,LocationName)

#define IWebBrowserApp_get_LocationURL(This,LocationURL)	\
    (This)->lpVtbl -> get_LocationURL(This,LocationURL)

#define IWebBrowserApp_get_Busy(This,pBool)	\
    (This)->lpVtbl -> get_Busy(This,pBool)


#define IWebBrowserApp_Quit(This)	\
    (This)->lpVtbl -> Quit(This)

#define IWebBrowserApp_ClientToWindow(This,pcx,pcy)	\
    (This)->lpVtbl -> ClientToWindow(This,pcx,pcy)

#define IWebBrowserApp_PutProperty(This,Property,vtValue)	\
    (This)->lpVtbl -> PutProperty(This,Property,vtValue)

#define IWebBrowserApp_GetProperty(This,Property,pvtValue)	\
    (This)->lpVtbl -> GetProperty(This,Property,pvtValue)

#define IWebBrowserApp_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IWebBrowserApp_get_HWND(This,pHWND)	\
    (This)->lpVtbl -> get_HWND(This,pHWND)

#define IWebBrowserApp_get_FullName(This,FullName)	\
    (This)->lpVtbl -> get_FullName(This,FullName)

#define IWebBrowserApp_get_Path(This,Path)	\
    (This)->lpVtbl -> get_Path(This,Path)

#define IWebBrowserApp_get_Visible(This,pBool)	\
    (This)->lpVtbl -> get_Visible(This,pBool)

#define IWebBrowserApp_put_Visible(This,Value)	\
    (This)->lpVtbl -> put_Visible(This,Value)

#define IWebBrowserApp_get_StatusBar(This,pBool)	\
    (This)->lpVtbl -> get_StatusBar(This,pBool)

#define IWebBrowserApp_put_StatusBar(This,Value)	\
    (This)->lpVtbl -> put_StatusBar(This,Value)

#define IWebBrowserApp_get_StatusText(This,StatusText)	\
    (This)->lpVtbl -> get_StatusText(This,StatusText)

#define IWebBrowserApp_put_StatusText(This,StatusText)	\
    (This)->lpVtbl -> put_StatusText(This,StatusText)

#define IWebBrowserApp_get_ToolBar(This,Value)	\
    (This)->lpVtbl -> get_ToolBar(This,Value)

#define IWebBrowserApp_put_ToolBar(This,Value)	\
    (This)->lpVtbl -> put_ToolBar(This,Value)

#define IWebBrowserApp_get_MenuBar(This,Value)	\
    (This)->lpVtbl -> get_MenuBar(This,Value)

#define IWebBrowserApp_put_MenuBar(This,Value)	\
    (This)->lpVtbl -> put_MenuBar(This,Value)

#define IWebBrowserApp_get_FullScreen(This,pbFullScreen)	\
    (This)->lpVtbl -> get_FullScreen(This,pbFullScreen)

#define IWebBrowserApp_put_FullScreen(This,bFullScreen)	\
    (This)->lpVtbl -> put_FullScreen(This,bFullScreen)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_Quit_Proxy( 
    IWebBrowserApp __RPC_FAR * This);


void __RPC_STUB IWebBrowserApp_Quit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_ClientToWindow_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [out][in] */ int __RPC_FAR *pcx,
    /* [out][in] */ int __RPC_FAR *pcy);


void __RPC_STUB IWebBrowserApp_ClientToWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_PutProperty_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [in] */ BSTR Property,
    /* [in] */ VARIANT vtValue);


void __RPC_STUB IWebBrowserApp_PutProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_GetProperty_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [in] */ BSTR Property,
    /* [retval][out] */ VARIANT __RPC_FAR *pvtValue);


void __RPC_STUB IWebBrowserApp_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_get_Name_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *Name);


void __RPC_STUB IWebBrowserApp_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_get_HWND_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pHWND);


void __RPC_STUB IWebBrowserApp_get_HWND_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_get_FullName_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *FullName);


void __RPC_STUB IWebBrowserApp_get_FullName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_get_Path_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *Path);


void __RPC_STUB IWebBrowserApp_get_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_get_Visible_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);


void __RPC_STUB IWebBrowserApp_get_Visible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_put_Visible_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL Value);


void __RPC_STUB IWebBrowserApp_put_Visible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_get_StatusBar_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);


void __RPC_STUB IWebBrowserApp_get_StatusBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_put_StatusBar_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL Value);


void __RPC_STUB IWebBrowserApp_put_StatusBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_get_StatusText_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *StatusText);


void __RPC_STUB IWebBrowserApp_get_StatusText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_put_StatusText_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [in] */ BSTR StatusText);


void __RPC_STUB IWebBrowserApp_put_StatusText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_get_ToolBar_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [retval][out] */ int __RPC_FAR *Value);


void __RPC_STUB IWebBrowserApp_get_ToolBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_put_ToolBar_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [in] */ int Value);


void __RPC_STUB IWebBrowserApp_put_ToolBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_get_MenuBar_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *Value);


void __RPC_STUB IWebBrowserApp_get_MenuBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_put_MenuBar_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL Value);


void __RPC_STUB IWebBrowserApp_put_MenuBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_get_FullScreen_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbFullScreen);


void __RPC_STUB IWebBrowserApp_get_FullScreen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE IWebBrowserApp_put_FullScreen_Proxy( 
    IWebBrowserApp __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL bFullScreen);


void __RPC_STUB IWebBrowserApp_put_FullScreen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWebBrowserApp_INTERFACE_DEFINED__ */


#ifndef __IWebBrowser2_INTERFACE_DEFINED__
#define __IWebBrowser2_INTERFACE_DEFINED__

/* interface IWebBrowser2 */
/* [object][dual][oleautomation][hidden][helpcontext][helpstring][uuid] */ 


EXTERN_C const IID IID_IWebBrowser2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D30C1661-CDAF-11d0-8A3E-00C04FC9E26E")
    IWebBrowser2 : public IWebBrowserApp
    {
    public:
        virtual /* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE Navigate2( 
            /* [in] */ VARIANT __RPC_FAR *URL,
            /* [optional][in] */ VARIANT __RPC_FAR *Flags,
            /* [optional][in] */ VARIANT __RPC_FAR *TargetFrameName,
            /* [optional][in] */ VARIANT __RPC_FAR *PostData,
            /* [optional][in] */ VARIANT __RPC_FAR *Headers) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE QueryStatusWB( 
            /* [in] */ OLECMDID cmdID,
            /* [retval][out] */ OLECMDF __RPC_FAR *pcmdf) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE ExecWB( 
            /* [in] */ OLECMDID cmdID,
            /* [in] */ OLECMDEXECOPT cmdexecopt,
            /* [optional][in] */ VARIANT __RPC_FAR *pvaIn,
            /* [optional][in][out] */ VARIANT __RPC_FAR *pvaOut) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE ShowBrowserBar( 
            /* [in] */ VARIANT __RPC_FAR *pvaClsid,
            /* [optional][in] */ VARIANT __RPC_FAR *pvarShow,
            /* [optional][in] */ VARIANT __RPC_FAR *pvarSize) = 0;
        
        virtual /* [bindable][propget][id] */ HRESULT STDMETHODCALLTYPE get_ReadyState( 
            /* [out][retval] */ READYSTATE __RPC_FAR *plReadyState) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Offline( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbOffline) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_Offline( 
            /* [in] */ VARIANT_BOOL bOffline) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Silent( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbSilent) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_Silent( 
            /* [in] */ VARIANT_BOOL bSilent) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_RegisterAsBrowser( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbRegister) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_RegisterAsBrowser( 
            /* [in] */ VARIANT_BOOL bRegister) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_RegisterAsDropTarget( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbRegister) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_RegisterAsDropTarget( 
            /* [in] */ VARIANT_BOOL bRegister) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_TheaterMode( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbRegister) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_TheaterMode( 
            /* [in] */ VARIANT_BOOL bRegister) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_AddressBar( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *Value) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_AddressBar( 
            /* [in] */ VARIANT_BOOL Value) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Resizable( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *Value) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_Resizable( 
            /* [in] */ VARIANT_BOOL Value) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWebBrowser2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWebBrowser2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWebBrowser2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GoBack )( 
            IWebBrowser2 __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GoForward )( 
            IWebBrowser2 __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GoHome )( 
            IWebBrowser2 __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GoSearch )( 
            IWebBrowser2 __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Navigate )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ BSTR URL,
            /* [optional][in] */ VARIANT __RPC_FAR *Flags,
            /* [optional][in] */ VARIANT __RPC_FAR *TargetFrameName,
            /* [optional][in] */ VARIANT __RPC_FAR *PostData,
            /* [optional][in] */ VARIANT __RPC_FAR *Headers);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Refresh )( 
            IWebBrowser2 __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Refresh2 )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [optional][in] */ VARIANT __RPC_FAR *Level);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Stop )( 
            IWebBrowser2 __RPC_FAR * This);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Container )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Document )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TopLevelContainer )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Type )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *Type);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Left )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Left )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ long Left);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Top )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Top )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ long Top);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Width )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Width )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ long Width);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Height )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Height )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ long Height);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocationName )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *LocationName);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocationURL )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *LocationURL);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Busy )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Quit )( 
            IWebBrowser2 __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ClientToWindow )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [out][in] */ int __RPC_FAR *pcx,
            /* [out][in] */ int __RPC_FAR *pcy);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PutProperty )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ BSTR Property,
            /* [in] */ VARIANT vtValue);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProperty )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ BSTR Property,
            /* [retval][out] */ VARIANT __RPC_FAR *pvtValue);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *Name);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HWND )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pHWND);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FullName )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *FullName);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Path )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *Path);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Visible )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Visible )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL Value);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StatusBar )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StatusBar )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL Value);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StatusText )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *StatusText);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StatusText )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ BSTR StatusText);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ToolBar )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ int __RPC_FAR *Value);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ToolBar )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ int Value);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MenuBar )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *Value);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MenuBar )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL Value);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FullScreen )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbFullScreen);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_FullScreen )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bFullScreen);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Navigate2 )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *URL,
            /* [optional][in] */ VARIANT __RPC_FAR *Flags,
            /* [optional][in] */ VARIANT __RPC_FAR *TargetFrameName,
            /* [optional][in] */ VARIANT __RPC_FAR *PostData,
            /* [optional][in] */ VARIANT __RPC_FAR *Headers);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryStatusWB )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ OLECMDID cmdID,
            /* [retval][out] */ OLECMDF __RPC_FAR *pcmdf);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExecWB )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ OLECMDID cmdID,
            /* [in] */ OLECMDEXECOPT cmdexecopt,
            /* [optional][in] */ VARIANT __RPC_FAR *pvaIn,
            /* [optional][in][out] */ VARIANT __RPC_FAR *pvaOut);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShowBrowserBar )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *pvaClsid,
            /* [optional][in] */ VARIANT __RPC_FAR *pvarShow,
            /* [optional][in] */ VARIANT __RPC_FAR *pvarSize);
        
        /* [bindable][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ReadyState )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [out][retval] */ READYSTATE __RPC_FAR *plReadyState);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Offline )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbOffline);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Offline )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bOffline);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Silent )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbSilent);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Silent )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bSilent);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_RegisterAsBrowser )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbRegister);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_RegisterAsBrowser )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bRegister);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_RegisterAsDropTarget )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbRegister);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_RegisterAsDropTarget )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bRegister);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TheaterMode )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbRegister);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_TheaterMode )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bRegister);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AddressBar )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *Value);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AddressBar )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL Value);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Resizable )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *Value);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Resizable )( 
            IWebBrowser2 __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL Value);
        
        END_INTERFACE
    } IWebBrowser2Vtbl;

    interface IWebBrowser2
    {
        CONST_VTBL struct IWebBrowser2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWebBrowser2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWebBrowser2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWebBrowser2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWebBrowser2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWebBrowser2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWebBrowser2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWebBrowser2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWebBrowser2_GoBack(This)	\
    (This)->lpVtbl -> GoBack(This)

#define IWebBrowser2_GoForward(This)	\
    (This)->lpVtbl -> GoForward(This)

#define IWebBrowser2_GoHome(This)	\
    (This)->lpVtbl -> GoHome(This)

#define IWebBrowser2_GoSearch(This)	\
    (This)->lpVtbl -> GoSearch(This)

#define IWebBrowser2_Navigate(This,URL,Flags,TargetFrameName,PostData,Headers)	\
    (This)->lpVtbl -> Navigate(This,URL,Flags,TargetFrameName,PostData,Headers)

#define IWebBrowser2_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IWebBrowser2_Refresh2(This,Level)	\
    (This)->lpVtbl -> Refresh2(This,Level)

#define IWebBrowser2_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IWebBrowser2_get_Application(This,ppDisp)	\
    (This)->lpVtbl -> get_Application(This,ppDisp)

#define IWebBrowser2_get_Parent(This,ppDisp)	\
    (This)->lpVtbl -> get_Parent(This,ppDisp)

#define IWebBrowser2_get_Container(This,ppDisp)	\
    (This)->lpVtbl -> get_Container(This,ppDisp)

#define IWebBrowser2_get_Document(This,ppDisp)	\
    (This)->lpVtbl -> get_Document(This,ppDisp)

#define IWebBrowser2_get_TopLevelContainer(This,pBool)	\
    (This)->lpVtbl -> get_TopLevelContainer(This,pBool)

#define IWebBrowser2_get_Type(This,Type)	\
    (This)->lpVtbl -> get_Type(This,Type)

#define IWebBrowser2_get_Left(This,pl)	\
    (This)->lpVtbl -> get_Left(This,pl)

#define IWebBrowser2_put_Left(This,Left)	\
    (This)->lpVtbl -> put_Left(This,Left)

#define IWebBrowser2_get_Top(This,pl)	\
    (This)->lpVtbl -> get_Top(This,pl)

#define IWebBrowser2_put_Top(This,Top)	\
    (This)->lpVtbl -> put_Top(This,Top)

#define IWebBrowser2_get_Width(This,pl)	\
    (This)->lpVtbl -> get_Width(This,pl)

#define IWebBrowser2_put_Width(This,Width)	\
    (This)->lpVtbl -> put_Width(This,Width)

#define IWebBrowser2_get_Height(This,pl)	\
    (This)->lpVtbl -> get_Height(This,pl)

#define IWebBrowser2_put_Height(This,Height)	\
    (This)->lpVtbl -> put_Height(This,Height)

#define IWebBrowser2_get_LocationName(This,LocationName)	\
    (This)->lpVtbl -> get_LocationName(This,LocationName)

#define IWebBrowser2_get_LocationURL(This,LocationURL)	\
    (This)->lpVtbl -> get_LocationURL(This,LocationURL)

#define IWebBrowser2_get_Busy(This,pBool)	\
    (This)->lpVtbl -> get_Busy(This,pBool)


#define IWebBrowser2_Quit(This)	\
    (This)->lpVtbl -> Quit(This)

#define IWebBrowser2_ClientToWindow(This,pcx,pcy)	\
    (This)->lpVtbl -> ClientToWindow(This,pcx,pcy)

#define IWebBrowser2_PutProperty(This,Property,vtValue)	\
    (This)->lpVtbl -> PutProperty(This,Property,vtValue)

#define IWebBrowser2_GetProperty(This,Property,pvtValue)	\
    (This)->lpVtbl -> GetProperty(This,Property,pvtValue)

#define IWebBrowser2_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IWebBrowser2_get_HWND(This,pHWND)	\
    (This)->lpVtbl -> get_HWND(This,pHWND)

#define IWebBrowser2_get_FullName(This,FullName)	\
    (This)->lpVtbl -> get_FullName(This,FullName)

#define IWebBrowser2_get_Path(This,Path)	\
    (This)->lpVtbl -> get_Path(This,Path)

#define IWebBrowser2_get_Visible(This,pBool)	\
    (This)->lpVtbl -> get_Visible(This,pBool)

#define IWebBrowser2_put_Visible(This,Value)	\
    (This)->lpVtbl -> put_Visible(This,Value)

#define IWebBrowser2_get_StatusBar(This,pBool)	\
    (This)->lpVtbl -> get_StatusBar(This,pBool)

#define IWebBrowser2_put_StatusBar(This,Value)	\
    (This)->lpVtbl -> put_StatusBar(This,Value)

#define IWebBrowser2_get_StatusText(This,StatusText)	\
    (This)->lpVtbl -> get_StatusText(This,StatusText)

#define IWebBrowser2_put_StatusText(This,StatusText)	\
    (This)->lpVtbl -> put_StatusText(This,StatusText)

#define IWebBrowser2_get_ToolBar(This,Value)	\
    (This)->lpVtbl -> get_ToolBar(This,Value)

#define IWebBrowser2_put_ToolBar(This,Value)	\
    (This)->lpVtbl -> put_ToolBar(This,Value)

#define IWebBrowser2_get_MenuBar(This,Value)	\
    (This)->lpVtbl -> get_MenuBar(This,Value)

#define IWebBrowser2_put_MenuBar(This,Value)	\
    (This)->lpVtbl -> put_MenuBar(This,Value)

#define IWebBrowser2_get_FullScreen(This,pbFullScreen)	\
    (This)->lpVtbl -> get_FullScreen(This,pbFullScreen)

#define IWebBrowser2_put_FullScreen(This,bFullScreen)	\
    (This)->lpVtbl -> put_FullScreen(This,bFullScreen)


#define IWebBrowser2_Navigate2(This,URL,Flags,TargetFrameName,PostData,Headers)	\
    (This)->lpVtbl -> Navigate2(This,URL,Flags,TargetFrameName,PostData,Headers)

#define IWebBrowser2_QueryStatusWB(This,cmdID,pcmdf)	\
    (This)->lpVtbl -> QueryStatusWB(This,cmdID,pcmdf)

#define IWebBrowser2_ExecWB(This,cmdID,cmdexecopt,pvaIn,pvaOut)	\
    (This)->lpVtbl -> ExecWB(This,cmdID,cmdexecopt,pvaIn,pvaOut)

#define IWebBrowser2_ShowBrowserBar(This,pvaClsid,pvarShow,pvarSize)	\
    (This)->lpVtbl -> ShowBrowserBar(This,pvaClsid,pvarShow,pvarSize)

#define IWebBrowser2_get_ReadyState(This,plReadyState)	\
    (This)->lpVtbl -> get_ReadyState(This,plReadyState)

#define IWebBrowser2_get_Offline(This,pbOffline)	\
    (This)->lpVtbl -> get_Offline(This,pbOffline)

#define IWebBrowser2_put_Offline(This,bOffline)	\
    (This)->lpVtbl -> put_Offline(This,bOffline)

#define IWebBrowser2_get_Silent(This,pbSilent)	\
    (This)->lpVtbl -> get_Silent(This,pbSilent)

#define IWebBrowser2_put_Silent(This,bSilent)	\
    (This)->lpVtbl -> put_Silent(This,bSilent)

#define IWebBrowser2_get_RegisterAsBrowser(This,pbRegister)	\
    (This)->lpVtbl -> get_RegisterAsBrowser(This,pbRegister)

#define IWebBrowser2_put_RegisterAsBrowser(This,bRegister)	\
    (This)->lpVtbl -> put_RegisterAsBrowser(This,bRegister)

#define IWebBrowser2_get_RegisterAsDropTarget(This,pbRegister)	\
    (This)->lpVtbl -> get_RegisterAsDropTarget(This,pbRegister)

#define IWebBrowser2_put_RegisterAsDropTarget(This,bRegister)	\
    (This)->lpVtbl -> put_RegisterAsDropTarget(This,bRegister)

#define IWebBrowser2_get_TheaterMode(This,pbRegister)	\
    (This)->lpVtbl -> get_TheaterMode(This,pbRegister)

#define IWebBrowser2_put_TheaterMode(This,bRegister)	\
    (This)->lpVtbl -> put_TheaterMode(This,bRegister)

#define IWebBrowser2_get_AddressBar(This,Value)	\
    (This)->lpVtbl -> get_AddressBar(This,Value)

#define IWebBrowser2_put_AddressBar(This,Value)	\
    (This)->lpVtbl -> put_AddressBar(This,Value)

#define IWebBrowser2_get_Resizable(This,Value)	\
    (This)->lpVtbl -> get_Resizable(This,Value)

#define IWebBrowser2_put_Resizable(This,Value)	\
    (This)->lpVtbl -> put_Resizable(This,Value)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_Navigate2_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *URL,
    /* [optional][in] */ VARIANT __RPC_FAR *Flags,
    /* [optional][in] */ VARIANT __RPC_FAR *TargetFrameName,
    /* [optional][in] */ VARIANT __RPC_FAR *PostData,
    /* [optional][in] */ VARIANT __RPC_FAR *Headers);


void __RPC_STUB IWebBrowser2_Navigate2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_QueryStatusWB_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [in] */ OLECMDID cmdID,
    /* [retval][out] */ OLECMDF __RPC_FAR *pcmdf);


void __RPC_STUB IWebBrowser2_QueryStatusWB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_ExecWB_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [in] */ OLECMDID cmdID,
    /* [in] */ OLECMDEXECOPT cmdexecopt,
    /* [optional][in] */ VARIANT __RPC_FAR *pvaIn,
    /* [optional][in][out] */ VARIANT __RPC_FAR *pvaOut);


void __RPC_STUB IWebBrowser2_ExecWB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_ShowBrowserBar_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *pvaClsid,
    /* [optional][in] */ VARIANT __RPC_FAR *pvarShow,
    /* [optional][in] */ VARIANT __RPC_FAR *pvarSize);


void __RPC_STUB IWebBrowser2_ShowBrowserBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_get_ReadyState_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [out][retval] */ READYSTATE __RPC_FAR *plReadyState);


void __RPC_STUB IWebBrowser2_get_ReadyState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_get_Offline_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbOffline);


void __RPC_STUB IWebBrowser2_get_Offline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_put_Offline_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL bOffline);


void __RPC_STUB IWebBrowser2_put_Offline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_get_Silent_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbSilent);


void __RPC_STUB IWebBrowser2_get_Silent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_put_Silent_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL bSilent);


void __RPC_STUB IWebBrowser2_put_Silent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_get_RegisterAsBrowser_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbRegister);


void __RPC_STUB IWebBrowser2_get_RegisterAsBrowser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_put_RegisterAsBrowser_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL bRegister);


void __RPC_STUB IWebBrowser2_put_RegisterAsBrowser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_get_RegisterAsDropTarget_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbRegister);


void __RPC_STUB IWebBrowser2_get_RegisterAsDropTarget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_put_RegisterAsDropTarget_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL bRegister);


void __RPC_STUB IWebBrowser2_put_RegisterAsDropTarget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_get_TheaterMode_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbRegister);


void __RPC_STUB IWebBrowser2_get_TheaterMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_put_TheaterMode_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL bRegister);


void __RPC_STUB IWebBrowser2_put_TheaterMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_get_AddressBar_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *Value);


void __RPC_STUB IWebBrowser2_get_AddressBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_put_AddressBar_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL Value);


void __RPC_STUB IWebBrowser2_put_AddressBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_get_Resizable_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *Value);


void __RPC_STUB IWebBrowser2_get_Resizable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE IWebBrowser2_put_Resizable_Proxy( 
    IWebBrowser2 __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL Value);


void __RPC_STUB IWebBrowser2_put_Resizable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWebBrowser2_INTERFACE_DEFINED__ */


#ifndef __DWebBrowserEvents2_DISPINTERFACE_DEFINED__
#define __DWebBrowserEvents2_DISPINTERFACE_DEFINED__

/* dispinterface DWebBrowserEvents2 */
/* [hidden][helpstring][uuid] */ 


EXTERN_C const IID DIID_DWebBrowserEvents2;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("34A715A0-6587-11D0-924A-0020AFC7AC4D")
    DWebBrowserEvents2 : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct DWebBrowserEvents2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            DWebBrowserEvents2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            DWebBrowserEvents2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            DWebBrowserEvents2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            DWebBrowserEvents2 __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            DWebBrowserEvents2 __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            DWebBrowserEvents2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            DWebBrowserEvents2 __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } DWebBrowserEvents2Vtbl;

    interface DWebBrowserEvents2
    {
        CONST_VTBL struct DWebBrowserEvents2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DWebBrowserEvents2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DWebBrowserEvents2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DWebBrowserEvents2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DWebBrowserEvents2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DWebBrowserEvents2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DWebBrowserEvents2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DWebBrowserEvents2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __DWebBrowserEvents2_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_WebBrowser_V1;

#ifdef __cplusplus

class DECLSPEC_UUID("EAB22AC3-30C1-11CF-A7EB-0000C05BAE0B")
WebBrowser_V1;
#endif

EXTERN_C const CLSID CLSID_WebBrowser;

#ifdef __cplusplus

class DECLSPEC_UUID("8856F961-340A-11D0-A96B-00C04FD705A2")
WebBrowser;
#endif

EXTERN_C const CLSID CLSID_InternetExplorer;

#ifdef __cplusplus

class DECLSPEC_UUID("0002DF01-0000-0000-C000-000000000046")
InternetExplorer;
#endif

EXTERN_C const CLSID CLSID_ShellBrowserWindow;

#ifdef __cplusplus

class DECLSPEC_UUID("c08afd90-f2a1-11d1-8455-00a0c91f3880")
ShellBrowserWindow;
#endif

#ifndef __DShellWindowsEvents_DISPINTERFACE_DEFINED__
#define __DShellWindowsEvents_DISPINTERFACE_DEFINED__

/* dispinterface DShellWindowsEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID_DShellWindowsEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("fe4106e0-399a-11d0-a48c-00a0c90a8f39")
    DShellWindowsEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct DShellWindowsEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            DShellWindowsEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            DShellWindowsEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            DShellWindowsEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            DShellWindowsEvents __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            DShellWindowsEvents __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            DShellWindowsEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            DShellWindowsEvents __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } DShellWindowsEventsVtbl;

    interface DShellWindowsEvents
    {
        CONST_VTBL struct DShellWindowsEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DShellWindowsEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DShellWindowsEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DShellWindowsEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DShellWindowsEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DShellWindowsEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DShellWindowsEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DShellWindowsEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __DShellWindowsEvents_DISPINTERFACE_DEFINED__ */


#ifndef __IShellWindows_INTERFACE_DEFINED__
#define __IShellWindows_INTERFACE_DEFINED__

/* interface IShellWindows */
/* [object][dual][oleautomation][helpstring][uuid] */ 


EXTERN_C const IID IID_IShellWindows;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85CB6900-4D95-11CF-960C-0080C7F4EE85")
    IShellWindows : public IDispatch
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long __RPC_FAR *Count) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Item( 
            /* [optional][in] */ VARIANT index,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *Folder) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE _NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk) = 0;
        
        virtual /* [hidden][helpstring] */ HRESULT STDMETHODCALLTYPE Register( 
            /* [in] */ IDispatch __RPC_FAR *pid,
            /* [in] */ long hwnd,
            /* [in] */ int swClass,
            /* [out] */ long __RPC_FAR *plCookie) = 0;
        
        virtual /* [hidden][helpstring] */ HRESULT STDMETHODCALLTYPE RegisterPending( 
            /* [in] */ long lThreadId,
            /* [in] */ VARIANT __RPC_FAR *pvarloc,
            /* [in] */ VARIANT __RPC_FAR *pvarlocRoot,
            /* [in] */ int swClass,
            /* [out] */ long __RPC_FAR *plCookie) = 0;
        
        virtual /* [hidden][helpstring] */ HRESULT STDMETHODCALLTYPE Revoke( 
            /* [in] */ long lCookie) = 0;
        
        virtual /* [hidden][helpstring] */ HRESULT STDMETHODCALLTYPE OnNavigate( 
            /* [in] */ long lCookie,
            /* [in] */ VARIANT __RPC_FAR *pvarLoc) = 0;
        
        virtual /* [hidden][helpstring] */ HRESULT STDMETHODCALLTYPE OnActivated( 
            /* [in] */ long lCookie,
            /* [in] */ VARIANT_BOOL fActive) = 0;
        
        virtual /* [hidden][helpstring] */ HRESULT STDMETHODCALLTYPE FindWindow( 
            /* [in] */ VARIANT __RPC_FAR *pvarLoc,
            /* [in] */ VARIANT __RPC_FAR *pvarLocRoot,
            /* [in] */ int swClass,
            /* [out] */ long __RPC_FAR *phwnd,
            /* [in] */ int swfwOptions,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppdispOut) = 0;
        
        virtual /* [hidden][helpstring] */ HRESULT STDMETHODCALLTYPE OnCreated( 
            /* [in] */ long lCookie,
            /* [in] */ IUnknown __RPC_FAR *punk) = 0;
        
        virtual /* [hidden][helpstring] */ HRESULT STDMETHODCALLTYPE ProcessAttachDetach( 
            /* [in] */ VARIANT_BOOL fAttach) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IShellWindowsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IShellWindows __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IShellWindows __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IShellWindows __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IShellWindows __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IShellWindows __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IShellWindows __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IShellWindows __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            IShellWindows __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *Count);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )( 
            IShellWindows __RPC_FAR * This,
            /* [optional][in] */ VARIANT index,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *Folder);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *_NewEnum )( 
            IShellWindows __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);
        
        /* [hidden][helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Register )( 
            IShellWindows __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *pid,
            /* [in] */ long hwnd,
            /* [in] */ int swClass,
            /* [out] */ long __RPC_FAR *plCookie);
        
        /* [hidden][helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RegisterPending )( 
            IShellWindows __RPC_FAR * This,
            /* [in] */ long lThreadId,
            /* [in] */ VARIANT __RPC_FAR *pvarloc,
            /* [in] */ VARIANT __RPC_FAR *pvarlocRoot,
            /* [in] */ int swClass,
            /* [out] */ long __RPC_FAR *plCookie);
        
        /* [hidden][helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Revoke )( 
            IShellWindows __RPC_FAR * This,
            /* [in] */ long lCookie);
        
        /* [hidden][helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnNavigate )( 
            IShellWindows __RPC_FAR * This,
            /* [in] */ long lCookie,
            /* [in] */ VARIANT __RPC_FAR *pvarLoc);
        
        /* [hidden][helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnActivated )( 
            IShellWindows __RPC_FAR * This,
            /* [in] */ long lCookie,
            /* [in] */ VARIANT_BOOL fActive);
        
        /* [hidden][helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindWindow )( 
            IShellWindows __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *pvarLoc,
            /* [in] */ VARIANT __RPC_FAR *pvarLocRoot,
            /* [in] */ int swClass,
            /* [out] */ long __RPC_FAR *phwnd,
            /* [in] */ int swfwOptions,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppdispOut);
        
        /* [hidden][helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnCreated )( 
            IShellWindows __RPC_FAR * This,
            /* [in] */ long lCookie,
            /* [in] */ IUnknown __RPC_FAR *punk);
        
        /* [hidden][helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ProcessAttachDetach )( 
            IShellWindows __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL fAttach);
        
        END_INTERFACE
    } IShellWindowsVtbl;

    interface IShellWindows
    {
        CONST_VTBL struct IShellWindowsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellWindows_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellWindows_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellWindows_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellWindows_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IShellWindows_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IShellWindows_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IShellWindows_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IShellWindows_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define IShellWindows_Item(This,index,Folder)	\
    (This)->lpVtbl -> Item(This,index,Folder)

#define IShellWindows__NewEnum(This,ppunk)	\
    (This)->lpVtbl -> _NewEnum(This,ppunk)

#define IShellWindows_Register(This,pid,hwnd,swClass,plCookie)	\
    (This)->lpVtbl -> Register(This,pid,hwnd,swClass,plCookie)

#define IShellWindows_RegisterPending(This,lThreadId,pvarloc,pvarlocRoot,swClass,plCookie)	\
    (This)->lpVtbl -> RegisterPending(This,lThreadId,pvarloc,pvarlocRoot,swClass,plCookie)

#define IShellWindows_Revoke(This,lCookie)	\
    (This)->lpVtbl -> Revoke(This,lCookie)

#define IShellWindows_OnNavigate(This,lCookie,pvarLoc)	\
    (This)->lpVtbl -> OnNavigate(This,lCookie,pvarLoc)

#define IShellWindows_OnActivated(This,lCookie,fActive)	\
    (This)->lpVtbl -> OnActivated(This,lCookie,fActive)

#define IShellWindows_FindWindow(This,pvarLoc,pvarLocRoot,swClass,phwnd,swfwOptions,ppdispOut)	\
    (This)->lpVtbl -> FindWindow(This,pvarLoc,pvarLocRoot,swClass,phwnd,swfwOptions,ppdispOut)

#define IShellWindows_OnCreated(This,lCookie,punk)	\
    (This)->lpVtbl -> OnCreated(This,lCookie,punk)

#define IShellWindows_ProcessAttachDetach(This,fAttach)	\
    (This)->lpVtbl -> ProcessAttachDetach(This,fAttach)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShellWindows_get_Count_Proxy( 
    IShellWindows __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *Count);


void __RPC_STUB IShellWindows_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IShellWindows_Item_Proxy( 
    IShellWindows __RPC_FAR * This,
    /* [optional][in] */ VARIANT index,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *Folder);


void __RPC_STUB IShellWindows_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IShellWindows__NewEnum_Proxy( 
    IShellWindows __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);


void __RPC_STUB IShellWindows__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][helpstring] */ HRESULT STDMETHODCALLTYPE IShellWindows_Register_Proxy( 
    IShellWindows __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *pid,
    /* [in] */ long hwnd,
    /* [in] */ int swClass,
    /* [out] */ long __RPC_FAR *plCookie);


void __RPC_STUB IShellWindows_Register_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][helpstring] */ HRESULT STDMETHODCALLTYPE IShellWindows_RegisterPending_Proxy( 
    IShellWindows __RPC_FAR * This,
    /* [in] */ long lThreadId,
    /* [in] */ VARIANT __RPC_FAR *pvarloc,
    /* [in] */ VARIANT __RPC_FAR *pvarlocRoot,
    /* [in] */ int swClass,
    /* [out] */ long __RPC_FAR *plCookie);


void __RPC_STUB IShellWindows_RegisterPending_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][helpstring] */ HRESULT STDMETHODCALLTYPE IShellWindows_Revoke_Proxy( 
    IShellWindows __RPC_FAR * This,
    /* [in] */ long lCookie);


void __RPC_STUB IShellWindows_Revoke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][helpstring] */ HRESULT STDMETHODCALLTYPE IShellWindows_OnNavigate_Proxy( 
    IShellWindows __RPC_FAR * This,
    /* [in] */ long lCookie,
    /* [in] */ VARIANT __RPC_FAR *pvarLoc);


void __RPC_STUB IShellWindows_OnNavigate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][helpstring] */ HRESULT STDMETHODCALLTYPE IShellWindows_OnActivated_Proxy( 
    IShellWindows __RPC_FAR * This,
    /* [in] */ long lCookie,
    /* [in] */ VARIANT_BOOL fActive);


void __RPC_STUB IShellWindows_OnActivated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][helpstring] */ HRESULT STDMETHODCALLTYPE IShellWindows_FindWindow_Proxy( 
    IShellWindows __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *pvarLoc,
    /* [in] */ VARIANT __RPC_FAR *pvarLocRoot,
    /* [in] */ int swClass,
    /* [out] */ long __RPC_FAR *phwnd,
    /* [in] */ int swfwOptions,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppdispOut);


void __RPC_STUB IShellWindows_FindWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][helpstring] */ HRESULT STDMETHODCALLTYPE IShellWindows_OnCreated_Proxy( 
    IShellWindows __RPC_FAR * This,
    /* [in] */ long lCookie,
    /* [in] */ IUnknown __RPC_FAR *punk);


void __RPC_STUB IShellWindows_OnCreated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][helpstring] */ HRESULT STDMETHODCALLTYPE IShellWindows_ProcessAttachDetach_Proxy( 
    IShellWindows __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL fAttach);


void __RPC_STUB IShellWindows_ProcessAttachDetach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IShellWindows_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_ShellWindows;

#ifdef __cplusplus

class DECLSPEC_UUID("9BA05972-F6A8-11CF-A442-00A0C90A8F39")
ShellWindows;
#endif

#ifndef __IShellUIHelper_INTERFACE_DEFINED__
#define __IShellUIHelper_INTERFACE_DEFINED__

/* interface IShellUIHelper */
/* [object][dual][oleautomation][helpstring][uuid] */ 


EXTERN_C const IID IID_IShellUIHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("729FE2F8-1EA8-11d1-8F85-00C04FC2FBE1")
    IShellUIHelper : public IDispatch
    {
    public:
        virtual /* [id][hidden] */ HRESULT STDMETHODCALLTYPE ResetFirstBootMode( void) = 0;
        
        virtual /* [id][hidden] */ HRESULT STDMETHODCALLTYPE ResetSafeMode( void) = 0;
        
        virtual /* [id][hidden] */ HRESULT STDMETHODCALLTYPE RefreshOfflineDesktop( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE AddFavorite( 
            /* [in] */ BSTR URL,
            /* [in][optional] */ VARIANT __RPC_FAR *Title) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE AddChannel( 
            /* [in] */ BSTR URL) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE AddDesktopComponent( 
            /* [in] */ BSTR URL,
            /* [in] */ BSTR Type,
            /* [in][optional] */ VARIANT __RPC_FAR *Left,
            /* [in][optional] */ VARIANT __RPC_FAR *Top,
            /* [in][optional] */ VARIANT __RPC_FAR *Width,
            /* [in][optional] */ VARIANT __RPC_FAR *Height) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE IsSubscribed( 
            /* [in] */ BSTR URL,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE NavigateAndFind( 
            /* [in] */ BSTR URL,
            /* [in] */ BSTR strQuery,
            /* [in] */ VARIANT __RPC_FAR *varTargetFrame) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ImportExportFavorites( 
            /* [in] */ VARIANT_BOOL fImport,
            /* [in] */ BSTR strImpExpPath) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE AutoCompleteSaveForm( 
            /* [in][optional] */ VARIANT __RPC_FAR *Form) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE AutoScan( 
            /* [in] */ BSTR strSearch,
            /* [in] */ BSTR strFailureUrl,
            /* [in][optional] */ VARIANT __RPC_FAR *pvarTargetFrame) = 0;
        
        virtual /* [id][hidden] */ HRESULT STDMETHODCALLTYPE AutoCompleteAttach( 
            /* [in][optional] */ VARIANT __RPC_FAR *Reserved) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ShowBrowserUI( 
            /* [in] */ BSTR bstrName,
            /* [in] */ VARIANT __RPC_FAR *pvarIn,
            /* [retval][out] */ VARIANT __RPC_FAR *pvarOut) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IShellUIHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IShellUIHelper __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IShellUIHelper __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IShellUIHelper __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IShellUIHelper __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IShellUIHelper __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IShellUIHelper __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IShellUIHelper __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id][hidden] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResetFirstBootMode )( 
            IShellUIHelper __RPC_FAR * This);
        
        /* [id][hidden] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResetSafeMode )( 
            IShellUIHelper __RPC_FAR * This);
        
        /* [id][hidden] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RefreshOfflineDesktop )( 
            IShellUIHelper __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddFavorite )( 
            IShellUIHelper __RPC_FAR * This,
            /* [in] */ BSTR URL,
            /* [in][optional] */ VARIANT __RPC_FAR *Title);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddChannel )( 
            IShellUIHelper __RPC_FAR * This,
            /* [in] */ BSTR URL);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddDesktopComponent )( 
            IShellUIHelper __RPC_FAR * This,
            /* [in] */ BSTR URL,
            /* [in] */ BSTR Type,
            /* [in][optional] */ VARIANT __RPC_FAR *Left,
            /* [in][optional] */ VARIANT __RPC_FAR *Top,
            /* [in][optional] */ VARIANT __RPC_FAR *Width,
            /* [in][optional] */ VARIANT __RPC_FAR *Height);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsSubscribed )( 
            IShellUIHelper __RPC_FAR * This,
            /* [in] */ BSTR URL,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NavigateAndFind )( 
            IShellUIHelper __RPC_FAR * This,
            /* [in] */ BSTR URL,
            /* [in] */ BSTR strQuery,
            /* [in] */ VARIANT __RPC_FAR *varTargetFrame);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ImportExportFavorites )( 
            IShellUIHelper __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL fImport,
            /* [in] */ BSTR strImpExpPath);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AutoCompleteSaveForm )( 
            IShellUIHelper __RPC_FAR * This,
            /* [in][optional] */ VARIANT __RPC_FAR *Form);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AutoScan )( 
            IShellUIHelper __RPC_FAR * This,
            /* [in] */ BSTR strSearch,
            /* [in] */ BSTR strFailureUrl,
            /* [in][optional] */ VARIANT __RPC_FAR *pvarTargetFrame);
        
        /* [id][hidden] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AutoCompleteAttach )( 
            IShellUIHelper __RPC_FAR * This,
            /* [in][optional] */ VARIANT __RPC_FAR *Reserved);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShowBrowserUI )( 
            IShellUIHelper __RPC_FAR * This,
            /* [in] */ BSTR bstrName,
            /* [in] */ VARIANT __RPC_FAR *pvarIn,
            /* [retval][out] */ VARIANT __RPC_FAR *pvarOut);
        
        END_INTERFACE
    } IShellUIHelperVtbl;

    interface IShellUIHelper
    {
        CONST_VTBL struct IShellUIHelperVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellUIHelper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellUIHelper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellUIHelper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellUIHelper_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IShellUIHelper_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IShellUIHelper_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IShellUIHelper_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IShellUIHelper_ResetFirstBootMode(This)	\
    (This)->lpVtbl -> ResetFirstBootMode(This)

#define IShellUIHelper_ResetSafeMode(This)	\
    (This)->lpVtbl -> ResetSafeMode(This)

#define IShellUIHelper_RefreshOfflineDesktop(This)	\
    (This)->lpVtbl -> RefreshOfflineDesktop(This)

#define IShellUIHelper_AddFavorite(This,URL,Title)	\
    (This)->lpVtbl -> AddFavorite(This,URL,Title)

#define IShellUIHelper_AddChannel(This,URL)	\
    (This)->lpVtbl -> AddChannel(This,URL)

#define IShellUIHelper_AddDesktopComponent(This,URL,Type,Left,Top,Width,Height)	\
    (This)->lpVtbl -> AddDesktopComponent(This,URL,Type,Left,Top,Width,Height)

#define IShellUIHelper_IsSubscribed(This,URL,pBool)	\
    (This)->lpVtbl -> IsSubscribed(This,URL,pBool)

#define IShellUIHelper_NavigateAndFind(This,URL,strQuery,varTargetFrame)	\
    (This)->lpVtbl -> NavigateAndFind(This,URL,strQuery,varTargetFrame)

#define IShellUIHelper_ImportExportFavorites(This,fImport,strImpExpPath)	\
    (This)->lpVtbl -> ImportExportFavorites(This,fImport,strImpExpPath)

#define IShellUIHelper_AutoCompleteSaveForm(This,Form)	\
    (This)->lpVtbl -> AutoCompleteSaveForm(This,Form)

#define IShellUIHelper_AutoScan(This,strSearch,strFailureUrl,pvarTargetFrame)	\
    (This)->lpVtbl -> AutoScan(This,strSearch,strFailureUrl,pvarTargetFrame)

#define IShellUIHelper_AutoCompleteAttach(This,Reserved)	\
    (This)->lpVtbl -> AutoCompleteAttach(This,Reserved)

#define IShellUIHelper_ShowBrowserUI(This,bstrName,pvarIn,pvarOut)	\
    (This)->lpVtbl -> ShowBrowserUI(This,bstrName,pvarIn,pvarOut)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id][hidden] */ HRESULT STDMETHODCALLTYPE IShellUIHelper_ResetFirstBootMode_Proxy( 
    IShellUIHelper __RPC_FAR * This);


void __RPC_STUB IShellUIHelper_ResetFirstBootMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][hidden] */ HRESULT STDMETHODCALLTYPE IShellUIHelper_ResetSafeMode_Proxy( 
    IShellUIHelper __RPC_FAR * This);


void __RPC_STUB IShellUIHelper_ResetSafeMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][hidden] */ HRESULT STDMETHODCALLTYPE IShellUIHelper_RefreshOfflineDesktop_Proxy( 
    IShellUIHelper __RPC_FAR * This);


void __RPC_STUB IShellUIHelper_RefreshOfflineDesktop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IShellUIHelper_AddFavorite_Proxy( 
    IShellUIHelper __RPC_FAR * This,
    /* [in] */ BSTR URL,
    /* [in][optional] */ VARIANT __RPC_FAR *Title);


void __RPC_STUB IShellUIHelper_AddFavorite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IShellUIHelper_AddChannel_Proxy( 
    IShellUIHelper __RPC_FAR * This,
    /* [in] */ BSTR URL);


void __RPC_STUB IShellUIHelper_AddChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IShellUIHelper_AddDesktopComponent_Proxy( 
    IShellUIHelper __RPC_FAR * This,
    /* [in] */ BSTR URL,
    /* [in] */ BSTR Type,
    /* [in][optional] */ VARIANT __RPC_FAR *Left,
    /* [in][optional] */ VARIANT __RPC_FAR *Top,
    /* [in][optional] */ VARIANT __RPC_FAR *Width,
    /* [in][optional] */ VARIANT __RPC_FAR *Height);


void __RPC_STUB IShellUIHelper_AddDesktopComponent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IShellUIHelper_IsSubscribed_Proxy( 
    IShellUIHelper __RPC_FAR * This,
    /* [in] */ BSTR URL,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);


void __RPC_STUB IShellUIHelper_IsSubscribed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IShellUIHelper_NavigateAndFind_Proxy( 
    IShellUIHelper __RPC_FAR * This,
    /* [in] */ BSTR URL,
    /* [in] */ BSTR strQuery,
    /* [in] */ VARIANT __RPC_FAR *varTargetFrame);


void __RPC_STUB IShellUIHelper_NavigateAndFind_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IShellUIHelper_ImportExportFavorites_Proxy( 
    IShellUIHelper __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL fImport,
    /* [in] */ BSTR strImpExpPath);


void __RPC_STUB IShellUIHelper_ImportExportFavorites_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IShellUIHelper_AutoCompleteSaveForm_Proxy( 
    IShellUIHelper __RPC_FAR * This,
    /* [in][optional] */ VARIANT __RPC_FAR *Form);


void __RPC_STUB IShellUIHelper_AutoCompleteSaveForm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IShellUIHelper_AutoScan_Proxy( 
    IShellUIHelper __RPC_FAR * This,
    /* [in] */ BSTR strSearch,
    /* [in] */ BSTR strFailureUrl,
    /* [in][optional] */ VARIANT __RPC_FAR *pvarTargetFrame);


void __RPC_STUB IShellUIHelper_AutoScan_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][hidden] */ HRESULT STDMETHODCALLTYPE IShellUIHelper_AutoCompleteAttach_Proxy( 
    IShellUIHelper __RPC_FAR * This,
    /* [in][optional] */ VARIANT __RPC_FAR *Reserved);


void __RPC_STUB IShellUIHelper_AutoCompleteAttach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IShellUIHelper_ShowBrowserUI_Proxy( 
    IShellUIHelper __RPC_FAR * This,
    /* [in] */ BSTR bstrName,
    /* [in] */ VARIANT __RPC_FAR *pvarIn,
    /* [retval][out] */ VARIANT __RPC_FAR *pvarOut);


void __RPC_STUB IShellUIHelper_ShowBrowserUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IShellUIHelper_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_ShellUIHelper;

#ifdef __cplusplus

class DECLSPEC_UUID("64AB4BB7-111E-11d1-8F79-00C04FC2FBE1")
ShellUIHelper;
#endif

#ifndef ___ShellFavoritesNameSpaceEvents_DISPINTERFACE_DEFINED__
#define ___ShellFavoritesNameSpaceEvents_DISPINTERFACE_DEFINED__

/* dispinterface _ShellFavoritesNameSpaceEvents */
/* [uuid] */ 


EXTERN_C const IID DIID__ShellFavoritesNameSpaceEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("55136806-B2DE-11D1-B9F2-00A0C98BC547")
    _ShellFavoritesNameSpaceEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _ShellFavoritesNameSpaceEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            _ShellFavoritesNameSpaceEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            _ShellFavoritesNameSpaceEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            _ShellFavoritesNameSpaceEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            _ShellFavoritesNameSpaceEvents __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            _ShellFavoritesNameSpaceEvents __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            _ShellFavoritesNameSpaceEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            _ShellFavoritesNameSpaceEvents __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } _ShellFavoritesNameSpaceEventsVtbl;

    interface _ShellFavoritesNameSpaceEvents
    {
        CONST_VTBL struct _ShellFavoritesNameSpaceEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _ShellFavoritesNameSpaceEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _ShellFavoritesNameSpaceEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _ShellFavoritesNameSpaceEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _ShellFavoritesNameSpaceEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _ShellFavoritesNameSpaceEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _ShellFavoritesNameSpaceEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _ShellFavoritesNameSpaceEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___ShellFavoritesNameSpaceEvents_DISPINTERFACE_DEFINED__ */


#ifndef __IShellFavoritesNameSpace_INTERFACE_DEFINED__
#define __IShellFavoritesNameSpace_INTERFACE_DEFINED__

/* interface IShellFavoritesNameSpace */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IShellFavoritesNameSpace;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("55136804-B2DE-11D1-B9F2-00A0C98BC547")
    IShellFavoritesNameSpace : public IDispatch
    {
    public:
        virtual /* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE MoveSelectionUp( void) = 0;
        
        virtual /* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE MoveSelectionDown( void) = 0;
        
        virtual /* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE ResetSort( void) = 0;
        
        virtual /* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE NewFolder( void) = 0;
        
        virtual /* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE Synchronize( void) = 0;
        
        virtual /* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE Import( void) = 0;
        
        virtual /* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE Export( void) = 0;
        
        virtual /* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE InvokeContextMenuCommand( 
            /* [in] */ BSTR strCommand) = 0;
        
        virtual /* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE MoveSelectionTo( void) = 0;
        
        virtual /* [helpstring][id][propget][hidden] */ HRESULT STDMETHODCALLTYPE get_FOfflinePackInstalled( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool) = 0;
        
        virtual /* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE CreateSubscriptionForSelection( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool) = 0;
        
        virtual /* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE DeleteSubscriptionForSelection( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool) = 0;
        
        virtual /* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE SetRoot( 
            /* [in] */ BSTR bstrFullPath) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IShellFavoritesNameSpaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IShellFavoritesNameSpace __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IShellFavoritesNameSpace __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IShellFavoritesNameSpace __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IShellFavoritesNameSpace __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IShellFavoritesNameSpace __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IShellFavoritesNameSpace __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IShellFavoritesNameSpace __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][hidden] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveSelectionUp )( 
            IShellFavoritesNameSpace __RPC_FAR * This);
        
        /* [helpstring][id][hidden] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveSelectionDown )( 
            IShellFavoritesNameSpace __RPC_FAR * This);
        
        /* [helpstring][id][hidden] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResetSort )( 
            IShellFavoritesNameSpace __RPC_FAR * This);
        
        /* [helpstring][id][hidden] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NewFolder )( 
            IShellFavoritesNameSpace __RPC_FAR * This);
        
        /* [helpstring][id][hidden] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Synchronize )( 
            IShellFavoritesNameSpace __RPC_FAR * This);
        
        /* [helpstring][id][hidden] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Import )( 
            IShellFavoritesNameSpace __RPC_FAR * This);
        
        /* [helpstring][id][hidden] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Export )( 
            IShellFavoritesNameSpace __RPC_FAR * This);
        
        /* [helpstring][id][hidden] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InvokeContextMenuCommand )( 
            IShellFavoritesNameSpace __RPC_FAR * This,
            /* [in] */ BSTR strCommand);
        
        /* [helpstring][id][hidden] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveSelectionTo )( 
            IShellFavoritesNameSpace __RPC_FAR * This);
        
        /* [helpstring][id][propget][hidden] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FOfflinePackInstalled )( 
            IShellFavoritesNameSpace __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);
        
        /* [helpstring][id][hidden] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateSubscriptionForSelection )( 
            IShellFavoritesNameSpace __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);
        
        /* [helpstring][id][hidden] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteSubscriptionForSelection )( 
            IShellFavoritesNameSpace __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);
        
        /* [helpstring][id][hidden] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetRoot )( 
            IShellFavoritesNameSpace __RPC_FAR * This,
            /* [in] */ BSTR bstrFullPath);
        
        END_INTERFACE
    } IShellFavoritesNameSpaceVtbl;

    interface IShellFavoritesNameSpace
    {
        CONST_VTBL struct IShellFavoritesNameSpaceVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellFavoritesNameSpace_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellFavoritesNameSpace_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellFavoritesNameSpace_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellFavoritesNameSpace_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IShellFavoritesNameSpace_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IShellFavoritesNameSpace_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IShellFavoritesNameSpace_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IShellFavoritesNameSpace_MoveSelectionUp(This)	\
    (This)->lpVtbl -> MoveSelectionUp(This)

#define IShellFavoritesNameSpace_MoveSelectionDown(This)	\
    (This)->lpVtbl -> MoveSelectionDown(This)

#define IShellFavoritesNameSpace_ResetSort(This)	\
    (This)->lpVtbl -> ResetSort(This)

#define IShellFavoritesNameSpace_NewFolder(This)	\
    (This)->lpVtbl -> NewFolder(This)

#define IShellFavoritesNameSpace_Synchronize(This)	\
    (This)->lpVtbl -> Synchronize(This)

#define IShellFavoritesNameSpace_Import(This)	\
    (This)->lpVtbl -> Import(This)

#define IShellFavoritesNameSpace_Export(This)	\
    (This)->lpVtbl -> Export(This)

#define IShellFavoritesNameSpace_InvokeContextMenuCommand(This,strCommand)	\
    (This)->lpVtbl -> InvokeContextMenuCommand(This,strCommand)

#define IShellFavoritesNameSpace_MoveSelectionTo(This)	\
    (This)->lpVtbl -> MoveSelectionTo(This)

#define IShellFavoritesNameSpace_get_FOfflinePackInstalled(This,pBool)	\
    (This)->lpVtbl -> get_FOfflinePackInstalled(This,pBool)

#define IShellFavoritesNameSpace_CreateSubscriptionForSelection(This,pBool)	\
    (This)->lpVtbl -> CreateSubscriptionForSelection(This,pBool)

#define IShellFavoritesNameSpace_DeleteSubscriptionForSelection(This,pBool)	\
    (This)->lpVtbl -> DeleteSubscriptionForSelection(This,pBool)

#define IShellFavoritesNameSpace_SetRoot(This,bstrFullPath)	\
    (This)->lpVtbl -> SetRoot(This,bstrFullPath)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE IShellFavoritesNameSpace_MoveSelectionUp_Proxy( 
    IShellFavoritesNameSpace __RPC_FAR * This);


void __RPC_STUB IShellFavoritesNameSpace_MoveSelectionUp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE IShellFavoritesNameSpace_MoveSelectionDown_Proxy( 
    IShellFavoritesNameSpace __RPC_FAR * This);


void __RPC_STUB IShellFavoritesNameSpace_MoveSelectionDown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE IShellFavoritesNameSpace_ResetSort_Proxy( 
    IShellFavoritesNameSpace __RPC_FAR * This);


void __RPC_STUB IShellFavoritesNameSpace_ResetSort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE IShellFavoritesNameSpace_NewFolder_Proxy( 
    IShellFavoritesNameSpace __RPC_FAR * This);


void __RPC_STUB IShellFavoritesNameSpace_NewFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE IShellFavoritesNameSpace_Synchronize_Proxy( 
    IShellFavoritesNameSpace __RPC_FAR * This);


void __RPC_STUB IShellFavoritesNameSpace_Synchronize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE IShellFavoritesNameSpace_Import_Proxy( 
    IShellFavoritesNameSpace __RPC_FAR * This);


void __RPC_STUB IShellFavoritesNameSpace_Import_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE IShellFavoritesNameSpace_Export_Proxy( 
    IShellFavoritesNameSpace __RPC_FAR * This);


void __RPC_STUB IShellFavoritesNameSpace_Export_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE IShellFavoritesNameSpace_InvokeContextMenuCommand_Proxy( 
    IShellFavoritesNameSpace __RPC_FAR * This,
    /* [in] */ BSTR strCommand);


void __RPC_STUB IShellFavoritesNameSpace_InvokeContextMenuCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE IShellFavoritesNameSpace_MoveSelectionTo_Proxy( 
    IShellFavoritesNameSpace __RPC_FAR * This);


void __RPC_STUB IShellFavoritesNameSpace_MoveSelectionTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget][hidden] */ HRESULT STDMETHODCALLTYPE IShellFavoritesNameSpace_get_FOfflinePackInstalled_Proxy( 
    IShellFavoritesNameSpace __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);


void __RPC_STUB IShellFavoritesNameSpace_get_FOfflinePackInstalled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE IShellFavoritesNameSpace_CreateSubscriptionForSelection_Proxy( 
    IShellFavoritesNameSpace __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);


void __RPC_STUB IShellFavoritesNameSpace_CreateSubscriptionForSelection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE IShellFavoritesNameSpace_DeleteSubscriptionForSelection_Proxy( 
    IShellFavoritesNameSpace __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pBool);


void __RPC_STUB IShellFavoritesNameSpace_DeleteSubscriptionForSelection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][hidden] */ HRESULT STDMETHODCALLTYPE IShellFavoritesNameSpace_SetRoot_Proxy( 
    IShellFavoritesNameSpace __RPC_FAR * This,
    /* [in] */ BSTR bstrFullPath);


void __RPC_STUB IShellFavoritesNameSpace_SetRoot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IShellFavoritesNameSpace_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_ShellFavoritesNameSpace;

#ifdef __cplusplus

class DECLSPEC_UUID("55136805-B2DE-11D1-B9F2-00A0C98BC547")
ShellFavoritesNameSpace;
#endif

#ifndef __IScriptErrorList_INTERFACE_DEFINED__
#define __IScriptErrorList_INTERFACE_DEFINED__

/* interface IScriptErrorList */
/* [object][dual][hidden][oleautomation][helpstring][uuid] */ 


EXTERN_C const IID IID_IScriptErrorList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F3470F24-15FD-11d2-BB2E-00805FF7EFCA")
    IScriptErrorList : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE advanceError( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE retreatError( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE canAdvanceError( 
            /* [retval][out] */ BOOL __RPC_FAR *pfCanAdvance) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE canRetreatError( 
            /* [retval][out] */ BOOL __RPC_FAR *pfCanRetreat) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getErrorLine( 
            /* [retval][out] */ LONG __RPC_FAR *plLine) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getErrorChar( 
            /* [retval][out] */ LONG __RPC_FAR *plChar) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getErrorCode( 
            /* [retval][out] */ LONG __RPC_FAR *plCode) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getErrorMsg( 
            /* [retval][out] */ BSTR __RPC_FAR *pstr) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getErrorUrl( 
            /* [retval][out] */ BSTR __RPC_FAR *pstr) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getAlwaysShowLockState( 
            /* [retval][out] */ BOOL __RPC_FAR *pfAlwaysShowLocked) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getDetailsPaneOpen( 
            /* [retval][out] */ BOOL __RPC_FAR *pfDetailsPaneOpen) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE setDetailsPaneOpen( 
            BOOL fDetailsPaneOpen) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getPerErrorDisplay( 
            /* [retval][out] */ BOOL __RPC_FAR *pfPerErrorDisplay) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE setPerErrorDisplay( 
            BOOL fPerErrorDisplay) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IScriptErrorListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IScriptErrorList __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IScriptErrorList __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IScriptErrorList __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IScriptErrorList __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IScriptErrorList __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IScriptErrorList __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IScriptErrorList __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *advanceError )( 
            IScriptErrorList __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *retreatError )( 
            IScriptErrorList __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *canAdvanceError )( 
            IScriptErrorList __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pfCanAdvance);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *canRetreatError )( 
            IScriptErrorList __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pfCanRetreat);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getErrorLine )( 
            IScriptErrorList __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *plLine);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getErrorChar )( 
            IScriptErrorList __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *plChar);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getErrorCode )( 
            IScriptErrorList __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *plCode);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getErrorMsg )( 
            IScriptErrorList __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pstr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getErrorUrl )( 
            IScriptErrorList __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pstr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getAlwaysShowLockState )( 
            IScriptErrorList __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pfAlwaysShowLocked);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getDetailsPaneOpen )( 
            IScriptErrorList __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pfDetailsPaneOpen);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setDetailsPaneOpen )( 
            IScriptErrorList __RPC_FAR * This,
            BOOL fDetailsPaneOpen);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getPerErrorDisplay )( 
            IScriptErrorList __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pfPerErrorDisplay);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setPerErrorDisplay )( 
            IScriptErrorList __RPC_FAR * This,
            BOOL fPerErrorDisplay);
        
        END_INTERFACE
    } IScriptErrorListVtbl;

    interface IScriptErrorList
    {
        CONST_VTBL struct IScriptErrorListVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IScriptErrorList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScriptErrorList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScriptErrorList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScriptErrorList_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IScriptErrorList_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IScriptErrorList_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IScriptErrorList_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IScriptErrorList_advanceError(This)	\
    (This)->lpVtbl -> advanceError(This)

#define IScriptErrorList_retreatError(This)	\
    (This)->lpVtbl -> retreatError(This)

#define IScriptErrorList_canAdvanceError(This,pfCanAdvance)	\
    (This)->lpVtbl -> canAdvanceError(This,pfCanAdvance)

#define IScriptErrorList_canRetreatError(This,pfCanRetreat)	\
    (This)->lpVtbl -> canRetreatError(This,pfCanRetreat)

#define IScriptErrorList_getErrorLine(This,plLine)	\
    (This)->lpVtbl -> getErrorLine(This,plLine)

#define IScriptErrorList_getErrorChar(This,plChar)	\
    (This)->lpVtbl -> getErrorChar(This,plChar)

#define IScriptErrorList_getErrorCode(This,plCode)	\
    (This)->lpVtbl -> getErrorCode(This,plCode)

#define IScriptErrorList_getErrorMsg(This,pstr)	\
    (This)->lpVtbl -> getErrorMsg(This,pstr)

#define IScriptErrorList_getErrorUrl(This,pstr)	\
    (This)->lpVtbl -> getErrorUrl(This,pstr)

#define IScriptErrorList_getAlwaysShowLockState(This,pfAlwaysShowLocked)	\
    (This)->lpVtbl -> getAlwaysShowLockState(This,pfAlwaysShowLocked)

#define IScriptErrorList_getDetailsPaneOpen(This,pfDetailsPaneOpen)	\
    (This)->lpVtbl -> getDetailsPaneOpen(This,pfDetailsPaneOpen)

#define IScriptErrorList_setDetailsPaneOpen(This,fDetailsPaneOpen)	\
    (This)->lpVtbl -> setDetailsPaneOpen(This,fDetailsPaneOpen)

#define IScriptErrorList_getPerErrorDisplay(This,pfPerErrorDisplay)	\
    (This)->lpVtbl -> getPerErrorDisplay(This,pfPerErrorDisplay)

#define IScriptErrorList_setPerErrorDisplay(This,fPerErrorDisplay)	\
    (This)->lpVtbl -> setPerErrorDisplay(This,fPerErrorDisplay)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id] */ HRESULT STDMETHODCALLTYPE IScriptErrorList_advanceError_Proxy( 
    IScriptErrorList __RPC_FAR * This);


void __RPC_STUB IScriptErrorList_advanceError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IScriptErrorList_retreatError_Proxy( 
    IScriptErrorList __RPC_FAR * This);


void __RPC_STUB IScriptErrorList_retreatError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IScriptErrorList_canAdvanceError_Proxy( 
    IScriptErrorList __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pfCanAdvance);


void __RPC_STUB IScriptErrorList_canAdvanceError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IScriptErrorList_canRetreatError_Proxy( 
    IScriptErrorList __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pfCanRetreat);


void __RPC_STUB IScriptErrorList_canRetreatError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IScriptErrorList_getErrorLine_Proxy( 
    IScriptErrorList __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *plLine);


void __RPC_STUB IScriptErrorList_getErrorLine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IScriptErrorList_getErrorChar_Proxy( 
    IScriptErrorList __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *plChar);


void __RPC_STUB IScriptErrorList_getErrorChar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IScriptErrorList_getErrorCode_Proxy( 
    IScriptErrorList __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *plCode);


void __RPC_STUB IScriptErrorList_getErrorCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IScriptErrorList_getErrorMsg_Proxy( 
    IScriptErrorList __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pstr);


void __RPC_STUB IScriptErrorList_getErrorMsg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IScriptErrorList_getErrorUrl_Proxy( 
    IScriptErrorList __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pstr);


void __RPC_STUB IScriptErrorList_getErrorUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IScriptErrorList_getAlwaysShowLockState_Proxy( 
    IScriptErrorList __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pfAlwaysShowLocked);


void __RPC_STUB IScriptErrorList_getAlwaysShowLockState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IScriptErrorList_getDetailsPaneOpen_Proxy( 
    IScriptErrorList __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pfDetailsPaneOpen);


void __RPC_STUB IScriptErrorList_getDetailsPaneOpen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IScriptErrorList_setDetailsPaneOpen_Proxy( 
    IScriptErrorList __RPC_FAR * This,
    BOOL fDetailsPaneOpen);


void __RPC_STUB IScriptErrorList_setDetailsPaneOpen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IScriptErrorList_getPerErrorDisplay_Proxy( 
    IScriptErrorList __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pfPerErrorDisplay);


void __RPC_STUB IScriptErrorList_getPerErrorDisplay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IScriptErrorList_setPerErrorDisplay_Proxy( 
    IScriptErrorList __RPC_FAR * This,
    BOOL fPerErrorDisplay);


void __RPC_STUB IScriptErrorList_setPerErrorDisplay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IScriptErrorList_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_CScriptErrorList;

#ifdef __cplusplus

class DECLSPEC_UUID("EFD01300-160F-11d2-BB2E-00805FF7EFCA")
CScriptErrorList;
#endif

#ifndef __ISearch_INTERFACE_DEFINED__
#define __ISearch_INTERFACE_DEFINED__

/* interface ISearch */
/* [object][dual][oleautomation][hidden][helpstring][uuid] */ 


EXTERN_C const IID IID_ISearch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ba9239a4-3dd5-11d2-bf8b-00c04fb93661")
    ISearch : public IDispatch
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Title( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrTitle) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Id( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrId) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Url( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrUrl) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISearchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISearch __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISearch __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISearch __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISearch __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISearch __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISearch __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISearch __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Title )( 
            ISearch __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrTitle);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Id )( 
            ISearch __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrId);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Url )( 
            ISearch __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrUrl);
        
        END_INTERFACE
    } ISearchVtbl;

    interface ISearch
    {
        CONST_VTBL struct ISearchVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISearch_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISearch_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISearch_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISearch_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISearch_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISearch_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISearch_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISearch_get_Title(This,pbstrTitle)	\
    (This)->lpVtbl -> get_Title(This,pbstrTitle)

#define ISearch_get_Id(This,pbstrId)	\
    (This)->lpVtbl -> get_Id(This,pbstrId)

#define ISearch_get_Url(This,pbstrUrl)	\
    (This)->lpVtbl -> get_Url(This,pbstrUrl)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE ISearch_get_Title_Proxy( 
    ISearch __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrTitle);


void __RPC_STUB ISearch_get_Title_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE ISearch_get_Id_Proxy( 
    ISearch __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrId);


void __RPC_STUB ISearch_get_Id_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE ISearch_get_Url_Proxy( 
    ISearch __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrUrl);


void __RPC_STUB ISearch_get_Url_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISearch_INTERFACE_DEFINED__ */


#ifndef __ISearches_INTERFACE_DEFINED__
#define __ISearches_INTERFACE_DEFINED__

/* interface ISearches */
/* [object][dual][oleautomation][hidden][helpstring][uuid] */ 


EXTERN_C const IID IID_ISearches;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("47c922a2-3dd5-11d2-bf8b-00c04fb93661")
    ISearches : public IDispatch
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long __RPC_FAR *plCount) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Default( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrDefault) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Item( 
            /* [optional][in] */ VARIANT index,
            /* [retval][out] */ ISearch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE _NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISearchesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISearches __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISearches __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISearches __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISearches __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISearches __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISearches __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISearches __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            ISearches __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plCount);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Default )( 
            ISearches __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrDefault);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )( 
            ISearches __RPC_FAR * This,
            /* [optional][in] */ VARIANT index,
            /* [retval][out] */ ISearch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *_NewEnum )( 
            ISearches __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);
        
        END_INTERFACE
    } ISearchesVtbl;

    interface ISearches
    {
        CONST_VTBL struct ISearchesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISearches_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISearches_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISearches_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISearches_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISearches_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISearches_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISearches_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISearches_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISearches_get_Default(This,pbstrDefault)	\
    (This)->lpVtbl -> get_Default(This,pbstrDefault)

#define ISearches_Item(This,index,ppid)	\
    (This)->lpVtbl -> Item(This,index,ppid)

#define ISearches__NewEnum(This,ppunk)	\
    (This)->lpVtbl -> _NewEnum(This,ppunk)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE ISearches_get_Count_Proxy( 
    ISearches __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plCount);


void __RPC_STUB ISearches_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE ISearches_get_Default_Proxy( 
    ISearches __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrDefault);


void __RPC_STUB ISearches_get_Default_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ISearches_Item_Proxy( 
    ISearches __RPC_FAR * This,
    /* [optional][in] */ VARIANT index,
    /* [retval][out] */ ISearch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB ISearches_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISearches__NewEnum_Proxy( 
    ISearches __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);


void __RPC_STUB ISearches__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISearches_INTERFACE_DEFINED__ */


#ifndef __ISearchAssistantOC_INTERFACE_DEFINED__
#define __ISearchAssistantOC_INTERFACE_DEFINED__

/* interface ISearchAssistantOC */
/* [unique][helpstring][dual][hidden][uuid][object] */ 


EXTERN_C const IID IID_ISearchAssistantOC;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("72423E8F-8011-11d2-BE79-00A0C9A83DA1")
    ISearchAssistantOC : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE AddNextMenuItem( 
            /* [in] */ BSTR bstrText,
            /* [in] */ long idItem) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SetDefaultSearchUrl( 
            /* [in] */ BSTR bstrUrl) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE NavigateToDefaultSearch( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE IsRestricted( 
            /* [in] */ BSTR bstrGuid,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShellFeaturesEnabled( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SearchAssistantDefault( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Searches( 
            /* [retval][out] */ ISearches __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_InWebFolder( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE PutProperty( 
            /* [in] */ VARIANT_BOOL bPerLocale,
            /* [in] */ BSTR bstrName,
            /* [in] */ BSTR bstrValue) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetProperty( 
            /* [in] */ VARIANT_BOOL bPerLocale,
            /* [in] */ BSTR bstrName,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrValue) = 0;
        
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_EventHandled( 
            /* [in] */ VARIANT_BOOL bHandled) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ResetNextMenu( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE FindOnWeb( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE FindFilesOrFolders( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE FindComputer( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE FindPrinter( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE FindPeople( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetSearchAssistantURL( 
            /* [in] */ VARIANT_BOOL bSubstitute,
            /* [in] */ VARIANT_BOOL bCustomize,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrValue) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE NotifySearchSettingsChanged( void) = 0;
        
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_ASProvider( 
            /* [in] */ BSTR Provider) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_ASProvider( 
            /* [retval][out] */ BSTR __RPC_FAR *pProvider) = 0;
        
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_ASSetting( 
            /* [in] */ int Setting) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_ASSetting( 
            /* [retval][out] */ int __RPC_FAR *pSetting) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE NETDetectNextNavigate( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE PutFindText( 
            /* [in] */ BSTR FindText) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_Version( 
            /* [retval][out] */ int __RPC_FAR *pVersion) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE EncodeString( 
            /* [in] */ BSTR bstrValue,
            /* [in] */ BSTR bstrCharSet,
            /* [in] */ VARIANT_BOOL bUseUTF8,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISearchAssistantOCVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISearchAssistantOC __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISearchAssistantOC __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddNextMenuItem )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [in] */ BSTR bstrText,
            /* [in] */ long idItem);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDefaultSearchUrl )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [in] */ BSTR bstrUrl);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NavigateToDefaultSearch )( 
            ISearchAssistantOC __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsRestricted )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [in] */ BSTR bstrGuid,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShellFeaturesEnabled )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SearchAssistantDefault )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Searches )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [retval][out] */ ISearches __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_InWebFolder )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PutProperty )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bPerLocale,
            /* [in] */ BSTR bstrName,
            /* [in] */ BSTR bstrValue);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProperty )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bPerLocale,
            /* [in] */ BSTR bstrName,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrValue);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_EventHandled )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bHandled);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResetNextMenu )( 
            ISearchAssistantOC __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindOnWeb )( 
            ISearchAssistantOC __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindFilesOrFolders )( 
            ISearchAssistantOC __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindComputer )( 
            ISearchAssistantOC __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindPrinter )( 
            ISearchAssistantOC __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindPeople )( 
            ISearchAssistantOC __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSearchAssistantURL )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bSubstitute,
            /* [in] */ VARIANT_BOOL bCustomize,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrValue);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NotifySearchSettingsChanged )( 
            ISearchAssistantOC __RPC_FAR * This);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ASProvider )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [in] */ BSTR Provider);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ASProvider )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pProvider);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ASSetting )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [in] */ int Setting);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ASSetting )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [retval][out] */ int __RPC_FAR *pSetting);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NETDetectNextNavigate )( 
            ISearchAssistantOC __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PutFindText )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [in] */ BSTR FindText);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Version )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [retval][out] */ int __RPC_FAR *pVersion);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EncodeString )( 
            ISearchAssistantOC __RPC_FAR * This,
            /* [in] */ BSTR bstrValue,
            /* [in] */ BSTR bstrCharSet,
            /* [in] */ VARIANT_BOOL bUseUTF8,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrResult);
        
        END_INTERFACE
    } ISearchAssistantOCVtbl;

    interface ISearchAssistantOC
    {
        CONST_VTBL struct ISearchAssistantOCVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISearchAssistantOC_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISearchAssistantOC_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISearchAssistantOC_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISearchAssistantOC_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISearchAssistantOC_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISearchAssistantOC_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISearchAssistantOC_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISearchAssistantOC_AddNextMenuItem(This,bstrText,idItem)	\
    (This)->lpVtbl -> AddNextMenuItem(This,bstrText,idItem)

#define ISearchAssistantOC_SetDefaultSearchUrl(This,bstrUrl)	\
    (This)->lpVtbl -> SetDefaultSearchUrl(This,bstrUrl)

#define ISearchAssistantOC_NavigateToDefaultSearch(This)	\
    (This)->lpVtbl -> NavigateToDefaultSearch(This)

#define ISearchAssistantOC_IsRestricted(This,bstrGuid,pVal)	\
    (This)->lpVtbl -> IsRestricted(This,bstrGuid,pVal)

#define ISearchAssistantOC_get_ShellFeaturesEnabled(This,pVal)	\
    (This)->lpVtbl -> get_ShellFeaturesEnabled(This,pVal)

#define ISearchAssistantOC_get_SearchAssistantDefault(This,pVal)	\
    (This)->lpVtbl -> get_SearchAssistantDefault(This,pVal)

#define ISearchAssistantOC_get_Searches(This,ppid)	\
    (This)->lpVtbl -> get_Searches(This,ppid)

#define ISearchAssistantOC_get_InWebFolder(This,pVal)	\
    (This)->lpVtbl -> get_InWebFolder(This,pVal)

#define ISearchAssistantOC_PutProperty(This,bPerLocale,bstrName,bstrValue)	\
    (This)->lpVtbl -> PutProperty(This,bPerLocale,bstrName,bstrValue)

#define ISearchAssistantOC_GetProperty(This,bPerLocale,bstrName,pbstrValue)	\
    (This)->lpVtbl -> GetProperty(This,bPerLocale,bstrName,pbstrValue)

#define ISearchAssistantOC_put_EventHandled(This,bHandled)	\
    (This)->lpVtbl -> put_EventHandled(This,bHandled)

#define ISearchAssistantOC_ResetNextMenu(This)	\
    (This)->lpVtbl -> ResetNextMenu(This)

#define ISearchAssistantOC_FindOnWeb(This)	\
    (This)->lpVtbl -> FindOnWeb(This)

#define ISearchAssistantOC_FindFilesOrFolders(This)	\
    (This)->lpVtbl -> FindFilesOrFolders(This)

#define ISearchAssistantOC_FindComputer(This)	\
    (This)->lpVtbl -> FindComputer(This)

#define ISearchAssistantOC_FindPrinter(This)	\
    (This)->lpVtbl -> FindPrinter(This)

#define ISearchAssistantOC_FindPeople(This)	\
    (This)->lpVtbl -> FindPeople(This)

#define ISearchAssistantOC_GetSearchAssistantURL(This,bSubstitute,bCustomize,pbstrValue)	\
    (This)->lpVtbl -> GetSearchAssistantURL(This,bSubstitute,bCustomize,pbstrValue)

#define ISearchAssistantOC_NotifySearchSettingsChanged(This)	\
    (This)->lpVtbl -> NotifySearchSettingsChanged(This)

#define ISearchAssistantOC_put_ASProvider(This,Provider)	\
    (This)->lpVtbl -> put_ASProvider(This,Provider)

#define ISearchAssistantOC_get_ASProvider(This,pProvider)	\
    (This)->lpVtbl -> get_ASProvider(This,pProvider)

#define ISearchAssistantOC_put_ASSetting(This,Setting)	\
    (This)->lpVtbl -> put_ASSetting(This,Setting)

#define ISearchAssistantOC_get_ASSetting(This,pSetting)	\
    (This)->lpVtbl -> get_ASSetting(This,pSetting)

#define ISearchAssistantOC_NETDetectNextNavigate(This)	\
    (This)->lpVtbl -> NETDetectNextNavigate(This)

#define ISearchAssistantOC_PutFindText(This,FindText)	\
    (This)->lpVtbl -> PutFindText(This,FindText)

#define ISearchAssistantOC_get_Version(This,pVersion)	\
    (This)->lpVtbl -> get_Version(This,pVersion)

#define ISearchAssistantOC_EncodeString(This,bstrValue,bstrCharSet,bUseUTF8,pbstrResult)	\
    (This)->lpVtbl -> EncodeString(This,bstrValue,bstrCharSet,bUseUTF8,pbstrResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_AddNextMenuItem_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [in] */ BSTR bstrText,
    /* [in] */ long idItem);


void __RPC_STUB ISearchAssistantOC_AddNextMenuItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_SetDefaultSearchUrl_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [in] */ BSTR bstrUrl);


void __RPC_STUB ISearchAssistantOC_SetDefaultSearchUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_NavigateToDefaultSearch_Proxy( 
    ISearchAssistantOC __RPC_FAR * This);


void __RPC_STUB ISearchAssistantOC_NavigateToDefaultSearch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_IsRestricted_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [in] */ BSTR bstrGuid,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB ISearchAssistantOC_IsRestricted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_get_ShellFeaturesEnabled_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB ISearchAssistantOC_get_ShellFeaturesEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_get_SearchAssistantDefault_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB ISearchAssistantOC_get_SearchAssistantDefault_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_get_Searches_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [retval][out] */ ISearches __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB ISearchAssistantOC_get_Searches_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_get_InWebFolder_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB ISearchAssistantOC_get_InWebFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_PutProperty_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL bPerLocale,
    /* [in] */ BSTR bstrName,
    /* [in] */ BSTR bstrValue);


void __RPC_STUB ISearchAssistantOC_PutProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_GetProperty_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL bPerLocale,
    /* [in] */ BSTR bstrName,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrValue);


void __RPC_STUB ISearchAssistantOC_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propput] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_put_EventHandled_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL bHandled);


void __RPC_STUB ISearchAssistantOC_put_EventHandled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_ResetNextMenu_Proxy( 
    ISearchAssistantOC __RPC_FAR * This);


void __RPC_STUB ISearchAssistantOC_ResetNextMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_FindOnWeb_Proxy( 
    ISearchAssistantOC __RPC_FAR * This);


void __RPC_STUB ISearchAssistantOC_FindOnWeb_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_FindFilesOrFolders_Proxy( 
    ISearchAssistantOC __RPC_FAR * This);


void __RPC_STUB ISearchAssistantOC_FindFilesOrFolders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_FindComputer_Proxy( 
    ISearchAssistantOC __RPC_FAR * This);


void __RPC_STUB ISearchAssistantOC_FindComputer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_FindPrinter_Proxy( 
    ISearchAssistantOC __RPC_FAR * This);


void __RPC_STUB ISearchAssistantOC_FindPrinter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_FindPeople_Proxy( 
    ISearchAssistantOC __RPC_FAR * This);


void __RPC_STUB ISearchAssistantOC_FindPeople_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_GetSearchAssistantURL_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL bSubstitute,
    /* [in] */ VARIANT_BOOL bCustomize,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrValue);


void __RPC_STUB ISearchAssistantOC_GetSearchAssistantURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_NotifySearchSettingsChanged_Proxy( 
    ISearchAssistantOC __RPC_FAR * This);


void __RPC_STUB ISearchAssistantOC_NotifySearchSettingsChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propput] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_put_ASProvider_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [in] */ BSTR Provider);


void __RPC_STUB ISearchAssistantOC_put_ASProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propget] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_get_ASProvider_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pProvider);


void __RPC_STUB ISearchAssistantOC_get_ASProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propput] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_put_ASSetting_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [in] */ int Setting);


void __RPC_STUB ISearchAssistantOC_put_ASSetting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propget] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_get_ASSetting_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [retval][out] */ int __RPC_FAR *pSetting);


void __RPC_STUB ISearchAssistantOC_get_ASSetting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_NETDetectNextNavigate_Proxy( 
    ISearchAssistantOC __RPC_FAR * This);


void __RPC_STUB ISearchAssistantOC_NETDetectNextNavigate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_PutFindText_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [in] */ BSTR FindText);


void __RPC_STUB ISearchAssistantOC_PutFindText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propget] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_get_Version_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [retval][out] */ int __RPC_FAR *pVersion);


void __RPC_STUB ISearchAssistantOC_get_Version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC_EncodeString_Proxy( 
    ISearchAssistantOC __RPC_FAR * This,
    /* [in] */ BSTR bstrValue,
    /* [in] */ BSTR bstrCharSet,
    /* [in] */ VARIANT_BOOL bUseUTF8,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrResult);


void __RPC_STUB ISearchAssistantOC_EncodeString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISearchAssistantOC_INTERFACE_DEFINED__ */


#ifndef __ISearchAssistantOC2_INTERFACE_DEFINED__
#define __ISearchAssistantOC2_INTERFACE_DEFINED__

/* interface ISearchAssistantOC2 */
/* [unique][helpstring][dual][hidden][uuid][object] */ 


EXTERN_C const IID IID_ISearchAssistantOC2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("72423E8F-8011-11d2-BE79-00A0C9A83DA2")
    ISearchAssistantOC2 : public ISearchAssistantOC
    {
    public:
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowFindPrinter( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbShowFindPrinter) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISearchAssistantOC2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISearchAssistantOC2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISearchAssistantOC2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddNextMenuItem )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [in] */ BSTR bstrText,
            /* [in] */ long idItem);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDefaultSearchUrl )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [in] */ BSTR bstrUrl);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NavigateToDefaultSearch )( 
            ISearchAssistantOC2 __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsRestricted )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [in] */ BSTR bstrGuid,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShellFeaturesEnabled )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SearchAssistantDefault )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Searches )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [retval][out] */ ISearches __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_InWebFolder )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PutProperty )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bPerLocale,
            /* [in] */ BSTR bstrName,
            /* [in] */ BSTR bstrValue);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProperty )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bPerLocale,
            /* [in] */ BSTR bstrName,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrValue);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_EventHandled )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bHandled);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResetNextMenu )( 
            ISearchAssistantOC2 __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindOnWeb )( 
            ISearchAssistantOC2 __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindFilesOrFolders )( 
            ISearchAssistantOC2 __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindComputer )( 
            ISearchAssistantOC2 __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindPrinter )( 
            ISearchAssistantOC2 __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindPeople )( 
            ISearchAssistantOC2 __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSearchAssistantURL )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bSubstitute,
            /* [in] */ VARIANT_BOOL bCustomize,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrValue);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NotifySearchSettingsChanged )( 
            ISearchAssistantOC2 __RPC_FAR * This);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ASProvider )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [in] */ BSTR Provider);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ASProvider )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pProvider);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ASSetting )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [in] */ int Setting);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ASSetting )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [retval][out] */ int __RPC_FAR *pSetting);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NETDetectNextNavigate )( 
            ISearchAssistantOC2 __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PutFindText )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [in] */ BSTR FindText);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Version )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [retval][out] */ int __RPC_FAR *pVersion);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EncodeString )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [in] */ BSTR bstrValue,
            /* [in] */ BSTR bstrCharSet,
            /* [in] */ VARIANT_BOOL bUseUTF8,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrResult);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShowFindPrinter )( 
            ISearchAssistantOC2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbShowFindPrinter);
        
        END_INTERFACE
    } ISearchAssistantOC2Vtbl;

    interface ISearchAssistantOC2
    {
        CONST_VTBL struct ISearchAssistantOC2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISearchAssistantOC2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISearchAssistantOC2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISearchAssistantOC2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISearchAssistantOC2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISearchAssistantOC2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISearchAssistantOC2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISearchAssistantOC2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISearchAssistantOC2_AddNextMenuItem(This,bstrText,idItem)	\
    (This)->lpVtbl -> AddNextMenuItem(This,bstrText,idItem)

#define ISearchAssistantOC2_SetDefaultSearchUrl(This,bstrUrl)	\
    (This)->lpVtbl -> SetDefaultSearchUrl(This,bstrUrl)

#define ISearchAssistantOC2_NavigateToDefaultSearch(This)	\
    (This)->lpVtbl -> NavigateToDefaultSearch(This)

#define ISearchAssistantOC2_IsRestricted(This,bstrGuid,pVal)	\
    (This)->lpVtbl -> IsRestricted(This,bstrGuid,pVal)

#define ISearchAssistantOC2_get_ShellFeaturesEnabled(This,pVal)	\
    (This)->lpVtbl -> get_ShellFeaturesEnabled(This,pVal)

#define ISearchAssistantOC2_get_SearchAssistantDefault(This,pVal)	\
    (This)->lpVtbl -> get_SearchAssistantDefault(This,pVal)

#define ISearchAssistantOC2_get_Searches(This,ppid)	\
    (This)->lpVtbl -> get_Searches(This,ppid)

#define ISearchAssistantOC2_get_InWebFolder(This,pVal)	\
    (This)->lpVtbl -> get_InWebFolder(This,pVal)

#define ISearchAssistantOC2_PutProperty(This,bPerLocale,bstrName,bstrValue)	\
    (This)->lpVtbl -> PutProperty(This,bPerLocale,bstrName,bstrValue)

#define ISearchAssistantOC2_GetProperty(This,bPerLocale,bstrName,pbstrValue)	\
    (This)->lpVtbl -> GetProperty(This,bPerLocale,bstrName,pbstrValue)

#define ISearchAssistantOC2_put_EventHandled(This,bHandled)	\
    (This)->lpVtbl -> put_EventHandled(This,bHandled)

#define ISearchAssistantOC2_ResetNextMenu(This)	\
    (This)->lpVtbl -> ResetNextMenu(This)

#define ISearchAssistantOC2_FindOnWeb(This)	\
    (This)->lpVtbl -> FindOnWeb(This)

#define ISearchAssistantOC2_FindFilesOrFolders(This)	\
    (This)->lpVtbl -> FindFilesOrFolders(This)

#define ISearchAssistantOC2_FindComputer(This)	\
    (This)->lpVtbl -> FindComputer(This)

#define ISearchAssistantOC2_FindPrinter(This)	\
    (This)->lpVtbl -> FindPrinter(This)

#define ISearchAssistantOC2_FindPeople(This)	\
    (This)->lpVtbl -> FindPeople(This)

#define ISearchAssistantOC2_GetSearchAssistantURL(This,bSubstitute,bCustomize,pbstrValue)	\
    (This)->lpVtbl -> GetSearchAssistantURL(This,bSubstitute,bCustomize,pbstrValue)

#define ISearchAssistantOC2_NotifySearchSettingsChanged(This)	\
    (This)->lpVtbl -> NotifySearchSettingsChanged(This)

#define ISearchAssistantOC2_put_ASProvider(This,Provider)	\
    (This)->lpVtbl -> put_ASProvider(This,Provider)

#define ISearchAssistantOC2_get_ASProvider(This,pProvider)	\
    (This)->lpVtbl -> get_ASProvider(This,pProvider)

#define ISearchAssistantOC2_put_ASSetting(This,Setting)	\
    (This)->lpVtbl -> put_ASSetting(This,Setting)

#define ISearchAssistantOC2_get_ASSetting(This,pSetting)	\
    (This)->lpVtbl -> get_ASSetting(This,pSetting)

#define ISearchAssistantOC2_NETDetectNextNavigate(This)	\
    (This)->lpVtbl -> NETDetectNextNavigate(This)

#define ISearchAssistantOC2_PutFindText(This,FindText)	\
    (This)->lpVtbl -> PutFindText(This,FindText)

#define ISearchAssistantOC2_get_Version(This,pVersion)	\
    (This)->lpVtbl -> get_Version(This,pVersion)

#define ISearchAssistantOC2_EncodeString(This,bstrValue,bstrCharSet,bUseUTF8,pbstrResult)	\
    (This)->lpVtbl -> EncodeString(This,bstrValue,bstrCharSet,bUseUTF8,pbstrResult)


#define ISearchAssistantOC2_get_ShowFindPrinter(This,pbShowFindPrinter)	\
    (This)->lpVtbl -> get_ShowFindPrinter(This,pbShowFindPrinter)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id][propget] */ HRESULT STDMETHODCALLTYPE ISearchAssistantOC2_get_ShowFindPrinter_Proxy( 
    ISearchAssistantOC2 __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbShowFindPrinter);


void __RPC_STUB ISearchAssistantOC2_get_ShowFindPrinter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISearchAssistantOC2_INTERFACE_DEFINED__ */


#ifndef ___SearchAssistantEvents_DISPINTERFACE_DEFINED__
#define ___SearchAssistantEvents_DISPINTERFACE_DEFINED__

/* dispinterface _SearchAssistantEvents */
/* [hidden][uuid] */ 


EXTERN_C const IID DIID__SearchAssistantEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("1611FDDA-445B-11d2-85DE-00C04FA35C89")
    _SearchAssistantEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _SearchAssistantEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            _SearchAssistantEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            _SearchAssistantEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            _SearchAssistantEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            _SearchAssistantEvents __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            _SearchAssistantEvents __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            _SearchAssistantEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            _SearchAssistantEvents __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } _SearchAssistantEventsVtbl;

    interface _SearchAssistantEvents
    {
        CONST_VTBL struct _SearchAssistantEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _SearchAssistantEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _SearchAssistantEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _SearchAssistantEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _SearchAssistantEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _SearchAssistantEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _SearchAssistantEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _SearchAssistantEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___SearchAssistantEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_SearchAssistantOC;

#ifdef __cplusplus

class DECLSPEC_UUID("B45FF030-4447-11D2-85DE-00C04FA35C89")
SearchAssistantOC;
#endif
#endif /* __SHDocVw_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



