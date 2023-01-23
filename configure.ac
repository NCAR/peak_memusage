AC_PREREQ([2.62]) dnl autoconf >= 2.62 for AC_OPENMP
AC_INIT([peak_memusage], [2.1.0], [csgteam@ucar.edu])
AC_MSG_RESULT([---------------------------------------------])
AC_MSG_RESULT([-------- Configuring peak_memusage ----------])
AC_MSG_RESULT([---------------------------------------------])

AC_CONFIG_SRCDIR([peak_memusage.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([dist-bzip2 tar-pax subdir-objects color-tests 1.11 foreign])
# use silent rules - automake 1.11
AM_SILENT_RULES(yes)

AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AC_CHECK_HEADERS_ONCE([sys/types.h stdio.h stdlib.h time.h string.h unistd.h sys/resource.h])


# options
AC_OPENMP
openmp_ok=no
enable_openmp=no
if test "x$ac_cv_prog_c_openmp" != "xunsupported" && test "x$ac_cv_prog_c_openmp" != "x"; then
  openmp_ok=yes
  enable_openmp=yes
  AC_MSG_RESULT([<<< OpenMP-capable compiler detected. >>>])
  AC_DEFINE([ENABLE_OPENMP], [1], [Include OpenMP interface.])
  CFLAGS="$CFLAGS $OPENMP_CFLAGS"
fi
AM_CONDITIONAL([OPENMP_BUILD], [test x"$enable_openmp" = xyes])

dnl AC_ARG_ENABLE(mpi,
dnl     [  --enable-mpi            Include MPI interface.],
dnl     [enable_mpi=$enableval],
dnl     [enable_mpi="no"])
dnl if test "$enable_mpi" = "yes"; then
dnl   AX_MPI([found_mpi=yes], [found_mpi=no; enable_mpi=no])
dnl   if test "$found_mpi" = "yes"; then
dnl     AC_DEFINE([ENABLE_MPI], [1], [Build with MPI support])
dnl     AC_MSG_RESULT([<<< Enabling MPI support. >>>])
dnl   fi
dnl fi
dnl AM_CONDITIONAL([MPI_BUILD], [test x"$enable_mpi" = xyes])


AC_CONFIG_FILES([
 Makefile
])
AC_OUTPUT
AC_MSG_RESULT([---------------------------------------------])
AC_MSG_RESULT([------ Done Configuring peak_memusage -------])
AC_MSG_RESULT([---------------------------------------------])

AX_SUMMARIZE_CONFIG
