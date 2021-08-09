#!/bin/busybox sh

curr_path=$(dirname $(readlink -f $0))
mod_path=$curr_path/double_D/modules

cd $mod_path
for file in `ls -a $mod_path`
do
    if [ "${file##*.}"x = "ko"x ]; then
        insmod $file
    fi
done