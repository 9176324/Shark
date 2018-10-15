/******************************Module*Header**********************************\
*
*                           **************************
*                           * DirectDraw SAMPLE CODE *
*                           **************************
*
* Module Name: dddownld.c
*
* Content: DirectDraw Blt implementation for sysmem-vidmem blts and clears
*
* Copyright (c) 1994-1999 3Dlabs Inc. Ltd. All rights reserved.
* Copyright (c) 1995-2002 Microsoft Corporation.  All rights reserved.
\*****************************************************************************/

#include "glint.h"
#include "dma.h"
#include "tag.h"

#define UNROLL_COUNT    8   // Number of iterations of transfer in an unrolled loop
#define P3_BLOCK_SIZE      (UNROLL_COUNT * 8)  // # of unrolled loops .
#define GAMMA_BLOCK_SIZE   (UNROLL_COUNT * 2)  // # of unrolled loops .
#define BLOCK_SIZE         (DWORD)((TLCHIP_GAMMA)?GAMMA_BLOCK_SIZE:P3_BLOCK_SIZE)

#define TAGGED_SIZE ((BLOCK_SIZE - 1) << 16)


#define UNROLLED()                  \
    MEMORY_BARRIER();               \
    dmaPtr[0] = pCurrentLine[0];    \
    ulTotalImageDWORDs--;           \
    MEMORY_BARRIER();               \
    dmaPtr[1] = pCurrentLine[1];    \
    ulTotalImageDWORDs--;           \
    MEMORY_BARRIER();               \
    dmaPtr[2] = pCurrentLine[2];    \
    ulTotalImageDWORDs--;           \
    MEMORY_BARRIER();               \
    dmaPtr[3] = pCurrentLine[3];    \
    ulTotalImageDWORDs--;           \
    MEMORY_BARRIER();               \
    dmaPtr[4] = pCurrentLine[4];    \
    ulTotalImageDWORDs--;           \
    MEMORY_BARRIER();               \
    dmaPtr[5] = pCurrentLine[5];    \
    ulTotalImageDWORDs--;           \
    MEMORY_BARRIER();               \
    dmaPtr[6] = pCurrentLine[6];    \
    ulTotalImageDWORDs--;           \
    MEMORY_BARRIER();               \
    dmaPtr[7] = pCurrentLine[7];    \
    ulTotalImageDWORDs--;           \
    MEMORY_BARRIER();               \
    dmaPtr += UNROLL_COUNT;         \
    CHECK_FIFO(UNROLL_COUNT);       \
    pCurrentLine += UNROLL_COUNT;


//-----------------------------------------------------------------------------
//
// _DD_P3Download
//
//
// Function to do an image download to the rectangular region.
// Uses the packed bit on Permedia to do the packing for us.  
//
//-----------------------------------------------------------------------------
void 
_DD_P3Download(
    P3_THUNKEDDATA* pThisDisplay,
    FLATPTR pSrcfpVidMem,
    FLATPTR pDestfpVidMem,    
    DWORD dwSrcChipPatchMode,
    DWORD dwDestChipPatchMode,  
    DWORD dwSrcPitch,
    DWORD dwDestPitch,   
    DWORD dwDestPixelPitch,  
    DWORD dwDestPixelSize,
    RECTL* rSrc,
    RECTL* rDest)
{
    // Work out pixel offset into the framestore of the rendered surface
    ULONG ulSCRoundedUpDWords;
    ULONG ulSCWholeDWords, ulSCDWordsCnt, ulSCExtraBytes;
    ULONG ulTotalImageDWORDs;
    DWORD SrcOffset; 
    DWORD DstOffset; 
    ULONG ulImageLines;
    ULONG count;
    ULONG renderData;
    DWORD dwDownloadTag;
    int rDestleft, rDesttop, rSrcleft, rSrctop;
    RECTL rNewDest;
    BYTE *pSurfaceData;
    UNALIGNED DWORD *pCurrentLine;
    P3_DMA_DEFS();
               
    // Because of a bug in RL we sometimes have to fiddle with these values
    rSrctop = rSrc->top;
    rSrcleft = rSrc->left;
    rDesttop = rDest->top;
    rDestleft = rDest->left;

    // Fix coords origin
    if(!_DD_BLT_FixRectlOrigin("_DD_P3Download", rSrc, rDest))
    {
        // Nothing to be blitted
        return;
    }    
    
       
    SrcOffset = (DWORD)(rSrc->left << dwDestPixelSize) + 
                                    (rSrc->top * dwSrcPitch);
    DstOffset = (DWORD)(rDest->left << dwDestPixelSize) + 
                                    (rDest->top * dwDestPitch);

    ulSCRoundedUpDWords = rDest->right - rDest->left;
    ulImageLines = rDest->bottom - rDest->top;

    P3_DMA_GET_BUFFER();
    P3_ENSURE_DX_SPACE(16);
    WAIT_FIFO(16);

    SEND_P3_DATA(FBWriteBufferAddr0, 
                        (DWORD)(pDestfpVidMem - 
                                   pThisDisplay->dwScreenFlatAddr) );
                                
    SEND_P3_DATA(FBWriteBufferWidth0, dwDestPixelPitch);
    SEND_P3_DATA(FBWriteBufferOffset0, 
                                        (rDest->top << 16) | 
                                        (rDest->left & 0xFFFF));

    SEND_P3_DATA(LogicalOpMode, 7);

    SEND_P3_DATA(PixelSize, (2 - dwDestPixelSize));

    SEND_P3_DATA(FBWriteMode, 
                    P3RX_FBWRITEMODE_WRITEENABLE(__PERMEDIA_ENABLE) |
                    P3RX_FBWRITEMODE_LAYOUT0(P3RX_LAYOUT_LINEAR));
                    
    SEND_P3_DATA(FBDestReadMode, 
                    P3RX_FBDESTREAD_READENABLE(__PERMEDIA_DISABLE) |
                    P3RX_FBDESTREAD_LAYOUT0(P3RX_LAYOUT_LINEAR));

    dwDownloadTag = Color_Tag;

    rNewDest = *rDest;

    DISPDBG((DBGLVL, "Image download %dx%d", ulSCRoundedUpDWords, ulImageLines));

    // ulSCWholeDWords is the number of whole DWORDs along each scanline
    // ulSCExtraBytes  is the number of extra BYTEs at the end of each scanline
    // ulSCRoundedUpDWords is the size of each scanline rounded up to DWORDs
    if (dwDestPixelSize != __GLINT_32BITPIXEL) 
    {
        if (dwDestPixelSize == __GLINT_8BITPIXEL) 
        {
            ulSCExtraBytes  = ulSCRoundedUpDWords & 3;
            ulSCWholeDWords = ulSCRoundedUpDWords >> 2;
            ulSCRoundedUpDWords = (ulSCRoundedUpDWords + 3) >> 2; 

            if (dwDownloadTag != Color_Tag)
            {
                rNewDest.right = rNewDest.left + (ulSCRoundedUpDWords << 2);
            }
        }
        else 
        {
            ulSCExtraBytes  = (ulSCRoundedUpDWords & 1) << 1;
            ulSCWholeDWords = ulSCRoundedUpDWords >> 1;
            ulSCRoundedUpDWords = (ulSCRoundedUpDWords + 1) >> 1;

            if (dwDownloadTag != Color_Tag)
            {
                rNewDest.right = rNewDest.left + (ulSCRoundedUpDWords << 1);
            }
        }
    }
    else
    {
        ulSCExtraBytes = 0;
        ulSCWholeDWords = ulSCRoundedUpDWords;
    }

    // Calc the total number of image DWORDs to send to GPU
    ulTotalImageDWORDs = ulImageLines * ulSCWholeDWords;

    P3_ENSURE_DX_SPACE(20);
    WAIT_FIFO(20);        

    SEND_P3_DATA(FBSourceReadMode, 
                      P3RX_FBSOURCEREAD_READENABLE(__PERMEDIA_DISABLE) |
                      P3RX_FBSOURCEREAD_LAYOUT(dwSrcChipPatchMode));

    SEND_P3_DATA(RectanglePosition, 0);

    if (dwDownloadTag == Color_Tag)
    {
        renderData =  P3RX_RENDER2D_WIDTH((rNewDest.right - rNewDest.left) & 0xfff )
                    | P3RX_RENDER2D_HEIGHT((rNewDest.bottom - rNewDest.top ) & 0xfff )
                    | P3RX_RENDER2D_OPERATION( P3RX_RENDER2D_OPERATION_SYNC_ON_HOST_DATA )
                    | P3RX_RENDER2D_SPANOPERATION( P3RX_RENDER2D_SPAN_VARIABLE )
                    | P3RX_RENDER2D_INCREASINGX( __PERMEDIA_ENABLE )
                    | P3RX_RENDER2D_INCREASINGY( __PERMEDIA_ENABLE );
        SEND_P3_DATA(Render2D, renderData);
    }
    else
    {
        // Don't use spans for the unpacking scheme, but use the 2D Setup
        // unit to do the work of setting up the destination
        SEND_P3_DATA(ScissorMinXY, 0);
        SEND_P3_DATA(ScissorMaxXY, P3RX_SCISSOR_X_Y(rDest->right, rDest->bottom));
        SEND_P3_DATA(ScissorMode, P3RX_SCISSORMODE_USER(__PERMEDIA_ENABLE));
        
        renderData =  P3RX_RENDER2D_WIDTH( (rNewDest.right - rNewDest.left) & 0xfff )
                    | P3RX_RENDER2D_HEIGHT( 0 )
                    | P3RX_RENDER2D_OPERATION( P3RX_RENDER2D_OPERATION_SYNC_ON_HOST_DATA )
                    | P3RX_RENDER2D_INCREASINGX( __PERMEDIA_ENABLE )
                    | P3RX_RENDER2D_INCREASINGY( __PERMEDIA_ENABLE );
        SEND_P3_DATA(Render2D, renderData);

        SEND_P3_DATA(Count, rDest->bottom - rDest->top );
        SEND_P3_DATA(Render, __RENDER_TRAPEZOID_PRIMITIVE | __RENDER_SYNC_ON_HOST_DATA);
    }

    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);

    __try
    {
        pSurfaceData = (BYTE *)pSrcfpVidMem + SrcOffset;
        pCurrentLine = (DWORD *)pSurfaceData;
        
        while (ulImageLines-- > 0)
        {
            DISPDBG((DBGLVL, "Image download lines %d", ulImageLines));

            // Initialize the number of DWORDs counter
            ulSCDWordsCnt = ulSCWholeDWords;

            // Send the texels in DWORDS
            while (ulSCDWordsCnt >= BLOCK_SIZE)
            {
                P3_ENSURE_DX_SPACE(BLOCK_SIZE + 1);
                WAIT_FIFO(BLOCK_SIZE + 1);
                ADD_FUNNY_DWORD(TAGGED_SIZE | dwDownloadTag);

                for (count = BLOCK_SIZE / UNROLL_COUNT; count; count--)
                {
                    DISPDBG((DBGLVL, "Image download count %d", count));
                    UNROLLED();
                }
                ulSCDWordsCnt -= BLOCK_SIZE;
            }

            // Finish off the rest of the whole DWORDs on the scanline
            if (ulSCDWordsCnt) 
            {
                P3_ENSURE_DX_SPACE(ulSCDWordsCnt + 1);
                WAIT_FIFO(ulSCDWordsCnt + 1);
                ADD_FUNNY_DWORD(((ulSCDWordsCnt - 1) << 16) | dwDownloadTag);

                for (count = 0; count < ulSCDWordsCnt; count++, pCurrentLine++) 
                {
                    ADD_FUNNY_DWORD(*pCurrentLine);
                    ulTotalImageDWORDs--;
                }
            }

            // Finish off the extra bytes at the end of the scanline
            if (ulSCExtraBytes) 
            {
                DWORD dwTemp;

                P3_ENSURE_DX_SPACE(1 + 1);   // 1 tag + 1 data DWORD
                WAIT_FIFO(1 + 1);
                ADD_FUNNY_DWORD(dwDownloadTag);

                memcpy(&dwTemp, pCurrentLine, ulSCExtraBytes);
                ADD_FUNNY_DWORD(dwTemp);
                ulTotalImageDWORDs--;
            }

            pSurfaceData += dwSrcPitch;
            pCurrentLine = (DWORD*)pSurfaceData;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        
        DISPDBG((ERRLVL, "Perm3 caused exception at line %u of file %s",
                         __LINE__,__FILE__));
        // Send enough DWORDs to the GPU to avoid deadlock
        for (count = 0; count < ulTotalImageDWORDs; count++)
        {
            ADD_FUNNY_DWORD(dwDownloadTag);
            ADD_FUNNY_DWORD(0);                 // Dummy pixel data
        }
    }

    P3_ENSURE_DX_SPACE(4);
    WAIT_FIFO(4);

    SEND_P3_DATA(WaitForCompletion, 0);
    SEND_P3_DATA(ScissorMode, __PERMEDIA_DISABLE);

    // Put back the values if we changed them.
    rSrc->top = rSrctop;
    rSrc->left = rSrcleft;
    rDest->top = rDesttop;
    rDest->left = rDestleft;

    P3_DMA_COMMIT_BUFFER();

}  // _DD_P3Download 

//-----------------------------------------------------------------------------
//
// _DD_P3DownloadDD
//
//
// Function to do an image download to the rectangular region.
// Uses the packed bit on Permedia to do the packing for us.  
//
//-----------------------------------------------------------------------------
void 
_DD_P3DownloadDD(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pSource,
    LPDDRAWI_DDRAWSURFACE_LCL pDest,
    P3_SURF_FORMAT* pFormatSource, 
    P3_SURF_FORMAT* pFormatDest,
    RECTL* rSrc,
    RECTL* rDest)
{
    _DD_P3Download(pThisDisplay,
                   pSource->lpGbl->fpVidMem,
                   pDest->lpGbl->fpVidMem,
                   P3RX_LAYOUT_LINEAR, // src
                   P3RX_LAYOUT_LINEAR, // dst,
                   pSource->lpGbl->lPitch,
                   pDest->lpGbl->lPitch,                   
                   DDSurf_GetPixelPitch(pDest),
                   DDSurf_GetChipPixelSize(pDest),
                   rSrc,
                   rDest);
                     
} // _DD_P3DownloadDD

//-----------------------------------------------------------------------------
//
// _DD_P3DownloadDstCh
//
// Function to do an image download to the rectangular region.
// Uses the packed bit on Permedia to do the packing for us.  
//
//-----------------------------------------------------------------------------
void 
_DD_P3DownloadDstCh(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pSource,
    LPDDRAWI_DDRAWSURFACE_LCL pDest,
    P3_SURF_FORMAT* pFormatSource, 
    P3_SURF_FORMAT* pFormatDest,
    LPDDHAL_BLTDATA lpBlt,
    RECTL* rSrc,
    RECTL* rDest)
{
    // Work out pixel offset into the framestore of the rendered surface
    ULONG ulSCRoundedUpDWords;
    ULONG ulSCWholeDWords, ulSCDWordsCnt, ulSCExtraBytes;
    ULONG ulTotalImageDWORDs;
    DWORD SrcOffset; 
    ULONG ulImageLines;
    ULONG count;
    ULONG renderData;
    DWORD dwDownloadTag;
    int rDestleft, rDesttop, rSrcleft, rSrctop;
    RECTL rNewDest;
    BOOL bDstKey = FALSE;
    P3_DMA_DEFS();
                
    // Because of a bug in RL we sometimes have to fiddle with these values
    rSrctop = rSrc->top;
    rSrcleft = rSrc->left;
    rDesttop = rDest->top;
    rDestleft = rDest->left;

    // Fix coords origin
    if(!_DD_BLT_FixRectlOrigin("_DD_P3DownloadDstCh", rSrc, rDest))
    {
        // Nothing to be blitted
        return;
    }    
    
       
    SrcOffset = (DWORD)(rSrc->left << DDSurf_GetChipPixelSize(pDest)) + (rSrc->top * pSource->lpGbl->lPitch);

    ulSCRoundedUpDWords = rDest->right - rDest->left;
    ulImageLines = rDest->bottom - rDest->top;

    P3_DMA_GET_BUFFER();
    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);

    SEND_P3_DATA(FBWriteBufferAddr0, DDSurf_SurfaceOffsetFromMemoryBase(pThisDisplay, pDest));
    SEND_P3_DATA(FBWriteBufferWidth0, DDSurf_GetPixelPitch(pDest));
    SEND_P3_DATA(FBWriteBufferOffset0, (rDest->top << 16) | (rDest->left & 0xFFFF));

    SEND_P3_DATA(LogicalOpMode, 7);

    SEND_P3_DATA(PixelSize, (2 - DDSurf_GetChipPixelSize(pDest)));

    if (lpBlt->dwFlags & DDBLT_KEYDESTOVERRIDE)
    {
        bDstKey = TRUE;

        // Dest keying.
        // The conventional chroma test is set up to key off the dest - the framebuffer.
        SEND_P3_DATA(ChromaTestMode, P3RX_CHROMATESTMODE_ENABLE(__PERMEDIA_ENABLE) |
                                        P3RX_CHROMATESTMODE_SOURCE(P3RX_CHROMATESTMODE_SOURCE_FBDATA) |
                                        P3RX_CHROMATESTMODE_PASSACTION(P3RX_CHROMATESTMODE_ACTION_PASS) |
                                        P3RX_CHROMATESTMODE_FAILACTION(P3RX_CHROMATESTMODE_ACTION_REJECT)
                                        );

        SEND_P3_DATA(ChromaLower, lpBlt->bltFX.ddckDestColorkey.dwColorSpaceLowValue);
        SEND_P3_DATA(ChromaUpper, lpBlt->bltFX.ddckDestColorkey.dwColorSpaceHighValue);

        // The source buffer is the source for the destination color key
        SEND_P3_DATA(FBSourceReadBufferAddr, DDSurf_SurfaceOffsetFromMemoryBase(pThisDisplay, pDest));
        SEND_P3_DATA(FBSourceReadBufferWidth, DDSurf_GetPixelPitch(pDest));
        SEND_P3_DATA(FBSourceReadBufferOffset, (rDest->top << 16) | (rDest->left & 0xFFFF));
    
        // Enable source reads to get the colorkey color
        SEND_P3_DATA(FBSourceReadMode, P3RX_FBSOURCEREAD_READENABLE(__PERMEDIA_ENABLE) |
                                        P3RX_FBSOURCEREAD_LAYOUT(P3RX_LAYOUT_LINEAR));
    }
    else
    {
        SEND_P3_DATA(FBSourceReadMode, P3RX_FBSOURCEREAD_READENABLE(__PERMEDIA_DISABLE));
    }

    SEND_P3_DATA(FBWriteMode, P3RX_FBWRITEMODE_WRITEENABLE(__PERMEDIA_ENABLE) |
                                P3RX_FBWRITEMODE_LAYOUT0(P3RX_LAYOUT_LINEAR));
    SEND_P3_DATA(FBDestReadMode, P3RX_FBDESTREAD_READENABLE(__PERMEDIA_DISABLE) |
                                    P3RX_FBDESTREAD_LAYOUT0(P3RX_LAYOUT_LINEAR));


    // This dest-colorkey download always needs to send unpacked color data
    // because it can't use spans.
    SEND_P3_DATA(DownloadTarget, Color_Tag);
    switch (DDSurf_GetChipPixelSize(pDest))
    {
        case __GLINT_8BITPIXEL:
            dwDownloadTag = Packed8Pixels_Tag;
            break;
        case __GLINT_16BITPIXEL:
            dwDownloadTag = Packed16Pixels_Tag;
            break;
        default:
            dwDownloadTag = Color_Tag;
            break;
    }
    
    rNewDest = *rDest;

    DISPDBG((DBGLVL, "Image download %dx%d", ulSCRoundedUpDWords, ulImageLines));

    if (DDSurf_GetChipPixelSize(pDest) != __GLINT_32BITPIXEL) 
    {
        if (DDSurf_GetChipPixelSize(pDest) == __GLINT_8BITPIXEL) 
        {
            ulSCExtraBytes  = ulSCRoundedUpDWords & 3;
            ulSCWholeDWords = ulSCRoundedUpDWords >> 2;
            ulSCRoundedUpDWords = (ulSCRoundedUpDWords + 3) >> 2;
            
            if (dwDownloadTag != Color_Tag)
            {
                rNewDest.right = rNewDest.left + (ulSCRoundedUpDWords << 2);
            }
        }
        else 
        {
            ulSCExtraBytes  = (ulSCRoundedUpDWords & 1) << 1;
            ulSCWholeDWords = ulSCRoundedUpDWords >> 1;
            ulSCRoundedUpDWords = (ulSCRoundedUpDWords + 1) >> 1;

            if (dwDownloadTag != Color_Tag)
            {
                rNewDest.right = rNewDest.left + (ulSCRoundedUpDWords << 1);
            }
        }
    }
    else
    {
        ulSCExtraBytes = 0;
        ulSCWholeDWords = ulSCRoundedUpDWords;
    }

    // Calc the total number of image DWORDs to send to GPU
    ulTotalImageDWORDs = ulImageLines * ulSCWholeDWords;

    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);

    SEND_P3_DATA(RectanglePosition, 0);


    // Don't use spans for the unpacking scheme, but use the 2D Setup
    // unit to do the work of setting up the destination
    SEND_P3_DATA(ScissorMinXY, 0 )
    SEND_P3_DATA(ScissorMaxXY,P3RX_SCISSOR_X_Y(rDest->right - rDest->left ,
                                               rDest->bottom - rDest->top ));
    SEND_P3_DATA(ScissorMode, P3RX_SCISSORMODE_USER(__PERMEDIA_ENABLE));
    
    renderData =  P3RX_RENDER2D_WIDTH( (rNewDest.right - rNewDest.left) & 0xfff )
                | P3RX_RENDER2D_HEIGHT( 0 )
                | P3RX_RENDER2D_OPERATION( P3RX_RENDER2D_OPERATION_SYNC_ON_HOST_DATA )
                | P3RX_RENDER2D_INCREASINGX( __PERMEDIA_ENABLE )
                | P3RX_RENDER2D_INCREASINGY( __PERMEDIA_ENABLE )
                | P3RX_RENDER2D_FBREADSOURCEENABLE((bDstKey ? __PERMEDIA_ENABLE : __PERMEDIA_DISABLE));
    SEND_P3_DATA(Render2D, renderData);
    SEND_P3_DATA(Count, rNewDest.bottom - rNewDest.top );
    SEND_P3_DATA(Render, P3RX_RENDER_PRIMITIVETYPE(P3RX_RENDER_PRIMITIVETYPE_TRAPEZOID) |
                         P3RX_RENDER_SYNCONHOSTDATA(__PERMEDIA_ENABLE) |
                         P3RX_RENDER_FBSOURCEREADENABLE((bDstKey ? __PERMEDIA_ENABLE : __PERMEDIA_DISABLE)));
    
    _try
    {
        BYTE *pSurfaceData = (BYTE *)pSource->lpGbl->fpVidMem + SrcOffset;
        DWORD *pCurrentLine = (DWORD *)pSurfaceData;
        
        while (ulImageLines-- > 0)
        {
            DISPDBG((DBGLVL, "Image download lines %d", ulImageLines));

            // Initialize the number of DWORDs counter
            ulSCDWordsCnt = ulSCWholeDWords;

            // Send the texels in DWORDS
            while (ulSCDWordsCnt >= BLOCK_SIZE)
            {
                P3_ENSURE_DX_SPACE(BLOCK_SIZE + 1);
                WAIT_FIFO(BLOCK_SIZE + 1);
                ADD_FUNNY_DWORD(TAGGED_SIZE | dwDownloadTag);

                for (count = BLOCK_SIZE / UNROLL_COUNT; count; count--)
                {
                    DISPDBG((DBGLVL, "Image download count %d", count));
                    UNROLLED();
                }
                ulSCDWordsCnt -= BLOCK_SIZE;
            }

            // Finish off the rest of the whole DWORDs on the scanline
            if (ulSCDWordsCnt) 
            {
                P3_ENSURE_DX_SPACE(ulSCDWordsCnt + 1);
                WAIT_FIFO(ulSCDWordsCnt + 1);
                ADD_FUNNY_DWORD(((ulSCDWordsCnt - 1) << 16) | dwDownloadTag);
                for (count = 0; count < ulSCDWordsCnt; count++, pCurrentLine++) 
                {
                    ADD_FUNNY_DWORD(*pCurrentLine);
                    ulTotalImageDWORDs--;
                }
            }

            // Finish off the extra bytes at the end of the scanline
            if (ulSCExtraBytes)
            {
                DWORD dwTemp;

                P3_ENSURE_DX_SPACE(1 + 1);   // 1 tag + 1 data DWORD
                WAIT_FIFO(1 + 1);
                ADD_FUNNY_DWORD(dwDownloadTag);

                memcpy(&dwTemp, pCurrentLine, ulSCExtraBytes);
                ADD_FUNNY_DWORD(dwTemp);
                ulTotalImageDWORDs--;
            }

            pSurfaceData += pSource->lpGbl->lPitch;
            pCurrentLine = (DWORD*)pSurfaceData;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DISPDBG((ERRLVL, "Perm3 caused exception at line %u of file %s",
                         __LINE__,__FILE__));
        // Send enough DWORDs to the GPU to avoid deadlock
        for (count = 0; count < ulTotalImageDWORDs; count++)
        {
            ADD_FUNNY_DWORD(dwDownloadTag);
            ADD_FUNNY_DWORD(0);                 // Dummy pixel data
        }
    }

    P3_ENSURE_DX_SPACE(6);
    WAIT_FIFO(6);

    SEND_P3_DATA(WaitForCompletion, 0);
    SEND_P3_DATA(ScissorMode, __PERMEDIA_DISABLE);
    SEND_P3_DATA(FBSourceReadMode, __PERMEDIA_DISABLE);

    // Put back the values if we changed them.
    rSrc->top = rSrctop;
    rSrc->left = rSrcleft;
    rDest->top = rDesttop;
    rDest->left = rDestleft;

    P3_DMA_COMMIT_BUFFER();

}  //_DD_P3DownloadDstCh 


