# #!/bin/bash
# mkdir etc dev mnt
# mkdir -p proc sys tmp mnt
# mkdir -p etc/init.d


# sudo chmod 755 etc/init.d/rcS
# sudo chmod 755 etc/inittab
# sudo cp -a /dev/{null,console,tty,tty1,tty2,tty3,tty4} dev/
# #sudo cp -a /etc/{hosts,resolv.conf,nsswitch.conf} etc/
# sudo cp -rf ../busybox-1.32.1/_install/linuxrc ./
# cd dev
# mknod console c 5 1
# mknod null c 1 3
# mknod tty1 c 4 1
# cd ..

# rm -rf rootfs.ext4
# rm -rf fs
# dd if=/dev/zero of=./rootfs.ext4 bs=1M count=64
# mkfs.ext4 rootfs.ext4
# mkdir fs
# mount -o loop rootfs.ext4 ./fs
# cp -rf . ./fs

# umount ./fs
# gzip --best -c rootfs.ext4 > ../manual_ramdisk.img 
# rm -rf rootfs.ext4
# rm -rf fs
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../manual_ramdisk.img
