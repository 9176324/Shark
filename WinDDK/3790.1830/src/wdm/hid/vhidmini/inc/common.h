
//
// Custom control codes are defined here. They are to be used for sideband 
// communication with the hid minidriver. These control codes are sent to 
// the hid minidriver using Hid_GetFeature() API to a custom collection 
// defined especially to handle such requests.
//
#define  HIDMINI_CONTROL_CODE_GET_ATTRIBUTES              0x00
#define  HIDMINI_CONTROL_CODE_DUMMY1                             0x01
#define  HIDMINI_CONTROL_CODE_DUMMY2                             0x02


//
// This is the report id of the collection to which the control codes are sent
//
#define CONTROL_COLLECTION_REPORT_ID                              0x01

#include <pshpack1.h>

typedef struct _HIDMINI_CONTROL_INFO {

    //report ID of the collection to which the control request is sent

    UCHAR    ReportId;   

    // One byte control code (user-defined) for communication with hid 
    // mini driver
    
    UCHAR   ControlCode;

    // This is a placeholder for address of the buffer allocated by the caller
    // equal to the size of ControlBufferLength.

    UCHAR   ControlBuffer[];
    
} HIDMINI_CONTROL_INFO, * PHIDMINI_CONTROL_INFO;

#include <poppack.h>




