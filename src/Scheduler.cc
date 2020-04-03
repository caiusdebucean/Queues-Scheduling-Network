#include <omnetpp.h>
#include "Utils.h"
#include "IntPackage_m.h"
#include "MultiplePackets_m.h"

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

    int isLAG_threshold;
    int istVeryLAG_threshold;

    int Correct_Decrease, Correct_End;
    int Lag_Increase, Lag_Start, Lag_Decrease, Lag_End;
    int VeryLag_Increase, VeryLag_Start;
    int maxValue;
    int maxId;
    int minValue;
    int minId;
    int middleValue;
    int middleId;
    int throughput;
    int W[3];//W is the number of packages the user will send
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
    void ErrorWeightedRoundRobin(cMessage *msg);
    void ErrorAdaptiveScheduler(cMessage *msg);
    void FuzzyScheduler(cMessage *msg);
    int adjustW(int w_value_packets, int error_value, int remaining_packets);
    bool isCorrect(int value);
    bool isCorrectAndLag(int value);
    bool isLag(int value);
    bool isLagAndVeryLag(int value);
    bool isVeryLag(int value);
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
if(schMethod == 0 || schMethod == 10 || schMethod == 2 || schMethod == 3)
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


isLAG_threshold = 20;
istVeryLAG_threshold = 80;
//fuzzy intervals
Correct_Decrease = 10;
Correct_End = 20;
Lag_Increase = Correct_Decrease;
Lag_Start = Correct_End;
Lag_Decrease = 70;
Lag_End = 80;
VeryLag_Increase = Lag_Decrease;
VeryLag_Start = Lag_End;


throughput = 18;
W[0] = 8;
W[1] = 6;
W[2] = 4;
}

void Scheduler::handleMessage(cMessage *msg) {
if(schMethod == 0)
    RoundRobinScheduler(msg);
else if (schMethod == 1)
    PriorityScheduler(msg);
else if (schMethod == 10)
    AlphaVersionRoundRobinScheduler(msg);
else if (schMethod == 2)
    ErrorWeightedRoundRobin(msg);
else if (schMethod == 3)
    ErrorAdaptiveScheduler(msg);
else if (schMethod == 4)
    FuzzyScheduler(msg);
}

int Scheduler::Maximum3(int a,int b,int c)
{
   if(a>b && a>c)
      return a;
   else if(b>a && b>c)
      return b;
   else
       return c;
}

int Scheduler::Minimum3(int a,int b,int c)
{
    if(a<b && a<c)
       return a;
    else if(b<a && b<c)
       return b;
    else
        return c;
}

bool Scheduler::isCorrect(int value)
    {
    if(value<=Correct_Decrease)
        return true;
    else
        return false;
    }
bool Scheduler::isCorrectAndLag(int value)
    {
    if(value > Lag_Increase && value <= Correct_End)
        return true;
    else
        return false;
    }
bool Scheduler::isLag(int value)
    {
    if(value > Lag_Start && Lag_Decrease <=70)
        return true;
    else
        return false;
    }
bool Scheduler::isLagAndVeryLag(int value)
    {
    if(value > VeryLag_Increase && value <= Lag_End)
        return true;
    else
        return false;
    }
bool Scheduler::isVeryLag(int value)
    {
    if(value > VeryLag_Start)
        return true;
    else
        return false;
    }


int Scheduler::adjustW(int w_value_packets, int error_value, int remaining_packets)
{
int delta_Correct=0, delta_Lag=4, delta_VeryLag=4;
int weight_Correct=0, weight_Lag=0, weight_VeryLag=0;
int W_new=1;

if(isCorrect(error_value))//if is only correct, return the same amount of packages or avaliable packages-1, depending on how many packages are available.
    {
    if(w_value_packets > remaining_packets)
        return (w_value_packets - 1);
    else
        return w_value_packets;
    }
else if(isCorrectAndLag(error_value))
    {
    weight_Correct = (20 - error_value)*10;
    weight_Lag = 100 - weight_Correct;
    W_new = (weight_Correct*(w_value_packets + delta_Correct) + weight_Lag*(w_value_packets + delta_Lag))/100;
    if(W_new > remaining_packets)//conditional to not use all packets
        return (W_new - 1);
    else
        return W_new;
    }
else if(isLag(error_value))
    {
    weight_Lag = 100;
    W_new = weight_Lag*(w_value_packets + delta_Lag)/100;
    if(W_new > remaining_packets)//conditional to not use all packets
        return (W_new - 1);
    else
        return W_new;
    }
else if(isLagAndVeryLag(error_value))
    {
    weight_Lag = (80 - error_value)*10;
    weight_VeryLag = 100 - weight_Correct;

    W_new = (weight_Lag*(w_value_packets + delta_Lag) + weight_VeryLag*(w_value_packets + delta_VeryLag))/100;
    if(W_new > remaining_packets)//conditional to not use all packets
        return (W_new - 1);
    else
        return W_new;
    }
else if(isVeryLag(error_value))
    {
    weight_VeryLag = 100;
    W_new = weight_VeryLag*(w_value_packets + delta_VeryLag)/100;
    if(W_new > remaining_packets)//conditional to not use all packets
        return (W_new - 1);
    else
        return W_new;
    }
else
    return 1;

}
void Scheduler::ErrorAdaptiveScheduler(cMessage *msg)
{
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
    MultiplePackets *userMsg = new MultiplePackets("multiplePackets");
    userMsg->setPacketsToSend(W[0]);
    userMsg->setId(1);
    userMsg->setKind(TYPE::MULTIPLE_PACKETS_FROM_ONE_MSG);
    send(userMsg, "txScheduling", 0);
    scheduleAt(simTime() + schedulingPeriod, standardEvent);
    return;
}
else if (msg == standardEvent){
    MultiplePackets *userMsg = new MultiplePackets("multiplePackets");
    userMsg->setPacketsToSend(W[1]);
    userMsg->setId(2);
    userMsg->setKind(TYPE::MULTIPLE_PACKETS_FROM_ONE_MSG);
    send(userMsg, "txScheduling", 1);
    scheduleAt(simTime() + schedulingPeriod, economyEvent);
    return;
}
else if (msg == economyEvent){
    MultiplePackets *userMsg = new MultiplePackets("multiplePackets");
    userMsg->setPacketsToSend(W[2]);
    userMsg->setId(3);
    userMsg->setKind(TYPE::MULTIPLE_PACKETS_FROM_ONE_MSG);
    send(userMsg, "txScheduling", 2);
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
fuzzyErrors[id] = ((IntPackage*)msg)->getFuzzy_value();
errors[id] = ((IntPackage*)msg)->getError_value();
totals[id] = ((IntPackage*)msg)->getTotal_value();
if (id == 2)
    {
    //get id with maximum error
    maxValue = Maximum3(fuzzyErrors[0],fuzzyErrors[1],fuzzyErrors[2]);
    if(maxValue == fuzzyErrors[0])
        maxId = 0;
    else if (maxValue == fuzzyErrors[1])
        maxId = 1;
    else
        maxId = 2;
    //get id with maximum error

    //get id with minimum error
    minValue = Minimum3(fuzzyErrors[0],fuzzyErrors[1],fuzzyErrors[2]);
    if(minValue == fuzzyErrors[0])
        minId = 0;
    else if (minValue == fuzzyErrors[1])
        minId = 1;
    else
        minId = 2;
    //get id with minimum error
    middleId = 3 - minId - maxId;
    middleValue = fuzzyErrors[middleId];




    int delta_W,delta_bonus; //fuzzy factors
    //WE CALCULATE FOR MOST ERRONATED USER
    if (maxValue > istVeryLAG_threshold)
        {
        delta_bonus = ((100 - maxValue)* W[maxId])/120;
        delta_W =  (maxValue * W[maxId])/100 + delta_bonus;
        if(delta_W == 0)//if by any chance the user sent low number of packages and delta is 0, he still is the most ERRONATED user with critical LAG, so he must get something meaningful
            delta_W = 3;
        W[maxId] = W[maxId] + delta_W;
        if(W[maxId]>(throughput-2)) //if by any chance we give the most ERRONATED user all the throughput, get something back. Each user has to send AT LEAST 1 package.
            W[maxId] = throughput-2;

        }
    else if (maxValue > isLAG_threshold)
        {
        //FUZZY LOGIC ACTION
        //add delta_w, which is fuzzyErrors, to the most ERRONATED/LAGGY user
        delta_W = (maxValue * W[maxId])/100;
        if(delta_W == 0)//if by any chance the user sent low number of packages and delta is 0, he still is the quite LAGGY and the most ERRONATED user, so he must get something meaningful
            delta_W = 2;
        W[maxId] = W[maxId] + delta_W;
        if(W[maxId]>(throughput-2)) //if by any chance we give the most ERRONATED user all the throughput, get something back. Each user has to send AT LEAST 1 package.
            W[maxId] = throughput-2;
        //add delta_w, which is fuzzyErrors, to the most ERRONATED/LAGGY user

        }
    //WE CALCULATE FOR MOST ERRONATED USER


    int remaining_packages;
    remaining_packages = throughput - W[maxId];
    if(middleValue > istVeryLAG_threshold)
        {
        if(W[middleId] < remaining_packages)
            {
            delta_bonus = ((100 - middleValue)* W[middleId])/100;
            delta_W = (fuzzyErrors[middleId] * W[middleId])/100 + delta_bonus;
            if(delta_W == 0)
                delta_W = 1;
            W[middleId] = W[middleId] + delta_W;
            if(W[middleId] > remaining_packages-1)
                W[middleId] = remaining_packages-1;
            W[minId] = remaining_packages - W[middleId];
            }
        else
            {
            W[middleId] = (remaining_packages * 80) /100; // since there are always 2 or more remaining packagages, there will always be one package left for W[minId]
            W[minId] = remaining_packages - W[middleId];
            }
        }
    else if(middleValue > isLAG_threshold)
        {
        if(W[middleId] < remaining_packages)
            {
            delta_W = (fuzzyErrors[middleId] * W[middleId])/100;
            if(delta_W == 0)
                delta_W = 1;
            W[middleId] = W[middleId] + delta_W;
            if(W[middleId] > remaining_packages-1)
                W[middleId] = remaining_packages-1;
            W[minId] = remaining_packages - W[middleId];
            }
        else
            {
            W[middleId] = (remaining_packages * 75) /100;
            W[minId] = remaining_packages - W[middleId];
            }
        }
    //FUZZY LOGIC ACTION
    else if (maxValue > isLAG_threshold)//if there was an error, and the other errors are not big enough, dont consider it ERRONATED or LAGGY anc continue with RR
        {
        if(middleId<minId)
            {
            W[middleId] = (remaining_packages * 67)/100;
            //W[minId] = throughput - W[middleId] - W[maxId];
            W[minId] = remaining_packages - W[middleId];
            }
        else
            {
            W[minId] = (remaining_packages * 67)/100;
            //W[middleId] = throughput - W[minId] - W[maxId];
            W[middleId] = remaining_packages - W[minId];
            }

        //INTERESTING IDEA BUT LETS KEEP WEIGHTED ROUND ROBBIN A THING
        //This keeps a priority even below the LAG threshold
        //W[middleId] = (remaining_packages * 67)/100;
        //W[minId] = remaining_packages - W[middleId];
        //INTERESTING IDEA BUT LETS KEEP WEIGHTED ROUND ROBBIN A THING
        }



    if ((fuzzyErrors[0] <= isLAG_threshold) && (fuzzyErrors[1] <= isLAG_threshold) && (fuzzyErrors[2] <= isLAG_threshold))
        {//COME BACK TO WEIGTHED ROUND ROBBIN
        W[0] = 8;
        W[1] = 6;
        W[2] = 4;
        }


    scheduleAt(simTime() + schedulingPeriod, premiumEvent);
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
        MultiplePackets *userMsg = new MultiplePackets("multiplePackets");
        userMsg->setPacketsToSend(W[0]);
        userMsg->setId(1);
        userMsg->setKind(TYPE::MULTIPLE_PACKETS_FROM_ONE_MSG);
        send(userMsg, "txScheduling", 0);
        scheduleAt(simTime() + schedulingPeriod, standardEvent);
        return;
    }
    else if (msg == standardEvent){
        MultiplePackets *userMsg = new MultiplePackets("multiplePackets");
        userMsg->setPacketsToSend(W[1]);
        userMsg->setId(2);
        userMsg->setKind(TYPE::MULTIPLE_PACKETS_FROM_ONE_MSG);
        send(userMsg, "txScheduling", 1);
        scheduleAt(simTime() + schedulingPeriod, economyEvent);
        return;
    }
    else if (msg == economyEvent){
        MultiplePackets *userMsg = new MultiplePackets("multiplePackets");
        userMsg->setPacketsToSend(W[2]);
        userMsg->setId(3);
        userMsg->setKind(TYPE::MULTIPLE_PACKETS_FROM_ONE_MSG);
        send(userMsg, "txScheduling", 2);
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
    fuzzyErrors[id] = ((IntPackage*)msg)->getFuzzy_value();
    errors[id] = ((IntPackage*)msg)->getError_value();
    totals[id] = ((IntPackage*)msg)->getTotal_value();
    if (id == 2)
        {


        //GET USER ORDER FROM ERRORS
        //get id with maximum error
        maxValue = Maximum3(fuzzyErrors[0],fuzzyErrors[1],fuzzyErrors[2]);
        if(maxValue == fuzzyErrors[0])
            maxId = 0;
        else if (maxValue == fuzzyErrors[1])
            maxId = 1;
        else
            maxId = 2;
        //get id with maximum error

        //get id with minimum error
        minValue = Minimum3(fuzzyErrors[0],fuzzyErrors[1],fuzzyErrors[2]);
        if(minValue == fuzzyErrors[0])
            minId = 0;
        else if (minValue == fuzzyErrors[1])
            minId = 1;
        else
            minId = 2;
        //get id with minimum error
        middleId = 3 - minId - maxId;
        middleValue = fuzzyErrors[middleId];
        //GET USER ORDER FROM ERRORS




        int delta_W,delta_bonus; //fuzzy factors
        int remaining_packages;

        W[maxId] = adjustW(W[maxId], maxValue, throughput);

        remaining_packages = throughput - W[maxId];
        W[middleId] = adjustW(W[middleId], middleValue, remaining_packages);

        remaining_packages = throughput - W[maxId] - W[middleId];
        W[minId] = remaining_packages;

        if ((fuzzyErrors[0] <= Correct_Decrease) && (fuzzyErrors[1] <= Correct_Decrease) && (fuzzyErrors[2] <= Correct_Decrease))
            {//COME BACK TO WEIGTHED ROUND ROBBIN if everything is correct
            W[0] = 8;
            W[1] = 6;
            W[2] = 4;
            }


        scheduleAt(simTime() + schedulingPeriod, premiumEvent);
        }

}



void Scheduler::ErrorWeightedRoundRobin(cMessage *msg){
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
