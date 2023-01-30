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
#include "log_memusage.h"

void printUsage(char **argv) {
  fprintf(stderr, "\nUsage:\n%s <filename-to-run> [<arguments-to-pass>] \n\n", argv[0]);
  exit(1);
}

int main(int argc, char **argv) {
  struct rusage us, ous;	/* resource us struct */
  char *runme=NULL, *space = " ", *extra = "; sleep 1";
  char hn[256];
  time_t start_time, stop_time;
  int detailed = 0, /* error, */ exit_status, signal, rank=0, v=0 /*, poolsize*/;
  char rank_env_vars[][64] = { "MPI_RANK",
                               "MP_CHILD",
                               "PMI_RANK",
                               "PMIX_RANK",
                               "OMPI_COMM_WORLD_RANK",
                               "MV2_COMM_WORLD_RANK",
                               "SLURM_PROCID" };

  const int nrank_env_vars = (sizeof rank_env_vars/ sizeof rank_env_vars[0]);

  gethostname(hn, sizeof(hn) / sizeof(char));

  /* try some common env vars to learn rank: */
  for (v=0; v<nrank_env_vars; ++v)
    if (NULL != getenv(rank_env_vars[v]))
      {
        rank = atoi(getenv(rank_env_vars[v]));
        break;
      }

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
    fprintf(stderr,
            "Memory usage:\n%12s %12s %12s %12s %12s %12s %12s %12s %12s\n",
            "Time(s)", "Resident (KB)", "Code", "Allocated", "Stack",
            "Task #", "Exit Status", "Signal", "command line");

    if ( getrusage(RUSAGE_SELF, &us) ) {
      fprintf(stderr, "\n\n%s ran (task #%4d). Exit status: %d. Signal: %d\n", runme, rank, exit_status, signal);
      fprintf(stderr, "Problems getting resource usage for RUSAGE_SELF\n");
    }
    else {
      fprintf(stderr,
              "%12.3f %12ld %12ld %12ld %12ld %12d %12d %12d %12s (RUSAGE_SELF)\n",
              difftime(stop_time, start_time),
              us.ru_maxrss, us.ru_ixrss, us.ru_idrss, us.ru_isrss,
              rank, exit_status, signal, runme);
    }
    if ( getrusage(RUSAGE_CHILDREN, &us) ) {
      fprintf(stderr, "\n\n%s ran (task #%4d). Exit status: %d. Signal: %d\n", runme, rank, exit_status, signal);
      fprintf(stderr, "Problems getting resource usage for RUSAGE_CHILDREN\n\n");
    }
    else {
      fprintf(stderr,
              "%12.3f %12ld %12ld %12ld %12ld %12d %12d %12d %12s (RUSAGE_CHILDREN)\n\n",
              difftime(stop_time, start_time),
              us.ru_maxrss, us.ru_ixrss, us.ru_idrss, us.ru_isrss,
              rank, exit_status, signal, runme);
    }
  } else {
    if ( getrusage(RUSAGE_CHILDREN, &us) || getrusage(RUSAGE_SELF, &ous) ) {
      fprintf(stderr, "\n\n%s ran (task #%4d). Exit status: %d. Signal: %d\n", runme, rank, exit_status, signal);
      fprintf(stderr, "Problem getting resource usage\n");
    }
    else {
      fprintf(stderr,
              "%s used memory in task %d: %dMiB (+%dMiB overhead). ExitStatus: %d. Signal: %d\n",
              hn, rank, (int) us.ru_maxrss/LOG_MEMUSAGE_OS_RSUSAGE_TO_MB, (int) ous.ru_maxrss/LOG_MEMUSAGE_OS_RSUSAGE_TO_MB, exit_status, signal);
    }

  }

  free(runme);

  return exit_status;
}
