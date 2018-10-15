/******************************Module*Header**********************************\
*
*                           *******************
*                           * GDI SAMPLE CODE *
*                           *******************
*
* Module Name: pxrxXfer.c
*
* Content: Bit transfer code
*
* Copyright (c) 1994-1999 3Dlabs Inc. Ltd. All rights reserved.
* Copyright (c) 1995-2002 Microsoft Corporation.  All rights reserved.
\*****************************************************************************/

#include "precomp.h"
#include "ereg.h"
#include "pxrx.h"

#if _DEBUG
static BOOL trapOnMisAlignment = TRUE;

#define  TEST_DWORD_ALIGNED(ptr)                                           \
    do                                                                     \
    {                                                                      \
        ULONG   addr = (ULONG) ptr;                                        \
                                                                           \
        if ( trapOnMisAlignment )                                          \
        {                                                                  \
            ASSERTDD((addr & 3) == 0, "TEST_DWORD_ALIGNED(ptr) failed!");  \
        }                                                                  \
        else if( addr & 3 )                                                \
        {                                                                  \
            DISPDBG((-1, "TEST_DWORD_ALIGNED(0x%08X) is out by %d bytes!", \
                          addr, addr & 3));                                \
        }                                                                  \
    } while(0)
#else
#define  TEST_DWORD_ALIGNED(addr)        do { ; } while(0)
#endif

/******************************************************************************\
 *                                                                            *
 * VOID pxrxXfer1bpp                                                          *
 *                                                                            *
\******************************************************************************/

VOID 
pxrxXfer1bpp( 
    PPDEV           ppdev, 
    RECTL          *prcl, 
    LONG            count, 
    ULONG           fgLogicOp, 
    ULONG           bgLogicOp, 
    SURFOBJ        *psoSrc, 
    POINTL         *pptlSrc, 
    RECTL          *prclDst, 
    XLATEOBJ       *pxlo) 
{
    DWORD           config2D, render2D;
    LONG            cx;
    LONG            cy;
    LONG            lSrcDelta;
    BYTE           *pjSrcScan0;
    BYTE           *pjSrc;
    LONG            dxSrc;
    LONG            dySrc;
    LONG            xLeft;
    LONG            yTop;
    LONG            xOffset;
    ULONG           fgColor;
    ULONG           bgColor;
    RBRUSH_COLOR    rbc;
    GLINT_DECL;

    ASSERTDD(count > 0, "Can't handle zero rectangles");
    ASSERTDD(fgLogicOp <= 15, "Weird fg hardware Rop");
    ASSERTDD(bgLogicOp <= 15, "Weird bg hardware Rop");
    ASSERTDD(pptlSrc != NULL && psoSrc != NULL, "Can't have NULL sources");

    DISPDBG((DBGLVL, "pxrxXfer1bpp: original dstRect: (%d,%d) to (%d,%d)", 
                     prclDst->left, prclDst->top, 
                     prclDst->right, prclDst->bottom));

    dxSrc = pptlSrc->x - prclDst->left;
    dySrc = pptlSrc->y - prclDst->top;    // Add to destination to get source

    lSrcDelta = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;
    
    DISPDBG((DBGLVL, "bitmap baseAddr 0x%x, stride %d, w %d, h %d", 
                     pjSrcScan0, lSrcDelta, 
                     psoSrc->sizlBitmap.cx, psoSrc->sizlBitmap.cy));
    DISPDBG((DBGLVL, "fgColor 0x%x, bgColor 0x%x", 
                     pxlo->pulXlate[1], pxlo->pulXlate[0]));
    DISPDBG((DBGLVL, "fgLogicOp %d, bgLogicOp %d", 
                     fgLogicOp, bgLogicOp));

    fgColor = pxlo->pulXlate[1];
    bgColor = pxlo->pulXlate[0];

    // we get some common operations which are really noops. we can save
    // lots of time by cutting these out. As this happens a lot for masking
    // operations it's worth doing.

    if ( ((fgLogicOp == __GLINT_LOGICOP_AND) && (fgColor == ppdev->ulWhite)) ||
         ((fgLogicOp == __GLINT_LOGICOP_OR ) && (fgColor == 0))              ||
         ((fgLogicOp == __GLINT_LOGICOP_XOR) && (fgColor == 0)) )
    {
        fgLogicOp = __GLINT_LOGICOP_NOOP;
    }

    // same for background
    if ( ((bgLogicOp == __GLINT_LOGICOP_AND) && (bgColor == ppdev->ulWhite)) ||
         ((bgLogicOp == __GLINT_LOGICOP_OR ) && (bgColor == 0))              ||
         ((bgLogicOp == __GLINT_LOGICOP_XOR) && (bgColor == 0)) )
    {
        bgLogicOp = __GLINT_LOGICOP_NOOP;
    }

    if ( (fgLogicOp == __GLINT_LOGICOP_NOOP) && 
         (bgLogicOp == __GLINT_LOGICOP_NOOP) ) 
    {
        DISPDBG((DBGLVL, "both ops are no-op so lets quit now"));
        return;
    }

    config2D = glintInfo->config2D;
    
    config2D &= ~(__CONFIG2D_LOGOP_FORE_ENABLE | 
                  __CONFIG2D_LOGOP_BACK_ENABLE | 
                  __CONFIG2D_ENABLES);
                  
    config2D |= __CONFIG2D_CONSTANTSRC | 
                __CONFIG2D_FBWRITE | 
                __CONFIG2D_USERSCISSOR;
                
    render2D = __RENDER2D_INCX | __RENDER2D_INCY | __RENDER2D_OP_SYNCBITMASK;

    if ( (fgLogicOp != __GLINT_LOGICOP_COPY) || 
         (bgLogicOp != __GLINT_LOGICOP_NOOP) ) 
    {
        config2D &= ~(__CONFIG2D_LOGOP_FORE_MASK | 
                      __CONFIG2D_LOGOP_BACK_MASK);
        config2D |= __CONFIG2D_OPAQUESPANS | 
                    __CONFIG2D_LOGOP_FORE(fgLogicOp) | 
                    __CONFIG2D_LOGOP_BACK(bgLogicOp);
        render2D |= __RENDER2D_SPANS;
    }

    SET_WRITE_BUFFERS;

    WAIT_PXRX_DMA_TAGS( 6 );

    if ( LogicopReadDest[fgLogicOp] || LogicopReadDest[bgLogicOp] ) 
    {
        config2D |= __CONFIG2D_FBDESTREAD;
        SET_READ_BUFFERS;
    }

    if ( LogicOpReadSrc[fgLogicOp] )
    {
        LOAD_FOREGROUNDCOLOUR( fgColor );
    }
    
    if ( LogicOpReadSrc[bgLogicOp] )
    {
        LOAD_BACKGROUNDCOLOUR( bgColor );
    }

    LOAD_CONFIG2D( config2D );

    while( TRUE ) 
    {
        DISPDBG((DBGLVL, "mono download to rect (%d,%d) to (%d,%d)", 
                         prcl->left, prcl->top, prcl->right, prcl->bottom));

        yTop  = prcl->top;
        xLeft = prcl->left;
        cx = prcl->right - xLeft;
        cy = prcl->bottom - yTop;

        // pjSrc is first dword containing a bit to download. xOffset is the
        // offset to that bit. i.e. the bit offset from the previous 32bit
        // boundary at the left hand edge of the rectangle.
        xOffset = (xLeft + dxSrc) & 0x1f;
        pjSrc = (BYTE*)((UINT_PTR)(pjSrcScan0 +
                                   (yTop + dySrc) * lSrcDelta +
                                   (xLeft + dxSrc) / 8  // byte aligned
                                  ) & ~3);              // dword aligned

        DISPDBG((DBGLVL, "pjSrc 0x%x, lSrcDelta %d", pjSrc, lSrcDelta));
        DISPDBG((DBGLVL, "\txOffset %d, cx %d, cy %d", xOffset, cx, cy));

        // this algorithm downloads aligned 32-bit chunks from the
        // source but uses the scissor clip to define the edge of the
        // rectangle.
        //
        {
            ULONG   AlignWidth, LeftEdge;
            AlignWidth = (xOffset + cx + 31) & ~31;
            LeftEdge = xLeft - xOffset;

            DISPDBG((7, "AlignWidth %d", AlignWidth));

            WAIT_PXRX_DMA_DWORDS( 5 );
            QUEUE_PXRX_DMA_INDEX4( __GlintTagFillScissorMinXY, 
                                   __GlintTagFillScissorMaxXY, 
                                   __GlintTagFillRectanglePosition, 
                                   __GlintTagFillRender2D );

            QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(xLeft,             0) );
            QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(prcl->right, 0x7fff) );

            QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(LeftEdge,             yTop) );
            QUEUE_PXRX_DMA_DWORD( render2D | 
                                  __RENDER2D_WIDTH(AlignWidth) | 
                                  __RENDER2D_HEIGHT(cy) );
            SEND_PXRX_DMA_BATCH;

            pxrxMonoDownloadRaw( ppdev, 
                                 AlignWidth, 
                                 (ULONG *) pjSrc, 
                                 lSrcDelta >> 2, 
                                 cy );
        }

        if( --count == 0 )
        {
            break;
        }

        prcl++;
    }

    // Reset the scissor maximums:
    if ( ppdev->cPelSize == GLINTDEPTH32 ) 
    {
        WAIT_PXRX_DMA_TAGS( 1 );
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY, 0x7FFF7FFF );
    }

    FLUSH_PXRX_PATCHED_RENDER2D(prclDst->left, prclDst->right);
    SEND_PXRX_DMA_BATCH;

    DISPDBG((DBGLVL, "pxrxXfer1bpp returning"));
}

/******************************************************************************\
 *                                                                            *
 * void pxrxMonoDownloadRaw                                                   *
 *                                                                            *
\******************************************************************************/

void 
pxrxMonoDownloadRaw( 
    PPDEV   ppdev, 
    ULONG   AlignWidth, 
    ULONG  *pjSrc, 
    LONG    lSrcDelta, 
    LONG    cy) 
{
    GLINT_DECL;

    if ( AlignWidth == 32 ) 
    {
        LONG    nSpaces = 0;
        ULONG   bits;
        DISPDBG((DBGLVL, "Doing Single Word per scan download"));
        do 
        {
            nSpaces = 10;
            WAIT_FREE_PXRX_DMA_DWORDS( nSpaces );
            
            if ( cy < --nSpaces )
            {
                nSpaces = cy;
            }
            
            cy -= nSpaces;

            QUEUE_PXRX_DMA_HOLD( __GlintTagBitMaskPattern, nSpaces );
            
            while( --nSpaces >= 0 ) 
            {
                TEST_DWORD_ALIGNED( pjSrc );
                QUEUE_PXRX_DMA_DWORD( *pjSrc );
                pjSrc += lSrcDelta;
            }
            
            SEND_PXRX_DMA_BATCH;
        } while( cy > 0 );        
    } 
    else 
    {
        // multiple 32 bit words per scanline. convert the delta to the
        // delta as we need it at the end of each line by subtracting the
        // width in bytes of the data we're downloading. Note, pjSrc
        // is always 1 LONG short of the end of the line because we break
        // before adding on the last ULONG. Thus, we subtract sizeof(ULONG)
        // from the original adjustment.
        LONG    nScan = AlignWidth >> 5;
        LONG    nRemainder;
        ULONG   bits;

        DISPDBG((7, "Doing Multiple Word per scan download"));
        while ( TRUE ) 
        {
            WAIT_PXRX_DMA_DWORDS( nScan + 1 );
            QUEUE_PXRX_DMA_HOLD( __GlintTagBitMaskPattern, nScan );
            TEST_DWORD_ALIGNED( pjSrc );
            QUEUE_PXRX_DMA_BUFF( pjSrc, nScan );
            SEND_PXRX_DMA_BATCH;
            pjSrc += lSrcDelta;

            if ( --cy == 0 )
            {
                break;
            }
        }
    }
}


/******************************************************************************\
 *                                                                            *
 * VOID pxrxXfer8bpp                                                          *
 *                                                                            *
\******************************************************************************/
VOID 
pxrxXfer8bpp( 
    PPDEV       ppdev, 
    RECTL      *prcl, 
    LONG        count, 
    ULONG       logicOp, 
    ULONG       bgLogicOp, 
    SURFOBJ    *psoSrc, 
    POINTL     *pptlSrc, 
    RECTL      *prclDst, 
    XLATEOBJ   *pxlo) 
{
    ULONG       config2D, render2D, lutMode, pixelSize;
    BOOL        invalidLUT = FALSE;
    LONG        dx, dy, cy;
    LONG        lSrcDelta, lSrcDeltaDW, lTrueDelta, alignOff;
    ULONG       AlignWidth, LeftEdge;
    BYTE*       pjSrcScan0;
    ULONG*      pjSrc;
    UINT_PTR    startPos;
    LONG        cPelInv;
    ULONG       ul;
    LONG        nRemainder;

    GLINT_DECL;

    DISPDBG((DBGLVL, "pxrxXfer8bpp(): src = (%d,%d) -> (%d,%d), "
                     "count = %d, logicOp = %d, palette id = %d", 
                     prcl->left, prcl->right, prcl->top, prcl->bottom, 
                     count, logicOp, pxlo->iUniq));

    // Set up the LUT table:

    if ( (ppdev->PalLUTType != LUTCACHE_XLATE) || 
         (ppdev->iPalUniq != pxlo->iUniq) ) 
    {
        // Someone has hijacked the LUT so we need to invalidate it:
        ppdev->PalLUTType = LUTCACHE_XLATE;
        ppdev->iPalUniq = pxlo->iUniq;
        invalidLUT = TRUE;
    } 
    else 
    {
        DISPDBG((DBGLVL, "pxrxXfer8bpp: reusing cached xlate"));
    }

    WAIT_PXRX_DMA_TAGS( 1 + 1 );

    lutMode = glintInfo->lutMode & ~((3 << 2) | (1 << 4) | (7 << 8));
    lutMode |= (ppdev->cPelSize + 2) << 8;
    LOAD_LUTMODE( lutMode );

    if ( invalidLUT ) 
    {
        ULONG   *pulXlate = pxlo->pulXlate;
        LONG    cEntries = 256;

        QUEUE_PXRX_DMA_TAG( __PXRXTagLUTIndex, 0 );

        if ( ppdev->cPelSize == 0 ) 
        {
            // 8bpp
            WAIT_PXRX_DMA_TAGS( cEntries );

            do 
            {
                ul = *(pulXlate++);
                ul |= ul << 8;
                ul |= ul << 16;
                QUEUE_PXRX_DMA_TAG( __PXRXTagLUTData, ul );
            } while( --cEntries );
        } 
        else if ( ppdev->cPelSize == 1 ) 
        {    
            // 16bpp
            WAIT_PXRX_DMA_TAGS( cEntries );

            do 
            {
                ul = *(pulXlate++);
                ul |= ul << 16;
                QUEUE_PXRX_DMA_TAG( __PXRXTagLUTData, ul );
            } while( --cEntries );
        } 
        else 
        {
            WAIT_PXRX_DMA_DWORDS( 1 + cEntries );

            QUEUE_PXRX_DMA_HOLD( __PXRXTagLUTData, cEntries );
            QUEUE_PXRX_DMA_BUFF( pulXlate, cEntries );
        }
    }

    config2D = __CONFIG2D_FBWRITE     | 
               __CONFIG2D_USERSCISSOR | 
               __CONFIG2D_EXTERNALSRC | 
               __CONFIG2D_LUTENABLE;
               
    render2D = __RENDER2D_INCX        |  
               __RENDER2D_INCY        | 
               __RENDER2D_OP_SYNCDATA | 
               __RENDER2D_SPANS;

    SET_WRITE_BUFFERS;

    WAIT_PXRX_DMA_TAGS( 6 );

    if ( logicOp != __GLINT_LOGICOP_COPY ) 
    {
        config2D |= __CONFIG2D_LOGOP_FORE(logicOp) | __CONFIG2D_FBWRITE;
        render2D |= __RENDER2D_SPANS;

        if ( LogicopReadDest[logicOp] ) 
        {
            config2D |= __CONFIG2D_FBDESTREAD;
            SET_READ_BUFFERS;
        }
    }

    LOAD_CONFIG2D( config2D );

    cPelInv = 2 - ppdev->cPelSize;
    pixelSize = (1 << 31)       | // Everything before the LUT runs at 8bpp
                (2 << 2)        | 
                (2 << 4)        | 
                (2 << 6)        |        
                (cPelInv << 8)  | 
                (cPelInv << 10) | 
                (cPelInv << 12) | 
                (cPelInv << 14);
                
    QUEUE_PXRX_DMA_TAG( __GlintTagPixelSize, pixelSize );

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;     // Add to destination to get source

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    while( TRUE ) 
    {
        DISPDBG((DBGLVL, "download to rect (%d,%d) to (%d,%d)", 
                         prcl->left, prcl->top, 
                         prcl->right, prcl->bottom));

        // 8bpp => 1 pixel per byte => 4 pixels per dword

        // Assume source bitmap width is dword aligned
        ASSERTDD((lSrcDelta & 3) == 0, 
                 "pxrxXfer8bpp: SOURCE BITMAP WIDTH IS NOT DWORD ALIGNED!!!");

        startPos = (((UINT_PTR) pjSrcScan0) + ((prcl->top + dy) * lSrcDelta)) 
                        + (prcl->left + dx);    // pointer to first pixel, 
                                                // in pixels/bytes
        pjSrc    = (ULONG *) (startPos & ~3);   // dword pointer to dword 
                                                // aligned first pixel           
        
        if ( pjSrc == NULL )
        {
            DISPDBG((ERRLVL, "ERROR: pxrxXfer8bpp return ,has pjSrc NULL"));
            return;
        }
        
        alignOff = (ULONG)(startPos & 3); // number of pixels past dword 
                                          // alignment of a scanline
        LeftEdge = prcl->left - alignOff; // dword aligned left edge in pixels
        AlignWidth = ((prcl->right - LeftEdge) + 3) & ~3; // dword aligned width 
                                                          // in pixels
        cy = prcl->bottom - prcl->top;    // number of scanlines to do

        DISPDBG((DBGLVL, "pjSrcScan0 = 0x%08X, "
                         "startPos = 0x%08X, pjSrc = 0x%08X", 
                         pjSrcScan0, startPos, pjSrc));
        DISPDBG((DBGLVL, "offset = %d pixels", alignOff));
        DISPDBG((DBGLVL, "Aligned rect = (%d -> %d) => %d pixels => %d dwords", 
                         LeftEdge, LeftEdge + AlignWidth, 
                         AlignWidth, AlignWidth >> 2));

        WAIT_PXRX_DMA_TAGS( 4 );

        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMinXY,         
                                    MAKEDWORD_XY(prcl->left,       0) );
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY,         
                                    MAKEDWORD_XY(prcl->right, 0x7fff) );
        QUEUE_PXRX_DMA_TAG( __GlintTagRectanglePosition,    
                                    MAKEDWORD_XY(LeftEdge, prcl->top) );
        QUEUE_PXRX_DMA_TAG( __GlintTagRender2D,             
                                    render2D                     | 
                                    __RENDER2D_WIDTH(AlignWidth) | 
                                    __RENDER2D_HEIGHT(cy) );
        SEND_PXRX_DMA_BATCH;

        AlignWidth  >>= 2;            // dword aligned width in dwords
        lSrcDeltaDW = lSrcDelta >> 2; // scanline delta in dwords 
                                      // (start to start)
        lTrueDelta  = lSrcDeltaDW - AlignWidth; // scanline delta in dwords 
                                                // (end   to start)
        DISPDBG((DBGLVL, "Delta = %d bytes = %d dwords -> %d - %d dwords", 
                         lSrcDelta, lSrcDeltaDW, lTrueDelta, AlignWidth));

        // Do a raw download:
        while ( TRUE ) 
        {
            DISPDBG((DBGLVL, "cy = %d", cy));

            WAIT_PXRX_DMA_DWORDS( AlignWidth + 1 );
            QUEUE_PXRX_DMA_HOLD( __GlintTagColor, AlignWidth );
            TEST_DWORD_ALIGNED( pjSrc );
            QUEUE_PXRX_DMA_BUFF( pjSrc, AlignWidth );
            SEND_PXRX_DMA_BATCH;

            if ( --cy == 0 )
            {
                break;
            }

            pjSrc += lSrcDeltaDW;
        }

        if ( --count == 0 )
        {
            break;
        }

        prcl++;
    }

    // Reset some defaults:
    WAIT_PXRX_DMA_TAGS( 2 );
    QUEUE_PXRX_DMA_TAG( __GlintTagPixelSize, cPelInv );
    if ( ppdev->cPelSize == GLINTDEPTH32 )
    {
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY, 0x7FFF7FFF );
    }

    SEND_PXRX_DMA_BATCH;

    DISPDBG((DBGLVL, "pxrxXfer8bpp return"));
}

/******************************************************************************\
 *                                                                            *
 * VOID pxrxXferImage                                                         *
 *                                                                            *
\******************************************************************************/

VOID 
pxrxXferImage( 
    PPDEV       ppdev, 
    RECTL      *prcl, 
    LONG        count, 
    ULONG       logicOp, 
    ULONG       bgLogicOp, 
    SURFOBJ    *psoSrc, 
    POINTL     *pptlSrc, 
    RECTL      *prclDst, 
    XLATEOBJ   *pxlo) 
{
    DWORD       config2D, render2D;
    LONG        dx, dy, cy;
    LONG        lSrcDelta, lTrueDelta, lSrcDeltaDW, alignOff;
    BYTE*       pjSrcScan0;
    ULONG*      pjSrc;
    UINT_PTR    startPos;
    LONG        cPel, cPelInv;
    ULONG       cPelMask;
    ULONG       AlignWidth, LeftEdge;
    LONG        nRemainder;

    GLINT_DECL;

    SEND_PXRX_DMA_FORCE;

    ASSERTDD((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL), 
             "Can handle trivial xlate only");
    ASSERTDD(psoSrc->iBitmapFormat == ppdev->iBitmapFormat, 
             "Source must be same colour depth as screen");
    ASSERTDD(count > 0, 
             "Can't handle zero rectangles");
    ASSERTDD(logicOp <= 15, 
             "Weird hardware Rop");

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top; // Add to destination to get source
    cPel = ppdev->cPelSize;         // number of bytes per pixel = 1 << cPel
    cPelInv = 2 - cPel;             // number of pixels per byte = 1 << cPelInv
                                    // (pixels -> dwords = >> cPenInv)
    cPelMask = (1 << cPelInv) - 1;  // mask to obtain number of pixels 
                                    // past a dword

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    DISPDBG((DBGLVL, "pxrxXferImage with logic op %d for %d rects", 
                     logicOp, count));

    config2D = glintInfo->config2D & ~(__CONFIG2D_LOGOP_FORE_ENABLE | 
                                       __CONFIG2D_LOGOP_BACK_ENABLE | 
                                       __CONFIG2D_ENABLES);
    config2D |= __CONFIG2D_FBWRITE    | 
                __CONFIG2D_USERSCISSOR;
    render2D = __RENDER2D_INCX        | 
               __RENDER2D_INCY        | 
               __RENDER2D_OP_SYNCDATA | 
               __RENDER2D_SPANS;
    
    SET_WRITE_BUFFERS;

    WAIT_PXRX_DMA_TAGS( 5 );

    if ( logicOp != __GLINT_LOGICOP_COPY ) 
    {
        config2D &= ~__CONFIG2D_LOGOP_FORE_MASK;
        config2D |= __CONFIG2D_LOGOP_FORE(logicOp) | 
                    __CONFIG2D_EXTERNALSRC;

        if ( LogicopReadDest[logicOp] ) 
        {
            config2D |= __CONFIG2D_FBDESTREAD;
            SET_READ_BUFFERS;
        }
    }

    LOAD_CONFIG2D( config2D );

    while ( TRUE ) 
    {
        cy = prcl->bottom - prcl->top;

        DISPDBG((DBGLVL, "download to rect (%d,%d) to (%d,%d)", 
                         prcl->left, prcl->top, prcl->right, prcl->bottom));

        ASSERTDD((lSrcDelta & 3) == 0, 
                 "pxrxXferImage: SOURCE BITMAP WIDTH IS NOT DWORD ALIGNED!!!");

        // pjSrc points to the first pixel to copy
        // lTrueDelta is the additional amount to add onto the pjSrc pointer
        //    when we get to the end of the scanline.
        startPos = ((UINT_PTR) pjSrcScan0) + ((prcl->top + dy) * lSrcDelta) + 
                                                  ((prcl->left + dx) << cPel);
        alignOff = ((ULONG) (startPos & 3)) >> cPel;  // number of pixels past
                                                      // dword aligned start
        pjSrc = (ULONG *) (startPos & ~3); // dword aligned pointer to 1st pixel
        
        if ( pjSrc == NULL )
        {
            DISPDBG((ERRLVL, "ERROR: "
                             "pxrxXferImage return because of pjSrc NULL"));
            return;
        }        
        
        // dword aligned left edge in pixels
        LeftEdge    = prcl->left - alignOff;                            
        // dword aligned width in pixels
        AlignWidth  = (prcl->right - LeftEdge + cPelMask) & ~cPelMask;    
        
        DISPDBG((DBGLVL, "Aligned rect = (%d -> %d) => %d pixels", 
                         LeftEdge, LeftEdge + AlignWidth, AlignWidth));
        DISPDBG((DBGLVL, "pjSrcScan0 = 0x%08X, "
                         "pjSrc = 0x%08X, alignOff = %d pixels", 
                         pjSrcScan0, pjSrc, alignOff));

        ASSERTDD( ((UINT_PTR) pjSrcScan0) + ((prcl->top + dy) * lSrcDelta) + 
                              ((LeftEdge + dx) << cPel) == (UINT_PTR) pjSrc,
                  "pxrxXferImage: "
                  "Aligned left edge does not match aligned pjSrc!" );

        WAIT_PXRX_DMA_DWORDS( 5 );

        QUEUE_PXRX_DMA_INDEX4( __GlintTagFillScissorMinXY, 
                               __GlintTagFillScissorMaxXY, 
                               __GlintTagFillRectanglePosition, 
                               __GlintTagFillRender2D );
                               
        QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(prcl->left, 0) );
        QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(prcl->right, 0x7fff) );
        QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(LeftEdge, prcl->top) );
        QUEUE_PXRX_DMA_DWORD( render2D | 
                              __RENDER2D_WIDTH(AlignWidth) | 
                              __RENDER2D_HEIGHT(cy) );
        SEND_PXRX_DMA_BATCH;

        AlignWidth >>= cPelInv;                 // dword aligned width in dwords
        lSrcDeltaDW = lSrcDelta >> 2;           // scanline delta in dwords 
                                                //(start to start)
        lTrueDelta  = lSrcDeltaDW - AlignWidth; // scanline delta in dwords 
                                                // (end   to start)
                                                
        DISPDBG((DBGLVL, "Delta = %d bytes = %d dwords -> %d - %d dwords", 
                         lSrcDelta, lSrcDeltaDW, lTrueDelta, AlignWidth));

        // Do a raw download:
        while ( TRUE ) 
        {
            DISPDBG((DBGLVL, "cy = %d", cy));

            WAIT_PXRX_DMA_DWORDS( AlignWidth + 1 );
            QUEUE_PXRX_DMA_HOLD( __GlintTagColor, AlignWidth );
            TEST_DWORD_ALIGNED( pjSrc );
            QUEUE_PXRX_DMA_BUFF( pjSrc, AlignWidth );
//          SEND_PXRX_DMA_BATCH;

            if ( --cy == 0 )
            {
                break;
            }
            pjSrc += lSrcDeltaDW;
        }

        if ( --count == 0 )
        {
            break;
        }

        prcl++;
    }

    // Reset the scissor maximums:
    if ( ppdev->cPelSize == GLINTDEPTH32 ) 
    {
        WAIT_PXRX_DMA_TAGS( 1 );
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY, 0x7FFF7FFF );
//      SEND_PXRX_DMA_BATCH;
    }
    FLUSH_PXRX_PATCHED_RENDER2D(prclDst->left, prclDst->right);
    SEND_PXRX_DMA_BATCH;

    DISPDBG((DBGLVL, "pxrxXferImage return"));
}

/******************************************************************************\
 *                                                                            *
 * VOID pxrxXfer4bpp                                                          *
 *                                                                            *
\******************************************************************************/

VOID 
pxrxXfer4bpp( 
    PPDEV       ppdev, 
    RECTL      *prcl, 
    LONG        count, 
    ULONG       logicOp, 
    ULONG       bgLogicOp, 
    SURFOBJ    *psoSrc, 
    POINTL     *pptlSrc, 
    RECTL      *prclDst, 
    XLATEOBJ   *pxlo) 
{
    ULONG       config2D, render2D, lutMode, pixelSize;
    BOOL        invalidLUT = FALSE;
    LONG        dx, dy;
    LONG        cy;
    BYTE*       pjSrcScan0;
    ULONG*      pjSrc;
    LONG        cPelInv;
    ULONG       ul;
    ULONG       AlignWidth, LeftEdge;
    UINT_PTR    startPos;
    LONG        nRemainder;
    LONG        lSrcDelta, lSrcDeltaDW;
    LONG        alignOff;
    GLINT_DECL;

    DISPDBG((DBGLVL, "pxrxXfer4bpp(): src = (%d,%d) -> (%d,%d), count = %d, "
                     "logicOp = %d, palette id = %d", 
                     prcl->left, prcl->right, prcl->top, prcl->bottom, count, 
                     logicOp, pxlo->iUniq));

    // Set up the LUT table:
    if ( (ppdev->PalLUTType != LUTCACHE_XLATE) || 
         (ppdev->iPalUniq != pxlo->iUniq) ) 
    {
        // Someone has hijacked the LUT so we need to invalidate it:
        ppdev->PalLUTType = LUTCACHE_XLATE;
        ppdev->iPalUniq = pxlo->iUniq;
        invalidLUT = TRUE;
    } 
    else 
    {
        DISPDBG((DBGLVL, "pxrxXfer4bpp: reusing cached xlate"));
    }

    WAIT_PXRX_DMA_TAGS( 1 + 1 + 16 );

    lutMode = glintInfo->lutMode & ~((3 << 2) | (1 << 4) | (7 << 8));
    lutMode |= (ppdev->cPelSize + 2) << 8;
    LOAD_LUTMODE( lutMode );

    if ( invalidLUT ) 
    {
        ULONG   *pulXlate = pxlo->pulXlate;
        LONG    cEntries = 16;

        QUEUE_PXRX_DMA_TAG( __PXRXTagLUTIndex, 0 );

        if ( ppdev->cPelSize == 0 )    // 8bpp
        {
            do 
            {
                ul = *(pulXlate++);
                ul |= ul << 8;
                ul |= ul << 16;
                QUEUE_PXRX_DMA_TAG( __PXRXTagLUTData, ul );
            } while ( --cEntries );
        }
        else if( ppdev->cPelSize == 1 )    // 16bpp
        {
            do 
            {
                ul = *(pulXlate++);
                ul |= ul << 16;
                QUEUE_PXRX_DMA_TAG( __PXRXTagLUTData, ul );
            } while( --cEntries );
        }
        else 
        {
            QUEUE_PXRX_DMA_HOLD( __PXRXTagLUTData, cEntries );
            QUEUE_PXRX_DMA_BUFF( pulXlate, cEntries );
        }
    }

    config2D = glintInfo->config2D & ~(__CONFIG2D_LOGOP_FORE_ENABLE | 
                                       __CONFIG2D_LOGOP_BACK_ENABLE | 
                                       __CONFIG2D_ENABLES);
                                       
    config2D |= __CONFIG2D_FBWRITE    | 
                __CONFIG2D_USERSCISSOR;
                
    render2D = __RENDER2D_INCX        | 
               __RENDER2D_INCY        | 
               __RENDER2D_OP_SYNCDATA | 
               __RENDER2D_SPANS;

    SET_WRITE_BUFFERS;

    WAIT_PXRX_DMA_TAGS( 6 );

    if ( logicOp != __GLINT_LOGICOP_COPY ) 
    {
        config2D &= ~(__CONFIG2D_LOGOP_FORE_MASK | 
                      __CONFIG2D_LOGOP_BACK_MASK);
        config2D |= __CONFIG2D_LOGOP_FORE(logicOp) | 
                    __CONFIG2D_FBWRITE;
                    
        render2D |= __RENDER2D_SPANS;

        if ( LogicopReadDest[logicOp] ) 
        {
            config2D |= __CONFIG2D_FBDESTREAD;
            SET_READ_BUFFERS;
        }

        if ( LogicOpReadSrc[logicOp] ) 
        {
            config2D |= __CONFIG2D_EXTERNALSRC | 
                        __CONFIG2D_LUTENABLE;
        }
    } 
    else 
    {
        config2D |= __CONFIG2D_EXTERNALSRC | 
                    __CONFIG2D_LUTENABLE;
    }

    LOAD_CONFIG2D( config2D );

    QUEUE_PXRX_DMA_TAG( __GlintTagDownloadTarget, __GlintTagColor );
    cPelInv = 2 - ppdev->cPelSize;
    // Everything before the LUT runs at 8bpp
    pixelSize = (1 << 31)       | 
                (2 << 2)        | 
                (2 << 4)        | 
                (2 << 6)        | 
                (2 << 16)       |        
                (cPelInv << 8)  | 
                (cPelInv << 10) | 
                (cPelInv << 12) | 
                (cPelInv << 14);
                
    QUEUE_PXRX_DMA_TAG( __GlintTagPixelSize, pixelSize );

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;  // Add to destination to get source
//  cPel = ppdev->cPelSize;
//  cPelMask = (1 << cPelInv) - 1;

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    while ( TRUE ) 
    {
        DISPDBG((DBGLVL, "download to rect (%d,%d) to (%d,%d)", 
                         prcl->left, prcl->top, prcl->right, prcl->bottom));

        // 4bpp => 2 pixels per byte => 8 pixels per dword

        // Assume source bitmap width is dword aligned
        ASSERTDD( (lSrcDelta & 3) == 0, 
                  "pxrxXfer4bpp: SOURCE BITMAP WIDTH IS NOT DWORD ALIGNED!!!");

        // pointer to first pixel, in bytes (32/64 bits long)
        startPos = (((UINT_PTR) pjSrcScan0) + ((prcl->top + dy) * lSrcDelta)) + 
                                                       ((prcl->left + dx) >> 1);    
        pjSrc = (ULONG *) (startPos & ~3); // dword pointer to dword 
                                           // aligned first pixel

        if (NULL == pjSrc)
        {
            DISPDBG((ERRLVL, "ERROR: "
                             "pxrxXfer4bpp return because of pjSrc NULL"));            
            return;
        }

        // pointer to first pixel, in pixels (33/65 bits long!)
        startPos = (( ((UINT_PTR) pjSrcScan0) + 
                      ((prcl->top + dy) * lSrcDelta)) << 1) 
                   + (prcl->left + dx);    
                   
        alignOff = (ULONG)(startPos & 7); // number of pixels past dword 
                                          // alignment of a scanline

        LeftEdge = prcl->left - alignOff; // dword aligned left edge in pixels
        // dword aligned width in pixels
        AlignWidth  = ((prcl->right - LeftEdge) + 7) & ~7;    
        cy          = prcl->bottom - prcl->top; // number of scanlines to do

        DISPDBG((DBGLVL, "pjSrcScan0 = 0x%08X, startPos = 0x%08X (>>1), "
                         "pjSrc = 0x%08X", 
                         pjSrcScan0, startPos >> 1, pjSrc));
        DISPDBG((DBGLVL, "offset = %d pixels", alignOff));
        DISPDBG((DBGLVL, "Aligned rect = (%d -> %d) => %d pixels => %d dwords", 
                         LeftEdge, LeftEdge + AlignWidth, 
                         AlignWidth, AlignWidth >> 3));

        WAIT_PXRX_DMA_TAGS( 4 );

        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMinXY,         
                                        MAKEDWORD_XY(prcl->left,       0) );
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY,         
                                        MAKEDWORD_XY(prcl->right, 0x7fff) );
        QUEUE_PXRX_DMA_TAG( __GlintTagRectanglePosition,    
                                        MAKEDWORD_XY(LeftEdge, prcl->top) );
        QUEUE_PXRX_DMA_TAG( __GlintTagRender2D,  render2D | 
                                                 __RENDER2D_WIDTH(AlignWidth) |
                                                 __RENDER2D_HEIGHT(cy) );
        SEND_PXRX_DMA_BATCH;

        AlignWidth  >>= 3;            // dword aligned width in dwords
        lSrcDeltaDW = lSrcDelta >> 2; // dword aligned scanline offset in dwords
        
        DISPDBG((DBGLVL, "Delta = %d pixels = %d dwords", 
                         lSrcDelta << 1, lSrcDeltaDW));

        //    pjSrc       = dword aligned pointer to first 
        //                         dword of first scanline
        //    AlignWidth  = number of dwords per scanline
        //    lTrueDelta  = dword offset between first dwords 
        //                         of consecutive scanlines
        //    cy          = number of scanlines

        while ( TRUE ) 
        {
            nRemainder = AlignWidth;

            DISPDBG((DBGLVL, "cy = %d", cy));

            WAIT_PXRX_DMA_DWORDS( AlignWidth + 1 );
            QUEUE_PXRX_DMA_HOLD( __GlintTagPacked4Pixels, AlignWidth );
            TEST_DWORD_ALIGNED( pjSrc );
            QUEUE_PXRX_DMA_BUFF( pjSrc, AlignWidth );

            if ( --cy == 0 )
            {
                break;
            }
            pjSrc += lSrcDeltaDW;
            SEND_PXRX_DMA_BATCH;
        }

        if ( --count == 0 )
        {
            break;
        }

        prcl++;
    }

    // Reset some defaults:
    WAIT_PXRX_DMA_TAGS( 2 );
    QUEUE_PXRX_DMA_TAG( __GlintTagPixelSize, cPelInv );
    if ( ppdev->cPelSize == GLINTDEPTH32 )
    {
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY, 0x7FFF7FFF );
    }

    SEND_PXRX_DMA_BATCH;

    DISPDBG((DBGLVL, "pxrxXfer4bpp return"));
}

/******************************************************************************\
 *                                                                            *
 * VOID pxrxCopyXfer24bpp                                                     *
 *                                                                            *
\******************************************************************************/      

VOID 
pxrxCopyXfer24bpp( 
    PPDEV       ppdev, 
    SURFOBJ    *psoSrc, 
    POINTL     *pptlSrc, 
    RECTL      *prclDst, 
    RECTL      *prcl, 
    LONG        count) 
{
    ULONG       config2D, render2D, pixelSize;
    LONG        dx, dy, cy, LeftEdge;
    LONG        lSrcDelta, lSrcDeltaDW, lTrueDelta, alignOff;
    UINT_PTR    startPos;
    BYTE*       pjSrcScan0;
    ULONG*      pjSrc;
    LONG        cPelInv;
    ULONG       ul, nRemainder;
    ULONG       padLeft, padLeftDW, padRight, padRightDW, dataWidth;
    ULONG       AlignWidth, AlignWidthDW, AlignExtra;
    GLINT_DECL;

    DISPDBG((DBGLVL, "pxrxCopyXfer24bpp(): "
                     "src = (%d,%d) -> (%d,%d), count = %d", 
                     prcl->left, prcl->right, prcl->top, prcl->bottom, count));

    config2D = glintInfo->config2D & ~(__CONFIG2D_LOGOP_FORE_ENABLE | 
                                       __CONFIG2D_LOGOP_BACK_ENABLE | 
                                       __CONFIG2D_ENABLES);
    config2D |= __CONFIG2D_FBWRITE     | 
                __CONFIG2D_EXTERNALSRC | 
                __CONFIG2D_USERSCISSOR;
                
    render2D = __RENDER2D_INCX        | 
               __RENDER2D_INCY        | 
               __RENDER2D_OP_SYNCDATA | 
               __RENDER2D_SPANS;

    SET_WRITE_BUFFERS;

    WAIT_PXRX_DMA_TAGS( 3 );

    QUEUE_PXRX_DMA_TAG( __GlintTagDownloadTarget,       __GlintTagColor );
    QUEUE_PXRX_DMA_TAG( __GlintTagDownloadGlyphWidth,   3 );
    LOAD_CONFIG2D( config2D );

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;     // Add to destination to get source

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    while ( TRUE ) 
    {
        DISPDBG((DBGLVL, "download to rect "
                         "(%d,%d -> %d,%d) + (%d, %d) = (%d x %d)", 
                         prcl->left, prcl->top, prcl->right, prcl->bottom,
                         dx, dy, 
                         prcl->right - prcl->left, 
                         prcl->bottom - prcl->top));

        // 24bpp => 1 pixel per 3 bytes => 4 pixel per 3 dwords

        // Assume source bitmap width is dword aligned
        ASSERTDD( (lSrcDelta & 3) == 0, 
                  "pxrxCopyXfer24bpp: "
                  "SOURCE BITMAP WIDTH IS NOT DWORD ALIGNED!!!" );
        ASSERTDD( (((UINT_PTR) pjSrcScan0) & 3) == 0, 
                  "pxrxCopyXfer24bpp: "
                  "SOURCE BITMAP START LOCATION IS NOT DWORD ALIGNED!!!" );

        cy = prcl->bottom - prcl->top;  // number of scanlines to do
        startPos = (((UINT_PTR) pjSrcScan0) + 
                     ((prcl->top + dy) * lSrcDelta)) + 
                   ((prcl->left + dx) * 3); // pointer to first pixel of first 
                                            // scanline, in bytes
                                            
        alignOff = (ULONG)(startPos & 3);    // number of bytes past dword 
                                             // alignment to first pixel
        pjSrc = (ULONG *) (startPos & ~3);   // dword pointer to dword aligned 
                                             // first pixel

        if ( pjSrc == NULL )
        {
            DISPDBG((ERRLVL, "ERROR: "
                             "pxrxCopyXfer24bpp return because of pjSrc NULL"));            
            return;
        }
        
        padLeft = (4 - alignOff) % 4;   // number of pixels to add to regain 
                                        // dword alignment on left edge
        padLeftDW = (padLeft * 3) / 4;  // number of dwords to add 
                                        // on the left edge
        LeftEdge = prcl->left - padLeft;

        // dword aligned width in pixels (= 4 pixel aligned = 3 dword aligned!)
        AlignWidth = (prcl->right - LeftEdge + 3) & ~3;        
        // number of pixels overhang on the right
        padRight = (LeftEdge + AlignWidth) - prcl->right;    
        // number of dwords to add on the right edge
        padRightDW = (padRight * 3) / 4;                        

        AlignWidthDW = (AlignWidth * 3) / 4; // dword aligned width in dwords
        lSrcDeltaDW = lSrcDelta >> 2;        // dword aligned scanline offset 
                                             //                    in dwords
        // the amount of AlignWidth which is actually src bitmap                                             
        dataWidth = AlignWidthDW - padLeftDW - padRightDW;    

        DISPDBG((DBGLVL, "startPos = 0x%08X, alignOff = %d, "
                         "pjSrc = 0x%08X, lSrcDeltaDW = %d", 
                         startPos, alignOff, pjSrc, lSrcDeltaDW));
        DISPDBG((DBGLVL, "padLeft = %d pixels = %d dwords, LeftEdge = %d", 
                         padLeft, padLeftDW, LeftEdge));
        DISPDBG((DBGLVL, "AlignWidth = %d pixels = %d dwords", 
                         AlignWidth, AlignWidthDW));
        DISPDBG((DBGLVL, "padRight = %d pixels = %d dwords", padRight, padRightDW));

        WAIT_PXRX_DMA_TAGS( 4 );

        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMinXY,         
                                            MAKEDWORD_XY(prcl->left,       0));
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY,         
                                            MAKEDWORD_XY(prcl->right, 0x7fff));
        QUEUE_PXRX_DMA_TAG( __GlintTagRectanglePosition,    
                                            MAKEDWORD_XY(LeftEdge, prcl->top));
        QUEUE_PXRX_DMA_TAG( __GlintTagRender2D,  render2D                     | 
                                                 __RENDER2D_WIDTH(AlignWidth) | 
                                                 __RENDER2D_HEIGHT(cy) );

        while ( cy-- ) 
        {
            DISPDBG((DBGLVL, "cy = %d", cy));

            WAIT_PXRX_DMA_DWORDS( AlignWidthDW + 1 );
            QUEUE_PXRX_DMA_HOLD( __GlintTagGlyphData, AlignWidthDW );

            if( padLeftDW )
            {
                QUEUE_PXRX_DMA_DWORD( 0xDEADDEAD );
            }
            
            if( padLeftDW == 2 )
            {
                QUEUE_PXRX_DMA_DWORD( 0xDEADDEAD );
            }

            QUEUE_PXRX_DMA_BUFF( pjSrc, dataWidth );

            if( padRightDW )
            {
                QUEUE_PXRX_DMA_DWORD( 0xDEADDEAD );
            }
            
            if( padRightDW == 2 )
            {
                QUEUE_PXRX_DMA_DWORD( 0xDEADDEAD );
            }

            SEND_PXRX_DMA_BATCH;

            pjSrc += lSrcDeltaDW;
        }
        
        if ( --count == 0 )
        {
            break;
        }

        prcl++;
    }

    // Reset the scissor maximums:
    if( ppdev->cPelSize == GLINTDEPTH32 ) 
    {
        WAIT_PXRX_DMA_TAGS( 1 );
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY, 0x7FFF7FFF );
    }

    SEND_PXRX_DMA_BATCH;

    DISPDBG((DBGLVL, "pxrxCopyXfer24bpp return"));
}

/******************************************************************************\
 *                                                                            *
 * VOID pxrxCopyXfer8bppLge                                                   *
 *                                                                            *
\******************************************************************************/

VOID 
pxrxCopyXfer8bppLge( 
    PPDEV       ppdev, 
    SURFOBJ    *psoSrc, 
    POINTL     *pptlSrc, 
    RECTL      *prclDst, 
    RECTL      *prcl, 
    LONG        count, 
    XLATEOBJ   *pxlo ) 
{
    ULONG       config2D, render2D, lutMode, pixelSize;
    BOOL        invalidLUT = FALSE;
    LONG        dx, dy, cy;
    LONG        lSrcDelta, lSrcDeltaDW, lTrueDelta, alignOff;
    ULONG       AlignWidth, LeftEdge;
    BYTE*       pjSrcScan0;
    ULONG*      pjSrc;
    UINT_PTR    startPos;
    LONG        cPelInv;
    ULONG       ul, i;
    LONG        nRemainder;
    ULONG      *tagPtr;
    ULONG      *pulXlate = pxlo->pulXlate;
    GLINT_DECL;

    DISPDBG((DBGLVL, "pxrxCopyXfer8bpp(): src = (%d,%d) -> (%d,%d), "
                     "count = %d, palette id = %d", 
                     prcl->left, prcl->right, prcl->top, prcl->bottom, 
                     count, pxlo->iUniq));

    SET_WRITE_BUFFERS;

    if ( (count == 1) && 
         ((cy = (prcl->bottom - prcl->top)) == 1) ) 
    {
        ULONG   width = prcl->right - prcl->left, extra;
        BYTE    *srcPtr;

        config2D = __CONFIG2D_FBWRITE    | 
                   __CONFIG2D_EXTERNALSRC;
        render2D = __RENDER2D_INCX        | 
                   __RENDER2D_INCY        | 
                   __RENDER2D_OP_SYNCDATA | 
                   __RENDER2D_SPANS;

        dx = pptlSrc->x - prclDst->left;
        dy = pptlSrc->y - prclDst->top;  // Add to destination to get source

        lSrcDelta  = psoSrc->lDelta;
        pjSrcScan0 = psoSrc->pvScan0;
        startPos = (((UINT_PTR) pjSrcScan0) + 
                     ((prcl->top + dy) * lSrcDelta)) + (prcl->left + dx);
        srcPtr = (BYTE *) startPos;

        WAIT_PXRX_DMA_DWORDS( 7 + width );

        LOAD_CONFIG2D( config2D );

        QUEUE_PXRX_DMA_TAG( __GlintTagRectanglePosition,    
                                        MAKEDWORD_XY(prcl->left, prcl->top) );
        QUEUE_PXRX_DMA_TAG( __GlintTagRender2D,  render2D | 
                                                 __RENDER2D_WIDTH(width) | 
                                                 __RENDER2D_HEIGHT(1) );

        if ( ppdev->cPelSize == 0 )     // 8bpp
        {
            extra = width & 3;
            width >>= 2;

            if ( extra ) 
            {
                QUEUE_PXRX_DMA_HOLD( __GlintTagColor, width + 1 );
                QUEUE_PXRX_DMA_BUFF_DELAYED( tagPtr, width + 1 );
            } 
            else 
            {
                QUEUE_PXRX_DMA_HOLD( __GlintTagColor, width );
                QUEUE_PXRX_DMA_BUFF_DELAYED( tagPtr, width );
            }

            DISPDBG((DBGLVL, "width was %d, is now %d + %d", 
                             prcl->right - prcl->left, width, extra));

            for ( i = 0; i < width; i++, srcPtr += 4 )
            {
                *(tagPtr++) = (pulXlate[srcPtr[3]] << 24) | 
                              (pulXlate[srcPtr[2]] << 16) | 
                              (pulXlate[srcPtr[1]] <<  8) | 
                               pulXlate[srcPtr[0]];
            }

            if ( extra == 1 )
            {
                *(tagPtr++) = pulXlate[srcPtr[0]];
            }
            else if ( extra == 2 )
            {
                *(tagPtr++) = (pulXlate[srcPtr[1]] << 8) | 
                               pulXlate[srcPtr[0]];
            }
            else if (extra == 3)
            {
                *(tagPtr++) = (pulXlate[srcPtr[2]] << 16) | 
                              (pulXlate[srcPtr[1]] <<  8) | 
                               pulXlate[srcPtr[0]];
            }
        } 
        else if ( ppdev->cPelSize == 1 )     // 16bpp
        {
            extra = width & 1;
            width >>= 1;

            QUEUE_PXRX_DMA_HOLD( __GlintTagColor, width + extra );
            QUEUE_PXRX_DMA_BUFF_DELAYED( tagPtr, width + extra );

            DISPDBG((DBGLVL, "width was %d, is now %d + %d", 
                             prcl->right - prcl->left, width, extra));

            for ( i = 0; i < width; i++, srcPtr += 2 )
            {
                *(tagPtr++) = (pulXlate[srcPtr[1]] << 16) | 
                               pulXlate[srcPtr[0]];
            }

            if ( extra )
            {
                *(tagPtr++) = pulXlate[srcPtr[0]];
            }
        } 
        else 
        {
            QUEUE_PXRX_DMA_HOLD( __GlintTagColor, width );
            QUEUE_PXRX_DMA_BUFF_DELAYED( tagPtr, width );

            DISPDBG((DBGLVL, "width was %d, is now %d + %d", 
                             prcl->right - prcl->left, width, 0));

            for ( i = 0; i < width; i++ )
            {
                *(tagPtr++) = pulXlate[*(srcPtr++)];
            }
        }

        SEND_PXRX_DMA_BATCH;

        return;
    }

    // Set up the LUT table:

    if ( (ppdev->PalLUTType != LUTCACHE_XLATE) || 
         (ppdev->iPalUniq != pxlo->iUniq) ) 
    {
        // Someone has hijacked the LUT so we need to invalidate it:
        ppdev->PalLUTType = LUTCACHE_XLATE;
        ppdev->iPalUniq = pxlo->iUniq;
        invalidLUT = TRUE;
    } 
    else 
    {
        DISPDBG((DBGLVL, "pxrxCopyXfer8bpp: reusing cached xlate"));
    }

    WAIT_PXRX_DMA_TAGS( 1 + 1 );

    lutMode = glintInfo->lutMode & ~((3 << 2) | (1 << 4) | (7 << 8));
    lutMode |= (ppdev->cPelSize + 2) << 8;
    LOAD_LUTMODE( lutMode );

    if ( invalidLUT ) 
    {
        LONG    cEntries = 256;
        
        pulXlate = pxlo->pulXlate;        

        QUEUE_PXRX_DMA_TAG( __PXRXTagLUTIndex, 0 );

        if ( ppdev->cPelSize == 0 )     // 8bpp
        {
            WAIT_PXRX_DMA_TAGS( cEntries );

            do 
            {
                ul = *(pulXlate++);
                ul |= ul << 8;
                ul |= ul << 16;
                QUEUE_PXRX_DMA_TAG( __PXRXTagLUTData, ul );
            } while( --cEntries );
        } 
        else if( ppdev->cPelSize == 1 )     // 16bpp
        {
            WAIT_PXRX_DMA_TAGS( cEntries );

            do 
            {
                ul = *(pulXlate++);
                ul |= ul << 16;
                QUEUE_PXRX_DMA_TAG( __PXRXTagLUTData, ul );
            } while( --cEntries );
        } 
        else 
        {
            WAIT_PXRX_DMA_DWORDS( 1 + cEntries );

            QUEUE_PXRX_DMA_HOLD( __PXRXTagLUTData, cEntries );
            QUEUE_PXRX_DMA_BUFF( pulXlate, cEntries );
        }
    }

    config2D = glintInfo->config2D & ~(__CONFIG2D_LOGOP_FORE_ENABLE | 
                                       __CONFIG2D_LOGOP_BACK_ENABLE | 
                                       __CONFIG2D_ENABLES);
    config2D |= __CONFIG2D_FBWRITE     | 
                __CONFIG2D_USERSCISSOR | 
                __CONFIG2D_EXTERNALSRC | 
                __CONFIG2D_LUTENABLE;
    render2D = __RENDER2D_INCX        | 
               __RENDER2D_INCY        | 
               __RENDER2D_OP_SYNCDATA | 
               __RENDER2D_SPANS;

    WAIT_PXRX_DMA_TAGS( 3 );

    LOAD_CONFIG2D( config2D );

    cPelInv = 2 - ppdev->cPelSize;
    // Everything before the LUT runs at 8bpp
    pixelSize = (1 << 31)       | 
                (2 << 2)        | 
                (2 << 4)        | 
                (2 << 6)        |        
                (cPelInv << 8)  | 
                (cPelInv << 10) | 
                (cPelInv << 12) | 
                (cPelInv << 14);
                
    QUEUE_PXRX_DMA_TAG( __GlintTagPixelSize, pixelSize );

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;     // Add to destination to get source

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    while ( TRUE ) 
    {
        DISPDBG((DBGLVL, "download to rect (%d,%d) to (%d,%d)", 
                         prcl->left, prcl->top, prcl->right, prcl->bottom));

        // 8bpp => 1 pixel per byte => 4 pixels per dword

        // Assume source bitmap width is dword aligned
        ASSERTDD( (lSrcDelta & 3) == 0, 
                  "pxrxCopyXfer8bpp: "
                  "SOURCE BITMAP WIDTH IS NOT DWORD ALIGNED!!!" );

        // pointer to first pixel, in pixels/bytes
        startPos    = (((UINT_PTR) pjSrcScan0) + 
                        ((prcl->top + dy) * lSrcDelta)) 
                      + (prcl->left + dx);    

        // dword pointer to dword aligned first pixel                      
        pjSrc       = (ULONG *) (startPos & ~3);     
        
        if ( pjSrc == NULL )
        {
            DISPDBG((ERRLVL, "ERROR: pxrxCopyXfer8bppLge "
                             "return because of pjSrc NULL"));
            return;
        }
        
        alignOff = (ULONG)(startPos & 3);  // number of pixels past dword 
                                           // alignment of a scanline
        LeftEdge = prcl->left - alignOff;  // dword aligned left edge in pixels
        AlignWidth = ((prcl->right - LeftEdge) + 3) & ~3; // dword aligned width 
                                                          // in pixels
        cy = prcl->bottom - prcl->top;     // number of scanlines to do

        DISPDBG((DBGLVL, "pjSrcScan0 = 0x%08X, startPos = 0x%08X, "
                         "pjSrc = 0x%08X", 
                         pjSrcScan0, startPos, pjSrc));
        DISPDBG((DBGLVL, "offset = %d pixels", alignOff));
        DISPDBG((DBGLVL, "Aligned rect = (%d -> %d) => %d pixels => %d dwords", 
                         LeftEdge, LeftEdge + AlignWidth, 
                         AlignWidth, AlignWidth >> 2));

        WAIT_PXRX_DMA_TAGS( 4 );

        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMinXY,         
                                        MAKEDWORD_XY(prcl->left,       0) );
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY,         
                                        MAKEDWORD_XY(prcl->right, 0x7fff) );
        QUEUE_PXRX_DMA_TAG( __GlintTagRectanglePosition,    
                                        MAKEDWORD_XY(LeftEdge, prcl->top) );
        QUEUE_PXRX_DMA_TAG( __GlintTagRender2D, render2D                     | 
                                                __RENDER2D_WIDTH(AlignWidth) | 
                                                __RENDER2D_HEIGHT(cy) );
        SEND_PXRX_DMA_BATCH;

        AlignWidth  >>= 2;              // dword aligned width in dwords
        lSrcDeltaDW = lSrcDelta >> 2;   // scanline delta in dwords 
                                        // (start to start)
        lTrueDelta  = lSrcDeltaDW - AlignWidth;  // scanline delta in dwords 
                                                 // (end   to start)
                                                 
        DISPDBG((DBGLVL, "Delta = %d bytes = %d dwords -> %d - %d dwords", 
                         lSrcDelta, lSrcDeltaDW, lTrueDelta, AlignWidth));

        // Do a raw download:
        while ( TRUE ) 
        {
            DISPDBG((DBGLVL, "cy = %d", cy));

            WAIT_PXRX_DMA_DWORDS( AlignWidth + 1 );
            QUEUE_PXRX_DMA_HOLD( __GlintTagColor, AlignWidth );
            TEST_DWORD_ALIGNED( pjSrc );
            QUEUE_PXRX_DMA_BUFF( pjSrc, AlignWidth );
            SEND_PXRX_DMA_BATCH;

            if ( --cy == 0 )
            {
                break;
            }

            pjSrc += lSrcDeltaDW;
        }

        if ( --count == 0 )
        {
            break;
        }

        prcl++;
    }

    // Reset some defaults:
    WAIT_PXRX_DMA_TAGS( 2 );
    QUEUE_PXRX_DMA_TAG( __GlintTagPixelSize, cPelInv );
    if ( ppdev->cPelSize == GLINTDEPTH32 )
    {
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY, 0x7FFF7FFF );
    }

    SEND_PXRX_DMA_BATCH;

    DISPDBG((DBGLVL, "pxrxCopyXfer8bpp return"));
}


/******************************************************************************\
 *                                                                            *
 * FUNC: pxrxMemUpload                                                        *
 * ARGS: ppdev (I) - pointer to the physical device object                    *
 *       crcl (I) - number of destination clipping rectangles                 *
 *       prcl (I) - array of destination clipping rectangles                  *
 *       psoDst (I) - destination surface                                     *
 *       pptlSrc (I) - offset into source surface                             *
 *       prclDst (I) - unclipped destination rectangle                        *
 * RETN:  void                                                                *
 *                                                                            *
\******************************************************************************/

VOID pxrxMemUpload(
    PPDEV ppdev, 
    LONG crcl, 
    RECTL *prcl, 
    SURFOBJ *psoDst, 
    POINTL *pptlSrc, 
    RECTL *prclDst)
{
    BYTE *pDst, *pSrc;
    LONG dwScanLineSize, cySrc, lSrcOff, lSrcStride;
    GLINT_DECL;

    // Make sure we're not performing other operations on the fb areas we want
    SYNC_WITH_GLINT;
    
    ASSERTDD(psoDst->iBitmapFormat == ppdev->iBitmapFormat, 
             "Dest must be same colour depth as screen");
             
    ASSERTDD(crcl > 0, "Can't handle zero rectangles");

    for(; --crcl >= 0; ++prcl) 
    {
        // This gives an offset for offscreen DIBs (zero for primary rectangles)
        lSrcOff = ppdev->DstPixelOrigin + 
                  (ppdev->xyOffsetDst & 0xffff) +
                  (ppdev->xyOffsetDst >> 16) * ppdev->DstPixelDelta;

        // Determine stride on wheter we are blitting from the 
        // primary or from an offscreen DIB
        if (( ppdev->DstPixelOrigin == 0 ) && 
            (ppdev->xyOffsetDst == 0)       )
        {
            lSrcStride = ppdev->lDelta;
        }
        else
        {
            lSrcStride = ppdev->DstPixelDelta * ppdev->cjPelSize;
        }              
    
        // pSrc must point to mem mapped primary      
        pSrc = (BYTE *)ppdev->pjScreen 
                 + (lSrcOff * ppdev->cjPelSize)         
                 + ((LONG)pptlSrc->x * ppdev->cjPelSize) 
                 + ((LONG)pptlSrc->y * lSrcStride); 
  
        // pDst must point to the sysmem SURFOBJ 
        pDst = (BYTE *)psoDst->pvScan0 
                 + ((LONG)prcl->left * ppdev->cjPelSize) 
                 + ((LONG)prcl->top  * (LONG)psoDst->lDelta);                     

        // dwScanLineSize must have the right size to transfer in bytes
        dwScanLineSize = ((LONG)prcl->right - (LONG)prcl->left) * ppdev->cjPelSize;

        // Number of scan lines to transfer
        cySrc = prcl->bottom - prcl->top;

        // Do the copy
        while (--cySrc >= 0) 
        {
            // memcpy(dst, src, size)
            memcpy(pDst, pSrc, dwScanLineSize);
            pDst += psoDst->lDelta; // add stride
            pSrc += lSrcStride;  // add stride
        }
    }

} // pxrxMemUpload


/******************************************************************************\
 *                                                                            *
 *  FUNC: pxrxFifoUpload                                                      *
 *  ARGS: ppdev (I) - pointer to the physical device object                   *
 *        crcl (I) - number of destination clipping rectangles                *
 *        prcl (I) - array of destination clipping rectangles                 *
 *        psoDst (I) - destination surface                                    *
 *        pptlSrc (I) - offset into source surface                            *
 *        prclDst (I) - unclipped destination rectangle                       *
 *  RETN:  void                                                               *
 *                                                                            *
 *  upload from on-chip source into host memory surface. Upload in spans      *
 *  (64-bit aligned) to minimise messages through the core and entries in the *
 *  host out fifo.                                                            *
 *                                                                            *
\******************************************************************************/
VOID pxrxFifoUpload(
    PPDEV       ppdev, 
    LONG        crcl, 
    RECTL      *prcl, 
    SURFOBJ    *psoDst, 
    POINTL     *pptlSrc, 
    RECTL      *prclDst)
{
    LONG        xDomSrc, xSubSrc, yStartSrc, cxSrc, cySrc;
    LONG        culPerSrcScan;
    LONG        culDstDelta;
    BOOL        bRemPerSrcScan;
    ULONG      *pulDst, *pulDstScan;
    ULONG       leftMask, rightMask;
    LONG        cul, ul;
    LONG        cFifoSpaces;
    __GlintFilterModeFmat   FilterMode;
    GLINT_DECL;

    WAIT_PXRX_DMA_TAGS(1);
    QUEUE_PXRX_DMA_TAG( __GlintTagFBDestReadMode, (glintInfo->fbDestMode | 0x103));
    SEND_PXRX_DMA_FORCE;

    DISPDBG((DBGLVL, "pxrxFifoUpload: prcl = (%d, %d -> %d, %d), "
                     "prclDst = (%d, %d -> %d, %d), ptlSrc(%d, %d), count = %d",
                     prcl->left, prcl->top, prcl->right, prcl->bottom, 
                     prclDst->left, prclDst->top, prclDst->right, 
                     prclDst->bottom, pptlSrc->x, pptlSrc->y, crcl));

    DISPDBG((DBGLVL, "pxrxFifoUpload: psoDst: cx = %d, cy = %d, "
                     "lDelta = %d, pvScan0=%P)",
                     psoDst->sizlBitmap.cx, psoDst->sizlBitmap.cy, 
                     psoDst->lDelta, psoDst->pvScan0));
                     
    DISPDBG((DBGLVL, "pxrxFifoUpload: xyOffsetDst = (%d, %d), "
                     "xyOffsetSrc = (%d, %d)",
                     ppdev->xyOffsetDst & 0xFFFF, ppdev->xyOffsetDst >> 16,
                     ppdev->xyOffsetSrc & 0xFFFF, ppdev->xyOffsetSrc >> 16));

    ASSERTDD(psoDst->iBitmapFormat == ppdev->iBitmapFormat, 
             "Dest must be same colour depth as screen");
    ASSERTDD(crcl > 0, "Can't handle zero rectangles");

    WAIT_PXRX_DMA_TAGS(5);

    LOAD_CONFIG2D(__CONFIG2D_FBDESTREAD);
    SET_READ_BUFFERS;

    // enable filter mode so we can get Sync 
    // and color messages on the output FIFO
    *(DWORD *)(&FilterMode) = 0;
    FilterMode.Synchronization = __GLINT_FILTER_TAG;
    FilterMode.Color             = __GLINT_FILTER_DATA;
    QUEUE_PXRX_DMA_TAG(__GlintTagFilterMode, *(DWORD *)(&FilterMode));

    for (; --crcl >= 0; ++prcl) 
    {
        DISPDBG((DBGLVL, "pxrxFifoUpload: dest prcl(%xh,%xh..%xh,%xh)", 
                         prcl->left, prcl->top, prcl->right, prcl->bottom));

        // calculate pixel-aligned source
        xDomSrc   = pptlSrc->x + prcl->left  - prclDst->left;
        xSubSrc   = pptlSrc->x + prcl->right - prclDst->left;
        yStartSrc = pptlSrc->y + prcl->top   - prclDst->top;
        cySrc     = prcl->bottom - prcl->top;

        DISPDBG((DBGLVL, "pxrxFifoUpload: src (%xh,%xh..%xh,%xh)", 
                         xDomSrc, yStartSrc, xSubSrc, yStartSrc + cySrc));

        // will upload ulongs aligned to ulongs
        if (ppdev->cPelSize == GLINTDEPTH32) 
        {
            cxSrc = xSubSrc - xDomSrc;
            culPerSrcScan = cxSrc;
            leftMask  = 0xFFFFFFFF;
            rightMask = 0xFFFFFFFF;
        }    
        else 
        {
            if (ppdev->cPelSize == GLINTDEPTH16) 
            {
                ULONG cPixFromUlongBoundary = prcl->left & 1;

                xDomSrc -= cPixFromUlongBoundary;
                cxSrc = xSubSrc - xDomSrc;
                culPerSrcScan  = (xSubSrc - xDomSrc + 1) >> 1;

                leftMask  = 0xFFFFFFFF << (cPixFromUlongBoundary << 4);
                rightMask = 0xFFFFFFFF >> (((xDomSrc - xSubSrc) & 1) << 4);

            }
            else 
            {
                ULONG cPixFromUlongBoundary = prcl->left & 3;

                xDomSrc -= cPixFromUlongBoundary;
                cxSrc = xSubSrc - xDomSrc;
                culPerSrcScan  = (xSubSrc - xDomSrc + 3) >> 2;

                leftMask  = 0xFFFFFFFF << (cPixFromUlongBoundary << 3);
                rightMask = 0xFFFFFFFF >> (((xDomSrc - xSubSrc) & 3) << 3);

            }     
            // We just want a single mask if the area to upload is less 
            // than one word wide.
            if (culPerSrcScan == 1)
            {
                leftMask &= rightMask;
            }
        }

        // uploading 64 bit aligned source
        bRemPerSrcScan = culPerSrcScan & 1;

        // Work out where the destination data goes to
        culDstDelta = psoDst->lDelta >> 2;
        pulDst = ((ULONG *)psoDst->pvScan0) + 
                  (prcl->left >> (2 - ppdev->cPelSize)) 
                 + culDstDelta * prcl->top;

        DISPDBG((DBGLVL, "pxrxFifoUpload: uploading aligned "
                         "src (%xh,%xh..%xh,%xh)", 
                         xDomSrc, yStartSrc, 
                         xDomSrc + cxSrc, yStartSrc + cySrc));

        // Render the rectangle
        WAIT_PXRX_DMA_TAGS(2);
        QUEUE_PXRX_DMA_TAG( __GlintTagRectanglePosition,
                                            MAKEDWORD_XY(xDomSrc, yStartSrc));
        QUEUE_PXRX_DMA_TAG( __GlintTagRender2D,         
                                            __RENDER2D_OP_NORMAL    | 
                                            __RENDER2D_SPANS        |
                                            __RENDER2D_INCY         | 
                                            __RENDER2D_INCX         | 
                                            __RENDER2D_WIDTH(cxSrc) | 
                                            __RENDER2D_HEIGHT(cySrc));
        SEND_PXRX_DMA_FORCE;
        
        // If the start and end masks are 0xffffffff, we can just upload 
        // the words and put them directly into the destination. Otherwise, 
        // or the first and last word on any scanline we have to mask 
        // off any pixels that are outside the render area. We know the 
        // glint will have 0 in the undesired right hand edge pixels, as 
        // these were not in the render area. We dont know anything about 
        // the destination though.
        
        if ((leftMask == 0xFFFFFFFF) && (rightMask == 0xFFFFFFFF))
        {
            DISPDBG((DBGLVL, "pxrxFifoUpload: no edge masks"));
            
            while (--cySrc >= 0) 
            {
                pulDstScan = pulDst;
                pulDst += culDstDelta;

                DISPDBG((DBGLVL, "pxrxFifoUpload: uploading scan of %xh "
                                 "ulongs to %p (Remainder %xh)", 
                                 culPerSrcScan, pulDstScan, bRemPerSrcScan));

                cul = culPerSrcScan;
                while (cul)
                {
                    WAIT_OUTPUT_FIFO_NOT_EMPTY(cFifoSpaces);
                    if (cFifoSpaces > cul)
                    {
                        cFifoSpaces = cul;
                    }

                    cul -= cFifoSpaces;
                    while (--cFifoSpaces >= 0) 
                    {
                        READ_OUTPUT_FIFO(ul);
                        DISPDBG((DBGLVL, "pxrxFifoUpload: read %08.8xh from "
                                         "output FIFO", ul));
                         *pulDstScan++ = ul;
                    }
                }
                
                if (bRemPerSrcScan)
                {
                    WAIT_OUTPUT_FIFO_NOT_EMPTY(cFifoSpaces);
                    READ_OUTPUT_FIFO(ul);
                    DISPDBG((DBGLVL, "pxrxFifoUpload: read remainder %08.8xh "
                                     "from output FIFO", ul));
                }
            }
        }
        else if (culPerSrcScan == 1)
        {
            DISPDBG((DBGLVL, "pxrxFifoUpload: single ulong per scan"));

            while (--cySrc >= 0) 
            {
                WAIT_OUTPUT_FIFO_NOT_EMPTY(cFifoSpaces);
                READ_OUTPUT_FIFO(ul);
                DISPDBG((DBGLVL, "pxrxFifoUpload: "
                                 "read %08.8xh from output FIFO", ul));

                // leftMask contains both masks in this case
                *pulDst = (*pulDst & ~leftMask) | (ul & leftMask);

                ASSERTDD(bRemPerSrcScan, "one word per scan upload should "
                                         "always leave a remainder");
                WAIT_OUTPUT_FIFO_NOT_EMPTY(cFifoSpaces);
                READ_OUTPUT_FIFO(ul);
                DISPDBG((DBGLVL, "pxrxFifoUpload: read remainder %08.8xh "
                                 "from output FIFO", ul));
                pulDst += culDstDelta;
            }
        }
        else
        {
            DISPDBG((DBGLVL, "pxrxFifoUpload: scan with left & right edge "
                             "masks: %08.8x .. %08.8x", leftMask, rightMask));

            while (--cySrc >= 0) 
            {
                pulDstScan = pulDst;
                pulDst += culDstDelta;

                DISPDBG((DBGLVL, "pxrxFifoUpload: uploading scan of %xh "
                                 "ulongs to %p", culPerSrcScan, pulDstScan));

                // get first ulong
                WAIT_OUTPUT_FIFO_NOT_EMPTY(cFifoSpaces);
                --cFifoSpaces;
                READ_OUTPUT_FIFO(ul);
                
                DISPDBG((DBGLVL, "pxrxFifoUpload: "
                                 "read %08.8xh from output FIFO", ul));
                                 
                *pulDstScan++ = (*pulDstScan & ~leftMask) | (ul & leftMask);
                 
                // get middle ulongs
                cul = culPerSrcScan - 2; 
                while (cul) 
                {
                    if (cFifoSpaces > cul)
                    {
                        cFifoSpaces = cul;
                    }

                    cul -= cFifoSpaces;
                    while (--cFifoSpaces >= 0) 
                    {
                        READ_OUTPUT_FIFO(ul);
                        DISPDBG((DBGLVL, "pxrxFifoUpload: "
                                         "read %08.8xh from output FIFO", ul));
                         *pulDstScan++ = ul;
                    }
                    WAIT_OUTPUT_FIFO_NOT_EMPTY(cFifoSpaces);
                }
                  
                // get last ulong
                READ_OUTPUT_FIFO(ul);
                DISPDBG((DBGLVL, "pxrxFifoUpload: "
                                 "read %08.8xh from output FIFO", ul));
                                 
                *pulDstScan = (*pulDstScan & ~rightMask) | (ul & rightMask);

                if (bRemPerSrcScan)
                {
                    WAIT_OUTPUT_FIFO_NOT_EMPTY(cFifoSpaces);
                    READ_OUTPUT_FIFO(ul);
                    DISPDBG((DBGLVL, "pxrxFifoUpload: read remainder "
                                     "%08.8xh from output FIFO", ul));
                }
            }
        }
    }

#if DBG
    cul = 0xaa55aa55;
    DISPDBG((DBGLVL, "pxrxFifoUpload: waiting for sync (id = %08.8xh)", cul));
    WAIT_PXRX_DMA_TAGS(1);
    QUEUE_PXRX_DMA_TAG(__GlintTagSync, cul);
    SEND_PXRX_DMA_FORCE;
    do
    {
        WAIT_OUTPUT_FIFO_READY;
        READ_OUTPUT_FIFO(ul);
        DISPDBG((DBGLVL, "pxrxFifoUpload: read %08.8xh from output FIFO", ul));
        if(ul != __GlintTagSync)
        {
            DISPDBG((ERRLVL,"pxrxFifoUpload: didn't read back sync!"));
        }
    }
    while (ul != __GlintTagSync);
    DISPDBG((DBGLVL, "pxrxFifoUpload: got sync"));
#endif

    // no need to initiate DMA with this tag - it will get flushed with the 
    // next primitive and meanwhile will not affect local memory
    WAIT_PXRX_DMA_TAGS(1);
    QUEUE_PXRX_DMA_TAG(__GlintTagFilterMode, 0);
    SEND_PXRX_DMA_BATCH;

    GLINT_CORE_IDLE;

    DISPDBG((DBGLVL, "pxrxFifoUpload: done"));
}

/******************************************************************************\
 * VOID vGlintCopyBltBypassDownloadXlate8bpp                                  *
 *                                                                            *
 * using the bypass mechanism we can take advantage of write-combining        *
 *       which can be quicker than using the FIFO                             *
 * NB. supports 32bpp and 16bpp destinations                                  *
\******************************************************************************/
VOID vGlintCopyBltBypassDownloadXlate8bpp(
    PDEV       *ppdev,
    SURFOBJ    *psoSrc,
    POINTL     *pptlSrc,
    RECTL      *prclDst,
    RECTL      *prclClip,
    LONG        crclClip,
    XLATEOBJ   *pxlo)
{
    LONG        xOff;
    BYTE       *pjSrcScan0;
    LONG        cjSrcDelta, xSrcOff, ySrcOff;
    ULONG      *pulDstScan0;
    LONG        culDstDelta, xDstOff;
    LONG        cScans, cPixPerScan, c;
    ULONG       cjSrcDeltaRem, cjDstDeltaRem;
    ULONG      *aulXlate;
    BYTE       *pjSrc;
    GLINT_DECL;

    pjSrcScan0 = (BYTE *)psoSrc->pvScan0;
    cjSrcDelta = psoSrc->lDelta;
    
    // need to add arclClip[n].left to get xSrc
    xSrcOff = pptlSrc->x - prclDst->left; 
    // need to add arclClip[n].top to get ySrc
    ySrcOff = pptlSrc->y - prclDst->top;  

    pulDstScan0 = (ULONG *)ppdev->pjScreen;
    culDstDelta = ppdev->DstPixelDelta >> (2 - ppdev->cPelSize);
    xDstOff     = ppdev->DstPixelOrigin + (ppdev->xyOffsetDst & 0xffff) +
                  (ppdev->xyOffsetDst >> 16) * ppdev->DstPixelDelta;

    aulXlate = pxlo->pulXlate;

    SYNC_IF_CORE_BUSY;

    for (; --crclClip >= 0; ++prclClip)
    {
        cScans = prclClip->bottom - prclClip->top;
        cPixPerScan = prclClip->right - prclClip->left;
        cjSrcDeltaRem = cjSrcDelta - cPixPerScan;
        pjSrc = -1 + pjSrcScan0 + xSrcOff + prclClip->left
                + ((prclClip->top + ySrcOff) * cjSrcDelta);

        if (ppdev->cPelSize == GLINTDEPTH32)
        {
            ULONG *pulDst;
            cjDstDeltaRem = (culDstDelta - cPixPerScan) << 2;
            pulDst = -1 + pulDstScan0 + xDstOff + prclClip->left
                     + prclClip->top * culDstDelta;
                     
            for (; 
                 --cScans >= 0; 
                 pjSrc += cjSrcDeltaRem, (BYTE *)pulDst += cjDstDeltaRem)
            {
                for (c = cPixPerScan; --c >= 0;)
                {
                    *++pulDst = aulXlate[*++pjSrc];
                }
            }
        }
        else // (GLINTDEPTH16)
        {
            USHORT *pusDst;
            cjDstDeltaRem = 
                        (culDstDelta << 2) - (cPixPerScan << ppdev->cPelSize);
                        
            pusDst = -1 + (USHORT *)pulDstScan0 + xDstOff + prclClip->left
                        + ((prclClip->top * culDstDelta) << 1);
                     
            for (; 
                 --cScans >= 0; 
                 pjSrc += cjSrcDeltaRem, (BYTE *)pusDst += cjDstDeltaRem)
            {
                for (c = cPixPerScan; --c >= 0;)
                {
                    *++pusDst = (USHORT)aulXlate[*++pjSrc];
                }
            }
        }
    }
}


