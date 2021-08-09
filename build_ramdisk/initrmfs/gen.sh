#!/bin/bash
sudo cp -a /dev/{null,console,tty,tty1,tty2,tty3,tty4} dev/
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../manual_ramdisk.img