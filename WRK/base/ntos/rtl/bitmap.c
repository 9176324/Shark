/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    BitMap.c

Abstract:

    Implementation of the bit map routines for the NT rtl.

    Bit numbers within the bit map are zero based.  The first is numbered
    zero.

    The bit map routines keep track of the number of bits clear or set by
    subtracting or adding the number of bits operated on as bit ranges
    are cleared or set; individual bit states are not tested.
    This means that if a range of bits is set,
    it is assumed that the total range is currently clear.

--*/

#include "ntrtlp.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,RtlInitializeBitMap)
#endif


#if DBG
VOID
DumpBitMap (
    PRTL_BITMAP BitMap
    )
{
    ULONG i;
    BOOLEAN AllZeros, AllOnes;

    DbgPrint(" BitMap:%08lx", BitMap);

    DbgPrint(" (%08x)", BitMap->SizeOfBitMap);
    DbgPrint(" %08lx\n", BitMap->Buffer);

    AllZeros = FALSE;
    AllOnes = FALSE;

    for (i = 0; i < ((BitMap->SizeOfBitMap + 31) / 32); i += 1) {

        if (BitMap->Buffer[i] == 0) {

            if (AllZeros) {

                NOTHING;

            } else {

                DbgPrint("%4d:", i);
                DbgPrint(" %08lx\n", BitMap->Buffer[i]);
            }

            AllZeros = TRUE;
            AllOnes = FALSE;

        } else if (BitMap->Buffer[i] == 0xFFFFFFFF) {

            if (AllOnes) {

                NOTHING;

            } else {

                DbgPrint("%4d:", i);
                DbgPrint(" %08lx\n", BitMap->Buffer[i]);
            }

            AllZeros = FALSE;
            AllOnes = TRUE;

        } else {

            AllZeros = FALSE;
            AllOnes = FALSE;

            DbgPrint("%4d:", i);
            DbgPrint(" %08lx\n", BitMap->Buffer[i]);
        }
    }
}
#endif


//
//  There are three macros to make reading the bytes in a bitmap easier.
//

#define GET_BYTE_DECLARATIONS() \
    PUCHAR _CURRENT_POSITION;

#define GET_BYTE_INITIALIZATION(RTL_BITMAP,BYTE_INDEX) {               \
    _CURRENT_POSITION = &((PUCHAR)((RTL_BITMAP)->Buffer))[BYTE_INDEX]; \
}

#define GET_BYTE(THIS_BYTE)  (         \
    THIS_BYTE = *(_CURRENT_POSITION++) \
)


//
//  Lookup table that tells how many contiguous bits are clear (i.e., 0) in
//  a byte
//

CONST CCHAR RtlpBitsClearAnywhere[] =
         { 8,7,6,6,5,5,5,5,4,4,4,4,4,4,4,4,
           4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
           5,4,3,3,2,2,2,2,3,2,2,2,2,2,2,2,
           4,3,2,2,2,2,2,2,3,2,2,2,2,2,2,2,
           6,5,4,4,3,3,3,3,3,2,2,2,2,2,2,2,
           4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,1,
           5,4,3,3,2,2,2,2,3,2,1,1,2,1,1,1,
           4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,1,
           7,6,5,5,4,4,4,4,3,3,3,3,3,3,3,3,
           4,3,2,2,2,2,2,2,3,2,2,2,2,2,2,2,
           5,4,3,3,2,2,2,2,3,2,1,1,2,1,1,1,
           4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,1,
           6,5,4,4,3,3,3,3,3,2,2,2,2,2,2,2,
           4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,1,
           5,4,3,3,2,2,2,2,3,2,1,1,2,1,1,1,
           4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,0 };

//
//  Lookup table that tells how many contiguous LOW order bits are clear
//  (i.e., 0) in a byte
//

CONST CCHAR RtlpBitsClearLow[] =
          { 8,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            7,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
            4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0 };

//
//  Lookup table that tells how many contiguous HIGH order bits are clear
//  (i.e., 0) in a byte
//

CONST CCHAR RtlpBitsClearHigh[] =
          { 8,7,6,6,5,5,5,5,4,4,4,4,4,4,4,4,
            3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
            2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
            2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

//
//  Lookup table that tells how many clear bits (i.e., 0) there are in a byte
//

CONST CCHAR RtlpBitsClearTotal[] =
          { 8,7,7,6,7,6,6,5,7,6,6,5,6,5,5,4,
            7,6,6,5,6,5,5,4,6,5,5,4,5,4,4,3,
            7,6,6,5,6,5,5,4,6,5,5,4,5,4,4,3,
            6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2,
            7,6,6,5,6,5,5,4,6,5,5,4,5,4,4,3,
            6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2,
            6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2,
            5,4,4,3,4,3,3,2,4,3,3,2,3,2,2,1,
            7,6,6,5,6,5,5,4,6,5,5,4,5,4,4,3,
            6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2,
            6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2,
            5,4,4,3,4,3,3,2,4,3,3,2,3,2,2,1,
            6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2,
            5,4,4,3,4,3,3,2,4,3,3,2,3,2,2,1,
            5,4,4,3,4,3,3,2,4,3,3,2,3,2,2,1,
            4,3,3,2,3,2,2,1,3,2,2,1,2,1,1,0 };

//
//  Bit Mask for clearing and setting bits within bytes.  FillMask[i] has the first
//  i bits set to 1.  ZeroMask[i] has the first i bits set to zero.
//

static CONST UCHAR FillMask[] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };

static CONST UCHAR ZeroMask[] = { 0xFF, 0xFE, 0xFC, 0xF8, 0xf0, 0xe0, 0xc0, 0x80, 0x00 };


VOID
RtlInitializeBitMap (
    IN PRTL_BITMAP BitMapHeader,
    IN PULONG BitMapBuffer,
    IN ULONG SizeOfBitMap
    )

/*++

Routine Description:

    This procedure initializes a bit map.

Arguments:

    BitMapHeader - Supplies a pointer to the BitMap Header to initialize

    BitMapBuffer - Supplies a pointer to the buffer that is to serve as the
        BitMap.  This must be an a multiple number of longwords in size.

    SizeOfBitMap - Supplies the number of bits required in the Bit Map.

Return Value:

    None.

--*/

{
    RTL_PAGED_CODE();

    //
    //  Initialize the BitMap header.
    //

    BitMapHeader->SizeOfBitMap = SizeOfBitMap;
    BitMapHeader->Buffer = BitMapBuffer;

    //
    //  And return to our caller
    //

    return;
}

VOID
RtlClearBit (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG BitNumber
    )

/*++

Routine Description:

    This procedure clears a single bit in the specified bit map.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized bit map.

    BitNumber - Supplies the number of the bit to be cleared in the bit map.

Return Value:

    None.

--*/

{

#if defined(_AMD64_)

    ASSERT(BitNumber < BitMapHeader->SizeOfBitMap);

    BitTestAndReset((PLONG)BitMapHeader->Buffer, BitNumber);

#else

    PCHAR ByteAddress;
    ULONG ShiftCount;

    ASSERT(BitNumber < BitMapHeader->SizeOfBitMap);

    ByteAddress = (PCHAR)BitMapHeader->Buffer + (BitNumber >> 3);
    ShiftCount = BitNumber & 0x7;
    *ByteAddress &= (CHAR)(~(1 << ShiftCount));

#endif

    return;
}

VOID
RtlSetBit (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG BitNumber
    )

/*++

Routine Description:

    This procedure sets a single bit in the specified bit map.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized bit map.

    BitNumber - Supplies the number of the bit to be set in the bit map.

Return Value:

    None.

--*/

{

#if defined(_AMD64_)

    ASSERT(BitNumber < BitMapHeader->SizeOfBitMap);

    BitTestAndSet((PLONG)BitMapHeader->Buffer, BitNumber);

#else

    PCHAR ByteAddress;
    ULONG ShiftCount;

    ASSERT(BitNumber < BitMapHeader->SizeOfBitMap);

    ByteAddress = (PCHAR)BitMapHeader->Buffer + (BitNumber >> 3);
    ShiftCount = BitNumber & 0x7;
    *ByteAddress |= (CHAR)(1 << ShiftCount);

#endif

    return;
}

BOOLEAN
RtlTestBit (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG BitNumber
    )

/*++

Routine Description:

    This procedure tests the state of a single bit in the specified bit map.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized bit map.

    BitNumber - Supplies the number of the bit to be tested in the bit map.

Return Value:

    The state of the specified bit is returned as the function value.

--*/

{

#if defined(_AMD64_)

    ASSERT(BitNumber < BitMapHeader->SizeOfBitMap);

    return BitTest((PLONG)BitMapHeader->Buffer, BitNumber);

#else

    PCHAR ByteAddress;
    ULONG ShiftCount;

    ASSERT(BitNumber < BitMapHeader->SizeOfBitMap);

    ByteAddress = (PCHAR)BitMapHeader->Buffer + (BitNumber >> 3);
    ShiftCount = BitNumber & 0x7;
    return (BOOLEAN)((*ByteAddress >> ShiftCount) & 1);

#endif

}

VOID
RtlClearAllBits (
    IN PRTL_BITMAP BitMapHeader
    )

/*++

Routine Description:

    This procedure clears all bits in the specified Bit Map.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized BitMap

Return Value:

    None.

--*/

{
    //
    //  Clear all the bits
    //

    RtlZeroMemory( BitMapHeader->Buffer,
                   ((BitMapHeader->SizeOfBitMap + 31) / 32) * 4
                 );

    //
    //  And return to our caller
    //

    return;
}


VOID
RtlSetAllBits (
    IN PRTL_BITMAP BitMapHeader
    )

/*++

Routine Description:

    This procedure sets all bits in the specified Bit Map.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized BitMap

Return Value:

    None.

--*/

{
    //
    //  Set all the bits
    //

    RtlFillMemoryUlong( BitMapHeader->Buffer,
                        ((BitMapHeader->SizeOfBitMap + 31) / 32) * 4,
                        0xffffffff
                      );

    //
    //  And return to our caller
    //

    return;
}

typedef ULONG_PTR      CHUNK, *PCHUNK;
#define CHUNK_BYTES    (sizeof(CHUNK))
#define CHUNK_BITS     (CHUNK_BYTES * 8)
#define CHUNK_CLEAR    ((CHUNK)0)
#define CHUNK_SET      ((CHUNK)-1)
#define CHUNK_HIGH_BIT ((CHUNK)1 << (CHUNK_BITS - 1))

#define RTL_INVALID_INDEX ((ULONG)-1)

#if defined(_WIN64)
#define BitScanForwardChunk BitScanForward64
#define BitScanReverseChunk BitScanReverse64
#else
#define BitScanForwardChunk _BitScanForward
#define BitScanReverseChunk _BitScanReverse
#endif


LOGICAL
FORCEINLINE
RtlpFindClearRunInChunk (
    IN CHUNK Chunk,
    IN ULONG RunLength,
    OUT ULONG *StartBit
    )

/*++

Routine Description:

    This routine determines whether a run of clear bits of length RunLength
    exists in a chunk, and if so returns the starting index of the first
    such run.

Arguments:

    Chunk - The chunk to examine.

    RunLength - The length of the desired run.

    StartBit - Supplies a pointer to the value that will contain the bit index
               of the first suitable run found within Chunk.
                                        
Return Value:

    TRUE - A suitable run was found, and *StartBit containts the bit index
           of that run

    FALSE - A suitable run was not found.

--*/

{
    CHUNK chunkMask;
    ULONG shift;
    ULONG shiftsRemaining;

    ASSERT(RunLength < CHUNK_BITS);
    ASSERT(RunLength > 1);

    //
    // This algorithm works by inverting the bits, then masking and shifting
    // (RunLength-1) times.  The lowest bit set, if any, marks the start
    // of the run.
    //
    // For example, given
    //
    // Chunk = 0y11000000011111000000000001111000
    // RunLength = 11
    //
    //
    // 0y00111111100000111111111110000111     shift = 5
    // 0y00000001100000000001111110000100     shift = 3
    // 0y00000000000000000000001110000000     shift = 1
    // 0y00000000000000000000000110000000     shift = 1
    // 0y00000000000000000000000010000000
    // 

    chunkMask = ~Chunk;
    shiftsRemaining = RunLength;

    do {
        shift = shiftsRemaining / 2;
        chunkMask &= chunkMask >> shift;
        if (chunkMask == 0) {
            return FALSE;
        }
        shiftsRemaining -= shift;
    } while (shiftsRemaining > 1);

    BitScanForwardChunk(StartBit,chunkMask);
    return TRUE;
}


ULONG
FORCEINLINE
RtlpClearMSBInChunk (
    IN CHUNK Chunk
    )

/*++

Routine Description:

    This routine calculates the number of consecutive, clear, most significant
    bits in Chunk.

Arguments:

    The chunk to examine.

Return Value:

    The number of consecutive, most significant, clear bits in Chunk.

--*/

{
    ULONG Index;

    if (BitScanReverseChunk(&Index, Chunk) == FALSE) {
        Index = CHUNK_BITS;
    } else {
        Index = CHUNK_BITS - Index - 1;
    }
    return Index;
}


ULONG
FORCEINLINE
RtlpClearLSBInChunk (
    IN CHUNK Chunk
    )

/*++

Routine Description:

    This routine calculates the number of consecutive, clear, least
    significant bits in Chunk.

Arguments:

    The chunk to examine.

Return Value:

    The number of consecutive, most significant, clear bits in Chunk.

--*/

{
    ULONG Index;

    if (BitScanForwardChunk(&Index, Chunk) == FALSE) {
        Index = CHUNK_BITS;
    }
    return Index;
}


ULONG
FORCEINLINE
RtlpFindClearBitsRange (
    IN PULONG BitMap,
    IN ULONG NumberToFind,
    IN ULONG RangeStart,
    IN ULONG RangeEnd,
    IN LOGICAL Invert
    )

/*++

Routine Description:

    This procedure searches the specified bit map for the specified
    contiguous region of clear bits.  If a run is not found from the
    hint to the end of the bitmap, we will search again from the
    beginning of the bitmap.

Arguments:

    BitMap - Supplies a pointer to a ULONG arrayto the previously initialized BitMap.

    NumberToFind - Supplies the size of the contiguous region to find.

    RangeStart - Supplies the index (zero based) of where we should start
        the search from within the bitmap.

    RangeEnd - Supplies the index of the last possible bit to include in the
        range.

    Invert - Evaluated at compile time, if TRUE changes this routine to
        RtlpFindSetBitsRange.

Return Value:

    ULONG - Receives the starting index (zero based) of the contiguous
        region of clear bits found.  If not such a region cannot be found
        a -1 (i.e. 0xffffffff) is returned.

--*/

{
    ULONG  bitsRemaining;
    CHUNK  chunk;
    PCHUNK chunkArray;
    PCHUNK chunkPtr;
    PCHUNK clearChunkRunEnd;
    ULONG  clearChunksRequired;
    CHUNK  firstChunk;
    CHUNK  headChunkMask;
    ULONG  headClearBits;
    PCHUNK lastPossibleClearChunk;
    ULONG  lastPossibleStartBit;
    PCHUNK lastPossibleStartChunk;
    ULONG  rangeEnd;
    PCHUNK rangeStartChunk;
    PCHUNK rangeEndChunk;
    ULONG  rangeStart;
    ULONG  runStartBit;
    ULONG  tailClearBits;

    #define READ_CHUNK(n) (Invert ? ~(*(chunkPtr+(n))) : (*(chunkPtr+(n))))

    rangeStart = RangeStart;
    rangeEnd = RangeEnd;
    chunkArray = (PCHUNK)BitMap;

    if ((rangeEnd - rangeStart + 1) < NumberToFind) {
        return RTL_INVALID_INDEX;
    }

    //
    // Calculate the address of the first chunk that could contain the
    // start of a suitable run.
    // 

    rangeStartChunk = chunkArray + (rangeStart / CHUNK_BITS);

    //
    // Calculate the last possible bit and chunk at which the run must
    // begin in order to fit in the given range.
    // 

    lastPossibleStartBit = rangeEnd - NumberToFind + 1;
    lastPossibleStartChunk = chunkArray + lastPossibleStartBit / CHUNK_BITS;

    //
    // Calculate a mask of bits to apply to the first chunk.  This is used
    // to mask off bits in the first chunk that fall outside of the range.
    // 

    headChunkMask = ((CHUNK)1 << (rangeStart % CHUNK_BITS)) - 1;

    //
    // Retrieve the first chunk and mask off inelligible bits.
    // 

    chunkPtr = rangeStartChunk;
    firstChunk = READ_CHUNK(0) | headChunkMask;

    //
    // Determine which of four search algorithms to use based on the desired
    // run length.
    // 

    if (NumberToFind > (2 * CHUNK_BITS - 1)) {

        //
        // The desired run length is such that the run must include at least
        // one clear chunk.
        //
        // Search strategy:
        //
        // 1) Scan forward looking for a clear chunk.
        //    If end of range reached, search failed.
        //
        // 2) Add in number of head bits (tail clear bits in previous chunk)
        //
        // 3) Ensure remaining number of clear chunks required exist
        //    If not, go to 1
        //
        // 4) Finally check that any tail bits are clear in the last chunk
        //    If not, go to 1
        // 

        //
        // First determine the last possible clear chunk in the array.
        // If a clear chunk is not located by this point, then a suitable
        // run does not exist.
        // 

        lastPossibleClearChunk = lastPossibleStartChunk;
        if ((lastPossibleStartBit % CHUNK_BITS) != 0) {
            lastPossibleClearChunk += 1;
        }

        //
        // If the first chunk is clear, set headClearBits to zero and branch
        // into the loop, skipping the code that checks for clear bits in
        // the previous chunk.
        // 

        if (firstChunk == CHUNK_CLEAR) {
            headClearBits = 0;
            goto largeLoopEntry;
        }
        chunkPtr += 1;

        //
        // If the second chunk is clear, set headClearBits according to
        // firstChunk (which contains the masked value of the first chunk).
        //

        if (READ_CHUNK(0) == CHUNK_CLEAR) {
            headClearBits = RtlpClearMSBInChunk(firstChunk);
            goto largeLoopEntry;
        }

        //
        // Scan forward looking for an all-zero chunk
        //

        while (TRUE) {

findRunStartLarge:

            if (chunkPtr > lastPossibleClearChunk) {
                return RTL_INVALID_INDEX;
            }

            chunkPtr += 1;

            if (READ_CHUNK(0) == CHUNK_CLEAR) {
                break;
            }
        }

        //
        // An all-zero chunk has been located, record this as the
        // potential start of a clear run, backing up by
        // the number of clear tail bits in the previous chunk.
        //

        headClearBits = RtlpClearMSBInChunk(READ_CHUNK(-1));

largeLoopEntry:

        runStartBit = (ULONG)((chunkPtr - chunkArray) * CHUNK_BITS);
        runStartBit -= headClearBits;

        if (runStartBit > lastPossibleStartBit) {
            return RTL_INVALID_INDEX;
        }

        //
        // Calculate how many clear chunks are needed in this potential
        // run.
        // 

        clearChunksRequired =
            (NumberToFind - headClearBits) / CHUNK_BITS;

        clearChunkRunEnd = chunkPtr + clearChunksRequired;

        //
        // We know we've got at least one chunk's worth of clear bits.
        // Make sure any necessary remaining chunks are zero as well.
        //

        chunkPtr += 1;
        while (chunkPtr != clearChunkRunEnd) {

            if (READ_CHUNK(0) != CHUNK_CLEAR) {

                //
                // This run isn't long enough.  Look for the start of
                // another run.
                //

                goto findRunStartLarge;
            }
            chunkPtr += 1;
        }

        //
        // Finally, calculate the needed number of bits in the tail and make
        // sure they are clear as well.
        // 

        tailClearBits = (NumberToFind - headClearBits) % CHUNK_BITS;
        if (tailClearBits != 0) {

            if (RtlpClearLSBInChunk(READ_CHUNK(0)) < tailClearBits) {

                //
                // Not enough bits in the last partial chunk.
                //

                goto findRunStartLarge;
            }
        }

        //
        // A suitable run has been found.  runStartBit is the index of the
        // first bit in the run, relative to chunkArray.
        // 

    } else if (NumberToFind >= CHUNK_BITS) {

        //
        // The sought run does not necessarily include a clear chunk.  It may
        // consume one chunk, or may span 2 or 3 chunks.
        //
        // Again, special case the first chunk as it has been masked.
        //
        // Search strategy:
        //
        // 1) Scan forward looking for a chunk with 1 or more clear tail bits
        //    If end of range reached, search failed.
        //
        // 2) Check for any additional necessary clear bits.
        //    If not found, go to 1
        //

        chunk = firstChunk;
        while (TRUE) {

            //
            // Look for a chunk that has at least one clear tail bit.
            //

            while (TRUE) {

                if ((chunk & CHUNK_HIGH_BIT) == 0) {
                    break;
                }
    
                chunkPtr += 1;

                if (chunkPtr > lastPossibleStartChunk) {
                    return RTL_INVALID_INDEX;
                }

                chunk = READ_CHUNK(0);
            }

            //
            // This chunk has some bits that can be used as the head portion
            // of the run.  Record the start of this potentially suitable
            // bit run.
            //
    
            headClearBits = RtlpClearMSBInChunk(chunk);
            runStartBit = (ULONG)((chunkPtr - chunkArray) * CHUNK_BITS);
            runStartBit += CHUNK_BITS - headClearBits;
            if (runStartBit > lastPossibleStartBit) {
                return RTL_INVALID_INDEX;
            }
    
            //
            // If bitsRemaining indicate that a clear chunk is needed next,
            // check for that.
            // 

            ASSERT(NumberToFind >= headClearBits);

            bitsRemaining = NumberToFind - headClearBits;
            if (bitsRemaining == 0) {
                break;
            }

            chunkPtr += 1;
            chunk = READ_CHUNK(0);

            if (bitsRemaining >= CHUNK_BITS) {

                if (chunk != CHUNK_CLEAR) {
                    continue;
                }

                bitsRemaining -= CHUNK_BITS;
                if (bitsRemaining == 0) {
                    break;
                }

                chunkPtr += 1;
                chunk = READ_CHUNK(0);
            }

            tailClearBits = RtlpClearLSBInChunk(chunk);
            if (tailClearBits >= bitsRemaining) {
                break;
            }
        }

    } else if (NumberToFind > 1) {

        //
        // NumberToFind is < CHUNK_BITS, and so could span two chunks or
        // lie anywhere within a chunk.
        //
        // Search strategy:
        //
        // 1) Scan forward looking for a chunk != CHUNK_SET.
        //    If end of range reached, search failed.
        //
        // 2) If the current chunk contains clear head bits, determine
        //    whether those combined with any clear tail bits from the
        //    previous chunk result in a suitable run.
        //    If so, search succeeds.
        //
        // 3) Search within the chunk for the first suitable run of clear
        //    bits.  If found, search succeeds.
        //
        // 4) Record the number of clear tail bits in the chunk, go to 1.
        //

        //
        // We'll use tailClearBits to track the clear MSB bits in the
        // last chunk we've examined.
        //

        tailClearBits = 0;
        rangeEndChunk = chunkArray + (rangeEnd / CHUNK_BITS);

        //
        // As always, start with a masked initial chunk.
        //

        chunk = firstChunk;
        while (TRUE) {

            //
            // Look for a chunk that has some clear bits.
            //

            if (chunk == CHUNK_SET) {
                do {
                    chunkPtr += 1;
                    if (chunkPtr > lastPossibleStartChunk) {
                        return RTL_INVALID_INDEX;
                    }
                    chunk = READ_CHUNK(0);
                } while (chunk == CHUNK_SET);

                tailClearBits = 0;
            }

            headClearBits = RtlpClearLSBInChunk(chunk);
            if ((headClearBits + tailClearBits) >= NumberToFind) {
    
                runStartBit = 0-tailClearBits;
    
            } else {
    
                //
                // The chunk does not start with a run of clear bits,
                // determine whether a suitable run is embedded within the
                // chunk.
                //
    
                if (RtlpFindClearRunInChunk(chunk,
                                            NumberToFind,
                                            &runStartBit) == FALSE) {
    
                    //
                    // The chunk does not contain a suitable run of clear
                    // bits.  If this is the last chunk in the range,
                    // then a suitable run does not exist.
                    //

                    if (chunkPtr == rangeEndChunk) {
                        return RTL_INVALID_INDEX;
                    }

                    //
                    // Record the number of clear tail bits in this chunk.
                    // 

                    tailClearBits = RtlpClearMSBInChunk(chunk);

                    chunkPtr += 1;
                    chunk = READ_CHUNK(0);
                    continue;
                }
            }

            runStartBit += (ULONG)((chunkPtr - chunkArray) * CHUNK_BITS);

            if (runStartBit > lastPossibleStartBit) {
                return RTL_INVALID_INDEX;
            } else {
                break;
            }
        }

    } else {

        //
        // NumberToFind == 1.  Just find a chunk that isn't all ones,
        // then find the first clear bit in the chunk.
        //

        chunk = firstChunk;

        while (chunk == CHUNK_SET) {
            chunkPtr += 1;
            if (chunkPtr > lastPossibleStartChunk) {
                return RTL_INVALID_INDEX;
            }
            chunk = READ_CHUNK(0);
        }

        BitScanForwardChunk(&runStartBit,~chunk);
        runStartBit += (ULONG)((chunkPtr - chunkArray) * CHUNK_BITS);
        if (runStartBit > lastPossibleStartBit) {
            return RTL_INVALID_INDEX;
        }
    }

    //
    // At this point, a suitable run has been found.  runStartBit represents
    // the bit offset of the start of the run, relative to chunkArray.
    //

    return runStartBit;
}



ULONG
RtlFindClearBits (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG NumberToFind,
    IN ULONG HintIndex
    )

/*++

Routine Description:

    This procedure searches the specified bit map for the specified
    contiguous region of clear bits.  If a run is not found from the
    hint to the end of the bitmap, we will search again from the
    beginning of the bitmap.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized BitMap.

    NumberToFind - Supplies the size of the contiguous region to find.

    HintIndex - Supplies the index (zero based) of where we should start
        the search from within the bitmap.

Return Value:

    ULONG - Receives the starting index (zero based) of the contiguous
        region of clear bits found.  If not such a region cannot be found
        a -1 (i.e. 0xffffffff) is returned.

--*/

{
    ULONG BitIndex;
    ULONG SizeOfBitMap;
    ULONG RangeEnd;
    ULONG RangeStart;
    PULONG Buffer;
    ULONG BufferAdjust;
    ULONG BufferAdjustBits;

    SizeOfBitMap = BitMapHeader->SizeOfBitMap;
    if (HintIndex >= SizeOfBitMap) {
        RangeStart = 0;
    } else {
        RangeStart = HintIndex;
    }
    RangeEnd = SizeOfBitMap - 1;
    Buffer = BitMapHeader->Buffer;

    //
    // Special case zero-length bitmap routines, to return the same value
    // as the old bitmap routines would.
    // 

    if (NumberToFind == 0) {
        return RangeStart & ~7;
    }

#if defined(_WIN64)

    //
    // RTL_BITMAP contains a buffer that is merely DWORD aligned.  On 64-bit
    // platforms a QWORD aligned buffer is required.  If necessary, align
    // the buffer pointer down and adjust the range bit offsets accordingly.
    //

    if (((ULONG_PTR)Buffer & 0x4) != 0) {
        BufferAdjust = 1;
        BufferAdjustBits = 32;
    } else

#endif
    {
        BufferAdjust = 0;
        BufferAdjustBits = 0;
    }

    while (TRUE) {

        BitIndex = RtlpFindClearBitsRange(Buffer - BufferAdjust,
                                          NumberToFind,
                                          RangeStart + BufferAdjustBits,
                                          RangeEnd + BufferAdjustBits,
                                          FALSE);

        if ((BitIndex != RTL_INVALID_INDEX) || (RangeStart == 0)) {
            break;
        }

        RangeEnd = HintIndex + NumberToFind;
        if (RangeEnd > SizeOfBitMap) {
            RangeEnd = SizeOfBitMap;
        }
        RangeEnd -= 1;
        RangeStart = 0;
    }

#if defined(_WIN64)

    //
    // Compensate for any buffer alignment performed above.
    //

    if (BitIndex != RTL_INVALID_INDEX) {
        BitIndex -= BufferAdjustBits;
    }

#endif

    return BitIndex;
}



ULONG
RtlFindSetBits (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG NumberToFind,
    IN ULONG HintIndex
    )

/*++

Routine Description:

    This procedure searches the specified bit map for the specified
    contiguous region of set bits.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized BitMap.

    NumberToFind - Supplies the size of the contiguous region to find.

    HintIndex - Supplies the index (zero based) of where we should start
        the search from within the bitmap.

Return Value:

    ULONG - Receives the starting index (zero based) of the contiguous
        region of set bits found.  If such a region cannot be found then
        a -1 (i.e., 0xffffffff) is returned.

--*/

{
    ULONG BitIndex;
    ULONG SizeOfBitMap;
    ULONG RangeEnd;
    ULONG RangeStart;
    PULONG Buffer;
    ULONG BufferAdjust;
    ULONG BufferAdjustBits;

    SizeOfBitMap = BitMapHeader->SizeOfBitMap;
    if (HintIndex >= SizeOfBitMap) {
        RangeStart = 0;
    } else {
        RangeStart = HintIndex;
    }
    RangeEnd = SizeOfBitMap - 1;
    Buffer = BitMapHeader->Buffer;

    //
    // Special case zero-length bitmap routines, to return the same value
    // as the old bitmap routines would.
    // 

    if (NumberToFind == 0) {
        return RangeStart & ~7;
    }

#if defined(_WIN64)

    //
    // RTL_BITMAP contains a buffer that is merely DWORD aligned.  On 64-bit
    // platforms a QWORD aligned buffer is required.  If necessary, align
    // the buffer pointer down and adjust the range bit offsets accordingly.
    //

    if (((ULONG_PTR)Buffer & 0x4) != 0) {
        BufferAdjust = 1;
        BufferAdjustBits = 32;
    } else

#endif
    {
        BufferAdjust = 0;
        BufferAdjustBits = 0;
    }

    while (TRUE) {

        BitIndex = RtlpFindClearBitsRange(Buffer - BufferAdjust,
                                          NumberToFind,
                                          RangeStart + BufferAdjustBits,
                                          RangeEnd + BufferAdjustBits,
                                          TRUE);

        if ((BitIndex != RTL_INVALID_INDEX) || (RangeStart == 0)) {
            break;
        }

        RangeEnd = HintIndex + NumberToFind;
        if (RangeEnd > SizeOfBitMap) {
            RangeEnd = SizeOfBitMap;
        }
        RangeEnd -= 1;
        RangeStart = 0;
    }

#if defined(_WIN64)

    //
    // Compensate for any buffer alignment performed above.
    //

    if (BitIndex != RTL_INVALID_INDEX) {
        BitIndex -= BufferAdjustBits;
    }

#endif

    return BitIndex;
}


ULONG
RtlFindClearBitsAndSet (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG NumberToFind,
    IN ULONG HintIndex
    )

/*++

Routine Description:

    This procedure searches the specified bit map for the specified
    contiguous region of clear bits, sets the bits and returns the
    number of bits found, and the starting bit number which was clear
    then set.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized BitMap.

    NumberToFind - Supplies the size of the contiguous region to find.

    HintIndex - Supplies the index (zero based) of where we should start
        the search from within the bitmap.

Return Value:

    ULONG - Receives the starting index (zero based) of the contiguous
        region found.  If such a region cannot be located a -1 (i.e.,
        0xffffffff) is returned.

--*/

{
    ULONG StartingIndex;

    //
    //  First look for a run of clear bits that equals the size requested
    //

    StartingIndex = RtlFindClearBits( BitMapHeader,
                                      NumberToFind,
                                      HintIndex );

    if (StartingIndex != 0xffffffff) {

        //
        //  We found a large enough run of clear bits so now set them
        //

        RtlSetBits( BitMapHeader, StartingIndex, NumberToFind );
    }

    //
    //  And return to our caller
    //

    return StartingIndex;

}


ULONG
RtlFindSetBitsAndClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG NumberToFind,
    IN ULONG HintIndex
    )

/*++

Routine Description:

    This procedure searches the specified bit map for the specified
    contiguous region of set bits, clears the bits and returns the
    number of bits found and the starting bit number which was set then
    clear.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized BitMap.

    NumberToFind - Supplies the size of the contiguous region to find.

    HintIndex - Supplies the index (zero based) of where we should start
        the search from within the bitmap.

Return Value:

    ULONG - Receives the starting index (zero based) of the contiguous
        region found.  If such a region cannot be located a -1 (i.e.,
        0xffffffff) is returned.


--*/

{
    ULONG StartingIndex;

    //
    //  First look for a run of set bits that equals the size requested
    //

    if ((StartingIndex = RtlFindSetBits( BitMapHeader,
                                         NumberToFind,
                                         HintIndex )) != 0xffffffff) {

        //
        //  We found a large enough run of set bits so now clear them
        //

        RtlClearBits( BitMapHeader, StartingIndex, NumberToFind );
    }

    //
    //  And return to our caller
    //

    return StartingIndex;
}


VOID
RtlClearBits (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG StartingIndex,
    IN ULONG NumberToClear
    )

/*++

Routine Description:

    This procedure clears the specified range of bits within the
    specified bit map.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized Bit Map.

    StartingIndex - Supplies the index (zero based) of the first bit to clear.

    NumberToClear - Supplies the number of bits to clear.

Return Value:

    None.

--*/

{
    PCHAR CurrentByte;
    ULONG BitOffset;

    ASSERT( StartingIndex + NumberToClear <= BitMapHeader->SizeOfBitMap );

    //
    //  Special case the situation where the number of bits to clear is
    //  zero.  Turn this into a noop.
    //

    if (NumberToClear == 0) {

        return;
    }

    //
    //  Get a pointer to the first byte that needs to be cleared.
    //

    CurrentByte = ((PCHAR) BitMapHeader->Buffer) + (StartingIndex / 8);

    //
    //  If all the bit's we're setting are in the same byte just do it and
    //  get out.
    //

    BitOffset = StartingIndex % 8;
    if ((BitOffset + NumberToClear) <= 8) {

        *CurrentByte &= ~(FillMask[ NumberToClear ] << BitOffset);

    }  else {

        //
        //  Do the first byte manually because the first bit may not be byte aligned.
        //
        //  Note:   The first longword will always be cleared byte wise to simplify the
        //          logic of checking for short copies (<32 bits).
        //

        if (BitOffset > 0) {

            *CurrentByte &= FillMask[ BitOffset ];
            CurrentByte += 1;
            NumberToClear -= 8 - BitOffset;

        }

        //
        //  Fill the full bytes in the middle.  Use the RtlZeroMemory() because its
        //  going to be hand tuned asm code spit out by the compiler.
        //

        if (NumberToClear > 8) {

            RtlZeroMemory( CurrentByte, NumberToClear / 8 );
            CurrentByte += NumberToClear / 8;
            NumberToClear %= 8;

        }

        //
        //  Clear the remaining bits, if there are any, in the last byte.
        //

        if (NumberToClear > 0) {

            *CurrentByte &= ZeroMask[ NumberToClear ];

        }

    }

    //
    //  And return to our caller
    //

    return;
}

VOID
RtlSetBits (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG StartingIndex,
    IN ULONG NumberToSet
    )

/*++

Routine Description:

    This procedure sets the specified range of bits within the
    specified bit map.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized BitMap.

    StartingIndex - Supplies the index (zero based) of the first bit to set.

    NumberToSet - Supplies the number of bits to set.

Return Value:

    None.

--*/
{
    PCHAR CurrentByte;
    ULONG BitOffset;

    ASSERT( StartingIndex + NumberToSet <= BitMapHeader->SizeOfBitMap );

    //
    //  Special case the situation where the number of bits to set is
    //  zero.  Turn this into a noop.
    //

    if (NumberToSet == 0) {

        return;
    }

    //
    //  Get a pointer to the first byte that needs to be set.
    //

    CurrentByte = ((PCHAR) BitMapHeader->Buffer) + (StartingIndex / 8);

    //
    //  If all the bit's we're setting are in the same byte just do it and
    //  get out.
    //

    BitOffset = StartingIndex % 8;
    if ((BitOffset + NumberToSet) <= 8) {

        *CurrentByte |= (FillMask[ NumberToSet ] << BitOffset);

    } else {

        //
        //  Do the first byte manually because the first bit may not be byte aligned.
        //
        //  Note:   The first longword will always be set byte wise to simplify the
        //          logic of checking for short copies (<32 bits).
        //

        if (BitOffset > 0) {

            *CurrentByte |= ZeroMask[ BitOffset ];
            CurrentByte += 1;
            NumberToSet -= 8 - BitOffset;

        }

        //
        //  Fill the full bytes in the middle.  Use the RtlFillMemory() because its
        //  going to be hand tuned asm code spit out by the compiler.
        //

        if (NumberToSet > 8) {

            RtlFillMemory( CurrentByte, NumberToSet / 8, 0xff );
            CurrentByte += NumberToSet / 8;
            NumberToSet %= 8;

        }

        //
        //  Set the remaining bits, if there are any, in the last byte.
        //

        if (NumberToSet > 0) {

            *CurrentByte |= FillMask[ NumberToSet ];

        }

    }

    //
    //  And return to our caller
    //

    return;
}


#if DBG
BOOLEAN NtfsDebugIt = FALSE;
#endif

ULONG
RtlFindClearRuns (
    IN PRTL_BITMAP BitMapHeader,
    PRTL_BITMAP_RUN RunArray,
    ULONG SizeOfRunArray,
    BOOLEAN LocateLongestRuns
    )

/*++

Routine Description:

    This procedure finds N contiguous runs of clear bits
    within the specified bit map.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized BitMap.

    RunArray - Receives the bit position, and length of each of the free runs
        that the procedure locates.  The array will be sorted according to
        length.

    SizeOfRunArray - Supplies the maximum number of entries the caller wants
        returned in RunArray

    LocateLongestRuns - Indicates if this routine is to return the longest runs
        it can find or just the first N runs.


Return Value:

    ULONG - Receives the number of runs that the procedure has located and
        returned in RunArray

--*/

{
    ULONG RunIndex;
    ULONG i;
    LONG j;

    ULONG SizeOfBitMap;
    ULONG SizeInBytes;

    ULONG CurrentRunSize;
    ULONG CurrentRunIndex;
    ULONG CurrentByteIndex;
    UCHAR CurrentByte;

    UCHAR BitMask;
    UCHAR TempNumber;

    GET_BYTE_DECLARATIONS();

    //
    //  Reference the bitmap header to make the loop run faster
    //

    SizeOfBitMap = BitMapHeader->SizeOfBitMap;
    SizeInBytes = (SizeOfBitMap + 7) / 8;

    //
    //  Set any unused bits in the last byte so we won't count them.  We do
    //  this by first checking if there is any odd bits in the last byte.
    //

    if ((SizeOfBitMap % 8) != 0) {

        //
        //  The last byte has some odd bits so we'll set the high unused
        //  bits in the last byte to 1's
        //

        ((PUCHAR)BitMapHeader->Buffer)[SizeInBytes - 1] |= ZeroMask[SizeOfBitMap % 8];
    }

    //
    //  Set it up so we can the use GET_BYTE macro
    //

    GET_BYTE_INITIALIZATION( BitMapHeader, 0);

    //
    //  Set our RunIndex and current run variables.  Run Index allays is the index
    //  of the next location to fill in or it could be one beyond the end of the
    //  array.
    //

    RunIndex = 0;
    for (i = 0; i < SizeOfRunArray; i += 1) { RunArray[i].NumberOfBits = 0; }

    CurrentRunSize = 0;
    CurrentRunIndex = 0;

    //
    //  Examine every byte in the BitMap
    //

    for (CurrentByteIndex = 0;
         CurrentByteIndex < SizeInBytes;
         CurrentByteIndex += 1) {

        GET_BYTE( CurrentByte );

#if DBG
        if (NtfsDebugIt) { DbgPrint("%d: %08lx %08lx %08lx %08lx %08lx\n",__LINE__,RunIndex,CurrentRunSize,CurrentRunIndex,CurrentByteIndex,CurrentByte); }
#endif

        //
        //  If the current byte is not all zeros we need to (1) check if
        //  the current run is big enough to be inserted in the output
        //  array, and (2) check if the current byte inside of itself can
        //  be inserted, and (3) start a new current run
        //

        if (CurrentByte != 0x00) {

            //
            //  Compute the final size of the current run
            //

            CurrentRunSize += RtlpBitsClearLow[CurrentByte];

            //
            //  Check if the current run be stored in the output array by either
            //  there being room in the array or the last entry is smaller than
            //  the current entry
            //

            if (CurrentRunSize > 0) {

                if ((RunIndex < SizeOfRunArray) ||
                    (RunArray[RunIndex-1].NumberOfBits < CurrentRunSize)) {

                    //
                    //  If necessary increment the RunIndex and shift over the output
                    //  array until we find the slot where the new run belongs.  We only
                    //  do the shifting if we're returning longest runs.
                    //

                    if (RunIndex < SizeOfRunArray) { RunIndex += 1; }

                    for (j = RunIndex-2; LocateLongestRuns && (j >= 0) && (RunArray[j].NumberOfBits < CurrentRunSize); j -= 1) {

                        RunArray[j+1] = RunArray[j];
                    }

                    RunArray[j+1].NumberOfBits = CurrentRunSize;
                    RunArray[j+1].StartingIndex = CurrentRunIndex;

#if DBG
                    if (NtfsDebugIt) { DbgPrint("%d: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n",
                        __LINE__,RunIndex,CurrentRunSize,CurrentRunIndex,CurrentByteIndex,CurrentByte,j,RunArray[j+1].NumberOfBits,RunArray[j+1].StartingIndex); }
#endif

                    //
                    //  Now if the array is full and we are not doing longest runs return
                    //  to our caller
                    //

                    if (!LocateLongestRuns && (RunIndex >= SizeOfRunArray)) {

                        return RunIndex;
                    }
                }
            }

            //
            //  The next run starts with the remaining clear bits in the
            //  current byte.  We set this up before we check inside the
            //  current byte for a longer run, because the latter test
            //  might require extra work.
            //

            CurrentRunSize = RtlpBitsClearHigh[ CurrentByte ];
            CurrentRunIndex = (CurrentByteIndex * 8) + (8 - CurrentRunSize);

            //
            //  Set the low and high bits, otherwise we'll wind up thinking that we have a
            //  small run that needs to get added to the array, but these bits have
            //  just been accounting for
            //

            CurrentByte |= FillMask[RtlpBitsClearLow[CurrentByte]] |
                           ZeroMask[8-RtlpBitsClearHigh[CurrentByte]];

            //
            //  Check if the current byte contains a run inside of it that
            //  should go into the output array.  There may be multiple
            //  runs in the byte that we need to insert.
            //

            while ((CurrentByte != 0xff)

                        &&

                   ((RunIndex < SizeOfRunArray) ||
                    (RunArray[RunIndex-1].NumberOfBits < (ULONG)RtlpBitsClearAnywhere[CurrentByte]))) {

                TempNumber = RtlpBitsClearAnywhere[CurrentByte];

                //
                //  Somewhere in the current byte is a run to be inserted of
                //  size TempNumber.  All we need to do is find the index for this run.
                //

                BitMask = FillMask[ TempNumber ];

                for (i = 0; (BitMask & CurrentByte) != 0; i += 1) {

                    BitMask <<= 1;
                }

                //
                //  If necessary increment the RunIndex and shift over the output
                //  array until we find the slot where the new run belongs.  We only
                //  do the shifting if we're returning longest runs.
                //

                if (RunIndex < SizeOfRunArray) { RunIndex += 1; }

                for (j = RunIndex-2; LocateLongestRuns && (j >= 0) && (RunArray[j].NumberOfBits < TempNumber); j -= 1) {

                    RunArray[j+1] = RunArray[j];
                }

                RunArray[j+1].NumberOfBits = TempNumber;
                RunArray[j+1].StartingIndex = (CurrentByteIndex * 8) + i;

#if DBG
                if (NtfsDebugIt) { DbgPrint("%d: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n",
                    __LINE__,RunIndex,CurrentRunSize,CurrentRunIndex,CurrentByteIndex,CurrentByte,j,RunArray[j+1].NumberOfBits,RunArray[j+1].StartingIndex); }
#endif

                //
                //  Now if the array is full and we are not doing longest runs return
                //  to our caller
                //

                if (!LocateLongestRuns && (RunIndex >= SizeOfRunArray)) {

                    return RunIndex;
                }

                //
                //  Mask out the bits and look for another run in the current byte
                //

                CurrentByte |= BitMask;
            }

        //
        //  Otherwise the current byte is all zeros and
        //  we simply continue with the current run
        //

        } else {

            CurrentRunSize += 8;
        }
    }

#if DBG
    if (NtfsDebugIt) { DbgPrint("%d: %08lx %08lx %08lx %08lx %08lx\n",__LINE__,RunIndex,CurrentRunSize,CurrentRunIndex,CurrentByteIndex,CurrentByte); }
#endif

    //
    //  See if we finished looking over the bitmap with an open current
    //  run that should be inserted in the output array
    //

    if (CurrentRunSize > 0) {

        if ((RunIndex < SizeOfRunArray) ||
            (RunArray[RunIndex-1].NumberOfBits < CurrentRunSize)) {

            //
            //  If necessary increment the RunIndex and shift over the output
            //  array until we find the slot where the new run belongs.
            //

            if (RunIndex < SizeOfRunArray) { RunIndex += 1; }

            for (j = RunIndex-2; LocateLongestRuns && (j >= 0) && (RunArray[j].NumberOfBits < CurrentRunSize); j -= 1) {

                RunArray[j+1] = RunArray[j];
            }

            RunArray[j+1].NumberOfBits = CurrentRunSize;
            RunArray[j+1].StartingIndex = CurrentRunIndex;

#if DBG
            if (NtfsDebugIt) { DbgPrint("%d: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n",
                __LINE__,RunIndex,CurrentRunSize,CurrentRunIndex,CurrentByteIndex,CurrentByte,j,RunArray[j+1].NumberOfBits,RunArray[j+1].StartingIndex); }
#endif
        }
    }

    //
    //  Return to our caller
    //

    return RunIndex;
}


ULONG
RtlFindLongestRunClear (
    IN PRTL_BITMAP BitMapHeader,
    OUT PULONG StartingIndex
    )

/*++

Routine Description:

    This procedure finds the largest contiguous range of clear bits
    within the specified bit map.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized BitMap.

    StartingIndex - Receives the index (zero based) of the first run
        equal to the longest run of clear bits in the BitMap.

Return Value:

    ULONG - Receives the number of bits contained in the largest contiguous
        run of clear bits.

--*/

{
    RTL_BITMAP_RUN RunArray[1];

    //
    //  Locate the longest run in the bitmap.  If there is one then
    //  return that run otherwise return the error condition.
    //

    if (RtlFindClearRuns( BitMapHeader, RunArray, 1, TRUE ) == 1) {

        *StartingIndex = RunArray[0].StartingIndex;
        return RunArray[0].NumberOfBits;
    }

    *StartingIndex = 0;
    return 0;
}


ULONG
RtlFindFirstRunClear (
    IN PRTL_BITMAP BitMapHeader,
    OUT PULONG StartingIndex
    )

/*++

Routine Description:

    This procedure finds the first contiguous range of clear bits
    within the specified bit map.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized BitMap.

    StartingIndex - Receives the index (zero based) of the first run
        equal to the longest run of clear bits in the BitMap.

Return Value:

    ULONG - Receives the number of bits contained in the first contiguous
        run of clear bits.

--*/

{
    return RtlFindNextForwardRunClear(BitMapHeader, 0, StartingIndex);
}


#define GET_CHUNK(THIS_CHUNK) {                     \
    THIS_CHUNK = *((CHUNK *)_CURRENT_POSITION);     \
    _CURRENT_POSITION += sizeof(CHUNK); }

#if defined(_WIN64)


FORCEINLINE
ULONG
RtlpNumberOfSetBitsInChunk (
    IN CHUNK Target
    )

/*++

Routine Description:

    This procedure counts and returns the number of set bits within
    a the chunk.

Arguments:

    Target - The integer containing the bits to be counted.

Return Value:

    ULONG - The total number of set bits in Target.
           
--*/

{
    //
    // The following algorithm was taken from the AMD publication
    // "Software Optimization Guide for the AMD Hammer Processor",
    // revision 1.19, section 8.6.
    //

    //
    // First break the chunk up into a set of two-bit fields, each field
    // representing the count of set bits.  Each field undergoes the
    // following transformation:
    //
    // 00b -> 00b
    // 01b -> 01b
    // 10b -> 01b
    // 11b -> 10b
    // 

    Target -= (Target >> 1) & 0x5555555555555555;

    //
    // Next, combine the totals in adjacent two-bit fields into a four-bit
    // field.
    // 

    Target = (Target & 0x3333333333333333) +
             ((Target >> 2) & 0x3333333333333333);

    //
    // Now, combine adjacent four-bit fields to end up with a set of 8-bit
    // totals.
    //

    Target = (Target + (Target >> 4)) & 0x0F0F0F0F0F0F0F0F;

    //
    // Finally, sum all of the 8-bit totals.  The result will be the
    // number of bits that were set in Target.
    //

    Target = (Target * 0x0101010101010101) >> 56;
    return (ULONG)Target;
}

#else


FORCEINLINE
ULONG
RtlpNumberOfSetBitsInChunk (
    IN CHUNK Target
    )

/*++

Routine Description:

    This procedure counts and returns the number of set bits within
    a the chunk.

Arguments:

    Target - The integer containing the bits to be counted.

Return Value:

    ULONG - The total number of set bits in Target.
           
--*/

{
    UCHAR setBits;

    Target = ~Target;
    setBits  = RtlpBitsClearTotal[ (UCHAR)Target ];
    Target >>= 8;

    setBits += RtlpBitsClearTotal[ (UCHAR)Target ];
    Target >>= 8;

    setBits += RtlpBitsClearTotal[ (UCHAR)Target ];
    Target >>= 8;

    setBits += RtlpBitsClearTotal[ Target ];
    return setBits;
}

#endif

ULONG
RtlNumberOfSetBits (
    IN PRTL_BITMAP BitMapHeader
    )

/*++

Routine Description:

    This procedure counts and returns the number of set bits within
    the specified bitmap.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized bitmap.

Return Value:

    ULONG - The total number of set bits in the bitmap

--*/

{
    ULONG SizeOfBitMap;
    ULONG SizeInBytes;

    UCHAR CurrentByte;
    CHUNK Chunk;

    ULONG BytesHead;
    ULONG BytesMiddle;
    ULONG BytesTail;

    ULONG TotalSet;

    GET_BYTE_DECLARATIONS();

    //
    //  Reference the bitmap header to make the loop run faster
    //

    SizeOfBitMap = BitMapHeader->SizeOfBitMap;
    SizeInBytes = (SizeOfBitMap + 7) / 8;

    //
    //  Clear any unused bits in the last byte so we don't count them.  We
    //  do this by first checking if there are any odd bits in the last byte
    //

    if ((SizeOfBitMap % 8) != 0) {

        //
        //  The last byte has some odd bits so we'll set the high unused
        //  bits in the last byte to 0's
        //

        ((PUCHAR)BitMapHeader->Buffer)[SizeInBytes - 1] &=
                                                    FillMask[SizeOfBitMap % 8];
    }

    //
    //  Set if up so we can use the GET_BYTE macro
    //

    GET_BYTE_INITIALIZATION( BitMapHeader, 0 );

    //
    //  Examine every byte in the bitmap.  The head and tail portions,
    //  if any, are examined byte-wise while the middle portion is processed
    //  8 bytes at a time.
    //

    TotalSet= 0;
    BytesHead = (ULONG)(-(LONG_PTR)BitMapHeader->Buffer & (sizeof(CHUNK)-1));
    BytesTail = (SizeInBytes - BytesHead) % sizeof(ULONG64);
    BytesMiddle = SizeInBytes - (BytesHead + BytesTail);

    while (BytesHead > 0) {

        GET_BYTE( CurrentByte );
        TotalSet += RtlpBitsSetTotal(CurrentByte);
        BytesHead -= 1;
    }

    while (BytesMiddle > 0) {

        GET_CHUNK( Chunk );
        TotalSet += RtlpNumberOfSetBitsInChunk( Chunk );
        BytesMiddle -= sizeof(CHUNK);
    }

    while (BytesTail > 0) {

        GET_BYTE( CurrentByte );
        TotalSet += RtlpBitsSetTotal(CurrentByte);
        BytesTail -= 1;
    }

    return TotalSet;
}


ULONG
RtlNumberOfClearBits (
    IN PRTL_BITMAP BitMapHeader
    )

/*++

Routine Description:

    This procedure counts and returns the number of clears bits within
    the specified bitmap.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized bitmap.

Return Value:

    ULONG - The total number of clear bits in the bitmap

--*/

{
    return BitMapHeader->SizeOfBitMap - RtlNumberOfSetBits( BitMapHeader );
}



BOOLEAN
RtlAreBitsClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG StartingIndex,
    IN ULONG Length
    )

/*++

Routine Description:

    This procedure determines if the range of specified bits are all clear.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized bitmap.

    StartingIndex - Supplies the starting bit index to examine

    Length - Supplies the number of bits to examine

Return Value:

    BOOLEAN - TRUE if the specified bits in the bitmap are all clear, and
        FALSE if any are set or if the range is outside the bitmap or if
        Length is zero.

--*/

{
    ULONG SizeOfBitMap;
    ULONG SizeInBytes;

    ULONG EndingIndex;

    ULONG StartingByte;
    ULONG EndingByte;

    ULONG StartingOffset;
    ULONG EndingOffset;

    ULONG i;
    UCHAR Byte;

    GET_BYTE_DECLARATIONS();

    //
    //  To make the loops in our test run faster we'll extract the fields
    //  from the bitmap header
    //

    SizeOfBitMap = BitMapHeader->SizeOfBitMap;
    SizeInBytes = (SizeOfBitMap + 7) / 8;

    //
    //  First make sure that the specified range is contained within the
    //  bitmap, and the length is not zero.
    //

    if ((StartingIndex + Length > SizeOfBitMap) || (Length == 0)) {

        return FALSE;
    }

    //
    //  Compute the ending index, starting and ending byte, and the starting
    //  and ending offset within each byte
    //

    EndingIndex = StartingIndex + Length - 1;

    StartingByte = StartingIndex / 8;
    EndingByte = EndingIndex / 8;

    StartingOffset = StartingIndex % 8;
    EndingOffset = EndingIndex % 8;

    //
    //  Set ourselves up to get the next byte
    //

    GET_BYTE_INITIALIZATION( BitMapHeader, StartingByte );

    //
    //  Special case the situation where the starting byte and ending
    //  byte are one in the same
    //

    if (StartingByte == EndingByte) {

        //
        //  Get the single byte we are to look at
        //

        GET_BYTE( Byte );

        //
        //  Now we compute the mask of bits we're after and then AND it with
        //  the byte.  If it is zero then the bits in question are all clear
        //  otherwise at least one of them is set.
        //

        if ((ZeroMask[StartingOffset] & FillMask[EndingOffset+1] & Byte) == 0) {

            return TRUE;

        } else {

            return FALSE;
        }

    } else {

        //
        //  Get the first byte that we're after, and then
        //  compute the mask of bits we're after for the first byte then
        //  AND it with the byte itself.
        //

        GET_BYTE( Byte );

        if ((ZeroMask[StartingOffset] & Byte) != 0) {

            return FALSE;
        }

        //
        //  Now for every whole byte inbetween read in the byte,
        //  and make sure it is all zeros
        //

        for (i = StartingByte+1; i < EndingByte; i += 1) {

            GET_BYTE( Byte );

            if (Byte != 0) {

                return FALSE;
            }
        }

        //
        //  Get the last byte we're after, and then
        //  compute the mask of bits we're after for the last byte then
        //  AND it with the byte itself.
        //

        GET_BYTE( Byte );

        if ((FillMask[EndingOffset+1] & Byte) != 0) {

            return FALSE;
        }
    }

    return TRUE;
}


BOOLEAN
RtlAreBitsSet (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG StartingIndex,
    IN ULONG Length
    )

/*++

Routine Description:

    This procedure determines if the range of specified bits are all set.

Arguments:

    BitMapHeader - Supplies a pointer to the previously initialized bitmap.

    StartingIndex - Supplies the starting bit index to examine

    Length - Supplies the number of bits to examine

Return Value:

    BOOLEAN - TRUE if the specified bits in the bitmap are all set, and
        FALSE if any are clear or if the range is outside the bitmap or if
        Length is zero.

--*/

{
    ULONG SizeOfBitMap;
    ULONG SizeInBytes;

    ULONG EndingIndex;

    ULONG StartingByte;
    ULONG EndingByte;

    ULONG StartingOffset;
    ULONG EndingOffset;

    ULONG i;
    UCHAR Byte;

    GET_BYTE_DECLARATIONS();

    //
    //  To make the loops in our test run faster we'll extract the fields
    //  from the bitmap header
    //

    SizeOfBitMap = BitMapHeader->SizeOfBitMap;
    SizeInBytes = (SizeOfBitMap + 7) / 8;

    //
    //  First make sure that the specified range is contained within the
    //  bitmap, and the length is not zero.
    //

    if ((StartingIndex + Length > SizeOfBitMap) || (Length == 0)) {

        return FALSE;
    }

    //
    //  Compute the ending index, starting and ending byte, and the starting
    //  and ending offset within each byte
    //

    EndingIndex = StartingIndex + Length - 1;

    StartingByte = StartingIndex / 8;
    EndingByte = EndingIndex / 8;

    StartingOffset = StartingIndex % 8;
    EndingOffset = EndingIndex % 8;

    //
    //  Set ourselves up to get the next byte
    //

    GET_BYTE_INITIALIZATION( BitMapHeader, StartingByte );

    //
    //  Special case the situation where the starting byte and ending
    //  byte are one in the same
    //

    if (StartingByte == EndingByte) {

        //
        //  Get the single byte we are to look at
        //

        GET_BYTE( Byte );

        //
        //  Now we compute the mask of bits we're after and then AND it with
        //  the complement of the byte If it is zero then the bits in question
        //  are all clear otherwise at least one of them is clear.
        //

        if ((ZeroMask[StartingOffset] & FillMask[EndingOffset+1] & ~Byte) == 0) {

            return TRUE;

        } else {

            return FALSE;
        }

    } else {

        //
        //  Get the first byte that we're after, and then
        //  compute the mask of bits we're after for the first byte then
        //  AND it with the complement of the byte itself.
        //

        GET_BYTE( Byte );

        if ((ZeroMask[StartingOffset] & ~Byte) != 0) {

            return FALSE;
        }

        //
        //  Now for every whole byte inbetween read in the byte,
        //  and make sure it is all ones
        //

        for (i = StartingByte+1; i < EndingByte; i += 1) {

            GET_BYTE( Byte );

            if (Byte != 0xff) {

                return FALSE;
            }
        }

        //
        //  Get the last byte we're after, and then
        //  compute the mask of bits we're after for the last byte then
        //  AND it with the complement of the byte itself.
        //

        GET_BYTE( Byte );

        if ((FillMask[EndingOffset+1] & ~Byte) != 0) {

            return FALSE;
        }
    }

    return TRUE;
}

static CONST ULONG FillMaskUlong[] = {
    0x00000000, 0x00000001, 0x00000003, 0x00000007,
    0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
    0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
    0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
    0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
    0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
    0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
    0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
    0xffffffff
};


ULONG
RtlFindNextForwardRunClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG FromIndex,
    IN PULONG StartingRunIndex
    )
{
    ULONG Start;
    ULONG End;
    PULONG PHunk, BitMapEnd;
    ULONG Hunk;

    //
    // Take care of the boundary case of the null bitmap
    //

    if (BitMapHeader->SizeOfBitMap == 0) {

        *StartingRunIndex = FromIndex;
        return 0;
    }

    //
    //  Compute the last word address in the bitmap
    //

    BitMapEnd = BitMapHeader->Buffer + ((BitMapHeader->SizeOfBitMap - 1) / 32);

    //
    //  Scan forward for the first clear bit
    //

    Start = FromIndex;

    //
    //  Build pointer to the ULONG word in the bitmap
    //  containing the Start bit
    //

    PHunk = BitMapHeader->Buffer + (Start / 32);

    //
    //  If the first subword is set then we can proceed to
    //  take big steps in the bitmap since we are now ULONG
    //  aligned in the search. Make sure we aren't improperly
    //  looking at the last word in the bitmap.
    //

    if (PHunk != BitMapEnd) {

        //
        //  Read in the bitmap hunk. Set the previous bits in this word.
        //

        Hunk = *PHunk | FillMaskUlong[Start % 32];

        if (Hunk == (ULONG)~0) {

            //
            //  Adjust the pointers forward
            //

            Start += 32 - (Start % 32);
            PHunk++;

            while ( PHunk < BitMapEnd ) {

                //
                //  Stop at first word with unset bits
                //

                if (*PHunk != (ULONG)~0) break;

                PHunk++;
                Start += 32;
            }
        }
    }

    //
    //  Bitwise search forward for the clear bit
    //

    while ((Start < BitMapHeader->SizeOfBitMap) && (RtlCheckBit( BitMapHeader, Start ) == 1)) { Start += 1; }

    //
    //  Scan forward for the first set bit
    //

    End = Start;

    //
    //  If we aren't in the last word of the bitmap we may be
    //  able to keep taking big steps
    //

    if (PHunk != BitMapEnd) {

        //
        //  We know that the clear bit was in the last word we looked at,
        //  so continue from there to find the next set bit, clearing the
        //  previous bits in the word
        //

        Hunk = *PHunk & ~FillMaskUlong[End % 32];

        if (Hunk == (ULONG)0) {

            //
            //  Adjust the pointers forward
            //

            End += 32 - (End % 32);
            PHunk++;

            while ( PHunk < BitMapEnd ) {

                //
                //  Stop at first word with set bits
                //

                if (*PHunk != (ULONG)0) break;

                PHunk++;
                End += 32;
            }
        }
    }

    //
    //  Bitwise search forward for the set bit
    //

    while ((End < BitMapHeader->SizeOfBitMap) && (RtlCheckBit( BitMapHeader, End ) == 0)) { End += 1; }

    //
    //  Compute the index and return the length
    //

    *StartingRunIndex = Start;
    return (End - Start);
}


ULONG
RtlFindLastBackwardRunClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG FromIndex,
    IN PULONG StartingRunIndex
    )
{
    ULONG Start;
    ULONG End;
    PULONG PHunk;
    ULONG Hunk;

    RTL_PAGED_CODE();

    //
    //  Take care of the boundary case of the null bitmap
    //

    if (BitMapHeader->SizeOfBitMap == 0) {

        *StartingRunIndex = FromIndex;
        return 0;
    }

    //
    //  Scan backwards for the first clear bit
    //

    End = FromIndex;

    //
    //  Build pointer to the ULONG word in the bitmap
    //  containing the End bit, then read in the bitmap
    //  hunk. Set the rest of the bits in this word, NOT
    //  inclusive of the FromIndex bit.
    //

    PHunk = BitMapHeader->Buffer + (End / 32);
    Hunk = *PHunk | ~FillMaskUlong[(End % 32) + 1];

    //
    //  If the first subword is set then we can proceed to
    //  take big steps in the bitmap since we are now ULONG
    //  aligned in the search
    //

    if (Hunk == (ULONG)~0) {

        //
        //  Adjust the pointers backwards
        //

        End -= (End % 32) + 1;
        PHunk--;

        while ( PHunk > BitMapHeader->Buffer ) {

            //
            //  Stop at first word with set bits
            //

            if (*PHunk != (ULONG)~0) break;

            PHunk--;
            End -= 32;
        }
    }

    //
    //  Bitwise search backward for the clear bit
    //

    while ((End != MAXULONG) && (RtlCheckBit( BitMapHeader, End ) == 1)) { End -= 1; }

    //
    //  Scan backwards for the first set bit
    //

    Start = End;

    //
    //  We know that the clear bit was in the last word we looked at,
    //  so continue from there to find the next set bit, clearing the
    //  previous bits in the word.
    //

    Hunk = *PHunk & FillMaskUlong[Start % 32];

    //
    //  If the subword is unset then we can proceed in big steps
    //

    if (Hunk == (ULONG)0) {

        //
        //  Adjust the pointers backward
        //

        Start -= (Start % 32) + 1;
        PHunk--;

        while ( PHunk > BitMapHeader->Buffer ) {

            //
            //  Stop at first word with set bits
            //

            if (*PHunk != (ULONG)0) break;

            PHunk--;
            Start -= 32;
        }
    }

    //
    //  Bitwise search backward for the set bit
    //

    while ((Start != MAXULONG) && (RtlCheckBit( BitMapHeader, Start ) == 0)) { Start -= 1; }

    //
    //  Compute the index and return the length
    //

    *StartingRunIndex = Start + 1;
    return (End - Start);
}

#define BM_4567 0xFFFFFFFF00000000UI64
#define BM_67   0xFFFF000000000000UI64
#define BM_7    0xFF00000000000000UI64
#define BM_5    0x0000FF0000000000UI64
#define BM_23   0x00000000FFFF0000UI64
#define BM_3    0x00000000FF000000UI64
#define BM_1    0x000000000000FF00UI64

#define BM_0123 0x00000000FFFFFFFFUI64
#define BM_01   0x000000000000FFFFUI64
#define BM_0    0x00000000000000FFUI64
#define BM_2    0x0000000000FF0000UI64
#define BM_45   0x0000FFFF00000000UI64
#define BM_4    0x000000FF00000000UI64
#define BM_6    0x00FF000000000000UI64

CCHAR
RtlFindMostSignificantBit (
    IN ULONGLONG Set
    )
/*++

Routine Description:

    This procedure finds the most significant non-zero bit in Set and
    returns it's zero-based position.

Arguments:

    Set - Supplies the 64-bit bitmap.

Return Value:

    Set != 0:
        Bit position of the most significant set bit in Set.

    Set == 0:
        -1.

--*/
{

#if defined(_AMD64_)

    ULONG bitOffset;

    if (BitScanReverse64(&bitOffset, Set)) {
        return (CCHAR)bitOffset;

    } else {
        return -1;
    }

#else

    UCHAR index;
    UCHAR bitOffset;
    UCHAR lookup;

    if ((Set & BM_4567) != 0) {
        if ((Set & BM_67) != 0) {
            if ((Set & BM_7) != 0) {
                bitOffset = 7 * 8;
            } else {
                bitOffset = 6 * 8;
            }
        } else {
            if ((Set & BM_5) != 0) {
                bitOffset = 5 * 8;
            } else {
                bitOffset = 4 * 8;
            }
        }
    } else {
        if ((Set & BM_23) != 0) {
            if ((Set & BM_3) != 0) {
                bitOffset = 3 * 8;
            } else {
                bitOffset = 2 * 8;
            }
        } else {
            if ((Set & BM_1) != 0) {
                bitOffset = 1 * 8;
            } else {

                //
                // The test for Set == 0 is postponed to here, it is expected
                // to be rare.  Note that if we had our own version of
                // RtlpBitsClearHigh[] we could eliminate this test entirely,
                // reducing the average number of tests from 3.125 to 3.
                //

                if (Set == 0) {
                    return -1;
                }

                bitOffset = 0 * 8;
            }
        }
    }

    lookup = (UCHAR)(Set >> bitOffset);
    index = (7 - RtlpBitsClearHigh[lookup]) + bitOffset;
    return index;

#endif

}

CCHAR
RtlFindLeastSignificantBit (
    IN ULONGLONG Set
    )
/*++

Routine Description:

    This procedure finds the least significant non-zero bit in Set and
    returns it's zero-based position.

Arguments:

    Set - Supplies the 64-bit bitmap.

Return Value:

    Set != 0:
        Bit position of the least significant non-zero bit in Set.

    Set == 0:
        -1.

--*/
{

#if defined(_AMD64_)

    ULONG bitOffset;

    if (BitScanForward64(&bitOffset, Set)) {
        return (CCHAR)bitOffset;

    } else {
        return -1;
    }

#else

    UCHAR index;
    UCHAR bitOffset;
    UCHAR lookup;

    if ((Set & BM_0123) != 0) {
        if ((Set & BM_01) != 0) {
            if ((Set & BM_0) != 0) {
                bitOffset = 0 * 8;
            } else {
                bitOffset = 1 * 8;
            }
        } else {
            if ((Set & BM_2) != 0) {
                bitOffset = 2 * 8;
            } else {
                bitOffset = 3 * 8;
            }
        }
    } else {
        if ((Set & BM_45) != 0) {
            if ((Set & BM_4) != 0) {
                bitOffset = 4 * 8;
            } else {
                bitOffset = 5 * 8;
            }
        } else {
            if ((Set & BM_6) != 0) {
                bitOffset = 6 * 8;
            } else {

                //
                // The test for Set == 0 is postponed to here, it is expected
                // to be rare.  Note that if we had our own version of
                // RtlpBitsClearHigh[] we could eliminate this test entirely,
                // reducing the average number of tests from 3.125 to 3.
                //

                if (Set == 0) {
                    return -1;
                }

                bitOffset = 7 * 8;
            }
        }
    }

    lookup = (UCHAR)(Set >> bitOffset);
    index = RtlpBitsClearLow[lookup] + bitOffset;
    return index;

#endif

}

