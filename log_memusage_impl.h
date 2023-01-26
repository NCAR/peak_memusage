#ifndef LOG_MEMUSAGE_IMPL_H
#define LOG_MEMUSAGE_IMPL_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

  /*
   * Private Implementation Details.
   */
  pthread_t log_memusage_get_parent_thread_ID();

  void log_memusage_print_trace ();
  void log_memusage_signal_handler (int signum);
  void log_memusage_register_signal_handler ();


#define LOG_MEMUSAGE_INVALID_RANK -1


#ifdef __cplusplus
}
#endif
#endif /* #define LOG_MEMUSAGE_IMPL_H */
