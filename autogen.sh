#!/bin/bash

autoreconf=$(which autoreconf 2>/dev/null)
# prefer autoreconf when it is available
if (test "x$autoreconf" != "x"); then
    if (test -x $autoreconf); then
        $autoreconf -iv --force
        exit 0
    fi
fi
