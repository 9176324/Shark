/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    Sep.c

Abstract:

    This Module implements the private security routine that are defined
    in sep.h

--*/

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SepCheckAcl)
#endif



BOOLEAN
SepCheckAcl (
    IN PACL Acl,
    IN ULONG Length
    )

/*++

Routine Description:

    This is a private routine that checks that an acl is well formed.

Arguments:

    Acl - Supplies the acl to check

    Length - Supplies the real size of the acl.  The internal acl size
        must agree.

Return Value:

    BOOLEAN - TRUE if the acl is well formed and FALSE otherwise

--*/
{
    if ((Length < sizeof(ACL)) || (Length != Acl->AclSize)) {
        return FALSE;
    }
    return RtlValidAcl( Acl );
}

