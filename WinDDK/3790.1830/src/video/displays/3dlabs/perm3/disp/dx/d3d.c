/******************************Module*Header**********************************\
*
*                           *******************
*                           * D3D SAMPLE CODE *
*                           *******************
*
* Module Name: d3d.c
*
* Content: Main D3D capabilites and callback tables
*
* Copyright (c) 1994-1999 3Dlabs Inc. Ltd. All rights reserved.
* Copyright (c) 1995-2002 Microsoft Corporation.  All rights reserved.
\*****************************************************************************/

//-----------------------------------------------------------------------------
//
// Certain conventions are followed in this sample driver in order to ease 
// the code reading process:
//
// - All driver function callbacks are prefixed with either D3D or DD. No other
//   functions start with such a prefix
//
// - Helper (or secondary) functions which are called from other places INSIDE
//   the driver (in different files) are prefixed with either _D3D or _DD
//
// - Helper functions called from within the same file are prefixed with __
//   (but not with __D3D or __DD !) so names as __CTX_CleanDirect3DContext arise
//
// - Data structures declared and used only be the driver are prefixed with P3
//
// - Very minor hungarian notation is used, basically in the form of prefixes 
//   for DWORDs (dw), pointers (p), handles (h), and counters (i).
//
// - Global data items are prefixed with g_
//
// - This driver is intended to be source code compatible between the NT and 
//   Win9x kernel display driver models. As such, most kernel structures retain
//   their Win9x name ( The DX8 d3dnthal.h shares the same names as the Win9x
//   d3dhal.h and for DX7 dx95type.h in the Win2K DDK will perform the 
//   required level of translation). Major differences however are observed 
//   using preprocessor #if statements.
//
//-----------------------------------------------------------------------------

#include "glint.h"

//-----------------------------------------------------------------------------
// in-the-file nonexported forward declarations
//-----------------------------------------------------------------------------
void __D3D_BuildTextureFormatsP3(P3_THUNKEDDATA *pThisDisplay, 
                             DDSURFACEDESC TexFmt[MAX_TEXTURE_FORMAT],
                             DWORD *pNumIntTextures,
                             DWORD *pNumExtTextures);

#if DIRECT3D_VERSION >= 0x0800
void __D3D_Fill_DX8Caps(D3DCAPS8 *pd3d8caps,
                        D3DDEVICEDESC_V1 *pDeviceDesc,
                        D3DHAL_D3DEXTENDEDCAPS *pD3DEC,
                        DDHALINFO *pDDHALInfo);
#endif // DX8_DDI                        

//-----------------------------------------------------------------------------
// This structure contains all the the primitive capabilities (D3DPRIMCAPS)
// this driver supports for triangles and lines. All of the information in this 
// table will be implementation specific according to the specifications of 
// the hardware.
//-----------------------------------------------------------------------------

#define P3RXTriCaps {                                                    \
    sizeof(D3DPRIMCAPS),                                                 \
    D3DPMISCCAPS_CULLCCW        |              /* MiscCaps */            \
        D3DPMISCCAPS_CULLCW     |                                        \
        D3DPMISCCAPS_CULLNONE   |                                        \
        D3DPMISCCAPS_MASKZ      |                                        \
        D3DPMISCCAPS_LINEPATTERNREP,                                     \
    D3DPRASTERCAPS_DITHER            |         /* RasterCaps */          \
        D3DPRASTERCAPS_PAT           |                                   \
        D3DPRASTERCAPS_SUBPIXEL      |                                   \
        D3DPRASTERCAPS_ZTEST         |                                   \
        D3DPRASTERCAPS_FOGVERTEX     |                                   \
        D3DPRASTERCAPS_FOGTABLE      |                                   \
        D3DPRASTERCAPS_ZFOG          |                                   \
        D3DPRASTERCAPS_STIPPLE       |                                   \
        D3DPRASTERCAPS_MIPMAPLODBIAS,                                    \
    D3DPCMPCAPS_NEVER            |             /* ZCmpCaps */            \
        D3DPCMPCAPS_LESS         |                                       \
        D3DPCMPCAPS_EQUAL        |                                       \
        D3DPCMPCAPS_LESSEQUAL    |                                       \
        D3DPCMPCAPS_GREATER      |                                       \
        D3DPCMPCAPS_NOTEQUAL     |                                       \
        D3DPCMPCAPS_GREATEREQUAL |                                       \
        D3DPCMPCAPS_ALWAYS       |                                       \
        D3DPCMPCAPS_LESSEQUAL,                                           \
    D3DPBLENDCAPS_ZERO             |           /* SourceBlendCaps */     \
        D3DPBLENDCAPS_ONE          |                                     \
        D3DPBLENDCAPS_SRCALPHA     |                                     \
        D3DPBLENDCAPS_INVSRCALPHA  |                                     \
        D3DPBLENDCAPS_DESTALPHA    |                                     \
        D3DPBLENDCAPS_INVDESTALPHA |                                     \
        D3DPBLENDCAPS_DESTCOLOR    |                                     \
        D3DPBLENDCAPS_INVDESTCOLOR |                                     \
        D3DPBLENDCAPS_SRCALPHASAT  |                                     \
        D3DPBLENDCAPS_BOTHSRCALPHA |                                     \
        D3DPBLENDCAPS_BOTHINVSRCALPHA,                                   \
    D3DPBLENDCAPS_ZERO            |            /* DestBlendCaps */       \
        D3DPBLENDCAPS_ONE         |                                      \
        D3DPBLENDCAPS_SRCCOLOR    |                                      \
        D3DPBLENDCAPS_INVSRCCOLOR |                                      \
        D3DPBLENDCAPS_SRCALPHA    |                                      \
        D3DPBLENDCAPS_INVSRCALPHA |                                      \
        D3DPBLENDCAPS_DESTALPHA   |                                      \
        D3DPBLENDCAPS_INVDESTALPHA,                                      \
    D3DPCMPCAPS_NEVER            |             /* Alphatest caps */      \
        D3DPCMPCAPS_LESS         |                                       \
        D3DPCMPCAPS_EQUAL        |                                       \
        D3DPCMPCAPS_LESSEQUAL    |                                       \
        D3DPCMPCAPS_GREATER      |                                       \
        D3DPCMPCAPS_NOTEQUAL     |                                       \
        D3DPCMPCAPS_GREATEREQUAL |                                       \
        D3DPCMPCAPS_ALWAYS,                                              \
    D3DPSHADECAPS_COLORFLATRGB              |  /* ShadeCaps */           \
        D3DPSHADECAPS_COLORGOURAUDRGB       |                            \
        D3DPSHADECAPS_SPECULARFLATRGB       |                            \
        D3DPSHADECAPS_SPECULARGOURAUDRGB    |                            \
        D3DPSHADECAPS_FOGFLAT               |                            \
        D3DPSHADECAPS_FOGGOURAUD            |                            \
        D3DPSHADECAPS_ALPHAFLATBLEND        |                            \
        D3DPSHADECAPS_ALPHAGOURAUDBLEND     |                            \
        D3DPSHADECAPS_ALPHAFLATSTIPPLED,                                 \
    D3DPTEXTURECAPS_PERSPECTIVE         |      /* TextureCaps */         \
        D3DPTEXTURECAPS_ALPHA           |                                \
        D3DPTEXTURECAPS_POW2            |                                \
        D3DPTEXTURECAPS_ALPHAPALETTE    |                                \
        D3DPTEXTURECAPS_TRANSPARENCY,                                    \
    D3DPTFILTERCAPS_NEAREST              |     /* TextureFilterCaps*/    \
        D3DPTFILTERCAPS_LINEAR           |                               \
        D3DPTFILTERCAPS_MIPNEAREST       |                               \
        D3DPTFILTERCAPS_MIPLINEAR        |                               \
        D3DPTFILTERCAPS_LINEARMIPNEAREST |                               \
        D3DPTFILTERCAPS_LINEARMIPLINEAR  |                               \
        D3DPTFILTERCAPS_MIPFPOINT        |                               \
        D3DPTFILTERCAPS_MIPFLINEAR       |                               \
        D3DPTFILTERCAPS_MAGFPOINT        |                               \
        D3DPTFILTERCAPS_MAGFLINEAR       |                               \
        D3DPTFILTERCAPS_MINFPOINT        |                               \
        D3DPTFILTERCAPS_MINFLINEAR,                                      \
    D3DPTBLENDCAPS_DECAL             |        /* TextureBlendCaps */     \
        D3DPTBLENDCAPS_DECALALPHA    |                                   \
        D3DPTBLENDCAPS_MODULATE      |                                   \
        D3DPTBLENDCAPS_MODULATEALPHA |                                   \
        D3DPTBLENDCAPS_ADD           |                                   \
        D3DPTBLENDCAPS_COPY,                                             \
    D3DPTADDRESSCAPS_WRAP       |              /* TextureAddressCaps */  \
        D3DPTADDRESSCAPS_MIRROR |                                        \
        D3DPTADDRESSCAPS_CLAMP  |                                        \
        D3DPTADDRESSCAPS_INDEPENDENTUV,                                  \
    8,                                         /* StippleWidth */        \
    8                                          /* StippleHeight */       \
}    

static D3DDEVICEDESC_V1 g_P3RXCaps = {
    sizeof(D3DDEVICEDESC_V1),                 // dwSize 
    D3DDD_COLORMODEL               |          // dwFlags 
        D3DDD_DEVCAPS              |
        D3DDD_TRICAPS              |
        D3DDD_LINECAPS             |
        D3DDD_DEVICERENDERBITDEPTH |
        D3DDD_DEVICEZBUFFERBITDEPTH,
    D3DCOLOR_RGB ,                            // dcmColorModel
    D3DDEVCAPS_CANRENDERAFTERFLIP       |     // devCaps 
        D3DDEVCAPS_FLOATTLVERTEX        |
        D3DDEVCAPS_SORTINCREASINGZ      |
        D3DDEVCAPS_SORTEXACT            |
        D3DDEVCAPS_TLVERTEXSYSTEMMEMORY |
        D3DDEVCAPS_EXECUTESYSTEMMEMORY  |
        D3DDEVCAPS_TEXTUREVIDEOMEMORY   |
        D3DDEVCAPS_DRAWPRIMTLVERTEX     |
        D3DDEVCAPS_DRAWPRIMITIVES2      |       
#if DX7_VERTEXBUFFERS
        D3DDEVCAPS_HWVERTEXBUFFER       |
#endif        
        D3DDEVCAPS_HWRASTERIZATION      |
        D3DDEVCAPS_DRAWPRIMITIVES2EX,
    { sizeof(D3DTRANSFORMCAPS), 0 },            // transformCaps 
    FALSE,                                      // bClipping 
    { sizeof(D3DLIGHTINGCAPS), 0 },             // lightingCaps 
    P3RXTriCaps,                                // lineCaps 
    P3RXTriCaps,                                // triCaps 
        DDBD_16 | DDBD_32,                      // dwDeviceRenderBitDepth 
        DDBD_16 | DDBD_32,                      // Z Bit depths 
        0,                                      // dwMaxBufferSize 
    0                                           // dwMaxVertexCount 
};

D3DHAL_D3DEXTENDEDCAPS gc_D3DEC = {
    sizeof(D3DHAL_D3DEXTENDEDCAPS),       // dwSize                   // DX5
    1,                                    // dwMinTextureWidth
    2048,                                 // dwMaxTextureWidth
    1,                                    // dwMinTextureHeight
    2048,                                 // dwMaxTextureHeight
    32,                                   // dwMinStippleWidth
    32,                                   // dwMaxStippleWidth
    32,                                   // dwMinStippleHeight
    32,                                   // dwMaxStippleHeight

    0,  /*azn*/                           // dwMaxTextureRepeat       //DX6
    0,                                    // dwMaxTextureAspectRatio (no limit)
    1,                                    // dwMaxAnisotropy
    -4096.0f,                             // dvGuardBandLeft
    -4096.0f,                             // dvGuardBandTop
    4096.0f,                              // dvGuardBandRight
    4096.0f,                              // dvGuardBandBottom
    0.0f,                                 // dvExtentsAdjust                           
    D3DSTENCILCAPS_KEEP    |              // dwStencilCaps
       D3DSTENCILCAPS_ZERO    |
       D3DSTENCILCAPS_REPLACE |
       D3DSTENCILCAPS_INCRSAT |
       D3DSTENCILCAPS_DECRSAT |
       D3DSTENCILCAPS_INVERT  |
       D3DSTENCILCAPS_INCR    |
       D3DSTENCILCAPS_DECR,                                        
    8,                                          // dwFVFCaps                  
    D3DTEXOPCAPS_DISABLE                      | // dwTextureOpCaps
       D3DTEXOPCAPS_SELECTARG1                | 
       D3DTEXOPCAPS_SELECTARG2                |
       D3DTEXOPCAPS_MODULATE                  |
       D3DTEXOPCAPS_MODULATE2X                |
       D3DTEXOPCAPS_MODULATE4X                |
       D3DTEXOPCAPS_ADD                       |
       D3DTEXOPCAPS_ADDSIGNED                 |
       D3DTEXOPCAPS_ADDSIGNED2X               |
       D3DTEXOPCAPS_SUBTRACT                  |
       D3DTEXOPCAPS_ADDSMOOTH                 |
       D3DTEXOPCAPS_BLENDDIFFUSEALPHA         |
       D3DTEXOPCAPS_BLENDTEXTUREALPHA         |
       D3DTEXOPCAPS_BLENDFACTORALPHA          |
       D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR    |
       D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA    |
       D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR |
       D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA |
       D3DTEXOPCAPS_DOTPRODUCT3,                                      
    2,                                    // wMaxTextureBlendStages
    2,                                    // wMaxSimultaneousTextures

    0,                                    // dwMaxActiveLights        // DX7
    0.0f,                                 // dvMaxVertexW
    0,                                    // wMaxUserClipPlanes
    0                                     // wMaxVertexBlendMatrices
};

#if DX9_DDI
static D3DCAPS9 g_P3RX_D3DCaps;
#elif DX8_DDI
static D3DCAPS8 g_P3RX_D3DCaps;
#endif // DX8_DDI

//--------------------------------------------------------
// Supported ZBuffer/Stencil Formats by this hardware
//--------------------------------------------------------

#define P3RX_Z_FORMATS 4

typedef struct 
{
    DWORD dwStructSize;

    DDPIXELFORMAT Format[P3RX_Z_FORMATS];
} ZFormats;

ZFormats P3RXZFormats = 
{
    P3RX_Z_FORMATS,
    {
        // Format 1 - 16 Bit Z Buffer, no stencil
        {
            sizeof(DDPIXELFORMAT),        
            DDPF_ZBUFFER,                     
            0,                            
            16,                     // Total bits in buffer
            0,                      // Stencil bits
            0xFFFF,                 // Z mask
            0,                      // Stencil mask
            0
        },
        // Format 2 - 24 bit Z Buffer, 8 bit stencil
        {
            sizeof(DDPIXELFORMAT),
            DDPF_ZBUFFER | DDPF_STENCILBUFFER,
            0,
            32,                     // Total bits in buffer
            8,                      // Stencil bits
            0x00FFFFFF,             // Z Mask
            0xFF000000,             // Stencil Mask
            0
        },
        // Format 3 - 15 bit Z Buffer, 1 bit stencil
        {
            sizeof(DDPIXELFORMAT),
            DDPF_ZBUFFER | DDPF_STENCILBUFFER,             
            0,                    
            16,                     // Total bits in buffer
            1,                      // Stencil bits
            0x7FFF,                 // Z Mask
            0x8000,                 // Stencil mask
            0
        },
        // Format 4 - 32 bit Z Buffer, no stencil
        {
            sizeof(DDPIXELFORMAT),
            DDPF_ZBUFFER,
            0,
            32,                     // Total bits in buffer
            0,                      // Stencil bits
            0xFFFFFFFF,             // Z Mask
            0,                      // Stencil Mask
            0
        }
    }
};

#if DX8_DDI
//----------------------------------------------------------------------------
// Supported DX8 RenderTarget/Texture/ZBuffer/Stencil Formats by this hardware
//----------------------------------------------------------------------------

#if DX8_MULTISAMPLING
// Note: For multisampling we need to setup appropriately both the rendertarget
// and the depth buffer format's multisampling fields.
#define D3DMULTISAMPLE_NUM_SAMPLES (1 << (D3DMULTISAMPLE_4_SAMPLES - 1))
#else
#define D3DMULTISAMPLE_NUM_SAMPLES  D3DMULTISAMPLE_NONE
#endif // DX8_MULTISAMPLING

#define DX8_FORMAT(FourCC, Ops, dwMSFlipTypes)                              \
    { sizeof(DDPIXELFORMAT), DDPF_D3DFORMAT, (FourCC), 0, (Ops),            \
        ((dwMSFlipTypes) & 0xFFFF ) << 16 | ((dwMSFlipTypes) & 0xFFFF), 0, 0 }

#if DX9_RT_COLORCONVERT                                          
#define PERM3_RT_CC_FLAGS    D3DFORMAT_OP_CONVERT_TO_ARGB 
#else
#define PERM3_RT_CC_FLAGS   0
#endif                                           

DDPIXELFORMAT DX8FormatTable[] =
{
    DX8_FORMAT(D3DFMT_X1R5G5B5,        D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE | 
                                         D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET, 
                                          D3DMULTISAMPLE_NUM_SAMPLES ),
    DX8_FORMAT(D3DFMT_R5G6B5,          D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_DISPLAYMODE |
                                         D3DFORMAT_OP_3DACCELERATION |
                                          PERM3_RT_CC_FLAGS |
                                           D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET, 
                                            D3DMULTISAMPLE_NUM_SAMPLES ),
    DX8_FORMAT(D3DFMT_R5G6B5,          D3DFORMAT_OP_VOLUMETEXTURE,                  0),
    DX8_FORMAT(D3DFMT_X8R8G8B8,        D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_DISPLAYMODE | 
                                         D3DFORMAT_OP_3DACCELERATION |
                                          PERM3_RT_CC_FLAGS |                                                 
                                           D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET,   0), 
    DX8_FORMAT(D3DFMT_X8R8G8B8,        D3DFORMAT_OP_VOLUMETEXTURE,                  0),
#if DX7_PALETTETEXTURE
    DX8_FORMAT(D3DFMT_P8,              D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE,                 0),
#endif

    DX8_FORMAT(D3DFMT_A1R5G5B5,        D3DFORMAT_OP_TEXTURE | 
//#if DX8_FULLSCREEN_FLIP_OR_DISCARD
                                           D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET |
//#endif
                                        D3DFORMAT_OP_VOLUMETEXTURE,                 0),
    DX8_FORMAT(D3DFMT_A4R4G4B4,        D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE,                 0),
    DX8_FORMAT(D3DFMT_A8R8G8B8,        D3DFORMAT_OP_TEXTURE | 
//#if DX8_FULLSCREEN_FLIP_OR_DISCARD
                                           D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET |
                                           D3DFORMAT_OP_SAME_FORMAT_UP_TO_ALPHA_RENDERTARGET |
//#endif
                                        D3DFORMAT_OP_VOLUMETEXTURE,                 0),
    DX8_FORMAT(D3DFMT_A4L4,            D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE,                 0),
    DX8_FORMAT(D3DFMT_A8L8,            D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE,                 0),
#if DX8_PRIVTEXFORMAT
    DX8_FORMAT(FOURCC_P3TT,            D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE,                 0),
#endif
    DX8_FORMAT(D3DFMT_L8,              D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE,                 0),
    DX8_FORMAT(D3DFMT_D16_LOCKABLE,    D3DFORMAT_OP_ZSTENCIL |
                                       D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH , 
                                            D3DMULTISAMPLE_NUM_SAMPLES ),
    DX8_FORMAT(D3DFMT_D32,             D3DFORMAT_OP_ZSTENCIL |
                                       D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH , 
                                            D3DMULTISAMPLE_NUM_SAMPLES ),
    DX8_FORMAT(D3DFMT_S8D24,           D3DFORMAT_OP_ZSTENCIL |
                                       D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH , 
                                            D3DMULTISAMPLE_NUM_SAMPLES ),
    DX8_FORMAT(D3DFMT_S1D15,           D3DFORMAT_OP_ZSTENCIL |
                                       D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH , 
                                            D3DMULTISAMPLE_NUM_SAMPLES )
};
#define DX8_FORMAT_COUNT (sizeof(DX8FormatTable) / sizeof(DX8FormatTable[0]))
#endif // DX8_DDI

#if DX9_ASYNC_NOTIFICATIONS    
D3DQUERYTYPE DX9QueryTable[] = 
{ 
    D3DQUERYTYPE_VERTEXSTATS,
#if DX9_EVENT_QUERY
    D3DQUERYTYPE_EVENT
#endif
};

#define DX9_QUERY_COUNT (sizeof(DX9QueryTable) / sizeof(DX9QueryTable[0]))
#endif // DX9_ASYNC_NOTIFICATIONS

#ifdef W95_DDRAW
#define DDHAL_D3DBUFCALLBACKS DDHAL_DDEXEBUFCALLBACKS 
#endif

//-----------------------------------------------------------------------------
//
// void _D3DHALCreateDriver
//
// _D3DHALCreateDriver is a helper function, not a callback.
//
// Its main purpouse is to centralize the first part of D3D initialization 
// (the second part is handled by _D3DGetDriverInfo) . _D3DHALCreateDriver:
//      Clears contexts
//      Fills entry points to D3D driver.
//      Generates and passes back texture formats.
//
// If the structures are succesfully created the internal pointers 
// (lpD3DGlobalDriverData, lpD3DHALCallbacks and (maybe) lpD3DBufCallbacks)
// are updated to point to valid data structures.
//
//-----------------------------------------------------------------------------
void  
_D3DHALCreateDriver(P3_THUNKEDDATA *pThisDisplay)
{
    BOOL bRet;
    ULONG Result;
    D3DHAL_GLOBALDRIVERDATA* pD3DDriverData = NULL;
    D3DHAL_CALLBACKS* pD3DHALCallbacks = NULL;
    DDHAL_D3DBUFCALLBACKS* pD3DBufCallbacks = NULL;

    DBG_ENTRY(_D3DHALCreateDriver);

    // Verify if we have already created the necessary data. If so, don't go
    // again through this process.
    if ((pThisDisplay->lpD3DGlobalDriverData != 0) &&
        (pThisDisplay->lpD3DHALCallbacks != 0))
    {
        DISPDBG((WRNLVL,"D3D Data already created for this PDEV, "
                        "not doing it again."));

        // we keep the same structure pointers to previously 
        // created and stored in pThisDisplay                 
        
        DBG_EXIT(_D3DHALCreateDriver, 0); 
        return;
    }
    else
    {
        DISPDBG((WRNLVL,"Creating D3D caps/callbacks for the "
                        "first time on this PDEV"));
    }

    // We set the structure pointers to NULL in case an error happens and 
    // we're forced to disable D3D support
    pThisDisplay->lpD3DGlobalDriverData = 0;
    pThisDisplay->lpD3DHALCallbacks = 0;
    pThisDisplay->lpD3DBufCallbacks = 0;       

    // Initialize the context handle data structures (array) . We are careful
    // not to initialize the data structures twice (as between mode changes,
    // for example) as this info needs to be persistent between such events.
    _D3D_CTX_HandleInitialization();

    //-----------------------------------
    // Allocate necessary data structures
    //-----------------------------------

    // Initialize our pointers to global driver 
    // data and to HAL callbacks to NULL
    pThisDisplay->pD3DDriverData16 = 0;
    pThisDisplay->pD3DDriverData32 = 0;

    pThisDisplay->pD3DHALCallbacks16 = 0;
    pThisDisplay->pD3DHALCallbacks32 = 0;

    pThisDisplay->pD3DHALExecuteCallbacks16 = 0;
    pThisDisplay->pD3DHALExecuteCallbacks32 = 0;       

    // Allocate memory for the global driver data structure.
    SHARED_HEAP_ALLOC(&pThisDisplay->pD3DDriverData16, 
                      &pThisDisplay->pD3DDriverData32, 
                      sizeof(D3DHAL_GLOBALDRIVERDATA));
             
    if (pThisDisplay->pD3DDriverData32 == 0)
    {
        DISPDBG((ERRLVL, "ERROR: _D3DHALCreateDriver: "
                         "Failed to allocate driverdata memory"));
        
        DBG_EXIT(_D3DHALCreateDriver,0);         
        return;
    }
    
    DISPDBG((DBGLVL,"Allocated D3DDriverData Memory: p16:0x%x, p32:0x%x", 
                     pThisDisplay->pD3DDriverData16, 
                     pThisDisplay->pD3DDriverData32));

    // Allocate memory for the global HAL callback data structure.
    SHARED_HEAP_ALLOC(&pThisDisplay->pD3DHALCallbacks16, 
                      &pThisDisplay->pD3DHALCallbacks32, 
                      sizeof(D3DHAL_CALLBACKS));
             
    if (pThisDisplay->pD3DHALCallbacks32 == 0)
    {
        DISPDBG((ERRLVL, "ERROR: _D3DHALCreateDriver: "
                         "Failed to allocate callback memory"));

        SHARED_HEAP_FREE(&pThisDisplay->pD3DDriverData16, 
                         &pThisDisplay->pD3DDriverData32,
                         TRUE);
                
        DBG_EXIT(_D3DHALCreateDriver, 0); 
        return;
    }

    DISPDBG((DBGLVL,"Allocated HAL Callbacks Memory: p16:0x%x, p32:0x%x", 
                    pThisDisplay->pD3DHALCallbacks16, 
                    pThisDisplay->pD3DHALCallbacks32));

    // Allocate memory for the global Vertex Buffer callback data structure.
    SHARED_HEAP_ALLOC(&pThisDisplay->pD3DHALExecuteCallbacks16, 
                      &pThisDisplay->pD3DHALExecuteCallbacks32, 
                      sizeof(DDHAL_D3DBUFCALLBACKS));
             
    if (pThisDisplay->pD3DHALExecuteCallbacks32 == 0)
    {       
        DISPDBG((ERRLVL, "ERROR: _D3DHALCreateDriver: "
                         "Failed to allocate callback memory"));

        SHARED_HEAP_FREE(&pThisDisplay->pD3DDriverData16, 
                         &pThisDisplay->pD3DDriverData32,
                         TRUE);
        SHARED_HEAP_FREE(&pThisDisplay->pD3DHALCallbacks16, 
                         &pThisDisplay->pD3DHALCallbacks32,
                         TRUE);                          
                
        DBG_EXIT(_D3DHALCreateDriver, 0); 
        return;
    }

    DISPDBG((DBGLVL,"Allocated Vertex Buffer Callbacks Memory: "
                    "p16:0x%x, p32:0x%x", 
                    pThisDisplay->pD3DHALExecuteCallbacks16, 
                    pThisDisplay->pD3DHALExecuteCallbacks32));
               
    //------------------------------------------------------
    // Fill in the data structures the driver has to provide
    //------------------------------------------------------
    
    // Get the Pointers
    pD3DDriverData = (D3DHAL_GLOBALDRIVERDATA*)pThisDisplay->pD3DDriverData32;
    pD3DHALCallbacks = (D3DHAL_CALLBACKS*)pThisDisplay->pD3DHALCallbacks32;
    pD3DBufCallbacks = 
                (DDHAL_D3DBUFCALLBACKS *)pThisDisplay->pD3DHALExecuteCallbacks32;

    // Clear the global data
    memset(pD3DDriverData, 0, sizeof(D3DHAL_GLOBALDRIVERDATA));
    pD3DDriverData->dwSize = sizeof(D3DHAL_GLOBALDRIVERDATA);
    
    // Clear the HAL callbacks
    memset(pD3DHALCallbacks, 0, sizeof(D3DHAL_CALLBACKS));
    pD3DHALCallbacks->dwSize = sizeof(D3DHAL_CALLBACKS);

    // Clear the Vertex Buffer callbacks
    memset(pD3DBufCallbacks, 0, sizeof(DDHAL_D3DBUFCALLBACKS));
    pD3DBufCallbacks->dwSize = sizeof(DDHAL_D3DBUFCALLBACKS);
                          
    // Report that we can texture from nonlocal vidmem only if the 
    // card is an AGP one and AGP is enabed.
    if (pThisDisplay->bCanAGP)
    {
        g_P3RXCaps.dwDevCaps |= D3DDEVCAPS_TEXTURENONLOCALVIDMEM;
    }

#if DX7_ANTIALIAS
    // Report we support fullscreen antialiasing
    g_P3RXCaps.dpcTriCaps.dwRasterCaps |= 
#if DX8_DDI    
                                D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE  |
#endif                                
                                D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT;
#endif // DX7_ANTIALIAS
               
#if DX8_3DTEXTURES
    // Report we support 3D textures
    g_P3RXCaps.dpcTriCaps.dwTextureCaps |= D3DPTEXTURECAPS_VOLUMEMAP |
                                           D3DPTEXTURECAPS_VOLUMEMAP_POW2;
#endif // DX8_3DTEXTURES


#if DX8_DDI    
    if (TLCHIP_GAMMA)
    {
        // Enable handling of the new D3DRS_COLORWRITEENABLE
        // but only for GVX1 since VX1 has trouble with this at 16bpp
        g_P3RXCaps.dpcTriCaps.dwMiscCaps |= D3DPMISCCAPS_COLORWRITEENABLE;
        g_P3RXCaps.dpcLineCaps.dwMiscCaps |= D3DPMISCCAPS_COLORWRITEENABLE;
    }

    // Enable new cap for mipmap support
    g_P3RXCaps.dpcTriCaps.dwTextureCaps |= D3DPTEXTURECAPS_MIPMAP;
    g_P3RXCaps.dpcLineCaps.dwTextureCaps |= D3DPTEXTURECAPS_MIPMAP; 

#endif // DX8_DDI  

    //---------------------------
    // Fill in global driver data
    //---------------------------

    // Hardware caps supoorted
    pD3DDriverData->dwNumVertices = 0;       
    pD3DDriverData->dwNumClipVertices = 0;
    pD3DDriverData->hwCaps = g_P3RXCaps;

    // Build a table with the texture formats supported. Store in pThisDisplay
    // as we will need this also for DdCanCreateSurface. ( Notice that since 
    // _D3DHALCreateDriver will be called in every driver load or mode change,
    // we will have valid TextureFormats in pThisDisplay whenever 
    // DdCanCreateSurface is called )
    __D3D_BuildTextureFormatsP3(pThisDisplay, 
                            &pThisDisplay->TextureFormats[0],
                            &pThisDisplay->dwNumTextureFormats,
                            &pD3DDriverData->dwNumTextureFormats);

    pD3DDriverData->lpTextureFormats = pThisDisplay->TextureFormats;

    //---------------------------------------
    // Fill in context handling HAL callbacks
    //---------------------------------------
    pD3DHALCallbacks->ContextCreate  = D3DContextCreate;
    pD3DHALCallbacks->ContextDestroy = D3DContextDestroy;


    //---------------------------------------------------
    // Fill in Vertex Buffer callbacks pointers and flags
    //---------------------------------------------------

#if !DX7_VERTEXBUFFERS   
    // We won't use this structure at all so delete it
    SHARED_HEAP_FREE(&pThisDisplay->pD3DHALExecuteCallbacks16, 
                     &pThisDisplay->pD3DHALExecuteCallbacks32,
                     TRUE);       
    pD3DBufCallbacks = NULL;

#endif // DX7_VERTEXBUFFERS         

    //---------------------------------------------------------
    // We return with updated pThisDisplay internal pointers to 
    // the driver data, HAL and Vertex Buffer structures.
    //---------------------------------------------------------
    pThisDisplay->lpD3DGlobalDriverData = (ULONG_PTR)pD3DDriverData;
    pThisDisplay->lpD3DHALCallbacks = (ULONG_PTR)pD3DHALCallbacks;
    pThisDisplay->lpD3DBufCallbacks = (ULONG_PTR)pD3DBufCallbacks;    

#ifndef WNT_DDRAW

    //
    // Set up the same information in DDHALINFO
    //

    pThisDisplay->ddhi32.lpD3DGlobalDriverData = (ULONG_PTR)pD3DDriverData;
    pThisDisplay->ddhi32.lpD3DHALCallbacks     = (ULONG_PTR)pD3DHALCallbacks;
    pThisDisplay->ddhi32.lpDDExeBufCallbacks   = pD3DBufCallbacks;

#endif

    DBG_EXIT(_D3DHALCreateDriver,0); 
    return;
} // _D3DHALCreateDriver


//-----------------------------------------------------------------------------
//
// void _D3DGetDriverInfo
//
// _D3DGetDriverInfo is a helper function called by DdGetDriverInfo , not a 
// callback. Its main purpouse is to centralize the second part of D3D 
// initialization (the first part is handled by _D3DHALCreateDriver) . 
//
// _D3DGetDriverInfo handles the 
//
//           GUID_D3DExtendedCaps
//           GUID_D3DParseUnknownCommandCallback         
//           GUID_D3DCallbacks3
//           GUID_ZPixelFormats
//           GUID_Miscellaneous2Callbacks
//
// GUIDs and fills all the relevant information associated to them. 
// GUID_D3DCallbacks2 is not handled at all because it is a legacy GUID.
//
//-----------------------------------------------------------------------------
void 
_D3DGetDriverInfo(
    LPDDHAL_GETDRIVERINFODATA lpData)
{
    DWORD dwSize;
    P3_THUNKEDDATA *pThisDisplay;

    DBG_ENTRY(_D3DGetDriverInfo);

    // Get a pointer to the chip we are on.
    
#if WNT_DDRAW
    pThisDisplay = (P3_THUNKEDDATA*)(((PPDEV)(lpData->dhpdev))->thunkData);
#else    
    pThisDisplay = (P3_THUNKEDDATA*)lpData->dwContext;
    if (! pThisDisplay) 
    {
        pThisDisplay = g_pDriverData;
    }    
#endif

    // Fill in required Miscellaneous2 callbacks
    if ( MATCH_GUID(lpData->guidInfo, GUID_Miscellaneous2Callbacks))
    {
        DDHAL_DDMISCELLANEOUS2CALLBACKS DDMisc2;

        DISPDBG((DBGLVL, "  GUID_Miscellaneous2Callbacks"));

        memset(&DDMisc2, 0, sizeof(DDMisc2));

        dwSize = min(lpData->dwExpectedSize, 
                     sizeof(DDHAL_DDMISCELLANEOUS2CALLBACKS));
        lpData->dwActualSize = sizeof(DDHAL_DDMISCELLANEOUS2CALLBACKS);

        ASSERTDD((lpData->dwExpectedSize == 
                    sizeof(DDHAL_DDMISCELLANEOUS2CALLBACKS)), 
                  "ERROR: Callbacks 2 size incorrect!");

        DDMisc2.dwSize = dwSize;
        DDMisc2.dwFlags = DDHAL_MISC2CB32_GETDRIVERSTATE | 
                          DDHAL_MISC2CB32_CREATESURFACEEX | 
                          DDHAL_MISC2CB32_DESTROYDDLOCAL;
                          
        DDMisc2.GetDriverState = D3DGetDriverState;
        DDMisc2.CreateSurfaceEx = D3DCreateSurfaceEx;
        DDMisc2.DestroyDDLocal = D3DDestroyDDLocal;

        memcpy(lpData->lpvData, &DDMisc2, dwSize);
        lpData->ddRVal = DD_OK;
    }

    // Fill in the extended caps 
    if (MATCH_GUID((lpData->guidInfo), GUID_D3DExtendedCaps) )
    {
        DISPDBG((DBGLVL, "  GUID_D3DExtendedCaps"));
        dwSize = min(lpData->dwExpectedSize, sizeof(D3DHAL_D3DEXTENDEDCAPS));

        lpData->dwActualSize = sizeof(D3DHAL_D3DEXTENDEDCAPS);
 
        memcpy(lpData->lpvData, &gc_D3DEC, sizeof(gc_D3DEC) );
        lpData->ddRVal = DD_OK;
    }

    // Grab the pointer to the ParseUnknownCommand OS callback 
    if ( MATCH_GUID(lpData->guidInfo, GUID_D3DParseUnknownCommandCallback) )
    {
        DISPDBG((DBGLVL, "Get D3DParseUnknownCommandCallback"));

        *(ULONG_PTR *)(&pThisDisplay->pD3DParseUnknownCommand) = 
                                                    (ULONG_PTR)lpData->lpvData;

        ASSERTDD((pThisDisplay->pD3DParseUnknownCommand),
                 "D3D ParseUnknownCommand callback == NULL");
                 
        lpData->ddRVal = DD_OK;
    }

    // Fill in ZBuffer/Stencil formats supported. If you don't respond to
    // this GUID, ZBuffer formats will be inferred from the D3DDEVICEDESC 
    // copied in _D3DHALCreateDriver
    if ( MATCH_GUID(lpData->guidInfo, GUID_ZPixelFormats))
    {
        DISPDBG((DBGLVL, "  GUID_ZPixelFormats"));

        dwSize = min(lpData->dwExpectedSize, sizeof(P3RXZFormats));
        lpData->dwActualSize = sizeof(P3RXZFormats);
        memcpy(lpData->lpvData, &P3RXZFormats, dwSize);

        lpData->ddRVal = DD_OK;
    }

    // Fill in required D3DCallbacks3 callbacks
    if ( MATCH_GUID(lpData->guidInfo, GUID_D3DCallbacks3) )
    {
        D3DHAL_CALLBACKS3 D3DCallbacks3;
        memset(&D3DCallbacks3, 0, sizeof(D3DCallbacks3));

        DISPDBG((DBGLVL, "  GUID_D3DCallbacks3"));
        dwSize = min(lpData->dwExpectedSize, sizeof(D3DHAL_CALLBACKS3));
        lpData->dwActualSize = sizeof(D3DHAL_CALLBACKS3);
        
        ASSERTDD((lpData->dwExpectedSize == sizeof(D3DHAL_CALLBACKS3)), 
                  "ERROR: Callbacks 3 size incorrect!");

        D3DCallbacks3.dwSize = dwSize;
        D3DCallbacks3.dwFlags = D3DHAL3_CB32_VALIDATETEXTURESTAGESTATE  |
                                D3DHAL3_CB32_DRAWPRIMITIVES2;

        D3DCallbacks3.DrawPrimitives2 = D3DDrawPrimitives2_P3;      
        D3DCallbacks3.ValidateTextureStageState = D3DValidateDeviceP3;

        memcpy(lpData->lpvData, &D3DCallbacks3, dwSize);
        lpData->ddRVal = DD_OK;
    }

    // Check for calls to GetDriverInfo2
    // Notice : GUID_GetDriverInfo2 has the same value as GUID_DDStereoMode
#if DIRECT3D_VERSION >= 0x0800
    if ( MATCH_GUID(lpData->guidInfo, GUID_GetDriverInfo2) )
#else
    if ( MATCH_GUID(lpData->guidInfo, GUID_DDStereoMode) )
#endif
    {
#if DX8_DDI
        // Make sure this is actually a call to GetDriverInfo2 
        // ( and not a call to DDStereoMode!)
        if (D3DGDI_IS_GDI2(lpData))
        {
            // Yes, its a call to GetDriverInfo2, fetch the
            // DD_GETDRIVERINFO2DATA data structure.
            DD_GETDRIVERINFO2DATA* pgdi2 = D3DGDI_GET_GDI2_DATA(lpData);
            DD_GETFORMATCOUNTDATA* pgfcd;
            DD_GETFORMATDATA*      pgfd;
            DD_DXVERSION*          pdxv;
#if DX9_ASYNC_NOTIFICATIONS
            DD_GETD3DQUERYCOUNTDATA* pqcd;
            DD_GETD3DQUERYDATA*      pd3dq;
#endif // DX9_ASYNC_NOTIFICATIONS
#if DX9_DDI
            DD_GETDDIVERSIONDATA*  pgddiver; 
#endif

            // What type of request is this?
            switch (pgdi2->dwType)
            {
            case D3DGDI2_TYPE_DXVERSION:
                // This is a way for a driver on NT to find out the DX-Runtime 
                // version. This information is provided to a new driver (i.e. 
                // one that  exposes GETDRIVERINFO2) for DX7 applications and 
                // DX8 applications. And you should get x0000800 for 
                // dwDXVersion; or more accurately, you should get
                // DD_RUNTIME_VERSION which is defined in ddrawi.h.
                pdxv = (DD_DXVERSION*)pgdi2;  
                pThisDisplay->dwDXVersion = pdxv->dwDXVersion;
                lpData->dwActualSize = sizeof(DD_DXVERSION);
                lpData->ddRVal       = DD_OK;                
                break;
                
            case D3DGDI2_TYPE_GETFORMATCOUNT:
                {
                    // Its a request for the number of texture formats
                    // we support. Get the extended data structure so
                    // we can fill in the format count field.
                    pgfcd = (DD_GETFORMATCOUNTDATA*)pgdi2;
                    pgfcd->dwFormatCount = DX8_FORMAT_COUNT;
                    // Note : DX9 runtime passes in 0x900 in dwReserved
                    lpData->dwActualSize = sizeof(DD_GETFORMATCOUNTDATA);
                    lpData->ddRVal       = DD_OK;
                }
                break;

            case D3DGDI2_TYPE_GETFORMAT:
                {
                    // Its a request for a particular format we support.
                    // Get the extended data structure so we can fill in
                    // the format field.
                    pgfd = (DD_GETFORMATDATA*)pgdi2;
                    
                    if (pgfd->dwFormatIndex >= DX8_FORMAT_COUNT)
                    {
                        // The format index is out of boundary
                        lpData->dwActualSize = 0;
                        lpData->ddRVal       = DDERR_INVALIDPARAMS;
                        break;
                    }

                    // Set up the return value to indicate success
                    lpData->dwActualSize = sizeof(DD_GETFORMATDATA);
                    lpData->ddRVal       = DD_OK;                    

                    // DX9 passes in 0x900 in format.dwSize
                    // Note : older DX8 runtime uses 0xBADDCAFE for sanity check
                    if ((pgfd->format.dwSize < 0x900) || 
                        (pgfd->format.dwSize > 0xBA000000))
                    {
                        memcpy(&pgfd->format, 
                               &DX8FormatTable[pgfd->dwFormatIndex], 
                               sizeof(pgfd->format));
                        break;
                    }

#if DX9_DDI
                    // Initialize the surface description and copy over
                    // the pixel format from out pixel format table.
                    memcpy(&pgfd->format, 
                           &DX8FormatTable[pgfd->dwFormatIndex], 
                           sizeof(pgfd->format));

#if DX9_DP2BLT
                    // Support BLT to texture or backbuffer through DP2 command
                    if (pgfd->format.dwOperations &
                        (D3DFORMAT_OP_TEXTURE | D3DFORMAT_OP_DISPLAYMODE))
                    {
                        pgfd->format.dwOperations |= D3DFORMAT_OP_OFFSCREENPLAIN;
                    }
#endif
#if DX9_RT_COLORCONVERT
                    if ((pgfd->format.dwOperations & D3DFORMAT_OP_TEXTURE) &&
                        (pgfd->format.dwOperations & D3DFORMAT_OP_DISPLAYMODE))
                    {
                        // This will be effectively applied only to D3DFMT_R5G6B5
                        // and D3DFMT_X8R8G8B8 formats on the Perm3, other hw might
                        // vary support on the formats which are allowed to be converted
                        pgfd->format.dwOperations |= D3DFORMAT_MEMBEROFGROUP_ARGB;
                    }
    
                    // New flag for DX9
                    if (pgfd->format.dwOperations & D3DFORMAT_OP_3DACCELERATION)
                    {
                        pgfd->format.dwOperations |= D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
                    }
                    if (pgfd->format.dwFourCC == D3DFMT_A8R8G8B8)
                    {
                        pgfd->format.dwOperations |= D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
                    }

                    if (pgfd->format.dwOperations & D3DFORMAT_OP_TEXTURE) 
                    {
                        pgfd->format.dwOperations |= D3DFORMAT_OP_CONVERT_TO_ARGB;                        
                    }
#endif
#if DX9_AUTOGENMIPMAP
                    if ((pgfd->format.dwOperations & D3DFORMAT_OP_TEXTURE) && 
                        (pgfd->format.dwOperations & D3DFORMAT_OP_3DACCELERATION))
                    {
                        pgfd->format.dwOperations |= D3DFORMAT_OP_AUTOGENMIPMAP;                        
                    }
#endif

#if DX8_MULTISAMPLING
                    //
                    // If runtime is DX9 or newer, handle multisample cap 
                    // differently (use D3DMULTISAMPLE_NONMASKABLE and 
                    // allows only 1 quality level)
                    // 
                    if (pgfd->format.MultiSampleCaps.wFlipMSTypes == D3DMULTISAMPLE_NUM_SAMPLES)
                    {
                        pgfd->format.MultiSampleCaps.wFlipMSTypes = D3DMULTISAMPLE_NONMASKABLE;
                    }
                    if (pgfd->format.MultiSampleCaps.wBltMSTypes == D3DMULTISAMPLE_NUM_SAMPLES)
                    {
                        pgfd->format.MultiSampleCaps.wBltMSTypes = D3DMULTISAMPLE_NONMASKABLE;
                    }
#endif // DX8_MULTISAMPLING
#endif // DX9_DDI
                }
                break;

            case D3DGDI2_TYPE_GETD3DCAPS8:
#if DX9_DDI
            case D3DGDI2_TYPE_GETD3DCAPS9:
#endif // DX9_DDI
                {
                    // The runtime is requesting the DX8 D3D caps 

                    int    i;
                    size_t copySize;                   
                    
                    // We will populate this caps as much as we can
                    // from the DX7 caps structure(s). ( We need anyway
                    // to be able to report DX7 caps for DX7 apps )

                    __D3D_Fill_DX8Caps((D3DCAPS8 *)&g_P3RX_D3DCaps,
                                       &g_P3RXCaps,
                                       &gc_D3DEC,
                                       &pThisDisplay->ddhi32);

                    // And now we fill anything that might not be there
                    // These fields are new and absent from any  other legacy 
                    // structure

                    g_P3RX_D3DCaps.DeviceType = D3DDEVTYPE_HAL;   // Device Info 
                    g_P3RX_D3DCaps.AdapterOrdinal = 0;

#if DX_NOT_SUPPORTED_FEATURE
                    // NOTE: In some beta releases of this sample driver we 
                    //       used to setup bit caps for using it as a pure 
                    //       device (D3DDEVCAPS_PUREDEVICE). On the final 
                    //       DX8 release pure devices are not allowed on 
                    //       non-TnL/non hwvp parts as they don't give any 
                    //       real advantage over non-pure ones. 
                    
                    g_P3RX_D3DCaps.DevCaps |= D3DDEVCAPS_PUREDEVICE;
#endif                    

#if DX_NOT_SUPPORTED_FEATURE
#if DX8_3DTEXTURES
                    // On Windows XP the ability to lock just a subvolume of a 
                    // volume texture has been introduced in DX8.1 (Windows 2000
                    // will ignore it), but it doesn't actually work well.
                    g_P3RX_D3DCaps.DevCaps |= D3DDEVCAPS_SUBVOLUMELOCK;
#endif // DX8_3DTEXTURES                    
#endif // DX_NOT_SUPPORTED_FEATURE
                    
                    // Indicating that the GDI part of the driver can change
                    // gamma ramp while running in full-screen mode.
                    g_P3RX_D3DCaps.Caps2 |= D3DCAPS2_FULLSCREENGAMMA;

#if DX8_FULLSCREEN_FLIP_OR_DISCARD
                    // Indicates that the device can respect the ALPHABLENDENABLE 
                    // render state when fullscreen while using the FLIP or 
                    // DISCARD swap effect. COPY and COPYVSYNC swap effects work 
                    // whether or not this flag is set.
                    g_P3RX_D3DCaps.Caps3 = 
                                D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD;
#else
                    g_P3RX_D3DCaps.Caps3 = 0;
#endif
                    
                    // The following field can/should be left as 0 as the
                    // runtime will field them by itself.               
                    g_P3RX_D3DCaps.PresentationIntervals = 0;

#if DX_NOT_SUPPORTED_FEATURE
                    // If your hw/driver supports colored cursor without
                    // limitations then set these caps as below. We don't
                    // do this in our driver because we have a hw limitation
                    // of 16 colors on the cursor. WHQL tests therefore
                    // fail because of this limitation
                    g_P3RX_D3DCaps.CursorCaps = D3DCURSORCAPS_COLOR;   
                    
                    // Signal that the driver does support hw cursors
                    // both for hi resolution modes ( height >= 400) and
                    // for low resolution modes as well.
                    g_P3RX_D3DCaps.CursorCaps |= D3DCURSORCAPS_LOWRES;
#else
                    // We have some limitations (read above) in the Perm3 
                    // hardware so we're not supporting these caps here
                    g_P3RX_D3DCaps.CursorCaps = 0;                    
#endif                                        
                    // Miscellanneous settings new DX8 features as
                    // pointsprites, multistreaming, 3D textures, 
                    // pixelshaders and vertex shaders
                    g_P3RX_D3DCaps.MaxVertexIndex = 0x000FFFFF;
                    
#if DX8_POINTSPRITES                      
                    // Notify we can handle pointsprite size
                    g_P3RX_D3DCaps.FVFCaps |= D3DFVFCAPS_PSIZE;
                    // Notice that the MaxPointSize has to be at least 16
                    // per the DX8 specification for pointsprites.
                    g_P3RX_D3DCaps.MaxPointSize = P3_MAX_POINTSPRITE_SIZE;
#endif                    

                    // Any DX8 driver must declare it suppports 
                    // AT LEAST 1 stream. Otherwise its used as a DX7 driver.
                    g_P3RX_D3DCaps.MaxStreams = 1;
                    
                    g_P3RX_D3DCaps.MaxVertexBlendMatrixIndex = 0; 
                    
                    // NOTE: It is essential that the macros D3DVS_VERSION
                    // and D3DPS_VERSION be used to intialize the vertex
                    // and pixel shader version respecitively. The format
                    // of the version DWORD is complex so please don't try
                    // and build the version DWORD manually.
                    g_P3RX_D3DCaps.VertexShaderVersion = D3DVS_VERSION(0, 0);
                    g_P3RX_D3DCaps.PixelShaderVersion  = D3DPS_VERSION(0, 0);

#if DX8_3DTEXTURES                     
                    g_P3RX_D3DCaps.MaxVolumeExtent = 2048;
#endif                    
        
                    // D3DPTFILTERCAPS for IDirect3DCubeTexture8's                    
                    g_P3RX_D3DCaps.CubeTextureFilterCaps = 0;      

                    // D3DLINECAPS
                    g_P3RX_D3DCaps.LineCaps = D3DLINECAPS_TEXTURE  |
                                              D3DLINECAPS_ZTEST    |
                                              D3DLINECAPS_BLEND    |
                                              D3DLINECAPS_ALPHACMP |
                                              D3DLINECAPS_FOG;
                                               
                    // max number of primitives per DrawPrimitive call
                    g_P3RX_D3DCaps.MaxPrimitiveCount = 0x000FFFFF;         
                     // max value of pixel shade
#if DX9_DDI
                    g_P3RX_D3DCaps.PixelShader1xMaxValue = 0;       
#elif DX8_DDI
                    g_P3RX_D3DCaps.MaxPixelShaderValue = 0;       
#endif // DX8_DDI
                    // max stride for SetStreamSource
                    // we will use this defualt value for now
                    g_P3RX_D3DCaps.MaxStreamStride = 256;    
                    // number of vertex shader constant 
                    g_P3RX_D3DCaps.MaxVertexShaderConst = 0;       

#if DX8_3DTEXTURES 
                    g_P3RX_D3DCaps.VolumeTextureFilterCaps = 
                                           D3DPTFILTERCAPS_MINFPOINT |
                                           D3DPTFILTERCAPS_MAGFPOINT;
                                           
                    g_P3RX_D3DCaps.VolumeTextureAddressCaps =     
                                           D3DPTADDRESSCAPS_WRAP     |
                                           D3DPTADDRESSCAPS_MIRROR   |
                                           D3DPTADDRESSCAPS_CLAMP;
#endif // DX8_3DTEXTURES

                    // It should be noted that the dwExpectedSize field
                    // of DD_GETDRIVERINFODATA is not used for
                    // GetDriverInfo2 calls and should be ignored.
#if DX9_DDI
                    if (pgdi2->dwType == D3DGDI2_TYPE_GETD3DCAPS9)
                    {
                        // Indicate that Perm3 is DX9 capable
                        g_P3RX_D3DCaps.DevCaps2 = D3DDEVCAPS2_STREAMOFFSET;

                        // Must be at least one for any DX9 driver
                        // Some drivers may support more than one 
                        // if they support MRTs
                        g_P3RX_D3DCaps.NumSimultaneousRTs = 1;
#if DX9_AUTOGENMIPMAP
                        // Indicate that Perm3 can generate mipmap sublevels
                        g_P3RX_D3DCaps.Caps2 |= D3DCAPS2_CANAUTOGENMIPMAP;
#endif // DX9_AUTOGENMIPMAP
#if DX9_RTZMAN
                        g_P3RX_D3DCaps.Caps3 |= D3DCAPS3_MANAGEDDEVICE;
#endif // DX9_RTZMAN
                        // Fill in new DX9 caps
#if DX9_SCISSORRECT
                        g_P3RX_D3DCaps.RasterCaps |= D3DPRASTERCAPS_SCISSORTEST;
#endif // DX9_SCISSORRECT

                        // DX9 driver should not support LINEPATTERNREP
                        g_P3RX_D3DCaps.PrimitiveMiscCaps &= (~D3DPMISCCAPS_LINEPATTERNREP);

#if DX9_ANTIALIASEDLINE
                        g_P3RX_D3DCaps.LineCaps |= D3DLINECAPS_ANTIALIAS;
#endif // DX9_ANTIALIASEDLINE

#if DX9_DP2BLT
                        // Supports both POINT and LINEAR filter
                        g_P3RX_D3DCaps.StretchRectFilterCaps =
                            (D3DPTFILTERCAPS_MAGFPOINT  | 
                             D3DPTFILTERCAPS_MAGFLINEAR |
                             D3DPTFILTERCAPS_MINFPOINT  |
                             D3DPTFILTERCAPS_MINFLINEAR);
#endif

                        // Decide the size of data to return
                        copySize = min(sizeof(D3DCAPS9),
                                       pgdi2->dwExpectedSize);
                    }
                    else
#endif // DX9_DDI
                    {
                        copySize = min(sizeof(D3DCAPS8), 
                                       pgdi2->dwExpectedSize);
                    }
                    memcpy(lpData->lpvData, &g_P3RX_D3DCaps, copySize);
                    lpData->dwActualSize = copySize;
                    lpData->ddRVal       = DD_OK;
                }
                break;

#if DX9_ASYNC_NOTIFICATIONS    
            case D3DGDI2_TYPE_GETD3DQUERYCOUNT:
                {
                    // Its a request for the number of asynchronous queries
                    // we support. Get the extended data structure so
                    // we can fill in the format count field.
                    pqcd = (DD_GETD3DQUERYCOUNTDATA*)pgdi2;
                    pqcd->dwNumQueries = DX9_QUERY_COUNT;

                    lpData->dwActualSize = sizeof(DD_GETD3DQUERYCOUNTDATA);
                    lpData->ddRVal       = DD_OK;
                }
                break;
            case D3DGDI2_TYPE_GETD3DQUERY:
                {
                    // Its a request for a particular asynchronous query 
                    // we support. Get the extended data structure so 
                    // we can fill in the format field.
                    pd3dq = (DD_GETD3DQUERYDATA*)pgdi2;
                    
                    // Copy over the query type from the supported queries table.
                    if (pd3dq->dwQueryIndex < DX9_QUERY_COUNT)
                    {
                        pd3dq->QueryType     =  DX9QueryTable[pd3dq->dwQueryIndex];
                        lpData->dwActualSize = sizeof(DD_GETD3DQUERYDATA);
                        lpData->ddRVal       = DD_OK;
                    }
                    else
                    {
                        // The query type index is out of boundary
                        lpData->dwActualSize = 0;
                        lpData->ddRVal       = DDERR_INVALIDPARAMS;
                    }

                }
                break;
#endif // DX9_ASYNC_NOTIFICATIONS

#if DX9_DDI
            case D3DGDI2_TYPE_GETDDIVERSION:
                {
                    // Return DDI version to the runtime
                    pgddiver = (DD_GETDDIVERSIONDATA*)pgdi2;
                    pgddiver->dwDDIVersion = DX9_DDI_VERSION;

                    lpData->dwActualSize = sizeof(DD_GETDDIVERSIONDATA);
                    lpData->ddRVal       = DD_OK;
                }
                break;
#endif

            default:
                // Default behavior for any other type.
                lpData->dwActualSize = 0;
                lpData->ddRVal       = DDERR_CURRENTLYNOTAVAIL;
                break;
            }
        }
        else
#endif // DX8_DDI
        {
#if WNT_DDRAW
#if DX7_STEREO
            PDD_STEREOMODE pDDStereoMode;

            // Permedia3 supports all modes as stereo modes.
            // for test purposes, we restrict them to something
            // larger than 320x240

            //
            // note: this GUID_DDStereoMode is only used on NT to
            // report stereo modes. There is no need to implement
            // it in win9x drivers. Win9x drivers report stereo
            // modes by setting the DDMODEINFO_STEREO bit in the
            // dwFlags member of the DDHALMODEINFO structure.
            // It is also recommended to report DDMODEINFO_MAXREFRESH
            // for stereo modes when running under a runtime >= DX7 to
            // allow applications to select higher refresh rates for
            // stereo modes.
            //

            if (lpData->dwExpectedSize >= sizeof(PDD_STEREOMODE))
            {
                pDDStereoMode = (PDD_STEREOMODE)lpData->lpvData;

                pDDStereoMode->bSupported =
                    _DD_bIsStereoMode(pThisDisplay,
                                      pDDStereoMode->dwWidth,
                                      pDDStereoMode->dwHeight,
                                      pDDStereoMode->dwBpp);

                DISPDBG((DBGLVL,"  GUID_DDStereoMode(%d,%d,%d,%d=%d)",
                    pDDStereoMode->dwWidth,
                    pDDStereoMode->dwHeight,
                    pDDStereoMode->dwBpp,
                    pDDStereoMode->dwRefreshRate,
                    pDDStereoMode->bSupported));

                lpData->dwActualSize = sizeof(DD_STEREOMODE);
                lpData->ddRVal = DD_OK;        
            }
#endif // DX7_STEREO
#endif // WNT_DDRAW
        }
    }

    DBG_EXIT(_D3DGetDriverInfo, 0);
    
} // _D3DGetDriverInfo

//-----------------------------------------------------------------------------
//
// __D3D_BuildTextureFormatsP3
//
// Fills a list of texture formats in.  
// Returns the number of formats specified.  
//
//-----------------------------------------------------------------------------
void 
__D3D_BuildTextureFormatsP3(
    P3_THUNKEDDATA *pThisDisplay, 
    DDSURFACEDESC TexFmt[MAX_TEXTURE_FORMAT],
    DWORD *pNumIntTextures,
    DWORD *pNumExtTextures)
{
    int i;

    // Initialise the defaults
    for (i = 0; i < MAX_TEXTURE_FORMAT; i++)
    {
        TexFmt[i].dwSize = sizeof(DDSURFACEDESC);
        TexFmt[i].dwFlags = DDSD_CAPS | DDSD_PIXELFORMAT;
        TexFmt[i].dwHeight = 0;
        TexFmt[i].dwWidth = 0;
        TexFmt[i].lPitch = 0;
        TexFmt[i].dwBackBufferCount = 0;
        TexFmt[i].dwZBufferBitDepth = 0;
        TexFmt[i].dwReserved = 0;
        TexFmt[i].lpSurface = 0;

        TexFmt[i].ddckCKDestOverlay.dwColorSpaceLowValue = 0;
        TexFmt[i].ddckCKDestOverlay.dwColorSpaceHighValue = 0;
        
        TexFmt[i].ddckCKDestBlt.dwColorSpaceLowValue = 0;
        TexFmt[i].ddckCKDestBlt.dwColorSpaceHighValue = 0;

        TexFmt[i].ddckCKSrcOverlay.dwColorSpaceLowValue = 0;
        TexFmt[i].ddckCKSrcOverlay.dwColorSpaceHighValue = 0;

        TexFmt[i].ddckCKSrcBlt.dwColorSpaceLowValue = 0;
        TexFmt[i].ddckCKSrcBlt.dwColorSpaceHighValue = 0;
        TexFmt[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    }
    i = 0;

    // 5:5:5 RGB
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_RGB;
    TexFmt[i].ddpfPixelFormat.dwRGBBitCount = 16;
    TexFmt[i].ddpfPixelFormat.dwRBitMask = 0x7C00;
    TexFmt[i].ddpfPixelFormat.dwGBitMask = 0x03E0;
    TexFmt[i].ddpfPixelFormat.dwBBitMask = 0x001F;
    TexFmt[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0;
    i++;

    // 8:8:8 RGB
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_RGB;
    TexFmt[i].ddpfPixelFormat.dwRGBBitCount = 32;
    TexFmt[i].ddpfPixelFormat.dwRBitMask = 0xff0000;
    TexFmt[i].ddpfPixelFormat.dwGBitMask = 0xff00;
    TexFmt[i].ddpfPixelFormat.dwBBitMask = 0xff;
    TexFmt[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0;
    i++;

    // 1:5:5:5 ARGB 
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    TexFmt[i].ddpfPixelFormat.dwRGBBitCount = 16;
    TexFmt[i].ddpfPixelFormat.dwRBitMask = 0x7C00;
    TexFmt[i].ddpfPixelFormat.dwGBitMask = 0x03E0;
    TexFmt[i].ddpfPixelFormat.dwBBitMask = 0x001F;
    TexFmt[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0x8000;
    i++;        
    
    // 4:4:4:4 ARGB
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    TexFmt[i].ddpfPixelFormat.dwRGBBitCount = 16;
    TexFmt[i].ddpfPixelFormat.dwRBitMask = 0xf00;
    TexFmt[i].ddpfPixelFormat.dwGBitMask = 0xf0;
    TexFmt[i].ddpfPixelFormat.dwBBitMask = 0xf;
    TexFmt[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0xf000;
    i++;
    
    // 8:8:8:8 ARGB 
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    TexFmt[i].ddpfPixelFormat.dwRGBBitCount = 32;
    TexFmt[i].ddpfPixelFormat.dwRBitMask = 0xff0000;
    TexFmt[i].ddpfPixelFormat.dwGBitMask = 0xff00;
    TexFmt[i].ddpfPixelFormat.dwBBitMask = 0xff;
    TexFmt[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
    i++;

    // 5:6:5
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_RGB;
    TexFmt[i].ddpfPixelFormat.dwRGBBitCount = 16;
    TexFmt[i].ddpfPixelFormat.dwRBitMask = 0xF800;
    TexFmt[i].ddpfPixelFormat.dwGBitMask = 0x07E0;
    TexFmt[i].ddpfPixelFormat.dwBBitMask = 0x001F;
    TexFmt[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0;
    i++;

    // A4L4
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_LUMINANCE | DDPF_ALPHAPIXELS;
    TexFmt[i].ddpfPixelFormat.dwLuminanceBitCount = 8;
    TexFmt[i].ddpfPixelFormat.dwLuminanceBitMask = 0x0F;
    TexFmt[i].ddpfPixelFormat.dwLuminanceAlphaBitMask = 0xF0;
    i++;

    // A8L8
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_LUMINANCE | DDPF_ALPHAPIXELS;
    TexFmt[i].ddpfPixelFormat.dwLuminanceBitCount = 16;
    TexFmt[i].ddpfPixelFormat.dwLuminanceBitMask = 0x00FF;
    TexFmt[i].ddpfPixelFormat.dwLuminanceAlphaBitMask = 0xFF00;
    i++;


    // L8 
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_LUMINANCE;
    TexFmt[i].ddpfPixelFormat.dwLuminanceBitCount = 8;
    TexFmt[i].ddpfPixelFormat.dwLuminanceBitMask = 0xFF;
    TexFmt[i].ddpfPixelFormat.dwLuminanceAlphaBitMask = 0;
    i++;

    *pNumExtTextures = i;

    // 
    // Formats initialized after point is not exposed to DX7 and below apps
    //

#if DX8_PRIVTEXFORMAT
    // P3TT
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = FOURCC_P3TT;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
    TexFmt[i].ddpfPixelFormat.dwYUVBitCount = 0;
    i++;
#endif

#if DX7_PALETTETEXTURE
    // P8 
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
    TexFmt[i].ddpfPixelFormat.dwRGBBitCount = 8;
    TexFmt[i].ddpfPixelFormat.dwRBitMask = 0x00000000;
    TexFmt[i].ddpfPixelFormat.dwGBitMask = 0x00000000;
    TexFmt[i].ddpfPixelFormat.dwBBitMask = 0x00000000;
    TexFmt[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0x00000000;
    i++;

    // For our DX8 caps list we DO list paletted texture formats as supported.
    // __SUR_bCheckTextureFormat uses what is returned in *pNumIntTextures to
    // make sure we can create a paletted texture when asked by DX8 or above
    // apps.

    // The whole reason behind this approach is because in legacy DX interfaces
    // the TextureSwap method causes the surface and palette handle association 
    // to be lost. While there are some ugly and tricky ways around this (as in 
    // the Permedia2 sample driver), and there is no rational way to fix the 
    // problem. 
    
#endif
    
    // Return the number of texture formats internal to the driver
    *pNumIntTextures = i;
    
} // __D3D_BuildTextureFormatsP3

#if DX8_DDI
//-----------------------------------------------------------------------------
//
// __D3D_Fill_DX8Caps
//
// Fills the D3DCAPS8 structure of a DX8 driver from legacy caps structures.
//
//-----------------------------------------------------------------------------
void 
__D3D_Fill_DX8Caps(
    D3DCAPS8 *pd3d8caps,
    D3DDEVICEDESC_V1 *pDeviceDesc,
    D3DHAL_D3DEXTENDEDCAPS *pD3DEC,
    DDHALINFO *pDDHALInfo)
{

    pd3d8caps->Caps  = pDDHALInfo->ddCaps.dwCaps;
    pd3d8caps->Caps2 = pDDHALInfo->ddCaps.dwCaps2;  

    pd3d8caps->DevCaps           = pDeviceDesc->dwDevCaps;

    pd3d8caps->PrimitiveMiscCaps = pDeviceDesc->dpcTriCaps.dwMiscCaps;
    pd3d8caps->RasterCaps        = pDeviceDesc->dpcTriCaps.dwRasterCaps;
    pd3d8caps->ZCmpCaps          = pDeviceDesc->dpcTriCaps.dwZCmpCaps;
    pd3d8caps->SrcBlendCaps      = pDeviceDesc->dpcTriCaps.dwSrcBlendCaps;
    pd3d8caps->DestBlendCaps     = pDeviceDesc->dpcTriCaps.dwDestBlendCaps;
    pd3d8caps->AlphaCmpCaps      = pDeviceDesc->dpcTriCaps.dwAlphaCmpCaps;
    pd3d8caps->ShadeCaps         = pDeviceDesc->dpcTriCaps.dwShadeCaps;
    pd3d8caps->TextureCaps       = pDeviceDesc->dpcTriCaps.dwTextureCaps;
    pd3d8caps->TextureFilterCaps = pDeviceDesc->dpcTriCaps.dwTextureFilterCaps;      
    pd3d8caps->TextureAddressCaps= pDeviceDesc->dpcTriCaps.dwTextureAddressCaps;

    pd3d8caps->MaxTextureWidth   = pD3DEC->dwMaxTextureWidth;
    pd3d8caps->MaxTextureHeight  = pD3DEC->dwMaxTextureHeight;
    
    pd3d8caps->MaxTextureRepeat  = pD3DEC->dwMaxTextureRepeat;
    pd3d8caps->MaxTextureAspectRatio = pD3DEC->dwMaxTextureAspectRatio;
    pd3d8caps->MaxAnisotropy     = pD3DEC->dwMaxAnisotropy;
    pd3d8caps->MaxVertexW        = pD3DEC->dvMaxVertexW;

    pd3d8caps->GuardBandLeft     = pD3DEC->dvGuardBandLeft;
    pd3d8caps->GuardBandTop      = pD3DEC->dvGuardBandTop;
    pd3d8caps->GuardBandRight    = pD3DEC->dvGuardBandRight;
    pd3d8caps->GuardBandBottom   = pD3DEC->dvGuardBandBottom;

    pd3d8caps->ExtentsAdjust     = pD3DEC->dvExtentsAdjust;
    pd3d8caps->StencilCaps       = pD3DEC->dwStencilCaps;

    pd3d8caps->FVFCaps           = pD3DEC->dwFVFCaps;
    pd3d8caps->TextureOpCaps     = pD3DEC->dwTextureOpCaps;
    pd3d8caps->MaxTextureBlendStages     = pD3DEC->wMaxTextureBlendStages;
    pd3d8caps->MaxSimultaneousTextures   = pD3DEC->wMaxSimultaneousTextures;

    pd3d8caps->VertexProcessingCaps      = pD3DEC->dwVertexProcessingCaps;
    pd3d8caps->MaxActiveLights           = pD3DEC->dwMaxActiveLights;
    pd3d8caps->MaxUserClipPlanes         = pD3DEC->wMaxUserClipPlanes;
    pd3d8caps->MaxVertexBlendMatrices    = pD3DEC->wMaxVertexBlendMatrices;

} // __D3D_Fill_DX8Caps
#endif // DX8_DDI



