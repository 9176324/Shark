/******************************Module*Header**********************************\
*
*                           *******************
*                           * D3D SAMPLE CODE *
*                           *******************
*
* Module Name: d3ddp2op.c
*
* Content: D3D DrawPrimitives2 command buffer operations support
*
* Copyright (c) 1994-1999 3Dlabs Inc. Ltd. All rights reserved.
* Copyright (c) 1995-2002 Microsoft Corporation.  All rights reserved.
\*****************************************************************************/

#include "glint.h"
#include "dma.h"
#include "tag.h"

//-----------------------------------------------------------------------------
//
// __OP_IntersectRectl
//
// This function intersects two RECTLs. If no intersection exists returns FALSE
//
//-----------------------------------------------------------------------------
BOOL 
__OP_IntersectRectl(
    RECTL*  prcresult,
    RECTL*  prcin1,
    RECT*  prcin2)
{
    prcresult->left  = max(prcin1->left,  prcin2->left);
    prcresult->right = min(prcin1->right, prcin2->right);

    if (prcresult->left < prcresult->right)
    {
        prcresult->top    = max(prcin1->top,    prcin2->top);
        prcresult->bottom = min(prcin1->bottom, prcin2->bottom);

        if (prcresult->top < prcresult->bottom)
        {
            return TRUE;
        }
    }

    return FALSE;
} // __OP_IntersectRectl

//-----------------------------------------------------------------------------
//
// __OP_PrepareFillColor
//
// Covert A8R8B8G8 to fill color used by Permedia3 chip
//
//-----------------------------------------------------------------------------

DWORD
_OP_PrepareFillColor(
    DWORD dwRBitMask,
    DWORD dwRGBAlphaBitMask,
    DWORD dwPixelSize,
    DWORD dwARGBColor)
{
    DWORD   color;

    // Initialize the color
    color = dwARGBColor;

    switch (dwPixelSize)
    {
        // 16 Bit colors come in as 32 Bit RGB Values
        // Color will be packed in the clear function
        case __GLINT_16BITPIXEL:
            if (dwRBitMask == 0x7C00)  // [A|X]1R5G5B5
            {
                color = ((dwARGBColor & 0xf8) >> 3)             |
                        ((dwARGBColor & 0xf800) >> (16 - 10))   |
                        ((dwARGBColor & 0xf80000) >> (24 - 15));
                if (dwRGBAlphaBitMask)
                {
                    color |= ((dwARGBColor & 0x80000000) >> 16);
                }
            } 
            else if (dwRBitMask == 0xF800) // R5G6B5
            {
                color = ((dwARGBColor & 0xff) >> 3)             |
                        ((dwARGBColor & 0xfc00) >> (16 - 11))   |
                        ((dwARGBColor & 0xf80000) >> (24 - 16));
            }
            else if (dwRBitMask == 0xF00) // [A|X]4R4G4B4
            {
                color = ((dwARGBColor & 0xf0) >> 4)             |
                        ((dwARGBColor & 0xf000) >> (16 - 8))    |
                        ((dwARGBColor & 0xf00000) >> (24 - 12));
                if (dwRGBAlphaBitMask)
                {
                    color |= ((dwARGBColor & 0xf0000000) >> 16);
                }
            }
            else
            {
                DISPDBG((DBGLVL, "A8L8 passed in for _OP_PrepareFillColor()"));
            }
            break;

        case __GLINT_24BITPIXEL:
            DISPDBG((ERRLVL, "Perm3 doesn't support 24 bpp surface"));
            break;

        default:
            break;
    }

    return (color);
}

//-----------------------------------------------------------------------------
// 
//  _OP_GetSimpleSurfVidMemPtr
//
//  Return the true surface vid mem pointer from the surface internal structure
//
//-----------------------------------------------------------------------------
FLATPTR _inline
_OP_GetSimpleSurfVidMemPtr(
    P3_SURF_INTERNAL *pSurfInt)
{
#if DX9_RTZMAN
    if (pSurfInt->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
    {
        return (pSurfInt->MipLevels[0].fpVidMemTM);
    }
    else
#endif
    {
        return (pSurfInt->fpVidMem);
    }
}

#if DX7_TEXMANAGEMENT       
VOID __OP_MarkManagedSurfDirty(P3_D3DCONTEXT* pContext, 
                               DWORD dwSurfHandle,
                               P3_SURF_INTERNAL* pTexture);
#endif

//-----------------------------------------------------------------------------
//
// _D3D_OP_Clear2
//
// This function processes the D3DDP2OP_CLEAR DP2 command token.
//
// It builds a mask and a value for the stencil/depth clears. The mask is used 
// to stop unwanted bits being updated during the clear. The value is scaled in 
// the case of the Z depth, and is shifted in the case of the stencil.  This 
// results in the correct value being written, at the correct location in the 
// ZBuffer, while doing fast-block fills through SGRAM
//-----------------------------------------------------------------------------

#define P3RX_UPDATE_WRITE_MASK(a)                   \
if (dwCurrentMask != a)                             \
{                                                   \
    P3_DMA_GET_BUFFER_ENTRIES(2);                   \
    SEND_P3_DATA(FBHardwareWriteMask, a);           \
    P3_DMA_COMMIT_BUFFER();                         \
    dwCurrentMask = a;                              \
}

VOID 
_D3D_OP_Clear2( 
    P3_D3DCONTEXT* pContext,
    D3DHAL_DP2CLEAR* lpcd2,
    DWORD dwNumRects)
{
    DWORD i;
    RECTL rect, rect_vwport;
    DWORD dwDepthValue;
    DWORD dwStencilValue;
    DWORD dwStencilMask;
    DWORD dwDepthMask;
    DWORD color;
    DWORD dwCurrentMask = 0xFFFFFFFF;
    BOOL bNoBlockFillZ = FALSE;
    BOOL bNoBlockFillStencil = FALSE;
    BOOL bComputeIntersections = FALSE;
    BYTE Bytes[4];
    P3_THUNKEDDATA* pThisDisplay = pContext->pThisDisplay;
    HRESULT ddrval;
    D3DHAL_DP2CLEAR WholeViewport;

    P3_DMA_DEFS();

    DBG_CB_ENTRY(_D3D_OP_Clear2);
    
    // Check if we were asked to clear a valid buffer
    if ((lpcd2->dwFlags & (D3DCLEAR_TARGET  | 
                           D3DCLEAR_ZBUFFER | 
                           D3DCLEAR_STENCIL)) == 0)
    {
        // We have been asked to do nothing - and that's what we've done.
        DBG_CB_EXIT(_D3D_OP_Clear2, DD_OK);    
        return;
    }

#if DX9_RTZMAN
    // Make sure the video memory copy of the render target is ready
    if ((lpcd2->dwFlags & D3DCLEAR_TARGET) &&
        (pContext->pSurfRenderInt != NULL) &&
        (! _D3D_TM_Preload_Tex_IntoVidMem(pContext, pContext->pSurfRenderInt)))
    {
        return;
    }

    // Make sure the video memory copy of the Z buffer is ready
    if ((lpcd2->dwFlags & D3DCLEAR_ZBUFFER) &&
        (pContext->pSurfZBufferInt != NULL) &&
        (! _D3D_TM_Preload_Tex_IntoVidMem(pContext, pContext->pSurfZBufferInt)))
    {
            return;
    }

    // Notify texture manager that the RT and ZBuf are in use
    _D3D_TM_Touch_Context(pContext);
#endif

#if DX8_DDI
    // When zero clear rects is passed to a DX8 driver with D3DDP2OP_CLEAR 
    // token, the driver should clear the whole viewport. The zero number 
    // of rects could be passed only if D3D is using a pure device. 

    // D3DCLEAR_COMPUTERECTS has been added to the dwFlags of D3DHAL_CLEARDATA.
    // When set, the flag means that user provided clear rects  should be 
    // culled against the current viewport.

    if (! (lpcd2->dwFlags & D3DCLEAR_COMPUTERECTS))
    {
        // Do nothing for non-pure device
    }
    else
    if (dwNumRects == 0)
    {
        // When wStateCount is zero we need to clear whole viewport
        WholeViewport.dwFlags = lpcd2->dwFlags;     
        WholeViewport.dwFillColor = lpcd2->dwFillColor; 
        WholeViewport.dvFillDepth = lpcd2->dvFillDepth; 
        WholeViewport.dwFillStencil = lpcd2->dwFillStencil;
        WholeViewport.Rects[0].left = pContext->ViewportInfo.dwX;
        WholeViewport.Rects[0].top = pContext->ViewportInfo.dwY;
        WholeViewport.Rects[0].right = pContext->ViewportInfo.dwX +
                                            pContext->ViewportInfo.dwWidth;
        WholeViewport.Rects[0].bottom = pContext->ViewportInfo.dwY + 
                                             pContext->ViewportInfo.dwHeight;    
        // Replace pointers and continue as usual
        lpcd2 = &WholeViewport;
        dwNumRects = 1;
    }
    else
    {
        // We need to cull all rects against the current viewport
        // but in order not to allocate a temporary RECT array in
        // kernel heap we'll compute this inside the clearing loop

        rect_vwport.left   = pContext->ViewportInfo.dwX;
        rect_vwport.top    = pContext->ViewportInfo.dwY;
        rect_vwport.right  = pContext->ViewportInfo.dwX + 
                             pContext->ViewportInfo.dwWidth;
        rect_vwport.bottom = pContext->ViewportInfo.dwY + 
                             pContext->ViewportInfo.dwHeight;

        bComputeIntersections = TRUE;

    }
#endif // DX8_DDI

    // Check if there is any rect to clear at all
    if (dwNumRects == 0)
    {
        // We have been asked to do nothing - and that's what we've done.
        DBG_CB_EXIT(_D3D_OP_Clear2, DD_OK);    
        return;
    }   

    // Wait until we have we finished flipping before clearing anything
    if (pThisDisplay->pGLInfo->InterfaceType != GLINT_DMA)
    {
        do
        {
            ddrval = _DX_QueryFlipStatus(pContext->pThisDisplay,
                                         _OP_GetSimpleSurfVidMemPtr(
                                             pContext->pSurfRenderInt), 
                                         TRUE);
        } while (FAILED(ddrval));
    }

    // Switch to hw Ddraw context in order to do the clears
    DDRAW_OPERATION(pContext, pThisDisplay);

    // Prepare any data necessary to clear the render target
    if ((lpcd2->dwFlags & D3DCLEAR_TARGET) && 
        (pContext->pSurfRenderInt != NULL))
    {
        color = _OP_PrepareFillColor(pContext->pSurfRenderInt->pixFmt.dwRBitMask,
                                     0, // Alway use 0 for compatibility
                                     pContext->pSurfRenderInt->dwPixelSize,
                                     lpcd2->dwFillColor);

    } // if (lpcd2->dwFlags & D3DCLEAR_TARGET)

    // Prepare any data necessary to clear the depth buffer
    if ((lpcd2->dwFlags & D3DCLEAR_ZBUFFER) && 
        (pContext->pSurfZBufferInt != NULL))
    {
        float fDepth;
        
        DDPIXELFORMAT* pPixFormat = &pContext->pSurfZBufferInt->pixFmt;
                
        DWORD dwZBitDepth = pPixFormat->dwZBufferBitDepth;

        // Find the depth bits, remembering to remove any stencil bits.
        if (pPixFormat->dwFlags & DDPF_STENCILBUFFER)
        {
            dwZBitDepth -= pPixFormat->dwStencilBitDepth;
        }

        dwDepthMask = (0xFFFFFFFF >> (32 - dwZBitDepth));

        // 32 bit depth buffers on Perm3 are really 
        // limited to 31 bits of precision
        if (dwZBitDepth == 32)
        {
            dwDepthMask = dwDepthMask >> 1; 
        }

        if (lpcd2->dvFillDepth == 1.0f)
        {
            dwDepthValue = dwDepthMask;
        }
        else
        {
            fDepth = lpcd2->dvFillDepth * (float)dwDepthMask;

            // This is a hardware dependency on how the Perm3 handles the 
            // limited precision of 32bit floats(24 bits of mantissa) and
            // converts the value into a 32bit z buffer value. This doesn't
            // happen with any other z bit depth but 32.
            if (dwZBitDepth == 32)
            {
                fDepth += 0.5f;
            }
            
            myFtoi((int*)&dwDepthValue, fDepth);
        }

        // As we are fast-block filling, make sure we copy the 
        // Mask to the top bits.
        switch (pContext->pSurfZBufferInt->dwPixelSize)
        {
            case __GLINT_16BITPIXEL:
                dwDepthMask &= 0xFFFF;
                dwDepthMask |= (dwDepthMask << 16);
                break;
            case __GLINT_8BITPIXEL:
                dwDepthMask &= 0xFF;
                dwDepthMask |= dwDepthMask << 8;
                dwDepthMask |= dwDepthMask << 16;
                break;
        }

        if (pThisDisplay->pGLInfo->bDRAMBoard)
        {
            // Check for a DRAM fill that the chip isn't emulating.
            Bytes[0] = (BYTE)(dwDepthMask & 0xFF);
            Bytes[1] = (BYTE)((dwDepthMask & 0xFF00) >> 8);
            Bytes[2] = (BYTE)((dwDepthMask & 0xFF0000) >> 16);
            Bytes[3] = (BYTE)((dwDepthMask & 0xFF000000) >> 24);
            if (((Bytes[0] != 0) && (Bytes[0] != 0xFF)) ||
                ((Bytes[1] != 0) && (Bytes[1] != 0xFF)) ||
                ((Bytes[2] != 0) && (Bytes[2] != 0xFF)) ||
                ((Bytes[3] != 0) && (Bytes[3] != 0xFF)))
            {
                bNoBlockFillZ = TRUE;
            }
        }

        DISPDBG((DBGLVL,"ZClear Value = 0x%x, ZClear Mask = 0x%x", 
                   dwDepthValue, dwDepthMask));
    } // if (lpcd2->dwFlags & D3DCLEAR_ZBUFFER)

    // Prepare any data necessary to clear the stencil buffer
    if ((lpcd2->dwFlags & D3DCLEAR_STENCIL) && 
        (pContext->pSurfZBufferInt != NULL))
    {
        int dwShiftCount = 0;
        DDPIXELFORMAT* pPixFormat = &pContext->pSurfZBufferInt->pixFmt;

        // Find out where to shift the 
        dwStencilMask = pPixFormat->dwStencilBitMask;

        if (dwStencilMask != 0)
        {
            while ((dwStencilMask & 0x1) == 0) 
            {
                dwStencilMask >>= 1;
                dwShiftCount++;
            }
            
            dwStencilValue = (lpcd2->dwFillStencil << dwShiftCount);
            dwStencilMask = pPixFormat->dwStencilBitMask;

            // As we are fast-block filling, make sure we copy the 
            // Mask to the top bits.
            switch (pContext->pSurfZBufferInt->dwPixelSize)
            {
                case __GLINT_16BITPIXEL:
                    dwStencilMask &= 0xFFFF;
                    dwStencilMask |= (dwStencilMask << 16);
                    break;
                    
                case __GLINT_8BITPIXEL:
                    dwStencilMask &= 0xFF;
                    dwStencilMask |= dwStencilMask << 8;
                    dwStencilMask |= dwStencilMask << 16;
                    break;
            }

            DISPDBG((DBGLVL,"Stencil Clear Value = 0x%x, Stencil Mask = 0x%x", 
                       dwStencilValue, dwStencilMask));
        }
        else
        {
            DISPDBG((ERRLVL,"ERROR: Stencil mask is not valid!"));
            dwStencilValue = 0;
            dwStencilMask = 0;
        }

        if (pThisDisplay->pGLInfo->bDRAMBoard)
        {
            // Check for a DRAM fill that the chip isn't emulating.
            Bytes[0] = (BYTE)(dwStencilMask & 0xFF);
            Bytes[1] = (BYTE)((dwStencilMask & 0xFF00) >> 8);
            Bytes[2] = (BYTE)((dwStencilMask & 0xFF0000) >> 16);
            Bytes[3] = (BYTE)((dwStencilMask & 0xFF000000) >> 24);
            if (((Bytes[0] != 0) && (Bytes[0] != 0xFF)) ||
                ((Bytes[1] != 0) && (Bytes[1] != 0xFF)) ||
                ((Bytes[2] != 0) && (Bytes[2] != 0xFF)) ||
                ((Bytes[3] != 0) && (Bytes[3] != 0xFF)))
            {
                bNoBlockFillStencil = TRUE;
            }
        }
    } // if (lpcd2->dwFlags & D3DCLEAR_STENCIL)


    // Loop through each clearing rect and perform the clearing hw operations
    i = dwNumRects;
    while (i-- > 0) 
    {
        if (bComputeIntersections)
        {
            // Compute intersection between the viewport and the incoming 
            // RECTLs. If no intersection exists skip into next one.

            if (!__OP_IntersectRectl(&rect, &rect_vwport, &lpcd2->Rects[i]))
            {
                // No intersection, so skip it
                goto Next_Rectl_To_Clear;
            }
        }
        else
        {
            // We already have the rects we need to clear, so 
            // just use them in reverse order
            rect.left   = lpcd2->Rects[i].left;
            rect.right  = lpcd2->Rects[i].right;
            rect.top    = lpcd2->Rects[i].top;
            rect.bottom = lpcd2->Rects[i].bottom;
        }

        // Make sure that rect is with the render target's dimension
        if (pContext->pSurfRenderInt)
        {
            P3_SURF_INTERNAL *pSurfRenderInt = pContext->pSurfRenderInt;

            if (((DWORD)rect.left) > pSurfRenderInt->wWidth)
            {
                rect.left = pSurfRenderInt->wWidth;
            }
            if (((DWORD)rect.right) > pSurfRenderInt->wWidth)
            {
                rect.right = pSurfRenderInt->wWidth;
            }
            if (((DWORD)rect.top) > pSurfRenderInt->wHeight)
            {
                rect.top = pSurfRenderInt->wHeight;
            }
            if (((DWORD)rect.bottom) > pSurfRenderInt->wHeight)
            {
                rect.bottom = pSurfRenderInt->wHeight;
            }
        }

        // Clear the frame buffer
        if ((lpcd2->dwFlags & D3DCLEAR_TARGET) && 
            (pContext->pSurfRenderInt != NULL))        
        {
            P3RX_UPDATE_WRITE_MASK(__GLINT_ALL_WRITEMASKS_SET);

#if DX8_MULTISAMPLING || DX7_ANTIALIAS
            if (pContext->Flags & SURFACE_ANTIALIAS)
            {
                RECTL Temp = rect;
                Temp.left *= 2;
                Temp.right *= 2;
                Temp.top *= 2;
                Temp.bottom *= 2;
            
                _DD_BLT_P3Clear_AA(pThisDisplay, 
                                   &Temp, 
                                   pContext->dwAliasBackBuffer - 
                                        pThisDisplay->dwScreenFlatAddr, 
                                   color, 
                                   FALSE,
                                   pContext->pSurfRenderInt->dwPatchMode,
                                   pContext->pSurfRenderInt->dwPixelPitch,
                                   pContext->pSurfRenderInt->pixFmt.dwRGBBitCount,
                                   pContext->pSurfRenderInt->ddsCapsInt);
            }
            else
#endif // DX8_MULTISAMPLING || DX7_ANTIALIAS            
            {

                _DD_BLT_P3Clear(pThisDisplay, 
                                &rect, 
                                color, 
                                FALSE, 
                                FALSE,
                                _OP_GetSimpleSurfVidMemPtr(
                                    pContext->pSurfRenderInt),
                                pContext->pSurfRenderInt->dwPatchMode,
                                pContext->pSurfRenderInt->dwPixelPitch,
                                pContext->pSurfRenderInt->pixFmt.dwRGBBitCount);

            }
            
        }


        // Clear the z buffer
        if ((lpcd2->dwFlags & D3DCLEAR_ZBUFFER) &&
            (pContext->pSurfZBufferInt != NULL) )
        {
            P3RX_UPDATE_WRITE_MASK(dwDepthMask);

            if (bNoBlockFillZ)
            {
                P3_DMA_GET_BUFFER_ENTRIES(4);
                SEND_P3_DATA(FBSoftwareWriteMask, dwDepthMask);
                SEND_P3_DATA(FBDestReadMode, 
                             P3RX_FBDESTREAD_READENABLE(__PERMEDIA_ENABLE) |
                             P3RX_FBDESTREAD_ENABLE0(__PERMEDIA_ENABLE));
                P3_DMA_COMMIT_BUFFER();
            }                   

#if DX8_MULTISAMPLING || DX7_ANTIALIAS
            if (pContext->Flags & SURFACE_ANTIALIAS)
            {
                RECTL Temp = rect;
                Temp.left *= 2;
                Temp.right *= 2;
                Temp.top *= 2;
                Temp.bottom *= 2;
                _DD_BLT_P3Clear_AA(pThisDisplay, 
                                   &Temp, 
                                   pContext->dwAliasZBuffer - 
                                        pThisDisplay->dwScreenFlatAddr, 
                                   dwDepthValue, 
                                   bNoBlockFillZ,
                                   pContext->pSurfZBufferInt->dwPatchMode,
                                   pContext->pSurfZBufferInt->dwPixelPitch,
                                   pContext->pSurfZBufferInt->pixFmt.dwRGBBitCount,
                                   pContext->pSurfZBufferInt->ddsCapsInt);

            }
            else
#endif // DX8_MULTISAMPLING || DX7_ANTIALIAS            
            {
                _DD_BLT_P3Clear(pThisDisplay, 
                                &rect, 
                                dwDepthValue, 
                                bNoBlockFillZ, 
                                TRUE,
                                _OP_GetSimpleSurfVidMemPtr(
                                    pContext->pSurfZBufferInt),
                                pContext->pSurfZBufferInt->dwPatchMode,
                                pContext->pSurfZBufferInt->dwPixelPitch,
                                pContext->pSurfZBufferInt->pixFmt.dwRGBBitCount
                                );
            }
            
            if (bNoBlockFillZ)
            {
                P3_DMA_GET_BUFFER_ENTRIES(4);
                SEND_P3_DATA(FBSoftwareWriteMask, __GLINT_ALL_WRITEMASKS_SET);
                SEND_P3_DATA(FBDestReadMode, __PERMEDIA_DISABLE);
                P3_DMA_COMMIT_BUFFER();
            }                   
        }

        // Clear the stencil buffer
        if ((lpcd2->dwFlags & D3DCLEAR_STENCIL) &&
            (pContext->pSurfZBufferInt != NULL) )
        {
            P3RX_UPDATE_WRITE_MASK(dwStencilMask);

            if (bNoBlockFillStencil)
            {
                P3_DMA_GET_BUFFER_ENTRIES(4);
                SEND_P3_DATA(FBSoftwareWriteMask, dwStencilMask);
                SEND_P3_DATA(FBDestReadMode, 
                             P3RX_FBDESTREAD_READENABLE(__PERMEDIA_ENABLE) |
                             P3RX_FBDESTREAD_ENABLE0(__PERMEDIA_ENABLE));
                P3_DMA_COMMIT_BUFFER();
            }                   

#if DX8_MULTISAMPLING || DX7_ANTIALIAS
            if (pContext->Flags & SURFACE_ANTIALIAS)
            {
                RECTL Temp = rect;
                Temp.left *= 2;
                Temp.right *= 2;
                Temp.top *= 2;
                Temp.bottom *= 2;
                _DD_BLT_P3Clear_AA(pThisDisplay, 
                                   &Temp, 
                                   pContext->dwAliasZBuffer - 
                                        pThisDisplay->dwScreenFlatAddr, 
                                   dwStencilValue, 
                                   bNoBlockFillStencil,
                                   pContext->pSurfZBufferInt->dwPatchMode,
                                   pContext->pSurfZBufferInt->dwPixelPitch,
                                   pContext->pSurfZBufferInt->pixFmt.dwRGBBitCount,
                                   pContext->pSurfZBufferInt->ddsCapsInt
                                   );

            }
            else
#endif // DX8_MULTISAMPLING || DX7_ANTIALIAS            
            {

                _DD_BLT_P3Clear(pThisDisplay, 
                                &rect, 
                                dwStencilValue, 
                                bNoBlockFillStencil, 
                                TRUE,
                                _OP_GetSimpleSurfVidMemPtr(
                                    pContext->pSurfZBufferInt),
                                pContext->pSurfZBufferInt->dwPatchMode,
                                pContext->pSurfZBufferInt->dwPixelPitch,
                                pContext->pSurfZBufferInt->pixFmt.dwRGBBitCount
                                );
            }

            if (bNoBlockFillStencil)
            {
                P3_DMA_GET_BUFFER_ENTRIES(4);
                SEND_P3_DATA(FBSoftwareWriteMask, __GLINT_ALL_WRITEMASKS_SET);
                SEND_P3_DATA(FBDestReadMode, __PERMEDIA_DISABLE);
                P3_DMA_COMMIT_BUFFER();
            }                   

        }
        
Next_Rectl_To_Clear:
        ;

    } // while

    // Make sure the WriteMask is reset to it's default value
    {                                               
        P3_DMA_GET_BUFFER_ENTRIES(4);          

        SEND_P3_DATA(FBHardwareWriteMask, __GLINT_ALL_WRITEMASKS_SET);
        SEND_P3_DATA(FBDestReadMode, __PERMEDIA_DISABLE);

        P3_DMA_COMMIT_BUFFER();
    }

    DBG_CB_EXIT(_D3D_OP_Clear2, DD_OK);     
    return;
    
} // _D3D_OP_Clear2

//-----------------------------------------------------------------------------
//
// _OP_D3DBlt
//
// D3D Engine function for surface blt (TextureBlt, DP2Blt, etc)
//
// Assumption : caller must switch the context to DDRAW
//
//-----------------------------------------------------------------------------
BOOL
_OP_D3DBlt(
    P3_D3DCONTEXT*      pContext,
    P3_SURF_INTERNAL*   pSrcSurfInt,
    P3_SURF_INTERNAL*   pDstSurfInt,
    DWORD               dwDestSurfHandle,
    int                 iCurrSrcLOD, 
    int                 iCurrDstLOD,
    DWORD               dwBltFilterFlags,
    RECTL*              prSrc,
    RECTL*              prDest)
{
    P3_THUNKEDDATA*     pThisDisplay = pContext->pThisDisplay;
    MIPTEXTURE*         pSrcMipLevel;
    MIPTEXTURE*         pDstMipLevel;
    BOOL                bUseCopy;

    DISPDBG((DBGLVL,"Blitting level %d into level %d",
                    iCurrSrcLOD,
                    iCurrDstLOD));

    // Make sure levels are valid
    if ((iCurrSrcLOD >= pSrcSurfInt->iMipLevels) || 
        (iCurrDstLOD >= pDstSurfInt->iMipLevels)) 
    {
        return (FALSE);
    }

    // Set up the level pointers
    pSrcMipLevel = &pSrcSurfInt->MipLevels[iCurrSrcLOD];
    pDstMipLevel = &pDstSurfInt->MipLevels[iCurrDstLOD];

    // Check whether stretching is necessary
    if (((prSrc->right - prSrc->left) == (prDest->right - prDest->left)) && 
        ((prSrc->bottom - prSrc->top) == (prDest->bottom - prDest->top)) &&
        (pSrcSurfInt->pFormatSurface == pDstSurfInt->pFormatSurface))
    {
        bUseCopy = TRUE;
    }
    else
    {
        bUseCopy = FALSE;
    }

    ///////////////////////////////////////////////////////////////////
    // Here we handle all possible blt cases between different types
    // of memory and different scenarios of managed/unmanaged surfaces
    ///////////////////////////////////////////////////////////////////

#if DX7_TEXMANAGEMENT
    if ((0 == (pDstSurfInt->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)) &&
        (0 == (pSrcSurfInt->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)) )
#endif // DX7_TEXMANAGEMENT                
    {           
        //----------------------------------
        //----------------------------------
        // TEXBLT among non-managed textures
        //----------------------------------
        //----------------------------------

        if ((pSrcSurfInt->Location == SystemMemory) && 
            (pDstSurfInt->Location == VideoMemory))
        {
            //----------------------------
            // Do the system->videomem blt
            //----------------------------
            _DD_P3Download(pThisDisplay,
                           pSrcMipLevel->fpVidMem,
                           pDstMipLevel->fpVidMem,
                           pSrcSurfInt->dwPatchMode,
                           pDstSurfInt->dwPatchMode,
                           pSrcMipLevel->lPitch,
                           pDstMipLevel->lPitch,                                                             
                           pDstMipLevel->P3RXTextureMapWidth.Width,
                           pDstSurfInt->dwPixelSize,
                           prSrc,
                           prDest);                                                           
        }
        else if ((pSrcSurfInt->Location == VideoMemory) && 
                 (pDstSurfInt->Location == VideoMemory) && 
                 (bUseCopy))
        {
            //------------------------------            
            // Do the videomem->videomem blt
            //------------------------------
            _DD_BLT_P3CopyBlt(pThisDisplay,
                              pSrcMipLevel->fpVidMem,
                              pDstMipLevel->fpVidMem,
                              pSrcSurfInt->dwPatchMode,
                              pDstSurfInt->dwPatchMode,
                              pSrcMipLevel->P3RXTextureMapWidth.Width,
                              pDstMipLevel->P3RXTextureMapWidth.Width,
                              pSrcMipLevel->dwOffsetFromMemoryBase,                                 
                              pDstMipLevel->dwOffsetFromMemoryBase,
                              pDstSurfInt->dwPixelSize,
                              prSrc,
                              prDest);
        }          
        else if (((pSrcSurfInt->Location == AGPMemory) && 
                  (pDstSurfInt->Location == VideoMemory)) ||
                 ((pSrcSurfInt->Location == VideoMemory) &&
                  (pDstSurfInt->Location == VideoMemory) &&
                  (! bUseCopy)))
        {
            //-------------------------------           
            // Do the AGP mem -> videomem blt
            //-------------------------------    
            DDCOLORKEY ddck_dummy = { 0 , 0 };

            // We use the strecth blt because it handles AGP source 
            // surfaces, not becuase we should stretch the surface in any way                
            _DD_P3BltStretchSrcChDstCh(
                              pThisDisplay,
                              // src data
                              pSrcMipLevel->fpVidMem,
                              pSrcSurfInt->pFormatSurface,
                              pSrcSurfInt->dwPixelSize,
                              pSrcMipLevel->wWidth,
                              pSrcMipLevel->wHeight,
                              pSrcMipLevel->P3RXTextureMapWidth.Width,
                              pSrcMipLevel->P3RXTextureMapWidth.Layout,
                              pSrcMipLevel->dwOffsetFromMemoryBase,
                              pSrcSurfInt->dwFlagsInt,
                              &pSrcSurfInt->pixFmt,
                              pSrcSurfInt->Location == AGPMemory ? TRUE : FALSE,
                              // dest data
                              pDstMipLevel->fpVidMem,
                              pDstSurfInt->pFormatSurface,
                              pDstSurfInt->dwPixelSize,
                              pDstMipLevel->wWidth,
                              pDstMipLevel->wHeight,
                              pDstMipLevel->P3RXTextureMapWidth.Width,
                              pDstMipLevel->P3RXTextureMapWidth.Layout,
                              pDstMipLevel->dwOffsetFromMemoryBase,                              
                              0,                // dwBltFlags no special blt effects
                              0,                // dwBltDDFX  no special effects info
                              dwBltFilterFlags, // Default filter setting
                              ddck_dummy,       // BltSrcColorKey dummy arg
                              ddck_dummy,       // BltDestColorKey dummy arg
                              prSrc,
                              prDest);
        }
#if DX9_LWMIPMAP
        else if ((pSrcSurfInt->Location == SystemMemory) &&
                 (pDstSurfInt->Location == AGPMemory))
        {
            // Sublevels of light weight mipmap can't be locked,
            // so DX9 driver must be able to handle this combination

            _DD_BLT_SysMemToSysMemCopy(
                        pSrcMipLevel->fpVidMem,
                        pSrcMipLevel->lPitch,
                        pSrcSurfInt->dwBitDepth,
                        pDstMipLevel->fpVidMem,
                        pDstMipLevel->lPitch,
                        pDstSurfInt->dwBitDepth,
                        prSrc,
                        prDest);
        }
#endif
        else            
        {
            DISPDBG((ERRLVL,"Non-managed Tex Blt variation "
                            "unimplemented! (from %d into %d)",
                            pSrcSurfInt->Location,
                            pDstSurfInt->Location));
        }
    }            
#if DX7_TEXMANAGEMENT              
    else if (pSrcSurfInt->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
    {
        //----------------------------------
        //----------------------------------
        // TEXBLT from a managed texture
        //----------------------------------
        //----------------------------------

        if ((pDstSurfInt->Location == SystemMemory) ||
            (pDstSurfInt->dwCaps2 & DDSCAPS2_TEXTUREMANAGE))
        {
            //-------------------------------------------------
            // Do the Managed surf -> sysmem | managed surf blt
            //-------------------------------------------------    

            // make sure we'll reload the vidmem copy of the dest surf
            if (pDstSurfInt->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
            {
                __OP_MarkManagedSurfDirty(pContext,
                                          dwDestSurfHandle,
                                          pDstSurfInt);                                              
            }

            _DD_BLT_SysMemToSysMemCopy(
                        pSrcMipLevel->fpVidMem,
                        pSrcMipLevel->lPitch,
                        pSrcSurfInt->dwBitDepth,  
                        pDstMipLevel->fpVidMem,
                        pDstMipLevel->lPitch,  
                        pDstSurfInt->dwBitDepth, 
                        prSrc,
                        prDest);
        }
        else if (pDstSurfInt->Location == VideoMemory) 
        {
            //-------------------------------------------------
            // Do the Managed surf -> vidmem surf blt
            //-------------------------------------------------                  

            // This might be optimized by doing a vidmem->vidmem 
            // when the source managed texture has a vidmem copy

            _DD_P3Download(pThisDisplay,
                           pSrcMipLevel->fpVidMem,
                           pDstMipLevel->fpVidMem,
                           pSrcSurfInt->dwPatchMode,
                           pDstSurfInt->dwPatchMode,
                           pSrcMipLevel->lPitch,
                           pDstMipLevel->lPitch,                                                             
                           pDstMipLevel->P3RXTextureMapWidth.Width,
                           pDstSurfInt->dwPixelSize,
                           prSrc,
                           prDest);                                          
        }
        else            
        {
            DISPDBG((ERRLVL,"Src-managed Tex Blt variation unimplemented! "
                            "(from %d into %d)",
                            pSrcSurfInt->Location,
                            pDstSurfInt->Location));
        }
        
        
    }
    else if (pDstSurfInt->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
    {
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        // TEXBLT into a managed texture (except from a managed texture)
        //--------------------------------------------------------------
        //--------------------------------------------------------------

        // managed->managed is handled in the previous case

        if (pSrcSurfInt->Location == SystemMemory)
        {                
            //-------------------------------------------------
            // Do the sysmem surf -> managed surf blt
            //-------------------------------------------------    

            // make sure we'll reload the vidmem copy of the dest surf
            __OP_MarkManagedSurfDirty(pContext,
                                      dwDestSurfHandle,
                                      pDstSurfInt);                                              
                                                          
            _DD_BLT_SysMemToSysMemCopy(
                        pSrcMipLevel->fpVidMem,
                        pSrcMipLevel->lPitch,
                        pSrcSurfInt->dwBitDepth,  
                        pDstMipLevel->fpVidMem,
                        pDstMipLevel->lPitch,  
                        pDstSurfInt->dwBitDepth, 
                        prSrc, 
                        prDest);
        }
        else if (pSrcSurfInt->Location == VideoMemory)                
        {
            //-------------------------------------------------
            // Do the vidmem surf -> Managed surf blt
            //-------------------------------------------------                                  

            if (0 != pSrcMipLevel->fpVidMemTM)
            {
                // Destination is already in vidmem so instead of
                // "dirtying" the managed texture lets do the
                // vidmem->vidmem blt which is faster than doing the
                // update later (in the hope we'll really use it)

                _DD_BLT_P3CopyBlt(pThisDisplay,
                                  pSrcMipLevel->fpVidMem,
                                  pDstMipLevel->fpVidMemTM,
                                  pSrcSurfInt->dwPatchMode,
                                  pDstSurfInt->dwPatchMode,
                                  pSrcMipLevel->P3RXTextureMapWidth.Width,
                                  pDstMipLevel->P3RXTextureMapWidth.Width,
                                  pSrcMipLevel->dwOffsetFromMemoryBase,                                 
                                  pDstMipLevel->dwOffsetFromMemoryBase,
                                  pDstSurfInt->dwPixelSize,
                                  prSrc,
                                  prDest);                        
            } 
            else
            {
                // make sure we'll reload the 
                // vidmem copy of the dest surf
                __OP_MarkManagedSurfDirty(pContext,
                                          dwDestSurfHandle,
                                          pDstSurfInt);   
            }

            // Do slow mem mapped framebuffer blt into sysmem
            // The source surface lives in video mem so we need to get a
            // "real" sysmem address for it:                    
            _DD_BLT_SysMemToSysMemCopy(
                        D3DMIPLVL_GETPOINTER(pSrcMipLevel, pThisDisplay),
                        pSrcMipLevel->lPitch,
                        pSrcSurfInt->dwBitDepth,  
                        pDstMipLevel->fpVidMem,
                        pDstMipLevel->lPitch,  
                        pDstSurfInt->dwBitDepth, 
                        prSrc,
                        prDest);                    
        }
        else if (pSrcSurfInt->Location == AGPMemory)                     
        {
            // make sure we'll reload the vidmem copy of the dest surf
            __OP_MarkManagedSurfDirty(pContext,
                                      dwDestSurfHandle,
                                      pDstSurfInt);                                              
        
            _DD_BLT_SysMemToSysMemCopy(
                        pSrcMipLevel->fpVidMem,
                        pSrcMipLevel->lPitch,
                        pSrcSurfInt->dwBitDepth,  
                        pDstMipLevel->fpVidMem,
                        pDstMipLevel->lPitch,  
                        pDstSurfInt->dwBitDepth, 
                        prSrc,
                        prDest);                
        }
        else            
        {
            DISPDBG((ERRLVL,"Dest-managed Tex Blt variation unimplemented! "
                            "(from %d into %d)",
                            pSrcSurfInt->Location,
                            pDstSurfInt->Location));

            return (FALSE);
        }
        
    }
    else            
    {
        DISPDBG((ERRLVL,"Tex Blt variation unimplemented! "
                        "(from %d into %d)",
                        pSrcSurfInt->Location,
                        pDstSurfInt->Location));

        return (FALSE);
    }
#endif // DX7_TEXMANAGEMENT
        
    return (TRUE);
} // _OP_D3DBlt    


//-----------------------------------------------------------------------------
//
// _D3D_OP_TextureBlt
//
// This function processes the D3DDP2OP_TEXBLT DP2 command token.
//
//-----------------------------------------------------------------------------   
VOID _D3D_OP_TextureBlt(P3_D3DCONTEXT* pContext, 
                        P3_THUNKEDDATA*pThisDisplay,
                        D3DHAL_DP2TEXBLT* pBlt)
{
    P3_SURF_INTERNAL* pSrcTexture;
    P3_SURF_INTERNAL* pDestTexture;
    RECTL rSrc, rDest;
    int iMaxLogWidth, iCurrLogWidth;
    int iSrcLOD, iDestLOD, iCurrSrcLOD, iCurrDstLOD;
    BOOL  bMipMap, bMipMapLevelsMatch;

    DISPDBG((DBGLVL, "TextureBlt Source %d Dest %d", 
                      pBlt->dwDDSrcSurface,
                      pBlt->dwDDDestSurface));

    if (0 == pBlt->dwDDSrcSurface)
    {
        DISPDBG((ERRLVL,"Invalid handle TexBlt from %08lx to %08lx",
                        pBlt->dwDDSrcSurface,pBlt->dwDDDestSurface));
        return;
    }

    // Get the source texture structure pointer
    pSrcTexture = GetSurfaceFromHandle(pContext, pBlt->dwDDSrcSurface);    

    // Check that the source texture is valid
    if (pSrcTexture == NULL)
    {
        DISPDBG((ERRLVL, "ERROR: Source texture %d is invalid!",
                         pBlt->dwDDSrcSurface));
        return;
    }

    // Validate the destination texture handle
    if (0 == pBlt->dwDDDestSurface)
    {
#if DX7_TEXMANAGEMENT  

        // If we do texture management then a destination handle of 0
        // has the special meaning of preloading the source texture.

        if (!_D3D_TM_Preload_Tex_IntoVidMem(pContext, pSrcTexture))
        {
            DISPDBG((ERRLVL,"_D3D_OP_TextureBlt unable to "
                            "preload texture"));
        }
        
        return;
        
#else
        // If there's no driver texture managament support we can't go
        // on if the destination handle is 0
        DISPDBG((ERRLVL,"Invalid handle TexBlt from %08lx to %08lx",
                        pBlt->dwDDSrcSurface,pBlt->dwDDDestSurface));
        return;
#endif        
    }
    
    // Get the destination texture structure pointer for regular TexBlts
    pDestTexture = GetSurfaceFromHandle(pContext, pBlt->dwDDDestSurface);
    
    // Check that the destination texture is valid
    if (pDestTexture == NULL)
    {
        DISPDBG((ERRLVL, "ERROR: Dest texture %d is invalid!",
                         pBlt->dwDDDestSurface));
        return;
    }

    // Make sure the textures are of the same proportion
    if (((pSrcTexture->wWidth * pDestTexture->wHeight) != 
         (pSrcTexture->wHeight * pDestTexture->wWidth)) &&
        (pDestTexture->wWidth != 1) &&
        (pDestTexture->wHeight != 1)) 
    {
        DISPDBG((ERRLVL, "ERROR: TEXBLT the src and dest textures are not of the same proportion"));
        return;
    }

    // It is possible that the source and destination textures may contain 
    // different number of mipmap levels. In this case, the driver is 
    // expected to BitBlt the common levels. For example, if a 256x256 source 
    // texture has 8 mipmap levels, and if the destination is a 64x64 texture 
    // with 6 levels, then the driver should BitBlt the 6 corresponding levels 
    // from the source. The driver can expect the dimensions of the top mip 
    // level of destination texture to be always equal to or lesser than the 
    // dimensions of the top mip level of the source texture. 

    // It might also be the case that only one of the textures is mipmapped
    // Since we keep all relevant info also in the MipLevels substructre for
    // the surface, we can treat both surfaces as mipmapped even if only
    // one of them was created as such and proceed with the TexBlt.
    
    if (pSrcTexture->bMipMap || pDestTexture->bMipMap)
    {
        bMipMap = TRUE;
        iMaxLogWidth = max(pSrcTexture->logWidth, pDestTexture->logWidth);
        iCurrLogWidth = iMaxLogWidth;
        iSrcLOD = 0;                           // start LOD for src        
        iDestLOD = 0;                          // start LOD for dest
    }
    else
    {
        // just one level
        bMipMap = FALSE;       // No mipmapping cases to be handled
        iMaxLogWidth = iCurrLogWidth = iSrcLOD = iDestLOD = 0;           
    }

    // Init the rects from and into which we will blt . This is top level 
    // mipmap or non-mipmap texture,  just use rect from Blt.
    rSrc = pBlt->rSrc;

    // Create a destination rectangle for compatibility 
    // with the DD blitting function we are calling.
    rDest.left = pBlt->pDest.x;
    rDest.top = pBlt->pDest.y;
    rDest.right = (pBlt->rSrc.right - pBlt->rSrc.left) + rDest.left;
    rDest.bottom = (pBlt->rSrc.bottom - pBlt->rSrc.top) + rDest.top;
    
    // Switch to the DirectDraw context
    DDRAW_OPERATION(pContext, pThisDisplay);

    // Traverse all the mip map levels and try to match them for a blt to be 
    // done. If no mipmaps are present just do for the "first" and only 
    // levels  present.
    do 
    {
        DISPDBG((DBGLVL,"TEXBLT iteration %d %d %d %d",
                        iMaxLogWidth,iCurrLogWidth,iSrcLOD,iDestLOD));
    
        // Get the local surface pointers and make sure the level sizes
        // match in the case of mip map Texblts.
        if (bMipMap)
        {        
            bMipMapLevelsMatch = FALSE;        
            
            // Verify you only look at valid mipmap levels - they might 
            // be incomplete (and we want not to AV or access garbage!)
            // for example, a source 256x256 texture may contain 5 levels, 
            // but the destination 256x256 texture may contain 8. The 
            // driver is expected to safely handle this case, but it is 
            // not expected to produce correct results            
            if ((iSrcLOD < pSrcTexture->iMipLevels) &&
                (iDestLOD < pDestTexture->iMipLevels))
            {          
                DISPDBG((DBGLVL,"Checking match! %d vs. %d", 
                                pSrcTexture->MipLevels[iSrcLOD].logWidth,
                                pDestTexture->MipLevels[iDestLOD].logWidth)); 
              
                // Do we currently have two levels that match in size ?
                bMipMapLevelsMatch = 
                        ((pSrcTexture->MipLevels[iSrcLOD].logWidth == 
                          pDestTexture->MipLevels[iDestLOD].logWidth) &&
                         (pSrcTexture->MipLevels[iSrcLOD].logHeight ==
                          pDestTexture->MipLevels[iDestLOD].logHeight));
            }

            // Record which levels are we currently blitting
            iCurrSrcLOD = iSrcLOD;
            iCurrDstLOD = iDestLOD;
            
            // Get ready for next loop by updating the LODs to use
            // increment LOD# if we are currently looking at a level 
            // equal or smaller to mip maps level 0 size
            if (iCurrLogWidth <= pSrcTexture->logWidth)
            {
                iSrcLOD++;
            }

            if ((iCurrLogWidth <= pDestTexture->logWidth) &&
                (bMipMapLevelsMatch) &&
                ((pDestTexture->bMipMap) &&
                 (iDestLOD < (pDestTexture->iMipLevels - 1))))
            {
                iDestLOD++;
            }            

            // Decrease the width into next smaller level
            iCurrLogWidth--;
        }
        else
        {
            // Single level blt - we set bMipMapLevelsMatch in order to blt it!
            bMipMapLevelsMatch = TRUE;
            iCurrSrcLOD = 0;
            iCurrDstLOD = 0;
        }

        if (bMipMapLevelsMatch)
        {
            // Call the D3D blt engine to handle one level
            _OP_D3DBlt(pContext, 
                       pSrcTexture, 
                       pDestTexture, 
                       pBlt->dwDDDestSurface, 
                       iCurrSrcLOD, 
                       iCurrDstLOD, 
                       0,                   // Default filtering mode.
                       &rSrc, 
                       &rDest);
        }

        // Update transfer rectangles if mip mapping
        if (bMipMap)
        {
            DWORD right, bottom;

            // Update source rectangle , the regions to be copied in mipmap 
            // sub-levels can be obtained by dividing rSrc and pDest by 
            // 2 at each level. 
            rSrc.left >>= 1;
            rSrc.top >>= 1;
            right = (rSrc.right + 1) >> 1;
            bottom = (rSrc.bottom + 1) >> 1;
            rSrc.right = ((right - rSrc.left) < 1) ? (rSrc.left + 1) : (right);
            rSrc.bottom = ((bottom - rSrc.top ) < 1) ? (rSrc.top + 1) : (bottom);     

            // Update destination rectangle   
            rDest.left >>= 1;
            rDest.top >>= 1;
            right = (rDest.right + 1) >> 1;
            bottom = (rDest.bottom + 1) >> 1;
            rDest.right = ((right - rDest.left) < 1) ? (rDest.left + 1) : (right);
            rDest.bottom = ((bottom - rDest.top ) < 1) ? (rDest.top + 1) : (bottom);              
        }

    } while (bMipMap && ((iSrcLOD < pSrcTexture->iMipLevels) &&
                         (iDestLOD < pDestTexture->iMipLevels))); // do until we're done looking at 1x1

    // Switch back to the Direct3D context
    D3D_OPERATION(pContext, pThisDisplay);
    
} // _D3D_OP_TextureBlt

//-----------------------------------------------------------------------------
//
// _D3D_OP_SetRenderTarget
//
// Sets up the hw for the chosen render target and depth buffer
//
//-----------------------------------------------------------------------------
HRESULT 
_D3D_OP_SetRenderTarget(
    P3_D3DCONTEXT* pContext, 
    P3_SURF_INTERNAL* pRenderInt,
    P3_SURF_INTERNAL* pZBufferInt,
    BOOL bNewAliasBuffers)
{

    P3_SOFTWARECOPY* pSoftP3RX = &pContext->SoftCopyGlint;
    P3_THUNKEDDATA *pThisDisplay = pContext->pThisDisplay;
    DWORD AAMultiplier = 1;
    P3_DMA_DEFS();

    DBG_ENTRY(_D3D_OP_SetRenderTarget);

    // Verify the render target is in video memory
    if (pRenderInt)
    {    
        if (pRenderInt->ddsCapsInt.dwCaps & DDSCAPS_SYSTEMMEMORY) 
        {
            DISPDBG((ERRLVL, "ERROR: Render Surface in SYSTEM MEMORY"));
            return DDERR_GENERIC;
        }

#if DX9_RTZMAN
        // Make sure the video memory copy of the render target is ready
        if (! _D3D_TM_Preload_Tex_IntoVidMem(pContext, pRenderInt))
        {
            return DDERR_OUTOFMEMORY;
        }
#endif
    }
    else
    {
        // Must have a render target
        DISPDBG((ERRLVL, "ERROR: Render Surface is NULL"));
        return DDERR_GENERIC;
    }

    // If a Z Buffer verify it
    if (pZBufferInt)
    {
        if (pZBufferInt->ddsCapsInt.dwCaps & DDSCAPS_SYSTEMMEMORY) 
        {
            DISPDBG((ERRLVL, "ERROR: Z Surface in SYSTEM MEMORY, failing"));
            return DDERR_GENERIC;
        }

#if DX9_RTZMAN
        // Make sure the video memory copy of the Z buffer is ready
        if (! _D3D_TM_Preload_Tex_IntoVidMem(pContext, pZBufferInt))
        {
            return DDERR_OUTOFMEMORY;
        }
#endif
    }    

#if DX9_RTZMAN
    // Notify texture manager that the RT and ZBuf are in use
    _D3D_TM_Touch_Context(pContext);
#endif

    // Validate the RenderTarget to be 32 bit or 16 bit 565 
    if ((pRenderInt->pixFmt.dwRGBBitCount == 32     ) &&
        (pRenderInt->pixFmt.dwRBitMask == 0x00FF0000) &&
        (pRenderInt->pixFmt.dwGBitMask == 0x0000FF00) &&        
        (pRenderInt->pixFmt.dwBBitMask == 0x000000FF))
    {
        // were OK at 32bpp
    }
    else
    if ((pRenderInt->pixFmt.dwRGBBitCount == 16 ) &&
        (pRenderInt->pixFmt.dwRBitMask == 0xF800) &&
        (pRenderInt->pixFmt.dwGBitMask == 0x07E0) &&        
        (pRenderInt->pixFmt.dwBBitMask == 0x001F))    
    {
        // were OK at 16bpp
    }
    else
    {
        // we cant set our render target to this format !!!
        DISPDBG((WRNLVL, " SRT Error    !!!"));
        DISPDBG((WRNLVL, "    dwRGBBitCount:          0x%x", 
                                         pRenderInt->pixFmt.dwRGBBitCount));
        DISPDBG((WRNLVL, "    dwR/Y BitMask:          0x%x", 
                                         pRenderInt->pixFmt.dwRBitMask));
        DISPDBG((WRNLVL, "    dwG/U BitMask:          0x%x", 
                                         pRenderInt->pixFmt.dwGBitMask));
        DISPDBG((WRNLVL, "    dwB/V BitMask:          0x%x", 
                                         pRenderInt->pixFmt.dwBBitMask));
        DISPDBG((WRNLVL, "    dwRGBAlphaBitMask:      0x%x", 
                                         pRenderInt->pixFmt.dwRGBAlphaBitMask));
        return DDERR_GENERIC;          
    }

#if DX8_MULTISAMPLING
    // Decide whether antialising is requested and can be handled
    if ((pContext->pSurfRenderInt->dwSampling) &&
        (! _D3D_ST_CanRenderAntialiased(pContext, bNewAliasBuffers)))
    {
        return DDERR_OUTOFMEMORY;
    }
#endif // DX8_MULTISAMPLING

    // If we page flipped, clear the flag
    pThisDisplay->bFlippedSurface = FALSE;

    P3_DMA_GET_BUFFER();

    P3_ENSURE_DX_SPACE(46);
    WAIT_FIFO(26);
    
    pContext->pSurfRenderInt = pRenderInt;
    pContext->pSurfZBufferInt = pZBufferInt;

    // Check for Z Buffer
    if (pZBufferInt)
    {
        DDPIXELFORMAT* pZFormat = &pZBufferInt->pixFmt;

        if( pThisDisplay->dwDXVersion >= DX6_RUNTIME)
        {
            // On DX6 we look in the pixel format for the depth and stencil info.
            switch(pZFormat->dwZBufferBitDepth)
            {
            default:
                DISPDBG((ERRLVL,"ERROR: Unknown Z Pixel format!"));
                // Regard the buffer as 16 bit one and fall through

            case 16:
                if (pZFormat->dwStencilBitDepth == 1)
                {
                    // 15 bit Z, 1 bit stencil
                    pSoftP3RX->P3RXLBReadFormat.StencilPosition = 0; // Ignored in this mode
                    pSoftP3RX->P3RXLBReadFormat.StencilWidth = P3RX_STENCIL_WIDTH_1; 
                    pSoftP3RX->P3RXLBReadFormat.DepthWidth = P3RX_DEPTH_WIDTH_15;

                    pSoftP3RX->P3RXStencilMode.StencilWidth = P3RX_STENCIL_WIDTH_1;

                    pSoftP3RX->P3RXLBWriteFormat.StencilPosition = 0; // Ignored in this mode
                    pSoftP3RX->P3RXLBWriteFormat.StencilWidth = P3RX_STENCIL_WIDTH_1;
                    pSoftP3RX->P3RXLBWriteFormat.DepthWidth = P3RX_DEPTH_WIDTH_15;
                    
                    pSoftP3RX->P3RXDepthMode.Width = P3RX_DEPTH_WIDTH_15;
                    
                    pSoftP3RX->P3RXLBSourceReadMode.Packed16 = 1;
                    pSoftP3RX->P3RXLBDestReadMode.Packed16 = 1;
                    pSoftP3RX->P3RXLBWriteMode.Packed16 = 1;
                    pSoftP3RX->P3RXLBWriteMode.ByteEnables = 0x3;
                }
                else
                {
                    // 16 bit Z, no stencil
                    pSoftP3RX->P3RXLBReadFormat.StencilPosition = 0; // Ignored in this mode
                    pSoftP3RX->P3RXLBReadFormat.StencilWidth = P3RX_STENCIL_WIDTH_0; 
                    pSoftP3RX->P3RXStencilMode.StencilWidth = P3RX_STENCIL_WIDTH_0;
                    pSoftP3RX->P3RXLBReadFormat.DepthWidth = P3RX_DEPTH_WIDTH_16;
                    pSoftP3RX->P3RXLBWriteFormat.StencilPosition = 0; // Ignored in this mode
                    pSoftP3RX->P3RXLBWriteFormat.StencilWidth = P3RX_STENCIL_WIDTH_0;
                    pSoftP3RX->P3RXLBWriteFormat.DepthWidth = P3RX_DEPTH_WIDTH_16;
                    pSoftP3RX->P3RXDepthMode.Width = P3RX_DEPTH_WIDTH_16;
                    pSoftP3RX->P3RXLBWriteMode.ByteEnables = 0x3;
                    pSoftP3RX->P3RXLBSourceReadMode.Packed16 = 1;
                    pSoftP3RX->P3RXLBDestReadMode.Packed16 = 1;
                    pSoftP3RX->P3RXLBWriteMode.Packed16 = 1;
                }
                break;
                
            case 32:
                if (pZFormat->dwStencilBitDepth == 8)
                {
                    // 24 bit Z, 8 bit stencil
                    pSoftP3RX->P3RXLBReadFormat.StencilPosition = P3RX_STENCIL_POSITION_24; 
                    pSoftP3RX->P3RXLBReadFormat.StencilWidth = P3RX_STENCIL_WIDTH_8; 
                    pSoftP3RX->P3RXStencilMode.StencilWidth = P3RX_STENCIL_WIDTH_8;
                    pSoftP3RX->P3RXLBReadFormat.DepthWidth = P3RX_DEPTH_WIDTH_24;
                    pSoftP3RX->P3RXLBWriteFormat.StencilPosition = P3RX_STENCIL_POSITION_24; 
                    pSoftP3RX->P3RXLBWriteFormat.StencilWidth = P3RX_STENCIL_WIDTH_8;
                    pSoftP3RX->P3RXLBWriteFormat.DepthWidth = P3RX_DEPTH_WIDTH_24;
                    pSoftP3RX->P3RXDepthMode.Width = P3RX_DEPTH_WIDTH_24;
                    pSoftP3RX->P3RXLBWriteMode.ByteEnables = 0xF;
                    pSoftP3RX->P3RXLBSourceReadMode.Packed16 = 0;
                    pSoftP3RX->P3RXLBDestReadMode.Packed16 = 0;
                    pSoftP3RX->P3RXLBWriteMode.Packed16 = 0;
                }
                else
                {
                    // 32 bit Z, no stencil
                    pSoftP3RX->P3RXLBReadFormat.StencilPosition = 0;
                    pSoftP3RX->P3RXLBReadFormat.StencilWidth = P3RX_STENCIL_WIDTH_0; 
                    pSoftP3RX->P3RXStencilMode.StencilWidth = P3RX_STENCIL_WIDTH_0;
                    pSoftP3RX->P3RXLBReadFormat.DepthWidth = P3RX_DEPTH_WIDTH_32;
                    pSoftP3RX->P3RXLBWriteFormat.StencilPosition = 0; 
                    pSoftP3RX->P3RXLBWriteFormat.StencilWidth = P3RX_STENCIL_WIDTH_0;
                    pSoftP3RX->P3RXLBWriteFormat.DepthWidth = P3RX_DEPTH_WIDTH_32;
                    pSoftP3RX->P3RXDepthMode.Width = P3RX_DEPTH_WIDTH_32;
                    pSoftP3RX->P3RXLBWriteMode.ByteEnables = 0xF;
                    pSoftP3RX->P3RXLBSourceReadMode.Packed16 = 0;
                    pSoftP3RX->P3RXLBDestReadMode.Packed16 = 0;
                    pSoftP3RX->P3RXLBWriteMode.Packed16 = 0;

                }
                break;
            }

        }
        else
        // On DX5 we don't look at the pixel format, just the depth of the Z Buffer.
        {
            // Choose the correct Z Buffer depth
            switch(pZBufferInt->pixFmt.dwRGBBitCount)
            {
            default:
                DISPDBG((ERRLVL,"ERROR: Unknown depth format in _D3D_OP_SetRenderTarget!"));
                // Regard the buffer as 16 bit one and fall through

            case 16:
                pSoftP3RX->P3RXLBReadFormat.DepthWidth = __GLINT_DEPTH_WIDTH_16;
                pSoftP3RX->P3RXLBWriteFormat.DepthWidth = __GLINT_DEPTH_WIDTH_16;
                pSoftP3RX->P3RXDepthMode.Width = __GLINT_DEPTH_WIDTH_16;
                pSoftP3RX->P3RXLBWriteMode.ByteEnables = 0x3;
                pSoftP3RX->P3RXLBSourceReadMode.Packed16 = 1;
                pSoftP3RX->P3RXLBDestReadMode.Packed16 = 1;
                pSoftP3RX->P3RXLBWriteMode.Packed16 = 1;
                break;
                
            case 24:
                pSoftP3RX->P3RXLBReadFormat.DepthWidth = __GLINT_DEPTH_WIDTH_24;
                pSoftP3RX->P3RXLBWriteFormat.DepthWidth = __GLINT_DEPTH_WIDTH_24;
                pSoftP3RX->P3RXDepthMode.Width = __GLINT_DEPTH_WIDTH_24;
                pSoftP3RX->P3RXLBWriteMode.ByteEnables = 0x7;
                pSoftP3RX->P3RXLBSourceReadMode.Packed16 = 0;
                pSoftP3RX->P3RXLBDestReadMode.Packed16 = 0;
                pSoftP3RX->P3RXLBWriteMode.Packed16 = 0;
                break;
                
            case 32:
                pSoftP3RX->P3RXLBReadFormat.DepthWidth = __GLINT_DEPTH_WIDTH_32;
                pSoftP3RX->P3RXLBWriteFormat.DepthWidth = __GLINT_DEPTH_WIDTH_32;
                pSoftP3RX->P3RXDepthMode.Width = __GLINT_DEPTH_WIDTH_32;
                pSoftP3RX->P3RXLBWriteMode.ByteEnables = 0xF;
                pSoftP3RX->P3RXLBSourceReadMode.Packed16 = 0;
                pSoftP3RX->P3RXLBDestReadMode.Packed16 = 0;
                pSoftP3RX->P3RXLBWriteMode.Packed16 = 0;
                break;
                
            }
        }

        pSoftP3RX->P3RXLBSourceReadMode.Layout = pZBufferInt->dwPatchMode;
        pSoftP3RX->P3RXLBDestReadMode.Layout = pZBufferInt->dwPatchMode;
        pSoftP3RX->P3RXLBWriteMode.Layout = pZBufferInt->dwPatchMode;
    } //  if (pZBufferInt) 
    
    switch (pRenderInt->dwPixelSize)
    {
    case __GLINT_8BITPIXEL:
        // 8 Bit color index mode
        pSoftP3RX->DitherMode.ColorFormat = 
            pSoftP3RX->P3RXAlphaBlendColorMode.ColorFormat = P3RX_ALPHABLENDMODE_COLORFORMAT_CI;
        SEND_P3_DATA(PixelSize, 2 - __GLINT_8BITPIXEL);
        break;
        
    case __GLINT_16BITPIXEL:
        if (pThisDisplay->ddpfDisplay.dwRBitMask == 0x7C00)
        {
            // 5551 format
            pSoftP3RX->DitherMode.ColorFormat = P3RX_DITHERMODE_COLORFORMAT_5551;   
            pSoftP3RX->P3RXAlphaBlendColorMode.ColorFormat = P3RX_ALPHABLENDMODE_COLORFORMAT_5551;
        }
        else
        {
            // 565 format
            pSoftP3RX->DitherMode.ColorFormat = P3RX_DITHERMODE_COLORFORMAT_565;    
            pSoftP3RX->P3RXAlphaBlendColorMode.ColorFormat = P3RX_ALPHABLENDMODE_COLORFORMAT_565;
        }
        SEND_P3_DATA(PixelSize, 2 - __GLINT_16BITPIXEL);
        break;
        
    case __GLINT_24BITPIXEL:
    case __GLINT_32BITPIXEL:
        // 32 Bit Color Index mode
        pSoftP3RX->DitherMode.ColorFormat =
            pSoftP3RX->P3RXAlphaBlendColorMode.ColorFormat = P3RX_ALPHABLENDMODE_COLORFORMAT_8888;
        SEND_P3_DATA(PixelSize, 2 - __GLINT_32BITPIXEL);
        break;
    }

    pSoftP3RX->P3RXFBDestReadMode.Layout0 = pRenderInt->dwPatchMode;
    pSoftP3RX->P3RXFBWriteMode.Layout0 = pRenderInt->dwPatchMode;
    pSoftP3RX->P3RXFBSourceReadMode.Layout = pRenderInt->dwPatchMode;

    COPY_P3_DATA(FBWriteMode, pSoftP3RX->P3RXFBWriteMode);
    COPY_P3_DATA(FBDestReadMode, pSoftP3RX->P3RXFBDestReadMode);
    COPY_P3_DATA(FBSourceReadMode, pSoftP3RX->P3RXFBSourceReadMode);

#if DX8_MULTISAMPLING || DX7_ANTIALIAS
    if (!(pContext->Flags & SURFACE_ANTIALIAS) ||
         (pContext->dwAliasBackBuffer == 0))
    {
#endif // DX8_MULTISAMPLING || DX7_ANTIALIAS
        pContext->PixelOffset = (DWORD)(_OP_GetSimpleSurfVidMemPtr(pRenderInt) - 
                                                 pThisDisplay->dwScreenFlatAddr );

        if (pContext->pSurfZBufferInt)
        {
            pContext->ZPixelOffset = (DWORD)(_OP_GetSimpleSurfVidMemPtr(pZBufferInt) - 
                                                        pThisDisplay->dwScreenFlatAddr);
        }
        AAMultiplier = 1;

        SEND_P3_DATA(PixelSize, (2 - pRenderInt->dwPixelSize));
#if DX8_MULTISAMPLING || DX7_ANTIALIAS
    }
    else
    {
        pContext->PixelOffset = pContext->dwAliasPixelOffset;
        pContext->ZPixelOffset = pContext->dwAliasZPixelOffset;
        AAMultiplier = 2;
    }
#endif // DX8_MULTISAMPLING || DX7_ANTIALIAS

    COPY_P3_DATA(AlphaBlendColorMode, pSoftP3RX->P3RXAlphaBlendColorMode);
    COPY_P3_DATA(DitherMode, pSoftP3RX->DitherMode);

    SEND_P3_DATA(FBWriteBufferAddr0, pContext->PixelOffset);
    SEND_P3_DATA(FBDestReadBufferAddr0, pContext->PixelOffset);
    SEND_P3_DATA(FBSourceReadBufferAddr, pContext->PixelOffset);

    SEND_P3_DATA(FBWriteBufferWidth0, 
                        pContext->pSurfRenderInt->dwPixelPitch  * AAMultiplier);
    SEND_P3_DATA(FBDestReadBufferWidth0, 
                        pContext->pSurfRenderInt->dwPixelPitch * AAMultiplier);
    SEND_P3_DATA(FBSourceReadBufferWidth, 
                        pContext->pSurfRenderInt->dwPixelPitch * AAMultiplier);

    WAIT_FIFO(20);

    // Is there a Z Buffer?
    if (pContext->pSurfZBufferInt != NULL)
    {
        // Offset is in BYTES
        SEND_P3_DATA(LBSourceReadBufferAddr, pContext->ZPixelOffset);
        SEND_P3_DATA(LBDestReadBufferAddr, pContext->ZPixelOffset);
        SEND_P3_DATA(LBWriteBufferAddr, pContext->ZPixelOffset);
        
        pSoftP3RX->P3RXLBWriteMode.Width = 
                        pContext->pSurfZBufferInt->dwPixelPitch * AAMultiplier;
        pSoftP3RX->P3RXLBSourceReadMode.Width = 
                        pContext->pSurfZBufferInt->dwPixelPitch * AAMultiplier;
        pSoftP3RX->P3RXLBDestReadMode.Width = 
                        pContext->pSurfZBufferInt->dwPixelPitch * AAMultiplier;
                        
        COPY_P3_DATA(LBDestReadMode, pSoftP3RX->P3RXLBDestReadMode);
        COPY_P3_DATA(LBSourceReadMode, pSoftP3RX->P3RXLBSourceReadMode);
        COPY_P3_DATA(LBWriteMode, pSoftP3RX->P3RXLBWriteMode);
        
        COPY_P3_DATA(StencilMode, pSoftP3RX->P3RXStencilMode);
        COPY_P3_DATA(LBReadFormat, pSoftP3RX->P3RXLBReadFormat);
        COPY_P3_DATA(LBWriteFormat, pSoftP3RX->P3RXLBWriteFormat);

        COPY_P3_DATA(DepthMode, pSoftP3RX->P3RXDepthMode);
    }

    DIRTY_VIEWPORT(pContext);

    P3_DMA_COMMIT_BUFFER();

#if DX8_DDI

    // Bind the D3DRS_COLORWRITEENABLE to actually P3RX hardware FB writemasks
    // For DX7 and below, writemask is controlled by D3DRENDERSTATE_PLANEMASK
    if (pContext->dwDXInterface >= 4)
    {
        _D3D_ST_ProcessOneRenderState(
            pContext, 
            D3DRS_COLORWRITEENABLE,
            pContext->RenderStates[D3DRS_COLORWRITEENABLE]);
    }
#endif

    DBG_EXIT(_D3D_OP_SetRenderTarget,0);

    return DD_OK;
    
} // _D3D_OP_SetRenderTarget


//-----------------------------------------------------------------------------
//
// _D3D_OP_SceneCapture
//
// This function is called twice, once at the start of the rendering,
// and once at the end of the rendering.  The start is ignored, but 
// the end might be used to ensure that the DMA buffer has been flushed.
// This is needed for the case where a scene has little in it, and 
// doesn't fill the buffer up.
//
//-----------------------------------------------------------------------------
VOID  
_D3D_OP_SceneCapture(
    P3_D3DCONTEXT *pContext,
    DWORD dwFlag)
{
    P3_THUNKEDDATA *pThisDisplay;
    
    pThisDisplay = pContext->pThisDisplay;
    
    if (dwFlag == D3DHAL_SCENE_CAPTURE_START)
    {
        DISPDBG((DBGLVL,"Scene Start"));
#if DX9_ASYNC_NOTIFICATIONS
        // Reset Vertex Stats
        pContext->vertexStats.NumRenderedTriangles = 0;
        pContext->vertexStats.NumExtraClippingTriangles = 0;
#endif // DX9_ASYNC_NOTIFICATIONS        
    }
    else if (dwFlag == D3DHAL_SCENE_CAPTURE_END)    
    {   
#if DX8_MULTISAMPLING || DX7_ANTIALIAS
        if (pContext->Flags & SURFACE_ANTIALIAS) 
        {
            // Since we were antialiasing we need to put the data where 
            // the user asked which requires a copy from our AA buffer 
            // into the true target buffer

            // P3 Shrinking is done in the DDRAW context. This means you 
            // don't have to save and restore the state around the call 
            // - the next D3D_OPERATION will recover for you
            DDRAW_OPERATION(pContext, pThisDisplay);

            P3RX_AA_Shrink(pContext);
        }
#endif // DX8_MULTISAMPLING || DX7_ANTIALIAS

        DISPDBG((DBGLVL,"Scene End"));
    }
      
    return;
} // _D3D_OP_SceneCapture

#if DX7_TEXMANAGEMENT  
//-----------------------------------------------------------------------------
//
// __OP_MarkManagedSurfDirty
//
// Make sure textures are setup again if the texture is being used in any of
// the texture stages (for reloading purpouses) and make sure we mark it as
// dirty (since we're modifying the sysmem copy of the texture)
//
//-----------------------------------------------------------------------------   
VOID __OP_MarkManagedSurfDirty(P3_D3DCONTEXT* pContext, 
                               DWORD dwSurfHandle,
                               P3_SURF_INTERNAL* pTexture)
{
    // If the destination texture is in use in any of the texture
    // stages, make sure hw gets re-setup again before using it.
    if ((pContext->TextureStageState[0].m_dwVal[D3DTSS_TEXTUREMAP] 
                                    == dwSurfHandle) ||
        (pContext->TextureStageState[1].m_dwVal[D3DTSS_TEXTUREMAP]
                                    == dwSurfHandle))
    {
        DIRTY_TEXTURE(pContext);
    }

    // Mark the destination texture as needing to be updated
    // into vidmem before using it.
    pTexture->m_bTMNeedUpdate = TRUE;
    
} // __OP_MarkManagedSurfDirty

//-----------------------------------------------------------------------------
//
// _D3D_OP_SetTexLod
//
// This function processes the D3DDP2OP_SETTEXLOD DP2 command token.
// This communicates to the texture manager the most detailed mip map level
// required to load for a given managed surface. 
//
//----------------------------------------------------------------------------- 
VOID
_D3D_OP_SetTexLod(
    P3_D3DCONTEXT *pContext,
    D3DHAL_DP2SETTEXLOD* pSetTexLod)
{
    P3_SURF_INTERNAL* pTexture;
    
    // Get the source texture structure pointer
    pTexture = GetSurfaceFromHandle(pContext, pSetTexLod->dwDDSurface);    

    if (pTexture == NULL)
    {
        return;
    }

    // Set up the HW texture states again if this texture is in use
    // and the new LOD value is smaller than the current setting.
    if ((pContext->TextureStageState[0].m_dwVal[D3DTSS_TEXTUREMAP]
                                    == pSetTexLod->dwDDSurface) ||
        (pContext->TextureStageState[1].m_dwVal[D3DTSS_TEXTUREMAP]
                                    == pSetTexLod->dwDDSurface))
    {
        DIRTY_TEXTURE(pContext);
    }

    // Change the texture's largest level to be actually used
    pTexture->m_dwTexLOD = pSetTexLod->dwLOD; 

} // _D3D_OP_SetTexLod

//-----------------------------------------------------------------------------
//
// _D3D_OP_SetPriority
//
// This function processes the D3DDP2OP_SETPRIORITY DP2 command token.
//
//----------------------------------------------------------------------------- 
VOID
_D3D_OP_SetPriority(
    P3_D3DCONTEXT *pContext,
    D3DHAL_DP2SETPRIORITY* pSetPriority)
{
    P3_SURF_INTERNAL* pTexture;
    
    // Get the source texture structure pointer
#if WNT_DDRAW
    pTexture = GetSurfaceFromHandle(pContext, pSetPriority->dwDDDestSurface);
#else
    pTexture = GetSurfaceFromHandle(pContext, pSetPriority->dwDDSurface);
#endif

    if (NULL != pTexture)
    {
        // Managed resources should be evicted depending on their priorities.
        // If of same priority then LRU is used to break the tie.
        pTexture->m_dwPriority = pSetPriority->dwPriority; 
    }

} // _D3D_OP_SetPriority

#if DX8_DDI
//-----------------------------------------------------------------------------
//
// _D3D_OP_AddDirtyRect
//
// This function processes the D3DDP2OP_ADDDIRTYRECT DP2 command token.
//
//----------------------------------------------------------------------------- 
VOID
_D3D_OP_AddDirtyRect(
    P3_D3DCONTEXT *pContext,
    D3DHAL_DP2ADDDIRTYRECT* pAddDirtyRect)
{
    P3_SURF_INTERNAL* pTexture;
    
    // Get the source texture structure pointer
    pTexture = GetSurfaceFromHandle(pContext, pAddDirtyRect->dwSurface);    

    if (NULL != pTexture)
    {
        //azn TODO
        // As a first implementation in this driver we mark the whole surface 
        // as dirty instead of marking just the indicated rect - which could be
        // transferred more efficiently
        __OP_MarkManagedSurfDirty(pContext, 
                                  pAddDirtyRect->dwSurface,
                                  pTexture);
    }

} // _D3D_OP_AddDirtyRect

//-----------------------------------------------------------------------------
//
// _D3D_OP_AddDirtyBox
//
// This function processes the D3DDP2OP_ADDDIRTYBOX DP2 command token.
//
//----------------------------------------------------------------------------- 
VOID
_D3D_OP_AddDirtyBox(
    P3_D3DCONTEXT *pContext,
    D3DHAL_DP2ADDDIRTYBOX* pAddDirtyBox)
{
    P3_SURF_INTERNAL* pTexture;
    
    // Get the source texture structure pointer
    pTexture = GetSurfaceFromHandle(pContext, pAddDirtyBox->dwSurface);    

    if (NULL != pTexture)
    {
        //azn TODO
        // As a first implementation in this driver we mark the whole surface 
        // as dirty instead of marking just the indicated rect - which could be
        // transferred more efficiently
        __OP_MarkManagedSurfDirty(pContext, 
                                  pAddDirtyBox->dwSurface,
                                  pTexture);
    }

} // _D3D_OP_AddDirtyBox
#endif
#endif // DX7_TEXMANAGEMENT
       


#if DX8_3DTEXTURES

//-----------------------------------------------------------------------------
//
// __OP_BasicVolumeBlt
//
// This function blts one single level/slice at a time for volume textures
//
//-----------------------------------------------------------------------------   
VOID __OP_BasicVolumeBlt(P3_D3DCONTEXT* pContext, 
                         P3_THUNKEDDATA*pThisDisplay,
                         P3_SURF_INTERNAL* pSrcTexture,
                         P3_SURF_INTERNAL* pDestTexture,
                         DWORD dwDestSurfHandle,
                         RECTL *prSrc, 
                         RECTL *prDest)
{
#if DX7_TEXMANAGEMENT
    if ((0 == (pDestTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)) &&
        (0 == (pSrcTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)) )
#endif // DX7_TEXMANAGEMENT     
    {
        if ((pSrcTexture->Location == SystemMemory) && 
            (pDestTexture->Location == VideoMemory))
        {
            //----------------------------
            // Do the system->videomem blt
            //----------------------------
            _DD_P3Download(pThisDisplay,
                           pSrcTexture->fpVidMem,
                           pDestTexture->fpVidMem,
                           pSrcTexture->dwPatchMode,
                           pDestTexture->dwPatchMode,
                           pSrcTexture->lPitch,
                           pDestTexture->lPitch,
                           pDestTexture->dwPixelPitch,
                           pDestTexture->dwPixelSize,
                           prSrc,
                           prDest);
        }
#if DX9_LWMIPMAP
        else if ((pSrcTexture->Location == SystemMemory) &&
                 (pDestTexture->Location == AGPMemory))
        {
            // Sublevels of light weight mipmap can't be locked,
            // so DX9 driver must be able to handle this combination

            _DD_BLT_SysMemToSysMemCopy(
                        pSrcTexture->fpVidMem,
                        pSrcTexture->lPitch,
                        pSrcTexture->dwBitDepth,
                        pDestTexture->fpVidMem,
                        pDestTexture->lPitch,
                        pDestTexture->dwBitDepth,
                        prSrc,
                        prDest);
        }
#endif
        else
        {
            DISPDBG((ERRLVL, "ERROR: __OP_BasicVolumeBlt b3DTexture (%d -> %d)"
                             "not suported yet!",
                         pSrcTexture->Location,
                         pDestTexture->Location));
        }
    }
#if DX7_TEXMANAGEMENT              
    else if (pSrcTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
    {
        //----------------------------------
        //----------------------------------
        // TEXBLT from a managed texture
        //----------------------------------
        //----------------------------------

        if ((pDestTexture->Location == SystemMemory) ||
            (pDestTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE))
        {
            //-------------------------------------------------
            // Do the Managed surf -> sysmem | managed surf blt
            //-------------------------------------------------    

            // make sure we'll reload the vidmem copy of the dest surf
            if (pDestTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
            {
                __OP_MarkManagedSurfDirty(pContext,
                                          dwDestSurfHandle,
                                          pDestTexture);                                              
            }

            _DD_BLT_SysMemToSysMemCopy(
                        pSrcTexture->fpVidMem,
                        pSrcTexture->lPitch,
                        pSrcTexture->dwBitDepth,  
                        pDestTexture->fpVidMem,
                        pDestTexture->lPitch,  
                        pDestTexture->dwBitDepth, 
                        prSrc,
                        prDest);
        }
        else if (pDestTexture->Location == VideoMemory) 
        {
            //-------------------------------------------------
            // Do the Managed surf -> vidmem surf blt
            //-------------------------------------------------                  

            // This might be optimized by doing a vidmem->vidmem 
            // when the source managed texture has a vidmem copy
            
            _DD_P3Download(pThisDisplay,
                           pSrcTexture->fpVidMem,
                           pDestTexture->fpVidMem,
                           pSrcTexture->dwPatchMode,
                           pDestTexture->dwPatchMode,
                           pSrcTexture->lPitch,
                           pDestTexture->lPitch,                                                             
                           pDestTexture->dwPixelPitch,
                           pDestTexture->dwPixelSize,
                           prSrc,
                           prDest);                                          
        }
        else            
        {
            DISPDBG((ERRLVL,"Src-managed __OP_BasicVolumeBlt variation "
                            "unimplemented! (from %d into %d)",
                            pSrcTexture->Location,
                            pDestTexture->Location));
        }
        
        
    }
    else if (pDestTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
    {
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        // TEXBLT into a managed texture (except from a managed texture)
        //--------------------------------------------------------------
        //--------------------------------------------------------------

        // managed->managed is handled in the previous case

        if (pSrcTexture->Location == SystemMemory)
        {                
            //-------------------------------------------------
            // Do the sysmem surf -> managed surf blt
            //-------------------------------------------------    

            // make sure we'll reload the vidmem copy of the dest surf
            __OP_MarkManagedSurfDirty(pContext,
                                      dwDestSurfHandle,
                                      pDestTexture);                                              
                                                          
            _DD_BLT_SysMemToSysMemCopy(
                        pSrcTexture->fpVidMem,
                        pSrcTexture->lPitch,
                        pSrcTexture->dwBitDepth,  
                        pDestTexture->fpVidMem,
                        pDestTexture->lPitch,  
                        pDestTexture->dwBitDepth, 
                        prSrc, 
                        prDest);
        }
        else if (pSrcTexture->Location == VideoMemory)                
        {
            //-------------------------------------------------
            // Do the vidmem surf -> Managed surf blt
            //-------------------------------------------------                                  

              if (0 != pSrcTexture->MipLevels[0].fpVidMemTM)
            {
                // Destination is already in vidmem so instead of
                // "dirtying" the managed texture lets do the
                // vidmem->vidmem blt which is faster than doing the
                // update later (in the hope we'll really use it)

                _DD_BLT_P3CopyBlt(pThisDisplay,
                                  pSrcTexture->fpVidMem,
                                  pDestTexture->MipLevels[0].fpVidMemTM,
                                  pSrcTexture->dwPatchMode,
                                  pDestTexture->dwPatchMode,
                                  pSrcTexture->dwPixelPitch,
                                  pDestTexture->dwPixelPitch,
                                  pSrcTexture->MipLevels[0].dwOffsetFromMemoryBase,                                 
                                  pDestTexture->MipLevels[0].dwOffsetFromMemoryBase,
                                  pDestTexture->dwPixelSize,
                                  prSrc,
                                  prDest);                        
            } 
            else
            {
                // make sure we'll reload the 
                // vidmem copy of the dest surf
                __OP_MarkManagedSurfDirty(pContext,
                                          dwDestSurfHandle,
                                          pDestTexture);   
            }

            // Do slow mem mapped framebuffer blt into sysmem
            // The source surface lives in video mem so we need to get a
            // "real" sysmem address for it:                    
            _DD_BLT_SysMemToSysMemCopy(
                        D3DSURF_GETPOINTER(pSrcTexture, pThisDisplay),
                        pSrcTexture->lPitch,
                        pSrcTexture->dwBitDepth,  
                        pDestTexture->fpVidMem,
                        pDestTexture->lPitch,  
                        pDestTexture->dwBitDepth, 
                        prSrc,
                        prDest);
        }
        else if (pSrcTexture->Location == AGPMemory)                     
        {
            // make sure we'll reload the vidmem copy of the dest surf
            __OP_MarkManagedSurfDirty(pContext,
                                      dwDestSurfHandle,
                                      pDestTexture);                                              
        
            _DD_BLT_SysMemToSysMemCopy(
                        pSrcTexture->fpVidMem,
                        pSrcTexture->lPitch,
                        pSrcTexture->dwBitDepth,  
                        pDestTexture->fpVidMem,
                        pDestTexture->lPitch,  
                        pDestTexture->dwBitDepth, 
                        prSrc,
                        prDest);                
        }
        else            
        {
            DISPDBG((ERRLVL,"Dest-managed __OP_BasicVolumeBlt variation "
                            "unimplemented! (from %d into %d)",
                            pSrcTexture->Location,
                            pDestTexture->Location));
        }
        
    }
    else            
    {
        DISPDBG((ERRLVL,"__OP_BasicVolumeBlt variation unimplemented! "
                        "(from %d into %d)",
                        pSrcTexture->Location,
                        pDestTexture->Location));
    }
#endif // DX7_TEXMANAGEMENT        


} // __OP_BasicVolumeBlt

//-----------------------------------------------------------------------------
//
// _D3D_OP_VolumeBlt
//
// This function processes the D3DDP2OP_VOLUMEBLT DP2 command token.
//
//-----------------------------------------------------------------------------   
VOID _D3D_OP_VolumeBlt(P3_D3DCONTEXT* pContext, 
                       P3_THUNKEDDATA*pThisDisplay,
                       D3DHAL_DP2VOLUMEBLT* pBlt)
{
    LPDDRAWI_DDRAWSURFACE_LCL pSrcLcl;
    LPDDRAWI_DDRAWSURFACE_LCL pDestLcl;
    P3_SURF_INTERNAL* pSrcTexture;
    P3_SURF_INTERNAL* pDestTexture;
    RECTL rSrc, rDest;
    DWORD dwSrcCurrDepth, dwDestCurrDepth, dwEndDepth;

    // Get the texture structure pointers
    pSrcTexture = GetSurfaceFromHandle(pContext, pBlt->dwDDSrcSurface);
    pDestTexture = GetSurfaceFromHandle(pContext, pBlt->dwDDDestSurface);
    
    // Check that the textures are valid
    if (pSrcTexture == NULL)
    {
        DISPDBG((ERRLVL, "ERROR: Source texture %d is invalid!",
                         pBlt->dwDDSrcSurface));
        return;
    }

    if (pDestTexture == NULL)
    {
        DISPDBG((ERRLVL, "ERROR: Dest texture %d is invalid!",
                         pBlt->dwDDDestSurface));
        return;
    }

    // If we are going to blt 3D texture, both have to be 3D texture
    if ((pSrcTexture->b3DTexture == FALSE) != (pDestTexture->b3DTexture == FALSE))
    {
        DISPDBG((ERRLVL, "ERROR: TEXBLT b3DTexture (%d %d)does not match!",
                         pSrcTexture->b3DTexture,
                         pDestTexture->b3DTexture));
        return;
    }

    // Do we blt whole 3D texture ?
    // Is its height smaller than 4095 when being treated as 2D image ?
    // (Perm3 can only blt 2D images with height up to 4095)
    if ((pBlt->srcBox.Left    == 0) &&
        (pBlt->srcBox.Top     == 0) &&
        (pBlt->srcBox.Right   == pSrcTexture->wWidth) &&
        (pBlt->srcBox.Bottom  == pSrcTexture->wHeight) &&
        (pBlt->srcBox.Front   == 0) &&
        (pBlt->srcBox.Back    == pSrcTexture->wDepth) &&
        (pBlt->dwDestX        == 0) &&
        (pBlt->dwDestY        == 0) &&
        (pBlt->dwDestZ        == 0) &&
        (pSrcTexture->wWidth  == pDestTexture->wWidth) &&
        (pSrcTexture->wHeight == pDestTexture->wHeight) &&
        (pSrcTexture->wDepth  == pDestTexture->wDepth) &&
        ((pBlt->srcBox.Bottom * pBlt->srcBox.Back) < 4095))
    {
        // Build source rectangle

        rSrc.left   = 0;
        rSrc.top    = 0;
        rSrc.right  = pBlt->srcBox.Right;
        rSrc.bottom = pBlt->srcBox.Bottom * pBlt->srcBox.Back;

        // Destination rectangle is same as source.

        rDest = rSrc;

        // Switch to the DirectDraw context
        DDRAW_OPERATION(pContext, pThisDisplay);

        // Do the Blt!
        __OP_BasicVolumeBlt(pContext,
                            pThisDisplay,
                            pSrcTexture,
                            pDestTexture,
                            pBlt->dwDDDestSurface,
                            &rSrc,
                            &rDest);                                
      
        // Switch back to the Direct3D context
        D3D_OPERATION(pContext, pThisDisplay);

        return;
    }

    // Build source rectangle.
    rSrc.left   = pBlt->srcBox.Left;
    rSrc.top    = pBlt->srcBox.Top;
    rSrc.right  = pBlt->srcBox.Right;
    rSrc.bottom = pBlt->srcBox.Bottom;

    // Build destination rectangle.
    rDest.left   = pBlt->dwDestX;
    rDest.top    = pBlt->dwDestY;
    rDest.right  = pBlt->dwDestX + (rSrc.right - rSrc.left);
    rDest.bottom = pBlt->dwDestY + (rSrc.bottom - rSrc.top);

    // Adjust rectangle if blt from non-1st slice.
    if (pBlt->srcBox.Front)
    {
        ULONG ulOffset = pSrcTexture->wDepth * pBlt->srcBox.Front;
        rSrc.top    += ulOffset;
        rSrc.bottom += ulOffset;
    }

    // Adjust rectangle if blt to non-1st slice.
    if (pBlt->dwDestZ)
    {
        ULONG ulOffset = pDestTexture->wDepth * pBlt->dwDestZ;
        rDest.top    += ulOffset;
        rDest.bottom += ulOffset;
    }

    dwSrcCurrDepth  = pBlt->srcBox.Front;
    dwDestCurrDepth = pBlt->dwDestZ;

    dwEndDepth   = min(pBlt->dwDestZ + (pBlt->srcBox.Back - pBlt->srcBox.Front),
                       pDestTexture->wDepth);
    dwEndDepth   = min(dwEndDepth, pSrcTexture->wDepth);

    // Switch to the DirectDraw context
    DDRAW_OPERATION(pContext, pThisDisplay);

    while(dwDestCurrDepth < dwEndDepth)
    {
        // Do the Blt!
        __OP_BasicVolumeBlt(pContext,
                            pThisDisplay,
                            pSrcTexture,
                            pDestTexture,
                            pBlt->dwDDDestSurface,
                            &rSrc,
                            &rDest);  
                            
        // Move the source and destination rect to next slice.
        rSrc.top     += pSrcTexture->wHeight;
        rSrc.bottom  += pSrcTexture->wHeight;
        rDest.top    += pDestTexture->wHeight;
        rDest.bottom += pDestTexture->wHeight;

        // Move on to next slice.
        dwSrcCurrDepth++;
        dwDestCurrDepth++;
    }

    // Switch back to the Direct3D context
    D3D_OPERATION(pContext, pThisDisplay);

} // _D3D_OP_VolumeBlt

#endif // DX8_3DTEXTURES

#if DX8_DDI  

//-----------------------------------------------------------------------------
//
// _D3D_OP_BufferBlt
//
// This function processes the D3DDP2OP_BUFFERBLT DP2 command token.
//
//-----------------------------------------------------------------------------   
VOID _D3D_OP_BufferBlt(P3_D3DCONTEXT* pContext, 
                       P3_THUNKEDDATA*pThisDisplay,
                       D3DHAL_DP2BUFFERBLT* pBlt)
{

#if DX7_VERTEXBUFFERS
    // This command token is only sent to drivers 
    // supporting videomemory vertexbuffers. That is
    // why we won't see it come down to this driver.
#endif DX7_VERTEXBUFFERS

} // _D3D_OP_BufferBlt

#endif // DX8_DDI 

#if DX8_VERTEXSHADERS
//-----------------------------------------------------------------------------
//
// _D3D_OP_VertexShader_Create
//
// This function processes the D3DDP2OP_CREATEVERTEXSHADER DP2 command token.
//
//-----------------------------------------------------------------------------
HRESULT 
_D3D_OP_VertexShader_Create(
    P3_D3DCONTEXT* pContext, 
    DWORD dwVtxShaderHandle,
    DWORD dwDeclSize,
    DWORD dwCodeSize,
    BYTE *pShader)
{
    // Here we would use the data passed by the vertex shader
    // creation block in order to instantiate or compile the
    // given vertex shader. Since this hardware can't support
    // vertex shaders at this time, we just skip the data.
    
    return DD_OK;
    
} // _D3D_OP_VertexShader_Create

//-----------------------------------------------------------------------------
//
// _D3D_OP_VertexShader_Delete
//
// This function processes the D3DDP2OP_DELETEVERTEXSHADER DP2 command token.
//
//-----------------------------------------------------------------------------
VOID 
_D3D_OP_VertexShader_Delete(
    P3_D3DCONTEXT* pContext, 
    DWORD dwVtxShaderHandle)
{
    // Here we would use the data passed by the vertex shader
    // delete block in order to destroy the given vertex shader.
    // Since this hardware can't support vertex shaders at 
    // this time, we just skip the data.
    
} // _D3D_OP_VertexShader_Delete

//-----------------------------------------------------------------------------
//
// _D3D_OP_VertexShader_Set
//
// This function processes the D3DDP2OP_SETVERTEXSHADER DP2 command token.
//
//-----------------------------------------------------------------------------
VOID 
_D3D_OP_VertexShader_Set(
    P3_D3DCONTEXT* pContext, 
    DWORD dwVtxShaderHandle)
{
    // Here we would use the data passed by the vertex shader
    // set block in order to setup the given vertex shader.
    // Since this hardware can't support vertex shaders at 
    // this time, we usually just skip the data. However under
    // the circumstances described below, we might be passed a
    // FVF vertex format

    DISPDBG((DBGLVL,"Setting up shader # 0x%x",dwVtxShaderHandle));

#if DX7_D3DSTATEBLOCKS
    if ( pContext->bStateRecMode ) 
    {
        _D3D_SB_Record_VertexShader_Set(pContext, dwVtxShaderHandle);
        return;
    }
#endif // DX7_D3DSTATEBLOCKS

    // Zero is a special handle that tells the driver to
    // invalidate the currently set shader.
    if( dwVtxShaderHandle == 0 )
    {
        DISPDBG((WRNLVL,"Invalidating the currently set shader"));
        return ;
    }    

    if( RDVSD_ISLEGACY(dwVtxShaderHandle) )
    {
        // Make it parse the FVF 
        pContext->dwVertexType = dwVtxShaderHandle;  
    }  
    else
    {
        DISPDBG((ERRLVL,"_D3D_OP_VertexShader_Set: Illegal shader handle "
                        "(This driver cant do vertex processing)"));
    }
    
} // _D3D_OP_VertexShader_Set

//-----------------------------------------------------------------------------
//
// _D3D_OP_VertexShader_SetConst
//
// This function processes the D3DDP2OP_SETVERTEXSHADERCONST DP2 command token.
//
//-----------------------------------------------------------------------------
VOID 
_D3D_OP_VertexShader_SetConst(
    P3_D3DCONTEXT* pContext, 
    DWORD dwRegister, 
    DWORD dwConst, 
    DWORD *pdwValues)
{
    // Here we would use the data passed by the vertex shader
    // constant block in order to set up the constant entry.
    // Since this hardware can't support vertex shaders at 
    // this time, we just skip the data.
    
} // _D3D_OP_VertexShader_SetConst

#endif // DX8_VERTEXSHADERS

#if DX8_PIXELSHADERS

//-----------------------------------------------------------------------------
//
// _D3D_OP_PixelShader_Create
//
// This function processes the D3DDP2OP_CREATEPIXELSHADER DP2 command token.
//
//-----------------------------------------------------------------------------
HRESULT 
_D3D_OP_PixelShader_Create(
    P3_D3DCONTEXT* pContext, 
    DWORD dwPxlShaderHandle,
    DWORD dwCodeSize,
    BYTE *pShader)
{
    // Here we would use the data passed by the pixel shader
    // creation block in order to instantiate or compile the
    // given pixel shader. 

    // Since this hardware can't support pixel shaders at this 
    // time, we fail the call in case we're called to create a
    // 255.255 version shader!

    return D3DERR_DRIVERINVALIDCALL;
    
} // _D3D_OP_PixelShader_Create

//-----------------------------------------------------------------------------
//
// _D3D_OP_PixelShader_Delete
//
// This function processes the D3DDP2OP_DELETEPIXELSHADER DP2 command token.
//
//-----------------------------------------------------------------------------
VOID 
_D3D_OP_PixelShader_Delete(
    P3_D3DCONTEXT* pContext, 
    DWORD dwPxlShaderHandle)
{
    // Here we would use the data passed by the pixel shader
    // delete block in order to destroy the given pixel shader.
    // Since this hardware can't support pixel shaders at 
    // this time, we just skip the data.
    
} // _D3D_OP_PixelShader_Delete

//-----------------------------------------------------------------------------
//
// _D3D_OP_PixelShader_Set
//
// This function processes the D3DDP2OP_SETPIXELSHADER DP2 command token.
//
//-----------------------------------------------------------------------------
VOID 
_D3D_OP_PixelShader_Set(
    P3_D3DCONTEXT* pContext, 
    DWORD dwPxlShaderHandle)
{
    // Here we would use the data passed by the pixel shader
    // set block in order to setup the given pixel shader.
    // Since this hardware can't support pixel shaders at 
    // this time, we just skip the data.
    
} // _D3D_OP_PixelShader_Set

//-----------------------------------------------------------------------------
//
// _D3D_OP_PixelShader_SetConst
//
// This function processes the D3DDP2OP_SETPIXELSHADERCONST DP2 command token.
//
//-----------------------------------------------------------------------------
VOID 
_D3D_OP_PixelShader_SetConst(
    P3_D3DCONTEXT* pContext, 
    DWORD dwRegister, 
    DWORD dwCount, 
    DWORD *pdwValues)
{
    // Here we would use the data passed by the pixel shader
    // set block in order to setup the given pixel shader constants.
    // Since this hardware can't support pixel shaders at 
    // this time, we just skip the data.
    
} // _D3D_OP_PixelShader_SetConst
#endif // DX8_PIXELSHADERS

#if DX8_MULTSTREAMS

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_SetSrc
//
// This function processes the D3DDP2OP_SETSTREAMSOURCE DP2 command token.
//
//-----------------------------------------------------------------------------
VOID 
_D3D_OP_MStream_SetSrc(
    P3_D3DCONTEXT* pContext, 
    DWORD dwStream,
    DWORD dwVBHandle,
#if DX9_SETSTREAMSOURCE2
    DWORD dwOffset,
#endif // DX9_SETSTREAMSOURCE2
    DWORD dwStride)
{
    P3_SURF_INTERNAL *pSrcStream;
    
    DBG_ENTRY(_D3D_OP_MStream_SetSrc);

#if DX7_D3DSTATEBLOCKS
    if ( pContext->bStateRecMode ) 
    {
        _D3D_SB_Record_MStream_SetSrc(pContext, 
                                      dwStream, 
                                      dwVBHandle, 
#if DX9_SETSTREAMSOURCE2
                                      dwOffset, 
#endif // DX9_SETSTREAMSOURCE2
                                      dwStride);
        return;
    }
#endif // DX7_D3DSTATEBLOCKS

    if (dwVBHandle != 0)
    {
        if (dwStream == 0)
        {
            // Get the surface structure pointers for stream #0
            pSrcStream = GetSurfaceFromHandle(pContext, dwVBHandle);

            if (pSrcStream)
            {
                DISPDBG((DBGLVL,"Address of VB = 0x%x "
                                "dwVBHandle = %d , dwStride = %d",
                                pSrcStream->fpVidMem,dwVBHandle, dwStride));
                pContext->lpVertices = (LPDWORD)((LPBYTE)pSrcStream->fpVidMem
#if DX9_SETSTREAMSOURCE2
                                                 + dwOffset);
#else
                                                );
#endif // DX9_SETSTREAMSOURCE2
                pContext->dwVerticesStride = dwStride;

                if (dwStride > 0)
                {
                    // DX8 has mixed types of vertices in one VB, size in bytes
                    // of the vertex buffer must be preserved
#if DX9_SETSTREAMSOURCE2
                    pContext->dwVBSizeInBytes = pSrcStream->lPitch - dwOffset;
#else
                    pContext->dwVBSizeInBytes = pSrcStream->lPitch;
#endif // DX9_SETSTREAMSOURCE2

                    // for VBs the wHeight should always be == 1.
                    // dwNumVertices stores the # of vertices in the VB
                    // On Win2K, both wWidth and lPitch are the buffer size
                    // On Win9x, only lPitch is the buffer size, wWidth is 0
                    // The same fact is also true for the index buffer
                    pContext->dwNumVertices = pContext->dwVBSizeInBytes / dwStride;

                    DISPDBG((DBGLVL,"dwVBHandle pContext->dwNumVertices = "
                                "pSrcStream->lPitch / dwStride = %d %d %d %d",
                                dwVBHandle,
                                pContext->dwNumVertices, 
                                pContext->dwVBSizeInBytes,
                                dwStride));  
                    
#if DX7_D3DSTATEBLOCKS
                    pContext->dwVBHandle = dwVBHandle;
#endif // DX7_D3DSTATEBLOCKS
                }
                else
                {
                    pContext->dwVBSizeInBytes = 0;
                    pContext->dwNumVertices = 0;
                    DISPDBG((ERRLVL,"INVALID Stride is 0. VB Size undefined"));
                }
            }
            else
            {
                DISPDBG((ERRLVL,"ERROR Address of VB is NULL, "
                                "dwStream = %d dwVBHandle = %d , dwStride = %d",
                                dwStream, dwVBHandle, dwStride));
            }
        }
        else
        {
            DISPDBG((WRNLVL,"We don't handle other streams than #0"));
        }
    }
    else
    {
        // We are unsetting the stream
        pContext->lpVertices = NULL;

        DISPDBG((WRNLVL,"Unsetting a stream: "
                        "dwStream = %d dwVBHandle = %d , dwStride = %d",
                        dwStream, dwVBHandle, dwStride));
    }

    DBG_EXIT(_D3D_OP_MStream_SetSrc, 0);
} // _D3D_OP_MStream_SetSrc

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_SetSrcUM
//
// This function processes the D3DDP2OP_SETSTREAMSOURCEUM DP2 command token.
//
//-----------------------------------------------------------------------------
VOID
_D3D_OP_MStream_SetSrcUM(
    P3_D3DCONTEXT* pContext, 
    DWORD dwStream,
    DWORD dwStride,
    LPBYTE pUMVtx,
    DWORD  dwVBSize)
{
    DBG_ENTRY(_D3D_OP_MStream_SetSrcUM);

    if (dwStream == 0)
    {
        // Set the stream # 0 information
        DISPDBG((DBGLVL,"_D3D_OP_MStream_SetSrcUM: "
                        "Setting VB@ 0x%x dwstride=%d", pUMVtx, dwStride));
        pContext->lpVertices = (LPDWORD)pUMVtx;
        pContext->dwVerticesStride = dwStride;
        pContext->dwVBSizeInBytes = dwVBSize  * dwStride;
        pContext->dwNumVertices = dwVBSize ;     // comes from the DP2 data 
                                                 // structure
    
    }
    else
    {
        DISPDBG((WRNLVL,"_D3D_OP_MStream_SetSrcUM: "
                        "We don't handle other streams than #0"));
    }
    
    DBG_EXIT(_D3D_OP_MStream_SetSrcUM, 0);
} // _D3D_OP_MStream_SetSrcUM

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_SetIndices
//
// This function processes the D3DDP2OP_SETINDICES DP2 command token.
//
//-----------------------------------------------------------------------------             
VOID
_D3D_OP_MStream_SetIndices(
    P3_D3DCONTEXT* pContext, 
    DWORD dwVBHandle,
    DWORD dwStride)
{
    P3_SURF_INTERNAL *pIndxStream;
    
    DBG_ENTRY(_D3D_OP_MStream_SetIndices);

#if DX7_D3DSTATEBLOCKS    
    if ( pContext->bStateRecMode )
    {
        _D3D_SB_Record_MStream_SetIndices(pContext, dwVBHandle, dwStride);
        return;
    }
#endif // DX7_D3DSTATEBLOCKS    

    // NULL dwVBHandle just means that the Index should be unset
    if (dwVBHandle != 0)
    {
        // Get the indices surface structure pointer
        pIndxStream = GetSurfaceFromHandle(pContext, dwVBHandle);

        if (pIndxStream)
        {
            DISPDBG((DBGLVL,"Address of VB = 0x%x", pIndxStream->fpVidMem));

            pContext->lpIndices = (LPDWORD)pIndxStream->fpVidMem;
            pContext->dwIndicesStride = dwStride; // 2 or 4 for 16/32bit indices
#if DX7_D3DSTATEBLOCKS
            pContext->dwIndexHandle = dwVBHandle; // Index buffer handle
#endif
        }
        else
        {
            DISPDBG((ERRLVL,"ERROR Address of Index Surface is NULL, "
                            "dwVBHandle = %d , dwStride = %d",
                             dwVBHandle, dwStride));
        }
    }
    else
    {
        // We are unsetting the stream
        pContext->lpIndices = NULL;

        DISPDBG((WRNLVL,"Unsetting an index stream: "
                        "dwVBHandle = %d , dwStride = %d",
                         dwVBHandle, dwStride));
    }
    
    DBG_EXIT(_D3D_OP_MStream_SetIndices, 0);
} // _D3D_OP_MStream_SetIndices

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_DrawPrim
//
// This function processes the D3DDP2OP_DRAWPRIMITIVE DP2 command token.
//
//-----------------------------------------------------------------------------
VOID
_D3D_OP_MStream_DrawPrim(
    P3_D3DCONTEXT* pContext, 
    D3DPRIMITIVETYPE primType,
    DWORD VStart,
    DWORD PrimitiveCount)
{
    DBG_ENTRY(_D3D_OP_MStream_DrawPrim);

    DISPDBG((DBGLVL,"_D3D_OP_MStream_DrawPrim "
               "primType=0x%x VStart=%d PrimitiveCount=%d", 
               primType, VStart, PrimitiveCount));

   _D3D_OP_MStream_DrawPrim2(pContext, 
                             primType,
                             VStart * pContext->FVFData.dwStride,
                             PrimitiveCount);
    
    DBG_EXIT(_D3D_OP_MStream_DrawPrim, 0);
} // _D3D_OP_MStream_DrawPrim

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_DrawIndxP
//
// This function processes the D3DDP2OP_DRAWINDEXEDPRIMITIVE DP2 command token.
//
//-----------------------------------------------------------------------------
VOID
_D3D_OP_MStream_DrawIndxP(
    P3_D3DCONTEXT* pContext, 
    D3DPRIMITIVETYPE primType,
    DWORD BaseVertexIndex,
    DWORD MinIndex,
    DWORD NumVertices,
    DWORD StartIndex,
    DWORD PrimitiveCount)
{
    DBG_ENTRY(_D3D_OP_MStream_DrawIndxP);

    DISPDBG((DBGLVL,"_D3D_OP_MStream_DrawIndxP " 
               "primType=0x%x BaseVertexIndex=%d MinIndex=%d"
               "NumVertices =%d StartIndex=%d PrimitiveCount=%d", 
               primType, BaseVertexIndex, MinIndex,
               NumVertices, StartIndex, PrimitiveCount));

    _D3D_OP_MStream_DrawIndxP2(pContext, 
                               primType,
                               BaseVertexIndex * pContext->FVFData.dwStride,
                               MinIndex,
                               NumVertices,
                               StartIndex * pContext->dwIndicesStride,
                               PrimitiveCount);
                               
    DBG_EXIT(_D3D_OP_MStream_DrawIndxP, 0);
} // _D3D_OP_MStream_DrawIndxP

//-----------------------------------------------------------------------------
//
// Validate the context settings to use the current streams
//
//-----------------------------------------------------------------------------
BOOL
__OP_ValidateStreams(
    P3_D3DCONTEXT* pContext,
    BOOL bCheckIndexStream)
{
    if ((pContext->dwVerticesStride == 0) || 
        (pContext->FVFData.dwStride == 0))
    {
        DISPDBG((ERRLVL,"The zero'th stream is doesn't have a valid VB set"));
        return FALSE;        
    }

    if (pContext->dwVerticesStride < pContext->FVFData.dwStride)
    {
        DISPDBG((ERRLVL,"The stride set for the vertex stream is "
                        "less than the FVF vertex size"));
        return FALSE;
    }


    if ((bCheckIndexStream) && (NULL == pContext->lpIndices))
    {
        DISPDBG((ERRLVL,"Pointer to index buffer is null"));
        return FALSE;    
    }

    if (NULL == pContext->lpVertices)
    {
        DISPDBG((ERRLVL,"Pointer to vertex buffer is null"));
        return FALSE;       
    }

    return TRUE;
} // __OP_ValidateStreams

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_DrawPrim2
//
// This function processes the D3DDP2OP_DRAWPRIMITIVE2 DP2 command token.
//
//-----------------------------------------------------------------------------

VOID
_D3D_OP_MStream_DrawPrim2(
    P3_D3DCONTEXT* pContext, 
    D3DPRIMITIVETYPE primType,
    DWORD FirstVertexOffset,
    DWORD PrimitiveCount)
{
    BOOL bError;
    WORD wVStart;
    DWORD dwFillMode = pContext->RenderStates[D3DRENDERSTATE_FILLMODE];
    LPBYTE lpVertices;
    DWORD dwNumVertices;

    DBG_ENTRY(_D3D_OP_MStream_DrawPrim2);

    DISPDBG((DBGLVL ,"_D3D_OP_MStream_DrawPrim2 "
               "primType=0x%x FirstVertexOffset=%d PrimitiveCount=%d", 
               primType, FirstVertexOffset, PrimitiveCount));

    if (!__OP_ValidateStreams(pContext, FALSE))
    {
        return;
    }

    // Watchout: Sometimes (particularly when CLIPPEDTRIFAN are drawn), 
    // FirstVertexOffset might not be divided evenly by the dwStride
    lpVertices = ((LPBYTE)pContext->lpVertices) + FirstVertexOffset;
    dwNumVertices = pContext->dwVBSizeInBytes - FirstVertexOffset;
    dwNumVertices /= pContext->dwVerticesStride;
    wVStart = 0;
    
    switch(primType)
    {
        case D3DPT_POINTLIST:
            {
                D3DHAL_DP2POINTS dp2Points;
                dp2Points.wVStart = wVStart;
                
#if DX8_POINTSPRITES
                if(IS_POINTSPRITE_ACTIVE(pContext))
                {
                    _D3D_R3_DP2_PointSprites_DWCount(pContext,
                                                     PrimitiveCount,
                                                     (LPBYTE)&dp2Points,
                                                     (LPD3DTLVERTEX)lpVertices,
                                                     dwNumVertices,
                                                     &bError);
                }
                else
#endif // DX8_POINTSPRITES
                {
                  
                    _D3D_R3_DP2_Points_DWCount(pContext,
                                               PrimitiveCount,
                                               (LPBYTE)&dp2Points,
                                               (LPD3DTLVERTEX)lpVertices,
                                               dwNumVertices,
                                               &bError);

                }  
            
            }
            break;
                
        case D3DPT_LINELIST:

            _D3D_R3_DP2_LineList(pContext,
                                 PrimitiveCount,
                                 (LPBYTE)&wVStart,
                                 (LPD3DTLVERTEX)lpVertices,
                                 dwNumVertices,
                                 &bError);
            break;        
            
        case D3DPT_LINESTRIP:

            _D3D_R3_DP2_LineStrip(pContext,
                                 PrimitiveCount,
                                 (LPBYTE)&wVStart,
                                 (LPD3DTLVERTEX)lpVertices,
                                 dwNumVertices,
                                 &bError);
            break;   
            
        case D3DPT_TRIANGLELIST:        

            _D3D_R3_DP2_TriangleList(pContext,
                                     PrimitiveCount,
                                     (LPBYTE)&wVStart,
                                     (LPD3DTLVERTEX)lpVertices,
                                     dwNumVertices,
                                     &bError);
            break;  
            
        case D3DPT_TRIANGLESTRIP:
        
            _D3D_R3_DP2_TriangleStrip(pContext,
                                     PrimitiveCount,
                                     (LPBYTE)&wVStart,
                                     (LPD3DTLVERTEX)lpVertices,
                                     dwNumVertices,
                                     &bError);        
            break; 
            
        case D3DPT_TRIANGLEFAN:
        
            _D3D_R3_DP2_TriangleFan(pContext,
                                    PrimitiveCount,
                                    (LPBYTE)&wVStart,
                                    (LPD3DTLVERTEX)lpVertices,
                                    dwNumVertices,
                                    &bError);
            break;         
    }
    
    DBG_EXIT(_D3D_OP_MStream_DrawPrim2, 0);
} // _D3D_OP_MStream_DrawPrim2

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_DrawIndxP2
//
// This function processes the D3DDP2OP_DRAWINDEXEDPRIMITIVE2 DP2 command token.
//
//-----------------------------------------------------------------------------
VOID
_D3D_OP_MStream_DrawIndxP2(
    P3_D3DCONTEXT* pContext, 
    D3DPRIMITIVETYPE primType,
    INT   BaseVertexOffset,
    DWORD MinIndex,
    DWORD NumVertices,
    DWORD StartIndexOffset,
    DWORD PrimitiveCount)
{
    INT      BaseIndexOffset;
    LPDWORD  lpVertices;
    LPBYTE   lpIndices;
    BOOL bError;

    R3_DP2_PRIM_TYPE_MS *pRenderFunc; 
    
    DBG_ENTRY(_D3D_OP_MStream_DrawIndxP2);

    DISPDBG((DBGLVL,"_D3D_OP_MStream_DrawIndxP2 "
               "primType=0x%x BaseVertexOffset=%d MinIndex=%d "
               "NumVertices=%d StartIndexOffset=%d PrimitiveCount=%d", 
               primType, BaseVertexOffset, MinIndex,
               NumVertices, StartIndexOffset, PrimitiveCount));

    if (!__OP_ValidateStreams(pContext, TRUE))
    {
        return;
    }

    // The MinIndex and NumVertices parameters specify the range of vertex i
    // ndices used for each DrawIndexedPrimitive call. These are used to 
    // optimize vertex processing of indexed primitives by processing a 
    // sequential range of vertices prior to indexing into these vertices

    // **********                IMPORTANT NOTE               **********
    //
    // BaseVertexOffset is a signed quantity (INT) unlike the other parameters
    // to this call which are DWORDS. This may appear strange. Why would
    // the offset into the vertex buffer be negative? Clearly you cannot access
    // vertex data before the start of the vertex buffer, and indeed, you never
    // do. When you have a negative BaseVertexOffset you will also receive
    // indices which are large enough that when applied to the start pointer
    // (obtained from adding a negative BaseVertexOffset to the vertex data
    // pointer) which fall within the correct range of the vertices in the
    // actual vertex buffer, i.e., the indices "undo" any negative vertex offset
    // and vertex accesses will end up being in the legal range for that vertex
    // buffer.
    //
    // Hence, you must write your driver code with this in mind. For example,
    // you can't assume that given an index i and with a current vertex buffer
    // of size v:
    //
    // ((StartIndexOffset + i) >= 0) && ((StartIndexOffset + i) < v)
    //
    // Your code needs to take into account that your indices are not offsets
    // from the start of the vertex buffer but rather from the start of the
    // vertex buffer plus BaseVertexOffset and that furthermore BaseVertexOffset
    // may be negative.
    //
    // The reason BaseVertexOffset can be negative is that it provides a
    // significant advantage to the runtime in certain vertex processing scenarios.

    lpVertices = (LPDWORD)((LPBYTE)pContext->lpVertices + BaseVertexOffset);

    lpIndices =  (LPBYTE)pContext->lpIndices + StartIndexOffset;  


    // Select the appropriate rendering function
    pRenderFunc = NULL;
    
    if (pContext->dwIndicesStride == 2)
    {   
        // Handle 16 bit indices
                                      
        switch(primType)
        {                    
            case D3DPT_LINELIST:
                pRenderFunc = _D3D_R3_DP2_IndexedLineList_MS_16IND;
                break;        
                
            case D3DPT_LINESTRIP:
                pRenderFunc = _D3D_R3_DP2_IndexedLineStrip_MS_16IND;            
                break;   
                
            case D3DPT_TRIANGLELIST:
                pRenderFunc = _D3D_R3_DP2_IndexedTriangleList_MS_16IND;  
                break;
                
            case D3DPT_TRIANGLESTRIP:
                pRenderFunc = _D3D_R3_DP2_IndexedTriangleStrip_MS_16IND;  
                break;            
                
            case D3DPT_TRIANGLEFAN:
                pRenderFunc = _D3D_R3_DP2_IndexedTriangleFan_MS_16IND;  
                break;                        
        }
    }
    else
    {
        // Handle 32 bit indices

        switch(primType)
        {                    
            case D3DPT_LINELIST:
                pRenderFunc = _D3D_R3_DP2_IndexedLineList_MS_32IND;
                break;        
                
            case D3DPT_LINESTRIP:
                pRenderFunc = _D3D_R3_DP2_IndexedLineStrip_MS_32IND;            
                break;   
                
            case D3DPT_TRIANGLELIST:
                pRenderFunc = _D3D_R3_DP2_IndexedTriangleList_MS_32IND;  
                break;
                
            case D3DPT_TRIANGLESTRIP:
                pRenderFunc = _D3D_R3_DP2_IndexedTriangleStrip_MS_32IND;  
                break;            
                
            case D3DPT_TRIANGLEFAN:
                pRenderFunc = _D3D_R3_DP2_IndexedTriangleFan_MS_32IND;  
                break;                        
        }        
    }

    // Call our rendering function
    if (pRenderFunc)
    {
        // As mentioned above, the actual range of indices seen by the driver
        // doesn't necessarily lie within the range zero to one less than
        // the number of vertices in the vertex buffer due to BaseVertexOffset.
        // If BaseVertexOffset is positive the range of valid indices is
        // smaller than the size of the vertex buffer (the vertices that
        // lie in the vertex buffer before the BaseVertexOffset are not
        // considered). Furthermore, if BaseVertexOffset a valid index can
        // actually by greater than the number of vertices in the vertex
        // buffer.
        //
        // To assist with the validation performed by the rendering functions
        // we here compute a minimum and maximum index which take into
        // account the value of BaseVertexOffset. Thus a test for a valid
        // index becomes:
        //
        // ((BaseIndexOffset + StartIndexOffset + Index) >= 0) &&
        // ((BaseIndexOffset + StartIndexOffset + Index) <  VertexCount)

        BaseIndexOffset = (BaseVertexOffset / (int)pContext->dwVerticesStride);

        DISPDBG((DBGLVL,"_D3D_OP_MStream_DrawIndxP2 BaseIndexOffset = %d",
            BaseIndexOffset));

        (*pRenderFunc)(pContext,
                       PrimitiveCount,
                       (LPBYTE)lpIndices,
                       (LPD3DTLVERTEX)lpVertices,
                       BaseIndexOffset,
                       pContext->dwNumVertices,
                       &bError);     
    }
    
    DBG_EXIT(_D3D_OP_MStream_DrawIndxP2, 0);
} // _D3D_OP_MStream_DrawIndxP2



//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_ClipTriFan
//
// This function processes the D3DDP2OP_CLIPPEDTRIANGLEFAN DP2 command token.
//
//-----------------------------------------------------------------------------
VOID
_D3D_OP_MStream_ClipTriFan(    
    P3_D3DCONTEXT* pContext, 
    DWORD FirstVertexOffset,
    DWORD dwEdgeFlags,
    DWORD PrimitiveCount)
{   
    BOOL bError;
    
    DBG_ENTRY(_D3D_OP_MStream_ClipTriFan);

    DISPDBG((DBGLVL,"_D3D_OP_MStream_ClipTriFan "
               "FirstVertexOffset=%d dwEdgeFlags=0x%x PrimitiveCount=%d", 
               FirstVertexOffset, dwEdgeFlags, PrimitiveCount));

    if (pContext->RenderStates[D3DRENDERSTATE_FILLMODE] == D3DFILL_WIREFRAME)
    {
        D3DHAL_DP2TRIANGLEFAN_IMM dp2TriFanWire;

        if (!__OP_ValidateStreams(pContext, FALSE))
        {
            DBG_EXIT(_D3D_OP_MStream_ClipTriFan, 0);
            return;
        }

        dp2TriFanWire.dwEdgeFlags = dwEdgeFlags;

        _D3D_R3_DP2_ClipTriFanWire(pContext,
                                   (WORD)PrimitiveCount,
                                   (LPBYTE)&dp2TriFanWire,
                                   (LPD3DTLVERTEX)(((LPBYTE)pContext->lpVertices) + FirstVertexOffset),
                                   pContext->dwNumVertices,
                                   &bError);
        
        
    }
    else
    {
       _D3D_OP_MStream_DrawPrim2(pContext, 
                                 D3DPT_TRIANGLEFAN,
                                 FirstVertexOffset,
                                 PrimitiveCount);
    }
    
    DBG_EXIT(_D3D_OP_MStream_ClipTriFan, 0);
} // _D3D_OP_MStream_ClipTriFan

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_DrawRectSurface
//
// This function processes the D3DDP2OP_DRAWRECTSURFACE DP2 command token.
//
//-----------------------------------------------------------------------------
VOID _D3D_OP_MStream_DrawRectSurface(P3_D3DCONTEXT* pContext, 
                                     DWORD Handle,
                                     DWORD Flags,
                                     PVOID lpPrim)
{
    // High order surfaces are only supported for hw/drivers with
    // TnL support and 1.0 vertex shader support
    
} // _D3D_OP_MStream_DrawRectSurface

//-----------------------------------------------------------------------------
//
// _D3D_OP_MStream_DrawTriSurface
//
// This function processes the D3DDP2OP_DRAWTRISURFACE DP2 command token.
//
//-----------------------------------------------------------------------------                                     
VOID _D3D_OP_MStream_DrawTriSurface(P3_D3DCONTEXT* pContext, 
                                    DWORD Handle,
                                    DWORD Flags,
                                    PVOID lpPrim)
{
    // High order surfaces are only supported for hw/drivers with
    // TnL support and 1.0 vertex shader support

} // _D3D_OP_MStream_DrawTriSurface

#endif // DX8_MULTSTREAMS

//-----------------------------------------------------------------------------
//
// _D3D_OP_Viewport
//
// This function processes the D3DDP2OP_VIEWPORTINFO DP2 command token.
//
//-----------------------------------------------------------------------------                                     

VOID _D3D_OP_Viewport(P3_D3DCONTEXT* pContext,
                      D3DHAL_DP2VIEWPORTINFO* lpvp)
{
#if DX7_D3DSTATEBLOCKS
    if ( pContext->bStateRecMode )
    {
         _D3D_SB_Record_Viewport(pContext, lpvp);
    }
    else
#endif // DX7_D3DSTATEBLOCKS    
    {
        pContext->ViewportInfo = *lpvp;
        DIRTY_VIEWPORT(pContext);
    }
} // _D3D_OP_Viewport

//-----------------------------------------------------------------------------
//
// _D3D_OP_ZRange
//
// This function processes the D3DDP2OP_ZRANGE DP2 command token.
//
//-----------------------------------------------------------------------------

VOID _D3D_OP_ZRange(P3_D3DCONTEXT* pContext,
                    D3DHAL_DP2ZRANGE* lpzr)
{
#if DX7_D3DSTATEBLOCKS
    if ( pContext->bStateRecMode )
    {
        _D3D_SB_Record_ZRange(pContext, lpzr);
    }
    else
#endif // DX7_D3DSTATEBLOCKS    
    {
        pContext->ZRange = *lpzr;
        DIRTY_VIEWPORT(pContext);
    }
} // _D3D_OP_ZRange

//-----------------------------------------------------------------------------
//
// _D3D_OP_UpdatePalette
//
// This function processes the D3DDP2OP_UPDATEPALETTE DP2 command token.
//
//      Note : This function is need to skip D3DDP2OP_UPDATEPALETTE sent down 
//             by some DX6 apps, even if when PALETTE TEXTURE is not supported
//             Also notice that for legacy DX apps, the palette doesn't get
//             properly restored in another app transitions into full screen
//             mode and back. This is because the (legacy) runtimes don't
//             sent proper notification (through UpdatePalette/SetPalette) of
//             this event
//
//-----------------------------------------------------------------------------

HRESULT _D3D_OP_UpdatePalette(P3_D3DCONTEXT* pContext,
                              D3DHAL_DP2UPDATEPALETTE* pUpdatePalette,
                              DWORD* pdwPalEntries)
{
#if DX7_PALETTETEXTURE
    D3DHAL_DP2UPDATEPALETTE* pPalette;
    P3_SURF_INTERNAL* pTexture;

    // Find internal palette pointer from handle
    pPalette = GetPaletteFromHandle(pContext,
                                    pUpdatePalette->dwPaletteHandle);

    // Palette doesn't exist
    if (! pPalette) 
    {
        DISPDBG((WRNLVL, "_D3D_OP_UpdatePalette : Can't find palette"));
        return DDERR_INVALIDPARAMS;
    }

    // Check the range of palette entries
    if (pUpdatePalette->wStartIndex > LUT_ENTRIES)
    {
        DISPDBG((WRNLVL, 
                 "_D3D_OP_UpdatePalette : wStartIndex (%d) is bigger than 256", 
                 pUpdatePalette->wStartIndex));
        return DDERR_INVALIDPARAMS;
    }
    if ((pUpdatePalette->wStartIndex + pUpdatePalette->wNumEntries) 
                                                            > LUT_ENTRIES) 
    {
        DISPDBG((WRNLVL, "_D3D_OP_UpdatePalette : too many entries"));
        return DDERR_INVALIDPARAMS;
    }

    // Each palette is ARGB 8:8:8:8
    memcpy(((LPBYTE)(pPalette + 1)) + pUpdatePalette->wStartIndex*sizeof(DWORD),
           pdwPalEntries,
           pUpdatePalette->wNumEntries*sizeof(DWORD));

    // Check if the palette is in use
    // Palette Texture can not be used alone in the 2nd stage, so only the
    // 1st stage must be checked.
    if (pContext->TextureStageState[0].m_dwVal[D3DTSS_TEXTUREMAP])
    {
        pTexture = GetSurfaceFromHandle(pContext,
                                        pContext->TextureStageState[0].m_dwVal[D3DTSS_TEXTUREMAP]);
        if (pTexture)
        {
            if ((pTexture->pFormatSurface->DeviceFormat == SURF_CI8) &&
                (pTexture->dwPaletteHandle == pUpdatePalette->dwPaletteHandle))
            {
                DIRTY_TEXTURE(pContext);
            }
        }
    }
 
    return DD_OK;
#else
    return DD_OK;
#endif // DX7_PALETTETEXTURE
} // D3D_OP_UpdatePalette

//-----------------------------------------------------------------------------
//
// _D3D_OP_SetPalette
//
// This function processes the D3DDP2OP_SETPALETTE DP2 command token.
//
//      Note : This function is need to skip D3DDP2OP_SETPALETTE sent down 
//             by some DX6 apps, even if when PALETTE TEXTURE is not supported
//
//-----------------------------------------------------------------------------

HRESULT _D3D_OP_SetPalettes(P3_D3DCONTEXT* pContext,
                            D3DHAL_DP2SETPALETTE* pSetPalettes,
                            int iNumSetPalettes)
{
#if DX7_PALETTETEXTURE
    int i;
    P3_SURF_INTERNAL* pTexture;
    D3DHAL_DP2UPDATEPALETTE* pPalette;
    
    // Loop to process N surface palette association
    for (i = 0; i < iNumSetPalettes; i++, pSetPalettes++)
    {

        DISPDBG((DBGLVL,"SETPALETTE: Binding surf # %d to palette # %d",
                        pSetPalettes->dwSurfaceHandle,
                        pSetPalettes->dwPaletteHandle));               
    
        // Find internal surface pointer from handle
        pTexture = GetSurfaceFromHandle(pContext, 
                                        pSetPalettes->dwSurfaceHandle);
        if (! pTexture)
        {
            // Associated texture can't be found
            DISPDBG((WRNLVL, 
                     "SetPalettes : invalid texture handle %08lx",
                     pSetPalettes->dwSurfaceHandle));
            return DDERR_INVALIDPARAMS;
        }
 
        // Create the internal palette structure if necessary
        if (pSetPalettes->dwPaletteHandle)
        {
            // Find internal palette pointer from handle
            pPalette = GetPaletteFromHandle(pContext,
                                            pSetPalettes->dwPaletteHandle);    
    
            if (! pPalette)
            {
                pPalette = (D3DHAL_DP2UPDATEPALETTE *)
                                HEAP_ALLOC(FL_ZERO_MEMORY,
                                           sizeof(D3DHAL_DP2UPDATEPALETTE) 
                                                    + LUT_ENTRIES*sizeof(DWORD),
                                           ALLOC_TAG_DX(P));
                // Out of memory case
                if (! pPalette) 
                {
                    DISPDBG((WRNLVL, "_D3D_OP_SetPalettes : Out of memory."));
                    return DDERR_OUTOFMEMORY;
                }

                // Add this texture to the surface list
                if (! PA_SetEntry(pContext->pPalettePointerArray, 
                                  pSetPalettes->dwPaletteHandle, 
                                  pPalette))
                {
                    HEAP_FREE(pPalette);
                    DISPDBG((WRNLVL, "_D3D_OP_SetPalettes : "
                                     "PA_SetEntry() failed."));
                    return DDERR_OUTOFMEMORY;
                }

                // Set up the internal data structure
                pPalette->dwPaletteHandle = pSetPalettes->dwPaletteHandle;
                pPalette->wStartIndex = 0;
                pPalette->wNumEntries = LUT_ENTRIES;
            } 
        }

        // Record palette handle and flags in internal surface data
        pTexture->dwPaletteHandle = pSetPalettes->dwPaletteHandle;
        pTexture->dwPaletteFlags = pSetPalettes->dwPaletteFlags;

        // Mark texture as dirty if current texture is affected
        if ((pContext->TextureStageState[0].m_dwVal[D3DTSS_TEXTUREMAP] == 
                                               pSetPalettes->dwSurfaceHandle) ||
            (pContext->TextureStageState[1].m_dwVal[D3DTSS_TEXTUREMAP] == 
                                               pSetPalettes->dwSurfaceHandle))
        {
            DIRTY_TEXTURE(pContext);
        }

    }

    return DD_OK;
#else
    return DD_OK;
#endif // DX7_PALETTETEXTURE
} // _D3D_OP_SetPalettes

#if DX9_SCISSORRECT

//-----------------------------------------------------------------------------
//
// _D3D_OP_SetScissorRect
//
// This function processes the D3DDP2OP_SETSCISSORRECT DP2 command token.
//
//-----------------------------------------------------------------------------

VOID _D3D_OP_SetScissorRect(P3_D3DCONTEXT* pContext,
                            D3DHAL_DP2SETSCISSORRECT* lpSRect)
{
#if DX7_D3DSTATEBLOCKS
    if ( pContext->bStateRecMode )
    {
         _D3D_SB_Record_ScissorRect(pContext, lpSRect);
    }
    else
#endif // DX7_D3DSTATEBLOCKS
    {
        pContext->ScissorRect = *lpSRect;
        
        if (pContext->RenderStates[D3DRS_SCISSORTESTENABLE])
        {
            DIRTY_VIEWPORT(pContext);
        }
    }
} // _D3D_OP_SetScissorRect

#endif // DX9_SCISSORRECT

#if DX9_DP2BLT

//-----------------------------------------------------------------------------
//
// _D3D_OP_Blt
//
// This function processes the D3DDP2OP_BLT DP2 command token.
//
//-----------------------------------------------------------------------------

VOID 
_D3D_OP_Blt(
    P3_D3DCONTEXT* pContext,
    D3DHAL_DP2BLT* pBlt)
{
    P3_THUNKEDDATA* pThisDisplay = pContext->pThisDisplay;
    P3_SURF_INTERNAL* pSrcSurfInt;
    P3_SURF_INTERNAL* pDstSurfInt;
    
    // Find the internal surface structure from the D3D surface handle
    pSrcSurfInt = GetSurfaceFromHandle(pContext, pBlt->dwSource);    
    pDstSurfInt = GetSurfaceFromHandle(pContext, pBlt->dwDest);
    if ((! pSrcSurfInt) || (! pDstSurfInt))
    {
        DISPDBG((ERRLVL, "_D3D_OP_Blt : Invalid surface"));
        return;
    }
    
    // Call the D3D BLT engine to do the job
    _OP_D3DBlt(pContext, 
               pSrcSurfInt, 
               pDstSurfInt,
               pBlt->dwDest,
               pBlt->dwSourceMipLevel,
               pBlt->dwDestMipLevel,
               pBlt->Flags,
               &pBlt->rSource,
               &pBlt->rDest);

}

//-----------------------------------------------------------------------------
//
// _D3D_OP_ColorFill
//
// This function processes the D3DDP2OP_COLORFILL DP2 command token.
//
//-----------------------------------------------------------------------------

VOID 
_D3D_OP_ColorFill(
    P3_D3DCONTEXT* pContext,
    D3DHAL_DP2COLORFILL* pColorFill)
{
    P3_THUNKEDDATA* pThisDisplay = pContext->pThisDisplay;
    P3_SURF_INTERNAL* pSurfInt;
    DWORD color;

    // Find the internal surface structure from the D3D surface handle
    pSurfInt = GetSurfaceFromHandle(pContext, pColorFill->dwSurface);    
    if ((! pSurfInt) || (pSurfInt->bMipMap) || 
        (! (pSurfInt->ddsCapsInt.dwCaps & DDSCAPS_LOCALVIDMEM)))
    {
        DISPDBG((ERRLVL, "_D3D_OP_ColorFill : Invalid surface"));
        return;
    }

    // Set up the color to fill for the surface
    color = _OP_PrepareFillColor(pSurfInt->pixFmt.dwRBitMask, 
                                 pSurfInt->pixFmt.dwRGBAlphaBitMask,
                                 pSurfInt->dwPixelSize, 
                                 pColorFill->Color);

    // Call the BLT engine to fill the surface
    _DD_BLT_P3Clear(pThisDisplay, 
                    &pColorFill->rRect, 
                    color, 
                    FALSE, 
                    FALSE,
                    pSurfInt->fpVidMem,
                    pSurfInt->dwPatchMode,
                    pSurfInt->dwPixelPitch,
                    pSurfInt->pixFmt.dwRGBBitCount);
}

//-----------------------------------------------------------------------------
//
// _D3D_OP_SurfaceBlt
//
// This function processes the D3DDP2OP_SURFACEBLT DP2 command token.
//
//-----------------------------------------------------------------------------

VOID
_D3D_OP_SurfaceBlt(
    P3_D3DCONTEXT*          pContext,
    D3DHAL_DP2SURFACEBLT*   pSurfBlt)
{
    P3_THUNKEDDATA*         pThisDisplay = pContext->pThisDisplay;
    P3_SURF_INTERNAL*       pSrcSurfInt;
    P3_SURF_INTERNAL*       pDstSurfInt;
    
    // Find the internal surface structure from the D3D surface handle
    pSrcSurfInt = GetSurfaceFromHandle(pContext, pSurfBlt->dwSource);    
    pDstSurfInt = GetSurfaceFromHandle(pContext, pSurfBlt->dwDest);
    if ((! pSrcSurfInt) || (! pDstSurfInt))
    {
        DISPDBG((ERRLVL, "_D3D_OP_SurfaceBlt : Invalid surface"));
        return;
    }

#if DBG
    // Valid the assumption : src must be sysmem, dest must be vidmem
    if ((pSrcSurfInt->Location != SystemMemory) || 
        (pDstSurfInt->Location == SystemMemory)) 
    {
        DISPDBG((ERRLVL, "_D3D_OP_SurfaceBlt : Invalid surface location"));
        return;
    }
#endif

    // Call the D3D blt engine funtion to handle it
    _OP_D3DBlt(pContext, 
               pSrcSurfInt, 
               pDstSurfInt, 
               pSurfBlt->dwDest, 
               pSurfBlt->dwSourceMipLevel, 
               pSurfBlt->dwDestMipLevel, 
               0, 
               &pSurfBlt->rSource, 
               &pSurfBlt->rDest);

}

#endif // DX9_DP2BLT

#if DX9_AUTOGENMIPMAP
//-----------------------------------------------------------------------------
//
// _D3D_OP_GenerateMipLevels
//
// This function processes the D3DDP2OP_GENERATEMIPSUBLEVELS DP2 command token.
//
//-----------------------------------------------------------------------------

VOID
_D3D_OP_GenerateMipLevels(
    P3_D3DCONTEXT* pContext,
    D3DHAL_DP2GENERATEMIPSUBLEVELS* pGenMipLevels)
{
    P3_THUNKEDDATA *pThisDisplay = pContext->pThisDisplay;
    P3_SURF_INTERNAL *pSurfInt;
    int i;
    MIPTEXTURE *pSrcLevel, *pDestLevel;
    FLATPTR fpSrcVidMem, fpDestVidMem;
    DWORD dwSrcOffset, dwDestOffset;
    DDCOLORKEY ddckDummy = { 0 , 0 };
    RECTL rSource, rDest;
    P3_DMA_DEFS();

    // Find the internal surface structure from the D3D surface handle
    pSurfInt = GetSurfaceFromHandle(pContext, pGenMipLevels->hSurface);
    if ((! pSurfInt) || (! pSurfInt->bMipMap))
    {
        DISPDBG((ERRLVL, "_D3D_OP_GenerateMipLevels : Invalid surface"));
        return;
    }

#if DX7_TEXMANAGEMENT
    // Preload managed texture
    if (! _D3D_TM_Preload_Tex_IntoVidMem(pContext, pSurfInt))
    {
        DISPDBG((ERRLVL, 
                 "_D3D_OP_GenerateMipLevels : failed to load managed texture"));
        return;
    }
#endif

    // Switch to hw Ddraw context in order to do the clears
    DDRAW_OPERATION(pContext, pThisDisplay);

    // Init the source and destination rectangles
    rSource.left = rSource.top = rDest.left = rDest.top = 0;

    // Generate the mip levels
    for (i = 0; i < pSurfInt->iMipLevels - 1; i++)
    {
        // Get the source and destination levels for the stretch blt
        pSrcLevel  = &pSurfInt->MipLevels[i];
        pDestLevel = (pSrcLevel + 1);

        // Prepare the source and destination rectangles
        rSource.right  = pSrcLevel->wWidth;
        rSource.bottom = pSrcLevel->wHeight;
        rDest.right    = pDestLevel->wWidth;
        rDest.bottom   = pDestLevel->wHeight;

        // Prepare the input parameters for the blt engine
#if DX7_TEXMANAGEMENT
        if (pSurfInt->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
        {
            fpSrcVidMem  = D3DTMMIPLVL_GETPOINTER(pSrcLevel, pThisDisplay);
            fpDestVidMem = D3DTMMIPLVL_GETPOINTER(pDestLevel, pThisDisplay);
            dwSrcOffset  = D3DTMMIPLVL_GETOFFSET(pSrcLevel, pThisDisplay);
            dwDestOffset = D3DTMMIPLVL_GETOFFSET(pDestLevel, pThisDisplay);
        }
        else
#endif // DX7_TEXMANAGEMENT
        {
            fpSrcVidMem  = pSrcLevel->fpVidMem;
            fpDestVidMem = pDestLevel->fpVidMem;
            dwSrcOffset  = pSrcLevel->dwOffsetFromMemoryBase;
            dwDestOffset = pDestLevel->dwOffsetFromMemoryBase;
        }

        // Validate the src & destination level
        if ((! pDestLevel->wWidth) || (! pDestLevel->wHeight) ||
            (! dwSrcOffset) || (! dwDestOffset))
        {
            DISPDBG((ERRLVL, "_D3D_OP_GenerateMipLevels : invalid dest level"));
            return;
        }

        // Using the stretch blt engine to generate subsequent mip level
        // Note : Blt need to support float point coordinates for good filter
        //        if the chip doesn't support it, then this may not be worth it
        //        Shift 1 sample to the right and to the bottom for point filter
        _DD_P3BltStretchSrcChDstCh(
            pThisDisplay,
            // src data
            fpSrcVidMem,
            pSurfInt->pFormatSurface,
            pSurfInt->dwPixelSize,
            pSrcLevel->wWidth,
            pSrcLevel->wHeight,
            pSrcLevel->P3RXTextureMapWidth.Width,
            pSurfInt->dwPatchMode,
            dwSrcOffset,
            pSurfInt->dwFlagsInt,
            &pSurfInt->pixFmt,
            0,                    // Src must be local vid mem
            // dest data
            fpDestVidMem,
            pSurfInt->pFormatSurface,
            pSurfInt->dwPixelSize,
            pDestLevel->wWidth,
            pDestLevel->wHeight,
            pDestLevel->P3RXTextureMapWidth.Width,
            pSurfInt->dwPatchMode,
            dwDestOffset,
            // no special effect
            0,
            0,
            pGenMipLevels->Filter == D3DTEXF_POINT ?
                (DP2BLT_POINT | (0x2 << 16)) : DP2BLT_LINEAR,
            ddckDummy,
            ddckDummy,
            &rSource,
            &rDest);

        // Wait for the rendering into the texture to be over before using it
        P3_DMA_GET_BUFFER_ENTRIES(2);
        SEND_P3_DATA(WaitForCompletion, 0);
        P3_DMA_COMMIT_BUFFER();
    }

#if DX7_TEXMANAGEMENT
    if (pSurfInt->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
    {
        P3_DMA_GET_BUFFER();
        P3_DMA_FLUSH_BUFFER();

        // Wait for outstanding operations before
        // accessing the vidmem source surface
        SYNC_WITH_GLINT;

        // Reflect the changes back to the system memory copy
        for (i = 1; i < pSurfInt->iMipLevels; i++)
        {
            pSrcLevel = pDestLevel = &pSurfInt->MipLevels[i];

            // Prepare the source and destination rectangles
            rSource.right  = pSrcLevel->wWidth;
            rSource.bottom = pSrcLevel->wHeight;

            // Prepare the host pointer to the VidMem copy
            fpSrcVidMem = D3DTMMIPLVL_GETPOINTER(pSrcLevel, pThisDisplay); 

            // Host copy from VidMem to SysMem
            _DD_BLT_SysMemToSysMemCopy(
                fpSrcVidMem,
                pSrcLevel->lPitch,
                pSurfInt->dwBitDepth,
                pDestLevel->fpVidMem,   // Pointer to the system memory copy
                pDestLevel->lPitch,
                pSurfInt->dwBitDepth,
                &rSource,
                &rSource);
        }
    }
#endif

    // Switch back to the Direct3D context
    D3D_OPERATION(pContext, pThisDisplay);
}
#endif // DX9_AUTOGENMIPMAP

#if DX9_DDI
//-----------------------------------------------------------------------------
//
// _D3D_OP_VertexShader_SetConstB
//
// This function processes the D3DDP2OP_SETVERTEXSHADERCONSTB DP2 command token.
//
//-----------------------------------------------------------------------------

VOID 
_D3D_OP_VertexShader_SetConstB(
    P3_D3DCONTEXT* pContext,
    D3DHAL_DP2SETVERTEXSHADERCONSTB *pVtxShaderConstB)
{
    // Set up the vertex shader boolean constants using the 
    // bitmask in pVtxShaderConstB
}

#if DX9_ASYNC_NOTIFICATIONS
//-----------------------------------------------------------------------------
//
// __OP_FindQuery
//
// Helper function to find the Query with specified ID
//
//-----------------------------------------------------------------------------
P3Query*
__OP_FindQuery(
    P3_D3DCONTEXT* pContext,
    DWORD dwQueryID)
{
    LIST_ENTRY* pNextEntry;
    P3Query* pQuery;

    // Find the query with the specified ID
    
    pNextEntry = pContext->queryList.Flink;

    while (pNextEntry != &pContext->queryList)
    {
        pQuery = CONTAINING_RECORD(
                     pNextEntry,
                     P3Query,
                     queryLink);

        if (pQuery->dp2Query.dwQueryID == dwQueryID)
        {
            return (pQuery);
        }

        // Move to the next query
        pNextEntry = pNextEntry->Flink;
    }

    // No query has the specified ID
    return (NULL);
}
#endif

//-----------------------------------------------------------------------------
//
// _D3D_OP_CreateQuery
//
// This function processes the D3DDP2OP_CREATEQUERY DP2 command token.
//
//-----------------------------------------------------------------------------

HRESULT 
_D3D_OP_CreateQuery(
    P3_D3DCONTEXT* pContext,
    D3DHAL_DP2CREATEQUERY *pCreateQuery)
{
#if DX9_ASYNC_NOTIFICATIONS    
    P3Query *pNewQuery;

    // Handle the query supported by the driver. Notice we need to skip the
    // token even if we can't process any queries.

    switch (pCreateQuery->QueryType)
    {
        case D3DQUERYTYPE_VERTEXSTATS:
        case D3DQUERYTYPE_EVENT:
            break;

        default:
            DISPDBG((ERRLVL, "_D3D_OP_Query : Invalid query (unsupported)"));
            return (DDERR_INVALIDPARAMS);
    }

    // Ever time a query is created, the runtime batches a DP2 token with the 
    // query type and a unique handle. When the command batch is flushed to 
    // the driver, the driver parses these queries and translates them to its 
    // hardware specific commands. If some of the previously issued queries 
    // have completed, the driver sets a flag on the return of a DP2 call and 
    // overwrites the outgoing command buffer with the requisite data. 
    // The runtime on seeing the flag set parses the returned command buffer 
    // and updates its internal data-structures. 

#if DBG
    if (__OP_FindQuery(pContext, pCreateQuery->dwQueryID))
    {
        return (DDERR_INVALIDPARAMS);
    }
#endif

    // Allocate enough memory to remember the query and (if necessary) store
    // the result and keep it in our current context

    pNewQuery = (P3Query *)HEAP_ALLOC(FL_ZERO_MEMORY, 
                                      sizeof(P3Query), 
                                      ALLOC_TAG_DX(Q));        
        
    if (! pNewQuery)
    {
        DISPDBG((ERRLVL, "_D3D_OP_Query : out of memory to service query"));
        return (DDERR_OUTOFMEMORY);
    }

    // Add query structure to contexts query list
    InsertHeadList(&pContext->queryList, &pNewQuery->queryLink);
    
    // Indicate that the query is not issued
    InitializeListHead(&pNewQuery->issuedQueryLink);

    // We will store the result and id (pCreateQuery->dwQueryID) in order to 
    // send them back to the runtime when conditions permit. Some queries
    // may require waiting or querying the hardware so the result might be
    // fetched in the result reporting function, _D3D_AUX_NotifyQueryResult
    
    // Set up field common to all query types
    pNewQuery->dp2Query        = *pCreateQuery;
    pNewQuery->bResultReady    = FALSE;
    pNewQuery->bResultReported = FALSE;

    switch (pCreateQuery->QueryType)
    {
        case D3DQUERYTYPE_VERTEXSTATS:
            pNewQuery->dwResultSize = sizeof(D3DDEVINFO_D3DVERTEXSTATS);
            break;

        case D3DQUERYTYPE_EVENT:
            pNewQuery->dwResultSize = sizeof(pNewQuery->bRenderIDDone);
            break;

        default:
            break;
    }        

    return (DD_OK);
#else
    return (DD_OK);
#endif // DX9_ASYNC_NOTIFICATIONS
}

//-----------------------------------------------------------------------------
//
// _D3D_OP_IssueQuery
//
// This function processes the D3DDP2OP_ISSUEQUERY DP2 command token.
//
//-----------------------------------------------------------------------------

HRESULT
_D3D_OP_IssueQuery(
    P3_D3DCONTEXT* pContext,
    D3DHAL_DP2ISSUEQUERY *pIssueQuery)
{
#if DX9_ASYNC_NOTIFICATIONS
    P3Query *pQuery;

    // Try to find the query with the specified ID
    if ((pQuery = __OP_FindQuery(pContext, pIssueQuery->dwQueryID)) == NULL)
    {
        return (DDERR_INVALIDPARAMS);
    }

    // Note : Query with the same ID can be re-issued before completion
    RemoveEntryList(&pQuery->issuedQueryLink);

    // Actuall issue the query to the Perm3 chip
    switch (pQuery->dp2Query.QueryType)
    {
        case D3DQUERYTYPE_VERTEXSTATS:
            {
                pQuery->dwResultSize = sizeof(D3DDEVINFO_D3DVERTEXSTATS);
            }
            break;

        case D3DQUERYTYPE_EVENT:
            {
                P3_THUNKEDDATA* pThisDisplay = pContext->pThisDisplay;
                P3_DMA_DEFS();

                pQuery->dwResultSize = sizeof(pQuery->bRenderIDDone);

                // Send the render ID to the GPU
                P3_DMA_GET_BUFFER_ENTRIES(6);

                // Make sure all reads and writes in video memory are finished
                SEND_P3_DATA(WaitForCompletion, 0);

                pQuery->dwRenderID = GET_HOST_RENDER_ID();
                SEND_HOST_RENDER_ID(GET_NEW_HOST_RENDER_ID());

                // Flush the P3 Data
                P3_DMA_COMMIT_BUFFER();
            }
            break;

        default:
            return (DDERR_INVALIDPARAMS);
            break;
    }

    // Insert the query into issued list
    InsertTailList(&pContext->issuedQueryList, &pQuery->issuedQueryLink);

    return (DD_OK);
#else
    return (DD_OK);
#endif // DX9_ASYNC_NOTIFICATIONS
}

//-----------------------------------------------------------------------------
//
// _D3D_OP_DeleteQuery
//
// This function processes the D3DDP2OP_DELETEQUERY DP2 command token.
//
//-----------------------------------------------------------------------------

HRESULT
_D3D_OP_DeleteQuery(
    P3_D3DCONTEXT* pContext,
    D3DHAL_DP2DELETEQUERY *pDeleteQuery)
{
#if DX9_ASYNC_NOTIFICATIONS
    P3Query *pQuery;
    
    // Try to find the query with the specified ID
    if ((pQuery = __OP_FindQuery(pContext, pDeleteQuery->dwQueryID)) == NULL)
    {
        return (DDERR_INVALIDPARAMS);
    }

    // Remove the query from the issued list
    RemoveEntryList(&pQuery->issuedQueryLink);

    // Remove the query from the existent query list
    RemoveEntryList(&pQuery->queryLink);

    // Free the resources used by the query
    HEAP_FREE(pQuery);

    return (DD_OK);
#else
    return (DD_OK);
#endif // DX9_ASYNC_NOTIFICATIONS
}

#if DX9_ASYNC_NOTIFICATIONS   
//-----------------------------------------------------------------------------
//
// _D3D_AUX_NotifyQueryResult
//
// This function notifies back the result of a D3DDP2OP_QUERY DP2 command token.
//
// returns the size of responses written in bytes
//
//-----------------------------------------------------------------------------
DWORD   
_D3D_AUX_NotifyQueryResult(
    P3_D3DCONTEXT* pContext,
    LPBYTE lpCBStart,
    DWORD dwCBLen)
{ 
    LIST_ENTRY *pNextEntry;
    P3Query *pQuery;
    D3DHAL_DP2RESPONSE *pDP2Response, dp2Response;
    D3DHAL_DP2RESPONSEQUERY *pDP2RespQuery;
    DWORD dwQuerySize;
    DWORD dwBytesWritten;
    P3_THUNKEDDATA* pThisDisplay = pContext->pThisDisplay;

    // Make sure the command buffer is not too small
    if (dwCBLen < 2*sizeof(D3DHAL_DP2RESPONSE))
    {
        return (0);
    }

    // make room for 1 D3DDP2OP_RESPONSE and 1 D3DDP2OP_RESPONSECONTINUE
    dwCBLen       -= (2*sizeof(D3DHAL_DP2RESPONSE)); 

    // Prepare the D3DDP2OP_RESPONSE
    pDP2Response = (LPD3DHAL_DP2RESPONSE)lpCBStart;
    dp2Response.bCommand    = D3DDP2OP_RESPONSEQUERY;
    dp2Response.bReserved   = 0;
    dp2Response.wStateCount = 0;       
    dp2Response.dwTotalSize = sizeof(D3DHAL_DP2RESPONSE);

    // Initialize the total bytes of queries written into command buffer
    dwBytesWritten = sizeof(D3DHAL_DP2RESPONSE); 

    // Prepare the pointer to the 1st query response
    pDP2RespQuery = (LPD3DHAL_DP2RESPONSEQUERY)(pDP2Response + 1);

    // Lets go through the query list to see which queries have been 
    // completed (or can be completed) and which fit into the current
    // outgoing command buffer.
    pNextEntry = pContext->issuedQueryList.Flink;

    while (pNextEntry != &pContext->issuedQueryList)
    {
        pQuery = CONTAINING_RECORD(
                     pNextEntry,
                     P3Query,
                     issuedQueryLink);

        // We could at this point verify if the results for the 
        // current query being examined are ready and update the 
        // ->bResultReady field if so.
        switch (pQuery->dp2Query.QueryType)
        {
            case D3DQUERYTYPE_VERTEXSTATS:
                memcpy(&pQuery->vertexStats,
                       &pContext->vertexStats,
                       sizeof(D3DDEVINFO_D3DVERTEXSTATS));    
                pQuery->bResultReady = TRUE;
                break;

            case D3DQUERYTYPE_EVENT:
                if (RENDER_ID_HAS_COMPLETED(pQuery->dwRenderID))
                {
                    pQuery->bRenderIDDone = TRUE;
                    pQuery->bResultReady  = TRUE;
                }
                break;

            default:
                DISPDBG((ERRLVL, 
                         "_D3D__AUX_NotifyQueryResult() : unknown query %x",
                         pQuery->dp2Query.QueryType));
                break;
        }
            
        // Move on to the next query           
        pNextEntry = pNextEntry->Flink; 

        // Is the current query completed ?
        if (! pQuery->bResultReady)
        {
            continue;
        }

        // As the command buffer area lives in user memory, we need to
        // access it bracketing it with a try/except block. This
        // is because the user memory might under some circumstances
        // become invalid while the driver is running and then it
        // would AV. Also, the driver might need to do some cleanup
        // before returning to the OS.
        __try
        {                      
            // Now check if we have enough space in the outgoing
            // command buffer to write it out
            if  ((sizeof(D3DHAL_DP2RESPONSEQUERY)+
                 pQuery->dwResultSize) > dwCBLen)
            {
                pDP2Response = (LPD3DHAL_DP2RESPONSE)pDP2RespQuery;
                
                // Prepare the RESPONSECONTINUE token
                pDP2Response->bCommand    = D3DDP2OP_RESPONSECONTINUE;
                pDP2Response->bReserved   = 0;
                pDP2Response->wStateCount = 1;
                pDP2Response->dwTotalSize = sizeof(D3DHAL_DP2RESPONSE);

                dwBytesWritten += sizeof(D3DHAL_DP2RESPONSE);

                // No space in command buffer is left, bail out
                break;
            }

            // we do have space -> lets place it there
            // (and keep track of the space we have left)

            pDP2RespQuery->dwSize    = pQuery->dwResultSize;
            pDP2RespQuery->dwQueryID = pQuery->dp2Query.dwQueryID;

            if (pQuery->dwResultSize)
            {
                memcpy((pDP2RespQuery + 1), 
                       &pQuery->dwResult, 
                       pQuery->dwResultSize);
            }

            dwQuerySize = sizeof(D3DHAL_DP2RESPONSEQUERY) + 
                          pQuery->dwResultSize;

            dp2Response.wStateCount++;
            dp2Response.dwTotalSize += dwQuerySize;
            memcpy(pDP2Response, &dp2Response, sizeof(D3DHAL_DP2RESPONSE));

            dwCBLen        -= dwQuerySize;
            dwBytesWritten += dwQuerySize;

            // Move the pointer for the next query
            pDP2RespQuery = 
                (LPD3DHAL_DP2RESPONSEQUERY)(((LPBYTE)pDP2RespQuery) + 
                                            dwQuerySize);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            // On this driver we don't need to do anything special
            // to recover from this exception (user mem AV)
            DISPDBG((ERRLVL,"Driver caused exception at "
                            "line %u of file %s",
                            __LINE__,__FILE__));
            return (0);
        }

        // Mark the query for removal from the issued list
        pQuery->bResultReported = TRUE;
    }

    // Remove the reported query from the issued list
    pNextEntry = pContext->issuedQueryList.Flink;

    while (pNextEntry != &pContext->issuedQueryList)
    {
        pQuery = CONTAINING_RECORD(
                     pNextEntry,
                     P3Query,
                     issuedQueryLink);

        // Move on to the next issued query
        pNextEntry = pNextEntry->Flink;

        if (pQuery->bResultReported)
        {
            pQuery->bResultReady    = FALSE;
            pQuery->bResultReported = FALSE;

            RemoveEntryList(&pQuery->issuedQueryLink);
        }
    }

    // If no query response has been actually written into the command buffer
    if (dwBytesWritten == sizeof(D3DHAL_DP2RESPONSE))
    {
        dwBytesWritten = 0;
    }

    return (dwBytesWritten);
}
#endif // DX9_ASYNC_NOTIFICATIONS

//-----------------------------------------------------------------------------
//
// _D3D_OP_VertexShaderDecl_Create
//
// This function creates vertex shader declaration
//
//-----------------------------------------------------------------------------
HRESULT 
_D3D_OP_VertexShaderDecl_Create(
    P3_D3DCONTEXT* pContext,
    DWORD dwVtxShaderDeclHandle,
    DWORD dwNumVtxShaderDeclElems,
    D3DVERTEXELEMENT9 *pVtxShaderDeclElems)
{
    P3VtxShaderDecl *pVtxShaderDecl;
    DWORD dwVtxShaderDeclSize;
    BOOL bRet;
 
    // Check whether the vertex shader declaration exists
    pVtxShaderDecl = (P3VtxShaderDecl *)
                     PA_GetEntry(pContext->pVtxShaderDeclPointerArray,
                                 dwVtxShaderDeclHandle);
    if (pVtxShaderDecl)
    {
        DISPDBG((ERRLVL, "_D3D_OP_VertexShaderDecl_Create :"
                 "Vertex shader declaration already exists.\n"));
        return (DDERR_INVALIDPARAMS);
    }

    // Allocate the new vertex shader declaration
    dwVtxShaderDeclSize = dwNumVtxShaderDeclElems * sizeof(D3DVERTEXELEMENT9);
    pVtxShaderDecl = (P3VtxShaderDecl *)
                     HEAP_ALLOC(HEAP_ZERO_MEMORY,
                                sizeof(P3VtxShaderDecl) + dwVtxShaderDeclSize,
                                ALLOC_TAG_DX(6));
    if (! pVtxShaderDecl) 
    {
        return (DDERR_OUTOFMEMORY);
    }

    // Copy vertex shader declaration data
    pVtxShaderDecl->dwNumVtxShaderDeclElems = dwNumVtxShaderDeclElems;
    memcpy((PBYTE)(pVtxShaderDecl + 1), 
           pVtxShaderDeclElems, dwVtxShaderDeclSize);

    // Record the new vertex shader declararion in the pointer array
    bRet = PA_SetEntry(pContext->pVtxShaderDeclPointerArray,
                       dwVtxShaderDeclHandle,
                       pVtxShaderDecl);

    if (bRet)
    {
        return (DD_OK);
    }
    else
    {
        return (DDERR_OUTOFMEMORY);
    }
}

//-----------------------------------------------------------------------------
//
// _D3D_OP_VertexShaderDecl_Create
//
// This function deletes vertex shader declaration
//
//-----------------------------------------------------------------------------
HRESULT 
_D3D_OP_VertexShaderDecl_Delete(
    P3_D3DCONTEXT* pContext,
    DWORD dwVtxShaderDeclHandle)
{
    PVOID pVtxShaderDecl;

    pVtxShaderDecl = PA_GetEntry(pContext->pVtxShaderDeclPointerArray,
                                 dwVtxShaderDeclHandle);
    if (pVtxShaderDecl)
    {
        PA_SetEntry(pContext->pVtxShaderDeclPointerArray,
                    dwVtxShaderDeclHandle, 
                    NULL);
    
        HEAP_FREE(pVtxShaderDecl);
    }
    else
    {
        DISPDBG((ERRLVL, "_D3D_OP_VertexShaderDecl_Delete : "
                 "Vertex shader declaration doesn't exist.\n"));
    }
 
    return (DD_OK);
}

//-----------------------------------------------------------------------------
//
// _D3D_OP_VertexShaderDecl_Create
//
// This function sets the current vertex shader declaration
//
//-----------------------------------------------------------------------------
HRESULT 
_D3D_OP_VertexShaderDecl_Set(
    P3_D3DCONTEXT* pContext,
    DWORD dwVtxShaderDeclHandle)
{
    pContext->dwVertexType = dwVtxShaderDeclHandle;
    return (DD_OK);
}

#define VTX_DECL_COLOR_USAGE_INDEX_DIFFUSE    0
#define VTX_DECL_COLOR_USAGE_INDEX_SPECULAR   1

HRESULT 
__CheckVtxShaderDecl(P3_D3DCONTEXT *pContext, DWORD dwVtxShaderDecl)
{
    HRESULT hr;
    P3VtxShaderDecl *pVtxShaderDecl;
    D3DVERTEXELEMENT9 *pCurVtxShaderDeclElm;
    FVFOFFSETS *pFVFData;
    DWORD i;
    DWORD dwCurStride, dwCurUsageIndex, dwCurTexStride;

    // Initialize the return value
    hr = DD_OK;

    // Check whether the vertex shader declaration exists
    pVtxShaderDecl = (P3VtxShaderDecl *)
                     PA_GetEntry(pContext->pVtxShaderDeclPointerArray,
                                 dwVtxShaderDecl);
    if (! pVtxShaderDecl)
    {
        DISPDBG((ERRLVL, "__CheckVtxShaderDecl :"
                 "Vertex shader declaration doesn't exist.\n"));
        return (DDERR_INVALIDPARAMS);
    }

    pFVFData = &pContext->FVFData;

    // Ensure the default offsets are setup
    ZeroMemory(pFVFData, sizeof(FVFOFFSETS));
    pFVFData->dwNonTexStride = (DWORD)-1;

    // Initialize the vertex stride
    dwCurStride = 0;
    
    // Set up the FVF offset data
    pCurVtxShaderDeclElm = (D3DVERTEXELEMENT9 *)(pVtxShaderDecl + 1); 
    for (i = 0; i < pVtxShaderDecl->dwNumVtxShaderDeclElems; i++)
    {
        // Sanity checks
        if (pCurVtxShaderDeclElm->Stream)
        {
            hr = DDERR_INVALIDPARAMS;
            break;
        }

        switch (pCurVtxShaderDeclElm->Usage)
        {
            case D3DDECLUSAGE_POSITIONT:
                // XYZ RHW has to be at the beginning of vertex data
                if ((pCurVtxShaderDeclElm->Type != D3DDECLTYPE_FLOAT4) ||
                    (pCurVtxShaderDeclElm->Offset))
                {
                    hr = DDERR_INVALIDPARAMS;
                    break;
                }
                dwCurStride += (4 * sizeof(D3DVALUE));
                break;

            case D3DDECLUSAGE_COLOR:
                dwCurUsageIndex = pCurVtxShaderDeclElm->UsageIndex;
                if ((pCurVtxShaderDeclElm->Type != D3DDECLTYPE_D3DCOLOR) ||
                    (pCurVtxShaderDeclElm->Offset != dwCurStride) ||
                    (dwCurUsageIndex > VTX_DECL_COLOR_USAGE_INDEX_SPECULAR))
                {
                    hr = DDERR_INVALIDPARAMS;
                    break;
                }
                if (dwCurUsageIndex == VTX_DECL_COLOR_USAGE_INDEX_DIFFUSE)
                {
                    pFVFData->dwColOffset = dwCurStride;
                }
                else
                {
                    pFVFData->dwSpcOffset = dwCurStride;;
                }
                dwCurStride += (sizeof(DWORD));
                break;

            case D3DDECLUSAGE_TEXCOORD:
                dwCurUsageIndex = pCurVtxShaderDeclElm->UsageIndex;
                // Only allow D3DDECLTYPE_FLOAT1/2/3/4, and 2 sets of tex coords
                if ((pCurVtxShaderDeclElm->Type > D3DDECLTYPE_FLOAT4) ||
                    (dwCurUsageIndex >= D3DHAL_TSS_MAXSTAGES)) 
                {
                    hr = DDERR_INVALIDPARAMS;
                    break;
                }
                pFVFData->dwTexOffset_ForStage[dwCurUsageIndex] = 
                pFVFData->dwTexOffset_ForCoordSet[dwCurUsageIndex] = 
                    pCurVtxShaderDeclElm->Offset;
                pFVFData->dwTexDim_ForStage[dwCurUsageIndex] = 
                pFVFData->dwTexDim_ForCoordSet[dwCurUsageIndex] = 
                    ((DWORD)pCurVtxShaderDeclElm->Type) + 1;

                dwCurTexStride = pCurVtxShaderDeclElm->Offset +
                                 (pFVFData->dwTexDim_ForStage[dwCurUsageIndex] *
                                  sizeof(D3DVALUE));
                if (dwCurTexStride > dwCurStride)
                {
                    dwCurStride = dwCurTexStride;
                }
                // Set the non tex coord stride
                if (pFVFData->dwNonTexStride > pCurVtxShaderDeclElm->Offset)
                {
                    pFVFData->dwNonTexStride = pCurVtxShaderDeclElm->Offset;
                }
                // Increase the number of tex coords
                pFVFData->dwTexCount++;
                break;

            case D3DDECLUSAGE_PSIZE: 
                if (pCurVtxShaderDeclElm->Type != D3DDECLTYPE_FLOAT1)
                {
                    hr = DDERR_INVALIDPARAMS;
                    break;
                }
                pFVFData->dwPntSizeOffset = pCurVtxShaderDeclElm->Offset;
                if ((pCurVtxShaderDeclElm->Offset + sizeof(D3DVALUE)) > dwCurStride)
                {
                    dwCurStride = pCurVtxShaderDeclElm->Offset + sizeof(D3DVALUE);
                }
                break;

            default:
                hr = DDERR_INVALIDPARAMS;
                break;
        }

        // Bail out if an error was detected
        if (hr != DD_OK)
        {
            break;
        }

        // Move on the next vertex shader element
        pCurVtxShaderDeclElm++;
    }

    if (hr == DD_OK)
    {
        pFVFData->dwStride = dwCurStride;
        if (pFVFData->dwNonTexStride == (DWORD)-1)
        {
            pFVFData->dwNonTexStride = dwCurStride;
        }

        // Update the offsets to our current (2) textures as we run this
        // function just before rendering any primitives. Which means we
        // the app has already setup any TSS values it might desire.
        if( pContext->iTexStage[0] != -1 )
        {
            DWORD dwTexCoordSet =
                pContext->TextureStageState[pContext->iTexStage[0]].
                                         m_dwVal[D3DTSS_TEXCOORDINDEX];

            // The texture coordinate index may contain texgen flags
            // in the high word. These flags are not interesting here
            // so we mask them off.
            dwTexCoordSet = dwTexCoordSet & 0x0000FFFFul;

            pContext->FVFData.dwTexOffset_ForStage[0] =
                    pContext->FVFData.dwTexOffset_ForCoordSet[dwTexCoordSet];

            pContext->FVFData.dwTexDim_ForStage[0] =
                    pContext->FVFData.dwTexDim_ForCoordSet[dwTexCoordSet];
        }

        if( pContext->iTexStage[1] != -1 )
        {
            DWORD dwTexCoordSet =
                pContext->TextureStageState[pContext->iTexStage[1]].
                                         m_dwVal[D3DTSS_TEXCOORDINDEX];

            // The texture coordinate index may contain texgen flags
            // in the high word. These flags are not interesting here
            // so we mask them off.
            dwTexCoordSet = dwTexCoordSet & 0x0000FFFFul;

            pContext->FVFData.dwTexOffset_ForStage[1] =
                    pContext->FVFData.dwTexOffset_ForCoordSet[dwTexCoordSet];

            pContext->FVFData.dwTexDim_ForStage[1] =
                    pContext->FVFData.dwTexDim_ForCoordSet[dwTexCoordSet];
        }
    }

    return (hr);
}

#endif // DX9_DDI


