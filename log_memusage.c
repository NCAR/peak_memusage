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
#include <signal.h>
#include <assert.h>
#include <stdarg.h>

#include "log_memusage.h"
#include "log_memusage_impl.h"


/**
 * \private
 * Structure to "hide" implementation details,
 * as much as possible in C anyway...
 */
static struct log_memusage_data_str
{
  char hostname[NAME_MAX];
  pid_t pid;

  char smapsname[PATH_MAX];

  char filename[PATH_MAX];
  FILE *fptr;

  /**
   * \private
   */
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

  int output_step_cnt;
  int retval;
  int rank;
  int maxGPU_MB;
  pthread_t thread;
  pthread_mutex_t mutex;
  bool running;

} log_memusage_impl_data;



__attribute__ ((visibility ("hidden")))
int log_memusage_msg (FILE* f, const char* format, ...)
{
  static bool firstcall = true;
  static int verbose = 0;
  va_list args;
  int rval;

  if (firstcall)
    {
      verbose = (getenv("LOG_MEMUSAGE_VERBOSE") != NULL) ? atoi(getenv("LOG_MEMUSAGE_VERBOSE")) : 0;
      if (log_memusage_impl_data.rank > 0)
        verbose = 0;
      firstcall = false;
    }

  if (!verbose)
    return 0;

  va_start (args, format);
  fprintf(f, LOG_MEMUSAGE_LOGGING_PREFIX);
  rval = vfprintf (f, format, args);
  va_end (args);

  return rval;
}



/* https://gist.github.com/avar/896026/c346c7c8e4a9ab18577b4e6abfca37e358de83c1 */
__attribute__ ((visibility ("hidden")))
int log_memusage_parse_smaps (int verbose)
{
#ifdef __APPLE__
  memset(&log_memusage_impl_data.sizes, 0, sizeof log_memusage_impl_data.sizes);
  log_memusage_impl_data.sizes.Rss = log_memusage_get() * LOG_MEMUSAGE_OS_RSUSAGE_TO_MB;
  return 0;
#else

  char line[BUFSIZ];
  FILE *file = fopen(log_memusage_impl_data.smapsname, "r");

  if (!file)
    {
      perror(line);
      return -1;
    }

  if (verbose > 1)
    printf(" --> reading memory summary from %s\n", log_memusage_impl_data.smapsname);

  memset(&log_memusage_impl_data.sizes, 0, sizeof log_memusage_impl_data.sizes);

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
      printf("MMUPageSize %d\n",    log_memusage_impl_data.sizes.MMUPageSize);
      printf("Private_Clean %d\n",  log_memusage_impl_data.sizes.Private_Clean);
      printf("Private_Dirty %d\n",  log_memusage_impl_data.sizes.Private_Dirty);
      printf("Pss %d\n",            log_memusage_impl_data.sizes.Pss);
      printf("Referenced %d\n",     log_memusage_impl_data.sizes.Referenced);
      printf("Rss %d\n",            log_memusage_impl_data.sizes.Rss);
      printf("Shared_Clean %d\n",   log_memusage_impl_data.sizes.Shared_Clean);
      printf("Shared_Dirty %d\n",   log_memusage_impl_data.sizes.Shared_Dirty);
      printf("Size %d\n",           log_memusage_impl_data.sizes.Size);
      printf("Swap %d\n",           log_memusage_impl_data.sizes.Swap);
    }

  return 0;
#endif
}



int log_memusage_annotate (const char* label)
{
  static bool firstcall = true;
  static int do_logging = 0;
  int nchar = -1;

  if (firstcall)
    {
      do_logging = atoi(getenv("LOG_MEMUSAGE_ENABLE_LOGFILE"));
      log_memusage_msg(stderr, "Found %d for LOG_MEMUSAGE_ENABLE_LOGFILE\n", do_logging);
      firstcall = false;
    }

  /* quick return */
  if (!do_logging)
    return 0;

  /* acquire a mutex lock to protect log_memusage_impl_data.fptr, log_memusage_impl_data.sizes */
  pthread_mutex_lock(&log_memusage_impl_data.mutex);
  if (NULL == log_memusage_impl_data.fptr)
    {
      log_memusage_msg(stderr, "logging memory usage for process %d to %s ...\n",
                       log_memusage_impl_data.pid,
                       log_memusage_impl_data.filename);

      log_memusage_impl_data.fptr = fopen(log_memusage_impl_data.filename, "w");
    }
  nchar = fprintf(log_memusage_impl_data.fptr, "# %s\n", label);
  pthread_mutex_unlock(&log_memusage_impl_data.mutex);
  /* done mutex */

  return nchar;
}



int log_memusage_get ()
{
  struct rusage rus;

  if (getrusage(RUSAGE_SELF, &rus))
    {
      log_memusage_msg(stderr, "Problem getting resource usage\n");
      return -1;
    }

  return (rus.ru_maxrss / LOG_MEMUSAGE_OS_RSUSAGE_TO_MB);
}



int log_memusage_report (const char* prefix)
{
  const int
    maxrss_MB = log_memusage_get(),
    ngpus = log_memusage_ngpus();

  fprintf(stderr, "%s%s / PID %d",
          prefix,
          log_memusage_impl_data.hostname,
          log_memusage_impl_data.pid);

  if (log_memusage_impl_data.rank != LOG_MEMUSAGE_INVALID_RANK)
    fprintf(stderr, " / MPI Rank %d",
            log_memusage_impl_data.rank);

  fprintf(stderr, ", peak used memory: %d MiB",
          maxrss_MB);
  if (ngpus > 0)
    fprintf(stderr, " (CPU), %d MiB (GPU)",
            log_memusage_impl_data.maxGPU_MB);
  fprintf(stderr, "\n");

  return maxrss_MB;
}



__attribute__ ((visibility ("hidden")))
void* log_memusage_execution_thread (void* ptr)
{
  static bool firstcall = true;
  static int ngpus = 0;
  struct timeval current_time;
  log_memusage_gpu_memory_t gpu_memory;
  int ierr = 0, curr_rssMB=0, last_rssMB=0, last_maxGPU_MB=0, gpu=0;
  unsigned long step=0;
  double elapsed=0., elapsed_us=0.;

  const int cpu_mem_tripwire = (getenv("LOG_MEMUSAGE_CPU_MEM_TRIPWIRE") != NULL) ? atoi(getenv("LOG_MEMUSAGE_CPU_MEM_TRIPWIRE")) : INT_MAX;
  const int gpu_mem_tripwire = (getenv("LOG_MEMUSAGE_GPU_MEM_TRIPWIRE") != NULL) ? atoi(getenv("LOG_MEMUSAGE_GPU_MEM_TRIPWIRE")) : INT_MAX;

  log_memusage_msg(stderr, "Using %d MB as LOG_MEMUSAGE_CPU_MEM_TRIPWIRE\n", cpu_mem_tripwire);
  log_memusage_msg(stderr, "Using %d MB as LOG_MEMUSAGE_GPU_MEM_TRIPWIRE\n", gpu_mem_tripwire);

  /* log_memusage_parse_smaps (/\* verbose = *\/ 2); */

  if (firstcall)
    {
      char str[PATH_MAX], str2[NAME_MAX];
      sprintf(str, "elapsed time (s), CPU RSS (MiB)");
      ngpus = log_memusage_ngpus();
      if (ngpus == 1)
        sprintf(str, "%s, GPU (MiB)", strcpy(str2, str));
      else if (ngpus > 1)
        for (gpu=0; gpu<ngpus; gpu++)
          sprintf(str, "%s, GPU[%d] (MiB)", strcpy(str2, str), gpu);
      log_memusage_annotate(str);
      firstcall = false;
    }

  /* main execution / monitoring loop */
  for (step=0; ; step++)
    {
      last_rssMB     = curr_rssMB;
      curr_rssMB     = log_memusage_get();
      last_maxGPU_MB = gpu_memory.max_used;

      /* printf("curr_rssMB=%d\n", curr_rssMB); */

      if (curr_rssMB > cpu_mem_tripwire)
        {
          log_memusage_msg(stderr, "killing Process %d for exceeding CPU memory limit: %d > %d\n",
                           log_memusage_impl_data.pid,
                           curr_rssMB, cpu_mem_tripwire);
          pthread_kill(log_memusage_get_parent_thread_ID(), SIGUSR2);
          return NULL;
        }

      if (ngpus > 0)
        {
          gpu_memory = log_memusage_get_each_gpu();

          if (gpu_memory.max_used > log_memusage_impl_data.maxGPU_MB)
            log_memusage_impl_data.maxGPU_MB = gpu_memory.max_used;

          if (gpu_memory.max_used > gpu_mem_tripwire)
            {
              log_memusage_msg(stderr, "killing Process %d for exceeding GPU memory limit: %d > %d\n",
                               log_memusage_impl_data.pid,
                               gpu_memory.max_used, gpu_mem_tripwire);
              pthread_kill(log_memusage_get_parent_thread_ID(), SIGUSR2);
              return NULL;
            }
        }

      /* only bother with any I/O if the log file has been opened */
      if (NULL != log_memusage_impl_data.fptr)
        {
          /* Do we want to log this step? */
          bool print_step = false;
          print_step = print_step || (curr_rssMB != last_rssMB);
          print_step = print_step || (last_maxGPU_MB != gpu_memory.max_used);
          print_step = print_step || (step % log_memusage_impl_data.output_step_cnt == 0);

          if (print_step)
            {
              gettimeofday(&current_time, NULL);

              elapsed_us = ((current_time.tv_sec  - log_memusage_impl_data.start_time.tv_sec) * 1000000 +
                            (current_time.tv_usec - log_memusage_impl_data.start_time.tv_usec));
              elapsed = elapsed_us / 1000000.;

              /* acquire a mutex lock to protect log_memusage_impl_data.fptr, log_memusage_impl_data.sizes, etc.. */
              pthread_mutex_lock(&log_memusage_impl_data.mutex);

              ierr = log_memusage_parse_smaps(/* verbose = */ 0);

              /* fprintf(log_memusage_impl_data.fptr, "%g, %d, %d, %d\n", */
              /*         elapsed, */
              /*         log_memusage_impl_data.sizes.Referenced / LOG_MEMUSAGE_OS_RSUSAGE_TO_MB , */
              /*         log_memusage_impl_data.sizes.Rss / LOG_MEMUSAGE_OS_RSUSAGE_TO_MB , */
              /*         log_memusage_impl_data.sizes.Pss / LOG_MEMUSAGE_OS_RSUSAGE_TO_MB ); */

              fprintf(log_memusage_impl_data.fptr, "%g, %d",
                      elapsed,
                      log_memusage_impl_data.sizes.Rss / LOG_MEMUSAGE_OS_RSUSAGE_TO_MB );
              for (gpu=0; gpu<ngpus; gpu++)
                fprintf(log_memusage_impl_data.fptr, ", %d",
                        gpu_memory.used[gpu]);
              fprintf(log_memusage_impl_data.fptr, "\n");

              pthread_mutex_unlock(&log_memusage_impl_data.mutex);
              /* done mutex */

              if (ierr) return NULL;
            }
        }

      nanosleep(&log_memusage_impl_data.sleep_time, NULL);
    }

  return NULL;
}



int log_memusage_pause ()
{
  /* acquire a mutex lock to acquire log_memusage_impl_data.fptr  (make sure not in use) */
  pthread_mutex_lock(&log_memusage_impl_data.mutex);

  if (log_memusage_impl_data.running)
    {
      pthread_cancel(log_memusage_impl_data.thread);
      pthread_join(log_memusage_impl_data.thread, NULL);
      log_memusage_impl_data.running = false;
    }

  pthread_mutex_unlock(&log_memusage_impl_data.mutex);

  return 0;
}



int log_memusage_resume ()
{
  /*
   * make sure logging thread is cancelled...
   */
  log_memusage_pause ();

  log_memusage_impl_data.running = true;

  log_memusage_impl_data.retval =
    pthread_create (&log_memusage_impl_data.thread, NULL, log_memusage_execution_thread, NULL);

  return 0;
}



__attribute__ ((visibility ("hidden")))
 pthread_t log_memusage_get_parent_thread_ID ()
{
  static pthread_t parent_thread_ID;
  static bool firstcall = true;
  if (firstcall)
    {
      parent_thread_ID = pthread_self();
      firstcall = false;
    }

  return parent_thread_ID;
}



__attribute__ ((visibility ("hidden")))
void log_memusage_initialize_environment ()
{
  char str[NAME_MAX];
  extern char **environ;
  int i=0;

  /* printf("..(constructor)... %s, line: %d\n", __FILE__, __LINE__); */

  /*
   * Initialize environment variables (conditionally, if not set already.
   */
  setenv("LOG_MEMUSAGE_VERBOSE",                        "0", /* overwrite = */ 0);
  setenv("LOG_MEMUSAGE_ENABLE_LOGFILE",                 "0", /* overwrite = */ 0);
  setenv("LOG_MEMUSAGE_LOGFILE_NAME",    "memory_usage.log", /* overwrite = */ 0);
  setenv("LOG_MEMUSAGE_POLL_INTERVAL",                "0.1", /* overwrite = */ 0);
  setenv("LOG_MEMUSAGE_OUTPUT_INTERVAL",               "1.", /* overwrite = */ 0);
  sprintf(str, "%d", INT_MAX);
  setenv("LOG_MEMUSAGE_CPU_MEM_TRIPWIRE",               str, /* overwrite = */ 0);
  setenv("LOG_MEMUSAGE_GPU_MEM_TRIPWIRE",               str, /* overwrite = */ 0);

  while (environ[i])
    {
      if (strstr(environ[i], "LOG_MEMUSAGE_"))
          log_memusage_msg(stderr, "(env) %s\n", environ[i]);
      i++;
    }
}



__attribute__((constructor))
void log_memusage_initialize ()
{
  /* printf("..(constructor)... %s, line: %d\n", __FILE__, __LINE__); */
  int v = 0;
  char rank_env_vars[][64] = { "MPI_RANK",
                               "MP_CHILD",
                               "PMI_RANK",
                               "PMIX_RANK",
                               "OMPI_COMM_WORLD_RANK",
                               "MV2_COMM_WORLD_RANK",
                               "SLURM_PROCID" };

  const int nrank_env_vars = (sizeof rank_env_vars/ sizeof rank_env_vars[0]);

  double polling_interval_sec = 0., output_interval_sec = 0.;
  int output_interval_step = 0;

  /* see if we can learn our MPI rank from the environment, before anything else... */
  log_memusage_impl_data.rank = LOG_MEMUSAGE_INVALID_RANK;
  for (v=0; v<nrank_env_vars; ++v)
    if (NULL != getenv(rank_env_vars[v]))
      {
        log_memusage_impl_data.rank = atoi(getenv(rank_env_vars[v]));
        break;
      }

  log_memusage_initialize_environment();
  log_memusage_initialize_nvml();

  /* call this function from the main thread to register the parent thread ID inside for later use. */
  log_memusage_get_parent_thread_ID();

  log_memusage_register_signal_handler();

  /*
   * Initialize Data Structures.
   */
  gethostname(log_memusage_impl_data.hostname, sizeof(log_memusage_impl_data.hostname) / sizeof(char));
  log_memusage_impl_data.pid = getpid();

  /* where do we get our usage data?*/
  /* /proc/<PID>/smaps_rollup is preferred... */
  sprintf(log_memusage_impl_data.smapsname, "/proc/%d/smaps_rollup", log_memusage_impl_data.pid);
  if (access(log_memusage_impl_data.smapsname, R_OK) != 0)
    {
      /* fall back to /proc/<PID>/smaps */
      sprintf(log_memusage_impl_data.smapsname, "/proc/%d/smaps", log_memusage_impl_data.pid);

      if (access(log_memusage_impl_data.smapsname, R_OK) != 0)
        log_memusage_msg(stderr, "Cannot locate /proc/%d/smap_rollup or /proc/%d/smaps file!!\n", log_memusage_impl_data.pid, log_memusage_impl_data.pid);
    }

  /* build up the output file name, depending on what info we have */
  {
    sprintf(log_memusage_impl_data.filename, "%s", getenv("LOG_MEMUSAGE_LOGFILE_NAME"));
    if (LOG_MEMUSAGE_INVALID_RANK != log_memusage_impl_data.rank)
      sprintf(log_memusage_impl_data.filename, "%s-rank_%d",
              getenv("LOG_MEMUSAGE_LOGFILE_NAME"),
              log_memusage_impl_data.rank);
  }

  /* set up the polling and output interval timing info */
  {
    /* set up the polling interval, for nanosleep, using input floating point value. */
    polling_interval_sec = atof(getenv("LOG_MEMUSAGE_POLL_INTERVAL"));
    log_memusage_msg(stderr, "Using %g (sec) as LOG_MEMUSAGE_POLL_INTERVAL\n", polling_interval_sec);
    /* set up the output interval, user specifies as a float in seconds, but we transform into an
       integer multiple of polling_interval_sec. */
    output_interval_sec = atof(getenv("LOG_MEMUSAGE_OUTPUT_INTERVAL"));
    log_memusage_msg(stderr, "Using %g (sec) as LOG_MEMUSAGE_OUTPUT_INTERVAL\n", output_interval_sec);
    if (output_interval_sec < polling_interval_sec)
      output_interval_sec = polling_interval_sec;

    output_interval_step = (int) (output_interval_sec / polling_interval_sec);
    log_memusage_msg(stderr, "Using %d as output_interval_step\n", output_interval_step);
    log_memusage_impl_data.output_step_cnt = output_interval_step;

    /* now set the polling time, we modify polling_interval_sec so be careful not to reuse that ... */
    log_memusage_impl_data.sleep_time.tv_sec = 0;
    while (polling_interval_sec >= 1.)
      {
        log_memusage_impl_data.sleep_time.tv_sec += 1;
        polling_interval_sec -= 1.;
      }
    assert (polling_interval_sec < 1.);
    log_memusage_impl_data.sleep_time.tv_nsec = (int) (1e9 * polling_interval_sec);
  }


  log_memusage_impl_data.fptr = NULL;

  gettimeofday(&log_memusage_impl_data.start_time, NULL);

  pthread_mutex_init(&log_memusage_impl_data.mutex, NULL);

  log_memusage_impl_data.running = false;

  log_memusage_resume();
}



__attribute__((destructor))
void log_memusage_finalize ()
{
  /* printf("..(destructor)... %s, line: %d\n", __FILE__, __LINE__); */

  log_memusage_pause();

  if (NULL != log_memusage_impl_data.fptr)
    {
      fclose(log_memusage_impl_data.fptr);
      log_memusage_impl_data.fptr = NULL;
    }

  log_memusage_report(/* prefix = */ LOG_MEMUSAGE_LOGGING_PREFIX);

  log_memusage_finalize_nvml();

  pthread_mutex_destroy(&log_memusage_impl_data.mutex);
}



#ifdef ENABLE_FORTRAN
/* Fortran API Follows */
void FC_FUNC_(log_memusage_annotate, LOG_MEMUSAGE_ANNOTATE) (const char* label)
{ log_memusage_annotate(label); }

void FC_FUNC_(log_memusage_report, LOG_MEMUSAGE_REPORT) ()
{ log_memusage_report(LOG_MEMUSAGE_LOGGING_PREFIX); }

void FC_FUNC_(log_memusage_initialize, LOG_MEMUSAGE_INITIALIZE) ()
{ log_memusage_initialize(); }

void FC_FUNC_(log_memusage_finalize, LOG_MEMUSAGE_FINALIZE) ()
{ log_memusage_finalize(); }

void FC_FUNC_(log_memusage_pause, LOG_MEMUSAGE_PAUSE) ()
{ log_memusage_pause(); }

void FC_FUNC_(log_memusage_resume, LOG_MEMUSAGE_RESUME) ()
{ log_memusage_resume(); }

#endif /* #ifdef ENABLE_FORTRAN */
