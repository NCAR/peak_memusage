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
        fprintf(stderr, "\nUsage:\n%s [--details] <filename-to-run> [<arguments-to-pass>] \n\n", argv[0]);
*/
        fprintf(stderr, "\nUsage:\n%s <filename-to-run> [<arguments-to-pass>] \n\n", argv[0]);
	exit(1);
}

int main(int argc, char **argv) {
	struct rusage us, ous;	/* resource us struct */
	char *runme, *space = " ", *extra = "; sleep 1";
	time_t start_time, stop_time;
	int detailed = 0, error, exit_status, signal, rank /*, poolsize*/;

#ifdef COMPILE_MPI 
	if(error = MPI_Init(NULL, NULL)) {
                fprintf(stderr, "MPI INIT error: %d", error);
                return 1;
        }
        if(error = MPI_Comm_rank(MPI_COMM_WORLD, &rank)) {
                fprintf(stderr, "MPI RANK error: %d", error);
                return 1;
        }
/*        if(error = MPI_Comm_size(MPI_COMM_WORLD, &poolsize)) {
                fprintf(stderr, "MPI SIZE error: %d", error);
                return 1;
        }
*/
#else
        char *value_child_mp, *value_slurm_rank, *value_rank_pmi /*, *value_procs_mp, *value_ncpus, *value_slurm_tasks*/;
	value_child_mp    = getenv ("MP_CHILD");
        value_rank_pmi    = getenv ("PMI_RANK");
        value_slurm_rank  = getenv ("SLURM_PROCID");
/*
	value_procs_mp    = getenv ("MP_PROCS");
        value_ncpus       = getenv ("NCPUS");
        value_slurm_tasks = getenv ("SLURM_NTASKS");
*/
	if (  ( (! value_child_mp) && (! value_rank_pmi) && (! value_slurm_rank) )
//           || ( (! value_procs_mp) && (! value_ncpus) && (! value_slurm_tasks) )
           ) {
		rank = 0;
//		poolsize = 1;
	} else {
                if (value_child_mp) {
                    rank = atoi(value_child_mp);
                } else if (value_slurm_rank) {
                    rank = atoi(value_slurm_rank);
                } else {
                    rank = atoi(value_rank_pmi);
                }
/*
                if (value_procs_mp) {
		    poolsize = atoi(value_procs_mp);
                } else if (value_slurm_tasks) {
		    poolsize = atoi(value_slurm_tasks);
                } else {
		    poolsize = atoi(value_ncpus);
                }
*/	}
#endif

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
		fprintf(stderr, "Allocating %d bytes for arguments", 
			total_size * sizeof(char));
		*/
                total_size += strlen(extra);
		
		runme = malloc(total_size * sizeof(char));
		strcpy(runme, argv[current_arg]);
		for (current_arg++; current_arg<argc; current_arg++) {
			strcat(runme, space);
			strcat(runme, argv[current_arg]);
		}
                strcat(runme, extra);
	}
	else 
		printUsage(argv);

	if (rank == 0)
            fprintf(stderr, "Running: %s  - Please wait...\n", runme);
	start_time = time(NULL);
	exit_status = system(runme);
	signal = exit_status << 8 & 255;
	exit_status = exit_status >> 8 & 255;
	stop_time = time(NULL);
	
	if (detailed) {
		fprintf(stderr, "Memory usage:\n%12s %12s %12s %12s %12s %12s %12s %12s %12s\n",
			"Time(s)", "Resident (KB)", "Code", "Allocated", "Stack",
			"Task #", "Exit Status", "Signal", "command line");
		
		if ( getrusage(RUSAGE_SELF, &us) ) {
			fprintf(stderr, "\n\n%s ran (task #%4d). Exit status: %d. Signal: %d\n", runme, rank, exit_status, signal);
			fprintf(stderr, "Problems getting resource usage for RUSAGE_SELF\n");
		}
		else {
			fprintf(stderr, "%12.3f %12d %12d %12d %12d %12d %12d %12d %12s (RUSAGE_SELF)\n",
				difftime(stop_time, start_time), 
				us.ru_maxrss, us.ru_ixrss, us.ru_idrss, us.ru_isrss,
				rank, exit_status, signal, runme);
		}
		if ( getrusage(RUSAGE_CHILDREN, &us) ) {
			fprintf(stderr, "\n\n%s ran (task #%4d). Exit status: %d. Signal: %d\n", runme, rank, exit_status, signal);
			fprintf(stderr, "Problems getting resource usage for RUSAGE_CHILDREN\n\n");
		}
		else {
			fprintf(stderr, "%12.3f %12d %12d %12d %12d %12d %12d %12d %12s (RUSAGE_CHILDREN)\n\n",
				difftime(stop_time, start_time), 
				us.ru_maxrss, us.ru_ixrss, us.ru_idrss, us.ru_isrss,
				rank, exit_status, signal, runme);
		}
	} else {
		if ( getrusage(RUSAGE_CHILDREN, &us) || getrusage(RUSAGE_SELF, &ous) ) {
			fprintf(stderr, "\n\n%s ran (task #%4d). Exit status: %d. Signal: %d\n", runme, rank, exit_status, signal);
			fprintf(stderr, "Problem getting resource usage\n");
		} else {
			fprintf(stderr, "Used memory in task %d: %.2fMiB (+%.2fMiB overhead). ExitStatus: %d. Signal: %d\n", 
				rank, us.ru_maxrss/1024., ous.ru_maxrss/1024., exit_status, signal);
		}
			
	}
	
#ifdef COMPILE_MPI 
	if(error = MPI_Finalize()) {
                fprintf(stderr, "MPI FINALIZE error: %d", error);
        }
#endif
	
	return exit_status;
}
