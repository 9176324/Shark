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
// @doc      INTERNAL
// @module   VampGPIO | This file contains the GPIO class. This class is
//           responsible for the enabling and disabling of the GPIO pins.
//           The I2C pins are ignored in this class because they have a
//           special behavior. This class has a set of functions for each part
//           of GPIO functionality. These are Video Out, VSB, Transport
//           stream, Audio stream and static GPIO.
// @end
// Filename: GPIO.h
//
// Routines/Functions:
//
//  public:
//      CVampGPIO
//      ~CVampGPIO
//      Reset
//      GetPinsConfiguration
//      EnableITU
//      EnableVIP11
//      EnableVIP20
//      EnableAudio
//      EnableTS
//      Set/GetTS_BytesPerPacket
//      Set/GetTS_NumOfPackets
//      ResetTS
//      EnableVSB_A
//      EnableVSB_B
//      EnableVSB_AB
//      WriteToPinNr
//      WriteToGPIONr
//      ReadFromPinNr
//      ReadFromGPIONr
//      GetObjectStatus
//
//  private:
//      SetTS_SOPPolarity
//      SetTS_VAL
//      InvertTS_Clock
//      DisableTS_LinePitch
//      CollapseTS_Packets
//      EvaluateTS_Edge
//      BitflipTS
//      ShiftDataTS
//      SwapVSB_SRC
//      ReadTSRegistry
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////
#ifndef GPIO__
#define GPIO__

#include "VampDeviceResources.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class CVampGPIO | This class just enables or disables the GPIO pins and
//        checks if the pin is already in use. This class is used by the
//        CVampTransportStream, CVampAudioDec, CVampVideoStream and
//        CVampGPIOStatic class.
// @end
//
//////////////////////////////////////////////////////////////////////////////
class CVampGPIO 
{
    // @access private 
private:

// Member variables
    // @cmember This array contains EEPROM/bootstrap information for each GPIO
    //          pin.<nl>
    tGPIOPinInfo    m_PIN_INFO[MAX_GPIO_PINS];

    // @cmember Pointer to the Device Resource object.<nl>
    CVampDeviceResources* m_pVampDevRes;

    // @cmember This variable stores the EEPROM/Bootstrap information of the
    //          pins in one variable.<nl>
    DWORD m_dwPinInfo;

    // @cmember This parameter stores the Bytes Per Packets of the transport
    //          stream.<nl>
    BYTE  m_ucBPP;

    // @cmember This parameter stores the Number Of Packets of the transport
    //          stream.<nl>
    DWORD m_dwNOP;

    // @cmember This parameter stores the Registry value "SwapSource" for the
    //          VSB part of the GPIO.<nl>
    BOOL m_bSwap;

    // @cmember This parameter stores the Registry value "SOP_Polarity" for
    //          the TS part of the GPIO.<nl>
    BOOL m_bSOPPolarity;

    // @cmember This parameter stores the Registry value "ForceVAL" for the TS
    //          part of the GPIO.<nl>
    BOOL m_bForceVAL;

    // @cmember This parameter stores the Registry value "VAL_Polarity" for
    //          the TS part of the GPIO.<nl>
    BOOL m_bVALPolarity;

    // @cmember This parameter stores the Registry value "Clock_Polarity" for
    //          the TS part of the GPIO.<nl>
    BOOL m_bInvertClock;

    // @cmember This parameter stores the Registry value "LinePitch" for the
    //          TS part of the GPIO.<nl>
    BOOL m_bLinePitch;

    // @cmember This parameter stores the Registry value "CollapsePackets" for
    //          the TS part of the GPIO.<nl>
    BOOL m_bCollapse;

    // @cmember This parameter stores the Registry value "ForceLEdge" for the
    //          TS part of the GPIO.<nl>
    BOOL m_bLEdge;

    // @cmember This parameter stores the Registry value "ForceTEdge" for the
    //          TS part of the GPIO.<nl>
    BOOL m_bTEdge;

    // @cmember This parameter stores the Registry value "GenerateLEOP" for
    //          the TS part of the GPIO.<nl>
    BOOL m_bGenerateL;

    // @cmember This parameter stores the Registry value "GenerateTEOP" for
    //          the TS part of the GPIO.<nl>
    BOOL m_bGenerateT;

    // @cmember This parameter stores the Registry value "Bitflip" for the TS
    //          part of the GPIO.<nl>
    BOOL m_bBitflip;

    // @cmember This parameter stores the Registry value "ShiftData" for the
    //          TS part of the GPIO.<nl>
    BOOL m_bShiftData;

	// @cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

	// @cmember This variable indicates that the GPIO pins are blocked from 
	//          the EnableITU function.
	BOOL  m_bITU;
	
	// @cmember This variable indicates that the GPIO pins are blocked from 
	//          the EnableVIP11 function.
	BOOL  m_bVIP11;
	
	// @cmember This variable indicates that the GPIO pins are blocked from 
	//          the EnableVIP20 function.
	BOOL  m_bVIP20;
	
	// @cmember This variable indicates that the GPIO pins are blocked from 
	//          the EnableAudio function.
	BOOL  m_bAudio;
	
	// @cmember This variable indicates that the GPIO pins are blocked from 
	//          the EnableTS(Parallel) function.
	BOOL  m_bTS_P;
	
	// @cmember This variable indicates that the GPIO pins are blocked from 
	//          the EnableTS(Serial) function.
	BOOL  m_bTS_S;
	
	// @cmember This variable indicates that the GPIO pins are blocked from 
	//          the EnableVSB_A function.
	BOOL  m_bVSB_A;
	
	// @cmember This variable indicates that the GPIO pins are blocked from 
	//          the EnableVSB_B function.
	BOOL  m_bVSB_B;
	
	// @cmember This variable indicates that the GPIO pins are blocked from 
	//          the EnableVSB_AB function.
	BOOL  m_bVSB_AB;

// Methods:
    // @cmember This function converts the bootstrap information into the
    //          detailed m_dwPinInfo variable.<nl>    
    // Parameterlist:<nl>
    // DWORD dwBootstrap // bootstrap info<nl>
	VOID ConvertBootstrap(
        DWORD dwBootstrap );

// Transport Stream
    // @cmember This function sets the polarity of the Start of Packets. The
    //          polarity of the SOP signal is a candidate for the
    //          registry.<nl>
    // Parameterlist:<nl>
    // BOOL bInvert // invert flag<nl>
    VAMPRET SetTS_SOPPolarity(
        BOOL bInvert );

    // @cmember This function sets the polarity of the valid signal and if
    //          this signal has to be forced. The polarity of the VAL signal
    //          is a candidate for the registry.<nl>
    // Parameterlist:<nl>
    // BOOL bForce // force flag<nl>
    // BOOL bInvert // invert flag<nl>
    VAMPRET SetTS_VAL(
        BOOL bForce, 
        BOOL bInvert );

    // @cmember Invert the clock. The polarity of the Clock signal is a
    //          candidate for the registry.<nl>
    // Parameterlist:<nl>
    // BOOL bInvert // invert flag<nl>
    VAMPRET InvertTS_Clock(
        BOOL bInvert );

    // The following functions make sense only for TS_Serial
    // @cmember Disable the adding of line pitch. This function is used for
    //          audio data. Disabling the line pitch is a candidate for the
    //          registry.<nl>
    // Parameterlist:<nl>
    // BOOL bDisable // disable flag<nl>
    VAMPRET DisableTS_LinePitch(
        BOOL bDisable );

    // @cmember Collapse packets into one DWORD. This function is used for
    //          audio data. Collapse the packets is a candidate for the
    //          registry.<nl>
    // Parameterlist:<nl>
    // BOOL nCollapse // collapse flag<nl>
    VAMPRET CollapseTS_Packets(
        BOOL nCollapse );

    // @cmember Evaluate or not the raising or trailing edge of Start of
    //          Packets and generate or not an End of Packets. This function
    //          makes only sense for serial transport stream. The edge and the
    //          evaluating of this are candidates for the registry.<nl>
    // Parameterlist:<nl>
    // eEdge nEdge // edge to evaluate (raising/trailing)<nl>
    // BOOL bEvaluate // evaluate flag<nl>
    // BOOL bGenerateEOP // generate flag<nl>
    VAMPRET EvaluateTS_Edge(
        eEdge nEdge, 
        BOOL bEvaluate, 
        BOOL bGenerateEOP );

    // @cmember Flip MSB and LSB bit by bit within a byte. This function makes
    //          only sense for serial transport stream. The bit flipping is a
    //          candidate for the registry.<nl>
    // Parameterlist:<nl>
    // BOOL bFlip // flipping flag<nl>
    VAMPRET BitflipTS(
        BOOL bFlip );

    // @cmember Shift data one clock cycle. This function makes only sense for
    //          serial transport stream and audio data. The shifting of the
    //          data is a candidate for the registry.<nl>
    // Parameterlist:<nl>
    // BOOL bShift // shift flag<nl>
    VAMPRET ShiftDataTS(
        BOOL bShift );

    // @cmember This function reads all the transport stream registry
    //          information and stores these values to private member
    //          variables.<nl>
    // Parameterlist:<nl>
    // eTSStreamType nStreamType // stream type<nl>
    VAMPRET ReadTSRegistry(
        eTSStreamType nStreamType );

// VSB Interface
    // @cmember This function swaps the source of the VSB ports. The swapping
    //          of the VSB sources is a candidate for the registry.<nl>
    //          TRUE  = VSB_A->ADC_B, VSB_B->ADC_A<nl>
    //          FALSE = VSB_A->ADC_A, VSB_B->ADC_B<nl>
    // Parameterlist:<nl>
    // BOOL bSwap // swap flag<nl>
    VAMPRET SwapVSB_SRC(
        BOOL bSwap );

    // @access public 
public:
    // @cmember The constructor has to get the information of the
    //          EEPROM/Bootstrap about the access of the pins and store this
    //          information in the m_PIN_INFO member variable.<nl>
    // Parameterlist:<nl>
    // CVampDeviceResources *pDevRes // pointer on device resources object<nl>
    CVampGPIO(
            CVampDeviceResources* pDevRes);

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    // @cmember Destructor. Disables all GPIO pins.<nl>
    virtual ~CVampGPIO();

    // @cmember Reset GPIO. This function uses the PRESET Pin Nr28. It has to
    //          be called before using anything from the GPIO interface.<nl>
    VAMPRET Reset();

    // @cmember Returns the EEPROM/Bootstrap information about the GPIO pins.
    //          The configuration of each pin is in the appropriated bit. One
    //          means InputOnly and zero means InputAndOutput.<nl>
    // Parameterlist:<nl>
    // DWORD *pdwPinsConfig // pointer on pin configuration<nl>
     VAMPRET GetPinsConfiguration(
         DWORD* pdwPinsConfig );

// Video Out interface
    // @cmember This function sets all the necessary registers for ITU GPIO
    //          out format and enables or disables it.<nl>
    // Parameterlist:<nl>
    // BOOL bEnable // enable flag<nl>
    VAMPRET EnableITU( 
        BOOL bEnable );

	// @cmember This function sets all the necessary registers for VIP 1.1 GPIO
    //          out format and enables or disables it.<nl>
    // Parameterlist:<nl>
    // BOOL bEnable // enable flag<nl>
    VAMPRET EnableVIP11( 
        BOOL bEnable );

	// @cmember This function sets all the necessary registers for VIP 2.0 GPIO
    //          out format and enables or disables it.<nl>
    // Parameterlist:<nl>
    // BOOL bEnable // enable flag<nl>
    VAMPRET EnableVIP20( 
        BOOL bEnable );

// Audio interface
    // @cmember This function sets all the necessary registers for Audio
    //          stream and enables or disables it.<nl>
    // Parameterlist:<nl>
    // BOOL bEnable // enable flag<nl>
    VAMPRET EnableAudio( 
        BOOL bEnable );

// Transport stream interface
    // @cmember This function enables or disables the serial or parallel
    //          transport stream.<nl>
    // Parameterlist:<nl>
    // eTSStreamType nStreamType // transport stream type (serial/parallel)<nl>
    // BOOL bEnable // enable flag<nl>
    VAMPRET EnableTS(
        eTSStreamType nStreamType,
        BOOL bEnable );

    // @cmember This function sets the bytes per packets. The default value
    //          is 188. If the bytes per packets are greater than 188, the
    //          VALID signals make no sense any more.<nl>
    // Parameterlist:<nl>
    // BYTE ucBPP // bytes per packets<nl>
    VAMPRET SetTS_BytesPerPacket(
        BYTE ucBPP );

    // @cmember This function returns the bytes per packets.<nl>
    BYTE GetTS_BytesPerPacket();

    // @cmember This function sets the number of packets. The default value is
    //          312.<nl>
    // Parameterlist:<nl>
    // DWORD dwNOP // number of packets<nl>
    VAMPRET SetTS_NumOfPackets(
        DWORD dwNOP );

    // @cmember This function returns the number of packets.<nl>
    DWORD GetTS_NumOfPackets();

    // @cmember This function resets the transport stream interface. It has to
    //          be used (ResetTS (FALSE)) before using anything from the TS
    //          interface.<nl>
    VAMPRET ResetTS();

// VSB interface
    // @cmember This function enables the VSB_A interface.<nl>
    // Parameterlist:<nl>
    // BOOL bEnable // enable flag<nl>
    VAMPRET EnableVSB_A(
        BOOL bEnable );

    // @cmember This function enables the VSB_B interface.<nl>
    // Parameterlist:<nl>
    // BOOL bEnable // enable flag<nl>
    VAMPRET EnableVSB_B(
        BOOL bEnable );

    // @cmember This function enables the VSB_A and VSB_B interface.<nl>
    // Parameterlist:<nl>
    // BOOL bEnable // enable flag<nl>
    VAMPRET EnableVSB_AB(
        BOOL bEnable );

// GPIO Static interface
    // @cmember This function writes a value to the GPIO pin. It fails if the
    //          pin is already in use or the direction can not be set.<nl>
    // Parameterlist:<nl>
    // BYTE nPinNr // pin number<nl>
    // BYTE ucValue // value<nl>
    VAMPRET WriteToPinNr(
        BYTE nPinNr, 
        BYTE ucValue );
    // @cmember This function writes a value to the GPIO pin. It fails if the
    //          pin is already in use or the direction can not be set.<nl>
    // Parameterlist:<nl>
    // BYTE nGPIONr // pin number<nl>
    // BYTE ucValue // value<nl>
    VAMPRET WriteToGPIONr(
        BYTE nGPIONr, 
        BYTE ucValue );

    // @cmember This function reads a value from the GPIO pin. It fails if the
    //          pin is already in use or the direction can not be set.<nl>
    // Parameterlist:<nl>
    // BYTE nPinNr // pin number<nl>
    // BYTE FAR * pucValue // value pointer<nl>
    VAMPRET ReadFromPinNr(
        BYTE nPinNr, 
        BYTE FAR * pucValue );
    // @cmember This function reads a value from the GPIO pin. It fails if the
    //          pin is already in use or the direction can not be set.<nl>
    // Parameterlist:<nl>
    // BYTE nGPIONr // pin number<nl>
    // BYTE FAR * pucValue // value pointer<nl>
    VAMPRET ReadFromGPIONr(
        BYTE nGPIONr, 
        BYTE FAR * pucValue );
};

#endif //GPIO__
