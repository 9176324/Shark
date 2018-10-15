        title "Global SpinLock declerations"
;++
;
; Copyright (c) Microsoft Corporation. All rights reserved. 
;
; You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
; If you do not agree to the terms, do not use the code.
;
;
; Module Name:
;
;    splocks.asm
;
; Abstract:
;
;    All global spinlocks in the kernel image are declared in this
;    module.  This is done so that each spinlock can be spaced out
;    sufficiently to guarantee that the L2 cache does not thrash
;    by having a spinlock and another high use variable in the same
;    cache line.
;
;--

.386p
        .xlist

ifdef NT_UP

PADLOCKS equ    4

_DATA   SEGMENT PARA PUBLIC 'DATA'

else

PADLOCKS equ    128

_DATA   SEGMENT PAGE PUBLIC 'DATA'

endif

ULONG   macro   VariableName

        align   PADLOCKS

        public  VariableName
VariableName    dd      0

        endm

;
; These values are referenced together so they are defined in a single cache
; line. They are never modified after they are initialized on during boot.
;
; pIofCallDriver - This is a pointer to the function to call a driver.
;

        align   PADLOCKS

        public  _pIofCallDriver
_pIofCallDriver dd      0

;
; pIofCompleteRequest - This is a pointer to the function to call to complete
;      an I/O request.
;

        public  _pIofCompleteRequest
_pIofCompleteRequest dd 0

;
; pIoAllocateIrp - This is pointer to the function to call to allocate an IRP.
;

        public  _pIoAllocateIrp
_pIoAllocateIrp dd      0

;
; pIoFreeIrp - This is a pointer to a function to call to free an IRP.
;

        public  _pIoFreeIrp
_pIoFreeIrp     dd      0

;
; These variables are updated frequently and under control of the dispatcher
; database lock. They are defined in a single cache line to reduce false
; sharing in MP systems.
;
; KiIdleSummary - This is the set of processors which are idle.  It is
;      used by the ready thread code to speed up the search for a thread
;      to preempt when a thread becomes runnable.
;

        align   PADLOCKS

        public  _KiIdleSummary
_KiIdleSummary  dd      0

;
; KiIdleSMTSummary - In multi threaded processors, this is the set of
;      idle processors in which all the logical processors that make up a
;      physical processor are idle.   That is, this is the set of logical
;      processors in completely idle physical processors.
;

        public  _KiIdleSMTSummary
_KiIdleSMTSummary dd    0

;
; KiTbFlushTimeStamp - This is the TB flush entire time stamp counter.
;
; This variable is in it own cache line to reduce false sharing on MP systems.
;

        align   PADLOCKS

        public  _KiTbFlushTimeStamp
_KiTbFlushTimeStamp dd      0

;
; These variables are referenced together and are defined in a single cache
; line to reduce sharing on MP systems.
;
; KeTickCount - This is the number of clock ticks that have occurred since
;      the system was booted. This count is used to compute a millisecond
;      tick counter.
;

        align   PADLOCKS

        public  _KeTickCount
_KeTickCount    dd      0, 0, 0

;
; KeMaximumIncrement - This is the maximum time between clock interrupts
;      in 100ns units that is supported by the host HAL.
;

        public  _KeMaximumIncrement
_KeMaximumIncrement dd      0

;
; KiTimeIncrementReciprocal - This is the reciprocal fraction of the time
;      increment value that is specified by the HAL when the system is
;      booted.
;

        public  _KiTimeIncrementReciprocal
_KiTimeIncrementReciprocal dq 0

;
; KeTimeAdjustment - This is the actual number of 100ns units that are to
;      be added to the system time at each interval timer interupt. This
;      value is copied from KeTimeIncrement at system start up and can be
;      later modified via the set system information service.
;      timer table entries.
;

        public  _KeTimeAdjustment
_KeTimeAdjustment dd      0

;
; KiTimeIncrementShiftCount - This is the shift count that corresponds to
;      the time increment reciprocal value.
;

        public  _KiTimeIncrementShiftCount
_KiTimeIncrementShiftCount dd 0

;
; KiTickOffset - This is the number of 100ns units remaining before a tick
;      is added to the tick count and the system time is updated.
;

        public  _KiTickOffset
_KiTickOffset   dd      0

;
; These variables are referenced together and are defined in a single cache
; line to reduce sharing on MP systems.
;
; KiMaximumDpcQueueDepth - This is used to control how many DPCs can be
;      queued before a DPC of medium importance will trigger a dispatch
;      interrupt.
;

        align   PADLOCKS

        public  _KiMaximumDpcQueueDepth
_KiMaximumDpcQueueDepth dd      4

;
; KiMinimumDpcRate - This is the rate of DPC requests per clock tick that
;      must be exceeded before DPC batching of medium importance DPCs
;      will occur.
;

        public  _KiMinimumDpcRate
_KiMinimumDpcRate dd      3

;
; KiAdjustDpcThreshold - This is the threshold used by the clock interrupt
;      routine to control the rate at which the processor's DPC queue depth
;      is dynamically adjusted.
;

        public  _KiAdjustDpcThreshold
_KiAdjustDpcThreshold dd      20

;
; KiIdealDpcRate - This is used to control the aggressiveness of the DPC
;      rate adjusting algorithm when decrementing the queue depth. As long
;      as the DPC rate for the last tick is greater than this rate, the
;      DPC queue depth will not be decremented.
;

        public  _KiIdealDpcRate
_KiIdealDpcRate dd      20

;
; These variables are referenced together and are defined in a single cache
; line to reduce sharing on MP systems.
;
; KeErrorMask - This is the value used to mask the error code passed to
;      memory management on page faults.
;

        align   PADLOCKS

        public  _KeErrorMask
_KeErrorMask    dd      1

;
; MmPaeErrMask - This is the value used to mask upper bits of a PAE error.
;

        public  _MmPaeErrMask
_MmPaeErrMask   dd      0

;
; MmPaeMask - This is the value used to mask upper bits of a PAE PTE.
;

        public  _MmPaeMask
_MmPaeMask      dq      0

;
; MmHighestUserAddress - This is the highest user virtual address.
;

        public  _MmHighestUserAddress
_MmHighestUserAddress dd 0

;
; MmSystemRangeStart - This is the start of the system virtual address space.
;

        public  _MmSystemRangeStart
_MmSystemRangeStart dd 0

;
; MmUserProbeAddress - This is the address that is used to probe user buffers.
;

        public  _MmUserProbeAddress
_MmUserProbeAddress dd 0

;
; MmHighestPhysicalPage - This is the highest physical page number in the
;       system.
;

        public  _MmHighestPhysicalPage
_MmHighestPhysicalPage dd 0

;
; MmPfnDatabase - This is the base address of the PFN database.
;

        public  _MmPfnDatabase
_MmPfnDatabase  dd 0

;
; MmSecondaryColors - This is the number of secondary page colors as determined
;       by the size of the second level cache.
;

        public  _MmSecondaryColors
_MmSecondaryColors dd 0

;
; MmSecondaryColorMask - This is the secondary color mask.
;

        public  _MmSecondaryColorMask
_MmSecondaryColorMask dd 0

;
; MmSecondaryColorNodeShift - This is the secondary color node shift.
;

        public  _MmSecondaryColorNodeShift
_MmSecondaryColorNodeShift db 0

;
; MmPfnDereferenceSListHead - This is used to store free blocks used for
;      deferred PFN reference count releasing.
;

        align   PADLOCKS

        public  _MmPfnDereferenceSListHead
_MmPfnDereferenceSListHead  dq      0

;
; MmPfnDeferredList - This is used to queue items that need reference count
;      decrement processing.
;

        align   PADLOCKS

        public  _MmPfnDeferredList
_MmPfnDeferredList          dd      0

;
; MmSystemLockPagesCount - This is the count of the number of locked pages
;       in the system.
;

        align   PADLOCKS

        public  _MmSystemLockPagesCount
_MmSystemLockPagesCount     dd      0

        align   PADLOCKS

_DATA   ends

        end

