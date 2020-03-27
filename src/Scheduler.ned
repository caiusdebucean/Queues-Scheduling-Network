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

//
// TODO auto-generated module
//
simple Scheduler
{
    parameters:
        @display("i=block/dispatch");
        volatile double schedulingPeriod @unit(ms) = default(1ms);
        int gateSize;
        int scheduleMethod;
        
		
    gates:
        input queueEmpty[gateSize];
        input fuzzyErrorNumber[gateSize];
        input user_priorities[gateSize];
        output txScheduling[gateSize];
}
