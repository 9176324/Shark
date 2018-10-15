//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 1999
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
// @module   VampError | Return types for errors, warnings and success.<nl>
// @end
// Filename: VampError.h
// 
// Routines/Functions:
//
//  public:
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(VAMPERROR_H_INCLUDED_)
#define VAMPERROR_H_INCLUDED_

//@type VAMPRET | Common status return type (LONG) for most of the HAL methods.
//  32 bit values of the following layout:<nl>
//<nl>
//  31 30 29 28             16 15        0<nl>
//  +----+---+--------------+---------+<nl>
//  Sev    C     Facility         Code <nl>
//  +----+---+--------------+---------+<nl>
//<nl>
//      Sev:		severity code<nl>
//      <tab>00 - Success<nl>
//      <tab>01 - Informational<nl>
//      <tab>10 - Warning<nl>
//      <tab>11 - Error<nl>
//      C:			customer code flag<nl>
//      Facility:	facility code<nl>
//      Code:		facility's status code<nl>
//<nl>
typedef long VAMPRET;

//@type PVAMPRET | Pointer to VAMPRET.
typedef VAMPRET *PVAMPRET;

#define VampRet VAMPRET 
//@defitem VampRet | defines VampRet to VAMPRET

#define VAMP_SUCCESS(Status)                    ((VAMPRET)(Status) >= 0)
//@defitem VAMP_SUCCESS | macro to evaluate the return value

#define VAMP_ERROR(Status)                      ((ULONG)(Status) >> 30 == 3)
//@defitem VAMP_ERROR | macro to evaluate the return value

#define VAMP_INFORMATION(Status)                ((ULONG)(Status) >> 30 == 1)
//@defitem VAMP_INFORMATION | macro to evaluate the return value

#define VAMP_WARNING(Status)                    ((ULONG)(Status) >> 30 == 2)
//@defitem VAMP_WARNING | macro to evaluate the return value

#define VAMPRET_SUCCESS                         ((VAMPRET)0x00000000L) 
//@defitem VAMPRET_SUCCESS | Success definition

#define VAMPRET_GENERAL_ERROR	                ((VAMPRET)0xFFFFFFFFL)
//@defitem VAMPRET_GENERAL_ERROR | Generic error

#define VAMPRET_FAIL                            VAMPRET_GENERAL_ERROR
//@defitem VAMPRET_FAIL | General failure definition

#define VAMPRET_DPC_REQUESTED                   ((VAMPRET)0x60000101L) 
//@defitem VAMPRET_DPC_REQUESTED | Information: Returned by ISR requesting a DPC

#define VAMPRET_NOT_SUPPORTED                   ((VAMPRET)0xE0000001L) 
//@defitem VAMPRET_NOT_SUPPORTED | Error: Function not supported

#define VAMPRET_NULL_POINTER_PARAMETER          ((VAMPRET)0xE0000002L) 
//@defitem VAMPRET_NULL_POINTER_PARAMETER | Error: NULL pointer submitted to function as output parameter

#define VAMPRET_ADDRESS_OUT_OF_RANGE	        ((VAMPRET)0xE0000100L) 
//@defitem VAMPRET_ADDRESS_OUT_OF_RANGE | Error: Address value is not in range

#define VAMPRET_REGISTRY_IO_FAILED	            ((VAMPRET)0xE000014DL)
//@defitem VAMPRET_REGISTRY_IO_FAILED | Error: Registry is not accessable

#define VAMPRET_GPIO_PIN_IN_USE		            ((VAMPRET)0xE0000200L) 
//@defitem VAMPRET_GPIO_PIN_IN_USE | Error: GPIO pin already used

#define VAMPRET_GPIO_CONFIG_ERR		            ((VAMPRET)0xE0000201L) 
//@defitem VAMPRET_GPIO_CONFIG_ERR | Error: GPIO configuration error

#define VAMPRET_ILLEGAL_FRAMERATE               ((VAMPRET)0xE0000101)
//@defitem VAMPRET_ILLEGAL_FRAMERATE | Error: Frame rate not possible

#define VAMPRET_STREAM_START_ERROR              ((VAMPRET)0xE0000102)
//@defitem VAMPRET_STREAM_START_ERROR | Error: Stream cannot start

#define VAMPRET_ILLEGAL_FIFO_SIZE               ((VAMPRET)0xE0000103)
//@defitem VAMPRET_ILLEGAL_FIFO_SIZE | Error: RAM area size is not possible

#define VAMPRET_BUFFER_SIZE                     ((VAMPRET)0xE0000104)
//@defitem VAMPRET_BUFFER_SIZE | Error: Buffer size is not correct

#define VAMPRET_BUFFER_IN_PROGRESS              ((VAMPRET)0xE0000105)
//@defitem VAMPRET_BUFFER_IN_PROGRESS | Error: The requested buffer is currently being filled

#define VAMPRET_NO_BUFFER_AVAILABLE             ((VAMPRET)0xE0000106)
//@defitem VAMPRET_NO_BUFFER_AVAILABLE | Error: No buffer available; empty list or out of memory

#define VAMPRET_BUFFER_QUEUE_CORRUPT            ((VAMPRET)0xE0000107) 
//@defitem VAMPRET_BUFFER_QUEUE_CORRUPT | Error: Buffer queue is corrupt 

#define VAMPRET_NO_DMA_CHANNEL                  ((VAMPRET)0x60000108) 
//@defitem VAMPRET_NO_DMA_CHANNEL | Error: No DMA channel available 

#define VAMPRET_ODD_EVEN_MISMATCH               ((VAMPRET)0xE0000109) 
//@defitem VAMPRET_ODD_EVEN_MISMATCH | Error: Mismatch of field type and buffer type

#define VAMPRET_ILLEGAL_FIFO_THRESHOLD          ((VAMPRET)0xE000010A) 
//@defitem VAMPRET_ILLEGAL_FIFO_THRESHOLD | Error: Illegal threshold value for FIFOs

#define VAMPRET_ILLEGAL_FIFO_NUMBER             ((VAMPRET)0xE000010B) 
//@defitem VAMPRET_ILLEGAL_FIFO_NUMBER | Error: Illegal FIFO parameter

#define VAMPRET_ILLEGAL_STATUS_TYPE             ((VAMPRET)0xE000010C) 
//@defitem VAMPRET_ILLEGAL_STATUS_TYPE | Error: Illegal DMA status 

#define VAMPRET_ILLEGAL_BUFFER_TYPE             ((VAMPRET)0xE000010D) 
//@defitem VAMPRET_ILLEGAL_BUFFER_TYPE | Error: Illegal buffer type

#define VAMPRET_INVALID_TIMESTAMP               ((VAMPRET)0xE000010E) 
//@defitem VAMPRET_INVALID_TIMESTAMP | Error: Time stamp is invalid

#define VAMPRET_INVALID_EVENT                   ((VAMPRET)0xE000010F) 
//@defitem VAMPRET_INVALID_EVENT | Error: Wrong event type

#define VAMPRET_SCALER_ERROR                    ((VAMPRET)0xE0000110) 
//@defitem VAMPRET_SCALER_ERROR | Error: Invalid scaler parameter

#define VAMPRET_ILLEGAL_SYNC_START              ((VAMPRET)0xE0000111) 
//@defitem VAMPRET_ILLEGAL_SYNC_START | Error: Invalid sync start parameter

#define VAMPRET_ILLEGAL_SYNC_STOP               ((VAMPRET)0xE0000112) 
//@defitem VAMPRET_ILLEGAL_SYNC_STOP | Error: Invalid sync stop parameter

#define VAMPRET_ILLEGAL_VGATE_START             ((VAMPRET)0xE0000113) 
//@defitem VAMPRET_ILLEGAL_VGATE_START | Error: Invalid V-Gate start parameter

#define VAMPRET_ILLEGAL_VGATE_STOP              ((VAMPRET)0xE0000114) 
//@defitem VAMPRET_ILLEGAL_VGATE_STOP | Error: Invalid V-Gate stop parameter

#define VAMPRET_CLIPPER_ERROR                   ((VAMPRET)0xE0000115)
//@defitem VAMPRET_CLIPPER_ERROR | Error: Invalid clipper parameter

#define VAMPRET_TIMEOUT                         ((VAMPRET)0xE0000116)
//@defitem VAMPRET_TIMEOUT | Error: Timeout occurred

#define VAMPRET_FIFO_ACTIVE                     ((VAMPRET)0xE0000117)
//@defitem VAMPRET_FIFO_ACTIVE | Error: FIFO is active

#define VAMPRET_CLIPPER_NOT_AVAILABLE           ((VAMPRET)0xE0000118)
//@defitem VAMPRET_CLIPPER_NOT_AVAILABLE | Error: Clipper not available

#define VAMPRET_OVERLAPPING_INPUT_WINDOWS       ((VAMPRET)0xE0000119)
//@defitem VAMPRET_OVERLAPPING_INPUT_WINDOWS | Error: Overlapping or illegal input windows of VBI and Video

#define VAMPRET_ALLOCATION_ERROR                ((VAMPRET)0xE000011A)
//@defitem VAMPRET_ALLOCATION_ERROR | Error occurred in object or memory allocation

// defines for the VAMPRET
#define VAMPRET_ADDRESS_NOT_MAPPED	            ((VAMPRET)0xE000011BL) 
//@defitem VAMPRET_ADDRESS_NOT_MAPPED | Error: Address is not mapped to an object

#define VAMPRET_INVALID_PARAMETER               ((VAMPRET)0xE000011CL)
//@defitem VAMPRET_INVALID_PARAMETER | Error: One or more calling parameters are wrong

#define VAMPRET_PLANAR_BUFFER_MISMATCH          ((VAMPRET)0xE000011D) 
//@defitem VAMPRET_PLANAR_BUFFER_MISMATCH | Error: Planar buffer parts do not fit together 

#define VAMPRET_SCALER_SIZE_EXCEEDED            ((VAMPRET)0xE000011E) 
//@defitem VAMPRET_SCALER_SIZE_EXCEEDED | Error: Scaler output window is too large 

#define VAMPRET_STREAM_RELEASE_ERROR            ((VAMPRET)0xE000011F) 
//@defitem VAMPRET_STREAM_RELEASE_ERROR  | Error: Stream could not be released

// Vamp returns ( errors ) concerned with interrupts
#define VAMPRET_NOT_A_VAMP_INTERRUPT            ((VAMPRET)0xE0000400)
//@defitem VAMPRET_NOT_A_VAMP_INTERRUPT | Error: Interrupt is not from our device

#define VAMPRET_UNKNOWN_INTERRUPT               ((VAMPRET)0xE0000401)
//@defitem VAMPRET_UNKNOWN_INTERRUPT | Error: Hardware interrupt is unknown

#define VAMPRET_NO_FIFO_CALLBACK_MAPPED         ((VAMPRET)0xE0000402)
//@defitem VAMPRET_NO_FIFO_CALLBACK_MAPPED | Error: No ISR callback mapped for this interrupt

#define VAMPRET_WRONG_STREAM_FOR_THIS_FIFO      ((VAMPRET)0xE0000403)
//@defitem VAMPRET_WRONG_STREAM_FOR_THIS_FIFO | Error: Stream cannot be used in this FIFO area

#define VAMPRET_UNKNOWN_EDGE_TRIGGER            ((VAMPRET)0xE0000404)
//@defitem VAMPRET_UNKNOWN_EDGE_TRIGGER | Error: Unknown edge type to trigger 

#define VAMPRET_NO_CALLBACK_MAPPED              ((VAMPRET)0xE0000405)
//@defitem VAMPRET_NO_CALLBACK_MAPPED | Error: No ISR callback mapped for this interrupt

// Temporary define to indicate that something hasn't yet been implemented
#define VAMPRET_NOT_YET_IMPLEMENTED             ((VAMPRET)0xE0000500)
//@defitem VAMPRET_NOT_YET_IMPLEMENTED | Error: Hasn't yet been implemented       

#define VAMPRET_DEVICE_NOT_AVAILABLE            ((VAMPRET)0xE0000600)
//@defitem VAMPRET_DEVICE_NOT_AVAILABLE | Error: Device index references a device that was not detected

#define VAMPRET_CALLBACK_ALREADY_MAPPED	        ((VAMPRET)0x80000100L)
//@defitem VAMPRET_CALLBACK_ALREADY_MAPPED | Warning: Callback is already mapped

#define VAMPRET_NO_FURTHER_CALLBACKS_AVAILABLE  ((VAMPRET)0x60000406)
//@defitem VAMPRET_NO_FURTHER_CALLBACKS_AVAILABLE | Warning: GetNextCallback returns this if the last Callback was taken out of the list

#define VAMPRET_FURTHER_CALLBACKS_AVAILABLE     ((VAMPRET)0x60000407)
//@defitem VAMPRET_FURTHER_CALLBACKS_AVAILABLE | Warning: GetNextCallback returns this if the list contains further callbacks

#define VAMPRET_WRONG_CALLING_ORDER             ((VAMPRET)0x60000408)
//@defitem VAMPRET_FUNCTION_ALREADY_CALLED | Warning: The method has been called in a wrong order

 
#endif // !defined(VAMPERROR_H_INCLUDED_)
