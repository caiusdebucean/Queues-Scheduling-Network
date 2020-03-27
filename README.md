# Queues-Scheduling-Network
This is an Omnet++ project written in C describing different scheduling algorithms for a network of queues(FIFOs).
This project was realised with **_omnetpp-5.5.1_** Windows version. (Linux distributions may prove problematic)
Code inspiration was provided by existing samples in the omnetpp folder and user manual.

## Describing the system

Each algorithm supposes we have 3 Users, each of which having a queue(FIFO) with package self-generation every *1ms*.
Every User wants to empty its queue or reduce the number of error packages, corresponding to the task at hand.
All packages go to the Sink, which is where they are '_processed_'.
A User cannot send packages to the Sink without having a _'greenlight'_ from the Scheduler. There are 3 types of users, based on their priority: 
1. **premium user** can send 3 packages to the Sink
2. **standard user** can send 2 packages to the Sink and the 
3. **economy user** can send only 1 package to the Sink
_(the above rule is not applicable in the Priority Scheduler Method)_.

![Queue Network](https://i.imgur.com/dadD22m.png)

The Scheduler sends packages to the Users and allows them to get rid of their packages. The order/priority of Users is decided by the _Scheduling Method_ we chose/. The Scheduler sends the packages at a constant period of time, and it recalculates its priority according to the algorithm, being either real-time aware or periodically aware of the users'state.
The generation period and packet transfer period is described in _omnetpp.ini_.
However, due to omnet++ limitations, the **scheduling algorithm** is chosen in the LTE_sched.net file in the _scheduleMethod_ variable.

## Scheduling Methods
### Fuzzy Logic Scheduler
To choose this _algorithm_, modify accordingly:
>scheduleMethod = 2;

This algorithm has an additional factor added to the system: _Packages sent by the users to the Sink have a probability % of sending error packages to the Sink._ This method tries to imitate the behaviour of a Fuzzy Logic Controller, and reduce the factor of _(error packages sent / total packages send)_, by shifting the priority to the 'worst performing' user, at a predetermined _fuzzy period_. This algorithm has at its base the _Weighted Round Robin_ algorithm, to the extent that an updated priority status is given to every user at each _fuzzy period_. When none of the users presents an error factor that passes a predetermined arbitrary threshold, the network keeps its current state of priority and works normally until the _fuzzy system_ is needed again.

### Priority Scheduler
To choose this _algorithm_ modify accordingly:
>scheduleMethod = 1;

This algorithm gives _sending_ priority to the User without an empty Queue, until it empties it. To better understand this, we need different generation periods for the sources of the Users, which should be less frequent than the _scheduling period_ at which the Scheduler takes action.

### Weighted Round Robin Scheduler
To choose this _algorithm_, modify accordingly:
>scheduleMethod = 0;

This works as a normal RR scheduler, with the users having a permanent priority. This algorithm is adaptable, as it takes into consideration if a prioritized user has an empty queue (thus it does not need to use the scheduler's time while other Users can be attended to), and shifts its priority to the next prioritized user until there is someone with a higher priority back _online_.

### Alpha Version Round Robin Scheduler
To choose this _algorithm_, modify accordingly:
>scheduleMethod = 10;

This works as the _Weighted Round Robin Scheduler_ described above, but it is not adaptable in the event of unnecessary priority occupation.

## Running the project
Make sure you have the correct version of omnet++ mentioned above. Choose a scheduling method through the _scheduleMethod_ variable, comment and uncomment the necessary lines in _omnetpp.ini_, and while you have that file window opened and selected, press the _Run simulations_ button.

P.S.: This is a slightly messy project, as there are redundant variables that are passed around and are irrelevant to some algorithm methods, but removing them would not allow the code to be multi-purpose, as omnet++ does not provide a very flexible environment.
