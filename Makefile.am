TESTS =
XFAIL_TESTS =
MPI_TESTS =
bin_PROGRAMS =
check_PROGRAMS =

bin_PROGRAMS += peak_memusage.exe
peak_memusage_exe_SOURCES = peak_memusage.c

#libexec_PROGRAMS = use_memory.exe
check_PROGRAMS += use_memory.exe
use_memory_exe_SOURCES = use_memory.c

XFAIL_TESTS += tests/forgot_exe.sh
MPI_TESTS += tests/mpi.sh
TESTS += tests/check.sh tests/with_args.sh $(XFAIL_TESTS)

if OPENMP_BUILD
  check_PROGRAMS += use_memory_openmp.exe
  use_memory_openmp_exe_CPPFLAGS = -DCOMPILE_OMP
  use_memory_openmp_exe_SOURCES = use_memory.c

  TESTS += tests/omp_threads.sh
  MPI_TESTS += tests/mpi_omp_threads.sh
endif

if MPI_BUILD
  check_PROGRAMS += hello_world_mpi
  hello_world_mpi_SOURCES = hello_world_mpi.c

  TESTS += $(MPI_TESTS)
endif



EXTRA_DIST = $(TESTS) $(MPI_TESTS)
