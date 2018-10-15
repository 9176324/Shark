/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmchek.c

Abstract:

    This module implements consistency checking for the registry.
    This module can be linked standalone, cmchek2.c cannot.

--*/

#include    "cmp.h"

#define     REG_MAX_PLAUSIBLE_KEY_SIZE \
                ((FIELD_OFFSET(CM_KEY_NODE, Name)) + \
                 (sizeof(WCHAR) * REG_MAX_KEY_NAME_LENGTH) + 16)

extern PCMHIVE CmpMasterHive;

//
// Private prototypes
//

ULONG
CmpCheckRegistry2(
    PHHIVE      HiveToCheck,
    ULONG       CheckFlags,
    HCELL_INDEX Cell,
    HCELL_INDEX ParentCell,
    BOOLEAN     ResetSD
    );

ULONG
CmpCheckKey(
    PHHIVE      HiveToCheck,
    ULONG       CheckFlags,
    HCELL_INDEX Cell,
    HCELL_INDEX ParentCell,
    BOOLEAN     ResetSD
    );

ULONG
CmpCheckValueList(
    PHHIVE      Hive,
    PCELL_DATA  List,
    ULONG       Count,
    HCELL_INDEX KeyCell
    );

BOOLEAN
CmpCheckLexicographicalOrder (  IN PHHIVE       HiveToCheck,
                                IN HCELL_INDEX  PriorSibling,
                                IN HCELL_INDEX  Current 
                                );

VOID
CmpLogError(IN NTSTATUS         NtStatusCode);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmCheckRegistry)
#pragma alloc_text(PAGE,CmpCheckRegistry2)
#pragma alloc_text(PAGE,CmpCheckKey)
#pragma alloc_text(PAGE,CmpCheckValueList)
#pragma alloc_text(PAGE,CmpCheckLexicographicalOrder)
#pragma alloc_text(PAGE,CmpLogError)

#endif

//
// debug structures
//

extern struct {
    PHHIVE      Hive;
    ULONG       Status;
} CmCheckRegistryDebug;

extern struct {
    PHHIVE      Hive;
    ULONG       Status;
} CmpCheckRegistry2Debug;

extern struct {
    PHHIVE      Hive;
    ULONG       Status;
    HCELL_INDEX Cell;
    PCELL_DATA  CellPoint;
    PVOID       RootPoint;
    ULONG       Index;
} CmpCheckKeyDebug;

extern struct {
    PHHIVE      Hive;
    ULONG       Status;
    PCELL_DATA  List;
    ULONG       Index;
    HCELL_INDEX Cell;
    PCELL_DATA  CellPoint;
} CmpCheckValueListDebug;


ULONG
CmCheckRegistry(
    PCMHIVE CmHive,
    ULONG   Flags
    )
/*++

Routine Description:

    Check consistency of the registry within a given hive.  Start from
    root, and check that:
        .   Each child key points back to its parent.
        .   All allocated cells are referred to exactly once
            (requires looking inside the hive structure...)
            [This also detects space leaks.]
        .   All allocated cells are reachable from the root.

    NOTE:   Exactly 1 ref rule may change with security.

Arguments:

    CmHive - supplies a pointer to the CM hive control structure for the
            hive of interest.

    Clean   - if TRUE, references to volatile cells will be zapped
              (done at startup only to avoid groveling hives twice.)
              if FALSE, nothing will be changed.
    
    HiveCheck - If TRUE, performs hive consistency check too (i.e. checks
                the bins)

Return Value:

    0 if Hive is OK.  Error return indicator if not.

    RANGE:  3000 - 3999

--*/
{
    PHHIVE                  Hive;
    ULONG                   rc = 0;
    ULONG                   Storage;
    PRELEASE_CELL_ROUTINE   ReleaseCellRoutine;
    BOOLEAN                 ResetSD = FALSE;

    if (CmHive == CmpMasterHive) {
        return(0);
    }

    CmCheckRegistryDebug.Hive = (PHHIVE)CmHive;
    CmCheckRegistryDebug.Status = 0;


    //
    // check the underlying hive and get storage use
    //
    Hive = &CmHive->Hive;

    if( Flags & CM_CHECK_REGISTRY_HIVE_CHECK ) {
        rc = HvCheckHive(Hive, &Storage);
        if (rc != 0) {
            CmCheckRegistryDebug.Status = rc;
            return rc;
        }
    }

    //
    // Store the release cell procedure so we can restore at the end;
    // Set it to NULL so we don't count : this saves us some pain during the check
    //
    ReleaseCellRoutine = Hive->ReleaseCellRoutine;
    Hive->ReleaseCellRoutine = NULL;

    //
    // Validate all the security descriptors in the hive
    //
    if (!CmpValidateHiveSecurityDescriptors(Hive,&ResetSD)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmCheckRegistry:"));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL," CmpValidateHiveSecurityDescriptors failed\n"));
        rc = 3040;
        CmCheckRegistryDebug.Status = rc;
        Hive->ReleaseCellRoutine = ReleaseCellRoutine;
        return rc;
    }

    rc = CmpCheckRegistry2((PHHIVE)CmHive,Flags,Hive->BaseBlock->RootCell, HCELL_NIL,ResetSD);

    //
    // Print a bit of a summary (make sure this data avail in all error cases)
    //
    if (rc > 0) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmCheckRegistry Failed (%d): CmHive:%p\n", rc, CmHive));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL," Hive:%p Root:%08lx\n", Hive, Hive->BaseBlock->RootCell));
    }

    //
    // restore the release cell routine
    // this saves us some pain during the check
    //
    Hive->ReleaseCellRoutine = ReleaseCellRoutine;

    return rc;
}

ULONG
CmpCheckRegistry2(
    PHHIVE      HiveToCheck,
    ULONG       CheckFlags,
    HCELL_INDEX Cell,
    HCELL_INDEX ParentCell,
    BOOLEAN     ResetSD
    )
/*++

Routine Description:

    Check consistency of the registry, from a particular cell on down.

        .   Check that the cell's value list, child key list, class,
            security are OK.
        .   Check that each value entry IN the list is OK.
        .   Apply self to each child key list.

    
    This version uses a stack in order to parse the tree "in-depth", 
    but not to touch any key_node.

Arguments:

    Cell - HCELL_INDEX of subkey to work on.

    ParentCell - expected value of parent cell for Cell, unless
                 HCELL_NIL, in which case ignore.

Return Value:

    0 if Hive is OK.  Error return indicator if not.

    RANGE:  4000 - 4999

--*/
{
    PCMP_CHECK_REGISTRY_STACK_ENTRY     CheckStack;
    LONG                                StackIndex;
    PCM_KEY_NODE                        Node;
    ULONG                               rc = 0;
    HCELL_INDEX                         SubKey;


    CmpCheckRegistry2Debug.Hive = HiveToCheck;
    CmpCheckRegistry2Debug.Status = 0;
    
    ASSERT( HiveToCheck->ReleaseCellRoutine == NULL );

    //
    // Initialize the stack to simulate recursion here
    //

    CmRetryExAllocatePoolWithTag(PagedPool,sizeof(CMP_CHECK_REGISTRY_STACK_ENTRY)*CMP_MAX_REGISTRY_DEPTH,CM_POOL_TAG|PROTECTED_POOL,CheckStack);
    if (CheckStack == NULL) {
        CmpCheckRegistry2Debug.Status = 4099;
        return 4099;
    }

Restart:

    CheckStack[0].Cell = Cell;
    CheckStack[0].ParentCell = ParentCell;
    CheckStack[0].PriorSibling = HCELL_NIL;
    CheckStack[0].ChildIndex = 0;
    CheckStack[0].CellChecked = FALSE;
    StackIndex = 0;


    while(StackIndex >=0) {
        //
        // first check the current cell
        //
        if( CheckStack[StackIndex].CellChecked == FALSE ) {
            CheckStack[StackIndex].CellChecked = TRUE;

            rc = CmpCheckKey(HiveToCheck,CheckFlags,CheckStack[StackIndex].Cell, CheckStack[StackIndex].ParentCell,ResetSD);
            if (rc != 0) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tChild is list entry #%08lx\n", CheckStack[StackIndex].ChildIndex));
                CmpCheckRegistry2Debug.Status = rc;
YankKey:
                if( CmDoSelfHeal() && StackIndex ) { // root cell damage is fatal.
                    //
                    // delete this key from the parent's list and restart the whole iteration (not best performance, but safest).
                    //
                    if( !CmpRemoveSubKeyCellNoCellRef(HiveToCheck,CheckStack[StackIndex].ParentCell,CheckStack[StackIndex].Cell) ) {
                        //
                        // unable to delete subkey; punt.
                        //
                        break;
                    }
                    CmMarkSelfHeal(HiveToCheck);
                    rc = 0;
                    goto Restart;
                } else {
                    // bail out
                    break;
                }
            } else if( StackIndex > 0 ) {
                //
                // key is OK, check lexicographical order with PriorSibling
                //
                if( CheckStack[StackIndex-1].PriorSibling != HCELL_NIL ) {
                
                    ASSERT( CheckStack[StackIndex-1].Cell == CheckStack[StackIndex].ParentCell );
                    
                    if( !CmpCheckLexicographicalOrder(HiveToCheck,CheckStack[StackIndex-1].PriorSibling,CheckStack[StackIndex].Cell) ) {
                        //
                        // invalid order
                        //
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\t Invalid subkey ordering key #%08lx\n", CheckStack[StackIndex].Cell));
                        CmpCheckRegistry2Debug.Status = 4091;
                        rc = 4091;
                        // attempt to yank the key
                        goto YankKey;
                    }
                }   
                CheckStack[StackIndex-1].PriorSibling = CheckStack[StackIndex].Cell;
            }
        }

        Node = (PCM_KEY_NODE)HvGetCell(HiveToCheck, CheckStack[StackIndex].Cell);
        if( Node == NULL ) {
            //
            // we couldn't map a view for the bin containing this cell
            //
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", CheckStack[StackIndex].Cell));
            CmpCheckRegistry2Debug.Status = 4098;
            rc = 4098;
            // bail out
            break;
        }

        if( CheckStack[StackIndex].ChildIndex < Node->SubKeyCounts[Stable] ) {
            //
            // we still have childs to check; add another entry for them and advance the 
            // StackIndex
            //
            SubKey = CmpFindSubKeyByNumber(HiveToCheck,
                                           Node,
                                           CheckStack[StackIndex].ChildIndex);
            if( SubKey == HCELL_NIL ) {
                //
                // we couldn't map cell;bail out
                //
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", CheckStack[StackIndex].Cell));
                CmpCheckRegistry2Debug.Status = 4097;
                rc = 4097;
                break;
            }
            //
            // next iteration will check the next child
            //
            CheckStack[StackIndex].ChildIndex++;

            StackIndex++;
            if( StackIndex == CMP_MAX_REGISTRY_DEPTH ) {
                //
                // we've run out of stack; registry tree has too many levels
                //
                CmpCheckRegistry2Debug.Status = 4096;
                rc = 4096;
                // bail out
                break;
            }
            CheckStack[StackIndex].Cell = SubKey;
            CheckStack[StackIndex].ParentCell = CheckStack[StackIndex-1].Cell;
            CheckStack[StackIndex].PriorSibling = HCELL_NIL;
            CheckStack[StackIndex].ChildIndex = 0;
            CheckStack[StackIndex].CellChecked = FALSE;

        } else {
            //
            // we have checked all childs for this node; go back
            //
            StackIndex--;

        }

    }

    ExFreePoolWithTag(CheckStack, CM_POOL_TAG|PROTECTED_POOL);
    return rc;
}


#if DBG

#define VOLATILE_KEY_NAME_LENGTH        PAGE_SIZE

HCELL_INDEX     CmpKeyCellDebug = 0;
WCHAR           CmpVolatileKeyNameBuffer[VOLATILE_KEY_NAME_LENGTH/2];
#endif //DBG

ULONG
CmpCheckKey(
    PHHIVE      HiveToCheck,
    ULONG       CheckFlags,
    HCELL_INDEX Cell,
    HCELL_INDEX ParentCell,
    BOOLEAN     ResetSD
    )
/*++

Routine Description:

    Check consistency of the registry, for a particular cell

        .   Check that the cell's value list, child key list, class,
            security are OK.
        .   Check that each value entry IN the list is OK.

Arguments:

    Cell - HCELL_INDEX of subkey to work on.

    ParentCell - expected value of parent cell for Cell, unless
                 HCELL_NIL, in which case ignore.

Return Value:

    0 if Hive is OK.  Error return indicator if not.

    RANGE:  4000 - 4999

--*/
{
    PCELL_DATA      pcell;
    ULONG           size;
    ULONG           usedlen;
    ULONG           ClassLength;
    HCELL_INDEX     Class;
    ULONG           ValueCount;
    HCELL_INDEX     ValueList;
    HCELL_INDEX     Security;
    ULONG           rc = 0;
    ULONG           nrc = 0;
    ULONG           i;
    PCM_KEY_INDEX   Root;
    PCM_KEY_INDEX   Leaf;
    ULONG           SubCount;

    CmpCheckKeyDebug.Hive = HiveToCheck;
    CmpCheckKeyDebug.Status = 0;
    CmpCheckKeyDebug.Cell = Cell;
    CmpCheckKeyDebug.CellPoint = NULL;
    CmpCheckKeyDebug.RootPoint = NULL;
    CmpCheckKeyDebug.Index = (ULONG)-1;

#if DBG
    if(CmpKeyCellDebug == Cell) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"Hive = %p :: Cell to debug = %lx\n",HiveToCheck,(ULONG)Cell));
        DbgBreakPoint();
    }
#endif //DBG

    //
    // Check key itself
    //
    if (! HvIsCellAllocated(HiveToCheck, Cell)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tNot allocated\n"));
        rc = 4010;
        CmpCheckKeyDebug.Status = rc;
        return rc;
    }
    pcell = HvGetCell(HiveToCheck, Cell);
    if( pcell == NULL ) {
        //
        // we couldn't map a view for the bin containing this cell
        //
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", Cell));
        CmpCheckKeyDebug.Status = 4095;
        return 4095;
    }

    CmpCheckKeyDebug.CellPoint = pcell;

    size = HvGetCellSize(HiveToCheck, pcell);
    if (size > REG_MAX_PLAUSIBLE_KEY_SIZE) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tImplausible size %lx\n", size));
        rc = 4020;
        CmpCheckKeyDebug.Status = rc;
        return rc;
    }
    usedlen = FIELD_OFFSET(CM_KEY_NODE, Name) + pcell->u.KeyNode.NameLength;
    if( (!pcell->u.KeyNode.NameLength) || (usedlen > size)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tKey is bigger than containing cell.\n"));
        rc = 4030;
        CmpCheckKeyDebug.Status = rc;
        return rc;
    }
    if (pcell->u.KeyNode.Signature != CM_KEY_NODE_SIGNATURE) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tNo key signature\n"));
        rc = 4040;
        CmpCheckKeyDebug.Status = rc;
        if( CmDoSelfHeal() ) {
            //
            // this could be only signature corruption; fix it;
            //
            if( HvMarkCellDirty(HiveToCheck, Cell,FALSE) ) {
                pcell->u.KeyNode.Signature = CM_KEY_NODE_SIGNATURE;
                rc = 0;
                CmMarkSelfHeal(HiveToCheck);
            } else {
                return rc;
            }
        } else {
            return rc;
        }
    }
    if (ParentCell != HCELL_NIL) {
        if (pcell->u.KeyNode.Parent != ParentCell) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tWrong parent value.\n"));
            rc = 4045;
            CmpCheckKeyDebug.Status = rc;
            if( CmDoSelfHeal() ) {
                //
                // this could isolated corruption; fix it;
                //
                if( HvMarkCellDirty(HiveToCheck, Cell,FALSE) ) {
                    pcell->u.KeyNode.Parent = ParentCell;
                    CmMarkSelfHeal(HiveToCheck);
                    rc = 0;
                } else {
                    return rc;
                }
            } else {
                return rc;
            }
        }
    }
    ClassLength = pcell->u.KeyNode.ClassLength;
    Class = pcell->u.KeyNode.Class;
    ValueCount = pcell->u.KeyNode.ValueList.Count;
    ValueList = pcell->u.KeyNode.ValueList.List;
    Security = pcell->u.KeyNode.Security;

    //
    // Check simple non-empty cases
    //
    if (ClassLength > 0) {
        if( Class == HCELL_NIL ) {
            pcell->u.KeyNode.ClassLength = 0;
            HvMarkCellDirty(HiveToCheck, Cell,FALSE);
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx has ClassLength = %lu and Class == HCELL_NIL\n", HiveToCheck, Cell,ClassLength));
        } else {
            if (HvIsCellAllocated(HiveToCheck, Class) == FALSE) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tClass:%08lx - unallocated class\n", Class));
                    rc = 4080;
                    CmpCheckKeyDebug.Status = rc;
                    if( CmDoSelfHeal() ) {
                        //
                        // yank the class
                        //
                        if( HvMarkCellDirty(HiveToCheck, Cell,FALSE) ) {
                            pcell->u.KeyNode.Class = HCELL_NIL;
                            pcell->u.KeyNode.ClassLength = 0;
                            CmMarkSelfHeal(HiveToCheck);
                            rc = 0;
                        } else {
                            return rc;
                        }
                    } else {
                        return rc;
                    }
            } 
        }
    }

    if (Security != HCELL_NIL) {
        if ((HvIsCellAllocated(HiveToCheck, Security) == FALSE) || 
            ((ParentCell != HCELL_NIL) && CmDoSelfHeal() && ResetSD) ) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tSecurity:%08lx - unallocated security\n", Security));
            rc = 4090;
            CmpCheckKeyDebug.Status = rc;
            goto SetParentSecurity;
        } 
        //
        // Else CmpValidateHiveSecurityDescriptors must do computation
        //
    } else {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"SecurityCell is HCELL_NIL for (%p,%08lx) !!!\n", HiveToCheck, Cell));
        rc = 4130;
        CmpCheckKeyDebug.Status = rc;
SetParentSecurity:
        if( CmDoSelfHeal() ) {
            //
            // attempt to set the same security as it's parent
            //
            PCM_KEY_NODE ParentNode = NULL;
            PCM_KEY_SECURITY SecurityNode = NULL;

            if( ParentCell != HCELL_NIL ) {
                ParentNode = (PCM_KEY_NODE )HvGetCell(HiveToCheck, ParentCell);
                SecurityNode = (PCM_KEY_SECURITY)HvGetCell(HiveToCheck, ParentNode->Security);
            }

            if( ParentNode == NULL || SecurityNode == NULL ) {
                //
                // we couldn't map a view for the bin containing this cell
                //
                return rc;
            }

            if( HvMarkCellDirty(HiveToCheck, Cell,FALSE) &&  HvMarkCellDirty(HiveToCheck, ParentNode->Security,FALSE) ) {
                pcell->u.KeyNode.Security = ParentNode->Security;
                SecurityNode->ReferenceCount++;
                rc = 0;
                CmMarkSelfHeal(HiveToCheck);
            } else {
                return rc;
            }
        } else {
            return rc;
        }
        
    }

    //
    // Check value list case
    //
    if (ValueCount > 0) {
        if (HvIsCellAllocated(HiveToCheck, ValueList) == FALSE) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tValueList:%08lx - unallocated valuelist\n", ValueList));
            rc = 4100;
            CmpCheckKeyDebug.Status = rc;
            goto YankValueList;
        } else {
            pcell = HvGetCell(HiveToCheck, ValueList);
            if( pcell == NULL ) {
                //
                // we couldn't map a view for the bin containing this cell
                //
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", ValueList));
                CmpCheckKeyDebug.Status = 4094;
                return 4094;
            }
            if( ValueCount * sizeof(HCELL_INDEX) > (ULONG)HvGetCellSize(HiveToCheck,pcell) ) {
                //
                // implausible value count.
                //
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tValueList:%08lx - Implausible ValueCount = %08lx\n", ValueList,ValueCount));
                rc = 4095;
                CmpCheckKeyDebug.Status = rc;
                goto YankValueList;
            }

            nrc = CmpCheckValueList(HiveToCheck, pcell, ValueCount,Cell);
            if (nrc != 0) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"List was for HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
                rc = nrc;
                CmpCheckKeyDebug.CellPoint = pcell;
                CmpCheckKeyDebug.Status = rc;
YankValueList:
                if( CmDoSelfHeal() ) {
                    PCM_KEY_NODE KeyNode;
                    //
                    // make the key valueless
                    //
                    if( HvMarkCellDirty(HiveToCheck, Cell,FALSE) && (KeyNode = (PCM_KEY_NODE)HvGetCell(HiveToCheck, Cell) ) ) {
                        KeyNode->ValueList.Count = 0;
                        KeyNode->ValueList.List = HCELL_NIL;
                        CmMarkSelfHeal(HiveToCheck);
                        rc = 0;
                    } else {
                        return rc;
                    }
                } else {
                    return rc;
                }
            }
        }
    }


    //
    // Check subkey list case
    //

    pcell = HvGetCell(HiveToCheck, Cell);
    if( pcell == NULL ) {
        //
        // we couldn't map a view for the bin containing this cell
        //
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", Cell));
        CmpCheckKeyDebug.Status = 4093;
        return 4093;
    }
    CmpCheckKeyDebug.CellPoint = pcell;
    if ((HvGetCellType(Cell) == Volatile) &&
        (pcell->u.KeyNode.SubKeyCounts[Stable] != 0))
    {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tVolatile Cell has Stable children\n"));
        rc = 4108;
        CmpCheckKeyDebug.Status = rc;
        return rc;
    } else if (pcell->u.KeyNode.SubKeyCounts[Stable] > 0) {
        if (! HvIsCellAllocated(HiveToCheck, pcell->u.KeyNode.SubKeyLists[Stable])) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tStableKeyList:%08lx - unallocated\n", pcell->u.KeyNode.SubKeyLists[Stable]));
            rc = 4110;
            CmpCheckKeyDebug.Status = rc;
            goto YankStableSubkeys;
        } else {
            //
            // Prove that the index is OK
            //
            Root = (PCM_KEY_INDEX)HvGetCell(
                                    HiveToCheck,
                                    pcell->u.KeyNode.SubKeyLists[Stable]
                                    );
            if( Root == NULL ) {
                //
                // we couldn't map a view for the bin containing this cell
                //
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", pcell->u.KeyNode.SubKeyLists[Stable]));
                CmpCheckKeyDebug.Status = 4093;
                return 4093;
            }
            CmpCheckKeyDebug.RootPoint = Root;
            if ((Root->Signature == CM_KEY_INDEX_LEAF) ||
                (Root->Signature == CM_KEY_FAST_LEAF)  ||
                (Root->Signature == CM_KEY_HASH_LEAF) ) {
                if ((ULONG)Root->Count != pcell->u.KeyNode.SubKeyCounts[Stable]) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tBad Index count @%08lx\n", Root));
                    rc = 4120;
                    CmpCheckKeyDebug.Status = rc;
                    if( CmDoSelfHeal() ) {
                        //
                        // fix the subkeycount
                        //
                        if( HvMarkCellDirty(HiveToCheck, Cell,FALSE) ) {
                            pcell->u.KeyNode.SubKeyCounts[Stable] = (ULONG)Root->Count;
                            CmMarkSelfHeal(HiveToCheck);
                            rc = 0;
                        } else {
                            return rc;
                        }
                    } else {
                        return rc;
                    } 
                }
            } else if (Root->Signature == CM_KEY_INDEX_ROOT) {
                SubCount = 0;
                for (i = 0; i < Root->Count; i++) {
                    CmpCheckKeyDebug.Index = i;
                    if (! HvIsCellAllocated(HiveToCheck, Root->List[i])) {
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: Hive:%p Cell:%08lx\n", HiveToCheck, Cell));
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tBad Leaf Cell %08lx Root@%08lx\n", Root->List[i], Root));
                        rc = 4130;
                        CmpCheckKeyDebug.Status = rc;
                        goto YankStableSubkeys;
                    }
                    Leaf = (PCM_KEY_INDEX)HvGetCell(HiveToCheck,
                                                    Root->List[i]);
                    if( Leaf == NULL ) {
                        //
                        // we couldn't map a view for the bin containing this cell
                        //
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", Root->List[i]));
                        CmpCheckKeyDebug.Status = 4092;
                        return 4092;
                    }

                    if ((Leaf->Signature != CM_KEY_INDEX_LEAF) &&
                        (Leaf->Signature != CM_KEY_FAST_LEAF)  &&
                        (Leaf->Signature != CM_KEY_HASH_LEAF) ) {
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tBad Leaf Index @%08lx Root@%08lx\n", Leaf, Root));
                        rc = 4140;
                        CmpCheckKeyDebug.Status = rc;
                        goto YankStableSubkeys;
                    }
                    SubCount += Leaf->Count;
                }
                if (pcell->u.KeyNode.SubKeyCounts[Stable] != SubCount) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tBad count in index, SubCount=%08lx\n", SubCount));
                    rc = 4150;
                    CmpCheckKeyDebug.Status = rc;
                    if( CmDoSelfHeal() ) {
                        //
                        // fix the subkeycount
                        //
                        if( HvMarkCellDirty(HiveToCheck, Cell,FALSE) ) {
                            pcell->u.KeyNode.SubKeyCounts[Stable] = SubCount;
                            CmMarkSelfHeal(HiveToCheck);
                            rc = 0;
                        } else {
                            return rc;
                        }
                    } else {
                        return rc;
                    } 
                }
            } else {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tBad Root index signature @%08lx\n", Root));
                rc = 4120;
                CmpCheckKeyDebug.Status = rc;
                goto YankStableSubkeys;
            }
        }
    }
    if( FALSE ) {
YankStableSubkeys:
        if( CmDoSelfHeal() ) {
            //
            // mark the key as no subkeys
            //
            if( HvMarkCellDirty(HiveToCheck, Cell,FALSE) ) {
                pcell->u.KeyNode.SubKeyCounts[Stable] = 0;
                pcell->u.KeyNode.SubKeyLists[Stable] = HCELL_NIL;
                CmMarkSelfHeal(HiveToCheck);
                rc = 0;
            } else {
                return rc;
            }
        } else {
            return rc;
        } 
    }
    //
    // force volatiles to be empty, if this is a load operation
    //
    if ( (CheckFlags & CM_CHECK_REGISTRY_FORCE_CLEAN) || // force clear out volatile info 
         ( 
             ( CheckFlags & (CM_CHECK_REGISTRY_CHECK_CLEAN | CM_CHECK_REGISTRY_LOADER_CLEAN) ) &&  // if asked to clear volatile info
             ( pcell->u.KeyNode.SubKeyCounts[Volatile] != 0 )                               // there is some volatile info saved from a previous version
         )                                             ||
         (
             ( CheckFlags & CM_CHECK_REGISTRY_SYSTEM_CLEAN ) &&         // system hive special case; the loader has cleaned only subkeycount
             (( pcell->u.KeyNode.SubKeyLists[Volatile] != HCELL_NIL ) ||    // now it is our job to clear Subkeylist, too
             (HiveToCheck->Version < HSYS_WHISTLER_BETA1) )
         ) 
        
        ) {
        //
        // go ahead and clear the volatile info for this key
        //
        if( CheckFlags & CM_CHECK_REGISTRY_SYSTEM_CLEAN ) {
            //
            // the loader must've left this on the previous value and cleared only the count
            //
            ASSERT( pcell->u.KeyNode.SubKeyLists[Volatile] == 0xBAADF00D || HiveToCheck->Version < HSYS_WHISTLER_BETA1 );
            ASSERT( pcell->u.KeyNode.SubKeyCounts[Volatile] == 0 );
#if DBG
            //
            // see who those volatile keys are
            //
            {
                ULONG           TotalLength = 0;
                HCELL_INDEX     CurrCell = Cell;
                PCM_KEY_NODE    CurrNode;
                PUCHAR          Dest;
                ULONG           k;

                Dest = ((PUCHAR)CmpVolatileKeyNameBuffer) + VOLATILE_KEY_NAME_LENGTH - 2;
                while(TRUE) {
                    CurrNode = (PCM_KEY_NODE)HvGetCell(HiveToCheck,CurrCell);
                    Dest -= CurrNode->NameLength;
                    TotalLength += CurrNode->NameLength;
                    if (CurrNode->Flags & KEY_COMP_NAME) {
                        Dest -= CurrNode->NameLength;
                        for (k=0;k<CurrNode->NameLength;k++) {
                            ((PWCHAR)Dest)[k] = (WCHAR)(((PUCHAR)CurrNode->Name)[k]);
                        }
                    } else {
                        RtlCopyMemory(
                            Dest,
                            CurrNode->Name,
                            CurrNode->NameLength
                            );
                    }
                    Dest -= 2;
                    TotalLength += (CurrNode->NameLength +2);
                    ((PWCHAR)Dest)[0] = (WCHAR)'\\';
                    if( CurrCell == HiveToCheck->BaseBlock->RootCell ) {
                        break;
                    }
                    CurrCell = CurrNode->Parent;
                }  
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"%.*S\n",TotalLength/2,Dest));
                
            }
#endif

        }

        HvMarkCellDirty(HiveToCheck, Cell,FALSE);
        pcell->u.KeyNode.SubKeyCounts[Volatile] = 0;
        if( (CheckFlags & CM_CHECK_REGISTRY_LOADER_CLEAN) &&
            (HiveToCheck->Version >= HSYS_WHISTLER_BETA1)
            ) {
            //
            // mark this as bad food
            //
            pcell->u.KeyNode.SubKeyLists[Volatile] = 0xBAADF00D;
        } else {
            //
            // clean it up 
            //
            pcell->u.KeyNode.SubKeyLists[Volatile] = HCELL_NIL;
        }
    }

    return rc;
}

ULONG
CmpCheckValueList(
    PHHIVE      Hive,
    PCELL_DATA  List,
    ULONG       Count,
    HCELL_INDEX KeyCell
    )
/*++

Routine Description:

    Check consistency of a value list.
        .   Each element allocated?
        .   Each element have valid signature?
        .   Data properly allocated?

Arguments:

    Hive - containing Hive.

    List - pointer to an array of HCELL_INDEX entries.

    Count - number of entries in list.

Return Value:

    0 if Hive is OK.  Error return indicator if not.

    RANGE:  5000 - 5999

--*/
{
    ULONG           i = 0,j;
    HCELL_INDEX     Cell;
    PCELL_DATA      pcell;
    ULONG           size;
    ULONG           usedlen;
    ULONG           DataLength;
    HCELL_INDEX     Data;
    ULONG           rc = 0;

    CmpCheckValueListDebug.Hive = Hive;
    CmpCheckValueListDebug.Status = 0;
    CmpCheckValueListDebug.List = List;
    CmpCheckValueListDebug.Index = (ULONG)-1;
    CmpCheckValueListDebug.Cell = 0;   // NOT HCELL_NIL
    CmpCheckValueListDebug.CellPoint = NULL;

    if( FALSE ) {
RemoveThisValue:
        if( CmDoSelfHeal() ) {
            //
            // remove value at index i
            //
            PCM_KEY_NODE    Node;
            Node = (PCM_KEY_NODE)HvGetCell(Hive,KeyCell);
            if( Node == NULL ) {
                return rc;
            }
            HvReleaseCell(Hive,KeyCell);

            if( HvMarkCellDirty(Hive, KeyCell,FALSE) &&
                HvMarkCellDirty(Hive, Node->ValueList.List,FALSE)) {
                Node->ValueList.Count--;
                Count--;
                RtlMoveMemory(&(List->u.KeyList[i]),&(List->u.KeyList[i+1]),(Count - i)*sizeof(HCELL_INDEX));
                rc = 0;
                CmMarkSelfHeal(Hive);
            } else {
                return rc;
            }
        } else {
            return rc;
        } 
    }

    for (; i < Count; i++) {

        //
        // Check out value entry's refs.
        //
        Cell = List->u.KeyList[i];
        if (Cell == HCELL_NIL) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckValueList: List:%p i:%08lx\n", List, i));
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tEntry is null\n"));
            rc = 5010;
            CmpCheckValueListDebug.Status = rc;
            CmpCheckValueListDebug.Index = i;
            CmpCheckValueListDebug.Cell = Cell;
            goto RemoveThisValue;
        }
        if (HvIsCellAllocated(Hive, Cell) == FALSE) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckValueList: List:%p i:%08lx\n", List, i));
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tEntry is not allocated\n"));
            rc = 5020;
            CmpCheckValueListDebug.Status = rc;
            CmpCheckValueListDebug.Index = i;
            CmpCheckValueListDebug.Cell = Cell;
            goto RemoveThisValue;
        } 

        //
        // Check out the value entry itself
        //
        pcell = HvGetCell(Hive, Cell);
        if( pcell == NULL ) {
            //
            // we couldn't map a view for the bin containing this cell
            //
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", Cell));
            CmpCheckValueListDebug.Status = 5099;
            CmpCheckValueListDebug.Index = i;
            CmpCheckValueListDebug.Cell = Cell;
            rc = 5099;
            goto Exit;
        }
        size = HvGetCellSize(Hive, pcell);
        if (pcell->u.KeyValue.Signature != CM_KEY_VALUE_SIGNATURE) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckValueList: List:%p i:%08lx\n", List, i));
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCell:%08lx - invalid value signature\n", Cell));
            rc = 5030;
            CmpCheckValueListDebug.Status = rc;
            CmpCheckValueListDebug.Index = i;
            CmpCheckValueListDebug.Cell = Cell;
            CmpCheckValueListDebug.CellPoint = pcell;
            goto RemoveThisValue;
        }
        usedlen = FIELD_OFFSET(CM_KEY_VALUE, Name) + pcell->u.KeyValue.NameLength;
        if (usedlen > size) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckValueList: List:%p i:%08lx\n", List, i));
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCell:%08lx - value bigger than containing cell\n", Cell));
            rc = 5040;
            CmpCheckValueListDebug.Status = rc;
            CmpCheckValueListDebug.Index = i;
            CmpCheckValueListDebug.Cell = Cell;
            CmpCheckValueListDebug.CellPoint = pcell;
            goto RemoveThisValue;
        }

        //
        // Check out value entry's data
        //
        DataLength = pcell->u.KeyValue.DataLength;
        if (DataLength < CM_KEY_VALUE_SPECIAL_SIZE) {
            Data = pcell->u.KeyValue.Data;
            if ((DataLength == 0) && (Data != HCELL_NIL)) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckValueList: List:%p i:%08lx\n", List, i));
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCell:%08lx Data:%08lx - data not null\n", Cell, Data));
                rc = 5050;
                CmpCheckValueListDebug.Status = rc;
                CmpCheckValueListDebug.Index = i;
                CmpCheckValueListDebug.Cell = Cell;
                CmpCheckValueListDebug.CellPoint = pcell;
                goto RemoveThisValue;
            }
            if (DataLength > 0) {
                if (HvIsCellAllocated(Hive, Data) == FALSE) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckValueList: List:%p i:%08lx\n", List, i));
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCell:%08lx Data:%08lx - unallocated\n", Cell, Data));
                    rc = 5060;
                    CmpCheckValueListDebug.Status = rc;
                    CmpCheckValueListDebug.Index = i;
                    CmpCheckValueListDebug.Cell = Cell;
                    CmpCheckValueListDebug.CellPoint = pcell;
                    goto RemoveThisValue;
                }
            }
            if( CmpIsHKeyValueBig(Hive,DataLength) == TRUE ) {
                PCM_BIG_DATA    BigData;
                PHCELL_INDEX    Plist;

                BigData = (PCM_BIG_DATA)HvGetCell(Hive, Data);
                if( BigData == NULL ) {
                    //
                    // we couldn't map a view for the bin containing this cell
                    //
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", Data));
                    CmpCheckValueListDebug.Status = 5098;
                    CmpCheckValueListDebug.Index = i;
                    CmpCheckValueListDebug.Cell = Data;
                    rc = 5098;
                    goto Exit;
                }
                
                if( (BigData->Signature != CM_BIG_DATA_SIGNATURE) ||
                    (BigData->Count == 0 ) ||
                    (BigData->List == HCELL_NIL) 
                    ) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tinvalid big data cell #%08lx\n", Data));
                    CmpCheckValueListDebug.Status = 5097;
                    CmpCheckValueListDebug.Index = i;
                    CmpCheckValueListDebug.Cell = Data;
                    rc = 5097;
                    goto RemoveThisValue;
                }
                
                if (HvIsCellAllocated(Hive, BigData->List) == FALSE) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckValueList: List:%p i:%08lx\n", List, i));
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCell:%08lx DataList:%08lx - unallocated\n", Cell, BigData->List));
                    rc = 5096;
                    CmpCheckValueListDebug.Status = rc;
                    CmpCheckValueListDebug.Index = i;
                    CmpCheckValueListDebug.Cell = BigData->List;
                    CmpCheckValueListDebug.CellPoint = (PCELL_DATA)BigData;
                    goto RemoveThisValue;
                }

                Plist = (PHCELL_INDEX)HvGetCell(Hive,BigData->List);
                if( Plist == NULL ) {
                    //
                    // we couldn't map a view for the bin containing this cell
                    //
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", BigData->List));
                    CmpCheckValueListDebug.Status = 5098;
                    CmpCheckValueListDebug.Index = i;
                    CmpCheckValueListDebug.Cell = BigData->List;
                    rc = 5095;
                    goto Exit;
                }

                //
                // check each and every big data cell to see if it is allocated.
                // 
                for(j=0;j<BigData->Count;j++) {
                    if (HvIsCellAllocated(Hive, Plist[j]) == FALSE) {
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckValueList: List:%p j:%08lx\n", BigData->List, j));
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCell:%08lx BigData:%08lx - unallocated\n", Plist[j], BigData->List));
                        rc = 5094;
                        CmpCheckValueListDebug.Status = rc;
                        CmpCheckValueListDebug.Index = j;
                        CmpCheckValueListDebug.Cell = Plist[j];
                        CmpCheckValueListDebug.CellPoint = (PCELL_DATA)BigData;
                        goto RemoveThisValue;
                    }
                }
            }

        }
    }

Exit:
    // cleanup
        
    return rc;
}

BOOLEAN
CmpCheckLexicographicalOrder (  IN PHHIVE       HiveToCheck,
                                IN HCELL_INDEX  PriorSibling,
                                IN HCELL_INDEX  Current 
                                )
{
    PCM_KEY_NODE    CurrentNode;
    PCM_KEY_NODE    PriorNode;
    UNICODE_STRING  PriorKeyName;
    UNICODE_STRING  CurrentKeyName;

    PAGED_CODE();


    CurrentNode = (PCM_KEY_NODE)HvGetCell(HiveToCheck, Current);
    PriorNode = (PCM_KEY_NODE)HvGetCell(HiveToCheck, PriorSibling);
    if( (CurrentNode == NULL) || (PriorNode == NULL ) ) {
        return FALSE;
    }
    if(CurrentNode->Flags & KEY_COMP_NAME) { 
        if(PriorNode->Flags & KEY_COMP_NAME) {
            //
            // most usual case
            //
            if( CmpCompareTwoCompressedNames(   PriorNode->Name,
                                                PriorNode->NameLength,
                                                CurrentNode->Name,
                                                CurrentNode->NameLength ) >=0 ) {
                return FALSE;
            }
        } else {
            PriorKeyName.Buffer = &(PriorNode->Name[0]);
            PriorKeyName.Length = PriorNode->NameLength;
            PriorKeyName.MaximumLength = PriorKeyName.Length;
            if( CmpCompareCompressedName(&PriorKeyName,
                                        CurrentNode->Name,
                                        CurrentNode->NameLength,
                                        0) >= 0 ) {
                return FALSE;            
            }

        }
    } else {
        if(PriorNode->Flags & KEY_COMP_NAME) {
            CurrentKeyName.Buffer = &(CurrentNode->Name[0]);
            CurrentKeyName.Length = CurrentNode->NameLength;
            CurrentKeyName.MaximumLength = CurrentKeyName.Length;
            if( CmpCompareCompressedName(&CurrentKeyName,
                                        PriorNode->Name,
                                        PriorNode->NameLength,
                                        0) <= 0 ) {
                return FALSE;            
            }
        } else {
            //
            // worst case: two unicode strings
            //
            PriorKeyName.Buffer = &(PriorNode->Name[0]);
            PriorKeyName.Length = PriorNode->NameLength;
            PriorKeyName.MaximumLength = PriorKeyName.Length;
            CurrentKeyName.Buffer = &(CurrentNode->Name[0]);
            CurrentKeyName.Length = CurrentNode->NameLength;
            CurrentKeyName.MaximumLength = CurrentKeyName.Length;
            if( RtlCompareUnicodeString(&PriorKeyName,
                                        &CurrentKeyName,
                                        TRUE) >= 0 ) {
                return FALSE;
            }

        }
    }
    
    return TRUE;
}

//
// EventLog helpers
//
VOID
CmpLogError(IN NTSTATUS         NtStatusCode)
/*++

Routine Description:

    This routine writes an eventlog entry to the eventlog.

Arguments:

Return Value:


--*/
{
    PIO_ERROR_LOG_PACKET pErrorLog;
    
    CM_PAGED_CODE();

    pErrorLog = (PIO_ERROR_LOG_PACKET) IoAllocateGenericErrorLogEntry(sizeof(IO_ERROR_LOG_PACKET));
    if (pErrorLog) {
        pErrorLog->FinalStatus = NtStatusCode;
        pErrorLog->ErrorCode = NtStatusCode;
        IoWriteErrorLogEntry(pErrorLog);
    }
}
