/* External definitions for car-rental model. */

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

/* Declare non-simlib global variables. */

int num_stations, num_seats, i, j, bus_position, bus_moving;
double mean_interarrival[MAX_NUM_STATIONS + 1], length_simulation, prob_distrib_dest[MAX_NUM_STATIONS], arrive_time_b;
FILE *infile, *outfile;

void arrive (int new_job, int station) 
{
	int destination;

	if (station == 1)
	{
		event_schedule(sim_time + expon (mean_interarrival[1], STREAM_INTERARRIVAL_1), EVENT_ARRIVAL_1);
		destination = 3;
	}
	else if (station == 2)
	{
		event_schedule(sim_time + expon (mean_interarrival[2], STREAM_INTERARRIVAL_2), EVENT_ARRIVAL_2);
		destination = 3;
	}
	else if (station == 3)
	{
		event_schedule(sim_time + expon (mean_interarrival[3], STREAM_INTERARRIVAL_3), EVENT_ARRIVAL_3);
		destination = random_integer (prob_distrib_dest, STREAM_DESTINATION_ARV);
	}

	transfer[1] = sim_time;
	transfer[2] = station;
	transfer[3] = destination;
	list_file(LAST, station);
}
void load (void) {}
void unload (void) {}
void arrive_b () {}
void move_b () {}
void report (void) {}

int
main ()				/* Main function. */
{
  /* Open input and output files. */

  infile = fopen ("carrental.in", "r");
  outfile = fopen ("carrental.out", "w");

  /* Read input parameters. */

  fscanf (infile, "%d %lg", &num_stations, &length_simulation);
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
  fprintf (outfile, "\n\nLength of the simulation%20.1f hours\n", length_simulation);

  /* Initialize bus position. */

  bus_position = 3;

  /* Initialize simlib */

  init_simlib ();

  /* Set maxatr = max(maximum number of attributes per record, 4) */

  maxatr = 4;			/* NEVER SET maxatr TO BE SMALLER THAN 4. */

  /* Schedule the arrival of the first job. */

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