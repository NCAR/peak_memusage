#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "log_memusage.h"
#include "log_memusage_impl.h"



/* Obtain a backtrace and print it to stdout. */
__attribute__ ((visibility ("hidden")))
void log_memusage_print_trace ()
{
  void *array[50];
  /* char **strings; */
  int size /*, i */;

  size = backtrace (array, sizeof(array) / sizeof(void*) );
  fprintf(stderr, "Obtained %d stack frames.\n", size);
  backtrace_symbols_fd(array, size, 2);

  /* strings = backtrace_symbols (array, size); */
  /* if (strings != NULL) */
  /*   { */
  /*     for (i = 0; i < size; i++) */
  /*       fprintf(stderr, "%s\n", strings[i]); */
  /*   } */

  /* free (strings); */
}



__attribute__ ((visibility ("hidden")))
void log_memusage_signal_handler (int signum)
{
  if ( ! pthread_equal(log_memusage_get_parent_thread_ID(), pthread_self()) )
    {
      printf("Exting Signal handler on thread...\n");
      return;
    }



  switch (signum)
    {
    case SIGINT:
      printf("...caught SIGINT\n");
      break;

    case SIGTERM:
      printf("...caught SIGTERM\n");
      break;

    case SIGUSR1:
      printf("...caught SIGUSR1, ignoring...\n");
      return;

    case SIGUSR2:
      printf("...caught SIGUSR2, tracing & aborting...\n");
      log_memusage_print_trace();
      abort();
      return;

    default:
      printf("...caught other unknown signal: %d\n", signum);
      printf("   see \"man 7 signal\" for a list of known signals\n");
      break;
    }


  return;
}



__attribute__ ((visibility ("hidden")))
void log_memusage_register_signal_handler ()
{
  const int verbose = (getenv("LOG_MEMUSAGE_VERBOSE")          != NULL) ? atoi(getenv("LOG_MEMUSAGE_VERBOSE")) : 0;
  if (verbose) printf("# (memusage) --> Registering user-specified signal handlers\n");

  /* signal(SIGINT,  log_memusage_signal_handler); */
  /* signal(SIGTERM, log_memusage_signal_handler); */
  /* signal(SIGUSR1, log_memusage_signal_handler); */
  signal(SIGUSR2, log_memusage_signal_handler);
}
