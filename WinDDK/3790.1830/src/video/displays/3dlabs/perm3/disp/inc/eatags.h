/******************************Module*Header**********************************\
*
*                           *******************
*                           *   SAMPLE CODE   *
*                           *******************
*
* Module Name: EATags.h
*
* Content: Debugging support macros and structures
*
* Copyright (c) 1994-1999 3Dlabs Inc. Ltd. All rights reserved.
* Copyright (c) 1995-2002 Microsoft Corporation.  All rights reserved.
\*****************************************************************************/

#define D3DCanCreateD3DBuffer_ID                1
#define D3DCreateD3DBuffer_ID                   2
#define D3DDestroyD3DBuffer_ID                  3
#define D3DLockD3DBuffer_ID                     4
#define D3DUnlockD3DBuffer_ID                   5
#define D3DContextCreate_ID                     6
#define D3DContextDestroy_ID                    7
#define _D3D_OP_Clear2_ID                       8
#define D3DDrawPrimitives2_P3_ID                9
#define D3DValidateDeviceP3_ID                  10
#define D3DGetDriverState_ID                    11
#define D3DCreateSurfaceEx_ID                   12
#define D3DDestroyDDLocal_ID                    13
#define _D3D_SU_SurfInternalSetDataRecursive_ID 14
#define DdMapMemory_ID                          15
#define DrvGetDirectDrawInfo_ID                 16
#define DdFlip_ID                               17
#define DdWaitForVerticalBlank_ID               18
#define DdLock_ID                               19
#define DdUnlock_ID                             20
#define DdGetScanLine_ID                        21
#define DdGetBltStatus_ID                       22
#define DdGetFlipStatus_ID                      23
#define DdGetDriverInfo_ID                      24
#define DdBlt_ID                                25
#define DdCanCreateSurface_ID                   26
#define DdCreateSurface_ID                      27
#define DdDestroySurface_ID                     28
#define DdSetColorKey_ID                        29
#define DdGetAvailDriverMemory_ID               30
#define DrvBitBlt_ID                            31
#define DrvCopyBits_ID                          32
#define DrvGradientFill_ID                      33
#define DrvTransparentBlt_ID                    34
#define DrvAlphaBlend_ID                        35
#define DrvRealizeBrush_ID                      36
#define DrvDitherColor_ID                       37
#define DrvResetPDEV_ID                         38
#define DrvEnablePDEV_ID                        39
#define DrvDisablePDEV_ID                       40
#define DrvCompletePDEV_ID                      41
#define DrvEnableSurface_ID                     42
#define DrvDisableSurface_ID                    43
#define DrvAssertMode_ID                        44
#define DrvGetModes_ID                          45
#define DrvEscape_ID                            46
#define DrvNotify_ID                            47
#define DrvFillPath_ID                          48
#define DrvCreateDeviceBitmap_ID                49
#define DrvDeleteDeviceBitmap_ID                50
#define DrvDeriveSurface_ID                     51
#define DrvLineTo_ID                            52
#define DrvPaint_ID                             53
#define DrvSetPalette_ID                        54
#define DrvIcmSetDeviceGammaRamp_ID             55
#define DrvMovePointer_ID                       56
#define DrvSetPointerShape_ID                   57
#define DrvStrokePath_ID                        58
#define DrvSynchronize_ID                       59
#define DrvTextOut_ID                           60

#define EA_TAG_ENABLE 0x80000000

#define MIN_EA_TAG 1
#define MAX_EA_TAG 60



