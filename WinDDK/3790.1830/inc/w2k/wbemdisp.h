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
/* at Fri Nov 12 15:41:50 1999
 */
/* Compiler settings for wbemdisp.idl:
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

#ifndef __wbemdisp_h__
#define __wbemdisp_h__

/* Forward Declarations */ 

#ifndef __ISWbemServices_FWD_DEFINED__
#define __ISWbemServices_FWD_DEFINED__
typedef interface ISWbemServices ISWbemServices;
#endif 	/* __ISWbemServices_FWD_DEFINED__ */


#ifndef __ISWbemLocator_FWD_DEFINED__
#define __ISWbemLocator_FWD_DEFINED__
typedef interface ISWbemLocator ISWbemLocator;
#endif 	/* __ISWbemLocator_FWD_DEFINED__ */


#ifndef __ISWbemObject_FWD_DEFINED__
#define __ISWbemObject_FWD_DEFINED__
typedef interface ISWbemObject ISWbemObject;
#endif 	/* __ISWbemObject_FWD_DEFINED__ */


#ifndef __ISWbemObjectSet_FWD_DEFINED__
#define __ISWbemObjectSet_FWD_DEFINED__
typedef interface ISWbemObjectSet ISWbemObjectSet;
#endif 	/* __ISWbemObjectSet_FWD_DEFINED__ */


#ifndef __ISWbemNamedValue_FWD_DEFINED__
#define __ISWbemNamedValue_FWD_DEFINED__
typedef interface ISWbemNamedValue ISWbemNamedValue;
#endif 	/* __ISWbemNamedValue_FWD_DEFINED__ */


#ifndef __ISWbemNamedValueSet_FWD_DEFINED__
#define __ISWbemNamedValueSet_FWD_DEFINED__
typedef interface ISWbemNamedValueSet ISWbemNamedValueSet;
#endif 	/* __ISWbemNamedValueSet_FWD_DEFINED__ */


#ifndef __ISWbemQualifier_FWD_DEFINED__
#define __ISWbemQualifier_FWD_DEFINED__
typedef interface ISWbemQualifier ISWbemQualifier;
#endif 	/* __ISWbemQualifier_FWD_DEFINED__ */


#ifndef __ISWbemQualifierSet_FWD_DEFINED__
#define __ISWbemQualifierSet_FWD_DEFINED__
typedef interface ISWbemQualifierSet ISWbemQualifierSet;
#endif 	/* __ISWbemQualifierSet_FWD_DEFINED__ */


#ifndef __ISWbemProperty_FWD_DEFINED__
#define __ISWbemProperty_FWD_DEFINED__
typedef interface ISWbemProperty ISWbemProperty;
#endif 	/* __ISWbemProperty_FWD_DEFINED__ */


#ifndef __ISWbemPropertySet_FWD_DEFINED__
#define __ISWbemPropertySet_FWD_DEFINED__
typedef interface ISWbemPropertySet ISWbemPropertySet;
#endif 	/* __ISWbemPropertySet_FWD_DEFINED__ */


#ifndef __ISWbemMethod_FWD_DEFINED__
#define __ISWbemMethod_FWD_DEFINED__
typedef interface ISWbemMethod ISWbemMethod;
#endif 	/* __ISWbemMethod_FWD_DEFINED__ */


#ifndef __ISWbemMethodSet_FWD_DEFINED__
#define __ISWbemMethodSet_FWD_DEFINED__
typedef interface ISWbemMethodSet ISWbemMethodSet;
#endif 	/* __ISWbemMethodSet_FWD_DEFINED__ */


#ifndef __ISWbemEventSource_FWD_DEFINED__
#define __ISWbemEventSource_FWD_DEFINED__
typedef interface ISWbemEventSource ISWbemEventSource;
#endif 	/* __ISWbemEventSource_FWD_DEFINED__ */


#ifndef __ISWbemObjectPath_FWD_DEFINED__
#define __ISWbemObjectPath_FWD_DEFINED__
typedef interface ISWbemObjectPath ISWbemObjectPath;
#endif 	/* __ISWbemObjectPath_FWD_DEFINED__ */


#ifndef __ISWbemLastError_FWD_DEFINED__
#define __ISWbemLastError_FWD_DEFINED__
typedef interface ISWbemLastError ISWbemLastError;
#endif 	/* __ISWbemLastError_FWD_DEFINED__ */


#ifndef __ISWbemSinkEvents_FWD_DEFINED__
#define __ISWbemSinkEvents_FWD_DEFINED__
typedef interface ISWbemSinkEvents ISWbemSinkEvents;
#endif 	/* __ISWbemSinkEvents_FWD_DEFINED__ */


#ifndef __ISWbemSink_FWD_DEFINED__
#define __ISWbemSink_FWD_DEFINED__
typedef interface ISWbemSink ISWbemSink;
#endif 	/* __ISWbemSink_FWD_DEFINED__ */


#ifndef __ISWbemSecurity_FWD_DEFINED__
#define __ISWbemSecurity_FWD_DEFINED__
typedef interface ISWbemSecurity ISWbemSecurity;
#endif 	/* __ISWbemSecurity_FWD_DEFINED__ */


#ifndef __ISWbemPrivilege_FWD_DEFINED__
#define __ISWbemPrivilege_FWD_DEFINED__
typedef interface ISWbemPrivilege ISWbemPrivilege;
#endif 	/* __ISWbemPrivilege_FWD_DEFINED__ */


#ifndef __ISWbemPrivilegeSet_FWD_DEFINED__
#define __ISWbemPrivilegeSet_FWD_DEFINED__
typedef interface ISWbemPrivilegeSet ISWbemPrivilegeSet;
#endif 	/* __ISWbemPrivilegeSet_FWD_DEFINED__ */


#ifndef __SWbemLocator_FWD_DEFINED__
#define __SWbemLocator_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemLocator SWbemLocator;
#else
typedef struct SWbemLocator SWbemLocator;
#endif /* __cplusplus */

#endif 	/* __SWbemLocator_FWD_DEFINED__ */


#ifndef __SWbemNamedValueSet_FWD_DEFINED__
#define __SWbemNamedValueSet_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemNamedValueSet SWbemNamedValueSet;
#else
typedef struct SWbemNamedValueSet SWbemNamedValueSet;
#endif /* __cplusplus */

#endif 	/* __SWbemNamedValueSet_FWD_DEFINED__ */


#ifndef __SWbemObjectPath_FWD_DEFINED__
#define __SWbemObjectPath_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemObjectPath SWbemObjectPath;
#else
typedef struct SWbemObjectPath SWbemObjectPath;
#endif /* __cplusplus */

#endif 	/* __SWbemObjectPath_FWD_DEFINED__ */


#ifndef __SWbemLastError_FWD_DEFINED__
#define __SWbemLastError_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemLastError SWbemLastError;
#else
typedef struct SWbemLastError SWbemLastError;
#endif /* __cplusplus */

#endif 	/* __SWbemLastError_FWD_DEFINED__ */


#ifndef __SWbemSink_FWD_DEFINED__
#define __SWbemSink_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemSink SWbemSink;
#else
typedef struct SWbemSink SWbemSink;
#endif /* __cplusplus */

#endif 	/* __SWbemSink_FWD_DEFINED__ */


#ifndef __SWbemServices_FWD_DEFINED__
#define __SWbemServices_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemServices SWbemServices;
#else
typedef struct SWbemServices SWbemServices;
#endif /* __cplusplus */

#endif 	/* __SWbemServices_FWD_DEFINED__ */


#ifndef __SWbemObject_FWD_DEFINED__
#define __SWbemObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemObject SWbemObject;
#else
typedef struct SWbemObject SWbemObject;
#endif /* __cplusplus */

#endif 	/* __SWbemObject_FWD_DEFINED__ */


#ifndef __SWbemObjectSet_FWD_DEFINED__
#define __SWbemObjectSet_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemObjectSet SWbemObjectSet;
#else
typedef struct SWbemObjectSet SWbemObjectSet;
#endif /* __cplusplus */

#endif 	/* __SWbemObjectSet_FWD_DEFINED__ */


#ifndef __SWbemNamedValue_FWD_DEFINED__
#define __SWbemNamedValue_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemNamedValue SWbemNamedValue;
#else
typedef struct SWbemNamedValue SWbemNamedValue;
#endif /* __cplusplus */

#endif 	/* __SWbemNamedValue_FWD_DEFINED__ */


#ifndef __SWbemQualifier_FWD_DEFINED__
#define __SWbemQualifier_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemQualifier SWbemQualifier;
#else
typedef struct SWbemQualifier SWbemQualifier;
#endif /* __cplusplus */

#endif 	/* __SWbemQualifier_FWD_DEFINED__ */


#ifndef __SWbemQualifierSet_FWD_DEFINED__
#define __SWbemQualifierSet_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemQualifierSet SWbemQualifierSet;
#else
typedef struct SWbemQualifierSet SWbemQualifierSet;
#endif /* __cplusplus */

#endif 	/* __SWbemQualifierSet_FWD_DEFINED__ */


#ifndef __SWbemProperty_FWD_DEFINED__
#define __SWbemProperty_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemProperty SWbemProperty;
#else
typedef struct SWbemProperty SWbemProperty;
#endif /* __cplusplus */

#endif 	/* __SWbemProperty_FWD_DEFINED__ */


#ifndef __SWbemPropertySet_FWD_DEFINED__
#define __SWbemPropertySet_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemPropertySet SWbemPropertySet;
#else
typedef struct SWbemPropertySet SWbemPropertySet;
#endif /* __cplusplus */

#endif 	/* __SWbemPropertySet_FWD_DEFINED__ */


#ifndef __SWbemMethod_FWD_DEFINED__
#define __SWbemMethod_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemMethod SWbemMethod;
#else
typedef struct SWbemMethod SWbemMethod;
#endif /* __cplusplus */

#endif 	/* __SWbemMethod_FWD_DEFINED__ */


#ifndef __SWbemMethodSet_FWD_DEFINED__
#define __SWbemMethodSet_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemMethodSet SWbemMethodSet;
#else
typedef struct SWbemMethodSet SWbemMethodSet;
#endif /* __cplusplus */

#endif 	/* __SWbemMethodSet_FWD_DEFINED__ */


#ifndef __SWbemEventSource_FWD_DEFINED__
#define __SWbemEventSource_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemEventSource SWbemEventSource;
#else
typedef struct SWbemEventSource SWbemEventSource;
#endif /* __cplusplus */

#endif 	/* __SWbemEventSource_FWD_DEFINED__ */


#ifndef __SWbemSecurity_FWD_DEFINED__
#define __SWbemSecurity_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemSecurity SWbemSecurity;
#else
typedef struct SWbemSecurity SWbemSecurity;
#endif /* __cplusplus */

#endif 	/* __SWbemSecurity_FWD_DEFINED__ */


#ifndef __SWbemPrivilege_FWD_DEFINED__
#define __SWbemPrivilege_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemPrivilege SWbemPrivilege;
#else
typedef struct SWbemPrivilege SWbemPrivilege;
#endif /* __cplusplus */

#endif 	/* __SWbemPrivilege_FWD_DEFINED__ */


#ifndef __SWbemPrivilegeSet_FWD_DEFINED__
#define __SWbemPrivilegeSet_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWbemPrivilegeSet SWbemPrivilegeSet;
#else
typedef struct SWbemPrivilegeSet SWbemPrivilegeSet;
#endif /* __cplusplus */

#endif 	/* __SWbemPrivilegeSet_FWD_DEFINED__ */


#ifndef __ISWbemLocator_FWD_DEFINED__
#define __ISWbemLocator_FWD_DEFINED__
typedef interface ISWbemLocator ISWbemLocator;
#endif 	/* __ISWbemLocator_FWD_DEFINED__ */


#ifndef __ISWbemServices_FWD_DEFINED__
#define __ISWbemServices_FWD_DEFINED__
typedef interface ISWbemServices ISWbemServices;
#endif 	/* __ISWbemServices_FWD_DEFINED__ */


#ifndef __ISWbemObject_FWD_DEFINED__
#define __ISWbemObject_FWD_DEFINED__
typedef interface ISWbemObject ISWbemObject;
#endif 	/* __ISWbemObject_FWD_DEFINED__ */


#ifndef __ISWbemLastError_FWD_DEFINED__
#define __ISWbemLastError_FWD_DEFINED__
typedef interface ISWbemLastError ISWbemLastError;
#endif 	/* __ISWbemLastError_FWD_DEFINED__ */


#ifndef __ISWbemObjectSet_FWD_DEFINED__
#define __ISWbemObjectSet_FWD_DEFINED__
typedef interface ISWbemObjectSet ISWbemObjectSet;
#endif 	/* __ISWbemObjectSet_FWD_DEFINED__ */


#ifndef __ISWbemNamedValueSet_FWD_DEFINED__
#define __ISWbemNamedValueSet_FWD_DEFINED__
typedef interface ISWbemNamedValueSet ISWbemNamedValueSet;
#endif 	/* __ISWbemNamedValueSet_FWD_DEFINED__ */


#ifndef __ISWbemNamedValue_FWD_DEFINED__
#define __ISWbemNamedValue_FWD_DEFINED__
typedef interface ISWbemNamedValue ISWbemNamedValue;
#endif 	/* __ISWbemNamedValue_FWD_DEFINED__ */


#ifndef __ISWbemObjectPath_FWD_DEFINED__
#define __ISWbemObjectPath_FWD_DEFINED__
typedef interface ISWbemObjectPath ISWbemObjectPath;
#endif 	/* __ISWbemObjectPath_FWD_DEFINED__ */


#ifndef __ISWbemProperty_FWD_DEFINED__
#define __ISWbemProperty_FWD_DEFINED__
typedef interface ISWbemProperty ISWbemProperty;
#endif 	/* __ISWbemProperty_FWD_DEFINED__ */


#ifndef __ISWbemPropertySet_FWD_DEFINED__
#define __ISWbemPropertySet_FWD_DEFINED__
typedef interface ISWbemPropertySet ISWbemPropertySet;
#endif 	/* __ISWbemPropertySet_FWD_DEFINED__ */


#ifndef __ISWbemQualifier_FWD_DEFINED__
#define __ISWbemQualifier_FWD_DEFINED__
typedef interface ISWbemQualifier ISWbemQualifier;
#endif 	/* __ISWbemQualifier_FWD_DEFINED__ */


#ifndef __ISWbemQualifierSet_FWD_DEFINED__
#define __ISWbemQualifierSet_FWD_DEFINED__
typedef interface ISWbemQualifierSet ISWbemQualifierSet;
#endif 	/* __ISWbemQualifierSet_FWD_DEFINED__ */


#ifndef __ISWbemMethod_FWD_DEFINED__
#define __ISWbemMethod_FWD_DEFINED__
typedef interface ISWbemMethod ISWbemMethod;
#endif 	/* __ISWbemMethod_FWD_DEFINED__ */


#ifndef __ISWbemMethodSet_FWD_DEFINED__
#define __ISWbemMethodSet_FWD_DEFINED__
typedef interface ISWbemMethodSet ISWbemMethodSet;
#endif 	/* __ISWbemMethodSet_FWD_DEFINED__ */


#ifndef __ISWbemSink_FWD_DEFINED__
#define __ISWbemSink_FWD_DEFINED__
typedef interface ISWbemSink ISWbemSink;
#endif 	/* __ISWbemSink_FWD_DEFINED__ */


#ifndef __ISWbemSinkEvents_FWD_DEFINED__
#define __ISWbemSinkEvents_FWD_DEFINED__
typedef interface ISWbemSinkEvents ISWbemSinkEvents;
#endif 	/* __ISWbemSinkEvents_FWD_DEFINED__ */


#ifndef __ISWbemEventSource_FWD_DEFINED__
#define __ISWbemEventSource_FWD_DEFINED__
typedef interface ISWbemEventSource ISWbemEventSource;
#endif 	/* __ISWbemEventSource_FWD_DEFINED__ */


#ifndef __ISWbemSecurity_FWD_DEFINED__
#define __ISWbemSecurity_FWD_DEFINED__
typedef interface ISWbemSecurity ISWbemSecurity;
#endif 	/* __ISWbemSecurity_FWD_DEFINED__ */


#ifndef __ISWbemPrivilege_FWD_DEFINED__
#define __ISWbemPrivilege_FWD_DEFINED__
typedef interface ISWbemPrivilege ISWbemPrivilege;
#endif 	/* __ISWbemPrivilege_FWD_DEFINED__ */


#ifndef __ISWbemPrivilegeSet_FWD_DEFINED__
#define __ISWbemPrivilegeSet_FWD_DEFINED__
typedef interface ISWbemPrivilegeSet ISWbemPrivilegeSet;
#endif 	/* __ISWbemPrivilegeSet_FWD_DEFINED__ */


/* header files for imported files */
#include "dispex.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __WbemScripting_LIBRARY_DEFINED__
#define __WbemScripting_LIBRARY_DEFINED__

/* library WbemScripting */
/* [helpstring][version][lcid][uuid] */ 





















typedef /* [helpstring][uuid][v1_enum] */  DECLSPEC_UUID("4A249B72-FC9A-11d1-8B1E-00600806D9B6") 
enum WbemChangeFlagEnum
    {	wbemChangeFlagCreateOrUpdate	= 0,
	wbemChangeFlagUpdateOnly	= 0x1,
	wbemChangeFlagCreateOnly	= 0x2,
	wbemChangeFlagUpdateCompatible	= 0,
	wbemChangeFlagUpdateSafeMode	= 0x20,
	wbemChangeFlagUpdateForceMode	= 0x40
    }	WbemChangeFlagEnum;

typedef /* [helpstring][uuid][v1_enum] */  DECLSPEC_UUID("4A249B73-FC9A-11d1-8B1E-00600806D9B6") 
enum WbemFlagEnum
    {	wbemFlagReturnImmediately	= 0x10,
	wbemFlagReturnWhenComplete	= 0,
	wbemFlagBidirectional	= 0,
	wbemFlagForwardOnly	= 0x20,
	wbemFlagNoErrorObject	= 0x40,
	wbemFlagReturnErrorObject	= 0,
	wbemFlagSendStatus	= 0x80,
	wbemFlagDontSendStatus	= 0,
	wbemFlagUseAmendedQualifiers	= 0x20000
    }	WbemFlagEnum;

typedef /* [helpstring][uuid][v1_enum] */  DECLSPEC_UUID("4A249B76-FC9A-11d1-8B1E-00600806D9B6") 
enum WbemQueryFlagEnum
    {	wbemQueryFlagDeep	= 0,
	wbemQueryFlagShallow	= 1,
	wbemQueryFlagPrototype	= 2
    }	WbemQueryFlagEnum;

typedef /* [helpstring][uuid][v1_enum] */  DECLSPEC_UUID("4A249B78-FC9A-11d1-8B1E-00600806D9B6") 
enum WbemTextFlagEnum
    {	wbemTextFlagNoFlavors	= 0x1
    }	WbemTextFlagEnum;

typedef /* [helpstring][uuid][v1_enum] */  DECLSPEC_UUID("BF078C2A-07D9-11d2-8B21-00600806D9B6") 
enum WbemTimeout
    {	wbemTimeoutInfinite	= 0xffffffff
    }	WbemTimeout;

typedef /* [helpstring][uuid][v1_enum] */  DECLSPEC_UUID("4A249B79-FC9A-11d1-8B1E-00600806D9B6") 
enum WbemComparisonFlagEnum
    {	wbemComparisonFlagIncludeAll	= 0,
	wbemComparisonFlagIgnoreQualifiers	= 0x1,
	wbemComparisonFlagIgnoreObjectSource	= 0x2,
	wbemComparisonFlagIgnoreDefaultValues	= 0x4,
	wbemComparisonFlagIgnoreClass	= 0x8,
	wbemComparisonFlagIgnoreCase	= 0x10,
	wbemComparisonFlagIgnoreFlavor	= 0x20
    }	WbemComparisonFlagEnum;

typedef /* [helpstring][uuid][v1_enum] */  DECLSPEC_UUID("4A249B7B-FC9A-11d1-8B1E-00600806D9B6") 
enum WbemCimtypeEnum
    {	wbemCimtypeSint8	= 16,
	wbemCimtypeUint8	= 17,
	wbemCimtypeSint16	= 2,
	wbemCimtypeUint16	= 18,
	wbemCimtypeSint32	= 3,
	wbemCimtypeUint32	= 19,
	wbemCimtypeSint64	= 20,
	wbemCimtypeUint64	= 21,
	wbemCimtypeReal32	= 4,
	wbemCimtypeReal64	= 5,
	wbemCimtypeBoolean	= 11,
	wbemCimtypeString	= 8,
	wbemCimtypeDatetime	= 101,
	wbemCimtypeReference	= 102,
	wbemCimtypeChar16	= 103,
	wbemCimtypeObject	= 13
    }	WbemCimtypeEnum;

typedef /* [helpstring][uuid][v1_enum] */  DECLSPEC_UUID("4A249B7C-FC9A-11d1-8B1E-00600806D9B6") 
enum WbemErrorEnum
    {	wbemNoErr	= 0,
	wbemErrFailed	= 0x80041001,
	wbemErrNotFound	= 0x80041002,
	wbemErrAccessDenied	= 0x80041003,
	wbemErrProviderFailure	= 0x80041004,
	wbemErrTypeMismatch	= 0x80041005,
	wbemErrOutOfMemory	= 0x80041006,
	wbemErrInvalidContext	= 0x80041007,
	wbemErrInvalidParameter	= 0x80041008,
	wbemErrNotAvailable	= 0x80041009,
	wbemErrCriticalError	= 0x8004100a,
	wbemErrInvalidStream	= 0x8004100b,
	wbemErrNotSupported	= 0x8004100c,
	wbemErrInvalidSuperclass	= 0x8004100d,
	wbemErrInvalidNamespace	= 0x8004100e,
	wbemErrInvalidObject	= 0x8004100f,
	wbemErrInvalidClass	= 0x80041010,
	wbemErrProviderNotFound	= 0x80041011,
	wbemErrInvalidProviderRegistration	= 0x80041012,
	wbemErrProviderLoadFailure	= 0x80041013,
	wbemErrInitializationFailure	= 0x80041014,
	wbemErrTransportFailure	= 0x80041015,
	wbemErrInvalidOperation	= 0x80041016,
	wbemErrInvalidQuery	= 0x80041017,
	wbemErrInvalidQueryType	= 0x80041018,
	wbemErrAlreadyExists	= 0x80041019,
	wbemErrOverrideNotAllowed	= 0x8004101a,
	wbemErrPropagatedQualifier	= 0x8004101b,
	wbemErrPropagatedProperty	= 0x8004101c,
	wbemErrUnexpected	= 0x8004101d,
	wbemErrIllegalOperation	= 0x8004101e,
	wbemErrCannotBeKey	= 0x8004101f,
	wbemErrIncompleteClass	= 0x80041020,
	wbemErrInvalidSyntax	= 0x80041021,
	wbemErrNondecoratedObject	= 0x80041022,
	wbemErrReadOnly	= 0x80041023,
	wbemErrProviderNotCapable	= 0x80041024,
	wbemErrClassHasChildren	= 0x80041025,
	wbemErrClassHasInstances	= 0x80041026,
	wbemErrQueryNotImplemented	= 0x80041027,
	wbemErrIllegalNull	= 0x80041028,
	wbemErrInvalidQualifierType	= 0x80041029,
	wbemErrInvalidPropertyType	= 0x8004102a,
	wbemErrValueOutOfRange	= 0x8004102b,
	wbemErrCannotBeSingleton	= 0x8004102c,
	wbemErrInvalidCimType	= 0x8004102d,
	wbemErrInvalidMethod	= 0x8004102e,
	wbemErrInvalidMethodParameters	= 0x8004102f,
	wbemErrSystemProperty	= 0x80041030,
	wbemErrInvalidProperty	= 0x80041031,
	wbemErrCallCancelled	= 0x80041032,
	wbemErrShuttingDown	= 0x80041033,
	wbemErrPropagatedMethod	= 0x80041034,
	wbemErrUnsupportedParameter	= 0x80041035,
	wbemErrMissingParameter	= 0x80041036,
	wbemErrInvalidParameterId	= 0x80041037,
	wbemErrNonConsecutiveParameterIds	= 0x80041038,
	wbemErrParameterIdOnRetval	= 0x80041039,
	wbemErrInvalidObjectPath	= 0x8004103a,
	wbemErrOutOfDiskSpace	= 0x8004103b,
	wbemErrBufferTooSmall	= 0x8004103c,
	wbemErrUnsupportedPutExtension	= 0x8004103d,
	wbemErrUnknownObjectType	= 0x8004103e,
	wbemErrUnknownPacketType	= 0x8004103f,
	wbemErrMarshalVersionMismatch	= 0x80041040,
	wbemErrMarshalInvalidSignature	= 0x80041041,
	wbemErrInvalidQualifier	= 0x80041042,
	wbemErrInvalidDuplicateParameter	= 0x80041043,
	wbemErrTooMuchData	= 0x80041044,
	wbemErrServerTooBusy	= 0x80041045,
	wbemErrInvalidFlavor	= 0x80041046,
	wbemErrCircularReference	= 0x80041047,
	wbemErrUnsupportedClassUpdate	= 0x80041048,
	wbemErrCannotChangeKeyInheritance	= 0x80041049,
	wbemErrCannotChangeIndexInheritance	= 0x80041050,
	wbemErrTooManyProperties	= 0x80041051,
	wbemErrUpdateTypeMismatch	= 0x80041052,
	wbemErrUpdateOverrideNotAllowed	= 0x80041053,
	wbemErrUpdatePropagatedMethod	= 0x80041054,
	wbemErrMethodNotImplemented	= 0x80041055,
	wbemErrMethodDisabled	= 0x80041056,
	wbemErrRefresherBusy	= 0x80041057,
	wbemErrUnparsableQuery	= 0x80041058,
	wbemErrNotEventClass	= 0x80041059,
	wbemErrMissingGroupWithin	= 0x8004105a,
	wbemErrMissingAggregationList	= 0x8004105b,
	wbemErrPropertyNotAnObject	= 0x8004105c,
	wbemErrAggregatingByObject	= 0x8004105d,
	wbemErrUninterpretableProviderQuery	= 0x8004105f,
	wbemErrBackupRestoreWinmgmtRunning	= 0x80041060,
	wbemErrQueueOverflow	= 0x80041061,
	wbemErrPrivilegeNotHeld	= 0x80041062,
	wbemErrInvalidOperator	= 0x80041063,
	wbemErrLocalCredentials	= 0x80041064,
	wbemErrCannotBeAbstract	= 0x80041065,
	wbemErrAmendedObject	= 0x80041066,
	wbemErrRegistrationTooBroad	= 0x80042001,
	wbemErrRegistrationTooPrecise	= 0x80042002,
	wbemErrTimedout	= 0x80043001,
	wbemErrResetToDefault	= 0x80043002
    }	WbemErrorEnum;

typedef /* [helpstring][uuid][v1_enum] */  DECLSPEC_UUID("B54D66E7-2287-11d2-8B33-00600806D9B6") 
enum WbemAuthenticationLevelEnum
    {	wbemAuthenticationLevelDefault	= 0,
	wbemAuthenticationLevelNone	= 1,
	wbemAuthenticationLevelConnect	= 2,
	wbemAuthenticationLevelCall	= 3,
	wbemAuthenticationLevelPkt	= 4,
	wbemAuthenticationLevelPktIntegrity	= 5,
	wbemAuthenticationLevelPktPrivacy	= 6
    }	WbemAuthenticationLevelEnum;

typedef /* [helpstring][uuid][v1_enum] */  DECLSPEC_UUID("B54D66E8-2287-11d2-8B33-00600806D9B6") 
enum WbemImpersonationLevelEnum
    {	wbemImpersonationLevelAnonymous	= 1,
	wbemImpersonationLevelIdentify	= 2,
	wbemImpersonationLevelImpersonate	= 3,
	wbemImpersonationLevelDelegate	= 4
    }	WbemImpersonationLevelEnum;

typedef /* [helpstring][uuid][v1_enum] */  DECLSPEC_UUID("176D2F70-5AF3-11d2-8B4A-00600806D9B6") 
enum WbemPrivilegeEnum
    {	wbemPrivilegeCreateToken	= 1,
	wbemPrivilegePrimaryToken	= 2,
	wbemPrivilegeLockMemory	= 3,
	wbemPrivilegeIncreaseQuota	= 4,
	wbemPrivilegeMachineAccount	= 5,
	wbemPrivilegeTcb	= 6,
	wbemPrivilegeSecurity	= 7,
	wbemPrivilegeTakeOwnership	= 8,
	wbemPrivilegeLoadDriver	= 9,
	wbemPrivilegeSystemProfile	= 10,
	wbemPrivilegeSystemtime	= 11,
	wbemPrivilegeProfileSingleProcess	= 12,
	wbemPrivilegeIncreaseBasePriority	= 13,
	wbemPrivilegeCreatePagefile	= 14,
	wbemPrivilegeCreatePermanent	= 15,
	wbemPrivilegeBackup	= 16,
	wbemPrivilegeRestore	= 17,
	wbemPrivilegeShutdown	= 18,
	wbemPrivilegeDebug	= 19,
	wbemPrivilegeAudit	= 20,
	wbemPrivilegeSystemEnvironment	= 21,
	wbemPrivilegeChangeNotify	= 22,
	wbemPrivilegeRemoteShutdown	= 23,
	wbemPrivilegeUndock	= 24,
	wbemPrivilegeSyncAgent	= 25,
	wbemPrivilegeEnableDelegation	= 26
    }	WbemPrivilegeEnum;


EXTERN_C const IID LIBID_WbemScripting;

#ifndef __ISWbemServices_INTERFACE_DEFINED__
#define __ISWbemServices_INTERFACE_DEFINED__

/* interface ISWbemServices */
/* [helpstring][hidden][unique][nonextensible][dual][oleautomation][uuid][object][local] */ 


EXTERN_C const IID IID_ISWbemServices;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("76A6415C-CB41-11d1-8B02-00600806D9B6")
    ISWbemServices : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Get( 
            /* [defaultvalue][optional][in] */ BSTR strObjectPath,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAsync( 
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ BSTR strObjectPath = L"",
            /* [defaultvalue][optional][in] */ long iFlags = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Delete( 
            /* [in] */ BSTR strObjectPath,
            /* [defaultvalue][optional][in] */ long iFlags = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DeleteAsync( 
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [in] */ BSTR strObjectPath,
            /* [defaultvalue][optional][in] */ long iFlags = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE InstancesOf( 
            /* [in] */ BSTR strClass,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE InstancesOfAsync( 
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [in] */ BSTR strClass,
            /* [defaultvalue][optional][in] */ long iFlags = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SubclassesOf( 
            /* [defaultvalue][optional][in] */ BSTR strSuperclass,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SubclassesOfAsync( 
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ BSTR strSuperclass = L"",
            /* [defaultvalue][optional][in] */ long iFlags = wbemQueryFlagDeep,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ExecQuery( 
            /* [in] */ BSTR strQuery,
            /* [defaultvalue][optional][in] */ BSTR strQueryLanguage,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ExecQueryAsync( 
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [in] */ BSTR strQuery,
            /* [defaultvalue][optional][in] */ BSTR strQueryLanguage = L"WQL",
            /* [defaultvalue][optional][in] */ long lFlags = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AssociatorsOf( 
            /* [in] */ BSTR strObjectPath,
            /* [defaultvalue][optional][in] */ BSTR strAssocClass,
            /* [defaultvalue][optional][in] */ BSTR strResultClass,
            /* [defaultvalue][optional][in] */ BSTR strResultRole,
            /* [defaultvalue][optional][in] */ BSTR strRole,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
            /* [defaultvalue][optional][in] */ BSTR strRequiredAssocQualifier,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AssociatorsOfAsync( 
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [in] */ BSTR strObjectPath,
            /* [defaultvalue][optional][in] */ BSTR strAssocClass = L"",
            /* [defaultvalue][optional][in] */ BSTR strResultClass = L"",
            /* [defaultvalue][optional][in] */ BSTR strResultRole = L"",
            /* [defaultvalue][optional][in] */ BSTR strRole = L"",
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly = FALSE,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly = FALSE,
            /* [defaultvalue][optional][in] */ BSTR strRequiredAssocQualifier = L"",
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier = L"",
            /* [defaultvalue][optional][in] */ long iFlags = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReferencesTo( 
            /* [in] */ BSTR strObjectPath,
            /* [defaultvalue][optional][in] */ BSTR strResultClass,
            /* [defaultvalue][optional][in] */ BSTR strRole,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReferencesToAsync( 
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [in] */ BSTR strObjectPath,
            /* [defaultvalue][optional][in] */ BSTR strResultClass = L"",
            /* [defaultvalue][optional][in] */ BSTR strRole = L"",
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly = FALSE,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly = FALSE,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier = L"",
            /* [defaultvalue][optional][in] */ long iFlags = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ExecNotificationQuery( 
            /* [in] */ BSTR strQuery,
            /* [defaultvalue][optional][in] */ BSTR strQueryLanguage,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemEventSource __RPC_FAR *__RPC_FAR *objWbemEventSource) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ExecNotificationQueryAsync( 
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [in] */ BSTR strQuery,
            /* [defaultvalue][optional][in] */ BSTR strQueryLanguage = L"WQL",
            /* [defaultvalue][optional][in] */ long iFlags = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ExecMethod( 
            /* [in] */ BSTR strObjectPath,
            /* [in] */ BSTR strMethodName,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemInParameters,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemOutParameters) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ExecMethodAsync( 
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [in] */ BSTR strObjectPath,
            /* [in] */ BSTR strMethodName,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemInParameters = 0,
            /* [defaultvalue][optional][in] */ long iFlags = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext = 0) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Security_( 
            /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemServicesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemServices __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemServices __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemServices __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Get )( 
            ISWbemServices __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR strObjectPath,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAsync )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ BSTR strObjectPath,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Delete )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ BSTR strObjectPath,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteAsync )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [in] */ BSTR strObjectPath,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InstancesOf )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ BSTR strClass,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InstancesOfAsync )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [in] */ BSTR strClass,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SubclassesOf )( 
            ISWbemServices __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR strSuperclass,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SubclassesOfAsync )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ BSTR strSuperclass,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExecQuery )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ BSTR strQuery,
            /* [defaultvalue][optional][in] */ BSTR strQueryLanguage,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExecQueryAsync )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [in] */ BSTR strQuery,
            /* [defaultvalue][optional][in] */ BSTR strQueryLanguage,
            /* [defaultvalue][optional][in] */ long lFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AssociatorsOf )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ BSTR strObjectPath,
            /* [defaultvalue][optional][in] */ BSTR strAssocClass,
            /* [defaultvalue][optional][in] */ BSTR strResultClass,
            /* [defaultvalue][optional][in] */ BSTR strResultRole,
            /* [defaultvalue][optional][in] */ BSTR strRole,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
            /* [defaultvalue][optional][in] */ BSTR strRequiredAssocQualifier,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AssociatorsOfAsync )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [in] */ BSTR strObjectPath,
            /* [defaultvalue][optional][in] */ BSTR strAssocClass,
            /* [defaultvalue][optional][in] */ BSTR strResultClass,
            /* [defaultvalue][optional][in] */ BSTR strResultRole,
            /* [defaultvalue][optional][in] */ BSTR strRole,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
            /* [defaultvalue][optional][in] */ BSTR strRequiredAssocQualifier,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReferencesTo )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ BSTR strObjectPath,
            /* [defaultvalue][optional][in] */ BSTR strResultClass,
            /* [defaultvalue][optional][in] */ BSTR strRole,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReferencesToAsync )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [in] */ BSTR strObjectPath,
            /* [defaultvalue][optional][in] */ BSTR strResultClass,
            /* [defaultvalue][optional][in] */ BSTR strRole,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExecNotificationQuery )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ BSTR strQuery,
            /* [defaultvalue][optional][in] */ BSTR strQueryLanguage,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemEventSource __RPC_FAR *__RPC_FAR *objWbemEventSource);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExecNotificationQueryAsync )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [in] */ BSTR strQuery,
            /* [defaultvalue][optional][in] */ BSTR strQueryLanguage,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExecMethod )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ BSTR strObjectPath,
            /* [in] */ BSTR strMethodName,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemInParameters,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemOutParameters);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExecMethodAsync )( 
            ISWbemServices __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [in] */ BSTR strObjectPath,
            /* [in] */ BSTR strMethodName,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemInParameters,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Security_ )( 
            ISWbemServices __RPC_FAR * This,
            /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity);
        
        END_INTERFACE
    } ISWbemServicesVtbl;

    interface ISWbemServices
    {
        CONST_VTBL struct ISWbemServicesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemServices_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemServices_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemServices_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemServices_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemServices_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemServices_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemServices_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemServices_Get(This,strObjectPath,iFlags,objWbemNamedValueSet,objWbemObject)	\
    (This)->lpVtbl -> Get(This,strObjectPath,iFlags,objWbemNamedValueSet,objWbemObject)

#define ISWbemServices_GetAsync(This,objWbemSink,strObjectPath,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> GetAsync(This,objWbemSink,strObjectPath,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_Delete(This,strObjectPath,iFlags,objWbemNamedValueSet)	\
    (This)->lpVtbl -> Delete(This,strObjectPath,iFlags,objWbemNamedValueSet)

#define ISWbemServices_DeleteAsync(This,objWbemSink,strObjectPath,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> DeleteAsync(This,objWbemSink,strObjectPath,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_InstancesOf(This,strClass,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> InstancesOf(This,strClass,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemServices_InstancesOfAsync(This,objWbemSink,strClass,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> InstancesOfAsync(This,objWbemSink,strClass,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_SubclassesOf(This,strSuperclass,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> SubclassesOf(This,strSuperclass,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemServices_SubclassesOfAsync(This,objWbemSink,strSuperclass,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> SubclassesOfAsync(This,objWbemSink,strSuperclass,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_ExecQuery(This,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> ExecQuery(This,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemServices_ExecQueryAsync(This,objWbemSink,strQuery,strQueryLanguage,lFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ExecQueryAsync(This,objWbemSink,strQuery,strQueryLanguage,lFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_AssociatorsOf(This,strObjectPath,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> AssociatorsOf(This,strObjectPath,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemServices_AssociatorsOfAsync(This,objWbemSink,strObjectPath,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> AssociatorsOfAsync(This,objWbemSink,strObjectPath,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_ReferencesTo(This,strObjectPath,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> ReferencesTo(This,strObjectPath,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemServices_ReferencesToAsync(This,objWbemSink,strObjectPath,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ReferencesToAsync(This,objWbemSink,strObjectPath,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_ExecNotificationQuery(This,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemEventSource)	\
    (This)->lpVtbl -> ExecNotificationQuery(This,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemEventSource)

#define ISWbemServices_ExecNotificationQueryAsync(This,objWbemSink,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ExecNotificationQueryAsync(This,objWbemSink,strQuery,strQueryLanguage,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_ExecMethod(This,strObjectPath,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemOutParameters)	\
    (This)->lpVtbl -> ExecMethod(This,strObjectPath,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemOutParameters)

#define ISWbemServices_ExecMethodAsync(This,objWbemSink,strObjectPath,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ExecMethodAsync(This,objWbemSink,strObjectPath,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemServices_get_Security_(This,objWbemSecurity)	\
    (This)->lpVtbl -> get_Security_(This,objWbemSecurity)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_Get_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ BSTR strObjectPath,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject);


void __RPC_STUB ISWbemServices_Get_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_GetAsync_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *objWbemSink,
    /* [defaultvalue][optional][in] */ BSTR strObjectPath,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_GetAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_Delete_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [in] */ BSTR strObjectPath,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet);


void __RPC_STUB ISWbemServices_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_DeleteAsync_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *objWbemSink,
    /* [in] */ BSTR strObjectPath,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_DeleteAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_InstancesOf_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [in] */ BSTR strClass,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);


void __RPC_STUB ISWbemServices_InstancesOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_InstancesOfAsync_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *objWbemSink,
    /* [in] */ BSTR strClass,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_InstancesOfAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_SubclassesOf_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ BSTR strSuperclass,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);


void __RPC_STUB ISWbemServices_SubclassesOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_SubclassesOfAsync_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *objWbemSink,
    /* [defaultvalue][optional][in] */ BSTR strSuperclass,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_SubclassesOfAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_ExecQuery_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [in] */ BSTR strQuery,
    /* [defaultvalue][optional][in] */ BSTR strQueryLanguage,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);


void __RPC_STUB ISWbemServices_ExecQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_ExecQueryAsync_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *objWbemSink,
    /* [in] */ BSTR strQuery,
    /* [defaultvalue][optional][in] */ BSTR strQueryLanguage,
    /* [defaultvalue][optional][in] */ long lFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_ExecQueryAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_AssociatorsOf_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [in] */ BSTR strObjectPath,
    /* [defaultvalue][optional][in] */ BSTR strAssocClass,
    /* [defaultvalue][optional][in] */ BSTR strResultClass,
    /* [defaultvalue][optional][in] */ BSTR strResultRole,
    /* [defaultvalue][optional][in] */ BSTR strRole,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
    /* [defaultvalue][optional][in] */ BSTR strRequiredAssocQualifier,
    /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);


void __RPC_STUB ISWbemServices_AssociatorsOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_AssociatorsOfAsync_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *objWbemSink,
    /* [in] */ BSTR strObjectPath,
    /* [defaultvalue][optional][in] */ BSTR strAssocClass,
    /* [defaultvalue][optional][in] */ BSTR strResultClass,
    /* [defaultvalue][optional][in] */ BSTR strResultRole,
    /* [defaultvalue][optional][in] */ BSTR strRole,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
    /* [defaultvalue][optional][in] */ BSTR strRequiredAssocQualifier,
    /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_AssociatorsOfAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_ReferencesTo_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [in] */ BSTR strObjectPath,
    /* [defaultvalue][optional][in] */ BSTR strResultClass,
    /* [defaultvalue][optional][in] */ BSTR strRole,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
    /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);


void __RPC_STUB ISWbemServices_ReferencesTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_ReferencesToAsync_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *objWbemSink,
    /* [in] */ BSTR strObjectPath,
    /* [defaultvalue][optional][in] */ BSTR strResultClass,
    /* [defaultvalue][optional][in] */ BSTR strRole,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
    /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_ReferencesToAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_ExecNotificationQuery_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [in] */ BSTR strQuery,
    /* [defaultvalue][optional][in] */ BSTR strQueryLanguage,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [retval][out] */ ISWbemEventSource __RPC_FAR *__RPC_FAR *objWbemEventSource);


void __RPC_STUB ISWbemServices_ExecNotificationQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_ExecNotificationQueryAsync_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *objWbemSink,
    /* [in] */ BSTR strQuery,
    /* [defaultvalue][optional][in] */ BSTR strQueryLanguage,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_ExecNotificationQueryAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_ExecMethod_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [in] */ BSTR strObjectPath,
    /* [in] */ BSTR strMethodName,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemInParameters,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemOutParameters);


void __RPC_STUB ISWbemServices_ExecMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_ExecMethodAsync_Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *objWbemSink,
    /* [in] */ BSTR strObjectPath,
    /* [in] */ BSTR strMethodName,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemInParameters,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);


void __RPC_STUB ISWbemServices_ExecMethodAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemServices_get_Security__Proxy( 
    ISWbemServices __RPC_FAR * This,
    /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity);


void __RPC_STUB ISWbemServices_get_Security__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemServices_INTERFACE_DEFINED__ */


#ifndef __ISWbemLocator_INTERFACE_DEFINED__
#define __ISWbemLocator_INTERFACE_DEFINED__

/* interface ISWbemLocator */
/* [helpstring][unique][oleautomation][nonextensible][hidden][dual][uuid][local][object] */ 


EXTERN_C const IID IID_ISWbemLocator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("76A6415B-CB41-11d1-8B02-00600806D9B6")
    ISWbemLocator : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ConnectServer( 
            /* [defaultvalue][optional][in] */ BSTR strServer,
            /* [defaultvalue][optional][in] */ BSTR strNamespace,
            /* [defaultvalue][optional][in] */ BSTR strUser,
            /* [defaultvalue][optional][in] */ BSTR strPassword,
            /* [defaultvalue][optional][in] */ BSTR strLocale,
            /* [defaultvalue][optional][in] */ BSTR strAuthority,
            /* [defaultvalue][optional][in] */ long iSecurityFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemServices __RPC_FAR *__RPC_FAR *objWbemServices) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Security_( 
            /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemLocatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemLocator __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemLocator __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemLocator __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemLocator __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemLocator __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemLocator __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemLocator __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ConnectServer )( 
            ISWbemLocator __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR strServer,
            /* [defaultvalue][optional][in] */ BSTR strNamespace,
            /* [defaultvalue][optional][in] */ BSTR strUser,
            /* [defaultvalue][optional][in] */ BSTR strPassword,
            /* [defaultvalue][optional][in] */ BSTR strLocale,
            /* [defaultvalue][optional][in] */ BSTR strAuthority,
            /* [defaultvalue][optional][in] */ long iSecurityFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemServices __RPC_FAR *__RPC_FAR *objWbemServices);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Security_ )( 
            ISWbemLocator __RPC_FAR * This,
            /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity);
        
        END_INTERFACE
    } ISWbemLocatorVtbl;

    interface ISWbemLocator
    {
        CONST_VTBL struct ISWbemLocatorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemLocator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemLocator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemLocator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemLocator_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemLocator_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemLocator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemLocator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemLocator_ConnectServer(This,strServer,strNamespace,strUser,strPassword,strLocale,strAuthority,iSecurityFlags,objWbemNamedValueSet,objWbemServices)	\
    (This)->lpVtbl -> ConnectServer(This,strServer,strNamespace,strUser,strPassword,strLocale,strAuthority,iSecurityFlags,objWbemNamedValueSet,objWbemServices)

#define ISWbemLocator_get_Security_(This,objWbemSecurity)	\
    (This)->lpVtbl -> get_Security_(This,objWbemSecurity)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemLocator_ConnectServer_Proxy( 
    ISWbemLocator __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ BSTR strServer,
    /* [defaultvalue][optional][in] */ BSTR strNamespace,
    /* [defaultvalue][optional][in] */ BSTR strUser,
    /* [defaultvalue][optional][in] */ BSTR strPassword,
    /* [defaultvalue][optional][in] */ BSTR strLocale,
    /* [defaultvalue][optional][in] */ BSTR strAuthority,
    /* [defaultvalue][optional][in] */ long iSecurityFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [retval][out] */ ISWbemServices __RPC_FAR *__RPC_FAR *objWbemServices);


void __RPC_STUB ISWbemLocator_ConnectServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemLocator_get_Security__Proxy( 
    ISWbemLocator __RPC_FAR * This,
    /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity);


void __RPC_STUB ISWbemLocator_get_Security__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemLocator_INTERFACE_DEFINED__ */


#ifndef __ISWbemObject_INTERFACE_DEFINED__
#define __ISWbemObject_INTERFACE_DEFINED__

/* interface ISWbemObject */
/* [helpstring][hidden][oleautomation][dual][uuid][object][local] */ 

#define	WBEMS_DISPID_DERIVATION	( 23 )


EXTERN_C const IID IID_ISWbemObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("76A6415A-CB41-11d1-8B02-00600806D9B6")
    ISWbemObject : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Put_( 
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectPath __RPC_FAR *__RPC_FAR *objWbemObjectPath) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PutAsync_( 
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ long iFlags = wbemChangeFlagCreateOrUpdate,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Delete_( 
            /* [defaultvalue][optional][in] */ long iFlags = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DeleteAsync_( 
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ long iFlags = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Instances_( 
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE InstancesAsync_( 
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ long iFlags = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Subclasses_( 
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SubclassesAsync_( 
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ long iFlags = wbemQueryFlagDeep,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Associators_( 
            /* [defaultvalue][optional][in] */ BSTR strAssocClass,
            /* [defaultvalue][optional][in] */ BSTR strResultClass,
            /* [defaultvalue][optional][in] */ BSTR strResultRole,
            /* [defaultvalue][optional][in] */ BSTR strRole,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
            /* [defaultvalue][optional][in] */ BSTR strRequiredAssocQualifier,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AssociatorsAsync_( 
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ BSTR strAssocClass = L"",
            /* [defaultvalue][optional][in] */ BSTR strResultClass = L"",
            /* [defaultvalue][optional][in] */ BSTR strResultRole = L"",
            /* [defaultvalue][optional][in] */ BSTR strRole = L"",
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly = FALSE,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly = FALSE,
            /* [defaultvalue][optional][in] */ BSTR strRequiredAssocQualifier = L"",
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier = L"",
            /* [defaultvalue][optional][in] */ long iFlags = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE References_( 
            /* [defaultvalue][optional][in] */ BSTR strResultClass,
            /* [defaultvalue][optional][in] */ BSTR strRole,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReferencesAsync_( 
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ BSTR strResultClass = L"",
            /* [defaultvalue][optional][in] */ BSTR strRole = L"",
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly = FALSE,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly = FALSE,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier = L"",
            /* [defaultvalue][optional][in] */ long iFlags = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ExecMethod_( 
            /* [in] */ BSTR strMethodName,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemInParameters,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemOutParameters) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ExecMethodAsync_( 
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [in] */ BSTR strMethodName,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemInParameters = 0,
            /* [defaultvalue][optional][in] */ long iFlags = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet = 0,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clone_( 
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetObjectText_( 
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ BSTR __RPC_FAR *strObjectText) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SpawnDerivedClass_( 
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SpawnInstance_( 
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CompareTo_( 
            /* [in] */ IDispatch __RPC_FAR *objWbemObject,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bResult) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Qualifiers_( 
            /* [retval][out] */ ISWbemQualifierSet __RPC_FAR *__RPC_FAR *objWbemQualifierSet) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Properties_( 
            /* [retval][out] */ ISWbemPropertySet __RPC_FAR *__RPC_FAR *objWbemPropertySet) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Methods_( 
            /* [retval][out] */ ISWbemMethodSet __RPC_FAR *__RPC_FAR *objWbemMethodSet) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Derivation_( 
            /* [retval][out] */ VARIANT __RPC_FAR *strClassNameArray) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Path_( 
            /* [retval][out] */ ISWbemObjectPath __RPC_FAR *__RPC_FAR *objWbemObjectPath) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Security_( 
            /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemObject __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemObject __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemObject __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemObject __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemObject __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemObject __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemObject __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Put_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectPath __RPC_FAR *__RPC_FAR *objWbemObjectPath);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PutAsync_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Delete_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteAsync_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Instances_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InstancesAsync_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Subclasses_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SubclassesAsync_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Associators_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR strAssocClass,
            /* [defaultvalue][optional][in] */ BSTR strResultClass,
            /* [defaultvalue][optional][in] */ BSTR strResultRole,
            /* [defaultvalue][optional][in] */ BSTR strRole,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
            /* [defaultvalue][optional][in] */ BSTR strRequiredAssocQualifier,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AssociatorsAsync_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ BSTR strAssocClass,
            /* [defaultvalue][optional][in] */ BSTR strResultClass,
            /* [defaultvalue][optional][in] */ BSTR strResultRole,
            /* [defaultvalue][optional][in] */ BSTR strRole,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
            /* [defaultvalue][optional][in] */ BSTR strRequiredAssocQualifier,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *References_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR strResultClass,
            /* [defaultvalue][optional][in] */ BSTR strRole,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReferencesAsync_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ BSTR strResultClass,
            /* [defaultvalue][optional][in] */ BSTR strRole,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExecMethod_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [in] */ BSTR strMethodName,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemInParameters,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemOutParameters);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExecMethodAsync_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [in] */ BSTR strMethodName,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemInParameters,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetObjectText_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ BSTR __RPC_FAR *strObjectText);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SpawnDerivedClass_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SpawnInstance_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CompareTo_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemObject,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bResult);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Qualifiers_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [retval][out] */ ISWbemQualifierSet __RPC_FAR *__RPC_FAR *objWbemQualifierSet);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Properties_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [retval][out] */ ISWbemPropertySet __RPC_FAR *__RPC_FAR *objWbemPropertySet);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Methods_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [retval][out] */ ISWbemMethodSet __RPC_FAR *__RPC_FAR *objWbemMethodSet);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Derivation_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *strClassNameArray);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Path_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [retval][out] */ ISWbemObjectPath __RPC_FAR *__RPC_FAR *objWbemObjectPath);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Security_ )( 
            ISWbemObject __RPC_FAR * This,
            /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity);
        
        END_INTERFACE
    } ISWbemObjectVtbl;

    interface ISWbemObject
    {
        CONST_VTBL struct ISWbemObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemObject_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemObject_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemObject_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemObject_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemObject_Put_(This,iFlags,objWbemNamedValueSet,objWbemObjectPath)	\
    (This)->lpVtbl -> Put_(This,iFlags,objWbemNamedValueSet,objWbemObjectPath)

#define ISWbemObject_PutAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> PutAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObject_Delete_(This,iFlags,objWbemNamedValueSet)	\
    (This)->lpVtbl -> Delete_(This,iFlags,objWbemNamedValueSet)

#define ISWbemObject_DeleteAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> DeleteAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObject_Instances_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> Instances_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemObject_InstancesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> InstancesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObject_Subclasses_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> Subclasses_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemObject_SubclassesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> SubclassesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObject_Associators_(This,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> Associators_(This,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemObject_AssociatorsAsync_(This,objWbemSink,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> AssociatorsAsync_(This,objWbemSink,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObject_References_(This,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> References_(This,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemObject_ReferencesAsync_(This,objWbemSink,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ReferencesAsync_(This,objWbemSink,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObject_ExecMethod_(This,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemOutParameters)	\
    (This)->lpVtbl -> ExecMethod_(This,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemOutParameters)

#define ISWbemObject_ExecMethodAsync_(This,objWbemSink,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ExecMethodAsync_(This,objWbemSink,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemObject_Clone_(This,objWbemObject)	\
    (This)->lpVtbl -> Clone_(This,objWbemObject)

#define ISWbemObject_GetObjectText_(This,iFlags,strObjectText)	\
    (This)->lpVtbl -> GetObjectText_(This,iFlags,strObjectText)

#define ISWbemObject_SpawnDerivedClass_(This,iFlags,objWbemObject)	\
    (This)->lpVtbl -> SpawnDerivedClass_(This,iFlags,objWbemObject)

#define ISWbemObject_SpawnInstance_(This,iFlags,objWbemObject)	\
    (This)->lpVtbl -> SpawnInstance_(This,iFlags,objWbemObject)

#define ISWbemObject_CompareTo_(This,objWbemObject,iFlags,bResult)	\
    (This)->lpVtbl -> CompareTo_(This,objWbemObject,iFlags,bResult)

#define ISWbemObject_get_Qualifiers_(This,objWbemQualifierSet)	\
    (This)->lpVtbl -> get_Qualifiers_(This,objWbemQualifierSet)

#define ISWbemObject_get_Properties_(This,objWbemPropertySet)	\
    (This)->lpVtbl -> get_Properties_(This,objWbemPropertySet)

#define ISWbemObject_get_Methods_(This,objWbemMethodSet)	\
    (This)->lpVtbl -> get_Methods_(This,objWbemMethodSet)

#define ISWbemObject_get_Derivation_(This,strClassNameArray)	\
    (This)->lpVtbl -> get_Derivation_(This,strClassNameArray)

#define ISWbemObject_get_Path_(This,objWbemObjectPath)	\
    (This)->lpVtbl -> get_Path_(This,objWbemObjectPath)

#define ISWbemObject_get_Security_(This,objWbemSecurity)	\
    (This)->lpVtbl -> get_Security_(This,objWbemSecurity)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_Put__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [retval][out] */ ISWbemObjectPath __RPC_FAR *__RPC_FAR *objWbemObjectPath);


void __RPC_STUB ISWbemObject_Put__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_PutAsync__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *objWbemSink,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);


void __RPC_STUB ISWbemObject_PutAsync__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_Delete__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet);


void __RPC_STUB ISWbemObject_Delete__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_DeleteAsync__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *objWbemSink,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);


void __RPC_STUB ISWbemObject_DeleteAsync__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_Instances__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);


void __RPC_STUB ISWbemObject_Instances__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_InstancesAsync__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *objWbemSink,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);


void __RPC_STUB ISWbemObject_InstancesAsync__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_Subclasses__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);


void __RPC_STUB ISWbemObject_Subclasses__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_SubclassesAsync__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *objWbemSink,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);


void __RPC_STUB ISWbemObject_SubclassesAsync__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_Associators__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ BSTR strAssocClass,
    /* [defaultvalue][optional][in] */ BSTR strResultClass,
    /* [defaultvalue][optional][in] */ BSTR strResultRole,
    /* [defaultvalue][optional][in] */ BSTR strRole,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
    /* [defaultvalue][optional][in] */ BSTR strRequiredAssocQualifier,
    /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);


void __RPC_STUB ISWbemObject_Associators__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_AssociatorsAsync__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *objWbemSink,
    /* [defaultvalue][optional][in] */ BSTR strAssocClass,
    /* [defaultvalue][optional][in] */ BSTR strResultClass,
    /* [defaultvalue][optional][in] */ BSTR strResultRole,
    /* [defaultvalue][optional][in] */ BSTR strRole,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
    /* [defaultvalue][optional][in] */ BSTR strRequiredAssocQualifier,
    /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);


void __RPC_STUB ISWbemObject_AssociatorsAsync__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_References__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ BSTR strResultClass,
    /* [defaultvalue][optional][in] */ BSTR strRole,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
    /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);


void __RPC_STUB ISWbemObject_References__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_ReferencesAsync__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *objWbemSink,
    /* [defaultvalue][optional][in] */ BSTR strResultClass,
    /* [defaultvalue][optional][in] */ BSTR strRole,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
    /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);


void __RPC_STUB ISWbemObject_ReferencesAsync__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_ExecMethod__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [in] */ BSTR strMethodName,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemInParameters,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemOutParameters);


void __RPC_STUB ISWbemObject_ExecMethod__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_ExecMethodAsync__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *objWbemSink,
    /* [in] */ BSTR strMethodName,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemInParameters,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
    /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);


void __RPC_STUB ISWbemObject_ExecMethodAsync__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_Clone__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject);


void __RPC_STUB ISWbemObject_Clone__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_GetObjectText__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [retval][out] */ BSTR __RPC_FAR *strObjectText);


void __RPC_STUB ISWbemObject_GetObjectText__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_SpawnDerivedClass__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject);


void __RPC_STUB ISWbemObject_SpawnDerivedClass__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_SpawnInstance__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject);


void __RPC_STUB ISWbemObject_SpawnInstance__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_CompareTo__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [in] */ IDispatch __RPC_FAR *objWbemObject,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bResult);


void __RPC_STUB ISWbemObject_CompareTo__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_get_Qualifiers__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [retval][out] */ ISWbemQualifierSet __RPC_FAR *__RPC_FAR *objWbemQualifierSet);


void __RPC_STUB ISWbemObject_get_Qualifiers__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_get_Properties__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [retval][out] */ ISWbemPropertySet __RPC_FAR *__RPC_FAR *objWbemPropertySet);


void __RPC_STUB ISWbemObject_get_Properties__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_get_Methods__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [retval][out] */ ISWbemMethodSet __RPC_FAR *__RPC_FAR *objWbemMethodSet);


void __RPC_STUB ISWbemObject_get_Methods__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_get_Derivation__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *strClassNameArray);


void __RPC_STUB ISWbemObject_get_Derivation__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_get_Path__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [retval][out] */ ISWbemObjectPath __RPC_FAR *__RPC_FAR *objWbemObjectPath);


void __RPC_STUB ISWbemObject_get_Path__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObject_get_Security__Proxy( 
    ISWbemObject __RPC_FAR * This,
    /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity);


void __RPC_STUB ISWbemObject_get_Security__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemObject_INTERFACE_DEFINED__ */


#ifndef __ISWbemObjectSet_INTERFACE_DEFINED__
#define __ISWbemObjectSet_INTERFACE_DEFINED__

/* interface ISWbemObjectSet */
/* [helpstring][nonextensible][hidden][dual][oleautomation][uuid][object][local] */ 


EXTERN_C const IID IID_ISWbemObjectSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("76A6415F-CB41-11d1-8B02-00600806D9B6")
    ISWbemObjectSet : public IDispatch
    {
    public:
        virtual /* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Item( 
            /* [in] */ BSTR strObjectPath,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long __RPC_FAR *iCount) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Security_( 
            /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemObjectSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemObjectSet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemObjectSet __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemObjectSet __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemObjectSet __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemObjectSet __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemObjectSet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemObjectSet __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [restricted][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            ISWbemObjectSet __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )( 
            ISWbemObjectSet __RPC_FAR * This,
            /* [in] */ BSTR strObjectPath,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            ISWbemObjectSet __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *iCount);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Security_ )( 
            ISWbemObjectSet __RPC_FAR * This,
            /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity);
        
        END_INTERFACE
    } ISWbemObjectSetVtbl;

    interface ISWbemObjectSet
    {
        CONST_VTBL struct ISWbemObjectSetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemObjectSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemObjectSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemObjectSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemObjectSet_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemObjectSet_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemObjectSet_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemObjectSet_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemObjectSet_get__NewEnum(This,pUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,pUnk)

#define ISWbemObjectSet_Item(This,strObjectPath,iFlags,objWbemObject)	\
    (This)->lpVtbl -> Item(This,strObjectPath,iFlags,objWbemObject)

#define ISWbemObjectSet_get_Count(This,iCount)	\
    (This)->lpVtbl -> get_Count(This,iCount)

#define ISWbemObjectSet_get_Security_(This,objWbemSecurity)	\
    (This)->lpVtbl -> get_Security_(This,objWbemSecurity)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectSet_get__NewEnum_Proxy( 
    ISWbemObjectSet __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk);


void __RPC_STUB ISWbemObjectSet_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectSet_Item_Proxy( 
    ISWbemObjectSet __RPC_FAR * This,
    /* [in] */ BSTR strObjectPath,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject);


void __RPC_STUB ISWbemObjectSet_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectSet_get_Count_Proxy( 
    ISWbemObjectSet __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *iCount);


void __RPC_STUB ISWbemObjectSet_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectSet_get_Security__Proxy( 
    ISWbemObjectSet __RPC_FAR * This,
    /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity);


void __RPC_STUB ISWbemObjectSet_get_Security__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemObjectSet_INTERFACE_DEFINED__ */


#ifndef __ISWbemNamedValue_INTERFACE_DEFINED__
#define __ISWbemNamedValue_INTERFACE_DEFINED__

/* interface ISWbemNamedValue */
/* [helpstring][nonextensible][hidden][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_ISWbemNamedValue;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("76A64164-CB41-11d1-8B02-00600806D9B6")
    ISWbemNamedValue : public IDispatch
    {
    public:
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Value( 
            /* [retval][out] */ VARIANT __RPC_FAR *varValue) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Value( 
            /* [in] */ VARIANT __RPC_FAR *varValue) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR __RPC_FAR *strName) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemNamedValueVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemNamedValue __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemNamedValue __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemNamedValue __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemNamedValue __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemNamedValue __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemNamedValue __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemNamedValue __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Value )( 
            ISWbemNamedValue __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *varValue);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Value )( 
            ISWbemNamedValue __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *varValue);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            ISWbemNamedValue __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *strName);
        
        END_INTERFACE
    } ISWbemNamedValueVtbl;

    interface ISWbemNamedValue
    {
        CONST_VTBL struct ISWbemNamedValueVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemNamedValue_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemNamedValue_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemNamedValue_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemNamedValue_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemNamedValue_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemNamedValue_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemNamedValue_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemNamedValue_get_Value(This,varValue)	\
    (This)->lpVtbl -> get_Value(This,varValue)

#define ISWbemNamedValue_put_Value(This,varValue)	\
    (This)->lpVtbl -> put_Value(This,varValue)

#define ISWbemNamedValue_get_Name(This,strName)	\
    (This)->lpVtbl -> get_Name(This,strName)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemNamedValue_get_Value_Proxy( 
    ISWbemNamedValue __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *varValue);


void __RPC_STUB ISWbemNamedValue_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISWbemNamedValue_put_Value_Proxy( 
    ISWbemNamedValue __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *varValue);


void __RPC_STUB ISWbemNamedValue_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemNamedValue_get_Name_Proxy( 
    ISWbemNamedValue __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *strName);


void __RPC_STUB ISWbemNamedValue_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemNamedValue_INTERFACE_DEFINED__ */


#ifndef __ISWbemNamedValueSet_INTERFACE_DEFINED__
#define __ISWbemNamedValueSet_INTERFACE_DEFINED__

/* interface ISWbemNamedValueSet */
/* [helpstring][nonextensible][hidden][dual][oleautomation][uuid][object][local] */ 


EXTERN_C const IID IID_ISWbemNamedValueSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CF2376EA-CE8C-11d1-8B05-00600806D9B6")
    ISWbemNamedValueSet : public IDispatch
    {
    public:
        virtual /* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Item( 
            /* [in] */ BSTR strName,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemNamedValue __RPC_FAR *__RPC_FAR *objWbemNamedValue) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long __RPC_FAR *iCount) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Add( 
            /* [in] */ BSTR strName,
            /* [in] */ VARIANT __RPC_FAR *varValue,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemNamedValue __RPC_FAR *__RPC_FAR *objWbemNamedValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( 
            /* [in] */ BSTR strName,
            /* [defaultvalue][optional][in] */ long iFlags = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ ISWbemNamedValueSet __RPC_FAR *__RPC_FAR *objWbemNamedValueSet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DeleteAll( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemNamedValueSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemNamedValueSet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemNamedValueSet __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemNamedValueSet __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemNamedValueSet __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemNamedValueSet __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemNamedValueSet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemNamedValueSet __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [restricted][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            ISWbemNamedValueSet __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )( 
            ISWbemNamedValueSet __RPC_FAR * This,
            /* [in] */ BSTR strName,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemNamedValue __RPC_FAR *__RPC_FAR *objWbemNamedValue);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            ISWbemNamedValueSet __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *iCount);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Add )( 
            ISWbemNamedValueSet __RPC_FAR * This,
            /* [in] */ BSTR strName,
            /* [in] */ VARIANT __RPC_FAR *varValue,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemNamedValue __RPC_FAR *__RPC_FAR *objWbemNamedValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Remove )( 
            ISWbemNamedValueSet __RPC_FAR * This,
            /* [in] */ BSTR strName,
            /* [defaultvalue][optional][in] */ long iFlags);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ISWbemNamedValueSet __RPC_FAR * This,
            /* [retval][out] */ ISWbemNamedValueSet __RPC_FAR *__RPC_FAR *objWbemNamedValueSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteAll )( 
            ISWbemNamedValueSet __RPC_FAR * This);
        
        END_INTERFACE
    } ISWbemNamedValueSetVtbl;

    interface ISWbemNamedValueSet
    {
        CONST_VTBL struct ISWbemNamedValueSetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemNamedValueSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemNamedValueSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemNamedValueSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemNamedValueSet_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemNamedValueSet_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemNamedValueSet_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemNamedValueSet_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemNamedValueSet_get__NewEnum(This,pUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,pUnk)

#define ISWbemNamedValueSet_Item(This,strName,iFlags,objWbemNamedValue)	\
    (This)->lpVtbl -> Item(This,strName,iFlags,objWbemNamedValue)

#define ISWbemNamedValueSet_get_Count(This,iCount)	\
    (This)->lpVtbl -> get_Count(This,iCount)

#define ISWbemNamedValueSet_Add(This,strName,varValue,iFlags,objWbemNamedValue)	\
    (This)->lpVtbl -> Add(This,strName,varValue,iFlags,objWbemNamedValue)

#define ISWbemNamedValueSet_Remove(This,strName,iFlags)	\
    (This)->lpVtbl -> Remove(This,strName,iFlags)

#define ISWbemNamedValueSet_Clone(This,objWbemNamedValueSet)	\
    (This)->lpVtbl -> Clone(This,objWbemNamedValueSet)

#define ISWbemNamedValueSet_DeleteAll(This)	\
    (This)->lpVtbl -> DeleteAll(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemNamedValueSet_get__NewEnum_Proxy( 
    ISWbemNamedValueSet __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk);


void __RPC_STUB ISWbemNamedValueSet_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemNamedValueSet_Item_Proxy( 
    ISWbemNamedValueSet __RPC_FAR * This,
    /* [in] */ BSTR strName,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [retval][out] */ ISWbemNamedValue __RPC_FAR *__RPC_FAR *objWbemNamedValue);


void __RPC_STUB ISWbemNamedValueSet_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemNamedValueSet_get_Count_Proxy( 
    ISWbemNamedValueSet __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *iCount);


void __RPC_STUB ISWbemNamedValueSet_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemNamedValueSet_Add_Proxy( 
    ISWbemNamedValueSet __RPC_FAR * This,
    /* [in] */ BSTR strName,
    /* [in] */ VARIANT __RPC_FAR *varValue,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [retval][out] */ ISWbemNamedValue __RPC_FAR *__RPC_FAR *objWbemNamedValue);


void __RPC_STUB ISWbemNamedValueSet_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemNamedValueSet_Remove_Proxy( 
    ISWbemNamedValueSet __RPC_FAR * This,
    /* [in] */ BSTR strName,
    /* [defaultvalue][optional][in] */ long iFlags);


void __RPC_STUB ISWbemNamedValueSet_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemNamedValueSet_Clone_Proxy( 
    ISWbemNamedValueSet __RPC_FAR * This,
    /* [retval][out] */ ISWbemNamedValueSet __RPC_FAR *__RPC_FAR *objWbemNamedValueSet);


void __RPC_STUB ISWbemNamedValueSet_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemNamedValueSet_DeleteAll_Proxy( 
    ISWbemNamedValueSet __RPC_FAR * This);


void __RPC_STUB ISWbemNamedValueSet_DeleteAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemNamedValueSet_INTERFACE_DEFINED__ */


#ifndef __ISWbemQualifier_INTERFACE_DEFINED__
#define __ISWbemQualifier_INTERFACE_DEFINED__

/* interface ISWbemQualifier */
/* [helpstring][unique][nonextensible][hidden][oleautomation][dual][uuid][local][object] */ 


EXTERN_C const IID IID_ISWbemQualifier;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79B05932-D3B7-11d1-8B06-00600806D9B6")
    ISWbemQualifier : public IDispatch
    {
    public:
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Value( 
            /* [retval][out] */ VARIANT __RPC_FAR *varValue) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Value( 
            /* [in] */ VARIANT __RPC_FAR *varValue) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR __RPC_FAR *strName) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_IsLocal( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsLocal) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_PropagatesToSubclass( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bPropagatesToSubclass) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_PropagatesToSubclass( 
            /* [in] */ VARIANT_BOOL bPropagatesToSubclass) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_PropagatesToInstance( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bPropagatesToInstance) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_PropagatesToInstance( 
            /* [in] */ VARIANT_BOOL bPropagatesToInstance) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_IsOverridable( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsOverridable) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_IsOverridable( 
            /* [in] */ VARIANT_BOOL bIsOverridable) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_IsAmended( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsAmended) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemQualifierVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemQualifier __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemQualifier __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemQualifier __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemQualifier __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemQualifier __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemQualifier __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemQualifier __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Value )( 
            ISWbemQualifier __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *varValue);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Value )( 
            ISWbemQualifier __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *varValue);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            ISWbemQualifier __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *strName);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsLocal )( 
            ISWbemQualifier __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsLocal);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PropagatesToSubclass )( 
            ISWbemQualifier __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bPropagatesToSubclass);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PropagatesToSubclass )( 
            ISWbemQualifier __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bPropagatesToSubclass);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PropagatesToInstance )( 
            ISWbemQualifier __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bPropagatesToInstance);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PropagatesToInstance )( 
            ISWbemQualifier __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bPropagatesToInstance);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsOverridable )( 
            ISWbemQualifier __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsOverridable);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_IsOverridable )( 
            ISWbemQualifier __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bIsOverridable);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsAmended )( 
            ISWbemQualifier __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsAmended);
        
        END_INTERFACE
    } ISWbemQualifierVtbl;

    interface ISWbemQualifier
    {
        CONST_VTBL struct ISWbemQualifierVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemQualifier_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemQualifier_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemQualifier_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemQualifier_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemQualifier_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemQualifier_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemQualifier_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemQualifier_get_Value(This,varValue)	\
    (This)->lpVtbl -> get_Value(This,varValue)

#define ISWbemQualifier_put_Value(This,varValue)	\
    (This)->lpVtbl -> put_Value(This,varValue)

#define ISWbemQualifier_get_Name(This,strName)	\
    (This)->lpVtbl -> get_Name(This,strName)

#define ISWbemQualifier_get_IsLocal(This,bIsLocal)	\
    (This)->lpVtbl -> get_IsLocal(This,bIsLocal)

#define ISWbemQualifier_get_PropagatesToSubclass(This,bPropagatesToSubclass)	\
    (This)->lpVtbl -> get_PropagatesToSubclass(This,bPropagatesToSubclass)

#define ISWbemQualifier_put_PropagatesToSubclass(This,bPropagatesToSubclass)	\
    (This)->lpVtbl -> put_PropagatesToSubclass(This,bPropagatesToSubclass)

#define ISWbemQualifier_get_PropagatesToInstance(This,bPropagatesToInstance)	\
    (This)->lpVtbl -> get_PropagatesToInstance(This,bPropagatesToInstance)

#define ISWbemQualifier_put_PropagatesToInstance(This,bPropagatesToInstance)	\
    (This)->lpVtbl -> put_PropagatesToInstance(This,bPropagatesToInstance)

#define ISWbemQualifier_get_IsOverridable(This,bIsOverridable)	\
    (This)->lpVtbl -> get_IsOverridable(This,bIsOverridable)

#define ISWbemQualifier_put_IsOverridable(This,bIsOverridable)	\
    (This)->lpVtbl -> put_IsOverridable(This,bIsOverridable)

#define ISWbemQualifier_get_IsAmended(This,bIsAmended)	\
    (This)->lpVtbl -> get_IsAmended(This,bIsAmended)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemQualifier_get_Value_Proxy( 
    ISWbemQualifier __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *varValue);


void __RPC_STUB ISWbemQualifier_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISWbemQualifier_put_Value_Proxy( 
    ISWbemQualifier __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *varValue);


void __RPC_STUB ISWbemQualifier_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemQualifier_get_Name_Proxy( 
    ISWbemQualifier __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *strName);


void __RPC_STUB ISWbemQualifier_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemQualifier_get_IsLocal_Proxy( 
    ISWbemQualifier __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsLocal);


void __RPC_STUB ISWbemQualifier_get_IsLocal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemQualifier_get_PropagatesToSubclass_Proxy( 
    ISWbemQualifier __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bPropagatesToSubclass);


void __RPC_STUB ISWbemQualifier_get_PropagatesToSubclass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISWbemQualifier_put_PropagatesToSubclass_Proxy( 
    ISWbemQualifier __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL bPropagatesToSubclass);


void __RPC_STUB ISWbemQualifier_put_PropagatesToSubclass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemQualifier_get_PropagatesToInstance_Proxy( 
    ISWbemQualifier __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bPropagatesToInstance);


void __RPC_STUB ISWbemQualifier_get_PropagatesToInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISWbemQualifier_put_PropagatesToInstance_Proxy( 
    ISWbemQualifier __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL bPropagatesToInstance);


void __RPC_STUB ISWbemQualifier_put_PropagatesToInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemQualifier_get_IsOverridable_Proxy( 
    ISWbemQualifier __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsOverridable);


void __RPC_STUB ISWbemQualifier_get_IsOverridable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISWbemQualifier_put_IsOverridable_Proxy( 
    ISWbemQualifier __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL bIsOverridable);


void __RPC_STUB ISWbemQualifier_put_IsOverridable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemQualifier_get_IsAmended_Proxy( 
    ISWbemQualifier __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsAmended);


void __RPC_STUB ISWbemQualifier_get_IsAmended_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemQualifier_INTERFACE_DEFINED__ */


#ifndef __ISWbemQualifierSet_INTERFACE_DEFINED__
#define __ISWbemQualifierSet_INTERFACE_DEFINED__

/* interface ISWbemQualifierSet */
/* [helpstring][nonextensible][hidden][dual][oleautomation][uuid][local][object] */ 


EXTERN_C const IID IID_ISWbemQualifierSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9B16ED16-D3DF-11d1-8B08-00600806D9B6")
    ISWbemQualifierSet : public IDispatch
    {
    public:
        virtual /* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Item( 
            /* [in] */ BSTR name,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemQualifier __RPC_FAR *__RPC_FAR *objWbemQualifier) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long __RPC_FAR *iCount) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Add( 
            /* [in] */ BSTR strName,
            /* [in] */ VARIANT __RPC_FAR *varVal,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bPropagatesToSubclass,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bPropagatesToInstance,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bIsOverridable,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemQualifier __RPC_FAR *__RPC_FAR *objWbemQualifier) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( 
            /* [in] */ BSTR strName,
            /* [defaultvalue][optional][in] */ long iFlags = 0) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemQualifierSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemQualifierSet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemQualifierSet __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemQualifierSet __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemQualifierSet __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemQualifierSet __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemQualifierSet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemQualifierSet __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [restricted][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            ISWbemQualifierSet __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )( 
            ISWbemQualifierSet __RPC_FAR * This,
            /* [in] */ BSTR name,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemQualifier __RPC_FAR *__RPC_FAR *objWbemQualifier);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            ISWbemQualifierSet __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *iCount);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Add )( 
            ISWbemQualifierSet __RPC_FAR * This,
            /* [in] */ BSTR strName,
            /* [in] */ VARIANT __RPC_FAR *varVal,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bPropagatesToSubclass,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bPropagatesToInstance,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bIsOverridable,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemQualifier __RPC_FAR *__RPC_FAR *objWbemQualifier);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Remove )( 
            ISWbemQualifierSet __RPC_FAR * This,
            /* [in] */ BSTR strName,
            /* [defaultvalue][optional][in] */ long iFlags);
        
        END_INTERFACE
    } ISWbemQualifierSetVtbl;

    interface ISWbemQualifierSet
    {
        CONST_VTBL struct ISWbemQualifierSetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemQualifierSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemQualifierSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemQualifierSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemQualifierSet_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemQualifierSet_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemQualifierSet_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemQualifierSet_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemQualifierSet_get__NewEnum(This,pUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,pUnk)

#define ISWbemQualifierSet_Item(This,name,iFlags,objWbemQualifier)	\
    (This)->lpVtbl -> Item(This,name,iFlags,objWbemQualifier)

#define ISWbemQualifierSet_get_Count(This,iCount)	\
    (This)->lpVtbl -> get_Count(This,iCount)

#define ISWbemQualifierSet_Add(This,strName,varVal,bPropagatesToSubclass,bPropagatesToInstance,bIsOverridable,iFlags,objWbemQualifier)	\
    (This)->lpVtbl -> Add(This,strName,varVal,bPropagatesToSubclass,bPropagatesToInstance,bIsOverridable,iFlags,objWbemQualifier)

#define ISWbemQualifierSet_Remove(This,strName,iFlags)	\
    (This)->lpVtbl -> Remove(This,strName,iFlags)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemQualifierSet_get__NewEnum_Proxy( 
    ISWbemQualifierSet __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk);


void __RPC_STUB ISWbemQualifierSet_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemQualifierSet_Item_Proxy( 
    ISWbemQualifierSet __RPC_FAR * This,
    /* [in] */ BSTR name,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [retval][out] */ ISWbemQualifier __RPC_FAR *__RPC_FAR *objWbemQualifier);


void __RPC_STUB ISWbemQualifierSet_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemQualifierSet_get_Count_Proxy( 
    ISWbemQualifierSet __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *iCount);


void __RPC_STUB ISWbemQualifierSet_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemQualifierSet_Add_Proxy( 
    ISWbemQualifierSet __RPC_FAR * This,
    /* [in] */ BSTR strName,
    /* [in] */ VARIANT __RPC_FAR *varVal,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bPropagatesToSubclass,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bPropagatesToInstance,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bIsOverridable,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [retval][out] */ ISWbemQualifier __RPC_FAR *__RPC_FAR *objWbemQualifier);


void __RPC_STUB ISWbemQualifierSet_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemQualifierSet_Remove_Proxy( 
    ISWbemQualifierSet __RPC_FAR * This,
    /* [in] */ BSTR strName,
    /* [defaultvalue][optional][in] */ long iFlags);


void __RPC_STUB ISWbemQualifierSet_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemQualifierSet_INTERFACE_DEFINED__ */


#ifndef __ISWbemProperty_INTERFACE_DEFINED__
#define __ISWbemProperty_INTERFACE_DEFINED__

/* interface ISWbemProperty */
/* [helpstring][unique][nonextensible][hidden][oleautomation][dual][uuid][local][object] */ 


EXTERN_C const IID IID_ISWbemProperty;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1A388F98-D4BA-11d1-8B09-00600806D9B6")
    ISWbemProperty : public IDispatch
    {
    public:
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Value( 
            /* [retval][out] */ VARIANT __RPC_FAR *varValue) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Value( 
            /* [in] */ VARIANT __RPC_FAR *varValue) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR __RPC_FAR *strName) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_IsLocal( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsLocal) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Origin( 
            /* [retval][out] */ BSTR __RPC_FAR *strOrigin) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_CIMType( 
            /* [retval][out] */ WbemCimtypeEnum __RPC_FAR *iCimType) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Qualifiers_( 
            /* [retval][out] */ ISWbemQualifierSet __RPC_FAR *__RPC_FAR *objWbemQualifierSet) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_IsArray( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsArray) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemPropertyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemProperty __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemProperty __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemProperty __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemProperty __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemProperty __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemProperty __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemProperty __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Value )( 
            ISWbemProperty __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *varValue);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Value )( 
            ISWbemProperty __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *varValue);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            ISWbemProperty __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *strName);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsLocal )( 
            ISWbemProperty __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsLocal);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Origin )( 
            ISWbemProperty __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *strOrigin);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CIMType )( 
            ISWbemProperty __RPC_FAR * This,
            /* [retval][out] */ WbemCimtypeEnum __RPC_FAR *iCimType);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Qualifiers_ )( 
            ISWbemProperty __RPC_FAR * This,
            /* [retval][out] */ ISWbemQualifierSet __RPC_FAR *__RPC_FAR *objWbemQualifierSet);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsArray )( 
            ISWbemProperty __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsArray);
        
        END_INTERFACE
    } ISWbemPropertyVtbl;

    interface ISWbemProperty
    {
        CONST_VTBL struct ISWbemPropertyVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemProperty_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemProperty_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemProperty_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemProperty_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemProperty_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemProperty_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemProperty_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemProperty_get_Value(This,varValue)	\
    (This)->lpVtbl -> get_Value(This,varValue)

#define ISWbemProperty_put_Value(This,varValue)	\
    (This)->lpVtbl -> put_Value(This,varValue)

#define ISWbemProperty_get_Name(This,strName)	\
    (This)->lpVtbl -> get_Name(This,strName)

#define ISWbemProperty_get_IsLocal(This,bIsLocal)	\
    (This)->lpVtbl -> get_IsLocal(This,bIsLocal)

#define ISWbemProperty_get_Origin(This,strOrigin)	\
    (This)->lpVtbl -> get_Origin(This,strOrigin)

#define ISWbemProperty_get_CIMType(This,iCimType)	\
    (This)->lpVtbl -> get_CIMType(This,iCimType)

#define ISWbemProperty_get_Qualifiers_(This,objWbemQualifierSet)	\
    (This)->lpVtbl -> get_Qualifiers_(This,objWbemQualifierSet)

#define ISWbemProperty_get_IsArray(This,bIsArray)	\
    (This)->lpVtbl -> get_IsArray(This,bIsArray)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemProperty_get_Value_Proxy( 
    ISWbemProperty __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *varValue);


void __RPC_STUB ISWbemProperty_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISWbemProperty_put_Value_Proxy( 
    ISWbemProperty __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *varValue);


void __RPC_STUB ISWbemProperty_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemProperty_get_Name_Proxy( 
    ISWbemProperty __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *strName);


void __RPC_STUB ISWbemProperty_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemProperty_get_IsLocal_Proxy( 
    ISWbemProperty __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsLocal);


void __RPC_STUB ISWbemProperty_get_IsLocal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemProperty_get_Origin_Proxy( 
    ISWbemProperty __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *strOrigin);


void __RPC_STUB ISWbemProperty_get_Origin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemProperty_get_CIMType_Proxy( 
    ISWbemProperty __RPC_FAR * This,
    /* [retval][out] */ WbemCimtypeEnum __RPC_FAR *iCimType);


void __RPC_STUB ISWbemProperty_get_CIMType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemProperty_get_Qualifiers__Proxy( 
    ISWbemProperty __RPC_FAR * This,
    /* [retval][out] */ ISWbemQualifierSet __RPC_FAR *__RPC_FAR *objWbemQualifierSet);


void __RPC_STUB ISWbemProperty_get_Qualifiers__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemProperty_get_IsArray_Proxy( 
    ISWbemProperty __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsArray);


void __RPC_STUB ISWbemProperty_get_IsArray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemProperty_INTERFACE_DEFINED__ */


#ifndef __ISWbemPropertySet_INTERFACE_DEFINED__
#define __ISWbemPropertySet_INTERFACE_DEFINED__

/* interface ISWbemPropertySet */
/* [helpstring][nonextensible][hidden][dual][oleautomation][uuid][local][object] */ 


EXTERN_C const IID IID_ISWbemPropertySet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DEA0A7B2-D4BA-11d1-8B09-00600806D9B6")
    ISWbemPropertySet : public IDispatch
    {
    public:
        virtual /* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Item( 
            /* [in] */ BSTR strName,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemProperty __RPC_FAR *__RPC_FAR *objWbemProperty) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long __RPC_FAR *iCount) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Add( 
            /* [in] */ BSTR strName,
            /* [in] */ WbemCimtypeEnum iCIMType,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bIsArray,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemProperty __RPC_FAR *__RPC_FAR *objWbemProperty) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( 
            /* [in] */ BSTR strName,
            /* [defaultvalue][optional][in] */ long iFlags = 0) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemPropertySetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemPropertySet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemPropertySet __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemPropertySet __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemPropertySet __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemPropertySet __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemPropertySet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemPropertySet __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [restricted][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            ISWbemPropertySet __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )( 
            ISWbemPropertySet __RPC_FAR * This,
            /* [in] */ BSTR strName,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemProperty __RPC_FAR *__RPC_FAR *objWbemProperty);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            ISWbemPropertySet __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *iCount);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Add )( 
            ISWbemPropertySet __RPC_FAR * This,
            /* [in] */ BSTR strName,
            /* [in] */ WbemCimtypeEnum iCIMType,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bIsArray,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemProperty __RPC_FAR *__RPC_FAR *objWbemProperty);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Remove )( 
            ISWbemPropertySet __RPC_FAR * This,
            /* [in] */ BSTR strName,
            /* [defaultvalue][optional][in] */ long iFlags);
        
        END_INTERFACE
    } ISWbemPropertySetVtbl;

    interface ISWbemPropertySet
    {
        CONST_VTBL struct ISWbemPropertySetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemPropertySet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemPropertySet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemPropertySet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemPropertySet_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemPropertySet_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemPropertySet_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemPropertySet_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemPropertySet_get__NewEnum(This,pUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,pUnk)

#define ISWbemPropertySet_Item(This,strName,iFlags,objWbemProperty)	\
    (This)->lpVtbl -> Item(This,strName,iFlags,objWbemProperty)

#define ISWbemPropertySet_get_Count(This,iCount)	\
    (This)->lpVtbl -> get_Count(This,iCount)

#define ISWbemPropertySet_Add(This,strName,iCIMType,bIsArray,iFlags,objWbemProperty)	\
    (This)->lpVtbl -> Add(This,strName,iCIMType,bIsArray,iFlags,objWbemProperty)

#define ISWbemPropertySet_Remove(This,strName,iFlags)	\
    (This)->lpVtbl -> Remove(This,strName,iFlags)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemPropertySet_get__NewEnum_Proxy( 
    ISWbemPropertySet __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk);


void __RPC_STUB ISWbemPropertySet_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemPropertySet_Item_Proxy( 
    ISWbemPropertySet __RPC_FAR * This,
    /* [in] */ BSTR strName,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [retval][out] */ ISWbemProperty __RPC_FAR *__RPC_FAR *objWbemProperty);


void __RPC_STUB ISWbemPropertySet_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemPropertySet_get_Count_Proxy( 
    ISWbemPropertySet __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *iCount);


void __RPC_STUB ISWbemPropertySet_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemPropertySet_Add_Proxy( 
    ISWbemPropertySet __RPC_FAR * This,
    /* [in] */ BSTR strName,
    /* [in] */ WbemCimtypeEnum iCIMType,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bIsArray,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [retval][out] */ ISWbemProperty __RPC_FAR *__RPC_FAR *objWbemProperty);


void __RPC_STUB ISWbemPropertySet_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemPropertySet_Remove_Proxy( 
    ISWbemPropertySet __RPC_FAR * This,
    /* [in] */ BSTR strName,
    /* [defaultvalue][optional][in] */ long iFlags);


void __RPC_STUB ISWbemPropertySet_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemPropertySet_INTERFACE_DEFINED__ */


#ifndef __ISWbemMethod_INTERFACE_DEFINED__
#define __ISWbemMethod_INTERFACE_DEFINED__

/* interface ISWbemMethod */
/* [helpstring][hidden][nonextensible][unique][oleautomation][dual][uuid][local][object] */ 


EXTERN_C const IID IID_ISWbemMethod;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("422E8E90-D955-11d1-8B09-00600806D9B6")
    ISWbemMethod : public IDispatch
    {
    public:
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR __RPC_FAR *strName) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Origin( 
            /* [retval][out] */ BSTR __RPC_FAR *strOrigin) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_InParameters( 
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemInParameters) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_OutParameters( 
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemOutParameters) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Qualifiers_( 
            /* [retval][out] */ ISWbemQualifierSet __RPC_FAR *__RPC_FAR *objWbemQualifierSet) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemMethodVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemMethod __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemMethod __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemMethod __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemMethod __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemMethod __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemMethod __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemMethod __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            ISWbemMethod __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *strName);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Origin )( 
            ISWbemMethod __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *strOrigin);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_InParameters )( 
            ISWbemMethod __RPC_FAR * This,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemInParameters);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_OutParameters )( 
            ISWbemMethod __RPC_FAR * This,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemOutParameters);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Qualifiers_ )( 
            ISWbemMethod __RPC_FAR * This,
            /* [retval][out] */ ISWbemQualifierSet __RPC_FAR *__RPC_FAR *objWbemQualifierSet);
        
        END_INTERFACE
    } ISWbemMethodVtbl;

    interface ISWbemMethod
    {
        CONST_VTBL struct ISWbemMethodVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemMethod_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemMethod_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemMethod_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemMethod_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemMethod_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemMethod_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemMethod_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemMethod_get_Name(This,strName)	\
    (This)->lpVtbl -> get_Name(This,strName)

#define ISWbemMethod_get_Origin(This,strOrigin)	\
    (This)->lpVtbl -> get_Origin(This,strOrigin)

#define ISWbemMethod_get_InParameters(This,objWbemInParameters)	\
    (This)->lpVtbl -> get_InParameters(This,objWbemInParameters)

#define ISWbemMethod_get_OutParameters(This,objWbemOutParameters)	\
    (This)->lpVtbl -> get_OutParameters(This,objWbemOutParameters)

#define ISWbemMethod_get_Qualifiers_(This,objWbemQualifierSet)	\
    (This)->lpVtbl -> get_Qualifiers_(This,objWbemQualifierSet)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemMethod_get_Name_Proxy( 
    ISWbemMethod __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *strName);


void __RPC_STUB ISWbemMethod_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemMethod_get_Origin_Proxy( 
    ISWbemMethod __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *strOrigin);


void __RPC_STUB ISWbemMethod_get_Origin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemMethod_get_InParameters_Proxy( 
    ISWbemMethod __RPC_FAR * This,
    /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemInParameters);


void __RPC_STUB ISWbemMethod_get_InParameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemMethod_get_OutParameters_Proxy( 
    ISWbemMethod __RPC_FAR * This,
    /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemOutParameters);


void __RPC_STUB ISWbemMethod_get_OutParameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemMethod_get_Qualifiers__Proxy( 
    ISWbemMethod __RPC_FAR * This,
    /* [retval][out] */ ISWbemQualifierSet __RPC_FAR *__RPC_FAR *objWbemQualifierSet);


void __RPC_STUB ISWbemMethod_get_Qualifiers__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemMethod_INTERFACE_DEFINED__ */


#ifndef __ISWbemMethodSet_INTERFACE_DEFINED__
#define __ISWbemMethodSet_INTERFACE_DEFINED__

/* interface ISWbemMethodSet */
/* [helpstring][hidden][nonextensible][dual][oleautomation][uuid][object][local] */ 


EXTERN_C const IID IID_ISWbemMethodSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C93BA292-D955-11d1-8B09-00600806D9B6")
    ISWbemMethodSet : public IDispatch
    {
    public:
        virtual /* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Item( 
            /* [in] */ BSTR strName,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemMethod __RPC_FAR *__RPC_FAR *objWbemMethod) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long __RPC_FAR *iCount) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemMethodSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemMethodSet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemMethodSet __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemMethodSet __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemMethodSet __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemMethodSet __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemMethodSet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemMethodSet __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [restricted][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            ISWbemMethodSet __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )( 
            ISWbemMethodSet __RPC_FAR * This,
            /* [in] */ BSTR strName,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemMethod __RPC_FAR *__RPC_FAR *objWbemMethod);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            ISWbemMethodSet __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *iCount);
        
        END_INTERFACE
    } ISWbemMethodSetVtbl;

    interface ISWbemMethodSet
    {
        CONST_VTBL struct ISWbemMethodSetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemMethodSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemMethodSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemMethodSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemMethodSet_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemMethodSet_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemMethodSet_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemMethodSet_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemMethodSet_get__NewEnum(This,pUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,pUnk)

#define ISWbemMethodSet_Item(This,strName,iFlags,objWbemMethod)	\
    (This)->lpVtbl -> Item(This,strName,iFlags,objWbemMethod)

#define ISWbemMethodSet_get_Count(This,iCount)	\
    (This)->lpVtbl -> get_Count(This,iCount)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemMethodSet_get__NewEnum_Proxy( 
    ISWbemMethodSet __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk);


void __RPC_STUB ISWbemMethodSet_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemMethodSet_Item_Proxy( 
    ISWbemMethodSet __RPC_FAR * This,
    /* [in] */ BSTR strName,
    /* [defaultvalue][optional][in] */ long iFlags,
    /* [retval][out] */ ISWbemMethod __RPC_FAR *__RPC_FAR *objWbemMethod);


void __RPC_STUB ISWbemMethodSet_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemMethodSet_get_Count_Proxy( 
    ISWbemMethodSet __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *iCount);


void __RPC_STUB ISWbemMethodSet_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemMethodSet_INTERFACE_DEFINED__ */


#ifndef __ISWbemEventSource_INTERFACE_DEFINED__
#define __ISWbemEventSource_INTERFACE_DEFINED__

/* interface ISWbemEventSource */
/* [helpstring][hidden][nonextensible][dual][oleautomation][uuid][local][object] */ 


EXTERN_C const IID IID_ISWbemEventSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("27D54D92-0EBE-11d2-8B22-00600806D9B6")
    ISWbemEventSource : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE NextEvent( 
            /* [defaultvalue][optional][in] */ long iTimeoutMs,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Security_( 
            /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemEventSourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemEventSource __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemEventSource __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemEventSource __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemEventSource __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemEventSource __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemEventSource __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemEventSource __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NextEvent )( 
            ISWbemEventSource __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ long iTimeoutMs,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Security_ )( 
            ISWbemEventSource __RPC_FAR * This,
            /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity);
        
        END_INTERFACE
    } ISWbemEventSourceVtbl;

    interface ISWbemEventSource
    {
        CONST_VTBL struct ISWbemEventSourceVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemEventSource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemEventSource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemEventSource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemEventSource_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemEventSource_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemEventSource_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemEventSource_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemEventSource_NextEvent(This,iTimeoutMs,objWbemObject)	\
    (This)->lpVtbl -> NextEvent(This,iTimeoutMs,objWbemObject)

#define ISWbemEventSource_get_Security_(This,objWbemSecurity)	\
    (This)->lpVtbl -> get_Security_(This,objWbemSecurity)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemEventSource_NextEvent_Proxy( 
    ISWbemEventSource __RPC_FAR * This,
    /* [defaultvalue][optional][in] */ long iTimeoutMs,
    /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject);


void __RPC_STUB ISWbemEventSource_NextEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemEventSource_get_Security__Proxy( 
    ISWbemEventSource __RPC_FAR * This,
    /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity);


void __RPC_STUB ISWbemEventSource_get_Security__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemEventSource_INTERFACE_DEFINED__ */


#ifndef __ISWbemObjectPath_INTERFACE_DEFINED__
#define __ISWbemObjectPath_INTERFACE_DEFINED__

/* interface ISWbemObjectPath */
/* [helpstring][unique][nonextensible][hidden][oleautomation][dual][uuid][local][object] */ 


EXTERN_C const IID IID_ISWbemObjectPath;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5791BC27-CE9C-11d1-97BF-0000F81E849C")
    ISWbemObjectPath : public IDispatch
    {
    public:
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Path( 
            /* [retval][out] */ BSTR __RPC_FAR *strPath) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Path( 
            /* [in] */ BSTR strPath) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_RelPath( 
            /* [retval][out] */ BSTR __RPC_FAR *strRelPath) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_RelPath( 
            /* [in] */ BSTR strRelPath) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Server( 
            /* [retval][out] */ BSTR __RPC_FAR *strServer) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Server( 
            /* [in] */ BSTR strServer) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Namespace( 
            /* [retval][out] */ BSTR __RPC_FAR *strNamespace) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Namespace( 
            /* [in] */ BSTR strNamespace) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_ParentNamespace( 
            /* [retval][out] */ BSTR __RPC_FAR *strParentNamespace) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_DisplayName( 
            /* [retval][out] */ BSTR __RPC_FAR *strDisplayName) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_DisplayName( 
            /* [in] */ BSTR strDisplayName) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Class( 
            /* [retval][out] */ BSTR __RPC_FAR *strClass) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Class( 
            /* [in] */ BSTR strClass) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_IsClass( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsClass) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetAsClass( void) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_IsSingleton( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsSingleton) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetAsSingleton( void) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Keys( 
            /* [retval][out] */ ISWbemNamedValueSet __RPC_FAR *__RPC_FAR *objWbemNamedValueSet) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Security_( 
            /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Locale( 
            /* [retval][out] */ BSTR __RPC_FAR *strLocale) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Locale( 
            /* [in] */ BSTR strLocale) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Authority( 
            /* [retval][out] */ BSTR __RPC_FAR *strAuthority) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Authority( 
            /* [in] */ BSTR strAuthority) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemObjectPathVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemObjectPath __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemObjectPath __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Path )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *strPath);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Path )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [in] */ BSTR strPath);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_RelPath )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *strRelPath);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_RelPath )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [in] */ BSTR strRelPath);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Server )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *strServer);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Server )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [in] */ BSTR strServer);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Namespace )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *strNamespace);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Namespace )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [in] */ BSTR strNamespace);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ParentNamespace )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *strParentNamespace);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DisplayName )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *strDisplayName);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DisplayName )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [in] */ BSTR strDisplayName);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Class )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *strClass);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Class )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [in] */ BSTR strClass);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsClass )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsClass);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetAsClass )( 
            ISWbemObjectPath __RPC_FAR * This);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsSingleton )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsSingleton);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetAsSingleton )( 
            ISWbemObjectPath __RPC_FAR * This);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Keys )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [retval][out] */ ISWbemNamedValueSet __RPC_FAR *__RPC_FAR *objWbemNamedValueSet);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Security_ )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Locale )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *strLocale);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Locale )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [in] */ BSTR strLocale);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Authority )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *strAuthority);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Authority )( 
            ISWbemObjectPath __RPC_FAR * This,
            /* [in] */ BSTR strAuthority);
        
        END_INTERFACE
    } ISWbemObjectPathVtbl;

    interface ISWbemObjectPath
    {
        CONST_VTBL struct ISWbemObjectPathVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemObjectPath_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemObjectPath_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemObjectPath_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemObjectPath_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemObjectPath_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemObjectPath_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemObjectPath_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemObjectPath_get_Path(This,strPath)	\
    (This)->lpVtbl -> get_Path(This,strPath)

#define ISWbemObjectPath_put_Path(This,strPath)	\
    (This)->lpVtbl -> put_Path(This,strPath)

#define ISWbemObjectPath_get_RelPath(This,strRelPath)	\
    (This)->lpVtbl -> get_RelPath(This,strRelPath)

#define ISWbemObjectPath_put_RelPath(This,strRelPath)	\
    (This)->lpVtbl -> put_RelPath(This,strRelPath)

#define ISWbemObjectPath_get_Server(This,strServer)	\
    (This)->lpVtbl -> get_Server(This,strServer)

#define ISWbemObjectPath_put_Server(This,strServer)	\
    (This)->lpVtbl -> put_Server(This,strServer)

#define ISWbemObjectPath_get_Namespace(This,strNamespace)	\
    (This)->lpVtbl -> get_Namespace(This,strNamespace)

#define ISWbemObjectPath_put_Namespace(This,strNamespace)	\
    (This)->lpVtbl -> put_Namespace(This,strNamespace)

#define ISWbemObjectPath_get_ParentNamespace(This,strParentNamespace)	\
    (This)->lpVtbl -> get_ParentNamespace(This,strParentNamespace)

#define ISWbemObjectPath_get_DisplayName(This,strDisplayName)	\
    (This)->lpVtbl -> get_DisplayName(This,strDisplayName)

#define ISWbemObjectPath_put_DisplayName(This,strDisplayName)	\
    (This)->lpVtbl -> put_DisplayName(This,strDisplayName)

#define ISWbemObjectPath_get_Class(This,strClass)	\
    (This)->lpVtbl -> get_Class(This,strClass)

#define ISWbemObjectPath_put_Class(This,strClass)	\
    (This)->lpVtbl -> put_Class(This,strClass)

#define ISWbemObjectPath_get_IsClass(This,bIsClass)	\
    (This)->lpVtbl -> get_IsClass(This,bIsClass)

#define ISWbemObjectPath_SetAsClass(This)	\
    (This)->lpVtbl -> SetAsClass(This)

#define ISWbemObjectPath_get_IsSingleton(This,bIsSingleton)	\
    (This)->lpVtbl -> get_IsSingleton(This,bIsSingleton)

#define ISWbemObjectPath_SetAsSingleton(This)	\
    (This)->lpVtbl -> SetAsSingleton(This)

#define ISWbemObjectPath_get_Keys(This,objWbemNamedValueSet)	\
    (This)->lpVtbl -> get_Keys(This,objWbemNamedValueSet)

#define ISWbemObjectPath_get_Security_(This,objWbemSecurity)	\
    (This)->lpVtbl -> get_Security_(This,objWbemSecurity)

#define ISWbemObjectPath_get_Locale(This,strLocale)	\
    (This)->lpVtbl -> get_Locale(This,strLocale)

#define ISWbemObjectPath_put_Locale(This,strLocale)	\
    (This)->lpVtbl -> put_Locale(This,strLocale)

#define ISWbemObjectPath_get_Authority(This,strAuthority)	\
    (This)->lpVtbl -> get_Authority(This,strAuthority)

#define ISWbemObjectPath_put_Authority(This,strAuthority)	\
    (This)->lpVtbl -> put_Authority(This,strAuthority)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_Path_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *strPath);


void __RPC_STUB ISWbemObjectPath_get_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_put_Path_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [in] */ BSTR strPath);


void __RPC_STUB ISWbemObjectPath_put_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_RelPath_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *strRelPath);


void __RPC_STUB ISWbemObjectPath_get_RelPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_put_RelPath_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [in] */ BSTR strRelPath);


void __RPC_STUB ISWbemObjectPath_put_RelPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_Server_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *strServer);


void __RPC_STUB ISWbemObjectPath_get_Server_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_put_Server_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [in] */ BSTR strServer);


void __RPC_STUB ISWbemObjectPath_put_Server_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_Namespace_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *strNamespace);


void __RPC_STUB ISWbemObjectPath_get_Namespace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_put_Namespace_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [in] */ BSTR strNamespace);


void __RPC_STUB ISWbemObjectPath_put_Namespace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_ParentNamespace_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *strParentNamespace);


void __RPC_STUB ISWbemObjectPath_get_ParentNamespace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_DisplayName_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *strDisplayName);


void __RPC_STUB ISWbemObjectPath_get_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_put_DisplayName_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [in] */ BSTR strDisplayName);


void __RPC_STUB ISWbemObjectPath_put_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_Class_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *strClass);


void __RPC_STUB ISWbemObjectPath_get_Class_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_put_Class_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [in] */ BSTR strClass);


void __RPC_STUB ISWbemObjectPath_put_Class_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_IsClass_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsClass);


void __RPC_STUB ISWbemObjectPath_get_IsClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_SetAsClass_Proxy( 
    ISWbemObjectPath __RPC_FAR * This);


void __RPC_STUB ISWbemObjectPath_SetAsClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_IsSingleton_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsSingleton);


void __RPC_STUB ISWbemObjectPath_get_IsSingleton_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_SetAsSingleton_Proxy( 
    ISWbemObjectPath __RPC_FAR * This);


void __RPC_STUB ISWbemObjectPath_SetAsSingleton_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_Keys_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [retval][out] */ ISWbemNamedValueSet __RPC_FAR *__RPC_FAR *objWbemNamedValueSet);


void __RPC_STUB ISWbemObjectPath_get_Keys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_Security__Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity);


void __RPC_STUB ISWbemObjectPath_get_Security__Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_Locale_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *strLocale);


void __RPC_STUB ISWbemObjectPath_get_Locale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_put_Locale_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [in] */ BSTR strLocale);


void __RPC_STUB ISWbemObjectPath_put_Locale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_get_Authority_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *strAuthority);


void __RPC_STUB ISWbemObjectPath_get_Authority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISWbemObjectPath_put_Authority_Proxy( 
    ISWbemObjectPath __RPC_FAR * This,
    /* [in] */ BSTR strAuthority);


void __RPC_STUB ISWbemObjectPath_put_Authority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemObjectPath_INTERFACE_DEFINED__ */


#ifndef __ISWbemLastError_INTERFACE_DEFINED__
#define __ISWbemLastError_INTERFACE_DEFINED__

/* interface ISWbemLastError */
/* [helpstring][hidden][oleautomation][dual][uuid][object][local] */ 


EXTERN_C const IID IID_ISWbemLastError;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D962DB84-D4BB-11d1-8B09-00600806D9B6")
    ISWbemLastError : public ISWbemObject
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct ISWbemLastErrorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemLastError __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemLastError __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemLastError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemLastError __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemLastError __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemLastError __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemLastError __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Put_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectPath __RPC_FAR *__RPC_FAR *objWbemObjectPath);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PutAsync_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Delete_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteAsync_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Instances_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InstancesAsync_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Subclasses_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SubclassesAsync_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Associators_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR strAssocClass,
            /* [defaultvalue][optional][in] */ BSTR strResultClass,
            /* [defaultvalue][optional][in] */ BSTR strResultRole,
            /* [defaultvalue][optional][in] */ BSTR strRole,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
            /* [defaultvalue][optional][in] */ BSTR strRequiredAssocQualifier,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AssociatorsAsync_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ BSTR strAssocClass,
            /* [defaultvalue][optional][in] */ BSTR strResultClass,
            /* [defaultvalue][optional][in] */ BSTR strResultRole,
            /* [defaultvalue][optional][in] */ BSTR strRole,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
            /* [defaultvalue][optional][in] */ BSTR strRequiredAssocQualifier,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *References_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ BSTR strResultClass,
            /* [defaultvalue][optional][in] */ BSTR strRole,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObjectSet __RPC_FAR *__RPC_FAR *objWbemObjectSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReferencesAsync_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [defaultvalue][optional][in] */ BSTR strResultClass,
            /* [defaultvalue][optional][in] */ BSTR strRole,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bClassesOnly,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bSchemaOnly,
            /* [defaultvalue][optional][in] */ BSTR strRequiredQualifier,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExecMethod_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [in] */ BSTR strMethodName,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemInParameters,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemOutParameters);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExecMethodAsync_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemSink,
            /* [in] */ BSTR strMethodName,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemInParameters,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemNamedValueSet,
            /* [defaultvalue][optional][in] */ IDispatch __RPC_FAR *objWbemAsyncContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetObjectText_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ BSTR __RPC_FAR *strObjectText);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SpawnDerivedClass_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SpawnInstance_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ ISWbemObject __RPC_FAR *__RPC_FAR *objWbemObject);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CompareTo_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [in] */ IDispatch __RPC_FAR *objWbemObject,
            /* [defaultvalue][optional][in] */ long iFlags,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bResult);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Qualifiers_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [retval][out] */ ISWbemQualifierSet __RPC_FAR *__RPC_FAR *objWbemQualifierSet);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Properties_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [retval][out] */ ISWbemPropertySet __RPC_FAR *__RPC_FAR *objWbemPropertySet);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Methods_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [retval][out] */ ISWbemMethodSet __RPC_FAR *__RPC_FAR *objWbemMethodSet);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Derivation_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *strClassNameArray);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Path_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [retval][out] */ ISWbemObjectPath __RPC_FAR *__RPC_FAR *objWbemObjectPath);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Security_ )( 
            ISWbemLastError __RPC_FAR * This,
            /* [retval][out] */ ISWbemSecurity __RPC_FAR *__RPC_FAR *objWbemSecurity);
        
        END_INTERFACE
    } ISWbemLastErrorVtbl;

    interface ISWbemLastError
    {
        CONST_VTBL struct ISWbemLastErrorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemLastError_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemLastError_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemLastError_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemLastError_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemLastError_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemLastError_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemLastError_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemLastError_Put_(This,iFlags,objWbemNamedValueSet,objWbemObjectPath)	\
    (This)->lpVtbl -> Put_(This,iFlags,objWbemNamedValueSet,objWbemObjectPath)

#define ISWbemLastError_PutAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> PutAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemLastError_Delete_(This,iFlags,objWbemNamedValueSet)	\
    (This)->lpVtbl -> Delete_(This,iFlags,objWbemNamedValueSet)

#define ISWbemLastError_DeleteAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> DeleteAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemLastError_Instances_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> Instances_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemLastError_InstancesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> InstancesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemLastError_Subclasses_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> Subclasses_(This,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemLastError_SubclassesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> SubclassesAsync_(This,objWbemSink,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemLastError_Associators_(This,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> Associators_(This,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemLastError_AssociatorsAsync_(This,objWbemSink,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> AssociatorsAsync_(This,objWbemSink,strAssocClass,strResultClass,strResultRole,strRole,bClassesOnly,bSchemaOnly,strRequiredAssocQualifier,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemLastError_References_(This,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)	\
    (This)->lpVtbl -> References_(This,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemObjectSet)

#define ISWbemLastError_ReferencesAsync_(This,objWbemSink,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ReferencesAsync_(This,objWbemSink,strResultClass,strRole,bClassesOnly,bSchemaOnly,strRequiredQualifier,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemLastError_ExecMethod_(This,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemOutParameters)	\
    (This)->lpVtbl -> ExecMethod_(This,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemOutParameters)

#define ISWbemLastError_ExecMethodAsync_(This,objWbemSink,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemAsyncContext)	\
    (This)->lpVtbl -> ExecMethodAsync_(This,objWbemSink,strMethodName,objWbemInParameters,iFlags,objWbemNamedValueSet,objWbemAsyncContext)

#define ISWbemLastError_Clone_(This,objWbemObject)	\
    (This)->lpVtbl -> Clone_(This,objWbemObject)

#define ISWbemLastError_GetObjectText_(This,iFlags,strObjectText)	\
    (This)->lpVtbl -> GetObjectText_(This,iFlags,strObjectText)

#define ISWbemLastError_SpawnDerivedClass_(This,iFlags,objWbemObject)	\
    (This)->lpVtbl -> SpawnDerivedClass_(This,iFlags,objWbemObject)

#define ISWbemLastError_SpawnInstance_(This,iFlags,objWbemObject)	\
    (This)->lpVtbl -> SpawnInstance_(This,iFlags,objWbemObject)

#define ISWbemLastError_CompareTo_(This,objWbemObject,iFlags,bResult)	\
    (This)->lpVtbl -> CompareTo_(This,objWbemObject,iFlags,bResult)

#define ISWbemLastError_get_Qualifiers_(This,objWbemQualifierSet)	\
    (This)->lpVtbl -> get_Qualifiers_(This,objWbemQualifierSet)

#define ISWbemLastError_get_Properties_(This,objWbemPropertySet)	\
    (This)->lpVtbl -> get_Properties_(This,objWbemPropertySet)

#define ISWbemLastError_get_Methods_(This,objWbemMethodSet)	\
    (This)->lpVtbl -> get_Methods_(This,objWbemMethodSet)

#define ISWbemLastError_get_Derivation_(This,strClassNameArray)	\
    (This)->lpVtbl -> get_Derivation_(This,strClassNameArray)

#define ISWbemLastError_get_Path_(This,objWbemObjectPath)	\
    (This)->lpVtbl -> get_Path_(This,objWbemObjectPath)

#define ISWbemLastError_get_Security_(This,objWbemSecurity)	\
    (This)->lpVtbl -> get_Security_(This,objWbemSecurity)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISWbemLastError_INTERFACE_DEFINED__ */


#ifndef __ISWbemSinkEvents_DISPINTERFACE_DEFINED__
#define __ISWbemSinkEvents_DISPINTERFACE_DEFINED__

/* dispinterface ISWbemSinkEvents */
/* [hidden][nonextensible][helpstring][uuid] */ 


EXTERN_C const IID DIID_ISWbemSinkEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("75718CA0-F029-11d1-A1AC-00C04FB6C223")
    ISWbemSinkEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct ISWbemSinkEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemSinkEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemSinkEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemSinkEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemSinkEvents __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemSinkEvents __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemSinkEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemSinkEvents __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } ISWbemSinkEventsVtbl;

    interface ISWbemSinkEvents
    {
        CONST_VTBL struct ISWbemSinkEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemSinkEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemSinkEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemSinkEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemSinkEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemSinkEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemSinkEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemSinkEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __ISWbemSinkEvents_DISPINTERFACE_DEFINED__ */


#ifndef __ISWbemSink_INTERFACE_DEFINED__
#define __ISWbemSink_INTERFACE_DEFINED__

/* interface ISWbemSink */
/* [helpstring][nonextensible][hidden][dual][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_ISWbemSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("75718C9F-F029-11d1-A1AC-00C04FB6C223")
    ISWbemSink : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemSink __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemSink __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemSink __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemSink __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemSink __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemSink __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemSink __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Cancel )( 
            ISWbemSink __RPC_FAR * This);
        
        END_INTERFACE
    } ISWbemSinkVtbl;

    interface ISWbemSink
    {
        CONST_VTBL struct ISWbemSinkVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemSink_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemSink_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemSink_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemSink_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemSink_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemSink_Cancel_Proxy( 
    ISWbemSink __RPC_FAR * This);


void __RPC_STUB ISWbemSink_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemSink_INTERFACE_DEFINED__ */


#ifndef __ISWbemSecurity_INTERFACE_DEFINED__
#define __ISWbemSecurity_INTERFACE_DEFINED__

/* interface ISWbemSecurity */
/* [helpstring][hidden][nonextensible][dual][oleautomation][uuid][local][object] */ 


EXTERN_C const IID IID_ISWbemSecurity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B54D66E6-2287-11d2-8B33-00600806D9B6")
    ISWbemSecurity : public IDispatch
    {
    public:
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_ImpersonationLevel( 
            /* [retval][out] */ WbemImpersonationLevelEnum __RPC_FAR *iImpersonationLevel) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_ImpersonationLevel( 
            /* [in] */ WbemImpersonationLevelEnum iImpersonationLevel) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_AuthenticationLevel( 
            /* [retval][out] */ WbemAuthenticationLevelEnum __RPC_FAR *iAuthenticationLevel) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_AuthenticationLevel( 
            /* [in] */ WbemAuthenticationLevelEnum iAuthenticationLevel) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Privileges( 
            /* [retval][out] */ ISWbemPrivilegeSet __RPC_FAR *__RPC_FAR *objWbemPrivilegeSet) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemSecurityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemSecurity __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemSecurity __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemSecurity __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemSecurity __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemSecurity __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemSecurity __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemSecurity __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ImpersonationLevel )( 
            ISWbemSecurity __RPC_FAR * This,
            /* [retval][out] */ WbemImpersonationLevelEnum __RPC_FAR *iImpersonationLevel);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ImpersonationLevel )( 
            ISWbemSecurity __RPC_FAR * This,
            /* [in] */ WbemImpersonationLevelEnum iImpersonationLevel);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AuthenticationLevel )( 
            ISWbemSecurity __RPC_FAR * This,
            /* [retval][out] */ WbemAuthenticationLevelEnum __RPC_FAR *iAuthenticationLevel);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AuthenticationLevel )( 
            ISWbemSecurity __RPC_FAR * This,
            /* [in] */ WbemAuthenticationLevelEnum iAuthenticationLevel);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Privileges )( 
            ISWbemSecurity __RPC_FAR * This,
            /* [retval][out] */ ISWbemPrivilegeSet __RPC_FAR *__RPC_FAR *objWbemPrivilegeSet);
        
        END_INTERFACE
    } ISWbemSecurityVtbl;

    interface ISWbemSecurity
    {
        CONST_VTBL struct ISWbemSecurityVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemSecurity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemSecurity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemSecurity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemSecurity_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemSecurity_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemSecurity_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemSecurity_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemSecurity_get_ImpersonationLevel(This,iImpersonationLevel)	\
    (This)->lpVtbl -> get_ImpersonationLevel(This,iImpersonationLevel)

#define ISWbemSecurity_put_ImpersonationLevel(This,iImpersonationLevel)	\
    (This)->lpVtbl -> put_ImpersonationLevel(This,iImpersonationLevel)

#define ISWbemSecurity_get_AuthenticationLevel(This,iAuthenticationLevel)	\
    (This)->lpVtbl -> get_AuthenticationLevel(This,iAuthenticationLevel)

#define ISWbemSecurity_put_AuthenticationLevel(This,iAuthenticationLevel)	\
    (This)->lpVtbl -> put_AuthenticationLevel(This,iAuthenticationLevel)

#define ISWbemSecurity_get_Privileges(This,objWbemPrivilegeSet)	\
    (This)->lpVtbl -> get_Privileges(This,objWbemPrivilegeSet)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemSecurity_get_ImpersonationLevel_Proxy( 
    ISWbemSecurity __RPC_FAR * This,
    /* [retval][out] */ WbemImpersonationLevelEnum __RPC_FAR *iImpersonationLevel);


void __RPC_STUB ISWbemSecurity_get_ImpersonationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISWbemSecurity_put_ImpersonationLevel_Proxy( 
    ISWbemSecurity __RPC_FAR * This,
    /* [in] */ WbemImpersonationLevelEnum iImpersonationLevel);


void __RPC_STUB ISWbemSecurity_put_ImpersonationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemSecurity_get_AuthenticationLevel_Proxy( 
    ISWbemSecurity __RPC_FAR * This,
    /* [retval][out] */ WbemAuthenticationLevelEnum __RPC_FAR *iAuthenticationLevel);


void __RPC_STUB ISWbemSecurity_get_AuthenticationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISWbemSecurity_put_AuthenticationLevel_Proxy( 
    ISWbemSecurity __RPC_FAR * This,
    /* [in] */ WbemAuthenticationLevelEnum iAuthenticationLevel);


void __RPC_STUB ISWbemSecurity_put_AuthenticationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemSecurity_get_Privileges_Proxy( 
    ISWbemSecurity __RPC_FAR * This,
    /* [retval][out] */ ISWbemPrivilegeSet __RPC_FAR *__RPC_FAR *objWbemPrivilegeSet);


void __RPC_STUB ISWbemSecurity_get_Privileges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemSecurity_INTERFACE_DEFINED__ */


#ifndef __ISWbemPrivilege_INTERFACE_DEFINED__
#define __ISWbemPrivilege_INTERFACE_DEFINED__

/* interface ISWbemPrivilege */
/* [helpstring][hidden][nonextensible][dual][oleautomation][uuid][local][object] */ 


EXTERN_C const IID IID_ISWbemPrivilege;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("26EE67BD-5804-11d2-8B4A-00600806D9B6")
    ISWbemPrivilege : public IDispatch
    {
    public:
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_IsEnabled( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsEnabled) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_IsEnabled( 
            /* [in] */ VARIANT_BOOL bIsEnabled) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR __RPC_FAR *strDisplayName) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_DisplayName( 
            /* [retval][out] */ BSTR __RPC_FAR *strDisplayName) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Identifier( 
            /* [retval][out] */ WbemPrivilegeEnum __RPC_FAR *iPrivilege) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemPrivilegeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemPrivilege __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemPrivilege __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemPrivilege __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemPrivilege __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemPrivilege __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemPrivilege __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemPrivilege __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsEnabled )( 
            ISWbemPrivilege __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsEnabled);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_IsEnabled )( 
            ISWbemPrivilege __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bIsEnabled);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            ISWbemPrivilege __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *strDisplayName);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DisplayName )( 
            ISWbemPrivilege __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *strDisplayName);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Identifier )( 
            ISWbemPrivilege __RPC_FAR * This,
            /* [retval][out] */ WbemPrivilegeEnum __RPC_FAR *iPrivilege);
        
        END_INTERFACE
    } ISWbemPrivilegeVtbl;

    interface ISWbemPrivilege
    {
        CONST_VTBL struct ISWbemPrivilegeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemPrivilege_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemPrivilege_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemPrivilege_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemPrivilege_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemPrivilege_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemPrivilege_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemPrivilege_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemPrivilege_get_IsEnabled(This,bIsEnabled)	\
    (This)->lpVtbl -> get_IsEnabled(This,bIsEnabled)

#define ISWbemPrivilege_put_IsEnabled(This,bIsEnabled)	\
    (This)->lpVtbl -> put_IsEnabled(This,bIsEnabled)

#define ISWbemPrivilege_get_Name(This,strDisplayName)	\
    (This)->lpVtbl -> get_Name(This,strDisplayName)

#define ISWbemPrivilege_get_DisplayName(This,strDisplayName)	\
    (This)->lpVtbl -> get_DisplayName(This,strDisplayName)

#define ISWbemPrivilege_get_Identifier(This,iPrivilege)	\
    (This)->lpVtbl -> get_Identifier(This,iPrivilege)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemPrivilege_get_IsEnabled_Proxy( 
    ISWbemPrivilege __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *bIsEnabled);


void __RPC_STUB ISWbemPrivilege_get_IsEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISWbemPrivilege_put_IsEnabled_Proxy( 
    ISWbemPrivilege __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL bIsEnabled);


void __RPC_STUB ISWbemPrivilege_put_IsEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemPrivilege_get_Name_Proxy( 
    ISWbemPrivilege __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *strDisplayName);


void __RPC_STUB ISWbemPrivilege_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemPrivilege_get_DisplayName_Proxy( 
    ISWbemPrivilege __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *strDisplayName);


void __RPC_STUB ISWbemPrivilege_get_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemPrivilege_get_Identifier_Proxy( 
    ISWbemPrivilege __RPC_FAR * This,
    /* [retval][out] */ WbemPrivilegeEnum __RPC_FAR *iPrivilege);


void __RPC_STUB ISWbemPrivilege_get_Identifier_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemPrivilege_INTERFACE_DEFINED__ */


#ifndef __ISWbemPrivilegeSet_INTERFACE_DEFINED__
#define __ISWbemPrivilegeSet_INTERFACE_DEFINED__

/* interface ISWbemPrivilegeSet */
/* [helpstring][nonextensible][hidden][dual][oleautomation][uuid][local][object] */ 


EXTERN_C const IID IID_ISWbemPrivilegeSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("26EE67BF-5804-11d2-8B4A-00600806D9B6")
    ISWbemPrivilegeSet : public IDispatch
    {
    public:
        virtual /* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Item( 
            /* [in] */ WbemPrivilegeEnum iPrivilege,
            /* [retval][out] */ ISWbemPrivilege __RPC_FAR *__RPC_FAR *objWbemPrivilege) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long __RPC_FAR *iCount) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Add( 
            /* [in] */ WbemPrivilegeEnum iPrivilege,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bIsEnabled,
            /* [retval][out] */ ISWbemPrivilege __RPC_FAR *__RPC_FAR *objWbemPrivilege) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( 
            /* [in] */ WbemPrivilegeEnum iPrivilege) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DeleteAll( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddAsString( 
            /* [in] */ BSTR strPrivilege,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bIsEnabled,
            /* [retval][out] */ ISWbemPrivilege __RPC_FAR *__RPC_FAR *objWbemPrivilege) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISWbemPrivilegeSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISWbemPrivilegeSet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISWbemPrivilegeSet __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISWbemPrivilegeSet __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISWbemPrivilegeSet __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISWbemPrivilegeSet __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISWbemPrivilegeSet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISWbemPrivilegeSet __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [restricted][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            ISWbemPrivilegeSet __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Item )( 
            ISWbemPrivilegeSet __RPC_FAR * This,
            /* [in] */ WbemPrivilegeEnum iPrivilege,
            /* [retval][out] */ ISWbemPrivilege __RPC_FAR *__RPC_FAR *objWbemPrivilege);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            ISWbemPrivilegeSet __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *iCount);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Add )( 
            ISWbemPrivilegeSet __RPC_FAR * This,
            /* [in] */ WbemPrivilegeEnum iPrivilege,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bIsEnabled,
            /* [retval][out] */ ISWbemPrivilege __RPC_FAR *__RPC_FAR *objWbemPrivilege);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Remove )( 
            ISWbemPrivilegeSet __RPC_FAR * This,
            /* [in] */ WbemPrivilegeEnum iPrivilege);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteAll )( 
            ISWbemPrivilegeSet __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddAsString )( 
            ISWbemPrivilegeSet __RPC_FAR * This,
            /* [in] */ BSTR strPrivilege,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL bIsEnabled,
            /* [retval][out] */ ISWbemPrivilege __RPC_FAR *__RPC_FAR *objWbemPrivilege);
        
        END_INTERFACE
    } ISWbemPrivilegeSetVtbl;

    interface ISWbemPrivilegeSet
    {
        CONST_VTBL struct ISWbemPrivilegeSetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISWbemPrivilegeSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISWbemPrivilegeSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISWbemPrivilegeSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISWbemPrivilegeSet_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISWbemPrivilegeSet_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISWbemPrivilegeSet_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISWbemPrivilegeSet_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISWbemPrivilegeSet_get__NewEnum(This,pUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,pUnk)

#define ISWbemPrivilegeSet_Item(This,iPrivilege,objWbemPrivilege)	\
    (This)->lpVtbl -> Item(This,iPrivilege,objWbemPrivilege)

#define ISWbemPrivilegeSet_get_Count(This,iCount)	\
    (This)->lpVtbl -> get_Count(This,iCount)

#define ISWbemPrivilegeSet_Add(This,iPrivilege,bIsEnabled,objWbemPrivilege)	\
    (This)->lpVtbl -> Add(This,iPrivilege,bIsEnabled,objWbemPrivilege)

#define ISWbemPrivilegeSet_Remove(This,iPrivilege)	\
    (This)->lpVtbl -> Remove(This,iPrivilege)

#define ISWbemPrivilegeSet_DeleteAll(This)	\
    (This)->lpVtbl -> DeleteAll(This)

#define ISWbemPrivilegeSet_AddAsString(This,strPrivilege,bIsEnabled,objWbemPrivilege)	\
    (This)->lpVtbl -> AddAsString(This,strPrivilege,bIsEnabled,objWbemPrivilege)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemPrivilegeSet_get__NewEnum_Proxy( 
    ISWbemPrivilegeSet __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pUnk);


void __RPC_STUB ISWbemPrivilegeSet_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemPrivilegeSet_Item_Proxy( 
    ISWbemPrivilegeSet __RPC_FAR * This,
    /* [in] */ WbemPrivilegeEnum iPrivilege,
    /* [retval][out] */ ISWbemPrivilege __RPC_FAR *__RPC_FAR *objWbemPrivilege);


void __RPC_STUB ISWbemPrivilegeSet_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE ISWbemPrivilegeSet_get_Count_Proxy( 
    ISWbemPrivilegeSet __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *iCount);


void __RPC_STUB ISWbemPrivilegeSet_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemPrivilegeSet_Add_Proxy( 
    ISWbemPrivilegeSet __RPC_FAR * This,
    /* [in] */ WbemPrivilegeEnum iPrivilege,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bIsEnabled,
    /* [retval][out] */ ISWbemPrivilege __RPC_FAR *__RPC_FAR *objWbemPrivilege);


void __RPC_STUB ISWbemPrivilegeSet_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemPrivilegeSet_Remove_Proxy( 
    ISWbemPrivilegeSet __RPC_FAR * This,
    /* [in] */ WbemPrivilegeEnum iPrivilege);


void __RPC_STUB ISWbemPrivilegeSet_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemPrivilegeSet_DeleteAll_Proxy( 
    ISWbemPrivilegeSet __RPC_FAR * This);


void __RPC_STUB ISWbemPrivilegeSet_DeleteAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISWbemPrivilegeSet_AddAsString_Proxy( 
    ISWbemPrivilegeSet __RPC_FAR * This,
    /* [in] */ BSTR strPrivilege,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL bIsEnabled,
    /* [retval][out] */ ISWbemPrivilege __RPC_FAR *__RPC_FAR *objWbemPrivilege);


void __RPC_STUB ISWbemPrivilegeSet_AddAsString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISWbemPrivilegeSet_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_SWbemLocator;

#ifdef __cplusplus

class DECLSPEC_UUID("76A64158-CB41-11d1-8B02-00600806D9B6")
SWbemLocator;
#endif

EXTERN_C const CLSID CLSID_SWbemNamedValueSet;

#ifdef __cplusplus

class DECLSPEC_UUID("9AED384E-CE8B-11d1-8B05-00600806D9B6")
SWbemNamedValueSet;
#endif

EXTERN_C const CLSID CLSID_SWbemObjectPath;

#ifdef __cplusplus

class DECLSPEC_UUID("5791BC26-CE9C-11d1-97BF-0000F81E849C")
SWbemObjectPath;
#endif

EXTERN_C const CLSID CLSID_SWbemLastError;

#ifdef __cplusplus

class DECLSPEC_UUID("C2FEEEAC-CFCD-11d1-8B05-00600806D9B6")
SWbemLastError;
#endif

EXTERN_C const CLSID CLSID_SWbemSink;

#ifdef __cplusplus

class DECLSPEC_UUID("75718C9A-F029-11d1-A1AC-00C04FB6C223")
SWbemSink;
#endif

EXTERN_C const CLSID CLSID_SWbemServices;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D63-21AE-11d2-8B33-00600806D9B6")
SWbemServices;
#endif

EXTERN_C const CLSID CLSID_SWbemObject;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D62-21AE-11d2-8B33-00600806D9B6")
SWbemObject;
#endif

EXTERN_C const CLSID CLSID_SWbemObjectSet;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D61-21AE-11d2-8B33-00600806D9B6")
SWbemObjectSet;
#endif

EXTERN_C const CLSID CLSID_SWbemNamedValue;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D60-21AE-11d2-8B33-00600806D9B6")
SWbemNamedValue;
#endif

EXTERN_C const CLSID CLSID_SWbemQualifier;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D5F-21AE-11d2-8B33-00600806D9B6")
SWbemQualifier;
#endif

EXTERN_C const CLSID CLSID_SWbemQualifierSet;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D5E-21AE-11d2-8B33-00600806D9B6")
SWbemQualifierSet;
#endif

EXTERN_C const CLSID CLSID_SWbemProperty;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D5D-21AE-11d2-8B33-00600806D9B6")
SWbemProperty;
#endif

EXTERN_C const CLSID CLSID_SWbemPropertySet;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D5C-21AE-11d2-8B33-00600806D9B6")
SWbemPropertySet;
#endif

EXTERN_C const CLSID CLSID_SWbemMethod;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D5B-21AE-11d2-8B33-00600806D9B6")
SWbemMethod;
#endif

EXTERN_C const CLSID CLSID_SWbemMethodSet;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D5A-21AE-11d2-8B33-00600806D9B6")
SWbemMethodSet;
#endif

EXTERN_C const CLSID CLSID_SWbemEventSource;

#ifdef __cplusplus

class DECLSPEC_UUID("04B83D58-21AE-11d2-8B33-00600806D9B6")
SWbemEventSource;
#endif

EXTERN_C const CLSID CLSID_SWbemSecurity;

#ifdef __cplusplus

class DECLSPEC_UUID("B54D66E9-2287-11d2-8B33-00600806D9B6")
SWbemSecurity;
#endif

EXTERN_C const CLSID CLSID_SWbemPrivilege;

#ifdef __cplusplus

class DECLSPEC_UUID("26EE67BC-5804-11d2-8B4A-00600806D9B6")
SWbemPrivilege;
#endif

EXTERN_C const CLSID CLSID_SWbemPrivilegeSet;

#ifdef __cplusplus

class DECLSPEC_UUID("26EE67BE-5804-11d2-8B4A-00600806D9B6")
SWbemPrivilegeSet;
#endif
#endif /* __WbemScripting_LIBRARY_DEFINED__ */

/* interface __MIDL_itf_wbemdisp_0265 */
/* [local] */ 

#define	WBEMS_DISPID_OBJECT_READY	( 1 )

#define	WBEMS_DISPID_COMPLETED	( 2 )

#define	WBEMS_DISPID_PROGRESS	( 3 )

#define	WBEMS_DISPID_OBJECT_PUT	( 4 )



extern RPC_IF_HANDLE __MIDL_itf_wbemdisp_0265_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wbemdisp_0265_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



