#include <omnetpp.h>
#include "Utils.h"

using namespace omnetpp;

class User : public cSimpleModule {
private:

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(User);

void User::initialize() {


}

void User::handleMessage(cMessage *msg) {
    EV << "I have received on " << msg->getArrivalGate() << " the following message:\n" << msg << "\n";

}
