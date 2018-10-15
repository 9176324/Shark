//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!! NOTE : Automatic generated FILE - DON'T CHANGE directly             
//!!        You only may change the file Saa7134_RegDef.h                
//!!        and call GenRegIF.bat for update, check in the necessary     
//!!        files before                                                 
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//*************************************************************
//------------- Page ----------------------
struct _PAGE_ {  
#define BASICSETTINGS_OFFS 0x0

  union _BASICSETTINGS_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :8;
        // 00: 4:2:2 YUV data input from decoder
        // 10: raw data input from ADC1 on luma byte, chroma undefined
        // 11: raw data input from ADC2 on luma byte, chroma undefined
        U32 DRaw                 :2;
        // 0: vertical phase offset are switched according input field ID
        // 1: vertical phase offset are switched according the field/page processing
        U32 FidC0                :1;
        // 0: RGB to YUV matrix output is switched to GP port of scaler data path
        // 1: gamma curve output is switched to GP port of scaler data path
        U32 GpSel                :1;
        // 0: RGB to YUV matrix output is switched to DMA port of scaler data path
        // 1: gamma curve output is switched to DMA port of scaler data path
        U32 PcSel                :1;
        // 0: gamma curve generation disabled
        // 1: gamma curve generation active
        U32 GaCon                :1;
        // 0: YUV to RGB matrix is active
        // 1: YUV to RGB matrix bypassed
        U32 CsCon                :1;
        // 0: RGB to YUV matrix is active
        // 1: RGB to YUV matrix is bypassed
        U32 MaCon                :1;
        // Defines the number of fields to be skipped and processed per page
        U32 FSkip                :8;
        U32                      :3;
        // 1: enter page processing only, if signal source is locked
        // 0: don't care the locking info
        U32 SLock                :1;
        // 1: enter page processing only, if signal source is 60 Hz
        // 0: don't enter
        U32 U60                  :1;
        // 1: enter page processing only, if signal source is 50 Hz
        // 0: don't enter
        U32 U50                  :1;
        // 1: enter page processing only, if field ID is 0
        // 0: don't enter
        U32 UEven                :1;
        // 1: enter page processing only, if field ID is 1
        // 0: don't enter
        U32 UOdd                 :1;
    # else /* !BIG_ENDIAN */
        // 1: enter page processing only, if field ID is 1
        // 0: don't enter
        U32 UOdd                 :1;
        // 1: enter page processing only, if field ID is 0
        // 0: don't enter
        U32 UEven                :1;
        // 1: enter page processing only, if signal source is 50 Hz
        // 0: don't enter
        U32 U50                  :1;
        // 1: enter page processing only, if signal source is 60 Hz
        // 0: don't enter
        U32 U60                  :1;
        // 1: enter page processing only, if signal source is locked
        // 0: don't care the locking info
        U32 SLock                :1;
        U32                      :3;
        // Defines the number of fields to be skipped and processed per page
        U32 FSkip                :8;
        // 0: RGB to YUV matrix is active
        // 1: RGB to YUV matrix is bypassed
        U32 MaCon                :1;
        // 0: YUV to RGB matrix is active
        // 1: YUV to RGB matrix bypassed
        U32 CsCon                :1;
        // 0: gamma curve generation disabled
        // 1: gamma curve generation active
        U32 GaCon                :1;
        // 0: RGB to YUV matrix output is switched to DMA port of scaler data path
        // 1: gamma curve output is switched to DMA port of scaler data path
        U32 PcSel                :1;
        // 0: RGB to YUV matrix output is switched to GP port of scaler data path
        // 1: gamma curve output is switched to GP port of scaler data path
        U32 GpSel                :1;
        // 0: vertical phase offset are switched according input field ID
        // 1: vertical phase offset are switched according the field/page processing
        U32 FidC0                :1;
        // 00: 4:2:2 YUV data input from decoder
        // 10: raw data input from ADC1 on luma byte, chroma undefined
        // 11: raw data input from ADC2 on luma byte, chroma undefined
        U32 DRaw                 :2;
        U32                      :8;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } BasicSettings; 

#define DATAINPUTWINDOWHOR_OFFS 0x4

  union _DATAINPUTWINDOWHOR_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :4;
        // input window stop at pixel number DyStop
        U32 DxStop              :12;
        U32                      :4;
        // input window start at pixel number DxStart
        U32 DxStart             :12;
    # else /* !BIG_ENDIAN */
        // input window start at pixel number DxStart
        U32 DxStart             :12;
        U32                      :4;
        // input window stop at pixel number DyStop
        U32 DxStop              :12;
        U32                      :4;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } DataInputWindowHor; 

#define DATAINPUTWINDOWVER_OFFS 0x8

  union _DATAINPUTWINDOWVER_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :4;
        // input window stop at pixel number DyStop
        U32 DyStop              :12;
        U32                      :4;
        // input window start at line number DxStart
        U32 DyStart             :12;
    # else /* !BIG_ENDIAN */
        // input window start at line number DxStart
        U32 DyStart             :12;
        U32                      :4;
        // input window stop at pixel number DyStop
        U32 DyStop              :12;
        U32                      :4;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } DataInputWindowVer; 

#define DATANUMBEROFOUTPUT_OFFS 0xC

  union _DATANUMBEROFOUTPUT_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :4;
        // Number of output lines per region
        U32 DyDif               :12;
        U32                      :4;
        // Number of output pixel per line
        U32 DxDif               :12;
    # else /* !BIG_ENDIAN */
        // Number of output pixel per line
        U32 DxDif               :12;
        U32                      :4;
        // Number of output lines per region
        U32 DyDif               :12;
        U32                      :4;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } DataNumberOfOutput; 

  U8 FillerAt0x10[0x4];

#define VIDEOINPUTWINDOWHOR_OFFS 0x14

  union _VIDEOINPUTWINDOWHOR_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :4;
        // input window stop at pixel number VxStop
        U32 VxStop              :12;
        U32                      :4;
        // input window start at pixel number VxStart
        U32 VxStart             :12;
    # else /* !BIG_ENDIAN */
        // input window start at pixel number VxStart
        U32 VxStart             :12;
        U32                      :4;
        // input window stop at pixel number VxStop
        U32 VxStop              :12;
        U32                      :4;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } VideoInputWindowHor; 

#define VIDEOINPUTWINDOWVER_OFFS 0x18

  union _VIDEOINPUTWINDOWVER_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :4;
        // input window stop at line number VyStart
        U32 VyStop              :12;
        U32                      :4;
        // input window start at line number VyStart
        U32 VyStart             :12;
    # else /* !BIG_ENDIAN */
        // input window start at line number VyStart
        U32 VyStart             :12;
        U32                      :4;
        // input window stop at line number VyStart
        U32 VyStop              :12;
        U32                      :4;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } VideoInputWindowVer; 

#define VIDEONUMBEROFOUTPUT_OFFS 0x1C

  union _VIDEONUMBEROFOUTPUT_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :4;
        // Number of output lines per region
        U32 VyDif               :12;
        U32                      :4;
        // Number of output pixel per line
        U32 VxDif               :12;
    # else /* !BIG_ENDIAN */
        // Number of output pixel per line
        U32 VxDif               :12;
        U32                      :4;
        // Number of output lines per region
        U32 VyDif               :12;
        U32                      :4;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } VideoNumberOfOutput; 

#define FIRFILTERPRESCALE_OFFS 0x20

  union _FIRFILTERPRESCALE_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // VBI data FIR prefilter control chrominance
        U32 DPfC                 :2;
        // VBI data FIR prefilter control luminance
        U32 DPfY                 :2;
        // Video data FIR prefilter control chrominance
        U32 VPfC                 :2;
        // Video data FIR prefilter control luminance
        U32 VPfY                 :2;
        U32                      :4;
        // Sequence weighting: 
        // 0: 1 + 1 + ... + 1 + 1
        // 1: 1 + 2 + ... + 2 + 1
        U32 XC21                 :1;
        // DC gain adjustment, gain = 1 / (2^[XDCG-1])
        U32 XDcG                 :3;
        U32                      :2;
        // Accumulation sequence length, range 0 to 63, need to be <= XPSC/2
        // real length is 1 to 64
        U32 XAcL                 :6;
        U32                      :2;
        // Integer prescale, range 1/1/ to 1/63, value 0 not allowed
        U32 XPSc                 :6;
    # else /* !BIG_ENDIAN */
        // Integer prescale, range 1/1/ to 1/63, value 0 not allowed
        U32 XPSc                 :6;
        U32                      :2;
        // Accumulation sequence length, range 0 to 63, need to be <= XPSC/2
        // real length is 1 to 64
        U32 XAcL                 :6;
        U32                      :2;
        // DC gain adjustment, gain = 1 / (2^[XDCG-1])
        U32 XDcG                 :3;
        // Sequence weighting: 
        // 0: 1 + 1 + ... + 1 + 1
        // 1: 1 + 2 + ... + 2 + 1
        U32 XC21                 :1;
        U32                      :4;
        // Video data FIR prefilter control luminance
        U32 VPfY                 :2;
        // Video data FIR prefilter control chrominance
        U32 VPfC                 :2;
        // VBI data FIR prefilter control luminance
        U32 DPfY                 :2;
        // VBI data FIR prefilter control chrominance
        U32 DPfC                 :2;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } FirFilterPrescale; 

#define SCALERBCS_OFFS 0x24

  union _SCALERBCS_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :8;
        // Chrominance saturation control, nominal value = 64 = 0x40
        // gain range 1/64 to 127/64
        U32 Saturation           :8;
        // Luminance contrast control, nominal value = 64 = 0x40
        U32 Contrast             :8;
        // Luminance Brightness control, nominal value = 128 = 0x80
        U32 Brightness           :8;
    # else /* !BIG_ENDIAN */
        // Luminance Brightness control, nominal value = 128 = 0x80
        U32 Brightness           :8;
        // Luminance contrast control, nominal value = 64 = 0x40
        U32 Contrast             :8;
        // Chrominance saturation control, nominal value = 64 = 0x40
        // gain range 1/64 to 127/64
        U32 Saturation           :8;
        U32                      :8;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } ScalerBCS; 

#define HORSCALEVBI_OFFS 0x28

  union _HORSCALEVBI_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // Phase shift chrominance, should by PhaseShiftLum / 2
        U32 DXPhShiftC           :8;
        // Phase shift luminance, range 1/32 to 255/32 of the input pixel distance
        U32 DXPhShiftY           :8;
        U32                      :3;
        // Scaling increment 1/1 = 1024 = 0x0400	
        // scale range 1024/DXSC = 1024/8191 to about 1024/300
        U32 DXInc               :13;
    # else /* !BIG_ENDIAN */
        // Scaling increment 1/1 = 1024 = 0x0400	
        // scale range 1024/DXSC = 1024/8191 to about 1024/300
        U32 DXInc               :13;
        U32                      :3;
        // Phase shift luminance, range 1/32 to 255/32 of the input pixel distance
        U32 DXPhShiftY           :8;
        // Phase shift chrominance, should by PhaseShiftLum / 2
        U32 DXPhShiftC           :8;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } HorScaleVbi; 

#define HORSCALEVIDEO_OFFS 0x2C

  union _HORSCALEVIDEO_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // Phase shift chrominance, should by PhaseShiftLum / 2
        U32 VXPhShiftC           :8;
        // Phase shift luminance, range 1/32 to 255/32 of the input pixel distance
        U32 VXPhShiftY           :8;
        U32                      :3;
        // Scaling increment 1/1 = 1024 = 0x0400	
        // scale range 1024/DXSC = 1024/8191 to about 1024/300
        U32 VXInc               :13;
    # else /* !BIG_ENDIAN */
        // Scaling increment 1/1 = 1024 = 0x0400	
        // scale range 1024/DXSC = 1024/8191 to about 1024/300
        U32 VXInc               :13;
        U32                      :3;
        // Phase shift luminance, range 1/32 to 255/32 of the input pixel distance
        U32 VXPhShiftY           :8;
        // Phase shift chrominance, should by PhaseShiftLum / 2
        U32 VXPhShiftC           :8;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } HorScaleVideo; 

#define VERSCALE_OFFS 0x30

  union _VERSCALE_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :14;
        // 1: Mirroring on => horizontal flipping
        U32 YMir                 :1;
        // 0: linear interpolation (LPI) 
        // 1: phase correct accumulation (ACM)
        U32 YMode                :1;
        // scaling increment, 1/1 = '1024' = 0x0400
        // scale range 1024/YSC = 1024/65535 to about 1024/1
        U32 YInc                :16;
    # else /* !BIG_ENDIAN */
        // scaling increment, 1/1 = '1024' = 0x0400
        // scale range 1024/YSC = 1024/65535 to about 1024/1
        U32 YInc                :16;
        // 0: linear interpolation (LPI) 
        // 1: phase correct accumulation (ACM)
        U32 YMode                :1;
        // 1: Mirroring on => horizontal flipping
        U32 YMir                 :1;
        U32                      :14;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } VerScale; 

#define VERPHASEOFFSET_OFFS 0x34

  union _VERPHASEOFFSET_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        // Phase offset 4: range 1/32 to 255/32 of the input pixel distance
        U32 Offset3              :8;
        // Phase offset 3: range 1/32 to 255/32 of the input pixel distance
        U32 Offset2              :8;
        // Phase offset 2: range 1/32 to 255/32 of the input pixel distance
        U32 Offset1              :8;
        // Phase offset 1: range 1/32 to 255/32 of the input pixel distance
        U32 Offset0              :8;
    # else /* !BIG_ENDIAN */
        // Phase offset 1: range 1/32 to 255/32 of the input pixel distance
        U32 Offset0              :8;
        // Phase offset 2: range 1/32 to 255/32 of the input pixel distance
        U32 Offset1              :8;
        // Phase offset 3: range 1/32 to 255/32 of the input pixel distance
        U32 Offset2              :8;
        // Phase offset 4: range 1/32 to 255/32 of the input pixel distance
        U32 Offset3              :8;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } VerPhaseOffset; 

#define DUMMYAT0X38_OFFS 0x38

// reserved
U8 DummyAt0x38[3];

};

//--------- End of Page ----------------------
//*************************************************************


//*************************************************************
//------------- DmaRegSet ----------------------
struct _DMAREGSET_ {  
#define BASEEVEN_OFFS 0x0

//Base Address 1 defines the second start point of the DMA transfer. For data sources 
//that presents an odd/even signal this base address is used for even fields.         
U32 BaseEven;

#define BASEODD_OFFS 0x4

//Base Address 2 defines the start point of the DMA transfer. For data sources that   
//presents an odd/even signal this base address is used for odd fields.               
U32 BaseOdd;

#define PITCH_OFFS 0x8

//Pitch defines the distance between the start points of two consecutive lines
U32 Pitch;

#define MMU_OFFS 0xC

  union _MMU_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :6;
        //Specifies whether or not a swap should be executed:	
        //00 no swap												   		
        //01 1234 -> 2143                                                              
        //10 1234 -> 3412											
        //11 1234 -> 4321									   		
        U32 Swap                 :2;
        //Maximum burst length of the PCI transfer	
        U32 Burst                :3;
        //mapping enable; this bit enables the MMU
        U32 MapEn                :1;
        //base address of the page table						
        U32 PTAdr               :20;
    # else /* !BIG_ENDIAN */
        //base address of the page table						
        U32 PTAdr               :20;
        //mapping enable; this bit enables the MMU
        U32 MapEn                :1;
        //Maximum burst length of the PCI transfer	
        U32 Burst                :3;
        //Specifies whether or not a swap should be executed:	
        //00 no swap												   		
        //01 1234 -> 2143                                                              
        //10 1234 -> 3412											
        //11 1234 -> 4321									   		
        U32 Swap                 :2;
        U32                      :6;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } Mmu; 

};

//--------- End of DmaRegSet ----------------------
//*************************************************************


//*************************************************************
//------------- DmaRamArea ----------------------
struct _DMARAMAREA_ {  
#define RAMAREA_OFFS 0x0

  union _RAMAREA_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        //Virtual address pointer of RAM area
        U32 VAP                 :30;
        //Source corresponding to VAP
        U32 Source               :1;
        //Task corresponding to VAP	
        U32 Task                 :1;
    # else /* !BIG_ENDIAN */
        //Task corresponding to VAP	
        U32 Task                 :1;
        //Source corresponding to VAP
        U32 Source               :1;
        //Virtual address pointer of RAM area
        U32 VAP                 :30;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } RamArea; 

};

//--------- End of DmaRamArea ----------------------
//*************************************************************


//*************************************************************
//------------- ScalerClippingInfo ----------------------
struct _SCALERCLIPPINGINFO_ {  
#define PIXELINFO_OFFS 0x0

  union _PIXELINFO_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :4;
        // Pixel (x coordinate) of overlay window corner
        U32 PixelPos            :12;
        U32                      :8;
        // Window on/off information; each bit corresponds to one overlay window
        U32 PixelWin             :8;
    # else /* !BIG_ENDIAN */
        // Window on/off information; each bit corresponds to one overlay window
        U32 PixelWin             :8;
        U32                      :8;
        // Pixel (x coordinate) of overlay window corner
        U32 PixelPos            :12;
        U32                      :4;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } PixelInfo; 

#define LINEINFO_OFFS 0x4

  union _LINEINFO_ {
    struct _ITEM_ {
      # ifdef BIG_ENDIAN
        U32                      :4;
        // Line (y coordinate) of overlay window corner
        U32 LinePos             :12;
        U32                      :8;
        // Window on/off information; each bit corresponds to one overlay window
        U32 LineWin              :8;
    # else /* !BIG_ENDIAN */
        // Window on/off information; each bit corresponds to one overlay window
        U32 LineWin              :8;
        U32                      :8;
        // Line (y coordinate) of overlay window corner
        U32 LinePos             :12;
        U32                      :4;
      # endif /* !BIG_ENDIAN */
    } Item;
    U32 Reg;
  } LineInfo; 

};

//--------- End of ScalerClippingInfo ----------------------
//*************************************************************

