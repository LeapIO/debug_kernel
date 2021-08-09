# http://nickdesaulniers.github.io/blog/2018/10/24/booting-a-custom-linux-kernel-in-qemu-and-debugging-it-with-gdb/
# https://wiki.archlinux.org/index.php/QEMU_(%E7%AE%80%E4%BD%93%E4%B8%AD%E6%96%87)

# change kernel code, commit log:
# 	populated kernel src + xxxxx

# cgdb -q -x gdbinit q与x的顺序还得必须是这样的

# path
DIR_CUR = $(shell pwd)
KERNEL_ROOT = $(DIR_CUR)/../
BZIMAGE = $(KERNEL_ROOT)/arch/x86/boot/bzImage
# mkinitramfs -o ramdisk.img
AUTO_RAMDISK = $(DIR_CUR)/auto_ramdisk.img
# manual initrmfs
# find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../manual_ramdisk.img
MANUAL_RAMDISK_DIR = $(DIR_CUR)/build_ramdisk/
GEN_MANUAL_RAMDISK_DIR = $(MANUAL_RAMDISK_DIR)/initrmfs/
GEN_MANUAL_RAMDISK_SHELL = $(GEN_MANUAL_RAMDISK_DIR)/gen.sh
MANUAL_RAMDISK = $(MANUAL_RAMDISK_DIR)/manual_ramdisk.img

# start qemu
QEMU = qemu-system-x86_64

# qemu parameter
# -kernel bzImage use 'bzImage' as kernel image
PARAMETER := -kernel $(BZIMAGE)
# PARAMETER += -hda $(HDIMG)
# nographic disable graphical output and redirect serial I/Os to console
PARAMETER += -nographic
# -initrd file    use 'file' as initial ram disk
# PARAMETER += -initrd $(AUTO_RAMDISK)
PARAMETER += -initrd $(MANUAL_RAMDISK)
# -append cmdline use 'cmdline' as kernel command line
PARAMETER += -append "console=ttyS0 nokaslr"  # kernel cmdline
# i meet a problem, Could not access KVM kernel module: No such file or directory
# maybe i need to enter bios to enable virtualization
# PARAMETER += --enable-kvm
# -m configure guest RAM 客户机, host共计4G内存
PARAMETER += -m 2G,slots=4,maxmem=16G
# 指定qemu虚拟机的核心数
PARAMETER += -smp 1
# PARAMETER += -cpu host
# -S pause the kernel unit we continue in gdb
# -s start gdbserver on port 1234
PARAMETER += -s -S

# qemu 模拟 nvdimm 所需要的独立参数
# the "nvdimm" machine option enables vNVDIMM feature
NVDIMM_PARAMETER := -machine pc,nvdimm
BACKEDN_NVDIMM1_PATH = $(DIR_CUR)/nvdimm/nvdimmdaxfile
NVDIMM_PARAMETER += -object memory-backend-file,id=mem1,share=on,mem-path=$(BACKEDN_NVDIMM1_PATH),size=4G,align=2M
# mem1的4G后备存储设备是host中的基于ext4文件系统的文件
NVDIMM_PARAMETER += -device nvdimm,id=nv1,memdev=mem1

# qemu 模拟nvme所需的独立参数
# file: This option defines which disk image (see disk_images) to use with this drive
# if=type: This option defines on which type on interface the drive is connected. Available types are:
# 	ide, scsi, sd, mtd, floppy, pflash, virtio, none.
BACKEDN_NVMe_PATH = $(DIR_CUR)/nvme/nvme.img
NVMe_PARAMETER := -drive file=$(BACKEDN_NVMe_PATH),if=none,id=D22 -device nvme,drive=D22,serial=1234

# qemu模拟一个disk设备给busybox用
# By default, interface is "ide" and index is automatically incremented
# 	default is ide disk under scsi
BACKEDN_DISK_PATH = $(DIR_CUR)/disk/disk.img
DISK_PARAMETER := -drive file=$(BACKEDN_DISK_PATH)

.PHONY:help
help:
	@echo make dk  -- start debug kernel with a disk device in /dev
	@echo make dn  -- start debug nvdimm with a nvdimm device in /dev
	@echo make dnvme  -- start debug nvme ssd with a nvme ssd device in /dev

# default target and cgdb -q -x gdbinit in another termianl
# dk means means debug kernel
dk:
	$(QEMU) $(PARAMETER) $(DISK_PARAMETER)

# debug nvdimm
dn:
	$(QEMU) $(PARAMETER) $(NVDIMM_PARAMETER)

#debug nvme ssd
dnvme:
	$(QEMU) $(PARAMETER) $(NVMe_PARAMETER)

# 利用mkinitramfs生成一个默认的initrmdisk
GEN_AUTO_RAMDISK = $(shell mkinitramfs -o auto_ramdisk.img)
aud:
	@echo $(GEN_AUTO_RAMDISK)

# 利用busybox手动生成initrmfs，需要在initrmfs下执行
# @ 一行只能有一个
# makefile中每一行都是一个单独的进程
mad: 
	@cd $(GEN_MANUAL_RAMDISK_DIR) && sh $(GEN_MANUAL_RAMDISK_SHELL)

.PHONY:test
test:
	@echo do check

.PHONY:clean
clean:
	@echo clean done