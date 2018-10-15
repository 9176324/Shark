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

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Converts ANSI source string into a UNICODE string.
//  This function is used only inside the registry class, we cannot make it
//  a private class function without changing the interface (osdepend.h)
//  so we have to make the function global C. A better architecture approach
//  would be to use the factory class for this, too.
//  Callers must be running in PASSIVE_LEVEL.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed.
//  STATUS_SUCCESS          Converted the string with sucess.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS GetUnicodeFromAnsi
(
    PCHAR pszAnsiString, //Pointer to the ANSI string to be converted to
                         //Unicode
    PUNICODE_STRING pwcUnicodeString //Pointer to a UNICODE_STRING structure
                                     //to hold the converted Unicode string.
)
{
    // check if parameters are valid
    if( !pszAnsiString || !pwcUnicodeString )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    // check current IRQ level
    if( KeGetCurrentIrql() != PASSIVE_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level not correct"));
        return STATUS_UNSUCCESSFUL;
    }
    // check if parameters are within valid range
    if( strlen(pszAnsiString) > MAX_PATH )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: String invalid"));
        return STATUS_UNSUCCESSFUL;
    }
    ANSI_STRING szAnsiString;
    // fill Ansi string structure
    szAnsiString.Length =
        static_cast <USHORT> ( strlen(pszAnsiString) );
    szAnsiString.MaximumLength =
        static_cast <USHORT> ( strlen(pszAnsiString) );
    szAnsiString.Buffer =
        pszAnsiString;
    // convert ansi string to unicode string
    NTSTATUS ntStatus = RtlAnsiStringToUnicodeString(pwcUnicodeString,
                                                     &szAnsiString,
                                                     TRUE);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: RtlAnsiStringToUnicodeString failed"));
        return ntStatus;
    }
    return ntStatus;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Converts given subkey path into WDM registry handle.
//  This function is used only inside the registry class, we cannot make it
//  a private class function without changing the interface (osdepend.h)
//  so we have to make the function global C. A better architecture approach
//  would be to use the factory class for this, too.
//  Callers must be running in PASSIVE_LEVEL.
//
// Return Value:
//  VAMPRET_GENERAL_ERROR   Operations fails,
//  STATUS_SUCCESS          Converted the subkey with success.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CreateRegistrySubkey
(
    IN PDEVICE_OBJECT pPhysicalDeviceObject,    //Pointer to a physical device
                                                //object.
    IN PCHAR pszRegistrySubkeyPath,             //Pointer to a null-terminated
                                                //string that specifies the
                                                //path to the registry subkey.
    OUT PHANDLE phRegistryHandle                //Handle to registry subkey
)
{
    // check if parameters are valid
    if( !pPhysicalDeviceObject || !phRegistryHandle )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    // check current IRQ level
    if( KeGetCurrentIrql() != PASSIVE_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level not correct"));
        return NULL;
    }

    // if we accidently return from this function in case of an error
    // the handle should be NULL
    *phRegistryHandle = NULL;
    // open hardware registry key
    NTSTATUS ntStatus = IoOpenDeviceRegistryKey(pPhysicalDeviceObject,
                                                PLUGPLAY_REGKEY_DRIVER,
                                                KEY_QUERY_VALUE     |
                                                KEY_SET_VALUE       |
                                                KEY_CREATE_SUB_KEY,
                                                phRegistryHandle);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IoOpenDeviceRegistryKey failed"));
        return VAMPRET_GENERAL_ERROR;
    }
    // if subkey was not defined return the actual handle
    if( !pszRegistrySubkeyPath )
    {
        _DbgPrintF(DEBUGLVL_BLAB,("Info: No subkey defined"));
        return STATUS_SUCCESS;
    }
    // check if parameters are within valid range
    if( strlen(pszRegistrySubkeyPath) > MAX_PATH )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: String invalid"));
        return STATUS_UNSUCCESSFUL;
    }

    //*** Subkeys detected !                                        ***//
    //*** The registry handling in WDM is very uncomfortable        ***//
    //*** so we have to spend some effort to support subkeys here:  ***//

    // store actual registry handle
    HANDLE hNextRegistryHandle = *phRegistryHandle;
    *phRegistryHandle = NULL;
    // we cannot access pszRegistrySubkeyPath directly so we allocate
    // temporary space
    PCHAR pszNextSubkey =
        new (NON_PAGED_POOL) CHAR[strlen(pszRegistrySubkeyPath)+1];
    if( !pszNextSubkey )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Memory allocation failed"));
        // close hardware registry key
        ntStatus = ZwClose(hNextRegistryHandle);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_GENERAL_ERROR;
    }
    // copy pointer for later deletion
    PCHAR pszTempPointer = pszNextSubkey;
    // copy subkey path into temporary space
    strncpy(pszNextSubkey, pszRegistrySubkeyPath, strlen(pszRegistrySubkeyPath)+1);

    // initialize additional variables
    PCHAR pszActualSubkey = NULL;
    HANDLE hActualRegistryHandle = NULL;
    UNICODE_STRING wcActualSubkey;
    OBJECT_ATTRIBUTES InitializedAttributes;

    // start subkey conversion
    do
    {
        // the previous "next" subkey is the actual subkey
        pszActualSubkey = pszNextSubkey;
        // the previous "next" handle is the actual handle
        hActualRegistryHandle = hNextRegistryHandle;
        // search for end of actual subkey in string and
        // set the next pointer to this locatation
        pszNextSubkey = strchr(pszActualSubkey, '\\');
        // check if there are further subkeys available
        if( pszNextSubkey )
        {
            // more subkeys available, make end of subkey end of string
            *pszNextSubkey = 0;
            // set next subkey pointer to beginning of next subkey
            pszNextSubkey++;
        }
        // convert actual subkey string to unicode
        ntStatus = GetUnicodeFromAnsi(pszActualSubkey, &wcActualSubkey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: GetUnicodeFromAscii failed"));
            // close hardware registry key
            ntStatus = ZwClose(hActualRegistryHandle);
            if( ntStatus != STATUS_SUCCESS )
            {
                _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
                return VAMPRET_GENERAL_ERROR;
            }
            return VAMPRET_GENERAL_ERROR;
        }
        // initialize object attributes for ZwCreateKey function
        InitializeObjectAttributes(&InitializedAttributes,
                                   &wcActualSubkey,
                                   OBJ_CASE_INSENSITIVE,
                                   hActualRegistryHandle,
                                   NULL);
        // create actual subkey
        ntStatus = ZwCreateKey(&hNextRegistryHandle,
                               KEY_ALL_ACCESS,
                               &InitializedAttributes,
                               0,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               NULL);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwCreateKey (Subkey) failed"));
            // release memory for unicode string
            RtlFreeUnicodeString(&wcActualSubkey);
            // close hardware registry key
            ntStatus = ZwClose(hActualRegistryHandle);
            if( ntStatus != STATUS_SUCCESS )
            {
                _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
                return VAMPRET_GENERAL_ERROR;
            }
            return VAMPRET_GENERAL_ERROR;
        }
        // close old hardware registry key
        ntStatus = ZwClose(hActualRegistryHandle);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            // release memory for unicode string
            RtlFreeUnicodeString(&wcActualSubkey);
            return VAMPRET_GENERAL_ERROR;
        }
        // release memory for unicode string
        RtlFreeUnicodeString(&wcActualSubkey);

    }while(pszNextSubkey);
    // de-allocate temporary memory
    delete [] pszTempPointer;
    pszNextSubkey = NULL;
    // store found registry handle in output pointer
    *phRegistryHandle = hNextRegistryHandle;
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor.
//  Assigns the member variable with the given physical device object.
//////////////////////////////////////////////////////////////////////////////
COSDependRegistry::COSDependRegistry
(
    PVOID pPhysicalDeviceObject //Pointer physical device object
)
{
    m_pvHandle = NULL;
    // check if parameters are valid
    if( !pPhysicalDeviceObject )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return;
    }
    m_pvHandle = pPhysicalDeviceObject;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor.
//
//////////////////////////////////////////////////////////////////////////////
COSDependRegistry::~COSDependRegistry()
{
     m_pvHandle = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Reads data from registry. Callers must be running at IRQL = PASSIVE_LEVEL.
//
// Return Value:
//  VAMPRET_GENERAL_ERROR   Operations failed,
//                          (a) the function parameters are invalid
//                          (b) the key does not exist
//                          (c) target buffer is too small
//                          (d) the IRQ level is not passive
//  VAMPRET_SUCCESS         Read the string from registry with success.
//
//////////////////////////////////////////////////////////////////////////////
VAMPRET COSDependRegistry::ReadRegistry
(
    PCHAR pszValueName,             //Pointer to a null-terminated string that
                                    //specifies the name of the string value.
    PCHAR pszTargetBuffer,          //Target buffer (the string will be placed
                                    //here).
    DWORD dwTargetBufferLength,     //Length of target buffer in byte (if
                                    //too short the I/O registry operation
                                    //will fail) If the length of the string
                                    //to read is unknow use MAX_PATH to ensure
                                    //that the string fits into the buffer.
    PCHAR pszRegistrySubkeyPath,    //Pointer to a null-terminated string that
                                    //specifies the registry path to the
                                    //string value. Can be omitted. The
                                    //default parameter NULL will be used
                                    //then.
    PCHAR pszDefaultString          //Pointer to null-terminated string that
                                    //specifies the default string in
                                    //pszBuffer (if operation fails). Can be
                                    //omitted. The default parameter NULL will
                                    //be used then.
)
{
    // skipped due to message reduction:
    // _DbgPrintF(DEBUGLVL_BLAB,("Info: ReadRegistry called"));

    // check if parameters are valid
    if( !pszValueName || !pszTargetBuffer || !pszDefaultString )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return VAMPRET_GENERAL_ERROR;
    }
    if( !m_pvHandle )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Important member not initialized"));
        return VAMPRET_GENERAL_ERROR;
    }
    // check if parameters are within valid range
    if( (strlen(pszDefaultString)+1) > dwTargetBufferLength )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Buffer overflow"));
        return VAMPRET_GENERAL_ERROR;
    }
    __try
    {
        strncpy(pszTargetBuffer, pszDefaultString, strlen(pszDefaultString)+1);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Copy default string failed"));
        return VAMPRET_GENERAL_ERROR;
    }
    // check current IRQ level
    if( KeGetCurrentIrql() != PASSIVE_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level is not passive"));
        return VAMPRET_GENERAL_ERROR;
    }
    // create registry subkey handle
    HANDLE hRegistryKey = NULL;
    NTSTATUS ntStatus = CreateRegistrySubkey(
                                static_cast <PDEVICE_OBJECT> (m_pvHandle),
                                pszRegistrySubkeyPath,
                                &hRegistryKey);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: CreateRegistrySubkey failed"));
        return VAMPRET_GENERAL_ERROR;
    }
    // convert value name into unicode
    UNICODE_STRING wcValueName;
    ntStatus = GetUnicodeFromAnsi(pszValueName, &wcValueName);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: GetUnicodeFromAscii failed"));
        // close hardware registry key
        ntStatus = ZwClose(hRegistryKey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_GENERAL_ERROR;
    }
    // get length of registry value information
    DWORD dwResultLength = 0;
    // this call is supposed to fail so we do not need to check the return
    // value
    ntStatus = ZwQueryValueKey(hRegistryKey,
                               &wcValueName,
                               KeyValuePartialInformation,
                               NULL,
                               0,
                               &dwResultLength);
    if( ntStatus != STATUS_SUCCESS )
    {
        //STATUS_BUFFER_OVERFLOW
        //STATUS_BUFFER_TOO_SMALL
        //STATUS_INVALID_PARAMETER
        //STATUS_OBJECT_NAME_NOT_FOUND
        //_DbgPrintF(DEBUGLVL_BLAB,("ZwQueryValueKey failed on purpose"));
    }

    if( !dwResultLength )
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("Warning: Registry key not found"));
        // release memory for unicode string
        RtlFreeUnicodeString(&wcValueName);
        // close hardware registry key
        ntStatus = ZwClose(hRegistryKey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_GENERAL_ERROR;
    }
    // allocate memory for information data
    PKEY_VALUE_PARTIAL_INFORMATION pValueInformation =
        reinterpret_cast <PKEY_VALUE_PARTIAL_INFORMATION>
        ( new (NON_PAGED_POOL) BYTE[dwResultLength] );
    if( !pValueInformation )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Cannot allocate memory for value information"));
        // release memory for unicode string
        RtlFreeUnicodeString(&wcValueName);
        // close hardware registry key
        ntStatus = ZwClose(hRegistryKey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_GENERAL_ERROR;
    }
    pValueInformation->DataLength = dwResultLength;
    // get information data from registry key
    ntStatus = ZwQueryValueKey(hRegistryKey,
                               &wcValueName,
                               KeyValuePartialInformation,
                               pValueInformation,
                               pValueInformation->DataLength,
                               &dwResultLength);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwQueryValueKey failed"));
        // release memory for information data
        delete [] reinterpret_cast <PBYTE> (pValueInformation);
        pValueInformation = NULL;
        // release memory for unicode string
        RtlFreeUnicodeString(&wcValueName);
        // close hardware registry key
        ntStatus = ZwClose(hRegistryKey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_GENERAL_ERROR;
    }
    // check if output buffer is large enough
    if( pValueInformation->DataLength+1 > dwTargetBufferLength )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Buffer overflow"));
        // release memory for information data
        delete [] reinterpret_cast <PBYTE> (pValueInformation);
        pValueInformation = NULL;
        // release memory for unicode string
        RtlFreeUnicodeString(&wcValueName);
        // close hardware registry key
        ntStatus = ZwClose(hRegistryKey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_GENERAL_ERROR;
    }
    // write registry data into output buffer
    __try
    {
        for(DWORD dwCounter = 0;dwCounter < dwTargetBufferLength; dwCounter++)
        {
            pszTargetBuffer[dwCounter] =
                static_cast <CHAR> (pValueInformation->Data[dwCounter]);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Copy registry string failed"));
        // release memory for information data
        delete [] ( reinterpret_cast <PBYTE> (pValueInformation) );
        pValueInformation = NULL;
        // release memory for unicode string
        RtlFreeUnicodeString(&wcValueName);
        // close hardware registry key
        ntStatus = ZwClose(hRegistryKey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_GENERAL_ERROR;
    }
    // release memory for information data
    delete [] ( reinterpret_cast <PBYTE> (pValueInformation) );
    pValueInformation = NULL;
    // release memory for unicode string
    RtlFreeUnicodeString(&wcValueName);
    // close hardware registry key
    ntStatus = ZwClose(hRegistryKey);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
        return VAMPRET_GENERAL_ERROR;
    }
    return VAMPRET_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Reads data from the registry.
//  Callers must be running at IRQL = PASSIVE_LEVEL.
//
// Return Value:
//  returns status of registry operation
//
//  VAMPRET_GENERAL_ERROR   Operations fails,
//  VAMPRET_SUCCESS         Read the binary value from registry with success.
//
//////////////////////////////////////////////////////////////////////////////
VAMPRET COSDependRegistry::ReadRegistry
(
    PCHAR pszValueName,             //Pointer to a null-terminated string that
                                    //specifies the name of the binary value.
    PDWORD pdwTargetBuffer,         //Pointer to a DWORD (the binary value
                                    //will be placed here)
    PCHAR pszRegistrySubkeyPath,    //Pointer to a null-terminated string that
                                    //specifies the registry path to the
                                    //binary value. Can be omitted. The
                                    //default parameter NULL will be used
                                    //then.
    DWORD dwDefaultValue            //Default value in pdwTarget Buffer if
                                    //operation fails. Can be omitted. The
                                    //default parameter 0 will be used then.
)
{
    // skipped due to message reduction:
    // _DbgPrintF(DEBUGLVL_BLAB,("Info: ReadRegistry called"));

    // check if parameters are valid
    if( !pszValueName || !pdwTargetBuffer )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return VAMPRET_GENERAL_ERROR;
    }
    if( !m_pvHandle )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Important member not initialized"));
        return VAMPRET_GENERAL_ERROR;
    }

    // pszRegistrySubkeyPath > MAX_PATH already checked
    // in calling CreateRegCreateRegistrySubkey function

    __try
    {
        *pdwTargetBuffer = dwDefaultValue;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Copy default value failed"));
        return VAMPRET_GENERAL_ERROR;
    }
    // check current IRQ level
    if( KeGetCurrentIrql() != PASSIVE_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level is not passive"));
        return VAMPRET_GENERAL_ERROR;
    }
    // create registry subkey handle
    HANDLE hRegistryKey = NULL;
    NTSTATUS ntStatus = CreateRegistrySubkey(
                            static_cast <PDEVICE_OBJECT> (m_pvHandle),
                            pszRegistrySubkeyPath,
                            &hRegistryKey);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: CreateRegistrySubkey failed"));
        return VAMPRET_GENERAL_ERROR;
    }
    // convert value name into unicode
    UNICODE_STRING wcValueName;
    ntStatus = GetUnicodeFromAnsi(pszValueName, &wcValueName);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: GetUnicodeFromAscii failed"));
        // close hardware registry key
        ntStatus = ZwClose(hRegistryKey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_GENERAL_ERROR;
    }
    // get length of registry value information
    DWORD dwResultLength = 0;
    // this call is supposed to fail so we do not need to check the return
    // value
    ntStatus = ZwQueryValueKey(hRegistryKey,
                               &wcValueName,
                               KeyValuePartialInformation,
                               NULL,
                               0,
                               &dwResultLength);
    if( ntStatus != STATUS_SUCCESS )
    {
        //STATUS_BUFFER_OVERFLOW
        //STATUS_BUFFER_TOO_SMALL
        //STATUS_INVALID_PARAMETER
        //STATUS_OBJECT_NAME_NOT_FOUND
        //_DbgPrintF(DEBUGLVL_BLAB,("ZwQueryValueKey failed on purpose"));
    }

    if( !dwResultLength )
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("Warning: Registry key not found"));
        // release memory for unicode string
        RtlFreeUnicodeString(&wcValueName);
        // close hardware registry key
        ntStatus = ZwClose(hRegistryKey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_GENERAL_ERROR;
    }
    // allocate memory for information data
    PKEY_VALUE_PARTIAL_INFORMATION pValueInformation =
        reinterpret_cast <PKEY_VALUE_PARTIAL_INFORMATION>
        ( new (NON_PAGED_POOL) BYTE[dwResultLength] );
    if( !pValueInformation )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Cannot allocate memory for value information"));
        // release memory for unicode string
        RtlFreeUnicodeString(&wcValueName);
        // close hardware registry key
        ntStatus = ZwClose(hRegistryKey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_GENERAL_ERROR;
    }
    pValueInformation->DataLength = dwResultLength;
    // get information data from registry key
    ntStatus = ZwQueryValueKey(hRegistryKey,
                               &wcValueName,
                               KeyValuePartialInformation,
                               pValueInformation,
                               pValueInformation->DataLength,
                               &dwResultLength);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwQueryValueKey failed"));
        // release memory for information data
        delete [] reinterpret_cast <PBYTE> (pValueInformation);
        pValueInformation = NULL;
        // release memory for unicode string
        RtlFreeUnicodeString(&wcValueName);
        // close hardware registry key
        ntStatus = ZwClose(hRegistryKey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_GENERAL_ERROR;
    }
    // check if output buffer is large enough
    if( pValueInformation->Type != REG_DWORD )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Buffer type does not match"));
        // release memory for information data
        delete [] reinterpret_cast <PBYTE> (pValueInformation);
        pValueInformation = NULL;
        // release memory for unicode string
        RtlFreeUnicodeString(&wcValueName);
        // close hardware registry key
        ntStatus = ZwClose(hRegistryKey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_GENERAL_ERROR;
    }
    // write registry data into output buffer
    __try
    {
        *pdwTargetBuffer =
            *( reinterpret_cast <PDWORD> (pValueInformation->Data) );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Copy registry value failed"));
        // release memory for information data
        delete [] reinterpret_cast <PBYTE> (pValueInformation);
        pValueInformation = NULL;
        // release memory for unicode string
        RtlFreeUnicodeString(&wcValueName);
        // close hardware registry key
        ntStatus = ZwClose(hRegistryKey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_GENERAL_ERROR;
    }
    // release memory for information data
    delete [] reinterpret_cast <PBYTE> (pValueInformation);
    pValueInformation = NULL;
    // release memory for unicode string
    RtlFreeUnicodeString(&wcValueName);
    // close hardware registry key
    ntStatus = ZwClose(hRegistryKey);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
        return VAMPRET_GENERAL_ERROR;
    }
    return VAMPRET_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Writes data to the registry.
//  Callers must be running at IRQL = PASSIVE_LEVEL.
//
// Return Value:
//  VAMPRET_GENERAL_ERROR   Operations fails,
//  VAMPRET_SUCCESS         Wrote the string to the registry with success.
//
//////////////////////////////////////////////////////////////////////////////
VAMPRET COSDependRegistry::WriteRegistry
(
    PCHAR pszValueName,         //Pointer to a null-terminated string that
                                //specifies the name of the binary value.
    PCHAR pszString,            //Pointer to the string to be written in the
                                //registry. Must be less than or equal to
                                //MAX_PATH.
    PCHAR pszRegistrySubkeyPath //Pointer to a null-terminated string that
                                //specifies the registry path to the string
                                //value. Can be omitted. The default parameter
                                //NULL will be used then.
)
{
    // skipped due to message reduction:
    // _DbgPrintF(DEBUGLVL_BLAB,("Info: WriteRegistry called"));

    // check if parameters are valid
    if( !pszValueName || !pszString )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return VAMPRET_GENERAL_ERROR;
    }
    if( !m_pvHandle )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Important member not initialized"));
        return VAMPRET_GENERAL_ERROR;
    }
    // check current IRQ level
    if( KeGetCurrentIrql() != PASSIVE_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level is not passive"));
        return VAMPRET_GENERAL_ERROR;
    }
    // create registry subkey handle
    HANDLE hRegistryKey = NULL;
    NTSTATUS ntStatus = CreateRegistrySubkey(
                                static_cast <PDEVICE_OBJECT> (m_pvHandle),
                                pszRegistrySubkeyPath,
                                &hRegistryKey);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: CreateRegistrySubkey failed"));
        return VAMPRET_GENERAL_ERROR;
    }
    // convert value name into unicode
    UNICODE_STRING wcValueName;
    ntStatus = GetUnicodeFromAnsi(pszValueName, &wcValueName);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: GetUnicodeFromAscii failed"));
        // close hardware registry key
        ntStatus = ZwClose(hRegistryKey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_GENERAL_ERROR;
    }
    // convert value string into unicode
    UNICODE_STRING wcValueString;
    ntStatus = GetUnicodeFromAnsi(pszString, &wcValueString);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: GetUnicodeFromAscii failed"));
        // release memory for unicode string
        RtlFreeUnicodeString(&wcValueName);
        // close hardware registry key
        ntStatus = ZwClose(hRegistryKey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_GENERAL_ERROR;
    }
    // set registry value information
    ntStatus = ZwSetValueKey(hRegistryKey,
                             &wcValueName,
                             NULL,
                             REG_SZ,
                             &wcValueString,
                             wcValueString.Length+1);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwSetValueKey failed"));
        // release memory for unicode string
        RtlFreeUnicodeString(&wcValueName);
        // close hardware registry key
        ntStatus = ZwClose(hRegistryKey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_REGISTRY_IO_FAILED;
    }
    // release memory for unicode string
    RtlFreeUnicodeString(&wcValueName);
    // close hardware registry key
    ntStatus = ZwClose(hRegistryKey);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
        return VAMPRET_GENERAL_ERROR;
    }
    return VAMPRET_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Writes data to the registry.
//  Callers must be running at IRQL = PASSIVE_LEVEL.
//
// Return Value:
//  VAMPRET_GENERAL_ERROR   Operations fails,
//  VAMPRET_SUCCESS         Wrote the value to the registry with success.
//
//////////////////////////////////////////////////////////////////////////////
VAMPRET COSDependRegistry::WriteRegistry
(
    PCHAR pszValueName,         //Pointer to a null-terminated string that
                                //specifies the name of the binary value.
    DWORD dwValue,              //The  value to be written in the registry.
    PCHAR pszRegistrySubkeyPath //Pointer to a null-terminated string that
                                //specifies the registry path to the string
                                //value. Can be omitted. The default parameter
                                //NULL will be used then.
)
{
    // skipped due to message reduction:
    // _DbgPrintF(DEBUGLVL_BLAB,("Info: WriteRegistry called"));

    // check if parameters are valid
    if( !pszValueName )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return VAMPRET_GENERAL_ERROR;
    }
    if( !m_pvHandle )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Important member not initialized"));
        return VAMPRET_GENERAL_ERROR;
    }
    // check current IRQ level
    if( KeGetCurrentIrql() != PASSIVE_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level is not passive"));
        return VAMPRET_GENERAL_ERROR;
    }
    // create registry subkey handle
    HANDLE hRegistryKey = NULL;
    NTSTATUS ntStatus = CreateRegistrySubkey(
                            static_cast <PDEVICE_OBJECT> (m_pvHandle),
                            pszRegistrySubkeyPath,
                            &hRegistryKey);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: CreateRegistrySubkey failed"));
        return VAMPRET_GENERAL_ERROR;
    }
    // convert value name into unicode
    UNICODE_STRING wcValueName;
    ntStatus = GetUnicodeFromAnsi(pszValueName, &wcValueName);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: GetUnicodeFromAscii failed"));
        // close hardware registry key
        ntStatus = ZwClose(hRegistryKey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_GENERAL_ERROR;
    }
    // set registry value information
    ntStatus = ZwSetValueKey(hRegistryKey,
                             &wcValueName,
                             NULL,
                             REG_DWORD,
                             &dwValue,
                             sizeof(dwValue));
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwSetValueKey failed"));
        // release memory for unicode string
        RtlFreeUnicodeString(&wcValueName);
        // close hardware registry key
        ntStatus = ZwClose(hRegistryKey);
        if( ntStatus != STATUS_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
            return VAMPRET_GENERAL_ERROR;
        }
        return VAMPRET_REGISTRY_IO_FAILED;
    }
    // release memory for unicode string
    RtlFreeUnicodeString(&wcValueName);
    // close hardware registry key
    ntStatus = ZwClose(hRegistryKey);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: ZwClose failed"));
        return VAMPRET_GENERAL_ERROR;
    }
    return VAMPRET_SUCCESS;
}
