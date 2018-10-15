//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright  1997 - 2003  Microsoft Corporation.  All Rights Reserved.
//
//  FILE:   BITMAP.H
//    
//
//  PURPOSE:    Define the COemPDEV class which stores the private
//          PDEV for the driver.
//
//
//  Functions:
//          COemPDEV constructor and destructor
//          COemPDEV::InitializeDDITable
//          COemPDEV::_SavePFN
//          COemPDEV::_ResetPointers
//
//
//  PLATFORMS:
//
//    Windows XP, Windows Server 2003
//
//
//  History: 
//          06/24/03    xxx created.
//
//

#ifndef _BITMAP_H
#define _BITMAP_H

#include "ddihook.h"
#include "debug.h"

#define OEM_SIGNATURE   'MSFT'
#define OEM_VERSION     0x00000001L

#define RGB_BLACK   RGB(0, 0, 0)
#define RGB_WHITE   RGB(255, 255, 255)

class COemPDEV
{
public:
    __stdcall COemPDEV(void)
    {

        // Note that since our parent has AddRef'd the UNIDRV interface,
        // we don't do so here since our scope is identical to our
        // parent.
        //

        OEMDBG(DBG_VERBOSE, L"In COemPDEV constructor...");

        _ResetPointers();
    }

    __stdcall ~COemPDEV(void)
    {
        OEMDBG(DBG_VERBOSE, L"In COemPDEV destructor...");
    }

    void __stdcall InitializeDDITable(DRVENABLEDATA* pded)
    {
        OEMDBG(DBG_VERBOSE, L"COemPDEV::InitializeDDITable entry.");

    #if defined(DDIS_HAVE_BEEN_IMPL)
        UINT   iDrvFn;
        UINT   cDrvFn = pded->c;
        PDRVFN pDrvFn = pded->pdrvfn;

        for (iDrvFn = 0; iDrvFn < cDrvFn; ++iDrvFn, ++pDrvFn)
        {
            _SavePFN(pDrvFn->iFunc, pDrvFn->pfn);
        }
    #else
        VERBOSE(DLLTEXT(L"No DDIs have been implemented!\r\n"));    
    #endif // any hooked DDI
    }

private:

#if defined(DDIS_HAVE_BEEN_IMPL)
    void __stdcall _SavePFN(ULONG uFunc, PFN pfn)
    {

        // Note: if a "pass-through" driver is being constructed, then
        // there is nothing for this method to do. So, to keep the 
        // compiler from complaining about a switch with no case
        // statements, but which has a default statement, we surround
        // the entire construct with preprocessor controls.
        //
        switch(uFunc)
        {
        // The following are the drawing related DDI hooks.
        //
    #if defined(IMPL_ALPHABLEND)
        // 71
        case INDEX_DrvAlphaBlend:
            m_pfnDrvAlphaBlend  = (PFN_DrvAlphaBlend)pfn;
            break;
    #endif

    #if defined(IMPL_BITBLT)
        // 18
        case INDEX_DrvBitBlt:
            m_pfnDrvBitBlt  = (PFN_DrvBitBlt)pfn;
            break;
    #endif

    #if defined(IMPL_COPYBITS)
        // 19
        case INDEX_DrvCopyBits:
            m_pfnDrvCopyBits = (PFN_DrvCopyBits)pfn;
            break;
    #endif

    #if defined(IMPL_DITHERCOLOR)
        // 13
        case INDEX_DrvDitherColor:
            m_pfnDrvDitherColor = (PFN_DrvDitherColor)pfn;
            break;
    #endif

    #if defined(IMPL_FILLPATH)
        // 15
        case INDEX_DrvFillPath:
            m_pfnDrvFillPath = (PFN_DrvFillPath)pfn;
            break;
    #endif

    #if defined(IMPL_FONTMANAGEMENT)
        // 47
        case INDEX_DrvFontManagement:
            m_pfnDrvFontManagement = (PFN_DrvFontManagement)pfn;
            break;
    #endif

    #if defined(IMPL_GETGLYPHMODE)
        // 37
        case INDEX_DrvGetGlyphMode:
            m_pfnDrvGetGlyphMode = (PFN_DrvGetGlyphMode)pfn;
            break;
    #endif

    #if defined(IMPL_GRADIENTFILL)
        // 68
        case INDEX_DrvGradientFill:
            m_pfnDrvGradientFill = (PFN_DrvGradientFill)pfn;
            break;
    #endif

    #if defined(IMPL_LINETO)
        // 31
        case INDEX_DrvLineTo:
            m_pfnDrvLineTo = (PFN_DrvLineTo)pfn;
            break;
    #endif

    #if defined(IMPL_PAINT)
        // 70
        case INDEX_DrvPaint:
            m_pfnDrvPaint = (PFN_DrvPaint)pfn;
            break;
    #endif

    #if defined(IMPL_PLGBLT)
        // 70
        case INDEX_DrvPlgBlt:
            m_pfnDrvPlgBlt = (PFN_DrvPlgBlt)pfn;
            break;
    #endif

    #if defined(IMPL_QUERYADVANCEWIDTHS)
        // 53
        case INDEX_DrvQueryAdvanceWidths:
            m_pfnDrvQueryAdvanceWidths = (PFN_DrvQueryAdvanceWidths)pfn;
            break;
    #endif

    #if defined(IMPL_QUERYFONT)
        // 26
        case INDEX_DrvQueryFont:
            m_pfnDrvQueryFont = (PFN_DrvQueryFont)pfn;
            break;
    #endif

    #if defined(IMPL_QUERYFONTDATA)
        // 28
        case INDEX_DrvQueryFontData:
            m_pfnDrvQueryFontData = (PFN_DrvQueryFontData)pfn;
            break;
    #endif

    #if defined(IMPL_QUERYFONTTREE)
        // 27
        case INDEX_DrvQueryFontTree:
            m_pfnDrvQueryFontTree = (PFN_DrvQueryFontTree)pfn;
            break;
    #endif

    #if defined(IMPL_REALIZEBRUSH)
        // 12
        case INDEX_DrvRealizeBrush:
            m_pfnDrvRealizeBrush = (PFN_DrvRealizeBrush)pfn;
            break;
    #endif

    #if defined(IMPL_STRETCHBLT)
        // 20
        case INDEX_DrvStretchBlt:
            m_pfnDrvStretchBlt = (PFN_DrvStretchBlt)pfn;
            break;
    #endif

    #if defined(IMPL_STRETCHBLTROP)
        // 69
        case INDEX_DrvStretchBltROP:
            m_pfnDrvStretchBltROP = (PFN_DrvStretchBltROP)pfn;
            break;
    #endif

    #if defined(IMPL_STROKEANDFILLPATH)
        // 16
        case INDEX_DrvStrokeAndFillPath:
            m_pfnDrvStrokeAndFillPath = (PFN_DrvStrokeAndFillPath)pfn;
            break;
    #endif

    #if defined(IMPL_STROKEPATH)
        // 14
        case INDEX_DrvStrokePath:
            m_pfnDrvStrokePath = (PFN_DrvStrokePath)pfn;
            break;
    #endif

    #if defined(IMPL_TEXTOUT)
        // 23
        case INDEX_DrvTextOut:
            m_pfnDrvTextOut = (PFN_DrvTextOut)pfn;
            break;
    #endif

    #if defined(IMPL_TRANSPARENTBLT)
        // 74
        case INDEX_DrvTransparentBlt:
            m_pfnDrvTransparentBlt = (PFN_DrvTransparentBlt)pfn;
            break;
    #endif

    // The following are the page/band related DDI hooks.
    //
    #if defined(IMPL_STARTDOC)
        // 35
        case INDEX_DrvStartDoc:
            m_pfnDrvStartDoc = (PFN_DrvStartDoc)pfn;
            break;
    #endif
    
    #if defined(IMPL_ENDDOC)
        // 34
        case INDEX_DrvEndDoc:
            m_pfnDrvEndDoc = (PFN_DrvEndDoc)pfn;
            break;
    #endif

    #if defined(IMPL_STARTPAGE)
        // 33
        case INDEX_DrvStartPage:
            m_pfnDrvStartPage = (PFN_DrvStartPage)pfn;
            break;
    #endif

    #if defined(IMPL_SENDPAGE)
        // 32
        case INDEX_DrvSendPage:
            m_pfnDrvSendPage = (PFN_DrvSendPage)pfn;
            break;
    #endif

    #if defined(IMPL_STARTBANDING)
        // 57
        case INDEX_DrvStartBanding:
            m_pfnDrvStartBanding = (PFN_DrvStartBanding)pfn;
            break;
    #endif

    #if defined(IMPL_NEXTBAND)
        // 58
        case INDEX_DrvNextBand:
            m_pfnDrvNextBand = (PFN_DrvNextBand)pfn;
            break;
    #endif

    #if defined(IMPL_ESCAPE)
        // 24
        case INDEX_DrvEscape:
            m_pfnDrvEscape = (PFN_DrvEscape)pfn;
            break;
    #endif

    #if defined(_DEBUG)
        // ----------------------------------------------------------
        default:
            // For development convenience, decode the requests that are 
            // not hooked. In this version of UNIDRV, at most 28 of the 
            // following could appear. The remainder are listed for 
            // diagnostic purposes.
            //

            // This section was mainly for diagnostic purposes. So it has
            // been stripped out for the time being. Will revisit when
            // examining the debug mechanism. 
            //
            break;
    #endif
        } // switch(uFunc)
    }
#endif
    void __stdcall _ResetPointers(void)
    {
        m_pfnDrvAlphaBlend          = NULL;
        m_pfnDrvBitBlt              = NULL;
        m_pfnDrvCopyBits                = NULL;
        m_pfnDrvDitherColor         = NULL;
        m_pfnDrvEndDoc              = NULL;
        m_pfnDrvEscape              = NULL;
        m_pfnDrvFillPath                = NULL;
        m_pfnDrvFontManagement      = NULL;
        m_pfnDrvGetGlyphMode        = NULL;
        m_pfnDrvGradientFill            = NULL;
        m_pfnDrvLineTo              = NULL;
        m_pfnDrvNextBand            = NULL;
        m_pfnDrvPaint               = NULL;
        m_pfnDrvPlgBlt              = NULL;
        m_pfnDrvQueryAdvanceWidths  = NULL;
        m_pfnDrvQueryFont           = NULL;
        m_pfnDrvQueryFontData       = NULL;
        m_pfnDrvQueryFontTree       = NULL;
        m_pfnDrvRealizeBrush            = NULL;
        m_pfnDrvSendPage            = NULL;
        m_pfnDrvStartBanding            = NULL;
        m_pfnDrvStartDoc                = NULL;
        m_pfnDrvStartPage           = NULL;
        m_pfnDrvStretchBlt          = NULL;
        m_pfnDrvStretchBltROP           = NULL;
        m_pfnDrvStrokeAndFillPath       = NULL;
        m_pfnDrvStrokePath          = NULL;
        m_pfnDrvTextOut             = NULL;
        m_pfnDrvTransparentBlt      = NULL;
    }

public:

    // Unidrv function pointers so that we can punt
    // back from the DDI hooks
    //
    PFN_DrvAlphaBlend           m_pfnDrvAlphaBlend;
    PFN_DrvBitBlt                   m_pfnDrvBitBlt;
    PFN_DrvCopyBits             m_pfnDrvCopyBits;
    PFN_DrvDitherColor          m_pfnDrvDitherColor;
    PFN_DrvEndDoc               m_pfnDrvEndDoc;
    PFN_DrvEscape               m_pfnDrvEscape;
    PFN_DrvFillPath             m_pfnDrvFillPath;
    PFN_DrvFontManagement       m_pfnDrvFontManagement;
    PFN_DrvGetGlyphMode         m_pfnDrvGetGlyphMode;
    PFN_DrvGradientFill         m_pfnDrvGradientFill;
    PFN_DrvLineTo               m_pfnDrvLineTo;
    PFN_DrvNextBand             m_pfnDrvNextBand;
    PFN_DrvPaint                    m_pfnDrvPaint;
    PFN_DrvPlgBlt                   m_pfnDrvPlgBlt;
    PFN_DrvQueryAdvanceWidths   m_pfnDrvQueryAdvanceWidths;
    PFN_DrvQueryFont                m_pfnDrvQueryFont;
    PFN_DrvQueryFontData            m_pfnDrvQueryFontData;
    PFN_DrvQueryFontTree            m_pfnDrvQueryFontTree;
    PFN_DrvRealizeBrush         m_pfnDrvRealizeBrush;
    PFN_DrvSendPage             m_pfnDrvSendPage;
    PFN_DrvStartBanding         m_pfnDrvStartBanding;
    PFN_DrvStartDoc             m_pfnDrvStartDoc;
    PFN_DrvStartPage                m_pfnDrvStartPage;
    PFN_DrvStretchBlt               m_pfnDrvStretchBlt;
    PFN_DrvStretchBltROP            m_pfnDrvStretchBltROP;
    PFN_DrvStrokeAndFillPath        m_pfnDrvStrokeAndFillPath;
    PFN_DrvStrokePath           m_pfnDrvStrokePath;
    PFN_DrvTextOut              m_pfnDrvTextOut;
    PFN_DrvTransparentBlt           m_pfnDrvTransparentBlt;

    /*******************************************/
    /*                                           */
    /*      Add any custom PDEV members here         */
    /*                                           */
    /*******************************************/

    PBYTE               pBufStart;          // Pointer to start of buffer to hold the bitmap data
    DWORD               dwBufSize;          // Buffer size
    BITMAPFILEHEADER    bmFileHeader;       // BitmapFileHeader for each dump
    BITMAPINFOHEADER    bmInfoHeader;       // BitmapInfoHeader for each dump
    int                 cPalColors;         // Count of colors in palette.
    HPALETTE            hpalDefault;            // Default palette to pass to GDI.
    RGBQUAD *           prgbq;              // Color table
    INT                 cbHeaderOffBits;        // Offset to bits in the file
    BOOL                bHeadersFilled;     // Flag to indicate if the header stuff has been filled
    BOOL                bColorTable;            // Flag to indicate if color table needs to be dumped

};
typedef COemPDEV* POEMPDEV;

BOOL bGrowBuffer(POEMPDEV, DWORD);      // Function to enlarge the buffer for holding the bitmap data
VOID vFreeBuffer(POEMPDEV);             // Function to free the buffer for holding the bitmap data
BOOL bFillColorTable(POEMPDEV);         // Function to fill the color table for the bitmap data

#endif


