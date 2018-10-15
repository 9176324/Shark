
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 5.03.0279 */
/* at Wed Apr 23 14:13:20 2003
 */
/* Compiler settings for msxml.idl:
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

#ifndef __msxml_h__
#define __msxml_h__

/* Forward Declarations */ 

#ifndef __IXMLDOMImplementation_FWD_DEFINED__
#define __IXMLDOMImplementation_FWD_DEFINED__
typedef interface IXMLDOMImplementation IXMLDOMImplementation;
#endif 	/* __IXMLDOMImplementation_FWD_DEFINED__ */


#ifndef __IXMLDOMNode_FWD_DEFINED__
#define __IXMLDOMNode_FWD_DEFINED__
typedef interface IXMLDOMNode IXMLDOMNode;
#endif 	/* __IXMLDOMNode_FWD_DEFINED__ */


#ifndef __IXMLDOMDocumentFragment_FWD_DEFINED__
#define __IXMLDOMDocumentFragment_FWD_DEFINED__
typedef interface IXMLDOMDocumentFragment IXMLDOMDocumentFragment;
#endif 	/* __IXMLDOMDocumentFragment_FWD_DEFINED__ */


#ifndef __IXMLDOMDocument_FWD_DEFINED__
#define __IXMLDOMDocument_FWD_DEFINED__
typedef interface IXMLDOMDocument IXMLDOMDocument;
#endif 	/* __IXMLDOMDocument_FWD_DEFINED__ */


#ifndef __IXMLDOMNodeList_FWD_DEFINED__
#define __IXMLDOMNodeList_FWD_DEFINED__
typedef interface IXMLDOMNodeList IXMLDOMNodeList;
#endif 	/* __IXMLDOMNodeList_FWD_DEFINED__ */


#ifndef __IXMLDOMNamedNodeMap_FWD_DEFINED__
#define __IXMLDOMNamedNodeMap_FWD_DEFINED__
typedef interface IXMLDOMNamedNodeMap IXMLDOMNamedNodeMap;
#endif 	/* __IXMLDOMNamedNodeMap_FWD_DEFINED__ */


#ifndef __IXMLDOMCharacterData_FWD_DEFINED__
#define __IXMLDOMCharacterData_FWD_DEFINED__
typedef interface IXMLDOMCharacterData IXMLDOMCharacterData;
#endif 	/* __IXMLDOMCharacterData_FWD_DEFINED__ */


#ifndef __IXMLDOMAttribute_FWD_DEFINED__
#define __IXMLDOMAttribute_FWD_DEFINED__
typedef interface IXMLDOMAttribute IXMLDOMAttribute;
#endif 	/* __IXMLDOMAttribute_FWD_DEFINED__ */


#ifndef __IXMLDOMElement_FWD_DEFINED__
#define __IXMLDOMElement_FWD_DEFINED__
typedef interface IXMLDOMElement IXMLDOMElement;
#endif 	/* __IXMLDOMElement_FWD_DEFINED__ */


#ifndef __IXMLDOMText_FWD_DEFINED__
#define __IXMLDOMText_FWD_DEFINED__
typedef interface IXMLDOMText IXMLDOMText;
#endif 	/* __IXMLDOMText_FWD_DEFINED__ */


#ifndef __IXMLDOMComment_FWD_DEFINED__
#define __IXMLDOMComment_FWD_DEFINED__
typedef interface IXMLDOMComment IXMLDOMComment;
#endif 	/* __IXMLDOMComment_FWD_DEFINED__ */


#ifndef __IXMLDOMProcessingInstruction_FWD_DEFINED__
#define __IXMLDOMProcessingInstruction_FWD_DEFINED__
typedef interface IXMLDOMProcessingInstruction IXMLDOMProcessingInstruction;
#endif 	/* __IXMLDOMProcessingInstruction_FWD_DEFINED__ */


#ifndef __IXMLDOMCDATASection_FWD_DEFINED__
#define __IXMLDOMCDATASection_FWD_DEFINED__
typedef interface IXMLDOMCDATASection IXMLDOMCDATASection;
#endif 	/* __IXMLDOMCDATASection_FWD_DEFINED__ */


#ifndef __IXMLDOMDocumentType_FWD_DEFINED__
#define __IXMLDOMDocumentType_FWD_DEFINED__
typedef interface IXMLDOMDocumentType IXMLDOMDocumentType;
#endif 	/* __IXMLDOMDocumentType_FWD_DEFINED__ */


#ifndef __IXMLDOMNotation_FWD_DEFINED__
#define __IXMLDOMNotation_FWD_DEFINED__
typedef interface IXMLDOMNotation IXMLDOMNotation;
#endif 	/* __IXMLDOMNotation_FWD_DEFINED__ */


#ifndef __IXMLDOMEntity_FWD_DEFINED__
#define __IXMLDOMEntity_FWD_DEFINED__
typedef interface IXMLDOMEntity IXMLDOMEntity;
#endif 	/* __IXMLDOMEntity_FWD_DEFINED__ */


#ifndef __IXMLDOMEntityReference_FWD_DEFINED__
#define __IXMLDOMEntityReference_FWD_DEFINED__
typedef interface IXMLDOMEntityReference IXMLDOMEntityReference;
#endif 	/* __IXMLDOMEntityReference_FWD_DEFINED__ */


#ifndef __IXMLDOMParseError_FWD_DEFINED__
#define __IXMLDOMParseError_FWD_DEFINED__
typedef interface IXMLDOMParseError IXMLDOMParseError;
#endif 	/* __IXMLDOMParseError_FWD_DEFINED__ */


#ifndef __IXTLRuntime_FWD_DEFINED__
#define __IXTLRuntime_FWD_DEFINED__
typedef interface IXTLRuntime IXTLRuntime;
#endif 	/* __IXTLRuntime_FWD_DEFINED__ */


#ifndef __XMLDOMDocumentEvents_FWD_DEFINED__
#define __XMLDOMDocumentEvents_FWD_DEFINED__
typedef interface XMLDOMDocumentEvents XMLDOMDocumentEvents;
#endif 	/* __XMLDOMDocumentEvents_FWD_DEFINED__ */


#ifndef __DOMDocument_FWD_DEFINED__
#define __DOMDocument_FWD_DEFINED__

#ifdef __cplusplus
typedef class DOMDocument DOMDocument;
#else
typedef struct DOMDocument DOMDocument;
#endif /* __cplusplus */

#endif 	/* __DOMDocument_FWD_DEFINED__ */


#ifndef __DOMFreeThreadedDocument_FWD_DEFINED__
#define __DOMFreeThreadedDocument_FWD_DEFINED__

#ifdef __cplusplus
typedef class DOMFreeThreadedDocument DOMFreeThreadedDocument;
#else
typedef struct DOMFreeThreadedDocument DOMFreeThreadedDocument;
#endif /* __cplusplus */

#endif 	/* __DOMFreeThreadedDocument_FWD_DEFINED__ */


#ifndef __IXMLHttpRequest_FWD_DEFINED__
#define __IXMLHttpRequest_FWD_DEFINED__
typedef interface IXMLHttpRequest IXMLHttpRequest;
#endif 	/* __IXMLHttpRequest_FWD_DEFINED__ */


#ifndef __XMLHTTPRequest_FWD_DEFINED__
#define __XMLHTTPRequest_FWD_DEFINED__

#ifdef __cplusplus
typedef class XMLHTTPRequest XMLHTTPRequest;
#else
typedef struct XMLHTTPRequest XMLHTTPRequest;
#endif /* __cplusplus */

#endif 	/* __XMLHTTPRequest_FWD_DEFINED__ */


#ifndef __IXMLDSOControl_FWD_DEFINED__
#define __IXMLDSOControl_FWD_DEFINED__
typedef interface IXMLDSOControl IXMLDSOControl;
#endif 	/* __IXMLDSOControl_FWD_DEFINED__ */


#ifndef __XMLDSOControl_FWD_DEFINED__
#define __XMLDSOControl_FWD_DEFINED__

#ifdef __cplusplus
typedef class XMLDSOControl XMLDSOControl;
#else
typedef struct XMLDSOControl XMLDSOControl;
#endif /* __cplusplus */

#endif 	/* __XMLDSOControl_FWD_DEFINED__ */


#ifndef __IXMLElementCollection_FWD_DEFINED__
#define __IXMLElementCollection_FWD_DEFINED__
typedef interface IXMLElementCollection IXMLElementCollection;
#endif 	/* __IXMLElementCollection_FWD_DEFINED__ */


#ifndef __IXMLDocument_FWD_DEFINED__
#define __IXMLDocument_FWD_DEFINED__
typedef interface IXMLDocument IXMLDocument;
#endif 	/* __IXMLDocument_FWD_DEFINED__ */


#ifndef __IXMLDocument2_FWD_DEFINED__
#define __IXMLDocument2_FWD_DEFINED__
typedef interface IXMLDocument2 IXMLDocument2;
#endif 	/* __IXMLDocument2_FWD_DEFINED__ */


#ifndef __IXMLElement_FWD_DEFINED__
#define __IXMLElement_FWD_DEFINED__
typedef interface IXMLElement IXMLElement;
#endif 	/* __IXMLElement_FWD_DEFINED__ */


#ifndef __IXMLElement2_FWD_DEFINED__
#define __IXMLElement2_FWD_DEFINED__
typedef interface IXMLElement2 IXMLElement2;
#endif 	/* __IXMLElement2_FWD_DEFINED__ */


#ifndef __IXMLAttribute_FWD_DEFINED__
#define __IXMLAttribute_FWD_DEFINED__
typedef interface IXMLAttribute IXMLAttribute;
#endif 	/* __IXMLAttribute_FWD_DEFINED__ */


#ifndef __IXMLError_FWD_DEFINED__
#define __IXMLError_FWD_DEFINED__
typedef interface IXMLError IXMLError;
#endif 	/* __IXMLError_FWD_DEFINED__ */


#ifndef __XMLDocument_FWD_DEFINED__
#define __XMLDocument_FWD_DEFINED__

#ifdef __cplusplus
typedef class XMLDocument XMLDocument;
#else
typedef struct XMLDocument XMLDocument;
#endif /* __cplusplus */

#endif 	/* __XMLDocument_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"
#include "objidl.h"
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_msxml_0000 */
/* [local] */ 

//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1997-1998.
//
//--------------------------------------------------------------------------
typedef struct _xml_error
    {
    unsigned int _nLine;
    BSTR _pchBuf;
    unsigned int _cchBuf;
    unsigned int _ich;
    BSTR _pszFound;
    BSTR _pszExpected;
    DWORD _reserved1;
    DWORD _reserved2;
    }	XML_ERROR;



extern RPC_IF_HANDLE __MIDL_itf_msxml_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msxml_0000_v0_0_s_ifspec;


#ifndef __MSXML_LIBRARY_DEFINED__
#define __MSXML_LIBRARY_DEFINED__

/* library MSXML */
/* [version][lcid][helpstring][uuid] */ 

//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1998.
//
//--------------------------------------------------------------------------



















typedef /* [helpstring] */ 
enum tagDOMNodeType
    {	NODE_INVALID	= 0,
	NODE_ELEMENT	= NODE_INVALID + 1,
	NODE_ATTRIBUTE	= NODE_ELEMENT + 1,
	NODE_TEXT	= NODE_ATTRIBUTE + 1,
	NODE_CDATA_SECTION	= NODE_TEXT + 1,
	NODE_ENTITY_REFERENCE	= NODE_CDATA_SECTION + 1,
	NODE_ENTITY	= NODE_ENTITY_REFERENCE + 1,
	NODE_PROCESSING_INSTRUCTION	= NODE_ENTITY + 1,
	NODE_COMMENT	= NODE_PROCESSING_INSTRUCTION + 1,
	NODE_DOCUMENT	= NODE_COMMENT + 1,
	NODE_DOCUMENT_TYPE	= NODE_DOCUMENT + 1,
	NODE_DOCUMENT_FRAGMENT	= NODE_DOCUMENT_TYPE + 1,
	NODE_NOTATION	= NODE_DOCUMENT_FRAGMENT + 1
    }	DOMNodeType;

//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1998.
//
//--------------------------------------------------------------------------









typedef /* [helpstring] */ 
enum tagXMLEMEM_TYPE
    {	XMLELEMTYPE_ELEMENT	= 0,
	XMLELEMTYPE_TEXT	= XMLELEMTYPE_ELEMENT + 1,
	XMLELEMTYPE_COMMENT	= XMLELEMTYPE_TEXT + 1,
	XMLELEMTYPE_DOCUMENT	= XMLELEMTYPE_COMMENT + 1,
	XMLELEMTYPE_DTD	= XMLELEMTYPE_DOCUMENT + 1,
	XMLELEMTYPE_PI	= XMLELEMTYPE_DTD + 1,
	XMLELEMTYPE_OTHER	= XMLELEMTYPE_PI + 1
    }	XMLELEM_TYPE;


EXTERN_C const IID LIBID_MSXML;

#ifndef __IXMLDOMImplementation_INTERFACE_DEFINED__
#define __IXMLDOMImplementation_INTERFACE_DEFINED__

/* interface IXMLDOMImplementation */
/* [uuid][dual][oleautomation][unique][nonextensible][object][local] */ 


EXTERN_C const IID IID_IXMLDOMImplementation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF8F-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMImplementation : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE hasFeature( 
            /* [in] */ BSTR feature,
            /* [in] */ BSTR version,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasFeature) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMImplementationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMImplementation __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMImplementation __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMImplementation __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMImplementation __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMImplementation __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMImplementation __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMImplementation __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *hasFeature )( 
            IXMLDOMImplementation __RPC_FAR * This,
            /* [in] */ BSTR feature,
            /* [in] */ BSTR version,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasFeature);
        
        END_INTERFACE
    } IXMLDOMImplementationVtbl;

    interface IXMLDOMImplementation
    {
        CONST_VTBL struct IXMLDOMImplementationVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMImplementation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMImplementation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMImplementation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMImplementation_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMImplementation_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMImplementation_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMImplementation_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMImplementation_hasFeature(This,feature,version,hasFeature)	\
    (This)->lpVtbl -> hasFeature(This,feature,version,hasFeature)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id] */ HRESULT STDMETHODCALLTYPE IXMLDOMImplementation_hasFeature_Proxy( 
    IXMLDOMImplementation __RPC_FAR * This,
    /* [in] */ BSTR feature,
    /* [in] */ BSTR version,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasFeature);


void __RPC_STUB IXMLDOMImplementation_hasFeature_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLDOMImplementation_INTERFACE_DEFINED__ */


#ifndef __IXMLDOMNode_INTERFACE_DEFINED__
#define __IXMLDOMNode_INTERFACE_DEFINED__

/* interface IXMLDOMNode */
/* [unique][helpstring][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDOMNode;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF80-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMNode : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_nodeName( 
            /* [retval][out] */ BSTR __RPC_FAR *name) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_nodeValue( 
            /* [retval][out] */ VARIANT __RPC_FAR *value) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_nodeValue( 
            /* [in] */ VARIANT value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_nodeType( 
            /* [retval][out] */ DOMNodeType __RPC_FAR *type) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_parentNode( 
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *parent) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_childNodes( 
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *childList) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_firstChild( 
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *firstChild) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_lastChild( 
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *lastChild) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_previousSibling( 
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *previousSibling) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_nextSibling( 
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextSibling) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_attributes( 
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *attributeMap) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE insertBefore( 
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ VARIANT refChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE replaceChild( 
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ IXMLDOMNode __RPC_FAR *oldChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outOldChild) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE removeChild( 
            /* [in] */ IXMLDOMNode __RPC_FAR *childNode,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *oldChild) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE appendChild( 
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE hasChildNodes( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasChild) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ownerDocument( 
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *DOMDocument) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE cloneNode( 
            /* [in] */ VARIANT_BOOL deep,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *cloneRoot) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_nodeTypeString( 
            /* [out][retval] */ BSTR __RPC_FAR *nodeType) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_text( 
            /* [out][retval] */ BSTR __RPC_FAR *text) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_text( 
            /* [in] */ BSTR text) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_specified( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSpecified) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_definition( 
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *definitionNode) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_nodeTypedValue( 
            /* [out][retval] */ VARIANT __RPC_FAR *typedValue) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_nodeTypedValue( 
            /* [in] */ VARIANT typedValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_dataType( 
            /* [out][retval] */ VARIANT __RPC_FAR *dataTypeName) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_dataType( 
            /* [in] */ BSTR dataTypeName) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_xml( 
            /* [out][retval] */ BSTR __RPC_FAR *xmlString) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE transformNode( 
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE selectNodes( 
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE selectSingleNode( 
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *resultNode) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_parsed( 
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isParsed) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_namespaceURI( 
            /* [out][retval] */ BSTR __RPC_FAR *namespaceURI) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_prefix( 
            /* [out][retval] */ BSTR __RPC_FAR *prefixString) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_baseName( 
            /* [out][retval] */ BSTR __RPC_FAR *nameString) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE transformNodeToObject( 
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [in] */ VARIANT outputObject) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMNodeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMNode __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMNode __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeName )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *name);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeValue )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeValue )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [in] */ VARIANT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeType )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [retval][out] */ DOMNodeType __RPC_FAR *type);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parentNode )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *parent);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_childNodes )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *childList);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_firstChild )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *firstChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_lastChild )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *lastChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_previousSibling )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *previousSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nextSibling )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributes )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *attributeMap);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertBefore )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ VARIANT refChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceChild )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ IXMLDOMNode __RPC_FAR *oldChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outOldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *childNode,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *oldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendChild )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *hasChildNodes )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ownerDocument )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *DOMDocument);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *cloneNode )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL deep,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *cloneRoot);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypeString )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nodeType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *text);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [in] */ BSTR text);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_specified )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSpecified);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_definition )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *definitionNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypedValue )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *typedValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeTypedValue )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [in] */ VARIANT typedValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dataType )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *dataTypeName);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_dataType )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [in] */ BSTR dataTypeName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_xml )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNode )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectNodes )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectSingleNode )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *resultNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parsed )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isParsed);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_namespaceURI )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *namespaceURI);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_prefix )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *prefixString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_baseName )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nameString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNodeToObject )( 
            IXMLDOMNode __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [in] */ VARIANT outputObject);
        
        END_INTERFACE
    } IXMLDOMNodeVtbl;

    interface IXMLDOMNode
    {
        CONST_VTBL struct IXMLDOMNodeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMNode_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMNode_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMNode_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMNode_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMNode_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMNode_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMNode_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMNode_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMNode_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMNode_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMNode_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMNode_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMNode_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMNode_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMNode_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMNode_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMNode_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMNode_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMNode_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMNode_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMNode_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMNode_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMNode_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMNode_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMNode_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMNode_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMNode_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMNode_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMNode_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMNode_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMNode_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMNode_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMNode_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMNode_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMNode_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMNode_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMNode_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMNode_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMNode_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMNode_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMNode_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMNode_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMNode_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_nodeName_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *name);


void __RPC_STUB IXMLDOMNode_get_nodeName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_nodeValue_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *value);


void __RPC_STUB IXMLDOMNode_get_nodeValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_put_nodeValue_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [in] */ VARIANT value);


void __RPC_STUB IXMLDOMNode_put_nodeValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_nodeType_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [retval][out] */ DOMNodeType __RPC_FAR *type);


void __RPC_STUB IXMLDOMNode_get_nodeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_parentNode_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *parent);


void __RPC_STUB IXMLDOMNode_get_parentNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_childNodes_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *childList);


void __RPC_STUB IXMLDOMNode_get_childNodes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_firstChild_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *firstChild);


void __RPC_STUB IXMLDOMNode_get_firstChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_lastChild_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *lastChild);


void __RPC_STUB IXMLDOMNode_get_lastChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_previousSibling_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *previousSibling);


void __RPC_STUB IXMLDOMNode_get_previousSibling_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_nextSibling_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextSibling);


void __RPC_STUB IXMLDOMNode_get_nextSibling_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_attributes_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *attributeMap);


void __RPC_STUB IXMLDOMNode_get_attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_insertBefore_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
    /* [in] */ VARIANT refChild,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);


void __RPC_STUB IXMLDOMNode_insertBefore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_replaceChild_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
    /* [in] */ IXMLDOMNode __RPC_FAR *oldChild,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outOldChild);


void __RPC_STUB IXMLDOMNode_replaceChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_removeChild_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [in] */ IXMLDOMNode __RPC_FAR *childNode,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *oldChild);


void __RPC_STUB IXMLDOMNode_removeChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_appendChild_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);


void __RPC_STUB IXMLDOMNode_appendChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_hasChildNodes_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasChild);


void __RPC_STUB IXMLDOMNode_hasChildNodes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_ownerDocument_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *DOMDocument);


void __RPC_STUB IXMLDOMNode_get_ownerDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_cloneNode_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL deep,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *cloneRoot);


void __RPC_STUB IXMLDOMNode_cloneNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_nodeTypeString_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *nodeType);


void __RPC_STUB IXMLDOMNode_get_nodeTypeString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_text_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *text);


void __RPC_STUB IXMLDOMNode_get_text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_put_text_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [in] */ BSTR text);


void __RPC_STUB IXMLDOMNode_put_text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_specified_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSpecified);


void __RPC_STUB IXMLDOMNode_get_specified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_definition_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *definitionNode);


void __RPC_STUB IXMLDOMNode_get_definition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_nodeTypedValue_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [out][retval] */ VARIANT __RPC_FAR *typedValue);


void __RPC_STUB IXMLDOMNode_get_nodeTypedValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_put_nodeTypedValue_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [in] */ VARIANT typedValue);


void __RPC_STUB IXMLDOMNode_put_nodeTypedValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_dataType_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [out][retval] */ VARIANT __RPC_FAR *dataTypeName);


void __RPC_STUB IXMLDOMNode_get_dataType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_put_dataType_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [in] */ BSTR dataTypeName);


void __RPC_STUB IXMLDOMNode_put_dataType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_xml_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *xmlString);


void __RPC_STUB IXMLDOMNode_get_xml_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_transformNode_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
    /* [out][retval] */ BSTR __RPC_FAR *xmlString);


void __RPC_STUB IXMLDOMNode_transformNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_selectNodes_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [in] */ BSTR queryString,
    /* [out][retval] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);


void __RPC_STUB IXMLDOMNode_selectNodes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_selectSingleNode_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [in] */ BSTR queryString,
    /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *resultNode);


void __RPC_STUB IXMLDOMNode_selectSingleNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_parsed_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isParsed);


void __RPC_STUB IXMLDOMNode_get_parsed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_namespaceURI_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *namespaceURI);


void __RPC_STUB IXMLDOMNode_get_namespaceURI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_prefix_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *prefixString);


void __RPC_STUB IXMLDOMNode_get_prefix_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_get_baseName_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *nameString);


void __RPC_STUB IXMLDOMNode_get_baseName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNode_transformNodeToObject_Proxy( 
    IXMLDOMNode __RPC_FAR * This,
    /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
    /* [in] */ VARIANT outputObject);


void __RPC_STUB IXMLDOMNode_transformNodeToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLDOMNode_INTERFACE_DEFINED__ */


#ifndef __IXMLDOMDocumentFragment_INTERFACE_DEFINED__
#define __IXMLDOMDocumentFragment_INTERFACE_DEFINED__

/* interface IXMLDOMDocumentFragment */
/* [unique][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDOMDocumentFragment;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3efaa413-272f-11d2-836f-0000f87a7782")
    IXMLDOMDocumentFragment : public IXMLDOMNode
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMDocumentFragmentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMDocumentFragment __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMDocumentFragment __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeName )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *name);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeValue )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeValue )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [in] */ VARIANT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeType )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [retval][out] */ DOMNodeType __RPC_FAR *type);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parentNode )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *parent);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_childNodes )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *childList);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_firstChild )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *firstChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_lastChild )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *lastChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_previousSibling )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *previousSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nextSibling )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributes )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *attributeMap);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertBefore )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ VARIANT refChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceChild )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ IXMLDOMNode __RPC_FAR *oldChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outOldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *childNode,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *oldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendChild )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *hasChildNodes )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ownerDocument )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *DOMDocument);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *cloneNode )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL deep,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *cloneRoot);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypeString )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nodeType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *text);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [in] */ BSTR text);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_specified )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSpecified);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_definition )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *definitionNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypedValue )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *typedValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeTypedValue )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [in] */ VARIANT typedValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dataType )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *dataTypeName);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_dataType )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [in] */ BSTR dataTypeName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_xml )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNode )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectNodes )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectSingleNode )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *resultNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parsed )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isParsed);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_namespaceURI )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *namespaceURI);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_prefix )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *prefixString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_baseName )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nameString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNodeToObject )( 
            IXMLDOMDocumentFragment __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [in] */ VARIANT outputObject);
        
        END_INTERFACE
    } IXMLDOMDocumentFragmentVtbl;

    interface IXMLDOMDocumentFragment
    {
        CONST_VTBL struct IXMLDOMDocumentFragmentVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMDocumentFragment_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMDocumentFragment_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMDocumentFragment_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMDocumentFragment_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMDocumentFragment_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMDocumentFragment_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMDocumentFragment_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMDocumentFragment_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMDocumentFragment_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMDocumentFragment_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMDocumentFragment_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMDocumentFragment_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMDocumentFragment_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMDocumentFragment_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMDocumentFragment_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMDocumentFragment_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMDocumentFragment_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMDocumentFragment_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMDocumentFragment_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMDocumentFragment_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMDocumentFragment_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMDocumentFragment_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMDocumentFragment_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMDocumentFragment_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMDocumentFragment_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMDocumentFragment_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMDocumentFragment_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMDocumentFragment_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMDocumentFragment_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMDocumentFragment_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMDocumentFragment_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMDocumentFragment_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMDocumentFragment_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMDocumentFragment_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMDocumentFragment_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMDocumentFragment_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMDocumentFragment_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMDocumentFragment_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMDocumentFragment_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMDocumentFragment_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMDocumentFragment_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMDocumentFragment_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMDocumentFragment_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IXMLDOMDocumentFragment_INTERFACE_DEFINED__ */


#ifndef __IXMLDOMDocument_INTERFACE_DEFINED__
#define __IXMLDOMDocument_INTERFACE_DEFINED__

/* interface IXMLDOMDocument */
/* [unique][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDOMDocument;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF81-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMDocument : public IXMLDOMNode
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_doctype( 
            /* [retval][out] */ IXMLDOMDocumentType __RPC_FAR *__RPC_FAR *documentType) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_implementation( 
            /* [retval][out] */ IXMLDOMImplementation __RPC_FAR *__RPC_FAR *impl) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_documentElement( 
            /* [retval][out] */ IXMLDOMElement __RPC_FAR *__RPC_FAR *DOMElement) = 0;
        
        virtual /* [helpstring][id][propputref] */ HRESULT STDMETHODCALLTYPE putref_documentElement( 
            /* [in] */ IXMLDOMElement __RPC_FAR *DOMElement) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE createElement( 
            /* [in] */ BSTR tagName,
            /* [retval][out] */ IXMLDOMElement __RPC_FAR *__RPC_FAR *element) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE createDocumentFragment( 
            /* [retval][out] */ IXMLDOMDocumentFragment __RPC_FAR *__RPC_FAR *docFrag) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE createTextNode( 
            /* [in] */ BSTR data,
            /* [retval][out] */ IXMLDOMText __RPC_FAR *__RPC_FAR *text) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE createComment( 
            /* [in] */ BSTR data,
            /* [retval][out] */ IXMLDOMComment __RPC_FAR *__RPC_FAR *comment) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE createCDATASection( 
            /* [in] */ BSTR data,
            /* [retval][out] */ IXMLDOMCDATASection __RPC_FAR *__RPC_FAR *cdata) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE createProcessingInstruction( 
            /* [in] */ BSTR target,
            /* [in] */ BSTR data,
            /* [retval][out] */ IXMLDOMProcessingInstruction __RPC_FAR *__RPC_FAR *pi) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE createAttribute( 
            /* [in] */ BSTR name,
            /* [retval][out] */ IXMLDOMAttribute __RPC_FAR *__RPC_FAR *attribute) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE createEntityReference( 
            /* [in] */ BSTR name,
            /* [retval][out] */ IXMLDOMEntityReference __RPC_FAR *__RPC_FAR *entityRef) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getElementsByTagName( 
            /* [in] */ BSTR tagName,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE createNode( 
            /* [in] */ VARIANT Type,
            /* [in] */ BSTR name,
            /* [in] */ BSTR namespaceURI,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *node) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE nodeFromID( 
            /* [in] */ BSTR idString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *node) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE load( 
            /* [in] */ VARIANT xmlSource,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSuccessful) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_readyState( 
            /* [out][retval] */ long __RPC_FAR *value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_parseError( 
            /* [out][retval] */ IXMLDOMParseError __RPC_FAR *__RPC_FAR *errorObj) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_url( 
            /* [out][retval] */ BSTR __RPC_FAR *urlString) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_async( 
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isAsync) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_async( 
            /* [in] */ VARIANT_BOOL isAsync) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE abort( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE loadXML( 
            /* [in] */ BSTR bstrXML,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSuccessful) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE save( 
            /* [in] */ VARIANT desination) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_validateOnParse( 
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isValidating) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_validateOnParse( 
            /* [in] */ VARIANT_BOOL isValidating) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_resolveExternals( 
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isResolving) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_resolveExternals( 
            /* [in] */ VARIANT_BOOL isResolving) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_preserveWhiteSpace( 
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isPreserving) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_preserveWhiteSpace( 
            /* [in] */ VARIANT_BOOL isPreserving) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_onreadystatechange( 
            /* [in] */ VARIANT readystatechangeSink) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ondataavailable( 
            /* [in] */ VARIANT ondataavailableSink) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ontransformnode( 
            /* [in] */ VARIANT ontransformnodeSink) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMDocumentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMDocument __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMDocument __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeName )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *name);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeValue )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeValue )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ VARIANT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeType )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [retval][out] */ DOMNodeType __RPC_FAR *type);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parentNode )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *parent);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_childNodes )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *childList);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_firstChild )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *firstChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_lastChild )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *lastChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_previousSibling )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *previousSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nextSibling )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributes )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *attributeMap);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertBefore )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ VARIANT refChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceChild )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ IXMLDOMNode __RPC_FAR *oldChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outOldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *childNode,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *oldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendChild )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *hasChildNodes )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ownerDocument )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *DOMDocument);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *cloneNode )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL deep,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *cloneRoot);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypeString )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nodeType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *text);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ BSTR text);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_specified )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSpecified);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_definition )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *definitionNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypedValue )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *typedValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeTypedValue )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ VARIANT typedValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dataType )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *dataTypeName);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_dataType )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ BSTR dataTypeName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_xml )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNode )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectNodes )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectSingleNode )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *resultNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parsed )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isParsed);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_namespaceURI )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *namespaceURI);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_prefix )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *prefixString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_baseName )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nameString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNodeToObject )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [in] */ VARIANT outputObject);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_doctype )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocumentType __RPC_FAR *__RPC_FAR *documentType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_implementation )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMImplementation __RPC_FAR *__RPC_FAR *impl);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_documentElement )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMElement __RPC_FAR *__RPC_FAR *DOMElement);
        
        /* [helpstring][id][propputref] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *putref_documentElement )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ IXMLDOMElement __RPC_FAR *DOMElement);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *createElement )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ BSTR tagName,
            /* [retval][out] */ IXMLDOMElement __RPC_FAR *__RPC_FAR *element);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *createDocumentFragment )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocumentFragment __RPC_FAR *__RPC_FAR *docFrag);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *createTextNode )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ BSTR data,
            /* [retval][out] */ IXMLDOMText __RPC_FAR *__RPC_FAR *text);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *createComment )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ BSTR data,
            /* [retval][out] */ IXMLDOMComment __RPC_FAR *__RPC_FAR *comment);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *createCDATASection )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ BSTR data,
            /* [retval][out] */ IXMLDOMCDATASection __RPC_FAR *__RPC_FAR *cdata);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *createProcessingInstruction )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ BSTR target,
            /* [in] */ BSTR data,
            /* [retval][out] */ IXMLDOMProcessingInstruction __RPC_FAR *__RPC_FAR *pi);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *createAttribute )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ BSTR name,
            /* [retval][out] */ IXMLDOMAttribute __RPC_FAR *__RPC_FAR *attribute);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *createEntityReference )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ BSTR name,
            /* [retval][out] */ IXMLDOMEntityReference __RPC_FAR *__RPC_FAR *entityRef);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getElementsByTagName )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ BSTR tagName,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *createNode )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ VARIANT Type,
            /* [in] */ BSTR name,
            /* [in] */ BSTR namespaceURI,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *node);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *nodeFromID )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ BSTR idString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *node);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *load )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ VARIANT xmlSource,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSuccessful);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_readyState )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out][retval] */ long __RPC_FAR *value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parseError )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out][retval] */ IXMLDOMParseError __RPC_FAR *__RPC_FAR *errorObj);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_url )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *urlString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_async )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isAsync);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_async )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL isAsync);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *abort )( 
            IXMLDOMDocument __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *loadXML )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ BSTR bstrXML,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSuccessful);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *save )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ VARIANT desination);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_validateOnParse )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isValidating);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_validateOnParse )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL isValidating);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_resolveExternals )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isResolving);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_resolveExternals )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL isResolving);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_preserveWhiteSpace )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isPreserving);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_preserveWhiteSpace )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL isPreserving);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_onreadystatechange )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ VARIANT readystatechangeSink);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ondataavailable )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ VARIANT ondataavailableSink);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ontransformnode )( 
            IXMLDOMDocument __RPC_FAR * This,
            /* [in] */ VARIANT ontransformnodeSink);
        
        END_INTERFACE
    } IXMLDOMDocumentVtbl;

    interface IXMLDOMDocument
    {
        CONST_VTBL struct IXMLDOMDocumentVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMDocument_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMDocument_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMDocument_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMDocument_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMDocument_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMDocument_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMDocument_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMDocument_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMDocument_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMDocument_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMDocument_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMDocument_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMDocument_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMDocument_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMDocument_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMDocument_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMDocument_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMDocument_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMDocument_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMDocument_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMDocument_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMDocument_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMDocument_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMDocument_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMDocument_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMDocument_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMDocument_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMDocument_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMDocument_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMDocument_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMDocument_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMDocument_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMDocument_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMDocument_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMDocument_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMDocument_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMDocument_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMDocument_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMDocument_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMDocument_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMDocument_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMDocument_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMDocument_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMDocument_get_doctype(This,documentType)	\
    (This)->lpVtbl -> get_doctype(This,documentType)

#define IXMLDOMDocument_get_implementation(This,impl)	\
    (This)->lpVtbl -> get_implementation(This,impl)

#define IXMLDOMDocument_get_documentElement(This,DOMElement)	\
    (This)->lpVtbl -> get_documentElement(This,DOMElement)

#define IXMLDOMDocument_putref_documentElement(This,DOMElement)	\
    (This)->lpVtbl -> putref_documentElement(This,DOMElement)

#define IXMLDOMDocument_createElement(This,tagName,element)	\
    (This)->lpVtbl -> createElement(This,tagName,element)

#define IXMLDOMDocument_createDocumentFragment(This,docFrag)	\
    (This)->lpVtbl -> createDocumentFragment(This,docFrag)

#define IXMLDOMDocument_createTextNode(This,data,text)	\
    (This)->lpVtbl -> createTextNode(This,data,text)

#define IXMLDOMDocument_createComment(This,data,comment)	\
    (This)->lpVtbl -> createComment(This,data,comment)

#define IXMLDOMDocument_createCDATASection(This,data,cdata)	\
    (This)->lpVtbl -> createCDATASection(This,data,cdata)

#define IXMLDOMDocument_createProcessingInstruction(This,target,data,pi)	\
    (This)->lpVtbl -> createProcessingInstruction(This,target,data,pi)

#define IXMLDOMDocument_createAttribute(This,name,attribute)	\
    (This)->lpVtbl -> createAttribute(This,name,attribute)

#define IXMLDOMDocument_createEntityReference(This,name,entityRef)	\
    (This)->lpVtbl -> createEntityReference(This,name,entityRef)

#define IXMLDOMDocument_getElementsByTagName(This,tagName,resultList)	\
    (This)->lpVtbl -> getElementsByTagName(This,tagName,resultList)

#define IXMLDOMDocument_createNode(This,Type,name,namespaceURI,node)	\
    (This)->lpVtbl -> createNode(This,Type,name,namespaceURI,node)

#define IXMLDOMDocument_nodeFromID(This,idString,node)	\
    (This)->lpVtbl -> nodeFromID(This,idString,node)

#define IXMLDOMDocument_load(This,xmlSource,isSuccessful)	\
    (This)->lpVtbl -> load(This,xmlSource,isSuccessful)

#define IXMLDOMDocument_get_readyState(This,value)	\
    (This)->lpVtbl -> get_readyState(This,value)

#define IXMLDOMDocument_get_parseError(This,errorObj)	\
    (This)->lpVtbl -> get_parseError(This,errorObj)

#define IXMLDOMDocument_get_url(This,urlString)	\
    (This)->lpVtbl -> get_url(This,urlString)

#define IXMLDOMDocument_get_async(This,isAsync)	\
    (This)->lpVtbl -> get_async(This,isAsync)

#define IXMLDOMDocument_put_async(This,isAsync)	\
    (This)->lpVtbl -> put_async(This,isAsync)

#define IXMLDOMDocument_abort(This)	\
    (This)->lpVtbl -> abort(This)

#define IXMLDOMDocument_loadXML(This,bstrXML,isSuccessful)	\
    (This)->lpVtbl -> loadXML(This,bstrXML,isSuccessful)

#define IXMLDOMDocument_save(This,desination)	\
    (This)->lpVtbl -> save(This,desination)

#define IXMLDOMDocument_get_validateOnParse(This,isValidating)	\
    (This)->lpVtbl -> get_validateOnParse(This,isValidating)

#define IXMLDOMDocument_put_validateOnParse(This,isValidating)	\
    (This)->lpVtbl -> put_validateOnParse(This,isValidating)

#define IXMLDOMDocument_get_resolveExternals(This,isResolving)	\
    (This)->lpVtbl -> get_resolveExternals(This,isResolving)

#define IXMLDOMDocument_put_resolveExternals(This,isResolving)	\
    (This)->lpVtbl -> put_resolveExternals(This,isResolving)

#define IXMLDOMDocument_get_preserveWhiteSpace(This,isPreserving)	\
    (This)->lpVtbl -> get_preserveWhiteSpace(This,isPreserving)

#define IXMLDOMDocument_put_preserveWhiteSpace(This,isPreserving)	\
    (This)->lpVtbl -> put_preserveWhiteSpace(This,isPreserving)

#define IXMLDOMDocument_put_onreadystatechange(This,readystatechangeSink)	\
    (This)->lpVtbl -> put_onreadystatechange(This,readystatechangeSink)

#define IXMLDOMDocument_put_ondataavailable(This,ondataavailableSink)	\
    (This)->lpVtbl -> put_ondataavailable(This,ondataavailableSink)

#define IXMLDOMDocument_put_ontransformnode(This,ontransformnodeSink)	\
    (This)->lpVtbl -> put_ontransformnode(This,ontransformnodeSink)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_doctype_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [retval][out] */ IXMLDOMDocumentType __RPC_FAR *__RPC_FAR *documentType);


void __RPC_STUB IXMLDOMDocument_get_doctype_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_implementation_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [retval][out] */ IXMLDOMImplementation __RPC_FAR *__RPC_FAR *impl);


void __RPC_STUB IXMLDOMDocument_get_implementation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_documentElement_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [retval][out] */ IXMLDOMElement __RPC_FAR *__RPC_FAR *DOMElement);


void __RPC_STUB IXMLDOMDocument_get_documentElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propputref] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_putref_documentElement_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ IXMLDOMElement __RPC_FAR *DOMElement);


void __RPC_STUB IXMLDOMDocument_putref_documentElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createElement_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ BSTR tagName,
    /* [retval][out] */ IXMLDOMElement __RPC_FAR *__RPC_FAR *element);


void __RPC_STUB IXMLDOMDocument_createElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createDocumentFragment_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [retval][out] */ IXMLDOMDocumentFragment __RPC_FAR *__RPC_FAR *docFrag);


void __RPC_STUB IXMLDOMDocument_createDocumentFragment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createTextNode_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ BSTR data,
    /* [retval][out] */ IXMLDOMText __RPC_FAR *__RPC_FAR *text);


void __RPC_STUB IXMLDOMDocument_createTextNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createComment_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ BSTR data,
    /* [retval][out] */ IXMLDOMComment __RPC_FAR *__RPC_FAR *comment);


void __RPC_STUB IXMLDOMDocument_createComment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createCDATASection_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ BSTR data,
    /* [retval][out] */ IXMLDOMCDATASection __RPC_FAR *__RPC_FAR *cdata);


void __RPC_STUB IXMLDOMDocument_createCDATASection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createProcessingInstruction_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ BSTR target,
    /* [in] */ BSTR data,
    /* [retval][out] */ IXMLDOMProcessingInstruction __RPC_FAR *__RPC_FAR *pi);


void __RPC_STUB IXMLDOMDocument_createProcessingInstruction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createAttribute_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ BSTR name,
    /* [retval][out] */ IXMLDOMAttribute __RPC_FAR *__RPC_FAR *attribute);


void __RPC_STUB IXMLDOMDocument_createAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createEntityReference_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ BSTR name,
    /* [retval][out] */ IXMLDOMEntityReference __RPC_FAR *__RPC_FAR *entityRef);


void __RPC_STUB IXMLDOMDocument_createEntityReference_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_getElementsByTagName_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ BSTR tagName,
    /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);


void __RPC_STUB IXMLDOMDocument_getElementsByTagName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_createNode_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ VARIANT Type,
    /* [in] */ BSTR name,
    /* [in] */ BSTR namespaceURI,
    /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *node);


void __RPC_STUB IXMLDOMDocument_createNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_nodeFromID_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ BSTR idString,
    /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *node);


void __RPC_STUB IXMLDOMDocument_nodeFromID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_load_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ VARIANT xmlSource,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSuccessful);


void __RPC_STUB IXMLDOMDocument_load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_readyState_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [out][retval] */ long __RPC_FAR *value);


void __RPC_STUB IXMLDOMDocument_get_readyState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_parseError_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [out][retval] */ IXMLDOMParseError __RPC_FAR *__RPC_FAR *errorObj);


void __RPC_STUB IXMLDOMDocument_get_parseError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_url_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *urlString);


void __RPC_STUB IXMLDOMDocument_get_url_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_async_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isAsync);


void __RPC_STUB IXMLDOMDocument_get_async_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_put_async_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL isAsync);


void __RPC_STUB IXMLDOMDocument_put_async_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_abort_Proxy( 
    IXMLDOMDocument __RPC_FAR * This);


void __RPC_STUB IXMLDOMDocument_abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_loadXML_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ BSTR bstrXML,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSuccessful);


void __RPC_STUB IXMLDOMDocument_loadXML_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_save_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ VARIANT desination);


void __RPC_STUB IXMLDOMDocument_save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_validateOnParse_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isValidating);


void __RPC_STUB IXMLDOMDocument_get_validateOnParse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_put_validateOnParse_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL isValidating);


void __RPC_STUB IXMLDOMDocument_put_validateOnParse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_resolveExternals_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isResolving);


void __RPC_STUB IXMLDOMDocument_get_resolveExternals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_put_resolveExternals_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL isResolving);


void __RPC_STUB IXMLDOMDocument_put_resolveExternals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_get_preserveWhiteSpace_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isPreserving);


void __RPC_STUB IXMLDOMDocument_get_preserveWhiteSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_put_preserveWhiteSpace_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL isPreserving);


void __RPC_STUB IXMLDOMDocument_put_preserveWhiteSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_put_onreadystatechange_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ VARIANT readystatechangeSink);


void __RPC_STUB IXMLDOMDocument_put_onreadystatechange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_put_ondataavailable_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ VARIANT ondataavailableSink);


void __RPC_STUB IXMLDOMDocument_put_ondataavailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocument_put_ontransformnode_Proxy( 
    IXMLDOMDocument __RPC_FAR * This,
    /* [in] */ VARIANT ontransformnodeSink);


void __RPC_STUB IXMLDOMDocument_put_ontransformnode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLDOMDocument_INTERFACE_DEFINED__ */


#ifndef __IXMLDOMNodeList_INTERFACE_DEFINED__
#define __IXMLDOMNodeList_INTERFACE_DEFINED__

/* interface IXMLDOMNodeList */
/* [unique][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDOMNodeList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF82-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMNodeList : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_item( 
            /* [in] */ long index,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *listItem) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_length( 
            /* [retval][out] */ long __RPC_FAR *listLength) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE nextNode( 
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextItem) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE reset( void) = 0;
        
        virtual /* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE get__newEnum( 
            /* [out][retval] */ IUnknown __RPC_FAR *__RPC_FAR *ppUnk) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMNodeListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMNodeList __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMNodeList __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMNodeList __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMNodeList __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMNodeList __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMNodeList __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMNodeList __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_item )( 
            IXMLDOMNodeList __RPC_FAR * This,
            /* [in] */ long index,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *listItem);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_length )( 
            IXMLDOMNodeList __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *listLength);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *nextNode )( 
            IXMLDOMNodeList __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextItem);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *reset )( 
            IXMLDOMNodeList __RPC_FAR * This);
        
        /* [id][hidden][restricted][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__newEnum )( 
            IXMLDOMNodeList __RPC_FAR * This,
            /* [out][retval] */ IUnknown __RPC_FAR *__RPC_FAR *ppUnk);
        
        END_INTERFACE
    } IXMLDOMNodeListVtbl;

    interface IXMLDOMNodeList
    {
        CONST_VTBL struct IXMLDOMNodeListVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMNodeList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMNodeList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMNodeList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMNodeList_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMNodeList_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMNodeList_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMNodeList_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMNodeList_get_item(This,index,listItem)	\
    (This)->lpVtbl -> get_item(This,index,listItem)

#define IXMLDOMNodeList_get_length(This,listLength)	\
    (This)->lpVtbl -> get_length(This,listLength)

#define IXMLDOMNodeList_nextNode(This,nextItem)	\
    (This)->lpVtbl -> nextNode(This,nextItem)

#define IXMLDOMNodeList_reset(This)	\
    (This)->lpVtbl -> reset(This)

#define IXMLDOMNodeList_get__newEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__newEnum(This,ppUnk)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNodeList_get_item_Proxy( 
    IXMLDOMNodeList __RPC_FAR * This,
    /* [in] */ long index,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *listItem);


void __RPC_STUB IXMLDOMNodeList_get_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNodeList_get_length_Proxy( 
    IXMLDOMNodeList __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *listLength);


void __RPC_STUB IXMLDOMNodeList_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNodeList_nextNode_Proxy( 
    IXMLDOMNodeList __RPC_FAR * This,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextItem);


void __RPC_STUB IXMLDOMNodeList_nextNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNodeList_reset_Proxy( 
    IXMLDOMNodeList __RPC_FAR * This);


void __RPC_STUB IXMLDOMNodeList_reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNodeList_get__newEnum_Proxy( 
    IXMLDOMNodeList __RPC_FAR * This,
    /* [out][retval] */ IUnknown __RPC_FAR *__RPC_FAR *ppUnk);


void __RPC_STUB IXMLDOMNodeList_get__newEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLDOMNodeList_INTERFACE_DEFINED__ */


#ifndef __IXMLDOMNamedNodeMap_INTERFACE_DEFINED__
#define __IXMLDOMNamedNodeMap_INTERFACE_DEFINED__

/* interface IXMLDOMNamedNodeMap */
/* [unique][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDOMNamedNodeMap;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF83-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMNamedNodeMap : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getNamedItem( 
            /* [in] */ BSTR name,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *namedItem) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setNamedItem( 
            /* [in] */ IXMLDOMNode __RPC_FAR *newItem,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nameItem) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE removeNamedItem( 
            /* [in] */ BSTR name,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *namedItem) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_item( 
            /* [in] */ long index,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *listItem) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_length( 
            /* [retval][out] */ long __RPC_FAR *listLength) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getQualifiedItem( 
            /* [in] */ BSTR baseName,
            /* [in] */ BSTR namespaceURI,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *qualifiedItem) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE removeQualifiedItem( 
            /* [in] */ BSTR baseName,
            /* [in] */ BSTR namespaceURI,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *qualifiedItem) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE nextNode( 
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextItem) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE reset( void) = 0;
        
        virtual /* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE get__newEnum( 
            /* [out][retval] */ IUnknown __RPC_FAR *__RPC_FAR *ppUnk) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMNamedNodeMapVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMNamedNodeMap __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMNamedNodeMap __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMNamedNodeMap __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMNamedNodeMap __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMNamedNodeMap __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMNamedNodeMap __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMNamedNodeMap __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getNamedItem )( 
            IXMLDOMNamedNodeMap __RPC_FAR * This,
            /* [in] */ BSTR name,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *namedItem);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setNamedItem )( 
            IXMLDOMNamedNodeMap __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newItem,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nameItem);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeNamedItem )( 
            IXMLDOMNamedNodeMap __RPC_FAR * This,
            /* [in] */ BSTR name,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *namedItem);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_item )( 
            IXMLDOMNamedNodeMap __RPC_FAR * This,
            /* [in] */ long index,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *listItem);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_length )( 
            IXMLDOMNamedNodeMap __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *listLength);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getQualifiedItem )( 
            IXMLDOMNamedNodeMap __RPC_FAR * This,
            /* [in] */ BSTR baseName,
            /* [in] */ BSTR namespaceURI,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *qualifiedItem);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeQualifiedItem )( 
            IXMLDOMNamedNodeMap __RPC_FAR * This,
            /* [in] */ BSTR baseName,
            /* [in] */ BSTR namespaceURI,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *qualifiedItem);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *nextNode )( 
            IXMLDOMNamedNodeMap __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextItem);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *reset )( 
            IXMLDOMNamedNodeMap __RPC_FAR * This);
        
        /* [id][hidden][restricted][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__newEnum )( 
            IXMLDOMNamedNodeMap __RPC_FAR * This,
            /* [out][retval] */ IUnknown __RPC_FAR *__RPC_FAR *ppUnk);
        
        END_INTERFACE
    } IXMLDOMNamedNodeMapVtbl;

    interface IXMLDOMNamedNodeMap
    {
        CONST_VTBL struct IXMLDOMNamedNodeMapVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMNamedNodeMap_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMNamedNodeMap_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMNamedNodeMap_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMNamedNodeMap_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMNamedNodeMap_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMNamedNodeMap_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMNamedNodeMap_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMNamedNodeMap_getNamedItem(This,name,namedItem)	\
    (This)->lpVtbl -> getNamedItem(This,name,namedItem)

#define IXMLDOMNamedNodeMap_setNamedItem(This,newItem,nameItem)	\
    (This)->lpVtbl -> setNamedItem(This,newItem,nameItem)

#define IXMLDOMNamedNodeMap_removeNamedItem(This,name,namedItem)	\
    (This)->lpVtbl -> removeNamedItem(This,name,namedItem)

#define IXMLDOMNamedNodeMap_get_item(This,index,listItem)	\
    (This)->lpVtbl -> get_item(This,index,listItem)

#define IXMLDOMNamedNodeMap_get_length(This,listLength)	\
    (This)->lpVtbl -> get_length(This,listLength)

#define IXMLDOMNamedNodeMap_getQualifiedItem(This,baseName,namespaceURI,qualifiedItem)	\
    (This)->lpVtbl -> getQualifiedItem(This,baseName,namespaceURI,qualifiedItem)

#define IXMLDOMNamedNodeMap_removeQualifiedItem(This,baseName,namespaceURI,qualifiedItem)	\
    (This)->lpVtbl -> removeQualifiedItem(This,baseName,namespaceURI,qualifiedItem)

#define IXMLDOMNamedNodeMap_nextNode(This,nextItem)	\
    (This)->lpVtbl -> nextNode(This,nextItem)

#define IXMLDOMNamedNodeMap_reset(This)	\
    (This)->lpVtbl -> reset(This)

#define IXMLDOMNamedNodeMap_get__newEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__newEnum(This,ppUnk)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_getNamedItem_Proxy( 
    IXMLDOMNamedNodeMap __RPC_FAR * This,
    /* [in] */ BSTR name,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *namedItem);


void __RPC_STUB IXMLDOMNamedNodeMap_getNamedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_setNamedItem_Proxy( 
    IXMLDOMNamedNodeMap __RPC_FAR * This,
    /* [in] */ IXMLDOMNode __RPC_FAR *newItem,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nameItem);


void __RPC_STUB IXMLDOMNamedNodeMap_setNamedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_removeNamedItem_Proxy( 
    IXMLDOMNamedNodeMap __RPC_FAR * This,
    /* [in] */ BSTR name,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *namedItem);


void __RPC_STUB IXMLDOMNamedNodeMap_removeNamedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_get_item_Proxy( 
    IXMLDOMNamedNodeMap __RPC_FAR * This,
    /* [in] */ long index,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *listItem);


void __RPC_STUB IXMLDOMNamedNodeMap_get_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_get_length_Proxy( 
    IXMLDOMNamedNodeMap __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *listLength);


void __RPC_STUB IXMLDOMNamedNodeMap_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_getQualifiedItem_Proxy( 
    IXMLDOMNamedNodeMap __RPC_FAR * This,
    /* [in] */ BSTR baseName,
    /* [in] */ BSTR namespaceURI,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *qualifiedItem);


void __RPC_STUB IXMLDOMNamedNodeMap_getQualifiedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_removeQualifiedItem_Proxy( 
    IXMLDOMNamedNodeMap __RPC_FAR * This,
    /* [in] */ BSTR baseName,
    /* [in] */ BSTR namespaceURI,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *qualifiedItem);


void __RPC_STUB IXMLDOMNamedNodeMap_removeQualifiedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_nextNode_Proxy( 
    IXMLDOMNamedNodeMap __RPC_FAR * This,
    /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextItem);


void __RPC_STUB IXMLDOMNamedNodeMap_nextNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_reset_Proxy( 
    IXMLDOMNamedNodeMap __RPC_FAR * This);


void __RPC_STUB IXMLDOMNamedNodeMap_reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNamedNodeMap_get__newEnum_Proxy( 
    IXMLDOMNamedNodeMap __RPC_FAR * This,
    /* [out][retval] */ IUnknown __RPC_FAR *__RPC_FAR *ppUnk);


void __RPC_STUB IXMLDOMNamedNodeMap_get__newEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLDOMNamedNodeMap_INTERFACE_DEFINED__ */


#ifndef __IXMLDOMCharacterData_INTERFACE_DEFINED__
#define __IXMLDOMCharacterData_INTERFACE_DEFINED__

/* interface IXMLDOMCharacterData */
/* [unique][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDOMCharacterData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF84-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMCharacterData : public IXMLDOMNode
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_data( 
            /* [retval][out] */ BSTR __RPC_FAR *data) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_data( 
            /* [in] */ BSTR data) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_length( 
            /* [retval][out] */ long __RPC_FAR *dataLength) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE substringData( 
            /* [in] */ long offset,
            /* [in] */ long count,
            /* [retval][out] */ BSTR __RPC_FAR *data) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE appendData( 
            /* [in] */ BSTR data) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE insertData( 
            /* [in] */ long offset,
            /* [in] */ BSTR data) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE deleteData( 
            /* [in] */ long offset,
            /* [in] */ long count) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE replaceData( 
            /* [in] */ long offset,
            /* [in] */ long count,
            /* [in] */ BSTR data) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMCharacterDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMCharacterData __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMCharacterData __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeName )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *name);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeValue )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeValue )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ VARIANT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeType )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [retval][out] */ DOMNodeType __RPC_FAR *type);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parentNode )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *parent);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_childNodes )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *childList);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_firstChild )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *firstChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_lastChild )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *lastChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_previousSibling )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *previousSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nextSibling )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributes )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *attributeMap);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertBefore )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ VARIANT refChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceChild )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ IXMLDOMNode __RPC_FAR *oldChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outOldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *childNode,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *oldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendChild )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *hasChildNodes )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ownerDocument )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *DOMDocument);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *cloneNode )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL deep,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *cloneRoot);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypeString )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nodeType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *text);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ BSTR text);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_specified )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSpecified);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_definition )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *definitionNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypedValue )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *typedValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeTypedValue )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ VARIANT typedValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dataType )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *dataTypeName);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_dataType )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ BSTR dataTypeName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_xml )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNode )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectNodes )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectSingleNode )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *resultNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parsed )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isParsed);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_namespaceURI )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *namespaceURI);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_prefix )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *prefixString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_baseName )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nameString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNodeToObject )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [in] */ VARIANT outputObject);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_data )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *data);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_data )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ BSTR data);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_length )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *dataLength);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *substringData )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ long offset,
            /* [in] */ long count,
            /* [retval][out] */ BSTR __RPC_FAR *data);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendData )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ BSTR data);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertData )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ long offset,
            /* [in] */ BSTR data);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *deleteData )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ long offset,
            /* [in] */ long count);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceData )( 
            IXMLDOMCharacterData __RPC_FAR * This,
            /* [in] */ long offset,
            /* [in] */ long count,
            /* [in] */ BSTR data);
        
        END_INTERFACE
    } IXMLDOMCharacterDataVtbl;

    interface IXMLDOMCharacterData
    {
        CONST_VTBL struct IXMLDOMCharacterDataVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMCharacterData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMCharacterData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMCharacterData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMCharacterData_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMCharacterData_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMCharacterData_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMCharacterData_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMCharacterData_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMCharacterData_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMCharacterData_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMCharacterData_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMCharacterData_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMCharacterData_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMCharacterData_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMCharacterData_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMCharacterData_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMCharacterData_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMCharacterData_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMCharacterData_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMCharacterData_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMCharacterData_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMCharacterData_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMCharacterData_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMCharacterData_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMCharacterData_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMCharacterData_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMCharacterData_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMCharacterData_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMCharacterData_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMCharacterData_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMCharacterData_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMCharacterData_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMCharacterData_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMCharacterData_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMCharacterData_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMCharacterData_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMCharacterData_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMCharacterData_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMCharacterData_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMCharacterData_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMCharacterData_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMCharacterData_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMCharacterData_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMCharacterData_get_data(This,data)	\
    (This)->lpVtbl -> get_data(This,data)

#define IXMLDOMCharacterData_put_data(This,data)	\
    (This)->lpVtbl -> put_data(This,data)

#define IXMLDOMCharacterData_get_length(This,dataLength)	\
    (This)->lpVtbl -> get_length(This,dataLength)

#define IXMLDOMCharacterData_substringData(This,offset,count,data)	\
    (This)->lpVtbl -> substringData(This,offset,count,data)

#define IXMLDOMCharacterData_appendData(This,data)	\
    (This)->lpVtbl -> appendData(This,data)

#define IXMLDOMCharacterData_insertData(This,offset,data)	\
    (This)->lpVtbl -> insertData(This,offset,data)

#define IXMLDOMCharacterData_deleteData(This,offset,count)	\
    (This)->lpVtbl -> deleteData(This,offset,count)

#define IXMLDOMCharacterData_replaceData(This,offset,count,data)	\
    (This)->lpVtbl -> replaceData(This,offset,count,data)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMCharacterData_get_data_Proxy( 
    IXMLDOMCharacterData __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *data);


void __RPC_STUB IXMLDOMCharacterData_get_data_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDOMCharacterData_put_data_Proxy( 
    IXMLDOMCharacterData __RPC_FAR * This,
    /* [in] */ BSTR data);


void __RPC_STUB IXMLDOMCharacterData_put_data_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMCharacterData_get_length_Proxy( 
    IXMLDOMCharacterData __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *dataLength);


void __RPC_STUB IXMLDOMCharacterData_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMCharacterData_substringData_Proxy( 
    IXMLDOMCharacterData __RPC_FAR * This,
    /* [in] */ long offset,
    /* [in] */ long count,
    /* [retval][out] */ BSTR __RPC_FAR *data);


void __RPC_STUB IXMLDOMCharacterData_substringData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMCharacterData_appendData_Proxy( 
    IXMLDOMCharacterData __RPC_FAR * This,
    /* [in] */ BSTR data);


void __RPC_STUB IXMLDOMCharacterData_appendData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMCharacterData_insertData_Proxy( 
    IXMLDOMCharacterData __RPC_FAR * This,
    /* [in] */ long offset,
    /* [in] */ BSTR data);


void __RPC_STUB IXMLDOMCharacterData_insertData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMCharacterData_deleteData_Proxy( 
    IXMLDOMCharacterData __RPC_FAR * This,
    /* [in] */ long offset,
    /* [in] */ long count);


void __RPC_STUB IXMLDOMCharacterData_deleteData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMCharacterData_replaceData_Proxy( 
    IXMLDOMCharacterData __RPC_FAR * This,
    /* [in] */ long offset,
    /* [in] */ long count,
    /* [in] */ BSTR data);


void __RPC_STUB IXMLDOMCharacterData_replaceData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLDOMCharacterData_INTERFACE_DEFINED__ */


#ifndef __IXMLDOMAttribute_INTERFACE_DEFINED__
#define __IXMLDOMAttribute_INTERFACE_DEFINED__

/* interface IXMLDOMAttribute */
/* [unique][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDOMAttribute;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF85-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMAttribute : public IXMLDOMNode
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_name( 
            /* [retval][out] */ BSTR __RPC_FAR *attributeName) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_value( 
            /* [retval][out] */ VARIANT __RPC_FAR *attributeValue) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_value( 
            /* [in] */ VARIANT attributeValue) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMAttributeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMAttribute __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMAttribute __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeName )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *name);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeValue )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeValue )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ VARIANT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeType )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [retval][out] */ DOMNodeType __RPC_FAR *type);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parentNode )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *parent);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_childNodes )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *childList);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_firstChild )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *firstChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_lastChild )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *lastChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_previousSibling )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *previousSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nextSibling )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributes )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *attributeMap);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertBefore )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ VARIANT refChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceChild )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ IXMLDOMNode __RPC_FAR *oldChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outOldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *childNode,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *oldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendChild )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *hasChildNodes )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ownerDocument )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *DOMDocument);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *cloneNode )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL deep,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *cloneRoot);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypeString )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nodeType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *text);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ BSTR text);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_specified )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSpecified);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_definition )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *definitionNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypedValue )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *typedValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeTypedValue )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ VARIANT typedValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dataType )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *dataTypeName);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_dataType )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ BSTR dataTypeName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_xml )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNode )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectNodes )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectSingleNode )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *resultNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parsed )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isParsed);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_namespaceURI )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *namespaceURI);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_prefix )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *prefixString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_baseName )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nameString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNodeToObject )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [in] */ VARIANT outputObject);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_name )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *attributeName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_value )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *attributeValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_value )( 
            IXMLDOMAttribute __RPC_FAR * This,
            /* [in] */ VARIANT attributeValue);
        
        END_INTERFACE
    } IXMLDOMAttributeVtbl;

    interface IXMLDOMAttribute
    {
        CONST_VTBL struct IXMLDOMAttributeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMAttribute_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMAttribute_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMAttribute_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMAttribute_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMAttribute_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMAttribute_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMAttribute_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMAttribute_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMAttribute_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMAttribute_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMAttribute_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMAttribute_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMAttribute_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMAttribute_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMAttribute_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMAttribute_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMAttribute_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMAttribute_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMAttribute_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMAttribute_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMAttribute_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMAttribute_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMAttribute_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMAttribute_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMAttribute_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMAttribute_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMAttribute_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMAttribute_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMAttribute_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMAttribute_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMAttribute_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMAttribute_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMAttribute_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMAttribute_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMAttribute_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMAttribute_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMAttribute_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMAttribute_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMAttribute_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMAttribute_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMAttribute_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMAttribute_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMAttribute_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMAttribute_get_name(This,attributeName)	\
    (This)->lpVtbl -> get_name(This,attributeName)

#define IXMLDOMAttribute_get_value(This,attributeValue)	\
    (This)->lpVtbl -> get_value(This,attributeValue)

#define IXMLDOMAttribute_put_value(This,attributeValue)	\
    (This)->lpVtbl -> put_value(This,attributeValue)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMAttribute_get_name_Proxy( 
    IXMLDOMAttribute __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *attributeName);


void __RPC_STUB IXMLDOMAttribute_get_name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMAttribute_get_value_Proxy( 
    IXMLDOMAttribute __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *attributeValue);


void __RPC_STUB IXMLDOMAttribute_get_value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDOMAttribute_put_value_Proxy( 
    IXMLDOMAttribute __RPC_FAR * This,
    /* [in] */ VARIANT attributeValue);


void __RPC_STUB IXMLDOMAttribute_put_value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLDOMAttribute_INTERFACE_DEFINED__ */


#ifndef __IXMLDOMElement_INTERFACE_DEFINED__
#define __IXMLDOMElement_INTERFACE_DEFINED__

/* interface IXMLDOMElement */
/* [unique][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDOMElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF86-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMElement : public IXMLDOMNode
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_tagName( 
            /* [retval][out] */ BSTR __RPC_FAR *tagName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getAttribute( 
            /* [in] */ BSTR name,
            /* [retval][out] */ VARIANT __RPC_FAR *value) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setAttribute( 
            /* [in] */ BSTR name,
            /* [in] */ VARIANT value) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE removeAttribute( 
            /* [in] */ BSTR name) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getAttributeNode( 
            /* [in] */ BSTR name,
            /* [retval][out] */ IXMLDOMAttribute __RPC_FAR *__RPC_FAR *attributeNode) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setAttributeNode( 
            /* [in] */ IXMLDOMAttribute __RPC_FAR *DOMAttribute,
            /* [retval][out] */ IXMLDOMAttribute __RPC_FAR *__RPC_FAR *attributeNode) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE removeAttributeNode( 
            /* [in] */ IXMLDOMAttribute __RPC_FAR *DOMAttribute,
            /* [retval][out] */ IXMLDOMAttribute __RPC_FAR *__RPC_FAR *attributeNode) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getElementsByTagName( 
            /* [in] */ BSTR tagName,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE normalize( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMElement __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMElement __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeName )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *name);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeValue )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeValue )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ VARIANT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeType )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [retval][out] */ DOMNodeType __RPC_FAR *type);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parentNode )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *parent);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_childNodes )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *childList);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_firstChild )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *firstChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_lastChild )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *lastChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_previousSibling )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *previousSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nextSibling )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributes )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *attributeMap);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertBefore )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ VARIANT refChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceChild )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ IXMLDOMNode __RPC_FAR *oldChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outOldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *childNode,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *oldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendChild )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *hasChildNodes )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ownerDocument )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *DOMDocument);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *cloneNode )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL deep,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *cloneRoot);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypeString )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nodeType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *text);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ BSTR text);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_specified )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSpecified);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_definition )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *definitionNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypedValue )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *typedValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeTypedValue )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ VARIANT typedValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dataType )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *dataTypeName);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_dataType )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ BSTR dataTypeName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_xml )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNode )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectNodes )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectSingleNode )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *resultNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parsed )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isParsed);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_namespaceURI )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *namespaceURI);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_prefix )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *prefixString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_baseName )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nameString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNodeToObject )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [in] */ VARIANT outputObject);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_tagName )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *tagName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getAttribute )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ BSTR name,
            /* [retval][out] */ VARIANT __RPC_FAR *value);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setAttribute )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ BSTR name,
            /* [in] */ VARIANT value);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeAttribute )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ BSTR name);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getAttributeNode )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ BSTR name,
            /* [retval][out] */ IXMLDOMAttribute __RPC_FAR *__RPC_FAR *attributeNode);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setAttributeNode )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ IXMLDOMAttribute __RPC_FAR *DOMAttribute,
            /* [retval][out] */ IXMLDOMAttribute __RPC_FAR *__RPC_FAR *attributeNode);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeAttributeNode )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ IXMLDOMAttribute __RPC_FAR *DOMAttribute,
            /* [retval][out] */ IXMLDOMAttribute __RPC_FAR *__RPC_FAR *attributeNode);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getElementsByTagName )( 
            IXMLDOMElement __RPC_FAR * This,
            /* [in] */ BSTR tagName,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *normalize )( 
            IXMLDOMElement __RPC_FAR * This);
        
        END_INTERFACE
    } IXMLDOMElementVtbl;

    interface IXMLDOMElement
    {
        CONST_VTBL struct IXMLDOMElementVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMElement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMElement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMElement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMElement_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMElement_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMElement_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMElement_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMElement_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMElement_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMElement_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMElement_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMElement_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMElement_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMElement_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMElement_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMElement_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMElement_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMElement_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMElement_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMElement_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMElement_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMElement_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMElement_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMElement_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMElement_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMElement_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMElement_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMElement_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMElement_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMElement_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMElement_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMElement_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMElement_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMElement_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMElement_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMElement_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMElement_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMElement_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMElement_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMElement_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMElement_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMElement_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMElement_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMElement_get_tagName(This,tagName)	\
    (This)->lpVtbl -> get_tagName(This,tagName)

#define IXMLDOMElement_getAttribute(This,name,value)	\
    (This)->lpVtbl -> getAttribute(This,name,value)

#define IXMLDOMElement_setAttribute(This,name,value)	\
    (This)->lpVtbl -> setAttribute(This,name,value)

#define IXMLDOMElement_removeAttribute(This,name)	\
    (This)->lpVtbl -> removeAttribute(This,name)

#define IXMLDOMElement_getAttributeNode(This,name,attributeNode)	\
    (This)->lpVtbl -> getAttributeNode(This,name,attributeNode)

#define IXMLDOMElement_setAttributeNode(This,DOMAttribute,attributeNode)	\
    (This)->lpVtbl -> setAttributeNode(This,DOMAttribute,attributeNode)

#define IXMLDOMElement_removeAttributeNode(This,DOMAttribute,attributeNode)	\
    (This)->lpVtbl -> removeAttributeNode(This,DOMAttribute,attributeNode)

#define IXMLDOMElement_getElementsByTagName(This,tagName,resultList)	\
    (This)->lpVtbl -> getElementsByTagName(This,tagName,resultList)

#define IXMLDOMElement_normalize(This)	\
    (This)->lpVtbl -> normalize(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMElement_get_tagName_Proxy( 
    IXMLDOMElement __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *tagName);


void __RPC_STUB IXMLDOMElement_get_tagName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMElement_getAttribute_Proxy( 
    IXMLDOMElement __RPC_FAR * This,
    /* [in] */ BSTR name,
    /* [retval][out] */ VARIANT __RPC_FAR *value);


void __RPC_STUB IXMLDOMElement_getAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMElement_setAttribute_Proxy( 
    IXMLDOMElement __RPC_FAR * This,
    /* [in] */ BSTR name,
    /* [in] */ VARIANT value);


void __RPC_STUB IXMLDOMElement_setAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMElement_removeAttribute_Proxy( 
    IXMLDOMElement __RPC_FAR * This,
    /* [in] */ BSTR name);


void __RPC_STUB IXMLDOMElement_removeAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMElement_getAttributeNode_Proxy( 
    IXMLDOMElement __RPC_FAR * This,
    /* [in] */ BSTR name,
    /* [retval][out] */ IXMLDOMAttribute __RPC_FAR *__RPC_FAR *attributeNode);


void __RPC_STUB IXMLDOMElement_getAttributeNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMElement_setAttributeNode_Proxy( 
    IXMLDOMElement __RPC_FAR * This,
    /* [in] */ IXMLDOMAttribute __RPC_FAR *DOMAttribute,
    /* [retval][out] */ IXMLDOMAttribute __RPC_FAR *__RPC_FAR *attributeNode);


void __RPC_STUB IXMLDOMElement_setAttributeNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMElement_removeAttributeNode_Proxy( 
    IXMLDOMElement __RPC_FAR * This,
    /* [in] */ IXMLDOMAttribute __RPC_FAR *DOMAttribute,
    /* [retval][out] */ IXMLDOMAttribute __RPC_FAR *__RPC_FAR *attributeNode);


void __RPC_STUB IXMLDOMElement_removeAttributeNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMElement_getElementsByTagName_Proxy( 
    IXMLDOMElement __RPC_FAR * This,
    /* [in] */ BSTR tagName,
    /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);


void __RPC_STUB IXMLDOMElement_getElementsByTagName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMElement_normalize_Proxy( 
    IXMLDOMElement __RPC_FAR * This);


void __RPC_STUB IXMLDOMElement_normalize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLDOMElement_INTERFACE_DEFINED__ */


#ifndef __IXMLDOMText_INTERFACE_DEFINED__
#define __IXMLDOMText_INTERFACE_DEFINED__

/* interface IXMLDOMText */
/* [unique][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDOMText;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF87-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMText : public IXMLDOMCharacterData
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE splitText( 
            /* [in] */ long offset,
            /* [retval][out] */ IXMLDOMText __RPC_FAR *__RPC_FAR *rightHandTextNode) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMTextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMText __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMText __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMText __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeName )( 
            IXMLDOMText __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *name);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeValue )( 
            IXMLDOMText __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeValue )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ VARIANT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeType )( 
            IXMLDOMText __RPC_FAR * This,
            /* [retval][out] */ DOMNodeType __RPC_FAR *type);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parentNode )( 
            IXMLDOMText __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *parent);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_childNodes )( 
            IXMLDOMText __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *childList);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_firstChild )( 
            IXMLDOMText __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *firstChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_lastChild )( 
            IXMLDOMText __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *lastChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_previousSibling )( 
            IXMLDOMText __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *previousSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nextSibling )( 
            IXMLDOMText __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributes )( 
            IXMLDOMText __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *attributeMap);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertBefore )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ VARIANT refChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceChild )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ IXMLDOMNode __RPC_FAR *oldChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outOldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *childNode,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *oldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendChild )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *hasChildNodes )( 
            IXMLDOMText __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ownerDocument )( 
            IXMLDOMText __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *DOMDocument);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *cloneNode )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL deep,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *cloneRoot);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypeString )( 
            IXMLDOMText __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nodeType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXMLDOMText __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *text);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ BSTR text);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_specified )( 
            IXMLDOMText __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSpecified);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_definition )( 
            IXMLDOMText __RPC_FAR * This,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *definitionNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypedValue )( 
            IXMLDOMText __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *typedValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeTypedValue )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ VARIANT typedValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dataType )( 
            IXMLDOMText __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *dataTypeName);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_dataType )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ BSTR dataTypeName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_xml )( 
            IXMLDOMText __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNode )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectNodes )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectSingleNode )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *resultNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parsed )( 
            IXMLDOMText __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isParsed);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_namespaceURI )( 
            IXMLDOMText __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *namespaceURI);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_prefix )( 
            IXMLDOMText __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *prefixString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_baseName )( 
            IXMLDOMText __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nameString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNodeToObject )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [in] */ VARIANT outputObject);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_data )( 
            IXMLDOMText __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *data);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_data )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ BSTR data);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_length )( 
            IXMLDOMText __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *dataLength);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *substringData )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ long offset,
            /* [in] */ long count,
            /* [retval][out] */ BSTR __RPC_FAR *data);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendData )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ BSTR data);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertData )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ long offset,
            /* [in] */ BSTR data);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *deleteData )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ long offset,
            /* [in] */ long count);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceData )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ long offset,
            /* [in] */ long count,
            /* [in] */ BSTR data);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *splitText )( 
            IXMLDOMText __RPC_FAR * This,
            /* [in] */ long offset,
            /* [retval][out] */ IXMLDOMText __RPC_FAR *__RPC_FAR *rightHandTextNode);
        
        END_INTERFACE
    } IXMLDOMTextVtbl;

    interface IXMLDOMText
    {
        CONST_VTBL struct IXMLDOMTextVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMText_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMText_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMText_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMText_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMText_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMText_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMText_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMText_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMText_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMText_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMText_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMText_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMText_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMText_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMText_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMText_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMText_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMText_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMText_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMText_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMText_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMText_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMText_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMText_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMText_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMText_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMText_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMText_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMText_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMText_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMText_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMText_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMText_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMText_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMText_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMText_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMText_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMText_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMText_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMText_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMText_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMText_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMText_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMText_get_data(This,data)	\
    (This)->lpVtbl -> get_data(This,data)

#define IXMLDOMText_put_data(This,data)	\
    (This)->lpVtbl -> put_data(This,data)

#define IXMLDOMText_get_length(This,dataLength)	\
    (This)->lpVtbl -> get_length(This,dataLength)

#define IXMLDOMText_substringData(This,offset,count,data)	\
    (This)->lpVtbl -> substringData(This,offset,count,data)

#define IXMLDOMText_appendData(This,data)	\
    (This)->lpVtbl -> appendData(This,data)

#define IXMLDOMText_insertData(This,offset,data)	\
    (This)->lpVtbl -> insertData(This,offset,data)

#define IXMLDOMText_deleteData(This,offset,count)	\
    (This)->lpVtbl -> deleteData(This,offset,count)

#define IXMLDOMText_replaceData(This,offset,count,data)	\
    (This)->lpVtbl -> replaceData(This,offset,count,data)


#define IXMLDOMText_splitText(This,offset,rightHandTextNode)	\
    (This)->lpVtbl -> splitText(This,offset,rightHandTextNode)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDOMText_splitText_Proxy( 
    IXMLDOMText __RPC_FAR * This,
    /* [in] */ long offset,
    /* [retval][out] */ IXMLDOMText __RPC_FAR *__RPC_FAR *rightHandTextNode);


void __RPC_STUB IXMLDOMText_splitText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLDOMText_INTERFACE_DEFINED__ */


#ifndef __IXMLDOMComment_INTERFACE_DEFINED__
#define __IXMLDOMComment_INTERFACE_DEFINED__

/* interface IXMLDOMComment */
/* [unique][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDOMComment;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF88-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMComment : public IXMLDOMCharacterData
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMCommentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMComment __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMComment __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeName )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *name);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeValue )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeValue )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ VARIANT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeType )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [retval][out] */ DOMNodeType __RPC_FAR *type);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parentNode )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *parent);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_childNodes )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *childList);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_firstChild )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *firstChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_lastChild )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *lastChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_previousSibling )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *previousSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nextSibling )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributes )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *attributeMap);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertBefore )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ VARIANT refChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceChild )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ IXMLDOMNode __RPC_FAR *oldChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outOldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *childNode,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *oldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendChild )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *hasChildNodes )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ownerDocument )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *DOMDocument);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *cloneNode )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL deep,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *cloneRoot);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypeString )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nodeType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *text);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ BSTR text);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_specified )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSpecified);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_definition )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *definitionNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypedValue )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *typedValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeTypedValue )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ VARIANT typedValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dataType )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *dataTypeName);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_dataType )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ BSTR dataTypeName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_xml )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNode )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectNodes )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectSingleNode )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *resultNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parsed )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isParsed);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_namespaceURI )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *namespaceURI);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_prefix )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *prefixString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_baseName )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nameString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNodeToObject )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [in] */ VARIANT outputObject);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_data )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *data);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_data )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ BSTR data);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_length )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *dataLength);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *substringData )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ long offset,
            /* [in] */ long count,
            /* [retval][out] */ BSTR __RPC_FAR *data);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendData )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ BSTR data);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertData )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ long offset,
            /* [in] */ BSTR data);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *deleteData )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ long offset,
            /* [in] */ long count);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceData )( 
            IXMLDOMComment __RPC_FAR * This,
            /* [in] */ long offset,
            /* [in] */ long count,
            /* [in] */ BSTR data);
        
        END_INTERFACE
    } IXMLDOMCommentVtbl;

    interface IXMLDOMComment
    {
        CONST_VTBL struct IXMLDOMCommentVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMComment_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMComment_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMComment_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMComment_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMComment_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMComment_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMComment_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMComment_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMComment_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMComment_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMComment_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMComment_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMComment_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMComment_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMComment_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMComment_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMComment_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMComment_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMComment_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMComment_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMComment_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMComment_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMComment_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMComment_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMComment_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMComment_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMComment_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMComment_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMComment_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMComment_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMComment_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMComment_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMComment_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMComment_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMComment_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMComment_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMComment_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMComment_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMComment_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMComment_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMComment_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMComment_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMComment_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMComment_get_data(This,data)	\
    (This)->lpVtbl -> get_data(This,data)

#define IXMLDOMComment_put_data(This,data)	\
    (This)->lpVtbl -> put_data(This,data)

#define IXMLDOMComment_get_length(This,dataLength)	\
    (This)->lpVtbl -> get_length(This,dataLength)

#define IXMLDOMComment_substringData(This,offset,count,data)	\
    (This)->lpVtbl -> substringData(This,offset,count,data)

#define IXMLDOMComment_appendData(This,data)	\
    (This)->lpVtbl -> appendData(This,data)

#define IXMLDOMComment_insertData(This,offset,data)	\
    (This)->lpVtbl -> insertData(This,offset,data)

#define IXMLDOMComment_deleteData(This,offset,count)	\
    (This)->lpVtbl -> deleteData(This,offset,count)

#define IXMLDOMComment_replaceData(This,offset,count,data)	\
    (This)->lpVtbl -> replaceData(This,offset,count,data)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IXMLDOMComment_INTERFACE_DEFINED__ */


#ifndef __IXMLDOMProcessingInstruction_INTERFACE_DEFINED__
#define __IXMLDOMProcessingInstruction_INTERFACE_DEFINED__

/* interface IXMLDOMProcessingInstruction */
/* [unique][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDOMProcessingInstruction;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF89-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMProcessingInstruction : public IXMLDOMNode
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_target( 
            /* [retval][out] */ BSTR __RPC_FAR *name) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_data( 
            /* [retval][out] */ BSTR __RPC_FAR *value) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_data( 
            /* [in] */ BSTR value) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMProcessingInstructionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeName )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *name);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeValue )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeValue )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ VARIANT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeType )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [retval][out] */ DOMNodeType __RPC_FAR *type);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parentNode )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *parent);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_childNodes )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *childList);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_firstChild )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *firstChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_lastChild )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *lastChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_previousSibling )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *previousSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nextSibling )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributes )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *attributeMap);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertBefore )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ VARIANT refChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceChild )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ IXMLDOMNode __RPC_FAR *oldChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outOldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *childNode,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *oldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendChild )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *hasChildNodes )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ownerDocument )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *DOMDocument);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *cloneNode )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL deep,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *cloneRoot);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypeString )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nodeType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *text);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ BSTR text);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_specified )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSpecified);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_definition )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *definitionNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypedValue )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *typedValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeTypedValue )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ VARIANT typedValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dataType )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *dataTypeName);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_dataType )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ BSTR dataTypeName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_xml )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNode )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectNodes )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectSingleNode )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *resultNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parsed )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isParsed);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_namespaceURI )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *namespaceURI);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_prefix )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *prefixString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_baseName )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nameString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNodeToObject )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [in] */ VARIANT outputObject);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_target )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *name);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_data )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_data )( 
            IXMLDOMProcessingInstruction __RPC_FAR * This,
            /* [in] */ BSTR value);
        
        END_INTERFACE
    } IXMLDOMProcessingInstructionVtbl;

    interface IXMLDOMProcessingInstruction
    {
        CONST_VTBL struct IXMLDOMProcessingInstructionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMProcessingInstruction_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMProcessingInstruction_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMProcessingInstruction_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMProcessingInstruction_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMProcessingInstruction_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMProcessingInstruction_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMProcessingInstruction_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMProcessingInstruction_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMProcessingInstruction_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMProcessingInstruction_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMProcessingInstruction_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMProcessingInstruction_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMProcessingInstruction_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMProcessingInstruction_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMProcessingInstruction_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMProcessingInstruction_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMProcessingInstruction_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMProcessingInstruction_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMProcessingInstruction_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMProcessingInstruction_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMProcessingInstruction_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMProcessingInstruction_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMProcessingInstruction_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMProcessingInstruction_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMProcessingInstruction_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMProcessingInstruction_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMProcessingInstruction_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMProcessingInstruction_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMProcessingInstruction_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMProcessingInstruction_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMProcessingInstruction_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMProcessingInstruction_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMProcessingInstruction_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMProcessingInstruction_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMProcessingInstruction_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMProcessingInstruction_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMProcessingInstruction_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMProcessingInstruction_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMProcessingInstruction_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMProcessingInstruction_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMProcessingInstruction_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMProcessingInstruction_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMProcessingInstruction_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMProcessingInstruction_get_target(This,name)	\
    (This)->lpVtbl -> get_target(This,name)

#define IXMLDOMProcessingInstruction_get_data(This,value)	\
    (This)->lpVtbl -> get_data(This,value)

#define IXMLDOMProcessingInstruction_put_data(This,value)	\
    (This)->lpVtbl -> put_data(This,value)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMProcessingInstruction_get_target_Proxy( 
    IXMLDOMProcessingInstruction __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *name);


void __RPC_STUB IXMLDOMProcessingInstruction_get_target_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMProcessingInstruction_get_data_Proxy( 
    IXMLDOMProcessingInstruction __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *value);


void __RPC_STUB IXMLDOMProcessingInstruction_get_data_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDOMProcessingInstruction_put_data_Proxy( 
    IXMLDOMProcessingInstruction __RPC_FAR * This,
    /* [in] */ BSTR value);


void __RPC_STUB IXMLDOMProcessingInstruction_put_data_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLDOMProcessingInstruction_INTERFACE_DEFINED__ */


#ifndef __IXMLDOMCDATASection_INTERFACE_DEFINED__
#define __IXMLDOMCDATASection_INTERFACE_DEFINED__

/* interface IXMLDOMCDATASection */
/* [unique][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDOMCDATASection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF8A-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMCDATASection : public IXMLDOMText
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMCDATASectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMCDATASection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMCDATASection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeName )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *name);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeValue )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeValue )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ VARIANT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeType )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [retval][out] */ DOMNodeType __RPC_FAR *type);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parentNode )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *parent);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_childNodes )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *childList);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_firstChild )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *firstChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_lastChild )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *lastChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_previousSibling )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *previousSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nextSibling )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributes )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *attributeMap);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertBefore )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ VARIANT refChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceChild )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ IXMLDOMNode __RPC_FAR *oldChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outOldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *childNode,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *oldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendChild )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *hasChildNodes )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ownerDocument )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *DOMDocument);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *cloneNode )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL deep,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *cloneRoot);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypeString )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nodeType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *text);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ BSTR text);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_specified )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSpecified);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_definition )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *definitionNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypedValue )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *typedValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeTypedValue )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ VARIANT typedValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dataType )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *dataTypeName);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_dataType )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ BSTR dataTypeName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_xml )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNode )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectNodes )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectSingleNode )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *resultNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parsed )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isParsed);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_namespaceURI )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *namespaceURI);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_prefix )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *prefixString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_baseName )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nameString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNodeToObject )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [in] */ VARIANT outputObject);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_data )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *data);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_data )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ BSTR data);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_length )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *dataLength);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *substringData )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ long offset,
            /* [in] */ long count,
            /* [retval][out] */ BSTR __RPC_FAR *data);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendData )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ BSTR data);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertData )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ long offset,
            /* [in] */ BSTR data);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *deleteData )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ long offset,
            /* [in] */ long count);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceData )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ long offset,
            /* [in] */ long count,
            /* [in] */ BSTR data);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *splitText )( 
            IXMLDOMCDATASection __RPC_FAR * This,
            /* [in] */ long offset,
            /* [retval][out] */ IXMLDOMText __RPC_FAR *__RPC_FAR *rightHandTextNode);
        
        END_INTERFACE
    } IXMLDOMCDATASectionVtbl;

    interface IXMLDOMCDATASection
    {
        CONST_VTBL struct IXMLDOMCDATASectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMCDATASection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMCDATASection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMCDATASection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMCDATASection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMCDATASection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMCDATASection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMCDATASection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMCDATASection_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMCDATASection_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMCDATASection_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMCDATASection_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMCDATASection_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMCDATASection_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMCDATASection_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMCDATASection_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMCDATASection_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMCDATASection_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMCDATASection_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMCDATASection_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMCDATASection_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMCDATASection_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMCDATASection_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMCDATASection_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMCDATASection_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMCDATASection_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMCDATASection_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMCDATASection_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMCDATASection_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMCDATASection_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMCDATASection_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMCDATASection_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMCDATASection_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMCDATASection_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMCDATASection_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMCDATASection_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMCDATASection_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMCDATASection_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMCDATASection_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMCDATASection_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMCDATASection_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMCDATASection_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMCDATASection_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMCDATASection_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMCDATASection_get_data(This,data)	\
    (This)->lpVtbl -> get_data(This,data)

#define IXMLDOMCDATASection_put_data(This,data)	\
    (This)->lpVtbl -> put_data(This,data)

#define IXMLDOMCDATASection_get_length(This,dataLength)	\
    (This)->lpVtbl -> get_length(This,dataLength)

#define IXMLDOMCDATASection_substringData(This,offset,count,data)	\
    (This)->lpVtbl -> substringData(This,offset,count,data)

#define IXMLDOMCDATASection_appendData(This,data)	\
    (This)->lpVtbl -> appendData(This,data)

#define IXMLDOMCDATASection_insertData(This,offset,data)	\
    (This)->lpVtbl -> insertData(This,offset,data)

#define IXMLDOMCDATASection_deleteData(This,offset,count)	\
    (This)->lpVtbl -> deleteData(This,offset,count)

#define IXMLDOMCDATASection_replaceData(This,offset,count,data)	\
    (This)->lpVtbl -> replaceData(This,offset,count,data)


#define IXMLDOMCDATASection_splitText(This,offset,rightHandTextNode)	\
    (This)->lpVtbl -> splitText(This,offset,rightHandTextNode)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IXMLDOMCDATASection_INTERFACE_DEFINED__ */


#ifndef __IXMLDOMDocumentType_INTERFACE_DEFINED__
#define __IXMLDOMDocumentType_INTERFACE_DEFINED__

/* interface IXMLDOMDocumentType */
/* [unique][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDOMDocumentType;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF8B-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMDocumentType : public IXMLDOMNode
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_name( 
            /* [retval][out] */ BSTR __RPC_FAR *rootName) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_entities( 
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *entityMap) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_notations( 
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *notationMap) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMDocumentTypeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMDocumentType __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMDocumentType __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeName )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *name);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeValue )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeValue )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [in] */ VARIANT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeType )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [retval][out] */ DOMNodeType __RPC_FAR *type);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parentNode )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *parent);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_childNodes )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *childList);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_firstChild )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *firstChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_lastChild )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *lastChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_previousSibling )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *previousSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nextSibling )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributes )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *attributeMap);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertBefore )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ VARIANT refChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceChild )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ IXMLDOMNode __RPC_FAR *oldChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outOldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *childNode,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *oldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendChild )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *hasChildNodes )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ownerDocument )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *DOMDocument);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *cloneNode )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL deep,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *cloneRoot);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypeString )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nodeType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *text);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [in] */ BSTR text);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_specified )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSpecified);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_definition )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *definitionNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypedValue )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *typedValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeTypedValue )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [in] */ VARIANT typedValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dataType )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *dataTypeName);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_dataType )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [in] */ BSTR dataTypeName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_xml )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNode )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectNodes )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectSingleNode )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *resultNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parsed )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isParsed);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_namespaceURI )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *namespaceURI);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_prefix )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *prefixString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_baseName )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nameString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNodeToObject )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [in] */ VARIANT outputObject);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_name )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *rootName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_entities )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *entityMap);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_notations )( 
            IXMLDOMDocumentType __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *notationMap);
        
        END_INTERFACE
    } IXMLDOMDocumentTypeVtbl;

    interface IXMLDOMDocumentType
    {
        CONST_VTBL struct IXMLDOMDocumentTypeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMDocumentType_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMDocumentType_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMDocumentType_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMDocumentType_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMDocumentType_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMDocumentType_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMDocumentType_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMDocumentType_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMDocumentType_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMDocumentType_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMDocumentType_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMDocumentType_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMDocumentType_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMDocumentType_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMDocumentType_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMDocumentType_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMDocumentType_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMDocumentType_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMDocumentType_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMDocumentType_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMDocumentType_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMDocumentType_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMDocumentType_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMDocumentType_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMDocumentType_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMDocumentType_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMDocumentType_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMDocumentType_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMDocumentType_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMDocumentType_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMDocumentType_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMDocumentType_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMDocumentType_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMDocumentType_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMDocumentType_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMDocumentType_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMDocumentType_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMDocumentType_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMDocumentType_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMDocumentType_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMDocumentType_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMDocumentType_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMDocumentType_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMDocumentType_get_name(This,rootName)	\
    (This)->lpVtbl -> get_name(This,rootName)

#define IXMLDOMDocumentType_get_entities(This,entityMap)	\
    (This)->lpVtbl -> get_entities(This,entityMap)

#define IXMLDOMDocumentType_get_notations(This,notationMap)	\
    (This)->lpVtbl -> get_notations(This,notationMap)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocumentType_get_name_Proxy( 
    IXMLDOMDocumentType __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *rootName);


void __RPC_STUB IXMLDOMDocumentType_get_name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocumentType_get_entities_Proxy( 
    IXMLDOMDocumentType __RPC_FAR * This,
    /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *entityMap);


void __RPC_STUB IXMLDOMDocumentType_get_entities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMDocumentType_get_notations_Proxy( 
    IXMLDOMDocumentType __RPC_FAR * This,
    /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *notationMap);


void __RPC_STUB IXMLDOMDocumentType_get_notations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLDOMDocumentType_INTERFACE_DEFINED__ */


#ifndef __IXMLDOMNotation_INTERFACE_DEFINED__
#define __IXMLDOMNotation_INTERFACE_DEFINED__

/* interface IXMLDOMNotation */
/* [unique][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDOMNotation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF8C-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMNotation : public IXMLDOMNode
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_publicId( 
            /* [retval][out] */ VARIANT __RPC_FAR *publicID) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_systemId( 
            /* [retval][out] */ VARIANT __RPC_FAR *systemID) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMNotationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMNotation __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMNotation __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeName )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *name);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeValue )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeValue )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [in] */ VARIANT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeType )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [retval][out] */ DOMNodeType __RPC_FAR *type);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parentNode )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *parent);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_childNodes )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *childList);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_firstChild )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *firstChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_lastChild )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *lastChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_previousSibling )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *previousSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nextSibling )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributes )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *attributeMap);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertBefore )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ VARIANT refChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceChild )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ IXMLDOMNode __RPC_FAR *oldChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outOldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *childNode,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *oldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendChild )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *hasChildNodes )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ownerDocument )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *DOMDocument);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *cloneNode )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL deep,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *cloneRoot);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypeString )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nodeType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *text);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [in] */ BSTR text);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_specified )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSpecified);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_definition )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *definitionNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypedValue )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *typedValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeTypedValue )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [in] */ VARIANT typedValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dataType )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *dataTypeName);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_dataType )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [in] */ BSTR dataTypeName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_xml )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNode )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectNodes )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectSingleNode )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *resultNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parsed )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isParsed);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_namespaceURI )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *namespaceURI);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_prefix )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *prefixString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_baseName )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nameString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNodeToObject )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [in] */ VARIANT outputObject);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_publicId )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *publicID);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_systemId )( 
            IXMLDOMNotation __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *systemID);
        
        END_INTERFACE
    } IXMLDOMNotationVtbl;

    interface IXMLDOMNotation
    {
        CONST_VTBL struct IXMLDOMNotationVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMNotation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMNotation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMNotation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMNotation_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMNotation_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMNotation_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMNotation_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMNotation_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMNotation_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMNotation_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMNotation_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMNotation_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMNotation_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMNotation_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMNotation_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMNotation_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMNotation_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMNotation_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMNotation_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMNotation_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMNotation_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMNotation_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMNotation_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMNotation_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMNotation_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMNotation_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMNotation_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMNotation_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMNotation_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMNotation_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMNotation_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMNotation_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMNotation_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMNotation_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMNotation_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMNotation_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMNotation_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMNotation_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMNotation_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMNotation_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMNotation_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMNotation_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMNotation_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMNotation_get_publicId(This,publicID)	\
    (This)->lpVtbl -> get_publicId(This,publicID)

#define IXMLDOMNotation_get_systemId(This,systemID)	\
    (This)->lpVtbl -> get_systemId(This,systemID)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNotation_get_publicId_Proxy( 
    IXMLDOMNotation __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *publicID);


void __RPC_STUB IXMLDOMNotation_get_publicId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMNotation_get_systemId_Proxy( 
    IXMLDOMNotation __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *systemID);


void __RPC_STUB IXMLDOMNotation_get_systemId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLDOMNotation_INTERFACE_DEFINED__ */


#ifndef __IXMLDOMEntity_INTERFACE_DEFINED__
#define __IXMLDOMEntity_INTERFACE_DEFINED__

/* interface IXMLDOMEntity */
/* [unique][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDOMEntity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF8D-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMEntity : public IXMLDOMNode
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_publicId( 
            /* [retval][out] */ VARIANT __RPC_FAR *publicID) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_systemId( 
            /* [retval][out] */ VARIANT __RPC_FAR *systemID) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_notationName( 
            /* [retval][out] */ BSTR __RPC_FAR *name) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMEntityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMEntity __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMEntity __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeName )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *name);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeValue )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeValue )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [in] */ VARIANT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeType )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [retval][out] */ DOMNodeType __RPC_FAR *type);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parentNode )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *parent);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_childNodes )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *childList);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_firstChild )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *firstChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_lastChild )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *lastChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_previousSibling )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *previousSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nextSibling )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributes )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *attributeMap);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertBefore )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ VARIANT refChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceChild )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ IXMLDOMNode __RPC_FAR *oldChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outOldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *childNode,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *oldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendChild )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *hasChildNodes )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ownerDocument )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *DOMDocument);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *cloneNode )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL deep,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *cloneRoot);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypeString )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nodeType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *text);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [in] */ BSTR text);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_specified )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSpecified);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_definition )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *definitionNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypedValue )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *typedValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeTypedValue )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [in] */ VARIANT typedValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dataType )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *dataTypeName);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_dataType )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [in] */ BSTR dataTypeName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_xml )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNode )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectNodes )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectSingleNode )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *resultNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parsed )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isParsed);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_namespaceURI )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *namespaceURI);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_prefix )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *prefixString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_baseName )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nameString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNodeToObject )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [in] */ VARIANT outputObject);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_publicId )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *publicID);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_systemId )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *systemID);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_notationName )( 
            IXMLDOMEntity __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *name);
        
        END_INTERFACE
    } IXMLDOMEntityVtbl;

    interface IXMLDOMEntity
    {
        CONST_VTBL struct IXMLDOMEntityVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMEntity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMEntity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMEntity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMEntity_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMEntity_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMEntity_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMEntity_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMEntity_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMEntity_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMEntity_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMEntity_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMEntity_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMEntity_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMEntity_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMEntity_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMEntity_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMEntity_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMEntity_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMEntity_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMEntity_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMEntity_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMEntity_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMEntity_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMEntity_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMEntity_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMEntity_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMEntity_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMEntity_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMEntity_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMEntity_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMEntity_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMEntity_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMEntity_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMEntity_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMEntity_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMEntity_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMEntity_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMEntity_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMEntity_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMEntity_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMEntity_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMEntity_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMEntity_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXMLDOMEntity_get_publicId(This,publicID)	\
    (This)->lpVtbl -> get_publicId(This,publicID)

#define IXMLDOMEntity_get_systemId(This,systemID)	\
    (This)->lpVtbl -> get_systemId(This,systemID)

#define IXMLDOMEntity_get_notationName(This,name)	\
    (This)->lpVtbl -> get_notationName(This,name)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMEntity_get_publicId_Proxy( 
    IXMLDOMEntity __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *publicID);


void __RPC_STUB IXMLDOMEntity_get_publicId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMEntity_get_systemId_Proxy( 
    IXMLDOMEntity __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *systemID);


void __RPC_STUB IXMLDOMEntity_get_systemId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMEntity_get_notationName_Proxy( 
    IXMLDOMEntity __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *name);


void __RPC_STUB IXMLDOMEntity_get_notationName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLDOMEntity_INTERFACE_DEFINED__ */


#ifndef __IXMLDOMEntityReference_INTERFACE_DEFINED__
#define __IXMLDOMEntityReference_INTERFACE_DEFINED__

/* interface IXMLDOMEntityReference */
/* [unique][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDOMEntityReference;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2933BF8E-7B36-11d2-B20E-00C04F983E60")
    IXMLDOMEntityReference : public IXMLDOMNode
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMEntityReferenceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMEntityReference __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMEntityReference __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeName )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *name);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeValue )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeValue )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [in] */ VARIANT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeType )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [retval][out] */ DOMNodeType __RPC_FAR *type);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parentNode )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *parent);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_childNodes )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *childList);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_firstChild )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *firstChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_lastChild )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *lastChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_previousSibling )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *previousSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nextSibling )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributes )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *attributeMap);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertBefore )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ VARIANT refChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceChild )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ IXMLDOMNode __RPC_FAR *oldChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outOldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *childNode,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *oldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendChild )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *hasChildNodes )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ownerDocument )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *DOMDocument);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *cloneNode )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL deep,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *cloneRoot);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypeString )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nodeType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *text);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [in] */ BSTR text);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_specified )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSpecified);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_definition )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *definitionNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypedValue )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *typedValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeTypedValue )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [in] */ VARIANT typedValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dataType )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *dataTypeName);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_dataType )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [in] */ BSTR dataTypeName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_xml )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNode )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectNodes )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectSingleNode )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *resultNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parsed )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isParsed);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_namespaceURI )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *namespaceURI);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_prefix )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *prefixString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_baseName )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nameString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNodeToObject )( 
            IXMLDOMEntityReference __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [in] */ VARIANT outputObject);
        
        END_INTERFACE
    } IXMLDOMEntityReferenceVtbl;

    interface IXMLDOMEntityReference
    {
        CONST_VTBL struct IXMLDOMEntityReferenceVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMEntityReference_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMEntityReference_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMEntityReference_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMEntityReference_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMEntityReference_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMEntityReference_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMEntityReference_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMEntityReference_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXMLDOMEntityReference_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXMLDOMEntityReference_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXMLDOMEntityReference_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXMLDOMEntityReference_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXMLDOMEntityReference_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXMLDOMEntityReference_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXMLDOMEntityReference_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXMLDOMEntityReference_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXMLDOMEntityReference_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXMLDOMEntityReference_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXMLDOMEntityReference_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXMLDOMEntityReference_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXMLDOMEntityReference_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXMLDOMEntityReference_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXMLDOMEntityReference_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXMLDOMEntityReference_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXMLDOMEntityReference_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXMLDOMEntityReference_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXMLDOMEntityReference_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXMLDOMEntityReference_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXMLDOMEntityReference_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXMLDOMEntityReference_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXMLDOMEntityReference_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXMLDOMEntityReference_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXMLDOMEntityReference_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXMLDOMEntityReference_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXMLDOMEntityReference_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXMLDOMEntityReference_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXMLDOMEntityReference_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXMLDOMEntityReference_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXMLDOMEntityReference_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXMLDOMEntityReference_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXMLDOMEntityReference_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXMLDOMEntityReference_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXMLDOMEntityReference_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IXMLDOMEntityReference_INTERFACE_DEFINED__ */


#ifndef __IXMLDOMParseError_INTERFACE_DEFINED__
#define __IXMLDOMParseError_INTERFACE_DEFINED__

/* interface IXMLDOMParseError */
/* [unique][helpstring][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDOMParseError;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3efaa426-272f-11d2-836f-0000f87a7782")
    IXMLDOMParseError : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_errorCode( 
            /* [out][retval] */ long __RPC_FAR *errorCode) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_url( 
            /* [out][retval] */ BSTR __RPC_FAR *urlString) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_reason( 
            /* [out][retval] */ BSTR __RPC_FAR *reasonString) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_srcText( 
            /* [out][retval] */ BSTR __RPC_FAR *sourceString) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_line( 
            /* [out][retval] */ long __RPC_FAR *lineNumber) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_linepos( 
            /* [out][retval] */ long __RPC_FAR *linePosition) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_filepos( 
            /* [out][retval] */ long __RPC_FAR *filePosition) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLDOMParseErrorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDOMParseError __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDOMParseError __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDOMParseError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDOMParseError __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDOMParseError __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDOMParseError __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDOMParseError __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_errorCode )( 
            IXMLDOMParseError __RPC_FAR * This,
            /* [out][retval] */ long __RPC_FAR *errorCode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_url )( 
            IXMLDOMParseError __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *urlString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_reason )( 
            IXMLDOMParseError __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *reasonString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_srcText )( 
            IXMLDOMParseError __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *sourceString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_line )( 
            IXMLDOMParseError __RPC_FAR * This,
            /* [out][retval] */ long __RPC_FAR *lineNumber);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_linepos )( 
            IXMLDOMParseError __RPC_FAR * This,
            /* [out][retval] */ long __RPC_FAR *linePosition);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_filepos )( 
            IXMLDOMParseError __RPC_FAR * This,
            /* [out][retval] */ long __RPC_FAR *filePosition);
        
        END_INTERFACE
    } IXMLDOMParseErrorVtbl;

    interface IXMLDOMParseError
    {
        CONST_VTBL struct IXMLDOMParseErrorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDOMParseError_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDOMParseError_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDOMParseError_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDOMParseError_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDOMParseError_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDOMParseError_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDOMParseError_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDOMParseError_get_errorCode(This,errorCode)	\
    (This)->lpVtbl -> get_errorCode(This,errorCode)

#define IXMLDOMParseError_get_url(This,urlString)	\
    (This)->lpVtbl -> get_url(This,urlString)

#define IXMLDOMParseError_get_reason(This,reasonString)	\
    (This)->lpVtbl -> get_reason(This,reasonString)

#define IXMLDOMParseError_get_srcText(This,sourceString)	\
    (This)->lpVtbl -> get_srcText(This,sourceString)

#define IXMLDOMParseError_get_line(This,lineNumber)	\
    (This)->lpVtbl -> get_line(This,lineNumber)

#define IXMLDOMParseError_get_linepos(This,linePosition)	\
    (This)->lpVtbl -> get_linepos(This,linePosition)

#define IXMLDOMParseError_get_filepos(This,filePosition)	\
    (This)->lpVtbl -> get_filepos(This,filePosition)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMParseError_get_errorCode_Proxy( 
    IXMLDOMParseError __RPC_FAR * This,
    /* [out][retval] */ long __RPC_FAR *errorCode);


void __RPC_STUB IXMLDOMParseError_get_errorCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMParseError_get_url_Proxy( 
    IXMLDOMParseError __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *urlString);


void __RPC_STUB IXMLDOMParseError_get_url_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMParseError_get_reason_Proxy( 
    IXMLDOMParseError __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *reasonString);


void __RPC_STUB IXMLDOMParseError_get_reason_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMParseError_get_srcText_Proxy( 
    IXMLDOMParseError __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *sourceString);


void __RPC_STUB IXMLDOMParseError_get_srcText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMParseError_get_line_Proxy( 
    IXMLDOMParseError __RPC_FAR * This,
    /* [out][retval] */ long __RPC_FAR *lineNumber);


void __RPC_STUB IXMLDOMParseError_get_line_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMParseError_get_linepos_Proxy( 
    IXMLDOMParseError __RPC_FAR * This,
    /* [out][retval] */ long __RPC_FAR *linePosition);


void __RPC_STUB IXMLDOMParseError_get_linepos_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDOMParseError_get_filepos_Proxy( 
    IXMLDOMParseError __RPC_FAR * This,
    /* [out][retval] */ long __RPC_FAR *filePosition);


void __RPC_STUB IXMLDOMParseError_get_filepos_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLDOMParseError_INTERFACE_DEFINED__ */


#ifndef __IXTLRuntime_INTERFACE_DEFINED__
#define __IXTLRuntime_INTERFACE_DEFINED__

/* interface IXTLRuntime */
/* [unique][helpstring][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXTLRuntime;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3efaa425-272f-11d2-836f-0000f87a7782")
    IXTLRuntime : public IXMLDOMNode
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE uniqueID( 
            /* [in] */ IXMLDOMNode __RPC_FAR *pNode,
            /* [retval][out] */ long __RPC_FAR *pID) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE depth( 
            /* [in] */ IXMLDOMNode __RPC_FAR *pNode,
            /* [retval][out] */ long __RPC_FAR *pDepth) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE childNumber( 
            /* [in] */ IXMLDOMNode __RPC_FAR *pNode,
            /* [retval][out] */ long __RPC_FAR *pNumber) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ancestorChildNumber( 
            /* [in] */ BSTR bstrNodeName,
            /* [in] */ IXMLDOMNode __RPC_FAR *pNode,
            /* [retval][out] */ long __RPC_FAR *pNumber) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE absoluteChildNumber( 
            /* [in] */ IXMLDOMNode __RPC_FAR *pNode,
            /* [retval][out] */ long __RPC_FAR *pNumber) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE formatIndex( 
            /* [in] */ long lIndex,
            /* [in] */ BSTR bstrFormat,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrFormattedString) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE formatNumber( 
            /* [in] */ double dblNumber,
            /* [in] */ BSTR bstrFormat,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrFormattedString) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE formatDate( 
            /* [in] */ VARIANT varDate,
            /* [in] */ BSTR bstrFormat,
            /* [optional][in] */ VARIANT varDestLocale,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrFormattedString) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE formatTime( 
            /* [in] */ VARIANT varTime,
            /* [in] */ BSTR bstrFormat,
            /* [optional][in] */ VARIANT varDestLocale,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrFormattedString) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXTLRuntimeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXTLRuntime __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXTLRuntime __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXTLRuntime __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeName )( 
            IXTLRuntime __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *name);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeValue )( 
            IXTLRuntime __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeValue )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ VARIANT value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeType )( 
            IXTLRuntime __RPC_FAR * This,
            /* [retval][out] */ DOMNodeType __RPC_FAR *type);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parentNode )( 
            IXTLRuntime __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *parent);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_childNodes )( 
            IXTLRuntime __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *childList);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_firstChild )( 
            IXTLRuntime __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *firstChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_lastChild )( 
            IXTLRuntime __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *lastChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_previousSibling )( 
            IXTLRuntime __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *previousSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nextSibling )( 
            IXTLRuntime __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *nextSibling);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributes )( 
            IXTLRuntime __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMNamedNodeMap __RPC_FAR *__RPC_FAR *attributeMap);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *insertBefore )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ VARIANT refChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *replaceChild )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [in] */ IXMLDOMNode __RPC_FAR *oldChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outOldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *childNode,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *oldChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *appendChild )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *newChild,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *outNewChild);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *hasChildNodes )( 
            IXTLRuntime __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *hasChild);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ownerDocument )( 
            IXTLRuntime __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *DOMDocument);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *cloneNode )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL deep,
            /* [retval][out] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *cloneRoot);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypeString )( 
            IXTLRuntime __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nodeType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXTLRuntime __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *text);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ BSTR text);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_specified )( 
            IXTLRuntime __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *isSpecified);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_definition )( 
            IXTLRuntime __RPC_FAR * This,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *definitionNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_nodeTypedValue )( 
            IXTLRuntime __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *typedValue);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_nodeTypedValue )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ VARIANT typedValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dataType )( 
            IXTLRuntime __RPC_FAR * This,
            /* [out][retval] */ VARIANT __RPC_FAR *dataTypeName);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_dataType )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ BSTR dataTypeName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_xml )( 
            IXTLRuntime __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNode )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [out][retval] */ BSTR __RPC_FAR *xmlString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectNodes )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNodeList __RPC_FAR *__RPC_FAR *resultList);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *selectSingleNode )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ BSTR queryString,
            /* [out][retval] */ IXMLDOMNode __RPC_FAR *__RPC_FAR *resultNode);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parsed )( 
            IXTLRuntime __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *isParsed);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_namespaceURI )( 
            IXTLRuntime __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *namespaceURI);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_prefix )( 
            IXTLRuntime __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *prefixString);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_baseName )( 
            IXTLRuntime __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *nameString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *transformNodeToObject )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *stylesheet,
            /* [in] */ VARIANT outputObject);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *uniqueID )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *pNode,
            /* [retval][out] */ long __RPC_FAR *pID);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *depth )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *pNode,
            /* [retval][out] */ long __RPC_FAR *pDepth);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *childNumber )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *pNode,
            /* [retval][out] */ long __RPC_FAR *pNumber);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ancestorChildNumber )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ BSTR bstrNodeName,
            /* [in] */ IXMLDOMNode __RPC_FAR *pNode,
            /* [retval][out] */ long __RPC_FAR *pNumber);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *absoluteChildNumber )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ IXMLDOMNode __RPC_FAR *pNode,
            /* [retval][out] */ long __RPC_FAR *pNumber);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *formatIndex )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ long lIndex,
            /* [in] */ BSTR bstrFormat,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrFormattedString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *formatNumber )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ double dblNumber,
            /* [in] */ BSTR bstrFormat,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrFormattedString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *formatDate )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ VARIANT varDate,
            /* [in] */ BSTR bstrFormat,
            /* [optional][in] */ VARIANT varDestLocale,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrFormattedString);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *formatTime )( 
            IXTLRuntime __RPC_FAR * This,
            /* [in] */ VARIANT varTime,
            /* [in] */ BSTR bstrFormat,
            /* [optional][in] */ VARIANT varDestLocale,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrFormattedString);
        
        END_INTERFACE
    } IXTLRuntimeVtbl;

    interface IXTLRuntime
    {
        CONST_VTBL struct IXTLRuntimeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXTLRuntime_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXTLRuntime_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXTLRuntime_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXTLRuntime_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXTLRuntime_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXTLRuntime_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXTLRuntime_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXTLRuntime_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IXTLRuntime_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IXTLRuntime_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IXTLRuntime_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IXTLRuntime_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IXTLRuntime_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IXTLRuntime_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IXTLRuntime_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IXTLRuntime_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IXTLRuntime_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IXTLRuntime_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IXTLRuntime_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IXTLRuntime_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IXTLRuntime_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IXTLRuntime_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IXTLRuntime_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IXTLRuntime_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IXTLRuntime_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IXTLRuntime_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IXTLRuntime_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IXTLRuntime_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IXTLRuntime_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IXTLRuntime_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IXTLRuntime_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IXTLRuntime_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IXTLRuntime_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IXTLRuntime_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IXTLRuntime_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IXTLRuntime_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IXTLRuntime_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IXTLRuntime_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IXTLRuntime_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IXTLRuntime_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IXTLRuntime_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IXTLRuntime_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IXTLRuntime_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IXTLRuntime_uniqueID(This,pNode,pID)	\
    (This)->lpVtbl -> uniqueID(This,pNode,pID)

#define IXTLRuntime_depth(This,pNode,pDepth)	\
    (This)->lpVtbl -> depth(This,pNode,pDepth)

#define IXTLRuntime_childNumber(This,pNode,pNumber)	\
    (This)->lpVtbl -> childNumber(This,pNode,pNumber)

#define IXTLRuntime_ancestorChildNumber(This,bstrNodeName,pNode,pNumber)	\
    (This)->lpVtbl -> ancestorChildNumber(This,bstrNodeName,pNode,pNumber)

#define IXTLRuntime_absoluteChildNumber(This,pNode,pNumber)	\
    (This)->lpVtbl -> absoluteChildNumber(This,pNode,pNumber)

#define IXTLRuntime_formatIndex(This,lIndex,bstrFormat,pbstrFormattedString)	\
    (This)->lpVtbl -> formatIndex(This,lIndex,bstrFormat,pbstrFormattedString)

#define IXTLRuntime_formatNumber(This,dblNumber,bstrFormat,pbstrFormattedString)	\
    (This)->lpVtbl -> formatNumber(This,dblNumber,bstrFormat,pbstrFormattedString)

#define IXTLRuntime_formatDate(This,varDate,bstrFormat,varDestLocale,pbstrFormattedString)	\
    (This)->lpVtbl -> formatDate(This,varDate,bstrFormat,varDestLocale,pbstrFormattedString)

#define IXTLRuntime_formatTime(This,varTime,bstrFormat,varDestLocale,pbstrFormattedString)	\
    (This)->lpVtbl -> formatTime(This,varTime,bstrFormat,varDestLocale,pbstrFormattedString)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXTLRuntime_uniqueID_Proxy( 
    IXTLRuntime __RPC_FAR * This,
    /* [in] */ IXMLDOMNode __RPC_FAR *pNode,
    /* [retval][out] */ long __RPC_FAR *pID);


void __RPC_STUB IXTLRuntime_uniqueID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXTLRuntime_depth_Proxy( 
    IXTLRuntime __RPC_FAR * This,
    /* [in] */ IXMLDOMNode __RPC_FAR *pNode,
    /* [retval][out] */ long __RPC_FAR *pDepth);


void __RPC_STUB IXTLRuntime_depth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXTLRuntime_childNumber_Proxy( 
    IXTLRuntime __RPC_FAR * This,
    /* [in] */ IXMLDOMNode __RPC_FAR *pNode,
    /* [retval][out] */ long __RPC_FAR *pNumber);


void __RPC_STUB IXTLRuntime_childNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXTLRuntime_ancestorChildNumber_Proxy( 
    IXTLRuntime __RPC_FAR * This,
    /* [in] */ BSTR bstrNodeName,
    /* [in] */ IXMLDOMNode __RPC_FAR *pNode,
    /* [retval][out] */ long __RPC_FAR *pNumber);


void __RPC_STUB IXTLRuntime_ancestorChildNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXTLRuntime_absoluteChildNumber_Proxy( 
    IXTLRuntime __RPC_FAR * This,
    /* [in] */ IXMLDOMNode __RPC_FAR *pNode,
    /* [retval][out] */ long __RPC_FAR *pNumber);


void __RPC_STUB IXTLRuntime_absoluteChildNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXTLRuntime_formatIndex_Proxy( 
    IXTLRuntime __RPC_FAR * This,
    /* [in] */ long lIndex,
    /* [in] */ BSTR bstrFormat,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrFormattedString);


void __RPC_STUB IXTLRuntime_formatIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXTLRuntime_formatNumber_Proxy( 
    IXTLRuntime __RPC_FAR * This,
    /* [in] */ double dblNumber,
    /* [in] */ BSTR bstrFormat,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrFormattedString);


void __RPC_STUB IXTLRuntime_formatNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXTLRuntime_formatDate_Proxy( 
    IXTLRuntime __RPC_FAR * This,
    /* [in] */ VARIANT varDate,
    /* [in] */ BSTR bstrFormat,
    /* [optional][in] */ VARIANT varDestLocale,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrFormattedString);


void __RPC_STUB IXTLRuntime_formatDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXTLRuntime_formatTime_Proxy( 
    IXTLRuntime __RPC_FAR * This,
    /* [in] */ VARIANT varTime,
    /* [in] */ BSTR bstrFormat,
    /* [optional][in] */ VARIANT varDestLocale,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrFormattedString);


void __RPC_STUB IXTLRuntime_formatTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXTLRuntime_INTERFACE_DEFINED__ */


#ifndef __XMLDOMDocumentEvents_DISPINTERFACE_DEFINED__
#define __XMLDOMDocumentEvents_DISPINTERFACE_DEFINED__

/* dispinterface XMLDOMDocumentEvents */
/* [uuid][hidden] */ 


EXTERN_C const IID DIID_XMLDOMDocumentEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("3efaa427-272f-11d2-836f-0000f87a7782")
    XMLDOMDocumentEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct XMLDOMDocumentEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            XMLDOMDocumentEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            XMLDOMDocumentEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            XMLDOMDocumentEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            XMLDOMDocumentEvents __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            XMLDOMDocumentEvents __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            XMLDOMDocumentEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            XMLDOMDocumentEvents __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } XMLDOMDocumentEventsVtbl;

    interface XMLDOMDocumentEvents
    {
        CONST_VTBL struct XMLDOMDocumentEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define XMLDOMDocumentEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define XMLDOMDocumentEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define XMLDOMDocumentEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define XMLDOMDocumentEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define XMLDOMDocumentEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define XMLDOMDocumentEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define XMLDOMDocumentEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __XMLDOMDocumentEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_DOMDocument;

#ifdef __cplusplus

class DECLSPEC_UUID("2933BF90-7B36-11d2-B20E-00C04F983E60")
DOMDocument;
#endif

EXTERN_C const CLSID CLSID_DOMFreeThreadedDocument;

#ifdef __cplusplus

class DECLSPEC_UUID("2933BF91-7B36-11d2-B20E-00C04F983E60")
DOMFreeThreadedDocument;
#endif

#ifndef __IXMLHttpRequest_INTERFACE_DEFINED__
#define __IXMLHttpRequest_INTERFACE_DEFINED__

/* interface IXMLHttpRequest */
/* [unique][helpstring][oleautomation][dual][uuid][object] */ 


EXTERN_C const IID IID_IXMLHttpRequest;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ED8C108D-4349-11D2-91A4-00C04F7969E8")
    IXMLHttpRequest : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE open( 
            /* [in] */ BSTR bstrMethod,
            /* [in] */ BSTR bstrUrl,
            /* [optional][in] */ VARIANT varAsync,
            /* [optional][in] */ VARIANT bstrUser,
            /* [optional][in] */ VARIANT bstrPassword) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setRequestHeader( 
            /* [in] */ BSTR bstrHeader,
            /* [in] */ BSTR bstrValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getResponseHeader( 
            /* [in] */ BSTR bstrHeader,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getAllResponseHeaders( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrHeaders) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE send( 
            /* [optional][in] */ VARIANT varBody) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE abort( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_status( 
            /* [retval][out] */ long __RPC_FAR *plStatus) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_statusText( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrStatus) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_responseXML( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppBody) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_responseText( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrBody) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_responseBody( 
            /* [retval][out] */ VARIANT __RPC_FAR *pvarBody) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_responseStream( 
            /* [retval][out] */ VARIANT __RPC_FAR *pvarBody) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_readyState( 
            /* [retval][out] */ long __RPC_FAR *plState) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_onreadystatechange( 
            /* [in] */ IDispatch __RPC_FAR *pReadyStateSink) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLHttpRequestVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLHttpRequest __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLHttpRequest __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *open )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [in] */ BSTR bstrMethod,
            /* [in] */ BSTR bstrUrl,
            /* [optional][in] */ VARIANT varAsync,
            /* [optional][in] */ VARIANT bstrUser,
            /* [optional][in] */ VARIANT bstrPassword);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setRequestHeader )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [in] */ BSTR bstrHeader,
            /* [in] */ BSTR bstrValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getResponseHeader )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [in] */ BSTR bstrHeader,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getAllResponseHeaders )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrHeaders);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *send )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [optional][in] */ VARIANT varBody);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *abort )( 
            IXMLHttpRequest __RPC_FAR * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_status )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plStatus);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_statusText )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrStatus);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_responseXML )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppBody);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_responseText )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrBody);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_responseBody )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *pvarBody);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_responseStream )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *pvarBody);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_readyState )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plState);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_onreadystatechange )( 
            IXMLHttpRequest __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *pReadyStateSink);
        
        END_INTERFACE
    } IXMLHttpRequestVtbl;

    interface IXMLHttpRequest
    {
        CONST_VTBL struct IXMLHttpRequestVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLHttpRequest_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLHttpRequest_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLHttpRequest_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLHttpRequest_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLHttpRequest_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLHttpRequest_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLHttpRequest_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLHttpRequest_open(This,bstrMethod,bstrUrl,varAsync,bstrUser,bstrPassword)	\
    (This)->lpVtbl -> open(This,bstrMethod,bstrUrl,varAsync,bstrUser,bstrPassword)

#define IXMLHttpRequest_setRequestHeader(This,bstrHeader,bstrValue)	\
    (This)->lpVtbl -> setRequestHeader(This,bstrHeader,bstrValue)

#define IXMLHttpRequest_getResponseHeader(This,bstrHeader,pbstrValue)	\
    (This)->lpVtbl -> getResponseHeader(This,bstrHeader,pbstrValue)

#define IXMLHttpRequest_getAllResponseHeaders(This,pbstrHeaders)	\
    (This)->lpVtbl -> getAllResponseHeaders(This,pbstrHeaders)

#define IXMLHttpRequest_send(This,varBody)	\
    (This)->lpVtbl -> send(This,varBody)

#define IXMLHttpRequest_abort(This)	\
    (This)->lpVtbl -> abort(This)

#define IXMLHttpRequest_get_status(This,plStatus)	\
    (This)->lpVtbl -> get_status(This,plStatus)

#define IXMLHttpRequest_get_statusText(This,pbstrStatus)	\
    (This)->lpVtbl -> get_statusText(This,pbstrStatus)

#define IXMLHttpRequest_get_responseXML(This,ppBody)	\
    (This)->lpVtbl -> get_responseXML(This,ppBody)

#define IXMLHttpRequest_get_responseText(This,pbstrBody)	\
    (This)->lpVtbl -> get_responseText(This,pbstrBody)

#define IXMLHttpRequest_get_responseBody(This,pvarBody)	\
    (This)->lpVtbl -> get_responseBody(This,pvarBody)

#define IXMLHttpRequest_get_responseStream(This,pvarBody)	\
    (This)->lpVtbl -> get_responseStream(This,pvarBody)

#define IXMLHttpRequest_get_readyState(This,plState)	\
    (This)->lpVtbl -> get_readyState(This,plState)

#define IXMLHttpRequest_put_onreadystatechange(This,pReadyStateSink)	\
    (This)->lpVtbl -> put_onreadystatechange(This,pReadyStateSink)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLHttpRequest_open_Proxy( 
    IXMLHttpRequest __RPC_FAR * This,
    /* [in] */ BSTR bstrMethod,
    /* [in] */ BSTR bstrUrl,
    /* [optional][in] */ VARIANT varAsync,
    /* [optional][in] */ VARIANT bstrUser,
    /* [optional][in] */ VARIANT bstrPassword);


void __RPC_STUB IXMLHttpRequest_open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLHttpRequest_setRequestHeader_Proxy( 
    IXMLHttpRequest __RPC_FAR * This,
    /* [in] */ BSTR bstrHeader,
    /* [in] */ BSTR bstrValue);


void __RPC_STUB IXMLHttpRequest_setRequestHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLHttpRequest_getResponseHeader_Proxy( 
    IXMLHttpRequest __RPC_FAR * This,
    /* [in] */ BSTR bstrHeader,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrValue);


void __RPC_STUB IXMLHttpRequest_getResponseHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLHttpRequest_getAllResponseHeaders_Proxy( 
    IXMLHttpRequest __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrHeaders);


void __RPC_STUB IXMLHttpRequest_getAllResponseHeaders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLHttpRequest_send_Proxy( 
    IXMLHttpRequest __RPC_FAR * This,
    /* [optional][in] */ VARIANT varBody);


void __RPC_STUB IXMLHttpRequest_send_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLHttpRequest_abort_Proxy( 
    IXMLHttpRequest __RPC_FAR * This);


void __RPC_STUB IXMLHttpRequest_abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLHttpRequest_get_status_Proxy( 
    IXMLHttpRequest __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plStatus);


void __RPC_STUB IXMLHttpRequest_get_status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLHttpRequest_get_statusText_Proxy( 
    IXMLHttpRequest __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrStatus);


void __RPC_STUB IXMLHttpRequest_get_statusText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLHttpRequest_get_responseXML_Proxy( 
    IXMLHttpRequest __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppBody);


void __RPC_STUB IXMLHttpRequest_get_responseXML_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLHttpRequest_get_responseText_Proxy( 
    IXMLHttpRequest __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrBody);


void __RPC_STUB IXMLHttpRequest_get_responseText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLHttpRequest_get_responseBody_Proxy( 
    IXMLHttpRequest __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *pvarBody);


void __RPC_STUB IXMLHttpRequest_get_responseBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLHttpRequest_get_responseStream_Proxy( 
    IXMLHttpRequest __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *pvarBody);


void __RPC_STUB IXMLHttpRequest_get_responseStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLHttpRequest_get_readyState_Proxy( 
    IXMLHttpRequest __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plState);


void __RPC_STUB IXMLHttpRequest_get_readyState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLHttpRequest_put_onreadystatechange_Proxy( 
    IXMLHttpRequest __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *pReadyStateSink);


void __RPC_STUB IXMLHttpRequest_put_onreadystatechange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLHttpRequest_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_XMLHTTPRequest;

#ifdef __cplusplus

class DECLSPEC_UUID("ED8C108E-4349-11D2-91A4-00C04F7969E8")
XMLHTTPRequest;
#endif

#ifndef __IXMLDSOControl_INTERFACE_DEFINED__
#define __IXMLDSOControl_INTERFACE_DEFINED__

/* interface IXMLDSOControl */
/* [unique][helpstring][hidden][nonextensible][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDSOControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("310afa62-0575-11d2-9ca9-0060b0ec3d39")
    IXMLDSOControl : public IDispatch
    {
    public:
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_XMLDocument( 
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *ppDoc) = 0;
        
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_XMLDocument( 
            /* [in] */ IXMLDOMDocument __RPC_FAR *ppDoc) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_JavaDSOCompatible( 
            /* [retval][out] */ BOOL __RPC_FAR *fJavaDSOCompatible) = 0;
        
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_JavaDSOCompatible( 
            /* [in] */ BOOL fJavaDSOCompatible) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_readyState( 
            /* [retval][out] */ long __RPC_FAR *state) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLDSOControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDSOControl __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDSOControl __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDSOControl __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDSOControl __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDSOControl __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDSOControl __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDSOControl __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_XMLDocument )( 
            IXMLDSOControl __RPC_FAR * This,
            /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *ppDoc);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_XMLDocument )( 
            IXMLDSOControl __RPC_FAR * This,
            /* [in] */ IXMLDOMDocument __RPC_FAR *ppDoc);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_JavaDSOCompatible )( 
            IXMLDSOControl __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *fJavaDSOCompatible);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_JavaDSOCompatible )( 
            IXMLDSOControl __RPC_FAR * This,
            /* [in] */ BOOL fJavaDSOCompatible);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_readyState )( 
            IXMLDSOControl __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *state);
        
        END_INTERFACE
    } IXMLDSOControlVtbl;

    interface IXMLDSOControl
    {
        CONST_VTBL struct IXMLDSOControlVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDSOControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDSOControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDSOControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDSOControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDSOControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDSOControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDSOControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDSOControl_get_XMLDocument(This,ppDoc)	\
    (This)->lpVtbl -> get_XMLDocument(This,ppDoc)

#define IXMLDSOControl_put_XMLDocument(This,ppDoc)	\
    (This)->lpVtbl -> put_XMLDocument(This,ppDoc)

#define IXMLDSOControl_get_JavaDSOCompatible(This,fJavaDSOCompatible)	\
    (This)->lpVtbl -> get_JavaDSOCompatible(This,fJavaDSOCompatible)

#define IXMLDSOControl_put_JavaDSOCompatible(This,fJavaDSOCompatible)	\
    (This)->lpVtbl -> put_JavaDSOCompatible(This,fJavaDSOCompatible)

#define IXMLDSOControl_get_readyState(This,state)	\
    (This)->lpVtbl -> get_readyState(This,state)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDSOControl_get_XMLDocument_Proxy( 
    IXMLDSOControl __RPC_FAR * This,
    /* [retval][out] */ IXMLDOMDocument __RPC_FAR *__RPC_FAR *ppDoc);


void __RPC_STUB IXMLDSOControl_get_XMLDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDSOControl_put_XMLDocument_Proxy( 
    IXMLDSOControl __RPC_FAR * This,
    /* [in] */ IXMLDOMDocument __RPC_FAR *ppDoc);


void __RPC_STUB IXMLDSOControl_put_XMLDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDSOControl_get_JavaDSOCompatible_Proxy( 
    IXMLDSOControl __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *fJavaDSOCompatible);


void __RPC_STUB IXMLDSOControl_get_JavaDSOCompatible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDSOControl_put_JavaDSOCompatible_Proxy( 
    IXMLDSOControl __RPC_FAR * This,
    /* [in] */ BOOL fJavaDSOCompatible);


void __RPC_STUB IXMLDSOControl_put_JavaDSOCompatible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDSOControl_get_readyState_Proxy( 
    IXMLDSOControl __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *state);


void __RPC_STUB IXMLDSOControl_get_readyState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLDSOControl_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_XMLDSOControl;

#ifdef __cplusplus

class DECLSPEC_UUID("550dda30-0541-11d2-9ca9-0060b0ec3d39")
XMLDSOControl;
#endif

#ifndef __IXMLElementCollection_INTERFACE_DEFINED__
#define __IXMLElementCollection_INTERFACE_DEFINED__

/* interface IXMLElementCollection */
/* [helpstring][hidden][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLElementCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("65725580-9B5D-11d0-9BFE-00C04FC99C8E")
    IXMLElementCollection : public IDispatch
    {
    public:
        virtual /* [id][hidden][restricted][propput] */ HRESULT STDMETHODCALLTYPE put_length( 
            /* [in] */ long v) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_length( 
            /* [out][retval] */ long __RPC_FAR *p) = 0;
        
        virtual /* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE get__newEnum( 
            /* [out][retval] */ IUnknown __RPC_FAR *__RPC_FAR *ppUnk) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE item( 
            /* [in][optional] */ VARIANT var1,
            /* [in][optional] */ VARIANT var2,
            /* [out][retval] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLElementCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLElementCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLElementCollection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLElementCollection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLElementCollection __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLElementCollection __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLElementCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLElementCollection __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id][hidden][restricted][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_length )( 
            IXMLElementCollection __RPC_FAR * This,
            /* [in] */ long v);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_length )( 
            IXMLElementCollection __RPC_FAR * This,
            /* [out][retval] */ long __RPC_FAR *p);
        
        /* [id][hidden][restricted][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__newEnum )( 
            IXMLElementCollection __RPC_FAR * This,
            /* [out][retval] */ IUnknown __RPC_FAR *__RPC_FAR *ppUnk);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *item )( 
            IXMLElementCollection __RPC_FAR * This,
            /* [in][optional] */ VARIANT var1,
            /* [in][optional] */ VARIANT var2,
            /* [out][retval] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
        END_INTERFACE
    } IXMLElementCollectionVtbl;

    interface IXMLElementCollection
    {
        CONST_VTBL struct IXMLElementCollectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLElementCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLElementCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLElementCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLElementCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLElementCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLElementCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLElementCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLElementCollection_put_length(This,v)	\
    (This)->lpVtbl -> put_length(This,v)

#define IXMLElementCollection_get_length(This,p)	\
    (This)->lpVtbl -> get_length(This,p)

#define IXMLElementCollection_get__newEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__newEnum(This,ppUnk)

#define IXMLElementCollection_item(This,var1,var2,ppDisp)	\
    (This)->lpVtbl -> item(This,var1,var2,ppDisp)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id][hidden][restricted][propput] */ HRESULT STDMETHODCALLTYPE IXMLElementCollection_put_length_Proxy( 
    IXMLElementCollection __RPC_FAR * This,
    /* [in] */ long v);


void __RPC_STUB IXMLElementCollection_put_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLElementCollection_get_length_Proxy( 
    IXMLElementCollection __RPC_FAR * This,
    /* [out][retval] */ long __RPC_FAR *p);


void __RPC_STUB IXMLElementCollection_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE IXMLElementCollection_get__newEnum_Proxy( 
    IXMLElementCollection __RPC_FAR * This,
    /* [out][retval] */ IUnknown __RPC_FAR *__RPC_FAR *ppUnk);


void __RPC_STUB IXMLElementCollection_get__newEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLElementCollection_item_Proxy( 
    IXMLElementCollection __RPC_FAR * This,
    /* [in][optional] */ VARIANT var1,
    /* [in][optional] */ VARIANT var2,
    /* [out][retval] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);


void __RPC_STUB IXMLElementCollection_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLElementCollection_INTERFACE_DEFINED__ */


#ifndef __IXMLDocument_INTERFACE_DEFINED__
#define __IXMLDocument_INTERFACE_DEFINED__

/* interface IXMLDocument */
/* [helpstring][hidden][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDocument;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F52E2B61-18A1-11d1-B105-00805F49916B")
    IXMLDocument : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_root( 
            /* [out][retval] */ IXMLElement __RPC_FAR *__RPC_FAR *p) = 0;
        
        virtual /* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE get_fileSize( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE get_fileModifiedDate( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE get_fileUpdatedDate( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_URL( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_URL( 
            /* [in] */ BSTR p) = 0;
        
        virtual /* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE get_mimeType( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_readyState( 
            /* [out][retval] */ long __RPC_FAR *pl) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_charset( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_charset( 
            /* [in] */ BSTR p) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_version( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_doctype( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE get_dtdURL( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE createElement( 
            /* [in] */ VARIANT vType,
            /* [in][optional] */ VARIANT var1,
            /* [out][retval] */ IXMLElement __RPC_FAR *__RPC_FAR *ppElem) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLDocumentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDocument __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDocument __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDocument __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDocument __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDocument __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDocument __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDocument __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_root )( 
            IXMLDocument __RPC_FAR * This,
            /* [out][retval] */ IXMLElement __RPC_FAR *__RPC_FAR *p);
        
        /* [id][hidden][restricted][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_fileSize )( 
            IXMLDocument __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [id][hidden][restricted][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_fileModifiedDate )( 
            IXMLDocument __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [id][hidden][restricted][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_fileUpdatedDate )( 
            IXMLDocument __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_URL )( 
            IXMLDocument __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_URL )( 
            IXMLDocument __RPC_FAR * This,
            /* [in] */ BSTR p);
        
        /* [id][hidden][restricted][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_mimeType )( 
            IXMLDocument __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_readyState )( 
            IXMLDocument __RPC_FAR * This,
            /* [out][retval] */ long __RPC_FAR *pl);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_charset )( 
            IXMLDocument __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_charset )( 
            IXMLDocument __RPC_FAR * This,
            /* [in] */ BSTR p);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_version )( 
            IXMLDocument __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_doctype )( 
            IXMLDocument __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [id][hidden][restricted][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dtdURL )( 
            IXMLDocument __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *createElement )( 
            IXMLDocument __RPC_FAR * This,
            /* [in] */ VARIANT vType,
            /* [in][optional] */ VARIANT var1,
            /* [out][retval] */ IXMLElement __RPC_FAR *__RPC_FAR *ppElem);
        
        END_INTERFACE
    } IXMLDocumentVtbl;

    interface IXMLDocument
    {
        CONST_VTBL struct IXMLDocumentVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDocument_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDocument_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDocument_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDocument_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDocument_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDocument_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDocument_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDocument_get_root(This,p)	\
    (This)->lpVtbl -> get_root(This,p)

#define IXMLDocument_get_fileSize(This,p)	\
    (This)->lpVtbl -> get_fileSize(This,p)

#define IXMLDocument_get_fileModifiedDate(This,p)	\
    (This)->lpVtbl -> get_fileModifiedDate(This,p)

#define IXMLDocument_get_fileUpdatedDate(This,p)	\
    (This)->lpVtbl -> get_fileUpdatedDate(This,p)

#define IXMLDocument_get_URL(This,p)	\
    (This)->lpVtbl -> get_URL(This,p)

#define IXMLDocument_put_URL(This,p)	\
    (This)->lpVtbl -> put_URL(This,p)

#define IXMLDocument_get_mimeType(This,p)	\
    (This)->lpVtbl -> get_mimeType(This,p)

#define IXMLDocument_get_readyState(This,pl)	\
    (This)->lpVtbl -> get_readyState(This,pl)

#define IXMLDocument_get_charset(This,p)	\
    (This)->lpVtbl -> get_charset(This,p)

#define IXMLDocument_put_charset(This,p)	\
    (This)->lpVtbl -> put_charset(This,p)

#define IXMLDocument_get_version(This,p)	\
    (This)->lpVtbl -> get_version(This,p)

#define IXMLDocument_get_doctype(This,p)	\
    (This)->lpVtbl -> get_doctype(This,p)

#define IXMLDocument_get_dtdURL(This,p)	\
    (This)->lpVtbl -> get_dtdURL(This,p)

#define IXMLDocument_createElement(This,vType,var1,ppElem)	\
    (This)->lpVtbl -> createElement(This,vType,var1,ppElem)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument_get_root_Proxy( 
    IXMLDocument __RPC_FAR * This,
    /* [out][retval] */ IXMLElement __RPC_FAR *__RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_root_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument_get_fileSize_Proxy( 
    IXMLDocument __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_fileSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument_get_fileModifiedDate_Proxy( 
    IXMLDocument __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_fileModifiedDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument_get_fileUpdatedDate_Proxy( 
    IXMLDocument __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_fileUpdatedDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument_get_URL_Proxy( 
    IXMLDocument __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_URL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDocument_put_URL_Proxy( 
    IXMLDocument __RPC_FAR * This,
    /* [in] */ BSTR p);


void __RPC_STUB IXMLDocument_put_URL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument_get_mimeType_Proxy( 
    IXMLDocument __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_mimeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument_get_readyState_Proxy( 
    IXMLDocument __RPC_FAR * This,
    /* [out][retval] */ long __RPC_FAR *pl);


void __RPC_STUB IXMLDocument_get_readyState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument_get_charset_Proxy( 
    IXMLDocument __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_charset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDocument_put_charset_Proxy( 
    IXMLDocument __RPC_FAR * This,
    /* [in] */ BSTR p);


void __RPC_STUB IXMLDocument_put_charset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument_get_version_Proxy( 
    IXMLDocument __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument_get_doctype_Proxy( 
    IXMLDocument __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_doctype_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument_get_dtdURL_Proxy( 
    IXMLDocument __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument_get_dtdURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDocument_createElement_Proxy( 
    IXMLDocument __RPC_FAR * This,
    /* [in] */ VARIANT vType,
    /* [in][optional] */ VARIANT var1,
    /* [out][retval] */ IXMLElement __RPC_FAR *__RPC_FAR *ppElem);


void __RPC_STUB IXMLDocument_createElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLDocument_INTERFACE_DEFINED__ */


#ifndef __IXMLDocument2_INTERFACE_DEFINED__
#define __IXMLDocument2_INTERFACE_DEFINED__

/* interface IXMLDocument2 */
/* [hidden][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLDocument2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2B8DE2FE-8D2D-11d1-B2FC-00C04FD915A9")
    IXMLDocument2 : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_root( 
            /* [out][retval] */ IXMLElement2 __RPC_FAR *__RPC_FAR *p) = 0;
        
        virtual /* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE get_fileSize( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE get_fileModifiedDate( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE get_fileUpdatedDate( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_URL( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_URL( 
            /* [in] */ BSTR p) = 0;
        
        virtual /* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE get_mimeType( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_readyState( 
            /* [out][retval] */ long __RPC_FAR *pl) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_charset( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_charset( 
            /* [in] */ BSTR p) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_version( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_doctype( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE get_dtdURL( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE createElement( 
            /* [in] */ VARIANT vType,
            /* [in][optional] */ VARIANT var1,
            /* [out][retval] */ IXMLElement2 __RPC_FAR *__RPC_FAR *ppElem) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_async( 
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *pf) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_async( 
            /* [in] */ VARIANT_BOOL f) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLDocument2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLDocument2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLDocument2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_root )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [out][retval] */ IXMLElement2 __RPC_FAR *__RPC_FAR *p);
        
        /* [id][hidden][restricted][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_fileSize )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [id][hidden][restricted][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_fileModifiedDate )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [id][hidden][restricted][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_fileUpdatedDate )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_URL )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_URL )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [in] */ BSTR p);
        
        /* [id][hidden][restricted][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_mimeType )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_readyState )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [out][retval] */ long __RPC_FAR *pl);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_charset )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_charset )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [in] */ BSTR p);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_version )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_doctype )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [id][hidden][restricted][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_dtdURL )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *createElement )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [in] */ VARIANT vType,
            /* [in][optional] */ VARIANT var1,
            /* [out][retval] */ IXMLElement2 __RPC_FAR *__RPC_FAR *ppElem);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_async )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [out][retval] */ VARIANT_BOOL __RPC_FAR *pf);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_async )( 
            IXMLDocument2 __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL f);
        
        END_INTERFACE
    } IXMLDocument2Vtbl;

    interface IXMLDocument2
    {
        CONST_VTBL struct IXMLDocument2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLDocument2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLDocument2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLDocument2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLDocument2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLDocument2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLDocument2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLDocument2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLDocument2_get_root(This,p)	\
    (This)->lpVtbl -> get_root(This,p)

#define IXMLDocument2_get_fileSize(This,p)	\
    (This)->lpVtbl -> get_fileSize(This,p)

#define IXMLDocument2_get_fileModifiedDate(This,p)	\
    (This)->lpVtbl -> get_fileModifiedDate(This,p)

#define IXMLDocument2_get_fileUpdatedDate(This,p)	\
    (This)->lpVtbl -> get_fileUpdatedDate(This,p)

#define IXMLDocument2_get_URL(This,p)	\
    (This)->lpVtbl -> get_URL(This,p)

#define IXMLDocument2_put_URL(This,p)	\
    (This)->lpVtbl -> put_URL(This,p)

#define IXMLDocument2_get_mimeType(This,p)	\
    (This)->lpVtbl -> get_mimeType(This,p)

#define IXMLDocument2_get_readyState(This,pl)	\
    (This)->lpVtbl -> get_readyState(This,pl)

#define IXMLDocument2_get_charset(This,p)	\
    (This)->lpVtbl -> get_charset(This,p)

#define IXMLDocument2_put_charset(This,p)	\
    (This)->lpVtbl -> put_charset(This,p)

#define IXMLDocument2_get_version(This,p)	\
    (This)->lpVtbl -> get_version(This,p)

#define IXMLDocument2_get_doctype(This,p)	\
    (This)->lpVtbl -> get_doctype(This,p)

#define IXMLDocument2_get_dtdURL(This,p)	\
    (This)->lpVtbl -> get_dtdURL(This,p)

#define IXMLDocument2_createElement(This,vType,var1,ppElem)	\
    (This)->lpVtbl -> createElement(This,vType,var1,ppElem)

#define IXMLDocument2_get_async(This,pf)	\
    (This)->lpVtbl -> get_async(This,pf)

#define IXMLDocument2_put_async(This,f)	\
    (This)->lpVtbl -> put_async(This,f)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument2_get_root_Proxy( 
    IXMLDocument2 __RPC_FAR * This,
    /* [out][retval] */ IXMLElement2 __RPC_FAR *__RPC_FAR *p);


void __RPC_STUB IXMLDocument2_get_root_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument2_get_fileSize_Proxy( 
    IXMLDocument2 __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument2_get_fileSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument2_get_fileModifiedDate_Proxy( 
    IXMLDocument2 __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument2_get_fileModifiedDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument2_get_fileUpdatedDate_Proxy( 
    IXMLDocument2 __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument2_get_fileUpdatedDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument2_get_URL_Proxy( 
    IXMLDocument2 __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument2_get_URL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDocument2_put_URL_Proxy( 
    IXMLDocument2 __RPC_FAR * This,
    /* [in] */ BSTR p);


void __RPC_STUB IXMLDocument2_put_URL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument2_get_mimeType_Proxy( 
    IXMLDocument2 __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument2_get_mimeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument2_get_readyState_Proxy( 
    IXMLDocument2 __RPC_FAR * This,
    /* [out][retval] */ long __RPC_FAR *pl);


void __RPC_STUB IXMLDocument2_get_readyState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument2_get_charset_Proxy( 
    IXMLDocument2 __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument2_get_charset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDocument2_put_charset_Proxy( 
    IXMLDocument2 __RPC_FAR * This,
    /* [in] */ BSTR p);


void __RPC_STUB IXMLDocument2_put_charset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument2_get_version_Proxy( 
    IXMLDocument2 __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument2_get_version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument2_get_doctype_Proxy( 
    IXMLDocument2 __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument2_get_doctype_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][hidden][restricted][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument2_get_dtdURL_Proxy( 
    IXMLDocument2 __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLDocument2_get_dtdURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLDocument2_createElement_Proxy( 
    IXMLDocument2 __RPC_FAR * This,
    /* [in] */ VARIANT vType,
    /* [in][optional] */ VARIANT var1,
    /* [out][retval] */ IXMLElement2 __RPC_FAR *__RPC_FAR *ppElem);


void __RPC_STUB IXMLDocument2_createElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLDocument2_get_async_Proxy( 
    IXMLDocument2 __RPC_FAR * This,
    /* [out][retval] */ VARIANT_BOOL __RPC_FAR *pf);


void __RPC_STUB IXMLDocument2_get_async_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLDocument2_put_async_Proxy( 
    IXMLDocument2 __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL f);


void __RPC_STUB IXMLDocument2_put_async_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLDocument2_INTERFACE_DEFINED__ */


#ifndef __IXMLElement_INTERFACE_DEFINED__
#define __IXMLElement_INTERFACE_DEFINED__

/* interface IXMLElement */
/* [helpstring][hidden][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3F7F31AC-E15F-11d0-9C25-00C04FC99C8E")
    IXMLElement : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_tagName( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_tagName( 
            /* [in] */ BSTR p) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_parent( 
            /* [out][retval] */ IXMLElement __RPC_FAR *__RPC_FAR *ppParent) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setAttribute( 
            /* [in] */ BSTR strPropertyName,
            /* [in] */ VARIANT PropertyValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getAttribute( 
            /* [in] */ BSTR strPropertyName,
            /* [out][retval] */ VARIANT __RPC_FAR *PropertyValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE removeAttribute( 
            /* [in] */ BSTR strPropertyName) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_children( 
            /* [out][retval] */ IXMLElementCollection __RPC_FAR *__RPC_FAR *pp) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_type( 
            /* [out][retval] */ long __RPC_FAR *plType) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_text( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_text( 
            /* [in] */ BSTR p) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE addChild( 
            /* [in] */ IXMLElement __RPC_FAR *pChildElem,
            long lIndex,
            long lReserved) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE removeChild( 
            /* [in] */ IXMLElement __RPC_FAR *pChildElem) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLElement __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLElement __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLElement __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLElement __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLElement __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLElement __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLElement __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_tagName )( 
            IXMLElement __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_tagName )( 
            IXMLElement __RPC_FAR * This,
            /* [in] */ BSTR p);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parent )( 
            IXMLElement __RPC_FAR * This,
            /* [out][retval] */ IXMLElement __RPC_FAR *__RPC_FAR *ppParent);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setAttribute )( 
            IXMLElement __RPC_FAR * This,
            /* [in] */ BSTR strPropertyName,
            /* [in] */ VARIANT PropertyValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getAttribute )( 
            IXMLElement __RPC_FAR * This,
            /* [in] */ BSTR strPropertyName,
            /* [out][retval] */ VARIANT __RPC_FAR *PropertyValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeAttribute )( 
            IXMLElement __RPC_FAR * This,
            /* [in] */ BSTR strPropertyName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_children )( 
            IXMLElement __RPC_FAR * This,
            /* [out][retval] */ IXMLElementCollection __RPC_FAR *__RPC_FAR *pp);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_type )( 
            IXMLElement __RPC_FAR * This,
            /* [out][retval] */ long __RPC_FAR *plType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXMLElement __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXMLElement __RPC_FAR * This,
            /* [in] */ BSTR p);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *addChild )( 
            IXMLElement __RPC_FAR * This,
            /* [in] */ IXMLElement __RPC_FAR *pChildElem,
            long lIndex,
            long lReserved);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXMLElement __RPC_FAR * This,
            /* [in] */ IXMLElement __RPC_FAR *pChildElem);
        
        END_INTERFACE
    } IXMLElementVtbl;

    interface IXMLElement
    {
        CONST_VTBL struct IXMLElementVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLElement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLElement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLElement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLElement_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLElement_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLElement_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLElement_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLElement_get_tagName(This,p)	\
    (This)->lpVtbl -> get_tagName(This,p)

#define IXMLElement_put_tagName(This,p)	\
    (This)->lpVtbl -> put_tagName(This,p)

#define IXMLElement_get_parent(This,ppParent)	\
    (This)->lpVtbl -> get_parent(This,ppParent)

#define IXMLElement_setAttribute(This,strPropertyName,PropertyValue)	\
    (This)->lpVtbl -> setAttribute(This,strPropertyName,PropertyValue)

#define IXMLElement_getAttribute(This,strPropertyName,PropertyValue)	\
    (This)->lpVtbl -> getAttribute(This,strPropertyName,PropertyValue)

#define IXMLElement_removeAttribute(This,strPropertyName)	\
    (This)->lpVtbl -> removeAttribute(This,strPropertyName)

#define IXMLElement_get_children(This,pp)	\
    (This)->lpVtbl -> get_children(This,pp)

#define IXMLElement_get_type(This,plType)	\
    (This)->lpVtbl -> get_type(This,plType)

#define IXMLElement_get_text(This,p)	\
    (This)->lpVtbl -> get_text(This,p)

#define IXMLElement_put_text(This,p)	\
    (This)->lpVtbl -> put_text(This,p)

#define IXMLElement_addChild(This,pChildElem,lIndex,lReserved)	\
    (This)->lpVtbl -> addChild(This,pChildElem,lIndex,lReserved)

#define IXMLElement_removeChild(This,pChildElem)	\
    (This)->lpVtbl -> removeChild(This,pChildElem)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLElement_get_tagName_Proxy( 
    IXMLElement __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLElement_get_tagName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLElement_put_tagName_Proxy( 
    IXMLElement __RPC_FAR * This,
    /* [in] */ BSTR p);


void __RPC_STUB IXMLElement_put_tagName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLElement_get_parent_Proxy( 
    IXMLElement __RPC_FAR * This,
    /* [out][retval] */ IXMLElement __RPC_FAR *__RPC_FAR *ppParent);


void __RPC_STUB IXMLElement_get_parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLElement_setAttribute_Proxy( 
    IXMLElement __RPC_FAR * This,
    /* [in] */ BSTR strPropertyName,
    /* [in] */ VARIANT PropertyValue);


void __RPC_STUB IXMLElement_setAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLElement_getAttribute_Proxy( 
    IXMLElement __RPC_FAR * This,
    /* [in] */ BSTR strPropertyName,
    /* [out][retval] */ VARIANT __RPC_FAR *PropertyValue);


void __RPC_STUB IXMLElement_getAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLElement_removeAttribute_Proxy( 
    IXMLElement __RPC_FAR * This,
    /* [in] */ BSTR strPropertyName);


void __RPC_STUB IXMLElement_removeAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLElement_get_children_Proxy( 
    IXMLElement __RPC_FAR * This,
    /* [out][retval] */ IXMLElementCollection __RPC_FAR *__RPC_FAR *pp);


void __RPC_STUB IXMLElement_get_children_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLElement_get_type_Proxy( 
    IXMLElement __RPC_FAR * This,
    /* [out][retval] */ long __RPC_FAR *plType);


void __RPC_STUB IXMLElement_get_type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLElement_get_text_Proxy( 
    IXMLElement __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLElement_get_text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLElement_put_text_Proxy( 
    IXMLElement __RPC_FAR * This,
    /* [in] */ BSTR p);


void __RPC_STUB IXMLElement_put_text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLElement_addChild_Proxy( 
    IXMLElement __RPC_FAR * This,
    /* [in] */ IXMLElement __RPC_FAR *pChildElem,
    long lIndex,
    long lReserved);


void __RPC_STUB IXMLElement_addChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLElement_removeChild_Proxy( 
    IXMLElement __RPC_FAR * This,
    /* [in] */ IXMLElement __RPC_FAR *pChildElem);


void __RPC_STUB IXMLElement_removeChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLElement_INTERFACE_DEFINED__ */


#ifndef __IXMLElement2_INTERFACE_DEFINED__
#define __IXMLElement2_INTERFACE_DEFINED__

/* interface IXMLElement2 */
/* [helpstring][hidden][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLElement2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2B8DE2FF-8D2D-11d1-B2FC-00C04FD915A9")
    IXMLElement2 : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_tagName( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_tagName( 
            /* [in] */ BSTR p) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_parent( 
            /* [out][retval] */ IXMLElement2 __RPC_FAR *__RPC_FAR *ppParent) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setAttribute( 
            /* [in] */ BSTR strPropertyName,
            /* [in] */ VARIANT PropertyValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getAttribute( 
            /* [in] */ BSTR strPropertyName,
            /* [out][retval] */ VARIANT __RPC_FAR *PropertyValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE removeAttribute( 
            /* [in] */ BSTR strPropertyName) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_children( 
            /* [out][retval] */ IXMLElementCollection __RPC_FAR *__RPC_FAR *pp) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_type( 
            /* [out][retval] */ long __RPC_FAR *plType) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_text( 
            /* [out][retval] */ BSTR __RPC_FAR *p) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_text( 
            /* [in] */ BSTR p) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE addChild( 
            /* [in] */ IXMLElement2 __RPC_FAR *pChildElem,
            long lIndex,
            long lReserved) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE removeChild( 
            /* [in] */ IXMLElement2 __RPC_FAR *pChildElem) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_attributes( 
            /* [out][retval] */ IXMLElementCollection __RPC_FAR *__RPC_FAR *pp) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLElement2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLElement2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLElement2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLElement2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLElement2 __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLElement2 __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLElement2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLElement2 __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_tagName )( 
            IXMLElement2 __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_tagName )( 
            IXMLElement2 __RPC_FAR * This,
            /* [in] */ BSTR p);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_parent )( 
            IXMLElement2 __RPC_FAR * This,
            /* [out][retval] */ IXMLElement2 __RPC_FAR *__RPC_FAR *ppParent);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setAttribute )( 
            IXMLElement2 __RPC_FAR * This,
            /* [in] */ BSTR strPropertyName,
            /* [in] */ VARIANT PropertyValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *getAttribute )( 
            IXMLElement2 __RPC_FAR * This,
            /* [in] */ BSTR strPropertyName,
            /* [out][retval] */ VARIANT __RPC_FAR *PropertyValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeAttribute )( 
            IXMLElement2 __RPC_FAR * This,
            /* [in] */ BSTR strPropertyName);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_children )( 
            IXMLElement2 __RPC_FAR * This,
            /* [out][retval] */ IXMLElementCollection __RPC_FAR *__RPC_FAR *pp);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_type )( 
            IXMLElement2 __RPC_FAR * This,
            /* [out][retval] */ long __RPC_FAR *plType);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_text )( 
            IXMLElement2 __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *p);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_text )( 
            IXMLElement2 __RPC_FAR * This,
            /* [in] */ BSTR p);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *addChild )( 
            IXMLElement2 __RPC_FAR * This,
            /* [in] */ IXMLElement2 __RPC_FAR *pChildElem,
            long lIndex,
            long lReserved);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *removeChild )( 
            IXMLElement2 __RPC_FAR * This,
            /* [in] */ IXMLElement2 __RPC_FAR *pChildElem);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_attributes )( 
            IXMLElement2 __RPC_FAR * This,
            /* [out][retval] */ IXMLElementCollection __RPC_FAR *__RPC_FAR *pp);
        
        END_INTERFACE
    } IXMLElement2Vtbl;

    interface IXMLElement2
    {
        CONST_VTBL struct IXMLElement2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLElement2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLElement2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLElement2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLElement2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLElement2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLElement2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLElement2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLElement2_get_tagName(This,p)	\
    (This)->lpVtbl -> get_tagName(This,p)

#define IXMLElement2_put_tagName(This,p)	\
    (This)->lpVtbl -> put_tagName(This,p)

#define IXMLElement2_get_parent(This,ppParent)	\
    (This)->lpVtbl -> get_parent(This,ppParent)

#define IXMLElement2_setAttribute(This,strPropertyName,PropertyValue)	\
    (This)->lpVtbl -> setAttribute(This,strPropertyName,PropertyValue)

#define IXMLElement2_getAttribute(This,strPropertyName,PropertyValue)	\
    (This)->lpVtbl -> getAttribute(This,strPropertyName,PropertyValue)

#define IXMLElement2_removeAttribute(This,strPropertyName)	\
    (This)->lpVtbl -> removeAttribute(This,strPropertyName)

#define IXMLElement2_get_children(This,pp)	\
    (This)->lpVtbl -> get_children(This,pp)

#define IXMLElement2_get_type(This,plType)	\
    (This)->lpVtbl -> get_type(This,plType)

#define IXMLElement2_get_text(This,p)	\
    (This)->lpVtbl -> get_text(This,p)

#define IXMLElement2_put_text(This,p)	\
    (This)->lpVtbl -> put_text(This,p)

#define IXMLElement2_addChild(This,pChildElem,lIndex,lReserved)	\
    (This)->lpVtbl -> addChild(This,pChildElem,lIndex,lReserved)

#define IXMLElement2_removeChild(This,pChildElem)	\
    (This)->lpVtbl -> removeChild(This,pChildElem)

#define IXMLElement2_get_attributes(This,pp)	\
    (This)->lpVtbl -> get_attributes(This,pp)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLElement2_get_tagName_Proxy( 
    IXMLElement2 __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLElement2_get_tagName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLElement2_put_tagName_Proxy( 
    IXMLElement2 __RPC_FAR * This,
    /* [in] */ BSTR p);


void __RPC_STUB IXMLElement2_put_tagName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLElement2_get_parent_Proxy( 
    IXMLElement2 __RPC_FAR * This,
    /* [out][retval] */ IXMLElement2 __RPC_FAR *__RPC_FAR *ppParent);


void __RPC_STUB IXMLElement2_get_parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLElement2_setAttribute_Proxy( 
    IXMLElement2 __RPC_FAR * This,
    /* [in] */ BSTR strPropertyName,
    /* [in] */ VARIANT PropertyValue);


void __RPC_STUB IXMLElement2_setAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLElement2_getAttribute_Proxy( 
    IXMLElement2 __RPC_FAR * This,
    /* [in] */ BSTR strPropertyName,
    /* [out][retval] */ VARIANT __RPC_FAR *PropertyValue);


void __RPC_STUB IXMLElement2_getAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLElement2_removeAttribute_Proxy( 
    IXMLElement2 __RPC_FAR * This,
    /* [in] */ BSTR strPropertyName);


void __RPC_STUB IXMLElement2_removeAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLElement2_get_children_Proxy( 
    IXMLElement2 __RPC_FAR * This,
    /* [out][retval] */ IXMLElementCollection __RPC_FAR *__RPC_FAR *pp);


void __RPC_STUB IXMLElement2_get_children_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLElement2_get_type_Proxy( 
    IXMLElement2 __RPC_FAR * This,
    /* [out][retval] */ long __RPC_FAR *plType);


void __RPC_STUB IXMLElement2_get_type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLElement2_get_text_Proxy( 
    IXMLElement2 __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *p);


void __RPC_STUB IXMLElement2_get_text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IXMLElement2_put_text_Proxy( 
    IXMLElement2 __RPC_FAR * This,
    /* [in] */ BSTR p);


void __RPC_STUB IXMLElement2_put_text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLElement2_addChild_Proxy( 
    IXMLElement2 __RPC_FAR * This,
    /* [in] */ IXMLElement2 __RPC_FAR *pChildElem,
    long lIndex,
    long lReserved);


void __RPC_STUB IXMLElement2_addChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IXMLElement2_removeChild_Proxy( 
    IXMLElement2 __RPC_FAR * This,
    /* [in] */ IXMLElement2 __RPC_FAR *pChildElem);


void __RPC_STUB IXMLElement2_removeChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLElement2_get_attributes_Proxy( 
    IXMLElement2 __RPC_FAR * This,
    /* [out][retval] */ IXMLElementCollection __RPC_FAR *__RPC_FAR *pp);


void __RPC_STUB IXMLElement2_get_attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLElement2_INTERFACE_DEFINED__ */


#ifndef __IXMLAttribute_INTERFACE_DEFINED__
#define __IXMLAttribute_INTERFACE_DEFINED__

/* interface IXMLAttribute */
/* [helpstring][hidden][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLAttribute;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D4D4A0FC-3B73-11d1-B2B4-00C04FB92596")
    IXMLAttribute : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_name( 
            /* [out][retval] */ BSTR __RPC_FAR *n) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_value( 
            /* [out][retval] */ BSTR __RPC_FAR *v) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLAttributeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLAttribute __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLAttribute __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLAttribute __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IXMLAttribute __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IXMLAttribute __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IXMLAttribute __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IXMLAttribute __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_name )( 
            IXMLAttribute __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *n);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_value )( 
            IXMLAttribute __RPC_FAR * This,
            /* [out][retval] */ BSTR __RPC_FAR *v);
        
        END_INTERFACE
    } IXMLAttributeVtbl;

    interface IXMLAttribute
    {
        CONST_VTBL struct IXMLAttributeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLAttribute_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLAttribute_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLAttribute_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLAttribute_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IXMLAttribute_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IXMLAttribute_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IXMLAttribute_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IXMLAttribute_get_name(This,n)	\
    (This)->lpVtbl -> get_name(This,n)

#define IXMLAttribute_get_value(This,v)	\
    (This)->lpVtbl -> get_value(This,v)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLAttribute_get_name_Proxy( 
    IXMLAttribute __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *n);


void __RPC_STUB IXMLAttribute_get_name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IXMLAttribute_get_value_Proxy( 
    IXMLAttribute __RPC_FAR * This,
    /* [out][retval] */ BSTR __RPC_FAR *v);


void __RPC_STUB IXMLAttribute_get_value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLAttribute_INTERFACE_DEFINED__ */


#ifndef __IXMLError_INTERFACE_DEFINED__
#define __IXMLError_INTERFACE_DEFINED__

/* interface IXMLError */
/* [helpstring][hidden][uuid][object][local] */ 


EXTERN_C const IID IID_IXMLError;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("948C5AD3-C58D-11d0-9C0B-00C04FC99C8E")
    IXMLError : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetErrorInfo( 
            XML_ERROR __RPC_FAR *pErrorReturn) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IXMLErrorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IXMLError __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IXMLError __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IXMLError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetErrorInfo )( 
            IXMLError __RPC_FAR * This,
            XML_ERROR __RPC_FAR *pErrorReturn);
        
        END_INTERFACE
    } IXMLErrorVtbl;

    interface IXMLError
    {
        CONST_VTBL struct IXMLErrorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IXMLError_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IXMLError_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IXMLError_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IXMLError_GetErrorInfo(This,pErrorReturn)	\
    (This)->lpVtbl -> GetErrorInfo(This,pErrorReturn)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IXMLError_GetErrorInfo_Proxy( 
    IXMLError __RPC_FAR * This,
    XML_ERROR __RPC_FAR *pErrorReturn);


void __RPC_STUB IXMLError_GetErrorInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IXMLError_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_XMLDocument;

#ifdef __cplusplus

class DECLSPEC_UUID("CFC399AF-D876-11d0-9C10-00C04FC99C8E")
XMLDocument;
#endif
#endif /* __MSXML_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



