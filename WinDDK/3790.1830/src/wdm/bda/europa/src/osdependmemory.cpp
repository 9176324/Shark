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
//  Allocates memory dependant to specified pool type. Pool type should
//  always be NON_PAGED_POOL.
//  Callers must be running at IRQL <= DISPATCH_LEVEL with one exception, if
//  paged pool is selected (not recommended) the caller must be running
//  below DISPATCH_LEVEL.
//  We wrote several helper functions in C to have global access from the HAL.
//  A better architecture approach would be to have these helper functions
//  in a "OSDepend" base class.
//
// Return Value:
//  NULL                Operation failed,
//                      (a) if there is insufficient memory for the allocation
//                      request.
//                      (b) if the current IRQ level is equal
//                      (iType == PAGED_POOL) or greater than DISPATCH_LEVEL,
//  A valid pointer     Pointer to a memory area.
//
//////////////////////////////////////////////////////////////////////////////
void * __cdecl operator new
(
    size_t nSize,     //Size of memory to allocate.
    eVampPoolType iType     //Paged or non paged memory block.
)
{
    if( nSize == 0 )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Demand memory Size is zero"));
        return NULL;
    }

    // check current IRQ level
    if( KeGetCurrentIrql() > DISPATCH_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level too high"));
        return NULL;
    }
    // check if parameters are fitting to current IRQ level
    if( (iType != NON_PAGED_POOL) && (KeGetCurrentIrql() == DISPATCH_LEVEL) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Paged pool allocations not possible at this IRQ level"));
        return NULL;
    }
    PVOID pResult = NULL;
    // allocate memory depending on selected pool type
    switch( iType )
    {
    case NON_PAGED_POOL:
        pResult = ExAllocatePoolWithTag(NonPagedPool, nSize, 'pmaV');
        break;
    case PAGED_POOL:
        _DbgPrintF(DEBUGLVL_VERBOSE,
            ("Warning: Paged pool allocation (not recommended)"));
        pResult = ExAllocatePoolWithTag(PagedPool, nSize, 'pmaV');
        break;
    case MAX_POOL_TYPE:
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Wrong pool type"));
        break;
    default:
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Unsupported pool type"));
        break;
    }
    // check if allocation was successful
    if( !pResult )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Not enough memory"));
        return NULL;
    }
    // initialize allocated memory with 0
    // this function does not support a return value, so we
    // assume that it never fails
    memset(pResult, 0, nSize);
    // return pointer to allocated memory
    return pResult;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Similar to the new operator a delete operator must be implemented
//  to support global classes. The delete method refers to a standard
//  free(). Callers must be running at IRQL <= DISPATCH_LEVEL.
//  We wrote several helper functions in C to have global access from the HAL.
//  A better architecture approach would be to have these helper functions
//  in a "OSDepend" base class.
//
// Return Value:
//  none.
//
//////////////////////////////////////////////////////////////////////////////
void __cdecl operator delete
(
    PVOID pMem  //Pointer to the object to de-allocate
)
{
    // check if parameter is valid
    if( !pMem )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return;
    }
    // check current IRQ level
    if( KeGetCurrentIrql() > DISPATCH_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level too high"));
        return;
    }
    // de-allocate memory
    // this function does not support a return value, so we
    // assume that it never fails
    ExFreePool(pMem);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
// _purecall stub required for virtual function usage on 98 Gold. 
//
// Return Value:
//  0
//
//////////////////////////////////////////////////////////////////////////////
int __cdecl 
_purecall (
    VOID
    ) 

{
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor
//
//////////////////////////////////////////////////////////////////////////////
COSDependMemory::COSDependMemory()
{
    m_cBytes = 0;
    m_pulVirtualPTAddress = NULL;
    m_pMDL = NULL;
    // object status is not needed here, but it was defined in the OSDepend
    // interface, so we have to set it correctly
    m_dwObjectStatus = STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor. Callers must be running at IRQL <= DISPATCH_LEVEL.
//
//////////////////////////////////////////////////////////////////////////////
COSDependMemory::~COSDependMemory()
{
    m_dwObjectStatus = CONSTRUCTOR_FAILED;
    if( m_pulVirtualPTAddress )
    {
        delete m_pulVirtualPTAddress;
        m_pulVirtualPTAddress = NULL;
    }
    m_cBytes = 0;
    if( m_pMDL )
    {
        if( KeGetCurrentIrql() > DISPATCH_LEVEL )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level not correct"));
            return;
        }
        // de-allocate MDL
        // this function does not support a return value, so we
        // assume that it never fails
        IoFreeMdl( static_cast <PMDL> (m_pMDL) );
        m_pMDL = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Returns the current status of the object
//
// Return Value:
//  STATUS_OK           Constructor was called successfully
//  CONSTRUCTOR_FAILED  The call to the constructor was unsuccessful
//
//////////////////////////////////////////////////////////////////////////////
DWORD COSDependMemory::GetObjectStatus()
{
    return m_dwObjectStatus;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Returns a system virtual address for the buffer that can
//  be used to read and  write the buffer memory from DPC or interrupt level
//  code. Callers must be running at IRQL <= DISPATCH_LEVEL.
//
// Return Value:
//  Returns the base system-space virtual address that maps the physical
//  pages described by the given MDL.
//
//////////////////////////////////////////////////////////////////////////////
PVOID COSDependMemory::GetSystemAddress
(
    PVOID pHandle    //unused
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: GetSystemAddress called"));
    // for WDM implementation, MDL is stored in the COSDependMemory object
    // and the handle given to the user is irrelevant, so check if MDL
    // member is initialized correctly instead of checking the handle
    if( !m_pMDL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Important member not initialized"));
        return NULL;
    }
    // check current IRQ level
    if( KeGetCurrentIrql() > DISPATCH_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level not correct"));
        return NULL;
    }
    // we still support 98SE, so we cannot use MmGetSystemAddressForMdlSafe as
    // recommended in the DDK
    PVOID pSystemAddress =
        MmGetSystemAddressForMdl( static_cast <PMDL> (m_pMDL) );
    if( !pSystemAddress )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Get system address failed"));
        return NULL;
    }
    return pSystemAddress;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Locks the physical pages mapped by the virtual address range in memory.
//  The memory description is updated to describe the physical pages.
//  Callers must be running at IRQL <= DISPATCH_LEVEL.
//
// Return Value:
//  NULL                Operation failed,
//                      (a) if the function is called again after successful
//                      locking without an previous unlock
//                      (b) if the memory cannot be allocated,
//  A valid pointer     The mapped virtual address of the locked physical page
//
//////////////////////////////////////////////////////////////////////////////
PVOID COSDependMemory::LockPages
(
    PVOID pVirtualAddress, //Points to the base virtual address of the buffer.
    size_t ulNumberOfBytes //Specifies the length in bytes of the buffer.
)
{
    // check if parameter is valid
    if( !pVirtualAddress )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return NULL;
    }
    // check if parameters are within valid range
    if( (ulNumberOfBytes < 1) || (ulNumberOfBytes > 2097152) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return NULL;
    }
    // check current IRQ level
    if( KeGetCurrentIrql() > DISPATCH_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level not correct"));
        return NULL;
    }
    // check if member was initialized before
    if( m_pMDL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: LockPages called twice"));
        return NULL;
    }
    // allocate MDL for given memory
    m_pMDL = static_cast <PVOID> (IoAllocateMdl(pVirtualAddress,
                                                ulNumberOfBytes,
                                                false,
                                                false,
                                                NULL));
    if( !m_pMDL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IoAllocateMdl failed"));
        return NULL;
    }
    // build MDL for given memory
    // this function does not support a return value, so we
    // assume that it never fails
    MmBuildMdlForNonPagedPool( static_cast <PMDL> (m_pMDL) );
    // store number of bytes in member variable
    m_cBytes = ulNumberOfBytes;

    return pVirtualAddress;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Locks the physical pages mapped by the user-mode virtual address range
//  in memory. The memory description is updated to describe the physical
//  pages. Access rights are checked for user process access.
//  Callers must be running at IRQL <= DISPATCH_LEVEL.
//
// Return Value:
//  NULL               Operation failed,
//                     (a) if the function is called again after successful
//                     locking without a previous unlock.
//                     (b) if the memory cannot be allocated
//                     (c) if the pages connat be locked,
//  A valid pointer
//
//////////////////////////////////////////////////////////////////////////////
PVOID COSDependMemory::LockUserPages
(
    PVOID pVirtualAddress, //Points to the virtual address to be locked.
    size_t ulNumberOfBytes //Specifies the length in bytes.
)
{
    // check if parameters are valid
    if( !pVirtualAddress )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return NULL;
    }
    // check if parameters are within valid range
    if( (ulNumberOfBytes < 1) || (ulNumberOfBytes > 2097152) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return NULL;
    }
    // check current IRQ level
    if( KeGetCurrentIrql() > DISPATCH_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level not correct"));
        return NULL;
    }
    // check if member was initialized before
    if( m_pMDL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: LockUserPages called twice"));
        return NULL;
    }
    // allocate MDL for given memory
    m_pMDL = static_cast <PVOID> (IoAllocateMdl(pVirtualAddress,
                                                ulNumberOfBytes,
                                                false,
                                                false,
                                                NULL));
    if( !m_pMDL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IoAllocateMdl failed"));
        return NULL;
    }
    // the user mode buffers have to be checked before accessing them,
    // therefore a try/except is neccessary because "Probe" will raise
    // an exception if the memory is not accessible
    // the build of the MDL is made also with the call to
    // MmProbeAndLockPages
    __try
    {
        // this function does not support a return value, so we
        // assume that it never fails
        MmProbeAndLockPages(static_cast <PMDL> (m_pMDL),
                            static_cast <char> (UserMode),
                            IoModifyAccess);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: MmProbeAndLockPages failed"));
        // this function does not support a return value, so we
        // assume that it never fails
        IoFreeMdl(static_cast <PMDL> (m_pMDL));
        m_pMDL = NULL;
        return NULL;
    }
    // store number of bytes in member variable
    m_cBytes = ulNumberOfBytes;

   return pVirtualAddress;
}

VAMPRET
COSDependMemory::GetPageTableFromMappings(
    PVOID pMemList,
    PDWORD *ppdwPageEntries,     //Page table address.
    DWORD *pdwNumberOfEntries)   //Number of page table entries.
{
    PKSSTREAM_POINTER_OFFSET pOffset = (PKSSTREAM_POINTER_OFFSET) pMemList;
    PKSMAPPING pMappings = (PKSMAPPING) pOffset->Mappings;

    m_pulVirtualPTAddress =
        reinterpret_cast <PULONG> (new (NON_PAGED_POOL) BYTE[2*ONE_PAGE] );
    if( !m_pulVirtualPTAddress )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Not enough memory"));
        return VAMPRET_ALLOCATION_ERROR;
    }    

    // align page table to DWORD boundaries
    *ppdwPageEntries = ALIGN_PAGE( m_pulVirtualPTAddress );

    DWORD dwIndex = 0, dwMappingsIndex = 0, dwCurrentMappingSize = 0;
    DWORD dwMaxTableEntries = ONE_PAGE / sizeof(DWORD);
    UCHAR bResetMappingsIndex = 1;

    // format the entries
    while ((dwMappingsIndex < pOffset->Count) && (dwIndex < dwMaxTableEntries))
    {
        if (dwCurrentMappingSize == 0)     
            dwCurrentMappingSize = pMappings[dwMappingsIndex].ByteCount;

        if (bResetMappingsIndex)
        {
            // the device can handle 32 bit addresses only
            ASSERT(pMappings[dwMappingsIndex].PhysicalAddress.HighPart == 0);
            if (pMappings[dwMappingsIndex].PhysicalAddress.HighPart != 0)
            {
                delete m_pulVirtualPTAddress;
                m_pulVirtualPTAddress = NULL;
                return VAMPRET_ALLOCATION_ERROR;
            }
            (*ppdwPageEntries)[dwIndex] = pMappings[dwMappingsIndex].PhysicalAddress.LowPart & 0xfffff000;
            bResetMappingsIndex = 0;
        } else 
            (*ppdwPageEntries)[dwIndex] = (*ppdwPageEntries)[dwIndex - 1] + ONE_PAGE;

        dwIndex++;

        if (dwCurrentMappingSize > ONE_PAGE) dwCurrentMappingSize -= ONE_PAGE;
        else dwCurrentMappingSize = 0;       

        if (dwCurrentMappingSize == 0) 
        {
            dwMappingsIndex++;
            bResetMappingsIndex = 1;
        }
    }

    // calculate page table entries
    *pdwNumberOfEntries = dwIndex;

    if (dwIndex >= dwMaxTableEntries) 
    {
        delete m_pulVirtualPTAddress;
        m_pulVirtualPTAddress = NULL;
        return VAMPRET_ALLOCATION_ERROR;
    }

    return VAMPRET_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Calculates page table that contains physical page addresses extracted from
//  the given buffer.
//
// Return Value:
//  VAMPRET_GENERAL_ERROR       The function LockPages must have been called
//                              before.
//  VAMPRET_ALLOCATION_ERROR    Memory cannot be allocated. For more details
//                              see description of operator new.
//  VAMPRET_SUCCESS             Calculated page table with success.
//
//////////////////////////////////////////////////////////////////////////////
VAMPRET COSDependMemory::GetPageTable
(
    PVOID pHandle,              //unused
    PDWORD *ppdwPageEntries,     //Page table address.
    DWORD *pdwNumberOfEntries   //Number of page table entries.
)
{
    // for WDM implementation, MDL is stored in the COSDependMemory object
    // and the handle given to the user is irrelevant, so check if MDL
    // member is initialized correctly instead of checking the handle
    if( !m_pMDL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Important member not initialized"));
        return VAMPRET_GENERAL_ERROR;
    }
    // cast MDL member from PVOID to PMDL for easier access
    PMDL pMDL = static_cast <PMDL> (m_pMDL);
    // cast MDL base address from PMDL to PDWORD for easier access
    PULONG_PTR ppFixedPages = reinterpret_cast <PULONG_PTR> (&pMDL[1]);

    m_pulVirtualPTAddress =
        reinterpret_cast <PULONG> (new (NON_PAGED_POOL) BYTE[2*ONE_PAGE] );
    if( !m_pulVirtualPTAddress )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Not enough memory"));
        return VAMPRET_ALLOCATION_ERROR;
    }

    // align page table to PAGE boundaries (new/ExAllocatePool already returns page aligned addresses)
    *ppdwPageEntries = ALIGN_PAGE( m_pulVirtualPTAddress );
    // calculate page table entries
    *pdwNumberOfEntries = BYTES_TO_PAGES(m_cBytes);
    // format the entries
    for( DWORD dwIndex = 0; dwIndex < *pdwNumberOfEntries; dwIndex++ )
    {
        // The device (DMA controller) can handle 32 bit addresses only.
        // The MDL itself contains a list of PFNs hence the PAGE_SHIFT
        ASSERT((ppFixedPages[dwIndex] & (0xffffffff >> PAGE_SHIFT)) == ppFixedPages[dwIndex]);
        if ((ppFixedPages[dwIndex] & (0xffffffff >> PAGE_SHIFT)) != ppFixedPages[dwIndex])
        {
            delete m_pulVirtualPTAddress;
            m_pulVirtualPTAddress = NULL;
            return VAMPRET_ALLOCATION_ERROR;
        }
        (*ppdwPageEntries)[dwIndex] = (DWORD) (ppFixedPages[dwIndex] << PAGE_SHIFT);
    }
    return VAMPRET_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Unlocks the physical pages mapped by the virtual address range in memory.
//  Callers must be running at IRQL <= DISPATCH_LEVEL.
//
// Return Value:
//  none.
//
//////////////////////////////////////////////////////////////////////////////
void COSDependMemory::UnlockPages
(
    PVOID pHandle //unused
)
{
    // for WDM implementation, MDL is stored in the COSDependMemory object
    // and the handle given to the user is irrelevant, so check if MDL
    // member is initialized correctly instead of checking the handle
    if ( !m_pMDL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Important member not initialized"));
        return;
    }
    // check current IRQ level
    if( KeGetCurrentIrql() > DISPATCH_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level not correct"));
        return;
    }
    // de-allocate MDL
    // this function does not support a return value, so we
    // assume that it never fails
    IoFreeMdl( static_cast <PMDL> (m_pMDL) );
    m_pMDL = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Unlocks the physical user pages mapped by the virtual address range in
//  memory.
//  Callers must be running at IRQL <= DISPATCH_LEVEL.
//
// Return Value:
//  none.
//
//////////////////////////////////////////////////////////////////////////////
void COSDependMemory::UnlockUserPages
(
    PVOID pHandle //unused
)
{
    // for WDM implementation, MDL is stored in the COSDependMemory object
    // and the handle given to the user is irrelevant, so check if MDL
    // member is initialized correctly instead of checking the handle
    if ( !m_pMDL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Important member not initialized"));
        return;
    }
    // check current IRQ level
    if( KeGetCurrentIrql() > DISPATCH_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level not correct"));
        return;
    }
    // unlock user pages
    // this function does not support a return value, so we
    // assume that it never fails
    MmUnlockPages( static_cast <PMDL> (m_pMDL) );
    // de-allocate MDL
    // this function does not support a return value, so we
    // assume that it never fails
    IoFreeMdl( static_cast <PMDL> (m_pMDL) );
    m_pMDL = NULL;
}


