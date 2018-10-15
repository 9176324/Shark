/*****************************************************************************
 *
 *  Main.c
 *
 *  Abstract:
 *
 *      Template effect driver that doesn't actually do anything.
 *
 *****************************************************************************/

#include "effdrv.h"

/*****************************************************************************
 *
 *      Constant globals:  Never change.  Ever.
 *
 *****************************************************************************/

/*
 * !!IHV!! You must run GUIDGEN or UUIDGEN to generate your own GUID/CLSID
 */

CLSID CLSID_MyServer =
{ 0x079A13E0, 0xB5C4, 0x11D0, 0x9A, 0xD0, 0x00, 0xA0, 0xC9, 0xA0, 0x6E, 0x35 };

GUID GUID_MySharedMemory =
{ 0x079A13E1, 0xB5C4, 0x11D0, 0x9A, 0xD0, 0x00, 0xA0, 0xC9, 0xA0, 0x6E, 0x35 };

GUID GUID_MyMutex =
{ 0x079A13E2, 0xB5C4, 0x11D0, 0x9A, 0xD0, 0x00, 0xA0, 0xC9, 0xA0, 0x6E, 0x35 };

/*****************************************************************************
 *
 *      Static globals:  Initialized at PROCESS_ATTACH and never modified.
 *
 *****************************************************************************/

HINSTANCE g_hinst;              /* This DLL's instance handle */
HINSTANCE g_hinst;              /* This DLL's instance handle */
PSHAREDMEMORY g_pshmem;         /* Our shared memory block */
HANDLE g_hfm;                   /* Handle to file mapping object */
HANDLE g_hmtxShared;            /* Handle to mutex that protects g_pshmem */

/*****************************************************************************
 *
 *      Dynamic Globals.  There should be as few of these as possible.
 *
 *      All access to dynamic globals must be thread-safe.
 *
 *****************************************************************************/

ULONG g_cRef;                   /* Global reference count */
CRITICAL_SECTION g_crst;        /* Global critical section */

/*****************************************************************************
 *
 *      DllAddRef / DllRelease
 *
 *      Adjust the DLL reference count.
 *
 *****************************************************************************/

STDAPI_(ULONG)
DllAddRef(void)
{
    return (ULONG)InterlockedIncrement((LPLONG)&g_cRef);
}

STDAPI_(ULONG)
DllRelease(void)
{
    return (ULONG)InterlockedDecrement((LPLONG)&g_cRef);
}

/*****************************************************************************
 *
 *      DllEnterCritical / DllLeaveCritical
 *
 *      Enter and exit the DLL critical section.
 *
 *****************************************************************************/

STDAPI_(void)
DllEnterCritical(void)
{
    EnterCriticalSection(&g_crst);
}

STDAPI_(void)
DllLeaveCritical(void)
{
    LeaveCriticalSection(&g_crst);
}

/*****************************************************************************
 *
 *      DllGetClassObject
 *
 *      OLE entry point.  Produces an IClassFactory for the indicated GUID.
 *
 *****************************************************************************/

STDAPI
DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvObj)
{
    HRESULT hres;

    if (IsEqualGUID(rclsid, &CLSID_MyServer)) {
        hres = CClassFactory_New(riid, ppvObj);
    } else {
        *ppvObj = 0;
        hres = CLASS_E_CLASSNOTAVAILABLE;
    }
    return hres;
}

/*****************************************************************************
 *
 *      DllCanUnloadNow
 *
 *      OLE entry point.  Fail iff there are outstanding refs.
 *
 *****************************************************************************/

STDAPI
DllCanUnloadNow(void)
{
    return g_cRef ? S_FALSE : S_OK;
}

/*****************************************************************************
 *
 *      DllNameFromGuid
 *
 *      Create the string version of a GUID.
 *
 *****************************************************************************/

STDAPI_(void)
DllNameFromGuid(LPTSTR ptszBuf, LPGUID pguid)
{
    wsprintf(ptszBuf,
             TEXT("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
             pguid->Data1, pguid->Data2, pguid->Data3,
             pguid->Data4[0], pguid->Data4[1],
             pguid->Data4[2], pguid->Data4[3],
             pguid->Data4[4], pguid->Data4[5],
             pguid->Data4[6], pguid->Data4[7]);
}

/*****************************************************************************
 *
 *      DllOnProcessAttach
 *
 *      Initialize the DLL.
 *
 *****************************************************************************/

STDAPI_(BOOL)
DllOnProcessAttach(HINSTANCE hinst)
{
    TCHAR tszName[256];
    g_hinst = hinst;

    /*
     *  Performance tweak: We do not need thread notifications.
     */
    DisableThreadLibraryCalls(hinst);

    /*
     *  !!IHV!! Initialize your DLL here.
     */

    InitializeCriticalSection(&g_crst);

    /*
     *  Create our mutex that protects the shared memory block.
     *  If it already exists, then we get access to the one that
     *  already exists.
     *
     *  The name of the shared memory block is GUID_MyMutex.
     */
    DllNameFromGuid(tszName, &GUID_MyMutex);

    g_hmtxShared = CreateMutex(NULL, FALSE, tszName);

    if (g_hmtxShared == NULL) {
        return FALSE;
    }

    /*
     *  Create our shared memory block.  If it already exists,
     *  then we get access to the one that already exists.
     *  If it doesn't already exist, then it gets created
     *  zero-filled (which is what we want anyway).
     *
     *  The name of the shared memory block is GUID_MySharedMemory.
     */
    DllNameFromGuid(tszName, &GUID_MySharedMemory);

    g_hfm = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
                              PAGE_READWRITE, 0,
                              sizeof(SHAREDMEMORY),
                              tszName);

    if (g_hfm == NULL) {
        CloseHandle(g_hmtxShared);
        g_hmtxShared = NULL;

        return FALSE;
    }

    g_pshmem = MapViewOfFile(g_hfm, FILE_MAP_WRITE | FILE_MAP_READ,
                             0, 0, 0);
    if (g_pshmem == NULL) {
        CloseHandle(g_hmtxShared);
        g_hmtxShared = NULL;

        CloseHandle(g_hfm);
        g_hfm = NULL;
        return FALSE;
    }

    return TRUE;

}

/*****************************************************************************
 *
 *      DllOnProcessDetach
 *
 *      De-initialize the DLL.
 *
 *****************************************************************************/

STDAPI_(void)
DllOnProcessDetach(void)
{
    /*
     *  !!IHV!! De-initialize your DLL here.
     */

    if (g_pshmem != NULL) {
        UnmapViewOfFile(g_pshmem);
        g_pshmem = NULL;
    }

    if (g_hfm != NULL) {
        CloseHandle(g_hfm);
        g_hfm = NULL;
    }

    if (g_hmtxShared != NULL) {
        CloseHandle(g_hmtxShared);
        g_hmtxShared = NULL;
    }

    DeleteCriticalSection(&g_crst);
}

/*****************************************************************************
 *
 *      DllMain
 *
 *      DLL entry point.
 *
 *****************************************************************************/

STDAPI_(BOOL)
DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) {

    case DLL_PROCESS_ATTACH:
        return DllOnProcessAttach(hinst);

    case DLL_PROCESS_DETACH:
        DllOnProcessDetach();
        break;
    }

    return 1;
}
