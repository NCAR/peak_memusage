/*

$Id$

*/

/* Includes */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <time.h>	/* time and difftime */
#include <string.h>	/* string stuff */

#include <sys/resource.h> /* getrusage */

#ifdef COMPILE_MPI
#include <mpi.h>
#endif

void printUsage(char **argv) {
/*	The details options has been removed from the help, because it was
        confusing for the users
        printf("\nUsage:\n%s [--details] <filename-to-run> [<arguments-to-pass>] \n\n", argv[0]);
*/
        printf("\nUsage:\n%s <filename-to-run> [<arguments-to-pass>] \n\n", argv[0]);
	exit(1);
}

int main(int argc, char **argv) {
	struct rusage us;	/* resource us struct */
	char *runme, space = ' ';
	time_t start_time, stop_time;
	int detailed = 0, error, exit_status, signal, rank;

	if (argc > 1) {
		int current_arg = 1;
		if (strcmp(argv[current_arg], "--details") == 0) {
			detailed = 1;
			current_arg++;
		}
		int curr_arg_ct = current_arg, total_size = 0;
		
		for(; curr_arg_ct<argc; curr_arg_ct++)
			total_size += strlen(argv[curr_arg_ct]) + 1;
		/* "+ 1" means ' ' or last '/0'
		printf("Allocating %d bytes for arguments", 
			total_size * sizeof(char));
		*/
		
		runme = malloc(total_size * sizeof(char));
		strcpy(runme, argv[current_arg]);
		for (current_arg++; current_arg<argc; current_arg++) {
			strcat(runme, &space);
			strcat(runme, argv[current_arg]);
		}
	}
	else 
		printUsage(argv);

	printf("\nRunning: %s \nPlease wait...\n\n", runme);
	start_time = time(NULL);
	exit_status = system(runme);
	signal = exit_status << 8 & 255;
	exit_status = exit_status >> 8 & 255;
	stop_time = time(NULL);
	
#ifdef COMPILE_MPI 
	if(error = MPI_Init(NULL, NULL)) {
                printf("Init error: %d", error);
                return 1;
        }
        if(error = MPI_Comm_rank(MPI_COMM_WORLD, &rank)) {
                printf("Rank error: %d", error);
                return 1;
        }
	if(error = MPI_Finalize()) {
                printf("Finalize error: %d", error);
        }
#else	
  #ifdef COMPILE_OMP
    #pragma omp parallel
    {
	rank = omp_get_thread_num();
  #else
    rank = 0;
  #endif
#endif
	
	if (detailed) {
		printf("Memory usage:\n%12s %12s %12s %12s %12s %12s %12s %12s %12s\n",
			"Time(s)", "Resident (KB)", "Code", "Allocated", "Stack",
			"Task #", "Exit Status", "Signal", "command line");
		
		if ( getrusage(RUSAGE_SELF, &us) ) {
			printf("\n\n%s ran (task #%4d). Exit status: %d. Signal: %d\n", runme, rank, exit_status, signal);
			printf("Problems getting resource usage for RUSAGE_SELF\n");
		}
		else {
			printf("%12.3f %12d %12d %12d %12d %12d %12d %12d %12s (RUSAGE_SELF)\n",
				difftime(stop_time, start_time), 
				us.ru_maxrss, us.ru_ixrss, us.ru_idrss, us.ru_isrss,
				rank, exit_status, signal, runme);
		}
		if ( getrusage(RUSAGE_CHILDREN, &us) ) {
			printf("\n\n%s ran (task #%4d). Exit status: %d. Signal: %d\n", runme, rank, exit_status, signal);
			printf("Problems getting resource usage for RUSAGE_CHILDREN\n\n");
		}
		else {
			printf("%12.3f %12d %12d %12d %12d %12d %12d %12d %12s (RUSAGE_CHILDREN)\n\n",
				difftime(stop_time, start_time), 
				us.ru_maxrss, us.ru_ixrss, us.ru_idrss, us.ru_isrss,
				rank, exit_status, signal, runme);
		}
	} else {
		if ( getrusage(RUSAGE_CHILDREN, &us) ) {
			printf("\n\n%s ran (task #%4d). Exit status: %d. Signal: %d\n", runme, rank, exit_status, signal);
			printf("Problem getting resource usage\n");
		} else {
			printf("Memory usage for %12s (task #%4d) is: %10d KB. Exit status: %d. Signal: %d\n", 
				runme, rank, us.ru_maxrss, exit_status, signal);
		}
			
	}
	
#ifdef COMPILE_OMP
    }
#endif
	
	return exit_status;
}
