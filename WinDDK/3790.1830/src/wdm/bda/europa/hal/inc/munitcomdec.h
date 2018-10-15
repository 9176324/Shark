//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!! NOTE : Automatic generated FILE - DON'T CHANGE directly             
//!!        You only may change the file Saa7134_RegDef.h                
//!!        and call GenRegIF.bat for update, check in the necessary     
//!!        files before                                                 
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#ifndef _MUNITCOMDEC_INCLUDE_
#define _MUNITCOMDEC_INCLUDE_


#include "SupportClass.h"


class CMunitComDec
{
public:
//constructor-destructor of class CMunitComDec
    CMunitComDec ( PBYTE );
    virtual ~CMunitComDec();


 //Register Description of this class


    CRegister VideoDecoder;
    // This register contains the current version identification number of the 
    // chip. initial version: 0001
    CRegField ChipVer;
    // The programming of the horizontal increment delay is used to match internal
    // processing delays to the delay of the AD-converter. Use recommenden position only
    // IDEL3 IDEL2 IDEL1 IDEL0
    //   1     1     1     1    : no update
    //   1     1     1     0    : min delay
    //   1     0     0     0    : recommended position
    //   0     0     0     0    : max delay
    CRegField HIncDel;
    // Update hysteeresis for 9-bit gain
    // GUDL1 GUDL0
    //   0    0    : OFF
    //   0    1    : +- 1LSB
    //   1    0    : +- 2LSB
    //   1    1    : +- 3LSB
    CRegField GainUpdL;
    // White Peak Control
    // 0: White peak control active (AD-signal is attenuated, if 
    //    nominal luminance output white level is exeeded
    // 1: White peak control disabled
    CRegField WPOff;  
    // Analog Control
    // Mode  1: CVBS (automatic gain) from AI11
    // Mode  2: CVBS (automatic gain) from AI12
    // Mode  3: CVBS (automatic gain) from AI21
    // Mode  4: CVBS (automatic gain) from AI22
    // Mode  5: CVBS (automatic gain) from AI23
    // Mode  6: Y (automatic gain) from AI11 + C(gain adjusted via GAI2 from AI11)
    // Mode  7: Y (automatic gain) from AI12 + C(gain adjusted via GAI2 from AI12) 
    // Mode  8: Y (automatic gain) from AI21 + C(gain adjusted via GAI2 from AI21)
    // Mode  9: Y (automatic gain) from AI22 + C(gain adjusted via GAI2 from AI22)
    // Mode 10: reserved
    // Mode 11: reserved
    // Mode 12: reserved
    // Mode 13: reserved
    // Mode 14: reserved
    // Mode 15: reserved
    CRegField AnCtrMode;
    // Analog function select FUSE
    // 00: Amplifier plus anti-alias filter bypassed
    // 01: Amplifier plus anti-alias filter bypassed
    // 10: Amplifier active
    // 11: Amplifier plus anti-alias filter active (recommended position)
    CRegField Fuse;   
    // Sign bit of gain control for channel 1
    CRegField Gain1_8;
    // Sign bit of gain control for channel 2
    CRegField Gain2_8;
    // Gain control fix
    CRegField GainFix;
    // Automatic gain control integration
    // 0: AGC active
    // 1: AGC integration hold (frozen)
    CRegField HoldGain;
    // Color peak off
    // 0: Color peak control active (AD-signal is attenuated, if maximum
    //    input level is exeeded, avoids clipping effects on screen(
    // 1: Color peak off
    CRegField ColPeakOff;
    // AGC hold during Vertical blanking period (VBSL)
    // 0: Short vertical blanking (AGC disabled during equalization- and serration(??)
    //    pulses, recommended setting
    // 1: Long vertical blanking (AGC disabled from start of pre euqlization pulses
    //    until start of active video (line 22 for 60 Hz, line 24 for 50 Hz)
    CRegField VBlankSL;
    // HL not reference select (HLNRS)
    // 0: Normal clamping if decoder is in unlocked state
    // 1: Reference select if decoder is in unlocked state
    CRegField HLNoRefSel;
    // Analog test environment
    // 0: inactive
    // 1: active
    CRegField Test;   
    CRegister GainControl;
    // Analog test environment register field: active, when 'Test' == 1
    CRegField AnalogTest;
    // Gain control analog channel 1
    CRegField Gain1_0;
    // Gain control analog channel 2
    CRegField Gain2_0;
    // Horizontal sync start
    CRegField HSBegin;
    // Horizontal sync stop
    CRegField HSStop; 
    CRegister SyncControl;
    // Vertical noise reduction (VNOI)
    // VNOI1  VNOI0 
    //   0      0    : Normal mode (recommended setting)
    //   0      1    : Fast mode (applicable for stable sources only,
    //                 automatic field detection [AUFD] must be disabled)
    //   1      0    : Free running mode
    //   1      1    : Vertical noise reduction bypassed
    CRegField VNoiseRed;
    // Horizontal PLL
    // 0: PLL closed
    // 1: PLL open, horizontal frequency fixed
    CRegField HPll;   
    // Horizontal time constant selection
    // HTC1  HTC0
    //  0     0   : TV Mode (recommended for poor quality TV signals only,
    //              do not use for new applications
    //  0     1   : VTR mode (recommended if a deflection control circuit
    //              is directly connected at the output of the decoder
    //  1     0   : reserved
    //  1     1   : fast lockin mode (recommended setting)
    CRegField HTimeCon;
    // Forced ODD/EVEN toggle
    // 0: ODD/EVEN signal toggles only with interlaced source
    // 1: ODD/EVEN signal toggles fieldwise even if source is non-interlaced
    CRegField ForceOETog;
    // Field selection
    // 0: 50 Hz, 625 lines
    // 1: 60 Hz, 525 lines
    CRegField FSel;   
    // Automatic field detection 
    // 0: Field state directly controlled via FSEL
    // 1: Automatic field detection (recommended setting)
    CRegField AutoFDet;
    // Sharpness Control, Luminance filter charachteristic
    // LUFI3 LUFI2 LUFI1 LUFI0
    //   0     0     0     0    : PLAIN
    //   0     0     0     1    : resolution enhancement filter 8.0  dB at 4.1 MHz
    //   0     0     1     0    : resolution enhancement filter 6.8  dB at 4.1 MHz
    //   0     0     1     1    : resolution enhancement filter 5.1  dB at 4.1 MHz
    //   0     1     0     0    : resolution enhancement filter 4.1  dB at 4.1 MHz
    //   0     1     0     1    : resolution enhancement filter 3.0  dB at 4.1 MHz
    //   0     1     1     0    : resolution enhancement filter 2.3  dB at 4.1 MHz
    //   0     1     1     1    : resolution enhancement filter 1.6  dB at 4.1 MHz
    //   1     0     0     0    : low pass filter 3 dB at 4.1 MHz
    //   1     0     0     1    : low pass filter 3 dB at 4.1 MHz
    //   1     0     1     0    : low pass filter 3 dB at 3.3 MHz, 4  dB at 4.1 MHz
    //   1     0     1     1    : low pass filter 3 dB at 2.6 MHz, 8  dB at 4.1 MHz
    //   1     1     0     0    : low pass filter 3 dB at 2.4 MHz, 14 dB at 4.1 MHz
    //   1     1     0     1    : low pass filter 3 dB at 2.2 MHz, notch at 3.4 MHz
    //   1     1     1     0    : low pass filter 3 dB at 1.9 MHz, notch at 3.0 MHz
    //   1     1     1     1    : low pass filter 3 dB at 1.7 MHz, notch at 2.5 MHz
    CRegField YFilter;
    // Remodulation bandwidth for luminance 
    // 0: Small remodulation bandwidth
    //    narrow chroma notch => higher  luminance bandwidth
    // 1: Large remodulation bandwidth
    //    wider chroma notch  => smaller luminance bandwidth 
    CRegField YBandw; 
    // Processing delay in non combfilter mode
    // 0: Processing delay is equal to internal pipelining delay
    // 1: One (NTSC-standards) or two (PAL-standards) video lines additional
    //    processing delay
    CRegField LDel;   
    // Adaptive luminance comb filter 
    // 0: Disabled (=chrominance trap enabled, if BYPS = 0)
    // 1: Active, if BYPS = 0
    CRegField YComb;  
    // Chrominance trap/comb filter bypass
    // 0: Chrominance trap or luminance comb filter active, default for CVBS mode
    // 1: Chrominance trap or luminance comb filter bypassed, default for S-Video mode
    CRegField CByps;  
    // Luminance Brightness adjustment
    CRegField DecBri; 
    // Luminance Contrast adjustment
    CRegField DecCon; 
    CRegister ChromaControl_1;
    // Chroma Saturation adjustment
    CRegField DecSat; 
    // Chroma Hue control
    CRegField HueCtr; 
    // Adaptive chrominance comb filter
    // 0: Disabled
    // 1: Active
    CRegField CComb;  
    // Automatic chrominance standard detection control 0 (AUTO0)
    // AUTO1 is lockated at offset 14h, D2
    // AUTO1 AUTO0 
    //  0     0    : Disabled
    //  0     1    : Active, filter settings and Sharpness control are preset to 
    //               default values according to the detected standard and mode 
    //               (recommended position)
    //  1     0    : Active, filter settings are preset to default values according
    //               to the detected standard and mode
    //  1     1    : Active, but no filter presets
    CRegField Auto0;  
    // Fast colour time constant (FCTC)
    // 0: Nominal time constant
    // 1: Fast time constant for special applications (High quality input source, 
    //    fast chroma lock required, automatic standard detection OFF
    CRegField FColTC; 
    // Chrominance vertical filter
    // 0: PAL phase error correction on (during active video lines)
    // 1: PAL phase error correction permanently off
    CRegField DCVFil; 
    // Color standard selection in non AUTO-mode
    // Number      50 Hz/625 lines              60 Hz/525 lines
    //---------------------------------------------------------
    //  000   :    PAL BGDHI(4.43 MHz)          NTSC M(3.58 MHz)
    //  001   :    NTSC 4.43                    PAL 4.43
    //  010   :    Combination PAL N (3.58 MHz) NTSC 4.43
    //  011   :    NTSC N(3.58 MHz)             PAL M(3.58 MHz)
    //  100   :    reserved                     NTSC Japan (3.58 MHz)
    //  101   :    SECAM                        reserved
    //  110   :    reserved                     reserved
    //  111   :    reserved                     reserved
    CRegField CStdSel;
    // Clear DTO
    // 0: Disabled
    // 1: Every time CDTO is set, the internal subcarrier DTO phase is reset to 0 degrees
    //    and the RTCO output generates a logic 0 at time slot 68). So an identical 
    //    subcarrier phase can be generated by an external device (e.g. an encoder)
    CRegField ClearDTO;
    // Chroma gain control
    CRegField CGain;  
    // Automatic Chroma gain Control
    // 0: ON (recommended setting)
    // 1: programmable gain via CGAIN
    CRegField AutoCGCtr;
    CRegister ChromaControl_2;
    // Combined Luminance/Chrominance bandwidth adjustment
    CRegField YCBandw;
    // Chrominance bandwidth
    // 0: small
    // 1: wide
    CRegField CBandw; 
    // Fine offset adjustment for R-Y component
    // 00 :   0     LSB
    // 01 :   +1/4  LSB
    // 10 :   +1/2  LSB
    // 11 :   +3/4  LSB
    CRegField OffV;   
    // Fine offset adjustment B-Y component
    // 00 :   0     LSB
    // 01 :   +1/4  LSB
    // 10 :   +1/2  LSB
    // 11 :   +3/4  LSB
    CRegField OffU;   
    // Luminance Delay compensation
    CRegField YDel;   
    // Fine position of HS, step size of 2/LLC
    // 00 : 0
    // 01 : 1
    // 10 : 2
    // 11 : 3 (should be obvious)
    CRegField HSDel;  
    // Colour on
    // 0: Automatic colour killer enabled (recommended setting)
    // 1: Colour forced on
    CRegField ColOn;  
    CRegister MiscControl;
    // ACD sample clock phase delay
    CRegField APhClk; 
    // Documented under ChromaControl_1::AUTO0
    CRegField Auto1;  
    // Update time interval for AGC-value
    // 0 : Horizontal update (once per line)
    // 1 : Vertical update (once per line)
    CRegField UpdTCV; 
    // Compatibility bit for SAA7199
    // 0: off (default)
    // 1: on (to be set only if SAA7199 is used for
    //    re-encoding in conjunction with RTCO active)
    CRegField Cm99;   
    // Start of VGATE pulse (01 transition) and polarity change of FID-pulse
    CRegField VgStart0;
    // Stop of VGATE pulse (10 transition)
    CRegField VgStop0;
    // MSB VGATE start
    CRegField VgStart8;
    // MSB VGATE stop
    CRegField VgStop8;
    // Alternative VGATE position
    // 0 : VGATE position accordint to tables 23 and 24
    // 1 : VGATE occurs one line earlier during field 1
    CRegField VgPos;  
    // Standard detection search loop latency
    // 000 = reserved, else field number
    CRegField Latency;
    CRegField LLClkEn;
    CRegister RawDataGain;
    // Raw data gain control
    CRegField RawGain;
    // Raw data offset control
    CRegField RawOffs;
    // SECAM color killer level
    CRegField SThr;   
    // PAL/NTSC color killer level
    CRegField QThr;   
    // Fast Sequence Correction
    CRegField FSqC;   
    // Automatic color limiter
    CRegField ACoL;   
    CRegister StatusByteDecoder;
    // detected color standard
    // 00 : No color (BW)
    // 01 : NTSC
    // 10 : PAL
    // 11 : SECAM
    CRegField DetColStd;
    // white peak loop is activated, active HIGH
    CRegField WPAct;  
    // gain value for active luminance channel is limited, active high
    CRegField GainLimBot;
    // gain value for active luminance channel is limited, active high
    CRegField GainLimTop;
    // slow time constant active in WIPA mode, active high
    CRegField SlowTCAct;
    // status bit for locked horizontal frequency, LOW = locked, HIGH = unlocked
    CRegField HLock;  
    // Ready for Capture(all internal loops locked), active HIGH
    CRegField RdCap;  
    // Copy protected source detected according to Macrovision version up to 7.01
    CRegField MVCopyProt;
    // Macrovision encoded Colorstripe any type detected
    CRegField MVColStripe;
    // Macrovision encoded Colorstripe burst type 3 (4 line version) detected
    CRegField MVProtType3;
    // identification bit for detected field frequency, LOW 50 Hz, HIGH 60 Hz
    CRegField FidT;   
    // status bit for horizontal and vertical loop : 
    // LOW  = both loops locked
    // HIGH = unlocked
    CRegField HVLoopN;
    // status bit for interlace detection
    // LOW  = noninterlaced
    // HIGH = interlaced
    CRegField Intl;   
  };



#endif _MUNITCOMDEC_
