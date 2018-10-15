/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    luminous.h

Abstract:

    This header exposes definitions for managing firefly devices.

Environment:

    User Mode.

Revision History:

    Eliyas Yakub  - Created Oct 18th, 2002

--*/
#ifndef _LUMINOUS_H_
#define _LUMINOUS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>
#include <wchar.h>
#include <initguid.h>
#include <wbemcli.h>

#define NAME_SPACE TEXT("root\\WMI")
#define CLASS_NAME  TEXT("FireflyDeviceInformation")
#define PROPERTY_NAME TEXT("TailLit")

class CLuminous {

public:

    CLuminous(VOID);

    virtual ~CLuminous(VOID);

    BOOL Open(VOID);

    VOID Close(VOID);

    BOOL Set(IN BOOL Enabled);

    BOOL Get(IN BOOL *Enabled);

private:

    IWbemServices *m_pIWbemServices;
    IWbemClassObject     *m_pIWbemClassObject;
    BOOL m_bCOMInitialized;
    
};


IWbemServices *ConnectToNamespace (
                                LPTSTR chNamespace);
IWbemClassObject *GetInstanceReference (
                                    IWbemServices *pIWbemServices,
                                    LPTSTR lpClassName);

BSTR AnsiToBstr (
    LPTSTR lpSrc, 
    int nLenSrc);

#endif // _LUMINOUS_H_

