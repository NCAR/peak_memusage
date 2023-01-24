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
#include <sys/time.h>
#include <limits.h> /* PATH_MAX */
#include <pthread.h>
#include <stdbool.h>

#include "log_memusage.h"

#define LOG_MEMUSAGE_INVALID_RANK -1

/*
 * Structure to "hide" implementation details,
 * as much as possible in C anyway...
 */
static struct log_memusage_data_str
{
  char hostname[NAME_MAX];
  pid_t pid;

  char filename[PATH_MAX];
  FILE *fptr;

  struct smaps_sizes {
    int KernelPageSize;
    int MMUPageSize;
    int Private_Clean;
    int Private_Dirty;
    int Pss;
    int Referenced;
    int Rss;
    int Shared_Clean;
    int Shared_Dirty;
    int Size;
    int Swap;
  } sizes;

  struct timeval start_time;
  struct timespec sleep_time;

  int retval;
  int rank;
  pthread_t thread;
  pthread_mutex_t mutex;

} log_memusage_impl_data;


/*
 * https://gist.github.com/avar/896026/c346c7c8e4a9ab18577b4e6abfca37e358de83c1
 */
int log_memusage_parse_smaps(int verbose)
{
  char line[BUFSIZ];
  sprintf(line, "/proc/%d/smaps", log_memusage_impl_data.pid);
  FILE *file = fopen(line, "r");
  memset(&log_memusage_impl_data.sizes, 0, sizeof log_memusage_impl_data.sizes);

  if (!file) {
    perror(line);
    return -1;
  }

  while (fgets(line, sizeof line, file))
    {
      char substr[32];
      int n;
      if (sscanf(line, "%31[^:]: %d", substr, &n) == 2)
        {
          if ((verbose > 1) && n)
            printf(" --> read %8d from \t%s", n, line);

          if      (strcmp(substr, "KernelPageSize") == 0) { log_memusage_impl_data.sizes.KernelPageSize += n; }
          else if (strcmp(substr, "MMUPageSize") == 0)    { log_memusage_impl_data.sizes.MMUPageSize += n; }
          else if (strcmp(substr, "Private_Clean") == 0)  { log_memusage_impl_data.sizes.Private_Clean += n; }
          else if (strcmp(substr, "Private_Dirty") == 0)  { log_memusage_impl_data.sizes.Private_Dirty += n; }
          else if (strcmp(substr, "Pss") == 0)            { log_memusage_impl_data.sizes.Pss += n; }
          else if (strcmp(substr, "Referenced") == 0)     { log_memusage_impl_data.sizes.Referenced += n; }
          else if (strcmp(substr, "Rss") == 0)            { log_memusage_impl_data.sizes.Rss += n; }
          else if (strcmp(substr, "Shared_Clean") == 0)   { log_memusage_impl_data.sizes.Shared_Clean += n; }
          else if (strcmp(substr, "Shared_Dirty") == 0)   { log_memusage_impl_data.sizes.Shared_Dirty += n; }
          else if (strcmp(substr, "Size") == 0)           { log_memusage_impl_data.sizes.Size += n; }
          else if (strcmp(substr, "Swap") == 0)           { log_memusage_impl_data.sizes.Swap += n; }
        }
    }
  fclose(file);

  if (verbose)
    {
      printf("KernelPageSize %d\n", log_memusage_impl_data.sizes.KernelPageSize);
      printf("MMUPageSize %d\n", log_memusage_impl_data.sizes.MMUPageSize);
      printf("Private_Clean %d\n", log_memusage_impl_data.sizes.Private_Clean);
      printf("Private_Dirty %d\n", log_memusage_impl_data.sizes.Private_Dirty);
      printf("Pss %d\n", log_memusage_impl_data.sizes.Pss);
      printf("Referenced %d\n", log_memusage_impl_data.sizes.Referenced);
      printf("Rss %d\n", log_memusage_impl_data.sizes.Rss);
      printf("Shared_Clean %d\n", log_memusage_impl_data.sizes.Shared_Clean);
      printf("Shared_Dirty %d\n", log_memusage_impl_data.sizes.Shared_Dirty);
      printf("Size %d\n", log_memusage_impl_data.sizes.Size);
      printf("Swap %d\n", log_memusage_impl_data.sizes.Swap);
    }

  return 0;
}



int log_memusage_annotate(const char* label)
{
  int nchar = -1;

  /* acquire a mutex lock to protect log_memusage_impl_data.fptr, log_memusage_impl_data.sizes */
  pthread_mutex_lock(&log_memusage_impl_data.mutex);
  if (NULL == log_memusage_impl_data.fptr)
    {
      fprintf(stderr, "logging memory usage for process %d to %s ...\n",
              log_memusage_impl_data.pid,
              log_memusage_impl_data.filename);

      log_memusage_impl_data.fptr = fopen(log_memusage_impl_data.filename, "w");
    }
  nchar = fprintf(log_memusage_impl_data.fptr, "# %s\n", label);
  pthread_mutex_unlock(&log_memusage_impl_data.mutex);
  /* done mutex */

  return nchar;
}



double log_memusage_get()
{
  struct rusage rus;

  if (getrusage(RUSAGE_SELF, &rus))
    {
      fprintf(stderr, "Problem getting resource usage\n");
      return -1.;
    }

  return rus.ru_maxrss/1024.;
}



double log_memusage_report(const char* prefix)
{
  const double maxrss_MB = log_memusage_get();

  fprintf(stderr, "%s%s / PID %d, peak used memory: %.2fMiB\n",
          prefix,
          log_memusage_impl_data.hostname,
          log_memusage_impl_data.pid,
          maxrss_MB);

  return maxrss_MB;
}



void* log_memusage_execution_thread (void* ptr)
{
  struct timeval current_time;

  double elapsed=0., elapsed_us=0.;

  /* char cmd[BUFSIZ]; */

  /* sprintf(cmd, "ls /proc/%d/ ; cat /proc/%d/smaps && echo", */
  /*         pid, pid); */
  /* system(cmd); */

  /* log_memusage_parse_smaps(/\* verbose = *\/ 2); */

  /* sprintf(cmd, "echo && echo -n 'Pss: ' && awk '/Pss:/{ sum += $2 } END { print sum }' /proc/%d/smaps && echo -n 'Rss: ' && awk '/Rss:/{ sum += $2 } END { print sum }' /proc/%d/smaps && echo", */
  /*         pid, pid); */
  /* system(cmd); */

  log_memusage_annotate("elapsed time (s), Referenced (MiB), RSS (MiB), PSS (MiB)");

  while (true)
    {
      gettimeofday(&current_time, NULL);

      elapsed_us = (current_time.tv_sec - log_memusage_impl_data.start_time.tv_sec) * 1000000 + current_time.tv_usec - log_memusage_impl_data.start_time.tv_usec;
      elapsed = elapsed_us / 1000000.;

      /* acquire a mutex lock to protect log_memusage_impl_data.fptr, log_memusage_impl_data.sizes */
      {
        pthread_mutex_lock(&log_memusage_impl_data.mutex);

        log_memusage_parse_smaps(/* verbose = */ 0);

        fprintf(log_memusage_impl_data.fptr, "%g, %g, %g, %g\n",
                elapsed,
                log_memusage_impl_data.sizes.Referenced / 1024.,
                log_memusage_impl_data.sizes.Rss / 1024.,
                log_memusage_impl_data.sizes.Pss / 1024.);
        pthread_mutex_unlock(&log_memusage_impl_data.mutex);
      }
      /* done mutex */

      nanosleep(&log_memusage_impl_data.sleep_time, NULL);
    }

  return NULL;
}



/*
 * ------------------------------------------------------------------
 */
__attribute__((constructor))
void initialize_log_memusage ()
{
  printf("..(constructor)... %s, line: %d\n", __FILE__, __LINE__);

  char rank_env_vars[][64] = { "MPI_RANK",
                               "MP_CHILD",
                               "PMI_RANK",
                               "PMIX_RANK",
                               "OMPI_COMM_WORLD_RANK",
                               "SLURM_PROCID" };

  const int nrank_env_vars = sizeof rank_env_vars/64;
  int v=0;

  /*
   * Initialize Data Structures.
   */
  gethostname(log_memusage_impl_data.hostname, sizeof(log_memusage_impl_data.hostname) / sizeof(char));
  log_memusage_impl_data.pid = getpid();
  log_memusage_impl_data.rank = LOG_MEMUSAGE_INVALID_RANK;

  /* try some common env vars to learn rank: */
  for (v=0; v<nrank_env_vars; ++v)
    if (NULL != getenv(rank_env_vars[v]))
      {
        log_memusage_impl_data.rank = atoi(getenv(rank_env_vars[v]));
        break;
      }

  /* build up the output file name, depending on what info we have */
  if (log_memusage_impl_data.rank != LOG_MEMUSAGE_INVALID_RANK)
    sprintf(log_memusage_impl_data.filename, "memory_usage-rank_%d.log", log_memusage_impl_data.rank);
  else
    sprintf(log_memusage_impl_data.filename, "memory_usage.log");

  log_memusage_impl_data.sleep_time.tv_sec = 0;
  log_memusage_impl_data.sleep_time.tv_nsec = 1e8;

  log_memusage_impl_data.fptr = NULL;

  gettimeofday(&log_memusage_impl_data.start_time, NULL);

  pthread_mutex_init(&log_memusage_impl_data.mutex, NULL);

  log_memusage_impl_data.retval =
    pthread_create (&log_memusage_impl_data.thread, NULL, log_memusage_execution_thread, NULL);
}



__attribute__((destructor))
void finalize_log_memusage ()
{
  printf("..(destructor)... %s, line: %d\n", __FILE__, __LINE__);

  pthread_cancel(log_memusage_impl_data.thread);
  pthread_join(log_memusage_impl_data.thread, NULL);

  fclose(log_memusage_impl_data.fptr);

  log_memusage_report(/* prefix = */ "# (memusage) --> ");

  pthread_mutex_destroy(&log_memusage_impl_data.mutex);
}
