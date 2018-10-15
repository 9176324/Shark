//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2000
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

#include "34AVStrm.h"
#include "OSDepend.h"

// some additional functions, which are not in the Kernel run time library

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function compares string1 and string2 lexicographically and returns
//  a value indicating their relationship. If the CaseInsensitive flag is set
//  the function compares lowercase versions of string1 and string2.
//  We wrote several helper functions in C to have global access from the HAL.
//  A better architecture approach would be to have these helper functions
//  in a "OSDepend" base class.
//
// Return Value:
//  < 0     string1 less than string2.
//  0       string1 identical to string2.
//  > 0     string1 greater than string2.
//
//////////////////////////////////////////////////////////////////////////////
LONG KMDCompareString
(
    IN PCHAR  pString1,   //Null-terminated strings to compare
    IN PCHAR  pString2,   //Null-terminated strings to compare
    BOOL bCaseInsensitive //Indicates if compares with lowercase.
)
{
    // check if parameters are valid
    if( !pString1 || !pString2 )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return 0;
    }
    // check if case sensitive comparision was selected
    if( bCaseInsensitive )
    {
        // compare case sensitive
        return _stricmp(pString1, pString2);
    }
    else
    {
        // compare case in-sensitive
        return strcmp(pString1, pString2);
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Writes formatted output using a pointer to a list of arguments.
//  This function formats and prints a series of characters and values to the
//  memory pointed to by buffer If arguments follow the format string, the
//  format string must contain specifications that determine the output format
//  for the arguments.
//  We wrote several helper functions in C to have global access from the HAL.
//  A better architecture approach would be to have these helper functions
//  in a "OSDepend" base class.
//
// Return Value:
//  Returns the number of characters written, not including the terminating
//  null character, or a negative value if an output error occurs.
//  if the number of bytes to write exceeds buffer, then count bytes are
//  written and -1 is returned.
//
//////////////////////////////////////////////////////////////////////////////
int __cdecl KMDsnprintf
(
    PCHAR pbuffer,      //Points to the storage location for output
    size_t count,       //Maximum number of characters to write
    const PCHAR format, //Format specification (see printf or more details)
    ...
)
{
    // check if parameters are valid
    if( !pbuffer )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return -1;
    }
    // initialize format list
    va_list vaFormatList;
    // go through format list and print it into the buffer
    va_start( vaFormatList, format );
    int ret = _vsnprintf( pbuffer, count, format, vaFormatList );
    va_end( vaFormatList );
    // return number of characters written or -1 if operation fails
    return ret;
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor intitiates an instance of COSDependRegisterAccess.
//
//////////////////////////////////////////////////////////////////////////////
COSDependRegisterAccess::COSDependRegisterAccess()
{
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor destroys the instance of COSDependRegisterAccess.
//
//////////////////////////////////////////////////////////////////////////////
COSDependRegisterAccess::~COSDependRegisterAccess()
{
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Reads a value from a register that was mapped into system space.
//
// Return Value:
//  Returns the value that was read.
//
// Remarks;
//  this inferface does not support a way to signal wether the pAddress
//  argument includes a invalid register address.
//
//////////////////////////////////////////////////////////////////////////////
DWORD COSDependRegisterAccess::GetReg
(
    PBYTE pAddress //Points to address (Register) to read from.
)
{
    // check if parameters are valid
    if( !pAddress )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return 0;
    }
    // check current IRQ level, we assume that user mode calls to this
    // function will occur below APC level so we can call ProbeForRead
    // before accessing the address
    if( KeGetCurrentIrql() < APC_LEVEL )
    {
        __try
        {
            // this function does not support a return value, so we
            // assume that it never fails
//            ProbeForRead(static_cast <PVOID> (pAddress),
//                         sizeof(DWORD),
//                         sizeof(char));
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid address"));
            return 0;
        }
    }
    // return value of given address
    return *(reinterpret_cast <PDWORD> (pAddress));
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Writes a value to a register that was mapped into system space.
//
// Return Value:
//  none.
//
//////////////////////////////////////////////////////////////////////////////
void COSDependRegisterAccess::SetReg
(
    PBYTE pAddress, //Points to address (Register) to write.
    DWORD dwValue   //The write value. Must be less than or equal to MAXDWORD.
)
{
    // check if parameters are valid
    if( !pAddress )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return;
    }
    // check current IRQ level, we assume that user mode calls to this
    // function will occur below APC level so we can call ProbeForRead
    // before accessing the address
    if( KeGetCurrentIrql() < APC_LEVEL )
    {
        __try
        {
            // this function does not support a return value, so we
            // assume that it never fails
//            ProbeForWrite(static_cast <PVOID> (pAddress),
//                          sizeof(DWORD),
//                          4);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid address"));
            return;
        }
    }
    // write value to given address
    *(reinterpret_cast <PDWORD>(pAddress)) = dwValue;
}
