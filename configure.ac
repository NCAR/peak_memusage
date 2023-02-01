AC_PREREQ([2.62]) dnl autoconf >= 2.62 for AC_OPENMP

AC_INIT([peak_memusage], [3.0.0], [csgteam@ucar.edu])

AC_MSG_RESULT([---------------------------------------------])
AC_MSG_RESULT([-------- Configuring peak_memusage ----------])
AC_MSG_RESULT([---------------------------------------------])

AC_CONFIG_SRCDIR([peak_memusage.cxx])
top_srcdir=$(dirname $0)
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

#-----------------------------------------------------
# Initialize libtool.  By default, we will build
# only shared libraries on platforms that support them
#-----------------------------------------------------
LT_INIT([disable-static])

AC_CONFIG_HEADERS([config.h])
AC_CHECK_HEADERS_ONCE([sys/types.h stdio.h stdlib.h time.h string.h unistd.h sys/resource.h])

AC_PATH_PROG([PKG_CONFIG], [pkg-config], [no])

AX_PTHREAD

# options
AC_ARG_ENABLE(fortran,
              [  --enable-fortran     Build Fortran API.],
              [enable_fortran=$enableval],
              [enable_fortran="yes"])
if test "x$enable_fortran" = "xyes"; then
    AC_DEFINE([ENABLE_FORTRAN], [1], [Build Fortran API.])
    AC_MSG_RESULT([<<< Enabling Fortran API >>>])
fi



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



AC_ARG_ENABLE(nvml,
              [  --enable-nvml           Include Nvidia NVML interface.],
              [enable_nvml=$enableval],
              [enable_nvml="yes"])
if test "x$enable_nvml" = "xyes"; then
  AC_PATH_PROG([NVIDIA_SMI], [nvidia-smi])
  AC_CHECK_LIB([dl], [dlopen], [], [enable_nvml=no])
  #AC_CHECK_LIB([nvidia-ml], [nvmlInit], [], [enable_nvml=no])
  if test "x$enable_nvml" = "xyes"; then
    AC_CHECK_HEADER([nvml.h], [nvml_label="(using external nvml.h)"; found_nvml_h=yes], [found_nvml_h=no])
    # If the library works, but we can't find the header, fall back to our
    # ./contrib version
    if test "x$found_nvml_h" = "xno"; then
      unset ac_cv_header_nvml_h nvml_label
      CPPFLAGS="${CPPFLAGS} -I${top_srcdir}/contrib/nvml/include"
      AC_CHECK_HEADER([nvml.h], [nvml_label="(using nvml.h from ./contrib)"; internal_nvml_h=yes], [enable_nvml=no])
    fi
  fi
  if test "x$enable_nvml" = "xyes"; then
    AC_DEFINE([ENABLE_NVML], [1], [Build with NVML GPU support])
    AC_MSG_RESULT([<<< Enabling Nvidia NVML GPU support ${nvml_label}. >>>])
  fi
fi
AM_CONDITIONAL([NVML_BUILD], [test x"$enable_nvml" = xyes])



AC_ARG_ENABLE(mpi,
              [  --enable-mpi            Include MPI interface.],
              [enable_mpi=$enableval],
              [enable_mpi="yes"])
if test "x$enable_mpi" = "xyes"; then
  AC_LANG_PUSH([C++])
  AX_MPI([found_mpicxx=yes], [found_mpicxx=no])
  AC_LANG_POP([C++])
  AX_MPI([found_mpi=yes], [found_mpi=no; enable_mpi=no])
  if test "x$found_mpi" = "xyes"; then
    AC_CHECK_PROGS([MPIEXEC], [mpiexec mpiexec_mpt mpirun], [mpiexec])
    AC_DEFINE([ENABLE_MPI], [1], [Build with MPI support (really only used for test suite)])
    AC_MSG_RESULT([<<< Enabling MPI support (used in test suite only). >>>])
    #CC="$MPICC"
  fi
fi
AM_CONDITIONAL([MPI_BUILD], [test x"$enable_mpi" = xyes])



AC_ARG_ENABLE(doc,
              [  --enable-doc            Build documentation (requires doxygen).],
              [enable_doc=$enableval],
              [enable_doc="no"])
    if test "x$enable_doc" = "xyes"; then
      AC_PROG_MKDIR_P
      AC_PATH_PROG([DOXYGEN], [doxygen], [disable])
      if test "x$DOXYGEN" = "xdisable"; then
        enable_doc="no"
      fi
    fi
AM_CONDITIONAL([BUILD_DOCS], [test x"$enable_doc" = xyes])

AC_CONFIG_FILES([
 Makefile
 mpi/Makefile
 doc/Doxyfile
 examples/Makefile
 log_memusage.pc
])
AC_OUTPUT
AC_MSG_RESULT([---------------------------------------------])
AC_MSG_RESULT([------ Done Configuring peak_memusage -------])
AC_MSG_RESULT([---------------------------------------------])

AX_SUMMARIZE_CONFIG
