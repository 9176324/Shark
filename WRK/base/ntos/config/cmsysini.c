/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmsysini.c

Abstract:

    This module contains init support for the configuration manager,
    particularly the registry.

Revision History:

     Added CmpSaveBootControlSet & CmpDeleteCloneTree in order to
     perform some of the LKG work that has been moved into the kernel.
     Modified system initialization to permit operation and LKG control
     set saves without a CurrentControlSet clone.

--*/

#include    "cmp.h"
#include    "arc.h"
#pragma hdrstop
#include    "arccodes.h"

#pragma warning(disable:4204)   // non constant aggregate initializer
#pragma warning(disable:4221)   // initialization using address of automatic

//
// paths
//

#define INIT_REGISTRY_MASTERPATH   L"\\REGISTRY\\"

extern  PKPROCESS   CmpSystemProcess;
extern  ERESOURCE   CmpRegistryLock;

extern  EX_PUSH_LOCK  CmpPostLock;

extern  BOOLEAN     CmFirstTime;
extern  BOOLEAN HvShutdownComplete;

//
// List of MACHINE hives to load.
//
extern  HIVE_LIST_ENTRY CmpMachineHiveList[];
extern  UCHAR           SystemHiveFullPathBuffer[];
extern  UNICODE_STRING  SystemHiveFullPathName;

#define SYSTEM_PATH L"\\registry\\machine\\system"

//
// special keys for backwards compatibility with 1.0
//
#define HKEY_PERFORMANCE_TEXT       (( HANDLE ) (ULONG_PTR)((LONG)0x80000050) )
#define HKEY_PERFORMANCE_NLSTEXT    (( HANDLE ) (ULONG_PTR)((LONG)0x80000060) )

extern UNICODE_STRING  CmpSystemFileName;
extern UNICODE_STRING  CmSymbolicLinkValueName;
extern UNICODE_STRING  CmpLoadOptions;         // sys options from FW or boot.ini
extern PWCHAR CmpProcessorControl;
extern PWCHAR CmpControlSessionManager;

//
//
// Object type definition support.
//
// Key objects (CmpKeyObjectType) represent open instances of keys in the
// registry.  They do not have object names, rather, their names are
// defined by the registry backing store.
//

//
// Master Hive
//
//  The KEY_NODEs for \REGISTRY, \REGISTRY\MACHINE, and \REGISTRY\USER
//  are stored in a small memory only hive called the Master Hive.
//  All other hives have link nodes in this hive which point to them.
//
extern   PCMHIVE CmpMasterHive;
extern   BOOLEAN CmpNoMasterCreates;    // Init False, Set TRUE after we're done to
                                        // prevent random creates in the
                                        // master hive, which is not backed
                                        // by a file.

extern   LIST_ENTRY  CmpHiveListHead;   // List of CMHIVEs


//
// Addresses of object type descriptors:
//

extern   POBJECT_TYPE CmpKeyObjectType;

//
// Define attributes that Key objects are not allowed to have.
//

#define CMP_KEY_INVALID_ATTRIBUTES  (OBJ_EXCLUSIVE  |\
                                     OBJ_PERMANENT)


//
// Global control values
//

//
// Write-Control:
//  CmpNoWrite is initially true.  When set this way write and flush
//  do nothing, simply returning success.  When cleared to FALSE, I/O
//  is enabled.  This change is made after the I/O system is started
//  AND autocheck (chkdsk) has done its thing.
//

extern BOOLEAN CmpNoWrite;

//
// Buffer used for quick-stash transfers in CmSetValueKey
//
extern PUCHAR  CmpStashBuffer;
extern ULONG   CmpStashBufferSize;


//
// set to true if disk full when trying to save the changes made between system hive loading and registry initialization
//
extern BOOLEAN CmpCannotWriteConfiguration;
//
// Global "constants"
//

extern   const UNICODE_STRING nullclass;
extern BOOLEAN CmpTrackHiveClose;

ULONG   CmSelfHeal = 1; // enabled by default

extern LIST_ENTRY	    CmpSelfHealQueueListHead;
extern KGUARDED_MUTEX	CmpSelfHealQueueLock;

//
// Private prototypes
//
VOID
CmpCreatePredefined(
    IN HANDLE Root,
    IN PWSTR KeyName,
    IN HANDLE PredefinedHandle
    );

VOID
CmpCreatePerfKeys(
    VOID
    );

BOOLEAN
CmpLinkKeyToHive(
    PWSTR   KeyPath,
    PWSTR   HivePath
    );

NTSTATUS
CmpCreateControlSet(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
CmpCloneControlSet(
    VOID
    );

NTSTATUS
CmpCreateObjectTypes(
    VOID
    );

BOOLEAN
CmpCreateRegistryRoot(
    VOID
    );

BOOLEAN
CmpCreateRootNode(
    IN PHHIVE   Hive,
    IN PWSTR    Name,
    OUT PHCELL_INDEX RootCellIndex
    );

VOID
CmpFreeDriverList(
    IN PHHIVE Hive,
    IN PLIST_ENTRY DriverList
    );

VOID
CmpInitializeHiveList(
    VOID
    );

BOOLEAN
CmpInitializeSystemHive(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
CmpInterlockedFunction (
    PWCHAR RegistryValueKey,
    VOID (*InterlockedFunction)(VOID)
    );

VOID
CmpConfigureProcessors (
    VOID
    );

NTSTATUS
CmpAddDockingInfo (
    IN HANDLE Key,
    IN PROFILE_PARAMETER_BLOCK * ProfileBlock
    );

NTSTATUS
CmpAddAliasEntry (
    IN HANDLE IDConfigDB,
    IN PROFILE_PARAMETER_BLOCK * ProfileBlock,
    IN ULONG  ProfileNumber
    );

NTSTATUS CmpDeleteCloneTree(VOID);

VOID
CmpDiskFullWarning(
    VOID
    );

VOID
CmpLoadHiveThread(
    IN PVOID StartContext
    );

NTSTATUS
CmpSetupPrivateWrite(
    PCMHIVE             CmHive
    );

NTSTATUS
CmpSetSystemValues(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
CmpSetNetworkValue(
    IN PNETWORK_LOADER_BLOCK NetworkLoaderBlock
    );

VOID
CmpInitCallback(VOID);

VOID
CmpMarkCurrentValueDirty(
                         IN PHHIVE SystemHive,
                         IN HCELL_INDEX RootCell
                         );

#ifdef ALLOC_PRAGMA
NTSTATUS
CmpHwprofileDefaultSelect (
    IN  PCM_HARDWARE_PROFILE_LIST ProfileList,
    OUT PULONG ProfileIndexToUse,
    IN  PVOID Context
    );
#pragma alloc_text(INIT,CmInitSystem1)
#pragma alloc_text(INIT,CmIsLastKnownGoodBoot)
#pragma alloc_text(INIT,CmpHwprofileDefaultSelect)
#pragma alloc_text(INIT,CmpCreateControlSet)
#pragma alloc_text(INIT,CmpCloneControlSet)
#pragma alloc_text(INIT,CmpCreateObjectTypes)
#pragma alloc_text(INIT,CmpCreateRegistryRoot)
#pragma alloc_text(INIT,CmpCreateRootNode)
#pragma alloc_text(INIT,CmpInitializeSystemHive)
#pragma alloc_text(INIT,CmGetSystemDriverList)
#pragma alloc_text(INIT,CmpFreeDriverList)
#pragma alloc_text(INIT,CmpSetSystemValues)
#pragma alloc_text(INIT,CmpSetNetworkValue)
#pragma alloc_text(PAGE,CmpInitializeHiveList)
#pragma alloc_text(PAGE,CmpLinkHiveToMaster)
#pragma alloc_text(PAGE,CmBootLastKnownGood)
#pragma alloc_text(PAGE,CmpSaveBootControlSet)
#pragma alloc_text(PAGE,CmpInitHiveFromFile)
#pragma alloc_text(PAGE,CmpLinkKeyToHive)
#pragma alloc_text(PAGE,CmpCreatePredefined)
#pragma alloc_text(PAGE,CmpCreatePerfKeys)
#pragma alloc_text(PAGE,CmpInterlockedFunction)
#pragma alloc_text(PAGE,CmpConfigureProcessors)
#pragma alloc_text(INIT,CmpAddDockingInfo)
#pragma alloc_text(INIT,CmpAddAliasEntry)
#pragma alloc_text(PAGE,CmpDeleteCloneTree)
#pragma alloc_text(PAGE,CmpSetupPrivateWrite)
#pragma alloc_text(PAGE,CmpLoadHiveThread)
#pragma alloc_text(PAGE,CmpMarkCurrentValueDirty)
#endif



BOOLEAN
CmInitSystem1(
    __in PLOADER_PARAMETER_BLOCK LoaderBlock
    )
/*++

Routine Description:

    This function is called as part of phase1 init, after the object
    manager has been initialized, but before IoInit.  It's purpose is to
    set up basic registry object operations, and transform data
    captured during boot into registry format (whether it was read
    from the SYSTEM hive file by the osloader or computed by recognizers.)
    After this call, Nt*Key calls work, but only part of the name
    space is available and any changes written must be held in
    memory.

    CmpMachineHiveList entries marked CM_PHASE_1 are available
    after return from this call, but writes must be held in memory.

    This function will:

        1.  Create the registry worker/lazy-write thread
        2.  Create the registry key object type
        4.  Create the master hive
        5.  Create the \REGISTRY node
        6.  Create a KEY object that refers to \REGISTRY
        7.  Create \REGISTRY\MACHINE node
        8.  Create the SYSTEM hive, fill in with data from loader
        9.  Create the HARDWARE hive, fill in with data from loader
       10.  Create:
                \REGISTRY\MACHINE\SYSTEM
                \REGISTRY\MACHINE\HARDWARE
                Both of which will be link nodes in the master hive.

    NOTE:   We do NOT free allocated pool in failure case.  This is because
            our caller is going to bugcheck anyway, and having the memory
            object to look at is useful.

Arguments:

    LoaderBlock - supplies the LoaderBlock passed in from the OSLoader.
        By looking through the memory descriptor list we can find the
        SYSTEM hive which the OSLoader has placed in memory for us.

Return Value:

    TRUE if all operations were successful, false if any failed.

    Bugchecks when something went wrong (CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,.....)

--*/
{
    HANDLE  key1;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS    status;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    PCMHIVE HardwareHive;

    //
    // Set the mini NT flag if we are booting into Mini NT 
    // environment
    //
    if (InitIsWinPEMode) {
        CmpMiniNTBoot = InitIsWinPEMode;        

        //
        // On Remote boot client share the system hives
        //
        // NOTE : We can't assume exclusive access to WinPE
        // remote boot clients. We don't flush anything to 
        // system hives in WinPE. All the system hives are 
        // loaded in memory in scratch mode
        //
        CmpShareSystemHives = TRUE;
    }

    CM_PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"CmInitSystem1\n"));

    //
    // Initialize Names of all registry paths.
    // This simply initializes unicode strings so we don't have to bother
    // with it later. This can not fail.
    //
    CmpInitializeRegistryNames();

    //
    // Compute registry global quota
    //
    CmpComputeGlobalQuotaAllowed();

    //
    // Initialize the hive list head
    //
    InitializeListHead(&CmpHiveListHead);
    ExInitializePushLock(&CmpHiveListHeadLock);
    ExInitializePushLock(&CmpLoadHiveLock);

    //
    // Initialize the global registry resource
    //
    ExInitializeResourceLite(&CmpRegistryLock);

    //
    // Initialize the PostList mutex
    //
    ExInitializePushLock(&CmpPostLock);

    //
    // Initialize the Stash Buffer mutex
    //
    ExInitializePushLock(&CmpStashBufferLock);

    //
    // Initialize the cache
    //
    CmpInitializeCache ();

    //
    // Initialize private allocator
    //
    CmpInitCmPrivateAlloc();
    CmpInitCmPrivateDelayAlloc();

    //
    // delay deref kcb
    //
    CmpInitDelayDerefKCBEngine();

    //
    // Initialize callback module
    //
    CmpInitCallback();

	//
	// Self Heal workitem queue
	//
    InitializeListHead(&CmpSelfHealQueueListHead);
    KeInitializeGuardedMutex(&CmpSelfHealQueueLock);

    //
    // start tracking quota allocations
    //
    CM_TRACK_QUOTA_START();

    //
    // Save the current process to allow us to attach to it later.
    //
    CmpSystemProcess = &PsGetCurrentProcess()->Pcb;

    CmpLockRegistryExclusive();

    //
    // Create the Key object type.
    //
    status = CmpCreateObjectTypes();
    if (!NT_SUCCESS(status) ) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmInitSystem1: CmpCreateObjectTypes failed\n"));
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,1,status,0); // could not register with object manager
    }


    //
    // Create the master hive and initialize it.
    //
    status = CmpInitializeHive(&CmpMasterHive,
                HINIT_CREATE,
                HIVE_VOLATILE,
                HFILE_TYPE_PRIMARY,     // i.e. no logging, no alterate
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                0);
    if (!NT_SUCCESS(status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmInitSystem1: CmpInitializeHive(master) failed\n"));

        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,2,status,0); // could not initialize master hive
    }

    //
    // try to allocate a stash buffer.  if we can't get 1 page this
    // early on, we're in deep trouble, so punt.
    //
    CmpStashBuffer = ExAllocatePoolWithTag(PagedPool, PAGE_SIZE,CM_STASHBUFFER_TAG);
    if (CmpStashBuffer == NULL) {
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,3,0,0); // odds against this are huge
    }
    CmpStashBufferSize = PAGE_SIZE;

    //
    // Create the \REGISTRY node
    //
    if (!CmpCreateRegistryRoot()) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmInitSystem1: CmpCreateRegistryRoot failed\n"));
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,4,0,0); // could not create root of the registry
    }

    //
    // --- 6. Create \REGISTRY\MACHINE and \REGISTRY\USER nodes ---
    //

    //
    // Get default security descriptor for the nodes we will create.
    //
    SecurityDescriptor = CmpHiveRootSecurityDescriptor();

    InitializeObjectAttributes(
        &ObjectAttributes,
        &CmRegistryMachineName,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        SecurityDescriptor
        );

    if (!NT_SUCCESS(status = NtCreateKey(
                        &key1,
                        KEY_READ | KEY_WRITE,
                        &ObjectAttributes,
                        0,
                        (PUNICODE_STRING)&nullclass,
                        0,
                        NULL
        )))
    {
        ExFreePool(SecurityDescriptor);
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmInitSystem1: NtCreateKey(MACHINE) failed\n"));
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,5,status,0); // could not create HKLM
    }

    NtClose(key1);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &CmRegistryUserName,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        SecurityDescriptor
        );

    if (!NT_SUCCESS(status = NtCreateKey(
                        &key1,
                        KEY_READ | KEY_WRITE,
                        &ObjectAttributes,
                        0,
                        (PUNICODE_STRING)&nullclass,
                        0,
                        NULL
        )))
    {
        ExFreePool(SecurityDescriptor);
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmInitSystem1: NtCreateKey(USER) failed\n"));
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,6,status,0); // could not create HKUSER
    }

    NtClose(key1);


    //
    // --- 7. Create the SYSTEM hive, fill in with data from loader ---
    //
    if (!CmpInitializeSystemHive(LoaderBlock)) {
        ExFreePool(SecurityDescriptor);

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpInitSystem1: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"Hive allocation failure for SYSTEM\n"));

        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,7,0,0); // could not create SystemHive
    }

    //
    // Create the symbolic link \Registry\Machine\System\CurrentControlSet
    //
    status = CmpCreateControlSet(LoaderBlock);
    if (!NT_SUCCESS(status)) {
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,8,status,0); // could not create CurrentControlSet
    }

    //
    // --- 8. Create the HARDWARE hive, fill in with data from loader ---
    //
    status = CmpInitializeHive(&HardwareHive,
                HINIT_CREATE,
                HIVE_VOLATILE,
                HFILE_TYPE_PRIMARY,     // i.e. no log, no alternate
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                0);
    if (!NT_SUCCESS(status)) {
        ExFreePool(SecurityDescriptor);

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpInitSystem1: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"Could not initialize HARDWARE hive\n"));

        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,11,status,0); // could not initialize hardware hive
    }

    //
    // Allocate the root node
    //
    status = CmpLinkHiveToMaster(
            &CmRegistryMachineHardwareName,
            NULL,
            HardwareHive,
            TRUE,
            SecurityDescriptor
            );
    if ( status != STATUS_SUCCESS )
    {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmInitSystem1: CmpLinkHiveToMaster(Hardware) failed\n"));
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,12,status,0); // could not link hardware hive to master hive
    }
    CmpAddToHiveFileList(HardwareHive);

    ExFreePool(SecurityDescriptor);

    CmpMachineHiveList[0].CmHive = HardwareHive;

    //
    // put loader configuration tree data to our hardware registry.
    //
    status = CmpInitializeHardwareConfiguration(LoaderBlock);

    if (!NT_SUCCESS(status)) {
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,13,status,0); // could not initialize hardware configuration
    }

    CmpNoMasterCreates = TRUE;
    CmpUnlockRegistry();

    //
    // put machine dependent configuration data to our hardware registry.
    //
    status = CmpInitializeMachineDependentConfiguration(LoaderBlock);
    if (!NT_SUCCESS(status)) {
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,14,status,0); // could not open CurrentControlSet\\Control
    }

    //
    // Write system start options to registry
    //
    status = CmpSetSystemValues(LoaderBlock);
    if (!NT_SUCCESS(status)) {
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,15,status,0);
    }

    ExFreePool(CmpLoadOptions.Buffer);

    //
    // Write Network LoaderBlock values to registry
    //
    if ( (LoaderBlock->Extension->Size >=
            RTL_SIZEOF_THROUGH_FIELD(LOADER_PARAMETER_EXTENSION, NetworkLoaderBlock)) &&
         (LoaderBlock->Extension->NetworkLoaderBlock != NULL) ) {
        status = CmpSetNetworkValue(LoaderBlock->Extension->NetworkLoaderBlock);
        if (!NT_SUCCESS(status)) {
            CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,16,status,0);
        }
    }
    
    return TRUE;
}

//
// All parallel threads will get this shared, and CmpInitializeHiveList will wait for it exclusive
//
KEVENT  CmpLoadWorkerEvent;
LONG   CmpLoadWorkerIncrement = 0;
KEVENT  CmpLoadWorkerDebugEvent;

VOID
CmpInitializeHiveList(
    VOID
    )
/*++

Routine Description:

    This function is called to map hive files to hives.  It both
    maps existing hives to files, and creates new hives from files.

    It operates on files in "\SYSTEMROOT\CONFIG".

    NOTE:   MUST run in the context of the process that the CmpWorker
            thread runs in.  Caller is expected to arrange this.

    NOTE:   Will bugcheck on failure.

Arguments:

Return Value:

    NONE.

--*/
{
    #define MAX_NAME    128
    HANDLE  Thread;
    NTSTATUS Status;

    UCHAR   FileBuffer[MAX_NAME];
    UCHAR   RegBuffer[MAX_NAME];

    UNICODE_STRING TempName;
    UNICODE_STRING FileName;
    UNICODE_STRING RegName;

    USHORT  FileStart;
    USHORT  RegStart;
    ULONG   i;
    PSECURITY_DESCRIPTOR SecurityDescriptor;

    
    CM_PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"CmpInitializeHiveList\n"));

    CmpNoWrite = FALSE;

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    FileName.MaximumLength = MAX_NAME;
    FileName.Length = 0;
    FileName.Buffer = (PWSTR)&(FileBuffer[0]);

    RegName.MaximumLength = MAX_NAME;
    RegName.Length = 0;
    RegName.Buffer = (PWSTR)&(RegBuffer[0]);

    RtlInitUnicodeString(
        &TempName,
        INIT_SYSTEMROOT_HIVEPATH
        );
    RtlAppendStringToString((PSTRING)&FileName, (PSTRING)&TempName);
    FileStart = FileName.Length;

    RtlInitUnicodeString(
        &TempName,
        INIT_REGISTRY_MASTERPATH
        );
    RtlAppendStringToString((PSTRING)&RegName, (PSTRING)&TempName);
    RegStart = RegName.Length;

    //
    // Initialize the synchronization event
    //
    KeInitializeEvent (&CmpLoadWorkerEvent, SynchronizationEvent, FALSE);
    KeInitializeEvent (&CmpLoadWorkerDebugEvent, SynchronizationEvent, FALSE);
    
    CmpSpecialBootCondition = TRUE;

    SecurityDescriptor = CmpHiveRootSecurityDescriptor();

    if (CmpShareSystemHives) {
        for (i = 0; i < CM_NUMBER_OF_MACHINE_HIVES; i++) {
            if (CmpMachineHiveList[i].Name) {
                CmpMachineHiveList[i].HHiveFlags |= HIVE_VOLATILE;
            }
        }
    }        

    for (i = 0; i < CM_NUMBER_OF_MACHINE_HIVES; i++) {
        ASSERT( CmpMachineHiveList[i].Name != NULL );
        //
        // just spawn the Threads to load the hives in parallel
        //
        Status = PsCreateSystemThread(
            &Thread,
            THREAD_ALL_ACCESS,
            NULL,
            0,
            NULL,
            CmpLoadHiveThread,
            (PVOID)(ULONG_PTR)(ULONG)i
            );

        if (NT_SUCCESS(Status)) {
            ZwClose(Thread);
        } else {
            //
            // cannot spawn thread; Fatal error
            //
            CM_BUGCHECK(BAD_SYSTEM_CONFIG_INFO,BAD_HIVE_LIST,3,i,Status);
        }
    }
    ASSERT( CmpMachineHiveList[i].Name == NULL );

    KeWaitForSingleObject( &CmpLoadWorkerEvent,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );
    
    CmpSpecialBootCondition = FALSE;
    ASSERT( CmpLoadWorkerIncrement == CM_NUMBER_OF_MACHINE_HIVES );
    //
    // Now add all hives to the hivelist
    //
    for (i = 0; i < CM_NUMBER_OF_MACHINE_HIVES; i++) {        
        ASSERT( CmpMachineHiveList[i].ThreadFinished == TRUE );
        ASSERT( CmpMachineHiveList[i].ThreadStarted == TRUE );

        if (CmpMachineHiveList[i].CmHive == NULL) {
            
            ASSERT( CmpMachineHiveList[i].CmHive2 != NULL );

            //
            // Compute the name of the file, and the name to link to in
            // the registry.
            //

            // REGISTRY

            RegName.Length = RegStart;
            RtlInitUnicodeString(
                &TempName,
                CmpMachineHiveList[i].BaseName
                );
            RtlAppendStringToString((PSTRING)&RegName, (PSTRING)&TempName);

            // REGISTRY\MACHINE or REGISTRY\USER

            if (RegName.Buffer[ (RegName.Length / sizeof( WCHAR )) - 1 ] == '\\') {
                RtlInitUnicodeString(
                    &TempName,
                    CmpMachineHiveList[i].Name
                    );
                RtlAppendStringToString((PSTRING)&RegName, (PSTRING)&TempName);
            }

            // REGISTRY\[MACHINE|USER]\HIVE

            // <sysroot>\config


            //
            // Link hive into master hive
            //
            Status = CmpLinkHiveToMaster(
                    &RegName,
                    NULL,
                    CmpMachineHiveList[i].CmHive2,
                    CmpMachineHiveList[i].Allocate,
                    SecurityDescriptor
                    );
            if ( Status != STATUS_SUCCESS)
            {

                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpInitializeHiveList: "));
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpLinkHiveToMaster failed\n"));
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"\ti=%d s='%ws'\n", i, CmpMachineHiveList[i]));

                CM_BUGCHECK(CONFIG_LIST_FAILED,BAD_CORE_HIVE,Status,i,&RegName);
            }

			if( CmpMachineHiveList[i].Allocate == TRUE ) {
				HvSyncHive((PHHIVE)(CmpMachineHiveList[i].CmHive2));
			}
           
        } else {
            //
            // do nothing here as all of it has been done in separate thread.
            //
        }

        if( CmpMachineHiveList[i].CmHive2 != NULL ) {
            CmpAddToHiveFileList(CmpMachineHiveList[i].CmHive2);
        }

    }   // for

    ExFreePool(SecurityDescriptor);

    //
    // Create symbolic link from SECURITY hive into SAM hive.
    //
    CmpLinkKeyToHive(
        L"\\Registry\\Machine\\Security\\SAM",
        L"\\Registry\\Machine\\SAM\\SAM"
        );

    //
    // Create symbolic link from S-1-5-18 to .Default 
    //
    CmpNoMasterCreates = FALSE;     
    CmpLinkKeyToHive(
        L"\\Registry\\User\\S-1-5-18",
        L"\\Registry\\User\\.Default"
        );
    CmpNoMasterCreates = TRUE;     

    //
    // Create predefined handles.
    //
    CmpCreatePerfKeys();

    return;
}

NTSTATUS
CmpCreateObjectTypes(
    VOID
    )
/*++

Routine Description:

    Create the Key object type

Arguments:

    NONE.

Return Value:

    Status of the ObCreateType call

--*/
{
   NTSTATUS Status;
   OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
   UNICODE_STRING TypeName;

   //
   // Structure that describes the mapping of generic access rights to object
   // specific access rights for registry key objects.
   //

   GENERIC_MAPPING CmpKeyMapping = {
      KEY_READ,
      KEY_WRITE,
      KEY_EXECUTE,
      KEY_ALL_ACCESS
   };

    CM_PAGED_CODE();
    //
    // --- Create the registry key object type ---
    //

    //
    // Initialize string descriptor.
    //

    RtlInitUnicodeString(&TypeName, L"Key");

    //
    // Create key object type descriptor.
    //

    RtlZeroMemory(&ObjectTypeInitializer, sizeof(ObjectTypeInitializer));
    ObjectTypeInitializer.Length = sizeof(ObjectTypeInitializer);
    ObjectTypeInitializer.InvalidAttributes = CMP_KEY_INVALID_ATTRIBUTES;
    ObjectTypeInitializer.GenericMapping = CmpKeyMapping;
    ObjectTypeInitializer.ValidAccessMask = KEY_ALL_ACCESS;
    ObjectTypeInitializer.DefaultPagedPoolCharge = sizeof(CM_KEY_BODY);
    ObjectTypeInitializer.SecurityRequired = TRUE;
    ObjectTypeInitializer.PoolType = PagedPool;
    ObjectTypeInitializer.MaintainHandleCount = FALSE;
    ObjectTypeInitializer.UseDefaultObject = TRUE;

    ObjectTypeInitializer.DumpProcedure = NULL;
    ObjectTypeInitializer.OpenProcedure = NULL;
    ObjectTypeInitializer.CloseProcedure = CmpCloseKeyObject;
    ObjectTypeInitializer.DeleteProcedure = CmpDeleteKeyObject;
    ObjectTypeInitializer.ParseProcedure = CmpParseKey;
    ObjectTypeInitializer.SecurityProcedure = CmpSecurityMethod;
    ObjectTypeInitializer.QueryNameProcedure = CmpQueryKeyName;

    Status = ObCreateObjectType(
                &TypeName,
                &ObjectTypeInitializer,
                (PSECURITY_DESCRIPTOR)NULL,
                &CmpKeyObjectType
                );


    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpCreateObjectTypes: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"ObCreateObjectType(Key) failed %08lx\n", Status));
    }

    return Status;
}



BOOLEAN
CmpCreateRegistryRoot(
    VOID
    )
/*++

Routine Description:

    Manually create \REGISTRY in the master hive, create a key
    object to refer to it, and insert the key object into
    the root (\) of the object space.

Arguments:

    None

Return Value:

    TRUE == success, FALSE == failure

--*/
{
    NTSTATUS                Status;
    PVOID                   ObjectPointer;
    PCM_KEY_BODY            Object;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    PCM_KEY_CONTROL_BLOCK   kcb;
    HCELL_INDEX             RootCellIndex;
    PSECURITY_DESCRIPTOR    SecurityDescriptor;
    PCM_KEY_NODE            TempNode;

    CM_PAGED_CODE();
    //
    // --- Create hive entry for \REGISTRY ---
    //

    if (!CmpCreateRootNode(
            &(CmpMasterHive->Hive), L"REGISTRY", &RootCellIndex))
    {
        return FALSE;
    }

    //
    // --- Create a KEY object that refers to \REGISTRY ---
    //


    //
    // Create the object manager object
    //

    //
    // WARNING: \\REGISTRY is not in pool, so if anybody ever tries to
    //          free it, we are in deep trouble.  On the other hand,
    //          this implies somebody has removed \\REGISTRY from the
    //          root, so we're in trouble anyway.
    //

    SecurityDescriptor = CmpHiveRootSecurityDescriptor();

    InitializeObjectAttributes(
        &ObjectAttributes,
        &CmRegistryRootName,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        SecurityDescriptor
        );


    Status = ObCreateObject(
        KernelMode,
        CmpKeyObjectType,
        &ObjectAttributes,
        UserMode,
        NULL,                   // Parse context
        sizeof(CM_KEY_BODY),
        0,
        0,
        (PVOID *)&Object
        );

    ExFreePool(SecurityDescriptor);

    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpCreateRegistryRoot: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"ObCreateObject(\\REGISTRY) failed %08lx\n", Status));
        return FALSE;
    }

    ASSERT( (&CmpMasterHive->Hive)->ReleaseCellRoutine == NULL );
    TempNode = (PCM_KEY_NODE)HvGetCell(&CmpMasterHive->Hive,RootCellIndex);
    if( TempNode == NULL ) {
        //
        // we couldn't map the bin containing this cell
        //
        return FALSE;
    }
    //
    // Create the key control block
    //
    kcb = CmpCreateKeyControlBlock(
            &(CmpMasterHive->Hive),
            RootCellIndex,
            TempNode,
            NULL,
            0,
            &CmRegistryRootName
            );

    if (kcb==NULL) {
        return(FALSE);
    }

    //
    // Initialize the type specific body
    //
    Object->Type = KEY_BODY_TYPE;
    Object->KeyControlBlock = kcb;
    Object->NotifyBlock = NULL;
    Object->ProcessID = PsGetCurrentProcessId();
    EnlistKeyBodyWithKCB(Object,0);

    //
    // Put the object in the root directory
    //
    Status = ObInsertObject(
                Object,
                NULL,
                (ACCESS_MASK)0,
                0,
                NULL,
                &CmpRegistryRootHandle
                );

    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpCreateRegistryRoot: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"ObInsertObject(\\REGISTRY) failed %08lx\n", Status));
        return FALSE;
    }

    //
    // We cannot make the root permanent because registry objects in
    // general are not allowed to be.  (They're stable via virtue of being
    // stored in the registry, not the object manager.)  But we never
    // ever want the root to go away.  So reference it.
    //
    if (! NT_SUCCESS(Status = ObReferenceObjectByHandle(
                        CmpRegistryRootHandle,
                        KEY_READ,
                        NULL,
                        KernelMode,
                        &ObjectPointer,
                        NULL
                        )))
    {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpCreateRegistryRoot: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"ObReferenceObjectByHandle failed %08lx\n", Status));
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
CmpCreateRootNode(
    IN PHHIVE   Hive,
    IN PWSTR    Name,
    OUT PHCELL_INDEX RootCellIndex
    )
/*++

Routine Description:

    Manually create the root node of a hive.

Arguments:

    Hive - pointer to a Hive (Hv level) control structure

    Name - pointer to a unicode name string

    RootCellIndex - supplies pointer to a variable to receive
                    the cell index of the created node.

Return Value:

    TRUE == success, FALSE == failure

--*/
{
    UNICODE_STRING temp;
    PCELL_DATA CellData;
    CM_KEY_REFERENCE Key;
    LARGE_INTEGER systemtime;

    CM_PAGED_CODE();
    //
    // Allocate the node.
    //
    RtlInitUnicodeString(&temp, Name);
    *RootCellIndex = HvAllocateCell(
                Hive,
                CmpHKeyNodeSize(Hive, &temp),
                Stable,
                HCELL_NIL
                );
    if (*RootCellIndex == HCELL_NIL) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpCreateRootNode: HvAllocateCell failed\n"));
        return FALSE;
    }

    Hive->BaseBlock->RootCell = *RootCellIndex;

    CellData = HvGetCell(Hive, *RootCellIndex);
    if( CellData == NULL ) {
        //
        // we couldn't map the bin containing this cell
        //
        return FALSE;
    }

    //
    // Initialize the node
    //
    CellData->u.KeyNode.Signature = CM_KEY_NODE_SIGNATURE;
    CellData->u.KeyNode.Flags = KEY_HIVE_ENTRY | KEY_NO_DELETE;
    KeQuerySystemTime(&systemtime);
    CellData->u.KeyNode.LastWriteTime = systemtime;
//    CellData->u.KeyNode.TitleIndex = 0;
    CellData->u.KeyNode.Parent = HCELL_NIL;

    CellData->u.KeyNode.SubKeyCounts[Stable] = 0;
    CellData->u.KeyNode.SubKeyCounts[Volatile] = 0;
    CellData->u.KeyNode.SubKeyLists[Stable] = HCELL_NIL;
    CellData->u.KeyNode.SubKeyLists[Volatile] = HCELL_NIL;

    CellData->u.KeyNode.ValueList.Count = 0;
    CellData->u.KeyNode.ValueList.List = HCELL_NIL;
    CellData->u.KeyNode.Security = HCELL_NIL;
    CellData->u.KeyNode.Class = HCELL_NIL;
    CellData->u.KeyNode.ClassLength = 0;

    CellData->u.KeyNode.MaxValueDataLen = 0;
    CellData->u.KeyNode.MaxNameLen = 0;
    CellData->u.KeyNode.MaxValueNameLen = 0;
    CellData->u.KeyNode.MaxClassLen = 0;

    CellData->u.KeyNode.NameLength = CmpCopyName(Hive,
                                                 CellData->u.KeyNode.Name,
                                                 &temp);
    if (CellData->u.KeyNode.NameLength < temp.Length) {
        CellData->u.KeyNode.Flags |= KEY_COMP_NAME;
    }

    Key.KeyHive = Hive;
    Key.KeyCell = *RootCellIndex;

    HvReleaseCell(Hive, *RootCellIndex);

    return TRUE;
}


NTSTATUS
CmpLinkHiveToMaster(
    PUNICODE_STRING LinkName,
    HANDLE RootDirectory,
    PCMHIVE CmHive,
    BOOLEAN Allocate,
    PSECURITY_DESCRIPTOR SecurityDescriptor
    )
/*++

Routine Description:

    The existing, "free floating" hive CmHive describes is linked into
    the name space at the node named by LinkName.  The node will be created.
    The hive is assumed to already have an appropriate root node.

Arguments:

    LinkName - supplies a pointer to a unicode string which describes where
                in the registry name space the hive is to be linked.
                All components but the last must exist.  The last must not.

    RootDirectory - Supplies the handle the LinkName is relative to.

    CmHive - pointer to a CMHIVE structure describing the hive to link in.

    Allocate - TRUE indicates that the root cell is to be created
               FALSE indicates the root cell already exists.

    SecurityDescriptor - supplies a pointer to the security descriptor to
               be placed on the hive root.

Return Value:

    TRUE == success, FALSE == failure

--*/
{
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE              KeyHandle;
    CM_PARSE_CONTEXT    ParseContext;
    NTSTATUS            Status;
    PCM_KEY_BODY        KeyBody;

    CM_PAGED_CODE();

    //
    // Fill in special ParseContext to indicate that we are creating
    // a link node and opening or creating a root node.
    //
    ParseContext.TitleIndex = 0;
    ParseContext.Class.Length = 0;
    ParseContext.Class.MaximumLength = 0;
    ParseContext.Class.Buffer = NULL;
    ParseContext.CreateOptions = 0;
    ParseContext.CreateLink = TRUE;
    ParseContext.ChildHive.KeyHive = &CmHive->Hive;
    ParseContext.CreateOperation = TRUE;
    ParseContext.OriginatingPoint = NULL;
    if (Allocate) {
        //
        // Creating a new root node
        //
        ParseContext.ChildHive.KeyCell = HCELL_NIL;
    } else {
        //
        // Opening an existing root node
        //
        ParseContext.ChildHive.KeyCell = CmHive->Hive.BaseBlock->RootCell;
    }

    //
    // Create a path to the hive
    //
    InitializeObjectAttributes(
        &ObjectAttributes,
        LinkName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        (HANDLE)RootDirectory,
        SecurityDescriptor
        );

    Status = ObOpenObjectByName( &ObjectAttributes,
                                 CmpKeyObjectType,
                                 KernelMode,
                                 NULL,
                                 KEY_READ | KEY_WRITE,
                                 (PVOID)&ParseContext,
                                 &KeyHandle );

    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpLinkHiveToMaster: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"ObOpenObjectByName() failed %08lx\n", Status));
        return Status;
    }

    //
    // mark hive as "clean" 
    //
    CmHive->Hive.DirtyFlag = FALSE;

    //
    // Report the notification event
    //
    Status = ObReferenceObjectByHandle(KeyHandle,
                                       0,
                                       CmpKeyObjectType,
                                       KernelMode,
                                       (PVOID *)&KeyBody,
                                       NULL);
    ASSERT(NT_SUCCESS(Status));
    if (NT_SUCCESS(Status)) {
        CmpReportNotify(KeyBody->KeyControlBlock,
                        KeyBody->KeyControlBlock->KeyHive,
                        KeyBody->KeyControlBlock->KeyCell,
                        REG_NOTIFY_CHANGE_NAME);

        ObDereferenceObject((PVOID)KeyBody);
    }

    ZwClose(KeyHandle);
    return STATUS_SUCCESS;
}


NTSTATUS
CmpInterlockedFunction (
    PWCHAR RegistryValueKey,
    VOID (*InterlockedFunction)(VOID)
    )
/*++

Routine Description:

    This routine guards calling the InterlockedFunction in the
    passed RegistryValueKey.

    The RegistryValueKey will record the status of the first
    call to the InterlockedFunction.  If the system crashes
    during this call then ValueKey will be left in a state
    where the InterlockedFunction will not be called on subsequent
    attempts.

Arguments:

    RegistryValueKey - ValueKey name for Control\Session Manager
    InterlockedFunction - Function to call

Return Value:

    STATUS_SUCCESS  - The interlocked function was successfully called


--*/
{
    OBJECT_ATTRIBUTES   objectAttributes;
    HANDLE              hControl, hSession;
    UNICODE_STRING      Name;
    UCHAR               Buffer [sizeof(KEY_VALUE_PARTIAL_INFORMATION)+sizeof(ULONG)];
    ULONG               length, Value;
    NTSTATUS            status;

    CM_PAGED_CODE();

    //
    // Open CurrentControlSet
    //

    InitializeObjectAttributes (
        &objectAttributes,
        &CmRegistryMachineSystemCurrentControlSet,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenKey (&hControl, KEY_READ | KEY_WRITE, &objectAttributes);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    //
    // Open Control\Session Manager
    //

    RtlInitUnicodeString (&Name, CmpControlSessionManager);
    InitializeObjectAttributes (
        &objectAttributes,
        &Name,
        OBJ_CASE_INSENSITIVE,
        hControl,
        NULL
        );

    status = NtOpenKey (&hSession, KEY_READ | KEY_WRITE, &objectAttributes );
    NtClose (hControl);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    //
    // Read ValueKey to interlock operation with
    //

    RtlInitUnicodeString (&Name, RegistryValueKey);
    status = NtQueryValueKey (hSession,
                              &Name,
                              KeyValuePartialInformation,
                              Buffer,
                              sizeof (Buffer),
                              &length );

    Value = 0;
    if (NT_SUCCESS(status)) {
        Value = ((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data[0];
    }

    //
    // Value 0  - Before InterlockedFunction
    //       1  - In the middle of InterlockedFunction
    //       2  - After InterlockedFunction
    //
    // If the value is a 0, then we haven't tried calling this
    // interlocked function, set the value to a 1 and try it.
    //
    // If the value is a 1, then we crased during an execution
    // of the interlocked function last time, don't try it again.
    //
    // If the value is a 2, then we called the interlocked function
    // before and it worked.  Call it again this time.
    //

    if (Value != 1) {

        if (Value != 2) {
            //
            // This interlocked function is not known to work.  Write
            // a 1 to this value so we can detect if we crash during
            // this call.
            //

            Value = 1;
            NtSetValueKey (hSession, &Name, 0L, REG_DWORD, &Value, sizeof (Value));
            NtFlushKey    (hSession);   // wait until it's on the disk
        }

        InterlockedFunction();

        if (Value != 2) {
            //
            // The worker function didn't crash - update the value for
            // this interlocked function to 2.
            //

            Value = 2;
            NtSetValueKey (hSession, &Name, 0L, REG_DWORD, &Value, sizeof (Value));
        }

    } else {
        status = STATUS_UNSUCCESSFUL;
    }

    NtClose (hSession);
    return status;
}

VOID
CmpConfigureProcessors (
    VOID
    )

/*++

Routine Description:

    Set each processor to it's optimal settings for NT.

--*/

{

#if defined(_X86_)

    ULONG   i;

    CM_PAGED_CODE();

    //
    // Set each processor into its best NT configuration
    //

    for (i=0; i < (ULONG)KeNumberProcessors; i++) {
        KeSetSystemAffinityThread(AFFINITY_MASK(i));
        KeOptimizeProcessorControlState ();
    }

    //
    // Restore threads affinity
    //

    KeRevertToUserAffinityThread();

#endif

    return;
}

BOOLEAN
CmpInitializeSystemHive(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
/*++

Routine Description:

    Initializes the SYSTEM hive based on the raw hive image passed in
    from the OS Loader.

Arguments:

    LoaderBlock - Supplies a pointer to the Loader Block passed in by
        the OS Loader.

Return Value:

    TRUE - it worked

    FALSE - it failed

--*/

{
    PCMHIVE SystemHive;
    PVOID HiveImageBase;
    BOOLEAN Allocate=FALSE;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    NTSTATUS Status;
    STRING  TempString;


    CM_PAGED_CODE();

    //
    // capture tail of boot.ini line (load options, portable)
    //
    RtlInitAnsiString(
        &TempString,
        LoaderBlock->LoadOptions
        );

    CmpLoadOptions.Length = 0;
    CmpLoadOptions.MaximumLength = (TempString.Length+1)*sizeof(WCHAR);
    CmpLoadOptions.Buffer = ExAllocatePool(
                                PagedPool, (TempString.Length+1)*sizeof(WCHAR));

    if (CmpLoadOptions.Buffer == NULL) {
        CM_BUGCHECK(BAD_SYSTEM_CONFIG_INFO,BAD_SYSTEM_HIVE,1,LoaderBlock,0);
    }
    RtlAnsiStringToUnicodeString(
        &CmpLoadOptions,
        &TempString,
        FALSE
        );
    CmpLoadOptions.Buffer[TempString.Length] = UNICODE_NULL;
    CmpLoadOptions.Length += sizeof(WCHAR);


    //
    // move the loaded registry into the real registry
    //
    HiveImageBase = LoaderBlock->RegistryBase;

    //
    // We need to initialize the system hive as NO_LAZY_FLUSH
    //  - this is just temporary, until we get a chance to open the primary
    // file for the hive. Failure to do so, will result in loss of data on the
    // LazyFlush worker (see CmpFileWrite, the
    //          if (FileHandle == NULL) {
    //              return TRUE;
    //          }
    // test. This might be a problem in 5.0 too, if system crashes between the
    // LazyFlush reported the hive as saved and the moment we actually open the
    // file and save it again
    //
    if (HiveImageBase == NULL) {
        //
        // No memory descriptor for the hive, so we must recreate it.
        //
        Status = CmpInitializeHive(&SystemHive,
                    HINIT_CREATE,
                    HIVE_NOLAZYFLUSH,
                    HFILE_TYPE_LOG,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &CmpSystemFileName,
                    0);
        if (!NT_SUCCESS(Status)) {

            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpInitializeSystemHive: "));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"Couldn't initialize newly allocated SYSTEM hive\n"));

            return(FALSE);
        }
        Allocate = TRUE;

    } else {

        //
        // There is a memory image for the hive, copy it and make it active
        //
        Status = CmpInitializeHive(&SystemHive,
                    HINIT_MEMORY,
                    HIVE_NOLAZYFLUSH,
                    HFILE_TYPE_LOG,
                    HiveImageBase,
                    NULL,
                    NULL,
                    NULL,
                    &CmpSystemFileName,
                    CM_CHECK_REGISTRY_SYSTEM_CLEAN);
        if (!NT_SUCCESS(Status)) {

            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpInitializeSystemHive: "));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"Couldn't initialize OS Loader-loaded SYSTEM hive\n"));

            CM_BUGCHECK(BAD_SYSTEM_CONFIG_INFO,BAD_SYSTEM_HIVE,2,SystemHive,Status);
        }

        Allocate = FALSE;

        //
        // Mark the system hive as volatile, while in MiniNT boot
        // case
        //
        if (CmpShareSystemHives) {
            SystemHive->Hive.HiveFlags = HIVE_VOLATILE;
        }
    }

    CmpBootType = SystemHive->Hive.BaseBlock->BootType;

    if( !CmSelfHeal ) {

        CmpSelfHeal = FALSE;

        if(CmpBootType & HBOOT_SELFHEAL)  {
            //
            // self healing disabled; but loader has detected corrupted system hive
            //
            CM_BUGCHECK(BAD_SYSTEM_CONFIG_INFO,BAD_SYSTEM_HIVE,3,SystemHive,0);
        }
    }

    //
    // Create the link node
    //
    SecurityDescriptor = CmpHiveRootSecurityDescriptor();

    Status = CmpLinkHiveToMaster(&CmRegistryMachineSystemName,
                                 NULL,
                                 SystemHive,
                                 Allocate,
                                 SecurityDescriptor);
    ExFreePool(SecurityDescriptor);

    if (!NT_SUCCESS(Status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmInitSystem1: CmpLinkHiveToMaster(Hardware) failed\n"));

        return(FALSE);
    }

    CmpMachineHiveList[SYSTEM_HIVE_INDEX].CmHive = SystemHive;

    return(TRUE);
}


PHANDLE
CmGetSystemDriverList(
    VOID
    )

/*++

Routine Description:

    Traverses the current SERVICES subtree and creates the list of drivers
    to be loaded during Phase 1 initialization.

Arguments:

    None

Return Value:

    A pointer to an array of handles, each of which refers to a key in
    the \Services section of the control set.  The caller will traverse
    this array and load and initialize the drivers described by the keys.

    The last key will be NULL.  The array is allocated in Pool and should
    be freed by the caller.

--*/

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE SystemHandle;
    UNICODE_STRING Name;
    NTSTATUS Status;
    PCM_KEY_BODY KeyBody;
    LIST_ENTRY DriverList;
    PHHIVE Hive;
    HCELL_INDEX RootCell;
    HCELL_INDEX ControlCell;
    ULONG DriverCount;
    PLIST_ENTRY Current;
    PHANDLE Handle;
    PBOOT_DRIVER_LIST_ENTRY DriverEntry;
    BOOLEAN Success;
    BOOLEAN AutoSelect;

    CM_PAGED_CODE();
    InitializeListHead(&DriverList);
    RtlInitUnicodeString(&Name,
                         L"\\Registry\\Machine\\System");

    InitializeObjectAttributes(&ObjectAttributes,
                               &Name,
                               OBJ_CASE_INSENSITIVE,
                               (HANDLE)NULL,
                               NULL);
    Status = NtOpenKey(&SystemHandle,
                       KEY_READ,
                       &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmGetSystemDriverList couldn't open registry key %wZ\n",&Name));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM:     status %08lx\n", Status));

        return(NULL);
    }


    Status = ObReferenceObjectByHandle( SystemHandle,
                                        KEY_QUERY_VALUE,
                                        CmpKeyObjectType,
                                        KernelMode,
                                        (PVOID *)(&KeyBody),
                                        NULL );
    if (!NT_SUCCESS(Status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmGetSystemDriverList couldn't dereference System handle\n"));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM:     status %08lx\n", Status));

        NtClose(SystemHandle);
        return(NULL);
    }

    CmpLockRegistryExclusive();

    Hive = KeyBody->KeyControlBlock->KeyHive;
    RootCell = KeyBody->KeyControlBlock->KeyCell;

    //
    // Now we have found out the PHHIVE and HCELL_INDEX of the root of the
    // SYSTEM hive, we can use all the same code that the OS Loader does.
    //

    RtlInitUnicodeString(&Name, L"Current");
    ControlCell = CmpFindControlSet(Hive,
                                    RootCell,
                                    &Name,
                                    &AutoSelect);
    if (ControlCell == HCELL_NIL) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmGetSystemDriverList couldn't find control set\n"));

        CmpUnlockRegistry();
        ObDereferenceObject((PVOID)KeyBody);
        NtClose(SystemHandle);
        return(NULL);
    }

    Success = CmpFindDrivers(Hive,
                             ControlCell,
                             SystemLoad,
                             NULL,
                             &DriverList);


    if (!Success) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmGetSystemDriverList couldn't find any valid drivers\n"));

        CmpFreeDriverList(Hive, &DriverList);
        CmpUnlockRegistry();
        ObDereferenceObject((PVOID)KeyBody);
        NtClose(SystemHandle);
        return(NULL);
    }

    if (!CmpSortDriverList(Hive,
                           ControlCell,
                           &DriverList)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmGetSystemDriverList couldn't sort driver list\n"));

        CmpFreeDriverList(Hive, &DriverList);
        CmpUnlockRegistry();
        ObDereferenceObject((PVOID)KeyBody);
        NtClose(SystemHandle);
        return(NULL);
    }

    if (!CmpResolveDriverDependencies(&DriverList)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmGetSystemDriverList couldn't resolve driver dependencies\n"));

        CmpFreeDriverList(Hive, &DriverList);
        CmpUnlockRegistry();
        ObDereferenceObject((PVOID)KeyBody);
        NtClose(SystemHandle);
        return(NULL);
    }
    CmpUnlockRegistry();
    ObDereferenceObject((PVOID)KeyBody);
    NtClose(SystemHandle);

    //
    // We now have a fully sorted and ordered list of drivers to be loaded
    // by IoInit.
    //

    //
    // Count the nodes in the list.
    //
    Current = DriverList.Flink;
    DriverCount = 0;
    while (Current != &DriverList) {
        ++DriverCount;
        Current = Current->Flink;
    }

    Handle = (PHANDLE)ExAllocatePool(NonPagedPool,
                                     (DriverCount+1) * sizeof(HANDLE));

    if (Handle == NULL) {
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM_DRIVER_LIST,1,0,0); // odds against this are huge
    }

    //
    // Walk the list, opening each registry key and adding it to the
    // table of handles.
    //
    Current = DriverList.Flink;
    DriverCount = 0;
    while (Current != &DriverList) {
        DriverEntry = CONTAINING_RECORD(Current,
                                        BOOT_DRIVER_LIST_ENTRY,
                                        Link);

        InitializeObjectAttributes(&ObjectAttributes,
                                   &DriverEntry->RegistryPath,
                                   OBJ_CASE_INSENSITIVE,
                                   (HANDLE)NULL,
                                   NULL);

        Status = NtOpenKey(Handle+DriverCount,
                           KEY_READ | KEY_WRITE,
                           &ObjectAttributes);
        if (!NT_SUCCESS(Status)) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmGetSystemDriverList couldn't open driver "));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"key %wZ\n", &DriverEntry->RegistryPath));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"    status %08lx\n",Status));
        } else {
            ++DriverCount;
        }
        Current = Current->Flink;
    }
    Handle[DriverCount] = NULL;

    CmpFreeDriverList(Hive, &DriverList);

    return(Handle);
}


VOID
CmpFreeDriverList(
    IN PHHIVE Hive,
    IN PLIST_ENTRY DriverList
    )

/*++

Routine Description:

    Walks down the driver list, freeing each node in it.

    Note that this calls the hive's free routine pointer to free the memory.

Arguments:

    Hive - Supplies  a pointer to the hive control structure.

    DriverList - Supplies a pointer to the head of the Driver List.  Note
            that the head of the list is not actually freed, only all the
            entries in the list.

Return Value:

    None.

--*/

{
    PLIST_ENTRY         Next;
    PLIST_ENTRY         Current;
    PBOOT_DRIVER_NODE   DriverNode;

    CM_PAGED_CODE();
    Current = DriverList->Flink;
    while (Current != DriverList) {
        Next = Current->Flink;
        DriverNode = (PBOOT_DRIVER_NODE)Current;
        if( DriverNode->Name.Buffer != NULL ){
            (Hive->Free)(DriverNode->Name.Buffer,DriverNode->Name.Length);
        }
        if( DriverNode->ListEntry.RegistryPath.Buffer != NULL ){
            (Hive->Free)(DriverNode->ListEntry.RegistryPath.Buffer,DriverNode->ListEntry.RegistryPath.MaximumLength);
        }
        if( DriverNode->ListEntry.FilePath.Buffer != NULL ){
            (Hive->Free)(DriverNode->ListEntry.FilePath.Buffer,DriverNode->ListEntry.FilePath.MaximumLength);
        }
        (Hive->Free)((PVOID)Current, sizeof(BOOT_DRIVER_NODE));
        Current = Next;
    }
}


NTSTATUS
CmpInitHiveFromFile(
    IN PUNICODE_STRING FileName,
    IN ULONG HiveFlags,
    OUT PCMHIVE *CmHive,
    IN OUT PBOOLEAN Allocate,
    IN  ULONG       CheckFlags
    )

/*++

Routine Description:

    This routine opens a file and log, allocates a CMHIVE, and initializes
    it.

Arguments:

    FileName - Supplies name of file to be loaded.

    HiveFlags - Supplies hive flags to be passed to CmpInitializeHive

    CmHive   - Returns pointer to initialized hive (if successful)

    Allocate - IN: if TRUE ok to allocate, if FALSE hive must exist
                    (but .log may get created)
               OUT: TRUE if actually created hive, FALSE if existed before

Return Value:

    NTSTATUS

--*/

{
    PCMHIVE         NewHive;
    ULONG           Disposition;
    ULONG           SecondaryDisposition;
    HANDLE          PrimaryHandle;
    HANDLE          LogHandle;
    NTSTATUS        Status;
    ULONG           FileType;
    ULONG           Operation;
    PVOID           HiveData = NULL;
    BOOLEAN         NoBuffering = FALSE;

    CM_PAGED_CODE();

RetryNoBuffering:

    *CmHive = NULL;

    Status = CmpOpenHiveFiles(FileName,
                              L".LOG",
                              &PrimaryHandle,
                              &LogHandle,
                              &Disposition,
                              &SecondaryDisposition,
                              *Allocate,
                              FALSE,
                              NoBuffering,
                              NULL);

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    if (LogHandle == NULL) {
        FileType = HFILE_TYPE_PRIMARY;
    } else {
        FileType = HFILE_TYPE_LOG;
    }

    if (Disposition == FILE_CREATED) {
        Operation = HINIT_CREATE;
        *Allocate = TRUE;
    } else {
        if( NoBuffering == TRUE ) {
            Operation = HINIT_FILE;
        } else {
            Operation = HINIT_MAPFILE;
        }
        *Allocate = FALSE;
    }

    if (CmpShareSystemHives) {
        FileType = HFILE_TYPE_PRIMARY;

        if (LogHandle) {
            ZwClose(LogHandle);
            LogHandle = NULL;
        }
    }

    if( HvShutdownComplete == TRUE ) {
        ZwClose(PrimaryHandle);
        if (LogHandle != NULL) {
            ZwClose(LogHandle);
        }
        return STATUS_TOO_LATE;        
    }

    Status = CmpInitializeHive(&NewHive,
                                Operation,
                                HiveFlags,
                                FileType,
                                HiveData,
                                PrimaryHandle,
                                LogHandle,
                                NULL,
                                FileName,
                                CheckFlags
                                );

    if (!NT_SUCCESS(Status)) {
        CmpTrackHiveClose = TRUE;
        ZwClose(PrimaryHandle);
        CmpTrackHiveClose = FALSE;
        if (LogHandle != NULL) {
            ZwClose(LogHandle);
        }

        if( Status == STATUS_RETRY ) {
            if( NoBuffering == FALSE ) {
                NoBuffering = TRUE;
                goto RetryNoBuffering;
            }
        }
        return(Status);
    } else {
        *CmHive = NewHive;

        //
        // mark handles as protected. If other kernel component tries to close them ==> bugcheck.
        //
        CmpSetHandleProtection(PrimaryHandle,TRUE);
        if (LogHandle != NULL) {
            CmpSetHandleProtection(LogHandle,TRUE);
        }
        
        //
        // Capture the file name; in case we need it later for double load check
        //
        (*CmHive)->FileUserName.Buffer = ExAllocatePoolWithTag(PagedPool,
                                                            FileName->Length,
                                                            CM_NAME_TAG | PROTECTED_POOL);

        if ((*CmHive)->FileUserName.Buffer) {

            RtlCopyMemory((*CmHive)->FileUserName.Buffer,
                          FileName->Buffer,
                          FileName->Length);

            (*CmHive)->FileUserName.Length = FileName->Length;
            (*CmHive)->FileUserName.MaximumLength = FileName->Length;

        } 
        if(((PHHIVE)(*CmHive))->BaseBlock->BootType & HBOOT_SELFHEAL) {
            //
            // Warn the user;
            //
            CmpRaiseSelfHealWarning(&((*CmHive)->FileUserName));
        }
        return(STATUS_SUCCESS);
    }
}


NTSTATUS
CmpAddDockingInfo (
    IN HANDLE Key,
    IN PROFILE_PARAMETER_BLOCK * ProfileBlock
    )
/*++

Routine Description:

    Write DockID SerialNumber DockState and Capabilities into the given
    registry key.

--*/
{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING name;
    ULONG value;

    CM_PAGED_CODE ();

    value = ProfileBlock->DockingState;
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_DOCKING_STATE);
    status = NtSetValueKey (Key,
                            &name,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof (value));

    if (!NT_SUCCESS (status)) {
        return status;
    }

    value = ProfileBlock->Capabilities;
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_CAPABILITIES);
    status = NtSetValueKey (Key,
                            &name,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof (value));

    if (!NT_SUCCESS (status)) {
        return status;
    }

    value = ProfileBlock->DockID;
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_DOCKID);
    status = NtSetValueKey (Key,
                            &name,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof (value));

    if (!NT_SUCCESS (status)) {
        return status;
    }

    value = ProfileBlock->SerialNumber;
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_SERIAL_NUMBER);
    status = NtSetValueKey (Key,
                            &name,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof (value));

    if (!NT_SUCCESS (status)) {
        return status;
    }

    return status;
}


NTSTATUS
CmpAddAliasEntry (
    IN HANDLE IDConfigDB,
    IN PROFILE_PARAMETER_BLOCK * ProfileBlock,
    IN ULONG  ProfileNumber
    )
/*++
 *
Routine Description:
    Create an alias entry in the IDConfigDB database for the given
    hardware profile.

    Create the "Alias" key if it does not exist.

Parameters:

    IDConfigDB - Pointer to "..\CurrentControlSet\Control\IDConfigDB"

    ProfileBlock - Description of the current Docking information

    ProfileNumber -

--*/
{
    OBJECT_ATTRIBUTES attributes;
    NTSTATUS        status = STATUS_SUCCESS;
    CHAR            asciiBuffer [128];
    WCHAR           unicodeBuffer [128];
    ANSI_STRING     ansiString;
    UNICODE_STRING  name;
    HANDLE          aliasKey = NULL;
    HANDLE          aliasEntry = NULL;
    ULONG           value;
    ULONG           disposition;
    ULONG           aliasNumber = 0;

    CM_PAGED_CODE ();

    //
    // Find the Alias Key or Create it if it does not already exist.
    //
    RtlInitUnicodeString (&name,CM_HARDWARE_PROFILE_STR_ALIAS);

    InitializeObjectAttributes (&attributes,
                                &name,
                                OBJ_CASE_INSENSITIVE,
                                IDConfigDB,
                                NULL);

    status = NtOpenKey (&aliasKey,
                        KEY_READ | KEY_WRITE,
                        &attributes);

    if (STATUS_OBJECT_NAME_NOT_FOUND == status) {
        status = NtCreateKey (&aliasKey,
                              KEY_READ | KEY_WRITE,
                              &attributes,
                              0, // no title
                              NULL, // no class
                              0, // no options
                              &disposition);
    }

    if (!NT_SUCCESS (status)) {
        aliasKey = NULL;
        goto Exit;
    }

    //
    // Create an entry key
    //

    while (aliasNumber < 200) {
        aliasNumber++;

        sprintf(asciiBuffer, "%04d", aliasNumber);

        RtlInitAnsiString(&ansiString, asciiBuffer);
        name.MaximumLength = sizeof(unicodeBuffer);
        name.Buffer = unicodeBuffer;
        status = RtlAnsiStringToUnicodeString(&name,
                                              &ansiString,
                                              FALSE);
        ASSERT (STATUS_SUCCESS == status);

        InitializeObjectAttributes(&attributes,
                                   &name,
                                   OBJ_CASE_INSENSITIVE,
                                   aliasKey,
                                   NULL);

        status = NtOpenKey (&aliasEntry,
                            KEY_READ | KEY_WRITE,
                            &attributes);

        if (NT_SUCCESS (status)) {
            NtClose (aliasEntry);

        } else if (STATUS_OBJECT_NAME_NOT_FOUND == status) {
            status = STATUS_SUCCESS;
            break;

        } else {
            break;
        }

    }
    if (!NT_SUCCESS (status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: cmpCreateAliasEntry error finding new set %08lx\n",status));

        aliasEntry = 0;
        goto Exit;
    }

    status = NtCreateKey (&aliasEntry,
                          KEY_READ | KEY_WRITE,
                          &attributes,
                          0,
                          NULL,
                          0,
                          &disposition);

    if (!NT_SUCCESS (status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: cmpCreateAliasEntry error creating new set %08lx\n",status));

        aliasEntry = 0;
        goto Exit;
    }

    //
    // Write the standard goo
    //
    CmpAddDockingInfo (aliasEntry, ProfileBlock);

    //
    // Write the Profile Number
    //
    value = ProfileNumber;
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_PROFILE_NUMBER);
    status = NtSetValueKey (aliasEntry,
                            &name,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof (value));

Exit:

    if (aliasKey) {
        NtClose (aliasKey);
    }

    if (aliasEntry) {
        NtClose (aliasEntry);
    }

    return status;
}


NTSTATUS
CmpHwprofileDefaultSelect (
    IN  PCM_HARDWARE_PROFILE_LIST ProfileList,
    OUT PULONG ProfileIndexToUse,
    IN  PVOID Context
    )
{
    UNREFERENCED_PARAMETER (ProfileList);
    UNREFERENCED_PARAMETER (Context);

    * ProfileIndexToUse = 0;

    return STATUS_SUCCESS;
}




NTSTATUS
CmpCreateControlSet(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

/*++

Routine Description:

    This routine sets up the symbolic links from

        \Registry\Machine\System\CurrentControlSet to
        \Registry\Machine\System\ControlSetNNN

        \Registry\Machine\System\CurrentControlSet\Hardware Profiles\Current to
        \Registry\Machine\System\ControlSetNNN\Hardware Profiles\NNNN

    based on the value of \Registry\Machine\System\Select:Current. and
                          \Registry\Machine\System\ControlSetNNN\Control\IDConfigDB:CurrentConfig

Arguments:

    None

Return Value:

    status

--*/

{
    UNICODE_STRING IDConfigDBName;
    UNICODE_STRING SelectName;
    UNICODE_STRING CurrentName;
    OBJECT_ATTRIBUTES Attributes;
    HANDLE SelectHandle;
    HANDLE CurrentHandle;
    HANDLE IDConfigDB = NULL;
    HANDLE CurrentProfile = NULL;
    HANDLE ParentOfProfile = NULL;
    CHAR AsciiBuffer[128];
    WCHAR UnicodeBuffer[128];
    UCHAR ValueBuffer[128];
    ULONG ControlSet;
    ULONG HWProfile;
    PKEY_VALUE_FULL_INFORMATION Value;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    ULONG ResultLength;
    ULONG Disposition;
    BOOLEAN signalAcpiEvent = FALSE;

    CM_PAGED_CODE();

    RtlInitUnicodeString(&SelectName, L"\\Registry\\Machine\\System\\Select");
    InitializeObjectAttributes(&Attributes,
                               &SelectName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtOpenKey(&SelectHandle,
                       KEY_READ,
                       &Attributes);
    if (!NT_SUCCESS(Status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCreateControlSet: Couldn't open Select node %08lx\n",Status));

        return(Status);
    }

    RtlInitUnicodeString(&CurrentName, L"Current");
    Status = NtQueryValueKey(SelectHandle,
                             &CurrentName,
                             KeyValueFullInformation,
                             ValueBuffer,
                             sizeof(ValueBuffer),
                             &ResultLength);
    NtClose(SelectHandle);
    if (!NT_SUCCESS(Status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCreateControlSet: Couldn't query Select value %08lx\n",Status));

        return(Status);
    }
    Value = (PKEY_VALUE_FULL_INFORMATION)ValueBuffer;
    ControlSet = *(PULONG)((PUCHAR)Value + Value->DataOffset);

    RtlInitUnicodeString(&CurrentName, L"\\Registry\\Machine\\System\\CurrentControlSet");
    InitializeObjectAttributes(&Attributes,
                               &CurrentName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtCreateKey(&CurrentHandle,
                         KEY_CREATE_LINK,
                         &Attributes,
                         0,
                         NULL,
                         REG_OPTION_VOLATILE | REG_OPTION_CREATE_LINK,
                         &Disposition);
    if (!NT_SUCCESS(Status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCreateControlSet: couldn't create CurrentControlSet %08lx\n",Status));

        return(Status);
    }

    //
    // Check to make sure that the key was created, not just opened.  Since
    // this key is always created volatile, it should never be present in
    // the hive when we boot.
    //
    ASSERT(Disposition == REG_CREATED_NEW_KEY);

    //
    // Create symbolic link for current hardware profile.
    //
    sprintf(AsciiBuffer, "\\Registry\\Machine\\System\\ControlSet%03d", ControlSet);
    RtlInitAnsiString(&AnsiString, AsciiBuffer);

    CurrentName.MaximumLength = sizeof(UnicodeBuffer);
    CurrentName.Buffer = UnicodeBuffer;
    Status = RtlAnsiStringToUnicodeString(&CurrentName,
                                          &AnsiString,
                                          FALSE);
    Status = NtSetValueKey(CurrentHandle,
                           &CmSymbolicLinkValueName,
                           0,
                           REG_LINK,
                           CurrentName.Buffer,
                           CurrentName.Length);

    if (!NT_SUCCESS(Status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCreateControlSet: couldn't create symbolic link "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"to %wZ\n",&CurrentName));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"    Status=%08lx\n",Status));

        NtClose(CurrentHandle);

        return(Status);
    }

    //
    // Determine the Current Hardware Profile Number
    //
    RtlInitUnicodeString(&IDConfigDBName, L"Control\\IDConfigDB");
    InitializeObjectAttributes(&Attributes,
                               &IDConfigDBName,
                               OBJ_CASE_INSENSITIVE,
                               CurrentHandle,
                               NULL);
    Status = NtOpenKey(&IDConfigDB,
                       KEY_READ,
                       &Attributes);
    NtClose(CurrentHandle);

    if (!NT_SUCCESS(Status)) {
        IDConfigDB = 0;
        goto Cleanup;
    }

    RtlInitUnicodeString(&CurrentName, L"CurrentConfig");
    Status = NtQueryValueKey(IDConfigDB,
                             &CurrentName,
                             KeyValueFullInformation,
                             ValueBuffer,
                             sizeof(ValueBuffer),
                             &ResultLength);

    if (!NT_SUCCESS(Status) ||
        (((PKEY_VALUE_FULL_INFORMATION)ValueBuffer)->Type != REG_DWORD)) {

        goto Cleanup;
    }

    Value = (PKEY_VALUE_FULL_INFORMATION)ValueBuffer;
    HWProfile = *(PULONG)((PUCHAR)Value + Value->DataOffset);
    //
    // We know now the config set that the user selected.
    // namely: HWProfile.
    //

    RtlInitUnicodeString(
              &CurrentName,
              L"\\Registry\\Machine\\System\\CurrentControlSet\\Hardware Profiles");
    InitializeObjectAttributes(&Attributes,
                               &CurrentName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtOpenKey(&ParentOfProfile,
                       KEY_READ,
                       &Attributes);

    if (!NT_SUCCESS (Status)) {
        ParentOfProfile = 0;
        goto Cleanup;
    }

    sprintf(AsciiBuffer, "%04d",HWProfile);
    RtlInitAnsiString(&AnsiString, AsciiBuffer);
    CurrentName.MaximumLength = sizeof(UnicodeBuffer);
    CurrentName.Buffer = UnicodeBuffer;
    Status = RtlAnsiStringToUnicodeString(&CurrentName,
                                          &AnsiString,
                                          FALSE);
    ASSERT (STATUS_SUCCESS == Status);

    InitializeObjectAttributes(&Attributes,
                               &CurrentName,
                               OBJ_CASE_INSENSITIVE,
                               ParentOfProfile,
                               NULL);

    Status = NtOpenKey (&CurrentProfile,
                        KEY_READ | KEY_WRITE,
                        &Attributes);

    if (!NT_SUCCESS (Status)) {
        CurrentProfile = 0;
        goto Cleanup;
    }

    //
    // We need to determine if Value was selected by exact match
    // (TRUE_MATCH) or because the profile selected was aliasable.
    //
    // If aliasable we need to manufacture another alias entry in the
    // alias table.
    //
    // If the profile information is there and not failed then we should
    // mark the Docking state information:
    // (DockID, SerialNumber, DockState, and Capabilities)
    //

    if (NULL != LoaderBlock->Extension) {
        PLOADER_PARAMETER_EXTENSION extension;
        extension = LoaderBlock->Extension;
        switch (extension->Profile.Status) {
        case HW_PROFILE_STATUS_PRISTINE_MATCH:
            //
            // If the selected profile is pristine then we need to clone.
            //
            Status = CmpCloneHwProfile (IDConfigDB,
                                        ParentOfProfile,
                                        CurrentProfile,
                                        HWProfile,
                                        extension->Profile.DockingState,
                                        &CurrentProfile,
                                        &HWProfile);
            if (!NT_SUCCESS (Status)) {
                CurrentProfile = 0;
                goto Cleanup;
            }

            RtlInitUnicodeString(&CurrentName, L"CurrentConfig");
            Status = NtSetValueKey (IDConfigDB,
                                    &CurrentName,
                                    0,
                                    REG_DWORD,
                                    &HWProfile,
                                    sizeof (HWProfile));
            if (!NT_SUCCESS (Status)) {
                goto Cleanup;
            }

            //
            // Fall through
            //
        case HW_PROFILE_STATUS_ALIAS_MATCH:
            //
            // Create the alias entry for this profile.
            //

            Status = CmpAddAliasEntry (IDConfigDB,
                                       &extension->Profile,
                                       HWProfile);

            //
            // Fall through
            //
        case HW_PROFILE_STATUS_TRUE_MATCH:
            //
            // Write DockID, SerialNumber, DockState, and Caps into the current
            // Hardware profile.
            //

            RtlInitUnicodeString (&CurrentName,
                                  CM_HARDWARE_PROFILE_STR_CURRENT_DOCK_INFO);

            InitializeObjectAttributes (&Attributes,
                                        &CurrentName,
                                        OBJ_CASE_INSENSITIVE,
                                        IDConfigDB,
                                        NULL);

            Status = NtCreateKey (&CurrentHandle,
                                  KEY_READ | KEY_WRITE,
                                  &Attributes,
                                  0,
                                  NULL,
                                  REG_OPTION_VOLATILE,
                                  &Disposition);

            ASSERT (STATUS_SUCCESS == Status);

            Status = CmpAddDockingInfo (CurrentHandle, &extension->Profile);

            NtClose(CurrentHandle);

            if (HW_PROFILE_DOCKSTATE_UNDOCKED == extension->Profile.DockingState) {
                signalAcpiEvent = TRUE;
            }

            break;


        case HW_PROFILE_STATUS_SUCCESS:
        case HW_PROFILE_STATUS_FAILURE:
            break;

        default:
            ASSERTMSG ("Invalid Profile status state", FALSE);
        }
    }

    //
    // Create the symbolic link.
    //
    RtlInitUnicodeString(&CurrentName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Hardware Profiles\\Current");
    InitializeObjectAttributes(&Attributes,
                               &CurrentName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtCreateKey(&CurrentHandle,
                         KEY_CREATE_LINK,
                         &Attributes,
                         0,
                         NULL,
                         REG_OPTION_VOLATILE | REG_OPTION_CREATE_LINK,
                         &Disposition);
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCreateControlSet: couldn't create Hardware Profile\\Current %08lx\n",Status));
    } else {
        ASSERT(Disposition == REG_CREATED_NEW_KEY);

        sprintf(AsciiBuffer, "\\Registry\\Machine\\System\\CurrentControlSet\\Hardware Profiles\\%04d",HWProfile);
        RtlInitAnsiString(&AnsiString, AsciiBuffer);
        CurrentName.MaximumLength = sizeof(UnicodeBuffer);
        CurrentName.Buffer = UnicodeBuffer;
        Status = RtlAnsiStringToUnicodeString(&CurrentName,
                                              &AnsiString,
                                              FALSE);
        ASSERT (STATUS_SUCCESS == Status);

        Status = NtSetValueKey(CurrentHandle,
                               &CmSymbolicLinkValueName,
                               0,
                               REG_LINK,
                               CurrentName.Buffer,
                               CurrentName.Length);

        NtClose(CurrentHandle);

        if (!NT_SUCCESS(Status)) {

            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCreateControlSet: couldn't create symbolic link "));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"to %wZ\n",&CurrentName));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"    Status=%08lx\n",Status));

        }
    }

    if (signalAcpiEvent) {
        //
        // We are booting in the undocked state.
        // This is interesting because our buddies in PnP cannot tell
        // us when we are booting without a dock.  They can only tell
        // us when they see a hot undock.
        //
        // Therefore in the interest of matching a boot undocked with
        // a hot undock, we need to simulate an acpi undock event.
        //

        PROFILE_ACPI_DOCKING_STATE newDockState;
        HANDLE profile;
        BOOLEAN changed;

        newDockState.DockingState = HW_PROFILE_DOCKSTATE_UNDOCKED;
        newDockState.SerialLength = 2;
        newDockState.SerialNumber[0] = L'\0';

        Status = CmSetAcpiHwProfile (&newDockState,
                                     CmpHwprofileDefaultSelect,
                                     NULL,
                                     &profile,
                                     &changed);

        ASSERT (NT_SUCCESS (Status));
        NtClose (profile);
    }


Cleanup:
    if (IDConfigDB) {
        NtClose (IDConfigDB);
    }
    if (CurrentProfile) {
        NtClose (CurrentProfile);
    }
    if (ParentOfProfile) {
        NtClose (ParentOfProfile);
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
CmpCloneControlSet(
    VOID
    )

/*++

Routine Description:

    First, create a new hive, \registry\machine\clone, which will be
    HIVE_VOLATILE.

    Second, link \Registry\Machine\System\Clone to it.

    Third, tree copy \Registry\Machine\System\CurrentControlSet into
    \Registry\Machine\System\Clone (and thus into the clone hive.)

    When the service controller is done with the clone hive, it can
    simply NtUnloadKey it to free its storage.

Arguments:

    None.  \Registry\Machine\System\CurrentControlSet must already exist.

Return Value:

    NTSTATUS

--*/

{
    UNICODE_STRING Current;
    UNICODE_STRING Clone;
    HANDLE CurrentHandle;
    HANDLE CloneHandle;
    OBJECT_ATTRIBUTES Attributes;
    NTSTATUS Status;
    PCM_KEY_BODY CurrentKey;
    PCM_KEY_BODY CloneKey;
    ULONG Disposition;
    PSECURITY_DESCRIPTOR Security;
    ULONG SecurityLength;

    CM_PAGED_CODE();

    RtlInitUnicodeString(&Current,
                         L"\\Registry\\Machine\\System\\CurrentControlSet");
    RtlInitUnicodeString(&Clone,
                         L"\\Registry\\Machine\\System\\Clone");

    InitializeObjectAttributes(&Attributes,
                               &Current,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtOpenKey(&CurrentHandle,
                       KEY_READ,
                       &Attributes);
    if (!NT_SUCCESS(Status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneControlSet couldn't open CurrentControlSet %08lx\n",Status));

        return(Status);
    }

    //
    // Get the security descriptor from the key so we can create the clone
    // tree with the correct ACL.
    //
    Status = NtQuerySecurityObject(CurrentHandle,
                                   DACL_SECURITY_INFORMATION,
                                   NULL,
                                   0,
                                   &SecurityLength);
    if (Status==STATUS_BUFFER_TOO_SMALL) {
        Security=ExAllocatePool(PagedPool,SecurityLength);
        if (Security!=NULL) {
            Status = NtQuerySecurityObject(CurrentHandle,
                                           DACL_SECURITY_INFORMATION,
                                           Security,
                                           SecurityLength,
                                           &SecurityLength);
            if (!NT_SUCCESS(Status)) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneControlSet - NtQuerySecurityObject failed %08lx\n",Status));
                ExFreePool(Security);
                Security=NULL;
            }
        }
    } else {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneControlSet - NtQuerySecurityObject returned %08lx\n",Status));
        Security=NULL;
    }

    InitializeObjectAttributes(&Attributes,
                               &Clone,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               Security);
    Status = NtCreateKey(&CloneHandle,
                         KEY_READ | KEY_WRITE,
                         &Attributes,
                         0,
                         NULL,
                         REG_OPTION_VOLATILE,
                         &Disposition);
    if (Security!=NULL) {
        ExFreePool(Security);
    }
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneControlSet couldn't create Clone %08lx\n",Status));
        NtClose(CurrentHandle);
        return(Status);
    }

    //
    // Check to make sure the key was created.  If it already exists,
    // something is wrong.
    //
    if (Disposition != REG_CREATED_NEW_KEY) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneControlSet: Clone tree already exists!\n"));

        //
        // WARNNOTE:
        //      If somebody somehow managed to create a key in our way,
        //      they'll thwart last known good.  Tough luck.  Claim it worked and go on.
        //
        Status = STATUS_SUCCESS;
        goto Exit;
    }

    Status = ObReferenceObjectByHandle(CurrentHandle,
                                       KEY_READ,
                                       CmpKeyObjectType,
                                       KernelMode,
                                       (PVOID *)(&CurrentKey),
                                       NULL);
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneControlSet: couldn't reference CurrentHandle %08lx\n",Status));
        goto Exit;
    }

    Status = ObReferenceObjectByHandle(CloneHandle,
                                       KEY_WRITE,
                                       CmpKeyObjectType,
                                       KernelMode,
                                       (PVOID *)(&CloneKey),
                                       NULL);
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneControlSet: couldn't reference CurrentHandle %08lx\n",Status));
        ObDereferenceObject((PVOID)CurrentKey);
        goto Exit;
    }

    CmpLockRegistryExclusive();

    if (CmpCopyTree(CurrentKey->KeyControlBlock->KeyHive,
                    CurrentKey->KeyControlBlock->KeyCell,
                    CloneKey->KeyControlBlock->KeyHive,
                    CloneKey->KeyControlBlock->KeyCell)) {
        //
        // Set the max subkey name property for the new target key.
        //
        CmpRebuildKcbCache(CloneKey->KeyControlBlock);
        Status = STATUS_SUCCESS;
    } else {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneControlSet: tree copy failed.\n"));
        Status = STATUS_REGISTRY_CORRUPT;
    }

    CmpUnlockRegistry();

    ObDereferenceObject((PVOID)CurrentKey);
    ObDereferenceObject((PVOID)CloneKey);

Exit:
    NtClose(CurrentHandle);
    NtClose(CloneHandle);
    return(Status);

}

NTSTATUS
CmpSaveBootControlSet(USHORT ControlSetNum)
/*++

Routine Description:

   This routine is responsible for saving the control set
   used to accomplish the latest boot into a different control
   set (presumably so that the different control set may be
   marked as the LKG control set).

   This routine is called from NtInitializeRegistry when
   a boot is accepted via that routine.

Arguments:

   ControlSetNum - The number of the control set that will
                   be used to save the boot control set.

Return Value:

   NTSTATUS result code from call, among the following:

      STATUS_SUCCESS - everything worked perfectly
      STATUS_REGISTRY_CORRUPT - could not save the boot control set,
                                it is likely that the copy or sync
                                operation used for this save failed
                                and some part of the boot control
                                set was not saved.
--*/
{
   UNICODE_STRING SavedBoot;
   HANDLE BootHandle, SavedBootHandle;
   OBJECT_ATTRIBUTES Attributes;
   NTSTATUS Status;
   PCM_KEY_BODY BootKey, SavedBootKey;
   ULONG Disposition;
   PSECURITY_DESCRIPTOR Security;
   ULONG SecurityLength;
   BOOLEAN CopyRet;
   WCHAR Buffer[128];

   //
   // Figure out where the boot control set is
   //
   InitializeObjectAttributes(&Attributes,
                              &CmRegistryMachineSystemCurrentControlSet,
                              OBJ_CASE_INSENSITIVE,
                              NULL,
                              NULL);

   //
   // Open the boot control set
   //

   Status = NtOpenKey(&BootHandle,
                      KEY_READ,
                      &Attributes);


   if (!NT_SUCCESS(Status)) return(Status);

   //
   // We may be saving the boot control set into a brand new
   // tree that we will create. If this is true, then we will
   // need to create the root node of this tree below
   // and give it the right security descriptor. So, we fish
   // the security descriptor out of the root node of the
   // boot control set tree.
   //

   Status = NtQuerySecurityObject(BootHandle,
                                  DACL_SECURITY_INFORMATION,
                                  NULL,
                                  0,
                                  &SecurityLength);


   if (Status==STATUS_BUFFER_TOO_SMALL) {

      Security=ExAllocatePool(PagedPool,SecurityLength);

      if (Security!=NULL) {

         Status = NtQuerySecurityObject(BootHandle,
                                        DACL_SECURITY_INFORMATION,
                                        Security,
                                        SecurityLength,
                                        &SecurityLength);


         if (!NT_SUCCESS(Status)) {
            ExFreePool(Security);
            Security=NULL;
         }
      }

   } else {
      Security=NULL;
   }

   //
   // Now, create the path of the control set we will be saving to
   //

   swprintf(Buffer, L"\\Registry\\Machine\\System\\ControlSet%03d", ControlSetNum);

   RtlInitUnicodeString(&SavedBoot,
                        Buffer);

   //
   // Open/Create the control set to which we are saving
   //

   InitializeObjectAttributes(&Attributes,
                              &SavedBoot,
                              OBJ_CASE_INSENSITIVE,
                              NULL,
                              Security);

   Status = NtCreateKey(&SavedBootHandle,
                        KEY_READ | KEY_WRITE,
                        &Attributes,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        &Disposition);


   if (Security) ExFreePool(Security);

   if (!NT_SUCCESS(Status)) {
      NtClose(BootHandle);
      return(Status);
   }

   //
   // Get the key objects for out two controls
   //

   Status = ObReferenceObjectByHandle(BootHandle,
                                      KEY_READ,
                                      CmpKeyObjectType,
                                      KernelMode,
                                      (PVOID *)(&BootKey),
                                      NULL);

   if (!NT_SUCCESS(Status)) goto Exit;

   Status = ObReferenceObjectByHandle(SavedBootHandle,
                                      KEY_WRITE,
                                      CmpKeyObjectType,
                                      KernelMode,
                                      (PVOID *)(&SavedBootKey),
                                      NULL);


   if (!NT_SUCCESS(Status)) {
      ObDereferenceObject((PVOID)BootKey);
      goto Exit;
   }

   //
   // Lock the registry and do the actual saving
   //

   CmpLockRegistryExclusive();

   if (Disposition == REG_CREATED_NEW_KEY) {
       PCM_KEY_NODE Node;

      //
      // If we are saving to a control set that we have just
      // created, it is most efficient to just copy
      // the boot control set tree into the new control set.
      //

      //
      // N.B. We copy the volatile keys only if we are using
      //      a clone and thus our boot control set tree is
      //      composed only of volatile keys.
      //

      CopyRet = CmpCopyTreeEx(BootKey->KeyControlBlock->KeyHive,
                              BootKey->KeyControlBlock->KeyCell,
                              SavedBootKey->KeyControlBlock->KeyHive,
                              SavedBootKey->KeyControlBlock->KeyCell,
                              FALSE);

        //
        // Set the max subkey name property for the new target key.
        //
        Node = (PCM_KEY_NODE)HvGetCell(BootKey->KeyControlBlock->KeyHive,BootKey->KeyControlBlock->KeyCell);
        if( Node ) {
            ULONG       MaxNameLen = Node->MaxNameLen;
            HvReleaseCell(BootKey->KeyControlBlock->KeyHive,BootKey->KeyControlBlock->KeyCell);
            Node = (PCM_KEY_NODE)HvGetCell(SavedBootKey->KeyControlBlock->KeyHive,SavedBootKey->KeyControlBlock->KeyCell);
            if( Node ) {
                if ( HvMarkCellDirty(SavedBootKey->KeyControlBlock->KeyHive,SavedBootKey->KeyControlBlock->KeyCell,FALSE) ) {
                    Node->MaxNameLen = MaxNameLen;
                }
                HvReleaseCell(SavedBootKey->KeyControlBlock->KeyHive,SavedBootKey->KeyControlBlock->KeyCell);
            }
        }

      CmpRebuildKcbCache(SavedBootKey->KeyControlBlock);
   } else {

      //
      // If we are saving to a control set that already exists
      // then its likely that this control set is nearly identical
      // to the boot control set (control sets don't change much
      // between boots).
      //
      // Furthermore, the control set we are saving to must be old
      // and hence has not been modified at all since it ceased
      // being a current control set.
      //
      // Thus, it is most efficient for us to simply synchronize
      // the target control set with the boot control set.
      //

      //
      // N.B. We sync the volatile keys only if we are using
      //      a clone for the same reasons as stated above.
      //

      CopyRet = CmpSyncTrees(BootKey->KeyControlBlock->KeyHive,
                             BootKey->KeyControlBlock->KeyCell,
                             SavedBootKey->KeyControlBlock->KeyHive,
                             SavedBootKey->KeyControlBlock->KeyCell,
                             FALSE);
      CmpRebuildKcbCache(SavedBootKey->KeyControlBlock);
   }

   //
   // Check if the Copy/Sync succeeded and adjust our return code
   // accordingly.
   //

   if (CopyRet) {
      Status = STATUS_SUCCESS;
   } else {
      Status = STATUS_REGISTRY_CORRUPT;
   }

   //
   // All done. Clean up.
   //

   CmpUnlockRegistry();

   ObDereferenceObject((PVOID)BootKey);
   ObDereferenceObject((PVOID)SavedBootKey);

Exit:

   NtClose(BootHandle);
   NtClose(SavedBootHandle);

   return(Status);

}

NTSTATUS
CmpDeleteCloneTree()
/*++

Routine Description:

   Deletes the cloned CurrentControlSet by unloading the CLONE hive.

Arguments:

   NONE.

Return Value:

   NTSTATUS return from NtUnloadKey.

--*/
{
   OBJECT_ATTRIBUTES   Obja;

   InitializeObjectAttributes(
       &Obja,
       &CmRegistrySystemCloneName,
       OBJ_CASE_INSENSITIVE,
       (HANDLE)NULL,
       NULL);

   return NtUnloadKey(&Obja);
}


VOID
CmBootLastKnownGood(
    __in ULONG ErrorLevel
    )

/*++

Routine Description:

    This function is called to indicate a failure during the boot process.
    The actual result is based on the value of ErrorLevel:

        IGNORE - Will return, boot should proceed
        NORMAL - Will return, boot should proceed

        SEVERE - If not booting LastKnownGood, will switch to LastKnownGood
                 and reboot the system.

                 If already booting LastKnownGood, will return.  Boot should
                 proceed.

        CRITICAL - If not booting LastKnownGood, will switch to LastKnownGood
                 and reboot the system.

                 If already booting LastKnownGood, will bugcheck.

Arguments:

    ErrorLevel - Supplies the severity level of the failure

Return Value:

    None.  If it returns, boot should proceed.  May cause the system to
    reboot.

--*/

{
    ARC_STATUS Status;

    CM_PAGED_CODE();

    if (CmFirstTime != TRUE) {

        //
        // NtInitializeRegistry has been called, so handling
        // driver errors is not a task for ScReg.
        // Treat all errors as Normal
        //
        return;
    }

    switch (ErrorLevel) {
        case NormalError:
        case IgnoreError:
            break;

        case SevereError:
            if (CmIsLastKnownGoodBoot()) {
                break;
            } else {
                Status = HalSetEnvironmentVariable("LastKnownGood", "TRUE");
                if (Status == ESUCCESS) {
                    HalReturnToFirmware(HalRebootRoutine);
                }
            }
            break;

        case CriticalError:
            if (CmIsLastKnownGoodBoot()) {
                CM_BUGCHECK( CRITICAL_SERVICE_FAILED, BAD_LAST_KNOWN_GOOD, 1, 0, 0 );
            } else {
                Status = HalSetEnvironmentVariable("LastKnownGood", "TRUE");
                if (Status == ESUCCESS) {
                    HalReturnToFirmware(HalRebootRoutine);
                } else {
                    CM_BUGCHECK( SET_ENV_VAR_FAILED, BAD_LAST_KNOWN_GOOD, 2, 0, 0 );
                }
            }
            break;
    }
    return;
}


BOOLEAN
CmIsLastKnownGoodBoot(
    VOID
    )

/*++

Routine Description:

    Determines whether the current system boot is a LastKnownGood boot or
    not.  It does this by comparing the following two values:

        \registry\machine\system\select:Current
        \registry\machine\system\select:LastKnownGood

    If both of these values refer to the same control set, and this control
    set is different from:

        \registry\machine\system\select:Default

    we are booting LastKnownGood.

Arguments:

    None.

Return Value:

    TRUE  - Booting LastKnownGood
    FALSE - Not booting LastKnownGood

--*/

{
    NTSTATUS Status;
    ULONG Default = 0;//prefast initialization
    ULONG Current = 0;//prefast initialization
    ULONG LKG = 0; //prefast initialization
    RTL_QUERY_REGISTRY_TABLE QueryTable[] = {
        {NULL,      RTL_QUERY_REGISTRY_DIRECT,
         L"Current", &Current,
         REG_DWORD, (PVOID)&Current, 0 },
        {NULL,      RTL_QUERY_REGISTRY_DIRECT,
         L"LastKnownGood", &LKG,
         REG_DWORD, (PVOID)&LKG, 0 },
        {NULL,      RTL_QUERY_REGISTRY_DIRECT,
         L"Default", &Default,
         REG_DWORD, (PVOID)&Default, 0 },
        {NULL,      0,
         NULL, NULL,
         REG_NONE, NULL, 0 }
    };

    CM_PAGED_CODE();

    Status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                    L"\\Registry\\Machine\\System\\Select",
                                    QueryTable,
                                    NULL,
                                    NULL);
    //
    // If this failed, something is severely wrong.
    //

    ASSERT(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"CmIsLastKnownGoodBoot: RtlQueryRegistryValues "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"failed, Status %08lx\n", Status));
        return(FALSE);
    }

    if ((LKG == Current) && (Current != Default)){
        return(TRUE);
    } else {
        return(FALSE);
    }
}

BOOLEAN
CmpLinkKeyToHive(
    PWSTR   KeyPath,
    PWSTR   HivePath
    )

/*++

Routine Description:

    Creates a symbolic link at KeyPath that points to HivePath.

Arguments:

    KeyPath - pointer to unicode string with name of key
              (e.g. L"\\Registry\\Machine\\Security\\SAM")

    HivePath - pointer to unicode string with name of hive root
               (e.g. L"\\Registry\\Machine\\SAM\\SAM")

Return Value:

    TRUE if links were successfully created, FALSE otherwise

--*/

{
    UNICODE_STRING KeyName;
    UNICODE_STRING LinkName;
    OBJECT_ATTRIBUTES Attributes;
    HANDLE LinkHandle;
    ULONG Disposition;
    NTSTATUS Status;

    CM_PAGED_CODE();

    //
    // Create link for CLONE hive
    //

    RtlInitUnicodeString(&KeyName, KeyPath);
    InitializeObjectAttributes(&Attributes,
                               &KeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtCreateKey(&LinkHandle,
                         KEY_CREATE_LINK,
                         &Attributes,
                         0,
                         NULL,
                         REG_OPTION_VOLATILE | REG_OPTION_CREATE_LINK,
                         &Disposition);
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpLinkKeyToHive: couldn't create %S\n", &KeyName));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"    Status = %08lx\n",Status));
        return(FALSE);
    }

    //
    // Check to make sure that the key was created, not just opened.  Since
    // this key is always created volatile, it should never be present in
    // the hive when we boot.
    //
    if (Disposition != REG_CREATED_NEW_KEY) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpLinkKeyToHive: %S already exists!\n", &KeyName));
        NtClose(LinkHandle);
        return(FALSE);
    }

    RtlInitUnicodeString(&LinkName, HivePath);
    Status = NtSetValueKey(LinkHandle,
                           &CmSymbolicLinkValueName,
                           0,
                           REG_LINK,
                           LinkName.Buffer,
                           LinkName.Length);
    NtClose(LinkHandle);
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpLinkKeyToHive: couldn't create symbolic link for %S\n", HivePath));
        return(FALSE);
    }

    return(TRUE);
}

VOID
CmpCreatePerfKeys(
    VOID
    )

/*++

Routine Description:

    Creates predefined keys for the performance text to support old apps on 1.0a

Arguments:

    None.

Return Value:

    None.

--*/

{
    HANDLE Perflib;
    NTSTATUS Status;
    WCHAR LanguageId[4];
    OBJECT_ATTRIBUTES Attributes;
    UNICODE_STRING String;
    USHORT Language;
    LONG i;
    WCHAR c;
    extern PWCHAR CmpRegistryPerflibString;

    RtlInitUnicodeString(&String, CmpRegistryPerflibString);

    InitializeObjectAttributes(&Attributes,
                               &String,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtOpenKey(&Perflib,
                       KEY_WRITE,
                       &Attributes);
    if (!NT_SUCCESS(Status)) {
        return;
    }


    //
    // Always create the predefined keys for the english language
    //
    CmpCreatePredefined(Perflib,
                        L"009",
                        HKEY_PERFORMANCE_TEXT);

    //
    // If the default language is not english, create a predefined key for
    // that, too.
    //
    if (PsDefaultSystemLocaleId != 0x00000409) {
        Language = LANGIDFROMLCID(PsDefaultUILanguageId) & 0xff;
        LanguageId[3] = L'\0';
        for (i=2;i>=0;i--) {
            c = Language % 16;
            if (c>9) {
                LanguageId[i]= c+L'A'-10;
            } else {
                LanguageId[i]= c+L'0';
            }
            Language = Language >> 4;
        }
        CmpCreatePredefined(Perflib,
                            LanguageId,
                            HKEY_PERFORMANCE_NLSTEXT);
    }
}


VOID
CmpCreatePredefined(
    IN HANDLE Root,
    IN PWSTR KeyName,
    IN HANDLE PredefinedHandle
    )

/*++

Routine Description:

    Creates a special key that will always return the given predefined handle
    instead of a real handle.

Arguments:

    Root - supplies the handle the keyname is relative to

    KeyName - supplies the name of the key.

    PredefinedHandle - supplies the predefined handle to be returned when this
        key is opened.

Return Value:

    None.

--*/

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    CM_PARSE_CONTEXT ParseContext;
    NTSTATUS Status;
    UNICODE_STRING Name;
    HANDLE Handle;

    ParseContext.Class.Length = 0;
    ParseContext.Class.Buffer = NULL;

    ParseContext.TitleIndex = 0;
    ParseContext.CreateOptions = REG_OPTION_VOLATILE | REG_OPTION_PREDEF_HANDLE;
    ParseContext.Disposition = 0;
    ParseContext.CreateLink = FALSE;
    ParseContext.PredefinedHandle = PredefinedHandle;
    ParseContext.CreateOperation = TRUE;
    ParseContext.OriginatingPoint = NULL;

    RtlInitUnicodeString(&Name, KeyName);
    InitializeObjectAttributes(&ObjectAttributes,
                               &Name,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               Root,
                               NULL);

    Status = ObOpenObjectByName(&ObjectAttributes,
                                CmpKeyObjectType,
                                KernelMode,
                                NULL,
                                KEY_READ,
                                (PVOID)&ParseContext,
                                &Handle);
                                
    ASSERT(CmpMiniNTBoot || NT_SUCCESS(Status));

    if (NT_SUCCESS(Status))
        ZwClose(Handle);
}

BOOLEAN   CmpSystemHiveConversionFailed = FALSE;

NTSTATUS
CmpSetupPrivateWrite(
    PCMHIVE             CmHive
    )
/*++

Routine Description:

	Converts the primary file to private write stream

Arguments:

    CmHive - hive to convert, typically SYSTEM

Return Value:

    NONE; bugchecks if something wrong

--*/
{   
    ULONG       FileOffset;
    ULONG       Data;
	NTSTATUS	Status;

	CM_PAGED_CODE()

    //
    //  We need to issue a read from the file, to trigger the cache initialization
    //
    FileOffset = 0;
    if ( ! (((PHHIVE)CmHive)->FileRead)(
                    (PHHIVE)CmHive,
                    HFILE_TYPE_PRIMARY,
                    &FileOffset,
                    (PVOID)&Data,
                    sizeof(ULONG)
                    )
       )
    {
        return STATUS_REGISTRY_IO_FAILED;
    }

    //
    // Acquire the file object for the primary; This should be called AFTER the
    // cache has been initialized.
    //
    Status = CmpAcquireFileObjectForFile(CmHive,CmHive->FileHandles[HFILE_TYPE_PRIMARY],&(CmHive->FileObject));
    if( !NT_SUCCESS(Status) ) {
		return Status;
    }

    //
    // set the getCell and releaseCell routines to the right one(s)
    //
    CmHive->Hive.GetCellRoutine = HvpGetCellMapped;
    CmHive->Hive.ReleaseCellRoutine = HvpReleaseCellMapped;

	return STATUS_SUCCESS;
}

//
// This thread is used to load the machine hives in parallel 
//
extern  ULONG   CmpCheckHiveIndex;

VOID
CmpLoadHiveThread(
    IN PVOID StartContext
    )
/*++

Routine Description:
    
    Loads the hive at index StartContext in CmpMachineHiveList

    Warning. We need to protect when enlisting the hives in CmpHiveListHead !!!

Arguments:

Return Value:

--*/
{
    UCHAR   FileBuffer[MAX_NAME];
    UCHAR   RegBuffer[MAX_NAME];

    UNICODE_STRING TempName;
    UNICODE_STRING FileName;
    UNICODE_STRING RegName;

    USHORT  FileStart;
    USHORT  RegStart;
    ULONG   i;
    PCMHIVE CmHive;
    HANDLE  PrimaryHandle;
    HANDLE  LogHandle;
    ULONG   PrimaryDisposition;
    ULONG   SecondaryDisposition;
    ULONG   Length;
    NTSTATUS Status = STATUS_SUCCESS;

    PVOID   ErrorParameters;
    ULONG   ErrorResponse;
    ULONG   ClusterSize;
    ULONG   LocalWorkerIncrement;

    CM_PAGED_CODE();

    i = (ULONG)(ULONG_PTR)StartContext;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"CmpLoadHiveThread %i ... starting\n",i));

    ASSERT( CmpMachineHiveList[i].Name != NULL );

    if( i == CmpCheckHiveIndex ) {
        //
        // we want to hold this thread until all the others finish, so we have a chance to debug it.
        // last one that finishes will wake us
        //
        KeWaitForSingleObject( &CmpLoadWorkerDebugEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );
        ASSERT( CmpLoadWorkerIncrement == (CM_NUMBER_OF_MACHINE_HIVES - 1) );
        DbgBreakPoint();
    }
    //
    // signal that we have started
    //
    CmpMachineHiveList[i].ThreadStarted = TRUE;

    FileName.MaximumLength = MAX_NAME;
    FileName.Length = 0;
    FileName.Buffer = (PWSTR)&(FileBuffer[0]);

    RegName.MaximumLength = MAX_NAME;
    RegName.Length = 0;
    RegName.Buffer = (PWSTR)&(RegBuffer[0]);

    RtlInitUnicodeString(
        &TempName,
        INIT_SYSTEMROOT_HIVEPATH
        );
    RtlAppendStringToString((PSTRING)&FileName, (PSTRING)&TempName);
    FileStart = FileName.Length;

    RtlInitUnicodeString(
        &TempName,
        INIT_REGISTRY_MASTERPATH
        );
    RtlAppendStringToString((PSTRING)&RegName, (PSTRING)&TempName);
    RegStart = RegName.Length;

    //
    // Compute the name of the file, and the name to link to in
    // the registry.
    //

    // REGISTRY

    RegName.Length = RegStart;
    RtlInitUnicodeString(
        &TempName,
        CmpMachineHiveList[i].BaseName
        );
    RtlAppendStringToString((PSTRING)&RegName, (PSTRING)&TempName);

    // REGISTRY\MACHINE or REGISTRY\USER

    if (RegName.Buffer[ (RegName.Length / sizeof( WCHAR )) - 1 ] == '\\') {
        RtlInitUnicodeString(
            &TempName,
            CmpMachineHiveList[i].Name
            );
        RtlAppendStringToString((PSTRING)&RegName, (PSTRING)&TempName);
    }

    // REGISTRY\[MACHINE|USER]\HIVE

    // <sysroot>\config

    RtlInitUnicodeString(
        &TempName,
        CmpMachineHiveList[i].Name
        );
    FileName.Length = FileStart;
    RtlAppendStringToString((PSTRING)&FileName, (PSTRING)&TempName);

    // <sysroot>\config\hive


    if (CmpMachineHiveList[i].CmHive == NULL) {

        //
        // Hive has not been initialized in any way.
        //

        CmpMachineHiveList[i].Allocate = TRUE;
        Status = CmpInitHiveFromFile(&FileName,
                                     CmpMachineHiveList[i].HHiveFlags,
                                     &CmHive,
                                     &(CmpMachineHiveList[i].Allocate),
                                     CM_CHECK_REGISTRY_CHECK_CLEAN
                                     );

        if ( (!NT_SUCCESS(Status)) ||
             (!CmpShareSystemHives && (CmHive->FileHandles[HFILE_TYPE_LOG] == NULL)) )
        {
            ErrorParameters = &FileName;
            ExRaiseHardError(
                STATUS_CANNOT_LOAD_REGISTRY_FILE,
                1,
                1,
                (PULONG_PTR)&ErrorParameters,
                OptionOk,
                &ErrorResponse
                );

        }

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"CmpInitializeHiveList:\n"));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"\tCmHive for '%ws' @", CmpMachineHiveList[i]));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"%08lx", CmHive));

        CmHive->Flags = CmpMachineHiveList[i].CmHiveFlags;
        CmpMachineHiveList[i].CmHive2 = CmHive;
        
    } else {

        CmHive = CmpMachineHiveList[i].CmHive;

        if (!(CmHive->Hive.HiveFlags & HIVE_VOLATILE)) {

            //
            // CmHive already exists.  It is not an entirely volatile
            // hive (we do nothing for those.)
            //
            // First, open the files (Primary and Alternate) that
            // back the hive.  Stuff their handles into the CmHive
            // object.  Force the size of the files to match the
            // in memory images.  Call HvSyncHive to write changes
            // out to disk.
            //
			BOOLEAN	NoBufering = FALSE; // first try to open it cached;

retryNoBufering:

            Status = CmpOpenHiveFiles(&FileName,
                                      L".LOG",
                                      &PrimaryHandle,
                                      &LogHandle,
                                      &PrimaryDisposition,
                                      &SecondaryDisposition,
                                      TRUE,
                                      TRUE,
                                      NoBufering,
                                      &ClusterSize);

            if ( ( ! NT_SUCCESS(Status)) ||
                 (LogHandle == NULL) )
            {
fatal:
                ErrorParameters = &FileName;
                ExRaiseHardError(
                    STATUS_CANNOT_LOAD_REGISTRY_FILE,
                    1,
                    1,
                    (PULONG_PTR)&ErrorParameters,
                    OptionOk,
                    &ErrorResponse
                    );

                //
                // WARNNOTE
                // We've just told the user that something essential,
                // like the SYSTEM hive, is hosed.  Don't try to run,
                // we just risk destroying user data.  Punt.
                //
                CM_BUGCHECK(BAD_SYSTEM_CONFIG_INFO,BAD_HIVE_LIST,0,i,Status);
            }

            CmHive->FileHandles[HFILE_TYPE_LOG] = LogHandle;
            CmHive->FileHandles[HFILE_TYPE_PRIMARY] = PrimaryHandle;

			if( NoBufering == FALSE ) {
				//
				// initialize cache and mark the stream as PRIVATE_WRITE;
				// next flush will do the actual conversion
				//
				Status = CmpSetupPrivateWrite(CmHive);
			}

			if( !NT_SUCCESS(Status) ) {
				if( (NoBufering == TRUE) || (Status != STATUS_RETRY) ) {
					//
					// we have tried both ways and it didn't work; bad luck
					//
					goto fatal;
				}

                DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Failed to convert SYSTEM hive to mapped (0x%lx) ... loading it in paged pool\n",Status);

				//
				// close handle and make another attempt to open them without buffering
				//
				CmpTrackHiveClose = TRUE;
				ZwClose(PrimaryHandle);
				CmpTrackHiveClose = FALSE;
				ZwClose(LogHandle);
				NoBufering = TRUE;

				goto retryNoBufering;
			}

            //
            // now that we successfully opened the hive files, clear off the lazy flush flag
            //
            ASSERT( CmHive->Hive.HiveFlags & HIVE_NOLAZYFLUSH );
            CmHive->Hive.HiveFlags &= (~HIVE_NOLAZYFLUSH);

            Length = CmHive->Hive.Storage[Stable].Length + HBLOCK_SIZE;

            //
            // When an in-memory hive is opened with no backing
            // file, ClusterSize is assumed to be 1.  When the file
            // is opened later (for the SYSTEM hive) we need
            // to update this field in the hive if we are
            // booting from media where the cluster size > 1
            //
            if (CmHive->Hive.Cluster != ClusterSize) {
                //
                // The cluster size is different than previous assumed.
                // Since a cluster in the dirty vector must be either
                // completely dirty or completely clean, go through the
                // dirty vector and mark all clusters that contain a dirty
                // logical sector as completely dirty.
                //
                PRTL_BITMAP  BitMap;
                ULONG        Index;

                BitMap = &(CmHive->Hive.DirtyVector);
                for (Index = 0;
                     Index < CmHive->Hive.DirtyVector.SizeOfBitMap;
                     Index += ClusterSize)
                {
                    if (!RtlAreBitsClear (BitMap, Index, ClusterSize)) {
                        RtlSetBits (BitMap, Index, ClusterSize);
                    }
                }
                //
                // Update DirtyCount and Cluster
                //
                CmHive->Hive.DirtyCount = RtlNumberOfSetBits(&CmHive->Hive.DirtyVector);
                CmHive->Hive.Cluster = ClusterSize;
            }

            if (!CmpFileSetSize(
                    (PHHIVE)CmHive, HFILE_TYPE_PRIMARY, Length,Length) 
               )
            {
                //
                // WARNNOTE
                // Data written into the system hive since boot
                // cannot be written out, punt.
                //
                CmpCannotWriteConfiguration = TRUE;
            }

            ASSERT(FIELD_OFFSET(CMHIVE, Hive) == 0);

            if( CmHive->Hive.BaseBlock->BootRecover != 0 ) {
                //
                // boot loader recovered the hive; we need to flush it all to the disk
                // mark everything dirty; the next flush will do take care of the rest
                //
                PRTL_BITMAP  BitMap;
                BitMap = &(CmHive->Hive.DirtyVector);
                RtlSetAllBits(BitMap);
                CmHive->Hive.DirtyCount = BitMap->SizeOfBitMap;
                //
                // we only need to flush the hive when the loader has recovered it
                //
                HvSyncHive((PHHIVE)CmHive);
                
            }

            CmpMachineHiveList[i].CmHive2 = CmHive;

            ASSERT( CmpMachineHiveList[i].CmHive == CmpMachineHiveList[i].CmHive2 );

            if( CmpCannotWriteConfiguration ) {
                //
                // The system disk is full; Give user a chance to log-on and make room
                //
                CmpDiskFullWarning();
            } 

            //
            // copy the full file name for the conversion worker thread 
            //
            SystemHiveFullPathName.MaximumLength = MAX_NAME;
            SystemHiveFullPathName.Length = 0;
            SystemHiveFullPathName.Buffer = (PWSTR)&(SystemHiveFullPathBuffer[0]);
            RtlAppendStringToString((PSTRING)&SystemHiveFullPathName, (PSTRING)&FileName);
        } else if (CmpMiniNTBoot) {
            //
            // copy the full file name for the conversion worker thread 
            //
            SystemHiveFullPathName.MaximumLength = MAX_NAME;
            SystemHiveFullPathName.Length = 0;
            SystemHiveFullPathName.Buffer = (PWSTR)&(SystemHiveFullPathBuffer[0]);
            RtlAppendStringToString((PSTRING)&SystemHiveFullPathName, (PSTRING)&FileName);
        }                
        if(i == SYSTEM_HIVE_INDEX) {
            //
            // marks the System\Select!Current value dirty so we preserve what was set by the loader.
            //
            CmpMarkCurrentValueDirty((PHHIVE)CmHive,CmHive->Hive.BaseBlock->RootCell);
        }
    }

    CmpMachineHiveList[i].ThreadFinished = TRUE;

    LocalWorkerIncrement = InterlockedIncrement (&CmpLoadWorkerIncrement);
    if ( LocalWorkerIncrement == CM_NUMBER_OF_MACHINE_HIVES ) {
        //
        // this was the last thread (the lazyest); signal the main thread
        //
        KeSetEvent (&CmpLoadWorkerEvent, 0, FALSE);
    }

    if ( (LocalWorkerIncrement == (CM_NUMBER_OF_MACHINE_HIVES -1)) && // there is one more thread
         (CmpCheckHiveIndex < CM_NUMBER_OF_MACHINE_HIVES ) // which is waiting to be debugged
        ) {
        //
        // wake up the thread to be debugged
        //
        KeSetEvent (&CmpLoadWorkerDebugEvent, 0, FALSE);
    }

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"CmpLoadHiveThread %i ... terminating\n",i));
    PsTerminateSystemThread(Status);
}


NTSTATUS
CmpSetNetworkValue(
    IN PNETWORK_LOADER_BLOCK NetworkLoaderBlock
    )
/*++

Routine Description:

    This function will save the information in the Network Loader
    Block to the registry.

Arguments:
    NetworkLoaderBlock - Supplies a pointer to the network loader block 
                         that was created by the OS Loader.

Return Value:

    NTSTATUS code.

--*/
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING string;
    HANDLE handle;
    ULONG disposition;


    ASSERT( NetworkLoaderBlock != NULL );
    ASSERT( NetworkLoaderBlock->DHCPServerACKLength > 0 );


    RtlInitUnicodeString( &string, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\PXE" );

    InitializeObjectAttributes(
        &objectAttributes,
        &string,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtCreateKey(&handle,
                         KEY_ALL_ACCESS,
                         &objectAttributes,
                         0,
                         (PUNICODE_STRING)NULL,
                         0,
                         &disposition
                         );
    if ( !NT_SUCCESS(status) ) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL, "CmpSetNetworkValue: Unable to open PXE key: %x\n", status ));
        goto Error;
    }

    RtlInitUnicodeString( &string, L"DHCPServerACK" );

    status = NtSetValueKey(handle,
                           &string,
                           0,
                           REG_BINARY,
                           NetworkLoaderBlock->DHCPServerACK,
                           NetworkLoaderBlock->DHCPServerACKLength
                           );
    if ( !NT_SUCCESS(status) ) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL, "CmpSetNetworkValue: Unable to set DHCPServerACK key: %x\n", status ));
        goto Error;
    }

    RtlInitUnicodeString( &string, L"BootServerReply" );

    status = NtSetValueKey(handle,
                           &string,
                           0,
                           REG_BINARY,
                           NetworkLoaderBlock->BootServerReplyPacket,
                           NetworkLoaderBlock->BootServerReplyPacketLength                           
                           );
    if ( !NT_SUCCESS(status) ) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL, "CmpSetNetworkValue: Unable to set BootServerReplyPacket key: %x\n", status ));
        goto Error;
    }

    status = STATUS_SUCCESS;

Cleanup:
    NtClose( handle );
    
    return status;

Error:
    goto Cleanup;
}



NTSTATUS
CmpSetSystemValues(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
/*++

Routine Description:

    This function will save the system start information to 
    the registry.

Arguments:
    LoaderBlock -  Supplies a pointer to the loader block.

Return Value:

    NTSTATUS code.

--*/
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING string;
    UNICODE_STRING value;
    HANDLE handle;


    ASSERT( LoaderBlock != NULL );


    value.Buffer = NULL;

    //
    // Open the control key
    //

    RtlInitUnicodeString( &string, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control" );

    InitializeObjectAttributes(
        &objectAttributes,
        &string,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenKey(
                       &handle,
                       KEY_ALL_ACCESS,
                       &objectAttributes
                      );
    if ( !NT_SUCCESS(status) ) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpSetSystemValues: Unable to Open Control Key: %x\n", status ));
        goto Error;
    }

    //
    // Set the System start options key
    //

    RtlInitUnicodeString( &string, L"SystemStartOptions" );

    status = NtSetValueKey  (
                            handle,
                            &string,
                            0,
                            REG_SZ,
                            CmpLoadOptions.Buffer,
                            CmpLoadOptions.Length
                            );
    if ( !NT_SUCCESS(status) ) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpSetSystemValue: Unable to set SystemStartOptions key: %x\n", status ));
        goto Error;
    }

    //
    // Set the System Boot Device
    //

    RtlInitUnicodeString( &string, L"SystemBootDevice" );
    RtlCreateUnicodeStringFromAsciiz( &value, LoaderBlock->ArcBootDeviceName );

    status = NtSetValueKey(handle,
                           &string,
                           0,
                           REG_SZ,
                           value.Buffer,
                           value.Length + sizeof(WCHAR)
                           );
    if ( !NT_SUCCESS(status) ) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpSetSystemValue: Unable to set SystemBootDevice key: %x\n", status ));
        goto Error;
    }

    status = STATUS_SUCCESS;

Cleanup:
    if ( value.Buffer ) {
        RtlFreeUnicodeString(&value);
    }

    NtClose( handle );
    
    return status;

Error:
    goto Cleanup;
}

VOID
CmpMarkCurrentValueDirty(
                         IN PHHIVE SystemHive,
                         IN HCELL_INDEX RootCell
                         )
{
    PCM_KEY_NODE    Node;
    HCELL_INDEX     Select;
    UNICODE_STRING  Name;
    HCELL_INDEX     ValueCell;

    CM_PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    //
    // Find \SYSTEM\SELECT node.
    //
    Node = (PCM_KEY_NODE)HvGetCell(SystemHive,RootCell);
    if( Node == NULL ) {
        //
        // we couldn't map a view for the bin containing this cell
        //

        return;
    }
    RtlInitUnicodeString(&Name, L"select");
    Select = CmpFindSubKeyByName(SystemHive,
                                Node,
                                &Name);
    HvReleaseCell(SystemHive,RootCell);
    if (Select == HCELL_NIL) {
        return;
    }
    Node = (PCM_KEY_NODE)HvGetCell(SystemHive,Select);
    if( Node == NULL ) {
        //
        // we couldn't map a view for the bin containing this cell
        //

        return;
    }

    RtlInitUnicodeString(&Name, L"Current");
    ValueCell = CmpFindValueByName(SystemHive,
                                   Node,
                                   &Name);
    HvReleaseCell(SystemHive,Select);
    if (ValueCell != HCELL_NIL) {
        HvMarkCellDirty(SystemHive, ValueCell,FALSE);
    }

}
