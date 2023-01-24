#!/bin/bash

mpiexec=$(which mpiexec_mpt 2>/dev/null || which mpiexec 2>/dev/null || echo "none")

# https://www.gnu.org/software/automake/manual/html_node/Scripts_002dbased-Testsuites.html
# 0:  success
# 77: skipped test
# 99: hard error
# other: fail
[ -x ${mpiexec} ] || exit 77

export MPI_SHEPHERD=true

${mpiexec} -n 4 ./hello_world_mpi || exit 77 # hello world isn't really a test, but if it fails, skip the real test

exit 0
