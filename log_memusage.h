#ifndef LOG_MEMUSAGE_H
#define LOG_MEMUSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

  int  log_memusage_annotate (const char*);
  int  log_memusage_get ();
  int  log_memusage_report (const char*);
  int  log_memusage_pause ();
  int  log_memusage_resume ();
  void log_memusage_initialize ();
  void log_memusage_finalize ();

#ifdef __cplusplus
}
#endif
#endif /* #define LOG_MEMUSAGE_H */
