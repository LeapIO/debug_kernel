#!/bin/bash
path=$(pwd)
mount=/mnt/toydfsclient/
for i in $(seq 1 32); do
    echo "$i begin create"
    cd $path && ./dfstest.sh
    echo "$i begin delete"
    cd $mount && rm -rf *
    echo "$i finish delete"
done
exit 0