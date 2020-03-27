//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


//
// Generates jobs (messages) with the given interarrival time,
// which can be a random variate.
//
package fifo_scheduler_network;

simple Source
{
    parameters:
        volatile double generatorPeriod @unit(ms);
        @display("i=block/source");
        int priority;
    gates:
        output txPackets;
        output userPriority;
}


