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
#define VAR_QUEUE_STATION       0  /* Zero index of statistic variable for queue in station 1/2/3 */
#define VAR_BUS_AT_STATION      3  /* Zero index of statistic variable for bus stop at station 1/2/3 */
#define VAR_PERSON_FROM_STATION 6  /* Zero index of statistic variable for person arrive at station 1/2/3 */
#define VAR_BUS                 10 /* Statistic variable for bus */

int bus_position, bus_moving, capacity, num_stations, num_seats, i, j, bus_idle, looping;
double waiting_time, arrive_time_b, mean_interarrival[MAX_NUM_STATIONS + 1], length_simulation, prob_distrib_dest[3], dist[MAX_NUM_STATIONS+1][MAX_NUM_STATIONS+1], loop_ori, loop_final, speed;
FILE *infile, *outfile;

void move_b(){
  int init = bus_position;
  int dest;
  bus_moving = 1;
  bus_idle = 0;
  
  if(bus_position != 3){
    dest = bus_position + 1;
  }
  else{
    dest = 3;
    // report time bus spent on loop here
  }
  fprintf (outfile, "Bus moving after%0.3f\n", sim_time - arrive_time_b);  

  sampst(sim_time - arrive_time_b, VAR_BUS_AT_STATION + bus_position); 
  event_schedule(sim_time + (dist[init][dest]), EVENT_ARRIVE_BUS);
  fprintf (outfile, "Ev move bus%21d\n", bus_position);
}

void load() {
  int arrival_time, origin, destination;
  int terminal = bus_position;
  double time_at_position = sim_time - arrive_time_b;
  
  bus_idle = 0;
  // fprintf (outfile, "time at position %0.3f\n",time_at_position);
  if (list_size[terminal] > 0 && capacity > 0) {
    list_remove(FIRST, terminal);
    arrival_time = transfer[1];
    destination = transfer[3]; // destination yg ditentukan saat arrive
    
    list_file(LAST, MAX_NUM_STATIONS + destination);

    --capacity;
    timest(MAX_NUM_SEATS - capacity, VAR_BUS); // report changing number on the bus

    sampst(sim_time - arrival_time, VAR_QUEUE_STATION + terminal); // report delay time queue in this station

    event_schedule(sim_time + uniform(0.0041667,0.0069444,STREAM_LOADING), EVENT_LOAD);
    fprintf (outfile, "Ev load%21d\n",capacity);
  }
  else {
    if (time_at_position >= waiting_time) {
      move_b();
    }
    else {
      event_schedule(sim_time + (waiting_time - time_at_position), EVENT_DEPARTURE_BUS);

      bus_idle = 1;
      fprintf (outfile, "remaining time %0.8f\n",waiting_time - time_at_position);
    }
  }
}

void unload() {
  int destination = bus_position;
  int origin;
  double arrival_time;

  if (list_size[MAX_NUM_STATIONS + destination] > 0) {
    list_remove(FIRST, MAX_NUM_STATIONS + destination);
    arrival_time = transfer[1];
    origin = transfer[2];

    ++capacity;
    timest(MAX_NUM_SEATS - capacity, VAR_BUS); // report changing number on the bus

    sampst(sim_time - arrival_time, VAR_PERSON_FROM_STATION + origin); // report time spent per person for each origin station

    fprintf (outfile, "Ev unload%19d\n",capacity);

    if (list_size[MAX_NUM_STATIONS + destination] > 0) {
      event_schedule(sim_time + uniform(0.00444,0.00667,STREAM_UNLOADING), EVENT_UNLOAD);
    }
    else {
      event_schedule(sim_time + uniform(0.00444,0.00667,STREAM_UNLOADING), EVENT_LOAD);
    }
  }
  else {
    load();
  }
}

void arrive (int new_job, int station) 
{
	int destination;

	if (station == 1)
	{
		event_schedule(sim_time + expon (mean_interarrival[1], STREAM_INTERARRIVAL_1), EVENT_ARRIVAL_1);
		fprintf (outfile, "Ev arrival 1\n");
    destination = 3;
	}
	else if (station == 2)
	{
		event_schedule(sim_time + expon (mean_interarrival[2], STREAM_INTERARRIVAL_2), EVENT_ARRIVAL_2);
		fprintf (outfile, "Ev arrival 2\n");
    destination = 3;
	}
	else if (station == 3)
	{
		event_schedule(sim_time + expon (mean_interarrival[3], STREAM_INTERARRIVAL_3), EVENT_ARRIVAL_3);
    destination = random_integer (prob_distrib_dest, STREAM_DESTINATION_ARV);
	  fprintf (outfile, "Ev arrival 3 %d\n", destination);
  }

  transfer[1] = sim_time;
  transfer[2] = station;
  transfer[3] = destination;
  list_file(LAST, station);
  
  if ((bus_position == station) && (bus_idle == 1))
  {
    event_cancel(EVENT_DEPARTURE_BUS);
    load();
  }
    
}

void arrive_b(){
  int init = bus_position;
  bus_moving = 0;

  if(bus_position != 3){
    bus_position++;
  }
  else{
    bus_position = 1;
    looping = 1;
  }

  if(bus_position == 3 && looping){
    loop_final = sim_time - dist[init][bus_position];
    sampst(loop_final - loop_ori, VAR_BUS);
    loop_ori = loop_final;
  }

  //sampst(sim_time - arrive_time_b - dist[init][bus_position], 1);
  
  arrive_time_b = sim_time;
  fprintf (outfile, "Ev arrive bus%19d\n", bus_position);
  fprintf (outfile, "Number to unload%16d\n",list_size[MAX_NUM_STATIONS+bus_position]);
  unload();
}

void report(void){
  fprintf (outfile, "a.\n");
	for (i = 1; i <= MAX_NUM_STATIONS; i++){
		filest(i);
		fprintf (outfile, "Average number in location %d queue: %0.3f\n", i, transfer[1]);
		fprintf (outfile, "Maximum number in location %d queue: %0.3f\n", i, transfer[2]);
	}
	
	fprintf (outfile, "\n\nb.\n");
	for (i = 1; i <= MAX_NUM_STATIONS; i++){
		sampst(0.0, -i);
		fprintf (outfile, "Average delay in location %d queue: %0.3f\n", i, transfer[1]);
		fprintf (outfile, "Maximum delay in location %d queue: %0.3f\n", i, transfer[3]);
	}
	
	fprintf (outfile, "\n\nc.\n");
	timest(0.0, -VAR_BUS);
	fprintf (outfile, "Average number on the bus: %0.3f\n", transfer[1]);
	fprintf (outfile, "Maximum number on the bus: %0.3f\n", transfer[2]);
	
	fprintf (outfile, "\n\nd.\n");
	for (i = 1; i <= MAX_NUM_STATIONS; i++){
    sampst(0.0, -i - VAR_BUS_AT_STATION);
		fprintf (outfile, "Average time stop in location %d: %0.3f\n", i, transfer[1]);
		fprintf (outfile, "Maximum time stop in location %d: %0.3f\n", i, transfer[3]);
		fprintf (outfile, "Minimum time stop in location %d: %0.3f\n", i, transfer[4]);
	}
	
	fprintf (outfile, "\n\ne.\n");
  sampst(0.0, -VAR_BUS);
	fprintf (outfile, "Average time to make a loop:	%0.3f\n", transfer[1]);
	fprintf (outfile, "Maximum time to make a loop:	%0.3f\n", transfer[3]);
	fprintf (outfile, "Minimum time to make a loop:	%0.3f\n", transfer[4]);
	
	fprintf (outfile, "\n\nf.\n");
	for (i = 1; i <= MAX_NUM_STATIONS; i++){
    sampst(0.0, -i - VAR_PERSON_FROM_STATION);
		fprintf (outfile, "Average time person in system from location %d: %0.3f\n", i, transfer[1]);
		fprintf (outfile, "Maximum time person in system from location %d: %0.3f\n", i, transfer[3]);
		fprintf (outfile, "Minimum time person in system from location %d: %0.3f\n", i, transfer[4]);
	}
}

int
main ()				/* Main function. */
{
  /* Open input and output files. */
  looping = 0;
  infile = fopen ("carrental.in", "r");
  outfile = fopen ("carrental.out", "w");

  /* Read input parameters. */
  fscanf (infile, "%d %lg", &num_stations, &length_simulation);
  fscanf (infile, "%lg %lg", &speed, &waiting_time);
  for (i = 1; i <= num_stations; ++i) {
    for (j = 1; j <=num_stations; ++j) {
        fscanf (infile, "%lg", &dist[i][j]);
        dist[i][j] = dist[i][j]/speed; // replace jarak in miles menjadi waktu tempuh in seconds  
      }
  }

  
  for (j = 1; j <= num_stations; ++j)
    fscanf (infile, "%lg", &mean_interarrival[j]);

  for (i = 1; i <= num_stations-1; ++i)
    fscanf (infile, "%lg", &prob_distrib_dest[i]);

  /* Write report heading and input parameters. */

  fprintf (outfile, "Car Rental model\n\n");
  fprintf (outfile, "Number of stations%21d", num_stations);
  fprintf (outfile, "\n\nDistribution function of destination  ");
  for (i = 1; i <= num_stations-1; ++i)
    fprintf (outfile, "%8.3f", prob_distrib_dest[i]);
  fprintf (outfile, "\n\nMean interarrival time of each stations ");
  for (i = 1; i <= num_stations; ++i)
    fprintf (outfile, "%8.3f", mean_interarrival[i]);
  fprintf (outfile, "\n\nLength of the simulation%20.1f hours", length_simulation);
  fprintf (outfile, "\n\nSpeed of bus%32.1f miles per hour", speed);
  fprintf (outfile, "\n\nBus idle time in station%20.1f minutes\n\n", waiting_time);

  /* Initialize bus position. */

  bus_position = 2;
  bus_idle = 1;
  waiting_time = waiting_time/60.0f;
  capacity = MAX_NUM_SEATS;

  /* Initialize simlib */

  init_simlib ();

  /* Set maxatr = max(maximum number of attributes per record, 4) */

  maxatr = 4;			/* NEVER SET maxatr TO BE SMALLER THAN 4. */

  /* Schedule the arrival of the first job. */
  event_schedule (0.0, EVENT_ARRIVE_BUS);

  event_schedule (expon (mean_interarrival[1], STREAM_INTERARRIVAL_1), EVENT_ARRIVAL_1);
  event_schedule (expon (mean_interarrival[2], STREAM_INTERARRIVAL_2), EVENT_ARRIVAL_2);
  event_schedule (expon (mean_interarrival[3], STREAM_INTERARRIVAL_3), EVENT_ARRIVAL_3);

  /* Schedule the end of the simulation.  (This is needed for consistency of
     units.) */

  event_schedule (length_simulation, EVENT_END_SIMULATION);

  /* Run the simulation until it terminates after an end-simulation event
     (type EVENT_END_SIMULATION) occurs. */

  do
    {

      /* Determine the next event. */

      timing ();

      /* Invoke the appropriate event function. */
      
      switch (next_event_type)
	{
	case EVENT_ARRIVAL_1:
	  arrive (1,1);
	  break;
	case EVENT_ARRIVAL_2:
	  arrive (1,2);
	  break;  	
	case EVENT_ARRIVAL_3:
	  arrive (1,3);
	  break;
	case EVENT_LOAD:
	  load ();
	  break;
	case EVENT_UNLOAD:
	  unload ();
	  break;
	case EVENT_ARRIVE_BUS:
	  arrive_b ();
	  break;
	case EVENT_DEPARTURE_BUS:
	  move_b ();
	  break;   
	case EVENT_END_SIMULATION:
	  report ();
	  break;
	}

      /* If the event just executed was not the end-simulation event (type
         EVENT_END_SIMULATION), continue simulating.  Otherwise, end the
         simulation. */

    }
  while (next_event_type != EVENT_END_SIMULATION);

  fclose (infile);
  fclose (outfile);

  return 0;
}
