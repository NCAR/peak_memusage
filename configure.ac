AC_PREREQ([2.62]) dnl autoconf >= 2.62 for AC_OPENMP

AC_INIT([peak_memusage], [3.0.0], [csgteam@ucar.edu])

AC_MSG_RESULT([---------------------------------------------])
AC_MSG_RESULT([-------- Configuring peak_memusage ----------])
AC_MSG_RESULT([---------------------------------------------])

AC_CONFIG_SRCDIR([peak_memusage.c])
top_srcdir=`dirname $0`
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([dist-bzip2 tar-pax subdir-objects color-tests 1.11 foreign])
AM_SILENT_RULES(yes) dnl # use silent rules - automake 1.11

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_FC

AC_FC_LIBRARY_LDFLAGS
AC_FC_WRAPPERS
AM_PROG_CC_C_O # mpl_examples/Makefile.am:74: warning: compiling ... in subdir requires 'AM_PROG_CC_C_O'

AC_LANG([C])
LT_INIT

AC_CONFIG_HEADERS([config.h])
AC_CHECK_HEADERS_ONCE([sys/types.h stdio.h stdlib.h time.h string.h unistd.h sys/resource.h])

AX_PTHREAD

# options
AC_OPENMP
openmp_ok=no
enable_openmp=no
if test "x$ac_cv_prog_c_openmp" != "xunsupported" && test "x$ac_cv_prog_c_openmp" != "x"; then
  openmp_ok=yes
  enable_openmp=yes
  AC_MSG_RESULT([<<< OpenMP-capable compiler detected. >>>])
  AC_DEFINE([ENABLE_OPENMP], [1], [Include OpenMP interface.])
fi
AM_CONDITIONAL([OPENMP_BUILD], [test x"$enable_openmp" = xyes])

AC_ARG_ENABLE(mpi,
    [  --enable-mpi            Include MPI interface.],
    [enable_mpi=$enableval],
    [enable_mpi="yes"])
    if test "x$enable_mpi" = "xyes"; then
      AX_MPI([found_mpi=yes], [found_mpi=no; enable_mpi=no])
      if test "x$found_mpi" = "xyes"; then
        AC_CHECK_PROGS([MPIEXEC], [mpiexec mpiexec_mpt mpirun], [mpiexec])
        AC_DEFINE([ENABLE_MPI], [1], [Build with MPI support (really only used for test suite)])
        AC_MSG_RESULT([<<< Enabling MPI support (used in test suite only). >>>])
        #CC="$MPICC"
      fi
    fi
AM_CONDITIONAL([MPI_BUILD], [test x"$enable_mpi" = xyes])

AC_CONFIG_FILES([
 Makefile
 mpi/Makefile
])
AC_OUTPUT
AC_MSG_RESULT([---------------------------------------------])
AC_MSG_RESULT([------ Done Configuring peak_memusage -------])
AC_MSG_RESULT([---------------------------------------------])

AX_SUMMARIZE_CONFIG
