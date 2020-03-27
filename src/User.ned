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

package fifo_scheduler_network;


import fifo_scheduler_network.Source;
import fifo_scheduler_network.Fifo;
import fifo_scheduler_network.FuzzyTimer;

module User
{
    parameters:
    	@display("i=block/user");
    	int pr;
    	
    @display("bgb=90,160");
    
   	gates:
   	    input rxScheduling;
    	output txPackets;
    	output userPriority;
    	output queueEmptyStats;
    	output fuzzyErrorNumber;
    	
    submodules:
        generator: Source {
            @display("p=40,40");
            priority = pr;
        }
		
        queue: Fifo {
            @display("p=40,120");
        }
        
        fuzzy_timer: FuzzyTimer {
        	@display("p=40,20");
        }
   	connections:
   	    rxScheduling --> queue.rxScheduling;
   	    generator.txPackets --> queue.rxPackets;
   	    queue.txPackets --> txPackets;
   	    queue.queueEmptyStatus --> queueEmptyStats;
   	    queue.fuzzyErrorNumber --> fuzzyErrorNumber;
   	    generator.userPriority --> userPriority;
   	    fuzzy_timer.itsFuzzyTime --> queue.itsFuzzyTime;
   	    
}
