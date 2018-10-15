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
// @module   VampHALTypes | This file contains common defines, enumeration types
// and type definition which are exclusively for the use inside the HAL. It is 
// the central header file for all internal modules.
// Enumerated types are identified by the 'e' prefix. Typedefs are preceeded by 
// the prefix 't'.<nl>
// @end
// Filename: VampHALTypes.h
//
//
//////////////////////////////////////////////////////////////////////////////

#if !defined(VAMPHALTYPES_H_INCLUDED_)
#define VAMPHALTYPES_H_INCLUDED_

// @head3 Common definitions | Defines for the HAL.

// @head4 Channel request flags for stream types (bits[6:0]: DMA channels) |
#define S_VIDEO_A           0x00010001  //@defitem S_VIDEO_A | Video stream Task A
#define S_VIDEO_B           0x00020002  //@defitem S_VIDEO_B | Video stream Task B
#define S_VIDEO_AB          0x00030003  //@defitem S_VIDEO_AB | Video stream Task AB
#define S_VBI_A             0x00040004  //@defitem S_VBI_A | VBI stream Task A
#define S_VBI_B             0x00080008  //@defitem S_VBI_B | VBI stream Task B
#define S_VBI_AB            0x000C000C  //@defitem S_VBI_AB | VBI stream Task AB
#define S_PLANAR_A          0x00100031  //@defitem S_PLANAR_A | planar Video stream Task A
#define S_PLANAR_B          0x00200032  //@defitem S_PLANAR_B | planar Video stream Task B
#define S_PLANAR_AB         0x00300033  //@defitem S_PLANAR_AB | planar Video stream Task AB
#define S_AUDIO             0x00400040  //@defitem S_AUDIO | Audio stream
#define S_TRANSPORT_S       0x01000020  //@defitem S_TRANSPORT_S | Transport stream serial
#define S_TRANSPORT_P       0x020000A0  //@defitem S_TRANSPORT_P | Transport stream parallel
#define S_VSB_A             0x0400011F  //@defitem S_VSB_A | GPIO stream from ADC1
#define S_VSB_B             0x0800019F  //@defitem S_VSB_B | GPIO stream from ADC2
#define S_VSB_AB            0x0C00019F  //@defitem S_VSB_AB | GPIO stream from both ADCs
#define S_ITU656            0x10000100  //@defitem S_ITU656 | GPIO stream from Decoder
#define S_VIP11             0x20000100  //@defitem S_VIP11 | GPIO stream from Scaler
#define S_VIP20             0x40000100  //@defitem S_VIP20 | GPIO stream from Scaler

#define S_FIFO_1            0x0000000F  //@defitem S_FIFO_1 | FIFO 1 (DMA channels 0-3)
#define S_FIFO_2            0x00000010  //@defitem S_FIFO_2 | FIFO 2 (DMA channel 4)
#define S_FIFO_3            0x00000020  //@defitem S_FIFO_3 | FIFO 3 (DMA channel 5)
#define S_FIFO_4            0x00000040  //@defitem S_FIFO_4 | FIFO 4 (DMA channel 6)
#define S_ALL_FIFOS         0x0000007F  //@defitem S_ALL_FIFOS | all FIFOs
#define S_NO_CHANNEL        0x00000000  //@defitem S_NO_CHANNEL | no DMA channel
#define S_CHANNEL_0         0x00000001  //@defitem S_CHANNEL_0 | DMA channel 0
#define S_CHANNEL_1         0x00000002  //@defitem S_CHANNEL_1 | DMA channel 1
#define S_CHANNEL_2         0x00000004  //@defitem S_CHANNEL_2 | DMA channel 2
#define S_CHANNEL_3         0x00000008  //@defitem S_CHANNEL_3 | DMA channel 3
#define S_CHANNEL_4         0x00000010  //@defitem S_CHANNEL_4 | DMA channel 4
#define S_CHANNEL_5         0x00000020  //@defitem S_CHANNEL_5 | DMA channel 5
#define S_CHANNEL_6         0x00000040  //@defitem S_CHANNEL_6 | DMA channel 6
#define S_ALL_CHANNELS      0x0000007F  //@defitem S_ALL_CHANNELS | all DMA channels

#define S_DMA               0x03FF0000  //@defitem S_DMA | all DMA streams
#define S_GPIO              0x7C000000  //@defitem S_GPIO | all GPIO streams
#define S_SCALER            0x603F0000  //@defitem S_SCALER | all Scaler related streams

// @head4 Control flags for RAM area configuration |
#define RAM_DEFAULT         0x00        //@defitem RAM_DEFAULT | default
#define RAM_PACKED          0x01        //@defitem RAM_PACKED | packed video
#define RAM_TRANSPORT       0x02        //@defitem RAM_TRANSPORT | transport stream
#define RAM_AUDIO           0x04        //@defitem RAM_AUDIO | audio stream
#define RAM_PLANAR          0x08        //@defitem RAM_PLANAR | planar video

// @head4 Control flags for Decoder register range check |
#define DEC_CTR_SYNC        0x00000001  //@defitem DEC_CTR_SYNC | sync control
#define DEC_CTR_VGATE       0x00000002  //@defitem DEC_CTR_VGATE | V-gate control

// @head4 Control flags for Scaler register setting |
#define SCA_CTR_ALL         0xffffffff  //@defitem SCA_CTR_ALL | all scaler registers
#define SCA_CTR_SOURCE_DEP  0x00000001  //@defitem SCA_CTR_SOURCE_DEP | source dependent registers
#define SCA_CTR_GAMMA       0x00000002  //@defitem SCA_CTR_GAMMA | post processing registers
#define SCA_CTR_BASIC       0x00000004  //@defitem SCA_CTR_BASIC | basic settings registers
#define SCA_CTR_FSKIP       0x00000008  //@defitem SCA_CTR_FSKIP | field handling registers
#define SCA_CTR_IO_DATA     0x00000010  //@defitem SCA_CTR_IO_DATA | I/O data path registers
#define SCA_CTR_INPUT_WIN   0x00000020  //@defitem SCA_CTR_INPUT_WIN | input window registers
#define SCA_CTR_OUTPUT_WIN  0x00000040  //@defitem SCA_CTR_OUTPUT_WIN | output window registers
#define SCA_CTR_FILTER      0x00000080  //@defitem SCA_CTR_FILTER | FIR filter and prescale registers
#define SCA_CTR_BRIGHTNESS  0x00000100  //@defitem SCA_CTR_BRIGHTNESS | brightness registers
#define SCA_CTR_CONTRAST    0x00000200  //@defitem SCA_CTR_CONTRAST | contrast registers
#define SCA_CTR_SATURATION  0x00000400  //@defitem SCA_CTR_SATURATION | saturation registers
#define SCA_CTR_SCALE       0x00000800  //@defitem SCA_CTR_SCALE | scaling registers
#define SCA_CTR_SC2DMA      0x00001000  //@defitem SCA_CTR_SC2DMA | scaler_to_DMA (Clipper) registers

// @head4 Scaler enable flags |
#define VIDENA              0x00000001  //@defitem VIDENA | enable Video stream Task A 
#define VBIENA              0x00000002  //@defitem VIDENB | enable Video stream Task B 
#define VIDENB              0x00000010  //@defitem VBIENA | enable VBI stream Task A 
#define VBIENB              0x00000020  //@defitem VBIENB | enable VBI stream Task B 

// @head4 Scaler status flags |
#define SCA_STS_VIDENA      0x00010000  //@defitem SCA_STS_VIDENA | Video stream Task A enabled
#define SCA_STS_VBIENA      0x00020000  //@defitem SCA_STS_VIDENB | Video stream Task B enabled
#define SCA_STS_VIDENB      0x00100000  //@defitem SCA_STS_VBIENA | VBI stream Task A enabled
#define SCA_STS_VBIENB      0x00200000  //@defitem SCA_STS_VBIENB | VBI stream Task B enabled
#define SCA_STS_TRIGERR     0x01000000  //@defitem SCA_STS_TRIGERR | trigger error 
#define SCA_STS_CONFIGERR   0x02000000  //@defitem SCA_STS_CONFIGERR | config error 
#define SCA_STS_LOADERR     0x04000000  //@defitem SCA_STS_LOADERR | load error 

// @head4 Masks for Scaler_to_DMA registers |
#define S2D_RGB_OUTPUTMODE  0x10        //@defitem S2D_RGB_OUTPUTMODE | video output is RGB format
#define S2D_CLIP_MODE_MASK  0x60606060  //@defitem S2D_CLIP_MODE_MASK | clipping modes

// @head4 Flags for DMA main control register |
#define DMA_CTR_ALL_TYPES   0x0000ff7f  //@defitem DMA_CTR_ALL_TYPES | all DMA control bits
#define DMA_CTR_TE          0x0000007f  //@defitem DMA_CTR_TE | transfer enable bits
#define DMA_CTR_ENBASEDAC   0x00000100  //@defitem DMA_CTR_ENBASEDAC | base band DAC control
#define DMA_CTR_ENBASEADC   0x00000200  //@defitem DMA_CTR_ENBASEADC | base band ADC control
#define DMA_CTR_ENSIFFE     0x00000400  //@defitem DMA_CTR_ENSIFFE | SIF frontend control
#define DMA_CTR_ENVIDFE2    0x00000800  //@defitem DMA_CTR_ENVIDFE2 | video frontend 2 control
#define DMA_CTR_ENVIDFE1    0x00001000  //@defitem DMA_CTR_ENVIDFE1 | video frontend 1 control
#define DMA_CTR_ENOSC       0x00002000  //@defitem DMA_CTR_ENOSC | oscillator control
#define DMA_CTR_ENAUDPLL    0x00004000  //@defitem DMA_CTR_ENAUDPLL | audio PLL control
#define DMA_CTR_ENVIDPLL    0x00008000  //@defitem DMA_CTR_ENVIDPLL | video PLL control


// @head4 Flags for DMA main status register |
#define DMA_STS_ALL_TYPES   0xffffffff  //@defitem DMA_STS_ALL_TYPES | complete DMA status bits
#define DMA_STS_CONFERR     0x00000001  //@defitem DMA_STS_CONFERR | configuration error occurred in scaler
#define DMA_STS_TRIGERR     0x00000002  //@defitem DMA_STS_TRIGERR | trigger error occurred in scaler
#define DMA_STS_LOADERR     0x00000004  //@defitem DMA_STS_LOADERR | load error occurred in scaler
#define DMA_STS_PWRON       0x00000008  //@defitem DMA_STS_PWRON | power on reset  
#define DMA_STS_RDCAP       0x00000010  //@defitem DMA_STS_RDCAP | decoder ready for capture 
#define DMA_STS_INTL        0x00000020  //@defitem DMA_STS_INTL | interlaced mode
#define DMA_STS_FIDT        0x00000040  //@defitem DMA_STS_FIDT | 50 Hz signal detected by decoder 
#define DMA_STS_MVM         0x00000380  //@defitem DMA_STS_MVM | Macrovision mode 
#define DMA_STS_IIC_STATUS  0x00000C00  //@defitem DMA_STS_IIC_STATUS | I2C main status
#define DMA_STS_AUDIO       0x0003f000  //@defitem DMA_STS_VSDP | Audio status bits
#define DMA_STS_VSDP        0x00001000  //@defitem DMA_STS_VSDP | NICAM decoder has locked on NICAM stream  
#define DMA_STS_DSB         0x00002000  //@defitem DMA_STS_DSB | NICAM decoder reiceives dual mono stream
#define DMA_STS_SMB         0x00004000  //@defitem DMA_STS_SMB | NICAM decoder reiceives stereo stream
#define DMA_STS_PILOT       0x00008000  //@defitem DMA_STS_PILOT | FM demodulator found pilot 
#define DMA_STS_DUAL        0x00010000  //@defitem DMA_STS_DUAL | FM decoder reiceives dual mono stream 
#define DMA_STS_STEREO      0x00020000  //@defitem DMA_STS_STEREO | FM decoder reiceives stereo stream

// @head4 Flags for DMA channel status register |
#define DMA_STS_DMA_ABORT   0x0000007f  //@defitem DMA_STS_DMA_ABORT | DMA abort received
#define DMA_STS_MMU_ABORT   0x00007f00  //@defitem DMA_STS_MMU_ABORT | MMU abort received
#define DMA_STS_ODD_EVEN    0x00f00000  //@defitem DMA_STS_ODD_EVEN | current field type of DMA
#define DMA_STS_DMA_DONE    0x0f000000  //@defitem DMA_STS_DMA_DONE | DMA has completed a field
#define DMA_STS_FIFO_EMPTY  0xf0000000  //@defitem DMA_STS_FIFO_EMPTY | FIFO is empty

// @head4 Flags for FIFO status register |
#define DMA_STS_FIFO_TASK   0x00000001  //@defitem DMA_STS_FIFO_TASK | task corresponding to FIFO
#define DMA_STS_FIFO_SOURCE 0x00000002  //@defitem DMA_STS_FIFO_SOURCE | source corresponding to FIFO
#define DMA_STS_FIFO_VAP    0xfffffffc  //@defitem DMA_STS_FIFO_VAP | virtual address pointer corresponding to FIFO

// @head4 DMA channel flags for FIFO status |
#define DMA_STS_ALL_CHANNELS 0xffffffff //@defitem DMA_STS_ALL_CHANNELS | all DMA channels
#define DMA_STS_CHANNEL0    0x11100101  //@defitem DMA_STS_CHANNEL0 | DMA channel 0
#define DMA_STS_CHANNEL1    0x11100202  //@defitem DMA_STS_CHANNEL1 | DMA channel 1
#define DMA_STS_CHANNEL2    0x11100404  //@defitem DMA_STS_CHANNEL2 | DMA channel 2
#define DMA_STS_CHANNEL3    0x11100808  //@defitem DMA_STS_CHANNEL3 | DMA channel 3
#define DMA_STS_CHANNEL4    0x22201010  //@defitem DMA_STS_CHANNEL4 | DMA channel 4
#define DMA_STS_CHANNEL5    0x44402020  //@defitem DMA_STS_CHANNEL5 | DMA channel 5
#define DMA_STS_CHANNEL6    0x88804040  //@defitem DMA_STS_CHANNEL6 | DMA channel 6
#define DMA_STS_FIFO1       0x11100f0f  //@defitem DMA_STS_FIFO1 | DMA FIFO 1 (channel 0-3)
#define DMA_STS_FIFO2       0x22201010  //@defitem DMA_STS_FIFO2 | DMA FIFO 2 (channel 4)
#define DMA_STS_FIFO3       0x44402020  //@defitem DMA_STS_FIFO3 | DMA FIFO 3 (channel 5)
#define DMA_STS_FIFO4       0x88804040  //@defitem DMA_STS_FIFO4 | DMA FIFO 4 (channel 6)

// @head4 Timeout definitions for Scaler and I2C |
#define SCA_TIMEOUT         100000      //@defitem SCA_TIMEOUT | timeout value for scaler disable
#define I2C_TIME_OUT        100000      //@defitem I2C_TIME_OUT | timeout value for I2C error reset

// @head4 Interrupt condition flags for RAM area complete |
#define IRQ_SET_EVEN        0x55        //@defitem IRQ_SET_EVEN | IRQ enable: even flags
#define IRQ_SET_ODD         0xaa        //@defitem IRQ_SET_ODD | IRQ enable: odd flags
#define IRQ_SET_TASKA       0x33        //@defitem IRQ_SET_TASKA | IRQ enable: TaskA flags
#define IRQ_SET_TASKB       0xcc        //@defitem IRQ_SET_TASKB | IRQ enable: TaskB flags
#define IRQ_SET_SOURCE0     0x0f        //@defitem IRQ_SET_VIDEO | IRQ enable: Source0 flags
#define IRQ_SET_SOURCE1     0xf0        //@defitem IRQ_SET_VBI | IRQ enable: Source1 flags
#define IRQ_SET_ALL         0xff        //@defitem IRQ_SET_ALL | IRQ enable: all flags

// @head4 Interrupt conditions for RAM area complete  |
#define IRQOnEven           0x00        //@defitem IRQOnEven | interrupt on even field
#define IRQOnOdd            0x01        //@defitem IRQOnOdd  | interrupt on odd field
#define IRQOnTaskA          0x00        //@defitem IRQOnTaskA | interrupt on task A
#define IRQOnTaskB          0x02        //@defitem IRQOnTaskB | interrupt on task B
#define IRQOnSource0        0x00        //@defitem IRQOnSource0 | interrupt on Video stream
#define IRQOnSource1        0x04        //@defitem IRQOnSource0 | interrupt on VBI stream

// @head4 Other interrupt related defines  |
#define MAX_LOST_INTERRUPTS 15          //@defitem MAX_LOST_INTERRUPTS | Maximum number of lost interrupts 
#define NUM_OF_FIFO_INT_CONDITIONS 8    //@defitem NUM_OF_FIFO_INT_CONDITIONS | Number of FIFO interrupt conditions
#define INT_REGISTER_MASK   0x0003ffff  //@defitem INT_REGISTER_MASK | Mask for interrupt registers
#define INT_ENABLE2_DEF     0x00000040  //@defitem INT_ENABLE2_DEF | Interrupt Enable2 default settings

// @head4 Buffer flags |
#define BUFFER_OVERFLOW_MAPPING_DIRECT 1   //@defitem BUFFER_OVERFLOW_MAPPING_DIRECT | overflow buffer, mapping contains physical address
#define BUFFER_OVERFLOW_MAPPING_NONE   2   //@defitem BUFFER_OVERFLOW_MAPPING_NONE | overflow buffer with no mapping info
#define BUFFER_POOL_MAPPING_NONE       3   //@defitem BUFFER_POOL_MAPPING_NONE | pool buffer with no mapping info
#define BUFFER_REGULAR_MAPPING_OS      4   //@defitem BUFFER_REGULAR_MAPPING_OS | regular buffer with OS specific mapping info
#define BUFFER_REGULAR_MAPPING_NONE    5   //@defitem BUFFER_REGULAR_MAPPING_OS | regular buffer with no mapping info

// @head4 GPIO defines |
#define EEPROM_DEV_ADDR			 0xA0		//@defitem EEPROM_DEV_ADDR | EEPROM I2C device address
#define EEPROM_SUB_ADDR			 0x10		//@defitem EEPROM_SUB_ADDR | EEPROM sub address for GPIO info
#define ENABLE_GPIOOUT_ITU_V1    0xA101B000 //@defitem ENABLE_GPIOOUT_ITU_V1 | Complete register to enable GPIO out for ITU format
#define DISBALE_GPIOOUT_ITU_V1   0x0001B000 //@defitem DISBALE_GPIOOUT_ITU_V1 | Complete register to disable GPIO out for ITU format
#define ENABLE_GPIOOUT_VIP11_V1  0xA0018590 //@defitem ENABLE_GPIOOUT_VIP11_V1 | Complete register to enable GPIO out for VIP 1.1 format
#define DISABLE_GPIOOUT_VIP11_V1 0x00018590 //@defitem DISABLE_GPIOOUT_VIP11_V1 | Complete register to disable GPIO out for VIP 1.1 format
#define ENABLE_GPIOOUT_VIP20_V1  0xA0018388 //@defitem ENABLE_GPIOOUT_VIP20_V1 | Complete register to enable GPIO out for VIP 2.0 format
#define DISABLE_GPIOOUT_VIP20_V1 0x00018388 //@defitem DISABLE_GPIOOUT_VIP20_V1 | Complete register to disable GPIO out for VIP 2.0 format
#define GPIOOUT_V2				 0x00000000 //@defitem GPIOOUT_V2 | Complete value for VideoOut 2 register
#define DEFAULT_PIN_CONFIG		 0x00C2FF00 //@defitem DEFAULT_PIN_CONFIG | Default value for GPIO pin configuration 

// @head4 Audio defines |
#define AUDIOSIZETOBYTES	4			// @defitem AUDIOSIZETOBYTES | Audio data field size to bytes factor
										// (Number of audio samples delivered in 32bit words -> Factor 32 / 8 = 4)
#define MAX_DSP_STD			15			// @defitem MAX_DSP_STD | maximum number of DSP standards (15)
#define NUM_OF_SEARCH_CYLES	 1			// @defitem NUM_OF_SEARCH_CYLES | number of A2/NICAM search cycles
#define MIN_SC1_Level       1036        // @defitem MIN_SC1_Level | minimum sound carrier level 1 (32768 * 0.03 = 1036)
#define MIN_FMDC_THRESH     3200        // @defitem MIN_FMDC_THRESH | minimum DC level for B/G A2 detection (32768 * 0.1 = 3200)
#define VAMP_WAIT_TIMER     50          // @defitem VAMP_WAIT_TIMER | timer interval
#define VAMP_WAIT_TIMEOUT   100/VAMP_WAIT_TIMER // @defitem VAMP_WAIT_TIMEOUT | 2 sec.
#define VAMP_WAIT_FMDC      200/VAMP_WAIT_TIMER // @defitem VAMP_WAIT_FMDC | 200 msec.


// @head3 Enumerations for Scaler_to_DMA unit |

enum eHWVideoOutputFormat {         //@enum Hardware Video formats
    SC_YUV422          = 0x00,      //@emem YUV422
    SC_YUV444          = 0x01,      //@emem YUV444
    SC_YUV411          = 0x03,      //@emem YUV411
    SC_Y8              = 0x06,      //@emem Y8 (luminance only)
    SC_YUV444_PLANAR   = 0x08,      //@emem YUV444 planar
    SC_YUV422_PLANAR   = 0x09,      //@emem YUV422 planar
    SC_YUV420_PLANAR   = 0x0a,      //@emem YUV420 planar
    SC_INDEO_PLANAR    = 0x0b,      //@emem INDEO planar
    SC_RGB565          = 0x10,      //@emem RGB565
    SC_RGB888          = 0x11,      //@emem RGB888
    SC_RGBA888         = 0x12,      //@emem RGB888 and alpha
    SC_RGBA555         = 0x13,      //@emem RGB555 and alpha
    SC_RGB55A5         = 0x14       //@emem RGB555 and alpha (swapped)
};

//@head3  Enumerations regarding the RAM areas |

enum eFifoArea {                    //@enum FIFO number (RAM area)
    FIFO_1      = 0,                //@emem Fifo 1
    FIFO_2      = 1,                //@emem Fifo 2
    FIFO_3      = 2,                //@emem Fifo 3
    FIFO_4      = 3,                //@emem Fifo 4
    FIFO_123    = 4,                //@emem Fifo 123
    FIFO_1234   = 5                 //@emem Fifo 1234
};


enum eFifoOverflowStatus {          //@enum FIFO status on interrupt
	FIFO_OK,                        //@emem correct
	FIFO_OVERFLOW                   //@emem FIFO overflow
};

//@head3  Enumerations for interrupt and event handling |

enum eInterruptCondition {                                      //@enum Interrupt conditions
    EVEN_TASKA_SRC0 = IRQOnEven | IRQOnTaskA | IRQOnSource0,    //@emem IRQ condition 0
    ODD_TASKA_SRC0  = IRQOnOdd  | IRQOnTaskA | IRQOnSource0,    //@emem IRQ condition 1
    EVEN_TASKB_SRC0 = IRQOnEven | IRQOnTaskB | IRQOnSource0,    //@emem IRQ condition 2
    ODD_TASKB_SRC0  = IRQOnOdd  | IRQOnTaskB | IRQOnSource0,    //@emem IRQ condition 3
    EVEN_TASKA_SRC1 = IRQOnEven | IRQOnTaskA | IRQOnSource1,    //@emem IRQ condition 4
    ODD_TASKA_SRC1  = IRQOnOdd  | IRQOnTaskA | IRQOnSource1,    //@emem IRQ condition 5
    EVEN_TASKB_SRC1 = IRQOnEven | IRQOnTaskB | IRQOnSource1,    //@emem IRQ condition 6
    ODD_TASKB_SRC1  = IRQOnOdd  | IRQOnTaskB | IRQOnSource1     //@emem IRQ condition 7
};

enum eLostInterruptSkipMode {       //@enum Indicates whether we are processing or skipping fields
	SKIPPING_FIELDS,                //@emem Fields are being skipped
	PROCESS_FIELDS                  //@emem Fields are being processed
};


enum eInterruptType {               //@enum Interrupt status register bits
    GPIO_23          = 0x00020000,  //@emem edge triggered from GPIO pin 23
    GPIO_22          = 0x00010000,  //@emem edge triggered from GPIO pin 22
    GPIO_18          = 0x00008000,  //@emem edge triggered from GPIO pin 18
    GPIO_16          = 0x00004000,  //@emem edge triggered from GPIO pin 16
    LOAD_ERROR       = 0x00002000,  //@emem load error occured in scaler
    CONFIG_ERROR     = 0x00001000,  //@emem config error occured in scaler
    TRIGGER_ERROR    = 0x00000800,  //@emem trigger error occured in scaler
    MV_MODE_CHANGE   = 0x00000400,  //@emem Macrovision mode changed
    FIDT_MODE_CHANGE = 0x00000200,  //@emem 50/60Hz mode changed
    INTL_MODE_CHANGE = 0x00000100,  //@emem decoder detected interlaced mode
    READY_CAPTURE    = 0x00000080,  //@emem decoder is ready for capture
    POWER_ON         = 0x00000040,  //@emem power on reset
    PARITY_ERROR     = 0x00000020,  //@emem parity error
    ABORT_RECEIVED   = 0x00000010,  //@emem abort received
    DONE_RA4         = 0x00000008,  //@emem buffer on RAM area 4 completed
    DONE_RA3         = 0x00000004,  //@emem buffer on RAM area 3 completed
    DONE_RA2         = 0x00000002,  //@emem buffer on RAM area 2 completed
    DONE_RA1         = 0x00000001   //@emem buffer on RAM area 1 completed
};

// @head3 Audio internal inputs definition |
enum AInputIntern
{
    IN_DISABLED,                    // @emem Audio input is Disabled
    IN_TUNER,                       // @emem Audio input is Tuner
    IN_ANALOG1,                     // @emem Audio input is Analog1
    IN_ANALOG2,                     // @emem Audio input is Analog2
    IN_UNDEFINED,                   // @emem Audio input is undefined
	IN_MUTE
};

//@head3  Audio internal standard detect state definitions |

enum ASearchState {					//@enum Audio search states
    SEARCH_IDLE,                    //@emem search idle state      
    SEARCH_STANDARD,                //@emem search standard state
    SEARCH_BG,                      //@emem search B/G standard state    
    SEARCH_DK,                      //@emem search D/K standard state    
    SEARCH_M,                       //@emem search M standard state
    SEARCH_I,                       //@emem search I standard state
    SEARCH_L                        //@emem search L standard state
};

enum ASearchSubState {              //@enum Audio search substates 
    SEARCH_SUB_RESET,               //@emem reset    
    SEARCH_SUB_PEAK_RESET,          //@emem reset peak                    
    SEARCH_SUB_PEAK_READ,           //@emem read peak                    
    SEARCH_SUB_SC1_45,              //@emem SC1 magnitude 4.5 MHz                    
    SEARCH_SUB_SC1_55,              //@emem SC1 magnitude 5.5 MHz                    
    SEARCH_SUB_SC1_60,              //@emem SC1 magnitude 6.0 MHz                    
    SEARCH_SUB_SC1_65,              //@emem SC1 magnitude 6.5 MHz                    
    SEARCH_SUB_BG_WNICAM,           //@emem BG wait NICAM or DC                    
//    SEARCH_SUB_BG_MONO,             //@emem BG wait MONO to avoid noise                    
    SEARCH_SUB_BG_WA2,              //@emem BG wait A2 or IDENT        
    SEARCH_SUB_DK_SC2_NICAM,        //@emem DK SC2 magnitude NICAM                         
    SEARCH_SUB_DK_SC2_1,            //@emem DK SC2 magnitude Type 1     
    SEARCH_SUB_DK_SC2_2,            //@emem DK SC2 magnitude Type 2       
    SEARCH_SUB_DK_WNICAM,           //@emem DK wait NICAM                         
    SEARCH_SUB_DK_W1,               //@emem DK wait A2 Type 1       
    SEARCH_SUB_DK_W2,               //@emem DK wait A2 Type 2       
    SEARCH_SUB_DK_W3,               //@emem DK wait A2 Type 3       
    SEARCH_SUB_M_MONO,              //@emem DK wait A2 Type 3       
    SEARCH_SUB_M_KOREA,             //@emem DK wait A2 Type 3       
    SEARCH_SUB_M_BTSC,              //@emem DK wait A2 Type 3       
    SEARCH_SUB_I_NICAM,             //@emem I wait NICAM       
    SEARCH_SUB_I_MONO,              //@emem I wait FM mono       
    SEARCH_SUB_L_NICAM,             //@emem L wait NICAM      
    SEARCH_SUB_L_AM                 //@emem L wait AM      
};

enum ASearchResult { 				//@enum Audio search result
    STANDARD_FAILED,                //@emem Failed       
    STANDARD_UNKNOWN,               //@emem Unknown
    STANDARD_BG_MONO,               //@emem BG Mono           
    STANDARD_DK_MONO,               //@emem DK Mono
    STANDARD_M_MONO,                //@emem M Mono
    STANDARD_BG_A2,                 //@emem BG A2
    STANDARD_BG_NICAM,              //@emem BG NICAM
    STANDARD_DK_A2_1,               //@emem DK A2 Type 1
    STANDARD_DK_A2_2,               //@emem DK A2 Type 2
    STANDARD_DK_A2_3,               //@emem DK A2 Type 3
    STANDARD_DK_NICAM,              //@emem DK NICAM
    STANDARD_L_NICAM,               //@emem L NICAM
    STANDARD_L_AM,                  //@emem L AM
    STANDARD_I_NICAM,               //@emem I NICAM
    STANDARD_I_MONO,                //@emem I Mono
    STANDARD_M_KOREA,               //@emem M Korea
    STANDARD_M_BTSC                 //@emem M BTSC
};

// definiton of search vector flags (bitwise ORed)
// (one bit for each standard or std. group)

enum ASearchFlag {		            //@enum Audio search flag 
    BG_FLAG = 1,                    //@emem B/G standard search    
    DK_FLAG = 2,                    //@emem D/K standard search
    L_FLAG =  4,                    //@emem L standard search
    I_FLAG =  8,                    //@emem I standard search
    M_FLAG = 16                     //@emem M standard search
};

enum AInternalInput {				//@enum Internal input 
    INTERNAL_IN_DSP,                //@emem Internal Input is DSP
    INTERNAL_IN_ANALOG,             //@emem Internal Input is Analog Frontend
    INTERNAL_IN_UNDEFINED,          //@emem Internal Input is undefined
};


//@head3 Type definitions for interrupt handling |

class CVampCoreStream;

typedef
struct FIFO_INTERRUPT_ELEM {                 //@struct Interrupt element for lost interrupt checking
    CVampCoreStream *pCoreStream;            //@field core stream pointer for this interrupt condition
    eInterruptCondition PreviousExpectedInt; //@field previously expected interrupt condition
} tFifoIntElem;


//@head3 Type definitions for Scaler |
typedef 
struct SCALINGVALUES {                  //@struct Scaling values
    DWORD dwAccumulationSeqLen;         //@field accumulation length 
    DWORD dwSeqWeight;                  //@field sequence weighting bit
    DWORD dwGainAdjust;                 //@field gain factor and weighting bit
    DWORD dwFirPrefilterControl;        //@field FIR pre filter settings
} tScalingValues;


//@head3 Type definitions for the stream manager |

typedef 
struct STREAMMANAGERINFO {              //@struct Stream manager info  
    DWORD dwFlags;                      //@field stream manager flags
    DWORD dwFSkip;                      //@field field skipping value
    DWORD dwVyStart;                    //@field video input window start line
} tStreamManagerInfo;

typedef
union {
    struct _FRACTION {                  //@struct Fraction  
        USHORT usNumerator;             //@field Numerator value
        USHORT usDenominator;           //@field Denominator value
    } f;
    DWORD d;                            //@field concatenated as DWORD
} tFraction;

//@head3 Type definitions for Audio |

typedef 
struct DSP_STD {                         // @struct DSP_STD
	DWORD		dwCarrierOneFrequ;       // @field DWORD | dwCarrierOneFrequ | The first carrier frequency.
    DWORD		dwCarrierTwoFrequ;       // @field DWORD | dwCarrierTwoFrequ | The second carrier frequency.
	DWORD		dwFM_DeEmphasis;	     // @field DWORD | dwFM_DeEmphasis | FM De-Emphasis characteristics.
	DWORD		dwIdentControl;	         // @field DWORD | dwIdentControl | FM Identification Mode.
	DWORD		dwDemodulatorConf;	     // @field DWORD | dwDemodulatorConf | Demodulator Configuration.
	DWORD		dwStereoDACOutputSelect; // @field DWORD | dwStereoDACOutputSelect | DAC selection (NICAM/FM)
} tDSPStd;


#endif // !defined(VAMPHALTYPES_H_INCLUDED_)
