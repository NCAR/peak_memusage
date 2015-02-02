# peak_memusage
This utility is a wrapper around getrusage, which reports peak memory 
use of any executable. It is OpenMP and MPI aware and tries to report 
thread- and task- specific data. Of course, being OpenMP shared memory,
that report cannot really separate thread-specific memory use.

Note that the memory usage is reported only at the end of the run.
