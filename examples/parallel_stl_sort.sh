#!/bin/bash
#PBS -q casper
#PBS -A SCSG0001
#PBS -l walltime=00:30:00
#PBS -l select=1:ncpus=8:ompthreads=8:mem=10G:ngpus=1:gpu_model=v100
#PBS -j oe

module reset >/dev/null 2>&1
module unload cuda    >/dev/null 2>&1
module unload openmpi >/dev/null 2>&1
module load nvhpc/22.5 >/dev/null 2>&1 || { echo "cannot find requested module!"; exit 1; }
module list

nvc++ -stdpar=multicore -O2 -o parallel_stl_sort parallel_stl_sort.C || exit 1

export LOG_MEMUSAGE_VERBOSE=1
export LOG_MEMUSAGE_ENABLE_LOGFILE=1
export LOG_MEMUSAGE_LOGFILE_NAME="memory_usage_multicore.log"
LD_PRELOAD=/glade/work/benkirk/peak_memusage/dav/install/lib/liblog_memusage.so ./parallel_stl_sort

nvc++ -stdpar=gpu -O2 -o parallel_stl_sort parallel_stl_sort.C || exit 1

export LOG_MEMUSAGE_LOGFILE_NAME="memory_usage_gpu.log"
LD_PRELOAD=/glade/work/benkirk/peak_memusage/dav/install/lib/liblog_memusage.so ./parallel_stl_sort

unset LOG_MEMUSAGE_LOGFILE_NAME
export LOG_MEMUSAGE_GPU_MEM_TRIPWIRE=4096
LD_PRELOAD=/glade/work/benkirk/peak_memusage/dav/install/lib/liblog_memusage.so ./parallel_stl_sort

echo && echo && echo "Done at $(date)"
