#ifndef LOG_MEMUSAGE_H
#define LOG_MEMUSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

  /**
   * \mainpage
   * \p log \p memusage is a shared library and optional API that monitors an applications memory usage.
   */

  /**
     \page peak_memusage
     \p peak_memusage foo
     \section peak_memusageNAME NAME
     \p peak_memusage - Determine the high-water mark memory usage of an application.

     \section peak_memusageSYNOPSIS SYNOPSIS
     \verbatim
     $ peak_memusage <program> --args --for-program

     $ mpiexec peak_memusage <mpi_program>
     \endverbatim

     \section peak_memusageDESCRIPTION DESCRIPTION
     peak_memusage determines the high-water mark of an
     application's memory usage.

     This utility is a wrapper around \p getrusage, which reports peak
     memory use of any executable. It is OpenMP and MPI aware and
     tries to report thread- and task- specific data. Of course,
     being OpenMP shared memory, that report cannot really separate
     thread-specific memory use.

     Note that the memory usage is reported only at the end of the
     run.

     \section peak_memusageOPTIONS OPTIONS
     The peak_memusage does not take any options.

     \section peak_memusageEXAMPLES EXAMPLES
     \verbatim
     $ ./peak_memusage ./use_memory
     Running: ./use_memory; sleep 1  - Please wait...
     Allocating, using and freeing 1000000 ints (3.81 MiB) in thread 0/1
     casper-login2 used memory in task 0: 30.82MiB (+23.03MiB overhead). ExitStatus: 0. Signal: 0
     \endverbatim
  */

  /**
     \page log_memusage log_memusage
     \p log_memusage bar
     \section log_memusageNAME NAME
     \p log_memusage - Logs memory usage history of an application.

     \section log_memusageSYNOPSIS SYNOPSIS
     \verbatim
     $ gcc -o my_app my_app.c -llog_memusage
     $ ./my_app

     $ mpiexec -x LD_PRELOAD=/path/to/liblog_memusage.so <mpi_program>
     \endverbatim

     \section log_memusageDESCRIPTION DESCRIPTION
     \p log_memusage is a library that monitors and reports application memory usage.

     \section log_memusageENVIRONMENT ENVIRONMENT

     General Environment Variables

     \subsection  LOG_MEMUSAGE_VERBOSE
     If enabled (1), prints key library events to \p stderr.

     Default: 0

     \subsection  LOG_MEMUSAGE_ENABLE_LOGFILE
     If enabled (1), logs memory usage vs. time to the output file "memory_usage.log",
     or \p LOG_MEMUSAGE_LOGFILE_NAME.

     Default: 0

     \subsection  LOG_MEMUSAGE_LOGFILE_NAME
     When \p LOG_MEMUSAGE_ENABLE_LOGFILE is enabled, \p LOG_MEMUSAGE_LOGFILE_NAME specifies the name of the
     output logfile.  Can be a relative or full path.  MPI rank information will be appended, if detected at run time.

     Default: "memory_usage.log"

     \subsection  LOG_MEMUSAGE_POLL_INTERVAL
     Floating point value specifying the polling frequency (in seconds).

     Default: 0.1 (sec)

     \subsection  LOG_MEMUSAGE_OUTPUT_INTERVAL
     Floating point value specifying the output frequency (in seconds).  Must be greater than or equal to \p LOG_MEMUSAGE_POLL_INTERVAL.

     Default: 1.0 (sec)

     \subsection LOG_MEMUSAGE_TRIPWIRES LOG_MEMUSAGE_CPU_MEM_TRIPWIRE, LOG_MEMUSAGE_GPU_MEM_TRIPWIRE
     If enabled, kill the application immediately if it exceeds the specified value (MB).

     Default: \p INT_MAX (MB)

     \section log_memusageEXAMPLES EXAMPLES
     \subsection baseline Baseline uninstrumented program:

     \verbatim
     $ ./use_memory -s 50000000
     Allocating, using and freeing 50000000 ints (190.73 MiB) in thread 0/1
     \endverbatim

     \subsection ld_preload Using LD_PRELOAD to instrument an existing program:
     \verbatim
     $ LD_PRELOAD=./liblog_memusage.so ./use_memory -s 50000000
     logging memory usage for process 103284 to memory_usage.log ...
     Allocating, using and freeing 50000000 ints (190.73 MiB) in thread 0/1
     # (memusage) --> casper-login2 / PID 103284, peak used memory: 213 MiB
     \endverbatim

     \subsection ld_preload_openmp Using LD_PRELOAD to instrument an existing OpenMP program:
     \verbatim
     $ LD_PRELOAD=./liblog_memusage.so OMP_NUM_THREADS=6 ./use_memory_openmp -s 50000000
     logging memory usage for process 106876 to memory_usage.log ...
     Allocating, using and freeing 100000000 ints (381.47 MiB) in thread 1/6
     Allocating, using and freeing 200000000 ints (762.94 MiB) in thread 3/6
     Allocating, using and freeing 150000000 ints (572.20 MiB) in thread 2/6
     Allocating, using and freeing 50000000 ints (190.73 MiB) in thread 0/6
     Allocating, using and freeing 250000000 ints (953.67 MiB) in thread 4/6
     Allocating, using and freeing 300000000 ints (1144.41 MiB) in thread 5/6
     # (memusage) --> casper-login2 / PID 106876, peak used memory: 4036 MiB
     \endverbatim

     \subsection pkgconfig Using pkg-config configuration to link with an application:
     \verbatim
     # compile:
     $ gcc -o use_memory ./use_memory.c $(pkg-config log_memusage --libs --cflags)

     # run:
     $ ./use_memory
     logging memory usage for process 111615 to memory_usage.log ...
     Allocating, using and freeing 1000000 ints (3.81 MiB) in thread 0/1
     # (memusage) --> casper-login2 / PID 111615, peak used memory: 24 MiB
     \endverbatim
  */

  /**
   * Insert the specified \p annotation into the memory log file.
   * This is useful for example to annotate what phase of the main program is executing at a given time.
   */
  int  log_memusage_annotate (const char* annotation);


  /**
   * Gets the amount of CPU memory used.
   * \returns The amount of CPU memory currently in use (MB).
   */
  int  log_memusage_get ();


  /**
   * Prints the current memory usage status to \p stderr.
   * \returns The amount of CPU  memory currently in use (MB).
   */
  int  log_memusage_report (const char* prefix);


  /**
   * Pauses memory monitoring by termininaing the execution thread.
   */
  int  log_memusage_pause ();


  /**
   * Resumes memory monitoring by restarting the execution thread.
   */
  int  log_memusage_resume ();

  /**
   * Initializes the memory logging data structures and execution thread.
   */
  void log_memusage_initialize ();

  /**
   * Terminates the memory logging execution thread.
   */
  void log_memusage_finalize ();

#ifndef LOG_MEMUSAGE_MAX_GPU_DEVICES
#  define LOG_MEMUSAGE_MAX_GPU_DEVICES 8
#endif

  /**
   * Structure for reporting memory usage for (potentially) multiple GPUs.
   */
  struct log_memusage_gpu_memory
  {
    /**
     * The number of GPU deviced dectected.
     */
    unsigned int device_count;

    /**
     * Current memory usage of each GPU (MB).
     */
    int used[LOG_MEMUSAGE_MAX_GPU_DEVICES];

    /**
     * Current free memory remaining of each GPU (MB).
     */
    int free[LOG_MEMUSAGE_MAX_GPU_DEVICES];

    /**
     * Maximum memory used on any of the \p device_count GPUs (MB).
     */
    int max_used;

    /**
     * Total memory used on all of the \p device_count GPUs (MB).
     */
    int total_used;
  };

  typedef struct log_memusage_gpu_memory log_memusage_gpu_memory_t;

  /**
   * \returns the number of GPUs active for this \p main() application.
   */
  int log_memusage_ngpus ();

  /**
   * \returns the latest memory information for all GPUs.
   */
  log_memusage_gpu_memory_t log_memusage_get_each_gpu ();

  /**
   * \returns the total memory used on all of the \p device_count GPUs (MB).
   */
  int log_memusage_get_all_gpus ();

  /**
   * \returns the maximum memory used on any of the \p device_count GPUs (MB).
   */
  int log_memusage_get_max_gpu ();

#ifdef __cplusplus
}
#endif
#endif /* #define LOG_MEMUSAGE_H */

//  LocalWords:  memusage subsubsection LOGFILE logfile
