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

//////////////////////////////////////////////////////////////////////////////
//
//  C-Function prototypes (due to MS interface we still need C-Functions)
//
//////////////////////////////////////////////////////////////////////////////


NTSTATUS
BasePinRemove
(
    IN PKSPIN pKSPin,
    IN PIRP pIrp
);

NTSTATUS
BasePinProcess
(
    IN PKSPIN pKSPin
);

NTSTATUS
BasePinSetDeviceState
(
    IN PKSPIN pKSPin,
    IN KSSTATE ToState,
    IN KSSTATE FromState
);
