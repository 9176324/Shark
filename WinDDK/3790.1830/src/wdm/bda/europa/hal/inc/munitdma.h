//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!! NOTE : Automatic generated FILE - DON'T CHANGE directly             
//!!        You only may change the file Saa7134_RegDef.h                
//!!        and call GenRegIF.bat for update, check in the necessary     
//!!        files before                                                 
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#ifndef _MUNITDMA_INCLUDE_
#define _MUNITDMA_INCLUDE_


#include "SupportClass.h"


class CMunitDma
{
public:
//constructor-destructor of class CMunitDma
    CMunitDma ( PBYTE );
    virtual ~CMunitDma();


 //Register Description of this class


    CDmaRegSet RegSet0;
    CDmaRegSet RegSet1;
    CDmaRegSet RegSet2;
    CDmaRegSet RegSet3;
    CDmaRegSet RegSet4;
    CDmaRegSet RegSet5;
    CDmaRegSet RegSet6;
    CRegister FifoConfig_1;
    //End of Ram Area 1. Equals begin of RAM area 2. Do not change this value if any      			 
    //channel is active! Make sure that ERA1 is smaller or equal than    
    //ERA2! Note that a write access to this register will clear the RAM 
    //areas RA2 and RA1.												   
    CRegField Era1;   
    //End of Ram Area 2. Equals begin or RAM area 3. Do not change this value if any      			 
    //channel is active! Make sure that ERA2 is greater or equal than    
    //ERA1 and smaller or equal than ERA3! Note that a write access to   
    //this register will clear the RAM areas RA3 and RA2.                
    CRegField Era2;   
    //End of Ram Area 3. Equals begin of RAM area 4. Do not change this value if any	   			 
    //channel is active! Make sure that ERA3 is greater or equal than    
    //ERA2 and smaller or equal than ERA4! Note that a write access to   
    //this register will clear the RAM areas RA4 and RA3				   
    CRegField Era3;   
    //End of Ram Area 4. Hardwired to 8h. Read only	
    CRegField Era4;   
    CRegister FifoConfig_2;
    //Threshold for RAM area 1	
    CRegField Thr1;   
    //Threshold for RAM area 2	
    CRegField Thr2;   
    //Threshold for RAM area 3	
    CRegField Thr3;   
    //Threshold for RAM area 4	
    CRegField Thr4;   
    CRegister MainControl;
    //Transfer enable: Opens the data input to the FIFO if set to one. If set to zero the  			 
    //					FIFO input will be closed and data remaining in the FIFO wil be transfered.	     
    CRegField TE;     
    //Enable basis band DAC and ADC
    CRegField EnBaseDACandADC;
    //Enable SIF frontend	
    CRegField EnSifFE;
    //Enable video frontend 2
    CRegField EnVideoFE2;
    //Enable video frontend 1	
    CRegField EnVideoFE1;
    //Enable oscillator	
    CRegField EnOsc;  
    //Enable audio PLL	
    CRegField EnAudioPll;
    //Enable video PLL
    CRegField EnVideoPll;
    CRegister StatusInfo;
    //DMA abort received. The transfer of a DMA channel finished with either target or		 
    //master abort. This will clear the FIFO and close the FIFO input until
    //the corresponding TE bit has been reset and set again. Read only!	 
    CRegField DmaAr;  
    //MMU abort received. The transfer of the MMU finished with either target or master	 			 
    //abort. This will clear the FIFO and close the FIFO input until the	 
    //corresponding TE bit has been reset and set again. Read only!		 
    CRegField MmuAr;  
    //Odd/Even: Reflects whether the DMA is currently operation in the buffer defined by	 						
    //					BA1 (OE = 1) or BA2 (OE = 0)										 												
    CRegField OddEven;
    //DMA_Done is set after the last data of a field/window has been transferred over the   
    //PCI bus. It is reset with the first transfer of the next field. Note 
    //that DMA_DONE may be a single cycle pulse if the FIFO runs not empty 
    //between two fields													 
    CRegField DmaDone;
    //FIFO emtpy: Set to one if the corresponding RAM area is empty Bit 28 corresponds with area 1.	
    CRegField FE;     
    CDmaRamArea RamArea1;
    CDmaRamArea RamArea2;
    CDmaRamArea RamArea3;
    CDmaRamArea RamArea4;
    CRegister MainStatus;
    //Configuration error occurring in scaler	
    CRegField ConfErr;
    //Trigger error orcurring in scaler	
    CRegField TrigErr;
    //Load error occuring in scaler	
    CRegField LoadErr;
    //Power on reset
    CRegField PwrOn;  
    //Video decoder is ready for capture
    CRegField RdCap;  
    //Decoder detected interlace mode
    CRegField Intl;   
    //Video decoder detected 50 Hz video signal	
    CRegField Fidt;   
    //Macrovision mode
    CRegField MVM;    
    // Equals the IIC main status register
    CRegField Iic_Status;
    //Nicam decoder has locked on NICAM stream
    CRegField Vsdp;   
    //NICAM decoder receives dualmono stream
    CRegField Dsb;    
    //NICAM decoder receives stereo stream
    CRegField Smb;    
    //FM demodulator found pilot
    CRegField Pilot;  
    //FM demodulator receives dualmono stream
    CRegField Dual;   
    //FM demodulator receives stereo stream
    CRegField Stereo; 
    CRegister IntEnable_1;
    //Interrupt enable at the end of field/buffer of RAM area 1	
    CRegField IntE_RA1;
    //Interrupt enable at the end of field/buffer of RAM area 2	
    CRegField IntE_RA2;
    //Interrupt enable at the end of field/buffer of RAM area 3	
    CRegField IntE_RA3;
    //Interrupt enable at the end of field/buffer of RAM area 4	
    CRegField IntE_RA4;
    CRegister IntEnable_2;
    //Enables an interrupt if a transfer using the corresponding register set finished with 
    //master abort. This may be a transfer of the MMU (read) or of the DMA channel itself (write) 
    CRegField IntE_AR;
    //Enables an interrupt if a parity error occurs on the PCI bus	
    CRegField IntE_PE;
    //Enables an interrupt at the PWR_ON signal
    CRegField IntE_Dec_PwrOn;
    //Enables an interrupt at any change of the RDCAP signal
    CRegField IntE_Dec_RdCap;
    //Enables an interrupt at any change of the INTL signal	
    CRegField IntE_Dec_Intl;
    //Enables an interrupt at any change of the FIDT signal
    CRegField IntE_Dec_Fidt;
    //Enables an interrupt at any change of the macrovision mode
    CRegField IntE_Dec_MMC;
    //Enables an interrupt at the positive edge of TRIG_ERR		
    CRegField IntE_Sc_TrigErr;
    //Enables an interrupt at the positive edge of CONF_ERR			
    CRegField IntE_Sc_ConfErr;
    //Enables an interrupt at the positive edge of LOAD_ERR	
    CRegField IntE_Sc_LoadErr;
    //Enables interrupt at the positve edge of GPIO input 16.	
    CRegField IntE_Gpio0;
    //Enables interrupt at the negative edge of GPIO input 16.	
    CRegField IntE_Gpio1;
    //Enables interrupt at the positve edge of GPIO input 18.	
    CRegField IntE_Gpio2;
    //Enables interrupt at the negative edge of GPIO input 18.	
    CRegField IntE_Gpio3;
    //Enables interrupt at the positve edge of GPIO input 22.	
    CRegField IntE_Gpio4;
    //Enables interrupt at the negative edge of GPIO input 22.	
    CRegField IntE_Gpio5;
    //Enables interrupt at the positve edge of GPIO input 23.	
    CRegField IntE_Gpio6;
    //Enables interrupt at the negative edge of GPIO input 23.	
    CRegField IntE_Gpio7;
    CRegister IntRegister;
    //Buffer of Ram area 1 finished: 1 if any of the condition enabled int INTE_RA1 occurs  						
    //and the last DWORD of the corresponding buffer has been transferred				 						
    CRegField Done_RA1;
    //Buffer of Ram area 2 finished: 1 if any of the condition enabled int INTE_RA2 occurs  						
    //and the last DWORD of the corresponding buffer has been transferred				 						
    CRegField Done_RA2;
    //Buffer of Ram area 3 finished: 1 if any of the condition enabled int INTE_RA3 occurs  
    //and the last DWORD of the corresponding buffer has been transferred				 
    CRegField Done_RA3;
    //Buffer of Ram area 4 finished: 1 if any of the condition enabled int INTE_RA4 occurs  
    //						and the last DWORD of the corresponding buffer has been transferred					 
    CRegField Done_RA4;
    //Abort received interrupt: 1 if a PCI transaction initialized by the SAA 7134 finished 
    //with master or target abort and INTE_AR is set to 1				 
    CRegField Int_AR; 
    //Parity error interrupt: 1 if an parity error occurs during any PCI transaction invol- 						
    //ving the SAA 7134 ant INTE_PE is set to 1						 						
    CRegField Int_PE; 
    //PWR_ON failure interrupt: if INTE_DEC1 is set to 1 an interrupt will be generated at  
    //the rising edge of the internal power-on reset. If INTE_DEC0 is  
    //set to 1 an interrupt will be generated at the falling edge		 
    CRegField Int_PwrOn;
    //Ready for capture interupt: 1 if the decoder status changes and INTE_DEC2 is set to 1 
    CRegField Int_RdCap;
    //Interlace interrupt: 1 if the video mode changes from interlace to non-interlace or   
    //vice-versa and INTE_DEC3 is set to 1							 
    CRegField Int_Intl;
    //50/60 Hz change interrupt: 1 if the video mode changes from 50 to 60 Hz or vice versa 
    //						and INTE_DEC3 is set to 1										 
    CRegField Int_Fidt;
    //Macrovision mode change: 1 if the macrovision mode in the decoder changes and		 
    //INTE_DEC5 is set to 1											 
    CRegField Int_MMC;
    //Trigger error interrupt: 1 if a trigger error in the scaler occurs and INTE_SC0 is set to 1		
    CRegField Int_TrigErr;
    //Configuration error interrupt: 1 if a configuration error in the scaler occurs and	 
    //						INTE_SC1 is set to 1											 
    CRegField Int_ConfErr;
    //Load error interrupt: 1 if a load error in the scaler occurs and INTE_SC2 is set to 1 
    CRegField Int_LoadErr;
    //Edge on GPIO input 14: ‘1’ if INTE_GPIO[0] or/and INTE_GPIO[1] was set to ‘1’ and the corresponding edge occurred.
    CRegField Int_Gpio16;
    //Edge on GPIO input 16: ‘1’ if INTE_GPIO[2] or/and INTE_GPIO[3] was set to ‘1’ and the corresponding edge occurred. 
    CRegField Int_Gpio18;
    //Edge on GPIO input 22: ‘1’ if INTE_GPIO[4] or/and INTE_GPIO[5] was set to ‘1’ and the corresponding edge occurred.
    CRegField Int_Gpio22;
    //Edge on GPIO input 23: ‘1’ if INTE_GPIO[6] or/and INTE_GPIO[7] was set to ‘1’ and the corresponding edge occurred.
    CRegField Int_Gpio23;
    CRegister IntStatus;
    // Number of lost interrupts of RAM area 1. Increased when the internal interrupt signal is set and the
    // corresponding interrupt bit is already set. If interrupt bit is set to '0', Li_RA1 will be cleared to 0.
    CRegField Li_RA1; 
    //Status, i.e. source, task and OE of RA1 in the moment interrupt DONE_RA1 raised	
    CRegField Sta_RA1;
    // Number of lost interrupts of RAM area 2. Increased when the internal interrupt signal is set and the
    // corresponding interrupt bit is already set. If interrupt bit is set to '0', Li_RA2 will be cleared to 0.
    CRegField Li_RA2; 
    //Status, i.e. source, task and OE of RA2 in the moment interrupt DONE_RA2 raised
    CRegField Sta_RA2;
    // Number of lost interrupts of RAM area 3. Increased when the internal interrupt signal is set and the
    // corresponding interrupt bit is already set. If interrupt bit is set to '0', Li_RA3 will be cleared to 0.
    CRegField Li_RA3; 
    //Status, i.e. source, task and OE of RA3 in the moment interrupt DONE_RA3 raised
    CRegField Sta_RA3;
    // Number of lost interrupts of RAM area 4. Increased when the internal interrupt signal is set and the
    // corresponding interrupt bit is already set. If interrupt bit is set to '0', Li_RA4 will be cleared to 0.
    CRegField Li_RA4; 
    //Status, i.e. source, task and OE of RA4 in the moment interrupt DONE_RA4 raised
    CRegField Sta_RA4;
  };



#endif _MUNITDMA_
