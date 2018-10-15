//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2000
//  All rights are reserved. Reproduction in whole or in part is prohibited
//  without the written consent of the copyright owner.
//
//  Philips reserves the right to make changes without notice at any time.
//  Philips makes no warranty, expressed, implied or statutory, including but
//  not limited to any implied warranty of merchantability or fitness for any
//  particular purpose, or that the use will not infringe any third party
//  patent, copyright or trademark. Philips must not be liable for any loss
//  or damage arising from its use.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// @doc      INTERNAL EXTERNAL
// @module   AbstractStream | This module contains pure virtual
//           declarations of the basic stream methods. 
// @end
// Filename: AbstractStream.h
//
// Routines/Functions:
//
//  public:
//          CVampBaseCoreStream
//          ~CVampBaseCoreStream
//          AddBuffer
//          GetNextDoneBuffer
//          ReleaseLastQueuedBuffer
//          RemoveBuffer
//          CancelAllBuffers
//          Open ()
//          Close ()
//
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////

#if !defined(_ABSTRACTSTREAM_H_)
#define _ABSTRACTSTREAM_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "OSDepend.h"
#include "VampTypes.h"

//autoduck doesn't like this PURE macro
#if !defined PURE
#define PURE                    = 0
#endif

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CCallOnDPC | Callback class.
// @end
//////////////////////////////////////////////////////////////////////////////
class CCallOnDPC 
{
//@access public
public:
//@cmember Pure definition of callback method. The callback object contains
//         notifcation methods for completed buffers. The object, which needs
//         a notification,  has to be derived from this class. It has to
//         implement the CallOnDPC method. <nl>
    //Parameterlist:<nl>
    //PVOID pStream // pointer to stream<nl>
    //PVOID pParam1 // pointer to first parameter<nl>
    //PVOID pParam2 // pointer to second parameter<nl>
    virtual VAMPRET CallOnDPC ( 
        PVOID pStream, 
        PVOID pParam1, 
        PVOID pParam2 ) = NULL;
};


//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CAbstractDPC | DPC handling.
// @base public | CCallOnDPC
// @end
//////////////////////////////////////////////////////////////////////////////

class CAbstractDPC : public CCallOnDPC
{
   	//@access protected 
protected:

    //@cmember pointer to DPC callback class<nl>
    CCallOnDPC *m_pCBOnDPC;

    //@cmember First Callback parameter for CallOnDPC<nl>
    PVOID pCallOnDPCParam1;
    //@cmember Second Callback parameter for CallOnDPC<nl>
    PVOID pCallOnDPCParam2;

   	//@access public 
public:
    //@cmember | CAbstractDPC | () | Constructor. Initializes members to NULL.<nl>
    CAbstractDPC() : 
        m_pCBOnDPC (NULL),
        pCallOnDPCParam1 (NULL),
        pCallOnDPCParam2 (NULL) 
    {};

    //@cmember Destructor.<nl>
    virtual ~CAbstractDPC() {};

    //@cmember Pure definition; This method gives the driver (or whoever) the
    //         opportunity to add a callback function to the core stream OnDPC
    //         method. To remove  the Callback, call this method with a NULL
    //         pointer.<nl>  It returns VAMPRET_SUCCESS or a warning
    //         VAMPRET_CALLBACK_ALREADY_MAPPED if a callback is already mapped
    //         to the OnCompletion method.
    //Parameterlist:<nl>
    //CCallOnDPC *pCBOnDPC // CCallOnDPC object pointer<nl>
    //PVOID pParam1 // pointer to first parameter<nl>
    //PVOID pParam2 // pointer to second parameter<nl>
    virtual VAMPRET AddCBOnDPC(
        CCallOnDPC * pCBOnDPC, 
        PVOID pParam1, 
        PVOID pParam2 ) = NULL;

};

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CAbstractCoreStream | Buffer handling and stream methods regarding
//         all core streams.
// @end
//////////////////////////////////////////////////////////////////////////////

class CAbstractCoreStream
{
   	//@access protected 
protected:

    //@cmember Pointer to COSDependTimeStamp object.<nl>
    COSDependTimeStamp *m_pClock;

    //@access public 
public:
    //@cmember | CAbstractCoreStream | () | Constructor.<nl>
    CAbstractCoreStream () : 
        m_pClock (NULL) {};

    //@cmember Pure definition. Add a new buffer to the core stream input queue.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *Buffer // pointer on buffer object to add<nl>
    virtual  VAMPRET AddBuffer(
            IN CVampBuffer *Buffer ) = NULL;

    //@cmember  Pure definition. Removes a buffer from the core stream empty or 
    //done list.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *Buffer // pointer on buffer object to remove<nl>
    virtual VAMPRET RemoveBuffer(
            IN CVampBuffer *Buffer ) = NULL;

    //@cmember Pure definition. Returns a done buffer from the core stream queue,
    //or NULL if the list is empty<nl>
    //Parameterlist:<nl>
    //CVampBuffer **pBuffer // buffer object to get<nl>
    virtual VAMPRET GetNextDoneBuffer(
            IN OUT CVampBuffer **pBuffer ) = NULL;

    //@cmember Pure definition. Releases the last buffer from the input queue.
    //         This is the buffer which was added last. Returns
    //         VAMPRET_BUFFER_IN_PROGRESS if the buffer is currently beeing
    //         filled or VAMPRET_BUFFER_NOT_AVAILABLE if the list is empty.
    virtual VAMPRET ReleaseLastQueuedBuffer() = NULL;

    //@cmember Pure definition. Moves all buffers from the empty list to the 
    //done list and marks the buffer with the status cancelled.
    virtual VAMPRET CancelAllBuffers() = NULL;


    //@cmember Pure definition. Adjusts the clock, since between open and start
   // might be a big time difference. 
    virtual VOID AdjustClock ( ) = NULL;
};


//////////////////////////////////////////////////////////////////////////////
//
// @doc      INTERNAL EXTERNAL
// @class    CVampAbstract | This class is a pure virtual class. It extends
//           CAbstractCoreStream with some methods which are needed only in
//           core  stream layers.
// @base public | CAbstractCoreStream
// @base public | CAbstractDPC
// @end
//
//////////////////////////////////////////////////////////////////////////////
class CVampAbstract : public CAbstractCoreStream, public CAbstractDPC
{
	//@access public
public:
	//@cmember Sets a pointer to a COSDependTimeStamp object. <nl>
    //Parameterlist:<nl>
    //COSDependTimeStamp *pClock // COSDependTimeStamp pointer<nl>
	virtual VOID SetClock( 
        COSDependTimeStamp* pClock ) = NULL;
};

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CAbstractStream | Buffer handling and stream methods regarding
//         the different stream types, eg. Audio, Video, VBI etc.
// @base public | CAbstractCoreStream
// @end
//////////////////////////////////////////////////////////////////////////////
class CAbstractStream : public CAbstractCoreStream
{
   	//@access public 
public:
    //@cmember Pure definition. Opens a stream.<nl>
    //Parameterlist:<nl>
    //tStreamParams *p_Params // Pointer to stream parameters<nl>
    virtual  VAMPRET Open( 
        tStreamParms *pStreamParms ) = NULL;

    //@cmember Pure definition. Closes a Stream.<nl>
    virtual  VAMPRET Close() = NULL;

    //@cmember Pure definition. Starts a Stream.<nl>.
    virtual VAMPRET Start() = NULL;

    //@cmember Pure definition. Stops a Stream.
    //Parameterlist:<nl>
    //BOOL bRelease // if TRUE: release channels<nl>
    virtual   VAMPRET Stop(
        IN BOOL bRelease ) = NULL;
};

#endif 
