//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!! NOTE : Automatic generated FILE - DON'T CHANGE directly             
//!!        You only may change the file Saa7134_RegDef.h                
//!!        and call GenRegIF.bat for update, check in the necessary     
//!!        files before                                                 
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#define MUNITSCALER_OFFS 0x0

//*************************************************************
//------------- MunitScaler ----------------------
struct _MUNITSCALER_ {  
#define SOURCEDEPENDENT_OFFS 0x0

  union _SOURCEDEPENDENT_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :17;
        // field ID polarity: 0 as is, 1 inverted
        U32 DFid                 :1;
        // define vertical trigger edge : 0: rising edge of input V-reference
        U32 DVEdge               :1;
        // define horizontal trigger edge:
        // 0: pixel counter at rising (trailing) / line counter at falling (leading) edge
        // 1: vice versa
        U32 DHEdge               :1;
        // vertical trigger edge occurs at line YPOS
        U32 YPos                :12;
    # else /* !BIG_ENDIAN */
        // vertical trigger edge occurs at line YPOS
        U32 YPos                :12;
        // define horizontal trigger edge:
        // 0: pixel counter at rising (trailing) / line counter at falling (leading) edge
        // 1: vice versa
        U32 DHEdge               :1;
        // define vertical trigger edge : 0: rising edge of input V-reference
        U32 DVEdge               :1;
        // field ID polarity: 0 as is, 1 inverted
        U32 DFid                 :1;
        U32                      :17;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } SourceDependent; 

#define STATUS_1_OFFS 0x4

  union _STATUS_1_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // actual page flag
        U32 Page                 :1;
        // logical XOR of FIDSCO and FIDSCI, may be used to find
        // permanent problem with the field ID interpretation
        U32 ScoXorSci            :1;
        // field ID as assigned by the scaler output
        U32 FidScO               :1;
        // field ID as seen at the scaler input
        U32 FidScI               :1;
        // Was reset flag -  assigns, that scaler data path was resetted
        // (returns from IDLE stat), flag is reset with the next input V-sync
        U32 WasRst               :1;
        // Load error flag - assigns, that scaler data path is overloaded, due to 
        // missmatched I/O or internal data rates, flag is reset with the next input V-sync
        U32 LdErr                :1;
        // Configuration Error flag -  scaler data path not ready at start of a region
        // I/O window configuration to be checked, flag is reset with the next iput V-Sync
        U32 CfErr                :1;
        // Trigger Error flag - scaler fails to trigger for at least 2 fields
        // trigger conditions need to be checked, flag is reset with the next input V-Sync
        U32 TrErr                :1;
        U32                      :2;
        // 1: VBI region Task B active
        // 0: inactive
        U32 VbiStatB             :1;
        // 1: video region Task B active
        // 0: inactive
        U32 VidStatB             :1;
        U32                      :2;
        // 1: VBI region Task A active
        // 0: inactive
        U32 VbiStatA             :1;
        // 1: video region Task A active
        // 0: inactive
        U32 VidStatA             :1;
        U32                      :8;
        // 1: active software reset
        U32 SwRst                :1;
        U32                      :1;
        // 1: enable VBI region Task B, as defined by DXSTxx and DYSTxx parameters
        // 0: disable
        U32 VbiEnB               :1;
        // 1: enable video region Task B, as defined by VXSTxx and VYSTxx parameters
        // 0: disable
        U32 VidEnB               :1;
        U32                      :2;
        // 1: enable VBI region Task A, as defined by DXSTxx and DYSTxx parameters
        // 0: disable
        U32 VbiEnA               :1;
        // 1: enable video region Task A, as defined by VXSTxx and VYSTxx parameters
        // 0: disable
        U32 VidEnA               :1;
    # else /* !BIG_ENDIAN */
        // 1: enable video region Task A, as defined by VXSTxx and VYSTxx parameters
        // 0: disable
        U32 VidEnA               :1;
        // 1: enable VBI region Task A, as defined by DXSTxx and DYSTxx parameters
        // 0: disable
        U32 VbiEnA               :1;
        U32                      :2;
        // 1: enable video region Task B, as defined by VXSTxx and VYSTxx parameters
        // 0: disable
        U32 VidEnB               :1;
        // 1: enable VBI region Task B, as defined by DXSTxx and DYSTxx parameters
        // 0: disable
        U32 VbiEnB               :1;
        U32                      :1;
        // 1: active software reset
        U32 SwRst                :1;
        U32                      :8;
        // 1: video region Task A active
        // 0: inactive
        U32 VidStatA             :1;
        // 1: VBI region Task A active
        // 0: inactive
        U32 VbiStatA             :1;
        U32                      :2;
        // 1: video region Task B active
        // 0: inactive
        U32 VidStatB             :1;
        // 1: VBI region Task B active
        // 0: inactive
        U32 VbiStatB             :1;
        U32                      :2;
        // Trigger Error flag - scaler fails to trigger for at least 2 fields
        // trigger conditions need to be checked, flag is reset with the next input V-Sync
        U32 TrErr                :1;
        // Configuration Error flag -  scaler data path not ready at start of a region
        // I/O window configuration to be checked, flag is reset with the next iput V-Sync
        U32 CfErr                :1;
        // Load error flag - assigns, that scaler data path is overloaded, due to 
        // missmatched I/O or internal data rates, flag is reset with the next input V-sync
        U32 LdErr                :1;
        // Was reset flag -  assigns, that scaler data path was resetted
        // (returns from IDLE stat), flag is reset with the next input V-sync
        U32 WasRst               :1;
        // field ID as seen at the scaler input
        U32 FidScI               :1;
        // field ID as assigned by the scaler output
        U32 FidScO               :1;
        // logical XOR of FIDSCO and FIDSCI, may be used to find
        // permanent problem with the field ID interpretation
        U32 ScoXorSci            :1;
        // actual page flag
        U32 Page                 :1;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } Status_1; 

  U8 FillerAt0x8[0x4];

#define STARTPOINTS_OFFS 0xC

  union _STARTPOINTS_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :8;
        // RED Path  : start point for curve generation (straigth binary)
        U32 PGa2Start            :8;
        // BLUE Path : start point for curve generation (straight binary)
        U32 PGa1Start            :8;
        // GREEN Path: start point for curve generation (straight binary)
        U32 PGa0Start            :8;
    # else /* !BIG_ENDIAN */
        // GREEN Path: start point for curve generation (straight binary)
        U32 PGa0Start            :8;
        // BLUE Path : start point for curve generation (straight binary)
        U32 PGa1Start            :8;
        // RED Path  : start point for curve generation (straigth binary)
        U32 PGa2Start            :8;
        U32                      :8;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } StartPoints; 

#define PGA0_OFFS 0x10

// GREEN path: points for gamma curve calculation (straight binary)
// special meaning of point P0GA0: defines start point and first 
// curve point start at ... , first point at ...
U32 PGa0[4];

#define PGA1_OFFS 0x20

// BLUE path: points for gamma curve calculation (straight binary)
// special meaning of point P1GA0: defines start point and first 
// curve point start at ... , first point at ...
U32 PGa1[4];

#define PGA2_OFFS 0x30

// RED path: points for gamma curve calculation (straight binary)
// special meaning of point P2GA0: defines start point and first 
// curve point start at ... , first point at ...
U32 PGa2[4];

#define PAGE1_OFFS 0x40
  _PAGE_ Page1;
#define PAGE2_OFFS 0x7C
  _PAGE_ Page2;

} MunitScaler;

//--------- End of MunitScaler ----------------------
//*************************************************************



  U8 FillerAt0xB8[0x48];

#define MUNITCOMDEC_OFFS 0x100

//*************************************************************
//------------- MunitComDec ----------------------
struct _MUNITCOMDEC_ {  
#define VIDEODECODER_OFFS 0x0

  union _VIDEODECODER_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // Analog test environment
        // 0: inactive
        // 1: active
        U32 Test                 :1;
        // HL not reference select (HLNRS)
        // 0: Normal clamping if decoder is in unlocked state
        // 1: Reference select if decoder is in unlocked state
        U32 HLNoRefSel           :1;
        // AGC hold during Vertical blanking period (VBSL)
        // 0: Short vertical blanking (AGC disabled during equalization- and serration(??)
        //    pulses, recommended setting
        // 1: Long vertical blanking (AGC disabled from start of pre euqlization pulses
        //    until start of active video (line 22 for 60 Hz, line 24 for 50 Hz)
        U32 VBlankSL             :1;
        // Color peak off
        // 0: Color peak control active (AD-signal is attenuated, if maximum
        //    input level is exeeded, avoids clipping effects on screen(
        // 1: Color peak off
        U32 ColPeakOff           :1;
        // Automatic gain control integration
        // 0: AGC active
        // 1: AGC integration hold (frozen)
        U32 HoldGain             :1;
        // Gain control fix
        U32 GainFix              :1;
        // Sign bit of gain control for channel 2
        U32 Gain2_8              :1;
        // Sign bit of gain control for channel 1
        U32 Gain1_8              :1;
        // Analog function select FUSE
        // 00: Amplifier plus anti-alias filter bypassed
        // 01: Amplifier plus anti-alias filter bypassed
        // 10: Amplifier active
        // 11: Amplifier plus anti-alias filter active (recommended position)
        U32 Fuse                 :2;
        U32                      :2;
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
        U32 AnCtrMode            :4;
        U32                      :1;
        // White Peak Control
        // 0: White peak control active (AD-signal is attenuated, if 
        //    nominal luminance output white level is exeeded
        // 1: White peak control disabled
        U32 WPOff                :1;
        // Update hysteeresis for 9-bit gain
        // GUDL1 GUDL0
        //   0    0    : OFF
        //   0    1    : +- 1LSB
        //   1    0    : +- 2LSB
        //   1    1    : +- 3LSB
        U32 GainUpdL             :2;
        // The programming of the horizontal increment delay is used to match internal
        // processing delays to the delay of the AD-converter. Use recommenden position only
        // IDEL3 IDEL2 IDEL1 IDEL0
        //   1     1     1     1    : no update
        //   1     1     1     0    : min delay
        //   1     0     0     0    : recommended position
        //   0     0     0     0    : max delay
        U32 HIncDel              :4;
        // This register contains the current version identification number of the 
        // chip. initial version: 0001
        U32 ChipVer              :4;
        U32                      :4;
    # else /* !BIG_ENDIAN */
        U32                      :4;
        // This register contains the current version identification number of the 
        // chip. initial version: 0001
        U32 ChipVer              :4;
        // The programming of the horizontal increment delay is used to match internal
        // processing delays to the delay of the AD-converter. Use recommenden position only
        // IDEL3 IDEL2 IDEL1 IDEL0
        //   1     1     1     1    : no update
        //   1     1     1     0    : min delay
        //   1     0     0     0    : recommended position
        //   0     0     0     0    : max delay
        U32 HIncDel              :4;
        // Update hysteeresis for 9-bit gain
        // GUDL1 GUDL0
        //   0    0    : OFF
        //   0    1    : +- 1LSB
        //   1    0    : +- 2LSB
        //   1    1    : +- 3LSB
        U32 GainUpdL             :2;
        // White Peak Control
        // 0: White peak control active (AD-signal is attenuated, if 
        //    nominal luminance output white level is exeeded
        // 1: White peak control disabled
        U32 WPOff                :1;
        U32                      :1;
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
        U32 AnCtrMode            :4;
        U32                      :2;
        // Analog function select FUSE
        // 00: Amplifier plus anti-alias filter bypassed
        // 01: Amplifier plus anti-alias filter bypassed
        // 10: Amplifier active
        // 11: Amplifier plus anti-alias filter active (recommended position)
        U32 Fuse                 :2;
        // Sign bit of gain control for channel 1
        U32 Gain1_8              :1;
        // Sign bit of gain control for channel 2
        U32 Gain2_8              :1;
        // Gain control fix
        U32 GainFix              :1;
        // Automatic gain control integration
        // 0: AGC active
        // 1: AGC integration hold (frozen)
        U32 HoldGain             :1;
        // Color peak off
        // 0: Color peak control active (AD-signal is attenuated, if maximum
        //    input level is exeeded, avoids clipping effects on screen(
        // 1: Color peak off
        U32 ColPeakOff           :1;
        // AGC hold during Vertical blanking period (VBSL)
        // 0: Short vertical blanking (AGC disabled during equalization- and serration(??)
        //    pulses, recommended setting
        // 1: Long vertical blanking (AGC disabled from start of pre euqlization pulses
        //    until start of active video (line 22 for 60 Hz, line 24 for 50 Hz)
        U32 VBlankSL             :1;
        // HL not reference select (HLNRS)
        // 0: Normal clamping if decoder is in unlocked state
        // 1: Reference select if decoder is in unlocked state
        U32 HLNoRefSel           :1;
        // Analog test environment
        // 0: inactive
        // 1: active
        U32 Test                 :1;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } VideoDecoder; 

#define GAINCONTROL_OFFS 0x4

  union _GAINCONTROL_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // Horizontal sync stop
        U32 HSStop               :8;
        // Horizontal sync start
        U32 HSBegin              :8;
        // Gain control analog channel 2
        U32 Gain2_0              :8;
        // Analog test environment register field: active, when 'Test' == 1
        // Gain control analog channel 1
        U32 Gain1_0              :8;
    # else /* !BIG_ENDIAN */
        // Analog test environment register field: active, when 'Test' == 1
        // Gain control analog channel 1
        U32 Gain1_0              :8;
        // Gain control analog channel 2
        U32 Gain2_0              :8;
        // Horizontal sync start
        U32 HSBegin              :8;
        // Horizontal sync stop
        U32 HSStop               :8;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } GainControl; 

#define SYNCCONTROL_OFFS 0x8

  union _SYNCCONTROL_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // Luminance Contrast adjustment
        U32 DecCon               :8;
        // Luminance Brightness adjustment
        U32 DecBri               :8;
        // Chrominance trap/comb filter bypass
        // 0: Chrominance trap or luminance comb filter active, default for CVBS mode
        // 1: Chrominance trap or luminance comb filter bypassed, default for S-Video mode
        U32 CByps                :1;
        // Adaptive luminance comb filter 
        // 0: Disabled (=chrominance trap enabled, if BYPS = 0)
        // 1: Active, if BYPS = 0
        U32 YComb                :1;
        // Processing delay in non combfilter mode
        // 0: Processing delay is equal to internal pipelining delay
        // 1: One (NTSC-standards) or two (PAL-standards) video lines additional
        //    processing delay
        U32 LDel                 :1;
        // Remodulation bandwidth for luminance 
        // 0: Small remodulation bandwidth
        //    narrow chroma notch => higher  luminance bandwidth
        // 1: Large remodulation bandwidth
        //    wider chroma notch  => smaller luminance bandwidth 
        U32 YBandw               :1;
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
        U32 YFilter              :4;
        // Automatic field detection 
        // 0: Field state directly controlled via FSEL
        // 1: Automatic field detection (recommended setting)
        U32 AutoFDet             :1;
        // Field selection
        // 0: 50 Hz, 625 lines
        // 1: 60 Hz, 525 lines
        U32 FSel                 :1;
        // Forced ODD/EVEN toggle
        // 0: ODD/EVEN signal toggles only with interlaced source
        // 1: ODD/EVEN signal toggles fieldwise even if source is non-interlaced
        U32 ForceOETog           :1;
        // Horizontal time constant selection
        // HTC1  HTC0
        //  0     0   : TV Mode (recommended for poor quality TV signals only,
        //              do not use for new applications
        //  0     1   : VTR mode (recommended if a deflection control circuit
        //              is directly connected at the output of the decoder
        //  1     0   : reserved
        //  1     1   : fast lockin mode (recommended setting)
        U32 HTimeCon             :2;
        // Horizontal PLL
        // 0: PLL closed
        // 1: PLL open, horizontal frequency fixed
        U32 HPll                 :1;
        // Vertical noise reduction (VNOI)
        // VNOI1  VNOI0 
        //   0      0    : Normal mode (recommended setting)
        //   0      1    : Fast mode (applicable for stable sources only,
        //                 automatic field detection [AUFD] must be disabled)
        //   1      0    : Free running mode
        //   1      1    : Vertical noise reduction bypassed
        U32 VNoiseRed            :2;
    # else /* !BIG_ENDIAN */
        // Vertical noise reduction (VNOI)
        // VNOI1  VNOI0 
        //   0      0    : Normal mode (recommended setting)
        //   0      1    : Fast mode (applicable for stable sources only,
        //                 automatic field detection [AUFD] must be disabled)
        //   1      0    : Free running mode
        //   1      1    : Vertical noise reduction bypassed
        U32 VNoiseRed            :2;
        // Horizontal PLL
        // 0: PLL closed
        // 1: PLL open, horizontal frequency fixed
        U32 HPll                 :1;
        // Horizontal time constant selection
        // HTC1  HTC0
        //  0     0   : TV Mode (recommended for poor quality TV signals only,
        //              do not use for new applications
        //  0     1   : VTR mode (recommended if a deflection control circuit
        //              is directly connected at the output of the decoder
        //  1     0   : reserved
        //  1     1   : fast lockin mode (recommended setting)
        U32 HTimeCon             :2;
        // Forced ODD/EVEN toggle
        // 0: ODD/EVEN signal toggles only with interlaced source
        // 1: ODD/EVEN signal toggles fieldwise even if source is non-interlaced
        U32 ForceOETog           :1;
        // Field selection
        // 0: 50 Hz, 625 lines
        // 1: 60 Hz, 525 lines
        U32 FSel                 :1;
        // Automatic field detection 
        // 0: Field state directly controlled via FSEL
        // 1: Automatic field detection (recommended setting)
        U32 AutoFDet             :1;
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
        U32 YFilter              :4;
        // Remodulation bandwidth for luminance 
        // 0: Small remodulation bandwidth
        //    narrow chroma notch => higher  luminance bandwidth
        // 1: Large remodulation bandwidth
        //    wider chroma notch  => smaller luminance bandwidth 
        U32 YBandw               :1;
        // Processing delay in non combfilter mode
        // 0: Processing delay is equal to internal pipelining delay
        // 1: One (NTSC-standards) or two (PAL-standards) video lines additional
        //    processing delay
        U32 LDel                 :1;
        // Adaptive luminance comb filter 
        // 0: Disabled (=chrominance trap enabled, if BYPS = 0)
        // 1: Active, if BYPS = 0
        U32 YComb                :1;
        // Chrominance trap/comb filter bypass
        // 0: Chrominance trap or luminance comb filter active, default for CVBS mode
        // 1: Chrominance trap or luminance comb filter bypassed, default for S-Video mode
        U32 CByps                :1;
        // Luminance Brightness adjustment
        U32 DecBri               :8;
        // Luminance Contrast adjustment
        U32 DecCon               :8;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } SyncControl; 

#define CHROMACONTROL_1_OFFS 0xC

  union _CHROMACONTROL_1_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // Automatic Chroma gain Control
        // 0: ON (recommended setting)
        // 1: programmable gain via CGAIN
        U32 AutoCGCtr            :1;
        // Chroma gain control
        U32 CGain                :7;
        // Clear DTO
        // 0: Disabled
        // 1: Every time CDTO is set, the internal subcarrier DTO phase is reset to 0 degrees
        //    and the RTCO output generates a logic 0 at time slot 68). So an identical 
        //    subcarrier phase can be generated by an external device (e.g. an encoder)
        U32 ClearDTO             :1;
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
        U32 CStdSel              :3;
        // Chrominance vertical filter
        // 0: PAL phase error correction on (during active video lines)
        // 1: PAL phase error correction permanently off
        U32 DCVFil               :1;
        // Fast colour time constant (FCTC)
        // 0: Nominal time constant
        // 1: Fast time constant for special applications (High quality input source, 
        //    fast chroma lock required, automatic standard detection OFF
        U32 FColTC               :1;
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
        U32 Auto0                :1;
        // Adaptive chrominance comb filter
        // 0: Disabled
        // 1: Active
        U32 CComb                :1;
        // Chroma Hue control
        U32 HueCtr               :8;
        // Chroma Saturation adjustment
        U32 DecSat               :8;
    # else /* !BIG_ENDIAN */
        // Chroma Saturation adjustment
        U32 DecSat               :8;
        // Chroma Hue control
        U32 HueCtr               :8;
        // Adaptive chrominance comb filter
        // 0: Disabled
        // 1: Active
        U32 CComb                :1;
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
        U32 Auto0                :1;
        // Fast colour time constant (FCTC)
        // 0: Nominal time constant
        // 1: Fast time constant for special applications (High quality input source, 
        //    fast chroma lock required, automatic standard detection OFF
        U32 FColTC               :1;
        // Chrominance vertical filter
        // 0: PAL phase error correction on (during active video lines)
        // 1: PAL phase error correction permanently off
        U32 DCVFil               :1;
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
        U32 CStdSel              :3;
        // Clear DTO
        // 0: Disabled
        // 1: Every time CDTO is set, the internal subcarrier DTO phase is reset to 0 degrees
        //    and the RTCO output generates a logic 0 at time slot 68). So an identical 
        //    subcarrier phase can be generated by an external device (e.g. an encoder)
        U32 ClearDTO             :1;
        // Chroma gain control
        U32 CGain                :7;
        // Automatic Chroma gain Control
        // 0: ON (recommended setting)
        // 1: programmable gain via CGAIN
        U32 AutoCGCtr            :1;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } ChromaControl_1; 

#define CHROMACONTROL_2_OFFS 0x10

  union _CHROMACONTROL_2_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :16;
        // Colour on
        // 0: Automatic colour killer enabled (recommended setting)
        // 1: Colour forced on
        U32 ColOn                :1;
        U32                      :1;
        // Fine position of HS, step size of 2/LLC
        // 00 : 0
        // 01 : 1
        // 10 : 2
        // 11 : 3 (should be obvious)
        U32 HSDel                :2;
        U32                      :1;
        // Luminance Delay compensation
        U32 YDel                 :3;
        // Fine offset adjustment B-Y component
        // 00 :   0     LSB
        // 01 :   +1/4  LSB
        // 10 :   +1/2  LSB
        // 11 :   +3/4  LSB
        U32 OffU                 :2;
        // Fine offset adjustment for R-Y component
        // 00 :   0     LSB
        // 01 :   +1/4  LSB
        // 10 :   +1/2  LSB
        // 11 :   +3/4  LSB
        U32 OffV                 :2;
        // Chrominance bandwidth
        // 0: small
        // 1: wide
        U32 CBandw               :1;
        // Combined Luminance/Chrominance bandwidth adjustment
        U32 YCBandw              :3;
    # else /* !BIG_ENDIAN */
        // Combined Luminance/Chrominance bandwidth adjustment
        U32 YCBandw              :3;
        // Chrominance bandwidth
        // 0: small
        // 1: wide
        U32 CBandw               :1;
        // Fine offset adjustment for R-Y component
        // 00 :   0     LSB
        // 01 :   +1/4  LSB
        // 10 :   +1/2  LSB
        // 11 :   +3/4  LSB
        U32 OffV                 :2;
        // Fine offset adjustment B-Y component
        // 00 :   0     LSB
        // 01 :   +1/4  LSB
        // 10 :   +1/2  LSB
        // 11 :   +3/4  LSB
        U32 OffU                 :2;
        // Luminance Delay compensation
        U32 YDel                 :3;
        U32                      :1;
        // Fine position of HS, step size of 2/LLC
        // 00 : 0
        // 01 : 1
        // 10 : 2
        // 11 : 3 (should be obvious)
        U32 HSDel                :2;
        U32                      :1;
        // Colour on
        // 0: Automatic colour killer enabled (recommended setting)
        // 1: Colour forced on
        U32 ColOn                :1;
        U32                      :16;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } ChromaControl_2; 

#define MISCCONTROL_OFFS 0x14

  union _MISCCONTROL_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32 LLClkEn              :2;
        // Standard detection search loop latency
        // 000 = reserved, else field number
        U32 Latency              :3;
        // Alternative VGATE position
        // 0 : VGATE position accordint to tables 23 and 24
        // 1 : VGATE occurs one line earlier during field 1
        U32 VgPos                :1;
        // MSB VGATE stop
        U32 VgStop8              :1;
        // MSB VGATE start
        U32 VgStart8             :1;
        // Stop of VGATE pulse (10 transition)
        U32 VgStop0              :8;
        // Start of VGATE pulse (01 transition) and polarity change of FID-pulse
        U32 VgStart0             :8;
        // Compatibility bit for SAA7199
        // 0: off (default)
        // 1: on (to be set only if SAA7199 is used for
        //    re-encoding in conjunction with RTCO active)
        U32 Cm99                 :1;
        // Update time interval for AGC-value
        // 0 : Horizontal update (once per line)
        // 1 : Vertical update (once per line)
        U32 UpdTCV               :1;
        U32                      :3;
        // Documented under ChromaControl_1::AUTO0
        U32 Auto1                :1;
        // ACD sample clock phase delay
        U32 APhClk               :2;
    # else /* !BIG_ENDIAN */
        // ACD sample clock phase delay
        U32 APhClk               :2;
        // Documented under ChromaControl_1::AUTO0
        U32 Auto1                :1;
        U32                      :3;
        // Update time interval for AGC-value
        // 0 : Horizontal update (once per line)
        // 1 : Vertical update (once per line)
        U32 UpdTCV               :1;
        // Compatibility bit for SAA7199
        // 0: off (default)
        // 1: on (to be set only if SAA7199 is used for
        //    re-encoding in conjunction with RTCO active)
        U32 Cm99                 :1;
        // Start of VGATE pulse (01 transition) and polarity change of FID-pulse
        U32 VgStart0             :8;
        // Stop of VGATE pulse (10 transition)
        U32 VgStop0              :8;
        // MSB VGATE start
        U32 VgStart8             :1;
        // MSB VGATE stop
        U32 VgStop8              :1;
        // Alternative VGATE position
        // 0 : VGATE position accordint to tables 23 and 24
        // 1 : VGATE occurs one line earlier during field 1
        U32 VgPos                :1;
        // Standard detection search loop latency
        // 000 = reserved, else field number
        U32 Latency              :3;
        U32 LLClkEn              :2;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } MiscControl; 

#define RAWDATAGAIN_OFFS 0x18

  union _RAWDATAGAIN_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :16;
        // Raw data offset control
        U32 RawOffs              :8;
        // Raw data gain control
        U32 RawGain              :8;
    # else /* !BIG_ENDIAN */
        // Raw data gain control
        U32 RawGain              :8;
        // Raw data offset control
        U32 RawOffs              :8;
        U32                      :16;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } RawDataGain; 

#define STATUSBYTEDECODER_OFFS 0x1C

  union _STATUSBYTEDECODER_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // status bit for interlace detection
        // LOW  = noninterlaced
        // HIGH = interlaced
        U32 Intl                 :1;
        // status bit for horizontal and vertical loop : 
        // LOW  = both loops locked
        // HIGH = unlocked
        U32 HVLoopN              :1;
        // identification bit for detected field frequency, LOW 50 Hz, HIGH 60 Hz
        U32 FidT                 :1;
        U32                      :1;
        // Macrovision encoded Colorstribe burst type 3 (4 line version) detected
        U32 MVProtType3          :1;
        // Macrovision encoded Colorstribe any type detected
        U32 MVColStripe          :1;
        // Copy protected source detected according to Macrovision version up to 7.01
        U32 MVCopyProt           :1;
        // Ready for Capture(all internal loops locked), active HIGH
        U32 RdCap                :1;
        U32                      :1;
        // status bit for locked horizontal frequency, LOW = locked, HIGH = unlocked
        U32 HLock                :1;
        // slow time constant active in WIPA mode, active high
        U32 SlowTCAct            :1;
        // gain value for active luminance channel is limited, active high
        U32 GainLimTop           :1;
        // gain value for active luminance channel is limited, active high
        U32 GainLimBot           :1;
        // white peak loop is activated, active HIGH
        U32 WPAct                :1;
        // detected color standard
        // 00 : No color (BW)
        // 01 : NTSC
        // 10 : PAL
        // 11 : SECAM
        U32 DetColStd            :2;
        U32                      :16;
    # else /* !BIG_ENDIAN */
        U32                      :16;
        // detected color standard
        // 00 : No color (BW)
        // 01 : NTSC
        // 10 : PAL
        // 11 : SECAM
        U32 DetColStd            :2;
        // white peak loop is activated, active HIGH
        U32 WPAct                :1;
        // gain value for active luminance channel is limited, active high
        U32 GainLimBot           :1;
        // gain value for active luminance channel is limited, active high
        U32 GainLimTop           :1;
        // slow time constant active in WIPA mode, active high
        U32 SlowTCAct            :1;
        // status bit for locked horizontal frequency, LOW = locked, HIGH = unlocked
        U32 HLock                :1;
        U32                      :1;
        // Ready for Capture(all internal loops locked), active HIGH
        U32 RdCap                :1;
        // Copy protected source detected according to Macrovision version up to 7.01
        U32 MVCopyProt           :1;
        // Macrovision encoded Colorstribe any type detected
        U32 MVColStripe          :1;
        // Macrovision encoded Colorstribe burst type 3 (4 line version) detected
        U32 MVProtType3          :1;
        U32                      :1;
        // identification bit for detected field frequency, LOW 50 Hz, HIGH 60 Hz
        U32 FidT                 :1;
        // status bit for horizontal and vertical loop : 
        // LOW  = both loops locked
        // HIGH = unlocked
        U32 HVLoopN              :1;
        // status bit for interlace detection
        // LOW  = noninterlaced
        // HIGH = interlaced
        U32 Intl                 :1;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } StatusByteDecoder; 

} MunitComDec;

//--------- End of MunitComDec ----------------------
//*************************************************************



  U8 FillerAt0x120[0x20];

#define MUNITAUDIO_OFFS 0x140

//*************************************************************
//------------- MunitAudio ----------------------
struct _MUNITAUDIO_ {  
#define NICAMSTATUS_OFFS 0x0

  union _NICAMSTATUS_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :6;
        // Auto-Mute status: If this bit is HIGH, it indicates that the auto-muting function has
        //                   switched from NICAM to the program of the first sound carrier (i.e
        //                   FM mono or AM in NICAM L systems)
        U32 NICAM_AutoMute       :1;
        // Identification of NICAM sound: NICAM_SoundFlag = 1, indicates that digital transmission is a sound source
        //                                NICAM_SoundFlag = 0, indicates the transmission
        U32 NICAM_SoundFlag      :1;
        // NICAM application control bits: These bits correspond to the control bits C1 to C4 in 
        //                                 the NICAM transmission
        U32 NICAM_ControlBits    :4;
        // Synchronization bit: NICAM_SyncBit = 1, indicates that the device has both frame and 
        //                      C0 (16 frame) synchronization. 
        //                      NICAM_SyncBit = 0, indicates the audio output from the NICAM part 
        //                      is digital silence
        U32 NICAM_SyncBit        :1;
        // Configuration change: NICAM_ConfigChange = 1 indicates a configuration change at the
        //                       16 frame (C0) boundary.
        U32 NICAM_ConfigChange   :1;
        // Identification of NICAM stereo. NICAM_SorMB = 1 indicates stereo mode
        U32 NICAM_SorMB          :1;
        // Idendification of NICAM dual mono. NICAM_DorSB = 1, indicates dual mono mode
        U32 NICAM_DorSB          :1;
        // If this bit is HIGH, new additional data bits are written to the IC without
        // the previous bits being read
        U32 AddData_OverWrite    :1;
        // When NewAddData = 1, new additional data is written into the IC. This bit is reset,
        // when additional Data bits are read
        U32 AddData_New          :1;
        // These two bits are CI bits decoded by majority logic
        // from the parity checks of the last ten samples in a frame
        U32 CI                   :2;
        U32                      :1;
        // Additional data word
        U32 AddData_Word        :11;
    # else /* !BIG_ENDIAN */
        // Additional data word
        U32 AddData_Word        :11;
        U32                      :1;
        // These two bits are CI bits decoded by majority logic
        // from the parity checks of the last ten samples in a frame
        U32 CI                   :2;
        // When NewAddData = 1, new additional data is written into the IC. This bit is reset,
        // when additional Data bits are read
        U32 AddData_New          :1;
        // If this bit is HIGH, new additional data bits are written to the IC without
        // the previous bits being read
        U32 AddData_OverWrite    :1;
        // Idendification of NICAM dual mono. NICAM_DorSB = 1, indicates dual mono mode
        U32 NICAM_DorSB          :1;
        // Identification of NICAM stereo. NICAM_SorMB = 1 indicates stereo mode
        U32 NICAM_SorMB          :1;
        // Configuration change: NICAM_ConfigChange = 1 indicates a configuration change at the
        //                       16 frame (C0) boundary.
        U32 NICAM_ConfigChange   :1;
        // Synchronization bit: NICAM_SyncBit = 1, indicates that the device has both frame and 
        //                      C0 (16 frame) synchronization. 
        //                      NICAM_SyncBit = 0, indicates the audio output from the NICAM part 
        //                      is digital silence
        U32 NICAM_SyncBit        :1;
        // NICAM application control bits: These bits correspond to the control bits C1 to C4 in 
        //                                 the NICAM transmission
        U32 NICAM_ControlBits    :4;
        // Identification of NICAM sound: NICAM_SoundFlag = 1, indicates that digital transmission is a sound source
        //                                NICAM_SoundFlag = 0, indicates the transmission
        U32 NICAM_SoundFlag      :1;
        // Auto-Mute status: If this bit is HIGH, it indicates that the auto-muting function has
        //                   switched from NICAM to the program of the first sound carrier (i.e
        //                   FM mono or AM in NICAM L systems)
        U32 NICAM_AutoMute       :1;
        U32                      :6;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } NICAMStatus; 

#define IDENTSTATUS_OFFS 0x4

  union _IDENTSTATUS_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // Level Read Out: These two bytes constitute a word that provides data from a location that has been specified
        //                 with the Monitor Select Register.
        U32 Monitor_SelectData  :16;
        // Identificaton of FM stereo, A2 systems, if FM_Stereo = 1, an FM stereo signal has been identified
        U32 FM_Stereo            :1;
        // Identification of FM dual sound,A2 systems. If FM_DualSound = 1, an FM dual-language signal has been
        //            identified. When neither FM_Stereo or FM_DualSound = 1, the received signal is assumed
        //            to be FM mono (A2 systems only)
        U32 FM_DualSound         :1;
        // FM_PilotCarrier = 1 indicates that an FM pilot carrier in the 2nd channel is detected
        // The pilot detector is faster then the stereo/dual idendification, but not as reliable and slightly
        // less sensitive.
        U32 FM_PilotCarrier      :1;
        // SIF level Data bits: These bits correspond to the input level at the selected SIF input
        U32 SIF_DataBits         :5;
        // Nicam Error count: Bits NICAM_ErrorCount contaiin the number of errors occurring in the previous
        //                    128 ms period. The register is updated every 128 ms
        U32 NICAM_ErrorCount     :8;
    # else /* !BIG_ENDIAN */
        // Nicam Error count: Bits NICAM_ErrorCount contaiin the number of errors occurring in the previous
        //                    128 ms period. The register is updated every 128 ms
        U32 NICAM_ErrorCount     :8;
        // SIF level Data bits: These bits correspond to the input level at the selected SIF input
        U32 SIF_DataBits         :5;
        // FM_PilotCarrier = 1 indicates that an FM pilot carrier in the 2nd channel is detected
        // The pilot detector is faster then the stereo/dual idendification, but not as reliable and slightly
        // less sensitive.
        U32 FM_PilotCarrier      :1;
        // Identification of FM dual sound,A2 systems. If FM_DualSound = 1, an FM dual-language signal has been
        //            identified. When neither FM_Stereo or FM_DualSound = 1, the received signal is assumed
        //            to be FM mono (A2 systems only)
        U32 FM_DualSound         :1;
        // Identificaton of FM stereo, A2 systems, if FM_Stereo = 1, an FM stereo signal has been identified
        U32 FM_Stereo            :1;
        // Level Read Out: These two bytes constitute a word that provides data from a location that has been specified
        //                 with the Monitor Select Register.
        U32 Monitor_SelectData  :16;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } IdentStatus; 

#define CONTROLREGISTER_OFFS 0x8

  union _CONTROLREGISTER_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :3;
        // Channel 2 receive mode 1: These bits control the hardware for the second sound carrier. NICAM mode employs
        //                           a wider bandwidth of the decimation filters than FM mode
        // 0    0  FM
        // 0    1  AM
        // 1    0  NICAM
        U32 Channel2_Mode1       :1;
        // selects filter bandwidth
        //  0   0   narrow/narrow         recommended for nominal terrestrial broadcast conditions and SAT with 2 carriers
        //  0   1   extra wide/ narrow    recommended for highly overmodulated single FM carriers. Only channel 1 is 
        //                                available for FM demodulation in this mode. 
        //                                On channel 2 NICAM can still be processed
        //  1   0   medium/medium         recommended for moderately overmodulated broadcast conditions
        //  1   1   wide/wide             recommended for strongly overmudulated broadcast conditions
        U32 Filter_BandwithMode  :2;
        // Channel 1 receive mode: CHANNEL1_Mode = 1 selects the hardware for the first sound carrier to operate in
        //                         AM mode. CHANNEL1_Mode = 0, FM mode is assumed. This applies to both terrestrial 
        //                         and satellite FM reception
        U32 Channel1_Mode        :1;
        // Ident Control register field: FM_DCXOPull & FM_DCXOTest & FM_ItegrateTime & FM_Area  
        // Channel 2 receive mode 0: These bits control the hardware for the second sound carrier. NICAM mode employs
        //                           a wider bandwidth of the decimation filters than FM mode
        // 0    0  FM
        // 0    1  AM
        // 1    0  NICAM
        U32 Channel2_Mode0       :1;
        U32                      :2;
        // DCXO frequency select: Select DCXO lower/upper test frequency during DCXO test mode. FM_DCXOPull = 1
        //                        sets the DCXO to the lower DCXO frequency. FM_DCXOPull = 0 sets the DCXO to its 
        //                        higher frequency.
        U32 FM_DCXOPull          :1;
        // DCXO test mode enable: FM_DCXOTest = 1 enables the DCXO test mode (available only during FM mode)
        //                        in this mode frequency pulling via FM_DCXOPull is enabled. FM_DCXOTest = 0
        //                        enables normal operation
        U32 FM_DCXOTest          :1;
        U32                      :1;
        // Identification mode for FM sound: These bits define the integrator time of the FM identification. A valid
        //                    result may be expected after twice this time has expired, at the latest. The longer
        //                    the time, the more reliable the identification
        U32 FM_ItegrateTime      :2;
        // Application area for FM identification: FM_Area = 1 selects FM identification frequencies in accordance with
        //             the specification for Korea. FM_Area - 0, frequencies for Europe are selected 
        U32 FM_Area              :1;
        // NICAM Upper Error Limit: When the auto-mute function is enabled and the NICAM bit error count is higher 
        //                          than the value contained in this register, the signal of the first sound carrier 
        //                          or the analog mono input is selected for reproduction.
        U32 NICAM_UppErrLimit    :8;
        // NICAM Lower Error Limit: When the auto-mute function is enabled and the NICAM bit error count is lower
        //                          than the value contained in this register, the NICAM signal is selected (again)
        //                          for reproduction
        U32 NICAM_LowErrLimit    :8;
    # else /* !BIG_ENDIAN */
        // NICAM Lower Error Limit: When the auto-mute function is enabled and the NICAM bit error count is lower
        //                          than the value contained in this register, the NICAM signal is selected (again)
        //                          for reproduction
        U32 NICAM_LowErrLimit    :8;
        // NICAM Upper Error Limit: When the auto-mute function is enabled and the NICAM bit error count is higher 
        //                          than the value contained in this register, the signal of the first sound carrier 
        //                          or the analog mono input is selected for reproduction.
        U32 NICAM_UppErrLimit    :8;
        // Application area for FM identification: FM_Area = 1 selects FM identification frequencies in accordance with
        //             the specification for Korea. FM_Area - 0, frequencies for Europe are selected 
        U32 FM_Area              :1;
        // Identification mode for FM sound: These bits define the integrator time of the FM identification. A valid
        //                    result may be expected after twice this time has expired, at the latest. The longer
        //                    the time, the more reliable the identification
        U32 FM_ItegrateTime      :2;
        U32                      :1;
        // DCXO test mode enable: FM_DCXOTest = 1 enables the DCXO test mode (available only during FM mode)
        //                        in this mode frequency pulling via FM_DCXOPull is enabled. FM_DCXOTest = 0
        //                        enables normal operation
        U32 FM_DCXOTest          :1;
        // DCXO frequency select: Select DCXO lower/upper test frequency during DCXO test mode. FM_DCXOPull = 1
        //                        sets the DCXO to the lower DCXO frequency. FM_DCXOPull = 0 sets the DCXO to its 
        //                        higher frequency.
        U32 FM_DCXOPull          :1;
        U32                      :2;
        // Ident Control register field: FM_DCXOPull & FM_DCXOTest & FM_ItegrateTime & FM_Area  
        // Channel 2 receive mode 0: These bits control the hardware for the second sound carrier. NICAM mode employs
        //                           a wider bandwidth of the decimation filters than FM mode
        // 0    0  FM
        // 0    1  AM
        // 1    0  NICAM
        U32 Channel2_Mode0       :1;
        // Channel 1 receive mode: CHANNEL1_Mode = 1 selects the hardware for the first sound carrier to operate in
        //                         AM mode. CHANNEL1_Mode = 0, FM mode is assumed. This applies to both terrestrial 
        //                         and satellite FM reception
        U32 Channel1_Mode        :1;
        // selects filter bandwidth
        //  0   0   narrow/narrow         recommended for nominal terrestrial broadcast conditions and SAT with 2 carriers
        //  0   1   extra wide/ narrow    recommended for highly overmodulated single FM carriers. Only channel 1 is 
        //                                available for FM demodulation in this mode. 
        //                                On channel 2 NICAM can still be processed
        //  1   0   medium/medium         recommended for moderately overmodulated broadcast conditions
        //  1   1   wide/wide             recommended for strongly overmudulated broadcast conditions
        U32 Filter_BandwithMode  :2;
        // Channel 2 receive mode 1: These bits control the hardware for the second sound carrier. NICAM mode employs
        //                           a wider bandwidth of the decimation filters than FM mode
        // 0    0  FM
        // 0    1  AM
        // 1    0  NICAM
        U32 Channel2_Mode1       :1;
        U32                      :3;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } ControlRegister; 

#define AGCGAINSELECT_OFFS 0xC

  union _AGCGAINSELECT_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :25;
        // AGC on or off. AGC_Mode = 1 forces the AGC block to a fixed gain as defined in the AGC Gain
        //                AGC_Mode = 0 the automatic gain control function is enabled and the contents of the
        //                             AGC gain register is ignored.
        U32 AGC_Mode             :1;
        // AGC decay time: AGC_DecayTime = 1, a longer decay time and larger hysteresis are selected for
        //                 input signals with strong viedo modulation (conventional intercarrier). This bit 
        //                 has only an effect, when bit AGC_Mode = 0. AGC_DecayTime = 0, selects normal attack and 
        //                 decay times for AGC and a small hysteresis
        U32 AGC_DecayTime        :1;
        // If the automatic gain control function is switched off in the general configuration register, the 
        // contents of this register will define a fixed gain of the AGC stage. (See Table 13 in munit_audio)
        U32 AGC_FixedGain        :5;
    # else /* !BIG_ENDIAN */
        // If the automatic gain control function is switched off in the general configuration register, the 
        // contents of this register will define a fixed gain of the AGC stage. (See Table 13 in munit_audio)
        U32 AGC_FixedGain        :5;
        // AGC decay time: AGC_DecayTime = 1, a longer decay time and larger hysteresis are selected for
        //                 input signals with strong viedo modulation (conventional intercarrier). This bit 
        //                 has only an effect, when bit AGC_Mode = 0. AGC_DecayTime = 0, selects normal attack and 
        //                 decay times for AGC and a small hysteresis
        U32 AGC_DecayTime        :1;
        // AGC on or off. AGC_Mode = 1 forces the AGC block to a fixed gain as defined in the AGC Gain
        //                AGC_Mode = 0 the automatic gain control function is enabled and the contents of the
        //                             AGC gain register is ignored.
        U32 AGC_Mode             :1;
        U32                      :25;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } AGCGainSelect; 

#define CARRIERONEFREQU_OFFS 0x10

  union _CARRIERONEFREQU_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :8;
        // These three bytes define a frequency control word ro represent the sound carrier (i.e. mixer)
        // Execution of the command starts only after all bytes have been received. If an error occurs, e.g.
        // a premature STOP condition, parital data for this function are ignored.
        U32 CarrierOne_Data     :24;
    # else /* !BIG_ENDIAN */
        // These three bytes define a frequency control word ro represent the sound carrier (i.e. mixer)
        // Execution of the command starts only after all bytes have been received. If an error occurs, e.g.
        // a premature STOP condition, parital data for this function are ignored.
        U32 CarrierOne_Data     :24;
        U32                      :8;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } CarrierOneFrequ; 

#define CARRIERTWOFREQU_OFFS 0x14

  union _CARRIERTWOFREQU_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :8;
        // These three bytes define a frequency control word ro represent the sound carrier (i.e. mixer)
        // Execution of the command starts only after all bytes have been received. If an error occurs, e.g.
        // a premature STOP condition, parital data for this function are ignored.
        U32 CarrierTwo_Data     :24;
    # else /* !BIG_ENDIAN */
        // These three bytes define a frequency control word ro represent the sound carrier (i.e. mixer)
        // Execution of the command starts only after all bytes have been received. If an error occurs, e.g.
        // a premature STOP condition, parital data for this function are ignored.
        U32 CarrierTwo_Data     :24;
        U32                      :8;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } CarrierTwoFrequ; 

#define AUDIOFORMAT_OFFS 0x18

  union _AUDIOFORMAT_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // Selects imput source for DMA
        //  0    0  Raw FM data (Mixer/Decimator) is input source
        //  0    1  BTSC data (Demodulator) is input source
        //  1    0  Baseband audio (Audio ADC's) is input source
        //  1    1  Dual-FM (Audio-DSP) is input source
        U32 Audio_InputSource    :2;
        // Samples relevance and transfer frequency (DMA Block)
        //  0    0    0   both samples are stored (single) in a 32 bit DMA
        //  1    0    1   any first (left) of two samples (stereo) is stored twice in the 32 bit DMA
        //  1    1    0   any second (right) of two samples (stereo) is stored twice
        U32 Audio_SampleRel      :3;
        // Output format to DMA block in two's complement
        U32 Audio_OutputFormat   :3;
        U32                      :2;
        // Number of audio samples: The logical value of the lines corresponds to the PC's memory size of the 
        // audio data field organized in 32 bit words.
        U32 Audio_SampleCount   :22;
    # else /* !BIG_ENDIAN */
        // Number of audio samples: The logical value of the lines corresponds to the PC's memory size of the 
        // audio data field organized in 32 bit words.
        U32 Audio_SampleCount   :22;
        U32                      :2;
        // Output format to DMA block in two's complement
        U32 Audio_OutputFormat   :3;
        // Samples relevance and transfer frequency (DMA Block)
        //  0    0    0   both samples are stored (single) in a 32 bit DMA
        //  1    0    1   any first (left) of two samples (stereo) is stored twice in the 32 bit DMA
        //  1    1    0   any second (right) of two samples (stereo) is stored twice
        U32 Audio_SampleRel      :3;
        // Selects imput source for DMA
        //  0    0  Raw FM data (Mixer/Decimator) is input source
        //  0    1  BTSC data (Demodulator) is input source
        //  1    0  Baseband audio (Audio ADC's) is input source
        //  1    1  Dual-FM (Audio-DSP) is input source
        U32 Audio_InputSource    :2;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } AudioFormat; 

  U8 FillerAt0x1C[0x4];

#define MONITORSELECT_OFFS 0x20

  union _MONITORSELECT_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :3;
        // De-Matrix characteristics: FM_AutoDematrixMode & FM_ManualDematrixMode
        // Channel 1 output Level adjust: This register is used to correct for standard and station dependent
        // differences of signal levels. The gain settings cn be chosen from 0 dB to +15 dB 
        // (FM_OutputLevelOne = 00000b to 01111b) and from -15 dB to -1 dB (FM_OutputLevelOne = 10000b to 11110b).
        // FM_OutputLevelOne = 11111b is not defined
        U32 FM_OutputLevelOne    :5;
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
        U32 FM_AutoDematrixMode  :1;
        U32                      :4;
        // De-Emphasis characteristics: FM_AdaptiveModeTwo & FM_DeEmphasisTwo & FM_AdaptiveModeOne & FM_DeEmphasisOne
        // Dematrix characteristics select: The state of these 3 bits select the dematrixing characteristics
        //                        L Output        R Output        Mode
        // 0   0   0                CH1              CH1           mono 1
        // 0   0   1                CH2              CH2           mono 2
        // 0   1   0                CH1              CH2           dual
        // 0   1   1                CH2              CH1           dual swapped
        // 1   0   0              2*CH1 - CH2        CH2           Stereo europe
        // 1   0   1          (CH1 + CH2) / 2    (CH1 - CH2) / 2   stereo Korea - 6 dB
        // 1   1   0              CH1 + CH2        CH1 - CH2       stereo Korea
        U32 FM_ManualDematrixMode :3;
        // Adaptive de-emphasis on/off: FM_AdaptiveModeTwo = 1 activates the adaptive de-emphasis function
        // which is required for certain satellite FM channels. The standard FM de-emphasis must then be set
        // to 75 mikros, FM_AdaptiveModeTwo = 0, the adaptive de-emphasis is off
        U32 FM_AdaptiveModeTwo   :1;
        // FM de-emphasis: The state of these 3 bits determine the FM de-emphasis for sound carrier 2
        // 0   0   0    50 mikros
        // 0   0   1    60 mikros
        // 0   1   0    75 mikros
        // 0   1   1    J17
        // 1   0   0    off
        U32 FM_DeEmphasisTwo     :3;
        // Adaptive de-emphasis on/off: FM_AdaptiveModeOne = 1 activates the adaptive de-emphasis function
        // which is required for certain satellite FM channels. The standard FM de-emphasis must then be set
        // to 75 mikros, FM_AdaptiveModeOne = 0, the adaptive de-emphasis is off
        U32 FM_AdaptiveModeOne   :1;
        // FM de-emphasis: The State of these 3 bits determine the FM de-emphasis for sound carrier 1
        // 0   0   0    50 mikros
        // 0   0   1    60 mikros
        // 0   1   0    75 mikros
        // 0   1   1    J17
        // 1   0   0    off
        U32 FM_DeEmphasisOne     :3;
        // Peak Level select: Monitor_PeakMode = 1 selects the rectified peak level of a source to be monitored.
        //                    Peak level value is reset to 0 after read out.
        U32 Monitor_PeakLevel    :1;
        U32                      :1;
        // Signal Channel select:
        // 0    0  (Channel 1 + Channel 2) / 2
        // 0    1  Channel 1
        // 1    0  Channel 2
        U32 Monitor_SignalChannel :2;
        U32                      :2;
        // Signal Source select
        //  0   0  DC output of FM/AM demodulator
        //  0   1  magnitude/phase of FM/AM demodulator
        //  1   0  FM/AM path output
        //  1   1  NICAM path output
        U32 Monitor_SignalSource :2;
    # else /* !BIG_ENDIAN */
        // Signal Source select
        //  0   0  DC output of FM/AM demodulator
        //  0   1  magnitude/phase of FM/AM demodulator
        //  1   0  FM/AM path output
        //  1   1  NICAM path output
        U32 Monitor_SignalSource :2;
        U32                      :2;
        // Signal Channel select:
        // 0    0  (Channel 1 + Channel 2) / 2
        // 0    1  Channel 1
        // 1    0  Channel 2
        U32 Monitor_SignalChannel :2;
        U32                      :1;
        // Peak Level select: Monitor_PeakMode = 1 selects the rectified peak level of a source to be monitored.
        //                    Peak level value is reset to 0 after read out.
        U32 Monitor_PeakLevel    :1;
        // FM de-emphasis: The State of these 3 bits determine the FM de-emphasis for sound carrier 1
        // 0   0   0    50 mikros
        // 0   0   1    60 mikros
        // 0   1   0    75 mikros
        // 0   1   1    J17
        // 1   0   0    off
        U32 FM_DeEmphasisOne     :3;
        // Adaptive de-emphasis on/off: FM_AdaptiveModeOne = 1 activates the adaptive de-emphasis function
        // which is required for certain satellite FM channels. The standard FM de-emphasis must then be set
        // to 75 mikros, FM_AdaptiveModeOne = 0, the adaptive de-emphasis is off
        U32 FM_AdaptiveModeOne   :1;
        // FM de-emphasis: The state of these 3 bits determine the FM de-emphasis for sound carrier 2
        // 0   0   0    50 mikros
        // 0   0   1    60 mikros
        // 0   1   0    75 mikros
        // 0   1   1    J17
        // 1   0   0    off
        U32 FM_DeEmphasisTwo     :3;
        // Adaptive de-emphasis on/off: FM_AdaptiveModeTwo = 1 activates the adaptive de-emphasis function
        // which is required for certain satellite FM channels. The standard FM de-emphasis must then be set
        // to 75 mikros, FM_AdaptiveModeTwo = 0, the adaptive de-emphasis is off
        U32 FM_AdaptiveModeTwo   :1;
        // De-Emphasis characteristics: FM_AdaptiveModeTwo & FM_DeEmphasisTwo & FM_AdaptiveModeOne & FM_DeEmphasisOne
        // Dematrix characteristics select: The state of these 3 bits select the dematrixing characteristics
        //                        L Output        R Output        Mode
        // 0   0   0                CH1              CH1           mono 1
        // 0   0   1                CH2              CH2           mono 2
        // 0   1   0                CH1              CH2           dual
        // 0   1   1                CH2              CH1           dual swapped
        // 1   0   0              2*CH1 - CH2        CH2           Stereo europe
        // 1   0   1          (CH1 + CH2) / 2    (CH1 - CH2) / 2   stereo Korea - 6 dB
        // 1   1   0              CH1 + CH2        CH1 - CH2       stereo Korea
        U32 FM_ManualDematrixMode :3;
        U32                      :4;
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
        U32 FM_AutoDematrixMode  :1;
        // De-Matrix characteristics: FM_AutoDematrixMode & FM_ManualDematrixMode
        // Channel 1 output Level adjust: This register is used to correct for standard and station dependent
        // differences of signal levels. The gain settings cn be chosen from 0 dB to +15 dB 
        // (FM_OutputLevelOne = 00000b to 01111b) and from -15 dB to -1 dB (FM_OutputLevelOne = 10000b to 11110b).
        // FM_OutputLevelOne = 11111b is not defined
        U32 FM_OutputLevelOne    :5;
        U32                      :3;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } MonitorSelect; 

#define NICAMCONFIG_OFFS 0x24

  union _NICAMCONFIG_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // Selection of stereo gain
        // 0   0    0 dB
        // 0   1    3 dB
        // 1   0    6 dB
        // 1   1    9 dB
        U32 Signal_StereoGain    :2;
        // Automatic Volume Level control function 
        //   0    0   off/reset
        //   0    1   short decay(2 s)
        //   1    0   medium decay(4 s)
        //   1    1   long decay(8 s)
        U32 Signal_Volumecontrol :2;
        U32                      :2;
        // Signal Source left and right:
        //                    LEFT          RIGHT
        //  0    0             FM/AM         FM/AM
        //  0    1             NICAM left     NICAM right
        //  1    0             FM/AM          NICAM M1
        //  1    1             FM/AM          NICAM M2
        U32 Signal_SourceSelect  :2;
        U32                      :3;
        // NICAM configuration: NICAM_AutoMuteSelect & NICAM_DeEmphasisMode & NICAM_AutoMuteMode
        //  This register is used to correct for standard and station dependent
        // differences of signal levels. The gain settings cn be chosen from 0 dB to +15 dB 
        // (NICAM_OutputLevel = 00000b to 01111b) and from -15 dB to -1 dB (NICAM_OutputLevel = 10000b to 11110b).
        // NICAM_OutputLevel = 11111b is not defined
        U32 NICAM_OutputLevel    :5;
        U32                      :5;
        // Auto-mute select. NICAM_AutoMuteSelect = 1, the auto mute will switch between NICAM sound and the analog
        // mono input. This bit has only an effect when the auto-mute function is enabled and when the DAC has been
        // selected in the Analog Output Select Register, NICAM_AutoMuteSelect = 0, the auto-mute will switch between 
        // NICAM sound on the first sound carrier (i.e. FM mono of AM)
        U32 NICAM_AutoMuteSelect :1;
        // De-emphasis on/off. NICAM_DeEmphasisMode = 1 switches the J17 de-emphasis off.
        // NICAM_DeEmphasisMode = 0 switches the J17 de-emphasis on.
        U32 NICAM_DeEmphasisMode :1;
        // Auto muting on/off: NICAM_AutoMuteMode = 1, automatic muting is disabled. This bit has only an effect, when the 
        // second sound carrier is set to NICAM. NICAM_AutoMute = 0, enables the automatic switching between NICAM 
        // and the program on the first sound carrier (i.e. FM mono or AM), dependent on the NICAM bit error rate. The
        // FM dematrix should be set to the mono position or FM_AutoDematrixMode should be set.
        U32 NICAM_AutoMuteMode   :1;
        U32                      :3;
        // Channel 2 output Level adjust: This register is used to correct for standard and station dependent
        // differences of signal levels. The gain settings cn be chosen from 0 dB to +15 dB 
        // (FM_OutputLevelTwo = 00000b to 01111b) and from -15 dB to -1 dB (FM_OutputLevelTwo = 10000b to 11110b).
        // FM_OutputLevelTwo = 11111b is not defined
        U32 FM_OutputLevelTwo    :5;
    # else /* !BIG_ENDIAN */
        // Channel 2 output Level adjust: This register is used to correct for standard and station dependent
        // differences of signal levels. The gain settings cn be chosen from 0 dB to +15 dB 
        // (FM_OutputLevelTwo = 00000b to 01111b) and from -15 dB to -1 dB (FM_OutputLevelTwo = 10000b to 11110b).
        // FM_OutputLevelTwo = 11111b is not defined
        U32 FM_OutputLevelTwo    :5;
        U32                      :3;
        // Auto muting on/off: NICAM_AutoMuteMode = 1, automatic muting is disabled. This bit has only an effect, when the 
        // second sound carrier is set to NICAM. NICAM_AutoMute = 0, enables the automatic switching between NICAM 
        // and the program on the first sound carrier (i.e. FM mono or AM), dependent on the NICAM bit error rate. The
        // FM dematrix should be set to the mono position or FM_AutoDematrixMode should be set.
        U32 NICAM_AutoMuteMode   :1;
        // De-emphasis on/off. NICAM_DeEmphasisMode = 1 switches the J17 de-emphasis off.
        // NICAM_DeEmphasisMode = 0 switches the J17 de-emphasis on.
        U32 NICAM_DeEmphasisMode :1;
        // Auto-mute select. NICAM_AutoMuteSelect = 1, the auto mute will switch between NICAM sound and the analog
        // mono input. This bit has only an effect when the auto-mute function is enabled and when the DAC has been
        // selected in the Analog Output Select Register, NICAM_AutoMuteSelect = 0, the auto-mute will switch between 
        // NICAM sound on the first sound carrier (i.e. FM mono of AM)
        U32 NICAM_AutoMuteSelect :1;
        U32                      :5;
        // NICAM configuration: NICAM_AutoMuteSelect & NICAM_DeEmphasisMode & NICAM_AutoMuteMode
        //  This register is used to correct for standard and station dependent
        // differences of signal levels. The gain settings cn be chosen from 0 dB to +15 dB 
        // (NICAM_OutputLevel = 00000b to 01111b) and from -15 dB to -1 dB (NICAM_OutputLevel = 10000b to 11110b).
        // NICAM_OutputLevel = 11111b is not defined
        U32 NICAM_OutputLevel    :5;
        U32                      :3;
        // Signal Source left and right:
        //                    LEFT          RIGHT
        //  0    0             FM/AM         FM/AM
        //  0    1             NICAM left     NICAM right
        //  1    0             FM/AM          NICAM M1
        //  1    1             FM/AM          NICAM M2
        U32 Signal_SourceSelect  :2;
        U32                      :2;
        // Automatic Volume Level control function 
        //   0    0   off/reset
        //   0    1   short decay(2 s)
        //   1    0   medium decay(4 s)
        //   1    1   long decay(8 s)
        U32 Signal_Volumecontrol :2;
        // Selection of stereo gain
        // 0   0    0 dB
        // 0   1    3 dB
        // 1   0    6 dB
        // 1   1    9 dB
        U32 Signal_StereoGain    :2;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } NICAMConfig; 

#define DIGITALINTERFACE_OFFS 0x28

  union _DIGITALINTERFACE_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // Auto select function for TV applications if set HIGH.
        U32 Analog_TVAutoSelect  :1;
        // Analog output channel selection mode
        //                         L Output     R Output
        //  0   0   0                 L input     R input
        //  0   0   1                 L input     L input
        //  0   1   0                 R input     R input
        //  0   1   1                 R input     L input
        //  1   0   0                (L + R)/2   (L + R)/2
        U32 Analog_OutputChannelMode :3;
        U32                      :7;
        // I2S output select: I2S_TVAutoSelect & I2S_OutputChannelMode & I2S_SignalSource
        //  This register is used to correct for standard and station dependent
        // differences of signal levels. The gain settings cn be chosen from 0 dB to +15 dB 
        // (I2S_OutputLevel = 00000b to 01111b) and from -15 dB to -1 dB (I2S_OutputLevel = 10000b to 11110b).
        // I2S_OutputLevel = 11111b is not defined
        U32 I2S_OutputLevel      :5;
        // Auto select function for TV applications if set HIGH.
        U32 I2S_TVAutoSelect     :1;
        // Output channel selection mode
        //                         L Output     R Output
        //  0   0   0                 L input     R input
        //  0   0   1                 L input     L input
        //  0   1   0                 R input     R input
        //  0   1   1                 R input     L input
        //  1   0   0                (L + R)/2   (L + R)/2
        U32 I2S_OutputChannelMode :3;
        U32                      :2;
        // Digital configuration: I2S_OutputFormat & I2S_OutputMode 
        // Signal source I2S Output (ignoring ISS2)
        //                    LEFT          RIGHT
        //  0    0             FM/AM         FM/AM
        //  0    1             NICAM left     NICAM right
        //  1    0             FM/AM          NICAM M1
        //  1    1             FM/AM          NICAM M2
        U32 I2S_SignalSource     :2;
        U32                      :6;
        // I2S output format: I2S_OutputFormat = 1 selects an MSB-aligned, MSB-first output format, i.e. a level change
        //        at the word select ping indicates the beginning of a new audio sample, I2S_OutputFormat = 0 
        //        selects the standard I2S output format
        U32 I2S_OutputFormat     :1;
        // I2S output mode: I2S_OutputMode = 1 enables the output of serial audio data (2 pins) plus serial
        //        bit clock and word select in a format determined by the I2S_OutputFormat bit. The device
        //        then is an I2S-bus master. I2S_OutputMode = 0, the outputs mentioned will be 3-stated, thereby
        //        improving EMC performance.
        U32 I2S_OutputMode       :1;
    # else /* !BIG_ENDIAN */
        // I2S output mode: I2S_OutputMode = 1 enables the output of serial audio data (2 pins) plus serial
        //        bit clock and word select in a format determined by the I2S_OutputFormat bit. The device
        //        then is an I2S-bus master. I2S_OutputMode = 0, the outputs mentioned will be 3-stated, thereby
        //        improving EMC performance.
        U32 I2S_OutputMode       :1;
        // I2S output format: I2S_OutputFormat = 1 selects an MSB-aligned, MSB-first output format, i.e. a level change
        //        at the word select ping indicates the beginning of a new audio sample, I2S_OutputFormat = 0 
        //        selects the standard I2S output format
        U32 I2S_OutputFormat     :1;
        U32                      :6;
        // Digital configuration: I2S_OutputFormat & I2S_OutputMode 
        // Signal source I2S Output (ignoring ISS2)
        //                    LEFT          RIGHT
        //  0    0             FM/AM         FM/AM
        //  0    1             NICAM left     NICAM right
        //  1    0             FM/AM          NICAM M1
        //  1    1             FM/AM          NICAM M2
        U32 I2S_SignalSource     :2;
        U32                      :2;
        // Output channel selection mode
        //                         L Output     R Output
        //  0   0   0                 L input     R input
        //  0   0   1                 L input     L input
        //  0   1   0                 R input     R input
        //  0   1   1                 R input     L input
        //  1   0   0                (L + R)/2   (L + R)/2
        U32 I2S_OutputChannelMode :3;
        // Auto select function for TV applications if set HIGH.
        U32 I2S_TVAutoSelect     :1;
        // I2S output select: I2S_TVAutoSelect & I2S_OutputChannelMode & I2S_SignalSource
        //  This register is used to correct for standard and station dependent
        // differences of signal levels. The gain settings cn be chosen from 0 dB to +15 dB 
        // (I2S_OutputLevel = 00000b to 01111b) and from -15 dB to -1 dB (I2S_OutputLevel = 10000b to 11110b).
        // I2S_OutputLevel = 11111b is not defined
        U32 I2S_OutputLevel      :5;
        U32                      :7;
        // Analog output channel selection mode
        //                         L Output     R Output
        //  0   0   0                 L input     R input
        //  0   0   1                 L input     L input
        //  0   1   0                 R input     R input
        //  0   1   1                 R input     L input
        //  1   0   0                (L + R)/2   (L + R)/2
        U32 Analog_OutputChannelMode :3;
        // Auto select function for TV applications if set HIGH.
        U32 Analog_TVAutoSelect  :1;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } DigitalInterface; 

#define AUDIOCONTROL_OFFS 0x2C

  union _AUDIOCONTROL_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :10;
        // Voltage Input Select for analog input EXTI1 (VSEL0) and EXTI2 (VSEL1). A HIGH
        // selects 2.0Vrms, a LOW 1.0Vrms input voltage.
        U32 VoltageInputSelect   :2;
        // Input Crossbar Select. A low selects analog input EXTIL/R1, a high EXTIL/R2.
        U32 InputCrossbarSelect  :1;
        // SIF control: SIF_GainSelect & SIF_ClockPhaseShift & DC_FilterEnable & SIF_SampleFreq 
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
        U32 OutputCrossbarSelect :3;
        // SIF gain selection; 00 -> 0dB, 01 -> -6dB, 10 -> -10dB
        U32 SIF_GainSelect       :2;
        // SIF clock phase shift. Shift the ADC clock in steps of 10ns against audio frontend clock
        U32 SIF_ClockPhaseShift  :2;
        U32                      :1;
        // Enables DC cancellation filter of decimationfilter
        U32 DC_FilterEnable      :1;
        // Audio mute control separation
        // Sample frequency select for audio. See table:
        // 0 0 32 kHz; SIF
        // 0 1 32 kHz; Baseband
        // 1 0 44.1 kHz; Baseband
        // 1 1 48 kHz; Baseband
        U32 SIF_SampleFreq       :2;
        // Audio mute control
        // Only bits 6 and 2 are used, all other bits should be set to logic 1. When any of these bits is set
        // HIGH the corresponding pair of output channels will be muted. A LOW bit allows normal signal ouptut
        //  Bit 6 = 1  Mute I2S output
        //  Bit 2 = 1  Mute Stereo output
        U32 Audio_MuteControl    :8;
    # else /* !BIG_ENDIAN */
        // Audio mute control
        // Only bits 6 and 2 are used, all other bits should be set to logic 1. When any of these bits is set
        // HIGH the corresponding pair of output channels will be muted. A LOW bit allows normal signal ouptut
        //  Bit 6 = 1  Mute I2S output
        //  Bit 2 = 1  Mute Stereo output
        U32 Audio_MuteControl    :8;
        // Audio mute control separation
        // Sample frequency select for audio. See table:
        // 0 0 32 kHz; SIF
        // 0 1 32 kHz; Baseband
        // 1 0 44.1 kHz; Baseband
        // 1 1 48 kHz; Baseband
        U32 SIF_SampleFreq       :2;
        // Enables DC cancellation filter of decimationfilter
        U32 DC_FilterEnable      :1;
        U32                      :1;
        // SIF clock phase shift. Shift the ADC clock in steps of 10ns against audio frontend clock
        U32 SIF_ClockPhaseShift  :2;
        // SIF gain selection; 00 -> 0dB, 01 -> -6dB, 10 -> -10dB
        U32 SIF_GainSelect       :2;
        // SIF control: SIF_GainSelect & SIF_ClockPhaseShift & DC_FilterEnable & SIF_SampleFreq 
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
        U32 OutputCrossbarSelect :3;
        // Input Crossbar Select. A low selects analog input EXTIL/R1, a high EXTIL/R2.
        U32 InputCrossbarSelect  :1;
        // Voltage Input Select for analog input EXTI1 (VSEL0) and EXTI2 (VSEL1). A HIGH
        // selects 2.0Vrms, a LOW 1.0Vrms input voltage.
        U32 VoltageInputSelect   :2;
        U32                      :10;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } AudioControl; 

#define AUDIOCLOCK_OFFS 0x30

  union _AUDIOCLOCK_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :6;
        // opens PLL
        U32 Audio_PLL_Open       :1;
        // ensures correct datatransfer of ACNI to PLL if SWLOOP=1
        U32 Audio_PLL_SWloop     :1;
        U32                      :2;
        // The audio clock is synthesized from the same crystal frequency as the line-locked video clock is generated.
        // ACNI[21:0] = round (audiofreq / crystalfreq * 2^23) => Audio master Clocks Nominal Increment
        // Example: audiofreq = 6.144 MHz and crystalfreq = 32.11 MHz => ACNI = 187DE7h
        U32 AudioClockNominalInc:22;
    # else /* !BIG_ENDIAN */
        // The audio clock is synthesized from the same crystal frequency as the line-locked video clock is generated.
        // ACNI[21:0] = round (audiofreq / crystalfreq * 2^23) => Audio master Clocks Nominal Increment
        // Example: audiofreq = 6.144 MHz and crystalfreq = 32.11 MHz => ACNI = 187DE7h
        U32 AudioClockNominalInc:22;
        U32                      :2;
        // ensures correct datatransfer of ACNI to PLL if SWLOOP=1
        U32 Audio_PLL_SWloop     :1;
        // opens PLL
        U32 Audio_PLL_Open       :1;
        U32                      :6;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } AudioClock; 

#define AUDIO_CPF_OFFS 0x34

  union _AUDIO_CPF_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :14;
        // An audio sample clock, that is locked to the field frequency, makes sure that there is always the 
        // same predefined number of audio samples associated with a field, or a set of fields.
        // ACPF = round (audiofreq / fieldfreq) => Audio master Clocks Per Field
        U32 AudioClocksPerField :18;
    # else /* !BIG_ENDIAN */
        // An audio sample clock, that is locked to the field frequency, makes sure that there is always the 
        // same predefined number of audio samples associated with a field, or a set of fields.
        // ACPF = round (audiofreq / fieldfreq) => Audio master Clocks Per Field
        U32 AudioClocksPerField :18;
        U32                      :14;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } Audio_CPF; 

} MunitAudio;

//--------- End of MunitAudio ----------------------
//*************************************************************



  U8 FillerAt0x178[0x8];

#define MUNITGPIO_OFFS 0x180

//*************************************************************
//------------- MunitGpio ----------------------
struct _MUNITGPIO_ {  
#define IICCONTROL_OFFS 0x0

  union _IICCONTROL_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // Timeout parameter, applicable for various sequential conditions to check:
        // bus arbitration, SCL stretching and DONE status echoing 
        // Timeout = IIC clock period * 2^(Timer)							
        U32 Iic_Timer            :4;
        U32                      :5;
        // Combination of the registers
        // Iic_Status & Iic_Attr & and Iic_ByteReadWrite
        // Nominal IIc clock rate
        //0: slow (below) 100 kHz, 33.3 MHz/400
        //1: fast (below) 400 kHz, 33.3 MHz/100
        U32 Iic_ClockSel         :1;
        U32                      :6;
        // Bytes to transfer: read or write on IIC bus according LSB
        // of most recent START-BYTE
        U32 Iic_ByteReadWrite    :8;
        // Transfer attribute associated with this command
        // 3 : START 
        // 2 : CONTINUE
        // 1 : STOP
        // 0 : NOP
        U32 Iic_Attr             :2;
        U32                      :2;
        // Status information of IIC interface,
        // see Register IF block documentation for details
        U32 Iic_Status           :4;
    # else /* !BIG_ENDIAN */
        // Status information of IIC interface,
        // see Register IF block documentation for details
        U32 Iic_Status           :4;
        U32                      :2;
        // Transfer attribute associated with this command
        // 3 : START 
        // 2 : CONTINUE
        // 1 : STOP
        // 0 : NOP
        U32 Iic_Attr             :2;
        // Bytes to transfer: read or write on IIC bus according LSB
        // of most recent START-BYTE
        U32 Iic_ByteReadWrite    :8;
        U32                      :6;
        // Combination of the registers
        // Iic_Status & Iic_Attr & and Iic_ByteReadWrite
        // Nominal IIc clock rate
        //0: slow (below) 100 kHz, 33.3 MHz/400
        //1: fast (below) 400 kHz, 33.3 MHz/100
        U32 Iic_ClockSel         :1;
        U32                      :5;
        // Timeout parameter, applicable for various sequential conditions to check:
        // bus arbitration, SCL stretching and DONE status echoing 
        // Timeout = IIC clock period * 2^(Timer)							
        U32 Iic_Timer            :4;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } IicControl; 

  U8 FillerAt0x4[0xC];

#define VIDEOOUTPUT_1_OFFS 0x10

  union _VIDEOOUTPUT_1_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // 1: reserve necessary GPIO pins
        U32 Vp_EnPort            :1;
        // Enable GPIO Video output port
        // 00: GPIO video port disabled(usable for other purposes)
        // 01:  8 bit video output mode on port A(port C usable for other purposes)
        // 10:  8 bit video output mode on port C(port A usable for other purposes)
        // 11: 16 bit video output mode on port A(luma) and C(chroma)
        U32 Vp_EnVideo           :2;
        // Swap chroma UV sequence at GPIO video output port
        // 8 bit video modes at port A or C:
        //     -> 0: Sequence: U-Y-V-Y ....
        //     -> 1: Sequence: Y-U-Y-V ....
        // 16 bit video modes:
        //	   -> 0: Sequence: Y-Y .... (at port A); U-V .... (at port C)
        //     -> 1: Sequence: U-V .... (at port A); Y-Y .... (at port C)
        U32 Vp_UvSwap            :1;
        U32 Vp_EnScVbi           :1;
        U32 Vp_EnScAct           :1;
        // VGATE_L
        U32 Vp_EnXv              :1;
        // 1: enables routing of decoder output to video output port, if no other
        // stream is enabled and gated 
        U32 Vp_EnDmsd            :1;
        U32                      :4;
        // Enable GPIO video out according to decoder lock status:
        // 0: output video data from sources "as is"
        // 1: video decoder lock status locked => output video data " as is"
        //    video decoder lock status NOT locked => output blanking values
        //    (as specified by VpBlank8010) and remove enable sync signals (tie to '0')
        U32 Vp_EnLock            :1;
        U32                      :1;
        // Video decoder (i.e. DMSD) source stream selector
        // Depending on the value of this 2 bit register, the V_ITU vertical reference
        // signal and the programmable VGATE_L (programmed in the DMSD
        // munit) the DMSD will output comb-filtered YUV data, straight (non
        // comb-filtered) YUV data or raw (non decoded) CVBS data.(cf. table 5).
        // NOTE: This effects the input of the Scaler as well as the GPIO video port!
        U32 Vp_DecRawVbi         :2;
        // Insert ITU/VIP codes on port A, if set to '1'
        U32 Vp_EnCodeA           :1;
        // Insert ITU/VIP codes on port C, if set to '1'
        U32 Vp_EnCodeC           :1;
        // Generate Hamming code bits for ITU/VIP (T-), V-, F-, and H bits, if set to '1'
        U32 Vp_GenHamm           :1;
        // 0: blanking values: luma = 00 hex, chroma = 00 hex
        // 1: blanking values: luma - 10 hex, chroma = 80 hex
        U32 Vp_Blank8010         :1;
        // Keep last video data set stable @ the video output until new valid data is
        // send. Required for e.g. VMI or DMSD Video formats
        // 0: non qualified video data samples are carrying blanking values during
        // horiz. active video
        // 1: non qualified video data samples are carrying the same value as the
        // last qualified video data sample during horiz. active video	
        U32 Vp_Keep              :1;
        // VIP code: T-Bit content
        // 00: V_ITU (VBLNK_CCIR_L) from decoder (default)
        // 01: indicates VBI data @ scaler output (SC_VBI_ACT)
        // 10: indicates act video data @ scaler output (SC_VGATE & !VBI_flag)
        // 11: VGATE_L from decoder
        U32 Vp_TCode             :2;
        // VIP code: T-Bit polarity
        // 0: non inverted V-Bit
        // 1: inverted V-Bit
        U32 Vp_TCodeP            :1;
        // ITU/VIP code: F-Bit content
        // 00: F_ITU from decoder (default)
        // 01: Scaler page: A->F = 1, B->F = 0
        // 10: F = SC_out_FID (Decoder dependant, sc_fid)
        // 11: F = SC_reg_FID (Scaler dpendant, sc_page)
        U32 Vp_FCode             :2;
        // ITU/VIP code: F-Bit polarity
        // 0: non inverted F-Bit
        // 1: inverted F-Bit
        U32 Vp_FCodeP            :1;
        // ITU/VIP code : V-Bit content
        // 00: V_ITU(VBLNK_CCIR_L) from decode (default)
        // 01: indicates VBI data @ scaler outp (SC_VBI_ACT)
        // 10: indicates act video data @ scaler output (SC_VGATE & !VBI_flag)
        U32 Vp_VCode             :2;
        // ITU/VIP code : V-Bit polarity
        // 0: not inverted V-Bit
        // 1: inverted V-Bit
        U32 Vp_VCodeP            :1;
        U32                      :2;
    # else /* !BIG_ENDIAN */
        U32                      :2;
        // ITU/VIP code : V-Bit polarity
        // 0: not inverted V-Bit
        // 1: inverted V-Bit
        U32 Vp_VCodeP            :1;
        // ITU/VIP code : V-Bit content
        // 00: V_ITU(VBLNK_CCIR_L) from decode (default)
        // 01: indicates VBI data @ scaler outp (SC_VBI_ACT)
        // 10: indicates act video data @ scaler output (SC_VGATE & !VBI_flag)
        U32 Vp_VCode             :2;
        // ITU/VIP code: F-Bit polarity
        // 0: non inverted F-Bit
        // 1: inverted F-Bit
        U32 Vp_FCodeP            :1;
        // ITU/VIP code: F-Bit content
        // 00: F_ITU from decoder (default)
        // 01: Scaler page: A->F = 1, B->F = 0
        // 10: F = SC_out_FID (Decoder dependant, sc_fid)
        // 11: F = SC_reg_FID (Scaler dpendant, sc_page)
        U32 Vp_FCode             :2;
        // VIP code: T-Bit polarity
        // 0: non inverted V-Bit
        // 1: inverted V-Bit
        U32 Vp_TCodeP            :1;
        // VIP code: T-Bit content
        // 00: V_ITU (VBLNK_CCIR_L) from decoder (default)
        // 01: indicates VBI data @ scaler output (SC_VBI_ACT)
        // 10: indicates act video data @ scaler output (SC_VGATE & !VBI_flag)
        // 11: VGATE_L from decoder
        U32 Vp_TCode             :2;
        // Keep last video data set stable @ the video output until new valid data is
        // send. Required for e.g. VMI or DMSD Video formats
        // 0: non qualified video data samples are carrying blanking values during
        // horiz. active video
        // 1: non qualified video data samples are carrying the same value as the
        // last qualified video data sample during horiz. active video	
        U32 Vp_Keep              :1;
        // 0: blanking values: luma = 00 hex, chroma = 00 hex
        // 1: blanking values: luma - 10 hex, chroma = 80 hex
        U32 Vp_Blank8010         :1;
        // Generate Hamming code bits for ITU/VIP (T-), V-, F-, and H bits, if set to '1'
        U32 Vp_GenHamm           :1;
        // Insert ITU/VIP codes on port C, if set to '1'
        U32 Vp_EnCodeC           :1;
        // Insert ITU/VIP codes on port A, if set to '1'
        U32 Vp_EnCodeA           :1;
        // Video decoder (i.e. DMSD) source stream selector
        // Depending on the value of this 2 bit register, the V_ITU vertical reference
        // signal and the programmable VGATE_L (programmed in the DMSD
        // munit) the DMSD will output comb-filtered YUV data, straight (non
        // comb-filtered) YUV data or raw (non decoded) CVBS data.(cf. table 5).
        // NOTE: This effects the input of the Scaler as well as the GPIO video port!
        U32 Vp_DecRawVbi         :2;
        U32                      :1;
        // Enable GPIO video out according to decoder lock status:
        // 0: output video data from sources "as is"
        // 1: video decoder lock status locked => output video data " as is"
        //    video decoder lock status NOT locked => output blanking values
        //    (as specified by VpBlank8010) and remove enable sync signals (tie to '0')
        U32 Vp_EnLock            :1;
        U32                      :4;
        // 1: enables routing of decoder output to video output port, if no other
        // stream is enabled and gated 
        U32 Vp_EnDmsd            :1;
        // VGATE_L
        U32 Vp_EnXv              :1;
        U32 Vp_EnScAct           :1;
        U32 Vp_EnScVbi           :1;
        // Swap chroma UV sequence at GPIO video output port
        // 8 bit video modes at port A or C:
        //     -> 0: Sequence: U-Y-V-Y ....
        //     -> 1: Sequence: Y-U-Y-V ....
        // 16 bit video modes:
        //	   -> 0: Sequence: Y-Y .... (at port A); U-V .... (at port C)
        //     -> 1: Sequence: U-V .... (at port A); Y-Y .... (at port C)
        U32 Vp_UvSwap            :1;
        // Enable GPIO Video output port
        // 00: GPIO video port disabled(usable for other purposes)
        // 01:  8 bit video output mode on port A(port C usable for other purposes)
        // 10:  8 bit video output mode on port C(port A usable for other purposes)
        // 11: 16 bit video output mode on port A(luma) and C(chroma)
        U32 Vp_EnVideo           :2;
        // 1: reserve necessary GPIO pins
        U32 Vp_EnPort            :1;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } VideoOutput_1; 

#define VIDEOOUTPUT_2_OFFS 0x14

  union _VIDEOOUTPUT_2_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // Enable the 1st GPIO video auxiliary port and select signal type:
        // 000: OFF		: no signal routed, pin usable for other (GPIO) purposes
        // 001: Combi-HGATE: indicates any active line in enabled regions
        // 010: CREF		: for DMSD-2 style interfaceing
        // 011: CombiVGATE : indicates any enabled active region
        // 100: inverted Combi/HGATE
        // 101: inverted CREF
        // 110: inverted Combi-VGATE
        // 111: RTCO		: Real Time control output to lock external DENC's
        U32 Vp_EnAux1            :3;
        // Select the 2nd GPIO video auxiliary (input) port function:
        // The V_AUX2 input:
        // 00: won’t be evaluated (OFF).
        // 01: is the external clock input for ADC Modes (VSB sidecar,
        // no meaning for the video port)
        // 10: forces all video output pins (incl. V_CLK) to be tri-stated if V_AUX2
        // (i.e. GPIO 18) set to ’1’ (latency is minimum 4 v_CLK clock cycles)
        // 11: is not defined (i.e. won’t be evaluated, => OFF)
        U32 Vp_Aux2Fct           :2;
        U32                      :4;
        // Video horizontal sync polarity at the video output port:
        // 0: NON inverted video clock (default)
        // 1: inverted video clock
        U32 Vp_HsP               :1;
        // Video horizontal sync type at the video output port:
        // 00: OFF: no H-Sync signal; pin usable for other (GPIO) purposes
        // 01: Combi-HREF (default)
        // 10: Plain HREF from decoder
        // 11: HS: Programmable horizontal sync from decoder
        U32 Vp_HsTyp             :2;
        // Video vertical sync polarity at the video output port:
        // 0: NON inverted video clock (default)
        // 1: inverted video clock
        U32 Vp_VsP               :1;
        // Video vertical sync type at the video output port:
        // 000: OFF     : no V-sync signal; pin usable for other (GPIO) purposes
        // 001: V123    : 'analog' V-sync with half line timing from decoder (default)
        // 010: V_ITU	: BLNK_CCIR_L from decoder (used for ITU)
        // 011: VGATE_L : Programmable vertical gate signal from decoder
        // 100: reserved, do not use!!!!!!!!
        // 101: reserved, do not use!!!!!!!!
        // 110: F_ITU   : Field ID according ITU from decoder
        // 111: SC_FID  : Field ID as generated by scaler output
        U32 Vp_VsTyp             :3;
        // Switch the PICLK OFF
        // -> 0: The PICLK (@GPIO24) is active, i.e. switched on (default)
        // -> 1: The PICLK (@GPIO24) is switched off (i.e. the pin is in tristate mode)
        U32 Vp_PiClkOff          :4;
        // vp_clk_ctrl: Video output clock control
        // -> 0xxx: Use NON gated clock
        // -> 1xxx: Use gated clock
        // -> x0xx: clock is not delayed
        // -> x1xx: clock is delayed
        // -> xx0x: Don’t invert the clock
        // -> xx1x: invert the clock
        // -> xxx0: Video clock based on ’clk0’ (@ 27 MHz)
        // -> xxx1: Video clock based on ’clk1’ (@13,5 MHz)
        U32 Vp_ClkCtrl           :4;
        // horiz. Sync. Squeeze counter
        // This counter defines the amount of time, the scaler video output is stalled
        // prior to receive data from the scaler. The time is measured in number
        // scaler backend clock cycles beginning with a horizontal sync. event (on
        // internal sc_hagte signal??????).
        // This is used to obtain a continous video data stream at the video output.
        U32 Vp_SqeeCnt           :8;
    # else /* !BIG_ENDIAN */
        // horiz. Sync. Squeeze counter
        // This counter defines the amount of time, the scaler video output is stalled
        // prior to receive data from the scaler. The time is measured in number
        // scaler backend clock cycles beginning with a horizontal sync. event (on
        // internal sc_hagte signal??????).
        // This is used to obtain a continous video data stream at the video output.
        U32 Vp_SqeeCnt           :8;
        // vp_clk_ctrl: Video output clock control
        // -> 0xxx: Use NON gated clock
        // -> 1xxx: Use gated clock
        // -> x0xx: clock is not delayed
        // -> x1xx: clock is delayed
        // -> xx0x: Don’t invert the clock
        // -> xx1x: invert the clock
        // -> xxx0: Video clock based on ’clk0’ (@ 27 MHz)
        // -> xxx1: Video clock based on ’clk1’ (@13,5 MHz)
        U32 Vp_ClkCtrl           :4;
        // Switch the PICLK OFF
        // -> 0: The PICLK (@GPIO24) is active, i.e. switched on (default)
        // -> 1: The PICLK (@GPIO24) is switched off (i.e. the pin is in tristate mode)
        U32 Vp_PiClkOff          :4;
        // Video vertical sync type at the video output port:
        // 000: OFF     : no V-sync signal; pin usable for other (GPIO) purposes
        // 001: V123    : 'analog' V-sync with half line timing from decoder (default)
        // 010: V_ITU	: BLNK_CCIR_L from decoder (used for ITU)
        // 011: VGATE_L : Programmable vertical gate signal from decoder
        // 100: reserved, do not use!!!!!!!!
        // 101: reserved, do not use!!!!!!!!
        // 110: F_ITU   : Field ID according ITU from decoder
        // 111: SC_FID  : Field ID as generated by scaler output
        U32 Vp_VsTyp             :3;
        // Video vertical sync polarity at the video output port:
        // 0: NON inverted video clock (default)
        // 1: inverted video clock
        U32 Vp_VsP               :1;
        // Video horizontal sync type at the video output port:
        // 00: OFF: no H-Sync signal; pin usable for other (GPIO) purposes
        // 01: Combi-HREF (default)
        // 10: Plain HREF from decoder
        // 11: HS: Programmable horizontal sync from decoder
        U32 Vp_HsTyp             :2;
        // Video horizontal sync polarity at the video output port:
        // 0: NON inverted video clock (default)
        // 1: inverted video clock
        U32 Vp_HsP               :1;
        U32                      :4;
        // Select the 2nd GPIO video auxiliary (input) port function:
        // The V_AUX2 input:
        // 00: won’t be evaluated (OFF).
        // 01: is the external clock input for ADC Modes (VSB sidecar,
        // no meaning for the video port)
        // 10: forces all video output pins (incl. V_CLK) to be tri-stated if V_AUX2
        // (i.e. GPIO 18) set to ’1’ (latency is minimum 4 v_CLK clock cycles)
        // 11: is not defined (i.e. won’t be evaluated, => OFF)
        U32 Vp_Aux2Fct           :2;
        // Enable the 1st GPIO video auxiliary port and select signal type:
        // 000: OFF		: no signal routed, pin usable for other (GPIO) purposes
        // 001: Combi-HGATE: indicates any active line in enabled regions
        // 010: CREF		: for DMSD-2 style interfaceing
        // 011: CombiVGATE : indicates any enabled active region
        // 100: inverted Combi/HGATE
        // 101: inverted CREF
        // 110: inverted Combi-VGATE
        // 111: RTCO		: Real Time control output to lock external DENC's
        U32 Vp_EnAux1            :3;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } VideoOutput_2; 

#define ADCCONVERTEROUTPUT_OFFS 0x18

  union _ADCCONVERTEROUTPUT_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :29;
        // SwaP VSB data sources
        // 0: (default) VSB_A data source is video ADC A, VSB_B data source is
        // video ADC B
        // 1: VSB_A data source is video ADC B and VSB_B data source is video
        // ADC A
        U32 SwapVsbSrc           :1;
        // Enable VSB_A output port
        // 0: No raw ADB data output (default)
        // 1: Mapped to GPIO {15 - 8, 17} (MSB to LSB). Direct output of Video
        // ADC data to VSB_A port of GPIO
        U32 VsbA_En              :1;
        // Enable VSB_B output port
        // 0: No raw ADB data output (default)
        // 1: Mapped to GPIO {7 - 0, 23} (MSB to LSB). Direct output of Video
        // ADC data to VSB_B port of GPIO
        U32 VsbB_En              :1;
    # else /* !BIG_ENDIAN */
        // Enable VSB_B output port
        // 0: No raw ADB data output (default)
        // 1: Mapped to GPIO {7 - 0, 23} (MSB to LSB). Direct output of Video
        // ADC data to VSB_B port of GPIO
        U32 VsbB_En              :1;
        // Enable VSB_A output port
        // 0: No raw ADB data output (default)
        // 1: Mapped to GPIO {15 - 8, 17} (MSB to LSB). Direct output of Video
        // ADC data to VSB_A port of GPIO
        U32 VsbA_En              :1;
        // SwaP VSB data sources
        // 0: (default) VSB_A data source is video ADC A, VSB_B data source is
        // video ADC B
        // 1: VSB_A data source is video ADC B and VSB_B data source is video
        // ADC A
        U32 SwapVsbSrc           :1;
        U32                      :29;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } AdcConverterOutput; 

  U8 FillerAt0x1C[0x4];

#define TSTREAMINTERFACE_1_OFFS 0x20

  union _TSTREAMINTERFACE_1_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :6;
        // Use the inverted transport stream clock, which accompanies the
        // incoming data at transport stream interface:
        // 0: Receive the data using the transprot stream clock as is.
        // 1: Receive the data using the inverted transprot stream clock.
        U32 Ts_InvClk            :1;
        // High active soft reset signal for GPIO TS Interface required (needed,
        // since a clock for the TS If. may not be avail during regular HW reset)
        // 0: Softreset inactive
        // 1: Softreset active (default after reset, since
        U32 Ts_SoftRes           :1;
        // Serial Interface: serial input Mode of TS interface is enabled if set to '1'
        U32 Ts_SEn               :1;
        // Serial Transport Stream, EValuate Leading EDGe of SOP
        // 1: leading edge of SOP wil be evaluated (default)
        // After enabling the Serial Transport Stream Interface the first packet or
        // audio sample received is always synchronized to the leading edge of the
        // SOP signal als long as ‘ts_ev_ledg’ is set to ‘1’ at that time.
        // 0: leading edge of SOP NOT considered
        // The polarity of the leading edge is defined by the “ts_ssopp” bit.
        U32 Ts_EvLedg            :1;
        // Serial Transport Stream, EValuate Trailing EDGe of SOP
        // 1: trailing edge of SOP evaluated
        // 0: trailing edge of SOP NOT considered (default)
        // Will be enabled for receiving IIS data (SOP than will be handeled as IIS
        // WS signal)
        // The trailing edge corresponds to the inverted “ts_ssopp” bit.
        U32 Ts_EvTedg            :1;
        // Serial Transport Stream: Generate EOP after having received data
        // syncronized to the Leading Edge of SOP
        // 1: (default) This is always used in case of transport streams, which are
        // always received using the leding edge of the SOP signal. This is used as
        // well for audio to write left and right samples in separate dwords within
        // DMA, i.e. each audio sample is handeled as a separate "packet")
        // 0: This setting is used only in case if two successive audio samples
        // shall be received as one packet.
        // Either ts_eop_le or ts_eop_te must be set, otherwise no EOP will be
        // generated.
        // The polarity of the leading edge is defined by the “ts_ssopp” bit.
        U32 Ts_EopLe             :1;
        // Serial Transport Stream: Generate EOP after having received data
        // syncronized to Trailing Edge of SOP
        // 1: This is needed only for audio in, to write left and right samples in
        // separate dwords within DMA, i.e. each audio sample is handeled as a
        // separate "packet").
        // 0: (default) No EOP to DMA generated by the trailling edge of SOP
        // Transport streams are only received on the leading edge of the SOP
        // signal.
        // The polarity of the trailing edge is the inverted “ts_ssopp” bit (address 16
        // hex, bit 6).
        U32 Ts_EopTe             :1;
        // Serial Transport Stream: flip MSB’s and LSB’s:
        // 0: transfer byte to DMA as received (default).
        // 1: flip MSB’s and LSB’s bit by bit within a byte and then transfer to
        // DMA.
        U32 Ts_Bitflip           :1;
        // Serial Transport Stream, SHIFT serial data for one clock cyle:
        // 0: Fist databit is received with SOP edge (default).
        // 1: Fist databit is received one clock cycle after SOP edge. (only
        // needed for serial audio in)
        U32 Ts_Shift             :1;
        U32                      :1;
        // Parallel Transport Stream, Number of expected bytes per packet -1
        U32 Ts_Pbpp              :8;
        // Parallel Interface: Enable Parallel Transport Stream Interface (higher priority than TsSEn)
        U32 Ts_PEn               :1;
        // Parallel Interface: Transport Stream Start of packet Polarity
        // 1: high active (default)
        // 0: low active
        U32 Ts_Psopp             :1;
        // Parallel Interface: Transport stream VALid signal Polarity
        // 1: high active (default)
        // 0: low active
        U32 Ts_Pvalp             :1;
        // Parallel Interface: Force the VALid
        // If set to '1' - this forces any data ta the data input to accepted as valid
        // (valid signal line will be ignored). This allows at the same time to use the 
        // external pin again for other purposes
        U32 Ts_PvalF             :1;
        // Parallel Interface: Transport stream LOCK detection Polarity
        // 1: high active (default)
        // 0: low active
        U32 Ts_Plockp            :1;
        // Parallel Interface: Force LOCK status
        // 1: force lock status, i.e. ignore GiTsLock (default)
        // 0: evaluate GiTsLock
        U32 Ts_PlockF            :1;
        U32                      :2;
    # else /* !BIG_ENDIAN */
        U32                      :2;
        // Parallel Interface: Force LOCK status
        // 1: force lock status, i.e. ignore GiTsLock (default)
        // 0: evaluate GiTsLock
        U32 Ts_PlockF            :1;
        // Parallel Interface: Transport stream LOCK detection Polarity
        // 1: high active (default)
        // 0: low active
        U32 Ts_Plockp            :1;
        // Parallel Interface: Force the VALid
        // If set to '1' - this forces any data ta the data input to accepted as valid
        // (valid signal line will be ignored). This allows at the same time to use the 
        // external pin again for other purposes
        U32 Ts_PvalF             :1;
        // Parallel Interface: Transport stream VALid signal Polarity
        // 1: high active (default)
        // 0: low active
        U32 Ts_Pvalp             :1;
        // Parallel Interface: Transport Stream Start of packet Polarity
        // 1: high active (default)
        // 0: low active
        U32 Ts_Psopp             :1;
        // Parallel Interface: Enable Parallel Transport Stream Interface (higher priority than TsSEn)
        U32 Ts_PEn               :1;
        // Parallel Transport Stream, Number of expected bytes per packet -1
        U32 Ts_Pbpp              :8;
        U32                      :1;
        // Serial Transport Stream, SHIFT serial data for one clock cyle:
        // 0: Fist databit is received with SOP edge (default).
        // 1: Fist databit is received one clock cycle after SOP edge. (only
        // needed for serial audio in)
        U32 Ts_Shift             :1;
        // Serial Transport Stream: flip MSB’s and LSB’s:
        // 0: transfer byte to DMA as received (default).
        // 1: flip MSB’s and LSB’s bit by bit within a byte and then transfer to
        // DMA.
        U32 Ts_Bitflip           :1;
        // Serial Transport Stream: Generate EOP after having received data
        // syncronized to Trailing Edge of SOP
        // 1: This is needed only for audio in, to write left and right samples in
        // separate dwords within DMA, i.e. each audio sample is handeled as a
        // separate "packet").
        // 0: (default) No EOP to DMA generated by the trailling edge of SOP
        // Transport streams are only received on the leading edge of the SOP
        // signal.
        // The polarity of the trailing edge is the inverted “ts_ssopp” bit (address 16
        // hex, bit 6).
        U32 Ts_EopTe             :1;
        // Serial Transport Stream: Generate EOP after having received data
        // syncronized to the Leading Edge of SOP
        // 1: (default) This is always used in case of transport streams, which are
        // always received using the leding edge of the SOP signal. This is used as
        // well for audio to write left and right samples in separate dwords within
        // DMA, i.e. each audio sample is handeled as a separate "packet")
        // 0: This setting is used only in case if two successive audio samples
        // shall be received as one packet.
        // Either ts_eop_le or ts_eop_te must be set, otherwise no EOP will be
        // generated.
        // The polarity of the leading edge is defined by the “ts_ssopp” bit.
        U32 Ts_EopLe             :1;
        // Serial Transport Stream, EValuate Trailing EDGe of SOP
        // 1: trailing edge of SOP evaluated
        // 0: trailing edge of SOP NOT considered (default)
        // Will be enabled for receiving IIS data (SOP than will be handeled as IIS
        // WS signal)
        // The trailing edge corresponds to the inverted “ts_ssopp” bit.
        U32 Ts_EvTedg            :1;
        // Serial Transport Stream, EValuate Leading EDGe of SOP
        // 1: leading edge of SOP wil be evaluated (default)
        // After enabling the Serial Transport Stream Interface the first packet or
        // audio sample received is always synchronized to the leading edge of the
        // SOP signal als long as ‘ts_ev_ledg’ is set to ‘1’ at that time.
        // 0: leading edge of SOP NOT considered
        // The polarity of the leading edge is defined by the “ts_ssopp” bit.
        U32 Ts_EvLedg            :1;
        // Serial Interface: serial input Mode of TS interface is enabled if set to '1'
        U32 Ts_SEn               :1;
        // High active soft reset signal for GPIO TS Interface required (needed,
        // since a clock for the TS If. may not be avail during regular HW reset)
        // 0: Softreset inactive
        // 1: Softreset active (default after reset, since
        U32 Ts_SoftRes           :1;
        // Use the inverted transport stream clock, which accompanies the
        // incoming data at transport stream interface:
        // 0: Receive the data using the transprot stream clock as is.
        // 1: Receive the data using the inverted transprot stream clock.
        U32 Ts_InvClk            :1;
        U32                      :6;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } TStreamInterface_1; 

#define TSTREAMINTERFACE_2_OFFS 0x24

  union _TSTREAMINTERFACE_2_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :8;
        // Disable adding line pitch (for audio mode)
        // 0: (default) Line Pitch will be added after EOP from GPIO Transport
        // stream If.
        // 1: Disable adding the line pitch after EOP
        U32 Ts_NoPitch           :1;
        // Collapse packets (used for audio mode to fit two "packets" (i.e. two 16 bit
        // audio samples) into one DWord in the DMA block
        // 0: (default) each end of packet generated by the Transport Stream
        // interface will cause that writing the next data will start in a new DWord in
        // the GPIO2DMA block
        // 1: GPIO2DMA Will ignore the EOP in that the next data is put into the
        // same Dword.
        // The use of this bit makes only sense if NO_PITCH is enabled too.
        U32 Ts_Collapse          :1;
        // Number of packets to be transferred into a host buffer.
        // Value to be programmed is: number_of_packets - 1
        // (default: 22’h137 = 22’d311, i.e.: 312 packets)
        U32 Ts_Nop              :22;
    # else /* !BIG_ENDIAN */
        // Number of packets to be transferred into a host buffer.
        // Value to be programmed is: number_of_packets - 1
        // (default: 22’h137 = 22’d311, i.e.: 312 packets)
        U32 Ts_Nop              :22;
        // Collapse packets (used for audio mode to fit two "packets" (i.e. two 16 bit
        // audio samples) into one DWord in the DMA block
        // 0: (default) each end of packet generated by the Transport Stream
        // interface will cause that writing the next data will start in a new DWord in
        // the GPIO2DMA block
        // 1: GPIO2DMA Will ignore the EOP in that the next data is put into the
        // same Dword.
        // The use of this bit makes only sense if NO_PITCH is enabled too.
        U32 Ts_Collapse          :1;
        // Disable adding line pitch (for audio mode)
        // 0: (default) Line Pitch will be added after EOP from GPIO Transport
        // stream If.
        // 1: Disable adding the line pitch after EOP
        U32 Ts_NoPitch           :1;
        U32                      :8;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } TStreamInterface_2; 

  U8 FillerAt0x28[0x8];

#define GPIOPORTMODESEL_OFFS 0x30

  union _GPIOPORTMODESEL_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // Rescan GPI values (on request of the user)
        // Rescan of GPI Values will only occur if this 
        // programming register is set from zero to one (i.e. only a 
        // rising edge will lead to an update GpStst register.
        // Rescan only effects those GpStat values, which are defined as 
        // general purpose inputs in the GpMode register (corresponding bit = 0)
        U32 Gp_Rescan            :1;
        U32                      :3;
        // Static GPIO Port register host write enable register
        // Each bit of this register corresponds to a bit (same bit number) of the 
        // static GPIO Port Status Registers (SpStat).
        // The function of each bit 'n' is:
        // 0: read mode (default)
        //    The corresponding bit 'n' of the static GPIO Port Status Register (SpStat)
        //    will be set by the external signal driving the corresponding GPIO pin.
        //    Hence a write action on the corresponding bit of the SpStat register will
        //    have no effect!
        // 1: write mode:
        //    Unless the corresponding GPIO pin is not used by other GPIO ports, its
        //    output value (1 or 0) will be defined by the value written to corresponding
        //    static GPIO port status register (SpStat) bit.
        U32 Gp_Mode             :28;
    # else /* !BIG_ENDIAN */
        // Static GPIO Port register host write enable register
        // Each bit of this register corresponds to a bit (same bit number) of the 
        // static GPIO Port Status Registers (SpStat).
        // The function of each bit 'n' is:
        // 0: read mode (default)
        //    The corresponding bit 'n' of the static GPIO Port Status Register (SpStat)
        //    will be set by the external signal driving the corresponding GPIO pin.
        //    Hence a write action on the corresponding bit of the SpStat register will
        //    have no effect!
        // 1: write mode:
        //    Unless the corresponding GPIO pin is not used by other GPIO ports, its
        //    output value (1 or 0) will be defined by the value written to corresponding
        //    static GPIO port status register (SpStat) bit.
        U32 Gp_Mode             :28;
        U32                      :3;
        // Rescan GPI values (on request of the user)
        // Rescan of GPI Values will only occur if this 
        // programming register is set from zero to one (i.e. only a 
        // rising edge will lead to an update GpStst register.
        // Rescan only effects those GpStat values, which are defined as 
        // general purpose inputs in the GpMode register (corresponding bit = 0)
        U32 Gp_Rescan            :1;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } GpioPortModeSel; 

#define GPIOPORTSTATUS_OFFS 0x34

  union _GPIOPORTSTATUS_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :4;
        // Static GPIO Port Status Register (read & write)
        // (refer to Static GPIO Port register host mode enable register ‘gp_mode’)
        U32 Gp_Stat             :28;
    # else /* !BIG_ENDIAN */
        // Static GPIO Port Status Register (read & write)
        // (refer to Static GPIO Port register host mode enable register ‘gp_mode’)
        U32 Gp_Stat             :28;
        U32                      :4;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } GpioPortStatus; 

  U8 FillerAt0x38[0x8];

#define AUDIOOUTSEL_OFFS 0x40

  union _AUDIOOUTSEL_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :31;
        // GPIO Audio OUTput ENable:
        // 0: GPIO audio output is disabled (default)
        // 1: GPIO audio output is enabled
        // The audio data source corresponds to the GaOutSel register setting
        U32 Ga_OutEn             :1;
    # else /* !BIG_ENDIAN */
        // GPIO Audio OUTput ENable:
        // 0: GPIO audio output is disabled (default)
        // 1: GPIO audio output is enabled
        // The audio data source corresponds to the GaOutSel register setting
        U32 Ga_OutEn             :1;
        U32                      :31;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } AudioOutSel; 

  U8 FillerAt0x44[0xC];

#define TESTMODE_OFFS 0x50

  union _TESTMODE_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :16;
        // Test Analog Output Select
        // Defines the analog signal to be output on the ’PRESET’ pin (AThe analog
        // output function must be enabled using ’tst_en_aout’)
        // -> 3’b000: VCH1 (Video Channel 1)
        // -> 3’b001: VCH2 (Video Channel 2)
        // -> 3’b010: ACH1 (Audio Channel)
        // -> 3’b011: BFOUTV (LFCO Video)
        // -> 3’b100: BFOUTA (LFCO Audio)
        // -> 3’b101: GND
        // -> 3’b110: RESERVED 1
        // -> 3’b111: RESERVED 2
        U32 Tst_AOutSel          :3;
        // Enable signal for analog AOUT function on ‘PRESET ‘ (GPIO 28) output
        // pin.
        // -> 0: The ‘PRESET’ pin carries the peripheral RESET function (default).
        // -> 1: The ‘PRESET’ pin carries the analog output test signals as defined
        // by ‘tst_mod’ bits.
        // In case of a reset AOUT will be disabled. Default value is ’0’, i.e. Reset
        // function on GPIO28 (= PRESET)
        U32 Tst_EnAOut           :1;
        // Test mode register; set through GPIO Munit default is 4’b0000, i.e.
        // application mode.
        // -> 4’b0000: application_mode.
        // -> 4’b0001: tst_video_adc_a
        // -> 4’b0010: tst_video_adc_b
        // -> 4’b0011: tst_video_gain_a
        // -> 4’b0100: tst_video_gain_b
        // -> 4’b0101: tst_video_pll
        // -> 4’b0110: tst_sif_adc
        // -> 4’b0111: tst_sif_gain
        // -> 4’b1000: tst_audio_adc_r
        // -> 4’b1001: tst_audio_adc_l
        // -> 4’b1010: tst_audio_dac_r
        // -> 4’b1011: tst_audio_dac_l
        // -> 4’b1100: tst_audio_pll
        // -> 4’b1101: tst_sif_adc_regd (same as tst_sif_adc, but registered with test
        // clock)
        // -> 4’b1110: (not defined)
        // -> 4’b1111: (not defined)
        U32 Tst_Mode             :4;
        U32                      :7;
        // Force PRESET (=GPIO28) (only AOUT functionon the same pin must be
        // disabled)
        // -> ’0’: (default) the reset @ GPIO28 is active
        // -> ’1’: the reset @ GPIO28 forced to inactive
        // NOTE: PRESET reset signal must be deactivated by SW in any case.
        U32 FPreset              :1;
    # else /* !BIG_ENDIAN */
        // Force PRESET (=GPIO28) (only AOUT functionon the same pin must be
        // disabled)
        // -> ’0’: (default) the reset @ GPIO28 is active
        // -> ’1’: the reset @ GPIO28 forced to inactive
        // NOTE: PRESET reset signal must be deactivated by SW in any case.
        U32 FPreset              :1;
        U32                      :7;
        // Test mode register; set through GPIO Munit default is 4’b0000, i.e.
        // application mode.
        // -> 4’b0000: application_mode.
        // -> 4’b0001: tst_video_adc_a
        // -> 4’b0010: tst_video_adc_b
        // -> 4’b0011: tst_video_gain_a
        // -> 4’b0100: tst_video_gain_b
        // -> 4’b0101: tst_video_pll
        // -> 4’b0110: tst_sif_adc
        // -> 4’b0111: tst_sif_gain
        // -> 4’b1000: tst_audio_adc_r
        // -> 4’b1001: tst_audio_adc_l
        // -> 4’b1010: tst_audio_dac_r
        // -> 4’b1011: tst_audio_dac_l
        // -> 4’b1100: tst_audio_pll
        // -> 4’b1101: tst_sif_adc_regd (same as tst_sif_adc, but registered with test
        // clock)
        // -> 4’b1110: (not defined)
        // -> 4’b1111: (not defined)
        U32 Tst_Mode             :4;
        // Enable signal for analog AOUT function on ‘PRESET ‘ (GPIO 28) output
        // pin.
        // -> 0: The ‘PRESET’ pin carries the peripheral RESET function (default).
        // -> 1: The ‘PRESET’ pin carries the analog output test signals as defined
        // by ‘tst_mod’ bits.
        // In case of a reset AOUT will be disabled. Default value is ’0’, i.e. Reset
        // function on GPIO28 (= PRESET)
        U32 Tst_EnAOut           :1;
        // Test Analog Output Select
        // Defines the analog signal to be output on the ’PRESET’ pin (AThe analog
        // output function must be enabled using ’tst_en_aout’)
        // -> 3’b000: VCH1 (Video Channel 1)
        // -> 3’b001: VCH2 (Video Channel 2)
        // -> 3’b010: ACH1 (Audio Channel)
        // -> 3’b011: BFOUTV (LFCO Video)
        // -> 3’b100: BFOUTA (LFCO Audio)
        // -> 3’b101: GND
        // -> 3’b110: RESERVED 1
        // -> 3’b111: RESERVED 2
        U32 Tst_AOutSel          :3;
        U32                      :16;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } TestMode; 

} MunitGpio;

//--------- End of MunitGpio ----------------------
//*************************************************************



  U8 FillerAt0x1D4[0x2C];

#define MUNITDMA_OFFS 0x200

//*************************************************************
//------------- MunitDma ----------------------
struct _MUNITDMA_ {  
#define REGSET0_OFFS 0x0
  _DMAREGSET_ RegSet0;
#define REGSET1_OFFS 0x10
  _DMAREGSET_ RegSet1;
#define REGSET2_OFFS 0x20
  _DMAREGSET_ RegSet2;
#define REGSET3_OFFS 0x30
  _DMAREGSET_ RegSet3;
#define REGSET4_OFFS 0x40
  _DMAREGSET_ RegSet4;
#define REGSET5_OFFS 0x50
  _DMAREGSET_ RegSet5;
#define REGSET6_OFFS 0x60
  _DMAREGSET_ RegSet6;

  U8 FillerAt0x70[0x30];

#define FIFOCONFIG_1_OFFS 0xA0

  union _FIFOCONFIG_1_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :4;
        //End of Ram Area 4. Hardwired to 8h. Read only	
        U32 Era4                 :4;
        U32                      :4;
        //End of Ram Area 3. Equals begin of RAM area 4. Do not change this value if any	   			 
        //channel is active! Make sure that ERA3 is greater or equal than    
        //ERA2 and smaller or equal than ERA4! Note that a write access to   
        //this register will clear the RAM areas RA4 and RA3				   
        U32 Era3                 :4;
        U32                      :4;
        //End of Ram Area 2. Equals begin or RAM area 3. Do not change this value if any      			 
        //channel is active! Make sure that ERA2 is greater or equal than    
        //ERA1 and smaller or equal than ERA3! Note that a write access to   
        //this register will clear the RAM areas RA3 and RA2.                
        U32 Era2                 :4;
        U32                      :4;
        //End of Ram Area 1. Equals begin of RAM area 2. Do not change this value if any      			 
        //channel is active! Make sure that ERA1 is smaller or equal than    
        //ERA2! Note that a write access to this register will clear the RAM 
        //areas RA2 and RA1.												   
        U32 Era1                 :4;
    # else /* !BIG_ENDIAN */
        //End of Ram Area 1. Equals begin of RAM area 2. Do not change this value if any      			 
        //channel is active! Make sure that ERA1 is smaller or equal than    
        //ERA2! Note that a write access to this register will clear the RAM 
        //areas RA2 and RA1.												   
        U32 Era1                 :4;
        U32                      :4;
        //End of Ram Area 2. Equals begin or RAM area 3. Do not change this value if any      			 
        //channel is active! Make sure that ERA2 is greater or equal than    
        //ERA1 and smaller or equal than ERA3! Note that a write access to   
        //this register will clear the RAM areas RA3 and RA2.                
        U32 Era2                 :4;
        U32                      :4;
        //End of Ram Area 3. Equals begin of RAM area 4. Do not change this value if any	   			 
        //channel is active! Make sure that ERA3 is greater or equal than    
        //ERA2 and smaller or equal than ERA4! Note that a write access to   
        //this register will clear the RAM areas RA4 and RA3				   
        U32 Era3                 :4;
        U32                      :4;
        //End of Ram Area 4. Hardwired to 8h. Read only	
        U32 Era4                 :4;
        U32                      :4;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } FifoConfig_1; 

#define FIFOCONFIG_2_OFFS 0xA4

  union _FIFOCONFIG_2_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :6;
        //Threshold for RAM area 4	
        U32 Thr4                 :2;
        U32                      :6;
        //Threshold for RAM area 3	
        U32 Thr3                 :2;
        U32                      :6;
        //Threshold for RAM area 2	
        U32 Thr2                 :2;
        U32                      :6;
        //Threshold for RAM area 1	
        U32 Thr1                 :2;
    # else /* !BIG_ENDIAN */
        //Threshold for RAM area 1	
        U32 Thr1                 :2;
        U32                      :6;
        //Threshold for RAM area 2	
        U32 Thr2                 :2;
        U32                      :6;
        //Threshold for RAM area 3	
        U32 Thr3                 :2;
        U32                      :6;
        //Threshold for RAM area 4	
        U32 Thr4                 :2;
        U32                      :6;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } FifoConfig_2; 

#define MAINCONTROL_OFFS 0xA8

  union _MAINCONTROL_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :16;
        //Enable video PLL
        U32 EnVideoPll           :1;
        //Enable audio PLL	
        U32 EnAudioPll           :1;
        //Enable oscillator	
        U32 EnOsc                :1;
        //Enable video frontend 1	
        U32 EnVideoFE1           :1;
        //Enable video frontend 2
        U32 EnVideoFE2           :1;
        //Enable SIF frontend	
        U32 EnSifFE              :1;
        U32                      :1;
        //Enable basis band DAC and ADC
        U32 EnBaseDACandADC      :1;
        U32                      :1;
        //Transfer enable: Opens the data input to the FIFO if set to one. If set to zero the  			 
        //					FIFO input will be closed and data remaining in the FIFO wil be transfered.	     
        U32 TE                   :7;
    # else /* !BIG_ENDIAN */
        //Transfer enable: Opens the data input to the FIFO if set to one. If set to zero the  			 
        //					FIFO input will be closed and data remaining in the FIFO wil be transfered.	     
        U32 TE                   :7;
        U32                      :1;
        //Enable basis band DAC and ADC
        U32 EnBaseDACandADC      :1;
        U32                      :1;
        //Enable SIF frontend	
        U32 EnSifFE              :1;
        //Enable video frontend 2
        U32 EnVideoFE2           :1;
        //Enable video frontend 1	
        U32 EnVideoFE1           :1;
        //Enable oscillator	
        U32 EnOsc                :1;
        //Enable audio PLL	
        U32 EnAudioPll           :1;
        //Enable video PLL
        U32 EnVideoPll           :1;
        U32                      :16;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } MainControl; 

#define STATUSINFO_OFFS 0xAC

  union _STATUSINFO_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        //FIFO emtpy: Set to one if the corresponding RAM area is empty Bit 28 corresponds with area 1.	
        U32 FE                   :4;
        //DMA_Done is set after the last data of a field/window has been transferred over the   
        //PCI bus. It is reset with the first transfer of the next field. Note 
        //that DMA_DONE may be a single cycle pulse if the FIFO runs not empty 
        //between two fields													 
        U32 DmaDone              :4;
        //Odd/Even: Reflects whether the DMA is currently operation in the buffer defined by	 						
        //					BA1 (OE = 1) or BA2 (OE = 0)										 												
        U32 OddEven              :4;
        U32                      :5;
        //MMU abort received. The transfer of the MMU finished with either target or master	 			 
        //abort. This will clear the FIFO and close the FIFO input until the	 
        //corresponding TE bit has been reset and set again. Read only!		 
        U32 MmuAr                :7;
        U32                      :1;
        //DMA abort received. The transfer of a DMA channel finished with either target or		 
        //master abort. This will clear the FIFO and close the FIFO input until
        //the corresponding TE bit has been reset and set again. Read only!	 
        U32 DmaAr                :7;
    # else /* !BIG_ENDIAN */
        //DMA abort received. The transfer of a DMA channel finished with either target or		 
        //master abort. This will clear the FIFO and close the FIFO input until
        //the corresponding TE bit has been reset and set again. Read only!	 
        U32 DmaAr                :7;
        U32                      :1;
        //MMU abort received. The transfer of the MMU finished with either target or master	 			 
        //abort. This will clear the FIFO and close the FIFO input until the	 
        //corresponding TE bit has been reset and set again. Read only!		 
        U32 MmuAr                :7;
        U32                      :5;
        //Odd/Even: Reflects whether the DMA is currently operation in the buffer defined by	 						
        //					BA1 (OE = 1) or BA2 (OE = 0)										 												
        U32 OddEven              :4;
        //DMA_Done is set after the last data of a field/window has been transferred over the   
        //PCI bus. It is reset with the first transfer of the next field. Note 
        //that DMA_DONE may be a single cycle pulse if the FIFO runs not empty 
        //between two fields													 
        U32 DmaDone              :4;
        //FIFO emtpy: Set to one if the corresponding RAM area is empty Bit 28 corresponds with area 1.	
        U32 FE                   :4;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } StatusInfo; 

#define RAMAREA1_OFFS 0xB0
  _DMARAMAREA_ RamArea1;
#define RAMAREA2_OFFS 0xB4
  _DMARAMAREA_ RamArea2;
#define RAMAREA3_OFFS 0xB8
  _DMARAMAREA_ RamArea3;
#define RAMAREA4_OFFS 0xBC
  _DMARAMAREA_ RamArea4;

#define MAINSTATUS_OFFS 0xC0

  union _MAINSTATUS_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :14;
        //FM demodulator receives stereo stream
        U32 Stereo               :1;
        //FM demodulator receives dualmono stream
        U32 Dual                 :1;
        //FM demodulator found pilot
        U32 Pilot                :1;
        //NICAM decoder receives stereo stream
        U32 Smb                  :1;
        //NICAM decoder receives dualmono stream
        U32 Dsb                  :1;
        //Nicam decoder has locked on NICAM stream
        U32 Vsdp                 :1;
        // Equals the IIC main status register
        U32 Iic_Status           :2;
        //Macrovision mode
        U32 MVM                  :3;
        //Video decoder detected 50 Hz video signal	
        U32 Fidt                 :1;
        //Decoder detected interlace mode
        U32 Intl                 :1;
        //Video decoder is ready for capture
        U32 RdCap                :1;
        //Power on reset
        U32 PwrOn                :1;
        //Load error occuring in scaler	
        U32 LoadErr              :1;
        //Trigger error orcurring in scaler	
        U32 TrigErr              :1;
        //Configuration error occurring in scaler	
        U32 ConfErr              :1;
    # else /* !BIG_ENDIAN */
        //Configuration error occurring in scaler	
        U32 ConfErr              :1;
        //Trigger error orcurring in scaler	
        U32 TrigErr              :1;
        //Load error occuring in scaler	
        U32 LoadErr              :1;
        //Power on reset
        U32 PwrOn                :1;
        //Video decoder is ready for capture
        U32 RdCap                :1;
        //Decoder detected interlace mode
        U32 Intl                 :1;
        //Video decoder detected 50 Hz video signal	
        U32 Fidt                 :1;
        //Macrovision mode
        U32 MVM                  :3;
        // Equals the IIC main status register
        U32 Iic_Status           :2;
        //Nicam decoder has locked on NICAM stream
        U32 Vsdp                 :1;
        //NICAM decoder receives dualmono stream
        U32 Dsb                  :1;
        //NICAM decoder receives stereo stream
        U32 Smb                  :1;
        //FM demodulator found pilot
        U32 Pilot                :1;
        //FM demodulator receives dualmono stream
        U32 Dual                 :1;
        //FM demodulator receives stereo stream
        U32 Stereo               :1;
        U32                      :14;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } MainStatus; 

#define INTENABLE_1_OFFS 0xC4

  union _INTENABLE_1_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        //Interrupt enable at the end of field/buffer of RAM area 4	
        U32 IntE_RA4             :8;
        //Interrupt enable at the end of field/buffer of RAM area 3	
        U32 IntE_RA3             :8;
        //Interrupt enable at the end of field/buffer of RAM area 2	
        U32 IntE_RA2             :8;
        //Interrupt enable at the end of field/buffer of RAM area 1	
        U32 IntE_RA1             :8;
    # else /* !BIG_ENDIAN */
        //Interrupt enable at the end of field/buffer of RAM area 1	
        U32 IntE_RA1             :8;
        //Interrupt enable at the end of field/buffer of RAM area 2	
        U32 IntE_RA2             :8;
        //Interrupt enable at the end of field/buffer of RAM area 3	
        U32 IntE_RA3             :8;
        //Interrupt enable at the end of field/buffer of RAM area 4	
        U32 IntE_RA4             :8;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } IntEnable_1; 

#define INTENABLE_2_OFFS 0xC8

  union _INTENABLE_2_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :14;
        //Enables interrupt at the negative edge of GPIO input 23.	
        U32 IntE_Gpio7           :1;
        //Enables interrupt at the positve edge of GPIO input 23.	
        U32 IntE_Gpio6           :1;
        //Enables interrupt at the negative edge of GPIO input 22.	
        U32 IntE_Gpio5           :1;
        //Enables interrupt at the positve edge of GPIO input 22.	
        U32 IntE_Gpio4           :1;
        //Enables interrupt at the negative edge of GPIO input 18.	
        U32 IntE_Gpio3           :1;
        //Enables interrupt at the positve edge of GPIO input 18.	
        U32 IntE_Gpio2           :1;
        //Enables interrupt at the negative edge of GPIO input 16.	
        U32 IntE_Gpio1           :1;
        //Enables interrupt at the positve edge of GPIO input 16.	
        U32 IntE_Gpio0           :1;
        //Enables an interrupt at the positive edge of LOAD_ERR	
        U32 IntE_Sc_LoadErr      :1;
        //Enables an interrupt at the positive edge of CONF_ERR			
        U32 IntE_Sc_ConfErr      :1;
        //Enables an interrupt at the positive edge of TRIG_ERR		
        U32 IntE_Sc_TrigErr      :1;
        //Enables an interrupt at any change of the macrovision mode
        U32 IntE_Dec_MMC         :1;
        //Enables an interrupt at any change of the FIDT signal
        U32 IntE_Dec_Fidt        :1;
        //Enables an interrupt at any change of the INTL signal	
        U32 IntE_Dec_Intl        :1;
        //Enables an interrupt at any change of the RDCAP signal
        U32 IntE_Dec_RdCap       :1;
        //Enables an interrupt at the PWR_ON signal
        U32 IntE_Dec_PwrOn       :1;
        //Enables an interrupt if a parity error occurs on the PCI bus	
        U32 IntE_PE              :1;
        //Enables an interrupt if a transfer using the corresponding register set finished with 
        //master abort. This may be a transfer of the MMU (read) or of the DMA channel itself (write) 
        U32 IntE_AR              :1;
    # else /* !BIG_ENDIAN */
        //Enables an interrupt if a transfer using the corresponding register set finished with 
        //master abort. This may be a transfer of the MMU (read) or of the DMA channel itself (write) 
        U32 IntE_AR              :1;
        //Enables an interrupt if a parity error occurs on the PCI bus	
        U32 IntE_PE              :1;
        //Enables an interrupt at the PWR_ON signal
        U32 IntE_Dec_PwrOn       :1;
        //Enables an interrupt at any change of the RDCAP signal
        U32 IntE_Dec_RdCap       :1;
        //Enables an interrupt at any change of the INTL signal	
        U32 IntE_Dec_Intl        :1;
        //Enables an interrupt at any change of the FIDT signal
        U32 IntE_Dec_Fidt        :1;
        //Enables an interrupt at any change of the macrovision mode
        U32 IntE_Dec_MMC         :1;
        //Enables an interrupt at the positive edge of TRIG_ERR		
        U32 IntE_Sc_TrigErr      :1;
        //Enables an interrupt at the positive edge of CONF_ERR			
        U32 IntE_Sc_ConfErr      :1;
        //Enables an interrupt at the positive edge of LOAD_ERR	
        U32 IntE_Sc_LoadErr      :1;
        //Enables interrupt at the positve edge of GPIO input 16.	
        U32 IntE_Gpio0           :1;
        //Enables interrupt at the negative edge of GPIO input 16.	
        U32 IntE_Gpio1           :1;
        //Enables interrupt at the positve edge of GPIO input 18.	
        U32 IntE_Gpio2           :1;
        //Enables interrupt at the negative edge of GPIO input 18.	
        U32 IntE_Gpio3           :1;
        //Enables interrupt at the positve edge of GPIO input 22.	
        U32 IntE_Gpio4           :1;
        //Enables interrupt at the negative edge of GPIO input 22.	
        U32 IntE_Gpio5           :1;
        //Enables interrupt at the positve edge of GPIO input 23.	
        U32 IntE_Gpio6           :1;
        //Enables interrupt at the negative edge of GPIO input 23.	
        U32 IntE_Gpio7           :1;
        U32                      :14;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } IntEnable_2; 

#define INTREGISTER_OFFS 0xCC

  union _INTREGISTER_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :14;
        //Edge on GPIO input 23: ‘1’ if INTE_GPIO[6] or/and INTE_GPIO[7] was set to ‘1’ and the corresponding edge occurred.
        U32 Int_Gpio23           :1;
        //Edge on GPIO input 22: ‘1’ if INTE_GPIO[4] or/and INTE_GPIO[5] was set to ‘1’ and the corresponding edge occurred.
        U32 Int_Gpio22           :1;
        //Edge on GPIO input 16: ‘1’ if INTE_GPIO[2] or/and INTE_GPIO[3] was set to ‘1’ and the corresponding edge occurred. 
        U32 Int_Gpio18           :1;
        //Edge on GPIO input 14: ‘1’ if INTE_GPIO[0] or/and INTE_GPIO[1] was set to ‘1’ and the corresponding edge occurred.
        U32 Int_Gpio16           :1;
        //Load error interrupt: 1 if a load error in the scaler occurs and INTE_SC2 is set to 1 
        U32 Int_LoadErr          :1;
        //Configuration error interrupt: 1 if a configuration error in the scaler occurs and	 
        //						INTE_SC1 is set to 1											 
        U32 Int_ConfErr          :1;
        //Trigger error interrupt: 1 if a trigger error in the scaler occurs and INTE_SC0 is set to 1		
        U32 Int_TrigErr          :1;
        //Macrovision mode change: 1 if the macrovision mode in the decoder changes and		 
        //INTE_DEC5 is set to 1											 
        U32 Int_MMC              :1;
        //50/60 Hz change interrupt: 1 if the video mode changes from 50 to 60 Hz or vice versa 
        //						and INTE_DEC3 is set to 1										 
        U32 Int_Fidt             :1;
        //Interlace interrupt: 1 if the video mode changes from interlace to non-interlace or   
        //vice-versa and INTE_DEC3 is set to 1							 
        U32 Int_Intl             :1;
        //Ready for capture interupt: 1 if the decoder status changes and INTE_DEC2 is set to 1 
        U32 Int_RdCap            :1;
        //PWR_ON failure interrupt: if INTE_DEC1 is set to 1 an interrupt will be generated at  
        //the rising edge of the internal power-on reset. If INTE_DEC0 is  
        //set to 1 an interrupt will be generated at the falling edge		 
        U32 Int_PwrOn            :1;
        //Parity error interrupt: 1 if an parity error occurs during any PCI transaction invol- 						
        //ving the SAA 7134 ant INTE_PE is set to 1						 						
        U32 Int_PE               :1;
        //Abort received interrupt: 1 if a PCI transaction initialized by the SAA 7134 finished 
        //with master or target abort and INTE_AR is set to 1				 
        U32 Int_AR               :1;
        //Buffer of Ram area 4 finished: 1 if any of the condition enabled int INTE_RA4 occurs  
        //						and the last DWORD of the corresponding buffer has been transferred					 
        U32 Done_RA4             :1;
        //Buffer of Ram area 3 finished: 1 if any of the condition enabled int INTE_RA3 occurs  
        //and the last DWORD of the corresponding buffer has been transferred				 
        U32 Done_RA3             :1;
        //Buffer of Ram area 2 finished: 1 if any of the condition enabled int INTE_RA2 occurs  						
        //and the last DWORD of the corresponding buffer has been transferred				 						
        U32 Done_RA2             :1;
        //Buffer of Ram area 1 finished: 1 if any of the condition enabled int INTE_RA1 occurs  						
        //and the last DWORD of the corresponding buffer has been transferred				 						
        U32 Done_RA1             :1;
    # else /* !BIG_ENDIAN */
        //Buffer of Ram area 1 finished: 1 if any of the condition enabled int INTE_RA1 occurs  						
        //and the last DWORD of the corresponding buffer has been transferred				 						
        U32 Done_RA1             :1;
        //Buffer of Ram area 2 finished: 1 if any of the condition enabled int INTE_RA2 occurs  						
        //and the last DWORD of the corresponding buffer has been transferred				 						
        U32 Done_RA2             :1;
        //Buffer of Ram area 3 finished: 1 if any of the condition enabled int INTE_RA3 occurs  
        //and the last DWORD of the corresponding buffer has been transferred				 
        U32 Done_RA3             :1;
        //Buffer of Ram area 4 finished: 1 if any of the condition enabled int INTE_RA4 occurs  
        //						and the last DWORD of the corresponding buffer has been transferred					 
        U32 Done_RA4             :1;
        //Abort received interrupt: 1 if a PCI transaction initialized by the SAA 7134 finished 
        //with master or target abort and INTE_AR is set to 1				 
        U32 Int_AR               :1;
        //Parity error interrupt: 1 if an parity error occurs during any PCI transaction invol- 						
        //ving the SAA 7134 ant INTE_PE is set to 1						 						
        U32 Int_PE               :1;
        //PWR_ON failure interrupt: if INTE_DEC1 is set to 1 an interrupt will be generated at  
        //the rising edge of the internal power-on reset. If INTE_DEC0 is  
        //set to 1 an interrupt will be generated at the falling edge		 
        U32 Int_PwrOn            :1;
        //Ready for capture interupt: 1 if the decoder status changes and INTE_DEC2 is set to 1 
        U32 Int_RdCap            :1;
        //Interlace interrupt: 1 if the video mode changes from interlace to non-interlace or   
        //vice-versa and INTE_DEC3 is set to 1							 
        U32 Int_Intl             :1;
        //50/60 Hz change interrupt: 1 if the video mode changes from 50 to 60 Hz or vice versa 
        //						and INTE_DEC3 is set to 1										 
        U32 Int_Fidt             :1;
        //Macrovision mode change: 1 if the macrovision mode in the decoder changes and		 
        //INTE_DEC5 is set to 1											 
        U32 Int_MMC              :1;
        //Trigger error interrupt: 1 if a trigger error in the scaler occurs and INTE_SC0 is set to 1		
        U32 Int_TrigErr          :1;
        //Configuration error interrupt: 1 if a configuration error in the scaler occurs and	 
        //						INTE_SC1 is set to 1											 
        U32 Int_ConfErr          :1;
        //Load error interrupt: 1 if a load error in the scaler occurs and INTE_SC2 is set to 1 
        U32 Int_LoadErr          :1;
        //Edge on GPIO input 14: ‘1’ if INTE_GPIO[0] or/and INTE_GPIO[1] was set to ‘1’ and the corresponding edge occurred.
        U32 Int_Gpio16           :1;
        //Edge on GPIO input 16: ‘1’ if INTE_GPIO[2] or/and INTE_GPIO[3] was set to ‘1’ and the corresponding edge occurred. 
        U32 Int_Gpio18           :1;
        //Edge on GPIO input 22: ‘1’ if INTE_GPIO[4] or/and INTE_GPIO[5] was set to ‘1’ and the corresponding edge occurred.
        U32 Int_Gpio22           :1;
        //Edge on GPIO input 23: ‘1’ if INTE_GPIO[6] or/and INTE_GPIO[7] was set to ‘1’ and the corresponding edge occurred.
        U32 Int_Gpio23           :1;
        U32                      :14;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } IntRegister; 

#define INTSTATUS_OFFS 0xD0

  union _INTSTATUS_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        //Status, i.e. source, task and OE of RA4 in the moment interrupt DONE_RA4 raised
        U32 Sta_RA4              :4;
        // Number of lost interrupts of RAM area 4. Increased when the internal interrupt signal is set and the
        // corresponding interrupt bit is already set. If interrupt bit is set to '0', Li_RA4 will be cleared to 0.
        U32 Li_RA4               :4;
        //Status, i.e. source, task and OE of RA3 in the moment interrupt DONE_RA3 raised
        U32 Sta_RA3              :4;
        // Number of lost interrupts of RAM area 3. Increased when the internal interrupt signal is set and the
        // corresponding interrupt bit is already set. If interrupt bit is set to '0', Li_RA3 will be cleared to 0.
        U32 Li_RA3               :4;
        //Status, i.e. source, task and OE of RA2 in the moment interrupt DONE_RA2 raised
        U32 Sta_RA2              :4;
        // Number of lost interrupts of RAM area 2. Increased when the internal interrupt signal is set and the
        // corresponding interrupt bit is already set. If interrupt bit is set to '0', Li_RA2 will be cleared to 0.
        U32 Li_RA2               :4;
        //Status, i.e. source, task and OE of RA1 in the moment interrupt DONE_RA1 raised	
        U32 Sta_RA1              :4;
        // Number of lost interrupts of RAM area 1. Increased when the internal interrupt signal is set and the
        // corresponding interrupt bit is already set. If interrupt bit is set to '0', Li_RA1 will be cleared to 0.
        U32 Li_RA1               :4;
    # else /* !BIG_ENDIAN */
        // Number of lost interrupts of RAM area 1. Increased when the internal interrupt signal is set and the
        // corresponding interrupt bit is already set. If interrupt bit is set to '0', Li_RA1 will be cleared to 0.
        U32 Li_RA1               :4;
        //Status, i.e. source, task and OE of RA1 in the moment interrupt DONE_RA1 raised	
        U32 Sta_RA1              :4;
        // Number of lost interrupts of RAM area 2. Increased when the internal interrupt signal is set and the
        // corresponding interrupt bit is already set. If interrupt bit is set to '0', Li_RA2 will be cleared to 0.
        U32 Li_RA2               :4;
        //Status, i.e. source, task and OE of RA2 in the moment interrupt DONE_RA2 raised
        U32 Sta_RA2              :4;
        // Number of lost interrupts of RAM area 3. Increased when the internal interrupt signal is set and the
        // corresponding interrupt bit is already set. If interrupt bit is set to '0', Li_RA3 will be cleared to 0.
        U32 Li_RA3               :4;
        //Status, i.e. source, task and OE of RA3 in the moment interrupt DONE_RA3 raised
        U32 Sta_RA3              :4;
        // Number of lost interrupts of RAM area 4. Increased when the internal interrupt signal is set and the
        // corresponding interrupt bit is already set. If interrupt bit is set to '0', Li_RA4 will be cleared to 0.
        U32 Li_RA4               :4;
        //Status, i.e. source, task and OE of RA4 in the moment interrupt DONE_RA4 raised
        U32 Sta_RA4              :4;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } IntStatus; 

} MunitDma;

//--------- End of MunitDma ----------------------
//*************************************************************



  U8 FillerAt0x2D4[0x2C];

#define MUNITSC2DMA_OFFS 0x300

//*************************************************************
//------------- MunitSc2Dma ----------------------
struct _MUNITSC2DMA_ {  
#define CONTROLREG_OFFS 0x0

  union _CONTROLREG_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // Enables dithering, active high
        U32 DitherDB             :1;
        // Select clipping mode for Video, Task A:
        U32 ClipModeDB           :2;
        // Video Output format Task A, Packmodes (YUV422, YUV444, .....)
        U32 PackModeDB           :5;
        // Enables dithering, active high
        U32 DitherVB             :1;
        // Select clipping mode for Video, Task A:
        U32 ClipModeVB           :2;
        // Video Output format Task A, Packmodes (YUV422, YUV444, .....)
        U32 PackModeVB           :5;
        // Enables dithering, active high
        U32 DitherDA             :1;
        // Select clipping mode for Video, Task A:
        U32 ClipModeDA           :2;
        // Video Output format Task A, Packmodes (YUV422, YUV444, .....)
        U32 PackModeDA           :5;
        // Enables dithering, active high
        U32 DitherVA             :1;
        // Select clipping mode for Video, Task A:
        // 00: clipping disabled
        // 01: clip flag from subblock clipping controls byte enables
        // 10: alpha clipping enabled
        // 11: clip flag inserts fixed colour
        U32 ClipModeVA           :2;
        // Video Output format Task A, Packmodes (YUV422, YUV444, .....)
        U32 PackModeVA           :5;
    # else /* !BIG_ENDIAN */
        // Video Output format Task A, Packmodes (YUV422, YUV444, .....)
        U32 PackModeVA           :5;
        // Select clipping mode for Video, Task A:
        // 00: clipping disabled
        // 01: clip flag from subblock clipping controls byte enables
        // 10: alpha clipping enabled
        // 11: clip flag inserts fixed colour
        U32 ClipModeVA           :2;
        // Enables dithering, active high
        U32 DitherVA             :1;
        // Video Output format Task A, Packmodes (YUV422, YUV444, .....)
        U32 PackModeDA           :5;
        // Select clipping mode for Video, Task A:
        U32 ClipModeDA           :2;
        // Enables dithering, active high
        U32 DitherDA             :1;
        // Video Output format Task A, Packmodes (YUV422, YUV444, .....)
        U32 PackModeVB           :5;
        // Select clipping mode for Video, Task A:
        U32 ClipModeVB           :2;
        // Enables dithering, active high
        U32 DitherVB             :1;
        // Video Output format Task A, Packmodes (YUV422, YUV444, .....)
        U32 PackModeDB           :5;
        // Select clipping mode for Video, Task A:
        U32 ClipModeDB           :2;
        // Enables dithering, active high
        U32 DitherDB             :1;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } ControlReg; 

#define CLIPREG_OFFS 0x4

  union _CLIPREG_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :16;
        // Alpha value for Clipping
        U32 AlphaClip            :8;
        // Alpha value vor non clipping
        U32 AlphaNonClip         :8;
    # else /* !BIG_ENDIAN */
        // Alpha value vor non clipping
        U32 AlphaNonClip         :8;
        // Alpha value for Clipping
        U32 AlphaClip            :8;
        U32                      :16;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } ClipReg; 

#define CLIPCOLOR_OFFS 0x8

  union _CLIPCOLOR_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // Clip Colour B portion
        U32 ClipColB             :8;
        // Clip Colour G portion
        U32 ClipColG             :8;
        // Clip Colour R portion
        U32 ClipColR             :8;
        U32                      :6;
        // indicates x position of taken uv sample
        // relevant to formats YUV422planar, YUV420planar and INDEO
        U32 UVPixel              :2;
    # else /* !BIG_ENDIAN */
        // indicates x position of taken uv sample
        // relevant to formats YUV422planar, YUV420planar and INDEO
        U32 UVPixel              :2;
        U32                      :6;
        // Clip Colour R portion
        U32 ClipColR             :8;
        // Clip Colour G portion
        U32 ClipColG             :8;
        // Clip Colour B portion
        U32 ClipColB             :8;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } ClipColor; 

  U8 FillerAt0xC[0x74];

#define CLIPPINGINFO0_OFFS 0x80
  _SCALERCLIPPINGINFO_ ClippingInfo0;
#define CLIPPINGINFO1_OFFS 0x88
  _SCALERCLIPPINGINFO_ ClippingInfo1;
#define CLIPPINGINFO2_OFFS 0x90
  _SCALERCLIPPINGINFO_ ClippingInfo2;
#define CLIPPINGINFO3_OFFS 0x98
  _SCALERCLIPPINGINFO_ ClippingInfo3;
#define CLIPPINGINFO4_OFFS 0xA0
  _SCALERCLIPPINGINFO_ ClippingInfo4;
#define CLIPPINGINFO5_OFFS 0xA8
  _SCALERCLIPPINGINFO_ ClippingInfo5;
#define CLIPPINGINFO6_OFFS 0xB0
  _SCALERCLIPPINGINFO_ ClippingInfo6;
#define CLIPPINGINFO7_OFFS 0xB8
  _SCALERCLIPPINGINFO_ ClippingInfo7;
#define CLIPPINGINFO8_OFFS 0xC0
  _SCALERCLIPPINGINFO_ ClippingInfo8;
#define CLIPPINGINFO9_OFFS 0xC8
  _SCALERCLIPPINGINFO_ ClippingInfo9;
#define CLIPPINGINFO10_OFFS 0xD0
  _SCALERCLIPPINGINFO_ ClippingInfo10;
#define CLIPPINGINFO11_OFFS 0xD8
  _SCALERCLIPPINGINFO_ ClippingInfo11;
#define CLIPPINGINFO12_OFFS 0xE0
  _SCALERCLIPPINGINFO_ ClippingInfo12;
#define CLIPPINGINFO13_OFFS 0xE8
  _SCALERCLIPPINGINFO_ ClippingInfo13;
#define CLIPPINGINFO14_OFFS 0xF0
  _SCALERCLIPPINGINFO_ ClippingInfo14;
#define CLIPPINGINFO15_OFFS 0xF8
  _SCALERCLIPPINGINFO_ ClippingInfo15;

} MunitSc2Dma;

//--------- End of MunitSc2Dma ----------------------
//*************************************************************


