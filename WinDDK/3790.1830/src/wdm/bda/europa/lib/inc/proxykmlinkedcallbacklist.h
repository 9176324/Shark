//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2000
//  All rights are reserved. Reproduction in whole or in part is prohibited
//  without the written consent of the copyright owner.
//
//  Philips reserves the right to make changes without notice at any time.
//  Philips makes no warranty, expressed, implied or statutory, including but
//  not limited to any implied warranty of merchantability or fitness for any
//  particular purpose, or that the use will not infringe any third party
//  patent, copyright or trademark. Philips must not be liable for any loss
//  or damage arising from its use.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// @doc      INTERNAL
// @module   KMLinkedCallbackList | Provides a linked list to the user mode
//           where all the Callbackfunctions are stored.

// Filename: KMLinkedCallbackList.h
//
// Routines/Functions:
//
//  public:
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////

#pragma once    // Specifies that the file, in which the pragma resides, will
                // be included (opened) only once by the compiler in a
                // build.

//HAL header
#include "VampDeviceResources.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL
// @class  This abstract class is the body for all list classes.

//
//////////////////////////////////////////////////////////////////////////////

class CAbstractNode
{
public:
    CAbstractNode(){}
    virtual ~CAbstractNode(){}
    virtual CAbstractNode* InsertCallback(PVOID pParam1,
                                          PVOID pParam2,
                                          eEventType tEvent) = NULL;
    virtual VAMPRET GetNextCallback(PVOID* ppParam1,
                                 PVOID* ppParam2,
                                 eEventType* ptEvent) = NULL;
    virtual CAbstractNode* GetNextNode() = NULL;
};

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL
// @class  This is the internal node that contains the callback (and all
//         dependend informations).

//
//////////////////////////////////////////////////////////////////////////////

class CInternalNode : public CAbstractNode
{
public:
    CInternalNode(CAbstractNode* pNextNode,
                  PVOID pParam1,
                  PVOID pParam2,
                  eEventType tEvent);

    ~CInternalNode();

    CAbstractNode* InsertCallback(PVOID pParam1,
                                  PVOID pParam2,
                                  eEventType tEvent);

    VAMPRET GetNextCallback(PVOID* ppParam1,
                         PVOID* ppParam2,
                         eEventType* ptEvent);

    CAbstractNode*  GetNextNode();
private:
    CAbstractNode* m_pNextNode;
    PVOID m_pParam1;
    PVOID m_pParam2;
    eEventType m_tEvent;
};

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL
// @class  This is the tail node that marks the end of the list. Every new
//         node is inserted before this node.

//
//////////////////////////////////////////////////////////////////////////////

class CTailNode : public CAbstractNode
{
public:
    CAbstractNode* InsertCallback(
        PVOID pParam1,
        PVOID pParam2,
        eEventType tEvent);
    VAMPRET GetNextCallback(
        PVOID* ppParam1,
        PVOID* ppParam2,
        eEventType* ptEvent);
    CAbstractNode*  GetNextNode();
};

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL
// @class  This is the class that builds the list interface to the kernel mode
//         proxy.

//
//////////////////////////////////////////////////////////////////////////////

class CLinkedCallbackList : public CAbstractNode
{
public:
    CLinkedCallbackList();
    ~CLinkedCallbackList();
    CAbstractNode* InsertCallback(
        PVOID pParam1,
        PVOID pParam2,
        eEventType tEvent);
    VAMPRET GetNextCallback(
        PVOID* ppParam1,
        PVOID* ppParam2,
        eEventType* ptEvent);
    CAbstractNode* GetNextNode();
	DWORD GetObjectStatus();
private:
    CAbstractNode* m_pNextNode;
	DWORD m_dwObjectStatus;
};
