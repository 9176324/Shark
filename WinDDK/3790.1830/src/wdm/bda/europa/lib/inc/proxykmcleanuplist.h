//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2001
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
// @doc      INTERNAL EXTERNAL
// @module   ModuleName | module description
////
// Filename: ProxyKMCleanupList.h
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

#pragma once

class CProxyKMCleanupObj;

// system-wide storage of cleanup objects. I imagine that the total count of
// objects and frequency of reference will both be low enough that a linear
// search will be sufficient. More efficient lookup schemes are left as an
// exercise to the reader.
class CProxyKMCleanupList
{
public:
	~CProxyKMCleanupList();
	VAMPRET AddCleanupObj(CProxyKMCleanupObj* pCleanupObj);
	VAMPRET RemoveCleanupObj(PVOID pCleanupObj);
	VAMPRET Cleanup(HANDLE hCleanupHandle);
private:
	CVampList<CProxyKMCleanupObj> m_CleanupList;
};
