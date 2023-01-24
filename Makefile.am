ACLOCAL_AMFLAGS = -I m4

TESTS =
XFAIL_TESTS =

bin_PROGRAMS =
check_PROGRAMS =
lib_LTLIBRARIES =
include_HEADERS =
SUBDIRS =



bin_PROGRAMS += peak_memusage
peak_memusage_SOURCES = peak_memusage.c

lib_LTLIBRARIES += liblog_memusage.la
liblog_memusage_la_SOURCES = log_memusage.c log_memusage.h
liblog_memusage_la_LIBADD = -lpthread

include_HEADERS += log_memusage.h

bin_PROGRAMS += demo_log_memusage
demo_log_memusage_SOURCES = demo_log_memusage.c
demo_log_memusage_LDADD = liblog_memusage.la

check_PROGRAMS += use_memory
use_memory_SOURCES = use_memory.c

XFAIL_TESTS += tests/forgot_exe.sh
TESTS += tests/check.sh tests/with_args.sh $(XFAIL_TESTS)

if OPENMP_BUILD
  check_PROGRAMS += use_memory_openmp
  use_memory_openmp_CPPFLAGS = -DCOMPILE_OMP
  use_memory_openmp_CFLAGS = $(OPENMP_CFLAGS)
  use_memory_openmp_SOURCES = use_memory.c

  TESTS += tests/omp_threads.sh
endif

if MPI_BUILD
  SUBDIRS += mpi
  TESTS += tests/mpi.sh

if OPENMP_BUILD
  TESTS += tests/mpi_omp_threads.sh
endif
endif


EXTRA_DIST = $(TESTS)


mytags:
	cd $(top_srcdir) ; pwd ; \
	if [ "x$(STR)" != "x" ]; then \
	  echo "Tagging files containing $(STR)" ; \
	  git grep -l $(STR) ; \
	  etags $$(git grep -l $(STR)) ; \
	else \
	  echo "Tagging all git managed files:" ; \
	  git ls-tree -r HEAD --name-only ; \
	  etags $$(git ls-tree -r HEAD --name-only) ; \
	fi
