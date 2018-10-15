/******************************Module*Header*******************************\
*
*                           *******************
*                           * D3D SAMPLE CODE *
*                           *******************
*
* Module Name: glint.h
*
* Content: DX Driver high level definitions and includes
*
* Copyright (c) 1994-1999 3Dlabs Inc. Ltd. All rights reserved.
* Copyright (c) 1995-2002 Microsoft Corporation.  All rights reserved.
\*****************************************************************************/
#ifndef __GLINT_H
#define __GLINT_H

//*****************************************************
// DRIVER FEATURES CONTROLLED BY DEFINED SYMBOLS
//*****************************************************

#ifndef COMPILE_AS_DX7_DRIVER

#ifndef COMPILE_AS_DX8_DRIVER
#pragma message("COMPILE_AS_DX9_DRIVER")
#define COMPILE_AS_DX9_DRIVER   1
#else
#pragma message("COMPILE_AS_DX8_DRIVER")
#undef COMPILE_AS_DX9_DRIVER
#endif

#else

#pragma message("COMPILE_AS_DX7_DRIVER")
#undef COMPILE_AS_DX8_DRIVER
#undef COMPILE_AS_DX9_DRIVER

#endif // COMPILE_AS_DX7_DRIVER

#if COMPILE_AS_DX9_DRIVER

#define DIRECT3D_VERSION        0x0900

#define DX9_DDI                 1
#define DX9_ANTIALIASEDLINE     1
#define DX9_SCISSORRECT         1
#define DX9_SETSTREAMSOURCE2    1
#define DX9_DP2BLT              1
#define DX9_RT_COLORCONVERT     1
#undef  DX9_ASYNC_NOTIFICATIONS
#undef  DX9_EVENT_QUERY
#define DX9_LWMIPMAP            1
#define DX9_AUTOGENMIPMAP       1

#if DX9_AUTOGENMIPMAP
#define DX9_LWMIPMAP            1
#endif // DX9_AUTOGENMIPMAP

#if DX9_DDI
// Canceled from DX9
// #define DX9_RTZMAN           1
#endif // DX9_DDI

#if DX9_RTZMAN
#define DX7_TEXMANAGEMENT       1
#endif

#define COMPILE_AS_DX8_DRIVER   1
#ifdef DX9_SETSTREAMSOURCE2
#define DX8_MULTSTREAMS         1
#endif // DX9_SETSTREAMSOURCE2

#endif // COMPILE_AS_DX9_DRIVER

#if COMPILE_AS_DX8_DRIVER

#ifndef DIRECT3D_VERSION
#define DIRECT3D_VERSION        0x0800 
#endif // DIRECT3D_VERSION

// DX8_DDI is 1 if the driver is going to advertise itself as a DX8 driver to
// the runtime. Notice that if we are a DX8 driver, we HAVE TO support the
// multisteraming command tokens and a limited semantic of the vertex shader
// tokens to interpret correctly the VertexType being processed
#define DX8_DDI           1
#define DX8_MULTSTREAMS   1
#define DX8_VERTEXSHADERS 1
// These other #defines enable and disable specific DX8 features
// they are included mainly in order to help driver writers locate most of the
// the code specific to the named features. Pixel shaders can't be supported on
// this hardware, only stub functions are offered.
#define DX8_POINTSPRITES  1
#define DX8_PIXELSHADERS  1
#define DX8_3DTEXTURES    1

#define DX8_PRIVTEXFORMAT 1

#if WNT_DDRAW
#define DX8_MULTISAMPLING 1
#else
// On Win9x, AA buffers must be released during Alt-tab switch. Since 
// Perm3 driver doesn't share the D3D context with the 16bit side, this
// feature is disabled to prevent corrupted rendering.
#endif

// 8.1 Feature to enable backbuffers with alpha channels
#define DX8_FULLSCREEN_FLIP_OR_DISCARD 1

#else

#define DIRECT3D_VERSION  0x0700 
#define DX8_DDI           0
#define DX8_MULTSTREAMS   0
#define DX8_VERTEXSHADERS 0
#define DX8_POINTSPRITES  0
#define DX8_PIXELSHADERS  0
#define DX8_3DTEXTURES    0
#define DX8_MULTISAMPLING 0
#define DX8_FULLSCREEN_FLIP_OR_DISCARD 0
#endif // COMPILE_AS_DX8_DRIVER

// DX7 features which have been highlighted in order to 
// ease their implementation for other hardware parts.
#if WNT_DDRAW
#define DX7_ANTIALIAS      1
#else
// On Win9x, AA buffers must be released during Alt-tab switch. Since 
// Perm3 driver doesn't share the D3D context with the 16bit side, this
// feature is disabled to prevent corrupted rendering.
#endif

#define DX7_D3DSTATEBLOCKS 1
#define DX7_PALETTETEXTURE 1
#define DX7_STEREO         1

// Texture management enables DX8 resource management features
#define DX7_TEXMANAGEMENT  1


// The below symbol is used only to ifdef code which is demonstrative for 
// other DX drivers but which for specific reasons is not part of the 
// current sample driver
#define DX_NOT_SUPPORTED_FEATURE 0

#if DX7_D3DSTATEBLOCKS 
// These #defines bracket code or comments which would be important for 
// TnL capable / pixel shader capable / vertex shader capable parts 
// when processing state blocks commands. 
#define DX7_SB_TNL            0
#define DX8_SB_SHADERS        0
#endif // DX7_D3DSTATEBLOCKS 

//*****************************************************
// PORTING WIN9X-WINNT 
//*****************************************************

// On IA64 , the following Macro sorts out the PCI bus caching problem.
// X86 doesn't need this, but it is handled by the same macro defined in
// ioaccess.h on Win2K/XP. For Win9x we need to define it ourselves as 
// doing nothing.

#if W95_DDRAW
#define MEMORY_BARRIER()
#endif


#if WNT_DDRAW
#define WANT_DMA 1
#endif // WNT_DDRAW

//*****************************************************
// INCLUDE FILES FOR ALL
//*****************************************************


#if DX9_DDI
#include <d3d9.h>
#if WNT_DDRAW
#include <d3d8caps.h>
#endif
#elif DX8_DDI
#include <d3d8.h>
#else
#include <d3d.h>
#endif


#if WNT_DDRAW
#include <stddef.h>
#include <windows.h>

#include <winddi.h>      // This includes d3dnthal.h and ddrawint.h
#include <devioctl.h>
#include <ntddvdeo.h>

#include <ioaccess.h>

#define DX8DDK_DX7HAL_DEFINES
#include <dx95type.h>    // For Win 2000 include dx95type which allows 
                         // us to work almost as if we were on Win9x

#include <math.h>

#else   //  WNT_DDRAW

// These need to be included in Win9x

#include <windows.h>
#include <ddraw.h>

#ifndef __cplusplus
#include <dciman.h>
#endif

#include <ddrawi.h>

#ifdef __cplusplus
#include <dciman.h> // dciman.h must be included before ddrawi.h, 
#endif              // and it needs windows.h

#include <d3dhal.h>

typedef struct tagLinearAllocatorInfo LinearAllocatorInfo, *pLinearAllocatorInfo;

#endif  //  WNT_DDRAW

#if DX8_DDI
// This include file has some utility macros to process
// the new GetDriverInfo2 GUID calls
#include <d3dhalex.h>
#endif

// Our drivers include files 
#include "driver.h"
#include "debug.h"
#include "softcopy.h"
#include "glglobal.h"
#include "glintreg.h"
#include "d3dstrct.h"
#include "linalloc.h"
#include "glddtk.h"
#include "directx.h"
#include "bitmac2.h"
#include "direct3d.h"
#include "dcontext.h"
#include "d3dsurf.h"
#include "dltamacr.h"

//*****************************************************
// TEXTURE MANAGEMENT DEFINITIONS
//*****************************************************
#if DX7_TEXMANAGEMENT
#if COMPILE_AS_DX8_DRIVER
// We will only collect stats in the DX7 driver
#define DX7_TEXMANAGEMENT_STATS 0
#else
#define DX7_TEXMANAGEMENT_STATS 1
#endif // COMPILE_AS_DX8_DRIVER

#include "d3dtxman.h"

#endif // DX7_TEXMANAGEMENT

#endif __GLINT_H


