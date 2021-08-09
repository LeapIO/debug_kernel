ifneq  ($(KERNELRELEASE),)
obj-m += kvisual.o  # 内核可视化模块
kvisual-objs := \
	entry.o \
	iovisual.o
else
PWD = $(shell pwd)
KERNEL_DIR = $(PWD)/../../../
endif

modules:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

TARGET_DIR = $(PWD)/../../build_ramdisk/initrmfs/double_D/modules
CP = $(shell cp kvisual.ko $(TARGET_DIR))
exp: $(CP)
	@echo export successfully

UPGTAGS = $(shell cd $(KERNEL_DIR) | global -u)
upg:
	@echo $(UPGTAGS)

.PHONY:test
test:
	@echo $(PWD) 
	@echo $(KERNEL_DIR)

.PHONY:clean
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean