/******************************Module*Header**********************************\
*
*                           *******************
*                           * GDI SAMPLE CODE *
*                           *******************
*
* Module Name: sync.c
*
* Content: DrvSynchronize
*
* Copyright (c) 1994-1999 3Dlabs Inc. Ltd. All rights reserved.
* Copyright (c) 1995-2002 Microsoft Corporation.  All rights reserved.
\*****************************************************************************/

#include "precomp.h"
#include "pxrx.h"

/******************************Public*Routine******************************\
* VOID DrvSynchronize
*
* Synchronize with the GLINT.
*
* Before letting GDI draw onto our screen DIBs we must synchronize with GLINT.
* We do this by hooking HOOK_SYNCHRONIZE when we create a DIV that points to
* the screen (or off-screen). GDI then calls DrvSynchronize() before trying
* to render to any of these screen DIBs. The advantage is that we can forget
* about doing SYNC_WITH_GLINT in the code and have GDI call DrvSynchronize
* at the appropriate moment.
*
\**************************************************************************/

VOID
DrvSynchronize(
    DHPDEV  dhpdev,
    RECTL  *prcl)
{
    PDEV*   ppdev = (PDEV*) dhpdev;
    GLINT_DECL;

    DBG_CB_ENTRY(DrvSynchronize);

    DISPDBG((DBGLVL, "DrvSynchronize called"));

    // These chips use cached syncs

    SYNC_IF_CORE_BUSY;
}


#if USE_SYNC_FUNCTION



void 
syncWithGlint(
    PPDEV           ppdev, 
    GlintDataPtr    glintInfo) 
{
    DWORD           gsync;
    ULONG           bmask;
    TEMP_MACRO_VARS;

    WAIT_DMA_COMPLETE;

    // Setting the pxrxDMA->bFlushRequired flag below to false ensures that 
    // the interrupt routine will not try to write to the chip. Without 
    // this check, the Vblank interrupt can attempt to flush outstanding 2D 
    // rendering by writing ContinueNewSub tag and data to the chip.
    // The problem with this is that the interrupt can occur between the 
    // writes of the tag and data in the code below, and if this happens,
    // the tag and data becomes out of step and typically results in a hang.
    //
    // Note that we have to set a MUTEX otherwise we still get hangs on a 
    // multi-processor system when an interrupt routine can be running 
    // simultaneously on another processor. The MUTEX avoids race conditions.
    
    GET_INTR_CMD_BLOCK_MUTEX(&glintInfo->pInterruptCommandBlock->General);
    glintInfo->pxrxDMA->bFlushRequired = FALSE ;

    WAIT_GLINT_FIFO(4);
    LD_GLINT_FIFO(__GlintTagFilterMode, 0x400);
    LD_GLINT_FIFO(__GlintTagSync, 0);
    LD_GLINT_FIFO(__GlintTagFilterMode, 0x0);
    
    do {
        WAIT_OUTPUT_FIFO_READY;
        READ_OUTPUT_FIFO(gsync);
        DISPDBG((DBGLVL, "SYNC: got 0x%x from output FIFO", gsync));
    } while (gsync != __GlintTagSync);
    
    glintInfo->bGlintCoreBusy = FALSE;
    RELEASE_INTR_CMD_BLOCK_MUTEX(&glintInfo->pInterruptCommandBlock->General);
}


void 
waitDMAcomplete(
    PPDEV           ppdev, 
    GlintDataPtr    glintInfo) 
{
    TEMP_MACRO_VARS;

    if (ppdev->currentCtxt == glintInfo->ddCtxtId)
    {
        SEND_PXRX_DMA_FORCE;
    }

    if (! (ppdev->g_GlintBoardStatus & GLINT_DMA_COMPLETE)) 
    {
        if (ppdev->g_GlintBoardStatus & GLINT_INTR_CONTEXT) 
        {
            // do any VBLANK wait, wait Q to empty and last DMA to complete
            PINTERRUPT_CONTROL_BLOCK pBlock = glintInfo->pInterruptCommandBlock;
            
            pBlock->Perm3WaitDMACompletion(pBlock);
        }
        
        ppdev->g_GlintBoardStatus |= GLINT_DMA_COMPLETE;
    }
}

#endif  // USE_SYNC_FUNCTION





