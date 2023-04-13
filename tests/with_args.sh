#!/bin/bash

./peak_memusage ./use_memory -s 8000000 || exit 1
./peak_memusage ./use_memory -s 2000000 -r 4 || exit 1

exit 0
