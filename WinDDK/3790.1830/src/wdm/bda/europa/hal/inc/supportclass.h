//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!! NOTE : Automatic generated FILE - DON'T CHANGE directly             
//!!        You only may change the file Saa7134_RegDef.h                
//!!        and call GenRegIF.bat for update, check in the necessary     
//!!        files before                                                 
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#ifndef _SUPPORTCLASS_INCLUDE_
#define _SUPPORTCLASS_INCLUDE_




#include "Regfield.h"
#include "RegArray.h"




class CPage
{
public:
//constructor-destructor of class CPage
    CPage ( PBYTE, DWORD );
    virtual ~CPage();


 //Register Description of this class


    CRegister BasicSettings;
    // 1: enter page processing only, if field ID is 1
    // 0: don't enter
    CRegField UOdd;   
    // 1: enter page processing only, if field ID is 0
    // 0: don't enter
    CRegField UEven;  
    // 1: enter page processing only, if signal source is 50 Hz
    // 0: don't enter
    CRegField U50;    
    // 1: enter page processing only, if signal source is 60 Hz
    // 0: don't enter
    CRegField U60;    
    // 1: enter page processing only, if signal source is locked
    // 0: don't care the locking info
    CRegField SLock;  
    // Defines the number of fields to be skipped and processed per page
    CRegField FSkip;  
    // 0: RGB to YUV matrix is active
    // 1: RGB to YUV matrix is bypassed
    CRegField MaCon;  
    // 0: YUV to RGB matrix is active
    // 1: YUV to RGB matrix bypassed
    CRegField CsCon;  
    // 0: gamma curve generation disabled
    // 1: gamma curve generation active
    CRegField GaCon;  
    // 0: RGB to YUV matrix output is switched to DMA port of scaler data path
    // 1: gamma curve output is switched to DMA port of scaler data path
    CRegField PcSel;  
    // 0: RGB to YUV matrix output is switched to GP port of scaler data path
    // 1: gamma curve output is switched to GP port of scaler data path
    CRegField GpSel;  
    // 0: vertical phase offset are switched according input field ID
    // 1: vertical phase offset are switched according the field/page processing
    CRegField FidC0;  
    // 00: 4:2:2 YUV data input from decoder
    // 10: raw data input from ADC1 on luma byte, chroma undefined
    // 11: raw data input from ADC2 on luma byte, chroma undefined
    CRegField DRaw;   
    CRegister DataInputWindowHor;
    // input window start at pixel number DxStart
    CRegField DxStart;
    // input window stop at pixel number DyStop
    CRegField DxStop; 
    CRegister DataInputWindowVer;
    // input window start at line number DxStart
    CRegField DyStart;
    // input window stop at pixel number DyStop
    CRegField DyStop; 
    CRegister DataNumberOfOutput;
    // Number of output pixel per line
    CRegField DxDif;  
    // Number of output lines per region
    CRegField DyDif;  
    CRegister VideoInputWindowHor;
    // input window start at pixel number VxStart
    CRegField VxStart;
    // input window stop at pixel number VxStop
    CRegField VxStop; 
    CRegister VideoInputWindowVer;
    // input window start at line number VyStart
    CRegField VyStart;
    // input window stop at line number VyStart
    CRegField VyStop; 
    CRegister VideoNumberOfOutput;
    // Number of output pixel per line
    CRegField VxDif;  
    // Number of output lines per region
    CRegField VyDif;  
    CRegister FirFilterPrescale;
    // Integer prescale, range 1/1/ to 1/63, value 0 not allowed
    CRegField XPSc;   
    // Accumulation sequence length, range 0 to 63, need to be <= XPSC/2
    // real length is 1 to 64
    CRegField XAcL;   
    // DC gain adjustment, gain = 1 / (2^[XDCG-1])
    CRegField XDcG;   
    // Sequence weighting: 
    // 0: 1 + 1 + ... + 1 + 1
    // 1: 1 + 2 + ... + 2 + 1
    CRegField XC21;   
    // Video data FIR prefilter control luminance
    CRegField VPfY;   
    // Video data FIR prefilter control chrominance
    CRegField VPfC;   
    // VBI data FIR prefilter control luminance
    CRegField DPfY;   
    // VBI data FIR prefilter control chrominance
    CRegField DPfC;   
    CRegister ScalerBCS;
    // Luminance Brightness control, nominal value = 128 = 0x80
    CRegField Brightness;
    // Luminance contrast control, nominal value = 64 = 0x40
    CRegField Contrast;
    // Chrominance saturation control, nominal value = 64 = 0x40
    // gain range 1/64 to 127/64
    CRegField Saturation;
    CRegister HorScaleVbi;
    // Scaling increment 1/1 = 1024 = 0x0400	
    // scale range 1024/DXSC = 1024/8191 to about 1024/300
    CRegField DXInc;  
    // Phase shift luminance, range 1/32 to 255/32 of the input pixel distance
    CRegField DXPhShiftY;
    // Phase shift chrominance, should by PhaseShiftLum / 2
    CRegField DXPhShiftC;
    CRegister HorScaleVideo;
    // Scaling increment 1/1 = 1024 = 0x0400	
    // scale range 1024/DXSC = 1024/8191 to about 1024/300
    CRegField VXInc;  
    // Phase shift luminance, range 1/32 to 255/32 of the input pixel distance
    CRegField VXPhShiftY;
    // Phase shift chrominance, should by PhaseShiftLum / 2
    CRegField VXPhShiftC;
    CRegister VerScale;
    // scaling increment, 1/1 = '1024' = 0x0400
    // scale range 1024/YSC = 1024/65535 to about 1024/1
    CRegField YInc;   
    // 0: linear interpolation (LPI) 
    // 1: phase correct accumulation (ACM)
    CRegField YMode;  
    // 1: Mirroring on => horizontal flipping
    CRegField YMir;   
    CRegister VerPhaseOffset;
    // Phase offset 1: range 1/32 to 255/32 of the input pixel distance
    CRegField Offset0;
    // Phase offset 2: range 1/32 to 255/32 of the input pixel distance
    CRegField Offset1;
    // Phase offset 3: range 1/32 to 255/32 of the input pixel distance
    CRegField Offset2;
    // Phase offset 4: range 1/32 to 255/32 of the input pixel distance
    CRegField Offset3;
    // reserved
    CRegArray DummyAt0x38;
  };





class CDmaRegSet
{
public:
//constructor-destructor of class CDmaRegSet
    CDmaRegSet ( PBYTE, DWORD );
    virtual ~CDmaRegSet();


 //Register Description of this class


    //Base Address 1 defines the second start point of the DMA transfer. For data sources 
    //that presents an odd/even signal this base address is used for even fields.         
    CRegister BaseEven;
    //Base Address 2 defines the start point of the DMA transfer. For data sources that   
    //presents an odd/even signal this base address is used for odd fields.               
    CRegister BaseOdd;
    //Pitch defines the distance between the start points of two consecutive lines
    CRegister Pitch;
    CRegister Mmu;
    //base address of the page table						
    CRegField PTAdr;  
    //mapping enable; this bit enables the MMU
    CRegField MapEn;  
    //Maximum burst length of the PCI transfer	
    CRegField Burst;  
    //Specifies whether or not a swap should be executed:	
    //00 no swap												   		
    //01 1234 -> 2143                                                              
    //10 1234 -> 3412											
    //11 1234 -> 4321									   		
    CRegField Swap;   
  };





class CDmaRamArea
{
public:
//constructor-destructor of class CDmaRamArea
    CDmaRamArea ( PBYTE, DWORD );
    virtual ~CDmaRamArea();


 //Register Description of this class


    CRegister RamArea;
    //Task corresponding to VAP	
    CRegField Task;   
    //Source corresponding to VAP
    CRegField Source; 
    //Virtual address pointer of RAM area
    CRegField VAP;    
  };





class CScalerClippingInfo
{
public:
//constructor-destructor of class CScalerClippingInfo
    CScalerClippingInfo ( PBYTE, DWORD );
    virtual ~CScalerClippingInfo();


 //Register Description of this class


    CRegister PixelInfo;
    // Window on/off information; each bit corresponds to one overlay window
    CRegField PixelWin;
    // Pixel (x coordinate) of overlay window corner
    CRegField PixelPos;
    CRegister LineInfo;
    // Window on/off information; each bit corresponds to one overlay window
    CRegField LineWin;
    // Line (y coordinate) of overlay window corner
    CRegField LinePos;
  };



#endif _SUPPORTCLASS_
