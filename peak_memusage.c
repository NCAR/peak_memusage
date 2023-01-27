/* Includes */
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include <sys/types.h>  /* Primitive System Data Types */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <time.h>	/* time and difftime */
#include <string.h>	/* string stuff */
#include <unistd.h>     /* gethostname */
#include <sys/resource.h> /* getrusage */

void printUsage(char **argv) {
        fprintf(stderr, "\nUsage:\n%s <filename-to-run> [<arguments-to-pass>] \n\n", argv[0]);
	exit(1);
}

int main(int argc, char **argv) {
	struct rusage us, ous;	/* resource us struct */
	char *runme, *space = " ", *extra = "; sleep 1";
        char hn[256];
	time_t start_time, stop_time;
	int detailed = 0, /* error, */ exit_status, signal, rank=0 /*, poolsize*/;

        gethostname(hn, sizeof(hn) / sizeof(char));

        // try some common env vars to learn rank:
        if (NULL != getenv("MP_CHILD"))
          rank = atoi(getenv("MP_CHILD"));

        else if (NULL != getenv("PMI_RANK"))
          rank = atoi(getenv("PMI_RANK"));

        else if (NULL != getenv("PMIX_RANK"))
          rank = atoi(getenv("PMIX_RANK"));

        else if (NULL != getenv("OMPI_COMM_WORLD_RANK"))
          rank = atoi(getenv("OMPI_COMM_WORLD_RANK"));

        else if (NULL != getenv("SLURM_PROCID"))
          rank = atoi(getenv("SLURM_PROCID"));

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
			fprintf(stderr, "%12.3f %12ld %12ld %12ld %12ld %12d %12d %12d %12s (RUSAGE_SELF)\n",
				difftime(stop_time, start_time),
				us.ru_maxrss, us.ru_ixrss, us.ru_idrss, us.ru_isrss,
				rank, exit_status, signal, runme);
		}
		if ( getrusage(RUSAGE_CHILDREN, &us) ) {
			fprintf(stderr, "\n\n%s ran (task #%4d). Exit status: %d. Signal: %d\n", runme, rank, exit_status, signal);
			fprintf(stderr, "Problems getting resource usage for RUSAGE_CHILDREN\n\n");
		}
		else {
			fprintf(stderr, "%12.3f %12ld %12ld %12ld %12ld %12d %12d %12d %12s (RUSAGE_CHILDREN)\n\n",
				difftime(stop_time, start_time),
				us.ru_maxrss, us.ru_ixrss, us.ru_idrss, us.ru_isrss,
				rank, exit_status, signal, runme);
		}
	} else {
		if ( getrusage(RUSAGE_CHILDREN, &us) || getrusage(RUSAGE_SELF, &ous) ) {
			fprintf(stderr, "\n\n%s ran (task #%4d). Exit status: %d. Signal: %d\n", runme, rank, exit_status, signal);
			fprintf(stderr, "Problem getting resource usage\n");
		} else {
			fprintf(stderr, "%s used memory in task %d: %.2fMiB (+%.2fMiB overhead). ExitStatus: %d. Signal: %d\n",
				hn, rank, us.ru_maxrss/1024., ous.ru_maxrss/1024., exit_status, signal);
		}

	}

	return exit_status;
}
