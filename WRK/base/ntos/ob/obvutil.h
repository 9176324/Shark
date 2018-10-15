/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    obvutil.h

Abstract:

    This header exposes various utilities required to do driver verification.

--*/

LONG_PTR
ObvUtilStartObRefMonitoring(
    IN PDEVICE_OBJECT DeviceObject
    );

LONG_PTR
ObvUtilStopObRefMonitoring(
    IN PDEVICE_OBJECT DeviceObject,
    IN LONG StartSkew
    );

