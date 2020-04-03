#include <omnetpp.h>
#include "Utils.h"
#include <omnetpp/ccomponent.h>
#include "IntPackage_m.h"
#include "MultiplePackets_m.h"

#define SIMTIME_DBL(t) ((t).dbl())

using namespace omnetpp;

class Fifo : public cSimpleModule {
private:
    cMessage *sendEvent;
    cMessage *customNrEvent;
    cQueue queue;
    int total;
    int errors;
    int fuzzyError;
    bool queueClear;
    cOutVector delayInfo;
    int error_rate;
    int nrToSend;
    int userID;
    int i;
    double tmpTime;
    double tmpId;
public:
    Fifo();
    virtual ~Fifo();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    void sendEmptyMessage();
};

Define_Module(Fifo);

Fifo::Fifo() {
    sendEvent = nullptr;
}

Fifo::~Fifo() {
    cancelAndDelete(sendEvent);
    queue.clear();
}



void Fifo::initialize() {
    sendEvent = new cMessage("initialized");
    customNrEvent = new cMessage("multiplePackets");
    customNrEvent->setKind(TYPE::MULTIPLE_PACKETS_SEQUENCE);
    queueClear = true;
    errors = 0;
    total = 0;
    fuzzyError = 0;
    error_rate = 20;
}

void Fifo::sendEmptyMessage() {
    cMessage *qStatus = new cMessage("queueIsEmpty");
    qStatus->setKind(QUEUE::IS_EMPTY);
    send(qStatus, "queueEmptyStatus");
}


void Fifo::handleMessage(cMessage *msg) {
    if (msg->arrivedOn("rxScheduling") || msg->getKind() == TYPE::MULTIPLE_PACKETS_SEQUENCE)
    {
    if ((msg->getKind() == TYPE::MULTIPLE_PACKETS_FROM_ONE_MSG))
        {
        //reset errors and total
        errors=0;
        total=0;
        fuzzyError = 0;
        //reset errors and total

        nrToSend = ((MultiplePackets*)msg)->getPacketsToSend();
        userID = ((MultiplePackets*)msg)->getId();
        tmpTime = 3;
        tmpId = (double)userID;
        i=1;
        customNrEvent = new cMessage("multiplePackets");
        customNrEvent->setKind(TYPE::MULTIPLE_PACKETS_SEQUENCE);
        scheduleAt(simTime()+tmpTime-tmpId+((double)i), customNrEvent);
        i++;
        }
    else if ((msg->getKind() == TYPE::MULTIPLE_PACKETS_SEQUENCE))
        {
        if(i<=nrToSend)
            {
            customNrEvent = new cMessage("multiplePackets");
            customNrEvent->setKind(TYPE::MULTIPLE_PACKETS_SEQUENCE);
            scheduleAt(simTime()+1, customNrEvent);
            i++;
            }

        cMessage *message = static_cast<cMessage*>(queue.pop());
        auto delay = (simTime() - message->getArrivalTime()).dbl();
        delayInfo.recordWithTimestamp(message->getArrivalTime(), delay);

        int random = intrand(100);
        if (random > error_rate)
            {
            message->setKind(ERROR::IS_NOT_ERROR);
            message->setName("NOT_ERROR");
            }
        else
            {
            message->setKind(ERROR::IS_ERROR);
            message->setName("ERROR");
            errors = errors + 1;
            }

        total = total + 1;
        send(message, "txPackets");
        }
    else
        {
        cancelAndDelete(msg);

        if (queue.isEmpty()) {
            sendEmptyMessage();
            return;
        }

        cMessage *message = static_cast<cMessage*>(queue.pop());
        auto delay = (simTime() - message->getArrivalTime()).dbl();
        delayInfo.recordWithTimestamp(message->getArrivalTime(), delay);

        int random = intrand(100);
        if (random > error_rate)
            {
            message->setKind(ERROR::IS_NOT_ERROR);
            message->setName("NOT_ERROR");
            }
        else
            {
            message->setKind(ERROR::IS_ERROR);
            message->setName("ERROR");
            errors = errors + 1;
            }

        total = total + 1;
        send(message, "txPackets");

        if (queue.isEmpty()) //IF AFTER POP IS EMPTY
            sendEmptyMessage();
        }
    }
    else if (msg->arrivedOn("itsFuzzyTime"))
    {
        if (total > 0)
        {
            fuzzyError = (errors*100) / total;
        }
        else
            fuzzyError = 0;

        IntPackage *msg = new IntPackage("fuzzyStuff");
        msg->setFuzzy_value(fuzzyError);
        msg->setError_value(errors);
        msg->setTotal_value(total);
        if (simTime() != 0)
            send(msg, "fuzzyErrorNumber");
    }
    else
    {
    queueClear = queue.isEmpty();

    queue.insert(msg);

        if (queueClear && simTime()!= 0 )
        {
        cMessage *msgQueueStatus = new cMessage("queueNotEmpty");
        msgQueueStatus->setKind(QUEUE::IS_NOT_EMPTY);
        send(msgQueueStatus, "queueEmptyStatus");
        }
    }
}

