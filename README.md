# Log Memusage
## peak_memusage
This utility is a wrapper around getrusage, which reports peak memory
use of any executable. It is OpenMP and MPI aware and tries to report
thread- and task- specific data. Of course, being OpenMP shared memory,
that report cannot really separate thread-specific memory use.

Note that the memory usage is reported only at the end of the run.

```pre
NAME
       peak_memusage - Determine the high-water mark memory usage of an application.

SYNOPSIS
       $ peak_memusage <program> --args --for-program

       $ mpiexec peak_memusage <mpi_program>

DESCRIPTION
       peak_memusage determines the high-water mark of an applciation's memory usage.

       This  utility  is  a wrapper around getrusage, which reports peak memory use of any executable. It is OpenMP and MPI aware and tries to report thread- and task- specific data. Of course, being OpenMP shared
       memory, that report cannot really separate thread-specific memory use.

       Note that the memory usage is reported only at the end of the run.

OPTIONS
       The peak_memusage does not take any options.

EXAMPLES
       $ ./peak_memusage ./use_memory
       Running: ./use_memory; sleep 1  - Please wait...
       Allocating, using and freeing 1000000 ints (3.81 MiB) in thread 0/1
       casper-login2 used memory in task 0: 30.82MiB (+23.03MiB overhead). ExitStatus: 0. Signal: 0
```

## liblog_memusage.so

```pre
NAME
       log_memusage - Logs memory usage history of an application.

SYNOPSIS
       $ gcc -o my_app my_app.c -llog_memusage
       $ ./my_app

       $ mpiexec -x LD_PRELOAD=/path/to/liblog_memusage.so <mpi_program>

DESCRIPTION
       log_memusage is a library that monitors and reports application memory usage.~

ENVIRONMENT
   General Environment Variables
       LOG_MEMUSAGE
           If enabled (1), logs memory usage vs. time to the output file "memory_usage.log".

           Default: 0

       LOG_MEMUSAGE_POLL_INTERVAL
           Floating point value specifying the polling frequency (in seconds).

           Default: 0.1

        LOG_MEMUSAGE_OUTPUT_INTERVAL
           Integer value specifying output frequency, in multiples of LOG_MEMUSAGE_POLL_INTERVAL.  For example, a value of 10 will update the memory usage output log file every 10 polling intervals.

           Default: 1

EXAMPLES
   Baseline, uninstrumented program:
       $ ./use_memory -s 50000000
       Allocating, using and freeing 50000000 ints (190.73 MiB) in thread 0/1

   Using LD_PRELOAD to instrument an existing program:
       $ LD_PRELOAD=./liblog_memusage.so ./use_memory -s 50000000
       logging memory usage for process 103284 to memory_usage.log ...
       Allocating, using and freeing 50000000 ints (190.73 MiB) in thread 0/1
       # (memusage) --> casper-login2 / PID 103284, peak used memory: 213 MiB

   Using LD_PRELOAD to instrument an existing OpenMP program:
       $ LD_PRELOAD=./liblog_memusage.so OMP_NUM_THREADS=6 ./use_memory_openmp -s 50000000
       logging memory usage for process 106876 to memory_usage.log ...
       Allocating, using and freeing 100000000 ints (381.47 MiB) in thread 1/6
       Allocating, using and freeing 200000000 ints (762.94 MiB) in thread 3/6
       Allocating, using and freeing 150000000 ints (572.20 MiB) in thread 2/6
       Allocating, using and freeing 50000000 ints (190.73 MiB) in thread 0/6
       Allocating, using and freeing 250000000 ints (953.67 MiB) in thread 4/6
       Allocating, using and freeing 300000000 ints (1144.41 MiB) in thread 5/6
       # (memusage) --> casper-login2 / PID 106876, peak used memory: 4036 MiB

   Using pkg-config configuration to link with an application:
       compile:
       $ gcc -o use_memory ./use_memory.c $(pkg-config log_memusage --libs --cflags)

       run:
       $ ./use_memory
       logging memory usage for process 111615 to memory_usage.log ...
       Allocating, using and freeing 1000000 ints (3.81 MiB) in thread 0/1
       # (memusage) --> casper-login2 / PID 111615, peak used memory: 24 MiB
```