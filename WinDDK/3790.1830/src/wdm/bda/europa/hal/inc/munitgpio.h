//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!! NOTE : Automatic generated FILE - DON'T CHANGE directly             
//!!        You only may change the file Saa7134_RegDef.h                
//!!        and call GenRegIF.bat for update, check in the necessary     
//!!        files before                                                 
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#ifndef _MUNITGPIO_INCLUDE_
#define _MUNITGPIO_INCLUDE_


#include "SupportClass.h"


class CMunitGpio
{
public:
//constructor-destructor of class CMunitGpio
    CMunitGpio ( PBYTE );
    virtual ~CMunitGpio();


 //Register Description of this class


    CRegister IicControl;
    // Status information of IIC interface,
    // see Register IF block documentation for details
    CRegField Iic_Status;
    // Transfer attribute associated with this command
    // 3 : START 
    // 2 : CONTINUE
    // 1 : STOP
    // 0 : NOP
    CRegField Iic_Attr;
    // Bytes to transfer: read or write on IIC bus according LSB
    // of most recent START-BYTE
    CRegField Iic_ByteReadWrite;
    // Combination of the registers
    // Iic_Status & Iic_Attr & and Iic_ByteReadWrite
    CRegField Iic_Transfer;
    // Nominal IIc clock rate
    //0: slow (below) 100 kHz, 33.3 MHz/400
    //1: fast (below) 400 kHz, 33.3 MHz/100
    CRegField Iic_ClockSel;
    // Timeout parameter, applicable for various sequential conditions to check:
    // bus arbitration, SCL stretching and DONE status echoing 
    // Timeout = IIC clock period * 2^(Timer)							
    CRegField Iic_Timer;
    CRegister VideoOutput_1;
    // ITU/VIP code : V-Bit polarity
    // 0: not inverted V-Bit
    // 1: inverted V-Bit
    CRegField Vp_VCodeP;
    // ITU/VIP code : V-Bit content
    // 00: V_ITU(VBLNK_CCIR_L) from decode (default)
    // 01: indicates VBI data @ scaler outp (SC_VBI_ACT)
    // 10: indicates act video data @ scaler output (SC_VGATE & !VBI_flag)
    CRegField Vp_VCode;
    // ITU/VIP code: F-Bit polarity
    // 0: non inverted F-Bit
    // 1: inverted F-Bit
    CRegField Vp_FCodeP;
    // ITU/VIP code: F-Bit content
    // 00: F_ITU from decoder (default)
    // 01: Scaler page: A->F = 1, B->F = 0
    // 10: F = SC_out_FID (Decoder dependant, sc_fid)
    // 11: F = SC_reg_FID (Scaler dpendant, sc_page)
    CRegField Vp_FCode;
    // VIP code: T-Bit polarity
    // 0: non inverted V-Bit
    // 1: inverted V-Bit
    CRegField Vp_TCodeP;
    // VIP code: T-Bit content
    // 00: V_ITU (VBLNK_CCIR_L) from decoder (default)
    // 01: indicates VBI data @ scaler output (SC_VBI_ACT)
    // 10: indicates act video data @ scaler output (SC_VGATE & !VBI_flag)
    // 11: VGATE_L from decoder
    CRegField Vp_TCode;
    // Keep last video data set stable @ the video output until new valid data is
    // send. Required for e.g. VMI or DMSD Video formats
    // 0: non qualified video data samples are carrying blanking values during
    // horiz. active video
    // 1: non qualified video data samples are carrying the same value as the
    // last qualified video data sample during horiz. active video	
    CRegField Vp_Keep;
    // 0: blanking values: luma = 00 hex, chroma = 00 hex
    // 1: blanking values: luma - 10 hex, chroma = 80 hex
    CRegField Vp_Blank8010;
    // Generate Hamming code bits for ITU/VIP (T-), V-, F-, and H bits, if set to '1'
    CRegField Vp_GenHamm;
    // Insert ITU/VIP codes on port C, if set to '1'
    CRegField Vp_EnCodeC;
    // Insert ITU/VIP codes on port A, if set to '1'
    CRegField Vp_EnCodeA;
    // Video decoder (i.e. DMSD) source stream selector
    // Depending on the value of this 2 bit register, the V_ITU vertical reference
    // signal and the programmable VGATE_L (programmed in the DMSD
    // munit) the DMSD will output comb-filtered YUV data, straight (non
    // comb-filtered) YUV data or raw (non decoded) CVBS data.(cf. table 5).
    // NOTE: This effects the input of the Scaler as well as the GPIO video port!
    CRegField Vp_DecRawVbi;
    // Enable GPIO video out according to decoder lock status:
    // 0: output video data from sources "as is"
    // 1: video decoder lock status locked => output video data " as is"
    //    video decoder lock status NOT locked => output blanking values
    //    (as specified by VpBlank8010) and remove enable sync signals (tie to '0')
    CRegField Vp_EnLock;
    // 1: enables routing of decoder output to video output port, if no other
    // stream is enabled and gated 
    CRegField Vp_EnDmsd;
    // VGATE_L
    CRegField Vp_EnXv;
    CRegField Vp_EnScAct;
    CRegField Vp_EnScVbi;
    // Swap chroma UV sequence at GPIO video output port
    // 8 bit video modes at port A or C:
    //     -> 0: Sequence: U-Y-V-Y ....
    //     -> 1: Sequence: Y-U-Y-V ....
    // 16 bit video modes:
    //	   -> 0: Sequence: Y-Y .... (at port A); U-V .... (at port C)
    //     -> 1: Sequence: U-V .... (at port A); Y-Y .... (at port C)
    CRegField Vp_UvSwap;
    // Enable GPIO Video output port
    // 00: GPIO video port disabled(usable for other purposes)
    // 01:  8 bit video output mode on port A(port C usable for other purposes)
    // 10:  8 bit video output mode on port C(port A usable for other purposes)
    // 11: 16 bit video output mode on port A(luma) and C(chroma)
    CRegField Vp_EnVideo;
    // 1: reserve necessary GPIO pins
    CRegField Vp_EnPort;
    CRegister VideoOutput_2;
    // horiz. Sync. Squeeze counter
    // This counter defines the amount of time, the scaler video output is stalled
    // prior to receive data from the scaler. The time is measured in number
    // scaler backend clock cycles beginning with a horizontal sync. event (on
    // internal sc_hagte signal??????).
    // This is used to obtain a continous video data stream at the video output.
    CRegField Vp_SqeeCnt;
    // vp_clk_ctrl: Video output clock control
    // -> 0xxx: Use NON gated clock
    // -> 1xxx: Use gated clock
    // -> x0xx: clock is not delayed
    // -> x1xx: clock is delayed
    // -> xx0x: Don’t invert the clock
    // -> xx1x: invert the clock
    // -> xxx0: Video clock based on ’clk0’ (@ 27 MHz)
    // -> xxx1: Video clock based on ’clk1’ (@13,5 MHz)
    CRegField Vp_ClkCtrl;
    // Switch the PICLK OFF
    // -> 0: The PICLK (@GPIO24) is active, i.e. switched on (default)
    // -> 1: The PICLK (@GPIO24) is switched off (i.e. the pin is in tristate mode)
    CRegField Vp_PiClkOff;
    // Video vertical sync type at the video output port:
    // 000: OFF     : no V-sync signal; pin usable for other (GPIO) purposes
    // 001: V123    : 'analog' V-sync with half line timing from decoder (default)
    // 010: V_ITU	: BLNK_CCIR_L from decoder (used for ITU)
    // 011: VGATE_L : Programmable vertical gate signal from decoder
    // 100: reserved, do not use!!!!!!!!
    // 101: reserved, do not use!!!!!!!!
    // 110: F_ITU   : Field ID according ITU from decoder
    // 111: SC_FID  : Field ID as generated by scaler output
    CRegField Vp_VsTyp;
    // Video vertical sync polarity at the video output port:
    // 0: NON inverted video clock (default)
    // 1: inverted video clock
    CRegField Vp_VsP; 
    // Video horizontal sync type at the video output port:
    // 00: OFF: no H-Sync signal; pin usable for other (GPIO) purposes
    // 01: Combi-HREF (default)
    // 10: Plain HREF from decoder
    // 11: HS: Programmable horizontal sync from decoder
    CRegField Vp_HsTyp;
    // Video horizontal sync polarity at the video output port:
    // 0: NON inverted video clock (default)
    // 1: inverted video clock
    CRegField Vp_HsP; 
    // Select the 2nd GPIO video auxiliary (input) port function:
    // The V_AUX2 input:
    // 00: won’t be evaluated (OFF).
    // 01: is the external clock input for ADC Modes (VSB sidecar,
    // no meaning for the video port)
    // 10: forces all video output pins (incl. V_CLK) to be tri-stated if V_AUX2
    // (i.e. GPIO 18) set to ’1’ (latency is minimum 4 v_CLK clock cycles)
    // 11: is not defined (i.e. won’t be evaluated, => OFF)
    CRegField Vp_Aux2Fct;
    // Enable the 1st GPIO video auxiliary port and select signal type:
    // 000: OFF		: no signal routed, pin usable for other (GPIO) purposes
    // 001: Combi-HGATE: indicates any active line in enabled regions
    // 010: CREF		: for DMSD-2 style interfaceing
    // 011: CombiVGATE : indicates any enabled active region
    // 100: inverted Combi/HGATE
    // 101: inverted CREF
    // 110: inverted Combi-VGATE
    // 111: RTCO		: Real Time control output to lock external DENC's
    CRegField Vp_EnAux1;
    CRegister AdcConverterOutput;
    // Enable VSB_B output port
    // 0: No raw ADB data output (default)
    // 1: Mapped to GPIO {7 - 0, 23} (MSB to LSB). Direct output of Video
    // ADC data to VSB_B port of GPIO
    CRegField VsbB_En;
    // Enable VSB_A output port
    // 0: No raw ADB data output (default)
    // 1: Mapped to GPIO {15 - 8, 17} (MSB to LSB). Direct output of Video
    // ADC data to VSB_A port of GPIO
    CRegField VsbA_En;
    // SwaP VSB data sources
    // 0: (default) VSB_A data source is video ADC A, VSB_B data source is
    // video ADC B
    // 1: VSB_A data source is video ADC B and VSB_B data source is video
    // ADC A
    CRegField SwapVsbSrc;
    CRegister TStreamInterface_1;
    // Parallel Interface: Force LOCK status
    // 1: force lock status, i.e. ignore GiTsLock (default)
    // 0: evaluate GiTsLock
    CRegField Ts_PlockF;
    // Parallel Interface: Transport stream LOCK detection Polarity
    // 1: high active (default)
    // 0: low active
    CRegField Ts_Plockp;
    // Parallel Interface: Force the VALid
    // If set to '1' - this forces any data ta the data input to accepted as valid
    // (valid signal line will be ignored). This allows at the same time to use the 
    // external pin again for other purposes
    CRegField Ts_PvalF;
    // Parallel Interface: Transport stream VALid signal Polarity
    // 1: high active (default)
    // 0: low active
    CRegField Ts_Pvalp;
    // Parallel Interface: Transport Stream Start of packet Polarity
    // 1: high active (default)
    // 0: low active
    CRegField Ts_Psopp;
    // Parallel Interface: Enable Parallel Transport Stream Interface (higher priority than TsSEn)
    CRegField Ts_PEn; 
    // Parallel Transport Stream, Number of expected bytes per packet -1
    CRegField Ts_Pbpp;
    // Serial Transport Stream, SHIFT serial data for one clock cyle:
    // 0: Fist databit is received with SOP edge (default).
    // 1: Fist databit is received one clock cycle after SOP edge. (only
    // needed for serial audio in)
    CRegField Ts_Shift;
    // Serial Transport Stream: flip MSB’s and LSB’s:
    // 0: transfer byte to DMA as received (default).
    // 1: flip MSB’s and LSB’s bit by bit within a byte and then transfer to
    // DMA.
    CRegField Ts_Bitflip;
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
    CRegField Ts_EopTe;
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
    CRegField Ts_EopLe;
    // Serial Transport Stream, EValuate Trailing EDGe of SOP
    // 1: trailing edge of SOP evaluated
    // 0: trailing edge of SOP NOT considered (default)
    // Will be enabled for receiving IIS data (SOP than will be handeled as IIS
    // WS signal)
    // The trailing edge corresponds to the inverted “ts_ssopp” bit.
    CRegField Ts_EvTedg;
    // Serial Transport Stream, EValuate Leading EDGe of SOP
    // 1: leading edge of SOP wil be evaluated (default)
    // After enabling the Serial Transport Stream Interface the first packet or
    // audio sample received is always synchronized to the leading edge of the
    // SOP signal als long as ‘ts_ev_ledg’ is set to ‘1’ at that time.
    // 0: leading edge of SOP NOT considered
    // The polarity of the leading edge is defined by the “ts_ssopp” bit.
    CRegField Ts_EvLedg;
    // Serial Interface: serial input Mode of TS interface is enabled if set to '1'
    CRegField Ts_SEn; 
    // High active soft reset signal for GPIO TS Interface required (needed,
    // since a clock for the TS If. may not be avail during regular HW reset)
    // 0: Softreset inactive
    // 1: Softreset active (default after reset, since
    CRegField Ts_SoftRes;
    // Use the inverted transport stream clock, which accompanies the
    // incoming data at transport stream interface:
    // 0: Receive the data using the transprot stream clock as is.
    // 1: Receive the data using the inverted transprot stream clock.
    CRegField Ts_InvClk;
    CRegister TStreamInterface_2;
    // Number of packets to be transferred into a host buffer.
    // Value to be programmed is: number_of_packets - 1
    // (default: 22’h137 = 22’d311, i.e.: 312 packets)
    CRegField Ts_Nop; 
    // Collapse packets (used for audio mode to fit two "packets" (i.e. two 16 bit
    // audio samples) into one DWord in the DMA block
    // 0: (default) each end of packet generated by the Transport Stream
    // interface will cause that writing the next data will start in a new DWord in
    // the GPIO2DMA block
    // 1: GPIO2DMA Will ignore the EOP in that the next data is put into the
    // same Dword.
    // The use of this bit makes only sense if NO_PITCH is enabled too.
    CRegField Ts_Collapse;
    // Disable adding line pitch (for audio mode)
    // 0: (default) Line Pitch will be added after EOP from GPIO Transport
    // stream If.
    // 1: Disable adding the line pitch after EOP
    CRegField Ts_NoPitch;
    CRegister GpioPortModeSel;
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
    CRegField Gp_Mode;
    // Rescan GPI values (on request of the user)
    // Rescan of GPI Values will only occur if this 
    // programming register is set from zero to one (i.e. only a 
    // rising edge will lead to an update GpStst register.
    // Rescan only effects those GpStat values, which are defined as 
    // general purpose inputs in the GpMode register (corresponding bit = 0)
    CRegField Gp_Rescan;
    CRegister GpioPortStatus;
    // Static GPIO Port Status Register (read & write)
    // (refer to Static GPIO Port register host mode enable register ‘gp_mode’)
    CRegField Gp_Stat;
    CRegister AudioOutSel;
    // GPIO Audio OUTput ENable:
    // 0: GPIO audio output is disabled (default)
    // 1: GPIO audio output is enabled
    // The audio data source corresponds to the GaOutSel register setting
    CRegField Ga_OutEn;
    CRegister TestMode;
    // Force PRESET (=GPIO28) (only AOUT functionon the same pin must be
    // disabled)
    // -> ’0’: (default) the reset @ GPIO28 is active
    // -> ’1’: the reset @ GPIO28 forced to inactive
    // NOTE: PRESET reset signal must be deactivated by SW in any case.
    CRegField FPreset;
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
    CRegField Tst_Mode;
    // Enable signal for analog AOUT function on ‘PRESET ‘ (GPIO 28) output
    // pin.
    // -> 0: The ‘PRESET’ pin carries the peripheral RESET function (default).
    // -> 1: The ‘PRESET’ pin carries the analog output test signals as defined
    // by ‘tst_mod’ bits.
    // In case of a reset AOUT will be disabled. Default value is ’0’, i.e. Reset
    // function on GPIO28 (= PRESET)
    CRegField Tst_EnAOut;
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
    CRegField Tst_AOutSel;
  };



#endif _MUNITGPIO_
