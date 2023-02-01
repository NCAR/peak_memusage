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

#include <string>
#include <vector>

#include "log_memusage.h"

void printUsage (char **argv)
{
  fprintf(stderr, "\nUsage:\n%s <filename-to-run> [<arguments-to-pass>] \n\n", argv[0]);
  exit(1);
}

int main (int argc, char **argv)
{
  struct rusage us, ous;	/* resource us struct */
  std::string runme;

  char hn[256];
  time_t start_time, stop_time;
  int detailed=0, exit_status, signal, rank=0;
  char rank_env_vars[][64] = { "MPI_RANK",
                               "MP_CHILD",
                               "PMI_RANK",
                               "PMIX_RANK",
                               "OMPI_COMM_WORLD_RANK",
                               "MV2_COMM_WORLD_RANK",
                               "SLURM_PROCID" };

  const int nrank_env_vars = (sizeof rank_env_vars/ sizeof rank_env_vars[0]);

  gethostname(hn, sizeof(hn) / sizeof(char));

  if ( !(argc > 1))
    {
      printUsage(argv);
      return 1;
    }

  /* try some common env vars to learn rank: */
  for (int v=0; v<nrank_env_vars; ++v)
    if (NULL != getenv(rank_env_vars[v]))
      {
        rank = atoi(getenv(rank_env_vars[v]));
        break;
      }

  for (int current_arg=1; current_arg<argc; current_arg++)
    {
      std::string arg(argv[current_arg]);

      if ("--details" == arg)
        {
          detailed = 1;
          continue;
        }

      runme += arg;
      runme += " ";
    }
  runme += "; sleep 1";

  if (rank == 0)
    fprintf(stderr, "Running: %s  - Please wait...\n", runme.c_str());

  start_time = time(NULL);
  exit_status = system(runme.c_str());
  signal = exit_status << 8 & 255;
  exit_status = exit_status >> 8 & 255;
  stop_time = time(NULL);

  if (detailed)
    {
      fprintf(stderr,
              "Memory usage:\n%12s %12s %12s %12s %12s %12s %12s %12s %12s\n",
              "Time(s)", "Resident (KB)", "Code", "Allocated", "Stack",
              "Task #", "Exit Status", "Signal", "command line");

      if ( getrusage(RUSAGE_SELF, &us) )
        {
          fprintf(stderr, "\n\n%s ran (task #%4d). Exit status: %d. Signal: %d\n", runme.c_str(), rank, exit_status, signal);
          fprintf(stderr, "Problems getting resource usage for RUSAGE_SELF\n");
        }
      else
        {
          fprintf(stderr,
                  "%12.3f %12ld %12ld %12ld %12ld %12d %12d %12d %12s (RUSAGE_SELF)\n",
                  difftime(stop_time, start_time),
                  us.ru_maxrss, us.ru_ixrss, us.ru_idrss, us.ru_isrss,
                  rank, exit_status, signal, runme.c_str());
        }
      if ( getrusage(RUSAGE_CHILDREN, &us) )
        {
          fprintf(stderr, "\n\n%s ran (task #%4d). Exit status: %d. Signal: %d\n", runme.c_str(), rank, exit_status, signal);
          fprintf(stderr, "Problems getting resource usage for RUSAGE_CHILDREN\n\n");
        }
      else
        {
          fprintf(stderr,
                  "%12.3f %12ld %12ld %12ld %12ld %12d %12d %12d %12s (RUSAGE_CHILDREN)\n\n",
                  difftime(stop_time, start_time),
                  us.ru_maxrss, us.ru_ixrss, us.ru_idrss, us.ru_isrss,
                  rank, exit_status, signal, runme.c_str());
        }
    }

  else
    {
      if ( getrusage(RUSAGE_CHILDREN, &us) || getrusage(RUSAGE_SELF, &ous) ) {
        fprintf(stderr, "\n\n%s ran (task #%4d). Exit status: %d. Signal: %d\n", runme.c_str(), rank, exit_status, signal);
        fprintf(stderr, "Problem getting resource usage\n");
      }
    else
      {
        fprintf(stderr,
                "%s used memory in task %d: %dMiB (+%dMiB overhead). ExitStatus: %d. Signal: %d\n",
                hn, rank, (int) us.ru_maxrss/LOG_MEMUSAGE_OS_RSUSAGE_TO_MB, (int) ous.ru_maxrss/LOG_MEMUSAGE_OS_RSUSAGE_TO_MB, exit_status, signal);
      }
    }

  return exit_status;
}
