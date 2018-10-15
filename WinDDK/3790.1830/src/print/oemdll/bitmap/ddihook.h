//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright  1998 - 2003  Microsoft Corporation.  All Rights Reserved.
//
//  FILE:    DDIHook.h
//    
//
//  PURPOSE:  Defines the constants used to control DDI hooking.
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

// ==================================================================
// Uncomment any of the following to implement the corresponding
// DDI call. Note that these are the only DDIs that may be hooked
// in this version of UNIDRV.
//
// For this bitmap driver, all we really want to implement is OEMEndDoc. 
// So we comment out the rest of the directives.
//
//  #define IMPL_ALPHABLEND
//  #define IMPL_BITBLT
//  #define IMPL_COPYBITS
//  #define IMPL_DITHERCOLOR
    #define IMPL_ENDDOC
//  #define IMPL_ESCAPE
//  #define IMPL_FILLPATH
//  #define IMPL_FONTMANAGEMENT
//  #define IMPL_GETGLYPHMODE
//  #define IMPL_GRADIENTFILL
//  #define IMPL_LINETO
//  #define IMPL_NEXTBAND
//  #define IMPL_PAINT
//  #define IMPL_PLGBLT
//  #define IMPL_QUERYADVANCEWIDTHS
//  #define IMPL_QUERYFONT
//  #define IMPL_QUERYFONTDATA
//  #define IMPL_QUERYFONTTREE
//  #define IMPL_REALIZEBRUSH
//  #define IMPL_SENDPAGE
//  #define IMPL_STARTBANDING
//  #define IMPL_STARTDOC
//  #define IMPL_STARTPAGE
//  #define IMPL_STRETCHBLT
//  #define IMPL_STRETCHBLTROP
//  #define IMPL_STROKEANDFILLPATH
//  #define IMPL_STROKEPATH
//  #define IMPL_TEXTOUT
//  #define IMPL_TRANSPARENTBLT


// Combine the constants above; there are a few places in which
// we need to test if any DDIs have been implemented (as a 
// convenience).
//
#if defined(IMPL_ALPHABLEND)                || \
    defined(IMPL_BITBLT)                    || \
    defined(IMPL_COPYBITS)              || \
    defined(IMPL_DITHERCOLOR)           || \
    defined(IMPL_ENDDOC)                    || \
    defined(IMPL_ESCAPE)                    || \
    defined(IMPL_FILLPATH)                  || \
    defined(IMPL_FONTMANAGEMENT)        || \
    defined(IMPL_GETGLYPHMODE)          || \
    defined(IMPL_GRADIENTFILL)          || \
    defined(IMPL_LINETO)                    || \
    defined(IMPL_NEXTBAND)              || \
    defined(IMPL_PAINT)                 || \
    defined(IMPL_PLGBLT)                    || \
    defined(IMPL_QUERYADVANCEWIDTHS)    || \
    defined(IMPL_QUERYFONT)             || \
    defined(IMPL_QUERYFONTDATA)         || \
    defined(IMPL_QUERYFONTTREE)         || \
    defined(IMPL_REALIZEBRUSH)          || \
    defined(IMPL_SENDPAGE)              || \
    defined(IMPL_STARTBANDING)          || \
    defined(IMPL_STARTDOC)              || \
    defined(IMPL_STARTPAGE)             || \
    defined(IMPL_STRETCHBLT)                || \
    defined(IMPL_STRETCHBLTROP)         || \
    defined(IMPL_STROKEANDFILLPATH)     || \
    defined(IMPL_STROKEPATH)                || \
    defined(IMPL_TEXTOUT)                   || \
    defined(IMPL_TRANSPARENTBLT)

    #define DDIS_HAVE_BEEN_IMPL
#endif


