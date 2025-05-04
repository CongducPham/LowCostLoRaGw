#!/bin/bash
(
    while : ; do
        wvdial $1
        sleep 10
    done
) &
