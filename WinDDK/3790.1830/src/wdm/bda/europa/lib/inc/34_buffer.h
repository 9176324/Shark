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
// @module   Buffer | Buffer handling for video, VBI and audio streams.
//           The buffer classes will be build by the driver and passed to the HAL.
//           The buffer may be already allocated. If not, the HAL will allocate it.

// 
// Filename: 34_Buffer.h
// 
// Routines/Functions:
//
//  public:
//			CVampBuffer
//			~CVampBuffer
//			SetContext
//			GetContext
//			GetStatus
//			GetFormat
//			GetKMBufferObject
//          GetKMContext
//          GetObjectStatus
//
//  private:
//          SetKMContext
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _BUFFER__
#define _BUFFER__

#include "34api.h"
#include "34_error.h"
#include "34_types.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampBuffer | This class is a repository of information about a single 
// unit of transfer. A buffer object can be initialized to use a previously 
// allocated buffer, a user mode buffer or non-paged kernel memory  can be allocated 
// by the  buffer object itself (using the OS specific memory management services 
// described in COSDepend..). 
// To allow the core stream class to correctly process buffer completions 
// without the intervention of derived classes, the buffer will maintain a  
// property describing its type , from the following list:<nl> 
// 
//      Odd	    -   To contain an odd field only<nl>
//      Even    -   Contains an even field only<nl>
//      Either  -   Not field sensitive - contains one buffer<nl>
//      Both    -   odd first	Contains two fields interlaced into a single<nl>   
//                  frame, with the odd field first<nl>
//      Both    -   even first	Contains two interlaced fields, but with the<nl>   
//                  even field first.<nl>  

//
//////////////////////////////////////////////////////////////////////////////

class P34_API CVampBuffer 
{
//@access Public
public:
    //@access Public functions
    //@cmember Constructor which gets a buffer<nl>
    //Parameterlist<nl>
    //PVOID pBufferAddress // Pointer to transmitted buffer<nl>
    //size_t NrOfBytes // Size of buffer in bytes<nl>
    //tBufferFormat* ptBufferFormat // Pointer to Buffer format structure<nl>
    CVampBuffer (
        PVOID pBufferAddress,
        size_t NrOfBytes,
        tBufferFormat* ptBufferFormat,
		BOOL bContiguousMemoryBuffer = FALSE);
    //@cmember Destructor<nl>
	virtual ~CVampBuffer();
    //@cmember Sets pointer on a driver / user context<nl>
    //Parameterlist<nl>
    //PVOID pDevContext // Sets pointer on a driver / user context<nl>
    VOID SetContext( PVOID pDevContext ); 
    //@cmember Returns a pointer on a driver / user context<nl>
    PVOID GetContext();
    //@cmember Gets the buffer status attributes<nl>
    tBufferStatus GetStatus();
    //@cmember Gets the buffer format attributes<nl>
    //Parameterlist<nl>
    //tBufferFormat* ptBufferFormat // Pointer to Buffer format structure<nl>
    VOID GetFormat(tBufferFormat* ptBufferFormat);
    //KM Buffer object<nl>
    PVOID GetKMBufferObject();	
    //@cmember Returns a pointer on a driver / user context<nl>
    static PVOID GetKMContext( PVOID pBuffer);
    // @cmember Reads and returns the current status of this object
    DWORD GetObjectStatus();
//@access Private
private:
    //@access Private variables
    //@cmember Pointer to corresponding KM Buffer object<nl>
    PVOID m_pKMBuffer;
    //@cmember Pointer on a UM driver / user context<nl>
    PVOID m_pContext;
    // @cmember Represents the current status of this object
    DWORD m_dwObjectStatus;
    //@access Private functions
    //@cmember Sets pointer on a KM driver / user context<nl>
    //Parameterlist<nl>
    //PVOID pDevContext // Sets pointer on a KM driver / user context<nl>
    VOID SetKMContext( PVOID pDevContext ); 
};

#endif // _BUFFER__
