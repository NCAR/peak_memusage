#ifndef LOG_MEMUSAGE_H
#define LOG_MEMUSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

  /*
   * Public API.
   */
  int  log_memusage_annotate (const char*);
  int  log_memusage_get ();
  int  log_memusage_report (const char*);
  int  log_memusage_pause ();
  int  log_memusage_resume ();
  void log_memusage_initialize ();
  void log_memusage_finalize ();

#ifndef LOG_MEMUSAGE_MAX_GPU_DEVICES
#  define LOG_MEMUSAGE_MAX_GPU_DEVICES 8
#endif

  struct log_memusage_gpu_memory
  {
    unsigned int device_count;
    int used[LOG_MEMUSAGE_MAX_GPU_DEVICES];
    int free[LOG_MEMUSAGE_MAX_GPU_DEVICES];
    int max_used;
    int total_used;
  };

  typedef struct log_memusage_gpu_memory log_memusage_gpu_memory_t;

  int log_memusage_ngpus ();
  log_memusage_gpu_memory_t log_memusage_get_each_gpu ();
  int log_memusage_get_all_gpus ();
  int log_memusage_get_max_gpu ();

#ifdef __cplusplus
}
#endif
#endif /* #define LOG_MEMUSAGE_H */
