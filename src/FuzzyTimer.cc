#include <omnetpp.h>
#include "Utils.h"

using namespace omnetpp;

class FuzzyTimer : public cSimpleModule
{
  private:
    cMessage *sendMessageEvent;
    double fuzzyPeriod;

  public:
    FuzzyTimer();
    virtual ~FuzzyTimer();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(FuzzyTimer);

FuzzyTimer::FuzzyTimer()
{
    sendMessageEvent = nullptr;
}

FuzzyTimer::~FuzzyTimer()
{
    cancelAndDelete(sendMessageEvent);
}

void FuzzyTimer::initialize()
{
    fuzzyPeriod = par("fuzzyPeriod").doubleValue();
    sendMessageEvent = new cMessage("prepareForFuzzy");
    scheduleAt(simTime(), sendMessageEvent);

}

void FuzzyTimer::handleMessage(cMessage *msg)
{
    ASSERT(msg == sendMessageEvent);

    cMessage *pachet = new cMessage("itsFuzzyTime");
    pachet->setKind(TYPE::FUZZY_TIMER);
    send(pachet, "itsFuzzyTime");
    scheduleAt(simTime() + fuzzyPeriod, sendMessageEvent);
}
