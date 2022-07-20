#!/bin/bash

disks_idx=(b c d e f g h i)

for di in ${disks_idx[@]}
do
	sudo mkfs.ext4 -F /dev/sd${di}
	sudo mkdir -p /mnt/sd${di}
	sudo mount /dev/sd${di} /mnt/sd${di}
done


