/*++

Copyright (c) 1991, Microsoft Corporation

Module Name:

    ioctlvdd.c

Abstract:

    Virtual Device Driver for DOSIOCTL sample

Environment:

    NT-MVDM (User Mode VDD)

Notes:

    This VDD presents a private interface for the DOSDRVR component
    of the DOSIOCTL sample. The three functions (OPEN, CLOSE, INFO)
    correspond to calls that the DOSAPP in the sample makes. To show
    how nicely symetrical a VDD can be architected, this VDD simply
    converts each of these calls made by the DOS device driver into
    NT-kernel device driver calls using WIN32 functions.

    Thus, when the DOS application of the sample issues a DOS OPEN,
    the DOS driver gets the request and calls our entry point with
    the VDD's VDDOPEN call. The VDD then calls the Win32 function
    CreateFile() to get a handle to the NT driver in the sample.

    When the DOS application does an IOCTL read, the DOS driver
    calls VDDINFO, which issues a Win32 DeviceIOControl() to the
    NT driver, after getting a flat address for the application's
    buffer.

    Finally, the DOSAPP's CLOSE is translated similarly to a WIN32
    CloseHandle().

    Note that the functions VDDOPEN, VDDCLOSE and VDDINFO are not
    architected in the NT VDD interface, and were invented just for
    this sample. The interface between the DOS driver and the VDD
    could have been defined completely differently. This VDD was
    coded only to show one possibility for designing a VDD interface.


--*/


#include "ioctlvdd.h"
#include "devioctl.h"


BOOL
VDDInitialize(
    HANDLE hVdd,
    DWORD dwReason,
    LPVOID lpReserved
    )

/*++

Routine Description:

    The entry point for the Vdd which handles intialization and termination.

Arguments:

    hVdd   - The handle to the VDD

    Reason - flag word that indicates why Dll Entry Point was invoked

    lpReserved - Unused

Return Value:
    BOOL bRet - if (dwReason == DLL_PROCESS_ATTACH)
                   TRUE    - Dll Intialization successful
                   FALSE   - Dll Intialization failed
                else
                   always returns TRUE
--*/

{
    HANDLE hDriver;

    switch ( dwReason ) {

        case DLL_PROCESS_ATTACH:


            //
            // The following call is here only to test the existence of
            // the KRNLDRVR component. If it isn't there, then there is
            // no reason to load.
            //

            hDriver = CreateFile("\\\\.\\krnldrvr",    /* open KRNLDRVR      */
                            GENERIC_READ | GENERIC_WRITE,  /* open for r/w   */
                            0,                         /* can't share        */
                            (LPSECURITY_ATTRIBUTES) NULL, /* no security     */
                            OPEN_EXISTING,             /* existing file only */
                            FILE_FLAG_OVERLAPPED,      /* overlapped I/O     */
                            (HANDLE) NULL);            /* no attr template   */

            if (hDriver == INVALID_HANDLE_VALUE) {
                MessageBox (NULL, "Unable to locate Kernel driver",
                        "Sample VDD", MB_OK | MB_ICONEXCLAMATION);
                return FALSE;                           /* Abort load       */
                }

            //
            // The call to CreateFile succeeded, close the handle again
            //

            CloseHandle (hDriver);

            break;

        default:

            break;

    }

    return TRUE;

}



VOID
VDDRegisterInit(
    VOID
    )
/*++

Arguments:

Return Value:

    SUCCESS - Client Carry Clear
    FAILURE - Client Carry Set

--*/


{
    // This routine is called when the DOSDRVR issues the RegisterModule
    // call.

    setCF(0);
    return;
}



VOID
VDDDispatch(
    VOID
    )
/*++

Routine Description:

    This subroutine implements the funcionality of the VDD. It handles
    client VDM calls from the DOS driver. The operation is as follows:

    VDDOPEN - Issue Win32 CreateFile() to "krnldrvr"
    VDDINFO - Issue Win32 DeviceIoControl() to "krnldrvr"
    VDDCLOSE - Issue Win32 CloseHandle()

Arguments:

    Client (DX)    = Command code (VDDOPEN, VDDCLOSE, VDDINFO)

    For VDDINFO:
    Client (ES:DI) = IOCTL Read Buffer address
    Client (CX)    = Buffer Size

Return Value:

    SUCCESS - Client Carry Clear
    FAILURE - Client Carry Set

--*/


{
    LPVOID  Buffer;
    ULONG   VDMAddress;
    DWORD   dwCount;
    BOOL    Success = TRUE;
    DWORD   BytesReturned;
    static HANDLE hDriver = INVALID_HANDLE_VALUE;

    switch (getDX()) {

        case VDDOPEN:

            hDriver = CreateFile("\\\\.\\krnldrvr",    /* open KRNLDRVR      */
                            GENERIC_READ | GENERIC_WRITE,  /* open for r/w   */
                            0,                         /* can't share        */
                            (LPSECURITY_ATTRIBUTES) NULL, /* no security     */
                            OPEN_EXISTING,             /* existing file only */
                            0,                         /* flags              */
                            (HANDLE) NULL);            /* no attr template   */

            if (hDriver == INVALID_HANDLE_VALUE) {
                setCF(1);
            } else
                setCF(0);

            break;

        case VDDCLOSE:

            if (hDriver != INVALID_HANDLE_VALUE) {
                CloseHandle (hDriver);
                hDriver = INVALID_HANDLE_VALUE;
            }

            break;

        case VDDINFO:

            dwCount = (DWORD) getCX();

            VDMAddress = (ULONG) (getES()<<16 | getDI());

            Buffer = (LPVOID) GetVDMPointer (VDMAddress, dwCount, FALSE);

            Success = DeviceIoControl (hDriver,
                (DWORD) IOCTL_KRNLDRVR_GET_INFORMATION,
                (LPVOID) NULL, 0,
                Buffer, dwCount,
                &BytesReturned, (LPVOID) NULL);

            if (Success) {
                setCF(0);
                setCX((WORD)BytesReturned);
            } else
                setCF(1);

            FreeVDMPointer (VDMAddress, dwCount, Buffer, FALSE);

            break;

        default:
            setCF(1);
    }
    return;
}


