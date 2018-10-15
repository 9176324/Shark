/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    vffilter.h

Abstract:

    This header contains prototypes for using the verifier driver filter.

--*/

VOID
VfFilterInit(
    VOID
    );

VOID
VfFilterAttach(
    IN  PDEVICE_OBJECT  PhysicalDeviceObject,
    IN  VF_DEVOBJ_TYPE  DeviceObjectType
    );

BOOLEAN
VfFilterIsVerifierFilterObject(
    IN  PDEVICE_OBJECT  DeviceObject
    );

