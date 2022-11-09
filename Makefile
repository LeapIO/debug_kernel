# http://nickdesaulniers.github.io/blog/2018/10/24/booting-a-custom-linux-kernel-in-qemu-and-debugging-it-with-gdb/
# https://wiki.archlinux.org/index.php/QEMU_(%E7%AE%80%E4%BD%93%E4%B8%AD%E6%96%87)

# change kernel code, commit log:
# 	populated kernel src + xxxxx

# cgdb -q -x gdbinit

# path
DIR_CUR = $(shell pwd)
KERNEL_ROOT = $(DIR_CUR)/../
BZIMAGE = $(KERNEL_ROOT)/arch/x86/boot/bzImage
# mkinitramfs -o auto_ramdisk.img
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
# -m configure guest RAM 客户机
PARAMETER += -m 8G,slots=4,maxmem=16G \
-object memory-backend-ram,id=mem0,size=2G \
-object memory-backend-ram,id=mem1,size=2G \
-object memory-backend-ram,id=mem2,size=2G \
-object memory-backend-ram,id=mem3,size=2G 
# iommu
# Currently only Q35 platform supports guest vIOMMU
# PARAMETER += -device intel-iommu,intremap=on
# 虚一下网卡，hostfwd 该选项可以把虚拟机端口 guest_port 映射到主机端口 host_port 上，从而实现外部对虚拟机的访问
# 而其中的virtio类型是qemu-kvm对半虚拟化IO（virtio）驱动的支持
# e1000代表的是网卡型号
# https://blog.51cto.com/u_15077545/3985916
#PARAMETER += -net nic,model=e1000 \
# -net user,hostfwd=tcp::2222-:22
PARAMETER += -net nic -net tap,ifname=tap0,script=no,downscript=net_exit 
#-nic tap,ifname=tap0 
# 指定 qemu 虚拟机的核心数 并且指定 numa node topology
# https://futurewei-cloud.github.io/ARM-Datacenter/qemu/how-to-configure-qemu-numa-nodes/
PARAMETER += -smp 4 
#\#
# -numa node,memdev=mem0,cpus=0-3,nodeid=0 \#
# -numa node,memdev=mem1,cpus=4-7,nodeid=1 \#
# -numa node,memdev=mem2,cpus=8-11,nodeid=2 \#
# -numa node,memdev=mem3,cpus=12-15,nodeid=3

# PARAMETER += -cpu host
# -S pause the kernel unit we continue in gdb
# -s start gdbserver on port 1234
# -gdb tcp::1236 则可以指定其它gdbserver的监听端口
PARAMETER += -gdb tcp::1234 -S

# qemu 模拟 nvdimm 所需要的独立参数
# the "nvdimm" machine option enables vNVDIMM feature
NVDIMM_PARAMETER := -machine pc,nvdimm=on
BACKEDN_NVDIMM1_PATH = $(DIR_CUR)/nvdimm/nvdimmdaxfile
NVDIMM_PARAMETER += -object memory-backend-file,id=mem4,share=on,mem-path=$(BACKEDN_NVDIMM1_PATH),size=4G,align=2M
# mem1的4G后备存储设备是host中的基于ext4文件系统的文件
NVDIMM_PARAMETER += -device nvdimm,id=nv1,memdev=mem4

# https://qemu-project.gitlab.io/qemu/system/devices/nvme.html?highlight=nvme
# 	qemu 文档 nvme 参数
# qemu 模拟nvme所需的独立参数
# file: This option defines which disk image (see disk_images) to use with this drive
# if=type: This option defines on which type on interface the drive is connected. Available types are:
# 	ide, scsi, sd, mtd, floppy, pflash, virtio, none.
# num_queues meads 
# 	Set the maximum number of allowed I/O queue pairs IO 队列的数量确实是SSD所决定的，包括每一个队列的队列深度，位于pcie的bar空间上
#	实际上这个值在驱动初始化的是是可以作为 参数 给进去的
BACKEDN_NVMe_PATH = $(DIR_CUR)/nvme/nvme.img
NVME_PARAMETER := -drive file=$(BACKEDN_NVMe_PATH),format=raw,if=none,id=D22 -device nvme,drive=D22,serial=1234,max_ioqpairs=128

# qemu模拟一个disk设备给busybox用
# By default, interface is "ide" and index is automatically incremented
# 	default is ide disk under scsi
# https://wiki.gentoo.org/wiki/QEMU/Options#Hard_drive
# -drive id=disk,file=IMAGE.img,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0
BACKEDN_DISK_PATH = $(DIR_CUR)/disk/disk.img
# DISK_PARAMETER := -drive file=$(BACKEDN_DISK_PATH) -device ahci
DISK_PARAMETER := -drive id=disk,file=$(BACKEDN_DISK_PATH),format=raw,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0

# qemu将主机的PCIe HBA通过vfio的方式传递给qemu内的虚拟机
#HBA_HOST := 0000:01:00.0  # 这个换了设备是需要update的
HBA_HOST := 0000:$(shell lspci -d 10ee: | awk '{print $$1}')  # 只试用于主机上插了一张卡，多个卡时，还需按上面一行的指定具体哪张……
# HBA_PARAMETER_1 := --enable-kvm  # 如果有这个参数则需要hbreak打硬件断点才可以
HBA_PARAMETER_1 += -cpu host
HBA_PARAMETER_1 += -machine q35,accel=kvm,kernel-irqchip=split
HBA_PARAMETER_1 += -device intel-iommu,intremap=on,caching-mode=on # 重点是intremap参数，其他参数是为开启这项参数服务的
# 想要在qemu中使用多个msi中断向量，需要使用intel-iommu的中断重映射
# 重点是要开启iommu中的intremap, 除了宿主机上需要开启iommu,还有两个地方需要配置
# qemu启动参数配置中需要令intremap=on, 编译虚拟机的内核镜像时也需要开启IOMMU
# Device Drivers->IOMMU Hardware Support->Support for Interrupt Remapping
HBA_PARAMETER_2 := -device vfio-pci,host=$(HBA_HOST)

.PHONY:help
help:
	@echo make dr  -- start debug kernel without dev
	@echo make dk  -- start debug kernel with a disk device in /dev
	@echo make dn  -- start debug nvdimm with a nvdimm device in /dev
	@echo make dnvme  -- start debug nvme ssd with a nvme ssd device in /dev
	@echo make dhba  -- start debug pcie scsi host, hba
	@echo make mad -- gen manual ramdisk.img
	@echo make aud -- gen auto ramdisk.img
	@echo make gdk -- gen disk.img
	@echo make gnvme -- gen nvme.img
	@echo make gnvdimm -- gen nvdimm file

dr:
	bash net_init
	$(QEMU) $(PARAMETER)
# default target and cgdb -q -x gdbinit in another termianl
# dk means means debug kernel
dk:
	$(QEMU) $(PARAMETER) $(DISK_PARAMETER)

# debug nvdimm
dn:
	$(QEMU) $(PARAMETER) $(NVDIMM_PARAMETER)

# debug nvme ssd
dnvme:
	$(QEMU) $(PARAMETER) $(NVME_PARAMETER)

# debug pci hba scsi
# 主要是要把主机的PCIe设备透传过来
# $(HBA_PARAMETER_1)
dhba:
	$(QEMU) $(PARAMETER) $(HBA_PARAMETER_1) $(HBA_PARAMETER_2)

# 利用mkinitramfs生成一个默认的initrmdisk
# 这个在ubuntu的docker container中会有问题
GEN_AUTO_RAMDISK = $(shell mkinitramfs -o auto_ramdisk.img)
aud:
	@echo $(GEN_AUTO_RAMDISK)

# 8G
# mac 2G
GEN_DISK_BACKEND = $(shell mkdir -p disk && dd if=/dev/zero of=./disk/disk.img bs=2048 count=1024K)
gdk:
	@echo $(GEN_DISK_BACKEND)

GEN_NVME_BACKEND = $(shell mkdir -p nvme && dd if=/dev/zero of=./nvme/nvme.img bs=2048 count=4096K)
gnvme:
	@echo $(GEN_NVME_BACKEND)

GEN_NVDIMM_BACKEND = $(shell mkdir -p nvdimm && truncate -s 8G nvdimmdaxfile)
gnvdimm:
	@echo $(GEN_NVDIMM_BACKEND)

# 利用busybox手动生成initrmfs，需要在initrmfs下执行
# @ 一行只能有一个
# makefile中每一行都是一个单独的进程
mad: 
	@cd $(GEN_MANUAL_RAMDISK_DIR) && bash $(GEN_MANUAL_RAMDISK_SHELL)

.PHONY:test
test:
	@echo do check

.PHONY:clean
clean:
	@echo clean done
