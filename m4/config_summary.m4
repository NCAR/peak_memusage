# SYNOPSIS
#
#   Summarizes configuration settings.
#
#   AX_SUMMARIZE_CONFIG([, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
#
# DESCRIPTION
#
#   Outputs a summary of relevant configuration settings.
#
# LAST MODIFICATION
#
#   2009-11-19
#

AC_DEFUN([AX_SUMMARIZE_CONFIG],
[

######################################################################################
echo
echo '----------------------------------- SUMMARY -----------------------------------'
echo
echo Package version............... : $PACKAGE-$VERSION
echo
echo C compiler.................... : $CC
echo CFLAGS........................ : $CFLAGS
echo CPPFLAGS...................... : $CPPFLAGS
echo C++ compiler.................. : $CXX
echo CXXFLAGS...................... : $CXXFLAGS
echo Fortran compiler.............. : $FC
echo FCFLAGS....................... : $FCFLAGS
echo Install dir................... : $prefix
#echo Python install dir............ : $pythondir
#echo PYTHONPATH.................... : $PYTHONPATH
echo Build user.................... : $USER

######################################################################################
echo
echo 'Required Dependencies:'
echo '   Pthreads................... : (required)'
echo '      PTHREAD_CC.............. :' ${PTHREAD_CC}
echo '      PTHREAD_CFLAGS.......... :' ${PTHREAD_CFLAGS}
echo '      PTHREAD_LIBS............ :' ${PTHREAD_LIBS}
echo 'Optional Features & Dependencies:'
echo '   Fortran API................ :' ${enable_fortran}
echo '   NVML....................... :' ${enable_nvml}
echo '   OpenMP..................... :' ${enable_openmp}
echo '   MPI (test suite only)...... :' ${enable_mpi}
if test "x${enable_mpi}" = "xyes"; then
  echo '      MPICC................... :' ${MPICC}
  echo '      MPICXX.................. :' ${MPICXX}
  echo '      MPIEXEC................. :' ${MPIEXEC}
fi
echo '   pkg-config................. :' ${PKG_CONFIG}
echo '   Documentation.............. :' ${enable_doc}
if test "x${enable_doc}" = "xyes"; then
  echo '      DOXYGEN................. :' ${DOXYGEN}
fi
echo '-------------------------------------------------------------------------------'

echo
echo Configure complete, now type \'make\' and then \'make install\'.
echo
])
