#!/bin/bash

OMP_NUM_THREADS=4 ./peak_memusage.exe ./use_memory_openmp.exe || exit 1

exit 0
