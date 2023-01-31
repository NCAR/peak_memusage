#!/bin/bash
#PBS -q regular
#PBS -A SCSG0001
#PBS -l walltime=00:30:00
#PBS -l select=1:ncpus=12:mpiprocs=12
#PBS -j oe

module reset >/dev/null 2>&1
#module unload cuda    >/dev/null 2>&1
#module unload openmpi >/dev/null 2>&1
#module load nvhpc/22.5 >/dev/null 2>&1 || { echo "cannot find requested module!"; exit 1; }
module list

mpicxx -o hello_world_mpi_ssend_recv hello_world_mpi_ssend_recv.C -std=c++11 || exit 1

mpiexec_mpt ./hello_world_mpi_ssend_recv

export LOG_MEMUSAGE_VERBOSE=1
export LOG_MEMUSAGE_ENABLE_LOGFILE=1
export LD_PRELOAD=/glade/work/benkirk/peak_memusage/chey/install/lib/liblog_memusage.so
mpiexec_mpt ./hello_world_mpi_ssend_recv

echo && echo && echo "Done at $(date)"
