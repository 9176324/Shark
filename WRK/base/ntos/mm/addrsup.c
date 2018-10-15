/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    addrsup.c

Abstract:

    This module implements a new version of the generic table package
    based on balanced binary trees (later named AVL), as described in
    Knuth, "The Art of Computer Programming, Volume 3, Sorting and Searching",
    and refers directly to algorithms as they are presented in the second
    edition Copyrighted in 1973.

    Used rtl\avltable.c as a starting point, adding the following:

    - Use less memory for structures as these are nonpaged & heavily used.
    - Caller allocates the pool to reduce mutex hold times.
    - Various VAD-specific customizations/optimizations.
    - Hints.

Environment:

    Kernel mode only, working set mutex held, APCs disabled.

--*/

#include "mi.h"

#if !defined (_USERMODE)
#define PRINT 
#define COUNT_BALANCE_MAX(a)
#else
extern MM_AVL_TABLE MmSectionBasedRoot;
#endif

#if (_MSC_VER >= 800)
#pragma warning(disable:4010)        // Allow pretty pictures without the noise
#endif

TABLE_SEARCH_RESULT
MiFindNodeOrParent (
    IN PMM_AVL_TABLE Table,
    IN ULONG_PTR StartingVpn,
    OUT PMMADDRESS_NODE *NodeOrParent
    );

VOID
MiPromoteNode (
    IN PMMADDRESS_NODE C
    );

ULONG
MiRebalanceNode (
    IN PMMADDRESS_NODE S
    );

PMMADDRESS_NODE
MiRealSuccessor (
    IN PMMADDRESS_NODE Links
    );

PMMADDRESS_NODE
MiRealPredecessor (
    IN PMMADDRESS_NODE Links
    );

VOID
MiInitializeVadTableAvl (
    IN PMM_AVL_TABLE Table
    );

PVOID
MiEnumerateGenericTableWithoutSplayingAvl (
    IN PMM_AVL_TABLE Table,
    IN PVOID *RestartKey
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MiCheckForConflictingNode)
#pragma alloc_text(PAGE,MiRealSuccessor)
#pragma alloc_text(PAGE,MiRealPredecessor)
#pragma alloc_text(PAGE,MiInitializeVadTableAvl)
#pragma alloc_text(PAGE,MiFindEmptyAddressRangeInTree)
#pragma alloc_text(PAGE,MiFindEmptyAddressRangeDownTree)
#pragma alloc_text(PAGE,MiFindEmptyAddressRangeDownBasedTree)
#endif

//
// Various Rtl macros that reference Parent use private versions here since
// Parent is overloaded with Balance.
//

//
//  The macro function Parent takes as input a pointer to a splay link in a
//  tree and returns a pointer to the splay link of the parent of the input
//  node.  If the input node is the root of the tree the return value is
//  equal to the input value.
//
//  PRTL_SPLAY_LINKS
//  MiParent (
//      PRTL_SPLAY_LINKS Links
//      );
//

#define MiParent(Links) (               \
    (PRTL_SPLAY_LINKS)(SANITIZE_PARENT_NODE((Links)->u1.Parent)) \
    )

//
//  The macro function IsLeftChild takes as input a pointer to a splay link
//  in a tree and returns TRUE if the input node is the left child of its
//  parent, otherwise it returns FALSE.
//
//  BOOLEAN
//  MiIsLeftChild (
//      PRTL_SPLAY_LINKS Links
//      );
//

#define MiIsLeftChild(Links) (                                   \
    (RtlLeftChild(MiParent(Links)) == (PRTL_SPLAY_LINKS)(Links)) \
    )

//
//  The macro function IsRightChild takes as input a pointer to a splay link
//  in a tree and returns TRUE if the input node is the right child of its
//  parent, otherwise it returns FALSE.
//
//  BOOLEAN
//  MiIsRightChild (
//      PRTL_SPLAY_LINKS Links
//      );
//

#define MiIsRightChild(Links) (                                   \
    (RtlRightChild(MiParent(Links)) == (PRTL_SPLAY_LINKS)(Links)) \
    )



#if DBG

//
// Build a table of the best case efficiency of a balanced binary tree,
// holding the most possible nodes that can possibly be held in a binary
// tree with a given number of levels.  The answer is always (2**n) - 1.
//
// (Used for debug only.)
//

ULONG MiBestCaseFill[33] = {
        0,          1,          3,          7,
        0xf,        0x1f,       0x3f,       0x7f,
        0xff,       0x1ff,      0x3ff,      0x7ff,
        0xfff,      0x1fff,     0x3fff,     0x7fff,
        0xffff,     0x1ffff,    0x3ffff,    0x7ffff,
        0xfffff,    0x1fffff,   0x3fffff,   0x7fffff,
        0xffffff,   0x1ffffff,  0x3ffffff,  0x7ffffff,
        0xfffffff,  0x1fffffff, 0x3fffffff, 0x7fffffff,
        0xffffffff
};

//
// Build a table of the worst case efficiency of a balanced binary tree,
// holding the fewest possible nodes that can possibly be contained in a
// balanced binary tree with the given number of levels.  After the first
// two levels, each level n is obviously occupied by a root node, plus
// one subtree the size of level n-1, and another subtree which is the
// size of n-2, i.e.:
//
//      MiWorstCaseFill[n] = 1 + MiWorstCaseFill[n-1] + MiWorstCaseFill[n-2]
//
// The efficiency of a typical balanced binary tree will normally fall
// between the two extremes, typically closer to the best case.  Note
// however that even with the worst case, it only takes 32 compares to
// find an element in a worst case tree populated with ~3.5M nodes.
//
// Unbalanced trees and splay trees, on the other hand, can and will sometimes
// degenerate to a straight line, requiring on average n/2 compares to
// find a node.
//
// A specific case is one where the nodes are inserted in collated order.
// In this case an unbalanced or a splay tree will generate a straight
// line, yet the balanced binary tree will always create a perfectly
// balanced tree (best-case fill) in this situation.
//
// (Used for debug only.)
//

ULONG MiWorstCaseFill[33] = {
        0,          1,          2,          4,
        7,          12,         20,         33,
        54,         88,         143,        232,
        376,        609,        986,        1596,
        2583,       4180,       6764,       10945,
        17710,      28656,      46367,      75024,
        121392,     196417,     317810,     514228,
        832039,     1346268,    2178308,    3524577,
        5702886
};

#endif


TABLE_SEARCH_RESULT
MiFindNodeOrParent (
    IN PMM_AVL_TABLE Table,
    IN ULONG_PTR StartingVpn,
    OUT PMMADDRESS_NODE *NodeOrParent
    )

/*++

Routine Description:

    This routine is used by all of the routines of the generic
    table package to locate the a node in the tree.  It will
    find and return (via the NodeOrParent parameter) the node
    with the given key, or if that node is not in the tree it
    will return (via the NodeOrParent parameter) a pointer to
    the parent.

Arguments:

    Table - The generic table to search for the key.

    StartingVpn - The starting virtual page number.

    NodeOrParent - Will be set to point to the node containing the
                   the key or what should be the parent of the node
                   if it were in the tree.  Note that this will *NOT*
                   be set if the search result is TableEmptyTree.

Return Value:

    TABLE_SEARCH_RESULT - TableEmptyTree: The tree was empty.  NodeOrParent
                                          is *not* altered.

                          TableFoundNode: A node with the key is in the tree.
                                          NodeOrParent points to that node.

                          TableInsertAsLeft: Node with key was not found.
                                             NodeOrParent points to what would
                                             be parent.  The node would be the
                                             left child.

                          TableInsertAsRight: Node with key was not found.
                                              NodeOrParent points to what would
                                              be parent.  The node would be
                                              the right child.

Environment:

    Kernel mode.  The PFN lock is held for some of the tables.

--*/

{
#if DBG
    ULONG NumberCompares = 0;
#endif
    PMMADDRESS_NODE Child;
    PMMADDRESS_NODE NodeToExamine;

    if (Table->NumberGenericTableElements == 0) {
        return TableEmptyTree;
    }

    NodeToExamine = (PMMADDRESS_NODE) Table->BalancedRoot.RightChild;

    do {

        //
        // Make sure the depth of tree is correct.
        //

        ASSERT(++NumberCompares <= Table->DepthOfTree);

        //
        // Compare the buffer with the key in the tree element.
        //

        if (StartingVpn < NodeToExamine->StartingVpn) {

            Child = NodeToExamine->LeftChild;

            if (Child != NULL) {
                NodeToExamine = Child;
            }
            else {

                //
                // Node is not in the tree.  Set the output
                // parameter to point to what would be its
                // parent and return which child it would be.
                //

                *NodeOrParent = NodeToExamine;
                return TableInsertAsLeft;
            }
        }
        else if (StartingVpn <= NodeToExamine->EndingVpn) {

            //
            // This is the node.
            //

            *NodeOrParent = NodeToExamine;
            return TableFoundNode;
        }
        else {

            Child = NodeToExamine->RightChild;

            if (Child != NULL) {
                NodeToExamine = Child;
            }
            else {

                //
                // Node is not in the tree.  Set the output
                // parameter to point to what would be its
                // parent and return which child it would be.
                //

                *NodeOrParent = NodeToExamine;
                return TableInsertAsRight;
            }
        }

    } while (TRUE);
}


PMMADDRESS_NODE
MiCheckForConflictingNode (
    IN ULONG_PTR StartVpn,
    IN ULONG_PTR EndVpn,
    IN PMM_AVL_TABLE Table
    )

/*++

Routine Description:

    The function determines if any addresses between a given starting and
    ending address is contained within a virtual address descriptor.

Arguments:

    StartVpn - Supplies the virtual address to locate a containing
                      descriptor.

    EndVpn - Supplies the virtual address to locate a containing
                      descriptor.

Return Value:

    Returns a pointer to the first conflicting virtual address descriptor
    if one is found, otherwise a NULL value is returned.

--*/

{
    PMMADDRESS_NODE Node;

    if (Table->NumberGenericTableElements == 0) {
        return NULL;
    }

    Node = (PMMADDRESS_NODE) Table->BalancedRoot.RightChild;
    ASSERT (Node != NULL);

    do {

        if (Node == NULL) {
            return NULL;
        }

        if (StartVpn > Node->EndingVpn) {
            Node = Node->RightChild;
        }
        else if (EndVpn < Node->StartingVpn) {
            Node = Node->LeftChild;
        }
        else {

            //
            // The starting address is less than or equal to the end VA
            // and the ending address is greater than or equal to the
            // start va.  Return this node.
            //

            return Node;
        }

    } while (TRUE);
}


PMMADDRESS_NODE
FASTCALL
MiGetFirstNode (
    IN PMM_AVL_TABLE Table
    )

/*++

Routine Description:

    This function locates the virtual address descriptor which contains
    the address range which logically is first within the address space.

Arguments:

    None.

Return Value:

    Returns a pointer to the virtual address descriptor containing the
    first address range, NULL if none.

--*/

{
    PMMADDRESS_NODE First;

    if (Table->NumberGenericTableElements == 0) {
        return NULL;
    }

    First = (PMMADDRESS_NODE) Table->BalancedRoot.RightChild;

    ASSERT (First != NULL);

    while (First->LeftChild != NULL) {
        First = First->LeftChild;
    }

    return First;
}


VOID
MiPromoteNode (
    IN PMMADDRESS_NODE C
    )

/*++

Routine Description:

    This routine performs the fundamental adjustment required for balancing
    the binary tree during insert and delete operations.  Simply put, the
    designated node is promoted in such a way that it rises one level in
    the tree and its parent drops one level in the tree, becoming now the
    child of the designated node.  Generally the path length to the subtree
    "opposite" the original parent.  Balancing occurs as the caller chooses
    which nodes to promote according to the balanced tree algorithms from
    Knuth.

    This is not the same as a splay operation, typically a splay "promotes"
    a designated node twice.

    Note that the pointer to the root node of the tree is assumed to be
    contained in a MMADDRESS_NODE structure itself, to allow the
    algorithms below to change the root of the tree without checking
    for special cases.  Note also that this is an internal routine,
    and the caller guarantees that it never requests to promote the
    root itself.

    This routine only updates the tree links; the caller must update
    the balance factors as appropriate.

Arguments:

    C - pointer to the child node to be promoted in the tree.

Return Value:

    None.

--*/

{
    PMMADDRESS_NODE P;
    PMMADDRESS_NODE G;

    //
    // Capture the current parent and grandparent (may be the root).
    //

    P = SANITIZE_PARENT_NODE (C->u1.Parent);
    G = SANITIZE_PARENT_NODE (P->u1.Parent);

    //
    // Break down the promotion into two cases based upon whether C
    // is a left or right child.
    //

    if (P->LeftChild == C) {

        //
        // This promotion looks like this:
        //
        //          G           G
        //          |           |
        //          P           C
        //         / \   =>    / \
        //        C   z       x   P
        //       / \             / \
        //      x   y           y   z
        //

        P->LeftChild = C->RightChild;

        if (P->LeftChild != NULL) {

            P->LeftChild->u1.Parent = MI_MAKE_PARENT (P, P->LeftChild->u1.Balance);
        }

        C->RightChild = P;

        //
        // Fall through to update parent and G <-> C relationship in
        // common code.
        //

    }
    else {

        ASSERT(P->RightChild == C);

        //
        // This promotion looks like this:
        //
        //        G               G
        //        |               |
        //        P               C
        //       / \     =>      / \
        //      x   C           P   z
        //         / \         / \
        //        y   z       x   y
        //

        P->RightChild = C->LeftChild;

        if (P->RightChild != NULL) {
            P->RightChild->u1.Parent = MI_MAKE_PARENT (P, P->RightChild->u1.Balance);
        }

        C->LeftChild = P;
    }

    //
    // Update parent of P, for either case above.
    //

    P->u1.Parent = MI_MAKE_PARENT (C, P->u1.Balance);

    //
    // Finally update G <-> C links for either case above.
    //

    if (G->LeftChild == P) {
        G->LeftChild = C;
    }
    else {
        ASSERT(G->RightChild == P);
        G->RightChild = C;
    }
    C->u1.Parent = MI_MAKE_PARENT (G, C->u1.Balance);
}


ULONG
MiRebalanceNode (
    IN PMMADDRESS_NODE S
    )

/*++

Routine Description:

    This routine performs a rebalance around the input node S, for which the
    Balance factor has just effectively become +2 or -2.  When called, the
    Balance factor still has a value of +1 or -1, but the respective longer
    side has just become one longer as the result of an insert or delete
    operation.

    This routine effectively implements steps A7.iii (test for Case 1 or
    Case 2) and steps A8 and A9 of Knuth's balanced insertion algorithm,
    plus it handles Case 3 identified in the delete section, which can
    only happen on deletes.

    The trick is, to convince yourself that while traveling from the
    insertion point at the bottom of the tree up, that there are only
    these two cases, and that when traveling up from the deletion point,
    that there are just these three cases.  Knuth says it is obvious!

Arguments:

    S - pointer to the node which has just become unbalanced.

Return Value:

    TRUE if Case 3 was detected (causes delete algorithm to terminate).

Environment:

    Kernel mode.  The PFN lock is held for some of the tables.

--*/

{
    PMMADDRESS_NODE R, P;
    SCHAR a;

    PRINT("rebalancing node %p bal=%x start=%x end=%x\n",
                    S,
                    S->u1.Balance,
                    S->StartingVpn,
                    S->EndingVpn);

    //
    // The parent node is never the argument node.
    //

    ASSERT (SANITIZE_PARENT_NODE(S->u1.Parent) != S);

    //
    // Capture which side is unbalanced.
    //

    a = (SCHAR) S->u1.Balance;

    if (a == +1) {
        R = S->RightChild;
    }
    else {
        R = S->LeftChild;
    }

    //
    // If the balance of R and S are the same (Case 1 in Knuth) then a single
    // promotion of R will do the single rotation.  (Step A8, A10)
    //
    // Here is a diagram of the Case 1 transformation, for a == +1 (a mirror
    // image transformation occurs when a == -1), and where the subtree
    // heights are h and h+1 as shown (++ indicates the node out of balance):
    //
    //                  |                   |
    //                  S++                 R
    //                 / \                 / \
    //               (h)  R+     ==>      S  (h+1)
    //                   / \             / \
    //                 (h) (h+1)       (h) (h)
    //
    // Note that on an insert we can hit this case by inserting an item in the
    // right subtree of R.  The original height of the subtree before the insert
    // was h+2, and it is still h+2 after the rebalance, so insert rebalancing
    // may terminate.
    //
    // On a delete we can hit this case by deleting a node from the left subtree
    // of S.  The height of the subtree before the delete was h+3, and after the
    // rebalance it is h+2, so rebalancing must continue up the tree.
    //

    if ((SCHAR) R->u1.Balance == a) {

        MiPromoteNode (R);
        R->u1.Balance = 0;
        S->u1.Balance = 0;

        return FALSE;
    }

    //
    // Otherwise, we have to promote the appropriate child of R twice (Case 2
    // in Knuth).  (Step A9, A10)
    //
    // Here is a diagram of the Case 2 transformation, for a == +1 (a mirror
    // image transformation occurs when a == -1), and where the subtree
    // heights are h and h-1 as shown.  There are actually two minor subcases,
    // differing only in the original balance of P (++ indicates the node out
    // of balance).
    //
    //                  |                   |
    //                  S++                 P
    //                 / \                 / \
    //                /   \               /   \
    //               /     \             /     \
    //             (h)      R-   ==>    S-      R
    //                     / \         / \     / \
    //                    P+ (h)     (h)(h-1)(h) (h)
    //                   / \
    //               (h-1) (h)
    //
    //
    //                  |                   |
    //                  S++                 P
    //                 / \                 / \
    //                /   \               /   \
    //               /     \             /     \
    //             (h)      R-   ==>    S       R+
    //                     / \         / \     / \
    //                    P- (h)     (h) (h)(h-1)(h)
    //                   / \
    //                 (h) (h-1)
    //
    // Note that on an insert we can hit this case by inserting an item in the
    // left subtree of R.  The original height of the subtree before the insert
    // was h+2, and it is still h+2 after the rebalance, so insert rebalancing
    // may terminate.
    //
    // On a delete we can hit this case by deleting a node from the left subtree
    // of S.  The height of the subtree before the delete was h+3, and after the
    // rebalance it is h+2, so rebalancing must continue up the tree.
    //

    if ((SCHAR) R->u1.Balance == -a) {

        //
        // Pick up the appropriate child P for the double rotation (Link(-a,R)).
        //

        if (a == 1) {
            P = R->LeftChild;
        }
        else {
            P = R->RightChild;
        }

        //
        // Promote him twice to implement the double rotation.
        //

        MiPromoteNode (P);
        MiPromoteNode (P);

        //
        // Now adjust the balance factors.
        //

        S->u1.Balance = 0;
        R->u1.Balance = 0;
        if ((SCHAR) P->u1.Balance == a) {
            PRINT("REBADJ A: Node %p, Bal %x -> %x\n", S, S->u1.Balance, -a);
            COUNT_BALANCE_MAX ((SCHAR)-a);
            S->u1.Balance = (ULONG_PTR) -a;
        }
        else if ((SCHAR) P->u1.Balance == -a) {
            PRINT("REBADJ B: Node %p, Bal %x -> %x\n", R, R->u1.Balance, a);
            COUNT_BALANCE_MAX ((SCHAR)a);
            R->u1.Balance = (ULONG_PTR) a;
        }

        P->u1.Balance = 0;
        return FALSE;
    }

    //
    // Otherwise this is Case 3 which can only happen on Delete (identical
    // to Case 1 except R->u1.Balance == 0).  We do a single rotation, adjust
    // the balance factors appropriately, and return TRUE.  Note that the
    // balance of S stays the same.
    //
    // Here is a diagram of the Case 3 transformation, for a == +1 (a mirror
    // image transformation occurs when a == -1), and where the subtree
    // heights are h and h+1 as shown (++ indicates the node out of balance):
    //
    //                  |                   |
    //                  S++                 R-
    //                 / \                 / \
    //               (h)  R      ==>      S+ (h+1)
    //                   / \             / \
    //                (h+1)(h+1)       (h) (h+1)
    //
    // This case can not occur on an insert, because it is impossible for
    // a single insert to balance R, yet somehow grow the right subtree of
    // S at the same time.  As we move up the tree adjusting balance factors
    // after an insert, we terminate the algorithm if a node becomes balanced,
    // because that means the subtree length did not change!
    //
    // On a delete we can hit this case by deleting a node from the left
    // subtree of S.  The height of the subtree before the delete was h+3,
    // and after the rebalance it is still h+3, so rebalancing may terminate
    // in the delete path.
    //

    MiPromoteNode (R);
    PRINT("REBADJ C: Node %p, Bal %x -> %x\n", R, R->u1.Balance, -a);
    COUNT_BALANCE_MAX ((SCHAR)-a);
    R->u1.Balance = -a;
    return TRUE;
}


VOID
FASTCALL
MiRemoveNode (
    IN PMMADDRESS_NODE NodeToDelete,
    IN PMM_AVL_TABLE Table
    )

/*++

Routine Description:

    This routine deletes the specified node from the balanced tree, rebalancing
    as necessary.  If the NodeToDelete has at least one NULL child pointers,
    then it is chosen as the EasyDelete, otherwise a subtree predecessor or
    successor is found as the EasyDelete.  In either case the EasyDelete is
    deleted and the tree is rebalanced.  Finally if the NodeToDelete was
    different than the EasyDelete, then the EasyDelete is linked back into the
    tree in place of the NodeToDelete.

Arguments:

    NodeToDelete - Pointer to the node which the caller wishes to delete.

    Table - The generic table in which the delete is to occur.

Return Value:

    None.

Environment:

    Kernel mode.  The PFN lock is held for some of the tables.

--*/

{
    PMMADDRESS_NODE Parent;
    PMMADDRESS_NODE EasyDelete;
    PMMADDRESS_NODE P;
    SCHAR a;

    //
    // If the NodeToDelete has at least one NULL child pointer, then we can
    // delete it directly.
    //

    if ((NodeToDelete->LeftChild == NULL) ||
        (NodeToDelete->RightChild == NULL)) {

        EasyDelete = NodeToDelete;
    }

    //
    // Otherwise, we may as well pick the longest side to delete from (if one is
    // is longer), as that reduces the probability that we will have to
    // rebalance.
    //

    else if ((SCHAR) NodeToDelete->u1.Balance >= 0) {

        //
        // Pick up the subtree successor.
        //

        EasyDelete = NodeToDelete->RightChild;
        while (EasyDelete->LeftChild != NULL) {
            EasyDelete = EasyDelete->LeftChild;
        }
    }
    else {

        //
        // Pick up the subtree predecessor.
        //

        EasyDelete = NodeToDelete->LeftChild;
        while (EasyDelete->RightChild != NULL) {
            EasyDelete = EasyDelete->RightChild;
        }
    }

    //
    // Rebalancing must know which side of the first parent the delete occurred
    // on.  Assume it is the left side and otherwise correct below.
    //

    a = -1;

    //
    // Now we can do the simple deletion for the no left child case.
    //

    if (EasyDelete->LeftChild == NULL) {

        Parent = SANITIZE_PARENT_NODE (EasyDelete->u1.Parent);

        if (MiIsLeftChild(EasyDelete)) {
            Parent->LeftChild = EasyDelete->RightChild;
        }
        else {
            Parent->RightChild = EasyDelete->RightChild;
            a = 1;
        }

        if (EasyDelete->RightChild != NULL) {
            EasyDelete->RightChild->u1.Parent = MI_MAKE_PARENT (Parent, EasyDelete->RightChild->u1.Balance);
        }

    //
    // Now we can do the simple deletion for the no right child case,
    // plus we know there is a left child.
    //

    }
    else {

        Parent = SANITIZE_PARENT_NODE (EasyDelete->u1.Parent);

        if (MiIsLeftChild(EasyDelete)) {
            Parent->LeftChild = EasyDelete->LeftChild;
        }
        else {
            Parent->RightChild = EasyDelete->LeftChild;
            a = 1;
        }

        EasyDelete->LeftChild->u1.Parent = MI_MAKE_PARENT (Parent,
                                            EasyDelete->LeftChild->u1.Balance);
    }

    //
    // For delete rebalancing, set the balance at the root to 0 to properly
    // terminate the rebalance without special tests, and to be able to detect
    // if the depth of the tree actually decreased.
    //

    Table->BalancedRoot.u1.Balance = 0;
    P = SANITIZE_PARENT_NODE (EasyDelete->u1.Parent);

    //
    // Loop until the tree is balanced.
    //

    while (TRUE) {

        //
        // First handle the case where the tree became more balanced.  Zero
        // the balance factor, calculate a for the next loop and move on to
        // the parent.
        //

        if ((SCHAR) P->u1.Balance == a) {

            P->u1.Balance = 0;

        //
        // If this node is curently balanced, we can show it is now unbalanced
        // and terminate the scan since the subtree length has not changed.
        // (This may be the root, since we set Balance to 0 above!)
        //

        }
        else if (P->u1.Balance == 0) {

            PRINT("REBADJ D: Node %p, Bal %x -> %x\n", P, P->u1.Balance, -a);
            COUNT_BALANCE_MAX ((SCHAR)-a);
            P->u1.Balance = -a;

            //
            // If we shortened the depth all the way back to the root, then
            // the tree really has one less level.
            //

            if (Table->BalancedRoot.u1.Balance != 0) {
                Table->DepthOfTree -= 1;
            }

            break;

        //
        // Otherwise we made the short side 2 levels less than the long side,
        // and rebalancing is required.  On return, some node has been promoted
        // to above node P.  If Case 3 from Knuth was not encountered, then we
        // want to effectively resume rebalancing from P's original parent which
        // is effectively its grandparent now.
        //

        }
        else {

            //
            // We are done if Case 3 was hit, i.e., the depth of this subtree is
            // now the same as before the delete.
            //

            if (MiRebalanceNode(P)) {
                break;
            }

            P = SANITIZE_PARENT_NODE (P->u1.Parent);
        }

        a = -1;
        if (MiIsRightChild(P)) {
            a = 1;
        }
        P = SANITIZE_PARENT_NODE (P->u1.Parent);
    }

    //
    // Finally, if we actually deleted a predecessor/successor of the
    // NodeToDelete, we will link him back into the tree to replace
    // NodeToDelete before returning.  Note that NodeToDelete did have
    // both child links filled in, but that may no longer be the case
    // at this point.
    //

    if (NodeToDelete != EasyDelete) {

        //
        // Note carefully - VADs are of differing sizes therefore it is not safe
        // to just overlay the EasyDelete node with the NodeToDelete like the
        // rtl avl code does.
        //
        // Copy just the links, preserving the rest of the original EasyDelete
        // VAD.
        //

        EasyDelete->u1.Parent = NodeToDelete->u1.Parent;
        EasyDelete->LeftChild = NodeToDelete->LeftChild;
        EasyDelete->RightChild = NodeToDelete->RightChild;

        if (MiIsLeftChild(NodeToDelete)) {
            Parent = SANITIZE_PARENT_NODE (EasyDelete->u1.Parent);
            Parent->LeftChild = EasyDelete;
        }
        else {
            ASSERT(MiIsRightChild(NodeToDelete));
            Parent = SANITIZE_PARENT_NODE (EasyDelete->u1.Parent);
            Parent->RightChild = EasyDelete;
        }
        if (EasyDelete->LeftChild != NULL) {
            EasyDelete->LeftChild->u1.Parent = MI_MAKE_PARENT (EasyDelete,
                                            EasyDelete->LeftChild->u1.Balance);
        }
        if (EasyDelete->RightChild != NULL) {
            EasyDelete->RightChild->u1.Parent = MI_MAKE_PARENT (EasyDelete,
                                            EasyDelete->RightChild->u1.Balance);
        }
    }

    Table->NumberGenericTableElements -= 1;

    //
    // Sanity check tree size and depth.
    //

    ASSERT((Table->NumberGenericTableElements >= MiWorstCaseFill[Table->DepthOfTree]) &&
           (Table->NumberGenericTableElements <= MiBestCaseFill[Table->DepthOfTree]));

    return;
}


PMMADDRESS_NODE
MiRealSuccessor (
    IN PMMADDRESS_NODE Links
    )

/*++

Routine Description:

    This function takes as input a pointer to a balanced link
    in a tree and returns a pointer to the successor of the input node within
    the entire tree.  If there is not a successor, the return value is NULL.

Arguments:

    Links - Supplies a pointer to a balanced link in a tree.

Return Value:

    PMMADDRESS_NODE - returns a pointer to the successor in the entire tree

--*/

{
    PMMADDRESS_NODE Ptr;

    /*
        First check to see if there is a right subtree to the input link
        if there is then the real successor is the left most node in
        the right subtree.  That is find and return S in the following diagram

                  Links
                     \
                      .
                     .
                    .
                   /
                  S
                   \
    */

    if ((Ptr = Links->RightChild) != NULL) {

        while (Ptr->LeftChild != NULL) {
            Ptr = Ptr->LeftChild;
        }

        return Ptr;
    }

    /*
        We do not have a right child so check to see if have a parent and if
        so find the first ancestor that we are a left decendant of. That
        is find and return S in the following diagram

                       S
                      /
                     .
                      .
                       .
                      Links

        Note that this code depends on how the BalancedRoot is initialized,
        which is Parent points to self, and the RightChild points to an
        actual node which is the root of the tree, and LeftChild does not
        point to self.
    */

    Ptr = Links;
    while (MiIsRightChild(Ptr)) {
        Ptr = SANITIZE_PARENT_NODE (Ptr->u1.Parent);
    }

    if (MiIsLeftChild(Ptr)) {
        return SANITIZE_PARENT_NODE (Ptr->u1.Parent);
    }

    //
    // Otherwise we are do not have a real successor so we simply return NULL.
    //
    // This can only occur when we get back to the root, and we can tell
    // that since the Root is its own parent.
    //

    ASSERT (SANITIZE_PARENT_NODE(Ptr->u1.Parent) == Ptr);

    return NULL;
}


PMMADDRESS_NODE
MiRealPredecessor (
    IN PMMADDRESS_NODE Links
    )

/*++

Routine Description:

    The RealPredecessor function takes as input a pointer to a balanced link
    in a tree and returns a pointer to the predecessor of the input node
    within the entire tree.  If there is not a predecessor, the return value
    is NULL.

Arguments:

    Links - Supplies a pointer to a balanced link in a tree.

Return Value:

    PMMADDRESS_NODE - returns a pointer to the predecessor in the entire tree

--*/

{
    PMMADDRESS_NODE Ptr;
    PMMADDRESS_NODE Parent;
    PMMADDRESS_NODE GrandParent;

    /*
      First check to see if there is a left subtree to the input link
      if there is then the real predecessor is the right most node in
      the left subtree.  That is find and return P in the following diagram

                  Links
                   /
                  .
                   .
                    .
                     P
                    /
    */

    if ((Ptr = Links->LeftChild) != NULL) {

        while (Ptr->RightChild != NULL) {
            Ptr = Ptr->RightChild;
        }

        return Ptr;

    }

    /*
      We do not have a left child so check to see if have a parent and if
      so find the first ancestor that we are a right decendant of. That
      is find and return P in the following diagram

                       P
                        \
                         .
                        .
                       .
                    Links

        Note that this code depends on how the BalancedRoot is initialized,
        which is Parent points to self, and the RightChild points to an
        actual node which is the root of the tree.
    */

    Ptr = Links;
    while (MiIsLeftChild(Ptr)) {
        Ptr = SANITIZE_PARENT_NODE (Ptr->u1.Parent);
    }

    if (MiIsRightChild(Ptr)) {
        Parent = SANITIZE_PARENT_NODE (Ptr->u1.Parent);
        GrandParent = SANITIZE_PARENT_NODE (Parent->u1.Parent);
        if (GrandParent != Parent) {
            return Parent;
        }
    }

    //
    // Otherwise we are do not have a real predecessor so we simply return
    // NULL.
    //

    return NULL;
}


VOID
MiInitializeVadTableAvl (
    IN PMM_AVL_TABLE Table
    )

/*++

Routine Description:

    This routine initializes a table.

Arguments:

    Table - Pointer to the generic table to be initialized.

Return Value:

    None.

--*/

{

#if DBG
    ULONG i;

    for (i = 2; i < 33; i += 1) {
        ASSERT(MiWorstCaseFill[i] == (1 + MiWorstCaseFill[i - 1] + MiWorstCaseFill[i - 2]));
    }
#endif

    //
    // Initialize each field in the argument Table.
    //

    RtlZeroMemory (Table, sizeof(MM_AVL_TABLE));

    Table->BalancedRoot.u1.Parent = MI_MAKE_PARENT (&Table->BalancedRoot, 0);
}


VOID
FASTCALL
MiInsertNode (
    IN PMMADDRESS_NODE NodeToInsert,
    IN PMM_AVL_TABLE Table
    )

/*++

Routine Description:

    This function inserts a new element in a table.

Arguments:

    NodeToInsert - The initialized address node to insert.

    Table - Pointer to the table in which to insert the new node.

Return Value:

    None.

Environment:

    Kernel mode.  The PFN lock is held for some of the tables.

--*/

{
    //
    // Holds a pointer to the node in the table or what would be the
    // parent of the node.
    //

    PMMADDRESS_NODE NodeOrParent;
    TABLE_SEARCH_RESULT SearchResult;

    ASSERT((Table->NumberGenericTableElements >= MiWorstCaseFill[Table->DepthOfTree]) &&
           (Table->NumberGenericTableElements <= MiBestCaseFill[Table->DepthOfTree]));

    SearchResult = MiFindNodeOrParent (Table,
                                       NodeToInsert->StartingVpn,
                                       &NodeOrParent);

    ASSERT (SearchResult != TableFoundNode);

    //
    // The node wasn't in the (possibly empty) tree.
    //
    // We just check that the table isn't getting too big.
    //

    ASSERT (Table->NumberGenericTableElements != (MAXULONG-1));

    NodeToInsert->LeftChild = NULL;
    NodeToInsert->RightChild = NULL;

    Table->NumberGenericTableElements += 1;

    //
    // Insert the new node in the tree.
    //

    if (SearchResult == TableEmptyTree) {

        Table->BalancedRoot.RightChild = NodeToInsert;
        NodeToInsert->u1.Parent = &Table->BalancedRoot;
        ASSERT (NodeToInsert->u1.Balance == 0);
        ASSERT(Table->DepthOfTree == 0);
        Table->DepthOfTree = 1;

    ASSERT((Table->NumberGenericTableElements >= MiWorstCaseFill[Table->DepthOfTree]) &&
           (Table->NumberGenericTableElements <= MiBestCaseFill[Table->DepthOfTree]));

    }
    else {

        PMMADDRESS_NODE R = NodeToInsert;
        PMMADDRESS_NODE S = NodeOrParent;

        if (SearchResult == TableInsertAsLeft) {
            NodeOrParent->LeftChild = NodeToInsert;
        }
        else {
            NodeOrParent->RightChild = NodeToInsert;
        }

        NodeToInsert->u1.Parent = NodeOrParent;
        ASSERT (NodeToInsert->u1.Balance == 0);

        //
        // The above completes the standard binary tree insertion, which
        // happens to correspond to steps A1-A5 of Knuth's "balanced tree
        // search and insertion" algorithm.  Now comes the time to adjust
        // balance factors and possibly do a single or double rotation as
        // in steps A6-A10.
        //
        // Set the Balance factor in the root to a convenient value
        // to simplify loop control.
        //

        PRINT("REBADJ E: Table %p, Bal %x -> %x\n", Table, Table->BalancedRoot.u1.Balance, -1);
        COUNT_BALANCE_MAX ((SCHAR)-1);
        Table->BalancedRoot.u1.Balance = (ULONG_PTR) -1;

        //
        // Now loop to adjust balance factors and see if any balance operations
        // must be performed, using NodeOrParent to ascend the tree.
        //

        do {

            SCHAR a;

            //
            // Calculate the next adjustment.
            //

            a = 1;
            if (MiIsLeftChild (R)) {
                a = -1;
            }

            PRINT("LW 0: Table %p, Bal %x, %x\n", Table, Table->BalancedRoot.u1.Balance, a);
            PRINT("LW 0: R Node %p, Bal %x, %x\n", R, R->u1.Balance, 1);
            PRINT("LW 0: S Node %p, Bal %x, %x\n", S, S->u1.Balance, 1);

            //
            // If this node was balanced, show that it is no longer and
            // keep looping.  This is essentially A6 of Knuth's algorithm,
            // where he updates all of the intermediate nodes on the
            // insertion path which previously had balance factors of 0.
            // We are looping up the tree via Parent pointers rather than
            // down the tree as in Knuth.
            //

            if (S->u1.Balance == 0) {

                PRINT("REBADJ F: Node %p, Bal %x -> %x\n", S, S->u1.Balance, a);
                COUNT_BALANCE_MAX ((SCHAR)a);
                S->u1.Balance = a;
                R = S;
                S = SANITIZE_PARENT_NODE (S->u1.Parent);
            }
            else if ((SCHAR) S->u1.Balance != a) {

                PRINT("LW 1: Table %p, Bal %x, %x\n", Table, Table->BalancedRoot.u1.Balance, -1);

                //
                // If this node has the opposite balance, then the tree got
                // more balanced (or we hit the root) and we are done.
                //
                // Step A7.ii
                //

                S->u1.Balance = 0;

                //
                // If S is actually the root, then this means the depth
                // of the tree just increased by 1!  (This is essentially
                // A7.i, but we just initialized the root balance to force
                // it through here.)
                //

                if (Table->BalancedRoot.u1.Balance == 0) {
                    Table->DepthOfTree += 1;
                }

                break;
            }
            else {

                PRINT("LW 2: Table %p, Bal %x, %x\n", Table, Table->BalancedRoot.u1.Balance, -1);

                //
                // The tree became unbalanced (path length differs
                // by 2 below us) and we need to do one of the balancing
                // operations, and then we are done.  The RebalanceNode routine
                // does steps A7.iii, A8 and A9.
                //

                MiRebalanceNode (S);
                break;
            }
            PRINT("LW 3: Table %p, Bal %x, %x\n", Table, Table->BalancedRoot.u1.Balance, -1);
        } while (TRUE);
        PRINT("LW 4: Table %p, Bal %x, %x\n", Table, Table->BalancedRoot.u1.Balance, -1);
    }

    //
    // Sanity check tree size and depth.
    //

    ASSERT((Table->NumberGenericTableElements >= MiWorstCaseFill[Table->DepthOfTree]) &&
           (Table->NumberGenericTableElements <= MiBestCaseFill[Table->DepthOfTree]));

    return;
}


PVOID
MiEnumerateGenericTableWithoutSplayingAvl (
    IN PMM_AVL_TABLE Table,
    IN PVOID *RestartKey
    )

/*++

Routine Description:

    The function EnumerateGenericTableWithoutSplayingAvl will return to the
    caller one-by-one the elements of of a table.  The return value is a
    pointer to the user defined structure associated with the element.
    The input parameter RestartKey indicates if the enumeration should
    start from the beginning or should return the next element.  If the
    are no more new elements to return the return value is NULL.  As an
    example of its use, to enumerate all of the elements in a table the
    user would write:

        *RestartKey = NULL;

        for (ptr = EnumerateGenericTableWithoutSplayingAvl(Table, &RestartKey);
             ptr != NULL;
             ptr = EnumerateGenericTableWithoutSplayingAvl(Table, &RestartKey)) {
                :
        }

Arguments:

    Table - Pointer to the generic table to enumerate.

    RestartKey - Pointer that indicates if we should restart or return the next
                element.  If the contents of RestartKey is NULL, the search
                will be started from the beginning.

Return Value:

    PVOID - Pointer to the user data.

--*/

{
    PMMADDRESS_NODE NodeToReturn;

    if (Table->NumberGenericTableElements == 0) {

        //
        // Nothing to do if the table is empty.
        //

        return NULL;

    }

    //
    // If the restart flag is true then go to the least element
    // in the tree.
    //

    if (*RestartKey == NULL) {

        //
        // Loop until we find the leftmost child of the root.
        //

        for (NodeToReturn = Table->BalancedRoot.RightChild;
             NodeToReturn->LeftChild;
             NodeToReturn = NodeToReturn->LeftChild) {

            NOTHING;
        }

        *RestartKey = NodeToReturn;

    }
    else {

        //
        // The caller has passed in the previous entry found
        // in the table to enable us to continue the search.  We call
        // RealSuccessor to step to the next element in the tree.
        //

        NodeToReturn = MiRealSuccessor (*RestartKey);

        if (NodeToReturn) {
            *RestartKey = NodeToReturn;
        }
    }

    //
    // Return the found element.
    //

    return NodeToReturn;
}


PMMADDRESS_NODE
FASTCALL
MiGetNextNode (
    IN PMMADDRESS_NODE Node
    )

/*++

Routine Description:

    This function locates the virtual address descriptor which contains
    the address range which logically follows the specified address range.

Arguments:

    Node - Supplies a pointer to a virtual address descriptor.

Return Value:

    Returns a pointer to the virtual address descriptor containing the
    next address range, NULL if none.

--*/

{
    PMMADDRESS_NODE Next;
    PMMADDRESS_NODE Parent;
    PMMADDRESS_NODE Left;

    Next = Node;

    if (Next->RightChild == NULL) {

        do {
                
            Parent = SANITIZE_PARENT_NODE (Next->u1.Parent);

            ASSERT (Parent != NULL);

            if (Parent == Next) {
                return NULL;
            }

            //
            // Locate the first ancestor of this node of which this
            // node is the left child of and return that node as the
            // next element.
            //

            if (Parent->LeftChild == Next) {
                return Parent;
            }

            Next = Parent;

        } while (TRUE);
    }

    //
    // A right child exists, locate the left most child of that right child.
    //

    Next = Next->RightChild;

    do {

        Left = Next->LeftChild;

        if (Left == NULL) {
            break;
        }

        Next = Left;

    } while (TRUE);

    return Next;

}

PMMADDRESS_NODE
FASTCALL
MiGetPreviousNode (
    IN PMMADDRESS_NODE Node
    )

/*++

Routine Description:

    This function locates the virtual address descriptor which contains
    the address range which logically precedes the specified virtual
    address descriptor.

Arguments:

    Node - Supplies a pointer to a virtual address descriptor.

Return Value:

    Returns a pointer to the virtual address descriptor containing the
    next address range, NULL if none.

--*/

{
    PMMADDRESS_NODE Previous;
    PMMADDRESS_NODE Parent;

    Previous = Node;

    if (Previous->LeftChild == NULL) {

        ASSERT (Previous->u1.Parent != NULL);

        Parent = SANITIZE_PARENT_NODE (Previous->u1.Parent);

        while (Parent != Previous) {

            //
            // Locate the first ancestor of this node of which this
            // node is the right child of and return that node as the
            // Previous element.
            //

            if (Parent->RightChild == Previous) {

                if (Parent == SANITIZE_PARENT_NODE (Parent->u1.Parent)) {
                    return NULL;
                }

                return Parent;
            }

            Previous = Parent;
            Parent = SANITIZE_PARENT_NODE (Previous->u1.Parent);
        }
        return NULL;
    }

    //
    // A left child exists, locate the right most child of that left child.
    //

    Previous = Previous->LeftChild;

    while (Previous->RightChild != NULL) {
        Previous = Previous->RightChild;
    }

    return Previous;
}


PMMADDRESS_NODE
FASTCALL
MiLocateAddressInTree (
    IN ULONG_PTR Vpn,
    IN PMM_AVL_TABLE Table
    )

/*++

Routine Description:

    The function locates the virtual address descriptor which describes
    a given address.

Arguments:

    Vpn - Supplies the virtual page number to locate a descriptor for.

Return Value:

    Returns a pointer to the virtual address descriptor which contains
    the supplied virtual address or NULL if none was located.

--*/

{
    PVOID NodeOrParent;
    TABLE_SEARCH_RESULT SearchResult;

    //
    // Lookup the element and save the result.
    //

    SearchResult = MiFindNodeOrParent (Table,
                                       Vpn,
                                       (PMMADDRESS_NODE *) &NodeOrParent);

    if (SearchResult == TableFoundNode) {

        //
        // Return the VAD.
        //

        return (PMMADDRESS_NODE) NodeOrParent;
    }

    return NULL;
}


NTSTATUS
MiFindEmptyAddressRangeInTree (
    IN SIZE_T SizeOfRange,
    IN ULONG_PTR Alignment,
    IN PMM_AVL_TABLE Table,
    OUT PMMADDRESS_NODE *PreviousVad,
    OUT PVOID *Base
    )

/*++

Routine Description:

    The function examines the virtual address descriptors to locate
    an unused range of the specified size and returns the starting
    address of the range.

Arguments:

    SizeOfRange - Supplies the size in bytes of the range to locate.

    Alignment - Supplies the alignment for the address.  Must be
                 a power of 2 and greater than the page_size.

    Table - Supplies the root of the tree to search through.

    PreviousVad - Supplies the Vad which is before this the found
                  address range.

    Base - Receives the starting address of a suitable range on success.

Return Value:

    NTSTATUS.

--*/

{
    PMMADDRESS_NODE Node;
    PMMADDRESS_NODE NextNode;
    ULONG_PTR AlignmentVpn;
    ULONG_PTR SizeOfRangeVpn;

    AlignmentVpn = Alignment >> PAGE_SHIFT;

    //
    // Locate the node with the lowest starting address.
    //

    ASSERT (SizeOfRange != 0);
    SizeOfRangeVpn = (SizeOfRange + (PAGE_SIZE - 1)) >> PAGE_SHIFT;
    ASSERT (SizeOfRangeVpn != 0);

    if (Table->NumberGenericTableElements == 0) {
        *Base = MM_LOWEST_USER_ADDRESS;
        return STATUS_SUCCESS;
    }

    Node = Table->BalancedRoot.RightChild;

    while (Node->LeftChild != NULL) {
        Node = Node->LeftChild;
    }

    //
    // Check to see if a range exists between the lowest address VAD
    // and lowest user address.
    //

    if (Node->StartingVpn > MI_VA_TO_VPN (MM_LOWEST_USER_ADDRESS)) {

        if (SizeOfRangeVpn <
            (Node->StartingVpn - MI_VA_TO_VPN (MM_LOWEST_USER_ADDRESS))) {

            *PreviousVad = NULL;
            *Base = MM_LOWEST_USER_ADDRESS;
            return STATUS_SUCCESS;
        }
    }

    do {

        NextNode = MiGetNextNode (Node);

        if (NextNode != NULL) {

            if (SizeOfRangeVpn <=
                ((ULONG_PTR)NextNode->StartingVpn -
                                MI_ROUND_TO_SIZE(1 + Node->EndingVpn,
                                                 AlignmentVpn))) {

                //
                // Check to ensure that the ending address aligned upwards
                // is not greater than the starting address.
                //

                if ((ULONG_PTR)NextNode->StartingVpn >
                        MI_ROUND_TO_SIZE(1 + Node->EndingVpn,
                                         AlignmentVpn)) {

                    *PreviousVad = Node;
                    *Base = (PVOID) MI_ROUND_TO_SIZE(
                                (ULONG_PTR)MI_VPN_TO_VA_ENDING(Node->EndingVpn),
                                    Alignment);
                    return STATUS_SUCCESS;
                }
            }

        } else {

            //
            // No more descriptors, check to see if this fits into the remainder
            // of the address space.
            //

            if ((((ULONG_PTR)Node->EndingVpn + MI_VA_TO_VPN(X64K)) <
                    MI_VA_TO_VPN (MM_HIGHEST_VAD_ADDRESS))
                        &&
                (SizeOfRange <=
                    ((ULONG_PTR)MM_HIGHEST_VAD_ADDRESS -
                         (ULONG_PTR)MI_ROUND_TO_SIZE(
                         (ULONG_PTR)MI_VPN_TO_VA(Node->EndingVpn), Alignment)))) {

                *PreviousVad = Node;
                *Base = (PVOID) MI_ROUND_TO_SIZE(
                            (ULONG_PTR)MI_VPN_TO_VA_ENDING(Node->EndingVpn),
                                Alignment);
                return STATUS_SUCCESS;
            }
            return STATUS_NO_MEMORY;
        }
        Node = NextNode;

    } while (TRUE);
}

NTSTATUS
MiFindEmptyAddressRangeDownTree (
    IN SIZE_T SizeOfRange,
    IN PVOID HighestAddressToEndAt,
    IN ULONG_PTR Alignment,
    IN PMM_AVL_TABLE Table,
    OUT PVOID *Base
    )

/*++

Routine Description:

    The function examines the virtual address descriptors to locate
    an unused range of the specified size and returns the starting
    address of the range.  The function examines from the high
    addresses down and ensures that starting address is less than
    the specified address.

    Note this cannot be used for the based section tree because only
    the nodes in that tree are stored as VAs instead of VPNs.

Arguments:

    SizeOfRange - Supplies the size in bytes of the range to locate.

    HighestAddressToEndAt - Supplies the virtual address that limits
                            the value of the ending address.  The ending
                            address of the located range must be less
                            than this address.

    Alignment - Supplies the alignment for the address.  Must be
                 a power of 2 and greater than the page_size.

    Table - Supplies the root of the tree to search through.

    Base - Receives the starting address of a suitable range on success.

Return Value:

    NTSTATUS.

--*/

{
    PMMADDRESS_NODE Node;
    PMMADDRESS_NODE PreviousNode;
    ULONG_PTR AlignedEndingVa;
    PVOID OptimalStart;
    ULONG_PTR OptimalStartVpn;
    ULONG_PTR HighestVpn;
    ULONG_PTR AlignmentVpn;

    //
    // Note this cannot be used for the based section tree because only
    // the nodes in that tree are stored as VAs instead of VPNs.
    //

    ASSERT (Table != &MmSectionBasedRoot);

    SizeOfRange = MI_ROUND_TO_SIZE (SizeOfRange, PAGE_SIZE);

    if (((ULONG_PTR)HighestAddressToEndAt + 1) < SizeOfRange) {
        return STATUS_NO_MEMORY;
    }

    ASSERT (HighestAddressToEndAt != NULL);
    ASSERT (HighestAddressToEndAt <= (PVOID)((ULONG_PTR)MM_HIGHEST_VAD_ADDRESS + 1));

    HighestVpn = MI_VA_TO_VPN (HighestAddressToEndAt);

    //
    // Locate the Node with the highest starting address.
    //

    OptimalStart = (PVOID)(MI_ALIGN_TO_SIZE(
                           (((ULONG_PTR)HighestAddressToEndAt + 1) - SizeOfRange),
                           Alignment));

    if (Table->NumberGenericTableElements == 0) {

        //
        // The tree is empty, any range is okay.
        //

        *Base = OptimalStart;
        return STATUS_SUCCESS;
    }

    Node = (PMMADDRESS_NODE) Table->BalancedRoot.RightChild;

    //
    // See if an empty slot exists to hold this range, locate the largest
    // element in the tree.
    //

    while (Node->RightChild != NULL) {
        Node = Node->RightChild;
    }

    //
    // Check to see if a range exists between the highest address VAD
    // and the highest address to end at.
    //

    AlignedEndingVa = (ULONG_PTR)MI_ROUND_TO_SIZE ((ULONG_PTR)MI_VPN_TO_VA_ENDING (Node->EndingVpn),
                                               Alignment);

    if (AlignedEndingVa < (ULONG_PTR)HighestAddressToEndAt) {

        if ( SizeOfRange < ((ULONG_PTR)HighestAddressToEndAt - AlignedEndingVa)) {

            *Base = MI_ALIGN_TO_SIZE(
                                  ((ULONG_PTR)HighestAddressToEndAt - SizeOfRange),
                                  Alignment);
            return STATUS_SUCCESS;
        }
    }

    //
    // Walk the tree backwards looking for a fit.
    //

    OptimalStartVpn = MI_VA_TO_VPN (OptimalStart);
    AlignmentVpn = MI_VA_TO_VPN (Alignment);

    do {

        PreviousNode = MiGetPreviousNode (Node);

        if (PreviousNode != NULL) {

            //
            // Is the ending Va below the top of the address to end at.
            //

            if (PreviousNode->EndingVpn < OptimalStartVpn) {
                if ((SizeOfRange >> PAGE_SHIFT) <=
                    ((ULONG_PTR)Node->StartingVpn -
                    (ULONG_PTR)MI_ROUND_TO_SIZE(1 + PreviousNode->EndingVpn,
                                            AlignmentVpn))) {

                    //
                    // See if the optimal start will fit between these
                    // two VADs.
                    //

                    if ((OptimalStartVpn > PreviousNode->EndingVpn) &&
                        (HighestVpn < Node->StartingVpn)) {
                        *Base = OptimalStart;
                        return STATUS_SUCCESS;
                    }

                    //
                    // Check to ensure that the ending address aligned upwards
                    // is not greater than the starting address.
                    //

                    if ((ULONG_PTR)Node->StartingVpn >
                            (ULONG_PTR)MI_ROUND_TO_SIZE(1 + PreviousNode->EndingVpn,
                                                    AlignmentVpn)) {

                        *Base = MI_ALIGN_TO_SIZE(
                                            (ULONG_PTR)MI_VPN_TO_VA (Node->StartingVpn) - SizeOfRange,
                                            Alignment);
                        return STATUS_SUCCESS;
                    }
                }
            }
        } else {

            //
            // No more descriptors, check to see if this fits into the remainder
            // of the address space.
            //

            if (Node->StartingVpn > MI_VA_TO_VPN (MM_LOWEST_USER_ADDRESS)) {
                if ((SizeOfRange >> PAGE_SHIFT) <=
                    ((ULONG_PTR)Node->StartingVpn - MI_VA_TO_VPN (MM_LOWEST_USER_ADDRESS))) {

                    //
                    // See if the optimal start will fit between these
                    // two VADs.
                    //

                    if (HighestVpn < Node->StartingVpn) {
                        *Base = OptimalStart;
                        return STATUS_SUCCESS;
                    }

                    *Base = MI_ALIGN_TO_SIZE(
                                  (ULONG_PTR)MI_VPN_TO_VA (Node->StartingVpn) - SizeOfRange,
                                  Alignment);
                    return STATUS_SUCCESS;
                }
            }
            return STATUS_NO_MEMORY;
        }
        Node = PreviousNode;

    } while (TRUE);
}


NTSTATUS
MiFindEmptyAddressRangeDownBasedTree (
    IN SIZE_T SizeOfRange,
    IN PVOID HighestAddressToEndAt,
    IN ULONG_PTR Alignment,
    IN PMM_AVL_TABLE Table,
    OUT PVOID *Base
    )

/*++

Routine Description:

    The function examines the virtual address descriptors to locate
    an unused range of the specified size and returns the starting
    address of the range.  The function examines from the high
    addresses down and ensures that starting address is less than
    the specified address.

    Note this is only used for the based section tree because only
    the nodes in that tree are stored as VAs instead of VPNs.

Arguments:

    SizeOfRange - Supplies the size in bytes of the range to locate.

    HighestAddressToEndAt - Supplies the virtual address that limits
                            the value of the ending address.  The ending
                            address of the located range must be less
                            than this address.

    Alignment - Supplies the alignment for the address.  Must be
                 a power of 2 and greater than the page_size.

    Table - Supplies the root of the tree to search through.

    Base - Receives the starting address of a suitable range on success.

Return Value:

    NTSTATUS.

--*/

{
    PMMADDRESS_NODE Node;
    PMMADDRESS_NODE PreviousNode;
    ULONG_PTR AlignedEndingVa;
    ULONG_PTR OptimalStart;

    //
    // Note this is only used for the based section tree because only
    // the nodes in that tree are stored as VAs instead of VPNs.
    //

    ASSERT (Table == &MmSectionBasedRoot);

    SizeOfRange = MI_ROUND_TO_SIZE (SizeOfRange, PAGE_SIZE);

    if (((ULONG_PTR)HighestAddressToEndAt + 1) < SizeOfRange) {
        return STATUS_NO_MEMORY;
    }

    ASSERT (HighestAddressToEndAt != NULL);
    ASSERT (HighestAddressToEndAt <= (PVOID)((ULONG_PTR)MM_HIGHEST_VAD_ADDRESS + 1));

    //
    // Locate the node with the highest starting address.
    //

    OptimalStart = (ULONG_PTR) MI_ALIGN_TO_SIZE (
                           (((ULONG_PTR)HighestAddressToEndAt + 1) - SizeOfRange),
                           Alignment);

    if (Table->NumberGenericTableElements == 0) {

        //
        // The tree is empty, any range is okay.
        //

        *Base = (PVOID) OptimalStart;
        return STATUS_SUCCESS;
    }

    Node = (PMMADDRESS_NODE) Table->BalancedRoot.RightChild;

    //
    // See if an empty slot exists to hold this range, locate the largest
    // element in the tree.
    //

    while (Node->RightChild != NULL) {
        Node = Node->RightChild;
    }

    //
    // Check to see if a range exists between the highest address VAD
    // and the highest address to end at.
    //

    AlignedEndingVa = MI_ROUND_TO_SIZE (Node->EndingVpn, Alignment);

    PRINT("search down0: %p %p %p\n", AlignedEndingVa, HighestAddressToEndAt, SizeOfRange);

    if ((AlignedEndingVa < (ULONG_PTR)HighestAddressToEndAt) &&
        (SizeOfRange < ((ULONG_PTR)HighestAddressToEndAt - AlignedEndingVa))) {

        *Base = MI_ALIGN_TO_SIZE(
                              ((ULONG_PTR)HighestAddressToEndAt - SizeOfRange),
                              Alignment);
        return STATUS_SUCCESS;
    }

    //
    // Walk the tree backwards looking for a fit.
    //

    do {

        PreviousNode = MiGetPreviousNode (Node);

        PRINT("search down1: %p %p %p %p\n", PreviousNode, Node, OptimalStart, Alignment);

        if (PreviousNode == NULL) {
            break;
        }

        //
        // Is the ending Va below the top of the address to end at.
        //

        if (PreviousNode->EndingVpn < OptimalStart) {

            if (SizeOfRange <= (Node->StartingVpn -
                    MI_ROUND_TO_SIZE(1 + PreviousNode->EndingVpn, Alignment))) {

                //
                // See if the optimal start will fit between these two VADs.
                //

                if ((OptimalStart > PreviousNode->EndingVpn) &&
                    ((ULONG_PTR) HighestAddressToEndAt < Node->StartingVpn)) {
                    *Base = (PVOID) OptimalStart;
                    return STATUS_SUCCESS;
                }

                //
                // Check to ensure that the ending address aligned upwards
                // is not greater than the starting address.
                //

                if (Node->StartingVpn >
                    MI_ROUND_TO_SIZE(1 + PreviousNode->EndingVpn, Alignment)) {

                    *Base = MI_ALIGN_TO_SIZE (Node->StartingVpn - SizeOfRange,
                                              Alignment);

                    return STATUS_SUCCESS;
                }
            }
        }

        Node = PreviousNode;

    } while (TRUE);


    //
    // No more descriptors, check to see if this fits into the remainder
    // of the address space.
    //

    if (Node->StartingVpn > (ULONG_PTR) MM_LOWEST_USER_ADDRESS) {

        if (SizeOfRange <= (Node->StartingVpn - (ULONG_PTR) MM_LOWEST_USER_ADDRESS)) {

            //
            // See if the optimal start will fit between these two VADs.
            //

            if ((ULONG_PTR) HighestAddressToEndAt < Node->StartingVpn) {
                *Base = (PVOID) OptimalStart;
                return STATUS_SUCCESS;
            }

            *Base = MI_ALIGN_TO_SIZE (Node->StartingVpn - SizeOfRange,
                                      Alignment);

            return STATUS_SUCCESS;
        }
    }
    return STATUS_NO_MEMORY;
}

#if !defined (_USERMODE)

PMMVAD
FASTCALL
MiLocateAddress (
    IN PVOID VirtualAddress
    )

/*++

Routine Description:

    The function locates the virtual address descriptor which describes
    a given address.

Arguments:

    VirtualAddress - Supplies the virtual address to locate a descriptor for.

    Table - Supplies the table describing the tree.

Return Value:

    Returns a pointer to the virtual address descriptor which contains
    the supplied virtual address or NULL if none was located.

--*/

{
    PMMVAD FoundVad;
    ULONG_PTR Vpn;
    PMM_AVL_TABLE Table;
    TABLE_SEARCH_RESULT SearchResult;

    Table = &PsGetCurrentProcess ()->VadRoot;

    //
    // Note the NodeHint *MUST* be captured locally - see the synchronization
    // comment below for details.
    //

    FoundVad = (PMMVAD) Table->NodeHint;

    if (FoundVad == NULL) {
        return NULL;
    }

    Vpn = MI_VA_TO_VPN (VirtualAddress);

    if ((Vpn >= FoundVad->StartingVpn) && (Vpn <= FoundVad->EndingVpn)) {
        return FoundVad;
    }

    //
    // Lookup the element and save the result.
    //

    SearchResult = MiFindNodeOrParent (Table,
                                       Vpn,
                                       (PMMADDRESS_NODE *) &FoundVad);

    if (SearchResult != TableFoundNode) {
        return NULL;
    }

    ASSERT (FoundVad != NULL);

    ASSERT ((Vpn >= FoundVad->StartingVpn) && (Vpn <= FoundVad->EndingVpn));

    //
    // Note the NodeHint field update is not synchronized in all cases, ie:
    // some callers hold the address space mutex and others hold the working
    // set pushlock.  It is ok that the update is not synchronized - as long
    // as care is taken above that it is read into a local variable and then
    // referenced.  Because no VAD can be removed from the tree without holding
    // both the address space & working set.
    //

    Table->NodeHint = (PVOID) FoundVad;

    //
    // Return the VAD.
    //

    return FoundVad;
}
#endif

#if DBG
VOID
MiNodeTreeWalk (
    IN PMM_AVL_TABLE Table
    )
{
    PVOID RestartKey;
    PMMADDRESS_NODE NewNode;
    PMMADDRESS_NODE PrevNode;
    PMMADDRESS_NODE NextNode;

    RestartKey = NULL;

    do {

        NewNode = MiEnumerateGenericTableWithoutSplayingAvl (Table,
                                                             &RestartKey);

        if (NewNode == NULL) {
            break;
        }

        PrevNode = MiGetPreviousNode (NewNode);
        NextNode = MiGetNextNode (NewNode);

        PRINT ("Node %p %x %x\n",
                        NewNode,
                        NewNode->StartingVpn,
                        NewNode->EndingVpn);

        if (PrevNode != NULL) {
            PRINT ("\tPrevNode %p %x %x\n",
                        PrevNode,
                        PrevNode->StartingVpn,
                        PrevNode->EndingVpn);
        }

        if (NextNode != NULL) {
            PRINT ("\tNextNode %p %x %x\n",
                        NextNode,
                        NextNode->StartingVpn,
                        NextNode->EndingVpn);
        }

    } while (TRUE);

    PRINT ("NumberGenericTableElements = 0x%x, Depth = 0x%x\n",
        Table->NumberGenericTableElements,
        Table->DepthOfTree);

    return;
}
#endif

#if defined (_USERMODE)

MMADDRESS_NODE MiBalancedLinks;

MM_AVL_TABLE MiAvlTable;
MM_AVL_TABLE MmSectionBasedRoot;

ULONG DeleteRandom = 1;

#if RANDOM
#define NUMBER_OF_VADS 32
#else
#define NUMBER_OF_VADS 4
#endif

int __cdecl
main(
int	argc,
PCHAR	argv[]
)
{
    ULONG i;
    PVOID StartingAddress;
    PVOID EndingAddress;
    NTSTATUS Status;
    PMMADDRESS_NODE NewNode;
#if RANDOM
    PMMADDRESS_NODE PrevNode;
    ULONG RandomNumber = 0x99887766;
    ULONG_PTR DeleteVpn = 0;
#endif
    PMM_AVL_TABLE Table;
    SIZE_T CapturedRegionSize;

    UNREFERENCED_PARAMETER (argc);
    UNREFERENCED_PARAMETER (argv);

#if RANDOM
    Table = &MiAvlTable;
#else
    Table = &MmSectionBasedRoot;
#endif

    MiInitializeVadTableAvl (Table);

    for (i = 0; i < NUMBER_OF_VADS; i += 1) {
        NewNode = malloc (sizeof (MMADDRESS_NODE));
        ASSERT (((ULONG_PTR)NewNode & 0x3) == 0);

        if (NewNode == NULL) {
            PRINT ("Malloc failed\n");
            exit (1);
        }

        NewNode->u1.Parent = NULL;
        NewNode->LeftChild = NULL;
        NewNode->RightChild = NULL;
        NewNode->u1.Balance = 0;

#if RANDOM
        RandomNumber = RtlRandom (&RandomNumber);

        CapturedRegionSize = (SIZE_T) (RandomNumber & 0x1FFFFF);

        Status = MiFindEmptyAddressRangeInTree (CapturedRegionSize,
                                                64 * 1024,      // align
                                                Table,
                                                &PrevNode,
                                                &StartingAddress);

#else
        CapturedRegionSize = 0x800000;

        Status = MiFindEmptyAddressRangeDownBasedTree (CapturedRegionSize,
                                                (PVOID) 0x7f7effff,     // highest addr
                                                64 * 1024,      // align
                                                Table,
                                                &StartingAddress);
#endif

        if (!NT_SUCCESS (Status)) {
            PRINT ("Could not find empty addr range in tree for size %p\n", CapturedRegionSize);
            free (NewNode);
            continue;
        }

#if RANDOM
        EndingAddress = (PVOID)(((ULONG_PTR)StartingAddress +
                              CapturedRegionSize - 1L) | (PAGE_SIZE - 1L));
#else
        EndingAddress = (PVOID)(((ULONG_PTR)StartingAddress +
                              CapturedRegionSize - 1L));
#endif

        printf ("Inserting addr range in tree @ %p %p\n", StartingAddress, EndingAddress);

#if RANDOM
        NewNode->StartingVpn = MI_VA_TO_VPN (StartingAddress);
        NewNode->EndingVpn = MI_VA_TO_VPN (EndingAddress);
#else
        NewNode->StartingVpn = (ULONG_PTR) StartingAddress;
        NewNode->EndingVpn = (ULONG_PTR) EndingAddress;
#endif

        MiInsertNode (NewNode, Table);

#if RANDOM
        RandomNumber = RtlRandom (&RandomNumber);

        if (RandomNumber & 0x3) {
            DeleteVpn = NewNode->StartingVpn;
        }

        if (DeleteRandom && ((i & 0x3) == 0)) {
            NewNode = MiLocateAddressInTree (DeleteVpn, Table);
            printf ("Located node for random deletion - vpn %p @ %p\n", DeleteVpn, NewNode);

            if (NewNode != NULL) {
                MiRemoveNode (NewNode, Table);
                printf ("Removed random node for vpn %p @ %p %p %p\n",
                   DeleteVpn, NewNode, NewNode->StartingVpn, NewNode->EndingVpn);
            }
        }
#endif
        printf ("\n");
    }

    MiNodeTreeWalk (Table);

    NewNode = MiLocateAddressInTree (5, Table);
    printf ("Located node for vpn 5 @ %p\n", NewNode);

    if (NewNode != NULL) {
        MiRemoveNode (NewNode, Table);
        printf ("Removed node for vpn 5 @ %p\n", NewNode);
    }

    NewNode = MiLocateAddressInTree (5, Table);
    printf("Located node for vpn 5 @ %p\n", NewNode);

    printf("all done, balmin=%x, balmax=%x\n", BalMin, BalMax);

    return 0;
}

#endif

