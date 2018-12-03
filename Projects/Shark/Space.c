/*
*
* Copyright (c) 2018 by blindtiger. All rights reserved.
*
* The contents of this file are subject to the Mozilla Public License Version
* 2.0 (the "License")); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. SEe the License
* for the specific language governing rights and limitations under the
* License.
*
* The Initial Developer of the Original e is blindtiger.
*
*/

#include <defs.h>

#include "Space.h"

#include "Jump.h"
#include "Reload.h"
#include "Rtx.h"
#include "Scan.h"

PPFN PfnDatabase;

PPFNLIST * FreePagesByColor;
PPFNSLIST_HEADER FreePageSlist[2];
ULONG MaximumColor;

PFNSLIST_HEADER FreeSlist;

#define MM_PTE_NO_EXECUTE ((ULONG64)1 << 63)

ULONG64 ProtectToPteMask[32] = {
    MM_PTE_NOACCESS | MM_PTE_NO_EXECUTE,
    MM_PTE_READONLY | MM_PTE_CACHE | MM_PTE_NO_EXECUTE,
    MM_PTE_EXECUTE | MM_PTE_CACHE,
    MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
    MM_PTE_READWRITE | MM_PTE_CACHE | MM_PTE_NO_EXECUTE,
    MM_PTE_WRITECOPY | MM_PTE_CACHE | MM_PTE_NO_EXECUTE,
    MM_PTE_EXECUTE_READWRITE | MM_PTE_CACHE,
    MM_PTE_EXECUTE_WRITECOPY | MM_PTE_CACHE,
    MM_PTE_NOACCESS | MM_PTE_NO_EXECUTE,
    MM_PTE_NOCACHE | MM_PTE_READONLY | MM_PTE_NO_EXECUTE,
    MM_PTE_NOCACHE | MM_PTE_EXECUTE,
    MM_PTE_NOCACHE | MM_PTE_EXECUTE_READ,
    MM_PTE_NOCACHE | MM_PTE_READWRITE | MM_PTE_NO_EXECUTE,
    MM_PTE_NOCACHE | MM_PTE_WRITECOPY | MM_PTE_NO_EXECUTE,
    MM_PTE_NOCACHE | MM_PTE_EXECUTE_READWRITE,
    MM_PTE_NOCACHE | MM_PTE_EXECUTE_WRITECOPY,
    MM_PTE_NOACCESS | MM_PTE_NO_EXECUTE,
    MM_PTE_GUARD | MM_PTE_READONLY | MM_PTE_CACHE | MM_PTE_NO_EXECUTE,
    MM_PTE_GUARD | MM_PTE_EXECUTE | MM_PTE_CACHE,
    MM_PTE_GUARD | MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
    MM_PTE_GUARD | MM_PTE_READWRITE | MM_PTE_CACHE | MM_PTE_NO_EXECUTE,
    MM_PTE_GUARD | MM_PTE_WRITECOPY | MM_PTE_CACHE | MM_PTE_NO_EXECUTE,
    MM_PTE_GUARD | MM_PTE_EXECUTE_READWRITE | MM_PTE_CACHE,
    MM_PTE_GUARD | MM_PTE_EXECUTE_WRITECOPY | MM_PTE_CACHE,
    MM_PTE_NOACCESS | MM_PTE_NO_EXECUTE,
    MM_PTE_WRITECOMBINE | MM_PTE_READONLY | MM_PTE_NO_EXECUTE,
    MM_PTE_WRITECOMBINE | MM_PTE_EXECUTE,
    MM_PTE_WRITECOMBINE | MM_PTE_EXECUTE_READ,
    MM_PTE_WRITECOMBINE | MM_PTE_READWRITE | MM_PTE_NO_EXECUTE,
    MM_PTE_WRITECOMBINE | MM_PTE_WRITECOPY | MM_PTE_NO_EXECUTE,
    MM_PTE_WRITECOMBINE | MM_PTE_EXECUTE_READWRITE,
    MM_PTE_WRITECOMBINE | MM_PTE_EXECUTE_WRITECOPY
};

VOID
NTAPI
FlushSingleTb(
    __in PVOID VirtualAddress
)
{
    IpiSingleCall(
        (PPS_APC_ROUTINE)NULL,
        (PKSYSTEM_ROUTINE)NULL,
        (PUSER_THREAD_START_ROUTINE)_FlushSingleTb,
        (PVOID)VirtualAddress);
}

VOID
NTAPI
FlushMultipleTb(
    __in PVOID VirtualAddress,
    __in SIZE_T RegionSize,
    __in BOOLEAN AllProcesors
)
{
    if (FALSE != AllProcesors) {
        IpiGenericCall(
            (PPS_APC_ROUTINE)NULL,
            (PKSYSTEM_ROUTINE)_FlushMultipleTb,
            (PUSER_THREAD_START_ROUTINE)VirtualAddress,
            (PVOID)RegionSize);
    }
    else {
        IpiSingleCall(
            (PPS_APC_ROUTINE)NULL,
            (PKSYSTEM_ROUTINE)_FlushMultipleTb,
            (PUSER_THREAD_START_ROUTINE)VirtualAddress,
            (PVOID)RegionSize);
    }
}

PVAD_NODE
NTAPI
GetVadRootProcess(
    __in PEPROCESS Process
)
{
    PVAD_NODE VadRoot = NULL;
    PCHAR ControlPc = NULL;
    ULONG Length = 0;
    UNICODE_STRING RoutineString = { 0 };
    ULONG BuildNumber = 0;

    static LONG Offset;

    PsGetVersion(NULL, NULL, &BuildNumber, NULL);

    if (0 == Offset) {
        RtlInitUnicodeString(&RoutineString, L"PsGetProcessExitStatus");

        ControlPc = MmGetSystemRoutineAddress(&RoutineString);

        if (NULL != ControlPc) {
            while (0 != _CmpByte(ControlPc[0], 0xc3) ||
                0 != _CmpByte(ControlPc[1], 0xc2)) {
                Length = DetourGetInstructionLength(ControlPc);

                if (6 == Length) {
                    if (0 == _CmpByte(ControlPc[0], 0x8b)) {
                        Offset = *(PLONG)(ControlPc + 2) + 4;

                        if (BuildNumber < 9600) {
                            VadRoot = ((PMM_AVL_NODE)((ULONG_PTR)Process + Offset))->RightChild;
                        }
                        else {
                            VadRoot = *(PVAD_NODE *)((ULONG_PTR)Process + Offset);
                        }

                        break;
                    }
                }

                ControlPc += Length;
            }
        }
    }
    else {
        if (BuildNumber < 9600) {
            VadRoot = ((PMM_AVL_NODE)((ULONG_PTR)Process + Offset))->RightChild;
        }
        else {
            VadRoot = *(PVAD_NODE *)((ULONG_PTR)Process + Offset);
        }
    }

    return VadRoot;
}

PVAD_NODE
FASTCALL
GetNextNode(
    __in PVAD_NODE Node
)
{
    PVAD_NODE Next = NULL;
    PVAD_NODE Parent = NULL;
    PVAD_NODE Left = NULL;
    ULONG BuildNumber = 0;

    PsGetVersion(NULL, NULL, &BuildNumber, NULL);

    Next = Node;

    if (BuildNumber < 9600) {
        if (NULL == Next->AvlRoot.RightChild) {
            do {
                Parent = (PVAD_NODE)((ULONG_PTR)Next->AvlRoot.Parent & ~3);

                if (NULL == Parent || Parent == Next) {
                    return NULL;
                }

                if (Parent->AvlRoot.LeftChild == Next) {
                    return Parent;
                }

                Next = Parent;
            } while (TRUE);
        }

        Next = Next->AvlRoot.RightChild;

        do {
            Left = Next->AvlRoot.LeftChild;

            if (NULL == Left) {
                break;
            }

            Next = Left;
        } while (TRUE);
    }
    else {
        if (NULL == Next->BalancedRoot.RightChild) {
            do {
                Parent = (PVAD_NODE)((ULONG_PTR)Next->BalancedRoot.Parent & ~3);

                if (NULL == Parent || Parent == Next) {
                    return NULL;
                }

                if (Parent->BalancedRoot.LeftChild == Next) {
                    return Parent;
                }

                Next = Parent;
            } while (TRUE);
        }

        Next = Next->BalancedRoot.RightChild;

        do {
            Left = Next->BalancedRoot.LeftChild;

            if (NULL == Left) {
                break;
            }

            Next = Left;
        } while (TRUE);
    }

    return Next;
}

PVAD_NODE
NTAPI
FindVadNode(
    __in ULONG_PTR Address
)
{
    PVAD_NODE VadRoot = NULL;
    PVAD_NODE Node = NULL;
    PVAD_NODE ResultVad = NULL;
    ULONG BuildNumber = 0;
    VAD_LOCAL_NODE VadLocalNode = { 0 };

    PsGetVersion(NULL, NULL, &BuildNumber, NULL);

    VadRoot = GetVadRootProcess(IoGetCurrentProcess());

    Node = VadRoot;

    if (BuildNumber < 9600) {
        while (Node->AvlRoot.LeftChild != NULL) {
            Node = Node->AvlRoot.LeftChild;
        }
    }
    else {
        while (Node->BalancedRoot.LeftChild != NULL) {
            Node = Node->BalancedRoot.LeftChild;
        }
    }

    do {
        LocalVpn(Node, &VadLocalNode);

        if (Address >= VadLocalNode.BeginAddress &&
            Address < VadLocalNode.EndAddress) {
            ResultVad = Node;

            break;
        }

        Node = GetNextNode(Node);
    } while (NULL != Node);

    return ResultVad;
}
