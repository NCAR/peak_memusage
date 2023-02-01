ACLOCAL_AMFLAGS = -I m4
AM_DISTCHECK_CONFIGURE_FLAGS = --enable-doc

TESTS =
XFAIL_TESTS =

bin_PROGRAMS =
check_PROGRAMS =
lib_LTLIBRARIES =
include_HEADERS =
SUBDIRS =
EXTRA_DIST =
CLEANFILES =
BUILT_SOURCES =

#----------------------------------------------------------
# programs & libraries
#----------------------------------------------------------
bin_PROGRAMS += peak_memusage
peak_memusage_SOURCES = peak_memusage.cxx

lib_LTLIBRARIES += liblog_memusage.la
liblog_memusage_la_SOURCES = log_memusage.c log_memusage.h \
                             log_memusage_impl.h log_memusage_tripwire.c \
	                     log_memusage_nvml.c
liblog_memusage_la_CFLAGS = $(PTHREAD_CFLAGS)
liblog_memusage_la_LIBADD = $(PTHREAD_LIBS)
include_HEADERS += log_memusage.h

bin_PROGRAMS += demo_log_memusage
demo_log_memusage_SOURCES = demo_log_memusage.c
demo_log_memusage_LDADD = liblog_memusage.la

# Support for pkgconfig
pkgconfigdir     = $(libdir)/pkgconfig
pkgconfig_DATA   = log_memusage.pc

#----------------------------------------------------------
# docs
#----------------------------------------------------------
cached_mans = doc/dist/peak_memusage.3 \
	      doc/dist/log_memusage.3 \
              doc/dist/log_memusage.h.3

EXTRA_DIST += $(cached_mans)

if BUILD_DOCS

man3_MANS = doc/doxygen/man/man3/peak_memusage.3 \
	    doc/doxygen/man/man3/log_memusage.3 \
            doc/doxygen/man/man3/log_memusage.h.3
doxygen:
	$(AM_V_GEN)$(DOXYGEN) ./doc/Doxyfile >/dev/null &&  \
	for file in $(man3_MANS); do \
	  cp -f $${file} $(top_srcdir)/doc/dist/ || continue ; \
	done

docstamp: Makefile $(liblog_memusage_la_SOURCES)
	@$(MAKE) doxygen
	$(AM_V_GEN)touch $@

BUILT_SOURCES += docstamp
else

man3_MANS = $(cached_mans)

endif

#----------------------------------------------------------
# tests
#----------------------------------------------------------
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


#----------------------------------------------------------
# final details
#----------------------------------------------------------
EXTRA_DIST += $(TESTS)
CLEANFILES += $(BUILT_SOURCES)

clean-local:
	rm -rf ./doc/doxygen

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
