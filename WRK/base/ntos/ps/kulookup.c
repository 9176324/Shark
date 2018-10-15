/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    kulookup.c

Abstract:

    The module implements the code necessary to lookup user mode entry points
    in the system DLL for exception dispatching and APC delivery.

--*/

#include "psp.h"
#pragma alloc_text(INIT, PspLookupKernelUserEntryPoints)

NTSTATUS
PspLookupKernelUserEntryPoints (
    VOID
    )

/*++

Routine Description:

    The function locates the address of the exception dispatch and user APC
    delivery routine in the system DLL and stores the respective addresses
    in the PCR.

Arguments:

    None.

Return Value:

    NTSTATUS

--*/

{

    NTSTATUS Status;
    PSZ EntryName;

    //
    // Lookup the user mode "trampoline" code for exception dispatching
    //

    EntryName = "KiUserExceptionDispatcher";
    Status = PspLookupSystemDllEntryPoint(EntryName,
                                          (PVOID *)&KeUserExceptionDispatcher);

    if (!NT_SUCCESS(Status)) {
        KdPrint(("Ps: Cannot find user exception dispatcher address\n"));
        return Status;
    }

    //
    // Lookup the user mode "trampoline" code for APC dispatching
    //

    EntryName = "KiUserApcDispatcher";
    Status = PspLookupSystemDllEntryPoint(EntryName,
                                          (PVOID *)&KeUserApcDispatcher);

    if (!NT_SUCCESS(Status)) {
        KdPrint(("Ps: Cannot find user apc dispatcher address\n"));
        return Status;
    }

    //
    // Lookup the user mode "trampoline" code for callback dispatching.
    //

    EntryName = "KiUserCallbackDispatcher";
    Status = PspLookupSystemDllEntryPoint(EntryName,
                                          (PVOID *)&KeUserCallbackDispatcher);

    if (!NT_SUCCESS(Status)) {
        KdPrint(("Ps: Cannot find user callback dispatcher address\n"));
        return Status;
    }

    //
    // Lookup the user mode "trampoline" code for raising a usermode exception.
    //

    EntryName = "KiRaiseUserExceptionDispatcher";
    Status = PspLookupSystemDllEntryPoint(EntryName,
                                          (PVOID *)&KeRaiseUserExceptionDispatcher);

    if (!NT_SUCCESS(Status)) {
        KdPrint(("Ps: Cannot find raise user exception dispatcher address\n"));
        return Status;
    }

    //
    // Lookup the user mode SLIST exception labels.
    //

    EntryName = "ExpInterlockedPopEntrySListEnd";
    Status = PspLookupSystemDllEntryPoint(EntryName,
                                          &KeUserPopEntrySListEnd);

    if (!NT_SUCCESS(Status)) {
        KdPrint(("Ps: Cannot find user slist end address\n"));
        return Status;
    }

    EntryName = "ExpInterlockedPopEntrySListFault";
    Status = PspLookupSystemDllEntryPoint(EntryName,
                                          &KeUserPopEntrySListFault);

    if (!NT_SUCCESS(Status)) {
        KdPrint(("Ps: Cannot find user slist fault address\n"));
        return Status;
    }

    EntryName = "ExpInterlockedPopEntrySListResume";
    Status = PspLookupSystemDllEntryPoint(EntryName,
                                          &KeUserPopEntrySListResume);

    if (!NT_SUCCESS(Status)) {
        KdPrint(("Ps: Cannot find user slist resume address\n"));
        return Status;
    }

#if defined(_X86_)

    if (KeFeatureBits & KF_FAST_SYSCALL) {

        //
        // Get the addresses of the fast system call code.
        //

        EntryName = "KiFastSystemCall";
        Status = PspLookupSystemDllEntryPoint(EntryName,
                                              (PVOID *)&SharedUserData->SystemCall);

        if (!NT_SUCCESS(Status)) {
            KdPrint(("Ps: Cannot find fast system call address\n"));
            return Status;
        }

        //
        // Get the addresses of the fast system call return code.
        //

        EntryName = "KiFastSystemCallRet";
        Status = PspLookupSystemDllEntryPoint(EntryName,
                                              (PVOID *)&SharedUserData->SystemCallReturn);

        if (!NT_SUCCESS(Status)) {
            KdPrint(("Ps: Cannot find fast system call return address\n"));
            return Status;
        }

    } else {

        //
        // Get the addresses of the fast system call code.
        //

        EntryName = "KiIntSystemCall";
        Status = PspLookupSystemDllEntryPoint(EntryName,
                                              (PVOID *)&SharedUserData->SystemCall);

        if (!NT_SUCCESS(Status)) {
            KdPrint(("Ps: Cannot find int2e system call address\n"));
            return Status;
        }
    }

    //
    // Ret instruction to test no execute state of shared user data.
    //

    SharedUserData->TestRetInstruction = 0xc3;

#endif

    return Status;
}

