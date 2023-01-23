#!/bin/bash

./peak_memusage.exe ./use_memory.exe -s 8000000 || exit 1
./peak_memusage.exe ./use_memory.exe -s 2000000 -r 4 || exit 1

exit 0
