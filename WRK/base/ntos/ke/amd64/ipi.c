/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ipi.c

Abstract:

    This module implements AMD64 specific interprocessor interrupt
    routines.

--*/

#include "ki.h"

VOID
KiRestoreProcessorState (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

/*++

Routine Description:

    This function restores the processor state to the specified exception
    and trap frames, and restores the processor control state.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame.

    ExceptionFrame - Supplies a pointer to an exception frame.

Return Value:

    None.

--*/

{

#if !defined(NT_UP)

    PKPRCB Prcb;
    KPROCESSOR_MODE PreviousMode;

    //
    // Get the address of the current processor block, move the specified
    // register state from the processor context structure to the specified
    // trap and exception frames, and restore the processor control state.
    //

    if ((TrapFrame->SegCs & MODE_MASK) != 0) {
        PreviousMode = UserMode;
    } else {
        PreviousMode = KernelMode;
    }

    Prcb = KeGetCurrentPrcb();
    KeContextToKframes(TrapFrame,
                       ExceptionFrame,
                       &Prcb->ProcessorState.ContextFrame,
                       CONTEXT_FULL,
                       PreviousMode);

    KiRestoreProcessorControlState(&Prcb->ProcessorState);

#else

    UNREFERENCED_PARAMETER(TrapFrame);
    UNREFERENCED_PARAMETER(ExceptionFrame);

#endif

    return;
}

VOID
KiSaveProcessorState (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

/*++

Routine Description:

    This function saves the processor state from the specified exception
    and trap frames, and saves the processor control state.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame.

    ExceptionFrame - Supplies a pointer to an exception frame.

Return Value:

    None.

--*/

{

#if !defined(NT_UP)

    PKPRCB Prcb;

    //
    // Get the address of the current processor block, move the specified
    // register state from specified trap and exception frames to the current
    // processor context structure, and save the processor control state.
    //

    Prcb = KeGetCurrentPrcb();
    Prcb->ProcessorState.ContextFrame.ContextFlags = CONTEXT_FULL;
    KeContextFromKframes(TrapFrame,
                         ExceptionFrame,
                         &Prcb->ProcessorState.ContextFrame);

    KiSaveProcessorControlState(&Prcb->ProcessorState);

#else

    UNREFERENCED_PARAMETER(TrapFrame);
    UNREFERENCED_PARAMETER(ExceptionFrame);

#endif

    return;
}

DECLSPEC_NOINLINE
VOID
KiIpiProcessRequests (
    VOID
    )

/*++

Routine Description:

    This routine processes interprocessor requests and returns a summary
    of the requests that were processed.

Arguments:

    None.

Return Value:

    None.

--*/

{

#if !defined(NT_UP)

    PVOID *End;
    ULONG64 Number;
    PKPRCB Packet;
    PKPRCB Prcb;
    ULONG Processor;
    REQUEST_SUMMARY Request;
    PREQUEST_MAILBOX RequestMailbox;
    PKREQUEST_PACKET RequestPacket;
    LONG64 SetMember;
    PKPRCB Source;
    KAFFINITY SummarySet;
    KAFFINITY TargetSet;
    PVOID *Virtual;

    //
    // Loop until the sender summary is zero.
    //

    Prcb = KeGetCurrentPrcb();
    TargetSet = ReadForWriteAccess(&Prcb->SenderSummary);
    SetMember = Prcb->SetMember;
    while (TargetSet != 0) {
        SummarySet = TargetSet;
        BitScanForward64(&Processor, SummarySet);
        do {
            Source = KiProcessorBlock[Processor];
            RequestMailbox = &Prcb->RequestMailbox[Processor];
            Request.Summary = RequestMailbox->RequestSummary;

            //
            // If the request type is flush multiple immediate, flush process,
            // flush single, or flush all, then packet done can be signaled
            // before processing the request. Otherwise, the request type must
            // be a packet request, a cache invalidate, or a flush multiple
            //

            if (Request.IpiRequest <= IPI_FLUSH_ALL) {

                //
                // If the synchronization type is target set, then the IPI was
                // only between two processors and target set should be used
                // for synchronization. Otherwise, packet barrier is used for
                // synchronization.
                //
    
                if (Request.IpiSynchType == 0) {
                    if (SetMember == InterlockedXor64((PLONG64)&Source->TargetSet,
                                                      SetMember)) {
    
                        Source->PacketBarrier = 0;
                    }
    
                } else {
                    Source->TargetSet = 0;
                }

                if (Request.IpiRequest == IPI_FLUSH_MULTIPLE_IMMEDIATE) {
                    Number = Request.Count;
                    Virtual = &RequestMailbox->Virtual[0];
                    End = Virtual + Number;
                    do {
                        KiFlushSingleTb(*Virtual);
                        Virtual += 1;
                    } while (Virtual < End);

                } else if (Request.IpiRequest == IPI_FLUSH_PROCESS) {
                    KiFlushProcessTb();
        
                } else if (Request.IpiRequest == IPI_FLUSH_SINGLE) {
                    KiFlushSingleTb((PVOID)Request.Parameter);
        
                } else {

                    ASSERT(Request.IpiRequest == IPI_FLUSH_ALL);

                    KeFlushCurrentTb();
                }

            } else {

                //
                // If the request type is packet ready, then call the worker
                // function. Otherwise, the request must be either a flush
                // multiple or a cache invalidate.
                //
        
                if (Request.IpiRequest == IPI_PACKET_READY) {
                    Packet = Source;
                    if (Request.IpiSynchType != 0) {
                        Packet = (PKPRCB)((ULONG64)Source + 1);
                    }
    
                    RequestPacket = (PKREQUEST_PACKET)&RequestMailbox->RequestPacket;
                    (RequestPacket->WorkerRoutine)((PKIPI_CONTEXT)Packet,
                                                   RequestPacket->CurrentPacket[0],
                                                   RequestPacket->CurrentPacket[1],
                                                   RequestPacket->CurrentPacket[2]);
        
                } else {
                    if (Request.IpiRequest == IPI_FLUSH_MULTIPLE) {
                        Number = Request.Count;
                        Virtual = (PVOID *)Request.Parameter;
                        End = Virtual + Number;
                        do {
                            KiFlushSingleTb(*Virtual);
                            Virtual += 1;
                        } while (Virtual < End);

                    } else if (Request.IpiRequest == IPI_INVALIDATE_ALL) {
                        WritebackInvalidate();

                    } else {

                        ASSERT(FALSE);

                    }
        
                    //
                    // If the synchronization type is target set, then the IPI was
                    // only between two processors and target set should be used
                    // for synchronization. Otherwise, packet barrier is used for
                    // synchronization.
                    //
        
                    if (Request.IpiSynchType == 0) {
                        if (SetMember == InterlockedXor64((PLONG64)&Source->TargetSet,
                                                          SetMember)) {
        
                            Source->PacketBarrier = 0;
                        }
        
                    } else {
                        Source->TargetSet = 0;
                    }
                }
            }
            
            SummarySet ^= AFFINITY_MASK(Processor);
        } while (BitScanForward64(&Processor, SummarySet) != FALSE);

        //
        // Clear target set in sender summary.
        //

        TargetSet = 
            InterlockedExchangeAdd64((LONG64 volatile *)&Prcb->SenderSummary,
                                     -(LONG64)TargetSet) - TargetSet;
    }

#endif

    return;
}

DECLSPEC_NOINLINE
PKAFFINITY
KiIpiSendRequest (
    IN KAFFINITY TargetSet,
    IN ULONG64 Parameter,
    IN ULONG64 Count,
    IN ULONG64 RequestType
    )

/*++

Routine Description:

    This routine executes the specified immediate request on the specified
    set of processors.

    N.B. This function MUST be called from a non-context switchable state.

Arguments:

   TargetProcessors - Supplies the set of processors on which the specfied
       operation is to be executed.

   Parameter - Supplies the parameter data that will be packed into the
       request summary.

   Count - Supplies the count data that will be packed into the request summary.

   RequestType - Supplies the type of immediate request.

Return Value:

    The address of the appropriate request barrier is returned as the function
    value.

--*/

{

#if !defined(NT_UP)

    PKAFFINITY Barrier;
    PKPRCB Destination;
    ULONG Number;
    KAFFINITY PacketTargetSet;
    PKPRCB Prcb;
    ULONG Processor;
    PREQUEST_MAILBOX RequestMailbox;
    KAFFINITY SetMember;
    PVOID *Start;
    KAFFINITY SummarySet;
    KAFFINITY TargetMember;
    REQUEST_SUMMARY Template;
    PVOID *Virtual;

    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

    //
    // Initialize request template.
    //

    Prcb = KeGetCurrentPrcb();
    Template.Summary = 0;
    Template.IpiRequest = RequestType;
    Template.Count = Count;
    Template.Parameter = Parameter;

    //
    // If the target set contains one and only one processor, then use the
    // target set for signal done synchronization. Otherwise, use packet
    // barrier for signal done synchronization.
    //

    Prcb->TargetSet = TargetSet;
    if ((TargetSet & (TargetSet - 1)) == 0) {
        Template.IpiSynchType = TRUE;
        Barrier = (PKAFFINITY)&Prcb->TargetSet;

    } else {
        Prcb->PacketBarrier = 1;
        Barrier = (PKAFFINITY)&Prcb->PacketBarrier;
    }

    //
    // Loop through the target set of processors and set the request summary.
    // If a target processor is already processing a request, then remove
    // that processor from the target set of processor that will be sent an
    // interprocessor interrupt.
    //
    // N.B. It is guaranteed that there is at least one bit set in the target
    //      set.
    //

    Number = Prcb->Number;
    SetMember = Prcb->SetMember;
    SummarySet = TargetSet;
    PacketTargetSet = TargetSet;
    BitScanForward64(&Processor, SummarySet);
    do {
        Destination = KiProcessorBlock[Processor];
        PrefetchForWrite(&Destination->SenderSummary);
        RequestMailbox = &Destination->RequestMailbox[Number];
        PrefetchForWrite(RequestMailbox);
        TargetMember = AFFINITY_MASK(Processor);

        //
        // Make sure that processing of the last IPI is complete before sending
        // another IPI to the same processor.
        // 

        while ((Destination->SenderSummary & SetMember) != 0) {
            KeYieldProcessor();
        }

        //
        // If the request type is flush multiple and the flush entries will
        // fit in the mailbox, then copy the virtual address array to the
        // destination mailbox and change the request type to flush immediate.
        //
        // If the request type is packet ready, then copy the packet to the
        // destination mailbox.
        //

        if (RequestType == IPI_FLUSH_MULTIPLE) {
            Virtual = &RequestMailbox->Virtual[0];
            Start = (PVOID *)Parameter;
            switch (Count) {

                //
                // Copy of up to seven virtual addresses and a conversion of
                // the request type to flush multiple immediate.
                //

            case 7:
                Virtual[6] = Start[6];
            case 6:
                Virtual[5] = Start[5];
            case 5:
                Virtual[4] = Start[4];
            case 4:
                Virtual[3] = Start[3];
            case 3:
                Virtual[2] = Start[2];
            case 2:
                Virtual[1] = Start[1];
            case 1:
                Virtual[0] = Start[0];
                Template.IpiRequest = IPI_FLUSH_MULTIPLE_IMMEDIATE;
                break;
            }

        } else if (RequestType == IPI_PACKET_READY) {
            RequestMailbox->RequestPacket = *(PKREQUEST_PACKET)Parameter;
        }

        RequestMailbox->RequestSummary = Template.Summary;
        if (InterlockedExchangeAdd64((LONG64 volatile *)&Destination->SenderSummary,
                                     SetMember) != 0) {

            TargetSet ^= TargetMember;
        }

        SummarySet ^= TargetMember;
    } while (BitScanForward64(&Processor, SummarySet) != FALSE);

    //
    // Request interprocessor interrupts on the remaining target set of
    // processors.
    //
    //
    // N.B. For packet sends, there exists a potential deadlock situation
    //      unless an IPI is sent to the original set of target processors.
    //      The deadlock arises from the fact that the targets will spin in
    //      their IPI routines.
    //

    if (RequestType == IPI_PACKET_READY) {
        TargetSet = PacketTargetSet;
    }

    if (TargetSet != 0) {
        HalRequestIpi(TargetSet);
    }

    return Barrier;

#else

    UNREFERENCED_PARAMETER(TargetSet);
    UNREFERENCED_PARAMETER(Parameter);
    UNREFERENCED_PARAMETER(Count);
    UNREFERENCED_PARAMETER(RequestType);

    return NULL;

#endif

}

VOID
KiIpiSendPacket (
    IN KAFFINITY TargetSet,
    IN PKIPI_WORKER WorkerFunction,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )

/*++

Routine Description:

    This routine executes the specified worker function on the specified
    set of processors.

    N.B. This function MUST be called from a non-context switchable state.

Arguments:

   TargetProcessors - Supplies the set of processors on which the specfied
       operation is to be executed.

   WorkerFunction  - Supplies the address of the worker function.

   Parameter1 - Parameter3 - Supplies worker function specific parameters.

Return Value:

    None.

--*/

{

#if !defined(NT_UP)

    KREQUEST_PACKET RequestPacket;

    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

    //
    // Initialize the worker packet information.
    //

    RequestPacket.CurrentPacket[0] = Parameter1;
    RequestPacket.CurrentPacket[1] = Parameter2;
    RequestPacket.CurrentPacket[2] = Parameter3;
    RequestPacket.WorkerRoutine = WorkerFunction;

    //
    // Send request.
    //

    KiIpiSendRequest(TargetSet, (ULONG64)&RequestPacket, 0, IPI_PACKET_READY);

#else

    UNREFERENCED_PARAMETER(TargetSet);
    UNREFERENCED_PARAMETER(WorkerFunction);
    UNREFERENCED_PARAMETER(Parameter1);
    UNREFERENCED_PARAMETER(Parameter2);
    UNREFERENCED_PARAMETER(Parameter3);

#endif

    return;
}

VOID
KiIpiSignalPacketDone (
    IN PKIPI_CONTEXT SignalDone
    )

/*++

Routine Description:

    This routine signals that a processor has completed a packet by clearing
    the calling processor's set member of the requesting processor's packet.

Arguments:

    SignalDone - Supplies a pointer to the processor block of the sending
        processor.

Return Value:

     None.

--*/

{

#if !defined(NT_UP)

    LONG64 SetMember;
    PKPRCB TargetPrcb;


    //
    // If the low bit of signal is set, then use target set to notify the
    // sender that the operation is complete on the current processor.
    // Otherwise, use packet barrier to notify the sender that the operation
    // is complete on the current processor.
    //

    if (((ULONG64)SignalDone & 1) == 0) {

        SetMember = KeGetCurrentPrcb()->SetMember;
        TargetPrcb = (PKPRCB)SignalDone;
        if (SetMember == InterlockedXor64((PLONG64)&TargetPrcb->TargetSet,
                                          SetMember)) {

            TargetPrcb->PacketBarrier = 0;
        }

    } else {
        TargetPrcb = (PKPRCB)((ULONG64)SignalDone - 1);
        TargetPrcb->TargetSet = 0;
    }

#else

    UNREFERENCED_PARAMETER(SignalDone);

#endif

    return;
}

