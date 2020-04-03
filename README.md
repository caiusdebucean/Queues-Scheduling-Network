# Queues-Scheduling-Network
This is an Omnet++ project written in C++ describing different scheduling algorithms for a network of queues(FIFOs).
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

The Scheduler sends packages to the Users and allows them to get rid of their packages. The order/priority of Users is decided by the _Scheduling Method_ we chose. The Scheduler sends the packages at a constant period of time, and it recalculates its priority (and number of packgages a user can send) according to the algorithm, being either real-time aware or periodically aware of the users'state.
The generation period and packet transfer period is described in _omnetpp.ini_.
However, due to omnet++ limitations, the **scheduling algorithm** is chosen in the LTE_sched.net file in the _scheduleMethod_ variable.

## Scheduling Methods
### Fuzzy Logic Scheduler

To choose this _algorithm_, modify accordingly:
>scheduleMethod = 4;

This method uses the same **rules** as the _Error Adaptive Scheduler_, but uses different intervals, and each interval has a linear decrease for the last 10 units of it. This algorithm uses **Fuzzy Logic**. For calculating the package allocation for each user, a _weight_ system is used. That means each time the algorithm needs to decide how many packages a user can send, a combination of _1 or 2 complementary weights_ are multiplied with the *old package allocation number* and the _interval delta_, depending in which **intervals** the error is. The _interval delta_ is a number of aditional packages, varying from each interval. This is the graph of the fuzzy logic:

![Queue Network](https://i.imgur.com/9iosRwF.png)

This is an example of the calculations for the number of packages to send by a user. The first example, _**point A**_, shows a case where it's both *correct* and *laggy*. This means that for calculating the required number, a weighted **sum** of the **products** between **interval deltas** and **previously sent packages**. For _**point B**_, the case is only _very laggy_, so the weights and deltas for the _correct_ and _laggy_ intervals are 0.

![Queue Network](https://i.imgur.com/HNSxs9Z.png)

The system options can be uncommented in the _.ini_ file.

### Error Adaptive Scheduler
To choose this _algorithm_, modify accordingly:
>scheduleMethod = 3;

This method is applied to a system with the following rules:
* Users have a 20% chance of sending an **ERROR** package
* Scheduling period is 20ms
* During one *Scheduling period*, a **max** of 18 packages can be sent by all _Users_ combined
* Each _User_ sends **at least 1** package per *Scheduling period*. This means that a single _User_ cand send a **max of 16 packages**, so that the others can send
* Each _User_ start sending at the same time, for the sake of easy observation
* All _error rates (or LAGs)_ reset at each _Scheduling period_

These said, this method has 3 intervals of acction, depending on the _error rate_  during an interval. The intervals are: _[0,20) - correct; [20,80) - laggy; [80,100] - very laggy_. The action graph is:

![Queue Network](https://i.imgur.com/x4M3vyN.png)

The user with the highest _LAG (error rate)_, will be modified by a _delta_ factor, depending on the interval it falls into. The final allocation is limited by the system's rule (e.g. if a User is _very laggy_ and tries to get more resources than possible - 16 - then hard limit it). The same is done for the rest of the _Users_, except the last one - which gets allocated what's left.
 Over an interval, the number of additional packages is varying, so there are no pre-programmed numbers. The only exception is the first case, where _if all users are correct_ - then we apply the classic _Weighted Round Robin_ algorithm.


### Error-Weighted Round Robin Scheduler
To choose this _algorithm_, modify accordingly:
>scheduleMethod = 2;

This algorithm has an additional factor added to the system: _Packages sent by the users to the Sink have a probability % of sending error packages to the Sink._ This method tries to imitate the behaviour of a Weight-Varying Round Robin Scheduler, and reduce the factor of _(error packages sent / total packages send)_, by shifting the priority to the 'worst performing' user, at a predetermined _scheduling period_. This algorithm has at its base the _Weighted Round Robin_ algorithm, to the extent that an updated priority status is given to every user at each _scheduling period_. When none of the users presents an error factor that passes a predetermined arbitrary threshold, the network keeps its current state of priority and works normally until the _error-weighted meethod_ is needed again.


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

## Understanding the simulations
![Alt Text](https://media.giphy.com/media/d7TYfGxcqzrcwk1K02/giphy.gif)

The above gif represents a rundown of the *Error-Weighted Round Robin Scheduler*. Here is some explanation:

![Simulation explanation](https://i.imgur.com/HWYThPE.png)
**TO BE MENTIONED: The above method is not a Fuzzy Logic Controller. It has fuzzy-names because the code used for this method was meant to be a FLC, but later repurposed into the Error-Weighted Round Robin Scheduler. This will be patched in a future version.**


The other scheduling algorithms are more straight forward and make use of the _emptyQueue_, which transmits the queue status.

P.S.: This is a slightly messy project, as there are redundant variables that are passed around and are irrelevant to some algorithm methods, but removing them would not allow the code to be multi-purpose, as omnet++ does not provide a very flexible environment. Also, because of developing new Scheduling methods, some nomenclature is confusing between methods. This will be corrected on the way.

<div>&copy;Debucean Caius-Ioan @ <b>github.com/caiusdebucean</b></div>

