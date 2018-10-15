/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    pinsup.c

Abstract:

    This module implements the pointer-based Pin support routines for the
    Cache subsystem.

--*/

#include "cc.h"

//
//  Define our debug constant
//

#define me 0x00000008

//
//  Internal routines
//

BOOLEAN
CcMapDataCommon (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG Flags,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    );

VOID
CcMapDataForOverwrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    );

POBCB
CcAllocateObcb (
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN PBCB FirstBcb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CcPinMappedData)
#pragma alloc_text(PAGE,CcPinRead)
#pragma alloc_text(PAGE,CcPreparePinWrite)
#pragma alloc_text(PAGE,CcMapDataCommon)
#pragma alloc_text(PAGE,CcMapData)
#pragma alloc_text(PAGE,CcUnpinData)
#pragma alloc_text(PAGE,CcSetBcbOwnerPointer)
#pragma alloc_text(PAGE,CcUnpinDataForThread)
#pragma alloc_text(PAGE,CcAllocateObcb)
#endif



BOOLEAN
CcMapData (
    __in PFILE_OBJECT FileObject,
    __in PLARGE_INTEGER FileOffset,
    __in ULONG Length,
    __in ULONG Flags,
    __out PVOID *Bcb,
    __deref_out_bcount(Length) PVOID *Buffer
    )

/*++

Routine Description:

    This routine attempts to map the specified file data in the cache.
    A pointer is returned to the desired data in the cache.

    If the caller does not want to block on this call, then
    Wait should be supplied as FALSE.  If Wait was supplied as FALSE and
    it is currently impossible to supply the requested data without
    blocking, then this routine will return FALSE.  However, if the
    data is immediately accessible in the cache and no blocking is
    required, this routine returns TRUE with a pointer to the data.

    Note that a call to this routine with Wait supplied as TRUE is
    considerably faster than a call with Wait supplies as FALSE, because
    in the Wait TRUE case we only have to make sure the data is mapped
    in order to return.

    It is illegal to modify data that is only mapped, and can in fact lead
    to serious problems.  It is impossible to check for this in all cases,
    however CcSetDirtyPinnedData may implement some Assertions to check for
    this.  If the caller wishes to modify data that it has only mapped, then
    it must *first* call CcPinMappedData.

    In any case, the caller MUST subsequently call CcUnpinData.
    Naturally if CcPinRead or CcPreparePinWrite were called multiple
    times for the same data, CcUnpinData must be called the same number
    of times.

    The returned Buffer pointer is valid until the data is unpinned, at
    which point it is invalid to use the pointer further.  This buffer pointer
    will remain valid if CcPinMappedData is called.

    Note that under some circumstances (like Wait supplied as FALSE or more
    than a page is requested), this routine may actually pin the data, however
    it is not necessary, and in fact not correct, for the caller to be concerned
    about this.

Arguments:

    FileObject - Pointer to the file object for a file which was
                 opened with NO_INTERMEDIATE_BUFFERING clear, i.e., for
                 which CcInitializeCacheMap was called by the file system.

    FileOffset - Byte offset in file for desired data.

    Length - Length of desired data in bytes.

    Wait - FALSE if caller may not block, TRUE otherwise (see description
           above)

    Bcb - On the first call this returns a pointer to a Bcb
          parameter which must be supplied as input on all subsequent
          calls, for this buffer

    Buffer - Returns pointer to desired data, valid until the buffer is
             unpinned or freed.  This pointer will remain valid if CcPinMappedData
             is called.

Return Value:

    FALSE - if Wait was supplied as FALSE and the data was not delivered

    TRUE - if the data is being delivered

--*/

{
    ULONG SavedState;
    volatile UCHAR ch;
    PVOID TempBcb;
    PVOID BaseAddress;
    ULONG PageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES((ULongToPtr(FileOffset->LowPart)), Length);
    PETHREAD Thread = PsGetCurrentThread();
    BOOLEAN ReturnStatus;

    DebugTrace(+1, me, "CcMapData\n", 0 );

    MmSavePageFaultReadAhead( Thread, &SavedState );

    ReturnStatus = CcMapDataCommon( FileObject,
                                    FileOffset,
                                    Length,
                                    Flags,
                                    &TempBcb,
                                    Buffer );

    if (!ReturnStatus) {

        //
        //  No need to revert the CcMissCounter in this case because we can
        //  only get here if Wait was FALSE and the CcMissCounter isn't set
        //  by CcMapDataCommon if Wait is FALSE.
        //

        return FALSE;
    }
    
    //
    //  Caller specifically requested he doesn't want data to be faulted in.
    //

    if (!FlagOn( Flags, MAP_NO_READ )) {

        //
        //  Now let's just sit here and take the miss(es) like a man (and count them).
        //

        try {

            //
            //  Loop to touch each page
            //

            BaseAddress = *Buffer;

            while (PageCount != 0) {

                MmSetPageFaultReadAhead( Thread, PageCount - 1 );

                ch = *((volatile UCHAR *)(BaseAddress));

                BaseAddress = (PCHAR) BaseAddress + PAGE_SIZE;
                PageCount -= 1;
            }

        } finally {

            MmResetPageFaultReadAhead( Thread, SavedState );

            if (AbnormalTermination() && (TempBcb != NULL)) {
                CcUnpinFileData( (PBCB)TempBcb, TRUE, UNPIN );
            }
        }
    }

    //
    //  CcMapDataCommon set the appropriate miss counter, but we need to reset
    //  it at this point.
    //

    CcMissCounter = &CcThrowAway;

    //
    //  Increment the pointer as a reminder that it is read only, and
    //  return it.  We pend this until now to avoid raising with a valid
    //  Bcb into caller's contexts.
    //

    *(PCHAR *)&TempBcb += 1;
    *Bcb = TempBcb;

    DebugTrace(-1, me, "CcMapData -> TRUE\n", 0 );

    return TRUE;
}


BOOLEAN
CcMapDataCommon (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG Flags,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    )

/*++

Routine Description:

    This routine attempts to map the specified file data in the cache.
    A pointer is returned to the desired data in the cache.

    If the caller does not want to block on this call, then
    Wait should be supplied as FALSE.  If Wait was supplied as FALSE and
    it is currently impossible to supply the requested data without
    blocking, then this routine will return FALSE.  However, if the
    data is immediately accessible in the cache and no blocking is
    required, this routine returns TRUE with a pointer to the data.

    Note that a call to this routine with Wait supplied as TRUE is
    considerably faster than a call with Wait supplies as FALSE, because
    in the Wait TRUE case we only have to make sure the data is mapped
    in order to return.

    If data is only mapped, Cc does not setup the structures to track the
    pages as dirty.  If the caller plans to dirty the data, it must do one
    of the following:
    * Call CcPinMappedData - Cc will then setup the structures to track that
        the buffer is dirty.  Then the caller can dirty the buffer and call
        CcSetDirtyPinnedData to tell Cc about it.
    * Track the dirty data itself.  If the caller chooses to do this, he/she
        is responsible for setting the address range modified after it is
        written, but before trying to flush the data.
        
    In any case, the caller MUST subsequently call CcUnpinData.
    Naturally if CcPinRead or CcPreparePinWrite were called multiple
    times for the same data, CcUnpinData must be called the same number
    of times.

    The returned Buffer pointer is valid until the data is unpinned, at
    which point it is invalid to use the pointer further.  This buffer pointer
    will remain valid if CcPinMappedData is called.

    Note that under some circumstances (like Wait supplied as FALSE or more
    than a page is requested), this routine may actually pin the data, however
    it is not necessary, and in fact not correct, for the caller to be concerned
    about this.

Arguments:

    FileObject - Pointer to the file object for a file which was
                 opened with NO_INTERMEDIATE_BUFFERING clear, i.e., for
                 which CcInitializeCacheMap was called by the file system.

    FileOffset - Byte offset in file for desired data.

    Length - Length of desired data in bytes.

    Wait - FALSE if caller may not block, TRUE otherwise (see description
           above)

    Bcb - On the first call this returns a pointer to a Bcb
          parameter which must be supplied as input on all subsequent
          calls, for this buffer

    Buffer - Returns pointer to desired data, valid until the buffer is
             unpinned or freed.  This pointer will remain valid if CcPinMappedData
             is called.

Return Value:

    FALSE - if Wait was supplied as FALSE and the data was not delivered

    TRUE - if the data is being delivered

--*/

{
    PSHARED_CACHE_MAP SharedCacheMap;
    LARGE_INTEGER BeyondLastByte;
    ULONG ReceivedLength;
    PVOID TempBcb;
    PETHREAD Thread = PsGetCurrentThread();

    DebugTrace(+1, me, "CcMapDataCommon\n", 0 );

    //
    //  Increment performance counters
    //

    if (FlagOn(Flags, MAP_WAIT)) {

        CcMapDataWait += 1;

        //
        //  Initialize the indirect pointer to our miss counter.
        //

        CcMissCounter = &CcMapDataWaitMiss;

    } else {
        CcMapDataNoWait += 1;
    }

    //
    //  Get pointer to SharedCacheMap.
    //

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

    //
    //  Call local routine to Map or Access the file data.  If we cannot map
    //  the data because of a Wait condition, return FALSE.
    //

    if (FlagOn(Flags, MAP_WAIT)) {

        *Buffer = CcGetVirtualAddress( SharedCacheMap,
                                       *FileOffset,
                                       (PVACB *)&TempBcb,
                                       &ReceivedLength );

        ASSERT( ReceivedLength >= Length );

    } else if (!CcPinFileData( FileObject,
                               FileOffset,
                               Length,
                               TRUE,
                               FALSE,
                               Flags,
                               (PBCB *)&TempBcb,
                               Buffer,
                               &BeyondLastByte )) {

        DebugTrace(-1, me, "CcMapData -> FALSE\n", 0 );

        CcMapDataNoWaitMiss += 1;

        return FALSE;

    } else {

        ASSERT( (BeyondLastByte.QuadPart - FileOffset->QuadPart) >= Length );

    }

    //
    //  Even though this pointer is read-only, we will leave it to the 
    //  caller to mark it as such since the caller will need to do some
    //  addition operations on this pointer before returning to the request
    //  originator.
    //

    *Bcb = TempBcb;

    DebugTrace(-1, me, "CcMapDataCommon -> TRUE\n", 0 );

    return TRUE;
}


BOOLEAN
CcPinMappedData (
    __in PFILE_OBJECT FileObject,
    __in PLARGE_INTEGER FileOffset,
    __in ULONG Length,
    __in ULONG Flags,
    __inout PVOID *Bcb
    )

/*++

Routine Description:

    This routine attempts to pin data that was previously only mapped.
    If the routine determines that in fact it was necessary to actually
    pin the data when CcMapData was called, then this routine does not
    have to do anything.

    If the caller does not want to block on this call, then
    Wait should be supplied as FALSE.  If Wait was supplied as FALSE and
    it is currently impossible to supply the requested data without
    blocking, then this routine will return FALSE.  However, if the
    data is immediately accessible in the cache and no blocking is
    required, this routine returns TRUE with a pointer to the data.

    If the data is not returned in the first call, the caller
    may request the data later with Wait = TRUE.  It is not required
    that the caller request the data later.

    If the caller subsequently modifies the data, it should call
    CcSetDirtyPinnedData.

    In any case, the caller MUST subsequently call CcUnpinData.
    Naturally if CcPinRead or CcPreparePinWrite were called multiple
    times for the same data, CcUnpinData must be called the same number
    of times.

    Note there are no performance counters in this routine, as the misses
    will almost always occur on the map above, and there will seldom be a
    miss on this conversion.

Arguments:

    FileObject - Pointer to the file object for a file which was
                 opened with NO_INTERMEDIATE_BUFFERING clear, i.e., for
                 which CcInitializeCacheMap was called by the file system.

    FileOffset - Byte offset in file for desired data.

    Length - Length of desired data in bytes.

    Flags - (PIN_WAIT, PIN_EXCLUSIVE, PIN_NO_READ, etc. as defined in cache.h)
            If the caller specifies PIN_NO_READ and PIN_EXCLUSIVE, then he must
            guarantee that no one else will be attempting to map the view, if he
            wants to guarantee that the Bcb is not mapped (view may be purged).
            If the caller specifies PIN_NO_READ without PIN_EXCLUSIVE, the data
            may or may not be mapped in the return Bcb.

    Bcb - On the first call this returns a pointer to a Bcb
          parameter which must be supplied as input on all subsequent
          calls, for this buffer

Return Value:

    FALSE - if Wait was not set and the data was not delivered

    TRUE - if the data is being delivered

--*/

{
    PVOID Buffer;
    LARGE_INTEGER BeyondLastByte;
    PSHARED_CACHE_MAP SharedCacheMap;
    LARGE_INTEGER LocalFileOffset = *FileOffset;
    POBCB MyBcb = NULL;
    PBCB *CurrentBcbPtr = (PBCB *)&MyBcb;
    BOOLEAN Result = FALSE;

    DebugTrace(+1, me, "CcPinMappedData\n", 0 );

    //
    // If the Bcb is no longer ReadOnly, then just return.
    //

    if ((*(PULONG)Bcb & 1) == 0) {
        return TRUE;
    }

    //
    // Remove the Read Only flag
    //

    *(PCHAR *)Bcb -= 1;

    //
    //  Get pointer to SharedCacheMap.
    //

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

    //
    //  We only count the calls to this routine, since they are almost guaranteed
    //  to be hits.
    //

    CcPinMappedDataCount += 1;

    //
    //  Guarantee we will put the flag back if required.
    //

    try {

        if (((PBCB)*Bcb)->NodeTypeCode != CACHE_NTC_BCB) {

            //
            //  Form loop to handle occasional overlapped Bcb case.
            //

            do {

                //
                //  If we have already been through the loop, then adjust
                //  our file offset and length from the last time.
                //

                if (MyBcb != NULL) {

                    //
                    //  If this is the second time through the loop, then it is time
                    //  to handle the overlap case and allocate an OBCB.
                    //

                    if (CurrentBcbPtr == (PBCB *)&MyBcb) {

                        MyBcb = CcAllocateObcb( FileOffset, Length, (PBCB)MyBcb );

                        //
                        //  Set CurrentBcbPtr to point at the first entry in
                        //  the vector (which is already filled in), before
                        //  advancing it below.
                        //

                        CurrentBcbPtr = &MyBcb->Bcbs[0];
                    }

                    Length -= (ULONG)(BeyondLastByte.QuadPart - LocalFileOffset.QuadPart);
                    LocalFileOffset.QuadPart = BeyondLastByte.QuadPart;
                    CurrentBcbPtr += 1;
                }

                //
                //  Call local routine to Map or Access the file data.  If we cannot map
                //  the data because of a Wait condition, return FALSE.
                //

                if (!CcPinFileData( FileObject,
                                    &LocalFileOffset,
                                    Length,
                                    (BOOLEAN)!FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED),
                                    FALSE,
                                    Flags,
                                    CurrentBcbPtr,
                                    &Buffer,
                                    &BeyondLastByte )) {

                    try_return( Result = FALSE );
                }

            //
            //  Continue looping if we did not get everything.
            //

            } while((BeyondLastByte.QuadPart - LocalFileOffset.QuadPart) < Length);

            //
            //  Free the Vacb before going on.
            //

            CcFreeVirtualAddress( (PVACB)*Bcb );

            *Bcb = MyBcb;

            //
            //  Debug routines used to insert and remove Bcbs from the global list
            //

        }

        //
        //  If he really has a Bcb, all we have to do is acquire it shared since he is
        //  no longer ReadOnly.
        //

        else {

            if (!ExAcquireSharedStarveExclusive( &((PBCB)*Bcb)->Resource, BooleanFlagOn(Flags, PIN_WAIT))) {

                try_return( Result = FALSE );
            }
        }

        Result = TRUE;

    try_exit: NOTHING;
    }
    finally {

        if (!Result) {

            //
            //  Put the Read Only flag back
            //

            *(PCHAR *)Bcb += 1;

            //
            //  We may have gotten partway through
            //

            if (MyBcb != NULL) {
                CcUnpinData( MyBcb );
            }
        }

        DebugTrace(-1, me, "CcPinMappedData -> %02lx\n", Result );
    }
    return Result;
}


BOOLEAN
CcPinRead (
    __in PFILE_OBJECT FileObject,
    __in PLARGE_INTEGER FileOffset,
    __in ULONG Length,
    __in ULONG Flags,
    __out PVOID *Bcb,
    __deref_out_bcount(Length) PVOID *Buffer
    )

/*++

Routine Description:

    This routine attempts to pin the specified file data in the cache.
    A pointer is returned to the desired data in the cache.  This routine
    is intended for File System support and is not intended to be called
    from Dpc level.

    If the caller does not want to block on this call, then
    Wait should be supplied as FALSE.  If Wait was supplied as FALSE and
    it is currently impossible to supply the requested data without
    blocking, then this routine will return FALSE.  However, if the
    data is immediately accessible in the cache and no blocking is
    required, this routine returns TRUE with a pointer to the data.

    If the data is not returned in the first call, the caller
    may request the data later with Wait = TRUE.  It is not required
    that the caller request the data later.

    If the caller subsequently modifies the data, it should call
    CcSetDirtyPinnedData.

    In any case, the caller MUST subsequently call CcUnpinData.
    Naturally if CcPinRead or CcPreparePinWrite were called multiple
    times for the same data, CcUnpinData must be called the same number
    of times.

    The returned Buffer pointer is valid until the data is unpinned, at
    which point it is invalid to use the pointer further.

Arguments:

    FileObject - Pointer to the file object for a file which was
                 opened with NO_INTERMEDIATE_BUFFERING clear, i.e., for
                 which CcInitializeCacheMap was called by the file system.

    FileOffset - Byte offset in file for desired data.

    Length - Length of desired data in bytes.

    Flags - (PIN_WAIT, PIN_EXCLUSIVE, PIN_NO_READ, etc. as defined in cache.h)
            If the caller specifies PIN_NO_READ and PIN_EXCLUSIVE, then he must
            guarantee that no one else will be attempting to map the view, if he
            wants to guarantee that the Bcb is not mapped (view may be purged).
            If the caller specifies PIN_NO_READ without PIN_EXCLUSIVE, the data
            may or may not be mapped in the return Bcb.

    Bcb - On the first call this returns a pointer to a Bcb
          parameter which must be supplied as input on all subsequent
          calls, for this buffer

    Buffer - Returns pointer to desired data, valid until the buffer is
             unpinned or freed.

Return Value:

    FALSE - if Wait was not set and the data was not delivered

    TRUE - if the data is being delivered

--*/

{
    PSHARED_CACHE_MAP SharedCacheMap;
    PVOID LocalBuffer;
    LARGE_INTEGER BeyondLastByte;
    LARGE_INTEGER LocalFileOffset = *FileOffset;
    POBCB MyBcb = NULL;
    PBCB *CurrentBcbPtr = (PBCB *)&MyBcb;
    BOOLEAN Result = FALSE;

    DebugTrace(+1, me, "CcPinRead\n", 0 );

    //
    //  Increment performance counters
    //

    if (FlagOn(Flags, PIN_WAIT)) {

        CcPinReadWait += 1;

        //
        //  Initialize the indirect pointer to our miss counter.
        //

        CcMissCounter = &CcPinReadWaitMiss;

    } else {
        CcPinReadNoWait += 1;
    }

    //
    //  Get pointer to SharedCacheMap.
    //

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

    try {

        //
        //  Form loop to handle occasional overlapped Bcb case.
        //

        do {

            //
            //  If we have already been through the loop, then adjust
            //  our file offset and length from the last time.
            //

            if (MyBcb != NULL) {

                //
                //  If this is the second time through the loop, then it is time
                //  to handle the overlap case and allocate an OBCB.
                //

                if (CurrentBcbPtr == (PBCB *)&MyBcb) {

                    MyBcb = CcAllocateObcb( FileOffset, Length, (PBCB)MyBcb );

                    //
                    //  Set CurrentBcbPtr to point at the first entry in
                    //  the vector (which is already filled in), before
                    //  advancing it below.
                    //

                    CurrentBcbPtr = &MyBcb->Bcbs[0];

                    //
                    //  Also on second time through, return starting Buffer
                    //

                    *Buffer = LocalBuffer;
                }

                Length -= (ULONG)(BeyondLastByte.QuadPart - LocalFileOffset.QuadPart);
                LocalFileOffset.QuadPart = BeyondLastByte.QuadPart;
                CurrentBcbPtr += 1;
            }

            //
            //  Call local routine to Map or Access the file data.  If we cannot map
            //  the data because of a Wait condition, return FALSE.
            //

            if (!CcPinFileData( FileObject,
                                &LocalFileOffset,
                                Length,
                                (BOOLEAN)!FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED),
                                FALSE,
                                Flags,
                                CurrentBcbPtr,
                                &LocalBuffer,
                                &BeyondLastByte )) {

                CcPinReadNoWaitMiss += 1;

                try_return( Result = FALSE );
            }

        //
        //  Continue looping if we did not get everything.
        //

        } while((BeyondLastByte.QuadPart - LocalFileOffset.QuadPart) < Length);

        *Bcb = MyBcb;

        //
        //  Debug routines used to insert and remove Bcbs from the global list
        //

        //
        //  In the normal (nonoverlapping) case we return the
        //  correct buffer address here.
        //

        if (CurrentBcbPtr == (PBCB *)&MyBcb) {
            *Buffer = LocalBuffer;
        }

        Result = TRUE;

    try_exit: NOTHING;
    }
    finally {

        CcMissCounter = &CcThrowAway;

        if (!Result) {

            //
            //  We may have gotten partway through
            //

            if (MyBcb != NULL) {
                CcUnpinData( MyBcb );
            }
        }

        DebugTrace(-1, me, "CcPinRead -> %02lx\n", Result );
    }

    return Result;
}


BOOLEAN
CcPreparePinWrite (
    __in PFILE_OBJECT FileObject,
    __in PLARGE_INTEGER FileOffset,
    __in ULONG Length,
    __in BOOLEAN Zero,
    __in ULONG Flags,
    __out PVOID *Bcb,
    __deref_out_bcount(Length) PVOID *Buffer
    )

/*++

Routine Description:

    This routine attempts to lock the specified file data in the cache
    and return a pointer to it along with the correct
    I/O status.  Pages to be completely overwritten may be satisfied
    with emtpy pages.

    If not all of the pages can be prepared, and Wait was supplied as
    FALSE, then this routine will return FALSE, and its outputs will
    be meaningless.  The caller may request the data later with
    Wait = TRUE.  However, it is not required that the caller request
    the data later.

    If Wait is supplied as TRUE, and all of the pages can be prepared
    without blocking, this call will return TRUE immediately.  Otherwise,
    this call will block until all of the pages can be prepared, and
    then return TRUE.

    When this call returns with TRUE, the caller may immediately begin
    to transfer data into the buffers via the Buffer pointer.  The
    buffer will already be marked dirty.

    The caller MUST subsequently call CcUnpinData.
    Naturally if CcPinRead or CcPreparePinWrite were called multiple
    times for the same data, CcUnpinData must be called the same number
    of times.

    The returned Buffer pointer is valid until the data is unpinned, at
    which point it is invalid to use the pointer further.

Arguments:

    FileObject - Pointer to the file object for a file which was
                 opened with NO_INTERMEDIATE_BUFFERING clear, i.e., for
                 which CcInitializeCacheMap was called by the file system.

    FileOffset - Byte offset in file for desired data.

    Length - Length of desired data in bytes.

    Zero - If supplied as TRUE, the buffer will be zeroed on return.  Ignored
           if PIN_CALLER_TRACKS_DIRTY_DATA is specified.

    Flags - (PIN_WAIT, PIN_EXCLUSIVE, PIN_NO_READ, etc. as defined in cache.h)
    
            If the caller specifies PIN_NO_READ and PIN_EXCLUSIVE, then he must
            guarantee that no one else will be attempting to map the view, if he
            wants to guarantee that the Bcb is not mapped (view may be purged).
            
            If the caller specifies PIN_NO_READ without PIN_EXCLUSIVE, the data
            may or may not be mapped in the return Bcb.

            If the caller specifies PIN_CALLER_TRACKS_DIRTY_DATA, all other flags
            are ignored.  The cache manager will not allocate its internal 
            structures for tracking dirty data (BCBs).  The caller is
            responsible for tracking dirty ranges, marking them dirty with
            MmSetAddressRangeModified then flushing them.  Ranges should only 
            be pinned via this manner only if the entire range will be written 
            or purged (one or the other must occur, otherwise dirty pages may
            remain in memory - see CcMapDataForOverwrite for details).

    Bcb - This returns a pointer to a Bcb parameter which must be
          supplied as input to CcPinWriteComplete.

    Buffer - Returns pointer to desired data, valid until the buffer is
             unpinned or freed.

Return Value:

    FALSE - if Wait was not set and the data was not delivered

    TRUE - if the pages are being delivered

--*/

{
    PSHARED_CACHE_MAP SharedCacheMap;
    PVOID LocalBuffer;
    LARGE_INTEGER BeyondLastByte;
    LARGE_INTEGER LocalFileOffset = *FileOffset;
    POBCB MyBcb = NULL;
    PBCB *CurrentBcbPtr = (PBCB *)&MyBcb;
    ULONG OriginalLength = Length;
    BOOLEAN Result = FALSE;

    DebugTrace(+1, me, "CcPreparePinWrite\n", 0 );

    //
    //  If PIN_CALLER_TRACKS_DIRTY_DATA is set, call CcMapDataForOverwrite to
    //  do the work because it is a special case which does not overlap with
    //  the other CcPreparePinWriteCases.
    //

    if (FlagOn( Flags, PIN_CALLER_TRACKS_DIRTY_DATA )) {

        CcMapDataForOverwrite( FileObject,
                               FileOffset, 
                               Length, 
                               Bcb,
                               Buffer );

        return TRUE;
    }

    //
    //  Get pointer to SharedCacheMap.
    //

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

    try {

        //
        //  Form loop to handle occasional overlapped Bcb case.
        //

        do {

            //
            //  If we have already been through the loop, then adjust
            //  our file offset and length from the last time.
            //

            if (MyBcb != NULL) {

                //
                //  If this is the second time through the loop, then it is time
                //  to handle the overlap case and allocate an OBCB.
                //

                if (CurrentBcbPtr == (PBCB *)&MyBcb) {

                    MyBcb = CcAllocateObcb( FileOffset, Length, (PBCB)MyBcb );

                    //
                    //  Set CurrentBcbPtr to point at the first entry in
                    //  the vector (which is already filled in), before
                    //  advancing it below.
                    //

                    CurrentBcbPtr = &MyBcb->Bcbs[0];

                    //
                    //  Also on second time through, return starting Buffer
                    //

                    *Buffer = LocalBuffer;
                }

                Length -= (ULONG)(BeyondLastByte.QuadPart - LocalFileOffset.QuadPart);
                LocalFileOffset.QuadPart = BeyondLastByte.QuadPart;
                CurrentBcbPtr += 1;
            }

            //
            //  Call local routine to Map or Access the file data.  If we cannot map
            //  the data because of a Wait condition, return FALSE.
            //

            if (!CcPinFileData( FileObject,
                                &LocalFileOffset,
                                Length,
                                FALSE,
                                TRUE,
                                Flags,
                                CurrentBcbPtr,
                                &LocalBuffer,
                                &BeyondLastByte )) {

                try_return( Result = FALSE );
            }

        //
        //  Continue looping if we did not get everything.
        //

        } while((BeyondLastByte.QuadPart - LocalFileOffset.QuadPart) < Length);

        //
        //  Debug routines used to insert and remove Bcbs from the global list
        //

        //
        //  In the normal (nonoverlapping) case we return the
        //  correct buffer address here.
        //

        if (CurrentBcbPtr == (PBCB *)&MyBcb) {
            *Buffer = LocalBuffer;
        }

        if (Zero) {
            RtlZeroMemory( *Buffer, OriginalLength );
        }

        CcSetDirtyPinnedData( MyBcb, NULL );

        //
        //  Fill in the return argument.
        //

        *Bcb = MyBcb;
        
        Result = TRUE;

    try_exit: NOTHING;
    }
    finally {

        CcMissCounter = &CcThrowAway;

        if (!Result) {

            //
            //  We may have gotten partway through
            //

            if (MyBcb != NULL) {
                CcUnpinData( MyBcb );
            }
        }

        DebugTrace(-1, me, "CcPreparePinWrite -> %02lx\n", Result );
    }

    return Result;
}


VOID
CcMapDataForOverwrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    )

/*++

Routine Description:

    This routine returns the range of the file mapped for write-only access
    (if possible, we will avoid reading the data from disk to provide valid
    memory for use, so the buffer returned should not be read).

    The difference between this routine and CcPreparePinWrite is that Cc does
    not allocate the BCB structure to track the dirty ranges of the file.  This
    responsibility is left to the caller.  The intention is that this API is
    used for log-type metadata files, where the log owner needs to separately
    track dirty ranges itself, therefore the additional tracking overhead by
    Cc is not needed. (NTFS's LFS is a prime example of this.)

    Note that although this is accessed via CcPreparePinWrite, the actual
    operations Cc needs to take internally are much close to CcMapData, 
    therefore we use a common routine that is shared with CcMapData (called
    CcMapDataCommon) to prepare the state appropriately.
    
    When CcPreparePinWrite is called with PIN_CALLER_TRACKS_DIRTY_DATA,
    THE CALLER OF THIS ROUTINE IS RESPONSIBLE FOR TRACKING THE DIRTY RANGES!
    That includes marking the buffer ranges dirty by calling 
    MmSetAddressRangeModified when the buffers are dirty and the caller wants
    the data flushed to disk when CcFlushCache is called.

    Also note that if a page is currently not resident, we will 

    The caller MUST subsequently call CcUnpinData when finished with using the
    buffer.

    The returned Buffer pointer is valid until the data is unpinned, at
    which point it is invalid to use the pointer further.

Arguments:

    FileObject - Pointer to the file object for a file which was
                 opened with NO_INTERMEDIATE_BUFFERING clear, i.e., for
                 which CcInitializeCacheMap was called by the file system.

    FileOffset - Byte offset in file for desired data.

    Length - Length of desired data in bytes.

    Bcb - On the first call this returns a pointer to a Bcb
          parameter which must be supplied as input on all subsequent
          calls, for this buffer

    Buffer - Returns pointer to desired data, valid until the buffer is
             unpinned or freed.  This pointer will remain valid if CcPinMappedData
             is called.

Return Value:

    None - this routine raises an exception if an error occurs.

--*/

{
    ULONG SavedState;
    PVOID TempBcb;
    PVOID BaseAddress;
    ULONG PageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES((ULongToPtr(FileOffset->LowPart)), Length);
    PETHREAD Thread = PsGetCurrentThread();
    PSHARED_CACHE_MAP SharedCacheMap;
    KIRQL OldIrql;
    BOOLEAN ReturnStatus;

    DebugTrace(+1, me, "CcMapDataForOverwrite\n", 0 );

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

    //
    //  If this flag isn't already set for this stream, set it now so that
    //  we know to flush the entire range in CcFlushCache since we will not
    //  have dirty hints to follow.
    //
    
    if (!FlagOn( SharedCacheMap->Flags, CALLER_TRACKS_DIRTY_DATA )) {

        CcAcquireMasterLock( &OldIrql );
        SetFlag( SharedCacheMap->Flags, CALLER_TRACKS_DIRTY_DATA );
        CcReleaseMasterLock( OldIrql );
    }

    MmSavePageFaultReadAhead( Thread, &SavedState );

    ReturnStatus = CcMapDataCommon( FileObject,
                                    FileOffset,
                                    Length,
                                    MAP_WAIT,
                                    &TempBcb,
                                    Buffer );

    ASSERTMSG( "CcMapDataCommon should never fail for a blocking caller",
               ReturnStatus == TRUE );
    
    //
    //  Now fault the data in using the zero page optimization exposed by
    //  MmCheckCachedPageState.
    //

    try {

        //
        //  Loop to touch each page
        //

        BaseAddress = *Buffer;

        while (PageCount != 0) {

            MmSetPageFaultReadAhead( Thread, PageCount - 1 );

            if (!MmCheckCachedPageState( BaseAddress, TRUE )) {

                //
                //  We don't have enough memory to produce a zero page, so make
                //  the request again and allow the data to be faulted from
                //  disk.
                //  

                MmCheckCachedPageState( BaseAddress, FALSE );
            }

            BaseAddress = (PCHAR) BaseAddress + PAGE_SIZE;
            PageCount -= 1;
        }

    } finally {

        MmResetPageFaultReadAhead( Thread, SavedState );

        if (AbnormalTermination() && (TempBcb != NULL)) {
            CcUnpinFileData( (PBCB)TempBcb, FALSE, UNPIN );
        }
    }

    //
    //  CcMapDataCommon set the appropriate miss counter, but we need to reset
    //  it at this point.
    //

    CcMissCounter = &CcThrowAway;

    //
    //  Return the BCB.  We pend this until now to avoid raising with a valid
    //  Bcb into caller's contexts.
    //

    *Bcb = TempBcb;

    DebugTrace(-1, me, "CcMapDataForOverwrite -> VOID\n", 0 );

    return;
}


VOID
CcUnpinData (
    __in PVOID Bcb
    )

/*++

Routine Description:

    This routine must be called at IPL0, some time after calling CcPinRead
    or CcPreparePinWrite.  It performs any cleanup that is necessary.

Arguments:

    Bcb - Bcb parameter returned from the last call to CcPinRead.

Return Value:

    None.

--*/

{
    DebugTrace(+1, me, "CcUnpinData:\n", 0 );
    DebugTrace( 0, me, "    >Bcb = %08lx\n", Bcb );

    //
    //  Test for ReadOnly and unpin accordingly.
    //

    if (((ULONG_PTR)Bcb & 1) != 0) {

        //
        //  Remove the Read Only flag
        //

        Bcb = (PVOID) ((ULONG_PTR)Bcb & ~1);

        CcUnpinFileData( (PBCB)Bcb, TRUE, UNPIN );

    } else {

        //
        //  Handle the overlapped Bcb case.
        //

        if (((POBCB)Bcb)->NodeTypeCode == CACHE_NTC_OBCB) {

            PBCB *BcbPtrPtr = &((POBCB)Bcb)->Bcbs[0];

            //
            //  Loop to free all Bcbs with recursive calls
            //  (rather than dealing with RO for this uncommon case).
            //

            while (*BcbPtrPtr != NULL) {
                CcUnpinData(*(BcbPtrPtr++));
            }

            //
            //  Then free the pool for the Obcb
            //

            ExFreePool( Bcb );

        //
        //  Otherwise, it is a normal Bcb
        //

        } else {
            CcUnpinFileData( (PBCB)Bcb, FALSE, UNPIN );
        }
    }

    DebugTrace(-1, me, "CcUnPinData -> VOID\n", 0 );
}


VOID
CcSetBcbOwnerPointer (
    __in PVOID Bcb,
    __in PVOID OwnerPointer
    )

/*++

Routine Description:

    This routine may be called to set the resource owner for the Bcb resource,
    for cases where another thread will do the unpin *and* the current thread
    may exit.

Arguments:

    Bcb - Bcb parameter returned from the last call to CcPinRead.

    OwnerPointer - A valid resource owner pointer, which means a pointer to
                   an allocated system address, with the low-order two bits
                   set.  The address may not be deallocated until after the
                   unpin call.

Return Value:

    None.

--*/

{
    ASSERT(((ULONG_PTR)Bcb & 1) == 0);

    //
    //  Handle the overlapped Bcb case.
    //

    if (((POBCB)Bcb)->NodeTypeCode == CACHE_NTC_OBCB) {

        PBCB *BcbPtrPtr = &((POBCB)Bcb)->Bcbs[0];

        //
        //  Loop to set owner for all Bcbs.
        //

        while (*BcbPtrPtr != NULL) {
            ExSetResourceOwnerPointer( &(*BcbPtrPtr)->Resource, OwnerPointer );
            BcbPtrPtr++;
        }

    //
    //  Otherwise, it is a normal Bcb
    //

    } else {

        //
        //  Handle normal case.
        //

        ExSetResourceOwnerPointer( &((PBCB)Bcb)->Resource, OwnerPointer );
    }
}


VOID
CcUnpinDataForThread (
    __in PVOID Bcb,
    __in ERESOURCE_THREAD ResourceThreadId
    )

/*++

Routine Description:

    This routine must be called at IPL0, some time after calling CcPinRead
    or CcPreparePinWrite.  It performs any cleanup that is necessary,
    releasing the Bcb resource for the given thread.

Arguments:

    Bcb - Bcb parameter returned from the last call to CcPinRead.

Return Value:

    None.

--*/

{
    DebugTrace(+1, me, "CcUnpinDataForThread:\n", 0 );
    DebugTrace( 0, me, "    >Bcb = %08lx\n", Bcb );
    DebugTrace( 0, me, "    >ResoureceThreadId = %08lx\n", ResoureceThreadId );

    //
    //  Test for ReadOnly and unpin accordingly.
    //

    if (((ULONG_PTR)Bcb & 1) != 0) {

        //
        //  Remove the Read Only flag
        //

        Bcb = (PVOID) ((ULONG_PTR)Bcb & ~1);

        CcUnpinFileData( (PBCB)Bcb, TRUE, UNPIN );

    } else {

        //
        //  Handle the overlapped Bcb case.
        //

        if (((POBCB)Bcb)->NodeTypeCode == CACHE_NTC_OBCB) {

            PBCB *BcbPtrPtr = &((POBCB)Bcb)->Bcbs[0];

            //
            //  Loop to free all Bcbs with recursive calls
            //  (rather than dealing with RO for this uncommon case).
            //

            while (*BcbPtrPtr != NULL) {
                CcUnpinDataForThread( *(BcbPtrPtr++), ResourceThreadId );
            }

            //
            //  Then free the pool for the Obcb
            //

            ExFreePool( Bcb );

        //
        //  Otherwise, it is a normal Bcb
        //

        } else {

            //
            //  If not readonly, we can release the resource for the thread first,
            //  and then call CcUnpinFileData.  Release resource first in case
            //  Bcb gets deallocated.
            //

            ExReleaseResourceForThreadLite( &((PBCB)Bcb)->Resource, ResourceThreadId );
            CcUnpinFileData( (PBCB)Bcb, TRUE, UNPIN );
        }
    }
    DebugTrace(-1, me, "CcUnpinDataForThread -> VOID\n", 0 );
}


POBCB
CcAllocateObcb (
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN PBCB FirstBcb
    )

/*++

Routine Description:

    This routine is called by the various pinning routines to allocate and
    initialize an overlap Bcb.

Arguments:

    FileOffset - Starting file offset for the Obcb (An Obcb starts with a
                 public structure, which someone could use)

    Length - Length of the range covered by the Obcb

    FirstBcb - First Bcb already created, which only covers the start of
               the desired range (low order bit may be set to indicate ReadOnly)

Return Value:

    Pointer to the allocated Obcb

--*/

{
    ULONG LengthToAllocate;
    POBCB Obcb;
    PBCB Bcb = (PBCB)((ULONG_PTR)FirstBcb & ~1);

    //
    //  Allocate according to the worst case, assuming that we
    //  will need as many additional Bcbs as there are pages
    //  remaining. Also throw in one more pointer to guarantee
    //  users of the OBCB can always terminate on NULL.
    //
    //  We remove fron consideration the range described by the
    //  first Bcb (note that the range of the Obcb is not strictly
    //  starting at the first Bcb) and add in locations for the first
    //  bcb and the null.
    //

    LengthToAllocate = FIELD_OFFSET(OBCB, Bcbs) + (2 * sizeof(PBCB)) +
                       ((Length -
                         (Bcb->ByteLength -
                          (FileOffset->HighPart?
                           (ULONG)(FileOffset->QuadPart - Bcb->FileOffset.QuadPart) :
                           FileOffset->LowPart - Bcb->FileOffset.LowPart)) +
                         PAGE_SIZE - 1) / PAGE_SIZE) * sizeof(PBCB);

    Obcb = ExAllocatePoolWithTag( NonPagedPool, LengthToAllocate, 'bOcC' );
    if (Obcb == NULL) {
        ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
    }

    RtlZeroMemory( Obcb, LengthToAllocate );
    Obcb->NodeTypeCode = CACHE_NTC_OBCB;
    Obcb->NodeByteSize = (USHORT)LengthToAllocate;
    Obcb->ByteLength = Length;
    Obcb->FileOffset = *FileOffset;
    Obcb->Bcbs[0] = FirstBcb;

    return Obcb;
}
