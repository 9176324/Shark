//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!! NOTE : Automatic generated FILE - DON'T CHANGE directly             
//!!        You only may change the file Saa7134_RegDef.h                
//!!        and call GenRegIF.bat for update, check in the necessary     
//!!        files before                                                 
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#ifndef _MUNITSC2DMA_INCLUDE_
#define _MUNITSC2DMA_INCLUDE_


#include "SupportClass.h"


class CMunitSc2Dma
{
public:
//constructor-destructor of class CMunitSc2Dma
    CMunitSc2Dma ( PBYTE );
    virtual ~CMunitSc2Dma();


 //Register Description of this class


    CRegister ControlReg;
    // Video Output format Task A, Packmodes (YUV422, YUV444, .....)
    CRegField PackModeVA;
    // Select clipping mode for Video, Task A:
    // 00: clipping disabled
    // 01: clip flag from subblock clipping controls byte enables
    // 10: alpha clipping enabled
    // 11: clip flag inserts fixed colour
    CRegField ClipModeVA;
    // Enables dithering, active high
    CRegField DitherVA;
    // Video Output format Task A, Packmodes (YUV422, YUV444, .....)
    CRegField PackModeDA;
    // Select clipping mode for Video, Task A:
    CRegField ClipModeDA;
    // Enables dithering, active high
    CRegField DitherDA;
    // Video Output format Task A, Packmodes (YUV422, YUV444, .....)
    CRegField PackModeVB;
    // Select clipping mode for Video, Task A:
    CRegField ClipModeVB;
    // Enables dithering, active high
    CRegField DitherVB;
    // Video Output format Task A, Packmodes (YUV422, YUV444, .....)
    CRegField PackModeDB;
    // Select clipping mode for Video, Task A:
    CRegField ClipModeDB;
    // Enables dithering, active high
    CRegField DitherDB;
    CRegister ClipReg;
    // Alpha value vor non clipping
    CRegField AlphaNonClip;
    // Alpha value for Clipping
    CRegField AlphaClip;
    CRegister ClipColor;
    // indicates x position of taken uv sample
    // relevant to formats YUV422planar, YUV420planar and INDEO
    CRegField UVPixel;
    // Clip List inversion
    CRegField ClipListInvert;
    // Clip Colour R portion
    CRegField ClipColR;
    // Clip Colour G portion
    CRegField ClipColG;
    // Clip Colour B portion
    CRegField ClipColB;
    CScalerClippingInfo ClippingInfo0;
    CScalerClippingInfo ClippingInfo1;
    CScalerClippingInfo ClippingInfo2;
    CScalerClippingInfo ClippingInfo3;
    CScalerClippingInfo ClippingInfo4;
    CScalerClippingInfo ClippingInfo5;
    CScalerClippingInfo ClippingInfo6;
    CScalerClippingInfo ClippingInfo7;
    CScalerClippingInfo ClippingInfo8;
    CScalerClippingInfo ClippingInfo9;
    CScalerClippingInfo ClippingInfo10;
    CScalerClippingInfo ClippingInfo11;
    CScalerClippingInfo ClippingInfo12;
    CScalerClippingInfo ClippingInfo13;
    CScalerClippingInfo ClippingInfo14;
    CScalerClippingInfo ClippingInfo15;
  };



#endif _MUNITSC2DMA_
