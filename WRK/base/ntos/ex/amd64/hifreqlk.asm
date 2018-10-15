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
;   hifreqlk.asm
;
; Abstract:
;
;   High frequency system spin locks are declared in this module. Each spin
;   lock is placed in its own cache line on MP systems.
;
;--

include ksamd64.inc

ifdef NT_UP

ALIGN_VALUE equ 16

else

ALIGN_VALUE equ 64

endif

_DATA$00 SEGMENT PAGE 'DATA'

;
; The Initial PCR must be the first allocation in the section so it will be
; page aligned.
;

        public  KiInitialPCR
KiInitialPCR    db ProcessorControlRegisterLength dup (0)

;
; These values are referenced together so they are defined in a single cache
; line. They are never modified after they are initialized on during boot.
;
; pIofCallDriver - This is a pointer to the function to call a driver.
;

        align   ALIGN_VALUE

        public  pIofCallDriver
pIofCallDriver  dq 0

;
; pIofCompleteRequest - This is a pointer to the function to call to complete
;      an I/O request.
;

        public  pIofCompleteRequest
pIofCompleteRequest dq 0

;
; pIoAllocateIrp - This is pointer to the function to call to allocate an IRP.
;

        public  pIoAllocateIrp
pIoAllocateIrp  dq 0

;
; pIoFreeIrp - This is a pointer to a function to call to free an IRP.
;

        public  pIoFreeIrp
pIoFreeIrp      dq 0

;
; These variables are referenced together and are defined in a single cache
; line to reduce sharing on MP systems.
;
; KiIdleSummary - This is the set of processors which are idle.  It is
;      used by the ready thread code to speed up the search for a thread
;      to preempt when a thread becomes runnable.
;

        align   ALIGN_VALUE

        public  KiIdleSummary
KiIdleSummary   dq 0

;
; KiIdleSMTSummary - This is the set of multithreaded processors whose physical
;       processors are totally idle.
;

        public  KiIdleSMTSummary
KiIdleSMTSummary dq 0

;
; KiTbFlushTimeStamp - This is the TB flush entire time stamp counter.
;
; This variable is in it own cache line to reduce false sharing on MP systems.
;

        align   ALIGN_VALUE

        public  KiTbFlushTimeStamp
KiTbFlushTimeStamp dd 0

;
; These variables are referenced together and are defined in a single cache
; line to reduce sharing on MP systems.
;
; KiTimeIncrementReciprocal - This is the reciprocal fraction of the time
;      increment value that is specified by the HAL when the system is
;      booted.
;

        align   ALIGN_VALUE

        public KiTimeIncrementReciprocal
KiTimeIncrementReciprocal dq 0

;
; KiTimeIncrement - This is the time increment value of the last clock
;      interrupt. This value is stored by update system time and is consumed
;      by the secondary clock interrupt routine.
;

        public  KiTimeIncrement
KiTimeIncrement dq 0

;
; KiTimeIncrementShiftCount - This is the shift count that corresponds to
;      the time increment reciprocal value.
;

        public KiTimeIncrementShiftCount
KiTimeIncrementShiftCount dd 0

;
; KeMaximumIncrement - This is the maximum time between clock interrupts
;      in 100ns units that is supported by the host HAL.
;

        public  KeMaximumIncrement
KeMaximumIncrement dd 0

;
; KeTimeAdjustment - This is the actual number of 100ns units that are to
;      be added to the system time at each interval timer interupt. This
;      value is copied from KeTimeIncrement at system start up and can be
;      later modified via the set system information service.
;      timer table entries.
;

        public  KeTimeAdjustment
KeTimeAdjustment dd 0

;
; KiTickOffset - This is the number of 100ns units remaining before a tick
;      is added to the tick count and the system time is updated.
;

        public  KiTickOffset
KiTickOffset    dd 0

;
; These variables are referenced together and are defined in a single cache
; line to reduce sharing on MP systems.
;
; KiMaximumDpcQueueDepth - This is used to control how many DPCs can be
;      queued before a DPC of medium importance will trigger a dispatch
;      interrupt.
;

        align   ALIGN_VALUE

        public  KiMaximumDpcQueueDepth
KiMaximumDpcQueueDepth dd 4

;
; KiMinimumDpcRate - This is the rate of DPC requests per clock tick that
;      must be exceeded before DPC batching of medium importance DPCs
;      will occur.
;

        public  KiMinimumDpcRate
KiMinimumDpcRate dd 3

;
; KiAdjustDpcThreshold - This is the threshold used by the clock interrupt
;      routine to control the rate at which the processor's DPC queue depth
;      is dynamically adjusted.
;

        public  KiAdjustDpcThreshold
KiAdjustDpcThreshold dd 20

;
; KiIdealDpcRate - This is used to control the aggressiveness of the DPC
;      rate adjusting algorithm when decrementing the queue depth. As long
;      as the DPC rate for the last tick is greater than this rate, the
;      DPC queue depth will not be decremented.
;

        public  KiIdealDpcRate
KiIdealDpcRate  dd 20

;
; MmPaeMask - This is the value used to mask upper bits of a PAE PTE.
;
; This variable is in it own cache line to reduce false sharing on MP systems.
;

        align   ALIGN_VALUE

        public  MmPaeMask
MmPaeMask       dq 0

;
; MmHighestUserAddress - This is the highest user virtual address.
;

        public  MmHighestUserAddress
MmHighestUserAddress dq 0

;
; MmSystemRangeStart - This is the start of the system virtual address space.
;

        public  MmSystemRangeStart
MmSystemRangeStart dq 0

;
; MmUserProbeAddress - This is the address that is used to probe user buffers.
;

        public  MmUserProbeAddress
MmUserProbeAddress dq 0

;
; MmHighestPhysicalPage - This is the highest physical page number in the
;       system.
;

        public  MmHighestPhysicalPage
MmHighestPhysicalPage dq 0

;
; MmPfnDatabase - This is the base address of the PFN database.
;

        public  MmPfnDatabase
MmPfnDatabase   dq 0

;
; MmPaeErrMask - This is the PAE error mask which is used to mask the page fault
;       error code.
;

        public  MmPaeErrMask
MmPaeErrMask    dd 0

;
; MmSecondaryColors - This is the number of secondary page colors as determined
;       by the size of the second level cache.
;

        public  MmSecondaryColors
MmSecondaryColors dd 0

;
; MmSecondaryColorMask - This is the secondary color mask.
;

        public  MmSecondaryColorMask
MmSecondaryColorMask dd 0

;
; MmSecondaryColorNodeShift - This is the secondary color node shift.
;

        public  MmSecondaryColorNodeShift
MmSecondaryColorNodeShift db 0

;
; MmPfnDereferenceSListHead - This is used to store free blocks used for
;       deferred PFN reference count releasing.
;
; This variable is in it own cache line to reduce false sharing on MP systems.
;

        align   ALIGN_VALUE

        public  MmPfnDereferenceSListHead
MmPfnDereferenceSListHead dq 2 dup (0)

;
; MmPfnDeferredList - This is used to queue items that need reference count
;      decrement processing.
;
; This variable is in it own cache line to reduce false sharing on MP systems.
;

        align   ALIGN_VALUE

        public  MmPfnDeferredList
MmPfnDeferredList dq 0

;
; MmSystemLockPagesCount - This is the count of the number of locked pages
;       in the system.
;

        align   ALIGN_VALUE

        public  MmSystemLockPagesCount
MmSystemLockPagesCount dq 0

;
; KdDebuggerNotPresent - This marks the beginning of a kernel debugger
;       information structure so that it can be located by a debugger DLL.
;

        align   ALIGN_VALUE

        public KdDebuggerNotPresent                     
        public KiTestDividend

KiTestDividend      db  04ch
                    db  072h
                    db  053h
                    db  0A0h
                    db  0A3h
                    db  05Fh
                    db  04Bh
KdDebuggerNotPresent db 000h

_DATA$00 ends

        end

