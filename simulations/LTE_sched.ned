//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package fifo_scheduler_network.simulations;

import fifo_scheduler_network.Scheduler;
import fifo_scheduler_network.User;
import fifo_scheduler_network.Sink;

//package LTE_Scheduler;

network LTE_Scheduler
{
parameters:
int usersCount = default(3);

submodules:
scheduler: Scheduler {
gateSize = 3;

scheduleMethod = 2;

@display("p=100,100");
}
sink: Sink {
gateSize = 3;
@display("p=800, 100");
}

User_1: User {
pr = 1;
@display("p=300, 30");
}
User_2: User {
pr = 2;
@display("p=300, 100");
}
User_3: User {
pr = 3;
@display("p=300, 170");
}

connections: 
    scheduler.txScheduling[0] --> User_1.rxScheduling;
    scheduler.txScheduling[1] --> User_2.rxScheduling;
    scheduler.txScheduling[2] --> User_3.rxScheduling;
    
    User_1.txPackets --> sink.rxPackets[0];
    User_2.txPackets --> sink.rxPackets[1];
    User_3.txPackets --> sink.rxPackets[2];
    
    User_1.userPriority --> scheduler.user_priorities[0];
    User_2.userPriority --> scheduler.user_priorities[1];
    User_3.userPriority --> scheduler.user_priorities[2];
    
    User_1.queueEmptyStats --> scheduler.queueEmpty[0];
    User_2.queueEmptyStats --> scheduler.queueEmpty[1];
    User_3.queueEmptyStats --> scheduler.queueEmpty[2];
    
    User_1.fuzzyErrorNumber --> scheduler.fuzzyErrorNumber[0];
    User_2.fuzzyErrorNumber --> scheduler.fuzzyErrorNumber[1];
    User_3.fuzzyErrorNumber --> scheduler.fuzzyErrorNumber[2];

}
