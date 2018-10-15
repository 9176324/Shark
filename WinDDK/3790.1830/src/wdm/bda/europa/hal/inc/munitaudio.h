//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!! NOTE : Automatic generated FILE - DON'T CHANGE directly             
//!!        You only may change the file Saa7134_RegDef.h                
//!!        and call GenRegIF.bat for update, check in the necessary     
//!!        files before                                                 
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#ifndef _MUNITAUDIO_INCLUDE_
#define _MUNITAUDIO_INCLUDE_


#include "SupportClass.h"


class CMunitAudio
{
public:
//constructor-destructor of class CMunitAudio
    CMunitAudio ( PBYTE );
    virtual ~CMunitAudio();


 //Register Description of this class


    CRegister NICAMStatus;
    // Additional data word
    CRegField AddData_Word;
    // These two bits are CI bits decoded by majority logic
    // from the parity checks of the last ten samples in a frame
    CRegField CI;     
    // When NewAddData = 1, new additional data is written into the IC. This bit is reset,
    // when additional Data bits are read
    CRegField AddData_New;
    // If this bit is HIGH, new additional data bits are written to the IC without
    // the previous bits being read
    CRegField AddData_OverWrite;
    // Idendification of NICAM dual mono. NICAM_DorSB = 1, indicates dual mono mode
    CRegField NICAM_DorSB;
    // Identification of NICAM stereo. NICAM_SorMB = 1 indicates stereo mode
    CRegField NICAM_SorMB;
    // Configuration change: NICAM_ConfigChange = 1 indicates a configuration change at the
    //                       16 frame (C0) boundary.
    CRegField NICAM_ConfigChange;
    // Synchronization bit: NICAM_SyncBit = 1, indicates that the device has both frame and 
    //                      C0 (16 frame) synchronization. 
    //                      NICAM_SyncBit = 0, indicates the audio output from the NICAM part 
    //                      is digital silence
    CRegField NICAM_SyncBit;
    // NICAM application control bits: These bits correspond to the control bits C1 to C4 in 
    //                                 the NICAM transmission
    CRegField NICAM_ControlBits;
    // Identification of NICAM sound: NICAM_SoundFlag = 1, indicates that digital transmission is a sound source
    //                                NICAM_SoundFlag = 0, indicates the transmission
    CRegField NICAM_SoundFlag;
    // Auto-Mute status: If this bit is HIGH, it indicates that the auto-muting function has
    //                   switched from NICAM to the program of the first sound carrier (i.e
    //                   FM mono or AM in NICAM L systems)
    CRegField NICAM_AutoMute;
    CRegister IdentStatus;
    // Nicam Error count: Bits NICAM_ErrorCount contaiin the number of errors occurring in the previous
    //                    128 ms period. The register is updated every 128 ms
    CRegField NICAM_ErrorCount;
    // SIF level Data bits: These bits correspond to the input level at the selected SIF input
    CRegField SIF_DataBits;
    // FM_PilotCarrier = 1 indicates that an FM pilot carrier in the 2nd channel is detected
    // The pilot detector is faster then the stereo/dual idendification, but not as reliable and slightly
    // less sensitive.
    CRegField FM_PilotCarrier;
    // Identification of FM dual sound,A2 systems. If FM_DualSound = 1, an FM dual-language signal has been
    //            identified. When neither FM_Stereo or FM_DualSound = 1, the received signal is assumed
    //            to be FM mono (A2 systems only)
    CRegField FM_DualSound;
    // Identificaton of FM stereo, A2 systems, if FM_Stereo = 1, an FM stereo signal has been identified
    CRegField FM_Stereo;
    // Level Read Out: These two bytes constitute a word that provides data from a location that has been specified
    //                 with the Monitor Select Register.
    CRegField Monitor_SelectData;
    CRegister ControlRegister;
    // NICAM Lower Error Limit: When the auto-mute function is enabled and the NICAM bit error count is lower
    //                          than the value contained in this register, the NICAM signal is selected (again)
    //                          for reproduction
    CRegField NICAM_LowErrLimit;
    // NICAM Upper Error Limit: When the auto-mute function is enabled and the NICAM bit error count is higher 
    //                          than the value contained in this register, the signal of the first sound carrier 
    //                          or the analog mono input is selected for reproduction.
    CRegField NICAM_UppErrLimit;
    // Application area for FM identification: FM_Area = 1 selects FM identification frequencies in accordance with
    //             the specification for Korea. FM_Area - 0, frequencies for Europe are selected 
    CRegField FM_Area;
    // Identification mode for FM sound: These bits define the integrator time of the FM identification. A valid
    //                    result may be expected after twice this time has expired, at the latest. The longer
    //                    the time, the more reliable the identification
    CRegField FM_ItegrateTime;
    // DCXO test mode enable: FM_DCXOTest = 1 enables the DCXO test mode (available only during FM mode)
    //                        in this mode frequency pulling via FM_DCXOPull is enabled. FM_DCXOTest = 0
    //                        enables normal operation
    CRegField FM_DCXOTest;
    // DCXO frequency select: Select DCXO lower/upper test frequency during DCXO test mode. FM_DCXOPull = 1
    //                        sets the DCXO to the lower DCXO frequency. FM_DCXOPull = 0 sets the DCXO to its 
    //                        higher frequency.
    CRegField FM_DCXOPull;
    // Ident Control register field: FM_DCXOPull & FM_DCXOTest & FM_ItegrateTime & FM_Area  
    CRegField IdentControl;
    // Channel 2 receive mode 0: These bits control the hardware for the second sound carrier. NICAM mode employs
    //                           a wider bandwidth of the decimation filters than FM mode
    // 0    0  FM
    // 0    1  AM
    // 1    0  NICAM
    CRegField Channel2_Mode0;
    // Channel 1 receive mode: CHANNEL1_Mode = 1 selects the hardware for the first sound carrier to operate in
    //                         AM mode. CHANNEL1_Mode = 0, FM mode is assumed. This applies to both terrestrial 
    //                         and satellite FM reception
    CRegField Channel1_Mode;
    // selects filter bandwidth
    //  0   0   narrow/narrow         recommended for nominal terrestrial broadcast conditions and SAT with 2 carriers
    //  0   1   extra wide/ narrow    recommended for highly overmodulated single FM carriers. Only channel 1 is 
    //                                available for FM demodulation in this mode. 
    //                                On channel 2 NICAM can still be processed
    //  1   0   medium/medium         recommended for moderately overmodulated broadcast conditions
    //  1   1   wide/wide             recommended for strongly overmudulated broadcast conditions
    CRegField Filter_BandwithMode;
    // Channel 2 receive mode 1: These bits control the hardware for the second sound carrier. NICAM mode employs
    //                           a wider bandwidth of the decimation filters than FM mode
    // 0    0  FM
    // 0    1  AM
    // 1    0  NICAM
    CRegField Channel2_Mode1;
    // Demodulator configuration: Channel2_Mode1 & Filter_BandwithMode & Channel1_Mode & Channel2_Mode0
    CRegField DemodulatorConf;
    CRegister AGCGainSelect;
    // If the automatic gain control function is switched off in the general configuration register, the 
    // contents of this register will define a fixed gain of the AGC stage. (See Table 13 in munit_audio)
    CRegField AGC_FixedGain;
    // AGC decay time: AGC_DecayTime = 1, a longer decay time and larger hysteresis are selected for
    //                 input signals with strong viedo modulation (conventional intercarrier). This bit 
    //                 has only an effect, when bit AGC_Mode = 0. AGC_DecayTime = 0, selects normal attack and 
    //                 decay times for AGC and a small hysteresis
    CRegField AGC_DecayTime;
    // AGC on or off. AGC_Mode = 1 forces the AGC block to a fixed gain as defined in the AGC Gain
    //                AGC_Mode = 0 the automatic gain control function is enabled and the contents of the
    //                             AGC gain register is ignored.
    CRegField AGC_Mode;
    CRegister CarrierOneFrequ;
    // These three bytes define a frequency control word ro represent the sound carrier (i.e. mixer)
    // Execution of the command starts only after all bytes have been received. If an error occurs, e.g.
    // a premature STOP condition, parital data for this function are ignored.
    CRegField CarrierOne_Data;
    CRegister CarrierTwoFrequ;
    // These three bytes define a frequency control word ro represent the sound carrier (i.e. mixer)
    // Execution of the command starts only after all bytes have been received. If an error occurs, e.g.
    // a premature STOP condition, parital data for this function are ignored.
    CRegField CarrierTwo_Data;
    CRegister AudioFormat;
    // Number of audio samples: The logical value of the lines corresponds to the PC's memory size of the 
    // audio data field organized in 32 bit words.
    CRegField Audio_SampleCount;
    // Output format to DMA block in two's complement
    CRegField Audio_OutputFormat;
    // Samples relevance and transfer frequency (DMA Block)
    //  0    0    0   both samples are stored (single) in a 32 bit DMA
    //  1    0    1   any first (left) of two samples (stereo) is stored twice in the 32 bit DMA
    //  1    1    0   any second (right) of two samples (stereo) is stored twice
    CRegField Audio_SampleRel;
    // Selects imput source for DMA
    //  0    0  Raw FM data (Mixer/Decimator) is input source
    //  0    1  BTSC data (Demodulator) is input source
    //  1    0  Baseband audio (Audio ADC's) is input source
    //  1    1  Dual-FM (Audio-DSP) is input source
    CRegField Audio_InputSource;
    CRegister MonitorSelect;
    // Signal Source select
    //  0   0  DC output of FM/AM demodulator
    //  0   1  magnitude/phase of FM/AM demodulator
    //  1   0  FM/AM path output
    //  1   1  NICAM path output
    CRegField Monitor_SignalSource;
    // Signal Channel select:
    // 0    0  (Channel 1 + Channel 2) / 2
    // 0    1  Channel 1
    // 1    0  Channel 2
    CRegField Monitor_SignalChannel;
    // Peak Level select: Monitor_PeakMode = 1 selects the rectified peak level of a source to be monitored.
    //                    Peak level value is reset to 0 after read out.
    CRegField Monitor_PeakLevel;
    // FM de-emphasis: The State of these 3 bits determine the FM de-emphasis for sound carrier 1
    // 0   0   0    50 mikros
    // 0   0   1    60 mikros
    // 0   1   0    75 mikros
    // 0   1   1    J17
    // 1   0   0    off
    CRegField FM_DeEmphasisOne;
    // Adaptive de-emphasis on/off: FM_AdaptiveModeOne = 1 activates the adaptive de-emphasis function
    // which is required for certain satellite FM channels. The standard FM de-emphasis must then be set
    // to 75 mikros, FM_AdaptiveModeOne = 0, the adaptive de-emphasis is off
    CRegField FM_AdaptiveModeOne;
    // FM de-emphasis: The state of these 3 bits determine the FM de-emphasis for sound carrier 2
    // 0   0   0    50 mikros
    // 0   0   1    60 mikros
    // 0   1   0    75 mikros
    // 0   1   1    J17
    // 1   0   0    off
    CRegField FM_DeEmphasisTwo;
    // Adaptive de-emphasis on/off: FM_AdaptiveModeTwo = 1 activates the adaptive de-emphasis function
    // which is required for certain satellite FM channels. The standard FM de-emphasis must then be set
    // to 75 mikros, FM_AdaptiveModeTwo = 0, the adaptive de-emphasis is off
    CRegField FM_AdaptiveModeTwo;
    // De-Emphasis characteristics: FM_AdaptiveModeTwo & FM_DeEmphasisTwo & FM_AdaptiveModeOne & FM_DeEmphasisOne
    CRegField FM_DeEmphasis;
    // Dematrix characteristics select: The state of these 3 bits select the dematrixing characteristics
    //                        L Output        R Output        Mode
    // 0   0   0                CH1              CH1           mono 1
    // 0   0   1                CH2              CH2           mono 2
    // 0   1   0                CH1              CH2           dual
    // 0   1   1                CH2              CH1           dual swapped
    // 1   0   0              2*CH1 - CH2        CH2           Stereo europe
    // 1   0   1          (CH1 + CH2) / 2    (CH1 - CH2) / 2   stereo Korea - 6 dB
    // 1   1   0              CH1 + CH2        CH1 - CH2       stereo Korea
    CRegField FM_ManualDematrixMode;
    // Automatic FM-dematrix switching: If set to logic 1, the FM dematrix is switched 
    // automatically in dependence on the current FM identification result. In case of stereo
    // the type of stereo dematrixing (Europe or Korea) is determined by bit FM_Area ind subadress
    // 0x0A. FM_DematrixState is ignored and dematix and set according the table below. With channel 2 in 
    // NICAM mode, always mono for channel 1 is selected
    //  Identification Mode           L Output      R Output
    //    Mono                          CH1           Ch1
    //    Stereo Europe              2*CH1 - CH2      CH2
    //    Stereo Korea                 CH1 + CH2    CH1 - CH2
    //    Dual                          CH1            CH2
    CRegField FM_AutoDematrixMode;
    // De-Matrix characteristics: FM_AutoDematrixMode & FM_ManualDematrixMode
    CRegField FM_Dematrix;
    // Channel 1 output Level adjust: This register is used to correct for standard and station dependent
    // differences of signal levels. The gain settings cn be chosen from 0 dB to +15 dB 
    // (FM_OutputLevelOne = 00000b to 01111b) and from -15 dB to -1 dB (FM_OutputLevelOne = 10000b to 11110b).
    // FM_OutputLevelOne = 11111b is not defined
    CRegField FM_OutputLevelOne;
    CRegister NICAMConfig;
    // Channel 2 output Level adjust: This register is used to correct for standard and station dependent
    // differences of signal levels. The gain settings cn be chosen from 0 dB to +15 dB 
    // (FM_OutputLevelTwo = 00000b to 01111b) and from -15 dB to -1 dB (FM_OutputLevelTwo = 10000b to 11110b).
    // FM_OutputLevelTwo = 11111b is not defined
    CRegField FM_OutputLevelTwo;
    // Auto muting on/off: NICAM_AutoMuteMode = 1, automatic muting is disabled. This bit has only an effect, when the 
    // second sound carrier is set to NICAM. NICAM_AutoMute = 0, enables the automatic switching between NICAM 
    // and the program on the first sound carrier (i.e. FM mono or AM), dependent on the NICAM bit error rate. The
    // FM dematrix should be set to the mono position or FM_AutoDematrixMode should be set.
    CRegField NICAM_AutoMuteMode;
    // De-emphasis on/off. NICAM_DeEmphasisMode = 1 switches the J17 de-emphasis off.
    // NICAM_DeEmphasisMode = 0 switches the J17 de-emphasis on.
    CRegField NICAM_DeEmphasisMode;
    // Auto-mute select. NICAM_AutoMuteSelect = 1, the auto mute will switch between NICAM sound and the analog
    // mono input. This bit has only an effect when the auto-mute function is enabled and when the DAC has been
    // selected in the Analog Output Select Register, NICAM_AutoMuteSelect = 0, the auto-mute will switch between 
    // NICAM sound on the first sound carrier (i.e. FM mono of AM)
    CRegField NICAM_AutoMuteSelect;
    // NICAM configuration: NICAM_AutoMuteSelect & NICAM_DeEmphasisMode & NICAM_AutoMuteMode
    CRegField NICAM_Configuration;
    //  This register is used to correct for standard and station dependent
    // differences of signal levels. The gain settings cn be chosen from 0 dB to +15 dB 
    // (NICAM_OutputLevel = 00000b to 01111b) and from -15 dB to -1 dB (NICAM_OutputLevel = 10000b to 11110b).
    // NICAM_OutputLevel = 11111b is not defined
    CRegField NICAM_OutputLevel;
    // Signal Source left and right:
    //                    LEFT          RIGHT
    //  0    0             FM/AM         FM/AM
    //  0    1             NICAM left     NICAM right
    //  1    0             FM/AM          NICAM M1
    //  1    1             FM/AM          NICAM M2
    CRegField Signal_SourceSelect;
    // Automatic Volume Level control function 
    //   0    0   off/reset
    //   0    1   short decay(2 s)
    //   1    0   medium decay(4 s)
    //   1    1   long decay(8 s)
    CRegField Signal_Volumecontrol;
    // Selection of stereo gain
    // 0   0    0 dB
    // 0   1    3 dB
    // 1   0    6 dB
    // 1   1    9 dB
    CRegField Signal_StereoGain;
    // Stereo DAC: Signal_StereoGain & Signal_Volumecontrol & Signal_SourceSelect
    CRegField Stereo_DAC_Select;
    CRegister DigitalInterface;
    // I2S output mode: I2S_OutputMode = 1 enables the output of serial audio data (2 pins) plus serial
    //        bit clock and word select in a format determined by the I2S_OutputFormat bit. The device
    //        then is an I2S-bus master. I2S_OutputMode = 0, the outputs mentioned will be 3-stated, thereby
    //        improving EMC performance.
    CRegField I2S_OutputMode;
    // I2S output format: I2S_OutputFormat = 1 selects an MSB-aligned, MSB-first output format, i.e. a level change
    //        at the word select ping indicates the beginning of a new audio sample, I2S_OutputFormat = 0 
    //        selects the standard I2S output format
    CRegField I2S_OutputFormat;
    // Digital configuration: I2S_OutputFormat & I2S_OutputMode 
    CRegField DigitalConfig;
    // Signal source I2S Output (ignoring ISS2)
    //                    LEFT          RIGHT
    //  0    0             FM/AM         FM/AM
    //  0    1             NICAM left     NICAM right
    //  1    0             FM/AM          NICAM M1
    //  1    1             FM/AM          NICAM M2
    CRegField I2S_SignalSource;
    // Output channel selection mode
    //                         L Output     R Output
    //  0   0   0                 L input     R input
    //  0   0   1                 L input     L input
    //  0   1   0                 R input     R input
    //  0   1   1                 R input     L input
    //  1   0   0                (L + R)/2   (L + R)/2
    CRegField I2S_OutputChannelMode;
    // Auto select function for TV applications if set HIGH.
    CRegField I2S_TVAutoSelect;
    // I2S output select: I2S_TVAutoSelect & I2S_OutputChannelMode & I2S_SignalSource
    CRegField I2S_OutputSelect;
    //  This register is used to correct for standard and station dependent
    // differences of signal levels. The gain settings cn be chosen from 0 dB to +15 dB 
    // (I2S_OutputLevel = 00000b to 01111b) and from -15 dB to -1 dB (I2S_OutputLevel = 10000b to 11110b).
    // I2S_OutputLevel = 11111b is not defined
    CRegField I2S_OutputLevel;
    // Analog output channel selection mode
    //                         L Output     R Output
    //  0   0   0                 L input     R input
    //  0   0   1                 L input     L input
    //  0   1   0                 R input     R input
    //  0   1   1                 R input     L input
    //  1   0   0                (L + R)/2   (L + R)/2
    CRegField Analog_OutputChannelMode;
    // Auto select function for TV applications if set HIGH.
    CRegField Analog_TVAutoSelect;
    // DSP output select: Analog_TVAutoSelect & Analog_OutputChannelMode
    CRegField DSP_OutputSelect;
    CRegister AudioControl;
    // Audio mute control
    // Only bits 6 and 2 are used, all other bits should be set to logic 1. When any of these bits is set
    // HIGH the corresponding pair of output channels will be muted. A LOW bit allows normal signal ouptut
    //  Bit 6 = 1  Mute I2S output
    //  Bit 2 = 1  Mute Stereo output
    CRegField Audio_MuteControl;
    // Audio mute control separation
    CRegField Audio_MuteOut;
    CRegField Audio_MuteI2S;
    // Sample frequency select for audio. See table:
    // 0 0 32 kHz; SIF
    // 0 1 32 kHz; Baseband
    // 1 0 44.1 kHz; Baseband
    // 1 1 48 kHz; Baseband
    CRegField SIF_SampleFreq;
    // Enables DC cancellation filter of decimationfilter
    CRegField DC_FilterEnable;
    // SIF clock phase shift. Shift the ADC clock in steps of 10ns against audio frontend clock
    CRegField SIF_ClockPhaseShift;
    // SIF gain selection; 00 -> 0dB, 01 -> -6dB, 10 -> -10dB
    CRegField SIF_GainSelect;
    // SIF control: SIF_GainSelect & SIF_ClockPhaseShift & DC_FilterEnable & SIF_SampleFreq 
    CRegField SIF_Control;
    // Output Crossbar Select. Selects between analog inputs and DAC output. See table 41 for details. 
    // If Auto-select function is enabled, the crossbar is switched automatically to external mono input 2 (101b).
    //         ANALOG OUTPUT LEFT    ANALOG OUTPUT RIGHT
    // 0 0 0   EXTIL1                EXTIR1
    // 0 0 1   EXTIL2                EXTIR2
    // 0 1 0   DAC L output          DAC R output
    // 0 1 1   EXTIL1                EXTIL1
    // 1 0 0   EXTIR1                EXTIR1
    // 1 0 1   EXTIL2                EXTIL2
    // 1 1 0   EXTIR2                EXTIR2
    CRegField OutputCrossbarSelect;
    // Input Crossbar Select. A low selects analog input EXTIL/R1, a high EXTIL/R2.
    CRegField InputCrossbarSelect;
    // Voltage Input Select for analog input EXTI1 (VSEL0) and EXTI2 (VSEL1). A HIGH
    // selects 2.0Vrms, a LOW 1.0Vrms input voltage.
    CRegField VoltageInputSelect;
    // Analog input/output control: VoltageInputSelect & InputCrossbarSelect & OutputCrossbarSelect 
    CRegField Analog_IO_Control;
    CRegister AudioClock;
    // The audio clock is synthesized from the same crystal frequency as the line-locked video clock is generated.
    // ACNI[21:0] = round (audiofreq / crystalfreq * 2^23) => Audio master Clocks Nominal Increment
    // Example: audiofreq = 6.144 MHz and crystalfreq = 32.11 MHz => ACNI = 187DE7h
    CRegField AudioClockNominalInc;
    // ensures correct datatransfer of ACNI to PLL if SWLOOP=1
    CRegField Audio_PLL_SWloop;
    // opens PLL
    CRegField Audio_PLL_Open;
    // Audio PLL control: Audio_PLL_SWloop & Audio_PLL_Open 
    CRegField Audio_PLL_Control;
    CRegister Audio_CPF;
    // An audio sample clock, that is locked to the field frequency, makes sure that there is always the 
    // same predefined number of audio samples associated with a field, or a set of fields.
    // ACPF = round (audiofreq / fieldfreq) => Audio master Clocks Per Field
    CRegField AudioClocksPerField;
  };



#endif _MUNITAUDIO_
