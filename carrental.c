#include "simlib.h"		/* Required for use of simlib.c. */

#define EVENT_ARRIVAL_1					1	 /* Event type for arrival of a job to terminal 1. */
#define EVENT_ARRIVAL_2					2	 /* Event type for arrival of a job to terminal 2. */
#define EVENT_ARRIVAL_3					3	 /* Event type for arrival of a job to car rental. */
#define EVENT_DEPARTURE_BUS			4	 /* Event type for departure of a bus. */
#define EVENT_ARRIVE_BUS				5	 /* Event type for arrival of a bus. */
#define EVENT_LOAD				      6	 /* Event type for loading passengers from a particular station. */
#define EVENT_UNLOAD				    7	 /* Event type for unloading passengers to a particular station. */
#define EVENT_END_SIMULATION		8	 /* Event type for end of the simulation. */
#define STREAM_INTERARRIVAL_1		1	 /* Random-number stream for interarrivals at terminal 1. */
#define STREAM_INTERARRIVAL_2		2	 /* Random-number stream for interarrivals at terminal 2. */
#define STREAM_INTERARRIVAL_3		3	 /* Random-number stream for interarrivals at car rental. */
#define STREAM_UNLOADING				4	 /* Random-number stream for unloading time. */
#define STREAM_LOADING					5	 /* Random-number stream for loading time. */
#define STREAM_DESTINATION_ARV	6	 /* Random-number stream for selecting destination from car rental. */
#define MAX_NUM_STATIONS				3	 /* Maximum number of stations. */
#define MAX_NUM_BUS							1	 /* Maximum number of bus. */
#define MAX_NUM_SEATS						20 /* Maximum number of seats. */

int bus_position, bus_moving, capacity, waiting_time;
double arrive_time_b;
FILE *infile, *outfile;

void arrive_b(void){
  int init = bus_position;
  bus_moving = 0;

  if(bus_position != 3){
    bus_position++;
  }
  else{
    bus_position = 1;
  }

  sampst(sim_time - arrive_time_b - dist[init][bus_position], 1);
  
  arrive_time_b = sim_time;
  unload();
  load();
}

void move_b(void){
  int init = bus_position;
  int dest;
  bus_moving = 1;
  
  if(bus_position != 3){
    dest = bus_position + 1;
  }
  else{
    dest = 3;
  }

  event_schedule(sim_time+dist[init][dest], EVENT_ARRIVE_BUS);
}

void unload() {
  int destination = bus_position;

  if (list_size[MAX_NUM_STATIONS + destination] > 0) {
    list_remove(FIRST, MAX_NUM_STATIONS + destination);
    ++capacity;
    event_schedule(sim_time + uniform(16,24,STREAM_UNLOADING), EVENT_UNLOAD);
  }
  else {
    load();
  }
}

void load() {
  int terminal = bus_position;
  int time_at_position = sim_time - arrive_time_b;

  if (list_size[terminal] > 0 && capacity > 0) {
    list_remove(FIRST, terminal);
    destination = transfer[1]; //destination yg ditentukan saat arrive
    list_file(LAST, MAX_NUM_STATIONS + destination);
    --capacity;
    event_scheduleo(sim_time + uniform(15,25,STREAM_LOADING), EVENT_LOAD);
  }
  else {
    if (time_at_position >= waiting_time) {
      move_b();
    }
    else {
      event_schedule(sim_time + (waiting_time - time_at_position), EVENT_DEPARTURE_BUS);

      // belum menangani kasus penumpang datang setelah loading selesai saat time_at_position < waiting_time
    }
  }
}

int main(){
  capacity = 20;
  waiting_time = 5 * 60; //5 minutes
}

