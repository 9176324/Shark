
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 5.03.0279 */
/* at Mon Mar 31 11:01:42 2003
 */
/* Compiler settings for shldisp.idl:
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

#ifndef __shldisp_h__
#define __shldisp_h__

/* Forward Declarations */ 

#ifndef __IFolderViewOC_FWD_DEFINED__
#define __IFolderViewOC_FWD_DEFINED__
typedef interface IFolderViewOC IFolderViewOC;
#endif 	/* __IFolderViewOC_FWD_DEFINED__ */


#ifndef __DShellFolderViewEvents_FWD_DEFINED__
#define __DShellFolderViewEvents_FWD_DEFINED__
typedef interface DShellFolderViewEvents DShellFolderViewEvents;
#endif 	/* __DShellFolderViewEvents_FWD_DEFINED__ */


#ifndef __ShellFolderViewOC_FWD_DEFINED__
#define __ShellFolderViewOC_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellFolderViewOC ShellFolderViewOC;
#else
typedef struct ShellFolderViewOC ShellFolderViewOC;
#endif /* __cplusplus */

#endif 	/* __ShellFolderViewOC_FWD_DEFINED__ */


#ifndef __DFConstraint_FWD_DEFINED__
#define __DFConstraint_FWD_DEFINED__
typedef interface DFConstraint DFConstraint;
#endif 	/* __DFConstraint_FWD_DEFINED__ */


#ifndef __ISearchCommandExt_FWD_DEFINED__
#define __ISearchCommandExt_FWD_DEFINED__
typedef interface ISearchCommandExt ISearchCommandExt;
#endif 	/* __ISearchCommandExt_FWD_DEFINED__ */


#ifndef __FolderItem_FWD_DEFINED__
#define __FolderItem_FWD_DEFINED__
typedef interface FolderItem FolderItem;
#endif 	/* __FolderItem_FWD_DEFINED__ */


#ifndef __FolderItems_FWD_DEFINED__
#define __FolderItems_FWD_DEFINED__
typedef interface FolderItems FolderItems;
#endif 	/* __FolderItems_FWD_DEFINED__ */


#ifndef __FolderItemVerb_FWD_DEFINED__
#define __FolderItemVerb_FWD_DEFINED__
typedef interface FolderItemVerb FolderItemVerb;
#endif 	/* __FolderItemVerb_FWD_DEFINED__ */


#ifndef __FolderItemVerbs_FWD_DEFINED__
#define __FolderItemVerbs_FWD_DEFINED__
typedef interface FolderItemVerbs FolderItemVerbs;
#endif 	/* __FolderItemVerbs_FWD_DEFINED__ */


#ifndef __Folder_FWD_DEFINED__
#define __Folder_FWD_DEFINED__
typedef interface Folder Folder;
#endif 	/* __Folder_FWD_DEFINED__ */


#ifndef __Folder2_FWD_DEFINED__
#define __Folder2_FWD_DEFINED__
typedef interface Folder2 Folder2;
#endif 	/* __Folder2_FWD_DEFINED__ */


#ifndef __FolderItem2_FWD_DEFINED__
#define __FolderItem2_FWD_DEFINED__
typedef interface FolderItem2 FolderItem2;
#endif 	/* __FolderItem2_FWD_DEFINED__ */


#ifndef __ShellFolderItem_FWD_DEFINED__
#define __ShellFolderItem_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellFolderItem ShellFolderItem;
#else
typedef struct ShellFolderItem ShellFolderItem;
#endif /* __cplusplus */

#endif 	/* __ShellFolderItem_FWD_DEFINED__ */


#ifndef __FolderItems2_FWD_DEFINED__
#define __FolderItems2_FWD_DEFINED__
typedef interface FolderItems2 FolderItems2;
#endif 	/* __FolderItems2_FWD_DEFINED__ */


#ifndef __IShellLinkDual_FWD_DEFINED__
#define __IShellLinkDual_FWD_DEFINED__
typedef interface IShellLinkDual IShellLinkDual;
#endif 	/* __IShellLinkDual_FWD_DEFINED__ */


#ifndef __IShellLinkDual2_FWD_DEFINED__
#define __IShellLinkDual2_FWD_DEFINED__
typedef interface IShellLinkDual2 IShellLinkDual2;
#endif 	/* __IShellLinkDual2_FWD_DEFINED__ */


#ifndef __ShellLinkObject_FWD_DEFINED__
#define __ShellLinkObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellLinkObject ShellLinkObject;
#else
typedef struct ShellLinkObject ShellLinkObject;
#endif /* __cplusplus */

#endif 	/* __ShellLinkObject_FWD_DEFINED__ */


#ifndef __IShellFolderViewDual_FWD_DEFINED__
#define __IShellFolderViewDual_FWD_DEFINED__
typedef interface IShellFolderViewDual IShellFolderViewDual;
#endif 	/* __IShellFolderViewDual_FWD_DEFINED__ */


#ifndef __ShellFolderView_FWD_DEFINED__
#define __ShellFolderView_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellFolderView ShellFolderView;
#else
typedef struct ShellFolderView ShellFolderView;
#endif /* __cplusplus */

#endif 	/* __ShellFolderView_FWD_DEFINED__ */


#ifndef __IShellDispatch_FWD_DEFINED__
#define __IShellDispatch_FWD_DEFINED__
typedef interface IShellDispatch IShellDispatch;
#endif 	/* __IShellDispatch_FWD_DEFINED__ */


#ifndef __IShellDispatch2_FWD_DEFINED__
#define __IShellDispatch2_FWD_DEFINED__
typedef interface IShellDispatch2 IShellDispatch2;
#endif 	/* __IShellDispatch2_FWD_DEFINED__ */


#ifndef __Shell_FWD_DEFINED__
#define __Shell_FWD_DEFINED__

#ifdef __cplusplus
typedef class Shell Shell;
#else
typedef struct Shell Shell;
#endif /* __cplusplus */

#endif 	/* __Shell_FWD_DEFINED__ */


#ifndef __ShellDispatchInproc_FWD_DEFINED__
#define __ShellDispatchInproc_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellDispatchInproc ShellDispatchInproc;
#else
typedef struct ShellDispatchInproc ShellDispatchInproc;
#endif /* __cplusplus */

#endif 	/* __ShellDispatchInproc_FWD_DEFINED__ */


#ifndef __WebViewFolderContents_FWD_DEFINED__
#define __WebViewFolderContents_FWD_DEFINED__

#ifdef __cplusplus
typedef class WebViewFolderContents WebViewFolderContents;
#else
typedef struct WebViewFolderContents WebViewFolderContents;
#endif /* __cplusplus */

#endif 	/* __WebViewFolderContents_FWD_DEFINED__ */


#ifndef __DSearchCommandEvents_FWD_DEFINED__
#define __DSearchCommandEvents_FWD_DEFINED__
typedef interface DSearchCommandEvents DSearchCommandEvents;
#endif 	/* __DSearchCommandEvents_FWD_DEFINED__ */


#ifndef __SearchCommand_FWD_DEFINED__
#define __SearchCommand_FWD_DEFINED__

#ifdef __cplusplus
typedef class SearchCommand SearchCommand;
#else
typedef struct SearchCommand SearchCommand;
#endif /* __cplusplus */

#endif 	/* __SearchCommand_FWD_DEFINED__ */


#ifndef __IFileSearchBand_FWD_DEFINED__
#define __IFileSearchBand_FWD_DEFINED__
typedef interface IFileSearchBand IFileSearchBand;
#endif 	/* __IFileSearchBand_FWD_DEFINED__ */


#ifndef __FileSearchBand_FWD_DEFINED__
#define __FileSearchBand_FWD_DEFINED__

#ifdef __cplusplus
typedef class FileSearchBand FileSearchBand;
#else
typedef struct FileSearchBand FileSearchBand;
#endif /* __cplusplus */

#endif 	/* __FileSearchBand_FWD_DEFINED__ */


#ifndef __IAutoComplete_FWD_DEFINED__
#define __IAutoComplete_FWD_DEFINED__
typedef interface IAutoComplete IAutoComplete;
#endif 	/* __IAutoComplete_FWD_DEFINED__ */


#ifndef __IAutoComplete2_FWD_DEFINED__
#define __IAutoComplete2_FWD_DEFINED__
typedef interface IAutoComplete2 IAutoComplete2;
#endif 	/* __IAutoComplete2_FWD_DEFINED__ */


#ifndef __IAsyncOperation_FWD_DEFINED__
#define __IAsyncOperation_FWD_DEFINED__
typedef interface IAsyncOperation IAsyncOperation;
#endif 	/* __IAsyncOperation_FWD_DEFINED__ */


/* header files for imported files */
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_shldisp_0000 */
/* [local] */ 


EXTERN_C const IID IID_IAsyncOperation;
#ifndef _SHLDISP_PUB_H_
#define _SHLDISP_PUB_H_



extern RPC_IF_HANDLE __MIDL_itf_shldisp_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shldisp_0000_v0_0_s_ifspec;


#ifndef __Shell32_LIBRARY_DEFINED__
#define __Shell32_LIBRARY_DEFINED__

/* library Shell32 */
/* [version][lcid][helpstring][uuid] */ 

typedef /* [helpstring][uuid] */  DECLSPEC_UUID("418f4e6a-b903-11d1-b0a6-00c04fc33aa5") 
enum SearchCommandExecuteErrors
    {	SCEE_PATHNOTFOUND	= 1,
	SCEE_MAXFILESFOUND	= SCEE_PATHNOTFOUND + 1,
	SCEE_INDEXSEARCH	= SCEE_MAXFILESFOUND + 1,
	SCEE_CONSTRAINT	= SCEE_INDEXSEARCH + 1,
	SCEE_SCOPEMISMATCH	= SCEE_CONSTRAINT + 1,
	SCEE_CASESENINDEX	= SCEE_SCOPEMISMATCH + 1,
	SCEE_INDEXNOTCOMPLETE	= SCEE_CASESENINDEX + 1
    }	SearchCommandExecuteErrors;





typedef /* [helpstring][uuid] */  DECLSPEC_UUID("35f1a0d0-3e9a-11d2-8499-005345000000") 
enum OfflineFolderStatus
    {	OFS_INACTIVE	= -1,
	OFS_ONLINE	= OFS_INACTIVE + 1,
	OFS_OFFLINE	= OFS_ONLINE + 1,
	OFS_SERVERBACK	= OFS_OFFLINE + 1,
	OFS_DIRTYCACHE	= OFS_SERVERBACK + 1
    }	OfflineFolderStatus;

typedef /* [helpstring][uuid] */  DECLSPEC_UUID("742A99A0-C77E-11D0-A32C-00A0C91EEDBA") 
enum ShellFolderViewOptions
    {	SFVVO_SHOWALLOBJECTS	= 0x1,
	SFVVO_SHOWEXTENSIONS	= 0x2,
	SFVVO_SHOWCOMPCOLOR	= 0x8,
	SFVVO_SHOWSYSFILES	= 0x20,
	SFVVO_WIN95CLASSIC	= 0x40,
	SFVVO_DOUBLECLICKINWEBVIEW	= 0x80,
	SFVVO_DESKTOPHTML	= 0x200
    }	ShellFolderViewOptions;

typedef /* [helpstring][uuid] */  DECLSPEC_UUID("CA31EA20-48D0-11CF-8350-444553540000") 
enum ShellSpecialFolderConstants
    {	ssfDESKTOP	= 0,
	ssfPROGRAMS	= 0x2,
	ssfCONTROLS	= 0x3,
	ssfPRINTERS	= 0x4,
	ssfPERSONAL	= 0x5,
	ssfFAVORITES	= 0x6,
	ssfSTARTUP	= 0x7,
	ssfRECENT	= 0x8,
	ssfSENDTO	= 0x9,
	ssfBITBUCKET	= 0xa,
	ssfSTARTMENU	= 0xb,
	ssfDESKTOPDIRECTORY	= 0x10,
	ssfDRIVES	= 0x11,
	ssfNETWORK	= 0x12,
	ssfNETHOOD	= 0x13,
	ssfFONTS	= 0x14,
	ssfTEMPLATES	= 0x15,
	ssfCOMMONSTARTMENU	= 0x16,
	ssfCOMMONPROGRAMS	= 0x17,
	ssfCOMMONSTARTUP	= 0x18,
	ssfCOMMONDESKTOPDIR	= 0x19,
	ssfAPPDATA	= 0x1a,
	ssfPRINTHOOD	= 0x1b,
	ssfLOCALAPPDATA	= 0x1c,
	ssfALTSTARTUP	= 0x1d,
	ssfCOMMONALTSTARTUP	= 0x1e,
	ssfCOMMONFAVORITES	= 0x1f,
	ssfINTERNETCACHE	= 0x20,
	ssfCOOKIES	= 0x21,
	ssfHISTORY	= 0x22,
	ssfCOMMONAPPDATA	= 0x23,
	ssfWINDOWS	= 0x24,
	ssfSYSTEM	= 0x25,
	ssfPROGRAMFILES	= 0x26,
	ssfMYPICTURES	= 0x27,
	ssfPROFILE	= 0x28,
	ssfSYSTEMx86	= 0x29,
	ssfPROGRAMFILESx86	= 0x30
    }	ShellSpecialFolderConstants;


EXTERN_C const IID LIBID_Shell32;

#ifndef __IFolderViewOC_INTERFACE_DEFINED__
#define __IFolderViewOC_INTERFACE_DEFINED__

/* interface IFolderViewOC */
/* [object][dual][oleautomation][hidden][helpcontext][helpstring][uuid] */ 


EXTERN_C const IID IID_IFolderViewOC;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9BA05970-F6A8-11CF-A442-00A0C90A8F39")
    IFolderViewOC : public IDispatch
    {
    public:
        virtual /* [helpcontext][helpstring] */ HRESULT STDMETHODCALLTYPE SetFolderView( 
            /* [in] */ IDispatch __RPC_FAR *pdisp) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IFolderViewOCVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IFolderViewOC __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IFolderViewOC __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IFolderViewOC __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IFolderViewOC __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IFolderViewOC __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IFolderViewOC __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IFolderViewOC __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpcontext][helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetFolderView )( 
            IFolderViewOC __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *pdisp);
        
        END_INTERFACE
    } IFolderViewOCVtbl;

    interface IFolderViewOC
    {
        CONST_VTBL struct IFolderViewOCVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFolderViewOC_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFolderViewOC_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFolderViewOC_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFolderViewOC_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFolderViewOC_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFolderViewOC_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFolderViewOC_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFolderViewOC_SetFolderView(This,pdisp)	\
    (This)->lpVtbl -> SetFolderView(This,pdisp)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpcontext][helpstring] */ HRESULT STDMETHODCALLTYPE IFolderViewOC_SetFolderView_Proxy( 
    IFolderViewOC __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *pdisp);


void __RPC_STUB IFolderViewOC_SetFolderView_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IFolderViewOC_INTERFACE_DEFINED__ */


#ifndef __DShellFolderViewEvents_DISPINTERFACE_DEFINED__
#define __DShellFolderViewEvents_DISPINTERFACE_DEFINED__

/* dispinterface DShellFolderViewEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID_DShellFolderViewEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("62112AA2-EBE4-11cf-A5FB-0020AFE7292D")
    DShellFolderViewEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct DShellFolderViewEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            DShellFolderViewEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            DShellFolderViewEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            DShellFolderViewEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            DShellFolderViewEvents __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            DShellFolderViewEvents __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            DShellFolderViewEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            DShellFolderViewEvents __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } DShellFolderViewEventsVtbl;

    interface DShellFolderViewEvents
    {
        CONST_VTBL struct DShellFolderViewEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DShellFolderViewEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DShellFolderViewEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DShellFolderViewEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DShellFolderViewEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DShellFolderViewEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DShellFolderViewEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DShellFolderViewEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __DShellFolderViewEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_ShellFolderViewOC;

#ifdef __cplusplus

class DECLSPEC_UUID("9BA05971-F6A8-11CF-A442-00A0C90A8F39")
ShellFolderViewOC;
#endif

#ifndef __DFConstraint_INTERFACE_DEFINED__
#define __DFConstraint_INTERFACE_DEFINED__

/* interface DFConstraint */
/* [object][dual][oleautomation][helpstring][uuid] */ 


EXTERN_C const IID IID_DFConstraint;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4a3df050-23bd-11d2-939f-00a0c91eedba")
    DFConstraint : public IDispatch
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR __RPC_FAR *pbs) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Value( 
            /* [retval][out] */ VARIANT __RPC_FAR *pv) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct DFConstraintVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            DFConstraint __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            DFConstraint __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            DFConstraint __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            DFConstraint __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            DFConstraint __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            DFConstraint __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            DFConstraint __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            DFConstraint __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Value )( 
            DFConstraint __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *pv);
        
        END_INTERFACE
    } DFConstraintVtbl;

    interface DFConstraint
    {
        CONST_VTBL struct DFConstraintVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DFConstraint_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DFConstraint_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DFConstraint_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DFConstraint_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DFConstraint_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DFConstraint_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DFConstraint_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define DFConstraint_get_Name(This,pbs)	\
    (This)->lpVtbl -> get_Name(This,pbs)

#define DFConstraint_get_Value(This,pv)	\
    (This)->lpVtbl -> get_Value(This,pv)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE DFConstraint_get_Name_Proxy( 
    DFConstraint __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbs);


void __RPC_STUB DFConstraint_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE DFConstraint_get_Value_Proxy( 
    DFConstraint __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *pv);


void __RPC_STUB DFConstraint_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __DFConstraint_INTERFACE_DEFINED__ */


#ifndef __ISearchCommandExt_INTERFACE_DEFINED__
#define __ISearchCommandExt_INTERFACE_DEFINED__

/* interface ISearchCommandExt */
/* [object][oleautomation][dual][helpstring][uuid] */ 


EXTERN_C const IID IID_ISearchCommandExt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1D2EFD50-75CE-11d1-B75A-00A0C90564FE")
    ISearchCommandExt : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ClearResults( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE NavigateToSearchResults( void) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_ProgressText( 
            /* [retval][out] */ BSTR __RPC_FAR *pbs) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SaveSearch( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetErrorInfo( 
            /* [out] */ BSTR __RPC_FAR *pbs,
            /* [retval][out] */ int __RPC_FAR *phr) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SearchFor( 
            /* [in] */ int iFor) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetScopeInfo( 
            /* [in] */ BSTR bsScope,
            /* [out] */ int __RPC_FAR *pdwScopeInfo) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RestoreSavedSearch( 
            /* [in] */ VARIANT __RPC_FAR *pvarFile) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Execute( 
            /* [optional][in] */ VARIANT __RPC_FAR *RecordsAffected,
            /* [optional][in] */ VARIANT __RPC_FAR *Parameters,
            /* [optional][in] */ long Options) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddConstraint( 
            /* [in] */ BSTR Name,
            /* [in] */ VARIANT Value) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetNextConstraint( 
            /* [in] */ VARIANT_BOOL fReset,
            /* [retval][out] */ DFConstraint __RPC_FAR *__RPC_FAR *ppdfc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISearchCommandExtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISearchCommandExt __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISearchCommandExt __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISearchCommandExt __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISearchCommandExt __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISearchCommandExt __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISearchCommandExt __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISearchCommandExt __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ClearResults )( 
            ISearchCommandExt __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NavigateToSearchResults )( 
            ISearchCommandExt __RPC_FAR * This);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ProgressText )( 
            ISearchCommandExt __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SaveSearch )( 
            ISearchCommandExt __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetErrorInfo )( 
            ISearchCommandExt __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbs,
            /* [retval][out] */ int __RPC_FAR *phr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SearchFor )( 
            ISearchCommandExt __RPC_FAR * This,
            /* [in] */ int iFor);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetScopeInfo )( 
            ISearchCommandExt __RPC_FAR * This,
            /* [in] */ BSTR bsScope,
            /* [out] */ int __RPC_FAR *pdwScopeInfo);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RestoreSavedSearch )( 
            ISearchCommandExt __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *pvarFile);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Execute )( 
            ISearchCommandExt __RPC_FAR * This,
            /* [optional][in] */ VARIANT __RPC_FAR *RecordsAffected,
            /* [optional][in] */ VARIANT __RPC_FAR *Parameters,
            /* [optional][in] */ long Options);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddConstraint )( 
            ISearchCommandExt __RPC_FAR * This,
            /* [in] */ BSTR Name,
            /* [in] */ VARIANT Value);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNextConstraint )( 
            ISearchCommandExt __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL fReset,
            /* [retval][out] */ DFConstraint __RPC_FAR *__RPC_FAR *ppdfc);
        
        END_INTERFACE
    } ISearchCommandExtVtbl;

    interface ISearchCommandExt
    {
        CONST_VTBL struct ISearchCommandExtVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISearchCommandExt_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISearchCommandExt_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISearchCommandExt_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISearchCommandExt_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISearchCommandExt_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISearchCommandExt_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISearchCommandExt_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISearchCommandExt_ClearResults(This)	\
    (This)->lpVtbl -> ClearResults(This)

#define ISearchCommandExt_NavigateToSearchResults(This)	\
    (This)->lpVtbl -> NavigateToSearchResults(This)

#define ISearchCommandExt_get_ProgressText(This,pbs)	\
    (This)->lpVtbl -> get_ProgressText(This,pbs)

#define ISearchCommandExt_SaveSearch(This)	\
    (This)->lpVtbl -> SaveSearch(This)

#define ISearchCommandExt_GetErrorInfo(This,pbs,phr)	\
    (This)->lpVtbl -> GetErrorInfo(This,pbs,phr)

#define ISearchCommandExt_SearchFor(This,iFor)	\
    (This)->lpVtbl -> SearchFor(This,iFor)

#define ISearchCommandExt_GetScopeInfo(This,bsScope,pdwScopeInfo)	\
    (This)->lpVtbl -> GetScopeInfo(This,bsScope,pdwScopeInfo)

#define ISearchCommandExt_RestoreSavedSearch(This,pvarFile)	\
    (This)->lpVtbl -> RestoreSavedSearch(This,pvarFile)

#define ISearchCommandExt_Execute(This,RecordsAffected,Parameters,Options)	\
    (This)->lpVtbl -> Execute(This,RecordsAffected,Parameters,Options)

#define ISearchCommandExt_AddConstraint(This,Name,Value)	\
    (This)->lpVtbl -> AddConstraint(This,Name,Value)

#define ISearchCommandExt_GetNextConstraint(This,fReset,ppdfc)	\
    (This)->lpVtbl -> GetNextConstraint(This,fReset,ppdfc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISearchCommandExt_ClearResults_Proxy( 
    ISearchCommandExt __RPC_FAR * This);


void __RPC_STUB ISearchCommandExt_ClearResults_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISearchCommandExt_NavigateToSearchResults_Proxy( 
    ISearchCommandExt __RPC_FAR * This);


void __RPC_STUB ISearchCommandExt_NavigateToSearchResults_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISearchCommandExt_get_ProgressText_Proxy( 
    ISearchCommandExt __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbs);


void __RPC_STUB ISearchCommandExt_get_ProgressText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISearchCommandExt_SaveSearch_Proxy( 
    ISearchCommandExt __RPC_FAR * This);


void __RPC_STUB ISearchCommandExt_SaveSearch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISearchCommandExt_GetErrorInfo_Proxy( 
    ISearchCommandExt __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbs,
    /* [retval][out] */ int __RPC_FAR *phr);


void __RPC_STUB ISearchCommandExt_GetErrorInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISearchCommandExt_SearchFor_Proxy( 
    ISearchCommandExt __RPC_FAR * This,
    /* [in] */ int iFor);


void __RPC_STUB ISearchCommandExt_SearchFor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISearchCommandExt_GetScopeInfo_Proxy( 
    ISearchCommandExt __RPC_FAR * This,
    /* [in] */ BSTR bsScope,
    /* [out] */ int __RPC_FAR *pdwScopeInfo);


void __RPC_STUB ISearchCommandExt_GetScopeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISearchCommandExt_RestoreSavedSearch_Proxy( 
    ISearchCommandExt __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *pvarFile);


void __RPC_STUB ISearchCommandExt_RestoreSavedSearch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISearchCommandExt_Execute_Proxy( 
    ISearchCommandExt __RPC_FAR * This,
    /* [optional][in] */ VARIANT __RPC_FAR *RecordsAffected,
    /* [optional][in] */ VARIANT __RPC_FAR *Parameters,
    /* [optional][in] */ long Options);


void __RPC_STUB ISearchCommandExt_Execute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISearchCommandExt_AddConstraint_Proxy( 
    ISearchCommandExt __RPC_FAR * This,
    /* [in] */ BSTR Name,
    /* [in] */ VARIANT Value);


void __RPC_STUB ISearchCommandExt_AddConstraint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISearchCommandExt_GetNextConstraint_Proxy( 
    ISearchCommandExt __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL fReset,
    /* [retval][out] */ DFConstraint __RPC_FAR *__RPC_FAR *ppdfc);


void __RPC_STUB ISearchCommandExt_GetNextConstraint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISearchCommandExt_INTERFACE_DEFINED__ */


#ifndef __FolderItem_INTERFACE_DEFINED__
#define __FolderItem_INTERFACE_DEFINED__

/* interface FolderItem */
/* [object][dual][oleautomation][helpstring][uuid] */ 

typedef /* [unique] */ FolderItem __RPC_FAR *LPFOLDERITEM;


EXTERN_C const IID IID_FolderItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAC32C80-CBE4-11CE-8350-444553540000")
    FolderItem : public IDispatch
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR __RPC_FAR *pbs) = 0;
        
        virtual /* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR bs) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Path( 
            /* [retval][out] */ BSTR __RPC_FAR *pbs) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_GetLink( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_GetFolder( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_IsLink( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pb) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_IsFolder( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pb) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_IsFileSystem( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pb) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_IsBrowsable( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pb) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_ModifyDate( 
            /* [retval][out] */ DATE __RPC_FAR *pdt) = 0;
        
        virtual /* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE put_ModifyDate( 
            /* [in] */ DATE dt) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Size( 
            /* [retval][out] */ LONG __RPC_FAR *pul) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Type( 
            /* [retval][out] */ BSTR __RPC_FAR *pbs) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Verbs( 
            /* [retval][out] */ FolderItemVerbs __RPC_FAR *__RPC_FAR *ppfic) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE InvokeVerb( 
            /* [optional][in] */ VARIANT vVerb) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct FolderItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            FolderItem __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            FolderItem __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            FolderItem __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            FolderItem __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            FolderItem __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            FolderItem __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            FolderItem __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            FolderItem __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            FolderItem __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            FolderItem __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Name )( 
            FolderItem __RPC_FAR * This,
            /* [in] */ BSTR bs);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Path )( 
            FolderItem __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GetLink )( 
            FolderItem __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GetFolder )( 
            FolderItem __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsLink )( 
            FolderItem __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pb);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsFolder )( 
            FolderItem __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pb);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsFileSystem )( 
            FolderItem __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pb);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsBrowsable )( 
            FolderItem __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pb);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ModifyDate )( 
            FolderItem __RPC_FAR * This,
            /* [retval][out] */ DATE __RPC_FAR *pdt);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ModifyDate )( 
            FolderItem __RPC_FAR * This,
            /* [in] */ DATE dt);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Size )( 
            FolderItem __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *pul);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Type )( 
            FolderItem __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Verbs )( 
            FolderItem __RPC_FAR * This,
            /* [retval][out] */ FolderItemVerbs __RPC_FAR *__RPC_FAR *ppfic);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InvokeVerb )( 
            FolderItem __RPC_FAR * This,
            /* [optional][in] */ VARIANT vVerb);
        
        END_INTERFACE
    } FolderItemVtbl;

    interface FolderItem
    {
        CONST_VTBL struct FolderItemVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define FolderItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define FolderItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define FolderItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define FolderItem_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define FolderItem_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define FolderItem_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define FolderItem_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define FolderItem_get_Application(This,ppid)	\
    (This)->lpVtbl -> get_Application(This,ppid)

#define FolderItem_get_Parent(This,ppid)	\
    (This)->lpVtbl -> get_Parent(This,ppid)

#define FolderItem_get_Name(This,pbs)	\
    (This)->lpVtbl -> get_Name(This,pbs)

#define FolderItem_put_Name(This,bs)	\
    (This)->lpVtbl -> put_Name(This,bs)

#define FolderItem_get_Path(This,pbs)	\
    (This)->lpVtbl -> get_Path(This,pbs)

#define FolderItem_get_GetLink(This,ppid)	\
    (This)->lpVtbl -> get_GetLink(This,ppid)

#define FolderItem_get_GetFolder(This,ppid)	\
    (This)->lpVtbl -> get_GetFolder(This,ppid)

#define FolderItem_get_IsLink(This,pb)	\
    (This)->lpVtbl -> get_IsLink(This,pb)

#define FolderItem_get_IsFolder(This,pb)	\
    (This)->lpVtbl -> get_IsFolder(This,pb)

#define FolderItem_get_IsFileSystem(This,pb)	\
    (This)->lpVtbl -> get_IsFileSystem(This,pb)

#define FolderItem_get_IsBrowsable(This,pb)	\
    (This)->lpVtbl -> get_IsBrowsable(This,pb)

#define FolderItem_get_ModifyDate(This,pdt)	\
    (This)->lpVtbl -> get_ModifyDate(This,pdt)

#define FolderItem_put_ModifyDate(This,dt)	\
    (This)->lpVtbl -> put_ModifyDate(This,dt)

#define FolderItem_get_Size(This,pul)	\
    (This)->lpVtbl -> get_Size(This,pul)

#define FolderItem_get_Type(This,pbs)	\
    (This)->lpVtbl -> get_Type(This,pbs)

#define FolderItem_Verbs(This,ppfic)	\
    (This)->lpVtbl -> Verbs(This,ppfic)

#define FolderItem_InvokeVerb(This,vVerb)	\
    (This)->lpVtbl -> InvokeVerb(This,vVerb)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItem_get_Application_Proxy( 
    FolderItem __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB FolderItem_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItem_get_Parent_Proxy( 
    FolderItem __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB FolderItem_get_Parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE FolderItem_get_Name_Proxy( 
    FolderItem __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbs);


void __RPC_STUB FolderItem_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propput][id] */ HRESULT STDMETHODCALLTYPE FolderItem_put_Name_Proxy( 
    FolderItem __RPC_FAR * This,
    /* [in] */ BSTR bs);


void __RPC_STUB FolderItem_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItem_get_Path_Proxy( 
    FolderItem __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbs);


void __RPC_STUB FolderItem_get_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItem_get_GetLink_Proxy( 
    FolderItem __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB FolderItem_get_GetLink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItem_get_GetFolder_Proxy( 
    FolderItem __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB FolderItem_get_GetFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItem_get_IsLink_Proxy( 
    FolderItem __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pb);


void __RPC_STUB FolderItem_get_IsLink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItem_get_IsFolder_Proxy( 
    FolderItem __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pb);


void __RPC_STUB FolderItem_get_IsFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItem_get_IsFileSystem_Proxy( 
    FolderItem __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pb);


void __RPC_STUB FolderItem_get_IsFileSystem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItem_get_IsBrowsable_Proxy( 
    FolderItem __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pb);


void __RPC_STUB FolderItem_get_IsBrowsable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItem_get_ModifyDate_Proxy( 
    FolderItem __RPC_FAR * This,
    /* [retval][out] */ DATE __RPC_FAR *pdt);


void __RPC_STUB FolderItem_get_ModifyDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE FolderItem_put_ModifyDate_Proxy( 
    FolderItem __RPC_FAR * This,
    /* [in] */ DATE dt);


void __RPC_STUB FolderItem_put_ModifyDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItem_get_Size_Proxy( 
    FolderItem __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *pul);


void __RPC_STUB FolderItem_get_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItem_get_Type_Proxy( 
    FolderItem __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbs);


void __RPC_STUB FolderItem_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE FolderItem_Verbs_Proxy( 
    FolderItem __RPC_FAR * This,
    /* [retval][out] */ FolderItemVerbs __RPC_FAR *__RPC_FAR *ppfic);


void __RPC_STUB FolderItem_Verbs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE FolderItem_InvokeVerb_Proxy( 
    FolderItem __RPC_FAR * This,
    /* [optional][in] */ VARIANT vVerb);


void __RPC_STUB FolderItem_InvokeVerb_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __FolderItem_INTERFACE_DEFINED__ */


#ifndef __FolderItems_INTERFACE_DEFINED__
#define __FolderItems_INTERFACE_DEFINED__

/* interface FolderItems */
/* [object][dual][oleautomation][helpstring][uuid] */ 


EXTERN_C const IID IID_FolderItems;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("744129E0-CBE5-11CE-8350-444553540000")
    FolderItems : public IDispatch
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long __RPC_FAR *plCount) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Item( 
            /* [optional][in] */ VARIANT index,
            /* [retval][out] */ FolderItem __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE _NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct FolderItemsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            FolderItems __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            FolderItems __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            FolderItems __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            FolderItems __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            FolderItems __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            FolderItems __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            FolderItems __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            FolderItems __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plCount);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            FolderItems __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            FolderItems __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )( 
            FolderItems __RPC_FAR * This,
            /* [optional][in] */ VARIANT index,
            /* [retval][out] */ FolderItem __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *_NewEnum )( 
            FolderItems __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);
        
        END_INTERFACE
    } FolderItemsVtbl;

    interface FolderItems
    {
        CONST_VTBL struct FolderItemsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define FolderItems_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define FolderItems_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define FolderItems_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define FolderItems_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define FolderItems_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define FolderItems_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define FolderItems_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define FolderItems_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define FolderItems_get_Application(This,ppid)	\
    (This)->lpVtbl -> get_Application(This,ppid)

#define FolderItems_get_Parent(This,ppid)	\
    (This)->lpVtbl -> get_Parent(This,ppid)

#define FolderItems_Item(This,index,ppid)	\
    (This)->lpVtbl -> Item(This,index,ppid)

#define FolderItems__NewEnum(This,ppunk)	\
    (This)->lpVtbl -> _NewEnum(This,ppunk)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItems_get_Count_Proxy( 
    FolderItems __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plCount);


void __RPC_STUB FolderItems_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItems_get_Application_Proxy( 
    FolderItems __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB FolderItems_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItems_get_Parent_Proxy( 
    FolderItems __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB FolderItems_get_Parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE FolderItems_Item_Proxy( 
    FolderItems __RPC_FAR * This,
    /* [optional][in] */ VARIANT index,
    /* [retval][out] */ FolderItem __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB FolderItems_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FolderItems__NewEnum_Proxy( 
    FolderItems __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);


void __RPC_STUB FolderItems__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __FolderItems_INTERFACE_DEFINED__ */


#ifndef __FolderItemVerb_INTERFACE_DEFINED__
#define __FolderItemVerb_INTERFACE_DEFINED__

/* interface FolderItemVerb */
/* [object][dual][oleautomation][helpstring][uuid] */ 


EXTERN_C const IID IID_FolderItemVerb;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("08EC3E00-50B0-11CF-960C-0080C7F4EE85")
    FolderItemVerb : public IDispatch
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR __RPC_FAR *pbs) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE DoIt( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct FolderItemVerbVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            FolderItemVerb __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            FolderItemVerb __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            FolderItemVerb __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            FolderItemVerb __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            FolderItemVerb __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            FolderItemVerb __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            FolderItemVerb __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            FolderItemVerb __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            FolderItemVerb __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            FolderItemVerb __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DoIt )( 
            FolderItemVerb __RPC_FAR * This);
        
        END_INTERFACE
    } FolderItemVerbVtbl;

    interface FolderItemVerb
    {
        CONST_VTBL struct FolderItemVerbVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define FolderItemVerb_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define FolderItemVerb_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define FolderItemVerb_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define FolderItemVerb_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define FolderItemVerb_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define FolderItemVerb_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define FolderItemVerb_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define FolderItemVerb_get_Application(This,ppid)	\
    (This)->lpVtbl -> get_Application(This,ppid)

#define FolderItemVerb_get_Parent(This,ppid)	\
    (This)->lpVtbl -> get_Parent(This,ppid)

#define FolderItemVerb_get_Name(This,pbs)	\
    (This)->lpVtbl -> get_Name(This,pbs)

#define FolderItemVerb_DoIt(This)	\
    (This)->lpVtbl -> DoIt(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItemVerb_get_Application_Proxy( 
    FolderItemVerb __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB FolderItemVerb_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItemVerb_get_Parent_Proxy( 
    FolderItemVerb __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB FolderItemVerb_get_Parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE FolderItemVerb_get_Name_Proxy( 
    FolderItemVerb __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbs);


void __RPC_STUB FolderItemVerb_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE FolderItemVerb_DoIt_Proxy( 
    FolderItemVerb __RPC_FAR * This);


void __RPC_STUB FolderItemVerb_DoIt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __FolderItemVerb_INTERFACE_DEFINED__ */


#ifndef __FolderItemVerbs_INTERFACE_DEFINED__
#define __FolderItemVerbs_INTERFACE_DEFINED__

/* interface FolderItemVerbs */
/* [object][dual][oleautomation][helpstring][uuid] */ 


EXTERN_C const IID IID_FolderItemVerbs;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1F8352C0-50B0-11CF-960C-0080C7F4EE85")
    FolderItemVerbs : public IDispatch
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long __RPC_FAR *plCount) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Item( 
            /* [optional][in] */ VARIANT index,
            /* [retval][out] */ FolderItemVerb __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE _NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct FolderItemVerbsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            FolderItemVerbs __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            FolderItemVerbs __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            FolderItemVerbs __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            FolderItemVerbs __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            FolderItemVerbs __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            FolderItemVerbs __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            FolderItemVerbs __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            FolderItemVerbs __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plCount);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            FolderItemVerbs __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            FolderItemVerbs __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )( 
            FolderItemVerbs __RPC_FAR * This,
            /* [optional][in] */ VARIANT index,
            /* [retval][out] */ FolderItemVerb __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *_NewEnum )( 
            FolderItemVerbs __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);
        
        END_INTERFACE
    } FolderItemVerbsVtbl;

    interface FolderItemVerbs
    {
        CONST_VTBL struct FolderItemVerbsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define FolderItemVerbs_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define FolderItemVerbs_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define FolderItemVerbs_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define FolderItemVerbs_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define FolderItemVerbs_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define FolderItemVerbs_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define FolderItemVerbs_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define FolderItemVerbs_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define FolderItemVerbs_get_Application(This,ppid)	\
    (This)->lpVtbl -> get_Application(This,ppid)

#define FolderItemVerbs_get_Parent(This,ppid)	\
    (This)->lpVtbl -> get_Parent(This,ppid)

#define FolderItemVerbs_Item(This,index,ppid)	\
    (This)->lpVtbl -> Item(This,index,ppid)

#define FolderItemVerbs__NewEnum(This,ppunk)	\
    (This)->lpVtbl -> _NewEnum(This,ppunk)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItemVerbs_get_Count_Proxy( 
    FolderItemVerbs __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plCount);


void __RPC_STUB FolderItemVerbs_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItemVerbs_get_Application_Proxy( 
    FolderItemVerbs __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB FolderItemVerbs_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE FolderItemVerbs_get_Parent_Proxy( 
    FolderItemVerbs __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB FolderItemVerbs_get_Parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE FolderItemVerbs_Item_Proxy( 
    FolderItemVerbs __RPC_FAR * This,
    /* [optional][in] */ VARIANT index,
    /* [retval][out] */ FolderItemVerb __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB FolderItemVerbs_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FolderItemVerbs__NewEnum_Proxy( 
    FolderItemVerbs __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);


void __RPC_STUB FolderItemVerbs__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __FolderItemVerbs_INTERFACE_DEFINED__ */


#ifndef __Folder_INTERFACE_DEFINED__
#define __Folder_INTERFACE_DEFINED__

/* interface Folder */
/* [object][dual][oleautomation][helpstring][uuid] */ 


EXTERN_C const IID IID_Folder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BBCBDE60-C3FF-11CE-8350-444553540000")
    Folder : public IDispatch
    {
    public:
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Title( 
            /* [retval][out] */ BSTR __RPC_FAR *pbs) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_ParentFolder( 
            /* [retval][out] */ Folder __RPC_FAR *__RPC_FAR *ppsf) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Items( 
            /* [retval][out] */ FolderItems __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ParseName( 
            /* [in] */ BSTR bName,
            /* [retval][out] */ FolderItem __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE NewFolder( 
            /* [in] */ BSTR bName,
            /* [optional][in] */ VARIANT vOptions) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE MoveHere( 
            /* [in] */ VARIANT vItem,
            /* [optional][in] */ VARIANT vOptions) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CopyHere( 
            /* [in] */ VARIANT vItem,
            /* [optional][in] */ VARIANT vOptions) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDetailsOf( 
            /* [in] */ VARIANT vItem,
            /* [in] */ int iColumn,
            /* [retval][out] */ BSTR __RPC_FAR *pbs) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct FolderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            Folder __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            Folder __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            Folder __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            Folder __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            Folder __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            Folder __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            Folder __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Title )( 
            Folder __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            Folder __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            Folder __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ParentFolder )( 
            Folder __RPC_FAR * This,
            /* [retval][out] */ Folder __RPC_FAR *__RPC_FAR *ppsf);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Items )( 
            Folder __RPC_FAR * This,
            /* [retval][out] */ FolderItems __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ParseName )( 
            Folder __RPC_FAR * This,
            /* [in] */ BSTR bName,
            /* [retval][out] */ FolderItem __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NewFolder )( 
            Folder __RPC_FAR * This,
            /* [in] */ BSTR bName,
            /* [optional][in] */ VARIANT vOptions);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveHere )( 
            Folder __RPC_FAR * This,
            /* [in] */ VARIANT vItem,
            /* [optional][in] */ VARIANT vOptions);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CopyHere )( 
            Folder __RPC_FAR * This,
            /* [in] */ VARIANT vItem,
            /* [optional][in] */ VARIANT vOptions);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDetailsOf )( 
            Folder __RPC_FAR * This,
            /* [in] */ VARIANT vItem,
            /* [in] */ int iColumn,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        END_INTERFACE
    } FolderVtbl;

    interface Folder
    {
        CONST_VTBL struct FolderVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Folder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Folder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Folder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Folder_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Folder_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Folder_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Folder_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Folder_get_Title(This,pbs)	\
    (This)->lpVtbl -> get_Title(This,pbs)

#define Folder_get_Application(This,ppid)	\
    (This)->lpVtbl -> get_Application(This,ppid)

#define Folder_get_Parent(This,ppid)	\
    (This)->lpVtbl -> get_Parent(This,ppid)

#define Folder_get_ParentFolder(This,ppsf)	\
    (This)->lpVtbl -> get_ParentFolder(This,ppsf)

#define Folder_Items(This,ppid)	\
    (This)->lpVtbl -> Items(This,ppid)

#define Folder_ParseName(This,bName,ppid)	\
    (This)->lpVtbl -> ParseName(This,bName,ppid)

#define Folder_NewFolder(This,bName,vOptions)	\
    (This)->lpVtbl -> NewFolder(This,bName,vOptions)

#define Folder_MoveHere(This,vItem,vOptions)	\
    (This)->lpVtbl -> MoveHere(This,vItem,vOptions)

#define Folder_CopyHere(This,vItem,vOptions)	\
    (This)->lpVtbl -> CopyHere(This,vItem,vOptions)

#define Folder_GetDetailsOf(This,vItem,iColumn,pbs)	\
    (This)->lpVtbl -> GetDetailsOf(This,vItem,iColumn,pbs)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE Folder_get_Title_Proxy( 
    Folder __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbs);


void __RPC_STUB Folder_get_Title_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE Folder_get_Application_Proxy( 
    Folder __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB Folder_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE Folder_get_Parent_Proxy( 
    Folder __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB Folder_get_Parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE Folder_get_ParentFolder_Proxy( 
    Folder __RPC_FAR * This,
    /* [retval][out] */ Folder __RPC_FAR *__RPC_FAR *ppsf);


void __RPC_STUB Folder_get_ParentFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE Folder_Items_Proxy( 
    Folder __RPC_FAR * This,
    /* [retval][out] */ FolderItems __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB Folder_Items_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE Folder_ParseName_Proxy( 
    Folder __RPC_FAR * This,
    /* [in] */ BSTR bName,
    /* [retval][out] */ FolderItem __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB Folder_ParseName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE Folder_NewFolder_Proxy( 
    Folder __RPC_FAR * This,
    /* [in] */ BSTR bName,
    /* [optional][in] */ VARIANT vOptions);


void __RPC_STUB Folder_NewFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE Folder_MoveHere_Proxy( 
    Folder __RPC_FAR * This,
    /* [in] */ VARIANT vItem,
    /* [optional][in] */ VARIANT vOptions);


void __RPC_STUB Folder_MoveHere_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE Folder_CopyHere_Proxy( 
    Folder __RPC_FAR * This,
    /* [in] */ VARIANT vItem,
    /* [optional][in] */ VARIANT vOptions);


void __RPC_STUB Folder_CopyHere_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE Folder_GetDetailsOf_Proxy( 
    Folder __RPC_FAR * This,
    /* [in] */ VARIANT vItem,
    /* [in] */ int iColumn,
    /* [retval][out] */ BSTR __RPC_FAR *pbs);


void __RPC_STUB Folder_GetDetailsOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __Folder_INTERFACE_DEFINED__ */


#ifndef __Folder2_INTERFACE_DEFINED__
#define __Folder2_INTERFACE_DEFINED__

/* interface Folder2 */
/* [object][dual][oleautomation][helpstring][uuid] */ 


EXTERN_C const IID IID_Folder2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f0d2d8ef-3890-11d2-bf8b-00c04fb93661")
    Folder2 : public Folder
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Self( 
            /* [retval][out] */ FolderItem __RPC_FAR *__RPC_FAR *ppfi) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_OfflineStatus( 
            /* [retval][out] */ LONG __RPC_FAR *pul) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Synchronize( void) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_HaveToShowWebViewBarricade( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbHaveToShowWebViewBarricade) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE DismissedWebViewBarricade( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct Folder2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            Folder2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            Folder2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            Folder2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            Folder2 __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            Folder2 __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            Folder2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            Folder2 __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Title )( 
            Folder2 __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            Folder2 __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            Folder2 __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ParentFolder )( 
            Folder2 __RPC_FAR * This,
            /* [retval][out] */ Folder __RPC_FAR *__RPC_FAR *ppsf);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Items )( 
            Folder2 __RPC_FAR * This,
            /* [retval][out] */ FolderItems __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ParseName )( 
            Folder2 __RPC_FAR * This,
            /* [in] */ BSTR bName,
            /* [retval][out] */ FolderItem __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NewFolder )( 
            Folder2 __RPC_FAR * This,
            /* [in] */ BSTR bName,
            /* [optional][in] */ VARIANT vOptions);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveHere )( 
            Folder2 __RPC_FAR * This,
            /* [in] */ VARIANT vItem,
            /* [optional][in] */ VARIANT vOptions);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CopyHere )( 
            Folder2 __RPC_FAR * This,
            /* [in] */ VARIANT vItem,
            /* [optional][in] */ VARIANT vOptions);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDetailsOf )( 
            Folder2 __RPC_FAR * This,
            /* [in] */ VARIANT vItem,
            /* [in] */ int iColumn,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Self )( 
            Folder2 __RPC_FAR * This,
            /* [retval][out] */ FolderItem __RPC_FAR *__RPC_FAR *ppfi);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_OfflineStatus )( 
            Folder2 __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *pul);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Synchronize )( 
            Folder2 __RPC_FAR * This);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HaveToShowWebViewBarricade )( 
            Folder2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbHaveToShowWebViewBarricade);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DismissedWebViewBarricade )( 
            Folder2 __RPC_FAR * This);
        
        END_INTERFACE
    } Folder2Vtbl;

    interface Folder2
    {
        CONST_VTBL struct Folder2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Folder2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Folder2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Folder2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Folder2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Folder2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Folder2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Folder2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Folder2_get_Title(This,pbs)	\
    (This)->lpVtbl -> get_Title(This,pbs)

#define Folder2_get_Application(This,ppid)	\
    (This)->lpVtbl -> get_Application(This,ppid)

#define Folder2_get_Parent(This,ppid)	\
    (This)->lpVtbl -> get_Parent(This,ppid)

#define Folder2_get_ParentFolder(This,ppsf)	\
    (This)->lpVtbl -> get_ParentFolder(This,ppsf)

#define Folder2_Items(This,ppid)	\
    (This)->lpVtbl -> Items(This,ppid)

#define Folder2_ParseName(This,bName,ppid)	\
    (This)->lpVtbl -> ParseName(This,bName,ppid)

#define Folder2_NewFolder(This,bName,vOptions)	\
    (This)->lpVtbl -> NewFolder(This,bName,vOptions)

#define Folder2_MoveHere(This,vItem,vOptions)	\
    (This)->lpVtbl -> MoveHere(This,vItem,vOptions)

#define Folder2_CopyHere(This,vItem,vOptions)	\
    (This)->lpVtbl -> CopyHere(This,vItem,vOptions)

#define Folder2_GetDetailsOf(This,vItem,iColumn,pbs)	\
    (This)->lpVtbl -> GetDetailsOf(This,vItem,iColumn,pbs)


#define Folder2_get_Self(This,ppfi)	\
    (This)->lpVtbl -> get_Self(This,ppfi)

#define Folder2_get_OfflineStatus(This,pul)	\
    (This)->lpVtbl -> get_OfflineStatus(This,pul)

#define Folder2_Synchronize(This)	\
    (This)->lpVtbl -> Synchronize(This)

#define Folder2_get_HaveToShowWebViewBarricade(This,pbHaveToShowWebViewBarricade)	\
    (This)->lpVtbl -> get_HaveToShowWebViewBarricade(This,pbHaveToShowWebViewBarricade)

#define Folder2_DismissedWebViewBarricade(This)	\
    (This)->lpVtbl -> DismissedWebViewBarricade(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE Folder2_get_Self_Proxy( 
    Folder2 __RPC_FAR * This,
    /* [retval][out] */ FolderItem __RPC_FAR *__RPC_FAR *ppfi);


void __RPC_STUB Folder2_get_Self_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE Folder2_get_OfflineStatus_Proxy( 
    Folder2 __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *pul);


void __RPC_STUB Folder2_get_OfflineStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE Folder2_Synchronize_Proxy( 
    Folder2 __RPC_FAR * This);


void __RPC_STUB Folder2_Synchronize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE Folder2_get_HaveToShowWebViewBarricade_Proxy( 
    Folder2 __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbHaveToShowWebViewBarricade);


void __RPC_STUB Folder2_get_HaveToShowWebViewBarricade_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE Folder2_DismissedWebViewBarricade_Proxy( 
    Folder2 __RPC_FAR * This);


void __RPC_STUB Folder2_DismissedWebViewBarricade_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __Folder2_INTERFACE_DEFINED__ */


#ifndef __FolderItem2_INTERFACE_DEFINED__
#define __FolderItem2_INTERFACE_DEFINED__

/* interface FolderItem2 */
/* [object][dual][oleautomation][helpstring][uuid] */ 


EXTERN_C const IID IID_FolderItem2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("edc817aa-92b8-11d1-b075-00c04fc33aa5")
    FolderItem2 : public FolderItem
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE InvokeVerbEx( 
            /* [optional][in] */ VARIANT vVerb,
            /* [optional][in] */ VARIANT vArgs) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ExtendedProperty( 
            /* [in] */ BSTR bstrPropName,
            /* [retval][out] */ VARIANT __RPC_FAR *pvRet) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct FolderItem2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            FolderItem2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            FolderItem2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            FolderItem2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            FolderItem2 __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            FolderItem2 __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            FolderItem2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            FolderItem2 __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            FolderItem2 __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            FolderItem2 __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            FolderItem2 __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring][propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Name )( 
            FolderItem2 __RPC_FAR * This,
            /* [in] */ BSTR bs);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Path )( 
            FolderItem2 __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GetLink )( 
            FolderItem2 __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GetFolder )( 
            FolderItem2 __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsLink )( 
            FolderItem2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pb);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsFolder )( 
            FolderItem2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pb);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsFileSystem )( 
            FolderItem2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pb);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsBrowsable )( 
            FolderItem2 __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pb);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ModifyDate )( 
            FolderItem2 __RPC_FAR * This,
            /* [retval][out] */ DATE __RPC_FAR *pdt);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ModifyDate )( 
            FolderItem2 __RPC_FAR * This,
            /* [in] */ DATE dt);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Size )( 
            FolderItem2 __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *pul);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Type )( 
            FolderItem2 __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Verbs )( 
            FolderItem2 __RPC_FAR * This,
            /* [retval][out] */ FolderItemVerbs __RPC_FAR *__RPC_FAR *ppfic);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InvokeVerb )( 
            FolderItem2 __RPC_FAR * This,
            /* [optional][in] */ VARIANT vVerb);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InvokeVerbEx )( 
            FolderItem2 __RPC_FAR * This,
            /* [optional][in] */ VARIANT vVerb,
            /* [optional][in] */ VARIANT vArgs);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExtendedProperty )( 
            FolderItem2 __RPC_FAR * This,
            /* [in] */ BSTR bstrPropName,
            /* [retval][out] */ VARIANT __RPC_FAR *pvRet);
        
        END_INTERFACE
    } FolderItem2Vtbl;

    interface FolderItem2
    {
        CONST_VTBL struct FolderItem2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define FolderItem2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define FolderItem2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define FolderItem2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define FolderItem2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define FolderItem2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define FolderItem2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define FolderItem2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define FolderItem2_get_Application(This,ppid)	\
    (This)->lpVtbl -> get_Application(This,ppid)

#define FolderItem2_get_Parent(This,ppid)	\
    (This)->lpVtbl -> get_Parent(This,ppid)

#define FolderItem2_get_Name(This,pbs)	\
    (This)->lpVtbl -> get_Name(This,pbs)

#define FolderItem2_put_Name(This,bs)	\
    (This)->lpVtbl -> put_Name(This,bs)

#define FolderItem2_get_Path(This,pbs)	\
    (This)->lpVtbl -> get_Path(This,pbs)

#define FolderItem2_get_GetLink(This,ppid)	\
    (This)->lpVtbl -> get_GetLink(This,ppid)

#define FolderItem2_get_GetFolder(This,ppid)	\
    (This)->lpVtbl -> get_GetFolder(This,ppid)

#define FolderItem2_get_IsLink(This,pb)	\
    (This)->lpVtbl -> get_IsLink(This,pb)

#define FolderItem2_get_IsFolder(This,pb)	\
    (This)->lpVtbl -> get_IsFolder(This,pb)

#define FolderItem2_get_IsFileSystem(This,pb)	\
    (This)->lpVtbl -> get_IsFileSystem(This,pb)

#define FolderItem2_get_IsBrowsable(This,pb)	\
    (This)->lpVtbl -> get_IsBrowsable(This,pb)

#define FolderItem2_get_ModifyDate(This,pdt)	\
    (This)->lpVtbl -> get_ModifyDate(This,pdt)

#define FolderItem2_put_ModifyDate(This,dt)	\
    (This)->lpVtbl -> put_ModifyDate(This,dt)

#define FolderItem2_get_Size(This,pul)	\
    (This)->lpVtbl -> get_Size(This,pul)

#define FolderItem2_get_Type(This,pbs)	\
    (This)->lpVtbl -> get_Type(This,pbs)

#define FolderItem2_Verbs(This,ppfic)	\
    (This)->lpVtbl -> Verbs(This,ppfic)

#define FolderItem2_InvokeVerb(This,vVerb)	\
    (This)->lpVtbl -> InvokeVerb(This,vVerb)


#define FolderItem2_InvokeVerbEx(This,vVerb,vArgs)	\
    (This)->lpVtbl -> InvokeVerbEx(This,vVerb,vArgs)

#define FolderItem2_ExtendedProperty(This,bstrPropName,pvRet)	\
    (This)->lpVtbl -> ExtendedProperty(This,bstrPropName,pvRet)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE FolderItem2_InvokeVerbEx_Proxy( 
    FolderItem2 __RPC_FAR * This,
    /* [optional][in] */ VARIANT vVerb,
    /* [optional][in] */ VARIANT vArgs);


void __RPC_STUB FolderItem2_InvokeVerbEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE FolderItem2_ExtendedProperty_Proxy( 
    FolderItem2 __RPC_FAR * This,
    /* [in] */ BSTR bstrPropName,
    /* [retval][out] */ VARIANT __RPC_FAR *pvRet);


void __RPC_STUB FolderItem2_ExtendedProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __FolderItem2_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_ShellFolderItem;

#ifdef __cplusplus

class DECLSPEC_UUID("2fe352ea-fd1f-11d2-b1f4-00c04f8eeb3e")
ShellFolderItem;
#endif

#ifndef __FolderItems2_INTERFACE_DEFINED__
#define __FolderItems2_INTERFACE_DEFINED__

/* interface FolderItems2 */
/* [object][dual][oleautomation][helpstring][uuid] */ 


EXTERN_C const IID IID_FolderItems2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C94F0AD0-F363-11d2-A327-00C04F8EEC7F")
    FolderItems2 : public FolderItems
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE InvokeVerbEx( 
            /* [optional][in] */ VARIANT vVerb,
            /* [optional][in] */ VARIANT vArgs) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct FolderItems2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            FolderItems2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            FolderItems2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            FolderItems2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            FolderItems2 __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            FolderItems2 __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            FolderItems2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            FolderItems2 __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            FolderItems2 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plCount);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            FolderItems2 __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            FolderItems2 __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )( 
            FolderItems2 __RPC_FAR * This,
            /* [optional][in] */ VARIANT index,
            /* [retval][out] */ FolderItem __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *_NewEnum )( 
            FolderItems2 __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InvokeVerbEx )( 
            FolderItems2 __RPC_FAR * This,
            /* [optional][in] */ VARIANT vVerb,
            /* [optional][in] */ VARIANT vArgs);
        
        END_INTERFACE
    } FolderItems2Vtbl;

    interface FolderItems2
    {
        CONST_VTBL struct FolderItems2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define FolderItems2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define FolderItems2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define FolderItems2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define FolderItems2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define FolderItems2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define FolderItems2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define FolderItems2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define FolderItems2_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define FolderItems2_get_Application(This,ppid)	\
    (This)->lpVtbl -> get_Application(This,ppid)

#define FolderItems2_get_Parent(This,ppid)	\
    (This)->lpVtbl -> get_Parent(This,ppid)

#define FolderItems2_Item(This,index,ppid)	\
    (This)->lpVtbl -> Item(This,index,ppid)

#define FolderItems2__NewEnum(This,ppunk)	\
    (This)->lpVtbl -> _NewEnum(This,ppunk)


#define FolderItems2_InvokeVerbEx(This,vVerb,vArgs)	\
    (This)->lpVtbl -> InvokeVerbEx(This,vVerb,vArgs)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE FolderItems2_InvokeVerbEx_Proxy( 
    FolderItems2 __RPC_FAR * This,
    /* [optional][in] */ VARIANT vVerb,
    /* [optional][in] */ VARIANT vArgs);


void __RPC_STUB FolderItems2_InvokeVerbEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __FolderItems2_INTERFACE_DEFINED__ */


#ifndef __IShellLinkDual_INTERFACE_DEFINED__
#define __IShellLinkDual_INTERFACE_DEFINED__

/* interface IShellLinkDual */
/* [object][hidden][dual][oleautomation][helpstring][uuid] */ 


EXTERN_C const IID IID_IShellLinkDual;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("88A05C00-F000-11CE-8350-444553540000")
    IShellLinkDual : public IDispatch
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Path( 
            /* [retval][out] */ BSTR __RPC_FAR *pbs) = 0;
        
        virtual /* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE put_Path( 
            /* [in] */ BSTR bs) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Description( 
            /* [retval][out] */ BSTR __RPC_FAR *pbs) = 0;
        
        virtual /* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE put_Description( 
            /* [in] */ BSTR bs) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_WorkingDirectory( 
            /* [retval][out] */ BSTR __RPC_FAR *pbs) = 0;
        
        virtual /* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE put_WorkingDirectory( 
            /* [in] */ BSTR bs) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Arguments( 
            /* [retval][out] */ BSTR __RPC_FAR *pbs) = 0;
        
        virtual /* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE put_Arguments( 
            /* [in] */ BSTR bs) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Hotkey( 
            /* [retval][out] */ int __RPC_FAR *piHK) = 0;
        
        virtual /* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE put_Hotkey( 
            /* [in] */ int iHK) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_ShowCommand( 
            /* [retval][out] */ int __RPC_FAR *piShowCommand) = 0;
        
        virtual /* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE put_ShowCommand( 
            /* [in] */ int iShowCommand) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Resolve( 
            /* [in] */ int fFlags) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetIconLocation( 
            /* [out] */ BSTR __RPC_FAR *pbs,
            /* [retval][out] */ int __RPC_FAR *piIcon) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetIconLocation( 
            /* [in] */ BSTR bs,
            /* [in] */ int iIcon) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Save( 
            /* [optional][in] */ VARIANT vWhere) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IShellLinkDualVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IShellLinkDual __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IShellLinkDual __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IShellLinkDual __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IShellLinkDual __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IShellLinkDual __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IShellLinkDual __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IShellLinkDual __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Path )( 
            IShellLinkDual __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Path )( 
            IShellLinkDual __RPC_FAR * This,
            /* [in] */ BSTR bs);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Description )( 
            IShellLinkDual __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Description )( 
            IShellLinkDual __RPC_FAR * This,
            /* [in] */ BSTR bs);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_WorkingDirectory )( 
            IShellLinkDual __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_WorkingDirectory )( 
            IShellLinkDual __RPC_FAR * This,
            /* [in] */ BSTR bs);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Arguments )( 
            IShellLinkDual __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Arguments )( 
            IShellLinkDual __RPC_FAR * This,
            /* [in] */ BSTR bs);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Hotkey )( 
            IShellLinkDual __RPC_FAR * This,
            /* [retval][out] */ int __RPC_FAR *piHK);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Hotkey )( 
            IShellLinkDual __RPC_FAR * This,
            /* [in] */ int iHK);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShowCommand )( 
            IShellLinkDual __RPC_FAR * This,
            /* [retval][out] */ int __RPC_FAR *piShowCommand);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ShowCommand )( 
            IShellLinkDual __RPC_FAR * This,
            /* [in] */ int iShowCommand);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Resolve )( 
            IShellLinkDual __RPC_FAR * This,
            /* [in] */ int fFlags);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIconLocation )( 
            IShellLinkDual __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbs,
            /* [retval][out] */ int __RPC_FAR *piIcon);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetIconLocation )( 
            IShellLinkDual __RPC_FAR * This,
            /* [in] */ BSTR bs,
            /* [in] */ int iIcon);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Save )( 
            IShellLinkDual __RPC_FAR * This,
            /* [optional][in] */ VARIANT vWhere);
        
        END_INTERFACE
    } IShellLinkDualVtbl;

    interface IShellLinkDual
    {
        CONST_VTBL struct IShellLinkDualVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellLinkDual_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellLinkDual_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellLinkDual_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellLinkDual_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IShellLinkDual_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IShellLinkDual_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IShellLinkDual_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IShellLinkDual_get_Path(This,pbs)	\
    (This)->lpVtbl -> get_Path(This,pbs)

#define IShellLinkDual_put_Path(This,bs)	\
    (This)->lpVtbl -> put_Path(This,bs)

#define IShellLinkDual_get_Description(This,pbs)	\
    (This)->lpVtbl -> get_Description(This,pbs)

#define IShellLinkDual_put_Description(This,bs)	\
    (This)->lpVtbl -> put_Description(This,bs)

#define IShellLinkDual_get_WorkingDirectory(This,pbs)	\
    (This)->lpVtbl -> get_WorkingDirectory(This,pbs)

#define IShellLinkDual_put_WorkingDirectory(This,bs)	\
    (This)->lpVtbl -> put_WorkingDirectory(This,bs)

#define IShellLinkDual_get_Arguments(This,pbs)	\
    (This)->lpVtbl -> get_Arguments(This,pbs)

#define IShellLinkDual_put_Arguments(This,bs)	\
    (This)->lpVtbl -> put_Arguments(This,bs)

#define IShellLinkDual_get_Hotkey(This,piHK)	\
    (This)->lpVtbl -> get_Hotkey(This,piHK)

#define IShellLinkDual_put_Hotkey(This,iHK)	\
    (This)->lpVtbl -> put_Hotkey(This,iHK)

#define IShellLinkDual_get_ShowCommand(This,piShowCommand)	\
    (This)->lpVtbl -> get_ShowCommand(This,piShowCommand)

#define IShellLinkDual_put_ShowCommand(This,iShowCommand)	\
    (This)->lpVtbl -> put_ShowCommand(This,iShowCommand)

#define IShellLinkDual_Resolve(This,fFlags)	\
    (This)->lpVtbl -> Resolve(This,fFlags)

#define IShellLinkDual_GetIconLocation(This,pbs,piIcon)	\
    (This)->lpVtbl -> GetIconLocation(This,pbs,piIcon)

#define IShellLinkDual_SetIconLocation(This,bs,iIcon)	\
    (This)->lpVtbl -> SetIconLocation(This,bs,iIcon)

#define IShellLinkDual_Save(This,vWhere)	\
    (This)->lpVtbl -> Save(This,vWhere)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShellLinkDual_get_Path_Proxy( 
    IShellLinkDual __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbs);


void __RPC_STUB IShellLinkDual_get_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE IShellLinkDual_put_Path_Proxy( 
    IShellLinkDual __RPC_FAR * This,
    /* [in] */ BSTR bs);


void __RPC_STUB IShellLinkDual_put_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShellLinkDual_get_Description_Proxy( 
    IShellLinkDual __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbs);


void __RPC_STUB IShellLinkDual_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE IShellLinkDual_put_Description_Proxy( 
    IShellLinkDual __RPC_FAR * This,
    /* [in] */ BSTR bs);


void __RPC_STUB IShellLinkDual_put_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShellLinkDual_get_WorkingDirectory_Proxy( 
    IShellLinkDual __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbs);


void __RPC_STUB IShellLinkDual_get_WorkingDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE IShellLinkDual_put_WorkingDirectory_Proxy( 
    IShellLinkDual __RPC_FAR * This,
    /* [in] */ BSTR bs);


void __RPC_STUB IShellLinkDual_put_WorkingDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShellLinkDual_get_Arguments_Proxy( 
    IShellLinkDual __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbs);


void __RPC_STUB IShellLinkDual_get_Arguments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE IShellLinkDual_put_Arguments_Proxy( 
    IShellLinkDual __RPC_FAR * This,
    /* [in] */ BSTR bs);


void __RPC_STUB IShellLinkDual_put_Arguments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShellLinkDual_get_Hotkey_Proxy( 
    IShellLinkDual __RPC_FAR * This,
    /* [retval][out] */ int __RPC_FAR *piHK);


void __RPC_STUB IShellLinkDual_get_Hotkey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE IShellLinkDual_put_Hotkey_Proxy( 
    IShellLinkDual __RPC_FAR * This,
    /* [in] */ int iHK);


void __RPC_STUB IShellLinkDual_put_Hotkey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShellLinkDual_get_ShowCommand_Proxy( 
    IShellLinkDual __RPC_FAR * This,
    /* [retval][out] */ int __RPC_FAR *piShowCommand);


void __RPC_STUB IShellLinkDual_get_ShowCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE IShellLinkDual_put_ShowCommand_Proxy( 
    IShellLinkDual __RPC_FAR * This,
    /* [in] */ int iShowCommand);


void __RPC_STUB IShellLinkDual_put_ShowCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellLinkDual_Resolve_Proxy( 
    IShellLinkDual __RPC_FAR * This,
    /* [in] */ int fFlags);


void __RPC_STUB IShellLinkDual_Resolve_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellLinkDual_GetIconLocation_Proxy( 
    IShellLinkDual __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbs,
    /* [retval][out] */ int __RPC_FAR *piIcon);


void __RPC_STUB IShellLinkDual_GetIconLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellLinkDual_SetIconLocation_Proxy( 
    IShellLinkDual __RPC_FAR * This,
    /* [in] */ BSTR bs,
    /* [in] */ int iIcon);


void __RPC_STUB IShellLinkDual_SetIconLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellLinkDual_Save_Proxy( 
    IShellLinkDual __RPC_FAR * This,
    /* [optional][in] */ VARIANT vWhere);


void __RPC_STUB IShellLinkDual_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IShellLinkDual_INTERFACE_DEFINED__ */


#ifndef __IShellLinkDual2_INTERFACE_DEFINED__
#define __IShellLinkDual2_INTERFACE_DEFINED__

/* interface IShellLinkDual2 */
/* [object][hidden][dual][oleautomation][helpstring][uuid] */ 


EXTERN_C const IID IID_IShellLinkDual2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("317EE249-F12E-11d2-B1E4-00C04F8EEB3E")
    IShellLinkDual2 : public IShellLinkDual
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Target( 
            /* [retval][out] */ FolderItem __RPC_FAR *__RPC_FAR *ppfi) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IShellLinkDual2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IShellLinkDual2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IShellLinkDual2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Path )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Path )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [in] */ BSTR bs);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Description )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Description )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [in] */ BSTR bs);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_WorkingDirectory )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_WorkingDirectory )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [in] */ BSTR bs);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Arguments )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Arguments )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [in] */ BSTR bs);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Hotkey )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [retval][out] */ int __RPC_FAR *piHK);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Hotkey )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [in] */ int iHK);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShowCommand )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [retval][out] */ int __RPC_FAR *piShowCommand);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ShowCommand )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [in] */ int iShowCommand);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Resolve )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [in] */ int fFlags);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIconLocation )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbs,
            /* [retval][out] */ int __RPC_FAR *piIcon);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetIconLocation )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [in] */ BSTR bs,
            /* [in] */ int iIcon);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Save )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [optional][in] */ VARIANT vWhere);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Target )( 
            IShellLinkDual2 __RPC_FAR * This,
            /* [retval][out] */ FolderItem __RPC_FAR *__RPC_FAR *ppfi);
        
        END_INTERFACE
    } IShellLinkDual2Vtbl;

    interface IShellLinkDual2
    {
        CONST_VTBL struct IShellLinkDual2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellLinkDual2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellLinkDual2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellLinkDual2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellLinkDual2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IShellLinkDual2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IShellLinkDual2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IShellLinkDual2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IShellLinkDual2_get_Path(This,pbs)	\
    (This)->lpVtbl -> get_Path(This,pbs)

#define IShellLinkDual2_put_Path(This,bs)	\
    (This)->lpVtbl -> put_Path(This,bs)

#define IShellLinkDual2_get_Description(This,pbs)	\
    (This)->lpVtbl -> get_Description(This,pbs)

#define IShellLinkDual2_put_Description(This,bs)	\
    (This)->lpVtbl -> put_Description(This,bs)

#define IShellLinkDual2_get_WorkingDirectory(This,pbs)	\
    (This)->lpVtbl -> get_WorkingDirectory(This,pbs)

#define IShellLinkDual2_put_WorkingDirectory(This,bs)	\
    (This)->lpVtbl -> put_WorkingDirectory(This,bs)

#define IShellLinkDual2_get_Arguments(This,pbs)	\
    (This)->lpVtbl -> get_Arguments(This,pbs)

#define IShellLinkDual2_put_Arguments(This,bs)	\
    (This)->lpVtbl -> put_Arguments(This,bs)

#define IShellLinkDual2_get_Hotkey(This,piHK)	\
    (This)->lpVtbl -> get_Hotkey(This,piHK)

#define IShellLinkDual2_put_Hotkey(This,iHK)	\
    (This)->lpVtbl -> put_Hotkey(This,iHK)

#define IShellLinkDual2_get_ShowCommand(This,piShowCommand)	\
    (This)->lpVtbl -> get_ShowCommand(This,piShowCommand)

#define IShellLinkDual2_put_ShowCommand(This,iShowCommand)	\
    (This)->lpVtbl -> put_ShowCommand(This,iShowCommand)

#define IShellLinkDual2_Resolve(This,fFlags)	\
    (This)->lpVtbl -> Resolve(This,fFlags)

#define IShellLinkDual2_GetIconLocation(This,pbs,piIcon)	\
    (This)->lpVtbl -> GetIconLocation(This,pbs,piIcon)

#define IShellLinkDual2_SetIconLocation(This,bs,iIcon)	\
    (This)->lpVtbl -> SetIconLocation(This,bs,iIcon)

#define IShellLinkDual2_Save(This,vWhere)	\
    (This)->lpVtbl -> Save(This,vWhere)


#define IShellLinkDual2_get_Target(This,ppfi)	\
    (This)->lpVtbl -> get_Target(This,ppfi)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShellLinkDual2_get_Target_Proxy( 
    IShellLinkDual2 __RPC_FAR * This,
    /* [retval][out] */ FolderItem __RPC_FAR *__RPC_FAR *ppfi);


void __RPC_STUB IShellLinkDual2_get_Target_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IShellLinkDual2_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_ShellLinkObject;

#ifdef __cplusplus

class DECLSPEC_UUID("11219420-1768-11d1-95BE-00609797EA4F")
ShellLinkObject;
#endif

#ifndef __IShellFolderViewDual_INTERFACE_DEFINED__
#define __IShellFolderViewDual_INTERFACE_DEFINED__

/* interface IShellFolderViewDual */
/* [object][dual][hidden][oleautomation][helpstring][uuid] */ 


EXTERN_C const IID IID_IShellFolderViewDual;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E7A1AF80-4D96-11CF-960C-0080C7F4EE85")
    IShellFolderViewDual : public IDispatch
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Folder( 
            /* [retval][out] */ Folder __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SelectedItems( 
            /* [retval][out] */ FolderItems __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_FocusedItem( 
            /* [retval][out] */ FolderItem __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SelectItem( 
            /* [in] */ VARIANT __RPC_FAR *pvfi,
            /* [in] */ int dwFlags) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE PopupItemMenu( 
            /* [in] */ FolderItem __RPC_FAR *pfi,
            /* [optional][in] */ VARIANT vx,
            /* [optional][in] */ VARIANT vy,
            /* [retval][out] */ BSTR __RPC_FAR *pbs) = 0;
        
        virtual /* [helpcontext][helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Script( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp) = 0;
        
        virtual /* [helpcontext][helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_ViewOptions( 
            /* [retval][out] */ long __RPC_FAR *plViewOptions) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IShellFolderViewDualVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IShellFolderViewDual __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IShellFolderViewDual __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IShellFolderViewDual __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IShellFolderViewDual __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IShellFolderViewDual __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IShellFolderViewDual __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IShellFolderViewDual __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            IShellFolderViewDual __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            IShellFolderViewDual __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Folder )( 
            IShellFolderViewDual __RPC_FAR * This,
            /* [retval][out] */ Folder __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SelectedItems )( 
            IShellFolderViewDual __RPC_FAR * This,
            /* [retval][out] */ FolderItems __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FocusedItem )( 
            IShellFolderViewDual __RPC_FAR * This,
            /* [retval][out] */ FolderItem __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SelectItem )( 
            IShellFolderViewDual __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *pvfi,
            /* [in] */ int dwFlags);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PopupItemMenu )( 
            IShellFolderViewDual __RPC_FAR * This,
            /* [in] */ FolderItem __RPC_FAR *pfi,
            /* [optional][in] */ VARIANT vx,
            /* [optional][in] */ VARIANT vy,
            /* [retval][out] */ BSTR __RPC_FAR *pbs);
        
        /* [helpcontext][helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Script )( 
            IShellFolderViewDual __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
        /* [helpcontext][helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ViewOptions )( 
            IShellFolderViewDual __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plViewOptions);
        
        END_INTERFACE
    } IShellFolderViewDualVtbl;

    interface IShellFolderViewDual
    {
        CONST_VTBL struct IShellFolderViewDualVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellFolderViewDual_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellFolderViewDual_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellFolderViewDual_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellFolderViewDual_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IShellFolderViewDual_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IShellFolderViewDual_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IShellFolderViewDual_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IShellFolderViewDual_get_Application(This,ppid)	\
    (This)->lpVtbl -> get_Application(This,ppid)

#define IShellFolderViewDual_get_Parent(This,ppid)	\
    (This)->lpVtbl -> get_Parent(This,ppid)

#define IShellFolderViewDual_get_Folder(This,ppid)	\
    (This)->lpVtbl -> get_Folder(This,ppid)

#define IShellFolderViewDual_SelectedItems(This,ppid)	\
    (This)->lpVtbl -> SelectedItems(This,ppid)

#define IShellFolderViewDual_get_FocusedItem(This,ppid)	\
    (This)->lpVtbl -> get_FocusedItem(This,ppid)

#define IShellFolderViewDual_SelectItem(This,pvfi,dwFlags)	\
    (This)->lpVtbl -> SelectItem(This,pvfi,dwFlags)

#define IShellFolderViewDual_PopupItemMenu(This,pfi,vx,vy,pbs)	\
    (This)->lpVtbl -> PopupItemMenu(This,pfi,vx,vy,pbs)

#define IShellFolderViewDual_get_Script(This,ppDisp)	\
    (This)->lpVtbl -> get_Script(This,ppDisp)

#define IShellFolderViewDual_get_ViewOptions(This,plViewOptions)	\
    (This)->lpVtbl -> get_ViewOptions(This,plViewOptions)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShellFolderViewDual_get_Application_Proxy( 
    IShellFolderViewDual __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB IShellFolderViewDual_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShellFolderViewDual_get_Parent_Proxy( 
    IShellFolderViewDual __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB IShellFolderViewDual_get_Parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShellFolderViewDual_get_Folder_Proxy( 
    IShellFolderViewDual __RPC_FAR * This,
    /* [retval][out] */ Folder __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB IShellFolderViewDual_get_Folder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellFolderViewDual_SelectedItems_Proxy( 
    IShellFolderViewDual __RPC_FAR * This,
    /* [retval][out] */ FolderItems __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB IShellFolderViewDual_SelectedItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShellFolderViewDual_get_FocusedItem_Proxy( 
    IShellFolderViewDual __RPC_FAR * This,
    /* [retval][out] */ FolderItem __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB IShellFolderViewDual_get_FocusedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellFolderViewDual_SelectItem_Proxy( 
    IShellFolderViewDual __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *pvfi,
    /* [in] */ int dwFlags);


void __RPC_STUB IShellFolderViewDual_SelectItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellFolderViewDual_PopupItemMenu_Proxy( 
    IShellFolderViewDual __RPC_FAR * This,
    /* [in] */ FolderItem __RPC_FAR *pfi,
    /* [optional][in] */ VARIANT vx,
    /* [optional][in] */ VARIANT vy,
    /* [retval][out] */ BSTR __RPC_FAR *pbs);


void __RPC_STUB IShellFolderViewDual_PopupItemMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShellFolderViewDual_get_Script_Proxy( 
    IShellFolderViewDual __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);


void __RPC_STUB IShellFolderViewDual_get_Script_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShellFolderViewDual_get_ViewOptions_Proxy( 
    IShellFolderViewDual __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plViewOptions);


void __RPC_STUB IShellFolderViewDual_get_ViewOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IShellFolderViewDual_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_ShellFolderView;

#ifdef __cplusplus

class DECLSPEC_UUID("62112AA1-EBE4-11cf-A5FB-0020AFE7292D")
ShellFolderView;
#endif

#ifndef __IShellDispatch_INTERFACE_DEFINED__
#define __IShellDispatch_INTERFACE_DEFINED__

/* interface IShellDispatch */
/* [object][dual][hidden][oleautomation][helpstring][uuid] */ 


EXTERN_C const IID IID_IShellDispatch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D8F015C0-C278-11CE-A49E-444553540000")
    IShellDispatch : public IDispatch
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE NameSpace( 
            /* [in] */ VARIANT vDir,
            /* [retval][out] */ Folder __RPC_FAR *__RPC_FAR *ppsdf) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE BrowseForFolder( 
            /* [in] */ long Hwnd,
            /* [in] */ BSTR Title,
            /* [in] */ long Options,
            /* [optional][in] */ VARIANT RootFolder,
            /* [retval][out] */ Folder __RPC_FAR *__RPC_FAR *ppsdf) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Windows( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Open( 
            /* [in] */ VARIANT vDir) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Explore( 
            /* [in] */ VARIANT vDir) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE MinimizeAll( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE UndoMinimizeALL( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FileRun( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CascadeWindows( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE TileVertically( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE TileHorizontally( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ShutdownWindows( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Suspend( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EjectPC( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetTime( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE TrayProperties( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Help( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FindFiles( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FindComputer( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RefreshMenu( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ControlPanelItem( 
            /* [in] */ BSTR szDir) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IShellDispatchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IShellDispatch __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IShellDispatch __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IShellDispatch __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IShellDispatch __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IShellDispatch __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IShellDispatch __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IShellDispatch __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            IShellDispatch __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            IShellDispatch __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NameSpace )( 
            IShellDispatch __RPC_FAR * This,
            /* [in] */ VARIANT vDir,
            /* [retval][out] */ Folder __RPC_FAR *__RPC_FAR *ppsdf);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BrowseForFolder )( 
            IShellDispatch __RPC_FAR * This,
            /* [in] */ long Hwnd,
            /* [in] */ BSTR Title,
            /* [in] */ long Options,
            /* [optional][in] */ VARIANT RootFolder,
            /* [retval][out] */ Folder __RPC_FAR *__RPC_FAR *ppsdf);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Windows )( 
            IShellDispatch __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Open )( 
            IShellDispatch __RPC_FAR * This,
            /* [in] */ VARIANT vDir);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Explore )( 
            IShellDispatch __RPC_FAR * This,
            /* [in] */ VARIANT vDir);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MinimizeAll )( 
            IShellDispatch __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UndoMinimizeALL )( 
            IShellDispatch __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FileRun )( 
            IShellDispatch __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CascadeWindows )( 
            IShellDispatch __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TileVertically )( 
            IShellDispatch __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TileHorizontally )( 
            IShellDispatch __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShutdownWindows )( 
            IShellDispatch __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Suspend )( 
            IShellDispatch __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EjectPC )( 
            IShellDispatch __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetTime )( 
            IShellDispatch __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TrayProperties )( 
            IShellDispatch __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Help )( 
            IShellDispatch __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindFiles )( 
            IShellDispatch __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindComputer )( 
            IShellDispatch __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RefreshMenu )( 
            IShellDispatch __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ControlPanelItem )( 
            IShellDispatch __RPC_FAR * This,
            /* [in] */ BSTR szDir);
        
        END_INTERFACE
    } IShellDispatchVtbl;

    interface IShellDispatch
    {
        CONST_VTBL struct IShellDispatchVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellDispatch_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellDispatch_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellDispatch_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellDispatch_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IShellDispatch_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IShellDispatch_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IShellDispatch_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IShellDispatch_get_Application(This,ppid)	\
    (This)->lpVtbl -> get_Application(This,ppid)

#define IShellDispatch_get_Parent(This,ppid)	\
    (This)->lpVtbl -> get_Parent(This,ppid)

#define IShellDispatch_NameSpace(This,vDir,ppsdf)	\
    (This)->lpVtbl -> NameSpace(This,vDir,ppsdf)

#define IShellDispatch_BrowseForFolder(This,Hwnd,Title,Options,RootFolder,ppsdf)	\
    (This)->lpVtbl -> BrowseForFolder(This,Hwnd,Title,Options,RootFolder,ppsdf)

#define IShellDispatch_Windows(This,ppid)	\
    (This)->lpVtbl -> Windows(This,ppid)

#define IShellDispatch_Open(This,vDir)	\
    (This)->lpVtbl -> Open(This,vDir)

#define IShellDispatch_Explore(This,vDir)	\
    (This)->lpVtbl -> Explore(This,vDir)

#define IShellDispatch_MinimizeAll(This)	\
    (This)->lpVtbl -> MinimizeAll(This)

#define IShellDispatch_UndoMinimizeALL(This)	\
    (This)->lpVtbl -> UndoMinimizeALL(This)

#define IShellDispatch_FileRun(This)	\
    (This)->lpVtbl -> FileRun(This)

#define IShellDispatch_CascadeWindows(This)	\
    (This)->lpVtbl -> CascadeWindows(This)

#define IShellDispatch_TileVertically(This)	\
    (This)->lpVtbl -> TileVertically(This)

#define IShellDispatch_TileHorizontally(This)	\
    (This)->lpVtbl -> TileHorizontally(This)

#define IShellDispatch_ShutdownWindows(This)	\
    (This)->lpVtbl -> ShutdownWindows(This)

#define IShellDispatch_Suspend(This)	\
    (This)->lpVtbl -> Suspend(This)

#define IShellDispatch_EjectPC(This)	\
    (This)->lpVtbl -> EjectPC(This)

#define IShellDispatch_SetTime(This)	\
    (This)->lpVtbl -> SetTime(This)

#define IShellDispatch_TrayProperties(This)	\
    (This)->lpVtbl -> TrayProperties(This)

#define IShellDispatch_Help(This)	\
    (This)->lpVtbl -> Help(This)

#define IShellDispatch_FindFiles(This)	\
    (This)->lpVtbl -> FindFiles(This)

#define IShellDispatch_FindComputer(This)	\
    (This)->lpVtbl -> FindComputer(This)

#define IShellDispatch_RefreshMenu(This)	\
    (This)->lpVtbl -> RefreshMenu(This)

#define IShellDispatch_ControlPanelItem(This,szDir)	\
    (This)->lpVtbl -> ControlPanelItem(This,szDir)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShellDispatch_get_Application_Proxy( 
    IShellDispatch __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB IShellDispatch_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShellDispatch_get_Parent_Proxy( 
    IShellDispatch __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB IShellDispatch_get_Parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_NameSpace_Proxy( 
    IShellDispatch __RPC_FAR * This,
    /* [in] */ VARIANT vDir,
    /* [retval][out] */ Folder __RPC_FAR *__RPC_FAR *ppsdf);


void __RPC_STUB IShellDispatch_NameSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_BrowseForFolder_Proxy( 
    IShellDispatch __RPC_FAR * This,
    /* [in] */ long Hwnd,
    /* [in] */ BSTR Title,
    /* [in] */ long Options,
    /* [optional][in] */ VARIANT RootFolder,
    /* [retval][out] */ Folder __RPC_FAR *__RPC_FAR *ppsdf);


void __RPC_STUB IShellDispatch_BrowseForFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_Windows_Proxy( 
    IShellDispatch __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB IShellDispatch_Windows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_Open_Proxy( 
    IShellDispatch __RPC_FAR * This,
    /* [in] */ VARIANT vDir);


void __RPC_STUB IShellDispatch_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_Explore_Proxy( 
    IShellDispatch __RPC_FAR * This,
    /* [in] */ VARIANT vDir);


void __RPC_STUB IShellDispatch_Explore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_MinimizeAll_Proxy( 
    IShellDispatch __RPC_FAR * This);


void __RPC_STUB IShellDispatch_MinimizeAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_UndoMinimizeALL_Proxy( 
    IShellDispatch __RPC_FAR * This);


void __RPC_STUB IShellDispatch_UndoMinimizeALL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_FileRun_Proxy( 
    IShellDispatch __RPC_FAR * This);


void __RPC_STUB IShellDispatch_FileRun_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_CascadeWindows_Proxy( 
    IShellDispatch __RPC_FAR * This);


void __RPC_STUB IShellDispatch_CascadeWindows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_TileVertically_Proxy( 
    IShellDispatch __RPC_FAR * This);


void __RPC_STUB IShellDispatch_TileVertically_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_TileHorizontally_Proxy( 
    IShellDispatch __RPC_FAR * This);


void __RPC_STUB IShellDispatch_TileHorizontally_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_ShutdownWindows_Proxy( 
    IShellDispatch __RPC_FAR * This);


void __RPC_STUB IShellDispatch_ShutdownWindows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_Suspend_Proxy( 
    IShellDispatch __RPC_FAR * This);


void __RPC_STUB IShellDispatch_Suspend_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_EjectPC_Proxy( 
    IShellDispatch __RPC_FAR * This);


void __RPC_STUB IShellDispatch_EjectPC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_SetTime_Proxy( 
    IShellDispatch __RPC_FAR * This);


void __RPC_STUB IShellDispatch_SetTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_TrayProperties_Proxy( 
    IShellDispatch __RPC_FAR * This);


void __RPC_STUB IShellDispatch_TrayProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_Help_Proxy( 
    IShellDispatch __RPC_FAR * This);


void __RPC_STUB IShellDispatch_Help_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_FindFiles_Proxy( 
    IShellDispatch __RPC_FAR * This);


void __RPC_STUB IShellDispatch_FindFiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_FindComputer_Proxy( 
    IShellDispatch __RPC_FAR * This);


void __RPC_STUB IShellDispatch_FindComputer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_RefreshMenu_Proxy( 
    IShellDispatch __RPC_FAR * This);


void __RPC_STUB IShellDispatch_RefreshMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch_ControlPanelItem_Proxy( 
    IShellDispatch __RPC_FAR * This,
    /* [in] */ BSTR szDir);


void __RPC_STUB IShellDispatch_ControlPanelItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IShellDispatch_INTERFACE_DEFINED__ */


#ifndef __IShellDispatch2_INTERFACE_DEFINED__
#define __IShellDispatch2_INTERFACE_DEFINED__

/* interface IShellDispatch2 */
/* [object][dual][hidden][oleautomation][helpstring][uuid] */ 


EXTERN_C const IID IID_IShellDispatch2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A4C6892C-3BA9-11d2-9DEA-00C04FB16162")
    IShellDispatch2 : public IShellDispatch
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsRestricted( 
            /* [in] */ BSTR Group,
            /* [in] */ BSTR Restriction,
            /* [retval][out] */ long __RPC_FAR *plRestrictValue) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ShellExecute( 
            /* [in] */ BSTR File,
            /* [optional][in] */ VARIANT vArgs,
            /* [optional][in] */ VARIANT vDir,
            /* [optional][in] */ VARIANT vOperation,
            /* [optional][in] */ VARIANT vShow) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FindPrinter( 
            /* [optional][in] */ BSTR name,
            /* [optional][in] */ BSTR location,
            /* [optional][in] */ BSTR model) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSystemInformation( 
            /* [in] */ BSTR name,
            /* [retval][out] */ VARIANT __RPC_FAR *pv) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ServiceStart( 
            /* [in] */ BSTR ServiceName,
            /* [in] */ VARIANT Persistent,
            /* [retval][out] */ VARIANT __RPC_FAR *pSuccess) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ServiceStop( 
            /* [in] */ BSTR ServiceName,
            /* [in] */ VARIANT Persistent,
            /* [retval][out] */ VARIANT __RPC_FAR *pSuccess) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsServiceRunning( 
            /* [in] */ BSTR ServiceName,
            /* [retval][out] */ VARIANT __RPC_FAR *pRunning) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CanStartStopService( 
            /* [in] */ BSTR ServiceName,
            /* [retval][out] */ VARIANT __RPC_FAR *pCanStartStop) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ShowBrowserBar( 
            /* [in] */ BSTR bstrClsid,
            /* [in] */ VARIANT bShow,
            /* [retval][out] */ VARIANT __RPC_FAR *pSuccess) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IShellDispatch2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IShellDispatch2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IShellDispatch2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Application )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Parent )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NameSpace )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [in] */ VARIANT vDir,
            /* [retval][out] */ Folder __RPC_FAR *__RPC_FAR *ppsdf);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BrowseForFolder )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [in] */ long Hwnd,
            /* [in] */ BSTR Title,
            /* [in] */ long Options,
            /* [optional][in] */ VARIANT RootFolder,
            /* [retval][out] */ Folder __RPC_FAR *__RPC_FAR *ppsdf);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Windows )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Open )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [in] */ VARIANT vDir);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Explore )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [in] */ VARIANT vDir);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MinimizeAll )( 
            IShellDispatch2 __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UndoMinimizeALL )( 
            IShellDispatch2 __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FileRun )( 
            IShellDispatch2 __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CascadeWindows )( 
            IShellDispatch2 __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TileVertically )( 
            IShellDispatch2 __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TileHorizontally )( 
            IShellDispatch2 __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShutdownWindows )( 
            IShellDispatch2 __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Suspend )( 
            IShellDispatch2 __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EjectPC )( 
            IShellDispatch2 __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetTime )( 
            IShellDispatch2 __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TrayProperties )( 
            IShellDispatch2 __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Help )( 
            IShellDispatch2 __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindFiles )( 
            IShellDispatch2 __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindComputer )( 
            IShellDispatch2 __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RefreshMenu )( 
            IShellDispatch2 __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ControlPanelItem )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [in] */ BSTR szDir);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsRestricted )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [in] */ BSTR Group,
            /* [in] */ BSTR Restriction,
            /* [retval][out] */ long __RPC_FAR *plRestrictValue);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShellExecute )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [in] */ BSTR File,
            /* [optional][in] */ VARIANT vArgs,
            /* [optional][in] */ VARIANT vDir,
            /* [optional][in] */ VARIANT vOperation,
            /* [optional][in] */ VARIANT vShow);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindPrinter )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [optional][in] */ BSTR name,
            /* [optional][in] */ BSTR location,
            /* [optional][in] */ BSTR model);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSystemInformation )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [in] */ BSTR name,
            /* [retval][out] */ VARIANT __RPC_FAR *pv);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ServiceStart )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [in] */ BSTR ServiceName,
            /* [in] */ VARIANT Persistent,
            /* [retval][out] */ VARIANT __RPC_FAR *pSuccess);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ServiceStop )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [in] */ BSTR ServiceName,
            /* [in] */ VARIANT Persistent,
            /* [retval][out] */ VARIANT __RPC_FAR *pSuccess);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsServiceRunning )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [in] */ BSTR ServiceName,
            /* [retval][out] */ VARIANT __RPC_FAR *pRunning);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CanStartStopService )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [in] */ BSTR ServiceName,
            /* [retval][out] */ VARIANT __RPC_FAR *pCanStartStop);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShowBrowserBar )( 
            IShellDispatch2 __RPC_FAR * This,
            /* [in] */ BSTR bstrClsid,
            /* [in] */ VARIANT bShow,
            /* [retval][out] */ VARIANT __RPC_FAR *pSuccess);
        
        END_INTERFACE
    } IShellDispatch2Vtbl;

    interface IShellDispatch2
    {
        CONST_VTBL struct IShellDispatch2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellDispatch2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellDispatch2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellDispatch2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellDispatch2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IShellDispatch2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IShellDispatch2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IShellDispatch2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IShellDispatch2_get_Application(This,ppid)	\
    (This)->lpVtbl -> get_Application(This,ppid)

#define IShellDispatch2_get_Parent(This,ppid)	\
    (This)->lpVtbl -> get_Parent(This,ppid)

#define IShellDispatch2_NameSpace(This,vDir,ppsdf)	\
    (This)->lpVtbl -> NameSpace(This,vDir,ppsdf)

#define IShellDispatch2_BrowseForFolder(This,Hwnd,Title,Options,RootFolder,ppsdf)	\
    (This)->lpVtbl -> BrowseForFolder(This,Hwnd,Title,Options,RootFolder,ppsdf)

#define IShellDispatch2_Windows(This,ppid)	\
    (This)->lpVtbl -> Windows(This,ppid)

#define IShellDispatch2_Open(This,vDir)	\
    (This)->lpVtbl -> Open(This,vDir)

#define IShellDispatch2_Explore(This,vDir)	\
    (This)->lpVtbl -> Explore(This,vDir)

#define IShellDispatch2_MinimizeAll(This)	\
    (This)->lpVtbl -> MinimizeAll(This)

#define IShellDispatch2_UndoMinimizeALL(This)	\
    (This)->lpVtbl -> UndoMinimizeALL(This)

#define IShellDispatch2_FileRun(This)	\
    (This)->lpVtbl -> FileRun(This)

#define IShellDispatch2_CascadeWindows(This)	\
    (This)->lpVtbl -> CascadeWindows(This)

#define IShellDispatch2_TileVertically(This)	\
    (This)->lpVtbl -> TileVertically(This)

#define IShellDispatch2_TileHorizontally(This)	\
    (This)->lpVtbl -> TileHorizontally(This)

#define IShellDispatch2_ShutdownWindows(This)	\
    (This)->lpVtbl -> ShutdownWindows(This)

#define IShellDispatch2_Suspend(This)	\
    (This)->lpVtbl -> Suspend(This)

#define IShellDispatch2_EjectPC(This)	\
    (This)->lpVtbl -> EjectPC(This)

#define IShellDispatch2_SetTime(This)	\
    (This)->lpVtbl -> SetTime(This)

#define IShellDispatch2_TrayProperties(This)	\
    (This)->lpVtbl -> TrayProperties(This)

#define IShellDispatch2_Help(This)	\
    (This)->lpVtbl -> Help(This)

#define IShellDispatch2_FindFiles(This)	\
    (This)->lpVtbl -> FindFiles(This)

#define IShellDispatch2_FindComputer(This)	\
    (This)->lpVtbl -> FindComputer(This)

#define IShellDispatch2_RefreshMenu(This)	\
    (This)->lpVtbl -> RefreshMenu(This)

#define IShellDispatch2_ControlPanelItem(This,szDir)	\
    (This)->lpVtbl -> ControlPanelItem(This,szDir)


#define IShellDispatch2_IsRestricted(This,Group,Restriction,plRestrictValue)	\
    (This)->lpVtbl -> IsRestricted(This,Group,Restriction,plRestrictValue)

#define IShellDispatch2_ShellExecute(This,File,vArgs,vDir,vOperation,vShow)	\
    (This)->lpVtbl -> ShellExecute(This,File,vArgs,vDir,vOperation,vShow)

#define IShellDispatch2_FindPrinter(This,name,location,model)	\
    (This)->lpVtbl -> FindPrinter(This,name,location,model)

#define IShellDispatch2_GetSystemInformation(This,name,pv)	\
    (This)->lpVtbl -> GetSystemInformation(This,name,pv)

#define IShellDispatch2_ServiceStart(This,ServiceName,Persistent,pSuccess)	\
    (This)->lpVtbl -> ServiceStart(This,ServiceName,Persistent,pSuccess)

#define IShellDispatch2_ServiceStop(This,ServiceName,Persistent,pSuccess)	\
    (This)->lpVtbl -> ServiceStop(This,ServiceName,Persistent,pSuccess)

#define IShellDispatch2_IsServiceRunning(This,ServiceName,pRunning)	\
    (This)->lpVtbl -> IsServiceRunning(This,ServiceName,pRunning)

#define IShellDispatch2_CanStartStopService(This,ServiceName,pCanStartStop)	\
    (This)->lpVtbl -> CanStartStopService(This,ServiceName,pCanStartStop)

#define IShellDispatch2_ShowBrowserBar(This,bstrClsid,bShow,pSuccess)	\
    (This)->lpVtbl -> ShowBrowserBar(This,bstrClsid,bShow,pSuccess)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch2_IsRestricted_Proxy( 
    IShellDispatch2 __RPC_FAR * This,
    /* [in] */ BSTR Group,
    /* [in] */ BSTR Restriction,
    /* [retval][out] */ long __RPC_FAR *plRestrictValue);


void __RPC_STUB IShellDispatch2_IsRestricted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch2_ShellExecute_Proxy( 
    IShellDispatch2 __RPC_FAR * This,
    /* [in] */ BSTR File,
    /* [optional][in] */ VARIANT vArgs,
    /* [optional][in] */ VARIANT vDir,
    /* [optional][in] */ VARIANT vOperation,
    /* [optional][in] */ VARIANT vShow);


void __RPC_STUB IShellDispatch2_ShellExecute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch2_FindPrinter_Proxy( 
    IShellDispatch2 __RPC_FAR * This,
    /* [optional][in] */ BSTR name,
    /* [optional][in] */ BSTR location,
    /* [optional][in] */ BSTR model);


void __RPC_STUB IShellDispatch2_FindPrinter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch2_GetSystemInformation_Proxy( 
    IShellDispatch2 __RPC_FAR * This,
    /* [in] */ BSTR name,
    /* [retval][out] */ VARIANT __RPC_FAR *pv);


void __RPC_STUB IShellDispatch2_GetSystemInformation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch2_ServiceStart_Proxy( 
    IShellDispatch2 __RPC_FAR * This,
    /* [in] */ BSTR ServiceName,
    /* [in] */ VARIANT Persistent,
    /* [retval][out] */ VARIANT __RPC_FAR *pSuccess);


void __RPC_STUB IShellDispatch2_ServiceStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch2_ServiceStop_Proxy( 
    IShellDispatch2 __RPC_FAR * This,
    /* [in] */ BSTR ServiceName,
    /* [in] */ VARIANT Persistent,
    /* [retval][out] */ VARIANT __RPC_FAR *pSuccess);


void __RPC_STUB IShellDispatch2_ServiceStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch2_IsServiceRunning_Proxy( 
    IShellDispatch2 __RPC_FAR * This,
    /* [in] */ BSTR ServiceName,
    /* [retval][out] */ VARIANT __RPC_FAR *pRunning);


void __RPC_STUB IShellDispatch2_IsServiceRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch2_CanStartStopService_Proxy( 
    IShellDispatch2 __RPC_FAR * This,
    /* [in] */ BSTR ServiceName,
    /* [retval][out] */ VARIANT __RPC_FAR *pCanStartStop);


void __RPC_STUB IShellDispatch2_CanStartStopService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShellDispatch2_ShowBrowserBar_Proxy( 
    IShellDispatch2 __RPC_FAR * This,
    /* [in] */ BSTR bstrClsid,
    /* [in] */ VARIANT bShow,
    /* [retval][out] */ VARIANT __RPC_FAR *pSuccess);


void __RPC_STUB IShellDispatch2_ShowBrowserBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IShellDispatch2_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_Shell;

#ifdef __cplusplus

class DECLSPEC_UUID("13709620-C279-11CE-A49E-444553540000")
Shell;
#endif

EXTERN_C const CLSID CLSID_ShellDispatchInproc;

#ifdef __cplusplus

class DECLSPEC_UUID("0A89A860-D7B1-11CE-8350-444553540000")
ShellDispatchInproc;
#endif

EXTERN_C const CLSID CLSID_WebViewFolderContents;

#ifdef __cplusplus

class DECLSPEC_UUID("1820FED0-473E-11D0-A96C-00C04FD705A2")
WebViewFolderContents;
#endif

#ifndef __DSearchCommandEvents_DISPINTERFACE_DEFINED__
#define __DSearchCommandEvents_DISPINTERFACE_DEFINED__

/* dispinterface DSearchCommandEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID_DSearchCommandEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("60890160-69f0-11d1-b758-00a0c90564fe")
    DSearchCommandEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct DSearchCommandEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            DSearchCommandEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            DSearchCommandEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            DSearchCommandEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            DSearchCommandEvents __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            DSearchCommandEvents __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            DSearchCommandEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            DSearchCommandEvents __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } DSearchCommandEventsVtbl;

    interface DSearchCommandEvents
    {
        CONST_VTBL struct DSearchCommandEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DSearchCommandEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DSearchCommandEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DSearchCommandEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DSearchCommandEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DSearchCommandEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DSearchCommandEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DSearchCommandEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __DSearchCommandEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_SearchCommand;

#ifdef __cplusplus

class DECLSPEC_UUID("B005E690-678D-11d1-B758-00A0C90564FE")
SearchCommand;
#endif

#ifndef __IFileSearchBand_INTERFACE_DEFINED__
#define __IFileSearchBand_INTERFACE_DEFINED__

/* interface IFileSearchBand */
/* [object][unique][hidden][dual][oleautomation][helpstring][uuid] */ 


EXTERN_C const IID IID_IFileSearchBand;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2D91EEA1-9932-11d2-BE86-00A0C9A83DA1")
    IFileSearchBand : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetFocus( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetSearchParameters( 
            /* [in] */ BSTR __RPC_FAR *pbstrSearchID,
            /* [in] */ VARIANT_BOOL bNavToResults,
            /* [optional][in] */ VARIANT __RPC_FAR *pvarScope,
            /* [optional][in] */ VARIANT __RPC_FAR *pvarQueryFile) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_SearchID( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrSearchID) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Scope( 
            /* [retval][out] */ VARIANT __RPC_FAR *pvarScope) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_QueryFile( 
            /* [retval][out] */ VARIANT __RPC_FAR *pvarFile) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IFileSearchBandVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IFileSearchBand __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IFileSearchBand __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IFileSearchBand __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IFileSearchBand __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IFileSearchBand __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IFileSearchBand __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IFileSearchBand __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetFocus )( 
            IFileSearchBand __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSearchParameters )( 
            IFileSearchBand __RPC_FAR * This,
            /* [in] */ BSTR __RPC_FAR *pbstrSearchID,
            /* [in] */ VARIANT_BOOL bNavToResults,
            /* [optional][in] */ VARIANT __RPC_FAR *pvarScope,
            /* [optional][in] */ VARIANT __RPC_FAR *pvarQueryFile);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SearchID )( 
            IFileSearchBand __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrSearchID);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Scope )( 
            IFileSearchBand __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *pvarScope);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_QueryFile )( 
            IFileSearchBand __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *pvarFile);
        
        END_INTERFACE
    } IFileSearchBandVtbl;

    interface IFileSearchBand
    {
        CONST_VTBL struct IFileSearchBandVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFileSearchBand_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFileSearchBand_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFileSearchBand_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFileSearchBand_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFileSearchBand_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFileSearchBand_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFileSearchBand_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFileSearchBand_SetFocus(This)	\
    (This)->lpVtbl -> SetFocus(This)

#define IFileSearchBand_SetSearchParameters(This,pbstrSearchID,bNavToResults,pvarScope,pvarQueryFile)	\
    (This)->lpVtbl -> SetSearchParameters(This,pbstrSearchID,bNavToResults,pvarScope,pvarQueryFile)

#define IFileSearchBand_get_SearchID(This,pbstrSearchID)	\
    (This)->lpVtbl -> get_SearchID(This,pbstrSearchID)

#define IFileSearchBand_get_Scope(This,pvarScope)	\
    (This)->lpVtbl -> get_Scope(This,pvarScope)

#define IFileSearchBand_get_QueryFile(This,pvarFile)	\
    (This)->lpVtbl -> get_QueryFile(This,pvarFile)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IFileSearchBand_SetFocus_Proxy( 
    IFileSearchBand __RPC_FAR * This);


void __RPC_STUB IFileSearchBand_SetFocus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IFileSearchBand_SetSearchParameters_Proxy( 
    IFileSearchBand __RPC_FAR * This,
    /* [in] */ BSTR __RPC_FAR *pbstrSearchID,
    /* [in] */ VARIANT_BOOL bNavToResults,
    /* [optional][in] */ VARIANT __RPC_FAR *pvarScope,
    /* [optional][in] */ VARIANT __RPC_FAR *pvarQueryFile);


void __RPC_STUB IFileSearchBand_SetSearchParameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IFileSearchBand_get_SearchID_Proxy( 
    IFileSearchBand __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrSearchID);


void __RPC_STUB IFileSearchBand_get_SearchID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IFileSearchBand_get_Scope_Proxy( 
    IFileSearchBand __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *pvarScope);


void __RPC_STUB IFileSearchBand_get_Scope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IFileSearchBand_get_QueryFile_Proxy( 
    IFileSearchBand __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *pvarFile);


void __RPC_STUB IFileSearchBand_get_QueryFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IFileSearchBand_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_FileSearchBand;

#ifdef __cplusplus

class DECLSPEC_UUID("C4EE31F3-4768-11D2-BE5C-00A0C9A83DA1")
FileSearchBand;
#endif
#endif /* __Shell32_LIBRARY_DEFINED__ */

/* interface __MIDL_itf_shldisp_0262 */
/* [local] */ 


//-------------------------------------------------------------------------
//
// IAutoComplete interface
//
//
// [Member functions]
//
// IAutoComplete::Init(hwndEdit, punkACL, pwszRegKeyPath, pwszQuickComplete)
//   This function initializes an AutoComplete object, telling it
//   what control to subclass, and what list of strings to process.
//
// IAutoComplete::Enable(fEnable)
//   This function enables or disables the AutoComplete functionality.
//
//-------------------------------------------------------------------------


extern RPC_IF_HANDLE __MIDL_itf_shldisp_0262_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shldisp_0262_v0_0_s_ifspec;

#ifndef __IAutoComplete_INTERFACE_DEFINED__
#define __IAutoComplete_INTERFACE_DEFINED__

/* interface IAutoComplete */
/* [unique][uuid][object][local][helpstring] */ 

typedef /* [unique] */ IAutoComplete __RPC_FAR *LPAUTOCOMPLETE;


EXTERN_C const IID IID_IAutoComplete;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00bb2762-6a77-11d0-a535-00c04fd7d062")
    IAutoComplete : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( 
            /* [in] */ HWND hwndEdit,
            /* [unique][in] */ IUnknown __RPC_FAR *punkACL,
            /* [unique][in] */ LPCOLESTR pwszRegKeyPath,
            /* [in] */ LPCOLESTR pwszQuickComplete) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enable( 
            /* [in] */ BOOL fEnable) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAutoCompleteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAutoComplete __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAutoComplete __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAutoComplete __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Init )( 
            IAutoComplete __RPC_FAR * This,
            /* [in] */ HWND hwndEdit,
            /* [unique][in] */ IUnknown __RPC_FAR *punkACL,
            /* [unique][in] */ LPCOLESTR pwszRegKeyPath,
            /* [in] */ LPCOLESTR pwszQuickComplete);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Enable )( 
            IAutoComplete __RPC_FAR * This,
            /* [in] */ BOOL fEnable);
        
        END_INTERFACE
    } IAutoCompleteVtbl;

    interface IAutoComplete
    {
        CONST_VTBL struct IAutoCompleteVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAutoComplete_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAutoComplete_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAutoComplete_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAutoComplete_Init(This,hwndEdit,punkACL,pwszRegKeyPath,pwszQuickComplete)	\
    (This)->lpVtbl -> Init(This,hwndEdit,punkACL,pwszRegKeyPath,pwszQuickComplete)

#define IAutoComplete_Enable(This,fEnable)	\
    (This)->lpVtbl -> Enable(This,fEnable)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IAutoComplete_Init_Proxy( 
    IAutoComplete __RPC_FAR * This,
    /* [in] */ HWND hwndEdit,
    /* [unique][in] */ IUnknown __RPC_FAR *punkACL,
    /* [unique][in] */ LPCOLESTR pwszRegKeyPath,
    /* [in] */ LPCOLESTR pwszQuickComplete);


void __RPC_STUB IAutoComplete_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAutoComplete_Enable_Proxy( 
    IAutoComplete __RPC_FAR * This,
    /* [in] */ BOOL fEnable);


void __RPC_STUB IAutoComplete_Enable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAutoComplete_INTERFACE_DEFINED__ */


#ifndef __IAutoComplete2_INTERFACE_DEFINED__
#define __IAutoComplete2_INTERFACE_DEFINED__

/* interface IAutoComplete2 */
/* [unique][uuid][object][local][helpstring] */ 

typedef /* [unique] */ IAutoComplete2 __RPC_FAR *LPAUTOCOMPLETE2;

typedef 
enum _tagAUTOCOMPLETEOPTIONS
    {	ACO_NONE	= 0,
	ACO_AUTOSUGGEST	= 0x1,
	ACO_AUTOAPPEND	= 0x2,
	ACO_SEARCH	= 0x4,
	ACO_FILTERPREFIXES	= 0x8,
	ACO_USETAB	= 0x10,
	ACO_UPDOWNKEYDROPSLIST	= 0x20,
	ACO_RTLREADING	= 0x40
    }	AUTOCOMPLETEOPTIONS;


EXTERN_C const IID IID_IAutoComplete2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EAC04BC0-3791-11d2-BB95-0060977B464C")
    IAutoComplete2 : public IAutoComplete
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetOptions( 
            /* [in] */ DWORD dwFlag) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOptions( 
            /* [out] */ DWORD __RPC_FAR *pdwFlag) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAutoComplete2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAutoComplete2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAutoComplete2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAutoComplete2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Init )( 
            IAutoComplete2 __RPC_FAR * This,
            /* [in] */ HWND hwndEdit,
            /* [unique][in] */ IUnknown __RPC_FAR *punkACL,
            /* [unique][in] */ LPCOLESTR pwszRegKeyPath,
            /* [in] */ LPCOLESTR pwszQuickComplete);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Enable )( 
            IAutoComplete2 __RPC_FAR * This,
            /* [in] */ BOOL fEnable);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetOptions )( 
            IAutoComplete2 __RPC_FAR * This,
            /* [in] */ DWORD dwFlag);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOptions )( 
            IAutoComplete2 __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwFlag);
        
        END_INTERFACE
    } IAutoComplete2Vtbl;

    interface IAutoComplete2
    {
        CONST_VTBL struct IAutoComplete2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAutoComplete2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAutoComplete2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAutoComplete2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAutoComplete2_Init(This,hwndEdit,punkACL,pwszRegKeyPath,pwszQuickComplete)	\
    (This)->lpVtbl -> Init(This,hwndEdit,punkACL,pwszRegKeyPath,pwszQuickComplete)

#define IAutoComplete2_Enable(This,fEnable)	\
    (This)->lpVtbl -> Enable(This,fEnable)


#define IAutoComplete2_SetOptions(This,dwFlag)	\
    (This)->lpVtbl -> SetOptions(This,dwFlag)

#define IAutoComplete2_GetOptions(This,pdwFlag)	\
    (This)->lpVtbl -> GetOptions(This,pdwFlag)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IAutoComplete2_SetOptions_Proxy( 
    IAutoComplete2 __RPC_FAR * This,
    /* [in] */ DWORD dwFlag);


void __RPC_STUB IAutoComplete2_SetOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAutoComplete2_GetOptions_Proxy( 
    IAutoComplete2 __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwFlag);


void __RPC_STUB IAutoComplete2_GetOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAutoComplete2_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_shldisp_0264 */
/* [local] */ 

// INTERFACE: IAsyncOperation
//
// This interface was implemented to turn some previously synchronous
// interfaces into async.  The following example is for
// doing the IDataObject::Drop() operation asynchronously.
//
// Sometimes the rendering of the IDataObject data (IDataObject::GetData() or
// STGMEDIUM.pStream->Read()) can be time intensive.  The IDropTarget
// may want to do this on another thread.
//
// Implimentation Check list:
// DoDragDrop Caller:
//    If this code can support asynch operations, then it needs to
//    QueryInterface() the IDataObject for IAsyncOperation.
//    IAsyncOperation::SetAsyncMode(VARIANT_TRUE).
//    After calling DoDragDrop(), call InOperation().  If any call fails
//    or InOperation() return FALSE, use the pdwEffect returned by DoDragDrop()
//    and the operation completed synchrously.
//
// OleSetClipboard Caller:
//    If this code can support asynch operations, then it needs to
//    QueryInterface() the IDataObject for IAsyncOperation.  Then call
//    IAsyncOperation::SetAsyncMode(VARIANT_TRUE).
//    If any of that fails, the final dwEffect should be passed to the IDataObject via
//    CFSTR_PERFORMEDDROPEFFECT.
//
// IDataObect Object:
//    IAsyncOperation::GetAsyncMode() should return whatever was last passed in
//          fDoOpAsync to ::SetAsyncMode() or VARIANT_FALSE if ::SetAsyncMode()
//          was never called.
//    IAsyncOperation::SetAsyncMode() should AddRef and store paocb.
//    IAsyncOperation::StartOperation() should store the fact that this was called and
//          cause InOperation() to return VARIANT_TRUE.  pbcReserved is not used and needs
//          to be NULL.
//    IAsyncOperation::InOperation() should return VARIANT_TRUE only if ::StartOperation()
//          was called.
//    IAsyncOperation::EndOperation() needs to call paocbpaocb->EndOperation() with the same
//          parameters.  Then release paocb.
//    IDataObject::SetData(CFSTR_PERFORMEDDROPEFFECT) When this happens, call
//          EndOperation(<into VAR>S_OK, NULL, <into VAR>dwEffect) and pass the dwEffect from the hglobal.
//
// IDropTarget Object:
//    IDropTarget::Drop() If asynch operations aren't supported, nothing is required.
//          The asynch operation can only happen if GetAsyncMode() returns VARIANT_TRUE.
//          Before starting the asynch operation, StartOperation(NULL) needs to be called before
//          returning from IDropTarget::Drop().



extern RPC_IF_HANDLE __MIDL_itf_shldisp_0264_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shldisp_0264_v0_0_s_ifspec;

#ifndef __IAsyncOperation_INTERFACE_DEFINED__
#define __IAsyncOperation_INTERFACE_DEFINED__

/* interface IAsyncOperation */
/* [object][uuid][helpstring] */ 

typedef /* [unique] */ IAsyncOperation __RPC_FAR *LPASYNCOPERATION;


EXTERN_C const IID IID_IAsyncOperation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3D8B0590-F691-11d2-8EA9-006097DF5BD4")
    IAsyncOperation : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetAsyncMode( 
            /* [in] */ BOOL fDoOpAsync) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAsyncMode( 
            /* [out] */ BOOL __RPC_FAR *pfIsOpAsync) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StartOperation( 
            /* [optional][unique][in] */ IBindCtx __RPC_FAR *pbcReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InOperation( 
            /* [out] */ BOOL __RPC_FAR *pfInAsyncOp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndOperation( 
            /* [in] */ HRESULT hResult,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbcReserved,
            /* [in] */ DWORD dwEffects) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAsyncOperationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAsyncOperation __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAsyncOperation __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAsyncOperation __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetAsyncMode )( 
            IAsyncOperation __RPC_FAR * This,
            /* [in] */ BOOL fDoOpAsync);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAsyncMode )( 
            IAsyncOperation __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pfIsOpAsync);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StartOperation )( 
            IAsyncOperation __RPC_FAR * This,
            /* [optional][unique][in] */ IBindCtx __RPC_FAR *pbcReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InOperation )( 
            IAsyncOperation __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pfInAsyncOp);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EndOperation )( 
            IAsyncOperation __RPC_FAR * This,
            /* [in] */ HRESULT hResult,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbcReserved,
            /* [in] */ DWORD dwEffects);
        
        END_INTERFACE
    } IAsyncOperationVtbl;

    interface IAsyncOperation
    {
        CONST_VTBL struct IAsyncOperationVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAsyncOperation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAsyncOperation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAsyncOperation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAsyncOperation_SetAsyncMode(This,fDoOpAsync)	\
    (This)->lpVtbl -> SetAsyncMode(This,fDoOpAsync)

#define IAsyncOperation_GetAsyncMode(This,pfIsOpAsync)	\
    (This)->lpVtbl -> GetAsyncMode(This,pfIsOpAsync)

#define IAsyncOperation_StartOperation(This,pbcReserved)	\
    (This)->lpVtbl -> StartOperation(This,pbcReserved)

#define IAsyncOperation_InOperation(This,pfInAsyncOp)	\
    (This)->lpVtbl -> InOperation(This,pfInAsyncOp)

#define IAsyncOperation_EndOperation(This,hResult,pbcReserved,dwEffects)	\
    (This)->lpVtbl -> EndOperation(This,hResult,pbcReserved,dwEffects)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IAsyncOperation_SetAsyncMode_Proxy( 
    IAsyncOperation __RPC_FAR * This,
    /* [in] */ BOOL fDoOpAsync);


void __RPC_STUB IAsyncOperation_SetAsyncMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAsyncOperation_GetAsyncMode_Proxy( 
    IAsyncOperation __RPC_FAR * This,
    /* [out] */ BOOL __RPC_FAR *pfIsOpAsync);


void __RPC_STUB IAsyncOperation_GetAsyncMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAsyncOperation_StartOperation_Proxy( 
    IAsyncOperation __RPC_FAR * This,
    /* [optional][unique][in] */ IBindCtx __RPC_FAR *pbcReserved);


void __RPC_STUB IAsyncOperation_StartOperation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAsyncOperation_InOperation_Proxy( 
    IAsyncOperation __RPC_FAR * This,
    /* [out] */ BOOL __RPC_FAR *pfInAsyncOp);


void __RPC_STUB IAsyncOperation_InOperation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAsyncOperation_EndOperation_Proxy( 
    IAsyncOperation __RPC_FAR * This,
    /* [in] */ HRESULT hResult,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbcReserved,
    /* [in] */ DWORD dwEffects);


void __RPC_STUB IAsyncOperation_EndOperation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAsyncOperation_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_shldisp_0265 */
/* [local] */ 

#endif // _SHLDISP_PUB_H_


extern RPC_IF_HANDLE __MIDL_itf_shldisp_0265_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shldisp_0265_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



