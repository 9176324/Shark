//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!! NOTE : Automatic generated FILE - DON'T CHANGE directly             
//!!        You only may change the file Saa7134_RegDef.h                
//!!        and call GenRegIF.bat for update, check in the necessary     
//!!        files before                                                 
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#ifndef _MUNITSCALER_INCLUDE_
#define _MUNITSCALER_INCLUDE_


#include "SupportClass.h"


class CMunitScaler
{
public:
//constructor-destructor of class CMunitScaler
    CMunitScaler ( PBYTE );
    virtual ~CMunitScaler();


 //Register Description of this class


    CRegister SourceDependent;
    // vertical trigger edge occurs at line YPOS
    CRegField YPos;   
    // define horizontal trigger edge:
    // 0: pixel counter at rising (trailing) / line counter at falling (leading) edge
    // 1: vice versa
    CRegField DHEdge; 
    // define vertical trigger edge : 0: rising edge of input V-reference
    CRegField DVEdge; 
    // field ID polarity: 0 as is, 1 inverted
    CRegField DFid;   
    CRegister Status_1;
    // 1: enable video region Task A, as defined by VXSTxx and VYSTxx parameters
    // 0: disable
    CRegField VidEnA; 
    // 1: enable VBI region Task A, as defined by DXSTxx and DYSTxx parameters
    // 0: disable
    CRegField VbiEnA; 
    // 1: enable video region Task B, as defined by VXSTxx and VYSTxx parameters
    // 0: disable
    CRegField VidEnB; 
    // 1: enable VBI region Task B, as defined by DXSTxx and DYSTxx parameters
    // 0: disable
    CRegField VbiEnB; 
    // 1: active software reset
    CRegField SwRst;  
    // 1: video region Task A active
    // 0: inactive
    CRegField VidStatA;
    // 1: VBI region Task A active
    // 0: inactive
    CRegField VbiStatA;
    // 1: video region Task B active
    // 0: inactive
    CRegField VidStatB;
    // 1: VBI region Task B active
    // 0: inactive
    CRegField VbiStatB;
    // Trigger Error flag - scaler fails to trigger for at least 2 fields
    // trigger conditions need to be checked, flag is reset with the next input V-Sync
    CRegField TrErr;  
    // Configuration Error flag -  scaler data path not ready at start of a region
    // I/O window configuration to be checked, flag is reset with the next iput V-Sync
    CRegField CfErr;  
    // Load error flag - assigns, that scaler data path is overloaded, due to 
    // missmatched I/O or internal data rates, flag is reset with the next input V-sync
    CRegField LdErr;  
    // Was reset flag -  assigns, that scaler data path was resetted
    // (returns from IDLE stat), flag is reset with the next input V-sync
    CRegField WasRst; 
    // field ID as seen at the scaler input
    CRegField FidScI; 
    // field ID as assigned by the scaler output
    CRegField FidScO; 
    // logical XOR of FIDSCO and FIDSCI, may be used to find
    // permanent problem with the field ID interpretation
    CRegField ScoXorSci;
    // actual page flag
    CRegField Page;   
    CRegister StartPoints;
    // GREEN Path: start point for curve generation (straight binary)
    CRegField PGa0Start;
    // BLUE Path : start point for curve generation (straight binary)
    CRegField PGa1Start;
    // RED Path  : start point for curve generation (straigth binary)
    CRegField PGa2Start;
    // GREEN path: points for gamma curve calculation (straight binary)
    // special meaning of point P0GA0: defines start point and first 
    // curve point start at ... , first point at ...
    CRegArray PGa0;
    // BLUE path: points for gamma curve calculation (straight binary)
    // special meaning of point P1GA0: defines start point and first 
    // curve point start at ... , first point at ...
    CRegArray PGa1;
    // RED path: points for gamma curve calculation (straight binary)
    // special meaning of point P2GA0: defines start point and first 
    // curve point start at ... , first point at ...
    CRegArray PGa2;
    CPage Page1;
    CPage Page2;
  };



#endif _MUNITSCALER_
