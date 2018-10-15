/*==========================================================================
 *
 *  Copyright (C) 1995-1996 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:   d3dnthal.h
 *  Content:    Direct3D HAL include file for NT
 *
 ***************************************************************************/

#ifndef _D3DNTHAL_H_
#define _D3DNTHAL_H_

#include <ddrawint.h>
#ifndef _WINDOWS_
#define _WINDOWS_
#include <d3dtypes.h>
#include <d3dcaps.h>
#undef _WINDOWS_
#else
#include <d3dtypes.h>
#include <d3dcaps.h>
#endif

/*
 * If the HAL driver does not implement clipping, it must reserve at least
 * this much space at the end of the LocalVertexBuffer for use by the HEL
 * clipping.  I.e. the vertex buffer contain dwNumVertices+dwNumClipVertices
 * vertices.  No extra space is needed by the HEL clipping in the
 * LocalHVertexBuffer.
 */
#define D3DNTHAL_NUMCLIPVERTICES    20

/*
 * If no dwNumVertices is given, this is what will be used.
 */
#define D3DNTHAL_DEFAULT_TL_NUM ((32 * 1024) / sizeof (D3DTLVERTEX))
#define D3DNTHAL_DEFAULT_H_NUM  ((32 * 1024) / sizeof (D3DHVERTEX))

/*
 * Description for a device.
 * This is used to describe a device that is to be created or to query
 * the current device.
 *
 * For DX5 and subsequent runtimes, D3DNTDEVICEDESC is a user-visible
 * structure that is not seen by the device drivers. The runtime
 * stitches a D3DNTDEVICEDESC together using the D3DNTDEVICEDESC_V1
 * embedded in the GLOBALDRIVERDATA and the extended caps queried
 * from the driver using GetDriverInfo.
 */

typedef struct _D3DNTHALDeviceDesc_V1 {
    DWORD        dwSize;             /* Size of D3DNTHALDEVICEDESC_V1 structure */
    DWORD        dwFlags;                /* Indicates which fields have valid data */
    D3DCOLORMODEL    dcmColorModel;      /* Color model of device */
    DWORD        dwDevCaps;              /* Capabilities of device */
    D3DTRANSFORMCAPS dtcTransformCaps;       /* Capabilities of transform */
    BOOL         bClipping;              /* Device can do 3D clipping */
    D3DLIGHTINGCAPS  dlcLightingCaps;        /* Capabilities of lighting */
    D3DPRIMCAPS      dpcLineCaps;
    D3DPRIMCAPS      dpcTriCaps;
    DWORD        dwDeviceRenderBitDepth; /* One of DDBB_8, 16, etc.. */
    DWORD        dwDeviceZBufferBitDepth;/* One of DDBD_16, 32, etc.. */
    DWORD        dwMaxBufferSize;        /* Maximum execute buffer size */
    DWORD        dwMaxVertexCount;       /* Maximum vertex count */
} D3DNTHALDEVICEDESC_V1, *LPD3DNTHALDEVICEDESC_V1;

#define D3DNTHALDEVICEDESCSIZE_V1 (sizeof(D3DNTHALDEVICEDESC_V1))

/*
 * This is equivalent to the D3DNTDEVICEDESC understood by DX5, available only
 * from DX6. It is the same as D3DNTDEVICEDESC structure in DX5.
 * D3DNTDEVICEDESC is still the user-visible structure that is not seen by the
 * device drivers. The runtime stitches a D3DNTDEVICEDESC together using the
 * D3DNTDEVICEDESC_V1 embedded in the GLOBALDRIVERDATA and the extended caps
 * queried from the driver using GetDriverInfo.
 */

typedef struct _D3DNTHALDeviceDesc_V2 {
    DWORD        dwSize;             /* Size of D3DNTDEVICEDESC structure */
    DWORD        dwFlags;                /* Indicates which fields have valid data */
    D3DCOLORMODEL    dcmColorModel;      /* Color model of device */
    DWORD        dwDevCaps;              /* Capabilities of device */
    D3DTRANSFORMCAPS dtcTransformCaps;       /* Capabilities of transform */
    BOOL         bClipping;              /* Device can do 3D clipping */
    D3DLIGHTINGCAPS  dlcLightingCaps;        /* Capabilities of lighting */
    D3DPRIMCAPS      dpcLineCaps;
    D3DPRIMCAPS      dpcTriCaps;
    DWORD            dwDeviceRenderBitDepth; /* One of DDBD_16, etc.. */
    DWORD        dwDeviceZBufferBitDepth;/* One of DDBD_16, 32, etc.. */
    DWORD        dwMaxBufferSize;        /* Maximum execute buffer size */
    DWORD        dwMaxVertexCount;       /* Maximum vertex count */

    DWORD        dwMinTextureWidth, dwMinTextureHeight;
    DWORD        dwMaxTextureWidth, dwMaxTextureHeight;
    DWORD        dwMinStippleWidth, dwMaxStippleWidth;
    DWORD        dwMinStippleHeight, dwMaxStippleHeight;

} D3DNTHALDEVICEDESC_V2, *LPD3DNTHALDEVICEDESC_V2;

#define D3DNTHALDEVICEDESCSIZE_V2 (sizeof(D3DNTHALDEVICEDESC_V2))

#if(DIRECT3D_VERSION >= 0x0700)
/*
 * This is equivalent to the D3DNTDEVICEDESC understood by DX6, available only
 * from DX6. It is the same as D3DNTDEVICEDESC structure in DX6.
 * D3DNTDEVICEDESC is still the user-visible structure that is not seen by the
 * device drivers. The runtime stitches a D3DNTDEVICEDESC together using the
 * D3DNTDEVICEDESC_V1 embedded in the GLOBALDRIVERDATA and the extended caps
 * queried from the driver using GetDriverInfo.
 */

typedef struct _D3DNTDeviceDesc_V3 {
    DWORD        dwSize;             /* Size of D3DNTDEVICEDESC structure */
    DWORD        dwFlags;                /* Indicates which fields have valid data */
    D3DCOLORMODEL    dcmColorModel;      /* Color model of device */
    DWORD        dwDevCaps;              /* Capabilities of device */
    D3DTRANSFORMCAPS dtcTransformCaps;       /* Capabilities of transform */
    BOOL         bClipping;              /* Device can do 3D clipping */
    D3DLIGHTINGCAPS  dlcLightingCaps;        /* Capabilities of lighting */
    D3DPRIMCAPS      dpcLineCaps;
    D3DPRIMCAPS      dpcTriCaps;
    DWORD            dwDeviceRenderBitDepth; /* One of DDBD_16, etc.. */
    DWORD        dwDeviceZBufferBitDepth;/* One of DDBD_16, 32, etc.. */
    DWORD        dwMaxBufferSize;        /* Maximum execute buffer size */
    DWORD        dwMaxVertexCount;       /* Maximum vertex count */

    DWORD        dwMinTextureWidth, dwMinTextureHeight;
    DWORD        dwMaxTextureWidth, dwMaxTextureHeight;
    DWORD        dwMinStippleWidth, dwMaxStippleWidth;
    DWORD        dwMinStippleHeight, dwMaxStippleHeight;

    DWORD       dwMaxTextureRepeat;
    DWORD       dwMaxTextureAspectRatio;
    DWORD       dwMaxAnisotropy;
    D3DVALUE    dvGuardBandLeft;
    D3DVALUE    dvGuardBandTop;
    D3DVALUE    dvGuardBandRight;
    D3DVALUE    dvGuardBandBottom;
    D3DVALUE    dvExtentsAdjust;
    DWORD       dwStencilCaps;
    DWORD       dwFVFCaps;  /* low 4 bits: 0 implies TLVERTEX only, 1..8 imply FVF aware */
    DWORD       dwTextureOpCaps;
    WORD        wMaxTextureBlendStages;
    WORD        wMaxSimultaneousTextures;
} D3DNTDEVICEDESC_V3, *LPD3DNTDEVICEDESC_V3;

#define D3DNTDEVICEDESCSIZE_V3 (sizeof(D3DNTDEVICEDESC_V3))
#endif /* DIRECT3D_VERSION >= 0x0700 */
/* --------------------------------------------------------------
 * Instantiated by the HAL driver on driver connection.
 *
 * Regarding dwNumVertices, specify 0 if you are relying on the HEL to do
 * everything and you do not need the resultant TLVertex buffer to reside
 * in device memory.
 * The HAL driver will be asked to allocate dwNumVertices + dwNumClipVertices
 * in the case described above.
 */
typedef struct _D3DNTHAL_GLOBALDRIVERDATA {
    DWORD       dwSize;         // Size of this structure
    D3DNTHALDEVICEDESC_V1 hwCaps;       // Capabilities of the hardware
    DWORD       dwNumVertices;      // see following comment
    DWORD       dwNumClipVertices;  // see following comment
    DWORD       dwNumTextureFormats;    // Number of texture formats
    LPDDSURFACEDESC lpTextureFormats;   // Pointer to texture formats
} D3DNTHAL_GLOBALDRIVERDATA;
typedef D3DNTHAL_GLOBALDRIVERDATA *LPD3DNTHAL_GLOBALDRIVERDATA;

#define D3DNTHAL_GLOBALDRIVERDATASIZE (sizeof(D3DNTHAL_GLOBALDRIVERDATA))

#if(DIRECT3D_VERSION >= 0x0700)
/* --------------------------------------------------------------
 * Extended caps introduced with DX5 and queried with
 * GetDriverInfo (GUID_D3DExtendedCaps).
 */
typedef struct _D3DNTHAL_D3DDX6EXTENDEDCAPS {
    DWORD       dwSize;         // Size of this structure

    DWORD       dwMinTextureWidth, dwMaxTextureWidth;
    DWORD       dwMinTextureHeight, dwMaxTextureHeight;
    DWORD       dwMinStippleWidth, dwMaxStippleWidth;
    DWORD       dwMinStippleHeight, dwMaxStippleHeight;

    /* fields added for DX6 */
    DWORD       dwMaxTextureRepeat;
    DWORD       dwMaxTextureAspectRatio;
    DWORD       dwMaxAnisotropy;
    D3DVALUE    dvGuardBandLeft;
    D3DVALUE    dvGuardBandTop;
    D3DVALUE    dvGuardBandRight;
    D3DVALUE    dvGuardBandBottom;
    D3DVALUE    dvExtentsAdjust;
    DWORD       dwStencilCaps;
    DWORD       dwFVFCaps;  /* low 4 bits: 0 implies TLVERTEX only, 1..8 imply FVF aware */
    DWORD       dwTextureOpCaps;
    WORD        wMaxTextureBlendStages;
    WORD        wMaxSimultaneousTextures;

} D3DNTHAL_D3DDX6EXTENDEDCAPS;
#endif /* DIRECT3D_VERSION >= 0x0700 */

/* --------------------------------------------------------------
 * Extended caps introduced with DX5 and queried with
 * GetDriverInfo (GUID_D3DExtendedCaps).
 */
typedef struct _D3DNTHAL_D3DEXTENDEDCAPS {
    DWORD       dwSize;         // Size of this structure
    DWORD       dwMinTextureWidth, dwMaxTextureWidth;
    DWORD       dwMinTextureHeight, dwMaxTextureHeight;
    DWORD       dwMinStippleWidth, dwMaxStippleWidth;
    DWORD       dwMinStippleHeight, dwMaxStippleHeight;

    /* fields added for DX6 */
    DWORD       dwMaxTextureRepeat;
    DWORD       dwMaxTextureAspectRatio;
    DWORD       dwMaxAnisotropy;
    D3DVALUE    dvGuardBandLeft;
    D3DVALUE    dvGuardBandTop;
    D3DVALUE    dvGuardBandRight;
    D3DVALUE    dvGuardBandBottom;
    D3DVALUE    dvExtentsAdjust;
    DWORD       dwStencilCaps;
    DWORD       dwFVFCaps;  /* 0 implies TLVERTEX only, 1..8 imply full FVF aware */
    DWORD       dwTextureOpCaps;
    WORD        wMaxTextureBlendStages;
    WORD        wMaxSimultaneousTextures;
#if(DIRECT3D_VERSION >= 0x0700)
    /* fields added for DX7 */
    DWORD       dwMaxActiveLights;
    D3DVALUE    dvMaxVertexW;

    WORD        wMaxUserClipPlanes;
    WORD        wMaxVertexBlendMatrices;

    DWORD       dwVertexProcessingCaps;

    DWORD       dwReserved1;
    DWORD       dwReserved2;
    DWORD       dwReserved3;
    DWORD       dwReserved4;
#endif /* DIRECT3D_VERSION >= 0x0700 */
} D3DNTHAL_D3DEXTENDEDCAPS;

typedef D3DNTHAL_D3DEXTENDEDCAPS *LPD3DNTHAL_D3DEXTENDEDCAPS;

#define D3DNTHAL_D3DEXTENDEDCAPSSIZE (sizeof(D3DNTHAL_D3DEXTENDEDCAPS))

// This is a temporary fix to make older NT drivers to compile
#define dvVertexProcessingCaps dwVertexProcessingCaps

#if(DIRECT3D_VERSION >= 0x0700)
typedef D3DNTHAL_D3DDX6EXTENDEDCAPS *LPD3DNTHAL_D3DDX6EXTENDEDCAPS;
#define D3DNTHAL_D3DDX6EXTENDEDCAPSSIZE (sizeof(D3DNTHAL_D3DDX6EXTENDEDCAPS))
#endif /* DIRECT3D_VERSION >= 0x0700 */

/* --------------------------------------------------------------
 * Argument to the HAL functions.
 */

typedef ULONG_PTR D3DINTHAL_BUFFERHANDLE, *LPD3DINTHAL_BUFFERHANDLE;

typedef struct _D3DNTHAL_CONTEXTCREATEDATA {
union
{
    PDD_DIRECTDRAW_GLOBAL lpDDGbl;  // in:  obsolete
    PDD_DIRECTDRAW_LOCAL lpDDLcl;   // in:  Driver struct
};
union
{
    PDD_SURFACE_LOCAL   lpDDS;      // in:  obsolete
    PDD_SURFACE_LOCAL   lpDDSLcl;   // in:  Surface to be used as target
};
union
{
    PDD_SURFACE_LOCAL   lpDDSZ;     // in:  obsolete
    PDD_SURFACE_LOCAL   lpDDSZLcl;  // in:  Surface to be used as Z
};
    DWORD               dwPID;      // in:  Current process id
    ULONG_PTR           dwhContext; // in/out: Context handle
    HRESULT             ddrval;     // out: Return value
} D3DNTHAL_CONTEXTCREATEDATA;
typedef D3DNTHAL_CONTEXTCREATEDATA *LPD3DNTHAL_CONTEXTCREATEDATA;

typedef struct _D3DNTHAL_CONTEXTDESTROYDATA {
    ULONG_PTR    dwhContext; // in:  Context handle
    HRESULT     ddrval;     // out: Return value
} D3DNTHAL_CONTEXTDESTROYDATA;
typedef D3DNTHAL_CONTEXTDESTROYDATA *LPD3DNTHAL_CONTEXTDESTROYDATA;

typedef struct _D3DNTHAL_CONTEXTDESTROYALLDATA {
    DWORD       dwPID;      // in:  Process id to destroy contexts for
    HRESULT     ddrval;     // out: Return value
} D3DNTHAL_CONTEXTDESTROYALLDATA;
typedef D3DNTHAL_CONTEXTDESTROYALLDATA *LPD3DNTHAL_CONTEXTDESTROYALLDATA;

typedef struct _D3DNTHAL_SCENECAPTUREDATA {
    ULONG_PTR    dwhContext; // in:  Context handle
    DWORD       dwFlag;     // in:  Indicates beginning or end
    HRESULT     ddrval;     // out: Return value
} D3DNTHAL_SCENECAPTUREDATA;
typedef D3DNTHAL_SCENECAPTUREDATA *LPD3DNTHAL_SCENECAPTUREDATA;

typedef struct _D3DNTHAL_TEXTURECREATEDATA {
    ULONG_PTR    dwhContext; // in:  Context handle
    HANDLE          hDDS;       // in:  Handle to surface object
    ULONG_PTR    dwHandle;   // out: Handle to texture
    HRESULT     ddrval;     // out: Return value
} D3DNTHAL_TEXTURECREATEDATA;
typedef D3DNTHAL_TEXTURECREATEDATA *LPD3DNTHAL_TEXTURECREATEDATA;

typedef struct _D3DNTHAL_TEXTUREDESTROYDATA {
    ULONG_PTR    dwhContext; // in:  Context handle
    ULONG_PTR    dwHandle;   // in:  Handle to texture
    HRESULT     ddrval;     // out: Return value
} D3DNTHAL_TEXTUREDESTROYDATA;
typedef D3DNTHAL_TEXTUREDESTROYDATA *LPD3DNTHAL_TEXTUREDESTROYDATA;

typedef struct _D3DNTHAL_TEXTURESWAPDATA {
    ULONG_PTR    dwhContext; // in:  Context handle
    ULONG_PTR    dwHandle1;  // in:  Handle to texture 1
    ULONG_PTR    dwHandle2;  // in:  Handle to texture 2
    HRESULT     ddrval;     // out: Return value
} D3DNTHAL_TEXTURESWAPDATA;
typedef D3DNTHAL_TEXTURESWAPDATA *LPD3DNTHAL_TEXTURESWAPDATA;

typedef struct _D3DNTHAL_TEXTUREGETSURFDATA {
    ULONG_PTR    dwhContext; // in:  Context handle
    HANDLE      hDDS;       // out: Handle to surface object
    ULONG_PTR    dwHandle;   // in:  Handle to texture
    HRESULT     ddrval;     // out: Return value
} D3DNTHAL_TEXTUREGETSURFDATA;
typedef D3DNTHAL_TEXTUREGETSURFDATA *LPD3DNTHAL_TEXTUREGETSURFDATA;

/* --------------------------------------------------------------
 * Flags for the data parameters.
 */

/*
 * SceneCapture()
 * This is used as an indication to the driver that a scene is about to
 * start or end, and that it should capture data if required.
 */
#define D3DNTHAL_SCENE_CAPTURE_START    0x00000000L
#define D3DNTHAL_SCENE_CAPTURE_END  0x00000001L

/* --------------------------------------------------------------
 * Return values from HAL functions.
 */

/*
 * The context passed in was bad.
 */
#define D3DNTHAL_CONTEXT_BAD        0x000000200L

/*
 * No more contexts left.
 */
#define D3DNTHAL_OUTOFCONTEXTS      0x000000201L

/* --------------------------------------------------------------
 * Direct3D HAL Table.
 * Instantiated by the HAL driver on connection.
 *
 * Calls take the form of:
 *  retcode = HalCall(HalCallData* lpData);
 */

typedef DWORD   (APIENTRY *LPD3DNTHAL_CONTEXTCREATECB)  (LPD3DNTHAL_CONTEXTCREATEDATA);
typedef DWORD   (APIENTRY *LPD3DNTHAL_CONTEXTDESTROYCB) (LPD3DNTHAL_CONTEXTDESTROYDATA);
typedef DWORD   (APIENTRY *LPD3DNTHAL_CONTEXTDESTROYALLCB) (LPD3DNTHAL_CONTEXTDESTROYALLDATA);
typedef DWORD   (APIENTRY *LPD3DNTHAL_SCENECAPTURECB)   (LPD3DNTHAL_SCENECAPTUREDATA);
typedef DWORD   (APIENTRY *LPD3DNTHAL_TEXTURECREATECB)  (LPD3DNTHAL_TEXTURECREATEDATA);
typedef DWORD   (APIENTRY *LPD3DNTHAL_TEXTUREDESTROYCB) (LPD3DNTHAL_TEXTUREDESTROYDATA);
typedef DWORD   (APIENTRY *LPD3DNTHAL_TEXTURESWAPCB)    (LPD3DNTHAL_TEXTURESWAPDATA);
typedef DWORD   (APIENTRY *LPD3DNTHAL_TEXTUREGETSURFCB) (LPD3DNTHAL_TEXTUREGETSURFDATA);

typedef struct _D3DNTHAL_CALLBACKS {
    DWORD           dwSize;

    // Device context
    LPD3DNTHAL_CONTEXTCREATECB  ContextCreate;
    LPD3DNTHAL_CONTEXTDESTROYCB ContextDestroy;
    LPD3DNTHAL_CONTEXTDESTROYALLCB ContextDestroyAll;

    // Scene Capture
    LPD3DNTHAL_SCENECAPTURECB   SceneCapture;

    // Execution
    LPVOID          dwReserved10;       // Must be zero (was Execute)
    LPVOID          dwReserved11;       // Must be zero (was ExecuteClipped)
    LPVOID          dwReserved22;       // Must be zero (was RenderState)
    LPVOID          dwReserved23;       // Must be zero (was RenderPrimitive)

    ULONG_PTR            dwReserved;     // Must be zero

    // Textures
    LPD3DNTHAL_TEXTURECREATECB  TextureCreate;
    LPD3DNTHAL_TEXTUREDESTROYCB TextureDestroy;
    LPD3DNTHAL_TEXTURESWAPCB    TextureSwap;
    LPD3DNTHAL_TEXTUREGETSURFCB TextureGetSurf;

    LPVOID                       dwReserved12;           // Must be zero
    LPVOID                       dwReserved13;           // Must be zero
    LPVOID                       dwReserved14;           // Must be zero
    LPVOID                       dwReserved15;           // Must be zero
    LPVOID                       dwReserved16;           // Must be zero
    LPVOID                       dwReserved17;           // Must be zero
    LPVOID                       dwReserved18;           // Must be zero
    LPVOID                       dwReserved19;           // Must be zero
    LPVOID                       dwReserved20;           // Must be zero
    LPVOID                       dwReserved21;           // Must be zero

    // Pipeline state
    LPVOID                       dwReserved24;           // Was GetState;

    ULONG_PTR            dwReserved0;        // Must be zero
    ULONG_PTR            dwReserved1;        // Must be zero
    ULONG_PTR            dwReserved2;        // Must be zero
    ULONG_PTR            dwReserved3;        // Must be zero
    ULONG_PTR            dwReserved4;        // Must be zero
    ULONG_PTR            dwReserved5;        // Must be zero
    ULONG_PTR            dwReserved6;        // Must be zero
    ULONG_PTR            dwReserved7;        // Must be zero
    ULONG_PTR            dwReserved8;        // Must be zero
    ULONG_PTR            dwReserved9;        // Must be zero

} D3DNTHAL_CALLBACKS;
typedef D3DNTHAL_CALLBACKS *LPD3DNTHAL_CALLBACKS;

#define D3DNTHAL_SIZE_V1 sizeof( D3DNTHAL_CALLBACKS )

typedef struct _D3DNTHAL_SETRENDERTARGETDATA {
    ULONG_PTR            dwhContext;     // in:  Context handle
    PDD_SURFACE_LOCAL   lpDDS;          // in:  new render target
    PDD_SURFACE_LOCAL   lpDDSZ;         // in:  new Z buffer
    HRESULT             ddrval;         // out: Return value
} D3DNTHAL_SETRENDERTARGETDATA;
typedef D3DNTHAL_SETRENDERTARGETDATA *LPD3DNTHAL_SETRENDERTARGETDATA;


typedef DWORD (APIENTRY *LPD3DNTHAL_SETRENDERTARGETCB) (LPD3DNTHAL_SETRENDERTARGETDATA);

typedef struct _D3DNTHAL_CALLBACKS2
{
    DWORD                       dwSize;                 // size of struct
    DWORD                       dwFlags;                // flags for callbacks

    LPD3DNTHAL_SETRENDERTARGETCB  SetRenderTarget;
    LPVOID                dwReserved1;           // was Clear
    LPVOID                dwReserved2;           // was DrawOnePrimitive
    LPVOID                dwReserved3;           // was DrawOneIndexedPrimitive
    LPVOID                dwReserved4;           // was DrawPrimitives
} D3DNTHAL_CALLBACKS2;
typedef D3DNTHAL_CALLBACKS2 *LPD3DNTHAL_CALLBACKS2;

#define D3DNTHAL2_CB32_SETRENDERTARGET    0x00000001L


typedef struct _D3DNTHAL_CLEAR2DATA
{
    ULONG_PTR            dwhContext;     // in:  Context handle

    // dwFlags can contain D3DCLEAR_TARGET or D3DCLEAR_ZBUFFER
    DWORD               dwFlags;        // in:  surfaces to clear

    DWORD               dwFillColor;    // in:  Color value for rtarget
    D3DVALUE            dvFillDepth;    // in:  Depth value for
                                        //      Z-buffer (0.0-1.0)
    DWORD               dwFillStencil;  // in:  value used to clear stencil buffer

    LPD3DRECT           lpRects;        // in:  Rectangles to clear
    DWORD               dwNumRects;     // in:  Number of rectangles

    HRESULT             ddrval;         // out: Return value
} D3DNTHAL_CLEAR2DATA;
typedef D3DNTHAL_CLEAR2DATA FAR *LPD3DNTHAL_CLEAR2DATA;

typedef struct _D3DNTHAL_VALIDATETEXTURESTAGESTATEDATA
{
    ULONG_PTR        dwhContext; // in:  Context handle
    DWORD           dwFlags;    // in:  Flags, currently set to 0
    ULONG_PTR        dwReserved; //
    DWORD           dwNumPasses;// out: Number of passes the hardware
                                //      can perform the operation in
    HRESULT         ddrval;     // out: return value
} D3DNTHAL_VALIDATETEXTURESTAGESTATEDATA;
typedef D3DNTHAL_VALIDATETEXTURESTAGESTATEDATA FAR *LPD3DNTHAL_VALIDATETEXTURESTAGESTATEDATA;

//-----------------------------------------------------------------------------
// DrawPrimitives2 DDI
//-----------------------------------------------------------------------------

//
// Command structure for vertex buffer rendering
//

typedef struct _D3DNTHAL_DP2COMMAND
{
    BYTE bCommand;           // vertex command
    BYTE bReserved;
    union
    {
        WORD wPrimitiveCount;   // primitive count for unconnected primitives
        WORD wStateCount;     // count of render states to follow
    };
} D3DNTHAL_DP2COMMAND, *LPDNT3DHAL_DP2COMMAND;

//
// DrawPrimitives2 commands:
//

typedef enum _D3DNTHAL_DP2OPERATION
{
    D3DNTDP2OP_POINTS               = 1,
    D3DNTDP2OP_INDEXEDLINELIST      = 2,
    D3DNTDP2OP_INDEXEDTRIANGLELIST  = 3,
    D3DNTDP2OP_RESERVED0            = 4,
    D3DNTDP2OP_RENDERSTATE          = 8,
    D3DNTDP2OP_LINELIST             = 15,
    D3DNTDP2OP_LINESTRIP            = 16,
    D3DNTDP2OP_INDEXEDLINESTRIP     = 17,
    D3DNTDP2OP_TRIANGLELIST         = 18,
    D3DNTDP2OP_TRIANGLESTRIP        = 19,
    D3DNTDP2OP_INDEXEDTRIANGLESTRIP = 20,
    D3DNTDP2OP_TRIANGLEFAN          = 21,
    D3DNTDP2OP_INDEXEDTRIANGLEFAN   = 22,
    D3DNTDP2OP_TRIANGLEFAN_IMM      = 23,
    D3DNTDP2OP_LINELIST_IMM         = 24,
    D3DNTDP2OP_TEXTURESTAGESTATE    = 25,
    D3DNTDP2OP_INDEXEDTRIANGLELIST2 = 26,
    D3DNTDP2OP_INDEXEDLINELIST2     = 27,
    D3DNTDP2OP_VIEWPORTINFO         = 28,
    D3DNTDP2OP_WINFO                = 29,
    D3DNTDP2OP_SETPALETTE           = 30,
    D3DNTDP2OP_UPDATEPALETTE        = 31,
#if(DIRECT3D_VERSION >= 0x0700)
    //new for DX7
    D3DNTDP2OP_ZRANGE               = 32,
    D3DNTDP2OP_SETMATERIAL          = 33,
    D3DNTDP2OP_SETLIGHT             = 34,
    D3DNTDP2OP_CREATELIGHT          = 35,
    D3DNTDP2OP_SETTRANSFORM         = 36,
    D3DNTDP2OP_EXT                  = 37,
    D3DNTDP2OP_TEXBLT               = 38,
    D3DNTDP2OP_STATESET             = 39,
    D3DNTDP2OP_SETPRIORITY          = 40,
#endif /* DIRECT3D_VERSION >= 0x0700 */
    D3DNTDP2OP_SETRENDERTARGET      = 41,
    D3DNTDP2OP_CLEAR                = 42,
#if(DIRECT3D_VERSION >= 0x0700)
    D3DNTDP2OP_SETTEXLOD            = 43,
    D3DNTDP2OP_SETCLIPPLANE         = 44,
#endif /* DIRECT3D_VERSION >= 0x0700 */
} D3DNTHAL_DP2OPERATION;

//
// DrawPrimitives2 point primitives
//

typedef struct _D3DNTHAL_DP2POINTS
{
    WORD wCount;
    WORD wVStart;
} D3DNTHAL_DP2POINTS;

//
// DrawPrimitives2 line primitives
//

typedef struct _D3DNTHAL_DP2STARTVERTEX {
    WORD wVStart;
} D3DNTHAL_DP2STARTVERTEX, *LPD3DNTHAL_DP2STARTVERTEX;

typedef struct _D3DNTHAL_DP2LINELIST
{
    WORD wVStart;
} D3DNTHAL_DP2LINELIST;

typedef struct _D3DNTHAL_DP2INDEXEDLINELIST
{
    WORD wV1;
    WORD wV2;
} D3DNTHAL_DP2INDEXEDLINELIST;

typedef struct _D3DNTHAL_DP2LINESTRIP
{
    WORD wVStart;
} D3DNTHAL_DP2LINESTRIP;

typedef struct _D3DNTHAL_DP2INDEXEDLINESTRIP
{
    WORD wV[2];
} D3DNTHAL_DP2INDEXEDLINESTRIP;

//
// DrawPrimitives2 triangle primitives
//

typedef struct _D3DNTHAL_DP2TRIANGLELIST
{
    WORD wVStart;
} D3DNTHAL_DP2TRIANGLELIST;

typedef struct _D3DNTHAL_DP2INDEXEDTRIANGLELIST
{
    WORD wV1;
    WORD wV2;
    WORD wV3;
    WORD wFlags;
} D3DNTHAL_DP2INDEXEDTRIANGLELIST;

typedef struct _D3DNTHAL_DP2INDEXEDTRIANGLELIST2 {
    WORD wV1;
    WORD wV2;
    WORD wV3;
} D3DNTHAL_DP2INDEXEDTRIANGLELIST2, *LPD3DNTHAL_DP2INDEXEDTRIANGLELIST2;

typedef struct _D3DNTHAL_DP2TRIANGLESTRIP
{
    WORD wVStart;
} D3DNTHAL_DP2TRIANGLESTRIP;

typedef struct _D3DNTHAL_DP2INDEXEDTRIANGLESTRIP
{
    WORD wV[3];
} D3DNTHAL_DP2INDEXEDTRIANGLESTRIP;

typedef struct _D3DNTHAL_DP2TRIANGLEFAN
{
    WORD wVStart;
} D3DNTHAL_DP2TRIANGLEFAN;

typedef struct _D3DNTHAL_DP2INDEXEDTRIANGLEFAN
{
    WORD wV[3];
} D3DNTHAL_DP2INDEXEDTRIANGLEFAN;

typedef struct _D3DNTHAL_DP2TRIANGLEFAN_IMM {
    DWORD dwEdgeFlags;
} D3DNTHAL_DP2TRIANGLEFAN_IMM, *LPD3DNTHAL_DP2TRIANGLEFAN_IMM;

//
// DrawPrimitives2 Renderstate changes
//

typedef struct _D3DNTHAL_DP2RENDERSTATE
{
    D3DRENDERSTATETYPE RenderState;
    union
    {
        D3DVALUE fState;
        DWORD dwState;
    };
} D3DNTHAL_DP2RENDERSTATE;

typedef struct _D3DNTHAL_DP2TEXTURESTAGESTATE
{
    WORD wStage;
    WORD TSState;
    DWORD dwValue;
} D3DNTHAL_DP2TEXTURESTAGESTATE;

typedef struct _D3DNTHAL_DP2VIEWPORTINFO {
    DWORD dwX;
    DWORD dwY;
    DWORD dwWidth;
    DWORD dwHeight;
} D3DNTHAL_DP2VIEWPORTINFO, *LPD3DNTHAL_DP2VIEWPORTINFO;

typedef struct _D3DNTHAL_DP2WINFO {
    D3DVALUE        dvWNear;
    D3DVALUE        dvWFar;
} D3DNTHAL_DP2WINFO, *LPD3DNTHAL_DP2WINFO;

typedef struct _D3DNTHAL_DP2SETPALETTE
{
    DWORD dwPaletteHandle;
    DWORD dwPaletteFlags;
    DWORD dwSurfaceHandle;
} D3DNTHAL_DP2SETPALETTE, *LPD3DNTHAL_DP2SETPALETTE;

typedef struct _D3DNTHAL_DP2UPDATEPALETTE
{
    DWORD dwPaletteHandle;
    WORD  wStartIndex;
    WORD  wNumEntries;
} D3DNTHAL_DP2UPDATEPALETTE, *LPD3DNTHAL_DP2UPDATEPALETTE;

typedef struct _D3DNTHAL_DP2SETRENDERTARGET
{
    DWORD hRenderTarget;
    DWORD hZBuffer;
} D3DNTHAL_DP2SETRENDERTARGET, *LPD3DNTHAL_DP2SETRENDERTARGET;

#if(DIRECT3D_VERSION >= 0x0700)
// Values for dwOperations in the D3DHAL_DP2STATESET
#define D3DHAL_STATESETBEGIN     0
#define D3DHAL_STATESETEND       1
#define D3DHAL_STATESETDELETE    2
#define D3DHAL_STATESETEXECUTE   3
#define D3DHAL_STATESETCAPTURE   4

typedef struct _D3DNTHAL_DP2STATESET
{
    DWORD       dwOperation;
    DWORD       dwParam;        // State set handle passed with D3DHAL_STATESETBEGIN,
                                // D3DHAL_STATESETEXECUTE, D3DHAL_STATESETDELETE
                                // D3DHAL_STATESETCAPTURE
    D3DSTATEBLOCKTYPE   sbType; // Type use with D3DHAL_STATESETBEGIN/END
} D3DNTHAL_DP2STATESET, *LPD3DNTHAL_DP2STATESET;
//
// T&L Hal specific stuff
//
typedef struct _D3DNTHAL_DP2ZRANGE
{
    D3DVALUE    dvMinZ;
    D3DVALUE    dvMaxZ;
} D3DNTHAL_DP2ZRANGE, *LPD3DNTHAL_DP2ZRANGE;

typedef D3DMATERIAL7 D3DNTHAL_DP2SETMATERIAL, *LPD3DNTHAL_DP2SETMATERIAL;

typedef struct _D3DNTHAL_DP2SETLIGHT
{
    DWORD     dwIndex;
    DWORD     lightData;
} D3DNTHAL_DP2SETLIGHT, *LPD3DNTHAL_DP2SETLIGHT;

typedef struct _D3DNTHAL_DP2SETCLIPPLANE
{
    DWORD     dwIndex;
    D3DVALUE  plane[4];
} D3DNTHAL_DP2SETCLIPPLANE, *LPD3DNTHAL_DP2SETCLIPPLANE;

typedef struct _D3DNTHAL_DP2CREATELIGHT
{
    DWORD dwIndex;
} D3DNTHAL_DP2CREATELIGHT, *LPD3DNTHAL_DP2CREATELIGHT;

typedef struct _D3DNTHAL_DP2SETTRANSFORM
{
    D3DTRANSFORMSTATETYPE xfrmType;
    D3DMATRIX matrix;
} D3DNTHAL_DP2SETTRANSFORM, *LPD3DNTHAL_DP2SETTRANSFORM;

typedef struct _D3DNTHAL_DP2EXT
{
    DWORD dwExtToken;
    DWORD dwSize;
} D3DNTHAL_DP2EXT, *LPD3DNTHAL_DP2EXT;
typedef struct _D3DNTHAL_DP2TEXBLT
{
    DWORD   dwDDDestSurface;// dest surface
    DWORD   dwDDSrcSurface; // src surface
    POINT   pDest;
    RECTL   rSrc;       // src rect
    DWORD   dwFlags;    // blt flags
} D3DNTHAL_DP2TEXBLT, *LPD3DNTHAL_DP2TEXBLT;

typedef struct _D3DNTHAL_DP2SETPRIORITY
{
    DWORD dwDDDestSurface;// dest surface
    DWORD dwPriority;
} D3DNTHAL_DP2SETPRIORITY, *LPD3DNTHAL_DP2SETPRIORITY;

typedef struct _D3DNTHAL_DP2CLEAR
{
  // dwFlags can contain D3DCLEAR_TARGET, D3DCLEAR_ZBUFFER, and/or D3DCLEAR_STENCIL
    DWORD               dwFlags;        // in:  surfaces to clear
    DWORD               dwFillColor;    // in:  Color value for rtarget
    D3DVALUE            dvFillDepth;    // in:  Depth value for Z buffer (0.0-1.0)
    DWORD               dwFillStencil;  // in:  value used to clear stencil buffer
    RECT                Rects[1];       // in:  Rectangles to clear
} D3DNTHAL_DP2CLEAR, *LPD3DNTHAL_DP2CLEAR;

typedef struct _D3DNTHAL_DP2SETTEXLOD
{
    DWORD dwDDSurface;
    DWORD dwLOD;
} D3DNTHAL_DP2SETTEXLOD, *LPD3DNTHAL_DP2SETTEXLOD;

#endif /* DIRECT3D_VERSION >= 0x0700 */


typedef struct _D3DNTHAL_DRAWPRIMITIVES2DATA
{
    ULONG_PTR          dwhContext;        // in: Context handle
    DWORD             dwFlags;           // in: flags (look below)
    DWORD             dwVertexType;      // in: vertex type
    PDD_SURFACE_LOCAL lpDDCommands;      // in: vertex buffer command data
    DWORD             dwCommandOffset;   // in: offset to start of vb commands
    DWORD             dwCommandLength;   // in: number of bytes of command data
    union
    {
        PDD_SURFACE_LOCAL lpDDVertex;    // in: surface containing vertex data
        LPVOID lpVertices;               // in: User mode pointer to vertices
    };
    DWORD             dwVertexOffset;    // in: offset to start of vertex data
    DWORD             dwVertexLength;    // in: number of bytes of vertex data
    DWORD             dwReqVertexBufSize;// in: number of bytes required for
                                         //     the next vertex buffer
    DWORD             dwReqCommandBufSize; // in: number if bytes required for
                                         //     the next commnand buffer
    LPDWORD           lpdwRStates;       // in: Pointer to the array where render states are updated
    union
    {
        DWORD         dwVertexSize;      // in: Size of each vertex in bytes
        HRESULT       ddrval;            // out: return value
    };
    DWORD             dwErrorOffset;     // out: offset in LPDDVBCOMMAND to
                                         //      first failed D3DNTHAL_VBCOMMAND
} D3DNTHAL_DRAWPRIMITIVES2DATA;
typedef D3DNTHAL_DRAWPRIMITIVES2DATA FAR *LPD3DNTHAL_DRAWPRIMITIVES2DATA;

// Indicates that the lpVertices field in the DrawPrimitives2 data is
// valid, i.e. user allocated memory.
#define D3DNTHALDP2_USERMEMVERTICES   0x00000001L
// Indicates that the command buffer and vertex buffer are a system memory execute buffer
// resulting from the use of the Execute buffer API.
#define D3DNTHALDP2_EXECUTEBUFFER     0x00000002L

// The swap flags indicate if it is OK for the driver to swap the submitted buffers with new
// buffers and asyncronously work on the submitted buffers.
#define D3DNTHALDP2_SWAPVERTEXBUFFER  0x00000004L
#define D3DNTHALDP2_SWAPCOMMANDBUFFER 0x00000008L
// The requested flags are present if the new buffers which the driver can allocate need to be
// of atleast a given size. If any of these flags are set, the corresponding dwReq* field in
// D3DNTHAL_DRAWPRIMITIVES2DATA will also be set with the requested size in bytes.
#define D3DNTHALDP2_REQVERTEXBUFSIZE  0x00000010L
#define D3DNTHALDP2_REQCOMMANDBUFSIZE 0x00000020L
// These flags are set by the driver upon return from DrawPrimitives2 indicating if the new
// buffers are not in system memory.
#define D3DNTHALDP2_VIDMEMVERTEXBUF   0x00000040L
#define D3DNTHALDP2_VIDMEMCOMMANDBUF  0x00000080L


// Return values for the driver callback used in DP2 implementations
// Used by the driver to ask runtime to parse the execute buffer
#define D3DNTERR_COMMAND_UNPARSED         MAKE_DDHRESULT(3000)


typedef DWORD (APIENTRY *LPD3DNTHAL_CLEAR2CB) (LPD3DNTHAL_CLEAR2DATA);
typedef DWORD (APIENTRY *LPD3DNTHAL_VALIDATETEXTURESTAGESTATECB) (LPD3DNTHAL_VALIDATETEXTURESTAGESTATEDATA);
typedef DWORD (APIENTRY *LPD3DNTHAL_DRAWPRIMITIVES2CB) (LPD3DNTHAL_DRAWPRIMITIVES2DATA);

typedef struct _D3DNTHAL_CALLBACKS3
{
    DWORD                       dwSize;                 // size of struct
    DWORD                       dwFlags;                // flags for callbacks

    LPD3DNTHAL_CLEAR2CB            Clear2;
    LPVOID                         lpvReserved;
    LPD3DNTHAL_VALIDATETEXTURESTAGESTATECB  ValidateTextureStageState;
    LPD3DNTHAL_DRAWPRIMITIVES2CB   DrawPrimitives2;
} D3DNTHAL_CALLBACKS3;
typedef D3DNTHAL_CALLBACKS3 *LPD3DNTHAL_CALLBACKS3;

#define D3DNTHAL3_CB32_CLEAR2             0x00000001L
#define D3DNTHAL3_CB32_RESERVED           0x00000002L
#define D3DNTHAL3_CB32_VALIDATETEXTURESTAGESTATE   0x00000004L
#define D3DNTHAL3_CB32_DRAWPRIMITIVES2    0x00000008L

// typedef for the Callback that the drivers can use to parse unknown commands
// passed to them via the DrawPrimitives2 callback. The driver obtains this
// callback thru a GetDriverInfo call with GUID_D3DParseUnknownCommandCallback
// made by ddraw somewhere around the initialization time.
typedef HRESULT (CALLBACK *PFND3DNTPARSEUNKNOWNCOMMAND) (LPVOID lpvCommands,
                                                         LPVOID *lplpvReturnedCommand);

/* --------------------------------------------------------------
 * Texture stage renderstate mapping definitions.
 *
 * 256 renderstate slots [256, 511] are reserved for texture processing
 * stage controls, which provides for 8 texture processing stages each
 * with 32 DWORD controls.
 *
 * The renderstates within each stage are indexed by the
 * D3DTEXTURESTAGESTATETYPE enumerants by adding the appropriate
 * enumerant to the base for a given texture stage.
 *
 * Note, "state overrides" bias the renderstate by 256, so the two
 * ranges overlap.  Overrides are enabled for exebufs only, so all
 * this means is that Texture3 cannot be used with exebufs.
 */

/*
 * Base of all texture stage state values in renderstate array.
 */
#define D3DNTHAL_TSS_RENDERSTATEBASE 256UL

/*
 * Maximum number of stages allowed.
 */
#define D3DNTHAL_TSS_MAXSTAGES 8

/*
 * Number of state DWORDS per stage.
 */
#define D3DNTHAL_TSS_STATESPERSTAGE 64

/*
 * Texture handle's offset into the 32-DWORD cascade state vector
 */
#ifndef D3DTSS_TEXTUREMAP
#define D3DTSS_TEXTUREMAP 0
#endif

#define D3DRENDERSTATE_EVICTMANAGEDTEXTURES 61  // DDI render state only to Evict textures
#define D3DRENDERSTATE_SCENECAPTURE         62  // DDI only to replace SceneCapture

#endif /* _D3DNTHAL_H */

