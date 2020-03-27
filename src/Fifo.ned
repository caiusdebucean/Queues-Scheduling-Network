//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


//
// Single-server queue, service rate independent of message length
//
package fifo_scheduler_network;

simple Fifo
{
    parameters:
        @display("i=block/queue;q=queue");
        @signal[qlen](type="long");
        @statistic[qlen](title="queue length";record=vector,timeavg,max;interpolationmode=sample-hold);
    gates:
        input rxScheduling;
        input rxPackets;
        input itsFuzzyTime;
        output txPackets;
        output queueEmptyStatus;
        output fuzzyErrorNumber;

}
