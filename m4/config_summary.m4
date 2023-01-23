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
#echo C++ compiler.................. : $CXX
#echo CXXFLAGS.......................: $CXXFLAGS
echo C compiler.................... : $CC
echo CFLAGS.........................: $CFLAGS
#echo Fortran compiler.............. : $FC
#echo Fortran 77 compiler........... : $F77
#echo FCFLAGS........................: $FCFLAGS
echo Install dir................... : $prefix
#echo Python install dir............ : $pythondir
#echo PYTHONPATH.................... : $PYTHONPATH
echo Build user.................... : $USER

######################################################################################
echo
echo Optional Dependencies:
echo '   'OpenMP:.................... : ${enable_openmp}
echo '   'MPI:....................... : ${enable_mpi}
echo
echo '-------------------------------------------------------------------------------'

echo
echo Configure complete, now type \'make\' and then \'make install\'.
echo
])
