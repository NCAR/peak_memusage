#ifndef LOG_MEMUSAGE_IMPL_H
#define LOG_MEMUSAGE_IMPL_H

#include <pthread.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

  /*
   * Private Implementation Details.
   */
  int log_memusage_msg (FILE* f, const char* format, ...);

  pthread_t log_memusage_get_parent_thread_ID();

  void log_memusage_print_trace ();
  void log_memusage_signal_handler (int signum);
  void log_memusage_register_signal_handler ();

  void log_memusage_initialize_nvml ();
  void log_memusage_finalize_nvml ();

#define LOG_MEMUSAGE_INVALID_RANK -1
#define LOG_MEMUSAGE_LOGGING_PREFIX "# (memusage) --> "

#ifdef __cplusplus
}
#endif
#endif /* #define LOG_MEMUSAGE_IMPL_H */
