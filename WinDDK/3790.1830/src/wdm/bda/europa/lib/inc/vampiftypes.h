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
// @module   VampIfTypes | This file contains common defines, enumeration types
// and type definition which are available for the HAL and the user interface.  
// It is the central header file for all interface modules.
// Enumerated types are identified by the 'e' prefix. Typedefs are preceeded by
// the prefix 't'.<nl>
// @end
// Filename: VampIfTypes.h
//
//
//////////////////////////////////////////////////////////////////////////////

// VampTypes.h: Vamp type definitions
//
//////////////////////////////////////////////////////////////////////
#if !defined(VAMPIFTYPES_H_INCLUDED_)
#define VAMPIFTYPES_H_INCLUDED_

// @head3 Common definitions | Defines for the HAL and the user interface.

// @head4 Device PCI config space definition |
#define PCI_CONFIG_MAX_OFFSET   0x48        //@defitem PCI_CONFIG_MAX_OFFSET | maximum offset of SAA713x PCI configuration space

// @head4 Device definition flags |
#define DEVICE_SAA7130      0x7130          //@defitem DEVICE_SAA7130 | Philips SAA7130 board (no audio)
#define DEVICE_SAA7134      0x7134          //@defitem DEVICE_SAA7134 | Philips SAA7134 board

#define MAX_NUM_OF_DEVICES 4                //@defitem MAX_NUM_OF_DEVICES | Maximum number of installable devices

// @head4 Capabilities flags of the devices |
#define DEV_ALL             0x0000000F      //@defitem DEV_ALL | all kinds of streams supported
#define DEV_VIDEO           0x00000001      //@defitem DEV_VIDEO | Video streams supported
#define DEV_VBI             0x00000002      //@defitem DEV_VBI | VBI stream supported
#define DEV_TRANSPORT       0x00000004      //@defitem DEV_TRANSPORT | Transport stream supported
#define DEV_AUDIO           0x00000008      //@defitem DEV_AUDIO | Audio stream supported

// @head4 Error status flags of created objects |
#define STATUS_OK           0x00000000      //@defitem STATUS_OK | object has been created without errors
#define CONSTRUCTOR_FAILED  0x00000001      //@defitem CONSTRUCTOR_FAILED | allocation error in constructor

// @head4 Driver flags for the stream manager |
#define S_MANAGER_FLAGS         0xFFFFFFFF  //@defitem S_MANAGER_FLAGS | mask for valid stream manager flags
#define S_FRAME_RATE_REDUCTION  0x00000001  //@defitem S_FRAME_RATE_REDUCTION | the frame rate may be reduced to start the task
#define S_ODD_EVEN_DONT_CARE    0x00000002  //@defitem S_ODD_EVEN_DONT_CARE | the requested field type may be changed
#define S_CREATE_TIMESTAMP      0x00000004  //@defitem S_CREATE_TIMESTAMP | HAL must provide video clock for SW framerate reduction
#define S_FORCE_TASKB           0x00000008  //@defitem S_FORCE_TASKB | force HAL to use Task B (scaler page 2) for video stream

// @head4 Event handler definitions |
#define eEventType                  eInterruptEnable        //@defitem eEventType | event type

typedef DWORD tNotifyOnEvents;      //#!# @defitem tNotifyOnEvents | Enables DPC notification on interrupts

// @head4 Event (interrupt) notification flags for ... |
#define NOTIFY_ON_GPIO_23_NEG       (1<<EN_GPIO_23_NEG)     //@defitem NOTIFY_ON_GPIO_23_NEG | ...falling vertical edge GPIO i/p 23
#define NOTIFY_ON_GPIO_23_POS       (1<<EN_GPIO_23_POS)     //@defitem NOTIFY_ON_GPIO_23_POS | ...rising vertical edge GPIO i/p 23
#define NOTIFY_ON_GPIO_22_NEG       (1<<EN_GPIO_22_NEG)     //@defitem NOTIFY_ON_GPIO_22_NEG | ...falling vertical edge GPIO i/p 22
#define NOTIFY_ON_GPIO_22_POS       (1<<EN_GPIO_22_POS)     //@defitem NOTIFY_ON_GPIO_22_POS | ...rising vertical edge GPIO i/p 22
#define NOTIFY_ON_GPIO_18_NEG       (1<<EN_GPIO_18_NEG)     //@defitem NOTIFY_ON_GPIO_18_NEG | ...falling vertical edge GPIO i/p 18
#define NOTIFY_ON_GPIO_18_POS       (1<<EN_GPIO_18_POS)     //@defitem NOTIFY_ON_GPIO_18_POS | ...rising vertical edge GPIO i/p 18
#define NOTIFY_ON_GPIO_16_NEG       (1<<EN_GPIO_16_NEG)     //@defitem NOTIFY_ON_GPIO_16_NEG | ...falling vertical edge GPIO i/p 16
#define NOTIFY_ON_GPIO_16_POS       (1<<EN_GPIO_16_POS)     //@defitem NOTIFY_ON_GPIO_16_POS | ...rising vertical edge GPIO i/p 16
#define NOTIFY_ON_LOAD_ERROR        (1<<EN_LOAD_ERROR)      //@defitem NOTIFY_ON_LOAD_ERROR | ...load error occured in scaler
#define NOTIFY_ON_CONFIG_ERROR      (1<<EN_CONFIG_ERROR)    //@defitem NOTIFY_ON_CONFIG_ERROR | ...config error occured in scaler
#define NOTIFY_ON_TRIGGER_ERROR     (1<<EN_TRIGGER_ERROR)   //@defitem NOTIFY_ON_TRIGGER_ERROR | ...trigger error occured in scaler
#define NOTIFY_ON_MV_MODE_CHANGE    (1<<EN_MV_MODE_CHANGE)  //@defitem NOTIFY_ON_MV_MODE_CHANGE | ...Macrovision mode changed
#define NOTIFY_ON_FIDT_MODE_CHANGE  (1<<EN_FIDT_MODE_CHANGE)//@defitem NOTIFY_ON_FIDT_MODE_CHANGE | ...50/60Hz mode changed
#define NOTIFY_ON_INTL_MODE_CHANGE  (1<<EN_INTL_MODE_CHANGE)//@defitem NOTIFY_ON_INTL_MODE_CHANGE | ...decoder interlaced mode detection
#define NOTIFY_ON_READY_CAPTURE     (1<<EN_READY_CAPTURE)   //@defitem NOTIFY_ON_READY_CAPTURE | ...decoder ready for capture
#define NOTIFY_ON_POWER_ON_POS      (1<<EN_POWER_ON_POS)    //@defitem NOTIFY_ON_POWER_ON_POS | ...power on reset on rising vertical edge
#define NOTIFY_ON_PARITY_ERROR      (1<<EN_PARITY_ERROR)    //@defitem NOTIFY_ON_PARITY_ERROR | ...PCI parity error
#define NOTIFY_ON_ABORT_RECEIVED    (1<<EN_ABORT_RECEIVED)  //@defitem NOTIFY_ON_ABORT_RECEIVED | ...PCI abort received
#define NOTIFY_ON_STREAM            (1<<EN_STREAM)          //@defitem NOTIFY_ON_STREAM | ...stream interrupts
#define NOTIFY_ON_ALL               0xffffffff              //@defitem NOTIFY_ON_ALL | ...all interrupts

// @head4 GPIO defines |
#define MAX_GPIO_PINS               27                      //@defitem MAX_GPIO_PINS | Maximum number of GPIO pins = 27. Ignore the I2C pins,
                                                            // the Reset pin (GPIO Nr. 28) and the video clock pin (GPIO Nr. 24).

// @head4 Output format defines |
#define MAX_FORMATS                 12                      //@defitem MAX_FORMATS | Maximum number of formats

// @head4 Scaler output window size limits |
#define MAX_PIX_SINGLE      500000      //@defitem MAX_PIX_SINGLE | maximum output pixels for single field
#define MAX_PIX_INTL        1000000     //@defitem MAX_PIX_INTL | maximum output pixels for interlaced field
#define MAX_PIX60_SINGLE    400000      //@defitem MAX_PIX60_SINGLE | maximum output pixels for 60Hz single field
#define MAX_PIX60_INTL      900000      //@defitem MAX_PIX60_INTL | maximum output pixels for 60Hz interlaced field

//@head3  Enumerations for Power control |
enum ePowerControl {            //@enum Power control modes
    EnableVideoPLL   = 0,       //@emem enables the Video PLL
    EnableAudioPLL,             //@emem enables the Audio PLL
    EnableOscillator,           //@emem enables the Oscillator
    EnableVideoFrontend1,       //@emem enables the Video Frontend 1
    EnableVideoFrontend2,       //@emem enables the Video Frontend 2
    EnableSifFrontEnd,          //@emem enables the Audio/SIF Frontend
    EnableBaseDacAdc            //@emem enables basis band DAC adn ADC
};

enum eVampPowerState {              //@enum Power mode states
    VampPowerDeviceUnspecified = 0, //@emem power mode ist unspecific
    VampPowerDeviceD0,              //@emem power is completly on
    VampPowerDeviceD1,              //@emem power partially off
    VampPowerDeviceD2,              //@emem power mostly off, no HW access
    VampPowerDeviceD3,              //@emem power is completly off
    VampPowerDeviceMaximum          //@emem maximum power state
};

//@head3 Enumerations for Clipper and Scaler |
enum eClipMode {                //@enum Clipping modes
    CLIP_NONE   = 0,            //@emem clipping disabled
    CLIP_NORMAL = 1,            //@emem clip flag from subblock clipping
    CLIP_ALPHA  = 2,            //@emem alpha clipping enabled
    CLIP_COLOR  = 3             //@emem clip flag inserts fixed color
};

enum eVideoOutputFormat {       //@enum Audio and Video output formats
    UYVY    = 0,                //@emem BO_1234 - YUV422        
    YUY2    = 1,                //@emem BO_2143 - YUV422 
    Y41P    = 2,                //@emem BO_1234 - YUV411
    Y8      = 3,                //@emem BO_1234 - Y8
    YV12    = 4,                //@emem BO_1234 - YUV420 planar
    YU12    = 5,                //@emem BO_1234 - YVU420 planar
    YVU9    = 6,                //@emem BO_1234 - YUV9(indeo) planar
    RGB565  = 7,                //@emem BO_1234 - RGB565 
    RGB888  = 8,                //@emem BO_1234 - RGB24
    RGB888a = 9,                //@emem BO_1234 - aRGB32  
    RGBa555 = 10,               //@emem BO_1234 - RGBa555 
    RGB55a5 = 11                //@emem BO_1234 - RGB55a5  
};

enum eScalerColorControl {      //@enum Scaler color control type
    SC_BRIGHTNESS,              //@emem control for brightness
    SC_CONTRAST,                //@emem control for contrast
    SC_SATURATION               //@emem control for saturation
};

//@head3 Enumerations for Decoder |
enum eVideoStatus {             //@enum Video decoder status
    VS_STATUS = 0,              //@emem complete status byte
    VS_CHIPVERSION,             //@emem chip version
    VS_DETCOLSTD,               //@emem detected color standard
    VS_WPACT,                   //@emem white peak loop is activated
    VS_GAINLIMBOT,              //@emem gain value is limited (bottom)
    VS_GAINLIMTOP,              //@emem gain value is limited (top)
    VS_SLOWTCACT,               //@emem slow time constant active in WIPA mode
    VS_HLOCK,                   //@emem status bit for locked horizontal frequency
    VS_RDCAP,                   //@emem ready for capture (all internal loops locked)
    VS_MVCOPYPROT,              //@emem copy protected source detected
    VS_MVCOLSTRIPE,             //@emem Macrovision encoded color stripe burst detected
    VS_MVPROTTYPE3,             //@emem Macrovision encoded color stripe burst Type 3
    VS_FIDT,                    //@emem identification bit for detected field frequency
    VS_HVLOOPN,                 //@emem status bit for horizontal and vertical loop
    VS_INTL                     //@emem status bit for interlace detection
};

enum eVideoStandard {           //@enum Video standard
    PAL         = 0x00,         //@emem normal PAL standard
    NTSC_443_50 = 0x01,         //@emem enhanced NTSC, Subtype 443 with 50 Hz
    PAL_N       = 0x02,         //@emem enhanced PAL, Subtype N with 50 Hz
    NTSC_N      = 0x03,         //@emem enhanced NTSC, Subtype N with 50 Hz
    SECAM       = 0x05,         //@emem normal SECAM standard
    NTSC_M      = 0x10,         //@emem normal NTSC standard
    PAL_443_60  = 0x11,         //@emem enhanced PAL, Subtype 443 with 60 Hz
    NTSC_443_60 = 0x12,         //@emem enhanced NTSC, Subtype 443 with 60 Hz
    PAL_M       = 0x13,         //@emem enhanced PAL, Subtype M with 60 Hz
    NTSC_JAP    = 0x14,         //@emem Japanese NTSC standard
    AUTOSTD     = 0x20,         //@emem autodetect standard
    PAL_50HZ    = 0x21,         //@emem PAL 60 Hz standard in Auto mode
    NTSC_50HZ   = 0x22,         //@emem NTSC 50 Hz standard in Auto mode
    NTSC_60HZ   = 0x23,         //@emem NTSC 60 Hz standard in Auto mode
    PAL_60HZ    = 0x24,         //@emem PAL 60 Hz standard in Auto mode
    SECAM_50HZ  = 0x25,         //@emem SECAM 50 Hz standard in Auto mode
    NO_COLOR_50HZ = 0x27,       //@emem no color (black/white) in Auto mode
    NO_COLOR_60HZ = 0x28        //@emem no color (black/white) in Auto mode
};

enum eVideoSource {             //@enum Video decoder input
    TUNER = 0,                  //@emem input is tuner
    COMPOSITE_1,                //@emem input is CVBS channel 1
    COMPOSITE_2,                //@emem input is CVBS channel 2
    S_VIDEO_1,                  //@emem input is S-Video channel 1
    S_VIDEO_2                   //@emem input is S-Video channel 2
};

enum eVideoSourceType {         //@enum Use timing settings appropriate to:
    CAMERA = 0,                 //@emem input source camera
    TV,                         //@emem input source TV
    VCR                         //@emem input source VCR
};

enum eColorControl {            //@enum Decoder control types
    BRIGHTNESS,                 //@emem control for brightness
    CONTRAST,                   //@emem control for contrast
    SATURATION,                 //@emem control for saturation
    HUE,                        //@emem control for hue
    SHARPNESS,                  //@emem control for sharpness
    GAIN_AUTO,                  //@emem automatic gain control
    GAIN_FREEZE,                //@emem freeze gain
    GAIN1,                      //@emem gain control for channel 1
    GAIN2,                      //@emem gain control for channel 2
    CGAIN_AUTO,                 //@emem automatic chroma gain control
    CGAIN,                      //@emem chroma gain control
    RAWGAIN,                    //@emem raw data gain control
    RAWOFFSET                   //@emem raw data offset control
};

//@head3 Enumerations for I2c |
enum eI2cStatus {               //@enum I2C status
    IDLE            = 0,        //@emem no I2C command pending
    DONE_STOP       = 1,        //@emem I2C command done and STOP executed
    BUSY            = 2,        //@emem executing I2C command
    TO_SCL          = 3,        //@emem executing I2C command, time out on clock stretching
    TO_ARB          = 4,        //@emem time out on arbitration trial, still trying
    DONE_WRITE      = 5,        //@emem I2C command done and awaiting next write command
    DONE_READ       = 6,        //@emem I2C command done and awaiting next read command
    DONE_WRITE_TO   = 7,        //@emem see 5, and time out on status echo
    DONE_READ_TO    = 8,        //@emem see 6, and time out on status echo
    NO_DEVICE       = 9,        //@emem no acknowledge on device slave address
    NO_ACKN         = 10,       //@emem no acknowledge after data byte transfer
    BUS_ERR         = 11,       //@emem bus error
    ARB_LOST        = 12,       //@emem arbitration lost during transfer
    SEQ_ERR         = 13,       //@emem erroneous programming sequence
    ST_ERR          = 14,       //@emem wrong status echoing
    SW_ERR          = 15        //@emem software error
};

enum eI2cAttr {                 //@enum I2C attributes
    NOP             = 0,        //@emem no operation on I2C bus
    STOP            = 1,        //@emem stop condition, no associated byte transfer
    CONTINUE        = 2,        //@emem continue with byte transfer
    START           = 3         //@emem start condition with byte transfer
};

//@head3 Enumerations for Video streaming |

enum eTaskType {                //@enum Video Task type
    TASK_A  = 0,                //@emem Task A
    TASK_B  = 1,                //@emem Task B
    TASK_AB = 2                 //@emem Task A and B
};

enum eFieldType {               //@enum Video field type
    ODD_FIELD           = 0,    //@emem odd field
    EVEN_FIELD          = 1,    //@emem even field
    INTERLACED_FIELD    = 2,    //@emem odd/even interlaced
    EITHER_FIELD        = 3     //@emem don't care if odd or even
};

enum eFieldSequence {           //@enum Video field sequence
    ODD_FIRST   = 0x00,         //@emem normal sequence of odd and even fields
    EVEN_FIRST  = 0x04          //@emem inverted sequence of odd and even fields
};

enum eStreamType {              //@enum Video stream type
    VIDEO_STREAM        = 0,    //@emem Video stream
    VBI_STREAM          = 1,    //@emem VBI stream
    AUDIO_STREAM        = 2,    //@emem Audio stream
    TRANSPORT_STREAM    = 3     //@emem transport stream
};

enum eStreamStatus {            //@enum Video stream status
    S_IDLE    = 0,              //@emem stream has been constructed
    S_OPEN    = 1,              //@emem stream has been opened
    S_LOCK    = 2,              //@emem stream has been requested and locked
    S_START   = 3               //@emem stream is running or temporarily paused
};

enum eVideoOutputType {         //@enum Video stream output types
    VSB_A,                      //@emem the data comes behind the 1.ADC. Only in VSB mode.
    VSB_B,                      //@emem the data comes behind the 2.ADC. Only in VSB mode.
    VSB_AB,                     //@emem the data comes behind both ADCs. Only in VSB mode.
    ITU656_8bit,                //@emem the data comes in front of the scaler and is an ITU format.
    VIP11,                      //@emem the data comes behind the scaler and is VIP 1.1 format.
    VIP20,                      //@emem VIP 2.0 format.
    DMA                         //@emem DMA is the output destination.
};

enum eTSStreamType {            //@enum Transport stream types
    TS_Serial,                  //@emem serial transport stream
    TS_Parallel,                //@emem parallel transport stream
    TS_None                     //@emem nothing is selected
};

//@head3  Enumerations for Interrupt handling |

enum eInterruptEnable {         //@enum Enable interrupts on ...
    EN_GPIO_23_NEG = 0,         //@emem ...falling vertical edge GPIO i/p 23
    EN_GPIO_23_POS,             //@emem ...rising vertical edge GPIO i/p 23
    EN_GPIO_22_NEG,             //@emem ...falling vertical edge GPIO i/p 22
    EN_GPIO_22_POS,             //@emem ...rising vertical edge GPIO i/p 22
    EN_GPIO_18_NEG,             //@emem ...falling vertical edge GPIO i/p 18
    EN_GPIO_18_POS,             //@emem ...rising vertical edge GPIO i/p 18
    EN_GPIO_16_NEG,             //@emem ...falling vertical edge GPIO i/p 16
    EN_GPIO_16_POS,             //@emem ...rising vertical edge GPIO i/p 16
    EN_LOAD_ERROR,              //@emem ...load error occured in scaler
    EN_CONFIG_ERROR,            //@emem ...config error occured in scaler
    EN_TRIGGER_ERROR,           //@emem ...trigger error occured in scaler
    EN_MV_MODE_CHANGE,          //@emem ...Macrovision mode changed
    EN_FIDT_MODE_CHANGE,        //@emem ...50/60Hz mode changed
    EN_INTL_MODE_CHANGE,        //@emem ...decoder interlaced mode detection
    EN_READY_CAPTURE,           //@emem ...decoder ready for capture
    EN_POWER_ON_POS,            //@emem ...power on reset on rising vertical edge
    EN_PARITY_ERROR,            //@emem ...PCI parity error
    EN_ABORT_RECEIVED,          //@emem ...PCI abort received
    EN_STREAM,                  //@emem ...streams 
    EN_MAX_EVENT                //@emem maximum number of eInterruptEnable
};

enum eFieldFrequency {          //@enum Field frequency
    FIELD_FREQ_50HZ = 0,        //@emem source is 50 Hz
    FIELD_FREQ_60HZ = 1,        //@emem source is 60 Hz
    FIELD_FREQ_AUTO = 2         //@emem source is detected automatically
};

enum eInterlacedMode {          //@enum Field mode
    NON_INTERLACED  = 0,        //@emem non interlaced mode
    INTERLACED      = 1         //@emem interlaced mode
};

enum eMacroVisionMode {         //@enum Macrovision mode
    COPY_PROTECTION_OFF  = 0,   //@emem MMC protection off
    COPY_PROTECTION_ON   = 1,   //@emem MMC protection on, but not colorstripe
    COLORSTRIPE_ANY_TYPE = 2,   //@emem MMC protection on on all colorstripes
    COLORSTRIPE_TYPE_3   = 3    //@emem MMC protection on, but not colorstripe type 3
};

enum eDecoderIntTaskType {      //@enum Stream and task type for decoder interrupt isr's
    ISR_TASK_A  = 0,            //@emem Decoder isr for Task A
    ISR_TASK_B  = 1,            //@emem Decoder isr for Task B
    ISR_TASK_AB = 2,            //@emem Decoder isr for Task A and B
    ISR_VBI     = 3             //@emem Decoder isr for VBI
};

//@head3  Enumerations for Memory management |

enum eVampPoolType {            //@enum memory pool type
    NON_PAGED_POOL  = 0,        //@emem memory from non paged pool
    PAGED_POOL      = 1,        //@emem memory from paged pool (not contiguous)
    MAX_POOL_TYPE   = 2
};

//@head3  Enumerations for Buffer management |

enum eByteOrder {               //@enum Byte order at DMA transfer
#if !defined BIG_ENDIAN
    BO_1234 = 0,                //@emem no byte swapping
    BO_2143 = 1,                //@emem byte swapping enabled, Type 1
    BO_3412 = 2,                //@emem byte swapping enabled, Type 2
    BO_4321 = 3,                //@emem byte swapping enabled, Type 3
    KEEP_AS_IS                  //@emem byte order from video format will be taken
#else                           //@end Big endian machines HP's, Sun's and Alpha need this
    BO_4321 = 0,                // no byte swapping
    BO_3412 = 1,                // byte swapping enabled, Type 1
    BO_2143 = 2,                // byte swapping enabled, Type 2
    BO_1234 = 3,                // byte swapping enabled, Type 3
    KEEP_AS_IS                  // the byte order from Video Format will be taken
#endif
};

enum eBufferType {              //@enum Kind of buffer
    CVAMP_BUFFER_STATIC  = 0,   //@emem static buffer
    CVAMP_BUFFER_QUEUED,        //@emem queued buffer
    CVAMP_BUFFER_UNKNOWN        //@emem unknown buffer type
};

enum eBufferStatus {                    //@enum Buffer status
    CVAMP_BUFFER_EMPTY          = 0x00, //@emem buffer is empty
    CVAMP_BUFFER_DONE           = 0x01, //@emem buffer has been completed
    CVAMP_BUFFER_CANCELLED      = 0x02, //@emem buffer has been removed from the empty list and is uncompleted
    CVAMP_BUFFER_CORRUPTED      = 0x04, //@emem buffer is corrupted
    CVAMP_BUFFER_RECOVERED      = 0x08, //@emem buffer has been recovered from a lost interrupt
    CVAMP_BUFFER_VALIDTIMESTAMP = 0x10, //@emem the buffer's time stamp of processing is valid
    CVAMP_BUFFER_IN_USE         = 0x40, //@emem buffer is in use
    CVAMP_BUFFER_DISCONTINUITY  = 0x80, //@emem there is a discontinuity in the audio stream
    CVAMP_BUFFER_ODD_COMPLETE   = 0x100 //@emem the odd field of an interlaced buffer is complete
};


//@head3  Enumerations for Audio |

enum AStandard {                //@enum Audio standard definition
    STANDARD_BTSC,              //@emem audio standard is btsc
    STANDARD_NICAM,             //@emem audio standard is nicam
    STANDARD_UNDEFINED,         //@emem audio standard is undefined
};

enum ASampleRate {              //@enum Sample rate
    SAMPLE_32_KHZ,              //@emem 32KHz sampling rate
    SAMPLE_44_KHZ,              //@emem 44KHz sampling rate
    SAMPLE_48_KHZ,              //@emem 48KHz sampling rate
};

enum ASampleSize {              //@enum Sample size
    SAMPLE_8_BIT,               //@emem 8 bit samples
    SAMPLE_16_BIT,              //@emem 16 bit samples
    SAMPLE_20_BIT_LEFT,         //@emem 20 bit samples left bounded
    SAMPLE_20_BIT_RIGHT         //@emem 20 bit samples left bounded
};

enum ASampleMode {              //@enum Sample mode
    SAMPLE_MONO,                //@emem mono samples
    SAMPLE_STEREO               //@emem stereo samples
};

enum AInput {                   //@enum Audio inputs definition
    IN_AUDIO_DISABLED,          //@emem The input is disabled
    IN_AUDIO_TUNER,             //@emem The input is of type tuner
    IN_AUDIO_COMPOSITE_1,       //@emem The input is general CVBS channel 1
    IN_AUDIO_COMPOSITE_2,       //@emem The input is general CVBS channel 2
    IN_AUDIO_S_VIDEO_1,         //@emem The input is a S-Video signal channel 1
    IN_AUDIO_S_VIDEO_2,         //@emem The input is a S-Video signal channel 2
	IN_AUDIO_FM,				//@emem The input is a FM signal
    IN_AUDIO_UNDEFINED          //@emem Audio input is undefined
};

enum AOutput {                  //@enum Audio output definition
    OUT_LOOPBACK,               //@emem audio output is Loopback
    OUT_STREAM,                 //@emem audio output is Streaming
    OUT_I2S,                    //@emem audio output is I2S
    OUT_UNDEFINED,              //@emem audio output is undefined
};

enum AFormat {                  //@enum Audio format definition
    FORMAT_MONO,                //@emem audio output is mono
    FORMAT_STEREO,              //@emem audio output is stereo
    FORMAT_CHANNEL_A,           //@emem audio format is channel A (dual channel)
    FORMAT_CHANNEL_B,           //@emem audio format is channel B (dual channel)
    FORMAT_UNDEFINED            //@emem audio format is undefined
};

//@head3  Enumerations for GPIO |

enum eEdge {                    // @enum Edge type to force.
    leading,                    // @emem leading edge of Start of Packet
    trailing                    // @emem trailing edge of Start of Packet
};

enum eAccessType {              // @enum Access type of the pin.
    InputOnly,                  // @emem pin can be only used as input.
    InputAndOutput,             // @emem pin can be used as input and output.
    OutputOnly                  // @emem pin can be only used as output.
};



//@head3 Type definitions for Devices |
typedef
struct DEVICEPARAMS {                       //@struct Device parameters
    IN OUT PBYTE pBaseAddress;              //@field base address of memory mapped registers
    IN OUT DWORD dwDeviceId;                //@field device Id
    IN OUT DWORD dwVendorId;                //@field vendor Id
    IN OUT DWORD dwSubsystemId;             //@field subsystem Id
    IN OUT DWORD dwSubvendorId;             //@field subvendor Id
    IN OUT DWORD dwDeviceRevision;          //@field device revision number
    IN OUT ULONG_PTR ulPtrDeviceNode;       //@field device node
    IN OUT BOOL  bDeviceIsPresent;          //@field TRUE, if this device is available
    IN OUT CHAR  pszFriendlyName[MAX_PATH]; //@field friendly name of device
} tDeviceParams;

//@head3 Type definitions for Cipper and Scaler |

typedef
struct VAMPRECT {               //@struct Window's rectangle
    LONG left;                  //@field left coordinate
    LONG top;                   //@field top coordinate
    LONG right;                 //@field right coordinate
    LONG bottom;                //@field bottom coordinate
} tVampRect;

typedef
struct RECTLIST {               //@struct Rectangle list
    int nSize;                  //@field number of rectangles in the list
    tVampRect *pRect;           //@field pointer to rectangles
} tRectList;

typedef
struct VIDEORECT {              //@struct Video rectangle
    DWORD nStartX;              //@field horizontal start value
    DWORD nStartY;              //@field vertical start value
    DWORD nWidth;               //@field width in pixels
    DWORD nHeight;              //@field number of lines
} tVideoRect;

typedef
struct GAMMACURVES {            //@struct Gamma curves for post processing
    BOOL ucGammaActivate;       //@field if TRUE: activate post processing
    DWORD dwStartGreenPath;     //@field start point for green path
    DWORD dwStartBluePath;      //@field start point for blue path
    DWORD dwStartRedPath;       //@field start point for red path
    DWORD dwGreenPath[4];       //@field points of gamma curves for green
    DWORD dwBluePath[4];        //@field points of gamma curves for blue
    DWORD dwRedPath[4];         //@field points of gamma curves for red
} tGammaCurves;

//@head3 Type definitions for Streams |

class CCallOnDPC;
class COSDependTimeStamp;
class COSDependRegistry;

typedef DWORD tStreamFlags;     //#!# @defitem tStreamFlags | driver info flags for the stream manager

typedef
struct STREAMPARMS {                        //@struct Common stream input parameters
    eFieldType nField;                      //@field Field type - O, E or I fields
    CCallOnDPC *pCallbackFktn;              //@field CB class, CB method will be called at DPC level
    PVOID pCBParam1;                        //@field Parameter 1 for CB method
    PVOID pCBParam2;                        //@field Parameter 2 for CB method
    COSDependTimeStamp *pTimeStamp;         //@field COSDependTimeStamp object
    union {
        struct
        {
            eVideoOutputType nOutputType;   //@field video output types [VSB_A,VSB_B,VSB_AB,ITU,VIP11,VIP20,DMA]
            eVideoStandard  nVideoStandard; //@field Video standard
            eVideoOutputFormat nFormat;     //@field Video format
            tVideoRect tVideoInputRect;     //@field Acquisition rectangle
            DWORD dwOutputSizeX;            //@field Video output window size horizontal
            DWORD dwOutputSizeY;            //@field Video output window size vertical
            union {
                struct {
                    WORD wFractionPart;     //@field fraction frame rate
                    WORD wFrameRate;        //@field total frame rate
                } FrameRateAndFraction;
                DWORD dwFrameRate;          //@field Frame rate, the high word contains the
                                            // total frames, the low word the fraction
            } u;
            tStreamFlags dwStreamFlags;     //@field Flags for the StreamManager to handle the video stream
        } tVStreamParms;
        // space for future addings, e.g. audio layer
    } u;
} tStreamParms;


//@head3 Type definitions for Buffers |

typedef LONGLONG tOpaqueTimestamp;  //#!# @defitem tOpaqueTimestamp | system dependent timestamp
typedef LONGLONG t100nsTime;        //#!# @defitem tOpaqueTimestamp | time in 100ns units
typedef DWORD tBufferStatusFlags;   //#!# @defitem tBufferStatusFlags | Buffer status flags

typedef
struct BUFFERFORMAT {               //@struct Buffer format 
    IN DWORD dwSamplesPerLine;      //@field number of samples per line
    IN DWORD dwChannelsPerSample;   //@field number of channels per sample (one, except for audio)
    IN DWORD dwBitsPerSample;       //@field number of bits per sample
    IN DWORD dwNrOfLines;           //@field number of lines (always one for audio)
    IN LONG lPitchInBytes;          //@field pitch in bytes between start of two consecutive lines (signed integer)
    IN LONG lBufferLength;          //@field buffer size in bytes at initialization, might be reduced to a 
                                    //filled buffer size
    IN eByteOrder  ByteOrder;       //@field byte order for DMA transfer, might be used to correct the little and 
                                    //big endian problem and also to extent the output formats
    IN eBufferType BufferType;      //@field kind of buffer (queued or static)
} tBufferFormat;

typedef
struct BUFFERSTATUS {                   //@struct Buffer status 
    OUT eFieldType nFieldType;          //@field type of fields
    OUT LONG lActualBufferLength;       //@field buffer length. might be reduced to a filled buffer size
    OUT tBufferStatusFlags Status;      //@field buffer status flags, a combination of eBufferStatus enumerations
    OUT DWORD dwLostBuffer;             //@field number of really lost buffers; if the buffer is a recovered buffer, 
                                        //the status flag CVAMP_BUFFER_RECOVERED is set. The driver is
                                        //responsible to recover the correct time stamp for this buffer.
    OUT tOpaqueTimestamp llTimeStamp;   //@field capture timestamp
} tBufferStatus;

//@head3 Type definitions for Audio |

typedef 
struct SAMPLE_PROPERTY {            // @struct Sample Property 
    ASampleRate  tSampleRate;       // @field ASampleRate | tSampleRate | The streaming sample rate.
    ASampleSize  tSampleSize;       // @field ASampleSize | tSampleSize | The streaming sample size.
    ASampleMode  tSampleMode;       // @field ASampleMode | tSampleMode | The streaming sample mode.
} tSampleProperty;



//@head3 Type definitions for GPIO |

typedef 
struct GPIO_PIN_INFO                // @struct Describes the EEPROM/bootstrap information for
                                    //         each GPIO pin and the current status.
{
    int         nGPIONr;            // @field int | nGPIONr | The GPIO number.
    int         nPinNr;             // @field int | nPinNr | The Pin number.
    eAccessType nAccess;            // @field AccessType | nAccess | This variable describes the access type of the
                                    // pin. The information of that should comes from an EPROM via I2C or from
                                    // the bootstrap pins.
    DWORD       dwMask;             // @field DWORD | dwMask | The mask to access this pin. It makes sense only for static GPIO.
    BOOL        bIsInUse;           // @field BOOL | bIsInUse | Describes if the pin is currently used.
} tGPIOPinInfo;

#define NUM_DMA_CHANNELS 7
typedef
struct PAGE_ADDRESS_INFO
{
    PVOID pVirtualAddress;
    DWORD dwPhysicalAddress;
    PVOID pAlignedVirtualAddress;
    DWORD dwAlignedPhysicalAddress;
} tPageAddressInfo;

#endif // !defined(VAMPTYPES_H_INCLUDED_)
