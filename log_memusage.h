#ifndef LOG_MEMUSAGE_H
#define LOG_MEMUSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

  int    log_memusage_annotate (const char*);
  double log_memusage_get ();
  double log_memusage_report (const char*);
  int    log_memusage_pause ();
  int    log_memusage_resume ();

#ifdef __cplusplus
}
#endif
#endif /* #define LOG_MEMUSAGE_H */
