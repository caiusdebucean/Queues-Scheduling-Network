#include <omnetpp.h>
#include "Utils.h"
#include "IntPackage_m.h"


#define SIMTIME_DBL(t) ((t).dbl())


using namespace omnetpp;

class Scheduler : public cSimpleModule {
private:
    int gateSize;
    int priorities[3];
    int queueStatus[3];
    double schedulingPeriod;
    cMessage *premiumEvent;
    cMessage *standardEvent;
    cMessage *economyEvent;
    cMessage *sendEvent;
    int counterPremium;
    int counterStandard;
    int counterEconomy;
    int i;
    int id;
    cPacket *package;
    int schMethod;
    int errors[3];
    int totals[3];
    int fuzzyErrors[3];
    cGate *arrivalFuzzyGate;
    double timed_current;
    SimTime currentTime;
    SimTime oldTime;
    int currentPriority;
    int oldPriority;

    int tmp_id;
    int first;
    int second;
    int third;
    int premium_glitch;
    int error_threshold;

public:
    Scheduler();
    virtual ~Scheduler();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void sendMessage(int userClass);
    void scheduleSelf(int userClass);
    void cancelSchedule(int userClass);
    void PriorityScheduler(cMessage *msg);
    void RoundRobinScheduler(cMessage *msg);
    void AlphaVersionRoundRobinScheduler(cMessage *msg);
    void FuzzyScheduler(cMessage *msg);
    int Maximum3(int a,int b,int c);
    int Minimum3(int a,int b,int c);
};

Define_Module(Scheduler);

Scheduler::Scheduler() {
gateSize = 3;
schedulingPeriod = 0;
premiumEvent = nullptr;
standardEvent = nullptr;
economyEvent = nullptr;
sendEvent = nullptr;
}

Scheduler::~Scheduler() {

}

void Scheduler::initialize() {
schedulingPeriod = par("schedulingPeriod").doubleValue();

schMethod = par("scheduleMethod").intValue();

premiumEvent = new cMessage("premium");
standardEvent = new cMessage("standard");
economyEvent = new cMessage("economy");
error_threshold = 41;
if(schMethod == 0 || schMethod == 10 || schMethod == 2)
    scheduleAt(simTime(), premiumEvent);
else
    scheduleSelf(0);

counterPremium = 0;
counterStandard = 0;
counterEconomy = 0;

queueStatus[0] = 0;
queueStatus[1] = 0;
queueStatus[2] = 0;

currentPriority = 0;
oldPriority = 0;

id = -1;
tmp_id = 0;

premium_glitch = 0;

first = 0;
second = 1;
third = 2;
}

void Scheduler::handleMessage(cMessage *msg) {
if(schMethod == 0)
    RoundRobinScheduler(msg);
else if (schMethod == 1)
    PriorityScheduler(msg);
else if (schMethod == 10)
    AlphaVersionRoundRobinScheduler(msg);
else if (schMethod == 2)
    FuzzyScheduler(msg);
}

int Scheduler::Maximum3(int a,int b,int c)
{
   if(a>b)
   {
      if(a>c)
      {
         return a;
      }
   }
   else if(b>a)
   {
      if(b>c)
      {
         return b;
      }
   }
   else if(c>b)
   {
    if(c>a)
      {
        return c;
      }
   }
}

int Scheduler::Minimum3(int a,int b,int c)
{
   if(a<b)
   {
      if(a<c)
      {
         return a;
      }
   }
   else if(b<a)
   {
      if(b<c)
      {
         return b;
      }
   }
   else if(c<b)
   {
    if(c<a)
      {
        return c;
      }
   }
}

void Scheduler::FuzzyScheduler(cMessage *msg){
    if (msg->arrivedOn("queueEmpty",0))
            {
            cancelAndDelete(msg);
            return;
            }
    else if (msg->arrivedOn("queueEmpty",1))
            {
            cancelAndDelete(msg);
            return;
            }
    else if (msg->arrivedOn("queueEmpty",2))
            {
            cancelAndDelete(msg);
            return;
            }

    if (msg == premiumEvent){
            sendEvent = new cMessage("premium");
            send(sendEvent, "txScheduling", first);
            if (premium_glitch != 3)
                counterPremium++;
            else
                premium_glitch++;
            if(counterPremium % 3 == 0)
                scheduleAt(simTime() + schedulingPeriod, standardEvent);
            else
                scheduleAt(simTime() + schedulingPeriod, premiumEvent);
            return;
    }
    else if (msg == standardEvent){
        sendEvent = new cMessage("standard");
        send(sendEvent, "txScheduling", second);
        counterStandard++;
        if(counterStandard % 2 == 0)
            scheduleAt(simTime() + schedulingPeriod, economyEvent);
        else
            scheduleAt(simTime() + schedulingPeriod, standardEvent);
        return;

    }
    else if (msg == economyEvent){
        sendEvent = new cMessage("economy");
        send(sendEvent, "txScheduling", third);
        scheduleAt(simTime() + schedulingPeriod, premiumEvent);
        return;
    }
    else if(msg->arrivedOn("fuzzyErrorNumber",0))
            id = 0;
    else if(msg->arrivedOn("fuzzyErrorNumber",1))
            id = 1;
    else if(msg->arrivedOn("fuzzyErrorNumber",2))
            id = 2;
    else
        {

        cancelAndDelete(msg);
        return;
        }
//    fuzzyErrors[id] = msg->findPar("fuzzy_value");
//    errors[id] = msg->findPar("error_value");
//    totals[id] = msg->findPar("total_value");
//    IntPackage *msg1 = msg;
    premium_glitch++;
    fuzzyErrors[id] = ((IntPackage*)msg)->getFuzzy_value();
    errors[id] = ((IntPackage*)msg)->getError_value();
    totals[id] = ((IntPackage*)msg)->getTotal_value();

    if ((fuzzyErrors[first] > 41) || (fuzzyErrors[second] > 41) || (fuzzyErrors[third] > 41))
        {
        if ((fuzzyErrors[0] > fuzzyErrors[1]) && (fuzzyErrors[0] > fuzzyErrors[2]))
            first = 0;
        else if ((fuzzyErrors[1] > fuzzyErrors[0]) && (fuzzyErrors[1] > fuzzyErrors[2]))
            first = 1;
        else if ((fuzzyErrors[2] > fuzzyErrors[0]) && (fuzzyErrors[2] > fuzzyErrors[1]))
            first = 2;

        if ((fuzzyErrors[0] < fuzzyErrors[1]) && (fuzzyErrors[0] < fuzzyErrors[2]))
            third = 0;
        else if ((fuzzyErrors[1] < fuzzyErrors[0]) && (fuzzyErrors[1] < fuzzyErrors[2]))
            third = 1;
        else if ((fuzzyErrors[2] < fuzzyErrors[0]) && (fuzzyErrors[2] < fuzzyErrors[1]))
            third = 2;

//    if ((fuzzyErrors[first] > 60) || (fuzzyErrors[second] > 60) || (fuzzyErrors[third] > 60))
//        {
//        first = Maximum3(fuzzyErrors[0],fuzzyErrors[1],fuzzyErrors[2]);
//        third = Minimum3(fuzzyErrors[0],fuzzyErrors[1],fuzzyErrors[2]);
        second = 3 - (first + third);
//        //IF EQUAL ASSIGN ACCORDING TO WHO HAS MORE ERRORS
//        if(fuzzyErrors[first] == fuzzyErrors[second])
//            {
//            int tmp;
//            if (totals[second] > totals[first])
//                {
//                tmp = first;
//                first = second;
//                second = tmp;
//                }
//            }
//        if(fuzzyErrors[second] == fuzzyErrors[third])
//                {
//                int tmp;
//                if (totals[third] > totals[second])
//                    {
//                    tmp = second;
//                    second = third;
//                    third = tmp;
//                    }
//                }
//        //IF EQUAL ASSIGN ACCORDING TO WHO HAS MORE ERRORS
        }
    else
    {
        first = 0;
        second = 1;
        third = 2;
    }
}


void Scheduler::PriorityScheduler(cMessage *msg){
    oldPriority = currentPriority;
    if(msg->arrivedOn("fuzzyErrorNumber",0))
        {
        cancelAndDelete(msg);
        return;
        }
    else if(msg->arrivedOn("fuzzyErrorNumber",1))
        {
        cancelAndDelete(msg);
        return;
        }
    else if(msg->arrivedOn("fuzzyErrorNumber",2))
        {
        cancelAndDelete(msg);
        return;
        }


    //DACA PRIMIM MESAJ PE PRIMUL Q-status
    if(msg->arrivedOn("queueEmpty",0)  && (msg->getKind() == QUEUE::IS_NOT_EMPTY)){
        currentPriority = 1;
        EV << "Current priority is "<<currentPriority;
        queueStatus[0] = 1;
    }
    if(msg->arrivedOn("queueEmpty",0)  && (msg->getKind() == QUEUE::IS_EMPTY)){
        queueStatus[0] = 0;
        if(queueStatus[1] == 1)
            currentPriority = 2;
        else if(queueStatus[2] == 1)
            currentPriority = 3;
        else
            currentPriority = 0;
        EV << "Current priority is "<<currentPriority;
    }
    //DACA PRIMIM MESAJ PE PRIMUL Q-status
    ////////////////////////////////////////////////////
    //DACA PRIMIM MESAJ PE AL DOILEA Q-status
    if(msg->arrivedOn("queueEmpty",1)  && (msg->getKind() == QUEUE::IS_NOT_EMPTY)){
        if(currentPriority != 1 && queueStatus[0] == 0)
            currentPriority = 2;

        queueStatus[1] = 1;
        EV << "Current priority is "<<currentPriority;

    }

    if(msg->arrivedOn("queueEmpty",1)  && (msg->getKind() == QUEUE::IS_EMPTY)){
        queueStatus[1] = 0;
        if(queueStatus[0] == 1)
            currentPriority = 1;
        else if(queueStatus[2] == 1)
            currentPriority = 3;
        else
            currentPriority = 0;
        EV << "Current priority is "<<currentPriority;
    }
    //DACA PRIMIM MESAJ PE AL DOILEA Q-status
    ////////////////////////////////////////////////////
    //DACA PRIMIM MESAJ PE AL TREILEA Q-status
    if(msg->arrivedOn("queueEmpty",2)  && (msg->getKind() == QUEUE::IS_NOT_EMPTY)){
        if(currentPriority != 1 && currentPriority != 2 && queueStatus[0] == 0 && queueStatus[1] == 0)
            currentPriority = 3;
        queueStatus[2] = 1;
        EV << "Current priority is "<<currentPriority;
    }

    if(msg->arrivedOn("queueEmpty",2)  && (msg->getKind() == QUEUE::IS_EMPTY)){
        queueStatus[2] = 0;
        if(queueStatus[0] == 1)
            currentPriority = 1;
        else if(queueStatus[1] == 1)
            currentPriority = 2;
        else
            currentPriority = 0;
        EV << "Current priority is "<<currentPriority;
    }
    //DACA PRIMIM MESAJ PE AL TREILEA Q-status


    if(currentPriority == 0)
        cancelAndDelete(msg);
    else if (msg->arrivedOn("user_priorities",0))
        cancelAndDelete(msg);
    else if (msg->arrivedOn("user_priorities",1))
        cancelAndDelete(msg);
    else if (msg->arrivedOn("user_priorities",2))
        cancelAndDelete(msg);
    else if (msg->arrivedOn("queueEmpty",0))
        cancelAndDelete(msg);
    else if (msg->arrivedOn("queueEmpty",1))
        cancelAndDelete(msg);
    else if (msg->arrivedOn("queueEmpty",2))
        cancelAndDelete(msg);
    else
    {
        sendMessage(currentPriority);
        scheduleSelf(0);

    }
    currentTime = simTime();



    if(oldPriority == 0 && currentPriority!=0 && (currentTime!=oldTime))
        scheduleSelf(0);

    if(currentTime != 0){
        oldTime = simTime() - schedulingPeriod;
    }
    //EV << "\nOld Time is "<<oldTime<<" and Current Time is "<<currentTime;
    //EV << "Old priority is "<<oldPriority<< " and Current priority is "<<currentPriority;
    oldPriority = currentPriority;
}

void Scheduler::AlphaVersionRoundRobinScheduler(cMessage *msg){
    if (msg == premiumEvent){
        sendEvent = new cMessage("premium");
        send(sendEvent, "txScheduling", 0);
        counterPremium++;
        if(counterPremium % 3 == 0)
            scheduleAt(simTime() + schedulingPeriod, standardEvent);
        else
            scheduleAt(simTime() + schedulingPeriod, premiumEvent);
    }
    else if (msg == standardEvent){
        sendEvent = new cMessage("standard");
        send(sendEvent, "txScheduling", 1);
        counterStandard++;
        if(counterStandard % 2 == 0)
            scheduleAt(simTime() + schedulingPeriod, economyEvent);
        else
            scheduleAt(simTime() + schedulingPeriod, standardEvent);

    }
    else if (msg == economyEvent){
        sendEvent = new cMessage("economy");
        send(sendEvent, "txScheduling", 2);
        scheduleAt(simTime() + schedulingPeriod, premiumEvent);
    }
}

void Scheduler::RoundRobinScheduler(cMessage *msg){
    if (msg == premiumEvent){
        sendEvent = new cMessage("premium");
        send(sendEvent, "txScheduling", 0);
        counterPremium++;
        if(counterPremium % 3 == 0)
            scheduleAt(simTime() + schedulingPeriod, standardEvent);
        else
            scheduleAt(simTime() + schedulingPeriod, premiumEvent);
    }
    else if (msg == standardEvent){
        sendEvent = new cMessage("standard");
        send(sendEvent, "txScheduling", 1);
        counterStandard++;
        if(counterStandard % 2 == 0)
            scheduleAt(simTime() + schedulingPeriod, economyEvent);
        else
            scheduleAt(simTime() + schedulingPeriod, standardEvent);

    }
    else if (msg == economyEvent){
        sendEvent = new cMessage("economy");
        send(sendEvent, "txScheduling", 2);
        scheduleAt(simTime() + schedulingPeriod, premiumEvent);
    }

    if(msg->arrivedOn("queueEmpty",0) && (msg->getKind() == QUEUE::IS_EMPTY))
        {
        if(counterPremium % 3 == 1)
            counterPremium++;
        else if(counterPremium % 3 == 0)
            counterPremium+=2;
        }
    if(msg->arrivedOn("queueEmpty",1)  && (msg->getKind() == QUEUE::IS_EMPTY))
        {
        if(counterStandard % 2 == 0)
            counterStandard++;;
        }
    }

void Scheduler::sendMessage(int userClass){
    if(userClass == 1){
        sendEvent = new cMessage("premium");
        send(sendEvent, "txScheduling", 0);
    }
    else if(userClass == 2){
            sendEvent = new cMessage("standard");
            send(sendEvent, "txScheduling", 1);
        }
    else if(userClass == 3){
            sendEvent = new cMessage("economy");
            send(sendEvent, "txScheduling", 2);
        }

}

void Scheduler::scheduleSelf(int userClass){
    if(userClass == 1){
        sendEvent = new cMessage("premium");
        scheduleAt(simTime() + schedulingPeriod, premiumEvent);
    }
    else if(userClass == 2){
            sendEvent = new cMessage("standard");
            scheduleAt(simTime() + schedulingPeriod, standardEvent);
        }
    else if(userClass == 3){
            sendEvent = new cMessage("economy");
            scheduleAt(simTime() + schedulingPeriod, economyEvent);
        }
    else if(userClass == 0){
        sendEvent = new cMessage("empty");
        scheduleAt(simTime() + schedulingPeriod, sendEvent);
    }

}

void Scheduler::cancelSchedule(int userClass){
    if(userClass == 1){
        sendEvent = new cMessage("premium");
        cancelEvent(sendEvent);
    }
    else if(userClass == 2){
            sendEvent = new cMessage("standard");
            cancelEvent(sendEvent);
        }
    else if(userClass == 3){
            sendEvent = new cMessage("economy");
            cancelEvent(sendEvent);
        }
    else if(userClass == 0){
        sendEvent = new cMessage("empty");
        cancelEvent(sendEvent);
    }

}
