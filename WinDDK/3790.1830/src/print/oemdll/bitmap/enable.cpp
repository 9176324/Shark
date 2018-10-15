//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright  1998 - 2003  Microsoft Corporation.  All Rights Reserved.
//
//  FILE:    Enable.cpp
//    
//
//  PURPOSE:  Enable routines for User Mode COM Customization DLL.
//
//
//  Functions:
//          OEMEnableDriver
//          OEMDisableDriver
//          OEMEnablePDEV
//          OEMResetPDEV
//          OEMDisablePDEV
//
//      
//
//
//  PLATFORMS:  Windows XP, Windows Server 2003
//
//
//  History: 
//          06/24/03    xxx created.
//
//

#include "precomp.h"
#include <PRCOMOEM.H>
#include "debug.h"
#include "bitmap.h"
#include "ddihook.h"


// ==================================================================
// The purpose of this array is to inform UNIDRV of the callbacks
// that are implemented in this driver.
//
// Note that there is *NO* order dependency in this array. New
// index values and their corresponding callbacks can be placed
// anywhere within the list as needed.
//
static const DRVFN s_aOemHookFuncs[] =
#if defined(DDIS_HAVE_BEEN_IMPL)
{
    // The following are defined in ddihook.cpp.
    //
#if defined(IMPL_ALPHABLEND)
    {INDEX_DrvAlphaBlend, (PFN)OEMAlphaBlend},
#endif

#if defined(IMPL_BITBLT)
    {INDEX_DrvBitBlt, (PFN)OEMBitBlt},
#endif

#if defined(IMPL_COPYBITS)
    {INDEX_DrvCopyBits, (PFN)OEMCopyBits},
#endif

#if defined(IMPL_DITHERCOLOR)
    {INDEX_DrvDitherColor, (PFN)OEMDitherColor},
#endif

#if defined(IMPL_FILLPATH)
    {INDEX_DrvFillPath, (PFN)OEMFillPath},
#endif

#if defined(IMPL_FONTMANAGEMENT)
    {INDEX_DrvFontManagement, (PFN)OEMFontManagement},
#endif

#if defined(IMPL_GETGLYPHMODE)
    {INDEX_DrvGetGlyphMode, (PFN)OEMGetGlyphMode},
#endif

#if defined(IMPL_GRADIENTFILL)
    {INDEX_DrvGradientFill, (PFN)OEMGradientFill},
#endif

#if defined(IMPL_LINETO)
    {INDEX_DrvLineTo, (PFN)OEMLineTo},
#endif

#if defined(IMPL_PAINT)
    {INDEX_DrvPaint, (PFN)OEMPaint},
#endif

#if defined(IMPL_PLGBLT)
    {INDEX_DrvPlgBlt, (PFN)OEMPlgBlt},
#endif

#if defined(IMPL_QUERYADVANCEWIDTHS)
    {INDEX_DrvQueryAdvanceWidths, (PFN)OEMQueryAdvanceWidths},
#endif

#if defined(IMPL_QUERYFONT)
    {INDEX_DrvQueryFont, (PFN)OEMQueryFont},
#endif

#if defined(IMPL_QUERYFONTDATA)
    {INDEX_DrvQueryFontData, (PFN)OEMQueryFontData},
#endif

#if defined(IMPL_QUERYFONTTREE)
    {INDEX_DrvQueryFontTree, (PFN)OEMQueryFontTree},
#endif

#if defined(IMPL_REALIZEBRUSH)
    {INDEX_DrvRealizeBrush, (PFN)OEMRealizeBrush},
#endif

#if defined(IMPL_STRETCHBLT)
    {INDEX_DrvStretchBlt, (PFN)OEMStretchBlt},
#endif

#if defined(IMPL_STRETCHBLTROP)
    {INDEX_DrvStretchBltROP, (PFN)OEMStretchBltROP},
#endif

#if defined(IMPL_STROKEANDFILLPATH)
    {INDEX_DrvStrokeAndFillPath, (PFN)OEMStrokeAndFillPath},
#endif

#if defined(IMPL_STROKEPATH)
    {INDEX_DrvStrokePath, (PFN)OEMStrokePath},
#endif

#if defined(IMPL_TEXTOUT)
    {INDEX_DrvTextOut, (PFN)OEMTextOut},
#endif

#if defined(IMPL_TRANSPARENTBLT)
    {INDEX_DrvTransparentBlt, (PFN)OEMTransparentBlt},
#endif

#if defined(IMPL_STARTDOC)
    {INDEX_DrvStartDoc, (PFN)OEMStartDoc},
#endif

#if defined(IMPL_ENDDOC)
    {INDEX_DrvEndDoc, (PFN)OEMEndDoc},
#endif

#if defined(IMPL_STARTPAGE)
    {INDEX_DrvStartPage, (PFN)OEMStartPage},
#endif

#if defined(IMPL_SENDPAGE)
    {INDEX_DrvSendPage, (PFN)OEMSendPage},
#endif

#if defined(IMPL_STARTBANDING)
    {INDEX_DrvStartBanding, (PFN)OEMStartBanding},
#endif

#if defined(IMPL_NEXTBAND)
    {INDEX_DrvNextBand, (PFN)OEMNextBand},
#endif

#if defined(IMPL_ESCAPE)
    {INDEX_DrvEscape, (PFN)OEMEscape},
#endif
};

#else
    // No DDI hooks have been enabled. This is provided to eliminate
    // a compiler error.
{
    {0,NULL}
};
#endif

BOOL APIENTRY 
OEMEnableDriver(
    DWORD               dwOEMintfVersion, 
    DWORD               dwSize, 
    PDRVENABLEDATA      pded
    )

/*++

Routine Description:

    Implementation of IPrintOemUni::EnableDriver. 
    OEMEnableDriver is called by IPrintOemUni::EnableDriver 
    which is defined in intrface.cpp. 

    The IPrintOemUni::EnableDriver method allows a rendering 
    plug-in to perform the same types of operations as the 
    DrvEnableDriver function. Like the DrvEnableDriver function, 
    the IPrintOemUni::EnableDriver method is responsible for 
    providing addresses of internally supported graphics DDI functions, 
    or DDI hook functions. 

    The method should fill the supplied DRVENABLEDATA structure 
    and allocate an array of DRVFN structures. It should fill the 
    array with pointers to hooking functions, along with winddi.h-defined 
    index values that identify the hooked out graphics DDI functions.
    
    Please refer to DDK documentation for more details.

Arguments:

    IN DriverVersion - interface version number. This value is defined 
                    by PRINTER_OEMINTF_VERSION, in printoem.h. 
    IN cbSize - size, in bytes, of the structure pointed to by pded. 
    OUT pded - pointer to a DRVENABLEDATA structure. Fill this structure 
            with pointers to the DDI hook functions.

Return Value:

    TRUE if successful, FALSE if there is an error

--*/

{
    OEMDBG(DBG_VERBOSE, L"OEMEnableDriver entry.");
    
    // We need to return the DDI functions that have been hooked
    // in pded. Here we fill out the fields in pded.
    //
    pded->iDriverVersion =  PRINTER_OEMINTF_VERSION;
    pded->c = sizeof(s_aOemHookFuncs) / sizeof(DRVFN);
    pded->pdrvfn = (DRVFN *) s_aOemHookFuncs;

    return TRUE;
}

VOID APIENTRY 
OEMDisableDriver(
    VOID
    )

/*++

Routine Description:

    Implementation of IPrintOemUni::DisableDriver. 
    OEMDisableDriver is called by IPrintOemUni::DisableDriver 
    which is defined in intrface.cpp. 

    The IPrintOemUni::DisableDriver method allows a rendering 
    plug-in for Unidrv to free resources that were allocated by the 
    plug-in's IPrintOemUni::EnableDriver method. This is the last 
    IPrintOemUni interface method that is called before the rendering 
    plug-in is unloaded.
    
    Please refer to DDK documentation for more details.

Arguments:

    NONE

Return Value:

    NONE

--*/

{
    OEMDBG(DBG_VERBOSE, L"OEMDisableDriver entry.");

    // Do any cleanup stuff here
    //

}

PDEVOEM APIENTRY 
OEMEnablePDEV(
    PDEVOBJ             pdevobj,
    PWSTR               pPrinterName,
    ULONG               cPatterns,
    HSURF               *phsurfPatterns,
    ULONG               cjGdiInfo,
    GDIINFO             *pGdiInfo,
    ULONG               cjDevInfo,
    DEVINFO             *pDevInfo,
    DRVENABLEDATA       *pded       // Unidrv's hook table
    )

/*++

Routine Description:

    Implementation of IPrintOemUni::EnablePDEV. 
    OEMEnablePDEV is    called by IPrintOemUni::EnablePDEV 
    which is defined in intrface.cpp. 

    The IPrintOemUni::EnablePDEV method performs the same types 
    of operations as the DrvEnablePDEV function that is exported 
    by a printer graphics DLL. Its purpose is to allow a rendering 
    plug-in to create its own PDEV structure. For more information 
    about PDEV structures, see "Customized PDEV Structures" in the DDK docs.
    
    Please refer to DDK documentation for more details.

Arguments:

    IN pdevobj - pointer to a DEVOBJ structure. 
    IN pPrinterName - pointer to a text string representing the logical 
                    address of the printer. 
    IN cPatterns - value representing the number of HSURF-typed 
                surface handles contained in the buffer pointed to 
                by phsurfPatterns. 
    IN phsurfPatterns - pointer to a buffer that is large enough to 
                    contain cPatterns number of HSURF-typed 
                    surface handles.  
    IN cjGdiInfo - value representing the size of the structure pointed 
                to by pGdiInfo. 
    IN pGdiInfo - pointer to a GDIINFO structure. 
    IN cjDevInfo - value representing the size of the structure pointed 
                to by pDevInfo. 
    IN pDevInfo - pointer to a DEVINFO structure. 
    IN pded - pointer to a DRVENABLEDATA structure containing the 
            addresses of the printer driver's graphics DDI hooking functions. 

Return Value:

    OUT pDevOem - pointer to a private PDEV structure.

    Return NULL if error occurs.

--*/

{
    OEMDBG(DBG_VERBOSE, L"OEMEnablePDEV entry.");
    
    // Allocate an instance of our private PDEV.
    //
    POEMPDEV pOemPDEV = new COemPDEV();

    if (NULL == pOemPDEV)
    {
        return NULL;
    }

    pOemPDEV->InitializeDDITable(pded);

    DBG_GDIINFO(DBG_VERBOSE, L"pGdiInfo", pGdiInfo);
    DBG_DEVINFO(DBG_VERBOSE, L"pDevInfo", pDevInfo);

    // Initializing private oempdev stuff
    //
    pOemPDEV->bHeadersFilled = FALSE;
    pOemPDEV->bColorTable = FALSE;
    pOemPDEV->cbHeaderOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    pOemPDEV->bmInfoHeader.biHeight = 0;
    pOemPDEV->bmInfoHeader.biSizeImage = 0;
    pOemPDEV->pBufStart = NULL;
    pOemPDEV->dwBufSize = 0;

    // We create a BGR palette for 24bpp so that we can avoid color byte order manipulation
    //
    if (pGdiInfo->cBitsPixel == 24)
        pDevInfo->hpalDefault = EngCreatePalette(PAL_BGR, 0, 0, 0, 0, 0);

    // Store the handle to the default palette so that we can fill the color table later
    //
    pOemPDEV->hpalDefault = pDevInfo->hpalDefault;

    return pOemPDEV;
    
}

BOOL APIENTRY 
OEMResetPDEV(
    PDEVOBJ     pdevobjOld,
    PDEVOBJ     pdevobjNew
    )

/*++

Routine Description:

    Implementation of IPrintOemUni::ResetPDEV. 
    OEMResetPDEV is called by IPrintOemUni::ResetPDEV 
    which is defined in intrface.cpp. 

    A rendering plug-in's IPrintOemUni::ResetPDEV method performs 
    the same types of operations as the DrvResetPDEV function. During 
    the processing of an application's call to the Platform SDK ResetDC 
    function, the IPrintOemUni::ResetPDEV method is called by Unidrv's 
    DrvResetPDEV function. For more information about when DrvResetPDEV 
    is called, see its description in the DDK docs.

    The rendering plug-in's private PDEV structure's address is contained 
    in the pdevOEM member of the DEVOBJ structure pointed to by pdevobjOld. 
    The IPrintOemUni::ResetPDEV method should use relevant members of this 
    old structure to fill in the new structure, which is referenced through pdevobjNew.

    Please refer to DDK documentation for more details.

Arguments:

    IN pdevobjOld - pointer to a DEVOBJ structure containing 
                current PDEV information. 
    OUT pdevobjNew - pointer to a DEVOBJ structure into which 
                the method should place new PDEV information.  

Return Value:

    TRUE if successful, FALSE if there is an error

--*/

{
    OEMDBG(DBG_VERBOSE, L"OEMResetPDEV entry.");

    return TRUE;
}

VOID APIENTRY 
OEMDisablePDEV(
    PDEVOBJ     pdevobj
    )

/*++

Routine Description:

    Implementation of IPrintOemUni::DisablePDEV. 
    OEMDisablePDEV is called by IPrintOemUni::DisablePDEV 
    which is defined in intrface.cpp. 

    The IPrintOemUni::DisablePDEV method performs the same types of 
    operations as the DrvDisablePDEV function. Its purpose is to allow a 
    rendering plug-in to delete the private PDEV structure that is pointed 
    to by the DEVOBJ structure's pdevOEM member. This PDEV structure 
    is one that was allocated by the plug-in's IPrintOemUni::EnablePDEV method.

    Please refer to DDK documentation for more details.

Arguments:

    IN pdevobj - pointer to a DEVOBJ structure.

Return Value:

    NONE

--*/

{
    OEMDBG(DBG_VERBOSE, L"OEMDisablePDEV entry.");
    
    // Release our COemPDEV instance created by the call to
    // EnablePDEV.
    //
    POEMPDEV pOemPDEV = (POEMPDEV)pdevobj->pdevOEM;
    delete pOemPDEV;
    pOemPDEV = NULL;
}


