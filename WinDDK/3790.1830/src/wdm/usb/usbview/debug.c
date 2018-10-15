/*++

Copyright (c) 1997-1998 Microsoft Corporation

Module Name:

    DEBUG.C

Abstract:

    This source file contains debug routines.

Environment:

    user mode

Revision History:

    07-08-97 : created

--*/

//*****************************************************************************
// I N C L U D E S
//*****************************************************************************

#include <windows.h>
#include <basetyps.h>
#include <winioctl.h>

#include "usbview.h"

#if DBG

//*****************************************************************************
// T Y P E D E F S
//*****************************************************************************

typedef struct _ALLOCHEADER
{
    LIST_ENTRY  ListEntry;

    PCHAR       File;

    ULONG       Line;

} ALLOCHEADER, *PALLOCHEADER;


//*****************************************************************************
// G L O B A L S
//*****************************************************************************

LIST_ENTRY AllocListHead =
{
    &AllocListHead,
    &AllocListHead
};


//*****************************************************************************
//
// MyAlloc()
//
//*****************************************************************************

HGLOBAL
MyAlloc (
    PCHAR   File,
    ULONG   Line,
    DWORD   dwBytes
)
{
    PALLOCHEADER header;

    if (dwBytes)
    {
        dwBytes += sizeof(ALLOCHEADER);

        header = (PALLOCHEADER)GlobalAlloc(GPTR, dwBytes);

        if (header != NULL)
        {
            InsertTailList(&AllocListHead, &header->ListEntry);

            header->File = File;
            header->Line = Line;

            return (HGLOBAL)(header + 1);
        }
    }

    return NULL;
}

//*****************************************************************************
//
// MyReAlloc()
//
//*****************************************************************************

HGLOBAL
MyReAlloc (
    HGLOBAL hMem,
    DWORD   dwBytes
)
{
    PALLOCHEADER header;
    PALLOCHEADER headerNew;

    if (hMem)
    {
        header = (PALLOCHEADER)hMem;

        header--;

        // Remove the old address from the allocation list
        //
        RemoveEntryList(&header->ListEntry);

        headerNew = GlobalReAlloc((HGLOBAL)header, dwBytes, GMEM_MOVEABLE|GMEM_ZEROINIT);

        if (headerNew != NULL)
        {
            // Add the new address to the allocation list
            //
            InsertTailList(&AllocListHead, &headerNew->ListEntry);

            return (HGLOBAL)(headerNew + 1);
        }
        else
        {
            // If GlobalReAlloc fails, the original memory is not freed,
            // and the original handle and pointer are still valid.
            // Add the old address back to the allocation list.
            //
            InsertTailList(&AllocListHead, &header->ListEntry);
        }

    }

    return NULL;
}


//*****************************************************************************
//
// MyFree()
//
//*****************************************************************************

HGLOBAL
MyFree (
    HGLOBAL hMem
)
{
    PALLOCHEADER header;

    if (hMem)
    {
        header = (PALLOCHEADER)hMem;

        header--;

        RemoveEntryList(&header->ListEntry);

        return GlobalFree((HGLOBAL)header);
    }

    return GlobalFree(hMem);
}

//*****************************************************************************
//
// MyCheckForLeaks()
//
//*****************************************************************************

VOID
MyCheckForLeaks (
    VOID
)
{
    PALLOCHEADER header;
    CHAR         buf[128];

    while (!IsListEmpty(&AllocListHead))
    {
        header = (PALLOCHEADER)RemoveHeadList(&AllocListHead);

        wsprintf(buf,
                 "File: %s, Line: %d",
                 header->File,
                 header->Line);

        MessageBox(NULL, buf, "USBView Memory Leak", MB_OK);
    }
}

#endif

