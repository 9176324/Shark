/**************************************************************************

    AVStream Simulated Hardware Sample

    Copyright (c) 2001, Microsoft Corporation.

    File:

        TStream.h

    Abstract:

    History:

        created 8/1/2001

**************************************************************************/

/**************************************************************************

    Constants

**************************************************************************/

/*************************************************

    CTsSynthesizer

    This class synthesizes a transport stream for output from the
    capture filter.

*************************************************/

class CTsSynthesizer {

protected:

    //
    // The packet size of the transport stream
    //
    ULONG m_PacketSize;

    //
    // The number of packets in a capture buffer
    //
    ULONG m_PacketsPerBuffer;

    //
    // The size of the actual data in the capture buffer
    //
    ULONG m_SampleSize;

    //
    // The synthesis buffer.  All transport stream samples are created in this
    // buffer.  This must be set with SetBuffer() before any sample creation
    // routines are called.
    //
    PUCHAR m_SynthesisBuffer;

    //
    // The default cursor.  This is a pointer into the synthesis buffer where
    // the next transport stream byte will be placed. 
    //
    PUCHAR m_Cursor;

public:

    //
    // SetSampleSize():
    //
    // Set the size of the synthesis buffer.
    //
    void
    SetSampleSize (
        ULONG PacketSize,
        ULONG PacketsPerBuffer
        )
    {
        m_PacketSize = PacketSize;
        m_PacketsPerBuffer = PacketsPerBuffer;
        m_SampleSize = PacketSize * PacketsPerBuffer;
    }

    //
    // SetBuffer():
    //
    // Set the buffer the synthesizer generates images to.
    //
    void
    SetBuffer (
        PUCHAR SynthesisBuffer
        )
    {
        m_SynthesisBuffer = SynthesisBuffer;
    }

    //
    // GetTsLocation():
    //
    // Set the cursor to point at the given packet index.
    //
    virtual PUCHAR
    GetTsLocation (
        ULONG PacketIndex
        )
    {
        if (   m_SynthesisBuffer 
            && m_PacketSize 
            && (PacketIndex < m_PacketsPerBuffer)
           )
        {
            m_Cursor = m_SynthesisBuffer + (PacketIndex * m_PacketSize);
        }
        else
        {
            m_Cursor = NULL;
        }

        return m_Cursor;
    }

    //
    // PutPacket():
    //
    // Place a transport stream packet at the default cursor location.
    // The cursor location must be set via GetTsLocation(PacketIndex).
    //
    virtual void
    PutPacket (
        PUCHAR  TsPacket
        )

    {
        //
        //  Copy the transport packet to the synthesis buffer.
        //
        RtlCopyMemory (
            m_Cursor,
            TsPacket,
            m_PacketSize
            );
        m_Cursor += m_PacketSize;
    }
    
    //
    // SynthesizeTS():
    //
    // Synthesize the next transport stream buffer to be captured.
    //
    virtual void
    SynthesizeTS (
        );

    //
    // DEFAULT CONSTRUCTOR
    //
    CTsSynthesizer (
        ) :
        m_PacketSize (0),
        m_PacketsPerBuffer (0),
        m_SynthesisBuffer (NULL)
    {
    }

    //
    // CONSTRUCTOR:
    //
    CTsSynthesizer (
        ULONG PacketSize,
        ULONG PacketsPerBuffer
        ) :
        m_PacketSize (PacketSize),
        m_PacketsPerBuffer (PacketsPerBuffer),
        m_SynthesisBuffer (NULL)
    {
    }

    //
    // DESTRUCTOR:
    //
    virtual
    ~CTsSynthesizer (
        )
    {
    }

};

