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


#pragma once

#include "34AVStrm.h"
#include "baseclass.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  Digital Tuner out pin of the Digital Tuner Filter.
//
//////////////////////////////////////////////////////////////////////////////
class CDgtlTunerOut: public CBaseClass
{
public:
    CDgtlTunerOut();

    ~CDgtlTunerOut();

    //dispatch routines

    NTSTATUS Create
    (
        IN PKSPIN pKSPin,
        IN PIRP pIrp
    );

    NTSTATUS Remove
    (
        IN PKSPIN pKSPin,
        IN PIRP pIrp
    );
    NTSTATUS SetDeviceState
    (
        IN PKSPIN pKSPin,           // Pointer to KSPIN.
        IN KSSTATE ToState,         // State that has to be active after this call.
        IN KSSTATE FromState        // State that was active before this call.
    );

private:
    UCHAR m_bFirstTimeAcquire;
};
