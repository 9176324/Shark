/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    sysinfo.c

Abstract:

    This module implements the NT set and query system information services.

--*/

#include "exp.h"
#pragma hdrstop

#include "stdlib.h"
#include "string.h"
#include "vdmntos.h"
#include <nturtl.h>
#include "pool.h"
#include "stktrace.h"
#include "align.h"

#if defined(_WIN64)

#include <wow64t.h>

#endif

#define COMPLUS_PACKAGE_KEYPATH      L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\.NETFramework"
#define COMPLUS_PACKAGE_ENABLE64BIT  L"Enable64Bit"
#define COMPLUS_PACKAGE_INVALID      (ULONG)-1


NTSTATUS
ExpReadComPlusPackage (
    VOID
    );

NTSTATUS
ExpUpdateComPlusPackage (
    IN ULONG ComPlusPackageStatus
    );


extern ULONG MmSystemCodePage;
extern ULONG MmSystemCachePage;
extern ULONG MmPagedPoolPage;
extern ULONG MmSystemDriverPage;
extern ULONG MmTotalSystemCodePages;
extern LONG MmTotalSystemDriverPages;
extern RTL_TIME_ZONE_INFORMATION ExpTimeZoneInformation;

//
// For SystemDpcBehaviorInformation
//

extern LIST_ENTRY MmLoadedUserImageList;

extern MMSUPPORT MmSystemCacheWs;
extern PFN_NUMBER MmTransitionSharedPages;
extern PFN_NUMBER MmTransitionSharedPagesPeak;

#define ROUND_UP(VALUE,ROUND) ((ULONG)(((ULONG)VALUE + \
                               ((ULONG)ROUND - 1L)) & (~((ULONG)ROUND - 1L))))
                               
//
// For referencing a user-supplied event handle
//

extern POBJECT_TYPE ExEventObjectType;

//
// Watchdog Handler
//

PWD_HANDLER ExpWdHandler = NULL;
PVOID ExpWdHandlerContext = NULL;

//
// COM+ Package Install Status
//

const static UNICODE_STRING KeyName = RTL_CONSTANT_STRING(COMPLUS_PACKAGE_KEYPATH);
static UNICODE_STRING KeyValueName = RTL_CONSTANT_STRING(COMPLUS_PACKAGE_ENABLE64BIT);

//
// Define Win32k full path.
//

#define WIN32K_FILE_PATH L"\\SystemRoot\\System32\\win32k.sys"
#define WIN32K_PATH_SIZE (sizeof(WIN32K_FILE_PATH) - sizeof(WCHAR))

WCHAR Win32kFullPath[] = WIN32K_FILE_PATH;

//
// Firmware Table handler list
//

LIST_ENTRY  ExpFirmwareTableProviderListHead;

//
// Firmware Table ERESOURCE
//

ERESOURCE  ExpFirmwareTableResource;

//
// Firmware Table handler node
//

typedef struct _SYSTEM_FIRMWARE_TABLE_HANDLER_NODE {
    SYSTEM_FIRMWARE_TABLE_HANDLER SystemFWHandler;  
    LIST_ENTRY FirmwareTableProviderList;
} SYSTEM_FIRMWARE_TABLE_HANDLER_NODE, *PSYSTEM_FIRMWARE_TABLE_HANDLER_NODE;

NTSTATUS
ExpValidateLocale (
    IN LCID LocaleId
    );

BOOLEAN
ExpIsValidUILanguage (
    IN WCHAR *pLangId
    );

NTSTATUS
ExpGetCurrentUserUILanguage (
    IN WCHAR *ValueName,
    OUT LANGID *CurrentUserUILanguageId,
    IN BOOLEAN bCheckGP
    );

NTSTATUS
ExpSetCurrentUserUILanguage (
    IN WCHAR *ValueName,
    IN LANGID DefaultUILanguageId
    );

NTSTATUS
ExpGetUILanguagePolicy (
    IN HANDLE CurrentUserKey,
    OUT LANGID *PolicyUILanguageId
    );

NTSTATUS
ExpGetProcessInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length OPTIONAL,
    IN PULONG SessionId OPTIONAL,
    IN BOOLEAN ExtendedInformation
    );

NTSTATUS
ExGetSessionPoolTagInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length,
    IN PULONG SessionId
    );

NTSTATUS
ExpGetProcessorIdleInformation (
    OUT PVOID   SystemInformation,
    IN  ULONG   SystemInformationLength,
    OUT PULONG  Length
    );

NTSTATUS
ExpGetProcessorPowerInformation (
    OUT PVOID   SystemInformation,
    IN  ULONG   SystemInformationLength,
    OUT PULONG  Length
    );

VOID
ExpCopyProcessInfo (
    IN PSYSTEM_PROCESS_INFORMATION ProcessInfo,
    IN PEPROCESS Process,
    IN BOOLEAN ExtendedInformation
    );

VOID
ExpCopyThreadInfo (
    IN PVOID ThreadInfoBuffer,
    IN PETHREAD Thread,
    IN BOOLEAN ExtendedInformation
    );

#if i386

NTSTATUS
ExpGetStackTraceInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    );

#endif // i386

NTSTATUS
ExpGetLockInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    );

NTSTATUS
ExpGetLookasideInformation (
    OUT PVOID Buffer,
    IN ULONG BufferLength,
    OUT PULONG Length
    );

NTSTATUS
ExpGetHandleInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    );

NTSTATUS
ExpGetHandleInformationEx (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    );

NTSTATUS
ExpGetObjectInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    );

NTSTATUS
ExpGetInstemulInformation (
    OUT PSYSTEM_VDM_INSTEMUL_INFO Info
    );

NTSTATUS
ExGetPoolTagInfo (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN OUT PULONG ReturnLength OPTIONAL
    );

NTSTATUS
ExGetSessionPoolTagInfo (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN OUT PULONG ReturnedEntries,
    IN OUT PULONG ActualEntries
    );

NTSTATUS
ExGetBigPoolInfo (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN OUT PULONG ReturnLength OPTIONAL
    );

NTSTATUS
ExpQueryModuleInformation (
    IN PLIST_ENTRY LoadOrderListHead,
    IN PLIST_ENTRY UserModeLoadOrderListHead,
    OUT PRTL_PROCESS_MODULES ModuleInformation,
    IN ULONG ModuleInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSTATUS
ExpQueryLegacyDriverInformation (
    IN PSYSTEM_LEGACY_DRIVER_INFORMATION LegacyInfo,
    IN PULONG Length
    );

NTSTATUS
ExpQueryNumaProcessorMap (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnedLength
    );

NTSTATUS
ExpQueryNumaAvailableMemory (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnedLength
    );

NTSTATUS
ExpGetSystemFirmwareTableInformation (
    IN OUT PVOID SystemFirmwareTableInformation,
    IN  KPROCESSOR_MODE PreviousMode,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSTATUS
ExpRegisterFirmwareTableInformationHandler (
    IN OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN KPROCESSOR_MODE PreviousMode
    );

#pragma alloc_text(PAGE, NtQueryDefaultLocale)
#pragma alloc_text(PAGE, NtSetDefaultLocale)
#pragma alloc_text(PAGE, NtQueryInstallUILanguage)
#pragma alloc_text(PAGE, NtQueryDefaultUILanguage)
#pragma alloc_text(PAGE, ExpGetCurrentUserUILanguage)
#pragma alloc_text(PAGE, NtSetDefaultUILanguage)
#pragma alloc_text(PAGE, ExpSetCurrentUserUILanguage)
#pragma alloc_text(PAGE, ExpValidateLocale)
#pragma alloc_text(PAGE, ExpGetUILanguagePolicy)
#pragma alloc_text(PAGE, NtQuerySystemInformation)
#pragma alloc_text(PAGE, NtSetSystemInformation)
#pragma alloc_text(PAGE, ExpGetHandleInformation)
#pragma alloc_text(PAGE, ExpGetHandleInformationEx)
#pragma alloc_text(PAGE, ExpGetObjectInformation)
#pragma alloc_text(PAGE, ExpQueryModuleInformation)
#pragma alloc_text(PAGE, ExpCopyProcessInfo)
#pragma alloc_text(PAGE, ExpQueryLegacyDriverInformation)
#pragma alloc_text(PAGE, ExLockUserBuffer)
#pragma alloc_text(PAGE, ExpQueryNumaAvailableMemory)
#pragma alloc_text(PAGE, ExpQueryNumaProcessorMap)
#pragma alloc_text(PAGE, ExpReadComPlusPackage)
#pragma alloc_text(PAGE, ExpUpdateComPlusPackage)
#pragma alloc_text(PAGE, ExGetSessionPoolTagInformation)
#pragma alloc_text(PAGELK, ExpGetLockInformation)
#pragma alloc_text(PAGELK, ExpGetProcessorPowerInformation)
#pragma alloc_text(PAGELK, ExpGetProcessorIdleInformation)
#pragma alloc_text(PAGE, ExpIsValidUILanguage)
#pragma alloc_text(PAGE, ExpGetSystemFirmwareTableInformation)
#pragma alloc_text(PAGE, ExpRegisterFirmwareTableInformationHandler)

NTSTATUS
ExpReadComPlusPackage (
    VOID
    )

/*++

Routine Description:

    This function reads the status of the 64-bit COM+ package from the registry
    and stick it inside the shared page.

Arguments:

    None.

Return Value:

    NTSTATUS.

--*/

{
    NTSTATUS Status;
    static OBJECT_ATTRIBUTES ObjectAttributes = RTL_CONSTANT_OBJECT_ATTRIBUTES (&KeyName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE);
    CHAR KeyValueBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG)];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG ResultLength;
    HANDLE Key;


    Status = ZwOpenKey (&Key,
                        GENERIC_READ,
                        &ObjectAttributes);

    if (NT_SUCCESS (Status)) {

        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;
        Status = ZwQueryValueKey (Key,
                                  &KeyValueName,
                                  KeyValuePartialInformation,
                                  KeyValueInformation,
                                  sizeof (KeyValueBuffer),
                                  &ResultLength);

        if (NT_SUCCESS (Status)) {

            if ((KeyValueInformation->Type == REG_DWORD) &&
                (KeyValueInformation->DataLength == sizeof(ULONG))) {
                SharedUserData->ComPlusPackage = *(PULONG)KeyValueInformation->Data;
            }
        }

        ZwClose (Key);
    }

    return Status;
}

NTSTATUS
ExpUpdateComPlusPackage (
    IN ULONG ComPlusPackageStatus
    )

/*++

Routine Description:

    This function updates the COM+ runtime package status on the system.
    The package status indicates whether the 64-bit or the 32-bit runtime
    should be used when executing IL_ONLY COM+ images.

Arguments:

    ComPlusPackageStatus - COM+ Runtime package status on the system


Return Value:

    NTSTATUS

--*/

{
    NTSTATUS Status;
    static OBJECT_ATTRIBUTES ObjectAttributes = RTL_CONSTANT_OBJECT_ATTRIBUTES (&KeyName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE | OBJ_FORCE_ACCESS_CHECK);
    ULONG Disposition;
    HANDLE Key;


    Status = ZwOpenKey (&Key,
                        GENERIC_WRITE,
                        &ObjectAttributes
                        );

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
        Status = ZwCreateKey (&Key,
                              GENERIC_WRITE,
                              &ObjectAttributes,
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              &Disposition
                            );
    }

    if (NT_SUCCESS (Status)) {

        Status = ZwSetValueKey (Key,
                                &KeyValueName,
                                0,
                                REG_DWORD,
                                &ComPlusPackageStatus,
                                sizeof(ULONG));
        ZwClose (Key);
    }

    return Status;
}

NTSTATUS
NtQueryDefaultLocale (
    __in BOOLEAN UserProfile,
    __out PLCID DefaultLocaleId
    )
{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PAGED_CODE();

    Status = STATUS_SUCCESS;
    try {

        //
        // Get previous processor mode and probe output argument if necessary.
        //

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForWriteUlong ((PULONG)DefaultLocaleId);
        }

        if (UserProfile) {
            *DefaultLocaleId = MmGetSessionLocaleId ();
        }
        else {
            *DefaultLocaleId = PsDefaultSystemLocaleId;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    return Status;
}

NTSTATUS
NtSetDefaultLocale (
    __in BOOLEAN UserProfile,
    __in LCID DefaultLocaleId
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyPath, KeyValueName;
    HANDLE CurrentUserKey = NULL, Key;
    WCHAR KeyValueBuffer[ 128 ];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG ResultLength;
    PWSTR s;
    ULONG n, i, Digit;
    WCHAR c;
    ULONG Flags;

    PAGED_CODE();

    if (DefaultLocaleId & 0xFFFF0000) {
        return STATUS_INVALID_PARAMETER;
    }

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;
    if (UserProfile) {
        Status = RtlOpenCurrentUser( KEY_ENUMERATE_SUB_KEYS, &CurrentUserKey );
        if (!NT_SUCCESS( Status )) {
            return Status;
        }

        RtlInitUnicodeString( &KeyValueName, L"Locale" );
        RtlInitUnicodeString( &KeyPath, L"Control Panel\\International" );
        Flags = OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE | OBJ_FORCE_ACCESS_CHECK;
    }
    else {
        RtlInitUnicodeString( &KeyValueName, L"Default" );
        RtlInitUnicodeString( &KeyPath, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\Language" );
        CurrentUserKey = NULL;
        Flags = OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE;
    }

    InitializeObjectAttributes (&ObjectAttributes,
                                &KeyPath,
                                Flags,
                                CurrentUserKey,
                                NULL);

    if (DefaultLocaleId == 0) {

        Status = ZwOpenKey (&Key, GENERIC_READ, &ObjectAttributes);

        if (NT_SUCCESS( Status )) {
            Status = ZwQueryValueKey( Key,
                                      &KeyValueName,
                                      KeyValuePartialInformation,
                                      KeyValueInformation,
                                      sizeof( KeyValueBuffer ),
                                      &ResultLength
                                    );
            if (NT_SUCCESS( Status )) {
                if (KeyValueInformation->Type == REG_SZ) {
                    s = (PWSTR)KeyValueInformation->Data;
                    for (i=0; i<KeyValueInformation->DataLength; i += sizeof( WCHAR )) {
                        c = *s++;
                        if (c >= L'0' && c <= L'9') {
                            Digit = c - L'0';
                        }
                        else if (c >= L'A' && c <= L'F') {
                            Digit = c - L'A' + 10;
                        }
                        else if (c >= L'a' && c <= L'f') {
                            Digit = c - L'a' + 10;
                        }
                        else {
                            break;
                        }

                        if (Digit >= 16) {
                            break;
                        }

                        DefaultLocaleId = (DefaultLocaleId << 4) | Digit;
                    }
                }
                else {
                    if (KeyValueInformation->Type == REG_DWORD &&
                        KeyValueInformation->DataLength == sizeof( ULONG )) {

                        DefaultLocaleId = *(PLCID)KeyValueInformation->Data;
                    }
                    else {
                        Status = STATUS_UNSUCCESSFUL;
                    }
                }
            }

            ZwClose( Key );
        }
    }
    else {

        Status = ExpValidateLocale( DefaultLocaleId );

        if (NT_SUCCESS(Status)) {

            Status = ZwOpenKey( &Key,
                                GENERIC_WRITE,
                                &ObjectAttributes
                              );

            if (NT_SUCCESS( Status )) {
                if (UserProfile) {
                    n = 8;
                }
                else {
                    n = 4;
                }

                s = &KeyValueBuffer[ n ];
                *s-- = UNICODE_NULL;
                i = (ULONG)DefaultLocaleId;

                while (s >= KeyValueBuffer) {
                    Digit = i & 0x0000000F;
                    if (Digit <= 9) {
                        *s-- = (WCHAR)(Digit + L'0');
                    }
                    else {
                        *s-- = (WCHAR)((Digit - 10) + L'A');
                    }

                    i = i >> 4;
                }

                Status = ZwSetValueKey( Key,
                                        &KeyValueName,
                                        0,
                                        REG_SZ,
                                        KeyValueBuffer,
                                        (n+1) * sizeof( WCHAR )
                                      );
                ZwClose( Key );
            }
        }
    }

    if( CurrentUserKey ) {
        ZwClose( CurrentUserKey );
    }

    if (NT_SUCCESS( Status )) {
        if (UserProfile) {
            MmSetSessionLocaleId (DefaultLocaleId);
        }
        else {
            PsDefaultSystemLocaleId = DefaultLocaleId;
        }
    }

    return Status;
}

NTSTATUS
NtQueryInstallUILanguage (
    __out LANGID *InstallUILanguageId
    )
{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PAGED_CODE();

    Status = STATUS_SUCCESS;
    try {

        //
        // Get previous processor mode and probe output argument if necessary.
        //

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForWriteUshort( (USHORT *)InstallUILanguageId );
            }

        *InstallUILanguageId = PsInstallUILanguageId;
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        }

    return Status;
}

NTSTATUS
NtQueryDefaultUILanguage (
    __out LANGID *DefaultUILanguageId
    )
{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PAGED_CODE();

    Status = STATUS_SUCCESS;
    try {

        //
        // Get previous processor mode and probe output argument if necessary.
        //

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForWriteUshort( (USHORT *)DefaultUILanguageId );
            }

        //
        // Read the UI language from the current security context.
        //
        if (!NT_SUCCESS(ExpGetCurrentUserUILanguage( L"MultiUILanguageId",
                                                     DefaultUILanguageId,
                                                     TRUE))) {
            *DefaultUILanguageId = PsInstallUILanguageId;
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        }

    return Status;
}

NTSTATUS
ExpGetUILanguagePolicy (
    IN HANDLE CurrentUserKey,
    OUT LANGID *PolicyUILanguageId
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyPath, KeyValueName;
    HANDLE Key;
    WCHAR KeyValueBuffer[ 128 ];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG ResultLength;
    ULONG Language;

    PAGED_CODE();

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;
    RtlInitUnicodeString( &KeyValueName, L"MultiUILanguageId" );
    RtlInitUnicodeString( &KeyPath, L"Software\\Policies\\Microsoft\\Control Panel\\Desktop" );

    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyPath,
                                (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                CurrentUserKey,
                                NULL
                              );

    //
    // Check if there is a Policy key
    //
    Status = ZwOpenKey( &Key,
                        GENERIC_READ,
                        &ObjectAttributes
                      );

    if (NT_SUCCESS( Status )) {

        Status = ZwQueryValueKey( Key,
                                  &KeyValueName,
                                  KeyValuePartialInformation,
                                  KeyValueInformation,
                                  sizeof( KeyValueBuffer ),
                                  &ResultLength
                                );

        if (NT_SUCCESS( Status )) {
            if ((KeyValueInformation->DataLength > 2) &&
                (KeyValueInformation->Type == REG_SZ) &&
                ExpIsValidUILanguage((PWSTR) KeyValueInformation->Data)) {

                RtlInitUnicodeString( &KeyValueName, (PWSTR) KeyValueInformation->Data );
                Status = RtlUnicodeStringToInteger( &KeyValueName,
                                                    (ULONG)16,
                                                    &Language
                                                  );
                //
                // Final check to make sure this is an MUI system
                //
                if (NT_SUCCESS( Status )) {
                    *PolicyUILanguageId = (LANGID)Language;
                    }
                }
            else {
                Status = STATUS_UNSUCCESSFUL;
                }
            }
            ZwClose( Key );
        }

    return Status;
}

NTSTATUS
ExpSetCurrentUserUILanguage (
    IN WCHAR *ValueName,
    IN LANGID CurrentUserUILanguage
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyPath, KeyValueName;
    HANDLE CurrentUserKey, Key;
    WCHAR KeyValueBuffer[ 128 ];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    PWSTR s;
    ULONG i, Digit;

    PAGED_CODE();

    if (CurrentUserUILanguage & 0xFFFF0000) {
        return STATUS_INVALID_PARAMETER;
        }

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;
    Status = RtlOpenCurrentUser( KEY_ENUMERATE_SUB_KEYS, &CurrentUserKey );
    if (!NT_SUCCESS( Status )) {
        return Status;
        }

    RtlInitUnicodeString( &KeyValueName, ValueName );
    RtlInitUnicodeString( &KeyPath, L"Control Panel\\Desktop" );
    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyPath,
                                (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE | OBJ_FORCE_ACCESS_CHECK),
                                CurrentUserKey,
                                NULL
                              );


    Status = ExpValidateLocale( MAKELCID( CurrentUserUILanguage, SORT_DEFAULT ) );

    if (NT_SUCCESS(Status)) {

        Status = ZwOpenKey( &Key,
                            GENERIC_WRITE,
                            &ObjectAttributes
                          );
        if (NT_SUCCESS( Status )) {

            s = &KeyValueBuffer[ 8 ];
            *s-- = UNICODE_NULL;
            i = (ULONG)CurrentUserUILanguage;

            while (s >= KeyValueBuffer) {
                Digit = i & 0x0000000F;
                if (Digit <= 9) {
                    *s-- = (WCHAR)(Digit + L'0');
                    }
                else {
                    *s-- = (WCHAR)((Digit - 10) + L'A');
                    }

                i = i >> 4;
                }

            Status = ZwSetValueKey( Key,
                                    &KeyValueName,
                                    0,
                                    REG_SZ,
                                    KeyValueBuffer,
                                    9 * sizeof( WCHAR )
                                  );
            ZwClose( Key );
            }
        }

    ZwClose( CurrentUserKey );

    return Status;
}

NTSTATUS
ExpGetCurrentUserUILanguage (
    IN WCHAR *ValueName,
    OUT LANGID *CurrentUserUILanguageId,
    IN BOOLEAN bCheckGP
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyPath, KeyValueName, UILanguage;
    HANDLE CurrentUserKey, Key;
    WCHAR KeyValueBuffer[ 128 ];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG ResultLength;
    ULONG Digit;

    PAGED_CODE();

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;
    Status = RtlOpenCurrentUser( KEY_ENUMERATE_SUB_KEYS, &CurrentUserKey );
    if (!NT_SUCCESS( Status )) {
        return Status;
        }
    RtlInitUnicodeString( &KeyValueName, ValueName );
    RtlInitUnicodeString( &KeyPath, L"Control Panel\\Desktop" );
    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyPath,
                                (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE | OBJ_FORCE_ACCESS_CHECK),
                                CurrentUserKey,
                                NULL
                              );

    //
    // Let's check if there is a policy installed for the UI language,
    // and if so, let's use it.
    //
    if (!bCheckGP || !NT_SUCCESS( ExpGetUILanguagePolicy( CurrentUserKey, CurrentUserUILanguageId ))) {
        Status = ZwOpenKey( &Key,
                            GENERIC_READ,
                            &ObjectAttributes
                          );
        if (NT_SUCCESS( Status )) {
            Status = ZwQueryValueKey( Key,
                                      &KeyValueName,
                                      KeyValuePartialInformation,
                                      KeyValueInformation,
                                      sizeof( KeyValueBuffer ),
                                      &ResultLength
                                    );
            if (NT_SUCCESS( Status )) {

                if (KeyValueInformation->Type == REG_SZ &&
                    ExpIsValidUILanguage((PWSTR) KeyValueInformation->Data)) {

                    RtlInitUnicodeString( &UILanguage, (PWSTR) KeyValueInformation->Data);
                    Status = RtlUnicodeStringToInteger( &UILanguage,
                                                        (ULONG) 16,
                                                        &Digit
                                                      );
                    if (NT_SUCCESS( Status )) {
                        *CurrentUserUILanguageId = (LANGID) Digit;
                        }
                    }
                else {
                    Status = STATUS_UNSUCCESSFUL;
                    }
                }
                ZwClose( Key );
            }
        }

    ZwClose( CurrentUserKey );

    return Status;
}

NTSTATUS
NtSetDefaultUILanguage (
    __in LANGID DefaultUILanguageId
    )
{
    NTSTATUS Status;
    LANGID LangId;

    //
    //  if this is called during user logon, then we need to update the user's registry.
    //
    if (DefaultUILanguageId == 0) {
          Status = ExpGetCurrentUserUILanguage( L"MUILanguagePending" ,
                                                &LangId,
                                                FALSE
                                                );
          if (NT_SUCCESS( Status )) {
            Status = ExpSetCurrentUserUILanguage( L"MultiUILanguageId" ,
                                                  LangId
                                                );
            }
          return Status;
        }

    return ExpSetCurrentUserUILanguage( L"MUILanguagePending", DefaultUILanguageId );
}

NTSTATUS
ExpValidateLocale (
    IN LCID LocaleId
    )
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER, ReturnStatus;
    UNICODE_STRING LocaleName, KeyValueName;
    UNICODE_STRING NlsLocaleKeyPath, NlsSortKeyPath, NlsLangGroupKeyPath;
    WCHAR LocaleNameBuffer[ 32 ];
    WCHAR KeyValueNameBuffer[ 32 ];
    WCHAR KeyValueBuffer[ 128 ];
    WCHAR *Ptr;
    HANDLE LocaleKey, SortKey, LangGroupKey;
    OBJECT_ATTRIBUTES NlsLocaleObjA, NlsSortObjA, NlsLangGroupObjA;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG i, ResultLength;

    //
    //  Convert the LCID to the form %08x (e.g. 00000409)
    //
    LocaleName.Length = sizeof( LocaleNameBuffer ) / sizeof( WCHAR );
    LocaleName.MaximumLength = LocaleName.Length;
    LocaleName.Buffer = LocaleNameBuffer;

    //
    //  Convert LCID to a string
    //
    ReturnStatus = RtlIntegerToUnicodeString( LocaleId, 16, &LocaleName );
    if (!NT_SUCCESS(ReturnStatus))
        goto Failed1;

    Ptr = KeyValueNameBuffer;
    for (i = ((LocaleName.Length)/sizeof(WCHAR));
         i < 8;
         i++, Ptr++) {
        *Ptr = L'0';
        }
    *Ptr = UNICODE_NULL;

    RtlInitUnicodeString(&KeyValueName, KeyValueNameBuffer);
    KeyValueName.MaximumLength = sizeof( KeyValueNameBuffer ) / sizeof( WCHAR );
    RtlAppendUnicodeToString(&KeyValueName, LocaleName.Buffer);


    //
    // Open Registry Keys : Locale, Sort and LanguageGroup
    //
    RtlInitUnicodeString(&NlsLocaleKeyPath,
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\Locale");

    InitializeObjectAttributes( &NlsLocaleObjA,
                                &NlsLocaleKeyPath,
                                (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                NULL,
                                NULL
                              );

    ReturnStatus = ZwOpenKey( &LocaleKey,
                              GENERIC_READ,
                              &NlsLocaleObjA
                            );
    if (!NT_SUCCESS(ReturnStatus))
         goto Failed1;

    RtlInitUnicodeString(&NlsSortKeyPath,
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\Locale\\Alternate Sorts");

    InitializeObjectAttributes( &NlsSortObjA,
                                &NlsSortKeyPath,
                                (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                NULL,
                                NULL
                              );

    ReturnStatus = ZwOpenKey( &SortKey,
                              GENERIC_READ,
                              &NlsSortObjA
                            );
    if (!NT_SUCCESS(ReturnStatus))
         goto Failed2;

    RtlInitUnicodeString(&NlsLangGroupKeyPath,
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\Language Groups");

    InitializeObjectAttributes( &NlsLangGroupObjA,
                                &NlsLangGroupKeyPath,
                                (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                NULL,
                                NULL
                              );


    ReturnStatus = ZwOpenKey( &LangGroupKey,
                              GENERIC_READ,
                              &NlsLangGroupObjA
                            );
    if (!NT_SUCCESS(ReturnStatus))
         goto Failed3;

    //
    // Validate Locale : Lookup the Locale's Language group, and make sure it is there.
    //
    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION) KeyValueBuffer;
    ReturnStatus = ZwQueryValueKey( LocaleKey,
                                    &KeyValueName,
                                    KeyValuePartialInformation,
                                    KeyValueInformation,
                                    sizeof( KeyValueBuffer ),
                                    &ResultLength
                                  );

    if (!NT_SUCCESS(ReturnStatus)) {
        ReturnStatus = ZwQueryValueKey( SortKey,
                                        &KeyValueName,
                                        KeyValuePartialInformation,
                                        KeyValueInformation,
                                        sizeof( KeyValueBuffer ),
                                        &ResultLength
                                      );
        }

    if ((NT_SUCCESS(ReturnStatus)) &&
        (KeyValueInformation->DataLength > 2)
       ) {

        RtlInitUnicodeString( &KeyValueName, (PWSTR) KeyValueInformation->Data );

        ReturnStatus = ZwQueryValueKey( LangGroupKey,
                                        &KeyValueName,
                                        KeyValuePartialInformation,
                                        KeyValueInformation,
                                        sizeof( KeyValueBuffer ),
                                        &ResultLength
                                      );
        if ((NT_SUCCESS(ReturnStatus)) &&
            (KeyValueInformation->Type == REG_SZ) &&
            (KeyValueInformation->DataLength > 2)
           ) {
            Ptr = (PWSTR) KeyValueInformation->Data;
            if (Ptr[0] == L'1' && Ptr[1] == UNICODE_NULL) {
                Status = STATUS_SUCCESS;
                }
            }
        }

    //
    // Close opened keys
    //

    ZwClose( LangGroupKey );

Failed3:
    ZwClose( SortKey );

Failed2:
    ZwClose( LocaleKey );

Failed1:

    //
    // If an error happens, let's record it.
    //
    if (!NT_SUCCESS(ReturnStatus)) {
        Status = ReturnStatus;
        }

    return Status;
}

NTSTATUS
ExpQueryNumaProcessorMap (
    OUT PVOID  SystemInformation,
    IN  ULONG  SystemInformationLength,
    OUT PULONG ReturnedLength
    )
{
    PSYSTEM_NUMA_INFORMATION Map;
    ULONG Length;
    ULONG ReturnCount;
#if !defined(NT_UP)
    ULONG i;
#endif

    Map = (PSYSTEM_NUMA_INFORMATION)SystemInformation;

    //
    // Must be able to return at least the number of nodes.
    //

    if (SystemInformationLength < sizeof(Map->HighestNodeNumber)) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    Map->HighestNodeNumber = KeNumberNodes - 1;

    //
    // Return as many node masks as possible in the SystemInformation
    // buffer.
    //

    Length = FIELD_OFFSET(SYSTEM_NUMA_INFORMATION,
                          ActiveProcessorsAffinityMask);

    ReturnCount = (SystemInformationLength - Length) /
                  sizeof(Map->ActiveProcessorsAffinityMask[0]);

    if (ReturnCount > KeNumberNodes) {
        ReturnCount = KeNumberNodes;
    }

    if ((Length > SystemInformationLength) ||
        (ReturnCount == 0)) {
        *ReturnedLength = sizeof(Map->HighestNodeNumber);
        return STATUS_SUCCESS;
    }

    *ReturnedLength = FIELD_OFFSET(SYSTEM_NUMA_INFORMATION,
                                   ActiveProcessorsAffinityMask[ReturnCount]);

#if !defined(NT_UP)

    for (i = 0; i < ReturnCount; i++) {
        Map->ActiveProcessorsAffinityMask[i] = KeNodeBlock[i]->ProcessorMask;
    }

#else

    if (ReturnCount) {
        Map->ActiveProcessorsAffinityMask[0] = 1;
    }

#endif

    return STATUS_SUCCESS;
}

NTSTATUS
ExpQueryNumaAvailableMemory (
    OUT PVOID  SystemInformation,
    IN  ULONG  SystemInformationLength,
    OUT PULONG ReturnedLength
    )
{
    PSYSTEM_NUMA_INFORMATION Map;
    ULONG Length;
    ULONG ReturnCount;

    Map = (PSYSTEM_NUMA_INFORMATION)SystemInformation;

    //
    // Must be able to return at least the number of nodes.
    //

    if (SystemInformationLength < sizeof(Map->HighestNodeNumber)) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    Map->HighestNodeNumber = KeNumberNodes - 1;

    //
    // Return as many node masks as possible in the SystemInformation
    // buffer.
    //

    Length = FIELD_OFFSET(SYSTEM_NUMA_INFORMATION,
                          AvailableMemory);

    ReturnCount = (SystemInformationLength - Length) /
                  sizeof(Map->AvailableMemory[0]);

    if (ReturnCount > KeNumberNodes) {
        ReturnCount = KeNumberNodes;
    }

    if ((Length > SystemInformationLength) ||
        (ReturnCount == 0)) {
        *ReturnedLength = sizeof(Map->HighestNodeNumber);
        return STATUS_SUCCESS;
    }

    *ReturnedLength = FIELD_OFFSET(SYSTEM_NUMA_INFORMATION,
                                   AvailableMemory[ReturnCount]);

    //
    // Return the approximate number of free bytes at this time.
    // (It's approximate because no lock is taken and with respect
    // to any user mode application its only a sample.
    //

#if !defined(NT_UP)

    if (KeNumberNodes > 1) {

        ULONG i;

        for (i = 0; i < ReturnCount; i++) {
            Map->AvailableMemory[i] =
                ((ULONGLONG)KeNodeBlock[i]->FreeCount[ZeroedPageList] +
                 (ULONGLONG)KeNodeBlock[i]->FreeCount[FreePageList])
                    << PAGE_SHIFT;
        }
    } else

#endif

    if (ReturnCount) {
        Map->AvailableMemory[0] = ((ULONGLONG)MmAvailablePages) << PAGE_SHIFT;
    }


    return STATUS_SUCCESS;
}

NTSTATUS
ExpGetSystemBasicInformation (
    OUT PSYSTEM_BASIC_INFORMATION BasicInfo
    )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    try {

        BasicInfo->NumberOfProcessors = KeNumberProcessors;
        BasicInfo->ActiveProcessorsAffinityMask = (ULONG_PTR)KeActiveProcessors;
        BasicInfo->Reserved = 0;
        BasicInfo->TimerResolution = KeMaximumIncrement;
        BasicInfo->NumberOfPhysicalPages = MmNumberOfPhysicalPages;
        BasicInfo->LowestPhysicalPageNumber = (SYSINF_PAGE_COUNT)MmLowestPhysicalPage;
        BasicInfo->HighestPhysicalPageNumber = (SYSINF_PAGE_COUNT)MmHighestPhysicalPage;
        BasicInfo->PageSize = PAGE_SIZE;
        BasicInfo->AllocationGranularity = MM_ALLOCATION_GRANULARITY;
        BasicInfo->MinimumUserModeAddress = (ULONG_PTR)MM_LOWEST_USER_ADDRESS;
        BasicInfo->MaximumUserModeAddress = (ULONG_PTR)MM_HIGHEST_USER_ADDRESS;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        NtStatus = GetExceptionCode();
    }

    return NtStatus;
}

NTSTATUS
ExpGetSystemProcessorInformation (
    OUT PSYSTEM_PROCESSOR_INFORMATION ProcessorInformation
    )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    try {

        ProcessorInformation->ProcessorArchitecture = KeProcessorArchitecture;
        ProcessorInformation->ProcessorLevel = KeProcessorLevel;
        ProcessorInformation->ProcessorRevision = KeProcessorRevision;
        ProcessorInformation->Reserved = 0;
        ProcessorInformation->ProcessorFeatureBits = KeFeatureBits;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        NtStatus = GetExceptionCode();
    }

    return NtStatus;
}

#if defined(_WIN64)

NTSTATUS
ExpGetSystemEmulationBasicInformation (
    OUT PSYSTEM_BASIC_INFORMATION BasicInfo
    )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    try {

        BasicInfo->NumberOfProcessors =  min(32, KeNumberProcessors);
        BasicInfo->ActiveProcessorsAffinityMask = (ULONG_PTR)
            ((KeActiveProcessors & 0xFFFFFFFF) | ((KeActiveProcessors & (((ULONG_PTR)0xFFFFFFFF) << 32) ) >> 32));
        BasicInfo->Reserved = 0;
        BasicInfo->TimerResolution = KeMaximumIncrement;
        BasicInfo->NumberOfPhysicalPages = (MmNumberOfPhysicalPages * (PAGE_SIZE >> PAGE_SHIFT_X86NT));
        BasicInfo->LowestPhysicalPageNumber = (SYSINF_PAGE_COUNT)MmLowestPhysicalPage;
        BasicInfo->HighestPhysicalPageNumber = (SYSINF_PAGE_COUNT)MmHighestPhysicalPage;
        BasicInfo->PageSize = PAGE_SIZE_X86NT;
        BasicInfo->AllocationGranularity = MM_ALLOCATION_GRANULARITY;
        BasicInfo->MinimumUserModeAddress = 0x00000000000010000UI64;
        
        //
        // NOTE: MmGetMaxWowAddress return the highest usermode address boundary,
        // thus we are subtracting one to get the maximum accessible usermode address        
        //

        BasicInfo->MaximumUserModeAddress = ((ULONG_PTR)MmGetMaxWowAddress () - 1);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        NtStatus = GetExceptionCode();
    }

    return NtStatus;
}

#endif

NTSTATUS
NtQuerySystemInformation (
    __in SYSTEM_INFORMATION_CLASS SystemInformationClass,
    __out_bcount_opt(SystemInformationLength) PVOID SystemInformation,
    __in ULONG SystemInformationLength,
    __out_opt PULONG ReturnLength
    )

/*++

Routine Description:

    This function queries information about the system.

Arguments:

    SystemInformationClass - The system information class about which
        to retrieve information.

    SystemInformation - A pointer to a buffer which receives the specified
        information.  The format and content of the buffer depend on the
        specified system information class.

        SystemInformation Format by Information Class:

        SystemBasicInformation - Data type is SYSTEM_BASIC_INFORMATION

            SYSTEM_BASIC_INFORMATION Structure

                ULONG Reserved - Always zero.

                ULONG TimerResolutionInMicroSeconds - The resolution of
                    the hardware time.  All time values in NT are
                    specified as 64-bit LARGE_INTEGER values in units of
                    100 nanoseconds.  This field allows an application to
                    understand how many of the low order bits of a system
                    time value are insignificant.

                ULONG PageSize - The physical page size for virtual memory
                    objects.  Physical memory is committed in PageSize
                    chunks.

                ULONG AllocationGranularity - The logical page size for
                    virtual memory objects.  Allocating 1 byte of virtual
                    memory will actually allocate AllocationGranularity
                    bytes of virtual memory.  Storing into that byte will
                    commit the first physical page of the virtual memory.

                ULONG MinimumUserModeAddress - The smallest valid user mode
                    address.  The first AllocationGranularity bytes of
                    the virtual address space are reserved.  This forces
                    access violations for code the dereferences a zero
                    pointer.

                ULONG MaximumUserModeAddress -  The largest valid user mode
                    address.  The next AllocationGranularity bytes of
                    the virtual address space are reserved.  This allows
                    system service routines to validate user mode pointer
                    parameters quickly.

                KAFFINITY ActiveProcessorsAffinityMask - The affinity mask
                    for the current hardware configuration.

                CCHAR NumberOfProcessors - The number of processors
                    in the current hardware configuration.

        SystemProcessorInformation - Data type is SYSTEM_PROCESSOR_INFORMATION

            SYSTEM_PROCESSOR_INFORMATION Structure

                USHORT ProcessorArchitecture - The processor architecture:
                    PROCESSOR_ARCHITECTURE_INTEL
                    PROCESSOR_ARCHITECTURE_IA64
                    PROCESSOR_ARCHITECTURE_MIPS
                    PROCESSOR_ARCHITECTURE_ALPHA
                    PROCESSOR_ARCHITECTURE_PPC

                USHORT ProcessorLevel - architecture dependent processor level.
                    This is the least common denominator for an MP system:

                    For PROCESSOR_ARCHITECTURE_INTEL:
                        3 - 386
                        4 - 486
                        5 - 586 or Pentium

                    For PROCESSOR_ARCHITECTURE_IA64:
                        7  - Itanium
                        31 - Itanium 2

                    For PROCESSOR_ARCHITECTURE_MIPS:
                        00xx - where xx is 8-bit implementation number (bits 8-15 of
                            PRId register.
                        0004 - R4000

                    For PROCESSOR_ARCHITECTURE_ALPHA:
                        xxxx - where xxxx is 16-bit processor version number (low
                            order 16 bits of processor version number from firmware)

                        21064 - 21064
                        21066 - 21066
                        21164 - 21164

                    For PROCESSOR_ARCHITECTURE_PPC:
                        xxxx - where xxxx is 16-bit processor version number (high
                            order 16 bits of Processor Version Register).
                        1 - 601
                        3 - 603
                        4 - 604
                        6 - 603+
                        9 - 604+
                        20 - 620

                USHORT ProcessorRevision - architecture dependent processor revision.
                    This is the least common denominator for an MP system:

                    For PROCESSOR_ARCHITECTURE_INTEL:
                        For Old Intel 386 or 486:
                            FFxx - where xx is displayed as a hexadecimal CPU stepping
                            (e.g. FFD0 is D0 stepping)

                        For Intel Pentium or Cyrix/NexGen 486
                            xxyy - where xx is model number and yy is stepping, so
                            0201 is Model 2, Stepping 1

                    For PROCESSOR_ARCHITECTURE_IA64:
                        xxyy - where xx is model number and yy is stepping, so
                            0201 is Model 2, Stepping 1

                    For PROCESSOR_ARCHITECTURE_MIPS:
                        00xx is 8-bit revision number of processor (low order 8 bits
                            of PRId Register

                    For PROCESSOR_ARCHITECTURE_ALPHA:
                        xxyy - where xxyy is 16-bit processor revision number (low
                            order 16 bits of processor revision number from firmware).
                            Displayed as Model 'A'+xx, Pass yy

                    For PROCESSOR_ARCHITECTURE_PPC:
                        xxyy - where xxyy is 16-bit processor revision number (low
                            order 16 bits of Processor Version Register).  Displayed
                            as a fixed point number xx.yy

                USHORT Reserved - Always zero.

                ULONG ProcessorFeatureBits - architecture dependent processor feature bits.
                    This is the least common denominator for an MP system.

        SystemPerformanceInformation - Data type is SYSTEM_PERFORMANCE_INFORMATION

            SYSTEM_PERFORMANCE_INFORMATION Structure

                LARGE_INTEGER IdleProcessTime - Returns the kernel time of the idle
                    process.

            LARGE_INTEGER IoReadTransferCount;
            LARGE_INTEGER IoWriteTransferCount;
            LARGE_INTEGER IoOtherTransferCount;
            LARGE_INTEGER KernelTime;
            LARGE_INTEGER UserTime;
            ULONG IoReadOperationCount;
            ULONG IoWriteOperationCount;
            ULONG IoOtherOperationCount;
            ULONG AvailablePages;
            ULONG CommittedPages;
            ULONG PageFaultCount;
            ULONG CopyOnWriteCount;
            ULONG TransitionCount;
            ULONG CacheTransitionCount;
            ULONG DemandZeroCount;
            ULONG PageReadCount;
            ULONG PageReadIoCount;
            ULONG CacheReadCount;
            ULONG CacheIoCount;
            ULONG DirtyPagesWriteCount;
            ULONG DirtyWriteIoCount;
            ULONG MappedPagesWriteCount;
            ULONG MappedWriteIoCount;
            ULONG PagedPoolPages;
            ULONG NonPagedPoolPages;
            ULONG PagedPoolAllocs;
            ULONG PagedPoolFrees;
            ULONG NonPagedPoolAllocs;
            ULONG NonPagedPoolFrees;
            ULONG LpcThreadsWaitingInReceive;
            ULONG LpcThreadsWaitingForReply;

        SystemProcessInformation - Data type is SYSTEM_PROCESS_INFORMATION

            SYSTEM_PROCESS_INFORMATION Structure

        SystemDockInformation - Data type is SYSTEM_DOCK_INFORMATION

             SYSTEM_DOCK_INFORMATION Structure

                 SYSTEM_DOCKED_STATE DockState - Ordinal specifying the current docking state. Possible values:
                     SystemDockStateUnknown - The docking state of the system could not be determined.
                     SystemUndocked - The system is undocked.
                     SystemDocked - The system is docked.

                 ULONG DockIdLength - Specifies the length in characters of the Dock ID string
                                      (not including terminating NULL).

                 ULONG SerialNumberOffset - Specifies the character offset of the Serial Number within
                                            the DockId buffer.

                 ULONG SerialNumberLength - Specifies the length in characters of the Serial Number
                                            string (not including terminating NULL).

                 WCHAR DockId - Character buffer containing two null-terminated strings.  The first
                                string is a character representation of the dock ID number, starting
                                at the beginning of the buffer.  The second string is a character
                                representation of the machine's serial number, starting at character
                                offset SerialNumberOffset in the buffer.


        SystemPowerSettings - Data type is SYSTEM_POWER_SETTINGS
            SYSTEM_POWER_INFORMATION Structure
                BOOLEAN SystemSuspendSupported - Supplies a BOOLEAN as to
                    whether the system suspend is enabled or not.
                BOOLEAN SystemHibernateSupported - Supplies a BOOLEAN as to
                    whether the system hibernate is enabled or not.
                BOOLEAN ResumeTimerSupportsSuspend - Supplies a BOOLEAN as to
                    whether the resuming from an external programmed timer
                    from within a system suspend is enabled or not.
                BOOLEAN ResumeTimerSupportsHibernate - Supplies a BOOLEAN as to
                    whether or resuming from an external programmed timer
                    from within a system hibernate is enabled or not.
                BOOLEAN LidSupported - Supplies a BOOLEAN as to whether or not
                    the suspending and resuming by Lid are enabled or not.
                BOOLEAN TurboSettingSupported - Supplies a BOOLEAN as to whether
                    or not the system supports a turbo mode setting.
                BOOLEAN TurboMode - Supplies a BOOLEAN as to whether or not
                    the system is in turbo mode.
                BOOLEAN SystemAcOrDc - Supplies a BOOLEAN as to whether or not
                    the system is in AC mode.
                BOOLEAN DisablePowerDown - If TRUE, signifies that all requests to
                    PoRequestPowerChange for a SET_POWER-PowerDown irp are to
                    be ignored.
                LARGE_INTEGER SpindownDrives - If non-zero, signifies to the
                    cache manager (or the IO subsystem) to optimize drive
                    accesses based upon power saves, are that drives are to
                    be spun down as appropriate. The value represents to user's
                    requested disk spin down timeout.

        SystemProcessorSpeedInformation - Data type is SYSTEM_PROCESSOR_SPEED_INFORMATION
            SYSTEM_PROCESSOR_SPEED_INFORMATION Structure (same as HalProcessorSpeedInformation)
                ULONG MaximumProcessorSpeed - The maximum hertz the processor is
                    capable of. This information is used by the UI to draw the
                    appropriate scale. This field is read-only and cannot be
                    set.
                ULONG CurrentAvailableSpeed - The hertz for which the processor
                    runs at when not idle. This field is read-only and cannot
                    be set.
                ULONG ConfiguredSpeedLimit - The hertz for which the processor
                    is limited to due to the current configuration.
                UCHAR PowerState
                    0 - Normal
                    1 - The processor speed is being limited due to available
                    power restrictions. This field id read-only by the system.
                UCHAR ThermalState
                    0 - Normal
                    1 - The processors speed is being limited due to thermal
                    restrictions. This field is read-only by the system.
                UCHAR TurboState
                    0 - Normal
                    1 - The processors speed is being limited by the fact that
                    the system turbo mode is currently disabled which is
                    requested to obtain more processor speed.

    SystemInformationLength - Specifies the length in bytes of the system
        information buffer.

    ReturnLength - An optional pointer which, if specified, receives the
        number of bytes placed in the system information buffer.

Return Value:

    Returns one of the following status codes:

        STATUS_SUCCESS - normal, successful completion.

        STATUS_INVALID_INFO_CLASS - The SystemInformationClass parameter
            did not specify a valid value.

        STATUS_INFO_LENGTH_MISMATCH - The value of the SystemInformationLength
            parameter did not match the length required for the information
            class requested by the SystemInformationClass parameter.

        STATUS_ACCESS_VIOLATION - Either the SystemInformation buffer pointer
            or the ReturnLength pointer value specified an invalid address.

        STATUS_WORKING_SET_QUOTA - The process does not have sufficient
            working set to lock the specified output structure in memory.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources exist
            for this request to complete.

--*/

{

    KPROCESSOR_MODE PreviousMode;
    SYSTEM_TIMEOFDAY_INFORMATION LocalTimeOfDayInfo;
    SYSTEM_PERFORMANCE_INFORMATION LocalPerformanceInfo;
    PSYSTEM_PERFORMANCE_INFORMATION PerformanceInfo;
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION ProcessorPerformanceInfo;
    PSYSTEM_CALL_COUNT_INFORMATION CallCountInformation;
    PSYSTEM_DEVICE_INFORMATION DeviceInformation;
    PCONFIGURATION_INFORMATION ConfigInfo;
    PSYSTEM_EXCEPTION_INFORMATION ExceptionInformation;
    PSYSTEM_FILECACHE_INFORMATION FileCache;
    PSYSTEM_QUERY_TIME_ADJUST_INFORMATION TimeAdjustmentInformation;
    PSYSTEM_KERNEL_DEBUGGER_INFORMATION KernelDebuggerInformation;
    PSYSTEM_CONTEXT_SWITCH_INFORMATION ContextSwitchInformation;
    PSYSTEM_INTERRUPT_INFORMATION InterruptInformation;
    PSYSTEM_SESSION_PROCESS_INFORMATION SessionProcessInformation;
    PVOID ProcessInformation;
    ULONG ProcessInformationLength;
    PSYSTEM_SESSION_POOLTAG_INFORMATION SessionPoolTagInformation;
    PSYSTEM_SESSION_MAPPED_VIEW_INFORMATION SessionMappedViewInformation;
    ULONG SessionPoolTagInformationLength;

    NTSTATUS Status;
    PKPRCB Prcb;
    ULONG Length = 0;
    ULONG i;
    ULONG ContextSwitches;
    PULONG TableLimit, TableCounts;
    PKSERVICE_TABLE_DESCRIPTOR Table;
    ULONG SessionId;
    ULONG Alignment;

    PAGED_CODE();

    //
    // Assume successful completion.
    //

    Status = STATUS_SUCCESS;
    try {

        //
        // Get previous processor mode and probe output argument if necessary.
        //

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {

            Alignment = sizeof(ULONG);

            if (SystemInformationClass == SystemKernelDebuggerInformation) {
                Alignment = sizeof(BOOLEAN);
            } else if (SystemInformationClass == SystemLocksInformation) {
                Alignment = sizeof(PVOID);
            }

            ProbeForWrite(SystemInformation,
                          SystemInformationLength,
                          Alignment);

            if (ARGUMENT_PRESENT(ReturnLength)) {
                ProbeForWriteUlong(ReturnLength);
            }
        }

        if (ARGUMENT_PRESENT(ReturnLength)) {
            *ReturnLength = 0;
        }

        switch (SystemInformationClass) {

        case SystemBasicInformation:

            if (SystemInformationLength != sizeof( SYSTEM_BASIC_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExpGetSystemBasicInformation ((PSYSTEM_BASIC_INFORMATION)SystemInformation);

            if (NT_SUCCESS (Status) && ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof( SYSTEM_BASIC_INFORMATION );
            }
            break;

        case SystemEmulationBasicInformation:

            if (SystemInformationLength != sizeof( SYSTEM_BASIC_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

#if defined(_WIN64)
            Status = ExpGetSystemEmulationBasicInformation ((PSYSTEM_BASIC_INFORMATION)SystemInformation);
#else
            Status = ExpGetSystemBasicInformation ((PSYSTEM_BASIC_INFORMATION)SystemInformation);
#endif

            if (NT_SUCCESS (Status) && ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof( SYSTEM_BASIC_INFORMATION );
            }
            break;

        case SystemProcessorInformation:
            if (SystemInformationLength < sizeof( SYSTEM_PROCESSOR_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExpGetSystemProcessorInformation ((PSYSTEM_PROCESSOR_INFORMATION)SystemInformation);

            if (NT_SUCCESS (Status) && ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof( SYSTEM_PROCESSOR_INFORMATION );
            }

            break;

        case SystemEmulationProcessorInformation:

            if (SystemInformationLength < sizeof( SYSTEM_PROCESSOR_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

#if defined(_WIN64)
            Status = ExpGetSystemEmulationProcessorInformation ((PSYSTEM_PROCESSOR_INFORMATION)SystemInformation);
#else
            Status = ExpGetSystemProcessorInformation ((PSYSTEM_PROCESSOR_INFORMATION)SystemInformation);
#endif

            if (NT_SUCCESS (Status) && ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof( SYSTEM_PROCESSOR_INFORMATION );
            }

            break;

        case SystemPerformanceInformation:
            if (SystemInformationLength < sizeof( SYSTEM_PERFORMANCE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            PerformanceInfo = (PSYSTEM_PERFORMANCE_INFORMATION)SystemInformation;

            //
            // Io information.
            //
            // These counters are kept on a per processor basis and must
            // be totaled.
            //

            {

                ULONG OtherOperationCount;
                LARGE_INTEGER OtherTransferCount;
                ULONG ReadOperationCount;
                LARGE_INTEGER ReadTransferCount;
                ULONG WriteOperationCount;
                LARGE_INTEGER WriteTransferCount;

                OtherOperationCount = IoOtherOperationCount;
                OtherTransferCount.QuadPart = IoOtherTransferCount.QuadPart;
                ReadOperationCount = IoReadOperationCount;
                ReadTransferCount.QuadPart = IoReadTransferCount.QuadPart;
                WriteOperationCount = IoWriteOperationCount;
                WriteTransferCount.QuadPart = IoWriteTransferCount.QuadPart;
                for (i = 0; i < (ULONG)KeNumberProcessors; i += 1) {
                    Prcb = KiProcessorBlock[i];
                    if (Prcb != NULL) {
                        OtherOperationCount += Prcb->IoOtherOperationCount;
                        OtherTransferCount.QuadPart += Prcb->IoOtherTransferCount.QuadPart;
                        ReadOperationCount += Prcb->IoReadOperationCount;
                        ReadTransferCount.QuadPart += Prcb->IoReadTransferCount.QuadPart;
                        WriteOperationCount += Prcb->IoWriteOperationCount;
                        WriteTransferCount.QuadPart += Prcb->IoWriteTransferCount.QuadPart;
                    }
                }

                LocalPerformanceInfo.IoReadTransferCount = ReadTransferCount;
                LocalPerformanceInfo.IoWriteTransferCount = WriteTransferCount;
                LocalPerformanceInfo.IoOtherTransferCount = OtherTransferCount;
                LocalPerformanceInfo.IoReadOperationCount = ReadOperationCount;
                LocalPerformanceInfo.IoWriteOperationCount = WriteOperationCount;
                LocalPerformanceInfo.IoOtherOperationCount = OtherOperationCount;
            }

            //
            // Ke information.
            //
            // These counters are kept on a per processor basis and must
            // be totaled.
            //

            {
                ULONG FirstLevelTbFills = 0;
                ULONG SecondLevelTbFills = 0;
                ULONG SystemCalls = 0;

                ContextSwitches = 0;
                for (i = 0; i < (ULONG)KeNumberProcessors; i += 1) {
                    Prcb = KiProcessorBlock[i];
                    if (Prcb != NULL) {
                        ContextSwitches += KeGetContextSwitches(Prcb);
                        FirstLevelTbFills += Prcb->KeFirstLevelTbFills;
                        SecondLevelTbFills += Prcb->KeSecondLevelTbFills;
                        SystemCalls += Prcb->KeSystemCalls;
                    }
                }

                LocalPerformanceInfo.ContextSwitches = ContextSwitches;
                LocalPerformanceInfo.FirstLevelTbFills = FirstLevelTbFills;
                LocalPerformanceInfo.SecondLevelTbFills = SecondLevelTbFills;
                LocalPerformanceInfo.SystemCalls = SystemCalls;
            }

            //
            // Mm information.
            //
            // some of these counters are kept on a per processor basis and
            // must be totaled.
            //

            LocalPerformanceInfo.AvailablePages = (ULONG)MmAvailablePages;
            LocalPerformanceInfo.CommittedPages = (SYSINF_PAGE_COUNT)MmTotalCommittedPages;
            LocalPerformanceInfo.CommitLimit = (SYSINF_PAGE_COUNT)MmTotalCommitLimit;
            LocalPerformanceInfo.PeakCommitment = (SYSINF_PAGE_COUNT)MmPeakCommitment;

            {

                ULONG PageFaultCount = 0;
                ULONG CopyOnWriteCount = 0;
                ULONG TransitionCount = 0;
                ULONG CacheTransitionCount = 0;
                ULONG DemandZeroCount = 0;
                ULONG PageReadCount = 0;
                ULONG PageReadIoCount = 0;
                ULONG CacheReadCount = 0;
                ULONG CacheIoCount = 0;
                ULONG DirtyPagesWriteCount = 0;
                ULONG DirtyWriteIoCount = 0;
                ULONG MappedPagesWriteCount = 0;
                ULONG MappedWriteIoCount = 0;

                for (i = 0; i < (ULONG)KeNumberProcessors; i += 1) {
                    Prcb = KiProcessorBlock[i];
                    if (Prcb != NULL) {
                        PageFaultCount += Prcb->MmPageFaultCount;
                        CopyOnWriteCount += Prcb->MmCopyOnWriteCount;
                        TransitionCount += Prcb->MmTransitionCount;
                        CacheTransitionCount += Prcb->MmCacheTransitionCount;
                        DemandZeroCount += Prcb->MmDemandZeroCount;
                        PageReadCount += Prcb->MmPageReadCount;
                        PageReadIoCount += Prcb->MmPageReadIoCount;
                        CacheReadCount += Prcb->MmCacheReadCount;
                        CacheIoCount += Prcb->MmCacheIoCount;
                        DirtyPagesWriteCount += Prcb->MmDirtyPagesWriteCount;
                        DirtyWriteIoCount += Prcb->MmDirtyWriteIoCount;
                        MappedPagesWriteCount += Prcb->MmMappedPagesWriteCount;
                        MappedWriteIoCount += Prcb->MmMappedWriteIoCount;
                    }
                }

                LocalPerformanceInfo.PageFaultCount = PageFaultCount;
                LocalPerformanceInfo.CopyOnWriteCount = CopyOnWriteCount;
                LocalPerformanceInfo.TransitionCount = TransitionCount;
                LocalPerformanceInfo.CacheTransitionCount = CacheTransitionCount;
                LocalPerformanceInfo.DemandZeroCount = DemandZeroCount;
                LocalPerformanceInfo.PageReadCount = PageReadCount;
                LocalPerformanceInfo.PageReadIoCount = PageReadIoCount;
                LocalPerformanceInfo.CacheReadCount = CacheReadCount;
                LocalPerformanceInfo.CacheIoCount = CacheIoCount;
                LocalPerformanceInfo.DirtyPagesWriteCount = DirtyPagesWriteCount;
                LocalPerformanceInfo.DirtyWriteIoCount = DirtyWriteIoCount;
                LocalPerformanceInfo.MappedPagesWriteCount = MappedPagesWriteCount;
                LocalPerformanceInfo.MappedWriteIoCount = MappedWriteIoCount;
            }

            LocalPerformanceInfo.FreeSystemPtes = MmGetNumberOfFreeSystemPtes ();
            LocalPerformanceInfo.ResidentSystemCodePage = MmSystemCodePage;
            LocalPerformanceInfo.ResidentSystemCachePage = MmSystemCachePage;
            LocalPerformanceInfo.ResidentPagedPoolPage = MmPagedPoolPage;
            LocalPerformanceInfo.ResidentSystemDriverPage = MmSystemDriverPage;
            LocalPerformanceInfo.TotalSystemCodePages = MmTotalSystemCodePages;
            LocalPerformanceInfo.TotalSystemDriverPages = MmTotalSystemDriverPages;
            LocalPerformanceInfo.AvailablePagedPoolPages = (ULONG)MmAvailablePoolInPages (PagedPool);

            //
            // Idle process information.
            //

            {
                ULONG TotalKernel;
                ULONG TotalUser;

                TotalKernel = KeQueryRuntimeProcess(&PsIdleProcess->Pcb,
                                                    &TotalUser);

                LocalPerformanceInfo.IdleProcessTime.QuadPart =
                                UInt32x32To64(TotalKernel, KeMaximumIncrement);
            }

            //
            // Pool information.
            //

            LocalPerformanceInfo.PagedPoolPages = 0;
            LocalPerformanceInfo.NonPagedPoolPages = 0;
            LocalPerformanceInfo.PagedPoolAllocs = 0;
            LocalPerformanceInfo.PagedPoolFrees = 0;
            LocalPerformanceInfo.PagedPoolLookasideHits = 0;
            LocalPerformanceInfo.NonPagedPoolAllocs = 0;
            LocalPerformanceInfo.NonPagedPoolFrees = 0;
            LocalPerformanceInfo.NonPagedPoolLookasideHits = 0;
            ExQueryPoolUsage( &LocalPerformanceInfo.PagedPoolPages,
                              &LocalPerformanceInfo.NonPagedPoolPages,
                              &LocalPerformanceInfo.PagedPoolAllocs,
                              &LocalPerformanceInfo.PagedPoolFrees,
                              &LocalPerformanceInfo.PagedPoolLookasideHits,
                              &LocalPerformanceInfo.NonPagedPoolAllocs,
                              &LocalPerformanceInfo.NonPagedPoolFrees,
                              &LocalPerformanceInfo.NonPagedPoolLookasideHits
                            );

            //
            // Cache Manager information.
            //

            LocalPerformanceInfo.CcFastReadNoWait = CcFastReadNoWait;
            LocalPerformanceInfo.CcFastReadWait = CcFastReadWait;
            LocalPerformanceInfo.CcFastReadResourceMiss = CcFastReadResourceMiss;
            LocalPerformanceInfo.CcFastReadNotPossible = CcFastReadNotPossible;
            LocalPerformanceInfo.CcFastMdlReadNoWait = CcFastMdlReadNoWait;
            LocalPerformanceInfo.CcFastMdlReadWait = CcFastMdlReadWait;
            LocalPerformanceInfo.CcFastMdlReadResourceMiss = CcFastMdlReadResourceMiss;
            LocalPerformanceInfo.CcFastMdlReadNotPossible = CcFastMdlReadNotPossible;
            LocalPerformanceInfo.CcMapDataNoWait = CcMapDataNoWait;
            LocalPerformanceInfo.CcMapDataWait = CcMapDataWait;
            LocalPerformanceInfo.CcMapDataNoWaitMiss = CcMapDataNoWaitMiss;
            LocalPerformanceInfo.CcMapDataWaitMiss = CcMapDataWaitMiss;
            LocalPerformanceInfo.CcPinMappedDataCount = CcPinMappedDataCount;
            LocalPerformanceInfo.CcPinReadNoWait = CcPinReadNoWait;
            LocalPerformanceInfo.CcPinReadWait = CcPinReadWait;
            LocalPerformanceInfo.CcPinReadNoWaitMiss = CcPinReadNoWaitMiss;
            LocalPerformanceInfo.CcPinReadWaitMiss = CcPinReadWaitMiss;
            LocalPerformanceInfo.CcCopyReadNoWait = CcCopyReadNoWait;
            LocalPerformanceInfo.CcCopyReadWait = CcCopyReadWait;
            LocalPerformanceInfo.CcCopyReadNoWaitMiss = CcCopyReadNoWaitMiss;
            LocalPerformanceInfo.CcCopyReadWaitMiss = CcCopyReadWaitMiss;
            LocalPerformanceInfo.CcMdlReadNoWait = CcMdlReadNoWait;
            LocalPerformanceInfo.CcMdlReadWait = CcMdlReadWait;
            LocalPerformanceInfo.CcMdlReadNoWaitMiss = CcMdlReadNoWaitMiss;
            LocalPerformanceInfo.CcMdlReadWaitMiss = CcMdlReadWaitMiss;
            LocalPerformanceInfo.CcReadAheadIos = CcReadAheadIos;
            LocalPerformanceInfo.CcLazyWriteIos = CcLazyWriteIos;
            LocalPerformanceInfo.CcLazyWritePages = CcLazyWritePages;
            LocalPerformanceInfo.CcDataFlushes = CcDataFlushes;
            LocalPerformanceInfo.CcDataPages = CcDataPages;

#if !defined(NT_UP)
            //
            // On an MP machines go sum up some other 'hot' cache manager
            // statistics.
            //

            for (i = 0; i < (ULONG)KeNumberProcessors; i++) {
                Prcb = KiProcessorBlock[i];

                LocalPerformanceInfo.CcFastReadNoWait += Prcb->CcFastReadNoWait;
                LocalPerformanceInfo.CcFastReadWait += Prcb->CcFastReadWait;
                LocalPerformanceInfo.CcFastReadNotPossible += Prcb->CcFastReadNotPossible;
                LocalPerformanceInfo.CcCopyReadNoWait += Prcb->CcCopyReadNoWait;
                LocalPerformanceInfo.CcCopyReadWait += Prcb->CcCopyReadWait;
                LocalPerformanceInfo.CcCopyReadNoWaitMiss += Prcb->CcCopyReadNoWaitMiss;
            }
#endif
            *PerformanceInfo = LocalPerformanceInfo;
            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof(LocalPerformanceInfo);
            }

            break;

        case SystemProcessorPerformanceInformation:
            if (SystemInformationLength <
                sizeof( SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            ProcessorPerformanceInfo =
                (PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) SystemInformation;

            Length = 0;
            for (i = 0; i < (ULONG)KeNumberProcessors; i++) {
                Prcb = KiProcessorBlock[i];
                if (Prcb != NULL) {
                    if (SystemInformationLength < Length + sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION))
                        break;

                    Length += sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION);

                    ProcessorPerformanceInfo->UserTime.QuadPart =
                                                UInt32x32To64(Prcb->UserTime,
                                                              KeMaximumIncrement);

                    ProcessorPerformanceInfo->KernelTime.QuadPart =
                                                UInt32x32To64(Prcb->KernelTime,
                                                              KeMaximumIncrement);

                    ProcessorPerformanceInfo->DpcTime.QuadPart =
                                                UInt32x32To64(Prcb->DpcTime,
                                                              KeMaximumIncrement);

                    ProcessorPerformanceInfo->InterruptTime.QuadPart =
                                                UInt32x32To64(Prcb->InterruptTime,
                                                              KeMaximumIncrement);

                    ProcessorPerformanceInfo->IdleTime.QuadPart =
                                                UInt32x32To64(Prcb->IdleThread->KernelTime,
                                                              KeMaximumIncrement);

                    ProcessorPerformanceInfo->InterruptCount = Prcb->InterruptCount;

                    ProcessorPerformanceInfo++;
                }
            }

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }

            break;

        case SystemProcessorPowerInformation:
            if (SystemInformationLength < sizeof( SYSTEM_PROCESSOR_POWER_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExpGetProcessorPowerInformation(
                SystemInformation,
                SystemInformationLength,
                &Length
                );

            if (ARGUMENT_PRESENT (ReturnLength)) {
                *ReturnLength = Length;
            }
            break;

        case SystemProcessorIdleInformation:
            if (SystemInformationLength < sizeof( SYSTEM_PROCESSOR_IDLE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExpGetProcessorIdleInformation(
                SystemInformation,
                SystemInformationLength,
                &Length
                );

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }
            break;

        case SystemTimeOfDayInformation:
            if (SystemInformationLength > sizeof (SYSTEM_TIMEOFDAY_INFORMATION)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            RtlZeroMemory (&LocalTimeOfDayInfo, sizeof(LocalTimeOfDayInfo));
            KeQuerySystemTime(&LocalTimeOfDayInfo.CurrentTime);
            LocalTimeOfDayInfo.BootTime = KeBootTime;
            LocalTimeOfDayInfo.TimeZoneBias = ExpTimeZoneBias;
            LocalTimeOfDayInfo.TimeZoneId = ExpCurrentTimeZoneId;
            LocalTimeOfDayInfo.BootTimeBias = KeBootTimeBias;
            LocalTimeOfDayInfo.SleepTimeBias = KeInterruptTimeBias;

            try {
                RtlCopyMemory (
                    SystemInformation,
                    &LocalTimeOfDayInfo,
                    SystemInformationLength
                    );

                if (ARGUMENT_PRESENT(ReturnLength) ) {
                    *ReturnLength = SystemInformationLength;
                }
            } except(EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode ();
            }

            break;

            //
            // Query system time adjustment information.
            //

        case SystemTimeAdjustmentInformation:
            if (SystemInformationLength != sizeof( SYSTEM_QUERY_TIME_ADJUST_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            TimeAdjustmentInformation =
                    (PSYSTEM_QUERY_TIME_ADJUST_INFORMATION)SystemInformation;

            TimeAdjustmentInformation->TimeAdjustment = KeTimeAdjustment;
            TimeAdjustmentInformation->TimeIncrement = KeMaximumIncrement;
            TimeAdjustmentInformation->Enable = KeTimeSynchronization;
            break;

        case SystemSummaryMemoryInformation:
        case SystemFullMemoryInformation:

            if (SystemInformationLength < sizeof( SYSTEM_MEMORY_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = MmMemoryUsage (SystemInformation,
                                    SystemInformationLength,
             (SystemInformationClass == SystemFullMemoryInformation) ? 0 : 1,
                                    &Length);

            if (NT_SUCCESS(Status) && ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }
            break;

        case SystemPathInformation:
#if DBG
            DbgPrint( "EX: SystemPathInformation now available via SharedUserData\n" );
            DbgBreakPoint();
#endif
            return STATUS_NOT_IMPLEMENTED;
            break;

        case SystemProcessInformation:
        case SystemExtendedProcessInformation:
            {
                BOOLEAN ExtendedInformation;

                if (SystemInformationClass == SystemProcessInformation ) {
                    ExtendedInformation = FALSE;
                } else {
                    ExtendedInformation = TRUE;
                }

                Status = ExpGetProcessInformation (SystemInformation,
                                               SystemInformationLength,
                                               ReturnLength,
                                               NULL,
                                               ExtendedInformation);
            }

            break;

        case SystemSessionProcessInformation:


            SessionProcessInformation =
                        (PSYSTEM_SESSION_PROCESS_INFORMATION)SystemInformation;

            if (SystemInformationLength < sizeof( SYSTEM_SESSION_PROCESS_INFORMATION)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }
            //
            // The lower level locks the buffer specified below into memory using MmProbeAndLockPages.
            // We don't need to probe the buffers here.
            //
            SessionId = SessionProcessInformation->SessionId;
            ProcessInformation = SessionProcessInformation->Buffer;
            ProcessInformationLength = SessionProcessInformation->SizeOfBuf;

            if (!POINTER_IS_ALIGNED (ProcessInformation, sizeof (ULONG))) {
                return STATUS_DATATYPE_MISALIGNMENT;
            }

            Status = ExpGetProcessInformation (ProcessInformation,
                                               ProcessInformationLength,
                                               ReturnLength,
                                               &SessionId,
                                               FALSE);
            break;

        case SystemCallCountInformation:

            Length = sizeof(SYSTEM_CALL_COUNT_INFORMATION) +
                        (NUMBER_SERVICE_TABLES * sizeof(ULONG));

            Table = KeServiceDescriptorTableShadow;

            for (i = 0; i < NUMBER_SERVICE_TABLES; i += 1) {
                if ((Table->Limit != 0) && (Table->Count != NULL)) {
                    Length += Table->Limit * sizeof(ULONG);
                }
                Table += 1;
            }

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }

            if (SystemInformationLength < Length) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            CallCountInformation = (PSYSTEM_CALL_COUNT_INFORMATION)SystemInformation;
            CallCountInformation->Length = Length;
            CallCountInformation->NumberOfTables = NUMBER_SERVICE_TABLES;

            TableLimit = (PULONG)(CallCountInformation + 1);
            TableCounts = TableLimit + NUMBER_SERVICE_TABLES;

            Table = KeServiceDescriptorTableShadow;

            for (i = 0; i < NUMBER_SERVICE_TABLES; i += 1) {
                if ((Table->Limit == 0) || (Table->Count == NULL)) {
                    *TableLimit++ = 0;
                } else {
                    *TableLimit++ = Table->Limit;
                    RtlCopyMemory((PVOID)TableCounts,
                                  (PVOID)Table->Count,
                                  Table->Limit * sizeof(ULONG));
                    TableCounts += Table->Limit;
                }
                Table += 1;
            }

            break;

        case SystemDeviceInformation:
            if (SystemInformationLength != sizeof( SYSTEM_DEVICE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            ConfigInfo = IoGetConfigurationInformation();
            DeviceInformation = (PSYSTEM_DEVICE_INFORMATION)SystemInformation;
            DeviceInformation->NumberOfDisks = ConfigInfo->DiskCount;
            DeviceInformation->NumberOfFloppies = ConfigInfo->FloppyCount;
            DeviceInformation->NumberOfCdRoms = ConfigInfo->CdRomCount;
            DeviceInformation->NumberOfTapes = ConfigInfo->TapeCount;
            DeviceInformation->NumberOfSerialPorts = ConfigInfo->SerialCount;
            DeviceInformation->NumberOfParallelPorts = ConfigInfo->ParallelCount;

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof( SYSTEM_DEVICE_INFORMATION );
            }
            break;

        case SystemFlagsInformation:
            if (SystemInformationLength != sizeof( SYSTEM_FLAGS_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            ((PSYSTEM_FLAGS_INFORMATION)SystemInformation)->Flags = NtGlobalFlag;

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof( SYSTEM_FLAGS_INFORMATION );
            }
            break;

        case SystemCallTimeInformation:
            return STATUS_NOT_IMPLEMENTED;

        case SystemModuleInformation:
            KeEnterCriticalRegion();
            ExAcquireResourceExclusiveLite( &PsLoadedModuleResource, TRUE );
            try {
                Status = ExpQueryModuleInformation( &PsLoadedModuleList,
                                                    &MmLoadedUserImageList,
                                                    (PRTL_PROCESS_MODULES)SystemInformation,
                                                    SystemInformationLength,
                                                    ReturnLength
                                                );
            } except(EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
            }
            ExReleaseResourceLite (&PsLoadedModuleResource);
            KeLeaveCriticalRegion();
            break;

        case SystemLocksInformation:
            if (SystemInformationLength < sizeof( RTL_PROCESS_LOCKS )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExpGetLockInformation (SystemInformation,
                                            SystemInformationLength,
                                            &Length);

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }

            break;

        case SystemStackTraceInformation:
            if (SystemInformationLength < sizeof( RTL_PROCESS_BACKTRACES )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

#if i386
            Status = ExpGetStackTraceInformation (SystemInformation,
                                                  SystemInformationLength,
                                                  &Length);
#else
            Status = STATUS_NOT_IMPLEMENTED;
#endif // i386

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }

            break;

        case SystemPagedPoolInformation:

            Status = STATUS_NOT_IMPLEMENTED;

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = 0;
            }
            break;

        case SystemNonPagedPoolInformation:

            Status = STATUS_NOT_IMPLEMENTED;

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = 0;
            }
            break;

        case SystemHandleInformation:
            if (SystemInformationLength < sizeof( SYSTEM_HANDLE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            if (!POINTER_IS_ALIGNED (SystemInformation, TYPE_ALIGNMENT (SYSTEM_HANDLE_INFORMATION))) {
                return STATUS_DATATYPE_MISALIGNMENT;
            }

            Status = ExpGetHandleInformation( SystemInformation,
                                              SystemInformationLength,
                                              &Length
                                            );

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }
            break;

        case SystemExtendedHandleInformation:
            if (SystemInformationLength < sizeof( SYSTEM_HANDLE_INFORMATION_EX )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            if (!POINTER_IS_ALIGNED (SystemInformation, TYPE_ALIGNMENT (SYSTEM_HANDLE_INFORMATION_EX))) {
                return STATUS_DATATYPE_MISALIGNMENT;
            }

            Status = ExpGetHandleInformationEx( SystemInformation,
                                                SystemInformationLength,
                                                &Length
                                              );

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }
            break;

        case SystemObjectInformation:
            if (SystemInformationLength < sizeof( SYSTEM_OBJECTTYPE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExpGetObjectInformation( SystemInformation,
                                              SystemInformationLength,
                                              &Length
                                            );

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }
            break;

        case SystemPageFileInformation:

            if (SystemInformationLength < sizeof( SYSTEM_PAGEFILE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = MmGetPageFileInformation( SystemInformation,
                                               SystemInformationLength,
                                               &Length
                                              );

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }
            break;


        case SystemFileCacheInformation:
        case SystemFileCacheInformationEx:

            {

            SYSTEM_FILECACHE_INFORMATION CapturedFileCacheInformation;

            //
            // This structure was extended in NT 4.0 from 12 bytes.
            // Use the previous size of 12 bytes for versioning info.
            //

            if (SystemInformationLength < 12) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            MmQuerySystemCacheWorkingSetInformation (&CapturedFileCacheInformation);

            FileCache = (PSYSTEM_FILECACHE_INFORMATION)SystemInformation;
            FileCache->CurrentSize = CapturedFileCacheInformation.CurrentSize;
            FileCache->PeakSize = CapturedFileCacheInformation.PeakSize;
            FileCache->PageFaultCount = CapturedFileCacheInformation.PageFaultCount;

            i = 12;

            if (SystemInformationLength >= sizeof( SYSTEM_FILECACHE_INFORMATION )) {
                i = sizeof (SYSTEM_FILECACHE_INFORMATION);
                FileCache->MinimumWorkingSet =
                            CapturedFileCacheInformation.MinimumWorkingSet;
                FileCache->MaximumWorkingSet =
                            CapturedFileCacheInformation.MaximumWorkingSet;
                FileCache->CurrentSizeIncludingTransitionInPages =
                    CapturedFileCacheInformation.CurrentSizeIncludingTransitionInPages;
                FileCache->PeakSizeIncludingTransitionInPages =
                    CapturedFileCacheInformation.PeakSizeIncludingTransitionInPages;
                FileCache->TransitionRePurposeCount =
                    CapturedFileCacheInformation.TransitionRePurposeCount;

                FileCache->Flags = CapturedFileCacheInformation.Flags;
            }

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = i;
            }
            break;

            }

        case SystemSessionPoolTagInformation:

            SessionProcessInformation =
                        (PSYSTEM_SESSION_PROCESS_INFORMATION)SystemInformation;

            if (SystemInformationLength < sizeof( SYSTEM_SESSION_PROCESS_INFORMATION)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            //
            // The lower level locks the buffer specified below into
            // memory using MmProbeAndLockPages.
            // We don't need to probe the buffers here.
            //

            SessionId = SessionProcessInformation->SessionId;
            SessionPoolTagInformation = SessionProcessInformation->Buffer;
            SessionPoolTagInformationLength = SessionProcessInformation->SizeOfBuf;

            if (!POINTER_IS_ALIGNED (SessionPoolTagInformation, sizeof (ULONGLONG))) {
                return STATUS_DATATYPE_MISALIGNMENT;
            }

            Status = ExGetSessionPoolTagInformation (
                                            SessionPoolTagInformation,
                                            SessionPoolTagInformationLength,
                                            &Length,
                                            &SessionId);

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }

            break;

        case SystemPoolTagInformation:

            if (SystemInformationLength < sizeof( SYSTEM_POOLTAG_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExGetPoolTagInfo (SystemInformation,
                                       SystemInformationLength,
                                       ReturnLength);

            break;

        case SystemBigPoolInformation:

            if (SystemInformationLength < sizeof( SYSTEM_BIGPOOL_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExGetBigPoolInfo (SystemInformation,
                                       SystemInformationLength,
                                       ReturnLength);

            break;

        case SystemSessionMappedViewInformation:

            SessionMappedViewInformation =
                        (PSYSTEM_SESSION_MAPPED_VIEW_INFORMATION)SystemInformation;

            if (SystemInformationLength < sizeof( SYSTEM_SESSION_MAPPED_VIEW_INFORMATION)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            //
            // The lower level locks the buffer specified below into
            // memory using MmProbeAndLockPages.
            // We don't need to probe the buffers here.
            //

            SessionId = SessionMappedViewInformation->SessionId;

            if (!POINTER_IS_ALIGNED (SessionMappedViewInformation, sizeof (ULONGLONG))) {
                return STATUS_DATATYPE_MISALIGNMENT;
            }

            Status = MmGetSessionMappedViewInformation (
                                            SessionMappedViewInformation,
                                            SystemInformationLength,
                                            &Length,
                                            &SessionId);

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }

            break;

        case SystemVdmInstemulInformation:
#ifdef i386
            if (SystemInformationLength < sizeof( SYSTEM_VDM_INSTEMUL_INFO )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExpGetInstemulInformation(
                                            (PSYSTEM_VDM_INSTEMUL_INFO)SystemInformation
                                            );

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof(SYSTEM_VDM_INSTEMUL_INFO);
            }
#else
            Status = STATUS_NOT_IMPLEMENTED;
#endif
            break;

            //
            // Get system exception information which includes the number
            // of exceptions that have dispatched, the number of alignment
            // fixups, and the number of floating emulations that have been
            // performed.
            //

        case SystemExceptionInformation:
            if (SystemInformationLength < sizeof( SYSTEM_EXCEPTION_INFORMATION)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof(SYSTEM_EXCEPTION_INFORMATION);
            }

            ExceptionInformation = (PSYSTEM_EXCEPTION_INFORMATION)SystemInformation;

            //
            // Ke information.
            //
            // These counters are kept on a per processor basis and must
            // be totaled.
            //

            {
                ULONG AlignmentFixupCount = 0;
                ULONG ExceptionDispatchCount = 0;
                ULONG FloatingEmulationCount = 0;
                ULONG ByteWordEmulationCount = 0;

                for (i = 0; i < (ULONG)KeNumberProcessors; i += 1) {
                    Prcb = KiProcessorBlock[i];
                    if (Prcb != NULL) {
                        AlignmentFixupCount += Prcb->KeAlignmentFixupCount;
                        ExceptionDispatchCount += Prcb->KeExceptionDispatchCount;
                        FloatingEmulationCount += Prcb->KeFloatingEmulationCount;
                    }
                }

                ExceptionInformation->AlignmentFixupCount = AlignmentFixupCount;
                ExceptionInformation->ExceptionDispatchCount = ExceptionDispatchCount;
                ExceptionInformation->FloatingEmulationCount = FloatingEmulationCount;
                ExceptionInformation->ByteWordEmulationCount = ByteWordEmulationCount;
            }

            break;

        case SystemKernelDebuggerInformation:

            if (SystemInformationLength < sizeof( SYSTEM_KERNEL_DEBUGGER_INFORMATION)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            KernelDebuggerInformation =
                (PSYSTEM_KERNEL_DEBUGGER_INFORMATION)SystemInformation;
            KernelDebuggerInformation->KernelDebuggerEnabled = KdDebuggerEnabled;
            KernelDebuggerInformation->KernelDebuggerNotPresent = KdDebuggerNotPresent;

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof(SYSTEM_KERNEL_DEBUGGER_INFORMATION);
            }

            break;

        case SystemContextSwitchInformation:

            if (SystemInformationLength < sizeof( SYSTEM_CONTEXT_SWITCH_INFORMATION)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            ContextSwitchInformation =
                (PSYSTEM_CONTEXT_SWITCH_INFORMATION)SystemInformation;

            //
            // Compute the total number of context switches and fill in the
            // remainder of the context switch information.
            //

            ContextSwitches = 0;
            for (i = 0; i < (ULONG)KeNumberProcessors; i += 1) {
                Prcb = KiProcessorBlock[i];
                if (Prcb != NULL) {
                    ContextSwitches += KeGetContextSwitches(Prcb);
                }

            }

            ContextSwitchInformation->ContextSwitches = ContextSwitches;
            ContextSwitchInformation->FindAny = KeThreadSwitchCounters.FindAny;
            ContextSwitchInformation->FindLast = KeThreadSwitchCounters.FindLast;
            ContextSwitchInformation->FindIdeal = KeThreadSwitchCounters.FindIdeal;
            ContextSwitchInformation->IdleAny = KeThreadSwitchCounters.IdleAny;
            ContextSwitchInformation->IdleCurrent = KeThreadSwitchCounters.IdleCurrent;
            ContextSwitchInformation->IdleLast = KeThreadSwitchCounters.IdleLast;
            ContextSwitchInformation->IdleIdeal = KeThreadSwitchCounters.IdleIdeal;
            ContextSwitchInformation->PreemptAny = KeThreadSwitchCounters.PreemptAny;
            ContextSwitchInformation->PreemptCurrent = KeThreadSwitchCounters.PreemptCurrent;
            ContextSwitchInformation->PreemptLast = KeThreadSwitchCounters.PreemptLast;
            ContextSwitchInformation->SwitchToIdle = KeThreadSwitchCounters.SwitchToIdle;

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof(SYSTEM_CONTEXT_SWITCH_INFORMATION);
            }

            break;

        case SystemRegistryQuotaInformation:

            if (SystemInformationLength < sizeof( SYSTEM_REGISTRY_QUOTA_INFORMATION)) {
                return(STATUS_INFO_LENGTH_MISMATCH);
            }
            CmQueryRegistryQuotaInformation((PSYSTEM_REGISTRY_QUOTA_INFORMATION)SystemInformation);

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof(SYSTEM_REGISTRY_QUOTA_INFORMATION);
            }
            break;

        case SystemDpcBehaviorInformation:
            {
                PSYSTEM_DPC_BEHAVIOR_INFORMATION DpcInfo;
                //
                // If the system information buffer is not the correct length,
                // then return an error.
                //
                if (SystemInformationLength != sizeof(SYSTEM_DPC_BEHAVIOR_INFORMATION)) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                DpcInfo = (PSYSTEM_DPC_BEHAVIOR_INFORMATION)SystemInformation;

                //
                // Exception handler for this routine will return the correct
                // error if any of these accesses fail.
                //
                //
                // Return the current DPC behavior variables
                //
                DpcInfo->DpcQueueDepth = KiMaximumDpcQueueDepth;
                DpcInfo->MinimumDpcRate = KiMinimumDpcRate;
                DpcInfo->AdjustDpcThreshold = KiAdjustDpcThreshold;
                DpcInfo->IdealDpcRate = KiIdealDpcRate;
            }
            break;

        case SystemInterruptInformation:

            if (SystemInformationLength < (sizeof(SYSTEM_INTERRUPT_INFORMATION) * KeNumberProcessors)) {
                return(STATUS_INFO_LENGTH_MISMATCH);
            }

            InterruptInformation = (PSYSTEM_INTERRUPT_INFORMATION)SystemInformation;
            for (i=0; i < (ULONG)KeNumberProcessors; i++) {
                Prcb = KiProcessorBlock[i];
                InterruptInformation->ContextSwitches = KeGetContextSwitches(Prcb);
                InterruptInformation->DpcCount = Prcb->DpcData[DPC_NORMAL].DpcCount;
                InterruptInformation->DpcRate = Prcb->DpcRequestRate;
                InterruptInformation->TimeIncrement = KeTimeIncrement;
                InterruptInformation->DpcBypassCount = 0;
                InterruptInformation->ApcBypassCount = 0;

                ++InterruptInformation;
            }

            break;

        case SystemCurrentTimeZoneInformation:
            if (SystemInformationLength < sizeof( RTL_TIME_ZONE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            RtlCopyMemory(SystemInformation,&ExpTimeZoneInformation,sizeof(ExpTimeZoneInformation));
            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = sizeof( RTL_TIME_ZONE_INFORMATION );
            }

            Status = STATUS_SUCCESS;
            break;

            //
            // Query pool lookaside list and general lookaside list
            // information.
            //

        case SystemLookasideInformation:
            Status = ExpGetLookasideInformation(SystemInformation,
                                                SystemInformationLength,
                                                &Length);

            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = Length;
            }

            break;

        case SystemRangeStartInformation:

            if ( SystemInformationLength != sizeof(ULONG_PTR) ) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            *(PULONG_PTR)SystemInformation = (ULONG_PTR)MmSystemRangeStart;

            if (ARGUMENT_PRESENT(ReturnLength) ) {
                *ReturnLength = sizeof(ULONG_PTR);
            }

            break;

        case SystemVerifierInformation:

            if (SystemInformationLength < sizeof( SYSTEM_VERIFIER_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = MmGetVerifierInformation( SystemInformation,
                                               SystemInformationLength,
                                               &Length
                                              );

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = Length;
            }
            break;

        case SystemLegacyDriverInformation:
            if (SystemInformationLength < sizeof(SYSTEM_LEGACY_DRIVER_INFORMATION)) {
                return(STATUS_INFO_LENGTH_MISMATCH);
            }
            Length = SystemInformationLength;
            Status = ExpQueryLegacyDriverInformation((PSYSTEM_LEGACY_DRIVER_INFORMATION)SystemInformation, &Length);
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = Length;
            }
            break;

        case SystemPerformanceTraceInformation:
            Status = STATUS_INVALID_INFO_CLASS;
            break;

        case SystemPrefetcherInformation:

            Status = CcPfQueryPrefetcherInformation(SystemInformationClass,
                                                    SystemInformation,
                                                    SystemInformationLength,
                                                    PreviousMode,
                                                    &Length
                                                    );

            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = Length;
            }

            break;

        case SystemNumaProcessorMap:

            Status = ExpQueryNumaProcessorMap(SystemInformation,
                                              SystemInformationLength,
                                              &Length);
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = Length;
            }
            break;

        case SystemNumaAvailableMemory:

            Status = ExpQueryNumaAvailableMemory(SystemInformation,
                                                 SystemInformationLength,
                                                 &Length);
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = Length;
            }
            break;

        case SystemRecommendedSharedDataAlignment:
            if (SystemInformationLength < sizeof(ULONG)) {
                return(STATUS_INFO_LENGTH_MISMATCH);
            }

            //
            // Alignment is guaranteed by the ProbeForWrite above
            // so just store the value as a ULONG.
            //

            *(PULONG)SystemInformation = KeGetRecommendedSharedDataAlignment();
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = sizeof(ULONG);
            }
            break;

        case SystemComPlusPackage:
            if (SystemInformationLength != sizeof(ULONG)) {
                return(STATUS_INFO_LENGTH_MISMATCH);
            }

            if (SharedUserData->ComPlusPackage == COMPLUS_PACKAGE_INVALID) {

                //
                // The initialization happens one time.
                //
                SharedUserData->ComPlusPackage = 0;

                ExpReadComPlusPackage ();
            }

            *(PULONG)SystemInformation = SharedUserData->ComPlusPackage;
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = sizeof(ULONG);
            }
            break;

        case SystemLostDelayedWriteInformation:

            if (SystemInformationLength < sizeof(ULONG)) {
                return(STATUS_INFO_LENGTH_MISMATCH);
            }

            *(PULONG)SystemInformation = CcLostDelayedWrites;
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = sizeof(ULONG);
            }
            break;

        case SystemObjectSecurityMode:

            if (SystemInformationLength != sizeof (ULONG)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            *(PULONG)SystemInformation = ObGetSecurityMode ();

            if (ARGUMENT_PRESENT (ReturnLength)) {
                *ReturnLength = sizeof (ULONG);
            }

            break;

        case SystemWatchdogTimerInformation:

            {
                PSYSTEM_WATCHDOG_TIMER_INFORMATION WdTimerInfo = (PSYSTEM_WATCHDOG_TIMER_INFORMATION) SystemInformation;

                //
                // Caller must be kernel mode with the proper parameters
                //

                if (PreviousMode != KernelMode || SystemInformation == NULL || SystemInformationLength != sizeof(SYSTEM_WATCHDOG_TIMER_INFORMATION)) {
                    ExRaiseStatus (STATUS_INVALID_PARAMETER);
                }

                if (ExpWdHandler == NULL) {

                    Status = STATUS_NOT_IMPLEMENTED;

                } else {

                    switch (WdTimerInfo->WdInfoClass) {
                        case WdInfoTimeoutValue:
                            Status = ExpWdHandler( WdActionQueryTimeoutValue, ExpWdHandlerContext, &WdTimerInfo->DataValue, FALSE );
                            break;

                        case WdInfoTriggerAction:
                            Status = ExpWdHandler( WdActionQueryTriggerAction, ExpWdHandlerContext, &WdTimerInfo->DataValue, FALSE );
                            break;

                        case WdInfoState:
                            Status = ExpWdHandler( WdActionQueryState, ExpWdHandlerContext, &WdTimerInfo->DataValue, FALSE );
                            break;

                        default:
                            Status = STATUS_INVALID_PARAMETER;
                            break;
                    }
                }
            }

            break;

        case SystemLogicalProcessorInformation:

            Status = KeQueryLogicalProcessorInformation(
                         SystemInformation,
                         SystemInformationLength,
                         &Length);
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = Length;
            }
            break;

        case SystemFirmwareTableInformation:
 
            if (SystemInformationLength < sizeof(SYSTEM_FIRMWARE_TABLE_INFORMATION)) {

                return STATUS_INFO_LENGTH_MISMATCH;
            }
           
            Status = ExpGetSystemFirmwareTableInformation(SystemInformation,
                                                          PreviousMode, 
                                                          ReturnLength);

            break;

        default:

            //
            // Invalid argument.
            //

            return STATUS_INVALID_INFO_CLASS;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();
    }

    return Status;
}

NTSTATUS
NTAPI
NtSetSystemInformation (
    __in SYSTEM_INFORMATION_CLASS SystemInformationClass,
    __in_bcount_opt(SystemInformationLength) PVOID SystemInformation,
    __in ULONG SystemInformationLength
    )

/*++

Routine Description:

    This function set information about the system.

Arguments:

    SystemInformationClass - The system information class which is to
        be modified.

    SystemInformation - A pointer to a buffer which contains the specified
        information. The format and content of the buffer depend on the
        specified system information class.


    SystemInformationLength - Specifies the length in bytes of the system
        information buffer.

Return Value:

    Returns one of the following status codes:

        STATUS_SUCCESS - Normal, successful completion.

        STATUS_ACCESS_VIOLATION - The specified system information buffer
            is not accessible.

        STATUS_INVALID_INFO_CLASS - The SystemInformationClass parameter
            did not specify a valid value.

        STATUS_INFO_LENGTH_MISMATCH - The value of the SystemInformationLength
            parameter did not match the length required for the information
            class requested by the SystemInformationClass parameter.

        STATUS_PRIVILEGE_NOT_HELD is returned if the caller does not have the
            privilege to set the system time.

--*/

{

    BOOLEAN Enable;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    ULONG TimeAdjustment;
    PSYSTEM_SET_TIME_ADJUST_INFORMATION TimeAdjustmentInformation;
    HANDLE EventHandle;
    PVOID Event;
    ULONG LoadFlags = MM_LOAD_IMAGE_IN_SESSION;

    PAGED_CODE();

    //
    // Establish an exception handle in case the system information buffer
    // is not accessible.
    //

    Status = STATUS_SUCCESS;

    try {

        //
        // Get the previous processor mode and probe the input buffer for
        // read access if necessary.
        //

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForRead((PVOID)SystemInformation,
                         SystemInformationLength,
                         sizeof(ULONG));
        }

        //
        // Dispatch on the system information class.
        //

        switch (SystemInformationClass) {
        case SystemFlagsInformation:
            if (SystemInformationLength != sizeof( SYSTEM_FLAGS_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            if (!SeSinglePrivilegeCheck( SeDebugPrivilege, PreviousMode )) {
                return STATUS_ACCESS_DENIED;
            }
            else {
                ULONG Flags;

                Flags = ((PSYSTEM_FLAGS_INFORMATION)SystemInformation)->Flags &
                         ~(FLG_KERNELMODE_VALID_BITS | FLG_BOOTONLY_VALID_BITS);
                Flags |= NtGlobalFlag & (FLG_KERNELMODE_VALID_BITS | FLG_BOOTONLY_VALID_BITS);
                NtGlobalFlag = Flags;
                ((PSYSTEM_FLAGS_INFORMATION)SystemInformation)->Flags = NtGlobalFlag;
            }
            break;

            //
            // Set system time adjustment information.
            //
            // N.B. The caller must have the SeSystemTime privilege.
            //

        case SystemTimeAdjustmentInformation:

            //
            // If the system information buffer is not the correct length,
            // then return an error.
            //

            if (SystemInformationLength != sizeof( SYSTEM_SET_TIME_ADJUST_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            //
            // If the current thread does not have the privilege to set the
            // time adjustment variables, then return an error.
            //

            if ((PreviousMode != KernelMode) &&
                (SeSinglePrivilegeCheck(SeSystemtimePrivilege, PreviousMode) == FALSE)) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }

            //
            // Set system time adjustment parameters.
            //

            TimeAdjustmentInformation =
                    (PSYSTEM_SET_TIME_ADJUST_INFORMATION)SystemInformation;

            Enable = TimeAdjustmentInformation->Enable;
            TimeAdjustment = TimeAdjustmentInformation->TimeAdjustment;

            if (Enable == TRUE) {
                KeTimeAdjustment = KeMaximumIncrement;
            } else {
                if (TimeAdjustment == 0) {
                    return STATUS_INVALID_PARAMETER_2;
                }
                KeTimeAdjustment = TimeAdjustment;
            }

            KeTimeSynchronization = Enable;
            break;

            //
            // Set an event to signal when the clock interrupt has been
            // masked for too long, causing the time to slip.
            // The event will be referenced to prevent it from being
            // deleted.  If the new event handle is valid or NULL, the
            // old event will be dereferenced and forgotten.  If the
            // event handle is non-NULL but invalid, the old event will
            // be remembered and a failure status will be returned.
            //
            // N.B. The caller must have the SeSystemTime privilege.
            //
        case SystemTimeSlipNotification:

            if (SystemInformationLength != sizeof(HANDLE)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            //
            // If the current thread does not have the privilege to set the
            // time adjustment variables, then return an error.
            //

            if ((PreviousMode != KernelMode) &&
                (SeSinglePrivilegeCheck(SeSystemtimePrivilege, PreviousMode) == FALSE)) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }

            EventHandle = *(PHANDLE)SystemInformation;

            if (EventHandle == NULL) {

                //
                // Dereference the old event and don't signal anything
                // for time slips.
                //

                Event = NULL;
                Status = STATUS_SUCCESS;

            } else {

                Status = ObReferenceObjectByHandle(EventHandle,
                                                   EVENT_MODIFY_STATE,
                                                   ExEventObjectType,
                                                   PreviousMode,
                                                   &Event,
                                                   NULL);
            }

            if (NT_SUCCESS(Status)) {
                KdUpdateTimeSlipEvent(Event);
            }

            break;

            //
            // Set registry quota limit.
            //
            // N.B. The caller must have SeIncreaseQuotaPrivilege
            //
        case SystemRegistryQuotaInformation:

            //
            // If the system information buffer is not the correct length,
            // then return an error.
            //

            if (SystemInformationLength != sizeof( SYSTEM_REGISTRY_QUOTA_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            //
            // If the current thread does not have the privilege to create
            // a pagefile, then return an error.
            //

            if ((PreviousMode != KernelMode) &&
                (SeSinglePrivilegeCheck(SeIncreaseQuotaPrivilege, PreviousMode) == FALSE)) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }

            //
            // Set registry quota parameters.
            //
            CmSetRegistryQuotaInformation((PSYSTEM_REGISTRY_QUOTA_INFORMATION)SystemInformation);

            break;

        case SystemPrioritySeperation:
            {
                ULONG PrioritySeparation;

                //
                // If the system information buffer is not the correct length,
                // then return an error.
                //

                if (SystemInformationLength != sizeof (ULONG)) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                if (!SeSinglePrivilegeCheck (SeTcbPrivilege, PreviousMode)) {
                    return STATUS_PRIVILEGE_NOT_HELD;
                }

                try {
                    PrioritySeparation = *(PULONG)SystemInformation;
                }
                except(EXCEPTION_EXECUTE_HANDLER) {
                    return GetExceptionCode();
                }

                PsChangeQuantumTable (TRUE, PrioritySeparation);
                Status = STATUS_SUCCESS;
            }
            break;

            //
            // N.B. If this function is called from user mode it can only be
            //      used to load the win32k subsystem. The calling process
            //      must be smss, the subsystem to be loaded must be win32k,
            //      and the caller must have the privilege to load drivers.
            //      If this function is called from kernel mode it can be
            //      used to load any driver.
            //

        case SystemExtendServiceTableInformation:
            {

                ULONG_PTR EntryPoint;
                UNICODE_STRING Image;
                PVOID ImageBaseAddress;
                PDRIVER_INITIALIZE InitRoutine;
                PIMAGE_NT_HEADERS NtHeaders;
                PVOID SectionPointer;
                DRIVER_OBJECT Win32KDevice;

                //
                // Check if the system information buffer is the correct
                // length.
                //

                if (SystemInformationLength != sizeof(UNICODE_STRING)) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                //
                // If the previous mode is not kernel, then verify that the
                // caller can load the win32k subsystem.
                //

                if (PreviousMode != KernelMode) {

                    //
                    // Check if the caller is the session leader on the system.
                    //

                    if (MmIsSessionLeaderProcess(PsGetCurrentProcess()) == FALSE) {
                        return STATUS_PRIVILEGE_NOT_HELD;
                    }

                    //
                    // Check if the caller has the privilege to load drivers.
                    //

                    if (SeSinglePrivilegeCheck(SeLoadDriverPrivilege, UserMode) == FALSE) {
                        return STATUS_PRIVILEGE_NOT_HELD;
                    }

                    //
                    // Check if the win32k susbsystem is being loaded.
                    //

                    try {

                        //
                        // Probe and read the unicode string descriptor.
                        //

                        ProbeAndReadUnicodeStringEx(&Image,
                                                    (PUNICODE_STRING)SystemInformation);

                        //
                        // Check if the unicode string is the correct length.
                        //

                        if (Image.Length != WIN32K_PATH_SIZE) {
                            return STATUS_PRIVILEGE_NOT_HELD;
                        }

                        //
                        // Probe the unicode string buffer and check the path
                        // name.
                        //

                        ProbeForReadSmallStructure(Image.Buffer,
                                                   WIN32K_PATH_SIZE,
                                                   sizeof(UCHAR));

                        if (memcmp(Image.Buffer, &Win32kFullPath[0], WIN32K_PATH_SIZE) != 0) {
                            return STATUS_PRIVILEGE_NOT_HELD;
                        }
    
                        //
                        // Initialize string descriptor for win32k full
                        // path name.
                        //

                        Image.Buffer = &Win32kFullPath[0];
                        Image.MaximumLength = WIN32K_PATH_SIZE;

                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        return GetExceptionCode();
                    }

                    //
                    // Recursively call this kernel service forcing previous
                    // mode to kernel.
                    //

                    Status = ZwSetSystemInformation(SystemExtendServiceTableInformation,
                                                    (PVOID)&Image,
                                                    sizeof(Image));

                    return Status;
                }

                //
                // The previous mode is kernel - load the specified driver.
                //

                Image = *(PUNICODE_STRING)SystemInformation;
                Status = MmLoadSystemImage(&Image,
                                           NULL,
                                           NULL,
                                           MM_LOAD_IMAGE_IN_SESSION,
                                           &SectionPointer,
                                           (PVOID *)&ImageBaseAddress);

                if (NT_SUCCESS(Status) == FALSE) {
                    return Status;
                }

                //
                // Get the image base address.
                //

                NtHeaders = RtlImageNtHeader(ImageBaseAddress);
                if (NtHeaders == NULL) {
                    MmUnloadSystemImage(SectionPointer);
                    return STATUS_INVALID_IMAGE_FORMAT;
                }

                //
                // Extract the initialization routine address and call the
                // driver initialization routine.
                //

                EntryPoint = NtHeaders->OptionalHeader.AddressOfEntryPoint;
                EntryPoint += (ULONG_PTR)ImageBaseAddress;
                InitRoutine = (PDRIVER_INITIALIZE)EntryPoint;
                RtlZeroMemory(&Win32KDevice, sizeof(Win32KDevice));

                ASSERT(KeGetCurrentIrql() == 0);

                Win32KDevice.DriverStart = (PVOID)ImageBaseAddress;
                Status = (InitRoutine)(&Win32KDevice, NULL);

                ASSERT(KeGetCurrentIrql() == 0);

                //
                // If the image initialization was unsuccessful, then unload
                // the system image. Otherwise, set the system image unload
                // address so the session can be unloaded cleanly.
                //

                if (NT_SUCCESS(Status) == FALSE) {
                    MmUnloadSystemImage(SectionPointer);

                } else {
                    MmSessionSetUnloadAddress(&Win32KDevice);
                }
            }

            break;

        case SystemUnloadGdiDriverInformation:
            {

                if (SystemInformationLength != sizeof( PVOID ) ) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                if (PreviousMode != KernelMode) {

                    //
                    // The caller's access mode is not kernel so fail.
                    // Only GDI from the kernel can call this.
                    //

                    return STATUS_PRIVILEGE_NOT_HELD;

                }

                MmUnloadSystemImage( *((PVOID *)SystemInformation) );

                Status = STATUS_SUCCESS;

            }
            break;

        case SystemLoadGdiDriverInSystemSpace:
            {
                LoadFlags &= ~MM_LOAD_IMAGE_IN_SESSION;
                //
                // Fall through
                //
            }

        case SystemLoadGdiDriverInformation:
            {

                UNICODE_STRING Image;
                PVOID ImageBaseAddress;
                ULONG_PTR EntryPoint;
                PVOID SectionPointer;

                PIMAGE_NT_HEADERS NtHeaders;

                //
                // If the system information buffer is not the correct length,
                // then return an error.
                //

                if (SystemInformationLength != sizeof( SYSTEM_GDI_DRIVER_INFORMATION ) ) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                if (PreviousMode != KernelMode) {

                    //
                    // The caller's access mode is not kernel so fail.
                    // Only GDI from the kernel can call this.
                    //

                    return STATUS_PRIVILEGE_NOT_HELD;
                }

                Image = ((PSYSTEM_GDI_DRIVER_INFORMATION)SystemInformation)->DriverName;

                Status = MmLoadSystemImage (&Image,
                                            NULL,
                                            NULL,
                                            LoadFlags,
                                            &SectionPointer,
                                            (PVOID *) &ImageBaseAddress);


                if ((NT_SUCCESS( Status ))) {

                    PSYSTEM_GDI_DRIVER_INFORMATION GdiDriverInfo =
                        (PSYSTEM_GDI_DRIVER_INFORMATION) SystemInformation;

                    ULONG Size;

                    GdiDriverInfo->ExportSectionPointer =
                        RtlImageDirectoryEntryToData(ImageBaseAddress,
                                                     TRUE,
                                                     IMAGE_DIRECTORY_ENTRY_EXPORT,
                                                     &Size);

                    //
                    // Capture the entry point.
                    //

                    NtHeaders = RtlImageNtHeader( ImageBaseAddress );
                    EntryPoint = NtHeaders->OptionalHeader.AddressOfEntryPoint;
                    EntryPoint += (ULONG_PTR) ImageBaseAddress;

                    GdiDriverInfo->ImageAddress = (PVOID) ImageBaseAddress;
                    GdiDriverInfo->SectionPointer = SectionPointer;
                    GdiDriverInfo->EntryPoint = (PVOID) EntryPoint;
                    GdiDriverInfo->ImageLength = NtHeaders->OptionalHeader.SizeOfImage;
                }
            }
            break;


        case SystemFileCacheInformation:
        case SystemFileCacheInformationEx:

            {

            BOOLEAN IncreasePerformed;
            ULONG Flags;

            if (SystemInformationLength < sizeof( SYSTEM_FILECACHE_INFORMATION )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            if (SystemInformationClass == SystemFileCacheInformation) {
                Flags = 0;
            }
            else {

                //
                // Obtain the callers flags and validate it - don't allow
                // any reserved bits to be set or any conflicting bits to
                // be set.
                //

                Flags = ((PSYSTEM_FILECACHE_INFORMATION)SystemInformation)->Flags;
                if (Flags & ~(MM_WORKING_SET_MAX_HARD_ENABLE |
                              MM_WORKING_SET_MAX_HARD_DISABLE |
                              MM_WORKING_SET_MIN_HARD_ENABLE |
                              MM_WORKING_SET_MIN_HARD_DISABLE)) {

                    return STATUS_INVALID_PARAMETER_2;
                }

                if ((Flags & (MM_WORKING_SET_MIN_HARD_ENABLE | MM_WORKING_SET_MIN_HARD_DISABLE)) == (MM_WORKING_SET_MIN_HARD_ENABLE | MM_WORKING_SET_MIN_HARD_DISABLE)) {
                    return STATUS_INVALID_PARAMETER_2;
                }

                if ((Flags & (MM_WORKING_SET_MAX_HARD_ENABLE | MM_WORKING_SET_MAX_HARD_DISABLE)) == (MM_WORKING_SET_MAX_HARD_ENABLE | MM_WORKING_SET_MAX_HARD_DISABLE)) {
                    return STATUS_INVALID_PARAMETER_2;
                }
            }

            if (!SeSinglePrivilegeCheck( SeIncreaseQuotaPrivilege, PreviousMode )) {
                return STATUS_ACCESS_DENIED;
            }

            return MmAdjustWorkingSetSizeEx (
                        ((PSYSTEM_FILECACHE_INFORMATION)SystemInformation)->MinimumWorkingSet,
                        ((PSYSTEM_FILECACHE_INFORMATION)SystemInformation)->MaximumWorkingSet,
                        TRUE,
                        TRUE,
                        Flags,
                        &IncreasePerformed);

            break;

            }

        case SystemDpcBehaviorInformation:
            {
                SYSTEM_DPC_BEHAVIOR_INFORMATION DpcInfo;
                //
                // If the system information buffer is not the correct length,
                // then return an error.
                //
                if (SystemInformationLength != sizeof(SYSTEM_DPC_BEHAVIOR_INFORMATION)) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                if (PreviousMode != KernelMode) {
                    //
                    // The caller's access mode is not kernel so check to ensure that
                    // the caller has the privilege to load a driver.
                    //

                    if (!SeSinglePrivilegeCheck( SeLoadDriverPrivilege, PreviousMode )) {
                        return STATUS_PRIVILEGE_NOT_HELD;
                    }
                }

                //
                // Exception handler for this routine will return the correct
                // error if this access fails.
                //
                DpcInfo = *(PSYSTEM_DPC_BEHAVIOR_INFORMATION)SystemInformation;

                //
                // Set the new DPC behavior variables
                //
                KiMaximumDpcQueueDepth = DpcInfo.DpcQueueDepth;
                KiMinimumDpcRate = DpcInfo.MinimumDpcRate;
                KiAdjustDpcThreshold = DpcInfo.AdjustDpcThreshold;
                KiIdealDpcRate = DpcInfo.IdealDpcRate;
            }
            break;

        case SystemSessionCreate:
            {

                //
                // Creation of a session space.
                //

                ULONG SessionId;

                //
                // If the system information buffer is not the correct length,
                // then return an error.
                //

                if (SystemInformationLength != sizeof(ULONG)) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                if (PreviousMode != KernelMode) {

                    //
                    // The caller's access mode is not kernel so check to
                    // ensure that the caller has the privilege to load
                    // a driver.
                    //

                    if (!SeSinglePrivilegeCheck (SeLoadDriverPrivilege, PreviousMode)) {
                        return STATUS_PRIVILEGE_NOT_HELD;
                    }

                    try {
                        ProbeForWriteUlong((PULONG)SystemInformation);
                    }
                    except (EXCEPTION_EXECUTE_HANDLER) {
                        return GetExceptionCode();
                    }
                }

                //
                // Create a session space in the current process.
                //

                Status = MmSessionCreate (&SessionId);

                if (NT_SUCCESS(Status)) {
                    if (PreviousMode != KernelMode) {
                        try {
                            *(PULONG)SystemInformation = SessionId;
                        }
                        except (EXCEPTION_EXECUTE_HANDLER) {
                            return GetExceptionCode();
                        }
                    }
                    else {
                        *(PULONG)SystemInformation = SessionId;
                    }
                }

                return Status;
            }
            break;

        case SystemSessionDetach:
            {
                ULONG SessionId;

                //
                // If the system information buffer is not the correct length,
                // then return an error.
                //

                if (SystemInformationLength != sizeof(ULONG)) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                if (PreviousMode != KernelMode) {

                    //
                    // The caller's access mode is not kernel so check to
                    // ensure that the caller has the privilege to load
                    // a driver.
                    //

                    if (!SeSinglePrivilegeCheck( SeLoadDriverPrivilege, PreviousMode )) {
                        return STATUS_PRIVILEGE_NOT_HELD;
                    }

                    try {
                        ProbeForRead ((PVOID)SystemInformation,
                                      sizeof(ULONG),
                                      sizeof(ULONG));

                        SessionId = *(PULONG)SystemInformation;
                    }
                    except (EXCEPTION_EXECUTE_HANDLER) {
                        return GetExceptionCode();
                    }
                }
                else {
                    SessionId = *(PULONG)SystemInformation;
                }

                //
                // Detach the current process from a session space
                // if it has one.
                //

                Status = MmSessionDelete (SessionId);

                return Status;
            }
            break;

        case SystemCrashDumpStateInformation:

            //
            // All this system information does when you set it is trigger a
            // reconfigurating of the current crashdump state based on the
            // registry.
            //
            Status = IoConfigureCrashDump(CrashDumpReconfigure);

            break;

        case SystemPerformanceTraceInformation:
            Status = STATUS_INVALID_INFO_CLASS;
            break;

        case SystemVerifierThunkExtend:

            if (PreviousMode != KernelMode) {

                //
                // The caller's access mode is not kernel so fail.
                // Only device drivers can call this.
                //

                return STATUS_PRIVILEGE_NOT_HELD;
            }

            Status = MmAddVerifierThunks (SystemInformation,
                                          SystemInformationLength);

            break;

        case SystemVerifierInformation:

            if (!SeSinglePrivilegeCheck (SeDebugPrivilege, PreviousMode)) {
                return STATUS_ACCESS_DENIED;
            }

            Status = MmSetVerifierInformation (SystemInformation,
                                               SystemInformationLength);

            break;

        case SystemVerifierAddDriverInformation:
        case SystemVerifierRemoveDriverInformation:

            {
                UNICODE_STRING Image;
                PUNICODE_STRING ImagePointer;
                PWSTR Buffer;

                //
                // If the system information buffer is not the correct length,
                // then return an error.
                //

                if (SystemInformationLength != sizeof( UNICODE_STRING ) ) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }

                Buffer = NULL;

                if (PreviousMode != KernelMode) {

                    //
                    // The caller's access mode is not kernel so check to ensure
                    // the caller has the privilege to add a verifier entry.
                    //

                    if (!SeSinglePrivilegeCheck( SeDebugPrivilege, PreviousMode )) {
                        return STATUS_PRIVILEGE_NOT_HELD;
                    }

                    try {
                        Image = *(PUNICODE_STRING)SystemInformation;

                        //
                        // Guard against overflow.
                        //

                        if (Image.Length > Image.MaximumLength) {
                            Image.Length = Image.MaximumLength;
                        }
                        if (Image.Length == 0) {
                            return STATUS_NO_MEMORY;
                        }

                        ProbeForRead(Image.Buffer, Image.Length, sizeof(UCHAR));

                        Buffer = ExAllocatePoolWithTag(PagedPool, Image.Length, 'ofnI');
                        if ( !Buffer ) {
                            return STATUS_NO_MEMORY;
                        }

                        RtlCopyMemory(Buffer, Image.Buffer, Image.Length);
                        Image.Buffer = Buffer;
                        Image.MaximumLength = Image.Length;
                    }
                    except(EXCEPTION_EXECUTE_HANDLER) {
                        if ( Buffer ) {
                            ExFreePool(Buffer);
                        }
                        return GetExceptionCode();
                    }
                    ImagePointer = &Image;
                }
                else {
                    ImagePointer = (PUNICODE_STRING)SystemInformation;
                }

                switch (SystemInformationClass) {
                    case SystemVerifierAddDriverInformation:
                        Status = MmAddVerifierEntry (ImagePointer);
                        break;
                    case SystemVerifierRemoveDriverInformation:
                        Status = MmRemoveVerifierEntry (ImagePointer);
                        break;
                    default:
                        Status = STATUS_INVALID_INFO_CLASS;
                        break;
                }

                if (Buffer) {
                    ExFreePool(Buffer);
                }
            }

            break;

        case SystemMirrorMemoryInformation:
            Status = MmCreateMirror ();
            break;

        case SystemPrefetcherInformation:

            Status = CcPfSetPrefetcherInformation(SystemInformationClass,
                                                  SystemInformation,
                                                  SystemInformationLength,
                                                  PreviousMode
                                                  );
            break;

        case SystemComPlusPackage:

            if (SystemInformationLength != sizeof( ULONG ) ) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            Status = ExpUpdateComPlusPackage (*(PULONG)SystemInformation);
            if (NT_SUCCESS (Status)) {
                SharedUserData->ComPlusPackage = *(PULONG)SystemInformation;
            }

            break;

        case SystemHotpatchInformation:
            Status = ExApplyCodePatch( SystemInformation,
                                       SystemInformationLength
                                       );
            break;

        case SystemWow64SharedInformation:
            
            //
            // If the system information buffer is not the correct length,
            // then return an error.
            //

#if defined(_WIN64)

            if (SystemInformationLength != sizeof (SharedUserData->Wow64SharedInformation)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            if (PreviousMode != KernelMode) {

                //
                // The caller's access mode is not kernel so check to 
                // ensure that the caller has the privilege to load
                // a driver.
                //

                if (!SeSinglePrivilegeCheck (SeLoadDriverPrivilege, PreviousMode)) {
                    return STATUS_PRIVILEGE_NOT_HELD;
                }

                try {
                    ProbeForRead (SystemInformation, 
                                  SystemInformationLength, 
                                  sizeof (UCHAR));

                    if (SharedUserData->Wow64SharedInformation[0] == 0) {
                        RtlCopyMemory (&SharedUserData->Wow64SharedInformation[0],
                                       SystemInformation,
                                       SystemInformationLength);

                        KeUserPopEntrySListEndWow64 =
                            UlongToPtr (SharedUserData->Wow64SharedInformation[SharedNtdll32ExpInterlockedPopEntrySListEnd]);

                        KeUserPopEntrySListFaultWow64 =
                            UlongToPtr (SharedUserData->Wow64SharedInformation[SharedNtdll32ExpInterlockedPopEntrySListFault]);

                        KeUserPopEntrySListResumeWow64 =
                            UlongToPtr (SharedUserData->Wow64SharedInformation[SharedNtdll32ExpInterlockedPopEntrySListResume]);
                    }

                } except (EXCEPTION_EXECUTE_HANDLER) {
                    return GetExceptionCode();
                }
            }

#else

            return STATUS_NOT_IMPLEMENTED;

#endif

            break;

        case SystemWatchdogTimerHandler:

            {
                PSYSTEM_WATCHDOG_HANDLER_INFORMATION WdHandlerInfo = (PSYSTEM_WATCHDOG_HANDLER_INFORMATION) SystemInformation;

                //
                // Caller must be kernel mode with the proper parameters
                //

                if (PreviousMode != KernelMode || SystemInformation == NULL || SystemInformationLength != sizeof(SYSTEM_WATCHDOG_HANDLER_INFORMATION)) {
                    ExRaiseStatus (STATUS_INVALID_PARAMETER);
                }

                ExpWdHandler = WdHandlerInfo->WdHandler;
                ExpWdHandlerContext = WdHandlerInfo->Context;
            }

            break;

        case SystemWatchdogTimerInformation:

            {
                PSYSTEM_WATCHDOG_TIMER_INFORMATION WdTimerInfo = (PSYSTEM_WATCHDOG_TIMER_INFORMATION) SystemInformation;

                //
                // Caller must be kernel mode with the proper parameters
                //

                if (PreviousMode != KernelMode || SystemInformation == NULL || SystemInformationLength != sizeof(SYSTEM_WATCHDOG_TIMER_INFORMATION)) {
                    ExRaiseStatus (STATUS_INVALID_PARAMETER);
                }

                if (ExpWdHandler == NULL) {

                    Status = STATUS_NOT_IMPLEMENTED;

                } else {

                    switch (WdTimerInfo->WdInfoClass) {
                        case WdInfoTimeoutValue:
                            Status = ExpWdHandler( WdActionSetTimeoutValue, ExpWdHandlerContext, &WdTimerInfo->DataValue, FALSE );
                            break;

                        case WdInfoResetTimer:
                            Status = ExpWdHandler( WdActionResetTimer, ExpWdHandlerContext, NULL, FALSE );
                            break;

                        case WdInfoStopTimer:
                            Status = ExpWdHandler( WdActionStopTimer, ExpWdHandlerContext, NULL, FALSE );
                            break;

                        case WdInfoStartTimer:
                            Status = ExpWdHandler( WdActionStartTimer, ExpWdHandlerContext, NULL, FALSE );
                            break;

                        case WdInfoTriggerAction:
                            Status = ExpWdHandler( WdActionSetTriggerAction, ExpWdHandlerContext, &WdTimerInfo->DataValue, FALSE );
                            break;

                        default:
                            Status = STATUS_INVALID_PARAMETER;
                            break;
                    }
                }
            }

            break;

        case SystemRegisterFirmwareTableInformationHandler:

            Status = ExpRegisterFirmwareTableInformationHandler(SystemInformation,
                                                                SystemInformationLength,
                                                                PreviousMode);
            break;

        default:
            Status = STATUS_INVALID_INFO_CLASS;
            break;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    return Status;
}

NTSTATUS
ExLockUserBuffer (
    __inout_bcount(Length) PVOID Buffer,
    __in ULONG Length,
    __in KPROCESSOR_MODE ProbeMode,
    __in LOCK_OPERATION LockMode,
    __deref_out PVOID *LockedBuffer,
    __deref_out PVOID *LockVariable
    )

/*++

Routine Description:

    Wrapper for MmProbeAndLockPages.  Creates an MDL and locks the
    specified buffer with that MDL.        

Arguments:

    Buffer - pointer to the buffer to be locked.
    Length - size of the buffer to be locked.
    ProbeMode - processor mode for doing the probe in MmProbeAndLockPages.
    LockMode - the mode the pages should be locked for.
    LockedBuffer - returns a pointer to the locked buffer for use by the 
                   caller.
    LockVariable - returns a context pointer.  This must be passed into
                   ExUnlockUserBuffer when complete so the MDL can be freed.                   

Return Value:

    Returns one of the following status codes:

        STATUS_SUCCESS - Normal, successful completion.

        STATUS_ACCESS_VIOLATION - The buffer is not accessible with the 
                 specified LockMode.
                 
        STATUS_INSUFFICIENT_RESOURCES - not enough memory to allocate the MDL.                                           
                 

--*/

{
    PMDL Mdl;
    SIZE_T MdlSize;

    //
    // It is the caller's responsibility to ensure zero cannot be passed in.
    //

    ASSERT (Length != 0);

    *LockedBuffer = NULL;
    *LockVariable = NULL;

    //
    // Allocate an MDL to map the request.
    //

    MdlSize = MmSizeOfMdl( Buffer, Length );
    Mdl = ExAllocatePoolWithQuotaTag (NonPagedPool | POOL_QUOTA_FAIL_INSTEAD_OF_RAISE,
                                      MdlSize,
                                      'ofnI');
    if (Mdl == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Initialize MDL for request.
    //

    MmInitializeMdl(Mdl, Buffer, Length);

    try {

        MmProbeAndLockPages (Mdl, ProbeMode, LockMode);

    } except (EXCEPTION_EXECUTE_HANDLER) {

        ExFreePool (Mdl);

        return GetExceptionCode();
    }

    Mdl->MdlFlags |= MDL_MAPPING_CAN_FAIL;
    *LockedBuffer = MmGetSystemAddressForMdl (Mdl);
    if (*LockedBuffer == NULL) {
        ExUnlockUserBuffer (Mdl);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    *LockVariable = Mdl;
    return STATUS_SUCCESS;
}

VOID
ExUnlockUserBuffer (
    __inout PVOID LockVariable
    )

{
    MmUnlockPages ((PMDL)LockVariable);
    ExFreePool ((PMDL)LockVariable);
    return;
}

NTSTATUS
ExpGetProcessInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length OPTIONAL,
    IN PULONG SessionId OPTIONAL,
    IN BOOLEAN ExtendedInformation
    )

/*++

Routine Description:

    This function returns information about all the processes and
    threads in the system.

Arguments:

    SystemInformation - A pointer to a buffer which receives the specified
        information.

    SystemInformationLength - Specifies the length in bytes of the system
        information buffer.

    Length - An optional pointer which, if specified, receives the
        number of bytes placed in the system information buffer.

    SessionId - Session Id.

    ExtendedInformation - TRUE if extended information (e.g., Process PDE) is needed.

Environment:

    Kernel mode.

    This routine could be made PAGELK but it is a high frequency routine
    so it is actually better to keep it nonpaged to avoid bringing in the
    entire PAGELK section.

Return Value:

    Returns one of the following status codes:

        STATUS_SUCCESS - normal, successful completion.

        STATUS_INVALID_INFO_CLASS - The SystemInformationClass parameter
            did not specify a valid value.

        STATUS_INFO_LENGTH_MISMATCH - The value of the SystemInformationLength
            parameter did not match the length required for the information
            class requested by the SystemInformationClass parameter.

        STATUS_ACCESS_VIOLATION - Either the SystemInformation buffer pointer
            or the Length pointer value specified an invalid address.

        STATUS_WORKING_SET_QUOTA - The process does not have sufficient
            working set to lock the specified output structure in memory.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources exist
            for this request to complete.

--*/

{
    KLOCK_QUEUE_HANDLE LockHandle;
    PEPROCESS Process = NULL;
    PETHREAD Thread;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PVOID ThreadInfo;
    ULONG ThreadInfoSize;
    PLIST_ENTRY NextThread;
    PVOID MappedAddress;
    PVOID LockVariable;
    ULONG ProcessSessionId;
    ULONG TotalSize = 0;
    ULONG NextEntryOffset = 0;
    PUCHAR Src;
    PWCHAR SrcW;
    PWSTR Dst;
    ULONG n, nc;
    NTSTATUS status = STATUS_SUCCESS, status1;
    PUNICODE_STRING pImageFileName;

    if (ARGUMENT_PRESENT(Length)) {
        *Length = 0;
    }

    if (SystemInformationLength > 0) {
        status1 = ExLockUserBuffer (SystemInformation,
                                    SystemInformationLength,
                                    KeGetPreviousMode(),
                                    IoWriteAccess,
                                    &MappedAddress,
                                    &LockVariable);

        if (!NT_SUCCESS(status1)) {
            return status1;
        }

    } else {

        //
        // This indicates the caller just wants to know the size of the
        // buffer to allocate but is not prepared to accept any data content
        // in this instance.
        //

        MappedAddress = NULL;
        LockVariable = NULL;
    }

    if (ExtendedInformation) {
        ThreadInfoSize = sizeof(SYSTEM_EXTENDED_THREAD_INFORMATION);

    } else {
        ThreadInfoSize = sizeof(SYSTEM_THREAD_INFORMATION);
    }

    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) MappedAddress;

    try {

        //
        // Do the idle process first then all the other processes.
        //

        for  (Process = PsIdleProcess;
              Process != NULL;
              Process = PsGetNextProcess ((Process == PsIdleProcess) ? NULL : Process)) {

            //
            // If the process is marked as exiting, the executive process has
            // no active threads, the kernel process has no threads, and the
            // kernel process has been signaled, then skip the process.
            //
            // N.B. It is safe to examine the kernel thread list without a
            //      lock since no list pointers are dereferenced.
            //

            if (((Process->Flags & PS_PROCESS_FLAGS_PROCESS_EXITING) != 0) &&
                (Process->Pcb.Header.SignalState != 0) &&
                (Process->ActiveThreads == 0) &&
                (IsListEmpty(&Process->Pcb.ThreadListHead) == TRUE)) {

                continue;
            }

            if (ARGUMENT_PRESENT(SessionId) && (Process == PsIdleProcess)) {
                continue;
            }

            ProcessSessionId = MmGetSessionId (Process);
            if ((ARGUMENT_PRESENT(SessionId)) &&
                (ProcessSessionId != *SessionId)) {
                continue;
            }

            ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
                            ((PUCHAR)MappedAddress + TotalSize);

            NextEntryOffset = sizeof(SYSTEM_PROCESS_INFORMATION);
            TotalSize += sizeof(SYSTEM_PROCESS_INFORMATION);
            if (TotalSize > SystemInformationLength) {
                status = STATUS_INFO_LENGTH_MISMATCH;
                if (ARGUMENT_PRESENT(Length) == FALSE) {
                    leave;
                }

            } else {

                //
                // Get information for each process.
                //

                ExpCopyProcessInfo (ProcessInfo, Process, ExtendedInformation);
                ProcessInfo->NumberOfThreads = 0;
                ProcessInfo->NextEntryOffset = 0;

                //
                // Store the Remote Terminal SessionId
                //

                ProcessInfo->SessionId = ProcessSessionId;
                ProcessInfo->ImageName.Buffer = NULL;
                ProcessInfo->ImageName.Length = 0;
                ProcessInfo->ImageName.MaximumLength = 0;
                if (Process == PsIdleProcess) {

                    //
                    // Since Idle process and system process share the same
                    // object table, zero out idle processes handle count to
                    // reduce confusion
                    //
                    // Idle Process always has SessionId 0
                    //

                    ProcessInfo->HandleCount = 0;
                    ProcessInfo->SessionId = 0;
                }
            }

            //
            // Raise IRQL to SYNCH_LEVEL, acquire the kernel process lock, and
            // get information for each thread.
            //

            ThreadInfo = (PVOID)(ProcessInfo + 1);
            KeAcquireInStackQueuedSpinLockRaiseToSynch(&Process->Pcb.ProcessLock,
                                                       &LockHandle);

            NextThread = Process->Pcb.ThreadListHead.Flink;
            while (NextThread != &Process->Pcb.ThreadListHead) {
                NextEntryOffset += ThreadInfoSize;
                TotalSize += ThreadInfoSize;

                if (TotalSize > SystemInformationLength) {
                    status = STATUS_INFO_LENGTH_MISMATCH;
                    if (ARGUMENT_PRESENT(Length) == FALSE) {
                        KeReleaseInStackQueuedSpinLock(&LockHandle);
                        leave;
                    }

                } else {
                    Thread = (PETHREAD)(CONTAINING_RECORD(NextThread,
                                                          KTHREAD,
                                                          ThreadListEntry));

                    //
                    // Lock dispatcher database to get atomic view of thread
                    // attributes.
                    //

                    KiLockDispatcherDatabaseAtSynchLevel();
                    ExpCopyThreadInfo (ThreadInfo, Thread, ExtendedInformation);
                    KiUnlockDispatcherDatabaseFromSynchLevel();
                    ProcessInfo->NumberOfThreads += 1;
                    ThreadInfo = (PCHAR) ThreadInfo + ThreadInfoSize;
                }

                NextThread = NextThread->Flink;
            }

            //
            // Unlock kernel process lock and lower IRQL to its previous value.
            //

            KeReleaseInStackQueuedSpinLock(&LockHandle);

            //
            // Get the image name.
            //

            if (Process != PsIdleProcess) {

                //
                // Try to use the real image name if we can that not limited to 16 characters
                //

                Dst = (PWSTR)(ThreadInfo);
                status1 = SeLocateProcessImageName (Process, &pImageFileName);
                if (NT_SUCCESS (status1)) {
                    n = pImageFileName->Length;
                    if (n == 0) {
                        ExFreePool (pImageFileName);
                    }

                } else {
                    n = 0;
                }

                if (n) {
                    SrcW = pImageFileName->Buffer + n / sizeof (WCHAR);
                    while (SrcW != pImageFileName->Buffer) {
                        if (*--SrcW == L'\\') {
                            SrcW = SrcW + 1;
                            break;
                        }
                    }

                    nc = n - (ULONG)(SrcW -  pImageFileName->Buffer) * sizeof (WCHAR);
                    n = ROUND_UP (nc + 1, sizeof(LARGE_INTEGER));
                    TotalSize += n;
                    NextEntryOffset += n;
                    if (TotalSize > SystemInformationLength) {
                        status = STATUS_INFO_LENGTH_MISMATCH;
                        if (ARGUMENT_PRESENT(Length) == FALSE) {
                            ExFreePool (pImageFileName);
                            leave;
                        }
                    } else {
                        RtlCopyMemory (Dst, SrcW, nc);
                        Dst += nc / sizeof (WCHAR);
                        *Dst++ = L'\0';
                    }

                    ExFreePool (pImageFileName);

                } else {
                    Src = Process->ImageFileName;
                    n = (ULONG) strlen ((PCHAR)Src);
                    if (n != 0) {
                        n = ROUND_UP( ((n + 1) * sizeof( WCHAR )), sizeof(LARGE_INTEGER) );
                        TotalSize += n;
                        NextEntryOffset += n;
                        if (TotalSize > SystemInformationLength) {
                            status = STATUS_INFO_LENGTH_MISMATCH;
                            if (ARGUMENT_PRESENT(Length) == FALSE) {
                                leave;
                            }

                        } else {
                            WCHAR c;

                            while (1) {
                                c = (WCHAR)*Src++;
                                *Dst++ =  c;
                                if (c == L'\0') {
                                    break;
                                }
                            }
                        }
                    }
                }

                if (NT_SUCCESS (status)) {
                    ProcessInfo->ImageName.Length = (USHORT)((PCHAR)Dst -
                                                             (PCHAR)ThreadInfo - sizeof( UNICODE_NULL ));

                    ProcessInfo->ImageName.MaximumLength = (USHORT)n;

                    //
                    // Set the image name to point into the user's memory.
                    //

                    ProcessInfo->ImageName.Buffer = (PWSTR)
                                ((PCHAR)SystemInformation +
                                 ((PCHAR)(ThreadInfo) - (PCHAR)MappedAddress));
                }
            }

            //
            // Point to next process.
            //

            if (NT_SUCCESS (status)) {
                ProcessInfo->NextEntryOffset = NextEntryOffset;
            }
        }

        if (NT_SUCCESS(status)) {
            ProcessInfo->NextEntryOffset = 0;
        }

        if (ARGUMENT_PRESENT(Length)) {
            *Length = TotalSize;
        }

    } finally {

        if ((Process != NULL) && (Process != PsIdleProcess)) {
            PsQuitNextProcess (Process);
        }

        if (MappedAddress != NULL) {
            ExUnlockUserBuffer (LockVariable);
        }
    }

    return status;
}

NTSTATUS
ExGetSessionPoolTagInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length,
    IN PULONG SessionId OPTIONAL
    )

/*++

Routine Description:

    This function returns information about all the per-session pool tag
    information in the system.

Arguments:

    SystemInformation - A pointer to a buffer which receives the specified
                        information.

    SystemInformationLength - Specifies the length in bytes of the system
                              information buffer.

    Length - Receives the number of bytes placed (or would have been placed)
             in the system information buffer.

    SessionId - Session Id (-1 indicates enumerate all sessions).

Environment:

    Kernel mode.

Return Value:

    Returns one of the following status codes:

        STATUS_SUCCESS - normal, successful completion.

        STATUS_INVALID_INFO_CLASS - The SystemInformationClass parameter
            did not specify a valid value.

        STATUS_INFO_LENGTH_MISMATCH - The value of the SystemInformationLength
            parameter did not match the length required for the information
            class requested by the SystemInformationClass parameter.

        STATUS_ACCESS_VIOLATION - Either the SystemInformation buffer pointer
            or the Length pointer value specified an invalid address.

        STATUS_WORKING_SET_QUOTA - The process does not have sufficient
            working set to lock the specified output structure in memory.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources exist
            for this request to complete.

--*/

{
    KAPC_STATE ApcState;
    PVOID MappedAddress;
    PVOID OpaqueSession;
    PVOID LockVariable;
    ULONG TotalSize;
    ULONG NextEntryOffset;
    ULONG CurrentSessionId;
    ULONG Count;
    ULONG AbsoluteCount;
    NTSTATUS status;
    NTSTATUS status1;
    PSYSTEM_SESSION_POOLTAG_INFORMATION SessionPoolTagInfo;

    *Length = 0;
    TotalSize = 0;
    NextEntryOffset = 0;
    status = STATUS_SUCCESS;
    SessionPoolTagInfo = NULL;

    if (SystemInformationLength > 0) {

        status1 = ExLockUserBuffer (SystemInformation,
                                    SystemInformationLength,
                                    KeGetPreviousMode(),
                                    IoWriteAccess,
                                    &MappedAddress,
                                    &LockVariable);

        if (!NT_SUCCESS(status1)) {
            return status1;
        }

    }
    else {

        //
        // This indicates the caller just wants to know the size of the
        // buffer to allocate but is not prepared to accept any data content
        // in this instance.
        //

        MappedAddress = NULL;
        LockVariable = NULL;
    }

    for (OpaqueSession = MmGetNextSession (NULL);
         OpaqueSession != NULL;
         OpaqueSession = MmGetNextSession (OpaqueSession)) {

        SessionPoolTagInfo = (PSYSTEM_SESSION_POOLTAG_INFORMATION)
                                ((PUCHAR)MappedAddress + TotalSize);

        //
        // If a specific session was requested, only extract that one.
        //

        CurrentSessionId = MmGetSessionId (OpaqueSession);

        if ((*SessionId == 0xFFFFFFFF) || (CurrentSessionId == *SessionId)) {

            //
            // Attach to session now to perform operations...
            //

            if (NT_SUCCESS (MmAttachSession (OpaqueSession, &ApcState))) {

                //
                // Session is still alive so include it.
                //

                NextEntryOffset = sizeof (SYSTEM_SESSION_POOLTAG_INFORMATION);
                TotalSize += sizeof (SYSTEM_SESSION_POOLTAG_INFORMATION);

                if (TotalSize > SystemInformationLength) {

                    status = STATUS_INFO_LENGTH_MISMATCH;

                    //
                    // Get absolute size for this session, ignore status as
                    // we must return the one above.
                    //

                    ExGetSessionPoolTagInfo (NULL,
                                             0,
                                             &Count,
                                             &AbsoluteCount);
                }
                else {

                    //
                    // Get pool tagging information for each session.
                    //

                    status = ExGetSessionPoolTagInfo (
                                    SessionPoolTagInfo->TagInfo,
                                    SystemInformationLength - TotalSize + sizeof (SYSTEM_POOLTAG),
                                    &Count,
                                    &AbsoluteCount);

                    SessionPoolTagInfo->SessionId = CurrentSessionId;
                    SessionPoolTagInfo->Count = Count;

                    //
                    // Point to next session.
                    //

                    if (NT_SUCCESS (status)) {
                        NextEntryOffset += ((Count - 1) * sizeof (SYSTEM_POOLTAG));
                        SessionPoolTagInfo->NextEntryOffset = NextEntryOffset;
                    }
                }

                TotalSize += ((AbsoluteCount - 1) * sizeof (SYSTEM_POOLTAG));

                //
                // Detach from session.
                //

                MmDetachSession (OpaqueSession, &ApcState);
            }

            //
            // Bail if only this session was of interest.
            //

            if (*SessionId != 0xFFFFFFFF) {
                MmQuitNextSession (OpaqueSession);
                break;
            }
        }
    }

    if ((NT_SUCCESS (status)) && (SessionPoolTagInfo != NULL)) {
        SessionPoolTagInfo->NextEntryOffset = 0;
    }

    if (MappedAddress != NULL) {
        ExUnlockUserBuffer (LockVariable);
    }

    *Length = TotalSize;

    return status;
}

NTSTATUS
ExpGetProcessorPowerInformation (
    OUT PVOID   SystemInformation,
    IN  ULONG   SystemInformationLength,
    OUT PULONG  Length
    )
{
    KAFFINITY                           currentAffinity;
    KAFFINITY                           processors;
    KIRQL                               oldIrql;
    PKPRCB                              Prcb;
    PPROCESSOR_POWER_STATE              PState;
    PPROCESSOR_PERF_STATE               PerfStates;
    PSYSTEM_PROCESSOR_POWER_INFORMATION CallerPowerInfo;
    SYSTEM_PROCESSOR_POWER_INFORMATION  ProcessorPowerInfo;
    NTSTATUS Status;

    //
    // We will walk this pointer to store the user data...
    //
    CallerPowerInfo = (PSYSTEM_PROCESSOR_POWER_INFORMATION) SystemInformation;

    //
    // Check to see if we have the space for this
    //

    *Length = sizeof(SYSTEM_PROCESSOR_POWER_INFORMATION) * KeNumberProcessors;

    if (SystemInformationLength < sizeof(SYSTEM_PROCESSOR_POWER_INFORMATION) * KeNumberProcessors) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    //
    // Lock everything down
    //
    MmLockPageableSectionByHandle (ExPageLockHandle);

    //
    // Walk the list of processors
    //
    processors = KeActiveProcessors;
    currentAffinity = 1;
    while (processors) {


        processors &= ~currentAffinity;
        KeSetSystemAffinityThread(currentAffinity);
        currentAffinity <<= 1;


        //
        // Get the PRCB and PowerState information
        //
        Prcb = KeGetCurrentPrcb();
        PState = &(Prcb->PowerState);

        //
        // Raise to DPC level to synchronize access to the data structures
        //
        KeRaiseIrql (DISPATCH_LEVEL, &oldIrql);

        PerfStates = PState->PerfStates;

        //
        // Grab the data that we care about
        //
        ProcessorPowerInfo.CurrentFrequency            = PState->CurrentThrottle;
        ProcessorPowerInfo.LastBusyFrequency           = PState->LastBusyPercentage;
        ProcessorPowerInfo.LastAdjustedBusyFrequency   = PState->LastAdjustedBusyPercentage;
        ProcessorPowerInfo.LastC3Frequency             = PState->LastC3Percentage;
        ProcessorPowerInfo.ProcessorMinThrottle        = PState->ProcessorMinThrottle;
        ProcessorPowerInfo.ProcessorMaxThrottle        = PState->ProcessorMaxThrottle;
        ProcessorPowerInfo.ErrorCount                  = PState->ErrorCount;
        ProcessorPowerInfo.RetryCount                  = PState->RetryCount;

        //
        // Do we have any kind of PerfStates?
        //
        if (PerfStates) {

            ProcessorPowerInfo.ThermalLimitFrequency       = PerfStates[PState->ThermalThrottleIndex].PercentFrequency;
            ProcessorPowerInfo.ConstantThrottleFrequency   = PerfStates[PState->KneeThrottleIndex].PercentFrequency;
            ProcessorPowerInfo.DegradedThrottleFrequency   = PerfStates[PState->ThrottleLimitIndex].PercentFrequency;

        } else {

            ProcessorPowerInfo.ThermalLimitFrequency       = 0;
            ProcessorPowerInfo.ConstantThrottleFrequency   = 0;
            ProcessorPowerInfo.DegradedThrottleFrequency   = 0;

        }

        ProcessorPowerInfo.CurrentFrequencyTime        =
            UInt32x32To64(
                (Prcb->KernelTime + Prcb->UserTime - PState->PerfTickCount),
                KeMaximumIncrement
                );
        ProcessorPowerInfo.CurrentProcessorTime        =
            UInt32x32To64(
                Prcb->KernelTime + Prcb->UserTime,
                KeMaximumIncrement
                );
        ProcessorPowerInfo.CurrentProcessorIdleTime    =
            UInt32x32To64( Prcb->IdleThread->KernelTime, KeMaximumIncrement );
        ProcessorPowerInfo.LastProcessorTime           =
            UInt32x32To64( PState->LastKernelUserTime, KeMaximumIncrement );
        ProcessorPowerInfo.LastProcessorIdleTime       =
            UInt32x32To64( PState->PerfIdleTime, KeMaximumIncrement );

        ProcessorPowerInfo.PromotionCount              = PState->PromotionCount;
        ProcessorPowerInfo.DemotionCount               = PState->DemotionCount;
        ProcessorPowerInfo.NumberOfFrequencies         = PState->PerfStatesCount;

        //
        // Return to the original level (should be IRQL 0)
        //
        KeLowerIrql (oldIrql);

        //
        // Copy the data to the correct place
        //
        try {
            RtlCopyMemory(
                CallerPowerInfo,
                &ProcessorPowerInfo,
                sizeof(SYSTEM_PROCESSOR_POWER_INFORMATION)
                );
        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode ();
            goto exit_revert_and_unlock;
        }

        //
        // Point to the next structure element
        //
        CallerPowerInfo++;

    }

    Status = STATUS_SUCCESS;

exit_revert_and_unlock:

    //
    // Revert to the original affinity
    //
    KeRevertToUserAffinityThread();

    //
    // Unlock everything

    MmUnlockPageableImageSection(ExPageLockHandle);

    return Status;
}

NTSTATUS
ExpGetProcessorIdleInformation (
    OUT PVOID   SystemInformation,
    IN  ULONG   SystemInformationLength,
    OUT PULONG  Length
    )
{
    KAFFINITY                           currentAffinity;
    KAFFINITY                           processors;
    KIRQL                               oldIrql;
    LARGE_INTEGER                       PerfFrequency;
    PKPRCB                              Prcb;
    PPROCESSOR_POWER_STATE              PState;
    PSYSTEM_PROCESSOR_IDLE_INFORMATION  CallerIdleInfo;
    SYSTEM_PROCESSOR_IDLE_INFORMATION   ProcessorIdleInfo;
    NTSTATUS Status;

    //
    // We will walk this pointer to store the user data...
    //
    CallerIdleInfo = (PSYSTEM_PROCESSOR_IDLE_INFORMATION) SystemInformation;

    *Length = sizeof(SYSTEM_PROCESSOR_IDLE_INFORMATION) * KeNumberProcessors;

    //
    // Check to see if we have the space for this
    //
    if (SystemInformationLength < sizeof(SYSTEM_PROCESSOR_IDLE_INFORMATION) * KeNumberProcessors) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    //
    // We need to know what frequency the perf counters are running at
    //
    KeQueryPerformanceCounter(&PerfFrequency);

    //
    // Lock everything down
    //
    MmLockPageableSectionByHandle (ExPageLockHandle);

    //
    // Walk the list of processors
    //
    processors = KeActiveProcessors;
    currentAffinity = 1;
    while (processors) {

        processors &= ~currentAffinity;
        KeSetSystemAffinityThread(currentAffinity);
        currentAffinity <<= 1;

        //
        // Get the PRCB and PowerState information
        //
        Prcb = KeGetCurrentPrcb();
        PState = &(Prcb->PowerState);

        //
        // Raise to DPC level to synchronize access to the data structures
        //
        KeRaiseIrql (DISPATCH_LEVEL, &oldIrql);


        //
        // Grab the data that we care about
        //
        ProcessorIdleInfo.IdleTime = UInt32x32To64(Prcb->IdleThread->KernelTime,KeMaximumIncrement);

        //
        // The Cx times are kept in units of the same frequency as KeQueryPerformanceCounter
        // This needs to be converted to standard 100ns units.
        //
        ProcessorIdleInfo.C1Time =  (PState->TotalIdleStateTime[0]*1000)/(PerfFrequency.QuadPart/10000);
        ProcessorIdleInfo.C2Time =  (PState->TotalIdleStateTime[1]*1000)/(PerfFrequency.QuadPart/10000);
        ProcessorIdleInfo.C3Time =  (PState->TotalIdleStateTime[2]*1000)/(PerfFrequency.QuadPart/10000);

        ProcessorIdleInfo.C1Transitions = PState->TotalIdleTransitions[0];
        ProcessorIdleInfo.C2Transitions = PState->TotalIdleTransitions[1];
        ProcessorIdleInfo.C3Transitions = PState->TotalIdleTransitions[2];

        //
        // Return to the original level (should be IRQL 0)
        //
        KeLowerIrql (oldIrql);

        //
        // Copy the data to the correct place
        //
        try {
            RtlCopyMemory(
                CallerIdleInfo,
                &ProcessorIdleInfo,
                sizeof(SYSTEM_PROCESSOR_IDLE_INFORMATION)
                );
        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode ();
            goto exit_revert_and_unlock;
        }

        //
        // Point to the next structure element
        //
        CallerIdleInfo++;
    }

    Status = STATUS_SUCCESS;

exit_revert_and_unlock:

    //
    // Revert to the original affinity
    //
    KeRevertToUserAffinityThread();

    //
    // Unlock everything
    //
    MmUnlockPageableImageSection(ExPageLockHandle);

    return Status;
}

VOID
ExpCopyProcessInfo (
    IN PSYSTEM_PROCESS_INFORMATION ProcessInfo,
    IN PEPROCESS Process,
    IN BOOLEAN ExtendedInformation
    )

{

    KPROCESS_VALUES Values;

    PAGED_CODE();

    ProcessInfo->HandleCount = ObGetProcessHandleCount (Process);
    ProcessInfo->CreateTime = Process->CreateTime;
    ProcessInfo->BasePriority = Process->Pcb.BasePriority;
    ProcessInfo->UniqueProcessId = Process->UniqueProcessId;
    ProcessInfo->InheritedFromUniqueProcessId = Process->InheritedFromUniqueProcessId;
    ProcessInfo->PeakVirtualSize = Process->PeakVirtualSize;
    ProcessInfo->VirtualSize = Process->VirtualSize;
    ProcessInfo->PageFaultCount = Process->Vm.PageFaultCount;
    ProcessInfo->PeakWorkingSetSize = ((SIZE_T)Process->Vm.PeakWorkingSetSize) << PAGE_SHIFT;
    ProcessInfo->WorkingSetSize = ((SIZE_T)Process->Vm.WorkingSetSize) << PAGE_SHIFT;
    ProcessInfo->QuotaPeakPagedPoolUsage =
                            Process->QuotaPeak[PsPagedPool];

    ProcessInfo->QuotaPagedPoolUsage = Process->QuotaUsage[PsPagedPool];
    ProcessInfo->QuotaPeakNonPagedPoolUsage =
                            Process->QuotaPeak[PsNonPagedPool];

    ProcessInfo->QuotaNonPagedPoolUsage =
                            Process->QuotaUsage[PsNonPagedPool];

    ProcessInfo->PagefileUsage = Process->QuotaUsage[PsPageFile] << PAGE_SHIFT;
    ProcessInfo->PeakPagefileUsage = Process->QuotaPeak[PsPageFile] << PAGE_SHIFT;
    ProcessInfo->PrivatePageCount = Process->CommitCharge << PAGE_SHIFT;

    //
    // Query the process kernel runtime, user runtime, and I/O statistics.
    //

    KeQueryValuesProcess(&Process->Pcb, &Values);
    ProcessInfo->UserTime.QuadPart = Values.UserTime;
    ProcessInfo->KernelTime.QuadPart = Values.KernelTime;
    ProcessInfo->ReadOperationCount.QuadPart = Values.ReadOperationCount;
    ProcessInfo->WriteOperationCount.QuadPart = Values.WriteOperationCount;
    ProcessInfo->OtherOperationCount.QuadPart = Values.OtherOperationCount;
    ProcessInfo->ReadTransferCount.QuadPart = Values.ReadTransferCount;
    ProcessInfo->WriteTransferCount.QuadPart = Values.WriteTransferCount;
    ProcessInfo->OtherTransferCount.QuadPart = Values.OtherTransferCount;
    if (ExtendedInformation) {
        ProcessInfo->PageDirectoryBase = MmGetDirectoryFrameFromProcess(Process);
    }
}

VOID
ExpCopyThreadInfo (
    IN PVOID ThreadInfoBuffer,
    IN PETHREAD Thread,
    IN BOOLEAN ExtendedInformation
    )

/*++

Routine Description:

    This function returns information about the specified thread.

Arguments:

    ThreadInfoBuffer - A pointer to a buffer which receives the specified
        information.

    Thread - Supplies a pointer to the desired thread.

    ExtendedInformation - TRUE if extended thread information is needed.

Environment:

    Kernel mode.  The dispatcher lock is held.

    This routine could be made PAGELK but it is a high frequency routine
    so it is actually better to keep it nonpaged to avoid bringing in the
    entire PAGELK section.

Return Value:

    None.

--*/

{
    PSYSTEM_THREAD_INFORMATION ThreadInfo;
    ThreadInfo = (PSYSTEM_THREAD_INFORMATION) ThreadInfoBuffer;

    ThreadInfo->KernelTime.QuadPart = UInt32x32To64(Thread->Tcb.KernelTime,
                                                    KeMaximumIncrement);

    ThreadInfo->UserTime.QuadPart = UInt32x32To64(Thread->Tcb.UserTime,
                                                  KeMaximumIncrement);

    ThreadInfo->CreateTime.QuadPart = PS_GET_THREAD_CREATE_TIME (Thread);
    ThreadInfo->WaitTime = Thread->Tcb.WaitTime;
    ThreadInfo->ClientId = Thread->Cid;
    ThreadInfo->ThreadState = Thread->Tcb.State;
    ThreadInfo->WaitReason = Thread->Tcb.WaitReason;
    ThreadInfo->Priority = Thread->Tcb.Priority;
    ThreadInfo->BasePriority = Thread->Tcb.BasePriority;
    ThreadInfo->ContextSwitches = Thread->Tcb.ContextSwitches;
    ThreadInfo->StartAddress = Thread->StartAddress;

    if (ExtendedInformation) {
        PSYSTEM_EXTENDED_THREAD_INFORMATION ExtendedThreadInfo;

        ExtendedThreadInfo = (PSYSTEM_EXTENDED_THREAD_INFORMATION) ThreadInfo;

        ExtendedThreadInfo->StackBase = Thread->Tcb.StackBase;
        ExtendedThreadInfo->StackLimit = Thread->Tcb.StackLimit;
        if (Thread->LpcReceivedMsgIdValid) {
            ExtendedThreadInfo->Win32StartAddress = 0;
        } else {
            ExtendedThreadInfo->Win32StartAddress = Thread->Win32StartAddress;
        }
        ExtendedThreadInfo->Reserved1 = 0;
        ExtendedThreadInfo->Reserved2 = 0;
        ExtendedThreadInfo->Reserved3 = 0;
        ExtendedThreadInfo->Reserved4 = 0;
    }

}

#if defined(_X86_)

extern ULONG ExVdmOpcodeDispatchCounts[256];
extern ULONG VdmBopCount;
extern ULONG ExVdmSegmentNotPresent;

#pragma alloc_text(PAGE, ExpGetInstemulInformation)

NTSTATUS
ExpGetInstemulInformation (
    OUT PSYSTEM_VDM_INSTEMUL_INFO Info
    )
{
    SYSTEM_VDM_INSTEMUL_INFO LocalInfo;

    LocalInfo.VdmOpcode0F       = ExVdmOpcodeDispatchCounts[VDM_INDEX_0F];
    LocalInfo.OpcodeESPrefix    = ExVdmOpcodeDispatchCounts[VDM_INDEX_ESPrefix];
    LocalInfo.OpcodeCSPrefix    = ExVdmOpcodeDispatchCounts[VDM_INDEX_CSPrefix];
    LocalInfo.OpcodeSSPrefix    = ExVdmOpcodeDispatchCounts[VDM_INDEX_SSPrefix];
    LocalInfo.OpcodeDSPrefix    = ExVdmOpcodeDispatchCounts[VDM_INDEX_DSPrefix];
    LocalInfo.OpcodeFSPrefix    = ExVdmOpcodeDispatchCounts[VDM_INDEX_FSPrefix];
    LocalInfo.OpcodeGSPrefix    = ExVdmOpcodeDispatchCounts[VDM_INDEX_GSPrefix];
    LocalInfo.OpcodeOPER32Prefix= ExVdmOpcodeDispatchCounts[VDM_INDEX_OPER32Prefix];
    LocalInfo.OpcodeADDR32Prefix= ExVdmOpcodeDispatchCounts[VDM_INDEX_ADDR32Prefix];
    LocalInfo.OpcodeINSB        = ExVdmOpcodeDispatchCounts[VDM_INDEX_INSB];
    LocalInfo.OpcodeINSW        = ExVdmOpcodeDispatchCounts[VDM_INDEX_INSW];
    LocalInfo.OpcodeOUTSB       = ExVdmOpcodeDispatchCounts[VDM_INDEX_OUTSB];
    LocalInfo.OpcodeOUTSW       = ExVdmOpcodeDispatchCounts[VDM_INDEX_OUTSW];
    LocalInfo.OpcodePUSHF       = ExVdmOpcodeDispatchCounts[VDM_INDEX_PUSHF];
    LocalInfo.OpcodePOPF        = ExVdmOpcodeDispatchCounts[VDM_INDEX_POPF];
    LocalInfo.OpcodeINTnn       = ExVdmOpcodeDispatchCounts[VDM_INDEX_INTnn];
    LocalInfo.OpcodeINTO        = ExVdmOpcodeDispatchCounts[VDM_INDEX_INTO];
    LocalInfo.OpcodeIRET        = ExVdmOpcodeDispatchCounts[VDM_INDEX_IRET];
    LocalInfo.OpcodeINBimm      = ExVdmOpcodeDispatchCounts[VDM_INDEX_INBimm];
    LocalInfo.OpcodeINWimm      = ExVdmOpcodeDispatchCounts[VDM_INDEX_INWimm];
    LocalInfo.OpcodeOUTBimm     = ExVdmOpcodeDispatchCounts[VDM_INDEX_OUTBimm];
    LocalInfo.OpcodeOUTWimm     = ExVdmOpcodeDispatchCounts[VDM_INDEX_OUTWimm];
    LocalInfo.OpcodeINB         = ExVdmOpcodeDispatchCounts[VDM_INDEX_INB];
    LocalInfo.OpcodeINW         = ExVdmOpcodeDispatchCounts[VDM_INDEX_INW];
    LocalInfo.OpcodeOUTB        = ExVdmOpcodeDispatchCounts[VDM_INDEX_OUTB];
    LocalInfo.OpcodeOUTW        = ExVdmOpcodeDispatchCounts[VDM_INDEX_OUTW];
    LocalInfo.OpcodeLOCKPrefix  = ExVdmOpcodeDispatchCounts[VDM_INDEX_LOCKPrefix];
    LocalInfo.OpcodeREPNEPrefix = ExVdmOpcodeDispatchCounts[VDM_INDEX_REPNEPrefix];
    LocalInfo.OpcodeREPPrefix   = ExVdmOpcodeDispatchCounts[VDM_INDEX_REPPrefix];
    LocalInfo.OpcodeHLT         = ExVdmOpcodeDispatchCounts[VDM_INDEX_HLT];
    LocalInfo.OpcodeCLI         = ExVdmOpcodeDispatchCounts[VDM_INDEX_CLI];
    LocalInfo.OpcodeSTI         = ExVdmOpcodeDispatchCounts[VDM_INDEX_STI];
    LocalInfo.BopCount          = VdmBopCount;
    LocalInfo.SegmentNotPresent = ExVdmSegmentNotPresent;

    RtlCopyMemory(Info,&LocalInfo,sizeof(LocalInfo));

    return STATUS_SUCCESS;
}

#endif

#if i386

NTSTATUS
ExpGetStackTraceInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    )
{
    NTSTATUS Status;
    PRTL_PROCESS_BACKTRACES BackTraceInformation;
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    PSTACK_TRACE_DATABASE DataBase;
    PRTL_STACK_TRACE_ENTRY p, *pp;
    ULONG RequiredLength, n;

    DataBase = RtlpAcquireStackTraceDataBase();

    if (DataBase == NULL) {
        return STATUS_UNSUCCESSFUL;
    }
    DataBase->DumpInProgress = TRUE;

    RtlpReleaseStackTraceDataBase();

    n = 0;
    RequiredLength = 0;
    Status = STATUS_INFO_LENGTH_MISMATCH;
    BackTraceInformation = (PRTL_PROCESS_BACKTRACES) SystemInformation;

    RequiredLength = FIELD_OFFSET( RTL_PROCESS_BACKTRACES, BackTraces );

    try {
        if (SystemInformationLength >= RequiredLength) {
            BackTraceInformation->CommittedMemory =
                (ULONG)DataBase->CurrentUpperCommitLimit - (ULONG)DataBase->CommitBase;
            BackTraceInformation->ReservedMemory =
                (ULONG)DataBase->EntryIndexArray - (ULONG)DataBase->CommitBase;
            BackTraceInformation->NumberOfBackTraceLookups = DataBase->NumberOfEntriesLookedUp;
            n = DataBase->NumberOfEntriesAdded;
            BackTraceInformation->NumberOfBackTraces = n;
        }

        RequiredLength += (sizeof( *BackTraceInfo ) * n);
        if (SystemInformationLength >= RequiredLength) {
            Status = STATUS_SUCCESS;
            BackTraceInfo = &BackTraceInformation->BackTraces[ 0 ];
            pp = DataBase->EntryIndexArray;
            while (n--) {
                p = *--pp;
                BackTraceInfo->SymbolicBackTrace = NULL;
                BackTraceInfo->TraceCount = p->TraceCount;
                BackTraceInfo->Index = p->Index;
                BackTraceInfo->Depth = p->Depth;
                RtlCopyMemory( BackTraceInfo->BackTrace,
                               p->BackTrace,
                               p->Depth * sizeof( PVOID )
                             );
                BackTraceInfo += 1;
            }
        }
    }
    finally {
        DataBase->DumpInProgress = FALSE;
    }

    if (ARGUMENT_PRESENT(ReturnLength)) {
        *ReturnLength = RequiredLength;
    }
    return Status;
}

#endif // i386

NTSTATUS
ExpGetLockInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    )

/*++

Routine Description:

    This function returns information about all the ERESOURCE locks
    in the system.

Arguments:

    SystemInformation - A pointer to a buffer which receives the specified
        information.

    SystemInformationLength - Specifies the length in bytes of the system
        information buffer.

    Length - An optional pointer which, if specified, receives the
        number of bytes placed in the system information buffer.


Return Value:

    Returns one of the following status codes:

        STATUS_SUCCESS - normal, successful completion.

        STATUS_INVALID_INFO_CLASS - The SystemInformationClass parameter
            did not specify a valid value.

        STATUS_INFO_LENGTH_MISMATCH - The value of the SystemInformationLength
            parameter did not match the length required for the information
            class requested by the SystemInformationClass parameter.

        STATUS_ACCESS_VIOLATION - Either the SystemInformation buffer pointer
            or the Length pointer value specified an invalid address.

        STATUS_WORKING_SET_QUOTA - The process does not have sufficient
            working set to lock the specified output structure in memory.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources exist
            for this request to complete.

--*/

{
    PRTL_PROCESS_LOCKS LockInfo;
    PVOID LockVariable;
    NTSTATUS Status;

    *Length = 0;

    Status = ExLockUserBuffer( SystemInformation,
                               SystemInformationLength,
                               KeGetPreviousMode(),
                               IoWriteAccess,
                               &LockInfo,
                               &LockVariable
                               );
    if (!NT_SUCCESS(Status)) {
        return( Status );
    }

    Status = STATUS_SUCCESS;

    MmLockPageableSectionByHandle (ExPageLockHandle);
    try {

        Status = ExQuerySystemLockInformation( LockInfo,
                                               SystemInformationLength,
                                               Length
                                             );
    }
    finally {
        ExUnlockUserBuffer( LockVariable );
        MmUnlockPageableImageSection(ExPageLockHandle);
    }

    return Status;
}

NTSTATUS
ExpGetLookasideInformation (
    OUT PVOID Buffer,
    IN ULONG BufferLength,
    OUT PULONG Length
    )

/*++

Routine Description:

    This function returns pool lookaside list and general lookaside
    list information.

Arguments:

    Buffer - Supplies a pointer to the buffer which receives the lookaside
        list information.

    BufferLength - Supplies the length of the information buffer in bytes.

    Length - Supplies a pointer to a variable that receives the length of
        lookaside information returned.

Environment:

    Kernel mode.

    This routine could be made PAGELK but it is a high frequency routine
    so it is actually better to keep it nonpaged to avoid bringing in the
    entire PAGELK section.

Return Value:

    Returns one of the following status codes:

        STATUS_SUCCESS - Normal, successful completion.

        STATUS_ACCESS_VIOLATION - The buffer could not be locked in memory.

--*/

{

    PVOID BufferLock;
    PLIST_ENTRY Entry;
    KIRQL OldIrql;
    ULONG Limit;
    PSYSTEM_LOOKASIDE_INFORMATION Lookaside;
    ULONG Number;
    PNPAGED_LOOKASIDE_LIST NPagedLookaside;
    PPAGED_LOOKASIDE_LIST PagedLookaside;
    PGENERAL_LOOKASIDE PoolLookaside;
    PGENERAL_LOOKASIDE SystemLookaside;
    PKSPIN_LOCK SpinLock;
    NTSTATUS Status;

    //
    // Compute the number of lookaside entries and set the return status to
    // success.
    //

    Limit = BufferLength / sizeof(SYSTEM_LOOKASIDE_INFORMATION);
    Number = 0;
    Status = STATUS_SUCCESS;

    //
    // If the number of lookaside entries to return is not zero, then collect
    // the lookaside information.
    //

    if (Limit != 0) {
        Status = ExLockUserBuffer(Buffer,
                                  BufferLength,
                                  KeGetPreviousMode(),
                                  IoWriteAccess,
                                  &Lookaside,
                                  &BufferLock);
        if (NT_SUCCESS(Status)) {

            Status = STATUS_SUCCESS;

            //
            // Copy nonpaged and paged pool lookaside information to
            // information buffer.
            //

            Entry = ExPoolLookasideListHead.Flink;
            while (Entry != &ExPoolLookasideListHead) {
                PoolLookaside = CONTAINING_RECORD(Entry,
                                                  GENERAL_LOOKASIDE,
                                                  ListEntry);

                Lookaside->CurrentDepth = ExQueryDepthSList(&PoolLookaside->ListHead);
                Lookaside->MaximumDepth = PoolLookaside->Depth;
                Lookaside->TotalAllocates = PoolLookaside->TotalAllocates;
                Lookaside->AllocateMisses =
                        PoolLookaside->TotalAllocates - PoolLookaside->AllocateHits;

                Lookaside->TotalFrees = PoolLookaside->TotalFrees;
                Lookaside->FreeMisses =
                        PoolLookaside->TotalFrees - PoolLookaside->FreeHits;

                Lookaside->Type = PoolLookaside->Type;
                Lookaside->Tag = PoolLookaside->Tag;
                Lookaside->Size = PoolLookaside->Size;
                Number += 1;
                if (Number == Limit) {
                    goto Finish2;
                }

                Entry = Entry->Flink;
                Lookaside += 1;
            }

            //
            // Copy nonpaged and paged system lookaside information to
            // information buffer.
            //

            Entry = ExSystemLookasideListHead.Flink;
            while (Entry != &ExSystemLookasideListHead) {
                SystemLookaside = CONTAINING_RECORD(Entry,
                                                    GENERAL_LOOKASIDE,
                                                    ListEntry);

                Lookaside->CurrentDepth = ExQueryDepthSList(&SystemLookaside->ListHead);
                Lookaside->MaximumDepth = SystemLookaside->Depth;
                Lookaside->TotalAllocates = SystemLookaside->TotalAllocates;
                Lookaside->AllocateMisses = SystemLookaside->AllocateMisses;
                Lookaside->TotalFrees = SystemLookaside->TotalFrees;
                Lookaside->FreeMisses = SystemLookaside->FreeMisses;
                Lookaside->Type = SystemLookaside->Type;
                Lookaside->Tag = SystemLookaside->Tag;
                Lookaside->Size = SystemLookaside->Size;
                Number += 1;
                if (Number == Limit) {
                    goto Finish2;
                }

                Entry = Entry->Flink;
                Lookaside += 1;
            }

            //
            // Copy nonpaged general lookaside information to buffer.
            //

            SpinLock = &ExNPagedLookasideLock;
            ExAcquireSpinLock(SpinLock, &OldIrql);
            Entry = ExNPagedLookasideListHead.Flink;
            while (Entry != &ExNPagedLookasideListHead) {
                NPagedLookaside = CONTAINING_RECORD(Entry,
                                                    NPAGED_LOOKASIDE_LIST,
                                                    L.ListEntry);

                Lookaside->CurrentDepth = ExQueryDepthSList(&NPagedLookaside->L.ListHead);
                Lookaside->MaximumDepth = NPagedLookaside->L.Depth;
                Lookaside->TotalAllocates = NPagedLookaside->L.TotalAllocates;
                Lookaside->AllocateMisses = NPagedLookaside->L.AllocateMisses;
                Lookaside->TotalFrees = NPagedLookaside->L.TotalFrees;
                Lookaside->FreeMisses = NPagedLookaside->L.FreeMisses;
                Lookaside->Type = 0;
                Lookaside->Tag = NPagedLookaside->L.Tag;
                Lookaside->Size = NPagedLookaside->L.Size;
                Number += 1;
                if (Number == Limit) {
                    goto Finish1;
                }

                Entry = Entry->Flink;
                Lookaside += 1;
            }

            ExReleaseSpinLock(SpinLock, OldIrql);

            //
            // Copy paged general lookaside information to buffer.
            //

            SpinLock = &ExPagedLookasideLock;
            ExAcquireSpinLock(SpinLock, &OldIrql);
            Entry = ExPagedLookasideListHead.Flink;
            while (Entry != &ExPagedLookasideListHead) {
                PagedLookaside = CONTAINING_RECORD(Entry,
                                                   PAGED_LOOKASIDE_LIST,
                                                   L.ListEntry);

                Lookaside->CurrentDepth = ExQueryDepthSList(&PagedLookaside->L.ListHead);
                Lookaside->MaximumDepth = PagedLookaside->L.Depth;
                Lookaside->TotalAllocates = PagedLookaside->L.TotalAllocates;
                Lookaside->AllocateMisses = PagedLookaside->L.AllocateMisses;
                Lookaside->TotalFrees = PagedLookaside->L.TotalFrees;
                Lookaside->FreeMisses = PagedLookaside->L.FreeMisses;
                Lookaside->Type = 1;
                Lookaside->Tag = PagedLookaside->L.Tag;
                Lookaside->Size = PagedLookaside->L.Size;
                Number += 1;
                if (Number == Limit) {
                    goto Finish1;
                }

                Entry = Entry->Flink;
                Lookaside += 1;
            }

Finish1:
            ExReleaseSpinLock(SpinLock, OldIrql);

Finish2:
            //
            // Unlock user buffer.
            //

            ExUnlockUserBuffer(BufferLock);
        }
    }

    *Length = Number * sizeof(SYSTEM_LOOKASIDE_INFORMATION);
    return Status;
}

NTSTATUS
ExpGetHandleInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    )

/*++

Routine Description:

    This function returns information about the open handles in the system.

Arguments:

    SystemInformation - A pointer to a buffer which receives the specified
        information.

    SystemInformationLength - Specifies the length in bytes of the system
        information buffer.

    Length - An optional pointer which, if specified, receives the
        number of bytes placed in the system information buffer.


Return Value:

    Returns one of the following status codes:

        STATUS_SUCCESS - normal, successful completion.

        STATUS_INVALID_INFO_CLASS - The SystemInformationClass parameter
            did not specify a valid value.

        STATUS_INFO_LENGTH_MISMATCH - The value of the SystemInformationLength
            parameter did not match the length required for the information
            class requested by the SystemInformationClass parameter.

        STATUS_ACCESS_VIOLATION - Either the SystemInformation buffer pointer
            or the Length pointer value specified an invalid address.

        STATUS_WORKING_SET_QUOTA - The process does not have sufficient
            working set to lock the specified output structure in memory.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources exist
            for this request to complete.

--*/

{
    PSYSTEM_HANDLE_INFORMATION HandleInfo;
    PVOID LockVariable;
    NTSTATUS Status;

    PAGED_CODE();

    *Length = 0;

    Status = ExLockUserBuffer( SystemInformation,
                               SystemInformationLength,
                               KeGetPreviousMode(),
                               IoWriteAccess,
                               &HandleInfo,
                               &LockVariable
                               );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = STATUS_SUCCESS;

    try {
        Status = ObGetHandleInformation( HandleInfo,
                                         SystemInformationLength,
                                         Length
                                       );

    }
    finally {
        ExUnlockUserBuffer( LockVariable );
    }

    return Status;
}

NTSTATUS
ExpGetHandleInformationEx (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    )

/*++

Routine Description:

    This function returns information about the open handles in the system.

Arguments:

    SystemInformation - A pointer to a buffer which receives the specified
        information.

    SystemInformationLength - Specifies the length in bytes of the system
        information buffer.

    Length - An optional pointer which, if specified, receives the
        number of bytes placed in the system information buffer.


Return Value:

    Returns one of the following status codes:

        STATUS_SUCCESS - normal, successful completion.

        STATUS_INVALID_INFO_CLASS - The SystemInformationClass parameter
            did not specify a valid value.

        STATUS_INFO_LENGTH_MISMATCH - The value of the SystemInformationLength
            parameter did not match the length required for the information
            class requested by the SystemInformationClass parameter.

        STATUS_ACCESS_VIOLATION - Either the SystemInformation buffer pointer
            or the Length pointer value specified an invalid address.

        STATUS_WORKING_SET_QUOTA - The process does not have sufficient
            working set to lock the specified output structure in memory.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources exist
            for this request to complete.

--*/

{
    PSYSTEM_HANDLE_INFORMATION_EX HandleInfo;
    PVOID LockVariable;
    NTSTATUS Status;

    PAGED_CODE();

    *Length = 0;

    Status = ExLockUserBuffer( SystemInformation,
                               SystemInformationLength,
                               KeGetPreviousMode(),
                               IoWriteAccess,
                               &HandleInfo,
                               &LockVariable
                               );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = STATUS_SUCCESS;

    try {
        Status = ObGetHandleInformationEx( HandleInfo,
                                           SystemInformationLength,
                                           Length
                                         );

    }
    finally {
        ExUnlockUserBuffer( LockVariable );
    }

    return Status;
}

NTSTATUS
ExpGetObjectInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    )

/*++

Routine Description:

    This function returns information about the objects in the system.

Arguments:

    SystemInformation - A pointer to a buffer which receives the specified
        information.

    SystemInformationLength - Specifies the length in bytes of the system
        information buffer.

    Length - An optional pointer which, if specified, receives the
        number of bytes placed in the system information buffer.


Return Value:

    Returns one of the following status codes:

        STATUS_SUCCESS - normal, successful completion.

        STATUS_INVALID_INFO_CLASS - The SystemInformationClass parameter
            did not specify a valid value.

        STATUS_INFO_LENGTH_MISMATCH - The value of the SystemInformationLength
            parameter did not match the length required for the information
            class requested by the SystemInformationClass parameter.

        STATUS_ACCESS_VIOLATION - Either the SystemInformation buffer pointer
            or the Length pointer value specified an invalid address.

        STATUS_WORKING_SET_QUOTA - The process does not have sufficient
            working set to lock the specified output structure in memory.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources exist
            for this request to complete.

--*/

{
    PSYSTEM_OBJECTTYPE_INFORMATION ObjectInfo;
    PVOID LockVariable;
    NTSTATUS Status;

    PAGED_CODE();

    *Length = 0;

    Status = ExLockUserBuffer( SystemInformation,
                               SystemInformationLength,
                               KeGetPreviousMode(),
                               IoWriteAccess,
                               &ObjectInfo,
                               &LockVariable
                               );
    if (!NT_SUCCESS(Status)) {
        return( Status );
    }

    Status = STATUS_SUCCESS;

    try {
        Status = ObGetObjectInformation( SystemInformation,
                                         ObjectInfo,
                                         SystemInformationLength,
                                         Length
                                       );

    }
    finally {
        ExUnlockUserBuffer( LockVariable );
    }

    return Status;
}

NTSTATUS
ExpQueryModuleInformation (
    IN PLIST_ENTRY LoadOrderListHead,
    IN PLIST_ENTRY UserModeLoadOrderListHead,
    OUT PRTL_PROCESS_MODULES ModuleInformation,
    IN ULONG ModuleInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    )
{
    NTSTATUS Status;
    ULONG RequiredLength;
    PLIST_ENTRY Next;
    PRTL_PROCESS_MODULE_INFORMATION ModuleInfo;
    PKLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    ANSI_STRING AnsiString;
    PCHAR s;
    ULONG NumberOfModules;

    NumberOfModules = 0;
    Status = STATUS_SUCCESS;
    RequiredLength = FIELD_OFFSET( RTL_PROCESS_MODULES, Modules );
    ModuleInfo = &ModuleInformation->Modules[ 0 ];

    Next = LoadOrderListHead->Flink;
    while ( Next != LoadOrderListHead ) {
        LdrDataTableEntry = CONTAINING_RECORD( Next,
                                               KLDR_DATA_TABLE_ENTRY,
                                               InLoadOrderLinks
                                             );

        RequiredLength += sizeof( RTL_PROCESS_MODULE_INFORMATION );
        if (ModuleInformationLength < RequiredLength) {
            Status = STATUS_INFO_LENGTH_MISMATCH;
        }
        else {

            ModuleInfo->MappedBase = NULL;
            ModuleInfo->ImageBase = LdrDataTableEntry->DllBase;
            ModuleInfo->ImageSize = LdrDataTableEntry->SizeOfImage;
            ModuleInfo->Flags = LdrDataTableEntry->Flags;
            ModuleInfo->LoadCount = LdrDataTableEntry->LoadCount;
            
            ModuleInfo->LoadOrderIndex = (USHORT)(NumberOfModules);
            
            ModuleInfo->InitOrderIndex = 0;
                
            AnsiString.Buffer = (PCHAR) ModuleInfo->FullPathName;
            AnsiString.Length = 0;
            AnsiString.MaximumLength = sizeof( ModuleInfo->FullPathName );
            Status = RtlUnicodeStringToAnsiString( &AnsiString,
                                          &LdrDataTableEntry->FullDllName,
                                          FALSE
                                        );

            if (NT_SUCCESS(Status) || (Status == STATUS_BUFFER_OVERFLOW)) {
                s = AnsiString.Buffer + AnsiString.Length;
                while (s > AnsiString.Buffer && *--s) {
                    if (*s == (UCHAR)OBJ_NAME_PATH_SEPARATOR) {
                        s += 1;
                        break;
                    }
                }
                ModuleInfo->OffsetToFileName = (USHORT)(s - AnsiString.Buffer);
            } else {
                ModuleInfo->FullPathName[0] = '\0';
                ModuleInfo->OffsetToFileName = 0;
            }
            
            ModuleInfo += 1;
        }

        NumberOfModules += 1;
        Next = Next->Flink;
    }

    if (ARGUMENT_PRESENT( UserModeLoadOrderListHead )) {
        Next = UserModeLoadOrderListHead->Flink;
        while ( Next != UserModeLoadOrderListHead ) {
            LdrDataTableEntry = CONTAINING_RECORD( Next,
                                                   KLDR_DATA_TABLE_ENTRY,
                                                   InLoadOrderLinks
                                                 );

            RequiredLength += sizeof( RTL_PROCESS_MODULE_INFORMATION );
            if (ModuleInformationLength < RequiredLength) {
                Status = STATUS_INFO_LENGTH_MISMATCH;
            }
            else {
                ModuleInfo->MappedBase = NULL;
                ModuleInfo->ImageBase = LdrDataTableEntry->DllBase;
                ModuleInfo->ImageSize = LdrDataTableEntry->SizeOfImage;
                ModuleInfo->Flags = LdrDataTableEntry->Flags;
                ModuleInfo->LoadCount = LdrDataTableEntry->LoadCount;

                ModuleInfo->LoadOrderIndex = (USHORT)(NumberOfModules);

                ModuleInfo->InitOrderIndex = ModuleInfo->LoadOrderIndex;
                    
                AnsiString.Buffer = (PCHAR) ModuleInfo->FullPathName;
                AnsiString.Length = 0;
                AnsiString.MaximumLength = sizeof( ModuleInfo->FullPathName );
                Status = RtlUnicodeStringToAnsiString( &AnsiString,
                                              &LdrDataTableEntry->FullDllName,
                                              FALSE
                                            );

                if (NT_SUCCESS (Status) || (Status == STATUS_BUFFER_OVERFLOW)) {
                    s = AnsiString.Buffer + AnsiString.Length;
                    while (s > AnsiString.Buffer && *--s) {
                        if (*s == (UCHAR)OBJ_NAME_PATH_SEPARATOR) {
                            s += 1;
                            break;
                        }
                    }
                    ModuleInfo->OffsetToFileName = (USHORT)(s - AnsiString.Buffer);
                } else {
                    ModuleInfo->FullPathName[0] = '\0';
                    ModuleInfo->OffsetToFileName = 0;
                }

                ModuleInfo += 1;
            }

            NumberOfModules += 1;
            Next = Next->Flink;
        }
    }

    if (ARGUMENT_PRESENT(ReturnLength)) {
        *ReturnLength = RequiredLength;
    }
    if (ModuleInformationLength >= FIELD_OFFSET( RTL_PROCESS_MODULES, Modules )) {
        ModuleInformation->NumberOfModules = NumberOfModules;
    } else {
        Status = STATUS_INFO_LENGTH_MISMATCH;
    }
    return Status;
}

BOOLEAN
ExIsProcessorFeaturePresent (
    __in ULONG ProcessorFeature
    )

{

    BOOLEAN rv;

    //
    // On AMD64 systems legacy floating point, MMX, and 3D-Now are not
    // supported for 64-bit applications (i.e., the legacy floating
    // point state is not saved and restored). Therefore, FALSE is always
    // returned for these features.
    //

#if defined(_AMD64_)

    if ((ProcessorFeature == PF_MMX_INSTRUCTIONS_AVAILABLE) ||
        (ProcessorFeature == PF_3DNOW_INSTRUCTIONS_AVAILABLE)) {

        return FALSE;
    }

#endif

    if (ProcessorFeature < PROCESSOR_FEATURE_MAX) {
        rv = SharedUserData->ProcessorFeatures[ProcessorFeature];

    } else {
        rv = FALSE;
    }

    return rv;
}

NTSTATUS
ExpQueryLegacyDriverInformation (
    IN PSYSTEM_LEGACY_DRIVER_INFORMATION LegacyInfo,
    IN PULONG Length
    )

/*++

Routine Description:

    Returns legacy driver information for figuring out why PNP/Power functionality
    is disabled.

Arguments:

    LegacyInfo - Returns the legacy driver information

    Length - Supplies the length of the LegacyInfo buffer
             Returns the amount of data written

Return Value:

    NTSTATUS

--*/

{
    PNP_VETO_TYPE VetoType;
    PWSTR VetoList = NULL;
    NTSTATUS Status;
    UNICODE_STRING String;
    ULONG ReturnLength;

    Status = IoGetLegacyVetoList(&VetoList, &VetoType);
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    RtlInitUnicodeString(&String, VetoList);
    ReturnLength = sizeof(SYSTEM_LEGACY_DRIVER_INFORMATION) + String.Length;
    try {
        if (ReturnLength > *Length) {
            Status = STATUS_BUFFER_OVERFLOW;
        } else {
            LegacyInfo->VetoType = VetoType;
            LegacyInfo->VetoList.Length = String.Length;
            LegacyInfo->VetoList.Buffer = (PWSTR)(LegacyInfo+1);
            RtlCopyMemory(LegacyInfo+1, String.Buffer, String.Length);
        }
    } finally {
        if (VetoList) {
            ExFreePool(VetoList);
        }
    }

    *Length = ReturnLength;
    return(Status);
}

VOID
ExGetCurrentProcessorCpuUsage (
    __out PULONG CpuUsage
    )

/*++

Routine Description:

    Returns an estimation of current cpu usage in percent.

Arguments:

    CpuUsage - Returns the current cpu usage in percent.

Return Value:

    Nothing

--*/
{
    PKPRCB Prcb;

    Prcb = KeGetCurrentPrcb();
    *CpuUsage = 100 - (ULONG)(UInt32x32To64(Prcb->IdleThread->KernelTime, 100) /
                               (ULONGLONG)(Prcb->KernelTime + Prcb->UserTime));
}

VOID
ExGetCurrentProcessorCounts (
    __out PULONG IdleCount,
    __out PULONG KernelAndUser,
    __out PULONG Index
    )

/*++

Routine Description:

    Returns information regarding idle time and kernel + user time for
    the current processor.

Arguments:

    IdleCount - Returns the kernel time of the idle thread on the current
                processor.

    KernelAndUser - Returns the kernel pluse user on the current processor.

    Index - Returns the number identifying the current processor.

Return Value:

    Nothing

--*/
{
    PKPRCB Prcb;

    Prcb = KeGetCurrentPrcb();
    *IdleCount = Prcb->IdleThread->KernelTime;
    *KernelAndUser = Prcb->KernelTime + Prcb->UserTime;
    *Index = (ULONG)Prcb->Number;
}

BOOLEAN
ExpIsValidUILanguage (
    IN WCHAR * pLangId
    )

/*++
Routine Description:

    Check if specified language ID is valid.

Arguments:

    pLangId - language ID hex string.

Return Value:

    TRUE: Valid
    FALSE: Invalid

--*/

{
    NTSTATUS Status;
    UNICODE_STRING KeyPath, KeyValueName;
    HANDLE hKey;
    WCHAR KeyValueBuffer[ 128 ];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG ResultLength;
    BOOLEAN bRet = FALSE;
    int iLen = 0;


    RtlInitUnicodeString(&KeyPath, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\MUILanguages");
    //
    // pLangId is passed in as DWORD or WORD hex string
    // LangId string in MUILanguages is set as WORD hex string
    //
    while (pLangId[iLen])
    {
      iLen++;
    }
    //
    // We need to validate both 4 digits and 8 digits LangId
    //
    RtlInitUnicodeString(&KeyValueName, iLen < 8? pLangId : &pLangId[4]);

    InitializeObjectAttributes (&ObjectAttributes,
                                &KeyPath,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL);

    if (NT_SUCCESS(ZwOpenKey( &hKey,  GENERIC_READ, &ObjectAttributes)))
    {
        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;

        Status = ZwQueryValueKey( hKey,
                                  &KeyValueName,
                                  KeyValuePartialInformation,
                                  KeyValueInformation,
                                  sizeof( KeyValueBuffer ),
                                  &ResultLength
                                );

        if (NT_SUCCESS(Status))
        {
            if (KeyValueInformation->Type == REG_SZ && *((PWSTR)(KeyValueInformation->Data)) == L'1')
            {
                bRet = TRUE;
            }
        }

        ZwClose(hKey);
    }

    return bRet;
}

NTSTATUS
ExpGetSystemFirmwareTableInformation(
    IN OUT PVOID SystemFirmwareTableInformation,
    IN  KPROCESSOR_MODE PreviousMode,
    OUT PULONG ReturnLength OPTIONAL
    )

/*++

Description:

    Look for the requested Firmware Table provider. if it exists, pass the 
    request on to it and return the result.

Parameters:

    SystemFirmwareTableInformation  - points to the SYSTEM_FIRMWARE_TABLE_INFORMATION 
                                      structure.
        
    ReturnLength                    - returns the bufferlength if present.

Return Value:

    STATUS_SUCCESS          - on success.
    STATUS_NOT_IMPLEMENTED  - on Failure.

--*/

{
    ULONG                               BufferLength = 0;
    PSYSTEM_FIRMWARE_TABLE_HANDLER_NODE HandlerCurrent;
    NTSTATUS                            Status = STATUS_NOT_IMPLEMENTED;
    PSYSTEM_FIRMWARE_TABLE_INFORMATION  TableInfo = NULL;
    PSYSTEM_FIRMWARE_TABLE_INFORMATION  TableInfoLocal = NULL;
    ULONG                               TotalStructSize = 0;
    
    PAGED_CODE();

    //
    // Point the incoming SystemFirmwareTableInformation to TableInfo. If this
    // is a usermode call, we will copy all the usermode data into local buffers
    // before calling the table provider.
    //
    
    TableInfo = (PSYSTEM_FIRMWARE_TABLE_INFORMATION)SystemFirmwareTableInformation;

    if (PreviousMode != KernelMode) {

        try {

            ProbeForRead(&(TableInfo->TableBufferLength),
                         sizeof(ULONG),
                         sizeof(ULONG));

            //
            // Save a copy of the TableBufferLength as this value may be changed 
            // by the table providers handler.
            //
            
            BufferLength = TableInfo->TableBufferLength;

        } except (EXCEPTION_EXECUTE_HANDLER) {
            
            return GetExceptionCode();        
        }

        TotalStructSize = sizeof(SYSTEM_FIRMWARE_TABLE_INFORMATION) + BufferLength;
        
        //
        // Allocate a local buffer. This way we always pass trusted memory to the
        // firmware table providers.
        //
        
        TableInfoLocal = ExAllocatePoolWithQuotaTag(PagedPool, 
                                                    TotalStructSize,
                                                    'TFRA');

        if (!TableInfoLocal) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(TableInfoLocal, TotalStructSize);
        
        try {

            //
            // Verify and copy the data to trusted memory.
            //
            
            ProbeForRead(TableInfo, 
                         sizeof(SYSTEM_FIRMWARE_TABLE_INFORMATION),
                         sizeof(ULONG));
            
            RtlCopyMemory(TableInfoLocal, 
                          TableInfo, 
                          sizeof(SYSTEM_FIRMWARE_TABLE_INFORMATION));
        } except (EXCEPTION_EXECUTE_HANDLER) {
            
            return GetExceptionCode();        
        }

        //
        // Now save a pointer to our copy of TableInfoLocal into Tableinfo. This
        // will improve code readability because all the shared code between user   
        // mode and kernel mode will use the same variable name.
        //

        TableInfo = TableInfoLocal;
    }

    //
    // Grab the resource as shared.
    //

    KeEnterCriticalRegion();

    ExAcquireResourceSharedLite(&ExpFirmwareTableResource, TRUE);
    
    //
    // Check if the required table provider is available.
    //

    EX_FOR_EACH_IN_LIST(SYSTEM_FIRMWARE_TABLE_HANDLER_NODE,
                        FirmwareTableProviderList,
                        &ExpFirmwareTableProviderListHead,
                        HandlerCurrent) {

        if (HandlerCurrent->SystemFWHandler.ProviderSignature == TableInfo->ProviderSignature) {

            //
            // Call the providers handler.
            //
            
            Status = (*(HandlerCurrent->SystemFWHandler.FirmwareTableHandler))(TableInfo);

            if (PreviousMode != KernelMode) {

                //
                // Copy the returned data to the callers buffer.
                //

                try {

                    //
                    // Point Tableinfo back to the original usermode buffer. Our
                    // new data is safe in TableInfoLocal.
                    //

                    TableInfo = (PSYSTEM_FIRMWARE_TABLE_INFORMATION)
                                    SystemFirmwareTableInformation;
                    
                    
                    if (NT_SUCCESS(Status)) {
                
                        //
                        // The usermode buffer (stored in TableInfo) in untrusted. 
                        // It's content could have changed. Use the cached value 
                        // in BufferLength to do the probe and make sure the 
                        // buffer is still valid.
                        
                        //
                        // Probe the Table buffer for writing.
                        //
                        
                        ProbeForWrite(TableInfo->TableBuffer, 
                                      BufferLength, 
                                      sizeof(UCHAR));

                        //
                        // Copy over the returned buffer to the user mode buffer.
                        //

                        RtlCopyMemory(TableInfo->TableBuffer, 
                                      TableInfoLocal->TableBuffer, 
                                      TableInfoLocal->TableBufferLength);
                    }

                    //
                    // Always update the returned buffer length.
                    //
                    
                    //
                    // Probe the structure for writing.
                    //

                    ProbeForWrite(&(TableInfo->TableBufferLength), 
                                  sizeof(ULONG), 
                                  sizeof(ULONG));

                    //
                    // Copy over the number of bytes written or required.
                    //

                    RtlCopyMemory(&(TableInfo->TableBufferLength), 
                                  &(TableInfoLocal->TableBufferLength), 
                                  sizeof(ULONG));        
                 
                    TableInfo = TableInfoLocal;

                    //
                    // Fill in ReturnLength.
                    //

                    if (ReturnLength) {

                        ProbeForWrite(ReturnLength, 
                                      sizeof(ULONG), 
                                      sizeof(ULONG));

                        *ReturnLength = TableInfo->TableBufferLength +
                                        sizeof(SYSTEM_FIRMWARE_TABLE_INFORMATION);
                    }
                
                    
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    
                    Status = GetExceptionCode();        
                }
                
            } else {

                 //
                 // Fill in ReturnLength.
                 //
                 if (ReturnLength) {

                    *ReturnLength = TableInfo->TableBufferLength +
                                    sizeof(SYSTEM_FIRMWARE_TABLE_INFORMATION);
                 }
            }
            
            break;
        }
    }

    if (TableInfoLocal) {

        ExFreePoolWithTag(TableInfoLocal, 'TFRA');
    }
    
    ExReleaseResourceLite(&ExpFirmwareTableResource);

    KeLeaveCriticalRegion();
    
    return Status;
}


NTSTATUS
ExpRegisterFirmwareTableInformationHandler(
    IN OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    IN KPROCESSOR_MODE PreviousMode
    )

/*++

Description:

    This routine registers and unregisters firmware table providers.

Parameters:

    SystemInformation       - points to the SYSTEM_FIRMWARE_TABLE_HANDLER 
                              structure.

    SystemInformationLength - returns the number of bytes written on success. if
                              the provided buffer was too small, it returns the 
                              required size.

    PreviousMode            - Previous mode (Kernel / User).

Return Value:

    STATUS_SUCCESS                  - On success.
    STATUS_PRIVILEGE_NOT_HELD       - If caller is from User mode.
    STATUS_INFO_LENGTH_MISMATCH     - Buffer too small.
    STATUS_INSUFFICIENT_RESOURCES   - On failure to allocate resources.
    STATUS_OBJECT_NAME_EXISTS       - Table already registered.
    STATUS_INVALID_PARAMETER        - Table not found / invalid request.

--*/

{
    
    BOOLEAN                             HandlerFound = FALSE;
    PSYSTEM_FIRMWARE_TABLE_HANDLER_NODE HandlerListCurrent = NULL;
    PSYSTEM_FIRMWARE_TABLE_HANDLER_NODE HandlerListNew = NULL;
    NTSTATUS                            Status = STATUS_SUCCESS;
    PSYSTEM_FIRMWARE_TABLE_HANDLER      SystemTableHandler = NULL;

    PAGED_CODE();

    if (PreviousMode != KernelMode) {

            Status = STATUS_PRIVILEGE_NOT_HELD;
    } else {

        if ((!SystemInformation) || 
           (SystemInformationLength < sizeof(SYSTEM_FIRMWARE_TABLE_HANDLER))) {

            Status = STATUS_INFO_LENGTH_MISMATCH;
        } else {

            //
            // Grab the resource to prevent state change via reentry.
            //

            KeEnterCriticalRegion();
            
            ExAcquireResourceExclusiveLite(&ExpFirmwareTableResource, 
                                           TRUE);
            
            SystemTableHandler = (PSYSTEM_FIRMWARE_TABLE_HANDLER)SystemInformation;
            
            EX_FOR_EACH_IN_LIST(SYSTEM_FIRMWARE_TABLE_HANDLER_NODE,
                                FirmwareTableProviderList,
                                &ExpFirmwareTableProviderListHead,
                                HandlerListCurrent) {

                if (HandlerListCurrent->SystemFWHandler.ProviderSignature == SystemTableHandler->ProviderSignature) {

                    HandlerFound = TRUE;
                    break;
                }
            }

            //
            // Handler was not found and this is a register request, so
            // allocate a new node and insert it into the list.
            //
            
            if ((!HandlerFound) && (SystemTableHandler->Register)) {

                //
                // This is a new Firmware table handler, allocate 
                // the space and add it to the list.
                //

                HandlerListNew = ExAllocatePoolWithTag(PagedPool, 
                                                       sizeof(SYSTEM_FIRMWARE_TABLE_HANDLER_NODE),
                                                       'TFRA');
                
                if (HandlerListNew) {

                    //
                    // Populate the new node.
                    //

                    HandlerListNew->SystemFWHandler.ProviderSignature = SystemTableHandler->ProviderSignature;

                    HandlerListNew->SystemFWHandler.FirmwareTableHandler = SystemTableHandler->FirmwareTableHandler;

                    HandlerListNew->SystemFWHandler.DriverObject = SystemTableHandler->DriverObject;

                    InitializeListHead(&HandlerListNew->FirmwareTableProviderList);
                    
                    //
                    // Grab a reference to the providers driverobject so that the
                    // driver does not get unloaded without our knowledge. The 
                    // handler must first be unregistered before unloading. 
                    //

                    ObReferenceObject((PVOID)HandlerListNew->SystemFWHandler.DriverObject);

                    //
                    // Update the LinkList.
                    //

                    InsertTailList(&ExpFirmwareTableProviderListHead, 
                                   &HandlerListNew->FirmwareTableProviderList);
                } else {

                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
                
            } else if ((HandlerFound) && (!(SystemTableHandler->Register))) {

            
                //
                // Check to make sure that a matching driver object was sent in.
                //

                if (HandlerListCurrent->SystemFWHandler.DriverObject == SystemTableHandler->DriverObject) {

                    //
                    // Remove the entry from the list.
                    //

                    RemoveEntryList(&HandlerListCurrent->FirmwareTableProviderList);
                    
                    //
                    // Deref the device object.
                    //

                    ObDereferenceObject((PVOID)HandlerListCurrent->SystemFWHandler.DriverObject);

                    //
                    // Free the unregistered list element.
                    //

                    ExFreePoolWithTag(HandlerListCurrent, 'TFRA');
                } else {

                    Status = STATUS_INVALID_PARAMETER;

                }
            } else if ((HandlerFound) && (SystemTableHandler->Register)) {

                //
                // A handler for this table has already been registered. return 
                // error.
                //

                Status = STATUS_OBJECT_NAME_EXISTS;
            } else {

                Status = STATUS_INVALID_PARAMETER;
            }
                
            ExReleaseResourceLite(&ExpFirmwareTableResource);

            KeLeaveCriticalRegion();
        }
    }
  
    return Status;
}

