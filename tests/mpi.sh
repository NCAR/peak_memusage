#!/bin/bash

mpiexec=$(which mpiexec_mpt 2>/dev/null || which mpiexec 2>/dev/null || echo "none")

[ -x ${mpiexec} ] || exit 0

export MPI_SHEPHERD=true

${mpiexec} -n 4 ./peak_memusage.exe ./use_memory.exe || exit 1

exit 0
