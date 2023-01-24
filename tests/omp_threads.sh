#!/bin/bash

OMP_NUM_THREADS=4 ./peak_memusage ./use_memory_openmp || exit 1

exit 0
