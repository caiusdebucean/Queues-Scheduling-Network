#include <omnetpp.h>
#include "Utils.h"
#include <omnetpp/ccomponent.h>
#include "IntPackage_m.h"

using namespace omnetpp;

class Fifo : public cSimpleModule {
private:
    cMessage *sendEvent;
    cQueue queue;
    int total;
    int errors;
    int fuzzyError;
    bool queueClear;
    cOutVector delayInfo;
    int error_rate;
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
    queueClear = true;
    errors = 0;
    total = 0;
    fuzzyError = 0;
    error_rate = 40;
}

void Fifo::sendEmptyMessage() {
    cMessage *qStatus = new cMessage("queueIsEmpty");
    qStatus->setKind(QUEUE::IS_EMPTY);
    send(qStatus, "queueEmptyStatus");
}


void Fifo::handleMessage(cMessage *msg) {
    if (msg->arrivedOn("rxScheduling"))
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
    else if (msg->arrivedOn("itsFuzzyTime"))
    {
        if (total > 0)
        {
            fuzzyError = (errors*100) / total;
        }
        else
            fuzzyError = 0;

//        (char *) msg = ((const char *)fuzzyError);
//        char *a = fuzzyError;
//        a[0] = static_cast<char*>(a);
//        const char *msg = a;
//        cMessage *msgFuzzyErrorNumber = new cMessage(std::to_string(fuzzyError));
//        cMessage *msgFuzzyErrorNumber = new cMessage("problemeBoss");
//        msgFuzzyErrorNumber->setName(msg);
        IntPackage *msg = new IntPackage("fuzzyStuff");
        msg->setFuzzy_value(fuzzyError);
        msg->setError_value(errors);
        msg->setTotal_value(total);
        send(msg, "fuzzyErrorNumber");
    }
    else
    {
    queueClear = queue.isEmpty();

    queue.insert(msg);

        if (queueClear)
        {
        cMessage *msgQueueStatus = new cMessage("queueNotEmpty");
        msgQueueStatus->setKind(QUEUE::IS_NOT_EMPTY);
        send(msgQueueStatus, "queueEmptyStatus");
        }
    }
}

