/******************************Module*Header**********************************\
*
*                           *******************
*                           * D3D SAMPLE CODE *
*                           *******************
*
* Module Name: d3dtxman.c
*
* Content:  D3D Texture cache manager
*
* Copyright (c) 1995-2002 Microsoft Corporation.  All rights Reserved.
\*****************************************************************************/
#include "glint.h"
#include "dma.h"

#if DX7_TEXMANAGEMENT

//-----------------------------------------------------------------------------
// The driver can optionally manage textures which have been marked as 
// manageable. These DirectDraw surfaces are marked as manageable with the 
// DDSCAPS2_TEXTUREMANAGE flag in the dwCaps2 field of the structure refered to 
// by lpSurfMore->ddCapsEx.
//
// The driver makes explicit its support for driver-managed textures by setting 
// DDCAPS2_CANMANAGETEXTURE in the dwCaps2 field of the DD_HALINFO structure. 
// DD_HALINFO is returned in response to the initialization of the DirectDraw 
// component of the driver, DrvGetDirectDrawInfo.
//
// The driver can then create the necessary surfaces in video or non-local 
// memory in a lazy fashion. That is, the driver leaves them in system memory 
// until it requires them, which is just before rasterizing a primitive that 
// makes use of the texture.
//
// Surfaces should be evicted primarily by their priority assignment. The driver 
// should respond to the D3DDP2OP_SETPRIORITY token in the D3dDrawPrimitives2 
// command stream, which sets the priority for a given surface. As a secondary 
// measure, it is expected that the driver will use a least recently used (LRU) 
// scheme. This scheme should be used as a tie breaker, whenever the priority of 
// two or more textures is identical in a particular scenario. Logically, any 
// surface that is in use shouldn't be evicted at all.
//
// The driver must be cautious of honoring DdBlt calls and DdLock calls when 
// managing textures. This is because any change to the system memory image of 
// the surface must be propagated into the video memory copy of it before the 
// texture is used again. The driver should determine if it is better to update 
// just a portion of the surface or all of it.
// 
// The driver is allowed to perform texture management in order to perform 
// optimization transformations on the textures or to decide for itself where 
// and when to transfer textures in memory.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Porting to your hardware/driver
//
// The following structures/functions/symbols are specific to this 
// implementation. You should supply your own if needed:
//
//     P3_SURF_INTERNAL
//     P3_D3DCONTEXT
//     DISPDBG
//     HEAP_ALLOC ALLOC_TAG_DX
//     _D3D_TM_HW_FreeVidmemSurface
//     _D3D_TM_HW_AllocVidmemSurface
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global texture management object and ref count
//-----------------------------------------------------------------------------
DWORD g_TMCount = 0;
TextureCacheManager g_TextureManager;

//-----------------------------------------------------------------------------
// Macros and definitions
//-----------------------------------------------------------------------------
// Number of pointers to DX surfaces for first allocated heap
#define TM_INIT_HEAP_SIZE  1024
// Subsequent increments
#define TM_GROW_HEAP_SIZE(n)  ((n)*2)

// Access to binary-tree structure in the heap. The heap is really just a
// linear array of pointers (to P3_SURF_INTERNAL structures) which is
// accessed as if it were a binary tree: m_data_p[1] is the root of the tree
// and the its immediate children are [2] and [3]. The children/parents of 
// element are uniquely defined by the below macros.
#define parent(k) ((k) / 2)
#define lchild(k) ((k) * 2)
#define rchild(k) ((k) * 2 + 1)

//-----------------------------------------------------------------------------
//
// void __TM_TextureHeapFindSlot
//
// Starting at element k in the TM heap, search the heap (towards the root node)
// for an element whose parent is cheaper than llCost. Return the value of it.
//
// An important difference between __TM_TextureHeapFindSlot and
// __TM_TextureHeapHeapify is that the former searches upwards in the tree
// whereas the latter searches downwards through the tree.
//
//-----------------------------------------------------------------------------
DWORD
__TM_TextureHeapFindSlot(
    PTextureHeap pTextureHeap, 
    ULONGLONG llCost,
    DWORD k)
{
    // Starting with element k, traverse the (binary-tree) heap upwards until 
    // you find an element whose parent is less expensive (cost-wise) 
    // than llCost . (Short circuited && expression below!)
    while( (k > 1) &&
           (llCost < TextureCost(pTextureHeap->m_data_p[parent(k)])) )
    {
        // Slot k is assumed to be empty. So since we are looking for
        // slot where to put things, we need to move downwards the
        // parent slot before we go on in order for the new k to be 
        // available
        pTextureHeap->m_data_p[k] = pTextureHeap->m_data_p[parent(k)];
        pTextureHeap->m_data_p[k]->m_dwHeapIndex = k; // update surf reference
        k = parent(k);                                // look now at parent
    }

    return k;
} // __TM_TextureHeapFindSlot

//-----------------------------------------------------------------------------
//
// void __TM_TextureHeapHeapify
//
// Starting at element k in the TM heap, make sure the heap is well-ordered
// (parents are always lower cost than their children). This algorithm assumes
// the heap is well ordered with the possible exception of element k
//
//-----------------------------------------------------------------------------
void 
__TM_TextureHeapHeapify(
    PTextureHeap pTextureHeap, 
    DWORD k)
{
    while(TRUE) 
    {
        DWORD smallest;
        DWORD l = lchild(k);
        DWORD r = rchild(k);

        // Figure out who has the smallest TextureCost, either k,l or r.
        if(l < pTextureHeap->m_next)
        {
            if(TextureCost(pTextureHeap->m_data_p[l]) <
               TextureCost(pTextureHeap->m_data_p[k]))
            {
                smallest = l;
            }
            else
            {
                smallest = k;
            }
        }
        else
        {
            smallest = k;
        }
        
        if(r < pTextureHeap->m_next)
        {
            if(TextureCost(pTextureHeap->m_data_p[r]) <
               TextureCost(pTextureHeap->m_data_p[smallest]))
            {
                smallest = r;
            }
        }
        
        if(smallest != k) 
        {
            // it wasn't k. So now swap k with the smallest of the three
            // and repeat the loop in order with the new position of k 
            // in order to keep the order right (parents are always lower 
            // cost than children)
            P3_SURF_INTERNAL* ptempD3DSurf = pTextureHeap->m_data_p[k];
            pTextureHeap->m_data_p[k] = pTextureHeap->m_data_p[smallest];
            pTextureHeap->m_data_p[k]->m_dwHeapIndex = k;
            pTextureHeap->m_data_p[smallest] = ptempD3DSurf;
            ptempD3DSurf->m_dwHeapIndex = smallest;
            k = smallest;
        }
        else
        {
            // it was k, so the order is now all right, leave here
            break;
        }
    }
} // __TM_TextureHeapHeapify

//-----------------------------------------------------------------------------
//
// BOOL __TM_TextureHeapAddSurface
//
// Add a DX surface to a texture management heap. 
// Returns success or failure status
//
//-----------------------------------------------------------------------------
BOOL 
__TM_TextureHeapAddSurface(
    PTextureHeap pTextureHeap, 
    P3_SURF_INTERNAL* pD3DSurf)
{
    P3_SURF_INTERNAL* *ppD3DSurf;
    ULONGLONG llCost;
    DWORD k;

    if(pTextureHeap->m_next == pTextureHeap->m_size) 
    {   
        // Heap is full, we must grow it
        DWORD dwNewSize = TM_GROW_HEAP_SIZE(pTextureHeap->m_size);

        ppD3DSurf = (P3_SURF_INTERNAL* *)
                          HEAP_ALLOC( FL_ZERO_MEMORY, 
                                       sizeof(P3_SURF_INTERNAL*) * dwNewSize,
                                       ALLOC_TAG_DX(B));
        if(ppD3DSurf == 0)
        {
            DISPDBG((ERRLVL,"Failed to allocate memory to grow heap."));
            return FALSE;
        }

        // Transfer data 
        memcpy(ppD3DSurf + 1, pTextureHeap->m_data_p + 1, 
            sizeof(P3_SURF_INTERNAL*) * (pTextureHeap->m_next - 1));

        // Free previous heap
        HEAP_FREE( pTextureHeap->m_data_p);
        
        // Update texture heap structure    
        pTextureHeap->m_size = dwNewSize;
        pTextureHeap->m_data_p = ppD3DSurf;
    }

    // Get the cost of the surface we are about to add
    llCost = TextureCost(pD3DSurf);

    // Starting at the last element in the heap (where we can theoretically
    // place our new element) search upwards for an appropriate place to put 
    // it in. This will also maintain our tree/heap balanced 
    k = __TM_TextureHeapFindSlot(pTextureHeap, llCost, pTextureHeap->m_next);

    // Add the new surface to the heap in the [k] place
    pTextureHeap->m_data_p[k] = pD3DSurf;
    ++pTextureHeap->m_next;    

    //Update the surface's reference to its place on the heap
    pD3DSurf->m_dwHeapIndex = k;
    
    return TRUE;
    
} // __TM_TextureHeapAddSurface

//-----------------------------------------------------------------------------
//
// void __TM_TextureHeapDelSurface
//
// Delete the k element from the TM heap
//
//-----------------------------------------------------------------------------
void __TM_TextureHeapDelSurface(PTextureHeap pTextureHeap, DWORD k)
{
    P3_SURF_INTERNAL* pD3DSurf = pTextureHeap->m_data_p[k];
    ULONGLONG llCost;

    // (virtually) delete the heaps last element and get its cost
    --pTextureHeap->m_next;
    llCost = TextureCost(pTextureHeap->m_data_p[pTextureHeap->m_next]);
    
    if(llCost < TextureCost(pD3DSurf))
    {
        // If the cost of the last element is less than that of the surface
        // we are really trying to delete (k), look for a new place where
        // to put the m_next element based on its cost.
    
        // Starting at the k element in the heap (where we can theoretically
        // place our new element) search upwards for an appropriate place to 
        // put it in
        k = __TM_TextureHeapFindSlot(pTextureHeap, llCost, k);

        // Overwrite the data of k with the data of the last element
        pTextureHeap->m_data_p[k] = pTextureHeap->m_data_p[pTextureHeap->m_next];
        pTextureHeap->m_data_p[k]->m_dwHeapIndex = k;
    }
    else
    {
        // If the cost of the last element is greather than that of the surface
        // we are really trying to delete (k), replace (k) by (m_next)
        pTextureHeap->m_data_p[k] = pTextureHeap->m_data_p[pTextureHeap->m_next];
        pTextureHeap->m_data_p[k]->m_dwHeapIndex = k;

        // Now make sure we keep the heap correctly ordered        
        __TM_TextureHeapHeapify(pTextureHeap,k);
    }
    
    pD3DSurf->m_dwHeapIndex = 0;
    
} // __TM_TextureHeapDelSurface


//-----------------------------------------------------------------------------
//
// P3_SURF_INTERNAL* __TM_TextureHeapExtractMin
//
// Extract
//
//-----------------------------------------------------------------------------
P3_SURF_INTERNAL* 
__TM_TextureHeapExtractMin(
    PTextureHeap pTextureHeap)
{
    // Obtaint pointer to surface we are extracting from the heap
    // (the root node, which is the least expensive element of the
    // whole heap because of the way we build the heap).
    P3_SURF_INTERNAL* pD3DSurf = pTextureHeap->m_data_p[1];

    // Update heap internal counter and move last 
    // element now to first position.
    --pTextureHeap->m_next;
    pTextureHeap->m_data_p[1] = pTextureHeap->m_data_p[pTextureHeap->m_next];
    pTextureHeap->m_data_p[1]->m_dwHeapIndex = 1;

    // Now make sure we keep the heap correctly ordered
    __TM_TextureHeapHeapify(pTextureHeap,1);

    // Clear the deleted surface's reference to its place on the heap (deleted)
    pD3DSurf->m_dwHeapIndex = 0;
    
    return pD3DSurf;
    
} // __TM_TextureHeapExtractMin

//-----------------------------------------------------------------------------
//
// P3_SURF_INTERNAL* __TM_TextureHeapExtractMax
//
//-----------------------------------------------------------------------------
P3_SURF_INTERNAL* 
__TM_TextureHeapExtractMax(
    PTextureHeap pTextureHeap)
{
    // When extracting the max element from the heap, we don't need to
    // search the entire heap, but just the leafnodes. This is because
    // it is guaranteed that parent nodes are cheaper than the leaf nodes
    // so once you have looked through the leaves, you won't find anything
    // cheaper. 
    // NOTE: (lchild(i) >= m_next) is true only for leaf nodes.
    // ALSO NOTE: You cannot have a rchild without a lchild, so simply
    //            checking for lchild is sufficient.
    unsigned max = pTextureHeap->m_next - 1;
    ULONGLONG llMaxCost = 0;
    ULONGLONG llCost;
    unsigned i;
    P3_SURF_INTERNAL* pD3DSurf;

    // Search all the terminal nodes of the binary-tree (heap)
    // for the most expensive element and store its index in max
    for(i = max; lchild(i) >= pTextureHeap->m_next; --i)
    {
        llCost = TextureCost(pTextureHeap->m_data_p[i]);
        if(llMaxCost < llCost)
        {
            llMaxCost = llCost;
            max = i;
        }
    }

    // Return the surface associated to this maximum cost heap element 
    pD3DSurf = pTextureHeap->m_data_p[max];

    // Delete it from the heap ( will automatically maintain heap ordered)
    __TM_TextureHeapDelSurface(pTextureHeap, max);
    
    return pD3DSurf;
    
} // __TM_TextureHeapExtractMax

//-----------------------------------------------------------------------------
//
// void __TM_TextureHeapUpdate
//
// Updates the priority & number of of ticks of surface # k in the heap
//
//-----------------------------------------------------------------------------
void 
__TM_TextureHeapUpdate(
    PTextureHeap pTextureHeap, 
    DWORD k,
    DWORD dwPriority, 
    DWORD dwTicks) 
{
    P3_SURF_INTERNAL* pD3DSurf = pTextureHeap->m_data_p[k];
    ULONGLONG llCost = 0;
#ifdef _X86_
    _asm
    {
        mov     edx, 0;
        shl     edx, 31;
        mov     eax, dwPriority;
        mov     ecx, eax;
        shr     eax, 1;
        or      edx, eax;
        mov     DWORD PTR llCost + 4, edx;
        shl     ecx, 31;
        mov     eax, dwTicks;
        shr     eax, 1;
        or      eax, ecx;
        mov     DWORD PTR llCost, eax;
    }
#else
    llCost = ((ULONGLONG)dwPriority << 31) + ((ULONGLONG)(dwTicks >> 1));
#endif
    if(llCost < TextureCost(pD3DSurf))
    {
        // Starting at the k element in the heap (where we can theoretically
        // place our new element) search upwards for an appropriate place 
        // to move it to in order to keep the tree well ordered.
        k = __TM_TextureHeapFindSlot(pTextureHeap, llCost, k);
        
        pD3DSurf->m_dwPriority = dwPriority;
        pD3DSurf->m_dwTicks = dwTicks;
        pD3DSurf->m_dwHeapIndex = k;
        pTextureHeap->m_data_p[k] = pD3DSurf;
    }
    else
    {
        pD3DSurf->m_dwPriority = dwPriority;
        pD3DSurf->m_dwTicks = dwTicks;

        // Now make sure we keep the heap correctly ordered        
        __TM_TextureHeapHeapify(pTextureHeap,k);
    }
}

//-----------------------------------------------------------------------------
//
// BOOL __TM_FreeTextures
//
// Free the LRU texture 
//
//-----------------------------------------------------------------------------
BOOL 
__TM_FreeTextures(
    P3_THUNKEDDATA* pThisDisplay,
    DWORD dwBytes)
{
    P3_SURF_INTERNAL* pD3DSurf;
    DWORD k;

    PTextureCacheManager pTextureCacheManager =  pThisDisplay->pTextureManager;
    
    // No textures left to be freed
    if(pTextureCacheManager->m_heap.m_next <= 1)
        return FALSE;

    // Keep remove textures until we accunulate dwBytes of removed stuff
    // or we have no more surfaces left to be removed.
    for(k = 0; 
        (pTextureCacheManager->m_heap.m_next > 1) && (k < dwBytes); 
        k += pD3DSurf->m_dwBytes)
    {
        // Find the LRU texture (the one with lowest cost) and remove it.
        pD3DSurf = __TM_TextureHeapExtractMin(&pTextureCacheManager->m_heap);
        _D3D_TM_RemoveTexture(pThisDisplay, pD3DSurf, FALSE);

#if DX7_TEXMANAGEMENT_STATS
        // Update stats
        pTextureCacheManager->m_stats.dwLastPri = pD3DSurf->m_dwPriority;
        ++pTextureCacheManager->m_stats.dwNumEvicts;
#endif        
        
        DISPDBG((WRNLVL, "Removed texture with timestamp %u,%u (current = %u).", 
                          pD3DSurf->m_dwPriority, 
                          pD3DSurf->m_dwTicks, 
                          pTextureCacheManager->tcm_ticks));
    }
    
    return TRUE;
    
} // __TM_FreeTextures

//-----------------------------------------------------------------------------
//
// HRESULT __TM_TextureHeapInitialize
//
// Allocate the heap and initialize
//
//-----------------------------------------------------------------------------
HRESULT 
__TM_TextureHeapInitialize(
    PTextureCacheManager pTextureCacheManager)
{
    pTextureCacheManager->m_heap.m_next = 1;
    pTextureCacheManager->m_heap.m_size = TM_INIT_HEAP_SIZE;
    pTextureCacheManager->m_heap.m_data_p = (P3_SURF_INTERNAL* *)
        HEAP_ALLOC( FL_ZERO_MEMORY, 
                     sizeof(P3_SURF_INTERNAL*) * 
                        pTextureCacheManager->m_heap.m_size,
                     ALLOC_TAG_DX(C));
            
    if(pTextureCacheManager->m_heap.m_data_p == 0)
    {
        DISPDBG((ERRLVL,"Failed to allocate texture heap."));
        return E_OUTOFMEMORY;
    }
    
    memset(pTextureCacheManager->m_heap.m_data_p, 
           0, 
           sizeof(P3_SURF_INTERNAL*) * pTextureCacheManager->m_heap.m_size);
        
    return DD_OK;
    
} // __TM_TextureHeapInitialize


//-----------------------------------------------------------------------------
//
// HRESULT _D3D_TM_Initialize
//
// Initialize texture management for this display device
// Should be called when the device is being created
//
//-----------------------------------------------------------------------------
HRESULT 
_D3D_TM_Initialize(
    P3_THUNKEDDATA* pThisDisplay)
{

    HRESULT hr = DD_OK;
    
    if (0 == g_TMCount)
    {
        // first use - must initialize
        hr = __TM_TextureHeapInitialize(&g_TextureManager);

        // init ticks count for LRU policy
        g_TextureManager.tcm_ticks = 0;
    }

    if (SUCCEEDED(hr))
    {   
        // Initialization succesful or uneccesary.
        // Increment the reference count and make the context 
        // remember where the texture management object is.
        g_TMCount++;
        pThisDisplay->pTextureManager = &g_TextureManager;
    }

    return hr;
    
} //  _D3D_TM_Initialize

//-----------------------------------------------------------------------------
//
// void _D3D_TM_Destroy
//
// Clean up texture management for this display device 
// Should be called when the device is being destroyed
//
//-----------------------------------------------------------------------------
void 
_D3D_TM_Destroy(    
    P3_THUNKEDDATA* pThisDisplay)
{
    // clean up texture manager stuff if it 
    // is already allocated for this context
    if (pThisDisplay->pTextureManager)
    {
        // Decrement reference count for the 
        // driver global texture manager object
        g_TMCount--;

        // If necessary, deallocate the texture manager heap;
        if (0 == g_TMCount)
        {
            if (0 != g_TextureManager.m_heap.m_data_p)
            {
                _D3D_TM_EvictAllManagedTextures(pThisDisplay);
                HEAP_FREE(g_TextureManager.m_heap.m_data_p);
                g_TextureManager.m_heap.m_data_p = NULL;
            }
        }

        pThisDisplay->pTextureManager = NULL;        
    }
} // _D3D_TM_Destroy

//-----------------------------------------------------------------------------
//
// HRESULT _D3D_TM_AllocTexture
//
// add a new HW handle and create a surface (for a managed texture) in vidmem
//
//-----------------------------------------------------------------------------
HRESULT 
_D3D_TM_AllocTexture(
    P3_THUNKEDDATA* pThisDisplay,
    P3_SURF_INTERNAL* pTexture)
{
    DWORD trycount = 0, bytecount = pTexture->m_dwBytes;
    PTextureCacheManager pTextureCacheManager = pThisDisplay->pTextureManager;
    INT iLOD;

    // Decide the largest level to allocate video memory based on what
    // is specified through D3DDP2OP_SETTEXLOD token
    iLOD = pTexture->m_dwTexLOD;
    if (iLOD > (pTexture->iMipLevels - 1))
    {
        iLOD = pTexture->iMipLevels - 1;
    }        

    // Attempt to allocate a texture. (do until the texture is in vidmem)
    while((FLATPTR)NULL == pTexture->MipLevels[iLOD].fpVidMemTM)
    {  
        _D3D_TM_HW_AllocVidmemSurface(pThisDisplay, pTexture);
        ++trycount;
                              
        DISPDBG((DBGLVL,"Got fpVidMemTM = %08lx",
                        pTexture->MipLevels[0].fpVidMemTM));

        // We were able to allocate the vidmem surface
        if ((FLATPTR)NULL != pTexture->MipLevels[iLOD].fpVidMemTM)
        {
            // No problem, there is enough memory. 
            pTexture->m_dwTicks = pTextureCacheManager->tcm_ticks;

            // Add texture to manager's heap to track it
            if(!__TM_TextureHeapAddSurface(&pTextureCacheManager->m_heap,
                                           pTexture))
            {          
                // Failed - undo vidmem allocation
                // This call will set all MipLevels[i].fpVidMemTM to NULL
                _D3D_TM_HW_FreeVidmemSurface(pThisDisplay, pTexture);                                           
                
                DISPDBG((ERRLVL,"Out of memory"));
                return DDERR_OUTOFMEMORY;
            }

            // Mark surface as needing update from sysmem before use
            pTexture->m_bTMNeedUpdate = TRUE;
            break;
        }
        else
        {
            // we weren't able to allocate the vidmem surface
            // we will now try to free some managed surfaces to make space
            if (!__TM_FreeTextures(pThisDisplay, bytecount))
            {
                DISPDBG((ERRLVL,"all Freed no further video memory available"));
                return DDERR_OUTOFVIDEOMEMORY; //nothing left
            }
            
            bytecount <<= 1;
        }
    }

    if(trycount > 1)
    {
        DISPDBG((DBGLVL, "Allocated texture after %u tries.", trycount));
    }
    
    __TM_STAT_Inc_TotSz(pTextureCacheManager, pTexture);
    __TM_STAT_Inc_WrkSet(pTextureCacheManager, pTexture);

#if DX7_TEXMANAGEMENT_STATS    
    ++pTextureCacheManager->m_stats.dwNumVidCreates;
#endif // DX7_TEXMANAGEMENT_STATS    
    
    return DD_OK;
    
} // _D3D_TM_AllocTexture

#if DX9_RTZMAN
//-----------------------------------------------------------------------------
//
// void _TM_SwapOutRTZSurf
//
// Swap out the RTZ from the video memory
//
//-----------------------------------------------------------------------------
void
_TM_SwapOutRTZSurf(
    P3_THUNKEDDATA *pThisDisplay,
    P3_SURF_INTERNAL* pTexture)
{
    RECTL rect;
    FLATPTR pCurProcMappedBase;
    VID_MEM_SWAP_MANAGER *pVMSMgr = pThisDisplay->pVidMemSwapMgr;

    if (! pVMSMgr->Perm3MapVidMemSwap(pVMSMgr, (PVOID *)&pCurProcMappedBase))
    {
        return;
    }

    rect.left = rect.top = 0;
    rect.right = pTexture->wWidth;
    rect.bottom = pTexture->wHeight;

    _DD_BLT_SysMemToSysMemCopy(
        D3DTMMIPLVL_GETPOINTER((&pTexture->MipLevels[0]), pThisDisplay),
        pTexture->lPitch,
        pTexture->dwBitDepth,
        pCurProcMappedBase + (ULONG)pTexture->MipLevels[0].fpVidMem,
        pTexture->lPitch,
        pTexture->dwBitDepth,
        &rect,
        &rect);

} // _TM_SwapOutRTZSurf
#endif // DX9_RTZMAN

//-----------------------------------------------------------------------------
//
// void _D3D_TM_RemoveTexture
//
// remove all HW handles and release the managed surface 
// (usually done for every surface in vidmem when D3DDestroyDDLocal is called)
//
//-----------------------------------------------------------------------------
void 
_D3D_TM_RemoveTexture(
    P3_THUNKEDDATA *pThisDisplay,
    P3_SURF_INTERNAL* pTexture,
    BOOL bDestroy)
{
    PTextureCacheManager pTextureCacheManager =  pThisDisplay->pTextureManager; 
    int i;
 
    // Check if surface is currently in video memory
    for (i = 0; i < pTexture->iMipLevels; i++)
    {
        if (pTexture->MipLevels[i].fpVidMemTM != (FLATPTR)NULL)
        {
#if DX9_RTZMAN
            // Swap out the RTZ from the video memory
            if ((! bDestroy) &&
                (! pTexture->m_bTMNeedUpdate) &&
                (pTexture->ddsCapsInt.dwCaps & (DDSCAPS_3DDEVICE | 
                                                DDSCAPS_ZBUFFER)))
            {
                _TM_SwapOutRTZSurf(pThisDisplay, pTexture);
            }
#endif

            // Free (deallocate) the surface from video memory
            // and mark the texture as not longer in vidmem
            _D3D_TM_HW_FreeVidmemSurface(pThisDisplay, pTexture);

            // Update statistics
            __TM_STAT_Dec_TotSz(pTextureCacheManager, pTexture);
            __TM_STAT_Dec_WrkSet(pTextureCacheManager, pTexture);        

            // The job is done
            break;
        }
    }

    // Remove heap references to this surface
    if (pTexture->m_dwHeapIndex && pTextureCacheManager->m_heap.m_data_p)
    {
        __TM_TextureHeapDelSurface(&pTextureCacheManager->m_heap,
                                   pTexture->m_dwHeapIndex); 
    }
    
} // _D3D_TM_RemoveTexture

//-----------------------------------------------------------------------------
//
// void _D3D_TM_RemoveDDSurface
//
// remove all HW handles and release the managed surface 
// (usually done for every surface in vidmem when D3DDestroyDDLocal is called)
//
//-----------------------------------------------------------------------------
void 
_D3D_TM_RemoveDDSurface(
    P3_THUNKEDDATA *pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pDDSLcl,
    BOOL bDestroy)
{
    // We don't know which D3D context this is so we have to do a search 
    // through them (if there are any at all)
    if (pThisDisplay->pDirectDrawLocalsHashTable != NULL)
    {
        DWORD dwSurfaceHandle = pDDSLcl->lpSurfMore->dwSurfaceHandle;
        PointerArray* pSurfaceArray;
       
        // Get a pointer to an array of surface pointers associated to this lpDD
        // The PDD_DIRECTDRAW_LOCAL was stored at D3DCreateSurfaceEx call time
        // in PDD_SURFACE_LOCAL->dwReserved1 
        pSurfaceArray = (PointerArray*)
                            HT_GetEntry(pThisDisplay->pDirectDrawLocalsHashTable,
                                        pDDSLcl->dwReserved1);

        if (pSurfaceArray)
        {
            // Found a surface array associated to this lpDD !
            P3_SURF_INTERNAL* pSurfInternal;

            // Check the surface in this array associated to this surface handle
            pSurfInternal = PA_GetEntry(pSurfaceArray, dwSurfaceHandle);

            if (pSurfInternal)
            {
                // Got it! remove it
                _D3D_TM_RemoveTexture(pThisDisplay, pSurfInternal, bDestroy);
            }
        }                                        
    } 


} // _D3D_TM_RemoveDDSurface

//-----------------------------------------------------------------------------
//
// void _D3D_TM_EvictAllManagedTextures
//
// Remove all managed surfaces from video memory
//
//-----------------------------------------------------------------------------
void 
_D3D_TM_EvictAllManagedTextures(
    P3_THUNKEDDATA* pThisDisplay)
{
    PTextureCacheManager pTextureCacheManager = pThisDisplay->pTextureManager;
    P3_SURF_INTERNAL* pD3DSurf;
    
    while(pTextureCacheManager->m_heap.m_next > 1)
    {
        pD3DSurf = __TM_TextureHeapExtractMin(&pTextureCacheManager->m_heap);
        _D3D_TM_RemoveTexture(pThisDisplay, pD3DSurf, FALSE);
    }
    
    pTextureCacheManager->tcm_ticks = 0;
    
} // _D3D_TM_EvictAllManagedTextures

//-----------------------------------------------------------------------------
//
// void _DD_TM_EvictAllManagedTextures
//
// Remove all managed surfaces from video memory
//
//-----------------------------------------------------------------------------
void 
_DD_TM_EvictAllManagedTextures(
    P3_THUNKEDDATA* pThisDisplay)
{
    PTextureCacheManager pTextureCacheManager = &g_TextureManager;
    P3_SURF_INTERNAL* pD3DSurf;
    
    while(pTextureCacheManager->m_heap.m_next > 1)
    {
        pD3DSurf = __TM_TextureHeapExtractMin(&pTextureCacheManager->m_heap);
        _D3D_TM_RemoveTexture(pThisDisplay, pD3DSurf, FALSE);
    }
    
    pTextureCacheManager->tcm_ticks = 0;
    
} // _DD_TM_EvictAllManagedTextures

//-----------------------------------------------------------------------------
//
// void _D3D_TM_TimeStampTexture
//
//-----------------------------------------------------------------------------
void
_D3D_TM_TimeStampTexture(
    PTextureCacheManager pTextureCacheManager,
    P3_SURF_INTERNAL* pD3DSurf)
{
    __TM_TextureHeapUpdate(&pTextureCacheManager->m_heap,
                           pD3DSurf->m_dwHeapIndex, 
                           pD3DSurf->m_dwPriority, 
                           pTextureCacheManager->tcm_ticks);
                           
    pTextureCacheManager->tcm_ticks += 2;
    
} // _D3D_TM_TimeStampTexture

//-----------------------------------------------------------------------------
//
// void _D3D_TM_HW_FreeVidmemSurface
//
// This is a hw/driver dependent function which takes care of evicting a
// managed texture that's living in videomemory out of it.
// After this all mipmaps fpVidMemTM should be NULL.
//
//-----------------------------------------------------------------------------
void
_D3D_TM_HW_FreeVidmemSurface(
    P3_THUNKEDDATA* pThisDisplay,
    P3_SURF_INTERNAL* pD3DSurf)
{
    INT i, iLimit;

    if (pD3DSurf->bMipMap)
    {
        iLimit = pD3DSurf->iMipLevels;
    }
    else
    {
        iLimit = 1;
    }

    for(i = 0; i < iLimit; i++)
    {
        if (pD3DSurf->MipLevels[i].fpVidMemTM != (FLATPTR)NULL)
        {
           // NOTE: if we weren't managing our own vidmem we would need to
           //       get the address of the VidMemFree callback using 
           //       EngFindImageProcAddress and use this callback into Ddraw to 
           //       do the video memory management. The declaration of 
           //       VidMemFree is
           //
           //       void VidMemFree(LPVMEMHEAP pvmh, FLATPTR ptr);  
           //
           //       You can find more information on this callback in the 
           //       Graphics Drivers DDK documentation           
           
            _DX_LIN_FreeLinearMemory(
                    &pThisDisplay->LocalVideoHeap0Info, 
                    (DWORD)(pD3DSurf->MipLevels[i].fpVidMemTM));

            pD3DSurf->MipLevels[i].fpVidMemTM = (FLATPTR)NULL;                    
        }    
    }
    
} // _D3D_TM_HW_FreeVidmemSurface

//-----------------------------------------------------------------------------
//
// void _D3D_TM_HW_AllocVidmemSurface
//
// This is a hw/driver dependent function which takes care of allocating a
// managed texture that's living only in system memory into videomemory.
// After this fpVidMemTM should not be NULL. This is also the way to
// check if the call failed or was succesful (notice we don't return a
// function result)
//
//-----------------------------------------------------------------------------
void
_D3D_TM_HW_AllocVidmemSurface(
    P3_THUNKEDDATA* pThisDisplay,
    P3_SURF_INTERNAL* pD3DSurf)
{
    INT i, iLimit, iStart;
    
    if (pD3DSurf->bMipMap)
    {
        // Load only the necessary levels given any D3DDP2OP_SETTEXLOD command
        iStart = pD3DSurf->m_dwTexLOD;
        if (iStart > (pD3DSurf->iMipLevels - 1))
        {
            // we should at least load the smallest mipmap if we're loading 
            // the texture into vidmem (and make sure we never try to use any
            // other than these levels), otherwise we won't be able to render            
            iStart = pD3DSurf->iMipLevels - 1;
        }        
    
        iLimit = pD3DSurf->iMipLevels;
    }
    else
    {
        iStart = 0;
        iLimit = 1;
    }

    for(i = iStart; i < iLimit; i++)
    {
        if (pD3DSurf->MipLevels[i].fpVidMemTM == (FLATPTR)NULL)
        {        
           // NOTE: if we weren't managing our own vidmem we would need to
           //       get the address of the HeapVidMemAllocAligned callback 
           //       using EngFindImageProcAddress and use this callback into 
           //       Ddraw to do the video off-screen allocation. The 
           //       declaration of HeapVidMemAllocAligned is
           //
           //       FLATPTR HeapVidMemAllocAligned(LPVIDMEM lpVidMem, 
           //                                      DWORD    dwWidth,
           //                                      DWORD    dwHeight,
           //                                      LPSURFACEALIGNEMENT lpAlign,
           //                                      LPLONG   lpNewPitch);
           //
           //       You can find more information on this callback in the 
           //       Graphics Drivers DDK documentation
           
            P3_MEMREQUEST mmrq;
            DWORD dwResult;
            
            memset(&mmrq, 0, sizeof(P3_MEMREQUEST));
            mmrq.dwSize = sizeof(P3_MEMREQUEST);
            mmrq.dwBytes = pD3DSurf->MipLevels[i].lPitch * 
                           pD3DSurf->MipLevels[i].wHeight;
#if DX8_3DTEXTURES
            if (pD3DSurf->dwCaps2 & DDSCAPS2_VOLUME)
            {
                mmrq.dwBytes *= pD3DSurf->wDepth;
            }
#endif // DX8_3DTEXTURES

            mmrq.dwAlign = 8;
            mmrq.dwFlags = MEM3DL_FIRST_FIT;
            mmrq.dwFlags |= MEM3DL_FRONT;

            dwResult = _DX_LIN_AllocateLinearMemory(
                            &pThisDisplay->LocalVideoHeap0Info,
                            &mmrq);        

            if (dwResult == GLDD_SUCCESS)
            {
                // Record the new vidmem addr for this managed mip level
                pD3DSurf->MipLevels[i].fpVidMemTM = mmrq.pMem;
            }
            else
            {
                // Failure, we'll need to deallocate any mipmap
                // allocated up to this point.
                _D3D_TM_HW_FreeVidmemSurface(pThisDisplay, pD3DSurf);
                
                break; // don't do the loop anymore
            }
        }    
    }

} // _D3D_TM_HW_AllocVidmemSurface

//-----------------------------------------------------------------------------
//
// void _D3D_TM_Preload_SurfInt_IntoVidMem
//
// Transfer a managed surface from system memory into videomemory.
// If the surface still hasn't been allocated video memory we try to do so
// (even evicting uneeded textures if necessary!).
//
//-----------------------------------------------------------------------------

BOOL
_TM_Preload_SurfInt_IntoVidMem(
    P3_THUNKEDDATA* pThisDisplay,
    P3_D3DCONTEXT* pContext,
    P3_SURF_INTERNAL* pD3DSurf)
{
    INT iLOD;

    if (! (pD3DSurf->dwCaps2 & DDSCAPS2_TEXTUREMANAGE))
    {
        DISPDBG((DBGLVL,"Must be a managed texture to do texture preload"));
        return TRUE; // Nothing to preload
    }

    // Decide the largest level to load based on what
    // is specified through D3DDP2OP_SETTEXLOD token
    iLOD = pD3DSurf->m_dwTexLOD;
    if (iLOD > (pD3DSurf->iMipLevels - 1))
    {
        iLOD = pD3DSurf->iMipLevels - 1;
    }
    
    // Check if the largest required mipmap level has been allocated vidmem
    // (only required mipmap levels are ever allocated vidmem)
    if ((FLATPTR)NULL == pD3DSurf->MipLevels[iLOD].fpVidMemTM)
    {
        // add a new HW handle and create a surface 
        // (for a managed texture) in vidmem        
        if ((FAILED(_D3D_TM_AllocTexture(pThisDisplay, pD3DSurf))) ||  
            ((FLATPTR)NULL == pD3DSurf->MipLevels[iLOD].fpVidMemTM))
        {
            DISPDBG((ERRLVL,"_TM_Preload_SurfInt_IntoVidMem unable to "
                            "allocate memory from heap"));
            return FALSE; // OUTOFVIDEOMEMORY
        }
        
        pD3DSurf->m_bTMNeedUpdate = TRUE;
    }
    
    if (pD3DSurf->m_bTMNeedUpdate)
    {
        // texture download   
        DWORD   iLimit, iCurrLOD;

        // Switch to the DirectDraw context if necessary
        if (pContext)
        {
            DDRAW_OPERATION(pContext, pThisDisplay);
        }

        if (pD3DSurf->bMipMap)
        {
            iLimit = pD3DSurf->iMipLevels;
        }
        else
        {
            iLimit = 1;
        }

#if DX9_RTZMAN
        if (! _D3D_TM_GetMipLevelSysMemPtr(pThisDisplay, 
                                           pD3DSurf,
                                           &pD3DSurf->MipLevels[0]))
        {
            return (FALSE);
        }
#endif // DX9_RTZMAN

        // Blt managed texture's required mipmap levels into vid mem
        for (iCurrLOD = iLOD; iCurrLOD < iLimit ; iCurrLOD++)
        {
            RECTL   rect;
            rect.left=rect.top = 0;
            rect.right = pD3DSurf->MipLevels[iCurrLOD].wWidth;
            rect.bottom = pD3DSurf->MipLevels[iCurrLOD].wHeight;

#if DX8_3DTEXTURES
            // Perm3 can only blt 2D images with height up to 4095
            if ((pD3DSurf->dwCaps2 & DDSCAPS2_VOLUME) &&
                ((rect.bottom * pD3DSurf->wDepth) > 4095)) 
            {
                int i;

                for (i = 0; i < pD3DSurf->wDepth; i++)
                {
                    // Download the current slice
                    _DD_P3Download(pThisDisplay,
                                   _D3D_TM_GetMipLevelSysMemPtr(
                                       pThisDisplay,
                                       pD3DSurf,
                                       &pD3DSurf->MipLevels[iCurrLOD]),
                                   pD3DSurf->MipLevels[iCurrLOD].fpVidMemTM,
                                   pD3DSurf->dwPatchMode,
                                   pD3DSurf->dwPatchMode,
                                   pD3DSurf->MipLevels[iCurrLOD].lPitch,
                                   pD3DSurf->MipLevels[iCurrLOD].lPitch,
                                   pD3DSurf->MipLevels[iCurrLOD].P3RXTextureMapWidth.Width,
                                   pD3DSurf->dwPixelSize,
                                   &rect,
                                   &rect);

                    // Move on to next slice
                    rect.top    += pD3DSurf->wHeight;
                    rect.bottom += pD3DSurf->wHeight;
                }
            }
            else
#endif // DX8_3DTEXTURES
            {
        
#if DX8_3DTEXTURES
                if (pD3DSurf->dwCaps2 & DDSCAPS2_VOLUME)
                {
                    rect.bottom *= pD3DSurf->wDepth;
                }
#endif // DX8_3DTEXTURES

                _DD_P3Download(pThisDisplay,
                               _D3D_TM_GetMipLevelSysMemPtr(
                                   pThisDisplay,
                                   pD3DSurf,
                                   &pD3DSurf->MipLevels[iCurrLOD]),
                               pD3DSurf->MipLevels[iCurrLOD].fpVidMemTM,
                               pD3DSurf->dwPatchMode,
                               pD3DSurf->dwPatchMode,
                               pD3DSurf->MipLevels[iCurrLOD].lPitch,
                               pD3DSurf->MipLevels[iCurrLOD].lPitch,                                                             
                               pD3DSurf->MipLevels[iCurrLOD].P3RXTextureMapWidth.Width,
                               pD3DSurf->dwPixelSize,
                               &rect,
                               &rect);   
                           
                DISPDBG((DBGLVL, "Copy from %08lx to %08lx"
                                 " w=%08lx h=%08lx p=%08lx",
                                 pD3DSurf->MipLevels[iCurrLOD].fpVidMem,
                                 pD3DSurf->MipLevels[iCurrLOD].fpVidMemTM,
                                 pD3DSurf->MipLevels[iCurrLOD].wWidth,
                                 pD3DSurf->MipLevels[iCurrLOD].wHeight,
                                 pD3DSurf->MipLevels[iCurrLOD].lPitch));                           
            }
        }

        // Texture updated in vidmem
        pD3DSurf->m_bTMNeedUpdate = FALSE;                                  

        // Switch back to the Direct3D context if DDRAW context was used
        if (pContext)
        {
            D3D_OPERATION(pContext, pThisDisplay);
        }
    }

    return TRUE;

} // _D3D_TM_Preload_SurfInt_IntoVidMem

//-----------------------------------------------------------------------------
//
// void _D3D_TM_Preload_Tex_IntoVidMem
//
// Transfer a texture from system memory into video memory.
//
//-----------------------------------------------------------------------------
BOOL
_D3D_TM_Preload_Tex_IntoVidMem(
    P3_D3DCONTEXT* pContext,
    P3_SURF_INTERNAL* pD3DSurf)
{
    return(_TM_Preload_SurfInt_IntoVidMem(pContext->pThisDisplay, 
                                          pContext, 
                                          pD3DSurf));

} // _D3D_TM_Preload_SurfInt_IntoVidMem

//-----------------------------------------------------------------------------
//
// P3_SURF_INTERNAL* _TM_GetSurfIntFromDDSurface
//
// Get D3D surface internal structure from DD surface local structure
//
//-----------------------------------------------------------------------------
P3_SURF_INTERNAL* _TM_GetSurfIntFromDDSurface(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pDDSLcl)
{
    DWORD dwSurfaceHandle = pDDSLcl->lpSurfMore->dwSurfaceHandle;
    PointerArray* pSurfaceArray;

    // We don't know which D3D context this is so we have to do a search 
    // through them (if there are any at all)
    if (! pThisDisplay->pDirectDrawLocalsHashTable)
    {
        return (NULL);
    }

    // Get a pointer to an array of surface pointers associated to this lpDD
    // The PDD_DIRECTDRAW_LOCAL was stored at D3DCreateSurfaceEx call time
    // in PDD_SURFACE_LOCAL->dwReserved1 
    pSurfaceArray = (PointerArray*)
                        HT_GetEntry(pThisDisplay->pDirectDrawLocalsHashTable,
                                    pDDSLcl->dwReserved1);

    if (! pSurfaceArray)
    {
        return (NULL);
    }

    // Found a surface array associated to this lpDD !
    // Check the surface in this array associated to this surface handle
    return (PA_GetEntry(pSurfaceArray, dwSurfaceHandle));
}

//-----------------------------------------------------------------------------
//
// HRESULT _D3D_TM_Prepare_DDSurf
//
// Make a managed DD surface as one in video memory.
//
//-----------------------------------------------------------------------------

HRESULT _D3D_TM_Prepare_DDSurf(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pDDSLcl,
    BOOL bSysMemSurfOK)
{
    P3_SURF_INTERNAL* pSurfInt;
    BOOL bRet;
    FLATPTR fpSysMem;

    // Nothing to prepare in case of there is no source surface
    if (! pDDSLcl)
    {
        return (DD_OK);
    }

    // Nothing to do for surface that is not managed by driver
    if (! (pDDSLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_TEXTUREMANAGE))
    {
        return (DD_OK);
    }
 
    // Only handle managed render target and Z buffer
    if (! (pDDSLcl->ddsCaps.dwCaps & (DDSCAPS_3DDEVICE | DDSCAPS_ZBUFFER)))
    {
        return (DD_OK);
    }

    // Turn off the managed flag
    pDDSLcl->lpSurfMore->ddsCapsEx.dwCaps2 &= (~DDSCAPS2_TEXTUREMANAGE);

    // Get the internal surface structure for full information
    pSurfInt = _TM_GetSurfIntFromDDSurface(pThisDisplay, pDDSLcl);
    if (! pSurfInt)
    {
        return (DDERR_GENERIC);
    }

    // Try to load the content of the surface into video memory
    bRet = _TM_Preload_SurfInt_IntoVidMem(pThisDisplay, NULL, pSurfInt);
    if (bRet)
    {
        // Fix up the DD surface local so that it appears as a vid mem surf
        pDDSLcl->lpGbl->fpVidMem  = pSurfInt->MipLevels[0].fpVidMemTM;
        pDDSLcl->ddsCaps.dwCaps  |= DDSCAPS_LOCALVIDMEM;

        return (DD_OK);
    }

    // Src surf can use the up-to-date sys mem copy
    if ((! bSysMemSurfOK) || (! pSurfInt->m_bTMNeedUpdate))
    {
        return (DDERR_OUTOFMEMORY);
    }

    // Get a pointer to the mapped section
    fpSysMem = _D3D_TM_GetDDSurfSysMemPtr(pThisDisplay, pDDSLcl);
    if (! fpSysMem)
    {
        return (DDERR_OUTOFMEMORY);
    }

    // Fix up the DD surface local so that it appears as sys mem surf
    pDDSLcl->lpGbl->fpVidMem  = fpSysMem;
    pDDSLcl->ddsCaps.dwCaps  |= DDSCAPS_SYSTEMMEMORY;
    pDDSLcl->ddsCaps.dwCaps  &= (~(DDSCAPS_LOCALVIDMEM    | 
                                   DDSCAPS_NONLOCALVIDMEM |
                                   DDSCAPS_VIDEOMEMORY));

    return (DD_OK);
}

//-----------------------------------------------------------------------------
//
// HRESULT _D3D_TM_Restore_DDSurf
//
// Restore the managed DD surface to its original state .
//
//-----------------------------------------------------------------------------
HRESULT
_D3D_TM_Restore_DDSurf(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pDDSLcl)
{
    P3_SURF_INTERNAL* pSurfInt;

    // Nothing to restore in case of there is no source surface
    if (! pDDSLcl)
    {
        return (DD_OK);
    }

    // Only handle managed render target and Z buffer
    if (! (pDDSLcl->ddsCaps.dwCaps & (DDSCAPS_3DDEVICE | DDSCAPS_ZBUFFER)))
    {
        return (DD_OK);
    }

    // Get the internal surface structure for full information
    pSurfInt = _TM_GetSurfIntFromDDSurface(pThisDisplay, pDDSLcl);
    if (! pSurfInt)
    {
        return (DDERR_GENERIC);
    }

    // Restore the managed flag
    pDDSLcl->lpSurfMore->ddsCapsEx.dwCaps2 = pSurfInt->dwCaps2; 

    // Restore the DD surface local structure
    pDDSLcl->lpGbl->fpVidMem = pSurfInt->MipLevels[0].fpVidMem;
    pDDSLcl->ddsCaps.dwCaps  = pSurfInt->ddsCapsInt.dwCaps;

    return (DD_OK);
}

//-----------------------------------------------------------------------------
//
// void _D3D_TM_MarkDDSurfaceAsDirty
//
// Help mark a DD surface as dirty given that we need to search for it
// based on its lpSurfMore->dwSurfaceHandle and the lpDDLcl.
//
//-----------------------------------------------------------------------------
void
_D3D_TM_MarkDDSurfaceAsDirty(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pDDSLcl, 
    BOOL bDirty)
{
    P3_SURF_INTERNAL* pSurfInt;
    
    if (pSurfInt = _TM_GetSurfIntFromDDSurface(pThisDisplay, pDDSLcl))
    {
        // Got it! Now update dirty TM value
        pSurfInt->m_bTMNeedUpdate = bDirty;
    }

} // _D3D_TM_MarkDDSurfaceAsDirty

//-----------------------------------------------------------------------------
// HRESULT _D3D_TM_LockDDSurface
//
// Return a pointer to the sys/vid mem copy
//
//-----------------------------------------------------------------------------

HRESULT 
_D3D_TM_LockDDSurface(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pDDSLcl, 
    ULONG ulOffset, 
    LPVOID *ppSurfData, 
    FLATPTR fpProcess)
{
#if DX9_RTZMAN


    // Render target texture is created using the vidmem swap memory,
    // so its RT property takes precedence over its texture property.
    if (pDDSLcl->ddsCaps.dwCaps & (DDSCAPS_3DDEVICE | DDSCAPS_ZBUFFER)) 
    {
        P3_SURF_INTERNAL* pSurfInt;
        VID_MEM_SWAP_MANAGER* pVMSMgr = pThisDisplay->pVidMemSwapMgr;
        FLATPTR pCurProcMappedBase;

        pSurfInt = _TM_GetSurfIntFromDDSurface(pThisDisplay, pDDSLcl);
        if (! pSurfInt) 
        {
            return (DDERR_GENERIC);
        }

        if (! pVMSMgr->Perm3MapVidMemSwap(pVMSMgr, 
                                          (PVOID *)&pCurProcMappedBase)) 
        {
            return (DDERR_OUTOFMEMORY);
        }

        // Refresh the sys mem copy if it is out of sync with vid mem copy
        if (! pSurfInt->m_bTMNeedUpdate)
        {
            RECTL rect;

            rect.left = rect.top = 0;
            rect.right = pSurfInt->wWidth;
            rect.bottom = pSurfInt->wHeight;

            _DD_BLT_SysMemToSysMemCopy(
                D3DTMMIPLVL_GETPOINTER((&pSurfInt->MipLevels[0]), pThisDisplay),
                pSurfInt->lPitch,
                pSurfInt->dwBitDepth,
                pCurProcMappedBase + (ULONG)pSurfInt->MipLevels[0].fpVidMem,
                pSurfInt->lPitch,
                pSurfInt->dwBitDepth,
                &rect,
                &rect);
        }
        
        // Always return a pointer to the system memory copy
        *ppSurfData = (LPBYTE)pCurProcMappedBase + 
                      (ULONG)pDDSLcl->lpGbl->fpVidMem + ulOffset;

        // Mark the managed RT/Z buf as dirty
        pSurfInt->m_bTMNeedUpdate = TRUE;

        return (DD_OK);
    }
#endif

    if (pDDSLcl->ddsCaps.dwCaps & DDSCAPS_TEXTURE)
    {
        _D3D_TM_MarkDDSurfaceAsDirty(pThisDisplay, pDDSLcl, TRUE);
        *ppSurfData = ((LPBYTE)pDDSLcl->lpGbl->fpVidMem) + ulOffset;
        return (DD_OK);
    }

    // Set return value to NULL
    *ppSurfData = NULL;

    // Other type of surfaces
    return (DDERR_GENERIC);
}

//-----------------------------------------------------------------------------
//
// void _D3D_TM_MarkDDSurfaceAsDirty
//
// Return a pointer to the surface's system memory copy
//
//-----------------------------------------------------------------------------
FLATPTR 
_D3D_TM_GetDDSurfSysMemPtr(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pDdSLcl)
{
#if DX9_RTZMAN
    if ((pDdSLcl->ddsCaps.dwCaps & (DDSCAPS_3DDEVICE | DDSCAPS_ZBUFFER)) && 
        (pDdSLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_TEXTUREMANAGE))
    {
        VID_MEM_SWAP_MANAGER *pVMSMgr = pThisDisplay->pVidMemSwapMgr;
        LPBYTE pCurProcMappedBase;

        if (! pVMSMgr->Perm3MapVidMemSwap(pVMSMgr, &pCurProcMappedBase))
        {
            return ((FLATPTR)NULL);
        }

        return ((FLATPTR)(pCurProcMappedBase + 
                          (ULONG)pDdSLcl->lpGbl->fpVidMem));
    }
    else
#endif
    {
        if (pDdSLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_TEXTUREMANAGE) 
        {
            return (pDdSLcl->lpGbl->fpVidMem);
        }
        else
        {
            return ((FLATPTR)NULL);
        }
    }
}

//-----------------------------------------------------------------------------
//
// void _D3D_TM_MarkDDSurfaceAsDirty
//
// Return a pointer to the surface's system memory copy
//
//-----------------------------------------------------------------------------
FLATPTR
_D3D_TM_GetMipLevelSysMemPtr(
    P3_THUNKEDDATA* pThisDisplay,
    P3_SURF_INTERNAL* pSurfInt,
    MIPTEXTURE* pMipLevel)
{
#if DX9_RTZMAN
    if (pSurfInt->ddsCapsInt.dwCaps & (DDSCAPS_3DDEVICE | DDSCAPS_ZBUFFER))
    {
        VID_MEM_SWAP_MANAGER *pVMSMgr = pThisDisplay->pVidMemSwapMgr;
        LPBYTE pCurProcMappedBase;

        if (! pVMSMgr->Perm3MapVidMemSwap(pVMSMgr, &pCurProcMappedBase))
        {
            return ((FLATPTR)NULL);
        }

        return ((FLATPTR)(pCurProcMappedBase +
                          (ULONG)pMipLevel->fpVidMem));
    }
#endif
    {
        return (pMipLevel->fpVidMem);
    }
}

//-----------------------------------------------------------------------------
//
// DWORD _D3D_TM_Get_VidMem_Offset
//
// Return the video memory offset for a managed render target or z buffer
//
//-----------------------------------------------------------------------------
DWORD
_D3D_TM_Get_VidMem_Offset(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pDdSLcl)
{
#if DX9_RTZMAN
    if (pDdSLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
    {
        P3_SURF_INTERNAL* pSurfInt;

        pSurfInt = _TM_GetSurfIntFromDDSurface(pThisDisplay, pDdSLcl);
        if (! pSurfInt)
        {
            return (0);
        }

        if (! _TM_Preload_SurfInt_IntoVidMem(pThisDisplay, NULL, pSurfInt))
        {
            return (0);
        }

        return ((DWORD)(pSurfInt->MipLevels[0].fpVidMemTM -
                        pThisDisplay->dwScreenFlatAddr));
    }
#endif
    {
        return ((DWORD)(pDdSLcl->lpGbl->fpVidMem - 
                        pThisDisplay->dwScreenFlatAddr));
    }
}

//-----------------------------------------------------------------------------
//
// VOID _D3D_TM_Touch_Context
//
// Mark the managed RT and ZBuf as the most recently used
//
//-----------------------------------------------------------------------------
#if DX9_RTZMAN
VOID
_D3D_TM_Touch_Context(
    P3_D3DCONTEXT* pContext)
{
    if ((pContext->pSurfRenderInt) &&
        (pContext->pSurfRenderInt->dwCaps2 & DDSCAPS2_TEXTUREMANAGE))
    {
        _D3D_TM_TimeStampTexture(pContext->pThisDisplay->pTextureManager,
                                 pContext->pSurfRenderInt);
    }
    if ((pContext->pSurfZBufferInt) &&
        (pContext->pSurfZBufferInt->dwCaps2 & DDSCAPS2_TEXTUREMANAGE))
    {
        _D3D_TM_TimeStampTexture(pContext->pThisDisplay->pTextureManager,
                                 pContext->pSurfZBufferInt);
    }
}
#endif

#endif // DX7_TEXMANAGEMENT



