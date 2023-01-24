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
#include <pthread.h>



struct log_memusage_data_str
{
  struct rusage usage;
  int retval;
  pthread_t thread;
} log_memusage_data;



void* log_memusage_thread ( void* ptr)
{
  fprintf(stderr, "logging memory...\n");
  usleep(5000);
  sleep(5);
}


int log_memusage ()
{
  log_memusage_data.retval =
    pthread_create (&log_memusage_data.thread, NULL, log_memusage_thread, NULL);
}



int report_memusage ()
{
  struct rusage ous;/* resource us struct */

  if (getrusage(RUSAGE_SELF, &ous))
    fprintf(stderr, "Problem getting resource usage\n");

  else
    fprintf(stderr, "Used memory: %.2fMiB\n", ous.ru_maxrss/1024.);

  return 0;
}



/**
 * ------------------------------------------------------------------
 */
__attribute__((constructor))
void initialize_log_memory_usage ()
{
  printf("..(constructor)...\n");
  log_memusage();
}



__attribute__((destructor))
void finalize_log_memory_usage ()
{
  printf("..(destructor)...\n");
  pthread_join(log_memusage_data.thread, NULL);

  report_memusage();
}



#ifdef BUILD_APP
int main (int argc, char **argv)
{
  char hn[256];

  gethostname(hn, sizeof(hn) / sizeof(char));

  printf("Hello from %s\n", hn);
}
#endif
