TESTS =
bin_PROGRAMS =
noinst_PROGRAMS =

bin_PROGRAMS += peak_memusage.exe
peak_memusage_exe_SOURCES = peak_memusage.c

#libexec_PROGRAMS = use_memory.exe
noinst_PROGRAMS += use_memory.exe
use_memory_exe_SOURCES = use_memory.c

if OPENMP_BUILD
  noinst_PROGRAMS += use_memory_openmp.exe
  use_memory_openmp_exe_CPPFLAGS = -DCOMPILE_OMP
  use_memory_openmp_exe_SOURCES = use_memory.c

  TESTS += tests/omp_threads.sh
endif

XFAIL_TESTS = tests/forgot_exe.sh
TESTS += tests/check.sh tests/with_args.sh $(XFAIL_TESTS)
