/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    debug.c

Abstract:

    This module implements functions to support debugging on NT.

--*/

#include "stdarg.h"
#include "stdio.h"
#include "ntrtlp.h"
#define NOEXTAPI
#include "wdbgexts.h"
#include <ntdbg.h>

//
// Forward referenced internal interface.
//

ULONG
vDbgPrintExWithPrefixInternal (
    __in PCH Prefix,
    __in ULONG ComponentId,
    __in ULONG Level,
    __in PCHAR Format,
    __in va_list arglist,
    __in BOOLEAN ControlC
    );

ULONG
DbgPrint (
    __in PCHAR Format,
    ...
    )

/*++

Routine Description:

    This routine provides a "printf" style capability for the kernel
    debugger.

    N.B. Control-C is consumed by the debugger and returned to this routine
        as status. If control-C was pressed, this routine breakpoints.

Arguments:

    Format - Supplies a pointer to a printf style format string.

    ... - Supplies additional arguments consumed according to the format
        string.

Return Value:

    Defined as returning a ULONG, actually returns status.

--*/

{

    va_list arglist;

    va_start(arglist, Format);
    return vDbgPrintExWithPrefixInternal("", -1, 0, Format, arglist, TRUE);
}

ULONG
DbgPrintReturnControlC (
    __in PCHAR Format,
    ...
    )

/*++

Routine Description:

    This routine provides a "printf" style capability for the kernel
    debugger.

    N.B. Control-C is consumed by the debugger and returned to this routine
        as status. If control-C was pressed, then the appropriate status is
        returned to the caller.

Arguments:

    Format - Supplies a pointer to a printf style format string.

    ... - Supplies additional arguments consumed according to the format
        string.

Return Value:

    Defined as returning a ULONG, actually returns status.

--*/

{

    va_list arglist;

    va_start(arglist, Format);
    return vDbgPrintExWithPrefixInternal("", -1, 0, Format, arglist, FALSE);
}

ULONG
DbgPrintEx (
    __in ULONG ComponentId,
    __in ULONG Level,
    __in PCHAR Format,
    ...
    )

/*++

Routine Description:

    This routine provides a "printf" style capability for the kernel
    debugger.

    N.B. Control-C is consumed by the debugger and returned to this routine
        as status. If control-C was pressed, this routine breakpoints.

Arguments:

    ComponentId - Supplies the Id of the calling component.

    Level - Supplies the output filter level.

    Format - Supplies a pointer to a printf style format string.

    ... - Supplies additional arguments consumed according to the format
        string.

Return Value:

    Defined as returning a ULONG, actually returns status.

--*/

{

    va_list arglist;

    va_start(arglist, Format);
    return vDbgPrintExWithPrefixInternal("",
                                         ComponentId,
                                         Level,
                                         Format,
                                         arglist,
                                         TRUE);
}

ULONG
vDbgPrintEx (
    __in ULONG ComponentId,
    __in ULONG Level,
    __in PCHAR Format,
    __in va_list arglist
    )

/*++

Routine Description:

    This routine provides a "printf" style capability for the kernel
    debugger.

    N.B. Control-C is consumed by the debugger and returned to this routine
        as status. If control-C was pressed, this routine breakpoints.

Arguments:

    ComponentId - Supplies the Id of the calling component.

    Level - Supplies the output filter level or mask.

    Format - Supplies a pointer to a printf style format string.

    arglist - Supplies a pointer to a variable argument list.

Return Value:

    Defined as returning a ULONG, actually returns status.

--*/

{

    return vDbgPrintExWithPrefixInternal("",
                                         ComponentId,
                                         Level,
                                         Format,
                                         arglist,
                                         TRUE);
}

ULONG
vDbgPrintExWithPrefix (
    __in PCH Prefix,
    __in ULONG ComponentId,
    __in ULONG Level,
    __in PCHAR Format,
    __in va_list arglist
    )

/*++

Routine Description:

    This routine provides a "printf" style capability for the kernel
    debugger.

    N.B. Control-C is consumed by the debugger and returned to this routine
        as status. If control-C was pressed, this routine breakpoints.

Arguments:

    Prefix - Supplies a pointer to text that is to prefix the formatted
        output.

    ComponentId - Supplies the Id of the calling component.

    Level - Supplies the output filter level or mask.

    Format - Supplies a pointer to a printf style format string.

    arglist - Supplies a pointer to a variable argument list.

Return Value:

    Defined as returning a ULONG, actually returns status.

--*/

{
    return vDbgPrintExWithPrefixInternal(Prefix,
                                         ComponentId,
                                         Level,
                                         Format,
                                         arglist,
                                         TRUE);
}

ULONG
vDbgPrintExWithPrefixInternal (
    __in PCH Prefix,
    __in ULONG ComponentId,
    __in ULONG Level,
    __in PCHAR Format,
    __in va_list arglist,
    __in BOOLEAN ControlC
    )

/*++

Routine Description:

    This routine provides a "printf" style capability for the kernel
    debugger.

Arguments:

    Prefix - Supplies a pointer to text that is to prefix the formatted
        output.

    ComponentId - Supplies the Id of the calling component.

    Level - Supplies the output filter level or mask.

    Format - Supplies a pointer to a "printf" style format string.

    arglist - Supplies a pointer to a variable argument list.

    ControlC - Supplies a boolean that determines whether control C is
        consumed or not.

Return Value:

    Defined as returning a ULONG, actually returns status.

--*/

{

    UCHAR Buffer[512];
    int cb;
    int Length;
    STRING Output;
    NTSTATUS Status;

    //
    // If the debug output will be suppressed, then return success
    // immediately.
    //

    if ((ComponentId != -1) &&
        (NtQueryDebugFilterState(ComponentId, Level) != TRUE)) {

        return STATUS_SUCCESS;
    }

    //
    // Format the output into a buffer and then print it.
    //

    try {
        cb = strlen(Prefix);
        if (cb > sizeof(Buffer)) {
            cb = sizeof(Buffer);
        }

        strncpy(Buffer, Prefix, cb);
        Length = _vsnprintf(Buffer + cb , sizeof(Buffer) - cb, Format, arglist);

    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode();
    }

    //
    // Check if buffer overflow occurred during formatting. If buffer overflow
    // occurred, then terminate the buffer with an end of line.
    //

    if (Length == -1) {
        Buffer[sizeof(Buffer) - 1] = '\n';
        Length = sizeof(Buffer);

    } else {
        Length += cb;
    }

    Output.Buffer = Buffer;
    Output.Length = (USHORT)Length;

    //
    // If APP is being debugged, raise an exception and the debugger
    // will catch and handle this. Otherwise, kernel debugger service
    // is called.
    //

    Status = DebugPrint(&Output, ComponentId, Level);
    if ((ControlC == TRUE) &&
        (Status == STATUS_BREAKPOINT)) {

        DbgBreakPointWithStatus(DBG_STATUS_CONTROL_C);
        Status = STATUS_SUCCESS;
    }

    return Status;
}

ULONG
DbgPrompt (
    __in PCHAR Prompt,
    __out_bcount(Length) PCHAR Response,
    __in ULONG Length
    )

/*++

Routine Description:

    This function displays the prompt string on the debug console and
    then reads a line of text from the debug console. The line read is
    returned in the memory pointed to by the second parameter. The third
    parameter specifies the maximum number of characters that can be
    stored in the response area.

Arguments:

    Prompt - Supplies a pointer to the text to display as the prompt.

    Response - Supplies a pointer to a buffer that receives the response
       read from the debug console.

    Length - Supplies the maximum number of characters that can be stored in
        the response buffer.

Return Value:

    Number of characters stored in the Response buffer including the
    terminating newline character, but not the ending null character.

--*/

{

    STRING Input;
    STRING Output;

    //
    // Output the prompt string and read input.
    //

    Input.MaximumLength = (USHORT)Length;
    Input.Buffer = Response;
    Output.Length = (USHORT)strlen(Prompt);
    Output.Buffer = Prompt;
    return DebugPrompt(&Output, &Input);
}

VOID
DbgLoadImageSymbols (
    __in PSTRING FileName,
    __in PVOID ImageBase,
    __in ULONG_PTR ProcessId
    )

/*++

Routine Description:

    Tells the debugger about newly loaded symbols.

Arguments:

Return Value:

--*/

{

    PIMAGE_NT_HEADERS NtHeaders;
    KD_SYMBOLS_INFO SymbolInfo;

    SymbolInfo.BaseOfDll = ImageBase;
    SymbolInfo.ProcessId = ProcessId;
    NtHeaders = RtlImageNtHeader( ImageBase );
    if (NtHeaders != NULL) {
        SymbolInfo.CheckSum = (ULONG)NtHeaders->OptionalHeader.CheckSum;
        SymbolInfo.SizeOfImage = (ULONG)NtHeaders->OptionalHeader.SizeOfImage;

    } else {

        SymbolInfo.SizeOfImage = 0;
        SymbolInfo.CheckSum    = 0;
    }

    DebugService2(FileName, &SymbolInfo, BREAKPOINT_LOAD_SYMBOLS);

    return;
}

VOID
DbgUnLoadImageSymbols (
    __in PSTRING FileName,
    __in PVOID ImageBase,
    __in ULONG_PTR ProcessId
    )

/*++

Routine Description:

    Tells the debugger about newly unloaded symbols.

Arguments:

Return Value:

--*/

{

    KD_SYMBOLS_INFO SymbolInfo;

    SymbolInfo.BaseOfDll = ImageBase;
    SymbolInfo.ProcessId = ProcessId;
    SymbolInfo.CheckSum    = 0;
    SymbolInfo.SizeOfImage = 0;

    DebugService2(FileName, &SymbolInfo, BREAKPOINT_UNLOAD_SYMBOLS);

    return;
}

VOID
DbgCommandString (
    __in PCH Name,
    __in PCH Command
    )

/*++

Routine Description:

    Tells the debugger to execute a command string

Arguments:

    Name - Identifies the originator of the command.

    Command - Command string.

Return Value:

--*/

{

    STRING NameStr, CommandStr;

    NameStr.Buffer = Name;
    NameStr.Length = (USHORT)strlen(Name);
    CommandStr.Buffer = Command;
    CommandStr.Length = (USHORT)strlen(Command);
    DebugService2(&NameStr, &CommandStr, BREAKPOINT_COMMAND_STRING);
}

NTSTATUS
DbgQueryDebugFilterState (
    __in ULONG ComponentId,
    __in ULONG Level
    )

/*++

Routine Description:

    This function queries the debug print enable for a specified component
    level.  If Level is > 31, it's assumed to be a mask otherwise, it indicates
    a specific debug level to test for (ERROR/WARNING/TRACE/INFO, etc).

Arguments:

    ComponentId - Supplies the component id.

    Level - Supplies the debug filter level number or mask.

Return Value:

    STATUS_INVALID_PARAMETER_1 is returned if the component id is not
        valid.

    TRUE is returned if output is enabled for the specified component
        and level or is enabled for the system.

    FALSE is returned if output is not enabled for the specified component
        and level and is not enabled for the system.

--*/

{

    return NtQueryDebugFilterState(ComponentId, Level);
}

NTSTATUS
DbgSetDebugFilterState (
    __in ULONG ComponentId,
    __in ULONG Level,
    __in BOOLEAN State
    )

/*++

Routine Description:

    This function sets the state of the debug print enable for a specified
    component and level. The debug print enable state for the system is set
    by specifying the distinguished value -1 for the component id.

Arguments:

    ComponentId - Supplies the Id of the calling component.

    Level - Supplies the output filter level or mask.

    State - Supplies a boolean value that determines the new state.

Return Value:

    STATUS_ACCESS_DENIED is returned if the required privilege is not held.

    STATUS_INVALID_PARAMETER_1 is returned if the component id is not
        valid.

    STATUS_SUCCESS  is returned if the debug print enable state is set for
        the specified component.

--*/

{
    return NtSetDebugFilterState(ComponentId, Level, State);
}

