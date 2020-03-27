#include <omnetpp.h>
#include "Utils.h"

using namespace omnetpp;

class Source : public cSimpleModule
{
  private:
    cMessage *sendMessageEvent;
    double generatorPeriod;
    cMessage *priorityMsg;
    int temp_priority;
  public:
    Source();
    virtual ~Source();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Source);

Source::Source()
{
    sendMessageEvent = nullptr;
}

Source::~Source()
{
    cancelAndDelete(sendMessageEvent);
}

void Source::initialize()
{
    generatorPeriod = par("generatorPeriod").doubleValue();
    sendMessageEvent = new cMessage("sendMessageEvent");
    scheduleAt(simTime(), sendMessageEvent);

    temp_priority = par("priority").intValue();
    priorityMsg = new cMessage("priority");

    if(temp_priority == 1)
        priorityMsg->setKind(PRIORITY::PREMIUM);
    else if (temp_priority == 2)
        priorityMsg->setKind(PRIORITY::STANDARD);
    else
        priorityMsg->setKind(PRIORITY::ECONOMY);

    send(priorityMsg, "userPriority");
}

void Source::handleMessage(cMessage *msg)
{
    ASSERT(msg == sendMessageEvent);

    cMessage *pachet = new cMessage("pachet");
    send(pachet, "txPackets");
    scheduleAt(simTime() + generatorPeriod, sendMessageEvent);
}
