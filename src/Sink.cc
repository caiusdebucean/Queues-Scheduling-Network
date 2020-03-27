#include <omnetpp.h>

using namespace omnetpp;

class Sink : public cSimpleModule {
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Sink);

void Sink::initialize() {
}

void Sink::handleMessage(cMessage *msg) {
    cancelAndDelete(msg);
}
